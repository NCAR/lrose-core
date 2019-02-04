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
// Radx.cc
//
// Definitions for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#include <Radx/Radx.hh>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cmath>
using namespace std;

// initialize constants

const Radx::InstrumentType_t Radx::missingInstrumentType
  = Radx::INSTRUMENT_TYPE_RADAR;
const Radx::PlatformType_t Radx::missingPlatformType
  = Radx::PLATFORM_TYPE_FIXED;
const Radx::SweepMode_t Radx::missingSweepMode
  = Radx::SWEEP_MODE_NOT_SET;
const Radx::FollowMode_t Radx::missingFollowMode
  = Radx::FOLLOW_MODE_NONE;
const Radx::PolarizationMode_t Radx::missingPolarizationMode
  = Radx::POL_MODE_NOT_SET;
const Radx::PrtMode_t Radx::missingPrtMode
  = Radx::PRT_MODE_NOT_SET;
const Radx::EventCause_t Radx::missingEventCause
  = Radx::EVENT_CAUSE_NOT_SET;

const double Radx::LIGHT_SPEED = 2.99792458e8; // m/s
const double Radx::DegToRad = 0.01745329251994372;
const double Radx::RadToDeg = 57.29577951308092;

// missing values in meta-data

double Radx::missingMetaDouble = -9999.0;
float Radx::missingMetaFloat = -9999.0;
int Radx::missingMetaInt = -9999;
char Radx::missingMetaChar = -128;

// missing values in field data

Radx::fl64 Radx::missingFl64 = -9.0e33;
Radx::fl32 Radx::missingFl32 = -9.0e33f;
Radx::si32 Radx::missingSi32 = -2147483647;
Radx::si16 Radx::missingSi16 = -32768;
Radx::si08 Radx::missingSi08 = -128;

// standard strings

const char* Radx::AIRCRAFT = "aircraft";
const char* Radx::AIRCRAFT_AFT = "aircraft_aft";
const char* Radx::AIRCRAFT_BELLY = "aircraft_belly";
const char* Radx::AIRCRAFT_FORE = "aircraft_fore";
const char* Radx::AIRCRAFT_NOSE = "aircraft_nose";
const char* Radx::AIRCRAFT_ROOF = "aircraft_roof";
const char* Radx::AIRCRAFT_TAIL = "aircraft_tail";
const char* Radx::AXIS_X = "axis_x";
const char* Radx::AXIS_X_PRIME = "axis_x_prime";
const char* Radx::AXIS_Y = "axis_y";
const char* Radx::AXIS_Y_PRIME = "axis_y_prime";
const char* Radx::AXIS_Z = "axis_z";
const char* Radx::AXIS_Z_PRIME = "axis_z_prime";
const char* Radx::AZIMUTH_SURVEILLANCE = "azimuth_surveillance";
const char* Radx::CALIBRATION = "calibration";
const char* Radx::CIRCULAR = "circular";
const char* Radx::COMPLEX_TRAJECTORY = "complex_trajectory";
const char* Radx::COPLANE = "coplane";
const char* Radx::DOPPLER_BEAM_SWINGING = "doppler_beam_swinging";
const char* Radx::DUAL = "dual";
const char* Radx::ELECTRONIC_STEERING = "electronic_steering";
const char* Radx::ELEVATION_SURVEILLANCE = "elevation_surveillance";
const char* Radx::FIXED = "fixed";
const char* Radx::HORIZONTAL = "horizontal";
const char* Radx::HV_ALT = "hv_alt";
const char* Radx::HV_SIM = "hv_sim";
const char* Radx::HV_H_XMIT = "hv_h_xmit";
const char* Radx::HV_V_XMIT = "hv_v_xmit";
const char* Radx::IDLE = "idle";
const char* Radx::LIDAR = "lidar";
const char* Radx::MANUAL = "manual";
const char* Radx::MANUAL_PPI = "manual_ppi";
const char* Radx::MANUAL_RHI = "manual_rhi";
const char* Radx::NONE = "none";
const char* Radx::NOT_SET = "not_set";
const char* Radx::POINTING = "pointing";
const char* Radx::RADAR = "radar";
const char* Radx::RHI = "rhi";
const char* Radx::SATELLITE_GEOSTAT = "satellite_geostat";
const char* Radx::SATELLITE_ORBIT = "satellite_orbit";
const char* Radx::SECTOR = "sector";
const char* Radx::SHIP = "ship";
const char* Radx::STAGGERED = "staggered";
const char* Radx::SUN = "sun";
const char* Radx::SUNSCAN = "sunscan";
const char* Radx::SUNSCAN_RHI = "sunscan_rhi";
const char* Radx::TARGET = "target";
const char* Radx::UNKNOWN = "unknown";
const char* Radx::VEHICLE = "vehicle";
const char* Radx::VERTICAL = "vertical";
const char* Radx::VERTICAL_POINTING = "vertical_pointing";

