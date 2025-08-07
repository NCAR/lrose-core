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
// VlevelManager.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2024
//
///////////////////////////////////////////////////////////////
//
// Manage the vlevel details, return vlevel information
//
///////////////////////////////////////////////////////////////

#include <string>
#include <cmath>
#include <iostream>

using namespace std;

#include "VlevelManager.hh"
#include "MdvReader.hh"
#include "GlobalData.hh"

/////////////////////////////////////////////////////////////
// Constructor

VlevelManager::VlevelManager(Params &params) :
        _params(params),
        _gd(GlobalData::Instance())
        
{

  _init();
  _requestedLevel = 0.0;

}

/////////////////////////////////////////////////////////////
// destructor

VlevelManager::~VlevelManager()

{
  _levels.clear();
}

/////////////////////////////////////////////////////////////
// set from a Radx volume

void VlevelManager::set(const RadxVol &vol)
  
{
  
  // init
  
  _levels.clear();
  _orderReversed = false;

  const vector<RadxSweep *> sweepsInVol = vol.getSweeps();

  // zero length case
  
  if (sweepsInVol.size() == 0) {
    _init();
    return;
  }
  
  // check if levels are in ascending or descending order
  // if descending, reverse so that they are asscending in this object
  // that matches a top-down rendering of the levels in the widget

  if (sweepsInVol[0]->getFixedAngleDeg() > 
      sweepsInVol[sweepsInVol.size()-1]->getFixedAngleDeg()) {
    _orderReversed = true;
  }
  
  for (ssize_t ii = 0; ii < (ssize_t) sweepsInVol.size(); ii++) {
    ssize_t jj = ii;
    if (_orderReversed) {
      jj = sweepsInVol.size() - 1 - ii;
    }
    _levels.push_back(sweepsInVol[jj]->getFixedAngleDeg());
  }

  // set selected index

  _setSelectedIndex();
  
  // set type
  
  _mdvxVlevelType = Mdvx::VERT_TYPE_ELEV;

  // set units
  
  _units = "deg";

  // debug message
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_orderReversed) {
      cerr << "INFO - VlevelManager set from Radx, vlevel list is reversed" << endl;
    }
  }

}

/////////////////////////////////////////////////////////////
// set from Mdvx data

void VlevelManager::set(const MdvReader &mread)

{

  // init
  
  _levels.clear();
  _orderReversed = false;

  // load up levels
  
  int nz = mread.ds_fhdr.nz;
  if (nz > MDV64_MAX_VLEVELS) {
    nz = MDV64_MAX_VLEVELS;
  }
  for (int iz = 0; iz < nz; iz++) {
    _levels.push_back(mread.ds_vhdr.level[iz]);
  }

  // set selected index

  _setSelectedIndex();
  
  // set type
  
  _mdvxVlevelType = (Mdvx::vlevel_type_t) mread.ds_fhdr.vlevel_type;

  // set units
  
  switch(_mdvxVlevelType) {
    case Mdvx::VERT_TYPE_Z:
    case Mdvx::VERT_TYPE_SIGMA_Z:
      _units = "km";
      break;
    case Mdvx::VERT_TYPE_PRESSURE:
    case Mdvx::VERT_TYPE_SIGMA_P:
      _units = "hPa";
      break;
    case Mdvx::VERT_TYPE_THETA:
      _units = "K";
      break;
    case Mdvx::VERT_TYPE_ELEV:
    case Mdvx::VERT_VARIABLE_ELEV:
    case Mdvx::VERT_TYPE_AZ:
      _units = "deg";
      break;
    case Mdvx::VERT_FLIGHT_LEVEL:
      _units = "FL";
      break;
    default:
      _units.clear();
  }

}
  
/////////////////////////////////////////////////////////////
// request a given vlevel
// size effect: sets the selected index
// returns selected level

double VlevelManager::requestLevel(double val)
  
{

  _checkConsistent();
  _requestedLevel = val;
  
  // find closest index
  
  _selectedIndex = 0;
  double minDiff = 1.0e99;
  for (size_t ii = 0; ii < _levels.size(); ii++) {
    double diff = fabs(_levels[ii] - val);
    if (diff < minDiff) {
      _selectedIndex = ii;
      minDiff = diff;
    }
  } // ii

  return _levels[_selectedIndex];
  
}

/////////////////////////////////////////////////////////////
// change selected index by the specified value

void VlevelManager::incrementIndex(int incr) 
{

  _selectedIndex += incr;
  _checkConsistent();
  
}


/////////////////////////////////////////////////////////////
// Get the vlevel, optionally specifying an index.
// If index == -1, use _selectedIndex.

double VlevelManager::getLevel(int index /* = -1*/) const 
{

  // return level for requested index, if supplied
  
  if (index >= 0 && index < (int) _levels.size()) {
    return _levels[index];
  }

  // check for bounds
  
  if (index < 0) {
    return _levels[0];
  }
  
  if (index >= (int) _levels.size()) {
    return _levels[_levels.size()-1];
  }

  // return val selected
  
  return _levels[_selectedIndex];

}

/////////////////////////////////////////////////////////////
// get level closest to the value passed in

double VlevelManager::getLevelClosest(double level,
                                      int *index /* = nullptr */)
  
{
  
  if (_levels.size() == 0) {
    if (index) {
      *index = 0;
    }
    return 0.0;
  }

  int indx = 0;
  double minDiff = 1.0e99;
  for (size_t ii = 0; ii < _levels.size(); ii++) {
    double diff = fabs(level - _levels[ii]);
    if (diff < minDiff) {
      indx = ii;
      minDiff = diff;
    }
  } // ii

  if (index) {
    *index = indx;
  }

  return _levels[indx];

}

  
/////////////////////////////////////////////////////////////
// get max level
  
double VlevelManager::getLevelMax() const
{
  if (_levels.size() < 1) {
    return 0.0;
  }
  return _levels[_levels.size()-1];
}

/////////////////////////////////////////////////////////////
// get min level
  
double VlevelManager::getLevelMin() const
{
  if (_levels.size() < 1) {
    return 0.0;
  }
  return _levels[0];
}

//////////////////////////////////////////////////////////////////
// init to single 0-val entry
// the object is guaranteed to have at least 1 entry in _levels

void VlevelManager::_init()
        
{
  
  _levels.clear();
  _levels.push_back(0.0);
  _selectedIndex = 0;
  _orderReversed = false;
  _mdvxVlevelType = Mdvx::VERT_TYPE_UNKNOWN;

}

//////////////////////////////////////////////////////////////////
// check that object is consistent
// if not, fix it

void VlevelManager::_checkConsistent()
        
{

  // make sure we have at least 1 entry
  
  if (_levels.size() == 0) {
    _levels.push_back(0.0);
  }

  // make sure selection is within bounds
  
  if (_selectedIndex < 0) {
    _selectedIndex = 0;
  } else if (_selectedIndex > (int) _levels.size() - 1) {
    _selectedIndex = _levels.size() - 1;
  }
  
}

/////////////////////////////////////////////////////////////
// set the selected index from the requested level,
// for internal consistency

void VlevelManager::_setSelectedIndex()
  
{

  _checkConsistent();
  
  // find index for closest vlevel
  
  _selectedIndex = 0;
  double minDiff = 1.0e99;
  for (size_t ii = 0; ii < _levels.size(); ii++) {
    double diff = fabs(_levels[ii] - _requestedLevel);
    if (diff < minDiff) {
      _selectedIndex = ii;
      minDiff = diff;
    }
  } // ii

}

