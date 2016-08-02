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
 * tidies up and quits
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 ****************************************************************************/

#include <signal.h>
#include "time_hist.h"

void tidy_and_exit(int sig)

{

  if (Glob->debug) {
    fprintf(stderr, "** tidy_and_exit **\n");
  }

  /*
   * unregister process
   */

  PMU_auto_unregister();

  /*
   * clean up system call environment
   */

  usystem_call_clean();

  /*
   * check the mallocs
   */

  umalloc_map();
  umalloc_verify();

  /*
   * hang up the track server
   */

  tserver_clear(&Glob->tdata_index);
  
  /*
   * free X resources unless called by xerror_handler
   */

  if (sig != -1)
    free_resources();

  /*
   * clear time_hist active flag
   */

  if (Glob->track_shmem) {
    Glob->track_shmem->time_hist_active = FALSE;
  }

  exit(sig);

}

