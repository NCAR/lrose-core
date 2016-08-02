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
 * 	edm.c
 *
 * PURPOSE
 * 	Given a set of intervals representing a two or three
 * dimensional data set calculate its Euclidean distance map.  See pg 322
 * of the Image Processing Handbook by Russ.
 * 	
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - Jul 12, 1994: Created.
 */



#include <limits.h>
#include <memory.h>
#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 * 	EG_edm_2d - Given an array of intervals corresponding to a set of
 * clumps, process the intervals using the euclidean distance map
 * algorithm.  This essentially assigns to each point in the underlying
 * input array, a value corresponding to its distance to the boundary of
 * the clump.  The underlying data is assumed to be offset in the
 * following manner:
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
 * translate_array, is provided to convert the output array back to an
 * array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *
 * OUTPUTS:
 * 	array - output array, max edm = 255
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	1.  Assign 0 to background pixels, UCHAR_MAX, to pixels in
 * foreground
 * 	2.  In left to right, North to South scan
 * assign pixel in feature a value 1 greater than the smallest of its
 * neighbors
 * 	3.  Repeat 2. going from right to left, South to North scan.
 *
 * This particular function uses the taxicab metric to determine
 * distance.
 *
 * IMPORTANT NOTE
 */

void EG_edm_2d(Row_hdr *row_hdr, unsigned char *array, int xdim, int ydim, int offset)

{
  int c1;
  int c2;
  int c3;
  int i;
  int j;
  Interval *iptr;
  unsigned char *ptr;
  unsigned char *ptr1;
  unsigned char *ptr2;
  unsigned char *ptr3;
  int size;

  size = xdim * ydim;

  /* assign 0 to all pixels in array */
  memset(array, 0, size * sizeof (unsigned char));

  /* assign UCHAR_MAX to foreground pixels in array */
  EG_set_intervals_row_hdr(array, row_hdr, ydim, xdim, (unsigned char)UCHAR_MAX);

  /*
   * scan from North to South, West to East assigning value 1 greater
   * than neighbors
   */
  for (i=offset; i<=ydim-offset-1; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  ptr = &array[(i-1)*xdim + iptr->begin];

	  /* process the array elements neighboring the interval */
	  for (ptr1=ptr, ptr2 = ptr1 + xdim, ptr3 = ptr2 + xdim;
	       ptr1<=ptr+iptr->end-iptr->begin;
	       ptr1++, ptr2++, ptr3++)
	    {

	      /* find minimum neighbor */
	      c1 = MIN(*ptr1, *(ptr2-1));
	      c2 = MIN(c1, *(ptr2+1));
	      c3 = MIN(c2, *ptr3);
	      *ptr2 = MIN(c3 + 1, 255);
	      
	    }
	}
    }

  /*
   * scan from South to North, East to West, assigning
   * value 1 greater than neighbors
   */
  for (i=ydim-offset-1; i>=offset; i--)
    {
      for (j=row_hdr[i].size-1; j>=0; j--)
	{
	  iptr = &row_hdr[i].intervals[j];
	  ptr = &array[(i-1)*xdim + iptr->end];

	  /* process the array elements neighboring the interval */
	  for (ptr1=ptr, ptr2 = ptr1 + xdim, ptr3 = ptr2 + xdim;
	       ptr1>=ptr-(iptr->end-iptr->begin);
	       ptr1--, ptr2--, ptr3--)
	    {

	      /* find minimum neighbor */
	      c1 = MIN(*ptr1, *(ptr2-1));
	      c2 = MIN(c1, *(ptr2+1));
	      c3 = MIN(c2, *ptr3);
	      *ptr2 = MIN(c3 + 1, 255);
	    }
	}
    }
}

/*
 * DESCRIPTION:    
 * 	edm_2d_ext - This is an extension of EG_edm_2d.  It utilizes
 * an array of shorts to get a better estimate of the distances.
 *
 * Given an array of intervals corresponding to a set of clumps, process
 * the intervals using the euclidean distance map algorithm.  This
 * essentially assigns to each point in the underlying input array, a
 * value corresponding to its distance to the boundary of the clump.  The
 * underlying data is assumed to be offset in the following manner:
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
 * translate_array, is provided to convert the output array back to an
 * array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *
 * OUTPUTS:
 * 	array - output array, 
 *
 * RETURNS:
 * 	scale factor for output array or -1 on failure
 *
 * NOTES:
 * 	1.  Assign 0 to background pixels, USHRT_MAX, to pixels in
 * foreground
 * 	2.  In left to right, North to South scan assign pixel in
 * feature a value proportional to 1 greater than the smallest of its
 * cardinal neighbors and proportional to 1.4 greater than the smallest
 * of its neighbors in intermediate directions.
 * 	3.  Repeat 2. going from right to left, South to North scan.
 *
 * This function does not use the taxicab metric to determine distance
 * since it looks at neighbors in the intermediate directions.
 *
 * IMPORTANT NOTE
 */

