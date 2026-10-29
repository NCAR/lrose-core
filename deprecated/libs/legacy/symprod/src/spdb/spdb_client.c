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
 * spdb_client.c
 *
 * Routines used by clients of an SPDB server
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * Sept 1996
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#include <toolsa/os_config.h>

#include <dataport/bigend.h>

#include <symprod/spdb.h>
#include <symprod/spdb_client.h>

#include <toolsa/db_access.h>
#include <toolsa/servmap.h>
#include <toolsa/smu.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <toolsa/pmu.h> /* Added by Niles December 1999 */

/*
 * Local defines
 */

#define SPDB_CLIENT_WAIT_MSECS      -1  /* wait forever because we are */
                                        /* now spawning a child process */
                                        /* to handle the communications */
#define SPDB_CLIENT_REPLY_TIMEOUT 5000  /* only wait 5 seconds for the */
                                        /* client to send a reply.  That */
                                        /* way we don't hang on the reply. */
#define SPDB_CLIENT_MAX_CHILDREN    64

/*
 * Local typedefs
 */

/*
 * File scope variables
 */

static int SockFd = -1;
static int SockFdOpen = FALSE;
static int NumChildren = 0;

/**************************
 * file scope buffers
 * 
 * These hold the chunk hdrs and chunk data for the get() calls.
 * They are allocated using alloc_for_gets().
 * They can be freed using SPDB_free_get().
 */

static spdb_chunk_ref_t *Chunk_hdr_buffer = NULL;
static ui08 *Chunk_data_buffer = NULL;

/*
 * file scope prototypes
 */

static void alloc_for_gets(int chunk_hdr_len,
			   int chunk_data_len);
     
static void copy_fetched_to_static(int n_chunks,
				   spdb_chunk_ref_t *chunk_hdrs,
				   void *chunk_data);

static int put_to_socket(char *host,
			 int port,
			 si32 product_id,
			 si32 request_type,
			 ui32 nchunks,
			 spdb_chunk_ref_t *chunk_hdrs,
			 void *chunk_data,
			 ui32 chunk_data_len,
			 FILE *log_file);

static int put_to_child_socket(char *host,
			       int port,
			       si32 product_id,
			       si32 request_type,
			       ui32 nchunks,
			       spdb_chunk_ref_t *chunk_hdrs,
			       void *chunk_data,
			       ui32 chunk_data_len);

