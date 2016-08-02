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
 * 	split_clump
 *
 * PURPOSE
 * 	Split a clump into pieces if the clump contains thin
 * connections in its interior
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - Jun 14, 1994: Created.
 */

#include <euclid/alloc.h>
#include <euclid/clump.h>
#include <euclid/link.h>

static char *prog;

struct array_position
{
  int index;
  int value;
};

/* struct mem_ptr
{
  void *p;
  int size;
}; */


#ifdef NOTNOW
static int check_adj4(unsigned char *new_clump_array, int ind, int xdim);
#endif
static int check_adj8(unsigned char *new_clump_array, int ind, int xdim);

/*
 * DESCRIPTION:    
 * 	split_clump_2d
 *
 * INPUTS:
 * 	intervals - array of intervals
 *      num_ints - size of intervals array
 *      xdim - number of columns in underlying array
 *      ydim - number of rows in underlying array
 *      threshold - number of points to erode
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array parray back
 * to an array without the extra rows and columns of zeroes.
 *
 * OUTPUTS:
 * 	new_clump_array - array of new clumps (This array will also be
 * dimensioned similar to the 2D Example above.
 *
 * RETURNS:
 *       1 grew completely back
 *       0 grew partially back but some clumps disappeared by erosion
 *       -1 on error
 *
 * NOTES:
 * 	
 */
