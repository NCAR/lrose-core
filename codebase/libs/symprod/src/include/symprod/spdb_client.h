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
 * spdb_client.h: header file for the symbolic products database
 *                client routines.
 *
 ******************************************************************/

#ifndef spdb_client_h
#define spdb_client_h

#include <stdio.h>

#include <toolsa/os_config.h>

#include <dataport/port_types.h>

#include <symprod/spdb.h>

#include <toolsa/servmap.h>

/*
 * The SPDB client routines support the following interface:
 *
 * Requests to retrieve data
 * -------------------------
 *
 * Requests:
 *
 * [Get the chunks stored at the requested time.]
 *                si32 request = SPDB_GET_DATA
 *                si32 data_type
 *                si32 request_time
 *
 * [Get the chunks stored at the closest time to that requested,
 *  within the time margin.]
 *                si32 request = SPDB_GET_DATA_CLOSEST
 *                si32 data_type
 *                si32 request_time
 *                si32 time_margin
 *
 * [Get the chunks sotred between the requested times.]
 *                si32 request = SPDB_GET_DATA_INTERVAL
 *                si32 data_type
 *                si32 start_time
 *                si32 end_time
 *
 * [Get the chunks valid at a given time.]
 *                si32 request = SPDB_GET_DATA_VALID
 *                si32 data_type
 *                si32 search_time
 *
 * Replies:
 *
 * [The data was available and here it is.]
 *                si32 reply = SPDB_DATA
 *                ui32 nchunks
 *                ui32 data_length
 *                spdb_chunk_ref_t index_array[nchunks]
 *                ui08 chunk_data[data_length]
 *
 * [There is no data matching the request.]
 *                si32 reply = SPDB_NO_DATA
 *
 * [There was an error retrieving the data requested.]
 *                si32 reply = SPDB_DATA_ERROR
 *
 * Requests to store data
 * ----------------------
 *
 * Requests:
 *
 * [Store chunks in the database, without overwrite.]
 *                 si32 request = SPDB_PUT_DATA
 *                 ui32 nchunks
 *                 ui32 data_length
 *                 spdb_chunk_ref_t index_array[nchunks]
 *                 ui08 chunk_data[data_length]
 *
 * [Store chunks in the database, adding chunks at the times
 *  indicated in the index_array.]
 *                 si32 request = SPDB_PUT_DATA_ADD
 *                 ui32 nchunks
 *                 ui32 data_length
 *                 spdb_chunk_ref_t index_array[nchunks]
 *                 ui08 chunk_data[data_length]
 *
 * [Store chunks in the database with overwrite.]
 *                 si32 request = SPDB_PUT_DATA_OVER
 *                 ui32 nchunks
 *                 ui32 data_length
 *                 spdb_chunk_ref_t index_array[nchunks]
 *                 ui08 chunk_data[data_length]
 *
 * Replies:
 *
 * [The data was stored successfully.]
 *                 si32 reply = SPDB_PUT_SUCCESSFUL
 *
 * [There was an error storing the data.]
 *                 si32 reply = SPDB_PUT_FAILED
 *
 * Replies to erroneous requests:
 * ------------------------------
 *
 * Replies:
 *
 * [The request could not be parsed.]
 *                 si32 reply = SPDB_REQUEST_ERROR
 *
 *
 */

/*
 * Client requests.
 */

typedef enum
{
  SPDB_GET_DATA = 1000,
  SPDB_GET_DATA_CLOSEST,
  SPDB_GET_DATA_INTERVAL,
  SPDB_GET_DATA_VALID,
  SPDB_GET_DATA_FIRST_BEFORE,
  SPDB_GET_DATA_FIRST_AFTER,

  SPDB_PUT_DATA = 2000,
  SPDB_PUT_DATA_ADD,
  SPDB_PUT_DATA_OVER
} spdb_request_t;


/*
 * Server replies.
 */

