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
/******************************************************************
 * LDATA_INFO.C
 *
 * This module handles latest data time information.
 *
 * See <toolsa/ldata_info.h> for interface.
 * 
 * Modified from libs/cidd/src/cdata_util/file_index.c
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * October 1997
 *
 ***********************************************************************/

#include <toolsa/ldata_info.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <toolsa/str.h>

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#define LDATA_INFO_TMP_NAME "latest_data_info.tmp"

/*
 * Need reasonable number here to remove code weakness of 
 * unchecked bound. 
 */

#define NUM_FCSTS_MAX 512 

/*
 * file scope prototypes
 */

static int do_read(LDATA_handle_t *handle, FILE *in);
     
static void fill_info(LDATA_handle_t* handle,
		      time_t latest_time,
		      const char* file_ext,
		      const char* user_info_1,
		      const char* user_info_2,
		      int n_fcasts,
		      const int *fcast_lead_times);
     
static int init_done(LDATA_handle_t *info);

static int read_info_file(LDATA_handle_t *handle,
			  const char *file_path);
     
static int write_info_file(LDATA_handle_t *handle,
			   const char *file_path,
			   const char *tmp_path);

/*******************************************************************
 * LDATA_init_handle()
 *
 * Initialize the handle.
 *
 * Inputs:
 *
 *   handle: pointer to static instance of LDATA_handle_t.
 *
 *   prog_name: program name
 *
 *   debug: flag, set to TRUE if you want debug printout
 */

void LDATA_init_handle(LDATA_handle_t *handle,
		       const char *prog_name,
		       int debug)
     
{

  /*
   * zero out handle
   */

  memset(handle, 0, sizeof(LDATA_handle_t));
  
  /*
   * allocate and load up prog name
   */

  handle->prog_name = umalloc(strlen(prog_name) + 1);
  strcpy(handle->prog_name, prog_name);

  /*
   * initialize debug field
   */
  
  handle->debug = debug;

  /*
   * initialize file name
   */

  handle->file_name = STRdup(LDATA_INFO_FILE_NAME);

  handle->not_exist_print = TRUE;
  handle->too_old_print = TRUE;
  handle->not_modified_print = TRUE;
  
  /*
   * set init flag
   */
  
  handle->init_flag = LDATA_INFO_INIT_FLAG;

}

/*******************************************************************
 * LDATA_free_handle()
 *
 * Free the memory associated with the handle.
 */

void LDATA_free_handle(LDATA_handle_t *handle)
     
{

  if (init_done(handle)) {

    if (handle->prog_name != NULL) {
      ufree(handle->prog_name);
    }
    
    if (handle->file_name != NULL) {
      STRfree(handle->file_name);
    }
    
    if (handle->n_fcasts_alloc > 0) {
      if (handle->fcast_lead_times != NULL) {
	ufree(handle->fcast_lead_times);
      }
      handle->n_fcasts_alloc = 0;
    }
    
    handle->init_flag = 0;

  }

}

/*******************************************************************
 * LDATA_set_file_name()
 *
 * Sets the file name to be used in the routines.
 * The actual file name will be this name prefixed with an '_'.
 *
 * The default name is 'latest_data_info'.
 */

void LDATA_set_file_name(LDATA_handle_t *handle,
			 const char *file_name)

{
  assert(init_done(handle));
  if (handle->file_name != NULL) {
    STRfree(handle->file_name);
  }
  handle->file_name = STRdup(file_name);
}
     
/*************************************
 * LDATA_info_print()
 *
 * Prints info to output stream
 *
 * returns 0 on success, -1 on failure
 */

int LDATA_info_print(LDATA_handle_t* handle,
		     FILE *out)
     
{

  int i;
  date_time_t *ltime;
  LDATA_info_t *info = &handle->info;

  assert(init_done(handle));

  /*
   * latest_time
   */
  
  ltime = &handle->ltime;
  if (fprintf(out, "%ld %d %d %d %d %d %d\n",
	      (long) ltime->unix_time,
	      ltime->year, ltime->month, ltime->day,
	      ltime->hour, ltime->min, ltime->sec) < 0) {
    return (-1);
  }

  /*
   * file_ext, user_info_1, user_info_2
   */
  
  if (fprintf(out, "%s\n%s\n%s\n",
	      info->file_ext,
	      info->user_info_1,
	      info->user_info_2) < 0) {
    return (-1);
  }
  
  /*
   * forecasts
   */
  
  if (fprintf(out, "%d\n", (int) info->n_fcasts) < 0) {
    return (-1);
  }
  for (i = 0; i < info->n_fcasts; i++) {
    if (fprintf(out, "%d\n",
		handle->fcast_lead_times[i]) < 0) {
      return (-1);
    }
  }

  return (0);

}

