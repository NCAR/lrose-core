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
// D3rNcRadxFile.cc
//
// D3rNcRadxFile object
//
// NetCDF file data for CSU/NASA D2R radial data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2015
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/D3rNcRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
using namespace std;

int D3rNcRadxFile::_prevSweepNumber = -1;
int D3rNcRadxFile::_volumeNumber = -1;

//////////////
// Constructor

D3rNcRadxFile::D3rNcRadxFile() : RadxFile()
  
{

  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

D3rNcRadxFile::~D3rNcRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void D3rNcRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _timeDim = NULL;
  _rangeDim = NULL;

  _timeVar = NULL;
  _iTimes.clear();
  _dTimes.clear();
  _rayTimesIncrease = true;
  _nTimesInFile = 0;

  _raysVol.clear();
  _raysFile.clear();

  _rangeKm.clear();
  _nRangeInFile = 0;
  _gateSpacingIsConstant = true;

  _azimuthVar = NULL;
  _elevationVar = NULL;
  _gateWidthVar = NULL;
  _startRangeVar = NULL;

  _azimuth.clear();
  _elevation.clear();
  _gateWidth.clear();
  _startRangeInt.clear();
  _startRange.clear();

  _startGateVar = NULL;
  _gcfStateVar = NULL;
  _polarizationModeVar = NULL;
  _prtModeVar = NULL;

  _startGate.clear();
  _gcfState.clear();
  _polarizationMode.clear();
  _prtMode.clear();
  
  _txFreqShortVar = NULL;
  _txFreqMediumVar = NULL;
  _txLengthShortVar = NULL;
  _txLengthMediumVar = NULL;
  
  _txFreqShort.clear();
  _txFreqMedium.clear();
  _txLengthShort.clear();
  _txLengthMedium.clear();
  
  _txPowerHShortVar = NULL;
  _txPowerHMediumVar = NULL;
  _txPowerVShortVar = NULL;
  _txPowerVMediumVar = NULL;

  _txPowerHShort.clear();
  _txPowerHMedium.clear();
  _txPowerVShort.clear();
  _txPowerVMedium.clear();

  _txPhaseHShortVar = NULL;
  _txPhaseHMediumVar = NULL;
  _txPhaseVShortVar = NULL;
  _txPhaseVMediumVar = NULL;
  
  _txPhaseHShort.clear();
  _txPhaseHMedium.clear();
  _txPhaseVShort.clear();
  _txPhaseVMedium.clear();

  _noiseSourcePowerHShortVar = NULL;
  _noiseSourcePowerVShortVar = NULL;

  _noiseSourcePowerHShort.clear();
  _noiseSourcePowerVShort.clear();

  _rxGainHShortVar = NULL;
  _rxGainHMediumVar = NULL;
  _rxGainVShortVar = NULL;
  _rxGainVMediumVar = NULL;
  
  _rxGainHShort.clear();
  _rxGainHMedium.clear();
  _rxGainVShort.clear();
  _rxGainVMedium.clear();
  
  _zdrBiasAppliedShortVar = NULL;
  _zdrBiasAppliedMediumVar = NULL;
  
  _zdrBiasAppliedShort.clear();
  _zdrBiasAppliedMedium.clear();

  // global attributes

  _netcdfRevision.clear();
  _gmaptdRevision.clear();
  _configRevision.clear();
  _campaignName.clear();
  _radarName.clear();
  
  _latitudeDeg = 0;
  _longitudeDeg = 0;
  _altitudeKm = 0;

  _numGates = 0;
  _scanId = 0;
  _scanType = 0;
  _sweepNumber = 0;

  _refTimeSecsFile = 0;
  
  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();
  _statusXml.clear();
  
  _siteName.clear();
  _scanName.clear();
  // int _scanId;
  _instrumentName.clear();
  
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool D3rNcRadxFile::isSupported(const string &path)

{
  
  if (isD3rNc(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a D3rNc file
// Returns true on success, false on failure

bool D3rNcRadxFile::isD3rNc(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_verbose) {
      cerr << "DEBUG - not D3rNc file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not D3rNc file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check existence of some variables

  Nc3Var *var1 = _file.getNc3File()->get_var("GateWidth");
  if (var1 == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not D3rNc file" << endl;
      cerr << "  GateWidth variable missing" << endl;
    }
    return false;
  }

  Nc3Var *var2 = _file.getNc3File()->get_var("TxFrequency_Short");
  if (var2 == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not D3rNc file" << endl;
      cerr << "  TxFrequency_Short variable missing" << endl;
    }
    return false;
  }

  Nc3Var *var3 = _file.getNc3File()->get_var("TxLength_Short");
  if (var3 == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not D3rNc file" << endl;
      cerr << "  TxLength_Short variable missing" << endl;
    }
    return false;
  }

  Nc3Var *var4 = _file.getNc3File()->get_var("StartGate_Short");
  if (var4 == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not D3rNc file" << endl;
      cerr << "  StartGate_Short variable missing" << endl;
    }
    return false;
  }

  // file has the correct dims and variables, so it is a D3rNc file

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

int D3rNcRadxFile::writeToDir(const RadxVol &vol,
                              const string &dir,
                              bool addDaySubDir,
                              bool addYearSubDir)
  
{

  // Writing D3rNc files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - D3rNcRadxFile::writeToDir" << endl;
  cerr << "  Writing D3rNc format files not supported" << endl;
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

int D3rNcRadxFile::writeToPath(const RadxVol &vol,
                               const string &path)
  
{

  // Writing D3rNc files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - D3rNcRadxFile::writeToPath" << endl;
  cerr << "  Writing D3rNc format files not supported" << endl;
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

int D3rNcRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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
    char cc;
    if (sscanf(start, "%4d%2d%2d%1c%2d%2d%2d",
               &year, &month, &day, &cc, &hour, &min, &sec) == 7) {
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

void D3rNcRadxFile::print(ostream &out) const
  
{
  
  out << "=============== D3rNcRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "  title: " << _title << endl;
  out << "  institution: " << _institution << endl;
  out << "  references: " << _references << endl;
  out << "  source: " << _source << endl;
  out << "  history: " << _history << endl;
  out << "  comment: " << _comment << endl;
  out << "  statusXml: " << _statusXml << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  scanName: " << _scanName << endl;
  out << "  scanId: " << _scanId << endl;
  out << "  instrumentName: " << _instrumentName << endl;
  out << "  refTime: " << RadxTime::strm((time_t) _refTimeSecsFile) << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " 
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  out << "  latitude: " << _latitudeDeg << endl;
  out << "  longitude: " << _longitudeDeg << endl;
  out << "  altitude: " << _altitudeKm << endl;
  if (_txFreqShort.size() > 0) {
    out << "  frequencyGhz: " << _txFreqShort[0] / 1.0e9 << endl;
  }
  out << "  startRangeKm: " << _remap.getStartRangeKm() << endl;
  out << "  gateSpacingKm: " << _remap.getGateSpacingKm() << endl;
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int D3rNcRadxFile::printNative(const string &path, ostream &out,
                               bool printRays, bool printData)
  
{

  _addErrStr("ERROR - D3rNcRadxFile::printNative");
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

int D3rNcRadxFile::readFromPath(const string &path,
                                RadxVol &vol)
  
{

  // initialize

  _initForRead(path, vol);
  clear();
  
  // Check if this is a D3rNc file

  if (!isD3rNc(path)) {
    _addErrStr("ERROR - D3rNcRadxFile::readFromPath");
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
      _addErrStr("ERROR - D3rNcRadxFile::readFromPath");
      return -1;
    }
  } else {
    if (_readFile(path)) {
      _addErrStr("ERROR - D3rNcRadxFile::readFromPath");
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

int D3rNcRadxFile::_readAggregatePaths(const string &path)
  
{

  // cerr << "11111111111 path: " << path << endl;
  
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

int D3rNcRadxFile::_readFile(const string &path)
  
{

  string errStr("ERROR - D3rNcRadxFile::readFromPath");
  
  // clear tmp vars
  
  _nTimesInFile = 0;
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

  // read time variable now if that is all that is needed
  
  if (_readTimesOnly) {
    if (_readTimes()) {
      _addErrStr(errStr);
      return -1;
    }
    return 0;
  }
  
  // read global attributes
  
  if (_readGlobalAttributes()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read time variable
  
  if (_readTimes()) {
    _addErrStr(errStr);
    return -1;
  }

  // read range variable
  // the range array size will be the max of the arrays found in
  // the files
  
  if (_readRangeVariables()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read in ray variables

  if (_readRayVariables()) {
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

  _clearRayVariables();

  return 0;

}

//////////////////////////////////////////////////////////////
// get list of paths for the same volume as the specified path
//
// We search for files contiguous in time and with the sweep
// numbers sequentially

void D3rNcRadxFile::_getVolumePaths(const string &path,
				    vector<string> &paths)
  
{
  
  paths.clear();

  // get file time
  
  RadxTime refTime;
  getTimeFromPath(path, refTime);
  int refHour = refTime.getHour();

  // find all files in the same day
  // directory as the specified path, within 1 hour of the time
  
  vector<string> pathList;
  RadxPath rpath(path);
  string dir = rpath.getDirectory();
  _addToPathList(dir, refTime, pathList);

  RadxPath dpath(dir);
  string parentDir = dpath.getDirectory();

  // if time is close to start of day, search previous directory

  if (refHour == 0) {
    RadxTime prevDate(refTime.utime() - RadxTime::RADX_SECS_IN_DAY);
    char prevDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(prevDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            prevDate.getYear(), prevDate.getMonth(), prevDate.getDay());
    _addToPathList(prevDir, refTime, pathList);
  }

  // if time is close to end of day, search previous directory

  if (refHour == 23) {
    RadxTime nextDate(refTime.utime() + RadxTime::RADX_SECS_IN_DAY);
    char nextDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(nextDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            nextDate.getYear(), nextDate.getMonth(), nextDate.getDay());
    _addToPathList(nextDir, refTime, pathList);
  }
  
  // sort the path list
  
  sort(pathList.begin(), pathList.end());

  // for (int ii = 0; ii < (int) pathList.size(); ii++) {
  //   cerr << "4444444444 ii, path: " << ii << ", " << pathList[ii] << endl;
  // }

  // find the index of the requested file in the path list

  string fileName = rpath.getFile();
  int refIndex = -1;
  for (int ii = 0; ii < (int) pathList.size(); ii++) {
    if (pathList[ii].find(fileName) != string::npos) {
      refIndex = ii;
    }
  }

  // cerr << "4444444444 refIndex: " << refIndex << endl;
  
  if (refIndex < 0) {
    paths.push_back(path);
    return;
  }
  
  // get the sweep number for this file
  
  int referenceSweepNum = _readSweepNumber(path);
  
  // find the start and end refIndex of the paths
  // for the same volume by checking for
  // a discontinuity in the sweep numbers

  int startIndex = 0;
  int prevSweepNum = referenceSweepNum;
  for (int ii = refIndex - 1; ii >= 0; ii--) {
    int sweepNum = _readSweepNumber(pathList[ii]);
    if (ii == 0) {
      startIndex = ii;
    } else {
      if (sweepNum < prevSweepNum) {
	prevSweepNum = sweepNum;
      } else {
	startIndex = ii + 1;
	break;
      }
    }
  }
  // cerr << "4444444444 startIndex: " << startIndex << endl;
  
  int endIndex = (int) pathList.size() - 1;
  for (int ii = refIndex + 1; ii < (int) pathList.size(); ii++) {
    int sweepNum = _readSweepNumber(pathList[ii]);
    if (ii == (int) (pathList.size() - 1)) {
      endIndex = ii;
    } else {
      if (sweepNum > prevSweepNum) {
	prevSweepNum = sweepNum;
      } else {
	endIndex = ii - 1;
	break;
      }
    }
  } // ii
  // cerr << "4444444444 endIndex: " << endIndex << endl;
  
  // load up all paths from start to end index

  for (int ii = startIndex; ii <= endIndex; ii++) {
    paths.push_back(pathList[ii]);
  }

}

///////////////////////////////////////////////////////////
// add to the path list, files with 1 hour of ref time

void D3rNcRadxFile::_addToPathList(const string &dir,
				   const RadxTime &refTime,
				   vector<string> &paths)
  
{

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

    RadxTime ftime;
    getTimeFromPath(fileName, ftime);
    double tdiff = fabs(ftime - refTime);
    if (tdiff < 3600) {
      // cerr << "33333333333333 fileName: " << fileName << endl;
      string filePath = dir;
      filePath += RadxPath::RADX_PATH_DELIM;
      filePath += fileName;
      paths.push_back(filePath);
    }

  } // dp

  closedir(dirp);

}

///////////////////////////////////
// read in the dimensions

int D3rNcRadxFile::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  iret |= _file.readDim("Radial", _timeDim);
  if (iret == 0) {
    _nTimesInFile = _timeDim->size();
  }

  _nRangeInFile = 0;
  iret |= _file.readDim("Gate", _rangeDim);
  if (iret == 0) {
    _nRangeInFile = _rangeDim->size();
  }
  
  if (iret) {
    _addErrStr("ERROR - D3rNcRadxFile::_file.readDimensions");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read the sweep number for a file

int D3rNcRadxFile::_readSweepNumber(const string &path)

{
  int sweepNumber = -1;
  Nc3xFile file;
  if (file.openRead(path)) {
    return -1;
  }
  file.readGlobAttr("SweepNumber", sweepNumber);
  file.close();
  return sweepNumber;
}

///////////////////////////////////
// read the global attributes

int D3rNcRadxFile::_readGlobalAttributes()

{

  _file.readGlobAttr("NetCDFRevision", _netcdfRevision);
  _file.readGlobAttr("GMAPTDRevision", _gmaptdRevision);
  _file.readGlobAttr("ConfigRevision", _configRevision);
  _file.readGlobAttr("CampaignName", _campaignName);
  _file.readGlobAttr("RadarName", _radarName);
  _file.readGlobAttr("Latitude", _latitudeDeg);
  _file.readGlobAttr("Longitude", _longitudeDeg);
  _file.readGlobAttr("Altitude", _altitudeKm);
  _altitudeKm /= 1000.0; // meters to km
  _file.readGlobAttr("NumGates", _numGates);
  _file.readGlobAttr("ScanId", _scanId);
  _file.readGlobAttr("ScanType", _scanType);
  _file.readGlobAttr("ScanType", _scanName);
  _file.readGlobAttr("SweepNumber", _sweepNumber);
  _file.readGlobAttr("Time", _refTimeSecsFile);

  _prevSweepNumber = _sweepNumber;

  _title = _netcdfRevision;
  _institution = "CSU/NASA";
  _references = _configRevision;
  _source = _radarName;
  _history = _campaignName;
  // _comment = "";
  _siteName = _campaignName;
  _instrumentName = _radarName;
  
  // set the status XML from the attributes

  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("STATUS", 0);
  for (int ii = 0; ii < _file.getNc3File()->num_atts(); ii++) {
    Nc3Att *att = _file.getNc3File()->get_att(ii);
    if (att != NULL) {
      const char* strc = att->as_string(0);
      string val(strc);
      delete[] strc;
      string name(att->name());
      delete att;
      _statusXml += RadxXml::writeString(name, 1, val);
    }
  }
  _statusXml += RadxXml::writeEndTag("STATUS", 0);

  return 0;

}

///////////////////////////////////
// read the times

int D3rNcRadxFile::_readTimes()

{

  // read the time variable

  _timeVar = _file.getNc3File()->get_var("Time");
  if (_timeVar == NULL) {
    _addErrStr("ERROR - D3rNcRadxFile::_readTimes");
    _addErrStr("  Cannot find time variable, name: ", "Time");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  if (_timeVar->num_dims() < 1) {
    _addErrStr("ERROR - D3rNcRadxFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = _timeVar->get_dim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - D3rNcRadxFile::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim->name());
    return -1;
  }

  // get units attribute
  
  Nc3Att* unitsAtt = _timeVar->get_att("Units");
  if (unitsAtt == NULL) {
    _addErrStr("ERROR - D3rNcRadxFile::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }
  string units = Nc3xFile::asString(unitsAtt);
  delete unitsAtt;

  // parse the time units reference time

  RadxTime stime(units);
  _refTimeSecsFile = stime.utime();
  
  // set the time array
  
  RadxArray<int> itimes_;
  int *itimes = itimes_.alloc(_nTimesInFile);
  if (_timeVar->get(itimes, _nTimesInFile) == 0) {
    _addErrStr("ERROR - D3rNcRadxFile::_readTimes");
    _addErrStr("  Cannot read Time variable");
    return -1;
  }
  _dTimes.clear();
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    _iTimes.push_back(itimes[ii]);
    _dTimes.push_back(itimes[ii]);
  }
  
  return 0;

}

///////////////////////////////////
// read the range-related variables

int D3rNcRadxFile::_readRangeVariables()

{
  
  // start range in km

  if (_readRayVar(_startRangeVar, "StartRange", _startRangeUnits, _startRangeInt, true)) {
    _addErrStr("ERROR - D3rNcRadxFile::_readRangeVariable");
    _addErrStr("  Cannot read StartRange");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  double kmPerUnitStartRange = 1.0; // default - units in km
  for (size_t ii = 0; ii < _startRangeUnits.size(); ii++) {
    _startRangeUnits[ii] = tolower(_startRangeUnits[ii]);
  }
  if (_startRangeUnits == "m" || _startRangeUnits == "meters") {
    kmPerUnitStartRange = 0.001;
  } else if (_startRangeUnits == "mm" || _startRangeUnits == "millimeters") {
    kmPerUnitStartRange = 0.000001;
  }

  for (size_t ii = 0; ii < _startRangeInt.size(); ii++) {
    _startRange.push_back(_startRangeInt[ii] * kmPerUnitStartRange);
  }
  
  // gate width in km
  
  if (_readRayVar(_gateWidthVar, "GateWidth", _gateWidthUnits, _gateWidth, true)) {
    _addErrStr("ERROR - D3rNcRadxFile::_readRangeVariable");
    _addErrStr("  Cannot read GateWidth");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  double kmPerUnitGateWidth = 1.0; // default - units in km
  for (size_t ii = 0; ii < _gateWidthUnits.size(); ii++) {
    _gateWidthUnits[ii] = tolower(_gateWidthUnits[ii]);
  }
  if (_gateWidthUnits == "m" || _gateWidthUnits == "meters") {
    kmPerUnitGateWidth = 0.001;
  } else if (_gateWidthUnits == "mm" || _gateWidthUnits == "millimeters") {
    kmPerUnitGateWidth = 0.000001;
  }

  for (size_t ii = 0; ii < _gateWidth.size(); ii++) {
    _gateWidth[ii] *= kmPerUnitGateWidth;
  }
  
  // start gate
  
  if (_readRayVar(_startGateVar, "StartGate", _startGate, true)) {
    _startGate.clear();
    for (size_t ii = 0; ii < _startRange.size(); ii++) {
      _startGate[ii] = 1;
    }
  }

  // set range vector

  _rangeKm.clear();
  double gateSpacingKm = _gateWidth[0];
  double startRangeKm = _startRange[0] + gateSpacingKm * (_startGate[0] - 1);
  for (size_t ii = 0; ii < _nRangeInFile; ii++) {
    _rangeKm.push_back(startRangeKm + ii * gateSpacingKm);
  }
  
  // set the geometry from the range vector
  
  _gateSpacingIsConstant = true;
  _geom.setRangeGeom(startRangeKm, gateSpacingKm);

  return 0;

}

///////////////////////////////////
// clear the ray variables

void D3rNcRadxFile::_clearRayVariables()

{

  _azimuth.clear();
  _elevation.clear();
  _gateWidth.clear();
  _startRange.clear();

  _startGate.clear();
  _gcfState.clear();
  _polarizationMode.clear();
  _prtMode.clear();
  
  _txFreqShort.clear();
  _txFreqMedium.clear();
  _txLengthShort.clear();
  _txLengthMedium.clear();
  
  _txPowerHShort.clear();
  _txPowerHMedium.clear();
  _txPowerVShort.clear();
  _txPowerVMedium.clear();

  _txPhaseHShort.clear();
  _txPhaseHMedium.clear();
  _txPhaseVShort.clear();
  _txPhaseVMedium.clear();

  _noiseSourcePowerHShort.clear();
  _noiseSourcePowerVShort.clear();

  _rxGainHShort.clear();
  _rxGainHMedium.clear();
  _rxGainVShort.clear();
  _rxGainVMedium.clear();
  
  _zdrBiasAppliedShort.clear();
  _zdrBiasAppliedMedium.clear();

}

///////////////////////////////////
// read in ray variables

int D3rNcRadxFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  _readRayVar(_azimuthVar, "Azimuth", _azimuthUnits, _azimuth);
  if ((int) _azimuth.size() != _timeDim->size()) {
    _addErrStr("ERROR - Azimuth variable required");
    iret = -1;
  }
  
  _readRayVar(_elevationVar, "Elevation", _elevationUnits, _elevation);
  if ((int) _elevation.size() != _timeDim->size()) {
    _addErrStr("ERROR - Elevation variable required");
    iret = -1;
  }

  _readRayVar(_gcfStateVar, "GcfState", _gcfState, false);
  _readRayVar(_polarizationModeVar, "PolarizationMode", _polarizationMode, false);
  _readRayVar(_prtModeVar, "PRTMode", _prtMode, false);
  
  _readRayVar(_txFreqShortVar, "TxFrequency_Short",
	      _txFreqShortUnits, _txFreqShort, false);
  _readRayVar(_txFreqMediumVar, "TxFrequency_Medium",
	      _txFreqMediumUnits, _txFreqMedium, false);

  _readRayVar(_txLengthShortVar, "TxLength_Short",
	      _txLengthShortUnits, _txLengthShort, false);
  _readRayVar(_txLengthMediumVar, "TxLength_Medium",
	      _txLengthMediumUnits, _txLengthMedium, false);
  
  _readRayVar(_txPowerHShortVar, "TxPowerH_Short",
	      _txPowerHShortUnits, _txPowerHShort, false);
  _readRayVar(_txPowerHMediumVar, "TxPowerH_Medium",
	      _txPowerHMediumUnits, _txPowerHMedium, false);
  _readRayVar(_txPowerVShortVar, "TxPowerV_Short",
	      _txPowerVShortUnits, _txPowerVShort, false);
  _readRayVar(_txPowerVMediumVar, "TxPowerV_Medium",
	      _txPowerVMediumUnits, _txPowerVMedium, false);

  _readRayVar(_txPhaseHShortVar, "TxPhaseH_Short",
	      _txPhaseHShortUnits, _txPhaseHShort, false);
  _readRayVar(_txPhaseHMediumVar, "TxPhaseH_Medium",
	      _txPhaseHMediumUnits, _txPhaseHMedium, false);
  _readRayVar(_txPhaseVShortVar, "TxPhaseV_Short",
	      _txPhaseVShortUnits, _txPhaseVShort, false);
  _readRayVar(_txPhaseVMediumVar, "TxPhaseV_Medium",
	      _txPhaseVMediumUnits, _txPhaseVMedium, false);

  _readRayVar(_noiseSourcePowerHShortVar, "NoiseSourcePowerH_Short",
	      _noiseSourcePowerHShortUnits, _noiseSourcePowerHShort, false);
  _readRayVar(_noiseSourcePowerVShortVar, "NoiseSourcePowerV_Short",
	      _noiseSourcePowerVShortUnits, _noiseSourcePowerVShort, false);

  _readRayVar(_rxGainHShortVar, "TxPowerH_Short",
	      _rxGainHShortUnits, _rxGainHShort, false);
  _readRayVar(_rxGainHMediumVar, "TxPowerH_Medium",
	      _rxGainHMediumUnits, _rxGainHMedium, false);
  _readRayVar(_rxGainVShortVar, "TxPowerV_Short",
	      _rxGainVShortUnits, _rxGainVShort, false);
  _readRayVar(_rxGainVMediumVar, "TxPowerV_Medium",
	      _rxGainVMediumUnits, _rxGainVMedium, false);

  _readRayVar(_zdrBiasAppliedShortVar, "ZDRBiasApplied_Short",
	      _zdrBiasAppliedShortUnits, _zdrBiasAppliedShort, false);
  _readRayVar(_zdrBiasAppliedMediumVar, "ZDRBiasApplied_Medium",
	      _zdrBiasAppliedMediumUnits, _zdrBiasAppliedMedium, false);

  if (iret) {
    _addErrStr("ERROR - D3rNcRadxFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int D3rNcRadxFile::_createRays(const string &path)

{

  // set up rays to read

  _raysFile.clear();
  
  for (size_t rayIndex = 0; rayIndex < _dTimes.size(); rayIndex++) {
    
    RadxRay *ray = new RadxRay;
    
    double gateSpacingKm = _gateWidth[rayIndex];
    double startRangeKm =
      _startRange[rayIndex] + gateSpacingKm * (_startGate[rayIndex] - 1);
    ray->setRangeGeom(startRangeKm, gateSpacingKm);

    // set time
    
    // double rayTimeDouble = _dTimes[rayIndex];
    time_t rayUtimeSecs = _iTimes[rayIndex];
    int rayNanoSecs = 0;
    ray->setTime(rayUtimeSecs, rayNanoSecs);
    
    // sweep info
    
    ray->setSweepNumber(_sweepNumber);
    ray->setAzimuthDeg(_azimuth[rayIndex]);
    ray->setElevationDeg(_elevation[rayIndex]);
    
    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    ray->setPolarizationMode(Radx::POL_MODE_HV_SIM);
    ray->setPrtMode(Radx::PRT_MODE_FIXED);
    
    if (_noiseSourcePowerHShort.size() > rayIndex) {
      ray->setEstimatedNoiseDbmHc(_noiseSourcePowerHShort[rayIndex]);
    }
    
    if (_noiseSourcePowerVShort.size() > rayIndex) {
      ray->setEstimatedNoiseDbmVc(_noiseSourcePowerVShort[rayIndex]);
    }
    
    // add to ray vector

    _raysFile.push_back(ray);
    
  } // rayIndex

  return 0;

}

////////////////////////////////////////////
// read the field variables

int D3rNcRadxFile::_readFieldVariables(bool metaOnly)

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
    if (ftype != nc3Double && ftype != nc3Float) {
      // not a valid type
      if (_verbose) {
        cerr << "DEBUG - D3rNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
        cerr << "  -->> Should be float or double: " << fieldName << endl;
      }
      continue;
    }

    // check that we need this field

    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - D3rNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - D3rNcRadxFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << fieldName << endl;
    }
    
    // set names, units, etc
    
    string name = var->name();
    
    string standardName;
    Nc3Att *standardNameAtt = var->get_att("standard_name");
    if (standardNameAtt != NULL) {
      standardName = Nc3xFile::asString(standardNameAtt);
      delete standardNameAtt;
    }
    
    string longName;
    Nc3Att *longNameAtt = var->get_att("long_name");
    if (longNameAtt != NULL) {
      longName = Nc3xFile::asString(longNameAtt);
      delete longNameAtt;
    }

    string units;
    Nc3Att *unitsAtt = var->get_att("Units");
    if (unitsAtt != NULL) {
      units = Nc3xFile::asString(unitsAtt);
      delete unitsAtt;
    }

    // if metadata only, don't read in fields

    if (metaOnly) {
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
        field->setStandardName(standardName);
        _readVol->addField(field);
      }
      continue;
    }

    int iret = 0;
    bool isDiscrete = false;
    bool fieldFolds = false;
    double foldLimitLower = 0.0;
    double foldLimitUpper = 0.0;
    
    switch (var->type()) {
      case nc3Double: {
        if (_addFl64FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case nc3Float: {
        if (_addFl32FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
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
      _addErrStr("ERROR - D3rNcRadxFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  } // ivar

  return 0;

}

///////////////////////////////////
// read a ray variable - double

int D3rNcRadxFile::_readRayVar(Nc3Var* &var,
			       const string &name,
			       string &units,
                               vector<double> &vals,
			       bool required)
  
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
      _addErrStr("ERROR - D3rNcRadxFile::_readRayVar");
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
      _addErrStr("ERROR - D3rNcRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;

  // get units

  Nc3Att* unitsAtt = var->get_att("Units");
  if (unitsAtt != NULL) {
    units = Nc3xFile::asString(unitsAtt);
    delete unitsAtt;
  } else {
    units.clear();
  }
  
  return iret;

}

////////////////////////////////////////////
// read a ray variable - integer with units

int D3rNcRadxFile::_readRayVar(Nc3Var* &var,
			       const string &name,
			       string &units,
                               vector<int> &vals,
			       bool required)

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
      _addErrStr("ERROR - D3rNcRadxFile::_readRayVar");
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
      _addErrStr("ERROR - D3rNcRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;

  // get units
  
  Nc3Att* unitsAtt = var->get_att("Units");
  if (unitsAtt != NULL) {
    units = Nc3xFile::asString(unitsAtt);
    delete unitsAtt;
  } else {
    units.clear();
  }
  
  return iret;

}

////////////////////////////////////////////
// read a ray variable - integer without units

int D3rNcRadxFile::_readRayVar(Nc3Var* &var,
			       const string &name,
                               vector<int> &vals,
			       bool required)

{
  string units;
  return _readRayVar(var, name, units, vals, required);
}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

Nc3Var* D3rNcRadxFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - D3rNcRadxFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - D3rNcRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - D3rNcRadxFile::_getRayVar");
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

int D3rNcRadxFile::_addFl64FieldToRays(Nc3Var* var,
                                       const string &name,
                                       const string &units,
                                       const string &standardName,
                                       const string &longName,
                                       bool isDiscrete,
                                       bool fieldFolds,
                                       float foldLimitLower,
                                       float foldLimitUpper)
  
{

  // get data from array

  size_t nGatesTot = _nTimesInFile * _nRangeInFile;
  Radx::fl64 *data = new Radx::fl64[nGatesTot];
  int iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::fl64 missingVal = Radx::missingFl64;
  Nc3Att *missingValueAtt = var->get_att("missing_value");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  } else {
    for (size_t ii = 0; ii < nGatesTot; ii++) {
      if (!isfinite(data[ii])) {
        data[ii] = missingVal;
      }
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysFile.size(); ii++) {
    
    size_t rayIndex = ii;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - D3rNcRadxFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
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
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add fl32 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int D3rNcRadxFile::_addFl32FieldToRays(Nc3Var* var,
                                       const string &name,
                                       const string &units,
                                       const string &standardName,
                                       const string &longName,
                                       bool isDiscrete,
                                       bool fieldFolds,
                                       float foldLimitLower,
                                       float foldLimitUpper)
  
{

  // get data from array

  size_t nGatesTot = _nTimesInFile * _nRangeInFile;
  Radx::fl32 *data = new Radx::fl32[nGatesTot];
  int iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value
  
  Radx::fl32 missingVal = Radx::missingFl32;
  Nc3Att *missingValueAtt = var->get_att("missing_value");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  } else {
    for (size_t ii = 0; ii < nGatesTot; ii++) {
      if (!isfinite(data[ii])) {
        data[ii] = missingVal;
      }
    }
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysFile.size(); ii++) {
    
    size_t rayIndex = ii;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - D3rNcRadxFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
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

    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int D3rNcRadxFile::_loadReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("D3R");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  if (_txFreqShort.size() > 0) {
    _readVol->addFrequencyHz(_txFreqShort[0]);
  }
  if (_txFreqMedium.size() > 0) {
    _readVol->addFrequencyHz(_txFreqMedium[0]);
  }

  _readVol->setTitle(_title);
  _readVol->setSource(_source);
  _readVol->setHistory(_history);
  _readVol->setInstitution(_institution);
  _readVol->setReferences(_references);
  _readVol->setComment(_comment);
  _readVol->setStatusXml(_statusXml);
  _readVol->setSiteName(_siteName);
  _readVol->setScanName(_scanName);
  _readVol->setScanId(_scanId);
  _readVol->setInstrumentName(_instrumentName);

  _readVol->setLatitudeDeg(_latitudeDeg);
  _readVol->setLongitudeDeg(_longitudeDeg);
  _readVol->setAltitudeKm(_altitudeKm);

  _readVol->copyRangeGeom(_geom);

  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    _raysVol[ii]->setVolumeNumber(_volumeNumber);
  }

  // add rays to vol - they will be freed by vol

  for (size_t ii = 0; ii < _raysVol.size(); ii++) {

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
      _addErrStr("ERROR - D3rNcRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - D3rNcRadxFile::_loadReadVolume");
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

  // compute fixed angles as mean angle from sweeps
  
  _computeFixedAngles();

  // set the sweep mode from rays
  
  _readVol->setSweepScanModeFromRayAngles();
    
  return 0;

}

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void D3rNcRadxFile::_computeFixedAngles()
  
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

