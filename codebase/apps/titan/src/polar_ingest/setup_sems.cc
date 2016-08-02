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
/*****************************************************************************
 * setup_sems.c
 *
 * Set up the semaphores
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * April 1996
 *
 ****************************************************************************/

#include "polar_ingest.h"

void setup_sems(void)

{

  /*
   * create the semaphore set for communicating with clients
   */

  if ((Glob->sem_id = usem_create(Glob->shmem_key,
				  N_POLAR_INGEST_SEMS,
				  S_PERMISSIONS)) < 0) {
    fprintf(stderr, "Sem_id = %d\n", Glob->sem_id);

    fprintf(stderr, "ERROR - %s:setup_sems.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot create semaphore set, key = %x\n",
	    Glob->shmem_key);
    tidy_and_exit(-1);
  }

  Glob->semaphores_attached = TRUE;

  /*
   * clear quit and polar2mdv active semaphores
   */

  if (usem_clear(Glob->sem_id, POLAR_INGEST_QUIT_SEM) != 0) {
    fprintf(stderr, "ERROR - %s:setup_sems.\n", Glob->prog_name);
    fprintf(stderr, "Clearing polar_ingest quit semaphore");
    tidy_and_exit(-1);
  }
  
  if (usem_clear(Glob->sem_id, POLAR2MDV_QUIT_SEM) != 0) {
    fprintf(stderr, "ERROR - %s:setup_sems.\n", Glob->prog_name);
    fprintf(stderr, "Clearing polar2mdv quit semaphore");
    tidy_and_exit(-1);
  }
  
  if (usem_clear(Glob->sem_id, POLAR2MDV_ACTIVE_SEM) != 0) {
    fprintf(stderr, "ERROR - %s:setup_sems.\n", Glob->prog_name);
    fprintf(stderr, "Clearing polar2mdv active semaphore");
    tidy_and_exit(-1);
  }
  
  /*
   * set not_ready sem
   */

  if (usem_set(Glob->sem_id, POLAR_INGEST_NOT_READY_SEM) != 0) {
    fprintf(stderr, "ERROR - %s:setup_sems.\n", Glob->prog_name);
    fprintf(stderr, "Setting not-ready semaphore");
    tidy_and_exit(-1);
  }

  /*
   * start polar2mdv
   */

  if (Glob->start_polar2mdv) {
    system(Glob->polar2mdv_command_line);
  }

  return;
  
}
