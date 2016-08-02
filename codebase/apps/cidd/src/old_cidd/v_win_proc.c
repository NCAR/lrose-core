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
 * V_WIN_PROC: Callback procedures for the Veritcal display window
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#include <X11/Xlib.h>

#define V_WIN_PROC 1
#include "cidd.h"

#include <rapplot/xrs.h>

extern void update_save_panel();

static int  b_startx,b_starty;  /* Boundry start point */
/*************************************************************************
 * Event callback function for `canvas1'.
 */
Notify_value
v_can_events(win, event, arg, type)
    Xv_window    win;
    Event        *event;
    Notify_arg    arg;
    Notify_event_type type;
{
    /* Process mouse and Keyboard events here */
    int x_gd,y_gd;  /* Data grid coords */
    double  x_km,ht_km;      /* Km coords */
    int field;              /* Data field number */
    int out_of_range_flag;  /* 1 = no such grid point in visible data */
    unsigned char    *ptr;           /* pointer to the data */
    double value;           /* data value after scaling */
    met_record_t *mr;       /* convienence pointer to a data record */
    char    text[128];
    Menu v_field_mu_gen_proc();

    if(event_action(event) == ACTION_MENU ) { /* Right button down  */
        if(event_is_down(event)) {
            menu_show(v_field_mu_gen_proc(NULL, MENU_DISPLAY),win,event,NULL);
        }
    }

    if(event_action(event) == ACTION_ADJUST ) { /* Middle button down  */
        if(event_is_down(event)) {
            gd.save_im_win = 1;
            update_save_panel();
            xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, TRUE,XV_SHOW, TRUE,NULL);
        }
    }
     
    if(event_action(event) == ACTION_SELECT ) { /* Left button up or down  */
        if(event_is_down(event)) {    /* report data value */
            b_startx = event_x(event);
            b_starty = event_y(event);
            field = gd.v_win.field;
            mr = gd.mrec[field];
            pixel_to_km_v(&gd.v_win.margin,b_startx,b_starty,&x_km,&ht_km);
            km_to_grid_v(mr,x_km,ht_km,&x_gd,&y_gd);

            out_of_range_flag = 0;
            x_gd = CLIP(x_gd,0,(mr->cols -1),out_of_range_flag);
            y_gd = CLIP(y_gd,0,(mr->rows -1),out_of_range_flag);
            if(out_of_range_flag) {
                sprintf(text,"V X: %.2fkm   Y: %.2fkm  Is not in this data:\n",x_km,ht_km);
                rd_v_msg(text);
            } else {

                ptr = mr->v_data;
                if(ptr != NULL) {
                    ptr += (mr->v_nx * y_gd) + x_gd;
					if(*ptr == mr->missing_val) {
                        sprintf(text,"Bad or Missing data at this location");
					} else {
                        value = ((double) *ptr * mr->v_scale) + mr->v_bias;
                        sprintf(text,"Value: %.2f %s: %.3f km along line, %.3f %s\n",
                            value,mr->field_name,x_km,ht_km,mr->units_label_sects);
					}
                    rd_v_msg(text);
                }
            }
        }
    }
    
    if(event_id(event) >= 49 && event_id(event) <= 49 +gd.num_datafields) {
        gd.v_win.field = event_id(event) - 49;
    }
    return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*************************************************************************
 * Repaint callback function for `canvas1'.
 */
void
v_can_repaint(canvas, paint_window, display, xid, rects)
    Canvas        canvas;
    Xv_window    paint_window;
    Display        *display;
    Xv_xrectlist    *rects;
{
    Drawable c_xid;
    int c_field = gd.v_win.field;

    if(gd.mrec[c_field]->background_render) {
        c_xid = gd.v_win.field_xid[c_field];
    } else {
        c_xid = gd.v_win.tmp_xid;
    }
    if((gd.v_win.vis_xid != 0) && (c_xid != 0)) {

        if (gd.debug2) fprintf(stderr,"\nDisplaying Vert final image - xid: %d to xid: %d\n",
                c_xid,gd.v_win.vis_xid);
        XCopyArea(gd.dpy,c_xid,
            gd.v_win.vis_xid,
            gd.def_gc,  0,0,
            gd.v_win.can_dim.width,
            gd.v_win.can_dim.height,
            gd.v_win.can_dim.x_pos,
            gd.v_win.can_dim.y_pos);
    }
}

/*************************************************************************
 * V_WIN_EVENTS: Handle resizing events in the verticle display
 */

Notify_value
v_win_events(frame,event,arg,type)
    Frame   frame;
    Event   *event;
    Notify_arg  arg;
    Notify_event_type   type;
{
    static  int in_progress;
    Window  root;    /* Root window ID of drawable */
    int x,y;            /* location of drawable relative to parent */
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */

    switch(event_action(event)) {
        case  WIN_RESIZE: if(!in_progress) {
            height = xv_get(frame,WIN_HEIGHT);
            width = xv_get(frame,WIN_WIDTH);
            if(height == gd.v_win.win_dim.height && width == gd.v_win.win_dim.width) {
                return(notify_next_event_func(frame,(Notify_event) event,arg,type));
            }
            gd.v_win.win_dim.height = height;
            gd.v_win.win_dim.width = width;
            in_progress = 1;
            if(height < gd.v_win.min_height) height = gd.v_win.min_height;
            if(width < gd.v_win.min_width) width = gd.v_win.min_width;

            xv_set(frame,WIN_WIDTH,width,WIN_HEIGHT,height,NULL);

            /* Set the canvas's width */
            xv_set(gd.v_win_v_win_pu->canvas1,XV_HEIGHT,height,XV_WIDTH,width,NULL);
            gd.vcan_xid = xv_get(canvas_paint_window(gd.v_win_v_win_pu->canvas1),XV_XID);
            gd.v_win.vis_xid = gd.vcan_xid;
            /* Clear drawing area */
            XFillRectangle(gd.dpy,gd.vcan_xid,gd.extras.background_color->gc,0,0,width,height);

            XGetGeometry(gd.dpy,gd.v_win.vis_xid,&root,&x,&y,
                &width,&height,&border_width,&depth);
 
            gd.v_win.can_dim.width = width;
            gd.v_win.can_dim.height = height;
            gd.v_win.can_dim.x_pos = x;
            gd.v_win.can_dim.y_pos = y;
            gd.v_win.can_dim.depth = depth;
 
            gd.v_win.img_dim.width = width - (gd.v_win.margin.left + gd.v_win.margin.right); 
            gd.v_win.img_dim.height = height - (gd.v_win.margin.bot + gd.v_win.margin.top);  
            gd.v_win.img_dim.x_pos = gd.v_win.margin.left;
            gd.v_win.img_dim.y_pos = gd.v_win.margin.top;
            gd.v_win.img_dim.depth = depth;
 
            /* Free the old pixmaps and get new ones */
            manage_v_pixmaps(2);
 
            update_save_panel();
			 
            set_redraw_flags(0,1);
 
            in_progress = 0;
        }
        break;
         
        case ACTION_OPEN:
        case ACTION_CLOSE :
                gd.v_win.win_dim.closed = xv_get(frame,FRAME_CLOSED);
        break;

        default:
        break;
    }

    return(notify_next_event_func(frame,(Notify_event) event,arg,type));
}   

/*************************************************************************
 * MANAGE_V_PIXMAPS: Manage the creation/ destruction of pixmaps for the 
 *      vertical display window 
 */ 
 
manage_v_pixmaps(mode) 
    int mode;   /* 1= create, 2 = replace, 3 = destroy */ 
{    
    int i,index; 
         
    switch (mode) {
        case 1:     /* Create all pixmaps */
        case 2:     /* Replace Pixmaps */
            for(i=0; i < gd.num_datafields; i++) {
                if(gd.v_win.field_xid[i]) {
                    XFreePixmap(gd.dpy,gd.v_win.field_xid[i]);                                     
                    gd.v_win.field_xid[i] = 0;
                }
                 /* Create new field Pixmaps */
                if(gd.mrec[i]->background_render) {
                    gd.v_win.field_xid[i] = XCreatePixmap(gd.dpy,
                        gd.v_win.vis_xid,   
                        gd.v_win.can_dim.width,
                        gd.v_win.can_dim.height,
                        gd.v_win.can_dim.depth);
                    gd.v_win.redraw[i] = 1;
                    /* Clear drawing area */
                    XFillRectangle(gd.dpy,gd.v_win.field_xid[i],gd.extras.background_color->gc,
                        0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height);
                }
            }  

            /* Create last stage Pixmap */
            gd.v_win.can_xid =  XCreatePixmap(gd.dpy,
                        gd.v_win.vis_xid,   
                        gd.v_win.can_dim.width,
                        gd.v_win.can_dim.height,
                        gd.v_win.can_dim.depth);
            XFillRectangle(gd.dpy,gd.v_win.can_xid,gd.extras.background_color->gc,
                        0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height);

            /* Create Pixmap for fields that don't update automatically */
            gd.v_win.tmp_xid =  XCreatePixmap(gd.dpy,
                        gd.v_win.vis_xid,   
                        gd.v_win.can_dim.width,
                        gd.v_win.can_dim.height,
                        gd.v_win.can_dim.depth);
            XFillRectangle(gd.dpy,gd.v_win.tmp_xid,gd.extras.background_color->gc,
                        0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height);
             
            for(i=0;i < MAX_FRAMES; i++ ) { 
                if(gd.movie.frame[i].v_xid) {
                    XFreePixmap(gd.dpy,gd.movie.frame[i].v_xid);
                    gd.movie.frame[i].v_xid = 0;
                }
            }        
 
            for(index=(gd.movie.first_index + gd.movie.start_frame),i=0;
                (i < gd.movie.num_pixmaps) && (i < gd.movie.num_frames);
                i++, index++) {

                if(index >= MAX_FRAMES) index -= MAX_FRAMES; 
                gd.movie.frame[index].v_xid = XCreatePixmap(gd.dpy,
                    gd.v_win.vis_xid,   
                    gd.v_win.can_dim.width,
                    gd.v_win.can_dim.height,
                    gd.v_win.can_dim.depth);
                gd.movie.frame[index].redraw_vert = 1;
                /* Clear drawing area */
                XFillRectangle(gd.dpy,gd.movie.frame[index].v_xid,gd.extras.background_color->gc,
                    0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height);
            }
 
        break;
 
        case 3:
            for(i=0;i < MAX_FRAMES; i++ ) {
                if(gd.movie.frame[i].v_xid)
                    XFreePixmap(gd.dpy,gd.movie.frame[i].v_xid);
                    gd.movie.frame[i].v_xid = 0;
            }
            for(i=0; i < gd.num_datafields; i++) {
                if(gd.v_win.field_xid[i]) {
                    XFreePixmap(gd.dpy,gd.v_win.field_xid[i]);
                    gd.v_win.field_xid[i] = 0;
                }
            }
            if(gd.v_win.can_xid) {
                XFreePixmap(gd.dpy,gd.v_win.can_xid);
                gd.v_win.can_xid = 0;
            }
            if(gd.v_win.tmp_xid) {
                XFreePixmap(gd.dpy,gd.v_win.tmp_xid);
                gd.v_win.tmp_xid = 0;
            }

        break;
    }
}

/*************************************************************************
 * Event callback function for `controls1'.
 */
