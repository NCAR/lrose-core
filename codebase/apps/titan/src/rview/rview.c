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
 * rview.c: rview plotting program
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#define MAIN
#include "rview.h"
#undef MAIN

#include <signal.h>

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  path_parts_t progname_parts;
  zoom_t *zoom;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));
  memset ((void *)  Glob,
          (int) 0, (size_t) sizeof(global_t));

  /*
   * set globals to command line args
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
   * initialize flags
   */
  
  memset ((void *)  Glob->zoom,
          (int) 0, (size_t) (NZOOM * sizeof(zoom_t)));
  memset ((void *)  &Glob->vsection,
          (int) 0, (size_t) sizeof(vsection_t));

  Glob->help = FALSE;
  Glob->zoom_level = 0;
  Glob->main_scale_invalid = TRUE;
  Glob->cappi_title_invalid = TRUE;
  Glob->vsection_title_invalid = TRUE;
  Glob->cappi_requires_expose = TRUE;
  Glob->vsection_requires_expose = TRUE;
  Glob->track_shmem = NULL;
  Glob->plot_composite = FALSE;
  Glob->pq_handle = NULL;
  
 /*
   * register function to trap termination and interrupts
   */

  PORTsignal(SIGQUIT, (void (*)())tidy_and_exit);
  PORTsignal(SIGTERM, (void (*)())tidy_and_exit);
  PORTsignal(SIGINT, (void (*)())tidy_and_exit);
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
   * initialize level 0 zoom state
   */

  Glob->zoom_level = 0;
  zoom = Glob->zoom + Glob->zoom_level;
  
  zoom->min_x = Glob->full_min_x;
  zoom->max_x = Glob->full_max_x;
  zoom->min_y = Glob->full_min_y;
  zoom->max_y = Glob->full_max_y;
  
  zoom->range_x = zoom->max_x - zoom->min_x;
  zoom->range_y = zoom->max_y - zoom->min_y;

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
   * read in field control data
   */

  read_field_control();

  /*
   * read in map file
   */

  read_map_files();

  /*
   * get the track data shared memory system going, if it is
   * needed
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
   * get contour intervals
   */

  get_contour_intervals();

  /*
   * create graphic contexts
   */

  set_xgcs();

  /*
   * setup the x windows
   */

  setup_cappi_windows();

  /*
   * input event solicitation
   */

  set_cappi_sens();

  /*
   * map the cappi windows
   */

  XMapRaised (Glob->rdisplay, Glob->main_window);
  XMapSubwindows (Glob->rdisplay, Glob->main_window);
  safe_XFlush(Glob->rdisplay);

  /*
   * set the interval timer going
   */

  set_timer();

  /*
   * main event handling loop
   */

  event_loop();

  /*
   * disable the alarm signal
   */

  PORTsignal(SIGALRM, SIG_IGN);

  /*
   * tidy up
   */

  tidy_and_exit(0);

  return(0);

}
