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
// NoxpNcRadxFile.cc
//
// NoxpNcRadxFile object
//
// NetCDF file data for radar radial data from OU NOXP radar
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2015
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/NoxpNcRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxComplex.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
using namespace std;

//////////////
// Constructor

NoxpNcRadxFile::NoxpNcRadxFile() : RadxFile()
  
{

  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

NoxpNcRadxFile::~NoxpNcRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NoxpNcRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _timeDim = NULL;
  _gateDim = NULL;

  _timeVar = NULL;
  _rangeVar = NULL;
  _latitudeVar = NULL;
  _longitudeVar = NULL;
  _altitudeVar = NULL;
  _azimuthVar = NULL;
  _elevationVar = NULL;
  
  _source_attr.clear();
  _address_attr.clear();
  _email_attr.clear();
  _radar_name_attr.clear();
  _frequency_attr.clear();
  _prf_attr.clear();
  _nsamples_attr.clear();
  _scan_type_attr.clear();
  _date_processed_attr.clear();

  _dTimes.clear();
  _rayTimesIncrease = true;
  _nTimes = 0;
  _refTimeSecsFile = 0;

  _rangeKm.clear();
  _gateSpacingIsConstant = true;

  _latitudeDeg = 0.0;
  _longitudeDeg = 0.0;
  _altitudeKm = 0.0;

  _azimuths.clear();
  _elevations.clear();

  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();
  _statusXml.clear(); // global attributes

  _siteName.clear();
  _scanName.clear();
  _scanId = 0;
  _instrumentName.clear();

  _volumeNumber = 0;
  _instrumentType = Radx::missingInstrumentType;
  _platformType = Radx::missingPlatformType;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;
  
}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool NoxpNcRadxFile::isSupported(const string &path)

{
  
  if (isNoxpNc(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a NoxpNc file
// Returns true on success, false on failure

bool NoxpNcRadxFile::isNoxpNc(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_verbose) {
      cerr << "DEBUG - not NoxpNc file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NoxpNc file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check for radar name in global attributes

  string radarName;
  _file.readGlobAttr("Radar", radarName);
  if (radarName.find("NOXP") == string::npos) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NoxpNc file" << endl;
      cerr << "  Global attr Radar is not NOXP" << endl;
    }
    return false;
  }

  Nc3Var *azVar = _file.getNc3File()->get_var("AZ");
  if (azVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NoxpNc file" << endl;
      cerr << "  AZ variable missing" << endl;
    }
    return false;
  }

  Nc3Var *elVar = _file.getNc3File()->get_var("EL");
  if (elVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not NoxpNc file" << endl;
      cerr << "  EL variable missing" << endl;
    }
    return false;
  }

  // file is an NoxpNc file

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

int NoxpNcRadxFile::writeToDir(const RadxVol &vol,
                               const string &dir,
                               bool addDaySubDir,
                               bool addYearSubDir)
  
