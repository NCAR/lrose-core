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
 * find_start_and_end.c
 *
 * Finds start and end date and time for data
 *
 * If start_path and end_path are not NULL, the file paths
 * are copied in there.
 *
 * Returns 0 on success, -1 if no files
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_server.h"
#include <dirent.h>

#define DIR_NAME_LEN 8

static int fname_compare(const void *, const void *);

int find_start_and_end(si32 *start_time_p,
		       si32 *end_time_p,
		       char **end_storm_file_path_p,
		       char **end_track_file_path_p)
     
{
  
  static int index_allocated = FALSE;
  static storm_file_handle_t s_handle;
  
  static char end_storm_file_path[MAX_PATH_LEN];
  static char end_track_file_path[MAX_PATH_LEN];
  
  char **file_name = NULL;
  char format_str[20];
  char ext[20];
  char file_path[MAX_PATH_LEN];
  char *ext_p;
  
  int ireturn = 0;
  
  si32 ifile;
  si32 year, month, day;
  si32 nfiles, nfiles_dir;
  si32 start_time, end_time;
  
  DIR *dirp;
  struct dirent	*dp;
  
  /*
   * initialize the file handle if needed
   */
  
  if (!index_allocated) {
    
    RfInitStormFileHandle(&s_handle, Glob->prog_name);

    index_allocated = TRUE;
    
  } /* if (!index_allocated) */
  
  /*
   * set up format string
   */
  
  sprintf(format_str, "%s%d%s",
	  "%4d%2d%2d.%", strlen(STORM_HEADER_FILE_EXT), "s");
  
  /*
   * open storm data directory file for reading
   */
  
  if ((dirp = opendir (Glob->params.storm_data_dir)) == NULL) {
    fprintf(stderr, "%s:find_start_and_end\n", Glob->prog_name);
    fprintf(stderr, "Could not open dir %s\n",
	    Glob->params.storm_data_dir);  
    return (-1);
  }
  
  /*
   * read through the directory to get the number of files
   */
  
  nfiles_dir = 0;
  
  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {
    
    if (dp != NULL)
      nfiles_dir++;
    
  }
  
  /*
   * allocate space for the file name array
   */
  
  file_name = (char **) ucalloc((ui32) nfiles_dir,
				(ui32) sizeof(char *));
  
  /*
   * read through the directory again to get the file names
   */
  
  closedir(dirp);
  
  if ((dirp = opendir (Glob->params.storm_data_dir)) == NULL) {
    fprintf(stderr, "%s:find_start_and_end\n", Glob->prog_name);
    fprintf(stderr, "Could not open dir %s\n",
	    Glob->params.storm_data_dir);  
    ireturn = -1;
    goto return_point;
  }
  
  nfiles = 0;
  
  for (ifile = 0; ifile < nfiles_dir; ifile++) {
    
    dp = readdir (dirp);
    
    /*
     * exclude dir entries and files beginning with '.'
     */
    
    if (dp->d_name[0] != '.') {
      
      /*
       * check that the file name is in the correct format
       */
      
      if (sscanf(dp->d_name, format_str,
		 &year, &month, &day, ext) != 4)
	continue;
      
      if (year < 1970 || year > 2100 || month < 1 || month > 12 ||
	  day < 1 || day > 31)
	continue;
      
      if(strcmp(ext, STORM_HEADER_FILE_EXT))
	continue;
      
      /*
       * file name is in correct format. Therefore, accept it
       */
      
      file_name[nfiles] =
	(char *) umalloc((ui32) (strlen(dp->d_name) + 1));
      strcpy(file_name[nfiles], dp->d_name);
      
      nfiles++;
      
    } /* if (dp->d_name[0] != '.') */
    
  } /* ifile */
  
  /*
   * close the directory file
   */
  
  closedir(dirp);
  
  /*
   * if there are no files, do not register, return now
   */
  
  if (nfiles == 0) {
    ireturn = -1;
    goto return_point;
  }
  
  /*
   * sort the file names
   */
  
  qsort((char *) file_name, (int) nfiles, sizeof(char *), fname_compare);
  
  /*
   * get stats on start file
   */
  
  sprintf(file_path, "%s%s%s", Glob->params.storm_data_dir,
	  PATH_DELIM, file_name[0]);
  
  /*
   * open storm properties files
   */
  
  if (RfOpenStormFiles (&s_handle, "r",
			file_path,
			(char *) NULL,
			"find_start_and_end")) {
    fprintf(stderr, "WARNING - %s:find_start_and_end\n", Glob->prog_name);
    fprintf(stderr, "Opening file %s.\n", s_handle.header_file_path);
    ireturn = -1;
    goto return_point;
  }
  
  /*
   * read in storm properties file header
   */
  
  if (RfReadStormHeader(&s_handle,
			"find_start_and_end")) {
    fprintf(stderr, "WARNING - %s:find_start_and_end\n", Glob->prog_name);
    fprintf(stderr, "Reading file %s.\n", s_handle.header_file_path);
    RfCloseStormFiles(&s_handle, "find_start_and_end");
    ireturn = -1;
    goto return_point;
  }
  
  start_time = s_handle.header->start_time;
  *start_time_p = start_time;
  
  RfCloseStormFiles(&s_handle,
		    "find_start_and_end");
  
  /*
   * get stats on end file
   */
  
  sprintf(file_path, "%s%s%s", Glob->params.storm_data_dir,
	  PATH_DELIM, file_name[nfiles - 1]);
  
  ustrncpy(end_storm_file_path, file_path, MAX_PATH_LEN);
  if (end_storm_file_path_p != NULL) {
    *end_storm_file_path_p = (char *) end_storm_file_path;
  }
  
  ustrncpy(end_track_file_path, file_path, MAX_PATH_LEN);
  ext_p = strstr(end_track_file_path, STORM_HEADER_FILE_EXT);
  *ext_p = '\0';
  ustr_concat(end_track_file_path,
	      TRACK_HEADER_FILE_EXT,
	      MAX_PATH_LEN);
  if (end_track_file_path_p != NULL) {
    *end_track_file_path_p = (char *) end_track_file_path;
  }
  
  if (RfOpenStormFiles (&s_handle, "r",
			file_path,
			(char *) NULL,
			"find_start_and_end")) {
    fprintf(stderr, "WARNING - %s:find_start_and_end\n", Glob->prog_name);
    fprintf(stderr, "Opening file %s.\n", s_handle.header_file_path);
    ireturn = -1;
    goto return_point;
  }
  
  /*
   * read in storm properties file header
   */
  
  if (RfReadStormHeader(&s_handle,
			"find_start_and_end") != R_SUCCESS) {
    fprintf(stderr, "WARNING - %s:find_start_and_end\n", Glob->prog_name);
    fprintf(stderr, "Reading file %s.\n", s_handle.header_file_path);
    RfCloseStormFiles(&s_handle, "find_start_and_end");
    ireturn = -1;
    goto return_point;
  }
  
  end_time = s_handle.header->end_time;
  *end_time_p = end_time;
  RfCloseStormFiles(&s_handle,
		    "find_start_and_end");

 return_point:

  /*
   * free up resources
   */
  
  if (file_name != NULL) {
    for (ifile = 0; ifile < nfiles; ifile++)
      if (file_name[ifile] != NULL)
	ufree((char *) file_name[ifile]);
    ufree((char *) file_name);
  }

  return (ireturn);
  
}

/*****************************************************************************
 * define function to be used for sorting
 */

static int fname_compare(const void *v1, const void *v2)

{
    char **s1, **s2;
     
    s1 = (char **) v1;
    s2 = (char **) v2;

    return strcmp(*s1, *s2);
}

