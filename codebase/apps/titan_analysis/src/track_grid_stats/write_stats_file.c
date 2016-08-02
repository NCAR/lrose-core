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

#include "track_grid_stats.h"
#include <toolsa/ldata_info.h>
#include <sys/stat.h>
#include <sys/time.h>

#define TMP_FILE_NAME "tmp_dobson_file"

/******************************************************************
 * write_stats_file.c
 *
 * Write the map and index files
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * August 1993
 */

void write_stats_file(vol_file_handle_t *v_handle,
		      date_time_t *data_start,
		      date_time_t *data_end)

{

  static int first_call = TRUE;
  static LDATA_handle_t ldata;

  char stats_file_path[MAX_PATH_LEN];
  
  date_time_t data_mid;
  
  if (first_call) {
    LDATA_init_handle(&ldata,
		      Glob->prog_name,
		      (Glob->params.debug > DEBUG_NORM));
    first_call = FALSE;
  }
  
  /*
   * compute mid time
   */

  data_mid.unix_time = (data_start->unix_time + data_end->unix_time) / 2;
  uconvert_from_utime(&data_mid);

  Rfdtime2rtime(data_start, &v_handle->vol_params->start_time);
  Rfdtime2rtime(&data_mid, &v_handle->vol_params->mid_time);
  Rfdtime2rtime(data_end, &v_handle->vol_params->end_time);

  /*
   * compute tmp file path
   */
  
  sprintf(stats_file_path, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	  Glob->params.grid_stats_dir, PATH_DELIM,
	  data_mid.year, data_mid.month, data_mid.day,
	  PATH_DELIM,
	  data_mid.hour, data_mid.min, data_mid.sec,
	  Glob->params.output_file_ext);
  
  /*
   * write stats file
   */
  
  v_handle->vol_file_path = stats_file_path;
  
  if (RfWriteVolume(v_handle, "write_stats_file"))
    tidy_and_exit(-1);
  
  /*
   * write latest data info
   */

  if (LDATA_info_write(&ldata,
		       Glob->params.grid_stats_dir,
		       data_mid.unix_time,
		       Glob->params.output_file_ext,
		       NULL, NULL, 0, NULL)) {
    
    fprintf(stderr, "WARNING - %s:write_stats_file\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot write latest data info to dir %s\n",
	    Glob->params.grid_stats_dir);
    
  }

}
