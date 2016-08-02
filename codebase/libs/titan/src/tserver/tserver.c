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
/************************************************************************
 * tserver.c
 *
 * Track data server routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * May 1993
 *
 *************************************************************************/

#include <titan/tdata_index.h>
#include <dataport/bigend.h>
#include <toolsa/sockutil.h>
#include <netinet/in.h>
#include <sys/time.h>



#define FLAG_POS (SERVMAP_USER_DATA_MAX - 1)

#define SERVER_ERROR -1
#define REQUEST_FAILURE -2
#define REQUEST_SUCCESS 0

#define NBYTES_BUF_INIT 100000

/*************************
 * module scope prototypes
 */

static int connect_to_server(char *host,
			     int port,
			     tdata_index_t *index);

static int find_a_server(tdata_index_t *index);

static int get_data(tdata_index_t *index, si32 command);

static void hangup(tdata_index_t *index);

static void load_string(const char *source, char **target);

static int query_server_mapper(tdata_index_t *index);

static int read_basic_with_params(tdata_index_t *index);
static int read_basic_without_params(tdata_index_t *index);
static int read_complete(tdata_index_t *index);

static int read_from_buffer(char *data,
			    si32 nbytes,
			    int id,
			    tdata_index_t *index);

static int read_notify(tdata_index_t *index);

static int read_reply(tdata_reply_t *reply,
		      tdata_index_t *index);

static int request_data(tdata_index_t *index, si32 command);

static int request_notify(tdata_index_t *index);

static int set_max_message_len(tdata_index_t *index);

static int verify_connect(tdata_index_t *index);

/*****************
 * public routines
 */

/*********************************************************************
 * tserver_check_notify()
 *
 * Returns TRUE if new track data is available, FALSE if not.
 *
 * Returns -1 on error
 *
 *********************************************************************/

int tserver_check_notify(tdata_index_t *index)
     
{
  
  fd_set read_fd;
  timeval_t timeout;

  if (!index->connected_to_server)
    return (-1);

  if (index->tserver_fd < 0)
    return (-1);

  /*
   * use a select to check if there is any message to be read
   */
  
  FD_ZERO(&read_fd);
  FD_SET(index->tserver_fd, &read_fd);
  memset ((void *)  &timeout,
	  (int) 0, (size_t) sizeof(timeval_t));

  select(FD_SETSIZE, FD_SET_P &read_fd, FD_SET_P 0, FD_SET_P 0, &timeout);
  
  if (FD_ISSET(index->tserver_fd, &read_fd)) {
    
    /*
     * server wants to notify client that data is available
     */
    
    if (read_notify(index)) {
      
      fprintf(stderr,
	      "WARNING - %s:tserver_check_notify\n",
	      index->prog_name);
      fprintf(stderr, "Reading data notification\n");
      hangup(index);
      
    }
    
    return (TRUE);
    
  } else {
    
    return (FALSE);
    
  }
  
}

/***********************************************************************
 *
 * tserver_clear()
 *
 * hangs up server, free up all memory
 */

void tserver_clear(tdata_index_t *index)
     
{
 
  hangup(index);

  tserver_free_all(index);
  
}

/***********************************************************************
 *
 * tserver_free_all()
 *
 * free up all memory associated with track server routines
 */

void tserver_free_all(tdata_index_t *index)
     
{
 
  tserver_free_track_data(index);
  tserver_free_read_buffer(index);

  ufree(index->default_host);
  index->default_host = (char *) NULL;
  
  ufree(index->prog_name);
  index->prog_name = (char *) NULL;
  
  ufree(index->server_instance);
  index->server_instance = (char *) NULL;
  
  ufree(index->server_subtype);
  index->server_subtype = (char *) NULL;
  
  ufree(index->servmap_host1);
  index->servmap_host1 = (char *) NULL;
  
  ufree(index->servmap_host2);
  index->servmap_host2 = (char *) NULL;
  
  ufree(index->server_info);
  index->server_info = (SERVMAP_info_t *) NULL;
  
}

/***********************************************************************
 *
 * tserver_free_read_buffer()
 *
 * free up memory associated with read buffer
 */

void tserver_free_read_buffer(tdata_index_t *index)
     
{
  
  ufree(index->read_buffer);
  index->read_buffer = (char *) NULL;
  index->nbytes_buffer = 0;
  index->nbytes_data = 0;
  index->read_posn = 0;
  
}

/***********************************************************************
 *
 * tserver_free_track_data()
 *
 * free up memory associated with track data
 */

void tserver_free_track_data(tdata_index_t *index)
     
{

  BLOCKfree(&index->bbuf);
  
}

/*************************************************************************
 * tserver_handle_disconnect()
 *
 * cleans up when the track server disconnects abnormally
 *
 *************************************************************************/

void tserver_handle_disconnect(tdata_index_t *index)
     
{
  
  SKU_close(index->tserver_fd);
  index->connected_to_server = FALSE;
  
}

/*****************************************************************
 * tserver_init()
 *
 * initialize tserver index
 *
 *   prog_name : program name - used for error messages
 *
 *   servmap_host1, servmap_host2 : server mapper hosts
 *
 *   server_subtype, server_instance : server details
 *
 *   default_host, default_port : defaults for server location to be
 *   used if server mapper fails
 *
 *   request_notify : set TRUE if notification for new realtime data
 *   is required
 *
 *   runs_included: should runs be included in the basic data
 *     TDATA_TRUE or TDATA_FALSE
 *
 *   max_message_len : max size of data buffer sent by server - should
 *     be 8K or greater. Larger buffers may increase speed, but
 *     this memory is allocated and hangs around.
 *
 *   messages_flag - if set, warnings and failure messages will be
 *     printed
 *
 *   debug_flag - if set, debug messages will be printed
 */

void tserver_init(const char *prog_name,
		  const char *servmap_host1,
		  const char *servmap_host2,
		  const char *server_subtype,
		  const char *server_instance,
		  const char *default_host,
		  int default_port,
		  int request_notify_flag,
		  int runs_included_flag,
		  int messages_flag,
		  int debug_flag,
		  si32 max_message_len,
		  tdata_index_t *index)
     
{
  
  memset((void *) index, (int) 0, (size_t) sizeof(tdata_index_t));
  
  load_string(prog_name, &index->prog_name);
  load_string(servmap_host1, &index->servmap_host1);
  load_string(servmap_host2, &index->servmap_host2);
  load_string(server_subtype, &index->server_subtype);
  load_string(server_instance, &index->server_instance);

  if (!strcmp(default_host, "local"))
    default_host = PORThostname();

  load_string(default_host, &index->default_host);
  
  index->default_port = default_port;
  index->request_notify = request_notify_flag;
  index->runs_included = runs_included_flag;
  index->max_message_len = max_message_len;

  index->messages_flag = messages_flag;
  index->debug_flag = debug_flag;
  
  index->current_port = -1;
  index->request_num = -1;

  /*
   * allocate block buffer
   */

  BLOCKinit(&index->bbuf, NBYTES_BUF_INIT);

}

