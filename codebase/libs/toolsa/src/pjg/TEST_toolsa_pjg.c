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
/*
 * Name: TEST_toolsa_pjg.c
 *
 * Purpose:
 *
 *      To test the PJG module in the library: libtoolsa.a
 *      This module is documented in the include file <toolsa/pjg.h>.
 *
 * Usage:
 *
 *       % TEST_toolsa_pjg
 *
 * Inputs: 
 *
 *       None
 *
 *
 * Author: Young Rhee       14-JUN-1994
 *
 */

/*
 * include files
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <toolsa/pjg_types.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/sincos.h>
#include "pjg_int.h"
#include "test_rand.h"

extern int TEST_PJGLatLonPlusRTheta(FILE *fp_err);
extern int TEST_PJGLatLon2RTheta(FILE *fp_err);
extern int TEST_PJG_flat(FILE *fp_err);
extern int TEST_PJG_grid(FILE *fp_err);

/*--------------------------------*/
int
TEST_PJGLatLonPlusRTheta(FILE *fp_err)
{
  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the subroutine PJGLatLonPlusRTheta(), 
   *      as defined in <toolsa/pjg.h>
   */

  /*
   * Test behavior is as expected...??
   */
  
  int error_code = 0;
  double r, theta;
  double lat1, lon1;
  double lat2, lon2;
  double circum = PJG_get_earth_radius() * M_PI * 2.0;
  double third_circum = circum / 3.0;
  double quarter_circum = circum / 4.0;
  double sixth_circum = circum / 6.0;

  lat1 = 0.0;
  lon1 = 0.0;
  r = sixth_circum;
  theta = 0.0;

  PJGLatLonPlusRTheta(lat1, lon1, r, theta, &lat2, &lon2);

  if ((fabs(lat2 - 60.0) > TINY_ANGLE) ||
      (fabs(lon2 - 0.0) > TINY_ANGLE)) {
    fprintf(fp_err, "FAILED - PJGLatLonPlusRTheta\n");
    fprintf(fp_err, "lat1, lon1, r, theta, lat2, lon2 = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, r, theta, lat2, lon2);
    error_code = 1;
  }

  lat1 = 70.0;
  lon1 = 45.0;
  r = third_circum;
  theta = 0.0;

  PJGLatLonPlusRTheta(lat1, lon1, r, theta, &lat2, &lon2);

  if ((fabs(lat2 + 10.0) > TINY_ANGLE) ||
      (fabs(lon2 - 225.0) > TINY_ANGLE &&
       fabs(lon2 + 135.0) > TINY_ANGLE )) {
    fprintf(fp_err, "FAILED - PJGLatLonPlusRTheta\n");
    fprintf(fp_err, "lat1, lon1, r, theta, lat2, lon2 = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, r, theta, lat2, lon2);
    error_code = 1;
  }

  lat1 = 60.0;
  lon1 = 0.0;
  r = quarter_circum;
  theta = 180.0;

  PJGLatLonPlusRTheta(lat1, lon1, r, theta, &lat2, &lon2);

  if ((fabs(lat2 + 30.0) > TINY_ANGLE) ||
      (fabs(lon2 - 0.0) > TINY_ANGLE &&
       fabs(lon2 + 360.0) > TINY_ANGLE )) {
    fprintf(fp_err, "FAILED - PJGLatLonPlusRTheta\n");
    fprintf(fp_err, "lat1, lon1, r, theta, lat2, lon2 = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, r, theta, lat2, lon2);
    error_code = 1;
  }

  lat1 = -45.0;
  lon1 = 20.0;
  r = third_circum;
  theta = 180.0;

  PJGLatLonPlusRTheta(lat1, lon1, r, theta, &lat2, &lon2);

  if ((fabs(lat2 + 15.0) > TINY_ANGLE) ||
      (fabs(lon2 - 200.0) > TINY_ANGLE &&
       fabs(lon2 + 160.0) > TINY_ANGLE )) {
    fprintf(fp_err, "FAILED - PJGLatLonPlusRTheta\n");
    fprintf(fp_err, "lat1, lon1, r, theta, lat2, lon2 = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, r, theta, lat2, lon2);
    error_code = 1;
  }

  lat1 = 0.0;
  lon1 = 20.0;
  r = quarter_circum;
  theta = 90.0;

  PJGLatLonPlusRTheta(lat1, lon1, r, theta, &lat2, &lon2);

  if ((fabs(lat2) > TINY_ANGLE) ||
      (fabs(lon2 - 110.0) > TINY_ANGLE &&
       fabs(lon2 + 250.0) > TINY_ANGLE )) {
    fprintf(fp_err, "FAILED - PJGLatLonPlusRTheta\n");
    fprintf(fp_err, "lat1, lon1, r, theta, lat2, lon2 = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, r, theta, lat2, lon2);
    error_code = 1;
  }

  lat1 = 0.0;
  lon1 = 10.0;
  r = sixth_circum;
  theta = -90.0;

  PJGLatLonPlusRTheta(lat1, lon1, r, theta, &lat2, &lon2);

  if ((fabs(lat2) > TINY_ANGLE) ||
      (fabs(lon2 - 310.0) > TINY_ANGLE &&
       fabs(lon2 + 50.0) > TINY_ANGLE )) {
    fprintf(fp_err, "FAILED - PJGLatLonPlusRTheta\n");
    fprintf(fp_err, "lat1, lon1, r, theta, lat2, lon2 = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, r, theta, lat2, lon2);
    error_code = 1;
  }

  lat1 = 0.0;
  lon1 = 160.0;
  r = quarter_circum;
  theta = 90.0;

  PJGLatLonPlusRTheta(lat1, lon1, r, theta, &lat2, &lon2);

  if ((fabs(lat2) > TINY_ANGLE) ||
      (fabs(lon2 - 250.0) > TINY_ANGLE &&
       fabs(lon2 + 110.0) > TINY_ANGLE )) {
    fprintf(fp_err, "FAILED - PJGLatLonPlusRTheta\n");
    fprintf(fp_err, "lat1, lon1, r, theta, lat2, lon2 = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, r, theta, lat2, lon2);
    error_code = 1;
  }

  lat1 = 0.0;
  lon1 = -160.0;
  r = third_circum;
  theta = -90.0;

  PJGLatLonPlusRTheta(lat1, lon1, r, theta, &lat2, &lon2);

  if ((fabs(lat2) > TINY_ANGLE) ||
      (fabs(lon2 - 80.0) > TINY_ANGLE &&
       fabs(lon2 + 200.0) > TINY_ANGLE )) {
    fprintf(fp_err, "FAILED - PJGLatLonPlusRTheta\n");
    fprintf(fp_err, "lat1, lon1, r, theta, lat2, lon2 = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, r, theta, lat2, lon2);
    error_code = 1;
  }

  if(error_code) {
      fprintf(stderr, "FAILED test TEST_PJGLatLonPlusRTheta\n");
  }
    
  return(error_code);

}

/*--------------------------------*/
int
TEST_PJGLatLon2RTheta(FILE *fp_err)
{
  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the subroutine PJGLatLon2Theta(), 
   *      as defined in <toolsa/pjg.h>
   */

  /*
   * Test behavior is as expected...??
   */
  
  int error_code = 0;
  double r, theta;
  double lat1, lon1;
  double lat2, lon2;
  double circum = PJG_get_earth_radius() * M_PI * 2.0;
  double half_circum = circum / 2.0;
  double third_circum = circum / 3.0;
  double quarter_circum = circum / 4.0;
  double sixth_circum = circum / 6.0;

  lat1 = -60.0;
  lon1 = 20.0;
  lat2 = 60.0;
  lon2 = 20.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(theta) > TINY_ANGLE) ||
      (fabs(r - third_circum) > TINY_DIST)) {
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    error_code = 1;
  }

  lat1 = -60.0;
  lon1 = 20.0;
  lat2 = 60.0;
  lon2 = 200.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(r - half_circum) > TINY_DIST)) {
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    error_code = 1;
  }

  lat1 = 0.0;
  lon1 = 50.0;
  lat2 = 0.0;
  lon2 = -130.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(r - half_circum) > TINY_DIST)) {
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    error_code = 1;
  }

  lat1 = -60.0;
  lon1 = 20.0;
  lat2 = -60.0;
  lon2 = 200.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(theta + 180.0) > TINY_ANGLE) ||
      (fabs(r - sixth_circum) > TINY_DIST)) {
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    error_code = 1;
  }

  lat1 = 80.0;
  lon1 = 170.0;
  lat2 = -10.0;
  lon2 = 170.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(theta + 180.0) > TINY_ANGLE &&
       fabs(theta - 180.0) > TINY_ANGLE) ||
      (fabs(r - quarter_circum) > TINY_DIST)) {
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    error_code = 1;
  }

  lat1 = 0.0;
  lon1 = 20.0;
  lat2 = 0.0;
  lon2 = 110.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(theta - 90.0) > TINY_ANGLE) ||
      (fabs(r - quarter_circum) > TINY_DIST)) {
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    error_code = 1;
  }

  lat1 = 0.0;
  lon1 = 20.0;
  lat2 = 0.0;
  lon2 = -70.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(theta + 90.0) > TINY_ANGLE &&
       fabs(theta - 270.0) > TINY_ANGLE ) ||
      (fabs(r - quarter_circum) > TINY_DIST)) {
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    error_code = 1;
  }

  lat1 = 0.0;
  lon1 = 120.0;
  lat2 = 0.0;
  lon2 = -120.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(theta - 90.0) > TINY_ANGLE) ||
      (fabs(r - third_circum) > TINY_DIST)) {
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    error_code = 1;
  }

  lat1 = 0.0;
  lon1 = -150.0;
  lat2 = 0.0;
  lon2 = 150.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(theta + 90.0) > TINY_ANGLE &&
       fabs(theta - 270.0) > TINY_ANGLE ) ||
      (fabs(r - sixth_circum) > TINY_DIST)) {
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    error_code = 1;
  }

  lat1 = 20.0;
  lon1 = 20.0;
  lat2 = 20.0;
  lon2 = 30.0;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  if ((fabs(theta - 88.28605) > TINY_ANGLE) ||
      (fabs(r - 1044.7684) > TINY_DIST)) {
    fprintf(fp_err, "FAILED - PJGLatLon2RTheta\n");
    fprintf(fp_err, "lat1, lon1, lat2, lon2, r, theta = "
	    "%15.10f, %15.10f, %15.10f, %15.10f, %15.10f, %15.10f\n",
	    lat1, lon1, lat2, lon2, r, theta);
    error_code = 1;
  }

  /*
   * Done
   */

  if(error_code) {
      fprintf(stderr, "FAILED test TEST_PJGLatLon2RTheta\n");
  }
    
  return(error_code);

}

