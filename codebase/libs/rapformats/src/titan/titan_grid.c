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
 * titan_grid.c
 *
 * titan grid manipulation routines - copied from mdv_grid.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Dec 1999
 *
 **************************************************************************/

#include <stdlib.h>
#include <rapformats/titan_grid.h>
#include <toolsa/pjg.h>
#include <math.h>
#include <memory.h>
#include <toolsa/toolsa_macros.h>

#define TINY_ANGLE 1.e-4
#define TINY_DIST 1.e-2
#define TINY_FLOAT 1.e-10

/*
 * file scope prototypes
 */

static void flat_latlon2xy(const titan_grid_comps_t *comps,
			   double lat, double lon,
			   double *x, double *y);

static void flat_xy2latlon(const titan_grid_comps_t *comps,
			   double x, double y,
			   double *lat, double *lon);

static void latlon_2_r_theta(double colat1,
			     double cos_colat1,
			     double sin_colat1,
			     double lon1,
			     double lat2, double lon2,
			     double *r, double *theta_rad);

static void latlon_plus_r_theta(double cos_colat1,
				double sin_colat1,
				double lon1_rad,
				double r, double theta_rad,
				double *lat2, double *lon2);

static void ll_latlon2xy(const titan_grid_comps_t *comps,
			 double lat, double lon,
			 double *x, double *y);

static void ll_xy2latlon(const titan_grid_comps_t *comps,
			 double x, double y,
			 double *lat, double *lon);

static void lc2_latlon2xy_2_tan(const titan_grid_comps_t *comps,
                                double lat, double lon,
                                double *x, double *y);

static void lc2_latlon2xy_1_tan(const titan_grid_comps_t *comps,
                                double lat, double lon,
                                double *x, double *y);

static void lc2_xy2latlon_2_tan(const titan_grid_comps_t *comps,
                                double x, double y,
                                double *lat, double *lon);

static void lc2_xy2latlon_1_tan(const titan_grid_comps_t *comps,
                                double x, double y,
                                double *lat, double *lon);

/*******************************
 * initialize generic projection
 */

void TITAN_init_proj(const titan_grid_t *grid,
		     titan_grid_comps_t *comps)

{

  memset(comps, 0, sizeof(titan_grid_comps_t));

  comps->proj_type = grid->proj_type;

  if (grid->proj_type == TITAN_PROJ_FLAT) {

    TITAN_init_flat(grid->proj_origin_lat,
		  grid->proj_origin_lon,
		  grid->proj_params.flat.rotation,
		  comps);

  } else if (grid->proj_type == TITAN_PROJ_LATLON) {

    TITAN_init_latlon(comps);

  } else if (grid->proj_type == TITAN_PROJ_LAMBERT_CONF) {

    TITAN_init_lc2(grid->proj_origin_lat,
                 grid->proj_origin_lon,
                 grid->proj_params.lc2.lat1,
                 grid->proj_params.lc2.lat2,
                 comps);

  } else {

    fprintf(stderr, "ERROR - TITAN_init_proj\n");
    fprintf(stderr, "TITAN proj type %d not supported\n",
	    grid->proj_type);
    exit (-1);

  }

}

/***********************************
 * initialize flat earth projection
 */

void TITAN_init_flat(double origin_lat,
		   double origin_lon,
		   double rotation,
		   titan_grid_comps_t *comps)

{

  comps->proj_type = TITAN_PROJ_FLAT;

  comps->origin_lat = origin_lat;
  comps->origin_lon = origin_lon;
  comps->rotation = rotation;
  
  comps->origin_lat_rad = origin_lat * RAD_PER_DEG;
  comps->origin_lon_rad = origin_lon * RAD_PER_DEG;
  comps->rotation_rad = rotation * RAD_PER_DEG;
  
  comps->origin_colat = (90.0 - comps->origin_lat) * RAD_PER_DEG;
  
  comps->sin_origin_colat = sin(comps->origin_colat);
  comps->cos_origin_colat = cos(comps->origin_colat);
  
  comps->latlon2xy = flat_latlon2xy;
  comps->xy2latlon = flat_xy2latlon;

}

