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
// Cf2RadxFile.cc
//
// CfRadial2 data for radar radial data in CF-compliant file
//
// See also Cf2RadxFile_read.cc and Cf2RadxFile_write.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2017
//
///////////////////////////////////////////////////////////////

#include <Radx/Cf2RadxFile.hh>
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

Cf2RadxFile::Cf2RadxFile() : RadxFile()
  
{

  _conventions = BaseConvention;
  _version = CurrentVersion;

  _ncFormat = NETCDF4;

  _writeVol = NULL;
  _readVol = NULL;

  clear();

}

/////////////
// destructor

Cf2RadxFile::~Cf2RadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void Cf2RadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _estNoiseAvailHc = false;
  _estNoiseAvailVc = false;
  _estNoiseAvailHx = false;
  _estNoiseAvailVx = false;

  _georefsActive = false;
  _correctionsActive = false;

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

  _nPoints = 0;

}

void Cf2RadxFile::_clearRays()
{
  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    delete _raysVol[ii];
  }
  _raysVol.clear();
}

void Cf2RadxFile::_clearSweeps()
{
  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    delete _sweeps[ii];
  }
  _sweeps.clear();
}

void Cf2RadxFile::_clearCals()
{
  for (int ii = 0; ii < (int) _rCals.size(); ii++) {
    delete _rCals[ii];
  }
  _rCals.clear();
}

