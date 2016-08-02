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
/****************************************************************
 * RENDER_PRODUCTS.c
 *
 * Supervisory routines for rendering products
 * 
 * Calls both PROD_SEL and PRDS rendering routines
 * F. Hage
 * June 1997
 *
 *****************************************************************/

#include "cidd.h"
#include "cidd_spr.h"

/*****************************************************************
 * UPDATE_LIVE_PRODUCTS(): This routine copies the "backing pixmap" and
 *  Renders products on it. Then it copies the backing pixmap
 *  onto the visible canvas. This is the mechanism that provides
 *  products to update over the current data grids and wind plots.
 */

void update_live_products(void)
{
  Drawable xid;
  time_t start_time, end_time;
  int prds_changed = 0;
  int prod_sel_changed = 0;
 
  /* Bail if underlying image still needs done */
  if(gd.h_win.redraw[gd.h_win.field] != 0) return;

  prds_changed = poll_prds();  

  if(!CSPR_products_on()) { /* See if Prod_sel is up */
      CSPR_init(); /* CSPR_products_on() will return TRUE when PQ exists*/
  } else {
      prod_sel_changed = CSPR_check_pq_update();
  }
							 

  if((prds_changed == 0) && (prod_sel_changed == 0)) return; 

  if (prod_sel_changed)
  {
   CSPR_clear_update_flag();
   if(gd.debug2) fprintf(stderr,"Live Update mode - prod_sel change detected:\n"); 
  }

  start_time =  gd.movie.frame[gd.movie.cur_frame].time_start.unix_time;
  end_time = gd.movie.frame[gd.movie.cur_frame].time_end.unix_time;

  /*
   * Get the "background" graphic into the final drawing area 
   */

   if(gd.mrec[gd.h_win.field]->background_render) {
       xid = gd.h_win.field_xid[gd.h_win.field];
   } else {
       xid = gd.h_win.tmp_xid;
   }

   XCopyArea(gd.dpy, xid,
           gd.h_win.can_xid,
           gd.def_gc,    0,0,
           gd.h_win.can_dim.width,
           gd.h_win.can_dim.height,
           gd.h_win.can_dim.x_pos,
           gd.h_win.can_dim.y_pos);

   gd.h_win.last_field = gd.h_win.field;

  /*
   * Render the products onto the final drawing area pixmap
   */
   render_horiz_products(gd.h_win.can_xid, start_time, end_time); 

  /* Now copy the final drawing area to the visible canvas */
  XCopyArea(gd.dpy, gd.h_win.can_xid,
                gd.h_win.vis_xid,
                gd.def_gc,    0,0,
                gd.h_win.can_dim.width,
                gd.h_win.can_dim.height,
                gd.h_win.can_dim.x_pos,
                gd.h_win.can_dim.y_pos);
  return;
}

/**********************************************************************
 * RENDER_HORIZ_PRODUCTS: Render selected products onto the given pixmap
 */

render_horiz_products(xid,start_time,end_time, draw_prod_queue)
    Drawable xid;
    long    start_time,end_time;
    int     draw_prod_queue;
{
    long tm;    /* current time */
    tm = time(0);

    /* allow overlays to be active through at least one time interval */
    if((start_time < tm) && (tm - start_time) < (gd.movie.time_interval * 60.0)) {
        start_time = tm - (gd.movie.time_interval * 60.0);
    }

    if(gd.prod.products_on) {
     draw_prds_products(xid,start_time,end_time);
    }

   if(CSPR_products_on()) { /* See if Prod_sel is up */
     if((gd.limited_mode != 1)  || (gd.prod.products_on != 0)) CSPR_draw_prod_queue(xid);
    } else {
      CSPR_init(); /* Try to attach to a prod_sel memory segment */
    }

}   
