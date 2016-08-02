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
/* node.h */

#ifndef NODE_H
#define NODE_H

#ifdef __cplusplus
 extern "C" {
#endif

#define NUM_DIRS 4

/* cardinal directions */
#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3

/* bit masks used to for checking whether nodes have been visited */
#define NORTH_BIT (1<<NORTH)
#define SOUTH_BIT (1<<SOUTH)
#define EAST_BIT (1<<EAST)
#define WEST_BIT (1<<WEST)

#define NULL_NODE (-1)		

/*
 * intermediate directions - Note that NW, SW and NE, SE both differ by
 * ADJ_CORNER = 2.  This property is used in match_lists() in
 * build_graph.c
 */
#define NW 0
#define NE 1
#define SW 2
#define SE 3
#define ADJ_CORNER 2
#define CORNER_BIT 1

#define BDRY_OFFSET 0.25

#define OFFSET_X 0.4
/* #define OFFSET_X 0.25 */
#define OFFSET_Y 0.5

#ifdef NOTNOW

#define OFFSET_X 1
#define OFFSET_Y 1
#define STRETCH_X 4
#define STRETCH_Y 2

#endif

/*
 * the following structure is allocated for each boundary point and is
 * used to build a graph which will then be traversed to determine the
 * boundary
 */
typedef struct node
{
  float x;			/* x coordinate of point */
  float y;			/* y coordinate of point */
  int adj_list[NUM_DIRS];	/* nodes this node is connected to */
  int row;			/* row of parent interval */
  int col;			/* column of parent interval */
  int near_x;		/* x coordinate of interval endpoint closest to node */
  int corner;			/* corner direction - NW, NE, SW, SE */
  int visited;			/* used to traverse graph */
  int adj_list_open;	        /*
				 * Lowest 4 bits of adj_list_open used to
				 * reflect nodes that have not been
				 * visited in traversal.  If
				 * adj_list_open == 0, all neighboring
				 * nodes have been visited.  Bits 0 thru
				 * 3 are associated with each direction
				 * and are 1 if the direction has been
				 * visited, 0 otherwise.
				 */
} Node;

typedef struct onode
{
  float x;			/* x coordinate of point */
  float y;			/* y coordinate of point */
  int adj_list[NUM_DIRS];	/* nodes this node is connected to */
  int row;			/* row of parent interval */
  int col;			/* column of parent interval */
  float near_x;		/* x coordinate of interval endpoint closest to node */
  int corner;			/* corner direction - NW, NE, SW, SE */
  int visited;			/* used to traverse graph */
  int adj_list_open;	        /*
				 * Lowest 4 bits of adj_list_open used to
				 * reflect nodes that have not been
				 * visited in traversal.  If
				 * adj_list_open == 0, all neighboring
				 * nodes have been visited.  Bits 0 thru
				 * 3 are associated with each direction
				 * and are 1 if the direction has been
				 * visited, 0 otherwise.
				 */
} ONode;

#ifdef __cplusplus
}
#endif

#endif
