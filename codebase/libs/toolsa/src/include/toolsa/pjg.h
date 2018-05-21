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
#ifdef __cplusplus
 extern "C" {
#endif
#ifndef PJG_WAS_INCLUDED
#define PJG_WAS_INCLUDED

#include <toolsa/pjg_types.h>
#include <toolsa/pjg_flat.h>

/* OVERVIEW
	Projective Geometry routines
	Spherical Earth model radius = 6371.204 km, except where noted
	All latitudes in range (90, -90)
	All longitudes in the range [180, -180]

	Reference : "Map Projections used by the US Geological Survey"
		Bulletin 1532, 2nd Edition, John P. Snyder

	KEYWORD: Projective Geometry, map, maps, coordinate transformations
	KEYWORD: Spherical Geometry, latitude, longitude
	KEYWORD: Stereographic, Lambert Conformal, Transverse Mercator

    General:
	PJGrange360     : put angle in range (0,360)
	PJGrange180     : put angle in range (-180,+180)

    General routines for working with any of the projections:
	PJGname		: name of projection
	PJGinit		: initialize transform
	PJGlatlon2xy 	: transform lat, lon to x,y 
	PJGxy2latlon 	: transform x,y to lat, lon

    Flat:
        PJGflat_init	  : initialize transform
	PJGflat_latlon2xy : transform lat, lon to x,y 
	PJGflat_xy2latlon : transform x,y to lat, lon

    Stereographic, arbitrary tangent point and origin:
	PJGps_init	: initialize transform
	PJGps_latlon2xy : transform lat, lon to x,y 
	PJGps_xy2latlon : transform x,y to lat, lon

    Stereographic, arbitrary tangent point and origin, ellipsoidal earth:
	PJGpse_init	: initialize transform
	PJGpse_latlon2xy : transform lat, lon to x,y 
	PJGpse_xy2latlon : transform x,y to lat, lon

    Lambert Conformal with two standard parellels (spherical earth):
	PJGlc2_init	 : initialize transform
	PJGlc2_latlon2xy : transform lat, lon to x,y 
	PJGlc2_xy2latlon : transform x,y to lat, lon

    Transverse Mercator:
	PJGtm_init	: initialize transform
	PJGtm_latlon2xy : transform lat,lon to x,y
	PJGtm_xy2latlon : transform x,y to lat,lon

    "Flat earth, rotated" Projection
	This projection surface is tangent at some point (lat0, lon0) and 
	has a y axis rotated from True North by some angle.  

    	We call it "flat" because it should only be used where the spherical
    	geometry of the earth is not significant. 

	PJGflat_init	: initialize transform
	PJGflat_latlon2xy: lat, lon to x,y
	PJGflat_xy2latlon: x,y to lat,lon

    "Denver Artcc"
	Uses same projection as Denver ARTCC code. Polar Stereographics,
	ellipsiodal earth, second order approximation.

	PJGart_init	: initilize transform
	PJGart_latlon2xy: lat, lon to x,y

    "Clyindrical_Equidistant"
	PJG_latlon_to_cyl_equidist: COnvert world lat lon to Cylindrical phi, lambda
	PJG_cyl_equidist_to_latlon: Convert Cylindrical phi, lambda to world lat lon.

    "LatLon"
	This maps lat <-> ay + y0 and lon <-> bx + x0.  
 */


/*****************************************************************************
 * earth radius
 */

extern void PJG_set_earth_radius(double val);
extern double PJG_get_earth_radius();

/***************** General routines **************/

extern double PJGrange360( double angle);
extern double PJGrange180( double angle);
/*
 * put the angle into the range (0,360)  or  (-180,180)
 *
 * angle = input angle in degrees 
 * Return the angle in the givenn range 
 */


/* free up struct */
extern void PJG_free_struct(PJGstruct *ps);

/************** Specific Projections **********************************/

/*********** Oblique Polar Stereographic ***************/

extern void PJGps_init( double lat0, double lon0, double lat1,
			   double lon1, double scale );
/* precompute constants for oblique polar stereographic projection

   lon0, lat0 = origin of coordinate system 
   lat1, lon1: tangent point of projection (lat = +/-90 is ok)
   scale: scale factor at tangent point, usually = 1.0
 
   you must call this before you call other PJGps_xxxx routines
 */

extern void PJGps_latlon2xy( double lat, double lon, double *x, 
				double *y );
/*
 *  Input : lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in km, polar stereographic projection, 
 *
 *  Will not handle case where lat = -lat0 or lon = +/- lon0
 */

extern void PJGps_xy2latlon( double x, double y, double *lat, 
				double *lon);
/*
 *  Input: x, y in km, polar stereographic projection, 
 *  Output : lat, lon in degrees (lat N+, lon E+)
 *
 */

/*********** Oblique Polar Stereographic, ellipsoidal earth  ***************/

extern void PJGpse_init( double lat0, double lon0, double lat1,
			 double lon1, double semimajor_axis, double scale );
/* precompute constants for oblique polar stereographic projection

   lon0, lat0 = origin of coordinate system (lat = +/- 90 currently NOT ok)
   lat1, lon1: tangent point of projection
   semimajor_axis: semimajor axis of the ellipsoidal earth in km
   scale: scale factor at tangent point, usually = 1.0 *
 
   you must call this before you call other PJGpse_xxxx routines
 */

extern void PJGpse_latlon2xy( double lat, double lon, double *x, double *y);
/*
 *  Input : lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in km, polar stereographic projection, ellipsoidal earth 
 *
 *  Will not handle case where lat = -lat0 or lon = +/- lon0
 */

extern void PJGpse_xy2latlon( double x, double y, double *lat, double *lon);
/*
 *  Input: x, y in km, polar stereographic projection, ellipsoidal earth
 *  Output : lat, lon in degrees (lat N+, lon E+)
 *
 */

/********** Lambert Conformal ******************************/

extern int PJGlc2_init( double lat0, double lon0, double lat1, double lat2);
/* precompute constants for lambert conformal, two standard parellels
   spherical Earth

   lon0, lat0 =  origin of the projection's coord. system
   lat1, lat2: 	 standard parellels; 
 
   you must call this before you call other PJGlc2_xxx routines
   return FALSE if any of the following are true:
	lat0, lat1, lat2 = +/- 90 deg
 */

extern void PJGlc2_latlon2xy(double lat, double lon, double *x, double *y);
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in lambert conformal projection 
 *     
 */

extern void PJGlc2_xy2latlon(double x, double y, double *lat, double *lon);
/*
 *  Input: x, y in lambert conformal projection
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *     
 */


/********** Transverse Mercator ********************/

extern void PJGtm_init(double lat0, double central_lon, double scale);
/* precompute constants for transverse mercator 
   spherical Earth

   central_lon = central longitude (cylinder is tangent along this longitude)
   lat0 =  origin of the projection's coord. system at lat0, central_lon
   scale = scale factor along the central meridian. = .9996 for UTM and 1:250,000
	quadrangle maps.
 
   you must call this before you call other PJGtm_xxx routines
 */

extern void PJGtm_latlon2xy(double lat, double lon, double *x, double *y);
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in transverse mercator projection 
 *     
 */

extern void PJGtm_xy2latlon(double x, double y, double *lat, double *lon);
/*
 *  Input: x, y in transverse mercator projection
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *     
 */

/****** Denver ARTCC projection *******/

extern void PJGart_init( void);
extern void PJGart_latlon2xy(double lat, double lon, double *x, double *y);
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y on Denver ARTCC projection plane 
 *     
 */
extern void PJGart_xy2latlon( double x, double y, double *lat, double *lon);
/*
 *  Input: x, y in km, polar stereographic projection, ellipsoidal earth
 *  Output : lat, lon in degrees (lat N+, lon E+)
 *
 * currently uses PJGpse_xy2latlon()
 *
 */

/****** "latlon" projection *******/

extern void PJGll_init( double lat0, double lon0, double xkm_per_deg, double ykm_per_deg);
/*
 * sets up the transformation as y = ykm_per_deg * (lat - lat0),
 *				 x = xkm_per_deg * (lon - lon0)
 *
 * typically you set the scale factors so that aspect ratio in both x and y are equal at
 * the center of the screen.
 */

extern void PJGll_latlon2xy(double lat, double lon, double *x, double *y);
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in km, "latlon" projection
 *     
 */
extern void PJGll_xy2latlon( double x, double y, double *lat, double *lon);
/*
 *  Input: x, y in km, "latlon" projection
 *  Output : lat, lon in degrees (lat N+, lon E+)
 *
 */

/****** Cylindrical Equidistant projection *******/
 
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
extern int PJG_cyl_equidist_to_latlon(int npoints,double phi0, double lam0, double *phi, double *lam, double *lat,double *lon);

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
extern int PJG_latlon_to_cyl_equidist(int npoints, double phi0, double lam0, double *lat,double *lon, double *phi, double *lam);

/************************************************************************************
 * CYL_TO_LATLON: Convert cylindrical equidistant phi, lambda to world lat lon.
 *   phi0, lam0 are the world coordinates of the center of the phi,lam coord system
 *   C language conversion of NMC supplied routine.
 *
 */
extern int  cyl_to_latlon(double phi0, double lam0, double phi, double lam, double *lat, double *lon);


/*********** General Projection routines ***************/
/* The PJGxxx_init() routines store initial values and precomputed values in
   a static area within each module.  In a general purpose program, you may not
   be able to assume that no one else has called PJGxxx_init().  Therefore, this
   following set of routines  stores initial values and precomputed values in
   a "PJGstruct", and the application passes it in to the latlon2xy() or xy2latlon()
   routine.  Generally most convenient to use from the PJGlatlon2xy() or 
   PJGxy2latlon() routines.

   Note that you must free() the PJGstruct when you are all done with it.
   
   Note that the cylindrical equidistant is not currently available through this
   method.
 */

extern char *PJGname( int proj_type);
/* return pointer to static string containing name of projection proj_type */

extern PJGstruct *PJGinit( int proj_type, double *params);
/* 
 * initialize the proj_type projection with the list of parameters
 *   The meaning of the parameters is specified by the individual
 *   PJGxx_init() call (see below).
 * This returns a pointer to a computation structure that is passed in
 *   to the PJGlatlon2xy and PJGxy2latlon routines, or NULL on error.
 *   This structure stores precomputations and initializations.
 *   The application is reponsible for calling free() on it when done.
 */

extern void PJGlatlon2xy( PJGstruct *proj, double lat, double lon, double *x, double *y);
/*
 * convert lat,lon to x,y for the projection proj_type 
 *
 *  Input : lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in km 
 *
 */

extern void PJGxy2latlon( PJGstruct *proj, double x, double y, double *lat, double *lon);
/*
 * convert x,y to lat,lon for the projection proj_type 
 *
 *  Input : x, y in km 
 *  Output: lat, lon in degrees (lat N+, lon E+)
 *
 */

extern PJGstruct *PJGs_ps_init( double lat0, double lon0, double lat1, double lon1, double scale);
extern void PJGs_ps_latlon2xy( PJGstruct *proj, double lat, double lon, double *x, double *y);
extern void PJGs_ps_xy2latlon( PJGstruct *proj, double x, double y, double *lat, double *lon);

extern PJGstruct *PJGs_lc2_init( double lat0, double lon0, double lat1, double lat2);
extern void PJGs_lc2_latlon2xy( PJGstruct *proj, double lat, double lon, double *x, double *y);
extern void PJGs_lc2_xy2latlon( PJGstruct *proj, double x, double y, double *lat, double *lon);

extern PJGstruct *PJGs_tm_init( double lat0, double central_lon, double scale);
extern void PJGs_tm_latlon2xy( PJGstruct *proj, double lat, double lon, double *x, double *y);
extern void PJGs_tm_xy2latlon( PJGstruct *proj, double x, double y, double *lat, double *lon);

extern PJGstruct *PJGs_pse_init( double lat0, double lon0, double lat1, double lon1, 
				 double semimajor_axis, double scale);
extern void PJGs_pse_latlon2xy( PJGstruct *proj, double lat, double lon, double *x, double *y);
extern void PJGs_pse_xy2latlon( PJGstruct *proj, double x, double y, double *lat, double *lon);

extern PJGstruct *PJGs_art_init( void);
extern void PJGs_art_latlon2xy( PJGstruct *proj, double lat, double lon, double *x, double *y);
extern void PJGs_art_xy2latlon( PJGstruct *proj, double x, double y, double *lat, double *lon);

extern PJGstruct *PJGs_flat_init( double lat0, double lon0, double rot_angle);
extern void PJGs_flat_latlon2xy( PJGstruct *proj, double lat, double lon, double *x, double *y);
extern void PJGs_flat_xy2latlon( PJGstruct *proj, double x, double y, double *lat, double *lon);

extern PJGstruct *PJGs_ll_init( double lat0, double lon0, double xkm_per_deg, double ykm_per_deg);
extern void PJGs_ll_latlon2xy( PJGstruct *proj, double lat, double lon, double *x, double *y);
extern void PJGs_ll_xy2latlon( PJGstruct *proj, double x, double y, double *lat, double *lon);

/*******************************************
 * pjg2mdv_type()
 *
 * Converts pjg projection type to mdv type
 */

extern int pjg2mdv_type(int pjg_type);

/*******************************************
 * mdv2pjg_type()
 *
 * Converts mdv projection type to pjg type
 */

extern int mdv2pjg_type(int mdv_type);


#endif

#ifdef __cplusplus
}
#endif
