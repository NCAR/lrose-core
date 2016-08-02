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
 * create_frames.c: create the world-coordinate frames
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
using namespace std;

void create_frames()

{

  double ps_pagewidth, ps_pagelength;
  si32 i;

  if (Glob->debug) {
    fprintf(stderr, "** create_frames **\n");
  }

  /*
   * get page size from the parameters resources
   */

  ps_pagewidth = uGetParamDouble(Glob->prog_name,
			      "ps_pagewidth", PS_PAGEWIDTH);
  ps_pagelength = uGetParamDouble(Glob->prog_name,
			       "ps_pagelength", PS_PAGELENGTH);

  /*
   * set up cappi frames
   */

  Glob->cappi_title_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);

  Glob->cappi_title_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->cappi_plot_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->cappi_plot_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  
  Glob->cappi_ps_plot_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);

  Glob->cappi_button_frame = (gframe_t **)
    umalloc((ui32)N_CAPPI_BUTTONS * sizeof(gframe_t *));

  for (i = 0; i < N_CAPPI_BUTTONS; i++) {
    Glob->cappi_button_frame[i] = GCreateFrame(0.0, 0.0, 1.0, 1.0);
    Glob->cappi_button_frame[i]->x->gc =
      XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  }

  /*
   * set up vsection frames
   */

  Glob->vsection_title_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->vsection_title_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

  Glob->vsection_plot_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->vsection_plot_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  
  Glob->vsection_ps_plot_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);

  Glob->vsection_button_frame = (gframe_t **)
    umalloc((ui32) N_VSECTION_BUTTONS * sizeof(gframe_t *));

  for (i = 0; i < N_VSECTION_BUTTONS; i++) {
    Glob->vsection_button_frame[i] = GCreateFrame(0.0, 0.0, 1.0, 1.0);
    Glob->vsection_button_frame[i]->x->gc =
      XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);
  }

  /*
   * setup main scale frame
   */

  Glob->main_scale_frame = GCreateFrame(0.0, 0.0, 1.0, 1.0);
  Glob->main_scale_frame->x->gc =
    XCreateGC(Glob->rdisplay, DefaultRootWindow(Glob->rdisplay), 0, 0);

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
  
  Glob->vert_page_frame = GCreateFrame(0.0, 0.0,
				       ps_pagewidth, ps_pagelength);

  Glob->horiz_page_frame = GCreateFrame(0.0, 0.0,
					ps_pagelength, ps_pagewidth);

  /*
   * debug output of GC Id's
   */

  if (Glob->debug) {

    fprintf(stderr, "cappi_title_frame gc = %ld\n",
	    (long) Glob->cappi_title_frame->x->gc);
    
    fprintf(stderr, "cappi_plot_frame gc = %ld\n",
	    (long) Glob->cappi_plot_frame->x->gc);
    
    for (i = 0; i < N_CAPPI_BUTTONS; i++)
      fprintf(stderr, "cappi_button_frame gc[%d] = %ld\n",
	      i, (long) Glob->cappi_button_frame[i]->x->gc);
    
    fprintf(stderr, "vsection_title_frame gc = %ld\n",
	    (long) Glob->vsection_title_frame->x->gc);
    
    fprintf(stderr, "vsection_plot_frame gc = %ld\n",
	    (long) Glob->vsection_plot_frame->x->gc);
    
    for (i = 0; i < N_VSECTION_BUTTONS; i++)
      fprintf(stderr, "vsection_button_frame gc[%d] = %ld\n",
	      i, (long) Glob->vsection_button_frame[i]->x->gc);
    
    fprintf(stderr, "help_title_frame gc = %ld\n",
	    (long) Glob->help_title_frame->x->gc);
    
    fprintf(stderr, "help_text_frame gc = %ld\n",
	    (long) Glob->help_text_frame->x->gc);
    
    for (i = 0; i < N_HELP_BUTTONS; i++)
      fprintf(stderr, "help_button_frame gc[%d] = %ld\n",
	      i, (long) Glob->help_button_frame[i]->x->gc);
    
    fprintf(stderr, "main_scale_frame gc = %ld\n",
	    (long) Glob->main_scale_frame->x->gc);

  } /* if (Glob->debug) */

}
