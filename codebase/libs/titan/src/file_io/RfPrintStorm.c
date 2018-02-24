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

#include <titan/storm.h>

#define BOOL_STR(a) (a == FALSE ? "false" : "true")

void RfPrintStormHeader(FILE *out,
			const char *spacer,
			const storm_file_header_t *header)
     
{

  char spacer2[128];

  sprintf(spacer2, "%s  ", spacer);
  
  /*
   * print out header
   */

  fprintf(out, "%sStorm file header :\n", spacer);

  fprintf(out, "%s  Major revision num : %ld\n", spacer,
	  (long) header->major_rev);
  fprintf(out, "%s  Minor revision num : %ld\n", spacer,
	  (long) header->minor_rev);
  
  fprintf(out, "%s  Dates and times : \n", spacer);
  fprintf(out, "%s    File   : %s\n",  spacer,
	  utimstr(header->file_time));
  fprintf(out, "%s    Start  : %s\n",  spacer,
	  utimstr(header->start_time));
  fprintf(out, "%s    End    : %s\n",  spacer,
	  utimstr(header->end_time));
  fprintf(out, "%s  Number of scans : %d\n", spacer,
	  header->n_scans);
  fprintf(out, "%s  Data file size  : %d\n", spacer,
	  header->data_file_size);
  fprintf(out, "\n");

  RfPrintStormParams(out, spacer2, &header->params);

  return;

}

/*-----------------------------
 */

void RfPrintStormHist(FILE *out,
                      const char *spacer,
		      const storm_file_params_t *params,
		      const storm_file_global_props_t *gprops,
		      const storm_file_dbz_hist_t *hist)
     
{

  si32 interval;
  const storm_file_dbz_hist_t *h = hist;

  if (gprops->n_dbz_intervals == 0) {
    return;
  }
    
  fprintf(out, "%sDbz histograms : \n", spacer);
  fprintf(out, "%s%10s %10s %12s %12s\n", spacer,
	  "Low dbz", "High Dbz", "% volume", "% area");
  
  for (interval = 0;
       interval < gprops->n_dbz_intervals; interval++, h++) {
    
    fprintf(out, "%s%10.1f %10.1f %12.2f %12.2f\n", spacer,
	    (params->low_dbz_threshold +
	     (double) interval * params->dbz_hist_interval),
	    (params->low_dbz_threshold +
	     (double) (interval + 1) * params->dbz_hist_interval),
	    h->percent_volume,
	    h->percent_area);
    
  } /* interval */

  fprintf(out, "\n");

  return;

}

/*-----------------------------
 */

void RfPrintStormHistXML(FILE *out,
                         const char *spacer,
                         const storm_file_params_t *params,
                         const storm_file_global_props_t *gprops,
                         const storm_file_dbz_hist_t *hist)
     
{

  si32 interval;
  const storm_file_dbz_hist_t *h = hist;

    /* return here if XML not yet set up */
    return;

  if (gprops->n_dbz_intervals == 0) {
    return;
  }
    
  fprintf(out, "%sDbz histograms : \n", spacer);
  fprintf(out, "%s%10s %10s %12s %12s\n", spacer,
	  "Low dbz", "High Dbz", "% volume", "% area");
  
  for (interval = 0;
       interval < gprops->n_dbz_intervals; interval++, h++) {
    
    fprintf(out, "%s%10.1f %10.1f %12.2f %12.2f\n", spacer,
	    (params->low_dbz_threshold +
	     (double) interval * params->dbz_hist_interval),
	    (params->low_dbz_threshold +
	     (double) (interval + 1) * params->dbz_hist_interval),
	    h->percent_volume,
	    h->percent_area);
    
  } /* interval */

  fprintf(out, "\n");

  return;

}

/*-------------------------------
 */

void RfPrintStormLayer(FILE *out,
		       const char *spacer,
		       const storm_file_params_t *params,
		       const storm_file_scan_header_t *scan,
		       const storm_file_global_props_t *gprops,
		       const storm_file_layer_props_t *layer)
     
