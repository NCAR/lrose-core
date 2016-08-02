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
 * generate.c
 *
 * Perform required generation.
 *
 * Returns data time for last file written
 *
 * RAP, NCAR, Boulder CO
 *
 * Jan 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "precip_map.h"

long generate(storm_file_handle_t *s_handle,
	      track_file_handle_t *t_handle,
	      vol_file_handle_t *v_handle,
	      vol_file_handle_t *map_v_handle,
	      date_time_t *scan_times,
	      si32 scan_num)

{
  
  int i;
  int n_forecast;
  long file_time;

  PMU_auto_register("In generate()");
  
  /*
   * generate the maps
   */

  switch (Glob->params.map_type) {
    
  case FORECAST:
    file_time =
      forecast_generate(s_handle, t_handle, v_handle, map_v_handle,
			scan_times, scan_num,
			Glob->params.forecast_durations.val[0]);
    break;

  case REFL_FORECAST:
    if (Glob->params.file_time_stamp == FORECAST_TIME) {
      n_forecast = Glob->params.forecast_durations.len;
    } else {
      n_forecast = 1;
    }

    for (i = 0; i < n_forecast; i++) {
      file_time = 
	refl_forecast_generate(s_handle, t_handle, v_handle, map_v_handle,
			       scan_times, scan_num,
			       Glob->params.forecast_durations.val[i]);
    }
    break;
    
  case PERSISTENCE:
    file_time =
      persistence_generate(s_handle, v_handle, map_v_handle, 
			   scan_times, scan_num,
			   Glob->params.forecast_durations.val[0]);
    break;
    
  case VERIFY:
    file_time =
      verify_generate(s_handle, v_handle, map_v_handle,
		      scan_times, scan_num,
		      Glob->params.forecast_durations.val[0]);
    break;
    
  case ACCUM_PERIOD:
    file_time =
      accum_generate(s_handle, t_handle, v_handle, map_v_handle,
		     scan_times, scan_num);
    break;

  case ACCUM_FROM_START:
    file_time =
      accum_generate(s_handle, t_handle, v_handle, map_v_handle,
		     scan_times, scan_num);
    break;
    
  } /* switch */
  
  return (file_time);

}
      
