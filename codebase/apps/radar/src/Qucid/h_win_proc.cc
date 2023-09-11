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
 * H_WIN_PROC.C: Callbacks for the Horiz. view window 
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define H_WIN_PROC    1

#include "cidd.h"

void show_xsect_panel( u_int value); // forward decleration

#ifdef NOTNOW
/*************************************************************************
 * Notify callback function for `im_cache_st'.
 */
void
im_cache_proc(Panel_item item, int value, Event *event)
{
    gd.h_win.cur_cache_im = value;
}
#endif

/*************************************************************************
 * SET_HEIGHT_LABEL: 
 */
void set_height_label()
{
#ifdef NOTNOW
   const char *label = NULL;
   label = height_label();
#endif
   // xv_set(gd.h_win_horiz_bw->cur_ht_msg, PANEL_LABEL_STRING, label, NULL);
}

/*************************************************************************
 * SHOW_CMD_MENU
 */
void show_cmd_menu(u_int value)
{
    Window  xid = 0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos = 0,y_pos = 0;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label = NULL;   // What this button is called 
#endif
    static int first_time = 1;

    if(value) {
      // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

        /* Windows position on xv_get is relative to the parent */
        /* so find out where the parent window is */
        XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
        XFree((caddr_t)children);
        XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
        /* take parents postion to get pos relative to root win */
        XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

      if(first_time) {
	first_time = 0;
	// calc the bit number of the widget 
#ifdef NOTNOW
	choice_num =  (int)(rint(log((double) gd.menu_bar.show_cmd_menu_bit) /
			    log(2.0)));
#endif
        

	// label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
        /* Position the popup Far Left, below control panel */
        // x_pos = xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) + 10;
        // y_pos = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT) + (4 * gd.h_win.margin.top);
        // xv_set(gd.cmd_pu->cmd_pu,
        //        XV_X,    p_x + x_pos, 
        //        XV_Y,    p_y + y_pos, 
        //        FRAME_CMD_PUSHPIN_IN, TRUE,
        //        XV_SHOW, TRUE,
	//        XV_LABEL, label,
        //        NULL);
      } else {
        // xv_set(gd.cmd_pu->cmd_pu,
        //        FRAME_CMD_PUSHPIN_IN, TRUE,
        //        XV_SHOW, TRUE,
        //        NULL);
      }
    } else {
        // xv_set(gd.cmd_pu->cmd_pu,
        //        FRAME_CMD_PUSHPIN_IN, FALSE,
        //        XV_SHOW, FALSE,
        //        NULL);
   }
}

/*************************************************************************
 * SHOW_DPD_MENU
 */
void show_dpd_menu(u_int value)
{
    Window  xid = 0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos = 0,y_pos = 0;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif
    static int first_time = 1;

    if(value) {
      // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

        /* Windows position on xv_get is relative to the parent */
        /* so find out where the parent window is */
        XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
        XFree((caddr_t)children);
        XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
        /* take parents postion to get pos relative to root win */
        XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

      if(first_time) {
	first_time = 0;
	// calc the bit number of the widget
#ifdef NOTNOW
	choice_num =  (int)(rint(log((double) gd.menu_bar.show_dpd_menu_bit) /
			    log(2.0)));
#endif

	// label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
        /* Position the popup Far Left, below control panel */
        // x_pos = xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) + 10;
#ifdef NOTNOW
        y_pos = gd.uparams->getLong("cidd.horiz_default_y_pos",0);
#endif
        // xv_set(gd.data_pu->data_pu,
        //        XV_X,    p_x + x_pos, 
        //        XV_Y,    p_y + y_pos, 
        //        FRAME_CMD_PUSHPIN_IN, TRUE,
        //        XV_SHOW, TRUE,
	//        XV_LABEL, label,
        //        NULL);
      } else {
        // xv_set(gd.data_pu->data_pu,
        //        FRAME_CMD_PUSHPIN_IN, TRUE,
        //        XV_SHOW, TRUE,
        //        NULL);
      }
    } else {
        // xv_set(gd.data_pu->data_pu,
        //        FRAME_CMD_PUSHPIN_IN, FALSE,
        //        XV_SHOW, FALSE,
        //        NULL);
   }
}
 
/*************************************************************************
 * SHOW_VIEW_MENU
 */
void show_view_menu(u_int value)
{
    Window  xid = 0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW   
    int x_pos=0,y_pos=0;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif
    static int first_time = 1;

    if(value) {
      // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

        /* Windows position on xv_get is relative to the parent */
        /* so find out where the parent window is */
        XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
        XFree((caddr_t)children);
        XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
        /* take parents postion to get pos relative to root win */
        XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

      if(first_time) {
	first_time = 0;
	// calc the bit number of the widget 
#ifdef NOTNOW
	choice_num =  (int)(rint(log((double) gd.menu_bar.show_view_menu_bit) /
			    log(2.0)));
#endif


	// label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
        /* Position the popup Just Right of the data_pu, below control panel */
        // x_pos = xv_get(gd.data_pu->data_pu,XV_WIDTH) + 10 + xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) + 10;
#ifdef NOTNOW
        y_pos = gd.uparams->getLong("cidd.horiz_default_y_pos",0);
#endif
        // xv_set(gd.zoom_pu->zoom_pu,
        //        XV_X,    p_x + x_pos, 
        //        XV_Y,    p_y + y_pos,
        //        FRAME_CMD_PUSHPIN_IN, TRUE,
        //        XV_SHOW, TRUE,
	//        XV_LABEL, label,
        //        NULL);
      } else {
        // xv_set(gd.zoom_pu->zoom_pu,
        //        FRAME_CMD_PUSHPIN_IN, TRUE,
        //        XV_SHOW, TRUE,
        //        NULL);
      }
    } else {
        // xv_set(gd.zoom_pu->zoom_pu,
        //        FRAME_CMD_PUSHPIN_IN, FALSE,
        //        XV_SHOW, FALSE,
        //        NULL);
   }
}
 


