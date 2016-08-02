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
 * mdv_client.c
 *
 * Routines used by clients of an MDV server
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * August 1998
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
#include <didss/ds_input_path.h>
#include <Mdv/mdv/mdv_client.h>
#include <Mdv/mdv/mdv_composite.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_write.h>
#include <toolsa/db_access.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

/*
 * Local defines
 */

#define MDV_CLIENT_WAIT_MSECS      -1  /* wait forever because we are */
                                       /* now spawning a child process */
                                       /* to handle the communications */
#define MDV_CLIENT_REPLY_TIMEOUT 10000 /* only wait 10 seconds for the */
                                       /* client to send a reply.  That */
                                       /* way we don't hang on the reply. */
#define MDV_CLIENT_MAX_CHILDREN    64

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
 * These hold the data for the get() calls.
 * They are allocated using alloc_for_gets().
 * They can be freed using MDV_free_get().
 */

static MDV_handle_t Mdv_handle;
static int Mdv_handle_init_flag = FALSE;


/**************************
 * file scope prototypes
 */

static void alloc_for_gets(void);
     
static int put_to_socket(char *host,
			 int port,
			 si32 request_type,
			 MDV_handle_t *mdv_handle,
			 FILE *log_file);

static int put_to_child_socket(char *host,
			       int port,
			       int request_type,
			       MDV_handle_t *mdv_handle);

static int get_from_disk(char *filename,
			 int return_type,
			 char *field_name,
			 int field_num,
			 int plane_ht_type,
			 double plane_height,
			 MDV_request_crop_t *crop_info,
			 int composite_type,
			 MDV_handle_t *mdv_handle);

static int get_from_socket(char *host,
			   int port,
			   int request_type,
			   void *request_info,
			   int request_info_len,
			   MDV_handle_t *mdv_handle);

static MDV_dataset_time_t *get_dataset_times_from_socket(char *host,
							 int port,
							 MDV_dataset_time_request_t *request_info,
							 int *num_datasets);


/*****************************************************
 * MDV_put()
 *
 * Puts MDV data to the given destination.
 * The destination can be either a socket or disk location.
 *
 * For a disk destination, the destination string should
 * contain the directory path for the database.
 *
 * For a socket destination, the destination string should
 * contain the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater").
 *
 * The all data except chunk data will be byte-swapped, as
 * appropriate, by this routine.  The chunk data must be put
 * into big-endian format by the caller before calling this
 * routine.
 *
 * Returns 0 on success, -1 on failure
 */

int MDV_put(char *destination,
	    MDV_handle_t *mdv_handle)
{
  static char *routine_name = "MDV_put";
  
  int loc_type = DB_location_type(destination);
  
  /*
   * See if we're putting to a socket or from disk.
   */
  switch(loc_type)
  {
  case DB_LOCATION_DISK :
  {
    if (MDV_write_to_dir(mdv_handle, destination,
			 MDV_PLANE_RLE8, TRUE) == MDV_SUCCESS)
      return(0);
    
    return(-1);
    
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
      fprintf(stderr, "ERROR: mdv_client::%s\n",
	      routine_name);
      fprintf(stderr, "Could not parse <%s> for host and port\n",
	      destination);
      return(-1);
    }
  
    if (put_to_child_socket(host,
			    port,
			    MDV_PUT_DATA,
			    mdv_handle) != 0)
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
			    MDV_PUT_DATA,
			    mdv_handle) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SERVMAP */
  } /* endswitch - loc_type */
  
  return(0);
}


/*****************************************************
 * MDV_get_closest()
 *
 * Gets the MDV data from the given source stored at the
 * closest time to that requested within the requested
 * time margin.  The source can be either a socket or
 * disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater") or it
 * should contain the server mapper information for the
 * server in the form "type::subtype::instance" (e.g.
 * "MDV::MDV::kavm").
 *
 * Parameters:
 *      source          source string as described above.
 *      request_time    request time for data.
 *      time_margin     maximum time difference allowed
 *                        between requested time and
 *                        returned data time, in seconds.
 *      return_type     desired return type for data
 *                        (e.g. MDV_INT8).  Use MDV_NATIVE
 *                        to retrieve data in its stored
 *                        format.
 *      field_name      name of field to retrieve.  Only used
 *                        if field number == -2.
 *      field_num       field number to retrieve.  Set to
 *                        -1 to retrieve all of the fields
 *                        in the data.  Set to -2 to retrieve
 *                        the field based on the field name
 *                        instead of the field number.
 *      plane_ht_type   value indicating units used for all
 *                        plane heights in this argument list.
 *      plane_height    requested plane height in units defined
 *                        by plane_ht_type.  Set to -1 to retrieve
 *                        entire volume.
 *      crop_info       pointer to requested cropping information.
 *                        Set to NULL if cropping is not desired.
 *      composite_type  type of composite desired in the returned data.
 *                        Set to MDV_COMPOSITE_NONE if the data shouldn't
 *                        be composited.
 *      mdv_handle      pointer to the retrieved MDV data.
 *
 * Upon return:
 *      mdv_handle contains the pointers to the MDV data.
 *
 * MEMORY POLICY.
 * Do not free up the mdv_handle.  It is static to
 * the MDV library. If you wish to free up the memory
 * between calls, use MDV_free_get(). If you do not free
 * the memory, it will be realloc'd during the next call to
 * a get() routine.
 *
 * Returns 0 on success, -1 on failure
 */

int MDV_get_closest(char *source,
		    time_t request_time,
		    int time_margin,
		    int return_type,
		    char *field_name,
		    int field_num,
		    int plane_ht_type,
		    double plane_height,
		    MDV_request_crop_t *crop_info,
		    int composite_type,
		    MDV_handle_t **mdv_handle)       /* output */
{
  static char *routine_name = "MDV_get_closest()";
  
  int loc_type = DB_location_type(source);
  
  /*
   * Make sure the space is allocated for the get.
   */

  alloc_for_gets();
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    time_t data_time;
    char *filename;
    
    DSINP_handle_t ds_handle;
    
    DSINP_create_realtime(&ds_handle,
			  "mdv_client",
			  FALSE,
			  source,
			  -1,
			  NULL);
    
    if ((filename = DSINP_get_closest(&ds_handle,
				      request_time,
				      time_margin,
				      &data_time)) == (char *)NULL)
    {
      fprintf(stderr, "No closest data in directory <%s>\n",
	      source);
      
      return(-1);
    }
    
    DSINP_free(&ds_handle);
    
    /*
     * Read the data.
     */

    if (get_from_disk(filename, return_type, field_name,
		      field_num, plane_ht_type, plane_height,
		      crop_info, composite_type, &Mdv_handle) != 0)
      return(-1);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    MDV_request_closest_t request_info;
    
    /*
     * Get the host and port information.
     */

    if (loc_type == DB_LOCATION_SOCKET)
    {
      if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not parse <%s> for host and port\n",
		source);
	return(-1);
      }
    }
    else
    {
      if (DB_get_servmap_host_port(source,
				   host, MAX_HOST_LEN,
				   &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not get host and port from servmap for source <%s>\n",
		source);
	return(-1);
      }
    }
    
    /*
     * Put together the request information buffer.
     */

    STRcopy((char *) request_info.gen_info.field_name,
	    field_name, MDV_LONG_FIELD_LEN);
    request_info.gen_info.field_num = field_num;
    request_info.gen_info.return_type = return_type;
    request_info.gen_info.composite_type = composite_type;
    request_info.gen_info.plane_height_type = plane_ht_type;
    request_info.gen_info.plane_height = plane_height;

    if (crop_info == (MDV_request_crop_t *)NULL)
    {
      request_info.gen_info.crop_flag = FALSE;
    }
    else
    {
      request_info.gen_info.crop_flag = TRUE;
      request_info.gen_info.crop_info = *crop_info;
    }
    
    request_info.time = request_time;
    request_info.margin = time_margin;

    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			MDV_GET_CLOSEST,
			(void *)&request_info,
			sizeof(request_info),
			&Mdv_handle) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET or DB_LOCATION_SERVMAP */

  default:
    fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
    fprintf(stderr, "Invalid location type %d returned by DB_location_type()\n",
	    loc_type);
    
    return(-1);
    
  } /* endswitch - loc_type */
  
  /*
   * Set the return values.
   */

  *mdv_handle = &Mdv_handle;
  
  return(0);
}


