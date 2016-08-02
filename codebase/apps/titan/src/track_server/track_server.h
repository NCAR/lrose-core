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

/*************************************************************************
 * track_server.h
 *
 * The header for track_server program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * March 1992
 *
 *************************************************************************/

#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <rapmath/umath.h>
#include <titan/track.h>
#include <dataport/bigend.h>
#include <tdrp/tdrp.h>
#include "track_server_tdrp.h"

#define TDATA_EXPECT_HEADERS

#include <titan/tdata_index.h>

/*
 * maximum number of clients
 */
  
#define MAX_CLIENTS 32

/*
 * struct which a child uses to report to the parent
 * after a request
 */
   
typedef struct {
  time_t time_request;
} request_info_t;

/*
 * global struct
 */
  
typedef struct {
    
  char *prog_name;                        /* program name */
    
  int port;                               /* listening port number */

  key_t msq_key;                          /* key for message queue */

  int msq_id;                             /* id of the message queue */

  TDRPtable *table;                       /* TDRP parsing table */

  track_server_tdrp_struct params;        /* parameter struct */

} global_t;

/*
 * declare the global structure locally in the main,
 * and as an extern in all other routines
 */

#ifdef MAIN

global_t *Glob = NULL;

#else

extern global_t *Glob;

#endif

/*
 * function prototypes
 */

extern int find_start_and_end(si32 *start_time_p,
			      si32 *end_time_p,
			      char **end_storm_file_path_p,
			      char **end_track_file_path_p);
     
extern int flush_write_buffer(int sockfd);

extern int get_archive_info(storm_file_handle_t *s_handle,
			    track_file_handle_t *t_handle,
			    tdata_request_t *request,
			    si32 *dtime,
			    char *error_text,
			    int *storm_locked_p,
			    int *track_locked_p);

extern int get_realtime_info(tdata_request_t *request,
			     storm_file_handle_t *s_handle,
			     track_file_handle_t *t_handle,
			     si32 *dtime,
			     char *error_text,
			     int *storm_locked_p,
			     int *track_locked_p);

extern int get_track_set(si32 track_set_type,
			 track_file_handle_t *t_handle,
			 si32 dtime,
			 si32 *n_current_tracks,
			 si32 **track_set_p,
			 char *error_text);

extern si32 max_product_entries(si32 header_len,
				si32 entry_len);

extern int new_data(void);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern int process_request(int sockfd,
			   tdata_request_t *request,
			   int *notification_on,
			   char **message);

extern int provide_data(int sockfd,
			tdata_request_t *request,
			storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			int *storm_locked_p,
			int *track_locked_p);

extern int read_packet(void);

extern void read_queue(void);

extern int read_request(int sockfd,
			tdata_request_t *request);

extern int read_sized_packet();

extern int realtime_avail(char **storm_header_file_path_p,
			  char **track_header_file_path_p,
			  si32 *latest_data_time_p);

extern void register_server_init(void);

extern int save_last_req_time(time_t time_last_request);

extern void server_main(int sockfd);

extern void server_product(int sockfd);

extern void set_derived_params(void);

extern int set_max_message_len(si32 len,
			       char *error_text);

extern int set_max_packet_len();

extern void tidy_and_exit(int sig);

extern void unregister_server(void);

extern void use_new_header();

extern void use_old_header();

extern int write_basic_with_params(tdata_request_t *request,
				   int sockfd,
				   storm_file_handle_t *s_handle,
				   track_file_handle_t *t_handle,
				   si32 dtime,
				   si32 n_current_tracks,
				   si32 *track_set);

extern int write_basic_without_params(tdata_request_t *request,
				      int sockfd,
				      storm_file_handle_t *s_handle,
				      track_file_handle_t *t_handle,
				      si32 dtime);

extern int write_buffer();

extern int write_complete_data(int sockfd,
			       storm_file_handle_t *s_handle,
			       track_file_handle_t *t_handle,
			       si32 dtime,
			       si32 n_current_tracks,
			       si32 *track_set);

extern int write_notify(int sockfd);

extern int write_packet();

extern int write_product(int sockfd,
			 storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 si32 dtime);

extern void write_queue(time_t request_time);

extern int write_reply(int sockfd,
		       int status,
		       char *text);

extern int write_to_buffer(int sockfd,
			   char *data,
			   si32 nbytes,
			   int id);

#ifdef __cplusplus
}
#endif
