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
 * read_disk_alenia.c
 *
 * reads the alenia data from the disk
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * October 1995
 *
 **************************************************************************/

#include "polar2gate.h"

si32 read_disk_alenia (ui08 *beam_buffer)

{

  static int first_call = TRUE;
  static char *buffer;

  si32 nread = 0;
  si32 log_rec_size;

  /*
   * initialize buffer
   */

  if (first_call) {

    buffer = (char *) umalloc(MAX_TAPE_REC_SIZE);
    first_call = FALSE;

  } /* if (first_call) */
  
  nread = read_alenia_record(buffer);
  
  if (nread < 0) {

    /*
     * read error or end of tape
     */

    log_rec_size = 0;

  } 
  else {

    log_rec_size = nread;
    memcpy ((void *) beam_buffer,
            (void *) buffer,
            (size_t) log_rec_size);

  }

  /*
   * wait a given time
   */

  if (Glob->device_read_wait > 0)
    uusleep((ui32) Glob->device_read_wait);

  /*
   * return record size
   */

  return log_rec_size;

}



