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

#include "precip_map.h"

static void write_index(long file_time,
			LDATA_handle_t *ldata);

static void write_index_with_fcasts(long file_time,
				    LDATA_handle_t *ldata);

void process_file(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		  vol_file_handle_t *v_handle,
		  vol_file_handle_t *map_v_handle,
		  char *track_file_path,
		  LDATA_handle_t *ldata)

{

  static int first_call = TRUE;
    
  si32 iscan, n_scans;
  long file_time;
  long time_latest_scan;
  date_time_t *scan_times;

  PMU_auto_register("In process_file");

  if (Glob->params.mode == ARCHIVE || Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "\nProcessing file %s\n", track_file_path);

  /*
   * If we are operating in realtime, make sure no-one else is using
   * the storm files.
   */

  /*
   * open track and storm files and read in headers
   */
  
  open_files(s_handle, t_handle, track_file_path);

  /*
   * load scan time array
   */

  scan_times = load_scan_times(s_handle);
  
  /*
   * generate the maps
   */

  n_scans = s_handle->header->n_scans;
  time_latest_scan = scan_times[n_scans - 1].unix_time;
  
  if (Glob->params.mode == ARCHIVE) {

    for (iscan = 0; iscan < n_scans; iscan++) {
      generate(s_handle, t_handle, v_handle, map_v_handle,
	       scan_times, iscan);
    }

  } else {

    /*
     * realtime mode
     */

    if (first_call) {

      clear_overwrite();
      for (iscan = 0; iscan < n_scans; iscan++) {
	file_time = generate(s_handle, t_handle, v_handle, map_v_handle,
			     scan_times, iscan);
      }
      first_call = FALSE;
      
    } else {
      
      set_overwrite();
      iscan = n_scans - 1;
      file_time = generate(s_handle, t_handle, v_handle, map_v_handle,
			   scan_times, iscan);
      
    } /* if (first_call) */
    
    /*
     * write index file as appropriate
     */
    
    if (file_time >= 0) {

      switch (Glob->params.map_type) {
    
      case FORECAST:
      case PERSISTENCE:
      case VERIFY:
      case REFL_FORECAST:
	write_index_with_fcasts(file_time, ldata);
	break;

      case ACCUM_PERIOD:
      case ACCUM_FROM_START:
	write_index(file_time, ldata);
	break;
	
      } /* switch */

    } /* if (file_time >= 0) */

  } /* if (Glob->params.mode == ARCHIVE) */

  /*
   * close files
   */
  
  RfCloseStormFiles(s_handle, "process_file");
  RfCloseTrackFiles(t_handle, "process_file");

  umalloc_verify();
  
}
      
static void write_index(long file_time,
			LDATA_handle_t *ldata)

{
  
  if (LDATA_info_write(ldata,
		       Glob->params.map_dir,
		       file_time,
		       Glob->params.output_file_ext,
		       NULL, NULL, 0, NULL)) {
    
    fprintf(stderr, "WARNING - %s:process_file:write_index\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot write index file to dir %s\n",
	    Glob->params.map_dir);

  }

}

static void write_index_with_fcasts(long file_time,
				    LDATA_handle_t *ldata)

{

  static int first_call = TRUE;
  static MEMbuf *mbuf;
  int i, n_fcasts;
  int fcast_lead_time;

  if (first_call) {
    mbuf = MEMbufCreate();
    first_call = FALSE;
  }
  MEMbufReset(mbuf);

  /*
   * fill the forecast times array
   */
  
  n_fcasts = Glob->params.forecast_durations.len;

  for (i = 0; i < n_fcasts; i++) {
    fcast_lead_time = Glob->params.forecast_durations.val[i];
    MEMbufAdd(mbuf, &fcast_lead_time, sizeof(int));
  }

  /*
   * write index file
   */
  
  if (LDATA_info_write(ldata,
		       Glob->params.map_dir,
		       file_time,
		       Glob->params.output_file_ext,
		       NULL, NULL,
		       n_fcasts,
		       MEMbufPtr(mbuf))) {
    
    fprintf(stderr, "WARNING - %s:process_file:write_index_with_fcasts\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot write index file to dir %s\n",
	    Glob->params.map_dir);

  }

}

