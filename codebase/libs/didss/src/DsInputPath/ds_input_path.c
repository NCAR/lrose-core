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
/*********************************************************
 * ds_input_path.c : Input data system file handling
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * March 1998
 *
 *********************************************************
 *
 * See <didss/ds_input_path.h> for details.
 *
 *********************************************************/

#include <time.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <didss/ds_input_path.h>
#include <toolsa/os_config.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>


/*
 * Static function prototypes.
 */

static int find_next_path(DSINP_handle_t *handle,
			  char *search_dir,
			  time_t *firstMtime);

static void add_dataset_time(time_t gen_time, time_t forecast_time);

static int compare_paths(const void *v1, const void *v2);

static void load_day(DSINP_handle_t *handle,
		     char *subdir_path,
		     date_time_t *midday,
		     time_t start_time,
		     time_t end_time);


/*
 * Global constants.
 */

const int Forever = TRUE;


/*
 * Global variables.
 */

DSINP_dataset_time_t *DataTimes = (DSINP_dataset_time_t *)NULL;
int DataTimesAlloc = 0;
int DataTimesUsed = 0;
const int DataTimesIncr = 10;


/*****************************
 * Constructor - Archive mode
 *
 * Pass in a list of file paths.
 */

void DSINP_create_archive_list(DSINP_handle_t *handle,
			       char *prog_name,
			       int debug,
			       int n_files,
			       char **file_paths)
{
  int i;
  
  /*
   * initialize
   */

  handle->prog_name = STRdup(prog_name);
  handle->debug = (debug? 1 : 0);
  
  handle->file_num = 0;
  handle->n_files = n_files;
  handle->mode = DSINP_ARCHIVE_MODE;
  
  /*
   * set up membuf for file paths array
   */

  handle->mbuf_paths = MEMbufCreate();
  for (i = 0; i < handle->n_files; i++)
  {
    char *path = (char *) umalloc(strlen(file_paths[i]) + 1);
    strcpy(path, file_paths[i]);
    MEMbufAdd(handle->mbuf_paths, &path, sizeof(char *));
  }
  handle->file_paths = (char **) MEMbufPtr(handle->mbuf_paths);

  /*
   * sort the paths
   */

  qsort(handle->file_paths, handle->n_files, sizeof(char *),
	compare_paths);

}

/*****************************
 * Constructor - Archive mode
 *
 * Pass in data directory and start and end times.
 */

void DSINP_create_archive_time(DSINP_handle_t *handle,
			       char *prog_name,
			       int debug,
			       char *input_dir,
			       time_t start_time,
			       time_t end_time)
{
  int start_day;
  int end_day;
  int iday;
  
  /*
   * initialize
   */

  handle->prog_name = STRdup(prog_name);
  handle->debug = (debug? 1 : 0);
  
  handle->input_dir = STRdup(input_dir);
  handle->file_num = 0;
  handle->n_files = 0;
  handle->mode = DSINP_ARCHIVE_MODE;
  
  /*
   * set up membuf for file paths array
   */

  handle->mbuf_paths = MEMbufCreate();

  /*
   * compute start and end day
   */

  start_day = start_time / 86400;
  end_day = end_time / 86400;
  
  /*
   * loop through days
   */

  for (iday = start_day; iday <= end_day; iday++)
  {
    date_time_t midday;
    char subdir_path[MAX_PATH_LEN];
    
    midday.unix_time = iday * 86400 + 43200;
    uconvert_from_utime(&midday);

    /*
     * compute subdir path
     */

    sprintf(subdir_path, "%s%s%.4d%.2d%.2d",
	    input_dir, PATH_DELIM,
	    midday.year, midday.month, midday.day);

    /*
     * load up file paths for this day
     */

    load_day(handle, subdir_path, &midday, start_time, end_time);

  } /* endfor - iday */
  
  handle->file_paths = (char **) MEMbufPtr(handle->mbuf_paths);

  /*
   * sort the paths
   */

  qsort(handle->file_paths, handle->n_files, sizeof(char *),
	compare_paths);

}

/*****************************
 * Constructor - realtime mode
 *
 * Pass in (a) the input directory to be watched.
 *         (b) the max valid age for a realtime file (secs)
 *             the routine will wait for a file with the age
 *             less than this.
 *         (c) pointer to heartbeat_func. If NULL this is ignored.
 *             If non-NULL, this is called once per second while
 *             the routine is waiting for new data.
 */

void DSINP_create_realtime(DSINP_handle_t *handle,
			   char *prog_name,
			   int debug,
			   char *input_dir,
			   int max_valid_age,
			   DsInput_heartbeat_t heartbeat_func)
{
  /*
   * initialize
   */

  handle->prog_name = STRdup(prog_name);
  handle->debug = (debug? 1 : 0);
  
  handle->input_dir = STRdup(input_dir);
  handle->max_age = max_valid_age;
  handle->heartbeat_func = heartbeat_func;
  handle->mode = DSINP_REALTIME_MODE;
  handle->use_ldata_info = FALSE;
  handle->latest_time_used = -1;
  
  /*
   * init latest data handle
   */
  
  LDATA_init_handle(&handle->last_data, handle->prog_name, handle->debug);
}

/*****************************************************************
 * Constructor - realtime mode version 2 - with non_ldata option.
 *
 * Pass in (a) the input directory to be watched.
 *         (b) the max valid age for a realtime file (secs)
 *             the routine will wait for a file with the age
 *             less than this.
 *         (c) pointer to heartbeat_func. If NULL this is ignored.
 *             If non-NULL, this is called once per second while
 *             the routine is waiting for new data.
 *         (d) use_ldata_info flag. If true, we use the latest_data_info
 *             file, if false we watch the directory recursively
 *             for new files.
 *         (e) latest_file_only flag. Only applies if use_ldata_info is
 *             false. If set, the routine returns the latest file.
 *             If false, it returns the earliest file which is younger than
 *             the max valid age and which has not been used yet.
 */

void DSINP_create_realtime_2(DSINP_handle_t *handle,
			     char *prog_name,
			     int debug,
			     char *input_dir,
			     int max_valid_age,
			     DsInput_heartbeat_t heartbeat_func,
			     int use_ldata_info,
			     int latest_file_only)

{

  /*
   * initialize
   */

  handle->prog_name = STRdup(prog_name);
  handle->debug = (debug? 1 : 0);
  
  handle->input_dir = STRdup(input_dir);
  handle->max_age = max_valid_age;
  handle->heartbeat_func = heartbeat_func;
  handle->mode = DSINP_REALTIME_MODE;
  handle->use_ldata_info = use_ldata_info;
  handle->latest_file_only = latest_file_only;
  handle->latest_time_used = -1;

  /*
   * init latest data handle
   */
  
  LDATA_init_handle(&handle->last_data, handle->prog_name, handle->debug);

}

/*****************************
 * Constructor - Triggered mode
 *
 * Pass in data directory.
 */

void DSINP_create_triggered(DSINP_handle_t *handle,
			    char *prog_name,
			    int debug,
			    char *input_dir)
{
  /*
   * initialize
   */

  handle->prog_name = STRdup(prog_name);
  handle->debug = (debug? 1 : 0);
  
  handle->mode = DSINP_TRIGGERED_MODE;
  
  handle->input_dir = STRdup(input_dir);

  handle->max_age = -1;
  
  /*
   * init latest data handle
   */
  
  LDATA_init_handle(&handle->last_data, handle->prog_name, handle->debug);
}

/*****************************
 * Destructor
 */

