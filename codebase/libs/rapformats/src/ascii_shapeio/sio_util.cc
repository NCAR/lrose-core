// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/************************************************************************
*                                                                       *
*                              sio_util.cc
*                                                                       *
*************************************************************************

                        Thu Mar 16 15:40:23 1995

       Description:           shapeio utility routines.
                              reading/writing files.
			      printing records.
			      clearing structs.

       See Also:              shapeio.c ascii_shapeio.h sio_index_file.c

       Author:                Dave Albo

       Contents:              SIO_read_data(),  SIO_extract_indexed_data(),
			      SIO_append_data(), SIO_write_to_named_file(), 
			      SIO_print_shape(), SIO_free_shapes(), 
			      SIO_file_dump(), SIO_clear_data_file()

      Static Functions:       min_positive(), special_copy(), best_extrap(),
                              days_diff(), delta_time_value(), 
			      data_fits_request(), get_representitive_delta(),
			      build_index_values(),
			      file_is_aligned(), prepare_index_1(), 
			      build_best_extrap(), prepare_data_access(), 
			      read_1(), append_all_1day(), 
			      read_closest(), read_1way(), 
			      read_all()
*/

/*
 * System include files
 */


#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h>

/*
 * Local include files
 */

#include <toolsa/os_config.h>
#include <rapformats/ascii_shapeio.h>
#include <toolsa/globals.h>
#include <toolsa/mem.h>

#include "shapeio_int.h"
using namespace std;

/*
 * Definitions / macros / types
 */

#define BIG_MAX 200

/*
 * External references / global variables 
 */

static SIO_shape_data_t *Stemp_list=NULL;
static int Num_stemp = 0;
static char openApp[1] = {'a'};
static char openRead[1] = {'r'};
static char openWrite[1] = {'w'};

/*----------------------------------------------------------------*
 *
 * STATIC FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 * return index to element of an array with minimum non negative value.
 */

static int min_positive(int int_array[], int array_size)
{
  int i;
  int minimum = 0;
  int index = -1;
    
  for (i = 0; i < array_size; i++)
  {
    if (int_array[i] < 0)
      continue;

    if (index == -1)
    {
      minimum = int_array[i];
      index = i;
    }
    else
    {
      if (int_array[i] < minimum)
      {
	minimum = int_array[i];
	index = i;
      }
    } /* endif - index == -1 */

  } /* endfor - i */

  return index;
}

/*----------------------------------------------------------------*/
/*
 * A unique and special copy:
 *   copy the given Polyline as the 1 object inside the given shape.
 *
 *   (its assumed memory for S->P has been freed up prior to call.)
 */

static void special_copy(SIO_polyline_object_t *new_polyline,
			 SIO_shape_data_t *shape)
{
  int float_array_size;
  
  /*
   * Check this tangled logic..
   */
#ifdef CHECKMONDAY
  if (shape->P != NULL)
    printf("WARNING..possible memory leak in 'spcecial_copy'\n");
#endif

  /*
   * Set # of objects properly.
   */

  shape->num_objects = 1;
	
  /*
   * Allocate space for the one polyline.
   */

  shape->P = (SIO_polyline_object_t *)umalloc(sizeof(SIO_polyline_object_t));

  /*
   * COpy over P, including pointers, from P into S->P.  These pointers
   * will be reallocated and overwritten below.
   */

  *(shape->P) = *new_polyline;
    
  /*
   * Allocate space for the locations inside S->P..(overwrite the pointers)
   * and copy the point values.
   */

  float_array_size = new_polyline->npoints * sizeof(float);
  
  shape->P->lat = (float *)umalloc(float_array_size);
  shape->P->lon = (float *)umalloc(float_array_size);

  memcpy(shape->P->lat, new_polyline->lat, float_array_size);
  memcpy(shape->P->lon, new_polyline->lon, float_array_size);
  
  //
  // Only allocate space for motion dir vector components, if they exist.
  //

  if (new_polyline->u_comp != NULL && new_polyline->v_comp != NULL)
  {
    shape->P->u_comp = (float *)umalloc(float_array_size);
    shape->P->v_comp = (float *)umalloc(float_array_size);

    memcpy(shape->P->u_comp, new_polyline->u_comp, float_array_size);
    memcpy(shape->P->v_comp, new_polyline->v_comp, float_array_size);
  }
  else
  {
    shape->P->u_comp = NULL;
    shape->P->v_comp = NULL;
  }

  //
  // Only allocate space for value, if it exists.
  //

  if (new_polyline->value != NULL)
  {
    shape->P->value = (float *)umalloc(float_array_size);
    memcpy(shape->P->value, new_polyline->value, float_array_size);
  }
  else
    shape->P->value = NULL;

  return;
}
	

/*----------------------------------------------------------------*/
/*
 * Return pointer to the "best" extrapolation that best satisfies the
 * input request.
 */