static int get_from_socket(char *host,
			   int port,
			   si32 product_id,
			   spdb_request_t request_type,
			   si32 *request_info,
			   int request_info_len,
			   ui32 *nchunks,
			   spdb_chunk_ref_t **chunk_hdrs,
			   void **chunk_data);

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
	     int chunk_data_len)
{
  static char *routine_name = "SPDB_put";
  
  int loc_type = DB_location_type(destination);
  
  /*
   * See if we're getting from a socket or from disk.
   */
  switch(loc_type)
  {
  case DB_LOCATION_DISK :
  {
    spdb_handle_t handle;
    int i;
    
    /*
     * Initialize the SPDB information
     */

    if (SPDB_init(&handle,
		  product_label,
		  product_id,
		  destination) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error initializing SPDB handle for directory <%s>\n",
	      destination);
      return(-1);
    }
    
    /*
     * Store the information.
     */

    for (i = 0; i < nchunks; i++)
    {
      if (SPDB_store(&handle,
		     chunk_hdrs[i].data_type,
		     chunk_hdrs[i].valid_time,
		     chunk_hdrs[i].expire_time,
		     (void *)((char *)chunk_data + chunk_hdrs[i].offset),
		     chunk_hdrs[i].len) != 0)
      {
	fprintf(stderr, "ERROR - spdb:%s\n",
		routine_name);
	fprintf(stderr, "Error storing data for chunk %d\n", i);
	return(-1);
      }
    }
    
    /*
     * Free the SPDB handle
     */

    SPDB_free(&handle);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  {
    char host[MAX_HOST_LEN];
    int port;
  
    /*
     * Get the host and port from the destination string.
     */

    if (DB_get_host_port(destination, host, MAX_HOST_LEN, &port) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      destination);
      return(-1);
    }
  
    if (put_to_child_socket(host,
		      port,
		      product_id,
		      SPDB_PUT_DATA,
		      nchunks,
		      chunk_hdrs,
		      chunk_data,
		      chunk_data_len) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET */

  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    
    /*
     * Get the host and port from the server mapper.
     */

    DB_get_servmap_host_port(destination,
			     host, MAX_HOST_LEN,
			     &port);
    
    if (put_to_child_socket(host,
			    port,
			    product_id,
			    SPDB_PUT_DATA,
			    nchunks,
			    chunk_hdrs,
			    chunk_data,
			    chunk_data_len) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}

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
		 int chunk_data_len)
{
  static char *routine_name = "SPDB_put_add";
  
  int loc_type = DB_location_type(destination);
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch(loc_type)
  {
  case DB_LOCATION_DISK :
  {
    spdb_handle_t handle;
    int i;
    
    /*
     * Initialize the SPDB information
     */

    if (SPDB_init(&handle,
		  product_label,
		  product_id,
		  destination) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error initializing SPDB handle for directory <%s>\n",
	      destination);
      return(-1);
    }
    
    /*
     * Store the information.
     */

    for (i = 0; i < nchunks; i++)
    {
      if (SPDB_store_add(&handle,
			 chunk_hdrs[i].data_type,
			 chunk_hdrs[i].valid_time,
			 chunk_hdrs[i].expire_time,
			 (void *)((char *)chunk_data + chunk_hdrs[i].offset),
			 chunk_hdrs[i].len) != 0)
      {
	fprintf(stderr, "ERROR - spdb:%s\n",
		routine_name);
	fprintf(stderr, "Error storing data for chunk %d\n", i);
	return(-1);
      }
    }
    
    /*
     * Free the SPDB handle
     */

    SPDB_free(&handle);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  {
    char host[MAX_HOST_LEN];
    int port;
  
    /*
     * Get the host and port from the destination string.
     */

    if (DB_get_host_port(destination, host, MAX_HOST_LEN, &port) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      destination);
      return(-1);
    }
  
    if (put_to_child_socket(host,
			    port,
			    product_id,
			    SPDB_PUT_DATA_ADD,
			    nchunks,
			    chunk_hdrs,
			    chunk_data,
			    chunk_data_len) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET */

  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
  
    /*
     * Get the host and port from the server mapper.
     */

    if (DB_get_servmap_host_port(destination, host,
				 MAX_HOST_LEN, &port) != 0)
      return(-1);
  
    if (put_to_child_socket(host,
			    port,
			    product_id,
			    SPDB_PUT_DATA_ADD,
			    nchunks,
			    chunk_hdrs,
			    chunk_data,
			    chunk_data_len) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}

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
		  int chunk_data_len)
{
  static char *routine_name = "SPDB_put_over";
  
  int loc_type = DB_location_type(destination);
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch(loc_type)
  {
  case DB_LOCATION_DISK :
  {
    spdb_handle_t handle;
    int i;
    
    /*
     * Initialize the SPDB information
     */

    if (SPDB_init(&handle,
		  product_label,
		  product_id,
		  destination) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error initializing SPDB handle for directory <%s>\n",
	      destination);
      return(-1);
    }
    
    /*
     * Store the information.
     */

    for (i = 0; i < nchunks; i++)
    {
      if (SPDB_store_over(&handle,
			  chunk_hdrs[i].data_type,
			  chunk_hdrs[i].valid_time,
			  chunk_hdrs[i].expire_time,
			  (void *)((char *)chunk_data + chunk_hdrs[i].offset),
			  chunk_hdrs[i].len) != 0)
      {
	fprintf(stderr, "ERROR - spdb:%s\n",
		routine_name);
	fprintf(stderr, "Error storing data for chunk %d\n", i);
	return(-1);
      }
      /*
       * Call to PMU added by Niles December 1999 
       */
      if ((i % 100) == 0) PMU_auto_register("Storing spdb data");
    }
    
    /*
     * Free the SPDB handle
     */

    SPDB_free(&handle);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  {
    char host[MAX_HOST_LEN];
    int port;
  
    /*
     * Get the host and port from the destination string.
     */

    if (DB_get_host_port(destination, host, MAX_HOST_LEN, &port) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      destination);
      return(-1);
    }
  
    if (put_to_child_socket(host,
			    port,
			    product_id,
			    SPDB_PUT_DATA_OVER,
			    nchunks,
			    chunk_hdrs,
			    chunk_data,
			    chunk_data_len) != 0)
    {
      return(-1);
    }
    
    break;
  } /* endcase - DB_LOCATION_SOCKET */

  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
  
    /*
     * Get the host and port from the server mapper.
     */

    if (DB_get_servmap_host_port(destination, host,
				 MAX_HOST_LEN, &port) != 0)
      return(-1);
  
    if (put_to_child_socket(host,
			    port,
			    product_id,
			    SPDB_PUT_DATA_OVER,
			    nchunks,
			    chunk_hdrs,
			    chunk_data,
			    chunk_data_len) != 0)
    {
      return(-1);
    }
    
    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}

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
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_get(char *source,
	     si32 product_id,
	     si32 data_type,
	     si32 request_time,
	     ui32 *nchunks,                  /* output */
	     spdb_chunk_ref_t **chunk_hdrs,  /* output */
	     void **chunk_data)              /* output */
{
  static char *routine_name = "SPDB_get";
  
  int loc_type = DB_location_type(source);
 
  /*
   * See if we're getting from a socket or from disk.
   */

  switch(loc_type)
  {
  case DB_LOCATION_DISK :
  {
    spdb_handle_t handle;
    si32 n;
    spdb_chunk_ref_t *hdrs = NULL;
    void *data = NULL;
    
    /*
     * Initialize the SPDB information
     */

    if (SPDB_init(&handle,
		  NULL,
		  product_id,
		  source) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error initializing SPDB handle for directory <%s>\n",
	      source);
      return(-1);
    }
    
    /*
     * Fetch the information.
     */

    if (SPDB_fetch(&handle, data_type, request_time,
		   &n, &hdrs, &data) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error fetching data for %s\n",
	      utimstr(request_time));
      return(-1);
    }

    /*
     * copy buffers to static memory, set return values
     */
    
    copy_fetched_to_static(n, hdrs, data);
    *nchunks = n;
    *chunk_hdrs = Chunk_hdr_buffer;
    *chunk_data = Chunk_data_buffer;

    /*
     * Free the SPDB handle
     */

    SPDB_free(&handle);
    
    break;
  } /* endcase - DB_LOCATION_DISK */
    
  case DB_LOCATION_SOCKET :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[2];
  
    /*
     * Get the host and port from the source string.
     */

    if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      source);
      return(-1);
    }
  
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = request_time;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET */
    
  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[2];
  
    /*
     * Get the host and port from the server mapper.
     */

    if (DB_get_servmap_host_port(source, host, MAX_HOST_LEN, &port) != 0)
      return(-1);
  
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = request_time;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}

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
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
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
		     void **chunk_data)              /* output */
{
  static char *routine_name = "SPDB_get_closest";
  
  int loc_type = DB_location_type(source);
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    spdb_handle_t handle;
    si32 n;
    spdb_chunk_ref_t *hdrs = NULL;
    void *data = NULL;
    
    /*
     * Initialize the SPDB information
     */

    if (SPDB_init(&handle,
		  NULL,
		  product_id,
		  source) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error initializing SPDB handle for directory <%s>\n",
	      source);
      return(-1);
    }
    
    /*
     * Fetch the information.
     */

    if (SPDB_fetch_closest(&handle, data_type, request_time, time_margin,
			   &n, &hdrs, &data) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error fetching data closest to %s\n",
	      utimstr(request_time));
      return(-1);
    }
    
    /*
     * copy buffers to static memory, set return values
     */
    
    copy_fetched_to_static(n, hdrs, data);
    *nchunks = n;
    *chunk_hdrs = Chunk_hdr_buffer;
    *chunk_data = Chunk_data_buffer;

    /*
     * Free the SPDB handle
     */

    SPDB_free(&handle);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[3];
    
    /*
     * Get the host and port from the source string.
     */

    if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      source);
      return(-1);
    }
  
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = request_time;
    request_info[2] = time_margin;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_CLOSEST,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET */

  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[3];
    
    /*
     * Get the host and port from the server mapper.
     */

    DB_get_servmap_host_port(source,
			     host, MAX_HOST_LEN,
			     &port);
    
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = request_time;
    request_info[2] = time_margin;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_CLOSEST,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}

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
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
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
			  void **chunk_data)              /* output */
{
  static char *routine_name = "SPDB_get_first_before";
  
  int loc_type = DB_location_type(source);
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    spdb_handle_t handle;
    si32 n;
    spdb_chunk_ref_t *hdrs = NULL;
    void *data = NULL;
    
    /*
     * Initialize the SPDB information
     */

    if (SPDB_init(&handle,
		  NULL,
		  product_id,
		  source) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error initializing SPDB handle for directory <%s>\n",
	      source);
      return(-1);
    }
    
    /*
     * Fetch the information.
     */

    if (SPDB_fetch_first_before(&handle, data_type, request_time, time_margin,
				&n, &hdrs, &data) != 0)
      {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error fetching data first_before to %s\n",
	      utimstr(request_time));
      return(-1);
    }
    
    /*
     * copy buffers to static memory, set return values
     */
    
    copy_fetched_to_static(n, hdrs, data);
    *nchunks = n;
    *chunk_hdrs = Chunk_hdr_buffer;
    *chunk_data = Chunk_data_buffer;

    /*
     * Free the SPDB handle
     */

    SPDB_free(&handle);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[3];
    
    /*
     * Get the host and port from the source string.
     */

    if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      source);
      return(-1);
    }
  
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = request_time;
    request_info[2] = time_margin;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_FIRST_BEFORE,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET */

  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[3];
    
    /*
     * Get the host and port from the server mapper.
     */

    DB_get_servmap_host_port(source,
			     host, MAX_HOST_LEN,
			     &port);
    
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = request_time;
    request_info[2] = time_margin;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_FIRST_BEFORE,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}

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
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
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
			 void **chunk_data)              /* output */
{
  static char *routine_name = "SPDB_get_first_after";
  
  int loc_type = DB_location_type(source);
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    spdb_handle_t handle;
    si32 n;
    spdb_chunk_ref_t *hdrs = NULL;
    void *data = NULL;
    
    /*
     * Initialize the SPDB information
     */

    if (SPDB_init(&handle,
		  NULL,
		  product_id,
		  source) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error initializing SPDB handle for directory <%s>\n",
	      source);
      return(-1);
    }
    
    /*
     * Fetch the information.
     */

    if (SPDB_fetch_first_after(&handle, data_type, request_time, time_margin,
			       &n, &hdrs, &data) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error fetching data first_after to %s\n",
	      utimstr(request_time));
      return(-1);
    }
    
    /*
     * copy buffers to static memory, set return values
     */
    
    copy_fetched_to_static(n, hdrs, data);
    *nchunks = n;
    *chunk_hdrs = Chunk_hdr_buffer;
    *chunk_data = Chunk_data_buffer;

   /*
     * Free the SPDB handle
     */

    SPDB_free(&handle);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[3];
    
    /*
     * Get the host and port from the source string.
     */

    if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      source);
      return(-1);
    }
  
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = request_time;
    request_info[2] = time_margin;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_FIRST_AFTER,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET */

  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[3];
    
    /*
     * Get the host and port from the server mapper.
     */

    DB_get_servmap_host_port(source,
			     host, MAX_HOST_LEN,
			     &port);
    
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = request_time;
    request_info[2] = time_margin;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_FIRST_AFTER,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}

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
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
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
		      void **chunk_data)              /* output */
{
  static char *routine_name = "SPDB_get_interval";
  
  int loc_type = DB_location_type(source);
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type) 
  {
  case DB_LOCATION_DISK :
    
  {
    spdb_handle_t handle;
    si32 n;
    spdb_chunk_ref_t *hdrs = NULL;
    void *data = NULL;
    
    /*
     * Initialize the SPDB information
     */

    if (SPDB_init(&handle,
		  NULL,
		  product_id,
		  source) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error initializing SPDB handle for directory <%s>\n",
	      source);
      return(-1);
    }
    
    /*
     * Fetch the information.
     */

    if (SPDB_fetch_interval(&handle, data_type, start_time, end_time,
			    &n, &hdrs, &data) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error fetching data for start time %s, end time %s\n",
	      utimstr(start_time), utimstr(end_time));
      return(-1);
    }
    
    /*
     * copy buffers to static memory, set return values
     */
    
    copy_fetched_to_static(n, hdrs, data);
    *nchunks = n;
    *chunk_hdrs = Chunk_hdr_buffer;
    *chunk_data = Chunk_data_buffer;

    /*
     * Free the SPDB handle
     */

    SPDB_free(&handle);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[3];
    
    /*
     * Get the host and port from the source string.
     */

    if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      source);
      return(-1);
    }
  
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = start_time;
    request_info[2] = end_time;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_INTERVAL,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET */

  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[3];
    
    /*
     * Get the host and port from the server mapper.
     */

    if (DB_get_servmap_host_port(source, host, MAX_HOST_LEN, &port) != 0)
      return(-1);
  
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = start_time;
    request_info[2] = end_time;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_INTERVAL,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}

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
 * MEMORY POLICY.
 * Do not use free() to free up chunk_hdrs or chunk_data pointers -
 * they are static to the spdb library. If you wish to free up
 * the memory between calls, use SPDB_free_get(). If you do
 * not free the memory, it will be realloc'd during the next
 * call to a get() routine.
 *
 * Returns 0 on success, -1 on failure
 */

