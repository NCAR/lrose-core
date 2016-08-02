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
 * restart.c
 *
 * Restart this program
 *
 * If we cannot prepare a new file, return -1. Otherwise,
 * tidy_and_exit.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1991
 *
 ****************************************************************************/

#include "storm_ident.h"

int restart(storm_file_handle_t *s_handle)

{

  time_t restart_time, new_start_time;

  /*
   * determine new_start_time - the new file overlaps the old
   * one by restart_overlap_period
   */
  
  restart_time = get_restart_time();
  new_start_time = (restart_time -
		    Glob->params.restart_overlap_period);

  /*
   * check that we have data after the new_restart_time - if not,
   * do not prepare a new storm file
   */

  if (new_start_time < s_handle->header->end_time) {
    if (prepare_restart_storm_file(s_handle,
				   restart_time, new_start_time) == -1) {
      return (-1);
    }
  }
  
  RfCloseStormFiles(s_handle, "main");
  
  if (Glob->params.remove_old_files_on_restart) {
    
    if (unlink(s_handle->header_file_path)) {
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stderr, "WARNING - %s:main\n", Glob->prog_name);
	fprintf(stderr, "Cannot remove old storm header file\n");
	perror(s_handle->header_file_path);
      }
    }
    
    if (unlink(s_handle->data_file_path)) {
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stderr, "WARNING - %s:main\n", Glob->prog_name);
	fprintf(stderr, "Cannot remove old storm data file\n");
	perror(s_handle->data_file_path);
      }
    }
    
  } /* if (Glob->params.remove_old_storm_files) */
  
  tidy_and_exit(EXIT_AND_RESTART);

  return (0);

}

