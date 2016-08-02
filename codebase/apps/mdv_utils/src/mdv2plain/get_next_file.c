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
 * get_next_file.c
 *
 * Gets the next available file - returns file name.
 *
 * Blocks until file available
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "mdv2plain.h"

#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <toolsa/pmu.h>
#include <toolsa/str.h>

static void PollDataDir(char *dir_name, int recursive);

static int New_file_found;
static char New_file_path[MAX_PATH_LEN];
static si32 Latest_file_time = 0;

char *get_next_file(void)

{

  New_file_found = FALSE;

  while (!New_file_found) {
    PMU_auto_register("Waiting for data");
      
    PollDataDir(Glob->params.input_dir, FALSE);
    if (!New_file_found) {
      sleep(5);
    }
  }

  return (New_file_path);
  
}

static void PollDataDir(char *dir_name, int recursive)
     
{
  
  /*
   * look in the data dir for new files since we last looked
   *
   * We are looking for the latest file with an age less
   * than the max_input_data_age.
   *
   * This imay be used recursively if there are subdirectories in
   * the data dir. The static 'last' keeps track of the time of
   * the last file sent, at all recursion depths.
   */
  
  DIR *dirp;
  struct dirent *dp;
  struct stat file_stat, dir_stat;
  si32 file_age;
  char fname[MAX_PATH_LEN];
  time_t now = time(NULL);
  
  /* see if the directory entry has changed  - non-recursive ops only */
  
  if (!recursive) {

    if (0 != stat(dir_name, &dir_stat)) {
      fprintf(stderr, "ERROR - %s:get_next_file\n", Glob->prog_name);
      fprintf(stderr, "Cannot stat '%s'\n", dir_name);
      return;
    }

    if (Latest_file_time == dir_stat.st_ctime) {
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stderr, "directory not changed at %s",
		utimstr(now));
      }
      return;
    }

    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "directory changed at %s",
	      utimstr(dir_stat.st_ctime));
    }

  } /* if (!recursive) */
  
  /*
   * look for new files
   */

  dirp = opendir(dir_name);

  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    STRncopy(fname, dir_name, MAX_PATH_LEN);
    STRconcat(fname, "/", MAX_PATH_LEN);
    STRconcat(fname, dp->d_name, MAX_PATH_LEN);
    
    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr, "Checking file %s\n", fname);
    }
    
    if (0 != stat(fname, &file_stat)) {
      fprintf(stderr, "cant stat file %s\n", fname);
      continue;
    }
    
    if (recursive && S_ISDIR(file_stat.st_mode) &&
	(fname[strlen(fname) - 1] != '.')) {

      /*
       * this is a sub-directory - call this routine recursively
       */
      
      PollDataDir(fname, recursive);
      continue;

    }

    /*
     * check file extension
     */
    
    if (NULL == strstr(dp->d_name, Glob->params.input_file_ext)) {
      continue;
    }
    
    file_age = now - file_stat.st_ctime;
    
    if (file_age < Glob->params.max_input_data_age &&
	Latest_file_time < file_stat.st_ctime) {

      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stderr, "new file %s\n      changed at %s", fname, 
		utimstr(file_stat.st_ctime));
      }
      
      STRncopy(New_file_path, fname, MAX_PATH_LEN);
      New_file_found = TRUE;
      Latest_file_time = file_stat.st_ctime;
      
    } /* if (file_age ... */
    
  } /* for (dp = .... */

  closedir(dirp);
  return;

}

