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
 * tidies up shared and quits
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1991
 *
 ****************************************************************************/

#include "polar2mdv.h"

void tidy_and_exit(int sig)

{

  int sem_id = get_sem_id();

  /*
   * unregister process
   */

  PMU_auto_unregister();

  /*
   * clean up the system call environment
   */

  usystem_call_clean();

  /*
   * clear the active sem
   */

  if (usem_clear(sem_id, POLAR2MDV_ACTIVE_SEM) != 0) {
    fprintf(stderr, "ERROR - %s:tidy_and_exit.\n", Glob->prog_name);
    fprintf(stderr, "Clearing active semaphore\n");
  }

  /*
   * If polar_ingest has not set its quit semaphore,
   * set the quit semaphore so that polar_ingest will know to quit
   */

  if (Glob->debug)
    fprintf(stderr, "polar2mdv quitting\n");

  if (!usem_check(sem_id, POLAR_INGEST_QUIT_SEM)) {

    if (usem_set(sem_id, POLAR2MDV_QUIT_SEM) != 0) {
      fprintf(stderr, "ERROR - %s:tidy_and_exit\n", Glob->prog_name);
      fprintf(stderr, "Setting quit semaphore.\n");
    }

  }

  /*
   * exit with code sig
   */

  exit(sig);

}