void DSINP_free(DSINP_handle_t *handle)
{
  int i;
  
  STRfree(handle->prog_name);

  switch(handle->mode)
  {
  case DSINP_ARCHIVE_MODE :
    /*
     * free up paths and membuf
     */

    for (i = 0; i < handle->n_files; i++)
      ufree(handle->file_paths[i]);

    MEMbufDelete(handle->mbuf_paths);

    /*
     * a hack to free up input_dir
     */
    if(handle->input_dir && strlen(handle->input_dir) > 0) {
      STRfree(handle->input_dir);
    }

    break;
    
  case DSINP_REALTIME_MODE :

    STRfree(handle->input_dir);
    
    /*
     * free up latest data info handle
     */

    LDATA_free_handle(&handle->last_data);
    
    break;
    
  case DSINP_TRIGGERED_MODE :
    STRfree(handle->input_dir);
    
    break;
    
  } /* endswitch - handle->mode */

}

/*****************************
 * get closest file to given time within the given time margin
 *
 * On success, returns the name of the closest file.  This pointer
 * points to static memory which should not be freed by the caller.
 * On failure, returns NULL.
 *
 * Returns the data time of the file (based on the file name) in
 * the argument list.
 */

char *DSINP_get_closest(DSINP_handle_t *handle,
			time_t search_time,
			int max_time_offset,
			time_t *data_time)
{
  time_t first_before_time;
  time_t first_after_time;
  
  char *first_before_file_ptr;
  char *first_after_file_ptr;
  
  char first_before_filename[MAX_PATH_LEN];
  char first_after_filename[MAX_PATH_LEN];
  
  /*
   * NOTE:  This is not an efficient implementation of this function,
   * but it works so I'm leaving it for now.
   */

  /*
   * Find the first file before the time.  Copy the data locally
   * because this routine returns a pointer to the same static memory
   * that getFirstAfter() uses.
   */

  first_before_file_ptr =
    DSINP_get_first_before(handle, search_time, max_time_offset,
			   &first_before_time);
  
  if (first_before_file_ptr != (char *)NULL)
    STRcopy(first_before_filename, first_before_file_ptr, MAX_PATH_LEN);
  
  /*
   * Find the first file after the time.  Again, copy the filename.
   */

  first_after_file_ptr = 
       DSINP_get_first_after(handle, search_time, max_time_offset,
			     &first_after_time);
  
  if (first_after_file_ptr != (char *)NULL)
    STRcopy(first_after_filename, first_after_file_ptr, MAX_PATH_LEN);
  
  /*
   * Determine which file to return
   */

  if (first_before_file_ptr == (char *)NULL &&
      first_after_file_ptr == (char *)NULL)
  {
    *data_time = -1;
    return((char *)NULL);
  }
  
  if (first_before_file_ptr == (char *)NULL)
  {
    *data_time = first_after_time;
    STRcopy(handle->input_path, first_after_filename, MAX_PATH_LEN);
    return(handle->input_path);
  }
  
  if (first_after_file_ptr == (char *)NULL)
  {
    *data_time = first_before_time;
    STRcopy(handle->input_path, first_before_filename, MAX_PATH_LEN);
    return(handle->input_path);
  }
  
  if (search_time - first_before_time <= first_after_time - search_time)
  {
    *data_time = first_before_time;
    STRcopy(handle->input_path, first_before_filename, MAX_PATH_LEN);
    return(handle->input_path);
  }
  
  *data_time = first_after_time;
  STRcopy(handle->input_path, first_after_filename, MAX_PATH_LEN);
  return(handle->input_path);
  
}

/*****************************
 * get closest file to given time within the given time margin, wait
 * if there is currently no data within the time margin.
 *
 * On success, returns the name of the closest file.  This pointer
 * points to static memory which should not be freed by the caller.
 * On failure, returns NULL.
 *
 * Returns the data time of the file (based on the file name) in
 * the argument list.
 */

char *DSINP_get_closest_blocking(DSINP_handle_t *handle,
				 time_t search_time,
				 int max_time_offset,
				 DsInput_heartbeat_t heartbeat_func,
				 time_t *data_time)
{
  time_t first_before_time;
  time_t first_after_time;
  
  char *first_before_file_ptr;
  char *first_after_file_ptr;
  
  char first_before_filename[MAX_PATH_LEN];
  char first_after_filename[MAX_PATH_LEN];
  
  /*
   * Find the first file before the time.  Copy the data locally
   * because this routine returns a pointer to the same static memory
   * that getFirstAfter() uses.
   */

  first_before_file_ptr =
    DSINP_get_first_before(handle, search_time, max_time_offset,
			   &first_before_time);
  
  if (first_before_file_ptr != (char *)NULL)
    STRcopy(first_before_filename, first_before_file_ptr, MAX_PATH_LEN);
  
  /*
   * Find the first file after the time.  Again, copy the filename.
   */

  first_after_file_ptr =
    DSINP_get_first_after(handle, search_time, max_time_offset,
			  &first_after_time);
  
  if (first_after_file_ptr != (char *)NULL)
    STRcopy(first_after_filename, first_after_file_ptr, MAX_PATH_LEN);
  
  /*
   * See if we can return one of these files
   */

  if (first_before_file_ptr == (char *)NULL &&
      first_after_file_ptr == (char *)NULL)
  {
    /*
     * continue
     */
  }
  else if (first_before_file_ptr == (char *)NULL)
  {
    *data_time = first_after_time;
    STRcopy(handle->input_path, first_after_filename, MAX_PATH_LEN);
    return(handle->input_path);
  }
  else if (first_after_file_ptr == (char *)NULL ||
           search_time - first_before_time <= first_after_time - search_time)
  {
    *data_time = first_before_time;
    STRcopy(handle->input_path, first_before_filename, MAX_PATH_LEN);
    return(handle->input_path);
  }
  else
  {
    *data_time = first_after_time;
    STRcopy(handle->input_path, first_after_filename, MAX_PATH_LEN);
    return(handle->input_path);
  }
  
  /*
   * Wait for more data until we pass the maximum data time
   */

  handle->last_data.prev_mod_time = -1;
  
  while (Forever)
  {
    if (LDATA_info_read(&handle->last_data, handle->input_dir,
			handle->max_age) == 0)
    {
      if (labs(handle->last_data.info.latest_time - search_time) <
	  max_time_offset)
      {
	*data_time = handle->last_data.info.latest_time;
	STRcopy(handle->input_path,
		LDATA_data_path(&handle->last_data, handle->input_dir),
		MAX_PATH_LEN);
	return(handle->input_path);
      }
    }
    
    if (search_time + max_time_offset < time((time_t *)NULL))
    {
      return((char *)NULL);
    }
    
    if (heartbeat_func != NULL)
      heartbeat_func("DSINP_get_closest_blocking: waiting for data");
    
    sleep(1);
  }
  
  return((char *)NULL);
}

/*****************************
 * get latest data file after the given time, but within the
 * given offset.
 *
 * On success, returns the name of the closest file.  This pointer
 * points to static memory which should not be freed by the caller.
 * On failure, returns NULL.
 *
 * Returns the data time of the file (based on the file name) in
 * the argument list.
 */

