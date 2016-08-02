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
OEG_print_clump_intervals(OClump_order *clump_order, int num_clumps)
{
  int i;
  int j;

  for (i=1; i<num_clumps+1; i++)
    {
      printf("Clump %d:\n", i);
      printf("\tnumber of intervals %d:\n", clump_order[i].size);
      printf("\tnumber of pts %d:\n", clump_order[i].pts);
      printf("\txmin %d:\n", clump_order[i].xmin);
      printf("\txmax %d:\n", clump_order[i].xmax);
      printf("\tymin %d:\n", clump_order[i].ymin);
      printf("\tymax %d:\n", clump_order[i].ymax);
      printf("\tzmin %d:\n", clump_order[i].zmin);
      printf("\tzmax %d:\n", clump_order[i].zmax);
      printf("\tmagnitude %f:\n", clump_order[i].mag);
      for (j=0; j<clump_order[i].size; j++)
	{
	  EG_print_interval(clump_order[i].ptr[j]);
	  printf("\n");
	}

    }

}

void OEG_print_clump_info(OClump_info *ci)
{
  printf("Clump info:\n");
  printf("num_planes: %d\n", ci->num_planes);
  printf("num_rows in plane: %d\n", ci->num_rows/ci->num_planes);
  printf("num_cols: %d\n", ci->num_cols);
  printf("total num_rows: %d\n", ci->num_rows);
  printf("threshold: %d\n", ci->threshold);
  printf("num_ints: %d\n", ci->num_ints);
  printf("num_clumps: %d\n", ci->num_clumps);
  printf("isize: %d\n", ci->isize);
  printf("intervals:\n");
  OEG_print_clump_intervals(ci->clump_order, ci->num_clumps);  
}



