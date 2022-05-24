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
// CompleteTrack.cc
//
// CompleteTrack object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
///////////////////////////////////////////////////////////////

#include "CompleteTrack.hh"
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>
#include <toolsa/DateTime.hh>
#include <toolsa/pjg.h>
#include <rapmath/math_macros.h>
#include <rapmath/umath.h>
using namespace std;

#define SMALL_ANGLE 0.000001
#define PI_BY_TWO 1.570796327
#define ALMOST_PI_BY_TWO (PI_BY_TWO - SMALL_ANGLE)
#ifndef PI
#define PI 3.141592654
#endif
#define ALMOST_PI (PI - SMALL_ANGLE)

//////////////
// Constructor

CompleteTrack::CompleteTrack(const string &prog_name, const Params &params) :
        Entity(prog_name, params),
        _cases(NULL)

{

  isOK = true;

  // print labels

  fprintf(stdout, "#labels: %s\n",
	  "Track_num,"
	  "Year,"
	  "Month,"
	  "Day,"
	  "Hour,"
	  "Min,"
	  "Sec,"
	  "Nscans,"
	  "Duration(hours),"
	  "RemDurAtMaxVol(hours),"
	  "MeanVolume(km3),"
	  "MaxVolume(km3),"
	  "MeanMass(ktons),"
	  "MaxMass(ktons),"
	  "MaxPrecipDepthEllipse(mm),"
	  "MeanPrecipDepthEllipse(mm),"
	  "MeanPrecipDepthRuns(mm),"
	  "MeanPrecipFlux(m3/s),"
	  "MaxPrecipFlux(m3/s),"
	  "MeanEnvelopeArea(km2),"
	  "MaxEnvelopeArea(km2),"
	  "MeanPrecipArea(km2),"
	  "MaxPrecipArea(km2),"
	  "MeanTop(kmMsl),"
	  "MaxTop(kmMsl),"
	  "MeanBase(kmMsl),"
	  "MaxBase(kmMsl),"
	  "MeanDBZ,"
	  "MaxDBZ,"
	  "RadarEstRainVol(m3),"
	  "ATI(km2hr),"
	  "SwathAreaEllipse(km2),"
	  "SwathAreaRuns(km2),"
          "MeanSpeed(km/hr),"
	  "MeanDirection(degT),"
	  "POD,"
	  "FAR,"
	  "CSI,"
          "minRange(km),"
          "maxRange(km),"
          "meanRange(km)");

  // read in case file

  if (_params.use_case_tracks_file) {
    if (_params.debug) {
      cerr << "Using case tracks file: "
           << _params.case_tracks_file_path << endl;
    }
    _caseTracks.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
    if (_caseTracks.readCaseFile(_params.case_tracks_file_path)) {
      cerr << "ERROR - Tracks2Ascii::CompleteTrack" << endl;
      cerr << "  Cannot read in case track file" << endl;
      isOK = false;
      return;
    }
    _cases = &_caseTracks.getCases();
  }

}

/////////////
// destructor

CompleteTrack::~CompleteTrack()

{


}

////////////////
// comps_init()
//

int CompleteTrack::comps_init(storm_file_handle_t *s_handle,
			      track_file_handle_t *t_handle)
  