int SPDB_get_valid(char *source,
		   si32 product_id,
		   si32 data_type,
		   si32 search_time,
		   ui32 *nchunks,                  /* output */
		   spdb_chunk_ref_t **chunk_hdrs,  /* output */
		   void **chunk_data)              /* output */
{
  static char *routine_name = "SPDB_get_valid";
  
  int loc_type = DB_location_type(source);
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    spdb_handle_t handle;
    si32 n;
    spdb_chunk_ref_t *hdrs = NULL;
    void *data = NULL;
    
    /*
     * Initialize the SPDB information
     */

    if (SPDB_init(&handle,
		  NULL,
		  product_id,
		  source) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error initializing SPDB handle for directory <%s>\n",
	      source);
      return(-1);
    }
    
    /*
     * Fetch the information.
     */

    if (SPDB_fetch_valid(&handle, data_type, search_time,
			 &n, &hdrs, &data) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Error fetching data valid at %s\n",
	      utimstr(search_time));
      return(-1);
    }
    
    /*
     * copy buffers to static memory, set return values
     */
    
    copy_fetched_to_static(n, hdrs, data);
    *nchunks = n;
    *chunk_hdrs = Chunk_hdr_buffer;
    *chunk_data = Chunk_data_buffer;

    /*
     * Free the SPDB handle
     */

    SPDB_free(&handle);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[2];
    
    /*
     * Get the host and port from the source string.
     */

    if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      source);
      return(-1);
    }
  
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = search_time;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_VALID,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET */

  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    si32 request_info[2];
    
    /*
     * Get the host and port from the server mapper.
     */

    if (DB_get_servmap_host_port(source, host, MAX_HOST_LEN, &port) != 0)
      return(-1);
  
    /*
     * Put together the request information buffer.
     */

    request_info[0] = data_type;
    request_info[1] = search_time;
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			product_id,
			SPDB_GET_DATA_VALID,
			request_info,
			sizeof(request_info),
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}

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

