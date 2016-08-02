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
/*********************************************************************
 * check_geom.c
 *
 * checks for consistency between the radar, cartesian table and
 * clutter table, as appropriate
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "polar2mdv.h"

#define CHECK_DISTANCE 0.50
#define MIN_RATIO 0.999
#define MAX_RATIO 1.001

void check_geom( rc_table_file_handle_t *rc_handle,
		clutter_table_file_handle_t *clutter_handle)

{
  
  int error_flag = FALSE;
  
  long ielev, iaz, ipoint, iplane, ilimit;
  si32 nplanes;
  si32 data_dbz_val, nclut_points;
  si32 dbz_field_pos;
  si32 **clutter_plane_heights;
  si32 **rc_plane_heights;
  
  double clutter_dbz;
  double check_ratio;
  double scalez;
  double data_lat, data_lon, table_lat, table_lon;
  double distance, theta;
  double clutter_scale, clutter_bias, data_scale, data_bias;
  double scale_diff, bias_diff;
 
  clutter_table_entry_t *clutter_entry;
  radar_params_t *rparams;
  rc_table_params_t *rc_params;
  clutter_table_params_t *clut_params;
  clutter_table_index_t **clut_index;
  rdata_shmem_header_t *shmem_header = get_shmem_header();
  field_params_t *field_params = get_field_params();
  scan_table_t *scan_table;
  
  /*
   * set local pointers
   */
  
  scan_table = rc_handle->scan_table;
  rparams = &shmem_header->radar;
  rc_params = rc_handle->table_params;
  clut_params = clutter_handle->table_params;
  clut_index = clutter_handle->table_index;
    
  rc_plane_heights = rc_handle->plane_heights;
  clutter_plane_heights = clutter_handle->plane_heights;

  /*
   * check the distance between the data radar location
   * and the radar-to-cart lookup table radar location
   */

  if (Glob->override_table_latlon) {
    
    Glob->radar_lat = ((double) rparams->latitude / DEG_FACTOR);
    Glob->radar_lon = ((double) rparams->longitude / DEG_FACTOR);

  } else {

    data_lat = ((double) rparams->latitude / DEG_FACTOR);
    data_lon = ((double) rparams->longitude / DEG_FACTOR);
    table_lat = ((double) rc_params->radar_latitude / DEG_FACTOR);
    table_lon = ((double) rc_params->radar_longitude / DEG_FACTOR);
    
    uLatLon2RTheta(data_lat, data_lon, table_lat, table_lon,
		   &distance, &theta);
    
    if (distance > CHECK_DISTANCE) {
      
      fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
      fprintf(stderr, "Radar positions do not match.\n");
      fprintf(stderr, "Data radar latitude = %12.7g\n", data_lat);
      fprintf(stderr, "Radar to cart. table radar latitude = %12.7g\n",
	      table_lat);
      fprintf(stderr, "Data radar longitude = %12.7g\n", data_lon);
      fprintf(stderr, "Radar to cart. table radar longitude = %12.7g\n",
	      table_lon);
      error_flag = TRUE;
      
    }
    
    Glob->radar_lat = table_lat;
    Glob->radar_lon = table_lon;

  }
  
  check_ratio = ((double) rparams->altitude / 
		 (double) rc_params->cart.radarz);
    
  if(check_ratio < MIN_RATIO || check_ratio > MAX_RATIO) {
    
    fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Radar altitude does not match.\n");
    fprintf(stderr, "Data radar altitude = %ld\n",
	    (long) rparams->altitude);
    fprintf(stderr, "Radar to cart. table radar altitude = %ld\n",
	    (long) rc_params->cart.radarz);
    error_flag = TRUE;
    
  }
    
  if(rparams->ngates != scan_table->ngates) {
    
    fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "No of gates do not match.\n");
    fprintf(stderr, "Radar volume ngates = %ld\n",
	    (long) rparams->ngates);
    fprintf(stderr, "Radar to cart. table ngates = %ld\n",
	    (long) scan_table->ngates);
    error_flag = TRUE;
      
  }
    
  if(fabs(rparams->gate_spacing / 1000000.0 -
	  scan_table->gate_spacing) > 0.5) {
      
    fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Gate spacing does not match.\n");
    fprintf(stderr, "Radar volume gate_spacing = %g\n",
	    rparams->gate_spacing / 1000000.0);
    fprintf(stderr, "Radar to cart. table gate_spacing = %g\n",
	    scan_table->gate_spacing);
    error_flag = TRUE;
      
  }

  if (fabs(rparams->start_range / 1000000.0 -
	   scan_table->start_range) > 0.001) {
      
    fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Start range does not match.\n");
    fprintf(stderr, "Radar volume start_range = %g\n",
	    rparams->start_range / 1000000.0);
    fprintf(stderr, "Radar to cart. table start_range = %g\n",
	    scan_table->start_range);
    error_flag = TRUE;
      
  }
    
  if (fabs(rparams->beam_width / DEG_FACTOR -
	   scan_table->beam_width) > 0.001) {
      
    fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Beam width does not match.\n");
    fprintf(stderr, "Radar volume beam_width = %g\n",
	    (double) rparams->beam_width / DEG_FACTOR);
    fprintf(stderr, "Radar to cart. table beam_width = %g\n",
	    (double) scan_table->beam_width);
    error_flag = TRUE;
    
  }
    
  if (Glob->remove_clutter) {
    
    /*
     * check that the radar to cart. parameters match the 
     * clutter table parameters
     */
    
    if (memcmp((void *) rc_params,
	       (void *) &clut_params->rc_params,
	       (size_t) sizeof(rc_table_params_t))) {
      
      fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
      fprintf(stderr,
	      "Radar to cart. params do not match clutter table params.\n");
      fprintf(stderr,
	      "Generate a new clutter map using the current rc table.\n");
      fprintf(stderr, "Run program 'clutter_table_generate'.\n");
      error_flag = TRUE;
    }

    /*
     * check that the plane height limits are the same for the
     * radar-to-cart table and the clutter table
     */

    nplanes = rc_params->cart.nz;
    scalez = (double) rc_params->cart.scalez;

    for (iplane = 0; iplane < nplanes; iplane++) {

      for (ilimit = 0; ilimit < N_PLANE_HEIGHT_VALUES; ilimit++) {

	if(rc_plane_heights[iplane][ilimit] !=
	   clutter_plane_heights[iplane][ilimit]) {
	  
	  fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
	  fprintf(stderr, "Plane height [%ld][%ld] does not match.\n",
		  iplane, ilimit);
	  fprintf(stderr, "Radar-to-cart table height = %g\n",
		  (double) rc_plane_heights[iplane][ilimit] / scalez);
	  fprintf(stderr, "Clutter table height = %g\n",
		  (double) clutter_plane_heights[iplane][ilimit] / scalez);
	  error_flag = TRUE;
	  
	} /* if (check_ratio ... */
	
      } /* ilimit */

    } /* iplane */
    
    /*
     * check that the scale and bias of the dbz field matches that used
     * to generate the clutter table. If not, adjust the
     * values accordingly.
     */
    
    clutter_scale = ((double) clut_params->dbz_scale /
		     (double) clut_params->factor);
    clutter_bias = ((double) clut_params->dbz_bias /
		    (double) clut_params->factor);
    
    dbz_field_pos = shmem_header->dbz_field_pos;
    
    data_scale = ((double) field_params[dbz_field_pos].scale /
		  (double) field_params[dbz_field_pos].factor);
    data_bias = ((double) field_params[dbz_field_pos].bias /
		 (double) field_params[dbz_field_pos].factor);
    
    scale_diff = fabs(clutter_scale - data_scale);
    bias_diff = fabs(clutter_bias - data_bias);
    
    /*
     * if the scale or bias differ, adjust the clutter table
     * to use the same scale as the data
     */
    
    if (scale_diff > 0.01 || bias_diff > 0.01) {
      
      for (ielev = 0; ielev < rparams->nelevations; ielev++) {
	
	for (iaz = 0; iaz < rparams->nazimuths; iaz++) {
	  
	  clutter_entry = clut_index[ielev][iaz].u.entry;
	  nclut_points = clut_index[ielev][iaz].nclut_points;
	  
	  for (ipoint = 0; ipoint < nclut_points; ipoint++) {
	    
	    clutter_dbz = (double) clutter_entry->dbz * clutter_scale +
	      clutter_bias;
	    
	    data_dbz_val = (si32) (((clutter_dbz - data_bias) /
				    data_scale) + 0.5);
	    
	    if (data_dbz_val < 0)
	      data_dbz_val = 0;
	    if (data_dbz_val > 255)
	      data_dbz_val = 255;
	    
	    clutter_entry->dbz = data_dbz_val;
	    
	    clutter_entry++;
	    
	  } /* ipoint */
	  
	} /* iaz */
	
      } /* ielev */
      
    } /* if (scale_diff....... */
    
  } /* if (Glob->remove_clutter) */
  
  if (error_flag == TRUE)
    exit(-1);
  
}
