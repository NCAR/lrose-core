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
 * 	extend_int
 *
 * PURPOSE
 * 	generate a set of intervals from given intervals and
 * extension information
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - Jan 29, 1993: Created.
 */

#include <stdio.h>
#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 * 	extend_int_2d - extend each interval in a given array of
 * intervals.  This function reallocates space if necessary.
 *
 * INPUTS:
 * 	interval - array of input intervals
 *      in_size - size of interval array
 *      x - displacement in x direction
 *      y - displacement in y direction
 *      box - bounding box for extension
 *
 * OUTPUTS:
 * 	pout_interval - pointer to array of output intervals
 *      (realloc'd if necessary)
 *      pout_size - size of pout_interval
 *      
 * RETURNS:
 *       the number of intervals in pout_interval
 *
 * NOTES:
 * 	This function extends intervals in both positive and negative
 * x and y directions.  It does not do the extension beyond the limits of
 * the bounding box.  Note that xmin, ymin should not be smaller than 0
 * and xmax, ymax should not be larger than the maximum x and y values
 * for the underlying grid for the intervals.
 */
int EG_extend_int_2d(Interval *interval, int in_size, int x, int y, Interval **pout_interval, int *pout_size, Box_2d *box)
{
  int min_size;
  int ymin;
  int ct;
  int i;
  int j;
  int xmin;
  int xmax;
  int row;
  Interval *out_interval;
  int out_size;
  int ymax;

  out_interval = *pout_interval;
  out_size = *pout_size;

  /* reallocate output array if necessary */
  min_size = in_size * (2 * y + 1);
  if (out_size < min_size)
    {
      out_interval = (Interval *)EG_realloc(out_interval, min_size * sizeof(Interval));
      if (out_interval == NULL)
	return(-1);
      
      out_size = min_size;
    }

  ct = 0;
  for (i=0; i<in_size; i++)
    {
      row = interval[i].row_in_vol;

      /* clip to bounding box */
      ymin = MAX(row-y, box->ymin);
      ymax = MIN(row+y, box->ymax);
      xmin = MAX(interval[i].begin - x, box->xmin);
      xmax = MIN(interval[i].end + x, box->xmax);


      for (j=ymin; j<=ymax; j++)
	{
	  out_interval[ct].row_in_vol = j;
	  out_interval[ct].begin = xmin;
	  out_interval[ct].end = xmax;
	  ct++;
	}
    }

  *pout_interval = out_interval;
  *pout_size =   out_size;

  return(ct);
}

int EG_extend_pint_2d(Interval **interval, int in_size, int x, int y, Interval **pout_interval, int *pout_size, Box_2d *box)
{
  int ymin;
  int ct;
  int i;
  int j;
  int min_size;
  int xmin;
  int xmax;
  int row;
  Interval *out_interval;
  int out_size;
  int ymax;

  out_interval = *pout_interval;
  out_size = *pout_size;

  /* reallocate output array if necessary */
  min_size = in_size * (2 * y + 1);
  if (out_size < min_size)
    {
      printf("out_size is %d, min_size %d\n", out_size, min_size);
      out_interval = (Interval *)EG_realloc(out_interval, min_size * sizeof(Interval));
      if (out_interval == NULL)
	return(-1);
      
      out_size = min_size;
    }

  ct = 0;
  for (i=0; i<in_size; i++)
    {
      row = interval[i]->row_in_vol;

      /* clip to bounding box */
      ymin = MAX(row-y, box->ymin);
      ymax = MIN(row+y, box->ymax);
      xmin = MAX(interval[i]->begin - x, box->xmin);
      xmax = MIN(interval[i]->end + x, box->xmax);


      for (j=ymin; j<=ymax; j++)
	{
	  out_interval[ct].row_in_vol = j;
	  out_interval[ct].begin = xmin;
	  out_interval[ct].end = xmax;
	  ct++;
	}
    }

  *pout_interval = out_interval;
  *pout_size =   out_size;

    return(ct);
}

int EG_extend_int_3d(Interval *interval, int in_size, int x, int y, int z, Interval **pout_interval, int *pout_size, Box_3d *box)
{
  int ymin;
  int ct;
  int i;
  int j;
  int k;
  int min_size;
  Interval *out_interval;
  int out_size;
  int plane;
  int xmin;
  int xmax;
  int row;
  int ymax;
  int zmax;
  int zmin;

  out_interval = *pout_interval;
  out_size = *pout_size;

  /* reallocate output array if necessary */
  min_size = in_size * (2 * y + 1) * (2 * z + 1);
  if (out_size < min_size)
    {
      out_interval = (Interval *)EG_realloc(out_interval, min_size * sizeof(Interval));
      if (out_interval == NULL)
	return(-1);
      
      out_size = min_size;
    }

  ct = 0;
  for (i=0; i<in_size; i++)
    {
      row = interval[i].row_in_vol;
      plane = interval[i].plane;

      /* clip to bounding box */
      xmin = MAX(interval[i].begin - x, box->xmin);
      xmax = MIN(interval[i].end + x, box->xmax);
      ymin = MAX(row-y, box->ymin);
      ymax = MIN(row+y, box->ymax);
      zmin = MAX(plane - z, box->zmin);
      zmax = MIN(plane + z, box->zmax);
      for (j=ymin; j<=ymax; j++)
	{
	  for (k=zmin; k<=zmax; k++)
	    {
	      out_interval[ct].row_in_vol = j;
	      out_interval[ct].plane = k;
	      out_interval[ct].begin = xmin;
	      out_interval[ct].end = xmax;
	      ct++;
	    }
	}
    }

  *pout_interval = out_interval;
  *pout_size =   out_size;

    return(ct);
}

int EG_extend_pint_3d(Interval **interval, int in_size, int x, int y, int z, Interval **pout_interval, int *pout_size, Box_3d *box)
{
  int ymin;
  int ct;
  int i;
  int j;
  int k;
  int min_size;
  Interval *out_interval;
  int out_size;
  int plane;
  int xmin;
  int xmax;
  int row;
  int ymax;
  int zmax;
  int zmin;

  out_interval = *pout_interval;
  out_size = *pout_size;

  /* reallocate output array if necessary */
  min_size = in_size * (2 * y + 1) * (2 * z + 1);
  if (out_size < min_size)
    {
      out_interval = (Interval *)EG_realloc(out_interval, min_size * sizeof(Interval));
      if (out_interval == NULL)
	return(-1);
      
      out_size = min_size;
    }

  ct = 0;
  for (i=0; i<in_size; i++)
    {
      row = interval[i]->row_in_vol;
      plane = interval[i]->plane;

      /* clip to bounding box */
      xmin = MAX(interval[i]->begin - x, box->xmin);
      xmax = MIN(interval[i]->end + x, box->xmax);
      ymin = MAX(row-y, box->ymin);
      ymax = MIN(row+y, box->ymax);
      zmin = MAX(plane - z, box->zmin);
      zmax = MIN(plane + z, box->zmax);
      for (j=ymin; j<=ymax; j++)
	{
	  for (k=zmin; k<=zmax; k++)
	    {
	      out_interval[ct].row_in_vol = j;
	      out_interval[ct].plane = k;
	      out_interval[ct].begin = xmin;
	      out_interval[ct].end = xmax;
	      ct++;
	    }
	}
    }

  *pout_interval = out_interval;
  *pout_size =   out_size;

    return(ct);
}

