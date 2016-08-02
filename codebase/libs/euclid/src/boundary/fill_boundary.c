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
 * 	fill_boundary.c
 *
 * PURPOSE
 * 	Given a boundary consisting of a finite number of horizontal
 * and vertical line segments, find the intervals that fill the boundary
 *
 * NOTES
 * 	The segments in the boundary must be HORIZONTAL and VERTICAL.
 *
 * HISTORY
 *     wiener - Nov 30, 1992: Created.
 */


/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <euclid/boundary.h>
#include <euclid/clump.h>
#include <euclid/node.h>

/* defines */


typedef struct column Column;

struct column
{
  int col;
  Column *ptr;
};

typedef struct
{
  int size;
  Column *ptr;
} Row;
  
/* file scope function declarations */
static void insertion(void *ptr[], size_t nel, int (*compar)(void *, void *));
static int compare(void *p, void *q);

/*
 * DESCRIPTION:    
 * 	fill_boundary takes a boundary list and an array of nodes and
 * then generates an of intervals corresponding to the interior of the boundary
 *
 * INPUTS:
 * 	bdry_list - list of indices of sequential nodes in the boundary
 *      bdry_size - size of bdry_list
 *      node - array of nodes both on and inside the boundary
 *      num_rows - number of rows in row_hdr
 * 	intervals - the address of a pointer for storing the output interval
 *         array.  
 *
 * OUTPUTS:
 * 	intervals - an array of intervals in the filled boundary.   Note
 *         that the interval array is allocated by this routine.
 *
 * RETURNS:
 *        the number of intervals in the filled boundary
 *
 * METHOD:
 * 	Sort the boundary nodes into buckets according to rows.  Copy
 * nodes in each row to an array.  Sort the nodes in each array according
 * to x coordinate.  Generate intervals.
 */
int EG_fill_boundary(int *bdry_list, int bdry_size, Node *node, Interval **interval, int num_rows)
{
  Column *column;
  int col_size = 0;
  Column *cptr;
  int count = 0;
  int i;
  Interval *intervals;
  int j;
  int k = 0;
  int max_size = 0;
  Node *np;
  Node *np1;
  Node *np_tmp;
  Row *row;
  Column **sort_array;

  /* determine the number of column structures to allocate */
  for (i=0; i<bdry_size-1; i++)
    {
      np = &node[bdry_list[i]];
      np1 = &node[bdry_list[i+1]];
      col_size += fabs(np->y - np1->y);
    }
  

  /* allocate an array of columns */
  column = (Column *)EG_calloc(col_size, sizeof(Column));
  if (column == NULL)
    return(-1);
  
  /* allocate an array of rows */
  row = (Row *)EG_calloc(num_rows, sizeof(Row));
  if (row == NULL)
    return(-1);

  /*
   * Iterate through the line segments in the boundary.  For each row in
   * each vertical boundary segment, assign the appropriate column for
   * the segment to the row structure.  After the iteration, the columns
   * are sorted in order to determine the intervals for each row.
   */
  for (i=0; i<bdry_size-1; i++)
    {
      np = &node[bdry_list[i]];
      np1 = &node[bdry_list[i+1]];

      /* if the two boundary nodes determine a horizontal segment continue */
      if (np->y == np1->y)
	continue;

      if (np->y > np1->y)
	{
	  /* swap them to run the following loop from bottom to top */
	  np_tmp = np;
	  np = np1;
	  np1 = np_tmp;
	}

      /*
       * Set the columns for each row.  This is done by forming a linked
       * list since the number of columns per row is not known in
       * advance.  Later each linked list will be copied to an array for
       * sorting.
       */
      for (j=np->y+OFFSET_Y; j<np1->y; j++)
	{
	  column[k].col = np->near_x; 
	  if (row[j].ptr == NULL)
	    {
	      row[j].ptr = &column[k];
	      row[j].size++;
	      column[k].ptr = NULL; 
	    }
	  else
	    {
	      column[k].ptr = row[j].ptr; 
	      row[j].ptr = &column[k]; 
	      row[j].size++;
	    }
	  k++;
	}
	
      /* find the maximum size for the number of columns in any given row */
      if (row[np->row].size > max_size)
	max_size = row[np->row].size;
    }

#ifdef DEBUG
  for (i=0; i<num_rows; i++)
    {
      (void) printf("row[%d].size = %d\n", i, row[i].size);
    } 
#endif

  /*
   * allocate an array big enough to contain the maximum number of
   * columns in any given row
   */
  sort_array = (Column **)EG_calloc(max_size, sizeof(Column *));
  if (row == NULL)
    return(-1);
  
  /*
   * Allocate an array for storing the intervals.  Note that k is the
   * total number of columns allocated for all rows.  This means that k/2
   * should be the total number of intervals that need to be allocated.
   */
  intervals = (Interval *)EG_calloc(k/2, sizeof(Interval));

  /* sort the columns in each row */
  for (i=0; i<num_rows; i++)
    {
      if (row[i].size > 0)
	{
	  /*
	   * copy the column information in the row to sort_array for
	   * sorting
	   */
	  cptr = row[i].ptr;
	  for (j=0; j<row[i].size; j++)
	    {
	      sort_array[j] = cptr;
/*	      printf("cptr->ptr %d, col %d, sort_array[%d] %d\n", cptr->ptr, cptr->col, j, sort_array[j]->col); */
	      cptr = cptr->ptr;
	    }
	  
	  /* sort the sort array using an insertion sort */
	  insertion((void **)sort_array, (size_t) row[i].size, compare);

#ifdef DEBUG
	  for (j=0; j<row[i].size; j++)
	    {
	      printf("sort_array[%d] %d\n",  j, sort_array[j]->col);
	    }
#endif
	  /* generate the intervals corresponding to each row */
	  for (j=0; j<row[i].size; j+=2)
	    {
	      intervals[count].row_in_vol = i;
	      intervals[count].begin = sort_array[j]->col;
	      intervals[count].end = sort_array[j+1]->col;
/*	      printf("intervals[%d], row %d, begin %d, end %d\n", count, intervals[count].row, intervals[count].begin, intervals[count].end); */
	      count++;
	    }
	}
    }

  *interval = intervals;

  /* free up allocated space */
  (void)EG_free(column);
  (void)EG_free(row);
  (void)EG_free(sort_array);
  return(count);
}


static void insertion(void *ptr[], size_t nel, int (*compar)(void *, void *))
{
  int i;
  int j;
  void *v;

  for (i=1; i<nel; i++)
    {
      v = ptr[i];
      j = i;

      while (j > 0 && (*compar)(ptr[j-1], v) > 0)
	{
	  ptr[j] = ptr[j-1];
	  j--;
	}
      ptr[j] = v;
    }
}

/* comparison function used for sorting */
static int compare(void *p, void *q)
{
  return(((Column *)p)->col - ((Column *)q)->col);
}