const char* Radx::EVENT_DONE = "EVENT_DONE";
const char* Radx::EVENT_TIMEOUT = "EVENT_TIMEOUT";
const char* Radx::EVENT_TIMER = "EVENT_TIMER";
const char* Radx::EVENT_ABORT = "EVENT_ABORT";
const char* Radx::EVENT_SCAN_ABORT = "EVENT_SCAN_ABORT";
const char* Radx::EVENT_RESTART = "EVENT_RESTART";
const char* Radx::EVENT_SCAN_STATE_TIMEOUT = "EVENT_SCAN_STATE_TIMEOUT";

///////////////////////////////
// get byte width of data type

int Radx::getByteWidth(DataType_t dtype)

{

  switch (dtype) {
    case FL64:
      return sizeof(fl64);
    case FL32:
      return sizeof(fl32);
    case SI32:
      return sizeof(si32);
    case SI16:
      return sizeof(si16);
    case SI08:
    default:
      return sizeof(si08);
  }

}

///////////////////////////////////////////
// convert enums to strings

string Radx::dataTypeToStr(DataType_t dtype)

{

  switch (dtype) {
    case FL64:
      return "fl64";
    case FL32:
      return "fl32";
    case SI32:
      return "si32";
    case SI16:
      return "si16";
    case SI08:
    default:
      return "si08";
  }
  
}

string Radx::instrumentTypeToStr(InstrumentType_t ptype)
{

  switch (ptype) {
    case INSTRUMENT_TYPE_RADAR: {
      return Radx::RADAR;
    }
    case INSTRUMENT_TYPE_LIDAR: {
      return Radx::LIDAR;
    }
    default: {
      return Radx::UNKNOWN;
    }
  }

}

Radx::InstrumentType_t Radx::instrumentTypeFromStr(const string &str)
{

  if (str.find(Radx::RADAR) != string::npos) {
    return INSTRUMENT_TYPE_RADAR;
  }
  if (str.find(Radx::LIDAR) != string::npos) {
    return INSTRUMENT_TYPE_LIDAR;
  }
  return INSTRUMENT_TYPE_RADAR;

}

string Radx::instrumentTypeOptions()
{

  string options;
  options += + Radx::RADAR;
  options += ", ";
  options += Radx::LIDAR;
  return options;

}

string Radx::primaryAxisToStr(PrimaryAxis_t ptype)
{

  switch (ptype) {
    case PRIMARY_AXIS_Y_PRIME: {
      return Radx::AXIS_Y_PRIME;
    }
    case PRIMARY_AXIS_Y: {
      return Radx::AXIS_Y;
    }
    case PRIMARY_AXIS_X_PRIME: {
      return Radx::AXIS_X_PRIME;
    }
    case PRIMARY_AXIS_X: {
      return Radx::AXIS_X;
    }
    case PRIMARY_AXIS_Z_PRIME: {
      return Radx::AXIS_Z_PRIME;
    }
    case PRIMARY_AXIS_Z:
    default: {
      return Radx::AXIS_Z;
    }
  }

}

