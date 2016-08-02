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
 * tidies up shared memory and quits
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1991
 *
 ****************************************************************************/

#include "storm_ident.h"

void tidy_and_exit(int sig)

{

  char call_str[BUFSIZ];
  int iarg;

  /*
   * check malloc
   */

  umalloc_verify();
  umalloc_map();

  /*
   * free the file indices
   */

  free_indices();

  if (Glob->sems_available) {

    /*
     * set the flag for removing the track file as appropriate
     */

    if (Glob->shmem_available) {
      if (Glob->params.remove_old_files_on_restart) {
	Glob->shmem->remove_track_file = TRUE;
      } else {
	Glob->shmem->remove_track_file = FALSE;
      }
      if (sig == EXIT_AND_RESTART) {
	Glob->shmem->auto_restart = TRUE;
      } else {
	Glob->shmem->auto_restart = FALSE;
      }
    }

    /*
     * set the quit semaphore, then wait for it to be 
     * cleared by storm_track
     */

    if (usem_set(Glob->sem_id, STORM_IDENT_QUIT_SEM) != 0) {
      fprintf(stderr, "ERROR - %s:tidy_and_exit.\n", Glob->prog_name);
      fprintf(stderr, "Setting STORM_IDENT_QUIT_SEM sempahore\n");
    }

    /*
     * set the storm_track active semaphore to indicate to
     * storm_track that it may go ahead and quit
     */
    
    if (usem_set(Glob->sem_id, STORM_TRACK_ACTIVE_SEM) != 0) {
      fprintf(stderr, "ERROR - %s:tidy_and_exit\n", Glob->prog_name);
      fprintf(stderr, "Setting STORM_TRACK_ACTIVE_SEM sempahore\n");
    }

    /*
     * wait for storm_track to clear semaphore
     */

    if (usem_test(Glob->sem_id, STORM_TRACK_ACTIVE_SEM) != 0) {
      fprintf(stderr, "ERROR - %s:tidy_and_exit\n", Glob->prog_name);
      fprintf(stderr, "Testing STORM_TRACK_ACTIVE_SEM sempahore\n");
      fprintf(stderr, "Waiting for storm_track to die\n");
    }

    /*
     * remove the semaphore
     */

    if ((sig != EXIT_AND_RESTART) &&
	usem_remove(Glob->params.shmem_key) != 0) {
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stderr, "WARNING - %s:tidy_and_exit\n", Glob->prog_name);
	fprintf(stderr, "Cannot remove sempahore set, key = %x.\n",
		(unsigned) Glob->params.shmem_key);
      }
    }
    
  } /* if (Glob->params.mode ... ) */
  
  if (sig != EXIT_AND_RESTART && Glob->shmem_available) {

    /*
     * remove shared memory
     */
    
    if (ushm_remove(Glob->params.shmem_key + 1) != 0) {
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stderr, "WARNING - %s:tidy_and_exit\n", Glob->prog_name);
	fprintf(stderr,
		"Cannot remove shared memory, key = %x.\n",
		(unsigned) (Glob->params.shmem_key + 1));
      }
    }

  } /* if (sig != EXIT_AND_RESTART ... ) */

  /*
   * If auto_restart, register so that procmap will have the latest time.
   * If not, unregister process.
   */

  if (sig == EXIT_AND_RESTART) {
    PMU_force_register("In tidy_and_exit for auto_restart");
  } else {
    PMU_auto_unregister();
  }

  /*
   * remove lock file
   */

  remove_lock_file();

  /*
   * if a restart is required, load up string with the original command
   * line and call system to restart this program
   */

  if (sig == EXIT_AND_RESTART) {

    memset ((void *) call_str,
            (int) 0, (size_t) BUFSIZ);

    for (iarg = 0; iarg < Glob->argc; iarg++) {
      ustr_concat(call_str, Glob->argv[iarg], BUFSIZ);
      ustr_concat(call_str, " ", BUFSIZ);
    }

    strcat(call_str, " & ");

    system(call_str);

  } /* if (sig == EXIT_AND_RESTART) */

  /*
   * exit with code sig
   */

  exit(sig);

}

