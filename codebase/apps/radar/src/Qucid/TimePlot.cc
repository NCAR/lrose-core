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
// TimePlot.cc
//
// Container object holding context for product rendering.
//
// Jan 2000
//
//////////////////////////////////////////////////////////

#include "TimePlot.hh"
#include "TimeList.hh"

#include "cidd.h"
#include <rapformats/coord_export.h>

//////////////////////////////////////////
// DEFAULT CONSTRUCTOR

TimePlot::TimePlot(Color_gc_t &background,
                    Color_gc_t &axis,
                    Color_gc_t &frame_bar,
                    Color_gc_t &epoch_indicator,
                    Color_gc_t &now,
                    Color_gc_t *time_tick1,
                    Color_gc_t *time_tick2,
                    Color_gc_t *time_tick3,
                    Color_gc_t *time_tick4,
                    Color_gc_t *time_tick5,
                    Color_gc_t *time_tick6) :
    background_color(background),
    axis_color(axis),
    frame_bar_color(frame_bar),
    epoch_ind(epoch_indicator),
    now_line(now)

{
    time_tick_color[0] = (time_tick1);
    time_tick_color[1] = (time_tick2);
    time_tick_color[2] = (time_tick3);
    time_tick_color[3] = (time_tick4);
    time_tick_color[4] = (time_tick5);
    time_tick_color[5] = (time_tick6);

    drag_in_progress = 0;
    center_offset = 0;
    left_offset = 0;
    right_offset = 0;
    time_scale_offset = 0;

    epoch_start = 0;
    epoch_end = 0;
    frame_start = 0;
    frame_end = 0;
    frame_len = 0;
    nframes = 0;
}


//////////////////////////////////////////
// INIT: Set up the Time plot graphics, etc
//
void TimePlot::Init(Display *dpy, GC gc, Colormap cmap /* , Canvas canvas */)
{
    Window  root; // Root window ID of drawable
    unsigned int    border_width,depth; 
    int x,y;


    display = dpy;
    cmap = cmap;
    gc = gc;
    // canvas = canvas;

    // win_height = xv_get(gd.movie_pu->movie_pu,WIN_HEIGHT);

    // can_xid = xv_get(canvas_paint_window(canvas),XV_XID);

    XGetGeometry(display,can_xid,&root,&x,&y,&width,&height,&border_width,&depth);

    drag_in_progress = 0;
    center_offset = 0;
    left_offset = 0;
    right_offset = 0;
    time_scale_offset = 0;
    start_grid_tlist_elem = 0;
    start_prod_tlist_elem = 0;

    num_rows = 2;
    // This version allows the axis 25%, the epoch indicator 25% and two
    // Rows at 25% each
    row_height = height / 4 - 1;
}

//////////////
// DESTRUCTOR
  
TimePlot::~TimePlot()
{
     prod_tlist.clear();
     grid_tlist.clear();
}

////////////////////////////////////////////////////////////////////////
// ADD_GRID_TLIST: Add this list of data times to the master list
//

void TimePlot::add_grid_tlist(string lab,time_t *t, int  ntimes, time_t data_time) 
{
    TimeList mytl(lab, t, ntimes, data_time);

    grid_tlist.push_back(mytl);
}


////////////////////////////////////////////////////////////////////////
// CLEAR_GRID_TLIST: Clear the list of data times
//

void TimePlot::clear_grid_tlist() 
{
     grid_tlist.clear();
}


////////////////////////////////////////////////////////////////////////
// ADD_PROD_TLIST: Add this list of data times to the master list
//

void TimePlot::add_prod_tlist(string lab,
			      const vector<time_t> &t) 
{
    TimeList mytl(lab, t, 0);
    prod_tlist.push_back(mytl);
}


////////////////////////////////////////////////////////////////////////
// CLEAR_PROD_TLIST: Clear the list of data times
//

void TimePlot::clear_prod_tlist() 
{
     prod_tlist.clear();
}


////////////////////////////////////////////////////////////////////////
// SET_TIMES
//

