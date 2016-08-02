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
 *	link_intervals.c
 *
 * PURPOSE
 *	link intervals into rows for sorting
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Jan 26, 1993: Created.
 */

#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 * 	link_intervals
 *
 * INPUTS:
 * 	int_array - array of intervals
 * 	int_array_size - dimension of int_array
 * 	num_rows - number of rows
 *
 * OUTPUTS:
 * 	link_array - array of ptrs to linked lists of intervals for each row
 * 			    (has dimension num_rows)
 * 	links - members of link_array containing interval ptrs and links
 * 		   (has dimension int_array_size)
 * RETURNS:
 *  	void     
 *
 * METHOD:
 * 	For each interval in int_array, determine its row, then link
 * it to the appropriate row.  This has the effect of sorting the
 * intervals in int_array by row.
 *
 * row1 -> link -> link -> 0
 * row2 -> link -> link -> 0
 * row3 -> link -> link -> 0
 * etc.
 */
void
EG_link_intervals(Interval *int_array, int int_array_size, int num_rows, Interval_link_hdr *link_array, Interval_link *links)
{
  int i;
  int row;

  printf("link before, num_rows %d\n", num_rows);
/*  EG_print_intervals(int_array, int_array_size); */

  /* initialize link array */
  for (i=0; i<num_rows; i++)
    {
      link_array[i].link = (Interval_link *)0;
      link_array[i].size = 0;
    }
  printf("link after\n");
/*  EG_print_intervals(int_array, int_array_size); */

  printf("int_array_size %d\n", int_array_size);
  for (i=0; i<int_array_size; i++)
    {
      links[i].iptr = &int_array[i];
/*      EG_print_interval(&int_array[i]);
      putchar('\n');*/
      row = int_array[i].row_in_vol;
/*      printf("row is %d, i is %d\n\n", row, i);*/
      links[i].link = link_array[row].link;
      link_array[row].link = &links[i];
      link_array[row].size++;
    }
}

/*
 * DESCRIPTION:    
 * 	link_pintervals - This function differs from link_intervals
 * only in terms of the input array pint_array which is an array of
 * pointers to intervals as opposed to an array of intervals
 *
 * INPUTS:
 * 	pint_array - array of pointers to intervals
 * 	int_array_size - dimension of int_array
 * 	num_rows - number of rows
 *
 * OUTPUTS:
 * 	link_array - array of ptrs to linked lists of intervals for each row
 * 			    (has dimension num_rows)
 * 	links - members of link_array containing interval ptrs and links
 * 		   (has dimension int_array_size)
 * RETURNS:
 *  	void     
 *
 * METHOD:
 * 	For each interval in int_array, determine its row, then link
 * it to the appropriate row.  This has the effect of sorting the
 * intervals in int_array by row.
 *
 * row1 -> link -> link -> 0
 * row2 -> link -> link -> 0
 * row3 -> link -> link -> 0
 * etc.
 */
void
EG_link_pintervals(Interval **pint_array, int int_array_size, int num_rows, Interval_link_hdr *link_array, Interval_link *links)
{
  int i;
  int row;

  /* initialize link array */
  for (i=0; i<num_rows; i++)
    {
      link_array[i].link = (Interval_link *)0;
      link_array[i].size = 0;
    }

  for (i=0; i<int_array_size; i++)
    {
      links[i].iptr = pint_array[i];
      row = pint_array[i]->row_in_vol;
      links[i].link = link_array[row].link;
      link_array[row].link = &links[i];
      link_array[row].size++;
    }
}

/*
 * DESCRIPTION:    
 * 	dump_links
 *
 * INPUTS:
 * 	link_array - array of linked list headers for each row
 * 	num_rows - dimension of link_array
 *
 * OUTPUTS:
 * 	int_array - place to store dumped interval pointers (must be
 * large enough to hold all intervals that are linked - this size is
 * usually known when allocating the links array)
 *
 * RETURNS:
 *       the number of links dumped
 *
 * METHOD:
 * 	Dumps the linked lists contained in link_array to
 * int_array.  
 */
int
EG_dump_links(Interval_link_hdr *link_array, int num_rows, Interval **int_array)
{
  int ct;
  int i;
  int j;
  Interval_link *link_ptr;

  ct = 0;
  for (i=0; i<num_rows; i++)
    {
      link_ptr = link_array[i].link;
      for (j=0; j<link_array[i].size; j++)
	{
	  int_array[ct] = link_ptr->iptr;
	  link_ptr = link_ptr->link;
	  ct++;
	}
    }
  return(ct);
}

/*
 * DESCRIPTION:    
 *	print_links - prints information contained in linked list of intervals
 *
 * INPUTS:
 * 	link_array - array of linked list headers for each row
 *	num_rows - dimension of link_array
 *
 * OUTPUTS:
 *	none
 *
 * RETURNS:
 *       void
 *
 * METHOD:
 *	
 */
void
EG_print_links(Interval_link_hdr *link_array, int num_rows)
{
  int i;
  int j;
  /* Interval *iptr; */
  Interval_link *link_ptr;

  for (i=0; i<num_rows; i++)
    {
      link_ptr = link_array[i].link;
      if (link_array[i].size == 0)
	continue;

      printf("row: %d, size: %d\n", i, link_array[i].size);
      for (j=0; j<link_array[i].size; j++)
	{
	  /* iptr = link_ptr->iptr; */
	  link_ptr = link_ptr->link;
	}
    }
}
