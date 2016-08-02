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

int get_mean_motion(storm_file_handle_t *s_handle,
		    track_file_handle_t *t_handle,
		    si32 iscan,
		    double *mean_dx_dt_p,
		    double *mean_dy_dt_p)
     
{

  si32 ientry;
  si32 n_entries;

  double dx_dt, dy_dt;
  double area;
  double x, y;

  double sum_area = 0.0;
  double sum_dx_dt = 0.0;
  double sum_dy_dt = 0.0;
  double mean_dx_dt;
  double mean_dy_dt;

  storm_file_global_props_t *gprops;

  track_file_entry_t *entry;
  track_file_forecast_props_t *fprops;

  /*
   * read in track file scan entries
   */
  
  if (RfReadTrackScanEntries(t_handle, iscan, "refl_forecast_generate"))
    return (-1);
  
  n_entries = t_handle->scan_index[iscan].n_entries;
  
  /*
   * loop through the entries
   */
  
  entry = t_handle->scan_entries;
  
  for (ientry = 0; ientry < n_entries; ientry++, entry++) {
    
    fprops = &entry->dval_dt;
    
    /*
     * read in storm props
     */
      
    if (RfReadStormProps(s_handle, entry->storm_num,
			 "refl_forecast_generate")) {
      return (-1);
    }
    gprops = s_handle->gprops + entry->storm_num;
	
    /*
     * compute current posn and area
     */

    /*
     * get posn and area
     */

    x = gprops->proj_area_centroid_x;
    y = gprops->proj_area_centroid_y;
    area = gprops->proj_area;

    /*
     * compute rates of change of posn and area
     */
      
    if (entry->forecast_valid) {
      dx_dt = fprops->proj_area_centroid_x;
      dy_dt = fprops->proj_area_centroid_y;
    } else {
      dx_dt = 0.0;
      dy_dt = 0.0;
    }

    /*
     * sum up for computing area-weighted mean
     */

    sum_area += area;
    sum_dx_dt += dx_dt * area;
    sum_dy_dt += dy_dt * area;
      
  } /* ientry */

  /*
   * compute area-weighted mean motion
   */

  if (sum_area >= Glob->params.min_area_unthresholded) {
    mean_dx_dt = sum_dx_dt / sum_area;
    mean_dy_dt = sum_dy_dt / sum_area;
  } else {
    mean_dx_dt = 0.0;
    mean_dy_dt = 0.0;
  }

  *mean_dx_dt_p = mean_dx_dt;
  *mean_dy_dt_p = mean_dy_dt;

  return (0);
  
}

