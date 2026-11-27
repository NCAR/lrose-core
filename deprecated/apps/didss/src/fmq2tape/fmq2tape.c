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
 * fmq2tape.c
 *
 * Writes an fmq to tape.
 *
 * Jaimi Yee
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * June 1997
 *
 ****************************************************************************/

#define MAIN
#include "fmq2tape.h"
#undef MAIN

int main(int argc, char **argv)

{
  char *params_file_path = NULL;

  int check_params;
  int print_params;
  
  path_parts_t progname_parts;
  tdrp_override_t override;

  /*
   * allocate global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));


  /*
   * register function to trap termination and interrupts
   */

  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGKILL, tidy_and_exit);
  PORTsignal(SIGUSR1, tidy_and_exit);
  PORTsignal(SIGPIPE, (void (*)())SIG_IGN);

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);

  /*
   * display ucopright message
   */

  ucopyright(Glob->prog_name);

  /*
   * parse the command line arguments
   */
  
  parse_args(argc, argv,
             &check_params, &print_params,
             &override,
             &params_file_path);

  /*
   * load up parameters
   */

  Glob->table = fmq2tape_tdrp_init(&Glob->params);

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
  /*
   * initialize process registration
   */
  
    PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL); 

  /*
   * initialize the fmq
   */

  Glob->fmq_handle_set = FALSE;
  Glob->fmq_file_open = FALSE;
  setup_fmq();

  /*
   * process the data
   */

  send_fmq_to_tape();

  tidy_and_exit(0);

  return(0);

}

