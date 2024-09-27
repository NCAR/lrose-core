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
 * RENDER_BOTTOM_MARGIN
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_BOTTOM_MARGIN

#include "cidd.h"

int draw_alternate_hwin_bot_margin( QPaintDevice *pdev);

// Slope and intercept for time to pixel mapping function 
static double secs_pixel; // Seconds per pixel
static double y_intercept; // Seconds per pixel

//////////////////////////////////////////////////////////////////////////
// TIME_FROM_PIXEL:  Return the time , given a pixel location, relative
//                   to the time axis

time_t time_from_pixel(int x_pixel)
{
    time_t result;

    result = (time_t)(secs_pixel * x_pixel + y_intercept);
    return result;
}

/**********************************************************************
 * DRAW_HWIN_BOT_MARGIN: Draw an axis and label the bottom margin
 */

int draw_hwin_bot_margin( QPaintDevice *pdev, int  page, time_t start_time, time_t end_time)
{
#ifdef NOTYET
    int x1,x2;
    int label_width;
    int label_height;
    int xmid,ymid;
    int x_start, x_end,y_start, y_end;
    time_t tick_seconds, label_seconds;
    time_t time_range;
    time_t min,max;
    time_t value;
    Font font;
    const char *fmt_str;
    const char *fmt_str0z;
    char string_buf[128];

    if(gd.h_win.margin.bot <=1) return 0;

    /* clear bot margin */
    XFillRectangle(gd.dpy, xid, gd.legends.margin_color->gc,
                 0, gd.h_win.can_dim.height - gd.h_win.margin.bot,
                 gd.h_win.can_dim.width, gd.h_win.margin.bot);
    

    switch(_params.bot_margin_render_style) {
      case 1:
         return draw_alternate_hwin_bot_margin(xid);
      break;

      default:  // Do nothing - Fall through to following code.
      case 2:
      break;
    }

    // Establish drawing areas
    x_start = gd.h_win.margin.left;
    y_start = gd.h_win.can_dim.height -  gd.h_win.margin.bot + 1 ;
    x_end = gd.h_win.can_dim.width - gd.h_win.margin.right -1;
    y_end = gd.h_win.can_dim.height - 1;

    // determine time span of movie loop.
    min = gd.movie.start_time;
    time_range = (int) (gd.movie.time_interval_mins * 60 * gd.movie.num_frames);
    max = (time_t)(min + time_range );

    secs_pixel = (max - min) / (x_end - x_start);
    y_intercept =  min - (secs_pixel * x_start);


    if(time_range <= 1200) {         // 20 minutes
      label_seconds = 300; 
      tick_seconds = 120; 
      fmt_str = " %H:%M";
      fmt_str0z = " %H:%M %b %d";
    } else if(time_range <= 2700) {  // 45 minutes
      label_seconds = 900; 
      tick_seconds = 300; 
      fmt_str = " %H:%M";
      fmt_str0z = " %H:%M %b %d";
    } else  if(time_range <= 5400) { // 90 minutes
      label_seconds = 900; 
      tick_seconds = 300; 
      fmt_str = " %H:%M";
      fmt_str0z = " 0Z %b %d";
    } else  if (time_range <= 10800) { // 3 hours
      label_seconds = 1800; 
      tick_seconds = 900; 
      fmt_str = " %H:%M";
      fmt_str0z =   (_params.use_local_timestamps)? " 0 %m/%d": " 0Z %m/%d";
    } else  if (time_range <= 21600) { // 6 hours
      label_seconds = 3600; 
      tick_seconds = 900; 
	  fmt_str =   (_params.use_local_timestamps)? " %H": " %HZ";
      fmt_str0z =   (_params.use_local_timestamps)? " 0 ": " 0Z %m/%d";
    } else  if (time_range <= 43200) { // 12 hours
      label_seconds = 7200; 
      tick_seconds = 1800; 
	  fmt_str =   (_params.use_local_timestamps)? " %H": " %HZ";
      fmt_str0z =   (_params.use_local_timestamps)? " 0 ": " 0Z %m/%d";
    } else  if (time_range <= 86400) { // 24 hours
      label_seconds = 14400; 
      tick_seconds = 3600;
	  fmt_str =   (_params.use_local_timestamps)? " %H": " %HZ";
      fmt_str0z = "%m/%d";
    } else  if (time_range <= 172800) { // 48 Hours
      label_seconds = 28800; 
      tick_seconds = 7200; 
	  fmt_str =   (_params.use_local_timestamps)? " %H": " %HZ";
      fmt_str0z = "%m/%d"; 
    } else  if (time_range <= 345600) { // 4 Days 
      label_seconds = 43200; 
      tick_seconds = 14400; 
      fmt_str = "%m/%d";
      fmt_str0z = "%m/%d";
    } else  if (time_range <= 604800) { // 7 days
      label_seconds = 86400; 
      tick_seconds = 14400; 
      fmt_str = "%m/%d";
      fmt_str0z = "%m/%d";
    } else  if (time_range <= 1209600) { // 14 days
      label_seconds = 172800; 
      tick_seconds = 14400; 
      fmt_str = "%m/%d";
      fmt_str0z = "%m/%d";
    } else  if (time_range <= 2419200) { // 28 days
      label_seconds = 604800; 
      tick_seconds = 86400; 
      fmt_str = "%m/%d";
      fmt_str0z = "%m/%d";
    } else  if (time_range <= 15552000) { // 180 days
      label_seconds = 12614400; // 365 Days
      tick_seconds = 2592000; // 30 days
      fmt_str = "%y";
      fmt_str0z = fmt_str;
    } else  {                           
      label_seconds = 63072000;  // 5 years
      tick_seconds = 12614400;  // 365 days
      fmt_str0z = "%y";
      fmt_str = fmt_str0z;
    }

    label_width = (int) (x_end / (time_range  / (label_seconds / 2.0)));
    label_height = (int) (gd.h_win.margin.bot * 0.85);

    //printf("Time Range = %d s Label: %d, Tick: %d seconds\n", time_range,label_seconds,tick_seconds);

    // Plot Frame limits
    x1 = (int) ((start_time - y_intercept) / secs_pixel);
    x2 =  (int) ((end_time - y_intercept) / secs_pixel);
    XFillRectangle(gd.dpy,xid,gd.legends.time_frame_color->gc,
        x1,y_start,(x2 - x1 + 1),(y_end - y_start + 1));

    // Draw Base of AXIS - Across whole bottom
    XDrawLine(gd.dpy,xid,gd.legends.time_axis_color->gc,
		  0, y_end, gd.h_win.can_dim.width -1, y_end);


    // Start with the next even minute tic
    value = (max - (max % tick_seconds)) + (2 * tick_seconds) ;

    // Plot time tics every N minutes - Work backwards from the latest time
    while ( value > min) {
	value -= tick_seconds; 
	// calc X pixel location.
	x1 =(int) ((value - y_intercept) / secs_pixel);

	if((value % label_seconds) == 0)  { // Place a time label every N min
	  struct tm res;
	  // Plot a large tick
	  XDrawLine(gd.dpy,xid,gd.legends.time_axis_color->gc,x1,y_end - label_height ,x1,y_end);
	  //XFillRectangle(gd.dpy,xid,gd.legends.time_axis_color->gc,x1-1,y_end -5 ,x1+2,y_end + 1);
	  if((value %  86400) == 0) {  // A 0 Z tickmark
         if(_params.use_local_timestamps) {
	      strftime(string_buf,128,fmt_str0z,localtime_r(&(value),&res));
         } else {
	      strftime(string_buf,128,fmt_str0z,gmtime_r(&(value),&res));
         }
	      font = choose_font(string_buf,label_width,label_height,&xmid,&ymid);
	      XSetFont(gd.dpy,gd.legends.time_axis_color->gc,font);
	      XDrawString(gd.dpy,xid,gd.legends.time_axis_color->gc,
			   x1 + xmid, y_start + (2 * ymid) + 2,string_buf,
			   strlen(string_buf));
	  } else {
         if(_params.use_local_timestamps) {
	      strftime(string_buf,128,fmt_str,localtime_r(&(value),&res));
         } else {
	      strftime(string_buf,128,fmt_str,gmtime_r(&(value),&res));
         }
	      font = choose_font(string_buf,label_width,label_height,&xmid,&ymid);
	      XSetFont(gd.dpy,gd.legends.time_axis_color->gc,font);
	      XDrawString(gd.dpy,xid,gd.legends.time_axis_color->gc,
			   x1 + xmid, y_start + (2 * ymid) + 2,string_buf,
			   strlen(string_buf));
	  }
       } else { // Plot a small tick
	  XDrawLine(gd.dpy,xid,gd.legends.time_axis_color->gc,x1,y_end -4,x1,y_end);
       }
    }
#endif
    
    return 1;
}

