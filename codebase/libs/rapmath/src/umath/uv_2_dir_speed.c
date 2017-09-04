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

#include <toolsa/toolsa_macros.h>

/*----------------------------------------------------------------*/
/*
 * Convert from u, v to speed, dir (clockwise from true north, i.e. radar 
 * coords).
 */
void 
uv_2_dir_speed_d(double u, double v, double *dir, double *speed)
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

void 
uv_2_dir_speed(float u, float v, float *dir, float *speed)
/* u, v  - in km/h, m/s, whatever
 * dir   - in degrees
 * speed - same units as u and v
 */
{
  double dir_double, speed_double;
  
  uv_2_dir_speed_d((double)u, (double)v, &dir_double, &speed_double);
  
  *dir = (float)dir_double;
  *speed = (float)speed_double;
}

/*----------------------------------------------------------------*/
/*
 * Convert from u, v to speed, dir (counter-clockwise from true north, 
 *                                  i.e. wind convention)
 */
void
uv_2_wind_dir_speed(float u, float v, float *dir, float *speed)
/* u, v  - in km/h, m/s, whatever
 * dir   - in degrees
 * speed - same units as u and v   
 */
{
   uv_2_dir_speed( u, v, dir, speed );

   /*
    * Convert the direction to the convention of "from wind to origin"
    */
   *dir += 180.0;
   if ( *dir > 360.0 ) {
      *dir -= 360.0;
   }
}
