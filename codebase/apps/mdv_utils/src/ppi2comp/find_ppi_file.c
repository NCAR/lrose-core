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
 * find_ppi_file()
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "ppi2comp.h"
#include <dirent.h>

static int find_best_file(char *dir_path,
			  char *file_path,
			  si32 start_time,
			  si32 end_time,
			  date_time_t *dir_dt,
			  si32 search_time,
			  si32 *time_error);

void find_ppi_file(ppi_t *trigger,
		   ppi_t *ppi,
		   ppi2comp_input *input)
     
{

  static int ldata_init = FALSE;
  static LDATA_handle_t ldata;

  char dir_path[MAX_PATH_LEN];

  int file_found;

  si32 trigger_time, ppi_time;
  si32 time_error;
  si32 iday, ndays;
  
  date_time_t start_dt, end_dt, *this_dt;
  
  if (!ldata_init) {
    LDATA_init_handle(&ldata, Glob->prog_name, Glob->params.debug);
    ldata_init = TRUE;
  }

  if (Glob->params.mode == REALTIME) {
    
    /*
     * realtime mode
     *
     * get file details from latest info file
     */

    if (LDATA_info_read(&ldata, input->dir, -1)) {

      /*
       * no index - file invalid
       */
      
      ppi->valid = FALSE;
      return;

    }
  
    trigger_time = Rfrtime2utime(&trigger->v_handle.vol_params->mid_time);
    ppi_time = ldata.info.latest_time;
    
    if ((trigger_time - ppi_time) > input->max_valid_age) {
      ppi->valid = FALSE;
      return;
    }

    /*
     * read in ppi volume
     */

    sprintf(ppi->file_path, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	    input->dir,
	    PATH_DELIM,
	    ldata.ltime.year, ldata.ltime.month, ldata.ltime.day,
	    PATH_DELIM,
	    ldata.ltime.hour, ldata.ltime.min, ldata.ltime.sec,
	    ldata.info.file_ext);

    ppi->v_handle.vol_file_path = ppi->file_path;
    if (RfReadVolume(&ppi->v_handle, "find_ppi_file") != R_SUCCESS) {
      ppi->valid = FALSE;
      return;
    }
    ppi->valid = TRUE;

    return;      

  } else {

    /*
     * archive mode
     *
     * search for file in directory structure
     */
    
    trigger_time = Rfrtime2utime(&trigger->v_handle.vol_params->mid_time);
    end_dt.unix_time = trigger_time + input->max_valid_age;
    uconvert_from_utime(&end_dt);
    start_dt.unix_time = trigger_time - input->max_valid_age;
    uconvert_from_utime(&start_dt);
    
    time_error = LARGE_LONG;
    file_found = FALSE;

    if (start_dt.day != end_dt.day)
      ndays = 2;
    else
      ndays = 1;
    
    for (iday = 0; iday < ndays; iday++) {
      
      if (iday == 0)
	this_dt = &start_dt;
      else
	this_dt = &end_dt;
      
      /*
       * search through the data directory for a matching file in
       * the correct subdirectory
       */
      
      sprintf(dir_path, "%s%s%.4ld%.2ld%.2ld",
	      input->dir, PATH_DELIM,
	      (long) this_dt->year,
	      (long) this_dt->month,
	      (long) this_dt->day);
      
      /*
       * load file name of the closest scan to the requested time
       */
      
      if(find_best_file(dir_path, ppi->file_path,
			start_dt.unix_time, end_dt.unix_time, this_dt,
			trigger_time, &time_error) == 0) {
	file_found = TRUE;
      }
      
    } /* iday */
    
    if (file_found) {
      ppi->v_handle.vol_file_path = ppi->file_path;
      if (RfReadVolume(&ppi->v_handle, "find_ppi_file") != R_SUCCESS) {
	ppi->valid = FALSE;
	return;
      } else {
	ppi->valid = TRUE;
	return;
      }
    } else {
      ppi->valid = FALSE;
      return;
    }
    
  } /* if (Glob->params.mode == REALTIME) */

}

/*****************************************************************
 * find_best_file()
 *
 * Find the most appropriate file given the
 * time frame requested
 */

static int find_best_file (char *dir_path,
			   char *file_path,
			   si32 start_time,
			   si32 end_time,
			   date_time_t *dir_dt,
			   si32 search_time,
			   si32 *time_error)

{
  
  char file_ext[16];
  si32 f_count;
  si32 time_diff;
  long hour, min, sec;
  
  date_time_t file_dt;
  
  struct dirent *dp;
  DIR *dirp;
  
  /*
   * open directory - return -1 if unable to open
   */
  
  if ((dirp = opendir(dir_path)) == NULL)
    return (-1);
  
  /*
   * Loop thru directory looking for the data file names
   */
  
  f_count = 0;
  
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    /*
     * exclude dir entries and files beginning with '.'
     */

    if (dp->d_name[0] == '.')
      continue;
      
    /*
     * check that the dir name is in the correct format
     */

    if (sscanf(dp->d_name, "%2ld%2ld%2ld.%s",
	       &hour, &min, &sec, file_ext) != 4)
      continue;
      
    if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	sec < 0 || sec > 59)
      continue;

    /*
     * file name is in correct format. Therefore, accept it
     */
    
    file_dt.year = dir_dt->year;
    file_dt.month = dir_dt->month;
    file_dt.day = dir_dt->day;
    file_dt.hour = hour;
    file_dt.min = min;
    file_dt.sec = sec;
	
    uconvert_to_utime(&file_dt);
	
    if (file_dt.unix_time >= start_time &&
	file_dt.unix_time <= end_time) {
	  
      time_diff = abs ((int) (file_dt.unix_time - search_time));
	  
      if (time_diff < *time_error) {
	    
	*time_error = time_diff;
	sprintf(file_path, "%s%s%s",
		dir_path, PATH_DELIM,
		dp->d_name);
	f_count++;
	    
      } /*if (time_diff < *time_error) */
	  
    } /* if (file_dt.unix_time >= com->time_min ... */

  } /* dp */
  
  closedir(dirp);
  
  if(f_count == 0)
    return (-1);
  else
    return (0);
  
}