void TimePlot::Set_times(time_t ep_start, time_t ep_end, 
                         time_t fr_start, time_t fr_end,
			 time_t fr_len, int nfr)
{
    epoch_start = ep_start;
    epoch_end = ep_end;
    frame_start = fr_start;
    frame_end = fr_end;
    frame_len = fr_len;
    nframes = nfr;

    //printf("\nepoch_start: %s",asctime(gmtime(&epoch_start)));
    //printf("epoch_end: %s",asctime(gmtime(&epoch_end)));
    //printf("frame_start: %s",asctime(gmtime(&frame_start)));
    //printf("frame_end: %s",asctime(gmtime(&frame_end)));
    //printf("frame_len: %d\n",frame_len);
    //printf("nframes: %d\n",nframes);     
}

////////////////////////////////////////////////////////////////////////
// Event_handler
//
//
void TimePlot::Event_handler(long x_pixel, long y_pixel, long event_id, long button_up)
{
    static int last_down_x = -1;
    static int last_down_y = -1;

    // printf("X,Y: %d,%d     ID:%d    UP:%d\n",x_pixel,y_pixel, event_id, button_up);

    switch(event_id) {
	case 32563:   // Xview Select 
	  if(button_up == 0) {
	      last_down_x = x_pixel;
	      last_down_y = y_pixel;

	      // Compute where the epoch indicator is
              int y1 = 2;   
              int y2 =  row_height;
              int thick = (y2 - y1); 

              int x1 =  (int) ((epoch_start + left_offset + center_offset) * pixels_per_sec + pps_bias);
              int x2 =  (int) ((epoch_end + right_offset + center_offset) * pixels_per_sec + pps_bias);

	      if(y_pixel >= y1 && y_pixel <= y2) { // could be in the indicator bar
		  if(x_pixel >= x1-thick && x_pixel <= x1) {
		      drag_in_progress = LEFT_EXTEND;
		  } else if(x_pixel > x1 && x_pixel < x2) {
		      drag_in_progress = CENTER_SHIFT;
		  } else if(x_pixel >= x2 && x_pixel <= x2+thick) {
		      drag_in_progress = RIGHT_EXTEND;
		  } else {
		      drag_in_progress = OUTSIDE;
		  }
	      }

	  } else {  // Button released
	      if((abs(x_pixel-last_down_x) < 10) && (abs(y_pixel-last_down_y) < 10)) {
		  // Was a simple click and release
		  time_t val = (time_t)((last_down_x - pps_bias) / pixels_per_sec);
		  if(val <= epoch_end && val >= epoch_start) {
		      int frame = (int) (((val - epoch_start) / frame_len) + 1.5); 
		      if(frame < 0) frame = 0; 
		      if(frame > nframes) frame = nframes;

		      // xv_set(gd.movie_pu->movie_frame_sl, PANEL_VALUE, frame, NULL);
		      // movie_frame_proc(gd.movie_pu->movie_frame_sl,frame, (Event*)NULL);
		  }
	      } else {
	          switch(drag_in_progress) {
		    int i,j;
		    int found;
		    int old_frames;
		    time_t    target_time;
		    movie_frame_t    tmp_frame[MAX_FRAMES]; // info about each frame

	            case LEFT_EXTEND: // Start time and # of frames has changed
                        target_time = gd.movie.start_time;
			target_time += (time_t) ((x_pixel - last_down_x)/ pixels_per_sec);
			target_time -= (gd.movie.start_time % gd.movie.round_to_seconds);

                        // Place start time on a nice even interval
			gd.movie.start_time = target_time - (target_time % (int)(gd.movie.time_interval * 60));

			// handle flip of beginning and end
			if(gd.movie.start_time > epoch_end) {
			    epoch_start = epoch_end;
			    epoch_end = gd.movie.start_time;
			    gd.movie.start_time = epoch_start;
			} else {
			    epoch_start = gd.movie.start_time;
			}

			// Record the time we're currently on
			target_time = gd.movie.frame[gd.movie.cur_frame].time_mid;

                        old_frames = gd.movie.num_frames;

			gd.movie.num_frames = (epoch_end - epoch_start) / frame_len;
			if(gd.movie.num_frames > MAX_FRAMES) gd.movie.num_frames = MAX_FRAMES;
			if(gd.movie.num_frames < 2) gd.movie.num_frames = 2;
			gd.movie.end_frame = gd.movie.num_frames -1;

                        // Make a temporary copy
                        memcpy(tmp_frame,gd.movie.frame,sizeof(movie_frame_t) * MAX_FRAMES);

                        // Zero out global array
                        memset(gd.movie.frame,0,sizeof(movie_frame_t) * MAX_FRAMES);
                    
                        if(gd.movie.num_frames > old_frames) {
                            // copy original frames
                            for(i = gd.movie.num_frames - old_frames, j = 0; j < old_frames; i++, j++) {
                                memcpy(&gd.movie.frame[i],&tmp_frame[j],sizeof(movie_frame_t));
                
                                // Render time selector in reused frame
                                draw_hwin_bot_margin(gd.movie.frame[i].h_xid,gd.h_win.page,
                                                     gd.movie.frame[i].time_start,
                                                     gd.movie.frame[i].time_end);
                            }
                
                            // Mark new frames for allocation & redrawing
                            j = gd.movie.num_frames - old_frames;
                            for(i = 0; i < j; i++) {
                                 memset(&gd.movie.frame[i],0,sizeof(movie_frame_t));
                                 gd.movie.frame[i].redraw_horiz = 1;
                                 gd.movie.frame[i].redraw_vert = 1;
                            }
                        } else {
                            for(i = 0, j = old_frames - gd.movie.num_frames; j < old_frames; i++, j++) {
                                memcpy(&gd.movie.frame[i],&tmp_frame[j],sizeof(movie_frame_t));
                                // Render time selector in reused frame
                                draw_hwin_bot_margin(gd.movie.frame[i].h_xid,gd.h_win.page,
                                     gd.movie.frame[i].time_start,
                                     gd.movie.frame[i].time_end);
                            }
                            // Copy unused frames too so they get de-allocated
                            for(j = 0, i = gd.movie.num_frames; j < old_frames - gd.movie.num_frames; i++, j++) {
                                memcpy(&gd.movie.frame[i],&tmp_frame[j],sizeof(movie_frame_t));
                            }
                        }

                        // Fill in time points on global array
                        reset_time_points();

                        for(i=0; i < gd.movie.num_frames; i++) {
                           if(target_time >= gd.movie.frame[i].time_start &&
                              target_time <= gd.movie.frame[i].time_end) {
                               gd.movie.cur_frame = i;
                           }
                        }       

			// Reset gridded and product data validity flags
			invalidate_all_data();
			adjust_pixmap_allocation(); 
			update_movie_popup();
	            break;

	            case CENTER_SHIFT: // Start time has changed
			gd.movie.start_time += (time_t) ((x_pixel - last_down_x)/ pixels_per_sec);
			gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);


			// Record the time we're currently on
			target_time = gd.movie.frame[gd.movie.cur_frame].time_mid;

			// Make a temporary copy

			gd.movie.mode = ARCHIVE_MODE;
			gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
			gd.coord_expt->time_seq_num++;

			// xv_set(gd.movie_pu->movie_type_st,PANEL_VALUE,ARCHIVE_MODE,NULL);

                       // Make a temporary copy
                        memcpy(tmp_frame,gd.movie.frame,sizeof(movie_frame_t) * MAX_FRAMES);

                        // Zero out global array
                        memset(gd.movie.frame,0,sizeof(movie_frame_t) * MAX_FRAMES);
                    
                        // Fill in time points on global array
                        reset_time_points();

                       // Search for frames already rendered for this interval and use them
                       for(i=0 ; i < gd.movie.num_frames; i++) {
                           found = 0;
                           for(j=0; j < MAX_FRAMES && !found; j++) {
                               if(gd.movie.frame[i].time_start == tmp_frame[j].time_start) {
                                   found = 1;
                                   memcpy(&gd.movie.frame[i],&tmp_frame[j],sizeof(movie_frame_t));
                                   // Render a new time selector for this frame
                                   draw_hwin_bot_margin(gd.movie.frame[i].h_xid,gd.h_win.page,
                                     gd.movie.frame[i].time_start,
                                     gd.movie.frame[i].time_end);
                                   memset(&tmp_frame[j],0,sizeof(movie_frame_t));
                               }

                           }
                       }

                       // Reuse pixmaps in unused frames
                       for(i=0 ; i < gd.movie.num_frames; i++) {
                           if(gd.movie.frame[i].h_xid) continue; // Alreday is accounted for.

                           found = 0;
                           for(j=0; j < MAX_FRAMES && !found; j++) {
                               if(tmp_frame[j].h_xid) {
                                   found = 1;
                                   gd.movie.frame[i].h_xid = tmp_frame[j].h_xid;
                                   gd.movie.frame[i].v_xid = tmp_frame[j].v_xid;
                                   gd.movie.frame[i].redraw_horiz = 1;
                                   gd.movie.frame[i].redraw_vert = 1;
                                   memset(&tmp_frame[j],0,sizeof(movie_frame_t));
                               }
                           }
                       }
                   
                       gd.movie.cur_frame = 0;
                       for(i=0; i < gd.movie.num_frames; i++) {
                           if(target_time >= gd.movie.frame[i].time_start &&
                              target_time <= gd.movie.frame[i].time_end) {

                               gd.movie.cur_frame = i;
                           }
                       }
 
			invalidate_all_data();
			update_movie_popup();
	            break;

	            case RIGHT_EXTEND: // end time and # of frames has changed
			epoch_end += (time_t) ((x_pixel - last_down_x)/ pixels_per_sec);
			epoch_end -= (epoch_end % gd.movie.round_to_seconds);

			// handle flip of beginning and end
			if(gd.movie.start_time > epoch_end) {
			    epoch_start = epoch_end;
			    epoch_end = gd.movie.start_time;
			    gd.movie.start_time = epoch_start;
			} else {
			    epoch_start = gd.movie.start_time;
			}

			old_frames = gd.movie.num_frames;
			gd.movie.num_frames = (epoch_end - epoch_start) / frame_len;
			if(gd.movie.num_frames > MAX_FRAMES) gd.movie.num_frames = MAX_FRAMES;
			if(gd.movie.num_frames < 2) gd.movie.num_frames = 2;
			gd.movie.end_frame = gd.movie.num_frames -1;

			// Record the time we're currently on
			target_time = gd.movie.frame[gd.movie.cur_frame].time_mid;

                        // Fill in time points on global array
                        reset_time_points();

                        if(gd.movie.num_frames > old_frames) {
                           for(i = gd.movie.num_frames -1; i < old_frames; i++) {
                                gd.movie.frame[i].redraw_horiz = 1;
                                gd.movie.frame[i].redraw_vert = 1;
                           }
                                   // Render time selector in reused frames
                           for(i= 0; i < old_frames; i++) {
                              draw_hwin_bot_margin(gd.movie.frame[i].h_xid,gd.h_win.page,
                                     gd.movie.frame[i].time_start,
                                     gd.movie.frame[i].time_end);
                           }
                        } else {
                           // Render time selector in reused frames
                           for(i= 0; i < gd.movie.num_frames; i++) {
                               draw_hwin_bot_margin(gd.movie.frame[i].h_xid,gd.h_win.page,
                                     gd.movie.frame[i].time_start,
                                     gd.movie.frame[i].time_end);
                           }
                        }

                         for(i=0; i < gd.movie.num_frames; i++) {
                           if(target_time >= gd.movie.frame[i].time_start &&
                              target_time <= gd.movie.frame[i].time_end) {

                               gd.movie.cur_frame = i;
                           }
                        }       

			adjust_pixmap_allocation(); 
			invalidate_all_data();
			update_movie_popup();
			drag_in_progress = 0;
	            break;

	            default: // do nothing
	            break;

	          }
	      }
	      center_offset = 0;
	      right_offset = 0;
	      left_offset = 0;
	      drag_in_progress = 0;
	  }
	break;

	case 32515:  // Xview LOC_DRAG
	    switch(drag_in_progress) {
	      case LEFT_EXTEND:
		  left_offset = (time_t) ((x_pixel - last_down_x)/ pixels_per_sec); 
	      break;

	      case CENTER_SHIFT:
		  center_offset = (time_t) ((x_pixel - last_down_x)/ pixels_per_sec);
	      break;

	      case RIGHT_EXTEND:
		  right_offset = (time_t) ((x_pixel - last_down_x)/ pixels_per_sec); 
	      break;

	      default: // do nothing
	      break;

	    }
            this->Draw();
	    
	break;
    }

}

