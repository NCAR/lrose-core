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
 * close_streams.c
 *
 * Close data streams.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * May 1997
 *
 **************************************************************************/

#include "ridds2mom.h"
using namespace std;

int close_streams(void)

{

  int iret = 0;

  switch (Glob->params.input_device) {

  case IN_TAPE:
    close_input_tape();
    break;

  case IN_UDP:
    close_input_udp();
    break;

  case IN_SHMEM:
    fprintf(stderr, "shared memory input not yet supported\n");
    iret = -1;
    break;

  } /* switch (Glob->params.input_device) */

  if (Glob->params.write_fmq_output) {
    close_output_fmq();
  }

  if (Glob->params.write_udp_output) {
    close_output_udp();
  }

  if (Glob->params.write_shmem_output) {
    delete_output_shmem();
  }
  
  if (Glob->params.write_archive_fmq) {
    close_archive_fmq();
  }

  if (Glob->params.output_format == DS_FORMAT) {
    delete Glob->radarMsg;
  }

  return (iret);
  
}

