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
/***************************************************************
 * perform_tracking.c
 *
 * sets up track file and controls tracking routine
 *
 * Returns 0 on success, -1 on failure
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * July 1993
 *
 ***************************************************************/

#include "storm_track.h"
#include <time.h>
#include <toolsa/file_io.h>

#define SEM_CHECK_PERIOD 15

/*
 * file scope variables
 */

static char State_file_path[MAX_PATH_LEN];
static storm_status_t *Storms1 = NULL;
static storm_status_t *Storms2 = NULL;
static si32 Nstorms1, Nstorms2;
static date_time_t Time1, Time2;
static si32 *Track_continues = NULL;
static track_utime_t *Track_utime = NULL;
static time_t State_tag;

/*
 * prototypes
 */

static void alloc_arrays(void);

static void free_arrays(track_file_handle_t *t_handle);

static void free_terminating(void);

static void init_arrays (track_file_handle_t *t_handle);

static void lock_header_file(track_file_handle_t *t_handle);

static void prepare_new_file(storm_file_handle_t *s_handle,
			     track_file_handle_t *t_handle);

static void print_matches(storm_status_t *storms1,
			  storm_status_t *storms2,
			  si32 nstorms1, si32 nstorms2);

static void print_status(storm_status_t *storm);

static int read_prev_state(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle,
			   char *storm_header_path);

static void save_current_state(track_file_handle_t *t_handle);

static void set_header_invalid(track_file_handle_t *t_handle);

static void set_header_valid(track_file_handle_t *t_handle,
			     si32 scan_num);

static void swap_arrays(void);

static void track(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		  si32 scan_num);

static void unlock_header_file(track_file_handle_t *t_handle);

/*
 * main routine
 */

int perform_tracking(storm_file_handle_t *s_handle,
		     track_file_handle_t *t_handle,
		     int call_mode,
		     char *storm_header_path)

{
  
  static int first_call = TRUE;
  static int file_prepared = FALSE;

  si32 iscan;
  si32 start_scan;
  si32 n_scans;
  si32 scan_num;
  path_parts_t path_parts;
  
  /*
   * on first call, compute the path for the file which stores the
   * state of the analysis for use on restart
   */

  if (first_call && call_mode != RETRACK_FILE) {
    
    uparse_path(storm_header_path, &path_parts);
    sprintf(State_file_path, "%s%s",
	    path_parts.dir, STATE_FILE_NAME);
    ufree_parsed_path(&path_parts);
    first_call = FALSE;
    
  } /* if (first_call) */

  switch (call_mode) {
    
  case RETRACK_FILE:
    
    open_files (s_handle, t_handle, "w+", storm_header_path);
    setup_scan(s_handle, 0L, &Nstorms1, &Time1, storm_header_path);
    alloc_arrays();
    load_storm_props(s_handle, Nstorms1, Storms1, &Time1); 
    prepare_new_file(s_handle, t_handle);

    n_scans = s_handle->header->n_scans;

    for (iscan = 1; iscan < n_scans; iscan++) {
      setup_scan(s_handle, iscan, &Nstorms2, &Time2, storm_header_path);
      init_arrays(t_handle);
      load_storm_props(s_handle, Nstorms2, Storms2, &Time2); 
      track(s_handle, t_handle, iscan);
      free_terminating();
      swap_arrays();
    } /* iscan */

    free_arrays(t_handle);
    RfTrackReinit(t_handle, "perform_tracking");
    RfCloseStormFiles(s_handle, "perform_tracking");
    RfCloseTrackFiles(t_handle, "perform_tracking");

    break;

  case PREPARE_NEW_FILE:

    open_files(s_handle, t_handle, "w+", storm_header_path);
    lock_header_file(t_handle);
    setup_scan(s_handle, 0L, &Nstorms1, &Time1, storm_header_path);
    alloc_arrays();
    load_storm_props(s_handle, Nstorms1, Storms1, &Time1); 
    prepare_new_file(s_handle, t_handle);
    save_current_state(t_handle);
    if (Glob->exit_signal)
      tidy_and_exit(Glob->exit_signal);
    file_prepared = TRUE;
    unlock_header_file(t_handle);
    break;
    
  case PREPARE_FOR_APPEND:
    
    if (read_prev_state(s_handle, t_handle, storm_header_path)) {
      
      open_files(s_handle, t_handle, "w+", storm_header_path);
      lock_header_file(t_handle);
      setup_scan(s_handle, 0L, &Nstorms1, &Time1, storm_header_path);
      alloc_arrays();
      load_storm_props(s_handle, Nstorms1, Storms1, &Time1); 
      prepare_new_file(s_handle, t_handle);
      save_current_state(t_handle);
      unlock_header_file(t_handle);

      if (Glob->exit_signal) {
	tidy_and_exit(Glob->exit_signal);
      }
      
      start_scan = 1;
      
    } else {

      start_scan = t_handle->header->last_scan_num + 1;
      if (RfSeekEndTrackData(t_handle, "perform_tracking"))
	tidy_and_exit(-1);
      
    }

    n_scans = s_handle->header->n_scans;

    for (iscan = start_scan; iscan < n_scans; iscan++) {

      lock_header_file(t_handle);
      setup_scan(s_handle, iscan, &Nstorms2, &Time2, storm_header_path);
      init_arrays(t_handle);
      load_storm_props(s_handle, Nstorms2, Storms2, &Time2); 
      track(s_handle, t_handle, iscan);
      free_terminating();
      swap_arrays();
      save_current_state(t_handle);
      unlock_header_file(t_handle);
      if (Glob->exit_signal)
	tidy_and_exit(Glob->exit_signal);

    } /* iscan */

    file_prepared = TRUE;

    break;
    
  case TRACK_LAST_SCAN:
  default:

    if (!file_prepared)
      return (-1);

    lock_header_file(t_handle);
    scan_num = setup_scan(s_handle, -1L, &Nstorms2, &Time2, storm_header_path);
    init_arrays(t_handle);
    load_storm_props(s_handle, Nstorms2, Storms2, &Time2); 
    track(s_handle, t_handle, scan_num);
    free_terminating();
    swap_arrays();
    save_current_state(t_handle);
    unlock_header_file(t_handle);
    if (Glob->exit_signal)
      tidy_and_exit(Glob->exit_signal);

    break;
    
  } /* switch */

  fflush(stdout);

  return (0);

}

