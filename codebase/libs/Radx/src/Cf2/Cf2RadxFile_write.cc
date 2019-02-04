// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// Cf2RadxFile_write.cc
//
// Write methods for Cf2RadxFile object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2017
//
///////////////////////////////////////////////////////////////

#include <Radx/Cf2RadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxStr.hh>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// If addDaySubDir is true, a  subdir will be
// created with the name dir/yyyymmdd/
//
// If addYearSubDir is true, a subdir will be
// created with the name dir/yyyy/
//
// If both addDaySubDir and addYearSubDir are true,
// the subdir will be dir/yyyy/yyyymmdd/
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getDirInUse() for dir written
// Use getPathInUse() for path written

int Cf2RadxFile::writeToDir(const RadxVol &vol,
                            const string &dir,
                            bool addDaySubDir,
                            bool addYearSubDir)
  
{

  if (_debug) {
    cerr << "DEBUG - Cf2RadxFile::writeToDir" << endl;
    cerr << "  Writing to dir: " << dir << endl;
  }

  _writePaths.clear();
  _writeDataTimes.clear();
  clearErrStr();

  // should we split the sweeps?

  if (_writeIndividualSweeps) {
    return _writeSweepsToDir(vol, dir, addDaySubDir, addYearSubDir);
  }

  _writeVol = &vol;
  _dirInUse = dir;

  RadxTime startTime(_writeVol->getStartTimeSecs());
  int startMillisecs = (int) (_writeVol->getStartNanoSecs() / 1.0e6 + 0.5);
  if (startMillisecs > 999) {
    startTime.set(_writeVol->getStartTimeSecs() + 1);
    startMillisecs -= 1000;
  }
  RadxTime endTime(_writeVol->getEndTimeSecs());
  int endMillisecs = (int) (_writeVol->getEndNanoSecs() / 1.0e6 + 0.5);
  if (endMillisecs > 999) {
    endTime.set(_writeVol->getEndTimeSecs() + 1);
    endMillisecs -= 1000;
  }

  RadxTime fileTime = startTime;
  int fileMillisecs = startMillisecs;
  if (_writeFileNameMode == FILENAME_WITH_END_TIME_ONLY) {
    fileTime = endTime;
    fileMillisecs = endMillisecs;
  }

  string outDir(dir);
  if (addYearSubDir) {
    char yearStr[BUFSIZ];
    sprintf(yearStr, "%s%.4d", PATH_SEPARATOR, fileTime.getYear());
    outDir += yearStr;
  }
  if (addDaySubDir) {
    char dayStr[BUFSIZ];
    sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_SEPARATOR,
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay());
    outDir += dayStr;
  }

  // make sure output subdir exists
  
  if (makeDirRecurse(outDir)) {
    _addErrStr("ERROR - Cf2RadxFile::writeToDir");
    _addErrStr("  Cannot make output dir: ", outDir);
    return -1;
  }
  
  // compute write path

  string writePath = _computeWritePath(vol,
                                       startTime, startMillisecs,
                                       endTime, endMillisecs,
                                       fileTime, fileMillisecs,
                                       outDir);

  // perform the write to the computed path

  int iret = writeToPath(*_writeVol, writePath);

  if (iret) {
    _addErrStr("ERROR - Cf2RadxFile::_writeToDir");
    return -1;
  }

  return 0;

}

////////////////////////////////////////////
// split volume into sweeps and write to dir

int Cf2RadxFile::_writeSweepsToDir(const RadxVol &vol,
                                   const string &dir,
                                   bool addDaySubDir,
                                   bool addYearSubDir)
  
{

  if (_debug) {
    cerr << "DEBUG - Cf2RadxFile::_writeSweepsToDir" << endl;
    cerr << "  Splitting volume into sweeps" << endl;
  }
  
  // write one file for each sweep
  
  const vector<RadxSweep *> &sweeps = vol.getSweeps();
  
  for (size_t ii = 0; ii < sweeps.size(); ii++) {
    
    const RadxSweep &sweep = *sweeps[ii];
    
    // construct new volume just for this sweep
    
    RadxVol *sweepVol = new RadxVol(vol, sweep.getSweepNumber());
    
    // write the sweep file
    
    if (_writeSweepToDir(*sweepVol, dir, addDaySubDir, addYearSubDir)) {
      delete sweepVol;
      return -1;
    }

    // clean up

    delete sweepVol;

  } // ii

  // success

  return 0;

}

////////////////////////
// write a sweep to dir

int Cf2RadxFile::_writeSweepToDir(const RadxVol &vol,
                                  const string &dir,
                                  bool addDaySubDir,
                                  bool addYearSubDir)
  
