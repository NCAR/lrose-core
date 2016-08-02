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

#include "polar2gate.h"

void parse_args(int argc, char **argv)

{

  int error_flag = 0;
  int warning_flag = 0;
  int i;
  int malloc_debug_level;
  si32 ltmp;
 
  char usage[BUFSIZ];
  char *debug_str, *malloc_debug_str;
  char *device_str;
  char *end_pt;

  /*
   * set defaults
   */
  
  debug_str = uGetParamString(Glob->prog_name, "debug", DEBUG_STR);
  
  malloc_debug_str = uGetParamString(Glob->prog_name,
				     "malloc_debug_level", "0");

  device_str = uGetParamString(Glob->prog_name, "device", DEVICE);

  Glob->filelist = FALSE;
  Glob->header_print = FALSE;
  Glob->summary_print = FALSE;

  /*
   * set usage
   */

  sprintf(usage, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	  "Usage: ", argv[0], "\n",
	  "       [--, -help, -man] produce this list.\n",
	  "       [-debug] print debug messages\n",
	  "       [-device ?] 'tape' or 'disk'\n",
	  "       [-device_name ?] tape or file name\n",
	  "       [-filelist] list files only, overrides most other options\n",
	  "       [-header [n]] print header each n records (n default 360)\n",
	  "       [-mdebug ?] set malloc debug level\n",
	  "       [-noparams] use no params file\n",
	  "       [-params ?] set parameters file name\n",
	  "       [-summary [n]] print summary each n records (n default 1)\n",
	  "\n");

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {

      fprintf(stderr, "%s", usage);
      exit(0);

    } else if (!strcmp(argv[i], "-debug")) {

      debug_str = "true";

    } else if (!strcmp(argv[i], "-device")) {
	
      if (i < argc - 1)
	device_str = argv[i+1];
      else
	error_flag = TRUE;
	
    } else if (!strcmp(argv[i], "-filelist")) {
	
      Glob->filelist = TRUE;
	
    } else if (!strcmp(argv[i], "-header")) {
      
      Glob->header_print = TRUE;
      
      if (i < argc - 1)
	ltmp = atol(argv[i+1]);
      else
	ltmp = 0;
      
      if (ltmp > 0) 
	Glob->header_interval = ltmp;
      
    } else if (!strcmp(argv[i], "-mdebug")) {
	
      if (i < argc - 1)
	malloc_debug_str = argv[i+1];
      else
	error_flag = TRUE;
	
    } else if (!strcmp(argv[i], "-summary")) {
      
      Glob->summary_print = TRUE;
      
      if (i < argc - 1)
	ltmp = atol(argv[i+1]);
      else
	ltmp = 0;
      
      if (ltmp > 0) 
	Glob->summary_interval = ltmp;
      
    } else if (!strcmp(argv[i], "-device_name")) {
	
      if (i < argc - 1)
	Glob->device_name = argv[i+1];
      else
	error_flag = TRUE;
	
    }

  }

  /*
   * set debug mode
   */
  
  if (!strcmp(debug_str, "yes") ||
      !strcmp(debug_str, "true")) {
    Glob->debug = TRUE;
  } else if (!strcmp(debug_str, "no") ||
	     !strcmp(debug_str, "false")) {
    Glob->debug = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr, "debug option '%s' not recognized.\n",
	    debug_str);
    fprintf(stderr,
	    "Valid options are 'true', 'yes', 'false' or 'no'\n");
    fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
    tidy_and_exit(-1);
  }
  
  /*
   * set the malloc debug level
   */

  errno = 0;
  ltmp = (double) strtol(malloc_debug_str, &end_pt, 10);

  if (errno == 0)
    malloc_debug_level = ltmp;
  else
    malloc_debug_level = 0;

  umalloc_debug ((int) malloc_debug_level);

  /*
   * device type
   */

  if (!strcmp(device_str, "tape")) {
    Glob->device = TAPE_DEVICE;
  } else if (!strcmp(device_str, "disk")) {
    Glob->device = DISK_DEVICE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr,
	    "Device option '%s' not recognized.\n", device_str);
    fprintf(stderr,
	    "Valid options are 'tape' or 'disk'\n");
    fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
    tidy_and_exit(-1);
  }

  /*
   * override header types in case of tape operations
   */

  if (Glob->device == TAPE_DEVICE) {

    if (Glob->input_format == NCAR_FORMAT)
      Glob->header_type = RP7_HEADER;
    else if (Glob->input_format == CHILL_FORMAT)
      Glob->header_type = CHILL_HEADER;

  } /* if (Glob->device == TAPE_DEVICE) */

  if (Glob->header_type == CHILL_HEADER &&
      Glob->nfields_out > MAX_CHILL_FIELDS) {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr,
	    "Too many fields for chill data - max %d\n",
	    MAX_CHILL_FIELDS);
    fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
    tidy_and_exit(-1);
  }

  /*
   * print message if warning or error flag set
   */

  if(error_flag || warning_flag) {
    fprintf(stderr, "%s", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    Glob->params_path_name);
  }

  if (error_flag)
    tidy_and_exit(-1);

}
