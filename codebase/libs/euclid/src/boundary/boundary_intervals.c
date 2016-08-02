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
/*
 * NAME
 * 	boundary_intervals.c
 *
 * PURPOSE
 * 	Given a set of intervals corresponding to a clump, determine
 * its boundary.
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - Feb 11, 1993: Created.
 */
#include <stdio.h>

#include <euclid/boundary.h>
#include <euclid/clump.h>
#include <euclid/node.h>

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
int EG_boundary_intervals(Row_hdr *row_hdr, int num_intervals, int num_rows, int num_cols, Point_d **pbdry_pts, int *pbdry_pts_size, int clump_id)
{
  int *bdry_list;
  Point_d *bdry_pts;
  int bdry_pts_size;		
  int bdry_size;
  int min_size;
  Node *node;
  int num_nodes;
  
  num_nodes = 4*num_intervals;
  
/*  printf("num_nodes is %d, num_rows %d, num_cols %d\n", num_nodes, num_rows, num_cols);  */

  /* allocate memory for node array to be set by bdry_graph */
  node = (Node *)EG_calloc(num_nodes, sizeof(Node));
  if (node == NULL)
    return(-1);

  /*
   * Call bdry_graph to set up the graph for the boundary.  As
   * described in bdry_graph.c:  Build a graph consisting of the
   * boundary points of a clump.  This graph will demonstrate the
   * connections between such boundary points and hence can be used
   * to find the boundary of the clump.  The function bdry_graph
   * outputs a number of boundary nodes containing the graph information.
   */
  EG_bdry_graph(row_hdr, num_rows, num_cols, node, num_nodes, clump_id);

  /*
   * allocate space for the boundary output list to be set by
   * traverse_graph
   */
  bdry_list = (int *)EG_calloc((num_nodes+1), sizeof(int));
  if (bdry_list == NULL)
    {
      (void)EG_free(node);
      return(-1);
    }

  /* reallocate output array if necessary */
  bdry_pts = *pbdry_pts;
  bdry_pts_size = *pbdry_pts_size;
  min_size = num_nodes + 1;

  if (bdry_pts_size < min_size)
    {
      bdry_pts = (Point_d *)EG_realloc(bdry_pts, min_size * sizeof(Point_d));

      if (bdry_pts == NULL)
	{
	  (void)EG_free(node);
	  (void)EG_free(bdry_list);
	  return(-1);
	}

      bdry_pts_size = min_size;
    }

  
  /* traverse the graph and generate the list of boundary nodes */
  bdry_size = EG_traverse_bdry_graph(node, 2, bdry_list);
  

#ifdef DEBUG
  printf("bdry_size %d\n", bdry_size);
  
  /* print out the boundary list */
  {
    int i;
    for (i=0; i<bdry_size; i++)
      printf("bdry_list[%d] = %d\n", i, bdry_list[i]);
  }
#endif

  /*
   * generate the array of boundary points from the list of boundary
   * nodes
   */
  bdry_size = EG_gen_bdry(bdry_pts, node, bdry_list, bdry_size); 
  
  (void)EG_free(node);
  (void)EG_free(bdry_list);

  *pbdry_pts = bdry_pts;
  *pbdry_pts_size = bdry_pts_size;
  return(bdry_size);
}
