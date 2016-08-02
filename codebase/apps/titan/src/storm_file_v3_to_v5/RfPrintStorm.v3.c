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
/*************************************************************************
 *
 * RfPrintStorm.c
 *
 * Storm file printing routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * January 1995
 *
 **************************************************************************/

#include "storm_file_v3_to_v5.h"

#define BOOL_STR(a) (a == FALSE ? "false" : "true")

void Rfv3PrintStormHeader(FILE *out,
			  char *spacer,
			  storm_v3_file_header_t *header)
     
{

  char spacer2[128];

  sprintf(spacer2, "%s  ", spacer);
  
  /*
   * print out header
   */

  fprintf(out, "%sStorm file header :\n", spacer);

  fprintf(out, "%s  Dates and times : \n", spacer);
  fprintf(out, "%s    File   : %s\n",  spacer,
	  utimestr((date_time_t *) &header->file_time));
  fprintf(out, "%s    Start  : %s\n",  spacer,
	  utimestr((date_time_t *) &header->start_time));
  fprintf(out, "%s    End    : %s\n",  spacer,
	  utimestr((date_time_t *) &header->end_time));
  fprintf(out, "%s  Number of scans : %d\n", spacer,
	  header->n_scans);
  fprintf(out, "%s  Data file size  : %d\n", spacer,
	  header->data_file_size);
  fprintf(out, "\n");

  Rfv3PrintStormParams(stdout, spacer2, &header->params);

  return;

}

/*-----------------------------
 */

void Rfv3PrintStormHist(FILE *out,
			char *spacer,
			storm_v3_file_params_t *params,
			storm_v3_file_global_props_t *gprops,
			storm_v3_file_dbz_hist_t *hist)
     
{

  si32 interval;
  storm_v3_file_dbz_hist_t *h;
  storm_v3_float_dbz_hist_t fl_hist;
  storm_v3_float_params_t fl_params;
  
  Rfv3DecodeStormParams(params, &fl_params);
  
  fprintf(out, "%sDbz histograms : \n", spacer);
  fprintf(out, "%s%10s %10s %12s %12s\n", spacer,
	  "Low dbz", "High Dbz", "% volume", "% precip area");
  
  h = hist;
    
  for (interval = 0;
       interval < gprops->n_dbz_intervals; interval++, h++) {
    
    Rfv3DecodeStormHist(params, h, &fl_hist);
    
    fprintf(out, "%s%10.1f %10.1f %12.2f %12.2f\n", spacer,
	    (fl_params.low_dbz_threshold +
	     (double) interval * fl_params.dbz_hist_interval),
	    (fl_params.low_dbz_threshold +
	     (double) (interval + 1) * fl_params.dbz_hist_interval),
	    fl_hist.percent_volume,
	    fl_hist.percent_precip_area);
    
  } /* interval */

  fprintf(out, "\n");

  return;

}

/*-------------------------------
 */

void Rfv3PrintStormLayer(FILE *out,
			 char *spacer,
			 storm_v3_file_params_t *params,
			 storm_v3_file_scan_header_t *scan,
			 storm_v3_file_global_props_t *gprops,
			 storm_v3_file_layer_props_t *layer)
     
{

  char *loc_label;
  int ilayer;
  storm_v3_file_layer_props_t *l;
  storm_v3_float_scan_header_t fl_scan;
  storm_v3_float_layer_props_t fl_layer;
  
  if (Rfv3StormGridType(params) == PJG_FLAT) {
    loc_label = "(km) ";
  } else {
    loc_label = "(deg)";
  }

  Rfv3DecodeStormScan(params, scan, &fl_scan);

  /*
   * layer properties
   */
  
  fprintf(out, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s %7s %12s\n",
	  spacer,
	  "Layer", "z", "x-cent", "y-cent", "x-Zcent", "y-Zcent",
	  "area", "max Z", "mean Z", "mass", "vorticity");
  fprintf(out, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s %7s %12s\n",
	  spacer, " ", "(km)", loc_label, loc_label, loc_label, loc_label,
	  "(km2)", "(dbz)", "(dbz)", "(ktons)", "(/s)");
  
  l = layer;
    
  for (ilayer = gprops->base_layer;
       ilayer < (gprops->base_layer + gprops->n_layers);
       ilayer++, l++) {
    
    Rfv3DecodeStormLayer(params, gprops, l, &fl_layer);

    fprintf(out, "%s%5d %5g %7.1f %7.1f %7.1f %7.1f "
	    "%6.1f %6.1f %7.1f %7.1f %12.2e\n",
	    spacer, ilayer,
	    (fl_scan.min_z + (double) ilayer * fl_scan.delta_z),
	    fl_layer.vol_centroid_x,
	    fl_layer.vol_centroid_y,
	    fl_layer.refl_centroid_x,
	    fl_layer.refl_centroid_y,
	    fl_layer.area,
	    fl_layer.dbz_max,
	    fl_layer.dbz_mean,
	    fl_layer.mass,
	    fl_layer.vorticity);
    
  } /* ilayer */
  
  fprintf(out, "\n");
  
  return;

}