{

  _totEntries = 0.0;
  _nstormsInBox = 0.0;

  _rerv = 0.0;
  _sumVolume = 0.0;
  _sumMass = 0.0;
  _sumPrecipFlux = 0.0;
  _sumPrecipArea = 0.0;
  _sumProjArea = 0.0;
  _sumDbz = 0.0;
  _sumBase = 0.0;
  _sumTop = 0.0;
  _sumU = 0.0;
  _sumV = 0.0;

  _sumRange = 0.0;
  _minRange = 1.0e99;
  _maxRange = 0.0;
  
  _maxVolume = -LARGE_DOUBLE;
  _maxMass = -LARGE_DOUBLE;
  _maxPrecipFlux = -LARGE_DOUBLE;
  _maxPrecipArea = -LARGE_DOUBLE;
  _maxProj_area = -LARGE_DOUBLE;
  _maxDbz = -LARGE_DOUBLE;
  _maxBase = -LARGE_DOUBLE;
  _maxTop = -LARGE_DOUBLE;

  // read in first storm scan
  
  if (RfReadStormScan(s_handle, (int) 0, "track_comps")) {
    return (-1);
  }

  _grid = s_handle->scan->grid;

  // allocate grids
  
  _runsSwathGrid = (ui08 **)
    ucalloc2 (_grid.ny, _grid.nx, sizeof(ui08));
  
  _ellipseSwathGrid = (ui08 **)
    ucalloc2 (_grid.ny, _grid.nx, sizeof(ui08));
  
  _ellipseGrid = (ui08 **)
    ucalloc2 (_grid.ny, _grid.nx, sizeof(ui08));
  
  _precipGrid = (double **)
    ucalloc2 (_grid.ny, _grid.nx, sizeof(double));
  
  // suppress compiler warnings
  s_handle = NULL;
  t_handle = NULL;

  return (0);

}

////////////
// compute()
//

int CompleteTrack::compute(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle)
  
