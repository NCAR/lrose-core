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
 * bearing2latlon.c
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
#include "bearing2latlon.hh"
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
	     &check_params, &print_params, &override,
	     &params_file_path);

  /*
   * load up parameters
   */
  
  // load TDRP params from command line
  
  char *paramsPath = (char *) "unknown";
  if (Glob->params.loadFromArgs(argc, argv,
                                override.list,
                                &paramsPath)) {
    cerr << "ERROR: " << Glob->prog_name << endl;
    cerr << "Problem with TDRP parameters." << endl;
    tidy_and_exit(-1);
  }
  
  if (Glob->params.malloc_debug_level > 0) {
    umalloc_debug(Glob->params.malloc_debug_level);
  }

  filter();

  /*
   * quit
   */
  
  return(0);

}

