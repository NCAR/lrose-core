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
// SweepModel.cc
//
// SweepModel object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2018
//
///////////////////////////////////////////////////////////////
//
// Manage the sweep details, return sweep information
// should only keep the selected sweep index
// display the sweep angle for each radio button
// DO NOT STORE SWEEPS HERE!
// get fresh sweep information from DataModel.
//
///////////////////////////////////////////////////////////////

#include "SweepModel.hh"
#include "DataModel.hh"
#include <toolsa/LogStream.hh>

#include <string>
#include <cmath>
#include <iostream>

using namespace std;

/////////////////////////////////////////////////////////////
// Constructor

SweepModel::SweepModel()
//const Params &params) :
//        _params(params)
        
{
  _selectedSweepNumber = 1;
  _selectedSweepAngle = 0.0;

}

/////////////////////////////////////////////////////////////
// destructor

SweepModel::~SweepModel()

{
  //_sweeps.clear();
}

/////////////////////////////////////////////////////////////
// set from a volume

void SweepModel::set() // const RadxVol &vol)
  
{

  //DataModel *vol = DataModel::Instance();

  //const vector<RadxSweep *> sweepsInVol = vol->getSweeps();

  // zero length case
  /*
  if (sweepsInVol.size() == 0) {
    cerr << "no sweeps found in volume" << endl;
    setGuiIndex(0);
    return;
  }

  // check if angles are in ascending or descending order
  // if ascending, reverse so that they are descending in this object
  // that matches a top-down rendering of the angles in the widget

  if (sweepsInVol[0]->getFixedAngleDeg() < 
      sweepsInVol[sweepsInVol.size()-1]->getFixedAngleDeg()) {
    //_reversedInGui = true;
  }

  for (ssize_t ii = 0; ii < (ssize_t) sweepsInVol.size(); ii++) {
    ssize_t jj = ii;
    if (_reversedInGui) {
      jj = sweepsInVol.size() - 1 - ii;
    }
    //GuiSweep gsweep;
    //gsweep.radx = sweepsInVol[jj];
    //gsweep.indexInFile = jj;
    //gsweep.indexInGui = ii;
    //_sweeps.push_back(gsweep);
  }
*/
  // initialize sweep index if needed
/*
  try {
    if (init || _guiIndex > ((int) _sweeps.size()-1)) {
      setGuiIndex(_sweeps.size() - 1);
    } else if (_guiIndex < 0) {
      _guiIndex = 0;
    }
  } catch (std::exception &ex) {
    cerr << ex.what() << endl;
  }

   // set selected angle

  _selectedAngle = _sweeps[_guiIndex].radx->getFixedAngleDeg();
*/
  //if (_params.debug >= Params::DEBUG_VERBOSE) {
//    if (_reversedInGui) {
//      LOG(DEBUG) << "INFO - SweepModel: sweep list is reversed in GUI";
//    }
  //}

}

/*
void SweepModel::reset(const RadxVol &vol)
  
{

  // first time?

  bool init = false;
  if (_sweeps.size() == 0) {
    init = true;
  }

  // init

  _sweeps.clear();
  _reversedInGui = false;

  const vector<RadxSweep *> sweepsInVol = vol.getSweeps();

  // zero length case
  
  if (sweepsInVol.size() == 0) {
    setGuiIndex(0);
    return;
  }

  // check if angles are in ascending or descending order
  // if ascending, reverse so that they are descending in this object
  // that matches a top-down rendering of the angles in the widget

  if (sweepsInVol[0]->getFixedAngleDeg() < 
      sweepsInVol[sweepsInVol.size()-1]->getFixedAngleDeg()) {
    _reversedInGui = true;
  }

  for (ssize_t ii = 0; ii < (ssize_t) sweepsInVol.size(); ii++) {
    ssize_t jj = ii;
    if (_reversedInGui) {
      jj = sweepsInVol.size() - 1 - ii;
    }
    GuiSweep gsweep;
    gsweep.radx = sweepsInVol[jj];
    gsweep.indexInFile = jj;
    gsweep.indexInGui = ii;
    _sweeps.push_back(gsweep);
  }

  // initialize sweep index if needed

  if (init || _guiIndex > ((int) _sweeps.size()-1)) {
    setGuiIndex(_sweeps.size() - 1);
  } else if (_guiIndex < 0) {
    _guiIndex = 0;
  }

  // set selected angle

  _selectedAngle = _sweeps[_guiIndex].radx->getFixedAngleDeg();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_reversedInGui) {
      cerr << "INFO - SweepModel: sweep list is reversed in GUI" << endl;
    }
  }

}
*/




