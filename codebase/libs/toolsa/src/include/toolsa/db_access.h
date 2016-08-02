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
 * db_access.h: header file for the database utility routines.
 *
 ******************************************************************/

#ifndef db_access_h
#define db_access_h


/*
 * Input/output location types.
 */

#define DB_LOCATION_SOCKET    1
#define DB_LOCATION_DISK      2
#define DB_LOCATION_SERVMAP   3

/*
 * RAP URL delimiters.
 */

#define DB_PROTO_TRANS_DELIM    ":"
#define DB_TRANS_PARAMS_DELIM   ":"
#define DB_PARAMS_HOST_DELIM    "//"
#define DB_HOST_PORT_DELIM      ":"
#define DB_PORT_FILE_DELIM      ":"
#define DB_FILE_ARGS_DELIM      "?"


/*************************
 * Defined types
 */

typedef struct
{
  char *protocol;
  char *translator;
  char *param_file;
  char *host;
  int port;
  char *file;
  char *args;
} DB_url_t;


/*
 * prototypes
 */

/*****************************************************
 * DB_location_type()
 *
 * Determines if the data location is disk or socket
 * depending on the location string.
 */

int DB_location_type(char *location_string);

/*****************************************************
 * DB_get_servmap_host_port()
 *
 * Get the server host and port information from the server mapper.
 *
 * Returns -1 if it wasn't able to get the host/port for some reason,
 * 0 otherwise.
 */

int DB_get_servmap_host_port(char *location_string,
			     char *host,
			     int host_len,
			     int *port);

/***************
 * DB_get_host_port()
 *
 * Gets the host and port information for a given string.
 *
 * Returns the host and port information in the calling
 * arguments.  Space must already be allocated for the host
 * name.
 *
 * Host/port information is stored in the following format:
 *     "port@host"
 *
 * Returns 0 on success, -1 on failure.
 */

int DB_get_host_port(char *host_port_string,
		     char *host,
		     int host_string_len,
		     int *port);

/***************
 * DB_get_servmap_info()
 *
 * Gets the server mapper information for a given location string.
 *
 * Returns the servmap type, subtype and instance information in the
 * calling arguments.  Space must already be allocated for the returned
 * information.
 *
 * Server mapper information is stored in the following format:
 *     "type::subtype::instance"
 *
 * Returns 0 on success, -1 on failure.
 */

int DB_get_servmap_info(char *location_string,
			char *servmap_type,
			int servmap_type_len,
			char *servmap_subtype,
			int servmap_subtype_len,
			char *servmap_instance,
			int servmap_instance_len);


/*************************
 * DB_decode_url()
 *
 * Decodes the RAP URL string into a DB_url_t structure.
 *
 * Returns a pointer to the decoded structure.  The
 * returned pointer points to static memory and so should
 * not be freed by the calling routine.  Returns NULL on
 * error.
 */

DB_url_t *DB_decode_url(char *url_string);


/*************************
 * DB_encode_url()
 *
 * Encodes a RAP URL string from the given DB_url_t structure.
 *
 * Returns a pointer to the encoded URL string.  The returned
 * pointer points to static memory and so should not be freed
 * by the calling routine.  Returns NULL on error.
 */

char *DB_encode_url(DB_url_t *url_struct);

#endif

#ifdef __cplusplus
}
#endif