char *DSINP_get_first_after(DSINP_handle_t *handle,
			    time_t search_time,
			    int max_time_offset,
			    time_t *data_time)
{
  static char *routine_name = "DSINP_get_first_after()";
  
  int file_found = FALSE;
  
  char file_ext[16];
  int time_diff;
  int min_time_diff = 0;
  int hour, min, sec;
  
  time_t end_time = search_time + max_time_offset;
  
  date_time_t search_dt;
  date_time_t end_dir_dt;
  date_time_t file_dt;
  date_time_t dir_dt;
  
  char dir_path[MAX_PATH_LEN];
  
  struct dirent *dp;
  DIR *dirp;
  
  time_t dir_time;
  time_t return_time = -1;
  
  /*
   * Calculate the end_dir_dt structure.  This structure should contain
   * the same time information as the starting directory, but should have
   * the date information of the ending directory.  That way, we can
   * increment from the start time to this time using the number of
   * seconds in a day to hit each directory in between.
   */

  search_dt.unix_time = search_time;
  uconvert_from_utime(&search_dt);
  
  end_dir_dt.unix_time = end_time;
  uconvert_from_utime(&end_dir_dt);

  end_dir_dt.hour = search_dt.hour;
  end_dir_dt.min = search_dt.min;
  end_dir_dt.sec = search_dt.sec;
  uconvert_to_utime(&end_dir_dt);
  
  /*
   * Start in the directory that includes the search time and
   * process all of the directories (backwards) until we reach
   * the end time or find a file.  
   */

  for (dir_time = search_time; dir_time <= end_dir_dt.unix_time;
       dir_time += SECS_IN_DAY)
  {
    dir_dt.unix_time = dir_time;
    uconvert_from_utime(&dir_dt);
    
    sprintf(dir_path, "%s%s%04d%02d%02d",
	    handle->input_dir, PATH_DELIM,
	    dir_dt.year, dir_dt.month, dir_dt.day);

    /*
     * Try to open the directory
     */
  
    if ((dirp = opendir(dir_path)) == NULL)
    {
      if (handle->debug)
      {
	fprintf(stderr,
		"ERROR: ds_input_path::%s\n", routine_name);
	fprintf(stderr,
		"Error opening directory <%s>\n",
		dir_path);
      }
    
      continue;
    }
  
    /*
     * Loop thru directory looking for the data file names
     */
  
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
      /*
       * exclude dir entries and files beginning with '.'
       */

      if (dp->d_name[0] == '.')
	continue;
      
      /*
       * check that the file name is in the correct format
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
    
      file_dt.year = dir_dt.year;
      file_dt.month = dir_dt.month;
      file_dt.day = dir_dt.day;
      file_dt.hour = hour;
      file_dt.min = min;
      file_dt.sec = sec;
	
      uconvert_to_utime(&file_dt);
	
      if (file_dt.unix_time >= search_time &&
	  file_dt.unix_time <= end_time)
      {
	time_diff = (int)(file_dt.unix_time - search_time);
	  
	if (time_diff <= max_time_offset &&
	    (!file_found ||
	     time_diff < min_time_diff))
	{
	  sprintf(handle->input_path, "%s%s%s",
		  dir_path, PATH_DELIM,
		  dp->d_name);

	  file_found = TRUE;
	  min_time_diff = time_diff;
	  return_time = file_dt.unix_time;
	  
	} /*if (time_diff < *time_error) */
	
      } /* if (file_dt.unix_time >= com->time_min ... */

    } /* dp */
  
    closedir(dirp);

    if (file_found)
    {
      *data_time = return_time;
      return(handle->input_path);
    }
    
  } /* endfor - dir_time */
  
  *data_time = -1;
  return((char *)NULL);
  
}

/*****************************
 * get latest data file before the given time, but within the
 * given offset.
 *
 * On success, returns the name of the closest file.  This pointer
 * points to static memory which should not be freed by the caller.
 * On failure, returns NULL.
 *
 * Returns the data time of the file (based on the file name) in
 * the argument list.
 */

char *DSINP_get_first_before(DSINP_handle_t *handle,
			     time_t search_time,
			     int max_time_offset,
			     time_t *data_time)
{
  static char *routine_name = "DSINP_get_first_before()";
  
  int file_found = FALSE;
  
  char file_ext[16];
  int time_diff;
  int min_time_diff = 0;
  int hour, min, sec;
  
  time_t end_time = search_time - max_time_offset;
  
  date_time_t search_dt;
  date_time_t end_dir_dt;
  date_time_t file_dt;
  date_time_t dir_dt;
  
  char dir_path[MAX_PATH_LEN];
  
  struct dirent *dp;
  DIR *dirp;
  
  time_t dir_time;
  time_t return_time = -1;
  
  /*
   * Calculate the end_dir_dt structure.  This structure should contain
   * the same time information as the starting directory, but should have
   * the date information of the ending directory.  That way, we can
   * increment from the start time to this time using the number of
   * seconds in a day to hit each directory in between.
   */

  search_dt.unix_time = search_time;
  uconvert_from_utime(&search_dt);
  
  end_dir_dt.unix_time = end_time;
  uconvert_from_utime(&end_dir_dt);

  end_dir_dt.hour = search_dt.hour;
  end_dir_dt.min = search_dt.min;
  end_dir_dt.sec = search_dt.sec;
  uconvert_to_utime(&end_dir_dt);
  
  /*
   * Start in the directory that includes the search time and
   * process all of the directories (backwards) until we reach
   * the end time or find a file.  
   */

  for (dir_time = search_time; dir_time >= end_dir_dt.unix_time;
       dir_time -= SECS_IN_DAY)
  {
    dir_dt.unix_time = dir_time;
    uconvert_from_utime(&dir_dt);
    
    sprintf(dir_path, "%s%s%04d%02d%02d",
	    handle->input_dir, PATH_DELIM,
	    dir_dt.year, dir_dt.month, dir_dt.day);

    /*
     * Try to open the directory
     */
  
    if ((dirp = opendir(dir_path)) == NULL)
    {
      if (handle->debug)
      {
	fprintf(stderr,
		"ERROR: ds_input_path::%s\n", routine_name);
	fprintf(stderr,
		"Error opening directory <%s>\n",
		dir_path);
      }
    
      continue;
    }
  
    /*
     * Loop thru directory looking for the data file names
     */
  
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
      /*
       * exclude dir entries and files beginning with '.'
       */

      if (dp->d_name[0] == '.')
	continue;
      
      /*
       * check that the file name is in the correct format
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
    
      file_dt.year = dir_dt.year;
      file_dt.month = dir_dt.month;
      file_dt.day = dir_dt.day;
      file_dt.hour = hour;
      file_dt.min = min;
      file_dt.sec = sec;
	
      uconvert_to_utime(&file_dt);
	
      if (file_dt.unix_time <= search_time &&
	  file_dt.unix_time >= end_time)
      {
	time_diff = (int)(search_time - file_dt.unix_time);
	  
	if (time_diff <= max_time_offset &&
	    (!file_found ||
	     time_diff < min_time_diff))
	{
	  sprintf(handle->input_path, "%s%s%s",
		  dir_path, PATH_DELIM,
		  dp->d_name);

	  file_found = TRUE;
	  min_time_diff = time_diff;
	  return_time = file_dt.unix_time;
	  
	} /*if (time_diff < *time_error) */
	
      } /* if (file_dt.unix_time >= com->time_min ... */

    } /* dp */
  
    closedir(dirp);

    if (file_found)
    {
      *data_time = return_time;
      return(handle->input_path);
    }
    
  } /* endfor - dir_time */
  
  *data_time = -1;
  return((char *)NULL);
  
}

/*****************************
 * get next file path
 *
 * Realtime and Archive modes only
 *
 * returns NULL on failure
 */

