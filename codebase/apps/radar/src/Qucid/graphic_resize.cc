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
/*************************************************************************
 * GRAPHIC_RESIZE.C: 
 *
 * For the Cartesian Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define GRAPHIC_RESIZE   1

#include "cidd.h"

/*************************************************************************
 * H_WIN_EVENTS: Handle resizing events
 */

#ifdef NOTNOW
Notify_value h_win_events( Frame frame, Event *event, Notify_arg arg, Notify_event_type type)
{
    static  int in_progress;
    Window  root;     /* Root window ID of drawable */

    int value;
    int x,y;            /* location of drawable relative to parent */
    int    num_rows;        
    int    cp_height;        /* control panel height */
    int    cp_width;        /* control panel width */
    int    main_st_width;        /* menu bar width */
    int    main_st_height;        /* menu bar height */
    u_int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
 
    switch(event_action(event))
    {
    case  WIN_RESIZE:
        if (!in_progress) {

          // height = xv_get(frame,WIN_HEIGHT);
	  if(height > (u_int)gd.h_win.min_height) {
	    // xv_set(gd.h_win_horiz_bw->cp,XV_SHOW,TRUE,NULL);
            // cp_width = xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH);
            // main_st_width = xv_get(gd.h_win_horiz_bw->main_st,XV_WIDTH);
	    // num_rows = xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_NROWS);
	    while(main_st_width < cp_width/1.5 && num_rows > 1) {
	       num_rows--;
	       // xv_set(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_NROWS,num_rows,NULL); 
               // main_st_width = xv_get(gd.h_win_horiz_bw->main_st,XV_WIDTH);
	       // main_st_height = xv_get(gd.h_win_horiz_bw->main_st,XV_HEIGHT); 
	       // xv_set(gd.h_win_horiz_bw->cp,XV_HEIGHT,main_st_height + 30,NULL); 
	    }

	    while(main_st_width >= cp_width) {
	       num_rows++;
	       // xv_set(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_NROWS,num_rows,NULL); 
               // main_st_width = xv_get(gd.h_win_horiz_bw->main_st,XV_WIDTH);
	       // main_st_height = xv_get(gd.h_win_horiz_bw->main_st,XV_HEIGHT); 
	       // xv_set(gd.h_win_horiz_bw->cp,XV_HEIGHT,main_st_height + 30,NULL); 
	    }

            // cp_height = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT);
	    // xv_set(gd.h_win_horiz_bw->canvas1,XV_Y,cp_height,NULL);
	    // xv_set(gd.h_win_horiz_bw->movie_frame_msg,XV_Y,cp_height - 15,NULL);
	    // xv_set(gd.h_win_horiz_bw->cur_ht_msg,XV_Y,cp_height - 15,NULL);
	    // xv_set(gd.h_win_horiz_bw->cur_time_msg,XV_Y,cp_height - 15,NULL);
	    // xv_set(gd.h_win_horiz_bw->im_cache_st,XV_Y,cp_height - 25,NULL);
	  } else {
	    cp_height = 0;
	    // xv_set(gd.h_win_horiz_bw->canvas1,XV_Y,cp_height,NULL);
	    // xv_set(gd.h_win_horiz_bw->cp,XV_SHOW,FALSE,NULL);
	  }


            // width = xv_get(frame,WIN_WIDTH);
            if (height == (u_int)gd.h_win.win_dim.height && width == (u_int)gd.h_win.win_dim.width) {
                return(notify_next_event_func(frame,(Notify_event) event,arg,type));
            }
	     
            gd.h_win.win_dim.height = height;
            gd.h_win.win_dim.width = width;

            in_progress = 1;

            /* Set the base frame's width */
            width =  (u_int) (((height  - cp_height - gd.h_win.margin.top -
                gd.h_win.margin.bot) * gd.aspect_ratio)  +
                gd.h_win.margin.left + gd.h_win.margin.right);

            // xv_set(frame,
            //        WIN_WIDTH, width,
            //        WIN_HEIGHT, height,
            //        NULL);

            /* Set the canvas's width, height */
	    height = height - cp_height;
            // xv_set(gd.h_win_horiz_bw->canvas1,
            //        XV_HEIGHT, height,
            //        XV_WIDTH, width,
            //        NULL);

	    if(gd.num_cache_zooms > 1) {  // Have a visible Image Cache Widget 

		// Time message goes to the left of the cache widget
                // xv_set(gd.h_win_horiz_bw->cur_time_msg,XV_X,
	        //   (width - xv_get(gd.h_win_horiz_bw->cur_time_msg,XV_WIDTH)) -
	        //   (10 + xv_get(gd.h_win_horiz_bw->im_cache_st,XV_WIDTH)),
	        //     NULL);

	        // xv_set(gd.h_win_horiz_bw->im_cache_st, XV_X,
	        //   (width - xv_get(gd.h_win_horiz_bw->im_cache_st,XV_WIDTH) -2),
	        //   NULL);
	     } else {
                // xv_set(gd.h_win_horiz_bw->cur_time_msg,XV_X,
	        //   (width - xv_get(gd.h_win_horiz_bw->cur_time_msg,XV_WIDTH) - 10),
		//   NULL);
	     }
		    
	    // xv_set(gd.h_win_horiz_bw->cur_ht_msg, XV_X, (width / 2), NULL);

            // gd.hcan_xid = xv_get(canvas_paint_window(gd.h_win_horiz_bw->canvas1),XV_XID);
            gd.h_win.vis_xid = gd.hcan_xid;
            /* clear drawing area */
            XFillRectangle(gd.dpy, gd.hcan_xid, gd.legends.background_color->gc,
                           0, 0, width, height);

            XGetGeometry(gd.dpy,gd.h_win.vis_xid,&root,&x,&y,
                         &width,&height,&border_width,&depth);

            gd.h_win.can_dim.width = width;
            gd.h_win.can_dim.height = height;
            gd.h_win.can_dim.x_pos = x;
            gd.h_win.can_dim.y_pos = y;
            gd.h_win.can_dim.depth = depth;

            gd.h_win.img_dim.width = width -
                (gd.h_win.margin.left + gd.h_win.margin.right);
            gd.h_win.img_dim.height = height -
                (gd.h_win.margin.bot + gd.h_win.margin.top);
            gd.h_win.img_dim.x_pos = gd.h_win.margin.left;
            gd.h_win.img_dim.y_pos = gd.h_win.margin.top;
            gd.h_win.img_dim.depth = depth;

            if (gd.debug || gd.debug1) {
              cerr << "DEBUG - h_win_events:" << endl;
              cerr << " h_win.can_dim.width: " << gd.h_win.can_dim.width << endl;
              cerr << " h_win.can_dim.height: " << gd.h_win.can_dim.height << endl;
              cerr << " h_win.can_dim.x_pos: " << gd.h_win.can_dim.x_pos << endl;
              cerr << " h_win.can_dim.y_pos: " << gd.h_win.can_dim.y_pos << endl;
              cerr << " h_win.can_dim.depth: " << gd.h_win.can_dim.depth << endl;
              cerr << " h_win.img_dim.width: " << gd.h_win.img_dim.width << endl;
              cerr << " h_win.img_dim.height: " << gd.h_win.img_dim.height << endl;
              cerr << " h_win.img_dim.x_pos: " << gd.h_win.img_dim.x_pos << endl;
              cerr << " h_win.img_dim.y_pos: " << gd.h_win.img_dim.y_pos << endl;
              cerr << " h_win.img_dim.depth: " << gd.h_win.img_dim.depth << endl;
            }

            /* Free the old pixmaps and get new ones */
            manage_h_pixmaps(2);
            update_save_panel();
            
            set_redraw_flags(1,0);
            in_progress = 0;
        }
        break;

    case ACTION_OPEN:
        // gd.h_win.win_dim.closed = xv_get(frame,FRAME_CLOSED);
        break;    
 
    case ACTION_CLOSE :
        // gd.h_win.win_dim.closed = xv_get(frame,FRAME_CLOSED);

	if(gd.close_popups) {
	  close_all_popups();

	  // Set the main menu bar cells to the correct state
	  // All cells are off except for the winds_onoff or symprods_onoff state
          // value = xv_get(gd.h_win_horiz_bw->main_st,PANEL_VALUE,NULL);
          value &= (gd.menu_bar.winds_onoff_bit | gd.menu_bar.symprods_onoff_bit | gd.menu_bar.landuse_onoff_bit);
          // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,value,NULL);
	  //     gd.menu_bar.last_callback_value = value;
	}

	// Stop any movielooping
	gd.movie.movie_on = 0;
	// Make sure the Movie popup's start/stop widget  value is correct
	// xv_set(gd.movie_pu->start_st, PANEL_VALUE,0, NULL);

        break;    
 
    default:
        break;
    }
 
    return(notify_next_event_func(frame,(Notify_event) event,arg,type));
}
#endif
