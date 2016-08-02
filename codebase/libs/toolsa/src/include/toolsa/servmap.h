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
#ifndef SERVMAP_WAS_INCLUDED
#define SERVMAP_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

/*****************************************************************************
 * server_mapper.h 
 *
 * C header file for data server mapper
 *
 * last changed: 
 *	11/15/93 JCaron: add SERVMAP_UNREGISTER_SERVER, more documentation
 *     	12/8/92	 NRehak: change SERVMAP_GET_SERVER_INFO description to allow
 *                     more general queries of servers
 *      7/8/92 	JCaron: add OLD_SERVERS type
 *****************************************************************************/

/*****************************************************************************
 *
 * OVERVIEW
 *
 * The server_mapper is intended to keep up-to-date information on the
 * available data servers. It is intended that the mapper will run in
 * two well-known places - a primary location and a secondary location
 * should the primary one fail. If clients cannot connect to the primary
 * mapper, they should try the secondary.
 *
 * DATA FORMAT
 *
 * All data is passed in XDR. Initially, this simply means
 * longs will be converted to and from network byte order. Any floats or
 * doubles used later on will need to be converted to and from XDR.
 *
 * Messaging is done with the standard packet headers (old or new). see
 * sockutil.h for details.  Use the message type
 * (e.g., SERVMAP_REGISTER_SERVER, and SERVMAP_GET_SERVER_INFO ) as the
 * socket header id.
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
 * 1) SERVMAP_REGISTER_SERVER
 *    server registration - at regular intervals (SERVMAP_REGISTER_INTERVAL)
 *    each server connects to the mapper and registers its current status.
 *    This includes the type and subtype, instance, host name, port number, 
 *    start and end times for historical data, and whether realtime data 
 *    is available. Note that the host, port is the way that servmap defines a 
 *    unique server.
 *
 *    The request packet contains the following structs:
 *      SERVMAP_info_t - passes server info
 *    The reply packet contains the following struct:
 *      SERVMAP_reply_t - return code
 *
 *
 * 2) SERVMAP_GET_SERVER_INFO
 *    request for server information - a client requests information
 *    for servers of interest to it.  It does this by indicating server
 *    type, subtype and instance and by indicating the realtime/historical
 *    data desired.  Empty strings indicate that ALL servers matching the
 *    other information given are desired.
 *
 *    The type and subtype are matched (exactly) if not the empty string;
 *    the instance is matched up to the number of chars of the request; thus
 *    request "OPERATIONS" will match both "OPERATIONS 1" and "OPERATIONS 2"
 *
 *    The client may specify it wants realtime data (want_realtime == TRUE)
 *    or wants all types of data (want_realtime == FALSE, and time == 0)
 *    or give a time of interest for historical data (want_realtime == FALSE,
 *    and time != 0).  If there is more than one server that matches, info
 *    about all will be returned.
 *
 *    The request packet contains the following structs:
 *      SERVMAP_request_t - defines types and time/current
 *    The reply packet contains the following structs:
 *      SERVMAP_reply_t - return code and number of servers
 *      n_servers * SERVMAP_info_t - info for each server
 *
 * 3) SERVMAP_UNREGISTER_SERVER
 *    A server about to exit sends this message to the server mapper to 
 *    unregister itself. Servmap uses only the host, port to find the server
 *    to unregister.
 *
 *    The request packet contains the following structs:
 *      SERVMAP_info_t - passes server info
 *    The reply packet contains the following struct:
 *      SERVMAP_reply_t - return code
 *
 * 4) SERVMAP_LAST_DATA
 *
 *    Any process which knows that the latest data time for a running server
 *    has changed may register that fact with servmap. If the server is not
 *    running this function is ignored by servmap.
 *
 *    The request packet contains the following structs:
 *      SERVMAP_info_t - passes server info
 *    The reply packet contains the following struct:
 *      SERVMAP_reply_t - return code
 *
 * 5) SERVMAP_DATA_TIMES
 *
 *    Any process which knows that the data start and end times for a running
 *    server has changed may register that fact with servmap. If the server
 *    is not running this function is ignored by servmap.
 *
 *    The request packet contains the following structs:
 *      SERVMAP_info_t - passes server info
 *    The reply packet contains the following struct:
 *      SERVMAP_reply_t - return code
 *
 * 6) info timeout - after SERVMAP_VALIDITY_INTERVAL the server information
 *    held by the mapper will time out and be removed. A functioning server
 *    should re-register its info before this occurs. The timeout will
 *    remove servers which have gone down.
 *
 * 7) connect responsibilities
 *    A client can ignore the reply from servmap, but must disconnect
 *    the socket whether it reads the reply or not.
 *
 * 8) server status information. 
 *	The SERVMAP_info_t struct has two time fields for servers to
 *      optionally indicate status when they register:
 *	1) last_data : last time data was received 
 *	2) last_request : last time a client made a request to the server
 *    These will be displayed by the servmap monitor for realtime servers
 *
 *****************************************************************************/

