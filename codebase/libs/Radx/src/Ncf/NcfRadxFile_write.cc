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
// NcfRadxFile_write.cc
//
// Write methods for NcfRadxFile object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
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

int NcfRadxFile::writeToDir(const RadxVol &vol,
                            const string &dir,
                            bool addDaySubDir,
                            bool addYearSubDir)
  
{

  if (_debug) {
    cerr << "DEBUG - NcfRadxFile::writeToDir" << endl;
    cerr << "  Writing to dir: " << dir << endl;
  }

  _writePaths.clear();
  _writeDataTimes.clear();
  clearErrStr();

  // should we split the sweeps?

  if (_writeIndividualSweeps) {
    return _writeSweepsToDir(vol, dir, addDaySubDir, addYearSubDir);
    // } else if (vol.getSweeps().size() == 1) {
    //   return _writeSweepToDir(vol, dir, addDaySubDir, addYearSubDir);
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
    _addErrStr("ERROR - NcfRadxFile::writeToDir");
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
    _addErrStr("ERROR - NcfRadxFile::_writeToDir");
    return -1;
  }

  return 0;

}

////////////////////////////////////////////
// split volume into sweeps and write to dir

int NcfRadxFile::_writeSweepsToDir(const RadxVol &vol,
                                   const string &dir,
                                   bool addDaySubDir,
                                   bool addYearSubDir)
  
{

  if (_debug) {
    cerr << "DEBUG - NcfRadxFile::_writeSweepsToDir" << endl;
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

int NcfRadxFile::_writeSweepToDir(const RadxVol &vol,
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
    cerr << "DEBUG - NcfRadxFile::_writeSweepToDir" << endl;
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
    _addErrStr("ERROR - NcfRadxFile::writeToDir");
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
            "cfrad.%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d"
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
            "cfrad.%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d"
            "_%s_v%d_s%.2d_%s%.2f_%s.nc",
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay(),
            fileTime.getHour(), fileTime.getMin(), fileTime.getSec(),
            fileMillisecs,
            instName.c_str(),
            volNum, sweepNum,
            fixedAngleLabel.c_str(), fixedAngle,
            scanType.c_str());
  }
  
  // make sure the file name is valid - i.e. no / or whitespace

  for (size_t ii = 0; ii < strlen(fileName); ii++) {
    if (isspace(fileName[ii]) || fileName[ii] == '/') {
      fileName[ii] = '_';
    }
  }

  char outPath[BUFSIZ];
  sprintf(outPath, "%s%s%s",
          outDir.c_str(), PATH_SEPARATOR,  fileName);
  
  int iret = writeToPath(*_writeVol, outPath);

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_writeToDir");
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

int NcfRadxFile::writeToPath(const RadxVol &vol,
                             const string &path)
  
{

  clearErrStr();
  _writeVol = &vol;
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _writePaths.clear();
  _writeDataTimes.clear();

  _gateGeomVaries = _writeVol->gateGeomVariesByRay();

  // open the output Nc file

  _tmpPath = tmpPathFromFilePath(path, "");

  if (_debug) {
    cerr << "DEBUG - NcfRadxFile::writeToPath" << endl;
    cerr << "  Writing to path: " << path << endl;
    cerr << "  Tmp path is: " << _tmpPath << endl;
    cerr << "  Writing fields and compressing ..." << endl;
  }

  if (_file.openWrite(_tmpPath, _getFileFormat(_ncFormat))) {
    _addErrStr("ERROR - NcfRadxFile::writeToPath");
    _addErrStr("  Cannot open tmp Nc file: ", _tmpPath);
    _addErrStr(_file.getErrStr());
    return -1;
  }

  // do the number of gates vary by ray?
  // force this on for now
  
  _writeVol->computeMaxNGates();
  _nGatesVary = _writeVol->getNGatesVary();
  if (_writeForceNgatesVary) {
    _nGatesVary = true;
  }

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
  
  if (_addGlobalAttributes()) {
    return _closeOnError("_addGlobalAttributes");
  }
  if (_addDimensions()) {
    return _closeOnError("_addDimensions");
  }
  if (_addScalarVariables()) {
    return _closeOnError("_addScalarVariables");
  }
  if (_addFrequencyVariable()) {
    return _closeOnError("_addFrequencyVariable");
  }
  if (_correctionsActive) {
    if (_addCorrectionVariables()) {
      return _closeOnError("_addCorrectionVariables");
    }
  }
  if (_addProjectionVariables()) {
    return _closeOnError("_addProjectionVariables");
  }
  if (_addSweepVariables()) {
    return _closeOnError("_addSweepVariables");
  }
  if (_addCalibVariables()) {
    return _closeOnError("_addCalibVariables");
  }
  if (_addCoordinateVariables()) {
    return _closeOnError("_addCoordinateVariables");
  }
  if (_addRayVariables()) {
    return _closeOnError("_addRayVariables");
  }
  if (_georefsActive) {
    if (_addGeorefVariables()) {
      return _closeOnError("_addGeorefVariables");
    }
  }

  // write variables

  if (_writeScalarVariables()) {
    return _closeOnError("_writeScalarVariables");
  }
  if (_writeFrequencyVariable()) {
    return _closeOnError("_writeFrequencyVariable");
  }
  if (_correctionsActive) {
    if (_writeCorrectionVariables()) {
      return _closeOnError("_writeCorrectionVariables");
    }
  }
  if (_writeProjectionVariables()) {
    return _closeOnError("_writeProjectionVariables");
  }
  if (_writeSweepVariables()) {
    return _closeOnError("_writeSweepVariables");
  }
  if (_writeCalibVariables()) {
    return _closeOnError("_writeCalibVariables");
  }
  if (_writeCoordinateVariables()) {
    return _closeOnError("_writeCoordinateVariables");
  }
  if (_writeRayVariables()) {
    return _closeOnError("_writeRayVariables");
  }
  if (_georefsActive) {
    if (_writeGeorefVariables()) {
      return _closeOnError("_writeGeorefVariables");
    }
  }
  if (_writeFieldVariables()) {
    return _closeOnError("_writeFieldVariables");
  }

  // close output file

  _file.close();

  // rename the tmp to final output file path
  
  if (rename(_tmpPath.c_str(), _pathInUse.c_str())) {
    int errNum = errno;
    _addErrStr("ERROR - NcfRadxFile::writeToPath");
    _addErrStr("  Cannot rename tmp file: ", _tmpPath);
    _addErrStr("  to: ", _pathInUse);
    _addErrStr(strerror(errNum));
    return -1;
  }

  if (_debug) {
    cerr << "DEBUG - NcfRadxFile::writeToPath" << endl;
    cerr << "  Renamed tmp path: " << _tmpPath << endl;
    cerr << "     to final path: " << path << endl;
  }

  _writePaths.push_back(path);
  _writeDataTimes.push_back(vol.getStartTimeSecs());
  
  return 0;

}

/////////////////////////////////////////////////
// check if georeferences are active on write

void NcfRadxFile::_checkGeorefsActiveOnWrite()
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

void NcfRadxFile::_checkCorrectionsActiveOnWrite()
{
  
  _correctionsActive = false;
  if (_writeVol->getCfactors() != NULL) {
    _correctionsActive = true;
  }

}
  
///////////////////////////////////////////////////////////////
// addGlobalAttributes()
//

int NcfRadxFile::_addGlobalAttributes()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_addGlobalAttributes()" << endl;
  }

  // Add required CF-1.5 global attributes
  
  _conventions = CfConvention;
  _subconventions = BaseConvention;
  _subconventions += " ";
  _subconventions += INSTRUMENT_PARAMETERS;
  if (_writeVol->getInstrumentType() == Radx::INSTRUMENT_TYPE_RADAR) {
    _subconventions += " ";
    _subconventions += RADAR_PARAMETERS;
    if (_writeVol->getRcalibs().size() > 0) {
      _subconventions += " ";
      _subconventions += RADAR_CALIBRATION;
    }
  } else {
    _subconventions += " ";
    _subconventions += LIDAR_PARAMETERS;
  }
  if (_writeVol->getPlatformType() != Radx::PLATFORM_TYPE_FIXED) {
    _subconventions += " ";
    _subconventions += PLATFORM_VELOCITY;
  }
  if (_writeVol->getCfactors() != NULL) {
    _subconventions += " ";
    _subconventions += GEOMETRY_CORRECTION;
  }

  if (_file.addGlobAttr(CONVENTIONS, _conventions)) {
    return -1;
  }
  if (_file.addGlobAttr(SUB_CONVENTIONS, _subconventions)) {
    return -1;
  }
  
  // Version

  _version = CurrentVersion;
  if (_writeVol->getVersion().size() > 0) {
    _version = _writeVol->getVersion();
  }
  if (_file.addGlobAttr(VERSION, _version)) {
    return -1;
  }
  
  // info strings

  if (_file.addGlobAttr(TITLE,  _writeVol->getTitle())) {
    return -1;
  }

  if (_file.addGlobAttr(INSTITUTION, _writeVol->getInstitution())) {
    return -1;
  }

  if (_file.addGlobAttr(REFERENCES,  _writeVol->getReferences())) {
    return -1;
  }

  if (_file.addGlobAttr(SOURCE,  _writeVol->getSource())) {
    return -1;
  }

  if (_file.addGlobAttr(HISTORY,  _writeVol->getHistory())) {
    return -1;
  }

  if (_file.addGlobAttr(COMMENT,  _writeVol->getComment())) {
    return -1;
  }

  if (_writeVol->getAuthor().size() > 0) {
    if (_file.addGlobAttr(AUTHOR,  _writeVol->getAuthor())) {
      return -1;
    }
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
    if (_file.addGlobAttr(ORIGINAL_FORMAT,  origFormat)) {
      return -1;
    }
  }

  if (_file.addGlobAttr(DRIVER,  _writeVol->getDriver())) {
    return -1;
  }

  if (_file.addGlobAttr(CREATED,  _writeVol->getCreated())) {
    return -1;
  }

  // times
  
  RadxTime startTime(_writeVol->getStartTimeSecs());
  _file.addGlobAttr(START_DATETIME, startTime.getW3cStr());
  _file.addGlobAttr(TIME_COVERAGE_START, startTime.getW3cStr());
  startTime += _writeVol->getStartNanoSecs() / 1.0e9;
  _file.addGlobAttr("start_time", startTime.asStringDashed(3));

  RadxTime endTime(_writeVol->getEndTimeSecs());
  _file.addGlobAttr(END_DATETIME, endTime.getW3cStr());
  _file.addGlobAttr(TIME_COVERAGE_END, endTime.getW3cStr());
  endTime += _writeVol->getEndNanoSecs() / 1.0e9;
  _file.addGlobAttr("end_time", endTime.asStringDashed(3));

  // names

  if (_file.addGlobAttr(INSTRUMENT_NAME,  _writeVol->getInstrumentName())) {
    return -1;
  }

  if (_file.addGlobAttr(SITE_NAME,  _writeVol->getSiteName())) {
    return -1;
  }

  if (_file.addGlobAttr(SCAN_NAME,  _writeVol->getScanName())) {
    return -1;
  }

  if (_file.addGlobAttr(SCAN_ID,  _writeVol->getScanId())) {
    return -1;
  }

  string attrStr;
  if (_writeVol->getPlatformType() == Radx::PLATFORM_TYPE_FIXED) {
    attrStr = "false";
  } else {
    attrStr = "true";
  }
  if (_file.addGlobAttr(PLATFORM_IS_MOBILE, attrStr)) {
    return -1;
  }
  
  if (_nGatesVary) {
    attrStr = "true";
  } else {
    attrStr = "false";
  }
  if (_file.addGlobAttr(N_GATES_VARY, attrStr)) {
    return -1;
  }

  if (_rayTimesIncrease) {
    attrStr = "true";
  } else {
    attrStr = "false";
  }
  if (_file.addGlobAttr(RAY_TIMES_INCREASE, attrStr)) {
    return -1;
  }

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
          cerr << "ERROR - NcfRadxFile::_addGlobalAttributes()" << endl;
          cerr << "  Cannot decode user-defined global attribute" << endl;
          cerr << "    name: " << attr.name << endl;
          cerr << "    type: int" << endl;
          cerr << "    val: " << attr.val << endl;
        }
        break;
      }
      case RadxVol::UserGlobAttr::ATTR_DOUBLE: {
        double dval;
        if (sscanf(attr.val.c_str(), "%lg", &dval) == 1) {
          _file.addGlobAttr(attr.name, dval);
        } else {
          cerr << "ERROR - NcfRadxFile::_addGlobalAttributes()" << endl;
          cerr << "  Cannot decode user-defined global attribute" << endl;
          cerr << "    name: " << attr.name << endl;
          cerr << "    type: double" << endl;
          cerr << "    val: " << attr.val << endl;
        }
        break;
      }
      case RadxVol::UserGlobAttr::ATTR_INT_ARRAY: {
        bool haveError = false;
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
          _file.addGlobAttr(attr.name, toks.size(), ivals);
        } else {
          cerr << "ERROR - NcfRadxFile::_addGlobalAttributes()" << endl;
          cerr << "  Cannot decode user-defined global attribute" << endl;
          cerr << "    name: " << attr.name << endl;
          cerr << "    type: int[]" << endl;
          cerr << "    vals: " << attr.val << endl;
        }
        break;
      }
      case RadxVol::UserGlobAttr::ATTR_DOUBLE_ARRAY: {
        bool haveError = false;
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
          _file.addGlobAttr(attr.name, toks.size(), dvals);
        } else {
          cerr << "ERROR - NcfRadxFile::_addGlobalAttributes()" << endl;
          cerr << "  Cannot decode user-defined global attribute" << endl;
          cerr << "    name: " << attr.name << endl;
          cerr << "    type: double[]" << endl;
          cerr << "    vals: " << attr.val << endl;
        }
        break;
      }
      default: {}
    } // switch
  } // ii

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// addDimensions()
//
//  Add Nc3Dims to the NetCDF file. We loop through the
//  GridInfo objects and record the dimensions of the
//  x and y coordinates. Then we loop through the VlevelInfo
//  objects and record the dimensions of the vertical coordinates