#define TP_MAX_ROWS 10
////////////////////////////////////////////////////////////////////////
// DRAW_TIME_LISTS: 
//

void TimePlot::draw_time_lists()
{
    unsigned int j,index;
    int i;
    int x1,y1,y2;
    int color_index;
    time_t value;
    static int first_time = 1;

    int xmid,ymid;
    Font font;

    y1 =  row_height - 2;  // Bottom of the epoch indicator, up 2
    y2 = y1 + row_height ;

    if(first_time) {
        font = choose_font("PROD_123",300,row_height,&xmid,&ymid); 
        XSetFont(display,time_tick_color[0]->gc,font);
        XSetFont(display,time_tick_color[1]->gc,font);
        XSetFont(display,time_tick_color[2]->gc,font);
        XSetFont(display,time_tick_color[3]->gc,font);
        XSetFont(display,time_tick_color[4]->gc,font);
        XSetFont(display,time_tick_color[5]->gc,font);
	first_time = 0;
    }
    color_index = 0;


    // For each time list in the Product Data  vector
    for(i=0, index = start_prod_tlist_elem;
	  i < num_rows && index < prod_tlist.size();
	  i++, index++) {

	  // Plot a small tick
	  for(j=0; j < prod_tlist[index].tim.size(); j++) {
	      value = prod_tlist[index].tim[j];
	      x1 =(int) (pixels_per_sec * value + pps_bias);

              XDrawLine(display,can_xid,time_tick_color[color_index]->gc,x1,y1+4,x1,y2-4); // Draw a tick

	  }

	  // Draw the label - if there are ticks.
	  // if(prod_tlist[index].tim.size() > 0) 
	  	XDrawImageString(display,can_xid,time_tick_color[color_index]->gc,
			(width-100) , y2 - row_height / 5 ,
		    prod_tlist[index].label.c_str() ,
	        prod_tlist[index].label.size()); 

	  y1 += row_height;
	  y2 += row_height;
	  color_index++;
	  if(color_index >= 6) color_index = 0;
    }

    y2 = height - row_height - 4   ;  // Place this row  Just above the time axis
    y1 = y2 - row_height;

    // For each time list in the grid data  vector
    for(i=0, index = start_grid_tlist_elem;
	  i < num_rows && index < grid_tlist.size();
	  i++, index++) {

	  // Plot a small square
	  for(j=0; j < grid_tlist[index].tim.size(); j++) {
	      value = grid_tlist[index].tim[j];
	      x1 =(int) (pixels_per_sec * value + pps_bias);

	      if(grid_tlist[index].tim[j] == grid_tlist[index].data_time &&
		 gd.movie.movie_on == 0) {
	          // Draw a bigger square
	          XFillRectangle(display,can_xid,time_tick_color[color_index]->gc,x1-2,y2-7,5,5);
	      } else {
	          // Draw a tiny square
	          XFillRectangle(display,can_xid,time_tick_color[color_index]->gc,x1-1,y2-6,3,3);
	      }

	  }

	  // Draw the label;
	  XDrawImageString(display,can_xid,time_tick_color[color_index]->gc,2 , y2 - row_height / 5 ,
		      grid_tlist[index].label.c_str() ,
	              grid_tlist[index].label.size()); 

	  y1 -= row_height;
	  y2 -= row_height;
	  color_index++;
	  if(color_index >= 6) color_index = 0;
    }


}

