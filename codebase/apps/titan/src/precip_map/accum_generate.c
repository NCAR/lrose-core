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
 * file scope
 */

static ui08 *Accum_grid = NULL;
static ui08 *Dbz_grid = NULL;
static int Cart_initialized = FALSE;
static double *Precip_grid = NULL;
static si32 Npoints;
static cart_params_t Cart;
static mdv_grid_t Grid;

static void init_accum_array(void);

#ifdef NOTNOW
static int init_accum_data(char *map_file_dir,
			   date_time_t *scan_init_time);
#endif

/**************************************************************************
 * accum_generate.c
 *
 * generate precip accumulation map
 *
 * returns the time of the file written, -1 on error
 *
 * June 1995
 *
 **************************************************************************/

time_t accum_generate(storm_file_handle_t *s_handle,
		      track_file_handle_t *t_handle,
		      vol_file_handle_t *v_handle,
		      vol_file_handle_t *map_v_handle,
		      date_time_t *scan_times,
		      si32 scan_num)
     
{

  static time_t *scan_start = NULL;
  static time_t *scan_end = NULL;

  ui08 *dbz;

  char rdata_file_path[MAX_PATH_LEN];
  char map_file_dir[MAX_PATH_LEN];
  char map_file_path[MAX_PATH_LEN];

  int map_initialized;
  int zero_time_to_ground;
  int i, ix, iy;
  int delta_ix, delta_iy;
  int refl_start_ix, refl_end_ix;
  int refl_start_iy, refl_end_iy;
  int missing_file_found = FALSE;

#ifdef NOTNOW
  int restart_from_beginning = FALSE;
#endif
  
  si32 iscan, jscan, kscan;
  si32 start_jscan, n_scans;
  si32 n_accum_scans;
  
  double precip_lookup[256];
  double contrib_secs;
  double contribs_so_far;
  double time_to_ground_hr;
  double contrib_hr;
  double mean_dx_dt, mean_dy_dt;
  double delta_x, delta_y;
  double coeff, expon;
  double *precip;
  
  time_t accum_duration, accum_start_time, accum_end_time;
  time_t time_this_scan, time_latest_scan;
  time_t time_start_of_first_scan;
  time_t scan_time_diff;

  date_time_t *stime, *this_time, ftime;
  field_params_t *dbz_fparams, *precip_fparams;

  n_scans = s_handle->header->n_scans;

  if (n_scans < 2) {
    return (-1);
  }

  /*
   * set up scan start and end time arrays
   */

  if (scan_start == NULL) {
    scan_start = (time_t *) umalloc (n_scans * sizeof(time_t));
  } else {
    scan_start = (time_t *) urealloc (scan_start,
				      n_scans * sizeof(time_t));
  }

  if (scan_end == NULL) {
    scan_end = (time_t *) umalloc (n_scans * sizeof(time_t));
  } else {
    scan_end = (time_t *) urealloc (scan_end,
				    n_scans * sizeof(time_t));
  }

  for (iscan = 0; iscan < n_scans; iscan++) {

    if (iscan == 0) {
      scan_start[iscan] = scan_times[0].unix_time -
	(scan_times[1].unix_time - scan_times[0].unix_time) / 2.0;
      scan_end[iscan] =
	(scan_times[0].unix_time + scan_times[1].unix_time) / 2.0;
    } else if (iscan == n_scans - 1) {
      scan_start[iscan] =
	(scan_times[n_scans - 1].unix_time +
	 scan_times[n_scans - 2].unix_time) / 2.0;
      scan_end[iscan] = scan_times[n_scans - 1].unix_time +
	(scan_times[n_scans - 1].unix_time -
	 scan_times[n_scans - 2].unix_time) / 2.0;
    } else {
      scan_start[iscan] =
	(scan_times[iscan].unix_time + scan_times[iscan - 1].unix_time) / 2.0;
      scan_end[iscan] =
	(scan_times[iscan].unix_time + scan_times[iscan + 1].unix_time) / 2.0;
    }

    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr, "Scan: %4d, start %s, end %s, delta %d\n",
	      (int) iscan,
	      utimstr(scan_start[iscan]), utimstr(scan_end[iscan]),
	      (int) (scan_end[iscan] - scan_start[iscan]));
    }

  } /* iscan */
  
  time_start_of_first_scan = scan_start[0];
  time_latest_scan = scan_times[s_handle->header->n_scans - 1].unix_time;

  stime = scan_times + scan_num;

  PMU_auto_register("In accum_generate");

  ftime.unix_time = stime->unix_time + Glob->params.time_to_ground;
  uconvert_from_utime(&ftime);
    
  time_this_scan = stime->unix_time;
  scan_time_diff = time_latest_scan - time_this_scan;
  map_initialized = FALSE;
    
  /*
   * for non-zero time to ground, get mean storm motion
   */
  
  if (Glob->params.time_to_ground < 0.1) {
    zero_time_to_ground = TRUE;
  } else {
    zero_time_to_ground = FALSE;
  }
      
  if (!zero_time_to_ground) {
    time_to_ground_hr = Glob->params.time_to_ground / 3600.0;
    if (get_mean_motion(s_handle, t_handle,
			scan_num, &mean_dx_dt, &mean_dy_dt)) {
      return (-1);
    }
  }
    
  /*
   * get path name for map file
   */

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
    return (-1);
  }
    
  missing_file_found = TRUE;

  /*
   * determine accum_start_time, accum_end_time and
   * n_accum_scans
   */
  
  accum_end_time = time_this_scan + Glob->params.scan_interval / 2.0;

  if (Glob->params.map_type == ACCUM_FROM_START) {

    n_accum_scans = 1;
    accum_start_time = time_start_of_first_scan;

  } else {

    accum_duration = Glob->params.accum_duration;
    accum_start_time = accum_end_time - accum_duration;
      
    if (accum_start_time < time_start_of_first_scan) {
	
      accum_start_time = time_start_of_first_scan;
      start_jscan = 0;
	
    } else {
	
      accum_start_time = accum_end_time - accum_duration;
      this_time = stime;
      start_jscan = 0;
      for (jscan = scan_num; jscan >= 0; jscan--, this_time--) {
	if (this_time->unix_time < accum_start_time) {
	  start_jscan = jscan + 1;
	  break;
	}
      } /* jscan */
	
    } /* if (accum_start_time < time_start_of_first_scan) */
      
    n_accum_scans = scan_num - start_jscan + 1;

  } /* if (Glob->params.map_type == ACCUM_FROM_START) */

  /*
   * adjust accum_duration
   */

  accum_duration = accum_end_time - accum_start_time;

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "\nScan %ld, time %s\n", (long) scan_num,
	    utimestr(stime));
    fprintf(stderr, "    start_jscan = %d\n", (int) start_jscan);
    fprintf(stderr, "    Accum_start_time: %s\n",
	    utimstr(accum_start_time));
    fprintf(stderr, "    Accum_end_time: %s\n",
	    utimstr(accum_end_time));
    fprintf(stderr, "    n_accum_scans : %ld\n",
	    (long) n_accum_scans);
    fprintf(stderr, "Accum_duration: %ld\n", (long) accum_duration);
    fprintf(stderr, "Map path: %s\n", map_file_path);
  } /* if (Glob->params.debug >= DEBUG_NORM) */
      
  /*
   * loop through the scans for the accumulation
   */
  
  this_time = stime;
  contribs_so_far = 0.0;
  kscan = scan_num;

  for (jscan = n_accum_scans - 1; jscan >= 0;
       jscan--, kscan--, this_time--) {
      
    /*
     * compute the radar data file path
     */
      
    sprintf(rdata_file_path,
	    "%s%s%.4ld%.2ld%.2ld%s%.2ld%.2ld%.2ld.%s",
	    Glob->params.rdata_dir, PATH_DELIM,
	    (long) this_time->year, (long) this_time->month,
	    (long) this_time->day,
	    PATH_DELIM,
	    (long) this_time->hour, (long) this_time->min,
	    (long) this_time->sec,
	    Glob->params.output_file_ext);
      
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "\n  Scan %ld, time %s\n", (long) kscan,
	      utimestr(this_time));
      fprintf(stderr, "  Rdata path: %s\n", rdata_file_path);
    } /* if (Glob->params.debug >= DEBUG_NORM) */
    
    /*
     * read in radar volume
     */
    
    v_handle->vol_file_path = rdata_file_path;
    
    if (RfReadVolume(v_handle, "accum_generate")) {
      fprintf(stderr, "ERROR - %s:accum_generate\n", Glob->prog_name);
      fprintf(stderr, "Could not read in Mdv radar volume\n");
      perror(rdata_file_path);
      return (-1);
    }
    
    /*
     * compute the contribution of this scan in seconds
     */
    
    if (Glob->params.map_type == ACCUM_FROM_START) {
      contrib_secs = scan_end[kscan] - scan_start[kscan];
    } else {
      if (kscan > 0) {
	contrib_secs = scan_end[kscan] - scan_start[kscan];
	contribs_so_far += contrib_secs;
      } else {
	contrib_secs = accum_duration - contribs_so_far;
      }
    }
      
    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr, "      Start time: %s\n",
	      utimstr(Rfrtime2utime(&v_handle->vol_params->start_time)));
      fprintf(stderr, "      End time: %s\n",
	      utimstr(Rfrtime2utime(&v_handle->vol_params->end_time)));
      fprintf(stderr, "      Scan %ld, contrib_time(mins) %g\n",
	      (long) kscan, contrib_secs / 60.0);
    } /* if (Glob->params.debug >= DEBUG_EXTRA) */

    if (contrib_secs < 0) {
      contrib_secs = 0;
    }

    if (!Cart_initialized) {

      /*
       * initialize
       */
	
      Cart = v_handle->vol_params->cart;
      Npoints = Cart.nx * Cart.ny;
      cart_params_to_mdv_grid(&Cart, &Grid, s_handle->scan->grid.proj_type);
	
      /*
       * allocate precip grids
       */
	
      Precip_grid = (double *) ucalloc
	((ui32) Npoints, (int) sizeof(double));
	
      Accum_grid = (ui08 *) ucalloc
	((ui32) Npoints, sizeof(ui08));

      Dbz_grid = (ui08 *) ucalloc
	((ui32) Npoints, sizeof(ui08));
	
      Cart_initialized = TRUE;
	
#ifdef NOTNOW
      /*
       * If we are not on the first scan (scan 0), then we need to
       * initialize the accumulated precip using the previously
       * created file.
       */
	
      if (scan_num > 0) {
	if (!init_accum_data(map_file_dir,
			     &(scan_times[scan_num-1]))) {
	  /*
	   * There was an error reading in the last accum file.
	   * Now, start over from the beginning regenerating the
	   * accum files as you go.
	   */

	  init_accum_array();
	  restart_from_beginning = TRUE;
	  scan_num = -1;
	  continue;
	    
	} /* endif - error in init_accum_data */
      } /* endif - scan_num > 0 */
#endif
	
    } else {
	
      /*
       * check cartesian params for consistency
       */
      
      if (memcmp((void *) &Cart,
		 (void *) &v_handle->vol_params->cart,
		 sizeof(cart_params_t))) {
	  
	fprintf(stderr, "ERROR - %s:accum_generate\n",
		Glob->prog_name);
	fprintf(stderr, "Cart params differ\n");
	return (-1);
	  
      }
	
    } /* if (!Cart_initialized) */

    if (!map_initialized) {
	
      init_map_index(map_v_handle, v_handle, &ftime);
	
      /*
       * In ACCUM_PERIOD case, zero out Precip_grid array.
       * In ACCUM_FROM_START case, this is allowed to accumulate.
       */
	
      if (Glob->params.map_type == ACCUM_PERIOD) {
	init_accum_array();
      }
      
      map_initialized = TRUE;
	
    } /* if (!map_initialized) */

    /*
     * create composite grid
     */
    
    create_composite(Npoints, cart_comp_base(&Cart), cart_comp_top(&Cart),
		     v_handle->field_plane[Glob->params.dbz_field],
		     Dbz_grid);

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
    contrib_hr = contrib_secs / 3600.0;
    
    /*
     * increment precip array
     */
    
    if (zero_time_to_ground) {
      
      precip = Precip_grid;
      dbz = Dbz_grid;
	
      for (i = 0; i < Npoints; i++, precip++, dbz++) {
	*precip += precip_lookup[*dbz] * contrib_hr;
      } /* i */

    } else {
	
      delta_x =
	((mean_dx_dt * time_to_ground_hr) / Grid.dx);
      delta_y =
	((mean_dy_dt * time_to_ground_hr) / Grid.dy);
    
      delta_ix = (long) floor(delta_x + 0.5);
      delta_iy = (long) floor(delta_y + 0.5);

      /*
       * move the refl grid by the forecast amount in each dirn,
       * and map it onto the precip grid
       */

      if (delta_ix < 0) {
	refl_start_ix = -delta_ix;
	refl_end_ix = Grid.nx - 1;
      } else {
	refl_start_ix = 0;
	refl_end_ix = Grid.nx - 1 - delta_ix;
      }
	
      if (delta_iy < 0) {
	refl_start_iy = -delta_iy;
	refl_end_iy = Grid.ny - 1;
      } else {
	refl_start_iy = 0;
	refl_end_iy = Grid.ny - 1 - delta_iy;
      }

      for (iy = refl_start_iy; iy <= refl_end_iy; iy++) {
	  
	dbz = Dbz_grid + (iy * Grid.nx) + refl_start_ix;
	precip = (Precip_grid + ((iy + delta_iy) * Grid.nx) +
		  (refl_start_ix + delta_ix));
	  
	for (ix = refl_start_ix; ix <= refl_end_ix;
	     ix++, dbz++, precip++) {
	  *precip += precip_lookup[*dbz] * contrib_hr;
	} /* ix */
	
      } /* iy */
      
    } /* if (zero_time_to_ground) */
    
  } /* jscan */
  
  /*
   * scale the precip array and copy into the first field
   * of the volume index
   */
  
  precip_fparams = map_v_handle->field_params[0];
  
  scale_data(Precip_grid,
	     Accum_grid,
	     Npoints,
	     precip_fparams->factor,
	     &precip_fparams->scale,
	     &precip_fparams->bias);
  
  /*
   * write the map file
   */
  
  sprintf(map_v_handle->vol_params->note,
	  "%s\n%s%ld\n%s%g\n%s%g\n",
	  "Precip accumulation",
	  "Accum duration : ", (long) accum_duration,
	  "Z-R coeff : ", coeff,
	  "Z-R expon : ", expon);
  
  sprintf(map_v_handle->field_params[0]->name,
	  "%.2f hr precip accum to time",
	  accum_duration / 3600.0);
  
  map_v_handle->field_plane[0][0] = Accum_grid;
  
  RfWriteDobson(map_v_handle, FALSE, Glob->params.debug,
		Glob->params.map_dir,
		Glob->params.output_file_ext,
		Glob->prog_name,
		"forecast_generate");
    
  return (ftime.unix_time);

}

