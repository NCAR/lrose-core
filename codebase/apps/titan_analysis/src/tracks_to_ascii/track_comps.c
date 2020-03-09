/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/***************************************************************************
 * track_comps.c
 *
 * Routines which perform the computations on complete tracks, and write
 * the results to stdout
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 ***************************************************************************/

#include "tracks_to_ascii.h"

#define SMALL_ANGLE 0.000001
#define PI_BY_TWO 1.570796327
#define ALMOST_PI_BY_TWO (PI_BY_TWO - SMALL_ANGLE)
#ifndef PI
#define PI 3.141592654
#endif
#define ALMOST_PI (PI - SMALL_ANGLE)

/*
 * file scope variables
 */

static ui08 **Runs_swath_grid;
static ui08 **Ellipse_swath_grid;
static ui08 **Ellipse_grid;
static double **Precip_grid;

static double Rerv;
static double Tot_entries;
static double Nstorms_in_box;
static double Sum_volume, Max_volume;
static double Sum_precip_area, Max_precip_area;
static double Sum_proj_area, Max_proj_area;
static double Sum_mass, Max_mass;
static double Sum_precip_flux, Max_precip_flux;
static double Sum_dbz, Max_dbz;
static double Sum_base, Max_base;
static double Sum_top, Max_top;
static double Sum_u, Sum_v;
static double Rem_duration;
static titan_grid_t Grid;

static autocorr_t Storm_autocorr_volume;
static autocorr_t Storm_autocorr_vol_two_thirds;
static autocorr_t Storm_autocorr_precip_area;
static autocorr_t Storm_autocorr_proj_area;

/*
 * file scope prototypes
 */

static void increment_autocorrelation(autocorr_t *storm_auto,
				      autocorr_t *tot_auto);

static void load_autocorrelation(autocorr_t *autocorr,
				 double current);

static void update_ellipse_precip(storm_file_handle_t *s_handle,
				  storm_file_params_t *sparams,
				  storm_file_global_props_t *gprops);

static void set_ellipse_grid(double ellipse_x,
			     double ellipse_y,
			     double major_radius,
			     double minor_radius,
			     double axis_rotation,
			     si32 *start_ix_p,
			     si32 *start_iy_p,
			     si32 *end_ix_p,
			     si32 *end_iy_p,
			     ui08 **grid);

static void print_grid(char *label1,
		       char *label2,
		       ui08 **ellipse_grid,
		       ui08 **runs_grid);

/*****************************************************
 * initialize_track_comps()
 */

/*ARGSUSED*/

void initialize_track_comps(storm_file_handle_t *s_handle,
			    track_file_handle_t *t_handle)

{
  
  Tot_entries = 0.0;
  Nstorms_in_box = 0.0;

  Rerv = 0.0;
  Sum_volume = 0.0;
  Sum_mass = 0.0;
  Sum_precip_flux = 0.0;
  Sum_precip_area = 0.0;
  Sum_proj_area = 0.0;
  Sum_dbz = 0.0;
  Sum_base = 0.0;
  Sum_top = 0.0;
  Sum_u = 0.0;
  Sum_v = 0.0;
  
  memset ((void *) &Storm_autocorr_volume,
	  (int) 0, sizeof(autocorr_t));

  memset ((void *) &Storm_autocorr_vol_two_thirds,
	  (int) 0, sizeof(autocorr_t));

  memset ((void *) &Storm_autocorr_precip_area,
	  (int) 0, sizeof(autocorr_t));

  memset ((void *) &Storm_autocorr_proj_area,
	  (int) 0, sizeof(autocorr_t));

  Max_volume = -LARGE_DOUBLE;
  Max_mass = -LARGE_DOUBLE;
  Max_precip_flux = -LARGE_DOUBLE;
  Max_precip_area = -LARGE_DOUBLE;
  Max_proj_area = -LARGE_DOUBLE;
  Max_dbz = -LARGE_DOUBLE;
  Max_base = -LARGE_DOUBLE;
  Max_top = -LARGE_DOUBLE;

  /*
   * read in first storm scan
   */
  
  if (RfReadStormScan(s_handle, (si32) 0, "track_comps"))
    tidy_and_exit(-1);

  Grid = s_handle->scan->grid;

  /*
   * allocate grids
   */

  Runs_swath_grid = (ui08 **) ucalloc2
    ((ui32) Grid.ny, (ui32) Grid.nx, sizeof(ui08));
  
  Ellipse_swath_grid = (ui08 **) ucalloc2
    ((ui32) Grid.ny, (ui32) Grid.nx, sizeof(ui08));
  
  Ellipse_grid = (ui08 **) ucalloc2
    ((ui32) Grid.ny, (ui32) Grid.nx, sizeof(ui08));
  
  Precip_grid = (double **) ucalloc2
    ((ui32) Grid.ny, (ui32) Grid.nx, sizeof(double));
  
}

