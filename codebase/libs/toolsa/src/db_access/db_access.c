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
 * db_access.c
 *
 * Routines used by generic RAP database applications.
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * August 1998
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>

#include <toolsa/os_config.h>

#include <toolsa/db_access.h>
#include <toolsa/servmap.h>
#include <toolsa/smu.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>


/*
 ********** GLOBAL VARIABLES ******************
 */

char *Module_name = "db_access";


/*****************************************************
 * DB_location_type()
 *
 * Determines if the data location is disk or socket
 * depending on the location string.
 */

int DB_location_type(char *location_string)
{
  int colon_pos;
  int colon_pos2;
  int at_pos;
  
  /*
   * Check for a servmap location.  A servmap location string
   * looks like the following:  <type>::<subtype>::<instance>
   */

  if ((colon_pos = STRpos(location_string, ":")) > 0 &&
      strlen(location_string) > colon_pos + 5 &&
      location_string[colon_pos+1] == ':')
  {
    char *location_substring = location_string + colon_pos + 2;
    
    if ((colon_pos2 = STRpos(location_substring, ":")) > 0 &&
	strlen(location_substring) > colon_pos2 + 2 &&
	location_substring[colon_pos2+1] == ':')
    {
      return(DB_LOCATION_SERVMAP);
    }
  }
  
  /*
   * Now check for a socket location.  A socket location string
   * looks like the following: <port>@<host>
   */

  if ((at_pos = STRpos(location_string, "@")) > 0)
  {
    int i;
    int socket_flag = TRUE;
    
    for (i = 0; i < at_pos; i++)
      if (!isdigit(location_string[i]))
	socket_flag = FALSE;
    
    if (socket_flag)
      return(DB_LOCATION_SOCKET);
  }
  
  /*
   * Assume anything else is a disk location.
   */

  return(DB_LOCATION_DISK);
}


/**********************
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
		     int *port)
{
  static char *routine_name = "DB_get_host_port";
  
  int at_pos = STRpos(host_port_string, "@");
  
  /*
   * Make sure the host/port string is valid
   */

  if (at_pos <= 0 ||
      at_pos >= strlen(host_port_string) - 1)
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Invalid host/port string <%s>\n",
	    host_port_string);
    return(-1);
  }
  
  /*
   * Extract the port number from the string
   */

  *port = atoi(host_port_string);
  
  /*
   * Extract the host name from the string.
   */

  STRcopy(host, &host_port_string[at_pos+1], host_string_len);
  
  return(0);
}


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
			     int *port)
{
  static char *routine_name = "DB_get_servmap_host_port";
  
  static int first_time = TRUE;
  static char *servmap_host1;
  static char *servmap_host2;
    
  char servmap_type[SERVMAP_NAME_MAX];
  char servmap_subtype[SERVMAP_NAME_MAX];
  char servmap_instance[SERVMAP_INSTANCE_MAX];
  
  SERVMAP_request_t request;
  int num_servers;
  SERVMAP_info_t *info;
  int smu_return;
  
  /*
   * Get the server mapper hosts.
   */

  if (first_time)
  {
    SMU_get_servmap_hosts(&servmap_host1, &servmap_host2);

    first_time = FALSE;
  }
    
  /*
   * Get the type, subtype and instance from the destination string.
   */

  if (DB_get_servmap_info(location_string,
			  servmap_type, SERVMAP_NAME_MAX,
			  servmap_subtype, SERVMAP_NAME_MAX,
			  servmap_instance, SERVMAP_INSTANCE_MAX) != 0)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    Module_name, routine_name);
    fprintf(stderr,
	    "Could not parse <%s> for servmap type, subtype and instance\n",
	    location_string);
    return(-1);
  }
  
  /*
   * Get the host and port from the server mapper.
   */

  STRcopy(request.server_type, servmap_type, SERVMAP_NAME_MAX);
  STRcopy(request.server_subtype, servmap_subtype, SERVMAP_NAME_MAX);
  STRcopy(request.instance, servmap_instance, SERVMAP_INSTANCE_MAX);
  request.want_realtime = FALSE;
  request.time = 0;
    
  smu_return = SMU_requestInfo(&request, &num_servers,
			       &info,
			       servmap_host1, servmap_host2);
  if (smu_return != 1)
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr,
	    "Error getting host/port for location <%s> from server mapper, smu_return = %d\n",
	    location_string, smu_return);
    return(-1);
  }
    
  if (num_servers <= 0)
  {
    fprintf(stderr, "WARNING - spdb:%s\n",
	    routine_name);
    fprintf(stderr,
	    "No servers for location <%s> registered with server mapper\n",
	    location_string);
    return(-1);
  }

  /*
   * Return the host/port information
   */

  STRcopy(host, info[0].host, host_len);
  *port = info[0].port;
  
  return(0);
}

    
/*************************
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
			int servmap_instance_len)
{
  static char *routine_name = "DB_get_servmap_info";
  
  char *location_substring;
  int colon_pos = STRpos(location_string, ":");
  
  /*
   * Check for errors around the servmap type
   */

  if (colon_pos <= 0 ||
      colon_pos >= strlen(location_string) - 5 ||
      location_string[colon_pos+1] != ':')
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Invalid servmap string <%s>\n",
	    location_string);
    return(-1);
  }
  
  /*
   * Extract the servmap type from the string
   */

  STRcopy(servmap_type, location_string,
	  MIN(servmap_type_len, colon_pos+1));
  
  /*
   * Check for errors around the servmap subtype
   */

  location_substring = location_string + colon_pos + 2;
  colon_pos = STRpos(location_substring, ":");
  
  if (colon_pos <= 0 ||
      colon_pos >= strlen(location_substring) - 2 ||
      location_substring[colon_pos+1] != ':')
  {
    fprintf(stderr, "ERROR - spdb:%s\n",
	    routine_name);
    fprintf(stderr, "Invalid servmap string <%s>\n",
	    location_string);
    return(-1);
  }
  
  /*
   * Extract the servmap subtype from the string
   */

  STRcopy(servmap_subtype, location_substring,
	  MIN(servmap_subtype_len, colon_pos+1));
  
  /*
   * Extract the servmap instance from the string
   */

  STRcopy(servmap_instance,
	  location_substring + colon_pos + 2,
	  servmap_instance_len);
  
  return(0);
}


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

