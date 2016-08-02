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
#include <time.h>
#include <unistd.h>

#include <toolsa/os_config.h>
#include <toolsa/port.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>

#include "procmap_register.h"

void parse_args(int argc, char **argv)
{
  int error_flag = 0;
  int i;
  char usage[BUFSIZ];
  
  /*
   * set usage string
   */
  
  sprintf(usage,
	  "Usage: %s [options as below]\n"
	  "options:\n"
	  "       [ --, -h, -help, -man] produce this list.\n"
	  "       [-instance ?] test instance (default Procmap_test)\n"
	  "       [-name ?] test name (default Procmap_test)\n"
	  "       [-pid ?] process PID (default this process)\n"
	  "       [-reg_int ?] register interval in seconds (default %d)\n"
	  "       [-start ?] start time in UNIX format (default now)\n"
	  "       [-status ?] process status (default 0)\n"
	  "       [-status_str ?] process status string (default OK)\n"
	  "\n",
 	  Glob->prog_name,
	  PROCMAP_REGISTER_INTERVAL);
  
  /*
   * set defaults
   */
  
  Glob->name = "Procmap_test";
  Glob->instance = "Procmap_test";
  Glob->pid = (int)getpid();
  Glob->reg_interval = PROCMAP_REGISTER_INTERVAL;
  Glob->start_time = time((time_t *)NULL);
  Glob->status = 0;
  Glob->status_str = "OK";
  
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
    else if (STRequal_exact(argv[i], "-pid"))
    {
      if (i < argc - 1)
	Glob->pid = atoi(argv[++i]);
      else
	error_flag = TRUE;
    }
    else if (STRequal_exact(argv[i], "-reg_int"))
    {
      if (i < argc - 1)
	Glob->reg_interval = atoi(argv[++i]);
      else
	error_flag = TRUE;
    }
    else if (STRequal_exact(argv[i], "-start"))
    {
      if (i < argc - 1)
	Glob->start_time = atoi(argv[++i]);
      else
	error_flag = TRUE;
    }
    else if (STRequal_exact(argv[i], "-status"))
    {
      if (i < argc - 1)
	Glob->status = atoi(argv[++i]);
      else
	error_flag = TRUE;
    }
    else if (STRequal_exact(argv[i], "-status_str"))
    {
      if (i < argc - 1)
	Glob->status_str = argv[++i];
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
