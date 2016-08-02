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
 * read_fmq_beam.c
 *
 * reads a logical record from FMQ
 *
 * returns the size of the logical record
 *
 **************************************************************************/

#include "polar_ingest.h"
#include <toolsa/fmq.h>
#include <toolsa/pmu.h>
#include <rapformats/lincoln.h>
#include <rapformats/ds_radar.h>

si32 read_fmq_beam (char **beam_buffer)

{

  static int first_call = TRUE;
  static FMQ_handle_t fmq;
  ui08 *buf;

  if (first_call) {

    /*
     * initialize FMQ
     */

    if (FMQ_init(&fmq, Glob->fmq_path,
		 Glob->debug,
		 Glob->prog_name)) {
      fprintf(stderr, "FMQ_init failed.\n");
      return (-1);
    }

    if (FMQ_set_heartbeat(&fmq, PMU_auto_register)) {
      return (-1);
    }

    if (FMQ_open_blocking(&fmq)) {
      fprintf(stderr, "FMQ_open_blocking failed.\n");
      return (-1);
    }

    if (Glob->fmq_seek_to_end) {
      if (FMQ_seek_end(&fmq)) {
	fprintf(stderr, "FMQ_seek_end failed.\n");
	return (-1);
      }
    }

    first_call = FALSE;

  }

  /*
   * read a beam
   */
  
  if (FMQ_read_type_blocking(&fmq, 1000, DS_MESSAGE_TYPE_LL_BEAM)) {
    return (-1);
  }

  /*
   * wait a given time
   */

  if (Glob->fmq_read_wait > 0)
    uusleep(Glob->fmq_read_wait);

  /*
   * swap as required
   */
  
  buf = (ui08 *) FMQ_msg(&fmq);
  BE_to_ll_params((ll_params_t *) buf);

  /*
   * set buffer pointer, return number of bytes read
   */
  
  *beam_buffer = (char *) buf;
  return (FMQ_msg_len(&fmq));

}

