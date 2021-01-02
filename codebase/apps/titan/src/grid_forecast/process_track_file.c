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
 * process_track_file.c
 *
 * Reads through track file, producing forecast grid
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 ***************************************************************************/

#include "grid_forecast.h"

void process_track_file(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			vol_file_handle_t *v_handle,
			date_time_t *scan_time)

{

  static ui08 *forecast_grid = NULL;
  ui08 *dbz, *fcast;

  int entry_found;

  si32 iscan, isimple, ientry, ipoint;
  si32 npoints;

  double forecast_dx, forecast_dy;

  date_time_t *stime;
  cart_params_t *cart;
  storm_file_global_props_t *gprops;
  track_file_entry_t *entry;
  // track_file_params_t *tparams;
  track_file_forecast_props_t *fprops;

  // tparams = &t_handle->header->params;

  /*
   * loop through the scans in the storm file
   */

  for (iscan = 0; iscan < s_handle->header->n_scans; iscan++) {

    stime = scan_time + iscan;

    /*
     * read in the relevant radar volume file
     */
    
    read_volume(v_handle, stime);

    /*
     * allocate the forecast grid, and zero out
     */

    cart = &v_handle->vol_params->cart;
    npoints = cart->nx * cart->ny;

    if (forecast_grid == NULL)
      forecast_grid = (ui08 *) umalloc
	((ui32) (npoints * sizeof(ui08)));
    else
      forecast_grid = (ui08 *) urealloc
	((char *) forecast_grid,
	 (ui32) (npoints * sizeof(ui08)));
    
    memset ((void *)  forecast_grid,
            (int) 0, (size_t) (npoints * sizeof(ui08)));

    /*
     * loop through all of the relevant simple tracks
     */
    
    for (isimple = 0;
	 isimple < t_handle->header->n_simple_tracks; isimple++) {
      
      if (stime->unix_time >= t_handle->track_utime[isimple].start_simple &&
	  stime->unix_time <= t_handle->track_utime[isimple].end_simple) {
	
	/*
	 * this simple track was active at the scan time, so process it
	 */
	
	if(RfRewindSimpleTrack(t_handle, isimple,
			       "process_track_file") != R_SUCCESS)
	  tidy_and_exit(-1);
	
	/*
	 * search for entry at this time
	 */
	
	entry_found = FALSE;
	
	for (ientry = 0;
	     ientry < t_handle->simple_params->duration_in_scans; ientry++) {
	  
	  if (RfReadTrackEntry(t_handle, "process_track_file") != R_SUCCESS)
	    tidy_and_exit(-1);
	  
	  if (t_handle->entry->time == stime->unix_time) {
	    
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
	  
	  fprintf(stderr, "ERROR - %s:process_track_file\n",
		  Glob->prog_name);
	  fprintf(stderr, "Time-matched entry not found.\n");
	  fprintf(stderr, "Simple track num %ld\n", (long) isimple);
	  fprintf(stderr, "Search time %s\n", utimestr(stime));
	  tidy_and_exit(-1);
	  
	}

	entry = t_handle->entry;

	/*
	 * read in storm data
	 */
	
	if (RfReadStormScan(s_handle, entry->scan_num,
			    "process_track_file") != R_SUCCESS)
	  tidy_and_exit(-1);
	
	gprops = s_handle->gprops + entry->storm_num;

	if (RfReadStormProps(s_handle, entry->storm_num,
			     "process_track_file") != R_SUCCESS)
	  tidy_and_exit(-1);
	
	/*
	 *  compute dx and dy from forecast
	 */

	if (entry->history_in_secs >=
	    Glob->min_history_in_secs) {

	  fprops = &entry->dval_dt;

	  forecast_dx = (fprops->proj_area_centroid_x
			 * Glob->forecast_lead_time);

	  forecast_dy = (fprops->proj_area_centroid_y
			 * Glob->forecast_lead_time);

	} else {

	  forecast_dx = 0.0;
	  forecast_dy = 0.0;

	}

	update_forecast_grid(s_handle, v_handle,
			     forecast_grid,
			     gprops->n_runs,
			     forecast_dx, forecast_dy);
	
      } /* if (stime.unix_time >= ... */
    
    } /* isimple */

    /*
     * merge the dbz plane with the forecast grid
     */

    dbz = v_handle->field_plane[Glob->dbz_field][0];
    fcast = forecast_grid;

    for (ipoint = 0; ipoint < npoints; ipoint++) {

      if (*fcast == DATA_MOVED_FLAG)
	*dbz = 0;
      else if (*fcast != 0)
	*dbz = *fcast;

      fcast++;
      dbz++;

    } /* ipoint */

    /*
     * write the forecast volume
     */

    write_volume(v_handle, stime);
    
  } /* iscan */

}
