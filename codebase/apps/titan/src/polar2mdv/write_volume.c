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
/***************************************************************************
 * write_volume.c
 *
 * Write a radar volume to file
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 * 
 * September 1990
 *
 **************************************************************************/

#include "polar2mdv.h"
#include <cidd/cdata_util.h>
#include <sys/stat.h>
#include <time.h>

void write_volume(vol_file_handle_t *v_handle,
		  si32 nscan,
		  si32 nrays_target,
		  si32 nrays,
		  si32 nnoise,
		  si32 *sum_noise)

{

  long ifield;
  si32 nmissing;
  
  date_time_t start, mid, end;
  date_time_t vstart;
  vol_params_t *vparams;

  int           compute_timestamp;
  static int    first_call = 1;

  if (Glob->debug) {
    fprintf(stderr,
	    "entering write_volume, time %s\n",
	    utimstr(time((time_t *) NULL)));
  }
  
  /*
   * compute number of missing rays and noise values
   */
  
  nmissing = nrays_target - nrays;
  vparams = v_handle->vol_params;
  vparams->radar.nmissing = nmissing;
    
  /*
   * if number of missing rays is less than max, write file
   */
  
  if (Glob->report_missing_beams) {
    
    fprintf(stderr, "\n");
    fprintf(stderr, "Nscan         %ld\n", (long) nscan);
    fprintf(stderr, "Nrays found   %ld\n", (long) nrays);
    fprintf(stderr, "Nrays target  %ld\n", (long) nrays_target);
    fprintf(stderr, "Nrays missing %ld\n", (long) nmissing);
    
  }
    
  if (!Glob->check_missing_beams ||
      nmissing < Glob->max_missing_beams) { 

    if (Glob->debug) {
      fprintf(stderr, "Entering write code\n");
    }
    
    /*
     * get local copies of start and end time
     */

    Rfrtime2dtime(&vparams->start_time, &start);
    Rfrtime2dtime(&vparams->end_time, &end);

    /*
     * check that the volume does not span a data 
     * outage problem - this is detected by a very
     * long (end-start) difference
     */

    if ((end.unix_time - start.unix_time) > Glob->max_vol_duration) {
      if (Glob->debug) {
	fprintf(stderr,
		"WARNING - not writing file because interval exceeds max\n");
	fprintf(stderr, "Vol start time: %s\n", utimstr(start.unix_time));
	fprintf(stderr, "Vol end time: %s\n", utimstr(end.unix_time));
	fprintf(stderr, "Max duration is %d secs\n", Glob->max_vol_duration);
	return;
      }
    } else {
      if (Glob->debug) {
	fprintf(stderr, "Note: vol duration is %d secs\n",
		(int) (end.unix_time - start.unix_time));
	fprintf(stderr, "Max duration is %d secs\n", Glob->max_vol_duration);
      }
    }

    /*
     * if end time is before start time (this may happed during playback
     * operations) return early because there is part of a scan from 
     * one time and part from another
     */

    if (end.unix_time < start.unix_time) {
      if (Glob->swap_times_if_incorrect) {
	radtim_t temp_radtim;
	date_time_t temp_time;

	temp_radtim = vparams->start_time;
	vparams->start_time = vparams->end_time;
	vparams->end_time = temp_radtim;
	
	temp_time = start;
	start = end;
	end = temp_time;
      }
      else {
	if (Glob->debug) {
	  fprintf(stderr, "WARNING - end time before start time\n");
	  fprintf(stderr, "Start time: %s\n", utimestr(&start));
	  fprintf(stderr, "End   time: %s\n", utimestr(&end));
	}
      
	return;
      }
    }
    
    /*
     * Determine whether the timestamp should be computed or
     * set using a data time on from a specified tilt
     */
    compute_timestamp = 1;
    if (Glob->timestamp_at_start_of_tilt != -1 ) {
      /*
       * use time of tilt number for mid-point in output file
       */
      compute_timestamp = 0;

      /*
       * handle the case where we've come into the ingest at the
       * middle of a volume 
       */
      if (Glob->volume_timestamp == -1 ) {
        if (first_call) {
          fprintf(stderr, "WARNING - computing timestamp for first volume.\n" );
          compute_timestamp = 1;
        }
        else {
          fprintf(stderr, "ERROR - incorrect tilt number specified for "
                        "'timestamp_at_start_of_tilt'\n"
                        "        Tilt %d not included in radar data ingest.\n",
                        Glob->timestamp_at_start_of_tilt );
          tidy_and_exit(-1);
        }
      }
    }
    if ( compute_timestamp ) {
      /*
       * compute mid julian date and time, convert back to calendar date
       */
      if (Glob->auto_mid_time)
        mid.unix_time =
	  start.unix_time + (end.unix_time - start.unix_time) / 2;
      else
        mid.unix_time = end.unix_time - Glob->age_at_end_of_volume;
    }
    else {
      mid.unix_time = Glob->volume_timestamp;
    }

    if ( first_call )
      first_call = 0;
    uconvert_from_utime(&mid);
    
    /*
     * copy mid date and time to vol_params
     */

    Rfdtime2rtime(&mid, &vparams->mid_time);

    fprintf(stdout, "%s: writing volume for time %s\n",
	    Glob->prog_name, utimestr(&mid));

    /*
     * loop through fields, loading up noise data
     */
    
    for (ifield =  0; ifield < Glob->nfields_processed; ifield++) {
      v_handle->field_params[ifield]->noise =
	(si32) ((double) sum_noise[ifield] / (double) nnoise + 0.5);
    }

    /*
     * override table latlon?
     */

    if (Glob->override_table_latlon) {
      v_handle->vol_params->radar.latitude = Glob->radar_lat * DEG_FACTOR;
      v_handle->vol_params->radar.longitude = Glob->radar_lon * DEG_FACTOR;
      v_handle->vol_params->cart.latitude = Glob->radar_lat * DEG_FACTOR;
      v_handle->vol_params->cart.longitude = Glob->radar_lon * DEG_FACTOR;
      v_handle->vol_params->cart.radarx = 0;
      v_handle->vol_params->cart.radary = 0;
    }

    /*
     * write output file
     */

    RfWriteDobsonRemote(v_handle, TRUE, Glob->debug,
			Glob->hostname,
			Glob->rdata_dir,
			Glob->output_file_ext,
			Glob->local_tmp_dir,
			Glob->prog_name,
			"write_volume");
    
  } else {
    
    if (Glob->report_missing_beams == TRUE) {
      
      fprintf(stderr, "WARNING - %s:write_volume.\n", Glob->prog_name);
      fprintf(stderr, "Too many missing rays.\n");
      fprintf(stderr, "%ld missing, only %ld allowed.\n",
	      (long) nmissing, (long) Glob->max_missing_beams);

      Rfrtime2dtime(&vparams->start_time, &vstart);

      fprintf(stderr, "Scan time %d/%d/%d %d:%d:%d\n",
	      vstart.year, vstart.month, vstart.day,
	      vstart.hour, vstart.min, vstart.sec);
      
    } /* if (Glob->report_missing_beams == TRUE) */
    
  } /* if (nmissing < Glob->max_nmissing) */

}