{

  // Writing NoxpNc files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - NoxpNcRadxFile::writeToDir" << endl;
  cerr << "  Writing NoxpNc format files not supported" << endl;
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

int NoxpNcRadxFile::writeToPath(const RadxVol &vol,
                                const string &path)
  
{

  // Writing NoxpNc files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - NoxpNcRadxFile::writeToPath" << endl;
  cerr << "  Writing NoxpNc format files not supported" << endl;
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

int NoxpNcRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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
    if (sscanf(start, "%2d%2d%4d%1c%2d%2d%2d",
               &month, &day, &year, &cc, &hour, &min, &sec) == 7) {
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

void NoxpNcRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NoxpNcRadxFile ===============" << endl;
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
  out << "  altitude: " << _altitudeKm << endl;
  out << "  frequencyHz: " << _frequencyHz << endl;
  
  out << "  startRangeKm: " << _remap.getStartRangeKm() << endl;
  out << "  gateSpacingKm: " << _remap.getGateSpacingKm() << endl;
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int NoxpNcRadxFile::printNative(const string &path, ostream &out,
                                bool printRays, bool printData)
  
{

  _addErrStr("ERROR - NoxpNcRadxFile::printNative");
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

int NoxpNcRadxFile::readFromPath(const string &path,
                                 RadxVol &vol)
  
{

  _initForRead(path, vol);
  
  if (_debug) {
    cerr << "Reading path: " << path << endl;
  }

  _addErrStr("ERROR - NoxpNcRadxFile::readFromPath");
  _addErrStr("  Path: ", path);
    
  // clear tmp rays
  
  _nTimes = 0;
  _rays.clear();

  // open file

  if (_file.openRead(path)) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    return -1;
  }

  // read time variable now if that is all that is needed
  
  if (_readTimesOnly) {
    if (_readTimes()) {
      return -1;
    }
    return 0;
  }
  
  // read global attributes
  
  if (_readGlobalAttributes()) {
    return -1;
  }
  
  // read time variable
  
  if (_readTimes()) {
    return -1;
  }

  // read position variables - lat/lon/alt
  
  if (_readPositionVariables()) {
    return -1;
  }

  // read in ray variables

  if (_readRayVariables()) {
    return -1;
  }

  if (_readMetadataOnly) {

    // read field variables
    
    if (_readFieldVariables(true)) {
      return -1;
    }
    
  } else {

    // create the rays to be read in, filling out the metadata
    
    if (_createRays(path)) {
      return -1;
    }
    
    // add field variables to file rays
    
    if (_readFieldVariables(false)) {
      return -1;
    }

  }

  // close file

  _file.close();

  // append to read paths
  
  _readPaths.push_back(path);

  // check we have at least 1 ray

  if (_rays.size() < 1) {
    _addErrStr("  No rays found");
    return -1;
  }
  
  // set range geometry

  if (_setRangeGeometry()) {
    return -1;
  }
  
  // load the data into the read volume

  if (_loadReadVolume()) {
    return -1;
  }
  
  // compute fixed angles as mean angle from sweeps
  
  _computeFixedAngles();
  
  // set format as read

  _fileFormat = FILE_FORMAT_NOXP_NC;

  // clean up

  _clearRayVariables();
  _dTimes.clear();

  return 0;

}

///////////////////////////////////
// read in the dimensions

int NoxpNcRadxFile::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  iret |= _file.readDim("Time", _timeDim);
  if (iret == 0) {
    _nTimes = _timeDim->size();
  }

  _nGates = 0;
  iret |= _file.readDim("Gate", _gateDim);
  if (iret == 0) {
    _nGates = _gateDim->size();
  }
  
  _nPositions = 0;
  iret |= _file.readDim("Position", _positionDim);
  if (iret == 0) {
    _nPositions = _positionDim->size();
  }
  
  if (iret) {
    _addErrStr("ERROR - NoxpNcRadxFile::_file.readDimensions");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read the global attributes

int NoxpNcRadxFile::_readGlobalAttributes()

{

  _file.readGlobAttr("Source", _source_attr);
  _file.readGlobAttr("Address", _address_attr);
  _file.readGlobAttr("e-mail", _email_attr);
  _file.readGlobAttr("Radar", _radar_name_attr);
  _file.readGlobAttr("Frequency", _frequency_attr);
  _file.readGlobAttr("PRF", _prf_attr);
  _file.readGlobAttr("Hits", _nsamples_attr);
  _file.readGlobAttr("Scan type", _scan_type_attr);
  _file.readGlobAttr("Date processed", _date_processed_attr);

  _title = "NOXP radar data";
  _institution = _source_attr;
  _references = _email_attr;
  _source = _address_attr;
  _history = _date_processed_attr;
  _comment = "";
  
  if (_scan_type_attr.find("RHI") != string::npos) {
    _sweepMode = Radx::SWEEP_MODE_RHI;
  } else if (_scan_type_attr.find("PPI") != string::npos) {
    _sweepMode = Radx::SWEEP_MODE_SECTOR;
  } else {
    _sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }

  _instrumentName = _radar_name_attr;
  _siteName = "unknown";
  _scanName = _scan_type_attr;

  double dval;
  _frequencyHz = Radx::missingMetaDouble;
  if (sscanf(_frequency_attr.c_str(), "%lg", &dval) == 1) {
    _frequencyHz = dval * 1.0e9;
  }
  
  _prfHz = Radx::missingMetaDouble;
  _prtSec = Radx::missingMetaDouble;
  if (sscanf(_prf_attr.c_str(), "%lg", &dval) == 1) {
    _prfHz = dval;
    _prtSec = 1.0 / _prfHz;
  }

  _nyquistMps = Radx::missingMetaDouble;
  if (_frequencyHz != Radx::missingMetaDouble &&
      _prtSec != Radx::missingMetaDouble) {
    double wavelengthM = Radx::LIGHT_SPEED / _frequencyHz;
    _nyquistMps = wavelengthM / (4.0 * _prtSec);
  }

  _nSamples = Radx::missingMetaInt;
  int ival;
  if (sscanf(_nsamples_attr.c_str(), "%d", &ival) == 1) {
    _nSamples = ival;
  }

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

int NoxpNcRadxFile::_readTimes()

{

  // read the time variable

  _timeVar = _file.getNc3File()->get_var("time");
  if (_timeVar == NULL) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readTimes");
    _addErrStr("  Cannot find time variable, name: ", "time");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  if (_timeVar->num_dims() < 1) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = _timeVar->get_dim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim->name());
    return -1;
  }

  // get units attribute
  
  Nc3Att* unitsAtt = _timeVar->get_att("units");
  if (unitsAtt == NULL) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }
  string units = Nc3xFile::asString(unitsAtt);
  delete unitsAtt;

#ifdef NOTNOW

  // check if this is a time variable, using udunits
  
  ut_unit *udUnit = ut_parse(_udunits.getSystem(), units.c_str(), UT_ASCII);
  if (udUnit == NULL) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readTimes");
    _addErrStr("  Cannot parse time units: ", units);
    _addErrInt("  udunits status: ", ut_get_status());
    return -1;
  }
    
  if (ut_are_convertible(udUnit, _udunits.getEpoch()) == 0) {
    // not a time variable
    _addErrStr("ERROR - NoxpNcRadxFile::_readTimes");
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
  double *dtimes = dtimes_.alloc(_nTimes);
  if (_timeVar->get(dtimes, _nTimes) == 0) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readTimes");
    _addErrStr("  Cannot read times variable");
    return -1;
  }
  _dTimes.clear();
  for (size_t ii = 0; ii < _nTimes; ii++) {
    _dTimes.push_back(dtimes[ii]);
  }

  return 0;

}

///////////////////////////////////
// set the range geometry

int NoxpNcRadxFile::_setRangeGeometry()
  
{

  // get first ray - we know we have at least 1 ray

  const RadxRay *ray0 = _rays[0];
  
  // get Range field

  if (ray0->getField("Range") == NULL) {
    _addErrStr("ERROR - NoxpNcRadxFile::_setRangeGeometry");
    _addErrStr("  No 'Range' variable in file");
    return -1;
  }
  RadxField rangeField(*ray0->getField("Range"));

  // convert to doubles

  rangeField.convertToFl64();

  // get units

  double kmPerUnit = 1.0; // default - units in km
  string units = rangeField.getUnits();
  if (units.find("km") != string::npos) {
    kmPerUnit = 1.0;
  } else if (units.find("m") != string::npos) {
    kmPerUnit = 0.001;
  }
  
  // set range vector
  
  const Radx::fl64 *ranges = rangeField.getDataFl64();
  _rangeKm.clear();
  for (size_t igate = 0; igate < _nGates; igate++) {
    _rangeKm.push_back(ranges[igate] * kmPerUnit);
  }

  // compute gate spacing - the range info is a bit weird because it
  // is rounded to 10 m accuracy, but gate spacing may be something like 75

  double startRangeKm = 0.0;
  double endRangeKm = _rangeKm[_nGates - 1];
  double gateSpacingKm = (endRangeKm - startRangeKm) / (_nGates - 1);
  
  // set geom
  
  _gateSpacingIsConstant = true;
  _geom.setRangeGeom(startRangeKm, gateSpacingKm);

  // copy geom to rays

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    ray->copyRangeGeom(_geom);
    for (size_t ifield = 0; ifield < ray->getNFields(); ifield++) {
      RadxField *field = ray->getField(ifield);
      field->copyRangeGeom(_geom);
    }
  }  
  return 0;

}