static SIO_polyline_object_t *best_extrap(SIO_shape_data_t *shape,
					  SIO_read_request_t *request)
{
  int i, time_diff;
  int min_diff = 0;
  int min_diff_index = -1;
    
  SIO_polyline_object_t *polyline;
    
  /*
   * For each object, see if it is the best one.
   */

  for (i = 0; i < shape->num_objects; ++i)
  {
    polyline = &(shape->P[i]);

    /*
     * check whether data extrapolation time fits the request.
     */

    time_diff = polyline->nseconds - request->extrap;

    if (time_diff < 0)
      time_diff = -time_diff;

    if (time_diff > request->delta_extrap)
      /*
       * Nope..
       */
      continue;

    if (min_diff_index == -1)
    {
      /*
       * First time..
       */

      min_diff = time_diff;
      min_diff_index = i;
    }
    else if (time_diff < min_diff)
    {
      /*
       * Better..
       */

      min_diff = time_diff;
      min_diff_index = i;
    }
  } /* endfor - i */

  if (min_diff_index == -1)
    /*
     * Nothing close enough
     */
    return NULL;
  else
    /*
     * Return pointer to this best one.
     */
    return (&(shape->P[min_diff_index]));
}

/*----------------------------------------------------------------*/
/*
 * return true if the inputs are different dates.
 */

static int days_diff(UTIMstruct *t0, UTIMstruct *t1)
{
  return (t0->year != t1->year ||
	  t0->month != t1->month ||
	  t0->day != t1->day);
}

/*----------------------------------------------------------------*/
/*
 * return delta as follows:
 *
 * which:               returned delta
 * ------------------   ---------------------
 * SIO_MODE_CLOSEST_LE     (target  -  data_t)
 * SIO_MODE_CLOSEST_GE     (data_t  -  target)
 * SIO_MODE_CLOSEST     abs(target  -  data_t)
 */

static int delta_time_value(int which,
			    time_t target_time,
			    time_t data_time)
{
  int delta;
    
  switch (which)
  {
  case SIO_MODE_ALL:
  case SIO_MODE_CLOSEST:
    delta = target_time - data_time;
    if (delta < 0)
      delta = -delta;
    break;

  case SIO_MODE_CLOSEST_LE:
    delta = target_time - data_time;
    break;

  case SIO_MODE_CLOSEST_GE:
    delta = data_time - target_time;
    break;

  default:
    printf("ERROR unknown mode in delta_time_value:%d\n", which);
    delta = -999;
  } /* endswitch - which */

  return delta;
}

/*----------------------------------------------------------------*/
/*
 * Return 1 if data might fit request based on its index.
 */