/*****************************************************
 * update_track_comps()
 */

/*ARGSUSED*/

void update_track_comps(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle)

{

  si32 irun;  
  si32 this_scan, last_scan;
  si32 precip_layer;

  double volume, vol_two_thirds;
  double mass, precip_flux;
  double precip_area, proj_area;
  double dbz_max, dbz_mean;
  double top, base;
  double u, v;
  double precip_z;

  date_time_t last_time, entry_time;
  storm_file_global_props_t *gprops;
  storm_file_run_t *run;
  storm_file_params_t *sparams;
  track_file_forecast_props_t *fprops;

  sparams = &s_handle->header->params;
  gprops = s_handle->gprops + t_handle->entry->storm_num;
  fprops = &t_handle->entry->dval_dt;

  if (Glob->use_box_limits) {
    
    if (gprops->refl_centroid_x >= Glob->box_min_x &&
	gprops->refl_centroid_x <= Glob->box_max_x &&
	gprops->refl_centroid_y >= Glob->box_min_y &&
	gprops->refl_centroid_y <= Glob->box_max_y) {

      Nstorms_in_box++;

    }

  } /* if (Glob->use_box_limits) */

  volume = gprops->volume;
  vol_two_thirds = pow(volume, 0.66666667);
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
  
  Rerv += precip_flux * Glob->scan_interval;
  Sum_volume += volume;
  Sum_mass += mass;
  Sum_precip_flux += precip_flux;
  Sum_precip_area += precip_area;
  Sum_proj_area += proj_area;
  Sum_top += top;
  Sum_base += base;
  Sum_dbz += dbz_mean;
  Sum_u += u;
  Sum_v += v;

  Max_mass = MAX(Max_mass, mass);
  Max_precip_flux = MAX(Max_precip_flux, precip_flux);
  Max_precip_area = MAX(Max_precip_area, precip_area);
  Max_proj_area = MAX(Max_proj_area, proj_area);
  Max_top = MAX(Max_top, top);
  Max_base = MAX(Max_base, base);
  Max_dbz = MAX(Max_dbz, dbz_max);

  if (Max_volume < volume) {

    Max_volume = volume;

    /*
     * compute life expectancy
     */
    
    last_time.unix_time =
      t_handle->simple_params->last_descendant_end_time;
    uconvert_from_utime(&last_time);

    entry_time.unix_time = t_handle->entry->time;
    uconvert_from_utime(&entry_time);

    this_scan = t_handle->entry->scan_num;
    last_scan = t_handle->simple_params->last_descendant_end_scan;

    if (this_scan == last_scan) {

      Rem_duration = 0.0;

    } else {

      /*
       * adjust for the fact that the times are taken at mid-scan,
       * by adding the duration of one-half of a scan
       */

      Rem_duration =
	((((double) last_time.unix_time -
	   (double) entry_time.unix_time) +
	  Glob->scan_interval / 2) / 3600.0);

    } /* if (this_scan == last_scan) */
      
  } /* if (Max_volume < volume) */

  Tot_entries++;

  /*
   * autocorrelation computations
   */

  load_autocorrelation(&Storm_autocorr_volume, volume);
  load_autocorrelation(&Storm_autocorr_vol_two_thirds, vol_two_thirds);
  load_autocorrelation(&Storm_autocorr_precip_area, precip_area);
  load_autocorrelation(&Storm_autocorr_proj_area, proj_area);

  /*
   * precip comps
   */

  if (gprops->precip_area > 0) {

    /*
     * check that the grid params have not changed
     */

    if (memcmp((void *) &Grid,
	       (void *) &s_handle->scan->grid,
	       sizeof(titan_grid_t))) {
      
      fprintf(stderr, "ERROR - %s:track_comps:update_track_comps\n",
	      Glob->prog_name);
      fprintf(stderr, "Grid params have changed.\n");
      return;
      
    }

    precip_z = s_handle->scan->min_z;
    precip_layer = (si32) ((precip_z - Grid.minz) / Grid.dz + 0.5);

    /*
     * update the Runs_swath_grid
     */

    if (RfReadStormProps(s_handle, t_handle->entry->storm_num,
			 "track_comps:update_track_comps")) {
      return;
    }

    run = s_handle->runs;
    
    for (irun = 0; irun < gprops->n_runs; irun++) {
      
      if (run->iz == precip_layer)
	memset((void *) (Runs_swath_grid[run->iy] + run->ix),
	       (int) 1, (int) run->n);
      
      run++;
      
    } /* irun */
    
    /*
     * update the ellipse_swath_grid and the precip grid, using
     * the precip area ellipse and the precip area reflectivity
     * distribution
     */
    
    update_ellipse_precip(s_handle, sparams, gprops);

  } /* if (gprops->precip_area > 0) */

}

