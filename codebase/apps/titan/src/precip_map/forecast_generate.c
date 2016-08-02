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

#include "precip_map.h"

/*
 * file scope variables
 */

static int N_forecast_intervals;
static int N_intervals_alloc = 0;
static double *Interval_contrib_time = NULL;
static double *Interval_lead_time = NULL;

/*
 * function prototypes
 */

static void initialize(double lead_time);

static ui08 *thresholded_forecast(storm_file_handle_t *s_handle,
				  track_file_handle_t *t_handle,
				  vol_file_handle_t *v_handle,
				  vol_file_handle_t *map_v_handle,
				  mdv_grid_t *grid,
				  int scan_num,
				  double *precip_lookup);
     
static ui08 *unthresholded_forecast(storm_file_handle_t *s_handle,
				    track_file_handle_t *t_handle,
				    vol_file_handle_t *v_handle,
				    vol_file_handle_t *map_v_handle,
				    mdv_grid_t *grid,
				    int scan_num,
				    double *precip_lookup);

/****************************************************************************
 * forecast_generate.c
 *
 * generate the map
 *
 * returns time of last file written, -1 if no file written
 *
 * July 1993
 *
 ****************************************************************************/

time_t forecast_generate(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 vol_file_handle_t *v_handle,
			 vol_file_handle_t *map_v_handle,
			 date_time_t *scan_times,
			 si32 scan_num,
			 double lead_time_requested)

