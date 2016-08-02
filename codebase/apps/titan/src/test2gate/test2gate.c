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
/**********************************************************************
 * test2gate.c
 *
 * Generates test TASS radar data and writes the data beam-by-beam to
 * an output socket. This socket is intended to be read by polar_ingest.
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * September 1995
 *
 **********************************************************************/

#include <signal.h>
#include <toolsa/pmu.h>

#define MAIN
#include "test2gate.h"
#undef MAIN

int main(int argc, char **argv)
{
  int  check_params;
  int  print_params;
  
  path_parts_t progname_parts;

  tdrp_override_t  override;
  
  /*
   * allocate global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));

  /*
   * register function to trap termination and interrupts
   */

  signal(SIGQUIT, (void (*)())tidy_and_exit);
  signal(SIGTERM, (void (*)())tidy_and_exit);
  signal(SIGINT, (void (*)())tidy_and_exit);
  signal(SIGPIPE, (void (*)())SIG_IGN);

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
  
  parse_args(argc, argv, &check_params, &print_params,
	     &Glob->params_path_name, &override);

  /*
   * load up parameters
   */

  Glob->table = test2gate_tdrp_init(&Glob->params);
  
  if (TDRP_read(Glob->params_path_name,
		Glob->table,
		&Glob->params,
		override.list) == FALSE)
  {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    Glob->params_path_name);
    tidy_and_exit(-1);
  }
  
  TDRP_free_override(&override);
  
  if (check_params)
  {
    TDRP_check_is_set(Glob->table, &Glob->params);
    tidy_and_exit(-1);
  }
  
  if (print_params)
  {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    tidy_and_exit(0);
  }
  
  if (Glob->params.malloc_debug_level > 0)
    umalloc_debug(Glob->params.malloc_debug_level);
  
  Glob->nfields = 1;
  if (Glob->params.output_vel_field) {
    Glob->nfields += 1;
  }
  if (Glob->params.output_geom_fields) {
    Glob->nfields += 3;
  }

  /*
   * init process mapper registration
   */

  PMU_auto_init(Glob->prog_name, Glob->params.instance, PROCMAP_REGISTER_INTERVAL);

  /*
   * Process the data
   */

  create_data_stream();

  tidy_and_exit(0);
  
  return(0);

}

