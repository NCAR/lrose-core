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
#include "cidd.h"

/////////////////////////////////////////////////////////////
// Constructor

VlevelManager::VlevelManager()
        
{
  
  _reversedInGui = false;
  _guiIndex = 0;
  _selectedLevel = 0.0;

}

/////////////////////////////////////////////////////////////
// destructor

VlevelManager::~VlevelManager()

{
  _vlevels.clear();
}

/////////////////////////////////////////////////////////////
// set from Mdvx data

void VlevelManager::setFromMdvx()

{

  _vlevels.clear();
  if (gd.h_win.page >= gd.num_datafields) {
    return;
  }
  MetRecord *mrec = gd.mrec[gd.h_win.page];
  if (mrec->ds_fhdr.nz > MDV64_MAX_VLEVELS) {
    return;
  }
  //  for (int iz = 0; iz < mrec->ds_fhdr.nz; iz++) {
  for (int iz = mrec->ds_fhdr.nz - 1; iz >= 0; iz--) {
    GuiVlevel glevel;
    glevel.indexInFile = iz;
    glevel.indexInGui = iz;
    glevel.level = mrec->ds_vhdr.level[iz];
    _vlevels.push_back(glevel);
  }

  switch(mrec->ds_fhdr.vlevel_type) {
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

  cerr << "QQQQQQQQQQQQQQQQ _vlevel.size(), units: " << _vlevels.size() << ", " << _units << endl;
  
}
  
/////////////////////////////////////////////////////////////
// set from a volume

void VlevelManager::set(const RadxVol &vol)
  
{

  // first time?

  bool init = false;
  if (_vlevels.size() == 0) {
    init = true;
  }

  // init

  _vlevels.clear();
  _reversedInGui = false;

  const vector<RadxSweep *> sweepsInVol = vol.getSweeps();

  // zero length case
  
  if (sweepsInVol.size() == 0) {
    setGuiIndex(0);
    return;
  }

  // check if levels are in ascending or descending order
  // if ascending, reverse so that they are descending in this object
  // that matches a top-down rendering of the levels in the widget

  if (sweepsInVol[0]->getFixedAngleDeg() < 
      sweepsInVol[sweepsInVol.size()-1]->getFixedAngleDeg()) {
    _reversedInGui = true;
  }

  for (ssize_t ii = 0; ii < (ssize_t) sweepsInVol.size(); ii++) {
    ssize_t jj = ii;
    if (_reversedInGui) {
      jj = sweepsInVol.size() - 1 - ii;
    }
    GuiVlevel glevel;
    glevel.level = sweepsInVol[jj]->getFixedAngleDeg();
    glevel.indexInFile = jj;
    glevel.indexInGui = ii;
    _vlevels.push_back(glevel);
  }

  // initialize vlevel index if needed

  if (init || _guiIndex > ((int) _vlevels.size()-1)) {
    setGuiIndex(_vlevels.size() - 1);
  } else if (_guiIndex < 0) {
    _guiIndex = 0;
  }

  // set selected angle

  _selectedLevel = _vlevels[_guiIndex].level;
  _units = "deg";

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_reversedInGui) {
      cerr << "INFO - VlevelManager: vlevel list is reversed in GUI" << endl;
    }
  }

}

/////////////////////////////////////////////////////////////
// set angle
// size effect: sets the selected index

void VlevelManager::setLevel(double level)
  
{
  
  _selectedLevel = level;
  _guiIndex = 0;
  
  if (_vlevels.size() == 0) {
    return;
  }
  
  double minDiff = 1.0e99;
  for (size_t ii = 0; ii < _vlevels.size(); ii++) {
    double level = _vlevels[ii].level;
    double diff = fabs(level - _selectedLevel);
    if (diff < minDiff) {
      _guiIndex = ii;
      minDiff = diff;
    }
  } // ii

  _selectedLevel = _vlevels[_guiIndex].level;
  gd.prev_ht = gd.h_win.cur_ht;
  gd.h_win.cur_ht = _selectedLevel;
  gd.selected_ht = _selectedLevel;
  gd.ht_has_changed = true;
  gd.redraw_horiz = true;
  
}

/////////////////////////////////////////////////////////////
// set selected gui index

void VlevelManager::setGuiIndex(int index) 
{

  _guiIndex = index;
  if (_guiIndex < 0) {
    _guiIndex = 0;
  } else if (_guiIndex > (int) _vlevels.size() - 1) {
    _guiIndex = _vlevels.size() - 1;
  }
  _selectedLevel = _vlevels[_guiIndex].level;

}


/////////////////////////////////////////////////////////////
// set selected file index

void VlevelManager::setFileIndex(int index) 
{

  for (size_t ii = 0; ii < _vlevels.size(); ii++) {
    if (_vlevels[ii].indexInFile == index) {
      setGuiIndex(index);
    }
  }

}


/////////////////////////////////////////////////////////////
// change selected index by the specified value

void VlevelManager::changeSelectedIndex(int increment) 
{

  _guiIndex += increment;
  if (_guiIndex < 0) {
    _guiIndex = 0;
  } else if (_guiIndex > (int) _vlevels.size() - 1) {
    _guiIndex = _vlevels.size() - 1;
  }
  _selectedLevel = _vlevels[_guiIndex].level;

}


/////////////////////////////////////////////////////////////
// get the vlevel, optionally specifying an index

double VlevelManager::getLevel(ssize_t vlevelIndex /* = -1*/) const 
{
  
  if (vlevelIndex < 0) {
    if (_guiIndex < 0) {
      return 0.0;
    } else if (_guiIndex > (int) _vlevels.size() - 1) {
      return 0.0;
    } else {
      return _vlevels[_guiIndex].level;
    }
  } 

  if (vlevelIndex < (ssize_t) _vlevels.size()) {
    return _vlevels[vlevelIndex].level;
  }

  return _vlevels[_vlevels.size()-1].level;

}

  
