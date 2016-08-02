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
/***************************************************************************
 * prepare_storm_file.c
 *
 * Prepares the storm data file. If the file does not exist, it is created
 * and the header is prepared and written. If the file exists and the params
 * in the header do not match the current ones, the old file is overwritten.
 * If the file exists and the parameters do match, the old file is opened for
 * read/write. The file is read and the scans in it are compared with those
 * in the time_list. The scan number and file markers are positioned
 * immediately after the last scan which matches the file list.
 * Processing will continue from this point.
 *
 * The next file number and next scan number to be processed are passed
 * back via the pointers.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1991
 *
 ****************************************************************************/

#include "storm_ident.h"

#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>

static void get_last_match(storm_file_handle_t *s_handle,
			   si32 first_scan_match,
			   si32 *last_scan_match,
			   si32 *last_time_match,
			   date_time_t *time_list,
			   si32 ntimes_in_list);

static void truncate_file(FILE **fd,
			  char *path,
			  si32 length);

/***************************************************************************
 * open_and_check_storm_file()
 *
 * returns 0 if success, -1  if problem
 *
 ****************************************************************************/

int open_and_check_storm_file(storm_file_handle_t *s_handle,
			      char *header_file_path,
			      storm_file_header_t *file_header)
     
{
  
  struct stat file_stat;

  /*
   * check if the file exists - if it does not exist, go to
   * end of loop
   */
  
  if (stat(header_file_path, &file_stat)) {

    /*
     * file does not exist - no scans match
     */
    
    return (-1);
    
  }
  
  /*
   * try to open the file read/write
   */
  
  if (RfOpenStormFiles (s_handle, "r",
			header_file_path,
			STORM_DATA_FILE_EXT,
			"open_and_check_storm_file")) {
    
    /*
     * cannot successfully open files - no scans match
     */
    
    fprintf(stderr,
	    "WARNING ONLY - %s:open_and_check_storm_file\n",
	    Glob->prog_name);
    fprintf(stderr, "IGNORE PREVIOUS ERROR from RfOpenStormFiles\n");
    fprintf(stderr, "New file will be created\n");

    return (-1);
    
  }
  
  /*
   * read in storm properties file header
   */
  
  if (RfReadStormHeader (s_handle, "open_and_check_storm_file")) {
    
    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf(stderr,
	      "WARNING ONLY - %s:open_and_check_storm_file\n",
	      Glob->prog_name);
      fprintf(stderr, "IGNORE PREVIOUS ERROR from RfReadStormHeader\n");
      fprintf(stderr, "File corrupted - preparing new file.\n");
    }
      
    return (-1);
    
  } /* if (RfReadStormHeader (s_handle .... */
  
  /*
   * compare the old file params with the current ones - if they do
   * not match, no scans match
   */
    
  if (memcmp((void *) &s_handle->header->params,
	     (void *) &file_header->params,
	     (size_t) sizeof(storm_file_params_t)) != 0) {
    
    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf(stderr,
	      "WARNING - %s:open_and_check_storm_file\n",
	      Glob->prog_name);
      fprintf(stderr, "Parameters do not match\n");
    }
    
    return (-1);
    
  } /* if (memcmp ... */

  return (0);

}
  
/***************************************************************************
 * get_prev_scan()
 *
 * Returns the prev used scan number.
 * This is the scan up to and including which the storm
 * file matches the file list.
 * If no scans applicable, returns -1
 *
 * Also adjusts the time list to start at the next scan to be processed,
 * and adjusts ntimes_in_list to the number of unused entries.
 *
 ****************************************************************************/

si32 get_prev_scan(storm_file_handle_t *s_handle,
		   date_time_t *time_list,
		   si32 *ntimes_in_list)
     
