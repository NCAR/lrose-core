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
 * Parse command line arguments.  Create a TDRP override list from
 * the command line arguments, as needed.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1990
 *
 * Mike Dixon
 *
 * Copied for use in ufnexrad2gate by Nancy Rehak, Mar 1995.
 *********************************************************************/

#include <errno.h>

#include "ufnexrad2gate.h"

void parse_args(int argc, char **argv,
		int *check_params_p,
		int *print_params_p,
		char **params_file_path_p,
		tdrp_override_t *override)

{

  int error_flag = 0;
  int warning_flag = 0;
  int i;
  si32 ltmp;
 
  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];
  
  /*
   * set usage
   */

  sprintf(usage, "%s%s%s",
	  "Usage: ", argv[0], "\n"
	  "       [--, -help, -man] produce this list.\n"
	  "       [-check_params] check parameter usage\n"
	  "       [-debug] print debug messages\n"
	  "       [-header [n]] print header each n records (n default 360)\n"
	  "       [-mdebug ?] set malloc debug level\n"
	  "       [-params ?] set parameters file name\n"
	  "       [-print_params] print parameter usage\n"
	  "       [-summary [n]] print summary each n records (n default 1)\n"
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

  for (i =  1; i < argc; i++)
  {
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-usage") ||
	!strcmp(argv[i], "-man"))
    {
      fprintf(stderr, "%s", usage);
      exit(0);
    }
    else if (!strcmp(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(override, tmp_str);
    }
    else if (!strcmp(argv[i], "-header"))
    {
      sprintf(tmp_str, "header_print = TRUE;");
      TDRP_add_override(override, tmp_str);
      
      if (i < argc - 1)
	ltmp = atol(argv[i+1]);
      else
	ltmp = 0;
      
      if (ltmp > 0) 
      {
	sprintf(tmp_str, "header_interval = %d;", ltmp);
	TDRP_add_override(override, tmp_str);
      }
    }
    else if (!strcmp(argv[i], "-mdebug"))
    {
      if (i < argc - 1)
      {
	sprintf(tmp_str, "malloc_debug_level = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      }
      else
	error_flag = TRUE;
    }
    else if (!strcmp(argv[i], "-summary"))
    {
      sprintf(tmp_str, "summary_print = TRUE;");
      TDRP_add_override(override, tmp_str);
      
      if (i < argc - 1)
	ltmp = atol(argv[i+1]);
      else
	ltmp = 0;
      
      if (ltmp > 0) 
      {
	sprintf(tmp_str, "summary_interval = %d;", ltmp);
	TDRP_add_override(override, tmp_str);
      }
    }
    else if (!strcmp(argv[i], "-params"))
    {
      if (i < argc - 1)
	*params_file_path_p = argv[i+1];
      else
	error_flag = TRUE;
    }
    else if (!strcmp(argv[i], "-check_params"))
    {
      *check_params_p = TRUE;
    }
    else if (!strcmp(argv[i], "-print_params"))
    {
      *print_params_p = TRUE;
    }
    
  }

  /*
   * print message if warning or error flag set
   */

  if(error_flag || warning_flag)
  {
    fprintf(stderr, "%s", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    Glob->params_path_name);
  }

  if (error_flag)
    tidy_and_exit(-1);

}
