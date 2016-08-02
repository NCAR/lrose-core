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
/*******************************************************************
 * fmq_output.c
 *
 * Routines writing a buffer to FMQ.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1997.
 ********************************************************************/

#include "ridds2mom.h"
#include <didss/ds_message.h>
using namespace std;


/********************************************************************
 * open_output_fmq()
 *
 * Initialize FMQ output.
 *
 */

int open_output_fmq(char *fmq_url, int buf_size, int nslots,
		    int compress, char *prog_name, int debug)

{

  Glob->radarQueue = new DsReformQueue;

  if ( Glob->radarQueue->init( fmq_url, prog_name, (bool)debug,           
                               DsFmq::READ_WRITE, DsFmq::END, 
                               (bool)compress, nslots, buf_size )) {
    fprintf(stderr, "ERROR - %s:open_output_fmq failed.\n", prog_name);
    return (-1);
  }

  return (0);
}


/******************************************************************
 * close_output_fmq()
 */

void close_output_fmq(void)

{

  if (Glob->radarQueue) {
    delete Glob->radarQueue;
  }

}