void Cf2RadxFile::_clearFields()
{
  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool Cf2RadxFile::isSupported(const string &path)

{
  
  if (isCfRadial2(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a CfRadial2 file
// Returns true on success, false on failure

bool Cf2RadxFile::isCfRadial2(const string &path)
  
{

  clear();
  
  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    if (_verbose) {
      cerr << "DEBUG - not CfRadial file: " << path << endl;
    }
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfRadial2 file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // read global attributes
  
  if (_readGlobalAttributes()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfRadial2 file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check history

  if (_history.find("Cf2RadxFile") == string::npos) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfRadial2 file" << endl;
      cerr << "  No Cf2RadxFile string in history" << endl;
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

int Cf2RadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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

NcxxType Cf2RadxFile::_getNcxxType(Radx::DataType_t dtype)

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

/////////////////////////////////////////////////////////
// print summary after read

void Cf2RadxFile::print(ostream &out) const
  
{
  
  out << "=============== Cf2RadxFile ===============" << endl;
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

int Cf2RadxFile::printNative(const string &path, ostream &out,
                             bool printRays, bool printData)
  
{

  _addErrStr("ERROR - Cf2RadxFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

//////////////////////////////////////////////////////////////////
// interpret float and double vals, with respect to missing vals

Radx::fl64 Cf2RadxFile::_checkMissingDouble(double val)

{
  if (fabs(val - Radx::missingMetaDouble) < 0.0001) {
    return Radx::missingMetaDouble;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaDouble;
  }
  return val;
}

Radx::fl32 Cf2RadxFile::_checkMissingFloat(float val)

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

const string Cf2RadxFile::CfConvention = "CF-1.6";
const string Cf2RadxFile::BaseConvention = "CF-Radial";
const string Cf2RadxFile::CurrentVersion = "2.0";

const char* Cf2RadxFile::ADD_OFFSET = "add_offset";
const char* Cf2RadxFile::AIRBORNE = "airborne";
const char* Cf2RadxFile::ALTITUDE = "altitude";
const char* Cf2RadxFile::ALTITUDE_AGL = "altitude_agl";
const char* Cf2RadxFile::ALTITUDE_CORRECTION = "altitude_correction";
const char* Cf2RadxFile::ALTITUDE_OF_PROJECTION_ORIGIN = "altitude_of_projection_origin";
const char* Cf2RadxFile::ANCILLARY_VARIABLES = "ancillary_variables";
const char* Cf2RadxFile::ANTENNA_GAIN_H = "antenna_gain_h";
const char* Cf2RadxFile::ANTENNA_GAIN_V = "antenna_gain_v";
const char* Cf2RadxFile::ANTENNA_TRANSITION = "antenna_transition";
const char* Cf2RadxFile::AUTHOR = "author";
const char* Cf2RadxFile::AXIS = "axis";
const char* Cf2RadxFile::AZIMUTH = "azimuth";
const char* Cf2RadxFile::AZIMUTH_CORRECTION = "azimuth_correction";
const char* Cf2RadxFile::BASE_DBZ_1KM_HC = "base_dbz_1km_hc";
const char* Cf2RadxFile::BASE_DBZ_1KM_HX = "base_dbz_1km_hx";
const char* Cf2RadxFile::BASE_DBZ_1KM_VC = "base_dbz_1km_vc";
const char* Cf2RadxFile::BASE_DBZ_1KM_VX = "base_dbz_1km_vx";
const char* Cf2RadxFile::BLOCK_AVG_LENGTH = "block_avg_length";
const char* Cf2RadxFile::CALENDAR = "calendar";
const char* Cf2RadxFile::CALIBRATION_TIME = "calibration_time";
const char* Cf2RadxFile::CFRADIAL = "cfradial";
const char* Cf2RadxFile::CM = "cm";
const char* Cf2RadxFile::COMMENT = "comment";
const char* Cf2RadxFile::COMPRESS = "compress";
const char* Cf2RadxFile::CONVENTIONS = "Conventions";
const char* Cf2RadxFile::COORDINATES = "coordinates";
const char* Cf2RadxFile::COUPLER_FORWARD_LOSS_H = "coupler_forward_loss_h";
const char* Cf2RadxFile::COUPLER_FORWARD_LOSS_V = "coupler_forward_loss_v";
const char* Cf2RadxFile::CREATED = "created";
const char* Cf2RadxFile::DB = "db";
const char* Cf2RadxFile::DBM = "dBm";
const char* Cf2RadxFile::DBZ = "dBZ";
const char* Cf2RadxFile::DBZ_CORRECTION = "dbz_correction";
const char* Cf2RadxFile::DEGREES = "degrees";
const char* Cf2RadxFile::DEGREES_EAST = "degrees_east";
const char* Cf2RadxFile::DEGREES_NORTH = "degrees_north";
const char* Cf2RadxFile::DEGREES_PER_SECOND = "degrees per second";
const char* Cf2RadxFile::DIELECTRIC_FACTOR_USED = "dielectric_factor_used";
const char* Cf2RadxFile::DORADE = "dorade";
const char* Cf2RadxFile::DOWN = "down";
const char* Cf2RadxFile::DRIFT = "drift";
const char* Cf2RadxFile::DRIFT_CORRECTION = "drift_correction";
const char* Cf2RadxFile::DRIVER = "driver";
const char* Cf2RadxFile::DRIVE_ANGLE_1 = "drive_angle_1";
const char* Cf2RadxFile::DRIVE_ANGLE_2 = "drive_angle_2";
const char* Cf2RadxFile::EASTWARD_VELOCITY = "eastward_velocity";
const char* Cf2RadxFile::EASTWARD_VELOCITY_CORRECTION = "eastward_velocity_correction";
const char* Cf2RadxFile::EASTWARD_WIND = "eastward_wind";
const char* Cf2RadxFile::ELEVATION = "elevation";
const char* Cf2RadxFile::ELEVATION_CORRECTION = "elevation_correction";
const char* Cf2RadxFile::END_DATETIME = "end_datetime";
const char* Cf2RadxFile::END_TIME = "end_time";
const char* Cf2RadxFile::FALSE_EASTING = "false_easting";
const char* Cf2RadxFile::FALSE_NORTHING = "false_northing";
const char* Cf2RadxFile::FFT_LENGTH = "fft_length";
const char* Cf2RadxFile::FIELD_FOLDS = "field_folds";
const char* Cf2RadxFile::FILL_VALUE = "_FillValue";
const char* Cf2RadxFile::FIXED_ANGLE = "fixed_angle";
const char* Cf2RadxFile::FLAG_MASKS = "flag_masks";
const char* Cf2RadxFile::FLAG_MEANINGS = "flag_meanings";
const char* Cf2RadxFile::FLAG_VALUES = "flag_values";
const char* Cf2RadxFile::FOLD_LIMIT_LOWER = "fold_limit_lower";
const char* Cf2RadxFile::FOLD_LIMIT_UPPER = "fold_limit_upper";
const char* Cf2RadxFile::FOLLOW_MODE = "follow_mode";
const char* Cf2RadxFile::FREQUENCY = "frequency";
const char* Cf2RadxFile::GATE_SPACING = "gate_spacing";
const char* Cf2RadxFile::GEOMETRY_CORRECTION = "geometry_correction";
const char* Cf2RadxFile::GEOREFERENCE = "georeference";
const char* Cf2RadxFile::GEOREFS_APPLIED = "georefs_applied";
const char* Cf2RadxFile::GEOREF_CORRECTION = "georef_correction";
const char* Cf2RadxFile::GEOREF_TIME = "georef_time";
const char* Cf2RadxFile::GREGORIAN = "gregorian";
const char* Cf2RadxFile::GRID_MAPPING = "grid_mapping";
const char* Cf2RadxFile::GRID_MAPPING_NAME = "grid_mapping_name";
const char* Cf2RadxFile::HEADING = "heading";
const char* Cf2RadxFile::HEADING_CHANGE_RATE = "heading_change_rate";
const char* Cf2RadxFile::HEADING_CORRECTION = "heading_correction";
const char* Cf2RadxFile::HISTORY = "history";
const char* Cf2RadxFile::HZ = "s-1";
const char* Cf2RadxFile::INDEX_VAR_NAME = "index_var_name";
const char* Cf2RadxFile::INSTITUTION = "institution";
const char* Cf2RadxFile::INSTRUMENT_NAME = "instrument_name";
const char* Cf2RadxFile::INSTRUMENT_PARAMETERS = "instrument_parameters";
const char* Cf2RadxFile::INSTRUMENT_TYPE = "instrument_type";
const char* Cf2RadxFile::INTERMED_FREQ_HZ = "intermed_freq_hz";
const char* Cf2RadxFile::IS_DISCRETE = "is_discrete";
const char* Cf2RadxFile::IS_QUALITY = "is_quality";
const char* Cf2RadxFile::IS_SPECTRUM = "is_spectrum";
const char* Cf2RadxFile::JOULES = "joules";
const char* Cf2RadxFile::JULIAN = "julian";
const char* Cf2RadxFile::LATITUDE = "latitude";
const char* Cf2RadxFile::LATITUDE_CORRECTION = "latitude_correction";
const char* Cf2RadxFile::LATITUDE_OF_PROJECTION_ORIGIN = "latitude_of_projection_origin";
const char* Cf2RadxFile::LDR_CORRECTION_H = "ldr_correction_h";
const char* Cf2RadxFile::LDR_CORRECTION_V = "ldr_correction_v";
const char* Cf2RadxFile::LEGEND_XML = "legend_xml";
const char* Cf2RadxFile::LIDAR_APERTURE_DIAMETER = "lidar_aperture_diameter";
const char* Cf2RadxFile::LIDAR_APERTURE_EFFICIENCY = "lidar_aperture_efficiency";
const char* Cf2RadxFile::LIDAR_BEAM_DIVERGENCE = "lidar_beam_divergence";
const char* Cf2RadxFile::LIDAR_CALIBRATION = "lidar_calibration";
const char* Cf2RadxFile::LIDAR_CONSTANT = "lidar_constant";
const char* Cf2RadxFile::LIDAR_FIELD_OF_VIEW = "lidar_field_of_view";
const char* Cf2RadxFile::LIDAR_PARAMETERS = "lidar_parameters";
const char* Cf2RadxFile::LIDAR_PEAK_POWER = "lidar_peak_power";
const char* Cf2RadxFile::LIDAR_PULSE_ENERGY = "lidar_pulse_energy";
const char* Cf2RadxFile::LONGITUDE = "longitude";
const char* Cf2RadxFile::LONGITUDE_CORRECTION = "longitude_correction";
const char* Cf2RadxFile::LONGITUDE_OF_PROJECTION_ORIGIN = "longitude_of_projection_origin";
const char* Cf2RadxFile::LONG_NAME = "long_name";
const char* Cf2RadxFile::META_GROUP = "meta_group";
const char* Cf2RadxFile::METERS = "meters";
const char* Cf2RadxFile::METERS_BETWEEN_GATES = "meters_between_gates";
const char* Cf2RadxFile::METERS_PER_SECOND = "meters per second";
const char* Cf2RadxFile::METERS_TO_CENTER_OF_FIRST_GATE = "meters_to_center_of_first_gate";
const char* Cf2RadxFile::MISSING_VALUE = "missing_value";
const char* Cf2RadxFile::MOVING = "moving";
const char* Cf2RadxFile::MONITORING = "monitoring";
const char* Cf2RadxFile::MRAD = "mrad";
const char* Cf2RadxFile::NOISE_HC = "noise_hc";
const char* Cf2RadxFile::NOISE_HX = "noise_hx";
const char* Cf2RadxFile::NOISE_SOURCE_POWER_H = "noise_source_power_h";
const char* Cf2RadxFile::NOISE_SOURCE_POWER_V = "noise_source_power_v";
const char* Cf2RadxFile::NOISE_VC = "noise_vc";
const char* Cf2RadxFile::NOISE_VX = "noise_vx";
const char* Cf2RadxFile::NORTHWARD_VELOCITY = "northward_velocity";
const char* Cf2RadxFile::NORTHWARD_VELOCITY_CORRECTION = "northward_velocity_correction";
const char* Cf2RadxFile::NORTHWARD_WIND = "northward_wind";
const char* Cf2RadxFile::NYQUIST_VELOCITY = "nyquist_velocity";
const char* Cf2RadxFile::N_GATES_VARY = "n_gates_vary";
const char* Cf2RadxFile::N_POINTS = "n_points";
const char* Cf2RadxFile::N_PRTS = "n_prts";
const char* Cf2RadxFile::N_SAMPLES = "n_samples";
const char* Cf2RadxFile::N_SPECTRA = "n_spectra";
const char* Cf2RadxFile::OPTIONS = "options";
const char* Cf2RadxFile::ORIGINAL_FORMAT = "original_format";
const char* Cf2RadxFile::PERCENT = "percent";
const char* Cf2RadxFile::PITCH = "pitch";
const char* Cf2RadxFile::PITCH_CHANGE_RATE = "pitch_change_rate";
const char* Cf2RadxFile::PITCH_CORRECTION = "pitch_correction";
const char* Cf2RadxFile::PLATFORM_IS_MOBILE = "platform_is_mobile";
const char* Cf2RadxFile::PLATFORM_TYPE = "platform_type";
const char* Cf2RadxFile::PLATFORM_VELOCITY = "platform_velocity";
const char* Cf2RadxFile::POLARIZATION_MODE = "polarization_mode";
const char* Cf2RadxFile::POLARIZATION_SEQUENCE = "polarization_sequence";
const char* Cf2RadxFile::POSITIVE = "positive";
const char* Cf2RadxFile::POWER_MEASURE_LOSS_H = "power_measure_loss_h";
const char* Cf2RadxFile::POWER_MEASURE_LOSS_V = "power_measure_loss_v";
const char* Cf2RadxFile::PRESSURE_ALTITUDE_CORRECTION = "pressure_altitude_correction";
const char* Cf2RadxFile::PRIMARY_AXIS = "primary_axis";
const char* Cf2RadxFile::PROBERT_JONES_CORRECTION = "probert_jones_correction";
const char* Cf2RadxFile::PROPOSED_STANDARD_NAME = "proposed_standard_name";
const char* Cf2RadxFile::PRT = "prt";
const char* Cf2RadxFile::PRT_MODE = "prt_mode";
const char* Cf2RadxFile::PRT_RATIO = "prt_ratio";
const char* Cf2RadxFile::PRT_SEQUENCE = "prt_sequence";
const char* Cf2RadxFile::PULSE_WIDTH = "pulse_width";
const char* Cf2RadxFile::QC_PROCEDURES = "qc_procedures";
const char* Cf2RadxFile::QUALIFIED_VARIABLES = "qualified_variables";
const char* Cf2RadxFile::RADAR_ANTENNA_GAIN_H = "radar_antenna_gain_h";
const char* Cf2RadxFile::RADAR_ANTENNA_GAIN_V = "radar_antenna_gain_v";
const char* Cf2RadxFile::RADAR_BEAM_WIDTH_H = "radar_beam_width_h";
const char* Cf2RadxFile::RADAR_BEAM_WIDTH_V = "radar_beam_width_v";
const char* Cf2RadxFile::RADAR_CALIBRATION = "radar_calibration";
const char* Cf2RadxFile::RADAR_CONSTANT_H = "radar_constant_h";
const char* Cf2RadxFile::RADAR_CONSTANT_V = "radar_constant_v";
const char* Cf2RadxFile::RADAR_ESTIMATED_NOISE_DBM_HC = "estimated_noise_dbm_hc";
const char* Cf2RadxFile::RADAR_ESTIMATED_NOISE_DBM_HX = "estimated_noise_dbm_hx";
const char* Cf2RadxFile::RADAR_ESTIMATED_NOISE_DBM_VC = "estimated_noise_dbm_vc";
const char* Cf2RadxFile::RADAR_ESTIMATED_NOISE_DBM_VX = "estimated_noise_dbm_vx";
const char* Cf2RadxFile::RADAR_MEASURED_COLD_NOISE = "measured_transmit_cold_noise";
const char* Cf2RadxFile::RADAR_MEASURED_HOT_NOISE = "measured_transmit_hot_noise";
const char* Cf2RadxFile::RADAR_MEASURED_SKY_NOISE = "measured_transmit_sky_noise";
const char* Cf2RadxFile::RADAR_MEASURED_TRANSMIT_POWER_H = "measured_transmit_power_h";
const char* Cf2RadxFile::RADAR_MEASURED_TRANSMIT_POWER_V = "measured_transmit_power_v";
const char* Cf2RadxFile::RADAR_PARAMETERS = "radar_parameters";
const char* Cf2RadxFile::RADAR_RX_BANDWIDTH = "radar_rx_bandwidth";
const char* Cf2RadxFile::RANGE = "range";
const char* Cf2RadxFile::RANGE_CORRECTION = "range_correction";
const char* Cf2RadxFile::RAYS_ARE_INDEXED = "rays_are_indexed";
const char* Cf2RadxFile::RAY_ANGLE_RES = "ray_angle_res";
const char* Cf2RadxFile::RAY_GATE_SPACING = "ray_gate_spacing";
const char* Cf2RadxFile::RAY_N_GATES = "ray_n_gates";
const char* Cf2RadxFile::RAY_START_INDEX = "ray_start_index";
const char* Cf2RadxFile::RAY_START_RANGE = "ray_start_range";
const char* Cf2RadxFile::RAY_TIMES_INCREASE = "ray_times_increase";
const char* Cf2RadxFile::R_CALIB_INDEX = "r_calib_index";
const char* Cf2RadxFile::RECEIVER_GAIN_HC = "receiver_gain_hc";
const char* Cf2RadxFile::RECEIVER_GAIN_HX = "receiver_gain_hx";
const char* Cf2RadxFile::RECEIVER_GAIN_VC = "receiver_gain_vc";
const char* Cf2RadxFile::RECEIVER_GAIN_VX = "receiver_gain_vx";
const char* Cf2RadxFile::RECEIVER_MISMATCH_LOSS = "receiver_mismatch_loss";
const char* Cf2RadxFile::RECEIVER_MISMATCH_LOSS_H = "receiver_mismatch_loss_h";
const char* Cf2RadxFile::RECEIVER_MISMATCH_LOSS_V = "receiver_mismatch_loss_v";
const char* Cf2RadxFile::RECEIVER_SLOPE_HC = "receiver_slope_hc";
const char* Cf2RadxFile::RECEIVER_SLOPE_HX = "receiver_slope_hx";
const char* Cf2RadxFile::RECEIVER_SLOPE_VC = "receiver_slope_vc";
const char* Cf2RadxFile::RECEIVER_SLOPE_VX = "receiver_slope_vx";
const char* Cf2RadxFile::REFERENCES = "references";
const char* Cf2RadxFile::ROLL = "roll";
const char* Cf2RadxFile::ROLL_CHANGE_RATE = "roll_change_rate";
const char* Cf2RadxFile::ROLL_CORRECTION = "roll_correction";
const char* Cf2RadxFile::ROTATION = "rotation";
const char* Cf2RadxFile::ROTATION_CORRECTION = "rotation_correction";
const char* Cf2RadxFile::RX_RANGE_RESOLUTION = "rx_range_resolution";
const char* Cf2RadxFile::R_CALIB = "r_calib";
const char* Cf2RadxFile::SAMPLING_RATIO = "sampling_ratio";
const char* Cf2RadxFile::SCALE_FACTOR = "scale_factor";
const char* Cf2RadxFile::SCANNING = "scanning";
const char* Cf2RadxFile::SCANNING_RADIAL = "scanning_radial";
const char* Cf2RadxFile::SCAN_ID = "scan_id";
const char* Cf2RadxFile::SCAN_NAME = "scan_name";
const char* Cf2RadxFile::SCAN_RATE = "scan_rate";
const char* Cf2RadxFile::SECONDS = "seconds";
const char* Cf2RadxFile::SECS_SINCE_JAN1_1970 = "seconds since 1970-01-01T00:00:00Z";
const char* Cf2RadxFile::SITE_NAME = "site_name";
const char* Cf2RadxFile::SOURCE = "source";
const char* Cf2RadxFile::SPACING_IS_CONSTANT = "spacing_is_constant";
const char* Cf2RadxFile::SPECTRUM_N_SAMPLES = "spectrum_n_samples";
const char* Cf2RadxFile::STANDARD = "standard";
const char* Cf2RadxFile::STANDARD_NAME = "standard_name";
const char* Cf2RadxFile::STARING = "staring";
const char* Cf2RadxFile::START_DATETIME = "start_datetime";
const char* Cf2RadxFile::START_TIME = "start_time";
const char* Cf2RadxFile::START_RANGE = "start_range";
const char* Cf2RadxFile::STATIONARY = "stationary";
const char* Cf2RadxFile::STATUS_XML = "status_xml";
const char* Cf2RadxFile::STATUS_XML_LENGTH = "status_xml_length";
const char* Cf2RadxFile::STRING_LENGTH_256 = "string_length_256";
const char* Cf2RadxFile::STRING_LENGTH_32 = "string_length_32";
const char* Cf2RadxFile::STRING_LENGTH_64 = "string_length_64";
const char* Cf2RadxFile::STRING_LENGTH_8 = "string_length_8";
const char* Cf2RadxFile::SUB_CONVENTIONS = "Sub_conventions";
const char* Cf2RadxFile::SUN_POWER_HC = "sun_power_hc";
const char* Cf2RadxFile::SUN_POWER_HX = "sun_power_hx";
const char* Cf2RadxFile::SUN_POWER_VC = "sun_power_vc";
const char* Cf2RadxFile::SUN_POWER_VX = "sun_power_vx";
const char* Cf2RadxFile::SWEEP = "sweep";
const char* Cf2RadxFile::SWEEP_END_RAY_INDEX = "sweep_end_ray_index";
const char* Cf2RadxFile::SWEEP_FIXED_ANGLE = "sweep_fixed_angle";
const char* Cf2RadxFile::SWEEP_GROUP_NAME = "sweep_group_name";
const char* Cf2RadxFile::SWEEP_MODE = "sweep_mode";
const char* Cf2RadxFile::SWEEP_NUMBER = "sweep_number";
const char* Cf2RadxFile::SWEEP_START_RAY_INDEX = "sweep_start_ray_index";
const char* Cf2RadxFile::SYSTEM_PHIDP = "system_phidp";
const char* Cf2RadxFile::TARGET_SCAN_RATE = "target_scan_rate";
const char* Cf2RadxFile::TEST_POWER_H = "test_power_h";
const char* Cf2RadxFile::TEST_POWER_V = "test_power_v";
const char* Cf2RadxFile::THRESHOLDING_XML = "thresholding_xml";
const char* Cf2RadxFile::TILT = "tilt";
const char* Cf2RadxFile::TILT_CORRECTION = "tilt_correction";
const char* Cf2RadxFile::TIME = "time";
const char* Cf2RadxFile::TIME_COVERAGE_END = "time_coverage_end";
const char* Cf2RadxFile::TIME_COVERAGE_START = "time_coverage_start";
const char* Cf2RadxFile::TIME_W3C_STR = "time_w3c_str";
const char* Cf2RadxFile::TITLE = "title";
const char* Cf2RadxFile::TRACK = "track";
const char* Cf2RadxFile::TWO_WAY_RADOME_LOSS_H = "two_way_radome_loss_h";
const char* Cf2RadxFile::TWO_WAY_RADOME_LOSS_V = "two_way_radome_loss_v";
const char* Cf2RadxFile::TWO_WAY_WAVEGUIDE_LOSS_H = "two_way_waveguide_loss_h";
const char* Cf2RadxFile::TWO_WAY_WAVEGUIDE_LOSS_V = "two_way_waveguide_loss_v";
const char* Cf2RadxFile::UNAMBIGUOUS_RANGE = "unambiguous_range";
const char* Cf2RadxFile::UNITS = "units";
const char* Cf2RadxFile::UP = "up";
const char* Cf2RadxFile::VALID_MAX = "valid_max";
const char* Cf2RadxFile::VALID_MIN = "valid_min";
const char* Cf2RadxFile::VALID_RANGE = "valid_range";
const char* Cf2RadxFile::VERSION = "version";
const char* Cf2RadxFile::VERTICAL_VELOCITY = "vertical_velocity";
const char* Cf2RadxFile::VERTICAL_VELOCITY_CORRECTION = "vertical_velocity_correction";
const char* Cf2RadxFile::VERTICAL_WIND = "vertical_wind";
const char* Cf2RadxFile::VOLUME = "volume";
const char* Cf2RadxFile::VOLUME_NUMBER = "volume_number";
const char* Cf2RadxFile::W3C_STR = "w3c_str";
const char* Cf2RadxFile::WATTS = "watts";
const char* Cf2RadxFile::XMIT_POWER_H = "xmit_power_h";
const char* Cf2RadxFile::XMIT_POWER_V = "xmit_power_v";
const char* Cf2RadxFile::ZDR_CORRECTION = "zdr_correction";
  
/////////////////////////////////////////////////////////////////////////////////////
// standard name string constant instantiation

const char* Cf2RadxFile::ALTITUDE_AGL_LONG = "altitude_above_ground_level";
const char* Cf2RadxFile::ALTITUDE_CORRECTION_LONG = "altitude_correction";
const char* Cf2RadxFile::ALTITUDE_LONG = "altitude";
const char* Cf2RadxFile::ANTENNA_GAIN_H_LONG = "calibrated_radar_antenna_gain_h_channel";
const char* Cf2RadxFile::ANTENNA_GAIN_V_LONG = "calibrated_radar_antenna_gain_v_channel";
const char* Cf2RadxFile::ANTENNA_TRANSITION_LONG = "antenna_is_in_transition_between_sweeps";
const char* Cf2RadxFile::AZIMUTH_CORRECTION_LONG = "azimuth_angle_correction";
const char* Cf2RadxFile::AZIMUTH_LONG = "ray_azimuth_angle";
const char* Cf2RadxFile::BASE_DBZ_1KM_HC_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_co_polar_channel";
const char* Cf2RadxFile::BASE_DBZ_1KM_HX_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_cross_polar_channel";
const char* Cf2RadxFile::BASE_DBZ_1KM_VC_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_co_polar_channel";
const char* Cf2RadxFile::BASE_DBZ_1KM_VX_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_cross_polar_channel";
const char* Cf2RadxFile::COUPLER_FORWARD_LOSS_H_LONG = "radar_calibration_coupler_forward_loss_h_channel";
const char* Cf2RadxFile::COUPLER_FORWARD_LOSS_V_LONG = "radar_calibration_coupler_forward_loss_v_channel";
const char* Cf2RadxFile::CO_TO_CROSS_POLAR_CORRELATION_RATIO_H = "co_to_cross_polar_correlation_ratio_h";
const char* Cf2RadxFile::CO_TO_CROSS_POLAR_CORRELATION_RATIO_V = "co_to_cross_polar_correlation_ratio_v";
const char* Cf2RadxFile::CROSS_POLAR_DIFFERENTIAL_PHASE = "cross_polar_differential_phase";
const char* Cf2RadxFile::CROSS_SPECTRUM_OF_COPOLAR_HORIZONTAL = "cross_spectrum_of_copolar_horizontal";
const char* Cf2RadxFile::CROSS_SPECTRUM_OF_COPOLAR_VERTICAL = "cross_spectrum_of_copolar_vertical";
const char* Cf2RadxFile::CROSS_SPECTRUM_OF_CROSSPOLAR_HORIZONTAL = "cross_spectrum_of_crosspolar_horizontal";
const char* Cf2RadxFile::CROSS_SPECTRUM_OF_CROSSPOLAR_VERTICAL = "cross_spectrum_of_crosspolar_vertical";
const char* Cf2RadxFile::DBZ_CORRECTION_LONG = "calibrated_radar_dbz_correction";
const char* Cf2RadxFile::DRIFT_CORRECTION_LONG = "platform_drift_angle_correction";
const char* Cf2RadxFile::DRIFT_LONG = "platform_drift_angle";
const char* Cf2RadxFile::DRIVE_ANGLE_1_LONG = "antenna_drive_angle_1";
const char* Cf2RadxFile::DRIVE_ANGLE_2_LONG = "antenna_drive_angle_2";
const char* Cf2RadxFile::EASTWARD_VELOCITY_CORRECTION_LONG = "platform_eastward_velocity_correction";
const char* Cf2RadxFile::EASTWARD_VELOCITY_LONG = "platform_eastward_velocity";
const char* Cf2RadxFile::EASTWARD_WIND_LONG = "eastward_wind_speed";
const char* Cf2RadxFile::ELEVATION_CORRECTION_LONG = "ray_elevation_angle_correction";
const char* Cf2RadxFile::ELEVATION_LONG = "ray_elevation_angle";
const char* Cf2RadxFile::FIXED_ANGLE_LONG = "ray_target_fixed_angle";
const char* Cf2RadxFile::FOLLOW_MODE_LONG = "follow_mode_for_scan_strategy";
const char* Cf2RadxFile::FREQUENCY_LONG = "transmission_frequency";
const char* Cf2RadxFile::GEOREF_TIME_LONG = "georef time in seconds since volume start";
const char* Cf2RadxFile::HEADING_CHANGE_RATE_LONG = "platform_heading_angle_rate_of_change";
const char* Cf2RadxFile::HEADING_CORRECTION_LONG = "platform_heading_angle_correction";
const char* Cf2RadxFile::HEADING_LONG = "platform_heading_angle";
const char* Cf2RadxFile::INSTRUMENT_NAME_LONG = "name_of_instrument";
const char* Cf2RadxFile::INSTRUMENT_TYPE_LONG = "type_of_instrument";
const char* Cf2RadxFile::INTERMED_FREQ_HZ_LONG = "intermediate_freqency_hz";
const char* Cf2RadxFile::LATITUDE_CORRECTION_LONG = "latitude_correction";
const char* Cf2RadxFile::LATITUDE_LONG = "latitude";
const char* Cf2RadxFile::LDR_CORRECTION_H_LONG = "calibrated_radar_ldr_correction_h_channel";
const char* Cf2RadxFile::LDR_CORRECTION_V_LONG = "calibrated_radar_ldr_correction_v_channel";
const char* Cf2RadxFile::LIDAR_APERTURE_DIAMETER_LONG = "lidar_aperture_diameter";
const char* Cf2RadxFile::LIDAR_APERTURE_EFFICIENCY_LONG = "lidar_aperture_efficiency";
const char* Cf2RadxFile::LIDAR_BEAM_DIVERGENCE_LONG = "lidar_beam_divergence";
const char* Cf2RadxFile::LIDAR_CONSTANT_LONG = "lidar_calibration_constant";
const char* Cf2RadxFile::LIDAR_FIELD_OF_VIEW_LONG = "lidar_field_of_view";
const char* Cf2RadxFile::LIDAR_PEAK_POWER_LONG = "lidar_peak_power";
const char* Cf2RadxFile::LIDAR_PULSE_ENERGY_LONG = "lidar_pulse_energy";
const char* Cf2RadxFile::LONGITUDE_CORRECTION_LONG = "longitude_correction";
const char* Cf2RadxFile::LONGITUDE_LONG = "longitude";
const char* Cf2RadxFile::NOISE_HC_LONG = "calibrated_radar_receiver_noise_h_co_polar_channel";
const char* Cf2RadxFile::NOISE_HX_LONG = "calibrated_radar_receiver_noise_h_cross_polar_channel";
const char* Cf2RadxFile::NOISE_SOURCE_POWER_H_LONG = "radar_calibration_noise_source_power_h_channel";
const char* Cf2RadxFile::NOISE_SOURCE_POWER_V_LONG = "radar_calibration_noise_source_power_v_channel";
const char* Cf2RadxFile::NOISE_VC_LONG = "calibrated_radar_receiver_noise_v_co_polar_channel";
const char* Cf2RadxFile::NOISE_VX_LONG = "calibrated_radar_receiver_noise_v_cross_polar_channel";
const char* Cf2RadxFile::NORTHWARD_VELOCITY_CORRECTION_LONG = "platform_northward_velocity_correction";
const char* Cf2RadxFile::NORTHWARD_VELOCITY_LONG = "platform_northward_velocity";
const char* Cf2RadxFile::NORTHWARD_WIND_LONG = "northward_wind";
const char* Cf2RadxFile::NYQUIST_VELOCITY_LONG = "unambiguous_doppler_velocity";
const char* Cf2RadxFile::N_SAMPLES_LONG = "number_of_samples_used_to_compute_moments";
const char* Cf2RadxFile::PITCH_CHANGE_RATE_LONG = "platform_pitch_angle_rate_of_change";
const char* Cf2RadxFile::PITCH_CORRECTION_LONG = "platform_pitch_angle_correction";
const char* Cf2RadxFile::PITCH_LONG = "platform_pitch_angle";
const char* Cf2RadxFile::PLATFORM_IS_MOBILE_LONG = "platform_is_mobile";
const char* Cf2RadxFile::PLATFORM_TYPE_LONG = "platform_type";
const char* Cf2RadxFile::POLARIZATION_MODE_LONG = "polarization_mode_for_sweep";
const char* Cf2RadxFile::POWER_MEASURE_LOSS_H_LONG = "radar_calibration_power_measurement_loss_h_channel";
const char* Cf2RadxFile::POWER_MEASURE_LOSS_V_LONG = "radar_calibration_power_measurement_loss_v_channel";
const char* Cf2RadxFile::PRESSURE_ALTITUDE_CORRECTION_LONG = "pressure_altitude_correction";
const char* Cf2RadxFile::PRIMARY_AXIS_LONG = "primary_axis_of_rotation";
const char* Cf2RadxFile::PRT_LONG = "pulse_repetition_time";
const char* Cf2RadxFile::PRT_MODE_LONG = "transmit_pulse_mode";
const char* Cf2RadxFile::PRT_RATIO_LONG = "pulse_repetition_frequency_ratio";
const char* Cf2RadxFile::PULSE_WIDTH_LONG = "transmitter_pulse_width";
const char* Cf2RadxFile::RADAR_ANTENNA_GAIN_H_LONG = "nominal_radar_antenna_gain_h_channel";
const char* Cf2RadxFile::RADAR_ANTENNA_GAIN_V_LONG = "nominal_radar_antenna_gain_v_channel";
const char* Cf2RadxFile::RADAR_BEAM_WIDTH_H_LONG = "half_power_radar_beam_width_h_channel";
const char* Cf2RadxFile::RADAR_BEAM_WIDTH_V_LONG = "half_power_radar_beam_width_v_channel";
const char* Cf2RadxFile::RADAR_CONSTANT_H_LONG = "calibrated_radar_constant_h_channel";
const char* Cf2RadxFile::RADAR_CONSTANT_V_LONG = "calibrated_radar_constant_v_channel";
const char* Cf2RadxFile::RADAR_ESTIMATED_NOISE_DBM_HC_LONG = "estimated_noise_dbm_hc";
const char* Cf2RadxFile::RADAR_ESTIMATED_NOISE_DBM_HX_LONG = "estimated_noise_dbm_hx";
const char* Cf2RadxFile::RADAR_ESTIMATED_NOISE_DBM_VC_LONG = "estimated_noise_dbm_vc";
const char* Cf2RadxFile::RADAR_ESTIMATED_NOISE_DBM_VX_LONG = "estimated_noise_dbm_vx";
const char* Cf2RadxFile::RADAR_MEASURED_TRANSMIT_POWER_H_LONG = "measured_radar_transmit_power_h_channel";
const char* Cf2RadxFile::RADAR_MEASURED_TRANSMIT_POWER_V_LONG = "measured_radar_transmit_power_v_channel";
const char* Cf2RadxFile::RADAR_RX_BANDWIDTH_LONG = "radar_receiver_bandwidth";
const char* Cf2RadxFile::RANGE_CORRECTION_LONG = "range_to_center_of_measurement_volume_correction";
const char* Cf2RadxFile::RANGE_LONG = "range_to_center_of_measurement_volume";
const char* Cf2RadxFile::RAYS_ARE_INDEXED_LONG = "flag_for_indexed_rays";
const char* Cf2RadxFile::RAY_ANGLE_RES_LONG = "angular_resolution_between_rays";
const char* Cf2RadxFile::R_CALIB_INDEX_LONG = "calibration_data_array_index_per_ray";
const char* Cf2RadxFile::RECEIVER_GAIN_HC_LONG = "calibrated_radar_receiver_gain_h_co_polar_channel";
const char* Cf2RadxFile::RECEIVER_GAIN_HX_LONG = "calibrated_radar_receiver_gain_h_cross_polar_channel";
const char* Cf2RadxFile::RECEIVER_GAIN_VC_LONG = "calibrated_radar_receiver_gain_v_co_polar_channel";
const char* Cf2RadxFile::RECEIVER_GAIN_VX_LONG = "calibrated_radar_receiver_gain_v_cross_polar_channel";
const char* Cf2RadxFile::RECEIVER_MISMATCH_LOSS_LONG = "radar_calibration_receiver_mismatch_loss";
const char* Cf2RadxFile::RECEIVER_SLOPE_HC_LONG = "calibrated_radar_receiver_slope_h_co_polar_channel";
const char* Cf2RadxFile::RECEIVER_SLOPE_HX_LONG = "calibrated_radar_receiver_slope_h_cross_polar_channel";
const char* Cf2RadxFile::RECEIVER_SLOPE_VC_LONG = "calibrated_radar_receiver_slope_v_co_polar_channel";
const char* Cf2RadxFile::RECEIVER_SLOPE_VX_LONG = "calibrated_radar_receiver_slope_v_cross_polar_channel";
const char* Cf2RadxFile::ROLL_CHANGE_RATE_LONG = "platform_roll_angle_rate_of_change";
const char* Cf2RadxFile::ROLL_CORRECTION_LONG = "platform_roll_angle_correction";
const char* Cf2RadxFile::ROLL_LONG = "platform_roll_angle";
const char* Cf2RadxFile::ROTATION_CORRECTION_LONG = "ray_rotation_angle_relative_to_platform_correction";
const char* Cf2RadxFile::ROTATION_LONG = "ray_rotation_angle_relative_to_platform";
const char* Cf2RadxFile::SCAN_ID_LONG = "volume_coverage_pattern";
const char* Cf2RadxFile::SCAN_NAME_LONG = "name_of_antenna_scan_strategy";
const char* Cf2RadxFile::SCAN_RATE_LONG = "antenna_angle_scan_rate";
const char* Cf2RadxFile::SITE_NAME_LONG = "name_of_instrument_site";
const char* Cf2RadxFile::SPACING_IS_CONSTANT_LONG = "spacing_between_range_gates_is_constant";
const char* Cf2RadxFile::SPECTRUM_COPOLAR_HORIZONTAL = "spectrum_copolar_horizontal";
const char* Cf2RadxFile::SPECTRUM_COPOLAR_VERTICAL = "spectrum_copolar_vertical";
const char* Cf2RadxFile::SPECTRUM_CROSSPOLAR_HORIZONTAL = "spectrum_crosspolar_horizontal";
const char* Cf2RadxFile::SPECTRUM_CROSSPOLAR_VERTICAL = "spectrum_crosspolar_vertical";
const char* Cf2RadxFile::SUN_POWER_HC_LONG = "calibrated_radar_sun_power_h_co_polar_channel";
const char* Cf2RadxFile::SUN_POWER_HX_LONG = "calibrated_radar_sun_power_h_cross_polar_channel";
const char* Cf2RadxFile::SUN_POWER_VC_LONG = "calibrated_radar_sun_power_v_co_polar_channel";
const char* Cf2RadxFile::SUN_POWER_VX_LONG = "calibrated_radar_sun_power_v_cross_polar_channel";
const char* Cf2RadxFile::SWEEP_END_RAY_INDEX_LONG = "index_of_last_ray_in_sweep";
const char* Cf2RadxFile::SWEEP_FIXED_ANGLE_LONG = "fixed_angle_for_sweep";
const char* Cf2RadxFile::SWEEP_GROUP_NAME_LONG = "group_name_for_sweep";
const char* Cf2RadxFile::SWEEP_MODE_LONG = "scan_mode_for_sweep";
const char* Cf2RadxFile::SWEEP_NUMBER_LONG = "sweep_index_number_0_based";
const char* Cf2RadxFile::SWEEP_START_RAY_INDEX_LONG = "index_of_first_ray_in_sweep";
const char* Cf2RadxFile::SYSTEM_PHIDP_LONG = "calibrated_radar_system_phidp";
const char* Cf2RadxFile::TARGET_SCAN_RATE_LONG = "target_scan_rate_for_sweep";
const char* Cf2RadxFile::TEST_POWER_H_LONG = "radar_calibration_test_power_h_channel";
const char* Cf2RadxFile::TEST_POWER_V_LONG = "radar_calibration_test_power_v_channel";
const char* Cf2RadxFile::TILT_CORRECTION_LONG = "ray_tilt_angle_relative_to_platform_correction";
const char* Cf2RadxFile::TILT_LONG = "ray_tilt_angle_relative_to_platform";
const char* Cf2RadxFile::TIME_COVERAGE_END_LONG = "data_volume_end_time_utc";
const char* Cf2RadxFile::TIME_COVERAGE_START_LONG = "data_volume_start_time_utc";
const char* Cf2RadxFile::TIME_LONG = "radar_calibration_time_utc";
const char* Cf2RadxFile::TRACK_LONG = "platform_track_over_the_ground";
const char* Cf2RadxFile::TWO_WAY_RADOME_LOSS_H_LONG = "radar_calibration_two_way_radome_loss_h_channel";
const char* Cf2RadxFile::TWO_WAY_RADOME_LOSS_V_LONG = "radar_calibration_two_way_radome_loss_v_channel";
const char* Cf2RadxFile::TWO_WAY_WAVEGUIDE_LOSS_H_LONG = "radar_calibration_two_way_waveguide_loss_h_channel";
const char* Cf2RadxFile::TWO_WAY_WAVEGUIDE_LOSS_V_LONG = "radar_calibration_two_way_waveguide_loss_v_channel";
const char* Cf2RadxFile::UNAMBIGUOUS_RANGE_LONG = "unambiguous_range";
const char* Cf2RadxFile::VERTICAL_VELOCITY_CORRECTION_LONG = "platform_vertical_velocity_correction";
const char* Cf2RadxFile::VERTICAL_VELOCITY_LONG = "platform_vertical_velocity";
const char* Cf2RadxFile::VERTICAL_WIND_LONG = "upward_air_velocity";
const char* Cf2RadxFile::VOLUME_NUMBER_LONG = "data_volume_index_number";
const char* Cf2RadxFile::XMIT_POWER_H_LONG = "calibrated_radar_xmit_power_h_channel";
const char* Cf2RadxFile::XMIT_POWER_V_LONG = "calibrated_radar_xmit_power_v_channel";
const char* Cf2RadxFile::ZDR_CORRECTION_LONG = "calibrated_radar_zdr_correction";
