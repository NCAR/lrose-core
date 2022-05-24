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
// StatsGrid.cc
//
// Loads up stats grid, compute stats and writes out MDV file.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////

#include "StatsGrid.hh"
#include <toolsa/str.h>
#include <toolsa/TaArray.hh>
#include <rapmath/math_macros.h>
#include <rapmath/umath.h>
#include <Mdv/MdvxField.hh>
#include <rapmath/stats.h>
using namespace std;

#define SET_LIMITS(x, low, high) if ((x) < (low)) (x) = (low); \
else if ((x) > (high)) (x) = (high)
  
#define SMALL_ANGLE 0.000001
#define PI_BY_TWO 1.570796327
#define ALMOST_PI_BY_TWO (PI_BY_TWO - SMALL_ANGLE)
#ifndef PI
#define PI 3.141592654
#endif
#define ALMOST_PI (PI - SMALL_ANGLE)
  
///////////////
// Constructor

StatsGrid::StatsGrid (const string &prog_name,
		      const Params &params,
		      const Args &args,
		      TrackData &trackData,
		      time_t startTime,
		      time_t endTime) :
  _progName(prog_name),
  _params(params),
  _args(args),
  _trackData(trackData),
  _startTime(startTime),
  _endTime(endTime)
  
{

  // initialize
  
  _stats = NULL;
  
  // load up track data

  _loadTrackData();
  
}

/////////////
// Destructor

StatsGrid::~StatsGrid()

{

  if (_stats) {
    ufree2((void **) _stats);
  }

}

///////////////////////////////////
// load up data from tracks
//

void StatsGrid::_loadTrackData()

