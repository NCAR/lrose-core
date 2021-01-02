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
/*******************************************************************************
 * open_files.c
 *
 * Opens the files, reads in the headers
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 *******************************************************************************/

#include "verify_day.h"

void open_files(storm_file_handle_t *s_handle,
		track_file_handle_t *t_handle,
		char **track_file_paths,
		si32 file_num)

{

  static int first_call = TRUE;
  static storm_file_params_t sparams;
  static track_file_params_t tparams;
  char storm_file_path[MAX_PATH_LEN];
  path_parts_t track_path_parts;

  /*
   * open track data file
   */

  if (RfOpenTrackFiles(t_handle,
		       "r",
		       track_file_paths[file_num],
		       (char *) NULL,
		       "open_files")) {
    
    fprintf(stderr, "ERROR - %s:open_files\n", Glob->prog_name);
    fprintf(stderr, "Cannot open track files.\n");
    perror(track_file_paths[file_num]);
    tidy_and_exit(-1);
    
  }
  
  /*
   * read in track file header
   */

  if (RfReadTrackHeader(t_handle, "open_files") != R_SUCCESS)
    tidy_and_exit(-1);

  /*
   * compute storm data file path
   */

  uparse_path(track_file_paths[file_num], &track_path_parts);
  
  sprintf(storm_file_path, "%s%s",
	  track_path_parts.dir,
	  t_handle->header->storm_header_file_name);
  
  /*
   * open storm file
   */

  if (RfOpenStormFiles(s_handle,
		       "r",
		       storm_file_path,
		       (char *) NULL,
		       "open_files")) {
    
    fprintf(stderr, "ERROR - %s:open_files\n", Glob->prog_name);
    fprintf(stderr, "Cannot open storm files.\n");
    perror(storm_file_path);
    tidy_and_exit(-1);
    
  }
  
  /*
   * read in storm properties file header
   */

  if (RfReadStormHeader(s_handle, "open_files") != R_SUCCESS)
    tidy_and_exit(-1);
  
  /*
   * free resources
   */

  ufree_parsed_path(&track_path_parts);

  /*
   * for first call copy over storm and track params.
   * for subsequent calls, check that these have not changed.
   */

  if (first_call) {

    memcpy ((void *) &sparams,
            (void *) &s_handle->header->params,
            (size_t) sizeof(storm_file_params_t));

    memcpy ((void *) &tparams,
            (void *) &t_handle->header->params,
            (size_t) sizeof(track_file_params_t));

    first_call = FALSE;

  } else {

    if(memcmp((void *) &s_handle->header->params,
	      (void *) &sparams,
	      (size_t) sizeof(storm_file_params_t)) != 0) {

      fprintf(stderr, "ERROR - %s:open_files\n", Glob->prog_name);
      fprintf(stderr, "Storm file params have changed.\n");
      fprintf(stderr, "Storm file '%s'\n", s_handle->header_file_path);
      
      tidy_and_exit(1);

    }

    if(memcmp((void *) &t_handle->header->params,
	      (void *) &tparams,
	      (size_t) sizeof(track_file_params_t)) != 0) {

      fprintf(stderr, "ERROR - %s:open_files\n", Glob->prog_name);
      fprintf(stderr, "Track file params have changed.\n");
      fprintf(stderr, "Track file '%s'\n", t_handle->header_file_path);
      
      tidy_and_exit(1);

    }

  } /* if (first_call) */

}