/*******************************************************************
 * alloc_arrays()
 *
 * initial allocation for arrays
 */

static void alloc_arrays(void)

{

  /*
   * initial array allocation
   */
  
  Storms1 = (storm_status_t *) ucalloc
    ((ui32) Nstorms1, (ui32) sizeof(storm_status_t));

  Storms2 = (storm_status_t *) ucalloc
    ((ui32) 1, (ui32) sizeof(storm_status_t));
  
  Track_continues = (si32 *) ucalloc
    ((ui32) Nstorms1, (ui32) sizeof(si32));
  
  Track_utime = (track_utime_t *) umalloc
    ((ui32) (Nstorms1 * sizeof(track_utime_t)));
  
}

/*******************************************************************
 * free_arrays()
 *
 * free allocation for arrays
 */

static void free_arrays(track_file_handle_t *t_handle)

{

  si32 jstorm;
  storm_status_t *storm;
  
  for (jstorm = 0; jstorm < Nstorms2; jstorm++) {
    
    if (Track_continues[jstorm]) {
      
      storm = Storms2 + jstorm;
      ufree_non_null((void **) &storm->match_array);
      ufree_non_null((void **) &storm->proj_runs);
      ufree_non_null((void **) &storm->track->history);
      ufree_non_null((void **) &storm->track);

    } /* if (!Track_continues..... */
    
  } /* jstorm */

  ufree(Storms1);
  ufree(Storms2);
  ufree(Track_continues);
  ufree(Track_utime);

}

/*******************************************************************
 * free_terminating()
 *
 * frees up Storms1 structs which have not been continued to
 * Storms2
 */

static void free_terminating(void)

{

  si32 istorm;
  storm_status_t *storm;
  
  for (istorm = 0; istorm < Nstorms1; istorm++) {
    
    if (!Track_continues[istorm]) {
      
      storm = Storms1 + istorm;
      ufree_non_null((void **) &storm->match_array);
      ufree_non_null((void **) &storm->proj_runs);
      ufree_non_null((void **) &storm->track->history);
      ufree_non_null((void **) &storm->track);

    } /* if (!Track_continues..... */
    
  } /* istorm */

}

/******************************************************************
 * init_arrays()
 *
 * reallocate and initialize arrays
 */

static void init_arrays(track_file_handle_t *t_handle)