int EG_edm_2d_ext(Row_hdr *row_hdr, unsigned short *array, int xdim, int ydim, int offset)
{
  int card_dist;		/* distance in cardinal direction */
  int c1;
  int c2;
  int c3;
  int c4;
  int c5;
  int c6;
  int i;
  int inter_dist;		/* distance in intermediate direction */
  int j;
  int max_dim;			/* maximum dimension */
  Interval *iptr;
  unsigned short *ptr;
  unsigned short *ptr1;
  unsigned short *ptr2;
  unsigned short *ptr3;
  int size;			/* size of input array */

  size = xdim * ydim;

  /* assign 0 to all pixels in array */
  for (i = 0; i < size; i++)
    array[i] = 0;

  max_dim = MAX(xdim, ydim);

  /*
   * Go for the best approximation to sqrt 2 given the dimensions of the
   * array.  We start at the max_dim < ~113 and work up to max_dim <
   * 65535.
   */
  if (max_dim < (65535 / 577))	/* ~113 */
    {
      card_dist = 408;
      inter_dist = 577;
    }
  else if (max_dim < (65535 / 239)) /* ~274 */
    {
      card_dist = 169;
      inter_dist = 239;
    }
  else if (max_dim < (65535 / 99)) /* ~661 */
    {
      card_dist = 70;
      inter_dist = 99;
    }
  else if (max_dim < (65535 / 41)) /* ~1598 */
    {
      card_dist = 29;
      inter_dist = 41;
    }
  else if (max_dim < (65535 / 17)) /* ~3855 */
    {
      card_dist = 12;
      inter_dist = 17;
    }
  else if (max_dim < (65535 / 7)) /* ~9362 */
    {
      card_dist = 5;
      inter_dist = 7;
    }
  else if (max_dim < (65535 / 4)) /* ~16383 */
    {
      card_dist = 3;
      inter_dist = 4;
    }
  else if (max_dim < (65535 / 3)) /* ~21845 */
    {
      card_dist = 2;
      inter_dist = 3;
    }
  else if (max_dim <= 65535)
    {
      card_dist = 1;
      inter_dist = 1;
    }
  else
    {
      /*
       * Input array has a dimension that is too big for this algorithm.
       * We avoid an extra check for overflow since we assume that most
       * 2d arrays used by this program will not have a dimension larger
       * than 65535.  One can adjust for such arrays by checking for
       * overflow as in EG_edm_2d.
       */
      return(-1);
    }

  /* assign USHRT_MAX to foreground pixels in array */
  EG_set_intervals_row_hdr_int16(array, row_hdr, ydim, xdim, USHRT_MAX);

  /* scan from North to South, West to East assigning appropriate values */
  for (i=offset; i<=ydim-offset-1; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  ptr = &array[(i-1)*xdim + iptr->begin];

	  /* process the array elements neighboring the interval */
	  for (ptr1=ptr, ptr2 = ptr1 + xdim, ptr3 = ptr2 + xdim;
	       ptr1<=ptr+iptr->end-iptr->begin;
	       ptr1++, ptr2++, ptr3++)
	    {

	      /*
	       * find minimum neighbor based on using a distance
	       * card_dist to neighbors in the cardinal directions (N, S,
	       * E, W) and inter_dist to neighbors in the intermediate
	       * directions (NW, NE, SW, SE)
	       */

	      /* calculate the minimum distance for the cardinal directions */
	      c1 = MIN(*ptr1, *(ptr2-1));
	      c2 = MIN(c1, *(ptr2+1));
	      c3 = MIN(c2, *ptr3);
/*	      printf("N %d, W %d, E %d, S %d\n", *ptr1, *(ptr2-1), *(ptr2+1), *ptr3); */

	      /*
	       * calculate the minimum distance for the intermediate
	       * directions
	       */
	      c4 = MIN(*(ptr1-1), *(ptr1+1));
	      c5 = MIN(c4, *(ptr3-1));
	      c6 = MIN(c5, *(ptr3+1));
/*	      printf("NW %d, NE %d, SW %d, SE %d\n", *(ptr1-1), *(ptr1+1), *(ptr3-1), *(ptr3+1)); */

	      /*
	       * adjust final minimum distance taking into account both
	       * cardinal and intermediate directions
	       */
	      *ptr2 = MIN(c3 + card_dist , c6 + inter_dist);
	      
/*	      printf("c3 %d, c6 %d, pos: %d, value: %d\n", c3, c6, ptr2 - array, *ptr2); */
	    }
	}
    }

  /*
   * scan from South to North, East to West, assigning
   * value 1 greater than neighbors
   */
  for (i=ydim-offset-1; i>=offset; i--)
    {
      for (j=row_hdr[i].size-1; j>=0; j--)
	{
	  iptr = &row_hdr[i].intervals[j];
	  ptr = &array[(i-1)*xdim + iptr->end];

	  /* process the array elements neighboring the interval */
	  for (ptr1=ptr, ptr2 = ptr1 + xdim, ptr3 = ptr2 + xdim;
	       ptr1>=ptr-(iptr->end-iptr->begin);
	       ptr1--, ptr2--, ptr3--)
	    {

	      /*
	       * find minimum neighbor based on using a distance
	       * card_dist to neighbors in the cardinal directions (N, S,
	       * E, W) and inter_dist to neighbors in the intermediate
	       * directions (NW, NE, SW, SE)
	       */

	      /* calculate the minimum distance for the cardinal directions */
	      c1 = MIN(*ptr1, *(ptr2-1));
	      c2 = MIN(c1, *(ptr2+1));
	      c3 = MIN(c2, *ptr3);

	      /*
	       * calculate the minimum distance for the intermediate
	       * directions
	       */
	      c4 = MIN(*(ptr1-1), *(ptr1+1));
	      c5 = MIN(c4, *(ptr3-1));
	      c6 = MIN(c5, *(ptr3+1));

	      /*
	       * adjust final minimum distance taking into account both
	       * cardinal and intermediate directions
	       */
	      *ptr2 = MIN(c3 + card_dist, c6 + inter_dist);

/*	      printf("c3 %d, c6 %d, pos: %d, value: %d\n", c3, c6, ptr2 - array, *ptr2); */
	    }
	}
    }

  return(card_dist);
}

