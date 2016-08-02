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
 * storm_track.c
 *
 * Tracks storms in a storm file.
 *
 * This program may be run on a storm file only, or is invoked by
 * storm_ident to run concurrently, in which case communication is done
 * via semaphores and shared memory.
 *
 * If required, the track data is written to a client process via a
 * socket connection.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1991
 *
 ****************************************************************************/

#define MAIN
#include "storm_track.h"
#undef MAIN

#include <time.h>

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char **file_paths;
  char *params_file_path = NULL;
  int forever = TRUE;
  int check_params;
  int print_params;
  si32 n_files = 0;
  si32 ifile;
  path_parts_t progname_parts;
  storm_file_handle_t s_handle;
  track_file_handle_t t_handle;
  tdrp_override_t override;
  
  /*
   * register function to trap termination and interrupts
   */

  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGKILL, tidy_and_exit);

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    ucalloc((ui32) 1, (ui32) sizeof(global_t));
  
  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * parse command line arguments
   */

  parse_args(argc, argv,
	     &check_params, &print_params,
	     &override,
	     &params_file_path,
	     &n_files, &file_paths);

  /*
   * load up parameters
   */
  
  Glob->table = storm_track_tdrp_init(&Glob->params);

  if (FALSE == TDRP_read(params_file_path,
			 Glob->table,
			 &Glob->params,
			 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_file_path);
    tidy_and_exit(-1);
  }
  TDRP_free_override(&override);

  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    tidy_and_exit(0);
  }

  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    tidy_and_exit(0);
  }

  set_derived_params(n_files);

  /*
   * initialize process registration
   */

  PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  /*
   * intialize storm file handle
   */
  
  RfInitStormFileHandle(&s_handle, Glob->prog_name);

  /*
   * intialize track file handle
   */
  
  RfInitTrackFileHandle(&t_handle, Glob->prog_name);

  /*
   * retracking
   */

  if (Glob->params.mode == RE_TRACK) {

    /*
     * if retracking only is requested, perform it and quit
     */
    
    for (ifile = 0; ifile < n_files; ifile++) {
      char *storm_header_path = file_paths[ifile];
      fprintf(stdout, "\nRetracking file %s\n", storm_header_path);
      fflush(stdout);
      perform_tracking(&s_handle, &t_handle, RETRACK_FILE, storm_header_path);
    }

    tidy_and_exit(0);

  } else {
  
    /*
     * get semaphore set and shared memory if this program is running
     * under the control of storm_ident
     */
    
    setup_shmem();

    /*
     * create lock file
     */

    if (create_lock_file(Glob->shmem->storm_data_dir)) {
      exit(-1);
    }
  
    /*
     * enter a loop which checks the storm_ident_quit semaphore.
     * If storm_ident quits, the loop exits.
     */

    while (forever) {

      int count;

      /*
       * wait for STORM_TRACK_ACTIVE_SEM to be set.
       * This is first done on a non-blocking loop so that
       * PMU_register may be called
       */

      count = 0;
      while (!usem_check(Glob->sem_id, STORM_TRACK_ACTIVE_SEM)) {
	PMU_auto_register("Waiting for storm_ident");
	if (Glob->params.debug >= DEBUG_VERBOSE && count == 0) {
	  fprintf(stderr, "In main - waiting for "
		  "STORM_TRACK_ACTIVE_SEM to be set\n");
	}
	count++;
	if (count > 60) {
	  count = 0;
	}
	sleep(1);
      }
      
      /*
       * check to see if need to quit
       */
      
      if (usem_check(Glob->sem_id, STORM_IDENT_QUIT_SEM)) {
	
	if (Glob->shmem->remove_track_file) {
	  remove_track_files();
	}

	tidy_and_exit(0);

      }

      /*
       * perform tracking
       */
      
      if (perform_tracking(&s_handle, &t_handle,
			   Glob->shmem->tracking_mode,
			   Glob->shmem->storm_header_file_path)) {
	
	/*
	 * failed - must wait a bit for storm_ident to get set
	 * up properly
	 */
	
	PMU_auto_register("Failed - waiting for storm_ident to get set up");
	sleep(1);

      }
      
      /*
       * clear the storm_track_active semaphore
       */

      if (usem_clear(Glob->sem_id, STORM_TRACK_ACTIVE_SEM)) {
	fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
	fprintf(stderr, "Clearing STORM_TRACK_ACTIVE sempahore\n");
	tidy_and_exit(-1);
      }

    } /* while (forever) */

  } /* if (Glob->mode == RETRACK) */

  return(0);

}
