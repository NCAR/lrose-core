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
/***********************************************************************
 * handle_beam.c
 *
 * Handle a beam.
 *  If normal beam, write to file.
 *  If a status packet, write status file.
 *  If end of volume, close file and open next one.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * November 1996
 *
 * returns 0 on success, -1 on failure.
 *
 ************************************************************************/

#include "dva_ingest.h"

int handle_beam(bprp_beam_t *beam,
		dva_rdas_cal_t *cal)

{

  static int first_call = TRUE;
  static int prev_tilt_no = -1;
  static int nbeams_this_vol = 0;
  static int nbeams_last_vol = 0;

  ui16 raycount;
  /* int good; */
  int i;      
  int shift, mult;
  int radar_id;
  int julday;
  int rc;
  int print_message;
  si32 tilt_no;
  double azimuth, elevation;
  date_time_t btime, jan1;

  /*
   * initialize
   */

  if (first_call) {
    new_beam_file();
    first_call = FALSE;
  }

  PMU_auto_register("In handle_beam");

  /*
   * swap the beam
   */
  
  BE_to_array_16((ui16 *) beam, sizeof(bprp_beam_t));
  
  if (beam->hdr.site_blk == 0) {
    return (-1);
  }
  
  /*
   * if required, check radar id and skip this
   * routine if it does not match the target
   */
  
  radar_id = (int) (beam->hdr.xmt & 0x1f);

  if (Glob->params.check_radar_id) {
    if (radar_id != Glob->params.target_radar_id) {
      return (-1);
    }
  }

  /*
   * if status, handle status
   */
  
  if (beam->hdr.date == 0) {
    handle_status(beam, nbeams_last_vol);
    return (0);
  }

  /*
   * check the calibration
   */

  /*  if (Glob->params.debug) { */
    read_rdas_cal(cal);
    /* } */
  check_calib(beam, cal);

  if (Glob->params.time_mode == LOCAL_TO_UCT) {

    struct tm date;
	
    date.tm_year = beam->hdr.date / 0x200;
    date.tm_mon = 0;
    date.tm_mday = beam->hdr.date & 0x1ff; /* Strip Julian day	*/
    date.tm_hour = beam->hdr.hour;
    date.tm_min = beam->hdr.min / 60;
    date.tm_sec = beam->hdr.min % 60;
    date.tm_isdst = 0;
    btime.unix_time = mktime(&date) + Glob->params.time_correction;
    uconvert_from_utime(&btime);

    jan1 = btime;
    jan1.month = 1;
    jan1.day = 1;
    uconvert_to_utime(&jan1);

    julday = (btime.unix_time - jan1.unix_time) / 86400 + 1;

    beam->hdr.date = (btime.year % 100) * 512 + julday;
    beam->hdr.hour = btime.hour;
    beam->hdr.min = btime.min * 60 + btime.sec;

  } else {

    btime.year = beam->hdr.date / 0x200;
    if (btime.year < 2000) {
      if (btime.year < 50) {
	btime.year += 2000;
      } else {
	btime.year += 1900;
      }
    }
    btime.month = 1;
    btime.day = beam->hdr.date & 0x1ff; /* Julian day */
    btime.hour = beam->hdr.hour;
    btime.min = beam->hdr.min / 60;
    btime.sec = beam->hdr.min % 60;

    uconvert_to_utime(&btime);
    btime.unix_time += Glob->params.time_correction;
    uconvert_from_utime(&btime);

  }
  
  raycount = beam->hdr.raycount - 513;

  if (Glob->params.debug >= DEBUG_NORM) {

    azimuth = 0.0;
    elevation = 0.0;
    
    for (i = 0, shift = 12, mult = 1000;
	 i < 4;
	 i++, shift -= 4, mult /= 10) {
      azimuth += ((beam->hdr.azimuth >> shift) & 0x0F) * mult; 
      elevation += (((beam->hdr.elevation >> shift) & 0x0F) * mult); 
    }
    
    azimuth /= 10.0;
    elevation /= 10.0;
    
    tilt_no =  beam->hdr.raycount >> 9;
    rc = (beam->hdr.raycount & 0x1FF);

 /* To recover 0 tilt_no from RDAS on last ray of tilt 
    if (tilt_no <= 0) {
        tilt_no = prev_tilt_no;
        good = 0;
    }
*/
    if (Glob->params.debug >= DEBUG_VERBOSE) {
      print_message = TRUE;
    } else {
      if (tilt_no != prev_tilt_no) {
	print_message = TRUE;
	prev_tilt_no = tilt_no;
      } else {
	print_message = FALSE;
      }
    }

    if (print_message) {
      fprintf(stderr,
	      "radar_id, rc, raycount, tilt_no, az, el: "
   	      "%s %3d %3d %3d, %3d, %5.1f %5.1f\n",
	      utimestr(&btime),
              radar_id, rc, raycount, tilt_no, azimuth, elevation);
    }

  } /* if (Glob->params.debug ... */
  
  /*
   * write beam to file
  if(good){
     write_beam(beam);
     nbeams_this_vol++;
  }
  else{
     good = 1;
  }
   */
     write_beam(beam);
     nbeams_this_vol++;

  /*
   * if end of volume, open new file
   */

  if(raycount == 0) {
    new_beam_file();
    nbeams_last_vol = nbeams_this_vol;
    nbeams_this_vol = 0;
  }

  return (0);

}