Radx::PrimaryAxis_t Radx::primaryAxisFromStr(const string &str)
{

  if (str.find(Radx::AXIS_Z_PRIME) != string::npos) {
    return PRIMARY_AXIS_Z_PRIME;
  }
  if (str.find(Radx::AXIS_Y_PRIME) != string::npos) {
    return PRIMARY_AXIS_Y_PRIME;
  }
  if (str.find(Radx::AXIS_X_PRIME) != string::npos) {
    return PRIMARY_AXIS_X_PRIME;
  }
  if (str.find(Radx::AXIS_Z) != string::npos) {
    return PRIMARY_AXIS_Z;
  }
  if (str.find(Radx::AXIS_Y) != string::npos) {
    return PRIMARY_AXIS_Y;
  }
  if (str.find(Radx::AXIS_X) != string::npos) {
    return PRIMARY_AXIS_X;
  }
  return PRIMARY_AXIS_Z;

}

string Radx::primaryAxisOptions()
{
  
  string options;
  options += + Radx::AXIS_Z;
  options += ", ";
  options += Radx::AXIS_Y;
  options += ", ";
  options += Radx::AXIS_X;
  options += ", ";
  options += Radx::AXIS_Z_PRIME;
  options += ", ";
  options += Radx::AXIS_Y_PRIME;
  options += ", ";
  options += Radx::AXIS_X_PRIME;
  return options;

}

string Radx::platformTypeToStr(PlatformType_t ptype)
{

  switch (ptype) {
    case PLATFORM_TYPE_NOT_SET: {
      return Radx::NOT_SET;
    }
    case PLATFORM_TYPE_FIXED: {
      return Radx::FIXED;
    }
    case PLATFORM_TYPE_VEHICLE: {
      return Radx::VEHICLE;
    }
    case PLATFORM_TYPE_SHIP: {
      return Radx::SHIP;
    }
    case PLATFORM_TYPE_AIRCRAFT: {
      return Radx::AIRCRAFT;
    }
    case PLATFORM_TYPE_AIRCRAFT_FORE: {
      return Radx::AIRCRAFT_FORE;
    }
    case PLATFORM_TYPE_AIRCRAFT_AFT: {
      return Radx::AIRCRAFT_AFT;
    }
    case PLATFORM_TYPE_AIRCRAFT_TAIL: {
      return Radx::AIRCRAFT_TAIL;
    }
    case PLATFORM_TYPE_AIRCRAFT_BELLY: {
      return Radx::AIRCRAFT_BELLY;
    }
    case PLATFORM_TYPE_AIRCRAFT_ROOF: {
      return Radx::AIRCRAFT_ROOF;
    }
    case PLATFORM_TYPE_AIRCRAFT_NOSE: {
      return Radx::AIRCRAFT_NOSE;
    }
    case PLATFORM_TYPE_SATELLITE_ORBIT: {
      return Radx::SATELLITE_ORBIT;
    }
    case PLATFORM_TYPE_SATELLITE_GEOSTAT: {
      return Radx::SATELLITE_GEOSTAT;
    }
    default: {
      return Radx::UNKNOWN;
    }
  }

}

Radx::PlatformType_t Radx::platformTypeFromStr(const string &str)
{

  if (str.find(Radx::FIXED) != string::npos) {
    return PLATFORM_TYPE_FIXED;
  }
  if (str.find(Radx::VEHICLE) != string::npos) {
    return PLATFORM_TYPE_VEHICLE;
  }
  if (str.find(Radx::SHIP) != string::npos) {
    return PLATFORM_TYPE_SHIP;
  }
  if (str.find(Radx::AIRCRAFT_FORE) != string::npos) {
    return PLATFORM_TYPE_AIRCRAFT_FORE;
  }
  if (str.find(Radx::AIRCRAFT_AFT) != string::npos) {
    return PLATFORM_TYPE_AIRCRAFT_AFT;
  }
  if (str.find(Radx::AIRCRAFT_TAIL) != string::npos) {
    return PLATFORM_TYPE_AIRCRAFT_TAIL;
  }
  if (str.find(Radx::AIRCRAFT_BELLY) != string::npos) {
    return PLATFORM_TYPE_AIRCRAFT_BELLY;
  }
  if (str.find(Radx::AIRCRAFT_ROOF) != string::npos) {
    return PLATFORM_TYPE_AIRCRAFT_ROOF;
  }
  if (str.find(Radx::AIRCRAFT_NOSE) != string::npos) {
    return PLATFORM_TYPE_AIRCRAFT_NOSE;
  }
  if (str.find(Radx::AIRCRAFT) != string::npos) {
    return PLATFORM_TYPE_AIRCRAFT;
  }
  if (str.find(Radx::SATELLITE_ORBIT) != string::npos) {
    return PLATFORM_TYPE_SATELLITE_ORBIT;
  }
  if (str.find(Radx::SATELLITE_GEOSTAT) != string::npos) {
    return PLATFORM_TYPE_SATELLITE_GEOSTAT;
  }
  return PLATFORM_TYPE_NOT_SET;

}