{
  
  int irun;  
  int this_scan, last_scan;
  int precip_layer;

  double volume;
  double mass, precip_flux;
  double precip_area, proj_area;
  double dbz_max, dbz_mean;
  double top, base;
  double u, v;
  double precip_z;

  date_time_t last_time, entry_time;
  storm_file_run_t *run;
  
  storm_file_params_t *sparams = &s_handle->header->params;
  storm_file_global_props_t *gprops = s_handle->gprops + t_handle->entry->storm_num;
  track_file_forecast_props_t *fprops = &t_handle->entry->dval_dt;

  if (_params.use_box_limits) {
    if (gprops->refl_centroid_x >= _params.box.min_x &&
	gprops->refl_centroid_x <= _params.box.max_x &&
	gprops->refl_centroid_y >= _params.box.min_y &&
	gprops->refl_centroid_y <= _params.box.max_y) {
      _nstormsInBox++;
    }
  } // if (_params.use_box_limits)

  // compute range and azimuth of refl centroid

  storm_file_global_props_t gpropsConvert = *gprops;
  RfStormGpropsXY2LatLon(s_handle->scan, &gpropsConvert);
  double refl_centroid_lon = gpropsConvert.refl_centroid_x;
  double refl_centroid_lat = gpropsConvert.refl_centroid_y;
  double reflCentroidRangeKm, reflCentroidAzimuthDeg;
  PJGLatLon2RTheta(_grid.proj_origin_lat, _grid.proj_origin_lon,
                   refl_centroid_lat, refl_centroid_lon,
                   &reflCentroidRangeKm, &reflCentroidAzimuthDeg);

  // set props

  volume = gprops->volume;
  mass = gprops->mass;
  precip_flux = gprops->precip_flux;
  precip_area = gprops->precip_area;
  proj_area = gprops->proj_area;
  top = gprops->top;
  base = gprops->base;
  dbz_max = gprops->dbz_max;
  dbz_mean = gprops->dbz_mean;
  
  u = fprops->proj_area_centroid_x;
  v = fprops->proj_area_centroid_y;
  
  _rerv += precip_flux * _params.scan_interval;
  _sumVolume += volume;
  _sumMass += mass;
  _sumPrecipFlux += precip_flux;
  _sumPrecipArea += precip_area;
  _sumProjArea += proj_area;
  _sumTop += top;
  _sumBase += base;
  _sumDbz += dbz_mean;
  _sumU += u;
  _sumV += v;

  _sumRange += reflCentroidRangeKm;
  if (reflCentroidRangeKm < _minRange) {
    _minRange = reflCentroidRangeKm;
  }
  if (reflCentroidRangeKm > _maxRange) {
    _maxRange = reflCentroidRangeKm;
  }

  _maxMass = MAX(_maxMass, mass);
  _maxPrecipFlux = MAX(_maxPrecipFlux, precip_flux);
  _maxPrecipArea = MAX(_maxPrecipArea, precip_area);
  _maxProj_area = MAX(_maxProj_area, proj_area);
  _maxTop = MAX(_maxTop, top);
  _maxBase = MAX(_maxBase, base);
  _maxDbz = MAX(_maxDbz, dbz_max);

  if (_maxVolume < volume) {

    _maxVolume = volume;

    // compute life expectancy
    
    last_time.unix_time =
      t_handle->simple_params->last_descendant_end_time;
    uconvert_from_utime(&last_time);
    
    entry_time.unix_time = t_handle->entry->time;
    uconvert_from_utime(&entry_time);

    this_scan = t_handle->entry->scan_num;
    last_scan = t_handle->simple_params->last_descendant_end_scan;
    
    if (this_scan == last_scan) {
      
      _remDuration = 0.0;

    } else {

      // adjust for the fact that the times are taken at mid-scan,
      // by adding the duration of one-half of a scan
      
      _remDuration =
	((((double) last_time.unix_time -
	   (double) entry_time.unix_time) +
	  _params.scan_interval / 2) / 3600.0);
      
    } // if (this_scan == last_scan)
      
  } // if (_maxVolume < volume) 

  _totEntries++;

  // precip comps

  if (gprops->precip_area > 0) {

    // check that the grid params have not changed

    if (_grid.nx != s_handle->scan->grid.nx ||
        _grid.ny != s_handle->scan->grid.ny ||
        _grid.nz != s_handle->scan->grid.nz ||
        fabs(_grid.dx - s_handle->scan->grid.dx) > 0.0001 ||
        fabs(_grid.dy - s_handle->scan->grid.dy) > 0.0001 ||
        fabs(_grid.dz - s_handle->scan->grid.dz) > 0.0001 ||
        fabs(_grid.minx - s_handle->scan->grid.minx) > 0.0001 ||
        fabs(_grid.miny - s_handle->scan->grid.miny) > 0.0001 ||
        fabs(_grid.minz - s_handle->scan->grid.minz) > 0.0001) {

      fprintf(stderr, "ERROR - %s:CompleteTrack::compute\n",
	      _progName.c_str());
      fprintf(stderr, "Grid params have changed.\n");
      
      fprintf(stderr, "====>> Previous grid <<=====\n");
      TITAN_print_grid(stderr, "", &s_handle->scan->grid);
      fprintf(stderr, "====>> This grid <<=====\n");
      TITAN_print_grid(stderr, "", &_grid);
      fprintf(stderr, "========================\n");

      return (-1);
    }

    precip_z = s_handle->header->params.precip_plane_ht;
    precip_layer = (int) ((precip_z - _grid.minz) / _grid.dz + 0.5);

    // update the _runsSwathGrid

    if (RfReadStormProps(s_handle, t_handle->entry->storm_num,
			 "track_comps:update_track_comps")) {
      return (-1);
    }

    run = s_handle->runs;
    
    for (irun = 0; irun < gprops->n_runs; irun++) {
      
      if (run->iz == precip_layer)
	memset((void *) (_runsSwathGrid[run->iy] + run->ix),
	       (int) 1, (int) run->n);
      
      run++;
      
    } // irun
    
    // update the _ellipseSwathGrid and the precip grid, using
    // the precip area ellipse and the precip area reflectivity
    // distribution
    
    update_ellipse_precip(s_handle, sparams, gprops);

  } // if (gprops->precip_area > 0)

  return (0);

}

////////////////
// comps_done()
//

int CompleteTrack::comps_done(storm_file_handle_t *s_handle,
			      track_file_handle_t *t_handle)
  
