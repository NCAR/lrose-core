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
/*********************************************************************
 * spdb.c
 *
 * Symbolic products data base
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * June 1996
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <toolsa/os_config.h>
#include <dataport/bigend.h>
#include <symprod/spdb.h>
#include <toolsa/mem.h>
#include <toolsa/membuf.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>
#include <toolsa/file_io.h>
#include <toolsa/ldata_info.h>

enum {
  STORE_ONCE,
  STORE_OVERWRITE,
  STORE_ADD
};

enum {
  SETUP_SUCCESS,
  SETUP_NO_FILES,
  SETUP_FAILURE,
  SETUP_READ,
  SETUP_WRITE
};

#define UNIX_TIME_MAX 2147483647

/*
 * file scope prototypes
 */

static si32 add_chunk_ref(spdb_handle_t *handle,
			  si32 data_type,
			  si32 valid_time,
			  si32 expire_time,
			  si32 offset,
			  si32 len);

static int alloc_chunk_refs(spdb_handle_t *handle,
			    si32 n_chunks);

static int check_init(spdb_handle_t *handle);

static int clear_locks(spdb_handle_t *handle);

static void close_files(spdb_handle_t *handle);

static si32 first_posn_after(spdb_handle_t *handle,
			     si32 start_time);
     
static si32 first_time_after(spdb_handle_t *handle,
			     si32 request_time,
			     si32 end_time,
			     si32 data_type,
			     si32 *data_time);

static si32 first_time_before(spdb_handle_t *handle,
			      si32 request_time,
			      si32 end_time,
			      si32 data_type,
			      si32 *data_time);

static int get_time_nearest(spdb_handle_t *handle,
			    si32 request_time,
			    si32 time_margin,
			    si32 data_type,
			    si32 *time_nearest);

static si32 posn_at_time(spdb_handle_t *handle,
			 si32 valid_time,
			 si32 data_type);

static si32 posn_1min_ahead(spdb_handle_t *handle,
			    si32 start_posn);

static int read_chunk(spdb_handle_t *handle,
		      si32 data_offset,
		      void *chunk, int len);

static int set_earliest_valid(spdb_handle_t *handle,
			      si32 valid_time,
			      si32 expire_time);

static int setup_files(spdb_handle_t *handle,
		       si32 valid_time,
		       int mode);

static int setup_locks(spdb_handle_t *handle, int mode);

static int store_chunk(spdb_handle_t *handle,
		       si32 data_type,
		       si32 valid_time, si32 expire_time,
		       void *chunk, int len,
		       int mode);
     
static int stored_posn(spdb_handle_t *handle,
		       si32 valid_time,
		       si32 data_type);
     
static int str_compare(const void *v1, const void *v2);

static int write_chunk(spdb_handle_t *handle,
		       si32 valid_time, si32 *data_offset,
		       void *chunk, int len);

static int write_indx_file(spdb_handle_t *handle);

/***********************
 * SPDB_init()
 *
 * Initialize an SPDB object
 *
 * returns 0 on success, -1 on failure
 */

int SPDB_init(spdb_handle_t *handle,
	      char *prod_label,
	      si32 prod_id,
	      char *dir)
     
{
  
  char pid_str[64];
  struct stat stat_buf;

  if (prod_label == NULL) {
    STRncopy(handle->prod_label, "none", SPDB_LABEL_MAX);
  } else {
    STRncopy(handle->prod_label, prod_label, SPDB_LABEL_MAX);
  }

  /*
   * check that the directory exists
   */
  
  if (dir == NULL) {
    fprintf(stderr, "ERROR - SPDB_init: product %s\n",
	    handle->prod_label);
    fprintf(stderr, "Data base dir must be non-null\n");
    return (-1);
  }
  if (stat(dir, &stat_buf)) {
    fprintf(stderr, "ERROR - SPDB_init: product %s\n",
	    handle->prod_label);
    fprintf(stderr, "Data base directory must exist\n");
    perror(dir);
    return (-1);
  }

  STRncopy(handle->dir, dir, SPDB_PATH_MAX);

  handle->indx_file = NULL;
  handle->data_file = NULL;
  handle->prod_id = prod_id;
  handle->active = FALSE;
  handle->locked = FALSE;
  handle->init = SPDB_INIT_FLAG;

  /* create membufs */

  handle->chunk_ref_buf = MEMbufCreate();
  handle->fetch_ref_buf = MEMbufCreate();
  handle->fetch_chunk_buf = MEMbufCreate();
  handle->fetch_read_buf = MEMbufCreate();
  
  /* init ldata info */

  sprintf(pid_str, "spdb in pid %d", getpid());
  LDATA_init_handle(&handle->ldata, pid_str, FALSE);
  
  return (0);
  
}

/***********************
 * SPDB_close()
 *
 * Close files associated with an SPDB object
 */

void SPDB_close(spdb_handle_t *handle)
     
{
  close_files(handle);
  return;
}

/**********************
 * SPDB_free()
 *
 * Free an SPDB object
 */

void SPDB_free(spdb_handle_t *handle)
     
{

  if (handle->init == SPDB_INIT_FLAG) {
    close_files(handle);
    MEMbufDelete(handle->chunk_ref_buf);
    MEMbufDelete(handle->fetch_ref_buf);
    MEMbufDelete(handle->fetch_chunk_buf);
    MEMbufDelete(handle->fetch_read_buf);
    LDATA_free_handle(&handle->ldata);
  }

  return;

}

/*****************************************************
 * SPDB_fetch()
 *
 * Fetches the chunks stored at the requested time with the
 * given data type.  If the data type is 0, all chunks are
 * returned.
 *
 * Return args are:
 *  n_chunks - number of chunks found
 *  refs - chunks refs, give times, offset, len
 *  chunks - chunk buffer
 *
 * Note that returned pointers point to handle memory that
 * is overwritten by each call to this routine.  Do NOT
 * free these pointers.  Also, copy any data that may be
 * needed after the next call to this routine (or any of
 * the other fetch routines).
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_fetch(spdb_handle_t *handle,     /* I */
	       si32 data_type,            /* I */
	       si32 request_time,         /* I */
	       si32 *n_chunks_p,          /* O */
	       spdb_chunk_ref_t **refs_p, /* O */
	       void **chunks_p)           /* O */
     
{
  
  void *chunk;
  int ret;
  si32 i;
  si32 posn;
  si32 n_chunks = 0;
  spdb_chunk_ref_t *refs;
  spdb_chunk_ref_t ref;

  MEMbufReset(handle->fetch_ref_buf);
  MEMbufReset(handle->fetch_chunk_buf);
  MEMbufReset(handle->fetch_read_buf);

  *n_chunks_p = 0;
  *refs_p = NULL;
  *chunks_p = NULL;
  
  if (check_init(handle)) {
    return (-1);
  }

  /*
   * set up files
   */
  
  ret = setup_files(handle, request_time, SETUP_READ);

  if (ret == SETUP_NO_FILES) {
    return (0);
  } else if (ret == SETUP_FAILURE) {
    return (-1);
  }

  /*
   * get the indx posn for this time
   */
  
  if ((posn = posn_at_time(handle, request_time, data_type)) < 0) {
    clear_locks(handle);
    return (0);
  }

  for (i = posn; i < handle->hdr.n_chunks; i++) {
    
    if (handle->chunk_refs[i].valid_time == request_time) {
      
      if (data_type == 0 ||
	  data_type == handle->chunk_refs[i].data_type)
      {
	/*
	 * set the offset for the chunk reference in the
	 * buffer we are building
	 */

	ref = handle->chunk_refs[i];
	ref.offset = MEMbufLen(handle->fetch_chunk_buf);
      
	refs = MEMbufAdd(handle->fetch_ref_buf, (void *) &ref,
			 sizeof(spdb_chunk_ref_t));
      
	chunk = MEMbufPrepare(handle->fetch_read_buf, handle->chunk_refs[i].len);
      
	if (read_chunk(handle, handle->chunk_refs[i].offset,
		       chunk, handle->chunk_refs[i].len)) {
	  clear_locks(handle);
	  return (-1);
	}
      
	MEMbufConcat(handle->fetch_chunk_buf, handle->fetch_read_buf);

	n_chunks++;
      } /* endif - correct data type */
      
    } else {
      
	break;
      
    }

  } /* i */
  
  clear_locks(handle);

  /*
   * set return values
   */
  
  *n_chunks_p = n_chunks;
  if (n_chunks > 0) {
    *refs_p = refs;
    *chunks_p = MEMbufPtr(handle->fetch_chunk_buf);
  }

  return (0);

}

/*****************************************************
 * SPDB_fetch_closest()
 *
 * Fetches the chunks stored at the closest time 
 * to that requested, within the time margin with the
 * given data type.  If the data type is 0, all chunks are
 * returned. If time_margin is -1, all data is searched.
 * However, the search will not traverse days for which
 * no spdb file exists.
 *
 * Return args are:
 *  n_chunks - number of chunks found
 *  refs - chunks refs, give times, offset, len
 *  chunks - chunk buffer
 *
 * Note that returned pointers point to handle memory that
 * is overwritten by each call to this routine.  Do NOT
 * free these pointers.  Also, copy any data that may be
 * needed after the next call to this routine (or any of
 * the other fetch routines).
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_fetch_closest(spdb_handle_t *handle,     /* I */
		       si32 data_type,            /* I */
		       si32 request_time,         /* I */
		       si32 time_margin,          /* I */
		       si32 *n_chunks_p,          /* O */
		       spdb_chunk_ref_t **refs_p, /* O */
		       void **chunks_p)           /* O */
     
