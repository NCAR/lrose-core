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
// Antenna.hh
//
// Antenna object - keeps track of antenna postion
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2004
//
///////////////////////////////////////////////////////////////

#ifndef Antenna_H
#define Antenna_H

#include <string>
#include <deque>
#include <Radx/RadxRay.hh>
#include "Params.hh"
using namespace std;

class Antenna {

public:

  typedef struct {
    double el;
    double az;
  } angle_delta_t;

  typedef enum {
    MODE_PPI, MODE_RHI, MODE_STOPPED, MODE_SLEWING, MODE_UNKNOWN
  } antenna_mode_t;
  
  typedef enum {
    ANG_INCR, ANG_DECR, ANG_CHANGE_NONE, ANG_CHANGE_UNKNOWN
  } angle_change_t;

  // constructor
  
  Antenna(const string &prog_name,
	  const Params &params);
  
  // destructor
  
  virtual ~Antenna();

  // add a ray
  // Returns true if this ray is the end-of-vol,
  // false otherwise

  bool addRay(const RadxRay *ray);

  // check if the antenna is in transition

  bool getInTransition() const {  return _inTransition; }
  
protected:
  
private:
  
  const string _progName;
  const Params &_params;
  
  bool _firstRay;
  double _prevAz;
  double _prevEl;
  double _accumDeltaEl;
  double _accumDeltaAz;
  double _sweepDeltaAz;
  
  deque<angle_delta_t> _deltas;
  
  antenna_mode_t _mode;
  angle_change_t _elDirn;
  angle_change_t _azDirn;

  double _volMinEl;
  double _volMaxEl;
  int _nRaysVol;

  bool _inTransition;

  // methods

  void _handleSlewing(angle_change_t elChange,
		      angle_change_t azChange,
		      bool &endOfVol);

  void _handlePpi(double delta_az, bool &endOfVol);
  void _handleRhi(bool &endOfVol);

};

#endif