/*****************************************************
 * MDV_get_first_before()
 *
 * Gets the MDV data from the given source stored at the
 * first time before the requested time within the requested
 * time margin.  The source can be either a socket or
 * disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater") or it
 * should contain the server mapper information for the
 * server in the form "type::subtype::instance" (e.g.
 * "MDV::MDV::kavm").
 *
 * Parameters:
 *      source          source string as described above.
 *      request_time    request time for data.
 *      time_margin     maximum time difference allowed
 *                        between requested time and
 *                        returned data time, in seconds.
 *      return_type     desired return type for data
 *                        (e.g. MDV_INT8).  Use MDV_NATIVE
 *                        to retrieve data in its stored
 *                        format.
 *      field_name      name of field to retrieve.  Only used
 *                        if field number == -2.
 *      field_num       field number to retrieve.  Set to
 *                        -1 to retrieve all of the fields
 *                        in the data.  Set to -2 to retrieve
 *                        the field based on the field name
 *                        instead of the field number.
 *      plane_ht_type   value indicating units used for all
 *                        plane heights in this argument list.
 *      plane_height    requested plane height in km.  Set
 *                        to -1 to retrieve entire volume.
 *      crop_info       pointer to requested cropping information.
 *                        Set to NULL if cropping is not desired.
 *      composite_type  type of composite desired in the returned data.
 *                        Set to MDV_COMPOSITE_NONE if the data shouldn't
 *                        be composited.
 *      mdv_handle      pointer to the retrieved MDV data.
 *
 * Upon return:
 *      mdv_handle contains the pointers to the MDV data.
 *
 * MEMORY POLICY.
 * Do not free up the mdv_handle.  It is static to
 * the MDV library. If you wish to free up the memory
 * between calls, use MDV_free_get(). If you do not free
 * the memory, it will be realloc'd during the next call to
 * a get() routine.
 *
 * Returns 0 on success, -1 on failure
 */

int MDV_get_first_before(char *source,
			 time_t request_time,
			 int time_margin,
			 int return_type,
			 char *field_name,
			 int field_num,
			 int plane_ht_type,
			 double plane_height,
			 MDV_request_crop_t *crop_info,
			 int composite_type,
			 MDV_handle_t **mdv_handle)       /* output */
{
  static char *routine_name = "MDV_get_first_before()";
  
  int loc_type = DB_location_type(source);
  
  /*
   * Make sure the space is allocated for the get.
   */

  alloc_for_gets();
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    time_t data_time;
    char *filename;
    
    DSINP_handle_t ds_handle;
    
    DSINP_create_realtime(&ds_handle,
			  "mdv_client",
			  FALSE,
			  source,
			  -1,
			  NULL);
    
    if ((filename = DSINP_get_first_before(&ds_handle,
					   request_time,
					   time_margin,
					   &data_time)) == (char *)NULL)
    {
      fprintf(stderr, "No first before data in directory <%s>\n",
	      source);
      
      return(-1);
    }
    
    DSINP_free(&ds_handle);
    
    /*
     * Read the data.
     */

    if (get_from_disk(filename, return_type, field_name,
		      field_num, plane_ht_type, plane_height,
		      crop_info, composite_type, &Mdv_handle) != 0)
      return(-1);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    MDV_request_closest_t request_info;
    
    /*
     * Get the host and port information.
     */

    if (loc_type == DB_LOCATION_SOCKET)
    {
      if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not parse <%s> for host and port\n",
		source);
	return(-1);
      }
    }
    else
    {
      if (DB_get_servmap_host_port(source,
				   host, MAX_HOST_LEN,
				   &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not get host and port from servmap for source <%s>\n",
		source);
	return(-1);
      }
    }
    
    /*
     * Put together the request information buffer.
     */

    STRcopy((char *) request_info.gen_info.field_name,
	    field_name, MDV_LONG_FIELD_LEN);
    request_info.gen_info.field_num = field_num;
    request_info.gen_info.return_type = return_type;
    request_info.gen_info.composite_type = composite_type;
    request_info.gen_info.plane_height_type = plane_ht_type;
    request_info.gen_info.plane_height = plane_height;
    
    if (crop_info == (MDV_request_crop_t *)NULL)
    {
      request_info.gen_info.crop_flag = FALSE;
    }
    else
    {
      request_info.gen_info.crop_flag = TRUE;
      request_info.gen_info.crop_info = *crop_info;
    }
    
    request_info.time = request_time;
    request_info.margin = time_margin;

    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			MDV_GET_FIRST_BEFORE,
			(void *)&request_info,
			sizeof(request_info),
			&Mdv_handle) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET or DB_LOCATION_SERVMAP */

  default:
    fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
    fprintf(stderr, "Invalid location type %d returned by DB_location_type()\n",
	    loc_type);
    
    return(-1);
    
  } /* endswitch - loc_type */
  
  /*
   * Set the return values.
   */

  *mdv_handle = &Mdv_handle;
  
  return(0);
}


/*****************************************************
 * MDV_get_first_after()
 *
 * Gets the MDV data from the given source stored at the
 * first time after the requested time within the requested
 * time margin.  The source can be either a socket or
 * disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater") or it
 * should contain the server mapper information for the
 * server in the form "type::subtype::instance" (e.g.
 * "MDV::MDV::kavm").
 *
 * Parameters:
 *      source          source string as described above.
 *      request_time    request time for data.
 *      time_margin     maximum time difference allowed
 *                        between requested time and
 *                        returned data time, in seconds.
 *      return_type     desired return type for data
 *                        (e.g. MDV_INT8).  Use MDV_NATIVE
 *                        to retrieve data in its stored
 *                        format.
 *      field_name      name of field to retrieve.  Only used
 *                        if field number == -2.
 *      field_num       field number to retrieve.  Set to
 *                        -1 to retrieve all of the fields
 *                        in the data.  Set to -2 to retrieve
 *                        the field based on the field name
 *                        instead of the field number.
 *      plane_ht_type   value indicating units used for all
 *                        plane heights in this argument list.
 *      plane_height    requested plane height in km.  Set
 *                        to -1 to retrieve entire volume.
 *      crop_info       pointer to requested cropping information.
 *                        Set to NULL if cropping is not desired.
 *      composite_type  type of composite desired in the returned data.
 *                        Set to MDV_COMPOSITE_NONE if the data shouldn't
 *                        be composited.
 *      mdv_handle      pointer to the retrieved MDV data.
 *
 * Upon return:
 *      mdv_handle contains the pointers to the MDV data.
 *
 * MEMORY POLICY.
 * Do not free up the mdv_handle.  It is static to
 * the MDV library. If you wish to free up the memory
 * between calls, use MDV_free_get(). If you do not free
 * the memory, it will be realloc'd during the next call to
 * a get() routine.
 *
 * Returns 0 on success, -1 on failure
 */

