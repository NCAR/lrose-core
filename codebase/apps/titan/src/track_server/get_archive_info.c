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
 * get_archive_info.c
 *
 * Searches for a storm data file which spans the time requested by the
 * client. Checks that this file contains data sufficiently close to
 * at the time requested. 
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
#include <sys/stat.h>

#define MAX_DAYS_BACK 30     /* number of days to search back for a suitable
			      * file. A storm file may span a number of days,
			      * if radar operation was uninterrupted for an
			      * extended period */

int get_archive_info (storm_file_handle_t *s_handle,
		      track_file_handle_t *t_handle,
		      tdata_request_t *request,
		      si32 *dtime, char *error_text,
		      int *storm_locked_p, int *track_locked_p)

{
  
  char storm_path[MAX_PATH_LEN];
  char track_path[MAX_PATH_LEN];
  
  int ireturn;
  int time_margin_exceeded = FALSE;
  int n_scans, scan_num;
  int iday, iscan;
  
  double time_diff;
  double min_time_diff = LARGE_DOUBLE;
  
  date_time_t search_date;
  date_time_t file_start_time, file_end_time;

  time_t scan_time;
  time_t start_time, end_time;

  struct stat file_stat;

  if (Glob->params.debug) {
    fprintf(stderr, "*** get_archive_info ***\n");
  }

  if (Glob->params.debug >= DEBUG_NORM) {
    
    if (request->mode == TDATA_COMPLETE)
      fprintf(stderr, "mode: tdata_complete\n");
    else
      fprintf(stderr, "mode: tdata_basic_only\n");
    
    if (request->source == TDATA_ARCHIVE)
      fprintf(stderr, "source: tdata_archive\n");
    else
      fprintf(stderr, "source: tdata_realtime\n");
    
    fprintf(stderr, "track_set = %ld\n", (long) request->track_set);
    
    if (request->request == TDATA_REQUEST_DATA_BEFORE) {
      fprintf(stderr, "requested data before time = %s:%ld\n",
	      utimstr(request->time), (long) request->time);
    } else {
      fprintf(stderr, "requested time = %s:%ld\n",
	      utimstr(request->time), (long) request->time);
    }

    fprintf(stderr, "time_margin = %ld secs\n", (long) request->time_margin);
    
  } /* if (Glob->params.debug >= DEBUG_NORM) */
  
  if (request->request == TDATA_REQUEST_DATA_BEFORE) {
    start_time = request->time - request->time_margin;
    end_time = request->time;
  } else {
    start_time = request->time - request->time_margin;
    end_time = request->time + request->time_margin;
  }

  /*
   * move back through possible julian dates
   */

  for (iday = 0; iday < MAX_DAYS_BACK; iday++) {
    
    /*
     * compute search date
     */
    
    search_date.unix_time = request->time - iday * 86400;
    
    /*
     * compute the calendar date from the unix time
     */
    
    uconvert_from_utime(&search_date);
    
    /*
     * compute file name for storm data with this date
     */
    
    sprintf(storm_path, "%s%s%.4d%.2d%.2d.%s",
	    Glob->params.storm_data_dir, PATH_DELIM,
	    search_date.year, search_date.month, search_date.day,
	    STORM_HEADER_FILE_EXT);

    /*
     * check if the file exists - if it does not exist, go to
     * end of loop
     */

    Rf_file_uncompress(storm_path);

    if (stat(storm_path, &file_stat)) {
      continue;
    }
    
    /*
     * open the storm file
     * end of loop
     */
    
    if (RfOpenStormFiles(s_handle, "r",
			 storm_path,
			 (char *) NULL,
			 "get_archive_info")) {
      sprintf(error_text,
	      "Error opening storm file '%s'", storm_path);
      ireturn = -1;
      goto return_point;
    }

    /*
     * lock storm header
     */
	
    if (ta_lock_file_procmap(s_handle->header_file_path,
			     s_handle->header_file, "r")) {
      fprintf(stderr, "WARNING - %s:get_archive_info\n", Glob->prog_name);
      *storm_locked_p = FALSE;
    } else {
      *storm_locked_p = TRUE;
    }

    /*
     * read in header
     */
    
    if (Glob->params.debug) {
      fprintf(stderr, "Trying to lock storm file\n");
    }

    if (RfReadStormHeader(s_handle,
			  "get_archive_info") != R_SUCCESS) {
      sprintf(error_text,
	      "Error reading storm file '%s'", storm_path);
      ireturn = -1;
      goto return_point;
    }
    
    if (Glob->params.debug) {
      fprintf(stderr, "storm file locked\n");
    }

    /*
     * copy the start and end times to local variables, and
     * compute the julian date for each
     */
    
    file_start_time.unix_time = s_handle->header->start_time;
    uconvert_from_utime(&file_start_time);
    
    file_end_time.unix_time = s_handle->header->end_time;
    uconvert_from_utime(&file_end_time);
    
    /*
     * if the requested time is within the file time limits,
     * this is a valid file
     */
    
    if (end_time >= file_start_time.unix_time &&
	start_time <= file_end_time.unix_time) {
      
      /*
       * search for the data time closest to the requested time
       */

      n_scans = s_handle->header->n_scans;
      scan_num = 0;

      for (iscan = 0; iscan < n_scans; iscan++) {
	
	/*
	 * read in the scan header
	 */
	
	if (RfReadStormScan(s_handle,
			    iscan,
			    "get_archive_info")
	    != R_SUCCESS) {
	  sprintf(error_text,
		  "Error reading storm file '%s'", storm_path);
	  ireturn = -1;
	  goto return_point;
	}
	
	/*
	 * get the scan time
	 */

	scan_time = s_handle->scan->time;
	
	/*
	 * break in TDATA_REQUEST_DATA_BEFORE mode if scan time is
	 * ahead of request time
	 */
	
	if ((request->request == TDATA_REQUEST_DATA_BEFORE) &&
	    request->time < scan_time) {
	  break;
	}

	/*
	 * compute the time difference
	 */
	
	time_diff = fabs ((double) scan_time - (double) request->time);
	
	if (time_diff < min_time_diff) {
	  scan_num = iscan;
	  min_time_diff = time_diff;
	} /* if (time_diff < min_time_diff) */

      } /* iscan */
      
      /*
       * if the minimum time difference is greater than the
       * time margin, return FALSE
       */
      
      if (min_time_diff > request->time_margin) {
	
	time_margin_exceeded = TRUE;
	ireturn = -1;
	goto return_point;
	
      } /* if (min_time_diff > request->time_margin) */
      
      /*
       * set the time based on the request type
       */
	
      if ((request->request == TDATA_REQUEST_DATA_PREV_SCAN) &&
	  scan_num > 0) {

	scan_num--;

      } else if ((request->request == TDATA_REQUEST_DATA_NEXT_SCAN) &&
		 scan_num < n_scans - 1) {

	scan_num++;

      }
	    
      /*
       * read in appropriate scan
       */
	
      if (RfReadStormScan(s_handle, scan_num,
			  "get_archive_info")) {
	sprintf(error_text,
		"Error reading storm file '%s'", storm_path);
	ireturn = -1;
	goto return_point;
      }
      
      /*
       * set return time
       */
      
      *dtime = s_handle->scan->time;

      /*
       * open corresponding track data file
       */
      
      sprintf(track_path, "%s%s%.4d%.2d%.2d.%s",
	      Glob->params.storm_data_dir, PATH_DELIM,
	      search_date.year, search_date.month, search_date.day,
	      TRACK_HEADER_FILE_EXT);
      
      /*
       * open the file if it exists - if it does not exist, return
       * failure
       */
      
      if (RfOpenTrackFiles (t_handle, "r",
			    track_path,
			    (char *) NULL,
			    "get_archive_info")) {
	sprintf(error_text,
		"Could not open track file '%s'", track_path);
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
	fprintf(stderr, "WARNING - %s:get_archive_info\n", Glob->prog_name);
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
      
      if (RfReadTrackHeader(t_handle,
			    "get_archive_info") != R_SUCCESS) {
	sprintf(error_text,
		"Error reading header of track file '%s'", track_path);
	ireturn = -1;
	goto return_point;
      }
      
      ireturn = 0;
      goto return_point;
      
    } else if (start_time > file_end_time.unix_time) {
       
      /*
       * have gone back beyond the requested time without
       * finding the data
       */
      
      time_margin_exceeded = TRUE;
      ireturn = -1;
      goto return_point;
      
    } /* if (end_time >= file_start_time.unix_time ... */
    
    /*
     * go back in time by a day
     */
    
    RfCloseStormFiles(s_handle, "get_archive_info");

  } /* iday */
  
  ireturn = -1;
  time_margin_exceeded = TRUE;
  
 return_point:
  
  if (ireturn) {
    
    RfCloseStormFiles(s_handle, "get_archive_info");
    RfCloseTrackFiles(t_handle, "get_archive_info");

  } /* if (ireturn) */
  
  if (time_margin_exceeded)
    sprintf(error_text,
	    "%s %ld %s %s",
	    "No data within time margin of",
	    (long) request->time_margin, "secs of",
	    utimstr(request->time));
  
  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "time = %s\n", utimstr(*dtime));
  } /* if (Glob->params.debug >= DEBUG_NORM) */
  
  return (ireturn);
  
}
