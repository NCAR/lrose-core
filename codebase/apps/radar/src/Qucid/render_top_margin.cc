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
/************************************************************************
 * RENDER_TOP_MARGIN
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_TOP_MARGIN

#include "cidd.h"

int draw_alternate_top_margin( QPaintDevice *pdev);
/**********************************************************************
 * DRAW_HWIN_TOP_LABEL: Label the top of the horizontal image with
 *   a distance scale.
 */

int draw_hwin_top_margin( QPaintDevice *pdev)
{

#ifdef NOTYET
  
    int    tick_xstart;
    int    tick_ystart,tick_yend;
    int    label_width;
    int    label_height;
    int    xmid,ymid;
    double    range;    
    double    min_val;
    double    tick_spacing;
    double    current_tick;
    double   lon_tick;
    char    label[256];
    Font    font;
    double unit_per_km;

    if(gd.h_win.margin.top <= 0) return 0;

    /* clear top margin */
    XFillRectangle(gd.dpy, xid, gd.legends.margin_color->gc,
                 0, 0,
                 gd.h_win.can_dim.width,
                 gd.h_win.margin.top);

    switch(_params.top_margin_render_style) {
      default:
      case 1:   // Do nothing - Fall through to distance scale code.
      break;

      case 2:
        return draw_alternate_top_margin(xid);
      break;

      case 3:  // Blank Label
       return 0;
      break;

    }

    if(gd.display_projection == Mdvx::PROJ_LATLON) {
	    unit_per_km = 1.0;
    } else{
          unit_per_km = _params.scale_units_per_km;
    }

    /* calc dimensions of drawable area */
    tick_yend = (int)(gd.h_win.margin.top * 0.75);

    range = gd.h_win.cmax_x - gd.h_win.cmin_x;    
    range *= unit_per_km;
    
    tick_spacing = compute_tick_interval(range);
    min_val = (int)(gd.h_win.cmin_x * unit_per_km) -1.0;

    current_tick = min_val + abs((int)min_val %  (int)((tick_spacing > 1.0) ?
                   tick_spacing:  1.0));

    label_width = (int)(gd.h_win.img_dim.width  * (tick_spacing / range) * 0.6);
    label_height = (int)(gd.h_win.margin.top * 0.75);
    while(current_tick < gd.h_win.cmax_x * unit_per_km) {
        disp_proj_to_pixel(&gd.h_win.margin,(current_tick / unit_per_km),
	            gd.h_win.cmax_y,&tick_xstart,&tick_ystart);

	if(tick_xstart >= gd.h_win.margin.left) {
          XDrawLine(gd.dpy,xid,gd.legends.foreground_color->gc,
       	    tick_xstart,tick_ystart,tick_xstart,tick_yend);

	  switch(gd.display_projection ) {
	    case  Mdvx::PROJ_LATLON:
	      lon_tick = current_tick;
	      // normailize to +/- 180
	      while (lon_tick < 180.0) lon_tick += 360.0;
	      while (lon_tick > 180.0) lon_tick -= 360.0;
	      if(range > 7.5) {
		if(lon_tick < 0.0) {
                  snprintf(label,256,"%.0fW",-lon_tick);
		} else {
                    snprintf(label,256,"%.0fE",lon_tick);
		}
	      } else {
		if(lon_tick < 0.0) {
                    snprintf(label,256,"%.1fW",-lon_tick);
		} else {
                    snprintf(label,256,"%.1fE",lon_tick);
		}
	      } 
	    break;

	    default:
	      if(range > 7.5) {
                snprintf(label,256,"%.0f",current_tick);
	      } else {
                snprintf(label,256,"%.1f",current_tick);
	      } 
	    break;
	   }
          font = choose_font(label,label_width,label_height,&xmid,&ymid);
          XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
          XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc,
           (tick_xstart + xmid),tick_yend -2,label,strlen(label));
	}

        current_tick += tick_spacing;
    };

#endif
    
  return CIDD_SUCCESS;
}

/**********************************************************************
 * DRAW_ALTERNATE_TOP_MARGIN: Label the top of the horizontal image
 *   Frame time and Height Level
 */     

int draw_alternate_top_margin( QPaintDevice *pdev)
{
#ifdef NOTYET
    int    label_width;
    int    label_height;
    int    start_x;
    int    xmid,ymid;
    const char   *msg;
    Font    font;

    label_width = (int)(gd.h_win.img_dim.width  * 0.7);
    label_height = gd.h_win.margin.top - 5;
    
    // Plot the Frame time message on the left
    msg = frame_time_msg(gd.movie.cur_frame);
    font = choose_font(msg,label_width,label_height,&xmid,&ymid);
    XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
    start_x =  gd.h_win.margin.left + 2;
    XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc,
           start_x,
           gd.h_win.margin.top  / 2 + ymid, msg,strlen(msg));


    // Plot the Height Label on the right
    msg = height_label();
    label_width = (int)(gd.h_win.img_dim.width  * 0.3);

    // If the Frame time label is short, center the height label
    font = choose_font(msg,label_width,label_height,&xmid,&ymid);
    XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
    XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc,
           gd.h_win.img_dim.width + gd.h_win.margin.left + (2 * xmid) - 2,
           gd.h_win.margin.top / 2 + ymid ,msg,strlen(msg));
#endif
  return CIDD_SUCCESS;
}
