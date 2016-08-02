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
/****************************************************************
 * setup_scan.c
 *
 * Reads in the file header and the scan_header and
 * sets up the variables for tracking
 *
 * If scan_num is positive, sets up for this scan, returns scan_num.
 *
 * If scan_num is negative, sets up for the last scan.
 */

#include "storm_track.h"

si32 setup_scan(storm_file_handle_t *s_handle,
		si32 scan_num,
		si32 *nstorms,
		date_time_t *scan_time,
		char *storm_header_path)

{
  
  /*
   * re-open storm file to force re-reads (in LINUX the re-read
   * without re-open does not seem to work)
   */
  
  RfCloseStormFiles(s_handle, "setup_scan");

  if (RfOpenStormFiles(s_handle, "r",
		       storm_header_path,
		       (char *) NULL, "setup_scan")) {
    
    fprintf(stderr, "Cannot open storm files.\n");
    perror(storm_header_path);
    tidy_and_exit(-1);
    
  }
  
  /*
   * Read in file header, to get the latest file information
   */

  if (RfReadStormHeader(s_handle, "setup_scan")) {
    tidy_and_exit(-1);
  }
  
  if (scan_num < 0) {
    scan_num = s_handle->header->n_scans - 1;
  }
  
  /*
   * read in data for this scan
   */
  
  if (RfReadStormScan(s_handle, scan_num, "setup_scan")) {
    tidy_and_exit(-1);
  }

  /*
   * store current scan time
   */
  
  scan_time->unix_time = s_handle->scan->time;
  uconvert_from_utime(scan_time);

  /*
   * set number of storms
   */

  *nstorms = s_handle->scan->nstorms;

  return (scan_num);

}

