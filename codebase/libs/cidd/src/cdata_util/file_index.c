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
/*************************************************************
 * FILE_INDEX.C
 *
 * Routines for handling the current file index
 *
 * Public routines are:
 *
 *  cdata_write_index_simple()
 *  cdata_write_index_simple2()
 *  cdata_read_index_simple()
 *
 * Mike Dixon.
 */

#include <toolsa/os_config.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <cidd/cdata_util.h>
#include <toolsa/utim.h>

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 1024
#endif

#ifndef PATH_DELIM
#define PATH_DELIM "/"
#endif

extern time_t time(time_t *);

static int read_index(char *dir,
		      int wait_msec,
		      long *prev_seq_num,
		      long max_valid_file_age,
		      cdata_current_index_t *index,
		      char *prog_name,
		      char *calling_routine,
		      int debug,
		      int read_fcasts);

static int read_index_file(char *index_file_path,
			   cdata_current_index_t *index,
			   char *prog_name,
			   char *calling_routine,
			   int debug,
			   int read_fcasts);

/*******************************************************************
 * cdata_index_init()
 *
 * Initialize the index struct for malloc'ing
 */

void cdata_index_init(cdata_current_index_t *index)
     
{
  
  index->n_fcasts = 0;
  index->n_fcasts_alloc = 0;
  index->fcast_times = NULL;

  return;

}

/*******************************************************************
 * cdata_index_alloc()
 *
 * Alloc space for forecasts
 *
 * Returns 0 on success, -1 on malloc error
 */

int cdata_index_alloc(cdata_current_index_t *index,
		      int n_fcasts,
		      char *prog_name,
		      char *calling_routine)
     
{
  
  if (n_fcasts > index->n_fcasts_alloc) {
    if (index->fcast_times == NULL) {
      index->fcast_times = (long *)  malloc(n_fcasts * sizeof(long));
    } else {
      index->fcast_times = (long *) realloc((char *) index->fcast_times,
				   n_fcasts * sizeof(long));
    }
    if (index->fcast_times == NULL) {
      fprintf(stderr, "ERROR - %s:%s:cdata_index_alloc\n",
	      prog_name, calling_routine);
      fprintf(stderr, "Malloc or realloc error, %d bytes\n",
	      n_fcasts * sizeof(long));
      return (-1);
    }
    index->n_fcasts_alloc = n_fcasts;
  }

  index->n_fcasts = n_fcasts;
  return (0);

}

/*****************************************************************
 * FILL_CURRENT_INDEX: Fill in structure members of a 
 * cdata_current_index_t data struct.
 *
 * If suffix, id1 or id2 are NULL, the relevant strings are
 * set empty.
 *
 */

void fill_current_index(cdata_current_index_t* index, time_t tsec,
			char* suffix, char* id1, char* id2)

{

  UTIMstruct tm;

  UTIMunix_to_date(tsec, &tm);

  index->year = tm.year;
  index->month = tm.month;
  index->day = tm.day;
  index->hour = tm.hour;
  index->min = tm.min;
  index->sec = tm.sec;
  index->unix_time = tm.unix_time;
  index->exact_time = TRUE;
  index->n_fcasts = 0;
  
  if (suffix != NULL) {
    strncpy(index->file_ext,suffix,CDATA_MAX_INDEX_STR);
  } else {
    index->file_ext[0] = '\0';
  }

  if (id1 != NULL) {
    strncpy(index->id1,id1,CDATA_MAX_INDEX_STR);
  } else {
    index->id1[0] = '\0';
  }

  if (id2 != NULL) {
    strncpy(index->id2,id2,CDATA_MAX_INDEX_STR);
  } else {
    index->id2[0] = '\0';
  }

}
  
/*****************************************************************
 * cdata_write_index_simple()
 *
 * Writes a cdata_file_index which does not contain
 * any forecasts.
 *
 * Writes to a tmp file first, then moves the tmp file to
 * the final file name when done.
 *
 * On success, returns a pointer to the path of the index file,
 * on failure return NULL
 */

char *cdata_write_index_simple(char *dir,
			       cdata_current_index_t *index,
			       char *prog_name,
			       char *calling_routine)

{

  /*
   * make sure n_fcasts is set to 0
   */
  
  index->n_fcasts = 0;
  
  /*
   * write the index
   */
  
  return (cdata_write_index(dir, index, prog_name, calling_routine));

}

/*****************************************************************
 * cdata_write_index_simple2()
 *
 * A variation on cdata_write_index_simple which fills out
 * the cdata_current_index_t based on the unix time argument
 *
 * (optional id fields are set to "unknown")
 * (exact_time is set to TRUE)
 *
 * On success, returns a pointer to the path of the index file,
 * on failure return NULL
 */

