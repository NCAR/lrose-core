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


// #include <X11/Xlib.h>
// #include <X11/Xutil.h>
// #include <X11/Xatom.h> 
// #include <xview/canvas.h> 

#include "cidd_macros.h" 
#include "cidd_colorscales.h" // for Color_gc_t def

#include "TimeList.hh"
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////
// class definition

class TimePlot

{

public:

  // default constructor
  
  TimePlot(Color_gc_t &background,
			Color_gc_t &axis,
			Color_gc_t &frame_bar,
			Color_gc_t &epoch_ind,
			Color_gc_t &now_line,
			Color_gc_t *time_tick1,
			Color_gc_t *time_tick2,
			Color_gc_t *time_tick3,
			Color_gc_t *time_tick4,
			Color_gc_t *time_tick5,
			Color_gc_t *time_tick6);

  void Init(Display *dpy, GC gc, Colormap cmap /* , Canvas canvas */);
  
  void Set_times(time_t ep_start, time_t ep_end,
                 time_t fr_start, time_t fr_end,
		 time_t fr_len, int nfr);

  void Event_handler(long x_pixel, long y_pixel, long event_id, long button_up);

  // Add this list of grid data times to the plot
  void add_grid_tlist(string lab, time_t *tlist, int ntimes,time_t data_time);
  void clear_grid_tlist();   // Clear the list of lists

  // Add this list of product data times to the plot
  void add_prod_tlist(string lab, const vector<time_t> &t);
  void clear_prod_tlist();   // Clear the list of lists

  void Draw();

  // destructor
  virtual ~TimePlot();

 typedef enum {
     OUTSIDE = 0,
     LEFT_EXTEND,
     CENTER_SHIFT,
     RIGHT_EXTEND
 } Time_shift_mode_t;

  // access functions

  unsigned int getWidth() { return width; }
  unsigned int getHeight() { return height; }
  double getPixelsPerSec() { return pixels_per_sec; }
  time_t getEpochStart() { return epoch_start; }
  time_t getEpochEnd() { return epoch_end; }
  time_t getFrameStart() { return frame_start; }
  time_t getFrameEnd() { return frame_end; }
  int getNFrames() { return nframes; }
protected:
  
private:
  Display *display;  // X11 related Members
  Colormap cmap;
  GC gc;

  Drawable can_xid;  // The visible Canvas's pixmap xid
  Drawable draw_xid; // A "backing" pixmap for the above pixmap

  // Canvas canvas;  // Xview Canvas


  unsigned int win_width;     // Size of window
  unsigned int win_height;
  unsigned int width;         // Size of canvas
  unsigned int height;

  double pixels_per_sec;  // Constant for scaling function Pixel_x =  pixels_per_sec * time_t + pps_bias
  double pps_bias;           // Y intercept of the scaling function.

  time_t epoch_start;
  time_t epoch_end;
  time_t frame_start;
  time_t frame_end;
  time_t frame_len;
  int    nframes;

  int drag_in_progress;
  time_t center_offset;   // Seconds from center the epoch time scale is.
  time_t left_offset;     // Seconds from normal left edge of the epoch time scale.
  time_t right_offset;    // Seconds from normal right edge of the epoch time scale.
  time_t time_scale_offset; // Seconds from normal center of the world time scale


  Color_gc_t &background_color;
  Color_gc_t &axis_color;
  Color_gc_t &frame_bar_color;
  Color_gc_t &epoch_ind;
  Color_gc_t &now_line;
  Color_gc_t *time_tick_color[6];

  int    row_height;    // Height of one row.
  int    num_rows;      // current number of rows

  void draw_time_axis();
  void draw_time_lists();
  void draw_epoch_indicator();

  int start_grid_tlist_elem;   // Offsets to start of list
  int start_prod_tlist_elem;

  vector<TimeList> grid_tlist;
  vector<TimeList> prod_tlist;

};

#endif