/********************
 * init_accum_array()
 */

static void init_accum_array(void)

{
  memset((void *) Precip_grid, 0, (Npoints * sizeof(double)));
  return;
}

#ifdef NOTNOW
/********************
 * init_accum_data()
 */

static int init_accum_data(char *map_file_dir,
			   date_time_t *scan_init_time)
     
{
  char map_file_path[MAX_PATH_LEN];
  vol_file_handle_t v_handle;
  
  /*
   * get the name for the file to use in initializing Accum_grid.
   */

  sprintf(map_file_path,
	  "%s%s%.2ld%.2ld%.2ld.%s",
	  map_file_dir, PATH_DELIM,
	  (long) scan_init_time->hour, (long) scan_init_time->min,
	  (long) scan_init_time->sec,
	  Glob->params.output_file_ext);
  
  /*
   * Read in the latest precip accum file.
   */

  v_handle.vol_file_path = map_file_path;
  v_handle.index_initialized = FALSE;
  
  if (RfReadVolume(&v_handle, "init_accum_data") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:init_accum_data\n", Glob->prog_name);
    fprintf(stderr, "Could not read in previous precip accum file\n");
    perror(map_file_path);
    
    return(FALSE);
  }
  
  /*
   * Initialize the Accum_grid with the precip accum data.
   */

  /*
   * Free the space used by the v_handle information.
   */

  if (RfFreeVolArrays(&v_handle, "init_accum_data") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:init_accum_data\n", Glob->prog_name);
    fprintf(stderr, "Could not free the v_handle memory\n");
  }
  
  return(TRUE);
}
#endif
