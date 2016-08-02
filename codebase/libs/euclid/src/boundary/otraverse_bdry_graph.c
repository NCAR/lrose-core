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

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/*
 * NAME
 * 	traverse_graph.c
 *
 * PURPOSE
 *
 * Traverse a graph consisting of the boundary points of a clump in order
 * to determine the boundary
 * 	
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - May 11, 1992: Created.
 */


/* includes */
#include <euclid/boundary.h>
#include <euclid/clump.h>
#include <euclid/node.h>

/* function declarations */
static int lowest_bit(int integer);
static int opposite_bit(int bit);
/* static void print_list(int *list, int bdry_size); */
static int visit(int from_node, ONode *node, int *list, int *pbdry_size);

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
int
OEG_traverse_bdry_graph(ONode *node, int start_node, int *bdry_list)
{
  int bdry_size = 1;

  bdry_list[0] = start_node;
  while (visit(start_node, node, bdry_list, &bdry_size))
    {
      start_node = bdry_list[bdry_size-1];
    }
  
  bdry_list[bdry_size++] = bdry_list[0];
  return(bdry_size);
} /* traverse_graph */

/*
 * PURPOSE:
 * 	Step from a node in a graph to a neighbor that has not been visited.
 *
 * INPUTS:
 * 	from_node  - index of node that we are starting from
 * 	node	   - array of nodes
 *
 * OUTPUTS:
 * 	list  - unvisited node will be placed at end of list of node indices
 * 	bdry_size - pointer to size of list
 *
 * RETURNS:
 * 	0 - cannot visit new node
 * 	1 - if new node is visited
 *
 * ALGORITHM:
 * 	1.  Select edge that has not been used.  (Such an edge will
 * 	always be available.)
 *
 * 	2.  If edge connects to a visited node, pop cycle off list.
 * 	   a.  If list is empty, return 0.
 * 	   b.  If list is not empty, return 1.
 *
 * 	3.  If edge connects to an unvisited node, remove the entrant
 * 	edge of the unvisited node from adj_list_open, place the node at the
 * 	end of list, increment bdry_size and return 1.
 *
 */
int
visit(int from_node, ONode *node, int *list, int *pbdry_size)
{
  int bit_pos;
  int bdry_size;
  int to_node;
  int orig_size;

  bdry_size = *pbdry_size;

  node[from_node].visited = 1;

  /* select edge that has not been used */
  bit_pos = lowest_bit(node[from_node].adj_list_open);
  to_node = node[from_node].adj_list[bit_pos];

  /*
   * remove this edge from from_node and to_node to disallow its future
   * use
   */
  node[from_node].adj_list_open -= (1 << bit_pos);
  node[to_node].adj_list_open -= opposite_bit(bit_pos);

  if (node[to_node].visited)
    {
      /* new node has been visited earlier */
      
      /* remember original size */
      orig_size = bdry_size;

      if (list[0] != to_node)
	{
	  /*
	   * there should not be any subcycles due to the geometry of the
	   * rectangles which prevent caddy corners
	   */
	  return(-1);
	}
      else
	{
	  *pbdry_size = orig_size;
	  return(0);
	}
    }
  else	
    {
      /* new node has not been visited */

      /* append new node to list */
      list[bdry_size++] = to_node;
      *pbdry_size = bdry_size;
      return(1);
    }
} /* visit */

/*
 * return the position of the smallest bit in integer (only the lowest 4
 * bits are of interest)
 */
int
lowest_bit(int integer)
{
  switch (integer - (integer & (integer - 1)))
    {
    case (1 << 0):
      return(0);

    case (1 << 1):
      return(1);

    case (1 << 2):
      return(2);

    case (1 << 3):
      return(3);
    }

  return(-1);
} /* lowest_bit */

/*
 * Find the direction bit opposite in direction to bit_pos.  In this
 * case,
 *
 * NORTH_BIT and SOUTH_BIT are opposites
 * EAST_BIT and WEST_BIT are opposites
 */
int
opposite_bit(int bit_pos)
{
  switch (bit_pos)
    {
    case NORTH:
      return(SOUTH_BIT);
    case SOUTH:
      return(NORTH_BIT);
    case EAST:
      return(WEST_BIT);
    case WEST:
      return(EAST_BIT);
    }
  return(-1);
} /* opposite_bit */


/* void print_list(int *list, int list_size) */
/* { */
/*   int i; */
  
/*   for (i=0; i<list_size; i++) */
/*     printf("list[%d] = %d\n", i, list[i]); */
/* } /\* print_list *\/ */
