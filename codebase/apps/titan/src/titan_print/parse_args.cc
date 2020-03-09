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
 * December 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "titan_print.h"

void parse_args(int argc, char **argv)

{
  
  int error_flag = 0;
  int warning_flag = 0;
  int i;
  int malloc_debug_level;
  long ldummy;
  char usage[BUFSIZ];
  char *malloc_debug_str, *end_pt;
  
  /*
   * set usage string
   */
  
  sprintf(usage, "%s%s%s%s",
	  "Usage: ", Glob->prog_name, " [options as below] file-name\n",
	  "options:\n"
	  "       [ -full ] provide a full listing (where applicable)\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -md ?] min. duration (secs) - print summary of tracks exceeding\n"
	  "                minimum duration (as applicable)\n"
	  "       [ -mdebug ?] set malloc debug level\n"
	  "       [ -summary ] provide a summary listing (where applicable)\n"
	  "       [ -tn ?] full listing for this track num (where applicable)\n"
	  "\n");
  
  if (argc < 2) {
    fprintf(stderr, "%s", usage);
    exit(1);
  }
  
  /*
   * set defaults
   */
  
  malloc_debug_str = (char *) "0";
  Glob->full = FALSE;
  Glob->summary = FALSE;
  Glob->min_duration = -1;
  Glob->track_num = -1;
  
  /*
   * set file name
   */
  
  Glob->file_name = argv[argc - 1];
  
  /*
   * look for command options
   */
  
  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-full")) {
      
      Glob->full = TRUE;
      
    } else if (!strcmp(argv[i], "-summary")) {
      
      Glob->summary = TRUE;
      
    } else if (!strcmp(argv[i], "-md")) {

      if(i < argc - 1) {
	errno = 0;
	ldummy = strtol(argv[i+1], &end_pt, 10);
	if (errno == 0)
	  Glob->min_duration = ldummy;
      } else {
	error_flag = TRUE;
      }
      
    } else if (!strcmp(argv[i], "-mdebug")) {
      
      if (i < argc - 1)
	malloc_debug_str = argv[i+1];
      else
	error_flag = TRUE;
      
    } else if (!strcmp(argv[i], "-tn")) {

      if(i < argc - 1) {
	Glob->track_num = atoi(argv[i+1]);
      } else {
	error_flag = TRUE;
      }
      
    }
    
  }
  
  /*
   * set the malloc debug level
   */
  
  errno = 0;
  ldummy = strtol(malloc_debug_str, &end_pt, 10);
  
  if (errno == 0)
    malloc_debug_level = ldummy;
  else
    malloc_debug_level = 0;
  
  umalloc_debug ((int) malloc_debug_level);
  
  /*
   * print message if warning or error flag set
   */
  
  if(error_flag || warning_flag) {
    fprintf(stderr, "%s", usage);
  }
  
  if (error_flag)
    exit(1);
  
}
