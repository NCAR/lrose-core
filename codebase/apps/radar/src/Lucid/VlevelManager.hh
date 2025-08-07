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
#include "GlobalData.hh"
#include "Params.hh"
#include <toolsa/DateTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Mdv/Mdvx.hh>

class MdvReader;

class VlevelManager {
  
public:

  // constructor
  
  VlevelManager(Params &params);
  
  // destructor
  
  virtual ~VlevelManager();

  // set from volume

  void set(const RadxVol &vol);
  
  // set from Mdvx data

  void set(const MdvReader &mread);
  
  // request a given vlevel
  // size effect: sets the selected index
  // returns selected level
  
  double requestLevel(double val);
  
  // change selected index by the specified increment
  // pos for up, neg for down
  
  void incrementIndex(int incr);
  
  // get number of levels
  
  size_t getNLevels() const { return _levels.size(); }

  // get levels
  
  const vector<double> &getLevels() const { return _levels; }
  
  // get the requested level
  
  double getRequestedLevel() const { return _requestedLevel; }

  // get the selected level
  
  double getSelectedLevel() const { return _levels[_selectedIndex]; }
  
  // get index for selected data
  
  int getSelectedIndex() const { return _selectedIndex; }

  // Get the vlevel, optionally specifying an index.
  // If index == -1, use _selectedIndex.
  
  double getLevel(int index = -1) const;
  
  // get level closest to passed-in value
  // side effect - sets index if non-null
  
  double getLevelClosest(double level, int *index = nullptr);

  // get max/min level
  
  double getLevelMax() const;
  double getLevelMin() const;
  
  // get type of Vlevel in Mdvx
  
  Mdvx::vlevel_type_t getMdvxVlevelType() const { return _mdvxVlevelType; }

  // get units
  
  const string &getUnits() const { return _units; }

  // was the order reversed compared to the data?
  
  bool getOrderReversed() const { return _orderReversed; }

private:
  
  Params &_params;
  GlobalData &_gd;

  vector<double> _levels;
  double _requestedLevel;
  int _selectedIndex;

  bool _orderReversed; // with respect to data
  
  string _units;
  Mdvx::vlevel_type_t _mdvxVlevelType;

  // init to single 0-valued entry
  
  void _init();

  // check that object is consistent, if not, fix it

  void _checkConsistent();
  
  // set the selected index from the requested level,
  // for internal consistency
  
  void _setSelectedIndex();
  
};

#endif