/**********************************************************************
 * DRAW_ALTERNATE_HWIN_BOT_MARGIN: Put a distance scale along the bottom
 */

int draw_alternate_hwin_bot_margin( QPaintDevice *pdev)
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
    double   units_per_km;
    char    label[256];
    Font    font;

    if(gd.display_projection == Mdvx::PROJ_LATLON) {
	units_per_km = 1.0;
    } else {
	units_per_km = _params.scale_units_per_km;
    }

    /* calc dimensions of drawable area */
    tick_yend = (int) (gd.h_win.margin.bot * 0.25) + gd.h_win.can_dim.height - gd.h_win.margin.bot;

    range = gd.h_win.cmax_x - gd.h_win.cmin_x;    
    range *= units_per_km;
    
    tick_spacing = compute_tick_interval(range);
    min_val = (int)(gd.h_win.cmin_x * units_per_km) -1.0;

    current_tick = min_val + abs((int)min_val %  (int)((tick_spacing > 1.0) ?
tick_spacing:  1.0));

    label_width = (int)(gd.h_win.img_dim.width  * (tick_spacing / range) * 0.6);
    label_height = (int)(gd.h_win.margin.bot * 0.75);
    while(current_tick < gd.h_win.cmax_x * units_per_km) {
        disp_proj_to_pixel(&gd.h_win.margin,(current_tick / units_per_km),
	            gd.h_win.cmin_y,&tick_xstart,&tick_ystart);

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
           (tick_xstart + xmid),gd.h_win.can_dim.height -2,label,strlen(label));
	}

        current_tick += tick_spacing;
    };
#endif
  return CIDD_SUCCESS;
}
