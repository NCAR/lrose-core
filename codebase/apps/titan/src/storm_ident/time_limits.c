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
/******************************************************************************
 * time_limits.c
 *
 * Routines for setting and getting the time limits for dobson data.
 *
 * Mike Dixon RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1994
 *
 *****************************************************************************/

#include "storm_ident.h"
#include <toolsa/ldata_info.h>
#include <dirent.h>
#include <assert.h>
#include <sys/stat.h>

#define DIR_NAME_LEN 8

static si32 Start_time = 0;
static si32 Restart_time = 0;
static si32 End_time = 0;
static time_t Storm_file_date;

static si32 get_start_time_from_storm_file(LDATA_handle_t *ldata);
static void set_time_limits_archive(void);
static void set_time_limits_realtime(void);
static si32 search_backwards_for_start_time(si32 end_time);
static void set_restart_time(void);

/*------------------
 * get_dobson_data_time()
 */

si32 get_dobson_data_time(void)

{
  
  static int first_call = TRUE;
  static LDATA_handle_t ldata;

  if (first_call) {
    LDATA_init_handle(&ldata, Glob->prog_name, FALSE);
    first_call = FALSE;
  }
  
  LDATA_info_read_blocking(&ldata,
			   Glob->params.rdata_dir,
			   Glob->params.max_realtime_valid_age,
			   2000,
			   PMU_auto_register);

  return (ldata.info.latest_time);
  
}

/*-------------------
 * get_restart_time()
 */

si32 get_restart_time(void)

{

  assert(Restart_time != 0);
  return (Restart_time);

}
     
/*--------------------------
 * get_startup_time_limits()
 */

void get_startup_time_limits(si32 *start_time_p, si32 *end_time_p)
     
{

  if (Glob->params.mode == TDATA_ARCHIVE) {

    set_time_limits_archive();

  } else {

    set_time_limits_realtime();

  }

  if (Glob->params.debug >= DEBUG_NORM) {

    fprintf(stderr, "Prep data start time: %s\n",
	    utimstr(Start_time));

    fprintf(stderr, "Prep data end time: %s\n",
	    utimstr(End_time));

    fprintf(stderr, "Storm file date: %s\n",
	    utimstr(Storm_file_date));

  }

  *start_time_p = Start_time;
  *end_time_p = End_time;
  return;

}

/*----------------------
 * get_storm_file_date()
 */

void get_storm_file_date(time_t *storm_file_date_p)

{
  *storm_file_date_p = Storm_file_date;
  return;
}

/*----------------------
 * set_storm_file_date()
 */

void set_storm_file_date(si32 date)

{
  Storm_file_date = date;
  return;
}

/*--------------------------
 * set_time_limits_archive()
 */

static void set_time_limits_archive(void)

{

  if (Glob->start_time != 0 && Glob->end_time != 0) {
    
    Start_time = Glob->start_time;
    End_time = Glob->end_time;
    
  } else if (Glob->ref_time != 0) {
    
    Start_time = search_backwards_for_start_time(Glob->ref_time);
    End_time = Glob->ref_time;
    
  } else {

    fprintf(stderr, "ERROR - %s:set_time_limits_archive\n", Glob->prog_name);
    fprintf(stderr, "Must set start and end times in archive mode\n");
    tidy_and_exit(-1);
    
  }
  
  set_storm_file_date(Start_time);

  return;

}

/*--------------------------
 * set_time_limits_realtime()
 */

static void set_time_limits_realtime(void)

{

  static int first_call = TRUE;
  static LDATA_handle_t ldata;
  date_time_t dtime;
  
  if (first_call) {
    LDATA_init_handle(&ldata, Glob->prog_name, FALSE);
    first_call = FALSE;
  }
  
  /*
   * get latest data time, set end time to this
   */
  
  LDATA_info_read_blocking(&ldata,
			   Glob->params.rdata_dir,
			   Glob->params.max_realtime_valid_age,
			   2000,
			   PMU_auto_register);

  End_time = ldata.info.latest_time;

  /*
   * if a storm file exists for this time, get the start time
   * from that  file
   */
  
  Start_time = get_start_time_from_storm_file(&ldata);
  
  /*
   * if this was successful, return now
   */
  
  if (Start_time > 0) {
    set_storm_file_date(End_time);
    set_restart_time();
    return;
  }

  if (Glob->params.auto_restart) {

    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "Auto_restart - computing start time\n");
    }

    /*
     * start time is restart_overlap_time before the start of
     * current day
     */

    dtime = ldata.ltime;
    dtime.hour = 0;
    dtime.min = 0;
    dtime.sec = 0;
    uconvert_to_utime(&dtime);
    Start_time = dtime.unix_time - Glob->params.restart_overlap_period;
    set_storm_file_date(End_time);
    set_restart_time();
    
  } else {

    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr,
	      "Not auto_restart - searching for start time in data\n");
    }

    Start_time = search_backwards_for_start_time(End_time);
    set_storm_file_date(Start_time);
    set_restart_time();
    
  }
  
}

