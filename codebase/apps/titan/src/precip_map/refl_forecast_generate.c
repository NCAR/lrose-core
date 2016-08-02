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

/****************************************************************************
 * refl_forecast_generate.c
 *
 * generate the reflectivity forecast
 *
 * Returns time of last file written, -1 on error
 *
 * August 1994
 *
 ****************************************************************************/

#include "precip_map.h"

/*
 * prototypes
 */

static ui08 *thresholded_forecast(storm_file_handle_t *s_handle,
				  track_file_handle_t *t_handle,
				  vol_file_handle_t *v_handle,
				  mdv_grid_t *grid,
				  double lead_time_hr,
				  si32 scan_num);

static ui08 *unthresholded_forecast(storm_file_handle_t *s_handle,
				    track_file_handle_t *t_handle,
				    vol_file_handle_t *v_handle,
				    cart_params_t *cart,
				    mdv_grid_t *grid,
				    double lead_time_hr,
				    si32 scan_num);

/*
 * main subroutine
 */

time_t refl_forecast_generate(storm_file_handle_t *s_handle,
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

  time_t time_this_scan, time_latest_scan;
  time_t scan_time_diff;
  time_t forecast_time;

  date_time_t *stime, ftime;

  cart_params_t *cart;
  mdv_grid_t grid;

  lock_and_read_headers(s_handle, t_handle);
  PMU_auto_register("In refl_forecast_generate");

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
      ((si32) floor((double) forecast_time /
		    Glob->params.rounding_interval + 0.5) *
       Glob->params.rounding_interval);
  }
  lead_time_used = forecast_time - time_this_scan;
  lead_time_hr = lead_time_used / 3600.0;

  /*
   * get path name for map file - forecast time is scan time
   * plus the forecast_duration as relevant
   */
  
  if (Glob->params.file_time_stamp == GENERATE_TIME) {
    ftime.unix_time = stime->unix_time;
  } else {
    ftime.unix_time = forecast_time;
  }
     
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
	  (long) stime->year, (long) stime->month, (long) stime->day,
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

  if (RfReadVolume(v_handle, "refl_forecast_generate")) {
    fprintf(stderr, "ERROR - %s:refl_forecast_generate\n",
	    Glob->prog_name);
    fprintf(stderr, "Could not read in Mdv radar volume\n");
    perror(rdata_file_path);
    unlock_files(s_handle, t_handle);
    return (-1);
  }
    
  /*
   * check geometry
   */
  
  cart = &v_handle->vol_params->cart;
  if (check_cart_geom(cart, &s_handle->scan->grid)) {
    fprintf(stderr, "ERROR - %s:refl_forecast_generate\n",
	    Glob->prog_name);
    fprintf(stderr, "Storm file cart geometry does not match\n");
    unlock_files(s_handle, t_handle);
    return(-1);
  }
  cart_params_to_mdv_grid(cart, &grid, s_handle->scan->grid.proj_type);
  
  /*
   * read in storm file scan
   */
    
  if (RfReadStormScan(s_handle, scan_num, "refl_forecast_generate")) {
    unlock_files(s_handle, t_handle);
    return (-1);
  }
    
  /*
   * compute thresholded forecast
   */

  if (Glob->params.thresholded_forecast) {
    
    forecast = thresholded_forecast(s_handle, t_handle, v_handle, &grid,
				    lead_time_hr, scan_num);
      
  } else {

    forecast = unthresholded_forecast(s_handle, t_handle, v_handle,
				      &v_handle->vol_params->cart,
				      &grid,
				      lead_time_hr, scan_num);
      
  } /* if (Glob->params.thresholded_forecast) */
    
  if (forecast != NULL) {

    /*
     * initialize map index
     */
      
    init_map_index(map_v_handle, v_handle, &ftime);

    /*
     * set fields
     */
      
    map_v_handle->field_plane[0][0] = forecast;

    STRcopy(map_v_handle->vol_params->cart.unitsx,
	    v_handle->vol_params->cart.unitsx, R_LABEL_LEN);
    STRcopy(map_v_handle->vol_params->cart.unitsy,
	    v_handle->vol_params->cart.unitsy, R_LABEL_LEN);
    
    STRcopy(map_v_handle->field_params[0]->units,
	    v_handle->field_params[Glob->params.dbz_field]->units,
	    R_LABEL_LEN);
    
    /*
     * write the map file
     */
      
    sprintf(map_v_handle->vol_params->note,
	    "%s\n%s%g\n",
	    "Reflectivity forecast",
	    "Refl threshold: ", s_handle->header->params.low_dbz_threshold);
      
    if (Glob->params.file_time_stamp == GENERATE_TIME) {
      sprintf(map_v_handle->field_params[0]->name,
	      "%g hr forecast ahead of time", lead_time_hr);
    } else {
      sprintf(map_v_handle->field_params[0]->name,
	      "%g hr forecast valid at time", lead_time_hr);
    }
      
    RfWriteDobson(map_v_handle, FALSE, Glob->params.debug,
		  Glob->params.map_dir,
		  Glob->params.output_file_ext,
		  Glob->prog_name,
		  "refl_forecast_generate");
      
    /*
     * write velocity grid files for debugging if required
     */
    
    if (Glob->params.write_motion_grid_files &&
	!Glob->params.thresholded_forecast) {
      init_motion_v_handle(map_v_handle);
      write_motion_grid_file();
    }
    
  } /* if (forecast != NULL) */
    
  unlock_files(s_handle, t_handle);
  return (ftime.unix_time);

}

