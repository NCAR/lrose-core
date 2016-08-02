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
 * loads up a beam in shared memory
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1991
 *
 **************************************************************************/

#include "polar_ingest.h"

void load_beam (si32 beam_num,
		int load_data)

{
  
  static int first_call = TRUE;
  static global_t glob;
  static double deg_factor;
  static int apply_flags;
  static int use_bit_mask;
  static ui08 flag_check_val;
  static ui08 flag_value_min;
  static ui08 flag_value_max;
  
  ui08 *in_data, *out_data;
  ui08 *source, *dest, *flag;

  int field_found;
  si32 ifield, igate;
  si32 nfields_found;
  si32 ngates_in;
  si32 field_flag;
  si32 nfields_current;
  si32 ngates_out, ngates_copied;
  si32 test_mask;
  si32 az;
  
  ll_params_t *ll_params;
  rp7_params_t *rp7_params;
  rdata_shmem_beam_header_t *bhdr;
  bprp_params_t *bprp_params;
  bprp_data_t *bprp_data;
  gate_data_radar_params_t *gate_rparams;
  gate_data_field_params_t *gate_fparams;
  gate_data_beam_header_t *gate_header;
  date_time_t btime;
  radar_params_t *rparams;
  
  /*
   * initialize
   */
  
  if (first_call) {
    
    deg_factor = 360.0 / 65536.0;

    /*
     * for speed, make local copy of globals
     */

    memcpy ((void *) &glob,
            (void *) Glob,
            (size_t) sizeof(global_t));

    /*
     * set up flag byte checking
     */

    apply_flags = glob.apply_flags;
    use_bit_mask = glob.use_bit_mask;
    flag_check_val = glob.flag_check_val;
    flag_value_min = glob.flag_value_min;
    flag_value_max = glob.flag_value_max;
    
  }
  
  rparams = &Glob->shmem_header->radar;

  /*
   * set local variables
   */
  
  bhdr = glob.beam_headers + beam_num;
  
  /*
   * switch on the header type, and set the relevant
   * portions of the shared memory area
   */
  
  switch (glob.header_type) {
    
  case LINCOLN_HEADER:

    get_lincoln_ptrs(&ll_params, &in_data);
    
    btime.year = ll_params->year;
    btime.month = ll_params->month;
    btime.day = ll_params->day;
    btime.hour = ll_params->hour;
    btime.min = ll_params->min;
    btime.sec = ll_params->sec;
    bhdr->beam_time = uunix_time(&btime);
    
    bhdr->azimuth = (double) ll_params->azimuth / 100.0;
    bhdr->elevation = (double) ll_params->elevation / 100.0;
    bhdr->target_elev = (double) ll_params->target_elev / 100.0;
    
    bhdr->vol_num = ll_params->vol_num;
    bhdr->tilt_num = ll_params->tilt_num;
    
    bhdr->end_of_tilt = -1;
    bhdr->end_of_volume = -1;
    
    ngates_in = ll_params->range_seg[0].gates_per_beam;

    if (first_call)
      bhdr->new_scan_limits = TRUE;
    else
      bhdr->new_scan_limits = FALSE;

    field_flag = 0xffffffff;
    nfields_current = glob.nfields_in;
    
    rparams->gate_spacing = ll_params->range_seg[0].gate_spacing * 1000;

    rparams->start_range =
      (si32) floor(((double) ll_params->range_to_first_gate +
		    ((double) ll_params->range_seg[0].gate_spacing / 2.0)) *
		   1000.0 + 0.5);

    break;
    
  case RP7_HEADER:
    
    get_rp7_ptrs(&rp7_params, &in_data);

    btime.year = rp7_params->year;
    btime.month = rp7_params->month;
    btime.day = rp7_params->day;
    btime.hour = rp7_params->hour;
    btime.min = rp7_params->min;
    btime.sec = rp7_params->sec;
    bhdr->beam_time = uunix_time(&btime);
    
    bhdr->azimuth = (double) rp7_params->azimuth * deg_factor;
    bhdr->elevation = (double) rp7_params->elevation * deg_factor;
    bhdr->target_elev = (double) rp7_params->target_elev * deg_factor;
    
    bhdr->vol_num = rp7_params->vol_num;
    bhdr->tilt_num = rp7_params->tilt_num;
    
    ngates_in = rp7_params->gates_per_beam;

    bhdr->end_of_tilt = -1;
    bhdr->end_of_volume = -1;
    
    if (first_call)
      bhdr->new_scan_limits = TRUE;
    else
      bhdr->new_scan_limits = FALSE;

    field_flag = 0xffffffff;
    nfields_current = glob.nfields_in;

    rparams->gate_spacing = rp7_params->gate_spacing * 1000;
    
    rparams->start_range =
      (si32) floor(((double) rp7_params->rhozero1 +
		    (double) rp7_params->rhozero2 +
		    ((double) rp7_params->gate_spacing / 2.0))
		   * 1000.0 + 0.5);
    
    break;
    
  case BPRP_HEADER:

    get_bprp_ptrs(&bprp_params, &bprp_data);
    
    btime.year = bprp_params->year;
    btime.month = bprp_params->month;
    btime.day = bprp_params->day;
    btime.hour = bprp_params->hour;
    btime.min = bprp_params->min;
    btime.sec = bprp_params->sec;
    bhdr->beam_time = uunix_time(&btime);

    bhdr->azimuth = bprp_params->azimuth;
    bhdr->elevation = bprp_params->elevation;
    bhdr->target_elev = bprp_params->target_elevation;

    bhdr->vol_num = (si32) bprp_params->vol_num;
    bhdr->tilt_num = (si32) bprp_params->tilt_num;
    
    in_data = (ui08 *) bprp_data;
    ngates_in = BPRP_GATES_PER_BEAM;                  

    bhdr->end_of_tilt = -1;
    bhdr->end_of_volume = -1;
    
    if (first_call)
      bhdr->new_scan_limits = TRUE;
    else
      bhdr->new_scan_limits = FALSE;

    field_flag = 0xffffffff;             
    nfields_current = glob.nfields_in;

    rparams->gate_spacing = (si32) (BPRP_GATE_SPACING * 1000000.0 + 0.5); 
    rparams->start_range =
      (si32) ((BPRP_START_RANGE +
	       BPRP_GATE_SPACING / 2.0)* 1000000.0 + 0.5);
    break;
    
  case GATE_DATA_HEADER:
    
    get_gate_data_ptrs(&gate_rparams, &gate_fparams,
		       &gate_header, &in_data);
    
    bhdr->beam_time = gate_header->time;

    az = gate_header->azimuth;

    bhdr->azimuth = (double) az / 1000000.0;
    bhdr->elevation = (double) gate_header->elevation / 1000000.0;
    bhdr->target_elev = (double) gate_header->target_elev / 1000000.0;
    
    bhdr->vol_num = gate_header->vol_num;
    bhdr->tilt_num = gate_header->tilt_num;

    bhdr->end_of_tilt = gate_header->end_of_tilt;
    bhdr->end_of_volume = gate_header->end_of_volume;
    
    ngates_in = gate_rparams->ngates;

    bhdr->new_scan_limits = gate_header->new_scan_limits;

    field_flag = gate_rparams->field_flag;
    nfields_current = gate_rparams->nfields_current;

    rparams->gate_spacing = gate_rparams->gate_spacing;
    rparams->start_range = gate_rparams->start_range;

    break;
    
  } /* switch */

  /*
   * fix time as needed
   */

  if (Glob->time_override) {
    bhdr->beam_time = time(NULL);
  } else if (Glob->time_correction != 0) {
    bhdr->beam_time += Glob->time_correction;
  }

  /*
   * check angle ranges
   */

  while (bhdr->azimuth < 0.0) {
    bhdr->azimuth += 360.0;
  }
  
  while (bhdr->azimuth > 360.0) {
    bhdr->azimuth -= 360.0;
  }
  
  while (bhdr->elevation < -180.0) {
    bhdr->elevation += 360.0;
  }

  while (bhdr->elevation > 180.0) {
    bhdr->elevation -= 360.0;
  }

  while (bhdr->target_elev < -180.0) {
    bhdr->target_elev += 360.0;
  }

  while (bhdr->target_elev > 180.0) {
    bhdr->target_elev -= 360.0;
  }

  /*
   * copy the incoming beam data into shared memory
   */
  
  if (load_data) {

    ngates_out = glob.ngates_out;
    ngates_copied = ngates_in - glob.ngates_dropped;
    if (ngates_copied > ngates_out)
      ngates_copied = ngates_out;

    /*
     * if required, apply flag field info to data to remove clutter
     * and/or second trip
     */

    if (apply_flags) {

      if (glob.data_field_by_field)
      {
	source = in_data + glob.ngates_dropped;
	flag = source + ngates_in * glob.flag_field_pos;
	
	for (igate = 0; igate < ngates_copied; igate++)
	{
	  if ((use_bit_mask &&
	       *flag & flag_check_val) ||
	      (*flag >= flag_value_min &&
	       *flag <= flag_value_max))
	  {
	    for (ifield = 0; ifield < glob.nfields_in; ifield++)
	      memset((void *)(source + ifield * ngates_in),
		     (int) 0, (int)1);
	  } /* endif - gate flagged */

	  source++;
	  flag++;
	  
	} /* endfor - igate */
	
      }
      else
      {
	source = in_data + glob.ngates_dropped * nfields_current;
	flag = source + glob.flag_field_pos;

	for (igate = 0; igate < ngates_copied; igate++) {
	  if ((use_bit_mask &&
	       *flag & flag_check_val) ||
	      (*flag >= flag_value_min &&
	       *flag <= flag_value_max)) {
	    memset ((void *) source, (int) 0, (int) nfields_current);
	  }
	  source += nfields_current;
	  flag += nfields_current;
	} /* igate */
      } /* endif - glob.data_field_by_field */

    } /* if (apply_flags) */

    /*
     * zero out beam data area
     */

    out_data = (ui08 *) glob.shmem_buffer + bhdr->field_data_offset;
    memset ((void *)  out_data,
            (int) 0, (size_t) (glob.nfields_in * glob.ngates_out));

    /*
     * check if all fields are present - this makes the
     * copying more straightforward
     */

    if (nfields_current == glob.nfields_in) {

      /*
       * all fields present
       */

      if (glob.data_field_by_field) {
	
	for (ifield = 0; ifield < nfields_current; ifield++) {

	  source = in_data + 
	    ifield * ngates_in + glob.ngates_dropped;

	  dest = out_data +
	    ifield * glob.ngates_out;
	  
	  memcpy ((void *) dest,
		  (void *) source,
		  (size_t) ngates_copied);
	  
	} /* ifield */

      } else {

	memcpy ((void *) out_data,
		(void *) (in_data +
			  glob.ngates_dropped * nfields_current),
		(size_t) (nfields_current * ngates_copied));

      } /* if (glob.data_field_by_field) */

    } else {

      /*
       * fields available are indicated by field flag
       */

      test_mask = 1;
      nfields_found = 0;

      for (ifield = 0; ifield < glob.nfields_in; ifield++) {

	if (field_flag & test_mask) {
	  field_found = TRUE;
	  nfields_found++;
	} else {
	  field_found = FALSE;
	}
	
	if (field_found) {

	  if (glob.data_field_by_field) {

	    source = in_data + 
	      (nfields_found - 1) * ngates_in + glob.ngates_dropped;
	    
	    dest = out_data +
	      ifield * glob.ngates_out;
	    
	    memcpy ((void *) dest,
		    (void *) source,
		    (size_t) ngates_copied);
	    
	  } else {
	    
	    source = in_data +
	      glob.ngates_dropped * nfields_current +
		(nfields_found - 1);
	    
	    dest = out_data + ifield;

	    for (igate = 0; igate < ngates_copied; igate++) {
	      *dest = *source;
	      source += nfields_current;
	      dest += glob.nfields_in;
	    } /* igate */
	    
	  } /* if (glob.data_field_by_field) */
	  
	} /* if (field_found) */

	test_mask <<= 1;
	
      } /* ifield */

    } /* if (nfields_current == glob.nfields_in) */

    /*
     * if rdi message queue is open, write to it
     */

    if (Glob->write_rdi_mmq) {
      write_rdi_message(bhdr, out_data, nfields_current);
    }

  } /* if (load_data) */
  
  first_call = FALSE;

}

