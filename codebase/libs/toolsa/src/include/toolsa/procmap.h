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
#ifndef PROCMAP_WAS_INCLUDED
#define PROCMAP_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

/*****************************************************************************
 * procmap.h 
 *
 * C header file for process mapper
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * OVERVIEW
 *
 * procmap is intended to keep up-to-date information on processes.
 * It is intended that the mapper will run on a well_known port
 * on the $PROCMAP_HOST. The port is defined in PMU_get_default_port()
 *
 * DATA FORMAT
 *
 * All data is passed in XDR. Initially, this simply means
 * longs will be converted to and from network byte order. Any floats or
 * doubles used later on will need to be converted to and from XDR.
 *
 * Messaging is done with the standard packet headers (old or new). see
 * sockutil.h for details.
 * 
 * TIME FORMAT
 *
 * All times are passed as longs, and are to be interpreted as the number of
 * seconds since Jan 1 1970 GMT. (see time(3V))
 *
 * MESSAGES / FUNCTIONALITY
 *
 * The functions are as follows:
 *
 * 1) PROCMAP_REGISTER
 *    proc registration - at regular intervals
 *    each proc connects to the mapper and registers its current status.
 *
 * 2) PROCMAP_GET_INFO
 *    request for proc information - a client requests information
 *    for procs of interest to it.  
 *
 * 3) PROCMAP_UNREGISTER
 *    A proc about to exit sends this message to the proc mapper to 
 *    unregister itself.
 *
 * 4) connect responsibilities
 *    A client can ignore the reply from procmap, but must disconnect
 *    the socket whether it reads the reply or not.
 *
 *****************************************************************************/

#include <dataport/port_types.h>

/*
 * return codes
 */

#define PROCMAP_SUCCESS 1
#define PROCMAP_FAILURE 0

#define PROCMAP_NAME_MAX         64
#define PROCMAP_USER_MAX         32
#define PROCMAP_HOST_MAX         64
#define PROCMAP_INSTANCE_MAX     64
#define PROCMAP_STATUS_MAX       256
#define PROCMAP_HOST_RELAY_MAX   512

/*
 * intervals for regsitering.  A process will be purged from the process
 * list if it hasn't registered within PROCMAP_PURGE_MULTIPLE * 
 * max_reg_interval seconds.
 */
   
#define PROCMAP_REGISTER_INTERVAL 60
#define PROCMAP_PURGE_MULTIPLE     5

/*
 * message types; should be in the range PROCMAP_REQUEST_FIRST to
 * PROCMAP_REQUESTLAST, so that we can tell if a client
 * has accidentally connected to the wrong port
 */

#define PROCMAP_REQUEST_FIRST     400
#define PROCMAP_REGISTER          400
#define PROCMAP_GET_INFO          401
#define PROCMAP_REPLY             402
#define PROCMAP_UNREGISTER        403
#define PROCMAP_REQUEST_LAST      403
#define PROCMAP_GET_INFO_RELAY    404

#define PROCMAP_INFO_NBYTES_SWAP 32

typedef struct {

  si32 pid;                            /* process id */

  si32 heartbeat_time;                 /* last time process registered 
				        * a heartbeat */

  si32 start_time;                     /* process start time */

  si32 max_reg_interval;               /* the expected max number of
					* secs between registrations */

  si32 status;

  si32 last_registered;                 /* filled in by procmap */
  si32 n_reg;                           /* number of registrations - 
					 * filled in by procmap */
  si32 spare;

  char name[PROCMAP_NAME_MAX];
  char instance[PROCMAP_INSTANCE_MAX];
  char host[PROCMAP_HOST_MAX];	       /* host where proc is located */
  char user[PROCMAP_USER_MAX];
  char status_str[PROCMAP_STATUS_MAX]; /* status string */

} PROCMAP_info_t;

/*
 * normal request
 */

typedef struct {

  char name[PROCMAP_NAME_MAX];
  char instance[PROCMAP_INSTANCE_MAX];

} PROCMAP_request_t;

/*
 * request via relay host(s)
 */

typedef struct {

  char relay_hosts[PROCMAP_HOST_RELAY_MAX];
  char name[PROCMAP_NAME_MAX];
  char instance[PROCMAP_INSTANCE_MAX];

} PROCMAP_request_relay_t;


#define PROCMAP_REPLY_NBYTES_SWAP 16

typedef struct {

  si32 uptime;                         /* number of secs the mapper has
					* been running */
  si32 return_code;

  si32 n_procs; /* the number of procs which match the request;
		 * this number of info_t structs follow */

  si32 reply_time; /* the time the reply message was created */
  
} PROCMAP_reply_t;

#ifdef __cplusplus
}
#endif

#endif