/*************************************************************************
 * SHOW_DPD_PANEL
 */
void show_dpd_panel( u_int value)
{
    Window  xid = 0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos=0;
    int size,cp_size;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif

    static int first_time = 1;

    if(value) { /* open or close the Page Config  Popup */

      // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

        /* Windows position on xv_get is relative to the parent */
        /* so find out where the parent window is */
        XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
        XFree((caddr_t)children);
        XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
        /* take parents postion to get pos relative to root win */
        XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

	if(first_time) { /* Only posistion this panel one time */
	    first_time = 0;
	    // calc the bit number of the widget 
#ifdef NOTNOW
	    choice_num =  (int)(rint(log((double) gd.menu_bar.show_dpd_panel_bit) /
			    log(2.0)));
#endif

	    // label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
            // x_pos = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_WIDTH) + 10;
            // cp_size = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT);
	    // size = xv_get(gd.page_pu->page_pu,XV_WIDTH);
            // xv_set(gd.page_pu->page_pu,
            //    XV_X, p_x + x_pos - size, /* Place agianst the Upper right edge */
            //    XV_Y, p_y + cp_size,
            //    FRAME_CMD_PUSHPIN_IN, TRUE,
            //    XV_SHOW, TRUE,
	    //    XV_LABEL, label,
            //    NULL);
	 } else { /* Just pop it up where ever the user last left it */
             // xv_set(gd.page_pu->page_pu,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
             //   NULL);
	 }
      } else {
        // xv_set(gd.page_pu->page_pu,
        //        FRAME_CMD_PUSHPIN_IN, FALSE,
        //        XV_SHOW, FALSE,
        //        NULL);
  }
}

/*************************************************************************
 * SET_ROUTE_MODE: Enable the Flight/ Cross Section mode - CIDD uses left
 *   mouse clicks to define a ployline for output
 */
void set_route_mode( u_int value)
{

   static route_track_t rt;

    if(value) {
	// keep a copy of the cross section state 
	memcpy(&rt,&gd.h_win.route,sizeof(route_track_t));

	gui_label_h_frame("Cross Section Mode:  Drag Mouse Button to Start- Click to extend", 1);
	add_message_to_status_win("Drag Mouse Button to Start- Click to extend",0);
	if( gd.drawing_mode == PICK_PROD_MODE) {
	    gd.r_context->set_draw_pick_boxes(false);
	    set_redraw_flags(1,0);
	}
	gd.drawing_mode = DRAW_ROUTE_MODE;
    } else {
	gd.drawing_mode = 0;
	gui_label_h_frame("Zoom Mode:  Drag Mouse to Zoom", 1);
	add_message_to_status_win("Drag Mouse to Zoom",0);

	// restore the copy of the cross section state 
	memcpy(&gd.h_win.route,&rt,sizeof(route_track_t));

    }

	// Make sure the Pick Button is up.
	if(gd.menu_bar.set_pick_mode_bit != 0) {
	  gd.menu_bar.last_callback_value &= ~gd.menu_bar.set_pick_mode_bit;
	  // xv_set(gd.h_win_horiz_bw->main_st,
	  //       	  PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);
	}
	
	// Make sure the Draw Button is up.
	if(gd.menu_bar.set_draw_mode_bit != 0) {
	  gd.menu_bar.last_callback_value &= ~gd.menu_bar.set_draw_mode_bit;
	  // xv_set(gd.h_win_horiz_bw->main_st,
	  //       	  PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);
	}
}

/*************************************************************************
 * SET_PICK_EXPORT_MODE: Enable the Pick /Export mode - CIDD uses left
 *   mouse clicks to select and drag productsd.
 */
void set_pick_export_mode( u_int value)
{
   static route_track_t rt;

    if(value) {
	// keep a copy of the cross section state 
	memcpy(&rt,&gd.h_win.route,sizeof(route_track_t));

	gui_label_h_frame("Pick Mode:  Click and Drag Objects" , 1);
	add_message_to_status_win("Click and Drag Objects",0);
	gd.drawing_mode = PICK_PROD_MODE;
	gd.r_context->set_draw_pick_boxes(true);
	set_redraw_flags(1,0);

    } else {
	gui_label_h_frame("Zoom Mode:  Drag Mouse to Zoom", 1);
	add_message_to_status_win("Drag Mouse to Zoom",0);
	gd.drawing_mode = 0;
	gd.r_context->set_draw_pick_boxes(false);
	set_redraw_flags(1,0);

	// restore the copy of the cross section state 
	memcpy(&gd.h_win.route,&rt,sizeof(route_track_t));

    }

	// Make sure the Route Button is up.
	if(gd.menu_bar.set_route_mode_bit != 0) {
	  gd.menu_bar.last_callback_value &= ~gd.menu_bar.set_route_mode_bit;
	  // xv_set(gd.h_win_horiz_bw->main_st,
	  //       	  PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);
	}
	
	// Make sure the Draw Button is up.
	if(gd.menu_bar.set_draw_mode_bit != 0) {
	  gd.menu_bar.last_callback_value &= ~gd.menu_bar.set_draw_mode_bit;
	  // xv_set(gd.h_win_horiz_bw->main_st,
	  //       	  PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);
	}
}

