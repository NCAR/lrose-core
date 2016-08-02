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
 * CIDD_SPR.C
 *
 * Symbolic Product Render routines
 *
 *    Specific to the CIDD program- Uses global color table,
 *    Window dimensions & coordinate transforms, etc
 *
 * Based on code by:
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 * F. Hage.
 *
 * June 1997
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <mdv/mdv_macros.h>
#include <rapformats/station_reports.h>
#include <rapplot/gplot.h>
#include "spr_98.h"
#include <symprod98/symprod.h>
#include <toolsa/umisc.h>

#include "cidd.h"
#include "cidd_spr.h"

#define CIDD_SPR_BUF_SIZE 2048
 
static pq_handle_t *PQ_handle = NULL;
 
/*********************************************************************
 * CSPR_init() Initialize this module -
 *  Connect to the Product Queue.
 */
void CSPR_init(void)
{
    static int buffer_key = -1;
    static int status_key = -1;

    if(buffer_key < 0) {
	buffer_key = XRSgetLong(gd.cidd_db, "cidd.prod_sel_buf_key", 0);
        status_key = XRSgetLong(gd.cidd_db, "cidd.prod_sel_stat_key", 0);
    }

    if(buffer_key == 0 || status_key == 0) return;

    if((PQ_handle = PQ_check_create_user(status_key,buffer_key)) == NULL) {
        if(gd.debug)
	   fprintf(stderr,
	     "PROD_SEL shared memory segements: %d, %d don't exist yet\n",
	      buffer_key,status_key);
	return; /* bail out */
    } else {
        if(gd.debug)
	   fprintf(stderr,
	     "Attached to PROD_SEL shared memory segements: %d, %d\n",
	      buffer_key,status_key);
    }

    if(gd.movie.mode == MOVIE_MR) {
        CSPR_set_time(-1);
    } else {
      if (gd.prod.product_time_select == 0) {
		if(gd.gather_data_mode == 0) {
          CSPR_set_time(gd.movie.frame[gd.movie.end_frame].time_mid.unix_time);
		} else {
          CSPR_set_time(gd.movie.frame[gd.movie.end_frame].time_end.unix_time);
		}
      } else {
		if(gd.gather_data_mode == 0) {
          CSPR_set_time(gd.movie.frame[gd.movie.start_frame].time_mid.unix_time);
		} else {
          CSPR_set_time(gd.movie.frame[gd.movie.start_frame].time_end.unix_time);
		}
	  }
    }
}

/*********************************************************************
 * CSPR_calc_and_set_time() - Calculate what the product time for the
 *                            product selector should be based on the
 *                            current state in Cidd and set the time in
 *                            the shared memory segment.
 *  
 */
void CSPR_calc_and_set_time(void)
{
  struct tm *g_tm;
  char time_string[64];
  time_t spr_time = -1;    /* default to realtime for cases not handled */
  time_t data_time = -1;   /* default to realtime for cases not handled */
  static time_t last_spr_time = -99;
  static time_t last_data_time = -99;
  
  if(PQ_handle)
  {
    switch (gd.movie.mode)
    {
    case MOVIE_MR :       /* Realtime mode */
      /*
       * If gd.prod.product_time_select is 0, we just want the realtime
       * products so we'll just break out of this case and set the
       * times to -1.
       */

      /*
       * For gd.prod.product_time_select of 1, we want the products to match
       * the current movie frame time.
       */

      if (gd.prod.product_time_select == 1)
      {
	if (gd.movie.cur_frame != gd.movie.end_frame)
	  spr_time = gd.movie.frame[gd.movie.cur_index].time_mid.unix_time;
          data_time =  gd.mrec[gd.h_win.field]->h_date.unix_time;
      }
      
      break;
      
    case MOVIE_TS :       /* Archive mode */
      if (gd.prod.product_time_select == 0) {
		if(gd.gather_data_mode == 0) {
          spr_time = gd.movie.frame[gd.movie.end_frame].time_mid.unix_time;
		} else {
          spr_time = gd.movie.frame[gd.movie.end_frame].time_end.unix_time;
		}
      } else {
		if(gd.gather_data_mode == 0) {
          spr_time = gd.movie.frame[gd.movie.cur_index].time_mid.unix_time;
		} else {
          spr_time = gd.movie.frame[gd.movie.cur_index].time_end.unix_time;
		}
	  }

      data_time =  gd.mrec[gd.h_win.field]->h_date.unix_time;

    break;
      
    case MOVIE_EL :       /* Elevation movie - no longer used */
      /*
       * Just set prod_sel to realtime mode since this mode
       * should no longer exist.  We can figure out what it should
       * be later if this mode is reimplemented.
       */

      spr_time = -1;
      
      break;

    } /* endswitch - gd.movie.mode */
    
    if(spr_time != last_spr_time) {
        CSPR_set_time(spr_time);
	last_spr_time = spr_time;
	if(spr_time != -1) {
	    g_tm = gmtime(&spr_time);
	    strftime(time_string,64,"Image Time: %T GMT",g_tm);
	} else {
	    sprintf(time_string,"Image Time: REALTIME");
	}
	xv_set(gd.extras_pu->prod_tm_msg,
	     PANEL_LABEL_STRING,time_string,NULL);
    }
    
    if(data_time != 0 && data_time != last_data_time) {
        CSPR_set_display_data_time(data_time);
	last_data_time = data_time;
	if(data_time != -1) {
	    g_tm = gmtime(&data_time);
	    strftime(time_string,64,"Data Time: %T GMT",g_tm);
	} else {
	    sprintf(time_string,"Data Time: REALTIME");
	}
	xv_set(gd.extras_pu->prod_data_tm_msg,
	     PANEL_LABEL_STRING,time_string,NULL);
    }

  } /* endif - PQ_handle */

  return;
}

