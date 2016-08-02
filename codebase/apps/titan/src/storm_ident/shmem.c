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
/*********************************************************************
 * setup_shmem()
 *
 * sets up the shared memory and semaphores for communicating
 * with the tracking program
 *
 * RAP, NCAR, Boulder CO
 *
 * january 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_ident.h"
#define SEM_CHECK_PERIOD 15

void create_shmem(void)

{

  /*
   * create the semaphore set
   */
  
  if(Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "%s: Trying to create semaphores\n", Glob->prog_name);

  if ((Glob->sem_id = usem_create(Glob->params.shmem_key,
				  N_STORM_IDENT_SEMS,
				  S_PERMISSIONS)) < 0) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot create semaphore set, key = %x\n",
	    (unsigned) Glob->params.shmem_key);
    tidy_and_exit(-1);
  }
  
  if(Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "%s: created semaphores\n", Glob->prog_name);
  }

  Glob->sems_available = TRUE;

  /*
   * initialize semaphores
   */

  if (usem_clear(Glob->sem_id, STORM_TRACK_ACTIVE_SEM)) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr, "Clearing STORM_TRACK_ACTIVE_SEM\n");
    tidy_and_exit(-1);
  }

  if (usem_clear(Glob->sem_id, STORM_IDENT_QUIT_SEM)) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr, "Clearing STORM_IDENT_QUIT_SEM\n");
    tidy_and_exit(-1);
  }

  /*
   * create shared memory
   */

  if(Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "%s: Trying to create shared memory\n", Glob->prog_name);
  }
  
  if ((Glob->shmem = (storm_tracking_shmem_t *)
       ushm_create(Glob->params.shmem_key + 1,
		   sizeof(storm_tracking_shmem_t),
		   S_PERMISSIONS)) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot create shared memory header, key = %x\n",
	    (unsigned) (Glob->params.shmem_key + 1));
    tidy_and_exit(-1);
  }

  strcpy(Glob->shmem->storm_data_dir,
	 Glob->params.storm_data_dir);
  
  Glob->shmem_available = TRUE;

  if(Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "%s: got shmem\n", Glob->prog_name);
  }

}

void init_shmem(char *header_file_path)

{

  /*
   * initialize shared mem
   */

  strcpy(Glob->shmem->storm_header_file_path,
	 header_file_path);

}

/*********************************************************************
 * perform_tracking()
 *
 *********************************************************************/

void perform_tracking(int tracking_mode)

{

  /*
   * make sure prints are flushed to keep them in order
   * with storm_track
   */

  fflush(stdout);

  Glob->shmem->tracking_mode = tracking_mode;
  
  /*
   * set the storm_track active semaphore to indicate to
   * storm_track that it may go ahead
   */
  
  if (usem_set(Glob->sem_id, STORM_TRACK_ACTIVE_SEM) != 0) {
    fprintf(stderr, "ERROR - %s:props_compute\n", Glob->prog_name);
    fprintf(stderr, "Setting STORM_TRACK_ACTIVE_SEM sempahore\n");
    tidy_and_exit(-1);
  }
  
  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "STORM_TRACK_ACTIVE_SEM set\n");
  }
    
  /*
   * wait for STORM_TRACK_ACTIVE_SEM to clear
   * This is done on a non-blocking loop so that
   * PMU_register may be called
   */
  
  while (usem_check(Glob->sem_id, STORM_TRACK_ACTIVE_SEM)) {
    PMU_auto_register("Waiting for storm_track");
    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf(stderr, "In perform_tracking - waiting for "
	      "STORM_TRACK_ACTIVE_SEM to clear\n");
    }
    sleep(1);
  }
  
  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "%s:perform_tracking.c - tracking complete\n",
	    Glob->prog_name);
  }
  
  fflush(stdout);

  return;

}
