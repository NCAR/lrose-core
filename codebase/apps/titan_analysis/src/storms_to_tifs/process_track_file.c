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

#include "storms_to_tifs.h"

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
  double dx_dt, dy_dt;
  double speed, dirn;
  double dvolume_dt, dproj_area_dt;
  double storm_lat, storm_lon;
  double cart_lat, cart_lon;
  double x, y, range, theta;

  date_time_t *stime, ltime;
  date_time_t dtime;
  date_time_t file_time;   /* rjp 27Oct99 */
  storm_file_params_t *sparams;
  storm_file_global_props_t *gprops;
  storm_file_dbz_hist_t *hist;
  /* track_file_params_t *tparams; */
  track_file_entry_t *entry;
  track_file_forecast_props_t *fprops;
  
  sparams = &s_handle->header->params;
  /* tparams = &t_handle->header->params; */
  ugmtime(&file_time);

 /* print header */
    fprintf(stdout,"storms_to_ascii \n");
    fprintf(stdout,"File created(UTC):   %s\n", utimestr(&file_time));
    fprintf(stdout,"Last scan time(UTC): %s\n",
        utimstr(s_handle->header->end_time));
    fprintf(stdout," \n");
    fprintf(stdout,"Radar location: \n");
    fprintf(stdout,"   Lat:  %8.3f\n",
        s_handle->scan->grid.proj_origin_lat);
    fprintf(stdout,"   Long: %8.3f\n",
        s_handle->scan->grid.proj_origin_lon);
    fprintf(stdout," \n");
    fprintf(stdout,"Storm file parameters: \n");
    fprintf(stdout,"   Low dBZ threshold:       %4.0f\n",
        sparams->low_dbz_threshold);
    fprintf(stdout,"   High dBZ threshold:      %4.0f\n",
        sparams->high_dbz_threshold);
    fprintf(stdout,"   Hail dBZ threshold:      %4.0f\n",
        sparams->hail_dbz_threshold);
    fprintf(stdout,"   Base threshold (km):     %5.1f\n",
        sparams->base_threshold);
    fprintf(stdout,"   Top threshold (km):      %5.1f\n",
        sparams->top_threshold);
    fprintf(stdout,"   Min radar tops (km):     %5.1f\n",
        sparams->min_radar_tops);
    fprintf(stdout,"   Size threshold(km3/km2): %6.0f\n",
        sparams->min_storm_size);
    fprintf(stdout,"   Z-R coefficient:         %4.0f\n",
        sparams->z_p_coeff);    
    fprintf(stdout,"   Z-R exponent:            %5.1f\n",
        sparams->z_p_exponent);
    fprintf(stdout,"\n");

    fprintf(stdout,"scan_num,complex_tk,simple_tk,");
    fprintf(stdout,"yyyy,mm,dd,hh,mm,ss,");
    fprintf(stdout,"centr_x(km),centr_y(km),lat(deg),long(deg),");
    fprintf(stdout,"precip_area(km2),precip_rate(mm/hr),");
    fprintf(stdout,"maj_rad(km),min_rad(km),orientation(degTN),");
    fprintf(stdout,"vol(km3),mass(ktons),top(km),");
    fprintf(stdout,"max_dBZ,mean_dBZ,");
    fprintf(stdout,"%%V>40dBZ,%%V>50dBZ,");
    fprintf(stdout,"%%V>60dBZ,%%V>70dBZ,");
    fprintf(stdout,"speed(km/hr),dirn(degT),");
    fprintf(stdout,"dV/dt(km3/hr),d(precip_area)/dt(km2/hr),");
    fprintf(stdout,"vil(kg/m2),ht_max_dBZ(km)\n");

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
	  fprintf(stderr, "Simple track num %ld\n", (long)isimple);
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
	  range = sqrt(x*x + y*y);
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
	 * iscan,complex_num,isimple,
	 * year,month,day,hour,min,sec
	 * proj_area_centroid_x (km)
	 * proj_area_centroid_y (km)
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
	 * vil kg/m^2 (pjp 28/2/03 add vil field
	 */

        /* (rjp 8Sep1999) Add scan num, complex_num, simple_num to output. */
        fprintf(stdout,
	        "%5d %5d %5d ",iscan, t_handle->simple_params->complex_track_num,
                isimple);
		
	fprintf(stdout,
		"%.4d %.2d %.2d %.2d %.2d %.2d ",
		ltime.year,
		ltime.month,
		ltime.day,
		ltime.hour,
		ltime.min,
		ltime.sec);
	
	fprintf(stdout,
		"%10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f ",
		x, y,
		storm_lat, storm_lon,
		proj_area, precip_rate,
		gprops->proj_area_major_radius,
		gprops->proj_area_minor_radius,
		gprops->proj_area_orientation);
		
	fprintf(stdout,
		"%10.2f %10.2f %10.2f %10.2f %10.2f ",
		gprops->volume,
		gprops->mass,
		gprops->top,
		gprops->dbz_max,
		gprops->dbz_mean);
		
	fprintf(stdout,
		"%10.2f %10.2f %10.2f %10.2f ",
		percent_40, percent_50,
		percent_60, percent_70);
		
	fprintf(stdout,
		"%10.2f %10.2f %10.2f %10.2f %10.2f %10.2f ",
		speed, dirn,
		dvolume_dt, dproj_area_dt,
		gprops->vil_from_maxz,
		gprops->ht_of_dbz_max);

	fprintf(stdout, "\n");

      } /* if (stime.unix_time >= ... */
    
    } /* isimple */

  } /* iscan */

}