int MDV_get_first_after(char *source,
			time_t request_time,
			int time_margin,
			int return_type,
			char *field_name,
			int field_num,
			int plane_ht_type,
			double plane_height,
			MDV_request_crop_t *crop_info,
			int composite_type,
			MDV_handle_t **mdv_handle)       /* output */
{
  static char *routine_name = "MDV_get_first_after()";
  
  int loc_type = DB_location_type(source);
  
  /*
   * Make sure the space is allocated for the get.
   */

  alloc_for_gets();
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    time_t data_time;
    char *filename;
    
    DSINP_handle_t ds_handle;
    
    DSINP_create_realtime(&ds_handle,
			  "mdv_client",
			  FALSE,
			  source,
			  -1,
			  NULL);
    
    if ((filename = DSINP_get_first_after(&ds_handle,
					  request_time,
					  time_margin,
					  &data_time)) == (char *)NULL)
    {
      fprintf(stderr, "No first after data in directory <%s>\n",
	      source);
      
      return(-1);
    }
    
    DSINP_free(&ds_handle);
    
    /*
     * Read the data.
     */

    if (get_from_disk(filename, return_type, field_name,
		      field_num, plane_ht_type, plane_height,
		      crop_info, composite_type, &Mdv_handle) != 0)
      return(-1);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    MDV_request_closest_t request_info;
    
    /*
     * Get the host and port information.
     */

    if (loc_type == DB_LOCATION_SOCKET)
    {
      if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not parse <%s> for host and port\n",
		source);
	return(-1);
      }
    }
    else
    {
      if (DB_get_servmap_host_port(source,
				   host, MAX_HOST_LEN,
				   &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not get host and port from servmap for source <%s>\n",
		source);
	return(-1);
      }
    }
    
    /*
     * Put together the request information buffer.
     */

    STRcopy((char *) request_info.gen_info.field_name,
	    field_name, MDV_LONG_FIELD_LEN);
    request_info.gen_info.field_num = field_num;
    request_info.gen_info.return_type = return_type;
    request_info.gen_info.composite_type = composite_type;
    request_info.gen_info.plane_height_type = plane_ht_type;
    request_info.gen_info.plane_height = plane_height;
    
    if (crop_info == (MDV_request_crop_t *)NULL)
    {
      request_info.gen_info.crop_flag = FALSE;
    }
    else
    {
      request_info.gen_info.crop_flag = TRUE;
      request_info.gen_info.crop_info = *crop_info;
    }
    
    request_info.time = request_time;
    request_info.margin = time_margin; 

   /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			MDV_GET_FIRST_AFTER,
			(void *)&request_info,
			sizeof(request_info),
			&Mdv_handle) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET or DB_LOCATION_SERVMAP */

  default:
    fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
    fprintf(stderr, "Invalid location type %d returned by DB_location_type()\n",
	    loc_type);
    
    return(-1);
    
  } /* endswitch - loc_type */
  
  /*
   * Set the return values.
   */

  *mdv_handle = &Mdv_handle;
  
  return(0);
}


/*****************************************************
 * MDV_get_latest()
 *
 * Gets the latest MDV data from the given source.  The
 * source can be either a socket or disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater") or it
 * should contain the server mapper information for the
 * server in the form "type::subtype::instance" (e.g.
 * "MDV::MDV::kavm").
 *
 * Parameters:
 *      source          source string as described above.
 *      return_type     desired return type for data
 *                        (e.g. MDV_INT8).  Use MDV_NATIVE
 *                        to retrieve data in its stored
 *                        format.
 *      field_name      name of field to retrieve.  Only used
 *                        if field number == -2.
 *      field_num       field number to retrieve.  Set to
 *                        -1 to retrieve all of the fields
 *                        in the data.  Set to -2 to retrieve
 *                        the field based on the field name
 *                        instead of the field number.
 *      plane_ht_type   value indicating units used for all
 *                        plane heights in this argument list.
 *      plane_height    requested plane height in km.  Set
 *                        to -1 to retrieve entire volume.
 *      crop_info       pointer to requested cropping information.
 *                        Set to NULL if cropping is not desired.
 *      composite_type  type of composite desired in the returned data.
 *                        Set to MDV_COMPOSITE_NONE if the data shouldn't
 *                        be composited.
 *      mdv_handle      pointer to the retrieved MDV data.
 *
 * Upon return:
 *      mdv_handle contains the pointers to the MDV data.
 *
 * MEMORY POLICY.
 * Do not free up the mdv_handle.  It is static to
 * the MDV library. If you wish to free up the memory
 * between calls, use MDV_free_get(). If you do not free
 * the memory, it will be realloc'd during the next call to
 * a get() routine.
 *
 * Returns 0 on success, -1 on failure
 */

int MDV_get_latest(char *source,
		   int return_type,
		   char *field_name,
		   int field_num,
		   int plane_ht_type,
		   double plane_height,
		   MDV_request_crop_t *crop_info,
		   int composite_type,
		   MDV_handle_t **mdv_handle)       /* output */
{
  static char *routine_name = "MDV_get_latest()";
  
  int loc_type = DB_location_type(source);
  
  /*
   * Make sure the space is allocated for the get.
   */

  alloc_for_gets();
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    char *filename;
    
    DSINP_handle_t ds_handle;
    
    DSINP_create_realtime(&ds_handle,
			  "mdv_client",
			  FALSE,
			  source,
			  -1,
			  NULL);
    
    if ((filename = DSINP_latest(&ds_handle)) == (char *)NULL)
    {
      fprintf(stderr, "No latest data in directory <%s>\n",
	      source);
      
      return(-1);
    }
    
    DSINP_free(&ds_handle);
    
    /*
     * Read the data.
     */

    if (get_from_disk(filename, return_type, field_name,
		      field_num, plane_ht_type, plane_height,
		      crop_info, composite_type, &Mdv_handle) != 0)
      return(-1);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    MDV_request_latest_t request_info;
    
    /*
     * Get the host and port information.
     */

    if (loc_type == DB_LOCATION_SOCKET)
    {
      if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not parse <%s> for host and port\n",
		source);
	return(-1);
      }
    }
    else
    {
      if (DB_get_servmap_host_port(source,
				   host, MAX_HOST_LEN,
				   &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not get host and port from servmap for source <%s>\n",
		source);
	return(-1);
      }
    }
    
    /*
     * Put together the request information buffer.
     */

    STRcopy((char *) request_info.gen_info.field_name,
	    field_name, MDV_LONG_FIELD_LEN);
    request_info.gen_info.field_num = field_num;
    request_info.gen_info.return_type = return_type;
    request_info.gen_info.composite_type = composite_type;
    request_info.gen_info.plane_height_type = plane_ht_type;
    request_info.gen_info.plane_height = plane_height;
    
    if (crop_info == (MDV_request_crop_t *)NULL)
    {
      request_info.gen_info.crop_flag = FALSE;
    }
    else
    {
      request_info.gen_info.crop_flag = TRUE;
      request_info.gen_info.crop_info = *crop_info;
    }
    
    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			MDV_GET_LATEST,
			(void *)&request_info,
			sizeof(request_info),
			&Mdv_handle) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET or DB_LOCATION_SERVMAP */

  default:
    fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
    fprintf(stderr, "Invalid location type %d returned by DB_location_type()\n",
	    loc_type);
    
    return(-1);
    
  } /* endswitch - loc_type */
  
  /*
   * Set the return values.
   */

  *mdv_handle = &Mdv_handle;
  
  return(0);
}