void SPDB_free_get(void)
     
{

  ufree (Chunk_hdr_buffer);
  Chunk_hdr_buffer = NULL;

  ufree (Chunk_data_buffer);
  Chunk_data_buffer = NULL;

}

/*****************************************************
 * SPDB_handle_sigpipe()
 *
 * Closes any open sockets that were used by the SPDB
 * routines.  Should be called by the application's
 * interrupt handler when a SIGPIPE interrupt is
 * received.
 * 
 */

void SPDB_handle_sigpipe(void)
{
  /*
   * Close any open sockets.
   */

  if (SockFdOpen)
  {
    SKU_close(SockFd);
    SockFdOpen = FALSE;
  }
  
  return;
}


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
			    int get_latest_data_only)
{
  spdb_poll_t *poll_struct = (spdb_poll_t *)umalloc(sizeof(spdb_poll_t));
  
  poll_struct->product_id = product_id;
  poll_struct->data_type = data_type;
  poll_struct->latest_only = get_latest_data_only;
  poll_struct->last_data = time(NULL);
  
  STRcopy(poll_struct->request.server_type,
	  servmap_type, SERVMAP_NAME_MAX);
  STRcopy(poll_struct->request.server_subtype,
	  servmap_subtype, SERVMAP_NAME_MAX);
  STRcopy(poll_struct->request.instance,
	  servmap_instance, SERVMAP_INSTANCE_MAX);
  poll_struct->request.want_realtime = FALSE;
  poll_struct->request.time = -1;
  
  SMU_get_servmap_hosts(&poll_struct->servmap_host1,
			&poll_struct->servmap_host2);
  
  return(poll_struct);
}


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
	      void **chunk_data)                /* output */
{
  int nservers;
  SERVMAP_info_t *info;
  int smu_status;
  
  /*
   * Initialize the returned values in case there is an error.
   */

  *nchunks = 0;
  *chunk_hdrs = NULL;
  *chunk_data = NULL;
  
  /*
   * Get the server information from the server mapper.
   */

  poll_struct->request.time = poll_struct->last_data;
  
  smu_status = SMU_requestInfo(&poll_struct->request,
			       &nservers,
			       &info,
			       poll_struct->servmap_host1,
			       poll_struct->servmap_host2);
  
  if (smu_status == 0)
    return(SPDB_SMU_NO_SERVERS);
  else if (smu_status != 1)
    return(SPDB_SMU_INFO_ERROR);
  
  /*
   * See if there is any new data.  Assume that the first returned
   * server is the one we want.
   */
  
  if (info[0].last_data > poll_struct->last_data)
  {
    char source[MAX_HOST_LEN];
    
    /*
     * Determine the data source.
     */

    sprintf(source, "%d@%s", info[0].port, info[0].host);
    
    /*
     * Get the data from the server.
     */

    if (poll_struct->latest_only)
    {
      si32 request_time = time(NULL);
      si32 time_margin = request_time - poll_struct->last_data;
      
      if (SPDB_get_closest(source,
			   poll_struct->product_id,
			   poll_struct->data_type,
			   request_time,
			   time_margin,
			   nchunks,
			   chunk_hdrs,
			   chunk_data) != 0)
      {
	fprintf(stderr,
		"ERROR: spdb:SPDB_poll\n");
	fprintf(stderr,
		"Error getting data from source <%s>\n", source);
	
	return(SPDB_DATA_GET_ERROR);
      }
      
    }
    else
    {
      si32 start_time = poll_struct->last_data;
      si32 end_time = time(NULL);
      
      if (SPDB_get_interval(source,
			    poll_struct->product_id,
			    poll_struct->data_type,
			    start_time,
			    end_time,
			    nchunks,
			    chunk_hdrs,
			    chunk_data) != 0)
      {
	fprintf(stderr,
		"ERROR: spdb:SPDB_poll\n");
	fprintf(stderr,
		"Error getting data from source <%s>\n", source);
	
	return(SPDB_DATA_GET_ERROR);
      }
      
    }
    
    /*
     * Update our last_data time.
     */

    poll_struct->last_data = info[0].last_data;
  }
  else
    return(SPDB_NO_DATA_AVAIL);
  
  return(SPDB_SUCCESS);
}


