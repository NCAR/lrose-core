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
 * File: ts_hull_smooth.c
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

#include <rapformats/tstorm_hull_smooth.h>
#include <euclid/geometry.h>

#define SEARCH_FORWARD 1
#define SEARCH_BACKWARD -1

/*
 * Forward function declarations
 */

static int ts_hull_is_concave(double r0,
			      double a1,
			      double r1,
			      double a2,
			      double r2,
			      double *interior_angle);

static int ts_hull_rt2xy(double radius,
			 double ang_deg,
			 double *x,
			 double *y);

static int ts_hull_next_valid_ray(int current,
				  ts_hull_ray_t *rlist,
				  int rpts);

static int ts_hull_prev_valid_ray(int current,
				  ts_hull_ray_t *rlist,
				  int rpts);

static int ts_hull_get_valid_ray(int current,
				 ts_hull_ray_t *rlist,
				 int rpts,
				 int direction);

static int ts_hull_link_skip_hidden(ts_hull_ray_t *rlist,
				    int rpts);

static int ts_hull_mark_duplicate_zeros(ts_hull_ray_t *rlist,
					int rpts);

static int ts_hull_calc_limits(ts_hull_ray_t *rlist,
			       int rpts,
			       const double inner_bnd_multiplier,
			       const double outer_bnd_multiplier,
			       const tstorm_spdb_header_t *header,
			       const tstorm_spdb_entry_t *entry,
			       const double lead_time,
                               const int grow);

static double ts_hull_get_angle_between_rays(double ray1_angle,
					     double ray2_angle);

static void ts_hull_find_bnd_intersect(ts_hull_ray_t *rlist,
				       int ray_npts,
				       int r0_ray_idx,
				       int r2_ray_idx,
				       int bnd_type,
				       int *extreme_idx,
				       double *extreme_ang,
				       int debug);

static int ts_hull_check_more_rays_to_process(ts_hull_ray_t *rlist,
					      int ray_npts);

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
                        const double lead_time, const int debug) 
{
   tstorm_growth_hull_smooth( header, entry, inner_bnd_multiplier,
                              outer_bnd_multiplier, hull_poly,
			      hull_poly_npts, lead_time, debug, 1 );
}

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
                               const int debug, const int grow)