/***********************************
 * initialize lambert conformal projection with two lats.
 */

void TITAN_init_lc2(double origin_lat,
		  double origin_lon,
		  double lat1,
		  double lat2,
		  titan_grid_comps_t *comps)

{

  /* check illegal values */                                             

  if (fabs(origin_lat - 90.0) < TINY_ANGLE) {
    fprintf(stderr, "WARNING - TITAN_init_lc2\n");
    fprintf(stderr, "  origin lat is at N pole: %g\n", origin_lat);
    origin_lat -= TINY_ANGLE;
  }

  if (fabs(origin_lat + 90.0) < TINY_ANGLE) {
    fprintf(stderr, "WARNING - TITAN_init_lc2\n");
    fprintf(stderr, "  origin lat is at S pole: %g\n", origin_lat);
    origin_lat += TINY_ANGLE;
  }

  if (fabs(lat1 - 90.0) < TINY_ANGLE) {
    fprintf(stderr, "WARNING - TITAN_init_lc2\n");
    fprintf(stderr, "  lat1 is at N pole: %g\n", origin_lat);
    lat1 -= TINY_ANGLE;
  }

  if (fabs(lat1 + 90.0) < TINY_ANGLE) {
    fprintf(stderr, "WARNING - TITAN_init_lc2\n");
    fprintf(stderr, "  lat1 is at S pole: %g\n", origin_lat);
    lat1 += TINY_ANGLE;
  }

  if (fabs(lat2 - 90.0) < TINY_ANGLE) {
    fprintf(stderr, "WARNING - TITAN_init_lc2\n");
    fprintf(stderr, "  lat2 is at N pole: %g\n", origin_lat);
    lat2 -= TINY_ANGLE;
  }

  if (fabs(lat2 + 90.0) < TINY_ANGLE) {
    fprintf(stderr, "WARNING - TITAN_init_lc2\n");
    fprintf(stderr, "  lat2 is at S pole: %g\n", origin_lat);
    lat2 += TINY_ANGLE;
  }

  comps->lc2_2_tan_lines = 1;
  if (fabs(lat2 - lat1) < TINY_ANGLE) {
    comps->lc2_2_tan_lines = 0;
  }

  comps->proj_type = TITAN_PROJ_LAMBERT_CONF;

  comps->origin_lat = origin_lat;
  comps->origin_lon = origin_lon;
  comps->rotation = 0.0;
  
  comps->origin_lat_rad = origin_lat * RAD_PER_DEG;
  comps->origin_lon_rad = origin_lon * RAD_PER_DEG;
  comps->rotation_rad = comps->rotation * RAD_PER_DEG;
  
  comps->origin_colat = (90.0 - comps->origin_lat) * RAD_PER_DEG;
  
  comps->sin_origin_colat = sin(comps->origin_colat);
  comps->cos_origin_colat = cos(comps->origin_colat);
  
  comps->lc2_lat1_rad = lat1 * RAD_PER_DEG;
  comps->lc2_lat2_rad = lat2 * RAD_PER_DEG;

  if (comps->lc2_2_tan_lines) {

    /* 2 distinct tan lines */

    double t1, t2, t1n, t0n;

    t1 = tan(M_PI_4 + comps->lc2_lat1_rad / 2.0);
    t2 = tan(M_PI_4 + comps->lc2_lat2_rad / 2.0);
    comps->lc2_n = (log( cos(comps->lc2_lat1_rad)/cos(comps->lc2_lat2_rad)) /
                    log(t2/t1));
    
    t1n = pow(t1, comps->lc2_n);
    comps->lc2_F = cos(comps->lc2_lat1_rad) * t1n / comps->lc2_n;
    
    t0n = pow(tan(M_PI_4 + comps->origin_lat_rad/2), comps->lc2_n);
    comps->lc2_rho = PJG_get_earth_radius() * comps->lc2_F / t0n;
    
    comps->latlon2xy = lc2_latlon2xy_2_tan;
    comps->xy2latlon = lc2_xy2latlon_2_tan;
    
  } else {
    
    comps->lc2_sin0 = sin(comps->lc2_lat1_rad);
    comps->lc2_tan0 = tan(M_PI_4 - comps->lc2_lat1_rad / 2.0);
    comps->lc2_rho = PJG_get_earth_radius() / tan(comps->lc2_lat1_rad);

    comps->latlon2xy = lc2_latlon2xy_1_tan;
    comps->xy2latlon = lc2_xy2latlon_1_tan;
    
  }

}