typedef enum
{
  SPDB_DATA = 5000,
  SPDB_NO_DATA,
  SPDB_DATA_ERROR,

  SPDB_PUT_SUCCESSFUL = 6000,
  SPDB_PUT_FAILED,

  SPDB_REQUEST_ERROR = 9999
} spdb_reply_t;


/*
 * Control structure for polling.  This structure
 * should NOT be manipulated by users.
 */

typedef struct
{
  si32 product_id;
  si32 data_type;
  int  latest_only;
  time_t last_data;
  SERVMAP_request_t request;
  char *servmap_host1;
  char *servmap_host2;
} spdb_poll_t;


/*
 * prototypes
 */

/*****************************************************
 * SPDB_put()
 *
 * Puts an array of SPDB chunks on the given destination.
 * The destination can be either a socket or disk location.
 *
 * For a disk destination, the destination string should
 * contain the directory path for the database.
 *
 * For a socket destination, the destination string should
 * contain the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * The chunk headers will be byte-swapped, as appropriate,
 * by this routine.  The chunk data must be put into big-
 * endian format by the caller before calling this routine.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_put(char *destination,
	     si32 product_id,
	     char *product_label,
	     ui32 nchunks,
	     spdb_chunk_ref_t *chunk_hdrs,
	     void *chunk_data,
	     int chunk_data_len);

/*****************************************************
 * SPDB_put_add()
 *
 * Puts an array of SPDB chunks on the given destination,
 * adding the chunks at the given time.  The destination
 * can be either a socket or disk location.
 *
 * For a disk destination, the destination string should
 * contain the directory path for the database.
 *
 * For a socket destination, the destination string should
 * contain the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * The chunk headers will be byte-swapped, as appropriate,
 * by this routine.  The chunk data must be put into big-
 * endian format by the caller before calling this routine.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_put_add(char *destination,
		 si32 product_id,
		 char *product_label,
		 ui32 nchunks,
		 spdb_chunk_ref_t *chunk_hdrs,
		 void *chunk_data,
		 int chunk_data_len);

/*****************************************************
 * SPDB_put_over()
 *
 * Puts an array of SPDB chunks on the given destination,
 * with overwrite.  The destination can be either a socket
 * or disk location.
 *
 * For a disk destination, the destination string should
 * contain the directory path for the database.
 *
 * For a socket destination, the destination string should
 * contain the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * The chunk headers will be byte-swapped, as appropriate,
 * by this routine.  The chunk data must be put into big-
 * endian format by the caller before calling this routine.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_put_over(char *destination,
		  si32 product_id,
		  char *product_label,
		  ui32 nchunks,
		  spdb_chunk_ref_t *chunk_hdrs,
		  void *chunk_data,
		  int chunk_data_len);

/*****************************************************
 * SPDB_get()
 *
 * Gets an array of SPDB chunks from the given source
 * stored at the requested time.  The source can be
 * either a socket or disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
 *
 * Upon return:
 *      nchunks contains the number of chunks received.
 *      chunk_hdrs points to an array of headers for the
 *                 received chunks.  The data in these
 *                 headers is byte-swapped back to the
 *                 native format before returning.
 *      chunk_data points to the actual chunk data received.
 *                 This data is returned in big-endian
 *                 format.  The caller must do any byte-
 *                 swapping that is required.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_get(char *source,
	     si32 product_id,
	     si32 data_type,
	     si32 request_time,
	     ui32 *nchunks,                  /* output */
	     spdb_chunk_ref_t **chunk_hdrs,  /* output */
	     void **chunk_data);             /* output */

