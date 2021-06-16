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
// SweepView.hh
//
// SweepView object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2021
//
///////////////////////////////////////////////////////////////
//
// Manage the sweep details, return sweep information
//
///////////////////////////////////////////////////////////////

#ifndef SweepView_HH
#define SweepView_HH

#include <cmath>
#include <string>
#include <vector>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include "ParamFile.hh"

class SweepView : public QGroupBox {

  Q_OBJECT
  
public:

  // constructor
  
  SweepView(QWidget *parent); 
  
  // destructor
  
  virtual ~SweepView();

  // set from volume

  void setAngle(double selectedAngle);

  // set the index for the GUI

  void setGuiIndex(int index);
  int getGuiIndex();

  // change selected index by the specified increment

  void changeSelectedIndex(int increment);

  // get methods

  size_t getNSweeps() const { return 0; } // _sweeps.size(); }

  double getSelectedAngle() const { return _selectedAngle; }

  //void _createSweepPanel(QWidget *parent);

  void createSweepRadioButtons(vector<double> *sweepAngles);

  void clearSweepRadioButtons();

public slots:

  void changeSweep(bool value);

private:
  
  ParamFile *_params;
  
  // sweeps




  // selection

  int _guiIndex;  // index of selected sweep
  double _selectedAngle;

  QVBoxLayout *_sweepVBoxLayout;
  vector<QRadioButton *> *_sweepRButtons;

};

#endif