{

  ts_hull_ray_t ray_list[N_POLY_SIDES];
  int ray_npts;

  int point_changed;
  int r1_concave;
  double r0, a1, r1, a2, r2;
  double cur_ang;

  int r0_ray_idx;
  int r1_ray_idx;
  int r2_ray_idx;

  int iret;
  int i, j;
  /* int hit_outer = FALSE; */

  double interior_ang;
  double extreme_ang;
  int extreme_idx;
  int bnd_type;

  int beg_r, end_r, done;
  int more_rays_to_process;
  int num_no_point_change;

  /*
   * Initialize the number of ray points
   */

  ray_npts = header->n_poly_sides;

  cur_ang = header->poly_start_az;
  for (i = 0; i < header->n_poly_sides; i++)
  {
    ray_list[i].radius_ori =
      entry->polygon_radials[i] * entry->polygon_scale;
    ray_list[i].angle = cur_ang;
    cur_ang += header->poly_delta_az;
  }

  /*
   * Compute inner, outer boundary radial lengths
   */

  iret = ts_hull_calc_limits(ray_list,
			     ray_npts,
			     inner_bnd_multiplier,
			     outer_bnd_multiplier,
			     header,
			     entry,
			     lead_time,
                             grow);
  if (iret != 0)
  {
    /*
     * Error condition - load original data into hull_poly and return
     */

     tstorm_spdb_load_growth_polygon(header,
			             entry,
                                     hull_poly,
			             lead_time,
                                     grow);

    *hull_poly_npts = ray_npts;

    hull_poly->pts[*hull_poly_npts].lon = hull_poly->pts[0].lon;
    hull_poly->pts[*hull_poly_npts].lat = hull_poly->pts[0].lat;
    *hull_poly_npts = ray_npts + 1;
    return;
  }

  /*
   * Check for duplicate 0 length rays
   */

  iret = ts_hull_mark_duplicate_zeros(ray_list,
				      ray_npts);

  /*
   * Relink the list, skipping any temporarily hidden links
   */

  iret = ts_hull_link_skip_hidden(ray_list,
				  ray_npts);

  /*
   * Print the list
   */

  if (debug >= 1)
    ts_hull_print_ray_struct(ray_list,
			     ray_npts);

  /*
   * Set initial conditions before main loop
   */

  num_no_point_change = 0;
  more_rays_to_process = TRUE;
  point_changed = TRUE;
  r0_ray_idx = ts_hull_next_valid_ray(0,
				      ray_list,
				      ray_npts);

  /*
   * Check for convex segments Loop until we have visited
   * each ray at least once and no boundary points have changed
   */

  while (more_rays_to_process || point_changed ||
	 (num_no_point_change < ray_npts))
  {

    /*
     * If a point was deleted on last round,
     * backup to previous point and recheck
     */

    if (point_changed)
    {
      r0_ray_idx = ts_hull_prev_valid_ray(r0_ray_idx,
					  ray_list,
					  ray_npts);
      point_changed = FALSE;
    }
    else
      r0_ray_idx = ts_hull_next_valid_ray(r0_ray_idx,
					  ray_list,
					  ray_npts);

    /*
     * Use final radius so we always get updated angles
     */

    r0 = ray_list[r0_ray_idx].radius_fin;

    r1_ray_idx = ts_hull_next_valid_ray(r0_ray_idx,
					ray_list,
					ray_npts);

    r1 = ray_list[r1_ray_idx].radius_fin;

    a1 = ts_hull_get_angle_between_rays(ray_list[r0_ray_idx].angle,
					ray_list[r1_ray_idx].angle);

    r2_ray_idx = ts_hull_next_valid_ray(r1_ray_idx,
					ray_list,
					ray_npts);
    r2 = ray_list[r2_ray_idx].radius_fin;

    a2 = ts_hull_get_angle_between_rays(ray_list[r1_ray_idx].angle,
					ray_list[r2_ray_idx].angle);

    if (debug >= 1)
      printf("midang 1 2: %5.1f %5.1f ",
	     a1,
	     a2);

    /*
     * Mark r0,r1,r2 rays as having been visited
     */

    ray_list[r0_ray_idx].visited = 1;
    ray_list[r1_ray_idx].visited = 1;
    ray_list[r2_ray_idx].visited = 1;

    /*
     * Find if ray r1 is a candidate for elimination from inner bound
     */
    
    r1_concave = ts_hull_is_concave(r0,
				    a1,
				    r1,
				    a2,
				    r2,
				    &interior_ang);

    if (debug >= 1)
      printf("%2d %2d concave: %2d\n",
	     i,
	     r0_ray_idx,
	     r1_concave);

    if (r1_concave == TRUE &&
	ray_list[r1_ray_idx].on_outer == FALSE)
    {

      /* ray r1 is a candidate ray for deletion, delete it */
      ray_list[r1_ray_idx].next_ray = -1;

      /* make sure this ray is hidden */
      ray_list[r1_ray_idx].hide = 1;

      /* relink rays list */
      iret = ts_hull_link_skip_hidden(ray_list,
				      ray_npts);

      point_changed = TRUE;

      /*
       * Check rays between r0 and r2 to see if any ray intersects
       * the line from r0.fin and r2.fin
       * Start at r0_ray_idx and step through each outer
       * boundary point to see if we cross the outer boundary.
       * If we cross the outer boundary, unmark that ray as "hidden",
       * mark the point on_outer = 1, and set radius_fin = radius_gro
       */

      bnd_type = BND_OUTER;
      beg_r = r0_ray_idx;
      end_r = r2_ray_idx;
      done = FALSE;
      while (!done)
      {
        ts_hull_find_bnd_intersect(ray_list,
				   ray_npts,
				   beg_r,
				   end_r,
				   bnd_type,
				   &extreme_idx,
				   &extreme_ang,
				   debug);
  
        if (extreme_idx == -1)
          done = TRUE;
        else
        {
          /* We found an outer boundary intersection */
          /* hit_outer = TRUE; */

          /* Mark point as having been visited */
          ray_list[extreme_idx].visited = 1;

          /* Mark point as on outer boundary */
          ray_list[extreme_idx].on_outer = 1;
  
          /* Use outer boundary for this ray */
          ray_list[extreme_idx].radius_fin =
	    ray_list[extreme_idx].radius_gro;

          /* Make sure this ray is not hidden */
          ray_list[extreme_idx].hide = 0;

          /* Relink rays list */
          iret = ts_hull_link_skip_hidden(ray_list,
					  ray_npts);

          /* Prepare for another iteration */
          beg_r = extreme_idx;

        } /* endif - (extreme_idx = -1) */

      } /* endwhile - (!done) */
     
    } /* endif - r1_concave */

    more_rays_to_process =
      ts_hull_check_more_rays_to_process(ray_list,
					 ray_npts);
    if (!more_rays_to_process)
      num_no_point_change++;
                                                          
    if (debug >= 2)
    {
      iret = ts_hull_write_bounds("tstorm_hull1.debug",
				  ray_list,
				  ray_npts);

      iret = ts_hull_write_storm("tstorm_hull2.debug",
				 ray_list,
				 ray_npts);
    }

  } /* endwhile - (more_rays_to_process)... */


  /*
   * We have all the points we need. Initialize the number of
   * final hull points and load up the final polygon
   */
  
  *hull_poly_npts = 0;

  for (j = 0; j < ray_npts; j++)
  {
    if (ray_list[j].hide == 0)
    {
      
      /* Load up lat/lon points from inner/outer boundaries */

      if (ray_list[j].on_outer == 1)
      {
	hull_poly->pts[*hull_poly_npts].lon = ray_list[j].lon_gro;
	hull_poly->pts[*hull_poly_npts].lat = ray_list[j].lat_gro;
      }
      else
      {
	hull_poly->pts[*hull_poly_npts].lon = ray_list[j].lon_shr;
	hull_poly->pts[*hull_poly_npts].lat = ray_list[j].lat_shr;
      }
      
      if (debug >= 1)
	printf("j, ang, rad, x, y, %2d %5.1f %8.3f %8.3f %8.3f\n",
	       j,
	       ray_list[j].angle,
	       ray_list[j].radius_fin,
	       hull_poly->pts[*hull_poly_npts].lon,
	       hull_poly->pts[*hull_poly_npts].lat);
      
      *hull_poly_npts = *hull_poly_npts + 1;
      
    }  /* endif - (ray_list[j].hide == 0) */
    
  }  /* endfor - j */

  /*
   * Add the first point to close the polygon
   */

  hull_poly->pts[*hull_poly_npts].lon = hull_poly->pts[0].lon;
  hull_poly->pts[*hull_poly_npts].lat = hull_poly->pts[0].lat;
  
  *hull_poly_npts = *hull_poly_npts + 1;

  /*
   * Write coordinates to debug files if requested
   */
  
  if (debug >= 2)
  {
    iret = ts_hull_write_bounds("tstorm_hull1.debug",
				ray_list,
				ray_npts);

    iret = ts_hull_write_storm("tstorm_hull2.debug",
			       ray_list,
			       ray_npts);
  }
  
}