static ui08 *thresholded_forecast(storm_file_handle_t *s_handle,
				  track_file_handle_t *t_handle,
				  vol_file_handle_t *v_handle,
				  mdv_grid_t *grid,
				  double lead_time_hr,
				  si32 scan_num)

{

  static int first_call = TRUE;
  static ui08 *Forecast_grid;
  static ui08 *Comp_grid;
  static int Npoints;
  
  ui08 *dbz_plane;
  ui08 *comp, *layer;
  ui08 comp_val;

  int base, top;

  si32 ix, jx, iy, jy;
  si32 ientry, irun;
  si32 n_entries;
  si32 grid_index;

  si32 cart_min_ix, cart_min_iy;
  si32 cart_max_ix, cart_max_iy;

  double dx_dt, dy_dt;
  double darea_dt;
  double current_area, forecast_area;
  double area_ratio, length_ratio;
  double grid_x, grid_y;
  double current_x, current_y;
  double forecast_x, forecast_y;
  double delta_area;
  double time_to_zero_growth;

  double x, y;
  double bounding_minx, bounding_miny;
  double bounding_maxx, bounding_maxy;
  double xratio, yratio;
  double forecast_ix, forecast_iy;

  mdv_grid_t fcast_cart;
  storm_file_global_props_t *gprops;
  storm_file_run_t *run;
  track_file_entry_t *entry;
  track_file_forecast_props_t *fprops;

  /*
   * allocate grids
   */
  
  if (first_call) {
    Npoints = grid->ny * grid->nx;
    Forecast_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    Comp_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    first_call = FALSE;
  }
  
  /*
   * zero out forecast grid
   */
  
  memset((void *) Forecast_grid, 0, Npoints * sizeof(ui08));

  /*
   * read in track file scan entries
   */
  
  if (RfReadTrackScanEntries(t_handle, scan_num, "refl_forecast_generate"))
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
      
    if (RfReadStormProps(s_handle, entry->storm_num,
			 "refl_forecast_generate"))
      return (NULL);
	
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
      
    if (Glob->params.debug >= DEBUG_EXTRA) {

      fprintf(stderr, "current_x, current_y, area : %g, %g, %g\n",
	      current_x, current_y, current_area);

      fprintf(stderr, "grid_x, grid_y : %g, %g\n",
	      grid_x, grid_y);

      fprintf(stderr,
	      "dx_dt, dy_dt, darea_dt, : %g, %g, %g\n",
	      dx_dt, dy_dt, darea_dt);

      run = s_handle->runs;
      for (irun = 0; irun < gprops->n_runs; irun++, run++) {
	fprintf(stderr, "ix, iy, iz, n : %d, %d, %d, %d\n",
		run->ix, run->iy, run->iz, run->n);
      } /* irun */

    } /* if (Glob->params.debug >= DEBUG_EXTRA) */

    /*
     * compute the forecast storm position and area
     * for the forecast scan time
     */
	
    forecast_x = current_x + dx_dt * lead_time_hr;
    forecast_y = current_y + dy_dt * lead_time_hr;

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
     * grid and remapping onto a refl map
     */
      
    area_ratio = forecast_area / current_area;
    length_ratio = sqrt(area_ratio);
      
    fcast_cart.dx = grid->dx * length_ratio;
    fcast_cart.dy = grid->dy * length_ratio;
	
    fcast_cart.minx = forecast_x - grid_x * fcast_cart.dx;
    fcast_cart.miny = forecast_y - grid_y * fcast_cart.dy;

    if (Glob->params.debug >= DEBUG_EXTRA) {
	
      fprintf(stderr,
	      "forecast_x, forecast_y, forecast_area : %g, %g, %g\n",
	      forecast_x, forecast_y, forecast_area);
	
      fprintf(stderr,
	      "area_ratio, length_ratio : %g, %g\n",
	      area_ratio, length_ratio);

      fprintf(stderr,
	      "fcast_cart.minx, fcast_cart.miny : %g, %g\n",
	      fcast_cart.minx, fcast_cart.miny);

      fprintf(stderr,
	      "fcast_cart.dx, fcast_cart.dy : %g, %g\n",
	      fcast_cart.dx, fcast_cart.dy);

    } /* if (Glob->params.debug >= DEBUG_EXTRA) */

    /*
     * zero out comp grid
     */
    
    memset((void *) Comp_grid, 0, Npoints * sizeof(ui08));

    /*
     * load up comp grid
     */

    base = mdv_comp_base(grid);
    top = mdv_comp_top(grid);
    
    run = s_handle->runs;
    for (irun = 0; irun < gprops->n_runs; irun++, run++) {
      
      dbz_plane = v_handle->field_plane[Glob->params.dbz_field][run->iz];
      if (run->iz < base || run->iz > top) {
	continue;
      }

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
     * storm movement and change in size
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
	
	if (Forecast_grid[grid_index] < comp_val) {
	  Forecast_grid[grid_index] = comp_val;
	}
	
      } /* ix */

    } /* iy */

  } /* ientry */

  return (Forecast_grid);
  
}

