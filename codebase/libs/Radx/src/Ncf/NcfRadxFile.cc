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
// NcfRadxFile.cc
//
// NcfRadxFile object
//
// NetCDF file data for radar radial data in CF-compliant file
//
// See also NcfRadxFile_read.cc and NcfRadxFile_write.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
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
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

//////////////
// Constructor

NcfRadxFile::NcfRadxFile() : RadxFile()
  
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

NcfRadxFile::~NcfRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NcfRadxFile::clear()
  
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
#ifdef NOTYET
  _stringLen64Dim = NULL;
  _stringLen256Dim = NULL;
#endif
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
  _rayStartRangeVar = NULL;
  _rayGateSpacingVar = NULL;

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
  _rCalDbzCorrectionVar = NULL;
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

void NcfRadxFile::_clearRays()
{
  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    delete _raysVol[ii];
  }
  _raysVol.clear();
}

void NcfRadxFile::_clearSweeps()
{
  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    delete _sweeps[ii];
  }
  _sweeps.clear();
}

void NcfRadxFile::_clearCals()
{
  for (int ii = 0; ii < (int) _rCals.size(); ii++) {
    delete _rCals[ii];
  }
  _rCals.clear();
}

void NcfRadxFile::_clearFields()
{
  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool NcfRadxFile::isSupported(const string &path)

{
  
  if (isCfRadial(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a CfRadial file
// Returns true on success, false on failure

bool NcfRadxFile::isCfRadial(const string &path)
  
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

int NcfRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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
// convert Radx::DataType_t to Nc3Type

Nc3Type NcfRadxFile::_getNc3Type(Radx::DataType_t dtype)

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
  NcfRadxFile::_getFileFormat(RadxFile::netcdf_format_t format)

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

void NcfRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NcfRadxFile ===============" << endl;
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

int NcfRadxFile::printNative(const string &path, ostream &out,
                             bool printRays, bool printData)
  
{

  _addErrStr("ERROR - NcfRadxFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

//////////////////////////////////////////////////////////////////
// interpret float and double vals, with respect to missing vals

Radx::fl64 NcfRadxFile::_checkMissingDouble(double val)

{
  if (fabs(val - Radx::missingMetaDouble) < 0.0001) {
    return Radx::missingMetaDouble;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaDouble;
  }
  return val;
}

Radx::fl32 NcfRadxFile::_checkMissingFloat(float val)

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

const string NcfRadxFile::CfConvention = "CF-1.6";
const string NcfRadxFile::BaseConvention = "CF-Radial";
const string NcfRadxFile::CurrentVersion = "CF-Radial-1.4";

const char* NcfRadxFile::ADD_OFFSET = "add_offset";
const char* NcfRadxFile::AIRBORNE = "airborne";
const char* NcfRadxFile::ALTITUDE = "altitude";
const char* NcfRadxFile::ALTITUDE_AGL = "altitude_agl";
const char* NcfRadxFile::ALTITUDE_CORRECTION = "altitude_correction";
const char* NcfRadxFile::ALTITUDE_OF_PROJECTION_ORIGIN = "altitude_of_projection_origin";
const char* NcfRadxFile::ANCILLARY_VARIABLES = "ancillary_variables";
const char* NcfRadxFile::ANTENNA_TRANSITION = "antenna_transition";
const char* NcfRadxFile::AUTHOR = "author";
const char* NcfRadxFile::AXIS = "axis";
const char* NcfRadxFile::AZIMUTH = "azimuth";
const char* NcfRadxFile::AZIMUTH_CORRECTION = "azimuth_correction";
const char* NcfRadxFile::BLOCK_AVG_LENGTH = "block_avg_length";
const char* NcfRadxFile::CALENDAR = "calendar";
const char* NcfRadxFile::CFRADIAL = "cfradial";
const char* NcfRadxFile::CM = "cm";
const char* NcfRadxFile::COMMENT = "comment";
const char* NcfRadxFile::COMPRESS = "compress";
const char* NcfRadxFile::CONVENTIONS = "Conventions";
const char* NcfRadxFile::COORDINATES = "coordinates";
const char* NcfRadxFile::CREATED = "created";
const char* NcfRadxFile::DB = "db";
const char* NcfRadxFile::DBM = "dBm";
const char* NcfRadxFile::DBZ = "dBZ";
const char* NcfRadxFile::DEGREES = "degrees";
const char* NcfRadxFile::DEGREES_EAST = "degrees_east";
const char* NcfRadxFile::DEGREES_NORTH = "degrees_north";
const char* NcfRadxFile::DEGREES_PER_SECOND = "degrees per second";
const char* NcfRadxFile::DORADE = "dorade";
const char* NcfRadxFile::DOWN = "down";
const char* NcfRadxFile::DRIFT = "drift";
const char* NcfRadxFile::DRIFT_CORRECTION = "drift_correction";
const char* NcfRadxFile::DRIVER = "driver";
const char* NcfRadxFile::DRIVE_ANGLE_1 = "drive_angle_1";
const char* NcfRadxFile::DRIVE_ANGLE_2 = "drive_angle_2";
const char* NcfRadxFile::EASTWARD_VELOCITY = "eastward_velocity";
const char* NcfRadxFile::EASTWARD_VELOCITY_CORRECTION = "eastward_velocity_correction";
const char* NcfRadxFile::EASTWARD_WIND = "eastward_wind";
const char* NcfRadxFile::ELEVATION = "elevation";
const char* NcfRadxFile::ELEVATION_CORRECTION = "elevation_correction";
const char* NcfRadxFile::END_DATETIME = "end_datetime";
const char* NcfRadxFile::FALSE_NORTHING = "false_northing";
const char* NcfRadxFile::FALSE_EASTING = "false_easting";
const char* NcfRadxFile::FFT_LENGTH = "fft_length";
const char* NcfRadxFile::FIELD_FOLDS = "field_folds";
const char* NcfRadxFile::FILL_VALUE = "_FillValue";
const char* NcfRadxFile::FIXED_ANGLE = "fixed_angle";
const char* NcfRadxFile::FLAG_MASKS = "flag_masks";
const char* NcfRadxFile::FLAG_MEANINGS = "flag_meanings";
const char* NcfRadxFile::FLAG_VALUES = "flag_values";
const char* NcfRadxFile::FOLD_LIMIT_LOWER = "fold_limit_lower";
const char* NcfRadxFile::FOLD_LIMIT_UPPER = "fold_limit_upper";
const char* NcfRadxFile::FOLLOW_MODE = "follow_mode";
const char* NcfRadxFile::FREQUENCY = "frequency";
const char* NcfRadxFile::GEOMETRY_CORRECTION = "geometry_correction";
const char* NcfRadxFile::GEOREFS_APPLIED = "georefs_applied";
const char* NcfRadxFile::GEOREF_TIME = "georef_time";
const char* NcfRadxFile::GEOREF_UNIT_NUM = "georef_unit_num";
const char* NcfRadxFile::GEOREF_UNIT_ID = "georef_unit_id";
const char* NcfRadxFile::GREGORIAN = "gregorian";
const char* NcfRadxFile::GRID_MAPPING = "grid_mapping";
const char* NcfRadxFile::GRID_MAPPING_NAME = "grid_mapping_name";
const char* NcfRadxFile::HEADING = "heading";
const char* NcfRadxFile::HEADING_CHANGE_RATE = "heading_change_rate";
const char* NcfRadxFile::HEADING_CORRECTION = "heading_correction";
const char* NcfRadxFile::HISTORY = "history";
const char* NcfRadxFile::HZ = "s-1";
const char* NcfRadxFile::INDEX_VAR_NAME = "index_var_name";
const char* NcfRadxFile::INSTITUTION = "institution";
const char* NcfRadxFile::INSTRUMENT_NAME = "instrument_name";
const char* NcfRadxFile::INSTRUMENT_PARAMETERS = "instrument_parameters";
const char* NcfRadxFile::INSTRUMENT_TYPE = "instrument_type";
const char* NcfRadxFile::IS_DISCRETE = "is_discrete";
const char* NcfRadxFile::IS_QUALITY = "is_quality";
const char* NcfRadxFile::IS_SPECTRUM = "is_spectrum";
const char* NcfRadxFile::INTERMED_FREQ_HZ = "intermed_freq_hz";
const char* NcfRadxFile::JOULES = "joules";
const char* NcfRadxFile::JULIAN = "julian";
const char* NcfRadxFile::LATITUDE = "latitude";
const char* NcfRadxFile::LATITUDE_CORRECTION = "latitude_correction";
const char* NcfRadxFile::LATITUDE_OF_PROJECTION_ORIGIN = "latitude_of_projection_origin";
const char* NcfRadxFile::LEGEND_XML = "legend_xml";
const char* NcfRadxFile::LIDAR_APERTURE_DIAMETER = "lidar_aperture_diameter";
const char* NcfRadxFile::LIDAR_APERTURE_EFFICIENCY = "lidar_aperture_efficiency";
const char* NcfRadxFile::LIDAR_BEAM_DIVERGENCE = "lidar_beam_divergence";
const char* NcfRadxFile::LIDAR_CALIBRATION = "lidar_calibration";
const char* NcfRadxFile::LIDAR_CONSTANT = "lidar_constant";
const char* NcfRadxFile::LIDAR_FIELD_OF_VIEW = "lidar_field_of_view";
const char* NcfRadxFile::LIDAR_PARAMETERS = "lidar_parameters";
const char* NcfRadxFile::LIDAR_PEAK_POWER = "lidar_peak_power";
const char* NcfRadxFile::LIDAR_PULSE_ENERGY = "lidar_pulse_energy";
const char* NcfRadxFile::LONGITUDE = "longitude";
const char* NcfRadxFile::LONGITUDE_CORRECTION = "longitude_correction";
const char* NcfRadxFile::LONGITUDE_OF_PROJECTION_ORIGIN = "longitude_of_projection_origin";
const char* NcfRadxFile::LONG_NAME = "long_name";
const char* NcfRadxFile::META_GROUP = "meta_group";
const char* NcfRadxFile::METERS = "meters";
const char* NcfRadxFile::METERS_BETWEEN_GATES = "meters_between_gates";
const char* NcfRadxFile::METERS_PER_SECOND = "meters per second";
const char* NcfRadxFile::METERS_TO_CENTER_OF_FIRST_GATE = "meters_to_center_of_first_gate";
const char* NcfRadxFile::MISSING_VALUE = "missing_value";
const char* NcfRadxFile::MOVING = "moving";
const char* NcfRadxFile::MRAD = "mrad";
const char* NcfRadxFile::NORTHWARD_VELOCITY = "northward_velocity";
const char* NcfRadxFile::NORTHWARD_VELOCITY_CORRECTION = "northward_velocity_correction";
const char* NcfRadxFile::NORTHWARD_WIND = "northward_wind";
const char* NcfRadxFile::NYQUIST_VELOCITY = "nyquist_velocity";
const char* NcfRadxFile::N_GATES_VARY = "n_gates_vary";
const char* NcfRadxFile::N_POINTS = "n_points";
const char* NcfRadxFile::N_PRTS = "n_prts";
const char* NcfRadxFile::N_SAMPLES = "n_samples";
const char* NcfRadxFile::N_SPECTRA = "n_spectra";
const char* NcfRadxFile::OPTIONS = "options";
const char* NcfRadxFile::ORIGINAL_FORMAT = "original_format";
const char* NcfRadxFile::PERCENT = "percent";
const char* NcfRadxFile::PITCH = "pitch";
const char* NcfRadxFile::PITCH_CHANGE_RATE = "pitch_change_rate";
const char* NcfRadxFile::PITCH_CORRECTION = "pitch_correction";
const char* NcfRadxFile::PLATFORM_IS_MOBILE = "platform_is_mobile";
const char* NcfRadxFile::PLATFORM_TYPE = "platform_type";
const char* NcfRadxFile::PLATFORM_VELOCITY = "platform_velocity";
const char* NcfRadxFile::POLARIZATION_MODE = "polarization_mode";
const char* NcfRadxFile::POLARIZATION_SEQUENCE = "polarization_sequence";
const char* NcfRadxFile::POSITIVE = "positive";
const char* NcfRadxFile::PRESSURE_ALTITUDE_CORRECTION = "pressure_altitude_correction";
const char* NcfRadxFile::PRIMARY_AXIS = "primary_axis";
const char* NcfRadxFile::PROPOSED_STANDARD_NAME = "proposed_standard_name";
const char* NcfRadxFile::PRT = "prt";
const char* NcfRadxFile::PRT_MODE = "prt_mode";
const char* NcfRadxFile::PRT_RATIO = "prt_ratio";
const char* NcfRadxFile::PRT_SEQUENCE = "prt_sequence";
const char* NcfRadxFile::PULSE_WIDTH = "pulse_width";
const char* NcfRadxFile::QUALIFIED_VARIABLES = "qualified_variables";
const char* NcfRadxFile::QC_PROCEDURES = "qc_procedures";
const char* NcfRadxFile::RADAR_ANTENNA_GAIN_H = "radar_antenna_gain_h";
const char* NcfRadxFile::RADAR_ANTENNA_GAIN_V = "radar_antenna_gain_v";
const char* NcfRadxFile::RADAR_BEAM_WIDTH_H = "radar_beam_width_h";
const char* NcfRadxFile::RADAR_BEAM_WIDTH_V = "radar_beam_width_v";
const char* NcfRadxFile::RADAR_CALIBRATION = "radar_calibration";
const char* NcfRadxFile::RADAR_ESTIMATED_NOISE_DBM_HC = "estimated_noise_dbm_hc";
const char* NcfRadxFile::RADAR_ESTIMATED_NOISE_DBM_HX = "estimated_noise_dbm_hx";
const char* NcfRadxFile::RADAR_ESTIMATED_NOISE_DBM_VC = "estimated_noise_dbm_vc";
const char* NcfRadxFile::RADAR_ESTIMATED_NOISE_DBM_VX = "estimated_noise_dbm_vx";
const char* NcfRadxFile::RADAR_MEASURED_COLD_NOISE = "measured_transmit_cold_noise";
const char* NcfRadxFile::RADAR_MEASURED_HOT_NOISE = "measured_transmit_hot_noise";
const char* NcfRadxFile::RADAR_MEASURED_SKY_NOISE = "measured_transmit_sky_noise";
const char* NcfRadxFile::RADAR_MEASURED_TRANSMIT_POWER_H = "measured_transmit_power_h";
const char* NcfRadxFile::RADAR_MEASURED_TRANSMIT_POWER_V = "measured_transmit_power_v";
const char* NcfRadxFile::RADAR_PARAMETERS = "radar_parameters";
const char* NcfRadxFile::RADAR_RX_BANDWIDTH = "radar_rx_bandwidth";
const char* NcfRadxFile::RANGE = "range";
const char* NcfRadxFile::RANGE_CORRECTION = "range_correction";
const char* NcfRadxFile::RAYS_ARE_INDEXED = "rays_are_indexed";
const char* NcfRadxFile::RAY_ANGLE_RES = "ray_angle_res";
const char* NcfRadxFile::RAY_GATE_SPACING = "ray_gate_spacing";
const char* NcfRadxFile::RAY_N_GATES = "ray_n_gates";
const char* NcfRadxFile::RAY_START_INDEX = "ray_start_index";
const char* NcfRadxFile::RAY_START_RANGE = "ray_start_range";
const char* NcfRadxFile::RAY_TIMES_INCREASE = "ray_times_increase";
const char* NcfRadxFile::REFERENCES = "references";
const char* NcfRadxFile::ROLL = "roll";
const char* NcfRadxFile::ROLL_CHANGE_RATE = "roll_change_rate";
const char* NcfRadxFile::ROLL_CORRECTION = "roll_correction";
const char* NcfRadxFile::ROTATION = "rotation";
const char* NcfRadxFile::ROTATION_CORRECTION = "rotation_correction";
const char* NcfRadxFile::RX_RANGE_RESOLUTION = "rx_range_resolution";
const char* NcfRadxFile::R_CALIB = "r_calib";
const char* NcfRadxFile::R_CALIB_ANTENNA_GAIN_H = "r_calib_antenna_gain_h";
const char* NcfRadxFile::R_CALIB_ANTENNA_GAIN_V = "r_calib_antenna_gain_v";
const char* NcfRadxFile::R_CALIB_BASE_DBZ_1KM_HC = "r_calib_base_dbz_1km_hc";
const char* NcfRadxFile::R_CALIB_BASE_DBZ_1KM_HX = "r_calib_base_dbz_1km_hx";
const char* NcfRadxFile::R_CALIB_BASE_DBZ_1KM_VC = "r_calib_base_dbz_1km_vc";
const char* NcfRadxFile::R_CALIB_BASE_DBZ_1KM_VX = "r_calib_base_dbz_1km_vx";
const char* NcfRadxFile::R_CALIB_COUPLER_FORWARD_LOSS_H = "r_calib_coupler_forward_loss_h";
const char* NcfRadxFile::R_CALIB_COUPLER_FORWARD_LOSS_V = "r_calib_coupler_forward_loss_v";
const char* NcfRadxFile::R_CALIB_DBZ_CORRECTION = "r_calib_dbz_correction";
const char* NcfRadxFile::R_CALIB_DIELECTRIC_FACTOR_USED = "r_calib_dielectric_factor_used";
const char* NcfRadxFile::R_CALIB_INDEX = "r_calib_index";
const char* NcfRadxFile::R_CALIB_LDR_CORRECTION_H = "r_calib_ldr_correction_h";
const char* NcfRadxFile::R_CALIB_LDR_CORRECTION_V = "r_calib_ldr_correction_v";
const char* NcfRadxFile::R_CALIB_NOISE_HC = "r_calib_noise_hc";
const char* NcfRadxFile::R_CALIB_NOISE_HX = "r_calib_noise_hx";
const char* NcfRadxFile::R_CALIB_NOISE_SOURCE_POWER_H = "r_calib_noise_source_power_h";
const char* NcfRadxFile::R_CALIB_NOISE_SOURCE_POWER_V = "r_calib_noise_source_power_v";
const char* NcfRadxFile::R_CALIB_NOISE_VC = "r_calib_noise_vc";
const char* NcfRadxFile::R_CALIB_NOISE_VX = "r_calib_noise_vx";
const char* NcfRadxFile::R_CALIB_POWER_MEASURE_LOSS_H = "r_calib_power_measure_loss_h";
const char* NcfRadxFile::R_CALIB_POWER_MEASURE_LOSS_V = "r_calib_power_measure_loss_v";
const char* NcfRadxFile::R_CALIB_PROBERT_JONES_CORRECTION = "r_calib_probert_jones_correction";
const char* NcfRadxFile::R_CALIB_PULSE_WIDTH = "r_calib_pulse_width";
const char* NcfRadxFile::R_CALIB_RADAR_CONSTANT_H = "r_calib_radar_constant_h";
const char* NcfRadxFile::R_CALIB_RADAR_CONSTANT_V = "r_calib_radar_constant_v";
const char* NcfRadxFile::R_CALIB_RECEIVER_GAIN_HC = "r_calib_receiver_gain_hc";
const char* NcfRadxFile::R_CALIB_RECEIVER_GAIN_HX = "r_calib_receiver_gain_hx";
const char* NcfRadxFile::R_CALIB_RECEIVER_GAIN_VC = "r_calib_receiver_gain_vc";
const char* NcfRadxFile::R_CALIB_RECEIVER_GAIN_VX = "r_calib_receiver_gain_vx";
const char* NcfRadxFile::R_CALIB_RECEIVER_MISMATCH_LOSS = "r_calib_receiver_mismatch_loss";
const char* NcfRadxFile::R_CALIB_RECEIVER_MISMATCH_LOSS_H = "r_calib_receiver_mismatch_loss_h";
const char* NcfRadxFile::R_CALIB_RECEIVER_MISMATCH_LOSS_V = "r_calib_receiver_mismatch_loss_v";
const char* NcfRadxFile::R_CALIB_RECEIVER_SLOPE_HC = "r_calib_receiver_slope_hc";
const char* NcfRadxFile::R_CALIB_RECEIVER_SLOPE_HX = "r_calib_receiver_slope_hx";
const char* NcfRadxFile::R_CALIB_RECEIVER_SLOPE_VC = "r_calib_receiver_slope_vc";
const char* NcfRadxFile::R_CALIB_RECEIVER_SLOPE_VX = "r_calib_receiver_slope_vx";
const char* NcfRadxFile::R_CALIB_SUN_POWER_HC = "r_calib_sun_power_hc";
const char* NcfRadxFile::R_CALIB_SUN_POWER_HX = "r_calib_sun_power_hx";
const char* NcfRadxFile::R_CALIB_SUN_POWER_VC = "r_calib_sun_power_vc";
const char* NcfRadxFile::R_CALIB_SUN_POWER_VX = "r_calib_sun_power_vx";
const char* NcfRadxFile::R_CALIB_SYSTEM_PHIDP = "r_calib_system_phidp";
const char* NcfRadxFile::R_CALIB_TEST_POWER_H = "r_calib_test_power_h";
const char* NcfRadxFile::R_CALIB_TEST_POWER_V = "r_calib_test_power_v";
const char* NcfRadxFile::R_CALIB_TIME = "r_calib_time";
const char* NcfRadxFile::R_CALIB_TIME_W3C_STR = "r_calib_time_w3c_str";
const char* NcfRadxFile::R_CALIB_TWO_WAY_RADOME_LOSS_H = "r_calib_two_way_radome_loss_h";
const char* NcfRadxFile::R_CALIB_TWO_WAY_RADOME_LOSS_V = "r_calib_two_way_radome_loss_v";
const char* NcfRadxFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H = "r_calib_two_way_waveguide_loss_h";
const char* NcfRadxFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V = "r_calib_two_way_waveguide_loss_v";
const char* NcfRadxFile::R_CALIB_XMIT_POWER_H = "r_calib_xmit_power_h";
const char* NcfRadxFile::R_CALIB_XMIT_POWER_V = "r_calib_xmit_power_v";
const char* NcfRadxFile::R_CALIB_ZDR_CORRECTION = "r_calib_zdr_correction";
const char* NcfRadxFile::SAMPLING_RATIO = "sampling_ratio";
const char* NcfRadxFile::SCALE_FACTOR = "scale_factor";
const char* NcfRadxFile::SCANNING = "scanning";
const char* NcfRadxFile::SCANNING_RADIAL = "scanning_radial";
const char* NcfRadxFile::SCAN_ID = "scan_id";
const char* NcfRadxFile::SCAN_NAME = "scan_name";
const char* NcfRadxFile::SCAN_RATE = "scan_rate";
const char* NcfRadxFile::SECONDS = "seconds";
const char* NcfRadxFile::SECS_SINCE_JAN1_1970 = "seconds since 1970-01-01T00:00:00Z";
const char* NcfRadxFile::SITE_NAME = "site_name";
const char* NcfRadxFile::SOURCE = "source";
const char* NcfRadxFile::SPACING_IS_CONSTANT = "spacing_is_constant";
const char* NcfRadxFile::SPECTRUM_N_SAMPLES = "spectrum_n_samples";
const char* NcfRadxFile::STANDARD = "standard";
const char* NcfRadxFile::STANDARD_NAME = "standard_name";
const char* NcfRadxFile::STARING = "staring";
const char* NcfRadxFile::START_DATETIME = "start_datetime";
const char* NcfRadxFile::STATIONARY = "stationary";
const char* NcfRadxFile::STATUS_XML = "status_xml";
const char* NcfRadxFile::STATUS_XML_LENGTH = "status_xml_length";
const char* NcfRadxFile::STRING_LENGTH_256 = "string_length_256";
const char* NcfRadxFile::STRING_LENGTH_32 = "string_length_32";
const char* NcfRadxFile::STRING_LENGTH_64 = "string_length_64";
const char* NcfRadxFile::STRING_LENGTH_8 = "string_length_8";
const char* NcfRadxFile::SUB_CONVENTIONS = "Sub_conventions";
const char* NcfRadxFile::SWEEP = "sweep";
const char* NcfRadxFile::SWEEP_END_RAY_INDEX = "sweep_end_ray_index";
const char* NcfRadxFile::SWEEP_MODE = "sweep_mode";
const char* NcfRadxFile::SWEEP_NUMBER = "sweep_number";
const char* NcfRadxFile::SWEEP_START_RAY_INDEX = "sweep_start_ray_index";
const char* NcfRadxFile::TARGET_SCAN_RATE = "target_scan_rate";
const char* NcfRadxFile::THRESHOLDING_XML = "thresholding_xml";
const char* NcfRadxFile::TILT = "tilt";
const char* NcfRadxFile::TILT_CORRECTION = "tilt_correction";
const char* NcfRadxFile::TIME = "time";
const char* NcfRadxFile::TIME_COVERAGE_END = "time_coverage_end";
const char* NcfRadxFile::TIME_COVERAGE_START = "time_coverage_start";
const char* NcfRadxFile::TITLE = "title";
const char* NcfRadxFile::TRACK = "track";
const char* NcfRadxFile::UNAMBIGUOUS_RANGE = "unambiguous_range";
const char* NcfRadxFile::UNITS = "units";
const char* NcfRadxFile::UP = "up";
const char* NcfRadxFile::VALID_MAX = "valid_max";
const char* NcfRadxFile::VALID_MIN = "valid_min";
const char* NcfRadxFile::VALID_RANGE = "valid_range";
const char* NcfRadxFile::VERSION = "version";
const char* NcfRadxFile::VERTICAL_VELOCITY = "vertical_velocity";
const char* NcfRadxFile::VERTICAL_VELOCITY_CORRECTION = "vertical_velocity_correction";
const char* NcfRadxFile::VERTICAL_WIND = "vertical_wind";
const char* NcfRadxFile::VOLUME = "volume";
const char* NcfRadxFile::VOLUME_NUMBER = "volume_number";
const char* NcfRadxFile::W3C_STR = "w3c_str";
const char* NcfRadxFile::WATTS = "watts";
  
/////////////////////////////////////////////////////////////////////////////////////
// standard name string constant instantiation

const char* NcfRadxFile::ALTITUDE_AGL_LONG = "altitude_above_ground_level";
const char* NcfRadxFile::ALTITUDE_CORRECTION_LONG = "altitude_correction";
const char* NcfRadxFile::ALTITUDE_LONG = "altitude";
const char* NcfRadxFile::ANTENNA_TRANSITION_LONG = "antenna_is_in_transition_between_sweeps";
const char* NcfRadxFile::AZIMUTH_CORRECTION_LONG = "azimuth_angle_correction";
const char* NcfRadxFile::AZIMUTH_LONG = "ray_azimuth_angle";
const char* NcfRadxFile::CO_TO_CROSS_POLAR_CORRELATION_RATIO_H = "co_to_cross_polar_correlation_ratio_h";
const char* NcfRadxFile::CO_TO_CROSS_POLAR_CORRELATION_RATIO_V = "co_to_cross_polar_correlation_ratio_v";
const char* NcfRadxFile::CROSS_POLAR_DIFFERENTIAL_PHASE = "cross_polar_differential_phase";
const char* NcfRadxFile::DRIFT_CORRECTION_LONG = "platform_drift_angle_correction";
const char* NcfRadxFile::DRIFT_LONG = "platform_drift_angle";
const char* NcfRadxFile::EASTWARD_VELOCITY_CORRECTION_LONG = "platform_eastward_velocity_correction";
const char* NcfRadxFile::EASTWARD_VELOCITY_LONG = "platform_eastward_velocity";
const char* NcfRadxFile::EASTWARD_WIND_LONG = "eastward_wind_speed";
const char* NcfRadxFile::ELEVATION_CORRECTION_LONG = "ray_elevation_angle_correction";
const char* NcfRadxFile::ELEVATION_LONG = "ray_elevation_angle";
const char* NcfRadxFile::FIXED_ANGLE_LONG = "ray_target_fixed_angle";
const char* NcfRadxFile::FOLLOW_MODE_LONG = "follow_mode_for_scan_strategy";
const char* NcfRadxFile::FREQUENCY_LONG = "transmission_frequency";
const char* NcfRadxFile::GEOREF_TIME_LONG = "georef time in seconds since volume start";
const char* NcfRadxFile::GEOREF_UNIT_NUM_LONG = "georef hardware unit number";
const char* NcfRadxFile::GEOREF_UNIT_ID_LONG = "georef hardware id or serial number";
const char* NcfRadxFile::HEADING_CHANGE_RATE_LONG = "platform_heading_angle_rate_of_change";
const char* NcfRadxFile::HEADING_CORRECTION_LONG = "platform_heading_angle_correction";
const char* NcfRadxFile::HEADING_LONG = "platform_heading_angle";
const char* NcfRadxFile::INSTRUMENT_NAME_LONG = "name_of_instrument";
const char* NcfRadxFile::INSTRUMENT_TYPE_LONG = "type_of_instrument";
const char* NcfRadxFile::INTERMED_FREQ_HZ_LONG = "intermediate_freqency_hz";
const char* NcfRadxFile::LATITUDE_CORRECTION_LONG = "latitude_correction";
const char* NcfRadxFile::LATITUDE_LONG = "latitude";
const char* NcfRadxFile::LIDAR_APERTURE_DIAMETER_LONG = "lidar_aperture_diameter";
const char* NcfRadxFile::LIDAR_APERTURE_EFFICIENCY_LONG = "lidar_aperture_efficiency";
const char* NcfRadxFile::LIDAR_BEAM_DIVERGENCE_LONG = "lidar_beam_divergence";
const char* NcfRadxFile::LIDAR_CONSTANT_LONG = "lidar_calibration_constant";
const char* NcfRadxFile::LIDAR_FIELD_OF_VIEW_LONG = "lidar_field_of_view";
const char* NcfRadxFile::LIDAR_PEAK_POWER_LONG = "lidar_peak_power";
const char* NcfRadxFile::LIDAR_PULSE_ENERGY_LONG = "lidar_pulse_energy";
const char* NcfRadxFile::LONGITUDE_CORRECTION_LONG = "longitude_correction";
const char* NcfRadxFile::LONGITUDE_LONG = "longitude";
const char* NcfRadxFile::NORTHWARD_VELOCITY_CORRECTION_LONG = "platform_northward_velocity_correction";
const char* NcfRadxFile::NORTHWARD_VELOCITY_LONG = "platform_northward_velocity";
const char* NcfRadxFile::NORTHWARD_WIND_LONG = "northward_wind";
const char* NcfRadxFile::NYQUIST_VELOCITY_LONG = "unambiguous_doppler_velocity";
const char* NcfRadxFile::N_SAMPLES_LONG = "number_of_samples_used_to_compute_moments";
const char* NcfRadxFile::PITCH_CHANGE_RATE_LONG = "platform_pitch_angle_rate_of_change";
const char* NcfRadxFile::PITCH_CORRECTION_LONG = "platform_pitch_angle_correction";
const char* NcfRadxFile::PITCH_LONG = "platform_pitch_angle";
const char* NcfRadxFile::PLATFORM_IS_MOBILE_LONG = "platform_is_mobile";
const char* NcfRadxFile::PLATFORM_TYPE_LONG = "platform_type";
const char* NcfRadxFile::POLARIZATION_MODE_LONG = "polarization_mode_for_sweep";
const char* NcfRadxFile::PRESSURE_ALTITUDE_CORRECTION_LONG = "pressure_altitude_correction";
const char* NcfRadxFile::PRIMARY_AXIS_LONG = "primary_axis_of_rotation";
const char* NcfRadxFile::PRT_MODE_LONG = "transmit_pulse_mode";
const char* NcfRadxFile::PRT_RATIO_LONG = "pulse_repetition_frequency_ratio";
const char* NcfRadxFile::PRT_LONG = "pulse_repetition_time";
const char* NcfRadxFile::PULSE_WIDTH_LONG = "transmitter_pulse_width";
const char* NcfRadxFile::RADAR_ANTENNA_GAIN_H_LONG = "nominal_radar_antenna_gain_h_channel";
const char* NcfRadxFile::RADAR_ANTENNA_GAIN_V_LONG = "nominal_radar_antenna_gain_v_channel";
const char* NcfRadxFile::RADAR_BEAM_WIDTH_H_LONG = "half_power_radar_beam_width_h_channel";
const char* NcfRadxFile::RADAR_BEAM_WIDTH_V_LONG = "half_power_radar_beam_width_v_channel";
const char* NcfRadxFile::RADAR_ESTIMATED_NOISE_DBM_HC_LONG = "estimated_noise_dbm_hc";
const char* NcfRadxFile::RADAR_ESTIMATED_NOISE_DBM_HX_LONG = "estimated_noise_dbm_hx";
const char* NcfRadxFile::RADAR_ESTIMATED_NOISE_DBM_VC_LONG = "estimated_noise_dbm_vc";
const char* NcfRadxFile::RADAR_ESTIMATED_NOISE_DBM_VX_LONG = "estimated_noise_dbm_vx";
const char* NcfRadxFile::RADAR_MEASURED_TRANSMIT_POWER_H_LONG = "measured_radar_transmit_power_h_channel";
const char* NcfRadxFile::RADAR_MEASURED_TRANSMIT_POWER_V_LONG = "measured_radar_transmit_power_v_channel";
const char* NcfRadxFile::RADAR_RX_BANDWIDTH_LONG = "radar_receiver_bandwidth";
const char* NcfRadxFile::RANGE_CORRECTION_LONG = "range_to_center_of_measurement_volume_correction";
const char* NcfRadxFile::RANGE_LONG = "range_to_center_of_measurement_volume";
const char* NcfRadxFile::RAYS_ARE_INDEXED_LONG = "flag_for_indexed_rays";
const char* NcfRadxFile::RAY_ANGLE_RES_LONG = "angular_resolution_between_rays";
const char* NcfRadxFile::ROLL_CHANGE_RATE_LONG = "platform_roll_angle_rate_of_change";
const char* NcfRadxFile::ROLL_CORRECTION_LONG = "platform_roll_angle_correction";
const char* NcfRadxFile::ROLL_LONG = "platform_roll_angle";
const char* NcfRadxFile::ROTATION_CORRECTION_LONG = "ray_rotation_angle_relative_to_platform_correction";
const char* NcfRadxFile::ROTATION_LONG = "ray_rotation_angle_relative_to_platform";
const char* NcfRadxFile::R_CALIB_ANTENNA_GAIN_H_LONG = "calibrated_radar_antenna_gain_h_channel";
const char* NcfRadxFile::R_CALIB_ANTENNA_GAIN_V_LONG = "calibrated_radar_antenna_gain_v_channel";
const char* NcfRadxFile::R_CALIB_BASE_DBZ_1KM_HC_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_co_polar_channel";
const char* NcfRadxFile::R_CALIB_BASE_DBZ_1KM_HX_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_BASE_DBZ_1KM_VC_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_co_polar_channel";
const char* NcfRadxFile::R_CALIB_BASE_DBZ_1KM_VX_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_COUPLER_FORWARD_LOSS_H_LONG = "radar_calibration_coupler_forward_loss_h_channel";
const char* NcfRadxFile::R_CALIB_COUPLER_FORWARD_LOSS_V_LONG = "radar_calibration_coupler_forward_loss_v_channel";
const char* NcfRadxFile::R_CALIB_DBZ_CORRECTION_LONG = "calibrated_radar_dbz_correction";
const char* NcfRadxFile::R_CALIB_INDEX_LONG = "calibration_data_array_index_per_ray";
const char* NcfRadxFile::R_CALIB_LDR_CORRECTION_H_LONG = "calibrated_radar_ldr_correction_h_channel";
const char* NcfRadxFile::R_CALIB_LDR_CORRECTION_V_LONG = "calibrated_radar_ldr_correction_v_channel";
const char* NcfRadxFile::R_CALIB_NOISE_HC_LONG = "calibrated_radar_receiver_noise_h_co_polar_channel";
const char* NcfRadxFile::R_CALIB_NOISE_HX_LONG = "calibrated_radar_receiver_noise_h_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_NOISE_SOURCE_POWER_H_LONG = "radar_calibration_noise_source_power_h_channel";
const char* NcfRadxFile::R_CALIB_NOISE_SOURCE_POWER_V_LONG = "radar_calibration_noise_source_power_v_channel";
const char* NcfRadxFile::R_CALIB_NOISE_VC_LONG = "calibrated_radar_receiver_noise_v_co_polar_channel";
const char* NcfRadxFile::R_CALIB_NOISE_VX_LONG = "calibrated_radar_receiver_noise_v_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_POWER_MEASURE_LOSS_H_LONG = "radar_calibration_power_measurement_loss_h_channel";
const char* NcfRadxFile::R_CALIB_POWER_MEASURE_LOSS_V_LONG = "radar_calibration_power_measurement_loss_v_channel";
const char* NcfRadxFile::R_CALIB_PULSE_WIDTH_LONG = "radar_calibration_pulse_width";
const char* NcfRadxFile::R_CALIB_RADAR_CONSTANT_H_LONG = "calibrated_radar_constant_h_channel";
const char* NcfRadxFile::R_CALIB_RADAR_CONSTANT_V_LONG = "calibrated_radar_constant_v_channel";
const char* NcfRadxFile::R_CALIB_RECEIVER_GAIN_HC_LONG = "calibrated_radar_receiver_gain_h_co_polar_channel";
const char* NcfRadxFile::R_CALIB_RECEIVER_GAIN_HX_LONG = "calibrated_radar_receiver_gain_h_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_RECEIVER_GAIN_VC_LONG = "calibrated_radar_receiver_gain_v_co_polar_channel";
const char* NcfRadxFile::R_CALIB_RECEIVER_GAIN_VX_LONG = "calibrated_radar_receiver_gain_v_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_RECEIVER_MISMATCH_LOSS_LONG = "radar_calibration_receiver_mismatch_loss";
const char* NcfRadxFile::R_CALIB_RECEIVER_SLOPE_HC_LONG = "calibrated_radar_receiver_slope_h_co_polar_channel";
const char* NcfRadxFile::R_CALIB_RECEIVER_SLOPE_HX_LONG = "calibrated_radar_receiver_slope_h_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_RECEIVER_SLOPE_VC_LONG = "calibrated_radar_receiver_slope_v_co_polar_channel";
const char* NcfRadxFile::R_CALIB_RECEIVER_SLOPE_VX_LONG = "calibrated_radar_receiver_slope_v_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_SUN_POWER_HC_LONG = "calibrated_radar_sun_power_h_co_polar_channel";
const char* NcfRadxFile::R_CALIB_SUN_POWER_HX_LONG = "calibrated_radar_sun_power_h_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_SUN_POWER_VC_LONG = "calibrated_radar_sun_power_v_co_polar_channel";
const char* NcfRadxFile::R_CALIB_SUN_POWER_VX_LONG = "calibrated_radar_sun_power_v_cross_polar_channel";
const char* NcfRadxFile::R_CALIB_SYSTEM_PHIDP_LONG = "calibrated_radar_system_phidp";
const char* NcfRadxFile::R_CALIB_TEST_POWER_H_LONG = "radar_calibration_test_power_h_channel";
const char* NcfRadxFile::R_CALIB_TEST_POWER_V_LONG = "radar_calibration_test_power_v_channel";
const char* NcfRadxFile::R_CALIB_TIME_LONG = "radar_calibration_time_utc";
const char* NcfRadxFile::R_CALIB_TWO_WAY_RADOME_LOSS_H_LONG = "radar_calibration_two_way_radome_loss_h_channel";
const char* NcfRadxFile::R_CALIB_TWO_WAY_RADOME_LOSS_V_LONG = "radar_calibration_two_way_radome_loss_v_channel";
const char* NcfRadxFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H_LONG = "radar_calibration_two_way_waveguide_loss_h_channel";
const char* NcfRadxFile::R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V_LONG = "radar_calibration_two_way_waveguide_loss_v_channel";
const char* NcfRadxFile::R_CALIB_XMIT_POWER_H_LONG = "calibrated_radar_xmit_power_h_channel";
const char* NcfRadxFile::R_CALIB_XMIT_POWER_V_LONG = "calibrated_radar_xmit_power_v_channel";
const char* NcfRadxFile::R_CALIB_ZDR_CORRECTION_LONG = "calibrated_radar_zdr_correction";
const char* NcfRadxFile::SCAN_ID_LONG = "volume_coverage_pattern";
const char* NcfRadxFile::SCAN_NAME_LONG = "name_of_antenna_scan_strategy";
const char* NcfRadxFile::SCAN_RATE_LONG = "antenna_angle_scan_rate";
const char* NcfRadxFile::SITE_NAME_LONG = "name_of_instrument_site";
const char* NcfRadxFile::SPACING_IS_CONSTANT_LONG = "spacing_between_range_gates_is_constant";
const char* NcfRadxFile::SPECTRUM_COPOLAR_HORIZONTAL = "spectrum_copolar_horizontal";
const char* NcfRadxFile::SPECTRUM_COPOLAR_VERTICAL = "spectrum_copolar_vertical";
const char* NcfRadxFile::SPECTRUM_CROSSPOLAR_HORIZONTAL = "spectrum_crosspolar_horizontal";
const char* NcfRadxFile::SPECTRUM_CROSSPOLAR_VERTICAL = "spectrum_crosspolar_vertical";
const char* NcfRadxFile::CROSS_SPECTRUM_OF_COPOLAR_HORIZONTAL = "cross_spectrum_of_copolar_horizontal";
const char* NcfRadxFile::CROSS_SPECTRUM_OF_COPOLAR_VERTICAL = "cross_spectrum_of_copolar_vertical";
const char* NcfRadxFile::CROSS_SPECTRUM_OF_CROSSPOLAR_HORIZONTAL = "cross_spectrum_of_crosspolar_horizontal";
const char* NcfRadxFile::CROSS_SPECTRUM_OF_CROSSPOLAR_VERTICAL = "cross_spectrum_of_crosspolar_vertical";
const char* NcfRadxFile::SWEEP_END_RAY_INDEX_LONG = "index_of_last_ray_in_sweep";
const char* NcfRadxFile::SWEEP_MODE_LONG = "scan_mode_for_sweep";
const char* NcfRadxFile::SWEEP_NUMBER_LONG = "sweep_index_number_0_based";
const char* NcfRadxFile::SWEEP_START_RAY_INDEX_LONG = "index_of_first_ray_in_sweep";
const char* NcfRadxFile::TARGET_SCAN_RATE_LONG = "target_scan_rate_for_sweep";
const char* NcfRadxFile::TILT_CORRECTION_LONG = "ray_tilt_angle_relative_to_platform_correction";
const char* NcfRadxFile::TILT_LONG = "ray_tilt_angle_relative_to_platform";
const char* NcfRadxFile::TIME_COVERAGE_END_LONG = "data_volume_end_time_utc";
const char* NcfRadxFile::TIME_COVERAGE_START_LONG = "data_volume_start_time_utc";
const char* NcfRadxFile::TIME_LONG = "time";
const char* NcfRadxFile::TRACK_LONG = "platform_track_over_the_ground";
const char* NcfRadxFile::UNAMBIGUOUS_RANGE_LONG = "unambiguous_range";
const char* NcfRadxFile::VERTICAL_VELOCITY_CORRECTION_LONG = "platform_vertical_velocity_correction";
const char* NcfRadxFile::VERTICAL_VELOCITY_LONG = "platform_vertical_velocity";
const char* NcfRadxFile::VERTICAL_WIND_LONG = "upward_air_velocity";
const char* NcfRadxFile::VOLUME_NUMBER_LONG = "data_volume_index_number";
