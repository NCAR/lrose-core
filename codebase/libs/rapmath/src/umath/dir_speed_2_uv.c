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
#include <rapmath/trig.h>

/*----------------------------------------------------------------*/
/*
 * Convert from speed, dir (clockwise from true north, i.e. radar coords) 
 * to u, v.
 */ 
void
dir_speed_2_uv(float speed, float dir, float *u, float *v)
/* speed - in km/h, m/s, whatever
 * dir   - in degrees
 * u, v  - same units as speed
 */
{
    double d, s, uu, vv;
    double sind, cosd;

    /*
     * Convert direction from radar to polar coordinates.
     */
    s = (double)speed;
    d = (double)(90.0 - dir);

    /*
     * Put in range [0.0, 360)
     */
    while (d < 0.0)
        d += 360.0;

    /*
     * Convert degrees to radians.
     * 1 degree = (pi/180) radians
     */
    d *= DEG_TO_RAD;

    /*
     * This is essentially a conversion from polar coordinates
     * to cartesian coordinates
     */
    
    rap_sincos(d, &sind, &cosd);
    vv = s * sind;
    uu = s * cosd;

    *u = (float)uu;
    *v = (float)vv;
}

/*----------------------------------------------------------------*/
/*
 * Convert from speed, dir (counter-clockwise from true north, 
 *                          i.e. wind convention)
 * to u, v.
 */
void
wind_dir_speed_2_uv(float speed, float dir, float *u, float *v)
/* speed - in km/h, m/s, whatever
 * dir   - in degrees
 * u, v  - same units as speed
 */
{
  /*
   * Convert the direction to the convention of "from wind to origin"
   */
   dir += 180.0;
   if ( dir > 360.0 ) {
      dir -= 360.0;
   }

   dir_speed_2_uv( speed, dir, u, v );
}

