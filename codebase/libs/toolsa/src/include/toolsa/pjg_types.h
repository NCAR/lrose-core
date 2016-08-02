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
#ifndef PJG_TYPES_WAS_INCLUDED
#define PJG_TYPES_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

#define PJG_ARTCC                       1
#define PJG_OBLIQUE_STEREOGRAPHIC 	2
#define PJG_LAMBERT_CONFORMAL2   	3
#define PJG_TRANSVERSE_MERCATOR   	4
#define PJG_FLAT  			5       /* Same as PJG_CARTESIAN */
#define PJG_CARTESIAN  			5
#define PJG_STEREOGRAPHIC_ELLIPSOID 	6
#define PJG_DENVER_ARTCC		7
#define PJG_LATLON			8
#define PJG_CYL_EQUIDISTANT		9
#define PJG_RADIAL                     21       /* I.E RADAR's etc, azimuth, elevation, & radial dist */ 
#define PJG_SPHERICAL                  21       /* Same as  PJG_RADIAL */
#define PJG_UNKNOWN                    99

#define PJG_MAX_COMPUTE 9
typedef struct PJGstruct_ {
  int type;
  double compute[PJG_MAX_COMPUTE];
  } PJGstruct;

/*
 * pjg_flat_grid_t
 *
 * The grid is applicable to the flat amd latlon projections only
 *
 * The grid is defined by a lat/lon reference point, which can be 
 * anywhere.  It is not constrained to be the center or even within the 
 * grid, although it is likely that the reference point will be somewhere 
 * within the grid. Grid rotation is relative to True North. With 
 * rotation == 0, y direction is N+/S- and x direction is E+/W-.
 *
 * The min_x and min_y values define the leftmost and bottom grid points
 * relative to the reference point in terms of the grid units.
 * Grid points refer to the center of a grid box. The dx and dy define the
 * distance between grid points and nx and ny define the number of grid points
 * in each dimension.  Grid indices are ordered such that min_x and min_y 
 * constitute the (0,0) point. The x index moves the fastest.
 *
 * Grid units are km, nm or deg, indicated by PJG_UNITS_KM,
 * PJG_UNITS_NM or PJG_UNITS_DEG.
 *
 */

enum pjg_grid_units {
  PJG_UNITS_KM,
  PJG_UNITS_NM,
  PJG_UNITS_DEG
};

#define pjg_units_str(a) \
(((a) == PJG_UNITS_KM) ? "km" : (((a) == PJG_UNITS_NM) ? "nm" : "deg"))
    
typedef struct
{

  double ref_lat, ref_lon; /* Lat/Lon of reference point */
  double rotation;         /* grid rotation clockwise relative
			    * to True North */
  double min_x, min_y;     /* Position of (0,0) point relative
			    * to ref pt */
  double dx, dy;           /* Grid point spacing  */
  int nx, ny;              /* Number of grid points in each dimension */
  int units;               /* PJG_UNITS_KM, PJG_UNITS_NM or PJG_UNITS_DEG */
  int type;                /* PJG_FLAT or PJG_LATLON */

} pjg_grid_geom_t;

typedef struct {
  double lat, lon;
} PJG_wpt;

#ifdef __cplusplus
}
#endif

#endif /* PJG_TYPES_WAS_INCLUDED */

