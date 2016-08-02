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
 * procmap_unregister.c :
 *
 * Un-registers a process with the process mapper - used by PERL scripts to
 * unregister.
 *
 * N. Rehak
 *
 * RAP NCAR Boulder Colorado USA
 *
 * June 1997
 *
 **************************************************************************/

#include <stdio.h>
#include <signal.h>

#include <toolsa/os_config.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>

#define MAIN
#include "procmap_unregister.h"
#undef MAIN


int main(int argc, char **argv)
{
  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    umalloc((u_int) sizeof(global_t));
  
  /*
   * set program name
   */
  
  Glob->prog_name = "procmap_unregister";
  
  /*
   * parse command line arguments
   */

  parse_args(argc, argv);

  /*
   * set signal handlers
   */

  PORTsignal(SIGPIPE, SIG_IGN);

  /*
   * Register with the process mapper
   */

  if (Glob->debug) {
    fprintf(stderr, "Unregistering name, instance, pid: %s, %s, %d\n",
	    Glob->name, Glob->instance, Glob->pid);
  }
  PMU_unregister_pid(Glob->name, Glob->instance, Glob->pid);

  return(0);
}