/******************************
 * initialize latlon projection
 */

void TITAN_init_latlon(titan_grid_comps_t *comps)

{

  comps->proj_type = TITAN_PROJ_LATLON;

  comps->latlon2xy = ll_latlon2xy;
  comps->xy2latlon = ll_xy2latlon;

}

/*********************
 * generic latlon2xy()
 */

void TITAN_latlon2xy(const titan_grid_comps_t *comps,
		     double lat, double lon,
		     double *x, double *y)
{

  comps->latlon2xy(comps, lat, lon, x, y);

}

/*********************
 * generic xy2latlon()
 */

void TITAN_xy2latlon(const titan_grid_comps_t *comps,
		     double x, double y,
		     double *lat, double *lon)
     
{
  
  comps->xy2latlon(comps, x, y, lat, lon);
  
}

/******************
 * flat latlon2xy()
 */

static void flat_latlon2xy(const titan_grid_comps_t *comps,
			   double lat, double lon,
			   double *x, double *y)

{

  double r, theta_rad;
  double grid_theta;

  latlon_2_r_theta(comps->origin_colat,
		   comps->cos_origin_colat,
		   comps->sin_origin_colat,
		   comps->origin_lon,
		   lat, lon, &r, &theta_rad);

  grid_theta = theta_rad - comps->rotation_rad;

  *x = r * sin(grid_theta);
  *y = r * cos(grid_theta);

}
     
/******************
 * flat xy2latlon()
 */

static void flat_xy2latlon(const titan_grid_comps_t *comps,
			   double x, double y,
			   double *lat, double *lon)

{
  double r, theta_rad;
  
  r = sqrt(x * x + y * y);

  if (x == 0.0 && y == 0.0) {
    theta_rad = comps->rotation_rad;
  } else {
    theta_rad = atan2(x, y) + comps->rotation_rad; /* rel to TN */
  }
  
  latlon_plus_r_theta(comps->cos_origin_colat,
		      comps->sin_origin_colat,
		      comps->origin_lon_rad,
		      r, theta_rad, lat, lon);

}
     
/********************
 * latlon latlon2xy()
 */

static void ll_latlon2xy(const titan_grid_comps_t *comps,
			 double lat, double lon,
			 double *x, double *y)

{
  *y = lat;
  *x = lon;
}
     
/********************
 * latlon xy2latlon()
 */

static void ll_xy2latlon(const titan_grid_comps_t *comps,
			 double x, double y,
			 double *lat, double *lon)

{
  *lat = y;
  *lon = x;
}
     