/*************************************************************************
 * tserver_read()
 *
 * read track data from track server
 *
 * Inputs:
 *
 *   mode : TDATA_COMPLETE - complete structures
 *          TDATA_BASIC_WITH_PARAMS - basic structs along with the
 *            complex and simple track params
 *          TDATA_BASIC_WITHOUT_PARAMS - basic structs with header
 *            only - no complex and simple track params
 *
 *   source : TDATA_REALTIME or TDATA_ARCHIVE
 *
 *   track_set : TDATA_ALL_AT_TIME or TDATA_ALL_IN_FILE
 *
 *   target_entries: TDATA_ALL_ENTRIES or
 *                   TDATA_CURRENT_ENTRIES_ONLY or
 *                   TDATA_ENTRIES_IN_TIME_WINDOW
 *
 *   time_requested (Unix time, secs since Jan 1 1970)
 *     only for mode == TDATA_ARCHIVE
 *
 *   time_margin (secs) - search margin around time_requested
 *     only for mode == TDATA_ARCHIVE
 *              target_entries == TDATA_ALL_ENTRIES
 *
 *   duration_before_request: (secs)
 *   duration_after_request : (secs) - window around time_requested
 *     only for mode == TDATA_ARCHIVE
 *              target_entries == TDATA_ENTRIES_IN_TIME_WINDOW
 *
 *   index : the index structure
 *
 * returns 0 on success, -1 on failure
 *
 *************************************************************************/

int tserver_read(int mode,
		 int source,
		 int track_set,
		 int target_entries,
		 si32 time_requested,
		 si32 time_margin,
		 si32 duration_before_request,
		 si32 duration_after_request,
		 tdata_index_t *index)
     
{
  
  /*
   * set index values
   */
  
  index->mode = mode;
  index->source = source;
  index->track_set = track_set;
  index->target_entries = target_entries;
  index->time_requested = time_requested;
  index->time_returned = time_requested;
  index->time_margin = time_margin;
  index->duration_before_request = duration_before_request;
  index->duration_after_request = duration_after_request;
  
  /*
   * set sockutil headers to new in case they were set to old by
   * the server mapper
   */
  
  SKU_set_headers_to_new();
  
  /*
   * get the data
   */
  
  if (request_data(index, TDATA_REQUEST_DATA)) {
    
    /*
     * failure
     */
    
    return (-1);
    
  } else {
    
    /*
     * success
     */
    
    return (0);
    
  }
  
}

/*************************************************************************
 * tserver_read_next_scan()
 *
 * read track data for next scan from track server. For details
 * see tserver_read()
 *
 * returns 0 on success, -1 on failure
 *
 *************************************************************************/

int tserver_read_next_scan(int mode,
			   int source,
			   int track_set,
			   int target_entries,
			   si32 time_requested,
			   si32 time_margin,
			   si32 duration_before_request,
			   si32 duration_after_request,
			   tdata_index_t *index)
     
{
  
  /*
   * set index values
   */
  
  index->mode = mode;
  index->source = source;
  index->track_set = track_set;
  index->target_entries = target_entries;
  index->time_requested = time_requested;
  index->time_returned = time_requested;
  index->time_margin = time_margin;
  index->duration_before_request = duration_before_request;
  index->duration_after_request = duration_after_request;
  
  /*
   * set sockutil headers to new in case they were set to old by
   * the server mapper
   */
  
  SKU_set_headers_to_new();
  
  /*
   * get the data
   */
  
  if (request_data(index, TDATA_REQUEST_DATA_NEXT_SCAN)) {
    
    /*
     * failure
     */
    
    return (-1);
    
  } else {
    
    /*
     * success
     */
    
    return (0);
    
  }
  
}

/*************************************************************************
 * tserver_read_prev_scan()
 *
 * read track data for previous scan from track server. For details
 * see tserver_read()
 *
 * returns 0 on success, -1 on failure
 *
 *************************************************************************/

int tserver_read_prev_scan(int mode,
			   int source,
			   int track_set,
			   int target_entries,
			   si32 time_requested,
			   si32 time_margin,
			   si32 duration_before_request,
			   si32 duration_after_request,
			   tdata_index_t *index)
     
{
  
  /*
   * set index values
   */
  
  index->mode = mode;
  index->source = source;
  index->track_set = track_set;
  index->target_entries = target_entries;
  index->time_requested = time_requested;
  index->time_returned = time_requested;
  index->time_margin = time_margin;
  index->duration_before_request = duration_before_request;
  index->duration_after_request = duration_after_request;
  
  /*
   * set sockutil headers to new in case they were set to old by
   * the server mapper
   */
  
  SKU_set_headers_to_new();
  
  /*
   * get the data
   */
  
  if (request_data(index, TDATA_REQUEST_DATA_PREV_SCAN)) {
    
    /*
     * failure
     */
    
    return (-1);
    
  } else {
    
    /*
     * success
     */
    
    return (0);
    
  }
  
}

/*************************************************************************
 * tserver_read_first_before()
 *
 * read first data from track server before time_requested, back to
 * time_margin before that.
 *
 * Inputs:
 *
 *   mode : TDATA_COMPLETE - complete structures
 *          TDATA_BASIC_WITH_PARAMS - basic structs along with the
 *            complex and simple track params
 *          TDATA_BASIC_WITHOUT_PARAMS - basic structs with header
 *            only - no complex and simple track params
 *
 *   source : TDATA_REALTIME or TDATA_ARCHIVE
 *
 *   track_set : TDATA_ALL_AT_TIME or TDATA_ALL_IN_FILE
 *
 *   target_entries: TDATA_ALL_ENTRIES or
 *                   TDATA_CURRENT_ENTRIES_ONLY or
 *                   TDATA_ENTRIES_IN_TIME_WINDOW
 *
 *   time_requested (Unix time, secs since Jan 1 1970)
 *     only for mode == TDATA_ARCHIVE
 *
 *   time_margin (secs) - search margin before time_requested
 *     only for mode == TDATA_ARCHIVE
 *              target_entries == TDATA_ALL_ENTRIES
 *
 *   duration_before_request: (secs)
 *   duration_after_request : (secs) - window around time_requested
 *     only for mode == TDATA_ARCHIVE
 *              target_entries == TDATA_ENTRIES_IN_TIME_WINDOW
 *
 *   index : the index structure
 *
 * returns 0 on success, -1 on failure
 *
 *************************************************************************/

int tserver_read_first_before(int mode,
			      int source,
			      int track_set,
			      int target_entries,
			      si32 time_requested,
			      si32 time_margin,
			      si32 duration_before_request,
			      si32 duration_after_request,
			      tdata_index_t *index)
     
