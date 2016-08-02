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

#include <euclid/boundary.h>
#include <euclid/node.h>
#include <euclid/point.h>

int OEG_gen_bdry(Point_d *bdry_pts, ONode *node, int *out_list, int list_size)

{
  int i;
  ONode *nodep;

#ifdef NOTNOW

  for (i=0; i<list_size; i++)
    {
      nodep = &node[out_list[i]];
      bdry_pts[i].x = nodep->x;
      bdry_pts[i].y = nodep->y; 
    }

#endif


  for (i=0; i<list_size; i++)
    {
      nodep = &node[out_list[i]];
      
      switch (nodep->corner)
	{
	case NW:
	  bdry_pts[i].x = (nodep->x + OFFSET_X);
	  bdry_pts[i].y = (nodep->y - OFFSET_Y) + 1;
	  break;
	  
	case NE:
	  bdry_pts[i].x = (nodep->x - OFFSET_X) + 1;
	  bdry_pts[i].y = (nodep->y - OFFSET_Y) + 1;
	  break;
	  
	case SW:
	  bdry_pts[i].x = (nodep->x + OFFSET_X);
	  bdry_pts[i].y = (nodep->y + OFFSET_Y);
	  break;
	  
	case SE:
	  bdry_pts[i].x = (nodep->x - OFFSET_X) + 1;
	  bdry_pts[i].y = (nodep->y + OFFSET_Y);
	  break;
	}
    }



#ifdef NOTNOW
  for (i=0; i<list_size; i++)
    {
      nodep = &node[out_list[i]];
      
      switch (nodep->corner)
	{
	case NW:
	  bdry_pts[i].x = (nodep->x + OFFSET_X)/STRETCH_X;
	  bdry_pts[i].y = (nodep->y - OFFSET_Y)/STRETCH_Y + 1;
	  break;
	  
	case NE:
	  bdry_pts[i].x = (nodep->x - OFFSET_X)/STRETCH_X + 1;
	  bdry_pts[i].y = (nodep->y - OFFSET_Y)/STRETCH_Y + 1;
	  break;
	  
	case SW:
	  bdry_pts[i].x = (nodep->x + OFFSET_X)/STRETCH_X;
	  bdry_pts[i].y = (nodep->y + OFFSET_Y)/STRETCH_Y;
	  break;
	  
	case SE:
	  bdry_pts[i].x = (nodep->x - OFFSET_X)/STRETCH_X + 1;
	  bdry_pts[i].y = (nodep->y + OFFSET_Y)/STRETCH_Y;
	  break;
	}
    }
#endif

  return(list_size);

} /* gen_bdry */

int OEG_gen_bdry1(Point_d *bdry_pts, ONode *node, int *out_list, int list_size)
{
  int i;
  ONode *nodep;

  for (i=0; i<list_size; i++)
    {
      nodep = &node[out_list[i]];
      bdry_pts[i].x = nodep->x;
      bdry_pts[i].y = nodep->y; 
    }

  return(list_size);
} /* gen_bdry1 */

/*
 * generate the boundary using only the endpoints of each interval as
 * opposed to using the rectangular boxes containing the intervals
 */
int OEG_gen_bdry2(Point_d *bdry_pts, ONode *node, int *out_list, int list_size)

{
  int ct;
  int i;
  ONode *nodep;

  nodep = &node[out_list[0]];
  
  bdry_pts[0].x = nodep->near_x;
  bdry_pts[0].y = nodep->row;
  ct = 1;
  for (i=1; i<list_size; i++)
    {
      nodep = &node[out_list[i]];
      
      bdry_pts[ct].x = nodep->near_x;
      bdry_pts[ct].y = nodep->row;
      if (bdry_pts[ct].x != bdry_pts[ct-1].x || bdry_pts[ct].y != bdry_pts[ct-1].y)
	ct++;
    }

  return(ct);
} /* gen_bdry2 */



