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
/* overlap_plane.c - find all row interval overlaps in a plane */

#include <stdio.h>
#include <sys/types.h>

#include <euclid/clump.h>

void EG_overlap_plane(int ydim, Row_hdr *row_hdr, int min_overlap)
{
  int j;
  
  /* initial overlaps for top and bottom rows */
  for (j=0; j<row_hdr[0].size; j++)
    {
      row_hdr[0].intervals[j].overlaps[NORTH_INTERVAL][0] = 1;
      row_hdr[0].intervals[j].overlaps[NORTH_INTERVAL][1] = 0;
    }
      
  for (j=0; j<row_hdr[ydim-1].size; j++)
    {
      row_hdr[ydim-1].intervals[j].overlaps[SOUTH_INTERVAL][0] = 1;
      row_hdr[ydim-1].intervals[j].overlaps[SOUTH_INTERVAL][1] = 0;
    }
      
  /* find overlaps for remaining rows */
  for (j=0; j<ydim-1; j++)
    {
      EG_overlap_rows(&(row_hdr[j]), &(row_hdr[j+1]),
		     SOUTH_INTERVAL, min_overlap);
      EG_overlap_rows(&row_hdr[j+1], &row_hdr[j],
		     NORTH_INTERVAL, min_overlap);
    }
}
