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
 * parse_args.c - TimeHist routine
 *
 * parse command line args, open files as required
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
#include "params.hh"
#include <toolsa/DateTime.hh>
#include <iostream>
using namespace std;

void parse_args(int argc, char **argv)

{

  int error_flag = 0;
  int warning_flag = 0;
  int i;

  char *debug_str;
  char usage[BUFSIZ];

  /*
   * load up usage string
   */

  sprintf(usage, "%s%s%s%s",
	  "Usage:\n\n", argv[0], " [options] as below:\n\n",
	  "       [ --, -h, -help, -man] produce this list.\n"
	  "       [ -bg, -background color] set background color\n"
	  "       [ -d, -display name] display name\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -fg, -foreground color] set foreground color\n"
	  "       [ -instance ?] program instance\n"
	  "       [ -noparams] use X data base instead of params file\n"
	  "       [ -params name] paramteres file name\n"
	  "       [ -print_params] print parameters\n"
	  "\n");

  Glob->instance = uGetParamString(Glob->prog_name,
				   "instance", "Test");
  
  debug_str = uGetParamString(Glob->prog_name,
			      "debug", "false");
  
  /*
   * set debug option
   */
  
  if (!strcmp(debug_str, "yes") || !strcmp(debug_str, "true")) {
    Glob->debug = TRUE;
  } else if (!strcmp(debug_str, "no") || !strcmp(debug_str, "false")) {
    Glob->debug = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr, "Debug option '%s' not recognized.\n",
	    debug_str);
    error_flag = 1;
  }
  
  /*
   * search for command options
   */

  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-man") ||
	!strcmp(argv[i], "--")) {

      printf("%s", usage);
      tidy_and_exit(0);

    } else if (!strcmp(argv[i], "-debug")) {
      
      Glob->debug = TRUE;
	
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	Glob->instance = argv[i+1];
      } else {
	error_flag = TRUE;
      }

    } else if (!strcmp(argv[i], "-foreground") ||
	       !strcmp(argv[i], "-fg")) {
	
      if (i < argc - 1) {
	Glob->foregroundstr = (char *)
	  umalloc((ui32)strlen(argv[i+1]) + 1);
	strcpy(Glob->foregroundstr, argv[i+1]);
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-background") ||
	       !strcmp(argv[i], "-bg")) {
	
      if (i < argc - 1) {
	Glob->backgroundstr = (char *) 
	  umalloc((ui32)strlen(argv[i+1]) + 1);
	strcpy(Glob->backgroundstr, argv[i+1]);
      } else {
	error_flag = TRUE;
      }
	
    } /* if */
    
  } /* i */

  if(error_flag || warning_flag) {
    fprintf(stderr, "%s\n", usage);
    fprintf(stderr, "Check the paramteres file '%s'.\n\n",
	    Glob->params_path_name);
  }
  
  if (error_flag)
    tidy_and_exit(1);
  
}

//////////////////////////////
// check for -print_params arg

int check_for_print_params(int argc, char **argv)

{

  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "-print_params")) {
      cout << "#################################################################" << endl;
      cout << "#  Example parameters file for TimeHist\n";
      cout << "#" << endl;
      cout << "#  " << DateTime::str() << endl;
      cout << "#################################################################" << endl;
      cout << endl;
      cout << ParamsText;
      return -1;
    }
    
  }

  return 0;

}