/*****************************************************
 * finalize_track_comps()
 */

/*ARGSUSED*/

void finalize_track_comps(storm_file_handle_t *s_handle,
			  track_file_handle_t *t_handle,
			  autocorr_t *autocorr_volume,
			  autocorr_t *autocorr_vol_two_thirds,
			  autocorr_t *autocorr_precip_area,
			  autocorr_t *autocorr_proj_area)

{

  ui08 *runs_p;
  ui08 *ellipse_p;

  int use_track;

  si32 ipt;

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
  double pod, far, csi;
  double percent_in_box;
  double element_area, runs_swath_area;
  double ellipse_swath_area;

  complex_track_params_t *cparams;

  cparams = t_handle->complex_params;

  duration = (double) t_handle->complex_params->duration_in_secs / 3600.0;

  if (cparams->ellipse_verify.n_success == 0) {
    pod = 0.0;
  } else if (cparams->ellipse_verify.n_failure == 0) {
    pod = 1.0;
  } else {
    pod = (double) cparams->ellipse_verify.n_success /
      ((double) cparams->ellipse_verify.n_success +
       (double) cparams->ellipse_verify.n_failure);
  }

  if (cparams->ellipse_verify.n_success == 0) {
    far = 1.0;
  } else if (cparams->ellipse_verify.n_false_alarm == 0) {
    far = 0.0;
  } else {
    far = (double) cparams->ellipse_verify.n_false_alarm /
      ((double) cparams->ellipse_verify.n_success +
       (double) cparams->ellipse_verify.n_false_alarm);
  }

  if (cparams->ellipse_verify.n_success == 0) {
    csi = 0.0;
  } else if (cparams->ellipse_verify.n_failure == 0 &&
	     cparams->ellipse_verify.n_false_alarm == 0 ) {
    csi = 1.0;
  } else {
    csi = (double) cparams->ellipse_verify.n_success /
      ((double) cparams->ellipse_verify.n_success +
       (double) cparams->ellipse_verify.n_failure +
       (double) cparams->ellipse_verify.n_false_alarm);
  }

  mean_volume = Sum_volume / Tot_entries;
  mean_mass = Sum_mass / Tot_entries;
  mean_precip_flux = Sum_precip_flux / Tot_entries;
  mean_precip_area = Sum_precip_area / Tot_entries;
  mean_proj_area = Sum_proj_area / Tot_entries;
  mean_top = Sum_top / Tot_entries;
  mean_base = Sum_base / Tot_entries;
  mean_dbz = Sum_dbz / Tot_entries;
  mean_u = Sum_u / Tot_entries;
  mean_v = Sum_v / Tot_entries;

  mean_speed = sqrt(mean_u * mean_u + mean_v * mean_v);

  if (mean_u == 0.0 && mean_v == 0.0)
    mean_dirn = 0.0;
  else
    mean_dirn = atan2(mean_u, mean_v) * RAD_TO_DEG;

  if (mean_dirn < 0.0)
    mean_dirn += 360.0;

  percent_in_box = (Nstorms_in_box / Tot_entries) * 100.0;

  /*
   * autocorrelations
   */

  compute_autocorrelation(&Storm_autocorr_volume);
  compute_autocorrelation(&Storm_autocorr_vol_two_thirds);
  compute_autocorrelation(&Storm_autocorr_precip_area);
  compute_autocorrelation(&Storm_autocorr_proj_area);

  increment_autocorrelation(&Storm_autocorr_volume,
			    autocorr_volume);
  increment_autocorrelation(&Storm_autocorr_vol_two_thirds,
			    autocorr_vol_two_thirds);
  increment_autocorrelation(&Storm_autocorr_precip_area,
			    autocorr_precip_area);
  increment_autocorrelation(&Storm_autocorr_proj_area,
			    autocorr_proj_area);

  /*
   * area time integral (km2.hr)
   */

  ati = mean_precip_area * duration;
  
  /*
   * compute swath areas (km2)
   */

  if (Glob->debug)
    print_grid("Runs_swath", "Ellipse_swath",
	       Runs_swath_grid,
	       Ellipse_swath_grid);

  element_area = Grid.dx * Grid.dy;
  runs_swath_area = 0.0;
  ellipse_swath_area = 0.0;
  ellipse_max_precip_depth = 0.0;
  ellipse_sum_precip_depth = 0.0;
  ellipse_n_precip = 0.0;
  
  runs_p = *Runs_swath_grid;
  ellipse_p = *Ellipse_swath_grid;
  precip_p = *Precip_grid;
  
  for (ipt = 0; ipt < Grid.ny * Grid.nx; ipt++) {

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
    runs_mean_precip_depth = Rerv / (runs_swath_area * 1000.0);
  }

  if (ellipse_n_precip == 0.0) {
    ellipse_mean_precip_depth = 0.0;
  } else {
    ellipse_mean_precip_depth =
      ellipse_sum_precip_depth / ellipse_n_precip;
  }

  use_track = TRUE;

  if (Glob->use_box_limits) {
    
    if (percent_in_box < Glob->min_percent_in_box ||
	Nstorms_in_box < Glob->min_nstorms_in_box) {

      use_track = FALSE;

    }

  } /* if (Glob->use_box_limits) */

  if (Glob->nonzero_verification_only) {
    
    if (cparams->ellipse_verify.n_success == 0 ||
	(cparams->ellipse_verify.n_failure == 0 &&
	 cparams->ellipse_verify.n_false_alarm == 0)) {

      use_track = FALSE;

    }

  } /* if (Glob->nonzero_verification_only) */

  if (use_track) {

    printf("%ld %s %ld %g %g ",
	   (long) cparams->complex_track_num,
	   utimstr(cparams->start_time),
	   (long) cparams->duration_in_scans,
	   duration,
	   Rem_duration);
    
    printf("%g %g %g %g %g %g %g %g %g ",
	   mean_volume,
	   Max_volume,
	   mean_mass,
	   Max_mass,
	   ellipse_max_precip_depth,
	   ellipse_mean_precip_depth,
	   runs_mean_precip_depth,
	   mean_precip_flux,
	   Max_precip_flux);

    printf("%g %g %g %g %g %g %g %g %g %g ",
	   mean_proj_area,
	   Max_proj_area,
	   mean_precip_area,
	   Max_precip_area,
	   mean_top,
	   Max_top,
	   mean_base,
	   Max_base,
	   mean_dbz,
	   Max_dbz);

    printf("%g %g %g %g %g %g ",
	   Storm_autocorr_volume.lag[1].corr,
	   Storm_autocorr_volume.lag[2].corr,
	   Storm_autocorr_precip_area.lag[1].corr,
	   Storm_autocorr_precip_area.lag[2].corr,
	   Storm_autocorr_proj_area.lag[1].corr,
	   Storm_autocorr_proj_area.lag[2].corr);

    printf("%g %g %g %g %g %g %g %g %g\n",
	   Rerv,
	   ati,
	   ellipse_swath_area,
	   runs_swath_area,
	   mean_speed,
	   mean_dirn,
	   pod,
	   far,
	   csi);
    
  }

  /*
   * free up mem
   */

  ufree2((void **) Runs_swath_grid);
  ufree2((void **) Ellipse_swath_grid);
  ufree2((void **) Ellipse_grid);
  ufree2((void **) Precip_grid);

}

