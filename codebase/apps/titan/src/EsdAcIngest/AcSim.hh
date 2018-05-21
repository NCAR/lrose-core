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
// AcSim.hh
//
// AcSim file class - aircraft simulator
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2000
//
///////////////////////////////////////////////////////////////

#ifndef AcSim_HH
#define AcSim_HH

#include <string>
#include "Params.hh"
using namespace std;

class AcSim {
  
public:
  
  // constructor

  AcSim(const string &callsign,
	double start_lat, double start_lon,
	double altitude, double speed,
	bool right_burn,
	bool left_burn,
	double ejectable_interval,
	double burn_in_place_interval,
	int n_bip_at_a_time,
        double dry_ice_interval,
	int n_optional_fields,
	const Params::field_name_t *optional_field_names);

  // destructor
  
  virtual ~AcSim();

  // get next position as a string

  string getNextPos();

protected:

private:

  // field names

  int _nOptionalFields;
  const Params::field_name_t *_optionalFieldNames;
  
  // aircraft data

  time_t _changeTime;
  double _startLat, _startLon;
  double _lat, _lon;
  double _altitude; // m
  double _speed;    // km/s
  double _heading;  // deg T
  double _headBias;
  string _callsign;
  time_t _timeLast;

  double _ejectableInterval;
  double _burnInPlaceInterval;
  int _nBipAtaTime;
  double _dryIceInterval;
  
  int _errorFlags;
  int _rBurn;
  int _lBurn;
  int _burnInPlace;
  int _dryIce;
  int _ejectable;
  int _nBurnInPlace;
  int _nEjectable;

  time_t _lastEjectableTime;
  time_t _lastBipTime;

  void _computeNext();

};

#endif

