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
// NoaaFslRadxFile.cc
//
// NoaaFslRadxFile object
//
// NetCDF file data for CSU/NASA D2R radial data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2018
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/NoaaFslRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxComplex.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
using namespace std;

int NoaaFslRadxFile::_volumeNumber = -1;

//////////////
// Constructor

NoaaFslRadxFile::NoaaFslRadxFile() : RadxFile()
  
{

  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

NoaaFslRadxFile::~NoaaFslRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NoaaFslRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _radialDim = NULL;
  _rangeDim = NULL;

  _rayTimes.clear();
  _dTimes.clear();
  _rayTimesIncrease = true;
  _nRadials = 0;
  _nTimes = 0;

  _raysVol.clear();
  _raysFile.clear();

  _rangeKm.clear();
  _nRangeInFile = 0;
  _gateSpacingIsConstant = true;

  _azimuth.clear();
  _elevation.clear();
  _latitudeDeg = 0;
  _longitudeDeg = 0;
  _altitudeM = 0;

  _sweepNumber = 0;

  // _refTimeSecsFile = 0;
  
  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();
  _statusXml.clear();
  
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool NoaaFslRadxFile::isSupported(const string &path)

{
  
  if (isNoaaFsl(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a NoaaFsl file
// Returns true on success, false on failure

bool NoaaFslRadxFile::isNoaaFsl(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_verbose) {
      cerr << "DEBUG - not NoaaFsl file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NoaaFsl file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check existence of some variables

  Nc3Var *var1 = _file.getNc3File()->get_var("radialAzim");
  if (var1 == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NoaaFsl file" << endl;
      cerr << "  radialAzim variable missing" << endl;
    }
    return false;
  }

  Nc3Var *var2 = _file.getNc3File()->get_var("radialElev");
  if (var2 == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NoaaFsl file" << endl;
      cerr << "  radialElev variable missing" << endl;
    }
    return false;
  }

  Nc3Var *var3 = _file.getNc3File()->get_var("radialTime");
  if (var3 == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NoaaFsl file" << endl;
      cerr << "  radialTime variable missing" << endl;
    }
    return false;
  }

  Nc3Var *var4 = _file.getNc3File()->get_var("esStartTime");
  if (var4 == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NoaaFsl file" << endl;
      cerr << "  esStartTime variable missing" << endl;
    }
    return false;
  }

  // file has the correct dims and variables, so it is a NoaaFsl file

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

int NoaaFslRadxFile::writeToDir(const RadxVol &vol,
                                const string &dir,
                                bool addDaySubDir,
                                bool addYearSubDir)
  
{

  // Writing NoaaFsl files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - NoaaFslRadxFile::writeToDir" << endl;
  cerr << "  Writing Noaa Fsl format files not supported" << endl;
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

int NoaaFslRadxFile::writeToPath(const RadxVol &vol,
                                 const string &path)
  
{

  // Writing NoaaFsl files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - NoaaFslRadxFile::writeToPath" << endl;
  cerr << "  Writing NoaaFsl format files not supported" << endl;
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

int NoaaFslRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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

  // yymmmddhhmmss
  
  const char *ptr = start;
  while (ptr < end - 6) {
    int year, month, day, hour, min, sec;
    if (sscanf(ptr, "%2d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      if (month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return -1;
      }
      if (year < 50) {
        year += 2000;
      } else {
        year += 1900;
      }
      rtime.set(year, month, day, hour, min, sec);
      return 0;
    }
    ptr++;
  }

  // yyyy_mm_dd_hh_mm_ss
  
  ptr = start;
  while (ptr < end - 6) {
    int year, month, day, hour, min, sec;
    if (sscanf(ptr, "%4d_%2d_%2d_%2d_%2d_%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      if (month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return -1;
      }
      rtime.set(year, month, day, hour, min, sec);
      return 0;
    }
    ptr++;
  }
  
  return -1;
  
}

/////////////////////////////////////////////////////////
// print summary after read

void NoaaFslRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NoaaFslRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "  title: " << _title << endl;
  out << "  institution: " << _institution << endl;
  out << "  references: " << _references << endl;
  out << "  source: " << _source << endl;
  out << "  history: " << _history << endl;
  out << "  comment: " << _comment << endl;
  out << "  statusXml: " << _statusXml << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " 
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  out << "  latitudeDeg: " << _latitudeDeg << endl;
  out << "  longitudeDeg: " << _longitudeDeg << endl;
  out << "  altitudeM: " << _altitudeM << endl;
  out << "  startRangeKm: " << _remap.getStartRangeKm() << endl;
  out << "  gateSpacingKm: " << _remap.getGateSpacingKm() << endl;
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  out << "  nyquistVel: " << _nyquistVel << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int NoaaFslRadxFile::printNative(const string &path, ostream &out,
                                 bool printRays, bool printData)
  
{
  
  _addErrStr("ERROR - NoaaFslRadxFile::printNative");
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

int NoaaFslRadxFile::readFromPath(const string &path,
                                  RadxVol &vol)
  
{

  // initialize

  _initForRead(path, vol);
  clear();
  
  // Check if this is a NoaaFsl file

  if (!isNoaaFsl(path)) {
    _addErrStr("ERROR - NoaaFslRadxFile::readFromPath");
    _addErrStr("  Not a D3R file: ", path);
    return -1;
  }
  
  if (_debug) {
    cerr << "Reading path: " << path << endl;
  }
  
  // if the flag is set to aggregate sweeps into a volume on read,
  // call the method to handle that
  
  if (_readAggregateSweeps) {
    if (_readAggregatePaths(path)) {
      _addErrStr("ERROR - NoaaFslRadxFile::readFromPath");
      return -1;
    }
  } else {
    if (_readFile(path)) {
      _addErrStr("ERROR - NoaaFslRadxFile::readFromPath");
      return -1;
    }
  }
  
  // load the data into the read volume

  _volumeNumber++;
  if (_loadReadVolume()) {
    return -1;
  }
  
  // set format as read

  _fileFormat = FILE_FORMAT_D3R_NC;

  return 0;

}

////////////////////////////////////////////////////////////
// Read in sweep data using the specified path as a starting
// point. Aggregate the data from the sweeps into a single
// volume.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int NoaaFslRadxFile::_readAggregatePaths(const string &path)
  
{

  // get the list of paths which make up this volume

  vector<string> paths;
  _getVolumePaths(path, paths);
  
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_readFile(paths[ii])) {
      return -1;
    }
  }

  return 0;

}


////////////////////////////////////////////////////////////
// Read in data from specified path,
// load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int NoaaFslRadxFile::_readFile(const string &path)
  
{

  string errStr("ERROR - NoaaFslRadxFile::readFromPath");
  
  // clear vars

  _nRadials = 0;
  _nTimes = 0;
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
  
  // read the scalars

  if (_readScalars()) {
    _addErrStr(errStr);
    return -1;
  }

  // read the sweep angles

  if (_readSweepAngles()) {
    _addErrStr(errStr);
    return -1;
  }

  // read time variable now if that is all that is needed
  
  if (_readTimesOnly) {
    if (_readTimes()) {
      _addErrStr(errStr);
      return -1;
    }
    return 0;
  }
  
  // read in ray az and el
  
  if (_readAzEl()) {
    _addErrStr(errStr);
    return -1;
  }

  // read time variable

  if (_readTimes()) {
    _addErrStr(errStr);
    return -1;
  }

  if (_readMetadataOnly) {

    // read field variables
    
    if (_readFieldVariables(true)) {
      _addErrStr(errStr);
      return -1;
    }
    
  } else {

    // create the rays to be read in, filling out the metadata
    
    if (_createRays(path)) {
      _addErrStr(errStr);
      return -1;
    }
    
    // add field variables to file rays
    
    if (_readFieldVariables(false)) {
      _addErrStr(errStr);
      return -1;
    }

  }

  // close file

  _file.close();
  
  // add file rays to vol rays

  for (size_t ii = 0; ii < _raysFile.size(); ii++) {

    RadxRay *ray = _raysFile[ii];
    
    // check if we should keep this ray or discard it
    
    bool keep = true;
    if (_readRemoveRaysAllMissing) {
      if (ray->checkDataAllMissing()) {
        keep = false;
      }
    }

    // add to valid vector if we are keeping it

    if (keep) {
      _raysVol.push_back(ray);
    } else {
      delete ray;
    }
    
  }

  _raysFile.clear();
  
  // append to read paths
  
  _readPaths.push_back(path);

  // clean up

  _azimuth.clear();
  _elevation.clear();

  return 0;

}

//////////////////////////////////////////////////////////////
// get list of paths for the same volume as the specified path
//
// We search for files with the same time in the path

void NoaaFslRadxFile::_getVolumePaths(const string &path,
                                      vector<string> &paths)
  
{
  
  paths.clear();

  // get file name

  RadxPath rpath(path);
  string fileName = rpath.getFile();
  string dir = rpath.getDirectory();
  
  // find '.'
  
  size_t dotPos = fileName.find('.');
  string rootName(fileName.substr(0, dotPos));

  // find all files in this dir with same root

  vector<string> pathList;

  // find all paths within the given time
  
  DIR *dirp;
  if((dirp = opendir(dir.c_str())) == NULL) {
    return;
  }
  
  struct dirent *dp;
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    string fileName(dp->d_name);

    // exclude dir entries which cannot be valid
    if (fileName.find(".nc") == string::npos) {
      continue;
    }
    
    if (fileName.size() < 10) {
      continue;
    }

    if (fileName.find(rootName) != string::npos) {
      string filePath(dir);
      filePath += RadxFile::PATH_SEPARATOR;
      filePath += fileName;
      paths.push_back(filePath);
    }
    
  } // dp

  closedir(dirp);

}