Notify_value
v_pan_event_proc(win, event, arg, type)
    Xv_window   win;
    Event       *event;
    Notify_arg  arg;
    Notify_event_type type;
{
    if(event_id(event) >= 49 && event_id(event) <= 49 +gd.num_datafields) {
        gd.v_win.field = event_id(event) - 49;
    }

    return notify_next_event_func(win, (Notify_event) event, arg, type);
}
 

/*************************************************************************
 * Set field for the vertical cross section 
 */
void
set_v_field(int field_no)
{
    int    i;
    
    gd.v_win.field = field_no;

    for(i=0; i < gd.num_datafields; i++) {
        if(gd.mrec[i]->background_render == 0) gd.v_win.redraw[i] = 1;
    }

    for(i=0; i < MAX_FRAMES; i++) {
        gd.movie.frame[i].redraw_vert = 1;
    }

    if(gd.movie.movie_on ) reset_data_valid_flags(0,1);
}

/*************************************************************************
 * SET_V_FIELD_PROC
 */
void
set_v_field_proc(menu, item)
    Menu        menu;
    Menu_item    item;
{
    set_v_field((int) xv_get(item,XV_KEY_DATA,MENU_KEY));
}
 
/*************************************************************************
 * Menu handler for `v_field_mu'.
 */
