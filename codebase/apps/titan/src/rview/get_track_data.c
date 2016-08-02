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
/************************************************************************
 * get_track_data.c
 *
 * get track data from the track data server routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * April 1992
 *
 *************************************************************************/

#include "rview.h"

void get_track_data(void)

{

  static int first_call = TRUE;

  int request_notify;
  int runs_included;
  int request_mode;
  int target_entries;
  int data_source;
  int messages_flag;
  int debug_flag;

  if (Glob->debug) {
    fprintf(stderr, "** get_track_data **\n");
  }

  if (first_call) {

    /*
     * initialize the track server struct
     */

    if (Glob->mode == REALTIME && !Glob->use_time_hist)
      request_notify = TRUE;
    else
      request_notify = FALSE;

    runs_included = Glob->runs_included;

    if (Glob->debug) {
      debug_flag = TRUE;
      messages_flag = TRUE;
    } else {
      debug_flag = FALSE;
      messages_flag = FALSE;
    }

    tserver_init(Glob->prog_name,
		 Glob->servmap_host1,
		 Glob->servmap_host2,
		 Glob->track_server_subtype,
		 Glob->track_server_instance,
		 Glob->track_server_default_host,
		 Glob->track_server_default_port,
		 request_notify,
		 runs_included,
		 messages_flag,
		 debug_flag,
		 Glob->max_message_len,
		 &Glob->tdata_index);

    first_call = FALSE;

  } /* if (first_call) */

  request_mode = TDATA_BASIC_WITH_PARAMS;
  target_entries = TDATA_ALL_ENTRIES;
    
  data_source = Glob->mode;

  if (Glob->mode == REALTIME)
    data_source = TDATA_REALTIME;
  else
    data_source = TDATA_ARCHIVE;

  if (tserver_read(request_mode,
		   data_source,
		   (int) Glob->track_shmem->track_set,
		   target_entries,
		   Glob->time,
		   Glob->track_shmem->track_data_time_margin,
		   (si32) 0, (si32) 0,
		   &Glob->tdata_index)) {
    
    Glob->track_data_available = FALSE;
    
  } else {
    
    Glob->time = Glob->tdata_index.time_returned;
    Glob->track_data_available = TRUE;
    
  }
  
}
      
/*************************************************************
 * handle_tserver_sigpipe()
 *
 * Routine to handle sigpipe from track server
 */

int handle_tserver_sigpipe(int sig)

{

  tserver_handle_disconnect(&Glob->tdata_index);

  return (sig);

}
