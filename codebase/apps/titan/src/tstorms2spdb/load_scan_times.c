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
 * load_scan_times.c
 *
 * Loads up an array of scans times, computes the unix time
 *
 * Returns number of scans
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Spet 1997
 *
 *******************************************************************************/

#include "tstorms2spdb.h"

int load_scan_times(storm_file_handle_t *s_handle,
		    date_time_t **scan_times_p)

{

  static int first_call = TRUE;
  static date_time_t *scan_times;
  int n_scans, iscan;

  n_scans = s_handle->header->n_scans;

  if (first_call == TRUE) {

    scan_times = (date_time_t *) umalloc
      ((ui32) (n_scans * sizeof(date_time_t)));

    first_call = FALSE;

  } else {

    scan_times = (date_time_t *) urealloc
      ((char *) scan_times,
       (ui32) (n_scans * sizeof(date_time_t)));

  }

  for (iscan = 0; iscan < n_scans; iscan++) {

    if (RfReadStormScan(s_handle, iscan, "load_scan_times"))
      tidy_and_exit(-1);

    scan_times[iscan].unix_time = s_handle->scan->time;
    uconvert_from_utime(scan_times + iscan);

  } /* iscan */

  *scan_times_p = scan_times;
  return (n_scans);

}
