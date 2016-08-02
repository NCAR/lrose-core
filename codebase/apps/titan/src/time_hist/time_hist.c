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
 * time_hist.c
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
 *
 * Mike Dixon
 *
 *********************************************************************/

#define MAIN
#include "time_hist.h"
#undef MAIN

#include <signal.h>

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  path_parts_t progname_parts;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));
  memset ((void *)  Glob,
          (int) 0, (size_t) sizeof(global_t));

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

  PORTsignal(SIGTERM, (void (*)())tidy_and_exit);
  PORTsignal(SIGINT, (void (*)())tidy_and_exit);
  PORTsignal(SIGQUIT, (void (*)())tidy_and_exit);
  PORTsignal(SIGPIPE, (void (*)())handle_tserver_sigpipe);

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
  Glob->windows_active = TRUE;

  /*
   * map the windows
   */

  XMapRaised (Glob->rdisplay, Glob->tscale_window);
  XMapSubwindows (Glob->rdisplay, Glob->tscale_window);

  XMapSubwindows (Glob->rdisplay, Glob->thist_window);
  XMapRaised (Glob->rdisplay, Glob->thist_window);

  XMapSubwindows (Glob->rdisplay, Glob->timeht_window);
  XMapRaised (Glob->rdisplay, Glob->timeht_window);

  XMapRaised (Glob->rdisplay, Glob->rdist_window);
  XMapSubwindows (Glob->rdisplay, Glob->rdist_window);

  /*
   * set the interval timer going
   */

  set_timer();

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

