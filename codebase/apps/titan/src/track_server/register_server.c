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
 *
 *********************************************************************/

#include "track_server.h"
#include <toolsa/sockutil.h>
#include <toolsa/smu.h>
#include <toolsa/ldata_info.h>
#include <netinet/in.h>
#include <time.h>

#define LAST_REQ_FILENAME "track_server.last_req"

static time_t Time_last_request = 0;
static si32 N_data_requests = 0;

/*
 * prototypes
 */

static void Fill_info(si32 *data_start_time_p,
		      si32 *data_end_time_p,
		      si32 *last_data_time_p,
		      si32 *last_request_time_p,
		      si32 *n_data_requests);

#ifdef NOTNOW
static int read_last_req_time(si32 *time_last_request);
#endif

static int read_last_data_time(si32  *time_last_data);

/************************
 * register_server_init()
 */

void register_server_init(void)

{

  SMU_auto_init(SERVMAP_TYPE_STORM_TRACK,
		Glob->params.subtype,
		Glob->params.instance,
		Glob->params.storm_data_dir,
		Glob->params.port,
		Glob->params.realtime_avail,
		Fill_info);

}

/*********************
 * unregister_server()
 */

void unregister_server(void)

{
  
  SMU_auto_unregister();

}

/*********************************************************
 * Fill_info()
 * 
 * Function called by the smu_auto routines to provide the 
 * data and request times
 */

static void Fill_info(si32 *data_start_time_p,
		      si32 *data_end_time_p,
		      si32 *last_data_time_p,
		      si32 *last_request_time_p,
		      si32 *n_data_requests_p)
     
{

  si32 start_time, end_time;
  
  /*
   * get start and end info
   */

  if (find_start_and_end(&start_time,
			 &end_time,
			 (char **) NULL,
			 (char **) NULL)) {
    start_time = 0;
    end_time = 0;
  }

  *data_start_time_p = start_time;
  *data_end_time_p = end_time;
  
  if (Glob->params.realtime_avail) {
    if (read_last_data_time(last_data_time_p)) {
      *last_data_time_p = 0;
    }
  } else {
    *last_data_time_p = end_time;
  }

  *last_request_time_p = Time_last_request;
  *n_data_requests_p = N_data_requests;

}

/****************************************
 * save_last_req_time()
 *
 * Saves the last request time to a file
 *
 * returns 0 on success, -1 on failure
 */

int save_last_req_time(time_t time_last_request)

{

  char filepath[MAX_PATH_LEN];
  FILE *fp;

  sprintf(filepath, "%s%s%s", Glob->params.storm_data_dir,
	  PATH_DELIM, LAST_REQ_FILENAME);

  if ((fp = fopen(filepath, "w")) == NULL) {
    fprintf(stderr, "WARNING - %s:save_last_req_time\n", Glob->prog_name);
    fprintf(stderr, "Cannot open last_req_time file\n");
    perror(filepath);
    return(-1);
  }
  
  fprintf(fp, "%ld\n", (long) time_last_request);
  fclose(fp);

  return (0);

}

#ifdef NOTNOW
/****************************************
 * read_last_req_time()
 *
 * Reads the last request time from file
 *
 * returns 0 on success, -1 on failure
 */

static int read_last_req_time(si32 *time_last_request)

{

  char filepath[MAX_PATH_LEN];
  long last_req;
  FILE *fp;

  /*
   * initialize
   */
  
  *time_last_request = -1;

  sprintf(filepath, "%s%s%s", Glob->params.storm_data_dir,
	  PATH_DELIM, LAST_REQ_FILENAME);

  if ((fp = fopen(filepath, "r")) == NULL) {
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "WARNING - %s:read_last_req_time\n", Glob->prog_name);
      fprintf(stderr, "Cannot open last_req_time file\n");
      perror(filepath);
    }
    return(-1);
  }
  
  if (fscanf(fp, "%ld", &last_req) != 1) {
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "WARNING - %s:read_last_req_time\n", Glob->prog_name);
      fprintf(stderr, "Cannot read last req time from file\n");
      perror(filepath);
    }
    fclose(fp);
    return(-1);
  }

  fclose(fp);
  *time_last_request = last_req;

  return (0);

}
#endif

/****************************************
 * read_last_data_time()
 *
 * Reads the last data time from file
 *
 * returns 0 on success, -1 on failure
 */

static int read_last_data_time(si32 *time_last_data)

{

  static int first_call = TRUE;
  static LDATA_handle_t ldata;

  /*
   * initialize
   */

  if (first_call) {
    LDATA_init_handle(&ldata,
		      Glob->prog_name,
		      Glob->params.debug);
    first_call = FALSE;
  }

  *time_last_data = -1;

  if (!Glob->params.realtime_avail) {
    return (-1);
  }

  /*
   * Get latest data time, set end time to this.
   * Set retry time to 10 msecs. We cannot set this to
   * 0, because then  cdata_read_index_simple() returns
   * without checking seq_num or max_valid_age.
   */
  
  if (LDATA_info_read(&ldata,
		      Glob->params.storm_data_dir,
		      Glob->params.max_realtime_valid_age)) {
    /*
     * no valid file
     */
    
    return (-1);
    
  } else {

    *time_last_data = ldata.info.latest_time;
    return (0);
    
  }

}

/***************************************************************************
 * read_queue()
 *
 * reads the message queue to get info from children needed for
 * server registration
 *
 ****************************************************************************/

void read_queue(void)

{

  request_info_t *info;

  while ((info = (request_info_t *)
	  umsg_recv(Glob->msq_id, sizeof(request_info_t), FALSE)) != NULL) {
    Time_last_request = info->time_request;
    N_data_requests++;
  }
					      
}

/***************************************************************************
 * write_queue()
 *
 * writes the time to the queue as the request time
 *
 ****************************************************************************/

void write_queue(time_t request_time)

{

  request_info_t info;

  info.time_request = request_time;

  if (umsg_snd(Glob->msq_id, sizeof(request_info_t), &info,
	       FALSE)) {
    fprintf(stderr, "WARNING - %s:write_queue\n", Glob->prog_name);
    fprintf(stderr, "Cannot write to message queue id %d\n",
	    Glob->msq_id);
  }
					      
}

