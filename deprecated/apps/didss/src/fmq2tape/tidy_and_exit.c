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
 * tidy_and_exit.c
 *
 * tidies up fmq and quits
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1990
 *
 ****************************************************************************/

#include "fmq2tape.h"

static int Tape_id = -1;

void save_tape_id(int id)

{
  Tape_id = id;
}

void tidy_and_exit(int sig)

{

  /*
   * unregister process
   */

  PMU_auto_unregister();

  if (sig == SIGUSR1)
  {
     fprintf(stderr,"rewinding and ejecting tape\n");
     TTAPE_rewoffl(Tape_id);
  }

  if (Tape_id >= 0) {
    close(Tape_id);
  }

  if (Glob->params.debug) {
    fprintf(stderr, "fmq2tape quitting\n");
  }

  if (Glob->fmq_file_open) {
     
      if (FMQ_close(Glob->fmq_handle) != 0) {
	 fprintf(stderr, "ERROR: tidy_and_exit\n");
	 fprintf(stderr, "Cannot close FMQ\n");
      }
  }
  
  if (Glob->fmq_handle_set) {
    
      if (FMQ_free(Glob->fmq_handle) != 0) {
	 fprintf(stderr, "ERROR: tidy_and_exit\n");
	 fprintf(stderr, "Cannot free FMQ handle\n");
      }
      
  }

   
  /*
   * exit with code sig
   */

  exit(sig);

}

