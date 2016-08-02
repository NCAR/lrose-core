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

#include "polar_ingest.h"

void parse_args(int argc,
		char **argv)

{

  char usage[BUFSIZ];
  char *debug_str;
  char *device_str;

  int error_flag = 0;
  int warning_flag = 0;
  int i;
  long tmp;
 
  /*
   * set defaults
   */
  
  debug_str = uGetParamString(Glob->prog_name, "debug", DEBUG_STR);
  device_str = uGetParamString(Glob->prog_name, "device", DEVICE);

  Glob->filelist = FALSE;
  Glob->monitor = FALSE;
  Glob->header_print = FALSE;
  Glob->summary_print = FALSE;

  /*
   * set usage
   */

  sprintf(usage, "%s%s%s%s",
	  "Usage: ", argv[0], "\n",
	  "       [--, -h, -help, -man] produce this list.\n"
	  "       [-debug] print debug messages\n"
	  "       [-device ?] 'tape', 'tcpip', 'll_udp',"
	  "                   'ncar_udp' or 'fmq'\n"
	  "       [-filelist] list files only, overrides most other options\n"
	  "       [-header [n]] print header each n records (n default 360)\n"
	  "       [-mdebug ?] set malloc debug level\n"
	  "       [-monitor] monitor only, do not wait for clients\n"
	  "       [-noparams] use no params file\n"
	  "       [-params ?] set parameters file name\n"
	  "       [-summary [n]] print summary each n records (n default 1)\n"
	  "       [-udp_port ?] set udp port number\n"
	  "\n");

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") || !strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") || !strcmp(argv[i], "-man")) {

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
      
      if (i < argc - 1) {
	if (sscanf(argv[i+1], "%ld", &tmp) == 1) {
	  Glob->header_interval = tmp;
	}
      }
      
    } else if (!strcmp(argv[i], "-mdebug")) {
	
      if (i < argc - 1) {
	if (sscanf(argv[i+1], "%ld", &Glob->malloc_debug_level) != 1) {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-monitor")) {
	
      Glob->monitor = TRUE;
	
    } else if (!strcmp(argv[i], "-summary")) {
      
      Glob->summary_print = TRUE;
      
      if (i < argc - 1) {
	if (sscanf(argv[i+1], "%ld", &tmp) == 1) {
	  Glob->summary_interval = tmp;
	}
      }
      
    } else if (!strcmp(argv[i], "-udp_port")) {
	
      if (i < argc - 1) {
	if (sscanf(argv[i+1], "%d", &Glob->udp_port) != 1) {
	  error_flag = TRUE;
	}
      } else {
	error_flag = TRUE;
      }
      
    } /* if */

  } /* i */

  /*
   * set debug mode
   */

  if (!strcmp(debug_str, "true")) {
    Glob->debug = TRUE;
  } else if (!strcmp(debug_str, "false")) {
    Glob->debug = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr, "debug option '%s' not recognized.\n",
	    debug_str);
    fprintf(stderr,
	    "Valid options are 'true' or 'false'\n");
    fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
    tidy_and_exit(-1);
  }
  
  umalloc_debug (Glob->malloc_debug_level);

  /*
   * device type
   */

  if (!strcmp(device_str, "tape")) {
    Glob->device = TAPE_DEVICE;
  } else if (!strcmp(device_str, "ll_udp")) {
    Glob->device = LL_UDP_DEVICE;
  } else if (!strcmp(device_str, "ll_udp")) {
    Glob->device = LL_UDP_DEVICE;
  } else if (!strcmp(device_str, "ncar_udp")) {
    Glob->device = NCAR_UDP_DEVICE;
  } else if (!strcmp(device_str, "tcpip")) {
    Glob->device = TCPIP_DEVICE;
  } else if (!strcmp(device_str, "fmq")) {
    Glob->device = FMQ_DEVICE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr,
	    "Device option '%s' not recognized.\n", device_str);
    fprintf(stderr,
	    "Valid options: 'tape', 'tcpip', 'll_udp', 'ncar_udp' or 'fmq'\n");
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
