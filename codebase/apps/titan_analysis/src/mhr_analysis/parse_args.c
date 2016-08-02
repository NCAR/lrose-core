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
 * August 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "mhr_analysis.h"

void parse_args(int argc,
		char **argv)

{

  int error_flag = 0;
  int warning_flag = 0;
  int i;
  si32 ltmp;
 
  char usage[BUFSIZ];

  /*
   * set defaults
   */
  
  Glob->filelist = FALSE;
  Glob->header_print = FALSE;
  Glob->summary_print = FALSE;
  Glob->header_interval = HEADER_INTERVAL;
  Glob->summary_interval = SUMMARY_INTERVAL;
  Glob->tape_name = getenv("TAPE");
  Glob->analyze = TRUE;
  
  /*
   * set usage
   */

  sprintf(usage, "%s%s%s%s%s%s%s%s%s%s",
	  "Usage: ", argv[0], "\n",
	  "       [--, -help, -man, -usage] produce this list.\n",
	  "       [-f ?] tape device name\n",
	  "       [-filelist] list files only, overrides most other options\n",
	  "       [-header [n]] print header each n records (n default 360)\n",
	  "       [-no_anal] no analysis\n",
	  "       [-summary [n]] print summary each n records (n default 1)\n",
	  "\n");
  
  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man") ||
	!strcmp(argv[i], "-usage")) {

      fprintf(stderr, "%s", usage);
      exit(0);

    } else if (!strcmp(argv[i], "-f")) {
	
      if (i < argc - 1)
	Glob->tape_name = argv[i+1];
      else
	error_flag = TRUE;
	
    } else if (!strcmp(argv[i], "-filelist")) {
	
      Glob->filelist = TRUE;
      Glob->analyze = FALSE;
	
    } else if (!strcmp(argv[i], "-header")) {
      
      Glob->header_print = TRUE;
      
      if (i < argc - 1)
	ltmp = atol(argv[i+1]);
      else
	ltmp = 0;
      
      if (ltmp > 0) 
	Glob->header_interval = ltmp;
      
    } else if (!strcmp(argv[i], "-summary")) {
      
      Glob->summary_print = TRUE;
      
      if (i < argc - 1)
	ltmp = atol(argv[i+1]);
      else
	ltmp = 0;
      
      if (ltmp > 0) 
	Glob->summary_interval = ltmp;
      
    } else if (!strcmp(argv[i], "-no_anal")) {
      
      Glob->analyze = FALSE;
      
    }

  }

  /*
   * print message if warning or error flag set
   */

  if(error_flag || warning_flag) {
    fprintf(stderr, "%s", usage);
  }

  if (error_flag)
    tidy_and_exit(-1);

}