int EG_split_clump_2d(Row_hdr *row_hdr, int num_intervals, int xdim, int ydim,
int threshold, unsigned char *new_clump_array, int *num_clumps)
{
  Interval **interval_order;
  Clump_order *clump_order;
  int ct;
  Interval *new_intervals;
  unsigned char *edm_array;
  Link *elt;
  int elt_offset;
  int grow_success;
  Link *head;
  Link head_link;
  int i;
  int int_array_size;
  int j;
  int k;
  int num_new_ints;
  int offset;
  struct array_position *place;
  Link *previous;
  struct mem_ptr ptr_array[32];
  int ptr_ct;
  Link *save;
  int size;
  int ret;
  Row_hdr *new_row_hdr;
  int yoff;

  ptr_ct = 0;
  yoff = 1;
  new_intervals = NULL;
  int_array_size = 0;
  grow_success = 1;

  /* allocate space for arrays */

  /* allocate space for edm_array */
  size = xdim * ydim;
  edm_array = (unsigned char *)EG_malloc(size);
  ptr_array[ptr_ct].p = edm_array;
  ptr_array[ptr_ct].size = size;
  ptr_ct++;
  if (edm_array == NULL)
    {
      fprintf(stderr, "%s: can't malloc edm_array\n", prog);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }

  /* allocate space for place array */
  place = (struct array_position *)EG_malloc(size*sizeof(struct array_position));
  ptr_array[ptr_ct].p = place;
  ptr_array[ptr_ct].size = size*sizeof(struct array_position);
  ptr_ct++;
  if (place == NULL)
    {
      fprintf(stderr, "%s: can't malloc place\n", prog);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* apply euclidean distance map */
  printf("calculating euclidean distance map\n");
  EG_edm_2d(row_hdr, edm_array, xdim, ydim, yoff);
/*  EG_print_carray_2d(edm_array, ydim, xdim); */

  /* allocate space for a new row header */
  new_row_hdr = EG_malloc(ydim*sizeof(Row_hdr));
  ptr_array[ptr_ct].p = new_row_hdr;
  ptr_array[ptr_ct].size = ydim*sizeof(Row_hdr);
  ptr_ct++;
  if (new_row_hdr == NULL)
    {
      fprintf(stderr, "%s: can't malloc row_hdr\n", prog);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* find the intervals over a given threshold distance */
  printf("finding new intervals based on erosion threshold\n");
  num_new_ints = EG_find_intervals(ydim, xdim, edm_array, &new_intervals, &int_array_size, new_row_hdr, threshold);
  ptr_array[ptr_ct].p = new_intervals;
  ptr_array[ptr_ct].size = int_array_size * sizeof(Interval);
  ptr_ct++;

  /* set clump ids in new_intervals to NULL_ID */
  EG_reset_clump_id(new_intervals, int_array_size);
  printf("the number of new intervals = %d\n", num_new_ints);
  /*  EG_print_intervals(new_intervals, num_new_ints); */
  
  /* allocate space for clump_order */
  clump_order = (Clump_order *)EG_malloc((num_intervals+1)*sizeof(Clump_order));
  ptr_array[ptr_ct].p = clump_order;
  ptr_array[ptr_ct].size = (num_intervals+1)*sizeof(Clump_order);
  ptr_ct++;
  if (clump_order == NULL)
    {
      fprintf(stderr, "%s: can't malloc clump_order\n", prog);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* allocate space for interval_order */
  interval_order = (Interval **)EG_malloc((num_intervals)*sizeof(Interval *));
  ptr_array[ptr_ct].p = interval_order;
  ptr_array[ptr_ct].size = (num_intervals)*sizeof(Interval *);
  ptr_ct++;
  if (interval_order == NULL)
    {
      fprintf(stderr, "%s: can't malloc interval_order\n", prog);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* reclump the output array to erode the original clump */
  printf("clumping eroded intervals\n");
  *num_clumps = EG_rclump_2d(new_row_hdr,  ydim, 0, 1, interval_order, clump_order);
  
  printf("number of clumps in eroded array =  %d\n", *num_clumps);
  
  /* assign clump values */
  EG_set_intervals_clump(new_clump_array, new_row_hdr, ydim, xdim);
  printf("eroded array is as follows:\n");
/*  EG_print_carray_2d(new_clump_array, ydim, xdim); */

  /* grow the clumps generated by the erosion */
  
  if (*num_clumps > 0)
    {
      /*
       * Iterate thru the input clumps taking array elements whose edm values
       * are below the threshold distance and throwing their indices onto a
       * linked list.
       */
      head = &head_link;
      EG_init_link_head(head);
      
      for (i=0; i<ydim; i++)
	{
	  offset = i * xdim;
	  for (j=0; j<row_hdr[i].size; j++)
	    {
	      for (k=row_hdr[i].intervals[j].begin; k<=row_hdr[i].intervals[j].end; k++)
		{
		  elt_offset = offset + k;
		  if (1 <= edm_array[elt_offset] && edm_array[elt_offset] <= threshold - 1)
		    {
		      elt = EG_new_link();
		      elt->data = elt_offset;
		      EG_add_link(head, elt);
		    }
		}
	    }
	}
      
/*      printf("printing links\n");
      EG_print_link(head); */
      
      /*
       * We now have a set of small clumps extracted from the larger clumps
       * by erosion.  Grow the small clumps back to the size of the original
       * clumps using points from the original intervals.
       */
      while (head->next != NULL)
	{
	  ct = 0;
	  
	  /*
	   * Iterate through the list checking adjacency to small clump array.
	   * If adjacent take index off list and place in array.
	   */
	  for (previous = head; previous->next != NULL; )
	    {
	      /* check adjacency */
	      ret = check_adj8(new_clump_array, (previous->next)->data, xdim);
/*	      printf("data is %d, ret is %d\n", previous->next->data, ret);  */
	      if (ret != 0)
		{
		  place[ct].index = (previous->next)->data;
		  place[ct].value = ret;
/*		  printf("place[%d].index = %d, place[%d].value = %d\n", ct, place[ct].index, ct, place[ct].value); */
		  EG_remove_link(previous);
		  ct++;
		  /*
		   * don't do previous = previous->next when removing a link
		   * otherwise will miss a link
		   */
		}
	      else
		{
		  previous = previous->next;
		  /*	      printf("ret was 0\n"); */
		}
/*	      EG_print_link(head); */
	    }

	  /* place all adjacent points in small clump array to grow the clump */
	  for (i=0; i<ct; i++)
	    {
	      /*	  printf("place[%d].index = %d, value %d\n", i, place[i].index, place[i].value); */
	      new_clump_array[place[i].index] = place[i].value;
	    }

	  /* break if can't go any further growing back */
	  if (ct == 0)
	    {
	      grow_success = 0;

	      /* restore clumps that were completely eroded */
	      previous = head;
	      while ((save = previous->next) != NULL)
		{
		  new_clump_array[save->data] = *num_clumps+1;
		  previous->next = save->next;
		  EG_delete_link(save);
		}
	      break;
	    }
	}
      printf("finished growing clumps\n");
/*      EG_print_carray_2d(new_clump_array, ydim, xdim); */
    }

  EG_free_mem(ptr_array, ptr_ct);
  return(grow_success);
}

#ifdef NOTNOW
/*
 * Check to see whether the element edm_array[index] is adjacent to an
 * element in new_clump_array.  Look at 4 neighbors.  If so return the value in
 * new_clump_array.  No bounds checking is performed as the input arrays
 * are assume to have two extra rows and columns.
 */
static int check_adj4(unsigned char *new_clump_array, int ind, int xdim)
{
  static off = 0;    /* this is used to reduce bias along a given direction */
  static int dirs[4] = {-1, 1, 0, 0};
  int i;
  int val;

  off = (off + 1) % 4;
  dirs[2] = -xdim;
  dirs[3] = xdim;

  if (new_clump_array[ind] != 0)
    return(0);

  for (i=0; i<4; i++)
    {
      val = new_clump_array[ind+dirs[(off+i)%4]];
      if (val != 0)
	return(val);
    }

  return(new_clump_array[ind+dirs[(off+7)%8]]);
#ifdef NOTNOW
  if (new_clump_array[ind+dirs[off%4]] == 0)
    {
      if (new_clump_array[ind+dirs[(off+1)%4]] == 0)
	{
	  if (new_clump_array[ind+dirs[(off+2)%4]] == 0)
	    {
	      return(new_clump_array[ind+dirs[(off+3)%4]]);
	    }
	  else
	    {
	      return(new_clump_array[ind+dirs[(off+2)%4]]);
	    }
	}
      else
	return(new_clump_array[ind+dirs[(off+1)%4]]);
    }
  else
    return(new_clump_array[ind+dirs[off]]);
#endif
}
#endif

/*
 * Check to see whether the element edm_array[index] is adjacent to an
 * element in new_clump_array.  Look at 8 neighbors.  If so return the value in
 * new_clump_array.  No bounds checking is performed as the input arrays
 * are assume to have two extra rows and columns.
 */
static int check_adj8(unsigned char *new_clump_array, int ind, int xdim)
{
  static int off = 0; /* this is used to reduce bias along a given direction */
  static int dirs[8] = {-1, 1, 0, 0, 0, 0, 0, 0};
  int i;
  int val;

  off = (off + 1) % 4;
  dirs[2] = -xdim;
  dirs[3] = xdim;
  dirs[4] = -xdim-1;
  dirs[5] = -xdim+1;
  dirs[6] = xdim-1;
  dirs[7] = xdim+1;
  

  if (new_clump_array[ind] != 0)
    return(0);

  for (i=0; i<7; i++)
    {
      val = new_clump_array[ind+dirs[(off+i)%8]];
      if (val != 0)
	return(val);
    }

  return(new_clump_array[ind+dirs[(off+7)%8]]);
}