/*******************************************************************
 * compute_autocorrelation()
 */

void compute_autocorrelation(autocorr_t *autocorr)

{
 
  si32 ilag;
  double num, denomsq;
  corr_t *corr;
  
  for (ilag = 0; ilag < NLAGS; ilag++) {
    
    corr = autocorr->lag + ilag;
    
    denomsq =
      ((corr->n * corr->sumx2 - corr->sumx * corr->sumx) *
       (corr->n * corr->sumy2 - corr->sumy * corr->sumy));

    num = corr->n * corr->sumxy - corr->sumx * corr->sumy;

    if (corr->n > 10 && denomsq > 0.0) {
      corr->corr = num / sqrt(denomsq);
    } else {
      corr->corr = 0.0;
    }

  } /* ilag */

}

/*******************************************************************
 * increment_autocorrelation()
 */

static void increment_autocorrelation(autocorr_t *storm_auto,
				      autocorr_t *tot_auto)

{
 
  si32 ilag;
  corr_t *storm_corr, *tot_corr;
  
  for (ilag = 0; ilag < NLAGS; ilag++) {
    
    storm_corr = storm_auto->lag + ilag;
    tot_corr = tot_auto->lag + ilag;
    
    tot_corr->sumx += storm_corr->sumx;
    tot_corr->sumx2 += storm_corr->sumx2;
    tot_corr->sumy += storm_corr->sumy;
    tot_corr->sumy2 += storm_corr->sumy2;
    tot_corr->sumxy += storm_corr->sumxy;
    tot_corr->n += storm_corr->n;

  } /* ilag */

}