{
  
  clearErrStr();
  _writeVol = &vol;
  _dirInUse = dir;
  
  const RadxSweep &sweep = *_writeVol->getSweeps()[0];
  int volNum = vol.getVolumeNumber();
  int sweepNum = sweep.getSweepNumber();
  string scanType(Radx::sweepModeToShortStr(sweep.getSweepMode()));
  double fixedAngle = sweep.getFixedAngleDeg();

  if (_debug) {
    cerr << "DEBUG - Cf2RadxFile::_writeSweepToDir" << endl;
    cerr << "  Writing sweep to dir: " << dir << endl;
    cerr << "  Vol num, scan mode: "
         << volNum << ", " << scanType << endl;
    cerr << "  Sweep num, fixed angle: "
         << sweepNum << ", " << fixedAngle << endl;
  }

  RadxTime startTime(_writeVol->getStartTimeSecs());
  int startMillisecs = (int) (_writeVol->getStartNanoSecs() / 1.0e6 + 0.5);
  RadxTime endTime(_writeVol->getEndTimeSecs());
  int endMillisecs = (int) (_writeVol->getEndNanoSecs() / 1.0e6 + 0.5);

  RadxTime fileTime = startTime;
  int fileMillisecs = startMillisecs;
  if (_writeFileNameMode == FILENAME_WITH_END_TIME_ONLY) {
    fileTime = endTime;
    fileMillisecs = endMillisecs;
  }

  string outDir(dir);
  if (addYearSubDir) {
    char yearStr[BUFSIZ];
    sprintf(yearStr, "%s%.4d", PATH_SEPARATOR, fileTime.getYear());
    outDir += yearStr;
  }
  if (addDaySubDir) {
    char dayStr[BUFSIZ];
    sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_SEPARATOR,
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay());
    outDir += dayStr;
  }

  // make sure output subdir exists
  
  if (makeDirRecurse(outDir)) {
    _addErrStr("ERROR - Cf2RadxFile::writeToDir");
    _addErrStr("  Cannot make output dir: ", outDir);
    return -1;
  }
  
  // compute path

  string fixedAngleLabel = "el";
  if (sweep.getSweepMode() == Radx::SWEEP_MODE_RHI ||
      sweep.getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    fixedAngleLabel = "az";
  }

  string instName(vol.getInstrumentName());
  if (instName.size() > 4) {
    instName.resize(4);
  }

  char fileName[BUFSIZ];
  if (_writeFileNameMode == FILENAME_WITH_START_AND_END_TIMES) {
    sprintf(fileName,
            "cfrad2.%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d"
            "_to_%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d"
            "_%s_v%d_s%.2d_%s%.2f_%s.nc",
            startTime.getYear(), startTime.getMonth(), startTime.getDay(),
            startTime.getHour(), startTime.getMin(), startTime.getSec(),
            startMillisecs,
            endTime.getYear(), endTime.getMonth(), endTime.getDay(),
            endTime.getHour(), endTime.getMin(), endTime.getSec(),
            endMillisecs,
            instName.c_str(),
            volNum, sweepNum,
            fixedAngleLabel.c_str(), fixedAngle,
            scanType.c_str());
  } else {
    sprintf(fileName,
            "cfrad2.%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d"
            "_%s_v%d_s%.2d_%s%.2f_%s.nc",
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay(),
            fileTime.getHour(), fileTime.getMin(), fileTime.getSec(),
            fileMillisecs,
            instName.c_str(),
            volNum, sweepNum,
            fixedAngleLabel.c_str(), fixedAngle,
            scanType.c_str());
  }
  
  char outPath[BUFSIZ];
  sprintf(outPath, "%s%s%s",
          outDir.c_str(), PATH_SEPARATOR,  fileName);
  
  int iret = writeToPath(*_writeVol, outPath);

  if (iret) {
    _addErrStr("ERROR - Cf2RadxFile::_writeToDir");
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int Cf2RadxFile::writeToPath(const RadxVol &vol,
                             const string &path)
  
{

  clearErrStr();
  _writeVol = &vol;
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _writePaths.clear();
  _writeDataTimes.clear();

  // open the output Ncxx file

  _tmpPath = tmpPathFromFilePath(path, "");

  if (_debug) {
    cerr << "DEBUG - Cf2RadxFile::writeToPath" << endl;
    cerr << "  Writing to path: " << path << endl;
    cerr << "  Tmp path is: " << _tmpPath << endl;
    cerr << "  Writing fields and compressing ..." << endl;
  }

  try {
    _file.open(_tmpPath, NcxxFile::replace, NcxxFile::nc4);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Cf2RadxFile::writeToPath");
    _addErrStr("  Cannot open tmp Ncxx file for writing: ", _tmpPath);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  if (_writeProposedStdNameInNcf) {
    _file.setUsedProposedStandardName(true);
  }

  // will we write for variable number of gates?
  
  _writeVol->computeMaxNGates();

  // get the set of unique field name

  _uniqueFieldNames = _writeVol->getUniqueFieldNameList();

  // check if georeferences and/or corrections are active
  // and count georef fields
  
  _checkGeorefsActiveOnWrite();
  _checkCorrectionsActiveOnWrite();
  _writeVol->countGeorefsNotMissing(_geoCount);

  if (_verbose) {
    cerr << "============= GEOREF FIELD COUNT ==================" << endl;
    _geoCount.print(cerr);
    cerr << "===================================================" << endl;
  }

  // add attributes, dimensions and variables
  
  try {
    _addGlobalAttributes();
  } catch (NcxxException e) {
    return _closeOnError("_addGlobalAttributes");
  }

  try {
    _addRootDimensions();
  } catch (NcxxException e) {
    return _closeOnError("_addRootDimensions");
  }

  try {
    _addRootScalarVariables();
  } catch (NcxxException e) {
    return _closeOnError("_addRootScalarVariables");
  }

  if (_writeVol->getInstrumentType() == Radx::INSTRUMENT_TYPE_RADAR) {
    try {
      _addRadarParameters();
    } catch (NcxxException e) {
      return _closeOnError("_addRadarParameters");
    }
  } else {
    try {
      _addLidarParameters();
    } catch (NcxxException e) {
      return _closeOnError("_addLidarParameters");
    }
  }

  try {
    _addRadarCalibration();
  } catch (NcxxException e) {
    return _closeOnError("_addRadarCalibration");
  }

  if (_correctionsActive) {
    try {
      _addGeorefCorrections();
    } catch (NcxxException e) {
      return _closeOnError("_addGeorefCorrections");
    }
  }

  try {
    _addLocation();
  } catch (NcxxException e) {
    return _closeOnError("_addLocation");
  }

  try {
    _addProjection();
  } catch (NcxxException e) {
    return _closeOnError("_addProjection");
  }

  // add sweep groups

  try {
    _addSweeps();
  } catch (NcxxException e) {
    return _closeOnError("_addSweeps");
  }

  // close output file

  _file.close();

  // rename the tmp to final output file path
  
  if (rename(_tmpPath.c_str(), _pathInUse.c_str())) {
    int errNum = errno;
    _addErrStr("ERROR - Cf2RadxFile::writeToPath");
    _addErrStr("  Cannot rename tmp file: ", _tmpPath);
    _addErrStr("  to: ", _pathInUse);
    _addErrStr(strerror(errNum));
    return -1;
  }

  if (_debug) {
    cerr << "DEBUG - Cf2RadxFile::writeToPath" << endl;
    cerr << "  Renamed tmp path: " << _tmpPath << endl;
    cerr << "     to final path: " << path << endl;
  }

  _writePaths.push_back(path);
  _writeDataTimes.push_back(vol.getStartTimeSecs());
  
  return 0;

}

/////////////////////////////////////////////////
// check if georeferences are active on write

void Cf2RadxFile::_checkGeorefsActiveOnWrite()
{
  
  _georefsActive = false;
  _georefsApplied = false;
  const vector<RadxRay *> &rays = _writeVol->getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay &ray = *rays[ii];
    const RadxGeoref *geo = ray.getGeoreference();
    if (geo != NULL) {
      _georefsActive = true;
      if (ray.getElevationDeg() != Radx::missingMetaDouble && 
          ray.getAzimuthDeg() != Radx::missingMetaDouble) {
        if (fabs(ray.getElevationDeg() - geo->getRotation()) > 0.001 ||
            fabs(ray.getAzimuthDeg() - geo->getTilt()) > 0.001) {
          _georefsApplied = true;
        }
      }
    }
  }

}
  
/////////////////////////////////////////////////
// check if corrections are active on write

void Cf2RadxFile::_checkCorrectionsActiveOnWrite()
{
  
  _correctionsActive = false;
  if (_writeVol->getCfactors() != NULL) {
    _correctionsActive = true;
  }

}
  
/////////////////////////////////////////////////
// set flags to indicate whether estimate noise
// is available

void Cf2RadxFile::_setEstNoiseAvailFlags()

{

  _estNoiseAvailHc = false;
  _estNoiseAvailVc = false;
  _estNoiseAvailHx = false;
  _estNoiseAvailVx = false;

  // estimated noise per channel

  const vector<RadxRay *> &rays = _writeVol->getRays();

  for (size_t ii = 0; ii < rays.size(); ii++) {
    if (rays[ii]->getEstimatedNoiseDbmHc() > -9990) {
      _estNoiseAvailHc = true;
      break;
    }
  }

  for (size_t ii = 0; ii < rays.size(); ii++) {
    if (rays[ii]->getEstimatedNoiseDbmVc() > -9990) {
      _estNoiseAvailVc = true;
      break;
    }
  }

  for (size_t ii = 0; ii < rays.size(); ii++) {
    if (rays[ii]->getEstimatedNoiseDbmHx() > -9990) {
      _estNoiseAvailHx = true;
      break;
    }
  }

  for (size_t ii = 0; ii < rays.size(); ii++) {
    if (rays[ii]->getEstimatedNoiseDbmVx() > -9990) {
      _estNoiseAvailVx = true;
      break;
    }
  }

}

///////////////////////////////////////////////////////////////
// add global attributes
// throws exception on error

void Cf2RadxFile::_addGlobalAttributes()
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_addGlobalAttributes()" << endl;
  }

  bool haveError = false;

  // Add required CF global attributes
  
  _conventions = CfConvention;
  _subconventions = BaseConvention;
  // _subconventions += " ";
  // _subconventions += INSTRUMENT_PARAMETERS;
  // if (_writeVol->getInstrumentType() == Radx::INSTRUMENT_TYPE_RADAR) {
  //   _subconventions += " ";
  //   _subconventions += RADAR_PARAMETERS;
  //   if (_writeVol->getRcalibs().size() > 0) {
  //     _subconventions += " ";
  //     _subconventions += RADAR_CALIBRATION;
  //   }
  // } else {
  //   _subconventions += " ";
  //   _subconventions += LIDAR_PARAMETERS;
  // }
  // if (_writeVol->getPlatformType() != Radx::PLATFORM_TYPE_FIXED) {
  //   _subconventions += " ";
  //   _subconventions += PLATFORM_VELOCITY;
  // }
  // if (_writeVol->getCfactors() != NULL) {
  //   _subconventions += " ";
  //   _subconventions += GEOMETRY_CORRECTION;
  // }

  _file.addGlobAttr(CONVENTIONS, _conventions);
  _file.addGlobAttr(SUB_CONVENTIONS, _subconventions);
  
  // Version

  _version = CurrentVersion2;
  // if (_writeVol->getVersion().size() > 0) {
  //   _version = _writeVol->getVersion();
  // }
  _file.addGlobAttr(VERSION, _version);
  
  // info strings

  _file.addGlobAttr(TITLE, _writeVol->getTitle());
  _file.addGlobAttr(INSTITUTION, _writeVol->getInstitution());
  _file.addGlobAttr(REFERENCES,  _writeVol->getReferences());
  _file.addGlobAttr(SOURCE,  _writeVol->getSource());

  string history = "Written by Cf2RadxFile class.";
  if (_writeVol->getHistory().size() > 0) {
    history += "\n";
    history += _writeVol->getHistory();
  }
  _file.addGlobAttr(HISTORY,  history);

  _file.addGlobAttr(COMMENT,  _writeVol->getComment());

  if (_writeVol->getAuthor().size() > 0) {
    _file.addGlobAttr(AUTHOR,  _writeVol->getAuthor());
  }

  string origFormat = "CFRADIAL";
  if (_writeVol->getOrigFormat().size() > 0) {
    origFormat = _writeVol->getOrigFormat();
  }
  if (origFormat.find("AR2") != string::npos) {
    // special case for NEXRAD AR2
    _file.addGlobAttr("format",  origFormat);
    _file.addGlobAttr(ORIGINAL_FORMAT, "NEXRAD");
  } else {
    _file.addGlobAttr(ORIGINAL_FORMAT,  origFormat);
  }

  _file.addGlobAttr(DRIVER,  _writeVol->getDriver());
  _file.addGlobAttr(CREATED,  _writeVol->getCreated());
    
  // times
  
  RadxTime startTime(_writeVol->getStartTimeSecs());
  _file.addGlobAttr(TIME_COVERAGE_START, startTime.getW3cStr());
  // _file.addGlobAttr(START_DATETIME, startTime.getW3cStr());
  startTime += _writeVol->getStartNanoSecs() / 1.0e9;
  _file.addGlobAttr(START_TIME, startTime.asStringDashed(3));

  RadxTime endTime(_writeVol->getEndTimeSecs());
  _file.addGlobAttr(TIME_COVERAGE_END, endTime.getW3cStr());
  // _file.addGlobAttr(END_DATETIME, endTime.getW3cStr());
  endTime += _writeVol->getEndNanoSecs() / 1.0e9;
  _file.addGlobAttr(END_TIME, endTime.asStringDashed(3));

  // names

  _file.addGlobAttr(INSTRUMENT_NAME,  _writeVol->getInstrumentName());
  _file.addGlobAttr(SITE_NAME,  _writeVol->getSiteName());
  _file.addGlobAttr(SCAN_NAME,  _writeVol->getScanName());
  _file.addGlobAttr(SCAN_ID,  _writeVol->getScanId());

  string attrStr;
  if (_writeVol->getPlatformType() == Radx::PLATFORM_TYPE_FIXED) {
    attrStr = "false";
  } else {
    attrStr = "true";
  }
  _file.addGlobAttr(PLATFORM_IS_MOBILE, attrStr);
    
  if (_rayTimesIncrease) {
    attrStr = "true";
  } else {
    attrStr = "false";
  }
  _file.addGlobAttr(RAY_TIMES_INCREASE, attrStr);

  // add user-specified global attributes

  const vector<RadxVol::UserGlobAttr> &attrs = _writeVol->getUserGlobAttr();
  for (size_t ii = 0; ii < attrs.size(); ii++) {
    const RadxVol::UserGlobAttr &attr = attrs[ii];
    switch (attr.attrType) {
      case RadxVol::UserGlobAttr::ATTR_STRING: {
        _file.addGlobAttr(attr.name, attr.val);
        break;
      }
      case RadxVol::UserGlobAttr::ATTR_INT: {
        int ival;
        if (sscanf(attr.val.c_str(), "%d", &ival) == 1) {
          _file.addGlobAttr(attr.name, ival);
        } else {
          _addErrStr("ERROR - Cf2RadxFile::_addGlobalAttributes()");
          _addErrStr("  Cannot decode user-defined global attribute");
          _addErrStr("   name: ", attr.name);
          _addErrStr("    type: int");
          _addErrStr("    val: ", attr.val);
        }
        break;
      }
      case RadxVol::UserGlobAttr::ATTR_DOUBLE: {
        double dval;
        if (sscanf(attr.val.c_str(), "%lg", &dval) == 1) {
          _file.addGlobAttr(attr.name, dval);
        } else {
          _addErrStr("ERROR - Cf2RadxFile::_addGlobalAttributes()");
          _addErrStr("  Cannot decode user-defined global attribute");
          _addErrStr("   name: ", attr.name);
          _addErrStr("    type: double");
          _addErrStr("    val: ", attr.val);
        }
        break;
      }
      case RadxVol::UserGlobAttr::ATTR_INT_ARRAY: {
        // tokenize values string
        vector<string> toks;
        RadxStr::tokenize(attr.val, " ,", toks);
        if (toks.size() == 0) {
          haveError = true;
        }
        RadxArray<int> ivals_;
        int *ivals = ivals_.alloc(toks.size());
        for (size_t jj = 0; jj < toks.size(); jj++) {
          int ival;
          if (sscanf(toks[jj].c_str(), "%d", &ival) == 1) {
            ivals[jj] = ival;
          } else {
            haveError = true;
          }
        } // jj
        if (!haveError) {
          try {
            _file.putAtt(attr.name, ncxxInt, toks.size(), ivals);
          } catch (NcxxException& e) {
            haveError = true;
            _addErrStr("ERROR - Cf2RadxFile::_addGlobalAttributes()");
            _addErrStr("  Cannot put attribute");
            _addErrStr("  Exception: ", e.what());
          }
        } else {
          _addErrStr("ERROR - Cf2RadxFile::_addGlobalAttributes()");
          _addErrStr("  Cannot decode user-defined global attribute");
        }
        if (haveError) {
          _addErrStr("   name: ", attr.name);
          _addErrStr("    type: int[]");
          _addErrStr("    val: ", attr.val);
        }
        break;
      }
      case RadxVol::UserGlobAttr::ATTR_DOUBLE_ARRAY: {
        // tokenize values string
        vector<string> toks;
        RadxStr::tokenize(attr.val, " ,", toks);
        if (toks.size() == 0) {
          haveError = true;
        }
        RadxArray<double> dvals_;
        double *dvals = dvals_.alloc(toks.size());
        for (size_t jj = 0; jj < toks.size(); jj++) {
          double dval;
          if (sscanf(toks[jj].c_str(), "%lg", &dval) == 1) {
            dvals[jj] = dval;
          } else {
            haveError = true;
          }
        } // jj
        if (!haveError) {
          try {
            _file.putAtt(attr.name, ncxxDouble, toks.size(), dvals);
          } catch (NcxxException& e) {
            haveError = true;
            _addErrStr("ERROR - Cf2RadxFile::_addGlobalAttributes()");
            _addErrStr("  Cannot put attribute");
            _addErrStr("  Exception: ", e.what());
          }
        } else {
          _addErrStr("ERROR - Cf2RadxFile::_addGlobalAttributes()");
          _addErrStr("  Cannot decode user-defined global attribute");
        }
        if (haveError) {
          _addErrStr("   name: ", attr.name);
          _addErrStr("    type: double[]");
          _addErrStr("    val: ", attr.val);
        }
        break;
      }
      default: {}
    } // switch
  } // ii
    
  if (haveError) {
    _addErrStr("ERROR - Cf2RadxFile::_addGlobalAttributes");
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

}

///////////////////////////////////////////////////////////////////////////
// Add top-level dims
// throws exception on error

void Cf2RadxFile::_addRootDimensions()
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_addRootDimensions()" << endl;
  }

  // add sweep dimension
  
  _sweepDim = _file.addDim(SWEEP, _writeVol->getSweeps().size());
  
  // add multiple frequencies dimension
  
  if (_writeVol->getFrequencyHz().size() > 0) {
    _frequencyDim = _file.addDim(FREQUENCY, _writeVol->getFrequencyHz().size());
  }
  
}

//////////////////////////////////////////////
// add root-level scalar variables
// throws exception on error

