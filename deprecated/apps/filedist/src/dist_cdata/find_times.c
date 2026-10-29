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
 * find_times.c
 * 
 * Find the file times.
 *
 * Search through the files and return a
 * pointer to a list of sorted data times; A null pointer indicates
 * no data was found. Sets time_ptr to point to a static list
 * and returns the number of entries in the list.
 */

#include "dist_cdata.h"
 
static int time_compare(const void *v1, const void *v2);

int find_times(time_t **time_ptr,
	       char *top_dir,
	       char *file_suffix,
	       time_t min_time,
	       time_t max_time)

{

  int year,month,day,hour,minute,second;

  UTIMstruct utm;
  struct dirent *dp;
  DIR *dirp1,*dirp2;
  char *ext_ptr, *dot_ptr;
  char ext_string[64];
  char dir_path[256];
  char file_name[256];
  UTIMstruct dir_date;
  UTIMstruct tmp_date;
  time_t min_day;
  time_t max_day;
  time_t dir_time;
  time_t data_time;
  
  static time_t *list;
  static int data_list_len = 0;
  static int num_entries = 0;
  static time_t last_time = 0;
  
  if(data_list_len == 0) {
    data_list_len = 256;
    list = (time_t *) ucalloc(data_list_len,sizeof(time_t));
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

  last_time = time(0);
  num_entries = 0;

  if((dirp1 = opendir(top_dir)) == NULL) {
    return (0);
  }
  
  for(dp = readdir(dirp1); dp != NULL; dp = readdir(dirp1)) {    /* Look for proper subdir's */

    if (dp->d_name[0] == '.') continue; /* skip any dot files */
    if (strlen(dp->d_name) != 8) continue; /* skip any files that dont match
					    * the specified format: YYYYMMDD */
    if (sscanf(dp->d_name, "%4d%2d%2d", &year, &month, &day) != 3) continue;
    if (year < 1970 || year > 2100 || month < 1
	|| month > 12 || day < 1 || day > 31) continue;

    /* Skip daily directories that aren't within the specified range */
    dir_date.year = year;
    dir_date.month = month;
    dir_date.day = day;
    dir_time = UTIMdate_to_unix(&dir_date);
    if(dir_time < min_day || dir_time > max_day) continue;
    
    sprintf(dir_path,"%s/%s", top_dir, dp->d_name);

    if (gd.debug) {
      fprintf(stderr, "Searching dir %s\n", dir_path);
    }

    if((dirp2 = opendir(dir_path)) == NULL) continue;
    for(dp = readdir(dirp2); dp != NULL; dp = readdir(dirp2)) {    /* Look for proper data files */
      if (dp->d_name[0] == '.') continue;

      /* check for an exact match of the file suffix */
      
      if((ext_ptr = strchr(dp->d_name,'.')) == NULL) continue; /* find the first dot */
      ext_ptr++; /* copy what's after it into a temp extension string */
      strncpy(ext_string,ext_ptr,64);
      /* Replace the next dot wiith a null - Avoids comparing .Z or.gz extension modifiers */
      if((dot_ptr = strchr(ext_string,'.')) != NULL) *dot_ptr = '\0';
      if(strcmp(ext_string,file_suffix)) continue;
      strncpy(file_name,dp->d_name,6);
      
      file_name[6] = '\0';
      if (sscanf(file_name, "%2d%2d%2d", &hour, &minute, &second) != 3) continue;
      if ( hour > 23 || minute > 59 || second > 59) continue;

      utm.year = year;
      utm.month = month;
      utm.day = day;
      utm.hour = hour;
      utm.min = minute;
      utm.sec = second;
      data_time = UTIMdate_to_unix(&utm);
      if(data_time >= min_time && data_time <= max_time) {

	if (gd.debug) {
	  fprintf(stderr, "Candidate file File %s\n", dp->d_name);
	}
      
	list[num_entries] = UTIMdate_to_unix(&utm);
	num_entries++;

	
	if(num_entries >= data_list_len) {
	  data_list_len *= 2;        /* increase buffer by two */
	  list = (time_t *)  realloc(list,data_list_len * sizeof(time_t));
	}
      }
    }
    closedir(dirp2);
  }
  closedir(dirp1);

  qsort((char *) list,num_entries,sizeof(time_t),time_compare);

  *time_ptr = list;
  return num_entries;

}
 
/*****************************************************************************
 * TIME_COMPARE: function for qsort'ing time_ts
 */

static int time_compare(const void *v1, const void *v2)

 {
     return (*((time_t *)v1) - *((time_t *)v2));
 }
 