/*****************************************************
 * SPDB_poll_delete()
 *
 * Deletes the SPDB poll structure and frees all
 * allocated space.
 */

void SPDB_poll_delete(spdb_poll_t **poll_struct)
{
  ufree(*poll_struct);
  
  *poll_struct = NULL;
}


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

char *SPDB_request2string(int request)
{
  switch(request)
  {
  case SPDB_GET_DATA :
    return("SPDB_GET_DATA");
    break;
    
  case SPDB_GET_DATA_CLOSEST :
    return("SPDB_GET_DATA_CLOSEST");
    break;
    
  case SPDB_GET_DATA_INTERVAL :
    return("SPDB_GET_DATA_INTERVAL");
    break;
    
  case SPDB_GET_DATA_VALID :
    return("SPDB_GET_DATA_VALID");
    break;
    
  case SPDB_PUT_DATA :
    return("SPDB_PUT_DATA");
    break;
    
  case SPDB_PUT_DATA_ADD :
    return("SPDB_PUT_DATA_ADD");
    break;
    
  case SPDB_PUT_DATA_OVER :
    return("SPDB_PUT_DATA_OVER");
    break;
    
  default:
    return("Unknown SPDB request");
    break;
  }

  return("Unknown SPDB request");
}

/*****************************************************
 * SPDB_reply2string()
 *
 * Converts a reply value to the matching string.
 *
 * Returns a pointer to a static area containing the string.
 * Do NOT free this pointer.
 */

char *SPDB_reply2string(int reply)
{
  static char unknown[128];

  switch(reply)
  {
  case SPDB_DATA :
    return("SPDB_DATA");
    break;
    
  case SPDB_NO_DATA :
    return("SPDB_NO_DATA");
    break;
    
  case SPDB_DATA_ERROR :
    return("SPDB_DATA_ERROR");
    break;
    
  case SPDB_PUT_SUCCESSFUL :
    return("SPDB_PUT_SUCCESSFUL");
    break;
    
  case SPDB_PUT_FAILED :
    return("SPDB_PUT_FAILED");
    break;
    
  case SPDB_REQUEST_ERROR :
    return("SPDB_REQUEST_ERROR");
    break;
    
  default:
    break;
  }

  sprintf(unknown, "Unknown SPDB reply <%d>", reply);
  return(unknown);

}

/*****************************************************
 * SPDB_reap_children()
 *
 * Check for any child processes that are finished and
 * clean them up.  This keeps us from getting overwhelmed
 * by "zombie" processes.
 */

void SPDB_reap_children(void)
{
  while (waitpid((pid_t)-1,
		 (int *)NULL,
		 (int)(WNOHANG | WUNTRACED)) > 0)
  {
    if (NumChildren > 0)
      NumChildren--;
  }
}

/*************************************************
 * STATIC functions
 *************************************************/

/*****************************************************
 * put_to_child_socket()
 *
 * Spawns a child process to handle the communications just
 * in case the communications lines are slow.
 *
 * Returns 0 on success, -1 on failure
 */