{

  int ret;
  si32 nearest_time;

  *n_chunks_p = 0;
  *refs_p = NULL;
  *chunks_p = NULL;
  
  if (check_init(handle)) {
    return (-1);
  }

  /*
   * set up files
   */
  
  ret = setup_files(handle, request_time, SETUP_READ);

  if (ret == SETUP_FAILURE) {
    return (-1);
  }

  /*
   * get the chunk position for the request time - if
   * negative there is no data exactly at the request time,
   * so get the nearest time.
   */
  
  if (posn_at_time(handle, request_time, data_type) >= 0) {

    /*
     * there is data exactly at the requested time
     */

    return (SPDB_fetch(handle, data_type, request_time,
		       n_chunks_p, refs_p, chunks_p));

  } else {

    /* 
     * search for closest time
     */

    if (get_time_nearest(handle, request_time, time_margin, data_type,
			 &nearest_time) != 0) {
      
      /*
       * There was no data in this time range.  This is okay,
       * just send back 0 chunks.
       */

      clear_locks(handle);

      *n_chunks_p = 0;
      *refs_p = NULL;
      *chunks_p = NULL;
      
      return (0);
      
    } else {
      
      return (SPDB_fetch(handle, data_type, nearest_time,
			 n_chunks_p, refs_p, chunks_p));
      
    }

  }

}

/*****************************************************
 * SPDB_fetch_first_before()
 *
 * Fetches the chunks stored at the first time at or
 * before that requested, within the time margin, with the
 * given data type.  If the data type is 0, all chunks are
 * returned. If time_margin is -1, all data is searched.
 * However, the search will not traverse days for which
 * no spdb file exists.
 *
 * Return args are:
 *  n_chunks - number of chunks found
 *  refs - chunks refs, give times, offset, len
 *  chunks - chunk buffer
 *
 * Note that returned pointers point to handle memory that
 * is overwritten by each call to this routine.  Do NOT
 * free these pointers.  Also, copy any data that may be
 * needed after the next call to this routine (or any of
 * the other fetch routines).
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_fetch_first_before(spdb_handle_t *handle,     /* I */
			    si32 data_type,            /* I */
			    si32 request_time,         /* I */
			    si32 time_margin,          /* I */
			    si32 *n_chunks_p,          /* O */
			    spdb_chunk_ref_t **refs_p, /* O */
			    void **chunks_p)           /* O */
     
{

  int ret;
  si32 nearest_time;
  si32 start_time;

  *n_chunks_p = 0;
  *refs_p = NULL;
  *chunks_p = NULL;
  
  if (check_init(handle)) {
    return (-1);
  }

  /*
   * set up files
   */
  
  ret = setup_files(handle, request_time, SETUP_READ);

  if (ret == SETUP_FAILURE) {
    return (-1);
  }

  /* 
   * search for first time before time, within time margin
   */

  if (time_margin >= 0) {
    start_time = request_time - time_margin;
  } else {
    start_time = 0;
  }

  if (first_time_before(handle,
			request_time,
			start_time,
			data_type,
			&nearest_time) != 0) {
      
    /*
     * There was no data in this time range.  This is okay,
     * just send back 0 chunks.
     */
    
    clear_locks(handle);
    
    *n_chunks_p = 0;
    *refs_p = NULL;
    *chunks_p = NULL;
    
    return (0);
    
  } else {
    
    return (SPDB_fetch(handle, data_type, nearest_time,
		       n_chunks_p, refs_p, chunks_p));
      
  }
  
}

/*****************************************************
 * SPDB_fetch_first_after()
 *
 * Fetches the chunks stored at the first time at or
 * after that requested, within the time margin, with the
 * given data type.  If the data type is 0, all chunks are
 * returned. If time_margin is -1, all data is searched.
 * However, the search will not traverse days for which
 * no spdb file exists.
 *
 * Return args are:
 *  n_chunks - number of chunks found
 *  refs - chunks refs, give times, offset, len
 *  chunks - chunk buffer
 *
 * Note that returned pointers point to handle memory that
 * is overwritten by each call to this routine.  Do NOT
 * free these pointers.  Also, copy any data that may be
 * needed after the next call to this routine (or any of
 * the other fetch routines).
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_fetch_first_after(spdb_handle_t *handle,     /* I */
			   si32 data_type,            /* I */
			   si32 request_time,         /* I */
			   si32 time_margin,          /* I */
			   si32 *n_chunks_p,          /* O */
			   spdb_chunk_ref_t **refs_p, /* O */
			   void **chunks_p)           /* O */
     
{

  int ret;
  si32 nearest_time;
  si32 end_time;
  
  *n_chunks_p = 0;
  *refs_p = NULL;
  *chunks_p = NULL;
  
  if (check_init(handle)) {
    return (-1);
  }

  /*
   * set up files
   */
  
  ret = setup_files(handle, request_time, SETUP_READ);

  if (ret == SETUP_FAILURE) {
    return (-1);
  }

  /* 
   * search for first time after time, within time margin
   */

  if (time_margin >= 0) {
    end_time = request_time + time_margin;
  } else {
    end_time = UNIX_TIME_MAX;
  }
  if (first_time_after(handle,
		       request_time,
		       end_time,
		       data_type,
		       &nearest_time) != 0) {
    
    /*
     * There was no data in this time range.  This is okay,
     * just send back 0 chunks.
     */
    
    clear_locks(handle);
    
    *n_chunks_p = 0;
    *refs_p = NULL;
    *chunks_p = NULL;
    
    return (0);
    
  } else {
    
    return (SPDB_fetch(handle, data_type, nearest_time,
		       n_chunks_p, refs_p, chunks_p));
      
  }
  
}

/*******************************************************
 * SPDB_fetch_interval()
 *
 * Fetches the chunks stored between the requested times with the
 * given data type.  If the data type is 0, all chunks are
 * returned.
 *
 * Return args are:
 *  n_chunks - number of chunks found
 *  refs - chunks refs, give times, offset, len
 *  chunks - chunk buffer
 *
 * Note that returned pointers point to handle memory that
 * is overwritten by each call to this routine.  Do NOT
 * free these pointers.  Also, copy any data that may be
 * needed after the next call to this routine (or any of
 * the other fetch routines).
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_fetch_interval(spdb_handle_t *handle,     /* I */
			si32 data_type,            /* I */
			si32 start_time,           /* I */
			si32 end_time,             /* I */
			si32 *n_chunks_p,          /* O */
			spdb_chunk_ref_t **refs_p, /* O */
			void **chunks_p)           /* O */
     
{
  
  char *chunk;
  int ret;
  si32 i;
  si32 posn;
  si32 file_start_time;
  si32 search_start = start_time;
  si32 search_end = end_time;
  si32 first_time, last_time;
  si32 n_chunks = 0;
  spdb_chunk_ref_t *refs;
  spdb_chunk_ref_t ref;

  MEMbufReset(handle->fetch_ref_buf);
  MEMbufReset(handle->fetch_chunk_buf);
  MEMbufReset(handle->fetch_read_buf);

  *n_chunks_p = 0;
  *refs_p = NULL;
  *chunks_p = NULL;

  if (check_init(handle)) {
    return (-1);
  }

  /*
   * constrain request using available data times
   */

  if (SPDB_first_and_last_times(handle, &first_time, &last_time) == 0) {
    search_start = MAX(search_start, first_time);
    search_end = MIN(search_end, last_time);
  }
  file_start_time = (search_start / SECS_IN_DAY) * SECS_IN_DAY;
  
  while (file_start_time <= search_end) {

    /*
     * set up files
     */

    ret = setup_files(handle, file_start_time, SETUP_READ);

    if (ret == SETUP_FAILURE) {
      return (-1);
    } else if (ret == SETUP_NO_FILES) {
      file_start_time += SECS_IN_DAY;
      continue;
    }
    
    /*
     * get the first indx posn at or after start time
     */
    
    posn = first_posn_after(handle, search_start);

    if (posn >= 0) {

      for (i = posn; i < handle->hdr.n_chunks; i++) {
      
	if (handle->chunk_refs[i].valid_time <= search_end) {

	  if (data_type == 0 ||
	      data_type == handle->chunk_refs[i].data_type)
	  {
	    /*
	     * set the offset for the chunk reference in the
	     * buffer we are building
	     */

	    ref = handle->chunk_refs[i];
	    ref.offset = MEMbufLen(handle->fetch_chunk_buf);
	  
	    refs = MEMbufAdd(handle->fetch_ref_buf, (void *) &ref,
			     sizeof(spdb_chunk_ref_t));
	  
	    chunk = (void *) MEMbufPrepare(handle->fetch_read_buf,
					   handle->chunk_refs[i].len);
	  
	    if (read_chunk(handle,
			   handle->chunk_refs[i].offset,
			   (void *) chunk,
			   handle->chunk_refs[i].len)) {
	      clear_locks(handle);
	      return (-1);
	    }
	  
	    MEMbufConcat(handle->fetch_chunk_buf, handle->fetch_read_buf);
	    
	    n_chunks++;
	  } /* endif - correct data type */
	  
	} else {
	  
	  break;
	  
	} /* if (handle->chunk_refs[i].valid_time <= search_end) */

      } /* i */

    } /* if (posn >= 0) */

    /*
     * move ahead by 1 day
     */

    file_start_time = handle->hdr.start_of_day + SECS_IN_DAY;
    search_start = file_start_time;

  } /* while */
  
  clear_locks(handle);

  /*
   * set return values
   */
  
  *n_chunks_p = n_chunks;
  if (n_chunks > 0) {
    *refs_p = refs;
    *chunks_p = MEMbufPtr(handle->fetch_chunk_buf);
  }
     
  return (0);

}

