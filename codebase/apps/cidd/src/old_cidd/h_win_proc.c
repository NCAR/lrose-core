/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*************************************************************************
 * H_WIN_PROC.C: Callbacks for the Horiz. view window 
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 *
 * Modification History:
 *   26 Aug 92   N. Rehak   Added code for zooming radial format data.
 */

#include <math.h>
#include <X11/Xlib.h>

#define H_WIN_PROC    1
#include "cidd.h"
#include <rapplot/xrs.h>

void set_zoom_proc();
 

/*************************************************************************
 * Notify callback function for `field_st'.
 */
void
field_proc(item, value, event)
    Panel_item    item;
    int        value;
    Event        *event;
{
    Window  xid;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
    int x_pos,y_pos;
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
    static int first_time = 1;

    xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);


    /* Windows position on xv_get is relative to the parent */
    /* so find out where the parent window is */
    XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
    XFree((caddr_t)children);
    XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
    /* take parents postion to get pos relative to root win */
    XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

    if(value) {
      if(first_time) {
	first_time = 0;
        /* Position the popup Far Left, below control panel */
        x_pos = 5;
        y_pos = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT) + 30;
        xv_set(gd.data_pu->data_pu,
               XV_X,    p_x + x_pos, 
               XV_Y,    p_y + y_pos, 
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
      } else {
        xv_set(gd.data_pu->data_pu,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
      }
    } else {
        xv_set(gd.data_pu->data_pu,
               FRAME_CMD_PUSHPIN_IN, FALSE,
               XV_SHOW, FALSE,
               NULL);
   }
}
 
/*************************************************************************
 * Notify callback function for `zoom_st'.
 */
void
zoom_proc(item, value, event)
        Panel_item      item;
        unsigned int    value;
        Event           *event;
{
    Window  xid;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
    int x_pos,y_pos;
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
    static int first_time = 1;

    xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);


    /* Windows position on xv_get is relative to the parent */
    /* so find out where the parent window is */
    XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
    XFree((caddr_t)children);
    XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
    /* take parents postion to get pos relative to root win */
    XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

    if(value) {
      if(first_time) {
	first_time = 0;
        /* Position the popup Just Right of the data_pu, below control panel */
        x_pos = xv_get(gd.data_pu->data_pu,XV_WIDTH) + 10;
        y_pos = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT) + 30;
        xv_set(gd.zoom_pu->zoom_pu,
               XV_X,    p_x + x_pos, 
               XV_Y,    p_y + y_pos,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
      } else {
        xv_set(gd.zoom_pu->zoom_pu,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
      }
    } else {
        xv_set(gd.zoom_pu->zoom_pu,
               FRAME_CMD_PUSHPIN_IN, FALSE,
               XV_SHOW, FALSE,
               NULL);
   }
}
 
/************************************************************************
 * Notify callback function for the elevation slider 
 */
void
sl_proc(item, value, event)
    Panel_item    item;
    int        value;
    Event        *event;
{
    int   slide_pos;
    char    label[32];
    met_record_t    *mr;
    static double last_ht =  0.0;

    mr = gd.mrec[gd.h_win.field];
    if(event == NULL) xv_set(gd.h_win_horiz_bw->slice_sl, PANEL_VALUE,value,NULL);

    if(value >= mr->sects) value = mr->sects -1;
    if(value >= 0 && value < mr->sects) {
	gd.h_win.cmin_ht = mr->vert[value].cent - ( gd.h_win.delta/2.0);
	gd.h_win.cmax_ht = mr->vert[value].cent + ( gd.h_win.delta/2.0);
    }else {
        gd.h_win.cmin_ht =  (value  * gd.h_win.delta) + gd.h_win.min_ht;
        gd.h_win.cmax_ht = gd.h_win.cmin_ht + gd.h_win.delta;
        if(gd.h_win.cmax_ht < (mr->vert[mr->sects-1].cent + ( gd.h_win.delta/2.0))) {
	    gd.h_win.cmin_ht = mr->vert[mr->sects-1].cent - ( gd.h_win.delta/2.0);
	    gd.h_win.cmax_ht = mr->vert[mr->sects-1].cent + ( gd.h_win.delta/2.0);
	}
    }

    sprintf(label,"%5.1f %s",((gd.h_win.cmin_ht + gd.h_win.cmax_ht) /2.0),
	    mr->units_label_sects);
        
    xv_set(gd.h_win_horiz_bw->cur_ht_msg, PANEL_LABEL_STRING, label, NULL);

    if(gd.h_win.cmax_ht ==  last_ht) return;  
    last_ht = gd.h_win.cmax_ht;
     
    switch(gd.movie.mode) {
    case MOVIE_MR: 
        set_redraw_flags(1,1);
        break;
        
    case MOVIE_TS: 
        set_redraw_flags(1,1);
        break;
    
    case MOVIE_EL: 
        gd.movie.cur_frame = value;
        if(gd.movie.cur_frame > gd.movie.end_frame)
            gd.movie.cur_frame = gd.movie.end_frame;
        if(gd.movie.cur_frame < gd.movie.start_frame)
            gd.movie.cur_frame = gd.movie.start_frame;
        break;
    }

    reset_data_valid_flags(1,0);
}