{

  ui08 *runs_p;
  ui08 *ellipse_p;

  int use_track;

  int ipt;

  double *precip_p;
  double duration;
  double ati;
  double mean_volume;
  double mean_mass;
  double ellipse_sum_precip_depth;
  double ellipse_n_precip;
  double ellipse_max_precip_depth;
  double ellipse_mean_precip_depth, runs_mean_precip_depth;
  double mean_precip_flux;
  double mean_precip_area;
  double mean_proj_area;
  double mean_top, mean_base;
  double mean_dbz;
  double mean_u, mean_v;
  double mean_speed, mean_dirn;
  double mean_range;
  double pod, far, csi;
  double percent_in_box;
  double element_area, runs_swath_area;
  double ellipse_swath_area;

  complex_track_params_t *cparams;

  cparams = t_handle->complex_params;

  if (t_handle->complex_params->duration_in_secs < 2) {
    duration = _params.scan_interval / 3600.0;
  } else {
    duration = (double) t_handle->complex_params->duration_in_secs / 3600.0;
  }

  // try polygon verification first

  if (cparams->polygon_verify.n_success != 0 ||
      cparams->polygon_verify.n_failure != 0 ||
      cparams->polygon_verify.n_false_alarm != 0) {

    compute_verification(cparams->polygon_verify, pod, far, csi);

  } else {

    // use ellipse verification

    compute_verification(cparams->ellipse_verify, pod, far, csi);

  }

  mean_volume = _sumVolume / _totEntries;
  mean_mass = _sumMass / _totEntries;
  mean_precip_flux = _sumPrecipFlux / _totEntries;
  mean_precip_area = _sumPrecipArea / _totEntries;
  mean_proj_area = _sumProjArea / _totEntries;
  mean_top = _sumTop / _totEntries;
  mean_base = _sumBase / _totEntries;
  mean_dbz = _sumDbz / _totEntries;
  mean_u = _sumU / _totEntries;
  mean_v = _sumV / _totEntries;
  mean_range = _sumRange / _totEntries;

  mean_speed = sqrt(mean_u * mean_u + mean_v * mean_v);

  if (mean_u == 0.0 && mean_v == 0.0)
    mean_dirn = 0.0;
  else
    mean_dirn = atan2(mean_u, mean_v) * RAD_TO_DEG;

  if (mean_dirn < 0.0)
    mean_dirn += 360.0;

  percent_in_box = (_nstormsInBox / _totEntries) * 100.0;

  // area time integral (km2.hr)

  ati = mean_precip_area * duration;
  
  // compute swath areas (km2)

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    print_grid("Runs_swath", "Ellipse_swath",
	       _runsSwathGrid, _ellipseSwathGrid);
  }

  element_area = _grid.dx * _grid.dy;
  runs_swath_area = 0.0;
  ellipse_swath_area = 0.0;
  ellipse_max_precip_depth = 0.0;
  ellipse_sum_precip_depth = 0.0;
  ellipse_n_precip = 0.0;
  
  runs_p = *_runsSwathGrid;
  ellipse_p = *_ellipseSwathGrid;
  precip_p = *_precipGrid;
  
  for (ipt = 0; ipt < _grid.ny * _grid.nx; ipt++) {

    if (*runs_p)
      runs_swath_area += element_area;

    if (*ellipse_p) {
      ellipse_swath_area += element_area;
      ellipse_sum_precip_depth += *precip_p;
      ellipse_n_precip++;
      ellipse_max_precip_depth =
	MAX(ellipse_max_precip_depth, *precip_p);
    }

    runs_p++;
    ellipse_p++;
    precip_p++;

  } /* ipt */
  
  /*
   * compute mean precip (mm)
   */

  if (runs_swath_area == 0.0) {
    runs_mean_precip_depth = 0.0;
  } else {
    runs_mean_precip_depth = _rerv / (runs_swath_area * 1000.0);
  }

  if (ellipse_n_precip == 0.0) {
    ellipse_mean_precip_depth = 0.0;
  } else {
    ellipse_mean_precip_depth =
      ellipse_sum_precip_depth / ellipse_n_precip;
  }

  use_track = TRUE;

  if (_params.use_box_limits) {
    
    if (percent_in_box < _params.box.min_percent &&
	_nstormsInBox < _params.box.min_nscans) {
      use_track = FALSE;
    }

  } // if (_params.use_box_limits)

  // do we need to check cases?

  if (_cases) {
    bool trackIsCase = false;
    for (int ii = 0; ii < (int) _cases->size(); ii++) {
      const SeedCaseTracks::CaseTrack &thisCase = (*_cases)[ii];
      if (thisCase.ref_time >= cparams->start_time &&
          thisCase.ref_time <= cparams->end_time &&
          thisCase.complex_track_num == cparams->complex_track_num) {
        if (_params.debug) {
          cerr << "Found case, num, ref_time, complex_track_num: "
               << thisCase.num << ", "
               << DateTime::strm(thisCase.ref_time) << ", "
               << thisCase.complex_track_num << endl;
            }
        trackIsCase = true;
        break;
      }
    }
    if (!trackIsCase) {
      use_track = false;
    }
  }

  if (use_track) {

    fprintf(stdout, "%ld ",
	   (long) cparams->complex_track_num);
    
    date_time_t st;
    st.unix_time = cparams->start_time;
    uconvert_from_utime(&st);
    
    fprintf(stdout, "%d %d %d %d %d %d ",
	   st.year, st.month, st.day,
	   st.hour, st.min, st.sec);

    fprintf(stdout, "%ld %g %g ",
	   (long) cparams->duration_in_scans,
	   duration,
	   _remDuration);
    
    fprintf(stdout, "%g %g %g %g %g %g %g %g %g ",
	   mean_volume,
	   _maxVolume,
	   mean_mass,
	   _maxMass,
	   ellipse_max_precip_depth,
	   ellipse_mean_precip_depth,
	   runs_mean_precip_depth,
	   mean_precip_flux,
	   _maxPrecipFlux);

    fprintf(stdout, "%g %g %g %g %g %g %g %g %g %g ",
	   mean_proj_area,
	   _maxProj_area,
	   mean_precip_area,
	   _maxPrecipArea,
	   mean_top,
	   _maxTop,
	   mean_base,
	   _maxBase,
	   mean_dbz,
	   _maxDbz);

    fprintf(stdout, "%g %g %g %g %g %g %g %g %g ",
	   _rerv,
	   ati,
	   ellipse_swath_area,
	   runs_swath_area,
	   mean_speed,
	   mean_dirn,
	   pod,
	   far,
	   csi);
    
    fprintf(stdout, "%g %g %g\n",
            _minRange,
            _maxRange,
            mean_range);
    
  }

  // free up mem

  ufree2((void **) _runsSwathGrid);
  ufree2((void **) _ellipseSwathGrid);
  ufree2((void **) _ellipseGrid);
  ufree2((void **) _precipGrid);
  
  // suppress compiler warnings
  s_handle = NULL;

  return (-1);

}