char *cdata_write_index_simple2(char *dir,
                                char *ext,
                                time_t indexTime,
                                char *prog_name,
                                char *calling_routine)

{
   UTIMstruct when;
   cdata_current_index_t cindex;

   UTIMunix_to_date( indexTime, &when );

   cindex.n_fcasts = 0;
   cindex.year       = when.year;
   cindex.month      = when.month;                           
   cindex.day        = when.day;
   cindex.hour       = when.hour;
   cindex.min        = when.min;
   cindex.sec        = when.sec;                   
   cindex.unix_time  = indexTime;
   cindex.exact_time = TRUE;

   memset( (void *)cindex.file_ext, 0, CDATA_MAX_INDEX_STR );
   memset( (void *)cindex.id1, 0, CDATA_MAX_INDEX_STR );
   memset( (void *)cindex.id2, 0, CDATA_MAX_INDEX_STR );
   strcpy( cindex.file_ext, ext);
   strcpy( cindex.id1, "unknown");
   strcpy( cindex.id2, "unknown");

  /*
   * write the index
   */

  return (cdata_write_index(dir, &cindex, prog_name, calling_routine));

}
/*****************************************************************
 i cdata_write_index()
 *
 * Writes a cdata_file_index file
 *
 * Writes to a tmp file first, then moves the tmp file to
 * the final file name when done.
 *
 * On success, returns a pointer to the path of the index file,
 * on failure return NULL
 */

char *cdata_write_index(char *dir,
			cdata_current_index_t *index,
			char *prog_name,
			char *calling_routine)
     
{

  static int first_call = TRUE;
  static long seq_num;
  static char current_index_path[MAX_PATH_LEN];
  char prev_index_path[MAX_PATH_LEN];
  char tmp_index_path[MAX_PATH_LEN];
  int i;
  FILE *index_file;

  if (first_call) {

    srand(time(NULL));
    seq_num = rand() % 10000;
    if (seq_num == 0) {
      seq_num = 1;
    }
    first_call = FALSE;

  }

  sprintf(current_index_path, "%s%s%s",
	  dir, PATH_DELIM, CDATA_CURRENT_FILE_INDEX);

  sprintf(prev_index_path, "%s%s%s",
	  dir, PATH_DELIM, CDATA_PREV_FILE_INDEX);
  
  sprintf(tmp_index_path, "%s%s%s",
	  dir, PATH_DELIM, "tmp_file_index");

  if ((index_file = fopen(tmp_index_path, "w")) == NULL) {

    fprintf(stderr, "ERROR - %s:%s:cdata_write_index_simple\n",
	    prog_name, calling_routine);
    fprintf(stderr, "Cannot create tmp index file\n");
    perror (tmp_index_path);
    return ((char *) NULL);

  }

  /*
   * increment seq_num
   */

  seq_num++;
  index->seq_num = seq_num;

  /*
   * write the data
   */

  if ((fprintf(index_file,
	       "%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n",
	       index->seq_num,
	       index->year,
	       index->month,
	       index->day,
	       index->hour,
	       index->min,
	       index->sec,
	       index->unix_time) == EOF) ||
      (fprintf(index_file,
	       "%ld\n%s\n%ld\n",
	       index->exact_time,
	       index->file_ext,
	       index->n_fcasts) == EOF)) {

    fprintf(stderr, "ERROR - %s:%s:cdata_write_index\n",
	    prog_name, calling_routine);
    fprintf(stderr, "Cannot write to tmp index file\n");
    perror (tmp_index_path);
    fclose(index_file);
    return ((char *) NULL);

  }

  for (i = 0; i < index->n_fcasts; i++) {
    if (fprintf(index_file, "%ld\n", index->fcast_times[i]) == EOF) {
      fprintf(stderr, "ERROR - %s:%s:cdata_write_index\n",
	      prog_name, calling_routine);
      fprintf(stderr, "Cannot write to tmp index file\n");
      perror (tmp_index_path);
      fclose(index_file);
      return ((char *) NULL);
    }
  } /* i */
      
  if (index->id1[0] == '\0') {
    strcpy(index->id1, "unknown");
  }

  if (index->id2[0] == '\0') {
    strcpy(index->id2, "unknown");
  }

  if (fprintf(index_file,
	      "%s\n%s\n",
	      index->id1,
	      index->id2) == EOF) {
    fprintf(stderr, "ERROR - %s:%s:cdata_write_index\n",
	    prog_name, calling_routine);
    fprintf(stderr, "Cannot write to tmp index file\n");
    perror (tmp_index_path);
    fclose(index_file);
    return ((char *) NULL);
  }

  fclose (index_file);

  /*
   * rename current file to prev file (if it exists)
   */

  rename(current_index_path, prev_index_path);

  /*
   * rename tmp file to current file
   */
  
  if(rename(tmp_index_path, current_index_path)) {
    
    fprintf(stderr, "ERROR - %s:%s:cdata_write_index_simple\n",
	    prog_name, calling_routine);
    fprintf(stderr, "Cannot rename '%s' to '%s'\n",
	    tmp_index_path, current_index_path);
    perror (tmp_index_path);
    return ((char *) NULL);
    
  }
  
  return (current_index_path);
  
}