/*************************************************************************
 * SET_DRAW_EXPORT_MODE: Enable the Draw/Export mode - CIDD uses left
 *   mouse clicks to define a ployline for output
 */
void set_draw_export_mode( u_int value)
{

   static route_track_t rt;

    if(value) {
	// keep a copy of the cross section state 
	memcpy(&rt,&gd.h_win.route,sizeof(route_track_t));

	gui_label_h_frame("Draw Mode:  Drag Mouse Button to Start  - Click to extend", 1);
	add_message_to_status_win("Drag Mouse Button to Start  - Click to extend",0);
	if( gd.drawing_mode == PICK_PROD_MODE) {
	    gd.r_context->set_draw_pick_boxes(false);
	    set_redraw_flags(1,0);
	}
	gd.drawing_mode = DRAW_FMQ_MODE;

    } else {
	gui_label_h_frame("Zoom Mode:  Drag Mouse to Zoom", 1);
	add_message_to_status_win("Drag Mouse to Zoom",0);
	gd.drawing_mode = 0;

	// restore the copy of the cross section state 
	memcpy(&gd.h_win.route,&rt,sizeof(route_track_t));

    }

	// Make sure the Route Button is up.
	if(gd.menu_bar.set_route_mode_bit != 0) {
	  gd.menu_bar.last_callback_value &= ~gd.menu_bar.set_route_mode_bit;
	  // xv_set(gd.h_win_horiz_bw->main_st,
	  //       	  PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);
	}
	
	// Make sure the Pick Button is up.
	if(gd.menu_bar.set_pick_mode_bit != 0) {
	  gd.menu_bar.last_callback_value &= ~gd.menu_bar.set_pick_mode_bit;
	  // xv_set(gd.h_win_horiz_bw->main_st,
	  //       	  PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);
	}
}


/*************************************************************************
 * SHOW_DRAW_PANEL
 */
void show_draw_panel( u_int value)
{
    Window  xid=0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos=0;
    int size,cp_size;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif

    static int first_time = 1;

    if(value) { /* open or close the Draw/Export Confirmation popup */

      // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

        /* Windows position on xv_get is relative to the parent */
        /* so find out where the parent window is */
        XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
        XFree((caddr_t)children);
        XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
        /* take parents postion to get pos relative to root win */
        XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

	if(first_time) { /* Only posistion this panel one time */
	    first_time = 0;
	    // calc the bit number of the widget 
#ifdef NOTNOW
	    choice_num =  (int)(rint(log((double) gd.menu_bar.set_draw_mode_bit) /
			    log(2.0)));
#endif

	    // label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
            // x_pos = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_WIDTH) + 10;
            // cp_size = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT);
	    // size = xv_get(gd.draw_pu->draw_pu,XV_WIDTH);
            // xv_set(gd.draw_pu->draw_pu,
            //    XV_X, p_x + x_pos - size, /* Place agianst the Upper right edge */
            //    XV_Y, p_y + cp_size,
            //    FRAME_CMD_PUSHPIN_IN, TRUE,
            //    XV_SHOW, TRUE,
	    //    XV_LABEL, label,
            //    NULL);
	 } else { /* Just pop it up where ever the user last left it */
             // xv_set(gd.draw_pu->draw_pu,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
             //   NULL);
	 }

	 int tries = 10;
	 // Grab the data time and set the current draw product
	 while(gd.mrec[gd.h_win.page] == NULL && tries-- > 0 ) {
	   uusleep(10000); // Sleep 10 milisecs, waiting for the data
	 }
	 if( gd.mrec[gd.h_win.page] != NULL) {
	   gd.draw.dexport[gd.draw.cur_draw_product].data_time =
	     gd.mrec[gd.h_win.page]->h_date.unix_time;
	 } else {
	   fprintf(stderr,"Warning: Draw/ Export Module could not find a data time - Using Current time\n");
	   gd.draw.dexport[gd.draw.cur_draw_product].data_time = time(0);
	 }


         update_draw_export_panel();

      } else {
        // xv_set(gd.draw_pu->draw_pu,
        //        FRAME_CMD_PUSHPIN_IN, FALSE,
        //        XV_SHOW, FALSE,
        //        NULL);
  }
}

/*************************************************************************
 * SHOW_PRODUCT_MENU
 */
void show_product_menu( u_int value)
{
    Window  xid=0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x=0,y=0;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos=0,y_pos=0;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif

    static int first_time = 1;

      if(value) {
	if(first_time) { /* Only posistion this panel one time */
             first_time = 0;
	     // calc the bit number of the widget 
#ifdef NOTNOW
	     choice_num =  (int)(rint(log((double) gd.menu_bar.show_prod_menu_bit) /
			    log(2.0)));
#endif

	     // label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
             // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
             /* Windows position on xv_get is relative to the parent */
             /* so find out where the parent window is */
             XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
             XFree((caddr_t)children);
             XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
             /* take parents postion to get pos relative to root win */
             XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

             // y_pos = xv_get(gd.data_pu->data_pu,XV_HEIGHT) + 30;
             // x_pos = xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) + 10;
             // xv_set(gd.prod_pu->prod_pu,
             //   XV_X, p_x + x_pos,
             //   XV_Y, p_y + y_pos,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
	     //   XV_LABEL, label,
             //   NULL);
	} else  {
             // xv_set(gd.prod_pu->prod_pu,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
             //   NULL);
	}
      } else {
        // xv_set(gd.prod_pu->prod_pu,
        //        FRAME_CMD_PUSHPIN_IN, FALSE,
        //        XV_SHOW, FALSE,
        //        NULL);
      }
}


