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

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>

/*************************************************
 * Directory creation routine
 *
 * Nancy Rehak, Rap, NCAR, Boulder, CO, 80303, USA
 *
 * Updated by Mike Dixon, Feb 1999.
 *
 * March 1996
 */


/********************************************************
 * makedir()
 *
 * Utility routine to create a directory.  If the directory
 * already exists, does nothing.
 *
 * Returns -1 on error, 0 otherwise.
 */

int makedir(const char *path)
{
  return (ta_makedir(path));
}

/********************************************************
 * ta_makedir()
 *
 * Utility routine to create a directory.  If the directory
 * already exists, does nothing.
 *
 * Returns -1 on error, 0 otherwise.
 */

int ta_makedir(const char *path)
{
  struct stat stat_buf;
  
  /*
   * Status the directory to see if it already exists.
   */

  if (stat(path, &stat_buf) == 0) {
    return(0);
  }
  
  /* this check seems to cause problems on Linux because errno is
   * not set correctly after stat - dixon
   *
   * if (errno != ENOENT) {
   *   return(-1); 
   * }
   */
  
  /*
   * Directory doesn't exist, create it.
   */

  if (mkdir(path,
	    S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
    /*
     * failed
     * check if dir has been made bu some other process
     * in the mean time, in which case return success
     */
    if (stat(path, &stat_buf) == 0) {
      return(0);
    }
    return(-1);
  }
  
  return(0);
}


/********************************************************
 * ta_makedir_recurse()
 *
 * Utility routine to create a directory recursively.
 * If the directory already exists, does nothing.
 * Otherwise it recurses through the path, making all
 * needed directories.
 *
 * Returns -1 on error, 0 otherwise.
 */

int ta_makedir_recurse(const char *path)
{

  char up_dir[MAX_PATH_LEN];
  char *last_delim;
  struct stat dir_stat;
  int delim = PATH_DELIM[0];
  
  /*
   * Status the directory to see if it already exists.
   * '/' dir will always exist, so this stops the recursion
   * automatically.
   */
  
  if (stat(path, &dir_stat) == 0) {
    return(0);
  }
  
  /*
   * create up dir - one up the directory tree -
   * by searching for the previous delim and removing it
   * from the string.
   * If no delim, try to make the directory non-recursively.
   */
  
  STRncopy(up_dir, path, MAX_PATH_LEN);
  last_delim = strrchr(up_dir, delim);
  if (last_delim == NULL) {
    return (ta_makedir(up_dir));
  }
  *last_delim = '\0';
  
  /*
   * make the up dir
   */
  
  if (ta_makedir_recurse(up_dir)) {
    return (-1);
  }

  /*
   * make this dir
   */

  if (ta_makedir(path)) {
    return(-1);
  } else {
    return(0);
  }

}

/********************************************************
 * ta_makedir_for_file()
 *
 * Utility routine to create a directory recursively,
 * given a file path. The directory name is determined
 * by stripping the file name off the end of the file path.
 * If the directory already exists, does nothing.
 * Otherwise it recurses through the path, making all
 * needed directories.
 *
 * Returns -1 on error, 0 otherwise.
 */

int ta_makedir_for_file(const char *file_path)
{
  
  char *delim, *lastDelim;
  int delimLen = strlen(PATH_DELIM);

  /*
   * get dir path from file path
   */

  delim = strstr(file_path, PATH_DELIM);
  if (delim == NULL) {
    /*
     * no delim, so file goes in current directory
     * no need to make dir
     */
    return 0;
  }

  lastDelim = delim; 
  while (delim != NULL) {
    delim = strstr(lastDelim + delimLen, PATH_DELIM);
    if (delim != NULL) {
      lastDelim = delim;
    }
  }

  /*
   * dir path extends to last delim
   */

  {
    int dirPathLen = lastDelim - file_path;
    char *dir_path = (char *) umalloc(dirPathLen + 1);
    strncpy(dir_path, file_path, dirPathLen);
    dir_path[dirPathLen] = '\0';
    if (ta_makedir_recurse(dir_path)) {
      free(dir_path);
      return -1;
    }
    free(dir_path);
  }

  return 0;

}