/*****************************************************
 * MDV_get_new()
 *
 * Gets the latest new (after the given last data time)
 * MDV data from the given source.  If no new data has
 * appeared since the given data time, -1 is returned.
 * The source can be either a socket or disk location.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater") or it
 * should contain the server mapper information for the
 * server in the form "type::subtype::instance" (e.g.
 * "MDV::MDV::kavm").
 *
 * Parameters:
 *      source          source string as described above.
 *      last_data_time  last time data was received from
 *                        this source.
 *      return_type     desired return type for data
 *                        (e.g. MDV_INT8).  Use MDV_NATIVE
 *                        to retrieve data in its stored
 *                        format.
 *      field_name      name of field to retrieve.  Only used
 *                        if field number == -2.
 *      field_num       field number to retrieve.  Set to
 *                        -1 to retrieve all of the fields
 *                        in the data.  Set to -2 to retrieve
 *                        the field based on the field name
 *                        instead of the field number.
 *      plane_ht_type   value indicating units used for all
 *                        plane heights in this argument list.
 *      plane_height    requested plane height in km.  Set
 *                        to -1 to retrieve entire volume.
 *      crop_info       pointer to requested cropping information.
 *                        Set to NULL if cropping is not desired.
 *      composite_type  type of composite desired in the returned data.
 *                        Set to MDV_COMPOSITE_NONE if the data shouldn't
 *                        be composited.
 *      mdv_handle      pointer to the retrieved MDV data.
 *
 * Upon return:
 *      mdv_handle contains the pointers to the MDV data.
 *
 * MEMORY POLICY.
 * Do not free up the mdv_handle.  It is static to
 * the MDV library. If you wish to free up the memory
 * between calls, use MDV_free_get(). If you do not free
 * the memory, it will be realloc'd during the next call to
 * a get() routine.
 *
 * Returns 0 on success, -1 on failure
 */

int MDV_get_new(char *source,
		time_t last_data_time,
		int return_type,
		char *field_name,
		int field_num,
		int plane_ht_type,
		double plane_height,
		MDV_request_crop_t *crop_info,
		int composite_type,
		MDV_handle_t **mdv_handle)       /* output */
{
  static char *routine_name = "MDV_get_new()";
  
  int loc_type = DB_location_type(source);
  
  /*
   * Make sure the space is allocated for the get.
   */

  alloc_for_gets();
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    char *filename;
    
    DSINP_handle_t ds_handle;
    
    DSINP_create_realtime(&ds_handle,
			  "mdv_client",
			  FALSE,
			  source,
			  -1,
			  NULL);
    
    if ((filename = DSINP_new_data(&ds_handle,
				   last_data_time)) == (char *)NULL)
    {
      fprintf(stderr, "No new data in directory <%s>\n",
	      source);
      
      return(-1);
    }
    
    DSINP_free(&ds_handle);
    
    /*
     * Read the data.
     */

    if (get_from_disk(filename, return_type, field_name,
		      field_num, plane_ht_type, plane_height,
		      crop_info, composite_type, &Mdv_handle) != 0)
      return(-1);
    
    break;
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    MDV_request_new_t request_info;
    
    /*
     * Get the host and port information.
     */

    if (loc_type == DB_LOCATION_SOCKET)
    {
      if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not parse <%s> for host and port\n",
		source);
	return(-1);
      }
    }
    else
    {
      if (DB_get_servmap_host_port(source,
				   host, MAX_HOST_LEN,
				   &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not get host and port from servmap for source <%s>\n",
		source);
	return(-1);
      }
    }
    
    /*
     * Put together the request information buffer.
     */

    STRcopy((char *) request_info.gen_info.field_name,
	    field_name, MDV_LONG_FIELD_LEN);
    request_info.gen_info.field_num = field_num;
    request_info.gen_info.return_type = return_type;
    request_info.gen_info.composite_type = composite_type;
    request_info.gen_info.plane_height_type = plane_ht_type;
    request_info.gen_info.plane_height = plane_height;
    
    if (crop_info == (MDV_request_crop_t *)NULL)
    {
      request_info.gen_info.crop_flag = FALSE;
    }
    else
    {
      request_info.gen_info.crop_flag = TRUE;
      request_info.gen_info.crop_info = *crop_info;
    }
    
    request_info.last_data_time = last_data_time;

    /*
     * Submit the request over the appropriate socket.
     */

    if (get_from_socket(host,
			port,
			MDV_GET_NEW,
			(void *)&request_info,
			sizeof(request_info),
			&Mdv_handle) != 0)
    {
      return(-1);
    }

    break;
  } /* endcase - DB_LOCATION_SOCKET or DB_LOCATION_SERVMAP */

  default:
    fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
    fprintf(stderr, "Invalid location type %d returned by DB_location_type()\n",
	    loc_type);
    
    return(-1);
    
  } /* endswitch - loc_type */
  
  /*
   * Set the return values.
   */

  *mdv_handle = &Mdv_handle;
  
  return(0);
}


/********************************************
 * MDV_free_get()
 *
 * Frees up memory used by MDV_get() calls.
 *
 * Memory allocated by MDV_get() calls is kept statically
 * within the MDV module.
 *
 * You may free it at any time using MDV_free_get().
 *
 * However, it is not necessary to do this between calls unless you
 * are concerned about total memory usage.
 * Generally this call is only used when no further MDV_get() calls
 * will be made.
 */

void MDV_free_get(void)
{
  if (Mdv_handle_init_flag)
  {
    MDV_free_handle(&Mdv_handle);
    
    Mdv_handle_init_flag = FALSE;
  }
}


/*****************************************************
 * MDV_handle_sigpipe()
 *
 * Closes any open sockets that were used by the MDV
 * routines.  Should be called by the application's
 * interrupt handler when a SIGPIPE interrupt is
 * received.
 */

void MDV_handle_sigpipe(void)
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
 * MDV_request_size()
 *
 * Returns the number of bytes in a request of the given
 * type.  Returns -1 if the request type isn't recognized.
 */

int MDV_request_size(int request)
{
  switch (request)
  {
  case MDV_GET_CLOSEST :
  case MDV_GET_FIRST_BEFORE :
  case MDV_GET_FIRST_AFTER :
    return(sizeof(MDV_request_closest_t));
    
  case MDV_GET_LATEST :
    return(sizeof(MDV_request_latest_t));
    
  case MDV_GET_NEW :
    return(sizeof(MDV_request_new_t));
  }
  
  return(-1);
}


/*****************************************************
 * MDV_request_to_BE()
 *
 * Converts the given request structure from native format to
 * big-endian format.
 *
 * Parameters:
 *       request            request id (e.g. MDV_GET_CLOSEST).
 *       request_ptr        pointer to request structure, in native
 *                            format.
 *       request_len        lenth of request structure in bytes.
 */

void MDV_request_to_BE(int request, void *request_ptr, int request_len)
{
  char *si32_ptr = (char *)request_ptr + MDV_LONG_FIELD_LEN;
  
  BE_from_array_32(si32_ptr, request_len - MDV_LONG_FIELD_LEN);
  
  return;
}


/*****************************************************
 * MDV_request_from_BE()
 *
 * Converts the given request structure from big-endian format to
 * native format.
 *
 * Parameters:
 *       request            request id (e.g. MDV_GET_CLOSEST).
 *       request_ptr        pointer to request structure, in
 *                            big-endian format.
 *       request_len        lenth of request structure in bytes.
 */

