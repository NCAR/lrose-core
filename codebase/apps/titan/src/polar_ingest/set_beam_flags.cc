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
 * set_beam_flags.c
 *
 * Sets the flags which indicate if the beam is the last in a tilt
 * or the last in a volume. Also sets the number of missing beams
 * since the previous beam.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1991
 *
 **************************************************************************/

#include "polar_ingest.h"

void set_beam_flags (si32 prev_beam_num,
		     si32 current_beam_num,
		     int current_beam_valid,
		     int prev_beam_valid,
		     int scan_mode,
		     int *end_of_vol)

{

  static int prev_vol_flag = -1;
  static int prev_tilt_flag = -1;

  rdata_shmem_beam_header_t *prev, *current;

  /*
   * set pointers
   */

  prev = Glob->beam_headers + prev_beam_num;
  current = Glob->beam_headers + current_beam_num;
  current->scan_mode = scan_mode;

  /*
   * if current beam is not valid but prev beam was valid, store the
   * vol and tilt flags for later use when a valid beam is received
   */

  if (!current_beam_valid && prev_beam_valid) {

    prev_vol_flag = prev->end_of_volume;
    prev_tilt_flag = prev->end_of_tilt;

  }

  /*
   * if current beam is valid and prev beam was not valid, use the
   * vol and tilt flags stored previously. This resets the flags to
   * those originally on the last valid beam before this one
   */

  if (current_beam_valid && !prev_beam_valid) {

    prev->end_of_volume = prev_vol_flag;
    prev->end_of_tilt = prev_tilt_flag;

  }

  /*
   * check for change in volume, if the end_of_volume flag has not
   * been set
   */

  if (prev->end_of_volume >= 0) {

    /*
     * end_of_volume flag set by a previous process
     */

    if (prev->end_of_volume)
      if (Glob->summary_print || Glob->header_print)
	fprintf (stderr, "End of volume - set by previous process\n");

  } else {

    /*
     * end_of_volume flag not yet set
     */

    if (Glob->end_of_vol_decision == END_OF_VOL_FLAG) {

      if (prev->vol_num != current->vol_num) {

	prev->end_of_volume = TRUE;

	if (Glob->summary_print || Glob->header_print)
	  fprintf (stderr, "End of volume - change in vol num\n");

      } else { 
      
	prev->end_of_volume = FALSE;

      }

    } else if (Glob->end_of_vol_decision == END_OF_GIVEN_TILT) {

      if (fabs(prev->target_elev -  current->target_elev) > 0.05 &&
	  prev->tilt_num == Glob->last_tilt_in_vol) {
	
	prev->end_of_volume = TRUE;

	if (Glob->summary_print || Glob->header_print)
	  fprintf (stderr, "End of volume - end of given tilt\n");

      } else { 
	
	prev->end_of_volume = FALSE;
	
      }
	
    } /* if (Glob->end_of_vol_decision ... */

  } /* if (prev->end_of_volume >= 0) */

  /*
   * check for change in tilt
   */

  if (prev->end_of_tilt >= 0) {

    /*
     * end_of_tilt flag set by a previous process
     */

    if (prev->end_of_tilt)
      if ((Glob->summary_print && Glob->summary_interval <= 90)
	  || Glob->header_print)
	fprintf (stderr, "End of tilt - set by previous process\n");

  } else if (scan_mode != DIX_RHI_MODE) {

    /*
     * end_of_tilt flag not yet set
     */

    if (prev->tilt_num != current->tilt_num) {

      prev->end_of_tilt = TRUE;

      if ((Glob->summary_print && Glob->summary_interval <= 90)
	  || Glob->header_print)
	fprintf (stderr, "End of tilt - change in tilt num\n");

    } else {

      prev->end_of_tilt = FALSE;

    }

  } /* if (prev->end_of_tilt >= 0) */

  *end_of_vol = prev->end_of_volume;

}