{
  
  /*
   * set index values
   */
  
  index->mode = mode;
  index->source = source;
  index->track_set = track_set;
  index->target_entries = target_entries;
  index->time_requested = time_requested;
  index->time_returned = time_requested;
  index->time_margin = time_margin;
  index->duration_before_request = duration_before_request;
  index->duration_after_request = duration_after_request;
  
  /*
   * set sockutil headers to new in case they were set to old by
   * the server mapper
   */
  
  SKU_set_headers_to_new();
  
  /*
   * get the data
   */
  
  if (request_data(index, TDATA_REQUEST_DATA_BEFORE)) {
    
    /*
     * failure
     */
    
    return (-1);
    
  } else {
    
    /*
     * success
     */
    
    return (0);
    
  }
  
}

/*************************
 * static module routines
 */

/*************************************************************************
 * connect_to_server()
 *
 * Connects to track server as necessary. First checks to see if the port
 * or hostname has changed. If so, hangs up from current server and
 * makes the connection to the new one.
 *
 * Returns 0 on success, -1 on failure.
 *
 *************************************************************************/

static int connect_to_server(char *host,
			     int port,
			     tdata_index_t *index)
     
{
  
  /*
   * if host or port has changed, hangup server
   */
  
  if (index->connected_to_server)
    if (port != index->current_port || strcmp(host, index->current_host))
      hangup(index);
  
  /*
   * connect to server if required
   */
  
  if (!index->connected_to_server) {
    
    if((index->tserver_fd = SKU_open_client(host, port)) < 0) {
      if (index->messages_flag) {
	fprintf(stderr,"WARNING - %s:tserver:connect_to_server\n",
		index->prog_name);
	fprintf(stderr,"Could not connect to server\n");
	fprintf(stderr, "Host '%s', port %d\n", host, port);
      }
      return(SERVER_ERROR);
    }
    
    index->connected_to_server = TRUE;
    
    /*
     * verify the connection
     */
    
    if(verify_connect(index))
      return (SERVER_ERROR);
    
    index->current_port = port;
    strncpy(index->current_host, host, TDATA_HOST_LEN);
    
    /*
     * set the max message len
     */
    
    set_max_message_len(index);
    
    /*
     * in realtime mode and time hist will not be active,
     * request notification of new data. If time_hist is active,
     * it will check for new data
     */
    
    if (index->request_notify)
      if(request_notify(index))
	return (SERVER_ERROR);
    
  }
  
  return (0);
  
}

/*************************************************************************
 * find_a_server()
 *
 * looks through the list for next suitable server
 *
 *************************************************************************/

static int find_a_server(tdata_index_t *index)
     
{
  
  si32 i;
  
  for (i = 0; i < index->n_servers; i++) {
    
    if (index->server_info[i].user_data[FLAG_POS]) {
      
      if(!connect_to_server(index->server_info[i].host,
			    (int) index->server_info[i].port,
			    index)) {
	
	/*
	 * success - set availability flag to indicate that
	 * this server has already been tried
	 */
	
	index->server_info[i].user_data[FLAG_POS] = FALSE;
	return(0);
	
      } 
      
    } /* if (index->server_info[i].user_data[FLAG_POS]) */ 
    
  } /* i */
  
  /*
   * failure so far - try the default port
   */
  
  if(!connect_to_server(index->default_host,
			index->default_port, index)) {
    return (0);
  } else {
    return (SERVER_ERROR);
  }
  
}

/***********************************************************************
 *
 * get_data()
 *
 */

static int get_data(tdata_index_t *index, si32 command)
     
{
  
  tdata_request_t request;
  tdata_reply_t reply;
  
  if (!index->connected_to_server) {
    return (SERVER_ERROR);
  }
  
  request.request = BE_from_si32(command);
  request.mode = BE_from_si32(index->mode);
  request.source = BE_from_si32(index->source);
  request.track_set = BE_from_si32(index->track_set);
  request.runs_included = BE_from_si32(index->runs_included);
  request.time = BE_from_si32(index->time_requested);
  request.time_margin = BE_from_si32(index->time_margin);
  request.target_entries = index->target_entries;
  request.duration_before_request = index->duration_before_request;
  request.duration_after_request = index->duration_after_request;
  
  if (request.mode == TDATA_BASIC_WITHOUT_PARAMS) {
    request.target_entries = TDATA_CURRENT_ENTRIES_ONLY;
    request.runs_included = FALSE;
  }
  
  if (request.mode == TDATA_COMPLETE) {
    request.runs_included = FALSE;
  }
  
  index->request_num++;
  
  if (SKU_writeh(index->tserver_fd, (char *) &request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 index->request_num) < 0) {
    
    fprintf(stderr, "ERROR - %s:tserver:get_data\n",
	    index->prog_name);
    fprintf(stderr, "Could not request_data.\n");

    return (SERVER_ERROR);
    
  }
  
  if (read_reply(&reply, index)) {
    
    fprintf(stderr, "ERROR - %s:tserver:get_data\n",
	    index->prog_name);
    fprintf(stderr, "Reading request_data reply.\n");

    return (SERVER_ERROR);
    
  }
  
  if (reply.status == TDATA_FAILURE) {
    
    if (index->messages_flag) {
      fprintf(stderr, "FAILURE - %s:tserver:get_data\n",
	      index->prog_name);
      fprintf(stderr, "%s\n", reply.text);
    }
    return (REQUEST_FAILURE);
    
  }
  
  switch (index->mode) {
    
  case TDATA_COMPLETE:
    
    if (read_complete(index))
      return (SERVER_ERROR);
    
    break;
    
  case TDATA_BASIC_WITH_PARAMS:
    
    if (read_basic_with_params(index))
      return (SERVER_ERROR);
    
    break;
    
  case TDATA_BASIC_WITHOUT_PARAMS:
    
    if (read_basic_without_params(index))
      return (SERVER_ERROR);
    
    break;
    
  } /* s witch */
  
  
  return (0);
  
}

/***********************************************************************
 *
 * hangup()
 *
 */

static void hangup(tdata_index_t *index)
     
{
  
  tdata_request_t request;
  tdata_reply_t reply;
  
  if (!index->connected_to_server)
    return;

  memset(&request, 0, sizeof(request));
  request.request = BE_from_si32(TDATA_HANGUP);
  
  index->request_num++;
  
  if (SKU_writeh(index->tserver_fd, (char *) &request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 index->request_num) < 0) {
    
    fprintf(stderr, "ERROR - %s:tserver:hangup\n",
	    index->prog_name);
    fprintf(stderr, "Trying to hangup.\n");
    
  }
  
  if (read_reply(&reply, index)) {
    
    fprintf(stderr, "ERROR - %s:tserver:hangup\n",
	    index->prog_name);
    fprintf(stderr, "Reading hangup reply.\n");
    
  }
  
  if (reply.status == TDATA_FAILURE) {
    
    if (index->messages_flag) {
      fprintf(stderr, "FAILURE - %s:tserver:hangup\n",
	      index->prog_name);
      fprintf(stderr, "%s\n", reply.text);
    }
    
  }
  
  SKU_close(index->tserver_fd);
  index->connected_to_server = FALSE;
  
}