/*********************************************************************
 * ts_hull_is_concave -  Uses the "inner" angle between two
 *                       sides of the polygon (with respect to 0,0)
 *                       to determine if the lines are concave
 *                       or convex.
 *
 * The two polygon sides to be checked are defined by three rays
 * and the two angles between the rays.
 *
 *  r0      r1
 *   |     /
 *   | a1 / 
 *   |   /
 *   |  /
 *   | /  a2
 *   |/    
 *   +----------r2
 *  0,0
 *
 * IN: r0 - first ray radius
 * IN: a1 - angle between r0 and r1
 * IN: r1 - second ray radius
 * IN: a2 - angle between r1 and r2
 * IN: r2 - third ray radius
 *
 * If the inner angle is > 180 degrees, the angle
 * is concave (from the "outside" of the polygon),
 * and is a candidate for deletion
 *
 * Does not handle a ray with 0 radius.
 *
 * Returns 1 if inner angle is > 180 degrees
 * Returns 0 if inner angle is <= 180 degrees
 */

static int ts_hull_is_concave(double r0,
			      double a1,
			      double r1,
			      double a2,
			      double r2,
			      double *interior_angle)

{
  double a1rad, a2rad;
  double b, c;
  double first_ang, second_ang;

  *interior_angle = 0;

  if (a1 > 0)
  {
    a1rad = a1 * DEG_TO_RAD;

    /*
     * Use law of cosines: C^2 = A^2 + B^2 - 2AB*COS(C)
     */

    c = sqrt((r0 * r0) + (r1 * r1) - (2 * r0 * r1 * cos(a1rad)));

    /*
     * solve for angle b
     */

    b = ((r1 * r1) + (c * c) - (r0 * r0)) / (2 * r1 * c);
    first_ang = acos(b) / DEG_TO_RAD;
  }
  else
    first_ang = 0.0;

  if (a2 > 0)
  {
    a2rad = a2 * DEG_TO_RAD;
    c = sqrt((r1 * r1) + (r2 * r2) - (2 * r1 * r2 * cos(a2rad)));
    b = ((r1 * r1) + (c * c) - (r2 * r2)) / (2 * r1 * c);

    second_ang = acos(b) / DEG_TO_RAD;
  }
  else
    second_ang = 0.0;

  *interior_angle = first_ang + second_ang;

  if (*interior_angle > 180)
    return 1;
  else
    return 0;
}


