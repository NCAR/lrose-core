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
 * December 1990
 *
 * Mike Dixon
 *
 * Modification History:
 *
 *  4 Dec 92   N. Rehak    Modified for use by test_procmap
 *
 *********************************************************************/

#include "test_procmap.hh"

void parse_args(int argc, char **argv)
     
{
  
  int error_flag = 0;
  int i;
  char usage[BUFSIZ];
  
  /*
   * set usage string
   */
  
  sprintf(usage, "%s%s%s",
	  "Usage: ",
	  Glob->prog_name,
	  " [options as below]\n"
	  "options:\n"
	  "       [ --, -h, -help, -man] produce this list.\n"
	  "       [-c n] provide a continuous testing,\n"
	  "                 repeated every n seconds.\n"
	  "       [-host ?] procmap host (default local)\n"
	  "       [-instance ?] test instance (default Procmap_test)\n"
	  "       [-reg_int n] max registration interval,\n"
	  "       [-name ?] test name (default Procmap_test)\n"
	  "       [-no_exit ] don't exit on an error\n"
	  "       [-no_register] do not register, get info only\n"
	  "       [-no_unregister] do not unregister, used by PERL scripts\n"
	  "\n");
  
  /*
   * set defaults
   */
  
  Glob->procmap_host = PORThostname();
  Glob->name = "Procmap_test";
  Glob->instance = "Procmap_test";
  Glob->do_register = TRUE;
  Glob->do_unregister = TRUE;
  Glob->do_repeat = FALSE;
  Glob->repeat_int = 0;
  Glob->no_exit_flag = FALSE;
  
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
      
    } else if (strcmp(argv[i], "-c") == 0) {
      
      if (i < argc - 1) {
	Glob->do_repeat = TRUE;
	Glob->repeat_int = atoi(argv[++i]);
      } else {
	error_flag = TRUE;
      }
      
    } else if (strcmp(argv[i], "-reg_int") == 0) {
      
      if (i < argc - 1) {
	Glob->max_reg_int = atoi(argv[++i]);
      } else {
	error_flag = TRUE;
      }
      
    } else if (strcmp(argv[i], "-host") == 0) {
      
      if (i < argc - 1) {
	Glob->procmap_host = argv[++i];
      } else {
	error_flag = TRUE;
      }
      
    } else if (strcmp(argv[i], "-instance") == 0) {
      
      if (i < argc - 1) {
	Glob->instance = argv[++i];
      } else {
	error_flag = TRUE;
      }
      
    } else if (strcmp(argv[i], "-name") == 0) {
      
      if (i < argc - 1) {
	Glob->name = argv[++i];
      } else {
	error_flag = TRUE;
      }
      
    } else if (strcmp(argv[i], "-no_exit") == 0) {
      
      Glob->no_exit_flag = TRUE;
      
    } else if (strcmp(argv[i], "-no_register") == 0) {
      
      Glob->do_register = FALSE;
      
    } else if (strcmp(argv[i], "-no_unregister") == 0) {
      
      Glob->do_unregister = FALSE;
      
    }
    
  }

  /*
   * print message if warning or error flag set
   */
  
  if(error_flag) {
    fprintf(stderr, "%s", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    Glob->param_path_name);
    exit (-1);
  }
  
}
