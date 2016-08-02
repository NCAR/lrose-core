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
/***************************************************************************
 * connect_to_server.c
 *
 * Connects to the cart data server (polar2mdv)
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * August 1992
 *
 ****************************************************************************/

#include "cart_slave.h"
#include <toolsa/servmap.h>
#include <toolsa/smu.h>

static si32 n_servers;
static SERVMAP_info_t *server_info;

static int query_server_mapper(void);

void connect_to_server(void)

{

  long i;

 retry:

  PMU_auto_register("Connecting to server");

  /*
   * if possible, get server info from mapper
   */

  query_server_mapper();

  /*
   * Try to connect to the server using server mapper info
   */

  for (i = 0; i < n_servers; i++) {
    
    if ((Glob->sock_dev =
	 SKU_open_client(server_info[i].host,
			 (int) server_info[i].port)) >= 0) {
      return;
    }

  } /* i */

  /*
   * that failed, so use default data
   */

  if ((Glob->sock_dev =
       SKU_open_client(Glob->params.polar2mdv_default_host,
		       Glob->params.polar2mdv_default_port)) >= 0) {
    return;
    
  }

  /*
   * if failed, try again
   */

  sleep (10);
  goto retry;

}

/*************************************************************************
 * query_server_mapper()
 *
 * get list of available servers
 *
 *************************************************************************/

static int query_server_mapper(void)

{

  static int first_call = TRUE;

  SERVMAP_request_t request;
  SERVMAP_info_t *info;
  
  if (first_call) {

    /*
     * allocate space for server info
     */
    
    server_info = (SERVMAP_info_t *) umalloc
      ((ui32) sizeof(SERVMAP_info_t));
    
    first_call = FALSE;
    
  }
  
  /*
   * load up request struct
   */
  
  memset ((void *) &request,
          (int) 0, (size_t) sizeof(SERVMAP_request_t));

  strcpy(request.server_type, SERVMAP_TYPE_CART_SLAVE);
  strcpy(request.server_subtype, SERVMAP_SUBTYPE_DOBSON);
  strncpy(request.instance, Glob->params.polar2mdv_instance,
	  SERVMAP_INSTANCE_MAX);
  
  request.want_realtime = TRUE;

  if (SMU_requestInfo(&request, (int *) &n_servers, &info,
		      getenv("SERVMAP_HOST"),
		      getenv("SERVMAP_HOST2")) != 1)
    return (-1);

  /*
   * realloc space for info, and copy info into it
   */

  server_info = (SERVMAP_info_t *) urealloc
    ((char *) server_info,
     (ui32) (n_servers * sizeof(SERVMAP_info_t)));

  memcpy ((void *) server_info,
          (void *) info,
          (size_t) (n_servers * sizeof(SERVMAP_info_t)));

  if (n_servers < 1)
    return (-1);
  else
    return (0);
  
}

