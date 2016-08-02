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
////////////////////////////////////////////////////////////////
// TimePlot.hh
//
//
////////////////////////////////////////////////////////////////

#ifndef TimePlot_HH
#define TimePlot_HH

using namespace std;

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h> 

#include <string>
#include <vector>

///////////////////////////////////////////////////////////////
// class definition

class TimePlot

{

public:

  // default constructor
  
  TimePlot();

  void set_debug_level(int level);

  void Init(Display *dpy, Drawable dr, GC def_gc, int fgc, int bgc, XFontStruct *axis_fst,   // Display, Xid and Axis/label GC's
			int xstart, int ystart, int w, int h, int axis_pos, // Plot location, width, height & axis locs.
			time_t t_start, time_t t_end); // Left, right edges - seconds since 1970-1-1
  

  // Draw a time series Line Plot
  void Draw_data(const vector<double> d, const vector<time_t> t,  GC gc, char * label, double bad_val);

  // Draw a Y-X plot - X as a function of Y (ht)
  void Draw_yx_plot(int x1, int x2, double scale_min, double scale_max,
							  const vector<double> d, const vector<double> ht, GC gc, double bad_val);

  void Draw_yx_legend(int x1, int x2, double scale_min, double scale_max, double ht, GC gc,
			   char * label, int draw_labels_on_top);

  // Draw Wind Barbs at given heights and times.
  void Draw_barbs(const vector<double> u, const vector<double> v, const vector<double> ht,  const vector<time_t> t,GC gc, double bad_val);

  // Draw Colored Cell at given height and time.
  void Draw_cell(time_t t1,time_t t2,double ht1, double ht2, GC gc);

  // Draw Colored Cell - in a 3-D "view"  at given height and time.
  void Draw_cell_3d(time_t t1,time_t t2,double ht1, double ht2, GC gc);

  // Draw a 3-D Cell Boundry
  void Draw_cell_border_3d(time_t t1,time_t t2,double ht1, double ht2, GC gc);

  void draw_y_scale(int x_pos, double data_min, double data_max, char * units,int plot_right); 

  void draw_ref_line(time_t t, int y_pos, char * label,GC gc); 

  // return the X pixel for a given time;
  int get_pix_x(time_t t);

  // return the Y pixel for a given height;
  int get_pix_y(double ht);

  // destructor
  virtual ~TimePlot();

protected:
  
private:
  Display *display;  // X11 related Members
  Drawable xid;      // The visible Canvas's pixmap xid
  GC gc;             // For drawing Axes and Labels
  XFontStruct *afst; // axis font struct


  int debug;
  int plot_start_x;       // Upper left position of plot on canvas
  int plot_start_y;
  int plot_width;         // width of drawable area
  int plot_height;        // height of drawable area
  int axis_height;        // height of data area
  int axis_position;      // Y position of Time axis

  int foreground_color;  // Color of axes, etc
  int background_color;  // Color of background

  double pixels_per_sec;  // Constant for scaling function Pixel_x =  pixels_per_sec * time_t + pps_bias
  double pps_bias;        // Y intercept of the scaling function.

  double pixels_per_value; // Constant for scaling function Pixel_y =  pixels_per_value * value + ppv_bias
  double ppv_bias;        // Intercept of the scaling function.

  time_t epoch_start;     // Left edge of time plot
  time_t epoch_end;       // right edge of plot

  void draw_time_axis(int y_pos);
  double nearest(double target,double delta);
  double compute_scale_range(double range);
  double compute_tick_interval(double range);

};

#endif