{

  char *loc_label;
  const storm_file_layer_props_t *l = layer;
    
  int ilayer;
  
  if (gprops->n_layers == 0) {
    return;
  }
    
  if (scan->grid.proj_type == TITAN_PROJ_FLAT) {
    loc_label = "(km) ";
  } else {
    loc_label = "(deg)";
  }

  /*
   * layer properties
   */
  
  fprintf(out, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s\n",
	  spacer,
	  "Layer", "z", "x-cent", "y-cent", "x-Zcent", "y-Zcent", "area",
	  "max Z", "mean Z");
  fprintf(out, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s\n",
	  spacer, " ", "(km)", loc_label, loc_label, loc_label, loc_label,
	  "(km2)", "(dbz)", "(dbz)");
  
  for (ilayer = gprops->base_layer;
       ilayer < (gprops->base_layer + gprops->n_layers);
       ilayer++, l++) {
    
    fprintf(out, "%s%5d %5g %7.1f %7.1f %7.1f %7.1f "
	    "%6.1f %6.1f %7.1f\n",
	    spacer, ilayer,
	    (scan->min_z + (double) ilayer * scan->delta_z),
	    l->vol_centroid_x,
	    l->vol_centroid_y,
	    l->refl_centroid_x,
	    l->refl_centroid_y,
	    l->area,
	    l->dbz_max,
	    l->dbz_mean);
    
  } /* ilayer */
  
  fprintf(out, "\n");
  
  fprintf(out, "%s%5s %5s %7s %9s %7s %12s\n", spacer,
	  "Layer", "z", "mass", "rvel_mean", "rvel_sd", "vorticity");
  fprintf(out, "%s%5s %5s %7s %9s %7s %12s\n",
	  spacer, " ", "(km)", "(ktons)", "(m/s)", "(m/s)", "(/s)");
  
  l = layer;
    
  for (ilayer = gprops->base_layer;
       ilayer < (gprops->base_layer + gprops->n_layers);
       ilayer++, l++) {
    
    fprintf(out, "%s%5d %5g %7.1f %9.2f %7.2f %12.2e\n",
	    spacer, ilayer,
	    (scan->min_z + (double) ilayer * scan->delta_z),
	    l->mass,
	    l->rad_vel_mean,
	    l->rad_vel_sd,
	    l->vorticity);
    
  } /* ilayer */
  
  fprintf(out, "\n");
  
  return;

}

/*-------------------------------
 */

void RfPrintStormLayerXML(FILE *out,
                          const char *spacer,
                          const storm_file_params_t *params,
                          const storm_file_scan_header_t *scan,
                          const storm_file_global_props_t *gprops,
                          const storm_file_layer_props_t *layer)
     
{

  char *loc_label;
  const storm_file_layer_props_t *l = layer;
    
  int ilayer;
  
  /* return here if XML not yet set up */
    return;

  if (gprops->n_layers == 0) {
    return;
  }
    
  if (scan->grid.proj_type == TITAN_PROJ_FLAT) {
    loc_label = "km ";
  } else {
    loc_label = "deg";
  }

  /*
   * layer properties
   */
  
  fprintf(out, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s\n",
	  spacer,
	  "Layer", "z", "x-cent", "y-cent", "x-Zcent", "y-Zcent", "area",
	  "max Z", "mean Z");
  fprintf(out, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s\n",
	  spacer, " ", "(km)", loc_label, loc_label, loc_label, loc_label,
	  "(km2)", "(dbz)", "(dbz)");
  
  for (ilayer = gprops->base_layer;
       ilayer < (gprops->base_layer + gprops->n_layers);
       ilayer++, l++) {
    
    fprintf(out, "%s%5d %5g %7.1f %7.1f %7.1f %7.1f "
	    "%6.1f %6.1f %7.1f\n",
	    spacer, ilayer,
	    (scan->min_z + (double) ilayer * scan->delta_z),
	    l->vol_centroid_x,
	    l->vol_centroid_y,
	    l->refl_centroid_x,
	    l->refl_centroid_y,
	    l->area,
	    l->dbz_max,
	    l->dbz_mean);
    
  } /* ilayer */
  
  fprintf(out, "\n");
  
  fprintf(out, "%s%5s %5s %7s %9s %7s %12s\n", spacer,
	  "Layer", "z", "mass", "rvel_mean", "rvel_sd", "vorticity");
  fprintf(out, "%s%5s %5s %7s %9s %7s %12s\n",
	  spacer, " ", "(km)", "(ktons)", "(m/s)", "(m/s)", "(/s)");
  
  l = layer;
    
  for (ilayer = gprops->base_layer;
       ilayer < (gprops->base_layer + gprops->n_layers);
       ilayer++, l++) {
    
    fprintf(out, "%s%5d %5g %7.1f %9.2f %7.2f %12.2e\n",
	    spacer, ilayer,
	    (scan->min_z + (double) ilayer * scan->delta_z),
	    l->mass,
	    l->rad_vel_mean,
	    l->rad_vel_sd,
	    l->vorticity);
    
  } /* ilayer */
  
  fprintf(out, "\n");
  
  return;

}

/*--------------------------------
 */

void RfPrintStormParams(FILE *out,
			const char *spacer,
			const storm_file_params_t *params)
     