/*******************************************************************
 * latlon_plus_r_theta()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  of length r which makes an angle of theta from true North.
 *  Theta is positive if east of North, negative (or > PI) if west of North,
 *  0 = North
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *	    arclength r (km), angle theta (degrees)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

static void latlon_plus_r_theta(double cos_colat1,
				double sin_colat1,
				double lon1_rad,
				double r, double theta_rad,
				double *lat2, double *lon2)
{
  
  double darc, colat2;
  double denom, delta_lon;
  double cos_theta;
  double cos_colat2, sin_colat2;
  double xx;
  
  darc = r / PJG_get_earth_radius();
  cos_theta = cos(theta_rad);

  xx = cos_colat1 * cos(darc) + sin_colat1 * sin(darc) * cos_theta;
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  colat2 = acos(xx);
  cos_colat2 = cos(colat2);
  sin_colat2 = sin(colat2);
  *lat2 = 90.0 - colat2 * RAD_TO_DEG;
  
  denom = sin_colat1 * sin_colat2;
  
  if ( fabs(denom) <= TINY_FLOAT) {
    delta_lon = 0.0;
  } else {
    xx = (cos(darc) - cos_colat1 * cos_colat2) / denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    delta_lon = acos(xx);
  }

  /*
   * reverse sign if theta is west
   */

  if (sin(theta_rad) < 0.0)
    delta_lon *= -1.0;
  
  *lon2 = (lon1_rad + delta_lon) * RAD_TO_DEG;

  if (*lon2 < -180.0)
    *lon2 += 360.0;
  if (*lon2 > 180.0)
    *lon2 -= 360.0;

  return;
  
}

/*********************************************************************
 * latlon_2_r_theta
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: r = the arc length from 1 to 2, in km 
 *	    theta =  angle with True North: positive if east of North,
 *          negative if west of North, 0 = North
 *
 *********************************************************************/

static void latlon_2_r_theta(double colat1,
			     double cos_colat1,
			     double sin_colat1,
			     double lon1,
			     double lat2, double lon2,
			     double *r, double *theta_rad)
{

  double darc, colat2, delon, denom, therad;
  double cos_colat2, sin_colat2;
  double xx;

  colat2 = (90.0 - lat2) * DEG_TO_RAD;

  cos_colat2 = cos(colat2);
  sin_colat2 = sin(colat2);

  delon = (lon2 - lon1) * DEG_TO_RAD;
  
  if (delon < -M_PI) {
    delon += 2.0 * M_PI;
  }
  
  if (delon > M_PI) {
    delon -= 2.0 * M_PI;
  }
  
  xx = cos_colat1 * cos_colat2 + sin_colat1 * sin_colat2 * cos(delon);
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  darc = acos(xx);
  
  *r = darc* PJG_get_earth_radius();
  
  denom = sin_colat1 * sin(darc);

  if ((fabs(colat1) <= TINY_ANGLE) || (fabs(denom) <= TINY_FLOAT)) {
    therad = 0.0;
  } else {
    xx = (cos_colat2 - cos_colat1 * cos(darc)) / denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    therad = acos(xx);
  }
  
  if ((delon < 0.0) || (delon > M_PI))
    therad *= -1.0;
  
  *theta_rad = therad;

}

static void lc2_latlon2xy_2_tan(const titan_grid_comps_t *comps,
                                double lat, double lon,
                                double *x, double *y)
{

  double lat_rad = lat * DEG_TO_RAD;
  double lon_rad = lon * DEG_TO_RAD;
  
  double theta = comps->lc2_n * (lon_rad - comps->origin_lon_rad);

  double tn = pow(tan(M_PI_4 + lat_rad / 2.0), comps->lc2_n);
  double r = PJG_get_earth_radius() * comps->lc2_F / tn;

  *x = r * sin(theta);
  *y = comps->lc2_rho - r * cos(theta);

}

static void lc2_latlon2xy_1_tan(const titan_grid_comps_t *comps,
                                double lat, double lon,
                                double *x, double *y)
{

  double lat_rad = lat * DEG_TO_RAD;
  double lon_rad = lon * DEG_TO_RAD;
  
  double tan_phi = tan(M_PI_4 - lat_rad / 2.0);
  double f1 = pow((tan_phi/comps->lc2_tan0), comps->lc2_sin0);
  double del_lon_rad = (lon_rad - comps->origin_lon_rad) * comps->lc2_sin0;

  *x = comps->lc2_rho * f1 * sin(del_lon_rad);
  *y = comps->lc2_rho * (1.0 - f1 * cos(del_lon_rad));

}

