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
/*****************************************************************************
 * CIDD_FILE_SEARCH.c Routines to find data files using the standard CIDD 
 * Naming convention style: top_dir/yyyymmdd/hhmmss.suffix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>
#include <dirent.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>
#include <toolsa/file_io.h>
#include <cidd/cdata_util.h>

#define CIDD_FILE_SEARCH
#define MAX_LONG 2147483647
#define MIN_LONG -2147483647
#define DIR_READ_DELAY 1	/* Delay reexamining directories for data this many seconds */

/************************************************************************
 * OPEN_DATA_TIME: Return a FILE * if the file exists, otherwize NULL
 *    Open the file read only! If the file is compressed, uncompress it.
 *
 *
 */

FILE *open_data_time(long time,
                     int num_top_dirs,
                     char **top_dir,
                     char *suffix)
{
  int i;
  char file_name[1024];
  char path_name[2048];
  char format_str[128];
  struct tm *tm;
  FILE *file = NULL;

  tm = gmtime(&time);
  strcpy(format_str,"%Y%m%d/%H%M%S."); /* Build the file name format string */
  strncat(format_str,suffix,127);       /* Add the file suffix */


  strftime(file_name,128,format_str,tm);  /* build the file name */

  /* Check in each directory unitl found */
  for(i=0; i < num_top_dirs && file == NULL; i++) {
    sprintf(path_name,"%s/%s",top_dir[i],file_name);
    file = ta_fopen_uncompress(path_name,"r");
  }
    
  return file;
}
 
 
/*****************************************************************************
 * TIME_COMPARE: function for qsort'ing longs
 */

static int time_compare(const void *v1, const void *v2)

{
  return (*((long *)v1) - *((long *)v2));
}
 

/************************************************************************
 *  FIND_ALL_DATA_TIMES: Search through the files and return a
 *        pointer to a list of sorted data times; A null pointer indicates
 *        no data was found. Sets time_ptr to point to a static list
 *        and returns the number of entries in the list.
 *        
 */

int find_all_data_times(long **time_ptr,
                        int  num_top_dirs,
                        char **top_dir,
                        char *file_suffix,
                        time_t min_time,
                        time_t max_time)
{
  int    i;
  long    year,month,day,hour,minute,second;

  UTIMstruct    utm;
  struct dirent *dp;
  DIR *dirp1,*dirp2;
  char    *ext_ptr,*dot_ptr;
  char    ext_string[64];
  char    dir_path[256];
  char    file_name[256];
  UTIMstruct dir_date;
  UTIMstruct tmp_date;
  time_t min_day;
  time_t max_day;
  time_t dir_time;
  time_t data_time;
     
  static long    *list;
  static int data_list_len = 0;
  static int num_entries = 0;
  /* static int last_num_top_dirs = 0; */
  /* static time_t last_time = 0; */


  if(data_list_len == 0) {
    data_list_len = 256;
    list = (long *) calloc(data_list_len,sizeof(long));
  }

  dir_date.hour = 0;
  dir_date.min = 0;
  dir_date.sec = 0;

  UTIMunix_to_date(min_time,&tmp_date);
  tmp_date.hour = 0;
  tmp_date.min = 0;
  tmp_date.sec = 0;
  min_day = UTIMdate_to_unix(&tmp_date);

  UTIMunix_to_date(max_time,&tmp_date);
  tmp_date.hour = 0;
  tmp_date.min = 0;
  tmp_date.sec = 0;
  max_day = UTIMdate_to_unix(&tmp_date);


  /* last_time = time(0); */
  /* last_num_top_dirs = num_top_dirs; */
  num_entries = 0;
  for(i=0; i < num_top_dirs; i++) {    /* Search throuh all directories for data */
    if((dirp1 = opendir(top_dir[i])) == NULL) continue;
    for(dp = readdir(dirp1); dp != NULL; dp = readdir(dirp1)) {    /* Look for proper subdir's */
      if (dp->d_name[0] == '.') continue; /* skip any dot files */
      if (strlen(dp->d_name) != 8) continue; /* skip any files that dont match the specified format: YYYYMMDD */
      if (sscanf(dp->d_name, "%4ld%2ld%2ld", &year, &month, &day) != 3) continue;
      if (year < 1970 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) continue;

      /* Skip daily directories that aren't within the specified range */
      dir_date.year = year;
      dir_date.month = month;
      dir_date.day = day;
      dir_time = UTIMdate_to_unix(&dir_date);
      if(dir_time < min_day || dir_time > max_day) continue;

      sprintf(dir_path,"%s/%s",top_dir[i],dp->d_name);

      if((dirp2 = opendir(dir_path)) == NULL) continue;
      for(dp = readdir(dirp2); dp != NULL; dp = readdir(dirp2)) {    /* Look for proper data files */
        if (dp->d_name[0] == '.') continue;

        /* check for an exact match of the file suffix */

        if((ext_ptr = strchr(dp->d_name,'.')) == NULL) continue; /* find the first dot */
        ext_ptr++; /* copy what's after it into a temp extension string */
        STRncopy(ext_string,ext_ptr,64);
        /* Replace the next dot wiith a null - Avoids comparing .Z or.gz extension modifiers */
        if((dot_ptr = strchr(ext_string,'.')) != NULL) *dot_ptr = '\0';
        if(strcmp(ext_string,file_suffix)) continue;
        strncpy(file_name,dp->d_name,6);
		 
        file_name[6] = '\0';
        if (sscanf(file_name, "%2ld%2ld%2ld", &hour, &minute, &second) != 3) continue;
        if ( hour > 23 || minute > 59 || second > 59) continue;
        utm.year = year;
        utm.month = month;
        utm.day = day;
        utm.hour = hour;
        utm.min = minute;
        utm.sec = second;
        data_time = UTIMdate_to_unix(&utm);
        if(data_time >= min_time && data_time <= max_time) {
          list[num_entries] = UTIMdate_to_unix(&utm);
          num_entries++;


          if(num_entries >= data_list_len) {
            data_list_len *= 2;        /* increase buffer by two */
            list = (long *)  realloc(list,data_list_len * sizeof(long));
          }
        }
      }
      closedir(dirp2);
    }
    closedir(dirp1);
  }

  qsort((char *) list,num_entries,sizeof(long),time_compare);

  *time_ptr = list;
  return num_entries;
}
 