void Cf2RadxFile::_addRootScalarVariables()
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_addRootScalarVariables()" << endl;
  }

  // start time
  
  {
    NcxxVar var =
      _file.addVar(TIME_COVERAGE_START, "", TIME_COVERAGE_START_LONG,
                   ncxxString, "", true);
    var.putAtt(COMMENT,
               "ray times are relative to start time in secs");
    RadxTime startTime(_writeVol->getStartTimeSecs());
    var.putStringScalar(startTime.getW3cStr());
  }
    
  // end time

  {
    NcxxVar var =
      _file.addVar(TIME_COVERAGE_END, "", TIME_COVERAGE_END_LONG,
                   ncxxString, "", true);
    var.putAtt(COMMENT,
               "ray times are relative to start time in secs");
    RadxTime endTime(_writeVol->getEndTimeSecs());
    var.putStringScalar(endTime.getW3cStr());
  }

  // volume number

  {
    NcxxVar var = _file.addVar(VOLUME_NUMBER, "", VOLUME_NUMBER_LONG,
                               ncxxInt, "", true);
    int volNum = _writeVol->getVolumeNumber();
    var.putVal(volNum);
  }


  // instrument type

  {
    NcxxVar var = _file.addVar(INSTRUMENT_TYPE, "", INSTRUMENT_TYPE_LONG,
                               ncxxString, "", true);
    var.putAtt(OPTIONS, Radx::instrumentTypeOptions());
    // var.putAtt(META_GROUP, INSTRUMENT_PARAMETERS);
    string instrumentType =
      Radx::instrumentTypeToStr(_writeVol->getInstrumentType());
    var.putStringScalar(instrumentType);
  }
    
  // platform type

  {
    NcxxVar var = _file.addVar(PLATFORM_TYPE, "", PLATFORM_TYPE_LONG,
                               ncxxString, "", true);
    var.putAtt(OPTIONS, Radx::platformTypeOptions());
    string platformType = Radx::platformTypeToStr(_writeVol->getPlatformType());
    var.putStringScalar(platformType);
  }

  // primary axis

  {
    NcxxVar var = _file.addVar(PRIMARY_AXIS, "", PRIMARY_AXIS_LONG,
                               ncxxString, "", true);
    var.putAtt(OPTIONS, Radx::primaryAxisOptions());
    string primaryAxis = Radx::primaryAxisToStr(_writeVol->getPrimaryAxis());
    var.putStringScalar(primaryAxis);
  }
    
  // status XML

  {
    NcxxVar var = _file.addVar(STATUS_XML, "", "status_of_instrument",
                               ncxxString, "", true);
    var.putStringScalar(_writeVol->getStatusXml());
  }

}

//////////////////////////////////////////////
// add radar parameters
// throws exception on error

void Cf2RadxFile::_addRadarParameters()
{
  
  if (_verbose) {
    cerr << "Cf2RadxFile::_addRadarParameters()" << endl;
  }

  NcxxGroup rparamsGroup = _file.addGroup(RADAR_PARAMETERS);

  // frequency

  _addFrequency(rparamsGroup);

  // radar antenna gain H
  
  {
    NcxxVar var =
      rparamsGroup.addVar(RADAR_ANTENNA_GAIN_H, "", RADAR_ANTENNA_GAIN_H_LONG,
                          ncxxFloat, DB, true);
    // var.putAtt(META_GROUP, RADAR_PARAMETERS);
    var.putVal((float) _writeVol->getRadarAntennaGainDbH());
  }
  
  // radar antenna gain V
  
  {
    NcxxVar var =
      rparamsGroup.addVar(RADAR_ANTENNA_GAIN_V, "", RADAR_ANTENNA_GAIN_V_LONG,
                          ncxxFloat, DB, true);
    // var.putAtt(META_GROUP, RADAR_PARAMETERS);
    var.putVal((float) _writeVol->getRadarAntennaGainDbV());
  }
  
  // radar beam width H
  
  {
    NcxxVar var =
      rparamsGroup.addVar(RADAR_BEAM_WIDTH_H, "", RADAR_BEAM_WIDTH_H_LONG,
                          ncxxFloat, DEGREES, true);
    // var.putAtt(META_GROUP, RADAR_PARAMETERS);
    var.putVal((float) _writeVol->getRadarBeamWidthDegH());
  }
  
  // radar beam width V
  
  {
    NcxxVar var =
      rparamsGroup.addVar(RADAR_BEAM_WIDTH_V, "", RADAR_BEAM_WIDTH_V_LONG,
                          ncxxFloat, DEGREES, true);
    // var.putAtt(META_GROUP, RADAR_PARAMETERS);
    var.putVal((float) _writeVol->getRadarBeamWidthDegV());
  }
  
  // radar band width
  
  {
    NcxxVar var =
      rparamsGroup.addVar(RADAR_RX_BANDWIDTH, "", RADAR_RX_BANDWIDTH_LONG,
                          ncxxFloat, HZ, true);
    // var.putAtt(META_GROUP, RADAR_PARAMETERS);
    float bandwidthHz = _writeVol->getRadarReceiverBandwidthMhz();
    if (bandwidthHz < 0) {
      bandwidthHz = Radx::missingMetaFloat;
    }
    var.putVal(bandwidthHz);
  }
      
}

//////////////////////////////////////////////
// add lidar parameters
// throws exception on error

void Cf2RadxFile::_addLidarParameters()
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_addLidarParameters()" << endl;
  }

  NcxxGroup lparamsGroup = _file.addGroup(LIDAR_PARAMETERS);

  // frequency

  _addFrequency(lparamsGroup);

  // lidar constant
      
  {
    NcxxVar var =
      lparamsGroup.addVar(LIDAR_CONSTANT, "",
                          LIDAR_CONSTANT_LONG,
                          ncxxFloat, DB, true);
    // var.putAtt(META_GROUP, LIDAR_PARAMETERS);
    var.putVal((float) _writeVol->getLidarConstant());
  }
  
  {
    NcxxVar var =
      lparamsGroup.addVar(LIDAR_PULSE_ENERGY, "",
                          LIDAR_PULSE_ENERGY_LONG,
                          ncxxFloat, JOULES, true);
    // var.putAtt(META_GROUP, LIDAR_PARAMETERS);
    var.putVal((float) _writeVol->getLidarPulseEnergyJ());
  }
  
  {
    NcxxVar var =
      lparamsGroup.addVar(LIDAR_PEAK_POWER, "",
                          LIDAR_PEAK_POWER_LONG,
                          ncxxFloat, WATTS, true);
    // var.putAtt(META_GROUP, LIDAR_PARAMETERS);
    var.putVal((float) _writeVol->getLidarPeakPowerW());
  }
  
  {
    NcxxVar var =
      lparamsGroup.addVar(LIDAR_APERTURE_DIAMETER, "",
                          LIDAR_APERTURE_DIAMETER_LONG,
                          ncxxFloat, CM, true);
    // var.putAtt(META_GROUP, LIDAR_PARAMETERS);
    var.putVal((float) _writeVol->getLidarApertureDiamCm());
  }
  
  {
    NcxxVar var =
      lparamsGroup.addVar(LIDAR_APERTURE_EFFICIENCY, "", 
                          LIDAR_APERTURE_EFFICIENCY_LONG,
                          ncxxFloat, PERCENT, true);
    // var.putAtt(META_GROUP, LIDAR_PARAMETERS);
    var.putVal((float) _writeVol->getLidarApertureEfficiency());
  }

  {
    NcxxVar var =
      lparamsGroup.addVar(LIDAR_FIELD_OF_VIEW, "",
                          LIDAR_FIELD_OF_VIEW_LONG,
                          ncxxFloat, MRAD, true);
    // var.putAtt(META_GROUP, LIDAR_PARAMETERS);
    var.putVal((float) _writeVol->getLidarFieldOfViewMrad());
  }
  
  {
    NcxxVar var =
      lparamsGroup.addVar(LIDAR_BEAM_DIVERGENCE,
                          "", LIDAR_BEAM_DIVERGENCE_LONG, ncxxFloat, MRAD, true);
    // var.putAtt(META_GROUP, LIDAR_PARAMETERS);
    var.putVal((float) _writeVol->getLidarBeamDivergenceMrad());
  }
  
}

//////////////////////////////////////////////
// add one or more frequencies
// throws exception on error

void Cf2RadxFile::_addFrequency(NcxxGroup &group)
{
  
  const vector<double> &frequency = _writeVol->getFrequencyHz();
  size_t nFreq = frequency.size();
  if (nFreq < 1) {
    return;
  }
  
  if (_verbose) {
    cerr << "Cf2RadxFile::_addFrequencyVariable()" << endl;
  }
  
  NcxxVar var =
    group.addVar(FREQUENCY, "", FREQUENCY_LONG,
                 ncxxFloat, _frequencyDim, HZ, true);

  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nFreq);
  for (size_t ii = 0; ii < nFreq; ii++) {
    fvals[ii] = frequency[ii];
  }
  var.putVal(fvals);

}

//////////////////////////////////////////////
// add georef corrections
// throws exception on error

