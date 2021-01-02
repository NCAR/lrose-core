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
 * process_file()
 *
 * process track file
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1993
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_grid_stats.h"

void process_file(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		  char *track_file_path,
		  grid_stats_t **stats,
		  date_time_t *data_start,
		  date_time_t *data_end,
		  si32 *n_scans_total)

{
  
  date_time_t dtime;

  fprintf(stderr, "\nProcessing file %s\n", track_file_path);

  /*
   * open track and storm files
   */
  
  open_files(s_handle, t_handle, track_file_path);
  
  /*
   * load up the stats grid
   */

  load_stats_grid(s_handle, t_handle, stats);
  
  /*
   * set start and end times if applicable
   */

  *n_scans_total += s_handle->header->n_scans;

  dtime.unix_time = s_handle->header->start_time;
  uconvert_from_utime(&dtime);
  
  if (dtime.unix_time < data_start->unix_time)
    *data_start = dtime;
  
  dtime.unix_time = s_handle->header->end_time;
  uconvert_from_utime(&dtime);
  
  if (dtime.unix_time > data_end->unix_time)
    *data_end = dtime;
  
  /*
   * close files
   */
  
  RfCloseStormFiles(s_handle, "process_file");
  RfCloseTrackFiles(t_handle, "process_file");
  
}
      