/********************************************************************
 * load_string()
 *
 * Allocates mem for a string and copies it in
 */

static void load_string(const char *source, char **target)
     
{
  
  if (*target == (char *) NULL) {
    *target = (char *) umalloc((strlen(source) + 1));
  } else {
    *target = (char *) urealloc((char *) *target,
				(strlen(source) + 1));
  }
  
  strcpy(*target, source);
  
}

/*************************************************************************
 * query_server_mapper()
 *
 * get list of available servers
 *
 *************************************************************************/

static int query_server_mapper(tdata_index_t *index)
     
{
  
  static int first_call = TRUE;
  
  si32 i;
  int n_servers;
  
  SERVMAP_request_t request;
  SERVMAP_info_t *info;
  
  if (first_call) {
    
    /*
     * allocate space for server info
     */
    
    index->server_info = (SERVMAP_info_t *) umalloc
      (sizeof(SERVMAP_info_t));
    
    first_call = FALSE;
    
  }
  
  /*
   * load up request struct
   */
  
  memset ((void *)  &request,
	  (int) 0, (size_t) sizeof(SERVMAP_request_t));
  
  strcpy(request.server_type, SERVMAP_TYPE_STORM_TRACK);
  ustrncpy(request.server_subtype, index->server_subtype,
	   SERVMAP_NAME_MAX);
  ustrncpy(request.instance, index->server_instance,
	   SERVMAP_INSTANCE_MAX);
  
  if (index->source == TDATA_REALTIME)
    request.want_realtime = TRUE;
  else
    request.want_realtime = FALSE;
  
  request.time = index->time_requested;
  
  if (SMU_requestInfo(&request, &n_servers, &info,
		      index->servmap_host1,
		      index->servmap_host2) != 1) {
    SKU_set_headers_to_new();
    return (SERVER_ERROR);
  }
  index->n_servers = n_servers;
  
  /*
   * set sockutil headers to new in case they were set to old by
   *  the server mapper
   */
  
  SKU_set_headers_to_new();
  
  /*
   * realloc space for info, and copy info into it
   */
  
  index->server_info = (SERVMAP_info_t *) urealloc
    ((char *) index->server_info,
     (index->n_servers * sizeof(SERVMAP_info_t)));
  
  memcpy ((void *) index->server_info,
	  (void *) info,
	  (size_t) (index->n_servers * sizeof(SERVMAP_info_t)));
  
  /*
   * set the flag values to show server info has not yet been used
   */
  
  for (i = 0; i < index->n_servers; i++)
    index->server_info[i].user_data[FLAG_POS] = TRUE;
  
  if (index->n_servers < 1) {
    return (SERVER_ERROR);
  } else {
    return (0);
  }
  
}

/***********************************************************************
 *
 * read_basic_with_params()
 *
 * Read basic track data set with parameters
 *
 ****************************************************************************/

static int read_basic_with_params(tdata_index_t *index)
     