string Radx::platformTypeOptions()
{

  string options;
  options += + Radx::FIXED;
  options += ", ";
  options += Radx::VEHICLE;
  options += ", ";
  options += Radx::SHIP;
  options += ", ";
  options += Radx::AIRCRAFT_FORE;
  options += ", ";
  options += Radx::AIRCRAFT_AFT;
  options += ", ";
  options += Radx::AIRCRAFT_TAIL;
  options += ", ";
  options += Radx::AIRCRAFT_BELLY;
  options += ", ";
  options += Radx::AIRCRAFT_ROOF;
  options += ", ";
  options += Radx::AIRCRAFT_NOSE;
  options += ", ";
  options += Radx::SATELLITE_ORBIT;
  options += ", ";
  options += Radx::SATELLITE_GEOSTAT;
  return options;

}

string Radx::sweepModeToStr(SweepMode_t mode)
{

  switch (mode) {
    case SWEEP_MODE_NOT_SET: {
      return Radx::NOT_SET;
    }
    case SWEEP_MODE_SECTOR: {
      return Radx::SECTOR;
    }
    case SWEEP_MODE_COPLANE: {
      return Radx::COPLANE;
    }
    case SWEEP_MODE_RHI: {
      return Radx::RHI;
    }
    case SWEEP_MODE_VERTICAL_POINTING: {
      return Radx::VERTICAL_POINTING;
    }
    case SWEEP_MODE_IDLE: {
      return Radx::IDLE;
    }
    case SWEEP_MODE_AZIMUTH_SURVEILLANCE: {
      return Radx::AZIMUTH_SURVEILLANCE;
    }
    case SWEEP_MODE_ELEVATION_SURVEILLANCE: {
      return Radx::ELEVATION_SURVEILLANCE;
    }
    case SWEEP_MODE_SUNSCAN: {
      return Radx::SUNSCAN;
    }
    case SWEEP_MODE_POINTING: {
      return Radx::POINTING;
    }
    case SWEEP_MODE_CALIBRATION: {
      return Radx::CALIBRATION;
    }
    case SWEEP_MODE_MANUAL_PPI: {
      return Radx::MANUAL_PPI;
    }
    case SWEEP_MODE_MANUAL_RHI: {
      return Radx::MANUAL_RHI;
    }
    case SWEEP_MODE_SUNSCAN_RHI: {
      return Radx::SUNSCAN_RHI;
    }
    case SWEEP_MODE_DOPPLER_BEAM_SWINGING: {
      return Radx::DOPPLER_BEAM_SWINGING;
    }
    case SWEEP_MODE_COMPLEX_TRAJECTORY: {
      return Radx::COMPLEX_TRAJECTORY;
    }
    case SWEEP_MODE_ELECTRONIC_STEERING: {
      return Radx::ELECTRONIC_STEERING;
    }
    default: {
      return Radx::UNKNOWN;
    }
  }
}