int NcfRadxFile::_addDimensions()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_addDimensions()" << endl;
  }

  // add time dimension - unlimited??

  // if (_addDim(_timeDim, TIME, -1)) {
  if (_file.addDim(_timeDim, TIME, _writeVol->getRays().size())) {
    return -1;
  }

  // add range dimension

  if (_file.addDim(_rangeDim, RANGE, _writeVol->getMaxNGates())) {
    return -1;
  }

  // add n_points dimension if applicable

  if (_nGatesVary) {
    if (_file.addDim(_nPointsDim, N_POINTS, _writeVol->getNPoints())) {
      return -1;
    }
  }

  // add sweep dimension

  if (_file.addDim(_sweepDim, SWEEP, _writeVol->getSweeps().size())) {
    return -1;
  }

  // add dimensions for strings in the file

  if (_file.addDim(_stringLen8Dim,
                   STRING_LENGTH_8, NCF_STRING_LEN_8)) {
    return -1;
  }
  if (_file.addDim(_stringLen32Dim,
                   STRING_LENGTH_32, NCF_STRING_LEN_32)) {
    return -1;
  }

#ifdef NOTYET
  if (_file.addDim(_stringLen64Dim,
                   STRING_LENGTH_64, NCF_STRING_LEN_64)) {
    return -1;
  }
  if (_file.addDim(_stringLen256Dim,
                   STRING_LENGTH_256, NCF_STRING_LEN_256)) {
    return -1;
  }
#endif

  if (_file.addDim(_statusXmlDim,
                   STATUS_XML_LENGTH,
                   _writeVol->getStatusXml().size() + 1)) {
    return -1;
  }

  // add calib dimension

  if (_writeVol->getRcalibs().size() > 0) {
    if (_file.addDim(_calDim, R_CALIB, 
                     _writeVol->getRcalibs().size())) {
      return -1;
    }
  }

  // add multiple frequencies dimension

  if (_writeVol->getFrequencyHz().size() > 0) {
    if (_file.addDim(_frequencyDim, FREQUENCY, 
                     _writeVol->getFrequencyHz().size())) {
      return -1;
    }
  }

  return 0;

}

////////////////////////////////////////////////
// add variables and attributes for coordinates

