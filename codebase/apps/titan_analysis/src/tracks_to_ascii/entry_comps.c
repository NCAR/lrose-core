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
/*****************************************************************************
 * entry_comps.c
 * 
 * Performs the computations on a track entry, writes
 * the results to stdout
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 *
 * January 1992
 *
 *****************************************************************************/

#include "tracks_to_ascii.h"

#define RAD_TO_DEG 57.29577951308092

void entry_comps(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle)

{

  int accept_entry;
  int i;

  si32 start_sampling_region;
  si32 time_diff;
  si32 this_scan, last_scan;

  double vol_centroid_x;
  double vol_centroid_y;
  double refl_centroid_x;
  double refl_centroid_y;
  double refl_centroid_z;
  double vol_centroid_z;
  double dbz_max, dbz_mean;
  double top, base;
  double ht_of_dbz_max;
  double volume;
  double mass;
  double area_mean;
  double precip_flux, precip_area;
  double tilt_angle;
  double tilt_dirn;
  double vorticity;

  double speed, dirn;
  double dx_dt, dy_dt;
  double dtop_dt;
  double dvolume_dt;
  double dprecip_flux_dt;
  double dmass_dt;
  double ddbz_max_dt;

  double rem_duration;

  date_time_t entry_time;
  date_time_t last_time;

  storm_file_params_t *sparams;
  storm_file_global_props_t *gprops;
  track_file_entry_t *entry;
  track_file_forecast_props_t *fprops;

  sparams = &s_handle->header->params;
  gprops = s_handle->gprops + t_handle->entry->storm_num;
  entry = t_handle->entry;
  fprops = &entry->dval_dt;

  /*
   * Decide if this entry is within a 'sampling region in time'. 
   * A 'sampling region' starts at regular sampling intervals, and
   * lasts for the duration of a scan interval.
   */

  entry_time.unix_time = entry->time;
  uconvert_from_utime(&entry_time);

  if (Glob->sample_interval > 0) {

    /*
     * test if time coincides with sampling time
     */

    start_sampling_region =
      ((entry_time.unix_time / Glob->sample_interval) *
       Glob->sample_interval);
    
    time_diff = entry_time.unix_time - start_sampling_region;

    if (time_diff > Glob->scan_interval * 1.1) {
      accept_entry = FALSE;
    } else {
      accept_entry = TRUE;
    }

  } else {

    /*
     * accept all entries
     */

    accept_entry = TRUE;

  }

  /*
   * Decide if this entry is within a 'sampling region in space'. 
   * The 'sampling region' is a box defined by the box limits
   */

  if (Glob->use_box_limits) {

    if (gprops->refl_centroid_x < Glob->box_min_x ||
	gprops->refl_centroid_x > Glob->box_max_x ||
	gprops->refl_centroid_y < Glob->box_min_y ||
	gprops->refl_centroid_y > Glob->box_max_y) {

      accept_entry = FALSE;

    }

  } /* if (Glob->use_box_limits) */

  /*
   * process entry if it is withing sample region
   */

  if (accept_entry) {

    /*
     * compute life expectancy
     */
    
    last_time.unix_time =
      t_handle->simple_params->last_descendant_end_time;
    uconvert_from_utime(&last_time);
    this_scan = entry->scan_num;
    last_scan = t_handle->simple_params->last_descendant_end_scan;

    if (this_scan == last_scan) {

      rem_duration = 0.0;

    } else {

      /*
       * adjust for the fact that the times are taken at mid-scan,
       * by adding the duration of one-half of a scan
       */

      rem_duration =
	((((double) last_time.unix_time -
	   (double) entry_time.unix_time) +
	  Glob->scan_interval / 2) / 3600.0);

    }
      
    /*
     * compute storm props
     */

    vol_centroid_x = gprops->vol_centroid_x;
    vol_centroid_y = gprops->vol_centroid_y;
    refl_centroid_x = gprops->refl_centroid_x;
    refl_centroid_y = gprops->refl_centroid_y;
    refl_centroid_z = gprops->refl_centroid_z;
    vol_centroid_z = gprops->vol_centroid_z;
    dbz_max = gprops->dbz_max;
    dbz_mean = gprops->dbz_mean;
    top = gprops->top;
    base = gprops->base;
    ht_of_dbz_max = gprops->ht_of_dbz_max;
    volume = gprops->volume;
    mass = gprops->mass;
    area_mean = gprops->area_mean;
    precip_flux = gprops->precip_flux;
    precip_area = gprops->precip_area;
    tilt_angle = gprops->tilt_angle;
    tilt_dirn = gprops->tilt_dirn;
    vorticity = gprops->vorticity;

    dx_dt = fprops->proj_area_centroid_x;
    dy_dt = fprops->proj_area_centroid_y;
    dtop_dt = fprops->top;
    dvolume_dt = fprops->volume;
    dprecip_flux_dt = fprops->precip_flux;
    dmass_dt = fprops->mass;
    ddbz_max_dt = fprops->dbz_max;

    if (dx_dt == 0.0 && dy_dt == 0.0) {

      speed = 0.0;
      dirn = 0.0;

    } else {

      speed = sqrt(dx_dt * dx_dt + dy_dt * dy_dt);
      dirn = atan2(dx_dt, dy_dt) * RAD_TO_DEG;

    }
    
    fprintf(stdout, "%3ld %4ld %4ld ",
	    (long) t_handle->complex_params->n_simple_tracks,
	    (long) t_handle->complex_params->complex_track_num,
	    (long) t_handle->simple_params->simple_track_num);

    fprintf(stdout,
	    "%.4d %.2d %.2d %.2d %.2d %.2d",
	    entry_time.year,
	    entry_time.month,
	    entry_time.day,
	    entry_time.hour,
	    entry_time.min,
	    entry_time.sec);

    fprintf(stdout, " %10g", sparams->low_dbz_threshold);
    fprintf(stdout, " %10g", rem_duration);
    fprintf(stdout, " %10g", vol_centroid_x);
    fprintf(stdout, " %10g", vol_centroid_y);
    fprintf(stdout, " %10g", refl_centroid_x);
    fprintf(stdout, " %10g", refl_centroid_y);
    fprintf(stdout, " %10g", refl_centroid_z);
    fprintf(stdout, " %10g", vol_centroid_z);
    fprintf(stdout, " %10g", dbz_max);
    fprintf(stdout, " %10g", dbz_mean);
    fprintf(stdout, " %10g", top);
    fprintf(stdout, " %10g", base);
    fprintf(stdout, " %10g", ht_of_dbz_max);
    fprintf(stdout, " %10g", volume);
    fprintf(stdout, " %10g", mass);
    fprintf(stdout, " %10g", area_mean);
    fprintf(stdout, " %10g", precip_flux);
    fprintf(stdout, " %10g", precip_area);
    fprintf(stdout, " %10g", tilt_angle);
    fprintf(stdout, " %10g", tilt_dirn);
    fprintf(stdout, " %10.3e", vorticity);

    fprintf(stdout, " %10g", speed);
    fprintf(stdout, " %10g", dirn);
    fprintf(stdout, " %10g", dtop_dt);
    fprintf(stdout, " %10g", dvolume_dt);
    fprintf(stdout, " %10g", dprecip_flux_dt);
    fprintf(stdout, " %10g", dmass_dt);
    fprintf(stdout, " %10g", ddbz_max_dt);

    if (Glob->print_polygons) {
      fprintf(stdout, " %d", (int) sparams->n_poly_sides);
      for (i = 0; i < sparams->n_poly_sides; i++) {
	fprintf(stdout, " %3.1f", gprops->proj_area_polygon[i]);
      }
    }

    fprintf(stdout, "\n");

  } /* if (accept_entry) */

}
