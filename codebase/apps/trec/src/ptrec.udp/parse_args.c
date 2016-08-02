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
 * May 1996
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "trec.h"

void parse_args(int argc,
		char **argv,
		int *check_params_p,
		int *print_params_brief_p,
		int *print_params_full_p,
		tdrp_override_t *override,
		char **params_file_path_p)


{

  int error_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];

  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s%s",
	  "Usage: ", Glob->prog_name, "[options as below]\n",
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -check_params ] check parameter usage\n"
	  "       [ -debug ] print debug messages\n"
	  "       [-header [n]] print header each n records (n default 360)\n"
	  "       [ -params ?] set params file name\n"
	  "       [ -print_params_brief ] brief parameter listing\n"
	  "       [ -print_params_full ] commented parameter listing\n"
	  "       [-summary [n]] print summary each n records (n default 90)\n"
	  "       [ -udp_port ?] udp port number\n"
	  "\n");

  /*
   * initialize
   */

  *check_params_p = FALSE;
  *print_params_brief_p = FALSE;
  *print_params_full_p = FALSE;
  TDRP_init_override(override);

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s\n", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-check_params")) {
      
      *check_params_p = TRUE;
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-header")) {
      
      sprintf(tmp_str, "print_header = TRUE;");
      TDRP_add_override(override, tmp_str);
      
      if (i < argc - 1) {
	sprintf(tmp_str, "header_interval = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      }

    } else if (!strcmp(argv[i], "-udp_port")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "udp_port = %s;", argv[i+1]);
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
	
    } else if (!strcmp(argv[i], "-print_params_brief")) {
      
      *print_params_brief_p = TRUE;
      
    } else if (!strcmp(argv[i], "-print_params_full")) {
      
      *print_params_full_p = TRUE;
      
    } else if (!strcmp(argv[i], "-summary")) {
      
      sprintf(tmp_str, "print_summary = TRUE;");
      TDRP_add_override(override, tmp_str);
      
      if (i < argc - 1) {
	sprintf(tmp_str, "summary_interval = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      }

    } /* if */
    
  } /* i */

  /*
   * print message if error flag set
   */

  if(error_flag) {
    fprintf(stderr, "%s\n", usage);
  }
  
  if (error_flag)
    exit(-1);
  
}