///////////////////////////////////
// read in the dimensions

int NoaaFslRadxFile::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  iret |= _file.readDim("radial", _radialDim);
  if (iret == 0) {
    _nRadials = _radialDim->size();
  }

  _nRangeInFile = 0;
  iret |= _file.readDim("bin", _rangeDim);
  if (iret == 0) {
    _nRangeInFile = _rangeDim->size();
  }
  
  _nSweeps = 0;
  iret |= _file.readDim("sweep", _sweepDim);
  if (iret == 0) {
    _nSweeps = _sweepDim->size();
  }

  _nTimes = _nRadials * _nSweeps;
  
  if (iret) {
    _addErrStr("ERROR - NoaaFslRadxFile::_file.readDimensions");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read the scalars

int NoaaFslRadxFile::_readScalars()
  
{

  int iret = 0;
  string units;

  // start time

  _startTime = 0.0;
  if (_file.readDoubleVal("esStartTime", _startTime, 0.0)) {
    if (_debug) {
      cerr << "WARNING - NoaaFlsRadxFile::_readScalars()" << endl;
      cerr << " Cannot find 'esStartTime' variable, setting to 0" << endl;
    }
    iret = -1;
  }

  // sweep number

  _sweepNumber = 0;
  if (_file.readIntVal("elevationNumber", _sweepNumber, Radx::missingMetaInt)) {
    if (_debug) {
      cerr << "WARNING - NoaaFlsRadxFile::_readScalars()" << endl;
      cerr << " Cannot find 'elevationNumber' variable, setting to 0" << endl;
    }
    iret = -1;
  }

  // elevation angle

  _elevationAngle = 0.0;
  if (_file.readDoubleVal("elevationAngle", _elevationAngle, Radx::missingMetaDouble)) {
    if (_debug) {
      cerr << "WARNING - NoaaFlsRadxFile::_readScalars()" << endl;
      cerr << " Cannot find 'elevationAngle' variable, setting to 0" << endl;
    }
    iret = -1;
  }

  // lat, lon, alt

  _latitudeDeg = 0.0;
  if (_file.readDoubleVal("siteLat", _latitudeDeg, Radx::missingMetaDouble)) {
    if (_debug) {
      cerr << "WARNING - NoaaFlsRadxFile::_readScalars()" << endl;
      cerr << " Cannot find 'siteLat' variable, setting to 0" << endl;
    }
    iret = -1;
  }

  _longitudeDeg = 0.0;
  if (_file.readDoubleVal("siteLon", _longitudeDeg, Radx::missingMetaDouble)) {
    if (_debug) {
      cerr << "WARNING - NoaaFlsRadxFile::_readScalars()" << endl;
      cerr << " Cannot find 'siteLon' variable, setting to 0" << endl;
    }
    iret = -1;
  }

  _altitudeM = 0.0;
  if (_file.readDoubleVal("siteAlt", _altitudeM,
                          units, Radx::missingMetaDouble)) {
    if (_debug) {
      cerr << "WARNING - NoaaFlsRadxFile::_readScalars()" << endl;
      cerr << " Cannot find 'siteAlt' variable, setting to 0" << endl;
    }
    iret = -1;
  }
  if (units == "km") {
    _altitudeM *= 1000.0;
  }

  // start range, gate spacing

  _gateSpacingM = 250.0;
  if (_file.readDoubleVal("gateSize", _gateSpacingM,
                          units, Radx::missingMetaDouble)) {
    if (_debug) {
      cerr << "WARNING - NoaaFlsRadxFile::_readScalars()" << endl;
      cerr << " Cannot find 'gateSize' variable, setting to 250" << endl;
    }
    iret = -1;
  }
  if (units == "km") {
    _gateSpacingM *= 1000.0;
  }

  // init start range to center of first gate

  _startRangeM += _gateSpacingM / 2.0;

  if (_file.readDoubleVal("firstgateRange", _startRangeM,
                          units, Radx::missingMetaDouble)) {
    if (_file.readDoubleVal("firstGateRange", _startRangeM,
                            units, Radx::missingMetaDouble)) {
      if (_debug) {
        cerr << "WARNING - NoaaFlsRadxFile::_readScalars()" << endl;
        cerr << " Cannot find 'firstGateRange' variable, setting to gateSize/2 = "
             << _startRangeM << endl;
      }
      iret = -1;
    }
  }
  if (units == "km") {
    _startRangeM *= 1000.0;
  }

  // nyquist

  _file.readDoubleVal("nyquist", _nyquistVel, Radx::missingMetaDouble);

  // calibration

  _file.readDoubleVal("calibConst", _calibConst, Radx::missingMetaDouble);
  if (_file.readDoubleVal("radarConst", _radarConst, Radx::missingMetaDouble)) {
    _file.readDoubleVal("RadarConst", _radarConst, Radx::missingMetaDouble);
  }

  // beam width

  _file.readDoubleVal("beamWidthHori", _beamWidthH, Radx::missingMetaDouble);
  _file.readDoubleVal("beamWidthVert", _beamWidthV, Radx::missingMetaDouble);

  // pulse width

  if (_file.readDoubleVal("pulseWidth", _pulseWidthUsec,
                          units, Radx::missingMetaDouble) == 0) {
    if (units == "sec" || units == "s") {
      _pulseWidthUsec *= 1.0e6;
    }
  }
  
  // band width

  if (_file.readDoubleVal("bandWidth", _bandWidthHertz,
                          units, Radx::missingMetaDouble) == 0) {
    if (units.find("mega") != string::npos) {
      _bandWidthHertz *= 1.0e6;
    } else if (units.find("giga") != string::npos) {
      _bandWidthHertz *= 1.0e9;
    }
  }

  if (_file.readDoubleVal("unambigrange", _unambigRangeM,
                          units, Radx::missingMetaDouble) == 0) {
    if (units == "kilometer" || units == "km") {
      _unambigRangeM *= 1.0e3;
    }
  }

  if (_file.readDoubleVal("waveLength", _wavelengthCm,
                          units, Radx::missingMetaDouble) == 0) {
    if (units == "m" || units == "meters") {
      _wavelengthCm *= 1.0e2;
    }
  }

  _file.readDoubleVal("azimuthSpeed", _azSpeedDegPerSec, Radx::missingMetaDouble);
  _file.readDoubleVal("angleResolution", _angleResDeg, Radx::missingMetaDouble);

  _file.readDoubleVal("_prfHigh", _prfHigh, Radx::missingMetaDouble);
  _file.readDoubleVal("_prfLow", _prfLow, Radx::missingMetaDouble);

  _file.readIntVal("sampleNum", _nSamples, Radx::missingMetaInt);

  // thresholds

  _file.readDoubleVal("SQIThresh", _sqiThresh, Radx::missingMetaDouble);
  _file.readDoubleVal("LOGThresh", _logThresh, Radx::missingMetaDouble);
  _file.readDoubleVal("SIGThresh", _sigThresh, Radx::missingMetaDouble);
  _file.readDoubleVal("CSRThresh", _csrThresh, Radx::missingMetaDouble);

  _file.readIntVal("DBTThreshFlag", _dbtThreshFlag, Radx::missingMetaDouble);
  _file.readIntVal("DBZThreshFlag", _dbzThreshFlag, Radx::missingMetaDouble);
  _file.readIntVal("VELThreshFlag", _velThreshFlag, Radx::missingMetaDouble);
  _file.readIntVal("WIDThreshFlag", _widThreshFlag, Radx::missingMetaDouble);

  // set the status XML from the thresholds

  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("Thresholds", 0);
  _statusXml += RadxXml::writeDouble("SQIThresh", 1, _sqiThresh);
  _statusXml += RadxXml::writeDouble("LOGThresh", 1, _logThresh);
  _statusXml += RadxXml::writeDouble("SIGThresh", 1, _sigThresh);
  _statusXml += RadxXml::writeDouble("CSRThresh", 1, _csrThresh);
  _statusXml += RadxXml::writeInt("DBTThreshFlag", 1, _dbtThreshFlag);
  _statusXml += RadxXml::writeInt("DBZThreshFlag", 1, _dbzThreshFlag);
  _statusXml += RadxXml::writeInt("VELThreshFlag", 1, _velThreshFlag);
  _statusXml += RadxXml::writeInt("WIDThreshFlag", 1, _widThreshFlag);
  _statusXml += RadxXml::writeEndTag("Thresholds", 0);

  // set range vector

  _rangeKm.clear();
  for (size_t ii = 0; ii < _nRangeInFile; ii++) {
    _rangeKm.push_back((_startRangeM + ii * _gateSpacingM) / 1000.0);
  }
  
  // set the geometry from the range vector
  
  _gateSpacingIsConstant = true;
  _geom.setRangeGeom(_startRangeM / 1000.0, _gateSpacingM / 1000.0);

  if (iret) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readScalars");
  }
    
  return 0;

}

