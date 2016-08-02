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
 * tidy_and_exit.c
 *
 * tidies up shared memory and quits
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1990
 *
 ****************************************************************************/

#include "polar_ingest.h"

void tidy_and_exit(int sig)

{

  if (Glob->debug) {
    fprintf(stderr, "polar_ingest quitting\n");
  }

  /*
   * unregister process
   */

  PMU_auto_unregister();

  if (Glob->semaphores_attached) {

    /*
     * set the quit semaphore to let clients know to quit
     */

    if (usem_set(Glob->sem_id, POLAR_INGEST_QUIT_SEM) != 0) {
      fprintf(stderr, "ERROR - %s.\n", Glob->prog_name);
      fprintf(stderr, "Setting quit semaphore");
    }

    /*
     * remove the semaphore set
     */

    if (usem_remove(Glob->shmem_key) != 0) {
      fprintf(stderr, "WARNING - %s:tidy_and_exit\n", Glob->prog_name);
      fprintf(stderr, "Cannot remove sempahore, key = %x.\n",
	      Glob->shmem_key + 1);
    }

  } /* if (Glob->semaphores_attached) */

  /*
   * remove shared memory header if attached
   */

  if (Glob->shmem_header_attached) {
    if (ushm_remove(Glob->shmem_key + 1) != 0) {
      fprintf(stderr, "WARNING - %s:tidy_and_exit\n", Glob->prog_name);
      fprintf(stderr, "Cannot remove shared memory for header, key = %x.\n",
	      Glob->shmem_key + 1);
    }
  }

  /*
   * remove shared memory buffer if attached
   */

  if (Glob->shmem_buffer_attached) {
    if (ushm_remove(Glob->shmem_key + 2) != 0) {
      fprintf(stderr, "WARNING - %s:tidy_and_exit\n", Glob->prog_name);
      fprintf(stderr, "Cannot remove shared memory for buffer, key = %x.\n",
	      Glob->shmem_key + 2);
    }
  }

  /*
   * close rdi message queue
   */

  close_rdi_mmq();

  /*
   * exit with code sig
   */

  exit(sig);

}

