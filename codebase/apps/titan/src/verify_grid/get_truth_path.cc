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
  * get_truth_path.c
  *
  * Gets path for truth file closest in time to the detect file
  * plus the time lag
  *
  * RAP, NCAR, Boulder CO
  *
  * July 1992
  *
  * Mike Dixon
  *
  *********************************************************************/
  
#include "verify_grid.h"
#include <dirent.h>

static int find_best_file(char *dir_path,
			  char *file_path,
			  si32 start_time,
			  si32 end_time,
			  date_time_t *dir_dt,
			  si32 search_time,
			  si32 *time_error);

char *get_truth_path (char *detect_file_path)

{

  static char truth_file_path[MAX_PATH_LEN * 2];

  char dir_path[MAX_PATH_LEN];
  char *name, file_ext[16];
  char *last_delim;

  int file_found;

  si32 time_error;
  si32 iday, ndays;
  si32 mid_time;

  date_time_t ft;
  date_time_t start_dt, end_dt, *this_dt;

  /*
   * parse the detect file path to get date and time
   */

  if ((last_delim = strrchr(detect_file_path, '/')) == NULL) {
    
    fprintf(stderr, "ERROR - %s:get_truth_path\n", Glob->prog_name);
    fprintf(stderr, "File paths do not include the date directories\n");
    fprintf(stderr, "Example is '%s'\n", detect_file_path);
    fprintf(stderr,
	    "Run the program from the detect data directory or above\n");
    tidy_and_exit(-1);

  }
  
  name = last_delim - 8;

  if (sscanf(name, "%4d%2d%2d/%2d%2d%2d.%s",
	     &ft.year, &ft.month, &ft.day,
	     &ft.hour, &ft.min, &ft.sec, file_ext) != 7) {

    fprintf(stderr, "ERROR - %s:get_truth_path\n", Glob->prog_name);
    fprintf(stderr, "Cannot process file path '%s'\n",
	    detect_file_path);
    tidy_and_exit(-1);

  }

  uconvert_to_utime(&ft);

  mid_time = ft.unix_time + Glob->params.time_lag;
  start_dt.unix_time = mid_time - Glob->params.time_margin;
  uconvert_from_utime(&start_dt);
  end_dt.unix_time = mid_time + Glob->params.time_margin;
  uconvert_from_utime(&end_dt);

  time_error = LARGE_LONG;
  file_found = FALSE;

  if (start_dt.day != end_dt.day)
    ndays = 2;
  else
    ndays = 1;

  for (iday = 0; iday < ndays; iday++) {
      
    if (iday == 0)
      this_dt = &start_dt;
    else
      this_dt = &end_dt;
      
    /*
     * search through the data directory for a matching file in
     * the correct subdirectory
     */
    
    snprintf(dir_path, MAX_PATH_LEN - 1, "%s/%.4d%.2d%.2d",
             Glob->params.truth_data_dir,
             this_dt->year, this_dt->month, this_dt->day);
	
    /*
     * load file name of the closest scan to the requested time
     */
    
    if(find_best_file(dir_path, truth_file_path,
		      start_dt.unix_time, end_dt.unix_time, this_dt,
		      mid_time, &time_error) == 0) {
      file_found = TRUE;
    }
    
  } /* iday */
  
  if (file_found)
    return (truth_file_path);
  else
    return ((char *) NULL);
    
}  

/*****************************************************************
 * find_best_file()
 *
 * Find the most appropriate file given the
 * time frame requested
 */

static int find_best_file (char *dir_path,
			   char *file_path,
			   si32 start_time,
			   si32 end_time,
			   date_time_t *dir_dt,
			   si32 search_time,
			   si32 *time_error)

{
  
  char file_ext[16];
  si32 f_count;
  si32 time_diff;
  si32 hour, min, sec;
  
  date_time_t file_dt;
  
  struct dirent *dp;
  DIR *dirp;
  
  /*
   * open directory - return -1 if unable to open
   */
  
  if ((dirp = opendir(dir_path)) == NULL)
    return (-1);
  
  /*
   * Loop thru directory looking for the data file names
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

    if (sscanf(dp->d_name, "%2d%2d%2d.%s",
	       &hour, &min, &sec, file_ext) != 4)
      continue;
      
    if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	sec < 0 || sec > 59)
      continue;

    /*
     * file name is in correct format. Therefore, accept it
     */
    
    file_dt.year = dir_dt->year;
    file_dt.month = dir_dt->month;
    file_dt.day = dir_dt->day;
    file_dt.hour = hour;
    file_dt.min = min;
    file_dt.sec = sec;
	
    uconvert_to_utime(&file_dt);
	
    if (file_dt.unix_time >= start_time &&
	file_dt.unix_time <= end_time) {
	  
      time_diff = abs ((int) (file_dt.unix_time - search_time));
	  
      if (time_diff < *time_error) {
        
	*time_error = time_diff;
	snprintf(file_path, MAX_PATH_LEN * 2 - 1, "%s%s%s",
                 dir_path, PATH_DELIM, dp->d_name);
	f_count++;
	    
      } /*if (time_diff < *time_error) */
	  
    } /* if (file_dt.unix_time >= com->time_min ... */

  } /* dp */
  
  closedir(dirp);
  
  if(f_count == 0)
    return (-1);
  else
    return (0);
  
}