/*******************************************************
 * SPDB_fetch_valid()
 *
 * Fetches the chunks valid at a given time with the
 * given data type.  If the data type is 0, all chunks are
 * returned.
 *
 * Return args are:
 *  n_chunks - number of chunks found
 *  refs - chunks refs, give times, offset, len
 *  chunks - chunk buffer
 *
 * Note that returned pointers point to handle memory that
 * is overwritten by each call to this routine.  Do NOT
 * free these pointers.  Also, copy any data that may be
 * needed after the next call to this routine (or any of
 * the other fetch routines).
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_fetch_valid(spdb_handle_t *handle,     /* I */
		     si32 data_type,            /* I */
		     si32 search_time,          /* I */
		     si32 *n_chunks_p,          /* O */
		     spdb_chunk_ref_t **refs_p, /* O */
		     void **chunks_p)           /* O */
     
{
  
  char *chunk;
  int ret;
  si32 i;
  si32 posn;
  si32 file_start_time;
  si32 first_time, last_time;
  si32 search_start, search_end;
  si32 n_chunks = 0;
  spdb_chunk_ref_t *refs;
  spdb_chunk_ref_t ref;

  MEMbufReset(handle->fetch_ref_buf);
  MEMbufReset(handle->fetch_chunk_buf);
  MEMbufReset(handle->fetch_read_buf);

  *n_chunks_p = 0;
  *refs_p = NULL;
  *chunks_p = NULL;

  if (check_init(handle)) {
    return (-1);
  }

  /*
   * read file header for search time, set start and end times
   * for search
   */
  
  if (setup_files(handle, search_time, SETUP_READ)) {
    clear_locks(handle);
    return (0);
  }
  search_start = handle->hdr.earliest_valid;
  search_end = search_time;

  /*
   * constrain request using available data times
   */

  if (SPDB_first_and_last_times(handle, &first_time, &last_time)) {
    clear_locks(handle);
    return (0);
  }

  search_start = MAX(search_start, first_time);
  search_end = MIN(search_end, last_time);
  file_start_time = (search_start / SECS_IN_DAY) * SECS_IN_DAY;

  while (file_start_time <= search_end) {

    /*
     * set up files
     */

    ret = setup_files(handle, file_start_time, SETUP_READ);

    if (ret == SETUP_FAILURE) {
      return (-1);
    } else if (ret == SETUP_NO_FILES) {
      file_start_time += SECS_IN_DAY;
      continue;
    }
    
    /*
     * get the first indx posn at or after start time
     */
    
    posn = first_posn_after(handle, search_start);

    if (posn >= 0) {

      for (i = posn; i < handle->hdr.n_chunks; i++) {
      
	/*
	 * check if chunk is valid at search time
	 */
	
	if (handle->chunk_refs[i].valid_time <= search_time &&
	    handle->chunk_refs[i].expire_time >= search_time) {

	  if (data_type == 0 ||
	      data_type == handle->chunk_refs[i].data_type)
	  {
	    /*
	     * set the offset for the chunk reference in the
	     * buffer we are building
	     */

	    ref = handle->chunk_refs[i];
	    ref.offset = MEMbufLen(handle->fetch_chunk_buf);
	  
	    refs = MEMbufAdd(handle->fetch_ref_buf, (void *) &ref,
			     sizeof(spdb_chunk_ref_t));
	  
	    chunk = (void *) MEMbufPrepare(handle->fetch_read_buf,
					   handle->chunk_refs[i].len);
	  
	    if (read_chunk(handle,
			   handle->chunk_refs[i].offset,
			   (void *) chunk,
			   handle->chunk_refs[i].len)) {
	      clear_locks(handle);
	      return (-1);
	    }
	  
	    MEMbufConcat(handle->fetch_chunk_buf, handle->fetch_read_buf);

	    n_chunks++;
	  } /* endif - correct data type */
	  
	} /* if (handle->chunk_refs[i].valid_time <= search_end) */

      } /* i */

    } /* if (posn >= 0) */

    /*
     * move ahead by 1 day
     */

    file_start_time = handle->hdr.start_of_day + SECS_IN_DAY;
    search_start = file_start_time;

  } /* while */
  
  clear_locks(handle);

  /*
   * set return values
   */
  
  *n_chunks_p = n_chunks;
  if (n_chunks > 0) {
    *refs_p = refs;
    *chunks_p = MEMbufPtr(handle->fetch_chunk_buf);
  }
     
  return (0);

}

/*******************************************************
 * SPDB_chunk_data_len()
 *
 * Returns the length of the chunk data buffer after a fetch.
 */

int SPDB_chunk_data_len(spdb_handle_t *handle)

{
  return (MEMbufLen(handle->fetch_chunk_buf));
}
     
/*****************************************************
 * SPDB_print_header()
 *
 * Prints the header file for a given time
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_print_header(spdb_handle_t *handle,     /* I */
		      si32 request_time,         /* I */
		      FILE *out)                 /* I */
     
{
  
  int i;
  spdb_chunk_ref_t *refs;

  if (check_init(handle)) {
    return (-1);
  }

  /*
   * set up files
   */
  
  if (setup_files(handle, request_time, SETUP_READ)) {
    fprintf(stderr, "ERROR - SPDB_print_header, db_dir %s\n",
	    handle->dir);
    fprintf(stderr, "Cannot open file for time %s\n",
	    UTIMstr(request_time));
    return (-1);
  }

  fprintf(out, "\n");
  fprintf(out, "SPDB FILE HEADER\n");
  fprintf(out, "\n");
  fprintf(out, "dir : %s\n", handle->dir);
  fprintf(out, "indx_path : %s\n", handle->indx_path);
  fprintf(out, "data_path : %s\n", handle->data_path);
  fprintf(out, "\n");

  SPDB_print_index_header(&handle->hdr, out, TRUE);
  
  fprintf(out, "\n");
  fprintf(out, "Chunk reference array:\n");
  fprintf(out, "\n");
  fprintf(out, "  %8s %20s %20s %20s %8s",
	  "Chunk", "Data Type", "Valid", "Expire", "Len");
  fprintf(out, "\n");

  refs = handle->chunk_refs;
  
  for (i = 0; i < handle->hdr.n_chunks; i++, refs++) {
    fprintf(out, "  %8d %20d %20s %20s %8d\n", i,
	    refs->data_type,
	    UTIMstr(refs->valid_time),
	    UTIMstr(refs->expire_time),
	    refs->len);
  }

  return (0);

}

/*****************************************************
 * SPDB_print_index_header()
 *
 * Prints the header structure
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_print_index_header(spdb_hdr_t *spdb_hdr,      /* I */
			    FILE *out,                 /* I */
			    int print_min_posn)        /* I */
     
{
  int i;

  fprintf(out, "prod_label : %s\n", spdb_hdr->prod_label);
  fprintf(out, "major_version : %d\n", spdb_hdr->major_version);
  fprintf(out, "minor_version : %d\n", spdb_hdr->minor_version);
  fprintf(out, "prod_id : %d\n", spdb_hdr->prod_id);
  fprintf(out, "n_chunks : %d\n", spdb_hdr->n_chunks);
  fprintf(out, "nbytes_frag : %d\n", spdb_hdr->nbytes_frag);
  fprintf(out, "nbytes_data : %d\n", spdb_hdr->nbytes_data);
  fprintf(out, "max_duration : %d\n", spdb_hdr->max_duration);
  fprintf(out, "start_of_day : %s\n", UTIMstr(spdb_hdr->start_of_day));
  fprintf(out, "end_of_day : %s\n", UTIMstr(spdb_hdr->end_of_day));
  fprintf(out, "start_valid : %s\n", UTIMstr(spdb_hdr->start_valid));
  fprintf(out, "end_valid : %s\n", UTIMstr(spdb_hdr->end_valid));
  fprintf(out, "latest_expire : %s\n", UTIMstr(spdb_hdr->latest_expire));
  fprintf(out, "earliest_valid : %s\n", UTIMstr(spdb_hdr->earliest_valid));
  fprintf(out, "\n");

  if (print_min_posn)
  {
    fprintf(out, "Minute position array:\n");
    fprintf(out, "\n");
    fprintf(out, "  %8s %8s", "Minute", "Posn");
    fprintf(out, "\n");

    for (i = 0; i < MINS_IN_DAY; i++) {
      if (spdb_hdr->minute_posn[i] >= 0) {
	fprintf(out, "  %8d %8d\n", i,
		spdb_hdr->minute_posn[i]);
      }
    }
  }
  
  fprintf(out, "\n");

  return (0);

}

/*****************************************************
 * SPDB_store()
 *
 * Stores a chunk in the data base, without overwrite.
 *
 * Returns 0 on success, -1 on failure,
 * or if chunk is already stored at this time.
 */

int SPDB_store(spdb_handle_t *handle,
	       si32 data_type,
	       si32 valid_time, si32 expire_time,
	       void *chunk, int len)
     
{

  if (check_init(handle)) {
    return (-1);
  }

  return (store_chunk(handle, data_type,
		      valid_time, expire_time,
		      chunk, len, STORE_ONCE));

}

/*****************************************************
 * SPDB_store_add()
 *
 * Stores a chunk in the data base, adding chunks at
 * the given time, without overwrite.
 *
 * Returns 0 on success, -1 on failure.
 */

int SPDB_store_add(spdb_handle_t *handle,
		   si32 data_type,
		   si32 valid_time, si32 expire_time,
		   void *chunk, int len)
     
{

  if (check_init(handle)) {
    return (-1);
  }

  return (store_chunk(handle, data_type,
		      valid_time, expire_time,
		      chunk, len, STORE_ADD));

}