/*********************************************************************
 * ts_hull_rt2xy - Determines x and y coordinates
 *                 given a radius and and angle
 *
 * IN:  radius    - radius from 0,0 to point
 * IN:  ang_deg   - angle in degrees,
 *                  0 is North, positive is clockwise,
 *                  range 0 to 360
 * OUT: x         - x coordinate
 * OUT: y         - y coordinate
 *
 * Returns  0 if x,y coordinates successfully computed
 * Returns -1 if x,y coordinates not successfully computed
 */

static int ts_hull_rt2xy(double radius,
			 double ang_deg,
			 double *x,
			 double *y)

{

  double ang_rad;   /* angle in radians */

  ang_rad = ang_deg * DEG_TO_RAD;

  if (radius < 0)
    return -1;

  if (radius == 0)
  {
    *x = 0.0;
    *y = 0.0;
  }
  else
  {
    *x = radius * sin(ang_rad);
    *y = radius * cos(ang_rad);
  }

  return 0;
}


/*********************************************************************
 * ts_hull_next_valid_ray - Finds next valid ray index
 *                          after the current ray index.
 *
 * Skips any hidden indexes of 1
 * Returns the next valid ray index if available
 * Returns -1 if next valid ray index is not available
 */

static int ts_hull_next_valid_ray(int current,
				  ts_hull_ray_t *rlist,
				  int rpts)

{
  return ts_hull_get_valid_ray(current,
			       rlist,
			       rpts,
			       SEARCH_FORWARD);
}


/*********************************************************************
 * ts_hull_prev_valid_ray - Finds previous valid ray index
 *                          before the current ray index.
 *
 * Skips any hidden indexes of 1
 *
 * Returns the previous valid ray index if available
 * Returns -1 if previous valid ray index is not available
 */

static int ts_hull_prev_valid_ray(int current,
				  ts_hull_ray_t *rlist,
				  int rpts)

{
  return ts_hull_get_valid_ray(current,
			       rlist,
			       rpts,
			       SEARCH_BACKWARD);
}


/*********************************************************************
 * ts_hull_get_valid_ray - Gets next valid ray index
 *                         before or after the current index,
 *                         depending on value in the
 *                         direction variable.
 *
 * Skips any hidden indexes of 1
 *
 * Returns next ray index if directions is positive
 * Returns previous ray index if directions is negative
 *
 * Returns -1 if a valid ray index could not be found
 */

static int ts_hull_get_valid_ray(int current,
				 ts_hull_ray_t *rlist,
				 int rpts,
				 int direction)

{
  int i, index;
  int inc;

  inc = (direction > 0) ? SEARCH_FORWARD : SEARCH_BACKWARD;

  index = current;
  for (i = 0; i < rpts; i++)
  {
    index += inc;

    if (index > rpts-1)
      index = 0;

    if (index < 0)
      index = rpts - 1;

    if (rlist[index].hide == 0)
      return index;

  } /* endfor - i */

  /* If we finished loop, we did not find any valid rays */
  return -1;
}


/*********************************************************************
 * ts_hull_link_skip_hidden - Changes the next_ray entry for each
 *                            member to be the next valid ray index,
 *                            ie the next ray that does not have a
 *                            1 in hidden.
 */

static int ts_hull_link_skip_hidden(ts_hull_ray_t *rlist,
				    int rpts)

{
  int i, next, nfound;

  nfound = 0;

  for (i = 0; i < rpts; i++)
  {

    if (rlist[i].hide == 0)
    {
      next = ts_hull_next_valid_ray(i, rlist, rpts);

      if (next == -1)
      {
        fprintf(stderr, "ERROR. ray2poly:skip_hidden\n");
        fprintf(stderr, "Error, no next ray index found\n");
        return -1;
      } /* endif - (next == -1) */

      if (rlist[i].next_ray != next)
        rlist[i].next_ray = next;

      nfound++;

    } /* endif - (rlist[i].next_ray != -1) */

  } /* endfor - i */

  /* If we have less than 3 verticies,
   * we probably don't have a polygon anymore
   */

  if (nfound < 3)
    return -1;
  else
    return 0;

}


/*********************************************************************
 * ts_hull_mark_duplicate_zeros - Marks the second of any two successive
 *                                0 length radii with next_ray = -1 and
 *                                hidden = 1, so that we have no
 *                                duplicate 0 length rays in the list.
 *
 * Checks radius_ori, the original radius.
 */