/*********************************************************************
 * CSPR_set_time() - Set the interest time for the Symbolic
 *  Product Selector 
 *  
 */
void CSPR_set_time(time_t time)
{
  if(PQ_handle)
  {
    if(gd.debug)
    {
      if (time > 0)
	printf("Setting PROD_SEL TIME to: %d (%s)\n",
	       time, utimstr(time));
      else
	printf("Setting PROD_SEL TIME to: %d\n",
	       time);
    }
    
    PQ_update_display_time(PQ_handle,time);
  }
}

/*********************************************************************
 * CSPR_set_display_data_time() - Set the ataitime for the Symbolic
 *  Product Selector 
 *  
 */
void CSPR_set_display_data_time(time_t time)
{
  if(PQ_handle)
  {
    if(gd.debug)
    {
      if (time > 0)
	printf("Setting PROD_SEL DATA TIME to: %d (%s)\n",
	       time, utimstr(time));
      else
	printf("Setting PROD_SEL DATA TIME to: %d\n",
	       time);
    }
    
    PQ_update_display_data_time(PQ_handle,time);
  }
}

/*********************************************************************
 * CSPR_set_map_flag() - Set the map flag value for the product selector
 *  
 */
void CSPR_set_map_flag(int flag_value)
{
  if(PQ_handle)
  {
    if (gd.debug)
      fprintf(stdout, "Setting PROD_SEL map flag to %d\n", flag_value);
      
    PQ_set_map_flag(PQ_handle, flag_value);
  }
}

/*********************************************************************
 * CSPR_products_on() - Checks to see if we are using the prod_sel
 * products.  Returns TRUE if we are, FALSE if we aren't.
 */
int CSPR_products_on(void)
{
  if(PQ_handle)
    return(TRUE);
  
  return(FALSE);
}

/*********************************************************************
 * CSPR_clear_update_flag() - Clear the server update flag.  This
 * shows that we've noticed that the products have been updated and
 * we've set the appropriate internal flags to render the new products.
 */
void CSPR_clear_update_flag(void)
{
  if (!PQ_handle)
    return;
  
  PQ_set_server_update(PQ_handle, FALSE);
  
  return;
}

/*********************************************************************
 * CSPR_data_current() - See if the data_time and display_time in the
 * product queue match
 *  
 */
int CSPR_data_current(void)
{
  if (!PQ_handle)
    return(TRUE);
  
  return(PQ_data_current(PQ_handle));
}

/*********************************************************************
 * CSPR_check_pq_update() - Check to see if the Symbolic
 *  Product Selector has new data. Returns TRUE if the PQ has
 *  been updated, false otherwise.
 *  
 */
int CSPR_check_pq_update()
{
    if(PQ_handle) {
        return PQ_check_server_update(PQ_handle);
    } else  {
    	return FALSE;
    }
}

/*********************************************************************
 * CSPR_destroy() - - Clean up before exiting
 */
void CSPR_destroy()
{
    if(PQ_handle) PQ_destroy(PQ_handle);
}