/*--------------------------------
 */

void Rfv3PrintStormParams(FILE *out,
			  char *spacer,
			  storm_v3_file_params_t *params)
     
{

  storm_v3_float_params_t fl_params;

  Rfv3DecodeStormParams(params, &fl_params);

  fprintf(out, "%sStorm file parameters : \n", spacer);

  fprintf(out, "%s  Minor revision num : %ld\n", spacer,
	  (long) params->minor_rev);
  fprintf(out, "%s  Low dBZ threshold : %g\n", spacer,
	  fl_params.low_dbz_threshold);
  fprintf(out, "%s  High dBZ threshold : %g\n", spacer,
	  fl_params.high_dbz_threshold);
  fprintf(out, "%s  Hail dBZ threshold : %g\n", spacer,
	  fl_params.hail_dbz_threshold);
  fprintf(out, "%s  Dbz hist interval : %g\n", spacer,
	  fl_params.dbz_hist_interval);
  fprintf(out, "%s  Top threshold (km) : %g\n", spacer,
	  fl_params.top_threshold);
  fprintf(out, "%s  Base threshold (km) : %g\n", spacer,
	  fl_params.base_threshold);
  fprintf(out, "%s  Merge_Ht threshold (km) : %g\n", spacer,
	  fl_params.merge_ht_threshold);
  fprintf(out, "%s  Min storm size (km2 or km3) : %g\n", spacer,
	  fl_params.min_storm_size);
  fprintf(out, "%s  Max storm size (km2 or km3) : %g\n", spacer,
	  fl_params.max_storm_size);
  fprintf(out, "%s  Z-R coefficient : %g\n", spacer,
	  fl_params.z_p_coeff);
  fprintf(out, "%s  Z-R exponent : %g\n", spacer,
	  fl_params.z_p_exponent);
  fprintf(out, "%s  Z-M coefficient : %g\n", spacer,
	  fl_params.z_m_coeff);
  fprintf(out, "%s  Z-M exponent : %g\n", spacer,
	  fl_params.z_m_exponent);
  fprintf(out, "%s  Sectrip vert aspect : %g\n", spacer,
	  fl_params.sectrip_vert_aspect);
  fprintf(out, "%s  Sectrip horiz aspect : %g\n", spacer,
	  fl_params.sectrip_horiz_aspect);
  fprintf(out, "%s  Sectrip orientation error : %g\n", spacer,
	  fl_params.sectrip_orientation_error);

  if (Rfv3StormGridType(params) == PJG_FLAT) {
    fprintf(out, "%s  Gridtype : flat\n", spacer);
  } else if (Rfv3StormGridType(params) == PJG_LATLON) {
    fprintf(out, "%s  Gridtype : latlon\n", spacer);
  }
  
  fprintf(out, "%s  Velocity data available? : %s\n", spacer,
	  BOOL_STR(params->vel_available));

  fprintf(out, "%s  N_poly_sides : %ld\n", spacer,
	  (long) params->n_poly_sides);
  fprintf(out, "%s  Poly_start_az : %g\n", spacer,
	  fl_params.poly_start_az);
  fprintf(out, "%s  Poly_delta_az : %g\n", spacer,
	  fl_params.poly_delta_az);
    
  fprintf(out, "\n");

  return;

}

/*-------------------------------
 */

void Rfv3PrintStormProps(FILE *out,
			 char *spacer,
			 storm_v3_file_params_t *params,
			 storm_v3_file_global_props_t *gprops)
     