static int data_fits_request(SIO_index_data_t *index,   /* data */
			     SIO_read_request_t *request, /* request*/
			     int *delta,          /* returned delta value*/
			     int *extrap_delta)   /* returned extrap delta*/
{
  time_t min_wanted, max_wanted;

  /*
   * Compute delta for inputs between data time and target time.
   * (for mode specified).
   */

  *delta = delta_time_value(request->mode,
			    request->target_time, index->data_time);

  if (*delta < 0 || *delta >= request->delta_time)
    /*
     * Data not in range specified by request...
     */
    return 0;
    
  /*
   * The data time is in range..what about the extrapolation criteria.
   */

  if (request->extrap == 0)
  {
    *extrap_delta = 0;
    return 1;
  }

  min_wanted = request->target_time + request->extrap - request->delta_extrap;
  max_wanted = request->target_time + request->extrap + request->delta_extrap;

  if (index->data_time + index->extrap_seconds > max_wanted ||
      index->data_time + index->extrap_seconds < min_wanted)
  {
    /*
     * The extrapolation falls outside the range wanted.
     */
    return 0;
  }
  else
  {
    *extrap_delta = (request->target_time + request->extrap) -
      (index->data_time + index->extrap_seconds);

    if (*extrap_delta < 0)
      *extrap_delta = -1 * (*extrap_delta);
  }	

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Goofy little routine to return a delta value from an index I setup.
 */

static int get_representitive_delta(SIO_read_request_t *request,
				    SIO_index_data_t *data_index,
				    int num_indexes,
				    int index[])
{
  if (num_indexes <= 0)
    return -1;
  else
    return (delta_time_value(request->mode, request->target_time,
			     data_index[index[0]].data_time));
}    

/*----------------------------------------------------------------*/
/*
 * Return updated index array list that does what I want..
 */

static int build_index_values(SIO_index_data_t *data_index, int i,
			      SIO_read_request_t *request,
			      int index[], int *num_index, int *min_delta,
			      int *min_extrap_delta)
{
  int delta, extrap_delta;
    
  if (data_fits_request(data_index, request, &delta, &extrap_delta) == 0)
    /*
     * Data outside range of interest..thats o.k. but don't do anything.
     */
    return 1;

  /*
   * Now the data is in range.
   */

  if (*num_index == 0)
  {
    /*
     * First time any data was in range.
     */
    *min_delta = delta;
    *min_extrap_delta = extrap_delta;
    index[0] = i;
    *num_index = 1;
  }
  else
  {
    if (*min_delta > delta)
    {
      /*
       * Better DATA than anything before..start the list over.
       */

      *num_index = 1;
      *min_delta = delta;
      *min_extrap_delta = extrap_delta;
      index[0] = i;
    }
    else if (*min_delta == delta)
    {
      /*
       * Equal to a growing list of best DATA delta's?
       * Check the extrap time..
       */

      if (*min_extrap_delta == extrap_delta)
      {
	index[*num_index] = i;
	if (++(*num_index) >= BIG_MAX-1)
	{
	  printf("ERROR overflow in index values\n");

	  /*
	   * Take the easy way out.
	   */
	  return 0;
	}
      }
      else if (*min_extrap_delta < extrap_delta)
	/*
	 * Worse
	 */
	return 1;
      else
      {
	/*
	 * Current extrap better than anything before..
	 * start over.
	 */

	*num_index = 1;
	*min_delta = delta;
	*min_extrap_delta = extrap_delta;
	index[0] = i;
      }
    }
  }
  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Prepare the index file and the list of index values that are the
 * ones we actually want to read from the data file.
 */

static int prepare_index_1(SIO_read_request_t *R,  /* what we want */
			   time_t file_time,       /* data file to read */
			   SIO_index_data_t **I,   /* returned index list*/
			   int *nindex,            /* returned list len */
			   int index[],            /* returned pointer list */
			   int *num_index,         /* # of pointers. */
			   char *bdry_index_dir,   /* dir for bdry index file */
			   char **bdry_file)       /* name of bdry file to use */
{
  int min_delta, min_extrap_delta, i;
  UTIMstruct ut;
    
  /*
   * Get the index file for this file..
   */

  *I = SIO_rebuild_index_file(R->directory, R->suffix, file_time, nindex, 
			      bdry_index_dir, 0, NULL);
    
  /*
   * Build up the index array.
   */

  *num_index = 0;

  for (i = 0; i < *nindex; ++i)
  {
    if (build_index_values(&((*I)[i]), i, R, index, num_index,
			   &min_delta, &min_extrap_delta) == 0)
      // error
      break;
  } // for (i=0; i<*nindex; ++i)

  if (*num_index <= 0)
  {
    // 
    // Check if file_time is <= 06Z.  If so, then check the previous
    // day's file.
    //

    UTIMunix_to_date(file_time, &ut);

    if (ut.hour <= 6)
    {
      // Get the index file for the previous day.

      *I = SIO_rebuild_index_file(R->directory, R->suffix, file_time, 
				  nindex, bdry_index_dir, 1, bdry_file);

      // Build up the index array.

      *num_index = 0;
      for (i=0; i<*nindex; ++i)
      {
	if (build_index_values(&((*I)[i]), i, R, index, num_index,
			       &min_delta, &min_extrap_delta) == 0)
	  // error
	  break;
      } // for (i=0; i<*nindex; ++i)

      if (*num_index <= 0)
	return 0;
      else
	return 1;
    } // if (ut.hour <= 6)

    return 0;
  } else
    return 1;
}

/*----------------------------------------------------------------*/
/*
 * Read in indicated data and use it if it fits the request
 * (append it to Stemp_list)
 */

static void build_best_extrap(FILE *fp,              /* opened data file*/
			      SIO_index_data_t *I,   /* record to read in */
			      SIO_read_request_t *R) /* the request */
{
  SIO_shape_data_t *S;
  SIO_polyline_object_t *P;

  /*
   * Get the data
   */

  if ((S = SIO_extract_indexed_data(fp, I)) == NULL)
  {
    printf("ERROR extracting an expected record from data\n");
    return;
  }

  /*
   * Does there exist an extrapolation that works?
   * (Find the best one)
   */

  if ((P = best_extrap(S, R)) == NULL)
  {
    SIO_free_shapes(S, 1);
    return;
  }
    
  /*
   * Now copy this thing into Stemp_list, copying only
   * the best extrapolation..free up the original.
   */

  Stemp_list[Num_stemp] = *S;
  special_copy(P, &Stemp_list[Num_stemp++]);
  SIO_free_shapes(S, 1);
}

/*----------------------------------------------------------------*/
/*
 * Set things up so ready to read data from data file into
 * Stemp_list.
 * Return 1 if things are ready to go.
 */

static int prepare_data_access(SIO_read_request_t *R, /* request*/
			       time_t file_time,    /* file */
			       int num_index,       /* # of recs wanted*/
			       FILE **fp,           /* opened on return*/
			       char **bdry_fname)   /* name of bdry file if
						       dif than issue time */
{
  /*
   * See if anything to do.
   */

  if (num_index <= 0)
    return 0;

  /*
   * Prepare to read the index list data records from the data file.
   * and to put the results into "Stemp_list."
   */

  if (*bdry_fname)
    *fp = SIO_open_named_data_file(*bdry_fname, openRead);
  else
    *fp = SIO_open_data_file(R->directory, R->suffix, file_time, openRead, 0);

  if (*fp == NULL)
    return 0;
    
  /*
   * Allocate space for at most num_index elements:
   */

  if (Stemp_list != NULL)
    ufree((char *)Stemp_list);
  Stemp_list =
    (SIO_shape_data_t *)ucalloc(num_index, sizeof(SIO_shape_data_t));

  /*
   * Prepare to fill stemp_list with data.
   */

  Num_stemp = 0;

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Return pointer to list of data records with the time closest to the
 * wanted time, as pulled out of indicated file.
 */

static SIO_shape_data_t *read_1(SIO_read_request_t *R, /* the request */
				    time_t file_time,    /* the file's time */
				    int *nshapes,        /* returned # */
				    char *bdry_index_dir)
{    
  SIO_index_data_t *I;
  int nindex;
  int index[BIG_MAX], num_index;
  int i;
  FILE *fp;
  char *bdry_fname = NULL;

  /*
   * Build up the index file, and the pointers into it.
   */

  if (prepare_index_1(R, file_time, &I, &nindex, index, &num_index, 
		      bdry_index_dir, &bdry_fname) == 0)
  {
    // didn't find any good bdry and extrap matches
    *nshapes = 0;
    return NULL;
  } // if (prepare_index_1(...) == 0)
    
  /*
   * Prepare the data file and Stemp_list based on this.
   */

  if (prepare_data_access(R, file_time, num_index, &fp, &bdry_fname) == 0)
  {
    *nshapes = 0;
    return NULL;
  } // if (prepare_data_access(...) == 0)
    
  /*
   * Now build up Stemp_list:
   */

  for (i=0; i<num_index; ++i)
    build_best_extrap(fp, &I[index[i]], R);
    
  /*
   * Done..close data file.
   */

  fclose(fp);

  /*
   * Did we store anything?
   */

  if (Num_stemp > 0)
    /*
     * Yes.
     */
    *nshapes = Num_stemp;
  else
  {
    /*
     * No
     */
    *nshapes = 0;
    ufree(Stemp_list);
    Stemp_list = NULL;
  } // if (Num_stemp > 0)

  return Stemp_list;
}

/*----------------------------------------------------------------*/
/*
 * Append in all records from the file indicated within given window of time
 * to the list Stemp_list.
 */

static SIO_shape_data_t *append_all_1day(SIO_read_request_t *R, /* request*/
					     time_t file_time,    /* data file*/
					     int *nshapes,        /* returned*/
					     char *bdry_index_dir)
					       /* dir for bdry index file */
{
  int delta, extrap_delta;
  FILE *fp;
  int i;
  int nindex;
  SIO_index_data_t *I;
    
  /*
   * Get an index file..
   */

  if ((I = SIO_rebuild_index_file(R->directory, R->suffix, file_time, &nindex,
				  bdry_index_dir, 0, NULL)) == NULL)
    return Stemp_list;
    
  /*
   * Open the data file
   */

  fp = SIO_open_data_file(R->directory, R->suffix, file_time, openRead, 0);
  if (fp == NULL)
    return Stemp_list;

  /*
   * (This is append, so build up Stemp_list as we go..start with
   * space for the next element (0th)).
   */

  if (Stemp_list == NULL)
    Stemp_list = (SIO_shape_data_t *)ucalloc(1,sizeof(SIO_shape_data_t));

  for (i=0; i<nindex; ++i)
  {
    /*
     * IF data in range, and extrap time in range..
     */

    if (data_fits_request(&I[i], R, &delta, &extrap_delta) == 1)
      /*
       * Build up an extrap and append it to Stemp_list.
       */
      build_best_extrap(fp, &I[i], R);

    /*
     * Prepare to need more space to store.
     */

    //
    // this looks like it could be thrashing memory a bit
    // consider resizing by more than one index at a time (tlb 3/7/97)
    //

    Stemp_list = (SIO_shape_data_t *)
      urealloc((SIO_shape_data_t *)Stemp_list,
	      (Num_stemp+1)*sizeof(SIO_shape_data_t));
  }
  fclose(fp);
    
  if (Num_stemp > 0)
  {
    *nshapes = Num_stemp;
    return Stemp_list;
  }
  else
  {
    *nshapes = 0;
    return NULL;
  }
}

/*----------------------------------------------------------------*/
/*
 * Return array of shapes, NULL for none.
 * A slightly Gnarly algorithm.
 */

static SIO_shape_data_t *read_closest(SIO_read_request_t *R, /* wanted*/
					  int *nshapes,        /* returned*/
					  char *bdry_index_dir)
                                             /* dir for bdry index file */
{
  UTIMstruct t[3];
  SIO_index_data_t *I[3];
  int nindex[3];
  int index[3][BIG_MAX];
  time_t file_time[3];
  int delta[3];
  int num_index[3];
  int j, k, ibest;
  FILE *fp;
  char *bdry_fname = NULL;
    
  /*
   * Prepare to handle the input request time, plus one day either
   * side.
   */

  file_time[0] = R->target_time-R->delta_time;
  file_time[1] = R->target_time;
  file_time[2] = R->target_time+R->delta_time;

  for (k=0; k<3; ++k)
  {
    UTIMunix_to_date(file_time[k], &t[k]);
    I[k] = NULL;
    nindex[k] = 0;
  }
    
  delta[0] = -1;
  if (days_diff(&t[1], &t[0]))
  {
    /*
     * The prior day has intersection with the request.
     * Pull up that index file (previous day has some data we might
     * want).
     */

    if (prepare_index_1(R, file_time[0], &I[0], &nindex[0], index[0],
			&num_index[0], bdry_index_dir, &bdry_fname) == 1)
      delta[0] = get_representitive_delta(R, I[0], num_index[0],
					  index[0]);
  } /* endif - days_diff */
    
  /*
   * We always want the target time's index file.
   */

  delta[1] = -1;
  if (prepare_index_1(R, file_time[1], &I[1], &nindex[1], index[1],
		      &num_index[1], bdry_index_dir, &bdry_fname) == 1)
    delta[1] = get_representitive_delta(R, I[1], num_index[1],
					index[1]);
    
  delta[2] = -1;
  if (days_diff(&t[2], &t[1]))
  {
    /*
     * The prior day has intersection with the request.
     * Pull up that index file (previous day has some data we might
     * want).
     */

    if (prepare_index_1(R, file_time[2], &I[2], &nindex[2], index[2],
			&num_index[2], bdry_index_dir, &bdry_fname) == 1)
      delta[2] = get_representitive_delta(R, I[2], num_index[2],
					  index[2]);
  } /* endif - days_diff */
    
  /*
   * Figure out which of these lists has closest time to target..
   */

  ibest = min_positive(delta, 3);
  if (ibest < 0)
  {
    *nshapes = 0;
    return NULL;
  }	

  /*
   * Prepare the data file and Stemp_list based on this best thing.
   */

  if (prepare_data_access(R, file_time[ibest], num_index[ibest], &fp, 
			  &bdry_fname) == 0)
  {
    *nshapes = 0;
    return NULL;
  }

  /*
   * Add in all records with this time.
   */

  for (j=0; j<num_index[ibest]; ++j)
    build_best_extrap(fp, &I[ibest][index[ibest][j]], R);

  /*
   * Done with data file..
   */

  fclose(fp);

  /*
   * prepare to and return.
   */

  if (Num_stemp > 0)
    *nshapes = Num_stemp;
  else
  {
    *nshapes = 0;
    ufree(Stemp_list);
    Stemp_list = NULL;
  }

  return Stemp_list;
}

/*----------------------------------------------------------------*/

static SIO_shape_data_t *read_1way(SIO_read_request_t *R,
				       time_t other_time,
				       int *nshapes, char *bdry_index_dir)
// other_time: R->target_time - R->delta_time; furthest back time to look
//             for boundaries.
{
  UTIMstruct t, tother;
  SIO_shape_data_t *S;
    
  /*
   * Try to start with current time and look for something as close
   * as possible in given mode.
   */

  if ((S = read_1(R, R->target_time, nshapes, bdry_index_dir)) == NULL)
  {
    /*
     * Since that failed, see if data in other_time might work.
     * (expand search to previous or next day (tother)).
     */

    UTIMunix_to_date(R->target_time, &t);
    UTIMunix_to_date(other_time, &tother);
    if (days_diff(&t, &tother))
      S = read_1(R, other_time, nshapes, bdry_index_dir);
  }

  if (S == NULL)
    *nshapes = 0;

  return S;
}

/*----------------------------------------------------------------*/

static SIO_shape_data_t *read_all(SIO_read_request_t *R, int *nshapes,
				      char *bdry_index_dir)
{
  UTIMstruct t, t0, t1;
  SIO_shape_data_t *S;
    
  /*
   * We need all shapes in range...which might span 2 days.
   */ 

  UTIMunix_to_date(R->target_time+R->delta_time, &t1);
  UTIMunix_to_date(R->target_time, &t);
  UTIMunix_to_date(R->target_time-R->delta_time, &t0);

  /*
   * Start out by clearing out stemp_list...now can just append to it..
   * If its already allocated, thats o.k., we'll just realloc as needed.
   */

  Num_stemp = 0;
  *nshapes = 0;

  /*
   * Is there any data in range in day before day?
   */

  if (days_diff(&t, &t0))
    /*
     * Add in shapes from day t0.
     */
    S = append_all_1day(R, R->target_time-R->delta_time, nshapes, 
			    bdry_index_dir);

  /*
   * Now main day.
   */

  S = append_all_1day(R, R->target_time, nshapes, bdry_index_dir);

  /*
   * What about day t1.
   */

  if (days_diff(&t, &t1))
    /*
     * Add in shapes from day t1.
     */
    S = append_all_1day(R, R->target_time+R->delta_time, nshapes,
			bdry_index_dir);

  return S;
}

/*----------------------------------------------------------------*
 *
 * LOCALLY EXPORTED FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 * Return 1 if file index indiecated by index record aligns nicely
 * with file contents.
 */

int file_is_aligned(FILE *fd, SIO_index_data_t *I)
{
  /*
   * Skip to position indicated
   */

  if (fseek(fd, (long)I->file_index, 0) != 0)
  {
    printf("ERROR seeking...to position %d\n", I->file_index);
    return 0;
  }

  /*
   * Reset work buf (clear it out)
   */

  SIO_clear_read_buf();

  /*
   * Now read in one line..
   */

  return (read_first_line(fd, 0));
}

/*----------------------------------------------------------------*
 *
 * GLOBALLY EXPORTED FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 * Skip to indicated position, read in data, return data.
 */

SIO_shape_data_t *SIO_extract_indexed_data(FILE *fd, SIO_index_data_t *I)
{
  static SIO_shape_data_t shape;
  static int first_time = TRUE;
  
  int newpos;
    
  /*
   * Clear the shape buffer the first time through
   */

  if (first_time)
  {
    memset(&shape, 0, sizeof(shape));
    first_time = FALSE;
  }
  
  /*
   * Skip to position indicated
   */

  if (fseek(fd, (long)I->file_index, 0) != 0)
  {
    printf("ERROR seeking...to position %d\n", I->file_index);
    return NULL;
  }

  /*
   * Reset work buf (clear it out)
   */

  SIO_clear_read_buf();

  /*
   * read in one record from the file into our shape buffer
   */

  if (SIO_read_record(fd, &shape, &newpos) == 0)
    return NULL;

  if (shape.data_time != I->data_time)
    printf("ERROR in alignment..what the heck is going on?\n");

  return &shape;
}

/*----------------------------------------------------------------*/
/*
 * Return array of shapes, satisfying input criteria, NULL for none.
 * Note:  extrapolation time is closest time to center of interval
 *   (data time + extrap-delta_extrap, data time + extrap + delta_extrap).
 *
 * where data time is the data record actually used.
 *
 */

SIO_shape_data_t *SIO_read_data(SIO_read_request_t *R, /* request*/
				int *nshapes,        /* returned # of shapes.*/
				char *bdry_index_dir)
			      /* dir for the bdry index file */
{
  SIO_shape_data_t *S = NULL;
    
  *nshapes = 0;
    
  /*
   * The time window spans two or more days..if its more than 1 day,
   * the rquest was a little bit too greedy..don't do anything.
   */

  if (2*R->delta_time > 24*60*60)
  {
    fprintf(stderr, "Time window in read_shape_data too big %d max=%d\n",
	    R->delta_time, 24*60*60);
    return S;
  }

  /*
   * Just in case..
   */

  SIO_clear_read_buf();

  /*
   * Do various searches.
   */

  switch (R->mode)
  {
  case SIO_MODE_CLOSEST_LE:
    S = read_1way(R, R->target_time - R->delta_time, nshapes, 
		  bdry_index_dir);
    break;

  case SIO_MODE_CLOSEST_GE:
    S = read_1way(R, R->target_time + R->delta_time, nshapes,
		  bdry_index_dir);
    break;

  case SIO_MODE_CLOSEST:
    S = read_closest(R, nshapes, bdry_index_dir);
    break;

  case SIO_MODE_ALL:
    S = read_all(R, nshapes, bdry_index_dir);
    break;

  default:
    printf("ERROR bad mode in read %d\n", R->mode);
  } /* endswitch - R->mode */

  return S;
}

/*----------------------------------------------------------------*/
/*
 * Append input data to proper file..write it.
 */

int SIO_append_data(char *directory,     /* where */
		    char *suffix,        /* file name suffix*/
		    SIO_shape_data_t *shape,   /* some data*/
		    int nshapes,         /* # of data shapes*/
		    int method,          /* SIO_APPEND_NORMAL or SIO_APPEND_SAFE*/
		    int minutes_offset)
/* minutes_offset: number of minutes to offset the time; effectively
 *                 adjusting the time when a new file is opened
 */
{
  char *filename;
  FILE *fp;
  int i, status, multi;
  UTIMstruct first_time, this_time;
    
  switch (method)
  {
  case SIO_APPEND_NORMAL:
    status = 1;
    for (i = 0; i < nshapes; ++i)
    {
      /*
       * Open the file for append..
       */

      fp = SIO_open_data_file(directory, suffix, shape[i].data_time, openApp,
			      minutes_offset);
      if (fp == NULL)
      {
	status = 0;
	continue;
      }

      if (SIO_write_data(fp, &shape[i], 0) != 1)
	status = 0;
      fclose(fp);
    }
    break;

  case SIO_APPEND_SAFE:
    status = 1;

    /*
     * Check the dates on each record, see if multiple days..
     */

    multi = FALSE;
    UTIMunix_to_date(shape[0].data_time, &first_time);
    for (i = 1; i < nshapes; ++i)
    {
      UTIMunix_to_date(shape[i].data_time, &this_time);
      if (first_time.year != this_time.year ||
	  first_time.month != this_time.month ||
	  first_time.day != this_time.day)
      {
	multi = TRUE;
	break;
      }
    } /* endfor - i */
    
    if (multi)
    {
      /*
       * Do each record separately..(slowly).
       */

      for (i = 0; i < nshapes; ++i)
      {
	/*
	 * Form the file name
	 */

	filename = SIO_file_name(suffix, shape[i].data_time);
		
	/*
	 * Append this data to this file in an indivisible manner..
	 */

	if (SIO_write_to_named_file(directory, filename, &shape[i],
				    1, openApp, 0) == 0)
	  status = 0;
      }
    }
    else
    {
      /*
       * Do the lump set of records..
       */

      filename = SIO_file_name(suffix, shape[0].data_time);
      if (SIO_write_to_named_file(directory, filename,
				  shape, nshapes, openApp, 0) == 0)
	status = 0;
    }
    break;

  default:
    printf("ERROR bad mode in sio-append_data %d\n", method);
    status = 0;
  } /* endswitch - method */

  return status;
}

/*----------------------------------------------------------------*/
/*
 * Write input data to named file.
 */

int SIO_write_to_named_file(char *directory,   /* where */
			    char *name,        /* file name */
			    SIO_shape_data_t *shape, /* products, filled in*/
			    int nshapes,       /* # of data shape products */
			    char *mode,        /* "w", "a" */
		            int print_shapes)  /* print shape info or not */
{
  char output_filename[MAX_PATH_LEN];
  char temp_filename[MAX_PATH_LEN];
  char sys_cmd[MAX_PATH_LEN * 2];
  FILE *fp;
  int i, status, append;
  char *tempdir;
  int pid;
    
  /*
   * Save the PID for use in the temporary file name.
   */

  pid = getpid();

  /*
   * Check the write mode (a = append, w = overwrite)
   */

  if (strcmp(mode, "a") == 0)
    append = TRUE;
  else if (strcmp(mode, "w") == 0)
    append = FALSE;
  else
  {
    printf("Unknown mode in sio_write_to_named %s\n", mode);
    return 0;
  }
	
  /*
   * Determine the filenames to use.
   */

  tempdir = getenv("HOME");
  if (tempdir == NULL)
  {
    printf("WARNING no $HOME, try to use current dir for temp files\n");
    tempdir = (char*)".";
  }
	
  sprintf(output_filename, "%s/%s", directory, name);
  sprintf(temp_filename, "%s/temp_file.%d", tempdir, pid);

  if (append)
  {
    /*
     * See if there is currently a file of the appropriate name.
     * If so, we need to copy this data into our temporary file
     * before adding the new data so we don't lose it.
     */

    fp = fopen(output_filename, "r");
    if (fp != NULL)
    {
      /*
       * Close the file so we can copy it.
       */

      fclose(fp);
	    
      sprintf(sys_cmd, "cp %s %s", output_filename, temp_filename);
      system(sys_cmd);
	    
    }
	
  }
    
  /*
   * Open the temp file in the mode wanted. (append or write).
   */

  fp = fopen(temp_filename, mode);
  if (fp == NULL)
  {
    printf("ERROR opening %s in mode %s\n", temp_filename, mode);
    return 0;
  }

  status = 1;
  for (i = 0; i < nshapes; ++i)
  {
    if (SIO_write_data(fp, &shape[i], print_shapes) != 1)
      status = 0;
  }

  fclose(fp);
  if (status == 0)
    return status;

  /*
   * Now overwrite the original file with the temp file.
   */

  sprintf(sys_cmd, "mv -f %s %s", temp_filename, output_filename);
  system(sys_cmd);

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * A useful routine to show what is in a SIO_shape_data_t buffer.
 */

void SIO_print_shape(SIO_shape_data_t *s,
		     int level /* 0 = 1line,
				* 1 = 1 line of 0th polyline in shape.
				* 2 = 1 line of all polylines in shape.
				* 3 = full, 0th polyline in shape.
				* 4 = full, all polylines in shape.
				*/
		     )
{
  int i, ii, k;
  UTIMstruct t;
  SIO_polyline_object_t *P;
    
  printf("PROD:type:%s sbtype:%s group:%d nobj:%d seq:%d motion:%.2f,%.2f\n",
	 s->type, s->sub_type, s->group_id, s->num_objects,
	 s->sequence_number, s->motion_dir, s->motion_speed);

  if (s->line_type != NULL)
    printf("Colide line type:%s qual value:%.2f qual threshold:%.2f\n",
	   s->line_type, s->qual_value, s->qual_thresh);

  UTIMunix_to_date(s->data_time, &t);
  printf("data   time:%d %2d %02d %02d %02d %02d\n",
	 (int)t.year, (int)t.month,
	 (int)t.day, (int)t.hour, (int)t.min, (int)t.sec);
  UTIMunix_to_date(s->valid_time, &t);
  printf("valid  time:%d %2d %02d %02d %02d %02d\n",
	 (int)t.year, (int)t.month,
	 (int)t.day, (int)t.hour, (int)t.min, (int)t.sec);
  UTIMunix_to_date(s->expire_time, &t);
  printf("expire time:%d %2d %02d %02d %02d %02d\n",
	 (int)t.year, (int)t.month,
	 (int)t.day, (int)t.hour, (int)t.min, (int)t.sec);
    
  if (level <= 0)
    return;
  for (k = 0; k < s->num_objects; ++k)
  {
    P = &s->P[k];
    printf("OBJECT %d: '%s', %d points %d seconds\n",
	   k, P->object_label, P->npoints, P->nseconds);
    if (level == 1)
      break;
    if (level == 3 || level == 4)
    {
      for (ii = 0,i = 0; i < P->npoints; ++i,++ii)
      {
	if (ii > 3)
	{
	  printf("\n");
	  ii = 0;
	}
	printf("(%.2f,%.2f)\t", P->lat[i], P->lon[i]);
	if (P->u_comp != NULL)
	  printf("(u,v) = (%.2f,%.2f)\n", P->u_comp[i], 
		 P->v_comp[i]);
	if (P->value != NULL)
	  printf("value = %.2f\n", P->value[i]);
      }
      printf("\n");
    }
    if (level == 3)
      break;
  }
}

/*----------------------------------------------------------------*/
/*
 * Free all memory allocated within SIO_shape_data_t structs.
 */

void SIO_free_shapes(SIO_shape_data_t *s, int nshapes)
{
  SIO_shape_data_t *S;
  SIO_polyline_object_t *P;
  int i, j;
  
  for (i = 0; i < nshapes; ++i)
  {
    S = &s[i];
    S->type = NULL;
    S->sub_type = NULL;
    S->sequence_number =0;
    S->group_id = 0;
    S->data_time = 0;
    S->valid_time = 0;
    S->expire_time = 0;
    S->description[0] = 0;
    S->motion_dir = 0.0;
    S->motion_speed = 0.0;
    S->line_type = NULL;
    S->qual_value = 0.0;
    S->qual_thresh = 0.0;
    if (S->P != NULL)
    {
      for (j = 0; j < S->num_objects; ++j)
      {	    
	P = &S->P[j];
	if (P->lat != NULL)
	  ufree((char *)P->lat);
	if (P->lon != NULL)
	  ufree((char *)P->lon);
	if (P->u_comp != NULL)
	  ufree((char *)P->u_comp);
	if (P->v_comp != NULL)
	  ufree((char *)P->v_comp);
	if (P->value != NULL)
	  ufree((char *)P->value);
      }
      ufree((char *)S->P);
      S->P = NULL;
    }
    S->num_objects = 0;
  }
  return;
}

/*----------------------------------------------------------------*/
/*
 * Print out entire contents of a shape data file.
 */

void SIO_file_dump(char *directory,  /* where is data */
		   char *suffix,     /* file name suffix*/
		   time_t t,         /* time of file */
		   int level)         /* 0 = 1line,
				       * 1 = 1 line of 0th polyline.
				       * 2 = 1 line of all polylines.
				       * 3 = full, 0th polyline.
				       * 4 = full, all polylines.
				       */
{
  char *c;
  SIO_shape_data_t *S;
  FILE *fp;
  SIO_index_data_t *I;
  int nindex, i;
    
  I = SIO_rebuild_index_file(directory, suffix, t, &nindex, NULL, 0, NULL);

  /*
   * Form the file name..
   */

  fp = SIO_open_data_file(directory, suffix, t, openRead, 0);
  c = SIO_file_name_full(directory, suffix, t, 0);
  printf("\n\t\tDUMP OF FILE %s\n", c);
  if (fp  == NULL)
  {
    printf("\tERROR opening %s\n", c);
    return;
  }

  for (i = 0; i < nindex; ++i)
  {
    S = SIO_extract_indexed_data(fp, &I[i]);
    if (S == NULL)
    {
      printf("ERROR extracting %dth data\n", i);
      continue;
    }	    
    SIO_print_shape(S, level);
    SIO_free_shapes(S, 1);
  }    

  fclose(fp);
  return;
}

/*----------------------------------------------------------------*/

int SIO_clear_data_file(char *directory,     /* where */
			char *suffix,         /* file name suffix*/
			time_t time)
{
  FILE *fp;
  
  fp = SIO_open_data_file(directory, suffix, time, openWrite, 0);
  if (fp == NULL)
    return 0;
    
  fclose(fp);
  return 1;
}

/*----------------------------------------------------------------*/