{

  fprintf(out, "%sStorm file parameters : \n", spacer);

  fprintf(out, "%s  Low dBZ threshold : %g\n", spacer,
	  params->low_dbz_threshold);
  fprintf(out, "%s  High dBZ threshold : %g\n", spacer,
	  params->high_dbz_threshold);
  fprintf(out, "%s  Hail dBZ threshold : %g\n", spacer,
	  params->hail_dbz_threshold);
  fprintf(out, "%s  Hail mass dBZ threshold : %g\n", spacer,
	  params->hail_mass_dbz_threshold);
  fprintf(out, "%s  Dbz hist interval : %g\n", spacer,
	  params->dbz_hist_interval);
  fprintf(out, "%s  Top threshold (km) : %g\n", spacer,
	  params->top_threshold);
  fprintf(out, "%s  Base threshold (km) : %g\n", spacer,
	  params->base_threshold);
  fprintf(out, "%s  Min storm size (km2 or km3) : %g\n", spacer,
	  params->min_storm_size);
  fprintf(out, "%s  Max storm size (km2 or km3) : %g\n", spacer,
	  params->max_storm_size);
  
  fprintf(out, "%s  Check morphology? : %s\n", spacer,
	  BOOL_STR(params->check_morphology));
  fprintf(out, "%s  Morphology_erosion_threshold (km) : %g\n", spacer,
	  params->morphology_erosion_threshold);
  fprintf(out, "%s  Morphology_refl_divisor (dbz/km) : %g\n", spacer,
	  params->morphology_refl_divisor);

  fprintf(out, "%s  Check tops? : %s\n", spacer,
	  BOOL_STR(params->check_tops));
  fprintf(out, "%s  Min_radar_tops (km) : %g\n", spacer,
	  params->min_radar_tops);
  fprintf(out, "%s  Tops_edge_margin (km) : %g\n", spacer,
	  params->tops_edge_margin);
  
  fprintf(out, "%s  Z-R coefficient : %g\n", spacer,
	  params->z_p_coeff);
  fprintf(out, "%s  Z-R exponent : %g\n", spacer,
	  params->z_p_exponent);
  fprintf(out, "%s  Z-M coefficient : %g\n", spacer,
	  params->z_m_coeff);
  fprintf(out, "%s  Z-M exponent : %g\n", spacer,
	  params->z_m_exponent);
  fprintf(out, "%s  Hail Z-M coefficient : %g\n", spacer,
	  params->hail_z_m_coeff);
  fprintf(out, "%s  Hail Z-M exponent : %g\n", spacer,
	  params->hail_z_m_exponent);

  fprintf(out, "%s  Sectrip vert aspect : %g\n", spacer,
	  params->sectrip_vert_aspect);
  fprintf(out, "%s  Sectrip horiz aspect : %g\n", spacer,
	  params->sectrip_horiz_aspect);
  fprintf(out, "%s  Sectrip orientation error : %g\n", spacer,
	  params->sectrip_orientation_error);

  fprintf(out, "%s  Velocity data available? : %s\n", spacer,
	  BOOL_STR(params->vel_available));

  fprintf(out, "%s  N_poly_sides : %ld\n", spacer,
	  (long) params->n_poly_sides);
  fprintf(out, "%s  Poly_start_az : %g\n", spacer,
	  params->poly_start_az);
  fprintf(out, "%s  Poly_delta_az : %g\n", spacer,
	  params->poly_delta_az);
    
  switch( params->gprops_union_type ) {
    case UNION_HAIL:
         fprintf(out, "%s  Storm properties union type : HAIL\n", spacer );
         break;
    case UNION_NEXRAD_HDA:
         fprintf(out, "%s  Storm properties union type : NEXRAD HDAL\n",
                 spacer );
         break;
    default:
         fprintf(out, "%s  Storm properties union type : NONE\n", spacer );
         break;
  }

  if (params->tops_dbz_threshold == 0.0) {
    fprintf(out, "%s  Tops dBZ threshold : %g\n", spacer,
	    params->low_dbz_threshold);
  } else {
    fprintf(out, "%s  Tops dBZ threshold : %g\n", spacer,
	    params->tops_dbz_threshold);
  }

  if (params->precip_computation_mode == TITAN_PRECIP_AT_LOWEST_VALID_HT) {
    fprintf(out, "%s  Precip computed for lowest valid CAPPI ht\n", spacer);
  } else if (params->precip_computation_mode == TITAN_PRECIP_AT_SPECIFIED_HT) {
    fprintf(out, "%s  Precip computed from specified ht (km): %g\n",
            spacer, params->precip_plane_ht);
  } else if (params->precip_computation_mode == TITAN_PRECIP_FROM_LOWEST_AVAILABLE_REFL) {
    fprintf(out, "%s  Precip computed from lowest available reflectivity\n",
            spacer);
  } else {
    fprintf(out, "%s  Precip computed from column max\n", spacer);
  }
  
  fprintf(out, "\n");

  return;

}

/*-------------------------------
 */

void RfPrintStormProps(FILE *out,
		       const char *spacer,
		       const storm_file_params_t *params,
		       const storm_file_scan_header_t *scan,
		       const storm_file_global_props_t *gprops)
     
