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
 * June 1995
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <tdrp/tdrp.h>

#include "cindex_test.h"

void parse_args(int argc,
		char **argv,
		int *check_params_p,
		int *print_params_p,
		char **params_file_path_p,
		tdrp_override_t *override)
{
  int error_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];
  
  /*
   * set usage string
   */
  
  sprintf(usage, "%s%s%s",
	  "Usage: ", Glob->prog_name, " [options as below]\n"
	  "options:\n"
	  "       [ --, -h, -help, -man ] produce this list\n"
	  "       [ -check_params ] check parameter usage\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -dir ?] data directory\n"
	  "       [ -ext ?] file extension\n"
	  "       [ -i ?] instance\n"
	  "       [ -mdebug ?] set malloc debug level\n"
	  "       [ -mode ?] REALTIME or ARCHIVE (default)\n"
	  "       [ -ntimes ?] number of times to write file, -1 (default) to write forever\n"
	  "       [ -params path ] specify params file\n"
	  "       [ -print_params ] print parameter usage\n"
	  "       [ -start y m d h m s] start time\n"
	  "       [ -u ?] update interval (secs)\n"
	  "       [ -warnings ] print warning messages\n"
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

    if (strcmp(argv[i], "--") == 0 ||
	strcmp(argv[i], "-h") == 0 ||
	strcmp(argv[i], "-help") == 0 ||
	strcmp(argv[i], "-man") == 0) {

      fprintf(stderr, "%s", usage);
      exit(0);

    } else if (strcmp(argv[i], "-check_params") == 0) {

      *check_params_p = TRUE;
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-extra")) {
      
      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-warnings")) {
      
      sprintf(tmp_str, "debug = DEBUG_WARNINGS;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mdebug")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "malloc_debug_level = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
	
    } else if (strcmp(argv[i], "-print_params") == 0) {

      *print_params_p = TRUE;

    } else if (strcmp(argv[i], "-params") == 0) {

      if (i < argc - 1) {
	*params_file_path_p = argv[++i];
      } else {
	error_flag = TRUE;
      }
      
    } else if (strcmp(argv[i], "-dir") == 0) {

      if (i < argc - 1) {
	sprintf(tmp_str, "data_dir = \"%s\";", argv[++i]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }

    } else if (strcmp(argv[i], "-ext") == 0) {

      if (i < argc - 1) {
	sprintf(tmp_str, "file_extension = \"%s\";", argv[++i]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }

    } else if (strcmp(argv[i], "-i") == 0) {

      if (i < argc - 1) {
	sprintf(tmp_str, "instance = \"%s\";", argv[++i]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }

    } else if (strcmp(argv[i], "-mode") == 0) {

      if (i < argc - 1) {
	sprintf(tmp_str, "mode = %s;", argv[++i]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
      
    } else if (strcmp(argv[i], "-ntimes") == 0) {

      if (i < argc - 1) {
	sprintf(tmp_str, "num_times = %s;", argv[++i]);
	TDRP_add_override(override, tmp_str);
      } else {
	error_flag = TRUE;
      }
      
    } else if (strcmp(argv[i], "-start") == 0) {
      
      if (i < argc - 6) {
	sprintf(tmp_str, "start_time = {%s, %s, %s, %s, %s, %s};",
		argv[++i], argv[++i], argv[++i],
		argv[++i], argv[++i], argv[++i]);
	TDRP_add_override(override, tmp_str);
	TDRP_add_override(override, "mode = ARCHIVE;");
      } else {
	error_flag = TRUE;
      }
      
    } else if (strcmp(argv[i], "-u") == 0) {

      if (i < argc - 1) {
	sprintf(tmp_str, "update_interval = %s;", argv[++i]);
	TDRP_add_override(override, tmp_str);
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
    exit (-1);
  }
  
  return;
  
}

