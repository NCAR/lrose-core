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
#include "Rview.hh"
#undef MAIN

#include <signal.h>
using namespace std;

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

  RviewGlobal glob;
  Glob = &glob;

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

  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);

  /*
   * load up params data base
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);

  /*
   * read the parameters
   */

  read_params();

  /* 
   * set SIGPIPE signal handler
   */
  
  PORTsignal(SIGPIPE, SIG_IGN);

  /*
   * read in products params file
   */

  if (Glob->plot_products) {
    char *params_path = Glob->product_params_path_name;
    if (Glob->_prodParams.loadApplyArgs(params_path, argc, argv,
					NULL, true)) {
      fprintf(stderr, "ERROR: %s\n", Glob->prog_name);
      fprintf(stderr, "  Problem with TDRP parameters for products\n");
      fprintf(stderr, "  Params path: %s\n", params_path);
      return -1;
    }
    if (Glob->_prodParams.exitDeferred()) {
      exit(0);
    }
  }
  
  /*
   * X display and screen setup
   */

  setup_x();

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
  Glob->_titanLdata.setDirFromUrl(Glob->titan_server_url);
  
  /*
   * create the product manager
   */

  if (Glob->plot_products) {
    Glob->_prodMgr = new ProductMgr(Glob->_prodParams, Glob->rdisplay);
  }

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
  XFlush(Glob->rdisplay);

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

RviewGlobal::RviewGlobal()
{

  // clear data members

  memset(&start_c, 0, (char *) &end_c - (char *) &start_c);
  
  memset (zoom, 0, NZOOM * sizeof(zoom_t));
  memset (&vsection, 0, sizeof(vsection_t));

  this->time = std::time(NULL);
  debug = FALSE;
  localtime = FALSE;
  
  help = FALSE;
  zoom_level = 0;
  main_scale_invalid = TRUE;
  cappi_title_invalid = TRUE;
  vsection_title_invalid = TRUE;
  cappi_requires_expose = TRUE;
  vsection_requires_expose = TRUE;
  track_shmem = NULL;
  plot_composite = FALSE;
  //  pq_handle = NULL;
  _prodMgr = NULL;

}

RviewGlobal::~RviewGlobal()
{
  if (Glob->_prodMgr) {
    delete Glob->_prodMgr;
  }
}

