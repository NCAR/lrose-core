/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/***************************************************************************
 * send_fmq_to_tape.c
 *
 * sends the fmq messages to tape.
 *
 * Jaimi Yee
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * June 1997
 *
 ****************************************************************************/

#include "fmq2tape.h"
#include <rapformats/ds_radar.h>

void send_fmq_to_tape(void)
{
   MEMbuf *buffer;
   int tape_id;
   int forever = 1;
   char *msg;
   int msg_len;
   int stored_len;
   int space_avail;
   int nwrite;
   int count = 0;
     
   tape_id = open(Glob->params.tape_device_name, O_WRONLY);
   if (tape_id == -1)
   {
      fprintf(stderr,"ERROR: fmq2tape\n");
      perror(Glob->params.tape_device_name);
      tidy_and_exit(-1);
   }

   save_tape_id(tape_id);
   TTAPE_set_var(tape_id);
   
   buffer = MEMbufCreate();
   MEMbufReset(buffer);
   
   while(forever)
   {
      PMU_auto_register("Getting a message from the FMQ");
      
      if (FMQ_read_blocking(Glob->fmq_handle, 100))
      {
         fprintf(stderr, "ERROR: fmq2tape\n");
	 fprintf(stderr, "FMQ_read failed.\n");
	 sleep (1);
      }
      else
      { 
	 msg = (char *)FMQ_msg(Glob->fmq_handle);
    
         if (FMQ_msg_type(Glob->fmq_handle) == DS_MESSAGE_TYPE_END_OF_VOLUME)
	 {
	    TTAPE_write_eof(tape_id, 1);
            if (Glob->params.debug)
	      fprintf(stderr, "->>>>>>>>>> writing eof marker to tape\n");
         }
         else if (FMQ_msg_type(Glob->fmq_handle) ==
		  DS_MESSAGE_TYPE_RIDDS_BEAM )
	 {
        
	     msg_len = FMQ_msg_len(Glob->fmq_handle);

	     if(msg_len > Glob->params.max_block_size)
	     {
		fprintf(stderr,"WARNING: fmq2tape\n");
		fprintf(stderr,"Message too large to write to tape.\n");
		continue;
	     }

	     count++;

	     stored_len = MEMbufLen(buffer);
	     space_avail = Glob->params.max_block_size - stored_len;
	     if((space_avail < msg_len) || 
		(count > Glob->params.max_message_per_block))
	     {
		nwrite = write(tape_id, MEMbufPtr(buffer), stored_len);
		if(nwrite != stored_len)
		{
		   fprintf(stderr,"ERROR: fmq2tape\n");
		   perror(Glob->params.tape_device_name);
		   fprintf(stderr,"rewinding and ejecting tape\n");
		   TTAPE_rewoffl(tape_id);
		   tidy_and_exit(-1);
		}
		if(Glob->params.debug)
		   fprintf(stderr, "wrote block of size %d to tape\n",
			   stored_len);
		MEMbufReset(buffer);
		count = 1;
	     }

             if (Glob->params.debug)
		fprintf(stderr, "read in message of length %d\n", msg_len);

	     MEMbufAdd(buffer, (void *)msg, (size_t)msg_len);
	 }
	 
      }
      
   }
   
	 
}


