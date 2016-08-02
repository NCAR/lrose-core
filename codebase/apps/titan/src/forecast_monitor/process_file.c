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

#include "forecast_monitor.h"
#include <toolsa/file_io.h>

void process_file(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		  vol_file_handle_t *v_handle,
		  char *track_file_path)

{

  date_time_t *scan_times;
  si32 n_scans, iscan;
  
  PMU_auto_register("In process_file");

  if (Glob->params.mode == ARCHIVE ||
      Glob->params.print_stats ||
      Glob->params.debug >= DEBUG_NORM) {
    fprintf(stdout, "\nProcessing file %s\n", track_file_path);
  }
  
  /*
   * open track and storm files
   */
  
  open_files(s_handle, t_handle, track_file_path);

  /*
   * get read lock on header files
   */

  if (ta_lock_file_procmap(s_handle->header_file_path,
			   s_handle->header_file, "r")) {
    fprintf(stderr, "WARNING - %s:process_file\n", Glob->prog_name);
  }

  if (ta_lock_file_procmap(t_handle->header_file_path,
			   t_handle->header_file, "r")) {
    fprintf(stderr, "WARNING - %s:process_file\n", Glob->prog_name);
  }

  /*
   * load scan time array
   */

  n_scans = load_scan_times(s_handle, &scan_times);

  /*
   * initialize the output module
   */
  
  init_output_module(s_handle);

  /*
   * generate the forecast stats
   */

  if (Glob->params.mode == REALTIME) {
    
    /*
     * realtime mode
     *
     * process latest scan
     */
    
    process_scan(s_handle, t_handle, v_handle,
		 n_scans - 1, n_scans, scan_times);
    
  } else {
    
    /*
     * archive mode
     *
     * loop through all scans
     */

    for (iscan = 0; iscan < n_scans; iscan++) {
      process_scan(s_handle, t_handle, v_handle,
		   iscan, n_scans,scan_times);
    } /* iscan */
    
  } /* if (Glob->params.mode == REALTIME) */

  /*
   * clear lock on header files
   */
  
  if (ta_unlock_file(s_handle->header_file_path,
		     s_handle->header_file)) {
    fprintf(stderr, "WARNING - %s:process_file\n", Glob->prog_name);
  }

  if (ta_unlock_file(t_handle->header_file_path,
		     t_handle->header_file)) {
    fprintf(stderr, "WARNING - %s:process_file\n", Glob->prog_name);
  }

  /*
   * close files
   */
  
  RfCloseStormFiles(s_handle, "process_file");
  RfCloseTrackFiles(t_handle, "process_file");
  
}
