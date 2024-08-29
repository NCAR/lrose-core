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
  _levels.clear();
}

/////////////////////////////////////////////////////////////
// set from a volume

void VlevelManager::set(const RadxVol &vol)
  
{

  // first time?

  bool init = false;
  if (_levels.size() == 0) {
    init = true;
  }

  // init

  _levels.clear();
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
    glevel.radx = sweepsInVol[jj];
    glevel.indexInFile = jj;
    glevel.indexInGui = ii;
    _levels.push_back(glevel);
  }

  // initialize vlevel index if needed

  if (init || _guiIndex > ((int) _levels.size()-1)) {
    setGuiIndex(_levels.size() - 1);
  } else if (_guiIndex < 0) {
    _guiIndex = 0;
  }

  // set selected angle

  _selectedLevel = _levels[_guiIndex].radx->getFixedAngleDeg();

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
  
  if (_levels.size() == 0) {
    return;
  }

  double minDiff = 1.0e99;
  for (size_t ii = 0; ii < _levels.size(); ii++) {
    double level = _levels[ii].radx->getFixedAngleDeg();
    double diff = fabs(level - _selectedLevel);
    if (diff < minDiff) {
      _guiIndex = ii;
      minDiff = diff;
    }
  } // ii

  _selectedLevel = _levels[_guiIndex].radx->getFixedAngleDeg();

}

/////////////////////////////////////////////////////////////
// set selected gui index

void VlevelManager::setGuiIndex(int index) 
{

  _guiIndex = index;
  if (_guiIndex < 0) {
    _guiIndex = 0;
  } else if (_guiIndex > (int) _levels.size() - 1) {
    _guiIndex = _levels.size() - 1;
  }
  _selectedLevel = _levels[_guiIndex].radx->getFixedAngleDeg();

}


/////////////////////////////////////////////////////////////
// set selected file index

void VlevelManager::setFileIndex(int index) 
{

  for (size_t ii = 0; ii < _levels.size(); ii++) {
    if (_levels[ii].indexInFile == index) {
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
  } else if (_guiIndex > (int) _levels.size() - 1) {
    _guiIndex = _levels.size() - 1;
  }
  _selectedLevel = _levels[_guiIndex].radx->getFixedAngleDeg();

}


/////////////////////////////////////////////////////////////
// get the vlevel, optionally specifying an index

double VlevelManager::getLevel(ssize_t vlevelIndex /* = -1*/) const 
{
  
  if (vlevelIndex < 0) {
    if (_guiIndex < 0) {
      return 0.0;
    } else {
      return _levels[_guiIndex].radx->getFixedAngleDeg();
    }
  } 

  if (vlevelIndex < (ssize_t) _levels.size()) {
    return _levels[vlevelIndex].radx->getFixedAngleDeg();
  }

  return _levels[_levels.size()-1].radx->getFixedAngleDeg();

}

  