////////////////////////////////////////////////////////////////////////
// compute verification

void CompleteTrack::compute_verification
  (const track_file_contingency_data_t &cont,
   double &pod, double &far, double &csi) 

{

  if (cont.n_success == 0) {
    pod = 0.0;
  } else if (cont.n_failure == 0) {
    pod = 1.0;
  } else {
    pod = (double) cont.n_success /
      ((double) cont.n_success +
       (double) cont.n_failure);
  }
  
  if (cont.n_success == 0) {
    far = 1.0;
  } else if (cont.n_false_alarm == 0) {
    far = 0.0;
  } else {
    far = (double) cont.n_false_alarm /
      ((double) cont.n_success +
       (double) cont.n_false_alarm);
  }
  
  if (cont.n_success == 0) {
    csi = 0.0;
  } else if (cont.n_failure == 0 &&
             cont.n_false_alarm == 0 ) {
    csi = 1.0;
  } else {
    csi = (double) cont.n_success /
      ((double) cont.n_success +
       (double) cont.n_failure +
       (double) cont.n_false_alarm);
  }

}


////////////////////////////////////////////////////////////////////////
// set_ellipse_grid()
//
// Flags regions in the grid with 1's if the given
// ellipse crosses into or contains the region.
//
// The method uses 3 steps.
//
// 1) Flag the region containing the ellipse centroid.
//
// 2) Consider all vertical grid lines which intersect the ellipse.
//    Flag all regions on either side of such a line for that
//    line segment which crosses the ellipse.
//
// 3) Consider all horizontal grid lines which intersect the ellipse.
//    Flag all regions on either side of such a line for that
//    line segment which crosses the ellipse.
//
////////////////////////////////////////////////////////////////////////

