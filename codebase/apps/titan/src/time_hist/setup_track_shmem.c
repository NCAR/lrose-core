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
 * setup_track_shmem.c
 *
 * Sets up the shared memory to communicate with the main display
 * program for track data purposes.
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "time_hist.h"

void setup_track_shmem(void)

{

  if (Glob->debug) {
    fprintf(stderr, "** setup_track_shmem **\n");
  }

  /*
   * get shared memory - will block till available
   */

  if ((Glob->coord_export = (coord_export_t *)
       ushm_get(Glob->track_shmem_key,
		sizeof(coord_export_t))) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_track_shmem.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot create shared memory for coord_export, key = %d\n",
	    Glob->track_shmem_key);
    tidy_and_exit(-1);
  }
  
  if ((Glob->track_shmem = (time_hist_shmem_t *)
       ushm_get(Glob->track_shmem_key + 1,
		sizeof(time_hist_shmem_t))) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_track_shmem.\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot get shared memory for track data, key = %d\n",
	    Glob->track_shmem_key + 1);
    tidy_and_exit(-1);
  }

  /*
   * initialize partial track params
   */
  
  Glob->track_shmem->partial_track_past_period =
    Glob->partial_track_past_period;
  Glob->track_shmem->partial_track_future_period =
    Glob->partial_track_future_period;

}
