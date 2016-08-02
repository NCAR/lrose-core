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
/* smu : server mapper utilities
   
   Modified:
   02/01/93: JCaron automatically register with "local" server mapper, try
   to get info from "local" server mapper. This is so that demo
   systems can override data servers.
   03/25/93: SYSV port
   11/15/93: maximum 10 second wait for read/write to server mapper
   
   */

#include <toolsa/globals.h>
#include <toolsa/port.h>
#include <toolsa/servmap.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/smu.h>
#include <dataport/bigend.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h> /* sin_family sin_addr */

#define MAX_WAIT_MSEC 10000 /* 10 secs */

/*********************************************************
 * convert data structures from network to host byte order
 */

void SMU_ntohl_Info(SERVMAP_info_t *info)
{
  BE_to_array_32((ui32 *) info, SERVMAP_INFO_NBYTES_SWAP);
}

void SMU_ntohl_Request(SERVMAP_request_t *req)
{
  req->want_realtime = BE_to_si32(req->want_realtime);
  req->time = BE_to_si32(req->time);
}

void SMU_ntohl_Reply(SERVMAP_reply_t *reply)
{
  reply->return_code = BE_to_si32(reply->return_code);
  reply->n_servers = BE_to_si32(reply->n_servers);
}

/*********************************************************
 * convert data structures from host to network byte order
 */

void SMU_htonl_Info(SERVMAP_info_t *info)
{
  BE_from_array_32((ui32 *) info, SERVMAP_INFO_NBYTES_SWAP);
}

void SMU_htonl_Request(SERVMAP_request_t *req)
{
  req->want_realtime = BE_from_si32(req->want_realtime);
  req->time = BE_from_si32(req->time);
}

void SMU_htonl_Reply(SERVMAP_reply_t *reply)
{
  reply->return_code = BE_from_si32(reply->return_code);
  reply->n_servers = BE_from_si32(reply->n_servers);
}

/****************************************************
 *
 * get the server mapper hosts from the standard environment
 * variables
 */

void SMU_get_servmap_hosts(char **servmap_host1,
			   char **servmap_host2)
{
  *servmap_host1 = getenv("SERVMAP_HOST");
  *servmap_host2 = getenv("SERVMAP_HOST2");
}


/**************************
 * get hostname for server
 */

static char *Smu_hostname(char *hostname)
{

  static char local[SERVMAP_HOST_MAX];
  static int first = TRUE;

  if (first) {
    STRncopy(local, PORThostnameFull(), SERVMAP_HOST_MAX);
    first = FALSE;
  }
  
  if ((hostname == NULL) || (strlen(hostname) == 0)) {
    return local;
  } else if (STRequal("local", hostname)) {
    return local;
  } else if (STRequal("none", hostname) || STRequal("null", hostname)) {
    return NULL;
  } else {
    return hostname;
  }

}

/****************************************************
 *
 * communicate with server mapper in standard manner
 *
 * Returns 1 on sucess, 0 on failure.
 */

static int Communicate(int type, char *host,
		       void *messin,
		       long messin_len, 
		       char **messout,
		       long *messout_len)
{
  static char *mess = NULL;
  static int mess_len = 0;
  int cd, ret;
  SKU_header_t head;
 
  if (host == NULL || (strlen(host) == 0) ||
      (!strcmp(host, "null")) || (!strcmp(host, "none"))) {
    return FALSE;
  }
 
  cd = SKU_open_client (host, SERVMAP_DEFAULT_PORT); 
  if (cd < 0) {
    return FALSE;
  }
 
  if (1 != SKU_writeh(cd, messin, messin_len, type, -1)) {
    SKU_close(cd);
    return FALSE;
  }
 
  ret = SKU_read_header(cd, &head, MAX_WAIT_MSEC);
 
  if(ret != 1) {
    SKU_close(cd);
    return(FALSE);
  }
 
  /* make sure messout is big enough */
  if (mess == NULL)
    {
      mess = (char *) malloc(head.len);
      if(!mess) {
	SKU_close(cd);
	return(FALSE);
      }
 
      mess_len = head.len;
    }
  else if (head.len > mess_len)
    {
      char * newmess = (char *) realloc(mess, head.len);
      if(!newmess) {
	SKU_close(cd);
	return(FALSE);
      }
      mess = newmess;
      mess_len = head.len;
    }

  SKU_read(cd, mess, head.len, -1);
  SKU_close(cd);
 
  *messout_len = head.len;
  *messout = mess;
  return (ret == 1);
}

