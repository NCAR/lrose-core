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
// AcSim.cc
//
// AcSim file class -  - aircraft simulator
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2015
//
/////////////////////////////////////////////////////////////

#include "AcSim.hh"
#include <toolsa/pjg.h>
#include <toolsa/uusleep.h>
#include <rapmath/stats.h>
#include <toolsa/DateTime.hh>
using namespace std;

AcSim::AcSim (const string &callsign,
	      double start_lat, double start_lon,
	      double altitude, double speed)
  
{

  // initialize up random number generator
  
  time_t now = time(NULL);
  STATS_uniform_seed(now);
  
  // initialize members
  
  _startLat = start_lat;
  _startLon = start_lon;
  _altitude = altitude * 0.3048;
  _speed = (speed * 1.852) / 3.6;
  _callsign = callsign;
  _changeTime = 0;
  _lat = _startLat;
  _lon = _startLon;
  _timeLast = now;
  _heading = STATS_uniform_gen() * 360.0;
  _headBias = 0.0;
  
}

AcSim::~AcSim ()

{

}

string AcSim::getNextPos()
  
{
  
  // compute next position
  
  _computeNext();
  
  // load the basic string
  
  char str[1024];
  DateTime  dtime(_timeLast);
  sprintf(str,
          "{\"timestamp\":\"%.4d-%.2d-%.2d %.2d:%.2d:%.2d\","
          "\"alt\":\"%g\","
          "\"lat\":\"%g\","
          "\"head\":\"%g\","
          "\"declination\":\"0\","
          "\"lon\":\"%g\"",
	  dtime.getYear(),
	  dtime.getMonth(),
	  dtime.getDay(),
	  dtime.getHour(),
	  dtime.getMin(),
	  dtime.getSec(),
          _altitude,
	  _lat,
          _heading,
	  _lon);
  
  string pos(str);
  
  return (pos);
  
}

// compute next position

void AcSim::_computeNext()

{

  umsleep(1000);
  
  time_t now = time(NULL);
  if (now - _changeTime > 120) {
    _changeTime = now;
    _headBias = -15.0 + STATS_uniform_gen() * 40.0;
  }

  // time since last computation

  double dtime = now - _timeLast;
  _timeLast = now;

  // compute heading

  double dhead = STATS_uniform_gen() * _headBias;
  _heading += dhead;

  // distance moved

  double distKm = (_speed * dtime) / 1000.0;

  // compute new location
  
  double new_lat, new_lon;
  PJGLatLonPlusRTheta(_lat, _lon,
		      distKm, _heading,
		      &new_lat, &new_lon);

  // check dist from start point

  double range, theta;
  PJGLatLon2RTheta(new_lat, new_lon,
		   _startLat, _startLon,
		   &range, &theta);

  if (range > 100.0) {
    _lat = _startLat;
    _lon = _startLon;
  } else {
    _lat = new_lat;
    _lon = new_lon;
  }

}


