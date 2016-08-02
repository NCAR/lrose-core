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
//////////////////////////////////////////////////////////
// TimePlot.cc  Plot Meteograms 
//
// Jan 2003
//
//////////////////////////////////////////////////////////

#include "TimePlot.hh"
#include <stdio.h>
#include <math.h>

//////////////////////////////////////////
// DEFAULT CONSTRUCTOR

TimePlot::TimePlot()

{
    epoch_start = 0;
    epoch_end = 0;
    debug = 0;
}

//////////////////////////////////////////
// 

void TimePlot::set_debug_level(int level)

{
    debug = level;
}


//////////////////////////////////////////
// INIT: Set up the Time plot graphics, etc
// xstart = Starting X position - Pixels.
// ystart = Starting Y position - Pixels.
// w = width of plot in pixels
// h = height of plot in pixels
// axis_pos = Y pixel location of axis - Text labels below 
// t_start = Plot left edge time
// t_end = Plot right edge time
// 
void TimePlot::Init(Display *dpy, Drawable dr, GC fgc, GC bgc, GC rgc, XFontStruct *axis_fst,
                    int xstart, int ystart, int w, int h, int axis_pos,
					time_t t_start, time_t t_end)
{
    int x,y;

    display = dpy;
    xid = dr;
    fore_gc = fgc;
    back_gc = bgc;
    ref_gc = rgc;
    afst = axis_fst;

	plot_start_x = xstart;
	plot_start_y = ystart;
	plot_width = w;
	plot_height = h;
	axis_position = axis_pos;
	axis_height = axis_pos - plot_start_y;
	label_pos_x = xstart + (plot_width/20);
	label_pos_y = ystart + (plot_height/20);
	epoch_start = t_start;
	epoch_end = t_end;

    // Clear the canvas
    XFillRectangle(display,xid,back_gc,plot_start_x,plot_start_y,plot_width,plot_height);

    // Draw the time axis - Establishes the mapping between time and pixels
    draw_time_axis(fore_gc,axis_pos);
}

//////////////
// DESTRUCTOR
  
TimePlot::~TimePlot()
{
}

////////////////////////////////////////////////////////////////////////
// DRAW_DATA: 
// 
//

void TimePlot::Draw_data( const vector<double> d, const vector<time_t> t,  GC gc )
{
  int i,n_points;
  XPoint *bpt = new XPoint[d.size()];
  
  for(i=0, n_points = 0; i < d.size(); i++) {
	  bpt[n_points].x = int (t[i] * pixels_per_sec  + pps_bias);
	  bpt[n_points++].y = int (d[i] * pixels_per_value  + ppv_bias);
  }

  XDrawLines(display,xid,gc,bpt,n_points,CoordModeOrigin);

  delete [] bpt;
}
////////////////////////////////////////////////////////////////////////
// DRAW_Label: 
// 
//

void TimePlot::Draw_label( GC gc,const char *label)
{
  int direct,ascent,descent;
  XCharStruct overall;

  if(label == NULL) return;
  XTextExtents(afst,label,strlen(label),
		 &direct,&ascent,&descent,&overall);
  XDrawImageString(display,xid,fore_gc,label_pos_x,label_pos_y, label ,strlen(label));

  label_pos_y += overall.ascent +5;

}
///////////////////////////////////////////////////////////////////////
// DRAW_TITLE: - DO not use % characters in title - Used to format
// Time stamps. 
// 
//

void TimePlot::Draw_title( GC gc,const char *label)
{
  int direct,ascent,descent,x_pos;
  XCharStruct overall;
  struct tm tms;
  if(label == NULL) return;

  // Draw the title
  XTextExtents(afst,label,strlen(label),&direct,&ascent,&descent,&overall);
  x_pos = label_pos_x + overall.width + 10;
  XDrawImageString(display,xid,fore_gc,label_pos_x,overall.ascent+2, label ,strlen(label));

  char   tlabel1[256];
  char   tlabel2[256];
  // Add the  plot time range to the title.
  strftime(tlabel1,256,"   %H:%M %Y-%m-%d",gmtime_r(&epoch_start,&tms));
  strcat(tlabel1," to %H:%M %Y-%m-%d");
  strftime(tlabel2,256,tlabel1,gmtime_r(&epoch_end,&tms));

  XTextExtents(afst,tlabel2,strlen(tlabel2),
		 &direct,&ascent,&descent,&overall);

  XDrawImageString(display,xid,fore_gc,x_pos,overall.ascent+2, tlabel2 ,strlen(tlabel2));

}
////////////////////////////////////////////////////////////////////////
// DRAW_TIME_AXIS: 
//

