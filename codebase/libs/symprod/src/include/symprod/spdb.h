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
#ifdef __cplusplus
 extern "C" {
#endif


/******************************************************************
 * spdb.h: header file for the symbolic products database routines.
 *
 ******************************************************************/

#ifndef spdb_h
#define spdb_h

#include <stdio.h>
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/ldata_info.h>
#include <toolsa/mem.h>

/*
 * The Symbolic Products Data Base (SPDB) consists of a pair
 * of files for each day. Each product type will be stored in
 * its own directory.
 *
 * For each day, there is an index file (extension .indx) and a
 * data file (extension .data). The index file header contains
 * a product type string and a product information string. The
 * info string may be optionally filled out.
 *
 * The header also has product ID field, which should be set.
 *
 * Included in the header is a fixed length array of 1440 integers,
 * minute_posn, which are used for rapid searching for data at a
 * given time. The minute_posn values store the first chunk position
 * for each minute of the day. If no chunk corresponds to this minute
 * the value is set to -1.
 *
 * Following the header is an array, n_chunks long, of the
 * spdb chunk structs. Each chunk points into the data file,
 * giving the offset and length of the data chunk for this
 * chunk. The valid time is used for sorting the chunks. The
 * expire time is included to allow retrieval of all chunks which
 * are valid at a given time.
 *
 * All values in the header are stored in Big-Endian format. The
 * chunk data is stored as it is passed to the library. The calling
 * program must make sure the chunks are in BE format.
 */

/*
 * Version numbers
 */

#define SPDB_FILE_MAJOR_VERSION 1
#define SPDB_FILE_MINOR_VERSION 0

/*
 * file path extensions
 */

#define SPDB_INDX_EXT "indx"
#define SPDB_DATA_EXT "data"

/*
 * text lengths
 */

#define SPDB_LABEL_MAX 64
#define SPDB_PATH_MAX 1024

/*
 * Return values.
 */

#define SPDB_SUCCESS              0

#define SPDB_NO_DATA_AVAIL        1

#define SPDB_SMU_NO_SERVERS      -1
#define SPDB_SMU_INFO_ERROR      -2
#define SPDB_DATA_GET_ERROR      -3

/*
 * other defines
 */

#define SECS_IN_DAY 86400
#define SECS_IN_MIN 60
#define MINS_IN_DAY 1440
#define SPDB_INIT_FLAG 987654321

/*
 * header struct - occurs once at the top of the
 * index file
 */

typedef struct {

  char prod_label[SPDB_LABEL_MAX];

  /*
   * There must be 80 si32's following, including spares
   */
  
  si32 major_version;
  si32 minor_version;

  si32 prod_id;
  si32 n_chunks;

  si32 nbytes_frag;  /* number of fragmented bytes - these
		      * are 'lost' because during overwrite
		      * the new chunk did not occupy exactly
		      * the same space as the old chunk.
		      */

  si32 nbytes_data;   /* number of bytes of usable data -
		       * file size = nbytes_data + nbytes_frag
		       */

  si32 max_duration;  /* max number of secs over which a product
		       * is valid
		       */
  
  si32 start_of_day;  /* time of start of day for this file */
  si32 end_of_day;    /* time of end of day for this file */

  si32 start_valid;   /* start valid time of data in this file */
  si32 end_valid;     /* end valid time of data in this file */

  si32 earliest_valid; /* the earliest time for which there are valid
			* products during this day. If no products from
			* previous files overlap into this day, this
			* value will be the same as start_valid.
			* If products from previous days are valid 
			* during this day, this time be set to the
			* earliest valid time for those products */
  
  si32 latest_expire; /* latest expire time for data in this file */

  si32 spares[67];

  /* minute_posn stores the first chunk position
   * for each minute of the day. If no chunk
   * corresponds to this minute the value is
   * -1 */

  si32 minute_posn[MINS_IN_DAY];
  
} spdb_hdr_t;

/*
 * chunk ref struct - there is an array of n_chunks of
 * these following the header. They are written in 
 * order sorted on the valid_time field.  The data_type
 * field can be used to filter data returned to clients.
 * To be used, the data_type field must be set to a non-zero
 * value.  Data requests for data type 0 return all data
 * in the calls in this library.
 */

typedef struct {

  si32 valid_time;
  si32 expire_time;
  si32 data_type;
  si32 prod_id;      /* filled in on read */
  ui32 offset;       /* offset of data chunk in .data file */
  ui32 len;          /* length of data chunk in data file */

} spdb_chunk_ref_t;

/*
 * 
 */

typedef struct {

  char dir[SPDB_PATH_MAX];
  char indx_path[SPDB_PATH_MAX];
  char data_path[SPDB_PATH_MAX];
  char prod_label[SPDB_LABEL_MAX];

  int init;
  int active;
  int locked;

  si32 prod_id;
  si32 vday;

  int indx_fd;
  int data_fd;
  
  FILE *indx_file;
  FILE *data_file;

  LDATA_handle_t ldata;
  
  spdb_hdr_t hdr;

  MEMbuf *chunk_ref_buf;
  spdb_chunk_ref_t *chunk_refs;

  MEMbuf *fetch_ref_buf;
  MEMbuf *fetch_chunk_buf;
  MEMbuf *fetch_read_buf;

} spdb_handle_t;

/*
 * prototypes
 */

/***********************
 * SPDB_init()
 *
 * Initialize an SPDB object
 *
 * returns 0 on success, -1 on failure
 */

extern int SPDB_init(spdb_handle_t *handle,
		     char *prod_label,
		     si32 prod_id,
		     char *dir);
     
/***********************
 * SPDB_close()
 *
 * Close files associated with an SPDB object
 */

extern void SPDB_close(spdb_handle_t *handle);
     
/***********************
 * SPDB_free()
 *
 * Free an SPDB object
 */

extern void SPDB_free(spdb_handle_t *handle);

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

extern int SPDB_fetch(spdb_handle_t *handle,     /* I */
		      si32 data_type,            /* I */
		      si32 request_time,         /* I */
		      si32 *n_chunks_p,          /* O */
		      spdb_chunk_ref_t **refs_p, /* O */
		      void **chunks_p);          /* O */

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

extern int SPDB_fetch_closest(spdb_handle_t *handle,     /* I */
			      si32 data_type,            /* I */
			      si32 request_time,         /* I */
			      si32 time_margin,          /* I */
			      si32 *n_chunks_p,          /* O */
			      spdb_chunk_ref_t **refs_p, /* O */
			      void **chunks_p);          /* O */
     
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

extern int SPDB_fetch_interval(spdb_handle_t *handle,     /* I */
			       si32 data_type,            /* I */
			       si32 start_time,           /* I */
			       si32 end_time,             /* I */
			       si32 *n_chunks_p,          /* O */
			       spdb_chunk_ref_t **refs_p, /* O */
			       void **chunks_p);          /* O */

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

extern int SPDB_fetch_valid(spdb_handle_t *handle,     /* I */
			    si32 data_type,            /* I */
			    si32 search_time,          /* I */
			    si32 *n_chunks_p,          /* O */
			    spdb_chunk_ref_t **refs_p, /* O */
			    void **chunks_p);          /* O */
  
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

extern int SPDB_fetch_first_before(spdb_handle_t *handle,     /* I */
				   si32 data_type,            /* I */
				   si32 request_time,         /* I */
				   si32 time_margin,          /* I */
				   si32 *n_chunks_p,          /* O */
				   spdb_chunk_ref_t **refs_p, /* O */
				   void **chunks_p);          /* O */
     
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
			   void **chunks_p);          /* O */
     
