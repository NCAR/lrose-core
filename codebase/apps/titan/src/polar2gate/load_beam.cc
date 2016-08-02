// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/***************************************************************************
 * load_beam.c
 *
 * loads beam into packet memory
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * September 1992
 *
 **************************************************************************/

#include "polar2gate.h"
#include <time.h>

static void set_tilt_and_vol_num(gate_data_beam_header_t *bhdr);

void load_beam (ui08 *beam_buffer,
		ui08 *gate_params_pkt,
		ui08 *gate_data_packet,
		int load_data)

{
  
  static int first_call = TRUE;
  static double deg_factor;
  static si32 prev_cwlim = -1;
  static si32 prev_ccwlim = -1;
  
  ui08 *in_data, *out_data;
  ui08 *in_ptr, *out_ptr, *out_start;

  si32 ifield, igate;
  si32 ngates_data;
  si32 ngates_out;
  si32 ngates_copied;
  si32 nfields_in, nfields_out;
  si32 cwlim, ccwlim;
  si32 mask;
  si32 current_time;
  int ii;
  
  date_time_t dt;
  ll_params_t *ll_params;
  rp7_params_t *rp7_params;
  chill_params_t *ch_params;
  chldat1_t *chldat1;
  lass_params_t * lass_params;
  gate_data_beam_header_t *bhdr;
  gate_data_radar_params_t *rparams;
  alenia_params_t *al_params;

  /*
   * initialize
   */
  
  if (first_call)
    deg_factor = 360.0 / 65535.0;
  
  bhdr = (gate_data_beam_header_t *) gate_data_packet;
  out_data = (ui08 *) gate_data_packet + sizeof(gate_data_beam_header_t);
  rparams = (gate_data_radar_params_t *) gate_params_pkt;
  current_time = time((time_t *) NULL);

  /*
   * switch on the header type, and set the relevant
   * portions of the shared memory area
   */
  
  switch (Glob->header_type) {
    
  case ALENIA_HEADER:
    al_params = (alenia_params_t *) beam_buffer;
    bhdr->time = al_params->time;
    bhdr->azimuth = (si32) (al_params->azimuth * 1000000.0 + 0.5);
    bhdr->elevation = (si32) (al_params->elevation * 1000000.0); 
    bhdr->target_elev = bhdr->elevation;
    bhdr->vol_num = 0;
    bhdr->tilt_num = 0;
    bhdr->end_of_tilt = 0;
    bhdr->end_of_volume = 0;
    in_data = (ui08 *) beam_buffer + sizeof(alenia_params_t) +
      Glob->ngates_dropped;
    ngates_data = al_params->ngates;
    cwlim = 0;
    ccwlim = 0;
    
    break;

  case LASS_HEADER:
    lass_params = (lass_params_t *)beam_buffer;
    bhdr->time = current_time;
    bhdr->azimuth = lass_params->myAzymuth * 1000000.0; 
    bhdr->elevation = lass_params->myElevation* 1000000.0; 
    bhdr->target_elev = lass_params->targetAngle* 1000000.0; 
    bhdr->vol_num = lass_params->volSummary.volume;
    bhdr->tilt_num = lass_params->rays.sweep;
    bhdr->end_of_tilt = 0;
    bhdr->end_of_volume = 0;
    in_data = (ui08 *) (&lass_params->fieldValues[0]);
    ngates_data = lass_params->rays.numgates;
    if ( Glob->debug )
    for ( ii = 0; ii < 20; ii+= 2 )
    {
         printf("socket db %d vel %d\n",in_data[ii],in_data[ii+1]);
    }
    cwlim = 0;
    ccwlim = 0;
    
    break;

  case LINCOLN_HEADER:
    
    ll_params = (ll_params_t *) beam_buffer;
    
    if (Glob->set_time_to_current) {
      bhdr->time = current_time;
    } else {
      dt.year = ll_params->year;
      dt.month = ll_params->month;
      dt.day = ll_params->day;
      dt.hour = ll_params->hour;
      dt.min = ll_params->min;
      dt.sec = ll_params->sec;
      uconvert_to_utime(&dt);
      bhdr->time = dt.unix_time + Glob->time_correction;
    }
    
    bhdr->azimuth = (si32)
      (((double) ll_params->azimuth / 100.0) * 1000000.0 + 0.5);

    bhdr->elevation = (si32)
      (((double) ll_params->elevation / 100.0) * 1000000.0 + 0.5);

    bhdr->target_elev = (si32)
      (((double) ll_params->target_elev / 100.0) * 1000000.0 + 0.5);
    
    bhdr->vol_num = ll_params->vol_num;
    bhdr->tilt_num = ll_params->tilt_num;

    bhdr->end_of_tilt = 0;
    bhdr->end_of_volume = 0;
    
    in_data = (ui08 *) beam_buffer + sizeof(ll_params_t) +
      Glob->ngates_dropped * Glob->nfields_in;
    
    ngates_data = ll_params->range_seg[0].gates_per_beam;

    cwlim = 0;
    ccwlim = 0;
    
    break;
    
  case RP7_HEADER:
    
    rp7_params = (rp7_params_t *) beam_buffer;
    
    if (Glob->set_time_to_current) {
      bhdr->time = current_time;
    } else {
      dt.year = rp7_params->year;
      dt.month = rp7_params->month;
      dt.day = rp7_params->day;
      dt.hour = rp7_params->hour;
      dt.min = rp7_params->min;
      dt.sec = rp7_params->sec;
      uconvert_to_utime(&dt);
      bhdr->time = dt.unix_time + Glob->time_correction;
    }
    
    bhdr->azimuth = (si32)
      (((double) rp7_params->azimuth * deg_factor) * 1000000.0 + 0.5);

    bhdr->elevation = (si32)
      (((double) rp7_params->elevation * deg_factor) * 1000000.0 + 0.5);

    bhdr->target_elev = (si32)
      (((double) rp7_params->target_elev * deg_factor) * 1000000.0 + 0.5);
    
    bhdr->end_of_tilt = 0;
    bhdr->end_of_volume = 0;
    
    in_data = (ui08 *) beam_buffer + sizeof(rp7_params_t) +
      Glob->ngates_dropped * Glob->nfields_in;
    
    bhdr->vol_num = rp7_params->vol_num;
    bhdr->tilt_num = rp7_params->tilt_num;
    
    ngates_data = rp7_params->gates_per_beam;

    cwlim = rp7_params->cw_az_lim;
    ccwlim = rp7_params->ccw_az_lim;
    
    break;
    
  case CHILL_HEADER:
    
    ch_params = (chill_params_t *) beam_buffer;
    chldat1 = &ch_params->chldat1;
    
    if (Glob->set_time_to_current) {
      bhdr->time = current_time;
    } else {
      dt.year = chldat1->year;
      dt.month = chldat1->month;
      dt.day = chldat1->day;
      dt.hour = chldat1->hour;
      dt.min = chldat1->min;
      dt.sec = chldat1->sec;
      uconvert_to_utime(&dt);
      bhdr->time = dt.unix_time + Glob->time_correction;
    }
    
    bhdr->azimuth = (si32)
      (((double) chldat1->az * CHILL_DEG_CONV) * 1000000.0 + 0.5);

    bhdr->elevation = (si32)
      (((double) chldat1->el * CHILL_DEG_CONV) * 1000000.0 + 0.5);

    bhdr->target_elev = (si32) (ch_params->target_el * 1000000.0 + 0.5);
    
    bhdr->end_of_tilt = 0;
    bhdr->end_of_volume = 0;
    
    in_data = (ui08 *) beam_buffer + sizeof(chill_params_t) +
      Glob->ngates_dropped;
    
    bhdr->vol_num = chldat1->volnum;
    bhdr->tilt_num = chldat1->sweepnum;
    
    ngates_data = ch_params->ngates;

    cwlim = chldat1->azcwlim;
    ccwlim = chldat1->azccwlim;
    
    break;
    
  } /* switch */

  if (Glob->use_elev_table) {
    set_tilt_and_vol_num(bhdr);
  }

  /*
   * set flag if scan limits have changed
   */

  if (rparams->scan_mode != GATE_DATA_SURVEILLANCE_MODE &&
      ccwlim != prev_ccwlim) {
      
    bhdr->new_scan_limits = TRUE;
    
    printf("Scan az limits have changed\n");
    printf("Prev cw, ccw = %g, %g\n",
	   (double) prev_cwlim * CHILL_FIXED_CONV,
	   (double) prev_ccwlim * CHILL_FIXED_CONV);
    printf("This cw, ccw = %g, %g\n",
	   (double) cwlim * CHILL_FIXED_CONV,
	   (double) ccwlim * CHILL_FIXED_CONV);
    
    prev_cwlim = cwlim;
    prev_ccwlim = ccwlim;
    
  } else {
  
    bhdr->new_scan_limits = FALSE;

  } /* if (rparams->scan_mode ... */

  /*
   * load up the beam data
   */

  if (load_data) {

    /*
     * copy the incoming beam data into packet
     */

    nfields_in = Glob->nfields_in;
    nfields_out = Glob->nfields_out;
    ngates_out = Glob->ngates_out;
    ngates_copied = ngates_data - Glob->ngates_dropped;

    if (ngates_copied > ngates_out)
      ngates_copied = ngates_out;

    memset ((void *)  out_data,
            (int) 0, (size_t) (rparams->nfields_current * ngates_out));

    if (Glob->data_field_by_field) {

      out_ptr = out_data;

      for (ifield = 0; ifield < Glob->nfields_out; ifield++) {

	in_ptr = in_data + Glob->out_field_pos[ifield] * ngates_data;
	
	mask = 1L << ifield;

	if (rparams->field_flag | mask) {

	  memcpy ((void *) out_ptr,
		  (void *) in_ptr,
		  (size_t) ngates_copied);
	  
	  out_ptr += ngates_out;

	}

      } /* ifield */

    } else {

      out_start = out_data;

      for (ifield = 0; ifield < Glob->nfields_out; ifield++) {

	mask = 1L << ifield;
	
	if (rparams->field_flag | mask) {
	
	  in_ptr = in_data + Glob->out_field_pos[ifield];
	  out_ptr = out_start;
	
	  for (igate = 0; igate < ngates_copied; igate++) {
	  
	    *out_ptr = *in_ptr;
	    in_ptr += nfields_in;
	    out_ptr += nfields_out;

	  } /* igate */

	  out_start++;

	} /* if (rparams ... */
	
      } /* ifield */

    } /* if (Glob->data_field_by_field) */

  } /* if (load_data) */
  
  first_call = FALSE;

}

static void set_tilt_and_vol_num(gate_data_beam_header_t *bhdr)

{

  static si32 vol_num = 0;
  static si32 prev_tilt_num = 0;
  int i;
  si32 tilt_num;
  double elevation;
  double min_diff = 1000.0;
  double diff;

  elevation = (double) bhdr->elevation / 1000000.0;

  for (i = 0; i < Glob->nelev_table; i++) {

    diff = fabs(Glob->elev_table[i] - elevation);

    if (diff < min_diff) {
      tilt_num = i;
      min_diff = diff;
    }

  } /* i */

  if (prev_tilt_num != 0 && tilt_num == 0) {
    vol_num++;
  }

  prev_tilt_num = tilt_num;
  bhdr->tilt_num = tilt_num;
  bhdr->vol_num = vol_num;

}
