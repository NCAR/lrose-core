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
// CfarrNcRadxFile.cc
//
// CfarrNcRadxFile object
//
// NetCDF data for radar radial data in CFAR netcdf format
// CFARR = Chilbolton Facility for Atmospheric and Radio Research
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, 
// July 2017
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/CfarrNcRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
using namespace std;

//////////////
// Constructor

CfarrNcRadxFile::CfarrNcRadxFile() : RadxFile()
  
{

  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

CfarrNcRadxFile::~CfarrNcRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void CfarrNcRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _timeDim = NULL;
  _rangeDim = NULL;

  _timeVar = NULL;
  _rangeVar = NULL;
  _latitudeVar = NULL;
  _longitudeVar = NULL;
  _heightVar = NULL;

  _frequencyVar = NULL;
  _prfVar = NULL;
  _beamwidthHVar = NULL;
  _beamwidthVVar = NULL;
  _antennaDiameterVar = NULL;
  _pulsePeriodVar = NULL;
  _transmitPowerVar = NULL;
  _sweepVar = NULL;
  _azimuthVar = NULL;
  _elevationVar = NULL;

  _dTimes.clear();
  _nTimesInFile = 0;
  _rayTimesIncrease = true;
  _refTimeSecsFile = 0;

  _rangeKm.clear();
  _nRangeInFile = 0;
  _gateSpacingIsConstant = true;

  _latitudeDeg = 0;
  _longitudeDeg = 0;
  _heightKm = 0;

  _frequencyGhz = Radx::missingMetaDouble;
  _prfHz = Radx::missingMetaDouble;
  _beamwidthHDeg = Radx::missingMetaDouble;
  _beamwidthVDeg = Radx::missingMetaDouble;
  _antennaDiameterM = Radx::missingMetaDouble;
  _pulsePeriodUs = Radx::missingMetaDouble;
  _transmitPowerW = Radx::missingMetaDouble;  

  _azimuths.clear();
  _elevations.clear();
  
  _British_National_Grid_Reference_attr.clear();
  _Conventions_attr.clear();
  _operator_attr.clear();
  _radar_attr.clear();
  _references_attr.clear();
  _scan_datetime_attr.clear();
  _scantype_attr.clear();

  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();

  _statusXml.clear(); // global attributes
  _siteName.clear();
  _scanName.clear();
  _instrumentName.clear();

  _scanId = 0;

  _volumeNumber = 0;
  _instrumentType = Radx::missingInstrumentType;
  _platformType = Radx::missingPlatformType;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

}

/////////////////////////////////////////////////////////
// Check if specified file is Cfarr format
// Returns true if supported, false otherwise

bool CfarrNcRadxFile::isSupported(const string &path)

