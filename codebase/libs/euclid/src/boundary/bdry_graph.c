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
 * 	bdry_graph.c
 *
 * PURPOSE
 *
 * Build a graph consisting of the boundary points of a clump.  This
 * graph will demonstrate the connections between such boundary points
 * and hence can be used to find the boundary of the clump.  The function
 * bdry_graph outputs a number of boundary nodes containing the graph
 * information.
 *
 * 	
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - May 11, 1992: Created.
 */


/* includes */
#include <stdio.h>
#include <euclid/boundary.h>
#include <euclid/clump.h>
#include <euclid/node.h>

static void
merge_lists(int *top_list, int top_size, int *bot_list, int bot_size, Node *node);

static void
set_node_list(Node *node, int node_ct, int dir, int bit, int value);

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

int
EG_bdry_graph(Row_hdr *row_hdr, int rowh_dim, int num_cols, Node *node, int num_nodes, int clump_id)
{
  float bottom;
  int i;
  int j;
  float left;
  int *list;
  int list_ct = 0;
  int list_size;
  int *next_list;
  int next_list_ct = 0;
  int node_ct = 0;
  int *previous_list;
  int previous_list_ct = 0;
  float right;
  int *temp_list;
  float top;

  list_size = 2*num_cols; /* max number of corner pts on a line */

  /* allocate memory for list, next_list, previous_list */
  list = EG_malloc(list_size*sizeof(int));
  next_list = EG_malloc(list_size*sizeof(int));
  previous_list = EG_malloc(list_size*sizeof(int));
  if (list == NULL || next_list == NULL || previous_list == NULL)
    return(-1);

  /* initialize list counts to 0 */
  list_ct = 0;
  next_list_ct = 0;
  previous_list_ct = 0;

  /* initialize the node adjacency list to -1 */
  for (i=0; i<num_nodes; i++)
    {
      node[i].adj_list[NORTH] = NULL_NODE;
      node[i].adj_list[SOUTH] = NULL_NODE;
      node[i].adj_list[EAST] = NULL_NODE;
      node[i].adj_list[WEST] = NULL_NODE;
    }

  /*
   * Iterate over all intervals to build a graph.  In each iteration,
   * calculate boundary points for each interval rectangle and then
   * determine appropriate boundary edges for each boundary point by
   * looking at overlapping intervals.  Note that we are working from the
   * bottom of the set of intervals up to the top.
   */
  for (i=0; i<rowh_dim; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  /*
	   * choose the appropriate intervals to use in determining the
	   * boundary
	   */
	  if (clump_id != 0 && row_hdr[i].intervals[j].id != clump_id)
	    continue;

	  /* find bounds of rectangle about this interval */
	    
	  left = row_hdr[i].intervals[j].begin - OFFSET_X;
	  right = row_hdr[i].intervals[j].end + OFFSET_X;
	  top = i + OFFSET_Y;
	  bottom = i - OFFSET_Y;
#ifdef NOTNOW
	  left = row_hdr[i].intervals[j].begin  * STRETCH_X - OFFSET_X;
	  right = row_hdr[i].intervals[j].end  * STRETCH_X + OFFSET_X;
	  top = i  * STRETCH_Y + OFFSET_Y;
	  bottom = i  * STRETCH_Y - OFFSET_Y;
#endif

	  /* set the boundary points of the rectangle about this interval */

	  /* set left top boundary point and next_list */
	  node[node_ct].x = left;
	  node[node_ct].y = top;
	  node[node_ct].row = i;
	  node[node_ct].col = j;
	  node[node_ct].near_x = row_hdr[i].intervals[j].begin;
	  set_node_list(node, node_ct, SOUTH, SOUTH_BIT, node_ct+2);
	  node[node_ct].corner = NW;
	  next_list[next_list_ct] = node_ct;
	  next_list_ct++;
	  node_ct++;

	  /* set right top boundary point and next_list */
	  node[node_ct].x = right;
	  node[node_ct].y = top;
	  node[node_ct].row = i;
	  node[node_ct].col = j;
	  node[node_ct].near_x = row_hdr[i].intervals[j].end;
	  set_node_list(node, node_ct, SOUTH, SOUTH_BIT, node_ct+2);
	  node[node_ct].corner = NE;
	  next_list[next_list_ct] = node_ct;
	  next_list_ct++;
	  node_ct++;
	  
	  /* set left bottom boundary point and list */
	  node[node_ct].x = left;
	  node[node_ct].y = bottom;
	  node[node_ct].row = i;
	  node[node_ct].col = j;
	  node[node_ct].near_x = row_hdr[i].intervals[j].begin;
	  set_node_list(node, node_ct, NORTH, NORTH_BIT, node_ct-2);
	  node[node_ct].corner = SW;
	  list[list_ct] = node_ct;
	  list_ct++;
	  node_ct++;
	  
	  /* set right bottom boundary point and list */
	  node[node_ct].x = right;
	  node[node_ct].y = bottom;
	  node[node_ct].row = i;
	  node[node_ct].col = j;
	  node[node_ct].near_x = row_hdr[i].intervals[j].end;
	  set_node_list(node, node_ct, NORTH, NORTH_BIT, node_ct-2);
	  node[node_ct].corner = SE;
	  list[list_ct] = node_ct;
	  list_ct++;
	  node_ct++;
	}
      
/*  for (k=0; k<next_list_ct; k++)
    printf("next_list[%d] = %d\n", k, next_list[k]);
  for (k=0; k<list_ct; k++)
    printf("list[%d] = %d\n", k, list[k]);
  for (k=0; k<previous_list_ct; k++)
    printf("previous_list[%d] = %d\n", k, previous_list[k]); */
      /*
       * Merge list with previous list assigning horizontal boundary
       * edges and working with duplicate boundary points appropriately.
       * This means that duplicates coming from overlapping rectangles do
       * not generate boundary edges.  Duplicates from caddy corner
       * rectangles generate two boundary edges.  This merge is done in
       * principle only since a merged list is not created.
       */
      merge_lists(list, list_ct, previous_list, previous_list_ct, node);

      /* exchange previous list with next list */
      temp_list = previous_list;
      previous_list = next_list;
      next_list = temp_list;
      
      /*
       * set previous_list_ct to next_list_ct and reset list_ct and
       * next_list_ct to 0
       */
      previous_list_ct = next_list_ct;
      next_list_ct = 0;
      list_ct = 0;

/*  for (k=0; k<previous_list_ct; k++)
    printf("previous_list[%d] = %d\n", k, previous_list[k]); */

    }	  

  /*
   * Assign horizontal boundary edges for the last list.  This list was
   * not handled in the above loop.
   */
  merge_lists(list, list_ct, previous_list, previous_list_ct, node);

  /* free memory for list, next_list, previous_list */
  (void)EG_free(list);
  (void)EG_free(next_list);
  (void)EG_free(previous_list);

  return(0);
} /* bdry_graph */