{

  char *loc_label;
  si32 i;
  
  if (scan->grid.proj_type == TITAN_PROJ_FLAT) {
    loc_label = "(km) ";
  } else {
    loc_label = "(deg)";
  }

  fprintf(out, "%sGLOBAL STORM PROPERTIES - storm number %ld\n",
	  spacer, (long) gprops->storm_num);
  
  fprintf(out, "%s  number of layers                : %ld\n", spacer,
	  (long) gprops->n_layers);
  fprintf(out, "%s  base layer number               : %ld\n", spacer,
	  (long) gprops->base_layer);
  fprintf(out, "%s  number of dbz intervals         : %ld\n", spacer,
	  (long) gprops->n_dbz_intervals);
  fprintf(out, "%s  number of runs                  : %ld\n", spacer,
	  (long) gprops->n_runs);
  fprintf(out, "%s  number of proj runs             : %ld\n", spacer,
	  (long) gprops->n_proj_runs);
  
  fprintf(out, "%s  range_limited                   : %ld\n", spacer,
	  (long) gprops->range_limited);
  fprintf(out, "%s  top_missing                     : %ld\n", spacer,
	  (long) gprops->top_missing);
  fprintf(out, "%s  second_trip                     : %ld\n", spacer,
	  (long) gprops->second_trip);
  fprintf(out, "%s  hail_present                    : %ld\n", spacer,
	  (long) gprops->hail_present);
  fprintf(out, "%s  anom_prop                       : %ld\n", spacer,
	  (long) gprops->anom_prop);
  
  fprintf(out, "%s  vol_centroid_x %s            : %g\n", spacer,
	  loc_label, gprops->vol_centroid_x);
  fprintf(out, "%s  vol_centroid_y %s            : %g\n", spacer,
	  loc_label, gprops->vol_centroid_y);
  fprintf(out, "%s  vol_centroid_z (km)             : %g\n", spacer,
	  gprops->vol_centroid_z);
  
  fprintf(out, "%s  refl_centroid_x %s           : %g\n", spacer,
	  loc_label, gprops->refl_centroid_x);
  fprintf(out, "%s  refl_centroid_y %s           : %g\n", spacer,
	  loc_label, gprops->refl_centroid_y);
  fprintf(out, "%s  refl_centroid_z (km)            : %g\n", spacer,
	  gprops->refl_centroid_z);
  
  fprintf(out, "%s  top (km)                        : %g\n", spacer,
	  gprops->top);
  fprintf(out, "%s  base (km)                       : %g\n", spacer,
	  gprops->base);
  fprintf(out, "%s  volume (km3)                    : %g\n", spacer,
	  gprops->volume);
  fprintf(out, "%s  mean area (km2)                 : %g\n", spacer,
	  gprops->area_mean);
  fprintf(out, "%s  precip flux (m3/s)              : %g\n", spacer,
	  gprops->precip_flux);
  fprintf(out, "%s  mass (ktons)                    : %g\n", spacer,
	  gprops->mass);
  fprintf(out, "%s  vil_from_maxz(kg/m2)            : %g\n", spacer,
	  gprops->vil_from_maxz);
  
  if ( params->gprops_union_type == UNION_HAIL ) {
    fprintf(out, "%s  vihm_from_maxz (kg/m2)          : %g\n", spacer,
  	    gprops->add_on.hail_metrics.vihm);
    fprintf(out, "%s  Hail mass aloft (ktons)         : %g\n", spacer,
  	    gprops->add_on.hail_metrics.hailMassAloft);
    fprintf(out, "%s  Waldvogel probability of hail   : %g\n", spacer,
  	    gprops->add_on.hail_metrics.waldvogelProbability);
    fprintf(out, "%s  FOKR storm category          : %d\n", spacer,
  	    gprops->add_on.hail_metrics.FOKRcategory);
  } else if ( params->gprops_union_type == UNION_NEXRAD_HDA ) {
    fprintf(out, "%s  POH       (%%) : %g\n", spacer, gprops->add_on.hda.poh);
    fprintf(out, "%s  SHI (Jm-1s-1) : %g\n", spacer, gprops->add_on.hda.shi);
    fprintf(out, "%s  POSH      (%%) : %g\n", spacer, gprops->add_on.hda.posh);
    fprintf(out, "%s  MEHS     (mm) : %g\n", spacer, gprops->add_on.hda.mehs);
  }
  
  fprintf(out, "%s  tilt angle (deg)                : %g\n", spacer,
	  gprops->tilt_angle);
  fprintf(out, "%s  tilt direction (deg)            : %g\n", spacer,
	  gprops->tilt_dirn);
  
  fprintf(out, "%s  dbz max                         : %g\n", spacer,
	  gprops->dbz_max);
  fprintf(out, "%s  dbz mean                        : %g\n", spacer,
	  gprops->dbz_mean);
  fprintf(out, "%s  dbz max gradient                : %g\n", spacer,
	  gprops->dbz_max_gradient);
  fprintf(out, "%s  dbz mean gradient               : %g\n", spacer,
	  gprops->dbz_mean_gradient);
  fprintf(out, "%s  height of max dbz               : %g\n", spacer,
	  gprops->ht_of_dbz_max);
  
  fprintf(out, "%s  rad_vel_mean (m/s)              : %g\n", spacer,
	  gprops->rad_vel_mean);
  fprintf(out, "%s  rad_vel_sd (m/s)                : %g\n", spacer,
	  gprops->rad_vel_sd);
  fprintf(out, "%s  vorticity (/s)                  : %.2e\n", spacer,
	  gprops->vorticity);
  
  fprintf(out, "%s  precip area (km2)               : %g\n", spacer,
	  gprops->precip_area);
  fprintf(out, "%s  precip area centroid x %s    : %g\n", spacer,
	  loc_label,
	  gprops->precip_area_centroid_x);
  fprintf(out, "%s  precip area centroid y %s    : %g\n", spacer,
	  loc_label,
	  gprops->precip_area_centroid_y);
  fprintf(out, "%s  precip area orientation (deg)   : %g\n", spacer,
	  gprops->precip_area_orientation);
  fprintf(out, "%s  precip area minor radius %s  : %g\n", spacer,
	  loc_label,
	  gprops->precip_area_minor_radius);
  fprintf(out, "%s  precip area major radius %s  : %g\n", spacer,
	  loc_label,
	  gprops->precip_area_major_radius);
  
  fprintf(out, "%s  proj. area (km2)                : %g\n", spacer,
	  gprops->proj_area);
  fprintf(out, "%s  proj. area centroid x %s     : %g\n", spacer,
	  loc_label,
	  gprops->proj_area_centroid_x);
  fprintf(out, "%s  proj. area centroid y %s     : %g\n", spacer,
	  loc_label,
	  gprops->proj_area_centroid_y);
  fprintf(out, "%s  proj. area orientation (deg)    : %g\n", spacer,
	  gprops->proj_area_orientation);
  fprintf(out, "%s  proj. area minor radius %s   : %g\n", spacer,
	  loc_label,
	  gprops->proj_area_minor_radius);
  fprintf(out, "%s  proj. area major radius %s   : %g\n", spacer,
	  loc_label,
	  gprops->proj_area_major_radius);
  
  fprintf(out, "%s  bounding_min_ix                 : %ld\n", spacer,
	  (long) gprops->bounding_min_ix);
  fprintf(out, "%s  bounding_min_iy                 : %ld\n", spacer,
	  (long) gprops->bounding_min_iy);
  fprintf(out, "%s  bounding_max_ix                 : %ld\n", spacer,
	  (long) gprops->bounding_max_ix);
  fprintf(out, "%s  bounding_max_iy                 : %ld\n", spacer,
	  (long) gprops->bounding_max_iy);
  
  fprintf(out, "%s\n    Proj. area polygon rays:\n", spacer);
  for (i = 0; i < N_POLY_SIDES; i++) {
    fprintf(out, "%s    side %ld : %g\n", spacer,
	    (long) i, gprops->proj_area_polygon[i]);
  }
  
  fprintf(out, "\n");

  return;

}