int NcfRadxFile::_addCoordinateVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_addCoordinateVariables()" << endl;
  }

  // Note that in CF coordinate variables have the same name as their
  // dimension so we will use the same naming scheme for vars as we
  // did for dimensions in method addDimensions()

  // time

  if ((_timeVar = _file.getNc3File()->add_var(TIME,
                                             nc3Double, _timeDim)) == NULL) {
    _addErrStr("ERROR - NcfRadxFile::_addCoordinateVariables");
    _addErrStr("  Cannot add time var");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  int iret = 0;
  iret |= _file.addAttr(_timeVar, STANDARD_NAME, TIME);
  iret |= _file.addAttr(_timeVar, LONG_NAME,
                        "time in seconds since volume start");
  iret |= _file.addAttr(_timeVar, CALENDAR, GREGORIAN);

  char timeUnitsStr[256];
  RadxTime stime(_writeVol->getStartTimeSecs());
  sprintf(timeUnitsStr, "seconds since %.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
          stime.getYear(), stime.getMonth(), stime.getDay(),
          stime.getHour(), stime.getMin(), stime.getSec());
  iret |= _file.addAttr(_timeVar, UNITS, timeUnitsStr);

  iret |= _file.addAttr(_timeVar, COMMENT,
                        "times are relative to the volume start_time");
  
  // range
  
  if (_gateGeomVaries) {
  
    if ((_rangeVar =
         _file.getNc3File()->add_var(RANGE,
                                    nc3Float, _timeDim, _rangeDim)) == NULL) {
      _addErrStr("ERROR - NcfRadxFile::_addCoordinateVariables");
      _addErrStr("  Cannot add range var");
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  } else {

    if ((_rangeVar = _file.getNc3File()->add_var(RANGE,
                                                nc3Float, _rangeDim)) == NULL) {
      _addErrStr("ERROR - NcfRadxFile::_addCoordinateVariables");
      _addErrStr("  Cannot add range var");
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  }

  iret |= _file.addAttr(_rangeVar, LONG_NAME, RANGE_LONG);
  iret |= _file.addAttr(_rangeVar, LONG_NAME,
                        "Range from instrument to center of gate");
  iret |= _file.addAttr(_rangeVar, UNITS, METERS);
  iret |= _file.addAttr(_rangeVar, SPACING_IS_CONSTANT, "true");
  iret |= _file.addAttr(_rangeVar, METERS_TO_CENTER_OF_FIRST_GATE,
                        (float) _writeVol->getStartRangeKm() * 1000.0);
  iret |= _file.addAttr(_rangeVar, METERS_BETWEEN_GATES, 
                        (float) _writeVol->getGateSpacingKm() * 1000.0);

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_addCoordinateVariables");
    _addErrStr("  Cannot add attributes");
    return -1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////
// add scalar variables

int NcfRadxFile::_addScalarVariables()
{

  int iret = 0;
  if (_verbose) {
    cerr << "NcfRadxFile::_addScalarVariables()" << endl;
  }

  iret |= _file.addMetaVar(_volumeNumberVar, VOLUME_NUMBER,
                           "", VOLUME_NUMBER_LONG, nc3Int, "");

  iret |= _file.addMetaVar(_platformTypeVar, PLATFORM_TYPE,
                           "", PLATFORM_TYPE_LONG, nc3Char, _stringLen32Dim, "");
  iret |= _file.addAttr(_platformTypeVar, OPTIONS, Radx::platformTypeOptions());
  
  iret |= _file.addMetaVar(_primaryAxisVar, PRIMARY_AXIS,
                           "", PRIMARY_AXIS_LONG, nc3Char, _stringLen32Dim, "");
  iret |= _file.addAttr(_primaryAxisVar, OPTIONS, Radx::primaryAxisOptions());

  iret |= _file.addMetaVar(_statusXmlVar, STATUS_XML,
                           "", "status_of_instrument", nc3Char, _statusXmlDim, "");
  
  iret |= _file.addMetaVar(_instrumentTypeVar, INSTRUMENT_TYPE,
                           "", INSTRUMENT_TYPE_LONG, nc3Char, _stringLen32Dim, "");
  iret |= _file.addAttr(_instrumentTypeVar, OPTIONS, Radx::instrumentTypeOptions());
  iret |= _file.addAttr(_instrumentTypeVar, META_GROUP, INSTRUMENT_PARAMETERS);

  if (_writeVol->getInstrumentType() == Radx::INSTRUMENT_TYPE_RADAR) {

    // radar

    iret |= _file.addMetaVar(_radarAntennaGainHVar, RADAR_ANTENNA_GAIN_H,
                             "", RADAR_ANTENNA_GAIN_H_LONG, nc3Float, DB);
    iret |= _file.addMetaVar(_radarAntennaGainVVar, RADAR_ANTENNA_GAIN_V,
                             "", RADAR_ANTENNA_GAIN_V_LONG, nc3Float, DB);
    iret |= _file.addMetaVar(_radarBeamWidthHVar, RADAR_BEAM_WIDTH_H,
                             "", RADAR_BEAM_WIDTH_H_LONG, nc3Float, DEGREES);
    iret |= _file.addMetaVar(_radarBeamWidthVVar, RADAR_BEAM_WIDTH_V,
                             "", RADAR_BEAM_WIDTH_V_LONG, nc3Float, DEGREES);
    iret |= _file.addMetaVar(_radarRxBandwidthVar, RADAR_RX_BANDWIDTH,
                             "", RADAR_RX_BANDWIDTH_LONG, nc3Float, HZ);
    
    iret |= _file.addAttr(_radarAntennaGainHVar, META_GROUP, RADAR_PARAMETERS);
    iret |= _file.addAttr(_radarAntennaGainVVar, META_GROUP, RADAR_PARAMETERS);
    iret |= _file.addAttr(_radarBeamWidthHVar, META_GROUP, RADAR_PARAMETERS);
    iret |= _file.addAttr(_radarBeamWidthVVar, META_GROUP, RADAR_PARAMETERS);
    iret |= _file.addAttr(_radarRxBandwidthVar, META_GROUP, RADAR_PARAMETERS);

  } else {

    // lidar

    iret |= _file.addMetaVar(_lidarConstantVar, LIDAR_CONSTANT,
                             "", LIDAR_CONSTANT_LONG, nc3Float, DB);
    iret |= _file.addMetaVar(_lidarPulseEnergyJVar, LIDAR_PULSE_ENERGY,
                             "", LIDAR_PULSE_ENERGY_LONG, nc3Float, JOULES);
    iret |= _file.addMetaVar(_lidarPeakPowerWVar, LIDAR_PEAK_POWER,
                             "", LIDAR_PEAK_POWER_LONG, nc3Float, WATTS);
    iret |= _file.addMetaVar(_lidarApertureDiamCmVar, LIDAR_APERTURE_DIAMETER,
                             "", LIDAR_APERTURE_DIAMETER_LONG, nc3Float, CM);
    iret |= _file.addMetaVar(_lidarApertureEfficiencyVar, LIDAR_APERTURE_EFFICIENCY,
                             "", LIDAR_APERTURE_EFFICIENCY_LONG, nc3Float, PERCENT);
    iret |= _file.addMetaVar(_lidarFieldOfViewMradVar, LIDAR_FIELD_OF_VIEW,
                             "", LIDAR_FIELD_OF_VIEW_LONG, nc3Float, MRAD);
    iret |= _file.addMetaVar(_lidarBeamDivergenceMradVar, LIDAR_BEAM_DIVERGENCE,
                             "", LIDAR_BEAM_DIVERGENCE_LONG, nc3Float, MRAD);
    
    iret |= _file.addAttr(_lidarConstantVar, META_GROUP, LIDAR_PARAMETERS);
    iret |= _file.addAttr(_lidarPulseEnergyJVar, META_GROUP, LIDAR_PARAMETERS);
    iret |= _file.addAttr(_lidarPeakPowerWVar, META_GROUP, LIDAR_PARAMETERS);
    iret |= _file.addAttr(_lidarApertureDiamCmVar, META_GROUP, LIDAR_PARAMETERS);
    iret |= _file.addAttr(_lidarApertureEfficiencyVar, META_GROUP, LIDAR_PARAMETERS);
    iret |= _file.addAttr(_lidarFieldOfViewMradVar, META_GROUP, LIDAR_PARAMETERS);
    iret |= _file.addAttr(_lidarBeamDivergenceMradVar, META_GROUP, LIDAR_PARAMETERS);

  }

  // start time

  iret |= _file.addMetaVar(_startTimeVar, TIME_COVERAGE_START,
                           "", TIME_COVERAGE_START_LONG, nc3Char, _stringLen32Dim, "");
  iret |= _file.addAttr(_startTimeVar, COMMENT,
                        "ray times are relative to start time in secs");

  // end time

  iret |= _file.addMetaVar(_endTimeVar, TIME_COVERAGE_END,
                           "", TIME_COVERAGE_END_LONG, nc3Char, _stringLen32Dim, "");

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_addScalarVariables");
    return -1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////
// add variable for one or more frequencies

int NcfRadxFile::_addFrequencyVariable()
{
  
  if (_writeVol->getFrequencyHz().size() < 1) {
    return 0;
  }
  
  if(_file.addMetaVar(_frequencyVar, FREQUENCY,
                      "", FREQUENCY_LONG, nc3Float, _frequencyDim, HZ)) {
    _addErrStr("ERROR - NcfRadxFile::_addFrequencyVariable");
    return -1;
  }
  _file.addAttr(_frequencyVar, META_GROUP, INSTRUMENT_PARAMETERS);

  return 0;

}

//////////////////////////////////////////////
// add correction variables

int NcfRadxFile::_addCorrectionVariables()
{

  int iret = 0;
  if (_verbose) {
    cerr << "NcfRadxFile::_addCorrectionVariables()" << endl;
  }

  iret |= _file.addMetaVar(_azimuthCorrVar, AZIMUTH_CORRECTION,
                           "", AZIMUTH_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_azimuthCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_elevationCorrVar, ELEVATION_CORRECTION,
                           "", ELEVATION_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_elevationCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_rangeCorrVar, RANGE_CORRECTION,
                           "", RANGE_CORRECTION_LONG, nc3Float, METERS);
  iret |= _file.addAttr(_rangeCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_longitudeCorrVar, LONGITUDE_CORRECTION,
                           "", LONGITUDE_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_longitudeCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_latitudeCorrVar, LATITUDE_CORRECTION,
                           "", LATITUDE_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_latitudeCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_pressureAltCorrVar, PRESSURE_ALTITUDE_CORRECTION,
                           "", PRESSURE_ALTITUDE_CORRECTION_LONG,
                           nc3Float, METERS);
  iret |= _file.addAttr(_pressureAltCorrVar, META_GROUP, GEOMETRY_CORRECTION);
  
  iret |= _file.addMetaVar(_altitudeCorrVar, ALTITUDE_CORRECTION,
                           "", ALTITUDE_CORRECTION_LONG, nc3Float, METERS);
  iret |= _file.addAttr(_altitudeCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_ewVelCorrVar, EASTWARD_VELOCITY_CORRECTION,
                           "", EASTWARD_VELOCITY_CORRECTION_LONG, 
                           nc3Float, METERS_PER_SECOND);
  iret |= _file.addAttr(_ewVelCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_nsVelCorrVar, NORTHWARD_VELOCITY_CORRECTION,
                           "", NORTHWARD_VELOCITY_CORRECTION_LONG,
                           nc3Float, METERS_PER_SECOND);
  iret |= _file.addAttr(_nsVelCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_vertVelCorrVar, VERTICAL_VELOCITY_CORRECTION,
                           "", VERTICAL_VELOCITY_CORRECTION_LONG,
                           nc3Float, METERS_PER_SECOND);
  iret |= _file.addAttr(_vertVelCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_headingCorrVar, HEADING_CORRECTION,
                           "", HEADING_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_headingCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_rollCorrVar, ROLL_CORRECTION,
                           "", ROLL_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_rollCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_pitchCorrVar, PITCH_CORRECTION,
                           "", PITCH_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_pitchCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_driftCorrVar, DRIFT_CORRECTION,
                           "", DRIFT_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_driftCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_rotationCorrVar, ROTATION_CORRECTION,
                           "", ROTATION_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_rotationCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  iret |= _file.addMetaVar(_tiltCorrVar, TILT_CORRECTION,
                           "", TILT_CORRECTION_LONG, nc3Float, DEGREES);
  iret |= _file.addAttr(_tiltCorrVar, META_GROUP, GEOMETRY_CORRECTION);

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_addCorrectionVariables");
    return -1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////
// add variables and attributes for projection

int NcfRadxFile::_addProjectionVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_addProjectionVariables()" << endl;
  }

  int iret = 0;

  // projection variable
  
  _projVar = _file.getNc3File()->add_var(GRID_MAPPING, nc3Int);
  if (_projVar == NULL) {
    _addErrStr("ERROR - NcfRadxFile::_addProjectionVariables");
    _addErrStr("  Cannot add projection variable:", GRID_MAPPING);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  iret |= _file.addAttr(_projVar, GRID_MAPPING_NAME, "radar_lidar_radial_scan");

  if (_writeVol->getPlatformType() == Radx::PLATFORM_TYPE_FIXED) {
    iret |= _file.addAttr(_projVar, LONGITUDE_OF_PROJECTION_ORIGIN,
                          _writeVol->getLongitudeDeg());
    iret |= _file.addAttr(_projVar, LATITUDE_OF_PROJECTION_ORIGIN,
                          _writeVol->getLatitudeDeg());
    iret |= _file.addAttr(_projVar, ALTITUDE_OF_PROJECTION_ORIGIN,
                          _writeVol->getAltitudeKm() * 1000.0);
    iret |= _file.addAttr(_projVar, FALSE_NORTHING, 0.0);
    iret |= _file.addAttr(_projVar, FALSE_EASTING, 0.0);
  }

  // lat/lon/alt

  if (!_georefsActive) {

    // georeferences on rays are not active, so write as scalars

    iret |= _file.addMetaVar(_latitudeVar, LATITUDE,
                             "", LATITUDE_LONG, nc3Double, DEGREES_NORTH);
    iret |= _file.addMetaVar(_longitudeVar, LONGITUDE,
                             "", LONGITUDE_LONG, nc3Double, DEGREES_EAST);
    iret |= _file.addMetaVar(_altitudeVar, ALTITUDE,
                             "", ALTITUDE_LONG, nc3Double, METERS);
    iret |= _file.addAttr(_altitudeVar, POSITIVE, UP);
    iret |= _file.addMetaVar(_altitudeAglVar, ALTITUDE_AGL,
                             "", ALTITUDE_AGL_LONG, nc3Double, METERS);
    iret |= _file.addAttr(_altitudeAglVar, POSITIVE, UP);

  }
    
  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_addProjectionVariables");
    _addErrStr("  Cannot add attributes");
    return -1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////
// add variables for sweep info

int NcfRadxFile::_addSweepVariables()
{
  
  if (_verbose) {
    cerr << "NcfRadxFile::_addSweepVariables()" << endl;
  }
  
  int iret = 0;

  iret |= _file.addMetaVar(_sweepNumberVar, SWEEP_NUMBER,
                           "", SWEEP_NUMBER_LONG,
                           nc3Int, _sweepDim, "");

  iret |= _file.addMetaVar(_sweepModeVar, SWEEP_MODE,
                           "", SWEEP_MODE_LONG,
                           nc3Char, _sweepDim, _stringLen32Dim, "");
  iret |= _file.addAttr(_sweepModeVar, OPTIONS, Radx::sweepModeOptions());

  iret |= _file.addMetaVar(_polModeVar, POLARIZATION_MODE,
                           "", POLARIZATION_MODE_LONG, 
                           nc3Char, _sweepDim, _stringLen32Dim, "");
  iret |= _file.addAttr(_polModeVar, OPTIONS, Radx::polarizationModeOptions());
  iret |= _file.addAttr(_polModeVar, META_GROUP, RADAR_PARAMETERS);
  
  iret |= _file.addMetaVar(_prtModeVar, PRT_MODE,
                           "", PRT_MODE_LONG, 
                           nc3Char, _sweepDim, _stringLen32Dim, "");
  iret |= _file.addAttr(_prtModeVar, OPTIONS, Radx::prtModeOptions());
  iret |= _file.addAttr(_prtModeVar, META_GROUP, RADAR_PARAMETERS);
  
  iret |= _file.addMetaVar(_sweepFollowModeVar, FOLLOW_MODE,
                           "", FOLLOW_MODE_LONG, 
                           nc3Char, _sweepDim, _stringLen32Dim, "");
  iret |= _file.addAttr(_sweepFollowModeVar, OPTIONS, Radx::followModeOptions());
  iret |= _file.addAttr(_sweepFollowModeVar, META_GROUP, INSTRUMENT_PARAMETERS);
  
  iret |= _file.addMetaVar(_sweepFixedAngleVar, FIXED_ANGLE,
                           "", FIXED_ANGLE_LONG, 
                           nc3Float, _sweepDim, DEGREES);
  iret |= _file.addMetaVar(_targetScanRateVar, TARGET_SCAN_RATE,
                           "", TARGET_SCAN_RATE_LONG, 
                           nc3Float, _sweepDim, DEGREES_PER_SECOND);

  iret |= _file.addMetaVar(_sweepStartRayIndexVar, SWEEP_START_RAY_INDEX,
                           "", SWEEP_START_RAY_INDEX_LONG, 
                           nc3Int, _sweepDim, "");
  iret |= _file.addMetaVar(_sweepEndRayIndexVar, SWEEP_END_RAY_INDEX,
                           "", SWEEP_END_RAY_INDEX_LONG, 
                           nc3Int, _sweepDim, "");
  
  iret |= _file.addMetaVar(_raysAreIndexedVar, RAYS_ARE_INDEXED,
                           "", RAYS_ARE_INDEXED_LONG, 
                           nc3Char, _sweepDim, _stringLen8Dim, "");

  iret |= _file.addMetaVar(_rayAngleResVar, RAY_ANGLE_RES,
                           "", RAY_ANGLE_RES_LONG, 
                           nc3Float, _sweepDim, DEGREES);
  
  bool haveIF = false;
  const vector<RadxSweep *> &sweeps = _writeVol->getSweeps();
  for (size_t ii = 0; ii < sweeps.size(); ii++) {
    if (sweeps[ii]->getIntermedFreqHz() != Radx::missingMetaDouble) {
      haveIF = true;
      break;
    }
  }
  if (haveIF) {
    iret |= _file.addMetaVar(_intermedFreqHzVar, INTERMED_FREQ_HZ,
                             "", INTERMED_FREQ_HZ_LONG, 
                             nc3Float, _sweepDim, HZ);
  }
  
  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_addSweepVariables");
    return -1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////
// add variables for calibration info

int NcfRadxFile::_addCalibVariables()
{
  
  if (_writeVol->getRcalibs().size() < 1) {
    return 0;
  }

  if (_verbose) {
    cerr << "NcfRadxFile::_addCalibVariables()" << endl;
  }
    
  int iret = 0;

  iret |= _file.addMetaVar(_rCalTimeVar, R_CALIB_TIME,
                           "", R_CALIB_TIME_LONG, 
                           nc3Char, _calDim, _stringLen32Dim, "");
  iret |= _file.addAttr(_rCalTimeVar, META_GROUP, RADAR_CALIBRATION);

  iret |= _addCalVar(_rCalPulseWidthVar, R_CALIB_PULSE_WIDTH,
                     R_CALIB_PULSE_WIDTH_LONG, SECONDS);

  iret |= _addCalVar(_rCalXmitPowerHVar, R_CALIB_XMIT_POWER_H,
                     R_CALIB_XMIT_POWER_H_LONG, DBM);
  iret |= _addCalVar(_rCalXmitPowerVVar, R_CALIB_XMIT_POWER_V,
                     R_CALIB_XMIT_POWER_V_LONG, DBM);

  iret |= _addCalVar(_rCalTwoWayWaveguideLossHVar, R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H,
                     R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H_LONG, DB);
  iret |= _addCalVar(_rCalTwoWayWaveguideLossVVar, R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V,
                     R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V_LONG, DB);

  iret |= _addCalVar(_rCalTwoWayRadomeLossHVar, R_CALIB_TWO_WAY_RADOME_LOSS_H,
                     R_CALIB_TWO_WAY_RADOME_LOSS_H_LONG, DB);
  iret |= _addCalVar(_rCalTwoWayRadomeLossVVar, R_CALIB_TWO_WAY_RADOME_LOSS_V,
                     R_CALIB_TWO_WAY_RADOME_LOSS_V_LONG, DB);

  iret |= _addCalVar(_rCalReceiverMismatchLossVar, R_CALIB_RECEIVER_MISMATCH_LOSS,
                     R_CALIB_RECEIVER_MISMATCH_LOSS_LONG, DB);

  iret |= _addCalVar(_rCalKSquaredWaterVar, R_CALIB_K_SQUARED_WATER,
                     R_CALIB_K_SQUARED_WATER);

  iret |= _addCalVar(_rCalRadarConstHVar, R_CALIB_RADAR_CONSTANT_H,
                     R_CALIB_RADAR_CONSTANT_H_LONG, DB);
  iret |= _addCalVar(_rCalRadarConstVVar, R_CALIB_RADAR_CONSTANT_V,
                     R_CALIB_RADAR_CONSTANT_V_LONG, DB);

  iret |= _addCalVar(_rCalAntennaGainHVar, R_CALIB_ANTENNA_GAIN_H,
                     R_CALIB_ANTENNA_GAIN_H_LONG, DB);
  iret |= _addCalVar(_rCalAntennaGainVVar, R_CALIB_ANTENNA_GAIN_V,
                     R_CALIB_ANTENNA_GAIN_V_LONG, DB);

  iret |= _addCalVar(_rCalNoiseHcVar, R_CALIB_NOISE_HC,
                     R_CALIB_NOISE_HC_LONG, DBM);
  iret |= _addCalVar(_rCalNoiseVcVar, R_CALIB_NOISE_VC,
                     R_CALIB_NOISE_VC_LONG, DBM);
  iret |= _addCalVar(_rCalNoiseHxVar, R_CALIB_NOISE_HX,
                     R_CALIB_NOISE_HX_LONG, DBM);
  iret |= _addCalVar(_rCalNoiseVxVar, R_CALIB_NOISE_VX,
                     R_CALIB_NOISE_VX_LONG, DBM);

  iret |= _addCalVar(_rCalI0HcVar, R_CALIB_I0_DBM_HC,
                     R_CALIB_I0_DBM_HC, DBM);
  iret |= _addCalVar(_rCalI0VcVar, R_CALIB_I0_DBM_VC,
                     R_CALIB_I0_DBM_VC, DBM);
  iret |= _addCalVar(_rCalI0HxVar, R_CALIB_I0_DBM_HX,
                     R_CALIB_I0_DBM_HX, DBM);
  iret |= _addCalVar(_rCalI0VxVar, R_CALIB_I0_DBM_VX,
                     R_CALIB_I0_DBM_VX, DBM);

  iret |= _addCalVar(_rCalReceiverGainHcVar, R_CALIB_RECEIVER_GAIN_HC,
                     R_CALIB_RECEIVER_GAIN_HC_LONG, DB);
  iret |= _addCalVar(_rCalReceiverGainVcVar, R_CALIB_RECEIVER_GAIN_VC,
                     R_CALIB_RECEIVER_GAIN_VC_LONG, DB);
  iret |= _addCalVar(_rCalReceiverGainHxVar, R_CALIB_RECEIVER_GAIN_HX,
                     R_CALIB_RECEIVER_GAIN_HX_LONG, DB);
  iret |= _addCalVar(_rCalReceiverGainVxVar, R_CALIB_RECEIVER_GAIN_VX,
                     R_CALIB_RECEIVER_GAIN_VX_LONG, DB);

  iret |= _addCalVar(_rCalReceiverSlopeHcVar, R_CALIB_RECEIVER_SLOPE_HC,
                     R_CALIB_RECEIVER_SLOPE_HC_LONG);
  iret |= _addCalVar(_rCalReceiverSlopeVcVar, R_CALIB_RECEIVER_SLOPE_VC,
                     R_CALIB_RECEIVER_SLOPE_VC_LONG);
  iret |= _addCalVar(_rCalReceiverSlopeHxVar, R_CALIB_RECEIVER_SLOPE_HX,
                     R_CALIB_RECEIVER_SLOPE_HX_LONG);
  iret |= _addCalVar(_rCalReceiverSlopeVxVar, R_CALIB_RECEIVER_SLOPE_VX,
                     R_CALIB_RECEIVER_SLOPE_VX_LONG);

  iret |= _addCalVar(_rCalDynamicRangeHcVar, R_CALIB_DYNAMIC_RANGE_DB_HC,
                     R_CALIB_DYNAMIC_RANGE_DB_HC, DB);
  iret |= _addCalVar(_rCalDynamicRangeVcVar, R_CALIB_DYNAMIC_RANGE_DB_VC,
                     R_CALIB_DYNAMIC_RANGE_DB_VC, DB);
  iret |= _addCalVar(_rCalDynamicRangeHxVar, R_CALIB_DYNAMIC_RANGE_DB_HX,
                     R_CALIB_DYNAMIC_RANGE_DB_HX, DB);
  iret |= _addCalVar(_rCalDynamicRangeVxVar, R_CALIB_DYNAMIC_RANGE_DB_VX,
                     R_CALIB_DYNAMIC_RANGE_DB_VX, DB);

  iret |= _addCalVar(_rCalBaseDbz1kmHcVar, R_CALIB_BASE_DBZ_1KM_HC,
                     R_CALIB_BASE_DBZ_1KM_HC_LONG, DBZ);
  iret |= _addCalVar(_rCalBaseDbz1kmVcVar, R_CALIB_BASE_DBZ_1KM_VC,
                     R_CALIB_BASE_DBZ_1KM_VC_LONG, DBZ);
  iret |= _addCalVar(_rCalBaseDbz1kmHxVar, R_CALIB_BASE_DBZ_1KM_HX,
                     R_CALIB_BASE_DBZ_1KM_HX_LONG, DBZ);
  iret |= _addCalVar(_rCalBaseDbz1kmVxVar, R_CALIB_BASE_DBZ_1KM_VX,
                     R_CALIB_BASE_DBZ_1KM_VX_LONG, DBZ);

  iret |= _addCalVar(_rCalSunPowerHcVar, R_CALIB_SUN_POWER_HC,
                     R_CALIB_SUN_POWER_HC_LONG, DBM);
  iret |= _addCalVar(_rCalSunPowerVcVar, R_CALIB_SUN_POWER_VC,
                     R_CALIB_SUN_POWER_VC_LONG, DBM);
  iret |= _addCalVar(_rCalSunPowerHxVar, R_CALIB_SUN_POWER_HX,
                     R_CALIB_SUN_POWER_HX_LONG, DBM);
  iret |= _addCalVar(_rCalSunPowerVxVar, R_CALIB_SUN_POWER_VX,
                     R_CALIB_SUN_POWER_VX_LONG, DBM);

  iret |= _addCalVar(_rCalNoiseSourcePowerHVar, R_CALIB_NOISE_SOURCE_POWER_H,
                     R_CALIB_NOISE_SOURCE_POWER_H_LONG, DBM);
  iret |= _addCalVar(_rCalNoiseSourcePowerVVar, R_CALIB_NOISE_SOURCE_POWER_V,
                     R_CALIB_NOISE_SOURCE_POWER_V_LONG, DBM);

  iret |= _addCalVar(_rCalPowerMeasLossHVar, R_CALIB_POWER_MEASURE_LOSS_H,
                     R_CALIB_POWER_MEASURE_LOSS_H_LONG, DB);
  iret |= _addCalVar(_rCalPowerMeasLossVVar, R_CALIB_POWER_MEASURE_LOSS_V,
                     R_CALIB_POWER_MEASURE_LOSS_V_LONG, DB);

  iret |= _addCalVar(_rCalCouplerForwardLossHVar, R_CALIB_COUPLER_FORWARD_LOSS_H,
                     R_CALIB_COUPLER_FORWARD_LOSS_H_LONG, DB);
  iret |= _addCalVar(_rCalCouplerForwardLossVVar, R_CALIB_COUPLER_FORWARD_LOSS_V,
                     R_CALIB_COUPLER_FORWARD_LOSS_V_LONG, DB);

  iret |= _addCalVar(_rCalDbzCorrectionVar, R_CALIB_DBZ_CORRECTION,
                     R_CALIB_DBZ_CORRECTION_LONG, DB);
  iret |= _addCalVar(_rCalZdrCorrectionVar, R_CALIB_ZDR_CORRECTION,
                     R_CALIB_ZDR_CORRECTION_LONG, DB);
  iret |= _addCalVar(_rCalLdrCorrectionHVar, R_CALIB_LDR_CORRECTION_H,
                     R_CALIB_LDR_CORRECTION_H_LONG, DB);
  iret |= _addCalVar(_rCalLdrCorrectionVVar, R_CALIB_LDR_CORRECTION_V,
                     R_CALIB_LDR_CORRECTION_V_LONG, DB);

  iret |= _addCalVar(_rCalSystemPhidpVar, R_CALIB_SYSTEM_PHIDP,
                     R_CALIB_SYSTEM_PHIDP_LONG, DEGREES);

  iret |= _addCalVar(_rCalTestPowerHVar, R_CALIB_TEST_POWER_H,
                     R_CALIB_TEST_POWER_H_LONG, DBM);
  iret |= _addCalVar(_rCalTestPowerVVar, R_CALIB_TEST_POWER_V,
                     R_CALIB_TEST_POWER_V_LONG, DBM);

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_addCalibVariables");
    return -1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////
// add variables for angles

int NcfRadxFile::_addRayVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_addRayVariables()" << endl;
  }

  int iret = 0;
  
  if (_nGatesVary) {
    iret |= _file.addMetaVar(_rayNGatesVar, RAY_N_GATES,
                             "", "number_of_gates", nc3Int, _timeDim);
    iret |= _file.addMetaVar(_rayStartIndexVar, RAY_START_INDEX,
                             "", "array_index_to_start_of_ray", nc3Int, _timeDim);
  }
  
  iret |= _file.addMetaVar(_rayStartRangeVar, RAY_START_RANGE,
                           "", "start_range_for_ray", nc3Float, _timeDim, METERS);
  iret |= _file.addAttr(_rayStartRangeVar, UNITS, METERS);

  iret |= _file.addMetaVar(_rayGateSpacingVar, RAY_GATE_SPACING,
                           "", "gate_spacing_for_ray", nc3Float, _timeDim, METERS);
  iret |= _file.addAttr(_rayGateSpacingVar, UNITS, METERS);

  iret |= _file.addMetaVar(_azimuthVar, AZIMUTH,
                           "", AZIMUTH_LONG, nc3Float, _timeDim, DEGREES);

  iret |= _file.addMetaVar(_elevationVar, ELEVATION,
                           "", ELEVATION_LONG, nc3Float, _timeDim, DEGREES);
  iret |= _file.addAttr(_elevationVar, POSITIVE, UP);
  
  iret |= _file.addMetaVar(_pulseWidthVar, PULSE_WIDTH,
                           "", PULSE_WIDTH_LONG, nc3Float, _timeDim, SECONDS);
  iret |= _file.addAttr(_pulseWidthVar, META_GROUP, INSTRUMENT_PARAMETERS);

  iret |= _file.addMetaVar(_prtVar, PRT,
                           "", PRT_LONG, nc3Float, _timeDim, SECONDS);
  iret |= _file.addAttr(_prtVar, META_GROUP, INSTRUMENT_PARAMETERS);
  
  iret |= _file.addMetaVar(_prtRatioVar, PRT_RATIO,
                           "", PRT_RATIO_LONG,
                           nc3Float, _timeDim, SECONDS);
  iret |= _file.addAttr(_prtRatioVar, META_GROUP, INSTRUMENT_PARAMETERS);

  iret |= _file.addMetaVar(_nyquistVar, NYQUIST_VELOCITY,
                           "", NYQUIST_VELOCITY_LONG, nc3Float, _timeDim, METERS_PER_SECOND);
  iret |= _file.addAttr(_nyquistVar, META_GROUP, INSTRUMENT_PARAMETERS);

  iret |= _file.addMetaVar(_unambigRangeVar, UNAMBIGUOUS_RANGE,
                           "", UNAMBIGUOUS_RANGE_LONG, nc3Float, _timeDim, METERS);
  iret |= _file.addAttr(_unambigRangeVar, META_GROUP, INSTRUMENT_PARAMETERS);

  iret |= _file.addMetaVar(_antennaTransitionVar, ANTENNA_TRANSITION,
                           "", ANTENNA_TRANSITION_LONG, nc3Byte, _timeDim, "");
  iret |= _file.addAttr(_antennaTransitionVar, COMMENT,
                        "1 if antenna is in transition, 0 otherwise");

  if (_georefsActive) {
    iret |= _file.addMetaVar(_georefsAppliedVar, GEOREFS_APPLIED,
                             "", "georefs_have_been_applied_to_ray", nc3Byte, _timeDim, "");
    iret |= _file.addAttr(_georefsAppliedVar, COMMENT,
                          "1 if georefs have been applied, 0 otherwise");
  }

  iret |= _file.addMetaVar(_nSamplesVar, N_SAMPLES,
                           "", N_SAMPLES_LONG, nc3Int, _timeDim, "");
  iret |= _file.addAttr(_nSamplesVar, META_GROUP, INSTRUMENT_PARAMETERS);

  if (_writeVol->getRcalibs().size() > 0) {
    iret |= _file.addMetaVar(_calIndexVar, R_CALIB_INDEX,
                             "", R_CALIB_INDEX_LONG, nc3Int, _timeDim, "");
    iret |= _file.addAttr(_calIndexVar, META_GROUP, RADAR_CALIBRATION);
    iret |= _file.addAttr(_calIndexVar, COMMENT,
                          "This is the index for the calibration which applies to this ray");
  }

  iret |= _file.addMetaVar(_xmitPowerHVar,
                           RADAR_MEASURED_TRANSMIT_POWER_H,
                           "", 
                           RADAR_MEASURED_TRANSMIT_POWER_H_LONG,
                           nc3Float, _timeDim, DBM);
  iret |= _file.addAttr(_xmitPowerHVar, META_GROUP, RADAR_PARAMETERS);

  iret |= _file.addMetaVar(_xmitPowerVVar,
                           RADAR_MEASURED_TRANSMIT_POWER_V,
                           "", 
                           RADAR_MEASURED_TRANSMIT_POWER_V_LONG, 
                           nc3Float, _timeDim, DBM);
  iret |= _file.addAttr(_xmitPowerVVar, META_GROUP, RADAR_PARAMETERS);

  iret |= _file.addMetaVar(_scanRateVar, SCAN_RATE,
                           "", SCAN_RATE_LONG, 
                           nc3Float, _timeDim, DEGREES_PER_SECOND);
  iret |= _file.addAttr(_scanRateVar, META_GROUP, INSTRUMENT_PARAMETERS);

  _setEstNoiseAvailFlags();

  if (_estNoiseAvailHc) {
    iret |= _file.addMetaVar(_estNoiseDbmHcVar,
                             RADAR_ESTIMATED_NOISE_DBM_HC,
                             "", 
                             RADAR_ESTIMATED_NOISE_DBM_HC_LONG,
                             nc3Float, _timeDim, DBM);
    iret |= _file.addAttr(_estNoiseDbmHcVar, META_GROUP, RADAR_PARAMETERS);
  }

  if (_estNoiseAvailVc) {
    iret |= _file.addMetaVar(_estNoiseDbmVcVar,
                             RADAR_ESTIMATED_NOISE_DBM_VC,
                             "", 
                             RADAR_ESTIMATED_NOISE_DBM_VC_LONG,
                             nc3Float, _timeDim, DBM);
    iret |= _file.addAttr(_estNoiseDbmVcVar, META_GROUP, RADAR_PARAMETERS);
  }

  if (_estNoiseAvailHx) {
    iret |= _file.addMetaVar(_estNoiseDbmHxVar,
                             RADAR_ESTIMATED_NOISE_DBM_HX,
                             "", 
                             RADAR_ESTIMATED_NOISE_DBM_HX_LONG,
                             nc3Float, _timeDim, DBM);
    iret |= _file.addAttr(_estNoiseDbmHxVar, META_GROUP, RADAR_PARAMETERS);
  }

  if (_estNoiseAvailVx) {
    iret |= _file.addMetaVar(_estNoiseDbmVxVar,
                             RADAR_ESTIMATED_NOISE_DBM_VX,
                             "", 
                             RADAR_ESTIMATED_NOISE_DBM_VX_LONG,
                             nc3Float, _timeDim, DBM);
    iret |= _file.addAttr(_estNoiseDbmVxVar, META_GROUP, RADAR_PARAMETERS);
  }

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_addRayVariables");
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////////
// set flags to indicate whether estimate noise
// is available

void NcfRadxFile::_setEstNoiseAvailFlags()

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

/////////////////////////////////////////////////
// add variables for ray georeference, if active

int NcfRadxFile::_addGeorefVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_addGeorefVariables()" << endl;
  }

  if (!_georefsActive) {
    return 0;
  }
  
  int iret = 0;

  // we always add the postion variables
  
  iret |= _file.addMetaVar(_georefTimeVar, GEOREF_TIME,
                           "", GEOREF_TIME_LONG, nc3Double,
                           _timeDim, SECONDS);

  iret |= _file.addMetaVar(_georefUnitNumVar, GEOREF_UNIT_NUM,
                           "", GEOREF_UNIT_NUM_LONG, nc3Int,
                           _timeDim);

  iret |= _file.addMetaVar(_georefUnitIdVar, GEOREF_UNIT_ID,
                           "", GEOREF_UNIT_ID_LONG, nc3Int,
                           _timeDim);

  iret |= _file.addMetaVar(_latitudeVar, LATITUDE,
                           "", LATITUDE_LONG, nc3Double,
                           _timeDim, DEGREES_NORTH);

  iret |= _file.addMetaVar(_longitudeVar, LONGITUDE,
                           "", LONGITUDE_LONG, nc3Double,
                           _timeDim, DEGREES_EAST);

  iret |= _file.addMetaVar(_altitudeVar, ALTITUDE,
                           "", ALTITUDE_LONG, nc3Double,
                           _timeDim, METERS);
  iret |= _file.addAttr(_altitudeVar, POSITIVE, UP);

  iret |= _file.addMetaVar(_altitudeAglVar, ALTITUDE_AGL,
                           "", ALTITUDE_AGL_LONG, nc3Double,
                           _timeDim, METERS);
  iret |= _file.addAttr(_altitudeAglVar, POSITIVE, UP);

  // conditionally add the georeference variables

  if (_geoCount.getHeading() > 0) {
    _file.addMetaVar(HEADING, "", HEADING_LONG, nc3Float,
                     _timeDim, DEGREES);
  }

  if (_geoCount.getTrack() > 0) {
    _file.addMetaVar(TRACK, "", TRACK_LONG, nc3Float,
                     _timeDim, DEGREES);
  }
  
  if (_geoCount.getRoll() > 0) {
    _file.addMetaVar(ROLL, "", ROLL_LONG, nc3Float,
                     _timeDim, DEGREES);
  }

  if (_geoCount.getPitch() > 0) {
    _file.addMetaVar(PITCH, "", PITCH_LONG, nc3Float,
                     _timeDim, DEGREES);
  }

  if (_geoCount.getDrift() > 0) {
    _file.addMetaVar(DRIFT, "", DRIFT_LONG, nc3Float,
                     _timeDim, DEGREES);
  }

  if (_geoCount.getRotation() > 0) {
    _file.addMetaVar(ROTATION, "", ROTATION_LONG, nc3Float,
                     _timeDim, DEGREES);
  }
  
  if (_geoCount.getTilt() > 0) {
    _file.addMetaVar(TILT, "", TILT_LONG, nc3Float,
                     _timeDim, DEGREES);
  }

  if (_geoCount.getEwVelocity() > 0) {
    Nc3Var *var = _file.addMetaVar(EASTWARD_VELOCITY, "", 
                                  EASTWARD_VELOCITY_LONG, nc3Float,
                                  _timeDim, METERS_PER_SECOND);
    if (var != NULL) {
      _file.addAttr(var, META_GROUP, PLATFORM_VELOCITY);
    }
  }
  
  if (_geoCount.getNsVelocity() > 0) {
    Nc3Var *var = _file.addMetaVar(NORTHWARD_VELOCITY,
                                  "", NORTHWARD_VELOCITY_LONG, nc3Float,
                                  _timeDim, METERS_PER_SECOND);
    if (var != NULL) {
      _file.addAttr(var, META_GROUP, PLATFORM_VELOCITY);
    }
  }

  if (_geoCount.getVertVelocity() > 0) {
    Nc3Var *var = _file.addMetaVar(VERTICAL_VELOCITY,
                                  "", VERTICAL_VELOCITY_LONG, nc3Float,
                                  _timeDim, METERS_PER_SECOND);
    if (var != NULL) {
      _file.addAttr(var, META_GROUP, PLATFORM_VELOCITY);
    }
  }
  
  if (_geoCount.getEwWind() > 0) {
    Nc3Var *var = _file.addMetaVar(EASTWARD_WIND,
                                  "", EASTWARD_WIND_LONG, nc3Float,
                                  _timeDim, METERS_PER_SECOND);
    if (var != NULL) {
      _file.addAttr(var, META_GROUP, PLATFORM_VELOCITY);
    }
  }

  if (_geoCount.getNsWind() > 0) {
    Nc3Var *var = _file.addMetaVar(NORTHWARD_WIND,
                                  "", NORTHWARD_WIND_LONG, nc3Float,
                                  _timeDim, METERS_PER_SECOND);
    if (var != NULL) {
      _file.addAttr(var, META_GROUP, PLATFORM_VELOCITY);
    }
  }

  if (_geoCount.getVertWind() > 0) {
    Nc3Var *var = _file.addMetaVar(VERTICAL_WIND,
                                  "", VERTICAL_WIND_LONG, nc3Float,
                                  _timeDim, METERS_PER_SECOND);
    if (var != NULL) {
      _file.addAttr(var, META_GROUP, PLATFORM_VELOCITY);
    }
  }

  if (_geoCount.getHeadingRate() > 0) {
    Nc3Var *var = _file.addMetaVar(HEADING_CHANGE_RATE,
                                  "", HEADING_CHANGE_RATE_LONG, nc3Float,
                                  _timeDim, DEGREES_PER_SECOND);
    if (var != NULL) {
      _file.addAttr(var, META_GROUP, PLATFORM_VELOCITY);
    }
  }
  
  if (_geoCount.getPitchRate() > 0) {
    Nc3Var *var = _file.addMetaVar(PITCH_CHANGE_RATE,
                                  "", PITCH_CHANGE_RATE_LONG, nc3Float,
                                  _timeDim, DEGREES_PER_SECOND);
    if (var != NULL) {
      _file.addAttr(var, META_GROUP, PLATFORM_VELOCITY);
    }
  }
    
  if (_geoCount.getRollRate() > 0) {
    Nc3Var *var = _file.addMetaVar(ROLL_CHANGE_RATE,
                                  "", ROLL_CHANGE_RATE_LONG, nc3Float,
                                  _timeDim, DEGREES_PER_SECOND);
    if (var != NULL) {
      _file.addAttr(var, META_GROUP, PLATFORM_VELOCITY);
    }
  }
    
  if (_geoCount.getDriveAngle1() > 0) {
    _file.addMetaVar(DRIVE_ANGLE_1, "", "antenna_drive_angle_1", nc3Float,
                     _timeDim, DEGREES);
  }

  if (_geoCount.getDriveAngle2() > 0) {
    _file.addMetaVar(DRIVE_ANGLE_2, "", "antenna_drive_angle_2", nc3Float,
                     _timeDim, DEGREES);
  }
  
  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_addGeorefVariables");
    return -1;
  } else {
    return 0;
  }

}

int NcfRadxFile::_addCalVar(Nc3Var* &var, const string &name,
                            const string &standardName,
                            const string &units /* = "" */)
{
  
  var = _file.getNc3File()->add_var(name.c_str(), nc3Float, _calDim);
  if (var == NULL) {
    _addErrStr("ERROR - NcfRadxFile::_addCalVar");
    _addErrStr("  Cannot add calib var, name: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  if (standardName.length() > 0) {
    if (_file.addAttr(var, LONG_NAME, standardName)) {
      return -1;
    }
  }

  if (_file.addAttr(var, UNITS, units)) {
    return -1;
  }
  
  if (_file.addAttr(var, META_GROUP, RADAR_CALIBRATION)) {
    return -1;
  }
  
  if (_file.addAttr(var, FILL_VALUE, Radx::missingMetaFloat)) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// write coordinate variables

int NcfRadxFile::_writeCoordinateVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_writeCoordinateVariables()" << endl;
  }

  // time

  int nRays = _writeVol->getNRays();
  RadxArray<double> dtime_;
  double *dtime = dtime_.alloc(nRays);

  double startTimeSecs = _writeVol->getStartTimeSecs();
  const vector<RadxRay *> &rays = _writeVol->getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay &ray = *rays[ii];
    double dsecs = ray.getTimeSecs() - startTimeSecs;
    dsecs += ray.getNanoSecs() / 1.0e9;
    dtime[ii] = dsecs;
  }
  if (!_timeVar->put(dtime, nRays)) {
    _addErrStr("ERROR - NcfRadxFile::_writeCoordinateVariables");
    _addErrStr("  Cannot write time var");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  // range

  if (_gateGeomVaries) {
  
    int maxNGates = _writeVol->getMaxNGates();
    RadxArray<float> rangeMeters_;
    float *rangeMeters = rangeMeters_.alloc(_writeVol->getNRays() * maxNGates);
    
    const vector<RadxRay *> &rays = _writeVol->getRays();
    int index = 0;
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxRay &ray = *rays[ii];
      double startRangeKm = ray.getStartRangeKm();
      double gateSpacingKm = ray.getGateSpacingKm();
      double rangeKm = startRangeKm;
      for (int jj = 0; jj < maxNGates; jj++, rangeKm += gateSpacingKm, index++) {
        rangeMeters[index] = rangeKm * 1000.0;
      }
    }

    if (!_rangeVar->put(rangeMeters, _writeVol->getNRays(), _writeVol->getMaxNGates())) {
      _addErrStr("ERROR - NcfRadxFile::_writeCoordinateVariables");
      return -1;
    }
  
  } else {
    
    int maxNGates = _writeVol->getMaxNGates();
    RadxArray<float> rangeMeters_;
    float *rangeMeters = rangeMeters_.alloc(maxNGates);
    double startRangeKm = _writeVol->getStartRangeKm();
    double gateSpacingKm = _writeVol->getGateSpacingKm();
    double rangeKm = startRangeKm;
    for (int ii = 0; ii < maxNGates; ii++, rangeKm += gateSpacingKm) {
      rangeMeters[ii] = rangeKm * 1000.0;
    }

    if (_file.writeVar(_rangeVar, _rangeDim, rangeMeters)) {
      _addErrStr("ERROR - NcfRadxFile::_writeCoordinateVariables");
      return -1;
    }
  
  }

  return 0;

}

////////////////////////////////////////////////
// write scalar variables

int NcfRadxFile::_writeScalarVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_writeScalarVariables()" << endl;
  }
  
  // volume number
  
  int volNum = _writeVol->getVolumeNumber();
  if (!_volumeNumberVar->put(&volNum, 1)) {
    _addErrStr("ERROR - NcfRadxFile::_writeScalarVariables");
    _addErrStr("  Cannot write volumeNumber");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  int iret = 0;
  iret |= _file.writeVar(_volumeNumberVar, volNum);

  // instrument type

  String32_t strn;
  memset(strn, 0, sizeof(String32_t));
  strncpy(strn,
          Radx::instrumentTypeToStr(_writeVol->getInstrumentType()).c_str(),
          sizeof(String32_t) - 1);
  iret |= _file.writeStringVar(_instrumentTypeVar, strn);

  // platform type

  memset(strn, 0, sizeof(String32_t));
  strncpy(strn,
          Radx::platformTypeToStr(_writeVol->getPlatformType()).c_str(),
          sizeof(String32_t) - 1);
  iret |= _file.writeStringVar(_platformTypeVar, strn);

  // primary axis

  memset(strn, 0, sizeof(String32_t));
  strncpy(strn,
          Radx::primaryAxisToStr(_writeVol->getPrimaryAxis()).c_str(),
          sizeof(String32_t) - 1);
  iret |= _file.writeStringVar(_primaryAxisVar, strn);

  // status xml
  
  size_t xmlLen =
    _writeVol->getStatusXml().size() + 1; // includes trailing NULL
  RadxArray<char> xmlStr_;
  char *xmlStr = xmlStr_.alloc(xmlLen);
  strncpy(xmlStr, _writeVol->getStatusXml().c_str(), xmlLen);
  iret |= _file.writeStringVar(_statusXmlVar, xmlStr);

  // start time
  
  RadxTime startTime(_writeVol->getStartTimeSecs());
  memset(strn, 0, sizeof(String32_t));
  strncpy(strn, startTime.getW3cStr().c_str(),
          sizeof(String32_t) - 1);
  iret |= _file.writeStringVar(_startTimeVar, strn);

  // end time
  
  RadxTime endTime(_writeVol->getEndTimeSecs());
  memset(strn, 0, sizeof(String32_t));
  strncpy(strn, endTime.getW3cStr().c_str(),
          sizeof(String32_t) - 1);
  iret |= _file.writeStringVar(_endTimeVar, strn);

  if (_writeVol->getInstrumentType() == Radx::INSTRUMENT_TYPE_RADAR) {
    // radar params
    iret |= _file.writeVar(_radarAntennaGainHVar,
                           (float) _writeVol->getRadarAntennaGainDbH());
    iret |= _file.writeVar(_radarAntennaGainVVar,
                           (float) _writeVol->getRadarAntennaGainDbV());
    iret |= _file.writeVar(_radarBeamWidthHVar,
                           (float) _writeVol->getRadarBeamWidthDegH());
    iret |= _file.writeVar(_radarBeamWidthVVar,
                           (float) _writeVol->getRadarBeamWidthDegV());
    double bandwidthHz = _writeVol->getRadarReceiverBandwidthMhz();
    if (_writeVol->getRadarReceiverBandwidthMhz() > 0) {
      bandwidthHz = _writeVol->getRadarReceiverBandwidthMhz() * 1.0e6; // non-missing
    }
    iret |= _file.writeVar(_radarRxBandwidthVar, (float) bandwidthHz);
  } else {
    // lidar params
    iret |= _file.writeVar(_lidarConstantVar,
                           (float) _writeVol->getLidarConstant());
    iret |= _file.writeVar(_lidarPulseEnergyJVar,
                           (float) _writeVol->getLidarPulseEnergyJ());
    iret |= _file.writeVar(_lidarPeakPowerWVar,
                           (float) _writeVol->getLidarPeakPowerW());
    iret |= _file.writeVar(_lidarApertureDiamCmVar,
                           (float) _writeVol->getLidarApertureDiamCm());
    iret |= _file.writeVar(_lidarApertureEfficiencyVar,
                           (float) _writeVol->getLidarApertureEfficiency());
    iret |= _file.writeVar(_lidarFieldOfViewMradVar,
                           (float) _writeVol->getLidarFieldOfViewMrad());
    iret |= _file.writeVar(_lidarBeamDivergenceMradVar,
                           (float) _writeVol->getLidarBeamDivergenceMrad());
  }

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////
// write correction variables

int NcfRadxFile::_writeCorrectionVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_writeCorrectionVariables()" << endl;
  }

  const RadxCfactors *cfac = _writeVol->getCfactors();
  
  int iret = 0;
  iret |= _file.writeVar(_azimuthCorrVar, (float) cfac->getAzimuthCorr());
  iret |= _file.writeVar(_elevationCorrVar, (float) cfac->getElevationCorr());
  iret |= _file.writeVar(_rangeCorrVar, (float) cfac->getRangeCorr());
  iret |= _file.writeVar(_longitudeCorrVar, (float) cfac->getLongitudeCorr());
  iret |= _file.writeVar(_latitudeCorrVar, (float) cfac->getLatitudeCorr());
  iret |= _file.writeVar(_pressureAltCorrVar, (float) cfac->getPressureAltCorr());
  iret |= _file.writeVar(_altitudeCorrVar, (float) cfac->getAltitudeCorr());
  iret |= _file.writeVar(_ewVelCorrVar, (float) cfac->getEwVelCorr());
  iret |= _file.writeVar(_nsVelCorrVar, (float) cfac->getNsVelCorr());
  iret |= _file.writeVar(_vertVelCorrVar, (float) cfac->getVertVelCorr());
  iret |= _file.writeVar(_headingCorrVar, (float) cfac->getHeadingCorr());
  iret |= _file.writeVar(_rollCorrVar, (float) cfac->getRollCorr());
  iret |= _file.writeVar(_pitchCorrVar, (float) cfac->getPitchCorr());
  iret |= _file.writeVar(_driftCorrVar, (float) cfac->getDriftCorr());
  iret |= _file.writeVar(_rotationCorrVar, (float) cfac->getRotationCorr());
  iret |= _file.writeVar(_tiltCorrVar, (float) cfac->getTiltCorr());

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////
// write projection variables

int NcfRadxFile::_writeProjectionVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_writeProjectionVariables()" << endl;
  }

  if (_georefsActive) {
    // these will be written later, as vectors instead of scalars
    return 0;
  }

  double latitude = _writeVol->getLatitudeDeg();
  if (!_latitudeVar->put(&latitude, 1)) {
    _addErrStr("ERROR - NcfRadxFile::_writeProjectionVariables");
    _addErrStr("  Cannot write latitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  double longitude = _writeVol->getLongitudeDeg();
  if (!_longitudeVar->put(&longitude, 1)) {
    _addErrStr("ERROR - NcfRadxFile::_writeProjectionVariables");
    _addErrStr("  Cannot write longitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  double altitudeM = Radx::missingMetaDouble;
  if (_writeVol->getAltitudeKm() != Radx::missingMetaDouble) {
    altitudeM = _writeVol->getAltitudeKm() * 1000.0;
  }
  if (!_altitudeVar->put(&altitudeM, 1)) {
    _addErrStr("ERROR - NcfRadxFile::_writeProjectionVariables");
    _addErrStr("  Cannot write altitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  double htAglM = Radx::missingMetaDouble;
  if (_writeVol->getSensorHtAglM() != Radx::missingMetaDouble) {
    htAglM = _writeVol->getSensorHtAglM();
  }
  if (htAglM != Radx::missingMetaDouble &&
      !_altitudeAglVar->put(&htAglM, 1)) {
    _addErrStr("ERROR - NcfRadxFile::_writeProjectionVariables");
    _addErrStr("  Cannot write altitude AGL");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// write angle variables

int NcfRadxFile::_writeRayVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_writeRayVariables()" << endl;
  }

  int nRays = _writeVol->getNRays();

  const vector<RadxRay *> &rays = _writeVol->getRays();

  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nRays);

  RadxArray<int> ivals_;
  int *ivals = ivals_.alloc(nRays);

  RadxArray<ncbyte> bvals_;
  ncbyte *bvals = bvals_.alloc(nRays);

  int iret = 0;

  if (_nGatesVary) {

    // number of gates in ray
  
    const vector<size_t> &rayNGates = _writeVol->getRayNGates();
    for (size_t ii = 0; ii < rays.size(); ii++) {
      ivals[ii] = rayNGates[ii];
    }
    iret |= _file.writeVar(_rayNGatesVar, _timeDim, ivals);

    // start index for data in ray
  
    const vector<size_t> &rayStartIndex = _writeVol->getRayStartIndex();
    for (size_t ii = 0; ii < rays.size(); ii++) {
      ivals[ii] = rayStartIndex[ii];
    }
    iret |= _file.writeVar(_rayStartIndexVar, _timeDim, ivals);

  }

  // start range

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] = rays[ii]->getStartRangeKm() * 1000.0;
  }
  iret |= _file.writeVar(_rayStartRangeVar, _timeDim, fvals);

  // gate spacing

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] = rays[ii]->getGateSpacingKm() * 1000.0;
  }
  iret |= _file.writeVar(_rayGateSpacingVar, _timeDim, fvals);

  // azimuth

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] = rays[ii]->getAzimuthDeg();
  }
  iret |= _file.writeVar(_azimuthVar, _timeDim, fvals);

  // elevation

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] =  rays[ii]->getElevationDeg();
  }
  iret |= _file.writeVar(_elevationVar, _timeDim, fvals);

  // pulse width

  for (size_t ii = 0; ii < rays.size(); ii++) {
    if (rays[ii]->getPulseWidthUsec() > 0) {
      fvals[ii] = rays[ii]->getPulseWidthUsec() * 1.0e-6;
    } else {
      fvals[ii] = rays[ii]->getPulseWidthUsec();
    }
  }
  iret |= _file.writeVar(_pulseWidthVar, _timeDim, fvals);

  // prt

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] = rays[ii]->getPrtSec();
  }
  iret |= _file.writeVar(_prtVar, _timeDim, fvals);

  // prt ratio

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] = rays[ii]->getPrtRatio();
  }
  iret |= _file.writeVar(_prtRatioVar, _timeDim, fvals);
  
  // nyquist

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] = rays[ii]->getNyquistMps();
  }
  iret |= _file.writeVar(_nyquistVar, _timeDim, fvals);

  // unambigRange

  for (size_t ii = 0; ii < rays.size(); ii++) {
    if (rays[ii]->getUnambigRangeKm() > 0) {
      fvals[ii] = rays[ii]->getUnambigRangeKm() * 1000.0;
    } else {
      fvals[ii] = rays[ii]->getUnambigRangeKm();
    }
  }
  iret |= _file.writeVar(_unambigRangeVar, _timeDim, fvals);

  // antennaTransition
  
  for (size_t ii = 0; ii < rays.size(); ii++) {
    bvals[ii] = rays[ii]->getAntennaTransition();
  }
  iret |= _file.writeVar(_antennaTransitionVar, _timeDim, bvals);

  // georefs applied?

  if (_georefsActive) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      if (_georefsApplied) {
        bvals[ii] = 1;
      } else {
        bvals[ii] = rays[ii]->getGeorefApplied();
      }
    }
    iret |= _file.writeVar(_georefsAppliedVar, _timeDim, bvals);
  }

  // number of samples in dwell
  
  for (size_t ii = 0; ii < rays.size(); ii++) {
    ivals[ii] = rays[ii]->getNSamples();
  }
  iret |= _file.writeVar(_nSamplesVar, _timeDim, ivals);

  // calibration number

  if (_calIndexVar != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      ivals[ii] = rays[ii]->getCalibIndex();
    }
    iret |= _file.writeVar(_calIndexVar, _timeDim, ivals);
  }

  // transmit power in H and V

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] = _checkMissingFloat(rays[ii]->getMeasXmitPowerDbmH());
  }
  iret |= _file.writeVar(_xmitPowerHVar, _timeDim, fvals);

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] = _checkMissingFloat(rays[ii]->getMeasXmitPowerDbmV());
  }
  iret |= _file.writeVar(_xmitPowerVVar, _timeDim, fvals);

  // scan rate

  for (size_t ii = 0; ii < rays.size(); ii++) {
    fvals[ii] = rays[ii]->getTrueScanRateDegPerSec();
  }
  iret |= _file.writeVar(_scanRateVar, _timeDim, fvals);

  // estimated noise per channel

  if (_estNoiseDbmHcVar) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      fvals[ii] = rays[ii]->getEstimatedNoiseDbmHc();
    }
    iret |= _file.writeVar(_estNoiseDbmHcVar, _timeDim, fvals);
  }

  if (_estNoiseDbmVcVar) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      fvals[ii] = rays[ii]->getEstimatedNoiseDbmVc();
    }
    iret |= _file.writeVar(_estNoiseDbmVcVar, _timeDim, fvals);
  }

  if (_estNoiseDbmHxVar) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      fvals[ii] = rays[ii]->getEstimatedNoiseDbmHx();
    }
    iret |= _file.writeVar(_estNoiseDbmHxVar, _timeDim, fvals);
  }

  if (_estNoiseDbmVxVar) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      fvals[ii] = rays[ii]->getEstimatedNoiseDbmVx();
    }
    iret |= _file.writeVar(_estNoiseDbmVxVar, _timeDim, fvals);
  }

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_writeRayVariables");
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////
// write ray georeference variables, if active

