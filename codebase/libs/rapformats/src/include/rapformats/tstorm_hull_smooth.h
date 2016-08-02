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


/*********************************************************************
 * File: ts_hull_.h
 *
 * Marty Petach, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * April 1999
 *
 * Given an initial TITAN storm polygon, the routine finds the
 * relative convex hull between an inner boundary polygon and an
 * outer boundary polygon. Boundary polygons are as specified by
 * multipliers of the original ray length.
 *********************************************************************/


#ifndef TSTORM_HULL_H
#define TSTORM_HULL_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <euclid/geometry.h>
#include <toolsa/toolsa_macros.h>
#include <rapformats/tstorm_spdb.h>

#define HULL_SHRINK_LIMIT 0.
#define HULL_GROWTH_LIMIT 10000.
#define BND_OUTER 0
#define BND_INNER 1

/*
 * Struct to hold polygon rays and intermediate processing info
 */
 
typedef struct
{
  double angle;
  double radius_ori;  /* original ray length */
  double radius_shr;  /* inner limit ray length */
  double radius_gro;  /* outer limit ray length */
  double radius_fin;  /* final polygon ray length */
  int on_outer;       /* 0: inside outer hull, 1: point on outer hull */
  int next_ray;       /* index number of next valid ray; skip if hide = -1 */
  int hide;           /* 0: ray is on final hull, 1: ray to be skipped */
  int visited;        /* 0: ray not visted, 1: ray "visited" in main loop */
  fl32 lat_ori;
  fl32 lon_ori;
  fl32 lat_shr;
  fl32 lon_shr;
  fl32 lat_gro;
  fl32 lon_gro;
} ts_hull_ray_t;
  
  
/* Forward function prototypes */

/*********************************************************************
 * tstorm_hull_smooth -        Finds the relative hull between inner
 *                             and outer limit polygons. Inner and outer
 *                             limit polygons are defined by multipliers
 *                             of the original ray length.
 *
 * Given a SPDB TITAN storm header, polygon entry,
 * an inner boundary multiplier (inner_bnd_multiplier), and
 * an outer boundary multiplier (outer_bnd_multiplier),
 * the routine finds the relative convex hull between the
 * inner boundary polygon and the outer boundary polygon.
 *
 * The relative convex hull is the path a rubber band
 * would take if it were forced to travel between the
 * inner boundary polygon and the outer boundary polygon.
 *
 * An outer_bnd_multiplier of 2.0 makes the outer boundary
 * polygon rays twice as long as the original rays.
 * An outer_bnd_multiplier of 1.0 makes the outer boundary
 * polygon rays the same length as the original rays.
 * An outer_bnd_multiplier of 0.5 makes the outer boundary
 * polygon rays half as long as the original polygon rays.
 *
 * An inner_bnd_multiplier of 0.5 makes the inner boundary
 * polygon rays half as long as the original rays.
 * An inner_bnd_multiplier of 1.0 makes the inner boundary
 * polygon rays the same length as the original rays.
 * An inner_bnd_multiplier of 2.0 makes the inner boundary
 * polygon rays twice as long as the original polygon rays.
 *
 * The boundary multipliers must be in the range:
 *      HULL_SHRINK_LIMIT < bnd_multiplier =< HULL_GROWTH_LIMIT
 * where HULL_SHRINK_LIMIT is 0 and HULL_GROWTH_LIMIT is 10000.
 *
 * If the inner_bnd_multiplier or outer_bnd_multiplier are outside
 * the limits specified above, or the inner boundary polygon is larger
 * than the outer boundary polygon, an error messsage will be printed
 * and the original polygon will be returned in hull_poly.
 *
 * Debug:
 *  0 no debugging output
 *  1 text messages only
 *  2 text messages and writes 2 debug files to current directory:
 *    tstorm_hull1.debug (inner, original, outer polygons) and
 *    tstorm_hull2.debug (final hull)
 *
 * Upon completion, the routine sets the lat/lon values of the final
 * relative convex hull polygon (hull_poly), and sets the number
 * of points in the relative convex hull polygon (hull_poly_npts).
 */

void tstorm_hull_smooth(const tstorm_spdb_header_t *header,
			const tstorm_spdb_entry_t *entry,
			const double inner_bnd_multiplier,
			const double outer_bnd_multiplier,
			tstorm_polygon_t *hull_poly,
			int *hull_poly_npts,
			const double lead_time,
			const int debug);

/******************************************************************
 * See notes for tstorm_hull_smooth
 *  This allows the user to specify whether to grow the storm
 *  or not
 */
void tstorm_growth_hull_smooth(const tstorm_spdb_header_t *header,
			       const tstorm_spdb_entry_t *entry,
			       const double inner_bnd_multiplier,
			       const double outer_bnd_multiplier,
			       tstorm_polygon_t *hull_poly,
			       int *hull_poly_npts,
			       const double lead_time,
			       const int debug, const int grow);


/*********************************************************************
 * ts_hull_print_ray_struct - prints the struct values in the ray list
 */
  
void ts_hull_print_ray_struct(ts_hull_ray_t *rlist,
			      int rpts);


/*********************************************************************
 * ts_hull_print_ray_coords - prints the ray coordinates
 */
  
void ts_hull_print_ray_coords(ts_hull_ray_t *rlist,
			      int rpts,
			      int skip_negs);


/*********************************************************************
 * ts_hull_write_storm - writes final hull ray data to file
 *                       for debugging
 */

int ts_hull_write_storm(char *outname,
			ts_hull_ray_t *rlist,
			int npts);


/*********************************************************************
 * ts_hull_write_bounds - writes inner boundary, original polygon,
 *                        and outer boundary data to file for
 *                        debugging                            
 */                               
 
int ts_hull_write_bounds(char *outname,                              
			 ts_hull_ray_t *rlist,
			 int npts);


#endif


#ifdef __cplusplus                                  
}
#endif    