/*-------------------------------
 */

void RfPrintStormPropsXML(FILE *out,
                          const char *spacer,
                          const storm_file_params_t *params,
                          const storm_file_scan_header_t *scan,
                          const storm_file_global_props_t *gprops)
     
{
  char *loc_label;
  si32 i;
  si32 num_sides = N_POLY_SIDES;
  
  
  if (scan->grid.proj_type == TITAN_PROJ_FLAT) {
    loc_label = "km ";
  } else {
    loc_label = "deg";
  }

  fprintf(out, "%s  <storm_number> %ld </storm_number>\n",
	  spacer, (long) gprops->storm_num);
  
  fprintf(out, "%s  <number_of_layers> %ld </number_of_layers>\n", spacer,
	  (long) gprops->n_layers);
  fprintf(out, "%s  <base_layer_number> %ld </base_layer_number> \n", spacer,
	  (long) gprops->base_layer);
  fprintf(out, "%s  <number_of_dbz_intervals> %ld </number_of_dbz_intervals> \n", spacer,
	  (long) gprops->n_dbz_intervals);
  fprintf(out, "%s  <number_of_runs> %ld </number_of_runs> \n", spacer,
	  (long) gprops->n_runs);
  fprintf(out, "%s  <number_of_proj_runs> %ld </number_of_proj_runs> \n", spacer,
	  (long) gprops->n_proj_runs);
  
  fprintf(out, "%s  <range_limited> %ld </range_limited> \n", spacer,
	  (long) gprops->range_limited);
  fprintf(out, "%s  <top_missing> %ld </top_missing> \n", spacer,
	  (long) gprops->top_missing);
  fprintf(out, "%s  <second_trip> %ld </second_trip> \n", spacer,
	  (long) gprops->second_trip);
  fprintf(out, "%s  <hail_present> %ld </hail_present> \n", spacer,
	  (long) gprops->hail_present);
  fprintf(out, "%s  <anom_prop> %ld </anom_prop> \n", spacer,
	  (long) gprops->anom_prop);
  
  fprintf(out, "%s  <vol_centroid_x unit=\"%s\"> %g </vol_centroid_x> \n", spacer,
	  loc_label, gprops->vol_centroid_x);
  fprintf(out, "%s  <vol_centroid_y unit=\"%s\"> %g </vol_centroid_y> \n", spacer,
	  loc_label, gprops->vol_centroid_y);
  fprintf(out, "%s  <vol_centroid_z unit=\"km\"> %g </vol_centroid_z> \n", spacer,
	  gprops->vol_centroid_z);
  
  fprintf(out, "%s  <refl_centroid_x unit=\"%s\"> %g </refl_centroid_x> \n", spacer,
	  loc_label, gprops->refl_centroid_x);
  fprintf(out, "%s  <refl_centroid_y unit=\"%s\"> %g </refl_centroid_y> \n", spacer,
	  loc_label, gprops->refl_centroid_y);
  fprintf(out, "%s  <refl_centroid_z unit=\"km\"> %g </refl_centroid_z> \n", spacer,
	  gprops->refl_centroid_z);
  
  fprintf(out, "%s  <top unit=\"km\"> %g </top> \n", spacer,
	  gprops->top);
  fprintf(out, "%s  <base unit=\"km\"> %g </base> \n", spacer,
	  gprops->base);
  fprintf(out, "%s  <volume unit=\"km3\"> %g </volume> \n", spacer,
	  gprops->volume);
  fprintf(out, "%s  <mean_area unit=\"km2\"> %g </mean_area> \n", spacer,
	  gprops->area_mean);
  fprintf(out, "%s  <precip_flux unit=\"m3/s\"> %g </precip_flux> \n", spacer,
	  gprops->precip_flux);
  fprintf(out, "%s  <mass unit=\"ktons\"> %g </mass> \n", spacer,
	  gprops->mass);
  fprintf(out, "%s  <vil unit=\"kg/m2\"> %g </vil> \n", spacer,
	  gprops->vil_from_maxz);
  
  fprintf(out, "%s  <tilt_angle unit=\"deg\"> %g </tilt_angle> \n", spacer,
	  gprops->tilt_angle);
  fprintf(out, "%s  <tilt_direction unit=\"deg\"> %g </tilt_direction> \n", spacer,
	  gprops->tilt_dirn);
  
  fprintf(out, "%s  <dbz_max> %g </dbz_max>\n", spacer,
	  gprops->dbz_max);
  fprintf(out, "%s  <dbz_mean> %g </dbz_mean> \n", spacer,
	  gprops->dbz_mean);
  fprintf(out, "%s  <dbz_max_gradient> %g </dbz_max_gradient> \n", spacer,
	  gprops->dbz_max_gradient);
  fprintf(out, "%s  <dbz_mean_gradient> %g </dbz_mean_gradient> \n", spacer,
	  gprops->dbz_mean_gradient);
  fprintf(out, "%s  <ht_max_dbz> %g </ht_max_dbz> \n", spacer,
	  gprops->ht_of_dbz_max);
  
  fprintf(out, "%s  <rad_vel_mean unit=\"m/s\"> %g </rad_vel_mean> \n", spacer,
	  gprops->rad_vel_mean);
  fprintf(out, "%s  <rad_vel_sd unit=\"m/s\"> %g </rad_vel_sd> \n", spacer,
	  gprops->rad_vel_sd);
  fprintf(out, "%s  <vorticity unit=\"s-1\"> %g </vorticity> \n", spacer,
	  gprops->vorticity);
  
  fprintf(out, "%s  <precip_area unit=\"km2\"> %g </precip_area> \n", spacer,
	  gprops->precip_area);
  fprintf(out, "%s  <precip_area_centroid_x unit=\"%s\"> %g </precip_area_centroid_x> \n", spacer,
	  loc_label,
	  gprops->precip_area_centroid_x);
  fprintf(out, "%s  <precip_area_centroid_y unit=\"%s\"> %g </precip_area_centroid_y> \n", spacer,
	  loc_label,
	  gprops->precip_area_centroid_y);
  fprintf(out, "%s  <precip_area_orientation unit=\"deg\"> %g </precip_area_orientation> \n", spacer,
	  gprops->precip_area_orientation);
  fprintf(out, "%s  <precip_area_minor_radius unit=\"%s\"> %g </precip_area_minor_radius> \n", spacer,
	  loc_label,
	  gprops->precip_area_minor_radius);
  fprintf(out, "%s  <precip_area_major_radius unit=\"%s\"> %g </precip_area_major_radius> \n", spacer,
	  loc_label,
	  gprops->precip_area_major_radius);
  
  fprintf(out, "%s  <proj_area unit=\"km2\"> %g </proj_area>\n", spacer,
	  gprops->proj_area);
  fprintf(out, "%s  <proj_area_centroid_x unit=\"%s\"> %g </proj_area_centroid_x>\n", spacer,
	  loc_label,
	  gprops->proj_area_centroid_x);
  fprintf(out, "%s  <proj_area_centroid_y unit=\"%s\"> %g </proj_area_centroid_y>\n", spacer,
	  loc_label,
	  gprops->proj_area_centroid_y);
  fprintf(out, "%s  <proj_area_orientation unit=\"deg\"> %g </proj_area_orientation>\n", spacer,
	  gprops->proj_area_orientation);
  fprintf(out, "%s  <proj_area_minor_radius unit=\"%s\"> %g </proj_area_minor_radius>\n", spacer,
	  loc_label,
	  gprops->proj_area_minor_radius);
  fprintf(out, "%s  <proj_area_major_radius unit=\"%s\"> %g </proj_area_major_radius>\n", spacer,
	  loc_label,
	  gprops->proj_area_major_radius);
  
  fprintf(out, "%s  <bounding_min_ix> %ld </bounding_min_ix>\n", spacer,
	  (long) gprops->bounding_min_ix);
  fprintf(out, "%s  <bounding_min_iy> %ld </bounding_min_iy>\n", spacer,
	  (long) gprops->bounding_min_iy);
  fprintf(out, "%s  <bounding_max_ix> %ld </bounding_max_ix>\n", spacer,
	  (long) gprops->bounding_max_ix);
  fprintf(out, "%s  <bounding_max_iy> %ld </bounding_max_iy>\n", spacer,
	  (long) gprops->bounding_max_iy);
  
  fprintf(out, "%s  <num_polygon_rays> %d </num_polygon_rays>\n",
	  spacer,num_sides);
  fprintf(out, "%s  <proj_area_polygon_rays>\n", spacer);
  for (i = 0; i < N_POLY_SIDES; i++) {
    fprintf(out, "%s     <side num=\"%ld\"> %g </side>\n", spacer,
	    (long) i, gprops->proj_area_polygon[i]);
  }
  fprintf(out, "%s  </proj_area_polygon_rays>\n\n", spacer);

  return;

}