int NcfRadxFile::_writeGeorefVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_writeGeorefVariables()" << endl;
  }

  if (!_georefsActive) {
    return 0;
  }
  
  int nRays = _writeVol->getNRays();
  const vector<RadxRay *> &rays = _writeVol->getRays();

  RadxArray<int> ivals_;
  int *ivals = ivals_.alloc(nRays);

  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nRays);

  RadxArray<double> dvals_;
  double *dvals = dvals_.alloc(nRays);

  int iret = 0;

  // we always write the time and position variables

  // geo time

  RadxTime volStartSecs(_writeVol->getStartTimeSecs());
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxGeoref *geo = rays[ii]->getGeoreference();
    if (geo) {
      RadxTime geoTime(geo->getTimeSecs(), geo->getNanoSecs() * 1.0e-9);
      double dsecs = geoTime - volStartSecs;
      dvals[ii] = dsecs;
    } else {
      dvals[ii] = Radx::missingMetaDouble;
    }
  }
  iret |= _file.writeVar(_georefTimeVar, _timeDim, dvals);

  // latitude

  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxGeoref *geo = rays[ii]->getGeoreference();
    if (geo) {
      dvals[ii] = _checkMissingDouble(geo->getLatitude());
    } else {
      dvals[ii] = Radx::missingMetaDouble;
    }
  }
  iret |= _file.writeVar(_latitudeVar, _timeDim, dvals);

  // longitude

  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxGeoref *geo = rays[ii]->getGeoreference();
    if (geo) {
      dvals[ii] = _checkMissingDouble(geo->getLongitude());
    } else {
      dvals[ii] = Radx::missingMetaDouble;
    }
  }
  iret |= _file.writeVar(_longitudeVar, _timeDim, dvals);

  // altitude msl

  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxGeoref *geo = rays[ii]->getGeoreference();
    if (geo) {
      double altKm = geo->getAltitudeKmMsl();
      if (altKm > -9990) {
        dvals[ii] = altKm * 1000.0; // meters
      } else {
        dvals[ii] = Radx::missingMetaDouble;
      }
    } else {
      dvals[ii] = Radx::missingMetaDouble;
    }
  }
  iret |= _file.writeVar(_altitudeVar, _timeDim, dvals);

  // altitude agl
  
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxGeoref *geo = rays[ii]->getGeoreference();
    if (geo) {
      double altKm = geo->getAltitudeKmAgl();
      if (altKm > -9990) {
        dvals[ii] = altKm * 1000.0; // meters
      } else {
        dvals[ii] = Radx::missingMetaDouble;
      }
    } else {
      dvals[ii] = Radx::missingMetaDouble;
    }
  }
  iret |= _file.writeVar(_altitudeAglVar, _timeDim, dvals);

  // we conditionally add the other georef variables

  Nc3Var *var;
  Nc3File *ncFile = _file.getNc3File();

  // unit num

  if ((var = ncFile->get_var(GEOREF_UNIT_NUM)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        ivals[ii] = (int) geo->getUnitNum();
      } else {
        ivals[ii] = 0;
      }
    }
    iret |= _file.writeVar(var, _timeDim, ivals);
  }

  // unit id

  if ((var = ncFile->get_var(GEOREF_UNIT_ID)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        ivals[ii] = (int) geo->getUnitId();
      } else {
        ivals[ii] = 0;
      }
    }
    iret |= _file.writeVar(var, _timeDim, ivals);
  }

  // ewVelocity


  if ((var = ncFile->get_var(EASTWARD_VELOCITY)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getEwVelocity());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // nsVelocity

  if ((var = ncFile->get_var(NORTHWARD_VELOCITY)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getNsVelocity());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // vertVelocity

  if ((var = ncFile->get_var(VERTICAL_VELOCITY)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getVertVelocity());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // heading

  if ((var = ncFile->get_var(HEADING)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getHeading());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // track

  if ((var = ncFile->get_var(TRACK)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getTrack());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // roll

  if ((var = ncFile->get_var(ROLL)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getRoll());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // pitch

  if ((var = ncFile->get_var(PITCH)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getPitch());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // driftAngle

  if ((var = ncFile->get_var(DRIFT)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getDrift());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // rotation
  
  if ((var = ncFile->get_var(ROTATION)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getRotation());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // tilt
  
  if ((var = ncFile->get_var(TILT)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getTilt());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // ewWind

  if ((var = ncFile->get_var(EASTWARD_WIND)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getEwWind());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // nsWind

  if ((var = ncFile->get_var(NORTHWARD_WIND)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getNsWind());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // verticalWind
  
  if ((var = ncFile->get_var(VERTICAL_WIND)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getVertWind());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // headingRate
  
  if ((var = ncFile->get_var(HEADING_CHANGE_RATE)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getHeadingRate());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // pitchRate

  if ((var = ncFile->get_var(PITCH_CHANGE_RATE)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getPitchRate());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // rollRate

  if ((var = ncFile->get_var(ROLL_CHANGE_RATE)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getRollRate());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // driveAngle1

  if ((var = ncFile->get_var(DRIVE_ANGLE_1)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getDriveAngle1());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // driveAngle2
  
  if ((var = ncFile->get_var(DRIVE_ANGLE_2)) != NULL) {
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxGeoref *geo = rays[ii]->getGeoreference();
      if (geo) {
        fvals[ii] = _checkMissingFloat(geo->getDriveAngle2());
      } else {
        fvals[ii] = Radx::missingMetaFloat;
      }
    }
    iret |= _file.writeVar(var, _timeDim, fvals);
  }

  // clean up

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_writeGeorefVariables");
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////
// write sweep variables

int NcfRadxFile::_writeSweepVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_writeSweepVariables()" << endl;
  }
  
  const vector<RadxSweep *> &sweeps = _writeVol->getSweeps();
  int nSweeps = (int) sweeps.size();

  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nSweeps);

  RadxArray<int> ivals_;
  int *ivals = ivals_.alloc(nSweeps);

  RadxArray<String8_t> strings8_;
  String8_t *strings8 = strings8_.alloc(nSweeps);

  RadxArray<String32_t> strings32_;
  String32_t *strings32 = strings32_.alloc(nSweeps);

  int iret = 0;

  // sweep number
  
  for (int ii = 0; ii < nSweeps; ii++) {
    ivals[ii] = sweeps[ii]->getSweepNumber();
  }
  iret |= _file.writeVar(_sweepNumberVar, _sweepDim, ivals);
  
  // sweep mode
  
  for (int ii = 0; ii < nSweeps; ii++) {
    memset(strings32[ii], 0, sizeof(String32_t));
    Radx::SweepMode_t mode = sweeps[ii]->getSweepMode();
    strncpy(strings32[ii], Radx::sweepModeToStr(mode).c_str(),
            sizeof(String32_t) - 1);
  }
  iret |= _file.writeStringVar(_sweepModeVar, strings32);

  // pol mode
  
  for (int ii = 0; ii < nSweeps; ii++) {
    memset(strings32[ii], 0, sizeof(String32_t));
    Radx::PolarizationMode_t mode = sweeps[ii]->getPolarizationMode();
    strncpy(strings32[ii], Radx::polarizationModeToStr(mode).c_str(),
            sizeof(String32_t) - 1);
  }
  iret |= _file.writeStringVar(_polModeVar, strings32);
  
  // prt mode
  
  for (int ii = 0; ii < nSweeps; ii++) {
    memset(strings32[ii], 0, sizeof(String32_t));
    Radx::PrtMode_t mode = sweeps[ii]->getPrtMode();
    strncpy(strings32[ii], Radx::prtModeToStr(mode).c_str(),
            sizeof(String32_t) - 1);
  }
  iret |= _file.writeStringVar(_prtModeVar, strings32);
  
  // follow mode
  
  for (int ii = 0; ii < nSweeps; ii++) {
    memset(strings32[ii], 0, sizeof(String32_t));
    Radx::FollowMode_t mode = sweeps[ii]->getFollowMode();
    strncpy(strings32[ii], Radx::followModeToStr(mode).c_str(),
            sizeof(String32_t) - 1);
  }
  iret |= _file.writeStringVar(_sweepFollowModeVar, strings32);
  
  // fixed angle
  
  for (int ii = 0; ii < nSweeps; ii++) {
    fvals[ii] = sweeps[ii]->getFixedAngleDeg();
  }
  iret |= _file.writeVar(_sweepFixedAngleVar, _sweepDim, fvals);

  // target scan rate
  
  for (int ii = 0; ii < nSweeps; ii++) {
    fvals[ii] = sweeps[ii]->getTargetScanRateDegPerSec();
  }
  iret |= _file.writeVar(_targetScanRateVar, _sweepDim, fvals);

  // start ray index
  
  for (int ii = 0; ii < nSweeps; ii++) {
    ivals[ii] = sweeps[ii]->getStartRayIndex();
  }
  iret |= _file.writeVar(_sweepStartRayIndexVar, _sweepDim, ivals);

  // end ray index
  
  for (int ii = 0; ii < nSweeps; ii++) {
    ivals[ii] = sweeps[ii]->getEndRayIndex();
  }
  iret |= _file.writeVar(_sweepEndRayIndexVar, _sweepDim, ivals);

  // rays are indexed
  
  for (int ii = 0; ii < nSweeps; ii++) {
    memset(strings8[ii], 0, sizeof(String8_t));
    if (sweeps[ii]->getRaysAreIndexed()) {
      strcpy(strings8[ii], "true");
    } else {
      strcpy(strings8[ii], "false");
    }
  }
  iret |= _file.writeStringVar(_raysAreIndexedVar, strings8);
  
  // ray angle res
  
  for (int ii = 0; ii < nSweeps; ii++) {
    fvals[ii] = sweeps[ii]->getAngleResDeg();
  }
  iret |= _file.writeVar(_rayAngleResVar, _sweepDim, fvals);

  if (_intermedFreqHzVar != NULL) {
    for (int ii = 0; ii < nSweeps; ii++) {
      fvals[ii] = sweeps[ii]->getIntermedFreqHz();
    }
    iret |= _file.writeVar(_intermedFreqHzVar, _sweepDim, fvals);
  }

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_writeSweepVariables");
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////
// write calibration variables

int NcfRadxFile::_writeCalibVariables()
{

  const vector<RadxRcalib *> &calibs = _writeVol->getRcalibs();
  int nCalib = (int) calibs.size();
  if (nCalib < 1) {
    return 0;
  }

  if (_verbose) {
    cerr << "NcfRadxFile::_writeCalibVariables()" << endl;
  }

  // calib time
  
  RadxArray<String32_t> timesStr_;
  String32_t *timesStr = timesStr_.alloc(nCalib);

  for (int ii = 0; ii < nCalib; ii++) {
    const RadxRcalib &calib = *calibs[ii];
    memset(timesStr[ii], 0, sizeof(String32_t));
    RadxTime rtime(calib.getCalibTime());
    strncpy(timesStr[ii], rtime.getW3cStr().c_str(),
            sizeof(String32_t) - 1);
  }
  if (_file.writeStringVar(_rCalTimeVar, timesStr)) {
    _addErrStr("ERROR - NcfRadxFile::_writeCalibVariables");
    return -1;
  }

  // calib params
  
  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nCalib);

  int iret = 0;

  // pulse width

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getPulseWidthUsec() * 1.0e-6;
  }
  iret |= _file.writeVar(_rCalPulseWidthVar, _calDim, fvals);

  // xmit power

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getXmitPowerDbmH();
  }
  iret |= _file.writeVar(_rCalXmitPowerHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getXmitPowerDbmV();
  }
  iret |= _file.writeVar(_rCalXmitPowerVVar, _calDim, fvals);

  // waveguide loss

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTwoWayWaveguideLossDbH();
  }
  iret |= _file.writeVar(_rCalTwoWayWaveguideLossHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTwoWayWaveguideLossDbV();
  }
  iret |= _file.writeVar(_rCalTwoWayWaveguideLossVVar, _calDim, fvals);

  // radome loss

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTwoWayRadomeLossDbH();
  }
  iret |= _file.writeVar(_rCalTwoWayRadomeLossHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTwoWayRadomeLossDbV();
  }
  iret |= _file.writeVar(_rCalTwoWayRadomeLossVVar, _calDim, fvals);

  // rx mismatch loss

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverMismatchLossDb();
  }
  iret |= _file.writeVar(_rCalReceiverMismatchLossVar, _calDim, fvals);

  // k squared water

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getKSquaredWater();
  }
  iret |= _file.writeVar(_rCalKSquaredWaterVar, _calDim, fvals);

  // radar constant

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getRadarConstantH();
  }
  iret |= _file.writeVar(_rCalRadarConstHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getRadarConstantV();
  }
  iret |= _file.writeVar(_rCalRadarConstVVar, _calDim, fvals);

  // antenna gain

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getAntennaGainDbH();
  }
  iret |= _file.writeVar(_rCalAntennaGainHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getAntennaGainDbV();
  }
  iret |= _file.writeVar(_rCalAntennaGainVVar, _calDim, fvals);

  // noise dbm
  
  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseDbmHc();
  }
  iret |= _file.writeVar(_rCalNoiseHcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseDbmHx();
  }
  iret |= _file.writeVar(_rCalNoiseHxVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseDbmVc();
  }
  iret |= _file.writeVar(_rCalNoiseVcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseDbmVx();
  }
  iret |= _file.writeVar(_rCalNoiseVxVar, _calDim, fvals);

  // i0 dbm
  
  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getI0DbmHc();
  }
  iret |= _file.writeVar(_rCalI0HcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getI0DbmHx();
  }
  iret |= _file.writeVar(_rCalI0HxVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getI0DbmVc();
  }
  iret |= _file.writeVar(_rCalI0VcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getI0DbmVx();
  }
  iret |= _file.writeVar(_rCalI0VxVar, _calDim, fvals);

  // receiver gain

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverGainDbHc();
  }
  iret |= _file.writeVar(_rCalReceiverGainHcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverGainDbHx();
  }
  iret |= _file.writeVar(_rCalReceiverGainHxVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverGainDbVc();
  }
  iret |= _file.writeVar(_rCalReceiverGainVcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverGainDbVx();
  }
  iret |= _file.writeVar(_rCalReceiverGainVxVar, _calDim, fvals);

  // receiver slope

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverSlopeDbHc();
  }
  iret |= _file.writeVar(_rCalReceiverSlopeHcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverSlopeDbHx();
  }
  iret |= _file.writeVar(_rCalReceiverSlopeHxVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverSlopeDbVc();
  }
  iret |= _file.writeVar(_rCalReceiverSlopeVcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getReceiverSlopeDbVx();
  }
  iret |= _file.writeVar(_rCalReceiverSlopeVxVar, _calDim, fvals);

  // dynamic range
  
  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getDynamicRangeDbHc();
  }
  iret |= _file.writeVar(_rCalDynamicRangeHcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getDynamicRangeDbHx();
  }
  iret |= _file.writeVar(_rCalDynamicRangeHxVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getDynamicRangeDbVc();
  }
  iret |= _file.writeVar(_rCalDynamicRangeVcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getDynamicRangeDbVx();
  }
  iret |= _file.writeVar(_rCalDynamicRangeVxVar, _calDim, fvals);

  // base dbz 1km

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getBaseDbz1kmHc();
  }
  iret |= _file.writeVar(_rCalBaseDbz1kmHcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getBaseDbz1kmHx();
  }
  iret |= _file.writeVar(_rCalBaseDbz1kmHxVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getBaseDbz1kmVc();
  }
  iret |= _file.writeVar(_rCalBaseDbz1kmVcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getBaseDbz1kmVx();
  }
  iret |= _file.writeVar(_rCalBaseDbz1kmVxVar, _calDim, fvals);

  // sun power

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSunPowerDbmHc();
  }
  iret |= _file.writeVar(_rCalSunPowerHcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSunPowerDbmHx();
  }
  iret |= _file.writeVar(_rCalSunPowerHxVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSunPowerDbmVc();
  }
  iret |= _file.writeVar(_rCalSunPowerVcVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSunPowerDbmVx();
  }
  iret |= _file.writeVar(_rCalSunPowerVxVar, _calDim, fvals);

  // noise source power

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseSourcePowerDbmH();
  }
  iret |= _file.writeVar(_rCalNoiseSourcePowerHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getNoiseSourcePowerDbmV();
  }
  iret |= _file.writeVar(_rCalNoiseSourcePowerVVar, _calDim, fvals);

  // power measurement loss

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getPowerMeasLossDbH();
  }
  iret |= _file.writeVar(_rCalPowerMeasLossHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getPowerMeasLossDbV();
  }
  iret |= _file.writeVar(_rCalPowerMeasLossVVar, _calDim, fvals);

  // coupler loss

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getCouplerForwardLossDbH();
  }
  iret |= _file.writeVar(_rCalCouplerForwardLossHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getCouplerForwardLossDbV();
  }
  iret |= _file.writeVar(_rCalCouplerForwardLossVVar, _calDim, fvals);

  // corrections

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getDbzCorrection();
  }
  iret |= _file.writeVar(_rCalDbzCorrectionVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getZdrCorrectionDb();
  }
  iret |= _file.writeVar(_rCalZdrCorrectionVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getLdrCorrectionDbH();
  }
  iret |= _file.writeVar(_rCalLdrCorrectionHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getLdrCorrectionDbV();
  }
  iret |= _file.writeVar(_rCalLdrCorrectionVVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getSystemPhidpDeg();
  }
  iret |= _file.writeVar(_rCalSystemPhidpVar, _calDim, fvals);

  // test power

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTestPowerDbmH();
  }
  iret |= _file.writeVar(_rCalTestPowerHVar, _calDim, fvals);

  for (int ii = 0; ii < nCalib; ii++) {
    fvals[ii] = calibs[ii]->getTestPowerDbmV();
  }
  iret |= _file.writeVar(_rCalTestPowerVVar, _calDim, fvals);

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_writeCalibVariables");
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////
// write frequency variable