{

  ui08 *forecast;

  char rdata_file_path[MAX_PATH_LEN];
  char map_file_dir[MAX_PATH_LEN];
  char map_file_path[MAX_PATH_LEN];

  int n_scans;

  double lead_time_used, lead_time_hr;
  double precip_lookup[256];
  double coeff, expon;

  time_t time_this_scan, time_latest_scan;
  time_t scan_time_diff;
  time_t forecast_time;

  date_time_t *stime, ftime;
  cart_params_t *cart;
  mdv_grid_t grid;
  field_params_t *dbz_fparams;

  PMU_auto_register("In forecast_generate");

  lock_and_read_headers(s_handle, t_handle);
  
  /*
   * set up times
   */

  stime = scan_times + scan_num;

  time_this_scan = stime->unix_time;
  scan_time_diff = time_latest_scan - time_this_scan;
  
  n_scans = s_handle->header->n_scans;
  time_latest_scan = scan_times[n_scans - 1].unix_time;

  /*
   * compute forecast time
   */

  forecast_time = time_this_scan + lead_time_requested;
  if (Glob->params.round_forecast_times) {
    forecast_time = (time_t)
      ((int) floor((double) forecast_time /
		   Glob->params.rounding_interval + 0.5) *
       Glob->params.rounding_interval);
  }
  lead_time_used = forecast_time - time_this_scan;
  lead_time_hr = lead_time_used / 3600.0;
  initialize(lead_time_used);

  /*
   * get path name for map file
   */

  ftime.unix_time = stime->unix_time;
     
  uconvert_from_utime(&ftime);
    
  sprintf(map_file_dir,
	  "%s%s%.4ld%.2ld%.2ld",
	  Glob->params.map_dir, PATH_DELIM,
	  (long) ftime.year, (long) ftime.month, (long) ftime.day);

  sprintf(map_file_path,
	  "%s%s%.2ld%.2ld%.2ld.%s",
	  map_file_dir, PATH_DELIM,
	  (long) ftime.hour, (long) ftime.min, (long) ftime.sec,
	  Glob->params.output_file_ext);

  /*
   * Check if we should write this file. If not, return -1
   */

  if (!check_write(map_file_path)) {
    unlock_files(s_handle, t_handle);
    return (-1);
  }
    
  sprintf(rdata_file_path,
	  "%s%s%.4ld%.2ld%.2ld%s%.2ld%.2ld%.2ld.%s",
	  Glob->params.rdata_dir, PATH_DELIM,
	  (long) stime->year, (long) stime->month,
	  (long) stime->day,
	  PATH_DELIM,
	  (long) stime->hour, (long) stime->min, (long) stime->sec,
	  Glob->params.output_file_ext);
	    
  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "\nScan %ld, time %s\n", (long) scan_num,
	    utimestr(stime));
    fprintf(stderr, "Rdata path: %s\n", rdata_file_path);
    fprintf(stderr, "Map path: %s\n", map_file_path);
    fprintf(stderr, "lead_time_requested: %g\n", lead_time_requested);
    fprintf(stderr, "lead_time_used: %g\n", lead_time_used);
  } /* if (Glob->params.debug >= DEBUG_NORM) */

  /*
   * read in radar volume
   */
  
  v_handle->vol_file_path = rdata_file_path;

  if (RfReadVolume(v_handle, "forecast_generate")) {
    fprintf(stderr, "ERROR - %s:forecast_generate\n", Glob->prog_name);
    fprintf(stderr, "Could not read in Mdv radar volume\n");
    perror(rdata_file_path);
    unlock_files(s_handle, t_handle);
    return (-1);
  }

  /*
   * get ZR params
   */
    
  if (Glob->params.get_zr_from_file) {
    if (RfGetZrClosest(Glob->params.zr_dir,
		       stime->unix_time, 7200,
		       &coeff, &expon)) {
      coeff = Glob->params.ZR.coeff;
      expon = Glob->params.ZR.expon;
    }
  } else {
    coeff = Glob->params.ZR.coeff;
    expon = Glob->params.ZR.expon;
  }

  dbz_fparams = v_handle->field_params[Glob->params.dbz_field];
  compute_precip_lookup(precip_lookup, dbz_fparams,
			coeff, expon);
    
  /*
   * check geometry
   */
  
  cart = &v_handle->vol_params->cart;
  if (check_cart_geom(cart, &s_handle->scan->grid)) {
    fprintf(stderr, "ERROR - %s:forecast_generate\n", Glob->prog_name);
    fprintf(stderr, "Radar and storm file cart geometry does not match\n");
    unlock_files(s_handle, t_handle);
    return (-1);
  }
  cart_params_to_mdv_grid(cart, &grid, s_handle->scan->grid.proj_type);
  
  /*
   * read in storm file scan
   */
  
  if (RfReadStormScan(s_handle, scan_num, "forecast_generate")) {
    unlock_files(s_handle, t_handle);
    return (-1);
  }
  
  init_map_index(map_v_handle, v_handle, &ftime);

  /*
   * compute forecast
   */
      
  if (Glob->params.thresholded_forecast) {
    
    forecast =
      thresholded_forecast(s_handle, t_handle, v_handle, map_v_handle,
			   &grid, scan_num,
			   precip_lookup);
    
  } else { /* if (Glob->params.thresholded_forecast) */
    
    forecast =
      unthresholded_forecast(s_handle, t_handle, v_handle, map_v_handle,
			     &grid, scan_num,
			     precip_lookup);
      
  } /* if (Glob->params.thresholded_forecast) */

  if (forecast != NULL) {
      
    map_v_handle->field_plane[0][0] = forecast;
      
    /*
     * write the map file
     */
      
    sprintf(map_v_handle->vol_params->note,
	    "%s\n%s%g\n%s%g\n%s%g\n",
	    "Precipitation forecast",
	    "Refl threshold: ", s_handle->header->params.low_dbz_threshold,
	    "Z-R coeff : ", coeff,
	    "Z-R expon : ", expon);
      
    sprintf(map_v_handle->field_params[0]->name,
	    "%g hr forecast ahead of time", lead_time_hr);
      
    RfWriteDobson(map_v_handle, FALSE, Glob->params.debug,
		  Glob->params.map_dir,
		  Glob->params.output_file_ext,
		  Glob->prog_name,
		  "forecast_generate");
      
  }

  unlock_files(s_handle, t_handle);
  return (ftime.unix_time);

}

static ui08 *thresholded_forecast(storm_file_handle_t *s_handle,
				  track_file_handle_t *t_handle,
				  vol_file_handle_t *v_handle,
				  vol_file_handle_t *map_v_handle,
				  mdv_grid_t *grid,
				  int scan_num,
				  double *precip_lookup)
     
