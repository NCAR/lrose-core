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
 * load_cont_data.c
 *
 * Load contingency data
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 ****************************************************************************/

#include "verify_day.h"

static void update_cont_data(storm_file_handle_t *s_handle,
			     track_file_handle_t *t_handle,
			     ucont_table_t *cont_table,
			     ui08 **verify_grid,
			     ui08 **forecast_grid,
			     time_t scan_time,
			     double delta_hr);

static void print_grids(ui08 **verify_grid,
			ui08 **forecast_grid);

void load_cont_data(storm_file_handle_t *s_handle,
		    track_file_handle_t *t_handle,
		    time_t *scan_time,
		    ucont_table_t *cont_table)
     
{
  
  static int first_call = TRUE;
  
  static ui08 **forecast_grid, **verify_grid;
  
  int scan_found;
  
  si32 iscan, jscan;
  si32 scan_num;

  double search_utime, delta_hr;
  double min_utime, max_utime;

  /*
   * allocate contingency data and grids
   */

  if (first_call == TRUE) {

    forecast_grid = (ui08 **) ucalloc2
      ((ui32) Glob->ny, (ui32) Glob->nx, (ui32) sizeof(ui08));

    verify_grid = (ui08 **) ucalloc2
      ((ui32) Glob->ny, (ui32) Glob->nx, (ui32) sizeof(ui08));

    first_call = FALSE;

  } /* if (first_call == TRUE) */

  /*
   * loop through the scans in the storm file
   */

  for (iscan = 0; iscan < s_handle->header->n_scans; iscan++) {

    if (Glob->debug > TRUE)
      fprintf(stderr, "Analysing scan number %ld\n", (long) iscan);

    /*
     * read the verification file if it exists
     */
    
    if (read_verification_file(s_handle->scan,
			       scan_time[iscan],
			       verify_grid) == R_SUCCESS) {

      /*
       * compute the approx time of the scan to be searched for -
       * units are days
       */

      search_utime = ((double) scan_time[iscan] -
		      (double) Glob->forecast_lead_time);

      min_utime = search_utime - (double) Glob->forecast_lead_time_margin;
      max_utime = search_utime + (double) Glob->forecast_lead_time_margin;

      /*
       * go back and try to find scan which is at this approx time
       */
      
      scan_found = FALSE;
      
      for (jscan = iscan; jscan >= 0; jscan--) {
	
	if (scan_time[jscan] >= min_utime &&
	    scan_time[jscan] <= max_utime) {

	  scan_num = jscan;
	  scan_found = TRUE;
	  break;
	  
	} /* if */
	
      } /* jscan */
      
      if (scan_found == TRUE) {
	
	if (Glob->debug)
	  fprintf(stderr, "Forecast scan_num, time = %ld, %s\n",
		  (long) scan_num, utimstr(scan_time[jscan]));
	
	/*
	 * compile the forecast grid and update the counters from
	 * which the contingency data will be computed
	 */
	
	delta_hr = ((double) scan_time[iscan] -
		    (double) scan_time[scan_num]) / 3600.0;

	update_cont_data(s_handle, t_handle,
			 cont_table,
			 verify_grid,
			 forecast_grid,
			 scan_time[scan_num],
			 delta_hr);
	
	if (Glob->debug)
	  print_grids(verify_grid, forecast_grid);
	
      } /* if (scan_found == TRUE) */

    } /* if (read_verification_file(s_handle, &v_handle, iscan) == R_SUCCESS) */

  } /* iscan */

  return;
  
}

static void update_cont_data(storm_file_handle_t *s_handle,
			     track_file_handle_t *t_handle,
			     ucont_table_t *cont_table,
			     ui08 **verify_grid,
			     ui08 **forecast_grid,
			     time_t scan_time,
			     double delta_hr)