/*****************************************************
 * SPDB_store_over()
 *
 * Stores a chunk in the data base, with overwrite.
 * Overwrite occurs if data is already stored at the
 * valid time.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_store_over(spdb_handle_t *handle,
		    si32 data_type,
		    si32 valid_time, si32 expire_time,
		    void *chunk, int len)
     
{

  if (check_init(handle)) {
    return (-1);
  }

  return (store_chunk(handle, data_type,
		      valid_time, expire_time,
		      chunk, len, STORE_OVERWRITE));

}

/*******************************************************
 * SPDB_first_and_last_times()
 *
 * Gets first and last times, if there is data in the
 * data base.
 *
 * Returns 0 on success, -1 on failure (i.e.no data)
 */

int SPDB_first_and_last_times(spdb_handle_t *handle,     /* I */
			      si32 *first_time,          /* O */
			      si32 *last_time)           /* O */

{

  char **file_names;
  int error_flag = FALSE;
  int ifile;
  si32 nfiles, nentries;
  long year, month, day;
  UTIMstruct first, last;
  DIR *dirp;
  struct dirent	*dp = NULL;

  /*
   * open data directory file for reading
   */
  
  if ((dirp = opendir (handle->dir)) == NULL) {
    return (-1);
  }

  /*
   * read through the directory to get the number of entries
   */
  
  nentries = 0;
  
  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {
    nentries++;
  } /* dp */
  
  /*
   * allocate space for the file name array
   */
  
  file_names = (char **) umalloc((ui32) (nentries * sizeof(char *)));
  
  /*
   * read through the directory again to get the valid file names
   */
  
  rewinddir(dirp);
  
  nfiles = 0;
  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {

    /*
     * exclude dir entries and files beginning with '.'
     */

    if (dp->d_name[0] == '.') {
      continue;
    }

    /*
     * check for the indx extension
     */

    if (strstr(dp->d_name, SPDB_INDX_EXT) == NULL) {
      continue;
    }
    
    /*
     * check that the file name is in the correct format
     */

    if (sscanf(dp->d_name, "%4ld%2ld%2ld",
	       &year, &month, &day) != 3) {
      continue;
    }
      
    if (year < 1970 || year > 2100 || month < 1 || month > 12 ||
	day < 1 || day > 31) {
      continue;
    }

    /*
     * file name is in correct format. Therefore, accept it
     */
    
    file_names[nfiles] = STRdup(dp->d_name);
      
    nfiles++;
    
  } /* readdir() */

  /*
   * close the directory file
   */

  closedir(dirp);

  /*
   * if there are no files, return now
   */

  if (nfiles == 0) {
    ufree((char *) file_names);
    return (-1);
  }

  /*
   * sort the file names
   */
	
  qsort((char *) file_names, (int) nfiles, sizeof(char *), str_compare);

  /*
   * get first time from first file
   */

  sscanf(file_names[0], "%4ld%2ld%2ld",
	 &first.year, &first.month, &first.day);
  first.hour = 12;
  first.min = 0;
  first.sec = 0;
  UTIMdate_to_unix(&first);
  
  if (setup_files(handle, first.unix_time, SETUP_READ)) {
    error_flag = TRUE;
  } else {
    first.unix_time = handle->hdr.start_valid;
  }

  /*
   * get last time from last file
   */

  sscanf(file_names[nfiles - 1], "%4ld%2ld%2ld",
	 &last.year, &last.month, &last.day);
  last.hour = 12;
  last.min = 0;
  last.sec = 0;
  UTIMdate_to_unix(&last);
  
  if (setup_files(handle, last.unix_time, SETUP_READ)) {
    error_flag = TRUE;
  } else {
    last.unix_time = handle->hdr.latest_expire;
  }

  /*
   * free up resources
   */
  
  clear_locks(handle);
  
  for (ifile = 0; ifile < nfiles; ifile++) {
    STRfree((char *) file_names[ifile]);
  }
  ufree((char *) file_names);

  /*
   * set return values
   */

  if (error_flag) {
    return (-1);
  } else {
    *first_time = first.unix_time;
    *last_time = last.unix_time;
    return (0);
  }
  
}
  

/*******************************************************
 * SPDB_last_valid_time()
 *
 * Gets the last data valid time, if there is data in the
 * data base.
 *
 * Returns 0 on success, -1 on failure (i.e.no data)
 */

int SPDB_last_valid_time(spdb_handle_t *handle,     /* I */
			 si32 *last_valid_time)     /* O */

{

  /*
   * Get the latest data time from the LDATA info file.
   */

  if (LDATA_info_read(&handle->ldata,
		      handle->dir, -1) != 0)
    return(-1);
  
  *last_valid_time = handle->ldata.info.latest_time;
  return(0);
  
}
  

/*****************************************************
 * SPDB_last_unique()
 *
 * Returns a buffer of chunks which includes only the chunks
 * with unique data_types with the latest valid times.  Note
 * that all chunks in the original buffer with a data_type of
 * 0 will be returned in the returned buffer.
 *
 * NOTE1:  The incoming buffer is assumed to be in chronological
 *         order, which is the order returned by all of the
 *         SPDB_fetch and SPDB_get routines.
 *
 * NOTE2: this is NOT THREAD SAFE.
 * Use class Spdb for a thread-safe version.
 */

void SPDB_last_unique(si32 nchunks,                         /* I */
		      spdb_chunk_ref_t *chunk_hdrs,         /* I */
		      void *chunk_data,                     /* I */
		      si32 *nchunks_unique,                 /* O */
		      spdb_chunk_ref_t **chunk_hdrs_unique, /* 0 */
		      void **chunk_data_unique)             /* O */
{

  static int *unique_flags = NULL;
  static int num_unique_flags = 0;
  
  static MEMbuf *local_hdrs = NULL;
  static MEMbuf *local_data = NULL;

  int num_unique = 0;
  int i, j;
  int unique_offset;
  
  /*
   * Allocate space for the local unique_flags array.
   */

  if (num_unique_flags < nchunks)
  {
    num_unique_flags = nchunks;
    
    if (unique_flags == (int *)NULL)
      unique_flags = (int *)umalloc(num_unique_flags * sizeof(int));
    else
      unique_flags = (int *)urealloc(unique_flags, num_unique_flags * sizeof(int));
  }
  
  /*
   * Initialize the memory buffers.
   */

  if (local_hdrs == (MEMbuf *)NULL)
    local_hdrs = MEMbufCreate();
  else
    MEMbufReset(local_hdrs);
  
  if (local_data == (MEMbuf *)NULL)
    local_data = MEMbufCreate();
  else
    MEMbufReset(local_data);
  
  /*
   * Loop through the incoming chunks to find the unique ones.
   */

  for (i = 0; i < nchunks; i++)
  {
    /*
     * Initialize the unique flag.
     */

    unique_flags[i] = TRUE;

    /*
     * Since the chunks arrive in chronological order, a chunk
     * is unique if there is no following chunk with the same
     * data_type (or if the chunk's data_type is 0).
     */

    if (chunk_hdrs[i].data_type == 0)
      continue;
    
    for (j = i + 1; j < nchunks; j++)
    {
      if (chunk_hdrs[i].data_type == chunk_hdrs[j].data_type)
      {
	unique_flags[i] = FALSE;
	break;
      }
    } /* endfor - j */
    
  } /* endfor - i */
  
  /*
   * Accumulate the unique chunks into the MEMbufs.
   */

  unique_offset = 0;
  
  for (i = 0; i < nchunks; i++)
  {
    if (unique_flags[i])
    {
      spdb_chunk_ref_t unique_hdr;
      
      num_unique++;
      
      unique_hdr = chunk_hdrs[i];
      unique_hdr.offset = unique_offset;
      unique_offset += chunk_hdrs[i].len;
      
      MEMbufAdd(local_hdrs, &unique_hdr, sizeof(spdb_chunk_ref_t));
      MEMbufAdd(local_data,
		(void *)((char *)chunk_data + chunk_hdrs[i].offset),
		chunk_hdrs[i].len);
    }
    
  } /* endfor - i */
  
  /*
   * Set the return values.
   */

  *nchunks_unique = num_unique;
  *chunk_hdrs_unique = (spdb_chunk_ref_t *)MEMbufPtr(local_hdrs);
  *chunk_data_unique = (void *)MEMbufPtr(local_data);
  
  return;
}

/*****************************************************
 * SPDB_print_chunk_ref()
 *
 * Prints the given chunk reference structure
 */

void SPDB_print_chunk_ref(spdb_chunk_ref_t *chunk_ref,   /* I */
			  FILE *out)                     /* I */
     
{
  fprintf(out, "\n");
  fprintf(out, "SPDB Chunk Reference\n");
  fprintf(out, "\n");
  fprintf(out, "valid_time : %s\n", UTIMstr(chunk_ref->valid_time));
  fprintf(out, "expire_time : %s\n", UTIMstr(chunk_ref->expire_time));
  fprintf(out, "data_type : %d\n", chunk_ref->data_type);
  fprintf(out, "offset : %d\n", chunk_ref->offset);
  fprintf(out, "len : %d\n", chunk_ref->len);
  fprintf(out, "\n");

  return;
}

/*****************
 * add_chunk_ref()
 *
 * returns posn of chunk in ref array, -1 on error.
 */

static int add_chunk_ref(spdb_handle_t *handle,
			 si32 data_type,
			 si32 valid_time,
			 si32 expire_time,
			 si32 offset,
			 si32 len)

