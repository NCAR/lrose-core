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
 * December 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_print.h"

void parse_args(int argc, char **argv,
		si32 *n_files_p,
		char ***file_paths_p)
     
{
  
  int error_flag = 0;
  int i, j;
  int malloc_debug_level;
  char usage[BUFSIZ];
  
  /*
   * set usage string
   */
  
  sprintf(usage, "%s%s%s%s",
	  "Usage: ", Glob->prog_name, " [options as below] file-name\n",
	  "options:\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -cn ?] complex track number\n"
	  "       [ -f, file_paths] set track file paths\n"
	  "       [ -layer] print layer data\n"
	  "       [ -md ?] min. track duration (secs)\n"
	  "       [ -mdebug ?] set malloc debug level\n"
	  "       [ -sn ?] complex track number\n"
	  "\n");
  
  if (argc < 2) {
    fprintf(stderr, "%s", usage);
    tidy_and_exit(-1);
  }
  
  /*
   * set defaults
   */
  
  Glob->print_layers = FALSE;
  Glob->print_complex = FALSE;
  Glob->print_simple = FALSE;
  Glob->min_duration = 0;
  
  /*
   * look for command options
   */
  
  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") || !strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") || !strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-layers")) {
      
      Glob->print_layers = TRUE;
      
    } else if (!strcmp(argv[i], "-md")) {
      
      if(i < argc - 1) {
	if (sscanf(argv[i+1], "%ld", &Glob->min_duration) != 1) {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-cn")) {
      
      if(i < argc - 1) {
	if (sscanf(argv[i+1], "%ld", &Glob->complex_num) == 1) {
	  Glob->print_complex = TRUE;
	} else {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-sn")) {
      
      if(i < argc - 1) {
	if (sscanf(argv[i+1], "%ld", &Glob->simple_num) == 1) {
	  Glob->print_simple = TRUE;
	} else {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-mdebug")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[i+1], "%d", &malloc_debug_level) == 1) {
	  umalloc_debug (malloc_debug_level);
	} else {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
      
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

	*n_files_p = j - i - 1;

	/*
	 * set file name array
	 */

	*file_paths_p = argv + i + 1;

      }

    } /* if (!strcmp ... */
    
  } /* i */
  
  /*
   * print message if warning or error flag set
   */
  
  if(error_flag) {
    fprintf(stderr, "%s", usage);
    tidy_and_exit(-1);
  }

  return;
  
}
