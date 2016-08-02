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
 * create_frames.c - TimeHist routine
 *
 * create the world-coordinate frames of reference
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
using namespace std;

void create_frames(void)

{

  si32 i;

  if (Glob->debug) {
    fprintf(stderr, "** create_frames **\n");
  }

  /*
   * set up time scale frames
   */

  Glob->tscale_title_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);

  umalloc_verify();

  Glob->tscale_title_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->tscale_plot_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->tscale_plot_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  
  Glob->tscale_button_frame = (gframe_t **)
    umalloc((ui32)N_TSCALE_BUTTONS * sizeof(gframe_t *));

  for (i = 0; i < N_TSCALE_BUTTONS; i++) {
    Glob->tscale_button_frame[i] = GCreateFrame(0.0, 0.0, 1.0, 1.0);
    Glob->tscale_button_frame[i]->x->gc =
      XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  }

  /*
   * set up time history frames
   */

  Glob->thist_title_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);

  Glob->thist_title_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->thist_plot_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->thist_plot_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  
  Glob->thist_button_frame = (gframe_t **)
    umalloc((ui32)N_THIST_BUTTONS * sizeof(gframe_t *));

  for (i = 0; i < N_THIST_BUTTONS; i++) {
    Glob->thist_button_frame[i] = GCreateFrame(0.0, 0.0, 1.0, 1.0);
    Glob->thist_button_frame[i]->x->gc =
      XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  }

  /*
   * set up time height profile frames
   */

  Glob->timeht_title_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);

  Glob->timeht_title_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->timeht_plot_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->timeht_plot_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  
  Glob->timeht_scale_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->timeht_scale_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->timeht_button_frame = (gframe_t **)
    umalloc((ui32)N_TIMEHT_BUTTONS * sizeof(gframe_t *));

  for (i = 0; i < N_TIMEHT_BUTTONS; i++) {
    Glob->timeht_button_frame[i] = GCreateFrame(0.0, 0.0, 1.0, 1.0);
    Glob->timeht_button_frame[i]->x->gc =
      XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  }

  /*
   * set up reflectivity distribution frames
   */

  Glob->rdist_title_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);

  Glob->rdist_title_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->rdist_plot_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->rdist_plot_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  
  Glob->rdist_scale_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->rdist_scale_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->rdist_button_frame = (gframe_t **)
    umalloc((ui32)N_RDIST_BUTTONS * sizeof(gframe_t *));

  for (i = 0; i < N_RDIST_BUTTONS; i++) {
    Glob->rdist_button_frame[i] = GCreateFrame(0.0, 0.0, 1.0, 1.0);
    Glob->rdist_button_frame[i]->x->gc =
      XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  }

  /*
   * set up union data frames
   */

  Glob->union_title_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);

  Glob->union_title_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->union_plot_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->union_plot_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  
  Glob->union_button_frame = (gframe_t **)
    umalloc((ui32)N_UNION_BUTTONS * sizeof(gframe_t *));

  for (i = 0; i < N_UNION_BUTTONS; i++) {
    Glob->union_button_frame[i] = GCreateFrame(0.0, 0.0, 1.0, 1.0);
    Glob->union_button_frame[i]->x->gc =
      XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  }

  /*
   * set up help frames
   */

  Glob->help_title_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->help_title_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->help_text_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->help_text_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  
  Glob->help_button_frame = (gframe_t **)
    umalloc((ui32) N_HELP_BUTTONS * sizeof(gframe_t *));

  for (i = 0; i < N_HELP_BUTTONS; i++) {
    Glob->help_button_frame[i] = GCreateFrame(0.0, 0.0, 1.0, 1.0);
    Glob->help_button_frame[i]->x->gc =
      XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  }

  /*
   * setup page description frames
   */
  
  Glob->vert_page_frame =
    GCreateFrame(0.0, 0.0, Glob->ps_page_width, Glob->ps_page_length);

  Glob->horiz_page_frame =
    GCreateFrame(0.0, 0.0, Glob->ps_page_length, Glob->ps_page_width);

  if (Glob->debug == TRUE) {

    fprintf(stderr, "tscale_title_frame gc = %ld\n",
	    (long) Glob->tscale_title_frame->x->gc);

    fprintf(stderr, "tscale_plot_frame gc = %ld\n",
	    (long) Glob->tscale_plot_frame->x->gc);

    for (i = 0; i < N_TSCALE_BUTTONS; i++)
      fprintf(stderr, "tscale_button_frame gc[%d] = %ld\n", i,
	      (long) Glob->tscale_button_frame[i]->x->gc);

    fprintf(stderr, "thist_title_frame gc = %ld\n",
	    (long) Glob->thist_title_frame->x->gc);

    fprintf(stderr, "thist_plot_frame gc = %ld\n",
	    (long) Glob->thist_plot_frame->x->gc);

    for (i = 0; i < N_THIST_BUTTONS; i++)
      fprintf(stderr, "thist_button_frame gc[%d] = %ld\n", i,
	      (long) Glob->thist_button_frame[i]->x->gc);

    fprintf(stderr, "timeht_title_frame gc = %ld\n",
	    (long) Glob->timeht_title_frame->x->gc);

    fprintf(stderr, "timeht_plot_frame gc = %ld\n",
	    (long) Glob->timeht_plot_frame->x->gc);

    fprintf(stderr, "timeht_scale_frame gc = %ld\n",
	    (long) Glob->timeht_scale_frame->x->gc);

    for (i = 0; i < N_TIMEHT_BUTTONS; i++)
      fprintf(stderr, "timeht_button_frame gc[%d] = %ld\n", i,
	      (long) Glob->timeht_button_frame[i]->x->gc);

    fprintf(stderr, "rdist_title_frame gc = %ld\n",
	    (long) Glob->rdist_title_frame->x->gc);

    fprintf(stderr, "rdist_plot_frame gc = %ld\n",
	    (long) Glob->rdist_plot_frame->x->gc);

    fprintf(stderr, "rdist_scale_frame gc = %ld\n",
	    (long) Glob->rdist_scale_frame->x->gc);

    for (i = 0; i < N_RDIST_BUTTONS; i++)
      fprintf(stderr, "rdist_button_frame gc[%d] = %ld\n", i,
	      (long) Glob->rdist_button_frame[i]->x->gc);

    fprintf(stderr, "union_title_frame gc = %ld\n",
	    (long) Glob->union_title_frame->x->gc);

    fprintf(stderr, "union_plot_frame gc = %ld\n",
	    (long) Glob->union_plot_frame->x->gc);

    for (i = 0; i < N_UNION_BUTTONS; i++)
      fprintf(stderr, "union_button_frame gc[%d] = %ld\n", i,
	      (long) Glob->union_button_frame[i]->x->gc);

    fprintf(stderr, "help_title_frame gc = %ld\n",
	    (long) Glob->help_title_frame->x->gc);

    fprintf(stderr, "help_text_frame gc = %ld\n",
	    (long) Glob->help_text_frame->x->gc);

    for (i = 0; i < N_HELP_BUTTONS; i++)
      fprintf(stderr, "help_button_frame gc[%d] = %ld\n", i,
	      (long) Glob->help_button_frame[i]->x->gc);

  } /* if (Glob->debug == TRUE) */

}