static int ts_hull_mark_duplicate_zeros(ts_hull_ray_t *rlist,
					int rpts)

{

  int i;
  int index;
  int prev_zero;
  /* double oldraylen; */

  index = 0;
  prev_zero = FALSE;
  /* oldraylen = rlist[0].radius_ori; */

  for (i = 1; i < rpts+3; i++, index++)
  {

    if (index > rpts-1)
      index = 0;

    if (rlist[index].radius_ori == 0 && prev_zero)
    {
      rlist[index].next_ray = -1;
      rlist[index].hide = 1;
      rlist[index].visited = 1;
    }
    else
    {
      prev_zero = FALSE;
    }

    if (rlist[index].radius_ori == 0)
      prev_zero = TRUE;

    /* oldraylen = rlist[index].radius_ori; */

  } /* endfor - i */

  return 0;
}


/*********************************************************************
 * ts_hull_print_ray_struct - prints the struct values in the ray list
 */

void ts_hull_print_ray_struct(ts_hull_ray_t *rlist,
			      int rpts)

{
  int i;

  for (i = 0; i < rpts; i++)
  {
    printf("%2d %5.1f %6.2f %6.2f %6.2f %2d %2d %2d\n",
           i,
           rlist[i].angle,
           rlist[i].radius_shr,
           rlist[i].radius_ori,
           rlist[i].radius_gro,
           rlist[i].hide,
           rlist[i].on_outer,
           rlist[i].next_ray);
  }

}


/*********************************************************************
 * ts_hull_print_ray_coords - prints the ray coordinates
 */

void ts_hull_print_ray_coords(ts_hull_ray_t *rlist,
                              int rpts,
                              int skip_negs)

{
  int i;
  double x, y;

  for (i = 0; i < rpts; i++)
  {

    if (rlist[i].hide == 1 && skip_negs == TRUE)
      continue;

    printf("%2d %5.1f %6.2f %6.2f %6.2f %2d %2d %2d",
           i,
           rlist[i].angle,
           rlist[i].radius_shr,
           rlist[i].radius_ori,
           rlist[i].radius_gro,
           rlist[i].hide,
           rlist[i].on_outer,
           rlist[i].next_ray);

    if (ts_hull_rt2xy(rlist[i].radius_shr, rlist[i].angle, &x, &y)) {
      printf("%8.3f %8.3f", x, y);
    }
    if (ts_hull_rt2xy(rlist[i].radius_ori, rlist[i].angle, &x, &y)) {
      printf("%8.3f %8.3f", x, y);
    }
    if (ts_hull_rt2xy(rlist[i].radius_gro, rlist[i].angle, &x, &y)) {
      printf("%8.3f %8.3f", x, y);
    }
    if (ts_hull_rt2xy(rlist[i].radius_fin, rlist[i].angle, &x, &y)) {
      printf("%8.3f %8.3f\n", x, y);
    }

  }

}


/*********************************************************************
 * ts_hull_calc_limits - compute storm growth and shrink limit
 *                       polygons, initialize radius_fin, next_ray
 *                       and on_outer
 */

static int ts_hull_calc_limits(ts_hull_ray_t *rlist,
			       int rpts,
			       const double inner_bnd_multiplier,
			       const double outer_bnd_multiplier,
			       const tstorm_spdb_header_t *header,
			       const tstorm_spdb_entry_t *entry,
			       const double lead_time,
                               const int grow)

