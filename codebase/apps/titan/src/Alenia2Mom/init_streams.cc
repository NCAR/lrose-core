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
 * init_streams.c
 *
 * Setup data streams.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * May 1997
 *
 **************************************************************************/

#include "Alenia2Mom.h"
using namespace std;

int init_streams(void)

{

  int iret = 0;

  /*
   * initialize input device
   */

  switch (Glob->params.input_device) {

  case IN_TAPE:
    if (open_input_tape(Glob->params.input_tape_name,
			Glob->params.input_tape_wait,
			Glob->params.debug)) {
      iret = -1;
    }
    break;

  case IN_UDP:
    if (open_input_udp(Glob->params.input_udp_port,
		       Glob->params.debug)) {
      iret = -1;
    }
    break;

  } /* switch (Glob->params.input_device) */

  /*
   * initialize output device(s)
   */

  if (Glob->params.write_fmq_output) {
    if (open_output_fmq(Glob->params.output_fmq_path,
			Glob->params.output_fmq_size,
			Glob->params.output_fmq_nslots,
			Glob->params.output_fmq_compress,
			Glob->prog_name,
			Glob->params.debug)) {
      iret = -1;
    }
  }

  if (Glob->params.write_udp_output) {
    if (open_output_udp(Glob->params.output_udp_address,
			Glob->params.output_udp_port,
			Glob->params.debug)) {
      iret = -1;
    }
  }

  /*
   * initialize archive output
   */

  if (Glob->params.write_archive_fmq) {

    if (open_archive_fmq(Glob->params.archive_fmq_path,
			 Glob->params.archive_fmq_size,
			 Glob->params.archive_fmq_nslots,
			 Glob->params.archive_fmq_compress,
			 Glob->prog_name,
			 Glob->params.debug)) {
      iret = -1;
    }

  }

  /*
   * initialize reformatter module
   */
  
  if (Glob->params.output_format == LL_FORMAT) {

    init_reformat2ll(Glob->params.radar_name,
		     Glob->params.site_name,
		     Glob->params.radar_location.latitude,
		     Glob->params.radar_location.longitude,
		     Glob->params.radar_location.altitude,
		     Glob->params.polarization_code,
		     Glob->params.beam_width,
		     Glob->params.avg_xmit_pwr,
		     Glob->params.wavelength,
		     Glob->params.noise_dbz_at_100km,
		     Glob->params.debug);

  } else if (Glob->params.output_format == DS_FORMAT) {

    init_reformat2ds(Glob->params.radar_name,
		     Glob->params.site_name,
		     Glob->params.radar_location.latitude,
		     Glob->params.radar_location.longitude,
		     Glob->params.radar_location.altitude,
		     Glob->params.polarization_code,
		     Glob->params.beam_width,
		     Glob->params.avg_xmit_pwr,
		     Glob->params.wavelength,
		     Glob->params.noise_dbz_at_100km,
		     Glob->params.debug);

  }

  return (iret);
  
}

