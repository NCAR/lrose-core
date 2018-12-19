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
// NexradCmdRadxFile.cc
//
// NexradCmdRadxFile object
//
// NetCDF file data for radar radial data in CF-compliant file
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/NexradCmdRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxRcalib.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
using namespace std;

//////////////
// Constructor

NexradCmdRadxFile::NexradCmdRadxFile() : RadxFile()
  
{

  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

NexradCmdRadxFile::~NexradCmdRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NexradCmdRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _timeDim = NULL;
  _rangeDim = NULL;

  _rangeVar = NULL;

  _startAzVar = NULL;
  _endAzVar = NULL;
  _startElVar = NULL;
  _endElVar = NULL;
  _startTimeVar = NULL;
  _endTimeVar = NULL;
  _hNoiseVar = NULL;
  _vNoiseVar = NULL;
  _dbz0Var = NULL;

  _title_attr.clear();
  _history_attr.clear();

  _title.clear();
  _history.clear();
  _statusXml.clear();

  _rayTimesIncrease = true;

  _sweepNumber = 0;
  _volumeNumber = 0;
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

  _rangeKm.clear();
  _gateSpacingIsConstant = true;

  // _latitudeVar = NULL;
  // _longitudeVar = NULL;
  // _altitudeVar = NULL;

  // _institution.clear();
  // _references.clear();
  // _source.clear();
  // _comment.clear();

  // _siteName.clear();
  // _scanName.clear();
  // _instrumentName.clear();

  // _scanId = 0;

  // _latitudeDeg = 0.0;
  // _longitudeDeg = 0.0;;
  // _altitudeKm = 0.0;

}

/////////////////////////////////////////////////////////
// Check if specified file is correct format
// Returns true if supported, false otherwise

bool NexradCmdRadxFile::isSupported(const string &path)

