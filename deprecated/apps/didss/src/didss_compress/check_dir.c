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
 * check_dir()
 *
 * Checks for files which require compression, and performs the
 * compression.
 *
 * This may be invoked recursively.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include <time.h>
#include <sys/stat.h>

#include <toolsa/pmu.h>
#include <toolsa/str.h>

#include "didss_compress.h"

void check_dir(char *dir_path)
     
{

  char file_path[MAX_PATH_LEN];
  long year, month, day;
  long hour, min, sec;
  long before, after, time_taken;
  long sleep_time;
  DIR *dirp;
  struct dirent	*dp;
  struct stat file_stat;

  if (Glob->verbose) {
    fprintf(stderr, "Checking dir %s\n", dir_path);
  }

  /*
   * open directory file for reading
   */

  if ((dirp = opendir (dir_path)) == NULL) {
    if (Glob->verbose) {
      fprintf(stderr, "Cannot open dir %s for reading\n", dir_path);
    }
    return;
  }
  
  /*
   * read through the data directory
   */
  
  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {
    
    /*
     * exclude dir entries and files beginning with '.'
     */
    
    if (dp->d_name[0] == '.')
      continue;
    
    /*
     * compute file path
     */
    
    STRncopy(file_path, dir_path, MAX_PATH_LEN);
    STRconcat(file_path, "/", MAX_PATH_LEN);
    STRconcat(file_path, dp->d_name, MAX_PATH_LEN);
      
    /*
     * check for  subdirectories
     */
      
    if (0 != stat( file_path, &file_stat)) {
      if (Glob->verbose) {
	fprintf(stderr, "Cannot stat %s\n", file_path);
      }
      continue;
    }
      
    if (Glob->recursive && S_ISDIR(file_stat.st_mode)) {
      
      /*
       * this is a sub-directory - call this routine recursively
       */
      
      check_dir(file_path);

      /* sleep 10 seconds between checks to avoid hoging the file system */

      PMU_auto_register("Sleeping 10 secs between checks");
      sleep(10); 

      continue;
      
    }

    /*
     * If required check date_type format
     */

    if (Glob->date_format) {
      if (sscanf(dp->d_name, "%4ld%2ld%2ld",
		 &year, &month, &day) != 3) {
	continue;
      }
      if (year < 1970 || year > 3000 || month < 1 || month > 12 ||
	  day < 1 || day > 31) {
	continue;
      }
    }
      
    /*
     * If required check time_type format
     */

    if (Glob->time_format) {
      if (sscanf(dp->d_name, "%2ld%2ld%2ld",
		 &hour, &min, &sec) != 3) {
	continue;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	  sec < 0 || sec > 59) {
	continue;
      }
    }
      
    /*
     * if required, check extension
     */

    if (Glob->check_ext) {
      if (strstr(dp->d_name, Glob->ext) == NULL) {
	continue;
      }
    }

    /*
     * check that the file has not been recently accessed
     */
      
    if ((time(NULL) - file_stat.st_atime) < Glob->min_age) {
      continue;
    }

    /*
     * name is in correct format. Therefore, compress if
     * required
     */

    before = time(NULL);

    if (compress_if_required(file_path)) {

      after = time(NULL);

      time_taken = after - before;
      if (time_taken < 1) {
	time_taken = 1;
      }

      sleep_time = time_taken * Glob->sleep_factor;
      
      if (Glob->verbose) {
	fprintf(stderr, "Time taken %d secs, sleeping %d secs\n",
		(int) time_taken, (int) sleep_time);
      }
      
      PMU_auto_register("Sleeping after compression");
      
      sleep(sleep_time);

      Glob->n_compressed++;

    } /* if (compress_if_required(file_path)) */
    
    uusleep(1000); /* sleep for at least 100 msec between directory entries */
  }

  closedir(dirp);

}

