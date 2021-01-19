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
// Velocity.cc
//
// Velocity object - generate a velocity.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#include "Velocity.hh"
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <rapmath/stats.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_read.h>
using namespace std;

// Constructor

Velocity::Velocity(const char *prog_name, const Params &params) :
        _params(params)

{
  
  _progName = STRdup(prog_name);
  _uMean = (double *) NULL;
  _vMean = (double *) NULL;
  OK = TRUE;

  // initialization of stat distribution generation
  // must be done in calling class - see Generate.cc

  // read in MDV file

  MDV_handle_t mdv;

  if (MDV_init_handle(&mdv)) {
    fprintf(stderr, "ERROR - %s:Velocity\n", _progName);
    fprintf(stderr, "Cannot init mdv handle\n");
    OK = FALSE;
    return;
  }

  if (MDV_read_all(&mdv, _params.velocity_file_path, MDV_INT8)) {
    fprintf(stderr, "ERROR - %s:Velocity\n", _progName);
    fprintf(stderr, "Cannot read mdv file %s\n",
	    _params.velocity_file_path);
    MDV_free_handle(&mdv);
    OK = FALSE;
    return;
  }
  
  // allocate u and v arrays
  
  MDV_field_header_t *fld;
  fld = mdv.fld_hdrs + _params.u_mean_field_num;

  int nX = fld->nx;
  int nY = fld->ny;
  int nGrid = nX * nY;
  
  _uMean = (double *) umalloc(nGrid * sizeof(double));
  _vMean = (double *) umalloc(nGrid * sizeof(double));

  // load up uMean array

  fld = mdv.fld_hdrs + _params.u_mean_field_num;
  double u_scale = fld->scale;
  double u_bias = fld->bias;

  ui08 *uByte =
    (ui08 *) mdv.field_plane[_params.u_mean_field_num][0];
  ui08 *ub = uByte;
  double *u = _uMean;

  for (int i = 0; i < nGrid; i++, ub++, u++) {
    *u = *ub * u_scale + u_bias;
  }

  // load up vMean array

  fld = mdv.fld_hdrs + _params.v_mean_field_num;
  double v_scale = fld->scale;
  double v_bias = fld->bias;

  ui08 *vByte =
    (ui08 *) mdv.field_plane[_params.v_mean_field_num][0];
  ui08 *vb = vByte;
  double *v = _vMean;

  for (int i = 0; i < nGrid; i++, vb++, v++) {
    *v = *vb * v_scale + v_bias;
  }

  // free up MDV stuff

  MDV_free_handle(&mdv); 

}

// destructor

Velocity::~Velocity()

{

  STRfree(_progName);

  if (_uMean != NULL) {
    ufree(_uMean);
  }

  if (_vMean != NULL) {
    ufree(_vMean);
  }

}

//////////////////////////////////////////////////
// Generate()
//
// Generate next velocity.

void Velocity::Generate(int grid_index, double *u_p, double *v_p)

{

  // get mean U and V for the given grid point

  double uMean = _uMean[grid_index];
  double vMean = _vMean[grid_index];

  // generate u and v from normal distribution

  double u = STATS_normal_gen(uMean, _params.u_sdev);
  double v = STATS_normal_gen(vMean, _params.v_sdev);

  *u_p = u;
  *v_p = v;

  return;

}