string Radx::sweepModeToShortStr(SweepMode_t mode)
{

  switch (mode) {
    case SWEEP_MODE_NOT_SET: {
      return "XXX";
    }
    case SWEEP_MODE_SECTOR: {
      return "PPI";
    }
    case SWEEP_MODE_COPLANE: {
      return "COP";
    }
    case SWEEP_MODE_RHI: {
      return "RHI";
    }
    case SWEEP_MODE_VERTICAL_POINTING: {
      return "VER";
    }
    case SWEEP_MODE_IDLE: {
      return "IDL";
    }
    case SWEEP_MODE_AZIMUTH_SURVEILLANCE: {
      return "SUR";
    }
    case SWEEP_MODE_ELEVATION_SURVEILLANCE: {
      return "AIR";
    }
    case SWEEP_MODE_SUNSCAN: {
      return "SUN";
    }
    case SWEEP_MODE_SUNSCAN_RHI: {
      return "SRH";
    }
    case SWEEP_MODE_CALIBRATION: {
      return "CAL";
    }
    case SWEEP_MODE_POINTING:
    case SWEEP_MODE_MANUAL_PPI:
    case SWEEP_MODE_MANUAL_RHI: {
      return "MAN";
    }
    case SWEEP_MODE_DOPPLER_BEAM_SWINGING: {
      return "DBS";
    }
    case SWEEP_MODE_COMPLEX_TRAJECTORY: {
      return "TRJ";
    }
    case SWEEP_MODE_ELECTRONIC_STEERING: {
      return "PAR";
    }
    default: {
      return "SUR";
    }
  }
}

///////////////////////////////////////////
// convert strings to enums

Radx::DataType_t Radx::dataTypeFromStr(const string &str)

{
  
  if (str == "fl64") {
    return FL64;
  }
  if (str == "fl32") {
    return FL32;
  }
  if (str == "si32") {
    return SI32;
  }
  if (str == "si16") {
    return SI16;
  }
  if (str == "si08") {
    return SI08;
  }

  // default

  return FL32;

}

Radx::SweepMode_t Radx::sweepModeFromStr(const string &str)
{

  if (str.find(Radx::SECTOR) != string::npos) {
    return SWEEP_MODE_SECTOR;
  }
  if (str.find(Radx::COPLANE) != string::npos) {
    return SWEEP_MODE_COPLANE;
  }
  if (str.find(Radx::RHI) != string::npos) {
    return SWEEP_MODE_RHI;
  }
  if (str.find(Radx::VERTICAL_POINTING) != string::npos) {
    return SWEEP_MODE_VERTICAL_POINTING;
  }
  if (str.find(Radx::IDLE) != string::npos) {
    return SWEEP_MODE_IDLE;
  }
  if (str.find(Radx::AZIMUTH_SURVEILLANCE) != string::npos) {
    return SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }
  if (str.find(Radx::ELEVATION_SURVEILLANCE) != string::npos) {
    return SWEEP_MODE_ELEVATION_SURVEILLANCE;
  }
  if (str.find(Radx::SUNSCAN) != string::npos) {
    return SWEEP_MODE_SUNSCAN;
  }
  if (str.find(Radx::POINTING) != string::npos) {
    return SWEEP_MODE_POINTING;
  }
  if (str.find(Radx::CALIBRATION) != string::npos) {
    return SWEEP_MODE_CALIBRATION;
  }
  if (str.find(Radx::MANUAL_PPI) != string::npos) {
    return SWEEP_MODE_MANUAL_PPI;
  }
  if (str.find(Radx::MANUAL_RHI) != string::npos) {
    return SWEEP_MODE_MANUAL_RHI;
  }
  if (str.find(Radx::SUNSCAN_RHI) != string::npos) {
    return SWEEP_MODE_SUNSCAN_RHI;
  }
  if (str.find(Radx::DOPPLER_BEAM_SWINGING) != string::npos) {
    return SWEEP_MODE_DOPPLER_BEAM_SWINGING;
  }
  if (str.find(Radx::COMPLEX_TRAJECTORY) != string::npos) {
    return SWEEP_MODE_COMPLEX_TRAJECTORY;
  }
  if (str.find(Radx::ELECTRONIC_STEERING) != string::npos) {
    return SWEEP_MODE_ELECTRONIC_STEERING;
  }
  return SWEEP_MODE_NOT_SET;
}