{
  
  if (isNexradCmd(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a NEXRAD CMD file
// Returns true on success, false on failure

bool NexradCmdRadxFile::isNexradCmd(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_verbose) {
      cerr << "DEBUG - not NexradCmd file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions

  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NexradCmd file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check existence of some variables

  Nc3Var *stdZdrFVar = _file.getNc3File()->get_var("STD_ZDR_F");
  if (stdZdrFVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NexradCmd file" << endl;
      cerr << "  STD_ZDR_F variable missing" << endl;
    }
    return false;
  }

  Nc3Var *tdbzFVar = _file.getNc3File()->get_var("TDBZ_F");
  if (tdbzFVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NexradCmd file" << endl;
      cerr << "  TDBZ_F variable missing" << endl;
    }
    return false;
  }

  // file has the correct dimensions, so it is a NexradCmd file

  _file.close();
  return true;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int NexradCmdRadxFile::writeToDir(const RadxVol &vol,
                                  const string &dir,
                                  bool addDaySubDir,
                                  bool addYearSubDir)
  
{

  // Writing NexradCmd files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - NexradCmdRadxFile::writeToDir" << endl;
  cerr << "  Writing NexradCmd format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;
  
  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToDir(vol, dir, addDaySubDir, addYearSubDir);

  // set return values

  _errStr = ncfFile.getErrStr();
  _dirInUse = ncfFile.getDirInUse();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int NexradCmdRadxFile::writeToPath(const RadxVol &vol,
                                   const string &path)
  
{

  // Writing NexradCmd files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - NexradCmdRadxFile::writeToPath" << endl;
  cerr << "  Writing NexradCmd format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;

  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToPath(vol, path);

  // set return values

  _errStr = ncfFile.getErrStr();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

////////////////////////////////////////////////
// get the date and time from a dorade file path
// returns 0 on success, -1 on failure

int NexradCmdRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

{

  RadxPath rpath(path);
  const string &fileName = rpath.getFile();
  
  // find first digit in entry name - if no digits, return now

  const char *start = NULL;
  for (size_t ii = 0; ii < fileName.size(); ii++) {
    if (isdigit(fileName[ii])) {
      start = fileName.c_str() + ii;
      break;
    }
  }
  if (!start) return -1;
  const char *end = start + strlen(start);
  
  // iteratively try getting the date and time from the string
  // moving along by one character at a time
  
  while (start < end - 6) {
    int year, month, day, hour, min, sec;
    if (sscanf(start, "%4d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return -1;
      }
      rtime.set(year, month, day, hour, min, sec);
      return 0;
    }
    start++;
  }
  
  return -1;
  
}

/////////////////////////////////////////////////////////
// print summary after read

void NexradCmdRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NexradCmdRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "  title: " << _title << endl;
  out << "  history: " << _history << endl;
  out << "  statusXml: " << _statusXml << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " 
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  out << "  startRangeKm: " << _remap.getStartRangeKm() << endl;
  out << "  gateSpacingKm: " << _remap.getGateSpacingKm() << endl;
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;

  // out << "  institution: " << _institution << endl;
  // out << "  references: " << _references << endl;
  // out << "  source: " << _source << endl;
  // out << "  comment: " << _comment << endl;
  // out << "  siteName: " << _siteName << endl;
  // out << "  scanName: " << _scanName << endl;
  // out << "  scanId: " << _scanId << endl;
  // out << "  instrumentName: " << _instrumentName << endl;
  // out << "  latitude: " << _latitudeDeg << endl;
  // out << "  longitude: " << _longitudeDeg << endl;
  // out << "  altitude: " << _altitudeKm << endl;
  // out << "  frequencyGhz: " << _frequencyGhz << endl;

  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int NexradCmdRadxFile::printNative(const string &path, ostream &out,
                                   bool printRays, bool printData)
  
{

  _addErrStr("ERROR - NexradCmdRadxFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int NexradCmdRadxFile::readFromPath(const string &path,
                                    RadxVol &vol)
  
{

  _initForRead(path, vol);
  
  if (_debug) {
    cerr << "Reading path: " << path << endl;
  }

  string errStr("ERROR - NexradCmdRadxFile::readFromPath");
  
  // clear tmp rays

  _nTimesInFile = 0;
  _raysToRead.clear();
  _raysValid.clear();
  _nRangeInFile = 0;

  // open file

  if (_file.openRead(path)) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    _addErrStr(errStr);
    return -1;
  }

  // read global attributes
  
  if (_readGlobalAttributes()) {
    _addErrStr(errStr);
    return -1;
  }

  // read status variables
  // i.e. details on processing

  if (_readStatusVariables()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read time variable
  
  // if (_readTimes()) {
  //   _addErrStr(errStr);
  //   return -1;
  // }

  // read range variable
  
  if (_readRangeVariable()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read position variables - lat/lon/alt
  
  // if (_readPositionVariables()) {
  //   _addErrStr(errStr);
  //   return -1;
  // }

  // read in ray variables

  if (_readRayVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // create the rays to be read in, filling out the metadata
  
  if (_createRays(path)) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read field variables
  
  if (!_readTimesOnly) {
    if (_readFieldVariables()) {
      _addErrStr(errStr);
      return -1;
    }
  }

  // close file

  _file.close();
  
  // add file rays to main rays
  
  _raysValid.clear();
  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    RadxRay *ray = _raysToRead[ii].ray;
    
    // check if we should keep this ray or discard it
    
    bool keep = true;
    if (_readRemoveRaysAllMissing) {
      if (ray->checkDataAllMissing()) {
        keep = false;
      }
    }

    // add to main vector if we are keeping it

    if (keep) {
      _raysValid.push_back(ray);
    } else {
      delete ray;
    }

  }
  
  _raysToRead.clear();
  
  // append to read paths
  
  _readPaths.push_back(path);

  // load the data into the read volume

  if (_loadReadVolume()) {
    return -1;
  }
  
  // compute fixed angles as mean angle from sweeps
  
  _computeFixedAngles();
  
  // set format as read
  
  _fileFormat = FILE_FORMAT_NEXRAD_CMD;

  // clean up

  _clearRayVariables();

  return 0;

}

///////////////////////////////////
// read in the dimensions

int NexradCmdRadxFile::_readDimensions()

{

  // read required dimensions
  
  int iret = 0;
  iret |= _file.readDim("radial_count", _timeDim);
  if (iret == 0) {
    _nTimesInFile = _timeDim->size();
  }

  _nRangeInFile = 0;
  iret |= _file.readDim("range", _rangeDim);
  if (iret == 0) {
    _nRangeInFile = _rangeDim->size();
  }
  
  _nRangeMask = 0;
  iret |= _file.readDim("RangeMaskRanges", _rangeMaskDim);
  if (iret == 0) {
    _nRangeMask = _rangeMaskDim->size();
  }
  
  if (iret) {
    _addErrStr("ERROR - NexradCmdRadxFile::_file.readDimensions");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read the global attributes

int NexradCmdRadxFile::_readGlobalAttributes()

{

  _file.readGlobAttr("title", _title_attr);
  _file.readGlobAttr("history", _history_attr);

  _title = _title_attr;
  _history = _history_attr;
  
  return 0;

}

///////////////////////////////////
// read the status variables

int NexradCmdRadxFile::_readStatusVariables()

{

  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("STATUS", 0);

  // loop through the variables, adding data fields as appropriate
  
  for (int ivar = 0; ivar < _file.getNc3File()->num_vars(); ivar++) {
    
    Nc3Var* var = _file.getNc3File()->get_var(ivar);
    if (var == NULL) {
      continue;
    }
    
    string varName = var->name();
    int numDims = var->num_dims();

    string units;
    Nc3Att *unitsAtt = var->get_att("units");
    if (unitsAtt != NULL) {
      units = Nc3xFile::asString(unitsAtt);
      delete unitsAtt;
    }
    
    string descr;
    Nc3Att *descrAtt = var->get_att("description");
    if (descrAtt != NULL) {
      descr = Nc3xFile::asString(descrAtt);
      delete descrAtt;
    }
    
    // field variable? not needed here
    
    if (numDims == 2) {
      Nc3Dim* timeDim = var->get_dim(0);
      Nc3Dim* rangeDim = var->get_dim(1);
      if (timeDim == _timeDim && rangeDim == _rangeDim) {
        continue;
      }
    }

    // variable with no dimensions?

    if (numDims == 0) {

      _statusXml += RadxXml::writeStartTag(varName, 1);
      _statusXml += RadxXml::writeString("units", 2, units);
      _statusXml += RadxXml::writeString("description", 2, descr);

      Nc3Type ftype = var->type();
      if (ftype == nc3Double) {
        double val;
        if (var->get(&val, 1)) {
          _statusXml += RadxXml::writeString("type", 2, "double");
          _statusXml += RadxXml::writeDouble("value", 2, val, "%g");
        }
      } else if (ftype == nc3Float) {
        float val;
        if (var->get(&val, 1)) {
          _statusXml += RadxXml::writeString("type", 2, "float");
          _statusXml += RadxXml::writeDouble("value", 2, val, "%g");
        }
      } else if (ftype == nc3Int) {
        int val;
        if (var->get(&val, 1)) {
          _statusXml += RadxXml::writeString("type", 2, "int");
          _statusXml += RadxXml::writeInt("value", 2, val);
        }
      } else if (ftype == nc3Short) {
        short val;
        if (var->get(&val, 1)) {
          _statusXml += RadxXml::writeString("type", 2, "short");
          _statusXml += RadxXml::writeInt("value", 2, val);
        }
      } else if (ftype == nc3Byte) {
        ncbyte val;
        if (var->get(&val, 1)) {
          _statusXml += RadxXml::writeString("type", 2, "byte");
          _statusXml += RadxXml::writeInt("value", 2, (int) val);
        }
      }
      
      _statusXml += RadxXml::writeEndTag(varName, 1);
      
      continue;

    } // if (numDims == 0) {

    // membership functions
    
    if (numDims == 1) {

      Nc3Dim* varDim = var->get_dim(0);
      string dimName = varDim->name();
      if (dimName.find("membership") != string::npos) {
        int nPoints = varDim->size();
        Nc3Type ftype = var->type();
        if (ftype == nc3Float) {
          float *vals = new float[nPoints];
          if (var->get(vals, nPoints)) {
            _statusXml += RadxXml::writeStartTag(varName, 1);
            _statusXml += RadxXml::writeString("units", 2, units);
            _statusXml += RadxXml::writeString("description", 2, descr);
            _statusXml += RadxXml::writeInt("npoints", 2, nPoints);
            for (int ii = 0; ii < nPoints; ii++) {
              _statusXml += RadxXml::writeDouble("value", 2, vals[ii], "%g");
            }
            _statusXml += RadxXml::writeEndTag(varName, 1);
          }
          delete[] vals;
        }
      }

    }

  } // ivar

  _statusXml += RadxXml::writeEndTag("STATUS", 0);

  return 0;

}

///////////////////////////////////
// read the range variable

int NexradCmdRadxFile::_readRangeVariable()

{

  _rangeVar = _file.getNc3File()->get_var("ranges");
  if (_rangeVar == NULL || _rangeVar->num_vals() < 1) {
    _addErrStr("ERROR - NexradCmdRadxFile::_readRangeVariable");
    _addErrStr("  Cannot read ranges");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  // get units
  
  double kmPerUnit = 1.0; // default - units in km
  Nc3Att* unitsAtt = _rangeVar->get_att("units");
  if (unitsAtt != NULL) {
    string units = Nc3xFile::asString(unitsAtt);
    if (units == "m") {
      kmPerUnit = 0.001;
    } else if (units == "meters") {
      kmPerUnit = 0.001;
    }
    delete unitsAtt;
  }

  // set range vector

  _rangeKm.clear();
  _nRangeInFile = _rangeVar->num_vals();
  RadxArray<double> rangeVals_;
  double *rangeVals = rangeVals_.alloc(_nRangeInFile);
  if (_rangeVar->get(rangeVals, _nRangeInFile)) {
    double *rr = rangeVals;
    double prevRange = *rr;
    for (size_t ii = 0; ii < _nRangeInFile; ii++, rr++) {
      double range = *rr;
      if (range < prevRange) {
        // make sure range is increasing
        _nRangeInFile = _rangeKm.size();
        break;
      }
      _rangeKm.push_back(range * kmPerUnit);
      prevRange = range;
    }
  }

  // set the geometry from the range vector
  
  _remap.computeRangeLookup(_rangeKm);
  _gateSpacingIsConstant = _remap.getGateSpacingIsConstant();
  _geom.setRangeGeom(_remap.getStartRangeKm(), _remap.getGateSpacingKm());

  return 0;

}

///////////////////////////////////
// read the position variables

// int NexradCmdRadxFile::_readPositionVariables()

// {

//   // find latitude, longitude, altitude

//   int iret = 0;
//   if (_file.readDoubleVar(_latitudeVar, "lat", _latitudeDeg, 0, true)) {
//     _addErrStr("ERROR - NexradCmdRadxFile::_readPositionVariables");
//     _addErrStr("  Cannot read latitude");
//     _addErrStr(_file.getNc3Error()->get_errmsg());
//     iret = -1;
//   }

//   if (_file.readDoubleVar(_longitudeVar, "lon", _longitudeDeg, 0, true)) {
//     _addErrStr("ERROR - NexradCmdRadxFile::_readPositionVariables");
//     _addErrStr("  Cannot read longitude");
//     _addErrStr(_file.getNc3Error()->get_errmsg());
//     iret = -1;
//   }

//   if (_file.readDoubleVar(_altitudeVar, "alt", _altitudeKm, 0, true)) {
//     _addErrStr("ERROR - NexradCmdRadxFile::_readPositionVariables");
//     _addErrStr("  Cannot read altitude");
//     _addErrStr(_file.getNc3Error()->get_errmsg());
//     iret = -1;
//   }
//   Nc3Att* unitsAtt = _altitudeVar->get_att("units");
//   if (unitsAtt != NULL) {
//     string units = Nc3xFile::asString(unitsAtt);
//     if (units == "m") {
//       _altitudeKm /= 1000.0;
//     }
//     delete unitsAtt;
//   }
  
//   return iret;

// }

///////////////////////////////////
// clear the ray variables

void NexradCmdRadxFile::_clearRayVariables()

{

  _startAz.clear();
  _endAz.clear();
  _rayAz.clear();

  _startEl.clear();
  _endEl.clear();
  _rayEl.clear();

  _startTime.clear();
  _endTime.clear();
  _rayTime.clear();

  _hNoise.clear();
  _vNoise.clear();
  _dbz0.clear();

}

///////////////////////////////////
// read in ray variables

int NexradCmdRadxFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  // azimuth

  _readRayVar(_startAzVar, "startAz", _startAz);
  if (_startAz.size() != _nTimesInFile) {
    _addErrStr("ERROR - startAz variable required");
    iret = -1;
  }
  
  _readRayVar(_endAzVar, "endAz", _endAz);
  if (_endAz.size() != _nTimesInFile) {
    _addErrStr("ERROR - endAz variable required");
    iret = -1;
  }

  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    double rayAz = (_startAz[ii] + _endAz[ii]) / 2.0;
    double startAz = _startAz[ii];
    double endAz = _endAz[ii];
    double deltaAz = endAz - startAz;
    if (fabs(deltaAz) > 180) {
      rayAz = (startAz + endAz + 360.0) / 2.0;
    }
    if (rayAz > 360) {
      rayAz -= 360.0;
    }
    _rayAz.push_back(rayAz);
  }

  // elevation

  _readRayVar(_startElVar, "startEl", _startEl);
  if (_startEl.size() != _nTimesInFile) {
    _addErrStr("ERROR - startEl variable required");
    iret = -1;
  }
  
  _readRayVar(_endElVar, "endEl", _endEl);
  if (_endEl.size() != _nTimesInFile) {
    _addErrStr("ERROR - endEl variable required");
    iret = -1;
  }
  
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    double rayEl = (_startEl[ii] + _endEl[ii]) / 2.0;
    _rayEl.push_back(rayEl);
  }

  // time

  _readRayVar(_startTimeVar, "StartTimeUTC", _startTime);
  if (_startTime.size() != _nTimesInFile) {
    _addErrStr("ERROR - startTime variable required");
    iret = -1;
  }
  
  _readRayVar(_endTimeVar, "EndTimeUTC", _endTime);
  if (_endTime.size() != _nTimesInFile) {
    _addErrStr("ERROR - endTime variable required");
    iret = -1;
  }

  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    double rayTime = (_startTime[ii] + _endTime[ii]) / 2.0;
    if (ii > 0 && rayTime < _rayTime[_rayTime.size()-1]) {
      _rayTimesIncrease = false;
    }
    _rayTime.push_back(rayTime);
  }
  
  _readRayVar(_hNoiseVar, "H_noise", _hNoise);
  _readRayVar(_vNoiseVar, "V_noise", _vNoise);
  _readRayVar(_dbz0Var, "dBZ0", _dbz0);
  
  if (iret) {
    _addErrStr("ERROR - NexradCmdRadxFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int NexradCmdRadxFile::_createRays(const string &path)

{

  _raysToRead.clear();
 
  for (size_t iray = 0; iray < _nTimesInFile; iray++) {
    
    RayInfo rayInfo;
    rayInfo.indexInFile = iray;

    // new ray

    RadxRay *ray = new RadxRay;
    rayInfo.ray = ray;

    ray->copyRangeGeom(_geom);
    
    // set time
    
    double rayTimeDouble = _rayTime[iray];
    time_t rayUtimeSecs = rayTimeDouble;
    double rayIntSecs;
    double rayFracSecs = modf(rayTimeDouble, &rayIntSecs);
    int rayNanoSecs = (int) (rayFracSecs * 1.0e9);
    ray->setTime(rayUtimeSecs, rayNanoSecs);
    ray->setSweepNumber(_sweepNumber);
    ray->setAzimuthDeg(_rayAz[iray]);
    ray->setElevationDeg(_rayEl[iray]);

    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    ray->setPolarizationMode(Radx::POL_MODE_HV_SIM);
    
    if (_hNoise.size() > iray) {
      ray->setEstimatedNoiseDbmHc(_hNoise[iray]);
    }
    if (_vNoise.size() > iray) {
      ray->setEstimatedNoiseDbmVc(_vNoise[iray]);
    }
    
    // add to ray vector
    
    _raysToRead.push_back(rayInfo);

  } // iray

  return 0;

}

////////////////////////////////////////////
// read the field variables

int NexradCmdRadxFile::_readFieldVariables()

{

  // loop through the variables, adding data fields as appropriate
  
  for (int ivar = 0; ivar < _file.getNc3File()->num_vars(); ivar++) {
    
    Nc3Var* var = _file.getNc3File()->get_var(ivar);
    if (var == NULL) {
      continue;
    }
    
    int numDims = var->num_dims();
    // we need fields with 2 dimensions
    if (numDims != 2) {
      continue;
    }
    // check that we have the correct dimensions
    Nc3Dim* timeDim = var->get_dim(0);
    Nc3Dim* rangeDim = var->get_dim(1);
    if (timeDim != _timeDim || rangeDim != _rangeDim) {
      continue;
    }
    
    // check the type
    string fieldName = var->name();
    Nc3Type ftype = var->type();
    if (ftype != nc3Double && ftype != nc3Float && ftype != nc3Byte) {
      // not a valid type
      if (_verbose) {
        cerr << "DEBUG - NexradCmdRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
        cerr << "  -->> Should be float or double: " << fieldName << endl;
      }
      continue;
    }

    // check that we need this field
    
    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - NexradCmdRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - NexradCmdRadxFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << fieldName << endl;
    }
    
    // set names, units, etc
    
    string name = var->name();
    
    string longName;
    Nc3Att *longNameAtt = var->get_att("description");
    if (longNameAtt != NULL) {
      longName = Nc3xFile::asString(longNameAtt);
      delete longNameAtt;
    }

    string units;
    Nc3Att *unitsAtt = var->get_att("units");
    if (unitsAtt != NULL) {
      units = Nc3xFile::asString(unitsAtt);
      delete unitsAtt;
    }

    // if metadata only, don't read in fields

    if (_readMetadataOnly) {
      bool fieldAlreadyAdded = false;
      for (size_t ii = 0; ii < _readVol->getNFields(); ii++) {
        if (_readVol->getField(ii)->getName() == name) {
          fieldAlreadyAdded = true;
          break;
        }
      }
      if (!fieldAlreadyAdded) {
        RadxField *field = new RadxField(name, units);
        field->setLongName(longName);
        _readVol->addField(field);
      }
      continue;
    }

    int iret = 0;

    switch (var->type()) {
      case nc3Double: {
        if (_addFl64FieldToRays(var, name, units, longName)) {
          iret = -1;
        }
        break;
      }
      case nc3Float: {
        if (_addFl32FieldToRays(var, name, units, longName)) {
          iret = -1;
        }
        break;
      }
      case nc3Byte: {
        if (_addSi08FieldToRays(var, name, units, longName)) {
          iret = -1;
        }
        break;
      }
      default: {
        iret = -1;
        // will not reach here because of earlier check on type
      }

    } // switch
    
    if (iret) {
      _addErrStr("ERROR - NexradCmdRadxFile::_readFieldVariables");
      return -1;
    }

  } // ivar

  return 0;

}

///////////////////////////////////
// read a ray variable - double

int NexradCmdRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                   vector<double> &vals, bool required)

{

  vals.clear();

  // get var

  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NexradCmdRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  double *data = new double[_nTimesInFile];
  double *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - NexradCmdRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - integer

int NexradCmdRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                   vector<int> &vals, bool required)

{

  vals.clear();

  // get var
  
  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NexradCmdRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  int *data = new int[_nTimesInFile];
  int *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - NexradCmdRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

Nc3Var* NexradCmdRadxFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - NexradCmdRadxFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - NexradCmdRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - NexradCmdRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has incorrect dimension, dim name: ", 
                 timeDim->name());
      _addErrStr("  should be: ", "time");
    }
    return NULL;
  }

  return var;

}

//////////////////////////////////////////////////////////////
// Add fl64 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NexradCmdRadxFile::_addFl64FieldToRays(Nc3Var* var,
                                           const string &name,
                                           const string &units,
                                           const string &longName)
  
{

  // get data from array

  Radx::fl64 *data = new Radx::fl64[_nTimesInFile * _nRangeInFile];
  int iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  if (iret) {
    _addErrStr("ERROR - NexradCmdRadxFile::_addFl64FieldToRays");
    _addErrStr("  variable name: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::fl64 missingVal = Radx::missingFl64;
  Nc3Att *missingValueAtt = var->get_att("missing_value");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - NexradCmdRadxFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    
    RadxField *field =
      _raysToRead[ii].ray->addField(name, units, nGates,
                                    missingVal,
                                    data + startIndex,
                                    true);

    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    field->setIsDiscrete(false);

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add fl32 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NexradCmdRadxFile::_addFl32FieldToRays(Nc3Var* var,
                                           const string &name,
                                           const string &units,
                                           const string &longName)
  
{

  // get data from array

  Radx::fl32 *data = new Radx::fl32[_nTimesInFile * _nRangeInFile];
  int iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  if (iret) {
    _addErrStr("ERROR - NexradCmdRadxFile::_addFl32FieldToRays");
    _addErrStr("  variable name: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    delete[] data;
    return -1;
  }

  // set missing value
  
  Radx::fl32 missingVal = Radx::missingFl32;
  Nc3Att *missingValueAtt = var->get_att("missing_value");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - NexradCmdRadxFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;

    RadxField *field =
      _raysToRead[ii].ray->addField(name, units, nGates,
                                    missingVal,
                                    data + startIndex,
                                    true);
    
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    field->setIsDiscrete(false);

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add byte field to _raysFromFile
// Returns 0 on success, -1 on failure

int NexradCmdRadxFile::_addSi08FieldToRays(Nc3Var* var,
                                           const string &name,
                                           const string &units,
                                           const string &longName)
  
{

  // get data from array
  
  Radx::si08 *data = new Radx::si08[_nTimesInFile * _nRangeInFile];
  int iret = !var->get((ncbyte *) data, _nTimesInFile, _nRangeInFile);
  if (iret) {
    _addErrStr("ERROR - NexradCmdRadxFile::_addSi08FieldToRays");
    _addErrStr("  variable name: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    delete[] data;
    return -1;
  }

  // set missing value
  
  Radx::si08 missingVal = Radx::missingSi08;
  Nc3Att *missingValueAtt = var->get_att("_FillValue");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }
  
  // load field on rays
  
  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;
    
    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - NexradCmdRadxFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    
    RadxField *field =
      _raysToRead[ii].ray->addField(name, units, nGates,
                                    missingVal,
                                    data + startIndex,
                                    1, 0,
                                    true);
    
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

    field->setIsDiscrete(true);

  }
  
  delete[] data;
  return 0;
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int NexradCmdRadxFile::_loadReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("DOE");
  _readVol->setVolumeNumber(0);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  // _readVol->addFrequencyHz(_frequencyGhz * 1.0e9);

  _readVol->setTitle(_title);
  // _readVol->setSource(_source);
  _readVol->setHistory(_history);
  // _readVol->setInstitution(_institution);
  // _readVol->setReferences(_references);
  // _readVol->setComment(_comment);
  _readVol->setStatusXml(_statusXml);
  // _readVol->setSiteName(_siteName);
  // _readVol->setScanName(_scanName);
  // _readVol->setScanId(_scanId);
  // _readVol->setInstrumentName(_instrumentName);

  // _readVol->setLatitudeDeg(_latitudeDeg);
  // _readVol->setLongitudeDeg(_longitudeDeg);
  // _readVol->setAltitudeKm(_altitudeKm);

  _readVol->copyRangeGeom(_geom);

  for (int ii = 0; ii < (int) _raysValid.size(); ii++) {
    _raysValid[ii]->setVolumeNumber(_volumeNumber);
    _raysValid[ii]->setSweepNumber(_sweepNumber);
  }

  // add rays to vol - they will be freed by vol

  for (size_t ii = 0; ii < _raysValid.size(); ii++) {

    // fake angles for testing
    // double el = 0.5;
    // double az = ii * 0.5;
    // while (az > 360) {
    //   az -= 360;
    // }
    // _raysValid[ii]->setElevationDeg(el);
    // _raysValid[ii]->setAzimuthDeg(az);
    // _raysValid[ii]->setFixedAngleDeg(el);
    // _raysValid[ii]->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);

    _readVol->addRay(_raysValid[ii]);
  }

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _raysValid.clear();
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();

  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - NexradCmdRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - NexradCmdRadxFile::_loadReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();
  
  // check for indexed rays, set info on rays

  _readVol->checkForIndexedRays();

  // add calibration

  RadxRcalib *calib = new RadxRcalib;
  
  double sumDbz0 = 0.0;
  double countDbz0 = 0.0;
  for (size_t ii = 0; ii < _dbz0.size(); ii++) {
    if (_dbz0[ii] != Radx::missingMetaDouble) {
      sumDbz0 += _dbz0[ii];
      countDbz0++;
    }
  }
  if (countDbz0 > 0) {
    double meanDbz0 = sumDbz0 / countDbz0;
    calib->setBaseDbz1kmHc(meanDbz0);
    calib->setBaseDbz1kmVc(meanDbz0);
  }

  double sumHNoise = 0.0;
  double countHNoise = 0.0;
  for (size_t ii = 0; ii < _hNoise.size(); ii++) {
    if (_hNoise[ii] != Radx::missingMetaDouble) {
      sumHNoise += 10.0 * log10(_hNoise[ii]);
      countHNoise++;
    }
  }
  if (countHNoise > 0) {
    double meanHNoise = sumHNoise / countHNoise;
    calib->setNoiseDbmHc(meanHNoise);
    calib->setReceiverGainDbHc(meanHNoise - (-114.5));
  }

  double sumVNoise = 0.0;
  double countVNoise = 0.0;
  for (size_t ii = 0; ii < _vNoise.size(); ii++) {
    if (_vNoise[ii] != Radx::missingMetaDouble) {
      sumVNoise += 10.0 * log10(_vNoise[ii]);
      countVNoise++;
    }
  }
  if (countVNoise > 0) {
    double meanVNoise = sumVNoise / countVNoise;
    calib->setNoiseDbmVc(meanVNoise);
    calib->setReceiverGainDbVc(meanVNoise - (-114.5));
  }

  _readVol->addCalib(calib);

  return 0;

}

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void NexradCmdRadxFile::_computeFixedAngles()
  
{

  for (size_t isweep = 0; isweep < _readVol->getNSweeps(); isweep++) {

    RadxSweep &sweep = *_readVol->getSweeps()[isweep];

    double sumElev = 0.0;
    double count = 0.0;

    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      const RadxRay &ray = *_readVol->getRays()[iray];
      sumElev += ray.getElevationDeg();
      count++;
    }

    double meanElev = sumElev / count;
    double fixedAngle = ((int) (meanElev * 100.0 + 0.5)) / 100.0;

    sweep.setFixedAngleDeg(fixedAngle);
      
    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      RadxRay &ray = *_readVol->getRays()[iray];
      ray.setFixedAngleDeg(fixedAngle);
    }

  } // isweep

  _readVol->loadFixedAnglesFromSweepsToRays();

}

