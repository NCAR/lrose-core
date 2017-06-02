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
// ForayNcRadxFile.cc
//
// ForayNcRadxFile object
//
// NetCDF file data for radar radial data in FORAY format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/ForayNcRadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
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

ForayNcRadxFile::ForayNcRadxFile() : RadxFile()
  
{

  _version = "1.0";

  _ncFormat = NETCDF_CLASSIC;

  _writeVol = NULL;
  _readVol = NULL;

  // _file = NULL;
  // _err = NULL;

  // _uds = NULL;
  // _udsEpoch = NULL;

  clear();

}

/////////////
// destructor

ForayNcRadxFile::~ForayNcRadxFile()

{
  clear();
  _file.close();
  // _udunitsFree();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void ForayNcRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _TimeDim = NULL;
  _maxCellsDim = NULL;
  _numSystemsDim = NULL;
  _fieldsDim = NULL;

  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;
  _writeFileNameMode = FILENAME_WITH_START_TIME_ONLY;

  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();

  _conventions.clear();
  _siteName.clear();
  _instrumentName.clear();
  _instrumentTypeStr.clear();
  _scanModeStr.clear();
  _projectName.clear();

  _dataIsUnsigned = false;

  _volumeNumber = Radx::missingMetaInt;
  _instrumentType = Radx::missingInstrumentType;
  _platformType = Radx::missingPlatformType;
  _sweepMode = Radx::missingSweepMode;

  _rangeKm.clear();
  _startRangeKm = Radx::missingMetaDouble;
  _gateSpacingKm = Radx::missingMetaDouble;
  _gateSpacingIsConstant = true;

  _sweepNums.clear();

  _Cell_Spacing_Method = 0;
  _calibration_data_present = 0;

  _Fixed_Angle = Radx::missingMetaDouble;
  _Range_to_First_Cell = Radx::missingMetaDouble;
  _Cell_Spacing = Radx::missingMetaDouble;
  _Nyquist_Velocity = Radx::missingMetaDouble;
  _Unambiguous_Range = Radx::missingMetaDouble;
  _Latitude = Radx::missingMetaDouble;
  _Longitude = Radx::missingMetaDouble;
  _Altitude = Radx::missingMetaDouble;
  _Radar_Constant = Radx::missingMetaDouble;
  _Wavelength = Radx::missingMetaDouble;
  _PRF = Radx::missingMetaDouble;

  _rcvr_gain = Radx::missingMetaDouble;
  _ant_gain = Radx::missingMetaDouble;
  _sys_gain = Radx::missingMetaDouble;
  _bm_width = Radx::missingMetaDouble;
  _pulse_width = Radx::missingMetaDouble;
  _band_width = Radx::missingMetaDouble;
  _peak_pwr = Radx::missingMetaDouble;
  _xmtr_pwr = Radx::missingMetaDouble;
  _noise_pwr = Radx::missingMetaDouble;
  _tst_pls_pwr = Radx::missingMetaDouble;
  _tst_pls_rng0 = Radx::missingMetaDouble;
  _tst_pls_rng1 = Radx::missingMetaDouble;

  _ant_gain_h_db = Radx::missingMetaDouble;
  _ant_gain_v_db = Radx::missingMetaDouble;
  _xmit_power_h_dbm = Radx::missingMetaDouble;
  _xmit_power_v_dbm = Radx::missingMetaDouble;
  _two_way_waveguide_loss_h_db = Radx::missingMetaDouble;
  _two_way_waveguide_loss_v_db = Radx::missingMetaDouble;
  _two_way_radome_loss_h_db = Radx::missingMetaDouble;
  _two_way_radome_loss_v_db = Radx::missingMetaDouble;
  _receiver_mismatch_loss_db = Radx::missingMetaDouble;
  _radar_constant_h = Radx::missingMetaDouble;
  _radar_constant_v = Radx::missingMetaDouble;
  _noise_hc_dbm = Radx::missingMetaDouble;
  _noise_vc_dbm = Radx::missingMetaDouble;
  _noise_hx_dbm = Radx::missingMetaDouble;
  _noise_vx_dbm = Radx::missingMetaDouble;
  _receiver_gain_hc_db = Radx::missingMetaDouble;
  _receiver_gain_vc_db = Radx::missingMetaDouble;
  _receiver_gain_hx_db = Radx::missingMetaDouble;
  _receiver_gain_vx_db = Radx::missingMetaDouble;
  _base_1km_hc_dbz = Radx::missingMetaDouble;
  _base_1km_vc_dbz = Radx::missingMetaDouble;
  _base_1km_hx_dbz = Radx::missingMetaDouble;
  _base_1km_vx_dbz = Radx::missingMetaDouble;
  _sun_power_hc_dbm = Radx::missingMetaDouble;
  _sun_power_vc_dbm = Radx::missingMetaDouble;
  _sun_power_hx_dbm = Radx::missingMetaDouble;
  _sun_power_vx_dbm = Radx::missingMetaDouble;
  _noise_source_power_h_dbm = Radx::missingMetaDouble;
  _noise_source_power_v_dbm = Radx::missingMetaDouble;
  _power_measure_loss_h_db = Radx::missingMetaDouble;
  _power_measure_loss_v_db = Radx::missingMetaDouble;
  _coupler_forward_loss_h_db = Radx::missingMetaDouble;
  _coupler_forward_loss_v_db = Radx::missingMetaDouble;
  _zdr_correction_db = Radx::missingMetaDouble;
  _ldr_correction_h_db = Radx::missingMetaDouble;
  _ldr_correction_v_db = Radx::missingMetaDouble;
  _system_phidp_deg = Radx::missingMetaDouble;

}