#include <dataport/port_types.h>

/*
 * true or false
 */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*
 * interval values - secs
 */

#define SERVMAP_REGISTER_INTERVAL 60
#define SERVMAP_VALIDITY_INTERVAL 180

#define SERVMAP_DEFAULT_PORT 5432

/*
 * server types
 */

#define SERVMAP_TYPE_ALG   	  "Algorithm"
#define SERVMAP_TYPE_ASCII   	  "Ascii"
#define SERVMAP_TYPE_CART_SLAVE   "CartSlave"
#define SERVMAP_TYPE_CIDD         "Cidd"
#define SERVMAP_TYPE_FOS          "FOS"
#define SERVMAP_TYPE_GIS   	  "GeogInfo"
#define SERVMAP_TYPE_GRIDDED      "Gridded"
#define SERVMAP_TYPE_GDS          "GDS"
#define SERVMAP_TYPE_GUI          "GUI"
#define SERVMAP_SUBTYPE_MCIDAS    "Mcidas"
#define SERVMAP_TYPE_OLD_SERVER   "OldServer"
#define SERVMAP_TYPE_PRODSERV	  "Product"
#define SERVMAP_TYPE_STORM_TRACK  "StormTrack"

/*
 * server subtypes
 */

/* Algorithm subtypes */
#define SERVMAP_SUBTYPE_WIA_AREA	"WIA_Area"

/* Cidd Subtypes */
#define SERVMAP_SUBTYPE_DOBSON 		"Dobson"
#define SERVMAP_SUBTYPE_DUAL_DOPPLER    "DualDoppler"
#define SERVMAP_SUBTYPE_TREC 		"Trec"
#define SERVMAP_SUBTYPE_SAT_VIS_1KM 	"Sat_Vis_1km"
#define SERVMAP_SUBTYPE_SAT_VIS_4KM 	"Sat_Vis_4km"
#define SERVMAP_SUBTYPE_SAT_IR_8KM 	"Sat_IR_8km"
#define SERVMAP_SUBTYPE_RADIAL      	"Radial"

/* FOS Subtypes */
#define SERVMAP_SUBTYPE_SAO         "sao_server"
#define SERVMAP_SUBTYPE_AIRMET      "airmet_server"
#define SERVMAP_SUBTYPE_SIGMET      "sigmet_server"

/* GIS Servers */
#define SERVMAP_SUBTYPE_POSLOC		"Position"

/* Gridded Subtypes*/
#define SERVMAP_SUBTYPE_ELEVATION_10KM 	"Elev_10km"
#define SERVMAP_SUBTYPE_KAVOURAS 	"Kavouras"
#define SERVMAP_SUBTYPE_WSI             "WSI"
#define SERVMAP_SUBTYPE_MOSAIC          "Mosaic"
#define SERVMAP_SUBTYPE_LAPS    	"Laps"
#define SERVMAP_SUBTYPE_MAPS    	"Maps"
#define SERVMAP_SUBTYPE_PRECIP_5NM 	"Precip_5nm"
#define SERVMAP_SUBTYPE_PRECIP_50NM 	"Precip_50nm"

/* GUI Subtypes */
#define SERVMAP_SUBTYPE_RED		"RegDisplay"
#define SERVMAP_SUBTYPE_NAT		"NatDisplay"

/* Product Server Subtype */
#define SERVMAP_SUBTYPE_TDWR		"Tdwr"
#define SERVMAP_SUBTYPE_AWDL		"Awdl"
#define SERVMAP_SUBTYPE_AWPS		"Awps"

