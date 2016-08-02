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
 * setup_shmem.c
 *
 * sets up the shared memory and semaphores for communicating
 * With the storm_ident
 *
 * RAP, NCAR, Boulder CO
 *
 * january 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_track.h"

void setup_shmem(void)

{

  /*
   * get the semaphore set
   */

  if(Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "%s: Trying to get semaphores\n", Glob->prog_name);

  if ((Glob->sem_id = usem_get(Glob->params.shmem_key,
			       N_STORM_IDENT_SEMS)) < 0) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot get semaphore set, key = %x\n",
	    (unsigned) Glob->params.shmem_key);
    tidy_and_exit(-1);
  }
  
  if(Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "%s: Got sempahore set.\n", Glob->prog_name);

  /*
   * get shared memory
   */

  if(Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "%s: Trying to get shared memory\n", Glob->prog_name);

  if ((Glob->shmem = (storm_tracking_shmem_t *)
       ushm_get(Glob->params.shmem_key + 1,
		sizeof(storm_tracking_shmem_t))) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot get shared memory header, key = %x\n",
	    (unsigned) (Glob->params.shmem_key + 1));
    tidy_and_exit(-1);
  }
  
  if(Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "%s: Got shmem\n", Glob->prog_name);

}


