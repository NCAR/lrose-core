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
 * mdv_client.h: header file for MDV client routines.
 *
 ******************************************************************/

#ifndef mdv_client_h
#define mdv_client_h

#include <stdio.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_utils.h>
#include <toolsa/servmap.h>

/*
 * The MDV client routines support the following interface:
 *
 * Requests to retrieve data
 * -------------------------
 *
 * Requests:
 *
 * [Get the MDV data at the closest time to that requested,
 *  within the time margin.]
 *                si32 request_id = MDV_GET_CLOSEST
 *                MDV_request_closest_t request_info
 *
 * [Get the first MDV data before the requested time, within
 *  the time margin.]
 *                si32 request_id = MDV_GET_FIRST_BEFORE
 *                MDV_request_closest_t request_info
 *
 * [Get the first MDV data after the requested time, within
 *  the time margin.]
 *                si32 request_id = MDV_GET_FIRST_AFTER
 *                MDV_request_closest_t request_info
 *
 * [Get the latest MDV data.]
 *                si32 request_id = MDV_GET_LATEST
 *                MDV_request_latest_t request_info
 *
 * [Get the latest new MDV data, after the given data time.]
 *                si32 request_id = MDV_GET_NEW
 *                MDV_request_new_t request_info
 *
 * Replies:
 *
 * [The data was available and here it is.]
 *                si32 reply_ID = MDV_DATA
 *                MDV_master_header_t master_hdr
 *                MDV_field_header_t field_hdr_1
 *                     .
 *                     .
 *                     .
 *                MDV_field_header_t field_hdr_n
 *                MDV_vlevel_header_t vlevel_hdr_1
 *                     .
 *                     .
 *                     .
 *                MDV_vlevel_header_t vlevel_hdr_n
 *                MDV_chunk_header_t chunk_hdr_1
 *                     .
 *                     .
 *                     .
 *                MDV_chunk_header_t chunk_hdr_n
 *                field 1 data
 *                     .
 *                     .
 *                     .
 *                field n data
 *                chunk 1 data
 *                     .
 *                     .
 *                     .
 *                chunk n data
 *
 * [There is no data matching the request.]
 *                si32 reply = MDV_NO_DATA
 *
 * [There was an error retrieving the data requested.]
 *                si32 reply = MDV_DATA_ERROR
 *
 * Requests to store data
 * ----------------------
 *
 * Requests:
 *
 * [Store the given MDV data.]
 *                si32 request_id = MDV_PUT_DATA
 *                MDV_master_header_t master_hdr
 *                MDV_field_header_t field_hdr_1
 *                     .
 *                     .
 *                     .
 *                MDV_field_header_t field_hdr_n
 *                MDV_vlevel_header_t vlevel_hdr_1
 *                     .
 *                     .
 *                     .
 *                MDV_vlevel_header_t vlevel_hdr_n
 *                MDV_chunk_header_t chunk_hdr_1
 *                     .
 *                     .
 *                     .
 *                MDV_chunk_header_t chunk_hdr_n
 *                field 1 data
 *                     .
 *                     .
 *                     .
 *                field n data
 *                chunk 1 data
 *                     .
 *                     .
 *                     .
 *                chunk n data
 *
 * Replies:
 *
 * [The data was stored successfully.]
 *                 si32 reply = MDV_PUT_SUCCESSFUL
 *
 * [There was an error storing the data.]
 *                 si32 reply = MDV_PUT_FAILED
 *
 * Requests to get data information
 * --------------------------------
 *
 * Request:
 *
 * [Retrieve the available dataset times.]
 *                si32 request_id = MDV_INFO_DATASET_TIMES_REQUEST
 *                MDV_dataset_time_request_t dataset_time_request
 *
 * Replies:
 *
 * [Here are the available data times.]
 *                 si32 reply = MDV_INFO_DATASET_TIMES_REPLY
 *                 si32 num_datasets  (could be 0)
 *                 MDV_dataset_time_t dataset_time_1
 *                     .
 *                     .
 *                     .
 *                 MDV_dataset_time_t dataset_time_n
 *
 * Replies to erroneous requests:
 * ------------------------------
 *
 * Replies:
 *
 * [The request could not be parsed.]
 *                 si32 reply = MDV_REQUEST_ERROR
 *
 *
 */

/*
 * Client request ids.
 */

#define MDV_GET_CLOSEST                  11000
#define MDV_GET_FIRST_BEFORE             11001
#define MDV_GET_FIRST_AFTER              11002
#define MDV_GET_LATEST                   11003
#define MDV_GET_NEW                      11004

#define MDV_PUT_DATA                     12000

#define MDV_INFO_DATASET_TIMES_REQUEST   13000

/*
 * Server reply ids.
 */

#define MDV_DATA                         15000
#define MDV_NO_DATA                      15001
#define MDV_DATA_ERROR                   15002

#define MDV_PUT_SUCCESSFUL               16000
#define MDV_PUT_FAILED                   16001

#define MDV_INFO_DATASET_TIMES_REPLY     17000

#define MDV_REQUEST_ERROR                19999


/****************************
 * Client get data request structures.  Note that when byte-swapping
 * request buffers, it is assumed that all of the fields are 32-bits.
 * If this is changed, some code changes will also have to be made in
 * MDV_request_to_BE() and MDV_request_from_BE().
 */

/*
 * Cropping information.
 */