{

  static int First_call = TRUE;
  static ui08 *Forecast_grid;
  static ui08 *Comp_grid;
  static double *Precip_grid;
  static int Npoints;
  
  ui08 *dbz_plane;
  ui08 *comp, *layer;
  ui08 comp_val;

  int comp_base, comp_top;

  si32 ix, jx, iy, jy;
  si32 ientry, irun, i_int;
  si32 n_entries;
  si32 grid_index;

  si32 cart_min_ix, cart_min_iy;
  si32 cart_max_ix, cart_max_iy;

  double lead_time_hr;
  double time_to_ground_hr;
  double dx_dt, dy_dt;
  double darea_dt;
  double current_area, forecast_area;
  double area_ratio, length_ratio;
  double grid_x, grid_y;
  double current_x, current_y;
  double forecast_x, forecast_y;
  double delta_area;
  double delta_precip;
  double time_to_zero_growth;

  double x, y;
  double bounding_minx, bounding_miny;
  double bounding_maxx, bounding_maxy;
  double xratio, yratio;
  double forecast_ix, forecast_iy;
  double contrib_time;

  mdv_grid_t fcast_cart;
  cart_params_t *cart;
  field_params_t *precip_fparams;
  storm_file_global_props_t *gprops;
  storm_file_run_t *run;
  track_file_entry_t *entry;
  track_file_forecast_props_t *fprops;

  cart = &v_handle->vol_params->cart;

  /*
   * allocate grids
   */
  
  if (First_call) {
    Npoints = grid->ny * grid->nx;
    Forecast_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    Comp_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    Precip_grid = (double *) umalloc (Npoints * sizeof(double));
    First_call = FALSE;
  }
  
  /*
   * zero out precip grid
   */
  
  memset((void *) Precip_grid, 0, Npoints * sizeof(double));

  /*
   * read in track file scan entries
   */
  
  if (RfReadTrackScanEntries(t_handle, scan_num, "forecast_generate"))
    return (NULL);
  
  n_entries = t_handle->scan_index[scan_num].n_entries;
  
  /*
   * loop through the entries
   */
  
  entry = t_handle->scan_entries;
  
  for (ientry = 0; ientry < n_entries; ientry++, entry++) {

    gprops = s_handle->gprops + entry->storm_num;
    fprops = &entry->dval_dt;
    
    /*
     * read in storm props
     */
    
    if (RfReadStormProps(s_handle, entry->storm_num, "forecast_generate")) {
      return (NULL);
    }
    
    /*
     * compute current posn and area
     */
    
    current_x = gprops->proj_area_centroid_x;
    current_y = gprops->proj_area_centroid_y;
    current_area = gprops->proj_area;
    
    /*
     * compute current posn in terms of the cartesian grid
     */
    
    grid_x = (current_x - grid->minx) / grid->dx;
    grid_y = (current_y - grid->miny) / grid->dy;
    
    /*
     * compute rates of change of posn and area
     */
    
    if (entry->forecast_valid) {
      dx_dt = fprops->proj_area_centroid_x;
      dy_dt = fprops->proj_area_centroid_y;
      darea_dt = fprops->proj_area;
    } else {
      dx_dt = 0.0;
      dy_dt = 0.0;
      darea_dt = 0.0;
    }
    
    /*
     * loop through the forecast intervals
     */
    
    for (i_int = 0; i_int < N_forecast_intervals; i_int++) {
      
      contrib_time = Interval_contrib_time[i_int];
      lead_time_hr = Interval_lead_time[i_int];
      time_to_ground_hr = Glob->params.time_to_ground / 3600.0;

      /*
       * compute the forecast storm position and area
       * for the forecast interval time, allowing for the time
       * the precip takes to get to the ground
       */
      
      forecast_x =
	current_x + dx_dt * (lead_time_hr + time_to_ground_hr);
      forecast_y =
	current_y + dy_dt * (lead_time_hr + time_to_ground_hr);
      
      if (darea_dt < 0.0) {
	
	/*
	 * linear trend for decay
	 */
	
	delta_area = darea_dt * lead_time_hr;
	
      } else {
	
	/*
	 * second order trend for growth
	 */
	
	time_to_zero_growth = 1000.0;
	delta_area = (darea_dt * lead_time_hr  -
		      (darea_dt * lead_time_hr * lead_time_hr) /
		      (time_to_zero_growth * 2.0));
	
      }
	  
      forecast_area = current_area + delta_area;
      
      if (forecast_area <= 0)
	continue;
      
      /*
       * Compute the cartesian grid parameters at the forecast time.
       * Here we are assuming that the storm grows or decays in area
       * but keeps the same reflectivity pattern. So we make the
       * precip map by moving and growing/shrinking the reflectivty
       * grid and remapping onto a precip map
       */

      area_ratio = forecast_area / current_area;
      length_ratio = sqrt(area_ratio);
      
      fcast_cart.dx = grid->dx * length_ratio;
      fcast_cart.dy = grid->dy * length_ratio;
      
      fcast_cart.minx = forecast_x - grid_x * fcast_cart.dx;
      fcast_cart.miny = forecast_y - grid_y * fcast_cart.dy;
      
      /*
       * zero out comp grid
       */
      
      memset((void *) Comp_grid, 0, Npoints * sizeof(ui08));
      
      /*
       * load up comp grid
       */

      comp_base = cart_comp_base(cart);
      comp_top = cart_comp_top(cart);

      run = s_handle->runs;
      for (irun = 0; irun < gprops->n_runs; irun++, run++) {

	if (run->iz < comp_base || run->iz > comp_top) {
	  continue;
	}
	
	dbz_plane = v_handle->field_plane[Glob->params.dbz_field][run->iz];
	
	grid_index = run->iy * grid->nx + run->ix;
	layer = dbz_plane + grid_index;
	comp = Comp_grid + grid_index;
	
	for (ix = 0; ix < run->n; ix++, comp++, layer++) {
	  if (*comp < *layer) {
	    *comp = *layer;
	  }
	} /* ix */
	
      } /* irun */

      /*
       * remap comp grid onto forecast grid, accounting for the
       * storm movement and change in size. Accumulate the precip
       * depth
       */

      bounding_minx =
	gprops->bounding_min_ix * fcast_cart.dx + fcast_cart.minx;
      cart_min_ix =
	(long) ((bounding_minx - grid->minx) / grid->dx + 0.5);
      cart_min_ix = MAX(0, cart_min_ix); 
      cart_min_ix = MIN(grid->nx - 1, cart_min_ix); 
      
      bounding_miny =
	gprops->bounding_min_iy * fcast_cart.dy + fcast_cart.miny;
      cart_min_iy =
	(long) ((bounding_miny - grid->miny) / grid->dy + 0.5);
      cart_min_iy = MAX(0, cart_min_iy); 
      cart_min_iy = MIN(grid->ny - 1, cart_min_iy); 
      
      bounding_maxx =
	gprops->bounding_max_ix * fcast_cart.dx + fcast_cart.minx;
      cart_max_ix =
	(long) ((bounding_maxx - grid->minx) / grid->dx + 0.5);
      cart_max_ix = MAX(0, cart_max_ix); 
      cart_max_ix = MIN(grid->nx - 1, cart_max_ix); 
      
      bounding_maxy =
	gprops->bounding_max_iy * fcast_cart.dy + fcast_cart.miny;
      cart_max_iy =
	(long) ((bounding_maxy - grid->miny) / grid->dy + 0.5);
      cart_max_iy = MAX(0, cart_max_iy); 
      cart_max_iy = MIN(grid->ny - 1, cart_max_iy); 
      
      y = cart_min_iy * grid->dy + grid->miny;
      yratio = grid->dy / fcast_cart.dy;
      forecast_iy = (y - fcast_cart.miny) / fcast_cart.dy;
      
      for (iy = cart_min_iy; iy <= cart_max_iy;
	   iy++, forecast_iy += yratio) {
	
	jy = (long) (forecast_iy + 0.5);
	
	x = cart_min_ix * grid->dx + grid->minx;
	xratio = grid->dx / fcast_cart.dx;
	forecast_ix = (x - fcast_cart.minx) / fcast_cart.dx;
	
	for (ix = cart_min_ix; ix <= cart_max_ix;
	     ix++, forecast_ix += xratio) {
	  
	  jx = (long) (forecast_ix + 0.5);
	  
	  comp_val = Comp_grid[jy * grid->nx + jx];
	  grid_index = iy * grid->nx + ix;
	  
	  delta_precip = precip_lookup[comp_val] * contrib_time;

	  Precip_grid[grid_index] += delta_precip;
	  
	} /* ix */
	
      } /* iy */
      
    } /* i_int */
    
  } /* ientry */

  /*
   * scale the precip array and copy into the first field
   * of the volume index
   */
  
  precip_fparams = map_v_handle->field_params[0];
  
  scale_data(Precip_grid, Forecast_grid,
	     Npoints,
	     precip_fparams->factor,
	     &precip_fparams->scale,
	     &precip_fparams->bias);
  
  return (Forecast_grid);
  
}

