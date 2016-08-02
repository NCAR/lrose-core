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
/**
 *
 * @file RadarMsg.cc
 *
 * @class RadarMsg
 *
 * RadarMsg handles filtering of radar data.
 *  
 * @date 4/5/2002
 *
 */

using namespace std;

#include <rapformats/DsRadarMsg.hh>

#include "RadarMsg.hh"
#include "ApRemoval.hh"


/**
 * Constructor
 */

RadarMsg::RadarMsg(const DsRadarMsg& radar_msg, const int content)
{
  _msg = new DsRadarMsg(radar_msg);
  _msgContent = content;
}


/**
 * Destructor
 */

RadarMsg::~RadarMsg()
{
  delete _msg;
}


/**
 * isFlags()
 */

bool RadarMsg::isFlags() const
{
  if (_msgContent & DsRadarMsg::RADAR_FLAGS)
    return true;
   
  return false;
}


/**
 * getTiltNum()
 */

int RadarMsg::getTiltNum() const
{
  if (isFlags())
    return _msg->getRadarFlags().tiltNum;
 
  return _msg->getRadarBeam().tiltNum;
}


/**
 * getVolumeNum()
 */

int RadarMsg::getVolumeNum() const
{
  if (isFlags())
    return _msg->getRadarFlags().volumeNum;
 
  return _msg->getRadarBeam().volumeNum;
}


/**
 * getTime()
 */

time_t RadarMsg::getTime() const
{
  if (isFlags())
    return _msg->getRadarFlags().time;
 
  return _msg->getRadarBeam().dataTime;
}


/**
 * getNumGates()
 */

int RadarMsg::getNumGates() const
{
  return _msg->getRadarParams().numGates;
}


/**
 * getGateSpacing()
 */

float RadarMsg::getGateSpacing() const
{
  return _msg->getRadarParams().gateSpacing;
}


/**
 * getStartRange()
 */

float RadarMsg::getStartRange() const
{
  return _msg->getRadarParams().startRange;
}


