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

#include "storm_track.h"

void tidy_and_exit(int sig)

{

  /*
   * If auto_restart, register so that procmap will have the latest time.
   * If not, unregister process.
   */

  if (Glob->shmem != NULL &&
      Glob->shmem->auto_restart) {
    PMU_force_register("In tidy_and_exit for auto_restart");
  } else {
    PMU_auto_unregister();
  }

  if (Glob->write_in_progress) {

    /*
     * if a write is in progress, let it finish - the file write
     * routines will call this routine when done
     */
    
    Glob->exit_signal = sig;
    return;

  }

  /*
   * check memory allocation
   */

  umalloc_map();
  umalloc_verify();
  
  /*
   * clear the storm_track_active semaphore
   */
  
  if (Glob->shmem != NULL) {
    if (usem_clear(Glob->sem_id, STORM_TRACK_ACTIVE_SEM)) {
      fprintf(stderr, "ERROR - %s:tidy_and_exit\n", Glob->prog_name);
      fprintf(stderr, "Clearing STORM_TRACK_ACTIVE sempahore\n");
    }
  }

  /*
   * remove track files on error exit
   */

  if (sig < 0) {
    remove_track_files();
    remove_current_state_file();
  }

  /*
   * remove lock file
   */

  remove_lock_file();

  /*
   * exit with code sig
   */

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "storm_track exiting, sig = %d\n", sig);
  }

  exit(sig);

}