{

  int i;
  int latest;
  si32 store_posn;
  si32 vmin;
  si32 *min_posn;
  spdb_chunk_ref_t *ref, *ref2;
  
  if (!handle->active) {
    fprintf(stderr, "ERROR - SPDB:add_chunk_ref\n");
    fprintf(stderr, "Handle not active\n");
    return (-1);
  }

  /*
   * alloc space
   */

  if (alloc_chunk_refs(handle, handle->hdr.n_chunks + 1)) {
    return (-1);
  }

  /*
   * find posn at which to store the chunk - they
   * are stored in valid_time order
   */
  
  ref = handle->chunk_refs + (handle->hdr.n_chunks - 1);
  for (i = handle->hdr.n_chunks - 1; i >= 0; i--, ref--) {
    if (valid_time >= ref->valid_time) {
      break;
    }
  } /* i */
  store_posn = i + 1;
  if (store_posn == handle->hdr.n_chunks) {
    latest = TRUE;
  } else {
    latest = FALSE;
  }

  /*
   * move the entries below down one slot
   */

  if (!latest) {

    ref = handle->chunk_refs + handle->hdr.n_chunks - 1;
    ref2 = ref + 1;
    for (i = handle->hdr.n_chunks; i > store_posn; i--, ref--, ref2--) {
      *ref2 = *ref;
    }

  } /* if (!latest) */

  /*
   * store the chunk ref in the array
   */

  ref = handle->chunk_refs + store_posn;
  ref->data_type = data_type;
  ref->valid_time = valid_time;
  ref->expire_time = expire_time;
  ref->prod_id = handle->hdr.prod_id;
  ref->offset = offset;
  ref->len = len;
  
  /*
   * increment the number of chunks and nbytes_data
   */

  handle->hdr.n_chunks++;
  handle->hdr.nbytes_data += len;

  /*
   * amend the minute posn array
   */

  vmin = (valid_time % SECS_IN_DAY) / SECS_IN_MIN;
  min_posn = handle->hdr.minute_posn + vmin;

  if (*min_posn == -1) {
    *min_posn = store_posn;
  } else if (*min_posn > store_posn) {
    *min_posn = store_posn;
  }

  if (!latest) {
    min_posn = handle->hdr.minute_posn + vmin + 1;
    for (i = vmin + 1; i < MINS_IN_DAY; i++, min_posn++) {
      if (*min_posn != -1) {
	*min_posn = *min_posn + 1;
      }
    } /* i */
  } /* if (!latest) */

  return (store_posn);
  
}

/**********************************************************
 * alloc_chunk_refs()
 *
 * Alloc space for chunk ref buffer, set pointer
 *
 * Returns 0 on success, -1 on failure.
 */

static int alloc_chunk_refs(spdb_handle_t *handle,
			    si32 n_chunks)

{

  handle->chunk_refs = (spdb_chunk_ref_t *)
    MEMbufAlloc(handle->chunk_ref_buf,
		n_chunks * sizeof(spdb_chunk_ref_t));

  return (0);
  
}

/*********************************************
 * check_init()
 *
 * Checks that the init was done.
 *
 * Returns 0 on success, -1 on failure
 */

static int check_init(spdb_handle_t *handle)

{
  if (handle->init == SPDB_INIT_FLAG) {
    return (0);
  } else {
    fprintf(stderr, "ERROR - SPDB: handle not initialized\n");
    return (-1);
  }
}

/*********************************************
 * clear_locks()
 *
 * Clear locks on files
 *
 * Returns 0 on success, -1 on failure.
 */

static int clear_locks(spdb_handle_t *handle)

{

  struct flock lock;

  if (handle->locked) {

    lock.l_type = F_UNLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    
    if (fcntl(handle->indx_fd, F_SETLKW, &lock) == -1) {
      fprintf(stderr, "ERROR - SPDB:clear_locks\n");
      perror(handle->indx_path);
      /* return (-1);  Tmp fix for no-compliant xmounts - dixon */
    }
    
    if (fcntl(handle->data_fd, F_SETLKW, &lock) == -1) {
      fprintf(stderr, "ERROR - SPDB:clear_locks\n");
      perror(handle->data_path);
      /* return (-1);  Tmp fix for no-compliant xmounts - dixon */
    }
    
    handle->locked = FALSE;

  }

  return (0);

}

/***************
 * close_files()
 */

static void close_files(spdb_handle_t *handle)

{

  clear_locks(handle);

  if (handle->indx_file != NULL) {
    fclose(handle->indx_file);
    handle->indx_file = NULL;
  }

  if (handle->data_file != NULL) {
    fclose(handle->data_file);
    handle->data_file = NULL;
  }

  handle->active = FALSE;

  return;

}

/************************************************
 * first_posn_after()
 *
 * Returns the first chunk ref posn at or after the
 * requested time, on the given day.
 *
 * Returns -1 if no chunk has been stored at or after
 * this time.
 */

static si32 first_posn_after(spdb_handle_t *handle,
			     si32 start_time)
     
{

  si32 i;
  si32 smin;
  si32 start_posn;
  si32 *min_posn;
  spdb_chunk_ref_t *ref;
  
  if (!handle->active) {
    return (-1);
  }

  /* constrain start time */

  if (start_time < (time_t) handle->hdr.start_of_day) {
    start_time = handle->hdr.start_of_day;
  } else if (start_time > (time_t) handle->hdr.end_of_day) {
    start_time = handle->hdr.end_of_day;
  }

  smin = (start_time % SECS_IN_DAY) / SECS_IN_MIN;

  start_posn = -1;
  min_posn = handle->hdr.minute_posn + smin;
  while (start_posn < 0 && smin < MINS_IN_DAY) {
    start_posn = *min_posn;
    smin++;
    min_posn++;
  }

  if (start_posn < 0) {
    /*
     * no data found at or after the start time
     */
    return(-1);
  }
  
  ref = handle->chunk_refs + start_posn;

  for (i = start_posn; i < handle->hdr.n_chunks; i++, ref++) {
    if (ref->valid_time >= start_time) {
      return (i);
    }
  } /* i */

  return (-1);

}

/************************************************
 * first_time_after()
 *
 * Returns the first chunk time at or after the
 * requested time, and before the end_time.
 *
 * If the search goes beyond the day for the current
 * handle, this routine calls itself recursively
 * for the following day.
 *
 * Returns 0 on success, -1 on failure
 */

static si32 first_time_after(spdb_handle_t *handle,  /* I */
			     si32 request_time,      /* I */
			     si32 end_time,          /* I */
			     si32 data_type,         /* I */
			     si32 *data_time)        /* O */
     
{

  si32 i;
  si32 rmin, emin;
  si32 posn;
  si32 *min_posn;
  si32 time_start_next_day;
  spdb_handle_t next_day_handle;
  spdb_chunk_ref_t *ref;
  UTIMstruct request_time_struct;
  
  if (handle->active) {
    
    /*
     * Find the first minute value for which there is data.
     */

    rmin = (request_time % SECS_IN_DAY) / SECS_IN_MIN;
    emin = rmin + ((end_time - request_time) / SECS_IN_MIN) + 1;
    emin = MAX(emin, MINS_IN_DAY - 1);
    
    posn = -1;
    min_posn = handle->hdr.minute_posn + rmin;
    while (posn < 0 && rmin <= emin) {
      posn = *min_posn;
      rmin++;
      min_posn++;
    }
    
    /*
     * Check the valid time for the found data.  Look for the
     * first data with an appropriate valid time.  We must do
     * this because the data found above could be earlier than
     * the requested time by a few seconds.  (e.g. data for
     * 13:31:15 when requesting data after 13:31:30, both appear
     * under minute 13:31).  Also make sure we find the appropriate
     * data_type.
     */

    if (posn >= 0) {
      
      ref = handle->chunk_refs + posn;
      for (i = posn; i < handle->hdr.n_chunks; i++, ref++) {
	if (ref->valid_time >= request_time &&
	    ref->valid_time <= end_time &&
	    (data_type == 0 ||
	     data_type == ref->data_type)) {
	  *data_time = ref->valid_time;
	  return (0);
	}
	else if (ref->valid_time > end_time)
	  return(-1);
      } /* i */
      
    }

  } /* if (handle->active) */

  /*
   * no valid time found after the request time - check if
   * the end_time goes to next day. If so, call recursively.
   */

  UTIMunix_to_date(request_time, &request_time_struct);
  
  request_time_struct.hour = 23;
  request_time_struct.min = 59;
  request_time_struct.sec = 59;
  
  time_start_next_day = UTIMdate_to_unix(&request_time_struct) + 1;
  
  if (end_time >= time_start_next_day) {

    SPDB_init(&next_day_handle, handle->hdr.prod_label,
	      handle->hdr.prod_id, handle->dir);
    
    if (setup_files(&next_day_handle, time_start_next_day, SETUP_READ)
	!= SETUP_SUCCESS) {
      SPDB_free(&next_day_handle);
      close_files(&next_day_handle);
      return (-1);
    }

    if (first_time_after(&next_day_handle, time_start_next_day,
			 end_time, data_type, data_time) != 0) {
      SPDB_free(&next_day_handle);
      close_files(&next_day_handle);
      return (-1);
    } else {
      SPDB_free(&next_day_handle);
      close_files(&next_day_handle);
      return (0);
    }

  } else {

    return (-1);

  }
    
}

/************************************************
 * first_time_before()
 *
 * Returns the first chunk time at or before the
 * requested time, and after the start_time.
 *
 * If the search goes before the day for the current
 * handle, this routine calls itself recursively
 * for the previous day.
 *
 * Returns 0 on success, -1 on failure
 */

static si32 first_time_before(spdb_handle_t *handle,  /* I */
			      si32 request_time,      /* I */
			      si32 start_time,        /* I */
			      si32 data_type,         /* I */
			      si32 *data_time)        /* O */
     