/*
 * DESCRIPTION:
 * EG_simple_edm_2d - This has the same purpose as
 * EG_edm_2d. The difference is that instead of using intervals to
 * indicate the image shapes, it uses a template array.
 *
 * This essentially assigns to each point in the underlying input array,
 * a value corresponding to its distance to the boundary of the clump.
 * The underlying data is assumed to be offset in the following manner:
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
 * translate_array, is provided to convert the output array back to an
 * array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	unsigned char *template_grid - input array, must have vals set 
 *             non-zero for shapes, 0 elsewhere.
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *
 * OUTPUTS:
 * 	unsigned char *edm_grid - output array, max edm = 255
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	1.  Assign 0 to background pixels, UCHAR_MAX, to pixels in
 * foreground
 * 	2.  In left to right, North to South scan
 * assign pixel in feature a value 1 greater than the smallest of its
 * neighbors
 * 	3.  Repeat 2. going from right to left, South to North scan.
 *
 * This particular function uses the taxicab metric to determine
 * distance.
 *
 * Mike Dixon, from code by Gerry Wiener
 */

void EG_simple_edm_2d(unsigned char *template_grid, unsigned char *edm_grid,
		      int xdim, int ydim)

{

  int c1;
  int c2;
  int c3;
  int ix, iy;
  unsigned char *tptr;
  unsigned char *eptr;
  int npoints_plane;

  npoints_plane = xdim * ydim;

  /*
   * assign 0 to all pixels in edm_grid 
   */
  
  memset(edm_grid, 0, npoints_plane * sizeof (unsigned char));

  /*
   * assign UCHAR_MAX to foreground pixels in edm_grid,
   * not including edge
   */

  tptr = template_grid + xdim + 1;
  eptr = edm_grid + xdim + 1;
  
  for (iy = 1; iy < ydim - 1; iy++, tptr += 2, eptr += 2) {
    for (ix = 1; ix < xdim - 1; ix++, tptr++, eptr++) {
      if (*tptr) {
	*eptr = UCHAR_MAX;
      }
    } /* ix */
  } /* iy */

  /*
   * scan from South to North, West to East assigning value 1 greater
   * than neighbors
   */

  eptr = edm_grid + xdim + 1;
  
  for (iy = 1; iy < ydim - 1; iy++, eptr += 2) {
    for (ix = 1; ix < xdim - 1; ix++, eptr++) {
      if (*eptr) {
	/* find minimum neighbor */
	c1 = MIN(*(eptr-1), *(eptr+1));
	c2 = MIN(*(eptr-xdim), *(eptr+xdim));
	c3 = MIN(c1, c2) + 1;
	*eptr = MIN(c3, UCHAR_MAX);
      } /* if (eptr) */
    } /* ix */
  } /* iy */
  
  /*
   * scan from North to South, East to West, assigning value 1 greater
   * than neighbors
   */

  eptr = edm_grid + npoints_plane - 2;
  
  for (iy = ydim - 2; iy > 0; iy--, eptr -= 2) {
    for (ix = xdim - 2; ix > 0; ix--, eptr--) {
      if (*eptr) {
	/* find minimum neighbor */
	c1 = MIN(*(eptr-1), *(eptr+1));
	c2 = MIN(*(eptr-xdim), *(eptr+xdim));
	c3 = MIN(c1, c2) + 1;
	*eptr = MIN(c3, UCHAR_MAX);
      } /* if (eptr) */
    } /* ix */
  } /* iy */
  
}

