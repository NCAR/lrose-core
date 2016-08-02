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
 * print_header.c
 *
 * Prints the header line
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Jan 1992
 *
 ****************************************************************************/

#include "tracks_to_ascii.h"

void print_header(si32 n_track_files,
		  char **track_file_paths)

{

  si32 i;

  date_time_t file_time;

  ulocaltime(&file_time);

  printf("#File create time: %s\n", utimestr(&file_time));

  printf("#File name(s):\n");
  
  for (i = 0; i < n_track_files; i++)
    printf("#    %s\n", track_file_paths[i]);

  if (Glob->target_entity == COMPLETE_TRACK) {

    printf("#target_entity: complete_track\n");

  } else if (Glob->target_entity == TRACK_ENTRY) {

    printf("#target_entity: track_entry\n");
    printf("#sample_interval (secs): %g\n",
	   (double) Glob->sample_interval);
    printf("#scan_interval (secs): %g\n",
	   (double) Glob->scan_interval);

  } else if (Glob->target_entity == TRENDS) {

    printf("#target_entity: trends\n");
    printf("#nscans_pre_trend: %ld\n", (long) Glob->nscans_pre_trend);
    printf("#min_nscans_pre_trend: %ld\n", (long) Glob->min_nscans_pre_trend);
    printf("#nscans_post_trend: %ld\n", (long) Glob->nscans_post_trend);
    printf("#min_nscans_post_trend: %ld\n",
	   (long) Glob->min_nscans_post_trend);
    printf("#min_nscans_monotonic: %ld\n",
	   (long) Glob->min_nscans_monotonic);

  }

  if (Glob->use_simple_tracks)
    printf("#simple tracks used\n");
   
  if (Glob->use_complex_tracks)
    printf("#complex tracks used\n");
  
  if (Glob->nonzero_verification_only)
    printf("#nonzero verification only\n");

  if (Glob->use_box_limits) {

    printf("#Box limits (%ld, %ld) to (%ld, %ld)\n",
	   (long) Glob->box_min_x, (long) Glob->box_min_y,
	   (long) Glob->box_max_x, (long) Glob->box_max_y);
    
    if (Glob->target_entity == COMPLETE_TRACK) {

      printf("#min percent in box: %g\n", Glob->min_percent_in_box);
      printf("#min nstorms in box: %ld\n", (long) Glob->min_nstorms_in_box);

    }
    
  } /* if (Glob->use_box_limits) */

  printf("#max_top_missing: %ld\n", (long) Glob->max_top_missing);
  printf("#max_range_limited: %ld\n", (long) Glob->max_range_limited);
  printf("#max_vol_at_start_of_sampling: %ld\n",
	 (long) Glob->max_vol_at_start_of_sampling);
  printf("#max_vol_at_end_of_sampling: %ld\n",
	 (long) Glob->max_vol_at_end_of_sampling);
  

  if (Glob->target_entity == COMPLETE_TRACK) {

    printf("#labels: %s\n",
	   "Track num,"
	   "Year,"
	   "Month,"
	   "Day,"
	   "Hour,"
	   "Min,"
	   "Sec,"
	   "Nscans,"
	   "Duration(hours),"
	   "Rem. dur. at max vol(hours),"
	   "Mean volume(km3),"
	   "Max volume(km3),"
	   "Mean mass(ktons),"
	   "Max mass(ktons),"
	   "Max precip depth - ellipse(mm),"
	   "Mean precip depth - ellipse(mm),"
	   "Mean precip depth - runs(mm),"
	   "Mean precip flux(m3/s),"
	   "Max precip flux(m3/s),"
	   "Mean projected area(km2),"
	   "Max projected area(km2),"
	   "Mean precip area(km2),"
	   "Max precip area(km2),"
	   "Mean top(km msl),"
	   "Max top(km msl),"
	   "Mean base(km msl),"
	   "Max base(km msl),"
	   "Mean dBZ,"
	   "Max dBZ,"
	   "Volume lag1 autocorrelation,"
	   "Volume lag2 autocorrelation,"
	   "Precip_area lag1 autocorrelation,"
	   "Precip_area lag2 autocorrelation,"
	   "Proj_area lag1 autocorrelation,"
	   "Proj_area lag2 autocorrelation,"
	   "Radar est. rain vol (m3),"
	   "ATI(km2.hr),"
	   "Swath area - ellipse(km2),"
	   "Swath area - runs(km2),"
	   "Mean speed(km/hr),"
	   "Mean direction(deg T),"
	   "POD,"
	   "FAR,"
	   "CSI");

  } else if (Glob->target_entity == TRACK_ENTRY) {

    printf("#labels: %s",
	   "N simple tracks,"
	   "Complex num,Simple num,"
	   "Year,Month,Day,Hour,Min,Sec,"
	   "dBZ threshold,"
	   "Remaining duration (hrs),"
	   "Vol. centroid x (km),Vol. centroid y (km),"
	   "Refl. centroid x (km),Refl. centroid y (km),"
	   "Ht of refl. centroid (km),Ht of centroid (km),"
	   "Max reflectivity (dBZ),Mean reflectivity (dBZ),"
	   "Top (km),Base(km),"
	   "Ht of max reflectivity (km),"
	   "Volume (km3),Mass (ktons),Mean area (km2),"
	   "Precip flux (m3/s),Precip area (km2),"
	   "Tilt angle (deg),Tilt orientation (deg T),"
	   "Vorticity (/s),"
	   "Speed (km/hr),Dirn (Deg T),"
	   "Dtop/Dt (km/hr),"
	   "Dvolume/Dt (km3/hr),Dprecip_flux/Dt (m3/s2), Dmass/Dt (ktons/hr),"
	   "DdBz_max/Dt (dBZ/hr)");

    if (Glob->print_polygons) {
      printf(" n_poly_sides polygon_rays*%d", N_POLY_SIDES);
    }

    printf("\n");

  } else if (Glob->target_entity == TRENDS) {

    printf("#labels: %s,%s,%s%d,%s%d,%s%d,%s,%s,%s,%s%d,%s%d,%s%d,%s,%s\n",

	   "Complex num,Nscans,Iscan,Year,Month,Day,Hour,Min,Sec",

	   "Vol_centroid_z,Refl_centroid_z,Dbz_max,Top,Ht_of_dbz_max",
	   "Max_ht_", (int) Glob->dbz_for_max_ht,
	   "Vol_percentile_", (int) Glob->vol_percentile,
	   "Percent_vol_above_", (int) Glob->dbz_for_percent_vol_above,
	   "Volume",

	   "Pre_trend_vol_centroid_z,Pre_trend_refl_centroid_z",
	   "Pre_trend_dbz_max,Pre_trend_ht_of_dbz_max",
	   "Pre_trend_max_ht_", (int) Glob->dbz_for_max_ht,
	   "Pre_trend_vol_percentile_", (int) Glob->vol_percentile,
	   "Pre_trend_percent_vol_above_",
	   (int) Glob->dbz_for_percent_vol_above,

	   "Pre_trend_volume,Norm_pre_trend_volume,Post_trend_volume",
	   "Norm_post_trend_volume,Norm_forecast_trend_volume");

  }

}

