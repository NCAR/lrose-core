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
// DoeNcRadxFile.cc
//
// DoeNcRadxFile object
//
// NetCDF file data for radar radial data in early DOE Netcdf
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/DoeNcRadxFile.hh>
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
using namespace std;

//////////////
// Constructor

DoeNcRadxFile::DoeNcRadxFile() : RadxFile()
  
{

  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

DoeNcRadxFile::~DoeNcRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void DoeNcRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _timeDim = NULL;
  _rangeDim = NULL;
  _sweepDim = NULL;

  _timeVar = NULL;
  _rangeVar = NULL;
  _latitudeVar = NULL;
  _longitudeVar = NULL;
  _altitudeVar = NULL;
  _azimuthVar = NULL;
  _azimuthRateVar = NULL;
  _elevationVar = NULL;
  _elevationRateVar = NULL;
  _noiseVar = NULL;
  _pedestalOpModeVar = NULL;
  _polarizationVar = NULL;
  _sweepTypeVar = NULL;
  _sweepStartIndexVar = NULL;
  _sweepLengthVar = NULL;

  _command_line_attr.clear();
  _process_version_attr.clear();
  _ingest_software_attr.clear();
  _dod_version_attr.clear();
  _site_id_attr.clear();
  _facility_id_attr.clear();
  _data_level_attr.clear();
  _input_source_attr.clear();
  _resolution_description_attr.clear();
  _sample_int_attr.clear();
  _serial_number_attr.clear();
  _calibration_constant_attr.clear();
  _averaging_int_attr.clear();
  _pulse_length_attr.clear();
  _corner_power_attr.clear();
  _center_freq_attr.clear();
  _decimation_attr.clear();
  _drx_gcsync_attr.clear();
  _drx_ncosync_attr.clear();
  _drx_zerorx_attr.clear();
  _external_clock_attr.clear();
  _integration_attr.clear();
  _resolution_setpoint_attr.clear();
  _radar_operating_frequency_attr.clear();
  _radar_wavelength_attr.clear();
  _pedestal_true_north_attr.clear();
  _nyquist_velocity_attr.clear();
  _nearfield_correction_description_attr.clear();
  _NumSpectralAverages_attr.clear();
  _fftlen_attr.clear();
  _prf_attr.clear();
  _StartGateDelay_attr.clear();
  _GateSpacing_attr.clear();
  _pol_mode_attr.clear();
  _antenna_altitude_attr.clear();
  _antenna_diameter_attr.clear();
  _elevation_correction_attr.clear();
  _scan_mode_attr.clear();
  _comment_attr.clear();
  _polarization_state_attr.clear();
  _qc_standards_version_attr.clear();
  _qc_method_attr.clear();
  _qc_comment_attr.clear();
  _qc_bit_1_description_attr.clear();
  _qc_bit_1_assessment_attr.clear();
  _qc_bit_2_description_attr.clear();
  _qc_bit_2_assessment_attr.clear();
  _qc_bit_3_description_attr.clear();
  _qc_bit_3_assessment_attr.clear();
  _qc_bit_4_description_attr.clear();
  _qc_bit_4_assessment_attr.clear();
  _zeb_platform_attr.clear();
  _history_attr.clear();

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

  _rayTimesIncrease = true;
  _refTimeSecsFile = 0;

  _volumeNumber = 0;
  _instrumentType = Radx::missingInstrumentType;
  _platformType = Radx::missingPlatformType;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

  _latitudeDeg = 0.0;
  _longitudeDeg = 0.0;;
  _altitudeKm = 0.0;

  _rangeKm.clear();
  _gateSpacingIsConstant = true;

}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool DoeNcRadxFile::isSupported(const string &path)