{

  si32 i;
  si32 rmin, smin;
  si32 posn, posn_ahead;
  si32 time_end_prev_day;
  si32 *min_posn;
  spdb_handle_t prev_day_handle;
  spdb_chunk_ref_t *ref;
  UTIMstruct request_time_struct;
  
  if (handle->active) {

    /*
     * Find the first minute value for which there is data.
     */

    rmin = ((request_time % SECS_IN_DAY) / SECS_IN_MIN);
    smin = (rmin - ((request_time - start_time) / SECS_IN_MIN)) - 1;
    smin = MAX(smin, 0);
    
    posn = -1;
    min_posn = handle->hdr.minute_posn + rmin;
    while (posn < 0 && rmin >= smin) {
      posn = *min_posn;
      rmin--;
      min_posn--;
    }

    /*
     * Check the valid time for the found data.  Look for the
     * first data with an appropriate valid time.  We must do
     * this because there could be data later than the data
     * found above, but still earlier than the requested time
     * (e.g. data existing for 13:31:15 and 13:31:30 when
     * requesting data before 13:31:45, all appear under
     * minute 13:31).  Also, make sure we find the appropriate
     * data type.
     */

    if (posn >= 0) {
      
      if ((posn_ahead = posn_1min_ahead(handle, posn)) < 0) {
	return (-1);
      }
      
      ref = handle->chunk_refs + posn_ahead;
      for (i = posn_ahead; i >= 0; i--, ref--) {
	if (ref->valid_time <= request_time &&
	    ref->valid_time >= start_time &&
	    (data_type == 0 ||
	     data_type == ref->data_type)) {
	  *data_time = ref->valid_time;
	  return (0);
	}
	else if (ref->valid_time < start_time)
	  return(-1);
      } /* i */
      
    }

  } /* if (handle->active) */

  /*
   * no valid time found before the request time - check if
   * the start_time goes to prev day. If so, call recursively.
   */

  UTIMunix_to_date(request_time, &request_time_struct);
  
  request_time_struct.hour = 0;
  request_time_struct.min = 0;
  request_time_struct.sec = 0;
  
  time_end_prev_day = UTIMdate_to_unix(&request_time_struct) - 1;
  
  if (start_time <= time_end_prev_day) {

    SPDB_init(&prev_day_handle, handle->hdr.prod_label,
	      handle->hdr.prod_id, handle->dir);
    
    if (setup_files(&prev_day_handle, time_end_prev_day, SETUP_READ)
	!= SETUP_SUCCESS) {
      SPDB_free(&prev_day_handle);
      close_files(&prev_day_handle);
      return (-1);
    }

    if (first_time_before(&prev_day_handle, time_end_prev_day,
			  start_time, data_type, data_time) != 0) {
      SPDB_free(&prev_day_handle);
      close_files(&prev_day_handle);
      return (-1);
    } else {
      SPDB_free(&prev_day_handle);
      close_files(&prev_day_handle);
      return (0);
    }
    
  } else {

    return (-1);

  }
    
}

/*****************************************************
 * get_time_nearest()
 *
 * Gets the nearest time with data within the time_margin.
 *
 * Returns 0 on success, -1 on failure
 */

static int get_time_nearest(spdb_handle_t *handle,     /* I */
			    si32 request_time,         /* I */
			    si32 time_margin,          /* I */
			    si32 data_type,            /* I */
			    si32 *time_nearest)        /* O */
     
{
  
  int before_found, after_found;
  si32 nearest_before, nearest_after;
  si32 start_time, end_time;
  
  if (time_margin >= 0) {
    end_time = request_time + time_margin;
  } else {
    end_time = UNIX_TIME_MAX;
  }
  if (first_time_after(handle, request_time,
		       end_time, data_type,
		       &nearest_after) == 0) {
    after_found = TRUE;
  } else {
    after_found = FALSE;
  }
    
  if (time_margin >= 0) {
    start_time = request_time - time_margin;
  } else {
    start_time = 0;
  }
  if (first_time_before(handle, request_time,
			start_time, data_type,
			&nearest_before) == 0) {
    before_found = TRUE;
  } else {
    before_found = FALSE;
  }
    
  if (before_found && after_found) {

    if ((nearest_after - request_time) <
	(request_time - nearest_before)) {
      *time_nearest = nearest_after;
    } else {
      *time_nearest = nearest_before;
    }
    
  } else if (before_found) {
    
    *time_nearest = nearest_before;
    
  } else if (after_found) {
    
    *time_nearest = nearest_after;
    
  } else {
    
    return (-1);
    
  }
  
  return (0);

}

/*********************************
 * posn_1min_ahead()
 *
 * Return posn ahead by 1 minute, or
 * the end of the array if that is less than
 * 1 min ahead.
 *
 * Returns -1 on error.
 */

static si32 posn_1min_ahead(spdb_handle_t *handle,
			    si32 start_posn)

{

  int i;
  si32 start_time, target_time;

  if (!handle->active) {
    return (-1);
  }

  start_time = handle->chunk_refs[start_posn].valid_time;
  target_time = start_time + SECS_IN_MIN;

  for (i = start_posn + 1; i < handle->hdr.n_chunks; i++) {
    if (handle->chunk_refs[i].valid_time >= target_time) {
      return (i);
    }
  }

  return (handle->hdr.n_chunks - 1);

}

/************************************************
 * posn_at_time()
 *
 * Returns the chunk ref posn for data_type at the
 * valid time. If data_type is 0, it is ignored.

 * This is -1 if no chunk has been stored for this
 * time.
 */

static si32 posn_at_time(spdb_handle_t *handle,
			 si32 valid_time,
			 si32 data_type)

{

  int i;
  si32 vmin;
  si32 minute_posn;
  spdb_chunk_ref_t *ref;
  
  if (!handle->active) {
    return (-1);
  }
  
  vmin = (valid_time % SECS_IN_DAY) / SECS_IN_MIN;
  minute_posn = handle->hdr.minute_posn[vmin];

  if (minute_posn < 0) {
    return (-1);
  }

  ref = handle->chunk_refs + minute_posn;

  for (i = minute_posn; i < handle->hdr.n_chunks; i++, ref++) {
    if ((ref->valid_time - valid_time) > SECS_IN_MIN) {
      return (-1);
    }
    if (data_type == 0 && 
	ref->valid_time == valid_time) {
      return (i);
    } else if (ref->data_type == data_type &&
	       ref->valid_time == valid_time) {
      return (i);
    }

  } /* i */

  return (-1);

}

/***************
 * read_chunk()
 *
 * Reads chunk into buffer provided.
 *
 * Returns 0 on success, -1 on failure.
 */

static int read_chunk(spdb_handle_t *handle,
		      si32 data_offset,
		      void *chunk, int len)

{
  
  if (!handle->active) {
    fprintf(stderr, "ERROR - SPDB:read_chunk\n");
    fprintf(stderr, "Handle not active\n");
    return (-1);
  }
  
  /*
   * seek to offset
   */
  
  fseek(handle->data_file, data_offset, SEEK_SET);
  
  /*
   * read data
   */

  if (ta_fread(chunk, 1, len, handle->data_file) != len) {
    fprintf(stderr, "ERROR - SPDB: product %s\n",
	    handle->hdr.prod_label);
    fprintf(stderr, "Cannot read chunk of len %d at data offset %d\n",
	    (int) len, (int) data_offset);
    perror(handle->data_path);
    return (-1);
  }

  return (0);

}

/*********************************************
 * set_earliest_valid()
 *
 * Set the earliest_valid_time for any files which
 * have times which overlap the valid period of
 * this product.
 *
 * Returns 0 on success, -1 on failure
 *
 */

static int set_earliest_valid(spdb_handle_t *handle,
			      si32 valid_time,
			      si32 expire_time)

{

  spdb_handle_t day_handle;
  
  si32 iday;
  si32 valid_day, expire_day;
  si32 midday_time;

  valid_day = valid_time / SECS_IN_DAY;
  expire_day = expire_time / SECS_IN_DAY;
  
  /*
   * limit the expire day to 3 days ahead
   */

  expire_day = MIN(expire_day, valid_day + 3);

  if (expire_day > valid_day) {

    if (SPDB_init(&day_handle, handle->prod_label,
		  handle->prod_id, handle->dir)) {
      fprintf(stderr, "ERROR - SPDB - set_earliest_valid\n");
      fprintf(stderr, "Cannot create spdb for dir %s\n", handle->dir);
      return (-1);
    }

    for (iday = valid_day + 1; iday <= expire_day; iday++) {

      /*
       * compute midday time
       */

      midday_time = iday * SECS_IN_DAY + SECS_IN_DAY / 2;
      
      /*
       * read in header
       */

      if (setup_files(&day_handle, midday_time, SETUP_WRITE)) {
	fprintf(stderr, "ERROR - SPDB - set_earliest_valid\n");
	fprintf(stderr, "Cannot setup index files for dir %s, time %s\n",
		handle->dir, UTIMstr(midday_time));
	return (-1);
      }

      /*
       * adjust earliest valid time as necessary
       */
      
      if (valid_time < day_handle.hdr.earliest_valid) {
	day_handle.hdr.earliest_valid = valid_time;
	write_indx_file(&day_handle);
      }

      close_files(&day_handle);

    }

    SPDB_free(&day_handle);

  }

  return (0);

}

/*********************************************
 * setup_files()
 *
 * If correct files are already open, close them
 * and reopen them so the header is re-read.  May
 * want to just re-read the headers in the future,
 * didn't have time right now.
 * Otherwise, open files as appropriate.
 * If the files exist, read in header.
 *
 * Returns SETUP_SUCCESS on success
 *         SETUP_NO_FILES if no files at this time
 *         SETUP_FAILURE on failure
 *
 */

static int setup_files(spdb_handle_t *handle,
		       si32 valid_time,
		       int mode)

