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
 * Module: convex_intersect.c
 *
 * Author: Gerry Wiener
 *
 * Date:   7/31/95
 *
 * Description:
 *     Determine the polygon representing the intersection of two convex polygons.
 */

#include <stdio.h>
#include <stdlib.h>

#include <euclid/geometry.h>

#ifdef NOTNOW
static int overlap(double begin1, double end1, double begin2, double end2, double *begin3, double *end3);
#endif

/*
 * Name:    
 *    convex_intersect
 *
 * Purpose:
 *    Find the convex polygon representing the intersection of two convex
 * polygons.  NOTE: We assume that each given polygon does not have two
 * collinear edges.
 *
 * In:
 *    poly1 - array of points in first polygon sorted in COUNTERCLOCKWISE!! order
 *    size1 - size of poly1
 *    poly2 - array of points in second polygon sorted in COUNTERCLOCKWISE!! order
 *    size2 - size of poly2
 *
 * Out:
 *    int_pt - array of points of intersection of polygon edges
 *    int_size - size of int_pt
 *    out_poly - array of points represent polygon of intersection
 *    (The arrayout_poly contains int_pt plus potentially additional vertices from
 *    poly1 and poly2 2.  It should have dimension at least size1+size2.)
 *    out_size - size of out_poly
 *
 * Returns:
 * -1 - error occured
 *  0 - no intersection 
 *  1 - poly1 is inside poly2
 *  2 - poly2 is inside poly1
 *  3 - there is at least one point of intersection of poly1 and poly2
 *
 * Notes:
 *
 *      1.  Iterate through the edges in poly1
 *      2.  Check for points of intersection from edges in poly2
 *      3.  Record points of intersection in order of occurence
 *
 *  See Computational Geometry by Shamos and Preparata for additional information.
 */