{
  
  if (isDoeNc(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a DoeNc file
// Returns true on success, false on failure

bool DoeNcRadxFile::isDoeNc(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_verbose) {
      cerr << "DEBUG - not DoeNc file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not DoeNc file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check existence of some variables

  Nc3Var *baseTimeVar = _file.getNc3File()->get_var("base_time");
  if (baseTimeVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not DoeNc file" << endl;
      cerr << "  base_time variable missing" << endl;
    }
    return false;
  }

  Nc3Var *qcTimeVar = _file.getNc3File()->get_var("qc_time");
  if (qcTimeVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not DoeNc file" << endl;
      cerr << "  qc_time variable missing" << endl;
    }
    return false;
  }

  // file has the correct dimensions, so it is a DoeNc file

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

int DoeNcRadxFile::writeToDir(const RadxVol &vol,
                              const string &dir,
                              bool addDaySubDir,
                              bool addYearSubDir)
  
{

  // Writing DoeNc files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - DoeNcRadxFile::writeToDir" << endl;
  cerr << "  Writing DoeNc format files not supported" << endl;
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

int DoeNcRadxFile::writeToPath(const RadxVol &vol,
                               const string &path)
  
{

  // Writing DoeNc files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - DoeNcRadxFile::writeToPath" << endl;
  cerr << "  Writing DoeNc format files not supported" << endl;
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

int DoeNcRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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

void DoeNcRadxFile::print(ostream &out) const
  
{
  
  out << "=============== DoeNcRadxFile ===============" << endl;
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

int DoeNcRadxFile::printNative(const string &path, ostream &out,
                               bool printRays, bool printData)
  
{

  _addErrStr("ERROR - DoeNcRadxFile::printNative");
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

int DoeNcRadxFile::readFromPath(const string &path,
                                RadxVol &vol)
  
{

  _initForRead(path, vol);
  
  if (_debug) {
    cerr << "Reading path: " << path << endl;
  }

  string errStr("ERROR - DoeNcRadxFile::readFromPath");
  
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

  _fileFormat = FILE_FORMAT_DOE_NC;

  // clean up

  _clearRayVariables();
  _dTimes.clear();

  return 0;

}

///////////////////////////////////
// read in the dimensions

int DoeNcRadxFile::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  iret |= _file.readDim("time", _timeDim);
  if (iret == 0) {
    _nTimesInFile = _timeDim->size();
  }

  _nRangeInFile = 0;
  iret |= _file.readDim("range_gates", _rangeDim);
  if (iret == 0) {
    _nRangeInFile = _rangeDim->size();
  }
  
  // _nGatesVary = false;

  iret |= _file.readDim("sweep", _sweepDim);

  if (iret) {
    _addErrStr("ERROR - DoeNcRadxFile::_file.readDimensions");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read the global attributes

int DoeNcRadxFile::_readGlobalAttributes()

{

  _file.readGlobAttr("command_line", _command_line_attr);
  _file.readGlobAttr("process_version", _process_version_attr);
  _file.readGlobAttr("ingest_software", _ingest_software_attr);
  _file.readGlobAttr("dod_version", _dod_version_attr);
  _file.readGlobAttr("site_id", _site_id_attr);
  _file.readGlobAttr("facility_id", _facility_id_attr);
  _file.readGlobAttr("data_level", _data_level_attr);
  _file.readGlobAttr("input_source", _input_source_attr);
  _file.readGlobAttr("resolution_description", _resolution_description_attr);
  _file.readGlobAttr("sample_int", _sample_int_attr);
  _file.readGlobAttr("serial_number", _serial_number_attr);
  _file.readGlobAttr("calibration_constant", _calibration_constant_attr);
  _file.readGlobAttr("averaging_int", _averaging_int_attr);
  _file.readGlobAttr("pulse_length", _pulse_length_attr);
  _file.readGlobAttr("corner_power", _corner_power_attr);
  _file.readGlobAttr("center_freq", _center_freq_attr);
  _file.readGlobAttr("decimation", _decimation_attr);
  _file.readGlobAttr("drx_gcsync", _drx_gcsync_attr);
  _file.readGlobAttr("drx_ncosync", _drx_ncosync_attr);
  _file.readGlobAttr("drx_zerorx", _drx_zerorx_attr);
  _file.readGlobAttr("external_clock", _external_clock_attr);
  _file.readGlobAttr("integration", _integration_attr);
  _file.readGlobAttr("resolution_setpoint", _resolution_setpoint_attr);
  _file.readGlobAttr("radar_operating_frequency",
                     _radar_operating_frequency_attr);
  _file.readGlobAttr("radar_wavelength", _radar_wavelength_attr);
  _file.readGlobAttr("pedestal_true_north", _pedestal_true_north_attr);
  _file.readGlobAttr("nyquist_velocity", _nyquist_velocity_attr);
  _file.readGlobAttr("nearfield_correction_description",
                     _nearfield_correction_description_attr);
  _file.readGlobAttr("NumSpectralAverages", _NumSpectralAverages_attr);
  _file.readGlobAttr("fftlen", _fftlen_attr);
  _file.readGlobAttr("prf", _prf_attr);
  _file.readGlobAttr("StartGateDelay", _StartGateDelay_attr);
  _file.readGlobAttr("GateSpacing", _GateSpacing_attr);
  _file.readGlobAttr("pol_mode", _pol_mode_attr);
  _file.readGlobAttr("antenna_altitude", _antenna_altitude_attr);
  _file.readGlobAttr("antenna_diameter", _antenna_diameter_attr);
  _file.readGlobAttr("elevation_correction", _elevation_correction_attr);
  _file.readGlobAttr("scan_mode", _scan_mode_attr);
  _file.readGlobAttr("comment", _comment_attr);
  _file.readGlobAttr("polarization_state", _polarization_state_attr);
  _file.readGlobAttr("qc_standards_version", _qc_standards_version_attr);
  _file.readGlobAttr("qc_method", _qc_method_attr);
  _file.readGlobAttr("qc_comment", _qc_comment_attr);
  _file.readGlobAttr("qc_bit_1_description", _qc_bit_1_description_attr);
  _file.readGlobAttr("qc_bit_1_assessment", _qc_bit_1_assessment_attr);
  _file.readGlobAttr("qc_bit_2_description", _qc_bit_2_description_attr);
  _file.readGlobAttr("qc_bit_2_assessment", _qc_bit_2_assessment_attr);
  _file.readGlobAttr("qc_bit_3_description", _qc_bit_3_description_attr);
  _file.readGlobAttr("qc_bit_3_assessment", _qc_bit_3_assessment_attr);
  _file.readGlobAttr("qc_bit_4_description", _qc_bit_4_description_attr);
  _file.readGlobAttr("qc_bit_4_assessment", _qc_bit_4_assessment_attr);
  _file.readGlobAttr("zeb_platform", _zeb_platform_attr);
  _file.readGlobAttr("history", _history_attr);

  _title = _dod_version_attr;
  _institution = _facility_id_attr;
  _references = _command_line_attr;
  _source = _input_source_attr;
  _history = _history_attr;
  _comment = _comment_attr;
  _comment += _qc_comment_attr;

  _siteName = _site_id_attr;
  _scanName = _scan_mode_attr;

  _instrumentName = _dod_version_attr;
  
  double dval;
  _pulseLenNs = Radx::missingMetaDouble;
  if (sscanf(_pulse_length_attr.c_str(), "%lg", &dval) == 1) {
    _pulseLenNs = dval;
  }

  _frequencyGhz = Radx::missingMetaDouble;
  if (sscanf(_radar_operating_frequency_attr.c_str(), "%lg", &dval) == 1) {
    _frequencyGhz = dval;
  }

  _wavelengthM = Radx::missingMetaDouble;
  if (sscanf(_radar_wavelength_attr.c_str(), "%lg", &dval) == 1) {
    _wavelengthM = dval;
  }

  _nyquistMps = Radx::missingMetaDouble;
  if (sscanf(_nyquist_velocity_attr.c_str(), "%lg", &dval) == 1) {
    _nyquistMps = dval;
  }

  _prfHz = Radx::missingMetaDouble;
  if (sscanf(_prf_attr.c_str(), "%lg", &dval) == 1) {
    _prfHz = dval;
  }

  _antennaHtAgl = Radx::missingMetaDouble;
  if (sscanf(_antenna_altitude_attr.c_str(), "%lg", &dval) == 1) {
    _antennaHtAgl = dval;
  }

  _antennaDiameterInches = Radx::missingMetaDouble;
  if (sscanf(_antenna_diameter_attr.c_str(), "%lg", &dval) == 1) {
    _antennaDiameterInches = dval;
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

int DoeNcRadxFile::_readTimes()

{

  // read the time variable

  _timeVar = _file.getNc3File()->get_var("time");
  if (_timeVar == NULL) {
    _addErrStr("ERROR - DoeNcRadxFile::_readTimes");
    _addErrStr("  Cannot find time variable, name: ", "time");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  if (_timeVar->num_dims() < 1) {
    _addErrStr("ERROR - DoeNcRadxFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = _timeVar->get_dim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - DoeNcRadxFile::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim->name());
    return -1;
  }

  // get units attribute
  
  Nc3Att* unitsAtt = _timeVar->get_att("units");
  if (unitsAtt == NULL) {
    _addErrStr("ERROR - DoeNcRadxFile::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }
  string units = Nc3xFile::asString(unitsAtt);
  delete unitsAtt;

#ifdef NOTNOW

  // check if this is a time variable, using udunits
  
  ut_unit *udUnit = ut_parse(_udunits.getSystem(), units.c_str(), UT_ASCII);
  if (udUnit == NULL) {
    _addErrStr("ERROR - DoeNcRadxFile::_readTimes");
    _addErrStr("  Cannot parse time units: ", units);
    _addErrInt("  udunits status: ", ut_get_status());
    return -1;
  }
    
  if (ut_are_convertible(udUnit, _udunits.getEpoch()) == 0) {
    // not a time variable
    _addErrStr("ERROR - DoeNcRadxFile::_readTimes");
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
    _addErrStr("ERROR - DoeNcRadxFile::_readTimes");
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

int DoeNcRadxFile::_readRangeVariable()

{

  _rangeVar = _file.getNc3File()->get_var("range");
  if (_rangeVar == NULL || _rangeVar->num_vals() < 1) {
    _addErrStr("ERROR - DoeNcRadxFile::_readRangeVariable");
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

int DoeNcRadxFile::_readPositionVariables()

{

  // find latitude, longitude, altitude

  int iret = 0;
  if (_file.readDoubleVar(_latitudeVar, "lat", _latitudeDeg, 0, true)) {
    _addErrStr("ERROR - DoeNcRadxFile::_readPositionVariables");
    _addErrStr("  Cannot read latitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_longitudeVar, "lon", _longitudeDeg, 0, true)) {
    _addErrStr("ERROR - DoeNcRadxFile::_readPositionVariables");
    _addErrStr("  Cannot read longitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }

  if (_file.readDoubleVar(_altitudeVar, "alt", _altitudeKm, 0, true)) {
    _addErrStr("ERROR - DoeNcRadxFile::_readPositionVariables");
    _addErrStr("  Cannot read altitude");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }
  Nc3Att* unitsAtt = _altitudeVar->get_att("units");
  if (unitsAtt != NULL) {
    string units = Nc3xFile::asString(unitsAtt);
    if (units == "m") {
      _altitudeKm /= 1000.0;
    }
    delete unitsAtt;
  }
  
  return iret;

}

///////////////////////////////////
// read the sweep meta-data

int DoeNcRadxFile::_readSweepVariables()

{

  // create vector for the sweeps

  size_t nSweepsInFile = _sweepDim->size();

  // initialize

  vector<int> sweepTypes, sweepStartIndexes, sweepLengths;
  
  int iret = 0;
  
  _readSweepVar(_sweepTypeVar, "sweep_type", sweepTypes);
  if (sweepTypes.size() != nSweepsInFile) {
    iret = -1;
  }

  _readSweepVar(_sweepStartIndexVar, "sweep_start_index", sweepStartIndexes);
  if (sweepStartIndexes.size() != nSweepsInFile) {
    iret = -1;
  }

  _readSweepVar(_sweepLengthVar, "sweep_length", sweepLengths);
  if (sweepLengths.size() != nSweepsInFile) {
    iret = -1;
  }

  if (iret) {
    return -1;
  }

  _sweeps.clear();
  for (size_t ii = 0; ii < nSweepsInFile; ii++) {
    RadxSweep *sweep = new RadxSweep;
    sweep->setSweepNumber(ii);
    if (sweepTypes[ii] == 0) {
      sweep->setSweepMode(Radx::SWEEP_MODE_VERTICAL_POINTING);
    } else if (sweepTypes[ii] == 1) {
      sweep->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    } else if (sweepTypes[ii] == 2) {
      sweep->setSweepMode(Radx::SWEEP_MODE_RHI);
    } else if (sweepTypes[ii] == 3) {
      sweep->setSweepMode(Radx::SWEEP_MODE_COPLANE);
    }
    sweep->setStartRayIndex(sweepStartIndexes[ii]);
    int endIndex = sweepStartIndexes[ii] + sweepLengths[ii] - 1;
    sweep->setEndRayIndex(endIndex);
    _sweeps.push_back(sweep);
  } // ii

  return 0;

}

///////////////////////////////////
// clear the ray variables

void DoeNcRadxFile::_clearRayVariables()

{

  _azimuths.clear();
  _elevations.clear();
  _azScanRates.clear();
  _elScanRates.clear();
  _noiseDbms.clear();
  _pedestalOpModes.clear();
  _polarizations.clear();

}

///////////////////////////////////
// read in ray variables

int DoeNcRadxFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  _readRayVar(_azimuthVar, "Azimuth_current", _azimuths);
  if ((int) _azimuths.size() != _timeDim->size()) {
    _addErrStr("ERROR - Azimuth_current variable required");
    iret = -1;
  }
  
  _readRayVar(_elevationVar, "Elevation_current", _elevations);
  if ((int) _elevations.size() != _timeDim->size()) {
    _addErrStr("ERROR - Elevation_current variable required");
    iret = -1;
  }

  _readRayVar(_azimuthRateVar, "Azimuth_scan_rate", _azScanRates, false);
  _readRayVar(_elevationRateVar, "Elevation_scan_rate", _elScanRates, false);
  _readRayVar(_pedestalOpModeVar, "Pedestal_op_mode", _pedestalOpModes, false);
  _readRayVar(_noiseVar, "Noise", _noiseDbms, false);
  _readRayVar(_polarizationVar, "Polarization", _polarizations, false);

  if (iret) {
    _addErrStr("ERROR - DoeNcRadxFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int DoeNcRadxFile::_createRays(const string &path)

{

  // compile a list of the rays to be read in, using the list of
  // sweeps to read

  vector<RayInfo> raysToRead;
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    RadxSweep *sweep = _sweeps[isweep];
    for (size_t ii = sweep->getStartRayIndex();
         ii <= sweep->getEndRayIndex(); ii++) {
      // add ray to list to be read
      RayInfo info;
      info.indexInFile = ii;
      info.sweep = sweep;
      raysToRead.push_back(info);
    } // ii
  } // isweep

  // create the rays

  _raysToRead.clear();
  
  for (size_t ii = 0; ii < raysToRead.size(); ii++) {
    
    RayInfo rayInfo = raysToRead[ii];
    size_t rayIndex = rayInfo.indexInFile;
    RadxSweep *sweep = rayInfo.sweep;

    // new ray

    RadxRay *ray = new RadxRay;
    rayInfo.ray = ray;

    ray->copyRangeGeom(_geom);
    
    // set time
    
    double rayTimeDouble = _dTimes[rayIndex];
    time_t rayUtimeSecs = _refTimeSecsFile + (time_t) rayTimeDouble;
    double rayIntSecs;
    double rayFracSecs = modf(rayTimeDouble, &rayIntSecs);
    int rayNanoSecs = (int) (rayFracSecs * 1.0e9);
    ray->setTime(rayUtimeSecs, rayNanoSecs);
    
    // sweep info
    
    ray->setSweepNumber(sweep->getSweepNumber());
    ray->setAzimuthDeg(_azimuths[rayIndex]);
    ray->setElevationDeg(_elevations[rayIndex]);
    
    if (_pedestalOpModes.size() > rayIndex) {
      int opMode = _pedestalOpModes[rayIndex];
      Radx::SweepMode_t sweepMode = Radx::SWEEP_MODE_NOT_SET;
      switch (opMode) {
        case 2: sweepMode = Radx::SWEEP_MODE_IDLE; break;
        case 3: sweepMode = Radx::SWEEP_MODE_POINTING; break;
        case 7: sweepMode = Radx::SWEEP_MODE_RHI; break;
        case 8: sweepMode = Radx::SWEEP_MODE_SECTOR; break;
        case 9: sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE; break;
      }
      ray->setSweepMode(sweepMode);
    }
    
    if (_polarizations.size() > rayIndex) {
      // int pol = _polarizations[rayIndex];
      ray->setPolarizationMode(Radx::POL_MODE_HV_SIM);
    }
    
    if (_noiseDbms.size() > rayIndex) {
      double noiseDbm = _noiseDbms[rayIndex];
      ray->setEstimatedNoiseDbmHc(noiseDbm);
      ray->setEstimatedNoiseDbmVc(noiseDbm);
    }
    
    if (_azScanRates.size() > rayIndex) {
      ray->setTrueScanRateDegPerSec(_azScanRates[rayIndex]);
    }
    if (_elScanRates.size() > rayIndex) {
      if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI) {
        ray->setTrueScanRateDegPerSec(_elScanRates[rayIndex]);
      }
    }

    // add to ray vector

    _raysToRead.push_back(rayInfo);

  } // ii

  return 0;

}

////////////////////////////////////////////
// read the field variables

int DoeNcRadxFile::_readFieldVariables(bool metaOnly)

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
        cerr << "DEBUG - DoeNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
        cerr << "  -->> Should be float or double: " << fieldName << endl;
      }
      continue;
    }

    // check that we need this field

    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - DoeNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - DoeNcRadxFile::_readFieldVariables" << endl;
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
    if (name.find("DopplerVelocity") != string::npos) {
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
      default: {
        iret = -1;
        // will not reach here because of earlier check on type
      }

    } // switch
    
    if (iret) {
      _addErrStr("ERROR - DoeNcRadxFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  } // ivar

  return 0;

}

///////////////////////////////////
// read a ray variable - double

int DoeNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
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
      _addErrStr("ERROR - DoeNcRadxFile::_readRayVar");
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
      _addErrStr("ERROR - DoeNcRadxFile::_readRayVar");
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

int DoeNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
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
      _addErrStr("ERROR - DoeNcRadxFile::_readRayVar");
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
      _addErrStr("ERROR - DoeNcRadxFile::_readRayVar");
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

Nc3Var* DoeNcRadxFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - DoeNcRadxFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - DoeNcRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - DoeNcRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has incorrect dimension, dim name: ", 
                 timeDim->name());
      _addErrStr("  should be: ", "time");
    }
    return NULL;
  }

  return var;

}

///////////////////////////////////
// read a sweep variable - integer

int DoeNcRadxFile::_readSweepVar(Nc3Var* &var, const string &name,
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
      _addErrStr("ERROR - DoeNcRadxFile::_readSweepVar");
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
      _addErrStr("ERROR - DoeNcRadxFile::_readSweepVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// get a sweep variable
// returns NULL on failure

Nc3Var* DoeNcRadxFile::_getSweepVar(const string &name)

{
  
  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    _addErrStr("ERROR - DoeNcRadxFile::_getSweepVar");
    _addErrStr("  Cannot read variable, name: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return NULL;
  }

  // check sweep dimension

  if (var->num_dims() < 1) {
    _addErrStr("ERROR - DoeNcRadxFile::_getSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no dimensions");
    return NULL;
  }
  Nc3Dim *sweepDim = var->get_dim(0);
  if (sweepDim != _sweepDim) {
    _addErrStr("ERROR - DoeNcRadxFile::_getSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has incorrect dimension, dim name: ",
               sweepDim->name());
    _addErrStr("  should be: ", "sweep");
    return NULL;
  }

  return var;

}

//////////////////////////////////////////////////////////////
// Add fl64 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int DoeNcRadxFile::_addFl64FieldToRays(Nc3Var* var,
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

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - DoeNcRadxFile::_addSi16FieldToRays" << endl;
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

int DoeNcRadxFile::_addFl32FieldToRays(Nc3Var* var,
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
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - DoeNcRadxFile::_addSi16FieldToRays" << endl;
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

int DoeNcRadxFile::_loadReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("DOE");
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
  _readVol->setAltitudeKm(_altitudeKm);

  _readVol->copyRangeGeom(_geom);

  for (int ii = 0; ii < (int) _raysValid.size(); ii++) {
    _raysValid[ii]->setVolumeNumber(_volumeNumber);
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
      _addErrStr("ERROR - DoeNcRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - DoeNcRadxFile::_loadReadVolume");
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

void DoeNcRadxFile::_computeFixedAngles()
  
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