/*---------------------------------
 * get_start_time_from_storm_file()
 */

static si32 get_start_time_from_storm_file(LDATA_handle_t *ldata)

{

  char header_file_path[MAX_PATH_LEN];
  si32 start_time;
  storm_file_handle_t s_handle;
  struct stat hdr_stat;

  /*
   * set up file path for file
   */
  
  sprintf(header_file_path, "%s%s%.4d%.2d%.2d.%s",
	  Glob->params.storm_data_dir,
	  PATH_DELIM,
	  ldata->ltime.year, ldata->ltime.month, ldata->ltime.day,
	  STORM_HEADER_FILE_EXT);
  
  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "get_start_time_from_storm_file\n");
    fprintf(stderr, "Searching for storm file: %s\n", header_file_path);
  }
  
  /*
   * initialize new storm file handle
   */
  
  RfInitStormFileHandle(&s_handle, Glob->prog_name);

  /*
   * open new storm properties file
   */

  if (stat(header_file_path, &hdr_stat) < 0) {
    /* file does not exist */
    return (-1L);
  }
  
  if (RfOpenStormFiles (&s_handle, "r",
			header_file_path,
			STORM_DATA_FILE_EXT,
			"get_start_time_from_storm_file")) {

    return (-1L);
  }

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "Found and opened file\n");
  }
  
  /*
   * read in header
   */
  
  if (RfReadStormHeader(&s_handle, "get_start_time_from_storm_file")) {
    return (-1L);
  }

  /*
   * determine start_time
   */

  start_time = s_handle.header->start_time;

  /*
   * close file and free up
   */

  RfCloseStormFiles (&s_handle,
		     "get_start_time_from_storm_file");

  RfFreeStormHeader(&s_handle, "get_start_time_from_storm_file");
  RfFreeStormFileHandle(&s_handle);

  return (start_time);

}

/*----------------------------------
 * search_backwards_for_start_time()
 *
 * This routine searches back through the data files on disk for the 
 * first break in the data which exceeds the max_missing_data_gap.
 * 
 * The method is as follows: a day is divided up into a number of
 * periods, the period length being max_missing_data_gap. When a 
 * file is found, it is placed in the relevant period. If no files
 * fall in a given period, there must be a gap at least as long
 * as the period length.
 * The start time of the most recent such period is returned as the
 * start time for the algorithm.
 */

typedef struct {
  int have_data;
  si32 start_time;
} time_period_t;

static si32 search_backwards_for_start_time(si32 end_time)

