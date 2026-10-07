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
// RadxNcfStr.cc
//
// Base class containing
// strings for NetCDF CF-compliant radar data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2018
//
///////////////////////////////////////////////////////////////
//
// Ncf File classes will inherit both RadxFile and this class
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxNcfStr.hh>

//////////////
// Constructor

RadxNcfStr::RadxNcfStr()
  
{

  // conventions

  CfConvention = "CF-1.7";
  BaseConvention = "CF-Radial";
  CurrentVersion = "CF-Radial-1.4";

  CfRadial2Conventions = "Cf/Radial";
  CfRadial2Version = "2.0";

  // create the set of ray variable names

  _createRayMetaNameSet();

}

/////////////
// destructor

RadxNcfStr::~RadxNcfStr()

{

}

//////////////////////////////////////////////////////////////////
// check if a variable is a standard ray metadat name

bool RadxNcfStr::isRayMetaName(const string &varName) const

{
  if (_rayMetaNames.find(varName) != _rayMetaNames.end()) {
    return true;
  } else {
    return false;
  }
}

//////////////////////////////////////////////////////////////////
// create set of ray variable names

void RadxNcfStr::_createRayMetaNameSet()

{
  if(_rayMetaNames.size() > 0) {
    // alread created
    return;
  }
  _rayMetaNames.insert(RadxNcfStr::TIME);
  _rayMetaNames.insert(RadxNcfStr::GEOREF_TIME);
  _rayMetaNames.insert(RadxNcfStr::LATITUDE);
  _rayMetaNames.insert(RadxNcfStr::LONGITUDE);
  _rayMetaNames.insert(RadxNcfStr::ALTITUDE);
  _rayMetaNames.insert(RadxNcfStr::ALTITUDE_AGL);
  _rayMetaNames.insert(RadxNcfStr::GEOREF_UNIT_NUM);
  _rayMetaNames.insert(RadxNcfStr::GEOREF_UNIT_ID);
  _rayMetaNames.insert(RadxNcfStr::EASTWARD_VELOCITY);
  _rayMetaNames.insert(RadxNcfStr::NORTHWARD_VELOCITY);
  _rayMetaNames.insert(RadxNcfStr::VERTICAL_VELOCITY);
  _rayMetaNames.insert(RadxNcfStr::HEADING);
  _rayMetaNames.insert(RadxNcfStr::ROLL);
  _rayMetaNames.insert(RadxNcfStr::PITCH);
  _rayMetaNames.insert(RadxNcfStr::DRIFT);
  _rayMetaNames.insert(RadxNcfStr::ROTATION);
  _rayMetaNames.insert(RadxNcfStr::TILT);
  _rayMetaNames.insert(RadxNcfStr::EASTWARD_WIND);
  _rayMetaNames.insert(RadxNcfStr::NORTHWARD_WIND);
  _rayMetaNames.insert(RadxNcfStr::VERTICAL_WIND);
  _rayMetaNames.insert(RadxNcfStr::HEADING_CHANGE_RATE);
  _rayMetaNames.insert(RadxNcfStr::PITCH_CHANGE_RATE);
  _rayMetaNames.insert(RadxNcfStr::DRIVE_ANGLE_1);
  _rayMetaNames.insert(RadxNcfStr::DRIVE_ANGLE_2);
  _rayMetaNames.insert(RadxNcfStr::TELESCOPE_ROLL_ANGLE_OFFSET);
  _rayMetaNames.insert(RadxNcfStr::ELEVATION);
  _rayMetaNames.insert(RadxNcfStr::AZIMUTH);
  _rayMetaNames.insert(RadxNcfStr::PULSE_WIDTH);
  _rayMetaNames.insert(RadxNcfStr::PRT);
  _rayMetaNames.insert(RadxNcfStr::PRT_RATIO);
  _rayMetaNames.insert(RadxNcfStr::NYQUIST_VELOCITY);
  _rayMetaNames.insert(RadxNcfStr::UNAMBIGUOUS_RANGE);
  _rayMetaNames.insert(RadxNcfStr::ANTENNA_TRANSITION);
  _rayMetaNames.insert(RadxNcfStr::GEOREFS_APPLIED);
  _rayMetaNames.insert(RadxNcfStr::N_SAMPLES);
  _rayMetaNames.insert(RadxNcfStr::R_CALIB_INDEX);
  _rayMetaNames.insert(RadxNcfStr::RADAR_MEASURED_TRANSMIT_POWER_H);
  _rayMetaNames.insert(RadxNcfStr::RADAR_MEASURED_TRANSMIT_POWER_V);
  _rayMetaNames.insert(RadxNcfStr::SCAN_RATE);
  _rayMetaNames.insert(RadxNcfStr::RADAR_ESTIMATED_NOISE_DBM_HC);
  _rayMetaNames.insert(RadxNcfStr::RADAR_ESTIMATED_NOISE_DBM_VC);
  _rayMetaNames.insert(RadxNcfStr::RADAR_ESTIMATED_NOISE_DBM_HX);
  _rayMetaNames.insert(RadxNcfStr::RADAR_ESTIMATED_NOISE_DBM_VX);
  _rayMetaNames.insert(RadxNcfStr::RAY_START_RANGE);
  _rayMetaNames.insert(RadxNcfStr::RAY_GATE_SPACING);
  _rayMetaNames.insert(RadxNcfStr::RAY_N_GATES);
  _rayMetaNames.insert(RadxNcfStr::RAY_START_INDEX);
  _rayMetaNames.insert(RadxNcfStr::TRACK_REL_ROT);
  _rayMetaNames.insert(RadxNcfStr::TRACK_REL_TILT);
  _rayMetaNames.insert(RadxNcfStr::TRACK_REL_AZ);
  _rayMetaNames.insert(RadxNcfStr::TRACK_REL_EL);

}