char *DSINP_next(DSINP_handle_t *handle)
{
  struct stat stat_buf;

  char *FName; /* Added by Niles 2 June 1999 */
  char *last2, *last3;
  static char TFName[MAX_PATH_LEN];

  switch(handle->mode)
  {
  case DSINP_ARCHIVE_MODE :
    /*
     * in archive mode, go through the file list
     */

    while (handle->file_num < handle->n_files)
    {
      handle->file_num++;

      /*
       * Make sure the file exists before returning it
       */

      if (ta_stat(handle->file_paths[handle->file_num - 1], &stat_buf) == 0)
	return (handle->file_paths[handle->file_num - 1]);

      /* Additions by Niles start. */

      /* Check to see if the filename is that of a compressed
	 file, and if so, has the file been uncompressed. If so,
	 return the uncompressed file name.
       */

      FName=(handle->file_paths[handle->file_num - 1]);

      last2=FName + strlen(FName) - 2;
      if (!(strncmp(last2,".Z",2))){
	*last2=(char)0;
	if (ta_stat(FName,&stat_buf)==0) return FName;
      }

      last3=FName + strlen(FName) - 3;
      if (!(strncmp(last3,".gz",3))){
	*last3=(char)0;
	if (ta_stat(FName,&stat_buf)==0) return FName;
      }

      /* Check to see if the file has been compressed. If so, return the
        compressed file name. */

      sprintf(TFName,"%s%s",(handle->file_paths[handle->file_num - 1]),".gz");
      if (ta_stat(TFName,&stat_buf)==0) return TFName;

      sprintf(TFName,"%s%s",(handle->file_paths[handle->file_num - 1]),".Z");
      if (ta_stat(TFName,&stat_buf)==0) return TFName;


      /* Additions by Niles end. */

    }

    /*
     * If we reach this point, we have returned all of the
     * existing files.
     */

    return (NULL);

  case DSINP_REALTIME_MODE :
    /*
     * Keep trying until we get a file that actually exists
     */

    while (Forever)
    {

      if (handle->use_ldata_info) {

	/*
	 * in realtime mode wait for change in latest info
	 * sleep 1 second between tries.
	 */
	
	LDATA_info_read_blocking(&handle->last_data, handle->input_dir,
				 handle->max_age, 1000,
				 handle->heartbeat_func);
	
	/*
	 * new data
	 */
	
	/*
	 * try constructing the file path from the time
	 */
	
	sprintf(handle->input_path,
		"%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
		handle->input_dir,
		PATH_DELIM,
		handle->last_data.ltime.year,
		handle->last_data.ltime.month,
		handle->last_data.ltime.day,
		PATH_DELIM,
		handle->last_data.ltime.hour,
		handle->last_data.ltime.min,
		handle->last_data.ltime.sec,
		handle->last_data.info.file_ext);

	if (ta_stat(handle->input_path, &stat_buf) == 0) {
	  return (handle->input_path);
	}

	/*
	 * try constructing the file path from the user_info_1
	 */
	
	sprintf(handle->input_path,
		"%s%s%s",
		handle->input_dir,
		PATH_DELIM,
		handle->last_data.info.user_info_1);
	
	if (ta_stat(handle->input_path, &stat_buf) == 0) {
	  return (handle->input_path);
	}

	/*
	 * try constructing the file path from the user_info_1 plus the
	 * file extension
	 */
	
	sprintf(handle->input_path,
		"%s%s%s.%s",
		handle->input_dir,
		PATH_DELIM,
		handle->last_data.info.user_info_1,
		handle->last_data.info.file_ext);
	
	if (ta_stat(handle->input_path, &stat_buf) == 0) {
	  return (handle->input_path);
	}

	/*
	 * try constructing the file path from the time and
         * compression suffix (.gz)
	 */

	sprintf(handle->input_path,
		"%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s%s",
		handle->input_dir,
		PATH_DELIM,
		handle->last_data.ltime.year,
		handle->last_data.ltime.month,
		handle->last_data.ltime.day,
		PATH_DELIM,
		handle->last_data.ltime.hour,
		handle->last_data.ltime.min,
		handle->last_data.ltime.sec,
		handle->last_data.info.file_ext,
                ".gz");

	if (ta_stat(handle->input_path, &stat_buf) == 0) {
	  return (handle->input_path);
	}

	/*
	 * try constructing the file path from the time and
         * compression suffix (.Z)
	 */

	sprintf(handle->input_path,
		"%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s%s",
		handle->input_dir,
		PATH_DELIM,
		handle->last_data.ltime.year,
		handle->last_data.ltime.month,
		handle->last_data.ltime.day,
		PATH_DELIM,
		handle->last_data.ltime.hour,
		handle->last_data.ltime.min,
		handle->last_data.ltime.sec,
		handle->last_data.info.file_ext,
                ".Z");

	if (ta_stat(handle->input_path, &stat_buf) == 0) {
	  return (handle->input_path);
	}
	
      } else {

	/*
	 * no latest_data_info
	 * recursively search the directory for new files
	 */
	
	time_t closestValidTime = -1;

	if (find_next_path(handle, handle->input_dir,
			   &closestValidTime) == 0) {
	  handle->latest_time_used = closestValidTime;
	  return (handle->input_path);
	}
	if (handle->heartbeat_func != NULL) {
	  handle->heartbeat_func("DSINP_next: waiting for files");
	}
	sleep(5);

      }
	
    }
    break;
    
  case DSINP_TRIGGERED_MODE :
    break;
    
  } /*  switch(handle->mode) */

  return (NULL);

}

/*****************************
 * Find next realtime path
 *
 * Finds next realtime path in a directory.
 *
 * Use realtime_2 constructor before using this function.
 *
 * Returns 0 on success, -1 if an error is encountered.
 */

static int find_next_path(DSINP_handle_t *handle,
			  char *search_dir,
			  time_t *closestValidTime)
{
  
  static char *routine_name = "DSINP__find_next_path()";
  
  DIR *dirp;
  struct dirent *dp;
  struct stat fileStat;
  char filePath[MAX_PATH_LEN];
  int age;
  
  if ((dirp = opendir(search_dir)) == NULL) {
    if (handle->debug) {
      fprintf(stderr, "ERROR: ds_input_path::%s\n", routine_name);
      fprintf(stderr, "Error opening directory <%s>\n",
	      search_dir);
      perror("search_dir");
    }
    return (-1);
  }
  
  /* read directory looking for data files to be transferred */
  
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    /* exclude dir entries and files beginning with '.' and '_' */
    
    if (dp->d_name[0] == '.') {
      continue;
    }
    
    if (dp->d_name[0] == '_') {
      continue;
    }
    
    if (strstr(dp->d_name, "latest_data_info") != NULL) {
      continue;
    }

    /* check file time */

    sprintf(filePath, "%s%s%s", search_dir, PATH_DELIM, dp->d_name);
    
    if (ta_stat(filePath, &fileStat)) {
      if (handle->debug) {
	fprintf(stderr, "WARNING: ds_input_path::%s\n", routine_name);
	fprintf(stderr, "Cannot stat file <%s>\n", filePath);
      }
      continue;
    }
    
    if (fileStat.st_mtime <= handle->latest_time_used) {
      continue;
    }

    age = time(NULL) - fileStat.st_mtime;
    if (age > handle->max_age || age < 5) {
      continue;
    }

    /* for directories, search recursively */
    
    if (S_ISDIR(fileStat.st_mode)) {
      char dirPath[MAX_PATH_LEN];
      sprintf(dirPath, "%s%s%s", search_dir, PATH_DELIM, dp->d_name);
      find_next_path(handle, dirPath, closestValidTime);
      continue;
    }

    if (!S_ISREG(fileStat.st_mode)) {
      continue;
    }

    /* make sure we're returning the first relevant file */
    
    if (*closestValidTime < 0) {
      *closestValidTime = fileStat.st_mtime;
      strcpy(handle->input_path, filePath);
    } else {
      if (handle->latest_file_only) {
	if (*closestValidTime < fileStat.st_mtime) {
	  *closestValidTime = fileStat.st_mtime;
	  strcpy(handle->input_path, filePath);
	}
      } else {
	if (*closestValidTime > fileStat.st_mtime) {
	  *closestValidTime = fileStat.st_mtime;
	  strcpy(handle->input_path, filePath);
	}
      }
    }
    
  } /* dp */
  
  closedir(dirp);

  if (*closestValidTime < 0) {
    return (-1);
  } else {
    return (0);
  }

}


