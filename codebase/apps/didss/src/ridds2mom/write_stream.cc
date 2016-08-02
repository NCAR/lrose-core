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
 * write_stream.c
 *
 * Write the output stream.
 *
 * Buffer copy is made to avoid side effects from byte_swapping
 * in the output modules.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * May 1997
 *
 **************************************************************************/

#include "ridds2mom.h"
#include <didss/ds_message.h>
#include <rapformats/swap.h>
using namespace std;

static ui08 *alloc_buffer_copy(int len);

int write_stream(ui08 *write_buffer, int nwrite)

{
  
  static int count = 0;
  ui08 *buffer_copy;
  int iret;
  ll_params_t *ll_params;

  if (Glob->params.write_fmq_output) {

    buffer_copy = alloc_buffer_copy(nwrite);
    memcpy(buffer_copy, write_buffer, nwrite);

    if (Glob->params.output_format == LL_FORMAT) {
      ll_params = (ll_params_t *) buffer_copy;
      BE_from_ll_params(ll_params);
      if (Glob->radarQueue->putLLBeam( buffer_copy, nwrite )) {
        return (-1);
      }

    } else if (Glob->params.output_format == DS_FORMAT) {
      if (Glob->radarQueue->putDsBeam( buffer_copy, nwrite )) {
        return (-1);
      }

    } else if (Glob->params.output_format == RIDDS_FORMAT) {
      fprintf(stderr, "ERROR - RIDDS_FORMAT FMQ output not supported\n");
      return (-1);
    }
  }

  if (Glob->params.write_udp_output) {

    buffer_copy = alloc_buffer_copy(nwrite);
    memcpy(buffer_copy, write_buffer, nwrite);

    if (Glob->params.output_format == LL_FORMAT) {
      ll_params = (ll_params_t *) buffer_copy;
      BE_from_ll_params(ll_params);
      if (write_output_ncar_udp(buffer_copy, nwrite)) {
	return (-1);
      }

    } else if (Glob->params.output_format == DS_FORMAT) {
      fprintf(stderr, "ERROR - DS_FORMAT UDP output not supported\n");
      return (-1);

    } else if (Glob->params.output_format == RIDDS_FORMAT) {
      if (write_output_ridds_udp(buffer_copy, nwrite)) {
	return (-1);
      }
    }
  }

  if (Glob->params.write_shmem_output) {

    buffer_copy = alloc_buffer_copy(nwrite);
    memcpy(buffer_copy, write_buffer, nwrite);

    if (Glob->params.output_format == LL_FORMAT) {
      ll_params = (ll_params_t *) buffer_copy;
      iret = write_output_ll_shmem(2, buffer_copy, nwrite);
      count++;
      if (count == 360) {
        count = 0;
      }
      if (iret < 4) {
        if (Glob->params.debug >= DEBUG_VERBOSE && (count == 0)) {
  	  if (iret == 3) {
	    fprintf(stderr, "\nwrite_stream - output buffer is full\n");
	  } else {
	    fprintf(stderr, "\nwrite_stream - client delay\n");
	  }
        }
        return (-1);
      } /* if (iret < 4) */

    } else if (Glob->params.output_format == DS_FORMAT) {
      fprintf(stderr, "ERROR - DS_FORMAT SHMEM output not supported\n");
      return (-1);

    } else if (Glob->params.output_format == RIDDS_FORMAT) {
      fprintf(stderr, "ERROR - RIDDS_FORMAT SHMEM output not supported\n");
      return (-1);

    }
  }

  return (0);
    
}

static ui08 *alloc_buffer_copy(int nwrite)

{

  static ui08 *buffer_copy = NULL;
  static int nalloc = 0;

  if (nwrite > nalloc) {
    if (buffer_copy == NULL) {
      buffer_copy = (ui08*)umalloc(nwrite);
    } else {
      buffer_copy = (ui08*)urealloc(buffer_copy, nwrite);
    }
    nalloc = nwrite;
  }

  return (buffer_copy);

}

