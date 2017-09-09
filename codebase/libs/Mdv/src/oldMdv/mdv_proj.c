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
 * mdv_proj.c
 *
 * mdv projection routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * May 1996
 *
 **************************************************************************/

#include <toolsa/pjg.h>
#include <Mdv/mdv/mdv_grid.h>
#include <toolsa/toolsa_macros.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>

#define TINY_ANGLE 1.e-4
#define TINY_DIST 1.e-2
#define TINY_FLOAT 1.e-10

/*
 * file scope prototypes
 */

static void flat_latlon2xy(mdv_grid_comps_t *comps,
			   double lat, double lon,
			   double *x, double *y);

static void flat_xy2latlon(mdv_grid_comps_t *comps,
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

static void ll_latlon2xy(mdv_grid_comps_t *comps,
			 double lat, double lon,
			 double *x, double *y);

static void ll_xy2latlon(mdv_grid_comps_t *comps,
			 double x, double y,
			 double *lat, double *lon);

static void lc2_latlon2xy(mdv_grid_comps_t *comps,
                          double lat, double lon,
                          double *x, double *y);

static void lc2_xy2latlon(mdv_grid_comps_t *comps,
                          double x, double y,
                          double *lat, double *lon);


/*******************************
 * initialize generic projection
 */

void MDV_init_proj(mdv_grid_t *grid,
		   mdv_grid_comps_t *comps)

{

  memset(comps, 0, sizeof(mdv_grid_comps_t));

  comps->proj_type = grid->proj_type;

  if (grid->proj_type == MDV_PROJ_FLAT) {

    MDV_init_flat(grid->proj_origin_lat,
		  grid->proj_origin_lon,
		  grid->proj_params.flat.rotation,
		  comps);

  } else if (grid->proj_type == MDV_PROJ_LATLON) {

    MDV_init_latlon(comps);

  } else if (grid->proj_type == MDV_PROJ_LAMBERT_CONF) {

    MDV_init_lc2(grid->proj_origin_lat,
                 grid->proj_origin_lon,
                 grid->proj_params.lc2.lat1,
                 grid->proj_params.lc2.lat2,
                 comps);

  } else {

    fprintf(stderr, "ERROR - MDV_init_proj\n");
    fprintf(stderr, "MDV proj type %d not supported\n",
	    grid->proj_type);
    exit (-1);

  }

}

/***********************************
 * initialize flat earth projection
 */

void MDV_init_flat(double origin_lat,
		   double origin_lon,
		   double rotation,
		   mdv_grid_comps_t *comps)

{

  comps->proj_type = MDV_PROJ_FLAT;

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

void MDV_init_lc2(double origin_lat,
		  double origin_lon,
		  double lat1,
		  double lat2,
		  mdv_grid_comps_t *comps)

{

  double  t1, t2, t0n, t1n;

  /* check illegal values */                                             
  if (fabs(origin_lat - 90.0) < TINY_ANGLE)
    fprintf(stderr, "ERROR - MDV_init_lc2\n"), exit(-1); /* error handling!? */
  if (fabs(origin_lat + 90.0) < TINY_ANGLE)
    fprintf(stderr, "ERROR - MDV_init_lc2\n"), exit(-1); /* error handling!? */
  if (fabs(lat1 - 90.0) < TINY_ANGLE)
    fprintf(stderr, "ERROR - MDV_init_lc2\n"), exit(-1); /* error handling!? */
  if (fabs(lat1 + 90.0) < TINY_ANGLE)
    fprintf(stderr, "ERROR - MDV_init_lc2\n"), exit(-1); /* error handling!? */
  if (fabs(lat2 - 90.0) < TINY_ANGLE)
    fprintf(stderr, "ERROR - MDV_init_lc2\n"), exit(-1); /* error handling!? */
  if (fabs(lat2 + 90.0) < TINY_ANGLE)
    fprintf(stderr, "ERROR - MDV_init_lc2\n"), exit(-1); /* error handling!? */
  if (fabs(lat2-lat1) < TINY_ANGLE)
    fprintf(stderr, "ERROR - MDV_init_lc2 can't handle one base lat\n"), exit(-1);

  comps->proj_type = MDV_PROJ_LAMBERT_CONF;

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

  t1 = tan( M_PI_4 + comps->lc2_lat1_rad / 2);
  t2 = tan( M_PI_4 + comps->lc2_lat2_rad / 2);
  comps->lc2_n = log( cos(comps->lc2_lat1_rad)/cos(comps->lc2_lat2_rad))
                 / log(t2/t1);

  t1n = pow(t1, comps->lc2_n);
  comps->lc2_F = cos(comps->lc2_lat1_rad) * t1n / comps->lc2_n;

  t0n = pow( tan(M_PI_4 + comps->origin_lat_rad/2), comps->lc2_n);
  comps->lc2_rho = PJG_get_earth_radius() * comps->lc2_F / t0n;
  
  comps->latlon2xy = lc2_latlon2xy;
  comps->xy2latlon = lc2_xy2latlon;
}


/******************************
 * initialize latlon projection
 */

void MDV_init_latlon(mdv_grid_comps_t *comps)

{

  comps->proj_type = MDV_PROJ_LATLON;

  comps->latlon2xy = ll_latlon2xy;
  comps->xy2latlon = ll_xy2latlon;

}

/*********************
 * generic latlon2xy()
 */

void MDV_latlon2xy(mdv_grid_comps_t *comps,
		   double lat, double lon,
                   double *x, double *y)
{

  comps->latlon2xy(comps, lat, lon, x, y);

}

/*********************
 * generic xy2latlon()
 */

void MDV_xy2latlon(mdv_grid_comps_t *comps,
		   double x, double y,
		   double *lat, double *lon)
     
{
  
  comps->xy2latlon(comps, x, y, lat, lon);
  
}

/******************
 * flat latlon2xy()
 */

static void flat_latlon2xy(mdv_grid_comps_t *comps,
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

static void flat_xy2latlon(mdv_grid_comps_t *comps,
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

static void ll_latlon2xy(mdv_grid_comps_t *comps,
			 double lat, double lon,
			 double *x, double *y)

{
  *y = lat;
  *x = lon;
}
     
/********************
 * latlon xy2latlon()
 */

static void ll_xy2latlon(mdv_grid_comps_t *comps,
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

static void lc2_latlon2xy(mdv_grid_comps_t *comps,
                          double lat, double lon,
                          double *x, double *y)
{
  double r, theta, tn;

  lat *= DEG_TO_RAD;
  lon *= DEG_TO_RAD;

  theta = comps->lc2_n * (lon - comps->origin_lon_rad);

  tn = pow( tan(M_PI_4 + lat / 2), comps->lc2_n);
  r = PJG_get_earth_radius() * comps->lc2_F / tn;

  *x = r * sin (theta);
  *y = comps->lc2_rho - r * cos(theta);
}

static void lc2_xy2latlon(mdv_grid_comps_t *comps,
                          double x, double y,
                          double *lat, double *lon)
{
  double r, theta, rn, yd;

  yd = (comps->lc2_rho - y);
  theta = atan2(x, yd);
  r = sqrt(x * x + yd * yd);
  if (comps->lc2_n < 0.0) {
    r *= -1.0;
  }

  *lon = (theta / comps->lc2_n + comps->origin_lon_rad) * RAD_TO_DEG;
  *lon = PJGrange180(*lon);

  if (fabs(r) < TINY_FLOAT) {
    *lat = ((comps->lc2_n < 0.0) ? -90.0 : 90.0);
  }
  else {
    rn = pow( PJG_get_earth_radius() * comps->lc2_F / r, 1 / comps->lc2_n);
    *lat = (2.0 * atan(rn) - M_PI_2) * RAD_TO_DEG;
  }

  *lat = PJGrange180( *lat);
}

