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
 *	extend_clump.c
 *
 * PURPOSE
 *	Given a clump of intervals, extend it by a number of points (pixels)
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Feb 1, 1993: Created.
 */

#include <stdio.h>
#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 * 	extend_clump - given a clump of intervals, generate a set of
 * intervals which extends this clump by a number of points
 *
 * INPUTS:
 * 	interval_order - array of pointers to intervals in the clump
 * 	num_intervals - dimension of interval_order
 *	x - number of points to extend the clump in x direction
 *	y - number of points to extend the clump in y direction
 *	box - two dimensional bounding box for the area of extension
 *	num_rows - number of rows in the plane that the clump lies in
 *
 * OUTPUTS:
 * 	out_array - output array 	
 *
 * RETURNS:
 *       the size of out_array or -1 on failure
 *
 * NOTES:
 * 	Each interval in the clump is expanded to generate a set of
 * intervals.
 *
 * 1.  Generate a new set of intervals by extending each interval in the
 * clump.
 *
 * 2.  Link each of the intervals in the new set to a linked list of
 * intervals for each row.
 *
 * 3.  Sort each row into interval ascending order.  Note that interval A
 * is before interval B if:
 *
 * a.  The left endpoint of A is to the left of the left endpoint of B
 * b.  If the left endpoints of A and B are identical, the right endpoint
 * of A is to the left to the right endpoint of B.
 *  (See overlap.c for details)
 *
 * 4.  Form the union of intervals for each row.
 *
 */
int
EG_extend_clump_2d(Interval *intervals, int num_intervals, int x, int y, Interval **pout_array, int *pout_size, Box_2d *box, int num_rows)
{
  int i;
  Interval **int_array;
  Interval_link_hdr *link_array;
  Interval_link *links;
  int num_ints;
  int offset;
  Interval *out_array;
  int out_size;
  Interval *work_array;
  int work_size;

  work_array = NULL;
  work_size = 0;

  /* extend all the intervals in the clump */

  num_ints = EG_extend_int_2d(intervals, num_intervals, x, y,  &work_array, &work_size, box);
    
  if (num_ints < 0)
    {
      /* free up unnecessary allocated space */
      (void)EG_free(work_array);
      return(-1);
    }

/*  printf("extended intervals\n");
  EG_print_intervals(work_array, num_ints); */

  /* allocate space for link arrays */
  link_array = EG_malloc(num_rows * sizeof(Interval_link_hdr));
  if (link_array == NULL)
    {
      /* free up unnecessary allocated space */
      (void)EG_free(work_array);
      (void)EG_free(link_array);
       return(-1);
     }

   links = EG_malloc(num_ints * sizeof(Interval_link));
   if (links == NULL)
     {
       /* free up unnecessary allocated space */
       (void)EG_free(work_array);
       (void)EG_free(link_array);
       (void)EG_free(links);
       return(-1);
     }

   /* link the intervals in work_array to appropriate rows */

   EG_link_intervals(work_array, num_ints, num_rows, link_array, links);

   /* allocate an array of pointers to intervals to dump the links */
   int_array= EG_malloc(num_ints * sizeof(Interval *));
   if (int_array == NULL)
     {
       /* free up unnecessary allocated space */
       (void)EG_free(work_array);
       (void)EG_free(link_array);
       (void)EG_free(links);
       (void)EG_free(int_array);
       return(-1);
     }

   /* reallocate output array if necessary */
   out_array = *pout_array;
   out_size = *pout_size;

   if (out_size < num_ints)
     {
       out_array = (Interval *)EG_realloc(out_array, num_ints * sizeof(Interval));
       if (out_array == NULL)
	 {
	   /* free up unnecessary allocated space */
	   (void)EG_free(work_array);
	   (void)EG_free(link_array);
	   (void)EG_free(links);
	   (void)EG_free(int_array);
	   return(-1);
	 }
       out_size = num_ints;
     }

   /*
    * for each row, dump the links, sort the resulting intervals into
    * ascending order and form the union of the intervals
    */
   offset = 0;
   for (i=0; i<num_rows; i++)
     {
       int size;

       if (link_array[i].size == 0)
	 continue;


       /* dump links for one row - this is why 1 is used in the call below */
       EG_dump_links(&link_array[i], 1, int_array);

       /* sort each row into interval ascending order */
       EG_sort_ints_1d(int_array, link_array[i].size);

/*	EG_print_pintervals(int_array, link_array[i].size); */

       /* form the union of intervals for each row */
       size = EG_union_row(int_array, link_array[i].size,  &out_array[offset]);
/*	EG_print_intervals(&out_array[offset], size); */

 /*      printf("row %d, size %d\n", i, size); */
       offset += size;
     }


   /* free up unnecessary allocated space */
  (void)EG_free(work_array);
  (void)EG_free(link_array);
  (void)EG_free(links);
  (void)EG_free(int_array);

   *pout_array = out_array;
   *pout_size =   out_size;

   return(offset);
 }
 
 /*
  * DESCRIPTION:    
  * 	extend_pclump_2d - given a clump of intervals, generate a set of
  * intervals which extends this clump by a number of points
  *
  * INPUTS:
  * 	interval_order - array of pointers to intervals in the clump
  * 	num_intervals - dimension of interval_order
  *	x - number of points to extend the clump in x direction
  *	y - number of points to extend the clump in y direction
  *	box - two dimensional bounding box for the area of extension
  *	num_rows - number of rows in the plane that the clump lies in
  *
  * OUTPUTS:
  * 	out_array - output array 	
  *
  * RETURNS:
  *       the size of out_array or -1 on failure
  *
  * NOTES:
  * 	Each interval in the clump is expanded to generate a set of
  * intervals.
  *
  * 1.  Generate a new set of intervals by extending each interval in the
  * clump.
  *
  * 2.  Link each of the intervals in the new set to a linked list of
  * intervals for each row.
  *
  * 3.  Sort each row into interval ascending order.  Note that interval A
  * is before interval B if:
  *
  * a.  The left endpoint of A is to the left of the left endpoint of B
  * b.  If the left endpoints of A and B are identical, the right endpoint
  * of A is to the left to the right endpoint of B.
  *  (See overlap.c for details)
  *
  * 4.  Form the union of intervals for each row.
  *
  */