static void lc2_xy2latlon_2_tan(const titan_grid_comps_t *comps,
                                double x, double y,
                                double *lat, double *lon)
{

  double yd = (comps->lc2_rho - y);
  double theta = atan2(x, yd);
  double r = sqrt(x * x + yd * yd);
  if (comps->lc2_n < 0.0) {
    r *= -1.0;
  }

  *lon = (theta / comps->lc2_n + comps->origin_lon_rad) * RAD_TO_DEG;
  *lon = PJGrange180(*lon);

  if (fabs(r) < TINY_FLOAT) {
    *lat = ((comps->lc2_n < 0.0) ? -90.0 : 90.0);
  } else {
    double rn = pow(PJG_get_earth_radius() * comps->lc2_F / r,
                    1.0 / comps->lc2_n);
    *lat = (2.0 * atan(rn) - M_PI_2) * RAD_TO_DEG;
  }

  *lat = PJGrange180( *lat);
}

static void lc2_xy2latlon_1_tan(const titan_grid_comps_t *comps,
                                double x, double y,
                                double *lat, double *lon)
{

  double loc_x = x;
  double inv_sin0 = 1.0 / comps->lc2_sin0;

  double del_lon;
  double lon_rad;
  double lat_rad;
  
  double sin_lon;
  double r;
  double to_sin0;
  double f1;

  if (fabs(loc_x) < TINY_FLOAT) {
    loc_x = 0.001;
  }

  del_lon = inv_sin0 * atan2(loc_x, (comps->lc2_rho - y));
  lon_rad = comps->origin_lon_rad + del_lon;
  
  sin_lon = sin(del_lon * comps->lc2_sin0);
  r = comps->lc2_rho * sin_lon;
  to_sin0 = pow((loc_x / r), inv_sin0);
  f1 = 2.0 * atan(comps->lc2_tan0 * to_sin0);

  *lon = PJGrange180(lon_rad * RAD_TO_DEG);
 
  lat_rad = (M_PI_2 - f1);
  *lat = PJGrange180(lat_rad * RAD_TO_DEG);

}

#define BOOL_STR(a) (a == 0 ? "false" : "true")

/******************
 * TITAN_print_grid()
 *
 * Print titan grid struct
 */

void  TITAN_print_grid(FILE *out, const char *spacer, const titan_grid_t *grid)
     
{
  
  fprintf(out, "%sTITAN grid parameters\n", spacer);
  fprintf(out, "%s-------------------\n", spacer);
  
  fprintf(out, "%s  nbytes_char : %ld\n",
	  spacer, (long) grid->nbytes_char);
  
  if (grid->proj_type == TITAN_PROJ_FLAT) {
    fprintf(out, "%s  gridtype : flat\n", spacer);
  } else if (grid->proj_type == TITAN_PROJ_LATLON) {
    fprintf(out, "%s  gridtype : latlon\n", spacer);
  } else {
    fprintf(out, "%s  gridtype : UNKNOWN\n", spacer);
  }

  fprintf(out, "%s  origin latitude : %g\n",
	  spacer, grid->proj_origin_lat);
  fprintf(out, "%s  origin longitude : %g\n",
	  spacer, grid->proj_origin_lon);
  fprintf(out, "%s  grid rotation : %g\n",
	  spacer, grid->proj_params.flat.rotation);

  fprintf(out, "%s  nx, ny, nz : %d, %d, %d\n",
	  spacer,
	  grid->nx, grid->ny, grid->nz);

  fprintf(out, "%s  minx, miny, minz : %g, %g, %g\n",
	  spacer,
	  grid->minx, grid->miny, grid->minz);
  
  fprintf(out, "%s  dx, dy, dz : %g, %g, %g\n", spacer,
	  grid->dx, grid->dy, grid->dz);
  
  fprintf(out, "%s  sensor_x, sensor_y, sensor_z : %g, %g, %g\n",
	  spacer,
	  grid->sensor_x, grid->sensor_y, grid->sensor_z);
  
  fprintf(out, "%s  sensor_lat, sensor_lon : %g, %g\n",
	  spacer,
	  grid->sensor_lat, grid->sensor_lon);
  
  fprintf(out, "%s  dz_constant: %s\n", spacer,
	  BOOL_STR(grid->dz_constant));

  fprintf(out, "%s  x units : %s\n", spacer, grid->unitsx);
  fprintf(out, "%s  y units : %s\n", spacer, grid->unitsy);
  fprintf(out, "%s  z units : %s\n", spacer, grid->unitsz);
  
}