/*********************************************************************
 * LDATA_info_read()
 *
 * Read the struct data from the current file info, including forecast
 * lead times if they are present.
 *
 * Inputs:
 *
 *   handle: see LDATA_init_handle()
 *
 *   source_str:
 *
 *     for file access, this is the data directory.
 *     for network access, this is either
 *                            port@host or
 *                            type::subtype::instance
 * 
 *   max_valid_age:
 *
 *     This is the max age (in secs) for which the 
 *     latest data info is considered valid. If the info is
 *     older than this, we need to wait for new info.
 *
 *     If max_valid_age is set negative, the age test is not done.
 *
 * Side effects:
 *
 *    (1) If new data found, sets handle->prev_mod_time to
 *        file modify time.
 *
 *        NOTE: For this to work, the handle must be static between calls
 *        since the prev_mod_time in the handle is used to determine when
 *        the time of the file has changed.
 *
 *    (2) Fills out the file path in the handle.
 *
 * Returns:
 *
 *    0 on success, -1 on failure.
 *
 *********************************************************************/

int LDATA_info_read(LDATA_handle_t* handle,
		    const char *source_str,
		    int max_valid_age)

{

  struct stat file_stat;
  time_t file_age;
  time_t now;
  
  assert(init_done(handle));

  /*
   * set the file path - new with underscore
   */
  
  sprintf(handle->file_path, "%s%s_%s",
	  source_str, PATH_DELIM, handle->file_name);

  /*
   * Check if info file exists
   */

  if (stat(handle->file_path, &file_stat)) {

    /*
     * try old style
     */

    sprintf(handle->file_path, "%s%s%s",
	    source_str, PATH_DELIM, handle->file_name);

    if (stat(handle->file_path, &file_stat)) {
      if (handle->debug && handle->not_exist_print) {
	fprintf(stderr,
		"LDATA_info_read: info file %s does not exist\n",
		handle->file_path);
	handle->not_exist_print = FALSE;
      }
      return (-1);
    }

  }
  
  /*
   * compute age, check for max valid age
   */

  if (max_valid_age >= 0) {
    now = time(NULL);
    file_age = now - file_stat.st_mtime;
    if (file_age > max_valid_age) {
      if (handle->debug && handle->too_old_print) {
	fprintf(stderr, "LDATA_info_read: info file %s too old\n",
		handle->file_path);
	handle->too_old_print = FALSE;
      }
      return (-1);
    }
  } /* if (max_valid_age >= 0) */

  /*
   * check for modified file time
   */

  if (file_stat.st_mtime == handle->prev_mod_time) {
    if (handle->debug && handle->not_modified_print) {
      fprintf(stderr, "LDATA_info_read: info file %s not modified\n",
	      handle->file_path);
      fprintf(stderr, "Last mod time: %s\n", utimstr(handle->prev_mod_time));
      handle->not_modified_print = FALSE;
    }
    return (-1);
  }

  /*
   * read in file
   */
    
  if (read_info_file(handle, handle->file_path)) {
    if (handle->debug) {
      fprintf(stderr,
	      "LDATA_info_read: error reading info file %s\n",
	      handle->file_path);

      /*
       * Removing bad ldata file.
       */

      unlink(handle->file_path);
    }
    return (-1);
  }

  if (handle->debug) {
    fprintf(stderr, "LDATA_info_read : success reading %s\n",
	    handle->file_path);
  }

  /*
   * set prev_mod_time to save it for next iteration
   */
  
  handle->prev_mod_time = file_stat.st_mtime;

  /*
   * reset print flags
   */
  
  handle->not_exist_print = TRUE;
  handle->too_old_print = TRUE;
  handle->not_modified_print = TRUE;
  
  return (0);
  
}

