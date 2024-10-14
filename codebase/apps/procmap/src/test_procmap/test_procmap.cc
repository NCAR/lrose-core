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
/*************************************************************************
 * test_procmap.c :
 *
 * Tests the server mapper
 *
 * N. Rehak
 *
 * RAP NCAR Boulder Colorado USA
 *
 * October 1994
 *
 **************************************************************************/

#define MAIN
#include "test_procmap.hh"
#undef MAIN

#include <signal.h>

void ProcessExit(int sig);

int main(int argc, char **argv)

{

  int forever = TRUE;
  
  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    malloc((u_int) sizeof(global_t));
  
  /*
   * set program name
   */
  
  Glob->prog_name = (char *) "test_procmap";
  
  /*
   * parse command line arguments
   */

  parse_args(argc, argv);

  /*
   * set signal handlers
   */

  PORTsignal(SIGINT, ProcessExit);
  PORTsignal(SIGHUP, ProcessExit);
  PORTsignal(SIGTERM, ProcessExit);
  PORTsignal(SIGPIPE, SIG_IGN);

  /*
   * loop with the indicated sleep interval
   */

  while (forever) {

    fprintf(stdout, "Test_procmap name, instance: %s, %s\n",
	    Glob->name, Glob->instance);

    /*
     * query the server mapper and print the response
     */

    if(test_mapper()) {
      if (Glob->no_exit_flag)
	fprintf(stderr, "**** Registration failed for %s %s\n",
		Glob->name, Glob->instance);
      else
	return (-1);
    }

    fflush(stdout);

    /*
     * sleep the appropriate interval
     */
    
    if (Glob->do_repeat) {
      sleep((unsigned)Glob->repeat_int);
    } else {
      break;
    }
    
  } /* while (forever) */

  if (Glob->do_unregister)
    PMU_unregister(Glob->name, Glob->instance);
  
  return(0);

}

void ProcessExit(int sig)

{

  PMU_unregister(Glob->name, Glob->instance);
  exit(sig);

}