/*****************************
 * get latest file written to the input directory
 *
 * Realtime mode only
 *
 * returns NULL on failure
 */

char *DSINP_latest(DSINP_handle_t *handle)
{
  struct stat stat_buf;
  
  switch(handle->mode)
  {
  case DSINP_REALTIME_MODE :
    /*
     * Get the latest file in the directory
     */

    if (LDATA_info_read(&handle->last_data, handle->input_dir,
			handle->max_age) != 0)
      return(NULL);
    
    /*
     * new data
     */

    STRcopy(handle->input_path,
	    LDATA_data_path(&handle->last_data, handle->input_dir),
	    MAX_PATH_LEN);
    
    if (ta_stat(handle->input_path, &stat_buf) == 0)
      return (handle->input_path);
    break;
    
  case DSINP_ARCHIVE_MODE :
  case DSINP_TRIGGERED_MODE :
    break;
    
  } /*  switch(handle->mode) */

  return (NULL);

}

/*****************************
 * get new file written to the input directory since
 * given last data time.
 *
 * Realtime mode only
 *
 * returns NULL on failure
 */

char *DSINP_new_data(DSINP_handle_t *handle,
		     time_t last_data_time)
{
  struct stat stat_buf;
  
  switch(handle->mode)
  {
  case DSINP_REALTIME_MODE :
    /*
     * Get the latest file in the directory
     */

    if (LDATA_info_read(&handle->last_data, handle->input_dir,
			handle->max_age) != 0)
      return(NULL);
    
    /*
     * See if the data is new.
     */

    if (handle->last_data.info.latest_time <= last_data_time)
      return(NULL);
    
    /*
     * new data
     */

    STRcopy(handle->input_path,
	    LDATA_data_path(&handle->last_data, handle->input_dir),
	    MAX_PATH_LEN);
    
    if (ta_stat(handle->input_path, &stat_buf) == 0)
      return (handle->input_path);
    break;
    
  case DSINP_ARCHIVE_MODE :
  case DSINP_TRIGGERED_MODE :
    break;
    
  } /*  switch(handle->mode) */

  return (NULL);

}

/*****************************
 * reset to start of list
 * 
 * Archive mode only.
 */

void DSINP_reset(DSINP_handle_t *handle)
{
  if (handle->mode == DSINP_ARCHIVE_MODE)
    handle->file_num = 0;
}

/*****************************
 * get the data time information from the given file path.  The
 * file path is assumed to be:
 *   YYYYMMDD/HHMMSS.ext
 *   or
 *   YYYYMMDD/g_HHMMSS/f_SSSSSSSS.ext
 * 
 * The path MUST include the subdirectory with the date information.
 *
 * Returns -1 if an error is encountered.
 *
 * Error messages are no longer printed to stderr since static data files,
 * such as terrain files, will generate these errors unnecessarily 
 * (terrib 3/27/00)
 */

time_t DSINP_get_data_time(DSINP_handle_t *handle,
			   char *file_path)
{
#ifdef NOT_NOW
  static char *routine_name = "DSINP_get_data_time()";
#endif
  
  char file_path_copy[MAX_PATH_LEN];
  char *filename = NULL;
  char *date_string = NULL;
  char *g_string = NULL;
  char *slash_posn = NULL;
    
  /** Copy the filename */
  STRcopy(file_path_copy, file_path, MAX_PATH_LEN);
  
  /** Extract the filename from the path */
  if(get_next_directory(file_path_copy, &slash_posn, &filename) != 0) {
#ifdef NOT_NOW
    fprintf(stderr, "ERROR: ds_input_path::%s\n", routine_name);
    fprintf(stderr, "Error parsing file path <%s>.\n", file_path);
    fprintf(stderr, "Couldn't find slash before filename\n");
#endif
    return(-1);
  }
  if(is_non_forecast_filename(filename) == 0) {
     get_next_directory(file_path_copy, &slash_posn, &date_string);
    if(is_date(date_string) == 0) {
      return get_non_forecast_time(filename, date_string);
    }
  }
  else if(is_forecast_filename(filename) == 0) {
    get_next_directory(file_path_copy, &slash_posn, &g_string);
    if(is_forecast_g_dir(g_string) == 0) {
      get_next_directory(file_path_copy, &slash_posn, &date_string);
      if(is_date(date_string) == 0) {
        return get_forecast_time(filename, g_string, date_string);
      }
    }
  }
  else {
#ifdef NOT_NOW
    fprintf(stderr, "ERROR: ds_input_path::%s\n", routine_name);
    fprintf(stderr, "Error parsing file path <%s>.\n", file_path);
    fprintf(stderr, "Filename <%s> has incorrect format.\n", filename);
#endif
  }
  return -1;
}

  
/*****************************
 * Get the latest data time for the data.  Note that this is the last
 * data created which is not necessarily the ending data time.
 *
 * Returns -1 if an error is encountered.
 */

time_t DSINP_get_last_time(DSINP_handle_t *handle)
{
  /*
   * Read the current LDATA information
   */

  if (LDATA_info_read(&handle->last_data,
		      handle->input_dir,
		      -1) != 0)
    return(-1);

  return(handle->last_data.info.latest_time);
}


/*****************************
 * Get the begin and end times for the data.
 *
 * Returns 0 on success, -1 if an error is encountered.
 */

