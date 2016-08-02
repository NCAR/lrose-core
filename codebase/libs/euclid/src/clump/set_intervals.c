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
 * 	set_intervals
 *
 * PURPOSE
 * 	set elements of an array corresponding to intervals to a
 * specified value
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - Jan 27, 1993: Created.
 */

#include <sys/types.h>
#include <string.h>
#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 * 	set_intervals - set all elements in an array corresponding to
 * intervals to a specified value.  Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 * 	interval_array - array of underlying intervals
 *      ncols - number of columns in 2d matrix corresponding to array
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */
void EG_set_intervals(unsigned char *array, int ncols, Interval **interval_array, int size, unsigned char value)
{
  int i;
  Interval *iptr;
  
  for (i=0; i<size; i++)
    {
      iptr = interval_array[i];
      memset (&array[iptr->row_in_vol * ncols + iptr->begin], (int)value, iptr->end - iptr->begin + 1);
    }
}

/*
 * DESCRIPTION:    
 * 	set_intervals_row_hdr - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals except that it uses a row_hdr array as
 * opposed to an array of intervals.    Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      nrows - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */
void EG_set_intervals_row_hdr(unsigned char *array, Row_hdr *row_hdr, int nrows, int ncols, unsigned char value)
{
  int i;
  int j;
  Interval *iptr;
  
  for (i=0; i<nrows; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
/*	  printf("row %d, begin %d, end %d\n", iptr->row, iptr->begin, iptr->end); */
	  memset (&array[iptr->row_in_vol * ncols + iptr->begin], (int)value, iptr->end - iptr->begin + 1);
	}
    }
}

/*
 * DESCRIPTION:    
 * 	set_intervals_int16 - set all elements in an array corresponding to
 * intervals to a specified value.  Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 * 	interval_array - array of underlying intervals
 *      ncols - number of columns in 2d matrix corresponding to array
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */
void EG_set_intervals_int16(unsigned short *array, int ncols, Interval **interval_array, int size, unsigned short value)
{
  int i;
  Interval *iptr;
  unsigned short *ptr;

  for (i=0; i<size; i++)
    {
      iptr = interval_array[i];
      for (ptr = &array[iptr->row_in_vol * ncols + iptr->begin]; ptr <= &array[iptr->row_in_vol * ncols + iptr->end]; ptr++)
	*ptr = value;

    }
}

/*
 * DESCRIPTION:    
 * 	set_intervals_row_hdr - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals except that it uses a row_hdr array as
 * opposed to an array of intervals.    Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      nrows - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */
void EG_set_intervals_row_hdr_int16(unsigned short *array, Row_hdr *row_hdr, int nrows, int ncols, unsigned short value)
{
  int i;
  int j;
  Interval *iptr;
  unsigned short *ptr;
  
  for (i=0; i<nrows; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  for (ptr = &array[iptr->row_in_vol * ncols + iptr->begin]; ptr <= &array[iptr->row_in_vol * ncols + iptr->end]; ptr++)
	    {
	      *ptr = value;
	    }
	}
    }
}

/*
 * DESCRIPTION:    
 * 	set_intervals_clump - set all elements in an array
 * corresponding to intervals to the clump value.  This function is
 * similar to set_intervals_row_hdr except that clump values are used in
 * setting the underlying array.  Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      nrows - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */
void EG_set_intervals_clump(unsigned char *array, Row_hdr *row_hdr, int nrows, int ncols)
{
  int i;
  int j;
  Interval *iptr;
  
  for (i=0; i<nrows; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
/*	  printf("row %d, begin %d, end %d, id %d\n", iptr->row, iptr->begin, iptr->end, iptr->id); */
	  memset (&array[iptr->row_in_vol * ncols + iptr->begin], (int)iptr->id, iptr->end - iptr->begin + 1);
	}
    }
}

/*
 * DESCRIPTION:    
 * 	set_intervals_translate_2d - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals_row_hdr except that it sets array values
 * using an offset.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      new_xdim - number of columns in array
 *      ydim - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 *      xoff - x offset of interval information from beginning of array
 *      yoff - y offset of interval information from beginning of array
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */
void EG_set_intervals_translate_2d(unsigned char *array, Row_hdr *row_hdr, int ydim, int new_xdim, int xoff, int yoff, unsigned char value)
{
  int i;
  int j;
  Interval *iptr;
  int loc;
  int new_row;
  int nrows;
  
  nrows = ydim;
  
  for (i=0; i<nrows; i++)
    {
      /* translate the row */
      new_row = i  + yoff;
      
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  loc = new_row*new_xdim + iptr->begin + xoff;
	  
	  /*	  printf("row %d, begin %d, end %d\n", iptr->row, iptr->begin, iptr->end);
		  printf("loc is %d\n", loc); */
	  memset (&array[loc], (int)value, iptr->end - iptr->begin + 1);
	}
    }
}

/*
 * DESCRIPTION:    
 * 	set_intervals_translate_3d - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals_translate_2d except that it uses 3
 * dimensions.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      new_xdim - number of columns in array
 *      ydim - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 *      xoff - x offset of interval information from beginning of array
 *      yoff - y offset of interval information from beginning of array
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */
void EG_set_intervals_translate_3d(unsigned char *array, Row_hdr *row_hdr, int ydim, int zdim, int new_xdim, int new_ydim, int xoff, int yoff, int zoff, unsigned char value)
{
  int i;
  int j;
  Interval *iptr;
  int loc;
  int new_row;
  int nrows;
  
  nrows = ydim * zdim;
  
  for (i=0; i<nrows; i++)
    {
      /* translate the row */
      new_row = i % ydim + (i/ydim + zoff) * new_ydim + yoff;
      
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  loc = new_row*new_xdim + iptr->begin + xoff;
	  
	  /*	  printf("row %d, begin %d, end %d\n", iptr->row, iptr->begin, iptr->end);
		  printf("loc is %d\n", loc); */
	  memset (&array[loc], (int)value, iptr->end - iptr->begin + 1);
	}
    }
}