{

  bool timesInit = false;

  double n_start, n_end;
  double distance_x, distance_y;
  double start_x, start_y;
  double end_x, end_y;
  double sum_start_x, sum_start_y;
  double sum_end_x, sum_end_y;

  const Mdvx::coord_t &grid = _trackData.getGrid();
  
  // allocate storm and precip grids
  
  ui08 ** storm_grid = NULL;
  double **precip_grid = NULL;
      
  // loop through tracks

  int no_more_tracks = FALSE;
  int n_tracks = 0;
  double sum_area = 0.0;

  while (!no_more_tracks) {

    if (_trackData.loadNextTrack(&no_more_tracks) || no_more_tracks) {
      continue;
    }

    // continue to next track if this one is too short
    
    if (_trackData.durationInSecs < _params.min_duration) {
      continue;
    }
    double duration_in_hr = (double) _trackData.durationInSecs / 3600.0;
    
    /*
     * initialize
     */
    
    n_tracks++;
    n_start = 0.0;
    n_end = 0.0;
    sum_start_x = 0.0;
    sum_start_y = 0.0;
    sum_end_x = 0.0;
    sum_end_y = 0.0;
    
    // loop through track entries
    
    int no_more_entries = FALSE;
    
    while (!no_more_entries) {

      if (_trackData.loadNextEntry(&no_more_entries) || no_more_entries) {
	continue;
      }
      
      // allocate grids first time through
      
      if (!_stats) {
	cerr << "alloc _stats, nx, ny: " << grid.nx << ", " << grid.ny << endl;
	_stats = (grid_stats_t **) ucalloc2
	  (grid.ny, grid.nx, sizeof(grid_stats_t));
      }
      
      // allocate storm and precip grids
      
      if (!storm_grid) {
	storm_grid = (ui08 **) ucalloc2 (grid.ny, grid.nx, sizeof(ui08));
      }
      if (!precip_grid) {
	precip_grid = (double **) ucalloc2 (grid.ny, grid.nx, sizeof(double));
      }
      
      if (_trackData.entryTime < _startTime ||
	  _trackData.entryTime > _endTime) {
	continue;
      }

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, "%s: %10g %10g %10g %10g %10g\n",
		utimstr(_trackData.entryTime),
		_trackData.centroidX, _trackData.centroidY,
		_trackData.area,
		_trackData.majorRadius, _trackData.minorRadius);
      }
      
      sum_area += _trackData.area;
      
      if (!timesInit) {
	_dataStart = _trackData.entryTime;
	_dataEnd = _trackData.entryTime;
	timesInit = true;
      } else {
	_dataStart = MIN(_dataStart, _trackData.entryTime);
	_dataEnd = MAX(_dataEnd, _trackData.entryTime);
      }
      
      int ix =
	(int) floor((_trackData.centroidX - grid.minx) / grid.dx + 0.5);
      int iy =
	(int) floor((_trackData.centroidY - grid.miny) / grid.dy + 0.5);
      
      if (iy >= 0 && iy < grid.ny && ix >= 0 && ix < grid.nx) {
	
	grid_stats_t *stat = _stats[iy] + ix;
	stat->n_events++;
	
	// deal with the centroid first
	
	if (_trackData.entryHistoryInScans == 1) {
	  
	  stat->n_start++;
	  
	  sum_start_x += _trackData.centroidX;
	  sum_start_y += _trackData.centroidY;
	  n_start++;
	  
	} /* if (_trackData.historyInScans == 1) */
	
	if (_trackData.entryHistoryInScans ==
	    _trackData.durationInScans / 2) {
	  stat->n_mid++;
	}
	
	if (_trackData.entryTime == _trackData.endTime) {
	  
	  sum_end_x += _trackData.centroidX;
	  sum_end_y += _trackData.centroidY;
	  n_end++;
	  
	} /* if (_trackData.historyInScans .... */
	
      } /* if (iy >= 0 ... */
      
      // initialize storm grid
      
      memset(*storm_grid, 0, grid.ny * grid.nx * sizeof(ui08));
      memset(*precip_grid, 0, grid.ny * grid.nx * sizeof(double));

      // load up storm and precip grids

      si32 start_ix, start_iy;
      si32 end_ix, end_iy;

      if (_params.spatial_representation == Params::STORM_RUNS &&
	  _trackData.nProjRuns > 0) {
	_loadFromRuns(start_ix, start_iy, end_ix, end_iy,
		      storm_grid, precip_grid);
      } else {
	_loadFromEllipse(start_ix, start_iy, end_ix, end_iy,
			 storm_grid, precip_grid);
      }
      
      // update the stats for the grid points in the storm
      
      for (int iiy = start_iy; iiy <= end_iy; iiy++) {
	
	double *precip = precip_grid[iiy] + start_ix;
	ui08 *storm = storm_grid[iiy] + start_ix;
	grid_stats_t *stat = _stats[iiy] + start_ix;
	
	for (int iix = start_ix; iix <= end_ix;
	     iix++, precip++, storm++, stat++) {
	  
	  if (*storm) {
	    
	    stat->n_weighted++;
	    
	    stat->precip += *precip;
	    
	    double u = _trackData.dxDt;
	    double v = _trackData.dyDt;
	    
	    if (grid.proj_type == Mdvx::PROJ_LATLON) {
	      u *= KM_PER_DEG_AT_EQ * cos(_trackData.centroidY * DEG_TO_RAD);
	      v *= KM_PER_DEG_AT_EQ;
	    }
	    
	    stat->u += u;
	    stat->v += v;
	    
	    stat->speed += sqrt(u * u + v * v);
	    stat->dbz_max += _trackData.dbzMax;
	    stat->tops += _trackData.tops;
	    stat->volume += _trackData.volume;
	    stat->area += _trackData.area;
	    stat->duration += duration_in_hr;
	    stat->ln_area += log(_trackData.area);

	    double aspect = _trackData.majorRadius / _trackData.minorRadius;
	    stat->ellipse_u +=
	      aspect * 10.0 * sin(_trackData.ellipseOrientation * DEG_TO_RAD);
	    stat->ellipse_v +=
	      aspect * 10.0 * cos(_trackData.ellipseOrientation * DEG_TO_RAD);
	    
	  }

	} /* iix */
	
      } /* iiy */

    } // while (!no_more_entries)
    
    // compute start and end points for complex track

    if (n_start == 0) {
      start_x = 0.0;
      start_y = 0.0;
    } else {
      start_x = sum_start_x / n_start;
      start_y = sum_start_y / n_start;
    }
    if (n_end == 0) {
      end_x = 0.0;
      end_y = 0.0;
    } else {
      end_x = sum_end_x / n_end;
      end_y = sum_end_y / n_end;
    }
    
    int ix = (int) floor((start_x - grid.minx) / grid.dx + 0.5);
    int iy = (int) floor((start_y - grid.miny) / grid.dy + 0.5);
    
    if (iy >= 0 && iy < grid.ny && ix >= 0 && ix < grid.nx) {
      
      grid_stats_t *stat = _stats[iy] + ix;
      stat->n_complex++;
      
      distance_x = end_x - start_x;
      distance_y = end_y - start_y;
      
      stat->distance +=
	sqrt(distance_x * distance_x + distance_y * distance_y);
      stat->dx += distance_x;
      stat->dy += distance_y;
      
    }
      
  } // if (!no_more_tracks)

  if (_params.debug) {
    fprintf(stderr, "N tracks processed: %d\n", n_tracks);
    fprintf(stderr, "Sum area: %g\n", sum_area);
  }

  // compute estimated number of scans elapsed

  _nScansElapsed = ((double) (_dataEnd - _dataStart) /
		    _trackData.scanIntervalSecs);

  // free up tmp grids
  
  ufree2((void **) storm_grid);
  ufree2((void **) precip_grid);
      
}

////////////
// compute()
//

void StatsGrid::compute()

{

  if (!_stats) {
    return;
  }

  if (_params.debug) {
    cerr << "Computing stats" << endl;
  }

  // compute densities for n_start, n_mid and n_events

  _computeDensities();

  // loop through grid, computing stats
  
  grid_stats_t *stat;
  const Mdvx::coord_t &grid = _trackData.getGrid();
  
  for (int iy = 0; iy < grid.ny; iy++) {
    
    stat = _stats[iy];
    
    for (int ix = 0; ix < grid.nx; ix++, stat++) {

      if (stat->n_weighted > 0) {
	if (_nScansElapsed > 0) {
	  stat->percent_activity =
	    ((stat->n_weighted / _nScansElapsed) * 100.0);
	} else {
	  stat->percent_activity = 0.0;
	}
	stat->u /= stat->n_weighted;
	stat->v /= stat->n_weighted;
	stat->speed /= stat->n_weighted;
	stat->volume /= stat->n_weighted;
	stat->area /= stat->n_weighted;
	stat->ln_area /= stat->n_weighted;
	stat->dbz_max /= stat->n_weighted;
	stat->tops /= stat->n_weighted;
	stat->duration /= stat->n_weighted;
	stat->ellipse_u /= stat->n_weighted;
	stat->ellipse_v /= stat->n_weighted;
      }

      if (stat->n_complex > 0) {
	stat->distance /= stat->n_complex;
	stat->dx /= stat->n_complex;
	stat->dy /= stat->n_complex;
      }

      if (_params.n_seasons > 0) {
	stat->precip /= (double) _params.n_seasons;
	stat->n_events /= (double) _params.n_seasons;
	stat->n_weighted /= (double) _params.n_seasons;
	stat->n_complex /= (double) _params.n_seasons;
	stat->n_start /= (double) _params.n_seasons;
	stat->n_mid /= (double) _params.n_seasons;
      }
      
    } // ix
    
  } // iy

}

