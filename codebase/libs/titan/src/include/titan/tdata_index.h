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
#ifndef tdata_index_h
#define tdata_index_h

#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************
 * tdata_index.h
 *
 * Index struct for tdata routines
 *
 ***********************************************************************/
   
#include <titan/tdata_server.h>
#include <titan/time_hist_shmem.h>
#include <titan/track.h>
#include <toolsa/servmap.h>
#include <toolsa/smu.h>
#include <toolsa/blockbuf.h>

/*
 * track complete data header struct
 */

typedef struct {

  storm_file_params_t sparams; /* storm.h */
  track_file_params_t tparams; /* storm.h */
  si32 n_complex_tracks; /* number of complex tracks */
  si32 time;             /* time of closest scan to time requested */
  si32 data_start_time;  /* the start time for track data */
  si32 data_end_time;    /* the end time for track data */    

} tdata_complete_header_t;

/*
 * tdata_complete_index_t - index to arrays for complete data
 */

typedef struct {

  track_file_entry_t entry;
  storm_file_scan_header_t scan;
  storm_file_global_props_t gprops;
  storm_file_layer_props_t *lprops;
  storm_file_dbz_hist_t *hist;

} tdata_complete_track_entry_t;

typedef struct {

  tdata_complete_header_t header;
  complex_track_params_t *complex_params;
  simple_track_params_t **simple_params;
  tdata_complete_track_entry_t ***track_entry;
  si32 **simples_per_complex;
  
} tdata_complete_index_t;

/*
 * tdata_basic_index_t - index to arrays for basic data with
 * params
 */

typedef struct {

  tdata_basic_header_t header;
  tdata_basic_complex_params_t *complex_params;
  tdata_basic_simple_params_t **simple_params;
  tdata_basic_track_entry_t ***track_entry;
  storm_file_run_t ****storm_runs;
  si32 **simples_per_complex;

} tdata_basic_with_params_index_t;

/*
 * tdata_basic_index_t - index to arrays for basic data without
 * params
 */

typedef struct {

  tdata_basic_header_t header;
  tdata_basic_track_entry_t *track_entry;

} tdata_basic_without_params_index_t;

typedef struct {

  char *default_host;
  char *prog_name;
  char *read_buffer;
  char *server_instance;
  char *server_subtype;
  char *servmap_host1;
  char *servmap_host2;
  char current_host[TDATA_HOST_LEN];

  int connected_to_server;
  int current_port;
  int default_port;
  int mode;
  int request_notify;
  int runs_included;
  int source;
  int target_entries;
  int track_set;
  int tserver_fd;
  int messages_flag;
  int debug_flag;
  
  si32 duration_after_request;
  si32 duration_before_request;
  si32 max_message_len;
  si32 n_servers;
  si32 nbytes_buffer;
  si32 nbytes_data;
  si32 read_posn;
  si32 request_num;
  si32 time_margin;
  si32 time_requested;
  si32 time_returned;

  SERVMAP_info_t *server_info;

  tdata_basic_with_params_index_t basic_p;

  tdata_basic_without_params_index_t basic;

  tdata_complete_index_t complete;

  BLOCKbuf_t bbuf;

} tdata_index_t;

/*
 * function prototypes
 */

/*********************************************************************
 * tserver_check_notify()
 *
 * Returns TRUE if new track data notification has been received,
 *         FALSE otherwise
 *
 *********************************************************************/

extern int tserver_check_notify(tdata_index_t *index);



/***********************************************************************
 *
 * tserver_clear()
 *
 */

extern void tserver_clear(tdata_index_t *index);
     


/***********************************************************************
 *
 * tserver_free_all()
 *
 * Free up all memory associated with track server routines
 */

extern void tserver_free_all(tdata_index_t *index);



/***********************************************************************
 *
 * tserver_free_read_buffer()
 *
 * Free up memory associated with read buffer
 */

extern void tserver_free_read_buffer(tdata_index_t *index);
     


/***********************************************************************
 *
 * tserver_free_track_data()
 *
 * Free up memory associated with track data
 */

extern void tserver_free_track_data(tdata_index_t *index);
     


/*************************************************************************
 * tserver_handle_disconnect()
 *
 * Hangs up when the track server disconnects abnormally
 *
 * Does not free any memory. A subsequent call to tserver_read()
 * will reconnect.
 *
 *************************************************************************/

extern void tserver_handle_disconnect(tdata_index_t *index);
     


/*****************************************************************
 * tserver_init()
 *
 * Initialize tserver index
 *
 * Does not connect to server
 *
 * For reinitialize operations, run tserver_clear() first
 *
 *   prog_name : program name - used for error messages
 *
 *   servmap_host1, servmap_host2 : server mapper hosts
 *
 *   server_subtype, server_instance : server details
 *
 *   default_host, default_port : defaults for server location to be
 *   used if server mapper fails
 *
 *   request_notify : set TRUE if notification for new realtime data
 *   is required
 *
 *   runs_included: should runs be included in the basic data
 *     TDATA_TRUE or TDATA_FALSE
 *
 *   max_message_len : max size of data buffer sent by server - should
 *     be 8K or greater. Larger buffers will increase speed, but
 *     this memory is allocated static.
 */

