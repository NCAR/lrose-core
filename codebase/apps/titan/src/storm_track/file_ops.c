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
 * open_files.c
 *
 * Open the storm data file for reading, and the track data file
 * for the specified access_mode
 *
 * If track file cannot be opened in desired manner, returns -1.
 * Otherwise returns 0.
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * July 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_track.h"
#include <sys/stat.h>

static char Track_header_file_path[MAX_PATH_LEN];
static char Track_data_file_path[MAX_PATH_LEN];
static track_file_handle_t *T_Handle;

int open_files(storm_file_handle_t *s_handle,
	       track_file_handle_t *t_handle,
	       char *access_mode,
	       char *storm_header_path)

{
  
  path_parts_t storm_header_parts;
  int retval = 0;
  T_Handle = t_handle;

  /*
   * close prev storm file, open new storm file
   */
  
  RfCloseStormFiles(s_handle, "open_files");

  if (RfOpenStormFiles(s_handle, "r",
		       storm_header_path,
		       (char *) NULL, "open_files")) {
    
    fprintf(stderr, "ERROR - %s:open_files\n", Glob->prog_name);
    fprintf(stderr, "Cannot open storm files.\n");
    perror(storm_header_path);
    tidy_and_exit(-1);
    
  }
  
  /*
   * read in storm header and first scan
   */
  
  if (RfReadStormHeader(s_handle, "open_files"))
    tidy_and_exit(-1);
  
  if (RfReadStormScan(s_handle, (si32) 0, "open_files"))
    tidy_and_exit(-1);
    
  /*
   * compute the track file path, which is the same as the storm file
   * path except for the extension
   */

  uparse_path(storm_header_path, &storm_header_parts);
  
  sprintf(Track_header_file_path, "%s%s%s%s",
	  storm_header_parts.dir,
	  storm_header_parts.base,
	  ".",
	  TRACK_HEADER_FILE_EXT);
  
  sprintf(Track_data_file_path, "%s%s%s%s",
	  storm_header_parts.dir,
	  storm_header_parts.base,
	  ".",
	  TRACK_DATA_FILE_EXT);
  
  /*
   * open track file for writing and subsequent reading
   */
    
  RfCloseTrackFiles(t_handle, "open_files");

  if (RfOpenTrackFiles(t_handle,
		       access_mode,
		       Track_header_file_path,
		       TRACK_DATA_FILE_EXT,
		       "open_files"))
    retval = -1;
  
  if (RfAllocTrackHeader(t_handle, "open_files"))
    tidy_and_exit(-1);
  
  if (RfAllocSimpleTrackParams(t_handle, "open_files"))
    tidy_and_exit(-1);

  if (RfAllocComplexTrackParams(t_handle, "open_files"))
    tidy_and_exit(-1);

  if (RfAllocTrackEntry(t_handle, "open_files"))
    tidy_and_exit(-1);

  /*
   * set file names in track_file_header
   */

  memset((void *) t_handle->header->header_file_name,
	 (int) 0, (int) R_LABEL_LEN);
  
  memset((void *) t_handle->header->data_file_name,
	 (int) 0, (int) R_LABEL_LEN);

  memset((void *) t_handle->header->storm_header_file_name,
	 (int) 0, (int) R_LABEL_LEN - 20);

  sprintf(t_handle->header->header_file_name,
	  "%s.%s",
	  storm_header_parts.base,
	  TRACK_HEADER_FILE_EXT);
  
  sprintf(t_handle->header->data_file_name,
	  "%s.%s",
	  storm_header_parts.base,
	  TRACK_DATA_FILE_EXT);
  
  ustrncpy(t_handle->header->storm_header_file_name,
	   storm_header_parts.name,
	   R_LABEL_LEN);
  
  /*
   * free up resources
   */
  
  ufree_parsed_path(&storm_header_parts);

  /*
   * on read, read in track file header
   */

  if (*access_mode == 'r' && retval == 0) {
    if (RfReadTrackHeader(t_handle, "open_files")) {
      retval = -1;
    }
    if (RfReadSimplesPerComplex(t_handle, "open_files")) {
      retval = -1;
    }
  }

  return (retval);

}

/*****************************************************************
 * remove_track_files()
 */

void remove_track_files(void)

{

  fclose(T_Handle->header_file);
  fclose(T_Handle->data_file);
  
  fprintf(stderr, "WARNING - %s:remove_track_files\n",
	  Glob->prog_name);
  fprintf(stderr, "Removing track header file '%s'\n",
	  Track_header_file_path);
  if (unlink(Track_header_file_path)) {
    if (Glob->params.debug >= DEBUG_NORM) {    
      fprintf(stderr, "WARNING - %s:remove_track_files\n",
	      Glob->prog_name);
      fprintf(stderr, "Cannot remove header file\n");
      perror(Track_header_file_path);
    }
  }

  fprintf(stderr, "WARNING - %s:remove_track_files\n",
	  Glob->prog_name);
  fprintf(stderr, "Removing track data file '%s'\n",
	  Track_data_file_path);
  if (unlink(Track_data_file_path)) {
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "WARNING - %s:remove_track_files\n",
	      Glob->prog_name);
      fprintf(stderr, "Cannot remove data file\n");
      perror(Track_data_file_path);
    }
  }

}