int
EG_extend_pclump_2d(Interval **interval_order, int num_intervals, int x, int y, Interval **pout_array, int *pout_size, Box_2d *box, int num_rows)
 {
   int i;
   Interval **int_array;
   Interval_link_hdr *link_array;
   Interval_link *links;
   int num_ints;
   int offset;
   Interval *out_array;
   int out_size;
   Interval *work_array;
   int work_size;

   work_array = NULL;
   work_size = 0;

   /* extend all the intervals in the clump */
   num_ints = EG_extend_pint_2d(interval_order, num_intervals, x, y,  &work_array, &work_size, box);

   if (num_ints < 0)
     {
       /* free up unnecessary allocated space */
       (void)EG_free(work_array);
       return(-1);
     }

 /*  printf("printing work array\n");
   EG_print_intervals(work_array, num_ints); */

   /* allocate space for link arrays */
   link_array = EG_malloc(num_rows * sizeof(Interval_link_hdr));
   if (link_array == NULL)
     {
       /* free up unnecessary allocated space */
       (void)EG_free(work_array);
       (void)EG_free(link_array);
       return(-1);
     }

   links = EG_malloc(num_ints * sizeof(Interval_link));
   if (links == NULL)
     {
       /* free up unnecessary allocated space */
       (void)EG_free(work_array);
       (void)EG_free(link_array);
       (void)EG_free(links);
       return(-1);
     }

   /* link the intervals in work_array to appropriate rows */
   EG_link_intervals(work_array, num_ints, num_rows, link_array, links);

   /* allocate an array of pointers to intervals to dump the links */
   int_array= EG_malloc(num_ints * sizeof(Interval *));
   if (int_array == NULL)
     {
       /* free up unnecessary allocated space */
       (void)EG_free(work_array);
       (void)EG_free(link_array);
       (void)EG_free(links);
       (void)EG_free(int_array);
       return(-1);
     }

   /* reallocate output array if necessary */
   out_array = *pout_array;
   out_size = *pout_size;

   if (out_size < num_ints)
     {
       out_array = (Interval *)EG_realloc(out_array, num_ints * sizeof(Interval));
       if (out_array == NULL)
	 {
	   /* free up unnecessary allocated space */
	   (void)EG_free(work_array);
	   (void)EG_free(link_array);
	   (void)EG_free(links);
	   (void)EG_free(int_array);
	   return(-1);
	 }
       out_size = num_ints;
     }

 /*  printf("after all allocation\n"); */

   /*
    * for each row, dump the links, sort the resulting intervals into
    * ascending order and form the union of the intervals
    */
   offset = 0;
   for (i=0; i<num_rows; i++)
     {
       int size;

       if (link_array[i].size == 0)
	 continue;

       /* dump links for one row - this is why 1 is used in the call below */
       EG_dump_links(&link_array[i], 1, int_array);

       /* sort each row into interval ascending order */
 /*      printf("before sort\n"); */
       EG_sort_ints_1d(int_array, link_array[i].size);

 /*      EG_print_pintervals(int_array, link_array[i].size);
       printf("before union_row\n"); */

       /* form the union of intervals for each row */
       size = EG_union_row(int_array, link_array[i].size,  &out_array[offset]);
 /*      printf("offset %d, row %d, size %d\n", offset, i, size); */
       offset += size;
     }

   /* free up unnecessary allocated space */
   (void)EG_free(work_array);
   (void)EG_free(link_array);
   (void)EG_free(links);
   (void)EG_free(int_array);

   *pout_array = out_array;
   *pout_size =   out_size;

   return(offset);
 }