/*********************************************************************
  * cdata_read_index_fcasts()
  *
  * Read the struct data from the current file index, including the
  * forecast lead times.
  *
  * Retry for a maximum of wait_msec number of milliseconds before
  * giving up.
  *
  * Return values:
  *
  * If wait_msec > 0
  *
  *   Returns 0 when file exists, is newer that max_valid_file_age,
  *   and has a sequence number != prev_seq_num.
  *   If *prev_seq_num is NULL, seq num check is ignored.
  *   If *prev_seq_num is non-NULL, sets prev_seq_number to current
  *   seq_num.
  *
  *   Returns -1 on time out.
  *
  *   Returns -2 on system or file format errors;
  *
  * If wait_msec == 0
  *
  *   Returns 0 when file exists and is read
  *   Returns -2 on error.
  *
  * Typically, the calling routine declares prev_seq_num as static,
  * initialized to 0 before the initial call.
  *
  *********************************************************************/

  
int cdata_read_index_fcasts(char *dir,
			    int wait_msec,
			    long *prev_seq_num,
			    long max_valid_file_age,
			    cdata_current_index_t *index,
			    char *prog_name,
			    char *calling_routine,
			    int debug)
     
{

  return (read_index(dir,
		     wait_msec,
		     prev_seq_num,
		     max_valid_file_age,
		     index,
		     prog_name,
		     calling_routine,
		     debug,
		     TRUE));

}
 
/*********************************************************************
  * cdata_read_index_simple()
  *
  * Read the struct data from the current file index.
  *
  * Retry for a maximum of wait_msec number of milliseconds before
  * giving up.
  *
  * Return values:
  *
  * If wait_msec > 0
  *
  *   Returns 0 when file exists, is newer that max_valid_file_age,
  *   and has a sequence number != prev_seq_num.
  *   If *prev_seq_num is NULL, seq num check is ignored.
  *   If *prev_seq_num is non-NULL, sets prev_seq_number to current
  *   seq_num.
  *
  *   Returns -1 on time out.
  *
  *   Returns -2 on system or file format errors;
  *
  * If wait_msec == 0
  *
  *   Returns 0 when file exists and is read
  *   Returns -2 on error.
  *
  * Typically, the calling routine declares prev_seq_num as static,
  * initialized to 0 before the initial call.
  *
  *********************************************************************/
  
int cdata_read_index_simple(char *dir,
			    int wait_msec,
			    long *prev_seq_num,
			    long max_valid_file_age,
			    cdata_current_index_t *index,
			    char *prog_name,
			    char *calling_routine,
			    int debug)
     
{

  return (read_index(dir,
		     wait_msec,
		     prev_seq_num,
		     max_valid_file_age,
		     index,
		     prog_name,
		     calling_routine,
		     debug,
		     FALSE));

}

/*********************************************************************
  * cdata_remove_index()
  *
  * Removes the index file from a directory.
  *
  * Returns 0 on success, -1 on failure.
  *
  *********************************************************************/
  
int cdata_remove_index(char *dir,
		       char *prog_name,
		       char *calling_routine)
     
{
  
  int iret = 0;
  char current_index_path[MAX_PATH_LEN];
  char prev_index_path[MAX_PATH_LEN];

  sprintf(current_index_path, "%s%s%s",
	  dir, PATH_DELIM, CDATA_CURRENT_FILE_INDEX);

  sprintf(prev_index_path, "%s%s%s",
	  dir, PATH_DELIM, CDATA_PREV_FILE_INDEX);
  
  if (unlink(current_index_path)) {
    fprintf(stderr, "WARNING - %s:%s\n",
	    prog_name, calling_routine);
    fprintf(stderr, "Cannot remove current index file\n");
    perror(current_index_path);
    iret = -1;
  }
  
  if (unlink(prev_index_path)) {
    fprintf(stderr, "WARNING - %s:%s\n",
	    prog_name, calling_routine);
    fprintf(stderr, "Cannot remove prev index file\n");
    perror(prev_index_path);
    iret = -1;
  }
  
  return (iret);

}