/* Other Subtype */
#define SERVMAP_SUBTYPE_LIGHTNING	"Lightning"
#define SERVMAP_SUBTYPE_NONE	""

/* standard instance strings */
#define SERVMAP_INSTANCE_OPERATIONS "Operations"
#define SERVMAP_INSTANCE_DEMO 	    "Demo"
#define SERVMAP_INSTANCE_TEST 	    "Test"
#define SERVMAP_INSTANCE_NONE	""

/*
 * message types; should be in the range SERVMAP_REQUEST_FIRST to
 * SERVMAP_REQUEST_LAST, so that we can tell if a client
 * has accidentally connected to the wrong port
 */

#define SERVMAP_REQUEST_FIRST     300
#define SERVMAP_REGISTER_SERVER   300
#define SERVMAP_GET_SERVER_INFO   301
#define SERVMAP_REPLY             302
#define SERVMAP_UNREGISTER_SERVER 303
#define SERVMAP_LAST_DATA         304
#define SERVMAP_DATA_TIMES        305
#define SERVMAP_REQUEST_LAST      305

/* status codes */
#define SERVMAP_STATUS_NONE 	0
#define SERVMAP_STATUS_OK 	1
#define SERVMAP_STATUS_NO_DATA 	2
#define SERVMAP_STATUS_ERR 	3

/*
 * return codes
 */

#define SERVMAP_SUCCESS 1
#define SERVMAP_FAILURE 0

/*
 * number of bytes to be swapped 
 */

#define SERVMAP_USER_DATA_MAX     8
#define SERVMAP_INFO_NBYTES_SWAP 72

/*
 * string lengths
 */

#define SERVMAP_NAME_MAX         32
#define SERVMAP_HOST_MAX         64
#define SERVMAP_USER_MAX         32
#define SERVMAP_INSTANCE_MAX     64
#define SERVMAP_USER_INFO_MAX    64
#define SERVMAP_DIR_MAX          128


typedef struct {

  si32 port;                    /* server port */
  si32 realtime_avail; 		/* TRUE if realtime data is available */
  si32 start_time;		/* start and end time if historical data */
  si32 end_time;		/* is available. = 0 if not */
  si32 last_data;		/* last time data received */
  si32 last_request;		/* last time client request was received */
  si32 status;			/* SERVMAP_STATUS_XXXXXX */
  si32 server_start_time;       /* start time for server process */

  si32 last_registered;         /* filled in by servmap */
  si32 n_servmap_queries;       /* filled in by servmap */

  si32 user_data[SERVMAP_USER_DATA_MAX];  /* reserved for the user -
					   * byte ordered */

  si32 n_data_requests;         /* number of requests for data */
  si32 n_reg;                   /* number of registrations - 
				 * filled in by servmap */

  si32 spare[6];

  char server_type[SERVMAP_NAME_MAX];
  char server_subtype[SERVMAP_NAME_MAX];
  char instance[SERVMAP_INSTANCE_MAX];
  char dir[SERVMAP_DIR_MAX];

  char user[SERVMAP_USER_MAX];  /* user name under which server
				 * was started */

  char host[SERVMAP_HOST_MAX];	/* server host */

  char ipaddr[SERVMAP_HOST_MAX]; /* server IP address */

  char user_info[SERVMAP_USER_INFO_MAX];  /* reserved for the user -
					   * not byte ordered */ 

  char extra[128];

} SERVMAP_info_t;

typedef struct {

  char server_type[SERVMAP_NAME_MAX];
  char server_subtype[SERVMAP_NAME_MAX];
  char instance[SERVMAP_INSTANCE_MAX];
  char dir[SERVMAP_DIR_MAX];

  si32 want_realtime; 		/* TRUE if want realtime data */
  si32 time; 		 	/* set if want historical data, else =0 */

  char extra[32]; 		/* not yet used : fill with zeroes */

} SERVMAP_request_t;


typedef struct {

  si32 return_code;
  si32 n_servers; /* the number of servers which match the request;
		     this number of info_t structs follow */
  char extra[16]; 		/* not yet used: fill with zeroes */

} SERVMAP_reply_t;

#ifdef __cplusplus
}
#endif

#endif
