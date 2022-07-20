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
// MslFile.cc
//
// For reading
// NetCDF file data for HSRL radial data from UW
// in MSL vert coords
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2015
//
///////////////////////////////////////////////////////////////

#include "MslFile.hh"
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

//////////////
// Constructor

MslFile::MslFile(const Params &params) :
        RadxFile(),
        _params(params)
  
{

  _conventions = BaseConvention;
  _version = CurrentVersion;

  _ncFormat = NETCDF_CLASSIC;

  _writeVol = NULL;
  _readVol = NULL;

  clear();

}

/////////////
// destructor

MslFile::~MslFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void MslFile::clear()
  
{

  clearErrStr();

  _file.close();

  _timeDim = NULL;
  _rangeDim = NULL;
  _nPointsDim = NULL;
  _sweepDim = NULL;
  _calDim = NULL;
  _stringLen8Dim = NULL;
  _stringLen32Dim = NULL;
  _frequencyDim = NULL;
  
  _volumeNumberVar = NULL;
  _instrumentTypeVar = NULL;
  _platformTypeVar = NULL;
  _primaryAxisVar = NULL;
  _startTimeVar = NULL;
  _endTimeVar = NULL;

  _frequencyVar = NULL;

  _radarAntennaGainHVar = NULL;
  _radarAntennaGainVVar = NULL;
  _radarBeamWidthHVar = NULL;
  _radarBeamWidthVVar = NULL;
  _radarRxBandwidthVar = NULL;

  _lidarConstantVar = NULL;
  _lidarPulseEnergyJVar = NULL;
  _lidarPeakPowerWVar = NULL;
  _lidarApertureDiamCmVar = NULL;
  _lidarApertureEfficiencyVar = NULL;
  _lidarFieldOfViewMradVar = NULL;
  _lidarBeamDivergenceMradVar = NULL;

  _timeVar = NULL;
  _rangeVar = NULL;
  _rayNGatesVar = NULL;
  _rayStartIndexVar = NULL;

  _projVar = NULL;
  _latitudeVar = NULL;
  _longitudeVar = NULL;
  _altitudeVar = NULL;

  _sweepNumberVar = NULL;
  _sweepModeVar = NULL;
  _polModeVar = NULL;
  _prtModeVar = NULL;
  _sweepFollowModeVar = NULL;
  _sweepFixedAngleVar = NULL;
  _fixedAnglesFound = false;
  _targetScanRateVar = NULL;
  _sweepStartRayIndexVar = NULL;
  _sweepEndRayIndexVar = NULL;
  _raysAreIndexedVar = NULL;
  _rayAngleResVar = NULL;
  _intermedFreqHzVar = NULL;

  _rCalTimeVar = NULL;
  _rCalPulseWidthVar = NULL;
  _rCalXmitPowerHVar = NULL;
  _rCalXmitPowerVVar = NULL;
  _rCalTwoWayWaveguideLossHVar = NULL;
  _rCalTwoWayWaveguideLossVVar = NULL;
  _rCalTwoWayRadomeLossHVar = NULL;
  _rCalTwoWayRadomeLossVVar = NULL;
  _rCalReceiverMismatchLossVar = NULL;
  _rCalRadarConstHVar = NULL;
  _rCalRadarConstVVar = NULL;
  _rCalAntennaGainHVar = NULL;
  _rCalAntennaGainVVar = NULL;
  _rCalNoiseHcVar = NULL;
  _rCalNoiseHxVar = NULL;
  _rCalNoiseVcVar = NULL;
  _rCalNoiseVxVar = NULL;
  _rCalReceiverGainHcVar = NULL;
  _rCalReceiverGainHxVar = NULL;
  _rCalReceiverGainVcVar = NULL;
  _rCalReceiverGainVxVar = NULL;
  _rCalBaseDbz1kmHcVar = NULL;
  _rCalBaseDbz1kmHxVar = NULL;
  _rCalBaseDbz1kmVcVar = NULL;
  _rCalBaseDbz1kmVxVar = NULL;
  _rCalSunPowerHcVar = NULL;
  _rCalSunPowerHxVar = NULL;
  _rCalSunPowerVcVar = NULL;
  _rCalSunPowerVxVar = NULL;
  _rCalNoiseSourcePowerHVar = NULL;
  _rCalNoiseSourcePowerVVar = NULL;
  _rCalPowerMeasLossHVar = NULL;
  _rCalPowerMeasLossVVar = NULL;
  _rCalCouplerForwardLossHVar = NULL;
  _rCalCouplerForwardLossVVar = NULL;
  _rCalZdrCorrectionVar = NULL;
  _rCalLdrCorrectionHVar = NULL;
  _rCalLdrCorrectionVVar = NULL;
  _rCalSystemPhidpVar = NULL;
  _rCalTestPowerHVar = NULL;
  _rCalTestPowerVVar = NULL;
  _rCalReceiverSlopeHcVar = NULL;
  _rCalReceiverSlopeHxVar = NULL;
  _rCalReceiverSlopeVcVar = NULL;
  _rCalReceiverSlopeVxVar = NULL;

  _azimuthVar = NULL;
  _elevationVar = NULL;
  _pulseWidthVar = NULL;
  _prtVar = NULL;
  _nyquistVar = NULL;
  _unambigRangeVar = NULL;
  _antennaTransitionVar = NULL;
  _georefsAppliedVar = NULL;
  _nSamplesVar = NULL;
  _calIndexVar = NULL;
  _xmitPowerHVar = NULL;
  _xmitPowerVVar = NULL;
  _scanRateVar = NULL;

  _estNoiseDbmHcVar = NULL;
  _estNoiseDbmVcVar = NULL;
  _estNoiseDbmHxVar = NULL;
  _estNoiseDbmVxVar = NULL;

  _estNoiseAvailHc = false;
  _estNoiseAvailVc = false;
  _estNoiseAvailHx = false;
  _estNoiseAvailVx = false;

  _georefsActive = false;
  _altitudeAglVar = NULL;

  _correctionsActive = false;
  _azimuthCorrVar = NULL;
  _elevationCorrVar = NULL;
  _rangeCorrVar = NULL;
  _longitudeCorrVar = NULL;
  _latitudeCorrVar = NULL;
  _pressureAltCorrVar = NULL;
  _altitudeCorrVar = NULL;
  _ewVelCorrVar = NULL;
  _nsVelCorrVar = NULL;
  _vertVelCorrVar = NULL;
  _headingCorrVar = NULL;
  _rollCorrVar = NULL;
  _pitchCorrVar = NULL;
  _driftCorrVar = NULL;
  _rotationCorrVar = NULL;
  _tiltCorrVar = NULL;

  _conventions.clear();
  _version.clear();
  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();
  _author.clear();
  _origFormat.clear();
  _driver.clear();
  _created.clear();
  _statusXml.clear();
  _siteName.clear();
  _scanName.clear();
  _scanId = 0;
  _instrumentName.clear();

  _rayTimesIncrease = true;
  _refTimeSecsFile = 0;

  _volumeNumber = 0;
  _instrumentType = Radx::missingInstrumentType;
  _platformType = Radx::missingPlatformType;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

  _radarBeamWidthDegH = Radx::missingMetaDouble;
  _radarBeamWidthDegV = Radx::missingMetaDouble;
  _radarAntennaGainDbH = Radx::missingMetaDouble;
  _radarAntennaGainDbV = Radx::missingMetaDouble;

  _lidarConstant = Radx::missingMetaDouble;
  _lidarPulseEnergyJ = Radx::missingMetaDouble;
  _lidarPeakPowerW = Radx::missingMetaDouble;
  _lidarApertureDiamCm = Radx::missingMetaDouble;
  _lidarApertureEfficiency = Radx::missingMetaDouble;
  _lidarFieldOfViewMrad = Radx::missingMetaDouble;
  _lidarBeamDivergenceMrad = Radx::missingMetaDouble;

  _latitude.clear();
  _longitude.clear();
  _altitude.clear();

  _rangeKm.clear();
  _gateSpacingIsConstant = true;

  _clearRays();
  _clearSweeps();
  _clearCals();
  _clearFields();

  _sweepsOrig.clear();
  _sweepsInFile.clear();
  _sweepsToRead.clear();

  _nGatesVary = false;
  _nPoints = 0;

}

void MslFile::_clearRays()
{
  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    delete _raysVol[ii];
  }
  _raysVol.clear();
}

void MslFile::_clearSweeps()
{
  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    delete _sweeps[ii];
  }
  _sweeps.clear();
}

void MslFile::_clearCals()
{
  for (int ii = 0; ii < (int) _rCals.size(); ii++) {
    delete _rCals[ii];
  }
  _rCals.clear();
}