///////////////////////////////////
// read the global attributes

int NoaaFslRadxFile::_readGlobalAttributes()

{

  _file.readGlobAttr("Content", _comment);
  _file.readGlobAttr("history", _history);
  _file.readGlobAttr("title", _title);
  _file.readGlobAttr("Conventions", _references);

  _institution = "NOAA";
  _source.clear();
  
  return 0;

}

///////////////////////////////////
// read the sweep angles

int NoaaFslRadxFile::_readSweepAngles()

{

  Nc3Var *elevsVar = _file.getNc3File()->get_var("elevationList");
  if (elevsVar == NULL) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readSweepAngles");
    _addErrStr("  Cannot find sweep angles variable, name: ", "elevationList");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  if (elevsVar->num_dims() < 1) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readSweepAngles");
    _addErrStr("  elevationList variable has no dimensions");
    return -1;
  }
  Nc3Dim *sweepDim = elevsVar->get_dim(0);
  if (sweepDim != _sweepDim) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readSweepAngles");
    _addErrStr("  elevationList has incorrect dimension, name: ", sweepDim->name());
    return -1;
  }

  // read the elevs array
  
  RadxArray<double> elevs_;
  double *elevs = elevs_.alloc(_nSweeps);
  if (elevsVar->get(elevs, _nSweeps) == 0) {
    _addErrStr("ERROdR - NoaaFslRadxFile::_readSweepAngles");
    _addErrStr("  Candnot read elevationList variable");
    return -1;
  }
  _elevList.clear();
  for (size_t ii = 0; ii < _nSweeps; ii++) {
    _elevList.push_back(elevs[ii]);
  }
  
  return 0;

}

  
///////////////////////////////////
// read the times

