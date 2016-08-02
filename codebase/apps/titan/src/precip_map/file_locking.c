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
 * file_locking.c
 *
 * File locking for storm and track files
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "precip_map.h"
#include <toolsa/file_io.h>

void lock_files(storm_file_handle_t *s_handle,
		track_file_handle_t *t_handle)
     
{

  /*
   * get read lock on header files
   */

  if (ta_lock_file_procmap(s_handle->header_file_path,
			   s_handle->header_file, "r")) {
    fprintf(stderr, "WARNING - %s:lock_files\n", Glob->prog_name);
  }

  if (ta_lock_file_procmap(t_handle->header_file_path,
			   t_handle->header_file, "r")) {
    fprintf(stderr, "WARNING - %s:lock_files\n", Glob->prog_name);
  }

}

void unlock_files(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle)

{

  /*
   * clear lock on header files
   */
  
  if (ta_unlock_file(s_handle->header_file_path,
		     s_handle->header_file)) {
    fprintf(stderr, "WARNING - %s:unlock_files\n", Glob->prog_name);
  }
  
  if (ta_unlock_file(t_handle->header_file_path,
		     t_handle->header_file)) {
    fprintf(stderr, "WARNING - %s:unlock_files\n", Glob->prog_name);
  }

}

void lock_and_read_headers(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle)
     
{

  /*
   * lock files
   */
  
  lock_files(s_handle, t_handle);
  
  /*
   * read in track file header
   */

  if (RfReadTrackHeader(t_handle, "open_files"))
    tidy_and_exit(-1);

  /*
   * read in storm properties file header
   */

  if (RfReadStormHeader(s_handle, "open_files"))
    tidy_and_exit(-1);

  /*
   * read in first scan
   */

  if (RfReadStormScan(s_handle, 0, "open_files")) {
    tidy_and_exit(-1);
  }

}

