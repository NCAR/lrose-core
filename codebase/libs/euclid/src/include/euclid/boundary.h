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
/* boundary.h */

#ifndef BOUNDARY_H
#define BOUNDARY_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <euclid/point.h>
#include <euclid/clump.h>
#include <euclid/node.h>

#define BDRY_SCHEME0 0
#define BDRY_SCHEME1 1
#define BDRY_SCHEME2 2

/*
 * function prototypes
 */

extern int
  EG_adj_star(Star_point *star_pts, int dim_star,
	      Point_d *ref_pt, Point_d *ray, double eps);

/*
 *  DESCRIPTION:    
 *
 *    Allocate memory for nodes in boundary routines
 *
 *  INPUTS:
 *
 *    int n_nodes - number of nodes to be catered for
 *
 *    int *n_nodes_alloc_p - pointer to N_nodes_alloc in
 *      calling routine, which should be static.
 *      Must be initialized to 0 before first call to alloc_nodes().
 *      Typical declaration in calling routine is:
 *        static int N_nodes_alloc = 0;
 *
 *    int **bdry_list_p - pointer to Bdry_list in
 *      calling routine, which should be static.
 *      Must be initialized to NULL before first call to alloc_nodes().
 *      Typical declaration in calling routine is:
 *        static int *Bdry_list = NULL;
 *
 *    Point_d **bdry_pts_p - pointer to Bdry_pts in
 *      calling routine, which should be static.
 *      Must be initialized to NULL before first call to alloc_nodes().
 *      Typical declaration in calling routine is:
 *        static Point_d *Bdry_pts = NULL;
 *
 *    int **nodes_p - pointer to Nodes in
 *      calling routine, which should be static.
 *      Must be initialized to NULL before first call to alloc_nodes().
 *      Typical declaration in calling routine is:
 *        static Node *nodes = NULL;
 *
 * OUTPUTS:
 *
 *    *n_nodes_alloc_p, *bdry_list_p, *bdry_pts_p and *nodes_p are set to
 *    new values as necessary to meet the needs of reallocation.
 *
 * RETURNS:
 *
 *   void
 *
 */

extern void EG_alloc_nodes(int n_nodes,
			   int *n_nodes_alloc_p,
			   int **bdry_list_p,
			   Point_d **bdry_pts_p,
			   Node **nodes_p);

extern void OEG_alloc_nodes(int n_nodes,
			   int *n_nodes_alloc_p,
			   int **bdry_list_p,
			   Point_d **bdry_pts_p,
			   ONode **nodes_p);

/*
 * DESCRIPTION:
 * 	This function builds a graph consisting of the boundary nodes
 * of a clump of intervals.
 *
 * INPUTS:
 * 	row_hdr - interval control block
 * 	rowh_dim - dimension of row_hdr
 * 	num_cols - number of columns in a row 
 *      num_nodes - size of output array node
 * 	clump_id - If 0, all intervals from row_hdr are used in
 * determining the boundary.  If greater than 0, only those intervals
 * having id == clump_id will be used in determining the boundary.
 *
 * OUTPUTS:
 * 	node - array of boundary points for clump
 *
 * RETURNS:
 * 	0 if successful, -1 on memory allocation failure
 *
 * METHOD:
 *  1.  Determine rectangles for all intervals by adjusting interval
 * endpoints using OFFSET_X in x and OFFSET_Y in y.
 *
 *  2.  Build a boundary edge graph using the information in 2.
 *
 * Note that another function is used to do a depth first search of
 * the boundary edge graph in 2) in order to determine the actual
 * boundary boundary of the clump.
 *
 * NOTES:
 * 	This function assumes that the input intervals belong to one
 * clump.  One can call EG_rclump_2d() or a related function to do the
 * clumping.  Such functions set the id in the interval structure to the
 * appropriate clump_id.  This is required since the function merge_lists
 * used below tacitly assumes that the boundary points all belong to one
 * clump.  The x and y values for a node are derived from the original x
 * and y values for the parent interval.
 */

extern int
  EG_bdry_graph(Row_hdr *row_hdr, int rowh_dim, int num_cols, Node *node,
		int num_nodes, int clump_id);

