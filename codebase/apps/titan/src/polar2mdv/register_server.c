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
 * register_server.c
 *
 * Registers this server with the server mapper
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "polar2mdv.h"
#include <toolsa/servmap.h>
#include <toolsa/smu.h>
#include <time.h>

void register_server(void)

{

  static time_t prev_time_reg = 0;
  time_t this_time;
  SERVMAP_info_t info;

  /*
   * register process
   */
  
  PMU_auto_register("Registering");

  /*
   * get the time
   */
  
  this_time = time((time_t *) NULL);

  /*
   * register server if time has arrived
   */
  
  if (Glob->transmit_cart_data &&
      ((this_time - prev_time_reg) > SERVMAP_REGISTER_INTERVAL)) {
    
    /*
     * load up server mapper info struct
     */
    
    memset ((void *)  &info,
	    (int) 0, (size_t) sizeof(SERVMAP_info_t));
    
    strcpy(info.server_type, SERVMAP_TYPE_CART_SLAVE);
    strcpy(info.server_subtype, SERVMAP_SUBTYPE_DOBSON);
    ustrncpy(info.instance, Glob->instance, SERVMAP_INSTANCE_MAX);
    ustrncpy(info.host, PORThostname(), SERVMAP_NAME_MAX);
    info.port = Glob->port;
    info.realtime_avail = TRUE;
    info.last_data = Glob->latest_data_time;
    info.last_request = Glob->latest_request_time;
    ustrncpy(info.user_info, Glob->info, SERVMAP_USER_INFO_MAX);
    
    /*
     * register with the primary and secondary server mappers.  Note that
     * SMU_register takes care of putting everything into network byte order.
     */
    
    SMU_register (&info,
		  Glob->servmap_host1,
		  Glob->servmap_host2);

    prev_time_reg = this_time;
    
  } /* if (Glob->transmit_cart_data ... */

}