/*-----------------------------
 */

void RfPrintStormRuns(FILE *out,
		      const char *spacer,
		      const storm_file_global_props_t *gprops,
		      const storm_file_run_t *runs)
     
{

  si32 irun;
  const storm_file_run_t *run;

  if (gprops->n_runs == 0) {
    return;
  }

  fprintf(out, "%sRuns\n", spacer);
  fprintf(out, "%s%8s %8s %8s %8s %8s\n", spacer,
	  "ix", "n", "maxx", "iy", "iz");
  
  run = runs;
  for (irun = 0; irun < gprops->n_runs; irun++, run++) {
    fprintf(out, "%s%8ld %8ld %8ld %8ld %8ld",
	    spacer,
	    (long) run->ix, (long) run->n,
	    (long) (run->ix + run->n - 1),
	    (long) run->iy, (long) run->iz);
    if (run->ix < gprops->bounding_min_ix ||
	(run->ix + run->n - 1) > gprops->bounding_max_ix ||
	run->iy < gprops->bounding_min_iy ||
	run->iy > gprops->bounding_max_iy) {
      fprintf(out, "\a *** Bounds exceeded");
    }
    fprintf(out, "\n");
  } /* irun */
  
  fprintf(out, "\n");

  return;
  
}

/*-----------------------------
 */