{

  si32 istorm, jstorm;
  storm_status_t *storm;

  /*
   * array reallocation
   */
    
  Storms2 = (storm_status_t *) urealloc
    ((char *) Storms2,
     Nstorms2 * sizeof(storm_status_t));
  
  Track_continues = (si32 *) urealloc
    ((char *) Track_continues, (ui32) (Nstorms1 * sizeof(si32)));
  
  Track_utime = (track_utime_t *) urealloc
    ((char *) Track_utime,
     (ui32) ((t_handle->header->n_simple_tracks + Nstorms2) *
	      sizeof(track_utime_t)));
      
  /*
   * initialize arrays
   */
    
  for (istorm = 0; istorm < Nstorms1; istorm++) {
    storm = Storms1 + istorm;
    storm->sum_overlap = 0.0;
    storm->sum_simple_tracks = 0;
    storm->n_match = 0;
    storm->match = -1;
  }
    
  memset((void *) Storms2, 0, Nstorms2 * sizeof(storm_status_t));
  
  for (jstorm = 0; jstorm < Nstorms2; jstorm++) {
    Storms2[jstorm].match = -1;
  }
    
  memset((void *) Track_continues, 0, Nstorms1 * sizeof(si32));
  
}

/*******************************************************************
 * lock_header_file()
 *
 * set write lock on header file
 */

static void lock_header_file(track_file_handle_t *t_handle)

{

  if (ta_lock_file_procmap(t_handle->header_file_path,
			   t_handle->header_file, "w")) {
    fprintf(stderr, "ERROR - %s:perform_tracking\n", Glob->prog_name);
    tidy_and_exit(-1);
  }

}

/*********************************************************************
 * prepare_new_file()
 *
 * prepare a new track file, with scan 0 data written
 */

static void prepare_new_file(storm_file_handle_t *s_handle,
			     track_file_handle_t *t_handle)

{

  si32 istorm;
  si32 entry_offset, first_entry_offset;
  si32 scan_num;

  if (Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "Preparing new file\n");

  /*
   * initialize the file header
   */

  init_file_header (s_handle, t_handle->header);

  /*
   * set up intial file position for track data file
   */

  if (RfSeekStartTrackData(t_handle, "perform_tracking:prepare_new_file"))
    tidy_and_exit(-1);
    
  /*
   * start new tracks for all the storms
   */
    
  scan_num = 0;

  set_header_invalid(t_handle);

  for (istorm = 0; istorm < Nstorms1; istorm++) {

    alloc_new_track(t_handle,
		    &Time1, Track_utime,
		    Storms1 + istorm,
		    scan_num,
		    TRUE, -1, 0, 0,
		    &Time1);
      
    update_times(&Time1, Track_utime, Storms1, istorm);
    
    entry_offset = write_track(s_handle,
			       t_handle,
			       &Time1,
			       Track_utime,
			       Storms1,
			       istorm,
			       scan_num);

    if (istorm == 0)
      first_entry_offset = entry_offset;
    
  } /* istorm */
    
  if (RfAllocTrackScanIndex(t_handle, scan_num + 1, "track"))
    tidy_and_exit(-1);

  t_handle->scan_index[scan_num].utime = Time1.unix_time;
  t_handle->scan_index[scan_num].n_entries = Nstorms1;
  t_handle->scan_index[scan_num].first_entry_offset = first_entry_offset;

  set_header_valid(t_handle, scan_num);
  
}

/*****************
 * print_matches()
 */

static void print_matches(storm_status_t *storms1,
			  storm_status_t *storms2,
			  si32 nstorms1, si32 nstorms2)