{
  
  int runs_included;
  
  si32 icomplex, isimple, ientry;
  si32 nbytes_runs;
  
  tdata_basic_header_t *header;
  tdata_basic_complex_params_t *complex_params;
  tdata_basic_simple_params_t *simple_params;
  tdata_basic_track_entry_t *track_entry;
  storm_file_run_t *runs;
  tdata_basic_with_params_index_t *b_index;
  
  b_index = &index->basic_p;
  
  /*
   * reset buffer memory
   */
  
  BLOCKreset(&index->bbuf);
  
  /*
   * main header
   */
  
  header = &b_index->header;
  
  if (read_from_buffer((char *) header,
		       (si32) sizeof(tdata_basic_header_t),
		       TDATA_BASIC_HEADER_ID,
		       index)) {
    
    fprintf(stderr, "ERROR - %s:tserver:read_basic_with_params\n",
	    index->prog_name);
    fprintf(stderr, "Packet out of order.\n");
    fprintf(stderr, "Expecting basic header packet.\n");
    hangup(index);
    return (SERVER_ERROR);
    
  }
  
  BE_to_array_32((ui32 *) header,
		 sizeof(tdata_basic_header_t));
  
  runs_included = header->runs_included;
  index->time_returned = header->time;

  /*
   * allocate memory
   */
  
  b_index->complex_params = (tdata_basic_complex_params_t *)
    BLOCKmalloc(&index->bbuf,
		(header->n_complex_tracks *
		 sizeof(tdata_basic_complex_params_t)));
  
  b_index->simple_params = (tdata_basic_simple_params_t **)
    BLOCKmalloc(&index->bbuf,
		(header->n_complex_tracks *
		 sizeof(tdata_basic_simple_params_t *)));
  
  b_index->track_entry = (tdata_basic_track_entry_t ***)
    BLOCKmalloc(&index->bbuf,
		(header->n_complex_tracks *
		 sizeof(tdata_basic_track_entry_t **)));
  
  b_index->simples_per_complex = (si32 **)
    BLOCKmalloc(&index->bbuf,
		header->n_complex_tracks * sizeof(si32 *));
  
  if (runs_included) {
    b_index->storm_runs = (storm_file_run_t ****)
      BLOCKmalloc(&index->bbuf,
		  (header->n_complex_tracks *
		   sizeof(storm_file_run_t ***)));
  }
  
  /*
   * complex tracks
   */
  
  for (icomplex = 0;
       icomplex < header->n_complex_tracks; icomplex++) {
    
    complex_params = b_index->complex_params + icomplex;
    
    if (read_from_buffer((char *) complex_params,
			 (si32) sizeof(tdata_basic_complex_params_t),
			 TDATA_BASIC_COMPLEX_PARAMS_ID,
			 index)) {
      
      fprintf(stderr, "ERROR - %s:tserver:read_basic_with_params\n",
	      index->prog_name);
      fprintf(stderr, "Packet out of order.\n");
      fprintf(stderr, "Expecting basic complex track params packet.\n");
      hangup(index);
      return (SERVER_ERROR);
      
    }
    
    BE_to_array_32((ui32 *) complex_params,
		   sizeof(tdata_basic_complex_params_t));
    
    /*
     * allocate memory
     */
    
    b_index->simples_per_complex[icomplex] = (si32 *)
      BLOCKmalloc(&index->bbuf,
		  complex_params->n_simple_tracks * sizeof(si32));
    
    b_index->simple_params[icomplex] = (tdata_basic_simple_params_t *)
      BLOCKmalloc(&index->bbuf,
		  (complex_params->n_simple_tracks *
		   sizeof(tdata_basic_simple_params_t)));
    
    b_index->track_entry[icomplex] = (tdata_basic_track_entry_t **)
      BLOCKmalloc(&index->bbuf,
		  (complex_params->n_simple_tracks *
		   sizeof(tdata_basic_track_entry_t *)));
    
    if (runs_included) {
      b_index->storm_runs[icomplex] = (storm_file_run_t ***)
	BLOCKmalloc(&index->bbuf,
		    (complex_params->n_simple_tracks *
		     sizeof(storm_file_run_t **)));
    }
      
    /*
     * read simples_per_complex
     */

    if (read_from_buffer((char *) b_index->simples_per_complex[icomplex],
			 complex_params->n_simple_tracks * sizeof(si32),
			 TDATA_BASIC_SIMPLES_PER_COMPLEX_ID,
			 index)) {
      
      fprintf(stderr, "ERROR - %s:tserver:read_basic_with_params\n",
	      index->prog_name);
      fprintf(stderr, "Packet out of order.\n");
      fprintf(stderr, "Expecting basic simples_per_complex packet.\n");
      hangup(index);
      return (SERVER_ERROR);
      
    }
    
    BE_to_array_32((ui32 *) b_index->simples_per_complex[icomplex],
		   complex_params->n_simple_tracks * sizeof(si32));
    
    /*
     * simple tracks
     */
    
    for (isimple = 0;
	 isimple < complex_params->n_simple_tracks; isimple++) {
      
      simple_params = b_index->simple_params[icomplex] + isimple;
      
      if (read_from_buffer((char *) simple_params,
			   sizeof(tdata_basic_simple_params_t),
			   TDATA_BASIC_SIMPLE_PARAMS_ID,
			   index)) {
	
	fprintf(stderr, "ERROR - %s:tserver:read_basic_with_params\n",
		index->prog_name);
	fprintf(stderr, "Packet out of order.\n");
	fprintf(stderr, "Expecting basic simple params packet.\n");
	hangup(index);
	return (SERVER_ERROR);
	
      }
      
      BE_to_array_32((ui32 *) simple_params,
		     sizeof(tdata_basic_simple_params_t));
      
      /*
       * allocate memory
       */
      
      b_index->track_entry[icomplex][isimple] =
	(tdata_basic_track_entry_t *)
	BLOCKmalloc(&index->bbuf,
		    (simple_params->duration_in_scans *
		     sizeof(tdata_basic_track_entry_t)));
      
      if (runs_included) {
	b_index->storm_runs[icomplex][isimple] = (storm_file_run_t **)
	  BLOCKmalloc(&index->bbuf,
		      (simple_params->duration_in_scans *
		       sizeof(storm_file_run_t *)));
      }
      
      /*
       * track file entries
       */
      
      for (ientry = 0;
	   ientry < simple_params->duration_in_scans; ientry++) {
	
	track_entry = b_index->track_entry[icomplex][isimple] + ientry;
	
	if (read_from_buffer((char *) track_entry,
			     (si32) sizeof(tdata_basic_track_entry_t),
			     TDATA_BASIC_TRACK_ENTRY_ID,
			     index)) {
	  
	  fprintf(stderr, "ERROR - %s:tserver:read_basic_with_params\n",
		  index->prog_name);
	  fprintf(stderr, "Packet out of order.\n");
	  fprintf(stderr, "Expecting basic track entry packet.\n");
	  hangup(index);
	  return (SERVER_ERROR);
	  
	}
	
	BE_to_array_32((ui32 *) track_entry,
		       sizeof(tdata_basic_track_entry_t));
	
	if (runs_included) {
	  
	  /*
	   * allocate memory for storm runs
	   */
	  
	  nbytes_runs = track_entry->n_runs * sizeof(storm_file_run_t);
	  
	  runs = (storm_file_run_t *) BLOCKmalloc(&index->bbuf, nbytes_runs);
	  b_index->storm_runs[icomplex][isimple][ientry] = runs;
	  
	  if (read_from_buffer((char *) runs,
			       (si32) nbytes_runs,
			       TDATA_RUNS_ID,
			       index)) {
	    
	    fprintf(stderr, "ERROR - %s:tserver:read_basic_with_params\n",
		    index->prog_name);
	    fprintf(stderr, "Packet out of order.\n");
	    fprintf(stderr, "Expecting basic track runs packet.\n");
	    hangup(index);
	    return (SERVER_ERROR);
	    
	  }
	  
	  BE_to_array_16((ui16 *) runs, nbytes_runs);
	  
	} /* if (runs_included) */
	
      } /* ientry */
      
    } /* isimple */
    
  } /* icomplex */

  return (0);
  
}

/***********************************************************************
 *
 * read_basic_without_params()
 *
 * Read basic track data set without parameters
 *
 ****************************************************************************/

static int read_basic_without_params(tdata_index_t *index)
     
{
  
  si32 ientry;
  tdata_basic_header_t *header;
  tdata_basic_track_entry_t *track_entry;
  tdata_basic_without_params_index_t *b_index;
  
  b_index = &index->basic;
  
  /*
   * reset buffer
   */
  
  BLOCKreset(&index->bbuf);
  
  /*
   * main header
   */
  
  header = &b_index->header;
  
  if (read_from_buffer((char *) header,
		       (si32) sizeof(tdata_basic_header_t),
		       TDATA_BASIC_HEADER_ID,
		       index)) {
    
    fprintf(stderr, "ERROR - %s:tserver:read_basic_without_params\n",
	    index->prog_name);
    fprintf(stderr, "Packet out of order.\n");
    fprintf(stderr, "Expecting basic header packet.\n");
    hangup(index);
    return (SERVER_ERROR);
    
  }
  
  BE_to_array_32((ui32 *) header,
		 sizeof(tdata_basic_header_t));
  
  index->time_returned = header->time;

  /*
   * allocate memory
   */
  
  b_index->track_entry = (tdata_basic_track_entry_t *)
    BLOCKmalloc(&index->bbuf,
		(header->n_entries *
		 sizeof(tdata_basic_track_entry_t)));
  
  /*
   * entries
   */
  
  for (ientry = 0; ientry < header->n_entries; ientry++) {
    
    track_entry = b_index->track_entry + ientry;
    
    if (read_from_buffer
	((char *) track_entry,
	 (si32) sizeof(tdata_basic_track_entry_t),
	 TDATA_BASIC_TRACK_ENTRY_ID,
	 index)) {
      
      fprintf(stderr, "ERROR - %s:tserver:read_basic_without_params\n",
	      index->prog_name);
      fprintf(stderr, "Packet out of order.\n");
      fprintf(stderr, "Expecting basic track entry packet.\n");
      hangup(index);
      return (SERVER_ERROR);
      
    }
    
    BE_to_array_32((ui32 *) track_entry,
		   sizeof(tdata_basic_track_entry_t));
    
  } /* ientry */
  
  return (0);
  
}

/***********************************************************************
 *
 * read_complete()
 *
 * Read complete track data set
 *
 ****************************************************************************/

static int read_complete(tdata_index_t *index)
     
