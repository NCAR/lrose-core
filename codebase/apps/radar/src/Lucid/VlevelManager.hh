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
// VlevelManager.hh
//
// VlevelManager object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2018
//
///////////////////////////////////////////////////////////////
//
// Manage the vlevel details, return vlevel information
//
///////////////////////////////////////////////////////////////

#ifndef VlevelManager_HH
#define VlevelManager_HH

#include <cmath>
#include <string>
#include <vector>
#include "Params.hh"
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>

class VlevelManager {
  
public:

  class GuiVlevel {
  public:
    double level;
    int indexInFile;
    int indexInGui;
    GuiVlevel() {
      level = 0.0;
      indexInFile = 0;
      indexInGui = 0;
    }
  };

  // constructor
  
  VlevelManager();
  
  // destructor
  
  virtual ~VlevelManager();

  // set from volume

  void set(const RadxVol &vol);
  
  // set from Mdvx data

  void setFromMdvx();
  
  // set the level
  // size effect: sets the selected index

  void setLevel(double selectedLevel);

  // set the index for the GUI

  void setGuiIndex(int index);

  // set the index for the file

  void setFileIndex(int index);

  // change selected index by the specified increment

  void changeSelectedIndex(int increment);

  // get methods

  size_t getNVlevels() const { return _vlevels.size(); }
  const vector<GuiVlevel> &getGuiLevels() const { return _vlevels; }
  const GuiVlevel getSelectedVlevel() const { return _vlevels[_guiIndex]; }
  double getLevel(int index) const {
    if (index >= (int) _vlevels.size()) {
      return _vlevels[_vlevels.size()-1].level;
    } else {
      return _vlevels[index].level;
    }
  }

  int getGuiIndex() const { return _guiIndex; }
  int getFileIndex() const { return _vlevels[_guiIndex].indexInFile; }
  int getSelectedLevel() const { return _selectedLevel; }
  bool getReversedInGui() const { return _reversedInGui; }
  
  double getLevel(ssize_t vlevelIndex = -1) const;
  const string &getUnits() const { return _units; }

  private:
  
  // vlevels

  vector<GuiVlevel> _vlevels;
  bool _reversedInGui;
  string _units;

  // selection

  int _guiIndex;
  double _selectedLevel;

};

#endif

