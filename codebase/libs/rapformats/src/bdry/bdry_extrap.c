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
 * bdry_extrap.c: Routines for extrapolating boundaries using the
 *                method used in colide.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1998
 *
 * Nancy Rehak
 *
 * Notes: Some of the functions use fixed sized temp arrays.
 *        To guarantee that there is no overflow, the lengths of 
 *        lines passed to these functions are checked in advance. These
 *        are done by limiting the line lengths in Read_ext_lines and
 *        all new lines are copied via Copy_line.
 *
 *********************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <toolsa/os_config.h>
#include <rapformats/bdry.h>
#include <rapformats/bdry_extrap.h>
#include <toolsa/globals.h>
#include <toolsa/mem.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/toolsa_macros.h>

/************************************************************************
 * BDRY_extrapolate(): Extrapolates the given boundary using simple
 *                     extrapolation of each point in the boundary using
 *                     the boundary motion direction and vector.
 *
 * Note: This routines updates the detections in place with the
 *       resulting extrapolation locations.
 */

void BDRY_extrapolate(BDRY_product_t *detections,
		      int num_detections,
		      int extrap_secs)
{
  int det, poly, pt;
  
  for (det = 0; det < num_detections; det++)
  {
    double extrap_km;
    double map_direction;
    
    /*
     * Calculate the distance each point will move in the
     * given extrapolation time.  Note that the detection
     * speed is given in m/s and extrapolation time is given
     * in seconds.
     */

    extrap_km = (detections[det].motion_speed * extrap_secs) / 1000.0;
    
    map_direction =
      BDRY_spdb_to_pjg_direction(detections[det].motion_direction);
    
    /*
     * Now extrapolate each point in each polyline.
     */

    for (poly = 0; poly < detections[det].num_polylines; poly++)
    {
      for (pt = 0; pt < detections[det].polylines[poly].num_pts; pt++)
      {
	double extrap_lat, extrap_lon;
	
	PJGLatLonPlusRTheta(detections[det].polylines[poly].points[pt].lat,
			    detections[det].polylines[poly].points[pt].lon,
			    extrap_km,
			    map_direction,
			    &extrap_lat,
			    &extrap_lon);
	
	detections[det].polylines[poly].points[pt].lat = extrap_lat;
	detections[det].polylines[poly].points[pt].lon = extrap_lon;
	
      } /* endfor - pt */
      
    } /* endfor - poly */
    
  } /* endfor - det */
  
  return;
}

/*----------------------------------------------------------------*/
/*
 * Convert from u, v to speed, dir (clockwise from true north, i.e. radar 
 * coords).
 */
static void 
_uv_2_dir_speed(float u, float v, float *dir, float *speed)
/* u, v  - in km/h, m/s, whatever
 * dir   - in degrees
 * speed - same units as u and v
 */
{
    double d;
    double vv, uu;

    /*
     * Get the direction of the wind vector.
     * wind dir = inv tan(v/u)  (in polar coordinates)
     * Check special cases first.
     */
    vv = (double)v;
    uu = (double)u;
    if (u == 0.0) {
        if (v == 0.0)
	    d = 0.0;
        else if (v < 0)
	    d = PI * -1/2.0;
        else
	    d = PI/2.0;
    } else if (v == 0.0) {
        if (u < 0)
	    d = PI;
        else
	    d = 0.0;
    } else
        d = atan2(vv, uu);

    /*
     * Convert to degrees.
     * 1 radian = (180/pi) degrees
     */
    d *= RAD_TO_DEG;

    /*
     * Convert polar to "radar" coordinates.
     */
    d = 90.0 - d;

    /*
     * Put in range [0.0, 360)
     */
    while (d < 0.0)
	d += 360.0;
    *dir = (float)d;

    /*
     * Get the speed of the wind vector.
     * wind speed = sqrt(u^2 + v^2)
     */
    d = u*u + v*v;
    d = sqrt(d);
    *speed = (float)d;
}

/************************************************************************
 * BDRY_extrap_pt_motion():  Extrapolates the given boundary using the
 *                           motion vectors associated with each point
 *                           on the boundary
 */

void BDRY_extrap_pt_motion(BDRY_product_t *detections,
		           int num_detections,
		           int extrap_secs) 
{
  int det, poly, pt;

  float lat, lon;
  float ucomp, vcomp;
  float bdry_pt_dir, bdry_pt_speed;

  double extrap_km;
  double extrap_lat, extrap_lon;
  
  for (det = 0; det < num_detections; det++)
  {
    /*
     * Extrapolate each point in each polyline.
     */
    for (poly = 0; poly < detections[det].num_polylines; poly++)
    {
      for (pt = 0; pt < detections[det].polylines[poly].num_pts; pt++)
      { 
         lat   = (float) detections[det].polylines[poly].points[pt].lat;
	 lon   = (float) detections[det].polylines[poly].points[pt].lon;
         ucomp = (float) detections[det].polylines[poly].points[pt].u_comp;
	 vcomp = (float) detections[det].polylines[poly].points[pt].v_comp;

         /* 
          * If the values of u and v are valid, move the point accordingly.
	  * If the values are not valid, do not move the point at all, i.e.
	  * do nothing.
	  */
	 if( ucomp != BDRY_VALUE_UNKNOWN &&
             vcomp != BDRY_VALUE_UNKNOWN ) {

	    /*
	     * Find the speed and direction of the boundary at this
	     * point.  Note that speed is in m/s and direction is
	     * in radar degrees.
	     */
	    _uv_2_dir_speed( ucomp, vcomp, &bdry_pt_dir, &bdry_pt_speed );
	 
            /*
             * Calculate the distance the point will move in the
             * given extrapolation time.  
             */
            extrap_km = (bdry_pt_speed * extrap_secs) / 1000.0;
	
	    PJGLatLonPlusRTheta(lat, lon,
			        extrap_km,
			        bdry_pt_dir,
			        &extrap_lat,
			        &extrap_lon);
	
	    detections[det].polylines[poly].points[pt].lat = extrap_lat;
	    detections[det].polylines[poly].points[pt].lon = extrap_lon;

	 } /* endif */
	
      } /* endfor - pt */
      
    } /* endfor - poly */
    
  } /* endfor - det */
  
  return;
}