/////////////////////////////////////////////////////////////
// _loadFromRuns()
//
// Loads up the grid from the ellipse shape.

void StatsGrid::_loadFromRuns(si32 &start_ix,
			      si32 &start_iy,
			      si32 &end_ix,
			      si32 &end_iy,
			      ui08 **storm_grid,
			      double **precip_grid)

{

  
  // load up the storm grid
  
  _setRunsInGrid(start_ix, start_iy,
		 end_ix, end_iy,
		 storm_grid);
  
  // now update the precip grid
  
  if (_params.compute_precip_from_dbz_histogram) {
    _setRunsPrecipFromHist(storm_grid, precip_grid);
  } else {
    _setRunsPrecipFromFlux(storm_grid, precip_grid);
  }
  
}

/////////////////////////////////////////////////////////////
// _loadFromEllipse()
//
// Loads up the grid from the ellipse shape.

void StatsGrid::_loadFromEllipse(si32 &start_ix,
				 si32 &start_iy,
				 si32 &end_ix,
				 si32 &end_iy,
				 ui08 **storm_grid,
				 double **precip_grid)

{

  
  // load up the storm grid
  
  double ellipse_x = _trackData.centroidX;
  double ellipse_y = _trackData.centroidY;
  
  double major_radius, minor_radius, axis_rotation;

  if (_params.override_ellipse) {
    major_radius = _params.circle_radius;
    minor_radius = _params.circle_radius;
    axis_rotation = 0.0;
  } else {
    major_radius = _trackData.majorRadius;
    minor_radius = _trackData.minorRadius;
    axis_rotation = _trackData.ellipseOrientation;
  }
  
  _setEllipseInGrid(ellipse_x,
		    ellipse_y,
		    major_radius,
		    minor_radius,
		    axis_rotation,
		    start_ix, start_iy,
		    end_ix, end_iy,
		    storm_grid);
  
  // now update the precip grid
  
  if (_params.compute_precip_from_dbz_histogram) {
    _setEllipsePrecipFromHist(ellipse_x,
			      ellipse_y,
			      major_radius,
			      minor_radius,
			      axis_rotation,
			      precip_grid);
  } else {
    _setEllipsePrecipFromFlux(start_ix, start_iy,
			      end_ix, end_iy,
			      storm_grid,
			      precip_grid);
  }
  
}

/////////////////////////////////////////////////////////////
// Updates the precip grid based on the storm grid, using the
// reflectivity histogram to determine reflectivity

void StatsGrid::_setRunsPrecipFromHist(ui08 **storm_grid,
				       double **precip_grid)
  
{

  // set up lookup table for cumulative reflectivity histogram
  // We use a granularity of 0.1%, so we need 1000 points to
  // go from 0 to 100 %.

  double low_dbz_threshold = _trackData.lowDbzThreshold;
  double dbz_hist_interval = _trackData.dbzHistInterval;
  int n_dbz_intervals = _trackData.nDbzIntervals;
  double *areaHist = _trackData.areaHist;
  double area_fraction = 0.0;
  double prev_fraction = area_fraction;
  double prev_dbz = low_dbz_threshold;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "low_dbz_threshold: " << low_dbz_threshold << endl;
    cerr << "dbz_hist_interval: " << dbz_hist_interval << endl;
    cerr << "n_dbz_intervals:" << n_dbz_intervals << endl;
  }

  double refl[1000];
  for (int i = 0; i < 1000; i++) {
    refl[i] = low_dbz_threshold;
  }

  for (int interval = 0; interval < n_dbz_intervals; interval++) {
    
    double dbz = low_dbz_threshold + ((double) interval + 0.5) * dbz_hist_interval;
    double delta_dbz = dbz - prev_dbz;
    double delta_fraction = areaHist[interval] / 100.0;
    area_fraction += delta_fraction;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "dbz, fraction, cuml: " << dbz << ", " << delta_fraction
	 << ", " << area_fraction << endl;
  }

    for (double frac = prev_fraction; frac < area_fraction; frac += 0.001) {
      int ifrac = (int) (frac / 0.001);
      if (ifrac > 999) {
	ifrac = 999;
      }
      refl[ifrac] =
	prev_dbz + delta_dbz * ((frac - prev_fraction) / delta_fraction);
    }

    prev_dbz = dbz;
    prev_fraction = area_fraction;

  } // interval

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (int i = 0; i < 1000; i++) {
      cerr << refl[i] << " ";
    }
    cerr << endl;
  }

  // loop through the grid points, randomly assigning the point a 
  // reflectivity value, and convert to precip

  const Mdvx::coord_t &grid = _trackData.getGrid();
  STATS_uniform_seed(98765432);

  for (int iy = 0; iy < grid.ny; iy++) {
    for (int ix = 0; ix < grid.nx; ix++) {

      if (storm_grid[iy][ix]) {

	double uu = STATS_uniform_gen();
	int ii = (int) (uu / 0.001);
	if (ii > 999) {
	  ii = 999;
	}
	double dbz = refl[ii];
	double z = pow(10.0, dbz / 10.0);
	double precip_rate = pow((z / _params.z_r_coeff),
				 (1.0 / _params.z_r_exponent));
	double precip_depth =
	  (precip_rate * (double) _trackData.scanIntervalSecs) / 3600.0;
	
	precip_grid[iy][ix] = precip_depth;
	
      } // if (storm_grid[iy][ix])
      
    } // ix
  } // iy

}

  
/////////////////////////////////////////////////////////////
// Updates the precip grid based on the storm precip flux

