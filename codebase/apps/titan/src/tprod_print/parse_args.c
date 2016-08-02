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
 * parse_args.c: parse command line args
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "tprod_print.h"

void parse_args(int argc,
		char **argv)

{

  int error_flag = 0;
  int i;
  int malloc_debug_level;

  char usage[BUFSIZ];

  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s%s",
	  "Usage: ", Glob->prog_name, " [options as below]\n",
	  "options:\n"
	  "       [ --, -help, -man, -usage, -h ] produce this list.\n"
	  "       [ -comments ] print comments in output\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -full ] full printout - one line per point\n"
	  "                   for polygons and ellipses\n"
	  "       [ -host ? ] track_server host (default local)\n"
	  "       [ -lead ?] forecast leadtime (secs, default 0)\n"
	  "       [ -mdebug level ] malloc debug level\n"
	  "       [ -port ? ] track_server port (default 44000)\n"
	  "\n");

  /*
   * initialize
   */

  Glob->port = 44000;
  Glob->host = PORThostname();
  Glob->debug = FALSE;
  Glob->comments = FALSE;
  Glob->lead_time = -1;
  malloc_debug_level = 0;

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-usage") ||
	!strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {

      Glob->debug = TRUE;
      
    } else if (!strcmp(argv[i], "-comments")) {

      Glob->comments = TRUE;
      
    } else if (!strcmp(argv[i], "-full")) {

      Glob->full = TRUE;
      
    } else if (!strcmp(argv[i], "-mdebug")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &malloc_debug_level) != 1) {
	  error_flag = TRUE;
	}
	if (malloc_debug_level > 0) {
	  umalloc_debug (malloc_debug_level);
	}
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-lead")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &Glob->lead_time) != 1) {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-port")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &Glob->port) != 1) {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-host")) {
      
      if (i < argc - 1) {
	Glob->host = argv[++i];
      } else {
	error_flag = TRUE;
      }
	
    } /* if */
    
  } /* i */

  /*
   * print message if error flag set
   */

  if(error_flag) {
    fprintf(stderr, "%s", usage);
  }

  if (Glob->debug) {
    fprintf(stderr, "----> Running program '%s' <-----\n", Glob->prog_name);
    fprintf(stderr, "  Debugging on\n");
    fprintf(stderr, "  Port number: %d\n", Glob->port);
    fprintf(stderr, "  Forecast lead time (secs): %d\n", Glob->lead_time);
    fprintf(stderr, "  Malloc debug level: %d\n", malloc_debug_level);
  }
  
  if (error_flag)
    exit(-1);
  
}