int NoaaFslRadxFile::_readTimes()

{

  // read the time variable
  
  Nc3Var *timeVar = _file.getNc3File()->get_var("radialTime");
  if (timeVar == NULL) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readTimes");
    _addErrStr("  Cannot find time variable, name: ", "radialTime");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  if (timeVar->num_dims() < 1) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = timeVar->get_dim(0);
  if (timeDim != _radialDim) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim->name());
    return -1;
  }
  
  // get units attribute
  
  Nc3Att* unitsAtt = timeVar->get_att("units");
  if (unitsAtt == NULL) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }
  string units = Nc3xFile::asString(unitsAtt);
  delete unitsAtt;

  // parse the time units reference time

  RadxTime stime(units);
  _refTimeSecsFile = stime.utime();
  
  // set the time array
  
  RadxArray<double> dtimes_;
  double *dtimes = dtimes_.alloc(_nRadials);
  if (timeVar->get(dtimes, _nRadials) == 0) {
    _addErrStr("ERROdR - NoaaFslRadxFile::_readTimes");
    _addErrStr("  Candnot read Time variable");
    return -1;
  }

  // compute az speed
  double sumDeltaTime = 0.0;
  double sumDeltaAz = 0.0;
  for (size_t ii = 1; ii < _nRadials; ii++) {
    double deltaTime = dtimes[ii] - dtimes[ii-1];
    double deltaAz = fabs(RadxComplex::computeDiffDeg(_azimuth[ii], _azimuth[ii-1]));
    sumDeltaTime += deltaTime;
    sumDeltaAz += deltaAz;
  } // ii
  double meanDeltaTime = sumDeltaTime / (_nRadials - 1.0);
  _azSpeedDegPerSec = sumDeltaAz / sumDeltaTime;

  // set ray times

  _dTimes.clear();
  double rayTime = _startTime + meanDeltaTime / 2.0;
  for (size_t ii = 0; ii < _nTimes; ii++) {
    time_t secs = rayTime;
    double subSecs = rayTime - secs;
    _rayTimes.push_back(RadxTime(secs, subSecs));
    _dTimes.push_back(rayTime);
    rayTime += meanDeltaTime;
  }
  
  return 0;

}