/**********************************************************************
 *
 * Communicate_both()
 *
 * Communicate with both hosts as appropriate
 *
 * Returns 1 on success, 0 on failure.
 */

static int
Communicate_both(int type,
		 SERVMAP_info_t *info,
		 char *servmap_host1, char *servmap_host2)
{

  int ret = FALSE;
  char *mess;
  long len;

  /*
   * register if primary host is non-null
   */
  
  if (Smu_hostname(servmap_host1) != NULL) {
    
    ret |= Communicate(type, Smu_hostname(servmap_host1),
		       info, sizeof(SERVMAP_info_t), &mess, &len);

    /*
     * register with second host if non-null and it is
     * different from primary
     */
    
    if (Smu_hostname(servmap_host2) != NULL &&
	strcmp(Smu_hostname(servmap_host1),
	       Smu_hostname(servmap_host2))) {
      
      ret |= Communicate(type, Smu_hostname(servmap_host2),
			 info, sizeof(SERVMAP_info_t), &mess, &len);

    }

  }

  return(ret);

}

/**********************************************************************
 *
 * Send server info to servers on servmap_host1 and servmap_host2
 * Note this routine will fill in info->host, and put info into network
 * byte order.
 * Special hostname handling: if "local", use local hostname; if
 * NULL, empty string, "null" or "none", then do not communicate.
 */

int
SMU_register(SERVMAP_info_t *info,
	     char *servmap_host1, char *servmap_host2)
{

  static int first = TRUE;
  static char host[SERVMAP_HOST_MAX];
  static char ipaddr[SERVMAP_HOST_MAX];
  static char user[SERVMAP_USER_MAX];

  SERVMAP_info_t info_copy = *info;

  if (first) {
    STRncopy(host, PORThostnameFull(), SERVMAP_HOST_MAX);
    STRncopy(ipaddr, PORThostIpAddr(), SERVMAP_HOST_MAX);
    STRncopy(user, getenv("USER"), SERVMAP_USER_MAX);
    first = FALSE;
  }

  STRncopy(info_copy.host, host, SERVMAP_HOST_MAX);
  STRncopy(info_copy.ipaddr, ipaddr, SERVMAP_HOST_MAX);
  STRncopy(info_copy.user, user, SERVMAP_USER_MAX);
  
  /*
   * byte swapping
   */
  
  SMU_htonl_Info(&info_copy);
    
  return (Communicate_both(SERVMAP_REGISTER_SERVER,
			   &info_copy,
			   servmap_host1, servmap_host2));

}

/***************************************************************
 *
 * SMU_reg_proxy is similar to SMU_register, except that the
 * host of the server is assumed to have been set in the
 * info structure. The proxy registration is intended for
 * a proxy process which registers on behalf of a server
 */

int
SMU_reg_proxy(SERVMAP_info_t *info,
	      char *servmap_host1, char *servmap_host2)

{

  SERVMAP_info_t info_copy = *info;

  /*
   * byte swapping
   */

  SMU_htonl_Info(&info_copy);

  return (Communicate_both(SERVMAP_REGISTER_SERVER,
			   &info_copy,
			   servmap_host1, servmap_host2));

}

/*****************************************************************
 *
 * request info about servers from servmap_host1; 
 * if fail, try servmap_host2
 * 
 * Input:  SERVMAP_request_t *req; 
 * Output  int *nservers;               number of info_t structs returned 
 * SERVMAP_info_t **info;       ptr to array of info_t structs 
 * Return 1 on success, 0 if no matching servers found, -1 if cant contact
 * server mapper(s).
 * This routine handles all network byte ordering conversions.
 * Note that info returned is a pointer to a static buffer.
 * 
 * Special hostname handling: if "local", use local hostname; if
 * NULL, empty string, or "none", then skip.  
 */

int SMU_requestInfo(SERVMAP_request_t *req, int *nservers, 
		    SERVMAP_info_t **info,
		    char *servmap_host1, char *servmap_host2)