int NcfRadxFile::_writeFrequencyVariable()
{

  const vector<double> &frequency = _writeVol->getFrequencyHz();
  int nFreq = frequency.size();
  if (nFreq < 1) {
    return 0;
  }
  
  RadxArray<float> fvals_;
  float *fvals = fvals_.alloc(nFreq);
  int iret = 0;
  for (int ii = 0; ii < nFreq; ii++) {
    fvals[ii] = frequency[ii];
  }
  iret |= _file.writeVar(_frequencyVar, _frequencyDim, fvals);

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_writeFrequencyVariable");
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////
// write field variables

int NcfRadxFile::_writeFieldVariables()
{

  if (_verbose) {
    cerr << "NcfRadxFile::_writeFieldVariables()" << endl;
  }

  // loop through the list of unique fields names in this volume

  int iret = 0;
  for (size_t ifield = 0; ifield < _uniqueFieldNames.size(); ifield++) {
      
    const string &name = _uniqueFieldNames[ifield];

    // make copy of the field
    
    RadxField *copy = _writeVol->copyField(name);
    if (copy == NULL) {
      if (_debug) {
        cerr << "  ... cannot find field: " << name
             << " .... skipping" << endl;
      }
      continue;
    }
    
    // write it out
    
    // create variable
    Nc3Var *var = _createFieldVar(*copy);
    if (var != NULL) {
      if (_writeFieldVar(var, copy)) {
        iret = -1;
      }
    } else {
      _addErrStr("ERROR - NcfRadxFile::_writeFieldVariables");
      _addErrStr("  Cannot create field: ", name);
      delete copy;
      return -1;
    }
    // free up
    delete copy;
    if (_debug) {
      cerr << "  ... writing field: " << name << endl;
    }
    
  } // ifield

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_writeFieldVariables");
    return -1;
  } else {
    return 0;
  }

}

