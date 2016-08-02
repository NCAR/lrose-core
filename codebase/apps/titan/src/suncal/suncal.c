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
 * suncal.c
 *
 * Reads in bearing/distance pairs from stdin, computes the
 * lat/lon pair and outputs to stdout.
 * 
 * Comments starting with # are passed through unchanged
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Nov 1995
 *
 ************************************************************************/

#define MAIN
#include "suncal.h"
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

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *) umalloc((unsigned int) sizeof(global_t));

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((unsigned int) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * parse command line arguments
   */
  
  parse_args(argc, argv,
	     &check_params, &print_params, &override,
	     &params_file_path);

  /*
   * load up parameters
   */
  
  Glob->table = suncal_tdrp_init(&Glob->params);
  
  if (params_file_path) {
    if (FALSE == TDRP_read(params_file_path,
			   Glob->table,
			   &Glob->params,
			   override.list)) {
      fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
      fprintf(stderr, "Cannot read params file '%s'\n",
	      params_file_path);
      tidy_and_exit(-1);
    }
  } /* if (params_file_path) */
  
  TDRP_free_override(&override);
  
  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    exit(0);
  }
  
  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    exit(0);
  }

  if (Glob->params.malloc_debug_level > 0) {
    umalloc_debug(Glob->params.malloc_debug_level);
  }

  create_table_2();

  /*
   * quit
   */
  
  return(0);

}