/************************************************************************
 *  FIND_LATEST_DATA_TIME: Search through the files and return 
 *        The unix time of the latest data. A return value of 0
 *        means there is no data available.
 *        
 */

long find_latest_data_time(int num_top_dirs,
                           char **top_dir,
                           char *suffix)
{
  long    num_entries;
  long *list;
  time_t now;
  static time_t last_latest = 0;
  now = time(0);

  num_entries = find_all_data_times(&list,
                                    num_top_dirs,
                                    top_dir,
                                    suffix,
                                    last_latest,
                                    now+1000000);

  if(num_entries > 0) {
    last_latest = list[num_entries -1];
    return(list[num_entries -1]);
  } else {
    return 0;
  }
}

 
#ifdef WANT_NEW
/************************************************************************
 *  FIND_CURRENT_DATA_TIME: Search for a Cidd Server index file
 *   for a given data set and find the time that is considered "current".
 *   Returns the unix time handle of of the data set or 0 for nothing
 *   found.
 */

long find_current_data_time(int num_top_dirs,
                            char **top_dir,
                            char *suffix)
{
  int    i;
  long   dummy; /* dummy variable for secquence number on indexes */

  cdata_current_index_t    index;
  struct stat buf;
  char name[1024];

  for(i=0; i < num_top_dirs; i++) {    /* Search through each top directory */
    sprintf(name,"%s/%s",top_dir[i],CDATA_CURRENT_FILE_INDEX);

    if(stat(name,&buf) == 0) {  /* If the index exists */
      dummy = 0;
      cdata_read_index_simple(top_dir[i],
                              0,
                              &dummy,
                              0,
                              &index,
                              "cidd_files",
                              "find_current_data_time",
                              0);

      /* Check to see if the file suffixes match */
      if(strncmp(index.file_ext,suffix,CDATA_MAX_INDEX_STR) == 0) { 
        if(index.exact_time) { 
          return index.unix_time;
        } else {     /* must look for the closest time */
          return(find_best_data_time(index.unix_time -3600,
                                     index.unix_time,
                                     index.unix_time +3600,
                                     num_top_dirs,
                                     top_dir,
                                     suffix));
        }
      }
    }
  }
  return 0;  /* No index or no data times within an hour of "current" */
}
#endif

/************************************************************************
 *  FIND_BEST_DATA_TIME: Search through the files and return 
 *        the data time of the data set that has the best match to the
 *         specified time window data times. A return value of 0
 *        means there is no data available in the indicated time window.
 *        
 */

long find_best_data_time(long begin,
                         long mid,
                         long end,
                         int num_top_dirs,
                         char **top_dir,
                         char *suffix)
{
  int i,index = 0;
  long num_entries;
  long dist;    
  long *list;

  num_entries = find_all_data_times(&list,
                                    num_top_dirs, top_dir,
                                    suffix, 
                                    begin, end);
  if(num_entries == 0) return 0;

  dist = MAX_LONG;
  for(i=0; i < num_entries; i++) {
    if(labs(list[i] - mid) < dist) {
      dist = labs(list[i] - mid);
      index = i;
    }
  }

  if(list[index] >= begin && list[index] <= end) {
    return list[index];
  } else {
    return 0;
  }
}

