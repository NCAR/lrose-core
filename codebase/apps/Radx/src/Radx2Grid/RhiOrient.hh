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
// RhiOrient.hh
//
// RhiOrient class.
// Compute echo orientation in a synthetic RHI
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2020
//
///////////////////////////////////////////////////////////////

#ifndef RhiOrient_HH
#define RhiOrient_HH

#include <Radx/RadxVol.hh>
#include <Radx/Radx.hh>
#include "Params.hh"

class RhiOrient

{

public:

  // constructor

  RhiOrient(const Params &params,
            RadxVol &readVol,
            double azimuth,
            double startRangeKm,
            double gateSpacingKm,
            double radarAltKm,
            const vector<double> &gridZLevels);
            
  // destructor

  ~RhiOrient();

  // compute the echo orientation

  void computeEchoOrientation();

  // did this succeed

  bool getSuccess() const { return _success; }

private:

  const Params &_params;
  const RadxVol &_readVol;

  bool _success;
  
  double _azimuth;
  size_t _maxNGates;
  double _startRangeKm;
  double _gateSpacingKm;
  double _radarAltKm;
  vector<double> _gridZLevels;

  vector<RadxRay *> _rays;
  bool _nGatesVary;
  double _meanAzimuth;

  double _DBZ_BAD;
  vector< vector<Radx::fl32> > _dbz; 
  vector< vector<Radx::fl32> > _gridError; 

  // private methods
  
  void _free();
  int _loadSyntheticRhi();
  void _sortRaysByElevation();
  int _loadDbzGrid();
  int _getZIndex(double zz);

  /// sorting rays by time or azimuth

  class RayPtr {
  public:
    RadxRay *ptr;
    RayPtr(RadxRay *p) : ptr(p) {}
  };
  class SortByRayElevation {
  public:
    bool operator()(const RayPtr &lhs, const RayPtr &rhs) const;
  };

};

#endif