{

  int i, j;
  track_match_t *match;
  storm_status_t *storm;

  fprintf(stderr, "\n*** MATCHES/OVERLAPS - STORM1 ARRAY ***\n\n");

  storm = storms1;
  for (i = 0; i < nstorms1; i++, storm++) {

    if (storm->match >= 0) {
      
      print_status(storm);
      fprintf(stderr, "Storm1 %4d matches storm2 %4d, "
	      "n_simple %4d, complex_num %4d\n",
	      i, (int) storm->match,
	      storm->track->n_simple_tracks,
	      storm->track->complex_track_num);
      
    } else if (storm->sum_overlap > 0) {

      print_status(storm);
      fprintf(stderr,
	      "Storm1 %4d,     sum_overlap %10g,"
	      " n_simple   %4d, complex_num %4d\n",
	      i, storm->sum_overlap,
	      (int) storm->track->n_simple_tracks,
	      (int) storm->track->complex_track_num);

      match = storm->match_array;
      
      for (j = 0; j < storm->n_match; j++, match++) {
	fprintf(stderr, "  overlaps storm2 %4d, area %10g\n",
		(int) match->storm_num,
		match->overlap);
      } /* j */

    } else {

      print_status(storm);
      fprintf(stderr, "Storm1 %4d not matched\n", i);

    }

    fprintf(stderr, "\n");

  } /* i */

  fprintf(stderr, "\n*** MATCHES/OVERLAPS - STORM2 ARRAY ***\n\n");

  storm = storms2;
  for (i = 0; i < nstorms2; i++, storm++) {

    if (storm->match >= 0) {

      print_status(storm);
      fprintf(stderr, "Storm2 %4d matches storm1 %4d, "
	      "n_simple %4d, complex_num %4d\n",
	      i, (int) storm->match,
	      storms1[storm->match].track->n_simple_tracks,
	      storms1[storm->match].track->complex_track_num);
      
    } else if (storm->sum_overlap > 0) {

      print_status(storm);
      fprintf(stderr,
	      "Storm2 %4d,     sum_overlap %10g, sum_simple %4d\n",
	      i, storm->sum_overlap, (int) storm->sum_simple_tracks);

      match = storm->match_array;

      for (j = 0; j < storm->n_match; j++, match++) {
	fprintf(stderr, "  overlaps storm1 %4d, area %10g,"
		" n_simple   %4d, complex_num %4d\n",
		(int) match->storm_num,
		match->overlap,
		(int) match->n_simple_tracks,
		(int) match->complex_track_num);
      } /* j */

    } else {

      print_status(storm);
      fprintf(stderr, "Storm2 %4d not matched\n", i);
      
    }

    fprintf(stderr, "\n");

  } /* i */

  return;

}

/****************
 * print_status()
 */

static void print_status(storm_status_t *storm)

{

  if (storm->starts) {
    fprintf(stderr, "STARTS\n");
  }
  if (storm->stops) {
    fprintf(stderr, "STOPS\n");
  }
  if (storm->continues) {
    fprintf(stderr, "IS_CONT\n");
  }
  if (storm->has_split) {
    fprintf(stderr, "IS_SPLIT\n");
  }
  if (storm->has_merger) {
    fprintf(stderr, "IS_MERGER\n");
  }
  if (storm->has_merger && storm->has_split) {
    fprintf(stderr, "\a");
  }
  return;

}

/****************************************************************
 * read_prev_state()
 *
 * reads the previous state from file
 *
 * returns 0 if successful, -1 if not
 */

static int read_prev_state(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle,
			   char *storm_header_path)

