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
/****************************************************************************
 * get_realtime_info.c
 *
 * Gets the info for the realtime data, from the shared memory segment
 * written to by storm_ident and storm_track. Opens the files etc.
 *
 * Returns 0 if successful, -1 on failure.
 *
 * If successsful, loads up the storm and file names, and the time
 * of the scan closest to the requested time.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * April 1992
 *
 ****************************************************************************/

#include "track_server.h"

int get_realtime_info (tdata_request_t *request,
		       storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle,
		       si32 *dtime,
		       char *error_text,
		       int *storm_locked_p, int *track_locked_p)

{

  char *storm_file_path;
  char *track_file_path;
  
  int ireturn;
  si32 first_data_time;
  si32 latest_data_time = 0;

  if (Glob->params.debug) {
    fprintf(stderr, "*** get_realtime_info ***\n");
  }

  /*
   * if realtime requested and there is no realtime data available,
   * set error return
   */

  if (request->source == TDATA_REALTIME) {

    if (!realtime_avail(&storm_file_path, &track_file_path,
			&latest_data_time)) {
      
      sprintf(error_text,
	      "No realtime track data available\n");
      
      ireturn = -1;
      goto return_point;
      
    } /* if (!realtime_avail ... */

  } else {

    /*
     * request->source == TDATA_LATEST
     */

    /*
     * first try to get realtime data
     */
    
    if (!realtime_avail(&storm_file_path, &track_file_path,
			&latest_data_time)) {
      
      /*
       * no realtime data, try to get limits of file data
       */
    
      if (find_start_and_end(&first_data_time,
			     &latest_data_time,
			     &storm_file_path,
			     &track_file_path)) {
	
	sprintf(error_text,
		"get_realtime info - no track data files\n");
	
	ireturn = -1;
	goto return_point;
	
      } /* if (find_start_and_end) */

    } /* if (!realtime_avail ... */

  } /* if (request->source == TDATA_REALTIME) */

  /*
   * open the storm file
   */
  
  if (RfOpenStormFiles (s_handle, "r",
			storm_file_path,
			(char *) NULL,
			"get_realtime_info")) {
    sprintf(error_text,
	    "Unable to open storm file '%s'\n",
	    storm_file_path);
    ireturn = -1;
    goto return_point;
  }

  /*
   * lock storm header
   */

  if (Glob->params.debug) {
    fprintf(stderr, "Trying to lock storm file\n");
  }

  if (ta_lock_file_procmap(s_handle->header_file_path,
			   s_handle->header_file, "r")) {
    fprintf(stderr, "WARNING - %s:get_realtime_info\n", Glob->prog_name);
    *storm_locked_p = FALSE;
  } else {
    *storm_locked_p = TRUE;
  }

  if (Glob->params.debug) {
    fprintf(stderr, "storm file locked\n");
  }

  /*
   * read in header
   */
    
  if (RfReadStormHeader(s_handle, "get_realtime_info") != R_SUCCESS) {
    sprintf(error_text,
	    "Error reading storm file '%s'",
	    storm_file_path);
    ireturn = -1;
    goto return_point;
  }
    
  /*
   * read in the header for scan 0
   */
  
  if (RfReadStormScan(s_handle, 0, "get_realtime_info")
      != R_SUCCESS) {
    sprintf(error_text,
	    "Error reading storm file '%s'",
	    storm_file_path);
    ireturn = -1;
    goto return_point;
  }
  
  /*
   * open the file if it exists - if it does not exist, return
   * failure
   */
      
  if (RfOpenTrackFiles (t_handle, "r",
			track_file_path,
			(char *) NULL,
			"get_realtime_info")) {
    sprintf(error_text,
	    "Could not open track file '%s'",
	    track_file_path);
    ireturn = -1;
    goto return_point;
  }
      
  /*
   * lock track header
   */
  
  if (Glob->params.debug) {
    fprintf(stderr, "Trying to lock track file\n");
  }

  if (ta_lock_file_procmap(t_handle->header_file_path,
			   t_handle->header_file, "r")) {
    fprintf(stderr, "WARNING - %s:get_realtime_info\n", Glob->prog_name);
    *track_locked_p = FALSE;
  } else {
    *track_locked_p = TRUE;
  }

  if (Glob->params.debug) {
    fprintf(stderr, "Track file locked\n");
  }

  /*
   * read in header
   */
      
  if (RfReadTrackHeader(t_handle, "get_realtime_info") != R_SUCCESS) {
	
    sprintf(error_text,
	    "Error reading header of track file '%s'",
	    track_file_path);
    
    ireturn = -1;
    goto return_point;
    
  }
      
  ireturn = 0;
      
 return_point:
  
  if (ireturn) {
    RfCloseStormFiles(s_handle, "get_realtime_info");
    RfCloseTrackFiles(t_handle, "get_realtime_info");
  } /* if (ireturn) */
  
  *dtime = latest_data_time;
  
  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "time = %s\n", utimstr(*dtime));
  } /* if (Glob->params.debug >= DEBUG_NORM) */
  
  return (ireturn);
  
}