{

  char *loc_label;
  si32 i;
  storm_v3_float_global_props_t fl_gprops;
  
  if (Rfv3StormGridType(params) == PJG_FLAT) {
    loc_label = "(km) ";
  } else {
    loc_label = "(deg)";
  }

  Rfv3DecodeStormProps(params, gprops, &fl_gprops, STORM_V3_ALL_PROPS);
  
  fprintf(out, "%sGLOBAL STORM PROPERTIES - storm number %ld\n",
	  spacer, (long) gprops->storm_num);
  
  fprintf(out, "%s  number of layers              : %ld\n", spacer,
	  (long) gprops->n_layers);
  fprintf(out, "%s  base layer number             : %ld\n", spacer,
	  (long) gprops->base_layer);
  fprintf(out, "%s  number of dbz intervals       : %ld\n", spacer,
	  (long) gprops->n_dbz_intervals);
  fprintf(out, "%s  number of runs                : %ld\n", spacer,
	  (long) gprops->n_runs);
  
  fprintf(out, "%s  range_limited                 : %ld\n", spacer,
	  (long) gprops->range_limited);
  fprintf(out, "%s  top_missing                   : %ld\n", spacer,
	  (long) gprops->top_missing);
  fprintf(out, "%s  second_trip                   : %ld\n", spacer,
	  (long) gprops->second_trip);
  fprintf(out, "%s  hail_present                  : %ld\n", spacer,
	  (long) gprops->hail_present);
  
  fprintf(out, "%s  vol_centroid_x %s          : %g\n", spacer,
	  loc_label, fl_gprops.vol_centroid_x);
  fprintf(out, "%s  vol_centroid_y %s          : %g\n", spacer,
	  loc_label, fl_gprops.vol_centroid_y);
  fprintf(out, "%s  vol_centroid_z (km)           : %g\n", spacer,
	  fl_gprops.vol_centroid_z);
  
  fprintf(out, "%s  refl_centroid_x %s         : %g\n", spacer,
	  loc_label, fl_gprops.refl_centroid_x);
  fprintf(out, "%s  refl_centroid_y %s         : %g\n", spacer,
	  loc_label, fl_gprops.refl_centroid_y);
  fprintf(out, "%s  refl_centroid_z (km)          : %g\n", spacer,
	  fl_gprops.refl_centroid_z);
  
  if (params->minor_rev >= STORM_V3_MINOR_REV_2) {
    fprintf(out, "%s  vol_mult                      : %ld\n", spacer,
	    (long) gprops->vol_mult);
    fprintf(out, "%s  area_mult                     : %ld\n", spacer,
	    (long) gprops->area_mult);
    fprintf(out, "%s  mass_mult                     : %ld\n", spacer,
	    (long) gprops->mass_mult);
    fprintf(out, "%s  flux_mult                     : %ld\n", spacer,
	    (long) gprops->flux_mult);
  }
  
  fprintf(out, "%s  top (km)                      : %g\n", spacer,
	  fl_gprops.top);
  fprintf(out, "%s  base (km)                     : %g\n", spacer,
	  fl_gprops.base);
  fprintf(out, "%s  volume (km3)                  : %g\n", spacer,
	  fl_gprops.volume);
  fprintf(out, "%s  mean area (km2)               : %g\n", spacer,
	  fl_gprops.area_mean);
  fprintf(out, "%s  precip flux (m3/s)            : %g\n", spacer,
	  fl_gprops.precip_flux);
  fprintf(out, "%s  mass (ktons)                  : %g\n", spacer,
	  fl_gprops.mass);
  
  fprintf(out, "%s  tilt angle (deg)              : %g\n", spacer,
	  fl_gprops.tilt_angle);
  fprintf(out, "%s  tilt direction (deg)          : %g\n", spacer,
	  fl_gprops.tilt_dirn);
  
  fprintf(out, "%s  dbz max                       : %g\n", spacer,
	  fl_gprops.dbz_max);
  fprintf(out, "%s  dbz mean                      : %g\n", spacer,
	  fl_gprops.dbz_mean);
  fprintf(out, "%s  height of max dbz             : %g\n", spacer,
	  fl_gprops.ht_of_dbz_max);
  
  fprintf(out, "%s  vorticity (/s)                : %.2e\n", spacer,
	  fl_gprops.vorticity);
  
  fprintf(out, "%s  precip area (km2)               : %g\n", spacer,
	  fl_gprops.precip_area);
  fprintf(out, "%s  precip area centroid x %s    : %g\n", spacer,
	  loc_label,
	  fl_gprops.precip_area_centroid_x);
  fprintf(out, "%s  precip area centroid y %s    : %g\n", spacer,
	  loc_label,
	  fl_gprops.precip_area_centroid_y);
  fprintf(out, "%s  precip area orientation (deg)   : %g\n", spacer,
	  fl_gprops.precip_area_orientation);
  fprintf(out, "%s  precip area minor sd %s      : %g\n", spacer,
	  loc_label,
	  fl_gprops.precip_area_minor_sd);
  fprintf(out, "%s  precip area major sd %s      : %g\n", spacer,
	  loc_label,
	  fl_gprops.precip_area_major_sd);
  fprintf(out, "%s  precip area minor radius %s  : %g\n", spacer,
	  loc_label,
	  fl_gprops.precip_area_minor_radius);
  fprintf(out, "%s  precip area major radius %s  : %g\n", spacer,
	  loc_label,
	  fl_gprops.precip_area_major_radius);
  
  fprintf(out, "%s  proj. area (km2)                : %g\n", spacer,
	  fl_gprops.proj_area);
  fprintf(out, "%s  proj. area centroid x %s     : %g\n", spacer,
	  loc_label,
	  fl_gprops.proj_area_centroid_x);
  fprintf(out, "%s  proj. area centroid y %s     : %g\n", spacer,
	  loc_label,
	  fl_gprops.proj_area_centroid_y);
  fprintf(out, "%s  proj. area orientation (deg)    : %g\n", spacer,
	  fl_gprops.proj_area_orientation);
  fprintf(out, "%s  proj. area minor sd %s       : %g\n", spacer,
	  loc_label,
	  fl_gprops.proj_area_minor_sd);
  fprintf(out, "%s  proj. area major sd %s       : %g\n", spacer,
	  loc_label,
	  fl_gprops.proj_area_major_sd);
  fprintf(out, "%s  proj. area minor radius %s   : %g\n", spacer,
	  loc_label,
	  fl_gprops.proj_area_minor_radius);
  fprintf(out, "%s  proj. area major radius %s   : %g\n", spacer,
	  loc_label,
	  fl_gprops.proj_area_major_radius);
  
  fprintf(out, "%s\n    Proj. area polygon rays:\n", spacer);
  for (i = 0; i < V3_N_POLY_SIDES; i++) {
    fprintf(out, "%s    side %ld : %g\n", spacer,
	    (long) i, fl_gprops.proj_area_polygon[i]);
  }
  
  fprintf(out, "\n");

  return;

}