void RfPrintStormRunsXML(FILE *out,
                         const char *spacer,
                         const storm_file_global_props_t *gprops,
                         const storm_file_run_t *runs)
     
{

  si32 irun;
  const storm_file_run_t *run;

  /* return here if XML not yet set up */
    return;

  if (gprops->n_runs == 0) {
    return;
  }

  fprintf(out, "%sRuns\n", spacer);
  fprintf(out, "%s%8s %8s %8s %8s %8s\n", spacer,
	  "ix", "n", "maxx", "iy", "iz");
  
  run = runs;
  for (irun = 0; irun < gprops->n_runs; irun++, run++) {
    fprintf(out, "%s%8ld %8ld %8ld %8ld %8ld",
	    spacer,
	    (long) run->ix, (long) run->n,
	    (long) (run->ix + run->n - 1),
	    (long) run->iy, (long) run->iz);
    if (run->ix < gprops->bounding_min_ix ||
	(run->ix + run->n - 1) > gprops->bounding_max_ix ||
	run->iy < gprops->bounding_min_iy ||
	run->iy > gprops->bounding_max_iy) {
      fprintf(out, "\a *** Bounds exceeded");
    }
    fprintf(out, "\n");
  } /* irun */
  
  fprintf(out, "\n");

  return;
  
}

