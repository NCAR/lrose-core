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
//////////////////////////////////////////////////////////////////////////
// HcrSurfaceVel.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2016
//
//////////////////////////////////////////////////////////////////////////
//
// HcrSurfaceVel is intended for use with downward-pointing airborne
// Doppler radars.
//
// HcrSurfaceVel reads in Radx moments, and computes the apparent velocity
// of the surface echo.
//
// Generally this will require some filtering. See:
//   HcrVelFirFilt
//
//////////////////////////////////////////////////////////////////////////

#ifndef HcrSurfaceVel_HH
#define HcrSurfaceVel_HH

#include <string>
#include <deque>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
class RadxRay;
using namespace std;

class HcrSurfaceVel {
  
public:

  // constructor
  
  HcrSurfaceVel();

  // destructor
  
  ~HcrSurfaceVel();

  // set methods

  void setDebug(bool val) { _debug = val; }
  void setVerbose(bool val) { _verbose = val; }

  // name of dbz and vel fields in data

  void setDbzFieldName(const string &name) { _dbzFieldName = name; };
  void setVelFieldName(const string &name) { _velFieldName = name; };

  // max pointing error off nadir for valid measurement

  void setMaxNadirErrorDeg(double val) { _maxNadirErrorDeg = val; }

  // finding the surface
  
  void setMinRangeToSurfaceKm(double val) { _minRangeToSurfaceKm = val; }
  void setMaxSurfaceHeightKm(double val) { _maxSurfaceHeightKm = val; }
  void setMinDbzForSurfaceEcho(double val) { _minDbzForSurfaceEcho = val; }
  void setNGatesForSurfaceEcho(int val) { _nGatesForSurfaceEcho = val; }

  // compute surface velocity - no filtering
  //
  // Sets vel to 0.0 if cannot determine velocity.
  // Also sets dbzSurf and rangeToSurf.
  // Returns 0 on success, -1 on failure.
  
  int computeSurfaceVel(const RadxRay *ray,
                        double &velSurf,
                        double &dbzSurf,
                        double &rangeToSurf) const;
  
protected:
private:

  // debugging

  bool _debug;
  bool _verbose;

  // field names

  string _dbzFieldName;
  string _velFieldName;

  // parameters
  
  double _minRangeToSurfaceKm;
  double _maxSurfaceHeightKm;
  double _minDbzForSurfaceEcho;
  int _nGatesForSurfaceEcho;
  double _maxNadirErrorDeg;

  // methods

  void _initFromFirFilters();
  void _applyStage1Filter();
  void _applySpikeFilter();
  void _applyFinalFilter();
  void _computeConditionedValue();
  void _shiftArraysBy1();

};

#endif
