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
// DisplayElevation.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////

#ifndef DisplayElevation_HH
#define DisplayElevation_HH

#include <string>
#include <QLabel>
#include <QDialog>
#include <iostream>
#include <qtplot/ColorMap.hh>

using namespace std;

class DisplayElevation {

public:
  
  DisplayElevation(const string &label,
               const string &name,
               const string &units,
               const string &shortcut);

  virtual ~DisplayElevation();

  const string &getLabel() const { return _label; }
  const string &getName() const { return _name; }
  const string &getUnits() const { return _units; }
  const string &getShortcut() const { return _shortcut; }

  void setSelectValue(double value) { _selectValue = value; }
  double getSelectValue() const { return _selectValue; }
  
  void createDialog(QDialog *mentor, const string &initText);
  void setDialogText(const string &text);

  const QLabel *getDialog() const { return _dialog; }
  QLabel *getDialog() { return _dialog; }
  const string getDialogText() const { return _dialog->text().toStdString(); }

  void print(ostream &out);

private:

  string _label;
  string _name;
  string _units;
  string _shortcut;

  double _selectValue;
  QLabel *_dialog;

};

#endif