{

  char dirname[MAX_PATH_LEN];
  char format_str[20];

  si32 iperiod, nperiods;
  si32 ijul, julday;
  si32 nfiles;
  si32 period_len;
  si32 latest_start_time;

  date_time_t d_end_time;
  date_time_t start_of_day;
  date_time_t file_time;

  DIR *dirp;
  struct dirent	*dp;
  time_period_t *periods;

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "search_backwards_for_start_time\n");
  }
  
  /*
   * initialize periods - if the division is not exact ignore the last
   * period in the day because it will be shorter than the rest
   */

  period_len = Glob->params.max_missing_data_gap;
  nperiods = 86400 / period_len;
  periods = (time_period_t *) umalloc
    ((ui32) (nperiods * sizeof(time_period_t)));

  latest_start_time = end_time;

  /*
   * set up format string
   */

  sprintf(format_str, "%s%s", "%2d%2d%2d.", Glob->params.rdata_file_ext);

  /*
   * move back through julian dates
   */

  d_end_time.unix_time = end_time;
  uconvert_from_utime(&d_end_time);
  julday = ujulian_date(d_end_time.day,
			d_end_time.month,
			d_end_time.year);

  for (ijul = julday; ijul >= 0; ijul--) {

    /*
     * compute the calendar date for this julian date
     */
    
    ucalendar_date(ijul,
		   &file_time.day, &file_time.month, &file_time.year);
    
    /*
     * compute the start_of_day time
     */
    
    start_of_day = file_time;
    start_of_day.hour = 0;
    start_of_day.min = 0;
    start_of_day.sec = 0;
    uconvert_to_utime(&start_of_day);

    /*
     * initialize the periods array
     */

    for (iperiod = 0; iperiod < nperiods; iperiod++) {
      periods[iperiod].have_data = FALSE;
      periods[iperiod].start_time =
	start_of_day.unix_time + iperiod * period_len;
    } /* iperiod */
    
    /*
     * compute directory name for data with this date
     */
    
    sprintf(dirname, "%s%s%.4d%.2d%.2d",
	    Glob->params.rdata_dir, PATH_DELIM,
	    file_time.year, file_time.month, file_time.day);
    
    /*
     * read through the directory to get the file names
     */

    if ((dirp = opendir (dirname)) == NULL) {
      goto done;
    }

    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "searching dir %s\n", dirname);
    }
  
    nfiles = 0;

    for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {

      /*
       * exclude dir entries and files beginning with '.'
       */

      if (dp->d_name[0] == '.') {
	continue;
      }

      /*
       * check that the file name is in the correct format
       */
      
      if (sscanf(dp->d_name, format_str,
		 &file_time.hour, &file_time.min, &file_time.sec) != 3) {

	if (Glob->params.debug >= DEBUG_NORM) {
	  fprintf(stderr,
		  "WARNING - %s:search_backwards_for_start_time\n",
		  Glob->prog_name);
	  fprintf(stderr, "File name '%s' invalid\n", dp->d_name);
	}
	continue;
	
      }
      
      if (!uvalid_datetime(&file_time)) {
	
	if (Glob->params.debug >= DEBUG_NORM) {
	  fprintf(stderr,
		  "WARNING - %s:search_backwards_for_start_time\n",
		  Glob->prog_name);
	  fprintf(stderr, "File name '%s' invalid\n", dp->d_name);
	}
	continue;
	
      }
      
      /*
       * file name is in correct format, therefore accept it.
       * Compute the file time
       */

      uconvert_to_utime(&file_time);

      /*
       * compute the period into which this file time falls
       */

      iperiod =
	(file_time.unix_time - start_of_day.unix_time) / period_len;

      if (iperiod > nperiods - 1) {
	iperiod = nperiods - 1;
      }

      periods[iperiod].have_data = TRUE;

      nfiles++;

    } /* ifile */
  
    /*
     * if no files have been found, return latest start time found
     */

    if (nfiles == 0) {
      goto done;
    }
    
    /*
     * close the directory file
     */
    
    closedir(dirp);
    
    /*
     * search for latest period without data
     */

    for (iperiod = nperiods - 1; iperiod >= 0; iperiod--) {

      if (periods[iperiod].start_time <= end_time &&
	  !periods[iperiod].have_data) {
	latest_start_time = periods[iperiod].start_time;
	goto done;
      }

    }

    latest_start_time = start_of_day.unix_time;

  } /* ijul */

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "start_time : %s\n", utimstr(latest_start_time));
  }
  
 done:
  ufree ((char *) periods);
  return (latest_start_time);
  
}

static void set_restart_time(void)

{

  date_time_t rtime;

  /*
   * start with the restart interval plus 1 day
   */

  if (Glob->params.restart_no_delay) {
    rtime.unix_time =
      Start_time + Glob->params.restart_overlap_period;
  } else {
    rtime.unix_time =
      Start_time + Glob->params.restart_overlap_period + 86400;
  }
  uconvert_from_utime(&rtime);
  
  /*
   * set the hour and min
   */
  
  rtime.hour = Glob->params.restart_time.hour;
  rtime.min = Glob->params.restart_time.min;
  rtime.sec = 0;
  uconvert_to_utime(&rtime);

  /*
   * if less that 1 day after start time, increase by 1 day
   */

  if (!Glob->params.restart_no_delay &&
      (rtime.unix_time - Start_time < 86400)) {
    rtime.unix_time += 86400;
  }

  Restart_time = rtime.unix_time;
  
  if (Glob->params.debug) {
    fprintf(stderr, "Start_time: %s\n", utimstr(Start_time ));
    fprintf(stderr, "Restart_time: %s\n", utimstr(Restart_time ));
    fprintf(stderr, "Overlap_period (secs): %d\n",
	    (int) Glob->params.restart_overlap_period);
  }

}

