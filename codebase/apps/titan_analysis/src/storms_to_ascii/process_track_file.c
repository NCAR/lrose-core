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
/***********************************************************************
 * process_track_file.c
 *
 * Reads through track file, printing storm data
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 **********************************************************************/

#include "storms_to_ascii.h"

/* #define RAD_TO_DEG 57.29577951308092 */

void process_track_file(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			date_time_t *scan_time)

{

  int entry_found;

  si32 iscan, isimple, ientry, interval;

  double proj_area, precip_flux, precip_rate;
  double level;
  double percent, percent_40, percent_50, percent_60, percent_70;
  double dx_dt, dy_dt, speed, dirn;
  double dvolume_dt, dproj_area_dt;
  double storm_lat, storm_lon;
  double cart_lat, cart_lon;
  double x, y, range, theta;

  date_time_t *stime, ltime;
  date_time_t dtime;
  storm_file_params_t *sparams;
  storm_file_global_props_t *gprops;
  storm_file_dbz_hist_t *hist;
  /* track_file_params_t *tparams; */
  track_file_entry_t *entry;
  track_file_forecast_props_t *fprops;
  
  sparams = &s_handle->header->params;
  /* tparams = &t_handle->header->params; */

  /*
   * loop through the scans in the storm file
   */

  for (iscan = 0; iscan < s_handle->header->n_scans; iscan++) {

    stime = scan_time + iscan;
    dtime = *stime;
    uconvert_to_utime(&dtime);

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
	     ientry < t_handle->simple_params->duration_in_scans;
	     ientry++) {
	  
	  if (RfReadTrackEntry(t_handle, "process_track_file") != R_SUCCESS)
	    tidy_and_exit(-1);
	  
	  if (t_handle->entry->time == dtime.unix_time) {

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
	 * compute properties
	 */

	cart_lat = s_handle->scan->grid.proj_origin_lat;
	cart_lon = s_handle->scan->grid.proj_origin_lon;
	x = gprops->proj_area_centroid_x;
	y = gprops->proj_area_centroid_y;

	if (x == 0.0 && y == 0.0) {
	  range = 0.0;
	  theta = 0.0;
	} else {
	  range = sqrt(x * x + y * y);
	  theta = atan2(x, y) * RAD_TO_DEG;
	}

	uLatLonPlusRTheta(cart_lat, cart_lon, 
			  range, theta,
			  &storm_lat, &storm_lon);

	precip_flux = gprops->precip_flux;
	proj_area = gprops->proj_area;

	if (proj_area == 0.0)
	  precip_rate = 0.0;
	else
	  precip_rate = (precip_flux / proj_area) * 3.6;

	percent_40 = 0;
	percent_50 = 0;
	percent_60 = 0;
	percent_70 = 0;
      
	for (interval = 0;
	     interval < gprops->n_dbz_intervals; interval++) {
	  
	  hist = s_handle->hist + interval;
	  level = (sparams->low_dbz_threshold +
		   (double) interval * sparams->dbz_hist_interval);

	  percent = hist->percent_volume;

	  if (level > 40.0)
	    percent_40 += percent;

	  if (level > 50.0)
	    percent_50 += percent;

	  if (level > 60.0)
	    percent_60 += percent;

	  if (level > 70.0)
	    percent_70 += percent;

	} /* interval */

	fprops = &entry->dval_dt;

	dx_dt = fprops->proj_area_centroid_x;
	dy_dt = fprops->proj_area_centroid_y;
	dproj_area_dt = fprops->proj_area;
	dvolume_dt = fprops->volume;

	if (dx_dt == 0.0 && dy_dt == 0.0) {
	  speed = 0.0;
	  dirn = 0.0;
	} else {
	  speed = sqrt(dx_dt * dx_dt + dy_dt * dy_dt);
	  dirn = atan2(dx_dt, dy_dt) * RAD_TO_DEG;
	}
    
	ltime.unix_time = stime->unix_time - Glob->gmt_offset;
	uconvert_from_utime(&ltime);

	/*
	 * print out properties as below
	 *
	 * year,month,day,hour,min,sec
	 * x (km)
	 * y (km)
	 * latitude (deg)
	 * longitude (deg - minus is west)
	 * precip area (km2)
	 * precip rate (mm/hr)
	 * major radius (km)
	 * minor radius (km)
	 * orientation from (deg from TN)
	 * volume (km3)
	 * mass (ktons)
	 * top (km msl)
	 * max dBZ
	 * mean dBZ
	 * % vol > 40 dBZ
	 * % vol > 50 dBZ
	 * % vol > 60 dBZ
	 * % vol > 70 dBZ
	 * speed (km\hr)
	 * direction of movement (deg from TN)
	 * rate of change of volume (km3 / hr)
	 * rate of change of precip area (km2 / hr)
	 */

	fprintf(stdout,
		"%d %d %d %d %d %d ",
		ltime.year,
		ltime.month,
		ltime.day,
		ltime.hour,
		ltime.min,
		ltime.sec);
	
	fprintf(stdout,
		"%g %g %g %g %g %g %g %g %g ",
		x, y,
		storm_lat, storm_lon,
		proj_area, precip_rate,
		gprops->proj_area_major_radius,
		gprops->proj_area_minor_radius,
		gprops->proj_area_orientation);
		
	fprintf(stdout,
		"%g %g %g %g %g ",
		gprops->volume,
		gprops->mass,
		gprops->top,
		gprops->dbz_max,
		gprops->dbz_mean);
		
	fprintf(stdout,
		"%g %g %g %g ",
		percent_40, percent_50,
		percent_60, percent_70);
		
	fprintf(stdout,
		"%g %g %g %g ",
		speed, dirn,
		dvolume_dt, dproj_area_dt);

	fprintf(stdout, "\n");

      } /* if (stime.unix_time >= ... */
    
    } /* isimple */

  } /* iscan */

}