/*
 * DESCRIPTION:
 * 	This function builds a graph consisting of the boundary nodes
 * of a clump of intervals.
 *
 * INPUTS:
 * 	row_hdr - interval control block
 * 	rowh_dim - dimension of row_hdr
 * 	num_cols - number of columns in a row 
 *      num_nodes - size of output array node
 * 	clump_id - If 0, all intervals from row_hdr are used in
 * determining the boundary.  If greater than 0, only those intervals
 * having id == clump_id will be used in determining the boundary.
 *
 * OUTPUTS:
 * 	node - array of boundary points for clump
 *
 * RETURNS:
 * 	0 if successful, -1 on memory allocation failure
 *
 * METHOD:
 *  1.  Determine rectangles for all intervals by adjusting interval
 * endpoints using OFFSET_X in x and OFFSET_Y in y.
 *
 *  2.  Build a boundary edge graph using the information in 2.
 *
 * Note that another function is used to do a depth first search of
 * the boundary edge graph in 2) in order to determine the actual
 * boundary boundary of the clump.
 *
 * NOTES:
 * 	This function assumes that the input intervals belong to one
 * clump.  One can call OEG_rclump_2d() or a related function to do the
 * clumping.  Such functions set the id in the interval structure to the
 * appropriate clump_id.  This is required since the function merge_lists
 * used below tacitly assumes that the boundary points all belong to one
 * clump.  The x and y values for a node are derived from the original x
 * and y values for the parent interval.
 */

extern int
  OEG_bdry_graph(Row_hdr *row_hdr, int rowh_dim, int num_cols, ONode *node,
		int num_nodes, int clump_id);


/*
 * DESCRIPTION:    
 * 	boundary_intervals - given a 2D clump of intervals, determine
 * its boundary
 *
 * INPUTS:
 * 	row_hdr - array containing input intervals
 * 	num_intervals - total number of intervals in row_hdr array
 * 	num_rows - number of rows
 * 	num_cols - number of columns
 * 	pbdry_pts - pointer to array of generated boundary points (Set
 *           to NULL if pbdry_pts has not been allocated and this routine
 * 	  will then allocate space.)
 * 	pbdry_size - size of pbdry_pts array (may be larger than the
 * 	     actual boundary size).  Set this to to be the size of
 *           pbdry_pts (set to 0 if pbdry_pts is NULL).
 *      clump_id - If 0, all intervals from row_hdr are used in
 *           determining the boundary.  If greater than 0, only those intervals
 *           having id == clump_id will be used in determining the boundary.
 *
 * OUTPUTS:
 * 	pbdry_pts - pointer to array of generated boundary points (may
 *           be reallocated if necessary)
 * 	pbdry_size - size of pbdry_pts array (may be larger than the
 * 	     actual boundary size).  
 *
 * RETURNS:
 *      number of points in pbdry_pts or -1 on failure
 *
 * NOTES:
 * 	The funtion assumes that the input intervals are clumped and
 * hence connected.  Unexpected behaviour may result if this is not so!
 * The function also may reallocate the space in pbdry_pts and will in
 * this case reset pbdry_size appropriately.
 */

extern int
  EG_boundary_intervals(Row_hdr *row_hdr, int num_intervals, int num_rows,
			int num_cols, Point_d **pbdry_pts, int *pbdry_pts_size, int clump_id);

/*
 * DESCRIPTION:    
 * 	find_extreme_pts - find the extreme points of a boundary
 * corresponding to a given input vector.
 *
 * INPUTS:
 * 	in_bdry - input array of boundary points
 * 	in_bdry_size - dimension of in_bdry
 * 	v0 - begin point of vector
 * 	v1 - end point of vector
 *
 * OUTPUTS:
 * 	max - array index of furthest boundary point 
 * 	min - array index of closest boundary point 
 *
 * RETURNS:
 *       void
 *
 * NOTES:
 * 	Assumes that line corresponding to input vector does not
 * intersect boundary.
 */

