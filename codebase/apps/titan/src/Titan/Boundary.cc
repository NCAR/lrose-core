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
// Boundary.cc
//
// Boundary class
//
// Provides services for run boundary identification.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "Boundary.hh"
#include <toolsa/umisc.h>
using namespace std;

//////////////
// constructor
//

Boundary::Boundary(const string &prog_name, const Params &params) :
  Worker(prog_name, params)

{

  OK = TRUE;

  _rowh = NULL;
  _nRowsAlloc = 0;
  _intervals = NULL;
  _nIntervalsAlloc = 0;
  
  _starRays = NULL;
  _rays = NULL;
  _radii = NULL;
  _nRaysAlloc = 0;


  _bdryList = NULL;
  _bdryPts = NULL;
  _nodes = NULL;
  _nNodesAlloc = 0;

}

/////////////
// destructor
//

Boundary::~Boundary()

{

  if (_starRays) {
    ufree(_starRays);
  }
  if (_rays) {
    ufree(_rays);
  }
  if (_radii) {
    ufree(_radii);
  }

  EG_free_nodes(&_nNodesAlloc, &_bdryList, &_bdryPts, &_nodes);
  EG_free_rowh(&_nRowsAlloc, &_rowh);
  EG_free_intervals(&_intervals, &_nIntervalsAlloc);

}

/////////////////////////////////////
// computeRadii()
//
// Compute the radii to the boundary
// Return pointer to array of radii

double *Boundary::computeRadii(int nx, int ny, ui08 *grid, int threshold,
			       int n_poly_sides, double ref_x, double ref_y)

{

  // check memory allocation

  EG_alloc_rowh(ny, &_nRowsAlloc, &_rowh);

  // get the intervals
  
  _nIntervals =
    EG_find_intervals(ny, nx, grid, &_intervals, &_nIntervalsAlloc,
		      _rowh, threshold);

  // allocate the rays

  _allocRays(n_poly_sides);

  // alloc nodes

  int num_nodes = 4 * _nIntervals;
  
  EG_alloc_nodes(num_nodes, &_nNodesAlloc,
		 &_bdryList, &_bdryPts, &_nodes);

  // call bdry_graph to set up the graph for the boundary
  
  if (EG_bdry_graph(_rowh, ny, nx, _nodes, num_nodes, 0)) {
    fprintf(stderr, "ERROR - %s:Boundary::compute\n", _progName.c_str());
    fprintf(stderr, "Cannot compute boundary\n");
  }
  
  // traverse the graph to determine the boundary
  
  int bdry_size = EG_traverse_bdry_graph(_nodes, 2, _bdryList);

  // initialize the array of rays

  Point_d refPt;
  refPt.x = ref_x;
  refPt.y = ref_y;
  double theta = EG_init_ray_TN(_rays, n_poly_sides, &refPt);
  
  // generate the array of boundary points from the array of
  // boundary nodes and a bdry_list
  
  EG_gen_bdry(_bdryPts, _nodes, _bdryList, bdry_size);
  
  // determine the intersections of the rays with the boundary and
  // store this information in star_ray
  
  EG_make_star_TN(_bdryPts, bdry_size, _rays, n_poly_sides,
		  theta, &refPt, _starRays);

  // copy to radii

  for (int i = 0; i < n_poly_sides; i++) {
    _radii[i] = _starRays[i].r;
  }

  return(_radii);

}

///////////////
// _allocRays()
//

void Boundary::_allocRays(int nrays)

{
  if (nrays > _nRaysAlloc) {
    _starRays = (Star_point *)
      urealloc (_starRays, (nrays + 1) * sizeof(Star_point));
    _rays = (Point_d *)
      urealloc (_rays, nrays * sizeof(Point_d));
    _radii = (double *)
      urealloc (_radii, nrays * sizeof(double));
    _nRaysAlloc = nrays;
  }
  memset(_radii, 0, nrays * sizeof(double));
}

