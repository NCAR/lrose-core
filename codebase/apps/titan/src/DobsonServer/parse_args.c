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

#include "DobsonServer.h"

void parse_args(int argc,
		char **argv,
		int *check_params_p,
		int *print_params_p,
		tdrp_override_t *override,
		char **params_file_path_p)

{
  
  int i;
  int error_flag = FALSE;
  int override_dir = FALSE;

  char dir_str[BUFSIZ];
  char tmp_str[BUFSIZ];
  char usage[BUFSIZ];
  
  sprintf(usage, "%s%s%s%s",
	  "Usage:\n\n", argv[0], " [options]\n\n",
	  "options:\n"
	  "         [ --, -help, -man] produce this list\n"
	  "         [ -check_params ] check parameter usage\n"
	  "         [ -c ?] compress for xfer\n"
	  "         [ -d ?] set data directory - multiple allowed\n"
	  "         [ -debug] set debugging on\n"
	  "         [ -f ?] realtime file path\n"
          "         [ -free] free data volume after transfer\n"
	  "         [ -h ?] server mapper host name (ignored - use $SERVMAP_HOST\n"
	  "         [ -i ?] instance\n"
	  "         [ -l ?] live update available\n"
	  "         [ -mdebug ?] set malloc debug level\n"
	  "         [ -n ?] subtype\n"
	  "         [ -p ?] port number\n"
	  "         [ -params ?] set parameters file name\n"
	  "         [ -print_params ] print parameter usage\n"
	  "         [ -u ?] live update available\n"
	  "         [ -verbose ] print verbose debug messages\n"
	  "\n");
  
  /*
   * initialize
   */

  *check_params_p = FALSE;
  *print_params_p = FALSE;
  TDRP_init_override(override);
  sprintf(dir_str, "data_dirs = {");

  /*
   * search for command options
   */
  
  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-check_params")) {
      
      *check_params_p = TRUE;
      
    } else if (!strcmp(argv[i], "-print_params")) {
      
      *print_params_p = TRUE;
      
    } else if (!strcmp(argv[i], "-params")) {
	
      if (i < argc - 1) {
	*params_file_path_p = argv[i+1];
      } else {
	error_flag = TRUE;
      }
	
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
	
    } else if (!strcmp(argv[i], "-d")) {
	
      if (i < argc - 1) {
	if (override_dir) {
	  sprintf(tmp_str, ",\"%s\"", argv[i+1]);
	} else {
	  sprintf(tmp_str, "\"%s\"", argv[i+1]);
	}
	STRconcat(dir_str, tmp_str, BUFSIZ);
	override_dir = TRUE;
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "realtime_file_path = \"%s\";", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	sprintf(tmp_str, "use_realtime_file = TRUE;");
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-free")) {

      sprintf(tmp_str, "free_volume = TRUE;");
      TDRP_add_override(override, tmp_str);
    
    } else if (!strcmp(argv[i], "-l") ||
	       !strcmp(argv[i], "-u")) {
      
      sprintf(tmp_str, "realtime_avail = TRUE;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-h")) {
      
      if (i < argc - 1) {
	
      /* ignore this option - use $SERVMAP_HOST instead */
	
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-i")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = \"%s\";", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-n")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "subtype = \"%s\";", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-c")) {
      
      sprintf(tmp_str, "compress_for_transfer = TRUE;");
      TDRP_add_override(override, tmp_str);
	
    } else if (!strcmp(argv[i], "-p")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "port = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-n")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "realtime_avail = FALSE;");
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } /* if */
    
  } /* i */
  
  if (override_dir) {
    STRconcat(dir_str, "};", BUFSIZ);
    TDRP_add_override(override, dir_str);
  }

  /*
   * print message if error flag set
   */
  
  if(error_flag) {
    fprintf(stderr, "%s", usage);
    tidy_and_exit(-1);
  }
  
}