void TimePlot::draw_time_axis(GC gc, int y_pos)
{
    int x1,y1;
    int text_period;
    int tick_period;
    time_t value;

    Font font;
    int direct,ascent,descent;
    XCharStruct overall;

    char *fmt_str;
    char *fmt_str0z;
    char string_buf[128];
    time_t t_start;
    time_t t_end;

    // Range of scale is 2.5 the movie loop length
    t_start = epoch_start;
    t_end = epoch_end;

    pixels_per_sec = (double) plot_width / (t_end - t_start);
    pps_bias = -(pixels_per_sec * t_start) + plot_start_x;

    // Pick the appropriate spacing for labels
    
    if(pixels_per_sec < 0.00001) {
        text_period = 2635200; // 30.5 Days
        tick_period = 604800;  // 7 days
        fmt_str0z = "%m/%d";
        fmt_str = fmt_str0z;
    } else if(pixels_per_sec < 0.00005) {
        text_period = 1728000; // 20 Days
        tick_period = text_period / 4;
        fmt_str0z = "%m/%d";
        fmt_str = fmt_str0z;
    } else if(pixels_per_sec < 0.0001) {
        text_period = 604800; // 7 days
        tick_period = text_period / 7;
        fmt_str0z = "%m/%d";
        fmt_str = fmt_str0z;
    } else if(pixels_per_sec < 0.0004) {
        text_period = 345600 ; // 4 days
        tick_period = text_period / 4 ;
        fmt_str0z = "%m/%d";
        fmt_str = fmt_str0z;
    } else if(pixels_per_sec < 0.0008) {
        text_period = 172800 ; // 2 days
        tick_period = text_period / 4 ;
        fmt_str0z = "%m/%d";
        fmt_str = fmt_str0z;
    } else if(pixels_per_sec < 0.001) {
        text_period = 86400; // 1 day
        tick_period = text_period / 4;
        fmt_str0z = "%m/%d";
        fmt_str = fmt_str0z;
    } else if(pixels_per_sec < 0.003) {
        text_period = 43200; // 12 hours
        tick_period = text_period / 12;
        fmt_str = " %HZ";
        fmt_str0z = " 0Z %m/%d";
    } else if(pixels_per_sec < 0.006) {
        text_period = 14400; // 4 hours
        tick_period = text_period / 4;
        fmt_str = " %HZ";
        fmt_str0z = " %m/%d";
    } else if(pixels_per_sec < 0.015) {
        text_period = 7200; // 2 hours
        tick_period = text_period / 4;
        fmt_str = " %HZ";
        fmt_str0z = " 0Z %m/%d";
    } else if(pixels_per_sec < 0.05) {
        text_period = 3600; // 1 hours
        tick_period = text_period / 4;
        fmt_str = " %HZ";
        fmt_str0z = " %HZ %m/%d";
    } else if(pixels_per_sec < 0.15) {
        text_period = 1800; //  30 min
        tick_period = text_period / 6;
        fmt_str = " %H:%M";
        fmt_str0z = " %HZ %b %d";
    } else {
        text_period = 900; // 15 min 
        tick_period = text_period / 3;
        fmt_str = " %H:%M";
        fmt_str0z = " %HZ %b %d";
    }

  if(debug) {
    fprintf(stderr,"Pixels_per_sec: %f  text_period: %d tick_period: %d\n",
    	   pixels_per_sec,text_period,tick_period);
   }

    XDrawLine(display,xid,gc,plot_start_x,y_pos,plot_start_x + plot_width,y_pos); // Draw the base 

	y1 = y_pos;
    /* Plot a tick every x minutes */
    value = t_start - (t_start % tick_period);
    while ( value < t_end) {
        value += tick_period; 
        x1 =(int) (pixels_per_sec * value + pps_bias);
	if((value % text_period) == 0) {
               XDrawLine(display,xid,gc,x1,y1-3,x1,y1+4); // Draw a tick

	    if((value % 86400) == 0) { /* Plot a 0 Hr label */
               strftime(string_buf,128,fmt_str0z,gmtime(&(value)));
               XTextExtents(afst,string_buf,strlen(string_buf),
		          &direct,&ascent,&descent,&overall);
               XDrawImageString(display,xid,gc, x1 - (overall.width /2) , y1 + overall.ascent + 5 ,string_buf ,strlen(string_buf));
	    } else {
               XDrawLine(display,xid,gc,x1,y1-3,x1,y1+3); // Draw a tick

               strftime(string_buf,128,fmt_str,gmtime(&(value)));
               XTextExtents(afst,string_buf,strlen(string_buf),
		          &direct,&ascent,&descent,&overall);
               XDrawImageString(display,xid,gc, x1 - (overall.width /2) ,  y1 + overall.ascent + 5 ,string_buf ,strlen(string_buf));
            }
    } else {
        if(value >= t_start) XDrawLine(display,xid,gc,x1,y1,x1,y1-3); // Draw a tick
    }
    
  }

}
    