void StatsGrid::_setRunsPrecipFromFlux(ui08 **storm_grid,
				       double **precip_grid)
  
{

  // compute precip vol in meters cubed

  double precipVolM3 = _trackData.precipFlux * _trackData.scanIntervalSecs;

  // compute precip depth in mm

  double precipDepthMm = 0.0;
  if (_trackData.precipArea > 0) {
    precipDepthMm = (precipVolM3 / _trackData.precipArea) / 1000.0;
  }

  // loop through the grid points, assigning the point a 
  // precip value

  const Mdvx::coord_t &grid = _trackData.getGrid();

  for (int iy = 0; iy < grid.ny; iy++) {
    for (int ix = 0; ix < grid.nx; ix++) {
      
      if (storm_grid[iy][ix]) {
	precip_grid[iy][ix] = precipDepthMm;
      }
      
    } // ix
  } // iy

}

  
/////////////////////////////////////////////////////////////
// Updates the grids based on the storm precip area ellipses,
// and uses the reflectivity distribution to estimate the
// precip swath
//

void StatsGrid::_setEllipsePrecipFromHist(double ellipse_x,
					  double ellipse_y,
					  double major_radius,
					  double minor_radius,
					  double axis_rotation,
					  double **precip_grid)
  
{

  // alloc grids

  const Mdvx::coord_t &grid = _trackData.getGrid();
  ui08 **pos_grid = (ui08 **) ucalloc2 (grid.ny, grid.nx, sizeof(ui08));

  // init

  double low_dbz_threshold = _trackData.lowDbzThreshold;
  double dbz_hist_interval = _trackData.dbzHistInterval;
  int n_dbz_intervals = _trackData.nDbzIntervals;

  // compute the area and incremental precip depth for
  // each reflectivity interval

  double area_fraction = 0.0;
  double *areaHist = _trackData.areaHist;

  for (int interval = n_dbz_intervals - 1; interval >= 0; interval--) {
    
    double low_dbz = low_dbz_threshold + interval * dbz_hist_interval;
    double mean_dbz = low_dbz + dbz_hist_interval / 2.0;
    if (mean_dbz > _params.hail_dbz_threshold) {
      mean_dbz = _params.hail_dbz_threshold;
    }
    
    double z = pow(10.0, low_dbz / 10.0);
    double precip_rate = pow((z / _params.z_r_coeff),
			     (1.0 / _params.z_r_exponent));
    double precip_depth =
      (precip_rate * (double) _trackData.scanIntervalSecs) / 3600.0;

    area_fraction += areaHist[interval] / 100.0;

    // set points in position grid

    double frac_major_radius = major_radius * sqrt(area_fraction);
    double frac_minor_radius = minor_radius * sqrt(area_fraction);
    
    si32 start_ix, start_iy;
    si32 end_ix, end_iy;
    
    _setEllipseInGrid(ellipse_x,
		      ellipse_y,
		      frac_major_radius,
		      frac_minor_radius,
		      axis_rotation,
		      start_ix, start_iy,
		      end_ix, end_iy,
		      pos_grid);
    
    // accum the precip
    
    for (int iy = start_iy; iy <= end_iy; iy++) {
      ui08 *pos = pos_grid[iy] + start_ix;
      double *precip = precip_grid[iy] + start_ix;
      for (int ix = start_ix; ix <= end_ix; ix++, pos++, precip++) {
	if (*pos && (*precip == 0.0)) {
 	  *precip = precip_depth;
	}
      } // ix
    } // iy

  } /* interval */

  // free grids

  ufree2((void **) pos_grid);

}

/////////////////////////////////////////////////////////////
// Updates the precip based on the storm precip area ellipses,
// and use the storm precip flux

void StatsGrid::_setEllipsePrecipFromFlux(si32 start_ix,
					  si32 start_iy,
					  si32 end_ix,
					  si32 end_iy,
					  ui08 **storm_grid,
					  double **precip_grid)
  
{

  // compute precip vol in meters cubed

  double precipVolM3 = _trackData.precipFlux * _trackData.scanIntervalSecs;

  // compute precip depth in mm

  double precipDepthMm = 0.0;
  if (_trackData.precipArea > 0) {
    precipDepthMm = (precipVolM3 / _trackData.precipArea) / 1000.0;
  }

  for (int iy = start_iy; iy <= end_iy; iy++) {
    for (int ix = start_ix; ix <= end_ix; ix++) {
      if (storm_grid[iy][ix]) {
	precip_grid[iy][ix] += precipDepthMm;
      }
    } // ix
  } // iy

}

