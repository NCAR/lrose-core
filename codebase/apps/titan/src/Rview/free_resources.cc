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
 * free_resources.c
 *
 * Frees up memory and X resources
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

void free_resources()

{

  int i;

  if (Glob->verbose) {
    fprintf(stderr, "** free_resources **\n");
  }

  xFreeGC(Glob->rdisplay, Glob->ticklabel_gc);
  xFreeGC(Glob->rdisplay, Glob->text_gc);
  xFreeGC(Glob->rdisplay, Glob->tick_gc);
  xFreeGC(Glob->rdisplay, Glob->ring_gc);
  xFreeGC(Glob->rdisplay, Glob->crosshair_gc);
  xFreeGC(Glob->rdisplay, Glob->copyarea_gc);
  free_map_gcs();

  if (Glob->main_scale_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->main_scale_frame->x->gc);

  if (Glob->cappi_button_frame != NULL)
    for (i  = 0; i < N_CAPPI_BUTTONS; i++)
      if (Glob->cappi_button_frame[i] != NULL)
	xFreeGC(Glob->rdisplay, Glob->cappi_button_frame[i]->x->gc);

  if (Glob->cappi_title_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->cappi_title_frame->x->gc);

  if (Glob->cappi_plot_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->cappi_plot_frame->x->gc);

  if (Glob->vsection_button_frame != NULL)
    for (i  = 0; i < N_VSECTION_BUTTONS; i++)
      if (Glob->vsection_button_frame[i] != NULL)
	xFreeGC(Glob->rdisplay, Glob->vsection_button_frame[i]->x->gc);

  if (Glob->vsection_title_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->vsection_title_frame->x->gc);

  if (Glob->vsection_plot_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->vsection_plot_frame->x->gc);

  xFreeFont(Glob->rdisplay, Glob->x_button_font);
  xFreeFont(Glob->rdisplay, Glob->x_title_font);
  xFreeFont(Glob->rdisplay, Glob->x_scale_font);
  xFreeFont(Glob->rdisplay, Glob->x_ticklabel_font);
  xFreeFont(Glob->rdisplay, Glob->x_ringlabel_font);
  xFreeFont(Glob->rdisplay, Glob->x_text_font);
  free_map_fonts();

  if (Glob->cappi_button_frame != NULL)
    for (i  = 0; i < N_CAPPI_BUTTONS; i++)
      GFreeFrame(Glob->cappi_button_frame[i]);

  GFreeFrame(Glob->cappi_title_frame);
  GFreeFrame(Glob->cappi_plot_frame);
  GFreeFrame(Glob->cappi_ps_plot_frame);

  if (Glob->vsection_button_frame != NULL)
    for (i  = 0; i < N_VSECTION_BUTTONS; i++)
      GFreeFrame(Glob->vsection_button_frame[i]);

  GFreeFrame(Glob->vsection_title_frame);
  GFreeFrame(Glob->vsection_plot_frame);
  GFreeFrame(Glob->vsection_ps_plot_frame);

  GFreeFrame(Glob->main_scale_frame);

  GFreeFrame(Glob->vert_page_frame);
  GFreeFrame(Glob->horiz_page_frame);

  if (Glob->color_index.n_entries > 0)
    xFreeColorList(Glob->rdisplay, Glob->cmap,
		   &Glob->color_index);
  
  if (Glob->vsection.active) {

    XFreePixmap(Glob->rdisplay, Glob->vsection.pixmap);
    XDestroySubwindows(Glob->rdisplay, Glob->vsection_window);
    XDestroyWindow(Glob->rdisplay, Glob->vsection_window);

  }

  if (Glob->cappi_active) {

    for (i = 0; i < NZOOM; i++)
      XFreePixmap(Glob->rdisplay, Glob->zoom[i].pixmap);

    XDestroySubwindows(Glob->rdisplay, Glob->main_window);
    XDestroyWindow(Glob->rdisplay, Glob->main_window);

    XCloseDisplay(Glob->rdisplay);

  }

}
