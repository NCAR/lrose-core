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
 * parse_args.c: parse command line args, open files as required
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "ascii_to_xgraph.h"

void parse_args(int argc, char **argv)

{

  int error_flag = 0;
  int i;
  int malloc_debug_level;
  int ldummy;

  char *end_pt;
  char usage[BUFSIZ];
  char *malloc_debug_str;
  char *mode_str;
  char *debug_str;
  char *client_str;

  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	  "Usage: ", Glob->prog_name, " [options as below] \n",
	  "options:\n",
	  "       [ --, -h, -help, -man] produce this list.\n",
	  "       [ -client ] acegr or xgraph\n",
	  "       [ -debug ] print debug messages\n",
	  "       [ -mdebug level] set malloc debug level\n",
	  "       [ -mode ?] 'scatter', 'hist' or 'percentile'\n",
	  "       [ -noparams] use no parameters file\n",
	  "       [ -params path] specify alternative parameters file\n",
	  "       [ -title] title of graph\n",
	  "       [ -x ?] label of x col in data\n",
	  "       [ -y ?] label of y col in data\n",
	  "\n");

  debug_str = uGetParamString(Glob->prog_name, "debug", "false");

  malloc_debug_str = uGetParamString(Glob->prog_name,
				     "malloc_debug_level", "0");

  mode_str = uGetParamString(Glob->prog_name, "mode", MODE);
  client_str = uGetParamString(Glob->prog_name, "client", CLIENT);

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-man")) {

      fprintf(stderr, "%s", usage);
      exit(0);

    } else if (!strcmp(argv[i], "-client")) {

      if(i < argc - 1) {
	client_str = argv[i+1];
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug_str = (char *) "true";
      
    } else if (!strcmp(argv[i], "-mdebug")) {
	
      if (i < argc - 1)
	malloc_debug_str = argv[i+1];
      else
	error_flag = TRUE;
	
    } else if (!strcmp(argv[i], "-x")) {

      if(i < argc - 1) {
	Glob->xlabel = argv[i+1];
      } else {
	error_flag = TRUE;
      }

    } else if (!strcmp(argv[i], "-y")) {

      if(i < argc - 1) {
	Glob->ylabel = argv[i+1];
      } else {
	error_flag = TRUE;
      }

    } else if (!strcmp(argv[i], "-title")) {

      if(i < argc - 1) {
	Glob->title = argv[i+1];
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-mode")) {

      if(i < argc - 1) {
	mode_str = argv[i+1];
      } else {
	error_flag = TRUE;
      }
      
    } /* if */
    
  } /* i */

  /*
   * set debug option
   */
  
  uset_true_false_param(Glob->prog_name, "read_params",
			Glob->params_path_name,
			debug_str, &Glob->debug, "debug");
  /*
   * set client option
   */
  
  if (uset_double_param(Glob->prog_name,
			"parse_args",
			Glob->params_path_name,
			client_str, &Glob->client,
			"xgraph", XGRAPH,
			"acegr", ACEGR,
			"client"))
    exit(-1);

  /*
   * set mode option
   */
  
  if (uset_quad_param(Glob->prog_name,
		      "parse_args",
		      Glob->params_path_name,
		      mode_str, &Glob->mode,
		      "scatter", SCATTER_MODE,
		      "hist", HIST_MODE,
		      "log_hist", LOG_HIST_MODE,
		      "percentile", PERCENTILE_MODE,
		      "mode"))
    exit(-1);

  if (Glob->mode == PERCENTILE_MODE && Glob->n_percentiles == 0) {

    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr, "mode is 'percentile' but no percentiles declared.\n");
    fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
    exit(-1);

  }

  if (Glob->mode != SCATTER_MODE)
    Glob->perform_attrition = FALSE;
  
  /*
   * set the malloc debug level
   */

  errno = 0;
  ldummy = (double) strtol(malloc_debug_str, &end_pt, 10);

  if (errno == 0) {
    malloc_debug_level = ldummy;
  } else {
    malloc_debug_level = 0;
  }

  umalloc_debug ((int) malloc_debug_level);

  /*
   * print message if warning or error flag set
   */

  if(error_flag) {
    fprintf(stderr, "%s", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    Glob->params_path_name);
    exit(-1);
  }

}