/////////////////////////////////////////////////////////////
/* set selected file index

void SweepModel::setFileIndex(int index) 
{

  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    if (_sweeps[ii].indexInFile == index) {
      setGuiIndex(index);
    }
  }

}
*/

size_t SweepModel::getNSweeps() {
  DataModel *dataModel = DataModel::Instance();
  return dataModel->getNSweeps();
}

vector<double> *SweepModel::getSweepAngles() {
  // connect to DataModel to get the sweep angles.
  DataModel *dataModel = DataModel::Instance();
  vector<double> *sweepAngles = dataModel->getSweepAngles();
  return sweepAngles;
}

vector<int> *SweepModel::getSweepNumbers() {
  // connect to DataModel to get the sweep numbers.
  DataModel *dataModel = DataModel::Instance();
  vector<int> *sweepNumbers = dataModel->getSweepNumbers();
  return sweepNumbers;
}

/*
vector<double> *SweepModel::getSweepAngles(string filePath) {
  // connect to DataModel to get the sweep angles.
  DataModel *dataModel = DataModel::Instance();
  vector<double> *sweepAngles = dataModel->getSweepAngles(filePath);
  return sweepAngles;
}
*/
int SweepModel::getSelectedSweepNumber() {
  return _selectedSweepNumber;
}

double SweepModel::getSelectedAngle() {
  DataModel *dataModel = DataModel::Instance();
  _selectedSweepNumber = dataModel->getSweepAngleFromSweepNumber(_selectedSweepNumber);
  LOG(DEBUG) << "exit _selectedSweepNumber = " << _selectedSweepNumber;
}

void SweepModel::setSelectedAngle(double value) {
  LOG(DEBUG) << "enter angle = " << value;
  // double delta = 0.01;
  _selectedSweepAngle = value;
  // translate the angle to the Sweep Number
  /*
  vector<double> *sweepAngles = getSweepAngles();
  int i = 0;
  bool done = false;
  while ((i < sweepAngles->size()) && !done) { 
    if (abs(sweepAngles->at(i) - value) < delta) {
      // i is the sweep index, we need the Sweep Number
      _selectedSweepNumber = i+1;
      done = true;
    }
    i += 1;
  }
  if (!done) {
    LOG(DEBUG) << "invalid sweep angle " << value;
    throw std::invalid_argument("invalid sweep angle");
  }
  */

  DataModel *dataModel = DataModel::Instance();
  _selectedSweepNumber = dataModel->getSweepNumber((float) value);
  LOG(DEBUG) << "exit _selectedSweepNumber = " << _selectedSweepNumber;
}

void SweepModel::setSelectedNumber(int value) {
  LOG(DEBUG) << "enter _selectedSweepNumber = " << _selectedSweepNumber;
  _selectedSweepNumber = value;
  LOG(DEBUG) << "exit _selectedSweepNumber = " << _selectedSweepNumber;
}

/////////////////////////////////////////////////////////////
// get the fixed angle, optionally specifying an index
/*
double SweepModel::getFixedAngleDeg(ssize_t sweepIndex ) const 
{
 
  if (sweepIndex < 0) {
    if (_guiIndex < 0) {
      return 0.0;
    } else {
      return _sweeps[_guiIndex].radx->getFixedAngleDeg();
    }
  } 

  if (sweepIndex < (ssize_t) _sweeps.size()) {
    return _sweeps[sweepIndex].radx->getFixedAngleDeg();
  }

  return _sweeps[_sweeps.size()-1].radx->getFixedAngleDeg();

}
*/
  