{

  char *mess;
  long mess_len = 0;

  SERVMAP_request_t req_copy = *req;
  SERVMAP_reply_t *reply;
  SERVMAP_info_t *infoptr;
  int ret, i;

  /* put in network byte order */

  SMU_htonl_Request(&req_copy);
  
  /*
   * Send request first to servmap_host1 and then
   * servmap_host2 if 1 failed
   */

  if (FALSE == (ret = Communicate(SERVMAP_GET_SERVER_INFO,
				  Smu_hostname(servmap_host1), &req_copy, 
				  sizeof(req_copy), &mess, &mess_len))) {

    ret = Communicate(SERVMAP_GET_SERVER_INFO,
		      Smu_hostname(servmap_host2), &req_copy, 
		      sizeof(req_copy), &mess, &mess_len);
    
  }

  if (!ret) {
    return (-1);
  }

  /*
   * send reply
   */
  
  reply = (SERVMAP_reply_t *) mess;
  SMU_ntohl_Reply(reply);
  if (SERVMAP_SUCCESS != reply->return_code) {
    *nservers = 0;
    return (0);
  }

  *nservers = reply->n_servers;
  if (*nservers == 0) {
    return (0);
  }

  /* set return pointer to info array */

  infoptr = (SERVMAP_info_t *) (mess + sizeof(SERVMAP_reply_t));
  *info = infoptr;

  /* set byte order on all info messages */
  
  for (i=0; i < reply->n_servers; i++, infoptr++) {
    SMU_ntohl_Info(infoptr);
  }

  return (1);

}

/*************************************************************************
 *
 * unregister the server from servmap_host1 and servmap_host2
 * Note this routine will fill in info->host; since servmap uses only host,
 * port, application only needs to fill in port.
 * Special hostname handling: if "local", use local hostname; if
 * NULL, empty string, or "NONE", then skip.  
 */

void SMU_unregister(SERVMAP_info_t *info,
		    char *servmap_host1, char *servmap_host2)

{

  static int first = TRUE;
  static char host[SERVMAP_HOST_MAX];
  static char ipaddr[SERVMAP_HOST_MAX];

  SERVMAP_info_t info_copy = *info;

  if (first) {
    STRncopy(host, PORThostnameFull(), SERVMAP_HOST_MAX);
    STRncopy(ipaddr, PORThostIpAddr(), SERVMAP_HOST_MAX);
    first = FALSE;
  }

  STRncopy(info_copy.host, host, SERVMAP_HOST_MAX);
  STRncopy(info_copy.ipaddr, ipaddr, SERVMAP_HOST_MAX);
  
  SMU_htonl_Info(&info_copy);

  Communicate_both(SERVMAP_UNREGISTER_SERVER,
		   &info_copy,
		   servmap_host1, servmap_host2);

}

/************************************************************************
 *
 * Register the last data time for a server - this is used by algorithms
 * etc to notify the server mapper of new data.
 */

int
SMU_last_data(char *servmap_host1, char *servmap_host2,
	      char *type, char *subtype,
	      char *instance, char *dir,
	      si32 last_data_time)
{

  SERVMAP_info_t info;
  
  STRncopy(info.server_type, type, SERVMAP_NAME_MAX);
  STRncopy(info.server_subtype, subtype, SERVMAP_NAME_MAX);
  STRncopy(info.instance, instance, SERVMAP_INSTANCE_MAX);
  STRncopy(info.dir, dir, SERVMAP_DIR_MAX);
  info.last_data = last_data_time;

  SMU_htonl_Info(&info);
  
  return (Communicate_both(SERVMAP_LAST_DATA,
			   &info,
			   servmap_host1, servmap_host2));

}

/************************************************************************
 *
 * Register the start and end data times for a server - this is used by 
 * algorithms etc to notify the server mapper of data times.
 */

int
SMU_start_and_end_data(char *servmap_host1, char *servmap_host2,
		       char *type, char *subtype,
		       char *instance, char *dir,
		       si32 data_start_time, si32 data_end_time)
{

  SERVMAP_info_t info;
  
  STRncopy(info.server_type, type, SERVMAP_NAME_MAX);
  STRncopy(info.server_subtype, subtype, SERVMAP_NAME_MAX);
  STRncopy(info.instance, instance, SERVMAP_INSTANCE_MAX);
  STRncopy(info.dir, dir, SERVMAP_DIR_MAX);
  info.start_time = data_start_time;
  info.end_time = data_end_time;
  
  SMU_htonl_Info(&info);
  
  return (Communicate_both(SERVMAP_DATA_TIMES,
			   &info,
			   servmap_host1, servmap_host2));

}
