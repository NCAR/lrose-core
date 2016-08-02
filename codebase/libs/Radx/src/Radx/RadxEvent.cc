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
// RadxEvent.cc
//
// Field object for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxEvent.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxRay.hh>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cassert>
using namespace std;

/////////////////////////////////////////////////////////
// RadxEvent constructor

RadxEvent::RadxEvent()
  
{
  _init();
}

/////////////////////////////
// Copy constructor
//

RadxEvent::RadxEvent(const RadxEvent &rhs)
     
{
  _init();
  _copy(rhs);
}

/////////////////////////////////////////////////////////
// RadxEvent destructor

RadxEvent::~RadxEvent()
  
{

}

/////////////////////////////
// Assignment
//

RadxEvent &RadxEvent::operator=(const RadxEvent &rhs)
  
  
{
  return _copy(rhs);
}

//////////////////////////////////////////////////
// set events from ray flags

void RadxEvent::setFromRayFlags(const RadxRay &ray)
  
{

  _timeSecs = ray.getTimeSecs();
  _nanoSecs = ray.getNanoSecs();

  if (ray.getEventFlagsSet()) {
    _startOfSweep = ray.getStartOfSweepFlag();
    _endOfSweep = ray.getEndOfSweepFlag();
    _startOfVolume = ray.getStartOfVolumeFlag();
    _endOfVolume = ray.getEndOfVolumeFlag();
  } else {
    _startOfSweep = false;
    _endOfSweep = false;
    _startOfVolume = false;
    _endOfVolume = false;
  }

  _sweepMode = ray.getSweepMode();
  _followMode = ray.getFollowMode();

  _volumeNum = ray.getVolumeNumber();
  _sweepNum = ray.getSweepNumber();

  _cause = Radx::missingEventCause;

  _currentFixedAngle = ray.getFixedAngleDeg();

}

//////////////////////////////////////////////////
// initialize

void RadxEvent::_init()
  
{

  _timeSecs = 0;
  _nanoSecs = 0;

  _startOfSweep = false;
  _endOfSweep = false;

  _startOfVolume = false;
  _endOfVolume = false;

  _sweepMode = Radx::missingSweepMode;
  _followMode = Radx::missingFollowMode;

  _volumeNum = Radx::missingMetaInt;
  _sweepNum = Radx::missingMetaInt;

  _cause = Radx::missingEventCause;

  _currentFixedAngle = Radx::missingMetaDouble;

}

//////////////////////////////////////////////////
// clear the event flags

void RadxEvent::clearFlags()
  
{

  _startOfSweep = false;
  _endOfSweep = false;

  _startOfVolume = false;
  _endOfVolume = false;

  _cause = Radx::missingEventCause;

}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxEvent &RadxEvent::_copy(const RadxEvent &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  _timeSecs = rhs._timeSecs;
  _nanoSecs = rhs._nanoSecs;

  _startOfSweep = rhs._startOfSweep;
  _endOfSweep = rhs._endOfSweep;

  _startOfVolume = rhs._startOfVolume;
  _endOfVolume = rhs._endOfVolume;

  _sweepMode = rhs._sweepMode;
  _followMode = rhs._followMode;

  _volumeNum = rhs._volumeNum;
  _sweepNum = rhs._sweepNum;

  _cause = rhs._cause;

  _currentFixedAngle = rhs._currentFixedAngle;

  return *this;

}

/////////////////////////////////////////////////////////
// print

void RadxEvent::print(ostream &out) const
  
{
  
  out << "=============== RadxEvent ===============" << endl;
  out << "  timeSecs: " << RadxTime::strm(_timeSecs) << endl;
  out << "  nanoSecs: " << _nanoSecs << endl;
  out << "  startOfSweep: " << _startOfSweep << endl;
  out << "  endOfSweep: " << _endOfSweep << endl;
  out << "  startOfVolume: " << _startOfVolume << endl;
  out << "  endOfVolume: " << _endOfVolume << endl;
  out << "  sweepMode: "
      << Radx::sweepModeToStr(_sweepMode) << endl;
  out << "  followMode: "
      << Radx::followModeToStr(_followMode) << endl;
  out << "  volumeNum: " << _volumeNum << endl;
  out << "  sweepNum: " << _sweepNum << endl;
  out << "  cause: "
      << Radx::eventCauseToStr(_cause) << endl;
  out << "  currentFixedAngle: " << _currentFixedAngle << endl;
  out << "=========================================" << endl;
  
}

