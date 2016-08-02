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
#ifndef PMU_WAS_INCLUDED
#define PMU_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

#include <toolsa/procmap.h>

/*****************************************************
 *
 * RULES FOR REGISTRATION
 *
 *  $PROCMAP_HOST is not set or empty string: Does not register
 *                (the intent being to avoid bothering
 *                people who are not running procmap 
 *                with DNS lookups)
 *
 *  $PROCMAP_HOST is set: registers with $PROCMAP_HOST
 *
 *  $PROCMAP_HOST2 is set: registers with $PROCMAP_HOST2 as well
 *
 *  $PROCMAP_HOST2 same as $PROCMAP_HOST: no double registration
 *
 *  $PROCMAP_HOST set to "none" or "null": no registration on either host
 */

/* convert data structures from network to host byte order */

extern void PMU_ntohl_Info(PROCMAP_info_t *info);
extern void PMU_ntohl_Request(PROCMAP_request_t *req);
extern void PMU_ntohl_Reply(PROCMAP_reply_t *reply);

/* convert data structures from host to network byte order */

extern void PMU_htonl_Info(PROCMAP_info_t *info);
extern void PMU_htonl_Request(PROCMAP_request_t *req);
extern void PMU_htonl_Reply(PROCMAP_reply_t *reply);

/***********************
 * register the process
 *
 * Registration action depends upon the following environment
 * variables:
 *
 *   If $PROCMAP_DIR is set, a file is written to this directory.
 *   The file name is 'procname.instance'.
 *   The file lines contain:
 *     line 0: pid
 *     line 1: max secs between registration
 *     line 2: status string
 *
 *   If $PROCMAP_HOST is set, it registers with procmap on
 *     that host
 *
 *   If $PROCMAP_HOST2 is set, it registers with procmap on
 *     that host as well
 */

extern void PMU_register(const char *prog_name, const char *instance,
			 int max_reg_interval, const char *status_str);

extern void PMU_register_pid(const char *prog_name, const char *instance,
			     int max_reg_interval, const char *status_str,
			     int pid, time_t start_time);

/**************************
 * un-register the process
 *
 * Unregistration action depends upon the following environment
 * variables:
 *
 *   If $PROCMAP_DIR is set, the relevant file in this directory
 *   is deleted.
 *
 *   If $PROCMAP_HOST is set, it unregisters with procmap on
 *     that host
 *
 *   If $PROCMAP_HOST2 is set, it unregisters with procmap on
 *     that host as well
 */

extern void PMU_unregister(const char *prog_name, const char *instance);

extern void PMU_unregister_pid(const char *prog_name,
			       const char *instance, int pid);

/******************************************
 * PMU_auto_init()
 *
 * Sets up statics for auto regsitration
 */

extern void PMU_auto_init(const char *prog_name,
			  const char *instance,
			  int reg_interval);
     
/******************************************
 * PMU_clear_init()
 *
 * Clears flag set up by init.
 * Use this to prevent any further registrations.
 */

extern void PMU_clear_init();
     
/******************************************
 * PMU_auto_register()
 *
 * Automatically registers if Reg_interval secs have expired
 * since the previous registration.
 *
 * This routine may be called frequently - registration will
 * only occur at the specified Reg_interval.
 * 
 * if $PROCMAP_VERBOSE env var is set, this will register on
 * every call.
 */

extern void PMU_auto_register(const char *status_str);

/******************************************
 * PMU_force_register()
 *
 * Forced registration.
 *
 * This routine should only be called from places in the code which do
 * not runb frequently. Call PMU_auto_register() from most places
 * in the code.
 */

extern void PMU_force_register(const char *status_str);

/******************************************
 * PMU_auto_unregister()
 *
 * Automatically unregisters - remembers process name
 * and instance
 */

extern void PMU_auto_unregister(void);

/******************************************
 * PMU_set_status()
 *
 * Set status, force register.
 *
 */

extern void PMU_set_status(si32 status, const char *status_str);

/******************************************
 * PMU_get_default_port()
 *
 * Get procmap port for communications
 * Returns: -1 for error, default port upon success
 *
 */

extern int PMU_get_default_port();

/******************************************
 * PMU_requestInfo()
 * 
 * Request info about procs from host1; 
 * if fail, try host2
 * 
 * Input:
 *   PROCMAP_request_t *req; 
 * Output:
 *   int *n_procs - number of info_t structs returned 
 *   PROCMAP_info_t **info - ptr to array of info_t structs 
 * 
 * Returns 1 on success, 0 if no matching procs found,
 *  -1 if cannot contact proc mapper(s).
 *
 * This routine handles all network byte ordering conversions.
 * Note that info returned is a pointer to a static buffer.
 * 
 * Special hostname handling: if "local", use local hostname; if
 * NULL, empty string, or "NONE", then skip.  
 */

extern int PMU_requestInfo(PROCMAP_request_t *req,
			   int *n_procs, 
			   long *uptime,
			   PROCMAP_info_t **info,
			   const char *host1, const char *host2);

/******************************************
 * PMU_init_done()
 *
 * Check whether the PMU_auto_init() function has been called.
 * Returns: 1 if init has been called, 0 if init has not been done.
 *
 */

int PMU_init_done();

#ifdef __cplusplus
}
#endif

#endif

