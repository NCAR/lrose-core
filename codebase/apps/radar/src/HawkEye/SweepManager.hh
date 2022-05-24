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
// SweepManager.hh
//
// SweepManager object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2018
//
///////////////////////////////////////////////////////////////
//
// Manage the sweep details, return sweep information
//
///////////////////////////////////////////////////////////////

#ifndef SweepManager_HH
#define SweepManager_HH

#include <cmath>
#include <string>
#include <vector>
#include "Params.hh"
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>

class SweepManager {
  
public:

  class GuiSweep {
  public:
    RadxSweep *radx;
    int indexInFile;
    int indexInGui;
    GuiSweep() {
      radx = NULL;
      indexInFile = 0;
      indexInGui = 0;
    }
  };

  // constructor
  
  SweepManager(const Params &params);
  
  // destructor
  
  virtual ~SweepManager();

  // set from volume

  void set(const RadxVol &vol);
  
  // set the angle
  // size effect: sets the selected index

  void setAngle(double selectedAngle);

  // set the index for the GUI

  void setGuiIndex(int index);

  // set the index for the file

  void setFileIndex(int index);

  // change selected index by the specified increment

  void changeSelectedIndex(int increment);

  // get methods

  size_t getNSweeps() const { return _sweeps.size(); }
  const vector<GuiSweep> &getGuiSweeps() const { return _sweeps; }
  const GuiSweep getSelectedSweep() const { return _sweeps[_guiIndex]; }

  int getGuiIndex() const { return _guiIndex; }
  int getFileIndex() const { return _sweeps[_guiIndex].indexInFile; }
  int getSelectedAngle() const { return _selectedAngle; }
  double getFixedAngleDeg(ssize_t sweepIndex = -1) const;
  bool getReversedInGui() const { return _reversedInGui; }
  
private:
  
  const Params &_params;
  
  // sweeps

  vector<GuiSweep> _sweeps;
  bool _reversedInGui;

  // selection

  int _guiIndex;
  double _selectedAngle;

};

#endif