{
  
  si32 icomplex, isimple;
  si32 ientry, ilayer, interval;
  si32 nbytes_char;
  
  tdata_complete_index_t *c_index;
  tdata_complete_header_t *header;
  complex_track_params_t *complex_params;
  simple_track_params_t *simple_params;
  tdata_complete_track_entry_t *track_entry;
  track_file_entry_t *entry;
  storm_file_scan_header_t *scan;
  storm_file_global_props_t *gprops;
  storm_file_layer_props_t *lprops;
  storm_file_dbz_hist_t *hist;
  
  c_index = &index->complete;
  
  /*
   * reset buffer
   */
  
  BLOCKreset(&index->bbuf);
  
  /*
   * main header
   */
  
  header = &c_index->header;
  
  if (read_from_buffer((char *) header,
		       (si32) sizeof(tdata_complete_header_t),
		       TDATA_COMPLETE_HEADER_ID,
		       index)) {
    
    fprintf(stderr, "ERROR - %s:tserver:read_complete_tserver_data\n",
	    index->prog_name);
    fprintf(stderr, "Packet out of order.\n");
    fprintf(stderr, "Expecting complete header packet.\n");
    hangup(index);
    return (SERVER_ERROR);
    
  }
  
  BE_to_array_32((ui32 *) header,
		 sizeof(tdata_complete_header_t));
  
  index->time_returned = header->time;

  /*
   * allocate memory
   */
  
  c_index->complex_params = (complex_track_params_t *)
    BLOCKmalloc(&index->bbuf, (header->n_complex_tracks *
			       sizeof(complex_track_params_t)));
  
  c_index->simple_params = (simple_track_params_t **)
    BLOCKmalloc(&index->bbuf, (header->n_complex_tracks *
			       sizeof(simple_track_params_t *)));
  
  c_index->track_entry = (tdata_complete_track_entry_t ***)
    BLOCKmalloc(&index->bbuf, (header->n_complex_tracks *
			       sizeof(tdata_complete_track_entry_t **)));
  
  c_index->simples_per_complex = (si32 **)
    BLOCKmalloc(&index->bbuf, header->n_complex_tracks * sizeof(si32 *));
  
  /*
   * complex tracks
   */
  
  for (icomplex = 0;
       icomplex < header->n_complex_tracks; icomplex++) {
    
    complex_params = c_index->complex_params + icomplex;
    
    if (read_from_buffer((char *) complex_params,
			 (si32) sizeof(complex_track_params_t),
			 TDATA_COMPLEX_TRACK_PARAMS_ID,
			 index)) {
      
      fprintf(stderr, "ERROR - %s:tserver:read_complete\n",
	      index->prog_name);
      fprintf(stderr, "Packet out of order.\n");
      fprintf(stderr, "Expecting complex track params packet.\n");
      hangup(index);
      return (SERVER_ERROR);
      
    }
    
    BE_to_array_32((ui32 *) complex_params,
		   sizeof(complex_track_params_t));
    
    /*
     * allocate memory
     */
    
    c_index->simples_per_complex[icomplex] = (si32 *)
      BLOCKmalloc(&index->bbuf,
		  complex_params->n_simple_tracks * sizeof(si32));
    
    c_index->simple_params[icomplex] = (simple_track_params_t *)
      BLOCKmalloc(&index->bbuf,
		  (complex_params->n_simple_tracks *
		   sizeof(simple_track_params_t)));
    
    c_index->track_entry[icomplex] = (tdata_complete_track_entry_t **)
      BLOCKmalloc(&index->bbuf,
		  (complex_params->n_simple_tracks *
		   sizeof(tdata_complete_track_entry_t *)));
    
    /*
     * read simples_per_complex
     */

    if (read_from_buffer((char *) c_index->simples_per_complex[icomplex],
			 complex_params->n_simple_tracks * sizeof(si32),
			 TDATA_COMPLETE_SIMPLES_PER_COMPLEX_ID,
			 index)) {
      
      fprintf(stderr, "ERROR - %s:tserver:read_complete\n",
	      index->prog_name);
      fprintf(stderr, "Packet out of order.\n");
      fprintf(stderr, "Expecting complete simples_per_complex packet.\n");
      hangup(index);
      return (SERVER_ERROR);
      
    }
    
    BE_to_array_32((ui32 *) c_index->simples_per_complex[icomplex],
		   complex_params->n_simple_tracks * sizeof(si32));
    
    /*
     * simple tracks
     */
    
    for (isimple = 0;
	 isimple < complex_params->n_simple_tracks; isimple++) {
      
      simple_params = c_index->simple_params[icomplex] + isimple;
      
      if (read_from_buffer((char *) simple_params,
			   (si32) sizeof(simple_track_params_t),
			   TDATA_SIMPLE_TRACK_PARAMS_ID,
			   index)) {
	
	fprintf(stderr, "ERROR - %s:tserver:read_complete\n",
		index->prog_name);
	fprintf(stderr, "Packet out of order.\n");
	fprintf(stderr, "Expecting simple track params packet.\n");
	hangup(index);
	return (SERVER_ERROR);
	
      }
      
      BE_to_array_32((ui32 *) simple_params,
		     sizeof(simple_track_params_t));
      
      /*
       * allocate memory
       */
      
      c_index->track_entry[icomplex][isimple] =
	(tdata_complete_track_entry_t *)
	BLOCKmalloc(&index->bbuf,
		    (simple_params->duration_in_scans *
		     sizeof(tdata_complete_track_entry_t)));
      
      /*
       * track file entries
       */
      
      for (ientry = 0;
	   ientry < simple_params->duration_in_scans; ientry++) {
	
	track_entry = c_index->track_entry[icomplex][isimple] + ientry;
	
	entry = &track_entry->entry;
	
	if (read_from_buffer((char *) entry,
			     (si32) sizeof(track_file_entry_t),
			     TDATA_TRACK_FILE_ENTRY_ID,
			     index)) {
	  
	  fprintf(stderr, "ERROR - %s:tserver:read_complete\n",
		  index->prog_name);
	  fprintf(stderr, "Packet out of order.\n");
	  fprintf(stderr, "Expecting track file entry packet.\n");
	  hangup(index);
	  return (SERVER_ERROR);
	  
	}
	
	BE_to_array_32((ui32 *) entry,
		       sizeof(track_file_entry_t));
	
	/*
	 * scan header
	 */
	
	scan = &track_entry->scan;
	
	if (read_from_buffer((char *) scan,
			     (si32) sizeof(storm_file_scan_header_t),
			     TDATA_STORM_FILE_SCAN_HEADER_ID,
			     index)) {
	  
	  fprintf(stderr, "ERROR - %s:tserver:read_complete\n",
		  index->prog_name);
	  fprintf(stderr, "Packet out of order.\n");
	  fprintf(stderr, "Expecting scan header packet.\n");
	  hangup(index);
	  return (SERVER_ERROR);
	  
	}
	
	nbytes_char = BE_from_si32(scan->nbytes_char);
	
	BE_to_array_32((ui32 *) scan,
		       (sizeof(storm_file_scan_header_t) -
			nbytes_char));
	
	/*
	 * global props
	 */
	
	gprops = &track_entry->gprops;
	
	if (read_from_buffer((char *) gprops,
			     (si32) sizeof(storm_file_global_props_t),
			     TDATA_STORM_FILE_GLOBAL_PROPS_ID,
			     index)) {
	  
	  fprintf(stderr, "ERROR - %s:tserver:read_complete\n",
		  index->prog_name);
	  fprintf(stderr, "Packet out of order.\n");
	  fprintf(stderr, "Expecting global props packet.\n");
	  hangup(index);
	  return (SERVER_ERROR);
	  
	}
	
	BE_to_array_32((ui32 *) gprops,
		       (sizeof(storm_file_global_props_t)));
	
	/*
	 * allocate memory
	 */
	
	track_entry->lprops = (storm_file_layer_props_t *)
	  BLOCKmalloc(&index->bbuf,
		      (gprops->n_layers *
		       sizeof(storm_file_layer_props_t)));
	
	/*
	 * layer properties
	 */
	
	for (ilayer = 0; ilayer < gprops->n_layers; ilayer++) {
	  
	  lprops = track_entry->lprops + ilayer;
	  
	  if (read_from_buffer
	      ((char *) lprops,
	       (si32) sizeof(storm_file_layer_props_t),
	       TDATA_STORM_FILE_LAYER_PROPS_ID,
	       index)) {
	    
	    fprintf(stderr, "ERROR - %s:tserver:read_complete\n",
		    index->prog_name);
	    fprintf(stderr, "Packet out of order.\n");
	    fprintf(stderr, "Expecting layer props packet.\n");
	    hangup(index);
	    return (SERVER_ERROR);
	    
	  }
	  
	  BE_to_array_32((ui32 *) lprops,
			 (sizeof(storm_file_layer_props_t)));
	  
	} /* ilayer */
	
	/*
	 * allocate memory
	 */
	
	track_entry->hist = (storm_file_dbz_hist_t *)
	  BLOCKmalloc (&index->bbuf,
		       (gprops->n_dbz_intervals *
			sizeof(storm_file_dbz_hist_t)));
	
	
	/*
	 * dbz histogram data
	 */
	
	for (interval = 0;
	     interval < gprops->n_dbz_intervals; interval++) {
	  
	  hist = track_entry->hist + interval;
	  
	  if (read_from_buffer
	      ((char *) hist,
	       (si32) sizeof(storm_file_dbz_hist_t),
	       TDATA_STORM_FILE_DBZ_HIST_ID,
	       index)) {
	    
	    fprintf(stderr, "ERROR - %s:tserver:read_complete\n",
		    index->prog_name);
	    fprintf(stderr, "Packet out of order.\n");
	    fprintf(stderr, "Expecting dbz hist packet.\n");
	    hangup(index);
	    return (SERVER_ERROR);
	    
	  }
	  
	  BE_to_array_32((ui32 *) hist,
			 (sizeof(storm_file_dbz_hist_t)));
	  
	} /* interval */
	
      } /* ientry */
      
    } /* isimple */
    
  } /* icomplex */

  return (0);
  
}

