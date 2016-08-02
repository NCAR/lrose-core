// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * register_server.c
 *
 * Registers this server with the server mapper
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "MDV_server.hh"
#include <toolsa/smu.h>
#include <ctime>
#include <dirent.h>
#include <netinet/in.h>
using namespace std;

#define DIR_NAME_LEN 8

static void Fill_info(si32 *data_start_time_p,
		      si32 *data_end_time_p,
		      si32 *last_data_time_p,
		      si32 *last_request_time_p,
		      si32 *n_data_requests_p);

static int str_compare(const void *, const void *);

static int search_data_dir(char *data_dir_name,
			   si32 *data_start_time,
			   si32 *data_end_time);

static int search_subdir(char *data_dir_name,
			 char *subdir_name,
			 si32 *data_start_time,
			 si32 *data_end_time);

void register_server_init(void)

{

  SMU_auto_init(Glob->params.type,
		Glob->params.subtype,
		Glob->params.instance,
		Glob->params._data_dirs[0],
		Glob->params.port,
		Glob->params.realtime_avail,
		Fill_info);

  Glob->time_last_request = 0;
  Glob->n_data_requests = 0;
  Glob->latest_data_time = 0;

}

void unregister_server(void)

{
  
  SMU_auto_unregister();

}

/*********************************************************
 * Fill_info()
 * 
 * Function called by the smu_auto routines to provide the 
 * data and request times
 */

static void Fill_info(si32 *data_start_time_p,
		      si32 *data_end_time_p,
		      si32 *last_data_time_p,
		      si32 *last_request_time_p,
		      si32 *n_data_requests_p)
     
{

  int data_found;

  si32 idir;
  si32 data_start_time;
  si32 data_end_time;

  data_start_time = LARGE_LONG;
  data_end_time = 0;
  data_found = FALSE;

  for (idir = 0; idir < Glob->params.data_dirs.len; idir++) {
    
    if (!search_data_dir(Glob->params.data_dirs.val[idir],
			 &data_start_time, &data_end_time)) {
      data_found = TRUE;
    }
    
  } /* idir */

  if (data_found) {
    *data_start_time_p = data_start_time;
    *data_end_time_p = data_end_time;
  } else {
    *data_start_time_p = 0;
    *data_end_time_p = 0;
  }

  if (Glob->params.use_realtime_file) {
    *last_data_time_p = Glob->latest_data_time;
  } else {
    *last_data_time_p = data_end_time;
  } /* if (Glob->shmem_avail) */

  *last_request_time_p = Glob->time_last_request;
  *n_data_requests_p = Glob->n_data_requests;

  return;
  
}

/**********************************************************************
 * search_data_dir()
 * 
 * performs a search below the given directory for the start time and end
 * time of the data
 *
 * returns 0 on success, -1 on failure (no data found)
 */

static int search_data_dir(char *data_dir_name,
			   si32 *data_start_time,
			   si32 *data_end_time)

{

  char **subdir_name;
  int data_found;
  int start_dir;
  long year, month, day;
  si32 idir;
  si32 nsubdir, nentries;
  DIR *dirp;
  struct dirent	*dp;

  /*
   * open directory file for reading
   */

  if ((dirp = opendir (data_dir_name)) == NULL)
    return (-1);
  
  /*
   * read through the data directory to get the number of entries
   */
  
  nentries = 0;
  
  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {
      nentries++;
  }
  
  /*
   * allocate space for the subdir name array
   */

  subdir_name = (char **) umalloc((ui32) (nentries * sizeof(char *)));
  
  /*
   * read through the directory again to get the valid ones
   */
  
  closedir(dirp);
  if ((dirp = opendir (data_dir_name)) == NULL)
    return (-1);
  
  nsubdir = 0;
  
  for (idir = 0; idir < nentries; idir++) {

    dp = readdir (dirp);

    /*
     * exclude dir entries and files beginning with '.'
     */

    if (dp->d_name[0] == '.')
      continue;
      
    /*
     * check that the dir name is in the correct format
     */

    if (sscanf(dp->d_name, "%4ld%2ld%2ld",
	       &year, &month, &day) != 3)
      continue;
      
    if (year < 1970 || year > 2100 || month < 1 || month > 12 ||
	day < 1 || day > 31)
      continue;

    /*
     * name is in correct format. Therefore, accept it
     */

    subdir_name[nsubdir] =
      (char *) umalloc((ui32) (strlen(dp->d_name) + 1));
    strcpy(subdir_name[nsubdir], dp->d_name);
      
    nsubdir++;

  } /* idir */

  /*
   * close the directory file
   */

  closedir(dirp);

  if (nsubdir == 0) {

    ufree((char *) subdir_name);
    return (-1);

  }

  /*
   * sort the names
   */
	
  qsort((char *) subdir_name, (int) nsubdir, sizeof(char *), str_compare);

  /*
   * search for first subdirectory with valid data
   */

  data_found = FALSE;
  for (idir = 0; idir < nsubdir; idir++) {
    if (search_subdir(data_dir_name, subdir_name[idir], 
		      data_start_time, data_end_time) == 0) {
      data_found = TRUE;
      start_dir = idir;
      break;
    }
  }

  if (data_found) {

    /*
     * search for last subdirectory with valid data
     */
    
    for (idir = nsubdir - 1; idir > start_dir; idir--) {
      if (search_subdir(data_dir_name, subdir_name[idir], 
			data_start_time, data_end_time) == 0) {
	break;
      }
    }
    
  } /* if (data_found) */
  
  /*
   * free up resources
   */
  
  for (idir = 0; idir < nsubdir; idir++)
    ufree((char *) subdir_name[idir]);
  ufree((char *) subdir_name);

  if (data_found) {
    return (0);
  } else {
    return (-1);
  }
  
}