void MslFile::_clearFields()
{
  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool MslFile::isSupported(const string &path)

{
  
  if (isCfRadial(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a CfRadial file
// Returns true on success, false on failure

bool MslFile::isCfRadial(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_verbose) {
      cerr << "DEBUG - not CfRadial file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfRadial file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // file has the correct dimensions, so it is a CfRadial file

  _file.close();
  return true;

}

////////////////////////////////////////////////
// get the date and time from a dorade file path
// returns 0 on success, -1 on failure

int MslFile::getTimeFromPath(const string &path, RadxTime &rtime)

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
    if (sscanf(start, "%4d%2d%2d_%2d%2d%2d",
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

////////////////////////////////////////
// convert Radx::DataType_t to NcType

Nc3Type MslFile::_getNc3Type(Radx::DataType_t dtype)

{
  switch (dtype) {
    case Radx::FL64:
      return nc3Double;
    case Radx::FL32:
      return nc3Float;
    case Radx::SI32:
      return nc3Int;
    case Radx::SI16:
      return nc3Short;
    case Radx::SI08:
    default:
      return nc3Byte;
  }
}

//////////////////////////////////////////////////////////
// convert RadxFile::netcdf_format_t to Nc3File::FileFormat

Nc3File::FileFormat 
  MslFile::_getFileFormat(RadxFile::netcdf_format_t format)

{
  switch (format) {
    case NETCDF4_CLASSIC:
      return Nc3File::Netcdf4Classic;
      break;
    case NETCDF_OFFSET_64BIT:
      return Nc3File::Offset64Bits;
      break;
    case NETCDF4:
      return Nc3File::Netcdf4;
      break;
    case NETCDF_CLASSIC:
    default:
      return Nc3File::Classic;
  }
}

/////////////////////////////////////////////////////////
// print summary after read

void MslFile::print(ostream &out) const
  
{
  
  out << "=============== MslFile ===============" << endl;
  RadxFile::print(out);
  out << "  title: " << _title << endl;
  out << "  institution: " << _institution << endl;
  out << "  references: " << _references << endl;
  out << "  source: " << _source << endl;
  out << "  history: " << _history << endl;
  out << "  comment: " << _comment << endl;
  out << "  author: " << _author << endl;
  out << "  origFormat: " << _origFormat << endl;
  out << "  driver: " << _driver << endl;
  out << "  created: " << _created << endl;
  out << "  statusXml: " << _statusXml << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  scanName: " << _scanName << endl;
  out << "  scanId: " << _scanId << endl;
  out << "  instrumentName: " << _instrumentName << endl;
  out << "  refTimeSecsFile: " << RadxTime::strm(_refTimeSecsFile) << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  if (_latitude.size() > 0) {
    out << "  latitude: " << _latitude[0] << endl;
  }
  if (_longitude.size() > 0) {
    out << "  longitude: " << _longitude[0] << endl;
  }
  if (_altitude.size() > 0) {
    out << "  altitude: " << _altitude[0] << endl;
  }
  for (size_t ii = 0; ii < _frequency.size(); ii++) {
    out << "  frequencyHz[" << ii << "]: " << _frequency[ii] << endl;
  }

  if (_instrumentType == Radx::INSTRUMENT_TYPE_RADAR) {
    out << "  radarBeamWidthDegH: " << _radarBeamWidthDegH << endl;
    out << "  radarBeamWidthDegV: " << _radarBeamWidthDegV << endl;
    out << "  radarAntennaGainDbH: " << _radarAntennaGainDbH << endl;
    out << "  radarAntennaGainDbV: " << _radarAntennaGainDbV << endl;
  } else {
    out << "  lidarConstant: " << _lidarConstant << endl;
    out << "  lidarPulseEnergyJ: " << _lidarPulseEnergyJ << endl;
    out << "  lidarPeakPowerW: " << _lidarPeakPowerW << endl;
    out << "  lidarApertureDiamCm: " << _lidarApertureDiamCm << endl;
    out << "  lidarApertureEfficiency: " << _lidarApertureEfficiency << endl;
    out << "  lidarFieldOfViewMrad: " << _lidarFieldOfViewMrad << endl;
    out << "  lidarBeamDivergenceMrad: " << _lidarBeamDivergenceMrad << endl;
  }
  _geom.print(out);
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int MslFile::printNative(const string &path, ostream &out,
                         bool printRays, bool printData)
  
{

  _addErrStr("ERROR - MslFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

//////////////////////////////////////////////////////////////////
// interpret float and double vals, with respect to missing vals

Radx::fl64 MslFile::_checkMissingDouble(double val)

{
  if (fabs(val - Radx::missingMetaDouble) < 0.0001) {
    return Radx::missingMetaDouble;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaDouble;
  }
  return val;
}

Radx::fl32 MslFile::_checkMissingFloat(float val)

{

  if (fabs(val - Radx::missingMetaFloat) < 0.0001) {
    return Radx::missingMetaFloat;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaFloat;
  }
  return val;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int MslFile::readFromPath(const string &path,
                          RadxVol &vol)
  
{

  _initForRead(path, vol);

  // If the flag is set to aggregate sweeps into a volume on read,
  // create a vector of paths.  Otherwise load just original path into
  // vector.

  vector<string> paths;
  paths.push_back(path);
  
  // load sweep information from files

  if (_loadSweepInfo(paths)) {
    _addErrStr("ERROR - MslFile::readFromPath");
    _addErrStr("  Loading sweep info");
    return -1;
  }

  // read from all paths

  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_readPath(paths[ii], ii)) {
      return -1;
    }
  }

  // load the data into the read volume

  _loadReadVolume();

  // remove transitions if applicable

  if (_readIgnoreTransitions) {
    _readVol->removeTransitionRays(_readTransitionNraysMargin);
  }

  // compute fixed angles if not found

  if (!_fixedAnglesFound) {
    _computeFixedAngles();
  }

  // set format as read

  _fileFormat = FILE_FORMAT_CFRADIAL;

  return 0;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
// Returns 0 on success, -1 on failure

int MslFile::_readPath(const string &path, size_t pathNum)
  
{

  if (_debug) {
    cerr << "Reading file num, path: "
         << pathNum << ", " << path << endl;
  }

  string errStr("ERROR - MslFile::readFromPath::_readPath");

  // clear tmp rays

  _nTimesInFile = 0;
  _raysFromFile.clear();
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
    if (_readTimes(pathNum)) {
      _addErrStr(errStr);
      return -1;
    }
    return 0;
  }
  
  // check if georeferences and/or corrections are active

  _checkGeorefsActiveOnRead();
  _checkCorrectionsActiveOnRead();

  // for first path in aggregated list, read in non-varying values

  if (pathNum == 0) {

    // read global attributes
    
    if (_readGlobalAttributes()) {
      _addErrStr(errStr);
      return -1;
    }

    // read in scalar variables
    
    _readScalarVariables();
    
    // read frequency variable
    
    if (_readFrequencyVariable()) {
      _addErrStr(errStr);
      return -1;
    }

    // read in correction variables
    
    if (_correctionsActive) {
      _readCorrectionVariables();
    }

  }

  // read time variable
  
  if (_readTimes(pathNum)) {
    _addErrStr(errStr);
    return -1;
  }

  // read range variable
  // the range array size will be the max of the arrays found in
  // the files
  
  if (_readRangeVariable()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read position variables - lat/lon/alt
  
  if (_readPositionVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // read in sweep variables

  if (_readSweepVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // read in ray variables

  if (_readRayVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // read in georef variables

  if (_georefsActive) {
    if (_readGeorefVariables()) {
      _addErrStr(errStr);
      return -1;
    }
  }

  // set the ray pointing angles

  _setPointingAngles();

  if (_readMetadataOnly) {

    // read field variables
    
    if (_readFieldVariables(true)) {
      _addErrStr(errStr);
      return -1;
    }
    
    if (_nGatesVary) {
      // set the packing so that the number of gates varies
      // this will ensure that a call to getNGatesVary() on
      // the volume will return true
      _readVol->addToPacking(1);
      _readVol->addToPacking(2);
    }

  } else {

    // create the rays to be read in, filling out the metadata
    
    if (_createRays(path)) {
      _addErrStr(errStr);
      return -1;
    }
    
    // read the ray ngates and offset vectors for each ray
    
    if (_readRayNgatesAndOffsets()) {
      _addErrStr(errStr);
      return -1;
    }
    
    // add field variables to file rays
    
    if (_readFieldVariables(false)) {
      _addErrStr(errStr);
      return -1;
    }

  }

  // read in calibration variables
  
  if (_readCalibrationVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // close file

  _file.close();

  // add file rays to main rays

  for (size_t ii = 0; ii < _raysFromFile.size(); ii++) {

    RadxRay *ray = _raysFromFile[ii];

    // check if we should keep this ray or discard it

    bool keep = true;
    if (_readRemoveRaysAllMissing) {
      if (ray->checkDataAllMissing()) {
        keep = false;
      }
    }

    // add to main vector if we are keeping it

    if (keep) {
      _raysVol.push_back(ray);
    } else {
      delete ray;
    }

  }

  // append to read paths

  _readPaths.push_back(path);

  // clean up

  _raysToRead.clear();
  _raysFromFile.clear();
  _clearGeorefVariables();
  _clearRayVariables();
  _dTimes.clear();

  return 0;

}

//////////////////////////////////////////////////////////
// get list of paths for the volume for the specified path
// returns the volume number

int MslFile::_getVolumePaths(const string &path,
                             vector<string> &paths)
  
{

  paths.clear();
  int volNum = -1;

  RadxPath rpath(path);
  string fileName = rpath.getFile();

  // find the volume number by searching for "_v"
  
  size_t vloc = fileName.find("_v");
  if (vloc == string::npos || vloc == 0) {
    // cannot find volume tag "_v"
    paths.push_back(path);
    return volNum;
  }

  // find trailing "_"

  size_t eloc = fileName.find("_", vloc + 2);
  if (eloc == string::npos || eloc == 0) {
    // cannot find trailing _
    paths.push_back(path);
    return volNum;
  }

  // is this a sweep file - i.e. not already a volume?
  
  size_t sloc = fileName.find("_s", vloc + 2);
  if (sloc == string::npos || sloc == 0) {
    // cannot find sweep tag "_s"
    paths.push_back(path);
    return volNum;
  }

  // set the vol str and numstr

  string volStr(fileName.substr(vloc, eloc - vloc + 1));
  string numStr(fileName.substr(vloc + 2, eloc - vloc - 2));

  // scan the volume number

  if (sscanf(numStr.c_str(), "%d", &volNum) != 1) {
    volNum = -1;
    return volNum;
  }

  // find all paths with this volume number in the same
  // directory as the specified path

  string dir = rpath.getDirectory();
  _addToPathList(dir, volStr, 0, 23, paths);
  
  RadxPath dpath(dir);
  string parentDir = dpath.getDirectory();

  // if time is close to start of day, search previous directory

  RadxTime rtime;
  if (getTimeFromPath(path, rtime)) {
    return volNum;
  }
  int rhour = rtime.getHour();

  if (rhour == 0) {
    RadxTime prevDate(rtime.utime() - RadxTime::RADX_SECS_IN_DAY);
    char prevDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(prevDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            prevDate.getYear(), prevDate.getMonth(), prevDate.getDay());
    _addToPathList(prevDir, volStr, 23, 23, paths);
  }

  // if time is close to end of day, search previous direectory

  if (rhour == 23) {
    RadxTime nextDate(rtime.utime() + RadxTime::RADX_SECS_IN_DAY);
    char nextDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(nextDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            nextDate.getYear(), nextDate.getMonth(), nextDate.getDay());
    _addToPathList(nextDir, volStr, 0, 0, paths);
  }

  // sort the path list

  sort(paths.begin(), paths.end());

  return volNum;

}

///////////////////////////////////////////////////////////
// add to the path list, given time constraints

void MslFile::_addToPathList(const string &dir,
                             const string &volStr,
                             int minHour, int maxHour,
                             vector<string> &paths)
  
{

  // find all paths with this volume number
  
  DIR *dirp;
  if((dirp = opendir(dir.c_str())) == NULL) {
    return;
  }
  
  struct dirent *dp;
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    string fileName(dp->d_name);

    // exclude dir entries which cannot be valid
    if (fileName.find("cfrad.") != 0) {
      continue;
    }
    if (fileName.find("IDL") != string::npos) {
      continue;
    }
    if (fileName.size() < 20) {
      continue;
    }

    if (fileName.find(volStr) == string::npos) {
      continue;
    }

    RadxTime rtime;
    if (getTimeFromPath(fileName, rtime)) {
      continue;
    }
    int hour = rtime.getHour();
    if (hour >= minHour && hour <= maxHour) {
      string filePath = dir;
      filePath += RadxPath::RADX_PATH_DELIM;
      filePath += fileName;
      paths.push_back(filePath);
    }

  } // dp

  closedir(dirp);

}

////////////////////////////////////////////////////////////
// Load up sweep information from files
// Returns 0 on success, -1 on failure

int MslFile::_loadSweepInfo(const vector<string> &paths)
{

  // read in the sweep info

  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_appendSweepInfo(paths[ii])) {
      return -1;
    }
  }

  if (_verbose) {
    cerr << "====>> Sweeps as originally in files <<=======" << endl;
    for (size_t ii = 0; ii < _sweepsOrig.size(); ii++) {
      cerr << "sweep info path: " << _sweepsOrig[ii].path << endl;
      cerr << "  num: " << _sweepsOrig[ii].sweepNum << endl;
      cerr << "  angle: " << _sweepsOrig[ii].fixedAngle << endl;
      cerr << "  indexInFile: " << _sweepsOrig[ii].indexInFile << endl;
    }
    cerr << "==============================================" << endl;
  }
  
  // if no limits set, all sweeps are read

  if (!_readFixedAngleLimitsSet && !_readSweepNumLimitsSet) {
    _sweepsToRead = _sweepsOrig;
    return 0;
  }
  
  // find sweeps which lie within the fixedAngle limits

  _sweepsToRead.clear();
  for (size_t ii = 0; ii < _sweepsOrig.size(); ii++) {
    if (_readFixedAngleLimitsSet) {
      double angle = _sweepsOrig[ii].fixedAngle;
      if (angle > (_readMinFixedAngle - 0.01) && 
          angle < (_readMaxFixedAngle + 0.01)) {
        _sweepsToRead.push_back(_sweepsOrig[ii]);
      }
    } else if (_readSweepNumLimitsSet) {
      int sweepNum = _sweepsOrig[ii].sweepNum;
      if (sweepNum >= _readMinSweepNum &&
          sweepNum <= _readMaxSweepNum) {
        _sweepsToRead.push_back(_sweepsOrig[ii]);
      }
    }
  }

  // make sure we have at least one sweep number
  
  if (_sweepsToRead.size() == 0) {

    // strict checking?

    if (_readStrictAngleLimits) {
      _addErrStr("ERROR - MslFile::_loadSweepInfo");
      _addErrStr("  No sweeps found within limits:");
      if (_readFixedAngleLimitsSet) {
        _addErrDbl("    min fixed angle: ", _readMinFixedAngle, "%g");
        _addErrDbl("    max fixed angle: ", _readMaxFixedAngle, "%g");
      } else if (_readSweepNumLimitsSet) {
        _addErrInt("    min sweep num: ", _readMinSweepNum);
        _addErrInt("    max sweep num: ", _readMaxSweepNum);
      }
      return -1;
    }

    int bestIndex = 0;
    if (_readFixedAngleLimitsSet) {
      double minDiff = 1.0e99;
      double meanAngle = (_readMinFixedAngle + _readMaxFixedAngle) / 2.0;
      if (_readMaxFixedAngle - _readMinFixedAngle < 0) {
        meanAngle -= 180.0;
      }
      if (meanAngle < 0) {
        meanAngle += 360.0;
      }
      for (size_t ii = 0; ii < _sweepsOrig.size(); ii++) {
        double angle = _sweepsOrig[ii].fixedAngle;
        double diff = fabs(angle - meanAngle);
        if (diff < minDiff) {
          minDiff = diff;
          bestIndex = ii;
        }
      }
    } else if (_readSweepNumLimitsSet) {
      double minDiff = 1.0e99;
      double meanNum = (_readMinSweepNum + _readMaxSweepNum) / 2.0;
      for (size_t ii = 0; ii < _sweepsOrig.size(); ii++) {
        int sweepNum = _sweepsOrig[ii].sweepNum;
        double diff = fabs(sweepNum - meanNum);
        if (diff < minDiff) {
          minDiff = diff;
          bestIndex = ii;
        }
      }
    }
    _sweepsToRead.push_back(_sweepsOrig[bestIndex]);
  }

  if (_verbose) {
    cerr << "====>> Sweeps to be read <<=======" << endl;
    for (size_t ii = 0; ii < _sweepsToRead.size(); ii++) {
      cerr << "sweep info path: " << _sweepsToRead[ii].path << endl;
      cerr << "  num: " << _sweepsToRead[ii].sweepNum << endl;
      cerr << "  angle: " << _sweepsToRead[ii].fixedAngle << endl;
      cerr << "  indexInFile: " << _sweepsToRead[ii].indexInFile << endl;
    }
    cerr << "=================================" << endl;
  }

  return 0;

}

int MslFile::_appendSweepInfo(const string &path)
{

  // open file

  if (_file.openRead(path)) {
    _addErrStr("ERROR - MslFile::_appendSweepInfo");
    _addErrStr(_file.getErrStr());
    return -1;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _addErrStr("ERROR - MslFile::_appendSweepInfo");
    return -1;
  }

  if (_readSweepVariables()) {
    _addErrStr("ERROR - MslFile::_appendSweepInfo");
    return -1;
  }
  
  // done with file

  _file.close();

  // add sweeps to list of file sweeps

  for (size_t ii = 0; ii < _sweepsInFile.size(); ii++) {
    RadxSweep *sweep = _sweepsInFile[ii];
    _readVol->addSweepAsInFile(sweep);
    SweepInfo info;
    info.path = path;
    info.sweepNum = sweep->getSweepNumber();
    info.fixedAngle = sweep->getFixedAngleDeg();
    info.indexInFile = ii;
    _sweepsOrig.push_back(info);
  }

  return 0;

}

/////////////////////////////////////////////////
// check if corrections are active on read

void MslFile::_checkGeorefsActiveOnRead()
{

  _georefsActive = false;

  // get latitude variable

  _latitudeVar = _file.getNc3File()->get_var(LATITUDE);
  if (_latitudeVar == NULL) {
    return;
  }

  // if the latitude has dimension of time, then latitude is a 
  // vector and georefs are active
  
  Nc3Dim *timeDim = _latitudeVar->get_dim(0);
  if (timeDim == _timeDim) {
    _georefsActive = true;
  }

}
  
/////////////////////////////////////////////////
// check if corrections are active on read

void MslFile::_checkCorrectionsActiveOnRead()
{

  _correctionsActive = false;
  if (_file.getNc3File()->get_var(AZIMUTH_CORRECTION) != NULL) {
    _correctionsActive = true;
  }

}
  
///////////////////////////////////
// read in the dimensions

int MslFile::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  iret |= _file.readDim(TIME, _timeDim);
  if (iret == 0) {
    _nTimesInFile = _timeDim->size();
  }

  _nRangeInFile = 0;
  iret |= _file.readDim(RANGE, _rangeDim);
  if (iret == 0) {
    _nRangeInFile = _rangeDim->size();
  }

  _nPointsDim = _file.getNc3File()->get_dim(N_POINTS);
  if (_nPointsDim == NULL) {
    _nGatesVary = false;
    _nPoints = 0;
  } else {
    _nGatesVary = true;
    _nPoints = _nPointsDim->size();
  }

  iret |= _file.readDim(SWEEP, _sweepDim);

  if (iret) {
    _addErrStr("ERROR - MslFile::_file.readDimensions");
    return -1;
  }

  // calibration dimension is optional

  _calDim = _file.getNc3File()->get_dim(R_CALIB);

  return 0;

}

///////////////////////////////////
// read the global attributes

int MslFile::_readGlobalAttributes()

{

  Nc3Att *att;
  
  // check for conventions

  att = _file.getNc3File()->get_att(CONVENTIONS);
  if (att == NULL) {
    _addErrStr("ERROR - MslFile::_readGlobalAttributes");
    _addErrStr("  Cannot find conventions attribute");
    return -1;
  }
  _conventions = Nc3xFile::asString(att);
  if (_conventions.find(BaseConvention) == string::npos) {
    if (_conventions.find("CF") == string::npos &&
        _conventions.find("Radial") == string::npos) {
      _addErrStr("ERROR - MslFile::_readGlobalAttributes");
      _addErrStr("  Invalid Conventions attribute: ", _conventions);
      return -1;
    }
  }

  // check for instrument name

  att = _file.getNc3File()->get_att(INSTRUMENT_NAME);
  if (att == NULL) {
    _addErrStr("ERROR - MslFile::_readGlobalAttributes");
    _addErrStr("  Cannot find instrument_name attribute");
    return -1;
  }
  _instrumentName = Nc3xFile::asString(att);
  if (_instrumentName.size() < 1) {
    _instrumentName = "unknown";
  }

  // Loop through the global attributes, use the ones which make sense

  _origFormat = "CFRADIAL"; // default

  for (int ii = 0; ii < _file.getNc3File()->num_atts(); ii++) {
    
    Nc3Att* att = _file.getNc3File()->get_att(ii);
    
    if (att == NULL) {
      continue;
    }
    if (!strcmp(att->name(), VERSION)) {
      _version = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), TITLE)) {
      _title = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), SOURCE)) {
      _source = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), HISTORY)) {
      _history = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), INSTITUTION)) {
      _institution = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), REFERENCES)) {
      _references = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), COMMENT)) {
      _comment = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), AUTHOR)) {
      _author = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), ORIGINAL_FORMAT)) {
      _origFormat = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), DRIVER)) {
      _driver = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), CREATED)) {
      _created = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), SITE_NAME)) {
      _siteName = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), SCAN_NAME)) {
      _scanName = Nc3xFile::asString(att);
    }
    if (!strcmp(att->name(), SCAN_ID)) {
      _scanId = att->as_int(0);
    }
    
    if (!strcmp(att->name(), RAY_TIMES_INCREASE)) {
      string rayTimesIncrease = Nc3xFile::asString(att);
      if (rayTimesIncrease == "true") {
        _rayTimesIncrease = true;
      } else {
        _rayTimesIncrease = false;
      }
    }

    // Caller must delete attribute

    delete att;
    
  } // ii

  return 0;

}

///////////////////////////////////
// read the times

int MslFile::_readTimes(int pathNum)

{

  // read the time variable

  _timeVar = _file.getNc3File()->get_var(TIME);
  if (_timeVar == NULL) {
    _addErrStr("ERROR - MslFile::_readTimes");
    _addErrStr("  Cannot find time variable, name: ", TIME);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  if (_timeVar->num_dims() < 1) {
    _addErrStr("ERROR - MslFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = _timeVar->get_dim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - MslFile::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim->name());
    return -1;
  }

  // get units attribute
  
  Nc3Att* unitsAtt = _timeVar->get_att(UNITS);
  if (unitsAtt == NULL) {
    _addErrStr("ERROR - MslFile::_readTimes");
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
  double *dtimes = dtimes_.alloc(_nTimesInFile);
  if (_timeVar->get(dtimes, _nTimesInFile) == 0) {
    _addErrStr("ERROR - MslFile::_readTimes");
    _addErrStr("  Cannot read times variable");
    return -1;
  }
  _dTimes.clear();
  bool badTimes = false;

  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    if (dtimes[ii] < 0 || fabs(dtimes[ii]) > 1.0e6) {
      badTimes = true;
    }
  }

  if (badTimes) {
    double deltaTime =
      (((double) _timeCoverageEnd - (double) _timeCoverageStart) / 
       (double) (_nTimesInFile - 1));
    double dtime = 0.0;
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      _dTimes.push_back(dtime);
      dtime += deltaTime;
    }
  } else {
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      _dTimes.push_back(dtimes[ii]);
    }
  }

  double startTime = _dTimes[0];
  double endTime = _dTimes[_dTimes.size()-1];
  time_t startTimeSecs = _refTimeSecsFile + (int) startTime;
  time_t endTimeSecs = _refTimeSecsFile + (int) endTime;
  double startNanoSecs = (startTime - (int) startTime) * 1.0e9;
  double endNanoSecs = (endTime - (int) endTime) * 1.0e9;

  _readVol->setStartTime(startTimeSecs, startNanoSecs);
  _readVol->setEndTime(endTimeSecs, endNanoSecs);

  return 0;

}

///////////////////////////////////
// read the range variable

int MslFile::_readRangeVariable()