void MDV_request_from_BE(int request, void *request_ptr, int request_len)
{
  char *si32_ptr = (char *)request_ptr + MDV_LONG_FIELD_LEN;
  
  BE_to_array_32(si32_ptr, request_len - MDV_LONG_FIELD_LEN);
  
  return;
}


/*****************************************************
 * MDV_get_dataset_times()
 *
 * Gets the data times for the datasets available from
 * the specified source.  The source can be either a socket
 * or disk location.  The returned list of times will only
 * include datasets generated between the given times.
 * If begin_gen_time is set to -1, retrieves all dataset
 * times from the beginning of the data.  If end_gen_time
 * is set to -1, retrieves all dataset times up to the
 * latest data time.
 *
 * For a disk source, the source string should contain
 * the directory path for the database.
 *
 * For a socket source, the source string should contain
 * the host and port information for the socket in
 * the form "port@host" (e.g. "62000@anteater") or it
 * should contain the server mapper information for the
 * server in the form "type::subtype::instance" (e.g.
 * "MDV::MDV::kavm").
 *
 * Parameters:
 *      source          source string as described above.
 *      begin_gen_time  beginning generation time of desired
 *                        datasets.
 *      end_gen_time    ending generation time of desired
 *                        datasets.
 *      num_datasets    the number of datasets in the returned
 *                        array.
 * Upon return:
 *      num_datasets contains the number of datasets in the
 *        returned array.
 *
 * MEMORY POLICY.
 * Returns a pointer to a dynamically allocated array.  This array
 * must be freed (using ufree()) by the calling routine.
 *
 * Returns a pointer to a list of num_datasets MDV_data_time_t
 * structures. Returns NULL on failure.
 */

MDV_dataset_time_t *MDV_get_dataset_times(char *source,
					  time_t begin_gen_time,
					  time_t end_gen_time,
					  int *num_datasets)       /* output */
{
  static char *routine_name = "MDV_get_dataset_times()";
  
  MDV_dataset_time_t *data_times_out;
  
  int loc_type = DB_location_type(source);
  
  /*
   * Initialize return values.
   */

  data_times_out = (MDV_dataset_time_t *)NULL;
  *num_datasets = 0;
  
  /*
   * See if we're getting from a socket or from disk.
   */

  switch (loc_type)
  {
  case DB_LOCATION_DISK :
  {
    int i;
    
    DSINP_dataset_time_t *data_times =
      DSINP_get_dataset_times(source, begin_gen_time, end_gen_time, num_datasets);
    
    data_times_out =
      (MDV_dataset_time_t *)umalloc(*num_datasets * sizeof(MDV_dataset_time_t));
    
    for (i = 0; i < *num_datasets; i++)
    {
      data_times_out[i].gen_time = data_times[i].gen_time;
      data_times_out[i].forecast_time = data_times[i].forecast_time;
    }
    
    return(data_times_out);
    
  } /* endcase - DB_LOCATION_DISK */

  case DB_LOCATION_SOCKET :
  case DB_LOCATION_SERVMAP :
  {
    char host[MAX_HOST_LEN];
    int port;
    MDV_dataset_time_request_t request_info;
    
    /*
     * Get the host and port information.
     */

    if (loc_type == DB_LOCATION_SOCKET)
    {
      if (DB_get_host_port(source, host, MAX_HOST_LEN, &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not parse <%s> for host and port\n",
		source);
	return((MDV_dataset_time_t *)NULL);
      }
    }
    else
    {
      if (DB_get_servmap_host_port(source,
				   host, MAX_HOST_LEN,
				   &port) != 0)
      {
	fprintf(stderr, "ERROR - mdv_client:%s\n",
		routine_name);
	fprintf(stderr, "Could not get host and port from servmap for source <%s>\n",
		source);
	return((MDV_dataset_time_t *)NULL);
      }
    }
    
    /*
     * Put together the request information buffer.
     */

    request_info.begin_gen_time = begin_gen_time;
    request_info.end_gen_time = end_gen_time;
    
    /*
     * Submit the request over the appropriate socket.
     */

    data_times_out =
      get_dataset_times_from_socket(host,
				    port,
				    &request_info,
				    num_datasets);
    break;
  } /* endcase - DB_LOCATION_SOCKET or DB_LOCATION_SERVMAP */

  default:
    fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
    fprintf(stderr, "Invalid location type %d returned by DB_location_type()\n",
	    loc_type);
    
    return((MDV_dataset_time_t *)NULL);
    
  } /* endswitch - loc_type */
  
  return(data_times_out);
}


/*****************************************************
 * MDV_dataset_time_request_to_BE()
 *
 * Converts the given dataset time request structure from native format to
 * big-endian format.
 *
 * Parameters:
 *       request_ptr        pointer to request structure, in
 *                            native format.
 */

void MDV_dataset_time_request_to_BE(MDV_dataset_time_request_t *request_ptr)
{
  request_ptr->begin_gen_time =
    BE_from_si32(request_ptr->begin_gen_time);
  request_ptr->end_gen_time =
    BE_from_si32(request_ptr->end_gen_time);
  
  return;
}


/*****************************************************
 * MDV_dataset_time_request_from_BE()
 *
 * Converts the given dataset time request structure from big-endian format to
 * native format.
 *
 * Parameters:
 *       request_ptr        pointer to request structure, in
 *                            big-endian format.
 */

void MDV_dataset_time_request_from_BE(MDV_dataset_time_request_t *request_ptr)
{
  request_ptr->begin_gen_time =
    BE_to_si32(request_ptr->begin_gen_time);
  request_ptr->end_gen_time =
    BE_to_si32(request_ptr->end_gen_time);
  
  return;
}


/*****************************************************
 * MDV_dataset_time_to_BE()
 *
 * Converts the given dataset time information structure from native format to
 * big-endian format.
 *
 * Parameters:
 *       info_ptr        pointer to information structure, in native format.
 */

void MDV_dataset_time_to_BE(MDV_dataset_time_t *info_ptr)
{
  info_ptr->gen_time = BE_from_si32(info_ptr->gen_time);
  info_ptr->forecast_time = BE_from_si32(info_ptr->forecast_time);
  
  return;
}


/*****************************************************
 * MDV_dataset_time_from_BE()
 *
 * Converts the given dataset time information structure from big-endian format to
 * native format.
 *
 * Parameters:
 *       info_ptr     pointer to information structure, in big-endian format.
 */

void MDV_dataset_time_from_BE(MDV_dataset_time_t *info_ptr)
{
  info_ptr->gen_time = BE_to_si32(info_ptr->gen_time);
  info_ptr->forecast_time = BE_to_si32(info_ptr->forecast_time);
  
  return;
}


/*****************************************************
 * Routiness for converting values to strings for printing
 *****************************************************/

/*****************************************************
 * MDV_request2string()
 *
 * Converts a request value to the matching string.
 *
 * Returns a pointer to a static area containing the string.
 * Do NOT free this pointer.
 */

char *MDV_request2string(int request)
{
  switch(request)
  {
  case MDV_GET_CLOSEST :
    return("MDV_GET_CLOSEST");
    break;
    
  case MDV_GET_FIRST_BEFORE :
    return("MDV_GET_FIRST_BEFORE");
    break;
    
  case MDV_GET_FIRST_AFTER :
    return("MDV_GET_FIRST_AFTER");
    break;
    
  case MDV_GET_LATEST :
    return("MDV_GET_LATEST");
    break;
    
  case MDV_GET_NEW :
    return("MDV_GET_NEW");
    break;
    
  case MDV_PUT_DATA :
    return("MDV_PUT_DATA");
    break;
    
  default:
    return("Unknown MDV request");
    break;
  }

  return("Unknown MDV request");
}

