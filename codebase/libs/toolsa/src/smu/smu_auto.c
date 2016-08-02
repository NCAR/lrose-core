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
/*************************************************
 * smu_auto : smu automatic utilities
 * 
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * Sept 1996
 *
 * Hacked from pmu.c
 */

#include <dataport/bigend.h>
#include <toolsa/globals.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/smu.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

static SERVMAP_info_t Info;
static SMU_fill_info_t Fill_info;
static int Verbose = FALSE;
static time_t Prev_time_registered = 0;
static int Init_done = FALSE;
static long Start_time = 0;
static char *Servmap_host;
static char *Servmap_host2;

static void do_register(void);

/******************************************
 * SMU_auto_init()
 *
 * Sets up statics for auto registration.
 *
 * Fill info is a function use to fill in some of the the data 
 * needed for registration. If NULL, defaults are used.
 *
 */

void SMU_auto_init(char *type,
		   char *subtype,
		   char *instance,
		   char *dir,
		   int port,
		   int realtime_avail,
		   SMU_fill_info_t fill_info)
{

  char *env_str;

  /*
   * make sure sigpipe is turned off
   */

  PORTsignal(SIGPIPE, SIG_IGN);
  
  memset(&Info, 0, sizeof(Info));
  STRncopy(Info.server_type, type, SERVMAP_NAME_MAX);
  STRncopy(Info.server_subtype, subtype, SERVMAP_NAME_MAX);
  STRncopy(Info.instance, instance, SERVMAP_INSTANCE_MAX);
  STRncopy(Info.dir, dir, SERVMAP_DIR_MAX);
  Info.port = port;
  Info.realtime_avail = realtime_avail;
  Fill_info = fill_info;

  SMU_get_servmap_hosts(&Servmap_host, &Servmap_host2);
  
  env_str = getenv("SERVMAP_VERBOSE");
  if (env_str && STRequal(env_str, "true")) {
    Verbose = TRUE;
  } else {
    Verbose = FALSE;
  }

  Start_time = time(NULL);
  Init_done = TRUE;
  return;
  
}

/******************************************
 * SMU_auto_register()
 *
 * Automatically registers if SERVMAP_REGISTER_INTERVAL secs
 * have expired since the previous registration.
 *
 * This routine may be called frequently - registration will
 * only occur at the specified SERVMAP_REGISTER_INTERVAL.
 */

void SMU_auto_register(void)

{

  time_t this_time;

  if (!Init_done) {
    fprintf(stderr, "WARNING - SMU_auto_register()\n");
    fprintf(stderr, "%s - %s - %s\n",
	    Info.server_type, Info.server_subtype, Info.instance);
    fprintf(stderr, "Cannot auto_register - init not done\n");
    return;
  }

  if (Verbose) {
    
    do_register();

  } else {

    /*
     * get the time
     */
    
    this_time = time((time_t *) NULL);
    
    /*
     * register process if time has arrived
     */
    
    if ((this_time - Prev_time_registered) > SERVMAP_REGISTER_INTERVAL) {
      do_register();
      Prev_time_registered = this_time;
    }
    
  } /* if (Verbose) */
  
  return;

}

/******************************************
 * SMU_auto_last_data()
 *
 * Registers last data time
 */

void SMU_auto_last_data(si32 last_data_time)
     
{
  
  if (Init_done) {
    SMU_last_data(Servmap_host, Servmap_host2,
		  Info.server_type, Info.server_subtype,
		  Info.instance, Info.dir, last_data_time);
  }
  
  return;

}

/******************************************
 * SMU_auto_start_and_end_data()
 *
 * Registers data start and end times
 */

void SMU_auto_start_and_end_data(si32 data_start_time,
				 si32 data_end_time)
     
{
  
  if (Init_done) {
    SMU_start_and_end_data(Servmap_host, Servmap_host2,
			   Info.server_type, Info.server_subtype,
			   Info.instance, Info.dir,
			   data_start_time, data_end_time);
  }
  
  return;

}

/******************************************
 * SMU_force_register()
 *
 * Forced registration.
 *
 * This routine should only be called from places in the code which do
 * not runb frequently. Call SMU_auto_register() from most places
 * in the code.
 */

void SMU_force_register(char *status_str)

{

  if (!Init_done) {
    fprintf(stderr, "WARNING - SMU_force_register()\n");
    fprintf(stderr, "Cannot register - init not done\n");
    return;
  }

  do_register();

  return;

}

/******************************************
 * SMU_auto_unregister()
 *
 * Automatically unregisters - remembers process name
 * and instance
 */

void SMU_auto_unregister(void)

{

  if (Init_done) {
    SMU_unregister(&Info, Servmap_host, Servmap_host2);
  }

  return;

}

/***********************
 * perform registration
 */

static void do_register(void)

{

  if (Start_time == 0) {
    Start_time = time(NULL);
  }
  
  Info.server_start_time = Start_time;
  
  /*
   * fill out the info struct with data supplied by calling
   * the specified routine
   */

  if (Fill_info != NULL) {
    Fill_info(&Info.start_time, &Info.end_time,
	      &Info.last_data, &Info.last_request,
	      &Info.n_data_requests);
  }
  
  /*
   * register
   */
  
  SMU_register(&Info, Servmap_host, Servmap_host2);

}
  