/*********************************************************************
 * LDATA_info_read_blocking()
 *
 * Read latest data info, blocking until info is available.
 *
 * See LDATA_info_read() for the non-blocking behavior upon
 * which this function is based.
 *
 * Inputs:
 *
 *   source_str: see LDATA_info_read()
 *
 *   max_valid_age (secs): see LDATA_info_read()
 *
 *   sleep_msecs (millisecs):
 *     While in the blocked state, the program sleeps for sleep_msecs
 *     millisecs at a time before checking again.
 *
 *   heartbeat_func(): heartbeat function
 *
 *     Each cycle, the function heartbeat_func() is called to allow
 *     any heartbeat actions to be carried out. If heartbeat_func is
 *     set to NULL, it is not called.
 *
 *     The string arg passed to the heartbeat
 *     function is "In LDATA_info_read_blocking".
 *
 * Side effects:
 *
 *   See LDATA_info_read()
 *
 *********************************************************************/

void LDATA_info_read_blocking(LDATA_handle_t* handle,
			      const char *source_str,
			      int max_valid_age,
			      int sleep_msecs,
			      LDATA_heartbeat_t heartbeat_func)

{

  while (LDATA_info_read(handle, source_str, max_valid_age)) {

    if (heartbeat_func != NULL) {
      heartbeat_func("In LDATA_info_read_blocking");
    }
    umsleep(sleep_msecs);

  }

}

/*****************************************************************
 * LDATA_info_write()
 *
 * Writes latest info to file.
 *
 * Writes to a tmp file first, then moves the tmp file to
 * the final file name when done.
 *
 * Inputs:
 *
 *   handle: see LDATA_init_handle()
 *
 *   source_str:
 *
 *     for file access, this is the data directory.
 *     for network access, this is either
 *                            port@host or
 *                            type::subtype::instance
 * 
 *   file_ext: file extension if applicable, otherwise set to NULL
 *
 *   user_info: set user information if applicable, otherwise NULL
 *
 *   n_fcasts: number of forecast time, usually 0
 *
 *   fcast_lead_times: array of forecast times,
 *                     set this to NULL if n_fcasts == 0
 *
 * Side effect:
 *   Fills out the file path in the handle.
 *
 * Returns:
 *   On success, returns 0, on failure returns -1.
 *
 * NOTE: If the environment variable $LDATA_NO_WRITE is defined, this
 *       routine does nothing.  This is useful when regenerating old
 *       data while also running in realtime mode.
 */

int LDATA_info_write(LDATA_handle_t* handle,
		     const char *source_str,
		     time_t latest_time,
		     const char* file_ext,
		     const char* user_info_1,
		     const char* user_info_2,
		     int n_fcasts,
		     const int *fcast_lead_times)
     
{

  char tmp_path[MAX_PATH_LEN];
  char old_path[MAX_PATH_LEN];
  struct stat old_stat;
  char *env_str;
  
  /*
   * Return now if $LDATA_NO_WRITE is defined.
   */

  env_str = getenv("LDATA_NO_WRITE");
  if (env_str && STRequal(env_str, "true")) {
    return(0);
  }

  assert(init_done(handle));

  /*
   * set the file paths
   */
  
  sprintf(handle->file_path, "%s%s_%s",
	  source_str, PATH_DELIM, handle->file_name);

  sprintf(tmp_path, "%s%s_%s",
	  source_str, PATH_DELIM, LDATA_INFO_TMP_NAME);

  /*
   * fill out info struct
   */

  fill_info(handle, latest_time,
	    file_ext, user_info_1, user_info_2,
	    n_fcasts, fcast_lead_times);

  /*
   * write the new-style file - has underscore
   */

  if (write_info_file(handle, handle->file_path, tmp_path)) {
    return (-1);
  }

  /*
   * remove the old-style file if it exists
   */

  sprintf(old_path, "%s%s%s",
	  source_str, PATH_DELIM, handle->file_name);
  if (stat(old_path, &old_stat) == 0) {
    unlink(old_path);
  }

  if (write_info_file(handle, old_path, tmp_path)) {
    return (-1);
  }

  return (0);
  
}

/*******************************************************************
 * LDATA_data_path()
 *
 * Returns path of latest file using std RAP naming convention,
 * relative to top_dir.
 *
 * Do not free the char * returned.
 */