/*******************************************************
 * SPDB_chunk_data_len()
 *
 * Returns the length of the chunk data buffer after a fetch.
 */

extern int SPDB_chunk_data_len(spdb_handle_t *handle);

/*****************************************************
 * SPDB_store()
 *
 * Stores a chunk in the data base, without overwrite.
 *
 * Returns 0 on success, -1 on failure,
 * or if chunk is already stored at this time.
 */

extern int SPDB_store(spdb_handle_t *handle,
		      si32 data_type,
		      si32 valid_time, si32 expire_time,
		      void *chunk, int len);

/*****************************************************
 * SPDB_store_add()
 *
 * Stores a chunk in the data base, adding chunks at
 * the given time.
 *
 * Returns 0 on success, -1 on failure.
 */

extern int SPDB_store_add(spdb_handle_t *handle,
			  si32 data_type,
			  si32 valid_time, si32 expire_time,
			  void *chunk, int len);
     
/*****************************************************
 * SPDB_store_over()
 *
 * Stores a chunk in the data base, with overwrite.
 *
 * Returns 0 on success, -1 on failure
 */

extern int SPDB_store_over(spdb_handle_t *handle,
			   si32 data_type,
			   si32 valid_time, si32 expire_time,
			   void *chunk, int len);
     
/*******************************************************
 * SPDB_last_valid_time()
 *
 * Gets the last data valid time, if there is data in the
 * data base.
 *
 * Returns 0 on success, -1 on failure (i.e.no data)
 */

extern int SPDB_last_valid_time(spdb_handle_t *handle,     /* I */
				si32 *last_valid_time);    /* O */

/*******************************************************
 * SPDB_first_and_last_times()
 *
 * Gets first and last times, if there is data in the
 * data base.
 *
 * Returns 0 on success, -1 on failure (i.e.no data)
 */

extern int SPDB_first_and_last_times(spdb_handle_t *handle,   /* I */
				     si32 *first_time,        /* O */
				     si32 *last_time);        /* O */

/*****************************************************
 * SPDB_last_unique()
 *
 * Returns a buffer of chunks which includes only the chunks
 * with unique data_types with the latest valid times.  Note
 * that all chunks in the original buffer with a data_type of
 * 0 will be returned in the returned buffer.
 *
 * NOTE:  The incoming buffer is assumed to be in chronological
 *        order, which is the order returned by all of the
 *        SPDB_fetch and SPDB_get routines.
 *
 * NOTE:  The returned pointers point to handle memory within
 *        this routine and should not be freed by the calling
 *        routine.  These pointers will be overwritten by the
 *        next call to this routine.
 */

void SPDB_last_unique(si32 nchunks,                         /* I */
		      spdb_chunk_ref_t *chunk_hdrs,         /* I */
		      void *chunk_data,                     /* I */
		      si32 *nchunks_unique,                 /* O */
		      spdb_chunk_ref_t **chunk_hdrs_unique, /* 0 */
		      void **chunk_data_unique);            /* O */

/*****************************************************
 * SPDB_print_chunk_ref()
 *
 * Prints the given chunk reference structure
 */

void SPDB_print_chunk_ref(spdb_chunk_ref_t *chunk_ref,   /* I */
			  FILE *out);                    /* I */
     
/*****************************************************
 * SPDB_print_header()
 *
 * Prints the header file for a given time
 *
 * Returns 0 on success, -1 on failure
 */

extern int SPDB_print_header(spdb_handle_t *handle,     /* I */
			     si32 request_time,         /* I */
			     FILE *out);                /* I */
     
/*****************************************************
 * SPDB_print_index_header()
 *
 * Prints the header structure
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_print_index_header(spdb_hdr_t *spdb_hdr,      /* I */
			    FILE *out,                 /* I */
			    int print_min_posn);       /* I */


#endif

#ifdef __cplusplus
}
#endif
