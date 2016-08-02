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
 * TimeHist.cc
 *
 * storm time history plotting program. This program takes its
 * input data from shared memory provided by and updated by
 * 'track_slave'. The program is intended to be spawned by
 * 'rview', and will terminate when rview sets the quit
 * semaphore.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1991
 * Converted to C++ - Feb 2001
 *
 * Mike Dixon
 *
 *********************************************************************/

#define MAIN
#include "TimeHist.hh"
#undef MAIN

#include <signal.h>
using namespace std;

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  path_parts_t progname_parts;

  /*
   * allocate space for the global structure
   */

  TimeHistGlobal glob;
  Glob = &glob;

  /*
   * set globals
   */

  Glob->argc = argc;
  Glob->argv = argv;

  /*
   * set program name
   */

  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *) umalloc
    ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);

  /*
   * check for print params
   */
  
  if (check_for_print_params(argc, argv)) {
    return 0;
  }

  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * set up system call environment
   */

  usystem_call_init();

  /*
   * register function to trap termination and interrupts
   */

  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);

  /*
   * X display and screen setup
   */

  setup_x();

  /*
   * load up params data base
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);

  /*
   * read the parameters or x data base
   */

  read_params();

  /* 
   * set SIGPIPE signal handler
   */
  
  PORTsignal(SIGPIPE, SIG_IGN);

  /*
   * parse the command line arguments, and open files as required
   */

  parse_args(argc, argv);

  /*
   * initialize process registration
   */

  PMU_auto_init(Glob->prog_name, Glob->instance,
		PROCMAP_REGISTER_INTERVAL);
  
  /*
   * set up access to track shared memory
   */

  setup_track_shmem();

  /*
   * set x synchronization
   */

  XSynchronize(Glob->rdisplay, Glob->x_sync);

  /*
   * create the frames of reference
   */

  create_frames();

  /*
   * set xfonts
   */

  set_xfonts();

  /*
   * get x color scales from color scale files
   */

  get_x_color_scales();

  /*
   * get postscript color (gray) scales from color scale files
   */

  get_ps_color_scales();

  /*
   * create graphic contexts
   */

  set_xgcs();

  /*
   * initialize modes
   */

  Glob->help = FALSE;

  /*
   * setup the x windows
   */

  setup_tscale_windows();
  setup_thist_windows();
  setup_timeht_windows();
  setup_rdist_windows();
  setup_union_windows();
  
  XMapRaised (Glob->rdisplay, Glob->tscale_window);
  XMapSubwindows (Glob->rdisplay, Glob->tscale_window);
  
  // map the active windows

  if (Glob->thist_active) {
    XMapSubwindows (Glob->rdisplay, Glob->thist_window);
    XMapRaised (Glob->rdisplay, Glob->thist_window);
  }
  if (Glob->timeht_active) {
    XMapSubwindows (Glob->rdisplay, Glob->timeht_window);
    XMapRaised (Glob->rdisplay, Glob->timeht_window);
  }
  if (Glob->rdist_active) {
    XMapRaised (Glob->rdisplay, Glob->rdist_window);
    XMapSubwindows (Glob->rdisplay, Glob->rdist_window);
  }
  if (Glob->union_active) {
    XMapRaised (Glob->rdisplay, Glob->union_window);
    XMapSubwindows (Glob->rdisplay, Glob->union_window);
  }
  Glob->windows_active = TRUE;
  
  /*
   * main event handling loop
   */

  event_loop();

  /*
   * tidy up
   */

  tidy_and_exit(0);

  return(0);

}

TimeHistGlobal::TimeHistGlobal()
{

  // clear data members

  memset(&start_c, 0, (char *) &end_c - (char *) &start_c);
  
}

TimeHistGlobal::~TimeHistGlobal()
{

}

