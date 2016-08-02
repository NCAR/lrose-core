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
 * load_storm.c
 *
 * Load up storm props for scan entries which are at either the
 * generate time or the forecast time
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1995
 *
 ****************************************************************************/

#include "forecast_monitor.h"

static void load_props(storm_file_handle_t *s_handle,
		       fm_storm_t *storm);

void load_storm(storm_file_handle_t *s_handle,
		track_file_handle_t *t_handle,
		si32 verify_scan_num,
		si32 generate_scan_num,
		fm_simple_track_t *strack)

{
  
  si32 ientry;
  track_file_entry_t *entry;
  
  /*
   * initialize
   */
  
  strack->have_generate = FALSE;
  strack->have_verify = FALSE;
      
  /*
   * search for entries which occur at the generate or verify time
   */
  
  for (ientry = 0;
       ientry < t_handle->simple_params->duration_in_scans; ientry++) {
    
    if (RfReadTrackEntry(t_handle, "analyze_sub_tree") != R_SUCCESS) {
      tidy_and_exit(-1);
    }
    
    entry = t_handle->entry;
    
    if (entry->scan_num == generate_scan_num) {
      strack->have_generate = TRUE;
      strack->generate.entry = *entry;
      load_props(s_handle, &strack->generate);
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stdout, "Generate entry %d: simple num %d, storm num %d\n",
		ientry,
		entry->simple_track_num, entry->storm_num);
      }
    }
    
    if (entry->scan_num == verify_scan_num) {
      strack->have_verify = TRUE;
      strack->verify.entry = *entry;
      load_props(s_handle, &strack->verify);
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stdout, "Verify entry %d: simple num %d, storm num %d\n",
		ientry,
		entry->simple_track_num, entry->storm_num);
      }
    }
    
  } /* ientry */

}

static void load_props(storm_file_handle_t *s_handle,
		       fm_storm_t *storm)

{

  if (RfReadStormScan(s_handle, storm->entry.scan_num,
		      "load_storm")) {
    tidy_and_exit(-1);
  }
	
  if (RfReadStormProps(s_handle, storm->entry.storm_num,
		       "load_storm")) {
    tidy_and_exit(-1);
  }
  
  storm->scan = *(s_handle->scan);
  storm->gprops = s_handle->gprops[storm->entry.storm_num];

  alloc_runs(storm);

  memcpy ((void *) storm->runs,
	  (void *) s_handle->runs,
	  (size_t) (storm->gprops.n_runs * sizeof(storm_file_run_t)));

}
