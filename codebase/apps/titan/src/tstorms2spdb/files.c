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
/****************************************************************************
 * files.c
 *
 * Opens & closes storm and track files
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Sept 1997
 *
 ****************************************************************************/

#include "tstorms2spdb.h"
#include <toolsa/file_io.h>

void open_track_files(storm_file_handle_t *s_handle,
		      track_file_handle_t *t_handle,
		      char *track_file_path)

{

  char storm_file_path[MAX_PATH_LEN];
  path_parts_t track_path_parts;

  /*
   * open track properties files
   */
  
  if (RfOpenTrackFiles (t_handle, "r",
			track_file_path,
			(char *) NULL,
			"open_track_files")) {
    
    fprintf(stderr, "ERROR - %s:open_track_files\n", Glob->prog_name);
    fprintf(stderr, "Opening track file '%s'.\n",
	    track_file_path);
    perror(track_file_path);
    tidy_and_exit(-1);

  }

  /*
   * read in track file header
   */

  if (RfReadTrackHeader(t_handle, "open_track_files")) {
    tidy_and_exit(-1);
  }

  /*
   * compute storm data file path
   */

  uparse_path(track_file_path, &track_path_parts);
  
  sprintf(storm_file_path, "%s%s",
	  track_path_parts.dir,
	  t_handle->header->storm_header_file_name);

  /*
   * open storm file
   */

  if (RfOpenStormFiles (s_handle, "r",
			storm_file_path,
			(char *) NULL,
			"open_track_files")) {
    
    fprintf(stderr, "ERROR - %s:open_track_files\n", Glob->prog_name);

    fprintf(stderr, "Opening storm file '%s'.\n",
	    storm_file_path);
    perror(storm_file_path);
    tidy_and_exit(-1);

  }

  /*
   * read in storm properties file header
   */

  if (RfReadStormHeader(s_handle, "open_track_files")) {
    tidy_and_exit(-1);
  }

  /*
   * read in first scan
   */

  if (RfReadStormScan(s_handle, 0, "open_track_files")) {
    tidy_and_exit(-1);
  }

  /*
   * get read lock on header files
   */

  if (ta_lock_file_procmap(s_handle->header_file_path,
			   s_handle->header_file, "r")) {
    fprintf(stderr, "WARNING - %s:open_track_files\n", Glob->prog_name);
  }

  if (ta_lock_file_procmap(t_handle->header_file_path,
			   t_handle->header_file, "r")) {
    fprintf(stderr, "WARNING - %s:open_track_files\n", Glob->prog_name);
  }

  /*
   * free resources
   */

  ufree_parsed_path(&track_path_parts);

}

void close_track_files(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle)

{

  /*
   * clear lock on header files
   */
  
  if (ta_unlock_file(s_handle->header_file_path,
		     s_handle->header_file)) {
    fprintf(stderr, "WARNING - %s:close_track_files\n", Glob->prog_name);
  }

  if (ta_unlock_file(t_handle->header_file_path,
		     t_handle->header_file)) {
    fprintf(stderr, "WARNING - %s:close_track_files\n", Glob->prog_name);
  }
  
  /*
   * close files
   */
  
  RfCloseStormFiles(s_handle, "close_track_files");
  RfCloseTrackFiles(t_handle, "close_track_files");
  
}