/*********************************************************************
 * set_ellipse_grid()
 *
 * Flags regions in the grid with 1's if the given
 * ellipse crosses into or contains the region.
 *
 * The method uses 3 steps.
 *
 * 1) Flag the region containing the ellipse centroid.
 *
 * 2) Consider all vertical grid lines which intersect the ellipse.
 *    Flag all regions on either side of such a line for that
 *    line segment which crosses the ellipse.
 *
 * 3) Consider all horizontal grid lines which intersect the ellipse.
 *    Flag all regions on either side of such a line for that
 *    line segment which crosses the ellipse.
 *
 *********************************************************************/

void StatsGrid::_setEllipseInGrid(double ellipse_x,
				  double ellipse_y,
				  double major_radius,
				  double minor_radius,
				  double axis_rotation,
				  si32 &start_ix,
				  si32 &start_iy,
				  si32 &end_ix,
				  si32 &end_iy,
				  ui08 **target_grid)
  
{
  
  si32 ix, iy, ix1, iy1, ix2, iy2;

  double grid_rotation, theta;
  double slope_prime, intercept_prime;
  double sin_rotation, cos_rotation, tan_rotation;

  double xprime1, yprime1, xprime2, yprime2;
  double x_1, y_1, x_2, y_2;
  double start_x, start_y;
  double end_x, end_y;
  double line_x, line_y;

  const Mdvx::coord_t &grid = _trackData.getGrid();

  // clear grid

  memset(*target_grid, 0, grid.nx * grid.ny * sizeof(ui08));

  // compute the grid_rotation, taking care to avoid 0, pi/2 and
  // pi, so that the trig functions will not fail. Remember that
  // the axis_rotation is relative to True North, and we need to
  // compute the grid rotation relative to the mathmatically
  // conventional axes

  theta = 90.0 - axis_rotation;

  if (theta == 0.0)
    grid_rotation = SMALL_ANGLE;
  else if (theta == 90.0)
    grid_rotation = ALMOST_PI_BY_TWO;
  else if (theta == -90.0)
    grid_rotation = - ALMOST_PI_BY_TWO;
  else if (theta == 180.0 || theta == -180.0)
    grid_rotation = ALMOST_PI;
  else
    grid_rotation = theta * DEG_TO_RAD;

  sin_rotation = sin(grid_rotation);
  cos_rotation = cos(grid_rotation);
  tan_rotation = tan(grid_rotation);
  
  // compute the start and end x and y - these values are
  // chosen for a circle of radius major_radius, which will
  // enclose the ellipse

  start_x = ellipse_x - major_radius;
  start_y = ellipse_y - major_radius;

  end_x = ellipse_x + major_radius;
  end_y = ellipse_y + major_radius;

  // set the end and start grid indices

  start_ix = (si32) floor((start_x - grid.minx) / grid.dx + 0.49);
  SET_LIMITS(start_ix, 0, grid.nx - 1);

  start_iy = (si32) floor((start_y - grid.miny) / grid.dy + 0.49);
  SET_LIMITS(start_iy, 0, grid.ny - 1);

  end_ix = (si32) floor((end_x - grid.minx) / grid.dx + 0.51);
  SET_LIMITS(end_ix, 0, grid.nx - 1);

  end_iy = (si32) floor((end_y - grid.miny) / grid.dy + 0.51);
  SET_LIMITS(end_iy, 0, grid.ny - 1);

  // flag the grid region which contains the ellipse centroid

  ix = (si32) floor((ellipse_x - grid.minx) / grid.dx + 0.5);
  iy = (si32) floor((ellipse_y - grid.miny) / grid.dy + 0.5);

  if (ix >= start_ix && ix <= end_ix && iy >= start_iy && iy <= end_iy) {
    target_grid[iy][ix] = 1;
  }

  // loop through the vertical lines which intersect the ellipse

  for (ix = start_ix; ix < end_ix; ix++) {

    // compute the slope and intercept of this line in the
    // transformed coordinate system with ths origin at the
    // center of the ellipse and the x-axis along the major
    // axis. The prime values refer to the transformed
    // coord system.
    
    line_x = grid.minx + ((double) ix + 0.5) * grid.dx;

    slope_prime = 1.0 / tan_rotation;

    intercept_prime  = - (line_x - ellipse_x) / sin_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2)) {

      // transform the points back into grid coords

      y_1 = ellipse_y + xprime1 * sin_rotation + yprime1 * cos_rotation;
      y_2 = ellipse_y + xprime2 * sin_rotation + yprime2 * cos_rotation;

      if  (y_1 <= y_2) {

	iy1 = (si32) floor((y_1 - grid.miny) / grid.dy + 0.5);
	iy2 = (si32) floor((y_2 - grid.miny) / grid.dy + 0.5);

      } else {

	iy1 = (si32) floor((y_2 - grid.miny) / grid.dy + 0.5);
	iy2 = (si32) floor((y_1 - grid.miny) / grid.dy + 0.5);

      }

      iy1 = MAX(iy1, start_iy);
      iy2 = MIN(iy2, end_iy);

      for (iy = iy1; iy <= iy2; iy++) {

	target_grid[iy][ix] = 1;
	target_grid[iy][ix + 1] = 1;

      } /* iy */

    } /* if (uline_through_ellipse(major_radius ... */

  } /* ix */

  // loop through the horizontal lines which intersect the ellipse

  for (iy = start_iy; iy < end_iy; iy++) {

    // compute the slope and intercept of this line in the
    // transformed coordinate system with ths origin at the
    // center of the ellipse and the x-axis along the major
    // axis. The prime values refer to the transformed
    // coord system.
    
    line_y = grid.miny + ((double) iy + 0.5) * grid.dy;

    slope_prime = - tan_rotation;

    intercept_prime  = (line_y - ellipse_y) / cos_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2)) {

      // transform the points back into grid coords

      x_1 = ellipse_x + xprime1 * cos_rotation - yprime1 * sin_rotation;
      x_2 = ellipse_x + xprime2 * cos_rotation - yprime2 * sin_rotation;

      if  (x_1 <= x_2) {

	ix1 = (si32) floor((x_1 - grid.minx) / grid.dx + 0.5);
	ix2 = (si32) floor((x_2 - grid.minx) / grid.dx + 0.5);

      } else {

	ix1 = (si32) floor((x_2 - grid.minx) / grid.dx + 0.5);
	ix2 = (si32) floor((x_1 - grid.minx) / grid.dx + 0.5);

      }

      ix1 = MAX(ix1, start_ix);
      ix2 = MIN(ix2, end_ix);

      for (ix = ix1; ix <= ix2; ix++) {

	target_grid[iy][ix] = 1;
	target_grid[iy + 1][ix] = 1;

      } /* ix */

    } /* if (uline_through_ellipse(major_radius ... */

  } /* iy */

}