////////////////////////////////////////////////////////////////////////
// DRAW_TIME_AXIS: 
//

void TimePlot::draw_time_axis()
{
    time_t x1,y1;
    time_t text_period;
    time_t tick_period;
    time_t value;

    int xmid,ymid;
    Font font;

    const char *fmt_str;
    const char *fmt_str0z;
    char string_buf[128];
    time_t t_start;
    time_t t_end;

    // Range of scale is 2.5 the movie loop length
    t_start = epoch_start - (epoch_end - epoch_start);
	if(t_start < 0) t_start = 0;  // Clamp to  The Unix epoch 0.
    t_end = epoch_end + (time_t) ((epoch_end - epoch_start) * .5);

    if (t_end == t_start)
      pixels_per_sec = 1.0;
    else
      pixels_per_sec = (double) width / (t_end - t_start);
    pps_bias = -(pixels_per_sec * t_start);

    // Pick the appropriate spacing for labels
    
    if(pixels_per_sec < 0.0000005) {
        text_period = 63072000; // 5 Years.
        tick_period = 12614400;  // 365 days
        fmt_str0z = "%y";
        fmt_str = fmt_str0z;
	} else if(pixels_per_sec < 0.000001) {
        text_period = 12614400; // 365.24 Days
        tick_period = 2592000;  // 30.5 days
        fmt_str0z = "%y";
        fmt_str = fmt_str0z;
     } else if(pixels_per_sec < 0.00001) {
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
        fmt_str =   (_params.use_local_timestamps)? " %H": " %HZ";
        fmt_str0z = " 0Z %m/%d";
    } else if(pixels_per_sec < 0.006) {
        text_period = 14400; // 4 hours
        tick_period = text_period / 4;
        fmt_str =   (_params.use_local_timestamps)? " %H": " %HZ";
        fmt_str0z = " %m/%d";
    } else if(pixels_per_sec < 0.015) {
        text_period = 7200; // 2 hours
        tick_period = text_period / 4;
        fmt_str =   (_params.use_local_timestamps)? " %H": " %HZ";
        fmt_str0z = " 0Z %m/%d";
    } else if(pixels_per_sec < 0.05) {
        text_period = 3600; // 1 hours
        tick_period = text_period / 4;
        fmt_str =   (_params.use_local_timestamps)? " %H": " %HZ";
        fmt_str0z =   (_params.use_local_timestamps)? " %H %m/%d": " %HZ %m/%d";
        fmt_str0z = " %HZ %m/%d";
    } else if(pixels_per_sec < 0.15) {
        text_period = 1800; //  30 min
        tick_period = text_period / 6;
        fmt_str =   (_params.use_local_timestamps)? " %H:%M": " %H:%MZ";
        fmt_str0z =   (_params.use_local_timestamps)? " %H %b %d": " %HZ %b %d";
    } else {
        text_period = 900; // 15 min 
        tick_period = text_period / 3;
        fmt_str =   (_params.use_local_timestamps)? " %H:%M": " %H:%MZ";
        fmt_str0z =   (_params.use_local_timestamps)? " %H %b %d": " %HZ %b %d";
    }

    //printf("Pixels_per_sec: %f  text_period: %d tick_period: %d\n",
    //	   pixels_per_sec,text_period,tick_period);

    y1 = height - row_height;
    //printf("num_rows: %d    height: %d\n",num_rows,height);

    XDrawLine(display,can_xid,axis_color.gc,0,y1,width,y1); // Draw the base 

    /* Plot a tick every x minutes */
    value = t_start - (t_start % tick_period);
    while ( value < t_end) {
        value += tick_period; 
        x1 =(time_t) (pixels_per_sec * value + pps_bias);
	if((value % text_period) == 0) {
               XDrawLine(display,can_xid,axis_color.gc,x1,y1,x1,y1-3); // Draw a tick

	    if((value % 86400) == 0) { /* Plot a 0 Z label */
               if(_params.use_local_timestamps) {
                   strftime(string_buf,128,fmt_str0z,localtime(&(value)));
               } else {
                   strftime(string_buf,128,fmt_str0z,gmtime(&(value)));
               }
               font = choose_font(string_buf, width,(height - y1 -4),&xmid,&ymid);
               XSetFont(display,axis_color.gc,font);
               XDrawImageString(display,can_xid,axis_color.gc, x1 + xmid , height -1 ,string_buf ,strlen(string_buf));
	} else {
               XDrawLine(display,can_xid,axis_color.gc,x1,y1,x1,y1-3); // Draw a tick

               if(_params.use_local_timestamps) {
                   strftime(string_buf,128,fmt_str,localtime(&(value)));
               } else {
                   strftime(string_buf,128,fmt_str,gmtime(&(value)));
               }
               font = choose_font(string_buf, width,(height - y1 -4),&xmid,&ymid);
               XSetFont(display,axis_color.gc,font);
               XDrawImageString(display,can_xid,axis_color.gc, x1 + xmid , height -1 ,string_buf ,strlen(string_buf));
            }
    } else {
        XDrawLine(display,can_xid,axis_color.gc,x1,y1,x1,y1+3); // Draw a tick
    }
    
  }

  // Draw a now indicator
  time_t now = time(0);
  if(now < t_end && now > t_start) {
      x1 =(time_t) (pixels_per_sec * now + pps_bias); 
      XDrawLine(display,can_xid,now_line.gc,x1,0,x1,height); 
      font = choose_font("NOW", width,(height - y1 - 4),&xmid,&ymid);
      XSetFont(display,now_line.gc,font);
      XDrawImageString(display,can_xid,now_line.gc, x1 + xmid , height ,"NOW" ,3);

  }
}

