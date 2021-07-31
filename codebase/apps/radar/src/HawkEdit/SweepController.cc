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
// SweepController.cc
//
// SweepController object
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

#include "SweepController.hh"
#include <toolsa/LogStream.hh>

#include <string>
#include <cmath>
#include <iostream>

using namespace std;

/////////////////////////////////////////////////////////////
// Constructor

SweepController::SweepController()
//const Params &params) :
//        _params(params)
        
{

  _model = new SweepModel();
  //_params = ParamFile::Instance();
  //_reversedInGui = false;
  //_guiIndex = 0;
  //_selectedAngle = 0.0;

}

/////////////////////////////////////////////////////////////
// destructor

SweepController::~SweepController()

{
  //_sweeps.clear();
}


void SweepController::dataFileChanged() {

  clearSweepRadioButtons();
  createSweepRadioButtons();

}

// TODO: this may not be needed
void SweepController::sweepSelected() {
}


//double SweepController::getSelectedSweepAngle() {
//  return _model->getSelectedSweepAngle();
//}
/////////////////////////////////////////////////////////////
// set from a volume

//void SweepController::set() // const RadxVol &vol)
  
//{

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
//      LOG(DEBUG) << "INFO - SweepController: sweep list is reversed in GUI";
//    }
  //}

//}

/*
void SweepController::reset(const RadxVol &vol)
  
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
      cerr << "INFO - SweepController: sweep list is reversed in GUI" << endl;
    }
  }

}
*/



/////////////////////////////////////////////////////////////
// set angle
// size effect: sets the selected index

void SweepController::setAngle(double angle)
  
{
  _model->setSelectedAngle(angle);
  /*
  _selectedAngle = angle;
  _guiIndex = 0;
  
  if (_sweeps.size() == 0) {
    return;
  }

  double minDiff = 1.0e99;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    double angle = _sweeps[ii].radx->getFixedAngleDeg();
    double diff = fabs(angle - _selectedAngle);
    if (diff < minDiff) {
      _guiIndex = ii;
      minDiff = diff;
    }
  } // ii

  _selectedAngle = _sweeps[_guiIndex].radx->getFixedAngleDeg();
*/
}

/////////////////////////////////////////////////////////////
// set selected gui index

void SweepController::setGuiIndex(int index) 
{
/*
  _guiIndex = index;
  if (_guiIndex <= 0) {
    _guiIndex = 0;
  } else if (_guiIndex > (int) _sweeps.size() - 1) {
    _guiIndex = _sweeps.size() - 1;
  }
  if (_sweeps.size() > 0) {
    _selectedAngle = _sweeps[_guiIndex].radx->getFixedAngleDeg();
  } else {
    _selectedAngle = 0;
  }
*/

}

int SweepController::getGuiIndex() 
{
  return _view->getGuiIndex();
}


/////////////////////////////////////////////////////////////
/* set selected file index

void SweepController::setFileIndex(int index) 
{

  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    if (_sweeps[ii].indexInFile == index) {
      setGuiIndex(index);
    }
  }

}
*/

void SweepController::createSweepRadioButtons() {
 
  vector<double> *sweepAngles = _model->getSweepAngles();
  _view->createSweepRadioButtons(sweepAngles);
  delete sweepAngles;
}

void SweepController::clearSweepRadioButtons() {
 
  _view->clearSweepRadioButtons();

}

/////////////////////////////////////////////////////////////
// change selected index by the specified value

void SweepController::changeSelectedIndex(int increment) 
{
/*
  _guiIndex += increment;
  if (_guiIndex < 0) {
    _guiIndex = 0;
  } else if (_guiIndex > (int) _sweeps.size() - 1) {
    _guiIndex = _sweeps.size() - 1;
  }
  _selectedAngle = _sweeps[_guiIndex].radx->getFixedAngleDeg();
*/
}

int SweepController::getSelectedIndex() 
{
  return _model->getSelectedSweepNumber();
/*
  _guiIndex += increment;
  if (_guiIndex < 0) {
    _guiIndex = 0;
  } else if (_guiIndex > (int) _sweeps.size() - 1) {
    _guiIndex = _sweeps.size() - 1;
  }
  _selectedAngle = _sweeps[_guiIndex].radx->getFixedAngleDeg();
*/
}

/////////////////////////////////////////////////////////////
// get the fixed angle, optionally specifying an index
/*
double SweepController::getFixedAngleDeg(ssize_t sweepIndex ) const 
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
  