static ui08 *unthresholded_forecast(storm_file_handle_t *s_handle,
				    track_file_handle_t *t_handle,
				    vol_file_handle_t *v_handle,
				    vol_file_handle_t *map_v_handle,
				    mdv_grid_t *grid,
				    int scan_num,
				    double *precip_lookup)

{

  static int First_call = TRUE;
  static ui08 *Forecast_grid;
  static ui08 *Comp_grid;
  static double *Precip_grid;
  static si32 Npoints;
  
  ui08 *refl;

  int ix, iy, i_int;
  int delta_ix, delta_iy;
  int refl_start_ix, refl_end_ix;
  int refl_start_iy, refl_end_iy;

  double lead_time_hr;
  double time_to_ground_hr;
  double forecast_delta_x;
  double forecast_delta_y;
  double mean_dx_dt;
  double mean_dy_dt;
  double contrib_time;
  double *precip;
  
  field_params_t *precip_fparams;

  /*
   * allocate grids
   */
  
  if (First_call) {
    Npoints = grid->ny * grid->nx;
    Forecast_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    Comp_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    Precip_grid = (double *) umalloc (Npoints * sizeof(double));
    First_call = FALSE;
  }
  
  /*
   * zero out precip grid
   */
  
  memset((void *) Precip_grid, 0, Npoints * sizeof(double));

  /*
   * create composite grid
   */
  
  create_composite(Npoints, mdv_comp_base(grid), mdv_comp_top(grid),
		   v_handle->field_plane[Glob->params.dbz_field],
		   Comp_grid);

  if (get_mean_motion(s_handle, t_handle,
		      scan_num, &mean_dx_dt, &mean_dy_dt)) {
    
    return (NULL);

  }

  /*
   * loop through the forecast times
   */
  
  for (i_int = 0; i_int < N_forecast_intervals; i_int++) {

    contrib_time = Interval_contrib_time[i_int];
    lead_time_hr = Interval_lead_time[i_int];
    time_to_ground_hr = Glob->params.time_to_ground / 3600.0;

    forecast_delta_x =
      (mean_dx_dt * (lead_time_hr + time_to_ground_hr) / grid->dx);
    forecast_delta_y =
      (mean_dy_dt * (lead_time_hr + time_to_ground_hr) / grid->dy);
    
    delta_ix = (long) floor(forecast_delta_x + 0.5);
    delta_iy = (long) floor(forecast_delta_y + 0.5);

    /*
     * move the refl grid by the forecast amount in each dirn,
     * and map it onto the precip grid
     */

    if (delta_ix < 0) {
      refl_start_ix = -delta_ix;
      refl_end_ix = grid->nx - 1;
    } else {
      refl_start_ix = 0;
      refl_end_ix = grid->nx - 1 - delta_ix;
    }
    
    if (delta_iy < 0) {
      refl_start_iy = -delta_iy;
      refl_end_iy = grid->ny - 1;
    } else {
      refl_start_iy = 0;
      refl_end_iy = grid->ny - 1 - delta_iy;
    }

    if (Glob->params.debug) {
      fprintf(stderr, "forecast_delta_x, forecast_delta_y: %g, %g\n",
	      forecast_delta_x, forecast_delta_y);
      fprintf(stderr, "delta_ix, delta_iy: %d, %d\n", delta_ix, delta_ix);
      fprintf(stderr, "refl_start_ix, refl_start_iy: %d, %d\n",
	      refl_start_ix, refl_start_ix);
      fprintf(stderr, "refl_end_ix, refl_end_iy: %d, %d\n",
	      refl_end_ix, refl_end_ix);
    }

    for (iy = refl_start_iy; iy <= refl_end_iy; iy++) {

      refl = Comp_grid + (iy * grid->nx) + refl_start_ix;
      precip = (Precip_grid + ((iy + delta_iy) * grid->nx) +
		(refl_start_ix + delta_ix));
      
      for (ix = refl_start_ix; ix <= refl_end_ix;
	   ix++, refl++, precip++) {
	*precip += precip_lookup[*refl] * contrib_time;
      } /* ix */

    } /* iy */
    
  } /* i_int */

  /*
   * scale the precip array and copy into the first field
   * of the volume index
   */
  
  precip_fparams = map_v_handle->field_params[0];
  
  scale_data(Precip_grid, Forecast_grid,
	     Npoints,
	     precip_fparams->factor,
	     &precip_fparams->scale,
	     &precip_fparams->bias);
  
  return (Forecast_grid);
  
}