{

  int i;
  int indx_file_exists;
  
  struct stat stat_buf;
  UTIMstruct vtime;

  int chunks_read;
  spdb_chunk_ref_t *ref;

  /*
   * close files if open
   */
  
  close_files(handle);

  /*
   * make directory if needed
   */

  if (mode == SETUP_WRITE) {
    ta_makedir_recurse(handle->dir);
  }

  /*
   * load the path names
   */
  
  UTIMunix_to_date(valid_time, &vtime);

  sprintf(handle->indx_path, "%s%s%.4ld%.2ld%.2ld.%s",
	  handle->dir, PATH_DELIM,
	  vtime.year, vtime.month, vtime.day,
	  SPDB_INDX_EXT);

  sprintf(handle->data_path, "%s%s%.4ld%.2ld%.2ld.%s",
	  handle->dir, PATH_DELIM,
	  vtime.year, vtime.month, vtime.day,
	  SPDB_DATA_EXT);

  /*
   * decide if indx file exists
   */
  
  if (ta_stat_uncompress(handle->indx_path, &stat_buf) == 0) {
    
    /*
     * file exists - does its size exceed the hdr size?
     */

    if (stat_buf.st_size >= sizeof(spdb_hdr_t)) {
      indx_file_exists = TRUE;
    } else {
      fprintf(stderr, "ERROR - spdb:setup_files\n");
      fprintf(stderr, "  Corrupt file: '%s'\n", handle->indx_path);
      indx_file_exists = FALSE;
    }

  } else {
    
    indx_file_exists = FALSE;

  }

  /*
   * if indx file does not exist, and we do not wish to
   * write, return now with error
   */
  
  if (!indx_file_exists && (mode == SETUP_READ)) {
    return (SETUP_NO_FILES);
  }

  /*
   * open files as appropriate
   */
  
  if (indx_file_exists) {

    char *open_mode;
    if (mode == SETUP_READ) {
      open_mode = "r";  /* read only */
    } else {
      open_mode = "r+"; /* read/write */
    }
    
    /*
     * open files for read/write
     */
    
    if ((handle->indx_file =
	 ta_fopen_uncompress(handle->indx_path, open_mode)) == NULL) {

      /*
       * failure - try read-only
       */

      if ((handle->indx_file =
	   ta_fopen_uncompress(handle->indx_path, "r")) == NULL) {

	fprintf(stderr, "ERROR - SPDB: product %s\n",
		handle->hdr.prod_label);
	fprintf(stderr, "Cannot open indx file for read\n");
	perror(handle->indx_path);
	return (SETUP_FAILURE);

      }
	
    }
    handle->indx_fd = fileno(handle->indx_file);

    if ((handle->data_file =
	 ta_fopen_uncompress(handle->data_path, open_mode)) == NULL) {

      /*
       * failure - try read-only
       */

      if ((handle->data_file =
	   ta_fopen_uncompress(handle->data_path, "r")) == NULL) {

	fprintf(stderr, "ERROR - SPDB: product %s\n",
		handle->hdr.prod_label);
	fprintf(stderr, "Cannot open data file for read\n");
	perror(handle->data_path);
	close_files(handle);
	return (SETUP_FAILURE);

      }

    }
    handle->data_fd = fileno(handle->data_file);

    /*
     * set file locking
     */
    
    if (setup_locks(handle, mode)) {
      return (SETUP_FAILURE);
    }

    /*
     * read in index header
     */

    if (ta_fread((char *) &handle->hdr, sizeof(spdb_hdr_t), 1,
		 handle->indx_file) != 1) {
      fprintf(stderr, "ERROR - SPDB: product %s\n",
	      handle->hdr.prod_label);
      fprintf(stderr, "Cannot read indx file hdr\n");
      perror(handle->indx_path);
      close_files(handle);
      return (SETUP_FAILURE);
    }

    /*
     * swap header as required, except for label which is char
     */

    BE_to_array_32((ui32 *) ((char *) &handle->hdr + SPDB_LABEL_MAX),
		   sizeof(spdb_hdr_t) - SPDB_LABEL_MAX);

    /*
     * allocate for chunk references
     */

    if (alloc_chunk_refs(handle, handle->hdr.n_chunks)) {
      close_files(handle);
      return (SETUP_FAILURE);
    }

    /*
     * read in chunk refs.  If there are not enough chunk headers
     * in the file, print out a warning and update the n_chunks
     * value in the index header.  This is done so we can recover
     * files which have been corrupted by a process dying before the
     * chunk header is written.  Because we are updating the index
     * header value, the file will no longer be corrupted after the
     * next write by the writing process so we don't need to rewrite
     * the index header here.
     */

    if ((chunks_read = ta_fread((char *) handle->chunk_refs,
				sizeof(spdb_chunk_ref_t),
				handle->hdr.n_chunks,
				handle->indx_file))
	!= handle->hdr.n_chunks) {
      fprintf(stderr, "WARNING - SPDB: product %s\n",
	      handle->hdr.prod_label);
      fprintf(stderr, "Problem reading indx file chunk refs\n");
      fprintf(stderr, "Read %d chunks, expected %d chunks\n",
	      chunks_read, handle->hdr.n_chunks);
      perror(handle->indx_path);

      handle->hdr.n_chunks = chunks_read;
    }

    /*
     * swap chunk refs as required
     */
    
    BE_to_array_32((ui32 *) handle->chunk_refs,
		   handle->hdr.n_chunks * sizeof(spdb_chunk_ref_t));

    /*
     * copy the data base prod_id into the chunk headers - this is
     * done because early data bases did not have the prod_id in
     * the chunk
     */

    ref = handle->chunk_refs;
    for (i = 0; i < handle->hdr.n_chunks; i++, ref++) {
      if (ref->prod_id == 0) {
	ref->prod_id = handle->hdr.prod_id;
      }
    }

    /*
     * check that the ID is correct - if ID of 0 is passed in
     * we accept any data ID
     */

    if (handle->prod_id > 0) {
      if (handle->hdr.prod_id == 0) {
	handle->hdr.prod_id = handle->prod_id;
      } else if (handle->hdr.prod_id != handle->prod_id) {
	fprintf(stderr, "ERROR - SPDB: product %s\n",
		handle->hdr.prod_label);
	fprintf(stderr, "Incorrect indx file ID %d - should be %d\n",
		handle->hdr.prod_id, handle->prod_id);
	close_files(handle);
	return (SETUP_FAILURE);
      }
    } else {
      handle->prod_id = handle->hdr.prod_id;
      STRncopy(handle->prod_label, handle->hdr.prod_label, SPDB_LABEL_MAX);
    }
    
  } else { /* if (indx_file_exists) */
    
    /*
     * open files for write/read
     */
    
    if ((handle->indx_file =
	 ta_fopen_uncompress(handle->indx_path, "w+")) == NULL) {
      fprintf(stderr, "ERROR - SPDB: product %s\n",
	      handle->hdr.prod_label);
      fprintf(stderr, "Cannot open indx file for write/read\n");
      perror(handle->indx_path);
      return (SETUP_FAILURE);
    }
    handle->indx_fd = fileno(handle->indx_file);

    if ((handle->data_file =
	 ta_fopen_uncompress(handle->data_path, "w+")) == NULL) {
      fprintf(stderr, "ERROR - SPDB: product %s\n",
	      handle->hdr.prod_label);
      fprintf(stderr, "Cannot open data file for write/read\n");
      perror(handle->data_path);
      close_files(handle);
      return (SETUP_FAILURE);
    }
    handle->data_fd = fileno(handle->data_file);
    
    /*
     * set file locking
     */
    
    if (setup_locks(handle, mode)) {
      close_files(handle);
      return (SETUP_FAILURE);
    }

    /*
     * initialize header
     */
    
    memset((void *) &handle->hdr, 0, sizeof(spdb_hdr_t));
    STRncopy(handle->hdr.prod_label, handle->prod_label,
	     SPDB_LABEL_MAX);
    handle->hdr.major_version = SPDB_FILE_MAJOR_VERSION;
    handle->hdr.minor_version = SPDB_FILE_MINOR_VERSION;
    handle->hdr.prod_id = handle->prod_id;
    handle->hdr.n_chunks = 0;
    handle->hdr.nbytes_frag = 0;
    handle->hdr.nbytes_data = 0;
    handle->hdr.max_duration = 0;
    handle->hdr.start_of_day = (valid_time / SECS_IN_DAY) * SECS_IN_DAY;
    handle->hdr.end_of_day = handle->hdr.start_of_day + 86399;
    handle->hdr.start_valid = handle->hdr.end_of_day;
    handle->hdr.end_valid = handle->hdr.start_of_day;
    handle->hdr.latest_expire = handle->hdr.start_of_day;
    handle->hdr.earliest_valid = handle->hdr.end_of_day;
    memset(handle->hdr.spares, 0, sizeof(handle->hdr.spares));
    
    for (i = 0; i < MINS_IN_DAY; i++) {
      handle->hdr.minute_posn[i] = -1;
    }

    if (write_indx_file(handle)) {
      close_files(handle);
      return (SETUP_FAILURE);
    }

  }

  /*
   * successful, so set vday and active flag
   */

  handle->vday = valid_time / SECS_IN_DAY;
  handle->active = TRUE;
  
  return (SETUP_SUCCESS);
  
}

/*********************************************
 * setup_locks()
 *
 * Sets up locks on files.
 *
 * Returns 0 on success, -1 on failure.
 */

static int setup_locks(spdb_handle_t *handle,
		       int mode)