////////////////////////////////////////////////////////////////////////
// DRAW_EPOCH_INDICATOR: 
//

void TimePlot::draw_epoch_indicator()
{
    time_t x1,y1;
    time_t x2,y2;
    time_t x3,y3;
    int num;
    time_t value;
    XPoint pt[3];


    y1 = 3; 
    y2 = row_height;
    y3 = (y1 + y2) / 2; 

    x1 =  (time_t) ((epoch_start + left_offset + center_offset) * pixels_per_sec + pps_bias);
    x2 =  (time_t) ((epoch_end + right_offset + center_offset) * pixels_per_sec + pps_bias);

    // double dist = (x2 - x1)/ (double) nframes;

    XDrawLine(display,can_xid,epoch_ind.gc,x1,y1,x2,y1);
    XDrawLine(display,can_xid,epoch_ind.gc,x1,y2,x2,y2);

    // Draw Left Arrowhead
    pt[0].x = x1 - (y2 - y1);
    pt[0].y = y3;
    pt[1].x = x1;
    pt[1].y = y1;
    pt[2].x = x1;
    pt[2].y = y2;
    XFillPolygon(display,can_xid,epoch_ind.gc,pt,3,Convex, CoordModeOrigin);

    // Draw the frame dividers
    value = epoch_start + left_offset + center_offset;

    num = 0;
    while(value < epoch_end + right_offset + center_offset) {
	x3 = (time_t) (value * pixels_per_sec + pps_bias);
	XDrawLine(display,can_xid,epoch_ind.gc,x3,y1,x3,y2);
	value += frame_len;
	num++;
    }

    // Draw Right Arrowhead
    pt[0].x = x2 + (y2 - y1);
    pt[0].y = y3;
    pt[1].x = x2;
    pt[1].y = y1;
    pt[2].x = x2;
    pt[2].y = y2;
    XFillPolygon(display,can_xid,epoch_ind.gc,pt,3,Convex, CoordModeOrigin);

    // Fill in the current frame
    if(!drag_in_progress) {
        x1 =  (time_t) (frame_start * pixels_per_sec + pps_bias);
        x2 =  (time_t) (frame_end * pixels_per_sec + pps_bias);
	
        XFillRectangle(display,can_xid,frame_bar_color.gc,x1,y1,x2-x1+1,y2-y1+1); 
    }

}

////////////////////////////////////////////////////////////////////////
// DRAW: 
//

void TimePlot::Draw()
{

    int height_grow;
    int new_rows;

    if(epoch_end == epoch_start || nframes == 0 ) return;

    // Choose the bigger of the two lists
    if(grid_tlist.size() > prod_tlist.size() ) {
	new_rows = grid_tlist.size();
    } else {
	new_rows = prod_tlist.size();
    }

    if(new_rows != num_rows) {
        height_grow = (new_rows - num_rows) * row_height;

        win_height += height_grow;
        num_rows = new_rows;

	height += height_grow;

    }


    // Window dimensions are set every time
    // xv_set(gd.movie_pu->movie_pu,WIN_HEIGHT,win_height,WIN_WIDTH,width,NULL);

    // Clear the canvas
    XFillRectangle(display,can_xid,background_color.gc,0,0,width,height);

    // Draw the time axis
    this->draw_time_axis();

    // Draw the epoch indicator 
    this->draw_epoch_indicator();

    // Draw the Data time ticks 
    this->draw_time_lists();
}

