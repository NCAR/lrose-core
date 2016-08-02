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
 * print_intervals.c - print intervals
 */

#include <stdio.h>
#include <sys/types.h>
#include <euclid/clump.h>

/*
 *
 * INPUTS:
 *
 * interval_order - array of pointers to intervals ordered by clump
 *
 * clump_order - array of structures per clump containing information as
 * to the intervals in each clump.  The clump_order structure contains
 * the number of intervals in the clump as well as a pointer into the
 * interval_order array.
 *
 * num_clumps - size of clump_order array
 *
 * RETURNS:
 *
 * void
 */
void
EG_print_clump_intervals(Interval **interval_order, Clump_order *clump_order, int num_clumps)
{
  int i;
  int j;

  for (i=1; i<num_clumps+1; i++)
    {
      printf("clump %d:\n", i);
      printf("  number of intervals %d:\n", clump_order[i].size);
      printf("  number of pts %d:\n", clump_order[i].pts);
      for (j=0; j<clump_order[i].size; j++)
	{
	  EG_print_interval(clump_order[i].ptr[j]);
	}

    }

}

void
EG_print_interval(Interval *iptr)
{
  printf("	id %d\n", iptr->id);
  printf("	plane %d\n", iptr->plane);
  printf("	row %d\n", iptr->row_in_vol);
  printf("	begin %d\n", iptr->begin);
  printf("	end %d\n", iptr->end);
}

/*
 *
 * INPUTS:
 *
 * intervals 	- array of intervals
 * num_ints	- dimension of intervals
 *
 * RETURNS:
 *
 * void
 */
void EG_print_pintervals(Interval **pintervals, int num_ints)
{
  int i;
  Interval *iptr;

  for (i=0; i<num_ints; i++)
      {
	iptr = pintervals[i];
	printf("\ninterval[%d]: \n", i);
	EG_print_interval(iptr);
      }
}

/*
 *
 * INPUTS:
 *
 * intervals 	- array of intervals
 * num_ints	- dimension of intervals
 *
 * RETURNS:
 *
 * void
 */
void EG_print_intervals(Interval *intervals, int num_ints)
{
  int i;
  Interval *iptr;

  for (i=0; i<num_ints; i++)
      {
	iptr = &intervals[i];
	printf("\ninterval[%d]: \n", i);
	EG_print_interval(iptr);
      }
}

/*
 * Print out relevant interval information in the array interval_row_cb.
 *
 * INPUTS:
 *
 * row_hdr - array of row_hdr structures
 * num_rows - the number of elements in intervals
 *
 * RETURNS:
 *
 * void
 */
void EG_print_row_hdr(Row_hdr *row_hdr, int num_rows)
{
  int i;
  int j;
  Interval *iptr;

  for (i=0; i<num_rows; i++)
      {
	if (row_hdr[i].size > 0)
	  {
	    printf("\nrow_hdr[%d].size %d\n", i, row_hdr[i].size);
	    for (j=0; j<row_hdr[i].size; j++)
	      {
		iptr = &row_hdr[i].intervals[j];
		printf("interval[%d]: \n", j);
		EG_print_interval(iptr);
	      }
	  }
      }
}