///////////////////////////////////////////////
// create a field variable
// Returns var ptr on success, NULL on failure
// Adds to errStr as appropriate

Nc3Var *NcfRadxFile::_createFieldVar(const RadxField &field)

{
  
  if (_verbose) {
    cerr << "NcfRadxFile::_createFieldVar()" << endl;
    cerr << "  Adding field: " << field.getName() << endl;
  }

  int iret = 0;

  const string &name = field.getName();
  if (name.size() < 1) {
    _addErrStr("ERROR - NcfRadxFile::_createFieldVar");
    _addErrStr("  Cannot add variable to Nc file object");
    _addErrStr("  Field name is zero-length");
    return NULL;
  }
  
  // check that the field name is CF-netCDF compliant - 
  // i.e must start with a letter
  //   if not, add "nc_" to start of name
  // and must only contain letters, digits and underscores

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

  Nc3Type ncType = _getNc3Type(field.getDataType());

  Nc3Var *var = NULL;

  if (_nGatesVary) {
    var = _file.getNc3File()->add_var(fieldName.c_str(), ncType, _nPointsDim);
  } else {
    var = _file.getNc3File()->add_var(fieldName.c_str(), ncType, _timeDim, _rangeDim);
  }
  
  if (var == NULL) {
    _addErrStr("ERROR - NcfRadxFile::_createFieldVar");
    _addErrStr("  Cannot add variable to Nc file object");
    _addErrStr("  Input field name: ", name);
    _addErrStr("  Output field name: ", fieldName);
    _addErrInt("  Nc3Type: ", ncType);
    _addErrStr("  Time dim name: ", _timeDim->name());
    _addErrInt("  Time dim size: ", _timeDim->size());
    _addErrStr("  Range dim name: ", _rangeDim->name());
    _addErrInt("  Range dim size: ", _rangeDim->size());
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return NULL;
  }

  if (field.getLongName().size() > 0) {
    iret |= _file.addAttr(var, LONG_NAME, field.getLongName());
  }

  // for standard names, we need to check if it is a proposed name
  // for proposed names, use the correct tag

  string stdName = field.getStandardName();
  bool isProposed = false;
  if (_writeProposedStdNameInNcf) {
    isProposed = true;
  }
  string propStr("(proposed)");
  size_t ppos = stdName.find(propStr);
  if (ppos != string::npos) {
    stdName.replace(ppos, propStr.size(), "");
    isProposed = true;
  }
  if (stdName.size() > 0) {
    if (isProposed) {
      iret |= _file.addAttr(var, PROPOSED_STANDARD_NAME, stdName);
    } else {
      iret |= _file.addAttr(var, STANDARD_NAME, stdName);
    }
  }
  iret |= _file.addAttr(var, UNITS, field.getUnits());
  if (field.getLegendXml().size() > 0) {
    iret |= _file.addAttr(var, LEGEND_XML, field.getLegendXml());
  }
  if (field.getThresholdingXml().size() > 0) {
    iret |= _file.addAttr(var, THRESHOLDING_XML, field.getThresholdingXml());
  }
  if (field.getComment().size() > 0) {
    iret |= _file.addAttr(var, COMMENT, field.getComment());
  }
  iret |= _file.addAttr(var, SAMPLING_RATIO, (float) field.getSamplingRatio());
  
  if (field.getFieldFolds()) {
    iret |= _file.addAttr(var, FIELD_FOLDS, "true");
    iret |= _file.addAttr(var, FOLD_LIMIT_LOWER, (float) field.getFoldLimitLower());
    iret |= _file.addAttr(var, FOLD_LIMIT_UPPER, (float) field.getFoldLimitUpper());
  }
  if (field.getIsDiscrete()) {
    iret |= _file.addAttr(var, IS_DISCRETE, "true");
  }

  switch (ncType) {
    case nc3Double: {
      iret |= _file.addAttr(var, FILL_VALUE, (double) field.getMissingFl64());
      break;
    }
    case nc3Float:
    default: {
      iret |= _file.addAttr(var, FILL_VALUE, (float) field.getMissingFl32());
      break;
    }
    case nc3Int: {
      iret |= _file.addAttr(var, FILL_VALUE, (int) field.getMissingSi32());
      iret |= _file.addAttr(var, SCALE_FACTOR, (float) field.getScale());
      iret |= _file.addAttr(var, ADD_OFFSET, (float) field.getOffset());
      break;
    }
    case nc3Short: {
      iret |= _file.addAttr(var, FILL_VALUE, (short) field.getMissingSi16());
      iret |= _file.addAttr(var, SCALE_FACTOR, (float) field.getScale());
      iret |= _file.addAttr(var, ADD_OFFSET, (float) field.getOffset());
      break;
    }
    case nc3Byte: {
      iret |= _file.addAttr(var, FILL_VALUE, (ncbyte) field.getMissingSi08());
      iret |= _file.addAttr(var, SCALE_FACTOR, (float) field.getScale());
      iret |= _file.addAttr(var, ADD_OFFSET, (float) field.getOffset());
      break;
    }
  } // switch

  iret |= _file.addAttr(var, GRID_MAPPING, GRID_MAPPING);
  iret |= _file.addAttr(var, COORDINATES, "time range");

  // set compression
  
  iret |= _setCompression(var);
  
  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_createFieldVar");
    return NULL;
  } else {
    return var;
  }

}