string Radx::sweepModeOptions()
{
  string options;
  options += Radx::SECTOR;
  options += ", ";
  options += Radx::COPLANE;
  options += ", ";
  options += Radx::RHI;
  options += ", ";
  options += Radx::VERTICAL_POINTING;
  options += ", ";
  options += Radx::IDLE;
  options += ", ";
  options += Radx::AZIMUTH_SURVEILLANCE;
  options += ", ";
  options += Radx::ELEVATION_SURVEILLANCE;
  options += ", ";
  options += Radx::SUNSCAN;
  options += ", ";
  options += Radx::POINTING;
  options += ", ";
  options += Radx::CALIBRATION;
  options += ", ";
  options += Radx::MANUAL_PPI;
  options += ", ";
  options += Radx::MANUAL_RHI;
  options += ", ";
  options += Radx::SUNSCAN_RHI;
  options += ", ";
  options += Radx::DOPPLER_BEAM_SWINGING;
  options += ", ";
  options += Radx::COMPLEX_TRAJECTORY;
  options += ", ";
  options += Radx::ELECTRONIC_STEERING;
  return options;
}

string Radx::followModeToStr(FollowMode_t mode)

{

  switch (mode) {
    case FOLLOW_MODE_NOT_SET: {
      return Radx::NOT_SET;
    }
    case FOLLOW_MODE_NONE: {
      return Radx::NONE;
    }
    case FOLLOW_MODE_SUN: {
      return Radx::SUN;
    }
    case FOLLOW_MODE_VEHICLE: {
      return Radx::VEHICLE;
    }
    case FOLLOW_MODE_AIRCRAFT: {
      return Radx::AIRCRAFT;
    }
    case FOLLOW_MODE_TARGET: {
      return Radx::TARGET;
    }
    case FOLLOW_MODE_MANUAL: {
      return Radx::MANUAL;
    }
    default: {
      return Radx::UNKNOWN;
    }
  }
}

Radx::FollowMode_t Radx::followModeFromStr(const string &str)

{

  if (str == Radx::NONE) {
    return FOLLOW_MODE_NONE;
  }
  if (str == Radx::SUN) {
    return FOLLOW_MODE_SUN;
  }
  if (str == Radx::VEHICLE) {
    return FOLLOW_MODE_VEHICLE;
  }
  if (str == Radx::AIRCRAFT) {
    return FOLLOW_MODE_AIRCRAFT;
  }
  if (str == Radx::TARGET) {
    return FOLLOW_MODE_TARGET;
  }
  if (str == Radx::MANUAL) {
    return FOLLOW_MODE_MANUAL;
  }
  return FOLLOW_MODE_NOT_SET;
}

string Radx::followModeOptions()

{
  string options;
  options += Radx::NONE;
  options += ", ";
  options += Radx::SUN;
  options += ", ";
  options += Radx::VEHICLE;
  options += ", ";
  options += Radx::AIRCRAFT;
  options += ", ";
  options += Radx::TARGET;
  options += ", ";
  options += Radx::MANUAL;
  return options;
}

string Radx::polarizationModeToStr(PolarizationMode_t mode)

{

  switch (mode) {
    case POL_MODE_NOT_SET: {
      return Radx::NOT_SET;
    }
    case POL_MODE_HORIZONTAL: {
      return Radx::HORIZONTAL;
    }
    case POL_MODE_VERTICAL: {
      return Radx::VERTICAL;
    }
    case POL_MODE_HV_ALT: {
      return Radx::HV_ALT;
    }
    case POL_MODE_HV_SIM: {
      return Radx::HV_SIM;
    }
    case POL_MODE_HV_H_XMIT: {
      return Radx::HV_H_XMIT;
    }
    case POL_MODE_HV_V_XMIT: {
      return Radx::HV_V_XMIT;
    }
    case POL_MODE_CIRCULAR: {
      return Radx::CIRCULAR;
    }
    default: {
      return Radx::UNKNOWN;
    }
  }
}

Radx::PolarizationMode_t Radx::polarizationModeFromStr(const string &str)