/*********************************************************************
 * CSPR_draw_prod_queue()
 *
 * Renders all of the active, non-expired, selected for display 
 * symbolic products currently in the product queue.
 */

void CSPR_draw_prod_queue(Drawable xid)
{
  int slot;
  
  int mdv_projection;
  gframe_t frame;
  xref_t   xref;
  
  double x_km_per_pixel;
  double y_km_per_pixel;
  
  double top_km;
  double bot_km;
  double left_km;
  double right_km;
  static char last_background_color_name[128];

  XRectangle    clip_rect;
  
  if(PQ_handle == NULL) return; /* Bail if no product queue */
   
  if (gd.debug)
    fprintf(stdout, "Drawing product queue\n");
  
  /*
   * Initialize the SPR routines.  Do this here in case the
   * projection changes.
   */

  switch(gd.projection_mode)
  {
  case CARTESIAN_PROJ :
    mdv_projection = MDV_PROJ_FLAT;
    break;
    
  case LAT_LON_PROJ :
    mdv_projection = MDV_PROJ_LATLON;
    break;
  }
  
  SPR_init(mdv_projection, gd.h_win.origin_lat, gd.h_win.origin_lon);

  /*
   * Convert margin values to world coordinates so we can
   * use the frame values expected in the SPR routines.
   */

  x_km_per_pixel = (gd.h_win.cmax_x - gd.h_win.cmin_x) /
    (double)gd.h_win.img_dim.width;
  y_km_per_pixel = (gd.h_win.cmax_y - gd.h_win.cmin_y) /
    (double)gd.h_win.img_dim.height;
  
  left_km = (double)gd.h_win.margin.left * x_km_per_pixel;
  right_km = (double)gd.h_win.margin.right * x_km_per_pixel;
  top_km = (double)gd.h_win.margin.top * y_km_per_pixel;
  bot_km = (double)gd.h_win.margin.bot * y_km_per_pixel;
  
  frame.w_xmin = gd.h_win.cmin_x - left_km;
  frame.w_ymin = gd.h_win.cmin_y - bot_km;
  frame.w_xmax = gd.h_win.cmax_x + right_km;
  frame.w_ymax = gd.h_win.cmax_y + top_km;
  
  /* Set up X rendering parameters */
  clip_rect.x = gd.h_win.img_dim.x_pos;
  clip_rect.y = gd.h_win.img_dim.y_pos;
  clip_rect.width = gd.h_win.img_dim.width;
  clip_rect.height = gd.h_win.img_dim.height;
  XSetClipRectangles(gd.dpy,gd.def_gc,0,0,&clip_rect,1,YXSorted);

  frame.x = &xref;
  
  xref.display = gd.dpy;
  xref.drawable = xid;
  xref.width = gd.h_win.can_dim.width;
  xref.height = gd.h_win.can_dim.height;
  xref.xscale = (double)xref.width / (frame.w_xmax - frame.w_xmin);
  xref.yscale = (double)xref.height / (frame.w_ymax - frame.w_ymin);
  xref.gc = gd.def_gc;
  xref.font = gd.fontst[gd.prod.prod_font_num];
  
  frame.ps = NULL;
  frame.psgc = NULL;
  
  /*
   * Clear the server update flag.
   */

  PQ_set_server_update(PQ_handle, FALSE);
  
  /*
   * Set the background color for SPR rendering when ever it's changed
   */

  if(strncmp(last_background_color_name,gd.extras.background_color->name,128) != 0) {
      SPR_set_background_color(gd.dpy, gd.cmap, NULL,
			   gd.extras.background_color->name);
      strncpy(last_background_color_name, gd.extras.background_color->name,128);
  }
  
  /*
   * Render the product queue
   */

  SPR_draw_prod_queue(XDEV, &frame, gd.dpy, gd.cmap, NULL,
		      PQ_handle);
  
  /* Reset the GC background color in case it was changed */

  XSetBackground(gd.dpy, gd.def_gc, gd.extras.background_color->pixval);
  
  /* Reset the Clip Rectangle */
  clip_rect.x = 0;
  clip_rect.y = 0;
  clip_rect.width = gd.h_win.can_dim.width;
  clip_rect.height = gd.h_win.can_dim.height;
  XSetClipRectangles(gd.dpy,gd.def_gc,0,0,&clip_rect,1,YXSorted);

  umalloc_verify();
  
  return;
  
}

