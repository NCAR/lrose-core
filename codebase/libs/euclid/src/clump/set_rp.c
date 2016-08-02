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
 *	set_rp.c
 *
 * PURPOSE
 *	Set the row and plane fields in an array of intervals
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Nov 13, 1992: Created.
 */

/* includes */
#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 *	set_rp - set the row and plane fields in a given array of intervals
 *
 * INPUTS:
 *	row_hdr - array of row header structures
 *	num_rows - size of array row_hdr
 *	num_planes - number of planes
 *
 * OUTPUTS:
 *	row_hdr - the row and plane fields in the intervals array are set
 *
 * RETURNS:
 *       void
 *
 * METHOD:
 */
void EG_set_rp(Row_hdr *row_hdr, int num_rows, int num_planes)
{
  int count = 0;
  int i, j, k;
  int plane_rows;
  
  /* find the number of rows in a plane */
  plane_rows = num_rows / num_planes;

  for (i=0; i<num_planes; i++)
    for (j=0; j<plane_rows; j++)
      {
	for (k=0; k<row_hdr[count].size; k++)
	  {
	    row_hdr[count].intervals[k].plane = i;
	    row_hdr[count].intervals[k].row_in_vol = j;
	  }
	count++;
      }
} /* set_rp */