///////////////////////////////////////////////////////////////////////////
// write a field variable
// Returns 0 on success, -1 on failure

int NcfRadxFile::_writeFieldVar(Nc3Var *var, RadxField *field)
  
{
  
  if (_verbose) {
    cerr << "NcfRadxFile::_writeFieldVar()" << endl;
    cerr << "  name: " << var->name() << endl;
  }

  if (var == NULL) {
    _addErrStr("ERROR - NcfRadxFile::_writeFieldVar");
    _addErrStr("  var is NULL");
    return -1;
  }

  int iret = 0;
  const void *data = field->getData();
  
  if (_nGatesVary) {

    switch (var->type()) {
      case nc3Double: {
        iret = !var->put((double *) data, _writeVol->getNPoints());
        break;
      }
      case nc3Float:
      default: {
        iret = !var->put((float *) data, _writeVol->getNPoints());
        break;
      }
      case nc3Int: {
        iret = !var->put((int *) data, _writeVol->getNPoints());
        break;
      }
      case nc3Short: {
        iret = !var->put((short *) data, _writeVol->getNPoints());
        break;
      }
      case nc3Byte: {
        iret = !var->put((ncbyte *) data, _writeVol->getNPoints());
        break;
      }
    } // switch

  } else {

    // get the max number of gates
    
    _writeVol->computeMaxNGates();
    
    switch (var->type()) {
      case nc3Double: {
        iret = !var->put((double *) data, _writeVol->getNRays(), _writeVol->getMaxNGates());
        break;
      }
      case nc3Float:
      default: {
        iret = !var->put((float *) data, _writeVol->getNRays(), _writeVol->getMaxNGates());
        break;
      }
      case nc3Int: {
        iret = !var->put((int *) data, _writeVol->getNRays(), _writeVol->getMaxNGates());
        break;
      }
      case nc3Short: {
        iret = !var->put((short *) data, _writeVol->getNRays(), _writeVol->getMaxNGates());
        break;
      }
      case nc3Byte: {
        iret = !var->put((ncbyte *) data, _writeVol->getNRays(), _writeVol->getMaxNGates());
        break;
      }
    } // switch

  }

  if (iret) {
    _addErrStr("ERROR - NcfRadxFile::_writeFieldVar");
    _addErrStr("  Cannot write var, name: ", var->name());
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  } else {
    return 0;
  }

}