/******************
 * TITAN_print_gridXML()
 *
 * Print titan grid struct
 */

void  TITAN_print_gridXML(FILE *out, const char *spacer, const titan_grid_t *grid)
     
{
  
  fprintf(out, "%s<!--TITAN grid parameters-->\n", spacer);
  
  fprintf(out, "%s  <nbytes_char> %ld </nbytes_char>\n",
	  spacer, (long) grid->nbytes_char);
  
  if (grid->proj_type == TITAN_PROJ_FLAT) {
    fprintf(out, "%s  <gridtype> flat </gridtype>\n", spacer);
  } else if (grid->proj_type == TITAN_PROJ_LATLON) {
    fprintf(out, "%s  <gridtype> latlon </gridtype>\n", spacer);
  } else {
    fprintf(out, "%s  <gridtype> UNKNOWN </gridtype>\n", spacer);
  } 

  fprintf(out, "%s  <origin_lat> %g </origin_lat>\n",
	  spacer, grid->proj_origin_lat);
  fprintf(out, "%s  <origin_lon> %g </origin_lon>\n",
	  spacer, grid->proj_origin_lon);
  fprintf(out, "%s  <grid_rotation> %g </grid_rotation>\n",
	  spacer, grid->proj_params.flat.rotation);

  fprintf(out, "%s  <nx unit=\"%s\"> %d </nx> <ny unit=\"%s\"> %d </ny> <nz unit=\"%s\"> %d </nz>\n",
	  spacer,
	  grid->unitsx, grid->nx, grid->unitsy, grid->ny, grid->unitsz, grid->nz);

  fprintf(out, "%s  <minx unit=\"%s\">  %g </minx>  <miny unit=\"%s\"> %g </miny> <minz unit=\"%s\"> %g </minz>\n",
	  spacer,
	  grid->unitsx, grid->minx, grid->unitsy, grid->miny, grid->unitsz, grid->minz);
  
  fprintf(out, "%s  <dx unit=\"%s\"> %g </dx> <dy unit=\"%s\"> %g </dy> <dz unit=\"%s\"> %g </dz>\n", spacer,
	  grid->unitsx, grid->dx, grid->unitsy, grid->dy, grid->unitsz, grid->dz);
  
  fprintf(out, "%s  <sensor_x unit=\"%s\"> %g </sensor_x> <sensor_y unit=\"%s\"> %g </sensor_y> <sensor_z unit=\"%s\"> %g </sensor_z>\n",
	  spacer,
	  grid->unitsx, grid->sensor_x, grid->unitsy, grid->sensor_y, grid->unitsz, grid->sensor_z);
  
  fprintf(out, "%s  <sensor_lat> %g </sensor_lat> <sensor_lon> %g </sensor_lon>\n",
	  spacer,
	  grid->sensor_lat, grid->sensor_lon);
  
  fprintf(out, "%s  <dz_constant> %s </dz_constant>\n", spacer,
	  BOOL_STR(grid->dz_constant));
}