void CompleteTrack::set_ellipse_grid(double ellipse_x,
				     double ellipse_y,
				     double major_radius,
				     double minor_radius,
				     double axis_rotation,
				     int *start_ix_p,
				     int *start_iy_p,
				     int *end_ix_p,
				     int *end_iy_p,
				     ui08 **grid)
  
{

  int ix, iy, ix1, iy1, ix2, iy2;
  int start_ix, start_iy;
  int end_ix, end_iy;

  double grid_rotation, theta;
  double slope_prime, intercept_prime;
  double sin_rotation, cos_rotation, tan_rotation;

  double xprime1, yprime1, xprime2, yprime2;
  double x_1, y_1, x_2, y_2;
  double start_x, start_y;
  double end_x, end_y;
  double line_x, line_y;

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

  start_ix = (int) ((start_x - _grid.minx) / _grid.dx + 0.49);
  start_ix = MAX(start_ix, 0);

  start_iy = (int) ((start_y - _grid.miny) / _grid.dy + 0.49);
  start_iy = MAX(start_iy, 0);

  end_ix = (int) ((end_x - _grid.minx) / _grid.dx + 0.51);
  end_ix = MIN(end_ix, _grid.nx - 1);

  end_iy = (int) ((end_y - _grid.miny) / _grid.dy + 0.51);
  end_iy = MIN(end_iy, _grid.ny - 1);

  *start_ix_p = start_ix;
  *start_iy_p = start_iy;
  *end_ix_p = end_ix;
  *end_iy_p = end_iy;

  // flag the grid region which contains the ellipse centroid

  ix = (int) ((ellipse_x - _grid.minx) / _grid.dx + 0.5);
  iy = (int) ((ellipse_y - _grid.miny) / _grid.dy + 0.5);

  if (ix >= start_ix && ix <= end_ix &&
      iy >= start_iy && iy <= end_iy)
    grid[iy][ix] = 1;

  // loop through the vertical lines which intersect the ellipse

  for (ix = start_ix; ix < end_ix; ix++) {

    // compute the slope and intercept of this line in the
    // transformed coordinate system with ths origin at the
    // center of the ellipse and the x-axis along the major
    // axis. The prime values refer to the transformed
    // coord system.

    
    line_x = _grid.minx + ((double) ix + 0.5) * _grid.dx;

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

	iy1 = (int) ((y_1 - _grid.miny) / _grid.dy + 0.5);
	iy2 = (int) ((y_2 - _grid.miny) / _grid.dy + 0.5);

      } else {

	iy1 = (int) ((y_2 - _grid.miny) / _grid.dy + 0.5);
	iy2 = (int) ((y_1 - _grid.miny) / _grid.dy + 0.5);

      }

      iy1 = MAX(iy1, 0);
      iy2 = MIN(iy2, _grid.ny - 1);

      for (iy = iy1; iy <= iy2; iy++) {

	grid[iy][ix] = 1;
	grid[iy][ix + 1] = 1;

      } /// iy ///

    } /// if (uline_through_ellipse(major_radius ... ///

  } /// ix ///

  // loop through the horizontal lines which intersect the ellipse

  for (iy = start_iy; iy < end_iy; iy++) {

    // compute the slope and intercept of this line in the
    // transformed coordinate system with ths origin at the
    // center of the ellipse and the x-axis along the major
    // axis. The prime values refer to the transformed
    // coord system.

    
    line_y = _grid.miny + ((double) iy + 0.5) * _grid.dy;

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

	ix1 = (int) ((x_1 - _grid.minx) / _grid.dx + 0.5);
	ix2 = (int) ((x_2 - _grid.minx) / _grid.dx + 0.5);

      } else {

	ix1 = (int) ((x_2 - _grid.minx) / _grid.dx + 0.5);
	ix2 = (int) ((x_1 - _grid.minx) / _grid.dx + 0.5);

      }

      ix1 = MAX(ix1, 0);
      ix2 = MIN(ix2, _grid.nx - 1);

      for (ix = ix1; ix <= ix2; ix++) {

	grid[iy][ix] = 1;
	grid[iy + 1][ix] = 1;

      } /// ix ///

    } /// if (uline_through_ellipse(major_radius ... ///

  } /// iy ///

}