typedef struct
{
  fl32 min_lat;            /* Minimum latitude of cropped data. */
  fl32 min_lon;            /* Minimum longitude of cropped data. */
  fl32 max_lat;            /* Maximum latitude of cropped data. */
  fl32 max_lon;            /* Maximum longitude of cropped data. */
  fl32 min_height;         /* Minimum height of cropped data in units */
                           /*   defined by plane_height_type below. */
  fl32 max_height;         /* Maximum height of cropped data in units */
                           /*   defined by plane_height_type below. */
} MDV_request_crop_t;


/*
 * Generic request information.
 */

typedef struct
{
  ui08 field_name[MDV_LONG_FIELD_LEN];
                           /* Requested field name, only used if */
                           /*   requested field number == -2. */
  si32 field_num;          /* Requested field number. */
  si32 return_type;        /* Data format to return (e.g. MDV_INT8). */
  si32 composite_type;     /* Composite type to be performed */
                           /*   (e.g. MDV_COMPOSITE_MAX). */
  si32 plane_height_type;  /* Value type for all height information */
                           /*   in the request.  This value tells the */
                           /*   server how to interpret height values */
                           /*   (km, etc.) in the request.  (e.g. */
                           /*   VERT_TYPE_Z) */
  si32 plane_height;       /* Height of requested plane in units */
                           /*   defined by plane_height_type. */
  si32 crop_flag;          /* If non-zero, returned data will be cropped */
                           /*   based on the info in crop_info. */
  MDV_request_crop_t crop_info;
                           /* Information on how to crop the returned */
                           /*   data.  Ignored if crop_flag is 0. */
} MDV_request_t;


/*
 * Request structure used with MDV_GET_CLOSEST, MDV_GET_FIRST_BEFORE
 * and MDV_GET_FIRST_AFTER requests.
 */

typedef struct
{
  MDV_request_t gen_info;  /* Generic request information. */

  si32 time;               /* Requested data time in UNIX format. */
  si32 margin;             /* Allowed time margin in seconds. */
} MDV_request_closest_t;


/*
 * Request structure used with MDV_GET_LATEST requests.
 */

typedef struct
{
  MDV_request_t gen_info;  /* Generic request information. */
} MDV_request_latest_t;


/*
 * Request structure used with MDV_GET_NEW requests.
 */

typedef struct
{
  MDV_request_t gen_info;  /* Generic request information. */

  si32 last_data_time;     /* Last data time from source. */
} MDV_request_new_t;


/****************************
 * Client information structures.
 */

/*
 * Dataset times request information.
 */

typedef struct
{
  si32 begin_gen_time;        /* Beginning dataset generation time for desired */
                              /*   dataset times. */
  si32 end_gen_time;          /* Ending dataset generation time for desired */
                              /*   dataset times. */
} MDV_dataset_time_request_t;

/*
 * Data times reply information.
 */

typedef struct
{
  si32 gen_time;          /* Dataset generation time. */
  si32 forecast_time;     /* Dataset forecast time.  For datasets that don't */
                          /*   include forecasts, this value will be -1. */
} MDV_dataset_time_t;


/*
 * prototypes
 */

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
	    MDV_handle_t *mdv_handle);

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
		    MDV_handle_t **mdv_handle);       /* output */

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
			 MDV_handle_t **mdv_handle);       /* output */

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
			MDV_handle_t **mdv_handle);       /* output */

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
		   MDV_handle_t **mdv_handle);       /* output */

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
		MDV_handle_t **mdv_handle);       /* output */

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

void MDV_free_get(void);

/*****************************************************
 * MDV_handle_sigpipe()
 *
 * Closes any open sockets that were used by the MDV
 * routines.  Should be called by the application's
 * interrupt handler when a SIGPIPE interrupt is
 * received.
 * 
 */

void MDV_handle_sigpipe(void);

/*****************************************************
 * MDV_request_size()
 *
 * Returns the number of bytes in a request of the given
 * type.  Returns -1 if the request type isn't recognized.
 */

int MDV_request_size(int request);

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

void MDV_request_to_BE(int request, void *request_ptr, int request_len);

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

void MDV_request_from_BE(int request, void *request_ptr, int request_len);

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
					  int *num_datasets);       /* output */

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

void MDV_dataset_time_request_to_BE(MDV_dataset_time_request_t *request_ptr);

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

void MDV_dataset_time_request_from_BE(MDV_dataset_time_request_t *request_ptr);

/*****************************************************
 * MDV_dataset_time_to_BE()
 *
 * Converts the given dataset time information structure from native format to
 * big-endian format.
 *
 * Parameters:
 *       info_ptr        pointer to information structure, in native format.
 */

void MDV_dataset_time_to_BE(MDV_dataset_time_t *info_ptr);

/*****************************************************
 * MDV_dataset_time_from_BE()
 *
 * Converts the given dataset time information structure from big-endian format to
 * native format.
 *
 * Parameters:
 *       info_ptr     pointer to information structure, in big-endian format.
 */

void MDV_dataset_time_from_BE(MDV_dataset_time_t *info_ptr);

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

char *MDV_request2string(int request);

/*****************************************************
 * MDV_reply2string()
 *
 * Converts a reply value to the matching string.
 *
 * Returns a pointer to a static area containing the string.
 * Do NOT free this pointer.
 */

char *MDV_reply2string(int reply);

/*****************************************************
 * MDV_reap_children()
 *
 * Check for any child processes that are finished and
 * clean them up.  This keeps us from getting overwhelmed
 * by "zombie" processes.
 */

void MDV_reap_children(void);


#endif

#ifdef __cplusplus
}
#endif