{

  int flag;
  si32 tag;
  si32 istorm;
  si32 n_simple_tracks;
  si32 last_scan_num;
  track_file_header_t theader;
  track_status_t *track;
  FILE *fp;

  /*
   * open files
   */

  if (open_files(s_handle, t_handle, "r+", storm_header_path))
    return (-1);

  /*
   * check that track file is valid
   */

  if (!t_handle->header->file_valid)
    return (-1);

  /*
   * initialize a test header and compare the header params
   * against the existing file. If they are different, return
   * error
   */

  init_file_header(s_handle, &theader);

  if (memcmp((void *) &theader.params,
	     (void *) &t_handle->header->params,
	     sizeof(track_file_params_t)))
      return (-1);
      
  /*
   * open state file
   */

  if ((fp = fopen(State_file_path, "r")) == NULL)
    return (-1);
  
  /*
   * read  flag - if not TRUE, return because the file was not
   * correctly written
   */
  
  if (ufread((char *) &flag, sizeof(int), 1, fp) != 1) {
    fclose(fp);
    return (-1);
  }
  
  if (!flag) {
    fclose(fp);
    return (-1);
  }

  /*
   * read the state tag, compare it with the track file code
   */

  if (ufread((char *) &tag, sizeof(si32), 1, fp) != 1) {
    fclose(fp);
    return (-1);
  }

  if (tag != t_handle->header->modify_code) {
    fclose(fp);
    return (-1);
  }

  /*
   * read the last scan num
   */

  if (ufread((char *) &last_scan_num, sizeof(si32), 1, fp) != 1) {
    fclose(fp);
    return (-1);
  }

  if (last_scan_num >= s_handle->header->n_scans) {

    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Start tracking again\n");
      fprintf(stderr, "Last_scan_num, n_scans : %ld, %ld\n",
	      (long) last_scan_num, (long) s_handle->header->n_scans);
    }

    return (-1);

  }

  /*
   * read in state data
   */
  
  ufread((char *) &Nstorms1, sizeof(si32), 1, fp);
  ufread((char *) &Time1, sizeof(date_time_t), 1, fp);
  
  Storms1 = (storm_status_t *) ucalloc
    ((ui32) Nstorms1, (ui32) sizeof(storm_status_t));

  for (istorm = 0; istorm < Nstorms1; istorm++) {

    Storms1[istorm].track = (track_status_t *) umalloc
      ((ui32) sizeof(track_status_t));
    
    track = Storms1[istorm].track;
    
    ufread((char *) track, sizeof(track_status_t), 1, fp);
    
    track->history = (storm_track_props_t *) umalloc
      ((ui32) (MAX_NWEIGHTS_FORECAST *
		sizeof(storm_track_props_t)));

    ufread((char *) track->history,
	   sizeof(storm_track_props_t),
	   MAX_NWEIGHTS_FORECAST, fp);

    ufread((char *) &track->dval_dt,
	   sizeof(storm_track_props_t),
	   1, fp);

  } /* istorm */

  ufread((char *) &n_simple_tracks, sizeof(si32), 1, fp);
  
  Track_utime = (track_utime_t *) ucalloc
    ((ui32) n_simple_tracks, (ui32) sizeof(track_utime_t));
  
  if ((n_simple_tracks != t_handle->header->n_simple_tracks) ||
      (ufread((char *) Track_utime, sizeof(track_utime_t),
	      (int) n_simple_tracks, fp) != n_simple_tracks)) {
    
    /*
     * file not successfully read, so free up arrays and
     * return
     */
    
    for (istorm = 0; istorm < Nstorms1; istorm++) {
      track = Storms1[istorm].track;
      ufree_non_null((void **) &track->history);
      ufree_non_null((void **) &track);
    } /* istorm */

    ufree_non_null((void **) &Storms1);

    ufree ((char *) Track_utime);

    fclose (fp);
    return (-1);

  } /* if ((n_simple_tracks ... */
  
  /*
   * close file
   */
  
  fclose(fp);

  /*
   * alloc other arrays
   */

  Storms2 = (storm_status_t *) ucalloc
    ((ui32) 1, (ui32) sizeof(storm_status_t));
  
  Track_continues = (si32 *) ucalloc
    ((ui32) 1, (ui32) sizeof(si32));

  /*
   * success
   */

  return (0);

}

/****************************************************************
 * save_current_state()
 *
 * saves the current state to be used in the case of a restart
 */

static void save_current_state(track_file_handle_t *t_handle)

{

  int flag;
  si32 istorm;
  track_status_t *track;
  FILE *fp;

  /*
   * open state file
   */

  if ((fp = fopen(State_file_path, "w")) == NULL) {
    fprintf(stderr, "%s:perform_tracking:save_current_state\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot create state file\n");
    perror(State_file_path);
    tidy_and_exit(-1);
  }

  /*
   * write a FALSE flag, which is changed to TRUE when save
   * is complete
   */
  
  flag = FALSE;
  ufwrite((char *) &flag, sizeof(int), 1, fp);

  /*
   * write the state data
   */

  ufwrite((char *) &State_tag, sizeof(si32), 1, fp);
  ufwrite((char *) &t_handle->header->last_scan_num, sizeof(si32), 1, fp);
  ufwrite((char *) &Nstorms1, sizeof(si32), 1, fp);

  ufwrite((char *) &Time1, sizeof(date_time_t), 1, fp);

  for (istorm = 0; istorm < Nstorms1; istorm++) {

    track = Storms1[istorm].track;

    ufwrite((char *) track, sizeof(track_status_t), 1, fp);
    
    ufwrite((char *) track->history,
	    sizeof(storm_track_props_t),
	    MAX_NWEIGHTS_FORECAST, fp);
    
    ufwrite((char *) &track->dval_dt,
	    sizeof(storm_track_props_t), 1, fp);
    
  } /* istorm */

  ufwrite((char *) &t_handle->header->n_simple_tracks, sizeof(si32), 1, fp);

  if (ufwrite((char *) Track_utime, sizeof(track_utime_t),
	      (int) t_handle->header->n_simple_tracks, fp) ==
      t_handle->header->n_simple_tracks) {
    
    /*
     * write a TRUE flag to indicate that save is successful
     */
  
    flag = TRUE;
    fseek(fp, 0L, 0);
    ufwrite((char *) &flag, sizeof(int), 1, fp);

  }
  
  /*
   * close file
   */
  
  fclose(fp);

}