{

  if (str == Radx::HORIZONTAL) {
    return POL_MODE_HORIZONTAL;
  }
  if (str == Radx::VERTICAL) {
    return POL_MODE_VERTICAL;
  }
  if (str == Radx::HV_ALT) {
    return POL_MODE_HV_ALT;
  }
  if (str == Radx::HV_SIM) {
    return POL_MODE_HV_SIM;
  }
  if (str == Radx::HV_H_XMIT) {
    return POL_MODE_HV_H_XMIT;
  }
  if (str == Radx::HV_V_XMIT) {
    return POL_MODE_HV_V_XMIT;
  }
  if (str == Radx::CIRCULAR) {
    return POL_MODE_CIRCULAR;
  }
  return POL_MODE_NOT_SET;
}

string Radx::polarizationModeOptions()

{
  string options;
  options += Radx::HORIZONTAL;
  options += ", ";
  options += Radx::VERTICAL;
  options += ", ";
  options += Radx::HV_ALT;
  options += ", ";
  options += Radx::HV_SIM;
  options += ", ";
  options += Radx::CIRCULAR;
  return options;
}

string Radx::prtModeToStr(PrtMode_t mode)

{

  switch (mode) {
    case PRT_MODE_NOT_SET: {
      return Radx::NOT_SET;
    }
    case PRT_MODE_FIXED: {
      return Radx::FIXED;
    }
    case PRT_MODE_STAGGERED: {
      return Radx::STAGGERED;
    }
    case PRT_MODE_DUAL: {
      return Radx::DUAL;
    }
    default: {
      return Radx::UNKNOWN;
    }
  }
}

Radx::PrtMode_t Radx::prtModeFromStr(const string &str)

{
  if (str == Radx::FIXED) {
    return PRT_MODE_FIXED;
  }
  if (str == Radx::STAGGERED) {
    return PRT_MODE_STAGGERED;
  }
  if (str == Radx::DUAL) {
    return PRT_MODE_DUAL;
  }
  return PRT_MODE_NOT_SET;
}

string Radx::prtModeOptions()

{
  string options;
  options += Radx::FIXED;
  options += ", ";
  options += Radx::STAGGERED;
  options += ", ";
  options += Radx::DUAL;
  return options;
}

string Radx::eventCauseToStr(EventCause_t cause)

{
  switch (cause) {
    case EVENT_CAUSE_DONE: {
      return Radx::EVENT_DONE;
    }
    case EVENT_CAUSE_TIMEOUT: {
      return Radx::EVENT_TIMEOUT;
    }
    case EVENT_CAUSE_TIMER: {
      return Radx::EVENT_TIMER;
    }
    case EVENT_CAUSE_ABORT: {
      return Radx::EVENT_ABORT;
    }
    case EVENT_CAUSE_SCAN_ABORT: {
      return Radx::EVENT_SCAN_ABORT;
    }
    case EVENT_CAUSE_RESTART: {
      return Radx::EVENT_RESTART;
    }
    case EVENT_CAUSE_SCAN_STATE_TIMEOUT: {
      return Radx::EVENT_SCAN_STATE_TIMEOUT;
    }
    default: {
      return Radx::UNKNOWN;
    }
  }
}

Radx::EventCause_t Radx::eventCauseFromStr(const string &str)

{

  if (str == Radx::EVENT_DONE) {
    return EVENT_CAUSE_DONE;
  }
  if (str == Radx::EVENT_TIMEOUT) {
    return EVENT_CAUSE_TIMEOUT;
  }
  if (str == Radx::EVENT_TIMER) {
    return EVENT_CAUSE_TIMER;
  }
  if (str == Radx::EVENT_ABORT) {
    return EVENT_CAUSE_ABORT;
  }
  if (str == Radx::EVENT_SCAN_ABORT) {
    return EVENT_CAUSE_SCAN_ABORT;
  }
  if (str == Radx::EVENT_RESTART) {
    return EVENT_CAUSE_RESTART;
  }
  if (str == Radx::EVENT_SCAN_STATE_TIMEOUT) {
    return EVENT_CAUSE_SCAN_STATE_TIMEOUT;
  }

  return EVENT_CAUSE_NOT_SET;

}

string Radx::eventCauseOptions()

