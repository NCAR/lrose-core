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
/************************************************************************
 * update_range.c
 *
 * Writes range/dbz pairs to stdout for xgraph
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1992
 *
 ***********************************************************************/

#include "verify_grid.h"

#define MODERATE 0
#define HEAVY 32

#define N_ARSRS 12
#define MH_LAT 39.87823 
#define MH_LON -104.75900
#define RAD_TO_DEG 57.29577951308092
#define DEG_TO_RAD 0.01745329251994372

void update_range(char *detect_file_path)

{
  
  static vol_file_handle_t truth_index;
  static vol_file_handle_t detect_index;
  static int first_call = TRUE;
  
  char *truth_file_path;
  ui08 *detect_plane;
  
  si32 radar_id;
  si32 ix, iy;
  
  double x, y, dx, dy, minx, miny;
  double radarx, radary;
  double detect_scale, detect_bias;
  double detect_val;
  double arsr_lat[N_ARSRS], arsr_lon[N_ARSRS];
  double point_lat, point_lon;
  double mh_range, mh_theta;
  double arsr_range, arsr_theta;
  
  cart_params_t *truth_cart;
  cart_params_t *detect_cart;
  
  field_params_t *detect_field;
  
  /*
   * initialize
   */
  
  if (first_call) {
    
    init_indices(&truth_index, &detect_index);
    
    arsr_lat[0]  = 36.0756;
    arsr_lat[1]  = 37.5472;
    arsr_lat[2]  = 37.5931;
    arsr_lat[3]  = 37.6644;
    arsr_lat[4]  = 39.6386;
    arsr_lat[5]  = 39.5942;
    arsr_lat[6]  = 40.8328;
    arsr_lat[7]  = 41.1411;
    arsr_lat[8]  = 41.4347;
    arsr_lat[9]  = 42.5939;
    arsr_lat[10] = 44.8169;
    arsr_lat[11] = 45.0508;
    
    arsr_lon[0]  = -108.8597;
    arsr_lon[1]  = -104.0136;
    arsr_lon[2]  = -112.8631;
    arsr_lon[3]  = -100.8714;
    arsr_lon[4]  = -108.7619;
    arsr_lon[5]  = -104.6928;
    arsr_lon[6]  = -100.7478;
    arsr_lon[7]  = -98.8047;
    arsr_lon[8]  = -109.1167;
    arsr_lon[9]  = -104.5878;
    arsr_lon[10] = -107.9014;
    arsr_lon[11] = -99.9556;
    
    first_call = FALSE;
    
  } /* if (first_call) */
  
  /*
   * get the file path for the truth data - if this returns
   * error, there is no truth data within the time margin of
   * the detect data, so return early
   */
  
  if ((truth_file_path = get_truth_path(detect_file_path)) == NULL)
    return;
  
  if (Glob->debug) {
    
    fprintf(stderr, "Detection file '%s'\n", detect_file_path);
    fprintf(stderr, "Truth     file '%s'\n", truth_file_path);
    
  }
  
  /*
   * read in the data from the truth and detection files
   */
  
  detect_index.vol_file_path = detect_file_path;
  
  if (RfReadVolume(&detect_index, "update_range") != R_SUCCESS)
    tidy_and_exit(-1);
  
  truth_index.vol_file_path = truth_file_path;
  
  if (RfReadVolume(&truth_index, "update_range") != R_SUCCESS)
    tidy_and_exit(-1);
  
  /*
   * check that the cartesian grids match
   */
  
  truth_cart = &truth_index.vol_params->cart;
  detect_cart = &detect_index.vol_params->cart;
  
  if (truth_cart->nx != detect_cart->nx ||
      truth_cart->ny != detect_cart->ny) {
    
    fprintf(stderr, "ERROR - %s:update_range\n", Glob->prog_name);
    fprintf(stderr, "Cartesian grids do not match\n\n");
    
    fprintf(stderr, "\nFile '%s'\n", detect_file_path);
    print_cart_params(&detect_index.vol_params->cart);
    
    fprintf(stderr, "\nFile '%s'\n", truth_file_path);
    print_cart_params(&truth_index.vol_params->cart);
    
    tidy_and_exit(-1);
    
  }
  
  /*
   * check that there is only 1 plane
   */
  
  if (truth_cart->nz != 1) {
    
    fprintf(stderr, "ERROR - %s:update_range\n", Glob->prog_name);
    fprintf(stderr, "Truth data has %ld planes - should be 1\n",
	    truth_cart->nz);
    tidy_and_exit(-1);
    
  }
  
  if (detect_cart->nz != 1) {
    
    fprintf(stderr, "ERROR - %s:update_range\n", Glob->prog_name);
    fprintf(stderr, "Detection data has %ld planes - should be 1\n",
	    detect_cart->nz);
    tidy_and_exit(-1);
    
  }
  
  /*
   * check that field number is valid
   */
  
  if (Glob->truth_field > truth_index.vol_params->nfields - 1) {
    
    fprintf(stderr, "ERROR - %s:update_range\n", Glob->prog_name);
    fprintf(stderr, "Truth data field %ld too large\n",
	    Glob->truth_field);
    fprintf(stderr, "Max allowed is %ld\n",
	    truth_index.vol_params->nfields - 1);
    tidy_and_exit(-1);
    
  }
  
  if (Glob->detect_field > detect_index.vol_params->nfields - 1) {
    
    fprintf(stderr, "ERROR - %s:update_range\n", Glob->prog_name);
    fprintf(stderr, "Detect data field %ld too large\n",
	    Glob->detect_field);
    fprintf(stderr, "Max allowed is %ld\n",
	    detect_index.vol_params->nfields - 1);
    tidy_and_exit(-1);
    
  }
  
  /*
   * retrieve range data
   */
  
  detect_field = detect_index.field_params[Glob->detect_field];
  detect_scale =
    (double) detect_field->scale / (double) detect_field->factor;
  detect_bias =
    (double) detect_field->bias / (double) detect_field->factor;
  
  detect_plane = detect_index.field_plane[Glob->detect_field][0];
  
  dx = (double) truth_cart->dx / (double) truth_cart->km_scalex;
  dy = (double) truth_cart->dy / (double) truth_cart->km_scaley;
  minx = (double) truth_cart->minx / (double) truth_cart->km_scalex;
  miny = (double) truth_cart->miny / (double) truth_cart->km_scaley;
  radarx = (double) truth_cart->radarx / (double) truth_cart->km_scalex;
  radary = (double) truth_cart->radary / (double) truth_cart->km_scaley;

  y = miny - radary;

  for (iy = 0; iy < truth_cart->ny; iy++) {

    x = minx - radarx;

    for (ix = 0; ix < truth_cart->nx; ix++) {

      mh_range = sqrt(x * x + y * y);

      if (!Glob->check_range ||
	  mh_range <= Glob->max_range) {
 
	detect_val = (double) *detect_plane * detect_scale + detect_bias;
	
	if (detect_val >= Glob->detect_level_lower &&
	    detect_val <= Glob->detect_level_upper) {

	  /*
	   * get radar_id number
	   */

	  if (*detect_plane < HEAVY)
	    radar_id = *detect_plane - 1;
	  else
	    radar_id = *detect_plane - HEAVY - 1;

	  if (x == 0.0 && y == 0.0)
	    mh_theta = 0.0;
	  else
	    mh_theta = atan2(x, y) * RAD_TO_DEG;
	  
	  uLatLonPlusRTheta(MH_LAT, MH_LON, mh_range, mh_theta,
			    &point_lat, &point_lon);
	  
	  uLatLon2RTheta(arsr_lat[radar_id], arsr_lon[radar_id],
			 point_lat, point_lon,
			 &arsr_range, &arsr_theta);

/*	  fprintf(stdout, "x, y = %lg, %lg\n", x, y);
	  fprintf(stdout, "mh_range, mh_theta = %lg, %lg\n",
		  mh_range, mh_theta);
	  fprintf(stdout, "point_lat, point_lon = %lg, %lg\n",
		  point_lat, point_lon);
	  fprintf(stdout, "radar_id = %ld\n", radar_id + 1);
	  fprintf(stdout, "arsr_lat, arsr_lon = %lg, %lg\n",
		  arsr_lat[radar_id], arsr_lon[radar_id]);
	  fprintf(stdout, "arsr_range, arsr_theta = %lg, %lg\n",
		  arsr_range, arsr_theta);

	  fprintf(stdout, "%lg, %lg\n", arsr_range, truth_val); */

	} /* if (detect_val ... */

      } /* if (mh_range ... */

      detect_plane++;
      x += dx;

    } /* ix */

    y += dy;

  } /* iy */

}
