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
// Circle.cc
//
// Contingency analysis for circular region.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "Circle.h"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pjg.h>
using namespace std;

//////////////
// Constructor

Circle::Circle(char *prog_name, Params *params,
	       char *region_name, double lat, double lon,
	       double radius, double percent_covered_target)

{

  _progName = STRdup(prog_name);
  _params = params;
  _regionName = STRdup(region_name);
  _lat = lat;
  _lon = lon;
  _radius = radius;
  _percentCoveredTarget = percent_covered_target;

  // initialize contingency table
  
  memset (&_cont, 0, sizeof(contingency_t));

}

/////////////
// destructor

Circle::~Circle()

{

  STRfree(_regionName);
  STRfree(_progName);

}

////////////////////////////
// update()
//
// Update contingency table
//
// Returns 0 on success, -1 on failure

int Circle::update(Grid *forecast_grid, Grid *truth_grid,
		   contingency_t *global_cont)

{

  // determine if there is a forecast event and a truth event

  int forecast = _isCovered("forecast", forecast_grid,
			    _params->forecast_level_lower,
			    _params->forecast_level_upper,
			    TRUE);

  int truth = _isCovered("truth", truth_grid,
			 _params->truth_level_lower,
			 _params->truth_level_upper,
			 FALSE);

  // determine outcome

  int success = FALSE;
  int failure = FALSE;
  int false_alarm = FALSE;
  int non_event = FALSE;

  if (truth && forecast) {
    success = TRUE;
  } else if (truth && !forecast) {
    failure = TRUE;
  } else if (!truth && forecast) {
    false_alarm = TRUE;
  } else {
    non_event = TRUE;
  }

  // add to accumulators

  if (forecast) {
    _cont.n_forecast++;
    global_cont->n_forecast++;
  }
  if (truth) {
    _cont.n_truth++;
    global_cont->n_truth++;
  }
  if (success) {
    _cont.n_success++;
    global_cont->n_success++;
  }
  if (failure) {
    _cont.n_failure++;
    global_cont->n_failure++;
  }
  if (false_alarm) {
    _cont.n_false_alarm++;
    global_cont->n_false_alarm++;
  }
  if (non_event) {
    _cont.n_non_event++;
    global_cont->n_non_event++;
  }

  return (0);

}

////////////////
// printHeader()
//

void Circle::printHeader(FILE *out)

{

  fprintf(out, "%10s %5s %5s %7s %7s %7s %5s %5s %5s %5s %5s\n",
	  "region", "n", "n", "n", "n", "n false",
	  "n non", "pod", "far", "csi", "hss");

  fprintf(out, "%10s %5s %5s %7s %7s %7s %5s %5s %5s %5s %5s\n",
	  "name", "fcast", "truth", "success", "failure", "alarm",
	  "event", "", "", "", "");
  
  fprintf(out, "%10s %5s %5s %7s %7s %7s %5s %5s %5s %5s %5s\n",
	  "------", "-----", "-----", "-------", "-------", "-------",
	  "-----", "---", "---", "---", "---");
  
  fflush(out);

}

//////////////
// printCont()
//

void Circle::printCont(FILE *out)

{

  double w, x, y, z;
  double pod, pod_denom;
  double far, far_denom;
  double csi, csi_denom;
  double hss, hss_denom;

  x = _cont.n_success;
  y = _cont.n_failure;
  z = _cont.n_false_alarm;
  w = _cont.n_non_event;

  pod_denom = x + y;
  far_denom = x + z;
  csi_denom = x + y + z;
  hss_denom =  (y * y) + (z * z) + (2.0 * x * w) + (y + z) * (x + w);

  if (pod_denom > 0)
    pod = x / pod_denom;
  else
    pod = 0.0;

  if (far_denom > 0)
    far = z / far_denom;
  else
    far = 0.0;

  if (csi_denom > 0)
    csi = x / csi_denom;
  else
    csi = 0.0;

  if (hss_denom > 0)
    hss = (2.0 * (x * w - y * z)) / hss_denom;
  else
    hss = 0.0;

  fprintf(out, "%10s %5g %5g %7g %7g %7g %5g %5.2f %5.2f %5.2f %5.2f\n",
	  _regionName,
	  _cont.n_forecast,
	  _cont.n_truth,
	  _cont.n_success,
	  _cont.n_failure,
	  _cont.n_false_alarm,
	  _cont.n_non_event,
	  pod, far, csi, hss);

  fflush(out);
  
}

///////////////
// _isCovered()
//
// Determines whether region is sufficiently covered to be an
// event.
//
// Returns TRUE if covered, FALSE if not.

int Circle::_isCovered(const char *label, Grid *grid,
		       double low_thresh, double high_thresh,
		       int is_forecast)
  
{

  // compute (x, y) location of center relative to the grid

  double centerX, centerY;
  PJGLatLon2DxDy(grid->originLat, grid->originLon,
		 _lat, _lon, &centerX, &centerY);

  // compute bounding limits of circle

  double lowX = centerX - _radius;
  double highX = centerX + _radius;
  double lowY = centerY - _radius;
  double highY = centerY + _radius;

  // compute grid search limits

  int ix1 = (int) ((lowX - grid->minx) / grid->dx + 1.0);
  ix1 = MAX(0, ix1);
  ix1 = MIN(grid->nx-1, ix1);

  int ix2 = (int) ((highX - grid->minx) / grid->dx);
  ix2 = MAX(0, ix2);
  ix2 = MIN(grid->nx-1, ix2);

  int iy1 = (int) ((lowY - grid->miny) / grid->dy + 1.0);
  iy1 = MAX(0, iy1);
  iy1 = MIN(grid->ny-1, iy1);

  int iy2 = (int) ((highY - grid->miny) / grid->dy);
  iy2 = MAX(0, iy2);
  iy2 = MIN(grid->ny-1, iy2);

  // sum up hits and total points within radius of circle

  double nhits = 0.0;
  double ntotal = 0.0;

  double rely = grid->miny + iy1 * grid->dy - centerY;

  for (int iy = iy1; iy <= iy2; iy++, rely += grid->dy) {

    float *valp = grid->dataArray + iy * grid->nx + ix1;
    double relx = grid->minx + ix1 * grid->dx - centerX;

    for (int ix = ix1; ix <= ix2; ix++, valp++, relx += grid->dx) {

      double radius = sqrt(rely * rely + relx * relx);
      
      if (radius <= _radius) {
	ntotal++;
	if (*valp >= low_thresh && *valp <= high_thresh) {
	  nhits++;
	}
      }
      
    } // ix
    
  } // iy

  // cpmpute percentage covered

  double perc_cov;
  if (ntotal > 0) {
    perc_cov = (nhits / ntotal) * 100.0;
  } else {
    perc_cov = 0.0;
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr,
	    "Region %10s: %8s: nhits %5g, ntotal %5g, perc_cov %5g\n",
	    _regionName, label, nhits, ntotal, perc_cov);
  }

  if (is_forecast) {
    _percentCoveredForecast = perc_cov;
  } else {
    _percentCoveredTruth = perc_cov;
  }

  if (perc_cov >= _percentCoveredTarget) {
    return (TRUE);
  } else {
    return (FALSE);
  }

}
