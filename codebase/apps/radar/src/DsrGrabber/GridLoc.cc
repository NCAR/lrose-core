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
///////////////////////////////////////////////////////////////
// GridLoc.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2007
//
///////////////////////////////////////////////////////////////
//
// Data for a grid location
//
////////////////////////////////////////////////////////////////

#include "GridLoc.hh"
#include <cmath>

// Constructor

GridLoc::GridLoc(const Params &params,
		 double el,
		 double az) :
  _params(params)
  
{

  _elMid = el;
  _azMid = az;

  _elMax = el + _params.grid_el_max_diff;
  _azMax = az + _params.grid_az_max_diff;

  _elMin = el - _params.grid_el_max_diff;
  _azMin = az - _params.grid_az_max_diff;

  for (int ii = 0; ii < _params.stats_fields_n + 1; ii++) {
    _interp.push_back(-9999.0);
  }

}

// destructor

GridLoc::~GridLoc()

{
  _beams.clear();
}

// add a beam, if it is within the required limits

void GridLoc::addBeam(const Beam &beam)

{
  
  if (beam.el >= _elMin && beam.el <= _elMax &&
      beam.az >= _azMin && beam.az <= _azMax) {

    _beams.push_back(beam);

  }

}

// interpolate for this location
// use minimum values is not enough beams

void GridLoc::interpolate(const vector<double> &minima)

{

//   cerr << "============================" << endl;
//   cerr << "  EL, AZ: " << _elMid << ", " << _azMid << endl;

  // find the beams, in each quadrant, closest to this beam

  const Beam *upperLeft = NULL;
  const Beam *lowerLeft = NULL;
  const Beam *upperRight = NULL;
  const Beam *lowerRight = NULL;

  double minDistUpperLeft = 1.0e99;
  double minDistLowerLeft = 1.0e99;
  double minDistUpperRight = 1.0e99;
  double minDistLowerRight = 1.0e99;

  for (int ii = 0; ii < (int) _beams.size(); ii++) {

    const Beam &beam = _beams[ii];

//     cerr << "---->> beam el, az: " << beam.el << ", " << beam.az << endl;
    
    // upper left
    
    if (beam.el >= _elMid && beam.az <= _azMid) {
      double deltaEl = beam.el - _elMid;
      double deltaAz = beam.az - _azMid;
      double dist = sqrt(deltaEl * deltaEl + deltaAz * deltaAz);
      if (dist < minDistUpperLeft) {
	upperLeft = &beam;
	minDistUpperLeft = dist;
      }
    }

    // lower left
    
    if (beam.el <= _elMid && beam.az <= _azMid) {
      double deltaEl = beam.el - _elMid;
      double deltaAz = beam.az - _azMid;
      double dist = sqrt(deltaEl * deltaEl + deltaAz * deltaAz);
      if (dist < minDistLowerLeft) {
	lowerLeft = &beam;
	minDistLowerLeft = dist;
      }
    }

    // upper right
    
    if (beam.el >= _elMid && beam.az >= _azMid) {
      double deltaEl = beam.el - _elMid;
      double deltaAz = beam.az - _azMid;
      double dist = sqrt(deltaEl * deltaEl + deltaAz * deltaAz);
      if (dist < minDistUpperRight) {
	upperRight = &beam;
	minDistUpperRight = dist;
      }
    }

    // lower right
    
    if (beam.el <= _elMid && beam.az >= _azMid) {
      double deltaEl = beam.el - _elMid;
      double deltaAz = beam.az - _azMid;
      double dist = sqrt(deltaEl * deltaEl + deltaAz * deltaAz);
      if (dist < minDistLowerRight) {
	lowerRight = &beam;
	minDistLowerRight = dist;
      }
    }
    
  } // ii

  // load valid beams into vector

  vector<const Beam *> beams;
  if (upperLeft != NULL) {
    beams.push_back(upperLeft);
  }
  if (lowerLeft != NULL) {
    beams.push_back(lowerLeft);
  }
  if (upperRight != NULL) {
    beams.push_back(upperRight);
  }
  if (lowerRight != NULL) {
    beams.push_back(lowerRight);
  }

//   cerr << "+++++++ close beams +++++++" << endl;
//   for (int ii = 0; ii < (int) beams.size(); ii++) {
//     const Beam &beam = *beams[ii];
//     cerr << "  el, az: " << beam.el << ", " << beam.az << endl;
//   }
//   cerr << "++++++++++++++++++++++++++++" << endl;

  // if no beams, set to minima
  
  if (beams.size() < 1) {
    for (int ii = 0; ii < (int) _interp.size(); ii++) {
      _interp[ii] = minima[ii];
    }
    return;
  }

  // compute weights

  double sumWeights = 0.0;
  vector<double> weights;
  for (int ii = 0; ii < (int) beams.size(); ii++) {
    const Beam &beam = *beams[ii];
    double deltaEl = beam.el - _elMid;
    double deltaAz = beam.az - _azMid;
    double dist = sqrt(deltaEl * deltaEl + deltaAz * deltaAz);
    double inverseDist = 1.0 / dist;
    weights.push_back(inverseDist);
    sumWeights += inverseDist;
//     cerr << "deltaEl, deltaAz, dist, inverseDist, sumWeights: "
// 	 << deltaEl << ", "
// 	 <<  deltaAz << ", "
// 	 <<  dist << ", "
// 	 <<  inverseDist << ", "
// 	 <<  sumWeights << endl;
  }
  for (int ii = 0; ii < (int) weights.size(); ii++) {
    weights[ii] /= sumWeights;
//     cerr << "ii, weights[ii]" << ii << ", " << weights[ii] << endl;
  }
  
  // interpolate

  for (int jj = 0; jj < (int) _interp.size(); jj++) {
    double wtSum = 0.0;
    for (int ii = 0; ii < (int) beams.size(); ii++) {
      const Beam &beam = *beams[ii];
      wtSum += weights[ii] * beam.fields[jj];
    }
    
    _interp[jj] = wtSum;
  }

//   cerr << "============================" << endl;

}

// clear all beams

void GridLoc::clearBeams()

{
  _beams.clear();
}

