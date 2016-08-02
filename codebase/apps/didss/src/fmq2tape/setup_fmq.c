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
/*********************************************************
 * setup_fmq.c
 *
 * setup file message queue for reading
 *
 * Jaimi Yee, RAP, NCAR, Boulder, CO, USA
 *
 * May 1997
 *
 *********************************************************/

#include "fmq2tape.h"

void setup_fmq(void)
{

  Glob->fmq_handle = (FMQ_handle_t *)
     umalloc((ui32) sizeof(FMQ_handle_t));
  
  if (FMQ_init(Glob->fmq_handle, 
               Glob->params.fmq_path, 
               Glob->params.debug,
               Glob->prog_name) != 0) {
      
     fprintf(stderr, "ERROR: setup_fmq\n");
     fprintf(stderr, "Could not initialize FMQ handle\n");
     tidy_and_exit(-1);
     
  }
  
  Glob->fmq_handle_set = TRUE;

  if (FMQ_open_blocking(Glob->fmq_handle) != 0) {

    fprintf(stderr, "ERROR: setup_fmq\n");
    fprintf(stderr, "Could not open the FMQ\n");
    tidy_and_exit(-1);
    
  }

  if (FMQ_set_heartbeat(Glob->fmq_handle, PMU_auto_register)) {
    tidy_and_exit (-1);
  }

  FMQ_seek_end(Glob->fmq_handle);

  Glob->fmq_file_open = TRUE;
  
}
              
