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
/*****************************************************************
 * PARSE_ARGS: Progess command line arguments. Set option flags
 * And print usage info if necessary
 */

#include "didss_compress.h"

#define MIN_AGE 3600L
#define TOP_DIR "."
#define VERBOSE 0
#define SLEEP_FACTOR 10.0
#define GZIP 0
#define DATE_FORMAT 0
#define TIME_FORMAT 0
#define CHECK_EXT 0
#define EXT ""
#define RECURSIVE 1

static void print_usage(FILE *fp, char *prog_path)
{
  fprintf(fp, "%s%s%s%s",
	  "Usage:\n\n", prog_path, " [options as below]\n\n",
	  "  [--, -h, -help, -man, -usage] produce this list.\n"
	  "  [-a ?] min file age for compression (secs).\n"
	  "    Default is 3600 - 1 hour\n"
	  "  [-d ?] top dir. Default is . - current dir.\n"
	  "    Multiple directories may be specified using\n"
	  "    multiple -d args.\n"
	  "  [-df] check file has date format name.\n"
	  "    This is YYYYMMDD.ext.\n"
	  "    Default is no format checking.\n"
	  "    Cannot be used with -tf\n"
          "  [-ext ?] Check file has this extension.\n"
	  "    Default is no checking.\n"
	  "  [-gzip] use gzip compression. Default is use compress.\n"
	  "  [-i ?] procmap instance. Default is none (doesn't register).\n"
	  "  [-nr] Non-recursive directory search. Default is recursive.\n"
	  "  [-reg ?] Specify procmap register interval in secs.  Default is PROCMAP_REGISTER_INTERVAL (60).\n"
	  "  [-sl ?] set sleep factor.\n"
	  "    After each compress the program sleeps for an interval.\n"
	  "    of sleep_factor * time_for_last_compression.\n"
	  "    Default is 10, minimum 1.\n"
	  "  [-tf] check file has date format name.\n"
	  "    This is HHMMSS.ext.\n"
	  "    Default is no format checking.\n"
	  "    Cannot be used with -df\n"
	  "  [-v] verbose mode. Default is off.\n"
	  "\n");
}

void parse_args(int argc, char **argv)

{
  
  int i;
  int error_flag = FALSE;
  
  /*
   * check the # of args
   */
  
  /*
   * set defaults
   */

  Glob->n_dirs = 0;
  Glob->top_dirs = NULL;
  Glob->min_age = MIN_AGE;
  Glob->verbose = VERBOSE;
  Glob->sleep_factor = SLEEP_FACTOR;
  Glob->gzip = GZIP;
  Glob->date_format = DATE_FORMAT;
  Glob->time_format = TIME_FORMAT;
  Glob->check_ext = CHECK_EXT;
  Glob->recursive = RECURSIVE;
  Glob->procmap_instance = NULL;
  Glob->procmap_register_interval = PROCMAP_REGISTER_INTERVAL;
  
  /*
   * search for command options
   */
  
  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man") ||
	!strcmp(argv[i], "-usage")) {

      print_usage(stdout, Glob->prog_name);
      exit (0);
      
    } else if (!strcmp(argv[i], "-a")) {
      
      /*
       * min age
       */

      if (i < argc - 1) {
	if (sscanf(argv[i+1], "%d", &Glob->min_age) != 1) {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-d")) {
      
      if (i < argc - 1) {

	Glob->n_dirs += 1;
	Glob->top_dirs =
	  (char **) urealloc ((char *) Glob->top_dirs,
			      (u_int) (Glob->n_dirs * sizeof(char *)));
	Glob->top_dirs[Glob->n_dirs - 1]  = argv[i+1];

      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-df")) {
      
      Glob->date_format = TRUE;
      
    } else if (!strcmp(argv[i], "-ext")) {
	
      if (i < argc - 1) {
	Glob->check_ext = TRUE;
	Glob->ext = argv[i+1];
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-gzip")) {
      
      Glob->gzip = TRUE;

    } else if (!strcmp(argv[i], "-i")) {
      
      /*
       * procmap instance
       */

      if (i < argc - 1) {
	Glob->procmap_instance = argv[i+1];
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-nr")) {
      
      Glob->recursive = FALSE;
      
    } else if (!strcmp(argv[i], "-reg")) {
      
      /*
       * procmap register interval
       */

      if (i < argc - 1) {
	Glob->procmap_register_interval = atoi(argv[i+1]);
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-sl")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[i+1], "%d", &Glob->sleep_factor) != 1) {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
      
      if (Glob->sleep_factor < 1) {
	Glob->sleep_factor = 1;
      }
      
    } else if (!strcmp(argv[i], "-tf")) {
      
      Glob->time_format = TRUE;
      
    } else if (!strcmp(argv[i], "-v")) {
      
      Glob->verbose = TRUE;
      
    }
    
  } /* i */

  /*
   * if no directories listed, use default
   */

  if (Glob->n_dirs == 0) {
    Glob->n_dirs = 1;
    Glob->top_dirs = (char **) umalloc ((u_int) sizeof(char *));
    Glob->top_dirs[0] = TOP_DIR;
  }
      
  /*
   * make sure that date_format and time_format are not both set
   */

  if (Glob->time_format && Glob->date_format) {
    error_flag = TRUE;
  }
  
  /*
   * check for errors
   */

  if (error_flag) {
    print_usage(stderr, Glob->prog_name);
    exit(-1);
  }
  
}

