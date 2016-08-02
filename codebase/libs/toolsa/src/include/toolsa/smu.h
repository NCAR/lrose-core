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
#ifndef SMU_WAS_INCLUDED
#define SMU_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

/* Server Mapper Utilities */

#include <toolsa/servmap.h>

/*****************************************************
 *
 * RULES FOR REGISTRATION
 *
 *  $SERVMAP_HOST is not set or empty string: registers with local
 *
 *  $SERVMAP_HOST is set: registers with $SERVMAP_HOST
 *
 *  $SERVMAP_HOST2 is set: registers with $SERVMAP_HOST2 as well
 *
 *  $SERVMAP_HOST2 same as $SERVMAP_HOST: no double registration
 *
 *  $SERVMAP_HOST set to "none" or "null": no registration on either host
 */

/* convert data structures from network to host byte order */

extern void SMU_ntohl_Info(SERVMAP_info_t *info);
extern void SMU_ntohl_Request(SERVMAP_request_t *req);
extern void SMU_ntohl_Reply(SERVMAP_reply_t *reply);

/* convert data structures from host to network byte order */

extern void SMU_htonl_Info(SERVMAP_info_t *info);
extern void SMU_htonl_Request(SERVMAP_request_t *req);
extern void SMU_htonl_Reply(SERVMAP_reply_t *reply);

/* get the server mapper host names from the standard
 * environment variables.
 */

extern void SMU_get_servmap_hosts(char **servmap_host1,
				  char **servmap_host2);

/* Send server info to servers on servmap_host1 and servmap_host2
 * Note this routine will fill in info->host, and put info into network
 * byte order.
 * Special hostname handling: if "local", use local hostname; if
 * NULL, empty string, or "NONE", then skip.  
 */

extern int SMU_register(SERVMAP_info_t *info,
			char *servmap_host1, char *servmap_host2);

/* Send server info to servers on servmap_host1 and servmap_host2
 * Note this routine assumes that info->host has already been set.
 * Special hostname handling: if "local", use local hostname; if
 * NULL, empty string, or "NONE", then skip.  
 */

extern int SMU_reg_proxy(SERVMAP_info_t *info,
			 char *servmap_host1, char *servmap_host2);

/* request info about servers from servmap_host1; 
 * if fail, try servmap_host2
 * 
 * Input:  SERVMAP_request_t *req; 
 * Output  int *nservers;		number of info_t structs returned 
 * SERVMAP_info_t **info; 	ptr to array of info_t structs 
 * Return 1 on success, 0 if no matching servers found, -1 if cant contact
 * server mapper(s).
 * This routine handles all network byte ordering conversions.
 * Note that info returned is a pointer to a static buffer.
 * 
 * Special hostname handling: if "local", use local hostname; if
 * NULL, empty string, or "NONE", then skip.  
 */

extern int SMU_requestInfo(SERVMAP_request_t *req, int *nservers, 
			   SERVMAP_info_t **info,
			   char *servmap_host1, char *servmap_host2);

/* unregister the server from servmap_host1 and servmap_host2
 * Note this routine will fill in info->host; since servmap uses only host,
 * port, application only needs to fill in port.
 * Special hostname handling: if "local", use local hostname; if
 * NULL, empty string, or "NONE", then skip.  
 */

extern void SMU_unregister(SERVMAP_info_t *info,
			   char *servmap_host1, char *servmap_host2);

/*
 * Register the last data time for a server - this is used by algorithms
 * etc to notify the server mapper of new data.
 */

extern int
  SMU_last_data(char *servmap_host1, char *servmap_host2,
		char *type, char *subtype,
		char *instance, char *dir,
		si32 last_data_time);


/*
 * Register the start and end data times for a server - this is used by 
 * algorithms etc to notify the server mapper of data times.
 */

extern int
SMU_start_and_end_data(char *servmap_host1, char *servmap_host2,
		       char *type, char *subtype,
		       char *instance, char *dir,
		       si32 data_start_time, si32 data_end_time);

/*
 * function type for filling out server mapper data struct
 * during registration
 */

typedef void (*SMU_fill_info_t)(si32 *data_start_time_p,
				si32 *data_end_time_p,
				si32 *last_data_time_p,
				si32 *last_request_time_p,
				si32 *n_data_requests_p);

/******************************************
 * SMU_auto_init()
 *
 * Sets up statics for auto registration.
 *
 * Fill info is a function use to fill in some of the the data 
 * needed for registration. If NULL, defaults are used.
 */

extern void SMU_auto_init(char *type,
			  char *subtype,
			  char *instance,
			  char *dir,
			  int port,
			  int realtime_avail,
			  SMU_fill_info_t fill_info);

/******************************************
 * SMU_auto_register()
 *
 * Automatically registers if SERVMAP_REGISTER_INTERVAL secs
 * have expired since the previous registration.
 *
 * This routine may be called frequently - registration will
 * only occur at the specified SERVMAP_REGISTER_INTERVAL.
 */

extern void SMU_auto_register(void);

/******************************************
 * SMU_auto_last_data()
 *
 * Registers last data time
 */

extern void SMU_auto_last_data(si32 last_data_time);
     
/******************************************
 * SMU_auto_start_and_end_data()
 *
 * Registers data start and end times
 */

extern void SMU_auto_start_and_end_data(si32 data_start_time,
					si32 data_end_time);
     
/******************************************
 * SMU_force_register()
 *
 * Forced registration.
 *
 * This routine should only be called from places in the code which do
 * not runb frequently. Call SMU_auto_register() from most places
 * in the code.
 */

extern void SMU_force_register(char *status_str);

/******************************************
 * SMU_auto_unregister()
 *
 * Automatically unregisters - remembers process name
 * and instance
 */

extern void SMU_auto_unregister(void);

/******************************************
 * SMU_last_data_init()
 *
 * Sets up statics for last registration.
 *
 * Fill info is a function use to fill in some of the the data 
 * needed for registration. If NULL, defaults are used.
 *
 */

extern void SMU_last_data_init(char *type,
			       char *subtype,
			       char *instance,
			       char *dir);

/******************************************
 * SMU_reg_last_data()
 *
 * Registers last data time.
 */

extern void SMU_reg_last_data(si32 last_data_time);
     

#ifdef __cplusplus
}
#endif

#endif

