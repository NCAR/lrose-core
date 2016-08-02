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
 * read_tape_lass.c
 *
 * reads the lassen data from the disk
 * If first_call then the SunriseSummary and the Volume Summary
 *  are read into static data
 * Also reads the current ray data into the buffer for every call.
 * If eof then rewinds the file pointer to the satrt and sets the
 * first_call to true.
 * returns the size of the logical record
 *
 * Mike Dixon/Gary Blackburn RAP NCAR September 1990
 *
 **************************************************************************/

#include "polar2gate.h"

si32 read_disk_lass (ui08 *beam_buffer)

{

  static int first_call = TRUE;
  static char *buffer;

  static si32 offset = 0;   /* Offset of a logical record */ 
  static si32 left = 0;	    /* Number of logical records left in buffer */
  static si32 nread = 0;
  
  si32 log_rec_size;        /* size of logical record */

  /*
   * initialize buffer
   */

  if (first_call) {

    buffer = (char *) umalloc(MAX_TAPE_REC_SIZE);
    first_call = FALSE;

  } /* if (first_call) */
  
  if (left == 0) {

    nread = read_lass_record(buffer);
    offset = 0;
    left = 0;

  }
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

