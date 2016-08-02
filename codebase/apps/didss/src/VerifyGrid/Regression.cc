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
// Regression.cc
//
// Regression object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "Regression.h"
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>
using namespace std;

//////////////
// Constructor

Regression::Regression(char *prog_name, Params *params) :
  Comps(prog_name, params)

{

}

/////////////
// destructor

Regression::~Regression()

{

}

/////////////
// print

void Regression::print(FILE *out)

{
  fprintf(out, "\n");
}

////////////////////////////
// update()
//
// Update regression struct
//
// Returns 0 on success, -1 on failure

int Regression::update(char *forecast_file_path)

{
  
  // load up computation grids

  if (_loadGrids(forecast_file_path)) {
    return (-1);
  }

  /*
   * print the stats pairs to the file
   */

  time_t truth_time = _truthGrid->timeCent;
  time_t forecast_time = _forecastGrid->timeCent;

  float *dd = _forecastGrid->dataArray;
  float *tt = _truthGrid->dataArray;
  float *rr = _forecastGrid->rangeArray;
  float missing = VGRID_MISSING;

  for (int iy = 0; iy < _params->specified_grid.ny; iy++) {

    for (int ix = 0; ix < _params->specified_grid.nx;
	 ix++, dd++, tt++, rr++) {
      
      if (!_params->check_range || *rr <= _params->max_range) {
	
	if (_params->regression_include_missing) {

	  if (*dd != missing || *tt != missing) {

	    fprintf (stdout, "%ld %ld %g %g\n",
		     (long) truth_time, (long) forecast_time, *tt, *dd);
	    
	  }
	    
	} else if (*tt >= _params->min_regression_val &&
		   *dd >= _params->min_regression_val &&
		   *dd != VGRID_MISSING &&
		   *tt != VGRID_MISSING) {
	  
	  fprintf (stdout, "%ld %ld %g %g\n",
		   (long) truth_time, (long) forecast_time, *tt, *dd);
	  
	} // if (_params->regression_include_missing ...
	  
      } // if (!_params->check_range ...
	       
    } // ix

  } // iy

  return (0);

}

