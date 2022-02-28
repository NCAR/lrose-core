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
  setTitle("Scan Angles");
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
  _sweepRButtons->at(_guiIndex)->setChecked(false);
  _guiIndex = index;
  if (_guiIndex <= 0) {
    _guiIndex = 0;
  } else if (_guiIndex > (int) _sweepRButtons->size() - 1) {
    _guiIndex = _sweepRButtons->size() - 1;
  }
  if (_sweepRButtons->size() > 0) {
    _selectedAngle = _sweepRButtons->at(_guiIndex)->text().toDouble();
    _sweepRButtons->at(_guiIndex)->setChecked(true);
  } else {
    _selectedAngle = 0;
  }
}

int SweepView::getGuiIndex() 
{
  return _guiIndex;
}

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
      _guiIndex = ii;
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

void SweepView::updateSweepRadioButtons(vector<double> *sweepAngles) 
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
  
  bool selectedFound = false;
  
  char buf[256];
  vector<QRadioButton *> *mergedList = new vector<QRadioButton *>();
  vector<QRadioButton *> *unusedSweeps = new vector<QRadioButton *>();

  // merge _sweepRButtons with sweepAngles into a new list
  // assume both are sorted in increasing order
  // create new buttons as needed
  vector<double>::iterator it_new = sweepAngles->begin();
  vector<QRadioButton *>::iterator it_old = _sweepRButtons->begin();

  while (it_new != sweepAngles->end() && it_old != _sweepRButtons->end()) {

    bool ok;
    QRadioButton *button = *it_old;
    double old_value = button->text().toDouble(&ok);
    if (!ok) old_value = -99999.9; // just set it to something to remove the button
    double new_value = *it_new;
    double diff = abs(old_value - new_value);
    if (diff < 0.1) {  // roughly equal
      // add old to merged list; we can still use this button
      mergedList->push_back(*it_old);
      if (button->isChecked()) {
        selectedFound = true;
      }
      ++it_old;
      ++it_new;
    } else {
      if (old_value < new_value) {
        // move old to unused list
        unusedSweeps->push_back(*it_old);
        ++it_old;
      }
      if (old_value > new_value) {

        // create new button
        std::snprintf(buf, 256, "%.2f", *it_new);
        QRadioButton *radio1 = new QRadioButton(buf); 
        radio1->setFont(fontm2);
        
        mergedList->push_back(radio1);
        //_sweepVBoxLayout->addWidget(radio1);
        
        // connect slot for sweep change
        connect(radio1, &QRadioButton::toggled, this, &SweepView::changeSweep);

        ++it_new;
      }
    }
  }

  if (it_new == sweepAngles->end()) {
    // push the rest of the previous sweeps to the unused list
    while (it_old != _sweepRButtons->end()) {
      unusedSweeps->push_back(*it_old);
      ++it_old;
    }
  } else if (it_old == _sweepRButtons->end()) {
    // push the rest of the new sweeps to the merged list
    while (it_new != sweepAngles->end()) {
      //create new Radio button
      std::snprintf(buf, 256, "%.2f", *it_new);
      QRadioButton *radio1 = new QRadioButton(buf); 
      radio1->setFont(fontm2);
      // add it to the merged list
      mergedList->push_back(radio1);
      connect(radio1, &QRadioButton::toggled, this, &SweepView::changeSweep);

      ++it_new;
    }
  } else {
    cerr << "unexpected condition in SweepView: updateSweepRadioButtons" << endl;
  }
  //if (_sweepRButtons->size() != 0) {
  //  cerr << "sweep radio button list NOT EMPTY: SweepView::updateSweepRadioButtons" 
  //    << endl;
  //}
  // remember, all the sweep radio buttons are saved in the merged list or in
  // the unused list.
  _sweepRButtons = mergedList;
  // clear layout
  clearSweepRadioButtons();
  // add each radio button to layout
  vector<QRadioButton *>::iterator it;
  for (it = mergedList->begin(); it != mergedList->end(); ++it) {
    _sweepVBoxLayout->addWidget(*it);     
  }
 
  if (!selectedFound) {
    setGuiIndex(0);
  }
  
  /*
  for (it = unusedSweeps->begin(); it != unusedSweeps->end(); ++it) {
    bool disconnected = 
      disconnect(*it, &QRadioButton::toggled, this, &SweepView::changeSweep);
    if (!disconnected) 
      cerr << "Could NOT disconnect button from signal " << (*it)->text().toStdString() << endl;
  }
  */
  delete unusedSweeps;


}


/////////////////////////////////////////////////////////////////////
// create radio buttons
// this requires that _sweepManager is up to date with sweep info
// TODO: we may not need this function; updateSweepRadioButtons should cover
// the initial create step. ???

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
    setGuiIndex(0);

  }

}

/////////////////////////////////////////////////////////////////////
// clear sweep panel of radio buttons

void SweepView::clearSweepRadioButtons() 
{

  QLayoutItem* child;
  if (_sweepVBoxLayout != NULL) {
    while ((child = _sweepVBoxLayout->takeAt(0)) != nullptr) {
      // delete happens in update of panel
      //if (child->widget() !=0) {
      //  delete child->widget();
      //}
      //delete child;
    }
  }
 
} 