/*************************************************************************
 * SHOW_MAP_MENU
 */
void show_map_menu( u_int value)
{
    Window  xid = 0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos=0,y_pos=0;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif

    static int first_time = 1;

      if(value) {
	if(first_time) { /* Only posistion this panel one time */
             first_time = 0;
	     // calc the bit number of the widget 
#ifdef NOTNOW
	     choice_num =  (int)(rint(log((double) gd.menu_bar.show_map_menu_bit) /
			    log(2.0)));
#endif

	     // label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
             // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
             /* Windows position on xv_get is relative to the parent */
             /* so find out where the parent window is */
             XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
             XFree((caddr_t)children);
             XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
             /* take parents postion to get pos relative to root win */
             XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

             // y_pos = xv_get(gd.zoom_pu->zoom_pu,XV_HEIGHT) + 30;
             // x_pos = xv_get(gd.data_pu->data_pu,XV_WIDTH) + 10 + xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) + 10;
             // xv_set(gd.over_pu->over_pu,
             //   XV_X, p_x + x_pos,
             //   XV_Y, p_y + y_pos,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
	     //   XV_LABEL, label,
             //   NULL);
	} else  {
             // xv_set(gd.over_pu->over_pu,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
             //   NULL);
	}
      } else {
        // xv_set(gd.over_pu->over_pu,
        //        FRAME_CMD_PUSHPIN_IN, FALSE,
        //        XV_SHOW, FALSE,
        //        NULL);
      }
}

/*************************************************************************
 * SHOW_PAST_TIME_MENU
 */
void show_past_time_menu( u_int value)
{
  Window  xid=0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos=0,y_pos=0;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif

    static int first_time = 1;

      if(value) {
	if(first_time) { /* Only posistion this panel one time */
             first_time = 0;
	     // calc the bit number of the widget 
#ifdef NOTNOW
	     choice_num =  (int)(rint(log((double) gd.menu_bar.show_past_menu_bit) /
			    log(2.0)));
#endif

	     // label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
             // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
             /* Windows position on xv_get is relative to the parent */
             /* so find out where the parent window is */
             XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
             XFree((caddr_t)children);
             XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
             /* take parents postion to get pos relative to root win */
             XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

#ifdef NOTNOW
             y_pos = gd.uparams->getLong("cidd.horiz_default_y_pos",0);
#endif
             // x_pos = xv_get(gd.data_pu->data_pu,XV_WIDTH) + 10 + xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.zoom_pu->zoom_pu,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.over_pu->over_pu,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.prod_pu->prod_pu,XV_WIDTH) + 10;
             // xv_set(gd.past_pu->past_pu,
             //   XV_X, p_x + x_pos,
             //   XV_Y, p_y + y_pos,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
	     //   XV_LABEL, label,
             //   NULL);
	} else  {
             // xv_set(gd.past_pu->past_pu,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
             //   NULL);
	}
      } else {
         // xv_set(gd.past_pu->past_pu,
         //   FRAME_CMD_PUSHPIN_IN, FALSE,
         //   XV_SHOW, FALSE,
         //   NULL);
      }
}

/*************************************************************************
 * SHOW_FCAST_TIME_MENU
 */
void show_forecast_time_menu( u_int value)
{
    Window  xid=0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos=0,y_pos=0;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif

    static int first_time = 1;

      if(value) {
	if(first_time) { /* Only posistion this panel one time */
             first_time = 0;
	     // calc the bit number of the widget 
#ifdef NOTNOW
	     choice_num =  (int)(rint(log((double) gd.menu_bar.show_forecast_menu_bit) /
			    log(2.0)));
#endif

	     // label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
             // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
             /* Windows position on xv_get is relative to the parent */
             /* so find out where the parent window is */
             XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
             XFree((caddr_t)children);
             XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
             /* take parents postion to get pos relative to root win */
             XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

#ifdef NOTNOW
             y_pos = gd.uparams->getLong("cidd.horiz_default_y_pos",0);
#endif
             // x_pos = xv_get(gd.data_pu->data_pu,XV_WIDTH) + 10 + xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.zoom_pu->zoom_pu,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.over_pu->over_pu,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.prod_pu->prod_pu,XV_WIDTH) + 10;
             // xv_set(gd.fcast_pu->fcast_pu,
             //   XV_X, p_x + x_pos,
             //   XV_Y, p_y + y_pos,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
	     //   XV_LABEL, label,
             //   NULL);
	} else  {
             // xv_set(gd.fcast_pu->fcast_pu,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
             //   NULL);
	}
      } else {
         // xv_set(gd.fcast_pu->fcast_pu,
         //   FRAME_CMD_PUSHPIN_IN, FALSE,
         //   XV_SHOW, FALSE,
         //   NULL);
      }
}

/*************************************************************************
 * SHOW_BOOKMK_MENU
 */