{

  int i;
  tstorm_spdb_entry_t tmp_entry;
  tstorm_polygon_t tmp_poly;

  tstorm_spdb_load_growth_polygon(header,
			          entry,
                                  &tmp_poly,
			          lead_time,
                                  grow);

  /*
   * Check the inner and outer multipliers
   */

  if ((inner_bnd_multiplier <= HULL_SHRINK_LIMIT) ||
      (inner_bnd_multiplier > HULL_GROWTH_LIMIT))
  {
    fprintf(stderr, "ERROR:tstorm_hull_smooth\n");
    fprintf(stderr,
            "inner_bnd_multiplier %f must be between %f and %f\n",
	    inner_bnd_multiplier,
	    HULL_SHRINK_LIMIT,
	    HULL_GROWTH_LIMIT);
    fprintf(stderr, "Original polygon will be used.\n");
    return -1;
  }

  if ((outer_bnd_multiplier <= HULL_SHRINK_LIMIT) ||
      (outer_bnd_multiplier > HULL_GROWTH_LIMIT))
  {
    fprintf(stderr, "ERROR:tstorm_hull_smooth\n");
    fprintf(stderr,
            "outer_bnd_multiplier %f must be between %f and %f\n",
	    outer_bnd_multiplier,
	    HULL_SHRINK_LIMIT,
	    HULL_GROWTH_LIMIT);
    fprintf(stderr, "Original polygon will be used.\n");
    return -1;
  }
  
  /*
   * Check to see if inner boundary polygon radius is larger than outer
   */

  if (inner_bnd_multiplier > outer_bnd_multiplier)
  {
    fprintf(stderr, "ERROR:tstorm_hull_smooth\n");
    fprintf(stderr, "Inner boundary is larger than outer boundary\n");
    fprintf(stderr, "inner_bnd_multiplier: %f outer_bnd_multiplier: %f\n",
            inner_bnd_multiplier, outer_bnd_multiplier);
    fprintf(stderr, "Original polygon will be used.\n");
    return -1;
  }
   
  for (i = 0; i < rpts; i++)
  {
    rlist[i].radius_shr = inner_bnd_multiplier * rlist[i].radius_ori;
    rlist[i].radius_gro = outer_bnd_multiplier * rlist[i].radius_ori;
    rlist[i].radius_fin = rlist[i].radius_shr;
    rlist[i].next_ray = i+1;
    rlist[i].on_outer = 0;
    rlist[i].hide = 0;
    rlist[i].visited = 0;

    rlist[i].lon_ori = tmp_poly.pts[i].lon;
    rlist[i].lat_ori = tmp_poly.pts[i].lat;
  }

  /*
   * Make the last ray is the same as the first
   */

  rlist[i-1].next_ray = 0;

  /*
   * Compute new polygon scale and get lat,lon coords for inner polygon
   */

  tmp_entry = *entry;
  
  tmp_entry.polygon_scale =
    entry->polygon_scale * inner_bnd_multiplier;

  tstorm_spdb_load_growth_polygon(header, &tmp_entry,
                                  &tmp_poly, lead_time, grow);

  for (i = 0; i < rpts; i++)
  {
    rlist[i].lon_shr = tmp_poly.pts[i].lon;
    rlist[i].lat_shr = tmp_poly.pts[i].lat;
  }
  
  /*
   * Compute new polygon scale and get lat/lon coords for outer polygon
   */
  
  tmp_entry = *entry;
  
  tmp_entry.polygon_scale =
    entry->polygon_scale * outer_bnd_multiplier;

  tstorm_spdb_load_growth_polygon(header, &tmp_entry,
                                  &tmp_poly, lead_time,
                                  grow);

  for (i = 0; i < rpts; i++)
  {
    rlist[i].lon_gro = tmp_poly.pts[i].lon;
    rlist[i].lat_gro = tmp_poly.pts[i].lat;
  }

  return 0;
}


/*********************************************************************
 * ts_hull_write_storm - writes final hull ray data to file
 *                       for debugging
 */

int ts_hull_write_storm(char *outname,
			ts_hull_ray_t *rlist,
			int npts)

{
  FILE *outfile;
  int i, j;
  double x, y;

  outfile = fopen (outname, "w");

  if (outfile == NULL)
  {
    printf("File < %s > was not opened successfully.\n", outname);
    exit (-1);
  }

  /*
   * Convert radials to x&y, Write the rays
   */

  for (i = 0, j = 0; i <= npts; i++, j++)
  {

    if (j > npts - 1)
      j = ts_hull_next_valid_ray(npts,
                                 rlist,
                                 npts);

    if (rlist[j].hide == 0)
    {
      if(ts_hull_rt2xy(rlist[j].radius_fin,
                       rlist[j].angle,
                       &x,
                       &y)) {
        fprintf(outfile, "%2d %9.3f %9.3f",
                j, x, y);
      }

      if (rlist[j].on_outer == 1)
      {
        fprintf(outfile, "%11.5f %11.5f",
                rlist[j].lon_gro, rlist[j].lat_gro);
      }
      else
      {
        fprintf(outfile, "%11.5f %11.5f",
                rlist[j].lon_shr, rlist[j].lat_shr);
      }

      fprintf(outfile, "%11.5f %11.5f",
              rlist[j].lon_shr, rlist[j].lat_shr);

      fprintf(outfile, "%11.5f %11.5f",
              rlist[j].lon_ori, rlist[j].lat_ori);

      fprintf(outfile, "%11.5f %11.5f\n",
              rlist[j].lon_gro, rlist[j].lat_gro);
    }

  }

  fclose(outfile);

  return 0;

}