/*
 * DESCRIPTION:    
 * EG_inverse_edm_2d.
 * This is similar to EG_simple_edm_2d, except that it operates in the
 * inverse sense. The euclidean distances are the distance outside
 * the shape, from the closest shape border.
 *
 * This essentially assigns to each point in the underlying
 * input array, a value corresponding to its distance to the boundary of
 * the clump.  The underlying data is assumed to be offset in the
 * following manner:
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
 * translate_array, is provided to convert the output array back to an
 * array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	unsigned char *template_grid - input array, must have vals set 
 *             non-zero for shapes, 0 elsewhere.
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *
 * OUTPUTS:
 * 	unsigned char *edm_grid - output array, max edm = 255
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	1.  Assign 0 to background pixels, UCHAR_MAX, to pixels in
 * foreground
 * 	2.  In left to right, North to South scan
 * assign pixel in feature a value 1 greater than the smallest of its
 * neighbors
 * 	3.  Repeat 2. going from right to left, South to North scan.
 *
 * This particular function uses the taxicab metric to determine
 * distance.
 *
 * Mike Dixon, from code by Gerry Wiener
 */

void EG_inverse_edm_2d(unsigned char *template_grid, unsigned char *edm_grid,
		       int xdim, int ydim)

{

  int c1;
  int c2;
  int c3;
  int ix, iy;
  unsigned char *tptr;
  unsigned char *eptr;
  int npoints_plane;

  npoints_plane = xdim * ydim;

  /*
   * assign 0 to all pixels in edm_grid 
   */
  
  memset(edm_grid, 0, npoints_plane * sizeof (unsigned char));

  /*
   * assign UCHAR_MAX to background pixels in edm_grid,
   * including edge
   */

  tptr = template_grid;
  eptr = edm_grid;
  
  for (iy = 0; iy < ydim; iy++) {
    for (ix = 0; ix < xdim; ix++, tptr++, eptr++) {
      if (!*tptr) {
	*eptr = UCHAR_MAX;
      }
    } /* ix */
  } /* iy */

  /*
   * scan from South to North, West to East assigning value 1 greater
   * than neighbors
   */

  eptr = edm_grid + xdim + 1;
  
  for (iy = 1; iy < ydim - 1; iy++, eptr += 2) {
    for (ix = 1; ix < xdim - 1; ix++, eptr++) {
      if (*eptr) {
	/* find minimum neighbor */
	c1 = MIN(*(eptr-1), *(eptr+1));
	c2 = MIN(*(eptr-xdim), *(eptr+xdim));
	c3 = MIN(c1, c2) + 1;
	*eptr = MIN(c3, UCHAR_MAX);
      } /* if (eptr) */
    } /* ix */
  } /* iy */
  
  /*
   * scan from North to South, East to West, assigning value 1 greater
   * than neighbors
   */

  eptr = edm_grid + npoints_plane -xdim - 2;
  
  for (iy = ydim - 2; iy > 0; iy--, eptr -= 2) {
    for (ix = xdim - 2; ix > 0; ix--, eptr--) {
      if (*eptr) {
	/* find minimum neighbor */
	c1 = MIN(*(eptr-1), *(eptr+1));
	c2 = MIN(*(eptr-xdim), *(eptr+xdim));
	c3 = MIN(c1, c2) + 1;
	*eptr = MIN(c3, UCHAR_MAX);
      } /* if (eptr) */
    } /* ix */
  } /* iy */

}

