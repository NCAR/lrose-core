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
 * prepare_restart_storm_file.c
 *
 * Prepares the storm file for a next date - copies over a set number
 * of scans from the previous day's data to provide some history
 *
 * Returns 0 if success, -1 if cannot prepare new file. The latter
 * condition applies when the old file and new file have the same
 * name, in which case we don't want to start a new file.
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * January 1993
 *
 * Mike Dixon
 *
 *********************************************************************/

#include <time.h>
#include <math.h>
#include "storm_ident.h"

int prepare_restart_storm_file(storm_file_handle_t *old_s_handle,
			       time_t restart_time,
			       time_t new_start_time)
     
{

  char new_file_path[MAX_PATH_LEN];

  si32 iscan, istorm;
  si32 n_scans;
  si32 nstorms;
  si32 new_scan_num;

  path_parts_t old_path_parts;
  date_time_t file_path_time;
  si32 scan_time;
  storm_file_global_props_t *gprops;
  storm_file_handle_t new_s_handle;

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr,
	    "%s:prepare_restart_storm_file - creating file for next day\n",
	    Glob->prog_name);
    fprintf(stderr, "Restart time is %s\n", utimstr(restart_time));
  }

  /*
   * read in header
   */

  if (RfReadStormHeader(old_s_handle, "prepare_restart_storm_file")) {
    fprintf(stderr, "ERROR - %s:prepare_restart_storm_file\n",
	    Glob->prog_name);
    fprintf(stderr, "Reading old file header.\n");
    tidy_and_exit(-1);
  }

  /*
   * determine the date for the new file form the restart time
   */

  file_path_time.unix_time = restart_time;
  uconvert_from_utime(&file_path_time);
  
  /*
   * set up file path for new file
   */

  uparse_path(old_s_handle->header_file_path, &old_path_parts);

  sprintf(new_file_path, "%s%.2d%.2d%.2d%s",
	  old_path_parts.dir,
	  file_path_time.year, file_path_time.month, file_path_time.day,
	  old_path_parts.ext);

  ufree_parsed_path(&old_path_parts);

  if (Glob->params.debug >= DEBUG_VERBOSE)
    fprintf(stderr, "New file : %s\n", new_file_path);

  /*
   * if the old and new files have the same name, don't
   * prepare new file - return error condition
   */
  
  if (strcmp(new_file_path, old_s_handle->header_file_path) == 0) {
    return (-1);
  }

  /*
   * initialize new storm file handle
   */
  
  RfInitStormFileHandle(&new_s_handle, Glob->prog_name);

  /*
   * open new storm properties file
   */
  
  if (RfOpenStormFiles (&new_s_handle, "w",
			new_file_path,
			STORM_DATA_FILE_EXT,
			"prepare_restart_storm_file")) {
    
    fprintf(stderr, "ERROR - %s:prepare_restart_storm_file\n",
	    Glob->prog_name);
    fprintf(stderr, "Creating new storm file %s.\n", new_file_path);
    tidy_and_exit(-1);
  }

  /*
   * allocate header
   */

  if (RfAllocStormHeader(&new_s_handle,
			 "prepare_restart_storm_file"))
    tidy_and_exit(-1);

  /*
   * copy old file header to new file
   */

  memcpy ((void *) new_s_handle.header,
          (void *) old_s_handle->header,
          (size_t) sizeof(storm_file_header_t));

  /*
   * set the new file at the correct point for data writes - after 
   * the label
   */

  if (RfSeekStartStormData(&new_s_handle,
			   "prepare_restart_storm_file"))
    tidy_and_exit(-1);

  /*
   * loop through the scans to be copied
   */

  n_scans = old_s_handle->header->n_scans;
  
  new_scan_num = 0;

  for (iscan = 0; iscan < n_scans; iscan++) {

    /*
     * read in scan
     */

    if (RfReadStormScan(old_s_handle, iscan, "prepare_restart_storm_file")) {
      tidy_and_exit(-1);
    }

    scan_time = old_s_handle->scan->time;

    /*
     * check time - if before the new start time,
     * skip ahead
     */

    if (scan_time < new_start_time) {
      continue;
    }
      
    /*
     * set data in header as applicable
     */
    
    if (new_scan_num == 0)
      new_s_handle.header->start_time = scan_time;
    new_s_handle.header->end_time = scan_time;
    new_s_handle.header->n_scans = new_scan_num + 1;
    
    if (Glob->params.debug >= DEBUG_VERBOSE)
      fprintf(stderr, "Copying scan %ld, time %s\n", (long) iscan,
	      utimstr(scan_time));
    
    /*
     * allocate scan and gprops array
     */
    
    nstorms = old_s_handle->scan->nstorms;
    
    RfAllocStormScan(&new_s_handle,
		     nstorms,
		     "prepare_restart_storm_file");
    
    /*
     * copy scan struct over to new index
     */
    
    memcpy ((void *) new_s_handle.scan,
	    (void *) old_s_handle->scan,
	    (size_t) sizeof(storm_file_scan_header_t));
    
    new_s_handle.scan->scan_num = new_scan_num;
    
    /*
     * copy over global props
     */

    memcpy ((void *) new_s_handle.gprops,
            (void *) old_s_handle->gprops,
            (size_t) (nstorms * sizeof(storm_file_global_props_t)));

    /*
     * loop through storms
     */

    for (istorm = 0; istorm < nstorms; istorm++) {

      gprops = old_s_handle->gprops + istorm;

      /*
       * read in layer, hist and runs props for storm
       */
      
      if (RfReadStormProps(old_s_handle, istorm,
			   "prepare_restart_storm_file"))
	tidy_and_exit(-1);
      
      /*
       * allocate layer, hist and run arrays
       */
      
      RfAllocStormProps(&new_s_handle,
			gprops->n_layers,
			gprops->n_dbz_intervals,
			gprops->n_runs,
			gprops->n_proj_runs,
			"prepare_restart_storm_file");
      
      /*
       * copy over the layer, hist and runs data
       */

      memcpy ((void *) new_s_handle.layer,
              (void *) old_s_handle->layer,
              (size_t) (gprops->n_layers *
			sizeof(storm_file_layer_props_t)));

      memcpy ((void *) new_s_handle.hist,
              (void *) old_s_handle->hist,
              (size_t) (gprops->n_dbz_intervals *
			sizeof(storm_file_dbz_hist_t)));
      
      memcpy ((void *) new_s_handle.runs,
              (void *) old_s_handle->runs,
              (size_t) (gprops->n_runs *
			sizeof(storm_file_run_t)));
      
      memcpy ((void *) new_s_handle.proj_runs,
              (void *) old_s_handle->proj_runs,
              (size_t) (gprops->n_proj_runs *
			sizeof(storm_file_run_t)));
      
      /*
       * write layer, hist and runs data to file
       */
      
      if (RfWriteStormProps(&new_s_handle, istorm,
			    "prepare_restart_storm_file"))
	tidy_and_exit(-1);

    } /* istorm */
    
    /*
     * write scan header and global props
     */

    if (RfWriteStormScan(&new_s_handle, new_scan_num,
			 "prepare_restart_storm_file"))
      tidy_and_exit(-1);

    new_scan_num++;

  } /* iscan */

  /*
   * write new header
   */

  if (RfWriteStormHeader(&new_s_handle,
			 "prepare_nextstorm_file"))
    tidy_and_exit(-1);

  /*
   * close files
   */

  RfCloseStormFiles (&new_s_handle,
		     "prepare_restart_storm_file");
  
  /*
   * free resources
   */

  RfFreeStormProps(&new_s_handle, "prepare_restart_storm_file");
  RfFreeStormScan(&new_s_handle, "prepare_restart_storm_file");
  RfFreeStormHeader(&new_s_handle, "prepare_restart_storm_file");

  return (0);

}