/*********************************************************************
 * read_from_buffer()
 *
 * Reads data buffer, loading up the data as requested.
 * If necessary, reads a new packet
 *
 * returns 0 on success, -1 on failure
 *
 *********************************************************************/

static int read_from_buffer(char *data,
			    si32 nbytes,
			    int id,
			    tdata_index_t *index)
     
{
  
  si32 tot_bytes;
  si32 struct_id;
  SKU_header_t packet_header;
  tdata_struct_header_t struct_header;
  
  tot_bytes = nbytes + sizeof(tdata_struct_header_t);
  
  if (index->read_posn + tot_bytes  > index->nbytes_data) {
    
    /*
     * read a new packet into the buffer
     */
    
    if (SKU_read_header(index->tserver_fd, &packet_header, -1L) < 0)
      return (SERVER_ERROR);
    
    if (packet_header.len > index->nbytes_buffer) {
      
      if (index->read_buffer == NULL)
	index->read_buffer = (char *) umalloc (packet_header.len);
      else
	index->read_buffer =
	  (char *) urealloc (index->read_buffer, packet_header.len);
      
      index->nbytes_buffer = packet_header.len;
      
    } /* if (packet_header.len > index->nbytes_buffer) */
    
    if (SKU_read(index->tserver_fd, index->read_buffer,
		 packet_header.len, -1)
	!= packet_header.len) {
      return (SERVER_ERROR);
    }
    
    index->nbytes_data = packet_header.len;
    index->read_posn = 0;
    
  } /* if (index->read_posn + tot_bytes > index->nbytes_data) */
  
  /*
   * check struct id
   */
  
  memcpy ((void *) &struct_header,
	  (void *) (index->read_buffer + index->read_posn),
	  (size_t) sizeof(tdata_struct_header_t));
  
  struct_id = BE_to_si32(struct_header.id);
  
  if (struct_id != id) {
    
    fprintf(stderr, "ERROR - %s:tserver:read_from_buffer\n",
	    index->prog_name);
    fprintf(stderr, "Structure id incorrect.\n");
    fprintf(stderr, "Requested id %d, id received %d\n",
	    id, struct_id);
    
    return (-1);
    
  }
  
  /*
   * copy buffer to data
   */
  
  memcpy ((void *) data,
	  (void *) (index->read_buffer + index->read_posn +
		    sizeof(tdata_struct_header_t)),
	  (size_t) nbytes);
  
  index->read_posn += tot_bytes;
  
  return (0);
  
}

/*********************************************************************
 * read_notify.c
 *
 * Reads a notification packet from the client
 *
 * returns 0 on success, -1 on failure
 *
 *********************************************************************/

static int read_notify(tdata_index_t *index)
     
{
  
  SKU_header_t header;
  tdata_reply_t reply;
  
  /*
   * read in the reply
   */
  
  if (SKU_readh(index->tserver_fd, (char *) &reply,
		(si32) sizeof(tdata_reply_t), &header, -1L) < 0)
    return (SERVER_ERROR);
  
  /*
   * if this is a notification packet, return successful.
   * Otherwise, return failure.
   */
  
  if (header.id == TDATA_NOTIFY_PACKET_ID)
    return (0);
  else
    return (SERVER_ERROR);
  
}

/*********************************************************************
 * read_reply.c
 *
 * Reads a reply packet from the server
 *
 * returns 0 on success, -1 on failure
 *
 *********************************************************************/

static int read_reply(tdata_reply_t *reply,
		      tdata_index_t *index)
     
{
  
  SKU_header_t header;
  
  /*
   * read in the reply
   */
  
read_another:
  
  if (SKU_readh(index->tserver_fd, (char *) reply,
		(si32) sizeof(tdata_reply_t),
		&header, -1L) < 0)
    return (SERVER_ERROR);
  
  /*
   * if this is a notification packet, discard and read another
   */
  
  if (header.id == TDATA_NOTIFY_PACKET_ID)
    goto read_another;
  
  if (header.id != TDATA_REPLY_PACKET_ID) {
    
    fprintf(stderr, "ERROR - %s:tserver:read_reply\n",
	    index->prog_name);
    fprintf(stderr, "Packet out of order.\n");
    fprintf(stderr, "Expecting reply packet.\n");
    fprintf(stderr, "Packet header id %d\n", header.id);
    hangup(index);
    return (SERVER_ERROR);
    
  }
  
  /*
   * get bytes into host byte order
   */
  
  reply->status = BE_to_si32(reply->status);
  
  return (0);
  
}

