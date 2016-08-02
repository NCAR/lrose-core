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

#include "ridds2mom.h"
#include <dataport/bigend.h>
#include <didss/ds_message.h>
#include <rapformats/swap.h>
using namespace std;

static int Init_done = FALSE;

/********************************************************************
 * open_archive_fmq()
 *
 * Initialize FMQ archive.
 *
 */

int open_archive_fmq(char *fmq_url, int buf_size, int nslots,
		     int compress, char *prog_name, int debug)

{

  Glob->archiveQueue = new DsReformQueue;

  if ( Glob->archiveQueue->init( fmq_url, prog_name, (bool)debug,
                                 DsFmq::READ_WRITE, DsFmq::END, 
                                 (bool)compress, nslots, buf_size )) {
    fprintf(stderr, "ERROR - %s:open_archive_fmq failed.\n", prog_name);
    return (-1);
  }

  Init_done = TRUE;
  return (0);
}

/*------------------------------------------------------------------------*/

int write_archive_fmq(ui08 *buffer, int buflen)

{

  static int seq_num = 0;

  int ncopy;
  int msg_hdr_offset;
  int data_offset;
  
  ui08 out[NEX_PACKET_SIZE];
  ui08 *data;

  RIDDS_data_hdr data_hdr;
  RIDDS_ctm_info *ctm_info;
  RIDDS_msg_hdr *msg_hdr;

  if (Init_done) {
    
    /*
     * make copy of header
     */
    
    data_hdr = *((RIDDS_data_hdr *) buffer);
    BE_to_RIDDS_data_hdr(&data_hdr);
    
    /*
     * set pointers
     */
    
    ctm_info = (RIDDS_ctm_info *) out;
    msg_hdr_offset = sizeof(RIDDS_ctm_info);
    msg_hdr = (RIDDS_msg_hdr *) (out + msg_hdr_offset);
    data_offset = msg_hdr_offset + sizeof(RIDDS_msg_hdr);
    data = out + data_offset;
  
    /*
     * load headers
     */

    memset(out, 0, NEX_PACKET_SIZE);
    msg_hdr->message_len =
      BE_from_si16((NEX_PACKET_SIZE - sizeof(RIDDS_ctm_info)) / 2);
    msg_hdr->message_type = DIGITAL_RADAR_DATA;
    msg_hdr->seq_num = BE_from_si16(++seq_num);
    msg_hdr->julian_date =
      BE_from_si16(data_hdr.julian_date);
    msg_hdr->millisecs_past_midnight =
      BE_from_si32(data_hdr.millisecs_past_midnight);
    msg_hdr->num_message_segs = BE_from_si16(1);
    msg_hdr->message_seg_num = BE_from_si16(1);

    /*
     * load data
     */

    ncopy = MIN(buflen, NEX_PACKET_SIZE - data_offset);
    memcpy(data, buffer, ncopy);
    
    return( Glob->archiveQueue->putRiddsBeam( out, NEX_PACKET_SIZE ));

  } else {

    fprintf(stderr, "ERROR - write_archive_fmq - init not done\n");
    return (-1);

  }
  
}

/******************************************************************
 * close_archive_fmq()
 */

void close_archive_fmq(void)

{

  if (Glob->archiveQueue) {
    delete Glob->archiveQueue;
  }

}