/*****************************************************
 * SPDB_get_closest()
 *
 * Gets an array of SPDB chunks from the given source
 * stored at the closest time to that requested within
 * the requested time margin.  The source can be either
 * a socket or disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
 *
 * Upon return:
 *      nchunks contains the number of chunks received.
 *      chunk_hdrs points to an array of headers for the
 *                 received chunks.  The data in these
 *                 headers is byte-swapped back to the
 *                 native format before returning.
 *      chunk_data points to the actual chunk data received.
 *                 This data is returned in big-endian
 *                 format.  The caller must do any byte-
 *                 swapping that is required.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_get_closest(char *source,
		     si32 product_id,
		     si32 data_type,
		     si32 request_time,
		     si32 time_margin,
		     ui32 *nchunks,                  /* output */
		     spdb_chunk_ref_t **chunk_hdrs,  /* output */
		     void **chunk_data);             /* output */

/*****************************************************
 * SPDB_get_first_before()
 *
 * Gets an array of SPDB chunks from the given source
 * stored at the first time at or before that requested within
 * the requested time margin.  The source can be either
 * a socket or disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
 *
 * Upon return:
 *      nchunks contains the number of chunks received.
 *      chunk_hdrs points to an array of headers for the
 *                 received chunks.  The data in these
 *                 headers is byte-swapped back to the
 *                 native format before returning.
 *      chunk_data points to the actual chunk data received.
 *                 This data is returned in big-endian
 *                 format.  The caller must do any byte-
 *                 swapping that is required.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_get_first_before(char *source,
			  si32 product_id,
			  si32 data_type,
			  si32 request_time,
			  si32 time_margin,
			  ui32 *nchunks,                  /* output */
			  spdb_chunk_ref_t **chunk_hdrs,  /* output */
			  void **chunk_data);             /* output */

/*****************************************************
 * SPDB_get_first_after()
 *
 * Gets an array of SPDB chunks from the given source
 * stored at the first time at or after that requested within
 * the requested time margin.  The source can be either
 * a socket or disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
 *
 * Upon return:
 *      nchunks contains the number of chunks received.
 *      chunk_hdrs points to an array of headers for the
 *                 received chunks.  The data in these
 *                 headers is byte-swapped back to the
 *                 native format after returning.
 *      chunk_data points to the actual chunk data received.
 *                 This data is returned in big-endian
 *                 format.  The caller must do any byte-
 *                 swapping that is required.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_get_first_after(char *source,
			 si32 product_id,
			 si32 data_type,
			 si32 request_time,
			 si32 time_margin,
			 ui32 *nchunks,                  /* output */
			 spdb_chunk_ref_t **chunk_hdrs,  /* output */
			 void **chunk_data);             /* output */

/*****************************************************
 * SPDB_get_interval()
 *
 * Gets an array of SPDB chunks from the given source
 * stored between the requested times.  The source can
 * be either a socket or disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
 *
 * Upon return:
 *      nchunks contains the number of chunks received.
 *      chunk_hdrs points to an array of headers for the
 *                 received chunks.  The data in these
 *                 headers is byte-swapped back to the
 *                 native format before returning.
 *      chunk_data points to the actual chunk data received.
 *                 This data is returned in big-endian
 *                 format.  The caller must do any byte-
 *                 swapping that is required.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_get_interval(char *source,
		      si32 product_id,
		      si32 data_type,
		      si32 start_time,
		      si32 end_time,
		      ui32 *nchunks,                  /* output */
		      spdb_chunk_ref_t **chunk_hdrs,  /* output */
		      void **chunk_data);             /* output */

/*****************************************************
 * SPDB_get_valid()
 *
 * Gets an array of SPDB chunks from the given source
 * valid at the given time.  The source can be either
 * a socket or disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
 *
 * Upon return:
 *      nchunks contains the number of chunks received.
 *      chunk_hdrs points to an array of headers for the
 *                 received chunks.  The data in these
 *                 headers is byte-swapped back to the
 *                 native format before returning.
 *      chunk_data points to the actual chunk data received.
 *                 This data is returned in big-endian
 *                 format.  The caller must do any byte-
 *                 swapping that is required.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_get_valid(char *source,
		   si32 product_id,
		   si32 data_type,
		   si32 search_time,
		   ui32 *nchunks,                  /* output */
		   spdb_chunk_ref_t **chunk_hdrs,  /* output */
		   void **chunk_data);             /* output */

