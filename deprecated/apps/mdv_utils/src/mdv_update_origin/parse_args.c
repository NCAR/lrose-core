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
 * October 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <dataport/port_types.h>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>

#include "mdv_update_origin.h"

void parse_args(int argc,
		char **argv,
		int *check_params_p,
		int *print_params_p,
                char **params_file_path_p,
                tdrp_override_t *override,
		si32 *n_input_files_p,
		char ***input_file_list)
{
  int error_flag = 0;
  int i, j;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];

  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s",
	  "Usage: ",
	  Glob->prog_name,
	  " [options as below] -if input_file_list\n"
	  "options:\n"
	  "       [ --, -h, -help, -man, -usage] produce this list.\n"
	  "       [ -check_params ] check parameter usage\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -if input_file_list ] files to be converted\n"
	  "       [ -lat new_lat ] new latitude for files (dft = 0.0)\n"
	  "       [ -lon new_lon ] new longitude for files (dft = 0.0)\n"
	  "       [ -mdebug level ] malloc debug level\n"
	  "       [ -params path ] specify params file\n"
	  "       [ -print_params ] print parameter usage\n"
	  "\n"
	  " NOTE:  This program updates the indicated MDV files IN PLACE.\n"
	  "        If you need to keep the original files, copy them before\n"
	  "        executing this command.\n"
	  "\n");

  /*
   * initialize
   */

  *check_params_p = FALSE;
  *print_params_p = FALSE;
  *n_input_files_p = 0;
  TDRP_init_override(override);

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man") ||
	STRequal_exact(argv[i], "-usage"))
    {
      fprintf(stderr, "%s", usage);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-check_params"))
    {
      *check_params_p = TRUE;
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(override, tmp_str);
    }
    else if (STRequal_exact(argv[i], "-mdebug"))
    {
      if (i < argc - 1)
      {
	sprintf(tmp_str, "malloc_debug_level = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;
      }
      else
      {
	error_flag = TRUE;
      }
    }
    else if (STRequal_exact(argv[i], "-if"))
    {
      if (i < argc - 1)
      {
	/*
	 * search for next arg which starts with '-'
	 */

	for (j = i + 1; j < argc; j++)
	  if (argv[j][0] == '-')
	    break;
	
	/*
	 * compute number of files
	 */

	*n_input_files_p = j - i - 1;

	/*
	 * set file name array
	 */

	*input_file_list = argv + i + 1;
      }
      else
      {
	error_flag = TRUE;
      }
    }
    else if (STRequal_exact(argv[i], "-lat"))
    {
      if (i < argc - 1)
      {
	sprintf(tmp_str, "new_grid_origin_lat = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;
      }
      else
      {
        error_flag = TRUE;
      }
    }
    else if (STRequal_exact(argv[i], "-lon"))
    {
      if (i < argc - 1)
      {
	sprintf(tmp_str, "new_grid_origin_lon = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;
      }
      else
      {
        error_flag = TRUE;
      }
    }
    else if (STRequal_exact(argv[i], "-params"))
    {
      if (i < argc - 1)
      {
        *params_file_path_p = argv[i+1];
	i++;
      }
      else
      {
        error_flag = TRUE;
      }
    }
    else if (STRequal_exact(argv[i], "-print_params"))
    {
      *print_params_p = TRUE;
    } /* if */
    
  } /* i */

  /*
   * print message if error flag set
   */

  if (error_flag)
  {
    fprintf(stderr, "%s", usage);
    exit(-1);
  }

  return;

}