DB_url_t *DB_decode_url(char *url_string)
{
  static char *routine_name = "DB_decode_url";
  
  static DB_url_t url_struct;
  static int first_time = TRUE;
  
  char *begin_string = url_string;
  char *delim_pos;
  int string_len;
  
  /*
   * Initialize the returned values.
   */

  if (!first_time)
  {
    if (url_struct.protocol != (char *)NULL)
      ufree(url_struct.protocol);

    if (url_struct.translator != (char *)NULL)
      ufree(url_struct.translator);
    
    if (url_struct.param_file != (char *)NULL)
      ufree(url_struct.param_file);
    
    if (url_struct.host != (char *)NULL)
      ufree(url_struct.host);
    
    if (url_struct.file != (char *)NULL)
      ufree(url_struct.file);
    
    if (url_struct.args != (char *)NULL)
      ufree(url_struct.args);
  }
  
  first_time = FALSE;

  url_struct.protocol = (char *)NULL;
  url_struct.translator = (char *)NULL;
  url_struct.param_file = (char *)NULL;
  url_struct.host = (char *)NULL;
  url_struct.port = 0;
  url_struct.file = (char *)NULL;
  url_struct.args = (char *)NULL;
  
  /*
   * Pull off the required protocol string.
   */

  if ((delim_pos = strstr(begin_string, DB_PROTO_TRANS_DELIM))
      == (char *)NULL)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", Module_name, routine_name);
    fprintf(stderr,
	    "URL string doesn't contain protocol/translator delimiter \"%s\"\n",
	    DB_PROTO_TRANS_DELIM);
    
    return((DB_url_t *)NULL);
  }
  
  if (delim_pos == begin_string)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", Module_name, routine_name);
    fprintf(stderr,
	    "URL string doesn't have a protocol specified\n");
    
    return((DB_url_t *)NULL);
  }
  
  string_len = delim_pos - begin_string + 1;
  url_struct.protocol = (char *)umalloc(string_len);
  STRcopy(url_struct.protocol, begin_string, string_len);
  
  begin_string = delim_pos + strlen(DB_PROTO_TRANS_DELIM);
  
  /*
   * Pull off the optional translator string.
   */

  if ((delim_pos = strstr(begin_string, DB_TRANS_PARAMS_DELIM))
      == (char *)NULL)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", Module_name, routine_name);
    fprintf(stderr,
	    "URL string doesn't contain translator/params delimiter \"%s\"\n",
	    DB_TRANS_PARAMS_DELIM);
    
    return((DB_url_t *)NULL);
  }
  
  if (delim_pos != begin_string)
  {
    string_len = delim_pos - begin_string + 1;
    url_struct.translator = (char *)umalloc(string_len);
    STRcopy(url_struct.translator, begin_string, string_len);
  }
  
  begin_string = delim_pos + strlen(DB_TRANS_PARAMS_DELIM);
  
  /*
   * Pull off the optional parameter file string.
   */

  if ((delim_pos = strstr(begin_string, DB_PARAMS_HOST_DELIM))
      == (char *)NULL)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", Module_name, routine_name);
    fprintf(stderr,
	    "URL string doesn't contain params/host delimiter \"%s\"\n",
	    DB_PARAMS_HOST_DELIM);
    
    return((DB_url_t *)NULL);
  }
  
  if (delim_pos != begin_string)
  {
    string_len = delim_pos - begin_string + 1;
    url_struct.param_file = (char *)umalloc(string_len);
    STRcopy(url_struct.param_file, begin_string, string_len);
  }
  
  begin_string = delim_pos + strlen(DB_PARAMS_HOST_DELIM);
  
  /*
   * Pull off the optional host string.
   */

  if ((delim_pos = strstr(begin_string, DB_HOST_PORT_DELIM))
      == (char *)NULL)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", Module_name, routine_name);
    fprintf(stderr,
	    "URL string doesn't contain host/port delimiter \"%s\"\n",
	    DB_HOST_PORT_DELIM);
    
    return((DB_url_t *)NULL);
  }
  
  if (delim_pos != begin_string)
  {
    string_len = delim_pos - begin_string + 1;
    url_struct.host = (char *)umalloc(string_len);
    STRcopy(url_struct.host, begin_string, string_len);
  }
  
  begin_string = delim_pos + strlen(DB_HOST_PORT_DELIM);
  
  /*
   * Pull off the optional port number.  Need to add error checking
   * for non-numeric in port number.
   */

  if ((delim_pos = strstr(begin_string, DB_PORT_FILE_DELIM))
      == (char *)NULL)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", Module_name, routine_name);
    fprintf(stderr,
	    "URL string doesn't contain port/file delimiter \"%s\"\n",
	    DB_PORT_FILE_DELIM);
    
    return((DB_url_t *)NULL);
  }
  
  if (delim_pos != begin_string)
  {
    char port_string[BUFSIZ];
    
    string_len = delim_pos - begin_string + 1;
    STRcopy(port_string, begin_string, string_len);

    url_struct.port = atoi(port_string);
  }
  
  begin_string = delim_pos + strlen(DB_PORT_FILE_DELIM);
  
  /*
   * Pull off the required file string and the optional args
   * string.  Note that the file/args delimiter is optional unless
   * an args string is specified.
   */

  if ((delim_pos = strstr(begin_string, DB_FILE_ARGS_DELIM))
      == (char *)NULL)
  {
    if (strlen(begin_string) <= 0)
    {
      fprintf(stderr,
	      "ERROR: %s::%s\n", Module_name, routine_name);
      fprintf(stderr,
	      "URL doesn't contain required file field\n");
      
      return((DB_url_t *)NULL);
    }
	
    url_struct.file = STRdup(begin_string);
  }
  else
  {
    if (delim_pos == begin_string)
    {
      fprintf(stderr,
	      "ERROR: %s::%s\n", Module_name, routine_name);
      fprintf(stderr,
	      "URL doesn't contain required file field\n");
      
      return((DB_url_t *)NULL);
    }
  
    string_len = delim_pos - begin_string + 1;
    url_struct.file = (char *)umalloc(string_len);
    STRcopy(url_struct.file, begin_string, string_len);
  
    begin_string = delim_pos + strlen(DB_PROTO_TRANS_DELIM);
  
    if (strlen(begin_string) > 0)
      url_struct.args = STRdup(begin_string);
  }
  
  return(&url_struct);
}


