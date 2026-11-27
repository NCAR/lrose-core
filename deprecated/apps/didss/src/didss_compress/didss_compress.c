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
 * DIDSS_COMPRESS.C
 *
 * Compression monitor for server files.
 *
 * At regular intervals, searches for uncompressed files
 * and compresses them.
 * 
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 */

#define MAIN
#include "didss_compress.h"
#undef MAIN

#include <time.h>

#include <toolsa/port.h>

/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */

int main(int argc, char **argv)

{
  
  path_parts_t progname_parts;
  int forever = TRUE;
  long i;
  
  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGPIPE, SIG_IGN);
  
  /*
   * allocate global structure and other memory, initialize
   */
  
  Glob = (global_t *) umalloc((u_int) sizeof(global_t));
  
  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((u_int) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * display ucopright
   */
  
  ucopyright(Glob->prog_name);
  
  /*
   * process command line arguments
   */
  
  parse_args(argc,argv);
  
  /*
   * Initialize process registration
   */

  if (Glob->procmap_instance != NULL)
    PMU_auto_init(Glob->prog_name,
		  Glob->procmap_instance,
		  Glob->procmap_register_interval);

  /*
   * turn process into a daemon
   */
  
  if (!Glob->verbose) {
    udaemonize();
  }
  
  /*
   * regularly check files
   */

  while (forever) {

    if (Glob->procmap_instance != NULL)
      PMU_force_register("Checking for uncompressed files");

    Glob->n_compressed = 0;
    
    if (Glob->verbose) {
      fprintf(stderr, "%s checking for uncompressed files\n",
	      Glob->prog_name);
    }

    for (i = 0; i < Glob->n_dirs; i++) {

      if (Glob->procmap_instance != NULL)
	PMU_auto_register("Checking for uncompressed files");

      check_dir(Glob->top_dirs[i]);
    
      if (Glob->verbose) {
	fprintf(stderr, "%d files compressed this cycle\n",
		(int) Glob->n_compressed);
      }

    } /* i */
      
    if (Glob->procmap_instance != NULL)
      PMU_force_register("Sleeping between checks");

    if (Glob->n_compressed == 0) {
      int num_sleeps = Glob->min_age / 10 + 1;
      
      for (i = 0; i < num_sleeps; i++)
      {
	PMU_auto_register("Sleeping min_age seconds after compressing files");
	sleep(10);
      }
    } else {
      PMU_force_register("Sleeping 30 seconds after compressing files");
      sleep(30);
    }

  } /* while */

  return(0);
  
}

