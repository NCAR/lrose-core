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
// BeamGeomMgr.hh
//
// Manages Beam geometry objects
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2007
//
///////////////////////////////////////////////////////////////

#ifndef BeamGeomMgr_H
#define BeamGeomMgr_H

#include "Params.hh"
#include "Beam.hh"
#include "BeamGeom.hh"
#include <rapformats/DsRadarParams.hh>
#include <vector>
using namespace std;

// beam geometry container class

class BeamGeomMgr {

public:

  BeamGeomMgr(const Params &params);
  ~BeamGeomMgr();

  // clear stored geometries

  void clear();

  // save the geometry for a beam

  void saveGeom(const Beam *beam);

  // load the predominant geometry  for later retrieval
  // Returns:
  //   true if predominant beam geometry has changed,
  //   false otherwise
  
  bool loadPredominantGeom() const;

  // Given a vector of radar params,
  // find the index to the radar params which matches
  // the predominant geometry
  //
  // Returns the index on success, -1 on failure
  
  int findPredomRadarParams(const vector<DsRadarParams *> &rparams) const;

  // get predominant geometry members

  double getPredomStartRange() const { return _predom.startRange; }
  double getPredomGateSpacing() const { return _predom.gateSpacing; }
  double getPredomAngularRes() const { return _predom.angularRes; }
  int getPredomMaxNGates() const { return _predom.maxNGates; }

  // print geometry table
  
  void printTable(ostream &out) const;

  // print predominant geometry

  void printPredom(ostream &out) const;

  // check if matches the predominant geometry
  // start range, gate spacing

  bool matchesPredom(double startRange, double gateSpacing) const;
  
private:

  // data members

  static const double _smallDiff;

  // params

  const Params &_params;

  // predominant geometry

  mutable BeamGeom _predom;

  // array of available geometries

  vector<BeamGeom> _geoms;

};

#endif