/*
 * PURPOSE:
 * 	Merges two lists of boundary points on the same horizontal
 * line in order to assign appropriate boundary edges.
 *
 * INPUTS:
 * 	top_list - first list of boundary points
 * 	top_size - size of top_list
 * 	bot_list - second list of boundary points
 * 	bot_size - size of bot_list
 *
 * OUTPUTS:
 * 	node - the array of boundary points is used in order to assign
 * appropriate boundary edges
 *
 * RETURNS:
 * 	void
 *
 * ALGORITHM:
 * 	Iterate through top_list and bot_list simulating a merge.
 *
 */

static void
merge_lists(int *top_list, int top_size, int *bot_list, int bot_size, Node *node)
{
  int bot_pos = 0;
  int bot_val;
  int prev_val = -1;
  int top_node;
  int top_pos = 0;
  int top_val;

/*  printf("top_size %d, bot_size %d\n", top_size, bot_size); 
  for (i=0; i<top_size; i++)
    printf("top_list[%d] = %d\n", i, top_list[i]);
  for (i=0; i<bot_size; i++)
    printf("bot_list[%d] = %d\n", i, bot_list[i]); */

  /* merge lists where lists overlap */
  while (top_pos < top_size && bot_pos < bot_size)
    {
      top_val = top_list[top_pos];
      bot_val = bot_list[bot_pos];

      /* if node on top is to the right of the node on bottom */
      if (node[bot_val].x < node[top_val].x)
	{
	  /* if there is a previous unconnected value */
	  if (prev_val != -1)
	    {
	      /* connect prev_val to bot_val */
	      set_node_list(node, prev_val, EAST, EAST_BIT, bot_val);
	      set_node_list(node, bot_val, WEST, WEST_BIT, prev_val);
	      prev_val = -1;
	    }
	  else
	    prev_val = bot_val;
	  bot_pos++;
	}
      /* if node on top is to the left of the node on bottom */
      else if (node[top_val].x < node[bot_val].x)
	{
	  if (prev_val != -1)
	    {
	      /* connect prev_val to top_val */
	      set_node_list(node, prev_val, EAST, EAST_BIT, top_val);
	      set_node_list(node, top_val, WEST, WEST_BIT, prev_val);
	      prev_val = -1;
	    }
	  else
	    prev_val = top_val;
	  top_pos++;
	}
      /* if node on top and node on bottom are in identical positions */
      else    /* node[top_val].x == node[bot_val].x */
	{
	  /* check if parent intervals overlap */

	  /*
	   * merge the two nodes into node[bot_val] and remove
	   * node[top_val]
	   */
	  top_node = node[top_val].adj_list[NORTH];
	  set_node_list(node, bot_val, NORTH, NORTH_BIT, top_node);
	  set_node_list(node, top_node, SOUTH, SOUTH_BIT, bot_val);
	  
	  /*
	   * determine whether the overlapping points are on the left or
	   * right sides of their respective interval boxes and proceed
	   * accordingly
	   */
	  if (node[top_val].adj_list[WEST] == NULL_NODE)
	    {
	      
#ifdef NOTNOTW
	      /* connect prev_val to bot_val */
	      set_node_list(node, prev_val, EAST, EAST_BIT, bot_val);
	      set_node_list(node, bot_val, WEST, WEST_BIT, prev_val);
#endif	   
	      top_pos++;
	      bot_pos++;
	    }
	  else
	    {
	      top_pos++;
	      bot_pos++;
	    }

	  /*
	   * Note that caddy corners cannot occur since one box corner
	   * will have x coord n + OFFSET_X while the other box
	   * corner will have x coord n - OFFSET_X
	   */
	}
    }

  /* if top_list has remaining elements, process them */
  while (top_pos < top_size)
    {
      top_val = top_list[top_pos];
      if (prev_val != -1)
	{
	  /* connect prev_val to top_val */
	  set_node_list(node, prev_val, EAST, EAST_BIT, top_val);
	  set_node_list(node, top_val, WEST, WEST_BIT, prev_val);

	  prev_val = -1;
	}
      else
	prev_val = top_val;
      top_pos++;
    }

  /* if bot_list has remaining elements, process them */
  while (bot_pos < bot_size)
    {
      bot_val = bot_list[bot_pos];
      if (prev_val != -1)
	{
	  /* connect prev_val to bot_val */
	  set_node_list(node, prev_val, EAST, EAST_BIT, bot_val);
	  set_node_list(node, bot_val, WEST, WEST_BIT, prev_val);

	  prev_val = -1;
	}
      else
	prev_val = bot_val;
      bot_pos++;
    }
} /* merge_lists */

/*
 * Set the adjacency list of node[node_ct] in the direction "dir" to
 * value.  Set a bit in the adjacency list open flag to reflect that this
 * direction is available for visiting.
 */
static void
set_node_list(Node *node, int node_ct, int dir, int bit, int value)
{
  node[node_ct].adj_list[dir] = value;
  node[node_ct].adj_list_open |= bit;
} /* set_node_list */