void show_bookmk_menu( u_int value)
{
    Window  xid = 0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos,y_pos;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif

    static int first_time = 1;

      if(value) {
	if(first_time) { /* Only posistion this panel one time */
             first_time = 0;
	     // calc the bit number of the widget 
#ifdef NOTNOW
	     choice_num =  (int)(rint(log((double) gd.menu_bar.show_bookmark_menu_bit) /
			    log(2.0)));
#endif

	     // label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
             // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
             /* Windows position on xv_get is relative to the parent */
             /* so find out where the parent window is */
             XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
             XFree((caddr_t)children);
             XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
             /* take parents postion to get pos relative to root win */
             XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

#ifdef NOTNOW
             y_pos = gd.uparams->getLong("cidd.horiz_default_y_pos",0);
#endif
             // x_pos = xv_get(gd.data_pu->data_pu,XV_WIDTH) + 10 + xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.zoom_pu->zoom_pu,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.over_pu->over_pu,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.prod_pu->prod_pu,XV_WIDTH) + 10;
             // x_pos += xv_get(gd.fcast_pu->fcast_pu,XV_WIDTH) + 10;
             // xv_set(gd.bookmk_pu->bookmk_pu,
             //   XV_X, p_x + x_pos,
             //   XV_Y, p_y + y_pos,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
	     //   XV_LABEL, label,
             //   NULL);
	} else  {
             // xv_set(gd.bookmk_pu->bookmk_pu,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW, TRUE,
             //   NULL);
	}
      } else {
        // xv_set(gd.bookmk_pu->bookmk_pu,
        //        FRAME_CMD_PUSHPIN_IN, FALSE,
        //        XV_SHOW, FALSE,
        //        NULL);
      }
}

/*************************************************************************
 * SHOW_XSECT_PANEL
 */
void show_xsect_panel( u_int value)
{
    Window  xid = 0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
#ifdef NOTNOW
    unsigned int display_width;
    unsigned int display_height;
#endif
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos,y_pos;   /* Where to position window */
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif
    static int first_time = 1;

    if(value) {
      // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

        /* Windows position on xv_get is relative to the parent */
        /* so find out where the parent window is */
        XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
        XFree((caddr_t)children);
        XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
        /* take parents postion to get pos relative to root win */
        XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);
#ifdef NOTNOW
	display_width = DisplayWidth(gd.dpy,0);
	display_height = DisplayHeight(gd.dpy,0);
#endif

        //XV_X,    p_x, /* On lower left corner of parent win */
        //XV_Y,    p_y + gd.h_win.can_dim.height - gd.v_win.can_dim.height, /* */
#ifdef NOTNOW
	x_pos = gd.uparams->getLong( "cidd.vert_default_x_pos", display_width  - gd.v_win.win_dim.width - 10);
	y_pos = gd.uparams->getLong( "cidd.vert_default_y_pos", display_height - gd.v_win.win_dim.height - 30);
#endif

      if(first_time) {
	first_time = 0;
	// calc the bit number of the widget 
#ifdef NOTNOW
	choice_num =  (int)(rint(log((double) gd.menu_bar.show_xsect_panel_bit) /
			    log(2.0)));
#endif

	// label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
        //     xv_set(gd.v_win_v_win_pu->v_win_pu,
        //            XV_X,    x_pos, 
        //            XV_Y,    y_pos,
	//            XV_LABEL, label,
        //            NULL);
      }
            // xv_set(gd.v_win_v_win_pu->v_win_pu,
            //        XV_SHOW, TRUE,
            //        NULL);
            gd.v_win.active = 1;
    } else {
            // xv_set(gd.v_win_v_win_pu->v_win_pu,
            //        XV_SHOW, FALSE,
            //        NULL);
            gd.v_win.active = 0;
    }
    set_redraw_flags(1,1);
}

/*************************************************************************
 * SHOW_TIME_PANEL
 */
void
show_time_panel(u_int value)
{
    Window  xid = 0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_width,y_ht,cp_size;
    int size_x,size_y;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif

    static int first_time = 1;

    if(value) {
      // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

       /* Windows position on xv_get is relative to the parent */
       /* so find out where the parent window is */
       XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
       XFree((caddr_t)children);
       XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
       /* take parents postion to get pos relative to root win */
       XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

	 if(first_time) { /* Position the popup on the lower right Once only */
	    first_time = 0;
	    // calc the bit number of the widget 
#ifdef NOTNOW
	    choice_num = (int)(rint(log((double) gd.menu_bar.show_time_panel_bit) / log(2.0)));
#endif

	    // label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
            // cp_size = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT);
            // x_width = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_WIDTH);
            // y_ht = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_HEIGHT);
            // size_x = xv_get(gd.movie_pu->movie_pu,XV_WIDTH);
            // size_y = xv_get(gd.movie_pu->movie_pu,XV_HEIGHT);
	     
            //  xv_set(gd.movie_pu->movie_pu,
            //    XV_X,   0, /* lower left corner screen */
            //    XV_Y,   DisplayHeight(gd.dpy,0) - size_y - 30,
            //    FRAME_CMD_PUSHPIN_IN, TRUE,
	    //    XV_LABEL, label,
            //    XV_SHOW,TRUE,NULL);
	     gd.movie.active = 1;
	     if(gd.time_plot) gd.time_plot->Draw();
	} else {  /* Show it where the user last left it */
             // xv_set(gd.movie_pu->movie_pu,
             //   FRAME_CMD_PUSHPIN_IN, TRUE,
             //   XV_SHOW,TRUE,NULL);
	     gd.movie.active = 1;
	     if(gd.time_plot) gd.time_plot->Draw();
	}
    } else {
        // xv_set(gd.movie_pu->movie_pu,
        //        FRAME_CMD_PUSHPIN_IN, FALSE,
        //        XV_SHOW, FALSE,
        //        NULL);
	     gd.movie.active = 0;
    }

    start_timer();	/* Xview sometimes "forgets the interval timer" -restart it just in case */
}

