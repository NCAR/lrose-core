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
/***********************************************************************
 * ac_ingest.c
 *
 * Ingests the wmi aircraft data and places in a realtime file
 * and an archive file.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * June 1996
 *
 ************************************************************************/

#define MAIN
#include "ac_ingest.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char *params_file_path = NULL;
  int check_params;
  int print_params;
  path_parts_t progname_parts;
  tdrp_override_t override;
  FILE *tty_file;

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *) umalloc(sizeof(global_t));

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc (strlen(progname_parts.base) + 1);
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * parse command line arguments
   */
  
  parse_args(argc, argv,
	     &check_params, &print_params,
	     &override, &params_file_path);

  /*
   * load up parameters
   */
  
  Glob->table = ac_ingest_tdrp_init(&Glob->params);
  
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
  
  if (Glob->params.malloc_debug_level > 0) {
    umalloc_debug(Glob->params.malloc_debug_level);
  }

  /*
   * initialize process registration
   */
  
  PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);

  if (Glob->params.mode == REALTIME_MODE) {

    /*
     * open up the serial port
     */
    
    if ((tty_file =
	 tty_open(Glob->params.input_device,
		  Glob->params.baud_rate)) == NULL) {
      fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
      fprintf(stderr, "Unable to open serial port\n");
      perror(Glob->params.input_device);
      tidy_and_exit(-1);
    }
    
    /*
     * process the incoming stream
     */
    
    process_realtime_stream(tty_file);

  } else {

    process_test_stream();

  }
    
  /*
   * quit
   */

  tidy_and_exit(0);
  return(0);

}

