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

#include "tstorms2spdb.h"

static storm_file_handle_t S_Handle;
static track_file_handle_t T_Handle;

static void init_indices(void);

void process_file(char *track_file_path)

{
  
  static int first_call = TRUE;

  date_time_t *scan_times;
  int n_scans, iscan;
  
  PMU_auto_register("In process_file");

  if (first_call) {
    init_indices();
    first_call = FALSE;
  }

  /*
   * open track and storm files
   */
  
  open_track_files(&S_Handle, &T_Handle, track_file_path);

  /*
   * load scan time array
   */

  n_scans = load_scan_times(&S_Handle, &scan_times);

  /*
   * generate the forecast stats
   */

  if (Glob->params.mode == REALTIME) {
    
    /*
     * realtime mode
     *
     * process latest scan
     */
    
    process_scan(&S_Handle, &T_Handle, (n_scans - 1), n_scans, scan_times);
    
  } else {
    
    /*
     * archive mode
     *
     * loop through all scans
     */

    for (iscan = 0; iscan < n_scans; iscan++) {
      process_scan(&S_Handle, &T_Handle, iscan, n_scans, scan_times);
    } /* iscan */
    
  } /* if (Glob->params.mode == REALTIME) */

  /*
   * close track and storm files
   */
  
  close_track_files(&S_Handle, &T_Handle);
  
}

static void init_indices(void)

{

  /*
   * initialize storm and track file indices
   */
  
  RfInitStormFileHandle(&S_Handle, Glob->prog_name);
  
  RfInitTrackFileHandle(&T_Handle, Glob->prog_name);
  
}
