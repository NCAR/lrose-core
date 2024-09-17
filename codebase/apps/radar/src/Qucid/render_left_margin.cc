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
 * RENDER_LEFT_MARGIN
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_LEFT_MARGIN 1

#include "cidd.h"

/**********************************************************************
 * DRAW_LEFT_LABEL: Draw an axis in the left margin & label it
 */

int draw_hwin_left_margin( QPaintDevice *pdev) 
{
    int    x_start;
    int    tick_xstart,tick_xend;
    int    tick_ystart;
    int    xmid,ymid;
    double    range;
    double    min_val;
    double    tick_spacing;
    double    current_tick;
    char    label[128];
    Font    font;
     
    double unit_per_km;
    const char *u_label;

    if(gd.h_win.margin.left == 0) return 0;

    /* clear left margin */
    XFillRectangle(gd.dpy, xid, gd.legends.margin_color->gc,
                 0, gd.h_win.margin.top,
                 gd.h_win.margin.left, gd.h_win.img_dim.height);
     
 
    if(gd.display_projection == Mdvx::PROJ_LATLON) {
	    u_label = "deg";
	    unit_per_km = 1.0;
    } else{
          unit_per_km = _params.scale_units_per_km;
	  u_label = _params.scale_units_label;
    }

    /* calc dimensions of drawable area */
    x_start = 0;
    tick_xstart = gd.h_win.margin.left - 3;

    range = gd.h_win.cmax_y - gd.h_win.cmin_y;    
    range *= unit_per_km;

    tick_spacing = compute_tick_interval(range);
    min_val = (int)(gd.h_win.cmin_y * unit_per_km) -1.0;
    current_tick = min_val + abs((int)min_val %  (int)((tick_spacing > 1.0) ? tick_spacing:  1.0));

    while(current_tick < gd.h_win.cmax_y * unit_per_km) {
        disp_proj_to_pixel(&gd.h_win.margin,gd.h_win.cmin_x ,(current_tick / unit_per_km) ,&tick_xend,&tick_ystart);
	if(tick_ystart <= (gd.h_win.can_dim.height - gd.h_win.margin.bot)) {
          XDrawLine(gd.dpy,xid,gd.legends.foreground_color->gc,tick_xstart,tick_ystart,tick_xend-1,tick_ystart);

	  if(range > 7.5) {
            snprintf(label,128,"%.0f",current_tick);
	  } else {
	    snprintf(label,128,"%.1f",current_tick);
	  }
          font = choose_font(label,tick_xstart,gd.h_win.margin.left,&xmid,&ymid);
          XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
          XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc,
            x_start,(tick_ystart + ymid),label,strlen(label));

	}
        current_tick += tick_spacing;
    };

    snprintf(label,128,"%s",u_label);
    font = choose_font(label,tick_xstart,gd.h_win.margin.left,&xmid,&ymid);
    XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
    if(_params.html_mode) {  // Draw label at the bottom
        XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc, x_start,gd.h_win.win_dim.height - (4*ymid),label,strlen(label));
    } else { // Draw it at the top
        XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc, x_start,(4*ymid),label,strlen(label));
    }

    return 1;
}