int DSINP_get_begin_and_end_times(DSINP_handle_t *handle,
				  time_t *begin_time, time_t *end_time)
{
  static char *routine_name = "DSINP_get_begin_end_end_times()";
  
  char begin_dir[MAX_PATH_LEN];
  char end_dir[MAX_PATH_LEN];
  
  char begin_file[MAX_PATH_LEN];
  char end_file[MAX_PATH_LEN];
  
  char begin_path[MAX_PATH_LEN];
  char end_path[MAX_PATH_LEN];
  
  DIR *dir_ptr;
  struct dirent *dir_ent_ptr;
  
  int first_dir;
  int first_file;
  
  /*
   * Initialize return values
   */

  *begin_time = -1;
  *end_time = -1;
  
  /*
   * Find the beginning and ending subdirectories
   */

  /*
   * Open the data directory
   */

  if ((dir_ptr = opendir(handle->input_dir)) == NULL)
  {
    if (handle->debug)
    {
      fprintf(stderr, "ERROR: ds_input_path::%s\n", routine_name);
      fprintf(stderr, "Error opening directory <%s>\n",
	      handle->input_dir);
      perror("");
    }
    
    return(-1);
  }
  
  /*
   * Read through the data directory and save the beginning and
   * ending dates.
   */

  first_dir = TRUE;

  for (dir_ent_ptr = readdir(dir_ptr); dir_ent_ptr != NULL;
       dir_ent_ptr = readdir(dir_ptr))
  {
    /*
     * Only look at directory entries that match the date pattern
     */

    if (strlen(dir_ent_ptr->d_name) != 8 ||
	!isdigit(dir_ent_ptr->d_name[0]) ||
	!isdigit(dir_ent_ptr->d_name[1]) ||
	!isdigit(dir_ent_ptr->d_name[2]) ||
	!isdigit(dir_ent_ptr->d_name[3]) ||
	!isdigit(dir_ent_ptr->d_name[4]) ||
	!isdigit(dir_ent_ptr->d_name[5]) ||
	!isdigit(dir_ent_ptr->d_name[6]) ||
	!isdigit(dir_ent_ptr->d_name[7]))
      continue;
    
    /*
     * See if this might be the first or last subdirectory
     */

    if (first_dir)
    {
      STRcopy(begin_dir, dir_ent_ptr->d_name, MAX_PATH_LEN);
      STRcopy(end_dir, dir_ent_ptr->d_name, MAX_PATH_LEN);
      
      first_dir = FALSE;
    }
    else
    {
      if (strcmp(begin_dir, dir_ent_ptr->d_name) > 0)
	STRcopy(begin_dir, dir_ent_ptr->d_name, MAX_PATH_LEN);
      
      if (strcmp(end_dir, dir_ent_ptr->d_name) < 0)
	STRcopy(end_dir, dir_ent_ptr->d_name, MAX_PATH_LEN);
    }
    
  } /* endfor - dir_ent_ptr */
  
  /*
   * Close the directory
   */

  closedir(dir_ptr);
  
  if (first_dir)
  {
    if (handle->debug)
    {
      fprintf(stderr, "WARNING: ds_input_path::%s\n", routine_name);
      fprintf(stderr,
	      "Cannot compute data times -- no subdirectories found\n");
    }
    
    return(-1);
  }
  
  sprintf(begin_path, "%s%s%s", handle->input_dir, PATH_DELIM, begin_dir);
  sprintf(end_path, "%s%s%s", handle->input_dir, PATH_DELIM, end_dir);
  
  /*
   * We have the beginning and ending subdirectories, so now we can
   * find the beginning and ending files.
   */

  /*
   * Get the first file in the beginning directory.
   */

  /*
   * Open the subdirectory
   */

  if ((dir_ptr = opendir(begin_path)) == NULL)
  {
    if (handle->debug)
    {
      fprintf(stderr, "ERROR: ds_input_path::%s\n", routine_name);
      fprintf(stderr, "Error opening begin subdirectory <%s>\n",
	      begin_path);
      perror("");
    }
    
    return(-1);
  }
  
  /*
   * Loop through the directory entries saving the starting file name
   */

  first_file = TRUE;
  
  for (dir_ent_ptr = readdir(dir_ptr); dir_ent_ptr != NULL;
       dir_ent_ptr = readdir(dir_ptr))
  {
    /*
     * Only look at files that match the time pattern
     */

    if (strlen(dir_ent_ptr->d_name) < 7 ||
	!isdigit(dir_ent_ptr->d_name[0]) ||
	!isdigit(dir_ent_ptr->d_name[1]) ||
	!isdigit(dir_ent_ptr->d_name[2]) ||
	!isdigit(dir_ent_ptr->d_name[3]) ||
	!isdigit(dir_ent_ptr->d_name[4]) ||
	!isdigit(dir_ent_ptr->d_name[5]) ||
	dir_ent_ptr->d_name[6] != '.')
      continue;
  
    if (first_file)
    {
      STRcopy(begin_file, dir_ent_ptr->d_name, MAX_PATH_LEN);
      
      first_file = FALSE;
    }
    else if (strcmp(begin_file, dir_ent_ptr->d_name) > 0)
    {
      STRcopy(begin_file, dir_ent_ptr->d_name, MAX_PATH_LEN);
    }
    
  } /* endfor - dir_ent_ptr */
  
  /*
   * Close the subdirectory
   */

  closedir(dir_ptr);
  
  sprintf(begin_path, "%s%s%s%s%s",
	  handle->input_dir, PATH_DELIM, begin_dir, PATH_DELIM, begin_file);
  
  /*
   * Get the last file in the ending directory.
   */

  /*
   * Open the subdirectory
   */

  if ((dir_ptr = opendir(end_path)) == NULL)
  {
    if (handle->debug)
    {
      fprintf(stderr, "ERROR: ds_input_path::%s\n", routine_name);
      fprintf(stderr, "Error opening end subdirectory <%s>\n",
	      end_path);
      perror("");
    }
    
    return(-1);
  }
  
  /*
   * Loop through the directory entries saving the starting file name
   */

  first_file = TRUE;
  
  for (dir_ent_ptr = readdir(dir_ptr); dir_ent_ptr != NULL;
       dir_ent_ptr = readdir(dir_ptr))
  {
    /*
     * Only look at files that match the time pattern
     */

    if (strlen(dir_ent_ptr->d_name) < 7 ||
	!isdigit(dir_ent_ptr->d_name[0]) ||
	!isdigit(dir_ent_ptr->d_name[1]) ||
	!isdigit(dir_ent_ptr->d_name[2]) ||
	!isdigit(dir_ent_ptr->d_name[3]) ||
	!isdigit(dir_ent_ptr->d_name[4]) ||
	!isdigit(dir_ent_ptr->d_name[5]) ||
	dir_ent_ptr->d_name[6] != '.')
      continue;
  
    if (first_file)
    {
      STRcopy(end_file, dir_ent_ptr->d_name, MAX_PATH_LEN);
      
      first_file = FALSE;
    }
    else if (strcmp(end_file, dir_ent_ptr->d_name) < 0)
    {
      STRcopy(end_file, dir_ent_ptr->d_name, MAX_PATH_LEN);
    }
    
  } /* endfor - dir_ent_ptr */
  
  /*
   * Close the subdirectory
   */

  closedir(dir_ptr);
  
  sprintf(end_path, "%s%s%s%s%s",
	  handle->input_dir, PATH_DELIM, end_dir, PATH_DELIM, end_file);
  
  /*
   * Set the return values.
   */

  *begin_time = DSINP_get_data_time(handle, begin_path);
  *end_time = DSINP_get_data_time(handle, end_path);
  
  return(0);
}


/********************************************************************
 * GENERAL UTILITIES (don't require a handle structure)
 ********************************************************************/

/*****************************
 * DSINP_get_dataset_times
 *
 * Retrieves the available data times for datasets generated between
 * the given times.  If begin_gen_time is -1, retrieves all dataset
 * times from the beginning of the data.  If end_gen_time is -1,
 * retrieves all dataset times until the latest time.
 *
 * Returns a pointer to an array of available dataset times, or NULL
 * if there are no datasets generated between the times or if there
 * was an error.  Returns a pointer to static memory which should NOT
 * be freed by the calling routine.
 */