static int put_to_child_socket(char *host,
			       int port,
			       si32 product_id,
			       si32 request_type,
			       ui32 nchunks,
			       spdb_chunk_ref_t *chunk_hdrs,
			       void *chunk_data,
			       ui32 chunk_data_len)
{
  static char *routine_name = "put_to_child_socket";
  
  int pid;
  char *child_log_dir;
  char child_log_filename[MAX_PATH_LEN];
  FILE *child_log_file = NULL;
  
  /*
   * See if we are logging what the child process is doing
   * for debugging purposes.
   */

  child_log_dir = getenv("DIST_SPDB_DATA_CHILD_LOG_DIR");
  
  /*
   * Make sure the number of child processes is below the
   * maximum.
   */

  while (NumChildren > SPDB_CLIENT_MAX_CHILDREN)
  {
    SPDB_reap_children();
    sleep(1);
  }
  
  /*
   * Spawn a child process to handle the communications
   */

  if ((pid = fork()) != 0)
  {
    /*
     * This is the parent process.
     */

    /*
     * Update the count of child processes.
     */

    NumChildren++;
    
    /*
     * Reap any children who are finished.  There should generally
     * just be one.  This keeps us from having a bunch of "defunct"
     * processes lying around until the next communication.
     */

    SPDB_reap_children();
    
    /* Return "successful".  In the future we may need to add
     * code to determine if there are errors in processing the
     * request.
     */

    return(0);
  }
  else
  {
    /*
     * Open the child logging file.
     */

    if (child_log_dir != NULL)
    {
      int child_pid;
      
      /*
       * Determine the log filename.
       */

      child_pid = getpid();
      sprintf(child_log_filename, "%s/dist_spdb_data_child.%d",
	      child_log_dir, child_pid);
      
      /*
       * Open the log file.
       */

      if ((child_log_file = fopen(child_log_filename, "w")) == NULL)
      {
	fprintf(stderr,
		"ERROR - symprod:%s\n",
		routine_name);
	fprintf(stderr,
		"Error opening child log file\n");
	perror(child_log_filename);
	
	child_log_dir = NULL;
      }
      
    } /* endif - child_log_dir */
    
    /*
     * Log the host and port we are servicing.
     */

    if (child_log_dir != NULL)
    {
      fprintf(child_log_file,
	      "Servicing host %s, port %d\n",
	      host, port);
      fflush(child_log_file);
    }
    
    /*
     * We are in the child process here.  Send the information
     * out the socket.
     */

    put_to_socket(host,
		  port,
		  product_id,
		  request_type,
		  nchunks,
		  chunk_hdrs,
		  chunk_data,
		  chunk_data_len,
		  child_log_file);
  
    /*
     * Close and delete the log file since we are done processing.
     */

    if (child_log_dir != NULL)
    { 
      fclose(child_log_file);
      unlink(child_log_filename);
    } /* endif - child_log_dir */
    
    /*
     * Kill the child after putting the information.
     */

    _exit(0);
  }
  
}

/******************
 * alloc_for_gets()
 *
 * Allocs space for Chunk_hdr_buffer & Chunk_data_buffer.
 * These are used by the get() calls.
 */

static void alloc_for_gets(int chunk_hdr_len,
			   int chunk_data_len)
     
{

  if (Chunk_hdr_buffer == NULL) {
    Chunk_hdr_buffer = (spdb_chunk_ref_t *) umalloc(chunk_hdr_len);
  } else {
    Chunk_hdr_buffer = (spdb_chunk_ref_t *) urealloc(Chunk_hdr_buffer,
						     chunk_hdr_len);
  }
  
  if (Chunk_data_buffer == NULL) {
    Chunk_data_buffer = (ui08 *) umalloc(chunk_data_len);
  } else {
    Chunk_data_buffer = (ui08 *) urealloc(Chunk_data_buffer,
					  chunk_data_len);
  }

}

/**************************
 * copy_fetched_to_static()
 *
 * Copies the fetched buffers to static memory.
 */

static void copy_fetched_to_static(int n_chunks,
				   spdb_chunk_ref_t *chunk_hdrs,
				   void *chunk_data)

{

  int i;
  int chunk_hdr_len;
  int chunk_data_len;
  spdb_chunk_ref_t *hdr;

  chunk_hdr_len = n_chunks * sizeof(spdb_chunk_ref_t);

  hdr = chunk_hdrs;
  chunk_data_len = 0;
  for (i = 0; i < n_chunks; i++, hdr++) {
    chunk_data_len += hdr->len;
  }

  alloc_for_gets(chunk_hdr_len, chunk_data_len);
  
  memcpy(Chunk_hdr_buffer, chunk_hdrs, chunk_hdr_len);
  memcpy(Chunk_data_buffer, chunk_data, chunk_data_len);

}

/*****************************************************
 * put_to_socket()
 *
 * Puts an array of SPDB chunks on the given destination
 * socket.
 *
 * The chunk headers will be byte-swapped, as appropriate,
 * by this routine.  The chunk data must be put into big-
 * endian format by the caller before calling this routine.
 *
 * Returns 0 on success, -1 on failure
 */

