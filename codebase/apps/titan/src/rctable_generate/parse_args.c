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
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rctable_generate.h"

static void print_usage(FILE *out, char *prog_name)

{

  fprintf(out, "%s%s%s%s",
	  "Usage: ", prog_name, " [options as below]\n",
	  "  options:\n"
	  "         [ -debug] set debug mode\n"
	  "         [ -mdebug ?] set malloc debug level\n"
	  "         [ -params name] set parameters file name\n"
	  "         [ --, -h, -help, -man] produce this list\n"
	  "\n");

}

void parse_args(int argc, char **argv)
{

  char *debug_str;
  char *malloc_debug_str;
  char *end_pt;
  si32 ldummy;
  int malloc_debug_level;
  int error_flag = 0;
  int i;
  
  /*
   * initialize
   */

  debug_str = uGetParamString(Glob->prog_name, "debug", "false");
  malloc_debug_str = uGetParamString(Glob->prog_name,
				     "malloc_debug_level", "0");
  
  /*
   * search for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      print_usage(stdout, argv[0]);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug_str = "true";
      
    } else if (!strcmp(argv[i], "-mdebug")) {
	
      if (i < argc - 1)
	malloc_debug_str = argv[i+1];
      else
	error_flag = TRUE;
	
    }

  }

  /*
   * set debug option
   */
  
  if (uset_true_false_param(Glob->prog_name,
			    "parse_args",
			    Glob->params_path_name,
			    debug_str, &Glob->debug,
			    "debug"))
    error_flag = TRUE;

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
   * check for errors
   */

  if(error_flag) {
    print_usage(stderr, argv[0]);
    exit (-1);
  }

}