{

  struct flock lock;

  if (mode == SETUP_READ) {
    lock.l_type = F_RDLCK;
  } else {
    lock.l_type = F_WRLCK;
  }
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;

  if (fcntl(handle->indx_fd, F_SETLKW, &lock) == -1) {
    fprintf(stderr, "ERROR - SPDB:setup_locks\n");
    perror(handle->indx_path);
    /* return (-1); Tmp fix for no-compliant xmounts - dixon */
  }
  
  if (fcntl(handle->data_fd, F_SETLKW, &lock) == -1) {
    fprintf(stderr, "ERROR - SPDB:setup_locks\n");
    perror(handle->data_path);
    /* return (-1); Tmp fix for no-compliant xmounts - dixon  */
  }

  handle->locked = TRUE;

  return (0);

}

/*****************************************************
 * store_chunk()
 *
 * Stores a chunk in the data base. There are 3 modes:
 *   STORE_ONCE - no overwrite
 *   STORE_OVERWRITE - will place the new chunk over the
 *     old one if it fits, otherwise at the end of the file
 *   STORE_ADD - adds the chunk, allows multiple chunks at
 *     the same time.
 *
 * Returns 0 on success, -1 on failure
 */

static int store_chunk(spdb_handle_t *handle,
		       si32 data_type,
		       si32 valid_time, si32 expire_time,
		       void *chunk, int len,
		       int mode)
     
{

  int append;
  int nfrag;
  si32 posn;
  si32 data_offset;
  spdb_chunk_ref_t *ref;
  
  /*
   * set up files
   */
  
  if (setup_files(handle, valid_time, SETUP_WRITE) != 0) {
    fprintf(stderr, "ERROR - SPDB: product %s\n",
	    handle->hdr.prod_label);
    fprintf(stderr, "Cannot set up files for chunk at time %s\n",
	    UTIMstr(valid_time));
    close_files(handle);
    return (-1);
  }

  switch (mode) {

  case STORE_ONCE:
    posn = stored_posn(handle, valid_time, data_type);
    if (posn >= 0) {
      clear_locks(handle);
      return (-1);
    } else {
      append = TRUE;
    }
    break;

  case STORE_ADD:
    append = TRUE;
    break;

  case STORE_OVERWRITE:
    posn = stored_posn(handle, valid_time, data_type);
    if (posn >= 0) {
      append = FALSE;
    } else {
      append = TRUE;
    }
    break;

  default:
    append = TRUE;
    
  } /* switch (mode) */

  if (append) {
    
    data_offset = -1;
    
  } else {
    
    ref = handle->chunk_refs + posn;

    if (ref->len >= len) {

      /*
       * chunk will fit in previous location. Fragment
       * is created at end of slot if they are not
       * the same size.
       */

      nfrag = ref->len - len;
      handle->hdr.nbytes_frag += nfrag;
      handle->hdr.nbytes_data -= nfrag;
      data_offset = ref->offset;
      
    } else {

      /*
       * chunk will not fit in previous location,
       * store at end of file. Entire slot
       * becomes a fragment.
       */

      handle->hdr.nbytes_frag += ref->len;
      handle->hdr.nbytes_data -= ref->len;
      data_offset = -1;

    } /* if (ref->len >= len) */

  } /* if (append) */

  /*
   * write data
   */
  
  if (write_chunk(handle, valid_time, &data_offset, chunk, len)) {
    fprintf(stderr, "ERROR - SPDB: product %s\n",
	    handle->hdr.prod_label);
    fprintf(stderr, "Cannot write chunk to data file\n");
    close_files(handle);
    return (-1);
  }

  if (append) {
    
    /*
     * add chunk reference
     */
    
    if ((posn = add_chunk_ref(handle, data_type, valid_time,
			      expire_time, data_offset, len)) < 0) {
      clear_locks(handle);
      return (-1);
    }

  } else {
    
    /*
     * store chunk ref at previous location
     */
    
    ref = handle->chunk_refs + posn;
    ref->data_type = data_type;
    ref->valid_time = valid_time;
    ref->expire_time = expire_time;
    ref->prod_id = handle->hdr.prod_id;
    ref->offset = data_offset;
    ref->len = len;
    
  } /* if (append) */

  /*
   * keep stats up to date
   */
  
  handle->hdr.max_duration =
    MAX(handle->hdr.max_duration,
	(expire_time - valid_time));

  handle->hdr.start_valid =
    MIN(handle->hdr.start_valid, valid_time);

  handle->hdr.end_valid =
    MAX(handle->hdr.end_valid, valid_time);
  
  handle->hdr.latest_expire =
    MAX(handle->hdr.latest_expire, expire_time);

  handle->hdr.earliest_valid =
    MIN(handle->hdr.earliest_valid, valid_time);

  set_earliest_valid(handle, valid_time, expire_time);

  /*
   * write index file
   */
  
  if (write_indx_file(handle)) {
    fprintf(stderr, "ERROR - SPDB: product %s\n",
	    handle->hdr.prod_label);
    fprintf(stderr, "Cannot write indx file\n");
    close_files(handle);
    return (-1);
  }

  /*
   * write latest data info
   */
  
  if (LDATA_info_write(&handle->ldata, handle->dir,
		       valid_time, SPDB_INDX_EXT,
		       NULL, NULL, 0, NULL)) {
    fprintf(stderr, "ERROR - SPDB: product %s\n",
	    handle->hdr.prod_label);
    fprintf(stderr, "Cannot write ldata_info file\n");
    close_files(handle);
    return (-1);
  }
 
  /*
   * clear locks
   */
  
  clear_locks(handle);

  return (0);

}

/****************************************
 * stored_posn()
 *
 * Finds posn of data of this type at the valid time.
 * If data_type is 0, type check is ignored.
 *
 * Returns posn on success, -1 on failure.
 */

static int stored_posn(spdb_handle_t *handle,
		       si32 valid_time,
		       si32 data_type)
     
{

  si32 posn;

  if (!handle->active) {
    return (-1);
  }
  
  if (handle->hdr.n_chunks == 0) {
    return (-1);
  }
  
  if (valid_time < handle->hdr.start_valid ||
      valid_time > handle->hdr.end_valid) {
    return (-1);
  }

  if ((posn = posn_at_time(handle, valid_time, data_type)) < 0) {
    return (-1);
  } else {
    return (posn);
  }

}

/************************************************
 * define function to be used for sorting strings
 */

static int str_compare(const void *v1, const void *v2)

{
    char **s1, **s2;
    s1 = (char **) v1;
    s2 = (char **) v2;
    return strcmp(*s1, *s2);
}

/********************
 * write_indx_file()
 *
 * Returns 0 on success, -1 on failure.
 */

static int write_indx_file(spdb_handle_t *handle)

{

  static int first_call = TRUE;
  static MEMbuf *tmp_ref_buf;
  char *tmp_refs;
  spdb_hdr_t tmp_hdr;

  if (first_call) {
    tmp_ref_buf = MEMbufCreate();
    first_call = FALSE;
  }
  
  /*
   * copy header and put into BE order
   */

  tmp_hdr = handle->hdr;
  
  BE_from_array_32((ui32 *) ((char *) &tmp_hdr + SPDB_LABEL_MAX),
		   sizeof(spdb_hdr_t) - SPDB_LABEL_MAX);
  
  /*
   * write out index header
   */

  fseek(handle->indx_file, 0, SEEK_SET);

  if (ta_fwrite((char *) &tmp_hdr, sizeof(spdb_hdr_t), 1,
		handle->indx_file) != 1) {
    fprintf(stderr, "ERROR - SPDB: product %s\n",
	    handle->hdr.prod_label);
    fprintf(stderr, "Cannot write indx file hdr\n");
    perror(handle->indx_path);
    return (-1);
  }

  if (handle->hdr.n_chunks > 0)
  {
    
    /*
     * copy to tmp_ref_buf
     */

    tmp_refs = MEMbufLoad(tmp_ref_buf, handle->chunk_refs,
			  handle->hdr.n_chunks * sizeof(spdb_chunk_ref_t));

    /*
     * swap chunk refs as required
     */
  
    BE_from_array_32((ui32 *) tmp_refs,
		     handle->hdr.n_chunks * sizeof(spdb_chunk_ref_t));

    /*
     * write out chunk refs
     */
  
    if (ta_fwrite((char *) tmp_refs, sizeof(spdb_chunk_ref_t),
		  handle->hdr.n_chunks,
		  handle->indx_file) != handle->hdr.n_chunks) {
      fprintf(stderr, "ERROR - SPDB: product %s\n",
	      handle->hdr.prod_label);
      fprintf(stderr, "Cannot write indx file chunk refs\n");
      perror(handle->indx_path);
      return (-1);
    }
  }
  
  /*
   * Flush the index file stream to make sure the data is written.
   */

  fflush(handle->indx_file);
  
  return (0);

}

/***************
 * write_chunk()
 *
 * Returns 0 on success, -1 on failure.
 */

static int write_chunk(spdb_handle_t *handle,
		       si32 valid_time, si32 *data_offset,
		       void *chunk, int len)
     
{
  
  if (!handle->active) {
    fprintf(stderr, "ERROR - SPDB:write_chunk\n");
    fprintf(stderr, "Handle not active\n");
    return (-1);
  }

  if (*data_offset < 0) {

    /*
     * for negative offsets, seek to end of data file
     */
    
    fseek(handle->data_file, 0, SEEK_END);
    *data_offset = ftell(handle->data_file);

  } else {

    /*
     * seek to offset
     */
    
    fseek(handle->data_file, *data_offset, SEEK_SET);

  }
  
  /*
   * write data
   */

  if (ta_fwrite(chunk, len, 1, handle->data_file) != 1) {
    fprintf(stderr, "ERROR - SPDB: product %s\n",
	    handle->hdr.prod_label);
    fprintf(stderr, "Cannot write chunk for time %s\n",
	    UTIMstr(valid_time));
    perror(handle->data_path);
    return (-1);
  }

  /*
   * Flush the data file stream to make sure the data is written.
   */

  fflush(handle->data_file);
  
  return (0);

}