extern void tserver_init(const char *prog_name,
			 const char *servmap_host1,
			 const char *servmap_host2,
			 const char *server_subtype,
			 const char *server_instance,
			 const char *default_host,
			 int default_port,
			 int request_notify,
			 int runs_included,
			 int messages_flag,
			 int debug_flag,
			 si32 max_message_len,
			 tdata_index_t *index);



/*************************************************************************
 * tserver_read()
 *
 * Read track data from track server
 *
 * Auto-connect on read.
 *
 * Must run tserver_init() first.
 *
 * If read fails, try again for reconnect.
 *
 * Inputs:
 *
 *   mode : TDATA_COMPLETE - complete structures
 *          TDATA_BASIC_WITH_PARAMS - basic structs along with the
 *            complex and simple track params
 *          TDATA_BASIC_WITHOUT_PARAMS - basic structs with header
 *            only - no complex and simple track params
 *
 *   source : TDATA_REALTIME or TDATA_ARCHIVE
 *
 *   track_set : TDATA_ALL_AT_TIME or TDATA_ALL_IN_FILE
 *
 *   target_entries: TDATA_ALL_ENTRIES or
 *                   TDATA_CURRENT_ENTRIES_ONLY or
 *                   TDATA_ENTRIES_IN_TIME_WINDOW
 *
 *   time_requested (Unix time, secs since Jan 1 1970)
 *     only for mode == TDATA_ARCHIVE
 *
 *   time_margin (secs) - search margin around time_requested
 *     only for mode == TDATA_ARCHIVE
 *              target_entries == TDATA_ALL_ENTRIES
 *
 *   duration_before_request: (secs)
 *   duration_after_request : (secs) - window around time_requested
 *     only for mode == TDATA_ARCHIVE
 *              target_entries == TDATA_ENTRIES_IN_TIME_WINDOW
 *
 *   index : the index structure
 *
 * returns 0 on success, -1 on failure
 *
 *************************************************************************/

extern int tserver_read(int mode,
			int source,
			int track_set,
			int target_entries,
			si32 time_requested,
			si32 time_margin,
			si32 duration_before_request,
			si32 duration_after_request,
			tdata_index_t *index);
     
/*************************************************************************
 * tserver_read_next_scan()
 *
 * read track data for next scan from track server. For details
 * see tserver_read()
 *
 * returns 0 on success, -1 on failure
 *
 *************************************************************************/

extern int tserver_read_next_scan(int mode,
				  int source,
				  int track_set,
				  int target_entries,
				  si32 time_requested,
				  si32 time_margin,
				  si32 duration_before_request,
				  si32 duration_after_request,
				  tdata_index_t *index);

/*************************************************************************
 * tserver_read_prev_scan()
 *
 * read track data for previous scan from track server. For details
 * see tserver_read()
 *
 * returns 0 on success, -1 on failure
 *
 *************************************************************************/

extern int tserver_read_prev_scan(int mode,
				  int source,
				  int track_set,
				  int target_entries,
				  si32 time_requested,
				  si32 time_margin,
				  si32 duration_before_request,
				  si32 duration_after_request,
				  tdata_index_t *index);
     
/*************************************************************************
 * tserver_read_first_before()
 *
 * read first data from track server before time_requested, back to
 * time_margin before that.
 *
 * Inputs:
 *
 *   mode : TDATA_COMPLETE - complete structures
 *          TDATA_BASIC_WITH_PARAMS - basic structs along with the
 *            complex and simple track params
 *          TDATA_BASIC_WITHOUT_PARAMS - basic structs with header
 *            only - no complex and simple track params
 *
 *   source : TDATA_REALTIME or TDATA_ARCHIVE
 *
 *   track_set : TDATA_ALL_AT_TIME or TDATA_ALL_IN_FILE
 *
 *   target_entries: TDATA_ALL_ENTRIES or
 *                   TDATA_CURRENT_ENTRIES_ONLY or
 *                   TDATA_ENTRIES_IN_TIME_WINDOW
 *
 *   time_requested (Unix time, secs since Jan 1 1970)
 *     only for mode == TDATA_ARCHIVE
 *
 *   time_margin (secs) - search margin before time_requested
 *     only for mode == TDATA_ARCHIVE
 *              target_entries == TDATA_ALL_ENTRIES
 *
 *   duration_before_request: (secs)
 *   duration_after_request : (secs) - window around time_requested
 *     only for mode == TDATA_ARCHIVE
 *              target_entries == TDATA_ENTRIES_IN_TIME_WINDOW
 *
 *   index : the index structure
 *
 * returns 0 on success, -1 on failure
 *
 *************************************************************************/

extern int tserver_read_first_before(int mode,
				     int source,
				     int track_set,
				     int target_entries,
				     si32 time_requested,
				     si32 time_margin,
				     si32 duration_before_request,
				     si32 duration_after_request,
				     tdata_index_t *index);
     
#ifdef __cplusplus
}
#endif

#endif

