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
/************************************************************************
 * verify_grid.c
 *
 * Verifies reflectivity data in a 2-d dobson grid against a grid
 * regarded as truth
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Jan 1992
 *
 ************************************************************************/

#define MAIN
#include "verify_grid.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  path_parts_t progname_parts;
  char **detect_file_paths;
  char *params_file_path = NULL;
  int check_params;
  int print_params;
  si32 n_files, ifile;
  contingency_t cont;
  statistics_t stats;
  tdrp_override_t override;

  /*
   * allocate space for the global structure
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
   * initialize
   */

  memset ((void *) &cont,
          (int) 0, (size_t) sizeof(contingency_t));
  memset ((void *) &stats,
          (int) 0, (size_t) sizeof(statistics_t));

  /*
   * parse command line arguments
   */

  parse_args(argc, argv,
	     &params_file_path,
	     &check_params,
	     &print_params,
	     &override,
	     &n_files,
	     &detect_file_paths);

  /*
   * load up parameters
   */
  
  Glob->table = verify_grid_tdrp_init(&Glob->params);

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
  
  if (Glob->params.malloc_debug_level > 0)
    umalloc_debug(Glob->params.malloc_debug_level);
  
  /*
   * preparation
   */

  switch (Glob->params.mode) {

  case CONT:
    if (!(Glob->params.relaxed_contingency ||
	  Glob->params.use_native_grid) &&
	Glob->params.output_intermediate_grids)
      init_intermediate_grid();
    break;

  case STATS:

    /*
     * allocate memory for histogram arrays
     */

    stats.hist_n_intervals = Glob->params.hist.n_intervals;
    stats.hist_low_limit = Glob->params.hist.low_limit;
    stats.hist_interval_size = Glob->params.hist.interval_size;

    stats.n_per_interval = (double *) ucalloc
      ((ui32) stats.hist_n_intervals, (ui32) sizeof(double));
  
    stats.percent_per_interval = (double *) ucalloc
      ((ui32) stats.hist_n_intervals, (ui32) sizeof(double));

    break;

  case REGRESSION:

    print_header(n_files, detect_file_paths, stdout);

    break;

  } /* switch */

  /*
   * loop through the files
   */

  for (ifile = 0; ifile < n_files; ifile++) {

    switch (Glob->params.mode) {

    case CONT:

      update_cont(detect_file_paths[ifile], &cont);
      break;

    case STATS:

      update_stats(detect_file_paths[ifile], &stats);
      break;

    case REGRESSION:

      update_regression(detect_file_paths[ifile],
			stdout);
      break;

    } /* switch */
  
  } /* ifile */

  /*
   * print out
   */

  fprintf(stdout, "\n%s\n\n", Glob->prog_name);
  fprintf(stdout, "Parameters:\n");
  TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, FALSE);

  switch (Glob->params.mode) {

  case CONT:

    print_contingency_table(&cont, stdout);
    break;

  case STATS:

    print_stats(&stats, stdout);
    break;

  case REGRESSION:

    break;

  } /* switch */

  /*
   * quit
   */
  
  tidy_and_exit(0);
  return(0);

}