/*
 * DESCRIPTION:    
 * 	edm_3d - Given an array of intervals corresponding to a set of
 * clumps, process the intervals using the euclidean distance map
 * algorithm.  This essentially assigns to each point in the underlying
 * input array, a value corresponding to its distance to the boundary of
 * the clump.  The underlying data is assumed to be offset in the
 * following manner:
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
 * translate_array, is provided to convert the output array back to an
 * array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 * 	int zdim - z dimension of input data set
 *
 * OUTPUTS:
 * 	array - output array, max edm = 255
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	1.  Assign 0 to background pixels, UCHAR_MAX, to pixels in
 * foreground
 * 	2.  In left to right, North to South, bottom to top scan
 * assign pixel in feature a value 1 greater than the smallest of its
 * neighbors
 * 	3.  Repeat 2. going from right to left, South to North, top to
 * bottom scan.
 *
 * This particular function uses the taxicab metric to determine
 * distance.
 *
 * IMPORTANT NOTE
 */
void EG_edm_3d(Row_hdr *row_hdr, unsigned char *array, int xdim, int ydim, int zdim, int offset)
{
  int c1;
  int c2;
  int c3;
  int c4;
  int c5;
  int i;
  int j;
  int k;
  Interval *iptr;
  int plane_size;
  unsigned char *ptr;
  unsigned char *ptr1;
  unsigned char *ptr2;
  unsigned char *ptr3;
  unsigned char *ptrb;
  unsigned char *ptru;
  int size;

  size = xdim * ydim * zdim;
  plane_size = xdim * ydim;

  /* assign 0 to all pixels in array */
  (void)memset(array, 0, size * sizeof (unsigned char));

  /* assign UCHAR_MAX to foreground pixels in array */
  EG_set_intervals_row_hdr(array, row_hdr, ydim*zdim, xdim, UCHAR_MAX);

  /*
   * Scan from North to South and West to East and bottom to top
   * assigning value 1 greater than neighbors.  If the planes in volume
   * are numbered from 0 to zdim-1, we need to iterate through rows on
   * plane offset through plane zdim-offset-1.  The rows in each plane
   * need to begin at offset and go through ydim - offset - 1.
   */
  for (k=offset; k<=zdim-offset-1; k++)
    {
      for (i=k * ydim + offset; i<=(k+1) * ydim - offset - 1; i++)
	{
	  for (j=0; j<row_hdr[i].size; j++)
	    {
	      iptr = &row_hdr[i].intervals[j];
	      ptr = &array[(i-1)*xdim + iptr->begin];
	      

	      /* process the array elements neighboring the interval */
	      for (ptr1=ptr, ptr2 = ptr1 + xdim,
		   ptr3 = ptr2 + xdim, ptrb = ptr2 - plane_size,
		   ptru = ptr2 + plane_size;
		   ptr1<=ptr+iptr->end-iptr->begin;
		   ptr1++, ptr2++, ptr3++, ptrb++, ptru++)
		{

		  /* find minimum neighbor */
		  c1 = MIN(*ptr1, *(ptr2-1));
		  c2 = MIN(c1, *(ptr2+1));
		  c3 = MIN(c2, *ptr3);
		  c4 = MIN(c3, *ptrb);
		  c5 = MIN(c4, *ptru);
		  *ptr2 = MIN(c5 + 1, 255);
		}
	    }
	}
    }
  
  /*
   * scan from South to North and East to West top to bottom assigning
   * value 1 greater than neighbors
   */
  for (k=zdim-offset-1; k>=offset; k--)
    {
      for (i=(k+1) * ydim - offset - 1; i>=k * ydim + offset; i--)
	{
	  for (j=row_hdr[i].size-1; j>=0; j--)
	    {
	      iptr = &row_hdr[i].intervals[j];
	      ptr = &array[(i-1)*xdim + iptr->end];
	      
	      /* process the array elements neighboring the interval */
	      for (ptr1=ptr, ptr2 = ptr1 + xdim,
		   ptr3 = ptr2 + xdim, ptrb = ptr2 - plane_size,
		   ptru = ptr2 + plane_size;
		   ptr1>=ptr-(iptr->end-iptr->begin);
		   ptr1--, ptr2--, ptr3--, ptrb--, ptru--)
		{
		  
		  /* find minimum neighbor */
		  c1 = MIN(*ptr1, *(ptr2-1));
		  c2 = MIN(c1, *(ptr2+1));
		  c3 = MIN(c2, *ptr3);
		  c4 = MIN(c3, *ptrb);
		  c5 = MIN(c4, *ptru);
		  *ptr2 = MIN(c5 + 1, 255);
		}
	    }
	}
    }
}

