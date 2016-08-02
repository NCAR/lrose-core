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
// Stats.cc
//
// Stats object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "Stats.h"
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>
using namespace std;

//////////////
// Constructor

Stats::Stats(char *prog_name, Params *params) :
  Comps(prog_name, params)

{

  // initialize stats struct
  
  memset (&_stats, 0, sizeof(statistics_t));

  // allocate memory for histogram arrays

  _stats.hist_n_intervals = _params->hist.n_intervals;
  _stats.hist_low_limit = _params->hist.low_limit;
  _stats.hist_interval_size = _params->hist.interval_size;
  
  _stats.n_per_interval =
    (double *) ucalloc (_stats.hist_n_intervals, sizeof(double));
  
  _stats.percent_per_interval =
    (double *) ucalloc (_stats.hist_n_intervals, sizeof(double));

}

/////////////
// destructor

Stats::~Stats()

{

  // free histogram arrays

  ufree(_stats.n_per_interval);
  ufree(_stats.percent_per_interval);

}

////////////////////////////
// update()
//
// Update stats struct
//
// Returns 0 on success, -1 on failure

int Stats::update(char *forecast_file_path)

{
  
  // load up computation grids

  if (_loadGrids(forecast_file_path)) {
    return (-1);
  }

  // accumulate stats

  float *truth_p = _truthGrid->dataArray;
  float *forecast_p = _forecastGrid->dataArray;

  for (int i = 0; i < _truthGrid->npoints; i++, truth_p++, forecast_p++) {

    float truth_val = *truth_p;
    float forecast_val = *forecast_p;

    if (forecast_val >= _params->forecast_level_lower &&
	forecast_val <= _params->forecast_level_upper) {
      
      int interval = (int)
	floor ((truth_val - _stats.hist_low_limit) /
	       _stats.hist_interval_size);

      if (interval >= 0 && interval < _stats.hist_n_intervals) {
	
	_stats.n_total += 1.0;
	_stats.n_per_interval[interval] += 1.0;
	_stats.sumx += truth_val;
	_stats.sum2x += truth_val * truth_val;
	
      } /* if (interval ... */

    } /* if (forecast_val ... */

  } /* i */

  // free up grid objects

  _freeGrids();

  return (0);

}

//////////
// print()
//

void Stats::print(FILE *out)

{

  si32 i;
  double var;
  double lower, upper;

  _stats.mean = _stats.sumx / _stats.n_total;

  var = ((_stats.n_total / (_stats.n_total - 1.0)) *
	 ((_stats.sum2x / _stats.n_total) -
	  pow(_stats.sumx / _stats.n_total, 2.0)));

  if (var < 0.0)
    _stats.sd = 0.0;
  else
    _stats.sd = sqrt(var);

  for (i = 0; i < _stats.hist_n_intervals; i++)
    _stats.percent_per_interval[i] =
      100.0 * (double) _stats.n_per_interval[i] / (double) _stats.n_total;
		   
  fprintf(out, "mean : %g\n", _stats.mean);
  fprintf(out, "sd   : %g\n", _stats.sd);

  fprintf(out, "\n");

  fprintf(out, "%10s %10s %10s %10s\n\n",
	  "lower", "upper", "n", "%");

  for (i = 0; i < _stats.hist_n_intervals; i++) {
    
    lower = _stats.hist_low_limit + i * _stats.hist_interval_size;
    upper = lower + _stats.hist_interval_size;
    
    fprintf(out, "%10g %10g %10g %10g\n",
	    lower, upper,
	    _stats.n_per_interval[i],
	    _stats.percent_per_interval[i]);
    
  } /* i */

}