/*--------------------------------*/
int
TEST_PJG_flat(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;
  int ix, iy;

  double xx1, yy1;
  double xx2, yy2;
  double lat1, lon1;
  double lat2, lon2;
  double diff_lat, diff_lon;
  double diff_x, diff_y;
  pjg_grid_geom_t geom;
  PJGstruct *ps;

  /*
   * set geometry for test grid
   */

  geom.ref_lat = rand_in_range(-70.0, 70.0);
  geom.ref_lon = rand_in_range(-180.0, 180.0);
  geom.rotation = rand_in_range(-45.0, 45.0);
  geom.min_x = rand_in_range(-100.0, 100.0);
  geom.min_y = rand_in_range(-100.0, 100.0);
  geom.dx = rand_in_range(0.25, 2.0);
  geom.dy = rand_in_range(0.25, 2.0);
  geom.nx = (int) rand_in_range(20.0, 100.0);
  geom.ny = (int) rand_in_range(20.0, 100.0);
  geom.type = PJG_FLAT;
  geom.units = PJG_UNITS_KM;

  fprintf(stdout, "==== FLAT TEST GRID ====\n");
  PJG_grid_geom_print(&geom, stdout);

  ps = PJGs_flat_init(geom.ref_lat, geom.ref_lon, geom.rotation);

  /*
   * print out lat,lon of each corner
   */

  ix = 0;
  iy = 0;
  PJG_fgrid_latloni(&geom, ix, iy, &lat1, &lon1);
  fprintf(stdout, "SW corner: (%d, %d) = (%g, %g)\n",
	  ix, iy, lat1, lon1);

  xx1 = geom.min_x;
  yy1 = geom.min_y;
  PJGs_flat_xy2latlon(ps, xx1, yy1, &lat2, &lon2);
  diff_lon = lon1 - lon2;
  diff_lat = lat1 - lat2;
  if (fabs(diff_lon) > TINY_ANGLE || fabs(diff_lat) > TINY_ANGLE) {
    fprintf(fp_err, "FAILED flat test\n");
    PJG_grid_geom_print(&geom, fp_err);
    fprintf(fp_err, "SW corner: (%g, %g) = (%g, %g)\n",
	    xx1, yy1, lat2, lon2);
    fprintf(fp_err, "lat1, lon1: %15.10f, %15.10f\n", lat1, lon1);
    fprintf(fp_err, "lat2, lon2: %15.10f, %15.10f\n", lat2, lon2);
    retval = 1;
  }
  PJGs_flat_latlon2xy(ps, lat2, lon2, &xx2, &yy2);
  diff_x = xx1 - xx2;
  diff_y = yy1 - yy2;
  if (fabs(diff_x) > TINY_ANGLE || fabs(diff_y) > TINY_ANGLE) {
    fprintf(fp_err, "FAILED flat test\n");
    PJG_grid_geom_print(&geom, fp_err);
    fprintf(fp_err, "SW corner: (%g, %g) = (%g, %g)\n",
	    xx1, yy1, lat2, lon2);
    fprintf(fp_err, "x1, y1: %15.10f, %15.10f\n", xx1, yy1);
    fprintf(fp_err, "x2, y2: %15.10f, %15.10f\n", xx2, yy2);
    retval = 1;
  }
    
  ix = geom.nx - 1;
  iy = 0;
  PJG_fgrid_latloni(&geom, ix, iy, &lat1, &lon1);
  fprintf(stdout, "SE corner: (%d, %d) = (%g, %g)\n",
	  ix, iy, lat1, lon1);
  xx1 = geom.min_x + (geom.nx - 1) * geom.dx;
  yy1 = geom.min_y;
  PJGs_flat_xy2latlon(ps, xx1, yy1, &lat2, &lon2);
  diff_lon = lon1 - lon2;
  diff_lat = lat1 - lat2;
  if (fabs(diff_lon) > TINY_ANGLE || fabs(diff_lat) > TINY_ANGLE) {
    fprintf(fp_err, "FAILED flat test\n");
    PJG_grid_geom_print(&geom, fp_err);
    fprintf(fp_err, "SE corner: (%g, %g) = (%g, %g)\n",
	    xx1, yy1, lat2, lon2);
    fprintf(fp_err, "lat1, lon1: %15.10f, %15.10f\n", lat1, lon1);
    fprintf(fp_err, "lat2, lon2: %15.10f, %15.10f\n", lat2, lon2);
    retval = 1;
  }
    

  ix = 0;
  iy = geom.ny - 1;
  PJG_fgrid_latloni(&geom, ix, iy, &lat1, &lon1);
  fprintf(stdout, "NW corner: (%d, %d) = (%g, %g)\n",
	  ix, iy, lat1, lon1);
  xx1 = geom.min_x;
  yy1 = geom.min_y + (geom.ny - 1) * geom.dy;
  PJGs_flat_xy2latlon(ps, xx1, yy1, &lat2, &lon2);
  diff_lon = lon1 - lon2;
  diff_lat = lat1 - lat2;
  if (fabs(diff_lon) > TINY_ANGLE || fabs(diff_lat) > TINY_ANGLE) {
    fprintf(fp_err, "FAILED flat test\n");
    PJG_grid_geom_print(&geom, fp_err);
    fprintf(fp_err, "NW corner: (%g, %g) = (%g, %g)\n",
	    xx1, yy1, lat2, lon2);
    fprintf(fp_err, "lat1, lon1: %15.10f, %15.10f\n", lat1, lon1);
    fprintf(fp_err, "lat2, lon2: %15.10f, %15.10f\n", lat2, lon2);
    retval = 1;
  }

  ix = geom.nx - 1;
  iy = geom.ny - 1;
  PJG_fgrid_latloni(&geom, ix, iy, &lat1, &lon1);
  fprintf(stdout, "NE corner: (%d, %d) = (%g, %g)\n",
	  ix, iy, lat1, lon1);
  xx1 = geom.min_x + (geom.nx - 1) * geom.dx;
  yy1 = geom.min_y + (geom.ny - 1) * geom.dy;
  PJGs_flat_xy2latlon(ps, xx1, yy1, &lat2, &lon2);
  diff_lon = lon1 - lon2;
  diff_lat = lat1 - lat2;
  if (fabs(diff_lon) > TINY_ANGLE || fabs(diff_lat) > TINY_ANGLE) {
    fprintf(fp_err, "FAILED flat test\n");
    PJG_grid_geom_print(&geom, fp_err);
    fprintf(fp_err, "NE corner: (%g, %g) = (%g, %g)\n",
	    xx1, yy1, lat2, lon2);
    fprintf(fp_err, "lat1, lon1: %15.10f, %15.10f\n", lat1, lon1);
    fprintf(fp_err, "lat2, lon2: %15.10f, %15.10f\n", lat2, lon2);
    retval = 1;
  }

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_PJG_grid.\n");
  }
    
  return(retval);

}

