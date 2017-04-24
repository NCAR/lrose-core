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
// NcxxRadxFile.cc
//
// NcxxRadxFile object
//
// NetCDF file data for radar radial data in CF-compliant file
//
// See also NcxxRadxFile_read.cc and NcxxRadxFile_write.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2016
//
///////////////////////////////////////////////////////////////

#include <Radx/NcxxRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

//////////////
// Constructor

NcxxRadxFile::NcxxRadxFile() : RadxFile()
  
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

NcxxRadxFile::~NcxxRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NcxxRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  // _timeDim = NULL;
  // _rangeDim = NULL;
  // _nPointsDim = NULL;
  // _sweepDim = NULL;
  // _calDim = NULL;
  // _stringLen8Dim = NULL;
  // _stringLen32Dim = NULL;
  // _frequencyDim = NULL;
  
  // _volumeNumberVar = NULL;
  // _instrumentTypeVar = NULL;
  // _platformTypeVar = NULL;
  // _primaryAxisVar = NULL;
  // _startTimeVar = NULL;
  // _endTimeVar = NULL;

  // _frequencyVar = NULL;

  // _radarAntennaGainHVar = NULL;
  // _radarAntennaGainVVar = NULL;
  // _radarBeamWidthHVar = NULL;
  // _radarBeamWidthVVar = NULL;
  // _radarRxBandwidthVar = NULL;

  // _lidarConstantVar = NULL;
  // _lidarPulseEnergyJVar = NULL;
  // _lidarPeakPowerWVar = NULL;
  // _lidarApertureDiamCmVar = NULL;
  // _lidarApertureEfficiencyVar = NULL;
  // _lidarFieldOfViewMradVar = NULL;
  // _lidarBeamDivergenceMradVar = NULL;

  // _timeVar = NULL;
  // _rangeVar = NULL;
  // _rayNGatesVar = NULL;
  // _rayStartIndexVar = NULL;
  // _rayStartRangeVar = NULL;
  // _rayGateSpacingVar = NULL;

  // _projVar = NULL;
  // _latitudeVar = NULL;
  // _longitudeVar = NULL;
  // _altitudeVar = NULL;

  // _sweepNumberVar = NULL;
  // _sweepModeVar = NULL;
  // _polModeVar = NULL;
  // _prtModeVar = NULL;
  // _sweepFollowModeVar = NULL;
  // _sweepFixedAngleVar = NULL;
  // _fixedAnglesFound = false;
  // _targetScanRateVar = NULL;
  // _sweepStartRayIndexVar = NULL;
  // _sweepEndRayIndexVar = NULL;
  // _raysAreIndexedVar = NULL;
  // _rayAngleResVar = NULL;
  // _intermedFreqHzVar = NULL;

  // _rCalTimeVar = NULL;
  // _rCalPulseWidthVar = NULL;
  // _rCalXmitPowerHVar = NULL;
  // _rCalXmitPowerVVar = NULL;
  // _rCalTwoWayWaveguideLossHVar = NULL;
  // _rCalTwoWayWaveguideLossVVar = NULL;
  // _rCalTwoWayRadomeLossHVar = NULL;
  // _rCalTwoWayRadomeLossVVar = NULL;
  // _rCalReceiverMismatchLossVar = NULL;
  // _rCalRadarConstHVar = NULL;
  // _rCalRadarConstVVar = NULL;
  // _rCalAntennaGainHVar = NULL;
  // _rCalAntennaGainVVar = NULL;
  // _rCalNoiseHcVar = NULL;
  // _rCalNoiseHxVar = NULL;
  // _rCalNoiseVcVar = NULL;
  // _rCalNoiseVxVar = NULL;
  // _rCalReceiverGainHcVar = NULL;
  // _rCalReceiverGainHxVar = NULL;
  // _rCalReceiverGainVcVar = NULL;
  // _rCalReceiverGainVxVar = NULL;
  // _rCalBaseDbz1kmHcVar = NULL;
  // _rCalBaseDbz1kmHxVar = NULL;
  // _rCalBaseDbz1kmVcVar = NULL;
  // _rCalBaseDbz1kmVxVar = NULL;
  // _rCalSunPowerHcVar = NULL;
  // _rCalSunPowerHxVar = NULL;
  // _rCalSunPowerVcVar = NULL;
  // _rCalSunPowerVxVar = NULL;
  // _rCalNoiseSourcePowerHVar = NULL;
  // _rCalNoiseSourcePowerVVar = NULL;
  // _rCalPowerMeasLossHVar = NULL;
  // _rCalPowerMeasLossVVar = NULL;
  // _rCalCouplerForwardLossHVar = NULL;
  // _rCalCouplerForwardLossVVar = NULL;
  // _rCalDbzCorrectionVar = NULL;
  // _rCalZdrCorrectionVar = NULL;
  // _rCalLdrCorrectionHVar = NULL;
  // _rCalLdrCorrectionVVar = NULL;
  // _rCalSystemPhidpVar = NULL;
  // _rCalTestPowerHVar = NULL;
  // _rCalTestPowerVVar = NULL;
  // _rCalReceiverSlopeHcVar = NULL;
  // _rCalReceiverSlopeHxVar = NULL;
  // _rCalReceiverSlopeVcVar = NULL;
  // _rCalReceiverSlopeVxVar = NULL;

  // _azimuthVar = NULL;
  // _elevationVar = NULL;
  // _pulseWidthVar = NULL;
  // _prtVar = NULL;
  // _nyquistVar = NULL;
  // _unambigRangeVar = NULL;
  // _antennaTransitionVar = NULL;
  // _georefsAppliedVar = NULL;
  // _nSamplesVar = NULL;
  // _calIndexVar = NULL;
  // _xmitPowerHVar = NULL;
  // _xmitPowerVVar = NULL;
  // _scanRateVar = NULL;

  // _estNoiseDbmHcVar = NULL;
  // _estNoiseDbmVcVar = NULL;
  // _estNoiseDbmHxVar = NULL;
  // _estNoiseDbmVxVar = NULL;

  _estNoiseAvailHc = false;
  _estNoiseAvailVc = false;
  _estNoiseAvailHx = false;
  _estNoiseAvailVx = false;

  _georefsActive = false;
  // _altitudeAglVar = NULL;

  _correctionsActive = false;
  // _azimuthCorrVar = NULL;
  // _elevationCorrVar = NULL;
  // _rangeCorrVar = NULL;
  // _longitudeCorrVar = NULL;
  // _latitudeCorrVar = NULL;
  // _pressureAltCorrVar = NULL;
  // _altitudeCorrVar = NULL;
  // _ewVelCorrVar = NULL;
  // _nsVelCorrVar = NULL;
  // _vertVelCorrVar = NULL;
  // _headingCorrVar = NULL;
  // _rollCorrVar = NULL;
  // _pitchCorrVar = NULL;
  // _driftCorrVar = NULL;
  // _rotationCorrVar = NULL;
  // _tiltCorrVar = NULL;

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