static ui08 *unthresholded_forecast(storm_file_handle_t *s_handle,
				    track_file_handle_t *t_handle,
				    vol_file_handle_t *v_handle,
				    cart_params_t *cart,
				    mdv_grid_t *grid,
				    double lead_time_hr,
				    si32 scan_num)

{

  static int first_call = TRUE;
  static ui08 *Comp_grid;
  static ui08 *Forecast_grid;
  static si32 Npoints;
  
  field_params_t *dbz_fparams;

  /*
   * allocate grid
   */
  
  if (first_call) {
    Npoints = cart->ny * cart->nx;
    Comp_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    Forecast_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    first_call = FALSE;
  }
  
  /*
   * create composite grid
   */
  
  create_composite(Npoints, cart_comp_base(cart), cart_comp_top(cart),
		   v_handle->field_plane[Glob->params.dbz_field],
		   Comp_grid);

  if (load_motion_grid(s_handle, t_handle, cart, scan_num, lead_time_hr)) {
    return (NULL);
  }

  dbz_fparams = v_handle->field_params[Glob->params.dbz_field];
  
  load_refl_motion_forecast(Comp_grid, Forecast_grid, dbz_fparams);

  return (Forecast_grid);
  
}

#ifdef OBSOLETE

static ui08 *simple_unthresholded_forecast(storm_file_handle_t *s_handle,
					   track_file_handle_t *t_handle,
					   vol_file_handle_t *v_handle,
					   cart_params_t *cart,
					   mdv_grid_t *grid,
					   double lead_time_hr,
					   si32 scan_num)
     
{

  static int first_call = TRUE;
  static ui08 *Forecast_grid;
  static si32 Npoints;
  
  ui08 *comp, *layer;

  int i, iz;

  double mean_dx_dt;
  double mean_dy_dt;
  double grid_delta_x;
  double grid_delta_y;
  
  /*
   * allocate grid
   */
  
  if (first_call) {
    Npoints = cart->ny * cart->nx;
    Forecast_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    first_call = FALSE;
  }
  
  create_composite(Npoints, cart_comp_base(cart), cart_comp_top(cart),
		   v_handle->field_plane[Glob->params.dbz_field],
		   Forecast_grid);

  if (get_mean_motion(s_handle, t_handle,
		      scan_num, &mean_dx_dt, &mean_dy_dt)) {
    return (NULL);
  }
     
  grid_delta_x = (mean_dx_dt * lead_time_hr) / grid->dx;
  grid_delta_y = (mean_dy_dt * lead_time_hr) / grid->dy;
  
  /*
   * change the reference point for the cart grid to move the
   * image at the mean speed and dirn
   */

  cart->minx += (si32) (grid_delta_x * cart->km_scalex + 0.5);
  cart->miny += (si32) (grid_delta_y * cart->km_scaley + 0.5);

  return (Forecast_grid);
  
}

#endif
