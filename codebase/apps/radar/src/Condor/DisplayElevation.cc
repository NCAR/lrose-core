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
// DisplayElevation.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////

#include "DisplayElevation.hh"

// constructor
// take a list of sweeps, elevtion values??
DisplayElevation::DisplayElevation(const vector<RadxSweep *> &sweeps) {
  //const string &label,
  //                         const string &name,
  //                         const string &units,
  //                         const string &shortcut) :
  //      _label(label),
  //      _name(name),
  //      _units(units),
  //      _shortcut(shortcut),
  //      _selectValue(0),
        _dialog(NULL)

{

}

// destructor

DisplayElevation::~DisplayElevation()

{
  if (_dialog) {
    delete _dialog;
  }
}

// ///////////////////////////////////////////////////////
// // create the dialog

void DisplayElevation::createDialog(QDialog *mentor,
                                 const string &initText)
  
{

  if (_dialog) {
    delete _dialog;
    _dialog = NULL;
  }

  _dialog = new QLabel(mentor);
  _dialog->setText(initText.c_str());

}

// set the dialog text

void DisplayElevation::setDialogText(const string &text)

{

  if (_dialog) {
    _dialog->setText(text.c_str());
  }

}

// print the object

void DisplayElevation::print(ostream &out)

{

  out << "========= DisplayElevation ========" << endl;
  out << "  label: " << _label << endl;
  out << "  name: " << _name << endl;
  out << "  units: " << _units << endl;
  out << "  shortcut: " << _shortcut << endl;
  out << "  buttonRow: " << _buttonRow << endl;
  out << "  selectValue: " << _selectValue << endl;
  out << "===============================" << endl;

}