/*********************************************************************
 * setRunsInGrid()
 *
 * Flags regions in the grid with 1's if the given
 * grid point is covered by a storm run.
 *
 *********************************************************************/

void StatsGrid::_setRunsInGrid(si32 &start_ix,
			       si32 &start_iy,
			       si32 &end_ix,
			       si32 &end_iy,
			       ui08 **target_grid)
  
{
  
  // clear grid
  
  const Mdvx::coord_t &grid = _trackData.getGrid();
  memset(*target_grid, 0, grid.nx * grid.ny * sizeof(ui08));

  start_ix = grid.nx - 1;
  start_iy = grid.ny - 1;
  end_ix = 0;
  end_iy = 0;
  
  // set grid
  
  storm_file_run_t *run = _trackData.projRuns;
  for (int irun = 0; irun < _trackData.nProjRuns; irun++, run++) {
    int ix1 = run->ix / _params.smoothing_kernel_size;
    int ix2 = (run->ix + run->n - 1) / _params.smoothing_kernel_size;
    int iy = run->iy / _params.smoothing_kernel_size;
    for (int ix = ix1; ix <= ix2; ix++) {
      target_grid[iy][ix] = 1;
    }
    start_ix = MIN(start_ix, ix1);
    end_ix = MAX(end_ix, ix2);
    start_iy = MIN(start_iy, iy);
    end_iy = MAX(end_iy, iy);
  }

}

///////////////////////////////////////
// writeOutputFile()
//
// returns 0 on success, -1 on failure.
//

int StatsGrid::writeOutputFile()