DSINP_dataset_time_t *DSINP_get_dataset_times(char *input_dir,
					      time_t begin_gen_time,
					      time_t end_gen_time,
					      int *num_datasets)
{
  
  date_time_t begin_day_time;
  date_time_t end_day_time;
  
  DIR *main_dirp;
  struct dirent *main_dp;
  DIR *sub_dirp;
  struct dirent *sub_dp;
  DIR *fore_dirp;
  struct dirent *fore_dp;
  
  char subdir_path[MAX_PATH_LEN];
  char forecast_path[MAX_PATH_LEN];
  
  /*
   * Initialize return values.
   */

  *num_datasets = 0;

  /*
   * Clear the dataset times array.
   */

  DataTimesUsed = 0;
  
  /*
   * Get the times for the beginning of the day containing begin_gen_time
   * and the beginning of the day containing end_gen_time so we can easily
   * determine which data subdirectories to parse.
   */

  begin_day_time.unix_time = begin_gen_time;
  uconvert_from_utime(&begin_day_time);
  begin_day_time.hour = 0;
  begin_day_time.min = 0;
  begin_day_time.sec = 0;
  uconvert_to_utime(&begin_day_time);
  
  end_day_time.unix_time = end_gen_time;
  uconvert_from_utime(&end_day_time);
  end_day_time.hour = 0;
  end_day_time.min = 0;
  end_day_time.sec = 0;
  uconvert_to_utime(&end_day_time);
  
  /*
   * Loop through the subdirectories of the input path and process those
   * subdirectories for days between the begin and end times.
   */

  if ((main_dirp = opendir(input_dir)) == NULL)
  {
    /*
     * No data available.
     */

    return((DSINP_dataset_time_t *)NULL);
  }

  for (main_dp = readdir(main_dirp); main_dp != NULL; main_dp = readdir(main_dirp))
  {
    date_time_t dataset_time;
    time_t forecast_time;
    
    /*
     * Exclude files that don't match our filenaming conventions.
     */

    if (strlen(main_dp->d_name) != 8)
      continue;
    
    if (!isdigit(main_dp->d_name[0]) ||
	!isdigit(main_dp->d_name[1]) ||
	!isdigit(main_dp->d_name[2]) ||
	!isdigit(main_dp->d_name[3]) ||
	!isdigit(main_dp->d_name[4]) ||
	!isdigit(main_dp->d_name[5]) ||
	!isdigit(main_dp->d_name[6]) ||
	!isdigit(main_dp->d_name[7]))
      continue;
    
    /*
     * Convert the directory name to a time so we can see if we need
     * to process this subdirectory.
     */

    dataset_time.year =  ((main_dp->d_name[0] - '0') * 1000) +
                         ((main_dp->d_name[1] - '0') * 100) +
                         ((main_dp->d_name[2] - '0') * 10) +
                          (main_dp->d_name[3] - '0');
    dataset_time.month = ((main_dp->d_name[4] - '0') * 10) +
                          (main_dp->d_name[5] - '0');
    dataset_time.day =   ((main_dp->d_name[6] - '0') * 10) +
                          (main_dp->d_name[7] - '0');
    dataset_time.hour = 0;
    dataset_time.min = 0;
    dataset_time.sec = 0;
    
    uconvert_to_utime(&dataset_time);
    
    /*
     * See if this is a subdirectory we should process.
     */

    if (begin_gen_time > 0 &&
	dataset_time.unix_time < begin_day_time.unix_time)
      continue;
    
    if (end_gen_time > 0 &&
	dataset_time.unix_time > end_day_time.unix_time)
      continue;
    
    /*
     * Process the subdirectory
     */

    sprintf(subdir_path, "%s%s%s", input_dir, PATH_DELIM, main_dp->d_name);
      
    if ((sub_dirp = opendir(subdir_path)) == NULL)
      continue;
      
    for (sub_dp = readdir(sub_dirp); sub_dp != NULL; sub_dp = readdir(sub_dirp))
    {
      /*
       * Exclude files that don't match our filenaming conventions.
       */

      if (strlen(sub_dp->d_name) < 8)
	continue;
	
      if (sub_dp->d_name[0] == 'g' &&
	  sub_dp->d_name[1] == '_')
      {
	/*
	 * This is a forecast subdirectory.
	 */

	/*
	 * Make sure the forecast subdirectory meets our naming conventions.
	 */

	if (strlen(sub_dp->d_name) != 8)
	  continue;
	  
	if (!isdigit(sub_dp->d_name[2]) ||
	    !isdigit(sub_dp->d_name[3]) ||
	    !isdigit(sub_dp->d_name[4]) ||
	    !isdigit(sub_dp->d_name[5]) ||
	    !isdigit(sub_dp->d_name[6]) ||
	    !isdigit(sub_dp->d_name[7]))
	  continue;
	  
	/*
	 * Determine the generation time of the forecast directory.
	 */
	
	dataset_time.hour = ((sub_dp->d_name[2] - '0') * 10) +
	                     (sub_dp->d_name[3] - '0');
	dataset_time.min =  ((sub_dp->d_name[4] - '0') * 10) +
	                     (sub_dp->d_name[5] - '0');
	dataset_time.sec =  ((sub_dp->d_name[6] - '0') * 10) +
	                     (sub_dp->d_name[7] - '0');
	uconvert_to_utime(&dataset_time);
	  
	if (begin_gen_time > 0 &&
	    dataset_time.unix_time < begin_gen_time)
	  continue;
	
	if (end_gen_time > 0 &&
	    dataset_time.unix_time > end_gen_time)
	  continue;
	
	/*
	 * Generation time meets requirements -- add files to dataset list.
	 */

	sprintf(forecast_path, "%s%s%s", subdir_path, PATH_DELIM, sub_dp->d_name);
	
	if ((fore_dirp = opendir(forecast_path)) == NULL)
	  continue;
	
	for (fore_dp = readdir(fore_dirp);
	     fore_dp != NULL;
	     fore_dp = readdir(fore_dirp))
	{
	  /*
	   * Make sure the file names meet our naming conventions.
	   */

	  if (strlen(fore_dp->d_name) < 12)
	    continue;
	  
	  if (fore_dp->d_name[0] != 'f' ||
	      fore_dp->d_name[1] != '_' ||
	      !isdigit(fore_dp->d_name[2]) ||
	      !isdigit(fore_dp->d_name[3]) ||
	      !isdigit(fore_dp->d_name[4]) ||
	      !isdigit(fore_dp->d_name[5]) ||
	      !isdigit(fore_dp->d_name[6]) ||
	      !isdigit(fore_dp->d_name[7]) ||
	      !isdigit(fore_dp->d_name[8]) ||
	      !isdigit(fore_dp->d_name[9]))
	    continue;
	  
	  forecast_time = dataset_time.unix_time + atoi(&fore_dp->d_name[2]);
	  
	  add_dataset_time(dataset_time.unix_time, forecast_time);
	  
	} /* endfor - fore_dp */
        closedir(fore_dirp);	
      } /* endif - forecast subdirectory */
      else
      {
	/*
	 * Make sure the data file meets our naming conventions.
	 */

	if (!isdigit(sub_dp->d_name[0]) ||
	    !isdigit(sub_dp->d_name[1]) ||
	    !isdigit(sub_dp->d_name[2]) ||
	    !isdigit(sub_dp->d_name[3]) ||
	    !isdigit(sub_dp->d_name[4]) ||
	    !isdigit(sub_dp->d_name[5]))
	  continue;
	
	/*
	 * Determine the dataset time.
	 */

	dataset_time.hour = ((sub_dp->d_name[0] - '0') * 10) +
	                     (sub_dp->d_name[1] - '0');
	dataset_time.min =  ((sub_dp->d_name[2] - '0') * 10) +
	                     (sub_dp->d_name[3] - '0');
	dataset_time.sec =  ((sub_dp->d_name[4] - '0') * 10) +
	                     (sub_dp->d_name[5] - '0');
	uconvert_to_utime(&dataset_time);
	
	if (begin_gen_time > 0 &&
	    dataset_time.unix_time < begin_gen_time)
	  continue;
	
	if (end_gen_time > 0 &&
	    dataset_time.unix_time > end_gen_time)
	  continue;
	
	add_dataset_time(dataset_time.unix_time, -1);
	
      } /* endelse - regular filenames */
	
    } /* endfor - sub_dp */
      
    /*
     * Close the subdirectory
     */

    closedir(sub_dirp);

  } /* endfor - main_dp */
  
  /*
   * Close the directory
   */

  closedir(main_dirp);

  /*
   * Set the return values.
   */

  *num_datasets = DataTimesUsed;
  
  return(DataTimes);
}


/********************************************************************
 * STATIC FUNCTIONS
 ********************************************************************/

/*****************************
 * load_day
 *
 * Load up file paths for a given day
 */

