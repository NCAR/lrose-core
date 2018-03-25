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
///////////////////////////////////////////////////////////////
// SweepManager.cc
//
// SweepManager object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2018
//
///////////////////////////////////////////////////////////////
//
// Manage the sweep details, return sweep information
//
///////////////////////////////////////////////////////////////

#include "SweepManager.hh"

#include <string>
#include <cmath>
#include <iostream>

using namespace std;

/////////////////////////////////////////////////////////////
// Constructor

SweepManager::SweepManager(const Params &params) :
        _params(params)
        
{

  _reversed = false;
  _selectedIndex = -1;
  _selectedAngle = NAN;

}

/////////////////////////////////////////////////////////////
// destructor

SweepManager::~SweepManager()

{
  _sweeps.clear();
}

/////////////////////////////////////////////////////////////
// set from a volume

void SweepManager::set(const RadxVol &vol,
                       double selectedAngle /* = NAN */)
  
{

  // init

  _sweeps.clear();
  _reversed = false;
  _selectedIndex = -1;
  _selectedAngle = selectedAngle;

  const vector<RadxSweep *> sweeps = vol.getSweeps();

  // zero length case
  
  if (sweeps.size() == 0) {
    return;
  }

  // check if angles are in ascending or descending order
  // if ascending, reverse because the angle widget is rendered top down

  if (sweeps[0]->getFixedAngleDeg() < sweeps[sweeps.size()-1]->getFixedAngleDeg()) {
    _reversed = true;
  }
  if (_reversed) {
    for (ssize_t ii = sweeps.size() - 1; ii >= 0; ii--) {
      _sweeps.push_back(sweeps[ii]);
    } // ii
  } else {
    for (ssize_t ii = 0; ii < (ssize_t) sweeps.size(); ii++) {
      _sweeps.push_back(sweeps[ii]);
    } // ii
  }

  if (isfinite(_selectedAngle)) {
    setSelectedIndex(_selectedAngle);
  }
  
}

/////////////////////////////////////////////////////////////
// set selected index from angle

void SweepManager::setSelectedIndex(double selectedAngle)
  
{

  _selectedAngle = selectedAngle;
  _selectedIndex = -1;

  if (_sweeps.size() == 0) {
    return;
  }

  double minDiff = 1.0e99;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    double angle = _sweeps[ii]->getFixedAngleDeg();
    double diff = fabs(angle - _selectedAngle);
    if (diff < minDiff) {
      _selectedIndex = ii;
      minDiff = diff;
    }
  } // ii

}