/***********************************************************************
 *
 * request_data()
 *
 */

static int request_data(tdata_index_t *index, si32 command)
     
{
  
  int retval;
  
  /*
   * connect to the server as necessary
   */
  
  if (!index->connected_to_server) {
    
    index->n_servers = 0;
    query_server_mapper(index);
    
    if (find_a_server(index))
      return (SERVER_ERROR);
    
  }
  
  /*
   * initial try to get data
   */
  
  retval = get_data(index, command);
  
  if (retval == REQUEST_SUCCESS) {
    
    /*
     * success
     */
    
    return (0);
    
  } else if (retval == REQUEST_FAILURE) {
    
    /*
     * data not available from server
     */
    
    return (-1);
    
  } else if (retval == SERVER_ERROR) {
    
    /*
     * access failed, so try another server
     */
    
    if (find_a_server(index)) {
      
      return (SERVER_ERROR);
      
    } else {
      
      if (!get_data(index, command))
	return (0);
      else
	return (SERVER_ERROR);
      
    } /* if */
    
  } else {
    
    return (-1);
    
  } /* if (!get_data ... */
  
}

/***********************************************************************
 *
 * request_notify()
 *
 */

static int request_notify(tdata_index_t *index)
     
{
  
  tdata_request_t request;
  tdata_reply_t reply;
  
  if (!index->connected_to_server)
    return (SERVER_ERROR);
  
  request.request = BE_from_si32(TDATA_REQUEST_NOTIFY);
  
  index->request_num++;
  
  if (SKU_writeh(index->tserver_fd, (char *) &request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 index->request_num) < 0) {
    
    fprintf(stderr, "ERROR - %s:tserver:request_notify\n",
	    index->prog_name);
    fprintf(stderr, "Could not request_notify.\n");
    return (SERVER_ERROR);
    
  }
  
  if (read_reply(&reply, index)) {
    
    fprintf(stderr, "ERROR - %s:tserver:request_notify\n",
	    index->prog_name);
    fprintf(stderr, "Reading request_notify reply.\n");
    return (SERVER_ERROR);
    
  }
  
  if (reply.status == TDATA_FAILURE) {
    
    if (index->messages_flag) {
      fprintf(stderr, "FAILURE - %s:tserver:request_notify\n",
	      index->prog_name);
      fprintf(stderr, "%s\n", reply.text);
    }
      
  }
  
  return (0);
  
}

/***********************************************************************
 *
 * set_max_message_len()
 *
 */

static int set_max_message_len(tdata_index_t *index)
     
{
  
  tdata_request_t request;
  tdata_reply_t reply;
  
  if (!index->connected_to_server)
    return (SERVER_ERROR);
  
  request.request = BE_from_si32(TDATA_SET_MAX_MESSAGE_LEN);
  request.max_message_len = BE_from_si32(index->max_message_len);
  
  index->request_num++;
  
  if (SKU_writeh(index->tserver_fd, (char *) &request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 index->request_num) < 0) {
    
    fprintf(stderr, "ERROR - %s:tserver:set_max_message_len\n",
	    index->prog_name);
    fprintf(stderr, "Could not set_max_message_len.\n");
    return (SERVER_ERROR);
    
  }
  
  if (read_reply(&reply, index)) {
    
    fprintf(stderr, "ERROR - %s:tserver:set_max_message_len\n",
	    index->prog_name);
    fprintf(stderr, "Reading set_max_message_len reply.\n");
    return (SERVER_ERROR);
    
  }
  
  if (reply.status == TDATA_FAILURE) {
    
    if (index->messages_flag) {
      fprintf(stderr, "FAILURE - %s:tserver:set_max_message_len\n",
	      index->prog_name);
      fprintf(stderr, "%s\n", reply.text);
    }
    
  }
  
  return (0);
  
}

#ifdef USE_STOP_NOTIFY

/***********************************************************************
 *
 * stop_notify()
 *
 */

static int stop_notify(tdata_index_t *index)
     
{
  
  tdata_request_t request;
  tdata_reply_t reply;
  
  if (!index->connected_to_server)
    return (SERVER_ERROR);
  
  request.request = BE_from_si32(TDATA_STOP_NOTIFY);
  
  index->request_num++;
  
  if (SKU_writeh(index->tserver_fd, (char *) &request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 index->request_num) < 0) {
    
    fprintf(stderr, "ERROR - %s:tserver_stop_notify\n",
	    index->prog_name);
    fprintf(stderr, "Could not stop_notify.\n");
    return (SERVER_ERROR);
    
  }
  
  if (read_reply(&reply, index)) {
    
    fprintf(stderr, "ERROR - %s:tserver_stop_notify\n",
	    index->prog_name);
    fprintf(stderr, "Reading stop_notify reply.\n");
    return (SERVER_ERROR);
    
  }
  
  if (reply.status == TDATA_FAILURE) {
    
    if (index->messages_flag) {
      fprintf(stderr, "FAILURE - %s:tserver_stop_notify\n",
	      index->prog_name);
      fprintf(stderr, "%s\n", reply.text);
    }
    
  }
  
  return (0);
  
}

#endif

/***********************************************************************
 *
 * verify_connect()
 *
 */

static int verify_connect(tdata_index_t *index)
     
{
  
  tdata_request_t request;
  tdata_reply_t reply;
  
  if (!index->connected_to_server)
    return (SERVER_ERROR);
  
  request.request = BE_from_si32(TDATA_VERIFY_CONNECT);
  
  index->request_num++;
  
  if (SKU_writeh(index->tserver_fd, (char *) &request,
		 (si32) sizeof(tdata_request_t),
		 (si32) TDATA_REQUEST_PACKET_ID,
		 index->request_num) < 0) {
    
    fprintf(stderr, "ERROR - %s:tserver:verify_connect\n",
	    index->prog_name);
    fprintf(stderr, "Could not verify_connect.\n");
    return (SERVER_ERROR);
    
  }
  
  if (read_reply(&reply, index)) {
    
    fprintf(stderr, "ERROR - %s:tserver:verify_connect\n",
	    index->prog_name);
    fprintf(stderr, "Reading verify_connect reply.\n");
    return (SERVER_ERROR);
    
  }
  
  if (reply.status == TDATA_FAILURE) {
    
    if (index->messages_flag) {
      fprintf(stderr, "FAILURE - %s:tserver:verify_connect\n",
	      index->prog_name);
      fprintf(stderr, "%s\n", reply.text);
    }
    
  }
  
  return (0);
  
}

 