{
  
  int initial_match_found;
  si32 i, iscan;
  si32 initial_scan_match;
  si32 last_scan_match;
  si32 last_time_match;
  si32 n_unused;

  /*
   * Search through the storms file for last time
   * which matches time list
   */
  
  initial_match_found = FALSE;

  for (iscan = 0; iscan < s_handle->header->n_scans; iscan++) {
    
    /*
     * read in scan
     */
    
    if (RfReadStormScan(s_handle, iscan,
			"get_prev_scan")) {
      
      /*
       * error in old file, no scans match
       */
      
      return (-1L);
      
    } /* if (RfReadStormScan(s_handle, iscan, ... */

    if (time_list[0].unix_time == s_handle->scan->time) {

      initial_scan_match = iscan;
      initial_match_found = TRUE;
      break;

    }

  } /* iscan */

  if (initial_match_found) {
      
    /*
     * Found a scan which matches the first file in the list.
     * Now match up the file list with the scan list to determine
     * at which scan the two lists differ.
     */
      
    get_last_match(s_handle,
		   initial_scan_match,
		   &last_scan_match,
		   &last_time_match,
		   time_list,
		   *ntimes_in_list);
    
    /*
     * move the unused time_list entries up to the start of
     * the list, and amend the number of entries in the list
     */
    
    n_unused = *ntimes_in_list - (last_time_match + 1);

    for (i = 0; i < n_unused; i++) {
      time_list[i] = time_list[i + last_time_match + 1];
    }

    *ntimes_in_list = n_unused;
    
    /*
     * print out file list
     */
    
    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf(stderr, "\nActive time list:\n");
      for (i = 0; i < *ntimes_in_list; i++) {
	fprintf(stderr, "Time %d: %s\n", i,
		utimestr((time_list + i)));
      }
    }
    
    return (last_scan_match);

  } else {
      
    /*
     * fell through loop
     *
     * the first time in the list does not match any of the
     * scans in the storms file, so no scans match
     */
    
    return (-1L);
    
  } /* if (initial_match_found) */
  
}

/****************************************************************************
 * get_last_match()
 *
 * Move through the storm file, matching up successive scans to the
 * available times in the list, returning the scan number of the
 * last match.
 */

static void get_last_match(storm_file_handle_t *s_handle,
			   si32 initial_scan_match,
			   si32 *last_scan_match_p,
			   si32 *last_time_match_p,
			   date_time_t *time_list,
			   si32 ntimes_in_list)
{
  
  si32 itime;
  si32 iscan;
  
  iscan = initial_scan_match;
  
  for (itime = 0; itime < ntimes_in_list; itime++, iscan++) {
    
    /*
     * read in scan
     */
    
    if (RfReadStormScan(s_handle, iscan,
			"get_last_match")) {
      
      if (Glob->params.debug >= DEBUG_VERBOSE) {
	fprintf(stderr,
		"WARNING ONLY - %s:get_last_match\n",
		Glob->prog_name);
	fprintf(stderr, "IGNORE PREVIOUS ERROR from RfReadStormScan\n");
	fprintf(stderr, "Time corrupted - preparing new file.\n");
      }
      
      *last_time_match_p = itime - 1;
      *last_scan_match_p = iscan - 1;
      return;
      
    }
    
    /*
     * compare the time time with the scan time, to see if they
     * match.
     */
    
    if (time_list[itime].unix_time != s_handle->scan->time) {
      
      /*
       * Times do not match. Therefore, the scan in the storms
       * file is not in sequence.
       */
      
      *last_time_match_p = itime - 1;
      *last_scan_match_p = iscan - 1;
      return;
      
    } /* if (memcmp((void *) &time_list[itime].time ... */
    
    if (iscan == s_handle->header->n_scans - 1) {
      
      /*
       * we are at the end of the storm file
       */
      
      *last_time_match_p = itime;
      *last_scan_match_p = iscan;
      return;
      
    } /* if (iscan == s_handle->header->n_scans) */
    
  } /* itime */
  
  /*
   * all scans match
   */
  
  *last_time_match_p = itime - 1;
  *last_scan_match_p = iscan - 1;
  return;
  
}

/***************************************************************************
 * prepare_new_file
 *
 */

void prepare_new_file(storm_file_handle_t *s_handle,
		      char *header_file_path,
		      storm_file_header_t *file_header)
     
{
  
  /*
   * open the file write/read
   */
  
  if (RfOpenStormFiles (s_handle, "w+",
			header_file_path,
			STORM_DATA_FILE_EXT,
			"prepare_new_file")) {
    
    fprintf(stderr, "ERROR - %s:prepare_new_file\n",
	    Glob->prog_name);
    fprintf(stderr,
	    "Cannot open storm file mode 'w+'.\n");
    perror(header_file_path);
    tidy_and_exit(-1);
  }
  
  if (RfAllocStormHeader(s_handle,
			 "prepare_new_file"))
    tidy_and_exit(-1);
  
  /*
   * copy file header to s_handle
   */
  
  memcpy ((void *) s_handle->header,
          (void *) file_header,
          (size_t) sizeof(storm_file_header_t));
  
  /*
   * write storm file header
   */
  
  if (RfWriteStormHeader(s_handle,
			 "prepare_new_file"))
    tidy_and_exit(-1);
  
  /*
   * set the data file at the correct point for data writes - after the
   * label
   */
  
  if (RfSeekStartStormData(s_handle,
			   "prepare_new_file"))
    tidy_and_exit(-1);
  
  /*
   * flush the buffer
   */
  
  if (RfFlushStormFiles(s_handle,
			"prepare_new_file")) {
    
    fprintf(stderr,
	    "ERROR - %s:prepare_new_file\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot flush storm props files.\n");
    perror(header_file_path);
    tidy_and_exit(-1);
  }
  
  return;
  
}


