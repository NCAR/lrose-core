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
 * file_search.c
 *
 * file-related subroutines for servers
 *
 * Public routines are:
 *
 *  void file_search_debug(int dflag)
 *
 *  int get_latest_file(char *dir_path,
 *                      char *file_path)
 *
 *  int find_best_file (si32 min_time,
 *                      si32 target_time,
 * 		        si32 max_time,
 * 		        si32 ntop_dir,
 * 		        char **top_dir,
 * 		        char *file_path)
 * 
 * Frank Hage, RAP, NCAR, Boulder, CO, USA
 *
 * Updated by Mike Dixon, Feb 1993
 */

#include "DobsonServer.h"
#include <toolsa/ldata_info.h>
#include <dirent.h>

#define LARGE_LONG 2000000000L

static int Debug = 0;

/*
 * prototypes
 */

static int get_best(si32 min_time,
		    si32 target_time,
		    si32 max_time,
		    si32 *time_error,
		    char *dir_path,
		    char *file_path);

static int get_latest_dir(si32 ntop_dir,
			  char **top_dir,
			  char *dir_path);

/*****************************************************************
 * file_search_debug()
 *
 * set debugging
 */

void file_search_debug(int dflag)
{

  Debug = dflag;

}

/*****************************************************************
 * GET_LATEST_FILE: Find the latest file under the named dir
 *
 * Returns 0 on success, -1 on failure
 */

int get_latest_file(si32 ntop_dir,
		    char **top_dir,
		    char *file_path)

{

  static int first_call = TRUE;
  static LDATA_handle_t ldata;
  
  char ext[16];
  char dir_path[MAX_PATH_LEN];
  char file_name[MAX_PATH_LEN];
  
  long idir;
  long hour, min, sec;
  si32 f_count;

  struct dirent *dp;
  DIR *dirp;

  /*
   * initialize latest data handle on first call
   */

  if (first_call) {
    LDATA_init_handle(&ldata, Glob->prog_name, Debug);
    first_call = FALSE;
  }
  
  /*
   * if we can get latest data info, read it and set the latest file
   * path based on the latest time
   */

  for (idir = 0; idir < ntop_dir; idir++) {
    
    if (LDATA_info_read(&ldata, top_dir[idir], -1) == 0) {

      sprintf(file_path,
	      "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	      top_dir[idir], PATH_DELIM,
	      ldata.ltime.year, ldata.ltime.month, ldata.ltime.day,
	      PATH_DELIM,
	      ldata.ltime.hour, ldata.ltime.min, ldata.ltime.sec,
	      ldata.info.file_ext);
      
      if (Debug)
	fprintf(stderr,
		"Using index file to get file path '%s'\n", file_path);
      
      return (0);

    } /* if (cdata_read_index_simple ... ) */
    
  } /* idir */

  /*
   * no latest data info file, so get most recent directory
   */
  
  if(get_latest_dir(ntop_dir,
		    top_dir,
		    dir_path)) /* returns dir_path*/
    return (-1);
  
  if((dirp = opendir(dir_path)) == NULL) {
    if (Debug)
      fprintf(stderr,
	      "get_latest_file - couldn't open directory: %s\n", dir_path);
    return (-1);
  } /* if((dirp ... */
  
  /*
   * Loop thru directory looking for the data file names
   */
  
  f_count = 0;
  
  memset ((void *) file_name,
          (int) 0, (size_t) MAX_PATH_LEN);

  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    /*
     * exclude dir entries and files beginning with '.'
     */

    if (dp->d_name[0] == '.')
      continue;
      
    /*
     * check that the dir name is in the correct format
     */
    
    if (sscanf(dp->d_name, "%2ld%2ld%2ld.%s",
	       &hour, &min, &sec, ext) != 4)
      continue;
      
    if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	sec < 0 || sec > 59)
      continue;

    /*
     * name is in correct format. Therefore, accept it
     */
    
    if (strcmp(dp->d_name, file_name) > 0) {
      strcpy(file_name, dp->d_name);
      sprintf(file_path, "%s/%s",
	      dir_path, file_name);
    }
    
    f_count++;
    
  } /* dp */
  
  closedir(dirp);
  
  if (f_count == 0)
    return (-1);
  else
    return (0);
  
}

/*****************************************************************
 * FIND_BEST_FILE: Find the most appropriate file given the
 * time frame requested - Looks for files closest to the
 * time centroid
 */

int find_best_file (si32 min_time,
		    si32 target_time,
		    si32 max_time,
		    si32 ntop_dir,
		    char **top_dir,
		    char *file_path)