{

  ui08 *vptr, *fptr;

  int entry_found;
  si32 isimple, ientry;
  si32 ix, iy;

  si32 truth, forecast;

  double f_proj_area;
  double f_centroid_x;
  double f_centroid_y;
  double f_ellipse_scale;
  double f_major_radius;
  double f_minor_radius;
  double f_orientation ;
  double forecast_dx, forecast_dy;

  storm_file_global_props_t *gprops;
  track_file_entry_t *entry;
  track_file_forecast_props_t *fprops;

  /*
   * zero out the forecast_grid
   */

  memset ((void *) *forecast_grid,
          (int) 0, (size_t) (Glob->nx * Glob->ny * sizeof(ui08)));

  /*
   * loop through all of the simple tracks
   */

  for (isimple = 0;
       isimple < t_handle->header->n_simple_tracks; isimple++) {

    if (scan_time >= t_handle->track_utime[isimple].start_simple &&
	scan_time <= t_handle->track_utime[isimple].end_simple) {

      /*
       * this simple track was active at the scan time, so process it
       */

      if(RfRewindSimpleTrack(t_handle, isimple,
			    "update_contingency_data") != R_SUCCESS)
	tidy_and_exit(1);

      /*
       * search for entry at this time
       */

      entry_found = FALSE;

      for (ientry = 0;
	   ientry < t_handle->simple_params->duration_in_scans; ientry++) {

	if (RfReadTrackEntry(t_handle, "update_contingency_data") !=
	    R_SUCCESS)
	  tidy_and_exit(1);

	if (t_handle->entry->time == scan_time) {

	  /*
	   * this entry time matches the scan time, so break now
	   */

	  entry_found = TRUE;
	  break;

	} /* if (memcmp .... */

      } /* ientry */

      /*
       * if no entry found, print error and quit
       */

      if (entry_found == FALSE) {

	fprintf(stderr, "ERROR - %s:update_contingency_data\n",
		Glob->prog_name);
 	fprintf(stderr, "Time-matched entry not found.\n");
	fprintf(stderr, "Simple track num %ld\n", (long) isimple);
	fprintf(stderr, "Search time  %s\n", utimstr(scan_time));
	tidy_and_exit(1);

      }

      entry = t_handle->entry;
      fprops = &entry->dval_dt;

      if (Glob->mode == ELLIPSE_MODE) {

	/*
	 * analyse this track entry if there is a long enough
	 * history associated with it
	 */
      
	if (entry->history_in_scans >=
	    Glob->min_valid_history) {

	  /*
	   * read in storm scan data
	   */
	
	  if (RfReadStormScan(s_handle, entry->scan_num,
			      "update_contingency_data") != R_SUCCESS)
	    tidy_and_exit(1);
	
	  gprops = s_handle->gprops + entry->storm_num;
	  
	  /*
	   * compute forecast
	   */
	
	  f_proj_area =
	    gprops->proj_area + fprops->proj_area * delta_hr;
	  
	  if (f_proj_area < 1.0)
	    f_proj_area = 1.0;
	  
	  f_centroid_x =
	    gprops->proj_area_centroid_x +
	      fprops->proj_area_centroid_x * delta_hr;

	  f_centroid_y =
	    gprops->proj_area_centroid_y +
	      fprops->proj_area_centroid_y * delta_hr;
	  
	  f_ellipse_scale = sqrt(f_proj_area / gprops->proj_area);
	
	  f_major_radius =
	    gprops->proj_area_major_radius * f_ellipse_scale;
	
	  f_minor_radius =
	    gprops->proj_area_minor_radius *  f_ellipse_scale;
	
	  f_orientation = gprops->proj_area_orientation;
	  
	  /*
	   * update the forecast grid
	   */
	
	  update_ellipse_grid(f_centroid_x, f_centroid_y,
			      f_major_radius, f_minor_radius,
			      f_orientation,
			      forecast_grid);
	
	} /* if (t_handle->entry->history_in_scans ... */

      } else if (Glob->mode == RUNS_MODE) {

	/*
	 * read in storm data
	 */
	
	if (RfReadStormScan(s_handle, entry->scan_num,
			    "update_contingency_data") != R_SUCCESS)
	  tidy_and_exit(-1);
	
	gprops = s_handle->gprops + entry->storm_num;

	if (RfReadStormProps(s_handle, entry->storm_num,
			     "update_contingency_data") != R_SUCCESS)
	  tidy_and_exit(-1);

	/*
	 *  compute dx and dy from forecast
	 */

	if (entry->history_in_scans >=
	    Glob->min_valid_history) {
	
	  forecast_dx = fprops->proj_area_centroid_x * delta_hr;
	  forecast_dy = fprops->proj_area_centroid_y * delta_hr;

	} else {

	  forecast_dx = 0.0;
	  forecast_dy = 0.0;

	}

	update_runs_grid(s_handle, gprops->n_runs,
			 forecast_dx, forecast_dy,
			 forecast_grid);

      } /* if (Glob->mode ... */
      
    } /* if (scan_time >= .... */
    
  } /* isimple */
  
  /*
   * update the contingency data counters
   */
  
  /*
   * set pointer to start of grids
   */
  
  vptr = *verify_grid;
  fptr = *forecast_grid;
  
  /*
   * loop through y
   */
  
  for (iy = 0; iy < Glob->ny; iy++) {
    
    /*
     * loop through x
     */
    
    for (ix = 0; ix < Glob->nx; ix++) {
      
      /*
       * get grid values
       */
      
      truth = *vptr;
      forecast = *fptr;
      
      if (forecast == 1 && truth > 0) {
	
	cont_table->n_success++;
	
      } else if (forecast == 0 && truth > 0) {
	
	cont_table->n_failure++;
	
      } else if (forecast == 1 && truth == 0) {
	
	cont_table->n_false_alarm++;
	
      } else {
	
	cont_table->n_non_event++;
	
      }
      
      /*
       * increment array pointers
       */
      
      vptr++;
      fptr++;
      
    } /* ix */
    
  } /* iy */

}

static void print_grids(ui08 **verify_grid,
			ui08 **forecast_grid)

{

  char *line;

  int data_found;
  si32 ix, iy;

  line = (char *) ucalloc ((ui32) (Glob->nx + 1), (ui32) 1);

  for (iy = Glob->ny - 1; iy >= 0; iy--) {

    memset(line, '-', (int) Glob->nx);
    data_found = FALSE;

    for (ix = 0; ix < Glob->nx; ix++) {

      if (verify_grid[iy][ix] > 0 &&
	  forecast_grid[iy][ix] > 0) {

	line[ix] = 'S';
	data_found = TRUE;

      } else if (verify_grid[iy][ix] > 0 &&
		 forecast_grid[iy][ix] == 0) {

	line[ix] = 'F';
	data_found = TRUE;

      } else if (verify_grid[iy][ix] == 0 &&
		 forecast_grid[iy][ix] > 0) {

	line[ix] = 'A';
	data_found = TRUE;
	
      } /* if/else */
      
    } /* ix */

    if (data_found)
      fprintf(stderr, "%5ld %s\n", (long) iy, line);

  } /* iy */

  ufree((char *) line);

}
