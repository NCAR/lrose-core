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
/*****************************************************************************
 * SKU_NAMED.C
 *
 * Named server interface
 */

#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/smu.h>

/******************
 * check_name_len()
 *
 * Checks there is space for name.
 * returns 1 on success, -1 on failure
 */

static int check_name_len(const char *str, int max_len,
			  const char *name)

{

  if (strlen(str) > max_len - 1) {
    fprintf(stderr, "sockutil - %s name too long - '%s'\n",
	    name, str);
    return (-1);
  } else {
    return (1);
  }

}
/*********************************************************************
 * SKU_init_named_server()
 *
 * Initializes server struct
 *
 * returns 1 on success, -1 on failure
 *
 *********************************************************************/

int SKU_init_named_server(SKU_named_server_t *server,
			  const char *servmap_host_1,
			  const char *servmap_host_2,
			  const char *type,
			  const char *subtype,
			  const char *instance,
			  const char *default_host,
			  int default_port)
		     
{
  
  /*
   * check name len for each field
   */
  
  int iret = 1;
  iret = check_name_len(servmap_host_1, SERVMAP_HOST_MAX,
			"Servmap host 1");
  iret = check_name_len(servmap_host_2, SERVMAP_HOST_MAX,
			"Servmap host 2");
  iret = check_name_len(type, SERVMAP_NAME_MAX,
			"Server type");
  iret = check_name_len(subtype, SERVMAP_NAME_MAX,
			"Server subtype");
  iret = check_name_len(instance, SERVMAP_INSTANCE_MAX,
			"Server instance");
  iret = check_name_len(default_host, SERVMAP_HOST_MAX,
			"Default host");

  if (iret < 0) {
    return (-1);
  }

  /*
   * copy in the data
   */

  strcpy(server->servmap_host_1, servmap_host_1);
  strcpy(server->servmap_host_2, servmap_host_2);
  strcpy(server->type, type);
  strcpy(server->subtype, subtype);
  strcpy(server->instance, instance);
  strcpy(server->default_host, default_host);
  server->default_port = default_port;
  strcpy(server->host, "");
  server->port = 0;
  server->fd = -1;

  return (1);

}

/*************************************************************************
 * SKU_open_named_conn: Open an Internet socket stream to
 * server named via the server mapper
 *
 * Returns file discriptor, or -1 on error
 */

int SKU_open_named_conn(SKU_named_server_t *server)

{

  SERVMAP_request_t request;
  SERVMAP_info_t *info;
  int n_servers;
  int fd;
  
  /*
   * load up servmap request struct
   */
  
  memset ((void *)  &request,
	  (int) 0, (size_t) sizeof(SERVMAP_request_t));
  
  STRncopy(request.server_type, server->type, SERVMAP_NAME_MAX);
  STRncopy(request.server_subtype, server->subtype,
	   SERVMAP_NAME_MAX);
  STRncopy(request.instance, server->instance,
	   SERVMAP_INSTANCE_MAX);

  /*
   * get server host and port
   */

  if (SMU_requestInfo(&request, (int *) &n_servers, &info,
		      server->servmap_host_1,
		      server->servmap_host_2) != 1) {
    /* failure - use defaults */
    STRncopy(server->host, server->default_host, SERVMAP_HOST_MAX);
    server->port = server->default_port;
  } else {
    /* success - set from server mapper, user first server returned */
    STRncopy(server->host, info->host, SERVMAP_HOST_MAX);
    server->port = info->port;
  }
  
  /*
   * set sockutil headers to new in case they were set to old by
   * the server mapper
   */
  
  SKU_set_headers_to_new();
  
  /*
   * open connection to server
   */

  fd = SKU_open_client(server->host, server->port);
  server->fd = fd;
  return(1);

}

/*************************************************************************
 * SKU_close_named_conn: closes connection to named server
 *
 */

void SKU_close_named_conn(SKU_named_server_t *server)

{
  
  SKU_close(server->fd);
  server->fd = -1;

}

/*********************************************************************
 * SKU_read_message_named()
 *
 * reads a RAP header and message, timing out as necessary.
 *
 * allocs or reallocs memory for the data.
 *
 * returns 1 on success, -1 on error
 *
 *********************************************************************/

int SKU_read_message_named(SKU_named_server_t *server,
			   SKU_header_t *header,
			   char **data,
			   long *data_len,
			   long wait_msecs)
{

  if (SKU_read_message(server->fd, header, data,
		       data_len, wait_msecs) == 1) {
    return (1);
  } else {
    SKU_close_named_conn(server);
    return (-1);
  }

}

/******************************************************
 * SKU_read_message_auto_connect()
 *
 * Gets message, connecting or reconnecting as required.
 * Only tries 1 reconnect per call.
 *
 * Returns 1 on success without reconnect, -1 on failure
 */

int SKU_read_message_auto_connect(SKU_named_server_t *server,
				  SKU_header_t *header,
				  char **data,
				  long *data_len,
				  long wait_msecs)

{

  /*
   * if not connected, connect
   */

  if (server->fd < 0) {
    if (SKU_open_named_conn(server) != 1) {
      return (-1);
    }
  }

  /*
   * read message
   */
  
  if (SKU_read_message(server->fd, header,
		       data, data_len, wait_msecs) == 1) {

    /*
     * success
     */

    return (1);

  } else {

    /*
     * failure - try again
     */
  
    if (server->fd < 0) {
      if (SKU_open_named_conn(server) != 1) {
	return (-1);
      }
    }
    
    return (SKU_read_message(server->fd, header,
			     data, data_len, wait_msecs));

  }

}

/*********************************************************************
 *
 * SKU_write_message_named()
 *
 * Writes message to named server
 *
 * returns 1 on success, -1 on failure
 *
 *********************************************************************/

int SKU_write_message_named(SKU_named_server_t *server,
			    long product_id,
			    char *data, long len)
{
  
  if (SKU_write_message(server->fd, product_id,
			data, len) == 1) {
    return (1);
  } else {
    SKU_close_named_conn(server);
    return (-1);
  }

}