#define BUFFER_SIZE   1024


/*************************************************************************
 * User-defined action for `export_st'.
 */
void
export_ctl_proc(item, value, event)
        Panel_item      item;
        unsigned int    value;
        Event           *event;
{
    char buf[BUFFER_SIZE];

    if(value) {
        if(switch_the_window_expt(1) != 0) {
            if(strlen(gd.exprt_command) > 1) {
                rd_h_msg("Starting the Symbolic Product Generator - Please wait a few seconds\n",1);
                STRcopy(buf,gd.exprt_command,BUFFER_SIZE-2);
                strcat(buf," &");
                if(gd.debug) fprintf(stderr,"Running: %s\n",buf);
                safe_system(buf,30);
                xv_set(item,PANEL_VALUE,0,NULL);
	    } else {
                rd_h_msg("Sorry: Product Drawing feature is not availible\n",1);
                xv_set(item,PANEL_VALUE,0,NULL);
	    }
        }
    } else {
       switch_the_window_expt(0);
    }
}

/*************************************************************************
 * User-defined action for `product_st'.
 */
void
product_ctl_proc(item, value, event)
        Panel_item      item;
        unsigned int    value;
        Event           *event;
{
    char buf[1024];

    if (CSPR_products_on() && gd.limited_mode == 0)
    {
      CSPR_set_map_flag(value);
      
      return;
    }
    
    if(gd.limited_mode) {
	gd.prod.products_on = value;
        set_redraw_flags(1,0);
	return;
    }
     
    if(value) {
        if(switch_the_window_prds(1) != 0) {
            if(strlen(gd.prds_command) > 1) {
                rd_h_msg("Starting the Symbolic Product Selector - Please wait a few seconds\n",1);
                STRcopy(buf,gd.prds_command,BUFFER_SIZE-2);
                strcat(buf," &");
                if(gd.debug) fprintf(stderr,"Running: %s\n",buf);
                safe_system(buf,30);
                xv_set(item,PANEL_VALUE,0,NULL);
	    } else {
                rd_h_msg("Sorry: Product Warnings feature is not availible\n",1);
                xv_set(item,PANEL_VALUE,0,NULL);
	    }
        }
    } else {
        switch_the_window_prds(0);
    }
}


/*************************************************************************
/*
 * User-defined action for `overlay_st'.
 */
void
overlay_ctl_proc(item, value, event)
        Panel_item      item;
        unsigned int    value;
        Event           *event;
{
    Window  xid;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
    int x_pos,y_pos;
    int size,cp_size;
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */

    static int first_time = 1;

    xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

    /* Windows position on xv_get is relative to the parent */
    /* so find out where the parent window is */
    XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
    XFree((caddr_t)children);
    XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
    /* take parents postion to get pos relative to root win */
    XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

    if(gd.limited_mode) { /* Pop up / Close the overlay selector */
      if(value) {
        y_pos = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT) + 30;
        x_pos = xv_get(gd.data_pu->data_pu,XV_WIDTH) + 10;
        x_pos += xv_get(gd.zoom_pu->zoom_pu,XV_WIDTH) + 5;
        xv_set(gd.over_pu->over_pu,
               XV_X, p_x + x_pos,
               XV_Y, p_y + y_pos,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
      } else {
        xv_set(gd.over_pu->over_pu,
               FRAME_CMD_PUSHPIN_IN, FALSE,
               XV_SHOW, FALSE,
               NULL);
      }
    } else {              
      if(value) { /* open or close the Extras Popup */
	if(first_time) { /* Only posistion this panel one time */
	    first_time = 0;
            x_pos = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_WIDTH);
            cp_size = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT);
	    size = xv_get(gd.extras_pu->extras_pu,XV_WIDTH);
            xv_set(gd.extras_pu->extras_pu,
               XV_X, p_x + x_pos - size, /* Place agianst the Upper right edge */
               XV_Y, p_y + cp_size,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
	 } else { /* Just pop it up where ever the user last left it */
             xv_set(gd.extras_pu->extras_pu,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
	 }
      } else {
        xv_set(gd.extras_pu->extras_pu,
               FRAME_CMD_PUSHPIN_IN, FALSE,
               XV_SHOW, FALSE,
               NULL);
     }
   }
}


