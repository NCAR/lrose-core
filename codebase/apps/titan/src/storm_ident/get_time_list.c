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
/******************************************************************************
 * get_time_list.c
 *
 * gets the list of times to be used for storm identifiction
 *
 * Passed in:   start_time
 *              end_time
 *
 * Passed back: time_list loaded with times
 *
 * Returned:    ntimes_in_list
 *
 * Mike Dixon RAP, NCAR, Boulder, Colorado, February 1991
 *
 *****************************************************************************/

#include "storm_ident.h"
#include <dirent.h>

#define DIR_NAME_LEN 8
#define DELTA_ALLOC 50

/*-----------
 * prototypes
 */

static int flist_compare(const void *v1, const void *v2);
static date_time_t *alloc_time_list(si32 ntimes);

/*-------------
 * main routine
 */

si32 get_time_list(si32 u_start_time,
		   si32 u_end_time,
		   date_time_t **time_list_p)

{

  char dirname[MAX_PATH_LEN];
  char format_str[20];

  si32 ijul, i;
  si32 start_julday, end_julday;
  si32 ntimes_in_list;

  date_time_t file_time;
  date_time_t start_time, end_time;

  DIR *dirp;
  struct dirent	*dp;
  date_time_t *time_list;

  /*
   * initialize
   */

  ntimes_in_list = 0;
  
  /*
   * set up format string
   */

  sprintf(format_str, "%s%s", "%2d%2d%2d.", Glob->params.rdata_file_ext);

  /*
   * compute start and end days
   */

  start_time.unix_time = u_start_time;
  uconvert_from_utime(&start_time);

  end_time.unix_time = u_end_time;
  uconvert_from_utime(&end_time);

  start_julday = ujulian_date(start_time.day,
			      start_time.month,
			      start_time.year);
  
  end_julday = ujulian_date(end_time.day,
			    end_time.month,
			    end_time.year);
  
  /*
   * move through julian dates
   */

  for (ijul = start_julday; ijul <= end_julday; ijul++) {
    
    /*
     * compute the calendar date for this julian date
     */
    
    ucalendar_date(ijul,
		   &file_time.day, &file_time.month, &file_time.year);
    
    /*
     * compute directory name for data with this date
     */
    
    sprintf(dirname, "%s%s%.4d%.2d%.2d",
	    Glob->params.rdata_dir, PATH_DELIM,
	    file_time.year, file_time.month, file_time.day);
    
    /*
     * open directory file for reading - if the directory does
     * not exist, this phase is done
     */
    
    if ((dirp = opendir (dirname)) == NULL) {
      continue;
    }
    
    /*
     * read through the directory
     */

    for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {

      /*
       * exclude dir entries and files beginning with '.'
       */

      if (dp->d_name[0] == '.') {
	continue;
      }
	
      /*
       * check that the file name is in the correct format
       */
      
      if (sscanf(dp->d_name, format_str,
		 &file_time.hour, &file_time.min, &file_time.sec) != 3) {
	
	if (Glob->params.debug >= DEBUG_NORM) {
	  fprintf(stderr, "WARNING - %s:get_time_list\n", Glob->prog_name);
	  fprintf(stderr, "File name '%s' invalid\n", dp->d_name);
	}
	continue;
	
      }
      
      /*
       * check for valid date & time
       */

      if (!uvalid_datetime(&file_time)) {
	
	if (Glob->params.debug >= DEBUG_NORM) {
	  fprintf(stderr, "WARNING - %s:get_time_list\n", Glob->prog_name);
	  fprintf(stderr, "File name '%s' invalid\n", dp->d_name);
	}
	continue;
	
      }
      
      /*
       * file name is in correct format. Therefore, accept it
       */
      
      uconvert_to_utime(&file_time);
      
      if (u_start_time <= file_time.unix_time &&
	  u_end_time >= file_time.unix_time) {
	
	time_list = alloc_time_list(ntimes_in_list + 1);
	
	time_list[ntimes_in_list] = file_time;
	
	ntimes_in_list++;
	
      } /* if (u_start_time <= file_time.unix_time ... */
      
    } /* while */

    /*
     * close the directory file
     */

    closedir(dirp);

  } /* ijul */

  /*
   * sort the time list
   */
  
  qsort((void *) time_list, (size_t) ntimes_in_list, sizeof(date_time_t),
	flist_compare);

  /*
   * print out time list
   */

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "\nFull time list:\n");
    for (i = 0; i < ntimes_in_list; i++) {
      fprintf(stderr, "Time %d: %s\n", i,
	      utimestr((time_list + i)));
    }
  }

  *time_list_p = time_list;
  return (ntimes_in_list);

}

/*------------------
 * memory allocation
 */

static date_time_t *alloc_time_list(si32 ntimes)

{

  static si32 ntimes_alloc = 0;
  static date_time_t *time_list = NULL;

  if (ntimes > ntimes_alloc) {
    if (ntimes_alloc == 0) {
      ntimes_alloc = DELTA_ALLOC;
      time_list = (date_time_t *)
	umalloc((ui32) (ntimes_alloc * sizeof(date_time_t)));
    } else {
      ntimes_alloc += DELTA_ALLOC;
      time_list = (date_time_t *)
	urealloc((char *) time_list,
		 (ui32) (ntimes_alloc * sizeof(date_time_t)));
    }
  }

  return (time_list);

}

/*---------------------------------------
 * define function to be used for sorting
 */

static int flist_compare(const void *v1, const void *v2)

{

    date_time_t *l1 = (date_time_t *) v1;
    date_time_t *l2 = (date_time_t *) v2;

    return (l1->unix_time - l2->unix_time);

}