///////////////////////////////////////////////////////////////
// update_ellipse_precip()
//
// Updates the grids based on the storm precip area ellipses,
// and uses the reflectivity distribution to estimate the
// precip swath
///////////////////////////////////////////////////////////////

void CompleteTrack::update_ellipse_precip(storm_file_handle_t *s_handle,
					  storm_file_params_t *sparams,
					  storm_file_global_props_t *gprops)

{

  ui08 *ellipse_p;
  ui08 *ellipse_swath_p;

  int interval;
  int n_dbz_intervals;
  int start_ix, start_iy;
  int end_ix, end_iy;
  int ix, iy;

  double low_dbz_threshold;
  double dbz_hist_interval;
  double hail_dbz_threshold;
  double ellipse_x;
  double ellipse_y;
  double major_radius;
  double minor_radius;
  double frac_major_radius;
  double frac_minor_radius;
  double axis_rotation;
  double z_p_coeff, z_p_exponent;
  double low_dbz;
  double mean_dbz;
  double accum_depth, incr_depth;
  double precip_rate, precip_depth;
  double area_fraction;
  double percent_area;
  double z;
  double *precip_p;

  storm_file_dbz_hist_t *hist;

  // set parameters

  low_dbz_threshold = sparams->low_dbz_threshold;
  dbz_hist_interval = sparams->dbz_hist_interval;
  hail_dbz_threshold = sparams->hail_dbz_threshold;
  n_dbz_intervals = gprops->n_dbz_intervals;

  z_p_coeff = sparams->z_p_coeff;
  z_p_exponent = sparams->z_p_exponent;
  ellipse_x = gprops->precip_area_centroid_x;
  ellipse_y = gprops->precip_area_centroid_y;
  major_radius = gprops->precip_area_major_radius;
  minor_radius = gprops->precip_area_minor_radius;
  axis_rotation = gprops->precip_area_orientation;
  
  // compute the area and incremental precip depth for
  // each reflectivity interval

  low_dbz = low_dbz_threshold;
  mean_dbz = low_dbz + dbz_hist_interval / 2.0;
  
  accum_depth = 0.0;
  hist = s_handle->hist;
  area_fraction = 1.0;

  for (interval = 0; interval < n_dbz_intervals; interval++, hist++) {

    z = pow(10.0, low_dbz / 10.0);
    precip_rate = pow((z / z_p_coeff), (1.0 / z_p_exponent));
    precip_depth = (precip_rate * (double) _params.scan_interval) / 3600.0;
    incr_depth = precip_depth - accum_depth;
    percent_area = hist->percent_area;

    // zero out ellipse grid
    
    memset((void *) *_ellipseGrid,
	   (int) 0,
	   (int) (_grid.nx * _grid.ny * sizeof(ui08)));

    // set points in ellipse grid

    frac_major_radius = major_radius * sqrt(area_fraction);
    frac_minor_radius = minor_radius * sqrt(area_fraction);

    set_ellipse_grid(ellipse_x,
		     ellipse_y,
		     frac_major_radius,
		     frac_minor_radius,
		    axis_rotation,
		     &start_ix, &start_iy,
		     &end_ix, &end_iy,
		     _ellipseGrid);

    // set the grids

    for (iy = start_iy; iy < end_iy; iy++) {

      ellipse_p = _ellipseGrid[iy] + start_ix;
      precip_p = _precipGrid[iy] + start_ix;
      ellipse_swath_p = _ellipseSwathGrid[iy] + start_ix;

      for (ix = start_ix; ix < end_ix; ix++) {

	if (*ellipse_p) {
	  *precip_p += incr_depth;
	  *ellipse_swath_p = 1;
	}

	ellipse_p++;
	precip_p++;
	ellipse_swath_p++;

      } /// ix ///

    } /// iy ///

    // increment and test for end of loop conditions
  
    accum_depth = precip_depth;
    area_fraction -= percent_area / 100.0;
    low_dbz += dbz_hist_interval;
    mean_dbz += dbz_hist_interval;

    if (low_dbz > hail_dbz_threshold)
      break;

    if (area_fraction <= 0.001)
      break;

  } /// interval ///

}

