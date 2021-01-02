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
/**************************************************************************
 * write_volume.c
 *
 * Writes the forecast volume to file
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1992
 *
 ************************************************************************/

#include "grid_forecast.h"
#include <limits.h>

void write_volume(vol_file_handle_t *v_handle,
		  date_time_t *stime)

{

  static int first_call = TRUE;

  static char prev_dir_path[MAX_PATH_LEN];
  
  char file_name[MAX_PATH_LEN];
  char file_path[MAX_PATH_LEN];
  char dir_path[MAX_PATH_LEN];
  char call_str[BUFSIZ];
  char note_add[VOL_PARAMS_NOTE_LEN];

  vol_params_t *vparams;
  date_time_t dt;

  /*
   * if first call, initialize
   */
  
  if (first_call) {
    
    strcpy(prev_dir_path, " ");
    first_call = FALSE;
    
  }

  vparams = v_handle->vol_params;

  /*
   * adjust the scan times to account for the forecast lead time
   */

  Rfrtime2dtime(&vparams->start_time, &dt);
  dt.unix_time += Glob->forecast_lead_time;
  uconvert_from_utime(&dt);
  Rfdtime2rtime(&dt, &vparams->start_time);

  Rfrtime2dtime(&vparams->mid_time, &dt);
  dt.unix_time += Glob->forecast_lead_time;
  uconvert_from_utime(&dt);
  Rfdtime2rtime(&dt, &vparams->mid_time);

  Rfrtime2dtime(&vparams->end_time, &dt);
  dt.unix_time += Glob->forecast_lead_time;
  uconvert_from_utime(&dt);
  Rfdtime2rtime(&dt, &vparams->end_time);

  /*
   * append to the note
   */
  
  sprintf(note_add, "%s%s%s%ld%s",
	  "\n Forecast volume, original time ",
	  utimestr(stime), ", lead_time ",
	  (long) Glob->forecast_lead_time,
	  " secs");

  strncat(vparams->note, note_add,
	  VOL_PARAMS_NOTE_LEN - strlen(vparams->note));
  
  /*
   * compute output file name from the mid date and time
   */
  
  sprintf(dir_path, "%s%s%.4d%.2d%.2d",
	  Glob->forecast_rdata_dir, PATH_DELIM,
	  vparams->mid_time.year,
	  vparams->mid_time.month,
	  vparams->mid_time.day);
    
  sprintf(file_name, "%.2d%.2d%.2d.dob",
	  vparams->mid_time.hour,
	  vparams->mid_time.min,
	  vparams->mid_time.sec);
    
  sprintf(file_path, "%s%s%s",
	  dir_path, PATH_DELIM,
	  file_name);
    
  /*
   * if current directory paths are not same as previous ones,
   * create directories
   */
  
  if (strcmp(dir_path, prev_dir_path)) {

    sprintf(call_str, "mkdir %s", dir_path);
    errno = 0;
    system(call_str);

    if (errno) {
      fprintf(stderr, "WARNING - %s:write_volume\n", Glob->prog_name);
      perror(call_str);
    }

    strcpy(prev_dir_path, dir_path);
	
  }
      
  v_handle->vol_file_path = file_path;

  if (Glob->debug)
    printf("writing file %s\n", file_path);

  if (RfWriteVolume(v_handle,
		    "output_to_cart:write_volume") != R_SUCCESS)
    exit(-1);

}