{

  _rangeVar = _file.getNc3File()->get_var(RANGE);
  if (_rangeVar == NULL || _rangeVar->num_vals() < 1) {
    _addErrStr("ERROR - MslFile::_readRangeVariable");
    _addErrStr("  Cannot read range");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  if (_rangeVar->num_vals() <= (int) _rangeKm.size()) {
    // use data from previously-read sweep
    return 0;
  }

  _rangeKm.clear();
  _nRangeInFile = _rangeVar->num_vals();
  double *rangeMeters = new double[_nRangeInFile];
  if (_rangeVar->get(rangeMeters, _nRangeInFile)) {
    double *rr = rangeMeters;
    for (size_t ii = 0; ii < _nRangeInFile; ii++, rr++) {
      _rangeKm.push_back(*rr / 1000.0);
    }
  }
  delete[] rangeMeters;
  
  // set the geometry from the range vector
  
  _gateSpacingIsConstant = _remap.checkGateSpacingIsConstant(_rangeKm);
  _geom.setRangeGeom(_remap.getStartRangeKm(), _remap.getGateSpacingKm());
  
  // get attributes and check for geometry

  double startRangeKm = Radx::missingMetaDouble;
  double gateSpacingKm = Radx::missingMetaDouble;

  for (int ii = 0; ii < _rangeVar->num_atts(); ii++) {
    
    Nc3Att* att = _rangeVar->get_att(ii);
    
    if (att == NULL) {
      continue;
    }
    
    if (!strcmp(att->name(), SPACING_IS_CONSTANT)) {
      string spacingIsConstant = Nc3xFile::asString(att);
      if (spacingIsConstant == "true") {
        _gateSpacingIsConstant = true;
      } else {
        _gateSpacingIsConstant = false;
      }
    }

    if (!strcmp(att->name(), METERS_TO_CENTER_OF_FIRST_GATE)) {
      if (att->type() == nc3Float || att->type() == nc3Double) {
        startRangeKm = att->as_double(0) / 1000.0;
      }
    }

    if (!strcmp(att->name(), METERS_BETWEEN_GATES)) {
      if (att->type() == nc3Float || att->type() == nc3Double) {
        gateSpacingKm = att->as_double(0) / 1000.0;
      }
    }
    
    // Caller must delete attribute

    delete att;
    
  } // ii

  if (startRangeKm != Radx::missingMetaDouble &&
      gateSpacingKm != Radx::missingMetaDouble) {
    _geom.setRangeGeom(startRangeKm, gateSpacingKm);
  }

  return 0;

}

///////////////////////////////////
// read the scalar variables

int MslFile::_readScalarVariables()

{
  
  int iret = 0;

  iret |= _file.readIntVar(_volumeNumberVar, VOLUME_NUMBER, 
                           _volumeNumber, Radx::missingMetaInt);

  string pstring;
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  if (_file.readStringVar(_instrumentTypeVar, INSTRUMENT_TYPE, pstring) == 0) {
    _instrumentType = Radx::instrumentTypeFromStr(pstring);
  }

  if (_file.readStringVar(_platformTypeVar, PLATFORM_TYPE, pstring) == 0) {
    _platformType = Radx::platformTypeFromStr(pstring);
  }

  if (_file.readStringVar(_primaryAxisVar, PRIMARY_AXIS, pstring) == 0) {
    _primaryAxis = Radx::primaryAxisFromStr(pstring);
  }

  if (_file.getNc3File()->get_var(STATUS_XML) != NULL) {
    if (_file.readStringVar(_statusXmlVar, STATUS_XML, pstring) == 0) {
      _statusXml = pstring;
    }
  }

  Nc3Var *var;
  if (_file.readStringVar(var, TIME_COVERAGE_START, pstring) == 0) {
    RadxTime stime(pstring);
    _timeCoverageStart = stime.utime();
  }
  if (_file.readStringVar(var, TIME_COVERAGE_END, pstring) == 0) {
    RadxTime stime(pstring);
    _timeCoverageEnd = stime.utime();
  }

  if (_instrumentType == Radx::INSTRUMENT_TYPE_RADAR) {

    _file.readDoubleVar(_radarAntennaGainHVar,
                        RADAR_ANTENNA_GAIN_H, _radarAntennaGainDbH, false);
    _file.readDoubleVar(_radarAntennaGainVVar,
                        RADAR_ANTENNA_GAIN_V, _radarAntennaGainDbV, false);
    _file.readDoubleVar(_radarBeamWidthHVar,
                        RADAR_BEAM_WIDTH_H, _radarBeamWidthDegH, false);
    _file.readDoubleVar(_radarBeamWidthVVar,
                        RADAR_BEAM_WIDTH_V, _radarBeamWidthDegV, false);
    _file.readDoubleVar(_radarRxBandwidthVar,
                        RADAR_RX_BANDWIDTH, _radarRxBandwidthHz, false);
  } else {

    _file.readDoubleVar(_lidarConstantVar, 
                        LIDAR_CONSTANT, _lidarConstant, false);
    _file.readDoubleVar(_lidarPulseEnergyJVar, 
                        LIDAR_PULSE_ENERGY, _lidarPulseEnergyJ, false);
    _file.readDoubleVar(_lidarPeakPowerWVar, 
                        LIDAR_PEAK_POWER, _lidarPeakPowerW, false);
    _file.readDoubleVar(_lidarApertureDiamCmVar, 
                        LIDAR_APERTURE_DIAMETER,
                        _lidarApertureDiamCm, false);
    _file.readDoubleVar(_lidarApertureEfficiencyVar, 
                        LIDAR_APERTURE_EFFICIENCY,
                        _lidarApertureEfficiency, false);
    _file.readDoubleVar(_lidarFieldOfViewMradVar, 
                        LIDAR_FIELD_OF_VIEW,
                        _lidarFieldOfViewMrad, false);
    _file.readDoubleVar(_lidarBeamDivergenceMradVar, 
                        LIDAR_BEAM_DIVERGENCE,
                        _lidarBeamDivergenceMrad, false);
    
  }

  if (iret) {
    _addErrStr("ERROR - MslFile::_readScalarVariables");
    return -1;
  } else {
    return 0;
  }

}

///////////////////////////////////
// read the correction variables

int MslFile::_readCorrectionVariables()

{
  
  _cfactors.clear();
  double val;

  if (_file.readDoubleVar(_azimuthCorrVar, 
                          AZIMUTH_CORRECTION, val, 0) == 0) {
    _cfactors.setAzimuthCorr(val);
  }
  
  if (_file.readDoubleVar(_elevationCorrVar,
                          ELEVATION_CORRECTION, val, 0) == 0) {
    _cfactors.setElevationCorr(val);
  }

  if (_file.readDoubleVar(_rangeCorrVar,
                          RANGE_CORRECTION, val, 0) == 0) {
    _cfactors.setRangeCorr(val);
  }

  if (_file.readDoubleVar(_longitudeCorrVar,
                          LONGITUDE_CORRECTION, val, 0) == 0) {
    _cfactors.setLongitudeCorr(val);
  }

  if (_file.readDoubleVar(_latitudeCorrVar,
                          LATITUDE_CORRECTION, val, 0) == 0) {
    _cfactors.setLatitudeCorr(val);
  }

  if (_file.readDoubleVar(_pressureAltCorrVar,
                          PRESSURE_ALTITUDE_CORRECTION, val, 0) == 0) {
    _cfactors.setPressureAltCorr(val);
  }

  if (_file.readDoubleVar(_altitudeCorrVar,
                          ALTITUDE_CORRECTION, val, 0) == 0) {
    _cfactors.setAltitudeCorr(val);
  }

  if (_file.readDoubleVar(_ewVelCorrVar, 
                          EASTWARD_VELOCITY_CORRECTION, val, 0) == 0) {
    _cfactors.setEwVelCorr(val);
  }

  if (_file.readDoubleVar(_nsVelCorrVar, 
                          NORTHWARD_VELOCITY_CORRECTION, val, 0) == 0) {
    _cfactors.setNsVelCorr(val);
  }

  if (_file.readDoubleVar(_vertVelCorrVar,
                          VERTICAL_VELOCITY_CORRECTION, val, 0) == 0) {
    _cfactors.setVertVelCorr(val);
  }

  if (_file.readDoubleVar(_headingCorrVar, 
                          HEADING_CORRECTION, val, 0) == 0) {
    _cfactors.setHeadingCorr(val);
  }

  if (_file.readDoubleVar(_rollCorrVar, 
                          ROLL_CORRECTION, val, 0) == 0) {
    _cfactors.setRollCorr(val);
  }
  
  if (_file.readDoubleVar(_pitchCorrVar, PITCH_CORRECTION, val, 0) == 0) {
    _cfactors.setPitchCorr(val);
  }

  if (_file.readDoubleVar(_driftCorrVar, DRIFT_CORRECTION, val, 0) == 0) {
    _cfactors.setDriftCorr(val);
  }

  if (_file.readDoubleVar(_rotationCorrVar, ROTATION_CORRECTION, val, 0) == 0) {
    _cfactors.setRotationCorr(val);
  }

  if (_file.readDoubleVar(_tiltCorrVar, TILT_CORRECTION, val, 0) == 0) {
    _cfactors.setTiltCorr(val);
  }

  return 0;
  
}

///////////////////////////////////
// read the position variables

int MslFile::_readPositionVariables()

{

  // time

  _georefTimeVar = _file.getNc3File()->get_var(GEOREF_TIME);
  if (_georefTimeVar != NULL) {
    if (_georefTimeVar->num_vals() < 1) {
      _addErrStr("ERROR - MslFile::_readPositionVariables");
      _addErrStr("  Cannot read georef time");
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }
    if (_latitudeVar->type() != nc3Double) {
      _addErrStr("ERROR - MslFile::_readPositionVariables");
      _addErrStr("  georef time is incorrect type: ", 
                 Nc3xFile::ncTypeToStr(_georefTimeVar->type()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  }

  // find latitude, longitude, altitude

  _latitudeVar = _file.getNc3File()->get_var(LATITUDE);
  if (_latitudeVar != NULL) {
    if (_latitudeVar->num_vals() < 1) {
      _addErrStr("ERROR - MslFile::_readPositionVariables");
      _addErrStr("  Cannot read latitude");
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }
    if (_latitudeVar->type() != nc3Double) {
      _addErrStr("ERROR - MslFile::_readPositionVariables");
      _addErrStr("  latitude is incorrect type: ", 
                 Nc3xFile::ncTypeToStr(_latitudeVar->type()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  } else {
    cerr << "WARNING - MslFile::_readPositionVariables" << endl;
    cerr << "  No latitude variable" << endl;
    cerr << "  Setting latitude to 0" << endl;
  }

  _longitudeVar = _file.getNc3File()->get_var(LONGITUDE);
  if (_longitudeVar != NULL) {
    if (_longitudeVar->num_vals() < 1) {
      _addErrStr("ERROR - MslFile::_readPositionVariables");
      _addErrStr("  Cannot read longitude");
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }
    if (_longitudeVar->type() != nc3Double) {
      _addErrStr("ERROR - MslFile::_readPositionVariables");
      _addErrStr("  longitude is incorrect type: ",
                 Nc3xFile::ncTypeToStr(_longitudeVar->type()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  } else {
    cerr << "WARNING - MslFile::_readPositionVariables" << endl;
    cerr << "  No longitude variable" << endl;
    cerr << "  Setting longitude to 0" << endl;
  }

  _altitudeVar = _file.getNc3File()->get_var(ALTITUDE);
  if (_altitudeVar != NULL) {
    if (_altitudeVar->num_vals() < 1) {
      _addErrStr("ERROR - MslFile::_readPositionVariables");
      _addErrStr("  Cannot read altitude");
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }
    if (_altitudeVar->type() != nc3Double) {
      _addErrStr("ERROR - MslFile::_readPositionVariables");
      _addErrStr("  altitude is incorrect type: ",
                 Nc3xFile::ncTypeToStr(_altitudeVar->type()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  } else {
    cerr << "WARNING - MslFile::_readPositionVariables" << endl;
    cerr << "  No altitude variable" << endl;
    cerr << "  Setting altitude to 0" << endl;
  }

  _altitudeAglVar = _file.getNc3File()->get_var(ALTITUDE_AGL);
  if (_altitudeAglVar != NULL) {
    if (_altitudeAglVar->num_vals() < 1) {
      _addErrStr("WARNING - MslFile::_readPositionVariables");
      _addErrStr("  Bad variable - altitudeAgl");
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    if (_altitudeAglVar->type() != nc3Double) {
      _addErrStr("WARNING - MslFile::_readPositionVariables");
      _addErrStr("  altitudeAgl is incorrect type: ",
                 Nc3xFile::ncTypeToStr(_altitudeAglVar->type()));
      _addErrStr("  expecting type: double");
    }
  }

  // set variables

  if (_latitudeVar != NULL) {
    double *data = new double[_latitudeVar->num_vals()];
    if (_latitudeVar->get(data, _latitudeVar->num_vals())) {
      double *dd = data;
      for (int ii = 0; ii < _latitudeVar->num_vals(); ii++, dd++) {
        _latitude.push_back(*dd);
      }
    }
    delete[] data;
  } else {
    _latitude.push_back(0.0);
  }

  if (_longitudeVar != NULL) {
    double *data = new double[_longitudeVar->num_vals()];
    if (_longitudeVar->get(data, _longitudeVar->num_vals())) {
      double *dd = data;
      for (int ii = 0; ii < _longitudeVar->num_vals(); ii++, dd++) {
        _longitude.push_back(*dd);
      }
    }
    delete[] data;
  } else {
    _longitude.push_back(0.0);
  }

  if (_altitudeVar != NULL) {
    double *data = new double[_altitudeVar->num_vals()];
    if (_altitudeVar->get(data, _altitudeVar->num_vals())) {
      double *dd = data;
      for (int ii = 0; ii < _altitudeVar->num_vals(); ii++, dd++) {
        _altitude.push_back(*dd);
      }
    }
    delete[] data;
  } else {
    _altitude.push_back(0.0);
  }

  if (_altitudeAglVar != NULL) {
    double *data = new double[_altitudeAglVar->num_vals()];
    if (_altitudeAglVar->get(data, _altitudeAglVar->num_vals())) {
      double *dd = data;
      for (int ii = 0; ii < _altitudeAglVar->num_vals(); ii++, dd++) {
        _altitudeAgl.push_back(*dd);
      }
    }
    delete[] data;
  }

  return 0;

}

///////////////////////////////////
// read the sweep meta-data

int MslFile::_readSweepVariables()

{

  // create vector for the sweeps

  size_t nSweepsInFile = _sweepDim->size();

  // initialize
  
  vector<int> sweepNums, startRayIndexes, endRayIndexes;
  vector<double> fixedAngles, targetScanRates, rayAngleRes, intermedFreqHz;
  vector<string> sweepModes, polModes, prtModes, followModes, raysAreIndexed;

  int iret = 0;
  
  _readSweepVar(_sweepNumberVar, SWEEP_NUMBER, sweepNums);
  if (sweepNums.size() < nSweepsInFile) {
    iret = -1;
  }

  _readSweepVar(_sweepStartRayIndexVar, SWEEP_START_RAY_INDEX, startRayIndexes);
  if (startRayIndexes.size() < nSweepsInFile) {
    iret = -1;
  }

  _readSweepVar(_sweepEndRayIndexVar, SWEEP_END_RAY_INDEX, endRayIndexes);
  if (endRayIndexes.size() < nSweepsInFile) {
    iret = -1;
  }

  _sweepFixedAngleVar = NULL;
  _readSweepVar(_sweepFixedAngleVar, FIXED_ANGLE, fixedAngles, false);
  if (!_sweepFixedAngleVar) {
    // try old string
    _readSweepVar(_sweepFixedAngleVar, "sweep_fixed_angle", fixedAngles, false);
  }
  if (!_sweepFixedAngleVar) {
    _fixedAnglesFound = false;
  } else {
    _fixedAnglesFound = true;
  }

  _readSweepVar(_targetScanRateVar, TARGET_SCAN_RATE, targetScanRates, false);

  _readSweepVar(_sweepModeVar, SWEEP_MODE, sweepModes);
  if (sweepModes.size() < nSweepsInFile) {
    iret = -1;
  }

  _readSweepVar(_polModeVar, POLARIZATION_MODE, polModes, false);
  _readSweepVar(_prtModeVar, PRT_MODE, prtModes, false);
  _readSweepVar(_sweepFollowModeVar, FOLLOW_MODE, followModes, false);
  
  _readSweepVar(_raysAreIndexedVar, RAYS_ARE_INDEXED, raysAreIndexed, false);
  _readSweepVar(_rayAngleResVar, RAY_ANGLE_RES, rayAngleRes, false);
  _readSweepVar(_intermedFreqHzVar, INTERMED_FREQ_HZ, intermedFreqHz, false);

  if (iret) {
    _addErrStr("ERROR - MslFile::_readSweepVariables");
    return -1;
  }
  
  _sweepsInFile.clear();
  for (size_t ii = 0; ii < nSweepsInFile; ii++) {

    RadxSweep *sweep = new RadxSweep;
    sweep->setVolumeNumber(_volumeNumber);

    if (sweepNums.size() > ii) {
      sweep->setSweepNumber(sweepNums[ii]);
    }
    if (startRayIndexes.size() > ii) {
      sweep->setStartRayIndex(startRayIndexes[ii]);
    }
    if (endRayIndexes.size() > ii) {
      sweep->setEndRayIndex(endRayIndexes[ii]);
    }
    if (fixedAngles.size() > ii) {
      sweep->setFixedAngleDeg(fixedAngles[ii]);
    }
    if (targetScanRates.size() > ii) {
      sweep->setTargetScanRateDegPerSec(targetScanRates[ii]);
    }
    if (sweepModes.size() > ii) {
      sweep->setSweepMode(Radx::sweepModeFromStr(sweepModes[ii]));
    }
    if (polModes.size() > ii) {
      sweep->setPolarizationMode(Radx::polarizationModeFromStr(polModes[ii]));
    }
    if (prtModes.size() > ii) {
      sweep->setPrtMode(Radx::prtModeFromStr(prtModes[ii]));
    }
    if (followModes.size() > ii) {
      sweep->setFollowMode(Radx::followModeFromStr(followModes[ii]));
    }

    if (raysAreIndexed.size() > ii) {
      if (raysAreIndexed[ii] == "true") {
        sweep->setRaysAreIndexed(true);
      } else {
        sweep->setRaysAreIndexed(false);
      }
    }

    if (rayAngleRes.size() > ii) {
      sweep->setAngleResDeg(rayAngleRes[ii]);
    }

    if (intermedFreqHz.size() > ii) {
      sweep->setIntermedFreqHz(intermedFreqHz[ii]);
    }

    _sweepsInFile.push_back(sweep);
    _sweeps.push_back(sweep);

  } // ii

  return 0;

}

///////////////////////////////////
// clear the georeference vectors

void MslFile::_clearGeorefVariables()

{

  _geoTime.clear();
  _geoLatitude.clear();
  _geoLongitude.clear();
  _geoAltitudeMsl.clear();
  _geoAltitudeAgl.clear();
  _geoEwVelocity.clear();
  _geoNsVelocity.clear();
  _geoVertVelocity.clear();
  _geoHeading.clear();
  _geoRoll.clear();
  _geoPitch.clear();
  _geoDrift.clear();
  _geoRotation.clear();
  _geoTilt.clear();
  _geoEwWind.clear();
  _geoNsWind.clear();
  _geoVertWind.clear();
  _geoHeadingRate.clear();
  _geoPitchRate.clear();
  _geoDriveAngle1.clear();
  _geoDriveAngle2.clear();

}

///////////////////////////////////
// read the georeference meta-data

int MslFile::_readGeorefVariables()

{

  _clearGeorefVariables();
  int iret = 0;

  _readRayVar(_georefTimeVar, GEOREF_TIME, _geoTime);
  if (_geoTime.size() < _raysFromFile.size()) {
    // iret = -1;
  }

  _readRayVar(_latitudeVar, LATITUDE, _geoLatitude);
  if (_geoLatitude.size() < _raysFromFile.size()) {
    iret = -1;
  }

  _readRayVar(_longitudeVar, LONGITUDE, _geoLongitude);
  if (_geoLongitude.size() < _raysFromFile.size()) {
    iret = -1;
  }

  _readRayVar(_altitudeVar, ALTITUDE, _geoAltitudeMsl); // meters
  if (_geoAltitudeMsl.size() < _raysFromFile.size()) {
    iret = -1;
  }

  _readRayVar(_altitudeAglVar, ALTITUDE_AGL, _geoAltitudeAgl, false); // meters

  _readRayVar(EASTWARD_VELOCITY, _geoEwVelocity, false);
  _readRayVar(NORTHWARD_VELOCITY, _geoNsVelocity, false);
  _readRayVar(VERTICAL_VELOCITY, _geoVertVelocity, false);

  _readRayVar(HEADING, _geoHeading, false);
  _readRayVar(ROLL, _geoRoll, false);
  _readRayVar(PITCH, _geoPitch, false);

  _readRayVar(DRIFT, _geoDrift, false);
  _readRayVar(ROTATION, _geoRotation, false);
  _readRayVar(TILT, _geoTilt, false);

  _readRayVar(EASTWARD_WIND, _geoEwWind, false);
  _readRayVar(NORTHWARD_WIND, _geoNsWind, false);
  _readRayVar(VERTICAL_WIND, _geoVertWind, false);

  _readRayVar(HEADING_CHANGE_RATE, _geoHeadingRate, false);
  _readRayVar(PITCH_CHANGE_RATE, _geoPitchRate, false);
  _readRayVar(DRIVE_ANGLE_1, _geoDriveAngle1, false);
  _readRayVar(DRIVE_ANGLE_2, _geoDriveAngle2, false);

  if (iret) {
    _addErrStr("ERROR - MslFile::_readGeorefVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// clear the ray variables

void MslFile::_clearRayVariables()

{

  _rayAzimuths.clear();
  _rayElevations.clear();
  _rayPulseWidths.clear();
  _rayPrts.clear();
  _rayPrtRatios.clear();
  _rayNyquists.clear();
  _rayUnambigRanges.clear();
  _rayAntennaTransitions.clear();
  _rayGeorefsApplied.clear();
  _rayNSamples.clear();
  _rayCalNum.clear();
  _rayXmitPowerH.clear();
  _rayXmitPowerV.clear();
  _rayScanRate.clear();
  _rayEstNoiseDbmHc.clear();
  _rayEstNoiseDbmVc.clear();
  _rayEstNoiseDbmHx.clear();
  _rayEstNoiseDbmVx.clear();

  _telescopeLocked.clear();
  _telescopeDirection.clear();
  _telescopeRollAngleOffset.clear();

}

///////////////////////////////////
// read in ray variables

int MslFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  _readRayVar(_azimuthVar, AZIMUTH, _rayAzimuths);
  if (_rayAzimuths.size() < _raysFromFile.size()) {
    _addErrStr("ERROR - azimuth variable required");
    iret = -1;
  }

  _readRayVar(_pulseWidthVar, PULSE_WIDTH, _rayPulseWidths, false);
  _readRayVar(_prtVar, PRT, _rayPrts, false);
  _readRayVar(_prtRatioVar, PRT_RATIO, _rayPrtRatios, false);
  _readRayVar(_nyquistVar, NYQUIST_VELOCITY, _rayNyquists, false);
  _readRayVar(_unambigRangeVar, UNAMBIGUOUS_RANGE, _rayUnambigRanges, false);
  _readRayVar(_antennaTransitionVar, ANTENNA_TRANSITION, 
              _rayAntennaTransitions, false);
  _readRayVar(_georefsAppliedVar, GEOREFS_APPLIED,
              _rayGeorefsApplied, false);
  _readRayVar(_nSamplesVar, N_SAMPLES, _rayNSamples, false);
  _readRayVar(_calIndexVar, R_CALIB_INDEX, _rayCalNum, false);
  _readRayVar(_xmitPowerHVar, RADAR_MEASURED_TRANSMIT_POWER_H, 
              _rayXmitPowerH, false);
  _readRayVar(_xmitPowerVVar, RADAR_MEASURED_TRANSMIT_POWER_V, 
              _rayXmitPowerV, false);
  _readRayVar(_scanRateVar, SCAN_RATE, _rayScanRate, false);
  _readRayVar(_estNoiseDbmHcVar, RADAR_ESTIMATED_NOISE_DBM_HC,
              _rayEstNoiseDbmHc, false);
  _readRayVar(_estNoiseDbmVcVar, RADAR_ESTIMATED_NOISE_DBM_VC,
              _rayEstNoiseDbmVc, false);
  _readRayVar(_estNoiseDbmHxVar, RADAR_ESTIMATED_NOISE_DBM_HX,
              _rayEstNoiseDbmHx, false);
  _readRayVar(_estNoiseDbmVxVar, RADAR_ESTIMATED_NOISE_DBM_VX,
              _rayEstNoiseDbmVx, false);
  
  // HSRL telescope

  // offset from vertical in degrees

  if (_readRayVar(_telescopeRollAngleOffsetVar,
                  "telescope_roll_angle_offset",
                  _telescopeRollAngleOffset, true)) {
    _addErrStr("ERROR - cannot find var telescope_roll_angle_offset");
    iret = -1;
  }

  // locked - 0 is locked, 1 is free

  if (_readRayVar(_telescopeLockedVar,
                  "TelescopeLocked",
                  _telescopeLocked, true)) {
    _addErrStr("ERROR - cannot find var TelescopeLocked");
    iret = -1;
  }

  // direction - 0 is down, 1 is up

  if (_readRayVar(_telescopeDirectionVar,
                  "TelescopeDirection",
                  _telescopeDirection, true)) {
    _addErrStr("ERROR - cannot find var TelescopeDirection");
    iret = -1;
  }

  if (iret) {
    _addErrStr("ERROR - MslFile::_readRayVariables");
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////
// set pointing angles from other variables

void MslFile::_setPointingAngles()

{

  // set elevation and azimuth
  
  _rayElevations.resize(_nTimesInFile);
  _rayAzimuths.resize(_nTimesInFile);
  _geoRotation.resize(_nTimesInFile);
  
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    if (_telescopeDirection[ii] == _params.telescope_direction_is_up) {
      // _rayElevations[ii] = 94.0;
      // _rayElevations[ii] = 86.0;
      _rayElevations[ii] = 90.0;
      _geoRotation[ii] = 360;
    } else {
      // _rayElevations[ii] = -94.0;
      // _rayElevations[ii] = -86.0;
      _rayElevations[ii] = -90.0;
      _geoRotation[ii] = 180;
    }
    _rayAzimuths[ii] = 0.0;
    
    // if (_geoRoll.size() > ii && _telescopeRollAngleOffset.size() > ii) {
    //   if (isfinite(_geoRoll[ii]) && isfinite(_telescopeRollAngleOffset[ii])) {
    //     _rayElevations[ii] -= _geoRoll[ii];
    //     // _rayElevations[ii] = - _geoRoll[ii] + (90.0 - _telescopeRollAngleOffset[ii]);
    //     cerr << "111111 roll, offset, elev: " << ", "
    //          << _geoRoll[ii] << ", "
    //          << _telescopeRollAngleOffset[ii] << ", "
    //          << _rayElevations[ii] << endl;
    //   }
    // }

    // if (_params.debug >= Params::DEBUG_EXTRA) {
    //   cerr << "---> Telescope details - ii, offset, dirn, locked, roll, el, rot: "
    //        << ii << ", "
    //        << _telescopeRollAngleOffset[ii] << ", "
    //        << _telescopeDirection[ii] << ", "
    //        << _telescopeLocked[ii] << ", "
    //        << _geoRoll[ii] << ", "
    //        << _rayElevations[ii] << ", "
    //        << _geoRotation[ii] << endl;
    // }
  }

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int MslFile::_createRays(const string &path)

{

  // compile a list of the rays to be read in, using the list of
  // sweeps to read

  _raysToRead.clear();

  for (size_t isweep = 0; isweep < _sweepsToRead.size(); isweep++) {

    if (path != _sweepsToRead[isweep].path) {
      // the references sweep is not in this file
      continue;
    }

    RadxSweep *sweep = _sweepsInFile[_sweepsToRead[isweep].indexInFile];

    for (size_t ii = sweep->getStartRayIndex();
         ii <= sweep->getEndRayIndex(); ii++) {

      // add ray to list to be read

      RayInfo info;
      info.indexInFile = ii;
      info.sweep = sweep;
      _raysToRead.push_back(info);

    } // ii

  } // isweep

  // create the rays to be read
  
  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {

    size_t rayIndex = _raysToRead[ii].indexInFile;
    const RadxSweep *sweep = _raysToRead[ii].sweep;

    // new ray

    RadxRay *ray = new RadxRay;
    ray->copyRangeGeom(_geom);
    
    // set time
    
    double rayTimeDouble = _dTimes[rayIndex];
    time_t rayUtimeSecs = _refTimeSecsFile + (time_t) rayTimeDouble;
    double rayIntSecs;
    double rayFracSecs = modf(rayTimeDouble, &rayIntSecs);
    int rayNanoSecs = (int) (rayFracSecs * 1.0e9);

    if (rayIntSecs < 0 || rayNanoSecs < 0) {
      rayUtimeSecs -= 1;
      rayNanoSecs = 1000000000 + rayNanoSecs;
    }
      
    ray->setTime(rayUtimeSecs, rayNanoSecs);

    // sweep info

    ray->setSweepNumber(sweep->getSweepNumber());
    ray->setSweepMode(sweep->getSweepMode());
    ray->setPolarizationMode(sweep->getPolarizationMode());
    ray->setPrtMode(sweep->getPrtMode());
    ray->setFollowMode(sweep->getFollowMode());
    ray->setFixedAngleDeg(sweep->getFixedAngleDeg());
    ray->setTargetScanRateDegPerSec(sweep->getTargetScanRateDegPerSec());
    ray->setIsIndexed(sweep->getRaysAreIndexed());
    ray->setAngleResDeg(sweep->getAngleResDeg());

    if (_rayAzimuths.size() > rayIndex) {
      ray->setAzimuthDeg(_rayAzimuths[rayIndex]);
    }
    if (_rayElevations.size() > rayIndex) {
      ray->setElevationDeg(_rayElevations[rayIndex]);
    }
    if (_rayPulseWidths.size() > rayIndex) {
      ray->setPulseWidthUsec(_rayPulseWidths[rayIndex] * 1.0e6);
    }
    if (_rayPrts.size() > rayIndex) {
      ray->setPrtSec(_rayPrts[rayIndex]);
    }
    if (_rayPrtRatios.size() > rayIndex) {
      ray->setPrtRatio(_rayPrtRatios[rayIndex]);
    }
    if (_rayNyquists.size() > rayIndex) {
      ray->setNyquistMps(_rayNyquists[rayIndex]);
    }
    if (_rayUnambigRanges.size() > rayIndex) {
      if (_rayUnambigRanges[rayIndex] > 0) {
        ray->setUnambigRangeKm(_rayUnambigRanges[rayIndex] / 1000.0);
      }
    }
    if (_rayAntennaTransitions.size() > rayIndex) {
      ray->setAntennaTransition(_rayAntennaTransitions[rayIndex]);
    }
    if (_rayGeorefsApplied.size() > rayIndex) {
      ray->setGeorefApplied(_rayGeorefsApplied[rayIndex]);
    }
    if (_rayNSamples.size() > rayIndex) {
      ray->setNSamples(_rayNSamples[rayIndex]);
    }
    if (_rayCalNum.size() > rayIndex) {
      ray->setCalibIndex(_rayCalNum[rayIndex]);
    }
    if (_rayXmitPowerH.size() > rayIndex) {
      ray->setMeasXmitPowerDbmH(_rayXmitPowerH[rayIndex]);
    }
    if (_rayXmitPowerV.size() > rayIndex) {
      ray->setMeasXmitPowerDbmV(_rayXmitPowerV[rayIndex]);
    }
    if (_rayScanRate.size() > rayIndex) {
      ray->setTrueScanRateDegPerSec(_rayScanRate[rayIndex]);
    }
    if (_rayEstNoiseDbmHc.size() > rayIndex) {
      ray->setEstimatedNoiseDbmHc(_rayEstNoiseDbmHc[rayIndex]);
    }
    if (_rayEstNoiseDbmVc.size() > rayIndex) {
      ray->setEstimatedNoiseDbmVc(_rayEstNoiseDbmVc[rayIndex]);
    }
    if (_rayEstNoiseDbmHx.size() > rayIndex) {
      ray->setEstimatedNoiseDbmHx(_rayEstNoiseDbmHx[rayIndex]);
    }
    if (_rayEstNoiseDbmVx.size() > rayIndex) {
      ray->setEstimatedNoiseDbmVx(_rayEstNoiseDbmVx[rayIndex]);
    }
    ray->copyRangeGeom(_geom);

    if (_georefsActive) {

      RadxGeoref geo;
      
      if (_geoTime.size() > rayIndex) {
        double geoTime = _geoTime[rayIndex];
        int secs = (int) geoTime;
        int nanoSecs = (int) ((geoTime - secs) * 1.0e9 + 0.5);
        time_t tSecs = _readVol->getStartTimeSecs() + secs;
        geo.setTimeSecs(tSecs);
        geo.setNanoSecs(nanoSecs);
      }
      if (_geoLatitude.size() > rayIndex) {
        geo.setLatitude(_geoLatitude[rayIndex]);
      }
      if (_geoLongitude.size() > rayIndex) {
        geo.setLongitude(_geoLongitude[rayIndex]);
      }
      if (_geoAltitudeMsl.size() > rayIndex) {
        geo.setAltitudeKmMsl(_geoAltitudeMsl[rayIndex] / 1000.0);
      }
      if (_geoAltitudeAgl.size() > rayIndex) {
        geo.setAltitudeKmAgl(_geoAltitudeAgl[rayIndex] / 1000.0);
      }
      if (_geoEwVelocity.size() > rayIndex) {
        geo.setEwVelocity(_geoEwVelocity[rayIndex]);
      }
      if (_geoNsVelocity.size() > rayIndex) {
        geo.setNsVelocity(_geoNsVelocity[rayIndex]);
      }
      if (_geoVertVelocity.size() > rayIndex) {
        geo.setVertVelocity(_geoVertVelocity[rayIndex]);
      }
      if (_geoHeading.size() > rayIndex) {
        geo.setHeading(_geoHeading[rayIndex]);
      }
      if (_geoRoll.size() > rayIndex) {
        geo.setRoll(_geoRoll[rayIndex]);
      }
      if (_geoPitch.size() > rayIndex) {
        geo.setPitch(_geoPitch[rayIndex]);
      }
      if (_geoDrift.size() > rayIndex) {
        geo.setDrift(_geoDrift[rayIndex]);
      }
      if (_geoRotation.size() > rayIndex) {
        geo.setRotation(_geoRotation[rayIndex]);
      }
      if (_geoTilt.size() > rayIndex) {
        geo.setTilt(_geoTilt[rayIndex]);
      }
      if (_geoEwWind.size() > rayIndex) {
        geo.setEwWind(_geoEwWind[rayIndex]);
      }
      if (_geoNsWind.size() > rayIndex) {
        geo.setNsWind(_geoNsWind[rayIndex]);
      }
      if (_geoVertWind.size() > rayIndex) {
        geo.setVertWind(_geoVertWind[rayIndex]);
      }
      if (_geoHeadingRate.size() > rayIndex) {
        geo.setHeadingRate(_geoHeadingRate[rayIndex]);
      }
      if (_geoPitchRate.size() > rayIndex) {
        geo.setPitchRate(_geoPitchRate[rayIndex]);
      }
      if (_geoDriveAngle1.size() > rayIndex) {
        geo.setDriveAngle1(_geoDriveAngle1[rayIndex]);
      }
      if (_geoDriveAngle2.size() > rayIndex) {
        geo.setDriveAngle2(_geoDriveAngle2[rayIndex]);
      }
      
      ray->setGeoref(geo);

    } // if (_georefsActive) 
  
    // add to ray vector

    _raysFromFile.push_back(ray);

  } // ii

  return 0;

}

///////////////////////////////////
// read the frequency variable

int MslFile::_readFrequencyVariable()

{

  _frequency.clear();
  _frequencyVar = _file.getNc3File()->get_var(FREQUENCY);
  if (_frequencyVar == NULL) {
    return 0;
  }

  int nFreq = _frequencyVar->num_vals();
  double *freq = new double[nFreq];
  if (_frequencyVar->get(freq, nFreq)) {
    for (int ii = 0; ii < nFreq; ii++) {
      _frequency.push_back(freq[ii]);
    }
  }
  delete[] freq;

  return 0;

}

/////////////////////////////////////
// read the ngates and offsets arrays

int MslFile::_readRayNgatesAndOffsets()

{
  
  _rayNGates.clear();
  _rayStartIndex.clear();
  
  // for constant number of gates, compute start indices
  
  if (!_nGatesVary) {
    _nPoints = 0;
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      _rayNGates.push_back(_nRangeInFile);
      _rayStartIndex.push_back(_nPoints);
      _nPoints += _nRangeInFile;
    }
    return 0;
  }
  
  // non-constant nGates - read in arrays

  int iret = 0;

  if (_readRayVar(_rayNGatesVar, RAY_N_GATES, _rayNGates)) {
    _addErrStr("ERROR - MslFile::_readRayNGatesAndOffsets");
    iret = -1;
  }
  
  if (_readRayVar(_rayStartIndexVar, RAY_START_INDEX, _rayStartIndex)) {
    _addErrStr("ERROR - MslFile::_readRayNGatesAndOffsets");
    iret = -1;
  }

  return iret;

}

///////////////////////////////////
// read the calibration variables

int MslFile::_readCalibrationVariables()

{

  if (_calDim == NULL) {
    // no cal available
    return 0;
  }
  
  int iret = 0;
  for (int ii = 0; ii < _calDim->size(); ii++) {
    RadxRcalib *cal = new RadxRcalib;
    if (_readCal(*cal, ii)) {
      _addErrStr("ERROR - MslFile::_readCalibrationVariables");
      _addErrStr("  calibration required, but error on read");
      iret = -1;
    }
    // check that this is not a duplicate
    bool alreadyAdded = false;
    for (size_t ii = 0; ii < _rCals.size(); ii++) {
      const RadxRcalib *rcal = _rCals[ii];
      if (fabs(rcal->getPulseWidthUsec()
               - cal->getPulseWidthUsec()) < 0.0001) {
        alreadyAdded = true;
      }
    }
    if (!alreadyAdded) {
      _rCals.push_back(cal);
    }
  } // ii

  return iret;

}
  
int MslFile::_readCal(RadxRcalib &cal, int index)

{

  int iret = 0;
  double val;
  time_t ctime;

  iret |= _readCalTime(R_CALIB_TIME, 
                       _rCalTimeVar, index, ctime);
  cal.setCalibTime(ctime);

  iret |= _readCalVar(R_CALIB_PULSE_WIDTH, 
                      _rCalPulseWidthVar, index, val, true);
  cal.setPulseWidthUsec(val * 1.0e6);

  iret |= _readCalVar(R_CALIB_XMIT_POWER_H, 
                      _rCalXmitPowerHVar, index, val);
  cal.setXmitPowerDbmH(val);

  iret |= _readCalVar(R_CALIB_XMIT_POWER_V, 
                      _rCalXmitPowerVVar, index, val);
  cal.setXmitPowerDbmV(val);

  iret |= _readCalVar(R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H,
                      _rCalTwoWayWaveguideLossHVar, index, val);
  cal.setTwoWayWaveguideLossDbH(val);

  iret |= _readCalVar(R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V,
                      _rCalTwoWayWaveguideLossVVar, index, val);
  cal.setTwoWayWaveguideLossDbV(val);

  iret |= _readCalVar(R_CALIB_TWO_WAY_RADOME_LOSS_H,
                      _rCalTwoWayRadomeLossHVar, index, val);
  cal.setTwoWayRadomeLossDbH(val);

  iret |= _readCalVar(R_CALIB_TWO_WAY_RADOME_LOSS_V,
                      _rCalTwoWayRadomeLossVVar, index, val);
  cal.setTwoWayRadomeLossDbV(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_MISMATCH_LOSS,
                      _rCalReceiverMismatchLossVar, index, val);
  cal.setReceiverMismatchLossDb(val);

  iret |= _readCalVar(R_CALIB_RADAR_CONSTANT_H, 
                      _rCalRadarConstHVar, index, val);
  cal.setRadarConstantH(val);

  iret |= _readCalVar(R_CALIB_RADAR_CONSTANT_V, 
                      _rCalRadarConstVVar, index, val);
  cal.setRadarConstantV(val);

  iret |= _readCalVar(R_CALIB_ANTENNA_GAIN_H, 
                      _rCalAntennaGainHVar, index, val);
  cal.setAntennaGainDbH(val);
  
  iret |= _readCalVar(R_CALIB_ANTENNA_GAIN_V, 
                      _rCalAntennaGainVVar, index, val);
  cal.setAntennaGainDbV(val);

  iret |= _readCalVar(R_CALIB_NOISE_HC, 
                      _rCalNoiseHcVar, index, val, true);
  cal.setNoiseDbmHc(val);

  iret |= _readCalVar(R_CALIB_NOISE_HX, 
                      _rCalNoiseHxVar, index, val);
  cal.setNoiseDbmHx(val);

  iret |= _readCalVar(R_CALIB_NOISE_VC, 
                      _rCalNoiseVcVar, index, val);
  cal.setNoiseDbmVc(val);

  iret |= _readCalVar(R_CALIB_NOISE_VX, 
                      _rCalNoiseVxVar, index, val);
  cal.setNoiseDbmVx(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_GAIN_HC, 
                      _rCalReceiverGainHcVar, index, val, true);
  cal.setReceiverGainDbHc(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_GAIN_HX, 
                      _rCalReceiverGainHxVar, index, val);
  cal.setReceiverGainDbHx(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_GAIN_VC, 
                      _rCalReceiverGainVcVar, index, val);
  cal.setReceiverGainDbVc(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_GAIN_VX, 
                      _rCalReceiverGainVxVar, index, val);
  cal.setReceiverGainDbVx(val);

  iret |= _readCalVar(R_CALIB_BASE_DBZ_1KM_HC, 
                      _rCalBaseDbz1kmHcVar, index, val);
  cal.setBaseDbz1kmHc(val);

  iret |= _readCalVar(R_CALIB_BASE_DBZ_1KM_HX, 
                      _rCalBaseDbz1kmHxVar, index, val);
  cal.setBaseDbz1kmHx(val);

  iret |= _readCalVar(R_CALIB_BASE_DBZ_1KM_VC, 
                      _rCalBaseDbz1kmVcVar, index, val);
  cal.setBaseDbz1kmVc(val);

  iret |= _readCalVar(R_CALIB_BASE_DBZ_1KM_VX, 
                      _rCalBaseDbz1kmVxVar, index, val);
  cal.setBaseDbz1kmVx(val);

  iret |= _readCalVar(R_CALIB_SUN_POWER_HC, 
                      _rCalSunPowerHcVar, index, val);
  cal.setSunPowerDbmHc(val);

  iret |= _readCalVar(R_CALIB_SUN_POWER_HX, 
                      _rCalSunPowerHxVar, index, val);
  cal.setSunPowerDbmHx(val);

  iret |= _readCalVar(R_CALIB_SUN_POWER_VC, 
                      _rCalSunPowerVcVar, index, val);
  cal.setSunPowerDbmVc(val);

  iret |= _readCalVar(R_CALIB_SUN_POWER_VX, 
                      _rCalSunPowerVxVar, index, val);
  cal.setSunPowerDbmVx(val);

  iret |= _readCalVar(R_CALIB_NOISE_SOURCE_POWER_H, 
                      _rCalNoiseSourcePowerHVar, index, val);
  cal.setNoiseSourcePowerDbmH(val);

  iret |= _readCalVar(R_CALIB_NOISE_SOURCE_POWER_V, 
                      _rCalNoiseSourcePowerVVar, index, val);
  cal.setNoiseSourcePowerDbmV(val);

  iret |= _readCalVar(R_CALIB_POWER_MEASURE_LOSS_H, 
                      _rCalPowerMeasLossHVar, index, val);
  cal.setPowerMeasLossDbH(val);

  iret |= _readCalVar(R_CALIB_POWER_MEASURE_LOSS_V, 
                      _rCalPowerMeasLossVVar, index, val);
  cal.setPowerMeasLossDbV(val);

  iret |= _readCalVar(R_CALIB_COUPLER_FORWARD_LOSS_H, 
                      _rCalCouplerForwardLossHVar, index, val);
  cal.setCouplerForwardLossDbH(val);

  iret |= _readCalVar(R_CALIB_COUPLER_FORWARD_LOSS_V, 
                      _rCalCouplerForwardLossVVar, index, val);
  cal.setCouplerForwardLossDbV(val);

  iret |= _readCalVar(R_CALIB_ZDR_CORRECTION, 
                      _rCalZdrCorrectionVar, index, val);
  cal.setZdrCorrectionDb(val);

  iret |= _readCalVar(R_CALIB_LDR_CORRECTION_H, 
                      _rCalLdrCorrectionHVar, index, val);
  cal.setLdrCorrectionDbH(val);

  iret |= _readCalVar(R_CALIB_LDR_CORRECTION_V, 
                      _rCalLdrCorrectionVVar, index, val);
  cal.setLdrCorrectionDbV(val);

  iret |= _readCalVar(R_CALIB_SYSTEM_PHIDP, 
                      _rCalSystemPhidpVar, index, val);
  cal.setSystemPhidpDeg(val);

  iret |= _readCalVar(R_CALIB_TEST_POWER_H, 
                      _rCalTestPowerHVar, index, val);
  cal.setTestPowerDbmH(val);

  iret |= _readCalVar(R_CALIB_TEST_POWER_V, 
                      _rCalTestPowerVVar, index, val);
  cal.setTestPowerDbmV(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_SLOPE_HC, 
                      _rCalReceiverSlopeHcVar, index, val);
  cal.setReceiverSlopeDbHc(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_SLOPE_HX, 
                      _rCalReceiverSlopeHxVar, index, val);
  cal.setReceiverSlopeDbHx(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_SLOPE_VC, 
                      _rCalReceiverSlopeVcVar, index, val);
  cal.setReceiverSlopeDbVc(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_SLOPE_VX, 
                      _rCalReceiverSlopeVxVar, index, val);
  cal.setReceiverSlopeDbVx(val);

  return iret;

}

////////////////////////////////////////////
// read the field variables

int MslFile::_readFieldVariables(bool metaOnly)

{

  // loop through the variables, adding data fields as appropriate
  
  for (int ivar = 0; ivar < _file.getNc3File()->num_vars(); ivar++) {
    
    Nc3Var* var = _file.getNc3File()->get_var(ivar);
    if (var == NULL) {
      continue;
    }
    
    int numDims = var->num_dims();
    if (_nGatesVary) {
      // variable number of gates per ray
      // we need fields with 1 dimension
      if (numDims != 1) {
        continue;
      }
      Nc3Dim* nPointsDim = var->get_dim(0);
      if (nPointsDim != _nPointsDim) {
        continue;
      }
    } else {
      // constant number of gates per ray
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
    }
    
    // check the type
    Nc3Type ftype = var->type();
    if (ftype != nc3Double && ftype != nc3Float && ftype != nc3Int &&
        ftype != nc3Short && ftype != nc3Byte) {
      // not a valid type
      continue;
    }

    // check that we need this field

    string fieldName = var->name();
    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - MslFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - MslFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << fieldName << endl;
    }

    // set names, units, etc
    
    string name = var->name();

    string standardName;
    Nc3Att *standardNameAtt = var->get_att(STANDARD_NAME);
    if (standardNameAtt != NULL) {
      standardName = Nc3xFile::asString(standardNameAtt);
      delete standardNameAtt;
    }
    
    string longName;
    Nc3Att *longNameAtt = var->get_att(LONG_NAME);
    if (longNameAtt != NULL) {
      longName = Nc3xFile::asString(longNameAtt);
      delete longNameAtt;
    }

    string units;
    Nc3Att *unitsAtt = var->get_att(UNITS);
    if (unitsAtt != NULL) {
      units = Nc3xFile::asString(unitsAtt);
      delete unitsAtt;
    }

    string legendXml;
    Nc3Att *legendXmlAtt = var->get_att(LEGEND_XML);
    if (legendXmlAtt != NULL) {
      legendXml = Nc3xFile::asString(legendXmlAtt);
      delete legendXmlAtt;
    }

    string thresholdingXml;
    Nc3Att *thresholdingXmlAtt = var->get_att(THRESHOLDING_XML);
    if (thresholdingXmlAtt != NULL) {
      thresholdingXml = Nc3xFile::asString(thresholdingXmlAtt);
      delete thresholdingXmlAtt;
    }

    float samplingRatio = Radx::missingMetaFloat;
    Nc3Att *samplingRatioAtt = var->get_att(SAMPLING_RATIO);
    if (samplingRatioAtt != NULL) {
      samplingRatio = samplingRatioAtt->as_float(0);
      delete samplingRatioAtt;
    }

    // folding

    bool fieldFolds = false;
    float foldLimitLower = Radx::missingMetaFloat;
    float foldLimitUpper = Radx::missingMetaFloat;
    Nc3Att *fieldFoldsAtt = var->get_att(FIELD_FOLDS);
    if (fieldFoldsAtt != NULL) {
      string fieldFoldsStr = Nc3xFile::asString(fieldFoldsAtt);
      if (fieldFoldsStr == "true"
          || fieldFoldsStr == "TRUE"
          || fieldFoldsStr == "True") {
        fieldFolds = true;
        Nc3Att *foldLimitLowerAtt = var->get_att(FOLD_LIMIT_LOWER);
        if (foldLimitLowerAtt != NULL) {
          foldLimitLower = foldLimitLowerAtt->as_float(0);
          delete foldLimitLowerAtt;
        }
        Nc3Att *foldLimitUpperAtt = var->get_att(FOLD_LIMIT_UPPER);
        if (foldLimitUpperAtt != NULL) {
          foldLimitUpper = foldLimitUpperAtt->as_float(0);
          delete foldLimitUpperAtt;
        }
      }
      delete fieldFoldsAtt;
    }

    // is this field discrete

    bool isDiscrete = false;
    Nc3Att *isDiscreteAtt = var->get_att(IS_DISCRETE);
    if (isDiscreteAtt != NULL) {
      string isDiscreteStr = Nc3xFile::asString(isDiscreteAtt);
      if (isDiscreteStr == "true"
          || isDiscreteStr == "TRUE"
          || isDiscreteStr == "True") {
        isDiscrete = true;
      }
      delete isDiscreteAtt;
    }

    // get offset and scale

    double offset = 0.0;
    Nc3Att *offsetAtt = var->get_att(ADD_OFFSET);
    if (offsetAtt != NULL) {
      offset = offsetAtt->as_double(0);
      delete offsetAtt;
    }

    double scale = 1.0;
    Nc3Att *scaleAtt = var->get_att(SCALE_FACTOR);
    if (scaleAtt != NULL) {
      scale = scaleAtt->as_double(0);
      delete scaleAtt;
    }

    // if metadata only, don't read in fields

    if (metaOnly) {
      if (!_readVol->fieldExists(name)) {
        RadxField *field = new RadxField(name, units);
        field->setLongName(longName);
        field->setStandardName(standardName);
        field->setSamplingRatio(samplingRatio);
        if (fieldFolds &&
            foldLimitLower != Radx::missingMetaFloat &&
            foldLimitUpper != Radx::missingMetaFloat) {
          field->setFieldFolds(foldLimitLower, foldLimitUpper);
        }
        if (isDiscrete) {
          field->setIsDiscrete(true);
        }
        if (legendXml.size() > 0) {
          field->setLegendXml(legendXml);
        }
        if (thresholdingXml.size() > 0) {
          field->setThresholdingXml(thresholdingXml);
        }
        _readVol->addField(field);
      }
      continue;
    }

    int iret = 0;
    
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
      case nc3Int: {
        if (_addSi32FieldToRays(var, name, units, standardName, longName,
                                scale, offset,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case nc3Short: {
        if (_addSi16FieldToRays(var, name, units, standardName, longName,
                                scale, offset,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case nc3Byte: {
        if (_addSi08FieldToRays(var, name, units, standardName, longName,
                                scale, offset,
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
      _addErrStr("ERROR - MslFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  } // ivar

  return 0;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set var, vals

int MslFile::_readRayVar(Nc3Var* &var, const string &name,
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
      _addErrStr("ERROR - MslFile::_readRayVar");
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
      _addErrStr("ERROR - MslFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set vals only

int MslFile::_readRayVar(const string &name,
                         vector<double> &vals, bool required)
{
  Nc3Var *var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////
// read a ray variable - integer
// side effects: set var, vals

int MslFile::_readRayVar(Nc3Var* &var, const string &name,
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
      _addErrStr("ERROR - MslFile::_readRayVar");
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
      _addErrStr("ERROR - MslFile::_readRayVar");
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
// side effects: set vals only

int MslFile::_readRayVar(const string &name,
                         vector<int> &vals, bool required)
{
  Nc3Var *var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////////////
// read a ray variable - boolean
// side effects: set var, vals

int MslFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<bool> &vals, bool required)
  
{
  
  vals.clear();
  
  // get var
  
  var = _getRayVar(name, false);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(false);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - MslFile::_readRayVar");
      return -1;
    }
  }

  // load up data
  
  int *data = new int[_nTimesInFile];
  int *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      if (*dd == 0) {
        vals.push_back(false);
      } else {
        vals.push_back(true);
      }
    }
  } else {
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      vals.push_back(false);
    }
    clearErrStr();
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - boolean
// side effects: set vals only

int MslFile::_readRayVar(const string &name,
                         vector<bool> &vals, bool required)
{
  Nc3Var *var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

Nc3Var* MslFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - MslFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - MslFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - MslFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has incorrect dimension, dim name: ", 
                 timeDim->name());
      _addErrStr("  should be: ", TIME);
    }
    return NULL;
  }

  return var;

}

///////////////////////////////////
// read a sweep variable - double

int MslFile::_readSweepVar(Nc3Var* &var, const string &name,
                           vector<double> &vals, bool required)

{

  vals.clear();

  // get var

  int nSweeps = _sweepDim->size();
  var = _getSweepVar(name);
  if (var == NULL) {
    if (!required) {
      for (int ii = 0; ii < nSweeps; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - MslFile::_readSweepVar");
      return -1;
    }
  }

  // load up data

  double *data = new double[nSweeps];
  double *dd = data;
  int iret = 0;
  if (var->get(data, nSweeps)) {
    for (int ii = 0; ii < nSweeps; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (int ii = 0; ii < nSweeps; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - MslFile::_readSweepVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a sweep variable - integer

int MslFile::_readSweepVar(Nc3Var* &var, const string &name,
                           vector<int> &vals, bool required)

{

  vals.clear();

  // get var

  int nSweeps = _sweepDim->size();
  var = _getSweepVar(name);
  if (var == NULL) {
    if (!required) {
      for (int ii = 0; ii < nSweeps; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - MslFile::_readSweepVar");
      return -1;
    }
  }

  // load up data

  int *data = new int[nSweeps];
  int *dd = data;
  int iret = 0;
  if (var->get(data, nSweeps)) {
    for (int ii = 0; ii < nSweeps; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (int ii = 0; ii < nSweeps; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - MslFile::_readSweepVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a sweep variable - string

int MslFile::_readSweepVar(Nc3Var* &var, const string &name,
                           vector<string> &vals, bool required)

{

  // get var
  
  int nSweeps = _sweepDim->size();
  var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (!required) {
      for (int ii = 0; ii < nSweeps; ii++) {
        vals.push_back("");
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - MslFile::_readSweepVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }
  }

  // check sweep dimension

  if (var->num_dims() < 2) {
    _addErrStr("ERROR - MslFile::_readSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has fewer than 2 dimensions");
    return -1;
  }
  Nc3Dim *sweepDim = var->get_dim(0);
  if (sweepDim != _sweepDim) {
    _addErrStr("ERROR - MslFile::_readSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has incorrect first dimension, dim name: ",
               sweepDim->name());
    _addErrStr("  should be: ", SWEEP);
    return -1;
  }
  Nc3Dim *stringLenDim = var->get_dim(1);
  if (stringLenDim == NULL) {
    _addErrStr("ERROR - MslFile::_readSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has NULL second dimension");
    _addErrStr("  should be a string length dimension");
    return -1;
  }

  Nc3Type ntype = var->type();
  if (ntype != nc3Char) {
    _addErrStr("ERROR - MslFile::_readSweepVar");
    _addErrStr("  Incorrect variable type");
    _addErrStr("  Expecting char");
    _addErrStr("  Found: ", Nc3xFile::ncTypeToStr(ntype));
    return -1;
  }

  // load up data

  int stringLen = stringLenDim->size();
  int nChars = nSweeps * stringLen;
  char *cvalues = new char[nChars];
  if (var->get(cvalues, nSweeps, stringLen)) {
    // replace white space with nulls
    for (int ii = 0; ii < nChars; ii++) {
      if (isspace(cvalues[ii])) {
        cvalues[ii] = '\0';
      }
    }
    // ensure null termination
    char *cv = cvalues;
    char *cval = new char[stringLen+1];
    for (int ii = 0; ii < nSweeps; ii++, cv += stringLen) {
      memcpy(cval, cv, stringLen);
      cval[stringLen] = '\0';
      vals.push_back(string(cval));
    }
    delete[] cval;
  } else {
    _addErrStr("ERROR - MslFile::_readSweepVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  delete[] cvalues;

  return 0;

}

///////////////////////////////////
// get a sweep variable
// returns NULL on failure

Nc3Var* MslFile::_getSweepVar(const string &name)

{
  
  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    _addErrStr("ERROR - MslFile::_getSweepVar");
    _addErrStr("  Cannot read variable, name: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return NULL;
  }

  // check sweep dimension

  if (var->num_dims() < 1) {
    _addErrStr("ERROR - MslFile::_getSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no dimensions");
    return NULL;
  }
  Nc3Dim *sweepDim = var->get_dim(0);
  if (sweepDim != _sweepDim) {
    _addErrStr("ERROR - MslFile::_getSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has incorrect dimension, dim name: ",
               sweepDim->name());
    _addErrStr("  should be: ", SWEEP);
    return NULL;
  }

  return var;

}

///////////////////////////////////
// get calibration time
// returns -1 on failure

int MslFile::_readCalTime(const string &name, Nc3Var* &var,
                          int index, time_t &val)

{

  var = _file.getNc3File()->get_var(name.c_str());

  if (var == NULL) {
    _addErrStr("ERROR - MslFile::_readCalTime");
    _addErrStr("  cal variable name: ", name);
    _addErrStr("  Cannot read calibration time");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

  // check cal dimension

  if (var->num_dims() < 2) {
    _addErrStr("ERROR - MslFile::_readCalTime");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has fewer than 2 dimensions");
    return -1;
  }

  Nc3Dim *rCalDim = var->get_dim(0);
  if (rCalDim != _calDim) {
    _addErrStr("ERROR - MslFile::_readCalTime");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has incorrect first dimension, dim name: ", 
               rCalDim->name());
    _addErrStr("  should be: ", R_CALIB);
    return -1;
  }

  Nc3Dim *stringLenDim = var->get_dim(1);
  if (stringLenDim == NULL) {
    _addErrStr("ERROR - MslFile::_readCalTime");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has NULL second dimension");
    _addErrStr("  should be a string length dimension");
    return -1;
  }
  
  Nc3Type ntype = var->type();
  if (ntype != nc3Char) {
    _addErrStr("ERROR - MslFile::_readCalTime");
    _addErrStr("  Incorrect variable type");
    _addErrStr("  Expecting char");
    _addErrStr("  Found: ", Nc3xFile::ncTypeToStr(ntype));
    return -1;
  }

  // load up data
  
  int nCals = _calDim->size();
  if (index > nCals - 1) {
    _addErrStr("ERROR - MslFile::_readCalTime");
    _addErrStr("  requested index too high");
    _addErrStr("  cal variable name: ", name);
    _addErrInt("  requested index: ", index);
    _addErrInt("  n cals available: ", nCals);
    return -1;
  }

  int stringLen = stringLenDim->size();
  int nChars = nCals * stringLen;
  char *cvalues = new char[nChars];
  vector<string> times;
  if (var->get(cvalues, nCals, stringLen)) {
    char *cv = cvalues;
    char *cval = new char[stringLen+1];
    for (int ii = 0; ii < nCals; ii++, cv += stringLen) {
      // ensure null termination
      memcpy(cval, cv, stringLen);
      cval[stringLen] = '\0';
      times.push_back(string(cval));
      cv[stringLen-1] = '\0';
    }
    delete[] cval;
  } else {
    _addErrStr("ERROR - MslFile::_readCalTime");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    delete[] cvalues;
    return -1;
  }

  const char *timeStr = times[index].c_str();
  int year, month, day, hour, min, sec;
  if (sscanf(timeStr, "%4d-%2d-%2dT%2d:%2d:%2dZ",
             &year, &month, &day, &hour, &min, &sec) != 6) {
    _addErrStr("ERROR - MslFile::_readCalTime");
    _addErrStr("  Cannot parse cal time string: ", timeStr);
    delete[] cvalues;
    return -1;
  }
  delete[] cvalues;
  RadxTime ctime(year, month, day, hour, min, sec);
  val = ctime.utime();

  return 0;

}

///////////////////////////////////
// get calibration variable
// returns -1 on failure

int MslFile::_readCalVar(const string &name, Nc3Var* &var,
                         int index, double &val, bool required)
  
{

  val = Radx::missingMetaDouble;
  var = _file.getNc3File()->get_var(name.c_str());

  if (var == NULL) {
    if (!required) {
      return 0;
    } else {
      _addErrStr("ERROR - MslFile::_readCalVar");
      _addErrStr("  cal variable name: ", name);
      _addErrStr("  Cannot read calibration variable");
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }
  }

  if (var->num_vals() < index-1) {
    _addErrStr("ERROR - MslFile::_readCalVar");
    _addErrStr("  requested index too high");
    _addErrStr("  cal variable name: ", name);
    _addErrInt("  requested index: ", index);
    _addErrInt("  n cals available: ", var->num_vals());
    return -1;
  }

  val = var->as_double(index);

  return 0;

}

//////////////////////////////////////////////////////////////
// Add fl64 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int MslFile::_addFl64FieldToRays(Nc3Var* var,
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

  Radx::fl64 *data = new Radx::fl64[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get(data, _nPoints);
  } else {
    iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::fl64 missingVal = Radx::missingFl64;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_double(0);
      delete missingValueAtt;
    }
  }

  // reset nans to missing
  
  for (int ii = 0; ii < _nPoints; ii++) {
    if (!std::isfinite(data[ii])) {
      data[ii] = missingVal;
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - MslFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }
    
    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
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

int MslFile::_addFl32FieldToRays(Nc3Var* var,
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

  Radx::fl32 *data = new Radx::fl32[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get(data, _nPoints);
  } else {
    iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::fl32 missingVal = Radx::missingFl32;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_double(0);
      delete missingValueAtt;
    }
  }
  
  // reset nans to missing
  
  for (int ii = 0; ii < _nPoints; ii++) {
    if (!std::isfinite(data[ii])) {
      data[ii] = missingVal;
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - MslFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }

    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
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
// Add si32 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int MslFile::_addSi32FieldToRays(Nc3Var* var,
                                 const string &name,
                                 const string &units,
                                 const string &standardName,
                                 const string &longName,
                                 double scale, double offset,
                                 bool isDiscrete,
                                 bool fieldFolds,
                                 float foldLimitLower,
                                 float foldLimitUpper)
  
{

  // get data from array

  Radx::si32 *data = new Radx::si32[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get(data, _nPoints);
  } else {
    iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::si32 missingVal = Radx::missingSi32;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_int(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_int(0);
      delete missingValueAtt;
    }
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - MslFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }
    
    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
                                  missingVal,
                                  data + startIndex,
                                  scale, offset,
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
// Add si16 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int MslFile::_addSi16FieldToRays(Nc3Var* var,
                                 const string &name,
                                 const string &units,
                                 const string &standardName,
                                 const string &longName,
                                 double scale, double offset,
                                 bool isDiscrete,
                                 bool fieldFolds,
                                 float foldLimitLower,
                                 float foldLimitUpper)
  
{

  // get data from array

  Radx::si16 *data = new Radx::si16[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get(data, _nPoints);
  } else {
    iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::si16 missingVal = Radx::missingSi16;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_short(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_short(0);
      delete missingValueAtt;
    }
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - MslFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }
    
    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
                                  missingVal,
                                  data + startIndex,
                                  scale, offset,
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
// Add si08 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int MslFile::_addSi08FieldToRays(Nc3Var* var,
                                 const string &name,
                                 const string &units,
                                 const string &standardName,
                                 const string &longName,
                                 double scale, double offset,
                                 bool isDiscrete,
                                 bool fieldFolds,
                                 float foldLimitLower,
                                 float foldLimitUpper)
  
{

  // get data from array

  Radx::si08 *data = new Radx::si08[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get((ncbyte *) data, _nPoints);
  } else {
    iret = !var->get((ncbyte *) data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::si08 missingVal = Radx::missingSi08;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_ncbyte(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_ncbyte(0);
      delete missingValueAtt;
    }
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - MslFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }
    
    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
                                  missingVal,
                                  data + startIndex,
                                  scale, offset,
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

void MslFile::_loadReadVolume()
  
{

  _readVol->setOrigFormat("CFRADIAL");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);
  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    _raysVol[ii]->setVolumeNumber(_volumeNumber);
  }

  for (size_t ii = 0; ii < _frequency.size(); ii++) {
    _readVol->addFrequencyHz(_frequency[ii]);
  }

  _readVol->setRadarAntennaGainDbH(_radarAntennaGainDbH);
  _readVol->setRadarAntennaGainDbV(_radarAntennaGainDbV);
  _readVol->setRadarBeamWidthDegH(_radarBeamWidthDegH);
  _readVol->setRadarBeamWidthDegV(_radarBeamWidthDegV);
  _readVol->setRadarReceiverBandwidthMhz(_radarRxBandwidthHz / 1.0e6);

  _readVol->setVersion(_version);
  _readVol->setTitle(_title);
  _readVol->setSource(_source);
  _readVol->setHistory(_history);
  _readVol->setInstitution(_institution);
  _readVol->setReferences(_references);
  _readVol->setComment(_comment);
  _readVol->setOrigFormat(_origFormat);
  _readVol->setDriver(_driver);
  _readVol->setCreated(_created);
  _readVol->setStatusXml(_statusXml);
  _readVol->setSiteName(_siteName);
  _readVol->setScanName(_scanName);
  _readVol->setScanId(_scanId);
  _readVol->setInstrumentName(_instrumentName);

  if (_latitude.size() > 0) {
    for (size_t ii = 0; ii < _latitude.size(); ii++) {
      if (_latitude[ii] > -9990) {
        _readVol->setLatitudeDeg(_latitude[ii]);
        break;
      }
    }
  }
  if (_longitude.size() > 0) {
    for (size_t ii = 0; ii < _longitude.size(); ii++) {
      if (_longitude[ii] > -9990) {
        _readVol->setLongitudeDeg(_longitude[ii]);
        break;
      }
    }
  }
  if (_altitude.size() > 0) {
    for (size_t ii = 0; ii < _altitude.size(); ii++) {
      if (_altitude[ii] > -9990) {
        _readVol->setAltitudeKm(_altitude[ii] / 1000.0);
        break;
      }
    }
  }
  if (_altitudeAgl.size() > 0) {
    for (size_t ii = 0; ii < _altitudeAgl.size(); ii++) {
      if (_altitudeAgl[ii] > -9990) {
        _readVol->setSensorHtAglM(_altitudeAgl[ii]);
        break;
      }
    }
  }

  _readVol->copyRangeGeom(_geom);

  if (_correctionsActive) {
    _readVol->setCfactors(_cfactors);
  }

  for (size_t ii = 0; ii < _raysVol.size(); ii++) {
    _readVol->addRay(_raysVol[ii]);
  }

  for (size_t ii = 0; ii < _rCals.size(); ii++) {
    _readVol->addCalib(_rCals[ii]);
  }

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _raysVol.clear();
  _rCals.clear();
  _fields.clear();

  // apply goeref info if applicable

  if (_readApplyGeorefs) {
    _readVol->applyGeorefs();
  }

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();
  
  // check for indexed rays, set info on rays

  _readVol->checkForIndexedRays();

}

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void MslFile::_computeFixedAngles()
  
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

/////////////////////////////////////////////////////////////////////////////////////
// string constant instantiation

const string MslFile::CfConvention = "CF-1.6";
const string MslFile::BaseConvention = "CF-Radial";
const string MslFile::CurrentVersion = "CF-Radial-1.3";

const char* MslFile::ADD_OFFSET = "add_offset";
const char* MslFile::AIRBORNE = "airborne";
const char* MslFile::ALTITUDE = "altitude";
const char* MslFile::ALTITUDE_AGL = "altitude_agl";
const char* MslFile::ALTITUDE_CORRECTION = "altitude_correction";
const char* MslFile::ALTITUDE_OF_PROJECTION_ORIGIN = "altitude_of_projection_origin";
const char* MslFile::ANTENNA_TRANSITION = "antenna_transition";
const char* MslFile::AUTHOR = "author";
const char* MslFile::AXIS = "axis";
const char* MslFile::AZIMUTH = "azimuth";
const char* MslFile::AZIMUTH_CORRECTION = "azimuth_correction";
const char* MslFile::CALENDAR = "calendar";
const char* MslFile::CFRADIAL = "cfradial";
const char* MslFile::CM = "cm";
const char* MslFile::COMMENT = "comment";
const char* MslFile::COMPRESS = "compress";
const char* MslFile::CONVENTIONS = "Conventions";
const char* MslFile::COORDINATES = "coordinates";
const char* MslFile::CREATED = "created";
const char* MslFile::DB = "db";
const char* MslFile::DBM = "dBm";
const char* MslFile::DBZ = "dBZ";
const char* MslFile::DEGREES = "degrees";
const char* MslFile::DEGREES_EAST = "degrees_east";
const char* MslFile::DEGREES_NORTH = "degrees_north";
const char* MslFile::DEGREES_PER_SECOND = "degrees per second";
const char* MslFile::DORADE = "dorade";
const char* MslFile::DOWN = "down";
const char* MslFile::DRIFT = "drift";
const char* MslFile::DRIFT_CORRECTION = "drift_correction";
const char* MslFile::DRIVER = "driver";
const char* MslFile::DRIVE_ANGLE_1 = "drive_angle_1";
const char* MslFile::DRIVE_ANGLE_2 = "drive_angle_2";
const char* MslFile::EASTWARD_VELOCITY = "eastward_velocity";
const char* MslFile::EASTWARD_VELOCITY_CORRECTION = "eastward_velocity_correction";
const char* MslFile::EASTWARD_WIND = "eastward_wind";
const char* MslFile::ELEVATION = "elevation";
const char* MslFile::ELEVATION_CORRECTION = "elevation_correction";
const char* MslFile::END_DATETIME = "end_datetime";
const char* MslFile::FALSE_NORTHING = "false_northing";
const char* MslFile::FALSE_EASTING = "false_easting";
const char* MslFile::FIELD_FOLDS = "field_folds";
const char* MslFile::FILL_VALUE = "_FillValue";
const char* MslFile::FIXED_ANGLE = "fixed_angle";
const char* MslFile::FOLD_LIMIT_LOWER = "fold_limit_lower";
const char* MslFile::FOLD_LIMIT_UPPER = "fold_limit_upper";
const char* MslFile::FOLLOW_MODE = "follow_mode";
const char* MslFile::FREQUENCY = "frequency";
const char* MslFile::GEOMETRY_CORRECTION = "geometry_correction";
const char* MslFile::GEOREFS_APPLIED = "georefs_applied";
const char* MslFile::GEOREF_TIME = "georef_time";
const char* MslFile::GREGORIAN = "gregorian";
const char* MslFile::GRID_MAPPING = "grid_mapping";
const char* MslFile::GRID_MAPPING_NAME = "grid_mapping_name";
const char* MslFile::HEADING = "heading";
const char* MslFile::HEADING_CHANGE_RATE = "heading_change_rate";
const char* MslFile::HEADING_CORRECTION = "heading_correction";
const char* MslFile::HISTORY = "history";
const char* MslFile::HZ = "s-1";
const char* MslFile::INSTITUTION = "institution";
const char* MslFile::INSTRUMENT_NAME = "instrument_name";
const char* MslFile::INSTRUMENT_PARAMETERS = "instrument_parameters";
const char* MslFile::INSTRUMENT_TYPE = "instrument_type";
const char* MslFile::IS_DISCRETE = "is_discrete";
const char* MslFile::INTERMED_FREQ_HZ = "intermed_freq_hz";
const char* MslFile::JOULES = "joules";
const char* MslFile::JULIAN = "julian";
const char* MslFile::LATITUDE = "latitude";
const char* MslFile::LATITUDE_CORRECTION = "latitude_correction";
const char* MslFile::LATITUDE_OF_PROJECTION_ORIGIN = "latitude_of_projection_origin";
const char* MslFile::LEGEND_XML = "legend_xml";
const char* MslFile::LIDAR_APERTURE_DIAMETER = "lidar_aperture_diameter";
const char* MslFile::LIDAR_APERTURE_EFFICIENCY = "lidar_aperture_efficiency";
const char* MslFile::LIDAR_BEAM_DIVERGENCE = "lidar_beam_divergence";
const char* MslFile::LIDAR_CALIBRATION = "lidar_calibration";
const char* MslFile::LIDAR_CONSTANT = "lidar_constant";
const char* MslFile::LIDAR_FIELD_OF_VIEW = "lidar_field_of_view";
const char* MslFile::LIDAR_PARAMETERS = "lidar_parameters";
const char* MslFile::LIDAR_PEAK_POWER = "lidar_peak_power";
const char* MslFile::LIDAR_PULSE_ENERGY = "lidar_pulse_energy";
const char* MslFile::LONGITUDE = "longitude";
const char* MslFile::LONGITUDE_CORRECTION = "longitude_correction";
const char* MslFile::LONGITUDE_OF_PROJECTION_ORIGIN = "longitude_of_projection_origin";
const char* MslFile::LONG_NAME = "long_name";
const char* MslFile::META_GROUP = "meta_group";
const char* MslFile::METERS = "meters";
const char* MslFile::METERS_BETWEEN_GATES = "meters_between_gates";
const char* MslFile::METERS_PER_SECOND = "meters per second";
const char* MslFile::METERS_TO_CENTER_OF_FIRST_GATE = "meters_to_center_of_first_gate";
const char* MslFile::MISSING_VALUE = "missing_value";
const char* MslFile::MOVING = "moving";
const char* MslFile::MRAD = "mrad";
const char* MslFile::NORTHWARD_VELOCITY = "northward_velocity";
const char* MslFile::NORTHWARD_VELOCITY_CORRECTION = "northward_velocity_correction";
const char* MslFile::NORTHWARD_WIND = "northward_wind";
const char* MslFile::NYQUIST_VELOCITY = "nyquist_velocity";
const char* MslFile::N_GATES_VARY = "n_gates_vary";
const char* MslFile::N_POINTS = "n_points";
const char* MslFile::N_SAMPLES = "n_samples";
const char* MslFile::OPTIONS = "options";
const char* MslFile::ORIGINAL_FORMAT = "original_format";
const char* MslFile::PERCENT = "percent";
const char* MslFile::PITCH = "pitch";
const char* MslFile::PITCH_CHANGE_RATE = "pitch_change_rate";
const char* MslFile::PITCH_CORRECTION = "pitch_correction";
const char* MslFile::PLATFORM_IS_MOBILE = "platform_is_mobile";
const char* MslFile::PLATFORM_TYPE = "platform_type";
const char* MslFile::PLATFORM_VELOCITY = "platform_velocity";
const char* MslFile::POLARIZATION_MODE = "polarization_mode";
const char* MslFile::POSITIVE = "positive";
const char* MslFile::PRESSURE_ALTITUDE_CORRECTION = "pressure_altitude_correction";
const char* MslFile::PRIMARY_AXIS = "primary_axis";
const char* MslFile::PRT = "prt";
const char* MslFile::PRT_MODE = "prt_mode";
const char* MslFile::PRT_RATIO = "prt_ratio";
const char* MslFile::PULSE_WIDTH = "pulse_width";
const char* MslFile::RADAR_ANTENNA_GAIN_H = "radar_antenna_gain_h";
const char* MslFile::RADAR_ANTENNA_GAIN_V = "radar_antenna_gain_v";
const char* MslFile::RADAR_BEAM_WIDTH_H = "radar_beam_width_h";
const char* MslFile::RADAR_BEAM_WIDTH_V = "radar_beam_width_v";
const char* MslFile::RADAR_CALIBRATION = "radar_calibration";
const char* MslFile::RADAR_ESTIMATED_NOISE_DBM_HC = "estimated_noise_dbm_hc";
const char* MslFile::RADAR_ESTIMATED_NOISE_DBM_HX = "estimated_noise_dbm_hx";
const char* MslFile::RADAR_ESTIMATED_NOISE_DBM_VC = "estimated_noise_dbm_vc";
const char* MslFile::RADAR_ESTIMATED_NOISE_DBM_VX = "estimated_noise_dbm_vx";
const char* MslFile::RADAR_MEASURED_TRANSMIT_POWER_H = "measured_transmit_power_h";
const char* MslFile::RADAR_MEASURED_TRANSMIT_POWER_V = "measured_transmit_power_v";
const char* MslFile::RADAR_PARAMETERS = "radar_parameters";
const char* MslFile::RADAR_RX_BANDWIDTH = "radar_rx_bandwidth";
const char* MslFile::RANGE = "range";
const char* MslFile::RANGE_CORRECTION = "range_correction";
const char* MslFile::RAYS_ARE_INDEXED = "rays_are_indexed";
const char* MslFile::RAY_ANGLE_RES = "ray_angle_res";
const char* MslFile::RAY_N_GATES = "ray_n_gates";
const char* MslFile::RAY_START_INDEX = "ray_start_index";
const char* MslFile::RAY_TIMES_INCREASE = "ray_times_increase";
const char* MslFile::REFERENCES = "references";
const char* MslFile::ROLL = "roll";
const char* MslFile::ROLL_CHANGE_RATE = "roll_change_rate";
const char* MslFile::ROLL_CORRECTION = "roll_correction";
const char* MslFile::ROTATION = "rotation";
const char* MslFile::ROTATION_CORRECTION = "rotation_correction";
const char* MslFile::R_CALIB = "r_calib";
const char* MslFile::R_CALIB_ANTENNA_GAIN_H = "r_calib_antenna_gain_h";
const char* MslFile::R_CALIB_ANTENNA_GAIN_V = "r_calib_antenna_gain_v";
const char* MslFile::R_CALIB_BASE_DBZ_1KM_HC = "r_calib_base_dbz_1km_hc";
const char* MslFile::R_CALIB_BASE_DBZ_1KM_HX = "r_calib_base_dbz_1km_hx";
const char* MslFile::R_CALIB_BASE_DBZ_1KM_VC = "r_calib_base_dbz_1km_vc";
const char* MslFile::R_CALIB_BASE_DBZ_1KM_VX = "r_calib_base_dbz_1km_vx";
const char* MslFile::R_CALIB_COUPLER_FORWARD_LOSS_H = "r_calib_coupler_forward_loss_h";
const char* MslFile::R_CALIB_COUPLER_FORWARD_LOSS_V = "r_calib_coupler_forward_loss_v";
const char* MslFile::R_CALIB_INDEX = "r_calib_index";
const char* MslFile::R_CALIB_LDR_CORRECTION_H = "r_calib_ldr_correction_h";
const char* MslFile::R_CALIB_LDR_CORRECTION_V = "r_calib_ldr_correction_v";
const char* MslFile::R_CALIB_NOISE_HC = "r_calib_noise_hc";
const char* MslFile::R_CALIB_NOISE_HX = "r_calib_noise_hx";
const char* MslFile::R_CALIB_NOISE_SOURCE_POWER_H = "r_calib_noise_source_power_h";
const char* MslFile::R_CALIB_NOISE_SOURCE_POWER_V = "r_calib_noise_source_power_v";
const char* MslFile::R_CALIB_NOISE_VC = "r_calib_noise_vc";
const char* MslFile::R_CALIB_NOISE_VX = "r_calib_noise_vx";
const char* MslFile::R_CALIB_POWER_MEASURE_LOSS_H = "r_calib_power_measure_loss_h";
const char* MslFile::R_CALIB_POWER_MEASURE_LOSS_V = "r_calib_power_measure_loss_v";
const char* MslFile::R_CALIB_PULSE_WIDTH = "r_calib_pulse_width";
const char* MslFile::R_CALIB_RADAR_CONSTANT_H = "r_calib_radar_constant_h";
const char* MslFile::R_CALIB_RADAR_CONSTANT_V = "r_calib_radar_constant_v";
const char* MslFile::R_CALIB_RECEIVER_GAIN_HC = "r_calib_receiver_gain_hc";
const char* MslFile::R_CALIB_RECEIVER_GAIN_HX = "r_calib_receiver_gain_hx";
const char* MslFile::R_CALIB_RECEIVER_GAIN_VC = "r_calib_receiver_gain_vc";
const char* MslFile::R_CALIB_RECEIVER_GAIN_VX = "r_calib_receiver_gain_vx";
const char* MslFile::R_CALIB_RECEIVER_MISMATCH_LOSS = "r_calib_receiver_mismatch_loss";
const char* MslFile::R_CALIB_RECEIVER_SLOPE_HC = "r_calib_receiver_slope_hc";
const char* MslFile::R_CALIB_RECEIVER_SLOPE_HX = "r_calib_receiver_slope_hx";
const char* MslFile::R_CALIB_RECEIVER_SLOPE_VC = "r_calib_receiver_slope_vc";
const char* MslFile::R_CALIB_RECEIVER_SLOPE_VX = "r_calib_receiver_slope_vx";
const char* MslFile::R_CALIB_SUN_POWER_HC = "r_calib_sun_power_hc";
const char* MslFile::R_CALIB_SUN_POWER_HX = "r_calib_sun_power_hx";
const char* MslFile::R_CALIB_SUN_POWER_VC = "r_calib_sun_power_vc";
const char* MslFile::R_CALIB_SUN_POWER_VX = "r_calib_sun_power_vx";
const char* MslFile::R_CALIB_SYSTEM_PHIDP = "r_calib_system_phidp";
const char* MslFile::R_CALIB_TEST_POWER_H = "r_calib_test_power_h";
const char* MslFile::R_CALIB_TEST_POWER_V = "r_calib_test_power_v";
const char* MslFile::R_CALIB_TIME = "r_calib_time";
const char* MslFile::R_CALIB_TIME_W3C_STR = "r_calib_time_w3c_str";
const char* MslFile::R_CALIB_TWO_WAY_RADOME_LOSS_H = "r_calib_two_way_radome_loss_h";
const char* MslFile::R_CALIB_TWO_WAY_RADOME_LOSS_V = "r_calib_two_way_radome_loss_v";
const char* MslFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H = "r_calib_two_way_waveguide_loss_h";
const char* MslFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V = "r_calib_two_way_waveguide_loss_v";
const char* MslFile::R_CALIB_XMIT_POWER_H = "r_calib_xmit_power_h";
const char* MslFile::R_CALIB_XMIT_POWER_V = "r_calib_xmit_power_v";
const char* MslFile::R_CALIB_ZDR_CORRECTION = "r_calib_zdr_correction";
const char* MslFile::SAMPLING_RATIO = "sampling_ratio";
const char* MslFile::SCALE_FACTOR = "scale_factor";
const char* MslFile::SCANNING = "scanning";
const char* MslFile::SCANNING_RADIAL = "scanning_radial";
const char* MslFile::SCAN_ID = "scan_id";
const char* MslFile::SCAN_NAME = "scan_name";
const char* MslFile::SCAN_RATE = "scan_rate";
const char* MslFile::SECONDS = "seconds";
const char* MslFile::SECS_SINCE_JAN1_1970 = "seconds since 1970-01-01T00:00:00Z";
const char* MslFile::SITE_NAME = "site_name";
const char* MslFile::SOURCE = "source";
const char* MslFile::SPACING_IS_CONSTANT = "spacing_is_constant";
const char* MslFile::STANDARD = "standard";
const char* MslFile::STANDARD_NAME = "standard_name";
const char* MslFile::STARING = "staring";
const char* MslFile::START_DATETIME = "start_datetime";
const char* MslFile::STATIONARY = "stationary";
const char* MslFile::STATUS_XML = "status_xml";
const char* MslFile::STATUS_XML_LENGTH = "status_xml_length";
const char* MslFile::STRING_LENGTH_256 = "string_length_256";
const char* MslFile::STRING_LENGTH_32 = "string_length_32";
const char* MslFile::STRING_LENGTH_64 = "string_length_64";
const char* MslFile::STRING_LENGTH_8 = "string_length_8";
const char* MslFile::SUB_CONVENTIONS = "Sub_conventions";
const char* MslFile::SWEEP = "sweep";
const char* MslFile::SWEEP_END_RAY_INDEX = "sweep_end_ray_index";
const char* MslFile::SWEEP_MODE = "sweep_mode";
const char* MslFile::SWEEP_NUMBER = "sweep_number";
const char* MslFile::SWEEP_START_RAY_INDEX = "sweep_start_ray_index";
const char* MslFile::TARGET_SCAN_RATE = "target_scan_rate";
const char* MslFile::THRESHOLDING_XML = "thresholding_xml";
const char* MslFile::TILT = "tilt";
const char* MslFile::TILT_CORRECTION = "tilt_correction";
const char* MslFile::TIME = "time";
const char* MslFile::TIME_COVERAGE_END = "time_coverage_end";
const char* MslFile::TIME_COVERAGE_START = "time_coverage_start";
const char* MslFile::TITLE = "title";
const char* MslFile::TRACK = "track";
const char* MslFile::UNAMBIGUOUS_RANGE = "unambiguous_range";
const char* MslFile::UNITS = "units";
const char* MslFile::UP = "up";
const char* MslFile::VALID_MAX = "valid_max";
const char* MslFile::VALID_MIN = "valid_min";
const char* MslFile::VALID_RANGE = "valid_range";
const char* MslFile::VERSION = "version";
const char* MslFile::VERTICAL_VELOCITY = "vertical_velocity";
const char* MslFile::VERTICAL_VELOCITY_CORRECTION = "vertical_velocity_correction";
const char* MslFile::VERTICAL_WIND = "vertical_wind";
const char* MslFile::VOLUME = "volume";
const char* MslFile::VOLUME_NUMBER = "volume_number";
const char* MslFile::W3C_STR = "w3c_str";
const char* MslFile::WATTS = "watts";
  
/////////////////////////////////////////////////////////////////////////////////////
// standard name string constant instantiation

const char* MslFile::ALTITUDE_AGL_LONG = "altitude_above_ground_level";
const char* MslFile::ALTITUDE_CORRECTION_LONG = "altitude_correction";
const char* MslFile::ALTITUDE_LONG = "altitude";
const char* MslFile::ANTENNA_TRANSITION_LONG = "antenna_is_in_transition_between_sweeps";
const char* MslFile::AZIMUTH_CORRECTION_LONG = "azimuth_angle_correction";
const char* MslFile::AZIMUTH_LONG = "ray_azimuth_angle";
const char* MslFile::DRIFT_CORRECTION_LONG = "platform_drift_angle_correction";
const char* MslFile::DRIFT_LONG = "platform_drift_angle";
const char* MslFile::EASTWARD_VELOCITY_CORRECTION_LONG = "platform_eastward_velocity_correction";
const char* MslFile::EASTWARD_VELOCITY_LONG = "platform_eastward_velocity";
const char* MslFile::EASTWARD_WIND_LONG = "eastward_wind_speed";
const char* MslFile::ELEVATION_CORRECTION_LONG = "ray_elevation_angle_correction";
const char* MslFile::ELEVATION_LONG = "ray_elevation_angle";
const char* MslFile::FIXED_ANGLE_LONG = "ray_target_fixed_angle";
const char* MslFile::FOLLOW_MODE_LONG = "follow_mode_for_scan_strategy";
const char* MslFile::FREQUENCY_LONG = "transmission_frequency";
const char* MslFile::GEOREF_TIME_LONG = "georef time in seconds since volume start";
const char* MslFile::HEADING_CHANGE_RATE_LONG = "platform_heading_angle_rate_of_change";
const char* MslFile::HEADING_CORRECTION_LONG = "platform_heading_angle_correction";
const char* MslFile::HEADING_LONG = "platform_heading_angle";
const char* MslFile::INSTRUMENT_NAME_LONG = "name_of_instrument";
const char* MslFile::INSTRUMENT_TYPE_LONG = "type_of_instrument";
const char* MslFile::INTERMED_FREQ_HZ_LONG = "intermediate_freqency_hz";
const char* MslFile::LATITUDE_CORRECTION_LONG = "latitude_correction";
const char* MslFile::LATITUDE_LONG = "latitude";
const char* MslFile::LIDAR_APERTURE_DIAMETER_LONG = "lidar_aperture_diameter";
const char* MslFile::LIDAR_APERTURE_EFFICIENCY_LONG = "lidar_aperture_efficiency";
const char* MslFile::LIDAR_BEAM_DIVERGENCE_LONG = "lidar_beam_divergence";
const char* MslFile::LIDAR_CONSTANT_LONG = "lidar_calibration_constant";
const char* MslFile::LIDAR_FIELD_OF_VIEW_LONG = "lidar_field_of_view";
const char* MslFile::LIDAR_PEAK_POWER_LONG = "lidar_peak_power";
const char* MslFile::LIDAR_PULSE_ENERGY_LONG = "lidar_pulse_energy";
const char* MslFile::LONGITUDE_CORRECTION_LONG = "longitude_correction";
const char* MslFile::LONGITUDE_LONG = "longitude";
const char* MslFile::NORTHWARD_VELOCITY_CORRECTION_LONG = "platform_northward_velocity_correction";
const char* MslFile::NORTHWARD_VELOCITY_LONG = "platform_northward_velocity";
const char* MslFile::NORTHWARD_WIND_LONG = "northward_wind";
const char* MslFile::NYQUIST_VELOCITY_LONG = "unambiguous_doppler_velocity";
const char* MslFile::N_SAMPLES_LONG = "number_of_samples_used_to_compute_moments";
const char* MslFile::PITCH_CHANGE_RATE_LONG = "platform_pitch_angle_rate_of_change";
const char* MslFile::PITCH_CORRECTION_LONG = "platform_pitch_angle_correction";
const char* MslFile::PITCH_LONG = "platform_pitch_angle";
const char* MslFile::PLATFORM_IS_MOBILE_LONG = "platform_is_mobile";
const char* MslFile::PLATFORM_TYPE_LONG = "platform_type";
const char* MslFile::POLARIZATION_MODE_LONG = "polarization_mode_for_sweep";
const char* MslFile::PRESSURE_ALTITUDE_CORRECTION_LONG = "pressure_altitude_correction";
const char* MslFile::PRIMARY_AXIS_LONG = "primary_axis_of_rotation";
const char* MslFile::PRT_MODE_LONG = "transmit_pulse_mode";
const char* MslFile::PRT_RATIO_LONG = "pulse_repetition_frequency_ratio";
const char* MslFile::PRT_LONG = "pulse_repetition_time";
const char* MslFile::PULSE_WIDTH_LONG = "transmitter_pulse_width";
const char* MslFile::RADAR_ANTENNA_GAIN_H_LONG = "nominal_radar_antenna_gain_h_channel";
const char* MslFile::RADAR_ANTENNA_GAIN_V_LONG = "nominal_radar_antenna_gain_v_channel";
const char* MslFile::RADAR_BEAM_WIDTH_H_LONG = "half_power_radar_beam_width_h_channel";
const char* MslFile::RADAR_BEAM_WIDTH_V_LONG = "half_power_radar_beam_width_v_channel";
const char* MslFile::RADAR_ESTIMATED_NOISE_DBM_HC_LONG = "estimated_noise_dbm_hc";
const char* MslFile::RADAR_ESTIMATED_NOISE_DBM_HX_LONG = "estimated_noise_dbm_hx";
const char* MslFile::RADAR_ESTIMATED_NOISE_DBM_VC_LONG = "estimated_noise_dbm_vc";
const char* MslFile::RADAR_ESTIMATED_NOISE_DBM_VX_LONG = "estimated_noise_dbm_vx";
const char* MslFile::RADAR_MEASURED_TRANSMIT_POWER_H_LONG = "measured_radar_transmit_power_h_channel";
const char* MslFile::RADAR_MEASURED_TRANSMIT_POWER_V_LONG = "measured_radar_transmit_power_v_channel";
const char* MslFile::RADAR_RX_BANDWIDTH_LONG = "radar_receiver_bandwidth";
const char* MslFile::RANGE_CORRECTION_LONG = "range_to_center_of_measurement_volume_correction";
const char* MslFile::RANGE_LONG = "range_to_center_of_measurement_volume";
const char* MslFile::RAYS_ARE_INDEXED_LONG = "flag_for_indexed_rays";
const char* MslFile::RAY_ANGLE_RES_LONG = "angular_resolution_between_rays";
const char* MslFile::ROLL_CHANGE_RATE_LONG = "platform_roll_angle_rate_of_change";
const char* MslFile::ROLL_CORRECTION_LONG = "platform_roll_angle_correction";
const char* MslFile::ROLL_LONG = "platform_roll_angle";
const char* MslFile::ROTATION_CORRECTION_LONG = "ray_rotation_angle_relative_to_platform_correction";
const char* MslFile::ROTATION_LONG = "ray_rotation_angle_relative_to_platform";
const char* MslFile::R_CALIB_ANTENNA_GAIN_H_LONG = "calibrated_radar_antenna_gain_h_channel";
const char* MslFile::R_CALIB_ANTENNA_GAIN_V_LONG = "calibrated_radar_antenna_gain_v_channel";
const char* MslFile::R_CALIB_BASE_DBZ_1KM_HC_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_co_polar_channel";
const char* MslFile::R_CALIB_BASE_DBZ_1KM_HX_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_cross_polar_channel";
const char* MslFile::R_CALIB_BASE_DBZ_1KM_VC_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_co_polar_channel";
const char* MslFile::R_CALIB_BASE_DBZ_1KM_VX_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_cross_polar_channel";
const char* MslFile::R_CALIB_COUPLER_FORWARD_LOSS_H_LONG = "radar_calibration_coupler_forward_loss_h_channel";
const char* MslFile::R_CALIB_COUPLER_FORWARD_LOSS_V_LONG = "radar_calibration_coupler_forward_loss_v_channel";
const char* MslFile::R_CALIB_INDEX_LONG = "calibration_data_array_index_per_ray";
const char* MslFile::R_CALIB_LDR_CORRECTION_H_LONG = "calibrated_radar_ldr_correction_h_channel";
const char* MslFile::R_CALIB_LDR_CORRECTION_V_LONG = "calibrated_radar_ldr_correction_v_channel";
const char* MslFile::R_CALIB_NOISE_HC_LONG = "calibrated_radar_receiver_noise_h_co_polar_channel";
const char* MslFile::R_CALIB_NOISE_HX_LONG = "calibrated_radar_receiver_noise_h_cross_polar_channel";
const char* MslFile::R_CALIB_NOISE_SOURCE_POWER_H_LONG = "radar_calibration_noise_source_power_h_channel";
const char* MslFile::R_CALIB_NOISE_SOURCE_POWER_V_LONG = "radar_calibration_noise_source_power_v_channel";
const char* MslFile::R_CALIB_NOISE_VC_LONG = "calibrated_radar_receiver_noise_v_co_polar_channel";
const char* MslFile::R_CALIB_NOISE_VX_LONG = "calibrated_radar_receiver_noise_v_cross_polar_channel";
const char* MslFile::R_CALIB_POWER_MEASURE_LOSS_H_LONG = "radar_calibration_power_measurement_loss_h_channel";
const char* MslFile::R_CALIB_POWER_MEASURE_LOSS_V_LONG = "radar_calibration_power_measurement_loss_v_channel";
const char* MslFile::R_CALIB_PULSE_WIDTH_LONG = "radar_calibration_pulse_width";
const char* MslFile::R_CALIB_RADAR_CONSTANT_H_LONG = "calibrated_radar_constant_h_channel";
const char* MslFile::R_CALIB_RADAR_CONSTANT_V_LONG = "calibrated_radar_constant_v_channel";
const char* MslFile::R_CALIB_RECEIVER_GAIN_HC_LONG = "calibrated_radar_receiver_gain_h_co_polar_channel";
const char* MslFile::R_CALIB_RECEIVER_GAIN_HX_LONG = "calibrated_radar_receiver_gain_h_cross_polar_channel";
const char* MslFile::R_CALIB_RECEIVER_GAIN_VC_LONG = "calibrated_radar_receiver_gain_v_co_polar_channel";
const char* MslFile::R_CALIB_RECEIVER_GAIN_VX_LONG = "calibrated_radar_receiver_gain_v_cross_polar_channel";
const char* MslFile::R_CALIB_RECEIVER_MISMATCH_LOSS_LONG = "radar_calibration_receiver_mismatch_loss";
const char* MslFile::R_CALIB_RECEIVER_SLOPE_HC_LONG = "calibrated_radar_receiver_slope_h_co_polar_channel";
const char* MslFile::R_CALIB_RECEIVER_SLOPE_HX_LONG = "calibrated_radar_receiver_slope_h_cross_polar_channel";
const char* MslFile::R_CALIB_RECEIVER_SLOPE_VC_LONG = "calibrated_radar_receiver_slope_v_co_polar_channel";
const char* MslFile::R_CALIB_RECEIVER_SLOPE_VX_LONG = "calibrated_radar_receiver_slope_v_cross_polar_channel";
const char* MslFile::R_CALIB_SUN_POWER_HC_LONG = "calibrated_radar_sun_power_h_co_polar_channel";
const char* MslFile::R_CALIB_SUN_POWER_HX_LONG = "calibrated_radar_sun_power_h_cross_polar_channel";
const char* MslFile::R_CALIB_SUN_POWER_VC_LONG = "calibrated_radar_sun_power_v_co_polar_channel";
const char* MslFile::R_CALIB_SUN_POWER_VX_LONG = "calibrated_radar_sun_power_v_cross_polar_channel";
const char* MslFile::R_CALIB_SYSTEM_PHIDP_LONG = "calibrated_radar_system_phidp";
const char* MslFile::R_CALIB_TEST_POWER_H_LONG = "radar_calibration_test_power_h_channel";
const char* MslFile::R_CALIB_TEST_POWER_V_LONG = "radar_calibration_test_power_v_channel";
const char* MslFile::R_CALIB_TIME_LONG = "radar_calibration_time_utc";
const char* MslFile::R_CALIB_TWO_WAY_RADOME_LOSS_H_LONG = "radar_calibration_two_way_radome_loss_h_channel";
const char* MslFile::R_CALIB_TWO_WAY_RADOME_LOSS_V_LONG = "radar_calibration_two_way_radome_loss_v_channel";
const char* MslFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H_LONG = "radar_calibration_two_way_waveguide_loss_h_channel";
const char* MslFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V_LONG = "radar_calibration_two_way_waveguide_loss_v_channel";
const char* MslFile::R_CALIB_XMIT_POWER_H_LONG = "calibrated_radar_xmit_power_h_channel";
const char* MslFile::R_CALIB_XMIT_POWER_V_LONG = "calibrated_radar_xmit_power_v_channel";
const char* MslFile::R_CALIB_ZDR_CORRECTION_LONG = "calibrated_radar_zdr_correction";
const char* MslFile::SCAN_ID_LONG = "volume_coverage_pattern";
const char* MslFile::SCAN_NAME_LONG = "name_of_antenna_scan_strategy";
const char* MslFile::SCAN_RATE_LONG = "antenna_angle_scan_rate";
const char* MslFile::SITE_NAME_LONG = "name_of_instrument_site";
const char* MslFile::SPACING_IS_CONSTANT_LONG = "spacing_between_range_gates_is_constant";
const char* MslFile::SWEEP_END_RAY_INDEX_LONG = "index_of_last_ray_in_sweep";
const char* MslFile::SWEEP_MODE_LONG = "scan_mode_for_sweep";
const char* MslFile::SWEEP_NUMBER_LONG = "sweep_index_number_0_based";
const char* MslFile::SWEEP_START_RAY_INDEX_LONG = "index_of_first_ray_in_sweep";
const char* MslFile::TARGET_SCAN_RATE_LONG = "target_scan_rate_for_sweep";
const char* MslFile::TILT_CORRECTION_LONG = "ray_tilt_angle_relative_to_platform_correction";
const char* MslFile::TILT_LONG = "ray_tilt_angle_relative_to_platform";
const char* MslFile::TIME_COVERAGE_END_LONG = "data_volume_end_time_utc";
const char* MslFile::TIME_COVERAGE_START_LONG = "data_volume_start_time_utc";
const char* MslFile::TIME_LONG = "time";
const char* MslFile::TRACK_LONG = "platform_track_over_the_ground";
const char* MslFile::UNAMBIGUOUS_RANGE_LONG = "unambiguous_range";
const char* MslFile::VERTICAL_VELOCITY_CORRECTION_LONG = "platform_vertical_velocity_correction";
const char* MslFile::VERTICAL_VELOCITY_LONG = "platform_vertical_velocity";
const char* MslFile::VERTICAL_WIND_LONG = "upward_air_velocity";
const char* MslFile::VOLUME_NUMBER_LONG = "data_volume_index_number";