{
  
  if (!_stats) {
    if (_params.debug) {
      cerr << "No data found" << endl;
    }
    return -1;
  }

  // compute mid time

  time_t data_mid = (_args.startTime + _args.endTime) / 2;

  // round the mid time depending on the time span

  int ndays = (_args.endTime - _args.startTime) / SECS_IN_DAY;

  if (ndays > 10) {
    data_mid = (data_mid / SECS_IN_DAY) * SECS_IN_DAY;
  } else if (ndays > 1) {
    data_mid = (data_mid / 3600) * 3600;
  }

  if (_params.debug) {
    cerr << "start time: " << utimstr(_args.startTime) << endl;
    cerr << "end time: " << utimstr(_args.endTime) << endl;
    cerr << "mid time: " << utimstr(data_mid) << endl;
  }

  // initialize the mdvx output object

  time_t centroid_time;

  if (_args.stampTime != 0) {
    centroid_time = _args.stampTime;
  } else {
    if (_params.output_time_stamp == Params::START_TIME) {
      centroid_time = _args.startTime;
    } else if (_params.output_time_stamp == Params::MID_TIME) {
      centroid_time = data_mid;
    } else {
      centroid_time = _args.endTime;
    }
  }

  DsMdvx mdvx;
  _initMdvx(mdvx, _args.startTime, centroid_time, _args.endTime);
  
  // convert grid as appropriate
  
  MdvxRemapLut lut;
  
  for (size_t i = 0; i < mdvx.getNFields(); i++) {
    
    MdvxField *fld = mdvx.getField(i);
    
    switch (_params.output_projection) {
      
    case Params::PROJ_NATIVE:
      break;
      
    case Params::PROJ_LATLON:
      fld->remap2Latlon(lut,
			_params.output_grid.nx,
			_params.output_grid.ny,
			_params.output_grid.minx,
			_params.output_grid.miny,
			_params.output_grid.dx,
			_params.output_grid.dy);
      break;
      
    case Params::PROJ_LAMBERT_CONF:
      fld->remap2Lc2(lut,
		     _params.output_grid.nx,
		     _params.output_grid.ny,
		     _params.output_grid.minx,
		     _params.output_grid.miny,
		     _params.output_grid.dx,
		     _params.output_grid.dy,
		     _params.output_origin_lat,
		     _params.output_origin_lon,
		     _params.output_lat1,
		     _params.output_lat2);
      
    case Params::PROJ_FLAT:
      fld->remap2Flat(lut,
		      _params.output_grid.nx,
		      _params.output_grid.ny,
		      _params.output_grid.minx,
		      _params.output_grid.miny,
		      _params.output_grid.dx,
		      _params.output_grid.dy,
		      _params.output_origin_lat,
		      _params.output_origin_lon,
		      _params.output_rotation);
      
    } // switch
    
  } // i 
  
  if (_params.debug) {
    cerr << "Writing MDV file, time: " << utimstr(data_mid)
	 << ", to url: " << _params.output_url
	 << endl;
  }
  
  // write to directory
  
  mdvx.setWriteLdataInfo();
  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - OutputFile::writeVol" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
// _initMdvx()
//
// Initialize the MDVX object
//

void StatsGrid::_initMdvx(DsMdvx &mdvx,
			  time_t start_time,
			  time_t centroid_time,
			  time_t end_time)
  
{

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.time_begin = start_time;
  mhdr.time_end = end_time;
  mhdr.time_centroid = centroid_time;
  mhdr.time_expire = end_time;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_SYNTHESIS;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;

  const Mdvx::coord_t &grid = _trackData.getGrid();
  mhdr.max_nx = grid.nx;
  mhdr.max_ny = grid.ny;
  mhdr.max_nz = 1;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;

  // info
  
  char info[2048];
  sprintf(info, "%s\n%s : %d\n%s : %g\n%s : %g\n",
	  _params.data_set_info,
	  "n_seasons", _params.n_seasons,
	  "scan_interval", _trackData.scanIntervalSecs,
	  "min_duration", _params.min_duration); 

  STRncopy(mhdr.data_set_info, info, MDV_INFO_LEN);
  STRncopy(mhdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
  STRncopy(mhdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);

  mdvx.setMasterHeader(mhdr);
  
  // fill in field headers and vlevel headers
  
  mdvx.clearFields();
  
  for (int out_field = 0; out_field < N_STATS_FIELDS; out_field++) {
    
    Mdvx::field_header_t fhdr;
    MEM_zero(fhdr);
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(vhdr);

    fhdr.nx = grid.nx;
    fhdr.ny = grid.ny;
    fhdr.nz = 1;

    fhdr.proj_type = (Mdvx::projection_type_t) grid.proj_type;
    
    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
    fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    fhdr.scaling_type = Mdvx::SCALING_NONE;

    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.dz_constant = true;

    fhdr.proj_origin_lat = grid.proj_origin_lat;
    fhdr.proj_origin_lon = grid.proj_origin_lon;
    fhdr.grid_dx = grid.dx;
    fhdr.grid_dy = grid.dy;
    fhdr.grid_minx = grid.minx;
    fhdr.grid_miny = grid.miny;

    fhdr.grid_dz = 1;
    fhdr.grid_minz = 0;

    fhdr.proj_rotation = 0.0;
    
    fhdr.bad_data_value = -999.0;
    fhdr.missing_data_value = -999.0;

    grid_stats_t *stats = *_stats;
    double *val = &stats->n_events;

    switch (out_field) {
      
    case N_EVENTS_POS:
      _setFieldName(fhdr, "N_events", "Number of events", "count");
      val = &stats->n_events;
      break;
      
    case N_WEIGHTED_POS:
      _setFieldName(fhdr, "N_weighted", "Weighted number of events", "count");
      val = &stats->n_weighted;
      break;
      
    case N_COMPLEX_POS:
      _setFieldName(fhdr, "N_complex", "Number of complex events", "count");
      val = &stats->n_complex;
      break;
      
    case PERCENT_ACTIVITY_POS:
      _setFieldName(fhdr, "Activity", "Activity in % of total time", "%");
      val = &stats->percent_activity;
      break;
      
    case N_START_POS:
      _setFieldName(fhdr, "N_start", "Number of starts", "count/km2");
      val = &stats->n_start;
      break;
      
    case N_MID_POS:
      _setFieldName(fhdr, "N_mid", "Number of mid track counts", "count/km2");
      val = &stats->n_mid;
      break;
      
    case PRECIP_POS:
      _setFieldName(fhdr, "Precip", "Seasonal mean precip", "mm");
      val = &stats->precip;
      break;
      
    case VOLUME_POS:
      _setFieldName(fhdr, "Mean_volume", "Mean storm volume", "km3");
      val = &stats->volume;
      break;
      
    case DBZ_MAX_POS:
      _setFieldName(fhdr, "Mean_max_dBZ", "Mean of storm max dBZ", "dBZ");
      val = &stats->dbz_max;
      break;
      
    case TOPS_POS:
      _setFieldName(fhdr, "Mean_tops", "Mean storm tops", "km");
      val = &stats->tops;
      break;
      
    case SPEED_POS:
      _setFieldName(fhdr, "Mean_speed", "Mean storm speed", "km/hr");
      val = &stats->speed;
      break;
      
    case U_POS:
      _setFieldName(fhdr, "Mean_U", "Mean U motion component", "km/hr");
      val = &stats->u;
      break;
      
    case V_POS:
      _setFieldName(fhdr, "Mean_V", "Mean V motion component", "km/hr");
      val = &stats->v;
      break;
      
    case DISTANCE_POS:
      _setFieldName(fhdr, "Dist_moved", "Mean distance moved", "km");
      val = &stats->distance;
      break;
      
    case DX_POS:
      _setFieldName(fhdr, "Dx_moved", "Mean x distance moved", "km");
      val = &stats->dx;
      break;
      
    case DY_POS:
      _setFieldName(fhdr, "Dy_moved", "Mean y distance moved", "km");
      val = &stats->dy;
      break;
      
    case AREA_POS:
      _setFieldName(fhdr, "Mean_area", "Mean storm area", "km2");
      val = &stats->area;
      break;
      
    case DURATION_POS:
      _setFieldName(fhdr, "Mean_duration", "Mean storm duration", "hr");
      val = &stats->duration;
      break;
      
    case LN_AREA_POS:
      _setFieldName(fhdr, "ln_area", "ln of area", "ln(km2)");
      val = &stats->ln_area;
      break;

    case ELLIPSE_U:
      _setFieldName(fhdr, "Ellipse_U",
		    "U representation of ellipse shape", "aspect");
      val = &stats->ellipse_u;
      break;

    case ELLIPSE_V:
      _setFieldName(fhdr, "Ellipse_V",
		    "V representation of ellipse shape", "aspect");
      val = &stats->ellipse_v;
      break;

    default: {}
      
    } // switch
    
    // vlevel header
    
    vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[0] = 0;

    // load up fl32 array with field values

    TaArray<fl32> valArray_;
    fl32 *valArray = valArray_.alloc(fhdr.nx * fhdr.ny);
    int nstats = sizeof(grid_stats_t) / sizeof(double);
    for (int i = 0; i < fhdr.nx * fhdr.ny; i++, val += nstats) {
      valArray[i] = (fl32) *val;
    }
    
    // create field
    
    MdvxField *field = new MdvxField(fhdr, vhdr, valArray);
    if (out_field == PRECIP_POS || out_field == PERCENT_ACTIVITY_POS) {
      field->transform2Log();
    }
    
    // add field to mdvx object

    mdvx.addField(field);
    
  } // out_field

}

////////////////////////////////////////////////////
// _setFieldName()
//
// Sets the field name, units etc
//

void StatsGrid::_setFieldName(Mdvx::field_header_t &fhdr,
			      const char *name,
			      const char *name_long,
			      const char *units)
  
{

  STRncopy(fhdr.field_name, name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.field_name_long, name_long, MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.units, units, MDV_UNITS_LEN);
  
}

///////////////////////////////////
// compute densities
//

void StatsGrid::_computeDensities()

{

  const Mdvx::coord_t &grid = _trackData.getGrid();

  // compute grid geometry

  double dxKm = grid.dx;
  double dyKm = grid.dy;
  if (grid.proj_type == Mdvx::PROJ_LATLON) {
    dxKm =
      grid.dx * KM_PER_DEG_AT_EQ * cos(_trackData.centroidY * DEG_TO_RAD);
    dyKm = grid.dy * KM_PER_DEG_AT_EQ;
  }
  double cellAreaKm2 = dxKm * dyKm;
  int radiusNx = (int) (_params.radius_for_density_stats / dxKm + 0.5);
  int radiusNy = (int) (_params.radius_for_density_stats / dyKm + 0.5);
  
  // allocate temporary grid, initializing to 0.0
  
  grid_stats_t **tmpStats = (grid_stats_t **) ucalloc2
    (grid.ny, grid.nx, sizeof(grid_stats_t));
  
  // accumulate counts on the number-type stats

  for (int iy = 0; iy < grid.ny; iy++) {
    for (int ix = 0; ix < grid.nx; ix++) {

      // compute limits of radius

      int startJy = iy - radiusNy;
      if (startJy < 0) {
	startJy = 0;
      }
	
      int startJx = ix - radiusNx;
      if (startJx < 0) {
	startJx = 0;
      }
	
      int endJy = iy + radiusNy;
      if (endJy > grid.ny - 1) {
	endJy = grid.ny - 1;
      }
	
      int endJx = ix + radiusNx;
      if (endJx > grid.nx - 1) {
	endJx = grid.nx - 1;
      }

      // loop through limits
      
      for (int jy = startJy; jy <= endJy; jy++) {
	for (int jx = startJx; jx <= endJx; jx++) {

	  // check if we are within the radius

	  double dy = (iy - jy) * dyKm;
	  double dx = (ix - jx) * dxKm;
	  double dist = sqrt(dy * dy + dx * dx);
	  if (dist > _params.radius_for_density_stats) {
	    continue;
	  }

	  // accumulate tmp stats
	  
	  tmpStats[iy][ix].n_events += _stats[jy][jx].n_events;
	  tmpStats[iy][ix].n_start += _stats[jy][jx].n_start;
	  tmpStats[iy][ix].n_mid += _stats[jy][jx].n_mid;
	  tmpStats[iy][ix].area += cellAreaKm2;

	} // jx
      } // jy
      
    } // ix
  } // iy

  // compute the density of the number-type stats

  for (int iy = 0; iy < grid.ny; iy++) {
    for (int ix = 0; ix < grid.nx; ix++) {

      _stats[iy][ix].n_events =
	tmpStats[iy][ix].n_events / tmpStats[iy][ix].area;

      _stats[iy][ix].n_start =
	tmpStats[iy][ix].n_start / tmpStats[iy][ix].area;

      _stats[iy][ix].n_mid =
	tmpStats[iy][ix].n_mid / tmpStats[iy][ix].area;

    } // ix
  } // iy

  // free up

  ufree2((void **) tmpStats);

}

