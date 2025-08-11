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
 * March 1992
 *
 * Mike Dixon
 * Minor Mods - F. Hage
 *
 *********************************************************************/

#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <netinet/in.h>
#include <toolsa/os_config.h>
#include <cidd/cidd_files.h>
#include <cidd/cidd_lib.h>
#include <toolsa/servmap.h>
#include <toolsa/smu.h>
#include <toolsa/str.h>
#include <toolsa/port.h>

#define DIR_NAME_LEN 8


void register_server(char   **dir,
		     int    num_dirs,
		     char   *suffix,
		     char   *instance,
		     char   *service_id,
		     char   *user_info,
		     int    port,
		     int    realtime_flag,
		     time_t last_request_time,
		     char   *host1,
		     char   *host2,
		     int    n_requests)
{

  static time_t prev_time_registered = 0;
  static time_t prev_time_scan_files = 0;
  static int  num_found;
  static long *data_time;

  time_t this_time;
  SERVMAP_info_t info;

  /*
   * get the time, determine if the server needs to be registered
   */

  this_time = time((time_t *) NULL);

  if ((this_time - prev_time_registered) < SERVMAP_REGISTER_INTERVAL) {
  
    /*
     * insufficient time has passed since previous registration
     * so return now
     */

    return;

  } else {

    prev_time_registered = this_time;

  }

  if(realtime_flag) {
      num_found = find_all_data_times(&data_time,num_dirs,dir,suffix,this_time - 7200,this_time + 7200);
  } else {
	 /* Avoid scanning the whole dir tree too often - It's archival data after all :) */
	 if((this_time - prev_time_scan_files) > (SERVMAP_REGISTER_INTERVAL * 5)) {
	    num_found = find_all_data_times(&data_time,num_dirs,dir,suffix,0,this_time + 7200);
	 }
  }

  /*
   * load up server mapper info struct
   */

  strcpy(info.server_type, SERVMAP_TYPE_CIDD);
  strcpy(info.server_subtype, service_id);
  STRncopy(info.instance, instance, SERVMAP_INSTANCE_MAX);
  if (num_dirs == 1) {
    STRncopy(info.dir, dir[0], SERVMAP_DIR_MAX);
  } else if (num_dirs > 1) {
    STRncopy(info.dir, dir[1], SERVMAP_DIR_MAX);
  }
  STRncopy(info.host, PORThostname(), SERVMAP_NAME_MAX);
  info.port = port;
  info.realtime_avail = realtime_flag;
  info.last_request = last_request_time;

  if (num_found > 0) {
    info.start_time = data_time[0];
    info.end_time = data_time[num_found -1];
    info.last_data = data_time[num_found -1];
    info.n_data_requests = n_requests;
    info.status = SERVMAP_STATUS_OK;

  } else {
    info.start_time = 0;
    info.last_data = 0;
    info.end_time = 0;
    info.status = SERVMAP_STATUS_NO_DATA;
  }

  memset((char *) info.user_data,0, (int) (SERVMAP_USER_DATA_MAX * sizeof(long)));

  STRncopy(info.user_info, user_info, SERVMAP_USER_INFO_MAX);
  
  /*
   * register with the primary and secondary server mappers
   */

#ifdef DBG
    puts("Registering Service\n");
#endif
  SMU_register (&info,host1 ,host2);
  
}