void Cf2RadxFile::_addGeorefCorrections()
{
  
  if (_verbose) {
    cerr << "Cf2RadxFile::_addGeorefCorrections()" << endl;
  }

  NcxxGroup group = _file.addGroup(GEOREF_CORRECTION);
  const RadxCfactors *cfac = _writeVol->getCfactors();
  
  {
    NcxxVar var =
      group.addVar(AZIMUTH_CORRECTION, "",
                   AZIMUTH_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getAzimuthCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(ELEVATION_CORRECTION, "",
                   ELEVATION_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getElevationCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(RANGE_CORRECTION, "",
                   RANGE_CORRECTION_LONG,
                   ncxxFloat, METERS, true);
    var.putVal((float) cfac->getRangeCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(LONGITUDE_CORRECTION, "",
                   LONGITUDE_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getLongitudeCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(LATITUDE_CORRECTION, "",
                   LATITUDE_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getLatitudeCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(PRESSURE_ALTITUDE_CORRECTION, "",
                   PRESSURE_ALTITUDE_CORRECTION_LONG,
                   ncxxFloat, METERS, true);
    var.putVal((float) cfac->getPressureAltCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(ALTITUDE_CORRECTION, "",
                   ALTITUDE_CORRECTION_LONG,
                   ncxxFloat, METERS, true);
    var.putVal((float) cfac->getAltitudeCorr());
  }

  {
    NcxxVar var =
      group.addVar(EASTWARD_VELOCITY_CORRECTION, "",
                   EASTWARD_VELOCITY_CORRECTION_LONG, 
                   ncxxFloat, METERS_PER_SECOND, true);
    var.putVal((float) cfac->getEwVelCorr());
  }
    
  {
    NcxxVar var =
      group.addVar(NORTHWARD_VELOCITY_CORRECTION, "",
                   NORTHWARD_VELOCITY_CORRECTION_LONG,
                   ncxxFloat, METERS_PER_SECOND, true);
    var.putVal((float) cfac->getNsVelCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(VERTICAL_VELOCITY_CORRECTION, "",
                   VERTICAL_VELOCITY_CORRECTION_LONG,
                   ncxxFloat, METERS_PER_SECOND, true);
    var.putVal((float) cfac->getVertVelCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(HEADING_CORRECTION, "",
                   HEADING_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getHeadingCorr());
  }

  {
    NcxxVar var =
      group.addVar(ROLL_CORRECTION, "",
                   ROLL_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getRollCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(PITCH_CORRECTION, "",
                   PITCH_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getPitchCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(DRIFT_CORRECTION, "",
                   DRIFT_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getDriftCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(ROTATION_CORRECTION, "",
                   ROTATION_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getRotationCorr());
  }
  
  {
    NcxxVar var =
      group.addVar(TILT_CORRECTION, "",
                   TILT_CORRECTION_LONG,
                   ncxxFloat, DEGREES, true);
    var.putVal((float) cfac->getTiltCorr());
  }
  
}

//////////////////////////////////////////////
// add location variables
// throws exception on error

void Cf2RadxFile::_addLocation()
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_addLocation()" << endl;
  }

  // set from volume

  double latitude = _writeVol->getLatitudeDeg();
  double longitude = _writeVol->getLongitudeDeg();
  double altitudeM = Radx::missingMetaDouble;
  if (_writeVol->getAltitudeKm() != Radx::missingMetaDouble) {
    altitudeM = _writeVol->getAltitudeKm() * 1000.0;
  }
  double altitudeAglM = Radx::missingMetaDouble;
  if (_writeVol->getSensorHtAglM() != Radx::missingMetaDouble) {
    altitudeAglM = _writeVol->getSensorHtAglM();
  }

  // if georefs are active, use starting ray

  if (_georefsActive && _writeVol->getNRays() > 0) {
    const RadxRay *ray0 = _writeVol->getRays()[0];
    const RadxGeoref *georef = ray0->getGeoreference();
    latitude = georef->getLatitude();
    longitude = georef->getLongitude();
    if (georef->getAltitudeKmMsl() > 0) {
      altitudeM = georef->getAltitudeKmMsl() * 1000.0;
    }
    if (georef->getAltitudeKmAgl() > 0) {
      altitudeAglM = georef->getAltitudeKmAgl() * 1000.0;
    }
  }

  // latitude
  
  {
    NcxxVar var = 
      _file.addVar(LATITUDE, "", LATITUDE_LONG,
                   ncxxDouble, DEGREES_NORTH, true);
    var.putVal(latitude);
  }
  
  // longitude
  
  {
    NcxxVar var = 
      _file.addVar(LONGITUDE, "", LONGITUDE_LONG,
                   ncxxDouble, DEGREES_EAST, true);
    var.putVal(longitude);
  }
  
  // altitude MSL
  
  {
    NcxxVar var = 
      _file.addVar(ALTITUDE, "", ALTITUDE_LONG,
                   ncxxDouble, METERS, true);
    var.putAtt(POSITIVE, UP);
    var.putVal(altitudeM);
  }
  
  // altitude AGL
  
  {
    NcxxVar var = 
      _file.addVar(ALTITUDE_AGL, "", ALTITUDE_AGL_LONG,
                   ncxxDouble, METERS, true);
    var.putAtt(POSITIVE, UP);
    var.putVal(altitudeAglM);
  }
  
}

//////////////////////////////////////////////
// add variables and attributes for projection
// throws exception on error

void Cf2RadxFile::_addProjection()
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_addProjection()" << endl;
  }

  // projection variable
    
  NcxxVar var = _file.addVar(GRID_MAPPING, ncxxInt);
  
  var.putAtt(GRID_MAPPING_NAME, "azimuthal_equidistant");
  // var.putAtt(GRID_MAPPING_NAME, "radar_lidar_radial_scan");
  
  if (_writeVol->getPlatformType() == Radx::PLATFORM_TYPE_FIXED) {
    var.addScalarAttr(LONGITUDE_OF_PROJECTION_ORIGIN,
                      _writeVol->getLongitudeDeg());
    var.addScalarAttr(LATITUDE_OF_PROJECTION_ORIGIN,
                      _writeVol->getLatitudeDeg());
    var.addScalarAttr(ALTITUDE_OF_PROJECTION_ORIGIN,
                      _writeVol->getAltitudeKm() * 1000.0);
    var.addScalarAttr(FALSE_NORTHING, 0.0);
    var.addScalarAttr(FALSE_EASTING, 0.0);
  }

}
////////////////////////////////////////////////
// add sweep groups
// throws exception on error

void Cf2RadxFile::_addSweeps()
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_addSweeps()" << endl;
  }
  
  _sweepGroupNames.clear();
  _sweepGroups.clear();

  // loop through the sweeps
  
  const vector<RadxSweep *> &sweeps = _writeVol->getSweeps();
  int nSweeps = (int) sweeps.size();

  for (int isweep = 0; isweep < nSweeps; isweep++) {

    // create a volume with just this sweep
    
    RadxSweep *sweep = sweeps[isweep];
    RadxVol sweepVol(*_writeVol, sweep->getSweepNumber());

    // convert fields from rays to 2-D arrays
    sweepVol.loadFieldsFromRays(true);
    
    // create name
  
    char name[128];
    sprintf(name, "sweep_%.4d", isweep + 1);
    _sweepGroupNames.push_back(name);

    if (_debug) {
      cerr << "adding sweep: " << name << endl;
    }

    // add group
  
    NcxxGroup sweepGroup;
    try {
      sweepGroup = _file.addGroup(name);
      _sweepGroups.push_back(sweepGroup);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Cf2RadxFile::_addSweeps");
      _addErrStr("  Cannot add sweep group: ", name);
      _addErrStr("  Exception: ", e.what());
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }

    // add dimensions
    
    size_t nRays = sweepVol.getNRays();
    NcxxDim timeDim = sweepGroup.addDim(TIME, nRays);
    
    size_t nGates = sweepVol.getMaxNGates();
    NcxxDim rangeDim = sweepGroup.addDim(RANGE, nGates);
    
    // add sweep group attributes

    try {
      _addSweepAttributes(sweeps[isweep], sweepGroup);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Cf2RadxFile::_addSweeps");
      _addErrStr("  Adding sweep group attributes");
      _addErrStr("  Exception: ", e.what());
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }
    
    // add sweep group variables

    try {
      _addSweepVariables(sweeps[isweep], sweepVol, 
                         sweepGroup, timeDim, rangeDim);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Cf2RadxFile::_addSweeps");
      _addErrStr("  Adding sweep group variables");
      _addErrStr("  Exception: ", e.what());
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }

    // add monitoring

    try {
      _addSweepMon(sweepVol, sweepGroup, timeDim);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Cf2RadxFile::_addSweeps");
      _addErrStr("  Adding sweep group monitoring");
      _addErrStr("  Exception: ", e.what());
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }

    // add georefs as needed

    if (_georefsActive) {
      try {
        _addSweepGeorefs(sweepVol, sweepGroup, timeDim);
      } catch (NcxxException& e) {
        _addErrStr("ERROR - Cf2RadxFile::_addSweeps");
        _addErrStr("  Adding sweep group georefs");
        _addErrStr("  Exception: ", e.what());
        throw(NcxxException(getErrStr(), __FILE__, __LINE__));
      }
    }

    // add sweep group fields

    try {
      _addSweepFields(sweeps[isweep], sweepVol, 
                      sweepGroup, timeDim, rangeDim);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Cf2RadxFile::_addSweeps");
      _addErrStr("  Adding sweep group fields");
      _addErrStr("  Exception: ", e.what());
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }

  } // isweep
  
  // save sweep names at root level
  
  try {
    NcxxVar var = _file.addVar(SWEEP_GROUP_NAME, "", SWEEP_GROUP_NAME_LONG, 
                               ncxxString, _sweepDim, "", true);
    const char **sweepNames = new const char*[nSweeps];
    for (int isweep = 0; isweep < nSweeps; isweep++) {
      sweepNames[isweep] = _sweepGroupNames[isweep].c_str();
    }
    var.putVal(sweepNames);
    delete[] sweepNames;
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Cf2RadxFile::_addSweeps");
    _addErrStr("  Cannot add root group var: ", SWEEP_GROUP_NAME);
    _addErrStr("  Exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  // save sweep fixed angles at root level
  
  try {
    NcxxVar var = _file.addVar(SWEEP_FIXED_ANGLE, "", SWEEP_FIXED_ANGLE_LONG, 
                               ncxxFloat, _sweepDim, DEGREES, true);
    RadxArray<float> fvals_;
    float *fvals = fvals_.alloc(nSweeps);
    for (int isweep = 0; isweep < nSweeps; isweep++) {
      fvals[isweep] = sweeps[isweep]->getFixedAngleDeg();
    }
    var.putVal(fvals);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Cf2RadxFile::_addSweeps");
    _addErrStr("  Cannot add root group var: ", SWEEP_FIXED_ANGLE);
    _addErrStr("  Exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

}

//////////////////////////////////////////////
// add sweep group attributes
// throws exception on error

void Cf2RadxFile::_addSweepAttributes(const RadxSweep *sweep,
                                      NcxxGroup &sweepGroup)

{

  // for now, using variables intead of attributes for
  // these values
  return;

  sweepGroup.putAtt(SWEEP_NUMBER, ncxxInt, sweep->getSweepNumber());

  float fixedAngle = sweep->getFixedAngleDeg();
  sweepGroup.putAtt(SWEEP_FIXED_ANGLE, ncxxFloat, fixedAngle);
  
  string sweepModeStr = Radx::sweepModeToStr(sweep->getSweepMode());
  sweepGroup.putAtt(SWEEP_MODE, sweepModeStr);
  
  string polModeStr =
    Radx::polarizationModeToStr(sweep->getPolarizationMode());
  sweepGroup.putAtt(POLARIZATION_MODE, polModeStr);

  string prtModeStr = Radx::prtModeToStr(sweep->getPrtMode());
  sweepGroup.putAtt(PRT_MODE, prtModeStr);
  
  string followModeStr = Radx::followModeToStr(sweep->getFollowMode());
  sweepGroup.putAtt(FOLLOW_MODE, followModeStr);
  
  float targetScanRate = sweep->getTargetScanRateDegPerSec();
  sweepGroup.putAtt(TARGET_SCAN_RATE, ncxxFloat, targetScanRate);
  
  if (sweep->getRaysAreIndexed()) {
    sweepGroup.putAtt(RAYS_ARE_INDEXED, "true");
    float angleRes = sweep->getAngleResDeg();
    sweepGroup.putAtt(RAY_ANGLE_RESOLUTION, ncxxFloat, angleRes);
  } else {
    sweepGroup.putAtt(RAYS_ARE_INDEXED, "false");
    float angleRes = -9999.0;
    sweepGroup.putAtt(RAY_ANGLE_RESOLUTION, ncxxFloat, angleRes);
  }
  
  float IFreq = sweep->getIntermedFreqHz();
  sweepGroup.putAtt(INTERMED_FREQ_HZ, ncxxFloat, IFreq);

}

//////////////////////////////////////////////
// add sweep group variables
// throws exception on error

void Cf2RadxFile::_addSweepVariables(const RadxSweep *sweep,
                                     RadxVol &sweepVol,
                                     NcxxGroup &sweepGroup,
                                     NcxxDim &timeDim,
                                     NcxxDim &rangeDim)
  
{
  
  // first the scalars

  _addSweepScalars(sweep, sweepGroup);

  // get rays
  
  const vector<RadxRay *> &rays = sweepVol.getRays();
  size_t nRays = sweepVol.getNRays(); 
  
  // make the geometry constant
  // set the number of gates to be constant

  sweepVol.remapToPredomGeom();
  sweepVol.setNGatesConstant();
  double startRangeKm = sweepVol.getStartRangeKm();
  double gateSpacingKm = sweepVol.getGateSpacingKm();
  
  // alloc utility arrays

  RadxArray<double> dvals_;
  double *dvals = dvals_.alloc(nRays);

  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nRays);

  RadxArray<int> ivals_;
  int *ivals = ivals_.alloc(nRays);

  RadxArray<signed char> bvals_;
  signed char *bvals = bvals_.alloc(nRays);

  // Add time coordinate variable.
  // Note that in CF, coordinate variables have the same name
  // as their dimension

  NcxxVar timeVar = sweepGroup.addVar(TIME, ncxxDouble, timeDim);
  timeVar.putAtt(STANDARD_NAME, TIME);
  timeVar.putAtt(LONG_NAME, "time in seconds since volume start");
  timeVar.putAtt(CALENDAR, GREGORIAN);
  
  char timeUnitsStr[256];
  RadxTime stime(_writeVol->getStartTimeSecs());
  sprintf(timeUnitsStr, "seconds since %.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
          stime.getYear(), stime.getMonth(), stime.getDay(),
          stime.getHour(), stime.getMin(), stime.getSec());
  timeVar.putAtt(UNITS, timeUnitsStr);
  timeVar.putAtt(COMMENT, "times are relative to the volume start_time");

  for (size_t iray = 0; iray < nRays; iray++) {
    RadxTime rayTime = rays[iray]->getRadxTime();
    double dsecs = rayTime - stime;
    dvals[iray] = dsecs;
  }
  timeVar.putVal(dvals);

  // start range

  NcxxVar startRangeVar =
    sweepGroup.addVar(START_RANGE, "", "range_to_center_of_first_gate",
                      ncxxFloat, METERS, true);
  startRangeVar.putAtt(UNITS, METERS);
  startRangeVar.putVal((float) (startRangeKm * 1000.0));
  
  // gate spacing

  NcxxVar gateSpacingVar =
    sweepGroup.addVar(RAY_GATE_SPACING, "", "spacing_between_gate_centers",
                      ncxxFloat, METERS, true);
  gateSpacingVar.putAtt(UNITS, METERS);
  gateSpacingVar.putVal((float) (gateSpacingKm * 1000.0));

  // add range coordinate variable
  
  NcxxVar rangeVar = sweepGroup.addVar(RANGE, ncxxFloat, rangeDim);
  rangeVar.putAtt(LONG_NAME, RANGE_LONG);
  rangeVar.putAtt(LONG_NAME, "Range from instrument to center of gate");
  rangeVar.putAtt(UNITS, METERS);
  rangeVar.putAtt(SPACING_IS_CONSTANT, "true");
  rangeVar.addScalarAttr(METERS_TO_CENTER_OF_FIRST_GATE,
                         (float) startRangeKm * 1000.0);
  rangeVar.addScalarAttr(METERS_BETWEEN_GATES, 
                         (float) gateSpacingKm * 1000.0);
  double rangeKm = startRangeKm;
  RadxArray<float> rangeM_;
  float *rangeM = rangeM_.alloc(rangeDim.getSize());
  for (size_t igate = 0; igate < rangeDim.getSize();
       igate++, rangeKm += gateSpacingKm) {
    rangeM[igate] = rangeKm * 1000.0;
  }
  rangeVar.putVal(rangeM);
  
  // elevation
  
  NcxxVar elVar =
    sweepGroup.addVar(ELEVATION, "", ELEVATION_LONG,
                      ncxxFloat, timeDim, DEGREES, true);
  for (size_t iray = 0; iray < nRays; iray++) {
    fvals[iray] =  rays[iray]->getElevationDeg();
  }
  // elVar.putAtt(POSITIVE, UP);
  elVar.putVal(fvals);

  // azimuth 

  NcxxVar azVar = 
    sweepGroup.addVar(AZIMUTH, "", AZIMUTH_LONG,
                      ncxxFloat, timeDim, DEGREES, true);
  for (size_t iray = 0; iray < nRays; iray++) {
    fvals[iray] = rays[iray]->getAzimuthDeg();
  }
  azVar.putVal(fvals);
  
  // pulse width

  NcxxVar pulseWidthVar = 
    sweepGroup.addVar(PULSE_WIDTH, "", PULSE_WIDTH_LONG,
                      ncxxFloat, timeDim, SECONDS, true);
  // pulseWidthVar.putAtt(META_GROUP, INSTRUMENT_PARAMETERS);
  for (size_t iray = 0; iray < nRays; iray++) {
    if (rays[iray]->getPulseWidthUsec() > 0) {
      fvals[iray] = rays[iray]->getPulseWidthUsec() * 1.0e-6;
    } else {
      fvals[iray] = Radx::missingFl32;
    }
  }
  pulseWidthVar.putVal(fvals);
  
  // prt
  
  NcxxVar prtVar = 
    sweepGroup.addVar(PRT, "", PRT_LONG,
                      ncxxFloat, timeDim, SECONDS, true);
  // prtVar.putAtt(META_GROUP, INSTRUMENT_PARAMETERS);
  for (size_t iray = 0; iray < nRays; iray++) {
    fvals[iray] = rays[iray]->getPrtSec();
  }
  prtVar.putVal(fvals);
  
  // prt ratio
  
  NcxxVar prtRatioVar = 
    sweepGroup.addVar(PRT_RATIO, "", PRT_RATIO_LONG,
                      ncxxFloat, timeDim, SECONDS, true);
  // prtRatioVar.putAtt(META_GROUP, INSTRUMENT_PARAMETERS);
  for (size_t iray = 0; iray < nRays; iray++) {
    fvals[iray] = rays[iray]->getPrtRatio();
  }
  prtRatioVar.putVal(fvals);
  
  // nyquist
  
  NcxxVar nyquistVar = 
    sweepGroup.addVar(NYQUIST_VELOCITY, "", NYQUIST_VELOCITY_LONG, 
                      ncxxFloat, timeDim, METERS_PER_SECOND, true);
  // nyquistVar.putAtt(META_GROUP, INSTRUMENT_PARAMETERS);
  for (size_t iray = 0; iray < nRays; iray++) {
    fvals[iray] = rays[iray]->getNyquistMps();
  }
  nyquistVar.putVal(fvals);
  
  // unambigRange
  
  NcxxVar unambigRangeVar = 
    sweepGroup.addVar(UNAMBIGUOUS_RANGE, "", UNAMBIGUOUS_RANGE_LONG,
                      ncxxFloat, timeDim, METERS, true);
  // unambigRangeVar.putAtt(META_GROUP, INSTRUMENT_PARAMETERS);
  for (size_t iray = 0; iray < nRays; iray++) {
    if (rays[iray]->getUnambigRangeKm() > 0) {
      fvals[iray] = rays[iray]->getUnambigRangeKm() * 1000.0;
    } else {
      fvals[iray] = Radx::missingFl32;
    }
  }
  unambigRangeVar.putVal(fvals);
  
  // antenna transition
  
  NcxxVar antennaTransitionVar = 
    sweepGroup.addVar(ANTENNA_TRANSITION, "", ANTENNA_TRANSITION_LONG,
                      ncxxByte, timeDim, "", true);
  antennaTransitionVar.putAtt(COMMENT,
                              "1 if antenna is in transition, 0 otherwise");
  for (size_t iray = 0; iray < nRays; iray++) {
    bvals[iray] = rays[iray]->getAntennaTransition();
  }
  antennaTransitionVar.putVal(bvals);
  
  // georefs applied?
  
  if (_georefsActive) {
    NcxxVar georefsAppliedVar = 
      sweepGroup.addVar(GEOREFS_APPLIED, "", "georefs_have_been_applied_to_ray",
                        ncxxByte, timeDim, "", true);
    georefsAppliedVar.putAtt
      (COMMENT, "1 if georefs have been applied, 0 otherwise");
    for (size_t iray = 0; iray < nRays; iray++) {
      if (_georefsApplied) {
        bvals[iray] = 1;
      } else {
        bvals[iray] = rays[iray]->getGeorefApplied();
      }
    }
    georefsAppliedVar.putVal(bvals);
  }
  
  // number of samples in dwell
  
  NcxxVar nSamplesVar = 
    sweepGroup.addVar(N_SAMPLES, "", N_SAMPLES_LONG,
                      ncxxInt, timeDim, "", true);
  // nSamplesVar.putAtt(META_GROUP, INSTRUMENT_PARAMETERS);
  for (size_t iray = 0; iray < nRays; iray++) {
    ivals[iray] = rays[iray]->getNSamples();
  }
  nSamplesVar.putVal(ivals);
  
  // calibration number
  
  if (_writeVol->getRcalibs().size() > 0) {
    NcxxVar calIndexVar = 
      sweepGroup.addVar(R_CALIB_INDEX, "", R_CALIB_INDEX_LONG,
                        ncxxInt, timeDim, "", true);
    // // calIndexVar.putAtt(META_GROUP, RADAR_CALIBRATION);
    calIndexVar.putAtt
      (COMMENT, "This is the index for the calibration which applies to this ray");
    for (size_t iray = 0; iray < nRays; iray++) {
      ivals[iray] = rays[iray]->getCalibIndex();
    }
    calIndexVar.putVal(ivals);
  }
  
}

//////////////////////////////////////////////
// add sweep group scalars at the top level
// throws exception on error

void Cf2RadxFile::_addSweepScalars(const RadxSweep *sweep,
                                   NcxxGroup &sweepGroup)
  
{
  
  // sweep number
  {
    NcxxVar var = 
      sweepGroup.addVar(SWEEP_NUMBER, "", SWEEP_NUMBER_LONG,
                        ncxxInt, "", true);
    var.putVal((int) sweep->getSweepNumber());
  }

  // fixed angle
  {
    NcxxVar var = 
      sweepGroup.addVar(SWEEP_FIXED_ANGLE, "", FIXED_ANGLE_LONG,
                        ncxxFloat, "", true);
    var.putAtt(UNITS, DEGREES);
    var.putVal((float) sweep->getFixedAngleDeg());
  }

  // sweep mode
  {
    NcxxVar var = 
      sweepGroup.addVar(SWEEP_MODE, "", SWEEP_MODE_LONG,
                        ncxxString, "", true);
    var.putAtt(OPTIONS, Radx::sweepModeOptions());
    string modeStr = Radx::sweepModeToStr(sweep->getSweepMode());
    var.putStringScalar(modeStr);
  }

  // polarization mode
  {
    NcxxVar var = 
      sweepGroup.addVar(POLARIZATION_MODE, "", POLARIZATION_MODE_LONG,
                        ncxxString, "", true);
    var.putAtt(OPTIONS, Radx::polarizationModeOptions());
    string modeStr = Radx::polarizationModeToStr(sweep->getPolarizationMode());
    var.putStringScalar(modeStr);
  }

  // prt mode
  {
    NcxxVar var = 
      sweepGroup.addVar(PRT_MODE, "", PRT_MODE_LONG,
                        ncxxString, "", true);
    var.putAtt(OPTIONS, Radx::prtModeOptions());
    string modeStr = Radx::prtModeToStr(sweep->getPrtMode());
    var.putStringScalar(modeStr);
  }

  // follow mode
  {
    NcxxVar var = 
      sweepGroup.addVar(FOLLOW_MODE, "", FOLLOW_MODE_LONG,
                        ncxxString, "", true);
    var.putAtt(OPTIONS, Radx::followModeOptions());
    string modeStr = Radx::followModeToStr(sweep->getFollowMode());
    var.putStringScalar(modeStr);
  }

  // target scan rate
  if ( sweep->getTargetScanRateDegPerSec() >= 0) {
    NcxxVar var = 
      sweepGroup.addVar(TARGET_SCAN_RATE, "", TARGET_SCAN_RATE_LONG,
                        ncxxFloat, "", true);
    var.putAtt(UNITS, DEGREES_PER_SECOND);
    var.putVal((float) sweep->getTargetScanRateDegPerSec());
  }

  // are rays indexed?
  {
    NcxxVar var = 
      sweepGroup.addVar(RAYS_ARE_INDEXED, "", RAYS_ARE_INDEXED_LONG,
                        ncxxString, "", true);
    var.putAtt(OPTIONS, "true, false");
    if (sweep->getRaysAreIndexed()) {
      var.putStringScalar("true");
    } else {
      var.putStringScalar("false");
    }
  }

  // ray angle resolution
  if (sweep->getRaysAreIndexed()) {
    NcxxVar var = 
      sweepGroup.addVar(RAY_ANGLE_RESOLUTION, "",
                        RAY_ANGLE_RESOLUTION_LONG,
                        ncxxFloat, "", true);
    var.putAtt(UNITS, DEGREES);
    var.putVal((float) sweep->getAngleResDeg());
  }

  // intermediate frequency
  if (sweep->getIntermedFreqHz() > 0) {
    NcxxVar var = 
      sweepGroup.addVar(INTERMED_FREQ_HZ, "", INTERMED_FREQ_HZ_LONG,
                        ncxxFloat, "", true);
    var.putAtt(UNITS, HZ);
    var.putVal((float) sweep->getIntermedFreqHz());
  }

}

//////////////////////////////////////////////
// add sweep group georefs
// throws exception on error

void Cf2RadxFile::_addSweepGeorefs(RadxVol &sweepVol,
                                   NcxxGroup &sweepGroup,
                                   NcxxDim &timeDim)
  
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_addSweepGeorefs()" << endl;
  }
  
  // get rays
  
  const vector<RadxRay *> &rays = sweepVol.getRays();
  size_t nRays = sweepVol.getNRays(); 
  
  // alloc utility arrays

  RadxArray<double> dvals_;
  double *dvals = dvals_.alloc(nRays);

  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nRays);

  RadxArray<Radx::si64> lvals_;
  Radx::si64 *lvals = lvals_.alloc(nRays);

  // create georeference subgroup

  NcxxGroup georefGroup = sweepGroup.addGroup(GEOREFERENCE);

  try {
    
    // always add geo time
    
    NcxxVar georefTimeVar = 
      georefGroup.addVar(GEOREF_TIME, "", GEOREF_TIME_LONG,
                         ncxxDouble, timeDim, SECONDS, true);
    RadxTime volStartSecs(_writeVol->getStartTimeSecs());
    for (size_t iray = 0; iray < rays.size(); iray++) {
      const RadxGeoref *geo = rays[iray]->getGeoreference();
      if (geo) {
        RadxTime geoTime(geo->getTimeSecs(), geo->getNanoSecs() * 1.0e-9);
        double dsecs = geoTime - volStartSecs;
        dvals[iray] = dsecs;
      } else {
        dvals[iray] = Radx::missingMetaDouble;
      }
    }
    georefTimeVar.putVal(dvals);
    
    // we always add the postion variables
    
    // latitude

    NcxxVar latitudeVar = 
      georefGroup.addVar(LATITUDE, "", LATITUDE_LONG,
                         ncxxDouble, timeDim, DEGREES_NORTH, true);
    for (size_t iray = 0; iray < rays.size(); iray++) {
      const RadxGeoref *geo = rays[iray]->getGeoreference();
      if (geo) {
        dvals[iray] = _checkMissingDouble(geo->getLatitude());
      } else {
        dvals[iray] = Radx::missingMetaDouble;
      }
    }
    latitudeVar.putVal(dvals);

    // longitude

    NcxxVar longitudeVar = 
      georefGroup.addVar(LONGITUDE, "", LONGITUDE_LONG,
                         ncxxDouble, timeDim, DEGREES_EAST, true);
    for (size_t iray = 0; iray < rays.size(); iray++) {
      const RadxGeoref *geo = rays[iray]->getGeoreference();
      if (geo) {
        dvals[iray] = _checkMissingDouble(geo->getLongitude());
      } else {
        dvals[iray] = Radx::missingMetaDouble;
      }
    }
    longitudeVar.putVal(dvals);

    // altitude msl
    
    NcxxVar altitudeVar = 
      georefGroup.addVar(ALTITUDE, "", ALTITUDE_LONG,
                         ncxxDouble, timeDim, METERS, true);
    altitudeVar.putAtt(POSITIVE, UP);
    for (size_t iray = 0; iray < rays.size(); iray++) {
      const RadxGeoref *geo = rays[iray]->getGeoreference();
      if (geo) {
        double altKm = geo->getAltitudeKmMsl();
        if (altKm > -9990) {
          dvals[iray] = altKm * 1000.0; // meters
        } else {
          dvals[iray] = Radx::missingMetaDouble;
        }
      } else {
        dvals[iray] = Radx::missingMetaDouble;
      }
    }
    altitudeVar.putVal(dvals);

    // altitude agl

    NcxxVar altitudeAglVar = 
      georefGroup.addVar(ALTITUDE_AGL, "", ALTITUDE_AGL_LONG,
                         ncxxDouble, timeDim, METERS, true);
    altitudeAglVar.putAtt(POSITIVE, UP);
    for (size_t iray = 0; iray < rays.size(); iray++) {
      const RadxGeoref *geo = rays[iray]->getGeoreference();
      if (geo) {
        double altKm = geo->getAltitudeKmAgl();
        if (altKm > -9990) {
          dvals[iray] = altKm * 1000.0; // meters
        } else {
          dvals[iray] = Radx::missingMetaDouble;
        }
      } else {
        dvals[iray] = Radx::missingMetaDouble;
      }
    }
    altitudeAglVar.putVal(dvals);

    // conditionally add the georeference variables

    // unit num

    if (_geoCount.getUnitNum() > 0) {
      NcxxVar var = 
        georefGroup.addVar(GEOREF_UNIT_NUM, "", GEOREF_UNIT_NUM_LONG,
                           ncxxInt64, timeDim, "", true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          lvals[iray] = geo->getUnitNum();
        } else {
          lvals[iray] = 0;
        }
      }
      var.putVal(lvals);
    }

    // unit id

    if (_geoCount.getUnitId() > 0) {
      NcxxVar var = 
        georefGroup.addVar(GEOREF_UNIT_ID, "", GEOREF_UNIT_ID_LONG,
                           ncxxInt64, timeDim, "", true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          lvals[iray] = geo->getUnitId();
        } else {
          lvals[iray] = 0;
        }
      }
      var.putVal(lvals);
    }

    // heading

    if (_geoCount.getHeading() > 0) {
      NcxxVar var = 
        georefGroup.addVar(HEADING, "", HEADING_LONG,
                           ncxxFloat, timeDim, DEGREES, true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getHeading());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // track

    if (_geoCount.getTrack() > 0) {
      NcxxVar var = 
        georefGroup.addVar(TRACK, "", TRACK_LONG, 
                           ncxxFloat, timeDim, DEGREES, true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getTrack());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // roll

    if (_geoCount.getRoll() > 0) {
      NcxxVar var = 
        georefGroup.addVar(ROLL, "", ROLL_LONG, 
                           ncxxFloat, timeDim, DEGREES, true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getRoll());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // pitch

    if (_geoCount.getPitch() > 0) {
      NcxxVar var = 
        georefGroup.addVar(PITCH, "", PITCH_LONG, 
                           ncxxFloat, timeDim, DEGREES, true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getPitch());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // drift

    if (_geoCount.getDrift() > 0) {
      NcxxVar var = 
        georefGroup.addVar(DRIFT, "", DRIFT_LONG, ncxxFloat,
                           timeDim, DEGREES, true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getDrift());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // rotation

    if (_geoCount.getRotation() > 0) {
      NcxxVar var = 
        georefGroup.addVar(ROTATION, "", ROTATION_LONG,
                           ncxxFloat, timeDim, DEGREES, true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getRotation());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }
  
    // tilt

    if (_geoCount.getTilt() > 0) {
      NcxxVar var = 
        georefGroup.addVar(TILT, "", TILT_LONG,
                           ncxxFloat, timeDim, DEGREES, true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getTilt());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // EW velocity

    if (_geoCount.getEwVelocity() > 0) {
      NcxxVar var = 
        georefGroup.addVar(EASTWARD_VELOCITY, "", EASTWARD_VELOCITY_LONG,
                           ncxxFloat, timeDim, METERS_PER_SECOND, true);
      // var.putAtt(META_GROUP, PLATFORM_VELOCITY);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getEwVelocity());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }
  
    // NS velocity

    if (_geoCount.getNsVelocity() > 0) {
      NcxxVar var = 
        georefGroup.addVar(NORTHWARD_VELOCITY, "", NORTHWARD_VELOCITY_LONG,
                           ncxxFloat, timeDim, METERS_PER_SECOND, true);
      // var.putAtt(META_GROUP, PLATFORM_VELOCITY);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getNsVelocity());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // vert velocity

    if (_geoCount.getVertVelocity() > 0) {
      NcxxVar var = 
        georefGroup.addVar(VERTICAL_VELOCITY, "", VERTICAL_VELOCITY_LONG,
                           ncxxFloat, timeDim, METERS_PER_SECOND, true);
      // var.putAtt(META_GROUP, PLATFORM_VELOCITY);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getVertVelocity());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // EW wind

    if (_geoCount.getEwWind() > 0) {
      NcxxVar var = 
        georefGroup.addVar(EASTWARD_WIND, "", EASTWARD_WIND_LONG,
                           ncxxFloat, timeDim, METERS_PER_SECOND, true);
      // var.putAtt(META_GROUP, PLATFORM_VELOCITY);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getEwWind());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // NS wind

    if (_geoCount.getNsWind() > 0) {
      NcxxVar var = 
        georefGroup.addVar(NORTHWARD_WIND, "", NORTHWARD_WIND_LONG,
                           ncxxFloat, timeDim, METERS_PER_SECOND, true);
      // var.putAtt(META_GROUP, PLATFORM_VELOCITY);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getNsWind());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // vert wind

    if (_geoCount.getVertWind() > 0) {
      NcxxVar var = 
        georefGroup.addVar(VERTICAL_WIND, "", VERTICAL_WIND_LONG,
                           ncxxFloat, timeDim, METERS_PER_SECOND, true);
      // var.putAtt(META_GROUP, PLATFORM_VELOCITY);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getVertWind());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // heading rate

    if (_geoCount.getHeadingRate() > 0) {
      NcxxVar var = 
        georefGroup.addVar(HEADING_CHANGE_RATE, "", HEADING_CHANGE_RATE_LONG,
                           ncxxFloat, timeDim, DEGREES_PER_SECOND, true);
      // var.putAtt(META_GROUP, PLATFORM_VELOCITY);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getHeadingRate());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // pitch rate

    if (_geoCount.getPitchRate() > 0) {
      NcxxVar var = 
        georefGroup.addVar(PITCH_CHANGE_RATE, "", PITCH_CHANGE_RATE_LONG,
                           ncxxFloat, timeDim, DEGREES_PER_SECOND, true);
      // var.putAtt(META_GROUP, PLATFORM_VELOCITY);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getPitchRate());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // roll rate

    if (_geoCount.getRollRate() > 0) {
      NcxxVar var = 
        georefGroup.addVar(ROLL_CHANGE_RATE, "", ROLL_CHANGE_RATE_LONG,
                           ncxxFloat, timeDim, DEGREES_PER_SECOND, true);
      // var.putAtt(META_GROUP, PLATFORM_VELOCITY);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getRollRate());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }

    // drive angle 1 - HCR

    if (_geoCount.getDriveAngle1() > 0) {
      NcxxVar var = 
        georefGroup.addVar(DRIVE_ANGLE_1, "", DRIVE_ANGLE_1_LONG,
                           ncxxFloat, timeDim, DEGREES, true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getDriveAngle1());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }
  
    // drive angle 2 - HCR

    if (_geoCount.getDriveAngle2() > 0) {
      NcxxVar var = 
        georefGroup.addVar(DRIVE_ANGLE_2, "", DRIVE_ANGLE_2_LONG,
                           ncxxFloat, timeDim, DEGREES, true);
      for (size_t iray = 0; iray < rays.size(); iray++) {
        const RadxGeoref *geo = rays[iray]->getGeoreference();
        if (geo) {
          fvals[iray] = _checkMissingFloat(geo->getDriveAngle2());
        } else {
          fvals[iray] = Radx::missingMetaFloat;
        }
      }
      var.putVal(fvals);
    }
  
  } catch (NcxxException& e) {
    
    _addErrStr("ERROR - Cf2RadxFile::_addSweepGeorefs");
    _addErrStr("  Exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    
  }

}

//////////////////////////////////////////////
// add sweep group monitoring
// throws exception on error

void Cf2RadxFile::_addSweepMon(RadxVol &sweepVol,
                               NcxxGroup &sweepGroup,
                               NcxxDim &timeDim)
  
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_addSweepMonitoring()" << endl;
  }
  
  // get rays
  
  const vector<RadxRay *> &rays = sweepVol.getRays();
  size_t nRays = sweepVol.getNRays(); 
  
  // alloc utility arrays

  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nRays);

  // create monitoring subgroup

  NcxxGroup monGroup = sweepGroup.addGroup(MONITORING);

  // transmit power in H and V
  
  NcxxVar xmitPowerHVar =
    monGroup.addVar(RADAR_MEASURED_TRANSMIT_POWER_H, "", 
                    RADAR_MEASURED_TRANSMIT_POWER_H_LONG,
                    ncxxFloat, timeDim, DBM, true);
  // xmitPowerHVar.putAtt(META_GROUP, RADAR_PARAMETERS);
  for (size_t iray = 0; iray < nRays; iray++) {
    fvals[iray] = _checkMissingFloat(rays[iray]->getMeasXmitPowerDbmH());
  }
  xmitPowerHVar.putVal(fvals);
  
  NcxxVar xmitPowerVVar =
    monGroup.addVar(RADAR_MEASURED_TRANSMIT_POWER_V, "", 
                    RADAR_MEASURED_TRANSMIT_POWER_V_LONG,
                    ncxxFloat, timeDim, DBM, true);
  // xmitPowerVVar.putAtt(META_GROUP, RADAR_PARAMETERS);
  for (size_t iray = 0; iray < nRays; iray++) {
    fvals[iray] = _checkMissingFloat(rays[iray]->getMeasXmitPowerDbmV());
  }
  xmitPowerVVar.putVal(fvals);
  for (size_t iray = 0; iray < nRays; iray++) {
    fvals[iray] = _checkMissingFloat(rays[iray]->getMeasXmitPowerDbmV());
  }
  xmitPowerVVar.putVal(fvals);
  
  // scan rate
  
  NcxxVar scanRateVar = 
    monGroup.addVar(SCAN_RATE, "", SCAN_RATE_LONG, 
                    ncxxFloat, timeDim, DEGREES_PER_SECOND, true);
  // scanRateVar.putAtt(META_GROUP, INSTRUMENT_PARAMETERS);
  for (size_t iray = 0; iray < nRays; iray++) {
    fvals[iray] = rays[iray]->getTrueScanRateDegPerSec();
  }
  scanRateVar.putVal(fvals);
  
  // estimated noise per channel
  
  _setEstNoiseAvailFlags();

  if (_estNoiseAvailHc) {
    NcxxVar estNoiseDbmHcVar = 
      monGroup.addVar(RADAR_ESTIMATED_NOISE_DBM_HC, "", 
                      RADAR_ESTIMATED_NOISE_DBM_HC_LONG,
                      ncxxFloat, timeDim, DBM, true);
    // estNoiseDbmHcVar.putAtt(META_GROUP, RADAR_PARAMETERS);
    for (size_t iray = 0; iray < nRays; iray++) {
      fvals[iray] = rays[iray]->getEstimatedNoiseDbmHc();
    }
    estNoiseDbmHcVar.putVal(fvals);
  }
  
  if (_estNoiseAvailVc) {
    NcxxVar estNoiseDbmVcVar = 
      monGroup.addVar(RADAR_ESTIMATED_NOISE_DBM_VC, "", 
                      RADAR_ESTIMATED_NOISE_DBM_VC_LONG,
                      ncxxFloat, timeDim, DBM, true);
    // estNoiseDbmVcVar.putAtt(META_GROUP, RADAR_PARAMETERS);
    for (size_t iray = 0; iray < nRays; iray++) {
      fvals[iray] = rays[iray]->getEstimatedNoiseDbmVc();
    }
    estNoiseDbmVcVar.putVal(fvals);
  }

  if (_estNoiseAvailHx) {
    NcxxVar estNoiseDbmHxVar = 
      monGroup.addVar(RADAR_ESTIMATED_NOISE_DBM_HX, "", 
                      RADAR_ESTIMATED_NOISE_DBM_HX_LONG,
                      ncxxFloat, timeDim, DBM, true);
    // estNoiseDbmHxVar.putAtt(META_GROUP, RADAR_PARAMETERS);
    for (size_t iray = 0; iray < nRays; iray++) {
      fvals[iray] = rays[iray]->getEstimatedNoiseDbmHx();
    }
    estNoiseDbmHxVar.putVal(fvals);
  }
  
  if (_estNoiseAvailVx) {
    NcxxVar estNoiseDbmVxVar = 
      monGroup.addVar(RADAR_ESTIMATED_NOISE_DBM_VX, "", 
                      RADAR_ESTIMATED_NOISE_DBM_VX_LONG,
                      ncxxFloat, timeDim, DBM, true);
    // estNoiseDbmVxVar.putAtt(META_GROUP, RADAR_PARAMETERS);
    for (size_t iray = 0; iray < nRays; iray++) {
      fvals[iray] = rays[iray]->getEstimatedNoiseDbmVx();
    }
    estNoiseDbmVxVar.putVal(fvals);
  }
  
}

//////////////////////////////////////////////
// add sweep group fields
// throws exception on error

void Cf2RadxFile::_addSweepFields(const RadxSweep *sweep,
                                  RadxVol &sweepVol,
                                  NcxxGroup &sweepGroup,
                                  NcxxDim &timeDim,
                                  NcxxDim &rangeDim)
  
{
  
  if (_verbose) {
    cerr << "Cf2RadxFile::_addSweepFields()" << endl;
  }

  // loop through the list of unique fields names in this volume

  vector<string> uniqueFieldNames = sweepVol.getUniqueFieldNameList();

  for (size_t ifield = 0; ifield < uniqueFieldNames.size(); ifield++) {
      
    const string &name = uniqueFieldNames[ifield];
    if (name.size() == 0) {
      // invalid field name
      continue;
    }

    // make copy of the field

    RadxField *copy = sweepVol.copyField(name);
    if (copy == NULL) {
      if (_debug) {
        cerr << "  ... cannot find field: " << name
             << " .... skipping" << endl;
      }
      continue;
    }
    
    // create the variable
    
    NcxxVar var;
    try {
      // add field variable
      var = _createFieldVar(*copy, sweepGroup, timeDim, rangeDim);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Cf2RadxFile::_addSweepFields");
      _addErrStr("  Cannot add field: ", name);
      delete copy;
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }

    // write it out
    
    try {
      _writeFieldVar(var, copy);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Cf2RadxFile::_addSweepFields");
      _addErrStr("  Cannot write field: ", name);
      delete copy;
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }

    // free up
    delete copy;
    if (_debug && (sweep->getSweepNumber() == 1)) {
      cerr << "  ... adding field: " << name << endl;
    }
    
  } // ifield

}

///////////////////////////////////////////////
// create and add a field variable
// Adds to errStr as appropriate

NcxxVar Cf2RadxFile::_createFieldVar(const RadxField &field,
                                     NcxxGroup &sweepGroup,
                                     NcxxDim &timeDim,
                                     NcxxDim &rangeDim)
  
{
  
  if (_verbose) {
    cerr << "Cf2RadxFile::_createFieldVar()" << endl;
    cerr << "  Creating field: " << field.getName() << endl;
  }

  // check that the field name is CF-netCDF compliant - 
  // i.e must start with a letter
  //   if not, add "nc_" to start of name
  // and must only contain letters, digits and underscores

  const string &name = field.getName();
  string fieldName;
  if (isalpha(name[0])) {
    fieldName = name;
  } else {
    fieldName = "nc_";
    fieldName += name;
  }
  for (int ii = 0; ii < (int) fieldName.size(); ii++) {
    if (!isalnum(fieldName[ii]) && fieldName[ii] != '_') {
      fieldName[ii] = '_';
    }
  }
  if (fieldName == "range") {
    // range is reserved
    fieldName += "_";
    cerr << "NOTE - 'range' is a reserved field name" << endl;
    cerr << "  Changing to: '" << fieldName << "'" << endl;
  }
  
  // Add var and the attributes relevant to no data packing

  NcxxType ncxxType = _getNcxxType(field.getDataType());
  NcxxVar var;

  try {
    vector<NcxxDim> dims;
    dims.push_back(timeDim);
    dims.push_back(rangeDim);
    var = sweepGroup.addVar(fieldName, ncxxType, dims);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Cf2RadxFile::_addFieldVar");
    _addErrStr("  Cannot add variable to Ncxx file object");
    _addErrStr("  Input field name: ", name);
    _addErrStr("  Output field name: ", fieldName);
    _addErrStr("  NcxxType: ", Ncxx::ncxxTypeToStr(ncxxType));
    _addErrStr("  Time dim name: ", timeDim.getName());
    _addErrInt("  Time dim size: ", timeDim.getSize());
    _addErrStr("  Range dim name: ", rangeDim.getName());
    _addErrInt("  Range dim size: ", rangeDim.getSize());
    _addErrStr("  Exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  
  try {

    if (field.getLongName().size() > 0) {
      var.putAtt(LONG_NAME, field.getLongName());
    }
    if (field.getStandardName().size() > 0) {
      if (_writeProposedStdNameInNcf) {
        var.putAtt(PROPOSED_STANDARD_NAME, field.getStandardName());
      } else {
        var.putAtt(STANDARD_NAME, field.getStandardName());
      }
    }
    var.putAtt(UNITS, field.getUnits());
    if (field.getLegendXml().size() > 0) {
      var.putAtt(LEGEND_XML, field.getLegendXml());
    }
    if (field.getThresholdingXml().size() > 0) {
      var.putAtt(THRESHOLDING_XML, field.getThresholdingXml());
    }
    if (field.getComment().size() > 0) {
      var.putAtt(COMMENT, field.getComment());
    }
    var.addScalarAttr(SAMPLING_RATIO, (float) field.getSamplingRatio());
    
    if (field.getFieldFolds()) {
      var.putAtt(FIELD_FOLDS, "true");
      var.addScalarAttr(FOLD_LIMIT_LOWER, (float) field.getFoldLimitLower());
      var.addScalarAttr(FOLD_LIMIT_UPPER, (float) field.getFoldLimitUpper());
    }
    if (field.getIsDiscrete()) {
      var.putAtt(IS_DISCRETE, "true");
    }
    
    switch (ncxxType.getTypeClass()) {
      case NcxxType::nc_DOUBLE: {
        var.addScalarAttr(FILL_VALUE, (double) field.getMissingFl64());
        break;
      }
      case NcxxType::nc_FLOAT:
      default: {
        var.addScalarAttr(FILL_VALUE, (float) field.getMissingFl32());
        break;
      }
      case NcxxType::nc_INT: {
        var.addScalarAttr(FILL_VALUE, (int) field.getMissingSi32());
        var.addScalarAttr(SCALE_FACTOR, (float) field.getScale());
        var.addScalarAttr(ADD_OFFSET, (float) field.getOffset());
        break;
      }
      case NcxxType::nc_SHORT: {
        var.addScalarAttr(FILL_VALUE, (short) field.getMissingSi16());
        var.addScalarAttr(SCALE_FACTOR, (float) field.getScale());
        var.addScalarAttr(ADD_OFFSET, (float) field.getOffset());
        break;
      }
      case NcxxType::nc_BYTE: {
        var.addScalarAttr(FILL_VALUE, (signed char) field.getMissingSi08());
        var.addScalarAttr(SCALE_FACTOR, (float) field.getScale());
        var.addScalarAttr(ADD_OFFSET, (float) field.getOffset());
        break;
      }
    } // switch

    var.putAtt(GRID_MAPPING, GRID_MAPPING);
    var.putAtt(COORDINATES, "time range");
    
    // set compression
    
    _setCompression(var);

  } catch (NcxxException& e) {

    _addErrStr("ERROR - Cf2RadxFile::_addFieldVar");
    _addErrStr("  Adding var to Ncxx file object");
    _addErrStr("  Input field name: ", name);
    _addErrStr("  Output field name: ", fieldName);
    _addErrStr("  Exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));

  }
  
  return var;

}

///////////////////////////////////////////////////////////////////////////
// write a field variable
// Returns 0 on success, -1 on failure

void Cf2RadxFile::_writeFieldVar(NcxxVar &var, RadxField *field)
  
{
  
  if (_verbose) {
    cerr << "Cf2RadxFile::_writeFieldVar()" << endl;
    cerr << "  name: " << var.getName() << endl;
  }

  if (var.isNull()) {
    _addErrStr("ERROR - Cf2RadxFile::_writeFieldVar");
    _addErrStr("  var is NULL");
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  const void *data = field->getData();
  if (data == NULL) {
    _addErrStr("ERROR - Cf2RadxFile::_writeFieldVar");
    _addErrStr("  data is NULL");
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  try {

    switch (var.getType().getTypeClass()) {
      case NcxxType::nc_DOUBLE: {
        var.putVal((double *) data);
        break;
      }
      case NcxxType::nc_INT: {
        var.putVal((int *) data);
        break;
      }
      case NcxxType::nc_SHORT: {
        var.putVal((short *) data);
        break;
      }
      case NcxxType::nc_BYTE: {
        var.putVal((signed char *) data);
        break;
      }
      case NcxxType::nc_FLOAT:
      default: {
        var.putVal((float *) data);
        break;
      }
    } // switch

  } catch (NcxxException& e) {
    
    _addErrStr("ERROR - Cf2RadxFile::_writeFieldVar");
    _addErrStr("  Cannot write var, name: ", var.getName());
    _addErrStr("  Exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    
  }

}

//////////////////////////////////////////////
// add radar calibration in separate group
// throws NcxxException on error

void Cf2RadxFile::_addRadarCalibration()
{
  
  const vector<RadxRcalib *> &calibs = _writeVol->getRcalibs();
  size_t nCalib = calibs.size();
  if (nCalib < 1) {
    return;
  }
  
  if (_verbose) {
    cerr << "Cf2RadxFile::_addCalibVariables()" << endl;
  }

  // add group
  
  NcxxGroup group = _file.addGroup(RADAR_CALIBRATION);
  NcxxDim dim = group.addDim(R_CALIB, _writeVol->getRcalibs().size());

  // time
  
  NcxxVar rCalTimeVar = 
    group.addVar(CALIBRATION_TIME, "", "", 
                 ncxxString, dim, "", true);
  
  vector<string> timeStrings;
  vector<const char*> timeChars;
  for (size_t ii = 0; ii < nCalib; ii++) {
    const RadxRcalib &calib = *calibs[ii];
    RadxTime rtime(calib.getCalibTime());
    timeStrings.push_back(rtime.getW3cStr());
    timeChars.push_back((char *) timeStrings[ii].c_str());
  }
  rCalTimeVar.putVal(&timeChars[0]);

  // cal fields

  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nCalib);
    
  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getPulseWidthUsec() * 1.0e-6;
  }
  _addCalVar(group, dim, fvals,
             PULSE_WIDTH, PULSE_WIDTH_LONG, SECONDS);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getXmitPowerDbmH();
  }
  _addCalVar(group, dim, fvals,
             XMIT_POWER_H, XMIT_POWER_H_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getXmitPowerDbmV();
  }
  _addCalVar(group, dim, fvals,
             XMIT_POWER_V, XMIT_POWER_V_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTwoWayWaveguideLossDbH();
  }
  _addCalVar(group, dim, fvals,
             TWO_WAY_WAVEGUIDE_LOSS_H, TWO_WAY_WAVEGUIDE_LOSS_H_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTwoWayWaveguideLossDbV();
  }
  _addCalVar(group, dim, fvals,
             TWO_WAY_WAVEGUIDE_LOSS_V, TWO_WAY_WAVEGUIDE_LOSS_V_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTwoWayRadomeLossDbH();
  }
  _addCalVar(group, dim, fvals,
             TWO_WAY_RADOME_LOSS_H, TWO_WAY_RADOME_LOSS_H_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTwoWayRadomeLossDbV();
  }
  _addCalVar(group, dim, fvals,
             TWO_WAY_RADOME_LOSS_V, TWO_WAY_RADOME_LOSS_V_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverMismatchLossDb();
  }
  _addCalVar(group, dim, fvals,
             RECEIVER_MISMATCH_LOSS, RECEIVER_MISMATCH_LOSS_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getRadarConstantH();
  }
  _addCalVar(group, dim, fvals,
             RADAR_CONSTANT_H, RADAR_CONSTANT_H_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getRadarConstantV();
  }
  _addCalVar(group, dim, fvals,
             RADAR_CONSTANT_V, RADAR_CONSTANT_V_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getAntennaGainDbH();
  }
  _addCalVar(group, dim, fvals,
             ANTENNA_GAIN_H, ANTENNA_GAIN_H_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getAntennaGainDbV();
  }
  _addCalVar(group, dim, fvals,
             ANTENNA_GAIN_V, ANTENNA_GAIN_V_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseDbmHc();
  }
  _addCalVar(group, dim, fvals,
             NOISE_HC, NOISE_HC_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseDbmVc();
  }
  _addCalVar(group, dim, fvals,
             NOISE_VC, NOISE_VC_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseDbmHx();
  }
  _addCalVar(group, dim, fvals,
             NOISE_HX, NOISE_HX_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseDbmVx();
  }
  _addCalVar(group, dim, fvals,
             NOISE_VX, NOISE_VX_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverGainDbHc();
  }
  _addCalVar(group, dim, fvals,
             RECEIVER_GAIN_HC, RECEIVER_GAIN_HC_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverGainDbVc();
  }
  _addCalVar(group, dim, fvals,
             RECEIVER_GAIN_VC, RECEIVER_GAIN_VC_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverGainDbHx();
  }
  _addCalVar(group, dim, fvals,
             RECEIVER_GAIN_HX, RECEIVER_GAIN_HX_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverGainDbVx();
  }
  _addCalVar(group, dim, fvals,
             RECEIVER_GAIN_VX, RECEIVER_GAIN_VX_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getBaseDbz1kmHc();
  }
  _addCalVar(group, dim, fvals,
             BASE_DBZ_1KM_HC, BASE_DBZ_1KM_HC_LONG, DBZ);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getBaseDbz1kmVc();
  }
  _addCalVar(group, dim, fvals,
             BASE_DBZ_1KM_VC, BASE_DBZ_1KM_VC_LONG, DBZ);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getBaseDbz1kmHx();
  }
  _addCalVar(group, dim, fvals,
             BASE_DBZ_1KM_HX, BASE_DBZ_1KM_HX_LONG, DBZ);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getBaseDbz1kmVx();
  }
  _addCalVar(group, dim, fvals,
             BASE_DBZ_1KM_VX, BASE_DBZ_1KM_VX_LONG, DBZ);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSunPowerDbmHc();
  }
  _addCalVar(group, dim, fvals,
             SUN_POWER_HC, SUN_POWER_HC_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSunPowerDbmVc();
  }
  _addCalVar(group, dim, fvals,
             SUN_POWER_VC, SUN_POWER_VC_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSunPowerDbmHx();
  }
  _addCalVar(group, dim, fvals,
             SUN_POWER_HX, SUN_POWER_HX_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSunPowerDbmVx();
  }
  _addCalVar(group, dim, fvals,
             SUN_POWER_VX, SUN_POWER_VX_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseSourcePowerDbmH();
  }
  _addCalVar(group, dim, fvals,
             NOISE_SOURCE_POWER_H, NOISE_SOURCE_POWER_H_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseSourcePowerDbmV();
  }
  _addCalVar(group, dim, fvals,
             NOISE_SOURCE_POWER_V, NOISE_SOURCE_POWER_V_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getPowerMeasLossDbH();
  }
  _addCalVar(group, dim, fvals,
             POWER_MEASURE_LOSS_H, POWER_MEASURE_LOSS_H_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getPowerMeasLossDbV();
  }
  _addCalVar(group, dim, fvals,
             POWER_MEASURE_LOSS_V, POWER_MEASURE_LOSS_V_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getCouplerForwardLossDbH();
  }
  _addCalVar(group, dim, fvals,
             COUPLER_FORWARD_LOSS_H, COUPLER_FORWARD_LOSS_H_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getCouplerForwardLossDbV();
  }
  _addCalVar(group, dim, fvals,
             COUPLER_FORWARD_LOSS_V, COUPLER_FORWARD_LOSS_V_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getDbzCorrection();
  }
  _addCalVar(group, dim, fvals,
             DBZ_CORRECTION, DBZ_CORRECTION_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getZdrCorrectionDb();
  }
  _addCalVar(group, dim, fvals,
             ZDR_CORRECTION, ZDR_CORRECTION_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getLdrCorrectionDbH();
  }
  _addCalVar(group, dim, fvals,
             LDR_CORRECTION_H, LDR_CORRECTION_H_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getLdrCorrectionDbV();
  }
  _addCalVar(group, dim, fvals,
             LDR_CORRECTION_V, LDR_CORRECTION_V_LONG, DB);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSystemPhidpDeg();
  }
  _addCalVar(group, dim, fvals,
             SYSTEM_PHIDP, SYSTEM_PHIDP_LONG, DEGREES);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTestPowerDbmH();
  }
  _addCalVar(group, dim, fvals,
             TEST_POWER_H, TEST_POWER_H_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTestPowerDbmV();
  }
  _addCalVar(group, dim, fvals,
             TEST_POWER_V, TEST_POWER_V_LONG, DBM);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverSlopeDbHc();
  }
  _addCalVar(group, dim, fvals,
             RECEIVER_SLOPE_HC, RECEIVER_SLOPE_HC_LONG);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverSlopeDbVc();
  }
  _addCalVar(group, dim, fvals,
             RECEIVER_SLOPE_VC, RECEIVER_SLOPE_VC_LONG);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverSlopeDbHx();
  }
  _addCalVar(group, dim, fvals,
             RECEIVER_SLOPE_HX, RECEIVER_SLOPE_HX_LONG);

  for (size_t ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverSlopeDbVx();
  }
  _addCalVar(group, dim, fvals,
             RECEIVER_SLOPE_VX, RECEIVER_SLOPE_VX_LONG);

}

//////////////////////////////////////////////
// add calibration variable

void Cf2RadxFile::_addCalVar(NcxxGroup &group,
                             NcxxDim &dim,
                             float *vals,
                             const string &name,
                             const string &standardName,
                             const string &units /* = "" */)
{

  // create var

  NcxxVar var = group.addVar(name, ncxxFloat, dim);
  
  // add attributes

  if (standardName.length() > 0) {
    var.addScalarAttr(LONG_NAME, standardName);
  }

  var.putAtt(UNITS, units);

  // var.putAtt(META_GROUP, RADAR_CALIBRATION);

  var.addScalarAttr(FILL_VALUE, Radx::missingMetaFloat);

  // add data

  var.putVal(vals);

}

#ifdef JUNK
////////////////////////////////////////////////
// write projection variables

int Cf2RadxFile::_writeProjectionVariables()
{

  if (_verbose) {
    cerr << "Cf2RadxFile::_writeProjectionVariables()" << endl;
  }

  if (_georefsActive) {
    // these will be written later, as vectors instead of scalars
    return 0;
  }

  try {

    _latitudeVar.putVal(_writeVol->getLatitudeDeg());
    _longitudeVar.putVal(_writeVol->getLongitudeDeg());

    double altitudeM = Radx::missingMetaDouble;
    if (_writeVol->getAltitudeKm() != Radx::missingMetaDouble) {
      altitudeM = _writeVol->getAltitudeKm() * 1000.0;
    }
    _altitudeVar.putVal(altitudeM);

    double htAglM = Radx::missingMetaDouble;
    if (_writeVol->getSensorHtAglM() != Radx::missingMetaDouble) {
      htAglM = _writeVol->getSensorHtAglM();
    }
    _altitudeAglVar.putVal(htAglM);

  } catch (NcxxException& e) {

    _addErrStr("ERROR - Cf2RadxFile::_writeProjectionVariables");
    _addErrStr("  Cannot write var");
    _addErrStr(_file.getErrStr());
    _addErrStr("  Exception: ", e.what());
    return -1;

  }
    
  return 0;

}
#endif

//////////////////
// close on error

int Cf2RadxFile::_closeOnError(const string &caller)
{
  _addErrStr("ERROR - Cf2RadxFile::" + caller);
  _addErrStr(_file.getErrStr());
  _file.close();
  unlink(_tmpPath.c_str());
  return -1;
}

///////////////////////////////////////////////////////////////////////////
// Set output compression for variable

int Cf2RadxFile::_setCompression(NcxxVar &var)  
{

  if (_ncFormat == NETCDF_CLASSIC || _ncFormat == NETCDF_OFFSET_64BIT) {
    // cannot compress
    return 0;
  }

  if (var.isNull()) {
    _addErrStr("ERROR - Cf2RadxFile::_setCompression");
    _addErrStr("  var is NULL");
    return -1;
  }
  
  if (!_writeCompressed) {
    return 0;
  }

  try {

    var.setCompression(false, _writeCompressed, _compressionLevel);
        
  } catch (NcxxException& e) {
    
    _addErrStr("ERROR - Cf2RadxFile::_setCompression");
    _addErrStr("  Cannot set compression: ", var.getName());
    _addErrStr("  Exception: ", e.what());
    return -1;

  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// Compute the output path

string Cf2RadxFile::_computeWritePath(const RadxVol &vol,
                                      const RadxTime &startTime,
                                      int startMillisecs,
                                      const RadxTime &endTime,
                                      int endMillisecs,
                                      const RadxTime &fileTime,
                                      int fileMillisecs,
                                      const string &dir)

{

  // compute path
  
  string scanType;
  if (_writeScanTypeInFileName) {
    scanType = "_SUR";
    if (_writeVol->getSweeps().size() > 0) {
      Radx::SweepMode_t predomSweepMode = _writeVol->getPredomSweepMode();
      scanType = "_";
      scanType += Radx::sweepModeToShortStr(predomSweepMode);
    }
  }

  string instName;
  if (_writeInstrNameInFileName) {
    if (vol.getInstrumentName().size() > 0) {
      instName = "_";
      instName += vol.getInstrumentName();
    }
  }

  string siteName;
  if (_writeSiteNameInFileName) {
    if (vol.getSiteName().size() > 0) {
      siteName = "_";
      siteName += vol.getSiteName();
    }
  }

  string scanName;
  if (vol.getScanName().size() > 0) {
    if (strcasestr(vol.getScanName().c_str(), "default") == NULL) {
      scanName += "_";
      scanName += vol.getScanName();
    }
  }

  int volNum = vol.getVolumeNumber();
  char volNumStr[1024];
  if (_writeVolNumInFileName && volNum >= 0) {
    sprintf(volNumStr, "_v%d", volNum);
  } else {
    volNumStr[0] = '\0'; // NULL str
  }

  string prefix = "cfrad2.";
  if (_writeFileNamePrefix.size() > 0) {
    prefix = _writeFileNamePrefix;
  }

  string suffix = "";
  if (_writeFileNameSuffix.size() > 0) {
    suffix = _writeFileNameSuffix;
  }

  char dateTimeConnector = '_';
  if (_writeHyphenInDateTime) {
    dateTimeConnector = '-';
  }

  char fileName[BUFSIZ];
  if (_writeFileNameMode == FILENAME_WITH_START_AND_END_TIMES) {

    char startSubsecsStr[64];
    char endSubsecsStr[64];
    if (_writeSubsecsInFileName) {
      sprintf(startSubsecsStr, ".%.3d", startMillisecs);
      sprintf(endSubsecsStr, ".%.3d", endMillisecs);
    } else {
      startSubsecsStr[0] = '\0';
      endSubsecsStr[0] = '\0';
    }

    sprintf(fileName,
            "%s%.4d%.2d%.2d%c%.2d%.2d%.2d%s"
            "_to_%.4d%.2d%.2d%c%.2d%.2d%.2d%s"
            "%s%s%s"
            "%s%s%s.nc",
            prefix.c_str(),
            startTime.getYear(), startTime.getMonth(), startTime.getDay(),
            dateTimeConnector,
            startTime.getHour(), startTime.getMin(), startTime.getSec(),
            startSubsecsStr,
            endTime.getYear(), endTime.getMonth(), endTime.getDay(),
            dateTimeConnector,
            endTime.getHour(), endTime.getMin(), endTime.getSec(),
            endSubsecsStr,
            instName.c_str(), siteName.c_str(), volNumStr,
            scanName.c_str(), scanType.c_str(), suffix.c_str());

  } else {
    
    char fileSubsecsStr[64];
    if (_writeSubsecsInFileName) {
      sprintf(fileSubsecsStr, ".%.3d", fileMillisecs);
    } else {
      fileSubsecsStr[0] = '\0';
    }

    sprintf(fileName,
            "%s%.4d%.2d%.2d%c%.2d%.2d%.2d%s"
            "%s%s%s"
            "%s%s%s.nc",
            prefix.c_str(),
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay(),
            dateTimeConnector,
            fileTime.getHour(), fileTime.getMin(), fileTime.getSec(),
            fileSubsecsStr,
            instName.c_str(), siteName.c_str(), volNumStr,
            scanName.c_str(), scanType.c_str(), suffix.c_str());

  }

  // make sure the file name is valid - i.e. no / or whitespace

  for (size_t ii = 0; ii < strlen(fileName); ii++) {
    if (isspace(fileName[ii]) || fileName[ii] == '/') {
      fileName[ii] = '_';
    }
  }

  // construct path

  string outPath(dir);
  outPath += PATH_SEPARATOR;
  outPath += fileName;
  return outPath;

}