/*************************************************************************
 * User-defined action for `x_sect_st'.
 */
void
x_sect_ctl_proc(item, value, event)
        Panel_item      item;
        unsigned int    value;
        Event           *event;
{
    Window  xid;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
    static int first_time = 1;

    xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

    /* Windows position on xv_get is relative to the parent */
    /* so find out where the parent window is */
    XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
    XFree((caddr_t)children);
    XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
    /* take parents postion to get pos relative to root win */
    XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

    if(value) {
      if(first_time) {
	first_time = 0;
            xv_set(gd.v_win_v_win_pu->v_win_pu,
                   XV_X,    p_x, /* On lower left corner of parent win */
                   XV_Y,    p_y + gd.h_win.can_dim.height - gd.v_win.can_dim.height, /* */
                   NULL);
      }
            xv_set(gd.v_win_v_win_pu->v_win_pu,
                   XV_SHOW, TRUE,
                   NULL);
            gd.v_win.active = 1;
    } else {
            xv_set(gd.v_win_v_win_pu->v_win_pu,
                   XV_SHOW, FALSE,
                   NULL);
            gd.v_win.active = 0;
    }

}


/*************************************************************************
* User-defined action for `movie_st'.
 */
void
movie_ctl_proc(item, value, event)
        Panel_item      item;
        unsigned int    value;
        Event           *event;
{
    Window  xid;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
    int x_width,y_ht,cp_size;
    int size_x,size_y;
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */

    static int first_time = 1;

    xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

    /* Windows position on xv_get is relative to the parent */
    /* so find out where the parent window is */
    XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
    XFree((caddr_t)children);
    XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
    /* take parents postion to get pos relative to root win */
    XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

    if(value) {
	 if(first_time) { /* Position the popup on the lower right Once only */
	    first_time = 0;
            cp_size = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT);
            x_width = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_WIDTH);
            y_ht = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_HEIGHT);
            size_x = xv_get(gd.movie_pu->movie_pu,XV_WIDTH);
            size_y = xv_get(gd.movie_pu->movie_pu,XV_HEIGHT);
	     
             xv_set(gd.movie_pu->movie_pu,
               XV_X,    p_x + x_width - size_x, /* On lower right corner of parent win */
               XV_Y,    p_y + y_ht - size_y - cp_size, /* */
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW,TRUE,NULL);
	} else {  /* Show it where the user last left it */
             xv_set(gd.movie_pu->movie_pu,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW,TRUE,NULL);
	}
    } else {
        xv_set(gd.movie_pu->movie_pu,
               FRAME_CMD_PUSHPIN_IN, FALSE,
               XV_SHOW, FALSE,
               NULL);
    }

    start_timer();	/* Xview sometimes "forgets the interval timer" -restart it just in case */
}

/*************************************************************************
  * Notify callback function for `field_sel_st'.
 */
void
field_sel_proc(item, value, event)
        Panel_item      item;
        unsigned int    value;
        Event           *event;
{
    Window  xid;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
    int x_pos,y_pos;
    int size;
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
	void update_config_gui();
    static int first_time = 1;

    xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);


    /* Windows position on xv_get is relative to the parent */
    /* so find out where the parent window is */
    XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
    XFree((caddr_t)children);
    XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
    /* take parents postion to get pos relative to root win */
    XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

    if(value) {
	 if(first_time) { /* Position the popup Once only */
	    first_time = 0;
            x_pos = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_WIDTH);
            y_pos = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_HEIGHT);
            size = xv_get(gd.fields_pu->fields_pu,XV_HEIGHT);
	    xv_set(gd.fields_pu->fields_pu,
               XV_X,    p_x, /* On lower left corner of parent win */
               XV_Y,    p_y + y_pos - size, /* */
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW,TRUE,NULL);
	} else {
	    xv_set(gd.fields_pu->fields_pu,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW,TRUE,NULL);
	}
	update_config_gui();
    } else {
	 xv_set(gd.fields_pu->fields_pu,
               FRAME_CMD_PUSHPIN_IN, FALSE,
               XV_SHOW, FALSE,
               NULL);
    }
}

/*************************************************************************
 * User-defined action for `vector_st'.
 * This toggles on or off all wind vectors
 */
void
set_vectors_proc(item, value, event)
    Panel_item  item;
    int     value;
    Event       *event;
{
	static int last_state = -1;
    

	if(last_state < 0) last_state = gd.extras.wind_vectors;

	if(value > 0) {
		 gd.extras.wind_vectors = last_state;
    } else {
         last_state = gd.extras.wind_vectors;
		 gd.extras.wind_vectors = 0;
    }

     set_redraw_flags(1,1);
}