/*************************
 * DB_encode_url()
 *
 * Encodes a RAP URL string from the given DB_url_t structure.
 *
 * Returns a pointer to the encoded URL string.  The returned
 * pointer points to static memory and so should not be freed
 * by the calling routine.  Returns NULL on error.
 */

char *DB_encode_url(DB_url_t *url_struct)
{
  static char *routine_name = "DB_encode_url";
  
  static char *url_string = (char *)NULL;
  static int url_string_alloc = 0;
  
  int max_string_len;
  char port_string[BUFSIZ];
  
  /*
   * Check for missing required fields.
   */

  if (url_struct->protocol == (char *)NULL ||
      url_struct->protocol[0] == '\0')
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", Module_name, routine_name);
    fprintf(stderr,
	    "Protocol field MUST be specified in URL\n");
    
    return((char *)NULL);
  }
  
  if (url_struct->file == (char *)NULL ||
      url_struct->file[0] == '\0')
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", Module_name, routine_name);
    fprintf(stderr,
	    "File path field MUST be specified in URL\n");
    
    return((char *)NULL);
  }
  
  /*
   * Convert the port number to a string.
   */

  if (url_struct->port <= 0)
    port_string[0] = '\0';
  else
    sprintf(port_string, "%d", url_struct->port);
  
  /*
   * Calculate the required length of the returned string and
   * allocate the space.
   */

  max_string_len = 1;    /* NULL terminator */
  max_string_len += strlen(url_struct->protocol);
  max_string_len += strlen(DB_PROTO_TRANS_DELIM);
  if (url_struct->translator != (char *)NULL)
    max_string_len += strlen(url_struct->translator);
  max_string_len += strlen(DB_TRANS_PARAMS_DELIM);
  if (url_struct->param_file != (char *)NULL)
    max_string_len += strlen(url_struct->param_file);
  max_string_len += strlen(DB_PARAMS_HOST_DELIM);
  if (url_struct->host != (char *)NULL)
    max_string_len += strlen(url_struct->host);
  max_string_len += strlen(DB_HOST_PORT_DELIM);
  max_string_len += strlen(port_string);
  max_string_len += strlen(DB_PORT_FILE_DELIM);
  max_string_len += strlen(url_struct->file);
  if (url_struct->args != (char *)NULL)
  {
    max_string_len += strlen(DB_FILE_ARGS_DELIM);
    max_string_len += strlen(url_struct->args);
  }
  
  if (max_string_len > url_string_alloc)
  {
    url_string_alloc = max_string_len;
    
    if (url_string == (char *)NULL)
      url_string = (char *)umalloc(url_string_alloc);
    else
      url_string = (char *)urealloc(url_string, url_string_alloc);
  }
  
  /*
   * Now construct the string.
   */

  STRcopy(url_string, url_struct->protocol, url_string_alloc);
  STRconcat(url_string, DB_PROTO_TRANS_DELIM, url_string_alloc);
  if (url_struct->translator != (char *)NULL)
    STRconcat(url_string, url_struct->translator, url_string_alloc);
  STRconcat(url_string, DB_TRANS_PARAMS_DELIM, url_string_alloc);
  if (url_struct->param_file != (char *)NULL)
    STRconcat(url_string, url_struct->param_file, url_string_alloc);
  STRconcat(url_string, DB_PARAMS_HOST_DELIM, url_string_alloc);
  if (url_struct->host != (char *)NULL)
    STRconcat(url_string, url_struct->host, url_string_alloc);
  STRconcat(url_string, DB_HOST_PORT_DELIM, url_string_alloc);
  STRconcat(url_string, port_string, url_string_alloc);
  STRconcat(url_string, DB_PORT_FILE_DELIM, url_string_alloc);
  STRconcat(url_string, url_struct->file, url_string_alloc);
  if (url_struct->args != (char *)NULL)
  {
    STRconcat(url_string, DB_FILE_ARGS_DELIM, url_string_alloc);
    STRconcat(url_string, url_struct->args, url_string_alloc);
  }
  
  return(url_string);
}