/*********************************************************************
 * ts_hull_write_bounds - writes shrink, orig, grow data to file
 *                        for debugging
 */

int ts_hull_write_bounds(char *outname,
			 ts_hull_ray_t *rlist,
			 int npts)

{
  FILE *outfile;
  int i, j;
  double x, y;

  outfile = fopen (outname, "w");

  if (outfile == NULL)
  {
    printf("File < %s > was not opened successfully.\n", outname);
    exit (-1);
  }

  /*
   * Write the rays
   */

  for (i = 0, j = 0; i <= npts; i++, j++)
  {
    if (j > npts - 1)
      j = 0;

    if (ts_hull_rt2xy(rlist[j].radius_shr, rlist[j].angle, &x, &y)) {
      fprintf(outfile, "%2d %9.3f %9.3f", j, x, y);
    }

    if (ts_hull_rt2xy(rlist[j].radius_ori, rlist[j].angle, &x, &y)) {
      fprintf(outfile, "%9.3f %9.3f", x, y);
    }

    if (ts_hull_rt2xy(rlist[j].radius_gro, rlist[j].angle, &x, &y)) {
      fprintf(outfile, "%11.5f %11.5f", x, y);
    }

    fprintf(outfile, "%11.5f %11.5f",
            rlist[j].lon_shr, rlist[j].lat_shr);

    fprintf(outfile, "%11.5f %11.5f",
            rlist[j].lon_ori, rlist[j].lat_ori);

    fprintf(outfile, "%11.5f %11.5f\n",
            rlist[j].lon_gro, rlist[j].lat_gro);

  }

  fclose(outfile);

  return 0;

}

/*********************************************************************
 * ts_hull_get_angle_between_rays - computes angle between two rays
 *                                  of polygon
 */

static double ts_hull_get_angle_between_rays(double ray1_angle,
					     double ray2_angle)

{

  if (ray2_angle >= ray1_angle)
    return (ray2_angle - ray1_angle);
  else
    return (ray2_angle - ray1_angle + 360);

}


/*********************************************************************
 * ts_hull_find_bnd_intersect - finds if any rays intersect the inner
 *                              or outer boundaries
 *                              If one of the rays did intersect the boundary,
 *                              set extreme_ang and extreme_idx to
 *                              be the index and angle for the
 *                              "farthest visible point" from r0_ray_idx
 *
 *                              If no intersection with boundary found,
 *                              set exterme_idx = -1
 */

static void ts_hull_find_bnd_intersect(ts_hull_ray_t *rlist,
				       int ray_npts,
				       int r0_ray_idx,
				       int r2_ray_idx,
				       int bnd_type,
				       int *extreme_idx,
				       double *extreme_ang,
				       int debug)