//////////////////////////////////////////////////////////////////////////
// DRAW_V_REF_LINE: Draw a vertical reference Line with a label at the indicated
// Y position. Place it at the proper place along the time line.
// Set label to NULL for no label.

void TimePlot::draw_v_ref_line(time_t t, int y_pos, char * label,GC gc)
{
  int x_pos;
  int label_pos;

  // Compute the position along the time line.
  x_pos = (int) (t * pixels_per_sec  + pps_bias);

  // Draw a vertical Reference Line
  XDrawLine(display,xid,gc,x_pos,plot_start_y,x_pos,plot_start_y + plot_height);

  if(label == NULL) return;

  int direct,ascent,descent;
  XCharStruct overall;
  XGCValues gcv;
  Font f;

  // Record the GC's font and then set it to the Axis font.
  XGetGCValues(display,gc,GCFont,&gcv);
  f = gcv.font;
  XSetFont(display, gc, afst->fid);
  
  XTextExtents(afst,label,strlen(label),
	 &direct,&ascent,&descent,&overall);

 // Center the label on the line
 label_pos = x_pos - (overall.width /2);

 // Make sure it's on the screen.
 if(label_pos <= plot_start_x) {
	 label_pos = plot_start_x;
 }

 if(label_pos >= (plot_start_x + plot_width - overall.width)) {
	 label_pos = plot_start_x + plot_width - overall.width;
 }

 // Plot the Label
 XDrawImageString(display,xid,gc,
		     label_pos ,y_pos,
		     label ,strlen(label));

  // Restore the original GC Font
  XSetFont(display, gc, f);

}
//////////////////////////////////////////////////////////////////////////
// DRAW_REF_LINE: Draw a  Horizontalreference Line with a label at the indicated
// X position. Place it at the indicated value along the ordinate axis.
//

void TimePlot::draw_h_ref_line(double val, int x_pos, char * label,GC gc)
{
  int y_pos;
  int label_pos;

  // Compute the position along the time line.
  y_pos = int (val * pixels_per_value  + ppv_bias);

  // Draw a vertical Reference Line
  XDrawLine(display,xid,gc,plot_start_x,y_pos,plot_start_x + plot_width,y_pos);

  if(label == NULL) return;

  int direct,ascent,descent;
  XCharStruct overall;
  XGCValues gcv;
  Font f;

  // Record the GC's font and then set it to the Axis font.
  XGetGCValues(display,gc,GCFont,&gcv);
  f = gcv.font;
  XSetFont(display, gc, afst->fid);
  
  XTextExtents(afst,label,strlen(label),
	 &direct,&ascent,&descent,&overall);

 // Center the label on the line
 label_pos = y_pos - (ascent /2);

 // Make sure it's on the screen.
 if(label_pos <= plot_start_y) {
	 label_pos = plot_start_y;
 }

 if(label_pos >= (plot_start_y + plot_height - ascent)) {
	 label_pos = plot_start_y + plot_height - ascent;
 }

 // Plot the Label
 XDrawImageString(display,xid,gc,
		     x_pos,label_pos,
		     label ,strlen(label));

  // Restore the original GC Font
  XSetFont(display, gc, f);

}

//////////////////////////////////////////////////////////////////////////
// DRAW_Y_SCALE: Set up a mapping between pixel space and data units
// And Draw vertical scale at the indicated position along the X axis.
//
//