static int put_to_socket(char *host,
			 int port,
			 si32 product_id,
			 si32 request_type,
			 ui32 nchunks,
			 spdb_chunk_ref_t *chunk_hdrs,
			 void *chunk_data,
			 ui32 chunk_data_len,
			 FILE *log_file)
{
  static char *routine_name = "put_to_socket";
  
  char *msg_buffer;
  char *buf_ptr;
  int buffer_len;
  long buffer_len_long;
  si32 reply;
  SKU_header_t sku_header;
  
  /*
   * Make sure we are using the new headers.
   */

  SKU_set_headers_to_new();
  
  if (log_file != NULL)
  {
    fprintf(log_file,
	    "Calling SKU_open_client on host %s, port %d\n",
	    host, port);
    fflush(log_file);
  }
  
  /*
   * Open the socket from the client to the server.
   */

  if ((SockFd = SKU_open_client(host, port)) < 0)
  {
    if (log_file != NULL)
    {
      fprintf(log_file,
	      "Could not open client socket on host %s, port %d\n",
	      host, port);
      fflush(log_file);
    }
    
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Could not open client socket on host %s, port %d\n",
	    host, port);

    if (log_file != NULL)
    {
      fprintf(log_file,
	      "Finished printing error message to stderr\n");
      fflush(log_file);
    }
    
    return(-1);
  }
  
  SockFdOpen = TRUE;
  
  if (log_file != NULL)
  {
    fprintf(log_file,
	    "Constructing output buffer\n");
    fflush(log_file);
  }
  
  /*
   * Construct the buffer to send to the server.  Byte-swap the
   * header information as we do so.
   */

  buffer_len = sizeof(si32) + (2 * sizeof(ui32)) +
    (nchunks * sizeof(spdb_chunk_ref_t)) + chunk_data_len;
  
  msg_buffer = umalloc(buffer_len);
  
  buf_ptr = msg_buffer;
  
  /* request */
  memcpy(buf_ptr, &request_type, sizeof(si32));
  BE_from_array_32(buf_ptr, sizeof(si32));
  buf_ptr += sizeof(si32);
  
  /* nchunks */
  memcpy(buf_ptr, &nchunks, sizeof(ui32));
  BE_from_array_32(buf_ptr, sizeof(ui32));
  buf_ptr += sizeof(ui32);
  
  /* data length */
  memcpy(buf_ptr, &chunk_data_len, sizeof(ui32));
  BE_from_array_32(buf_ptr, sizeof(ui32));
  buf_ptr += sizeof(ui32);
  
  /* chunk headers */
  memcpy(buf_ptr, chunk_hdrs, nchunks * sizeof(spdb_chunk_ref_t));
  BE_from_array_32(buf_ptr, nchunks * sizeof(spdb_chunk_ref_t));
  buf_ptr += nchunks * sizeof(spdb_chunk_ref_t);
  
  /* chunk data - byte swapping done by caller */
  memcpy(buf_ptr, chunk_data, chunk_data_len);
  
  if (log_file != NULL)
  {
    fprintf(log_file,
	    "Sending output buffer\n");
    fflush(log_file);
  }
  
  /*
   * Send the buffer to the server.
   */

  if (SKU_write_message(SockFd, product_id,
			msg_buffer, buffer_len) != 1)
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Error writing message to host %s, port %d\n",
	    host, port);
    
    SKU_close(SockFd);
    SockFdOpen = FALSE;
    ufree(msg_buffer);
    return(-1);
  }
  
  ufree(msg_buffer);
  
  if (log_file)
  {
    fprintf(log_file,
	    "Waiting for response from client\n");
    fflush(log_file);
  }
  
  /*
   * Get the response from the server.  Note that you should NOT free
   * the msg_buffer pointer after this call.  SKU_read_message sets
   * this pointer to a static buffer in the routine.
   */

  if (SKU_read_message(SockFd, &sku_header,
		       &msg_buffer, &buffer_len_long,
		       SPDB_CLIENT_REPLY_TIMEOUT) != 1)
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Error reading reply from server on host %s, port %d\n",
	    host, port);
    
    SKU_close(SockFd);
    SockFdOpen = FALSE;
    
    return(-1);
  }
  
  /*
   * Close the socket.
   */

  SKU_close(SockFd);
  SockFdOpen = FALSE;
  
  if (log_file != NULL)
  {
    fprintf(log_file,
	    "Checking response from client\n");
    fflush(log_file);
  }
  
  /*
   * Check the response from the server.  Right now, all of the responses
   * for the put messages are one si32 reply.
   */

  if (buffer_len_long != sizeof(si32))
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr,
	    "Response from server should be %ld bytes, received %ld bytes\n",
	    (long int)sizeof(si32), (long int)buffer_len_long);
    
    return(-1);
  }
  
  reply = *(si32 *)msg_buffer;
  reply = BE_from_si32(reply);
  if (reply != SPDB_PUT_SUCCESSFUL)
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Server returned error code %s for put operation to %d at %s\n",
	    SPDB_reply2string(reply), port, host);
    
    return(-1);
  }
  
  return(0);
}