/*******************************************************************
 * load_autocorrelation()
 *
 * Load accumulators for autocorrelation comps
 */

static void load_autocorrelation(autocorr_t *storm_auto,
				 double current)

{
 
  si32 ilag;
  double prev;
  corr_t *corr;

  storm_auto->lag[0].val = current;
  storm_auto->lag[0].val_available = TRUE;
  
  for (ilag = 0; ilag < NLAGS; ilag++) {
    
    corr = storm_auto->lag + ilag;
    
    if (corr->val_available) {

      prev = corr->val;

      corr->sumx += prev;
      corr->sumx2 += prev * prev;
      corr->sumy += current;
      corr->sumy2 += current * current;
      corr->sumxy += prev * current;
      corr->n++;

    } /* if (corr->val_available) */
    
  } /* ilag */

  for (ilag = NLAGS - 1; ilag > 0; ilag--) {

    storm_auto->lag[ilag].val = storm_auto->lag[ilag - 1].val;

    storm_auto->lag[ilag].val_available =
      storm_auto->lag[ilag - 1].val_available;

  } /* ilag */

}

/*******************************************************************
 * print_autocorrelation()
 */

void print_autocorrelation(char *label,
			   autocorr_t *autocorr)

{
 
  si32 ilag;

  fprintf(stderr, "%s autocorrelation function : lag(min) : corr\n", label);
  
  for (ilag = 0; ilag < NLAGS; ilag++)
    fprintf(stderr, "%10g %10g\n",
	    (double) ilag * Glob->scan_interval / 60.0,
	    autocorr->lag[ilag].corr);
  
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

static void set_ellipse_grid(double ellipse_x,
			     double ellipse_y,
			     double major_radius,
			     double minor_radius,
			     double axis_rotation,
			     si32 *start_ix_p,
			     si32 *start_iy_p,
			     si32 *end_ix_p,
			     si32 *end_iy_p,
			     ui08 **grid)

{

  si32 ix, iy, ix1, iy1, ix2, iy2;
  si32 start_ix, start_iy;
  si32 end_ix, end_iy;

  double grid_rotation, theta;
  double slope_prime, intercept_prime;
  double sin_rotation, cos_rotation, tan_rotation;

  double xprime1, yprime1, xprime2, yprime2;
  double x_1, y_1, x_2, y_2;
  double start_x, start_y;
  double end_x, end_y;
  double line_x, line_y;

  /*
   * compute the grid_rotation, taking care to avoid 0, pi/2 and
   * pi, so that the trig functions will not fail. Remember that
   * the axis_rotation is relative to True North, and we need to
   * compute the grid rotation relative to the mathmatically
   * conventional axes
   */

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
  
  /*
   * compute the start and end x and y - these values are
   * chosen for a circle of radius major_radius, which will
   * enclose the ellipse
   */

  start_x = ellipse_x - major_radius;
  start_y = ellipse_y - major_radius;

  end_x = ellipse_x + major_radius;
  end_y = ellipse_y + major_radius;

  /*
   * set the end and start grid indices
   */

  start_ix = (si32) ((start_x - Grid.minx) / Grid.dx + 0.49);
  start_ix = MAX(start_ix, 0);

  start_iy = (si32) ((start_y - Grid.miny) / Grid.dy + 0.49);
  start_iy = MAX(start_iy, 0);

  end_ix = (si32) ((end_x - Grid.minx) / Grid.dx + 0.51);
  end_ix = MIN(end_ix, Grid.nx - 1);

  end_iy = (si32) ((end_y - Grid.miny) / Grid.dy + 0.51);
  end_iy = MIN(end_iy, Grid.ny - 1);

  *start_ix_p = start_ix;
  *start_iy_p = start_iy;
  *end_ix_p = end_ix;
  *end_iy_p = end_iy;

  /*
   * flag the grid region which contains the ellipse centroid
   */

  ix = (si32) ((ellipse_x - Grid.minx) / Grid.dx + 0.5);
  iy = (si32) ((ellipse_y - Grid.miny) / Grid.dy + 0.5);

  if (ix >= start_ix && ix <= end_ix &&
      iy >= start_iy && iy <= end_iy)
    grid[iy][ix] = 1;

  /*
   * loop through the vertical lines which intersect the ellipse
   */

  for (ix = start_ix; ix < end_ix; ix++) {

    /*
     * compute the slope and intercept of this line in the
     * transformed coordinate system with ths origin at the
     * center of the ellipse and the x-axis along the major
     * axis. The prime values refer to the transformed
     * coord system.
     */

    
    line_x = Grid.minx + ((double) ix + 0.5) * Grid.dx;

    slope_prime = 1.0 / tan_rotation;

    intercept_prime  = - (line_x - ellipse_x) / sin_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2)) {

      /*
       * transform the points back into grid coords
       */

      y_1 = ellipse_y + xprime1 * sin_rotation + yprime1 * cos_rotation;
      y_2 = ellipse_y + xprime2 * sin_rotation + yprime2 * cos_rotation;

      if  (y_1 <= y_2) {

	iy1 = (si32) ((y_1 - Grid.miny) / Grid.dy + 0.5);
	iy2 = (si32) ((y_2 - Grid.miny) / Grid.dy + 0.5);

      } else {

	iy1 = (si32) ((y_2 - Grid.miny) / Grid.dy + 0.5);
	iy2 = (si32) ((y_1 - Grid.miny) / Grid.dy + 0.5);

      }

      iy1 = MAX(iy1, 0);
      iy2 = MIN(iy2, Grid.ny - 1);

      for (iy = iy1; iy <= iy2; iy++) {

	grid[iy][ix] = 1;
	grid[iy][ix + 1] = 1;

      } /* iy */

    } /* if (uline_through_ellipse(major_radius ... */

  } /* ix */

  /*
   * loop through the horizontal lines which intersect the ellipse
   */

  for (iy = start_iy; iy < end_iy; iy++) {

    /*
     * compute the slope and intercept of this line in the
     * transformed coordinate system with ths origin at the
     * center of the ellipse and the x-axis along the major
     * axis. The prime values refer to the transformed
     * coord system.
     */

    
    line_y = Grid.miny + ((double) iy + 0.5) * Grid.dy;

    slope_prime = - tan_rotation;

    intercept_prime  = (line_y - ellipse_y) / cos_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2)) {

      /*
       * transform the points back into grid coords
       */

      x_1 = ellipse_x + xprime1 * cos_rotation - yprime1 * sin_rotation;
      x_2 = ellipse_x + xprime2 * cos_rotation - yprime2 * sin_rotation;

      if  (x_1 <= x_2) {

	ix1 = (si32) ((x_1 - Grid.minx) / Grid.dx + 0.5);
	ix2 = (si32) ((x_2 - Grid.minx) / Grid.dx + 0.5);

      } else {

	ix1 = (si32) ((x_2 - Grid.minx) / Grid.dx + 0.5);
	ix2 = (si32) ((x_1 - Grid.minx) / Grid.dx + 0.5);

      }

      ix1 = MAX(ix1, 0);
      ix2 = MIN(ix2, Grid.nx - 1);

      for (ix = ix1; ix <= ix2; ix++) {

	grid[iy][ix] = 1;
	grid[iy + 1][ix] = 1;

      } /* ix */

    } /* if (uline_through_ellipse(major_radius ... */

  } /* iy */

}