///////////////////////////////////
// read in ray variables

int NoaaFslRadxFile::_readAzEl()

{

  _azimuth.clear();
  _elevation.clear();
  int iret = 0;

  // elevation
  
  Nc3Var *elevVar = _file.getNc3File()->get_var("radialElev");
  if (elevVar == NULL) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readAzEl");
    _addErrStr("  Cannot find elev angles variable, name: ", "radialElev");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  if (elevVar->num_dims() < 1) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readAzEl");
    _addErrStr("  radialElev variable has no dimensions");
    return -1;
  }
  Nc3Dim *radialDim = elevVar->get_dim(0);
  if (radialDim != _radialDim) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readAzEl");
    _addErrStr("  elevations has incorrect dimension, name: ", radialDim->name());
    return -1;
  }

  // read the elev array
  
  RadxArray<double> elev_;
  double *elev = elev_.alloc(_nRadials);
  if (elevVar->get(elev, _nRadials) == 0) {
    _addErrStr("ERROdR - NoaaFslRadxFile::_readAzEl");
    _addErrStr("  Candnot read elevation angles variable: radialElev");
    return -1;
  }
  for (size_t ii = 0; ii < _nRadials; ii++) {
    _elevation.push_back(elev[ii]);
  }

  // azimuth
  
  Nc3Var *azVar = _file.getNc3File()->get_var("radialAzim");
  if (azVar == NULL) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readAzEl");
    _addErrStr("  Cannot find az angles variable, name: ", "radialAzim");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  if (azVar->num_dims() < 1) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readAzEl");
    _addErrStr("  radialAzim variable has no dimensions");
    return -1;
  }
  if (radialDim != _radialDim) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readAzEl");
    _addErrStr("  azimuth has incorrect dimension, name: ", radialDim->name());
    return -1;
  }

  // read the az array
  
  RadxArray<double> az_;
  double *az = az_.alloc(_nRadials);
  if (azVar->get(az, _nRadials) == 0) {
    _addErrStr("ERROdR - NoaaFslRadxFile::_readAzEl");
    _addErrStr("  Candnot read azimuth angles variable: radialAzim");
    return -1;
  }
  for (size_t ii = 0; ii < _nRadials; ii++) {
    _azimuth.push_back(az[ii]);
  }
  
  if (iret) {
    _addErrStr("ERROR - NoaaFslRadxFile::_readAzEl");
    return -1;
  }
  
  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int NoaaFslRadxFile::_createRays(const string &path)

