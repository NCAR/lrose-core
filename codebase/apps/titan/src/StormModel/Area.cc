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
// Area.cc
//
// Area object - generate a area measure.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#include "Area.hh"
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <rapmath/stats.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_read.h>
using namespace std;

// Constructor

Area::Area(const char *prog_name, const Params &params) :
        _params(params)

{
  
  _progName = STRdup(prog_name);
  _lnArea = (double *) NULL;
  OK = TRUE;

  // initialization of stat distribution generation
  // must be done in calling class - see Generate.cc

  // read in MDV file

  MDV_handle_t mdv;

  if (MDV_init_handle(&mdv)) {
    fprintf(stderr, "ERROR - %s:Area\n", _progName);
    fprintf(stderr, "Cannot init mdv handle\n");
    OK = FALSE;
    return;
  }

  if (MDV_read_all(&mdv, _params.ln_area_file_path, MDV_INT8)) {
    fprintf(stderr, "ERROR - %s:Area\n", _progName);
    fprintf(stderr, "Cannot read mdv file %s\n",
	    _params.ln_area_file_path);
    MDV_free_handle(&mdv);
    OK = FALSE;
    return;
  }
  
  // allocate area array
  
  MDV_field_header_t *fld;
  fld = mdv.fld_hdrs + _params.ln_area_field_num;

  int nX = fld->nx;
  int nY = fld->ny;
  int nGrid = nX * nY;
  
  _lnArea = (double *) umalloc(nGrid * sizeof(double));

  // load up lnArea array

  double scale = fld->scale;
  double bias = fld->bias;

  ui08 *db =
    (ui08 *) mdv.field_plane[_params.ln_area_field_num][0];
  double *lnArea = _lnArea;
  double sum = 0.0;
  double sum2 = 0.0;

  for (int i = 0; i < nGrid; i++, db++, lnArea++) {
    double lna = *db * scale + bias;
    *lnArea = lna;
    sum += lna;
    sum2 += lna * lna;
  }

  // compute mean and sdev

  double n = nGrid;
  _lnAreaMean = sum / n;
  double var = (sum2 / n) - (_lnAreaMean * _lnAreaMean);
  if (var < 0) {
    _lnAreaSdev = 0.0;
  } else {
    _lnAreaSdev = sqrt(var) * (n / (n - 1.0));
  }

  // free up MDV stuff

  MDV_free_handle(&mdv); 

}

// destructor

Area::~Area()

{

  STRfree(_progName);

  if (_lnArea != NULL) {
    ufree(_lnArea);
  }

}

//////////////////////////////////////////////////
// Generate()
//
// Generate next A.

double Area::Generate(int grid_index, double Dm)

{

  // Generate mean area A from gamma
  
  double lnA_shape, lnA_scale, lnA_lbound;
  double A;
  double lnA;
  
  _interp_lnA(Dm, &lnA_shape, &lnA_scale, &lnA_lbound);

  lnA = _params.max_lnA * 2.0;

  while (lnA > _params.max_lnA) {

    lnA = STATS_gamma3_gen(lnA_shape, lnA_scale, lnA_lbound);

    // get mean area for the given grid point
    
    double lnAreaThisPoint = _lnArea[grid_index];
    
    // adjust by rel_delta times the sdev
    
    // lnA += rel_delta * _lnAreaSdev;
    
    lnA *= (lnAreaThisPoint / _lnAreaMean);

  } // while
  
  // compute A

  A = exp(lnA);

  return (A);

}

//////////////////////////////////////////////////
// _interp_lnA()
//
// Interpolate for ln(A) from Dm
//
// Sets shape, scale and lbound params for 3-param gamma of
// ln(A) conditional on Dm.

void Area::_interp_lnA(double Dm, double *shape_p,
		       double *scale_p, double *lbound_p)

{

  Params::lnA_vs_Dm_t *lnA = _params._lnA_vs_Dm;
  int npts = _params.lnA_vs_Dm_n;

  if (Dm < lnA[0].Dm) {

    // Dm below range - use lowest value

    *shape_p = lnA[0].shape;
    *scale_p = lnA[0].scale;
    *lbound_p = lnA[0].lbound;
    return;
  }

  if (Dm > lnA[npts-1].Dm) {

    // Dm above range - use highest value

    *shape_p = lnA[npts-1].shape;
    *scale_p = lnA[npts-1].scale;
    *lbound_p = lnA[npts-1].lbound;
    return;
  }

  // Dm in range - interpolate

  for (int i = 1; i < npts; i++) {
    if ((Dm >= lnA[i-1].Dm) && (Dm <= lnA[i].Dm)) {
      double frac = (Dm - lnA[i-1].Dm) / (lnA[i].Dm - lnA[i-1].Dm);
      *shape_p = lnA[i-1].shape + frac * (lnA[i].shape - lnA[i-1].shape);
      *scale_p = lnA[i-1].scale + frac * (lnA[i].scale - lnA[i-1].scale);
      *lbound_p = lnA[i-1].lbound + frac * (lnA[i].lbound - lnA[i-1].lbound);
      return;
    }
  }

  return;

}

