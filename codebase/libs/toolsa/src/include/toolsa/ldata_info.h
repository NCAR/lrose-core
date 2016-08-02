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
#ifdef __cplusplus
 extern "C" {
#endif


/******************************************************************
 * LDATA_INFO.H
 *
 * Structures and defines for latest data info services.
 *
 * Modified from libs/cidd/src/include/cdata_util.h
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * October 1997
 *
 ***********************************************************************/

#ifndef LDATA_INFO_H
#define LDATA_INFO_H

#include <toolsa/udatetime.h>
#include <dataport/port_types.h>
#include <stdio.h>

/*************************************************************
 * LDATA_INFO services
 *
 * This module handles latest data time information. It is
 * intended for use by programs which must poll to see when
 * new data has been added to some directory or server.
 *
 * At present it is only file based. However, it will be
 * extended to include server-based data.
 * 
 * The LDATA_info_t struct is stored in a file in the 
 * directory containing the data. The info file is
 * ASCII.
 *
 */

#define LDATA_INFO_FILE_NAME "latest_data_info"
#define LDATA_INFO_STR_LEN 64
#define LDATA_INFO_INIT_FLAG 975318642

typedef void (*LDATA_heartbeat_t)(const char *label);

/*************
 * Info struct
 *
 * This is intended to be a minimalist struct - only essential
 * data for storing in the file.
 */

typedef struct {

  si32 latest_time;                      /* latest unix time (secs) */

  si32 n_fcasts;                         /* number of forecasts in data set -
					  * usually 0. There are n_fcasts
					  * fcats_times following this
					  * struct in the file.
					  */

  /*
   * File extension if applicable.
   * Only applicable for file-based queries.
   * Defaults to "none"
   */
  
  char file_ext[LDATA_INFO_STR_LEN];
  
  /*
   * User info strings, for user program use.
   * Only applicable for file-based queries.
   * Defaults to "none"
   */
  
  char user_info_1[LDATA_INFO_STR_LEN];
  char user_info_2[LDATA_INFO_STR_LEN];

} LDATA_info_t;

/******************************
 * Handle for latest data info.
 *
 * Apart from the info stuct, the handle contains temporary data
 * which is not stored in the file.
 */

typedef struct {

  date_time_t ltime;         /* latest time - full struct */

  int init_flag;             /* flag to indicate init has been done */

  int debug;                 /* debug flag - for prints */

  int not_exist_print;       /* print flags */
  int too_old_print;
  int not_modified_print;

  long prev_mod_time;        /* previous file or data-base modify time.
			      * Used to determine info age */

  int n_fcasts_alloc;       /* size of allocated forecast array */

  int *fcast_lead_times;    /* forecast lead times array */

  char *prog_name;           /* program name */

  char *source_str;          /* source string for latest info  - this is
			      * either the data directory, or server
			      * source string in formats:
			      *   port@host
			      *   type::subtype::instance
			      */

  char *file_name;

  char file_path[MAX_PATH_LEN]; /* file_path for latest info
				 * has underscore as prefix */
  
  char *tmp_path;            /* tmp file path - info is written to
			      * this file which is then renamed */

  LDATA_info_t info;  /* info struct */

} LDATA_handle_t;

/*
 * function prototypes
 */

/*******************************************************************
 * LDATA_init_handle()
 *
 * Initialize the handle.
 *
 * Inputs:
 *
 *   handle: pointer to static instance of LDATA_handle_t.
 *
 *   prog_name: program name
 *
 *   debug: flag, set to TRUE if you want debug printout
 */

extern void LDATA_init_handle(LDATA_handle_t *handle,
			      const char *prog_name,
			      int debug);

/*******************************************************************
 * LDATA_free_handle()
 *
 * Free the memory associated with the handle.
 */

extern void LDATA_free_handle(LDATA_handle_t *handle);

/*******************************************************************
 * LDATA_set_file_name()
 *
 * Sets the file name to be used in the routines.
 * The actual file name will be this name prefixed with an '_'.
 *
 * The default name is 'latest_data_info'.
 */

extern void LDATA_set_file_name(LDATA_handle_t *handle,
				const char *file_name);

/*************************************
 * LDATA_info_print()
 *
 * Prints info to output stream
 *
 * returns 0 on success, -1 on failure
 */

extern int LDATA_info_print(LDATA_handle_t* handle,
			    FILE *out);

/*********************************************************************
 * LDATA_info_read()
 *
 * Read the struct data from the current file info, including forecast
 * lead times if they are present.
 *
 * If the unix time in the file is not -1, the date and time is
 * computed from the unix time.
 * If the unix time in the file is -1, it is computed from the
 * date and time.
 *
 * Inputs:
 *
 *   handle: see LDATA_init_handle()
 *
 *   source_str:
 *
 *     for file access, this is the data directory.
 *     for network access, this is either
 *                            port@host or
 *                            type::subtype::instance
 * 
 *   max_valid_age:
 *
 *     This is the max age (in secs) for which the 
 *     latest data info is considered valid. If the info is
 *     older than this, we need to wait for new info.
 *
 *     If max_valid_age is set negative, the age test is not done.
 *
 * Side effects:
 *
 *    (1) If new data found, sets handle->prev_mod_time to
 *        file modify time.
 *
 *        NOTE: For this to work, the handle must be static between calls
 *        since the prev_mod_time in the handle is used to determine when
 *        the time of the file has changed.
 *
 *    (2) Fills out the file path in the handle.
 *
 * Returns:
 *
 *    0 on success, -1 on failure.
 *
 *********************************************************************/

extern int LDATA_info_read(LDATA_handle_t* handle,
			   const char *source_str,
			   int max_valid_age);

/*********************************************************************
 * LDATA_info_read_blocking()
 *
 * Read latest data info, blocking until info is available.
 *
 * See LDATA_info_read() for the non-blocking behavior upon
 * which this function is based.
 *
 * Inputs:
 *
 *   source_str: see LDATA_info_read()
 *
 *   max_valid_age (secs): see LDATA_info_read()
 *
 *   sleep_msecs (millisecs):
 *     While in the blocked state, the program sleeps for sleep_msecs
 *     millisecs at a time before checking again.
 *
 *   heartbeat_func(): heartbeat function
 *
 *     Each cycle, the function heartbeat_func() is called to allow
 *     any heartbeat actions to be carried out. If heartbeat_func is
 *     set to NULL, it is not called.
 *
 *     The string arg passed to the heartbeat
 *     function is "In LDATA_info_read_blocking".
 *
 * Side effects:
 *
 *   See LDATA_info_read()
 *
 *********************************************************************/

extern void LDATA_info_read_blocking(LDATA_handle_t* handle,
				     const char *source_str,
				     int max_valid_age,
				     int sleep_msecs,
				     LDATA_heartbeat_t heartbeat_func);

/*****************************************************************
 * LDATA_info_write()
 *
 * Writes latest info to file.
 *
 * Writes to a tmp file first, then moves the tmp file to
 * the final file name when done.
 *
 * Inputs:
 *
 *   handle: see LDATA_init_handle()
 *
 *   source_str:
 *
 *     for file access, this is the data directory.
 *     for network access, this is either
 *                            port@host or
 *                            type::subtype::instance
 * 
 *   file_ext: file extension if applicable, otherwise set to NULL
 *
 *   user_info: set user information if applicable, otherwise NULL
 *
 *   n_fcasts: number of forecast time, usually 0
 *
 *   fcast_lead_times: array of forecast lead times,
 *                     set this to NULL if n_fcasts == 0
 *
 * Side effect:
 *   Fills out the file path in the handle.
 *
 * Returns:
 *   On success, returns 0, on failure returns -1.
 *
 * NOTE: If the environment variable $LDATA_NO_WRITE is defined, this
 *       routine does nothing.  This is useful when regenerating old
 *       data while also running in realtime mode.
 */

extern int LDATA_info_write(LDATA_handle_t* handle,
			    const char *source_str,
			    time_t latest_time,
			    const char* file_ext,
			    const char* user_info_1,
			    const char* user_info_2,
			    int n_fcasts,
			    const int *fcast_lead_times);
     
/*******************************************************************
 * LDATA_data_path()
 *
 * Returns path of latest file using std RAP naming convention,
 * relative to top_dir.
 *
 * Note: this returns a pointer to memory in the handle. Do not free!!
 */

extern char *LDATA_data_path(LDATA_handle_t *handle, char *top_dir);
     
/*******************************************************************
 * alloc_fcasts()
 *
 * Alloc space for forecasts
 *
 */

void LDATA_alloc_fcasts(LDATA_handle_t *handle, int n_fcasts);

#endif

#ifdef __cplusplus
}
#endif