/*--------------------------------*/
int
TEST_PJG_grid(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;
  int i;
  int ix, iy;
  int error_printed;

  double r;
  double ixd, iyd;
  double ixd1, iyd1;
  double ixd2, iyd2;
  double ixd3, iyd3;
  double lat, lon;
  double lat1, lon1;
  double lat2, lon2;
  double diff, diff_x, diff_y;
  double move_x, move_y;
  double move_dist, move_phi;
  double rel_x, rel_y;
  double error_x, error_y;
  double distort_x, distort_y, max_distort;
  double ang, sinang, cosang;

  pjg_grid_geom_t geom1, geom2, geom3;

  /*
   * set geometry for test grid 1
   */

  geom1.ref_lat = rand_in_range(-70.0, 70.0);
  geom1.ref_lon = rand_in_range(-180.0, 180.0);
  geom1.rotation = rand_in_range(-45.0, 45.0);
  geom1.min_x = rand_in_range(-100.0, 100.0);
  geom1.min_y = rand_in_range(-100.0, 100.0);
  geom1.dx = rand_in_range(0.25, 2.0);
  geom1.dy = rand_in_range(0.25, 2.0);
  geom1.nx = (int) rand_in_range(20.0, 100.0);
  geom1.ny = (int) rand_in_range(20.0, 100.0);
  geom1.type = PJG_FLAT;
  geom1.units = PJG_UNITS_KM;

  fprintf(stdout, "==== TEST GRID 1 ====\n");
  PJG_grid_geom_print(&geom1, stdout);

  /*
   * print out lat,lon of each corner
   */

  ix = 0;
  iy = 0;
  PJG_fgrid_latloni(&geom1, ix, iy, &lat, &lon);
  fprintf(stdout, "SW corner: (%d, %d) = (%g, %g)\n",
	  ix, iy, lat, lon);

  ix = geom1.nx - 1;
  iy = 0;
  PJG_fgrid_latloni(&geom1, ix, iy, &lat, &lon);
  fprintf(stdout, "SE corner: (%d, %d) = (%g, %g)\n",
	  ix, iy, lat, lon);

  ix = 0;
  iy = geom1.ny - 1;
  PJG_fgrid_latloni(&geom1, ix, iy, &lat, &lon);
  fprintf(stdout, "NW corner: (%d, %d) = (%g, %g)\n",
	  ix, iy, lat, lon);

  ix = geom1.nx - 1;
  iy = geom1.ny - 1;
  PJG_fgrid_latloni(&geom1, ix, iy, &lat, &lon);
  fprintf(stdout, "NE corner: (%d, %d) = (%g, %g)\n",
	  ix, iy, lat, lon);

  /*
   * check inverses - perform 100 times, coords change randomly
   */
  
  for  (i = 0; i < 100; i++) {

    ixd1 = rand_in_range(-30.0, 30.0);
    iyd1 = rand_in_range(-30.0, 30.0);
    PJG_fgrid_latlond(&geom1, ixd1, iyd1, &lat, &lon);
    PJG_fgrid_xyd(&geom1, lat, lon, &ixd2, &iyd2);
    
    diff = ixd1 - ixd2;
    if (fabs(diff) > TINY_ANGLE) {
      fprintf(fp_err, "FAILED inverse test ix - lon - ix\n");
      fprintf(fp_err, "ixd1, iyd1: %15.10f, %15.10f\n", ixd1, iyd1);
      fprintf(fp_err, "ixd2, iyd2: %15.10f, %15.10f\n", ixd2, iyd2);
      fprintf(fp_err, "lat, lon: %15.10f, %15.10f\n", lat, lon);
      retval = 1;
    }
    
    diff = iyd1 - iyd2;
    if (fabs(diff) > TINY_ANGLE) {
      fprintf(fp_err, "FAILED inverse test iy - lat - iy\n");
      fprintf(fp_err, "ixd1, iyd1: %15.10f, %15.10f\n", ixd1, iyd1);
      fprintf(fp_err, "ixd2, iyd2: %15.10f, %15.10f\n", ixd2, iyd2);
      fprintf(fp_err, "lat, lon: %15.10f, %15.10f\n", lat, lon);
      retval = 1;
    }
    
    lat1 = rand_in_range(-89.0, 89.0);
    lon1 = rand_in_range(-180.0, 180.0);
    PJG_fgrid_xyd(&geom1, lat1, lon1, &ixd, &iyd);
    PJG_fgrid_latlond(&geom1, ixd, iyd, &lat2, &lon2);
    
    diff = lat1 - lat2;
    if (fabs(diff) > TINY_ANGLE) {
      fprintf(fp_err, "FAILED inverse test lat - iy - lat\n");
      fprintf(fp_err, "ixd, iyd: %15.10f, %15.10f\n", ixd, iyd);
      fprintf(fp_err, "lat1, lon1: %15.10f, %15.10f\n", lat1, lon1);
      fprintf(fp_err, "lat2, lon2: %15.10f, %15.10f\n", lat2, lon2);
      retval = 1;
    }
    
    diff = lon1 - lon2;
    if (fabs(diff) > TINY_ANGLE) {
      fprintf(fp_err, "FAILED inverse test lon - ix - lon\n");
      fprintf(fp_err, "ixd, iyd: %15.10f, %15.10f\n", ixd, iyd);
      fprintf(fp_err, "lat1, lon1: %15.10f, %15.10f\n", lat1, lon1);
      fprintf(fp_err, "lat2, lon2: %15.10f, %15.10f\n", lat2, lon2);
      retval = 1;
    }

  } /* i */

  /*
   * print flat projection distortion
   */

  fprintf(stdout, "Typical distortions(km) vs. range(km) from grid origin\n");
  for (i = 0; i < 10; i++) {
    r = i * 100.0;
    PJGLatLonPlusRTheta(geom1.ref_lat, geom1.ref_lon, r, geom1.rotation,
			&lat1, &lon1);
    PJGLatLonPlusRTheta(lat1, lon1, r, geom1.rotation + 180.0,
			&lat2, &lon2);
    PJGLatLon2DxDy(geom1.ref_lat, geom1.ref_lon, lat2, lon2,
		   &distort_x, &distort_y);
    fprintf(stdout, "range, distort_x, distort_y: %g, %g, %g\n",
	    r, distort_x, distort_y);
  }

  /*
   * get distortion at edge of grid
   */

  r = sqrt (pow(geom1.dx * geom1.nx, 2.0) *
	    pow(geom1.dx * geom1.nx, 2.0));
  PJGLatLonPlusRTheta(geom1.ref_lat, geom1.ref_lon, r, geom1.rotation,
		      &lat1, &lon1);
  PJGLatLonPlusRTheta(lat1, lon1, r, geom1.rotation + 180.0,
		      &lat2, &lon2);
  PJGLatLon2DxDy(geom1.ref_lat, geom1.ref_lon, lat2, lon2,
		 &distort_x, &distort_y);
  max_distort =
    sqrt(distort_x * distort_x + distort_y * distort_y) * 5.0 + 0.5;
  fprintf(stdout, "max distort for grid: %g\n", max_distort);

  /*
   * set geometry for test grid 2 - this is rotated 90 degrees to
   * grid 1
   */

  fprintf(stdout, "Testing match of grid 1 with 90 deg rotated grid 2\n");

  geom2.ref_lat = geom1.ref_lat;
  geom2.ref_lon = geom1.ref_lon;
  geom2.rotation = 270.0 + geom1.rotation;
  geom2.min_x = geom1.min_y;
  geom2.min_y = -((geom1.nx - 1) * geom1.dx) - geom1.min_x;
  geom2.dx = geom1.dy;
  geom2.dy = geom1.dx;
  geom2.nx = geom1.ny;
  geom2.ny = geom1.nx;
  geom2.type = PJG_FLAT;
  geom2.units = PJG_UNITS_KM;

  fprintf(stdout, "==== TEST GRID 2 ====\n");
  PJG_grid_geom_print(&geom2, stdout);

  /*
   * check that the points in the grids are the same, but with
   * the x and y axes transposed
   */

  error_printed = 0;
  for (ix = 0; ix < geom1.nx; ix++) {
    for (iy = 0; iy < geom1.ny; iy++) {
      PJG_fgrid_latloni(&geom1, ix, iy, &lat1, &lon1);
      PJG_fgrid_latloni(&geom2, iy, geom1.nx - ix - 1, &lat2, &lon2);
      diff = lat1 - lat2;
      if (fabs(diff) > TINY_ANGLE) {
	if (!error_printed) {
	  fprintf(fp_err, "FAILED rotated grid test, lat1 - lat2\n");
	  fprintf(fp_err, "lat1, lat2: %15.10f, %15.10f\n",
		  lat1, lat2);
	  error_printed = 1;
	}
	retval = 1;
      }
      diff = lon1 - lon2;
      if (fabs(diff) > TINY_ANGLE) {
	if (!error_printed) {
	  fprintf(fp_err, "FAILED rotated grid test, lon1 - lon2\n");
	  error_printed = 1;
	}
	retval = 1;
      }
    }
  }

  /*
   * create grid 3 by moving grid 1 a small distance
   * also change to nm.
   */

  fprintf(stdout, "Testing match of grid 1 with translated grid 3\n");

  geom3 = geom1;

  move_x = geom1.dx * geom1.nx;
  move_y = geom1.dy * geom1.ny;

  move_dist = sqrt(move_x * move_x +
		   move_y * move_y);
  
  if (move_x == 0.0 && move_y == 0.0) {
    move_phi = 0.0;
  } else {
    move_phi = atan2(move_x, move_y) * RAD_TO_DEG;
  }

  /*
   * compute the movement relative to the grids, taking
   * orientation into account
   */

  ang = (move_phi - geom3.rotation) * DEG_TO_RAD;
  ta_sincos(ang, &sinang, &cosang);
  rel_x = move_dist * sinang;
  rel_y = move_dist * cosang;

  fprintf(stdout, "move_x, move_y: %g, %g\n",
	  move_x, move_y);
  fprintf(stdout, "move_dist, move_phi: %g, %g\n",
	  move_dist, move_phi);

  PJGLatLonPlusDxDy(geom1.ref_lat, geom1.ref_lon,
		    move_x, move_y,
		    &geom3.ref_lat, &geom3.ref_lon);
  PJGLatLon2DxDy(geom1.ref_lat, geom1.ref_lon,
		 geom3.ref_lat, geom3.ref_lon,
		 &diff_x, &diff_y);

  geom3.min_x = geom1.min_x / KM_PER_NM;
  geom3.min_y = geom1.min_y / KM_PER_NM;
  geom3.dx = geom1.dx / KM_PER_NM;
  geom3.dy = geom1.dy / KM_PER_NM;
  geom3.type = PJG_FLAT;
  geom3.units = PJG_UNITS_NM;

  fprintf(stdout, "==== TEST GRID 3 ====\n");
  PJG_grid_geom_print(&geom3, stdout);

  /*
   * determine the diff in x and y between each pair of
   * points in geom1 and geom3 - should be move_x and move_y
   */

  error_printed = 0;
  for (ix = 0; ix < geom3.nx; ix++) {
    for (iy = 0; iy < geom3.ny; iy++) {

      ixd3 = (double) ix;
      iyd3 = (double) iy;

      PJG_fgrid_latloni(&geom3, ix, iy, &lat, &lon);
      PJG_fgrid_xyd(&geom1, lat, lon, &ixd1, &iyd1);

      diff_x = (ixd1 - ixd3) * geom1.dx;
      diff_y = (iyd1 - iyd3) * geom1.dy;

      /*
       * check for dist errors of 200 m or more
       */

      error_x = fabs(diff_x - rel_x);
      error_y = fabs(diff_y - rel_y);

      if (error_x > max_distort || error_y > max_distort) {
 	if (!error_printed) {
	  fprintf(fp_err, "FAILED translated grid test\n");
	  fprintf(fp_err, "error_x, error_y: %15.10f, %15.10f\n",
		  error_x, error_y);
	  fprintf(fp_err, "ixd1, iyd1: %15.10f, %15.10f\n",
		  ixd1, iyd1);
	  fprintf(fp_err, "ixd3, iyd3: %15.10f, %15.10f\n",
		  ixd3, iyd3);
	  fprintf(fp_err, "lat, lon: %15.10f, %15.10f\n",
		  lat, lon);
	  fprintf(fp_err, "rel_x, rel_y: %15.10f, %15.10f\n",
		  rel_x, rel_y);
	  fprintf(fp_err, "diff_x, diff_y: %15.10f, %15.10f\n",
		  diff_x, diff_y);
	  error_printed = 1;
 	}
	retval = 1;
      }
    } /* iy */
  } /* ix */

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_PJG_grid.\n");
  }
    
  return(retval);

}