{

  char dir_path[MAX_PATH_LEN];

  int file_found;

  long idir, iday;
  si32 time_error;
  si32 ndays;
  
  date_time_t this_dt, start_dt, end_dt;
  
  time_error = LARGE_LONG;
  file_found = FALSE;
  
  start_dt.unix_time = min_time;
  uconvert_from_utime(&start_dt);
  end_dt.unix_time = max_time;
  uconvert_from_utime(&end_dt);
  
  if (start_dt.day != end_dt.day)
    ndays = 2;
  else
    ndays = 1;
  
  for (iday = 0; iday < ndays; iday++) {
    
    if (iday == 0)
      this_dt.unix_time = min_time;
    else
      this_dt.unix_time = max_time;
    
    /*
     * expand to get date and time
     */
    
    uconvert_from_utime(&this_dt);
    
    /*
     * search through the data directories for a matching file in
     * the correct subdirectory
     */
    
    for (idir = 0; idir < ntop_dir; idir++) {
      
      sprintf(dir_path, "%s/%.4d%.2d%.2d",
	      top_dir[idir],
	      this_dt.year, this_dt.month, this_dt.day);
      
      /*
       * load file name of the closest scan to the requested time
       */
      
      if (get_best(min_time, target_time, max_time, 
		   &time_error,
		   dir_path, file_path) == 0) {
	
	file_found = TRUE;
	
      }
      
    } /* idir */
	  
  } /* iday */

  if (file_found)
    return (0);
  else
    return (-1);
  
}

/*****************************************************************
 * GET_BEST: Find the most appropriate file given the
 * time frame requested and the given directory
 */

static int get_best(si32 min_time,
		    si32 target_time,
		    si32 max_time,
		    si32 *time_error,
		    char *dir_path,
		    char *file_path)

{
  
  char ext[16];
  char *date_str;
  
  si32 f_count;
  si32 time_diff;
  long year, month, day;
  long hour, min, sec;
  
  date_time_t file_dt;
  
  struct dirent *dp;
  DIR *dirp;
  
  /*
   * open directory - return -1 if unable to open
   */
  
  if ((dirp = opendir(dir_path)) == NULL)
    return (-1);

  /*
   * get date from directory name
   */

  date_str = dir_path + (strlen(dir_path) - 8);

  if (sscanf(date_str, "%4ld%2ld%2ld", &year, &month, &day) != 3)
    return (-1);
  
  if (year < 1970 || year > 2100 || month < 1 || month > 12 ||
      day < 1 || day > 31)
    return (-1);
  
  /*
   * Loop through directory looking for the data file names
   */
  
  f_count = 0;
  
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    /*
     * exclude dir entries and files beginning with '.'
     */

    if (dp->d_name[0] == '.')
      continue;
      
    /*
     * check that the dir name is in the correct format
     */

    if (sscanf(dp->d_name, "%2ld%2ld%2ld.%s",
	       &hour, &min, &sec, ext) != 4)
      continue;
      
    if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	sec < 0 || sec > 59)
      continue;

    /*
     * file name is in correct format. Therefore, accept it
     */
    
    file_dt.year = year;
    file_dt.month = month;
    file_dt.day = day;
    file_dt.hour = hour;
    file_dt.min = min;
    file_dt.sec = sec;
	
    uconvert_to_utime(&file_dt);
    
    if (file_dt.unix_time >= min_time &&
	file_dt.unix_time <= max_time) {
      
      time_diff = abs ((int) (file_dt.unix_time - target_time));
	  
      if (time_diff < *time_error) {
	    
	*time_error = time_diff;
	sprintf(file_path, "%s/%s", dir_path, dp->d_name);
	f_count++;
	
      } /*if (time_diff < *time_error) */
	  
    } /* if (file_dt.unix_time >= min_time ... */

  } /* dp */
  
  closedir(dirp);
  
  if(f_count == 0)
    return (-1);
  else
    return (0);
  
}

/*****************************************************************
 * GET_LATEST_DIR
 *
 * Find the Latest Directory under the data dir names.
 * In this scheme the directories are named "YYYYMMDD" where
 * YYYY - Year MM- month -DD day
 *
 * Searches through the directories given in top_dir array.
 *
 * returns 0 on success, -1 on failure
 */

static int get_latest_dir(si32 ntop_dir,
			  char **top_dir,
			  char *dir_path)

{

  char dir_name[MAX_PATH_LEN];

  long idir;
  si32 d_count;
  long year, month, day;
  
  struct dirent *dp;
  DIR *dirp;
  
  d_count = 0;

  memset ((void *) dir_name,
          (int) 0, (size_t) MAX_PATH_LEN);

  for (idir = 0; idir < ntop_dir; idir++) {
    
    if((dirp = opendir(top_dir[idir])) == NULL) {
      
      if (Debug)
	fprintf(stderr,"get_latest_dir - couldn't open directory: %s\n",
		top_dir[idir]);
      
      continue;
    
    } /* if((dirp ... */
    
    /*
     * Loop thru directory looking for the directory names
     */
    
    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
      
      if (dp->d_name[0] == '.')
	continue;
      
      if (sscanf(dp->d_name, "%4ld%2ld%2ld",
		 &year, &month, &day) != 3)
	continue;
      
      if (year < 1970 || year > 2100 || month < 1 || month > 12 ||
	  day < 1 || day > 31)
	continue;
    
      /*
       * correct format, so check if this is the latest dir
       */
      
      if (strcmp(dp->d_name, dir_name) > 0) {
	
	strcpy(dir_name, dp->d_name);
	
	sprintf(dir_path, "%s/%s",
		top_dir[idir], dir_name);
	
	d_count++;
      
      }
    
    } /* dp */
    
    closedir(dirp);

  } /* idir */
    
  if (d_count == 0)
    return (-1);
  else
    return (0);
    
}