Menu
v_field_mu_gen_proc(menu, op)
        Menu            menu;
        Menu_generate   op;
{
    int    i;
    Menu_item mi;
    static Menu field_menu;

        switch (op) {
        case MENU_DISPLAY:
            if(field_menu == 0) {
                field_menu = xv_create(0,MENU,NULL);
            }
            /* Remove any old items */
            for(i=(int) xv_get(field_menu,MENU_NITEMS) ; i > 0; i--) {
                xv_set(field_menu,MENU_REMOVE,i,NULL);
                xv_destroy(xv_get(field_menu,MENU_NTH_ITEM,i));
            }

            /* Build the field menu */
            for(i=0; i < gd.num_datafields; i++) {
              if(gd.mrec[i]->currently_displayed) {
                  mi = xv_create(0,MENUITEM,
                    MENU_STRING, gd.mrec[i]->field_name,
                    MENU_NOTIFY_PROC,    set_v_field_proc,
                    XV_KEY_DATA,    MENU_KEY, i,
                    NULL);
                  xv_set(field_menu,MENU_APPEND_ITEM,mi,NULL);
              }
            }

                break;

        case MENU_DISPLAY_DONE:
                break;

        case MENU_NOTIFY:
                break;
 
        case MENU_NOTIFY_DONE:
                break;
        }
        return field_menu;
}

#ifndef LINT
static char RCS_id[] = "$Id: v_win_proc.c,v 1.10 2016/03/07 18:28:26 dixon Exp $";
#endif /* not LINT */
