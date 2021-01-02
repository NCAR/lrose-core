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

#include "grid_forecast.h"

void parse_args(int argc,
		char **argv,
		si32 *n_track_files,
		char ***track_file_paths)

{

  int error_flag = 0;
  int warning_flag = 0;
  int i, j;
  int malloc_debug_level;
  int file_paths_set = FALSE;
  si32 ldummy;

  char *end_pt;
  char usage[BUFSIZ];
  char *malloc_debug_str;
  char *debug_str;

  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s%s%s%s%s%s%s%s%s",
	  "Usage: ", Glob->prog_name, " [options as below]",
	  " [-f file_paths]\n",
	  "options:\n",
	  "       [ --, -help, -man ] produce this list.\n",
	  "       [ -debug ] print debug messages\n",
	  "       [ -mdebug level ] set malloc debug level\n",
	  "       [ -noparams ] use no parameter file\n",
	  "       [ -params path ] specify alternative parameters file\n",
	  "\n");

  /*
   * set defaults
   */

  debug_str = uGetParamString(Glob->prog_name, "debug", DEBUG_STR);
  malloc_debug_str = uGetParamString(Glob->prog_name,
				     "malloc_debug_level", "0");

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug_str = "true";
      
    } else if (!strcmp(argv[i], "-mdebug")) {
	
      if (i < argc - 1)
	malloc_debug_str = argv[i+1];
      else
	error_flag = TRUE;
	
    } else if (!strcmp(argv[i], "-f")) {

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

	*n_track_files = j - i - 1;

	/*
	 * set file name array
	 */

	*track_file_paths = argv + i + 1;

	file_paths_set = TRUE;

      } else {

	error_flag = TRUE;

      }

    } /* if */
    
  } /* i */

  if (file_paths_set == FALSE) {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr,
	    "No file paths given - the '-f' arg is required.\n");
    error_flag = TRUE;
  }

  /*
   * set debug option
   */
  
  if (!strcmp(debug_str, "yes") || !strcmp(debug_str, "true")) {
    Glob->debug = TRUE;
  } else if (!strcmp(debug_str, "no") || !strcmp(debug_str, "false")) {
    Glob->debug = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr, "Debug option '%s' not recognized.\n",
	    debug_str);
    error_flag = TRUE;
  }
  
  /*
   * set the malloc debug level
   */

  errno = 0;
  ldummy = (double) strtol(malloc_debug_str, &end_pt, 10);

  if (errno == 0)
    malloc_debug_level = ldummy;
  else
    malloc_debug_level = 0;

  umalloc_debug ((int) malloc_debug_level);

  /*
   * print message if warning or error flag set
   */

  if(error_flag || warning_flag) {
    fprintf(stderr, "%s", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    Glob->params_path_name);
  }
  
  if (error_flag)
    tidy_and_exit(-1);
  
}
