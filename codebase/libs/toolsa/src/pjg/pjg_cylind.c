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
/***************************************************************
 * PJG_CYL:  Routines to transform to/from Cylindrical equidistant
 *  Corrdinate systems
 *  F. Hage   Feb 1994.
 */

#include <math.h>
#include <toolsa/pjg.h>
#include <toolsa/sincos.h>
#include "pjg_int.h"

#define DEGREES_PER_RADIAN (180.0/M_PI)
/***************************************************************
 * Convert World Lat, Lon's to Cylindrical Equidistant Phi,Lam's 
 *  Inputs: npoints. - Number of points in vectors to transform
 *          phi0     - Latitude on Earth of the center of the CYL coord system
 *          lam0     - Longitude on Earth of the center of the CYL coord system
 *          lat[]    - Array of Earth Latitudes (degrees)
 *          lon[]    - Array of Earth Longitudes (degrees)
 *
 *  Outputs: phi[]   - Array of Latitudes in CYl Proj (degrees)
 *           lam[]   - Array of Longitudes in CYl Proj (degrees)
 *
 * Does not modify the Input arrays 
 *
 * Returns the number of points transformed.
 * Based on routines by R. Bullock. - Current version: F. Hage Feb 1994
 */
int PJG_latlon_to_cyl_equidist(int npoints, double phi0, double lam0, double *lat,double *lon, double *phi, double *lam)
{
    int i;
    double sp0, cp0, sp, cp, sl, cl;
    double x, y, z;
    double loc_lat,loc_lon;

    phi0 /= DEGREES_PER_RADIAN;
    lam0 /= DEGREES_PER_RADIAN;

    ta_sincos(phi0, &sp0, &cp0);

    for(i=0; i < npoints; i++) {

	loc_lat = lat[i] / DEGREES_PER_RADIAN;
	loc_lon = lon[i] / DEGREES_PER_RADIAN;

	ta_sincos(loc_lat, &sp, &cp);
	ta_sincos(loc_lon - lam0, &sl, &cl);

        x = sp * sp0 + cp * cp0 * cl;
        y = -cp * sl;
        z = sp * cp0 - cp * sp0 * cl;

        phi[i] = asin(z) * DEGREES_PER_RADIAN;

        if ((x*x + y*y) < 0.000001 )
           lam[i] = 0.0;
        else
           lam[i] = atan2(y, x) * DEGREES_PER_RADIAN;
    }
    return npoints;
}

/***************************************************************
 * Convert Cylindrical Equidistant Phi,Lam to world, Lat lon.
 *
 *  Inputs: npoints. - Number of points in vectors to transform
 *          phi0     - Latitude on Earth of the center of the CYL coord system
 *          lam0     - Longitude on Earth of the center of the CYL coord system
 *          phi[]   - Array of Latitudes in CYl Proj (degrees)
 *          lam[]   - Array of Longitudes in CYl Proj (degrees)
 *
 *  Outputs:lat[]    - Array of Earth Latitudes (degrees)
 *          lon[]    - Array of Earth Longitudes (degrees)
 *
 * Does not modify the Input arrays 
 *
 * Returns the number of points transformed.
 * Based on routines by R. Bullock. Current version: F. Hage Feb 1994
 */
int PJG_cyl_equidist_to_latlon(int npoints,double phi0, double lam0, double *phi, double *lam, double *lat,double *lon)
{
    int i;
    double sp, cp, sp0, cp0, sl, cl, sl0, cl0;
    double x, y, z;
    double loc_phi,loc_lam;

    /* convert to radians */
    phi0 /= DEGREES_PER_RADIAN;
    lam0 /= DEGREES_PER_RADIAN;

    ta_sincos(phi0, &sp0, &cp0);
    ta_sincos(lam0, &sl0, &cl0);

    for(i=0; i < npoints; i++) {
        loc_phi = phi[i] / DEGREES_PER_RADIAN;
        loc_lam = lam[i] / DEGREES_PER_RADIAN;
	
	ta_sincos(loc_phi, &sp, &cp);
	ta_sincos(loc_lam, &sl, &cl);

        x = cp * cp0 * cl * cl0 + cp * sl * sl0 - sp * sp0 * cl0;
        y = cp * cp0 * cl * sl0 - cp * sl * cl0 - sp * sp0 * sl0;

        z = sp * cp0 + cp *sp0 *cl;
        lat[i] = asin(z) * DEGREES_PER_RADIAN;

        if ((x*x + y*y)  < 0.000001 )
           lon[i] = 0.0;
        else
           lon[i] =  atan2( y, x ) * DEGREES_PER_RADIAN;
    }

    return npoints;
}

/************************************************************************************
 * CYL_TO_LATLON: Convert cylindrical equidistant phi, lambda to world lat lon.
 *   phi0, lam0 are the world coordinates of the center of the phi,lam coord system
 *
 */

int  cyl_to_latlon(double phi0, double lam0, double phi, double lam, double *lat, double *lon)
{
    double a,b,c,d;
    double sin_phi, cos_phi;
    double sin_phi0, cos_phi0;
    double cos_lam;
    double local_lat;
    double local_lon;

    /* first convert everything to radians */
    phi0 /= DEGREES_PER_RADIAN;
    lam0 /= DEGREES_PER_RADIAN;
    phi /= DEGREES_PER_RADIAN;
    lam /= DEGREES_PER_RADIAN;

    ta_sincos(phi, &sin_phi, &cos_phi);
    ta_sincos(phi0, &sin_phi0, &cos_phi0);
    cos_lam = cos(lam);

    a = sin_phi * cos_phi0 + (cos_phi * sin_phi0 * cos_lam);
    local_lat = asin(a);
    
    b = (cos_phi * cos_lam) / (cos(local_lat) * cos_phi0);
    d = b - tan(local_lat) * tan(phi0);
    if(d > 1.0) {
        c = 0.0;
    } else {
        c = acos(d);
        if(lam < 0.0) c = -c;
    }
    local_lon = lam0 + c;

    /* now convert everything back to degrees */
    *lat = local_lat *  DEGREES_PER_RADIAN;
    *lon = local_lon * DEGREES_PER_RADIAN;

    return 0;
}