/***************************************************************************
 * prepare_old_file
 *
 */

void prepare_old_file(storm_file_handle_t *s_handle,
		      char *header_file_path,
		      si32 current_scan_num)
     
{
  
  int trunc_flag;
  si32 init_data_len;
  si32 init_header_len;
  
  /*
   * open the file read/write
   */
  
  if (RfOpenStormFiles (s_handle, "r+",
			header_file_path,
			STORM_DATA_FILE_EXT,
			"prepare_new_file")) {
    
    fprintf(stderr, "ERROR - %s:prepare_old_file\n",
	    Glob->prog_name);
    fprintf(stderr,
	    "Cannot open storm file mode 'r+'.\n");
    perror(header_file_path);
    tidy_and_exit(-1);
  }
  
  /*
   * read in header
   */
  
  if (RfReadStormHeader(s_handle,
			"prepare_old_file")) {
    tidy_and_exit(-1);
  }
  
  /*
   * read in last valid scan
   */
  
  if (RfReadStormScan(s_handle, current_scan_num,
		      "prepare_old_file")) {
    fprintf(stderr, "Reading in last valid scan.\n");
    tidy_and_exit(-1);
  }
  
  /*
   * check whether file will need truncation
   */
  
  if (current_scan_num < s_handle->header->n_scans - 1) {
    
    init_data_len = s_handle->scan->last_offset + 1;
    init_header_len = (R_FILE_LABEL_LEN + sizeof(storm_file_header_t) +
		       (current_scan_num + 1) * sizeof(si32));
    trunc_flag = TRUE;
    
  } else {
    
    trunc_flag =  FALSE;
    
  }
  
  /*
   * copy scan time to header as end time
   */
  
  s_handle->header->end_time = s_handle->scan->time;
  
  /*
   * set other parameters
   */
  
  s_handle->header->n_scans = current_scan_num + 1;
  
  /*
   * write storm file header
   */
  
  if (RfWriteStormHeader(s_handle,
			 "prepare_old_file")) {
    tidy_and_exit(-1);
  }
  
  /*
   * truncate if necessary, and position the file
   */
  
  if (trunc_flag) {
    
    truncate_file(&s_handle->header_file,
		  s_handle->header_file_path,
		  init_header_len);
    
    truncate_file(&s_handle->data_file,
		  s_handle->data_file_path,
		  init_data_len);
    
  }
  
  /*
   * position at end of file
   */
  
  if (RfSeekEndStormData(s_handle,
			 "prepare_new_file"))
    tidy_and_exit(-1);
  
  /*
   * flush the buffer
   */
  
  if (RfFlushStormFiles(s_handle,
			"prepare_old_file")) {
    fprintf(stderr,
	    "ERROR - %s:prepare_old_file\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot flush storm props files.\n");
    perror(header_file_path);
    tidy_and_exit(-1);
  }
  
  return;
  
}

/***************************************************************************
 * truncate_storm_file
 *
 */

static void truncate_file(FILE **fd,
			  char *path,
			  si32 length)
     
{
  
  int low_d;
  
  /*
   * close the buffered file
   */
  
  fclose(*fd);
  
  /*
   * open for low-level io
   */
  
  if ((low_d = open(path, O_WRONLY)) < 0) {
    
    fprintf(stderr, "ERROR - %s:truncate_file\n",
	    Glob->prog_name);
    fprintf(stderr,
	    "Cannot open storm data file - low level - for truncation.\n");
    perror(path);
    tidy_and_exit(-1);
    
  }
  
  /*
   * truncate the file
   */
  
  if (ftruncate(low_d, length) != 0) {
    fprintf(stderr, "ERROR - %s:truncate_file\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot truncate storm props file.\n");
    perror(path);
    tidy_and_exit(-1);
  }
  
  /*
   * close low-level io
   */
  
  close(low_d);
  
  /*
   * re-open the file for buffered i/o
   */
  
  if ((*fd = fopen(path, "r+")) == NULL) {
    fprintf(stderr, "ERROR - %s:truncate_file\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot re-open file.\n");
    perror(path);
    tidy_and_exit(-1);
  }
  
}
