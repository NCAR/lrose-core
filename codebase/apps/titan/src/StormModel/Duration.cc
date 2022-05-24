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
// Duration.cc
//
// Duration object - generate a duration measure.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#include "Duration.hh"
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <rapmath/stats.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_read.h>
using namespace std;

// Constructor

Duration::Duration(const char *prog_name, const Params &params) :
  _params(params)

{
  
  _progName = STRdup(prog_name);
  _dur = (double *) NULL;
  OK = TRUE;

  // initialization of stat distribution generation
  // must be done in calling class - see Generate.cc

  // read in MDV file

  MDV_handle_t mdv;

  if (MDV_init_handle(&mdv)) {
    fprintf(stderr, "ERROR - %s:Duration\n", _progName);
    fprintf(stderr, "Cannot init mdv handle\n");
    OK = FALSE;
    return;
  }

  if (MDV_read_all(&mdv, _params.duration_file_path, MDV_INT8)) {
    fprintf(stderr, "ERROR - %s:Duration\n", _progName);
    fprintf(stderr, "Cannot read mdv file %s\n",
	    _params.duration_file_path);
    MDV_free_handle(&mdv);
    OK = FALSE;
    return;
  }
  
  // allocate dur array
  
  MDV_field_header_t *fld;
  fld = mdv.fld_hdrs + _params.duration_field_num;

  int nX = fld->nx;
  int nY = fld->ny;
  int nGrid = nX * nY;
  
  _dur = (double *) umalloc(nGrid * sizeof(double));

  // load up durMean array

  double scale = fld->scale;
  double bias = fld->bias;

  ui08 *db =
    (ui08 *) mdv.field_plane[_params.duration_field_num][0];
  double *dur = _dur;
  double sum = 0.0;

  for (int i = 0; i < nGrid; i++, db++, dur++) {
    *dur = *db * scale + bias;
    sum += *dur;
  }
  _durMean = sum / (double) nGrid;

  // free up MDV stuff

  MDV_free_handle(&mdv); 

}

// destructor

Duration::~Duration()

{

  STRfree(_progName);

  if (_dur != NULL) {
    ufree(_dur);
  }

}

//////////////////////////////////////////////////
// Generate()
//
// Generate next Dm.

double Duration::Generate(int grid_index)

{

  // generate Duration Measure Dm from gamma

  double Dm;

  Dm = STATS_gamma3_gen(_params.Dm_gamma.shape,
			_params.Dm_gamma.scale,
			_params.Dm_gamma.lbound);
  
  // get duration for the given grid point
  
  double dur = _dur[grid_index];

  // scale Dm by the ratio of this duration to the mean for the grid

  Dm *= (dur / _durMean);

  return (Dm);

}