/*-----------------------------
 */

void Rfv3PrintStormRuns(FILE *out,
			char *spacer,
			storm_v3_file_global_props_t *gprops,
			storm_v3_file_run_t *runs)
     
{

  si32 irun;
  storm_v3_file_run_t *run;
  
  fprintf(out, "%sRuns\n", spacer);
  fprintf(out, "%s%10s %10s %10s %10s\n", spacer,
	  "ix", "iy", "iz", "n");
  
  run = runs;
  for (irun = 0; irun < gprops->n_runs; irun++, run++) {
    fprintf(out, "%s%10d %10d %10d %10d\n",
	    spacer, run->ix, run->iy, run->iz, run->n);
  } /* irun */
  
  fprintf(out, "\n");

  return;
  
}

/*-----------------------------
 */

void Rfv3PrintStormScan(FILE *out,
			char *spacer,
			storm_v3_file_params_t *params,
			storm_v3_file_scan_header_t *scan)
     
{

  char buf[128];
  storm_v3_float_scan_header_t fl_scan;
  
  Rfv3DecodeStormScan(params, scan, &fl_scan);

  fprintf(out, "%sScan number %d\n", spacer, scan->scan_num);
  fprintf(out, "%snbytes_char : %d\n", spacer, scan->nbytes_char);
  fprintf(out, "%snstorms : %d\n", spacer, scan->nstorms);
  fprintf(out, "%sTime: %s\n", spacer, utimestr((date_time_t *) &scan->time));
  fprintf(out, "%sDatum latitude (deg) : %g\n", spacer,
	  fl_scan.datum_latitude);
  fprintf(out, "%sDatum longitude (deg) : %g\n", spacer,
	  fl_scan.datum_longitude);
  fprintf(out, "%sMin z (km) : %g\n", spacer, fl_scan.min_z);
  fprintf(out, "%sDelta z (km) : %g\n", spacer, fl_scan.delta_z);
  fprintf(out, "\n");
  
  sprintf(buf, "%s  ", spacer);
  RfPrintCartParams(stdout, buf, &scan->cart);
  fprintf(out, "\n");
  
  return;

}