/************************************************************************
 * initialize()
 *
 * compute the number of forecast intervals, the lead time and the
 * time contribution for each forecast interval. It is assumed that the
 * current data will be used for estimating the precip for the
 * first half of an interval. Then each interval will contribute 
 * an interval. The last interval will contribute any time remaining
 * to the end of the forecast duration. Times are computed in hours
 * because track entry rates_of_change are given in hours.
 */

static void initialize(double lead_time)

{

  int i_int;

  /*
   * init forecast times
   */

  N_forecast_intervals = (int) (lead_time /
			    Glob->params.scan_interval + 0.5);

  N_forecast_intervals = MAX(N_forecast_intervals, 1);
  
  if (N_forecast_intervals > N_intervals_alloc) {
    
    N_intervals_alloc = N_forecast_intervals;
    
    if (Interval_contrib_time == NULL) {
      Interval_contrib_time = (double *) umalloc
	((ui32) ((N_intervals_alloc)* sizeof(double)));
    } else {
      Interval_contrib_time = (double *) urealloc
	((char *) Interval_contrib_time,
	 (ui32) ((N_intervals_alloc)* sizeof(double)));
    }
    
    if (Interval_lead_time == NULL) {
      Interval_lead_time = (double *) umalloc
	((ui32) ((N_intervals_alloc)* sizeof(double)));
    } else {
      Interval_lead_time = (double *) urealloc
	((char *) Interval_lead_time,
	 (ui32) ((N_intervals_alloc)* sizeof(double)));
    }
    
  } /* if (N_forecast_intervals < N_intervals_alloc) */

  for (i_int = 0; i_int < N_forecast_intervals; i_int++) {
    Interval_contrib_time[i_int] = Glob->params.scan_interval / 3600.0;
    Interval_lead_time[i_int] =
      (((double) i_int + 1.0) * Glob->params.scan_interval) / 3600.0;
  } /* i_int */

  Interval_contrib_time[N_forecast_intervals - 1] =
    (lead_time -
     ((double) N_forecast_intervals - 1.0) *
     Glob->params.scan_interval) / 3600.0;

  if (Glob->params.debug >= DEBUG_EXTRA) {

    fprintf(stderr, "N_forecast_intervals : %ld\n",
	    (long) N_forecast_intervals);
    
    for (i_int = 0; i_int < N_forecast_intervals; i_int++)
      fprintf(stderr,
	      "Forecast interval #, lead_time, contrib_time : %ld, %g, %g\n",
	      (long) i_int,
	      Interval_lead_time[i_int],
	      Interval_contrib_time[i_int]);
    
  } /* if (Glob->params.debug >= DEBUG_EXTRA) */

}

