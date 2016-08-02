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
// Boundary.hh
//
// Boundary class
//
// Provides services for boundary identification.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef Boundary_HH
#define Boundary_HH

#include <dataport/port_types.h>
#include <euclid/boundary.h>
#include <euclid/point.h>
#include <euclid/node.h>
#include "Worker.hh"
using namespace std;

////////////////////////////////
// Boundary

class Boundary : public Worker {
  
public:

  // constructor

  Boundary(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~Boundary();

  int OK;

  // compute the boundary radii

  double *computeRadii(int nx, int ny, ui08 *grid, int threshold,
		       int n_poly_sides, double ref_x, double ref_y);

  // data member access

  int nIntervals() { return (_nIntervals); }
  Interval *intervals() {return (_intervals); }
  
protected:
  
private:

  Row_hdr *_rowh;
  int _nRowsAlloc;

  Interval *_intervals;
  int _nIntervals;
  int _nIntervalsAlloc;
  
  int *_bdryList;
  Point_d *_bdryPts;
  Node *_nodes;
  int _nNodesAlloc;

  Star_point *_starRays;
  Point_d *_rays;
  double *_radii;
  int _nRaysAlloc;

  void _allocRays(int nrays);

};

#endif



