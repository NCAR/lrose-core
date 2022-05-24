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
 * parse_args.c: parse command line args, open files as required
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "verify_grid.h"

void parse_args(int argc,
		char **argv,
		char **params_file_path_p,
		int *check_params_p,
		int *print_params_p,
		tdrp_override_t *override,
		si32 *n_track_files_p,
		char ***track_file_paths)

{

  int error_flag = 0;
  int i, j;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];

  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s%s",
	  "Usage: ", Glob->prog_name, " [options as below]\n",
	  "options:\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -check_params ] check parameter usage\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -f, -filepath file_paths] set grid file paths\n"
	  "       [ -mdebug level ] set malloc debug level\n"
	  "       [ -mode ?] archive or realtime\n"
	  "       [ -params path ] specify params file (default 'params')\n"
	  "       [ -print_params ] print parameter usage\n"
	  "\n");

  /*
   * initialize
   */

  *check_params_p = FALSE;
  *print_params_p = FALSE;
  TDRP_init_override(override);

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-check_params")) {
      
      *check_params_p = TRUE;
      
    } else if (!strcmp(argv[i], "-print_params")) {
      
      *print_params_p = TRUE;
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mdebug")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "malloc_debug_level = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-mode")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "mode = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-params")) {
	
      if (i < argc - 1) {
	*params_file_path_p = argv[i+1];
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "-filepath")) {

      if(i < argc - 1) {

	/*
	 * search for next arg which starts with '-'
	 */

	for (j = i + 1; j < argc; j++)
	  if (argv[j][0] == '-')
	    break;
	
	/*
	 * compute number of files
	 */

	*n_track_files_p = j - i - 1;

	/*
	 * set file name array
	 */

	*track_file_paths = argv + i + 1;

      }

    } /* if */
    
  } /* i */

  /*
   * print message if error flag set
   */

  if(error_flag) {
    fprintf(stderr, "%s", usage);
  }
  
  if (error_flag)
    exit(-1);
  
}

