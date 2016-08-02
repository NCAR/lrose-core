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
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * January 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_ident.h"

static si32 load_time(char *time_str, char *label);

void parse_args(int argc,
		char **argv,
		int *check_params_p,
		int *print_params_p,
		tdrp_override_t *override,
		char **params_file_path_p)
     
{
  
  int error_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];
  
  /*
   * set usage
   */
  
  sprintf(usage, "%s%s%s%s",
	  "Usage: ", argv[0], "\n",
	  " [ --, -h, -help, -man ] produce this list\n"
	  " [ -debug ] print debug messages\n"
	  " [ -endtime yyyy/mm/dd_hh:mm:ss ] end time (archive mode)\n"
	  " [ -mdebug ?] set malloc debug level\n"
	  " [ -mode ?] 'archive' or 'realtime'\n"
	  " [ -params name ] set parameters file name\n"
	  " [ -reftime yyyy/mm/dd_hh:mm:ss ] ref time (archive mode)\n"
	  "                (will be phased out - should not be used)\n"
	  " [ -starttime yyyy/mm/dd_hh:mm:ss ] start time (archive mode)\n"
	  " [ -track ] perform tracking?\n"
	  " [ -verbose ] print verbose messages\n"
	  "\n");
  
  /*
   * initialize
   */

  *check_params_p = FALSE;
  *print_params_p = FALSE;
  TDRP_init_override(override);
  Glob->ref_time = 0;
  Glob->end_time = 0;
  Glob->start_time = 0;

  /*
   * look for command options
   */
  
  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "-help")
	|| !strcmp(argv[i], "-h")
	|| !strcmp(argv[i], "-man")
	|| !strcmp(argv[i], "--")) {
      
      printf("%s", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-check_params")) {
      
      *check_params_p = TRUE;
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
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
	
    } else if (!strcmp(argv[i], "-print_params")) {
      
      *print_params_p = TRUE;
      
    } else if (!strcmp(argv[i], "-track")) {
      
      sprintf(tmp_str, "track = TRUE;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-reftime")) {
      
      if (i < argc - 1) {
	if ((Glob->ref_time = load_time(argv[i+1], "ref")) < 0) {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }

      fprintf(stderr, "WARNING: %s/parse_args\n", Glob->prog_name);
      fprintf(stderr,
	      "Use of -reftime will not be supported in the future\n");
      fprintf(stderr, "Use -starttime and -endtime instead\n");
      
    } else if (!strcmp(argv[i], "-endtime")) {
      
      if (i < argc - 1) {
	if ((Glob->end_time = load_time(argv[i+1], "end")) < 0) {
	  error_flag = 0;
	}
      } else {
	error_flag = TRUE;
      }

    } else if (!strcmp(argv[i], "-starttime")) {
      
      if (i < argc - 1) {
	if ((Glob->start_time = load_time(argv[i+1], "start")) < 0) {
	  error_flag = 0;
	}
      } else {
	error_flag = TRUE;
      }

    }	
    
  } /* i */
  
  /*
   * print message if warning or error flag set
   */
  
  if(error_flag) {
    fprintf(stderr, "%s", usage);
    exit(-1);
  }
  
}

/*
 * load time from string - returns -1 on error
 */

static si32 load_time(char *time_str, char *label)


{

  date_time_t dtime;

  if (sscanf(time_str, "%d/%d/%d_%d:%d:%d",
	     &dtime.year,
	     &dtime.month,
	     &dtime.day,
	     &dtime.hour,
	     &dtime.min,
	     &dtime.sec) != 6) {
    
    fprintf(stderr, "ERROR - %s:parse_args:load_time\n", Glob->prog_name);
    fprintf(stderr, "Incorrect format for %s time.\n", label);
    fprintf(stderr, "Trying to decode '%s'\n", time_str);
    fprintf(stderr, "Format is yyyy/mm/dd_hh:mm:ss\n");
    return (-1L);
  }
    
  /*
   * check that time is valid
   */
  
  if (!uvalid_datetime(&dtime)) {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr, "Invalid reference date and time.\n");
    fprintf(stderr, "Trying to decode '%s'\n", time_str);
    return (-1L);
  }
    
  uconvert_to_utime(&dtime);

  return (dtime.unix_time);

}