/***************************************************************
 * update_ellipse_precip()
 *
 * Updates the grids based on the storm precip area ellipses,
 * and uses the reflectivity distribution to estimate the
 * precip swath
 */

static void update_ellipse_precip(storm_file_handle_t *s_handle,
				  storm_file_params_t *sparams,
				  storm_file_global_props_t *gprops)

{

  ui08 *ellipse_p;
  ui08 *ellipse_swath_p;

  si32 interval;
  si32 n_dbz_intervals;
  si32 start_ix, start_iy;
  si32 end_ix, end_iy;
  si32 ix, iy;

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

  /*
   * set parameters
   */

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
  
  /*
   * compute the area and incremental precip depth for
   * each reflectivity interval
   */

  low_dbz = low_dbz_threshold;
  mean_dbz = low_dbz + dbz_hist_interval / 2.0;
  
  accum_depth = 0.0;
  hist = s_handle->hist;
  area_fraction = 1.0;

  for (interval = 0; interval < n_dbz_intervals; interval++, hist++) {

    z = pow(10.0, low_dbz / 10.0);
    precip_rate = pow((z / z_p_coeff), (1.0 / z_p_exponent));
    precip_depth = (precip_rate * (double) Glob->scan_interval) / 3600.0;
    incr_depth = precip_depth - accum_depth;
    percent_area = hist->percent_area;

    /*
     * zero out ellipse grid
     */
    
    memset((void *) *Ellipse_grid,
	   (int) 0,
	   (int) (Grid.nx * Grid.ny * sizeof(ui08)));

    /*
     * set points in ellipse grid
     */

    frac_major_radius = major_radius * sqrt(area_fraction);
    frac_minor_radius = minor_radius * sqrt(area_fraction);

    set_ellipse_grid(ellipse_x,
		     ellipse_y,
		     frac_major_radius,
		     frac_minor_radius,
		     axis_rotation,
		     &start_ix, &start_iy,
		     &end_ix, &end_iy,
		     Ellipse_grid);

    /*
     * set the grids
     */

    for (iy = start_iy; iy < end_iy; iy++) {

      ellipse_p = Ellipse_grid[iy] + start_ix;
      precip_p = Precip_grid[iy] + start_ix;
      ellipse_swath_p = Ellipse_swath_grid[iy] + start_ix;

      for (ix = start_ix; ix < end_ix; ix++) {

	if (*ellipse_p) {
	  *precip_p += incr_depth;
	  *ellipse_swath_p = 1;
	}

	ellipse_p++;
	precip_p++;
	ellipse_swath_p++;

      } /* ix */

    } /* iy */

    /*
     * increment and test for end of loop conditions
     */
  
    accum_depth = precip_depth;
    area_fraction -= percent_area / 100.0;
    low_dbz += dbz_hist_interval;
    mean_dbz += dbz_hist_interval;

    if (low_dbz > hail_dbz_threshold)
      break;

    if (area_fraction <= 0.001)
      break;

  } /* interval */

}

/****************************************************************
 * print_grid()
 */

static void print_grid(char *label1,
		       char *label2,
		       ui08 **grid1,
		       ui08 **grid2)

{

  int info_found;
  si32 count;
  si32 ix, iy;
  si32 min_ix, max_ix, nx_active;

  /*
   * search for max and min x activity
   */

  min_ix = 1000000;
  max_ix = 0;
  
  for (iy = Grid.ny - 1; iy >= 0; iy--) {

    for (ix = 0; ix < Grid.nx; ix++) {

      if (grid2[iy][ix] ||
	  grid1[iy][ix]) {
	
	min_ix = MIN(min_ix, ix);
	max_ix = MAX(max_ix, ix);
	
      } /* if */
      
    } /* ix */

  } /* iy */

  nx_active = max_ix - min_ix + 1;

  /*
   * print header
   */

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
  } /* ix */

  fprintf(stderr, "\n");

  /*
   * print grid
   */

  for (iy = Grid.ny - 1; iy >= 0; iy--) {

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
	
      } /* ix */

      fprintf(stderr, "\n");

    } /* if (info_found) */

  } /* iy */

}