void NcxxRadxFile::_clearRays()
{
  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    delete _raysVol[ii];
  }
  _raysVol.clear();
}

void NcxxRadxFile::_clearSweeps()
{
  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    delete _sweeps[ii];
  }
  _sweeps.clear();
}

void NcxxRadxFile::_clearCals()
{
  for (int ii = 0; ii < (int) _rCals.size(); ii++) {
    delete _rCals[ii];
  }
  _rCals.clear();
}

void NcxxRadxFile::_clearFields()
{
  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool NcxxRadxFile::isSupported(const string &path)

{
  
  if (isCfRadial(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a CfRadial file
// Returns true on success, false on failure

bool NcxxRadxFile::isCfRadial(const string &path)
  
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

int NcxxRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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

NcxxType NcxxRadxFile::_getNcxxType(Radx::DataType_t dtype)

{
  switch (dtype) {
    case Radx::FL64:
      return ncxxDouble;
    case Radx::FL32:
      return ncxxFloat;
    case Radx::SI32:
      return ncxxInt;
    case Radx::SI16:
      return ncxxShort;
    case Radx::SI08:
    default:
      return ncxxByte;
  }
}

//////////////////////////////////////////////////////////
// convert RadxFile::netcdf_format_t to NcFile::FileFormat

NcxxFile::FileFormat 
  NcxxRadxFile::_getFileFormat(RadxFile::netcdf_format_t format)

{
  switch (format) {
    case NETCDF4_CLASSIC:
      return NcxxFile::nc4classic;
      break;
    case NETCDF_OFFSET_64BIT:
      return NcxxFile::classic64;
      break;
    case NETCDF4:
      return NcxxFile::nc4;
      break;
    case NETCDF_CLASSIC:
    default:
      return NcxxFile::classic;
  }
}

/////////////////////////////////////////////////////////
// print summary after read

void NcxxRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NcxxRadxFile ===============" << endl;
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

int NcxxRadxFile::printNative(const string &path, ostream &out,
                              bool printRays, bool printData)
  
{

  _addErrStr("ERROR - NcxxRadxFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

//////////////////////////////////////////////////////////////////
// interpret float and double vals, with respect to missing vals

Radx::fl64 NcxxRadxFile::_checkMissingDouble(double val)

{
  if (fabs(val - Radx::missingMetaDouble) < 0.0001) {
    return Radx::missingMetaDouble;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaDouble;
  }
  return val;
}

Radx::fl32 NcxxRadxFile::_checkMissingFloat(float val)

{

  if (fabs(val - Radx::missingMetaFloat) < 0.0001) {
    return Radx::missingMetaFloat;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaFloat;
  }
  return val;

}


/////////////////////////////////////////////////////////////////////////////////////
// string constant instantiation

const string NcxxRadxFile::CfConvention = "CF-1.6";
const string NcxxRadxFile::BaseConvention = "CF-Radial";
const string NcxxRadxFile::CurrentVersion = "CF-Radial-1.3";

const char* NcxxRadxFile::ADD_OFFSET = "add_offset";
const char* NcxxRadxFile::AIRBORNE = "airborne";
const char* NcxxRadxFile::ALTITUDE = "altitude";
const char* NcxxRadxFile::ALTITUDE_AGL = "altitude_agl";
const char* NcxxRadxFile::ALTITUDE_CORRECTION = "altitude_correction";
const char* NcxxRadxFile::ALTITUDE_OF_PROJECTION_ORIGIN = "altitude_of_projection_origin";
const char* NcxxRadxFile::ANCILLARY_VARIABLES = "ancillary_variables";
const char* NcxxRadxFile::ANTENNA_TRANSITION = "antenna_transition";
const char* NcxxRadxFile::AUTHOR = "author";
const char* NcxxRadxFile::AXIS = "axis";
const char* NcxxRadxFile::AZIMUTH = "azimuth";
const char* NcxxRadxFile::AZIMUTH_CORRECTION = "azimuth_correction";
const char* NcxxRadxFile::BLOCK_AVG_LENGTH = "block_avg_length";
const char* NcxxRadxFile::CALENDAR = "calendar";
const char* NcxxRadxFile::CFRADIAL = "cfradial";
const char* NcxxRadxFile::CM = "cm";
const char* NcxxRadxFile::COMMENT = "comment";
const char* NcxxRadxFile::COMPRESS = "compress";
const char* NcxxRadxFile::CONVENTIONS = "Conventions";
const char* NcxxRadxFile::COORDINATES = "coordinates";
const char* NcxxRadxFile::CREATED = "created";
const char* NcxxRadxFile::DB = "db";
const char* NcxxRadxFile::DBM = "dBm";
const char* NcxxRadxFile::DBZ = "dBZ";
const char* NcxxRadxFile::DEGREES = "degrees";
const char* NcxxRadxFile::DEGREES_EAST = "degrees_east";
const char* NcxxRadxFile::DEGREES_NORTH = "degrees_north";
const char* NcxxRadxFile::DEGREES_PER_SECOND = "degrees per second";
const char* NcxxRadxFile::DORADE = "dorade";
const char* NcxxRadxFile::DOWN = "down";
const char* NcxxRadxFile::DRIFT = "drift";
const char* NcxxRadxFile::DRIFT_CORRECTION = "drift_correction";
const char* NcxxRadxFile::DRIVER = "driver";
const char* NcxxRadxFile::DRIVE_ANGLE_1 = "drive_angle_1";
const char* NcxxRadxFile::DRIVE_ANGLE_2 = "drive_angle_2";
const char* NcxxRadxFile::EASTWARD_VELOCITY = "eastward_velocity";
const char* NcxxRadxFile::EASTWARD_VELOCITY_CORRECTION = "eastward_velocity_correction";
const char* NcxxRadxFile::EASTWARD_WIND = "eastward_wind";
const char* NcxxRadxFile::ELEVATION = "elevation";
const char* NcxxRadxFile::ELEVATION_CORRECTION = "elevation_correction";
const char* NcxxRadxFile::END_DATETIME = "end_datetime";
const char* NcxxRadxFile::FALSE_NORTHING = "false_northing";
const char* NcxxRadxFile::FALSE_EASTING = "false_easting";
const char* NcxxRadxFile::FFT_LENGTH = "fft_length";
const char* NcxxRadxFile::FIELD_FOLDS = "field_folds";
const char* NcxxRadxFile::FILL_VALUE = "_FillValue";
const char* NcxxRadxFile::FIXED_ANGLE = "fixed_angle";
const char* NcxxRadxFile::FLAG_MASKS = "flag_masks";
const char* NcxxRadxFile::FLAG_MEANINGS = "flag_meanings";
const char* NcxxRadxFile::FLAG_VALUES = "flag_values";
const char* NcxxRadxFile::FOLD_LIMIT_LOWER = "fold_limit_lower";
const char* NcxxRadxFile::FOLD_LIMIT_UPPER = "fold_limit_upper";
const char* NcxxRadxFile::FOLLOW_MODE = "follow_mode";
const char* NcxxRadxFile::FREQUENCY = "frequency";
const char* NcxxRadxFile::GEOMETRY_CORRECTION = "geometry_correction";
const char* NcxxRadxFile::GEOREFS_APPLIED = "georefs_applied";
const char* NcxxRadxFile::GEOREF_TIME = "georef_time";
const char* NcxxRadxFile::GREGORIAN = "gregorian";
const char* NcxxRadxFile::GRID_MAPPING = "grid_mapping";
const char* NcxxRadxFile::GRID_MAPPING_NAME = "grid_mapping_name";
const char* NcxxRadxFile::HEADING = "heading";
const char* NcxxRadxFile::HEADING_CHANGE_RATE = "heading_change_rate";
const char* NcxxRadxFile::HEADING_CORRECTION = "heading_correction";
const char* NcxxRadxFile::HISTORY = "history";
const char* NcxxRadxFile::HZ = "s-1";
const char* NcxxRadxFile::INDEX_VAR_NAME = "index_var_name";
const char* NcxxRadxFile::INSTITUTION = "institution";
const char* NcxxRadxFile::INSTRUMENT_NAME = "instrument_name";
const char* NcxxRadxFile::INSTRUMENT_PARAMETERS = "instrument_parameters";
const char* NcxxRadxFile::INSTRUMENT_TYPE = "instrument_type";
const char* NcxxRadxFile::IS_DISCRETE = "is_discrete";
const char* NcxxRadxFile::IS_QUALITY = "is_quality";
const char* NcxxRadxFile::IS_SPECTRUM = "is_spectrum";
const char* NcxxRadxFile::INTERMED_FREQ_HZ = "intermed_freq_hz";
const char* NcxxRadxFile::JOULES = "joules";
const char* NcxxRadxFile::JULIAN = "julian";
const char* NcxxRadxFile::LATITUDE = "latitude";
const char* NcxxRadxFile::LATITUDE_CORRECTION = "latitude_correction";
const char* NcxxRadxFile::LATITUDE_OF_PROJECTION_ORIGIN = "latitude_of_projection_origin";
const char* NcxxRadxFile::LEGEND_XML = "legend_xml";
const char* NcxxRadxFile::LIDAR_APERTURE_DIAMETER = "lidar_aperture_diameter";
const char* NcxxRadxFile::LIDAR_APERTURE_EFFICIENCY = "lidar_aperture_efficiency";
const char* NcxxRadxFile::LIDAR_BEAM_DIVERGENCE = "lidar_beam_divergence";
const char* NcxxRadxFile::LIDAR_CALIBRATION = "lidar_calibration";
const char* NcxxRadxFile::LIDAR_CONSTANT = "lidar_constant";
const char* NcxxRadxFile::LIDAR_FIELD_OF_VIEW = "lidar_field_of_view";
const char* NcxxRadxFile::LIDAR_PARAMETERS = "lidar_parameters";
const char* NcxxRadxFile::LIDAR_PEAK_POWER = "lidar_peak_power";
const char* NcxxRadxFile::LIDAR_PULSE_ENERGY = "lidar_pulse_energy";
const char* NcxxRadxFile::LONGITUDE = "longitude";
const char* NcxxRadxFile::LONGITUDE_CORRECTION = "longitude_correction";
const char* NcxxRadxFile::LONGITUDE_OF_PROJECTION_ORIGIN = "longitude_of_projection_origin";
const char* NcxxRadxFile::LONG_NAME = "long_name";
const char* NcxxRadxFile::META_GROUP = "meta_group";
const char* NcxxRadxFile::METERS = "meters";
const char* NcxxRadxFile::METERS_BETWEEN_GATES = "meters_between_gates";
const char* NcxxRadxFile::METERS_PER_SECOND = "meters per second";
const char* NcxxRadxFile::METERS_TO_CENTER_OF_FIRST_GATE = "meters_to_center_of_first_gate";
const char* NcxxRadxFile::MISSING_VALUE = "missing_value";
const char* NcxxRadxFile::MOVING = "moving";
const char* NcxxRadxFile::MRAD = "mrad";
const char* NcxxRadxFile::NORTHWARD_VELOCITY = "northward_velocity";
const char* NcxxRadxFile::NORTHWARD_VELOCITY_CORRECTION = "northward_velocity_correction";
const char* NcxxRadxFile::NORTHWARD_WIND = "northward_wind";
const char* NcxxRadxFile::NYQUIST_VELOCITY = "nyquist_velocity";
const char* NcxxRadxFile::N_GATES_VARY = "n_gates_vary";
const char* NcxxRadxFile::N_POINTS = "n_points";
const char* NcxxRadxFile::N_PRTS = "n_prts";
const char* NcxxRadxFile::N_SAMPLES = "n_samples";
const char* NcxxRadxFile::N_SPECTRA = "n_spectra";
const char* NcxxRadxFile::OPTIONS = "options";
const char* NcxxRadxFile::ORIGINAL_FORMAT = "original_format";
const char* NcxxRadxFile::PERCENT = "percent";
const char* NcxxRadxFile::PITCH = "pitch";
const char* NcxxRadxFile::PITCH_CHANGE_RATE = "pitch_change_rate";
const char* NcxxRadxFile::PITCH_CORRECTION = "pitch_correction";
const char* NcxxRadxFile::PLATFORM_IS_MOBILE = "platform_is_mobile";
const char* NcxxRadxFile::PLATFORM_TYPE = "platform_type";
const char* NcxxRadxFile::PLATFORM_VELOCITY = "platform_velocity";
const char* NcxxRadxFile::POLARIZATION_MODE = "polarization_mode";
const char* NcxxRadxFile::POLARIZATION_SEQUENCE = "polarization_sequence";
const char* NcxxRadxFile::POSITIVE = "positive";
const char* NcxxRadxFile::PRESSURE_ALTITUDE_CORRECTION = "pressure_altitude_correction";
const char* NcxxRadxFile::PRIMARY_AXIS = "primary_axis";
const char* NcxxRadxFile::PRT = "prt";
const char* NcxxRadxFile::PRT_MODE = "prt_mode";
const char* NcxxRadxFile::PRT_RATIO = "prt_ratio";
const char* NcxxRadxFile::PRT_SEQUENCE = "prt_sequence";
const char* NcxxRadxFile::PULSE_WIDTH = "pulse_width";
const char* NcxxRadxFile::QUALIFIED_VARIABLES = "qualified_variables";
const char* NcxxRadxFile::QC_PROCEDURES = "qc_procedures";
const char* NcxxRadxFile::RADAR_ANTENNA_GAIN_H = "radar_antenna_gain_h";
const char* NcxxRadxFile::RADAR_ANTENNA_GAIN_V = "radar_antenna_gain_v";
const char* NcxxRadxFile::RADAR_BEAM_WIDTH_H = "radar_beam_width_h";
const char* NcxxRadxFile::RADAR_BEAM_WIDTH_V = "radar_beam_width_v";
const char* NcxxRadxFile::RADAR_CALIBRATION = "radar_calibration";
const char* NcxxRadxFile::RADAR_ESTIMATED_NOISE_DBM_HC = "estimated_noise_dbm_hc";
const char* NcxxRadxFile::RADAR_ESTIMATED_NOISE_DBM_HX = "estimated_noise_dbm_hx";
const char* NcxxRadxFile::RADAR_ESTIMATED_NOISE_DBM_VC = "estimated_noise_dbm_vc";
const char* NcxxRadxFile::RADAR_ESTIMATED_NOISE_DBM_VX = "estimated_noise_dbm_vx";
const char* NcxxRadxFile::RADAR_MEASURED_COLD_NOISE = "measured_transmit_cold_noise";
const char* NcxxRadxFile::RADAR_MEASURED_HOT_NOISE = "measured_transmit_hot_noise";
const char* NcxxRadxFile::RADAR_MEASURED_SKY_NOISE = "measured_transmit_sky_noise";
const char* NcxxRadxFile::RADAR_MEASURED_TRANSMIT_POWER_H = "measured_transmit_power_h";
const char* NcxxRadxFile::RADAR_MEASURED_TRANSMIT_POWER_V = "measured_transmit_power_v";
const char* NcxxRadxFile::RADAR_PARAMETERS = "radar_parameters";
const char* NcxxRadxFile::RADAR_RX_BANDWIDTH = "radar_rx_bandwidth";
const char* NcxxRadxFile::RANGE = "range";
const char* NcxxRadxFile::RANGE_CORRECTION = "range_correction";
const char* NcxxRadxFile::RAYS_ARE_INDEXED = "rays_are_indexed";
const char* NcxxRadxFile::RAY_ANGLE_RES = "ray_angle_res";
const char* NcxxRadxFile::RAY_GATE_SPACING = "ray_gate_spacing";
const char* NcxxRadxFile::RAY_N_GATES = "ray_n_gates";
const char* NcxxRadxFile::RAY_START_INDEX = "ray_start_index";
const char* NcxxRadxFile::RAY_START_RANGE = "ray_start_range";
const char* NcxxRadxFile::RAY_TIMES_INCREASE = "ray_times_increase";
const char* NcxxRadxFile::REFERENCES = "references";
const char* NcxxRadxFile::ROLL = "roll";
const char* NcxxRadxFile::ROLL_CHANGE_RATE = "roll_change_rate";
const char* NcxxRadxFile::ROLL_CORRECTION = "roll_correction";
const char* NcxxRadxFile::ROTATION = "rotation";
const char* NcxxRadxFile::ROTATION_CORRECTION = "rotation_correction";
const char* NcxxRadxFile::RX_RANGE_RESOLUTION = "rx_range_resolution";
const char* NcxxRadxFile::R_CALIB = "r_calib";
const char* NcxxRadxFile::R_CALIB_ANTENNA_GAIN_H = "r_calib_antenna_gain_h";
const char* NcxxRadxFile::R_CALIB_ANTENNA_GAIN_V = "r_calib_antenna_gain_v";
const char* NcxxRadxFile::R_CALIB_BASE_DBZ_1KM_HC = "r_calib_base_dbz_1km_hc";
const char* NcxxRadxFile::R_CALIB_BASE_DBZ_1KM_HX = "r_calib_base_dbz_1km_hx";
const char* NcxxRadxFile::R_CALIB_BASE_DBZ_1KM_VC = "r_calib_base_dbz_1km_vc";
const char* NcxxRadxFile::R_CALIB_BASE_DBZ_1KM_VX = "r_calib_base_dbz_1km_vx";
const char* NcxxRadxFile::R_CALIB_COUPLER_FORWARD_LOSS_H = "r_calib_coupler_forward_loss_h";
const char* NcxxRadxFile::R_CALIB_COUPLER_FORWARD_LOSS_V = "r_calib_coupler_forward_loss_v";
const char* NcxxRadxFile::R_CALIB_DBZ_CORRECTION = "r_calib_dbz_correction";
const char* NcxxRadxFile::R_CALIB_DIELECTRIC_FACTOR_USED = "r_calib_dielectric_factor_used";
const char* NcxxRadxFile::R_CALIB_INDEX = "r_calib_index";
const char* NcxxRadxFile::R_CALIB_LDR_CORRECTION_H = "r_calib_ldr_correction_h";
const char* NcxxRadxFile::R_CALIB_LDR_CORRECTION_V = "r_calib_ldr_correction_v";
const char* NcxxRadxFile::R_CALIB_NOISE_HC = "r_calib_noise_hc";
const char* NcxxRadxFile::R_CALIB_NOISE_HX = "r_calib_noise_hx";
const char* NcxxRadxFile::R_CALIB_NOISE_SOURCE_POWER_H = "r_calib_noise_source_power_h";
const char* NcxxRadxFile::R_CALIB_NOISE_SOURCE_POWER_V = "r_calib_noise_source_power_v";
const char* NcxxRadxFile::R_CALIB_NOISE_VC = "r_calib_noise_vc";
const char* NcxxRadxFile::R_CALIB_NOISE_VX = "r_calib_noise_vx";
const char* NcxxRadxFile::R_CALIB_POWER_MEASURE_LOSS_H = "r_calib_power_measure_loss_h";
const char* NcxxRadxFile::R_CALIB_POWER_MEASURE_LOSS_V = "r_calib_power_measure_loss_v";
const char* NcxxRadxFile::R_CALIB_PROBERT_JONES_CORRECTION = "r_calib_probert_jones_correction";
const char* NcxxRadxFile::R_CALIB_PULSE_WIDTH = "r_calib_pulse_width";
const char* NcxxRadxFile::R_CALIB_RADAR_CONSTANT_H = "r_calib_radar_constant_h";
const char* NcxxRadxFile::R_CALIB_RADAR_CONSTANT_V = "r_calib_radar_constant_v";
const char* NcxxRadxFile::R_CALIB_RECEIVER_GAIN_HC = "r_calib_receiver_gain_hc";
const char* NcxxRadxFile::R_CALIB_RECEIVER_GAIN_HX = "r_calib_receiver_gain_hx";
const char* NcxxRadxFile::R_CALIB_RECEIVER_GAIN_VC = "r_calib_receiver_gain_vc";
const char* NcxxRadxFile::R_CALIB_RECEIVER_GAIN_VX = "r_calib_receiver_gain_vx";
const char* NcxxRadxFile::R_CALIB_RECEIVER_MISMATCH_LOSS = "r_calib_receiver_mismatch_loss";
const char* NcxxRadxFile::R_CALIB_RECEIVER_MISMATCH_LOSS_H = "r_calib_receiver_mismatch_loss_h";
const char* NcxxRadxFile::R_CALIB_RECEIVER_MISMATCH_LOSS_V = "r_calib_receiver_mismatch_loss_v";
const char* NcxxRadxFile::R_CALIB_RECEIVER_SLOPE_HC = "r_calib_receiver_slope_hc";
const char* NcxxRadxFile::R_CALIB_RECEIVER_SLOPE_HX = "r_calib_receiver_slope_hx";
const char* NcxxRadxFile::R_CALIB_RECEIVER_SLOPE_VC = "r_calib_receiver_slope_vc";
const char* NcxxRadxFile::R_CALIB_RECEIVER_SLOPE_VX = "r_calib_receiver_slope_vx";
const char* NcxxRadxFile::R_CALIB_SUN_POWER_HC = "r_calib_sun_power_hc";
const char* NcxxRadxFile::R_CALIB_SUN_POWER_HX = "r_calib_sun_power_hx";
const char* NcxxRadxFile::R_CALIB_SUN_POWER_VC = "r_calib_sun_power_vc";
const char* NcxxRadxFile::R_CALIB_SUN_POWER_VX = "r_calib_sun_power_vx";
const char* NcxxRadxFile::R_CALIB_SYSTEM_PHIDP = "r_calib_system_phidp";
const char* NcxxRadxFile::R_CALIB_TEST_POWER_H = "r_calib_test_power_h";
const char* NcxxRadxFile::R_CALIB_TEST_POWER_V = "r_calib_test_power_v";
const char* NcxxRadxFile::R_CALIB_TIME = "r_calib_time";
const char* NcxxRadxFile::R_CALIB_TIME_W3C_STR = "r_calib_time_w3c_str";
const char* NcxxRadxFile::R_CALIB_TWO_WAY_RADOME_LOSS_H = "r_calib_two_way_radome_loss_h";
const char* NcxxRadxFile::R_CALIB_TWO_WAY_RADOME_LOSS_V = "r_calib_two_way_radome_loss_v";
const char* NcxxRadxFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H = "r_calib_two_way_waveguide_loss_h";
const char* NcxxRadxFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V = "r_calib_two_way_waveguide_loss_v";
const char* NcxxRadxFile::R_CALIB_XMIT_POWER_H = "r_calib_xmit_power_h";
const char* NcxxRadxFile::R_CALIB_XMIT_POWER_V = "r_calib_xmit_power_v";
const char* NcxxRadxFile::R_CALIB_ZDR_CORRECTION = "r_calib_zdr_correction";
const char* NcxxRadxFile::SAMPLING_RATIO = "sampling_ratio";
const char* NcxxRadxFile::SCALE_FACTOR = "scale_factor";
const char* NcxxRadxFile::SCANNING = "scanning";
const char* NcxxRadxFile::SCANNING_RADIAL = "scanning_radial";
const char* NcxxRadxFile::SCAN_ID = "scan_id";
const char* NcxxRadxFile::SCAN_NAME = "scan_name";
const char* NcxxRadxFile::SCAN_RATE = "scan_rate";
const char* NcxxRadxFile::SECONDS = "seconds";
const char* NcxxRadxFile::SECS_SINCE_JAN1_1970 = "seconds since 1970-01-01T00:00:00Z";
const char* NcxxRadxFile::SITE_NAME = "site_name";
const char* NcxxRadxFile::SOURCE = "source";
const char* NcxxRadxFile::SPACING_IS_CONSTANT = "spacing_is_constant";
const char* NcxxRadxFile::SPECTRUM_N_SAMPLES = "spectrum_n_samples";
const char* NcxxRadxFile::STANDARD = "standard";
const char* NcxxRadxFile::STANDARD_NAME = "standard_name";
const char* NcxxRadxFile::STARING = "staring";
const char* NcxxRadxFile::START_DATETIME = "start_datetime";
const char* NcxxRadxFile::STATIONARY = "stationary";
const char* NcxxRadxFile::STATUS_XML = "status_xml";
const char* NcxxRadxFile::STATUS_XML_LENGTH = "status_xml_length";
const char* NcxxRadxFile::STRING_LENGTH_256 = "string_length_256";
const char* NcxxRadxFile::STRING_LENGTH_32 = "string_length_32";
const char* NcxxRadxFile::STRING_LENGTH_64 = "string_length_64";
const char* NcxxRadxFile::STRING_LENGTH_8 = "string_length_8";
const char* NcxxRadxFile::SUB_CONVENTIONS = "Sub_conventions";
const char* NcxxRadxFile::SWEEP = "sweep";
const char* NcxxRadxFile::SWEEP_END_RAY_INDEX = "sweep_end_ray_index";
const char* NcxxRadxFile::SWEEP_MODE = "sweep_mode";
const char* NcxxRadxFile::SWEEP_NUMBER = "sweep_number";
const char* NcxxRadxFile::SWEEP_START_RAY_INDEX = "sweep_start_ray_index";
const char* NcxxRadxFile::TARGET_SCAN_RATE = "target_scan_rate";
const char* NcxxRadxFile::THRESHOLDING_XML = "thresholding_xml";
const char* NcxxRadxFile::TILT = "tilt";
const char* NcxxRadxFile::TILT_CORRECTION = "tilt_correction";
const char* NcxxRadxFile::TIME = "time";
const char* NcxxRadxFile::TIME_COVERAGE_END = "time_coverage_end";
const char* NcxxRadxFile::TIME_COVERAGE_START = "time_coverage_start";
const char* NcxxRadxFile::TITLE = "title";
const char* NcxxRadxFile::TRACK = "track";
const char* NcxxRadxFile::UNAMBIGUOUS_RANGE = "unambiguous_range";
const char* NcxxRadxFile::UNITS = "units";
const char* NcxxRadxFile::UP = "up";
const char* NcxxRadxFile::VALID_MAX = "valid_max";
const char* NcxxRadxFile::VALID_MIN = "valid_min";
const char* NcxxRadxFile::VALID_RANGE = "valid_range";
const char* NcxxRadxFile::VERSION = "version";
const char* NcxxRadxFile::VERTICAL_VELOCITY = "vertical_velocity";
const char* NcxxRadxFile::VERTICAL_VELOCITY_CORRECTION = "vertical_velocity_correction";
const char* NcxxRadxFile::VERTICAL_WIND = "vertical_wind";
const char* NcxxRadxFile::VOLUME = "volume";
const char* NcxxRadxFile::VOLUME_NUMBER = "volume_number";
const char* NcxxRadxFile::W3C_STR = "w3c_str";
const char* NcxxRadxFile::WATTS = "watts";
  
/////////////////////////////////////////////////////////////////////////////////////
// standard name string constant instantiation

const char* NcxxRadxFile::ALTITUDE_AGL_LONG = "altitude_above_ground_level";
const char* NcxxRadxFile::ALTITUDE_CORRECTION_LONG = "altitude_correction";
const char* NcxxRadxFile::ALTITUDE_LONG = "altitude";
const char* NcxxRadxFile::ANTENNA_TRANSITION_LONG = "antenna_is_in_transition_between_sweeps";
const char* NcxxRadxFile::AZIMUTH_CORRECTION_LONG = "azimuth_angle_correction";
const char* NcxxRadxFile::AZIMUTH_LONG = "ray_azimuth_angle";
const char* NcxxRadxFile::CO_TO_CROSS_POLAR_CORRELATION_RATIO_H = "co_to_cross_polar_correlation_ratio_h";
const char* NcxxRadxFile::CO_TO_CROSS_POLAR_CORRELATION_RATIO_V = "co_to_cross_polar_correlation_ratio_v";
const char* NcxxRadxFile::CROSS_POLAR_DIFFERENTIAL_PHASE = "cross_polar_differential_phase";
const char* NcxxRadxFile::DRIFT_CORRECTION_LONG = "platform_drift_angle_correction";
const char* NcxxRadxFile::DRIFT_LONG = "platform_drift_angle";
const char* NcxxRadxFile::EASTWARD_VELOCITY_CORRECTION_LONG = "platform_eastward_velocity_correction";
const char* NcxxRadxFile::EASTWARD_VELOCITY_LONG = "platform_eastward_velocity";
const char* NcxxRadxFile::EASTWARD_WIND_LONG = "eastward_wind_speed";
const char* NcxxRadxFile::ELEVATION_CORRECTION_LONG = "ray_elevation_angle_correction";
const char* NcxxRadxFile::ELEVATION_LONG = "ray_elevation_angle";
const char* NcxxRadxFile::FIXED_ANGLE_LONG = "ray_target_fixed_angle";
const char* NcxxRadxFile::FOLLOW_MODE_LONG = "follow_mode_for_scan_strategy";
const char* NcxxRadxFile::FREQUENCY_LONG = "transmission_frequency";
const char* NcxxRadxFile::GEOREF_TIME_LONG = "georef time in seconds since volume start";
const char* NcxxRadxFile::HEADING_CHANGE_RATE_LONG = "platform_heading_angle_rate_of_change";
const char* NcxxRadxFile::HEADING_CORRECTION_LONG = "platform_heading_angle_correction";
const char* NcxxRadxFile::HEADING_LONG = "platform_heading_angle";
const char* NcxxRadxFile::INSTRUMENT_NAME_LONG = "name_of_instrument";
const char* NcxxRadxFile::INSTRUMENT_TYPE_LONG = "type_of_instrument";
const char* NcxxRadxFile::INTERMED_FREQ_HZ_LONG = "intermediate_freqency_hz";
const char* NcxxRadxFile::LATITUDE_CORRECTION_LONG = "latitude_correction";
const char* NcxxRadxFile::LATITUDE_LONG = "latitude";
const char* NcxxRadxFile::LIDAR_APERTURE_DIAMETER_LONG = "lidar_aperture_diameter";
const char* NcxxRadxFile::LIDAR_APERTURE_EFFICIENCY_LONG = "lidar_aperture_efficiency";
const char* NcxxRadxFile::LIDAR_BEAM_DIVERGENCE_LONG = "lidar_beam_divergence";
const char* NcxxRadxFile::LIDAR_CONSTANT_LONG = "lidar_calibration_constant";
const char* NcxxRadxFile::LIDAR_FIELD_OF_VIEW_LONG = "lidar_field_of_view";
const char* NcxxRadxFile::LIDAR_PEAK_POWER_LONG = "lidar_peak_power";
const char* NcxxRadxFile::LIDAR_PULSE_ENERGY_LONG = "lidar_pulse_energy";
const char* NcxxRadxFile::LONGITUDE_CORRECTION_LONG = "longitude_correction";
const char* NcxxRadxFile::LONGITUDE_LONG = "longitude";
const char* NcxxRadxFile::NORTHWARD_VELOCITY_CORRECTION_LONG = "platform_northward_velocity_correction";
const char* NcxxRadxFile::NORTHWARD_VELOCITY_LONG = "platform_northward_velocity";
const char* NcxxRadxFile::NORTHWARD_WIND_LONG = "northward_wind";
const char* NcxxRadxFile::NYQUIST_VELOCITY_LONG = "unambiguous_doppler_velocity";
const char* NcxxRadxFile::N_SAMPLES_LONG = "number_of_samples_used_to_compute_moments";
const char* NcxxRadxFile::PITCH_CHANGE_RATE_LONG = "platform_pitch_angle_rate_of_change";
const char* NcxxRadxFile::PITCH_CORRECTION_LONG = "platform_pitch_angle_correction";
const char* NcxxRadxFile::PITCH_LONG = "platform_pitch_angle";
const char* NcxxRadxFile::PLATFORM_IS_MOBILE_LONG = "platform_is_mobile";
const char* NcxxRadxFile::PLATFORM_TYPE_LONG = "platform_type";
const char* NcxxRadxFile::POLARIZATION_MODE_LONG = "polarization_mode_for_sweep";
const char* NcxxRadxFile::PRESSURE_ALTITUDE_CORRECTION_LONG = "pressure_altitude_correction";
const char* NcxxRadxFile::PRIMARY_AXIS_LONG = "primary_axis_of_rotation";
const char* NcxxRadxFile::PRT_MODE_LONG = "transmit_pulse_mode";
const char* NcxxRadxFile::PRT_RATIO_LONG = "pulse_repetition_frequency_ratio";
const char* NcxxRadxFile::PRT_LONG = "pulse_repetition_time";
const char* NcxxRadxFile::PULSE_WIDTH_LONG = "transmitter_pulse_width";
const char* NcxxRadxFile::RADAR_ANTENNA_GAIN_H_LONG = "nominal_radar_antenna_gain_h_channel";
const char* NcxxRadxFile::RADAR_ANTENNA_GAIN_V_LONG = "nominal_radar_antenna_gain_v_channel";
const char* NcxxRadxFile::RADAR_BEAM_WIDTH_H_LONG = "half_power_radar_beam_width_h_channel";
const char* NcxxRadxFile::RADAR_BEAM_WIDTH_V_LONG = "half_power_radar_beam_width_v_channel";
const char* NcxxRadxFile::RADAR_ESTIMATED_NOISE_DBM_HC_LONG = "estimated_noise_dbm_hc";
const char* NcxxRadxFile::RADAR_ESTIMATED_NOISE_DBM_HX_LONG = "estimated_noise_dbm_hx";
const char* NcxxRadxFile::RADAR_ESTIMATED_NOISE_DBM_VC_LONG = "estimated_noise_dbm_vc";
const char* NcxxRadxFile::RADAR_ESTIMATED_NOISE_DBM_VX_LONG = "estimated_noise_dbm_vx";
const char* NcxxRadxFile::RADAR_MEASURED_TRANSMIT_POWER_H_LONG = "measured_radar_transmit_power_h_channel";
const char* NcxxRadxFile::RADAR_MEASURED_TRANSMIT_POWER_V_LONG = "measured_radar_transmit_power_v_channel";
const char* NcxxRadxFile::RADAR_RX_BANDWIDTH_LONG = "radar_receiver_bandwidth";
const char* NcxxRadxFile::RANGE_CORRECTION_LONG = "range_to_center_of_measurement_volume_correction";
const char* NcxxRadxFile::RANGE_LONG = "range_to_center_of_measurement_volume";
const char* NcxxRadxFile::RAYS_ARE_INDEXED_LONG = "flag_for_indexed_rays";
const char* NcxxRadxFile::RAY_ANGLE_RES_LONG = "angular_resolution_between_rays";
const char* NcxxRadxFile::ROLL_CHANGE_RATE_LONG = "platform_roll_angle_rate_of_change";
const char* NcxxRadxFile::ROLL_CORRECTION_LONG = "platform_roll_angle_correction";
const char* NcxxRadxFile::ROLL_LONG = "platform_roll_angle";
const char* NcxxRadxFile::ROTATION_CORRECTION_LONG = "ray_rotation_angle_relative_to_platform_correction";
const char* NcxxRadxFile::ROTATION_LONG = "ray_rotation_angle_relative_to_platform";
const char* NcxxRadxFile::R_CALIB_ANTENNA_GAIN_H_LONG = "calibrated_radar_antenna_gain_h_channel";
const char* NcxxRadxFile::R_CALIB_ANTENNA_GAIN_V_LONG = "calibrated_radar_antenna_gain_v_channel";
const char* NcxxRadxFile::R_CALIB_BASE_DBZ_1KM_HC_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_BASE_DBZ_1KM_HX_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_BASE_DBZ_1KM_VC_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_BASE_DBZ_1KM_VX_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_COUPLER_FORWARD_LOSS_H_LONG = "radar_calibration_coupler_forward_loss_h_channel";
const char* NcxxRadxFile::R_CALIB_COUPLER_FORWARD_LOSS_V_LONG = "radar_calibration_coupler_forward_loss_v_channel";
const char* NcxxRadxFile::R_CALIB_DBZ_CORRECTION_LONG = "calibrated_radar_dbz_correction";
const char* NcxxRadxFile::R_CALIB_INDEX_LONG = "calibration_data_array_index_per_ray";
const char* NcxxRadxFile::R_CALIB_LDR_CORRECTION_H_LONG = "calibrated_radar_ldr_correction_h_channel";
const char* NcxxRadxFile::R_CALIB_LDR_CORRECTION_V_LONG = "calibrated_radar_ldr_correction_v_channel";
const char* NcxxRadxFile::R_CALIB_NOISE_HC_LONG = "calibrated_radar_receiver_noise_h_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_NOISE_HX_LONG = "calibrated_radar_receiver_noise_h_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_NOISE_SOURCE_POWER_H_LONG = "radar_calibration_noise_source_power_h_channel";
const char* NcxxRadxFile::R_CALIB_NOISE_SOURCE_POWER_V_LONG = "radar_calibration_noise_source_power_v_channel";
const char* NcxxRadxFile::R_CALIB_NOISE_VC_LONG = "calibrated_radar_receiver_noise_v_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_NOISE_VX_LONG = "calibrated_radar_receiver_noise_v_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_POWER_MEASURE_LOSS_H_LONG = "radar_calibration_power_measurement_loss_h_channel";
const char* NcxxRadxFile::R_CALIB_POWER_MEASURE_LOSS_V_LONG = "radar_calibration_power_measurement_loss_v_channel";
const char* NcxxRadxFile::R_CALIB_PULSE_WIDTH_LONG = "radar_calibration_pulse_width";
const char* NcxxRadxFile::R_CALIB_RADAR_CONSTANT_H_LONG = "calibrated_radar_constant_h_channel";
const char* NcxxRadxFile::R_CALIB_RADAR_CONSTANT_V_LONG = "calibrated_radar_constant_v_channel";
const char* NcxxRadxFile::R_CALIB_RECEIVER_GAIN_HC_LONG = "calibrated_radar_receiver_gain_h_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_RECEIVER_GAIN_HX_LONG = "calibrated_radar_receiver_gain_h_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_RECEIVER_GAIN_VC_LONG = "calibrated_radar_receiver_gain_v_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_RECEIVER_GAIN_VX_LONG = "calibrated_radar_receiver_gain_v_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_RECEIVER_MISMATCH_LOSS_LONG = "radar_calibration_receiver_mismatch_loss";
const char* NcxxRadxFile::R_CALIB_RECEIVER_SLOPE_HC_LONG = "calibrated_radar_receiver_slope_h_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_RECEIVER_SLOPE_HX_LONG = "calibrated_radar_receiver_slope_h_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_RECEIVER_SLOPE_VC_LONG = "calibrated_radar_receiver_slope_v_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_RECEIVER_SLOPE_VX_LONG = "calibrated_radar_receiver_slope_v_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_SUN_POWER_HC_LONG = "calibrated_radar_sun_power_h_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_SUN_POWER_HX_LONG = "calibrated_radar_sun_power_h_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_SUN_POWER_VC_LONG = "calibrated_radar_sun_power_v_co_polar_channel";
const char* NcxxRadxFile::R_CALIB_SUN_POWER_VX_LONG = "calibrated_radar_sun_power_v_cross_polar_channel";
const char* NcxxRadxFile::R_CALIB_SYSTEM_PHIDP_LONG = "calibrated_radar_system_phidp";
const char* NcxxRadxFile::R_CALIB_TEST_POWER_H_LONG = "radar_calibration_test_power_h_channel";
const char* NcxxRadxFile::R_CALIB_TEST_POWER_V_LONG = "radar_calibration_test_power_v_channel";
const char* NcxxRadxFile::R_CALIB_TIME_LONG = "radar_calibration_time_utc";
const char* NcxxRadxFile::R_CALIB_TWO_WAY_RADOME_LOSS_H_LONG = "radar_calibration_two_way_radome_loss_h_channel";
const char* NcxxRadxFile::R_CALIB_TWO_WAY_RADOME_LOSS_V_LONG = "radar_calibration_two_way_radome_loss_v_channel";
const char* NcxxRadxFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H_LONG = "radar_calibration_two_way_waveguide_loss_h_channel";
const char* NcxxRadxFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V_LONG = "radar_calibration_two_way_waveguide_loss_v_channel";
const char* NcxxRadxFile::R_CALIB_XMIT_POWER_H_LONG = "calibrated_radar_xmit_power_h_channel";
const char* NcxxRadxFile::R_CALIB_XMIT_POWER_V_LONG = "calibrated_radar_xmit_power_v_channel";
const char* NcxxRadxFile::R_CALIB_ZDR_CORRECTION_LONG = "calibrated_radar_zdr_correction";
const char* NcxxRadxFile::SCAN_ID_LONG = "volume_coverage_pattern";
const char* NcxxRadxFile::SCAN_NAME_LONG = "name_of_antenna_scan_strategy";
const char* NcxxRadxFile::SCAN_RATE_LONG = "antenna_angle_scan_rate";
const char* NcxxRadxFile::SITE_NAME_LONG = "name_of_instrument_site";
const char* NcxxRadxFile::SPACING_IS_CONSTANT_LONG = "spacing_between_range_gates_is_constant";
const char* NcxxRadxFile::SPECTRUM_COPOLAR_HORIZONTAL = "spectrum_copolar_horizontal";
const char* NcxxRadxFile::SPECTRUM_COPOLAR_VERTICAL = "spectrum_copolar_vertical";
const char* NcxxRadxFile::SPECTRUM_CROSSPOLAR_HORIZONTAL = "spectrum_crosspolar_horizontal";
const char* NcxxRadxFile::SPECTRUM_CROSSPOLAR_VERTICAL = "spectrum_crosspolar_vertical";
const char* NcxxRadxFile::CROSS_SPECTRUM_OF_COPOLAR_HORIZONTAL = "cross_spectrum_of_copolar_horizontal";
const char* NcxxRadxFile::CROSS_SPECTRUM_OF_COPOLAR_VERTICAL = "cross_spectrum_of_copolar_vertical";
const char* NcxxRadxFile::CROSS_SPECTRUM_OF_CROSSPOLAR_HORIZONTAL = "cross_spectrum_of_crosspolar_horizontal";
const char* NcxxRadxFile::CROSS_SPECTRUM_OF_CROSSPOLAR_VERTICAL = "cross_spectrum_of_crosspolar_vertical";
const char* NcxxRadxFile::SWEEP_END_RAY_INDEX_LONG = "index_of_last_ray_in_sweep";
const char* NcxxRadxFile::SWEEP_MODE_LONG = "scan_mode_for_sweep";
const char* NcxxRadxFile::SWEEP_NUMBER_LONG = "sweep_index_number_0_based";
const char* NcxxRadxFile::SWEEP_START_RAY_INDEX_LONG = "index_of_first_ray_in_sweep";
const char* NcxxRadxFile::TARGET_SCAN_RATE_LONG = "target_scan_rate_for_sweep";
const char* NcxxRadxFile::TILT_CORRECTION_LONG = "ray_tilt_angle_relative_to_platform_correction";
const char* NcxxRadxFile::TILT_LONG = "ray_tilt_angle_relative_to_platform";
const char* NcxxRadxFile::TIME_COVERAGE_END_LONG = "data_volume_end_time_utc";
const char* NcxxRadxFile::TIME_COVERAGE_START_LONG = "data_volume_start_time_utc";
const char* NcxxRadxFile::TIME_LONG = "time";
const char* NcxxRadxFile::TRACK_LONG = "platform_track_over_the_ground";
const char* NcxxRadxFile::UNAMBIGUOUS_RANGE_LONG = "unambiguous_range";
const char* NcxxRadxFile::VERTICAL_VELOCITY_CORRECTION_LONG = "platform_vertical_velocity_correction";
const char* NcxxRadxFile::VERTICAL_VELOCITY_LONG = "platform_vertical_velocity";
const char* NcxxRadxFile::VERTICAL_WIND_LONG = "upward_air_velocity";
const char* NcxxRadxFile::VOLUME_NUMBER_LONG = "data_volume_index_number";
