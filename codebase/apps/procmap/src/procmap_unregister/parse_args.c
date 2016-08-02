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
 * June 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <toolsa/os_config.h>
#include <toolsa/port.h>
#include <toolsa/str.h>

#include "procmap_unregister.h"

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
	  " -name pname -instance pinst -pid PID [options as below]\n"
	  "options:\n"
	  "       [ --, -h, -help, -man] produce this list.\n"
	  "       [-debug ] Turn on debug output\n"
	  "\n");
  
  /*
   * set defaults
   */
  
  Glob->name = "Procmap_test";
  Glob->instance = "Procmap_test";
  Glob->pid = (int)getpid();
  Glob->debug = FALSE;

  /*
   * look for command options
   */
  
  for (i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      fprintf(stderr, "%s", usage);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-instance"))
    {
      if (i < argc - 1)
	Glob->instance = argv[++i];
      else
	error_flag = TRUE;
    }
    else if (STRequal_exact(argv[i], "-name"))
    {
      if (i < argc - 1)
	Glob->name = argv[++i];
      else
	error_flag = TRUE;
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      Glob->debug = TRUE;
    }
    else if (STRequal_exact(argv[i], "-pid"))
    {
      if (i < argc - 1)
	Glob->pid = atoi(argv[++i]);
      else
	error_flag = TRUE;
    }
    
  }

  /*
   * print message if warning or error flag set
   */
  
  if (error_flag)
  {
    fprintf(stderr, "%s", usage);
    exit (-1);
  }
  
}
