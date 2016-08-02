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
 * 	translate_array
 *
 * PURPOSE
 * 	Take a 2d or 3d array and copy it into a larger array with
 * specified offsets.
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - Jun 10, 1994: Created.
 */

/* includes */
#include <euclid/clump.h>
#include <string.h>

/*
 * DESCRIPTION:    
 * 	translate_array_2d - Suppose that a larger grid A overlaps a
 * smaller grid B.  Set the values in B according to the overlapping
 * values in A.  We assume that A is offset from B using the values xoff,
 * yoff.  Hence if the dimensions for A are xdim, ydim then the
 * dimensions for B are xdim - xoff, ydim - yoff.  This function may be
 * useful in conjunction with the function edm_2d.
 *
 * INPUTS:
 * 	in_array - input array corresponding to A above
 *      xdim - the x dimension for in_array
 *      ydim - the y dimension for in_array
 *      xoff - the x offset of A from B
 *      yoff - the y offset of A from B
 *
 * OUTPUTS:
 * 	out_array - output array corresponding to B above
 *
 * RETURNS:
 *       void
 *
 * NOTES:
 * 	
 */
void EG_translate_array_2d(unsigned char *in_array, int xdim, int ydim, int xoff, int yoff, unsigned char *out_array)
{
  int i;
  int new_xdim;
  int new_ydim;
  int nrows;
  int old_row;

  new_xdim = xdim - 2*xoff;
  new_ydim = ydim - 2*yoff;

  /* copy the overlapping elements row by row using memcpy */
  nrows = new_ydim;
/*  printf("xdim %d, ydim %d, zdim %d, new_xdim %d,  new_ydim %d,  new_zdim %d, nrows %d\n", xdim, ydim, zdim, new_xdim, new_ydim, new_zdim, nrows); */

  for (i=0; i<nrows; i++)
    {
      old_row = i + yoff;
      memcpy(&out_array[i*new_xdim], &in_array[old_row * xdim + xoff], new_xdim);
    }
}


/*
 * DESCRIPTION:    
 * 	translate_array_3d - Suppose that a larger grid A overlaps a
 * smaller grid B.  Set the values in B according to the overlapping
 * values in A.  We assume that A is offset from B using the values xoff,
 * yoff, zoff.  Hence if the dimensions for A are xdim, ydim, zdim then
 * the dimensions for B are xdim - xoff, ydim - yoff and zdim - zoff.
 * This function may be useful in conjunction with the function edm_3d.
 *
 * INPUTS:
 * 	in_array - input array corresponding to A above
 *      xdim - the x dimension for in_array
 *      ydim - the y dimension for in_array
 *      zdim - the z dimension for in_array
 *      xoff - the x offset of A from B
 *      yoff - the y offset of A from B
 *      zoff - the z offset of A from B
 *
 * OUTPUTS:
 * 	out_array - output array corresponding to B above
 *
 * RETURNS:
 *       void
 *
 * NOTES:
 * 	
 */
void EG_translate_array_3d(unsigned char *in_array, int xdim, int ydim, int zdim, int xoff, int yoff, int zoff, unsigned char *out_array)
{
  int i;
  int new_xdim;
  int new_ydim;
  int new_zdim;
  int nrows;
  int old_row;

  new_xdim = xdim - 2*xoff;
  new_ydim = ydim - 2*yoff;
  new_zdim = zdim - 2*zoff;

  /* copy the overlapping elements row by row using memcpy */
  nrows = new_ydim * new_zdim;
/*  printf("xdim %d, ydim %d, zdim %d, new_xdim %d,  new_ydim %d,  new_zdim %d, nrows %d\n", xdim, ydim, zdim, new_xdim, new_ydim, new_zdim, nrows); */

  for (i=0; i<nrows; i++)
    {
      old_row = i % new_ydim + (i/new_ydim + zoff) * ydim + yoff;
      memcpy(&out_array[i*new_xdim], &in_array[old_row * xdim + xoff], new_xdim);
    }
}