{

  int iret      = 0;
  int j         = 0;
  int n         = 0;
  int nloops    = 0;
  int hit_outer = FALSE;
  int hit_inner = FALSE;

  double test_r0 = 0;
  double test_r2 = 0;
  double a1      = 0;
  double a2      = 0;
  double interior_ang = 0;
  /* double tmp_concave  = 0; */

  Point_d LP1, LP2, LP3, LP4, intersect;
  double pos;

  /*
   * Initialize the extreme index and angle values
   */

  *extreme_idx = -1;
  *extreme_ang = -1;

  if (r2_ray_idx < r0_ray_idx)
    nloops = r2_ray_idx - r0_ray_idx + ray_npts - 1;
  else
    nloops = r2_ray_idx - r0_ray_idx - 1;

  j = r0_ray_idx + 1;

  for (n = 0; n < nloops; n++, j++)
  {
  
    if (j > ray_npts - 1)
      j = 0;

    iret = ts_hull_rt2xy(rlist[r0_ray_idx].radius_fin,
			 rlist[r0_ray_idx].angle,
			 &LP1.x,
			 &LP1.y);

    iret = ts_hull_rt2xy(rlist[r2_ray_idx].radius_fin,
			 rlist[r2_ray_idx].angle,
			 &LP2.x,
			 &LP2.y);

    iret = ts_hull_rt2xy(0.0,
			 0.0,
			 &LP3.x,
			 &LP3.y);

    if (bnd_type == BND_OUTER)
      iret = ts_hull_rt2xy(rlist[j].radius_gro,
			   rlist[j].angle,
			   &LP4.x,
			   &LP4.y);

    if (bnd_type == BND_INNER)
      iret = ts_hull_rt2xy(rlist[j].radius_shr,
			   rlist[j].angle,
			   &LP4.x,
			   &LP4.y);

    /*
     * Determine whether the line segments LP1, LP2 and LP3, LP4 intersect.
     * If they intersect in one point return 1.  If they overlap return 2.
     * If one of the segments is not a line but a point, return -1.
     * Otherwise return 0.
     */

    iret = EG_segment_intersect(&LP1, &LP2, &LP3, &LP4,
				&intersect, &pos);

    if (iret == 0 && bnd_type == BND_OUTER)
    {
      hit_outer = TRUE;
      break;
    }

    if (iret == 1 && bnd_type == BND_INNER)
    {
      hit_inner = TRUE;
      break;
    }

  } /* endfor - n */

  if (debug >= 1)
    printf("intersect %2d hit_outer %d hit_inner %d\n",
           iret,
           hit_outer,
           hit_inner);

  if ((hit_outer == TRUE && bnd_type == BND_OUTER) ||
      (hit_inner == TRUE && bnd_type == BND_INNER))
  {

    if (bnd_type == BND_OUTER)
      *extreme_ang = 1000;
    else if (bnd_type == BND_INNER)
      *extreme_ang = -1000;
    else
    {
      printf("NEW: Warning, unknown bnd_type\n");
      exit(-1);
    } /* endif - (bnd_type == BND_OUTER) */

    j = r0_ray_idx + 1;
	
    for (n = 0; n < nloops; n++, j++)
    {
      if (j > ray_npts - 1)
	j = 0;
	  
      /*
       * Use final radius for r0
       */

      test_r0 = rlist[r0_ray_idx].radius_fin;

      if (bnd_type == BND_OUTER)
	test_r2 = rlist[j].radius_gro;
      else if (bnd_type == BND_INNER)
	test_r2 = rlist[j].radius_shr;
      else
      {
	printf("NEW: Warning, unknown bnd_type\n");
	exit(-1);
      } /* endif - (bnd_type == BND_OUTER) */

      a1 = ts_hull_get_angle_between_rays(rlist[r0_ray_idx].angle,
					  rlist[r0_ray_idx].angle);

      a2 = ts_hull_get_angle_between_rays(rlist[r0_ray_idx].angle,
					  rlist[j].angle);

      /* tmp_concave = */
      ts_hull_is_concave(test_r0,
                         a1,
                         test_r0,
                         a2,
                         test_r2,
                         &interior_ang);

      if (bnd_type == BND_OUTER)
      {
	/*
         * Want to find the smallest interior angle
         */

	if (interior_ang <= *extreme_ang)
	{
	  *extreme_ang = MIN(*extreme_ang, interior_ang);
	  *extreme_idx = j;
	}

      } /* endif - bnd_type == BND_OUTER */


      if (bnd_type == BND_INNER)
      {
	/*
         * Want to find the largest interior angle
         */

	if (interior_ang >= *extreme_ang)
	{
	  *extreme_ang = MAX(*extreme_ang, interior_ang);
	  *extreme_idx = j;
	}

      } /* endif - bnd_type == BND_INNER */

      if (debug >= 1)
      {
        printf("Extreme angle %f point %d r0 %d r2 %d\n",
	       *extreme_ang, j, r0_ray_idx, r2_ray_idx);
        printf("Extreme idx is : %d\n", *extreme_idx);
      }

    } /* endfor - n */


    if (bnd_type == BND_OUTER)
    {
      if (*extreme_ang > 999)
      {
        printf("Failed to find minimum angle\n");
        *extreme_ang = -1;
      } /* endif - extreme_ang */

    } /* endif - bnd_type == BND_OUTER) */

    if (bnd_type == BND_INNER)
    {
      if (*extreme_ang < -999)
      {
        printf("Failed to find maximum angle\n");
        *extreme_ang = -1;
      } /* endif - extreme_ang */

    } /* endif - bnd_type == BND_OUTER) */

  } /* endif - (hit_outer == TRUE &&... */
}


/*********************************************************************
 * ts_hull_check_more_rays_to_process - checks array list to see if
 *                                      there are any rays which have
 *                                      not been processed. A ray is
 *                                      considered processed at least
 *                                      once if r0_ray_idx has been set
 *                                      to the ray index.
 *
 * Return TRUE if any rays have not yet been processed
 * Return FALSE if all rays have been processed at least once
 */

static int ts_hull_check_more_rays_to_process(ts_hull_ray_t *rlist,
					      int ray_npts)

{
  int i;
  for (i = 0; i < ray_npts; i++)
  {
    if (rlist[i].visited == 0)
      return TRUE;
  }

  return FALSE;

}