{
  
  if (isCfarrNc(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a CfarrNc file
// Returns true on success, false on failure

bool CfarrNcRadxFile::isCfarrNc(const string &path)
  
{

  clear();
  if (_debug) {
    cerr << "DEBUG - inside isCfarrNc file" << endl;
  }
  // open file

  if (_file.openRead(path)) {
    if (_debug) {
      cerr << "DEBUG openRead failed" << endl;
    }
    if (_verbose) {
      cerr << "DEBUG - not CfarrNc file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }
  
  if (_debug) {
    cerr << "DEBUG - before read dimensions " << endl;
  }
  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfarrNc file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check existence of some variables
  if (_debug) {
    cerr << "DEBUG - before reading beamwidthV" << endl;
  }
  Nc3Var *baseTimeVar = _file.getNc3File()->get_var("beamwidthV");
  if (baseTimeVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfarrNc file" << endl;
      cerr << "  beamwidthV variable missing" << endl;
    }
    return false;
  }
  
  if (_debug) {
    cerr << "DEBUG - before reading transmit_power" << endl;
  }
  Nc3Var *qcTimeVar = _file.getNc3File()->get_var("transmit_power");
  if (qcTimeVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfarrNc file" << endl;
      cerr << "  transmit_power variable missing" << endl;
    }
    return false;
  }

  // file has the correct dimensions, so it is a CfarrNc file

  _file.close();
  if (_debug) {
    cerr << "DEBUG - it's all good! we have a Cfarr file " << endl;
  }
  return true;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int CfarrNcRadxFile::writeToDir(const RadxVol &vol,
                                const string &dir,
                                bool addDaySubDir,
                                bool addYearSubDir)
  
{

  // Writing CfarrNc files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - CfarrNcRadxFile::writeToDir" << endl;
  cerr << "  Writing CfarrNc format files not supported" << endl;
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

int CfarrNcRadxFile::writeToPath(const RadxVol &vol,
                                 const string &path)
  
{

  // Writing CfarrNc files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - CfarrNcRadxFile::writeToPath" << endl;
  cerr << "  Writing CfarrNc format files not supported" << endl;
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

int CfarrNcRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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

void CfarrNcRadxFile::print(ostream &out) const
  
{
  
  out << "=============== CfarrNcRadxFile ===============" << endl;
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
  out << "  refTimeSecsFile: " << RadxTime::strm(_refTimeSecsFile) << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " 
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  out << "  latitude: " << _latitudeDeg << endl;
  out << "  longitude: " << _longitudeDeg << endl;
  out << "  height: " << _heightKm << endl;
  out << "  frequencyGhz: " << _frequencyGhz << endl;
  
  out << "  startRangeKm: " << _remap.getStartRangeKm() << endl;
  out << "  gateSpacingKm: " << _remap.getGateSpacingKm() << endl;
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int CfarrNcRadxFile::printNative(const string &path, ostream &out,
                                 bool printRays, bool printData)
  
{

  _addErrStr("ERROR - CfarrNcRadxFile::printNative");
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

int CfarrNcRadxFile::readFromPath(const string &path,
                                  RadxVol &vol)
  
{

  _initForRead(path, vol);
  
  if (_debug) {
    cerr << "Reading path: " << path << endl;
  }

  string errStr("ERROR - CfarrNcRadxFile::readFromPath");
  
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

  // read time variable now if that is all that is needed
  
  if (_readTimesOnly) {
    if (_readTimes()) {
      _addErrStr(errStr);
      return -1;
    }
    return 0;
  }
  
  // read global attributes
  if (_debug) {
    cerr << "Reading global attributes " << endl; 
  }
  if (_readGlobalAttributes()) {
    _addErrStr(errStr);
    return -1;
  }
 
  if (_debug) {
    cerr << " reading time variable " << endl; 
  }
  // read time variable
  
  if (_readTimes()) {
    _addErrStr(errStr);
    return -1;
  }

  // read range variable
  // the range array size will be the max of the arrays found in
  // the files
   
  if (_debug) {
    cerr << " reading range  variable " << endl; 
  }
  if (_readRangeVariable()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read position variables - lat/lon/alt 
  if (_debug) {
    cerr << " reading position  variable " << endl; 
  }
  if (_readPositionVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // read scalar variables
  if (_debug) {
    cerr << " reading scalar variables " << endl; 
  }
  if (_readScalarVariables()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read in ray variables
  if (_debug) {
    cerr << " reading ray  variable " << endl; 
  }
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
  if (_debug) {
    cerr << " finished reading  " << endl; 
  }
  // close file

  _file.close();
  
  // add file rays to main rays
  
  _raysValid.clear();
  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    RadxRay *ray = _raysToRead[ii];
    
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
  if (_debug) {
    cerr << "before _loadReadVolume() " << endl; 
  }
  if (_loadReadVolume()) {
    return -1;
  }
  if (_debug) {
    cerr << "after _loadReadVolume() " << endl;
  }
  // compute fixed angles as mean angle from sweeps
  
  _computeFixedAngles();
  
  // set format as read

  _fileFormat = FILE_FORMAT_CFARR;

  // clean up

  _clearRayVariables();
  _dTimes.clear();

  return 0;

}

///////////////////////////////////
// read in the dimensions

int CfarrNcRadxFile::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  iret |= _file.readDim("time", _timeDim);
  if (iret == 0) {
    _nTimesInFile = _timeDim->size();
  }

  _nRangeInFile = 0;
  iret |= _file.readDim("range", _rangeDim);
  if (iret == 0) {
    _nRangeInFile = _rangeDim->size();
  }
  
  // There is only one sweep in a Cfarr file.

  if (iret) {
    _addErrStr("ERROR - CfarrNcRadxFile::_file.readDimensions");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read the global attributes

int CfarrNcRadxFile::_readGlobalAttributes()

{

   _file.readGlobAttr("scan_number", _scan_number_attr);
   _file.readGlobAttr("file_number", _file_number_attr);

   _file.readGlobAttr("scantype", _scantype_attr);
   _file.readGlobAttr("experiment_id", _experiment_id_attr);
   _file.readGlobAttr("operator", _operator_attr);
   _file.readGlobAttr("scan_velocity", _scan_velocity_attr);
   _file.readGlobAttr("min_range", _min_range_attr);
   _file.readGlobAttr("max_range", _max_range_attr);
   _file.readGlobAttr("min_angle", _min_angle_attr);
   _file.readGlobAttr("max_angle", _max_angle_attr);
   _file.readGlobAttr("scan_angle", _scan_angle_attr);
   _file.readGlobAttr("scan_datetime", _scan_datetime_attr);
   _file.readGlobAttr("extra_attenuation", _extra_attenuation_attr);
   _file.readGlobAttr("ADC_bits_per_sample", _ADC_bits_per_sample_attr);
   _file.readGlobAttr("samples_per_pulse", _samples_per_pulse_attr);
   _file.readGlobAttr("pulses_per_daq_cycle", _pulses_per_daq_cycle_attr);
   _file.readGlobAttr("ADC_channels", _ADC_channels_attr);
   _file.readGlobAttr("delay_clocks", _delay_clocks_attr);
   _file.readGlobAttr("pulses_per_ray", _pulses_per_ray_attr);
   _file.readGlobAttr("radar_constant", _radar_constant_attr);
   _file.readGlobAttr("receiver_gain", _receiver_gain_attr);
   _file.readGlobAttr("cable_losses", _cable_losses_attr);
   _file.readGlobAttr("year", _year_attr);
   _file.readGlobAttr("month", _month_attr);
   _file.readGlobAttr("day", _day_attr);
   _file.readGlobAttr("British_National_Grid_Reference", _British_National_Grid_Reference_attr);
   _file.readGlobAttr("history", _history);
   _file.readGlobAttr("source", _source);
   _file.readGlobAttr("radar", _radar_attr);
   _file.readGlobAttr("Conventions", _Conventions_attr);
   _file.readGlobAttr("title", _title);
   _file.readGlobAttr("comment", _comment);
   _file.readGlobAttr("institution", _institution);
   _file.readGlobAttr("references", _references);

   _siteName = _radar_attr;
   _instrumentName = _radar_attr;

  return 0;

}

///////////////////////////////////
// read the times

int CfarrNcRadxFile::_readTimes()

{

  // read the time variable

  _timeVar = _file.getNc3File()->get_var("time");
  if (_timeVar == NULL) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readTimes");
    _addErrStr("  Cannot find time variable, name: ", "time");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  if (_timeVar->num_dims() < 1) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = _timeVar->get_dim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim->name());
    return -1;
  }

  // get units attribute
  
  Nc3Att* unitsAtt = _timeVar->get_att("units");
  if (unitsAtt == NULL) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }
  string units = Nc3xFile::asString(unitsAtt);
  delete unitsAtt;

#ifdef NOTNOW

  // check if this is a time variable, using udunits
  
  ut_unit *udUnit = ut_parse(_udunits.getSystem(), units.c_str(), UT_ASCII);
  if (udUnit == NULL) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readTimes");
    _addErrStr("  Cannot parse time units: ", units);
    _addErrInt("  udunits status: ", ut_get_status());
    return -1;
  }
    
  if (ut_are_convertible(udUnit, _udunits.getEpoch()) == 0) {
    // not a time variable
    _addErrStr("ERROR - CfarrNcRadxFile::_readTimes");
    _addErrStr("  Time does not have convertible units: ", units);
    _addErrInt("  udunits status: ", ut_get_status());
    ut_free(udUnit);
    return -1;
  }
  
  // get ref time as unix time using udunits
  
  cv_converter *conv = ut_get_converter(udUnit, _udunits.getEpoch());
  double refTimeDouble = cv_convert_double(conv, 0);
  _refTimeSecsFile = (time_t) refTimeDouble;

#endif
  
  // parse the time units reference time

  RadxTime stime(units);
  _refTimeSecsFile = stime.utime();
  
  // set the time array
  
  RadxArray<double> dtimes_;
  double *dtimes = dtimes_.alloc(_nTimesInFile);
  if (_timeVar->get(dtimes, _nTimesInFile) == 0) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readTimes");
    _addErrStr("  Cannot read times variable");
    return -1;
  }
  _dTimes.clear();
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    _dTimes.push_back(dtimes[ii]);
  }

  return 0;

}

///////////////////////////////////
// read the range variable

int CfarrNcRadxFile::_readRangeVariable()

{

  _rangeVar = _file.getNc3File()->get_var("range");
  if (_rangeVar == NULL || _rangeVar->num_vals() < 1) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readRangeVariable");
    _addErrStr("  Cannot read range");
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
    for (size_t ii = 0; ii < _nRangeInFile; ii++, rr++) {
      _rangeKm.push_back(*rr * kmPerUnit);
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

int CfarrNcRadxFile::_readPositionVariables()

{

  // find latitude, longitude, height

  int iret = 0;
  if (_file.readDoubleVar(_latitudeVar, "latitude", _latitudeDeg, 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readPositionVariables");
    _addErrStr("  Cannot read latitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_longitudeVar, "longitude", _longitudeDeg, 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readPositionVariables");
    _addErrStr("  Cannot read longitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_heightVar, "height", _heightKm, 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readPositionVariables");
    _addErrStr("  Cannot read height");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }
  Nc3Att* unitsAtt = _heightVar->get_att("units");
  if (unitsAtt != NULL) {
    string units = Nc3xFile::asString(unitsAtt);
    if (units == "m") {
      _heightKm /= 1000.0;
    }
    delete unitsAtt;
  }
  
  return iret;

}///////////////////////////////////
// read scalar variables

int CfarrNcRadxFile::_readScalarVariables()

{

  // 

  int iret = 0;
  if (_file.readDoubleVar(_beamwidthHVar, "beamwidthH", _beamwidthHDeg , 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read beam width H");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_beamwidthVVar, "beamwidthV", _beamwidthVDeg , 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read beam width V");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_frequencyVar, "frequency", _frequencyGhz, 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read frequency");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_prfVar, "prf", _prfHz, 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read prf");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_antennaDiameterVar, "antenna_diameter", _antennaDiameterM, 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read antenna diameter");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_transmitPowerVar, "transmit_power", _transmitPowerW, 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read transmit power");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_pulsePeriodVar, "pulse_period", _pulsePeriodUs, 0, true)) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readScalarVariables");
    _addErrStr("  Cannot read pulse period");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  return iret;

}

///////////////////////////////////
// clear the ray variables

void CfarrNcRadxFile::_clearRayVariables()

{
  _azimuths.clear();
  _elevations.clear();
}

///////////////////////////////////
// read in ray variables

int CfarrNcRadxFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  _readRayVar(_azimuthVar, "azimuth", _azimuths);
  if ((int) _azimuths.size() != _timeDim->size()) {
    _addErrStr("ERROR - Azimuth_current variable required");
    iret = -1;
  }
  
  _readRayVar(_elevationVar, "elevation", _elevations);
  if ((int) _elevations.size() != _timeDim->size()) {
    _addErrStr("ERROR - Elevation_current variable required");
    iret = -1;
  }

  if (iret) {
    _addErrStr("ERROR - CfarrNcRadxFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int CfarrNcRadxFile::_createRays(const string &path)

{
  // create the rays array

  _raysToRead.clear();
  
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {

    // new ray
    
    RadxRay *ray = new RadxRay;
    // rayInfo.ray = ray;

    ray->copyRangeGeom(_geom);
    
    // set time
    
    double rayTimeDouble = _dTimes[ii];
    time_t rayUtimeSecs = _refTimeSecsFile + (time_t) rayTimeDouble;
    double rayIntSecs;
    double rayFracSecs = modf(rayTimeDouble, &rayIntSecs);
    int rayNanoSecs = (int) (rayFracSecs * 1.0e9);
    ray->setTime(rayUtimeSecs, rayNanoSecs);
    
    // sweep info
    
    ray->setAzimuthDeg(_azimuths[ii]);
    ray->setElevationDeg(_elevations[ii]);
    ray->setPrtSec(1.0/_prfHz);
    ray->setTargetScanRateDegPerSec(_scan_velocity_attr);
    ray->setNSamples(_pulses_per_ray_attr);

    // add to ray vector for reading

    _raysToRead.push_back(ray);

  } // ii

  return 0;

}

////////////////////////////////////////////
// read the field variables

int CfarrNcRadxFile::_readFieldVariables(bool metaOnly)

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
        cerr << "DEBUG - CfarrNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
        cerr << "  -->> Should be float or double: " << fieldName << endl;
      }
      continue;
    }

    // check that we need this field

    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - CfarrNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - CfarrNcRadxFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << fieldName << endl;
    }
    
    // set names, units, etc
    
    string name = var->name();
    
    string standardName;
    Nc3Att *standardNameAtt = var->get_att("chilbolton_standard_name");
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
    Nc3Att *unitsAtt = var->get_att("units");
    if (unitsAtt != NULL) {
      units = Nc3xFile::asString(unitsAtt);
      delete unitsAtt;
    }
 
    // folding
    string shortName;
    Nc3Att *shortNameAtt = var->get_att("short_name");
    if (shortNameAtt != NULL) {
      shortName = Nc3xFile::asString(shortNameAtt);
      delete shortNameAtt;
    }

    bool fieldFolds = false;
    float foldLimitLower = Radx::missingMetaFloat;
    float foldLimitUpper = Radx::missingMetaFloat;
    if (shortName.find("Doppler velocity") != string::npos) {
      fieldFolds = true;
      float foldingVelocity = 0.0;
      Nc3Att *foldingVelocityAtt = var->get_att("folding_velocity");
      if (foldingVelocityAtt != NULL) {
        foldingVelocity = foldingVelocityAtt->as_double(0);
        delete foldingVelocityAtt;
      }
      foldLimitLower = foldingVelocity * -1.0;
      foldLimitUpper = foldingVelocity;
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
        if (fieldFolds &&
            foldLimitLower != Radx::missingMetaFloat &&
            foldLimitUpper != Radx::missingMetaFloat) {
          field->setFieldFolds(foldLimitLower, foldLimitUpper);
        }
        _readVol->addField(field);
      }
      continue;
    }

    int iret = 0;
    bool isDiscrete = false;

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
      _addErrStr("ERROR - CfarrNcRadxFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  } // ivar

  return 0;

}

///////////////////////////////////
// read a ray variable - double

int CfarrNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
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
      _addErrStr("ERROR - CfarrNcRadxFile::_readRayVar");
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
      _addErrStr("ERROR - CfarrNcRadxFile::_readRayVar");
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

int CfarrNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
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
      _addErrStr("ERROR - CfarrNcRadxFile::_readRayVar");
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
      _addErrStr("ERROR - CfarrNcRadxFile::_readRayVar");
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

Nc3Var* CfarrNcRadxFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - CfarrNcRadxFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - CfarrNcRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - CfarrNcRadxFile::_getRayVar");
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

int CfarrNcRadxFile::_addFl64FieldToRays(Nc3Var* var,
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

  Radx::fl64 *data = new Radx::fl64[_nTimesInFile * _nRangeInFile];
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
  }

  // look for fill value, if not present, then use missing value

  Radx::fl64 fillVal = missingVal;
  Nc3Att *fillValueAtt = var->get_att("_FillValue");
  if (fillValueAtt != NULL) {
    fillVal = fillValueAtt->as_double(0);
    delete fillValueAtt;
  }

  // replace any NaN's with fill value
  for (size_t jj = 0; jj < _nTimesInFile * _nRangeInFile; jj++) {
    if (std::isnan(data[jj]))
      data[jj] = fillVal;
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    int nGates = _nRangeInFile;
    int startIndex = ii * _nRangeInFile;
    
    RadxField *field =
      _raysToRead[ii]->addField(name, units, nGates,
				missingVal,
				data + startIndex,
				true);

    field->setMissingFl64(missingVal);
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

int CfarrNcRadxFile::_addFl32FieldToRays(Nc3Var* var,
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

  Radx::fl32 *data = new Radx::fl32[_nTimesInFile * _nRangeInFile];
  int iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value
  
  Radx::fl32 missingVal = Radx::missingFl32;
  Nc3Att *missingValueAtt = var->get_att("missing_value");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_float(0);
    delete missingValueAtt;
  }

  // look for fill value, if not present, then use missing value

  Radx::fl32 fillVal = missingVal;
  Nc3Att *fillValueAtt = var->get_att("_FillValue");
  if (fillValueAtt != NULL) {
    fillVal = fillValueAtt->as_float(0);
    delete fillValueAtt;
  }

  // replace any NaN's with fill value
  for (size_t jj = 0; jj < _nTimesInFile * _nRangeInFile; jj++) {
    if (std::isnan(data[jj]))
      data[jj] = fillVal;
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    int nGates = _nRangeInFile;
    int startIndex = ii * _nRangeInFile;

    RadxField *field =
      _raysToRead[ii]->addField(name, units, nGates,
				missingVal,
				data + startIndex,
				true);
    
    field->setMissingFl32(missingVal);
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

int CfarrNcRadxFile::_loadReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("Cfarr");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  _readVol->addFrequencyHz(_frequencyGhz * 1.0e9);

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
  _readVol->setAltitudeKm(_heightKm);

  _readVol->copyRangeGeom(_geom);

  _readVol->setRadarBeamWidthDegH(_beamwidthHDeg);
  _readVol->setRadarBeamWidthDegV(_beamwidthVDeg);

  // add calibration 
  RadxRcalib *cal = new RadxRcalib;
  cal->setPulseWidthUsec(_pulsePeriodUs);
  cal->setRadarConstantH(_radar_constant_attr);
  cal->setRadarConstantV(_radar_constant_attr);
  cal->setReceiverGainDbHc(_receiver_gain_attr);
  cal->setReceiverGainDbHx(_receiver_gain_attr);
  cal->setReceiverGainDbVc(_receiver_gain_attr);
  cal->setReceiverGainDbVx(_receiver_gain_attr);
  cal->setPowerMeasLossDbH(_cable_losses_attr);
  cal->setPowerMeasLossDbV(_cable_losses_attr);
  double transmitPowerDbm = 10.0 * log10(_transmitPowerW * 1000.0);
  cal->setXmitPowerDbmH(transmitPowerDbm);
  cal->setXmitPowerDbmV(transmitPowerDbm);
  _readVol->addCalib(cal); 

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  for (int ii = 0; ii < (int) _raysValid.size(); ii++) {
    _raysValid[ii]->setVolumeNumber(_volumeNumber);
  }

  // add rays to vol - they will be freed by vol

  for (size_t ii = 0; ii < _raysValid.size(); ii++) {
    _readVol->addRay(_raysValid[ii]);
  }
  
  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _raysValid.clear();

  // set sweep mode from the ray angles

  Radx::SweepMode_t sweepMode = _readVol->getPredomSweepModeFromAngles();
  vector<RadxRay *> &rays = _readVol->getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    rays[ii]->setSweepMode(sweepMode);
  }
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();

  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - CfarrNcRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - CfarrNcRadxFile::_loadReadVolume");
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

  return 0;

}

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void CfarrNcRadxFile::_computeFixedAngles()
  
{

  for (size_t isweep = 0; isweep < _readVol->getNSweeps(); isweep++) {

    RadxSweep &sweep = *_readVol->getSweeps()[isweep];

    double sumAngle = 0.0;
    double count = 0.0;

    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      const RadxRay &ray = *_readVol->getRays()[iray];
      if (ray.getSweepMode() == Radx::SWEEP_MODE_RHI) {
	sumAngle += ray.getAzimuthDeg();
      } else {
	sumAngle += ray.getElevationDeg();
      }
      count++;
    }

    double meanAngle = sumAngle / count;
    double fixedAngle = ((int) (meanAngle * 100.0 + 0.5)) / 100.0;

    sweep.setFixedAngleDeg(fixedAngle);
      
    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      RadxRay &ray = *_readVol->getRays()[iray];
      ray.setFixedAngleDeg(fixedAngle);
    }

  } // isweep

  _readVol->loadFixedAnglesFromSweepsToRays();

}

