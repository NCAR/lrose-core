// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * parse_args.cc
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1998
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "vergrid_spdb2symprod.hh"

/*******************************************************************
 * parse_args() - Parse the command line arguments.
 */

void parse_args(char *prog_name, int argc, char **argv,
		int *check_params_p,
		int *print_params_p,
		char **params_file_path_p,
		tdrp_override_t *override)

{

  int error_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];
  
  // Initialize the returned values

  *check_params_p = FALSE;
  *print_params_p = FALSE;
  *params_file_path_p = NULL;
  
  // load up usage string

  sprintf(usage, "%s%s%s",
	  "Usage:\n\n", prog_name, " [options] as below:\n\n"
	  "       [ --, -help, -man] produce this list.\n"
	  "       [ -check_params] check parameter usage\n"
	  "       [ -print_params] print parameter usage\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -mdebug level] set malloc debug level\n"
	  "       [ -params name] parameters file name\n"
	  "       [ -data data_dir] input data directory\n"
	  "       [ -port port] output port number\n"
	  "\n");

  // initialize

  *check_params_p = FALSE;
  TDRP_init_override(override);
  
  // search for command options
  
  for (i = 1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      printf("%s", usage);
      tidy_and_exit(1);
    }
    else if (STRequal_exact(argv[i], "-check_params"))
    {
      *check_params_p = TRUE;
    }
    else if (STRequal_exact(argv[i], "-print_params"))
    {
      *print_params_p = TRUE;
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = DEBUG_MSGS;");
      TDRP_add_override(override, tmp_str);
    }
    else if (i < argc - 1)
    {
      if (STRequal_exact(argv[i], "-params"))
      {
	*params_file_path_p = argv[i+1];
	i++;
      }
      else if (STRequal_exact(argv[i], "-mdebug"))
      {
	sprintf(tmp_str, "malloc_debug_level = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;
      }
      else if (STRequal_exact(argv[i], "-data"))
      {
	sprintf(tmp_str, "database_dir = \"%s\";", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;
      }
      else if (STRequal_exact(argv[i], "-port"))
      {
	sprintf(tmp_str, "port = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;
      }
      
    } /* if (i < argc - 1) */
    
  } /* i */

  // print usage if there was an error

  if (error_flag)
  {
    fprintf(stderr, "%s\n", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    *params_file_path_p);
  }

  if (error_flag)
    tidy_and_exit(1);

  return;
}


