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
// BeamGeomMgr.cc
//
// Manages beam geometry objects
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2007
//
///////////////////////////////////////////////////////////////

#include "BeamGeomMgr.hh"
#include <cmath>
using namespace std;

const double BeamGeomMgr::_smallDiff = 0.0001;

/////////////////////////////////////////////////////////
// constructor

BeamGeomMgr::BeamGeomMgr(const Params &params) :
        _params(params)
{
  return;
}

/////////////////////////////////////////////////////////
// destructor

BeamGeomMgr::~BeamGeomMgr()
{
  return;
}

////////////////////////////////////////////////////////////////
// clear stored geometries

void BeamGeomMgr::clear()

{
  _geoms.clear();
  return;
}

////////////////////////////////////////////////////////////////
// save the geometry for a beam

void BeamGeomMgr::saveGeom(const Beam *beam)

{

  // determine the angular resolution, if available
  // this applies to indexed beams only

  double angularRes = 0.0;
  if (beam->isIndexed && beam->angularResolution > 0) {
    angularRes = beam->angularResolution;
  }
  
  // check if we already have a matching geometry
  
  for (int ii = 0; ii < (int) _geoms.size(); ii++) {

    BeamGeom &geom = _geoms[ii];
    
    // check the geometry
    
    if (fabs(beam->startRange - geom.startRange) < _smallDiff &&
        fabs(beam->gateSpacing - geom.gateSpacing) < _smallDiff &&
        fabs(angularRes - geom.angularRes) < _smallDiff) {
      
      geom.nBeams++;

      // increase the max gate number if needed
      
      if (geom.maxNGates < beam->nGates) {
	geom.maxNGates = beam->nGates;
      }

      return;

    }

  } // ii 

  // no match, add one

  BeamGeom geom(beam->startRange,
                beam->gateSpacing,
                angularRes);

  _geoms.push_back(geom);

  return;

}

////////////////////////////////////////////////////////////////
// load the predominant geometry for later retrieval
//
// Returns:
//   true if predominant beam geometry has changed,
//   false otherwise

bool BeamGeomMgr::loadPredominantGeom() const
  
{
  
  if (_geoms.size() == 0) {
    return false;
  }
  
  // find the predominant geometry, which is that with the
  // largest number of beams
  
  int predominant = 0;
  int maxBeams = 0;
  for (size_t ii = 0; ii < _geoms.size(); ii++) {
    if (_geoms[ii].nBeams > maxBeams) {
      maxBeams = _geoms[ii].nBeams;
      predominant = (int) ii;
    }
  } // ii
  const BeamGeom &pd = _geoms[predominant];
  
  // check if geometry has changed

  bool changed = false;

  if (fabs(pd.startRange - _predom.startRange) > _smallDiff) {
    changed = true;
    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << "--> startRange has changed" << endl;
      cerr << "    from: " << _predom.startRange << endl;
      cerr << "      to: " << pd.startRange << endl;
    }
  }

  if (fabs(pd.gateSpacing - _predom.gateSpacing) > _smallDiff) {
    changed = true;
    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << "--> gateSpacing has changed" << endl;
      cerr << "    from: " << _predom.gateSpacing << endl;
      cerr << "      to: " << pd.gateSpacing << endl;
    }
  }

  if (fabs(pd.angularRes - _predom.angularRes) > _smallDiff) {
    changed = true;
    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << "--> angularRes has changed" << endl;
      cerr << "    from: " << _predom.angularRes << endl;
      cerr << "      to: " << pd.angularRes << endl;
    }
  }

  if (pd.maxNGates > _predom.maxNGates) {
    changed = true;
    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << "--> maxNGates has increased" << endl;
      cerr << "    from: " << _predom.maxNGates << endl;
      cerr << "      to: " << pd.maxNGates << endl;
    }
  }

  if (changed) {
    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << endl << "==== BEAM GEOMETRY HAS CHANGED ====" << endl;
      cerr << "Dsr2Vol will only put the tilts for which the predominant" << endl;
      cerr << "is valid." << endl;
      cerr << "If the gate spacing has changed, then you can consider" << endl;
      cerr << "remapping all the data onto the finest resolution." << endl;
      cerr << "e.g if the data are a mix of 250m and 1Km spacings, " << endl;
      cerr << "then remap all the data onto 250m spacing." << endl << endl;
    }
    _predom = pd;
  }
  
  return changed;

}

/////////////////////////////////////////////////////////////////
// find the index to the radar params which matches
// the predominant geometry
//
// Returns the index on success, -1 on failure

int BeamGeomMgr::findPredomRadarParams(const vector<DsRadarParams *> &rparams) const

{

  for (int ii = 0; ii < (int) rparams.size(); ii++) {
    
    const DsRadarParams &rp = *rparams[ii];

    if (fabs(rp.startRange - _predom.startRange) < _smallDiff &&
        fabs(rp.gateSpacing - _predom.gateSpacing) < _smallDiff) {
      return ii;
    }

  }

  return -1;

}

///////////////////////////////////////////////////////////////
// check if matches the predominant geometry
// start range, gate spacing

bool BeamGeomMgr::matchesPredom(double startRange, double gateSpacing) const
  
{
  
  if (fabs(startRange - _predom.startRange) < _smallDiff &&
      fabs(gateSpacing - _predom.gateSpacing) < _smallDiff) {
    return true;
  } else {
    return false;
  }

}

/////////////////////////////////////////
// print geometry table

void BeamGeomMgr::printTable(ostream &out) const

{

  if (_geoms.size() > 0) {
    out << "Beam geometry table" << endl;
    out << "===================" << endl;
    for (size_t ii = 0; ii < _geoms.size(); ii++) {
      _geoms[ii].print(out);
    }
    out << endl;
  }

}

/////////////////////////////////////////
// print predominant geometry

void BeamGeomMgr::printPredom(ostream &out) const

{

  out << "==== PREDOMINANT BEAM GEOMETRY ====" << endl;
  _predom.print(out);
  out << endl;

}