/****************************************************************
 * set_header_invalid()
 *
 * sets file header invalid while data writes are in progress
 * so that it can be determined if the writes were completed
 */

static void set_header_invalid(track_file_handle_t *t_handle)

{

  Glob->write_in_progress = TRUE;

  /*
   * Clear valid flag and write header
   */

  t_handle->header->file_valid = FALSE;
    
  if (RfWriteTrackHeader(t_handle, "track"))
    tidy_and_exit(-1);

}

/****************************************************************
 * set_header_valid()
 *
 * sets file header to valid after data writes are in progress
 * so that it can be determined that the writes were completed
 */

static void set_header_valid(track_file_handle_t *t_handle,
			     si32 scan_num)

{

  /*
   * set up state tag as the current unix time and store in file
   * header. Set valid flag.
   */

  State_tag = time((time_t *) NULL);
  t_handle->header->modify_code = State_tag;
  t_handle->header->file_valid = TRUE;
    
  t_handle->header->last_scan_num = scan_num;
  t_handle->header->n_scans = t_handle->header->last_scan_num + 1;
  
  if (RfWriteTrackHeader(t_handle, "track"))
    tidy_and_exit(-1);

  if (RfFlushTrackFiles(t_handle, "props_compute"))
    tidy_and_exit(-1);

  Glob->write_in_progress = FALSE;
  
}

/****************************************************************
 * swap_arrays()
 *
 * Swaps the storms structs from time 2 to time 1 in preparation
 * for the next scan
 */

static void swap_arrays(void)

{

  si32 ns1;
  storm_status_t *stat1;
  date_time_t tim1;
  
  stat1 = Storms1;
  Storms1 = Storms2;
  Storms2 = stat1;
  
  ns1 = Nstorms1;
  Nstorms1 = Nstorms2;
  Nstorms2 = ns1;
  
  tim1 = Time1;
  Time1 = Time2;
  Time2 = tim1;

}

/*******************************************************************
 * track()
 *
 * performs tracking algorithm from previous scan to this scan
 *
 *******************************************************************/

static void track(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		  si32 scan_num)