/*****************************************************
 * MDV_reply2string()
 *
 * Converts a reply value to the matching string.
 *
 * Returns a pointer to a static area containing the string.
 * Do NOT free this pointer.
 */

char *MDV_reply2string(int reply)
{
  switch(reply)
  {
  case MDV_DATA :
    return("MDV_DATA");
    break;
    
  case MDV_NO_DATA :
    return("MDV_NO_DATA");
    break;
    
  case MDV_DATA_ERROR :
    return("MDV_DATA_ERROR");
    break;
    
  case MDV_PUT_SUCCESSFUL :
    return("MDV_PUT_SUCCESSFUL");
    break;
    
  case MDV_PUT_FAILED :
    return("MDV_PUT_FAILED");
    break;
    
  case MDV_REQUEST_ERROR :
    return("MDV_REQUEST_ERROR");
    break;
    
  default:
    return("Unknown MDV reply");
    break;
  }

  return("Unknown MDV reply");
}

/*****************************************************
 * MDV_reap_children()
 *
 * Check for any child processes that are finished and
 * clean them up.  This keeps us from getting overwhelmed
 * by "zombie" processes.
 */

void MDV_reap_children(void)
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
			       int request_type,
			       MDV_handle_t *mdv_handle)
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

  child_log_dir = getenv("MDV_CLIENT_CHILD_LOG_DIR");
  
  /*
   * Make sure the number of child processes is below the
   * maximum.
   */

  while (NumChildren > MDV_CLIENT_MAX_CHILDREN)
  {
    MDV_reap_children();
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

    MDV_reap_children();
    
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
      sprintf(child_log_filename, "%s/mdv_client_child.%d",
	      child_log_dir, child_pid);
      
      /*
       * Open the log file.
       */

      if ((child_log_file = fopen(child_log_filename, "w")) == NULL)
      {
	fprintf(stderr,
		"ERROR: mdv_client::%s\n",
		routine_name);
	fprintf(stderr,
		"Error opening child log file\n");
	perror(child_log_filename);
	
	child_log_dir = (char *)NULL;
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
		  request_type,
		  mdv_handle,
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

    exit(0);
  }
  
}

/******************
 * alloc_for_gets()
 *
 * Allocs space for buffers used in the get() calls.
 */

static void alloc_for_gets(void)
{
  if (!Mdv_handle_init_flag)
  {
    if (MDV_init_handle(&Mdv_handle) == 0)
      Mdv_handle_init_flag = TRUE;
  }
  
}

/*****************************************************
 * put_to_socket()
 *
 * Puts MDV data to the given destination socket.
 *
 * All data except the chunk data will be byte-swapped,
 * as appropriate, by this routine.  The chunk data must
 * be put into big-endian format by the caller before
 * calling this routine.
 *
 * Returns 0 on success, -1 on failure
 */