/*********************************************************************/
/* Returns TRUE if time1 is greater than time2, FALSE Otherwise */
static int t1_gt_t2(struct timeval *t1,struct timeval *t2) 
{
   /* Handle differences in the seconds */
   if(t1->tv_sec < t2->tv_sec) return FALSE;
   if(t1->tv_sec > t2->tv_sec) return TRUE;
    
   /* Only different by less than a second */
   if(t1->tv_usec > t2->tv_usec)  return TRUE;

   return FALSE;
}
 
/*********************************************************************
  * read_index()
  *
  * Read the struct data from the current file index, including forecast
  * lead times if requested.
  *
  * Retry for a maximum of wait_msec number of milliseconds before
  * giving up.
  *
  * Return values:
  *
  * If wait_msec > 0
  *
  *   Returns 0 when file exists, is newer that max_valid_file_age,
  *   and has a sequence number != prev_seq_num.
  *   If *prev_seq_num is NULL, seq num check is ignored.
  *   If *prev_seq_num is non-NULL, sets prev_seq_number to current
  *   seq_num.
  *
  *   Returns -1 on time out.
  *
  *   Returns -2 on system or file format errors;
  *
  * If wait_msec == 0
  *
  *   Returns 0 when file exists and is read
  *   Returns -2 on error.
  *
  * Typically, the calling routine declares prev_seq_num as static,
  * initialized to 0 before the initial call.
  *
  *********************************************************************/

static int read_index(char *dir,
		      int wait_msec,  
		      long *prev_seq_num, 
		      long max_valid_file_age, 
		      cdata_current_index_t *index,
		      char *prog_name,
		      char *calling_routine,
		      int debug,
		      int read_fcasts)
     
{
 
  char current_index_path[MAX_PATH_LEN];
  
  long sleep_usecs;
  struct stat stats;
  struct timeval tv_now;
  struct timeval expire_time;
  time_t file_age;

  if(prev_seq_num == NULL) return -2;

  /*
   * if 1 msec  Forces blocking -  For backward compatability
   */
  
  if (wait_msec == 1 ) {
    wait_msec = -1; 
  }
  
  sprintf(current_index_path, "%s%s%s",
	  dir, PATH_DELIM, CDATA_CURRENT_FILE_INDEX);
  
  /*
   * Compute a sleep interval in usecs - base it on what the caller
   * is willing to wait
   */

  if(wait_msec > 0 && wait_msec < 1000) {
     sleep_usecs = wait_msec * 500; /* set sleep time to 1/2 wait period */
  } else {
     sleep_usecs = 1000000; /* set sleep time to 1 second */
  }

  /*
   * Compute an expire time - when this routine should time out
   */

  gettimeofday(&tv_now, NULL);
  
  expire_time.tv_sec = tv_now.tv_sec + (wait_msec / 1000);
  expire_time.tv_usec =  tv_now.tv_usec + ((wait_msec % 1000) * 1000);

  if(expire_time.tv_usec >= 1000000) {
    expire_time.tv_sec +=1;
    expire_time.tv_usec -= 1000000;
  }
  
  /*
   * Wait for index file to exist
   */

  while (stat(current_index_path, &stats)) {
    
    if (wait_msec == 0) {
      
      /*
       * if non-blocking, return now because file does not exist
       */
      
      return (-1);
      
    } else {

      if (errno != ENOENT) {
	fprintf(stderr, "ERROR - %s:%s:cdata_read_index_simple\n",
		prog_name, calling_routine);
	fprintf(stderr, "Could not obtain stats on index file\n");
	perror(current_index_path);
	return (-2);
      }

    }
    
    gettimeofday(&tv_now, NULL);

    if (t1_gt_t2(&tv_now, &expire_time)) {
      return (-1);
    }

    tu_sleep(sleep_usecs);

  } /* while */


  if (debug) {
    fprintf(stderr, "cdata_read_index_simple : index file exists\n");
  }
  
  if (wait_msec == 0) {

    /*
     * do not block - read and return if error
     */
    
    if (read_index_file(current_index_path,
			index,
			prog_name,
			calling_routine,
			debug,
			read_fcasts)) {
      return(-2);
    }
    
  } else {

    /*
     * wait until file is more recent than max valid age
     * or we have exceeded the expire time
     */
  
    gettimeofday(&tv_now, NULL);

    file_age = tv_now.tv_sec - stats.st_mtime;
    
    while ( file_age > max_valid_file_age) {
      
      stat(current_index_path, &stats);
       
      gettimeofday(&tv_now, NULL);

      if(t1_gt_t2(&tv_now,&expire_time)) return  -1;

      file_age = tv_now.tv_sec - stats.st_mtime;
       
      tu_sleep(sleep_usecs);
       
    } /* while */

    
    if (debug) {
      fprintf(stderr,
	      "cdata_read_index_simple : index file recent enough\n");
    }
    
    /*
     * if prev_seq_num is non-NULL, wait until sequence number
     * changes.
     */
    
    if (prev_seq_num != NULL) {

      index->seq_num = *prev_seq_num;
      
      while (index->seq_num == *prev_seq_num) {
	
	if (read_index_file(current_index_path,
			    index,
			    prog_name,
			    calling_routine,
			    FALSE,
			    read_fcasts)) {
	  
	  index->seq_num = *prev_seq_num;
	  
	} /* if (read_index_file ...) */
	
	/*
	 * wait a short while if the seq_num hasn't changed
	 */
	
	if(index->seq_num == *prev_seq_num) {

	  tu_sleep(sleep_usecs);
	  gettimeofday(&tv_now, NULL);

	  if(t1_gt_t2(&tv_now,&expire_time)) {
	    return (-1);
	  }

	}
      
      } /* while (index->seq_num == *prev_seq_num) */
    
      *prev_seq_num = index->seq_num;  /* Sets prev_seq_num on exit */

    } /* if (prev_seq_num != NULL) */

  } /* if (wait_msec != 0) */
    
  if (debug) {
    fprintf(stderr, "cdata_read_index_simple : done\n");
  }
    
  return (0);
  
}