//////////////////
// close on error

int NcfRadxFile::_closeOnError(const string &caller)
{
  _addErrStr("ERROR - NcfRadxFile::" + caller);
  _addErrStr(_file.getErrStr());
  _file.close();
  unlink(_tmpPath.c_str());
  return -1;
}

///////////////////////////////////////////////////////////////////////////
// Set output compression for variable

int NcfRadxFile::_setCompression(Nc3Var *var)  
{

  if (_ncFormat == NETCDF_CLASSIC || _ncFormat == NETCDF_OFFSET_64BIT) {
    // cannot compress
    return 0;
  }

  if (var == NULL) {
    _addErrStr("ERROR - NcfRadxFile::_setCompression");
    _addErrStr("  var is NULL");
    return -1;
  }
  
  if (!_writeCompressed) {
    return 0;
  }

  int fileId = _file.getNc3File()->id();
  int varId = var->id();
  int shuffle = 0;
  
  if (nc_def_var_deflate(fileId, varId, shuffle,
                         _writeCompressed, _compressionLevel)!= NC_NOERR) {
    cerr << "WARNING NcfRadxFile::_setCompression" << endl;
    cerr << "  Cannot compress field: " << var->name() << endl;
    cerr << "  Will be written uncompressed instead" << endl;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// Compute the output path

string NcfRadxFile::_computeWritePath(const RadxVol &vol,
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

  string prefix = "cfrad.";
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

  