///////////////////////////////////
// read the position variables

int NoxpNcRadxFile::_readPositionVariables()

{

  // find latitude, longitude, altitude

  int iret = 0;
  if (_file.readDoubleVar(_latitudeVar, "Latitude", _latitudeDeg, 0, true)) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readPositionVariables");
    _addErrStr("  Cannot read latitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_longitudeVar, "Longitude", _longitudeDeg, 0, true)) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readPositionVariables");
    _addErrStr("  Cannot read longitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_altitudeVar, "Altitude", _altitudeKm, 0, true)) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readPositionVariables");
    _addErrStr("  Cannot read altitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }
  Nc3Att* unitsAtt = _altitudeVar->get_att("units");
  if (unitsAtt != NULL) {
    string units = Nc3xFile::asString(unitsAtt);
    if (units.find("meters") != string::npos) {
      _altitudeKm /= 1000.0;
    }
    delete unitsAtt;
  }
  
  return iret;

}

///////////////////////////////////
// clear the ray variables

void NoxpNcRadxFile::_clearRayVariables()

{

  _azimuths.clear();
  _elevations.clear();

}

///////////////////////////////////
// read in ray variables

int NoxpNcRadxFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  _readRayVar(_azimuthVar, "AZ", _azimuths);
  if ((int) _azimuths.size() != _timeDim->size()) {
    _addErrStr("ERROR - AZ variable required");
    iret = -1;
  }
  
  _readRayVar(_elevationVar, "EL", _elevations);
  if ((int) _elevations.size() != _timeDim->size()) {
    _addErrStr("ERROR - EL variable required");
    iret = -1;
  }

  if (iret) {
    _addErrStr("ERROR - NoxpNcRadxFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int NoxpNcRadxFile::_createRays(const string &path)

{

  // create the rays
  
  _rays.clear();
  RadxTime zeroTime(0, 1, 1, 0, 0, 0);
  RadxTime jan1970(1970, 1, 0, 0, 0, 0);
  double jan1970Secs = jan1970 - zeroTime;
  double jan1970Days = jan1970Secs / 86400.0;

  for (size_t iray = 0; iray < _nTimes; iray++) {
    
    // new ray
    
    RadxRay *ray = new RadxRay;
    
    // set time
    
    double daysSinceZero = _dTimes[iray];
    double daysSinceJan1970 = daysSinceZero - jan1970Days;
    double dusecs = daysSinceJan1970 * 86400.0;
    time_t usecs = (time_t) dusecs;
    double subsecs = dusecs - usecs;
    int nanoSecs = (int) (subsecs * 1.0e9);
    ray->setTime(usecs, nanoSecs);

    // sweep info
    
    ray->setSweepNumber(0);
    ray->setAzimuthDeg(_azimuths[iray]);
    ray->setElevationDeg(_elevations[iray]);
    ray->setSweepMode(_sweepMode);
    
    // add to ray vector

    _rays.push_back(ray);

  } // ii

  return 0;

}

////////////////////////////////////////////
// read the field variables

int NoxpNcRadxFile::_readFieldVariables(bool metaOnly)

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
    Nc3Dim* rangeDim = var->get_dim(0);
    Nc3Dim* timeDim = var->get_dim(1);
    if (timeDim != _timeDim || rangeDim != _gateDim) {
      continue;
    }
    
    // check the type
    string fieldName = var->name();
    Nc3Type ftype = var->type();
    if (ftype != nc3Double && ftype != nc3Float && ftype != nc3Int && ftype != nc3Short) {
      // not a valid type
      if (_verbose) {
        cerr << "DEBUG - NoxpNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
        cerr << "  -->> Should be short, int, float or double: " << fieldName << endl;
      }
      continue;
    }

    // check that we need this field

    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - NoxpNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - NoxpNcRadxFile::_readFieldVariables" << endl;
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
    Nc3Att *unitsAtt = var->get_att("units");
    if (unitsAtt != NULL) {
      units = Nc3xFile::asString(unitsAtt);
      delete unitsAtt;
    }

    // folding

    bool fieldFolds = false;
    float foldLimitLower = Radx::missingMetaFloat;
    float foldLimitUpper = Radx::missingMetaFloat;
    if (name.find("VR") != string::npos ||
        name.find("VEL") != string::npos) {
      fieldFolds = true;
      foldLimitLower = _nyquistMps * -1.0;
      foldLimitUpper = _nyquistMps;
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
      case nc3Int: {
        if (_addSi32FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case nc3Short: {
        if (_addSi16FieldToRays(var, name, units, standardName, longName,
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
      _addErrStr("ERROR - NoxpNcRadxFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }
    
  } // ivar
  
  return 0;

}

///////////////////////////////////
// read a ray variable - double

int NoxpNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                vector<double> &vals, bool required)
  
{

  vals.clear();

  // get var

  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NoxpNcRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  double *data = new double[_nTimes];
  double *dd = data;
  int iret = 0;
  if (var->get(data, _nTimes)) {
    for (size_t ii = 0; ii < _nTimes; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - NoxpNcRadxFile::_readRayVar");
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

int NoxpNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                vector<int> &vals, bool required)

{

  vals.clear();

  // get var
  
  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NoxpNcRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  int *data = new int[_nTimes];
  int *dd = data;
  int iret = 0;
  if (var->get(data, _nTimes)) {
    for (size_t ii = 0; ii < _nTimes; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - NoxpNcRadxFile::_readRayVar");
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

Nc3Var* NoxpNcRadxFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - NoxpNcRadxFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - NoxpNcRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - NoxpNcRadxFile::_getRayVar");
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
// Add fl64 fields to _rays
// Returns 0 on success, -1 on failure

int NoxpNcRadxFile::_addFl64FieldToRays(Nc3Var* var,
                                        const string &name,
                                        const string &units,
                                        const string &standardName,
                                        const string &longName,
                                        bool isDiscrete,
                                        bool fieldFolds,
                                        float foldLimitLower,
                                        float foldLimitUpper)
  
{

  // allocate arrays

  RadxArray<Radx::fl64> data_, rayData_;
  Radx::fl64 *data = data_.alloc(_nGates * _nTimes);
  Radx::fl64 *rayData = rayData_.alloc(_nGates);

  // get data from array

  int iret = !var->get(data, _nGates, _nTimes);
  if (iret) {
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
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    int index = iray;
    for (size_t igate = 0; igate < _nGates; igate++, index += _nTimes) {
      rayData[igate] = data[index];
    }
    
    RadxField *field =
      _rays[iray]->addField(name, units, _nGates,
                            missingVal,
                            rayData,
                            true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }

  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add fl32 fields to _rays
// Returns 0 on success, -1 on failure

int NoxpNcRadxFile::_addFl32FieldToRays(Nc3Var* var,
                                        const string &name,
                                        const string &units,
                                        const string &standardName,
                                        const string &longName,
                                        bool isDiscrete,
                                        bool fieldFolds,
                                        float foldLimitLower,
                                        float foldLimitUpper)
  
{

  // allocate arrays

  RadxArray<Radx::fl32> data_, rayData_;
  Radx::fl32 *data = data_.alloc(_nGates * _nTimes);
  Radx::fl32 *rayData = rayData_.alloc(_nGates);

  // get data from array

  int iret = !var->get(data, _nGates, _nTimes);
  if (iret) {
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

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    int index = iray;
    for (size_t igate = 0; igate < _nGates; igate++, index += _nTimes) {
      rayData[igate] = data[index];
    }
    
    RadxField *field =
      _rays[iray]->addField(name, units, _nGates,
                            missingVal,
                            rayData,
                            true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si32 fields to _rays
// Returns 0 on success, -1 on failure

int NoxpNcRadxFile::_addSi32FieldToRays(Nc3Var* var,
                                        const string &name,
                                        const string &units,
                                        const string &standardName,
                                        const string &longName,
                                        bool isDiscrete,
                                        bool fieldFolds,
                                        float foldLimitLower,
                                        float foldLimitUpper)
  
{

  // allocate arrays

  RadxArray<Radx::si32> data_, rayData_;
  Radx::si32 *data = data_.alloc(_nGates * _nTimes);
  Radx::si32 *rayData = rayData_.alloc(_nGates);

  // get data from array

  int iret = !var->get(data, _nGates, _nTimes);
  if (iret) {
    return -1;
  }

  // set missing value, scale factor and add offset

  Radx::si32 missingVal = Radx::missingSi32;
  Nc3Att *missingValueAtt = var->get_att("missing_value");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }

  double scaleFactor = 1.0;
  Nc3Att *scaleFactorAtt = var->get_att("scale_factor");
  if (scaleFactorAtt != NULL) {
    scaleFactor = scaleFactorAtt->as_double(0);
    delete scaleFactorAtt;
  }
  
  double addOffset = 0.0;
  Nc3Att *addOffsetAtt = var->get_att("add_offset");
  if (addOffsetAtt != NULL) {
    addOffset = addOffsetAtt->as_double(0);
    delete addOffsetAtt;
  }
  
  // load field on rays

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    int index = iray;
    for (size_t igate = 0; igate < _nGates; igate++, index += _nTimes) {
      rayData[igate] = data[index];
    }

    RadxField *field =
      _rays[iray]->addField(name, units, _nGates,
                            missingVal,
                            rayData,
                            scaleFactor,
                            addOffset,
                            true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si16 fields to _rays
// Returns 0 on success, -1 on failure

int NoxpNcRadxFile::_addSi16FieldToRays(Nc3Var* var,
                                        const string &name,
                                        const string &units,
                                        const string &standardName,
                                        const string &longName,
                                        bool isDiscrete,
                                        bool fieldFolds,
                                        float foldLimitLower,
                                        float foldLimitUpper)
  
{

  // allocate arrays

  RadxArray<Radx::si16> data_, rayData_;
  Radx::si16 *data = data_.alloc(_nGates * _nTimes);
  Radx::si16 *rayData = rayData_.alloc(_nGates);

  // get data from array

  int iret = !var->get(data, _nGates, _nTimes);
  if (iret) {
    return -1;
  }

  // set missing value, scale factor and add offset

  Radx::si16 missingVal = Radx::missingSi16;
  Nc3Att *missingValueAtt = var->get_att("missing_value");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }

  double scaleFactor = 1.0;
  Nc3Att *scaleFactorAtt = var->get_att("scale_factor");
  if (scaleFactorAtt != NULL) {
    scaleFactor = scaleFactorAtt->as_double(0);
    delete scaleFactorAtt;
  }
  
  double addOffset = 0.0;
  Nc3Att *addOffsetAtt = var->get_att("add_offset");
  if (addOffsetAtt != NULL) {
    addOffset = addOffsetAtt->as_double(0);
    delete addOffsetAtt;
  }
  
  // load field on rays

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    int index = iray;
    for (size_t igate = 0; igate < _nGates; igate++, index += _nTimes) {
      rayData[igate] = data[index];
    }

    RadxField *field =
      _rays[iray]->addField(name, units, _nGates,
                            missingVal,
                            rayData,
                            scaleFactor,
                            addOffset,
                            true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  return 0;
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int NoxpNcRadxFile::_loadReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("DOE");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  _readVol->addFrequencyHz(_frequencyHz);

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
  _readVol->setSensorHtAglM(3.0);

  _readVol->setRadarBeamWidthDegH(0.9);
  _readVol->setRadarBeamWidthDegV(0.9);

  _readVol->setRadarAntennaGainDbH(44.5);
  _readVol->setRadarAntennaGainDbV(44.5);

  _readVol->copyRangeGeom(_geom);

  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    _rays[ii]->setVolumeNumber(_volumeNumber);
  }

  // add rays to vol - they will be freed by vol

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _readVol->addRay(_rays[ii]);
  }

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _rays.clear();
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();

  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - NoxpNcRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - NoxpNcRadxFile::_loadReadVolume");
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

void NoxpNcRadxFile::_computeFixedAngles()
  
{

  for (size_t isweep = 0; isweep < _readVol->getNSweeps(); isweep++) {

    RadxSweep &sweep = *_readVol->getSweeps()[isweep];

    double sumU = 0.0;
    double sumV = 0.0;
    
    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      const RadxRay &ray = *_readVol->getRays()[iray];
      double angle = ray.getElevationDeg();
      if (_sweepMode == Radx::SWEEP_MODE_RHI) {
        angle = ray.getAzimuthDeg();
      }
      double sinVal, cosVal;
      RadxComplex::sinCos(angle * Radx::DegToRad, sinVal, cosVal);
      sumU += cosVal;
      sumV += sinVal;
    }
    
    double meanAngle = atan2(sumV, sumU) * Radx::RadToDeg;
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