/*
 * DESCRIPTION:    
 * 	boundary_intervals - given a 2D clump of intervals, determine
 * its boundary
 *
 * INPUTS:
 * 	row_hdr - array containing input intervals
 * 	num_intervals - total number of intervals in row_hdr array
 * 	num_rows - number of rows
 * 	num_cols - number of columns
 * 	pbdry_pts - pointer to array of generated boundary points (Set
 *           to NULL if pbdry_pts has not been allocated and this routine
 * 	  will then allocate space.)
 * 	pbdry_size - size of pbdry_pts array (may be larger than the
 * 	     actual boundary size).  Set this to to be the size of
 *           pbdry_pts (set to 0 if pbdry_pts is NULL).
 *      clump_id - If 0, all intervals from row_hdr are used in
 *           determining the boundary.  If greater than 0, only those intervals
 *           having id == clump_id will be used in determining the boundary.
 *
 * OUTPUTS:
 * 	pbdry_pts - pointer to array of generated boundary points (may
 *           be reallocated if necessary)
 * 	pbdry_size - size of pbdry_pts array (may be larger than the
 * 	     actual boundary size).  
 *
 * RETURNS:
 *      number of points in pbdry_pts or -1 on failure
 *
 * NOTES:
 * 	The funtion assumes that the input intervals are clumped and
 * hence connected.  Unexpected behaviour may result if this is not so!
 * The function also may reallocate the space in pbdry_pts and will in
 * this case reset pbdry_size appropriately.
 */
int OEG_boundary_intervals
(
 Row_hdr *row_hdr,
 int num_intervals,
 int num_rows,
 int num_cols,
 Point_d **pbdry_pts,
 int *pbdry_pts_size,
 int clump_id,
 int boundary_scheme		/* select the type of boundary generation */
 );

extern void
  EG_find_extreme_pts(Point_d *in_bdry, int in_bdry_size,
		      Point_d *v0, Point_d *v1, int *max, int *min);

extern void EG_free_nodes(int *n_nodes_alloc_p,
			  int **bdry_list_p,
			  Point_d **bdry_pts_p,
			  Node **nodes_p);

extern void OEG_free_nodes(int *n_nodes_alloc_p,
			  int **bdry_list_p,
			  Point_d **bdry_pts_p,
			  ONode **nodes_p);

/*
 * NAME
 * 	gen_bdry
 *
 * PURPOSE
 * 	Generate an array of boundary points from an array of
 * boundary nodes and a bdry_list containing the indices of such nodes.
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - May 11, 1992: Created.
 */

extern int
  EG_gen_bdry(Point_d *bdry_pts, Node *node,
	      int *out_list, int list_size);

/*
 * NAME
 * 	gen_bdry
 *
 * PURPOSE
 * 	Generate an array of boundary points from an array of
 * boundary nodes and a bdry_list containing the indices of such nodes.
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - May 11, 1992: Created.
 */

extern int
  OEG_gen_bdry(Point_d *bdry_pts, ONode *node,
	      int *out_list, int list_size);

/*
 * generate the boundary using only the endpoints of each interval as
 * opposed to using the rectangular boxes containing the intervals
 */

extern int
  EG_gen_bdry_alt(Point_d *bdry_pts, Node *node,
		  int *out_list, int list_size);

extern int
  OEG_gen_bdry_alt(Point_d *bdry_pts, ONode *node,
		  int *out_list, int list_size);

extern int
  OEG_gen_bdry1(Point_d *bdry_pts, ONode *node, int *out_list, int list_size);

extern int
  OEG_gen_bdry2(Point_d *bdry_pts, ONode *node, int *out_list, int list_size);


/*
 * Divide a circle into "div" divisions about a reference point, ref_pt.
 * Store the ray endpoints in ray.
 */

extern double
  EG_init_ray(Point_d *ray, int div, Point_d *ref_pt);

/*
 * Divide a circle into "div" divisions about a reference point, ref_pt.
 * Store the ray endpoints in ray. This is done relative to True North,
 * instead of the usual Cartesian axes.
 */

extern double
  EG_init_ray_TN(Point_d *ray, int div, Point_d *ref_pt);

/*
 * DESCRIPTION:    
 * 	make_star takes a boundary consisting of a sequence of line
 * segments perpendicular to the x or y axes, a number of rays, num_rays,
 * and a reference point and then outputs a star-shaped set consisting of
 * the points of intersection of the given boundary with rays emanating
 * from the reference point at equidistant angles.  The initial ray is
 * identical with the positive x axis and sweeps counterclockwise.
 *
 * INPUTS:
 * 	bdry_pts - array of points determining the boundary, 
 * 		   the initial and ending boundary points are identical
 * 	dim_bdry_pts - dimension of array of boundary points
 * 	ray - array of sines and cosines for each ray endpoint
 * 	div - dimension of ray (short for divisions)
 * 	theta - angle used for producing ray
 * 	ref_pt - reference point
 *
 * OUTPUTS:
 * 	star_pts - output array of points
 *
 * RETURNS:
 *     	dimension of star_pts or -1 on failure
 *
 * METHOD:
 * 	Iterate through the boundary segments doing the following:
 *
 * 1.  Utilizing the endpoints of a segment, determine the range of
 * rays that the segment intersects.
 *
 * For each ray found in 1) do
 *
 * a.  Determine the point of intersection of the segment with the ray
 * and then calculate the length of the segment from the point of
 * intersection to ref_pt.  If this length is greater than the current
 * maximal length for the ray, replace the point of intersection stored
 * for the ray with the new point of intersection.
 */