static int put_to_socket(char *host,
			 int port,
			 si32 request_type,
			 MDV_handle_t *mdv_handle,
			 FILE *log_file)
{
  static char *routine_name = "put_to_socket()";
  
  char *msg_buffer;
  char *buf_ptr;
  int buffer_len;
  long buffer_len_long;
  si32 reply;
  SKU_header_t sku_header;
  
  int num_fields;
  int num_chunks;
  int num_vlevel;
  
  int field;
  int plane;
  int i;
  
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
    
    fprintf(stderr, "ERROR - mdv_client:%s\n",
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

  num_fields = mdv_handle->master_hdr.n_fields;
  num_chunks = mdv_handle->master_hdr.n_chunks;
  
  if (mdv_handle->master_hdr.vlevel_included)
    num_vlevel = num_fields;
  else
    num_vlevel = 0;
  
  buffer_len = sizeof(si32) + sizeof(MDV_master_header_t) +
    (num_fields * sizeof(MDV_field_header_t)) +
    (num_vlevel * sizeof(MDV_vlevel_header_t)) +
    (num_chunks * sizeof(MDV_chunk_header_t));
  
  for (i = 0; i < num_fields; i++)
    buffer_len += mdv_handle->fld_hdrs[i].volume_size;
  
  for (i = 0; i < num_chunks; i++)
    buffer_len += mdv_handle->chunk_hdrs[i].size;
  
  msg_buffer = (char *)umalloc(buffer_len);
  
  buf_ptr = msg_buffer;
  
  /* request_id */
  memcpy(buf_ptr, &request_type, sizeof(si32));
  BE_from_array_32(buf_ptr, sizeof(si32));
  buf_ptr += sizeof(si32);

  /* master header */
  memcpy(buf_ptr, &mdv_handle->master_hdr, sizeof(MDV_master_header_t));
  MDV_master_header_to_BE((MDV_master_header_t *)buf_ptr);
  buf_ptr += sizeof(MDV_master_header_t);
  
  /* field headers */
  for (i = 0; i < num_fields; i++)
  {
    memcpy(buf_ptr, &mdv_handle->fld_hdrs[i], sizeof(MDV_field_header_t));
    MDV_field_header_to_BE((MDV_field_header_t *)buf_ptr);
    buf_ptr += sizeof(MDV_field_header_t);
  }
  
  /* vlevel headers */
  for (i = 0; i < num_vlevel; i++)
  {
    memcpy(buf_ptr, &mdv_handle->vlv_hdrs[i], sizeof(MDV_vlevel_header_t));
    MDV_vlevel_header_to_BE((MDV_vlevel_header_t *)buf_ptr);
    buf_ptr += sizeof(MDV_vlevel_header_t);
  }
  
  /* chunk headers */
  for (i = 0; i < num_chunks; i++)
  {
    memcpy(buf_ptr, &mdv_handle->chunk_hdrs[i], sizeof(MDV_chunk_header_t));
    MDV_chunk_header_to_BE((MDV_chunk_header_t *)buf_ptr);
    buf_ptr += sizeof(MDV_chunk_header_t);
  }
  
  /* field data */
  for (field = 0; field < num_fields; field++)
  {
    MDV_field_header_t *field_hdr = &mdv_handle->fld_hdrs[field];
    
    for (plane = 0; plane < field_hdr->nz; plane++)
    {
      void *data_ptr = mdv_handle->field_plane[field][plane];
      int plane_size = MDV_calc_plane_size(field_hdr, plane, data_ptr);
      
      memcpy(buf_ptr, data_ptr, plane_size);
      MDV_plane_to_BE(field_hdr, (void *)buf_ptr);
      buf_ptr += plane_size;
    }
  }
  
  /* chunk data -- don't swap */
  for (i = 0; i < num_chunks; i++)
  {
    int chunk_size = mdv_handle->chunk_hdrs[i].size;
    
    memcpy(buf_ptr, mdv_handle->chunk_data[i], chunk_size);
    buf_ptr += chunk_size;
  }
  
  if (log_file != NULL)
  {
    fprintf(log_file,
	    "Sending output buffer\n");
    fflush(log_file);
  }
  
  /*
   * Send the buffer to the server.
   */

  if (SKU_write_message(SockFd, MDV_DATA,
			msg_buffer, buffer_len) != 1)
  {
    fprintf(stderr, "ERROR: mdv_client::%s\n",
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
		       MDV_CLIENT_REPLY_TIMEOUT) != 1)
  {
    fprintf(stderr, "ERROR - mdv_client:%s\n",
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
    fprintf(stderr, "ERROR: mdv_client::%s\n",
	    routine_name);
    fprintf(stderr,
	    "Response from server should be %ld bytes, received %ld bytes\n",
	    (long int)sizeof(si32), (long int)buffer_len_long);
    
    return(-1);
  }
  
  reply = *(si32 *)msg_buffer;
  reply = BE_from_si32(reply);

  if (reply != MDV_PUT_SUCCESSFUL)
  {
    fprintf(stderr, "ERROR: mdv_client::%s\n",
	    routine_name);
    fprintf(stderr, "Server returned error code %s for put operation to %d at %s\n",
	    MDV_reply2string(reply), port, host);
    
    return(-1);
  }
  
  return(0);
}

/*****************************************************
 * get_from_disk()
 *
 * Gets MDV data from the given source directory.
 *
 * Upon return:
 *      mdv_handle contains the MDV data.
 *
 * Returns 0 on success, -1 on failure
 */

static int get_from_disk(char *filename,
			 int return_type,
			 char *field_name,
			 int field_num,
			 int plane_ht_type,
			 double plane_height,
			 MDV_request_crop_t *crop_info,
			 int composite_type,
			 MDV_handle_t *mdv_handle)       /* output */
{
  static char *routine_name = "get_from_disk";
  
  int local_field_num = field_num;
  
  if (MDV_read_all(mdv_handle, filename, return_type) != 0)
  {
    fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
    fprintf(stderr, "Error reading data from file <%s>\n",
	    filename);
      
    return(-1);
  }
    
  /*
   * Remove all but the requested fields.
   */

  if (local_field_num == -2)
  {
    local_field_num = MDV_field_name_to_pos(mdv_handle,
					    field_name);
    
    if (local_field_num < 0)
    {
      fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
      fprintf(stderr, "No field with name <%s> found in file <%s>\n",
	      field_name, filename);
      
      return(-1);
    }
    
  }
  
  if (local_field_num >= 0)
  {
    int fields_before;
    int fields_after;
    int i;
      
    if (local_field_num >= Mdv_handle.master_hdr.n_fields)
    {
      fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
      fprintf(stderr, "Invalid field number %d requested.\n",
	      local_field_num);
      fprintf(stderr, "File only has %d fields.\n",
	      mdv_handle->master_hdr.n_fields);
	
      return(-1);
    }
      
    fields_before = local_field_num;
    fields_after = mdv_handle->master_hdr.n_fields - local_field_num - 1;
      
    for (i = 0; i < fields_before; i++)
      MDV_remove_field(mdv_handle, 0);
      
    for (i = 0; i < fields_after; i++)
      MDV_remove_field(mdv_handle, 1);
      
  }
   
  /*
   * Remove all but the requested planes.
   */

  if (plane_height >= 0)
  {
    int ifield;
    int i;
      
    /*
     * See if we can handle this request.
     */

    if (plane_ht_type != mdv_handle->master_hdr.vlevel_type)
    {
      fprintf(stderr, "ERROR: mdv_client::%s\n", routine_name);
      fprintf(stderr, "Request has plane heights in %s format units.\n",
	      MDV_verttype2string(plane_ht_type));
      fprintf(stderr, "File has plane heights in %s format units.\n",
	      MDV_verttype2string(mdv_handle->master_hdr.vlevel_type));
      fprintf(stderr, "Converting between plane height types not yet implemented.\n");
      
      return(-1);
    }
    
    /*
     * Remove the undesired planes all of the fields.
     */

    for (ifield = 0; ifield < mdv_handle->master_hdr.n_fields; ifield++)
    {
      MDV_field_header_t *field_hdr = &mdv_handle->fld_hdrs[ifield];
	
      int plane_num;
      double actual_height;
      
      if (MDV_field_ht_to_num(mdv_handle,
			      ifield,
			      plane_height,
			      &plane_num,
			      &actual_height) != 0)
	return(-1);
      
      if (plane_num >= 0)
      {
	/*
	 * First delete all of the planes following this one, then
	 * delete all of the planes preceding this one.  The order
	 * is important here because we can only delete the first
	 * or last plane in a volume without vlevel headers and
	 * still keep the header values consistent with the data.
	 */

	for (i = field_hdr->nz - 1; i > plane_num; i--)
	  MDV_remove_field_plane(mdv_handle, ifield, i);

	for (i = 0; i < plane_num; i++)
	  MDV_remove_field_plane(mdv_handle, ifield, 0);
      } /* endif - plane_num >= 0 */
      
    } /* endfor - ifield */
  } /* endif - plane_height >= 0 */
  
  /*
   * Crop the data, if requested.
   */

  if (crop_info != (MDV_request_crop_t *)NULL)
  {
    int i;
    
    /*
     * Crop planes.
     */

    if (MDV_crop_planes(mdv_handle,
			crop_info->min_lat, crop_info->max_lat,
			crop_info->min_lon, crop_info->max_lon) != 0)
    {
      fprintf(stderr, "WARNING: mdv_client::%s\n", routine_name);
      fprintf(stderr, "Error cropping planes to following bounds:\n");
      fprintf(stderr, "   min_lat = %f, max_lat = %f\n",
	      crop_info->min_lat, crop_info->max_lat);
      fprintf(stderr, "   min_lon = %f, max_lon = %f\n",
	      crop_info->min_lon, crop_info->max_lon);
    }
    
    /*
     * Crop heights.
     */

    if (crop_info->min_height >= 0.0 ||
	crop_info->max_height >= 0.0)
    {
      /*
       * See if we can handle this request.
       */
      
      if (plane_ht_type == mdv_handle->master_hdr.vlevel_type)
      {
	/*
	 * Perform the cropping.
	 */

	for (i = 0; i < mdv_handle->master_hdr.n_fields; i++)
	{
	  int minz, maxz;
	  double actual_height;
	  int j;
      
	  MDV_field_header_t *field_hdr = &mdv_handle->fld_hdrs[i];
      
	  /*
	   * Calculate the range of plane indexes to keep.
	   */

	  if (crop_info->min_height < 0.0)
	    minz = 0;
	  else
	  {
	    if (MDV_field_ht_to_num(mdv_handle,
				    i,
				    crop_info->min_height,
				    &minz,
				    &actual_height) != 0)
	      minz = 0;
	  }
      
	  if (crop_info->max_height < 0.0)
	    maxz = field_hdr->nz - 1;
	  else
	  {
	    if (MDV_field_ht_to_num(mdv_handle,
				    i,
				    crop_info->max_height,
				    &maxz,
				    &actual_height) != 0)
	      maxz = field_hdr->nz - 1;
	  }
      
	  /*
	   * Get rid of the unwanted planes.
	   */

	  for (j = field_hdr->nz - 1; j > maxz; j--)
	    MDV_remove_field_plane(mdv_handle, i, j);
	
	  for (j = 0; j < minz; j++)
	    MDV_remove_field_plane(mdv_handle, i, 0);
	} /* endfor - i */
      }
      else
      {
	fprintf(stderr, "WARNING: mdv_client::%s\n", routine_name);
	fprintf(stderr, "Request has plane heights in %s format units.\n",
		MDV_verttype2string(plane_ht_type));
	fprintf(stderr, "File has plane heights in %s format units.\n",
		MDV_verttype2string(mdv_handle->master_hdr.vlevel_type));
	fprintf(stderr,
		"Converting between plane height types not yet implemented.\n");
	fprintf(stderr, "Cropping not performed\n");
      }

    } /* endif - height cropping requested */
    
  } /* endif - cropping requested */
  
  /*
   * Composite the data
   */

  MDV_composite_data(mdv_handle, composite_type);
  
  return(0);
}


/*****************************************************
 * get_from_socket()
 *
 * Gets MDV data from the given source socket.
 *
 * Upon return:
 *      mdv_handle contains the MDV data.
 *
 * Returns 0 on success, -1 on failure
 */

static int get_from_socket(char *host,
			   int port,
			   int request_type,
			   void *request_info,
			   int request_info_len,
			   MDV_handle_t *mdv_handle)       /* output */
{
  static char *routine_name = "get_from_socket()";
  
  si32 request = request_type;
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
  
  /*
   * Open the socket from the client to the server.
   */

  if ((SockFd = SKU_open_client(host, port)) < 0)
  {
    fprintf(stderr, "ERROR: mdv_client::%s\n",
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
  
  msg_buffer = (char *)umalloc(buffer_len);
  
  buf_ptr = msg_buffer;
  
  /* request */
  memcpy(buf_ptr, &request, sizeof(si32));
  BE_from_array_32(buf_ptr, sizeof(si32));
  buf_ptr += sizeof(si32);
  
  /* request_info */
  if (request_info != NULL)
  {
    memcpy(buf_ptr, request_info, request_info_len);
    MDV_request_to_BE(request, buf_ptr, request_info_len);
  }
  
  /*
   * Send the request to the server.
   */

  if (SKU_write_message(SockFd, request_type,
			msg_buffer, buffer_len) != 1)
  {
    fprintf(stderr, "ERROR: mdv_client::%s\n",
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
		       MDV_CLIENT_WAIT_MSECS) != 1)
  {
    fprintf(stderr, "ERROR: mdv_client:%s\n",
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
  case MDV_DATA :
  {
    if (MDV_load_all(mdv_handle, buf_ptr, MDV_NATIVE) != 0)
    {
      fprintf(stderr, "ERROR: mdv_client::%s\n",
	      routine_name);
      fprintf(stderr, "Error loading MDV data from received buffer\n");
      
      return(-1);
    }
    
    break;
  }
    
  case MDV_NO_DATA :
  case MDV_DATA_ERROR :
    return(-1);
    break;
    
  default:
    fprintf(stderr, "ERROR: mdv_client::%s\n",
	    routine_name);
    fprintf(stderr, "Invalid reply %d received from server\n",
	    reply);
    
    return(-1);
    break;
  }
  
  return(0);
}


/*****************************************************
 * get_dataset_times_from_socket()
 *
 * Gets available dataset times from the given source socket.
 *
 * Upon return:
 *      num_datasets contains the number of datasets generated
 *                     during the requested period.
 *
 * Returns a pointer to a list of available data set times;
 * returns NULL if there are no available datasets within the
 * given time period or if there was an error.
 */

static MDV_dataset_time_t *get_dataset_times_from_socket(char *host,
							 int port,
							 MDV_dataset_time_request_t *request_info,
							 int *num_datasets) /* output */
{
  static char *routine_name = "get_dataset_times_from_socket()";
  
  si32 request = MDV_INFO_DATASET_TIMES_REQUEST;
  
  char *msg_buffer;
  char *buf_ptr;
  int buffer_len;
  long buffer_len_long;
  si32 reply;
  SKU_header_t sku_header;
  
  MDV_dataset_time_t *data_times;

  /*
   * Initialize return values.
   */

  *num_datasets = 0;
  data_times = (MDV_dataset_time_t *)NULL;
  
  /*
   * Make sure we are using the new headers.
   */

  SKU_set_headers_to_new();
  
  /*
   * Open the socket from the client to the server.
   */

  if ((SockFd = SKU_open_client(host, port)) < 0)
  {
    fprintf(stderr, "ERROR: mdv_client::%s\n",
	    routine_name);
    fprintf(stderr, "Could not open client socket on host %s, port %d\n",
	    host, port);
    return((MDV_dataset_time_t *)NULL);
  }
  
  SockFdOpen = TRUE;
  
  /*
   * Construct the request to send to the server.  Byte-swap the
   * header information as we do so.
   */

  buffer_len = sizeof(si32) + sizeof(MDV_dataset_time_request_t);
  
  msg_buffer = (char *)umalloc(buffer_len);
  
  buf_ptr = msg_buffer;
  
  /* request */
  memcpy(buf_ptr, &request, sizeof(si32));
  BE_from_array_32(buf_ptr, sizeof(si32));
  buf_ptr += sizeof(si32);
  
  /* request_info */
  memcpy(buf_ptr, request_info, sizeof(MDV_dataset_time_request_t));
  MDV_dataset_time_request_to_BE((MDV_dataset_time_request_t *)buf_ptr);
  
  /*
   * Send the request to the server.
   */

  if (SKU_write_message(SockFd, request,
			msg_buffer, buffer_len) != 1)
  {
    fprintf(stderr, "ERROR: mdv_client::%s\n",
	    routine_name);
    fprintf(stderr, "Error writing message to host %s, port %d\n",
	    host, port);
    
    SKU_close(SockFd);
    SockFdOpen = FALSE;
    ufree(msg_buffer);
    return((MDV_dataset_time_t *)NULL);
  }
  
  ufree(msg_buffer);
  
  /*
   * Get the response from the server.
   */

  if (SKU_read_message(SockFd, &sku_header,
		       &msg_buffer, &buffer_len_long,
		       MDV_CLIENT_WAIT_MSECS) != 1)
  {
    fprintf(stderr, "ERROR: mdv_client:%s\n",
	    routine_name);
    fprintf(stderr, "Error reading reply from server on host %s, port %d\n",
	    host, port);
    
    SKU_close(SockFd);
    SockFdOpen = FALSE;
    
    return((MDV_dataset_time_t *)NULL);
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
  case MDV_INFO_DATASET_TIMES_REPLY :
  {
    si32 num_datasets_received;
    
    int i;
    
    /*
     * Determine the number of datasets.
     */

    num_datasets_received = *(si32 *)buf_ptr;
    num_datasets_received = BE_to_si32(num_datasets_received);
    buf_ptr += sizeof(si32);
    
    /*
     * Make sure we received enough information over the socket.
     */

    if (buffer_len_long != (num_datasets_received * sizeof(MDV_dataset_time_t)) +
	(2 * sizeof(si32)))
    {
      fprintf(stderr, "ERROR: mdv_client:%s\n",
	      routine_name);
      fprintf(stderr, "Wrong number of bytes received over socket\n");
      fprintf(stderr, "Expected %ld bytes, received %d bytes\n",
	      (num_datasets_received * sizeof(MDV_dataset_time_t)) + (2 * sizeof(si32)),
	      (int)buffer_len_long);
      
      return((MDV_dataset_time_t *)NULL);
    }
    
    /*
     * Allocate space for the returned array.
     */

    data_times = (MDV_dataset_time_t *)umalloc(num_datasets_received *
					       sizeof(MDV_dataset_time_t));
    
    /*
     * Copy the returned data.
     */

    memcpy(data_times, buf_ptr, num_datasets_received * sizeof(MDV_dataset_time_t));
    
    /*
     * Byte-swap the returned data
     */

    for (i = 0; i < num_datasets_received; i++)
      MDV_dataset_time_from_BE(&data_times[i]);
    
    /*
     * Save the number of datasets.
     */

    *num_datasets = num_datasets_received;
    
    break;
  }
    
  default:
    fprintf(stderr, "ERROR: mdv_client::%s\n",
	    routine_name);
    fprintf(stderr, "Invalid reply %d received from server\n",
	    reply);
    
    return((MDV_dataset_time_t *)NULL);
    
    break;
  }
  
  return(data_times);
}
