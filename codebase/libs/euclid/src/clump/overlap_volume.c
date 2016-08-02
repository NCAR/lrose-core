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
/* overlap_volume.c - find all row interval overlaps in a volume */

#include <stdio.h>
#include <sys/types.h>

#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 *	overlap_volume
 *
 * INPUTS:
 *	zdim - number of planes in volume
 *	ydim - number of rows in each plane
 *	row_hdr - array of row header information for each row
 *	(has dimension zdim * ydim)
 *
 * OUTPUTS:
 *	row_hdr - overlap field in intervals array for each row header
 *	struct is set
 *
 * RETURNS:
 *  	void     
 *
 * METHOD:
 *	
 */
void
EG_overlap_volume(int zdim, int ydim, Row_hdr *row_hdr, int min_overlap)
{
  int base;
  int i;
  int j;
  int rows;

  /* adjust DOWN_INTERVAL for bottom plane (no intervals below) */
  for (i=0; i<ydim; i++)
    for (j=0; j<row_hdr[i].size; j++)
      {
	row_hdr[i].intervals[j].overlaps[DOWN_INTERVAL][0] = 1;
	row_hdr[i].intervals[j].overlaps[DOWN_INTERVAL][1] = 0;
      }

  /* adjust UP_INTERVAL for top plane (no intervals above) */
  for (i=(zdim-1)*ydim; i<zdim*ydim; i++)
    for (j=0; j<row_hdr[i].size; j++)
      {
	row_hdr[i].intervals[j].overlaps[UP_INTERVAL][0] = 1;
	row_hdr[i].intervals[j].overlaps[UP_INTERVAL][1] = 0;
      }


  for (i=0; i<zdim; i++)
    {
      rows = i * ydim;

      /* initialize overlaps for top row of each plane */
      for (j=0; j<row_hdr[rows].size; j++)
	{
	  row_hdr[rows].intervals[j].overlaps[NORTH_INTERVAL][0] = 1;
	  row_hdr[rows].intervals[j].overlaps[NORTH_INTERVAL][1] = 0;
	}
      
      /* initialize overlaps for bottom row for each plane*/
      for (j=0; j<row_hdr[rows+ydim-1].size; j++)
	{
	  row_hdr[rows+ydim-1].intervals[j].overlaps[SOUTH_INTERVAL][0] = 1;
	  row_hdr[rows+ydim-1].intervals[j].overlaps[SOUTH_INTERVAL][1] = 0;
	}
      
      for (j=0; j<ydim; j++)
	{
	  base = rows + j;
	  if (j + 1 < ydim)
	    {
	      /* overlap rows in the horizontal */
	      EG_overlap_rows(&(row_hdr[base]), &(row_hdr[base+1]),
			     SOUTH_INTERVAL, min_overlap);
	      EG_overlap_rows(&(row_hdr[base+1]), &(row_hdr[base]),
			     NORTH_INTERVAL, min_overlap);
	    }
	  if (i + 1 < zdim)
	    {
	      /* overlap rows in the vertical */
	      EG_overlap_rows(&(row_hdr[base]), &(row_hdr[base+ydim]),
			     UP_INTERVAL, min_overlap);
	      EG_overlap_rows(&(row_hdr[base+ydim]), &(row_hdr[base]),
			     DOWN_INTERVAL, min_overlap);
	    }
	}
    }
}