extern int
  EG_make_star(Point_d bdry_pts[], int dim_bdry_pts, Point_d *ray,
	       int div, double theta, Point_d *ref_pt,
	       Star_point star_pts[]);

/*
 * DESCRIPTION:    
 * 	make_star_TN is similar to make_star, except that the star
 *      starts at True North and moves clockwise, as opposed to the
 *      usual Cartesian axes which start at the X axis and move
 *      counter-clockwise
 */

extern int
  EG_make_star_TN(Point_d bdry_pts[], int dim_bdry_pts, Point_d *ray,
		  int div, double theta, Point_d *ref_pt,
		  Star_point star_pts[]);

/*
 * DESCRIPTION:    
 * 	translate_bdry - find the boundary enclosing all points
 * generated by translating a boundary by a vector
 *
 * INPUTS:
 * 	in_bdry - array of input boundary points
 * 	in_bdry_size - dimension of in_bdry
 * 	vect - translation vector
 * 	bbox - box bounding the boundary
 *
 * OUTPUTS:
 * 	out_bdry - array of output boundary points
 * 	out_bdry_size - dimension of out_bdry (should be in_bdry_size + 2)
 *
 * RETURNS:
 *       0 on success, -1 on failure
 *
 * NOTES:
 *  	The bounding box is given in integer coordinates and the
 * boundary is given in floating point coordinates.  This routine assumes
 * that the boundary consists of line segments parallel to the x and y
 * coordinate axes.  If the boundary is a simple closed polygon, 
 * translate_poly_bdry below.
 */

extern int
  EG_translate_bdry(Point_d *in_bdry, int in_bdry_size, Point_d vect,
		    Box_2d bbox, Point_d *out_bdry, int out_bdry_size);

/*
 * PURPOSE:
 * 	This function traverses a graph consisting of the boundary
 * nodes of a clump of intervals.  It outputs a list of sequential
 * boundary points.
 *
 * INPUTS:
 * 	node - array of boundary points for clump
 * 	start_node - index of starting node to begin traversing the boundary
 *
 * OUTPUTS:
 * 	bdry_list - output list of node indices that constitute the
 * boundary
 *
 * RETURNS:
 * 	the size of bdry_list which is equivalent to the number of
 * nodes in the boundary
 *
 * ALGORITHM:
 * 	Starting at start_node, do a depth first search of the graph.
 * Throw out any intermediate cycles until arriving back at start_node.
 * The cycle containing start_node will be the boundary of the graph.
 */

extern int
  EG_traverse_bdry_graph(Node *node, int start_node, int *bdry_list);

/*
 * PURPOSE:
 * 	This function traverses a graph consisting of the boundary
 * nodes of a clump of intervals.  It outputs a list of sequential
 * boundary points.
 *
 * INPUTS:
 * 	node - array of boundary points for clump
 * 	start_node - index of starting node to begin traversing the boundary
 *
 * OUTPUTS:
 * 	bdry_list - output list of node indices that constitute the
 * boundary
 *
 * RETURNS:
 * 	the size of bdry_list which is equivalent to the number of
 * nodes in the boundary
 *
 * ALGORITHM:
 * 	Starting at start_node, do a depth first search of the graph.
 * Throw out any intermediate cycles until arriving back at start_node.
 * The cycle containing start_node will be the boundary of the graph.
 */

extern int
  OEG_traverse_bdry_graph(ONode *node, int start_node, int *bdry_list);

/* find bounding boxes for the clumps in a clump_info structure */

extern void OEG_find_ci_3d_bbox(OClump_info *ci);

#ifdef __cplusplus
}
#endif

#endif