{
  string options;
  options += EVENT_DONE;
  options += ", ";
  options += EVENT_TIMEOUT;
  options += ", ";
  options += EVENT_TIMER;
  options += ", ";
  options += EVENT_ABORT;
  options += ", ";
  options += EVENT_SCAN_ABORT;
  options += ", ";
  options += EVENT_RESTART;
  options += ", ";
  options += EVENT_SCAN_STATE_TIMEOUT;
  return options;
}

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void Radx::addErrInt(string &errStr, string label,
                     int iarg, bool cr)
{
  errStr += label;
  char str[32];
  sprintf(str, "%d", iarg);
  errStr += str;
  if (cr) {
    errStr += "\n";
  }
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void Radx::addErrDbl(string &errStr,
                     string label, double darg,
                     string format, bool cr)
  
{
  errStr += label;
  char str[128];
  sprintf(str, format.c_str(), darg);
  errStr += str;
  if (cr) {
    errStr += "\n";
  }
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void Radx::addErrStr(string &errStr, string label,
                     string strarg, bool cr)

{
  errStr += label;
  errStr += strarg;
  if (cr) {
    errStr += "\n";
  }
}

/////////////////////////////
// make string from char text
// Ensure null termination

string Radx::makeString(const char *text, int len)
  
{

  char *copy = new char[len + 1];
  memcpy(copy, text, len);
  // force null termination
  copy[len] = '\0';
  // remove trailing spaces or non-printable characters
  for (int ii = len - 1; ii >= 0; ii--) {
    char cc = copy[ii];
    if (!isprint(cc) || isspace(cc)) {
      copy[ii] = '\0';
    } else {
      break;
    }
  }
  // convert to string
  string str(copy);
  delete[] copy;
  return str;

}

//////////////////////////////////////////////
// replace spaces in a string with underscores

void Radx::replaceSpacesWithUnderscores(string &str)

{

  for (size_t ii = 0; ii < str.size(); ii++) {
    if (str[ii] == ' ') {
      str[ii] = '_';
    }
  }

}

///////////////////////////
// safe print for char text
// Ensure null termination

void Radx::printString(const string &label, const char *text,
                       int len, ostream &out)
  
{
  out << "  " << label << ": " << makeString(text, len) << endl;
}

/////////////////////////////////
/// compute sin and cos together

void Radx::sincos(double radians, double &sinVal, double &cosVal)

{
  
  double cosv, sinv, interval;

  // compute cosine
  
  cosv = cos(radians);
  cosVal = cosv;

  // compute sine magnitude

  sinv = sqrt(1.0 - cosv * cosv);
  
  // set sine sign from location relative to PI

  interval = floor(radians / M_PI);
  if (fabs(fmod(interval, 2.0)) == 0) {
    sinVal = sinv;
  } else {
    sinVal = -1.0 * sinv;
  }

}

/////////////////////////////////////
/// condition az to between 0 and 360

double Radx::conditionAz(double az)

{

  while (az < 0.0) {
    az += 360.0;
  }
  while (az > 360.0) {
    az -= 360.0;
  }

  return az;

}

///////////////////////////////////////////////////
/// condition el to between -180 and 180

double Radx::conditionEl(double el)

{

  while (el < -180.0) {
    el += 360.0;
  }
  while (el > 180.0) {
    el -= 360.0;
  }

  return el;

}

///////////////////////////////////////////////////
/// condition angle delta to between -180 and 180

double Radx::conditionAngleDelta(double delta)

{

  if (delta < -180.0) {
    delta += 360.0;
  } else if (delta > 180.0) {
    delta -= 360.0;
  }

  return delta;

}

///////////////////////////////////////////////////
/// compute diff between 2 angles: (ang1 - ang2)

double Radx::computeAngleDiff(double ang1, double ang2)

{
  double delta = Radx::conditionAngleDelta(ang1 - ang2);
  return delta;
}

////////////////////////////////////////////////////////////
///
/// compute mean of 2 angles - ang1 + ((ang2 - ang1)/2)

double Radx::computeAngleMean(double ang1, double ang2)

{
  double delta = Radx::conditionAngleDelta(ang2 - ang1);
  double mean = ang1 + delta / 2.0;
  if (ang1 > 180 || ang2 > 180) {
    mean = Radx::conditionAz(mean);
  } else {
    mean = Radx::conditionEl(mean);
  }
  return mean;
}


  