static void load_day(DSINP_handle_t *handle,
		     char *subdir_path,
		     date_time_t *midday,
		     time_t start_time,
		     time_t end_time)
{
  struct dirent *dp;
  DIR *dirp;

  if ((dirp = opendir(subdir_path)) == NULL)
    return;

  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
  {
    int hour, min, sec;
    date_time_t file_time;
    char *path;
    
    /*
     * exclude dir entries and files beginning with '.'
     */

    if (dp->d_name[0] == '.')
      continue;
      
    /*
     * check that the dir name is in the correct format
     */

    if (sscanf(dp->d_name, "%2d%2d%2d",
	       &hour, &min, &sec) != 3)
      continue;
      
    if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	sec < 0 || sec > 59)
      continue;

    /*
     * check file time is within limits
     */

    file_time = *midday;
    file_time.hour = hour;
    file_time.min = min;
    file_time.sec = sec;
    uconvert_to_utime(&file_time);

    if (file_time.unix_time < start_time ||
	file_time.unix_time > end_time)
      continue;

    /*
     * name is in correct format. Therefore, accept it
     */

    path = (char *) umalloc(strlen(subdir_path) +
				  strlen(PATH_DELIM) +
				  strlen(dp->d_name) + 1);
    
    sprintf(path, "%s%s%s", subdir_path, PATH_DELIM, dp->d_name);

    /*
     * add to mem buffer
     */
    
    MEMbufAdd(handle->mbuf_paths, &path, sizeof(char *));
    handle->n_files++;

  } /* endfor - dp */
  
  closedir(dirp);

  return;
}


/*****************************
 * Add a dataset to the dataset times list.
 */

static void add_dataset_time(time_t gen_time, time_t forecast_time)
{
  /*
   * Make sure there is enough space in the list.
   */

  if (DataTimesUsed >= DataTimesAlloc)
  {
    DataTimesAlloc += DataTimesIncr;
    
    if (DataTimes == (DSINP_dataset_time_t *)NULL)
      DataTimes = (DSINP_dataset_time_t *)umalloc(DataTimesAlloc *
						  sizeof(DSINP_dataset_time_t));
    else
      DataTimes = (DSINP_dataset_time_t *)urealloc(DataTimes,
						   DataTimesAlloc *
						   sizeof(DSINP_dataset_time_t));
  }
  
  /*
   * Add the dataset information to the list.
   */

  DataTimes[DataTimesUsed].gen_time = gen_time;
  DataTimes[DataTimesUsed].forecast_time = forecast_time;
  
  DataTimesUsed++;
  
}


/*****************************
 * define function to be used for sorting paths
 */

static int compare_paths(const void *v1, const void *v2)
{
  char *path1 = *((char **) v1);
  char *path2 = *((char **) v2);

  return (strcmp(path1, path2));
}

/********************************************************************
 * HELPER FUNCTIONS
 ********************************************************************/

/**
 * _get_forecast_time
 *
 * parse time from strings
 *   filename = f_########
 *   g_string = g_######
 *   date_string = ########
 */
time_t get_forecast_time(char *filename, char* g_string, char *date_string)
{
#ifdef NOT_NOW
  static char *routine_name = "_get_forecast_time()";
#endif

  date_time_t data_time;

  /** Retrieve the date information from the date string */
  if(get_ymd(&data_time, date_string) != 0) {
    return(-1);
  }
  
  /** Retrieve the time information from the g directory name */
  if(get_hms(&data_time, g_string, 2) != 0) {
    return(-1);
  }

   /** Retrieve the forecast seconds from the file name, add to time */
  int forecast = get_forecast_seconds(filename);
  uconvert_to_utime(&data_time);
  data_time.unix_time = data_time.unix_time + forecast;
  uconvert_from_utime(&data_time);

  /** Calculate and return the data time */
  return(uunix_time(&data_time));
}

/**
 * _get_non_forecast_time
 *
 * parse time from strings
 *   filename = ######
 *   date_string = ########
 */
time_t get_non_forecast_time(char *filename, char *date_string)
{
#ifdef NOT_NOW
  static char *routine_name = "_get_non_forecast_time()";
#endif

  date_time_t data_time;

  /** Retrieve the date information from the date string */
  if(get_ymd(&data_time, date_string) != 0) {
    return(-1);
  }
  
  /** Retrieve the time information from the file name */
  if(get_hms(&data_time, filename, 0) != 0) {
    return(-1);
  }

  /** Calculate and return the data time */
  return(uunix_time(&data_time));
}


int is_non_forecast_filename(char *filename)
{
  return match_expression(filename, "######");
}

int is_forecast_filename(char *filename)
{
  return match_expression(filename, "f_########");
}  


int is_forecast_g_dir(char *filename)
{
  return match_expression(filename, "g_######");
}  

int is_date(char *name){
  return match_expression(name, "########");
}

/**
 * _match_expression
 *
 * compare string to expression
 * matches # to any digit
 */
int match_expression(char *str, char *expr)
{
  unsigned int len = strlen(expr);
  if(strlen(str) < len) {
    return -1;
  }
  int i;
  for(i=0; i<len; i++) {
    if(expr[i] == '#') {
      if(!isdigit(str[i])) {
        return -1;
      }
    }
    else{
      if( str[i] != expr[i]) {
        return -1;
      }
    }
  }
  return 0;
}

/**
 * _get_hms
 *
 * read YYYYMMDD from string, set date_time_t variables
 */
int get_ymd(date_time_t* dt, char* str) {
  dt->year = ((str[0] - '0') * 1000) +
    ((str[1] - '0') * 100) +
    ((str[2] - '0') * 10) +
    (str[3] - '0');
  dt->month = ((str[4] - '0') * 10) +
    (str[5] - '0');
  dt->day = ((str[6] - '0') * 10) +
    (str[7] - '0');

  if(dt->year < 0 ||
    dt->month < 1 || dt->month > 12 ||
    dt->day < 1 || dt->day > 31)
  {
#ifdef NOT_NOW
    fprintf(stderr, "ERROR: ds_input_path::%s\n", routine_name);
    fprintf(stderr, "Error parsing file path <%s>.\n", file_path);
    fprintf(stderr,
      "Date subdirectory <%s> has incorrect format, should be YYYYMMDD.\n",
      date_string);
#endif
    
    return -1;
  }
  return 0;
}

/**
 * _get_hms
 *
 * read HHMMSS from string starting at index, set date_time_t variables
 */
int get_hms(date_time_t* dt, char* str, int index) {
  int tmp;
  int i = index;

  tmp = (str[i++] - '0') * 10;
  tmp += (str[i++] - '0');
  dt->hour = tmp;

  tmp = (str[i++] - '0') * 10;
  tmp += (str[i++] - '0');
  dt->min = tmp;

  tmp = (str[i++] - '0') * 10;
  tmp += (str[i++] - '0');
  dt->sec = tmp;
  
  if (dt->hour < 0 || dt->hour > 23 ||
      dt->min < 0 || dt->min > 59 ||
      dt->sec < 0 || dt->sec > 59)
  {
#ifdef NOT_NOW
    fprintf(stderr, "ERROR: ds_input_path::%s\n", routine_name);
    fprintf(stderr, "Error parsing file path <%s>.\n", file_path);
    fprintf(stderr,
      "Filename <%s> has incorrect format, should be HHMMSS.<ext>.\n",
      filename);
#endif
    
    return -1;
  }
  return 0;
}

/**
 * _get_forecast_seconds
 *
 * convert forecast string to int value
 * expected string format: f_########
 */
int get_forecast_seconds(char* str) {
  return ( (str[2] - '0') * 10000000) + 
    ( (str[3] - '0') * 1000000) +   
    ( (str[4] - '0') * 100000) +   
    ( (str[5] - '0') * 10000) +   
    ( (str[6] - '0') * 1000) +   
    ( (str[7] - '0') * 100) +   
    ( (str[8] - '0') * 10) +   
      (str[9] - '0');
}

/**
 * _get_next_directory
 *
 * moves slash_posn to the next directory (parent) in the path,
 * sets str to the directory string 
 */
int get_next_directory(char* file_path_copy, char** slash_posn, char** str) {
	int ret = 0;
  *slash_posn = strrchr(file_path_copy, '/');
  
  if (*slash_posn == (char *)NULL) {
    *str = file_path_copy;
    ret = -1;
  } else {
    *str = *slash_posn + 1;
    *(slash_posn[0]) = '\0';
  }

  return ret;
}
