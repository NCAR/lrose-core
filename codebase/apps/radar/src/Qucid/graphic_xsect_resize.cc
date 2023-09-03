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
 * GRAPHIC_XSECT_RESIZE: 
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define GRAPHIC_XSECT_RESIZE 1

#include "cidd.h"

#include <rapplot/xrs.h>

extern void update_save_panel();

/*************************************************************************
 * V_WIN_EVENTS: Handle resizing events in the verticle display
 */

Notify_value
v_win_events(Frame frame, Event *event, Notify_arg arg,
             Notify_event_type type)
{
    static  int in_progress;
    Window  root;    /* Root window ID of drawable */
    int x,y;            /* location of drawable relative to parent */
    u_int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
    int xpos;

    switch(event_action(event)) {
        case  WIN_RESIZE: if(!in_progress) {
            // height = xv_get(frame,WIN_HEIGHT);
            // width = xv_get(frame,WIN_WIDTH);
            if(height == (u_int)gd.v_win.win_dim.height && width == (u_int)gd.v_win.win_dim.width) {
                return(notify_next_event_func(frame,(Notify_event) event,arg,type));
            }
            gd.v_win.win_dim.height = height;
            gd.v_win.win_dim.width = width;
            in_progress = 1;
            if(height < (u_int)gd.v_win.min_height) height = gd.v_win.min_height;
            if(width < (u_int)gd.v_win.min_width) width = gd.v_win.min_width;

            // xv_set(frame,WIN_WIDTH,width,WIN_HEIGHT,height,NULL);

            /* Set the canvas's width */
            // xv_set(gd.v_win_v_win_pu->canvas1,XV_HEIGHT,height,XV_WIDTH,width,NULL);
            // gd.vcan_xid = xv_get(canvas_paint_window(gd.v_win_v_win_pu->canvas1),XV_XID);
            gd.v_win.vis_xid = gd.vcan_xid;
            /* Clear drawing area */
            XFillRectangle(gd.dpy,gd.vcan_xid,gd.legends.background_color->gc,0,0,width,height);

            XGetGeometry(gd.dpy,gd.v_win.vis_xid,&root,&x,&y,
                &width,&height,&border_width,&depth);
 
            gd.v_win.can_dim.width = width;
            // gd.v_win.can_dim.height = height - xv_get(gd.v_win_v_win_pu->controls1,XV_HEIGHT)
              ;
            gd.v_win.can_dim.x_pos = x;
            gd.v_win.can_dim.y_pos = y;
            gd.v_win.can_dim.depth = depth;
 
            gd.v_win.img_dim.width = width - (gd.v_win.margin.left + gd.v_win.margin.right); 
            gd.v_win.img_dim.height =  gd.v_win.can_dim.height - (gd.v_win.margin.bot + gd.v_win.margin.top);  
            gd.v_win.img_dim.x_pos = gd.v_win.margin.left;
            gd.v_win.img_dim.y_pos = gd.v_win.margin.top;
            gd.v_win.img_dim.depth = depth;
	    
            if (gd.debug || gd.debug1) {
              cerr << "DEBUG - v_win_events:" << endl;
              cerr << " v_win.can_dim.width: " << gd.v_win.can_dim.width << endl;
              cerr << " v_win.can_dim.height: " << gd.v_win.can_dim.height << endl;
              cerr << " v_win.can_dim.x_pos: " << gd.v_win.can_dim.x_pos << endl;
              cerr << " v_win.can_dim.y_pos: " << gd.v_win.can_dim.y_pos << endl;
              cerr << " v_win.can_dim.depth: " << gd.v_win.can_dim.depth << endl;
              cerr << " v_win.img_dim.width: " << gd.v_win.img_dim.width << endl;
              cerr << " v_win.img_dim.height: " << gd.v_win.img_dim.height << endl;
              cerr << " v_win.img_dim.x_pos: " << gd.v_win.img_dim.x_pos << endl;
              cerr << " v_win.img_dim.y_pos: " << gd.v_win.img_dim.y_pos << endl;
              cerr << " v_win.img_dim.depth: " << gd.v_win.img_dim.depth << endl;
            }

	    // Keep the dismiss button and others tied to the right edge.
	    // xpos = width - 10 -  xv_get(gd.v_win_v_win_pu->dismiss_bt,XV_WIDTH);
	    // xv_set(gd.v_win_v_win_pu->dismiss_bt,XV_X,xpos,NULL);
	    // xpos -= xv_get(gd.v_win_v_win_pu->v_unzoom_bt,XV_WIDTH) + 10;
	    // xv_set(gd.v_win_v_win_pu->v_unzoom_bt,XV_X,xpos,NULL);
	    
	    // xpos -= xv_get(gd.v_win_v_win_pu->scale_base_tx,XV_WIDTH) + 10;
	    // xv_set(gd.v_win_v_win_pu->ht_top_tx,XV_X,xpos,NULL);
	    // xv_set(gd.v_win_v_win_pu->scale_base_tx,XV_X,xpos,NULL);
	    
            /* Free the old pixmaps and get new ones */
            manage_v_pixmaps(2);
 
            update_save_panel();
			 
            set_redraw_flags(0,1);
 
            in_progress = 0;
        }
        break;
         
        case ACTION_OPEN:
                // gd.v_win.win_dim.closed = xv_get(frame,FRAME_CLOSED);
        break;

        case ACTION_CLOSE :
		// v_panel_dismiss(gd.v_win_v_win_pu->dismiss_bt,NULL);
                // gd.v_win.win_dim.closed = xv_get(frame,FRAME_CLOSED);
        break;

        default:
        break;
    }

    return(notify_next_event_func(frame,(Notify_event) event,arg,type));
}   