/*****************************************************
 * get_from_socket()
 *
 * Gets an array of SPDB chunks from the given source socket.
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

static int get_from_socket(char *host,
			   int port,
			   si32 product_id,
			   spdb_request_t request_type,
			   si32 *request_info,
			   int request_info_len,
			   ui32 *nchunks,                  /* output */
			   spdb_chunk_ref_t **chunk_hdrs,  /* output */
			   void **chunk_data)              /* output */
{
  static char *routine_name = "get_from_socket";
  
  si32 request = request_type;
  char *msg_buffer;
  char *buf_ptr;
  int buffer_len;
  long buffer_len_long;
  si32 reply;
  SKU_header_t sku_header;
  
  /*
   * Initialize the returned information in case there is an error.
   */

  *nchunks = 0;
  *chunk_hdrs = NULL;
  *chunk_data = NULL;
  
  /*
   * Make sure we are using the new headers.
   */

  SKU_set_headers_to_new();
  
  /*
   * Open the socket from the client to the server.
   */

  if ((SockFd = SKU_open_client(host, port)) < 0)
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Could not open client socket on host %s, port %d\n",
	    host, port);
    return(-1);
  }
  
  SockFdOpen = TRUE;
  
  /*
   * Construct the request to send to the server.  Byte-swap the
   * header information as we do so.
   */

  buffer_len = sizeof(si32) + request_info_len;
  
  msg_buffer = umalloc(buffer_len);
  
  buf_ptr = msg_buffer;
  
  /* request */
  memcpy(buf_ptr, &request, sizeof(si32));
  BE_from_array_32(buf_ptr, sizeof(si32));
  buf_ptr += sizeof(si32);
  
  /* request_info */
  if (request_info != NULL)
  {
    memcpy(buf_ptr, request_info, request_info_len);
    BE_from_array_32(buf_ptr, request_info_len);
  }
  
  /*
   * Send the request to the server.
   */

  if (SKU_write_message(SockFd, product_id,
			msg_buffer, buffer_len) != 1)
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Error writing message to host %s, port %d\n",
	    host, port);
    
    SKU_close(SockFd);
    SockFdOpen = FALSE;
    ufree(msg_buffer);
    return(-1);
  }
  
  ufree(msg_buffer);
  
  /*
   * Get the response from the server.
   */

  if (SKU_read_message(SockFd, &sku_header,
		       &msg_buffer, &buffer_len_long,
		       SPDB_CLIENT_WAIT_MSECS) != 1)
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Error reading reply from server on host %s, port %d\n",
	    host, port);
    
    SKU_close(SockFd);
    SockFdOpen = FALSE;
    
    return(-1);
  }
  
  /*
   * Close the socket.
   */

  SKU_close(SockFd);
  SockFdOpen = FALSE;
  
  /*
   * Extract the reply type from the message, byte-swapping as necessary.
   */

  buf_ptr = msg_buffer;

  reply = *(si32 *)buf_ptr;
  reply = BE_to_si32(reply);
  buf_ptr += sizeof(si32);
  
  switch (reply)
  {
  case SPDB_DATA :
  {
    ui32 chunks_returned;
    ui32 data_length_returned;
    spdb_chunk_ref_t *chunk_hdrs_returned;
    ui08 *chunk_data_returned;
    si32 chunk_hdr_len;
    
    long expected_buffer_len;
    
    /*
     * Extract the length information from the message.
     */

    chunks_returned = *(ui32 *)buf_ptr;
    chunks_returned = BE_to_si32(chunks_returned);
    buf_ptr += sizeof(ui32);
    
    data_length_returned = *(ui32 *)buf_ptr;
    data_length_returned = BE_to_si32(data_length_returned);
    buf_ptr += sizeof(ui32);
    
    /*
     * Make sure the message length is okay
     */

    expected_buffer_len = sizeof(si32) +    /* reply type */
                          sizeof(ui32) +    /* nchunks */
                          sizeof(ui32) +    /* data length */
                          sizeof(spdb_chunk_ref_t) * chunks_returned +
                          data_length_returned;
    
    if (buffer_len_long != expected_buffer_len)
    {
      fprintf(stderr, "ERROR - spdb:%s\n",
	      routine_name);
      fprintf(stderr, "Wrong number of bytes in reply.  Expected %d, got %d\n",
	      (int) expected_buffer_len, (int) buffer_len_long);
      fprintf(stderr, "nchunks = %d\n", chunks_returned);
      
      return(-1);
    }
    
    /*
     * Allocate space for the data in the message.
     */

    chunk_hdr_len = chunks_returned * sizeof(spdb_chunk_ref_t);

    alloc_for_gets(chunk_hdr_len, data_length_returned);
    chunk_hdrs_returned = Chunk_hdr_buffer;
    chunk_data_returned = Chunk_data_buffer;

    /*
     * Copy the data from the message.
     */

    memcpy(chunk_hdrs_returned, buf_ptr, chunk_hdr_len);
    BE_to_array_32(chunk_hdrs_returned, chunk_hdr_len);
    buf_ptr += chunk_hdr_len;
    
    memcpy(chunk_data_returned, buf_ptr, data_length_returned);
    
    /*
     * Set the return values.
     */

    *nchunks = chunks_returned;
    *chunk_hdrs = chunk_hdrs_returned;
    *chunk_data = chunk_data_returned;
    
    break;
  }
    
  case SPDB_NO_DATA :
    *nchunks = 0;
    *chunk_hdrs = NULL;
    *chunk_data = NULL;
    break;
    
  case SPDB_DATA_ERROR :
    return(-1);
    break;
    
  default:
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Invalid reply %d received from server\n",
	    reply);
    
    return(-1);
    break;
  }
  
  return(0);
}

/******************************************************************************
 * SPDB_4CHARS_TO_INT32: Convert the first 4 characters of an ID STRING to a int
 * On error Returns -1.  This function guarnteed to never return 0
 */
si32 SPDB_4chars_to_int32(char *id_string)
{
    int i;
    int len;
    si32 value = 0;

    if(id_string == NULL) return -1;

    len = strlen(id_string);

    if(len == 0) return -1; 
    if(len > 4) len = 4;  /* only use the first four characters */

    for(i= 0 ; i < len; i++) {
        value |= ((int) id_string[i]) << (i * 8);
    }

    if(value == 0) value = 1;

    return value;
}

/******************************************************************************
 * SPDB_INT32_TO_4chars: Convert an int into a 4 Character ID String
 *  Returns a pointer to a static string containing the ASCII ID
 *  Users should copy the ID string as it gets overwritten on each call.
 */
char * SPDB_int32_to_4chars(si32 id_int)
{
    static char id_string[8]; 

    memset(id_string, 0, 8);

    id_string[3] = (char) ((id_int & 0xff000000) >> 24);
    id_string[2] = (char) ((id_int & 0x00ff0000) >> 16);
    id_string[1] = (char) ((id_int & 0x0000ff00) >> 8);
    id_string[0] = (char) (id_int & 0x000000ff);

    return id_string;
}
