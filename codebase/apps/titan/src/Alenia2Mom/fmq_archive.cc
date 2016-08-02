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
 * fmq_archive.c
 *
 * Routines writing a buffer to FMQ.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1997.
 ********************************************************************/

#include "Alenia2Mom.h"
#include <dataport/bigend.h>
#include <toolsa/fmq.h>
#include <didss/ds_message.h>
#include <rapformats/swap.h>
using namespace std;

static FMQ_handle_t Fmq;
static int Debug;
static int Init_done = FALSE;

/********************************************************************
 * open_archive_fmq()
 *
 * Initialize FMQ archive.
 *
 */

int open_archive_fmq(char *fmq_path, int buf_size, int nslots,
		     int compress, char *prog_name, int debug)

{

  Debug = debug;
  
  if (FMQ_init(&Fmq, fmq_path, debug, prog_name)) {
    fprintf(stderr, "ERROR - %s:do_write_test\n", prog_name);
    fprintf(stderr, "FMQ_init failed.\n");
    return (-1);
  }

  if (FMQ_set_heartbeat(&Fmq, PMU_auto_register)) {
    return (-1);
  }
  
  if (FMQ_open_rdwr(&Fmq, nslots, buf_size)) {
    fprintf(stderr, "FMQ_open_rdwr failed.\n");
    return (-1);
  }
  
  if (compress) {
    FMQ_set_compress(&Fmq);
  }

  Init_done = TRUE;
  
  return (0);

}

/*------------------------------------------------------------------------*/

int write_archive_fmq(ui08 *buffer, int buflen)

{

  /*
   * this routine has not yet been specified
   *
   * suppress compiler warnings
   */

  buffer = NULL;
  buflen = 0;

  return (0);
  
}

/*------------------------------------------------------------------------*/

/*
 * write an End-of-volume message to the FMQ.
 */

int write_archive_eov(void)

{

  int i = -1;
  
  if (Init_done) {
    if (FMQ_write(&Fmq, &i, sizeof(i),
		  DS_MESSAGE_TYPE_END_OF_VOLUME, 0)) {
      fprintf(stderr, "FMQ_write failed\n");
      return (-1);
    } else {
      return (0);
    }
  } else {
    fprintf(stderr, "ERROR - write_archive_eof - init not done\n");
    return (-1);
  }
  
  
}

/******************************************************************
 * close_archive_fmq()
 */

void close_archive_fmq(void)

{

  if (Init_done) {
    FMQ_free(&Fmq);
  }

}

