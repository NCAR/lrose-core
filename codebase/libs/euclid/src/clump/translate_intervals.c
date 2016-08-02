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
 *	translate_intervals
 *
 * PURPOSE
 *	translate an array of intervals by a vector
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Feb 22, 1993: Created.
 */
#include <math.h>
#include <stdio.h>
#include <euclid/clump.h>

#define DELTA 1.0

int EG_translate_int_2d(Interval *interval, int in_size, double x, double y, Interval **pout_interval, int *pout_size, Box_2d *box)
{
  double begin;
  double end;
  /* int last_row; */
  int min_size;
  int ct;
  int i;
  int j;
  double row;
  Interval *out_interval;
  int out_size;
  double xabs;
  double yabs;
  double xadj;
  double yadj;
  
  
  /* last_row = -1; */
  xabs = fabs(x);
  yabs = fabs(y);
  out_interval = *pout_interval;
  out_size = *pout_size;

  /* reallocate output array if necessary */
  min_size = in_size * ((int)yabs + (int)xabs + 1);
  printf("min_size is %d\n", min_size);

  if (out_size < min_size)
    {
      out_interval = (Interval *)EG_realloc(out_interval, min_size * sizeof(Interval));
      if (out_interval == NULL)
	return(-1);
      
      out_size = min_size;
    }

  if (x == 0 && y == 0)
    {
      for (i=0; i<in_size; i++)
	{
	  out_interval[i] = interval[i];
	}
      *pout_interval = out_interval;
      *pout_size =   out_size;
      return(in_size);
    }


  ct = 0;
  if (yabs > xabs)
    {
      xadj = sgn(x) * xabs / yabs;
      yadj = sgn(y);
/*      printf("yabs %f, yadj %f, xabs %f, xadj %f, \n", yabs, yadj, xabs, xadj);*/
      for (i=0; i<in_size; i++)
	{
	  row = interval[i].row_in_vol;
	  begin = interval[i].begin;
	  end = interval[i].end;
	  for (j=0; j<=yabs; j++)
	    {
	      if (row < box->ymin || row > box->ymax)
		break;	     /* break into outer loop since row won't change */
	      if (begin < box->xmin)
		begin = box->xmin;
	      if (end > box->xmax)
		end = box->xmax;
	      out_interval[ct].row_in_vol = row;
	      out_interval[ct].begin = begin;
	      out_interval[ct].end = end;
	      /*	      printf("out: row %d, begin %d, end %d\n", out_interval[ct].row, out_interval[ct].begin, out_interval[ct].end, row, begin, end); */
	      row += yadj;
	      begin += xadj;
	      end += xadj;
	      ct++;
	    }
	}
    }
  else
    {
      xadj = sgn(x);
      yadj = sgn(y) * yabs / xabs;
/*      printf("yabs %f, yadj %f, xabs %f, xadj %f, \n", yabs, yadj, xabs, xadj);*/
      for (i=0; i<in_size; i++)
	{
	  row = interval[i].row_in_vol;
	  begin = interval[i].begin;
	  end = interval[i].end;
	  for (j=0; j<=xabs; j++)
	    {
	      if (row < box->ymin || row > box->ymax)
		break;	     /* break into outer loop since row won't change */
	      if (begin < box->xmin)
		{
		  begin = box->xmin;
		  if (end < box->xmin)
		    end = box->xmin;
		}
	      if (end > box->xmax)
		{
		  end = box->xmax;
		  if (begin > box->xmax)
		    begin = box->xmax;
		}
	      out_interval[ct].row_in_vol = row;
	      out_interval[ct].begin = begin;
	      out_interval[ct].end = end;
/*	      printf("out: row %d, begin %d, end %d\n", out_interval[ct].row, out_interval[ct].begin, out_interval[ct].end, row, begin, end); */
	      row += yadj;
	      begin += xadj;
	      end += xadj;
	      ct++;
	    }
	}
    }

  *pout_interval = out_interval;
  *pout_size =   out_size;
  printf("ct is %d, out_size is %d\n", ct, out_size);
  return(ct);

}


