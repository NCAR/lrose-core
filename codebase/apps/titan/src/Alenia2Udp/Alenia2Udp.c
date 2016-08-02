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
 * Alenia2Udp.c
 *
 * Reads radar data from Alenia files, puts out a UDP data stream
 * in Alenia real-time format.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1997
 *
 ****************************************************************************/

#define MAIN
#include "Alenia2Udp.h"
#undef MAIN

int main(int argc, char **argv)

{
  char *params_file_path = NULL;
  char **file_paths;

  int check_params;
  int print_params;
  si32 n_files = 0;
  si32 ifile;
  
  path_parts_t progname_parts;
  tdrp_override_t override;

  /*
   * register function to trap termination and interrupts
   */

  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGKILL, tidy_and_exit);
  PORTsignal(SIGPIPE, (void (*)())SIG_IGN);

  /*
   * allocate global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));

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
   * parse the command line arguments
   */
  
  parse_args(argc, argv,
             &check_params, &print_params,
             &override,
             &params_file_path,
	     &n_files, &file_paths);

  /*
   * load up parameters
   */

  Glob->table = Alenia2Udp_tdrp_init(&Glob->params);

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

  set_derived_params();

  /*
   * initialize process registration
   */
  
  PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);

  /*
   * open udp output stream
   */

  if (open_output_udp(Glob->params.output_udp_address,
		      Glob->params.output_udp_port,
		      (Glob->params.debug > DEBUG_NORM))) {
    tidy_and_exit(-1);
  }

  /*
   * process files
   */

  do {
    for (ifile = 0; ifile < n_files; ifile++) {
      process_file(file_paths[ifile]);
    }
  } while (Glob->params.auto_repeat);

  /*
   * close udp output stream
   */

  close_output_udp();

  /*
   * exit
   */
  
  tidy_and_exit(0);
  return(0);

}