static void print_usage(char *prog_name, FILE *out)

{
    fprintf(out, "Usage: %s [-many]\n", prog_name);
}

/*--------------------------------*/

/*
 * main program driver
 */

int
main(int argc, char *argv[])
{

  int retval = 0;
  int i;
  int repeats = 1;
  FILE *fp_err;

  /*
   * check usage
   */

  if (argc > 2 || (argc == 2 && !strcmp(argv[1], "-h"))) {
      print_usage(argv[0], stdout);
      return (0);
  }

  if (argc > 2 || (argc == 2 && strcmp(argv[1], "-many"))) {
      print_usage(argv[0], stderr);
      return (-1);
  }

  if (argc == 2 && !strcmp(argv[1], "-many")) {
      repeats = 1000000;
  }

  /*
   * open errlog file
   */

  if ((fp_err = fopen("test_pjg.errlog", "a")) == NULL) {
      fprintf(stderr, "Cannot open file test_pjg.errlog\n");
      perror("test_pjg.errlog");
      return (1);
  }

  /*
   * seed random generator
   */

  gb_init_rand((unsigned int) time(NULL));

  /*
   * Test the individual module subroutines
   */

  for (i = 0; i < repeats; i++) {

    fprintf(stdout, "*** testing loop %d\n", i);

    if (TEST_PJGLatLonPlusRTheta(fp_err)) {
	retval = 1;
    }

    if (TEST_PJGLatLon2RTheta(fp_err)) {
	retval = 1;
    }

    if (TEST_PJG_flat(fp_err)) {
      retval = 1;
    }

    if (TEST_PJG_grid(fp_err)) {
      retval = 1;
    }

    if (retval)
	break;

    fflush(stdout);

  }

  if (retval) {
      fprintf(stderr, "PJG module failed test\n");
  } else {
      fprintf(stdout, "PJG module passed test\n");
  }
  
  fclose(fp_err);
  return(retval);
}

