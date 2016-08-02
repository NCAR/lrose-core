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
 * November 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "clutter_compute.h"

void parse_args(int argc, char **argv)

{

  char usage[BUFSIZ];
  char *debug_str, *malloc_debug_str;
  char *end_pt;
  
  int error_flag = FALSE;
  int malloc_debug_level;
  int i;

  long ldummy;
  
  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s%s%s%s%s%s%s",
	  "Usage: ", argv[0], "\n",
	  "       [--, -man, -help] produce this list.\n",
	  "       [-debug] set debug mode on\n",
	  "       [-mdebug ?] set malloc debug level\n",
	  "       [-noparams] use no parameters file\n",
	  "       [-params path] specify alternative parameters file\n",
	  "\n");

  /*
   * set defaults
   */

  debug_str = uGetParamString(Glob->prog_name,
			      "debug", "false");
  
  malloc_debug_str = uGetParamString(Glob->prog_name,
				     "malloc_debug_level", "0");

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {

      printf("%s", usage);
      exit(0);

    } else if (!strcmp(argv[i], "-debug")) {

      debug_str = "true";

    } else if (!strcmp(argv[i], "-mdebug")) {

      if (i < argc - 1) {
      
	malloc_debug_str = argv[i+1];

      } else {

	error_flag = TRUE;

      }

    } /* if */

  } /* i */

  /*
   * set debug option
   */
  
  if (!strcmp(debug_str, "yes") ||
      !strcmp(debug_str, "true")) {
    Glob->debug = TRUE;
  } else if (!strcmp(debug_str, "no") ||
	     !strcmp(debug_str, "false")) {
    Glob->debug = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr, "Debug option '%s' not recognized.\n",
	    debug_str);
    fprintf(stderr, "Valid options are 'true', 'false', 'yes', 'no'\n");
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

  if(error_flag) {
    fprintf(stderr, "%s", usage);
    fprintf(stderr,
	    "Check the parameter file '%s'.\n\n",
	    Glob->params_path_name);
    exit(1);
  }

}