/*************************************************************************
  * Show the Status panel
 */
void show_status_panel( u_int value)
{
    // int x_pos = xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) + 10;
#ifdef NOTNOW
    int y_pos = gd.uparams->getLong("cidd.horiz_default_y_pos",0);
#endif
    // xv_set(gd.status_pu->status_pu,
    //        XV_X, x_pos, 
    //        XV_Y, y_pos,
    //        NULL);
    if(value) {
	// xv_set(gd.status_pu->status_pu,
	// 	FRAME_CMD_PUSHPIN_IN, TRUE,
	// 	XV_SHOW, TRUE,
	// 	NULL);

    } else {
	 // xv_set(gd.status_pu->status_pu,
         //       FRAME_CMD_PUSHPIN_IN, FALSE,
         //       XV_SHOW, FALSE,
         //       NULL);
    }
}

/*************************************************************************
  * Show the Grid parameter editor panel
 */
void show_grid_panel( u_int value)
{
    Window  xid = 0;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
#ifdef NOTNOW
    int x_pos,y_pos;
    int size;
#endif
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
#ifdef NOTNOW
    int  choice_num;
    char *label;   // What this button is called
#endif
    void update_grid_config_gui();
    void update_wind_config_gui();
    void update_prod_config_gui();
    static int first_time = 1;

    if(value) {
      // xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
        
        /* Windows position on xv_get is relative to the parent */
        /* so find out where the parent window is */
        XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
        XFree((caddr_t)children);
        XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
        /* take parents postion to get pos relative to root win */
        XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

	 if(first_time) { /* Position the popup Once only */
	    first_time = 0;
	    // calc the bit number of the widget 
#ifdef NOTNOW
	    choice_num =  (int)(rint(log((double) gd.menu_bar.show_grid_panel_bit) /
			    log(2.0)));
#endif

	    // label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
            // x_pos = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_WIDTH) + 10;
            // y_pos = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_HEIGHT);
            // size = xv_get(gd.fields_pu->fields_pu,XV_HEIGHT);
	    // xv_set(gd.fields_pu->fields_pu,
            //    XV_X,    p_x, /* On lower left corner of parent win */
            //    XV_Y,    p_y + y_pos - size, /* */
            //    FRAME_CMD_PUSHPIN_IN, TRUE,
	    //    XV_LABEL, label,
            //    XV_SHOW,TRUE,NULL);
	} else {
	    // xv_set(gd.fields_pu->fields_pu,
            //    FRAME_CMD_PUSHPIN_IN, TRUE,
            //    XV_SHOW,TRUE,NULL);
	}
	update_grid_config_gui();
	update_wind_config_gui();
	update_prod_config_gui();
    } else {
	 // xv_set(gd.fields_pu->fields_pu,
         //       FRAME_CMD_PUSHPIN_IN, FALSE,
         //       XV_SHOW, FALSE,
         //       NULL);
    }
}

/*************************************************************************
 * This toggles on or off the automatic report mode 
 */
void report_mode_onoff( u_int value)
{
    if(value > 0) {
        gd.report_mode  = 1;
	if(gd.movie.cur_frame != gd.movie.last_frame) {
	    gather_hwin_data(gd.h_win.page,
		      gd.movie.frame[gd.movie.cur_frame].time_start,
		      gd.movie.frame[gd.movie.cur_frame].time_end);
	    gd.movie.last_frame = gd.movie.cur_frame;
	    gui_label_h_frame("Gathering correct data",-1);
        }
    } else {
         gd.report_mode = 0;
    }
}

/*************************************************************************
 * This toggles on or off the land use base map
 */
void landuse_onoff( u_int value)
{
    if(value > 0) {
         gd.layers.earth.landuse_active = 1;
         // xv_set(gd.page_pu->land_use_st,PANEL_VALUE,1,NULL);
    } else {
         gd.layers.earth.landuse_active = 0;
         // xv_set(gd.page_pu->land_use_st,PANEL_VALUE,0,NULL);
    }

    set_redraw_flags(1,0);
}

/*************************************************************************
 * This toggles on or off all wind vectors
 */
void winds_onoff( u_int value)
{
    if(value > 0) {
         gd.layers.wind_vectors = 1;
    } else {
         gd.layers.wind_vectors = 0;
    }

     set_redraw_flags(1,1);
}

/*************************************************************************
 * This toggles on or off the symbolic products
 */
void symprods_onoff( u_int value)
{
    if(value > 0) {
        gd.prod.products_on = 1;
    } else {
        gd.prod.products_on = 0;
    }

    set_redraw_flags(1,0);
}

/*************************************************************************
 * This Initiates printing  
 */
void startup_printing( u_int value)
{
    u_int p_value;

    gd.save_im_win = 0;

    if(gd.v_win.active) { 
       gd.save_im_win |= XSECT_VIEW; // Route window
    } 
    if(gd.h_win.active) { 
       gd.save_im_win |= PLAN_VIEW; // Main window
    }
    dump_cidd_image(gd.save_im_win,0,1,gd.h_win.page);

    p_value = gd.menu_bar.last_callback_value & ~(gd.menu_bar.print_button_bit);

    gd.menu_bar.last_callback_value = p_value;
    // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,p_value,NULL);
}

#define SNAPSHOT_CMD "Cidd_snap"
/*************************************************************************
 * This Initiates A Snapshot  
 */