/********************************************
 * SPDB_free_get()
 *
 * Frees up memory used by SPDB_get() calls.
 *
 * Memory allocated by SPDB_get() calls is kept statically
 * within the SPDB module.
 *
 * You may free it at any time using SPDB_free_get().
 *
 * However, it is not necessary to do this between calls unless you
 * are concerned about total memory usage.
 * Generally this call is only used when no further SPDB_get() calls
 * will be made.
 */

extern void SPDB_free_get(void);
     
/*****************************************************
 * SPDB_handle_sigpipe()
 *
 * Closes any open sockets that were used by the SPDB
 * routines.  Should be called by the application's
 * interrupt handler when a SIGPIPE interrupt is
 * received.
 * 
 */

void SPDB_handle_sigpipe(void);

/*****************************************************
 * SPDB_reap_children()
 *
 * Check for any child processes that are finished and
 * clean them up.  This keeps us from getting overwhelmed
 * by "zombie" processes.
 */

void SPDB_reap_children(void);


/*****************************************************
 * POLL routines
 *****************************************************/

/*****************************************************
 * SPDB_poll_init()
 *
 * Initializes the SPDB poll structure.  This structure
 * should NOT be manipulated directly by the user.
 *
 * Returns a pointer to the initialized structure or
 * NULL if there is an error.
 */

spdb_poll_t *SPDB_poll_init(char *servmap_type,
			    char *servmap_subtype,
			    char *servmap_instance,
			    si32 product_id,
			    si32 data_type,
			    int get_latest_data_only);

/*****************************************************
 * SPDB_poll()
 *
 * Polls the SPDB server for any new data since the last
 * call.  If the "get latest data" flag is set in the poll
 * structure, only the latest data available to the server
 * will be returned.  Otherwise, all data since the last
 * poll will be returned.
 *
 * Sets nchunks to the number of chunks returned.  Sets
 * chunk_hdrs to point to the array of chunk headers.  Sets
 * chunk_data to point to the chunk data buffer.
 *
 * Returns SPDB_SUCCESS on success.
 * On error, returns one of:
 *               SPDB_NO_DATA_AVAIL
 *               SPDB_SMU_INFO_ERROR
 *               SPDB_SMU_NO_SERVERS
 */

int SPDB_poll(spdb_poll_t *poll_struct,
	      ui32 *nchunks,                    /* output */
	      spdb_chunk_ref_t **chunk_hdrs,    /* output */
	      void **chunk_data);               /* output */

/*****************************************************
 * SPDB_poll_delete()
 *
 * Deletes the SPDB poll structure and frees all
 * allocated space.
 */

void SPDB_poll_delete(spdb_poll_t **poll_struct);

/*****************************************************
 * Routiness for converting values to strings for printing
 *****************************************************/

/*****************************************************
 * SPDB_request2string()
 *
 * Converts a request value to the matching string.
 *
 * Returns a pointer to a static area containing the string.
 * Do NOT free this pointer.
 */

char *SPDB_request2string(int request);

/*****************************************************
 * SPDB_reply2string()
 *
 * Converts a reply value to the matching string.
 *
 * Returns a pointer to a static area containing the string.
 * Do NOT free this pointer.
 */

char *SPDB_reply2string(int reply);

/******************************************************************************
 * SPDB_4CHARS_TO_INT32: Convert the first 4 characters of an ID STRING to a int
 * On error Returns -1.  This function guarnteed to never return 0
 */
 
si32 SPDB_4chars_to_int32(char *id_string);

/******************************************************************************
 * SPDB_INT32_TO_4chars: Convert an int into a 4 Character ID String
 *  Returns a pointer to a static string containing the ASCII ID
 *  Users should copy the ID string as it gets overwritten on each call.
 */
char * SPDB_int32_to_4chars(si32 id_int);

#endif

#ifdef __cplusplus
}
#endif
