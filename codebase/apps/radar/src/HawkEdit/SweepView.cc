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
// SweepView.cc
//
// SweepView object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Brenda Javornik
//
// March 2021
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

#include "SweepView.hh"
#include <toolsa/LogStream.hh>

#include <string>
#include <cmath>
#include <iostream>

#include <QLabel>

using namespace std;

/////////////////////////////////////////////////////////////
// Constructor

SweepView::SweepView(QWidget *parent)
//const Params &params) :
//        _params(params)
        
{

  //_params = ParamFile::Instance();
  //_reversedInGui = false;
  _guiIndex = 0;
  _selectedAngle = 0.0;
//  _createSweepPanel(parent);
//}

/////////////////////////////////////////////////////////////
// create the sweep panel
// buttons will be filled in by createSweepRadioButtons()

//void SweepView::_createSweepPanel(QWidget *parent)
//{
  
//  _sweepPanel = new QGroupBox("Sweeps", parent);
  setTitle("Sweeps");
  _sweepVBoxLayout = new QVBoxLayout;
  setLayout(_sweepVBoxLayout);
  setAlignment(Qt::AlignHCenter);

  _sweepRButtons = new vector<QRadioButton *>();

}

/////////////////////////////////////////////////////////////
// destructor

SweepView::~SweepView()

{
  //_sweeps.clear();
}

/////////////////////////////////////////////////////////////
// set from a volume

//void SweepView::set() // const RadxVol &vol)
  
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
//      LOG(DEBUG) << "INFO - SweepView: sweep list is reversed in GUI";
//    }
  //}

//}

/*
void SweepView::reset(const RadxVol &vol)
  
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
      cerr << "INFO - SweepView: sweep list is reversed in GUI" << endl;
    }
  }

}
*/



/////////////////////////////////////////////////////////////
// set angle
// size effect: sets the selected index

void SweepView::setAngle(double angle)
  
{
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

void SweepView::setGuiIndex(int index) 
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

int SweepView::getGuiIndex() 
{
  return _guiIndex;
}

/////////////////////////////////////////////////////////////
/* set selected file index

void SweepView::setFileIndex(int index) 
{

  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    if (_sweeps[ii].indexInFile == index) {
      setGuiIndex(index);
    }
  }

}
*/

///////////////////////////////////////////////////////////////
// change sweep

void SweepView::changeSweep(bool value) {

  LOG(DEBUG) << "enter";

// TODO: fix up ...
  if (!value) {
    return;
  }

  for (size_t ii = 0; ii < _sweepRButtons->size(); ii++) {
    if (_sweepRButtons->at(ii)->isChecked()) {
      LOG(DEBUG) << "sweepRButton " << ii << " is checked; moving to sweep index " << ii;
      _selectedAngle = _sweepRButtons->at(ii)->text().toDouble();
      emit selectedSweepChanged(_selectedAngle);
      //_sweepManager.setGuiIndex(ii);
      //_ppi->setStartOfSweep(true);
      //_rhi->setStartOfSweep(true);
      //_moveUpDown();
      //return;
    }
  } // ii

}

/////////////////////////////////////////////////////////////
// change selected index by the specified value

void SweepView::changeSelectedIndex(int increment) 
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


/////////////////////////////////////////////////////////////
// get the fixed angle, optionally specifying an index
/*
double SweepView::getFixedAngleDeg(ssize_t sweepIndex ) const 
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

/////////////////////////////////////////////////////////////////////
// create radio buttons
// this requires that _sweepManager is up to date with sweep info

void SweepView::createSweepRadioButtons(vector<double> *sweepAngles) 
{

  _params = ParamFile::Instance();
  // fonts
  
  QLabel dummy;
  QFont font = dummy.font();
  QFont fontm2 = dummy.font();
  int fsize = _params->label_font_size;
  int fsizem2 = _params->label_font_size - 2;
  font.setPixelSize(fsize);
  fontm2.setPixelSize(fsizem2);
  
  // radar and site name
  
  char buf[256];
  _sweepRButtons = new vector<QRadioButton *>();

  for (vector<double>::iterator it = sweepAngles->begin(); it != sweepAngles->end(); ++it) {
  //for (int ielev = 0; ielev < (int) _sweepManager.getNSweeps(); ielev++) {
    
    //std::snprintf(buf, 256, "%.2f", _sweepManager.getFixedAngleDeg(ielev));
    std::snprintf(buf, 256, "%.2f", *it);
    QRadioButton *radio1 = new QRadioButton(buf); 
    radio1->setFont(fontm2);
    
    //if (ielev == _sweepManager.getGuiIndex()) {
    //  radio1->setChecked(true);
    //}
    
    _sweepRButtons->push_back(radio1);
    _sweepVBoxLayout->addWidget(radio1);
    
    // connect slot for sweep change

    connect(radio1, SIGNAL(toggled(bool)), this, SLOT(changeSweep(bool)));

  }

}

/////////////////////////////////////////////////////////////////////
// create sweep panel of radio buttons

void SweepView::clearSweepRadioButtons() 
{

  QLayoutItem* child;
  if (_sweepVBoxLayout != NULL) {
    while (_sweepVBoxLayout->count() !=0) {
      child = _sweepVBoxLayout->takeAt(0);
      if (child->widget() !=0) {
        delete child->widget();
      }
      delete child;
    }
  }
 
} 


