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
#ifndef DATAMAP_WAS_INCLUDED
#define DATAMAP_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

/*****************************************************************************
 * datamap.h 
 *
 * C header file for data mapper
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * OVERVIEW
 *
 * datamap is intended to keep up-to-date information on datasets.
 * It is intended that the mapper will run on a well_known port
 * on the $DATAMAP_HOST. The port is defined as DATAMAP_DEFAULT_PORT.
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
 * 1) DATAMAP_REGISTER
 *    data registration - at regular intervals
 *    a process connects to the mapper and registers its current status
 *    after successfully reading or writing data.
 *
 * 2) DATAMAP_GET_INFO
 *    request for dataset information - a client requests information
 *    for datasets of interest to it.  
 *
 * 3) DATAMAP_UNREGISTER - *** is this applicable here ??? ***
 *    A proc about to exit sends this message to the proc mapper to 
 *    unregister itself.
 *
 * 4) connect responsibilities
 *    A client can ignore the reply from datamap, but must disconnect
 *    the socket whether it reads the reply or not.
 *
 *****************************************************************************/

#include <dataport/port_types.h>

#define DATAMAP_DEFAULT_PORT 5435

/*
 * return codes
 */

#define DATAMAP_SUCCESS 1
#define DATAMAP_FAILURE 0

#define DATAMAP_NAME_MAX         64
#define DATAMAP_USER_MAX         32
#define DATAMAP_HOST_MAX         64
#define DATAMAP_INSTANCE_MAX     64
#define DATAMAP_STATUS_MAX       256

/*
 * intervals for registering
 */
   
#define DATAMAP_REGISTER_INTERVAL 60
#define DATAMAP_PURGE_INTERVAL 600

/*
 * message types; should be in the range DATAMAP_REQUEST_FIRST to
 * DATAMAP_REQUESTLAST, so that we can tell if a client
 * has accidentally connected to the wrong port
 */

#define DATAMAP_REQUEST_FIRST     500
#define DATAMAP_REGISTER_READ     500
#define DATAMAP_REGISTER_WRITE    501
#define DATAMAP_GET_INFO          502
#define DATAMAP_REPLY             503
#define DATAMAP_UNREGISTER        504
#define DATAMAP_REQUEST_LAST      504

#define DATAMAP_INFO_NBYTES_SWAP 64

/*
 * data quality control status values
 */
#define DATAMAP_QC_OKAY		0
#define DATAMAP_QC_MARGINAL	1
#define DATAMAP_QC_BAD		2
#define DATAMAP_QC_NA		-1

typedef struct {
  
  si32 write_time;		/* last time process registered a write		*/
  si32 last_write_id;		/* record id of last write			*/
  si32 last_write_pid;		/* process id of last writer			*/
  si32 status_on_write;		/* data quality status				*/
  si32 n_write;			/* number of write registrations		*/
  
  si32 read_time;		/* last time process registered a read		*/
  si32 last_read_id;		/* record id of last read			*/
  si32 last_read_pid;		/* process id of last reader			*/
  si32 status_on_read;		/* data quality status				*/
  si32 n_read;			/* number of read registrations			*/
  
  si32 data_interval;           /* expected number of secs between registrations*/
  si32 late_interval;		/* number of seconds between registrations 	*
				 * after which data is considered late		*/
  si32 impaired_interval;	/* number of seconds between registrations	*
				 * after which data is considered impaired	*/

  si32 first_registered;	
  si32 last_registered;         /* filled in by datamap 			*/
  si32 n_reg;                   /* number of registrations -  filled by datamap */

  char name[DATAMAP_NAME_MAX];		/* dataset name, e.g., MD base name	*/
  char instance[DATAMAP_INSTANCE_MAX];	/* dataset instance, e.g., MD base dir	*/
  char host[DATAMAP_HOST_MAX];		/* host where dataset is located 	*/
  char user[DATAMAP_USER_MAX];
  char write_status_str[DATAMAP_STATUS_MAX]; 	/* status string 		*/
  char read_status_str[DATAMAP_STATUS_MAX]; 	/* status string 		*/

} DATAMAP_info_t;

typedef struct {

  char name[DATAMAP_NAME_MAX];
  char instance[DATAMAP_INSTANCE_MAX];
  char host[DATAMAP_HOST_MAX];
  
} DATAMAP_request_t;


#define DATAMAP_REPLY_NBYTES_SWAP 16

typedef struct {

  si32 uptime;		/* seconds the mapper has been monitoring dataset	*/
  si32 return_code;

  si32 n_datasets; 	/* the number of datasets which match the request;	*
			 * this number of info_t structs follow 		*/

  si32 spare;
  
} DATAMAP_reply_t;

#ifdef __cplusplus
}
#endif

#endif