int EG_convex_intersect(Point_d *poly1, int poly1_size, Point_d *poly2, int poly2_size, Point_d *out_poly, int *out_size, Point_d *int_pt, int *int_size)
{
  int base1;
  int base2;
  int finish;
  int first;
  int i;
  int *ind1;
  int *ind2;
  int int_ct;
  int j;
  int k;
  double cross_prod;
  int ct;
  double *position;
  double poly1_pos;
  Point_d pt;
  int ret;
  int start;
  int size1;
  int size2;
  int tmp_ind1;
  int tmp_ind2;
  double tmp_pos;
  Point_d tmp_pt;
  Point_d v1;
  Point_d v2;

  size1 = poly1_size - 1;
  size2 = poly2_size - 1;
  if (size1 <= 2 || size2 <= 2)
    return(-1);                        /* an input polygon is degenerate */

  ind1 = (int *)malloc((size1 + size2 + 1) * sizeof(int));
  ind2 = (int *)malloc((size1 + size2 + 1) * sizeof(int));
  position = (double *)malloc((size1 + size2 + 1) * sizeof(double));
  if (ind1 == NULL || ind2 == NULL || position == NULL)
  {
    if (ind1)
       free(ind1);
    if (ind2)
       free(ind2);
    if (position)
       free(position);
    return(-1);
  }
  /* find all intersections */
  k = 0;
  for (i=0; i<size1; i++)
    {
      start = k;
      for (j=0; j<size2; j++)
        {
          /*
           * The order of the arguments, poly2 and poly1, in
           * segment_intersect is significant since the value poly1_pos
           * is used to calculate the position of the point of
           * intersection with respect to the second line segment.
           */
          ret = EG_segment_intersect(&poly2[j], &poly2[j+1], &poly1[i], &poly1[i+1], &pt, &poly1_pos);

          if (ret == 1)
            {
              /* there is one point of intersection */
/*              printf("ret %d, segments poly1[%d] (%f,%f) (%f, %f) and poly2[%d] (%f,%f) (%f, %f), poly1_pos %f\n", ret, i, poly1[i].x, poly1[i].y, poly1[i+1].x, poly1[i+1].y, j, poly2[j].x, poly2[j].y, poly2[j+1].x, poly2[j+1].y, poly1_pos); */

              int_pt[k] = pt;
                  
              /* record indices of intersecting segments */
              ind1[k] = i;
              ind2[k] = j;

              /* intersection is at poly1[i] + poly1_pos * (poly1[i+1] - poly1[i]) */
              position[k] = poly1_pos; 

              k++;
            }
          else if (ret == -1)
            /*
             * one of the segments in the input polygons is really a
             * point so error return
             */
            goto error_return;
          else if (ret == 2)
            {
              /*
               * Find the dot product of the line segments.  If negative,
               * the polygons cannot intersect.
               */
              v1.x = poly1[i+1].x - poly1[i].x;
              v1.y = poly1[i+1].y - poly1[i].y;

              v2.x = poly2[j+1].x - poly2[j].x;
              v2.y = poly2[j+1].y - poly2[j].y;

              if (DOT(&v1, &v2) < 0)
	      {
                free (ind1);
                free (ind2);
                free (position);
                return(0);
              }
                
#ifdef NOTNOW
              /* the segments are collinear and overlap in a point or line segment */
/*              printf("ret %d, segments poly1[%d] (%f,%f) (%f, %f) and poly2[%d] (%f,%f) (%f, %f)\n", ret, i, poly1[i].x, poly1[i].y, poly1[i+1].x, poly1[i+1].y, j, poly2[j].x, poly2[j].y, poly2[j+1].x, poly2[j+1].y); */
              /*
               * Handle overlap of segments in which there are two points
               * of intersection.  
               */
              denomx = poly1[i+1].x - poly1[i].x;
              denomy = poly1[i+1].y - poly1[i].y;
              if (ABS(denomx) > MACH_EPS)
                {
                  pos1 = (poly2[j].x - poly1[i].x)/denomx;
                  pos2 = (poly2[j+1].x - poly1[i].x)/denomx;
                  if (pos1 < pos2)
                    {
                      if (overlap(pos1, pos2, 0, 1, &begin, &end) < 0)
                        goto error_return;
                    }
                  else if (pos1 > pos2)
                    {
                      if (overlap(pos2, pos1, 0, 1, &begin, &end) < 0)
                        goto error_return;
                    }
                  else
                    goto error_return;
                }
              else
                {
                  if (ABS(denomy) < MACH_EPS)
                    goto error_return;

                  pos1 = (poly2[j].y - poly1[i].y)/denomy;
                  pos2 = (poly2[j+1].y - poly1[i].y)/denomy;
                  if (pos1 < pos2)
                    {
                      if (overlap(pos1, pos2, 0, 1, &begin, &end) < 0)
                        goto error_return;
                    }
                  else if (pos1 > pos2)
                    {
                      if (overlap(pos2, pos1, 0, 1, &begin, &end) < 0)
                        goto error_return;
                    }
                  else
                    goto error_return;
                }

              if (ABS(begin-end) < MACH_EPS)
                {
                  int_pt[k].x = poly1[i].x + begin * denomx;
                  int_pt[k].y = poly1[i].y + begin * denomy;
                  ind1[k] = i;
                  ind2[k] = j;

                  /* intersection is at poly1[i] + begin * (poly1[i+1] - poly1[i]) */
                  position[k] = begin; 
/*                  printf("ind1[%d] = %d, ind2[%d] = %d, pos %f\n\n", k, ind1[k], k, ind2[k], begin); */
                  k++;
                }
              else
                {
                  int_pt[k].x = poly1[i].x + begin * denomx;
                  int_pt[k].y = poly1[i].y + begin * denomy;
                  ind1[k] = i;
                  ind2[k] = j;
/*                  printf("ind1[%d] = %d, ind2[%d] = %d\n\n", k, ind1[k], k, ind2[k]); */
                  k++;
                  int_pt[k].x = poly1[i].x + end * denomx;
                  int_pt[k].y = poly1[i].y + end * denomy;
                  ind1[k] = i;
                  ind2[k] = j;
/*                  printf("ind1[%d] = %d, ind2[%d] = %d\n\n", k, ind1[k], k, ind2[k]); */
                  k++;
                }
#endif
            }
        }

      /*
       * In the algorithm generating the points of intersection, each
       * point of intersection can appear one, two, three or four times
       * for a given edge.  Sort intersection points into positional
       * order along each edge from initiating vertex to ending vertex.
       * Nothing has to be done for 1 intersection.  If the the edge has
       * two intersections, they can be ordered appropriately.  dentical
       * in position.  Finally for four intersections, all must be
       * identical.
       */
      
      /* print the points of intersection */
#ifdef NOTNOW
      if (k - start > 1)
        {
          printf("BEFORE: points on edge\n"); 
          for (l=start; l<k; l++)
            {
              (void) printf("int_pt[%d] = %f, %f, ind1 %d, ind2 %d, pos %f\n", l, int_pt[l].x, int_pt[l].y, ind1[l], ind2[l], position[l]);
            }
        }
#endif
      switch (k - start)
        {
        case 0:
        case 1:
          break;

        case 2:
          /*
           * Here there can be two identical or two distinct
           * intersections.  If there are two distinct intersections,
           * order by position relative to the beginning of the segment.
           */
          if (position[start+1] < position[start])
            {
              /* switch */
              tmp_pt = int_pt[start];
              tmp_ind1 = ind1[start];
              tmp_ind2 = ind2[start];
              tmp_pos = position[start];

              int_pt[start] = int_pt[start+1];
              ind1[start] = ind1[start+1];
              ind2[start] = ind2[start+1];
              position[start] = position[start+1];

              int_pt[start+1] = tmp_pt;
              ind1[start+1] = tmp_ind1;
              ind2[start+1] = tmp_ind2;
              position[start+1] = tmp_pos;
            }
          else if (position[start+1] == position[start])
            {
              if (ind2[start] == 0 && ind2[start+1] == size2 - 1)
                {
                  /* switch */
                  tmp_pt = int_pt[start];
                  tmp_ind1 = ind1[start];
                  tmp_ind2 = ind2[start];
                  tmp_pos = position[start];

                  int_pt[start] = int_pt[start+1];
                  ind1[start] = ind1[start+1];
                  ind2[start] = ind2[start+1];
                  position[start] = position[start+1];

                  int_pt[start+1] = tmp_pt;
                  ind1[start+1] = tmp_ind1;
                  ind2[start+1] = tmp_ind2;
                  position[start+1] = tmp_pos;
                }
            }
          break;

        case 3:
          /*
           * Here two of the intersections must be identical.
           */
          if (position[start] == position[start+1])
            {
              if (position[start] > position[start+2])
                {
                  /* right circular shift */
                  tmp_pt = int_pt[start+2];
                  tmp_ind1 = ind1[start+2];
                  tmp_ind2 = ind2[start+2];
                  tmp_pos = position[start+2];
                  
                  int_pt[start+2] = int_pt[start+1];
                  ind1[start+2] = ind1[start+1];
                  ind2[start+2] = ind2[start+1];
                  position[start+2] = position[start+1];
                  
                  int_pt[start+1] = int_pt[start];
                  ind1[start+1] = ind1[start];
                  ind2[start+1] = ind2[start];
                  position[start+1] = position[start];
                  
                  int_pt[start] = tmp_pt;
                  ind1[start] = tmp_ind1;
                  ind2[start] = tmp_ind2;
                  position[start] = tmp_pos;
                }
              else if (position[start] < position[start+2])
                break;
              else
                goto error_return;
            }
          else if (position[start+1] == position[start+2])
            {
              if (position[start] > position[start+2])
                {
                  /* left circular shift */
                  tmp_pt = int_pt[start];
                  tmp_ind1 = ind1[start];
                  tmp_ind2 = ind2[start];
                  tmp_pos = position[start];
                  
                  int_pt[start] = int_pt[start+1];
                  ind1[start] = ind1[start+1];
                  ind2[start] = ind2[start+1];
                  position[start] = position[start+1];
                  
                  int_pt[start+1] = int_pt[start+2];
                  ind1[start+1] = ind1[start+2];
                  ind2[start+1] = ind2[start+2];
                  position[start+1] = position[start+2];
                  
                  int_pt[start+2] = tmp_pt;
                  ind1[start+2] = tmp_ind1;
                  ind2[start+2] = tmp_ind2;
                  position[start+2] = tmp_pos;
                }
              else if (position[start] < position[start+2])
                break;
              else
                goto error_return;
            }
          else if (position[start] == position[start+2])
            {
              if (position[start] > position[start+1])
                {
                  /* left circular shift */
                  tmp_pt = int_pt[start];
                  tmp_ind1 = ind1[start];
                  tmp_ind2 = ind2[start];
                  tmp_pos = position[start];
                  
                  int_pt[start] = int_pt[start+1];
                  ind1[start] = ind1[start+1];
                  ind2[start] = ind2[start+1];
                  position[start] = position[start+1];
                  
                  int_pt[start+1] = int_pt[start+2];
                  ind1[start+1] = ind1[start+2];
                  ind2[start+1] = ind2[start+2];
                  position[start+1] = position[start+2];
                  
                  int_pt[start+2] = tmp_pt;
                  ind1[start+2] = tmp_ind1;
                  ind2[start+2] = tmp_ind2;
                  position[start+2] = tmp_pos;
                }
              else if (position[start] < position[start+1])
                {
                  /* right circular shift */
                  tmp_pt = int_pt[start+2];
                  tmp_ind1 = ind1[start+2];
                  tmp_ind2 = ind2[start+2];
                  tmp_pos = position[start+2];
                  
                  int_pt[start+2] = int_pt[start+1];
                  ind1[start+2] = ind1[start+1];
                  ind2[start+2] = ind2[start+1];
                  position[start+2] = position[start+1];
                  
                  int_pt[start+1] = int_pt[start];
                  ind1[start+1] = ind1[start];
                  ind2[start+1] = ind2[start];
                  position[start+1] = position[start];
                  
                  int_pt[start] = tmp_pt;
                  ind1[start] = tmp_ind1;
                  ind2[start] = tmp_ind2;
                  position[start] = tmp_pos;
                }
              else
                goto error_return;
            }
          else
            goto error_return;
          break;

        case 4:
          /*
           * Here there must be two sets of two identical intersections.
           * Re-order if necessary.
           */
          if (position[start] < position[start+1])
            {
              if (position[start+1] != position[start+2] || position[start] != position[start+3])
                goto error_return;
              else
                {
                  /* right circular shift */
                  tmp_pt = int_pt[start+3];
                  tmp_ind1 = ind1[start+3];
                  tmp_ind2 = ind2[start+3];
                  tmp_pos = position[start+3];
                  
                  int_pt[start+3] = int_pt[start+2];
                  ind1[start+3] = ind1[start+2];
                  ind2[start+3] = ind2[start+2];
                  position[start+3] = position[start+2];
                  
                  int_pt[start+2] = int_pt[start+1];
                  ind1[start+2] = ind1[start+1];
                  ind2[start+2] = ind2[start+1];
                  position[start+2] = position[start+1];
                  
                  int_pt[start+1] = int_pt[start];
                  ind1[start+1] = ind1[start];
                  ind2[start+1] = ind2[start];
                  position[start+1] = position[start];
                  
                  int_pt[start] = tmp_pt;
                  ind1[start] = tmp_ind1;
                  ind2[start] = tmp_ind2;
                  position[start] = tmp_pos;
                }
            }
          else if (position[start] > position[start+1])
            {
              if (position[start+1] != position[start+2] || position[start] != position[start+3])
                goto error_return;
              else
                {
                  /* left circular shift */
                  tmp_pt = int_pt[start];
                  tmp_ind1 = ind1[start];
                  tmp_ind2 = ind2[start];
                  tmp_pos = position[start];
                  
                  int_pt[start] = int_pt[start+1];
                  ind1[start] = ind1[start+1];
                  ind2[start] = ind2[start+1];
                  position[start] = position[start+1];
                  
                  int_pt[start+1] = int_pt[start+2];
                  ind1[start+1] = ind1[start+2];
                  ind2[start+1] = ind2[start+2];
                  position[start+1] = position[start+2];
                  
                  int_pt[start+2] = int_pt[start+3];
                  ind1[start+2] = ind1[start+3];
                  ind2[start+2] = ind2[start+3];
                  position[start+2] = position[start+3];
                  
                  int_pt[start+3] = tmp_pt;
                  ind1[start+3] = tmp_ind1;
                  ind2[start+3] = tmp_ind2;
                  position[start+3] = tmp_pos;
                }
            }
          else if (position[start] > position[start+2])
            {
              if (position[start+1] != position[start+2] || position[start] != position[start+3])
                goto error_return;
              else
                {
                  /* right circular shift 2 */
                  tmp_pt = int_pt[start+3];
                  tmp_ind1 = ind1[start+3];
                  tmp_ind2 = ind2[start+3];
                  tmp_pos = position[start+3];
                  
                  int_pt[start+3] = int_pt[start+1];
                  ind1[start+3] = ind1[start+1];
                  ind2[start+3] = ind2[start+1];
                  position[start+3] = position[start+1];
                  
                  int_pt[start+1] = tmp_pt;
                  ind1[start+1] = tmp_ind1;
                  ind2[start+1] = tmp_ind2;
                  position[start+1] = tmp_pos;
                  
                  tmp_pt = int_pt[start+2];
                  tmp_ind1 = ind1[start+2];
                  tmp_ind2 = ind2[start+2];
                  tmp_pos = position[start+2];
                  
                  int_pt[start] = int_pt[start+2];
                  ind1[start] = ind1[start+2];
                  ind2[start] = ind2[start+2];
                  position[start] = position[start+2];
                  
                  int_pt[start+2] = tmp_pt;
                  ind1[start+2] = tmp_ind1;
                  ind2[start+2] = tmp_ind2;
                  position[start+2] = tmp_pos;
                }
            }
          else if (position[start] < position[start+2])
            {
              if (position[start+1] != position[start+2] || position[start] != position[start+3])
                goto error_return;
              else
                break;
            }
          else
            goto error_return;
          break;

        default:
          goto error_return;
        }

#ifdef NOTNOW
      if (k - start > 1)
        {
          printf("AFTER: points on edge\n");
          for (l=start; l<k; l++)
            {
              (void) printf("int_pt[%d] = %f, %f, ind1 %d, ind2 %d, pos %f\n", l, int_pt[l].x, int_pt[l].y, ind1[l], ind2[l], position[l]);
            }
        }
#endif
    }

  /* set sentinels */
  int_ct = k;
  *int_size = int_ct;
  *out_size = 0;                /* just in case there is no intersection */
  if (int_ct == 0)
    {
      ret = EG_inside_poly(&poly1[0], poly2, size2);
      if (ret)
        {
          free(ind1);
          free(ind2);
          free(position);

          /* poly1 is inside poly2 and is the polygon of intersection */
          for (i=0; i<poly1_size; i++)
            {
              out_poly[i].x = poly1[i].x;
              out_poly[i].y = poly1[i].y;
            }

          *out_size = poly1_size;
          return(1);
        }

      /* check for containment */
      ret = EG_inside_poly(&poly2[0], poly1, size1);
      if (ret)
        {
          free(ind1);
          free(ind2);
          free(position);

          /* poly2 is inside poly1 and is the polygon of intersection */
          for (i=0; i<poly2_size; i++)
            {
              out_poly[i].x = poly2[i].x;
              out_poly[i].y = poly2[i].y;
            }

          *out_size = poly2_size;
          return(2);
        }

      free(ind1);
      free(ind2);
      free(position);
      return(0);
    }

  /* print the points of intersection */
#ifdef NOTNOW
  for (i=0; i<int_ct; i++)
    {
      (void) printf("int_pt[%d] = %f, %f, ind1 %d, ind2 %d, pos %f\n", i, int_pt[i].x, int_pt[i].y, ind1[i], ind2[i], position[i]);
    }
#endif  
  
  /*
   * Determine the sequence of points of the intersection polygon.  Start
   * at the first point of intersection.  
   * We handle the following cases:
   *
   * 0.  Exactly one point of intesection has been generated - find the
   * cross product of the edges that comprise the intersection starting
   * with an edge from poly1.  If the cross product is positive, use the
   * edge from poly2, otherwise use the edge from poly1.
   *
   * 1.  Two identical points of intersection have been generated -
   *     a. a vertex of poly1, for example, could lie on an edge of
   * poly2
   *     b. a vertex of poly1 and a vertex of poly2 could overlap at a
   * shared edge
   *
   * 2.  Four identical points of intersection have been generated - a
   * corner point of poly1 must be identical with a corner point of
   * poly2, the polygons will not share an edge at this vertex and the
   * polygons must have a non-trivial intersection.  Two edges must enter
   * the intersection and two edges must leave.  Find the cross product
   * of the edges that leave using the strategy in 0.
   */

  /*
   * If intersection points at the beginning and ends of int_pt agree,
   * shift those at the beginning to the end.  This allows us to handle
   * the case of multiple identical intersection points uniformly.  Note
   * that the shifting order is significant.
   */
  start = 0;
  if (int_ct > 2)
    {
      for (i=0; i<int_ct-1 && NORM1(&int_pt[i], &int_pt[int_ct-1]) < MACH_EPS; i++)
        {
          int_pt[int_ct + i] = int_pt[i];
          ind1[int_ct + i] = ind1[i];
          ind2[int_ct + i] = ind2[i];
          position[int_ct + i] = position[i];
        }
      start = i;
    }

  finish = int_ct + start;
  ind1[int_ct+start] = ind1[start];
  ind2[int_ct+start] = ind2[start];

#ifdef NOTNOW
  for (i=0; i<int_ct+start; i++)
    {
      (void) printf("int_pt[%d] = %f, %f, ind1 %d, ind2 %d, pos %f\n", i, int_pt[i].x, int_pt[i].y, ind1[i], ind2[i], position[i]);
    }
#endif

  i = start;
  k = 0;
  while (i < int_ct + start)
    {
      first = i;
      if (k == 0 || NORM1(&int_pt[first], &out_poly[k-1]) > MACH_EPS)
        out_poly[k] = int_pt[first];
      k++;
      i++;
      while (i < finish && NORM1(&int_pt[i], &int_pt[first]) < MACH_EPS)
        i++;

      switch (i - first)
        {
        case 1:
          /* single intersection point */
          v1.x = poly1[ind1[first]+1].x - poly1[ind1[first]].x;
          v1.y = poly1[ind1[first]+1].y - poly1[ind1[first]].y;
          v2.x = poly2[ind2[first]+1].x - poly2[ind2[first]].x;
          v2.y = poly2[ind2[first]+1].y - poly2[ind2[first]].y;
          cross_prod = CROSS(&v1, &v2);
          if (cross_prod > 0)
            {
              /* march along poly2 */
              if (ind2[first+1] == ind2[first])
                break;
              else
                {
                  for (j=(ind2[first]+1)%size2; j!=ind2[first+1]; j = (j+1)%size2)
                    {
                      out_poly[k] = poly2[j];
                      k++;
                    }
                  out_poly[k] = poly2[j];
                  k++;
                }
            }
          else
            {
              /* march along poly1 */
              if (ind1[first+1] == ind1[first])
                break;
              else
                {
                  for (j=(ind1[first]+1)%size1; j!=ind1[first+1]; j=(j+1)%size1)
                    {
                      out_poly[k] = poly1[j];
                      k++;
                    }
                      out_poly[k] = poly1[j];
                      k++;
                }
            }
          break;

        case 2:
          /* two consecutive points of intersection are identical */

          /* change modulus ??? like case 1 */
          /* check for vertex impinging on an edge */
          if (ind2[first] < ind2[first+1])
            {
              /* march along poly2 */
              for (j=ind2[first]+1; j!=ind2[first+2]; j++)
                {
                  out_poly[k] = poly2[j];
                  k++;
                }
              out_poly[k] = poly2[j];
              k++;
            }
          else if (ind2[first] > ind2[first+1])
            {
              for (j=ind2[first+1]+1; j!=ind2[first+2]; j++)
                {
                  out_poly[k] = poly2[j];
                  k++;
                }
              out_poly[k] = poly2[j];
              k++;
            }
          else if (ind1[first] < ind1[first+1])
            {
              /* march along poly1 */
              for (j=ind1[first]+1; j!=ind1[first+2]; j++)
                {
                  out_poly[k] = poly1[j];
                  k++;
                }
              out_poly[k] = poly1[j];
              k++;
            }
          else if (ind1[first] > ind1[first+1])
            {
              /* march along poly1 */
              for (j=ind1[first+1]+1; j!=ind1[first+2]; j++)
                {
                  out_poly[k] = poly1[j];
                  k++;
                }
              out_poly[k] = poly1[j];
              k++;
            }
          else
            goto error_return;
            
          break;

        case 4:
          /* four consecutive points of intersection are identical */

          /* determine the edges that leave */
#ifdef NOTNOW
          if (NORM1(&int_pt[first], &poly1[ind1[first]]) < MACH_EPS)
            base1 = ind1[first];
          else 
            base1 = (ind1[first] + 1)%size1;
          if (NORM1(&int_pt[first], &poly2[ind2[first]]) < MACH_EPS)
            base2 = ind2[first];
          else
            base2 = (ind2[first] + 1)%size2;
#endif
          base1 = ind1[first+2];
          base2 = ind2[first+1];
          v1.x = poly1[base1+1].x - poly1[base1].x;
          v1.y = poly1[base1+1].y - poly1[base1].y;
          v2.x = poly2[base2+1].x - poly2[base2].x;
          v2.y = poly2[base2+1].y - poly2[base2].y;
          cross_prod = CROSS(&v1, &v2);
          if (cross_prod > 0)
            {
              /* march along poly2 */
              for (j=base2; j!=ind2[first+4]; j=(j+1)%size2)
                {
                  out_poly[k] = poly2[j];
                  k++;
                }
              out_poly[k] = poly2[j];
              k++;
              
            }
          else
            {
              /* march along poly1 */
              for (j=base1; j!=ind1[first+4]; j=(j+1)%size1)
                {
                  out_poly[k] = poly1[j];
                  k++;
                }
              out_poly[k] = poly1[j];
              k++;
            }

          break;
        }
    }
  
  out_poly[k] = out_poly[0];
  k++;

/*  printf ("k is %d\n", k); */
  *out_size = k;
  free(ind1);
  free(ind2);
  free(position);

  ct = 0;
/*  (void) printf("out_poly[%d] = %f, %f\n", ct, out_poly[ct].x, out_poly[ct].y);  */
  for (i=1; i<*out_size; i++)
    {
      if (NORM1(&out_poly[i], &out_poly[ct]) > MACH_EPS)
        {
          ct++;
          if (i != ct)
            {
              out_poly[ct].x = out_poly[i].x;
              out_poly[ct].y = out_poly[i].y;
            }
/*          (void) printf("out_poly[%d] = %f, %f\n", ct, out_poly[ct].x, out_poly[ct].y);  */
        }
    }

  ct++;
  out_poly[ct].x = out_poly[0].x;
  out_poly[ct].y = out_poly[0].y;
  *out_size = ct;
  return(3);

 error_return:
  printf("error occured\n");
  free(ind1);
  free(ind2);
  free(position);
  return(-1);
}

#ifdef NOTNOW
static int overlap(double begin1, double end1, double begin2, double end2, double *begin3, double *end3)
{
  if (begin1 < begin2)
    {
      if (end1 < begin2)
        return(0);
      else
        {
          if (end1 < end2)
            {
              *begin3 = begin2;
              *end3 = end1;
              return(1);
            }
          else                        /* end1 >= end2 */
            {
              *begin3 = begin2;
              *end3 = end2;
              return(1);
            }
        }
    }
  else                                /* begin1 >= begin2 */
    {
      if (begin1 > end2)
        return(0);
      else
        {
          if (end1 < end2)
            {
              *begin3 = begin1;
              *end3 = end1;
              return(1);
            }
          else                        /* end1 >= end2 */
            {
              *begin3 = begin1;
              *end3 = end2;
              return(1);
            }
        }
    }
}
#endif