char *LDATA_data_path(LDATA_handle_t *handle, char *top_dir)
     
{

  /*
   * compute file path
   */
  
  sprintf(handle->file_path,
	  "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	  top_dir, PATH_DELIM,
	  handle->ltime.year, handle->ltime.month, handle->ltime.day,
	  PATH_DELIM,
	  handle->ltime.hour, handle->ltime.min, handle->ltime.sec,
	  handle->info.file_ext);

  return (handle->file_path);

}
      


/*******************************************************************
 * alloc_fcasts()
 *
 * Alloc space for forecasts
 *
 */

void LDATA_alloc_fcasts(LDATA_handle_t *handle,
			int n_fcasts)
     
{
  
  assert(init_done(handle));
  
  if (n_fcasts > handle->n_fcasts_alloc) {
    if (handle->fcast_lead_times == NULL) {
      handle->fcast_lead_times =
	(int *) umalloc(n_fcasts * sizeof(int));
    } else {
      handle->fcast_lead_times =
	(int *) urealloc(handle->fcast_lead_times,
			 n_fcasts * sizeof(int));
    }
    handle->n_fcasts_alloc = n_fcasts;
  }
  
  handle->info.n_fcasts = n_fcasts;

}

/********************************************************************
 * FILE SCOPE ROUTINES
 ********************************************************************/

/*************************************************************************
 * do_read()
 *
 * reads info from open file descriptor
 *
 * returns 0 on success, -1 on failure
 */

static int do_read(LDATA_handle_t *handle, FILE *in)
     
{

  int i;
  int n_fcasts;
  int itime;
  char line[BUFSIZ];
  long ltime;
  date_time_t dtime;

  /*
   * latest time
   */
  
  if (fgets(line, BUFSIZ, in) == NULL) {
    return (-1);
  }
  if (sscanf(line, "%ld", &ltime) != 1) {
    return (-1);
  }
  if (ltime == -1) {
    if (sscanf(line, "%ld%d%d%d%d%d%d", &ltime,
	       &dtime.year, &dtime.month, &dtime.day,
	       &dtime.hour, &dtime.min, &dtime.sec) != 7) {
      return (-1);
    }
    uconvert_to_utime(&dtime);
    handle->ltime = dtime;
    handle->info.latest_time = dtime.unix_time;
  } else {
    handle->info.latest_time = ltime;
    handle->ltime.unix_time = ltime;
    uconvert_from_utime(&handle->ltime);
  }

  /*
   * read file_ext, user_info
   */

  if (fgets(handle->info.file_ext,
	    LDATA_INFO_STR_LEN, in) == NULL) {
    return (-1);
  }
  handle->info.file_ext[strlen(handle->info.file_ext) - 1] = '\0';

  if (fgets(handle->info.user_info_1,
	    LDATA_INFO_STR_LEN, in) == NULL) {
    return (-1);
  }
  handle->info.user_info_1[strlen(handle->info.user_info_1) - 1] = '\0';

  if (fgets(handle->info.user_info_2,
	    LDATA_INFO_STR_LEN, in) == NULL) {
    return (-1);
  }
  handle->info.user_info_2[strlen(handle->info.user_info_2) - 1] = '\0';

  /*
   * number of forecasts
   */
  
  if (fgets(line, BUFSIZ, in) == NULL) {
    return (-1);
  }
  {
    size_t len = strlen(line);
    if(len == 0  || line[len-1] != '\n'){
      return(-1);
    }
  }
   
  if (sscanf(line, "%d", &n_fcasts) != 1) {
    return (-1);
  }
  
  /*
   * Satisfy loop bound check: CWE-606
   */ 
  if (n_fcasts > NUM_FCSTS_MAX) {
    return (-1);
  } 
  
  handle->info.n_fcasts = n_fcasts;

  /*
   * forecasts
   */
  LDATA_alloc_fcasts(handle, n_fcasts);
  
  for (i = 0; i < n_fcasts; i++) {
    if (fgets(line, BUFSIZ, in) == NULL) {
      return (-1);
    }
    if (sscanf(line, "%d", &itime) != 1) {
      return (-1);
    }
    handle->fcast_lead_times[i] = itime;
  }

  return (0);

}

