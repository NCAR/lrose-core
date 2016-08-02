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
/******************************************************************************
 * cindex_test.c:
 *
 * Writes cdata index files for testing
 * 
 * Mike Dixon
 *
 * RAP NCAR Boulder Colorado USA
 *
 * October 1996
 *
 *******************************************************************************/

#define MAIN
#include "cindex_test.h"
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

  int num_times;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *) umalloc((u_int) sizeof(global_t));

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *) strdup(progname_parts.base);

  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);
  
  /*
   * parse command line arguments
   */

  parse_args(argc, argv, &check_params, &print_params,
	     &params_file_path, &override);
  
  /*
   * load up parameters
   */

  Glob->table = cindex_test_tdrp_init(&Glob->params);
  
  if (!TDRP_read(params_file_path,
		 Glob->table,
		 &Glob->params,
		 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Problems with param file '%s'\n",
	    params_file_path);
    tidy_and_exit(-1);
  }
  
  TDRP_free_override(&override);
  
  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    tidy_and_exit(-1);
  }
  
  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    tidy_and_exit(-1);
  }

  num_times = 0;

  while (Glob->params.num_times < 0 ||
	 num_times < Glob->params.num_times) {

    write_index();

    num_times++;

    /*
     * Don't sleep if we are about to exit the loop
     */

    if (Glob->params.num_times < 0 ||
	num_times < Glob->params.num_times)
      sleep(Glob->params.update_interval);
  }

  tidy_and_exit(0);
  return(0);

}