/*-----------------------------
 */

void RfPrintStormProjRuns(FILE *out,
			  const char *spacer,
			  const storm_file_global_props_t *gprops,
			  const storm_file_run_t *proj_runs)
     
{

  si32 irun;
  const storm_file_run_t *run;
  
  if (gprops->n_proj_runs == 0) {
    return;
  }

  fprintf(out, "%sProj area runs\n", spacer);
  fprintf(out, "%s%8s %8s %8s %8s %8s\n", spacer,
	  "ix", "n", "maxx", "iy", "iz");
  
  run = proj_runs;
  for (irun = 0; irun < gprops->n_proj_runs; irun++, run++) {
    fprintf(out, "%s%8ld %8ld %8ld %8ld %8ld",
	    spacer,
	    (long) run->ix, (long) run->n,
	    (long) (run->ix + run->n - 1),
	    (long) run->iy, (long) run->iz);
    if (run->ix < gprops->bounding_min_ix ||
	(run->ix + run->n - 1) > gprops->bounding_max_ix ||
	run->iy < gprops->bounding_min_iy ||
	run->iy > gprops->bounding_max_iy) {
      fprintf(out, "\a *** Bounds exceeded");
    }
    fprintf(out, "\n");
  } /* irun */
  
  fprintf(out, "\n");

  return;
  
}

/*-----------------------------
 */

void RfPrintStormProjRunsXML(FILE *out,
                             const char *spacer,
                             const storm_file_global_props_t *gprops,
                             const storm_file_run_t *proj_runs)
     
{
  si32 irun;
  const storm_file_run_t *run;
  
  /* return here if XML not yet set up */
    return;
    
  if (gprops->n_proj_runs == 0) {
    return;
  }

  fprintf(out, "%sProj area runs\n", spacer);
  fprintf(out, "%s%8s %8s %8s %8s %8s\n", spacer,
	  "ix", "n", "maxx", "iy", "iz");
  
  run = proj_runs;
  for (irun = 0; irun < gprops->n_proj_runs; irun++, run++) {
    fprintf(out, "%s%8ld %8ld %8ld %8ld %8ld",
	    spacer,
	    (long) run->ix, (long) run->n,
	    (long) (run->ix + run->n - 1),
	    (long) run->iy, (long) run->iz);
    if (run->ix < gprops->bounding_min_ix ||
	(run->ix + run->n - 1) > gprops->bounding_max_ix ||
	run->iy < gprops->bounding_min_iy ||
	run->iy > gprops->bounding_max_iy) {
      fprintf(out, "\a *** Bounds exceeded");
    }
    fprintf(out, "\n");
  } /* irun */
  
  fprintf(out, "\n");

  return;
  
}

/*-----------------------------
 */

void RfPrintStormScan(FILE *out,
		      const char *spacer,
		      const storm_file_params_t *params,
		      const storm_file_scan_header_t *scan)
     
{

  char buf[128];

  fprintf(out, "%sScan number %d\n", spacer, scan->scan_num);
  fprintf(out, "%snbytes_char : %d\n", spacer, scan->nbytes_char);
  fprintf(out, "%snstorms : %d\n", spacer, scan->nstorms);
  fprintf(out, "%sTime: %s\n", spacer, utimestr(udate_time(scan->time)));
  fprintf(out, "%sMin z (km) : %g\n", spacer, scan->min_z);
  fprintf(out, "%sDelta z (km) : %g\n", spacer, scan->delta_z);
  fprintf(out, "%sHeight of freezing (km) : %g\n", spacer, scan->ht_of_freezing);
  fprintf(out, "\n");
  
  sprintf(buf, "%s  ", spacer);
  TITAN_print_grid(out, buf, &scan->grid);
  fprintf(out, "\n");
  
  return;

}

void RfPrintStormScanXML(FILE *out,
                         const char *spacer,
                         const storm_file_params_t *params,
                         const storm_file_scan_header_t *scan)
     
{

  char buf[128];

  fprintf(out, "%s<scan_number> %d </scan_number>\n", spacer, scan->scan_num);
  fprintf(out, "%s<nbytes_char> %d </nbytes_char>\n", spacer, scan->nbytes_char);
  fprintf(out, "%s<nstorms> %d </nstorms>\n", spacer, scan->nstorms);
  fprintf(out, "%s<time_storm><unixtime> %d </unixtime></time_storm>\n", spacer, scan->time);
  fprintf(out, "%s<min_z unit=\"km\"> %g </min_z>\n", spacer, scan->min_z);
  fprintf(out, "%s<delta_z unit=\"km\"> %g </delta_z>\n", spacer, scan->delta_z);
  fprintf(out, "\n");
  
  sprintf(buf, "%s  ", spacer);
  TITAN_print_gridXML(out, buf, &scan->grid);
  fprintf(out, "\n");
  
  return;

}