/*****************************************************************
 * fill_info
 *
 * Fill in structure members of a LDATA_handle_t data struct.
 *
 * If file_ext, user_info_1 or user_info_2 are NULL,
 * the relevant strings are set empty.
 *
 */

static void fill_info(LDATA_handle_t* handle,
		      time_t latest_time,
		      const char* file_ext,
		      const char* user_info_1,
		      const char* user_info_2,
		      int n_fcasts,
		      const int *fcast_lead_times)
     
{
  
  int i;
  LDATA_info_t *info = &handle->info;
  
  assert(init_done(handle));

  /*
   * copy in latest time
   */
  
  info->latest_time = latest_time;

  /*
   * set fully qualified time struct in the handle
   */

  handle->ltime.unix_time = latest_time;
  uconvert_from_utime(&handle->ltime);
  
  /*
   * copy in strings
   */
  
  if (file_ext != NULL) {
    STRncopy(info->file_ext, file_ext, LDATA_INFO_STR_LEN);
  } else {
    STRncopy(info->file_ext, "none", LDATA_INFO_STR_LEN);
  }

  if (user_info_1 != NULL) {
    STRncopy(info->user_info_1, user_info_1, LDATA_INFO_STR_LEN);
  } else {
    STRncopy(info->user_info_1, "none", LDATA_INFO_STR_LEN);
  }

  if (user_info_2 != NULL) {
    STRncopy(info->user_info_2, user_info_2, LDATA_INFO_STR_LEN);
  } else {
    STRncopy(info->user_info_2, "none", LDATA_INFO_STR_LEN);
  }

  LDATA_alloc_fcasts(handle, n_fcasts);

  info->n_fcasts = n_fcasts;
  for (i = 0; i < n_fcasts; i++) {
    handle->fcast_lead_times[i] = fcast_lead_times[i];
  }

}
  
/*************
 * init_done()
 */

static int init_done(LDATA_handle_t *handle)

{

  if (handle->init_flag == LDATA_INFO_INIT_FLAG) {
    return (TRUE);
  } else {
    return (FALSE);
  }

}

/*************************************************************************
 * read_info_file()
 *
 * Open, read and close info file.
 *
 * returns 0 on success, -1 on failure
 */

static int read_info_file(LDATA_handle_t *handle,
			  const char *file_path)
     
{

  FILE *info_file;

  /*
   * open file
   */
  
  if ((info_file = fopen(file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:LDATA_info_read\n",
	    handle->prog_name);
    fprintf(stderr, "Could not open latest data info file\n");
    perror(file_path);
    return (-1);
  }

  /*
   * perform read
   */
  
  if (do_read(handle, info_file)) {
    fclose(info_file);
    return(-1);
  }

  /*
   * debug print
   */

  if (handle->debug) {
    LDATA_info_print(handle, stderr);
  }

  /*
   * close file
   */

  fclose(info_file);
  return(0);

}

/*******************
 * write_info_file()
 */

static int write_info_file(LDATA_handle_t *handle,
			   const char *file_path,
			   const char *tmp_path)

{

  FILE *info_file;

  /*
   * open file
   */

  unlink(tmp_path);
  if ((info_file = fopen(tmp_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:LDATA_info_write\n",
	    handle->prog_name);
    fprintf(stderr, "Cannot create tmp info file\n");
    perror (tmp_path);
    return (-1);
  }

  /*
   * write the info
   */

  if (LDATA_info_print(handle, info_file)) {
    fprintf(stderr, "ERROR - %s:LDATA_info_write\n",
	    handle->prog_name);
    fprintf(stderr, "Cannot write to tmp info file\n");
    perror (tmp_path);
    fclose(info_file);
    return (-1);
  }

  /*
   * close file
   */
  
  fclose (info_file);

  /*
   * remove file to ensure that NFS cache is properly updated
   */
  
  unlink(file_path);

  /*
   * rename tmp file to current file
   */

  if(rename(tmp_path, file_path)) {
    fprintf(stderr, "ERROR - %s:LDATA_info_write\n",
	    handle->prog_name);
    fprintf(stderr, "Cannot rename '%s' to '%s'\n",
	    tmp_path, file_path);
    perror (" ");
    return (-1);
  }

  return (0);

}