/////////////////////////////////////////////////////////
// Check if specified file is FORAY NC format
// Returns true if supported, false otherwise

bool ForayNcRadxFile::isSupported(const string &path)

{
  
  if (isForayNc(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a FORAY NC file
// Returns true on success, false on failure

bool ForayNcRadxFile::isForayNc(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    _addErrStr("ERROR - ForayNcRadxFile::isForayNc");
    _addErrStr(_file.getErrStr());
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    _addErrStr("ERROR - ForayNcRadxFile::isForayNc");
    return false;
  }

  // file has the correct dimensions, so it is a Foray NC file
  
  _file.close();
  return true;

}

////////////////////////////////////////////////
// get the date and time from a dorade file path
// returns 0 on success, -1 on failure

int ForayNcRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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

/////////////////////////////////////////////////////////
// print summary after read

void ForayNcRadxFile::print(ostream &out) const
  
{
  
  out << "=============== ForayNcRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "  title: " << _title << endl;
  out << "  institution: " << _institution << endl;
  out << "  references: " << _references << endl;
  out << "  source: " << _source << endl;
  out << "  history: " << _history << endl;
  out << "  comment: " << _comment << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  scanMode: " << _scanModeStr << endl;
  out << "  instrumentName: " << _instrumentName << endl;
  out << "  startTimeSecs: " << RadxTime::strm(_startTimeSecs) << endl;
  out << "  endTimeSecs: " << RadxTime::strm(_endTimeSecs) << endl;
  out << "  startNanoSecs: " << _startNanoSecs << endl;
  out << "  endNanoSecs: " << _endNanoSecs << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  latitude: " << _Latitude << endl;
  out << "  longitude: " << _Longitude << endl;
  out << "  altitude: " << _Altitude << endl;
  out << "  wavelength: " << _Wavelength << endl;
  out << "  startRangeKm: " << _startRangeKm << endl;
  out << "  gateSpacingKm: " << _gateSpacingKm << endl;
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  _geom.print(out);
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int ForayNcRadxFile::printNative(const string &path, ostream &out,
                                 bool printRays, bool printData)
  
{

  _addErrStr("ERROR - ForayNcRadxFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

////////////////////////////////////////
// convert Radx::DataType_t to Nc3Type

Nc3Type ForayNcRadxFile::_getNc3Type(Radx::DataType_t dtype)

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
  ForayNcRadxFile::_getFileFormat(RadxFile::netcdf_format_t format)

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

/////////////////////////////////////////////////////////////////////////////////////
// static string instantiation

const char* ForayNcRadxFile::ADD_OFFSET = "add_offset";
const char* ForayNcRadxFile::LONG_NAME = "long_name";
const char* ForayNcRadxFile::FILL_VALUE = "_FillValue";
const char* ForayNcRadxFile::MISSING_VALUE = "missing_value";
const char* ForayNcRadxFile::SCALE_FACTOR = "scale_factor";
const char* ForayNcRadxFile::SECS_SINCE_JAN1_1970 = "seconds since 1970-01-01T00:00:00Z";
const char* ForayNcRadxFile::STANDARD_NAME = "standard_name";
const char* ForayNcRadxFile::TIME = "Time";
const char* ForayNcRadxFile::TIME_OFFSET = "time_offset";
const char* ForayNcRadxFile::UNITS = "units";

const double ForayNcRadxFile::missingDouble = -9999.0;
const float ForayNcRadxFile::missingFloat = -9999.0F;
const int ForayNcRadxFile::missingInt = -9999;