/**********************************************************************
 * search_subdir()
 * 
 * performs a search in the given directory for the start time and end
 * time of the data
 *
 * returns 0 on success, -1 on failure (no data found)
 */

static int search_subdir(char *data_dir_name,
			 char *subdir_name,
			 si32 *data_start_time,
			 si32 *data_end_time)

{

  char **file_name;
  char dir_path[BUFSIZ];
  
  si32 ifile;
  long hour, min, sec;
  si32 nfiles, nentries;

  date_time_t file_time;

  DIR *dirp;
  struct dirent	*dp = NULL;

  /*
   * open data directory file for reading
   */
  
  sprintf(dir_path, "%s%s%s",
	  data_dir_name, PATH_DELIM, subdir_name);
  
  if ((dirp = opendir (dir_path)) == NULL)
    return (-1);

  /*
   * read through the directory to get the number of entries
   */
  
  nentries = 0;
  
  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {

    if (dp != NULL)
      nentries++;

  } /* dp */

  /*
   * allocate space for the file name array
   */

  file_name = (char **) umalloc((ui32) (nentries * sizeof(char *)));
  
  /*
   * read through the directory again to get the valid file names
   */
  
  closedir(dirp);
  if ((dirp = opendir (dir_path)) == NULL)
    return (-1);

  nfiles = 0;

  for (ifile = 0; ifile < nentries; ifile++) {
    
    dp = readdir (dirp);

    /*
     * exclude dir entries and files beginning with '.'
     */

    if (dp->d_name[0] == '.')
      continue;
      
    /*
     * check that the file name is in the correct format
     */

    if (sscanf(dp->d_name, "%2ld%2ld%2ld",
	       &hour, &min, &sec) != 3)
      continue;
      
    if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	sec < 0 || sec > 59)
      continue;

    /*
     * file name is in correct format. Therefore, accept it
     */

    file_name[nfiles] =
      (char *) umalloc((ui32) (strlen(dp->d_name) + 1));
    strcpy(file_name[nfiles], dp->d_name);
      
    nfiles++;

  } /* ifile */

  /*
   * close the directory file
   */

  closedir(dirp);

  /*
   * if there are no files, return now
   */

  if (nfiles == 0) {

    ufree((char *) file_name);
    return (-1);

  }

  /*
   * sort the file names
   */
	
  qsort((char *) file_name, (int) nfiles, sizeof(char *), str_compare);

  /*
   * load time struct from subdir name and last file name,
   * and check if this time is earlier that the current
   * data_start_time
   */

  sscanf(subdir_name, "%4d%2d%2d",
	 &file_time.year, &file_time.month, &file_time.day);
  
  sscanf(file_name[0], "%2d%2d%2d",
	 &file_time.hour, &file_time.min, &file_time.sec);

  uconvert_to_utime(&file_time);

  if (file_time.unix_time < *data_start_time)
    *data_start_time = file_time.unix_time;

  /*
   * load time struct from subdir name and last file name,
   * and check if this time is later than the current
   * data_end_time
   */

  sscanf(file_name[nfiles - 1], "%2d%2d%2d",
	 &file_time.hour, &file_time.min, &file_time.sec);
  
  uconvert_to_utime(&file_time);

  if (file_time.unix_time > *data_end_time)
    *data_end_time = file_time.unix_time;

  /*
   * free up resources
   */
  
  for (ifile = 0; ifile < nfiles; ifile++)
    ufree((char *) file_name[ifile]);
  ufree((char *) file_name);

  return (0);

}
  

/*****************************************************************************
 * define function to be used for sorting
 */

static int str_compare(const void *v1, const void *v2)

{
    char **s1, **s2;
    s1 = (char **) v1;
    s2 = (char **) v2;
    return strcmp(*s1, *s2);
}