{

  si32 jstorm;
  si32 entry_offset, first_entry_offset;
  long d_secs;
  double d_hours;

  PMU_auto_register("Tracking");

  /*
   * compute delta time in hours and mins
   */

  d_secs = Time2.unix_time - Time1.unix_time;
  d_hours = (double) d_secs / 3600.0;
  
  /*
   * print to stdout
   */
  
  fprintf(stdout, "============ tracking ============\n");
  fprintf(stdout, "Tracking scan %ld to scan %ld\n",
	  (long) (scan_num - 1), (long) scan_num);
  fprintf(stdout, "Time 1: %2d/%2d/%2d %2d:%2d:%2d, nstorms %ld\n",
	  Time1.year, Time1.month, Time1.day,
	  Time1.hour, Time1.min, Time1.sec, (long) Nstorms1);
  fprintf(stdout, "Time 2: %2d/%2d/%2d %2d:%2d:%2d, nstorms %ld\n",
	  Time2.year, Time2.month, Time2.day,
	  Time2.hour, Time2.min, Time2.sec, (long) Nstorms2);
  fprintf(stdout, "Dtime (secs) : %ld\n", (long) d_secs);
  fprintf(stdout, "\n");
  fflush (stdout);
  
  if (d_secs <= 0) {
    fprintf(stderr, "ERROR - %s:track\n", Glob->prog_name);
    fprintf(stderr, "Illegal condition - Dtime <= 0\n");
    tidy_and_exit(-1);
  }

  /*
   * match the storms from time1 to time2 - if match_scans returns failure,
   * set track numbers to next available ones, and reinitialize forecast
   */
  
  if (d_secs <= Glob->params.max_delta_time &&
      Nstorms1 != 0 && Nstorms2 != 0) {
      
    /*
     * compute bounding box limits for each current storm
     * and for the forecast position of each previous storm
     */
    
    load_bounds(s_handle, t_handle, Nstorms1, Nstorms2,
		Storms1, Storms2, d_hours);
    
    /*
     * load up overlap areas between polygons from time 1 and
     * polygons at time 2
     */
    
    find_overlaps(s_handle, Nstorms1, Nstorms2,
		  Storms1, Storms2, d_hours);

    /*
     * check overlap matches for max number of parents and children
     */

    check_matches(Storms1, Storms2, Nstorms1, Nstorms2);

    /*
     * perform optimal match on storms which were not matched
     * using overlaps
     */
     
    match_storms(s_handle, Nstorms1, Nstorms2,
		 Storms1, Storms2, d_hours);
    
    /*
     * resolve matches for those storms which do not have overlaps
     * but had a match identified by match_storms()
     */
    
    resolve_matches(Storms1, Storms2, Nstorms1, Nstorms2);

    /*
     * consolidate complex tracks
     */
    
    consolidate_complex_tracks(t_handle, Storms1, Storms2,
			       Nstorms1, Nstorms2, Track_utime);
    
    if (Glob->params.debug >= DEBUG_EXTRA) {
      print_matches(Storms1, Storms2, Nstorms1, Nstorms2);
    }

  } else {

    for (jstorm = 0; jstorm < Nstorms2; jstorm++) {
      Storms2[jstorm].starts = TRUE;
    }
    
  } /* if (d_secs ... */

  /*
   * set header invalid in case program fails before writes are
   * complete
   */

  set_header_invalid(t_handle);
  
  /*
   * set up the tracks at time 2
   */

  update_tracks(s_handle,
		t_handle,
		&Time2,
		scan_num,
		d_hours,
		Track_utime,
		Nstorms1, Nstorms2,
		Storms1, Storms2,
		Track_continues);
  
  /*
   * update the track times and compute the forecasts
   */
    
  for (jstorm = 0; jstorm < Nstorms2; jstorm++) {
    
    update_times(&Time2, Track_utime, Storms2, jstorm);
    compute_forecast(s_handle, Storms2, jstorm);
    
  } /* jstorm */

  if (Nstorms2 > 0) {

    /*
     * smooth the speed and direction forecasts
     */
    
    smooth_spatial_forecasts(s_handle, Storms2, Nstorms2);
    
    /*
     * compute speed and direction
     */
    
    compute_speed_and_dirn(s_handle, Storms2, Nstorms2);

  }
  
  /*
   * write the tracks
   */
    
  for (jstorm = 0; jstorm < Nstorms2; jstorm++) {
    
    entry_offset = write_track(s_handle,
			       t_handle,
			       &Time2,
			       Track_utime,
			       Storms2,
			       jstorm,
			       scan_num);

    if (jstorm == 0)
      first_entry_offset = entry_offset;
    
  } /* jstorm */
    
  if (RfAllocTrackScanIndex(t_handle, scan_num + 1, "track"))
    tidy_and_exit(-1);

  t_handle->scan_index[scan_num].utime = Time2.unix_time;
  t_handle->scan_index[scan_num].n_entries = Nstorms2;
  t_handle->scan_index[scan_num].first_entry_offset = first_entry_offset;
  
  /*
   * set header valid and write to file
   */
    
  set_header_valid(t_handle, scan_num);

  if (Glob->params.debug >= DEBUG_NORM)
    umalloc_count();

}

/*******************************************************************
 * unlock_header_file()
 *
 * clear write lock on header file
 */

static void unlock_header_file(track_file_handle_t *t_handle)

{

  if (ta_unlock_file(t_handle->header_file_path,
		     t_handle->header_file)) {
    fprintf(stderr, "ERROR - %s:perform_tracking\n", Glob->prog_name);
    tidy_and_exit(-1);
  }
  
}

/****************************************************************
 * remove_current_state_file()
 *
 * Remove the current state file
 */

void remove_current_state_file(void)

{

  fprintf(stderr, "WARNING - %s:remove_current_state_file\n",
	  Glob->prog_name);
  fprintf(stderr, "Removing current state file '%s'\n",
	  State_file_path);
  if (unlink(State_file_path)) {
    if (Glob->params.debug >= DEBUG_NORM) {    
      fprintf(stderr, "WARNING - %s:remove_current_state_file\n",
	      Glob->prog_name);
      fprintf(stderr, "Cannot remove state file\n");
      perror(State_file_path);
    }
  }

}



