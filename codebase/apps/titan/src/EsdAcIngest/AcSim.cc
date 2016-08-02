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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2000
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
	      double altitude, double speed,
	      bool right_burn,
	      bool left_burn,
	      double ejectable_interval,
	      int burn_in_place_interval,
	      int n_bip_at_a_time,
	      int n_optional_fields,
	      const Params::field_name_t *optional_field_names) :
  _optionalFieldNames(optional_field_names)
  
{

  // initialize up random number generator
  
  time_t now = time(NULL);
  STATS_uniform_seed(now);

  // initialize members

  _startLat = start_lat;
  _startLon = start_lon;
  _altitude = altitude * 0.3048;
  _speed = (speed * 1.852) / 3600.0;
  _callsign = callsign;
  _changeTime = 0;
  _lat = _startLat;
  _lon = _startLon;
  _timeLast = now;
  _heading = STATS_uniform_gen() * 360.0;
  _headBias = 0.0;
  _nOptionalFields = n_optional_fields;
  
  _errorFlags = 0;
  _rBurn = right_burn;
  _lBurn = left_burn;
  _ejectableInterval = ejectable_interval;
  _burnInPlaceInterval = burn_in_place_interval;
  _nBipAtaTime = n_bip_at_a_time;

  _burnInPlace = 0;
  _ejectable = 0;
  _nBurnInPlace = 0;
  _nEjectable = 0;
  _lastEjectableTime = time(NULL);
  _lastBipTime = time(NULL);

}

AcSim::~AcSim ()

{

}

string AcSim::getNextPos ()

{

  // compute next position

  _computeNext();

  // load the basic string

  char str[1024];
  DateTime  dtime(_timeLast);
  sprintf(str, "%s,%.4d,%.2d,%.2d,%.2d,%.2d,%.2d,%g,%g,%g",
	  _callsign.c_str(),
	  dtime.getYear(),
	  dtime.getMonth(),
	  dtime.getDay(),
	  dtime.getHour(),
	  dtime.getMin(),
	  dtime.getSec(),
	  _lat,
	  _lon,
	  _altitude);

  string pos(str);

  for (int i = 0; i < _nOptionalFields; i++) {

    switch (_optionalFieldNames[i]) {

    case Params::GS:
      sprintf(str, ",%g", _speed);
      pos += str;
      break;

    case Params::ERROR_FLAGS:
      sprintf(str, ",%.4d", _errorFlags);
      pos += str;
      break;

    case Params::R_BURN:
      sprintf(str, ",%d", _rBurn);
      pos += str;
      break;

    case Params::L_BURN:
      sprintf(str, ",%d", _lBurn);
      pos += str;
      break;

    case Params::BURN_IN_PLACE:
      sprintf(str, ",%d", _burnInPlace);
      pos += str;
      break;

    case Params::EJECTABLE:
      sprintf(str, ",%d", _ejectable);
      pos += str;
      break;

    case Params::N_BURN_IN_PLACE:
      sprintf(str, ",%d", _nBurnInPlace);
      pos += str;
      break;

    case Params::N_EJECTABLE:
      sprintf(str, ",%d", _nEjectable);
      pos += str;
      break;

    default: {}

    } // switch

  } // i
  
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

  double dist = _speed * dtime;

  // compute new location
  
  double new_lat, new_lon;
  PJGLatLonPlusRTheta(_lat, _lon,
		      dist, _heading,
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

  if (_ejectableInterval >= 0) {
    int tdiff = now - _lastEjectableTime;
    if (tdiff >= _ejectableInterval) {
      _ejectable = 1;
      _nEjectable += (int) ((double) tdiff / _ejectableInterval);
      _lastEjectableTime = now;
    } else {
      _ejectable = 0;
    }
  }

  if (_burnInPlaceInterval >= 0) {
    int tdiff = now - _lastBipTime;
    if (tdiff >= _burnInPlaceInterval) {
      _burnInPlace = 1;
       _nBurnInPlace += _nBipAtaTime;
      _lastBipTime = now;
    } else {
      _burnInPlace = 0;
    }
  }

}


