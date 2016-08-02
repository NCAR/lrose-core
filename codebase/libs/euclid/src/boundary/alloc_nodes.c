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
/**************************************************
 * alloc_nodes()
 *
 * allocate memory for nodes in boundary routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307
 *
 * May 1995
 */

#include <string.h>
#include <euclid/alloc.h>
#include <euclid/boundary.h>
#include <euclid/node.h>
#include <euclid/point.h>

/*
 *  DESCRIPTION:    
 *
 *    Allocates or reallocates the node arrays
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

void EG_alloc_nodes(int n_nodes,
		    int *n_nodes_alloc_p,
		    int **bdry_list_p,
		    Point_d **bdry_pts_p,
		    Node **nodes_p)

{

  int n_nodes_alloc = *n_nodes_alloc_p;
  int *bdry_list = *bdry_list_p;
  Point_d *bdry_pts = *bdry_pts_p;
  Node *nodes = *nodes_p;
  
  if (n_nodes_alloc < n_nodes) {
    
    if (bdry_list == NULL) {
      bdry_list = (int *) EG_malloc
	((unsigned int) ((n_nodes + 2) * sizeof(int)));
    } else {
      bdry_list = (int *) EG_realloc
	((char *) bdry_list,
	 (unsigned int) ((n_nodes + 2) * sizeof(int)));
    }
     
    if (bdry_pts == NULL) {
      bdry_pts = (Point_d *) EG_malloc
	((unsigned int) ((n_nodes + 2) * sizeof(Point_d)));
    } else {
      bdry_pts = (Point_d *) EG_realloc
	((char *) bdry_pts,
	 (unsigned int) ((n_nodes + 2) * sizeof(Point_d)));
    }
      
    if (nodes == NULL) {
      nodes = (Node *) EG_malloc
	((unsigned int) ((n_nodes + 1) * sizeof(Node)));
    } else {
      nodes = (Node *) EG_realloc
	((char *) nodes,
	 (unsigned int) ((n_nodes + 1) * sizeof(Node)));
    }
    
    n_nodes_alloc = n_nodes;
    
  } /* if (n_nodes_alloc ... */

  memset ((void *)  bdry_list,
          (int) 0, (size_t) ((n_nodes + 2) * sizeof(int)));
  
  memset ((void *)  bdry_pts,
          (int) 0, (size_t) ((n_nodes + 2) * sizeof(Point_d)));
  
  memset ((void *)  nodes,
          (int) 0, (size_t) ((n_nodes + 1) * sizeof(Node)));
  
  *n_nodes_alloc_p = n_nodes_alloc;
  *bdry_list_p = bdry_list;
  *bdry_pts_p = bdry_pts;
  *nodes_p = nodes;
  
  return;

}

void EG_free_nodes(int *n_nodes_alloc_p,
		   int **bdry_list_p,
		   Point_d **bdry_pts_p,
		   Node **nodes_p)

{

  if (*bdry_list_p) {
    EG_free((void *) *bdry_list_p);
  }
  if (*nodes_p) {
    EG_free((void *) *nodes_p);
  }
  if (*bdry_pts_p) {
    EG_free((void *) *bdry_pts_p);
  }

  *bdry_list_p = NULL;
  *nodes_p = NULL;
  *bdry_pts_p = NULL;

  *n_nodes_alloc_p = 0;

  return;

}