{

  // set up rays to read

  _raysFile.clear();
  
  for (size_t rayIndex = 0; rayIndex < _nTimes; rayIndex++) {
    
    RadxRay *ray = new RadxRay;
    
    double gateSpacingKm = _gateSpacingM / 1000.0;
    double startRangeKm = _startRangeM / 1000.0;
    ray->setRangeGeom(startRangeKm, gateSpacingKm);

    // set time
    
    ray->setTime(_rayTimes[rayIndex]);
    
    // sweep info

    int sweepNum = rayIndex / _nRadials;
    ray->setSweepNumber(sweepNum);

    int radialNum = rayIndex % _nRadials;
    ray->setElevationDeg(_elevList[sweepNum]);
    ray->setAzimuthDeg(_azimuth[radialNum]);

    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    ray->setPrtMode(Radx::PRT_MODE_FIXED);
    
    // add to ray vector

    _raysFile.push_back(ray);
    
  } // rayIndex

  return 0;

}

////////////////////////////////////////////
// read the field variables

int NoaaFslRadxFile::_readFieldVariables(bool metaOnly)

{
  
  // loop through the variables, adding data fields as appropriate
  
  for (int ivar = 0; ivar < _file.getNc3File()->num_vars(); ivar++) {
    
    Nc3Var* var = _file.getNc3File()->get_var(ivar);
    if (var == NULL) {
      continue;
    }
    
    int numDims = var->num_dims();

    // check that we have the correct dimensions

    if (numDims == 2) {
      Nc3Dim* radialDim = var->get_dim(0);
      Nc3Dim* rangeDim = var->get_dim(1);
      if (radialDim != _radialDim ||
          rangeDim != _rangeDim) {
        continue;
      }
    } else if (numDims == 3) {
      Nc3Dim* sweepDim = var->get_dim(0);
      Nc3Dim* radialDim = var->get_dim(1);
      Nc3Dim* rangeDim = var->get_dim(2);
      if (sweepDim != _sweepDim ||
          radialDim != _radialDim ||
          rangeDim != _rangeDim) {
        continue;
      }
    } else {
      continue;
    }
    
    string fieldName = var->name();
    Nc3Type ftype = var->type();

    // check the type
    // if (ftype != nc3Double && ftype != nc3Float) {
    //   // not a valid type
    //   if (_verbose) {
    //     cerr << "DEBUG - NoaaFslRadxFile::_readFieldVariables" << endl;
    //     cerr << "  -->> rejecting field: " << fieldName << endl;
    //     cerr << "  -->> Should be float or double: " << fieldName << endl;
    //   }
    //   continue;
    // }

    // check that we need this field

    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - NoaaFslRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - NoaaFslRadxFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << fieldName << endl;
    }
    
    // set names, units, etc
    
    string name = var->name();
    
    string longName;
    Nc3Att *longNameAtt = var->get_att("long_name");
    if (longNameAtt != NULL) {
      longName = Nc3xFile::asString(longNameAtt);
      delete longNameAtt;
    }

    string standardName;
    Nc3Att *standardNameAtt = var->get_att("standard_name");
    if (standardNameAtt != NULL) {
      standardName = Nc3xFile::asString(standardNameAtt);
      delete standardNameAtt;
    }
    
    string units;
    Nc3Att *unitsAtt = var->get_att("units");
    if (unitsAtt != NULL) {
      units = Nc3xFile::asString(unitsAtt);
      delete unitsAtt;
    }

    // scale, offset
    
    double scaleFactor = 1.0;
    Nc3Att *scaleAtt = var->get_att("scale_factor");
    if (scaleAtt != NULL) {
      scaleFactor = scaleAtt->values()[0].as_double(0);
      delete scaleAtt;
    }

    double addOffset = 0.0;
    Nc3Att *offsetAtt = var->get_att("add_offset");
    if (offsetAtt != NULL) {
      addOffset = offsetAtt->values()[0].as_double(0);
      delete offsetAtt;
    }

    // if metadata only, don't read in fields

    if (metaOnly) {
      if (!_readVol->fieldExists(name)) {
        RadxField *field = new RadxField(name, units);
        field->setLongName(longName);
        field->setStandardName(standardName);
        _readVol->addField(field);
      }
      continue;
    }

    int iret = 0;
    
    switch (ftype) {
      case nc3Double: {
        if (_addFl64FieldToRays(var, name, units,
                                standardName, longName)) {
          iret = -1;
        }
        break;
      }
      case nc3Float: {
        if (_addFl32FieldToRays(var, name, units,
                                standardName, longName)) {
          iret = -1;
        }
        break;
      }
      case nc3Int: {
        if (_addSi32FieldToRays(var, name, units,
                                standardName, longName,
                                scaleFactor, addOffset)) {
          iret = -1;
        }
        break;
      }
      case nc3Short: {
        if (_addSi16FieldToRays(var, name, units,
                                standardName, longName,
                                scaleFactor, addOffset)) {
          iret = -1;
        }
        break;
      }
      case nc3Byte: {
        if (_addSi08FieldToRays(var, name, units,
                                standardName, longName,
                                scaleFactor, addOffset)) {
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
      _addErrStr("ERROR - NoaaFslRadxFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  } // ivar

  return 0;

}

//////////////////////////////////////////////////////////////
// Add fl64 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NoaaFslRadxFile::_addFl64FieldToRays(Nc3Var* var,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName)
  
{

  // get data from array
  
  int numDims = var->num_dims();
  size_t nGatesTot = _nTimes * _nRangeInFile;
  Radx::fl64 *data = new Radx::fl64[nGatesTot];
  int iret = 0;
  if (numDims == 2) {
    iret = !var->get(data, _nRadials, _nRangeInFile);
  } else {
    iret = !var->get(data, _nSweeps, _nRadials, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }
  
  // set missing value

  Radx::fl64 missingVal = Radx::missingFl64;
  Nc3Att *missingValueAtt = var->get_att("_FillValue");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  } else {
    // check for -999.9 and -9999
    double nMiss_999 = 0;
    double nMiss_9999 = 0;
    for (size_t ii = 0; ii < nGatesTot; ii++) {
      if (fabs(data[ii] - -999.9) < 10) {
        nMiss_999++;
      } else if (fabs(data[ii] - -9999.0) < 10) { 
        nMiss_9999++;
      }
    }
    if (nMiss_999 > nMiss_9999) {
      missingVal = -999.9;
    } else if (nMiss_9999 > 0) {
      missingVal = -9999.0;
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysFile.size(); ii++) {
    
    size_t rayIndex = ii;
    
    if (rayIndex > _nTimes - 1) {
      cerr << "WARNING - NoaaFslRadxFile::_addFl64FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimes << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    
    RadxField *field =
      _raysFile[ii]->addField(name, units, nGates,
			      missingVal,
			      data + startIndex,
			      true);

    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    
  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add fl32 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NoaaFslRadxFile::_addFl32FieldToRays(Nc3Var* var,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName)
  
{

  // get data from array

  int numDims = var->num_dims();
  size_t nGatesTot = _nTimes * _nRangeInFile;
  Radx::fl32 *data = new Radx::fl32[nGatesTot];
  int iret = 0;
  if (numDims == 2) {
    iret = !var->get(data, _nRadials, _nRangeInFile);
  } else {
    iret = !var->get(data, _nSweeps, _nRadials, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value
  
  Radx::fl32 missingVal = Radx::missingFl32;
  Nc3Att *missingValueAtt = var->get_att("_FillValue");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  } else {
    // check for -999.9 and -9999
    double nMiss_999 = 0;
    double nMiss_9999 = 0;
    for (size_t ii = 0; ii < nGatesTot; ii++) {
      if (fabs(data[ii] - -999.9) < 10) {
        nMiss_999++;
      } else if (fabs(data[ii] - -9999.0) < 10) { 
        nMiss_9999++;
      }
    }
    if (nMiss_999 > nMiss_9999) {
      missingVal = -999.9;
    } else if (nMiss_9999 > 0) {
      missingVal = -9999.0;
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysFile.size(); ii++) {
    
    size_t rayIndex = ii;

    if (rayIndex > _nTimes - 1) {
      cerr << "WARNING - NoaaFslRadxFile::_addFl32FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimes << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;

    RadxField *field =
      _raysFile[ii]->addField(name, units, nGates,
			      missingVal,
			      data + startIndex,
			      true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si32 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NoaaFslRadxFile::_addSi32FieldToRays(Nc3Var* var,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName,
                                         double scaleFactor,
                                         double addOffset)
  
{

  // get data from array
  
  int numDims = var->num_dims();
  size_t nGatesTot = _nTimes * _nRangeInFile;
  Radx::si32 *data = new Radx::si32[nGatesTot];
  int iret = 0;
  if (numDims == 2) {
    iret = !var->get(data, _nRadials, _nRangeInFile);
  } else {
    iret = !var->get(data, _nSweeps, _nRadials, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value
  
  Radx::si32 missingVal = Radx::missingSi32;
  Nc3Att *missingValueAtt = var->get_att("_FillValue");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysFile.size(); ii++) {
    
    size_t rayIndex = ii;

    if (rayIndex > _nTimes - 1) {
      cerr << "WARNING - NoaaFslRadxFile::_addSi32FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimes << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    
    RadxField *field =
      _raysFile[ii]->addField(name, units, nGates,
			      missingVal,
			      data + startIndex,
                              scaleFactor, addOffset,
			      true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si16 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NoaaFslRadxFile::_addSi16FieldToRays(Nc3Var* var,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName,
                                         double scaleFactor,
                                         double addOffset)
  
{

  // get data from array
  
  int numDims = var->num_dims();
  size_t nGatesTot = _nTimes * _nRangeInFile;
  Radx::si16 *data = new Radx::si16[nGatesTot];
  int iret = 0;
  if (numDims == 2) {
    iret = !var->get(data, _nRadials, _nRangeInFile);
  } else {
    iret = !var->get(data, _nSweeps, _nRadials, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value
  
  Radx::si16 missingVal = Radx::missingSi16;
  Nc3Att *missingValueAtt = var->get_att("_FillValue");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysFile.size(); ii++) {
    
    size_t rayIndex = ii;

    if (rayIndex > _nTimes - 1) {
      cerr << "WARNING - NoaaFslRadxFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimes << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    
    RadxField *field =
      _raysFile[ii]->addField(name, units, nGates,
			      missingVal,
			      data + startIndex,
                              scaleFactor, addOffset,
			      true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si08 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NoaaFslRadxFile::_addSi08FieldToRays(Nc3Var* var,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName,
                                         double scaleFactor,
                                         double addOffset)
  
{

  // get data from array
  
  int numDims = var->num_dims();
  size_t nGatesTot = _nTimes * _nRangeInFile;
  Radx::si08 *data = new Radx::si08[nGatesTot];
  int iret = 0;
  if (numDims == 2) {
    iret = !var->get(data, _nRadials, _nRangeInFile);
  } else {
    iret = !var->get(data, _nSweeps, _nRadials, _nRangeInFile);
  }
  if (iret) {
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

  for (size_t ii = 0; ii < _raysFile.size(); ii++) {
    
    size_t rayIndex = ii;

    if (rayIndex > _nTimes - 1) {
      cerr << "WARNING - NoaaFslRadxFile::_addSi08FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimes << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    
    RadxField *field =
      _raysFile[ii]->addField(name, units, nGates,
			      missingVal,
			      data + startIndex,
                              scaleFactor, addOffset,
			      true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

  }
  
  delete[] data;
  return 0;
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int NoaaFslRadxFile::_loadReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("NOAAFSL");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  _readVol->setTitle(_title);
  _readVol->setSource(_source);
  _readVol->setHistory(_history);
  _readVol->setInstitution(_institution);
  _readVol->setReferences(_references);
  _readVol->setComment(_comment);
  _readVol->setStatusXml(_statusXml);

  _readVol->setLatitudeDeg(_latitudeDeg);
  _readVol->setLongitudeDeg(_longitudeDeg);
  _readVol->setAltitudeKm(_altitudeM / 1000.0);

  _readVol->setRadarBeamWidthDegH(_beamWidthH);
  _readVol->setRadarBeamWidthDegV(_beamWidthV);
  _readVol->setRadarReceiverBandwidthMhz(_bandWidthHertz / 1.0e6);

  _readVol->copyRangeGeom(_geom);
  
  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    _raysVol[ii]->setVolumeNumber(_volumeNumber);
    _raysVol[ii]->setPulseWidthUsec(_pulseWidthUsec);
    _raysVol[ii]->setNyquistMps(_nyquistVel);
  }

  // add rays to vol - they will be freed by vol

  for (size_t ii = 0; ii < _raysVol.size(); ii++) {
    _readVol->addRay(_raysVol[ii]);
  }

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _raysVol.clear();
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();

  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - NoaaFslRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - NoaaFslRadxFile::_loadReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // calibration

  RadxRcalib *calib = new RadxRcalib;
  calib->setRadarConstantH(_radarConst);
  _readVol->addCalib(calib);

  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();
  
  // check for indexed rays, set info on rays

  _readVol->checkForIndexedRays();

  // compute fixed angles as mean angle from sweeps
  
  _computeFixedAngles();

  // set the sweep mode from rays
  
  _readVol->setSweepScanModeFromRayAngles();
    
  return 0;

}

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void NoaaFslRadxFile::_computeFixedAngles()
  
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

