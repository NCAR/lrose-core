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
// MidPoint.cc
//
// MidPoint object - generate a mid point.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#include "MidPoint.hh"
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <rapmath/stats.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_read.h>
using namespace std;

// Constructor

MidPoint::MidPoint(const char *prog_name, const Params &params) :
        _params(params)

{

  _progName = STRdup(prog_name);
  _cdf = (double *) NULL;
  OK = TRUE;

  // initialization of stat distribution generation
  // must be done in calling class - see Generate.cc

  // read in MDV file

  MDV_handle_t mdv;

  if (MDV_init_handle(&mdv)) {
    fprintf(stderr, "ERROR - %s:MidPoint\n", _progName);
    fprintf(stderr, "Cannot init mdv handle\n");
    OK = FALSE;
    return;
  }

  if (MDV_read_all(&mdv, _params.mid_point_file_path, MDV_INT8)) {
    fprintf(stderr, "ERROR - %s:MidPoint\n", _progName);
    fprintf(stderr, "Cannot read mdv file %s\n",
	    _params.mid_point_file_path);
    MDV_free_handle(&mdv);
    OK = FALSE;
    return;
  }

  // allocate cdf array

  MDV_field_header_t *fld;
  fld = mdv.fld_hdrs + _params.mid_point_field_num;

  _nX = fld->nx;
  _nY = fld->ny;
  _nGrid = _nX * _nY;
  _minX = fld->grid_minx;
  _minY = fld->grid_miny;
  _dX = fld->grid_dx;
  _dY = fld->grid_dy;
  
  _cdf = (double *) umalloc(_nGrid * sizeof(double));

  // load up cdf array

  double scale = fld->scale;
  double bias = fld->bias;

  ui08 *mid_count =
    (ui08 *) mdv.field_plane[_params.mid_point_field_num][0];

  ui08 *sc = mid_count;
  double *cdf = _cdf;
  double cum = 0.0;

  for (int i = 0; i < _nGrid; i++, sc++, cdf++) {
    cum += *sc * scale + bias;
    *cdf = cum;
  }

  // scale cdf array from 0 to 1

  cdf = _cdf;
  for (int i = 0; i < _nGrid; i++, cdf++) {
    *cdf /= cum;
  }

  // free up MDV stuff

  MDV_free_handle(&mdv); 

}

// destructor

MidPoint::~MidPoint()

{

  STRfree(_progName);

  if (_cdf != NULL) {
    ufree(_cdf);
  }

}

//////////////////////////////////////////////////
// Generate()
//
// Generate next mid point.

void MidPoint::Generate(int *grid_index_p,
			  double *xmid_p, double *ymid_p)

{

  // Get a uniform variate

  double u;

  u = STATS_uniform_gen();

  // find the array index for which the cdf matches this uniform
  // variate

  int index;
  double *cdf = _cdf;
  for (index = 0; index < _nGrid - 1; index++, cdf++) {
    if (*cdf > u) {
      break;
    }
  }
  *grid_index_p = index;

   // compute x and y for that index

  int iy = index / _nY;
  int ix = index - iy * _nX;

  double xx = ix * _dX + _minX;
  double yy = iy * _dY + _minY;

  // compute random variations about the point

  double dxx = STATS_normal_gen(0.0, _params.mid_point_sdev);
  xx += dxx;

  double dyy = STATS_normal_gen(0.0, _params.mid_point_sdev);
  yy += dyy;

  // set return values

  *xmid_p = xx;
  *ymid_p = yy;

  return;

}