void TimePlot::draw_y_scale(int x_pos, double data_min, double data_max, char *units)
{
  int y1;
  int plot_right = 0;
  double data_range, scale_range; 
  double tick_base,tick_delta;
  double value;
  int direct,ascent,descent;
  XCharStruct overall;
  char string_buf[128];
  
  data_range = data_max - data_min;
  if (data_range < 1.0) {
    data_range = 1.0;
  }

  // Axes on left - labels on  left, Axes on right, labels on right.
  if(x_pos > (plot_width / 2))  plot_right = 1;

  scale_range = compute_scale_range(data_range);
  
  /* Pick a nice even tick delta */
  tick_delta = compute_tick_interval(data_range);

  double average = (data_min + data_max) / 2.0;
    
  /* center the range over the average */
  tick_base = average - (scale_range / 2.0);

  if(data_min == 0) tick_base = 0;
  
  if(tick_base > data_min) tick_base = data_min;  
  
  /* adjust the scale bottom to the nearest tick_delta units */
  tick_base  = nearest(tick_base,tick_delta);
  if(tick_base > data_min) tick_base -= tick_delta;  
  
  while(tick_base + scale_range < data_max)  {
    scale_range += tick_delta;
  }
  if(scale_range > data_range) data_range = scale_range;
  if(scale_range < data_range) scale_range = data_range;
  
  if(debug) {
    fprintf(stderr,"Min: %g, Max: %g Range: %g\n",
	    data_min,data_max,data_range);
  }
  
  
  /* Compute the slope and intercept (bias) of the mapping function */

  pixels_per_value = -(axis_height / scale_range);
  ppv_bias = (plot_start_y + axis_height) - (pixels_per_value * tick_base);

  if(debug) {
    fprintf(stderr,"Tick_base: %g, tick_delta: %g, Scale_range: %g pix_per_val: %g bias: %g\n",
	    tick_base,tick_delta,scale_range,pixels_per_value,ppv_bias);
  }
  // Draw the vertical Axes
  XDrawLine(display,xid,fore_gc,x_pos,plot_start_y,x_pos,plot_start_y + plot_height);

  /* Draw a set of tick marks; right, or left */
  for(value = tick_base; value <= (tick_base + scale_range); value += tick_delta) {
      y1 = (int) (pixels_per_value * value + ppv_bias);
      XDrawLine(display,xid,fore_gc,x_pos-2,y1,x_pos+2,y1);

      /* Label  scale */
      sprintf(string_buf,"%.0f",value);
      XTextExtents(afst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);
      if(plot_right) {
          XDrawString(display,xid,fore_gc,
		     (x_pos + 4),y1 + (overall.ascent / 2),
		     string_buf ,strlen(string_buf));
      } else {
          XDrawString(display,xid,fore_gc,
		     (x_pos - 4 - overall.width),y1+ (overall.ascent / 2)  ,
		     string_buf ,strlen(string_buf));
      }
      draw_h_ref_line(value,0,NULL,ref_gc);
  }

  // Add a units label
  if(plot_right) {
      sprintf(string_buf,"%s",units);
      XTextExtents(afst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);
          XDrawImageString(display,xid,fore_gc,
		     x_pos + 2, overall.ascent + 2,
		     string_buf ,strlen(string_buf));
  } else {
      sprintf(string_buf,"%s",units);
      XTextExtents(afst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);
          XDrawImageString(display,xid,fore_gc,
		     (x_pos - 1 - overall.width),overall.ascent + 2,
		     string_buf ,strlen(string_buf));
  }

}

////////////////////////////////////////////////////////////////////////
// NEAREST: Compute the value nearest the target which is divisible by
//         the absolute value of delta
//
 
double TimePlot::nearest(double target,double delta)
{
    double answer;
    double rem;
 
    delta = fabs(delta);
    rem = fmod(target,delta);
 
    if(target >= 0.0) {
        if(rem > (delta / 2.0)) {
           answer = target + (delta - rem);
        } else {
          answer = target -  rem;
        }
    } else {
        if(fabs(rem) > (delta / 2.0)) {
           answer = target - (delta + rem);
        } else {
          answer = target -  rem;
        }
    }
 
    return answer;
}
 
//////////////////////////////////////////////////////////////////////////
// COMPUTE_TICK_INTERVAL: Return the tick interval given a range
//        Range Assumed to be > 1.0 && < 10000.0
//
 
double TimePlot::compute_tick_interval(double range)
{
    double    arange = fabs(range);
 
    if(arange <= 0.5) return (0.1);
    if(arange <= 1.0) return (0.25);
    if(arange <= 2.0) return (0.5);
    if(arange <= 5.0) return (1.0);
    if(arange <= 10.0) return (2.0);
    if(arange <= 30.0) return (5.0);
    if(arange <= 50.0) return (10.0);
    if(arange <= 100.0) return (20.0);
    if(arange <= 300.0) return (50.0);
    if(arange == 360.0) return (90.0);
    if(arange <= 1500.0) return (100.0);
    if(arange <= 3000.0) return (200.0);
    if(arange <= 7500.0) return (500.0);
    return(1000.0);
}

//////////////////////////////////////////////////////////////////////////
// COMPUTE_SCALE_RANGE: Return a range for a scale based on the dynimic range
//   Range Assumed to be > 1.0 && < 10000.0
//
 
double TimePlot::compute_scale_range(double range)
{
    double    arange = fabs(range);
 
    if(arange <= 1.0) return (1.0);
    if(arange <= 2.0) return (2.0);
    if(arange <= 5.0) return (5.0);
    if(arange <= 10.0) return (10.0);
    if(arange <= 20.0) return (20.0);
    if(arange <= 25.0) return (25.0);
    if(arange <= 40.0) return (40.0);
    if(arange <= 50.0) return (50.0);
    if(arange <= 100.0) return (100.0);
    if(arange <= 200.0) return (200.0);
    if(arange <= 360.0) return (360.0);
    if(arange <= 400.0) return (400.0);
    if(arange <= 500.0) return (500.0);
    return(1000.0);
}
