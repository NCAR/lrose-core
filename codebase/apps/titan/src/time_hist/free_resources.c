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

#include "time_hist.h"

void free_resources(void)

{

  int i;

  xFreeGC(Glob->rdisplay, Glob->ticklabel_gc);
  xFreeGC(Glob->rdisplay, Glob->header_gc);
  xFreeGC(Glob->rdisplay, Glob->tick_gc);
  xFreeGC(Glob->rdisplay, Glob->crosshair_gc);
  xFreeGC(Glob->rdisplay, Glob->copyarea_gc);

  if (Glob->tscale_button_frame != NULL)
    for (i  = 0; i < N_TSCALE_BUTTONS; i++)
      if (Glob->tscale_button_frame[i] != NULL)
	xFreeGC(Glob->rdisplay, Glob->tscale_button_frame[i]->x->gc);

  if (Glob->tscale_title_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->tscale_title_frame->x->gc);

  if (Glob->tscale_plot_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->tscale_plot_frame->x->gc);

  if (Glob->thist_button_frame != NULL)
    for (i  = 0; i < N_THIST_BUTTONS; i++)
      if (Glob->thist_button_frame[i] != NULL)
	xFreeGC(Glob->rdisplay, Glob->thist_button_frame[i]->x->gc);

  if (Glob->thist_title_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->thist_title_frame->x->gc);

  if (Glob->thist_plot_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->thist_plot_frame->x->gc);

  if (Glob->timeht_button_frame != NULL)
    for (i  = 0; i < N_TIMEHT_BUTTONS; i++)
      if (Glob->timeht_button_frame[i] != NULL)
	xFreeGC(Glob->rdisplay, Glob->timeht_button_frame[i]->x->gc);

  if (Glob->timeht_title_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->timeht_title_frame->x->gc);

  if (Glob->timeht_plot_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->timeht_plot_frame->x->gc);

  if (Glob->timeht_scale_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->timeht_scale_frame->x->gc);

  if (Glob->rdist_button_frame != NULL)
    for (i  = 0; i < N_RDIST_BUTTONS; i++)
      if (Glob->rdist_button_frame[i] != NULL)
	xFreeGC(Glob->rdisplay, Glob->rdist_button_frame[i]->x->gc);

  if (Glob->rdist_title_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->rdist_title_frame->x->gc);

  if (Glob->rdist_plot_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->rdist_plot_frame->x->gc);

  if (Glob->rdist_scale_frame != NULL)
    xFreeGC(Glob->rdisplay, Glob->rdist_scale_frame->x->gc);

  xFreeFont(Glob->rdisplay, Glob->x_button_font);
  xFreeFont(Glob->rdisplay, Glob->x_title_font);
  xFreeFont(Glob->rdisplay, Glob->x_scale_font);
  xFreeFont(Glob->rdisplay, Glob->x_ticklabel_font);
  xFreeFont(Glob->rdisplay, Glob->x_header_font);

  if (Glob->tscale_button_frame != NULL)
    for (i  = 0; i < N_TSCALE_BUTTONS; i++)
      GFreeFrame(Glob->tscale_button_frame[i]);

  GFreeFrame(Glob->tscale_title_frame);
  GFreeFrame(Glob->tscale_plot_frame);

  if (Glob->thist_button_frame != NULL)
    for (i  = 0; i < N_THIST_BUTTONS; i++)
      GFreeFrame(Glob->thist_button_frame[i]);

  GFreeFrame(Glob->thist_title_frame);
  GFreeFrame(Glob->thist_plot_frame);

  if (Glob->timeht_button_frame != NULL)
    for (i  = 0; i < N_TIMEHT_BUTTONS; i++)
      GFreeFrame(Glob->timeht_button_frame[i]);

  GFreeFrame(Glob->timeht_title_frame);
  GFreeFrame(Glob->timeht_plot_frame);
  GFreeFrame(Glob->timeht_scale_frame);

  if (Glob->rdist_button_frame != NULL)
    for (i  = 0; i < N_RDIST_BUTTONS; i++)
      GFreeFrame(Glob->rdist_button_frame[i]);

  GFreeFrame(Glob->rdist_title_frame);
  GFreeFrame(Glob->rdist_plot_frame);
  GFreeFrame(Glob->rdist_scale_frame);

  GFreeFrame(Glob->vert_page_frame);
  GFreeFrame(Glob->horiz_page_frame);

  xFreeColorList(Glob->rdisplay,
		 DefaultColormap(Glob->rdisplay, Glob->rscreen),
		 &Glob->color_index);

  if (Glob->windows_active == TRUE) {

    XDestroySubwindows(Glob->rdisplay, Glob->rdist_window);
    XDestroyWindow(Glob->rdisplay, Glob->rdist_window);

    XDestroySubwindows(Glob->rdisplay, Glob->timeht_window);
    XDestroyWindow(Glob->rdisplay, Glob->timeht_window);

    XDestroySubwindows(Glob->rdisplay, Glob->thist_window);
    XDestroyWindow(Glob->rdisplay, Glob->thist_window);

    XDestroySubwindows(Glob->rdisplay, Glob->tscale_window);
    XDestroyWindow(Glob->rdisplay, Glob->tscale_window);

    XCloseDisplay(Glob->rdisplay);

  }

}