/*************************************************************************
 * read_index_file()
 *
 * returns 0 on success, 1 on failure
 */

static int read_index_file(char *current_index_path,
			   cdata_current_index_t *index,
			   char *prog_name,
			   char *calling_routine,
			   int debug,
			   int read_fcasts)

{

  int i;
  FILE *index_file;

  /*
   * open file
   */
  
  if ((index_file = fopen(current_index_path, "r")) == NULL) {
    
    if (debug) {
      fprintf(stderr, "WARNING - %s:%s:cdata_read_index\n",
	      prog_name, calling_routine);
      fprintf(stderr, "could not open file '%s'\n", current_index_path);
      perror(current_index_path);
    }
    
    return (-1);
    
  }
  
  /*
   * read
   */

  if ((fscanf(index_file,
	      "%ld%ld%ld%ld%ld%ld%ld%ld",
	      &index->seq_num,
	      &index->year,
	      &index->month,
	      &index->day,
	      &index->hour,
	      &index->min,
	      &index->sec,
	      &index->unix_time) != 8) ||
      (fscanf(index_file,
	      "%ld%s%ld",
	      &index->exact_time,
	      (char *)index->file_ext,
	      &index->n_fcasts) != 3)) {
    
    fprintf(stderr, "ERROR - %s:%s:cdata_read_index\n",
	    prog_name, calling_routine);
    fprintf(stderr, "fscanf failed - '%s'\n", current_index_path);
    perror(current_index_path);
    fclose(index_file);
    return (-1);
    
  }

  if (read_fcasts) {

    if (cdata_index_alloc(index,
			  index->n_fcasts,
			  prog_name,
			  calling_routine)) {
      fclose(index_file);
      return (-1);
    }

    for(i=0; i < index->n_fcasts; i++ ) {
      if ((fscanf(index_file, "%ld", index->fcast_times + i)) != 1) {
        fprintf(stderr, "ERROR - %s:%s:cdata_read_index\n",
	      prog_name, calling_routine);
        fprintf(stderr, "fscanf failed - '%s'\n", current_index_path);
        perror(current_index_path);
        fclose(index_file);
        return (-1);
      }
    }
    
  } /* if (read_fcasts) */
      
  if (fscanf(index_file,
	     "%s%s",
	     (char *)index->id1,
	     (char *)index->id2) != 2) {
    
    fprintf(stderr, "ERROR - %s:%s:cdata_read_index\n",
	    prog_name, calling_routine);
    fprintf(stderr, "fscanf failed - '%s'\n", current_index_path);
    perror(current_index_path);
    fclose(index_file);
    return (-1);
  }

  fclose(index_file);

  if (debug)
    fprintf(stderr, "latest time : %.4ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld\n",
	    index->year, index->month, index->day,
	    index->hour, index->min, index->sec);
  
  return (0);

}
