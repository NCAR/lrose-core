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
 * parse_args.c
 *
 * parse command line args
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "fmq_print.h"

void parse_args(int argc, char **argv,
		char **fmq_path_p)

{
  
  int error_flag = 0;
  int i;
  char usage[BUFSIZ];
  
  /*
   * set usage string
   */
  
  sprintf(usage, "%s%s%s%s",
	  "Usage: ", Glob->prog_name, " [options as below] fmq-name\n",
	  "options:\n"
	  "       [ -all ] full listing - not yet implemented\n"
	  "       [ -slots ] slots listing\n"
	  "       [ -stats ] stats listing - default\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "\n");
  
  if (argc < 2) {
    fprintf(stderr, "%s", usage);
    exit(-1);
  }
  
  /*
   * set defaults
   */
  
  Glob->print_stats = TRUE;
  Glob->print_slots = FALSE;
  Glob->print_all = FALSE;

  /*
   * set file name
   */
  
  *fmq_path_p = argv[argc - 1];
  
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
      
    } else if (!strcmp(argv[i], "-all")) {
      
      Glob->print_all = TRUE;
      
    } else if (!strcmp(argv[i], "-slots")) {
      
      Glob->print_slots = TRUE;
      
    } else if (!strcmp(argv[i], "-stats")) {
      
      Glob->print_stats = TRUE;
      
    }
    
  }
  
  /*
   * print message if warning or error flag set
   */
  
  if(error_flag) {
    fprintf(stderr, "%s", usage);
    exit(-1);
  }
  
}