//////////////////////////////////////////////
// print_grid()
//

void CompleteTrack::print_grid(const char *label1,
			       const char *label2,
			       ui08 **grid1,
			       ui08 **grid2)

{

  int info_found;
  int count;
  int ix, iy;
  int min_ix, max_ix, nx_active;

  // search for max and min x activity

  min_ix = 1000000;
  max_ix = 0;
  
  for (iy = _grid.ny - 1; iy >= 0; iy--) {

    for (ix = 0; ix < _grid.nx; ix++) {

      if (grid2[iy][ix] ||
	  grid1[iy][ix]) {
	
	min_ix = MIN(min_ix, ix);
	max_ix = MAX(max_ix, ix);
	
      } /// if ///
      
    } /// ix ///

  } /// iy ///

  nx_active = max_ix - min_ix + 1;

  // print header

  fprintf(stderr, "%s (1), %s (2), both (B)\n", label1, label2);
  fprintf(stderr, "start_ix = %ld\n", (long) min_ix);

  fprintf(stderr, "     ");
  count = nx_active % 10;
  for (ix = 0; ix < nx_active; ix++) {
    if (count < 10) {
      fprintf(stderr, " ");
    } else {
      fprintf(stderr, "|");
      count = 0;
    }
    count++;
  } /// ix ///

  fprintf(stderr, "\n");

  // print grid

  for (iy = _grid.ny - 1; iy >= 0; iy--) {

    info_found = FALSE;

    for (ix = min_ix; ix <= max_ix; ix++)
      if (grid2[iy][ix] ||
	  grid1[iy][ix])
	info_found = TRUE;

    if (info_found) {

      fprintf(stderr, "%4ld ", (long) iy);
    
      for (ix = min_ix; ix <= max_ix; ix++) {

	if (grid2[iy][ix] > 0 &&
	    grid1[iy][ix] > 0) {

	  fprintf(stderr, "B");

	} else if (grid2[iy][ix] > 0 &&
		   grid1[iy][ix] == 0) {

	  fprintf(stderr, "1");
	  
	  
	} else if (grid2[iy][ix] == 0 &&
		   grid1[iy][ix] > 0) {

	  fprintf(stderr, "2");
	  
	} else {

	  fprintf(stderr, "-");
	  
	}
	
      } /// ix ///

      fprintf(stderr, "\n");

    } /// if (info_found) ///

  } /// iy ///

}

