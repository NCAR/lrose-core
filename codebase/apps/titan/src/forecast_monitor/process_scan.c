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

#include "forecast_monitor.h"
#include <limits.h>

/****************************************************************************
 * process_scan.c
 *
 * Process scan of a given storm file
 *
 * July 1995
 *
 ****************************************************************************/

void process_scan(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		  vol_file_handle_t *v_handle,
		  si32 verify_scan_num,
		  si32 n_scans,
		  date_time_t *scan_times)

{
  si32 iscan, min_iscan;
  si32 search_generate_time;
  si32 time_diff, min_diff;
  si32 generate_scan_num;
  double actual_lead_time;
  double mean_scan_duration;
  double search_margin;
  date_time_t *verify_scan_time;
  date_time_t *forecast_generate_time;
  
  PMU_auto_register("In process_scan");

  /*
   * return now if less that 2 scans
   */
  
  if (n_scans < 2) {
    return;
  }

  verify_scan_time = scan_times + verify_scan_num;

  /*
   * reset the output module
   */

  reset_output_module(verify_scan_time->unix_time);

  if (Glob->params.print_stats) {
    fprintf(stdout, "\n");
    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "Scan %ld\n", (long) verify_scan_num);
    fprintf(stdout, "  Verify time:            %s\n",
	    utimestr(verify_scan_time));
  }
  
  /*
   * compute average scan duration and search margin
   */

  mean_scan_duration =
    (double) (scan_times[n_scans - 1].unix_time - scan_times[0].unix_time) /
      (double) (n_scans - 1);
  search_margin = mean_scan_duration / 2.0 + Glob->params.time_search_margin;

  /*
   * find scan num from which forecast for the
   * verify scan would have been made
   */

  search_generate_time = (verify_scan_time->unix_time -
			  Glob->params.forecast_lead_time);
  
  min_diff = LONG_MAX;

  for (iscan = 0; iscan < verify_scan_num; iscan++) {
    time_diff = abs(scan_times[iscan].unix_time - search_generate_time);
    if (time_diff < min_diff) {
      min_diff = time_diff;
      min_iscan = iscan;
    }
  }
  
  if (min_diff > search_margin) {
    if (Glob->params.print_stats) {
      fprintf(stdout, "  No forecast available for this time\n");
    }
    return;
  }

  generate_scan_num = min_iscan;
  forecast_generate_time = scan_times + generate_scan_num;

  if (Glob->params.print_stats) {
    fprintf(stdout, "  Forecast generate time: %s\n",
	    utimestr(forecast_generate_time));
  }
  
  actual_lead_time = (verify_scan_time->unix_time -
		      forecast_generate_time->unix_time);
  
  score_forecast(s_handle, t_handle, v_handle,
		 verify_scan_num, generate_scan_num,
		 n_scans, actual_lead_time, scan_times);

  /*
   * write the output file
   */

  write_output_file();

  return;

}