void startup_snapshot( u_int value)
{
    char cmd[1024];
    u_int p_value;

    sprintf(cmd,"%s %ld %d %d > /dev/null 2>&1 &\n",SNAPSHOT_CMD,
     gd.h_win.can_xid[gd.h_win.cur_cache_im],
     gd.h_win.can_dim.width,gd.h_win.can_dim.height);

    fprintf(stderr,"Executing: %s",cmd);
    system(cmd);

    p_value = gd.menu_bar.last_callback_value & ~(gd.menu_bar.snapshot_bit);
    gd.menu_bar.last_callback_value = p_value;
    // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,p_value,NULL);
}

#define BUFFER_SIZE   1024
/*************************************************************************
 * This Initiates a Help system  
 */
void startup_help( u_int value)
{
    u_int p_value;
    char buf[BUFFER_SIZE];

    if(value) {
       if(strlen(gd.help_command) > 1) {
          gui_label_h_frame("Sending your Browser to the User Manual",1);
                STRcopy(buf,gd.help_command,BUFFER_SIZE-2);
                // strcat(buf," &");
                if(gd.debug) fprintf(stderr,"Running: %s\n",buf);
		          safe_system(buf,gd.simple_command_timeout_secs);
                //system(buf);
       }
    }

    p_value = gd.menu_bar.last_callback_value & ~(gd.menu_bar.help_button_bit);

    gd.menu_bar.last_callback_value = p_value;
    // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,p_value,NULL);
}

#ifdef NOTNOW

/*************************************************************************
 * Notify callback function for `main_st'.
 */
void main_st_proc(Panel_item item, u_int value, Event *event)
{
    int bit;

    gd.last_event_time = time((time_t *) NULL);

    // Use XOR to find the cell/bit that changed
    bit = gd.menu_bar.last_callback_value ^ value;
    gd.menu_bar.last_callback_value = value;

    if(bit == gd.menu_bar.loop_onoff_bit) {
	movie_start(value & gd.menu_bar.loop_onoff_bit);

    } else if(bit ==  gd.menu_bar.winds_onoff_bit) {
	winds_onoff(value & gd.menu_bar.winds_onoff_bit);

    } else if(bit ==  gd.menu_bar.report_mode_bit) {
	report_mode_onoff(value & gd.menu_bar.report_mode_bit);

    } else if(bit ==  gd.menu_bar.landuse_onoff_bit) {
	landuse_onoff(value & gd.menu_bar.landuse_onoff_bit);

    } else if(bit ==  gd.menu_bar.show_forecast_menu_bit) {
	show_forecast_time_menu(value & gd.menu_bar.show_forecast_menu_bit);

    } else if(bit ==  gd.menu_bar.show_past_menu_bit) {
	show_past_time_menu(value & gd.menu_bar.show_past_menu_bit);

    } else if(bit ==  gd.menu_bar.show_gen_time_win_bit) {
	show_gen_time_menu(value & gd.menu_bar.show_gen_time_win_bit);

    } else if(bit ==  gd.menu_bar.symprods_onoff_bit) {
	symprods_onoff(value & gd.menu_bar.symprods_onoff_bit);

    } else if(bit ==  gd.menu_bar.help_button_bit) {
	startup_help(value & gd.menu_bar.help_button_bit);

    } else if(bit ==  gd.menu_bar.exit_button_bit) {
	if(gd.finished_init) {
	    kill(getpid(),SIGTERM);
	} else {
	    exit(0);
	}
	 

    } else if(bit ==  gd.menu_bar.snapshot_bit) {
	startup_snapshot(value & gd.menu_bar.snapshot_bit);

    } else if(bit ==  gd.menu_bar.print_button_bit) {
	startup_printing(value & gd.menu_bar.print_button_bit);

    } else if(bit ==  gd.menu_bar.show_view_menu_bit) {
	show_view_menu(value & gd.menu_bar.show_view_menu_bit);

    } else if(bit ==  gd.menu_bar.show_time_panel_bit) {
	show_time_panel(value & gd.menu_bar.show_time_panel_bit);

    } else if(bit ==  gd.menu_bar.show_dpd_menu_bit) {
	show_dpd_menu(value & gd.menu_bar.show_dpd_menu_bit);

    } else if(bit ==  gd.menu_bar.show_cmd_menu_bit) {
	show_cmd_menu(value & gd.menu_bar.show_cmd_menu_bit);

    } else if(bit ==  gd.menu_bar.show_dpd_panel_bit) {
	show_dpd_panel(value & gd.menu_bar.show_dpd_panel_bit);

    } else if(bit ==  gd.menu_bar.set_route_mode_bit) {
	set_route_mode(value & gd.menu_bar.set_route_mode_bit);

    } else if(bit ==  gd.menu_bar.set_draw_mode_bit) {
	set_draw_export_mode(value & gd.menu_bar.set_draw_mode_bit);

    } else if(bit ==  gd.menu_bar.set_pick_mode_bit) {
	set_pick_export_mode(value & gd.menu_bar.set_pick_mode_bit);

    } else if(bit ==  gd.menu_bar.show_xsect_panel_bit) {
	show_xsect_panel(value & gd.menu_bar.show_xsect_panel_bit);

    } else if(bit ==  gd.menu_bar.show_map_menu_bit) {
	show_map_menu(value & gd.menu_bar.show_map_menu_bit);

    } else if(bit ==  gd.menu_bar.show_prod_menu_bit) {
	show_product_menu(value & gd.menu_bar.show_prod_menu_bit);

    } else if(bit ==  gd.menu_bar.show_bookmark_menu_bit) {
	show_bookmk_menu(value & gd.menu_bar.show_bookmark_menu_bit);
    } else if(bit ==  gd.menu_bar.print_button_bit) {
	startup_printing(value & gd.menu_bar.print_button_bit);

    } else if(bit ==  gd.menu_bar.show_view_menu_bit) {
	show_view_menu(value & gd.menu_bar.show_view_menu_bit);

    } else if(bit ==  gd.menu_bar.show_time_panel_bit) {
	show_time_panel(value & gd.menu_bar.show_time_panel_bit);

    } else if(bit ==  gd.menu_bar.show_dpd_menu_bit) {
	show_dpd_menu(value & gd.menu_bar.show_dpd_menu_bit);

    } else if(bit ==  gd.menu_bar.show_cmd_menu_bit) {
	show_cmd_menu(value & gd.menu_bar.show_cmd_menu_bit);

    } else if(bit ==  gd.menu_bar.show_dpd_panel_bit) {
	show_dpd_panel(value & gd.menu_bar.show_dpd_panel_bit);

    } else if(bit ==  gd.menu_bar.show_xsect_panel_bit) {
	show_xsect_panel(value & gd.menu_bar.show_xsect_panel_bit);

    } else if(bit ==  gd.menu_bar.show_map_menu_bit) {
	show_map_menu(value & gd.menu_bar.show_map_menu_bit);

    } else if(bit ==  gd.menu_bar.show_prod_menu_bit) {
	show_product_menu(value & gd.menu_bar.show_prod_menu_bit);

    } else if(bit ==  gd.menu_bar.show_bookmark_menu_bit) {
	show_bookmk_menu(value & gd.menu_bar.show_bookmark_menu_bit);

    } else if(bit ==  gd.menu_bar.show_grid_panel_bit) {
	show_grid_panel(value & gd.menu_bar.show_grid_panel_bit);

    } else if(bit ==  gd.menu_bar.show_status_win_bit) {
	show_status_panel(value & gd.menu_bar.show_status_win_bit);

    } else if(bit ==  gd.menu_bar.close_popups_bit) {
	close_all_popups();

    } else if(bit ==  gd.menu_bar.zoom_back_bit) {
        zoom_back();

    } else if(bit ==  gd.menu_bar.clone_button_bit) {
	 char command_buf[MAX_PATH_LEN];
	 *command_buf = '\0';

	 // Rebuild the command line
	 for(int i= 0; i < gd.argc; i++) {
	     strncat(command_buf,gd.argv[i],MAX_PATH_LEN);
	     strncat(command_buf," ",MAX_PATH_LEN);
	 }
	 strncat(command_buf," &",MAX_PATH_LEN);

	 // Go back to the original starting directory
	 chdir(gd.orig_wd);

	 // start another instance of the display
	 system(command_buf);

	 // Pop back up the button
	 gd.menu_bar.last_callback_value &= ~gd.menu_bar.clone_button_bit;
	 xv_set(item,PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);

    } else if(bit ==  gd.menu_bar.set_to_now_bit) {
	 // Pop back up the button
	 gd.menu_bar.last_callback_value &= ~gd.menu_bar.set_to_now_bit;
	 xv_set(item,PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);

	 // Set the Forecast & past choice setting to Now
	 xv_set(gd.fcast_pu->fcast_st,PANEL_VALUE,0,NULL);
	 xv_set(gd.past_pu->past_hr_st,PANEL_VALUE,0,NULL);

	 // Make sure to hide the menus
	 xv_set(gd.fcast_pu->fcast_pu,FRAME_CMD_PUSHPIN_IN, FALSE, XV_SHOW, FALSE, NULL);
	 xv_set(gd.past_pu->past_pu,FRAME_CMD_PUSHPIN_IN, FALSE, XV_SHOW, FALSE, NULL);


	 gd.movie.cur_frame = gd.movie.end_frame;
	 time_t    clock = time(0);

	 // Reset time to now.
	 gd.movie.start_time =  clock - (time_t) ((gd.movie.num_frames -1) *
			       gd.movie.time_interval * 60.0);

	 // Round time
	 gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);

	 set_display_time(gd.movie.start_time);

     //  restore movie frame interval back to original value if in magnify mode.
     if(gd.movie.magnify_mode != 0 ) {
	   gd.movie.time_interval /= gd.movie.magnify_factor;
	   gd.movie.magnify_mode = 0;
     }

     gd.movie.mode = REALTIME_MODE;

	 // Make sure the movie interval reflects Realtime
         reset_time_points();
         update_movie_popup();
         update_frame_time_msg(gd.movie.cur_frame);
         reset_data_valid_flags(1,1);
         gd.prod_mgr->reset_product_valid_flags();

      set_redraw_flags(1,1);

    } else if(bit ==  gd.menu_bar.reset_bit) {
	 reset_display();

	 // Pop back up the button
	 gd.menu_bar.last_callback_value &= ~gd.menu_bar.reset_bit;
	 xv_set(item,PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);

    } else if(bit ==  gd.menu_bar.reload_bit) {
	 invalidate_all_data();
	 // Indicate a need to redraw
         set_redraw_flags(1,1);

	 // Pop back up the button
	 gd.menu_bar.last_callback_value &= ~gd.menu_bar.reload_bit;
	 xv_set(item,PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);
    }
}
#endif
