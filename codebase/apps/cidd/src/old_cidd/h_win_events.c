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
 * H_WIN_EVENTS.C: EWvent handling for the Horiz. view window 
 *
 * For the Cartesian Interactive Data Display (CIDD) 
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

static int    b_lastx,b_lasty;    /* Boundry end point */
static int    b_startx,b_starty;    /* Boundry start point */
static int    p_lastx,p_lasty;    /* Pan end point */
static int    p_startx,p_starty;    /* Pan start point */
static int    r_lastx,r_lasty;    /* RHI end point */
static int    r_startx,r_starty;    /* RHI start point */

/*
 * Declare notify procedures explicitly called in the code when
 * certain keyboard keys are pressed by the user to simulate mouse actions.
 */

void erase_zoom_box();
void redraw_zoom_box();
extern void field_proc();
extern void sl_proc();
extern void movie_frame_proc();
extern void update_save_panel();
/*************************************************************************
 * Event callback function for `canvas1'.
 */
Notify_value
can_event_proc(win, event, arg, type)
    Xv_window    win;
    Event        *event;
    Notify_arg    arg;
    Notify_event_type type;
{
    
    int   x, y;
    int   dx,dy;                /* pixel distances */
    int   x_gd,y_gd;            /* Data grid coords */
    int   slide_pos;
    int   slide_delt;
    int   field;                /* Data field number */
    int   out_of_range_flag;    /* 1 = no such grid point in visible data */
    int   num_fields;         /* num fields in field choice list */
    int   field_index;        /* field index in field choice list */
    int   curr_elev;
    int   curr_frame;
    int   max_frame;
    int msg[8], button;
    
     
    double    x_km,y_km;        /* Km coords */
    double    lat,lon;        /* latitude, longitude coords */
    double value;            /* data value after scaling */
    double    delta;

    unsigned char    *ptr;            /* pointer to the data */
    char    label[32];
    char    text[128];
    char    lat_string[16];
    char    lon_string[16];
    met_record_t    *mr;

    struct timeval tp;
    struct timezone tzp;

    static int sid = 0, button_up = 0;
    static int last_frame = -1;

    /* Move to front of drag events */
    while (XCheckMaskEvent(gd.dpy, ButtonMotionMask, event->ie_xevent))
        /* NULL BODY */;
    field = gd.h_win.field;
    mr = gd.mrec[field];

    /* process mouse events based on current drawing mode */
    if (gd.drawing_mode) {
        if (event_is_up(event) || event_is_down(event)) {
            button = event_id(event) - MS_LEFT;
              if(button >=0 && button <= 2) {
                if (event_is_down(event)) {
                    sid = 2 - button;
                    button_up = 1;
                } else button_up = 0;
                msg[0] = sid;
                msg[1] = button_up;
                window_to_world(event->ie_locx, event->ie_locy, &x, &y);
                msg[2] = x;
                msg[3] = y;
                send_event(msg);
              }
        } /* endif - button up/down event */

        if (event_id(event) == LOC_MOVEWHILEBUTDOWN) {
            button_up = 2;
            msg[0] = sid;
            msg[1] = button_up;
            window_to_world(event->ie_locx, event->ie_locy, &x, &y);
            msg[2] = x;
            msg[3] = y;
            send_event(msg);
            
        } /* endif - move while button down */
    }  else {                                  /* endif - in drawing mode */

        gettimeofday(&tp, &tzp);                /* record when this event took place */

        /* Mouse is move while any button is down */
        if (event_action(event) == LOC_DRAG ) {
            if(event_left_is_down(event))    /* ZOOM Define */ {
                redraw_zoom_box();    /* Erase the last line */
                b_lastx = event_x(event);
                b_lasty = event_y(event);
                redraw_zoom_box();    /* Erase the last line */
            }
            if ( event_middle_is_down(event))    /* PAN Adjust */ {
                if(gd.h_win.zoom_level >= 1) {
                    redraw_pan_line();
                    p_lastx = event_x(event);
                    p_lasty = event_y(event);
                    redraw_pan_line();
                }
            }

            if(!gd.limited_mode &&  event_right_is_down(event))    /* RHI Define */ {
                redraw_rhi_line();
                r_lastx = event_x(event);
                r_lasty = event_y(event);
                redraw_rhi_line();
            }
        }

        /* Left button up or down  */
        if(event_action(event) == ACTION_SELECT ) {
            if(event_is_down(event)) {
                gd.zoom_in_progress = 1;
                b_startx = event_x(event);
                b_starty = event_y(event);
                b_lastx = b_startx;
                b_lasty = b_starty;
            } else {
                gd.zoom_in_progress = 0;

                pixel_to_km(&gd.h_win.margin, b_lastx, b_lasty, &x_km, &y_km);

		switch(gd.projection_mode) {
		  case CARTESIAN_PROJ:
                     PJGLatLonPlusDxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,
			x_km,y_km,&lat,&lon);
		  break;

		  case LAT_LON_PROJ:
		      lat = y_km;
		      lon = x_km;
		  break;
		}

		/* Nicely format the lat lons */
		switch(gd.latlon_mode) {
		    default:
		    case 0:  /* Decimal Degrees */
			sprintf(lat_string,"%.4f",lat);
			sprintf(lon_string,"%.4f",lon);
		    break;

		    case 1:   /* Degrees minutes seconds */
		       {
		       int ideg,imin;
		       double fmin,fsec;
			
		       ideg = (int) lat;
		       fmin = fabs(lat - ideg); /* extract decimal fraction */
		       imin = (int) (fmin * 60.0);
		       fsec = (fmin - (imin / 60.0)) * 3600.0; 
		       sprintf(lat_string,"%d %d\' %.0f\"",ideg,imin,fsec);

		       ideg = (int) lon;
		       fmin = fabs(lon - ideg); /* extract decimal fraction */
		       imin = (int) (fmin * 60.0);
		       fsec = (fmin - (imin / 60.0)) * 3600.0; 
		       sprintf(lon_string,"%d %d\' %.0f\"",ideg,imin,fsec);
		       }
		    break;
		}

                gd.coord_expt->button = event->ie_xevent->xbutton.button;
                gd.coord_expt->selection_sec = tp.tv_sec;
                gd.coord_expt->selection_usec = tp.tv_usec;
                gd.coord_expt->pointer_x = x_km;
                gd.coord_expt->pointer_y = y_km;
                gd.coord_expt->pointer_lon = lon;
                gd.coord_expt->pointer_lat = lat;
                gd.coord_expt->pointer_alt_min = gd.h_win.cmin_ht;
                gd.coord_expt->pointer_alt_max = gd.h_win.cmax_ht;
                gd.coord_expt->data_altitude = mr->vert[mr->plane].cent;
                gd.coord_expt->pointer_seq_num++;

                if ((abs(b_lastx - b_startx) > 20 || abs(b_lasty - b_starty) > 20)) {
                    do_zoom();
                } else        /* report data value */ {
                    pixel_to_grid(mr, &gd.h_win.margin, b_lastx, b_lasty, &x_gd, &y_gd);

                    out_of_range_flag = 0;
                    x_gd = CLIP(x_gd,0,(mr->cols -1),out_of_range_flag);
                    y_gd = CLIP(y_gd,0,(mr->rows -1),out_of_range_flag);
                    if(out_of_range_flag) {
                        sprintf(text, "X,Y: %.2fkm,%.2fkm  LAT,LON:%s,%s  Outside Data grid:\n",
							x_km, y_km,
							lat_string,lon_string);
                        rd_h_msg(text,-5);
                    } else {
		      if(gd.movie.cur_frame == last_frame) {
                        if(mr->h_data != NULL) {
                            ptr = mr->h_data;
                            ptr += (mr->h_nx * (y_gd - mr->y1)) +
                                (x_gd - mr->x1);
							if(*ptr == mr->missing_val) {
                                sprintf(text, "%s: No Data at X,Y:%.2f,%.2f LAT,LON:%s,%s \n",
                                    mr->field_name, 
                                    x_km, y_km,
                                    lat_string,lon_string);
							} else {
                                value = ((double) *ptr * mr->h_scale) + mr->h_bias;
                                sprintf(text, "%s VALUE:%.2f   X,Y:%.2f,%.2f   LAT,LON:%s,%s\n",
                                    mr->field_name,value, 
                                    x_km, y_km,
                                    lat_string,lon_string);
							}
                            rd_h_msg(text,-5);
                        }
		      } else {
			    mr->h_data_valid = 0;
			    gather_hwin_data(gd.h_win.field,
				gd.movie.frame[gd.movie.cur_index].time_start.unix_time,
				gd.movie.frame[gd.movie.cur_index].time_end.unix_time);
				
                            rd_h_msg("Try Again",-1);
			    last_frame = gd.movie.cur_frame;
		      }
                    }
                } 
            }
        }

        /* Middle button up or down */
        if(event_action(event) == ACTION_ADJUST ) {
            if(gd.h_win.zoom_level >= 1) {
                if(event_is_down(event)) {
                    gd.pan_in_progress = 1;
                    p_startx = event_x(event);
                    p_starty = event_y(event);
                    p_lastx = p_startx;
                    p_lasty = p_starty;

                    pixel_to_km(&gd.h_win.margin, p_lastx, p_lasty, &x_km, &y_km);

                    PJGLatLonPlusDxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,x_km,y_km,&lat,&lon);
                
                    gd.coord_expt->button = event->ie_xevent->xbutton.button;
                    gd.coord_expt->selection_sec = tp.tv_sec;
                    gd.coord_expt->selection_usec = tp.tv_usec;
                    gd.coord_expt->pointer_x = x_km;
                    gd.coord_expt->pointer_y = y_km;
                    gd.coord_expt->pointer_lon = lon;
                    gd.coord_expt->pointer_lat = lat;
                    gd.coord_expt->pointer_alt_min = gd.h_win.cmin_ht;
                    gd.coord_expt->pointer_alt_max = gd.h_win.cmax_ht;
                    gd.coord_expt->data_altitude = mr->vert[mr->plane].cent;
                    gd.coord_expt->pointer_seq_num++;

                } else {
                    if (abs(p_lastx - p_startx) > 20 || abs(p_lasty - p_starty) > 20) {
                        do_zoom_pan();
                    } else {
                       gd.save_im_win = 0;
                       update_save_panel();
                       xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, TRUE,XV_SHOW, TRUE,NULL);
					}
                    gd.pan_in_progress = 0;
                }
            }
        }


     
        /* RIGHT Button up or down */
        if (!gd.limited_mode && event_action(event) == ACTION_MENU ) {
            if(event_is_down(event)) {
                r_startx = event_x(event);
                r_starty = event_y(event);
                r_lastx = r_startx;
                r_lasty = r_starty;
                gd.rhi_in_progress = 1;

                pixel_to_km(&gd.h_win.margin, r_lastx, r_lasty, &x_km, &y_km);

                PJGLatLonPlusDxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,x_km,y_km,&lat,&lon);
                
                gd.coord_expt->button = event->ie_xevent->xbutton.button;
                gd.coord_expt->selection_sec = tp.tv_sec;
                gd.coord_expt->selection_usec = tp.tv_usec;
                gd.coord_expt->pointer_x = x_km;
                gd.coord_expt->pointer_y = y_km;
                gd.coord_expt->pointer_lon = lon;
                gd.coord_expt->pointer_lat = lat;
                gd.coord_expt->pointer_alt_min = gd.h_win.cmin_ht;
                gd.coord_expt->pointer_alt_max = gd.h_win.cmax_ht;
                gd.coord_expt->data_altitude = mr->vert[mr->plane].cent;
                gd.coord_expt->pointer_seq_num++;
            } else {
                if(abs(r_lastx - r_startx) > 20 || abs(r_lasty - r_starty) > 20) {
                    setup_rhi_area();
                }
                gd.rhi_in_progress = 0;
            }
        }
    } /* end else - not in drawing mode */

    /*
     * Process keyboard events.  Only process them when the key is
     * released to prevent repeated processing when the key is held
     * down (may want to change this in the future).
     */

    if (event_is_up(event)) {
        /*
         * Pressing a number key selects the corresponding field in the field
         * choice list.
         */

        num_fields = gd.num_menu_fields;
    
        if (event_id(event) >= '1' && event_id(event) <= '1' + num_fields - 1) {
            field_index = event_id(event) - '1';
        
            /* simulate the user selecting a new field */
            set_field(field_index);
        }

        /*
         * Arrow keys:
         *    up    - move up one elevation
         *    down  - move down on elevation
         *    left  - move back one movie frame
         *    right - move forward one movie frame
         */

        switch(event_action(event)) {
        case ACTION_GO_COLUMN_BACKWARD :      /* ACTION_UP */
            slide_pos = (int) xv_get(gd.h_win_horiz_bw->slice_sl, PANEL_VALUE);
	    if(slide_pos < gd.h_win.num_slices -2) 
                sl_proc(gd.h_win_horiz_bw->slice_sl, slide_pos + 1, (Event *)NULL);
        break;
        
        case ACTION_GO_COLUMN_FORWARD :         /* ACTION_DOWN */
            slide_pos = (int)xv_get(gd.h_win_horiz_bw->slice_sl,    PANEL_VALUE);
	    if(slide_pos > 0) 
                sl_proc(gd.h_win_horiz_bw->slice_sl, slide_pos - 1, (Event *)NULL);
        break;

        case ACTION_GO_CHAR_BACKWARD :       /* ACTION_LEFT */
            curr_frame = (int)xv_get(gd.movie_pu->movie_frame_sl,
                                     PANEL_VALUE);
            
            if (curr_frame > 1) {
                curr_frame--;
                
                xv_set(gd.movie_pu->movie_frame_sl,
                       PANEL_VALUE, curr_frame,
                       NULL);
                
                movie_frame_proc(gd.movie_pu->movie_frame_sl,
                                 curr_frame,
                                 (Event *)NULL);
            }
            
        break;
        
        case ACTION_GO_CHAR_FORWARD :         /* ACTION_RIGHT */
            curr_frame = (int)xv_get(gd.movie_pu->movie_frame_sl,
                                     PANEL_VALUE);
            max_frame = (int)xv_get(gd.movie_pu->movie_frame_sl,
                                    PANEL_MAX_VALUE);
            
            if (curr_frame < max_frame) {
                curr_frame++;
                
                xv_set(gd.movie_pu->movie_frame_sl,
                       PANEL_VALUE, curr_frame,
                       NULL);
                
                movie_frame_proc(gd.movie_pu->movie_frame_sl,
                                 curr_frame,
                                 (Event *)NULL);
            }
            
        break;
        
        default:
        break;
        } /* endswitch - event_action(event) */

    } /* endif - event_is_up */
    

    return notify_next_event_func(win, (Notify_event) event, arg, type);
}
 
/*************************************************************************
 * Repaint callback function for `canvas1'.
 */
void
can_repaint(canvas, paint_window, display, xid, rects)
    Canvas        canvas;
    Xv_window    paint_window;
    Display        *display;
    Window        xid;
    Xv_xrectlist    *rects;
{
    Drawable c_xid;
    int    c_field = gd.h_win.field;


    if((gd.h_win.vis_xid != 0) && (gd.h_win.can_xid != 0)) {
        if (gd.debug2) fprintf(stderr,"\nRepaint: Displaying Horiz final image xid: %d to xid: %d\n",
				gd.h_win.can_xid,gd.h_win.vis_xid);
        XCopyArea(gd.dpy,gd.h_win.can_xid,
                  gd.h_win.vis_xid,
                  gd.def_gc,  0,0,
                  gd.h_win.can_dim.width,
                  gd.h_win.can_dim.height,
                  gd.h_win.can_dim.x_pos,
                  gd.h_win.can_dim.y_pos);
    }

}

/*************************************************************************
 * H_WIN_EVENTS: Handle resizing events
 */

Notify_value
h_win_events(frame, event, arg, type)
    Frame   frame;
    Event   *event;
    Notify_arg  arg;
    Notify_event_type   type;
{
    static  int in_progress;
    Window  root;     /* Root window ID of drawable */
    int x,y;            /* location of drawable relative to parent */
    int    cp_width;        /* control panel width */
    int    cp_height;        /* control panel height */
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
 
    switch(event_action(event))
    {
    case  WIN_RESIZE:
        if (!in_progress)
        {
            height = xv_get(frame,WIN_HEIGHT);
            cp_height = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT);
            width = xv_get(frame,WIN_WIDTH);
            if (height == gd.h_win.win_dim.height && width == gd.h_win.win_dim.width) {
                return(notify_next_event_func(frame,(Notify_event) event,arg,type));
            }
	     
            gd.h_win.win_dim.height = height;
            gd.h_win.win_dim.width = width;

            in_progress = 1;
            if(height < gd.h_win.min_height) height = gd.h_win.min_height;


            /* Set the base frame's width */
            width =  ((height  - cp_height - gd.h_win.margin.top - gd.h_win.margin.bot) * gd.aspect_ratio)  +
                gd.h_win.margin.left + gd.h_win.margin.right;

            xv_set(frame,
                   WIN_WIDTH, width,
                   WIN_HEIGHT, height,
                   NULL);

            /* Set the canvas's width, height */
	    height = height - cp_height;
            xv_set(gd.h_win_horiz_bw->canvas1,
                   XV_HEIGHT, height,
                   XV_WIDTH, width,
                   NULL);

            xv_set(gd.h_win_horiz_bw->cur_time_msg,XV_X,
				 (width - xv_get(gd.h_win_horiz_bw->cur_time_msg,XV_WIDTH)) -5 ,
				 NULL);
		    
            gd.hcan_xid = xv_get(canvas_paint_window(gd.h_win_horiz_bw->canvas1),XV_XID);
            gd.h_win.vis_xid = gd.hcan_xid;
            /* clear drawing area */
            XFillRectangle(gd.dpy, gd.hcan_xid, gd.extras.background_color->gc,
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

            /* Free the old pixmaps and get new ones */
            manage_h_pixmaps(2);
            update_save_panel();
            
            set_redraw_flags(1,0);
            /* reset_data_valid_flags(1,1); /* Seems unnecessary? 5/97 - FH.  */
            in_progress = 0;
        }
        break;

    case ACTION_OPEN:
        gd.h_win.win_dim.closed = xv_get(frame,FRAME_CLOSED);
        break;    
 
    case ACTION_CLOSE :
        gd.h_win.win_dim.closed = xv_get(frame,FRAME_CLOSED);
        xv_set(gd.data_pu->data_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
        xv_set(gd.zoom_pu->zoom_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
        xv_set(gd.extras_pu->extras_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
        xv_set(gd.v_win_v_win_pu->v_win_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
        xv_set(gd.movie_pu->movie_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
        xv_set(gd.fields_pu->fields_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
        xv_set(gd.h_win_horiz_bw->field_st,PANEL_VALUE,0, NULL);
        xv_set(gd.h_win_horiz_bw->zoom_st,PANEL_VALUE,0, NULL);
        xv_set(gd.h_win_horiz_bw->export_st,PANEL_VALUE,0, NULL);
        xv_set(gd.h_win_horiz_bw->product_st,PANEL_VALUE,0, NULL);
        xv_set(gd.h_win_horiz_bw->overlay_st,PANEL_VALUE,0, NULL);
        xv_set(gd.h_win_horiz_bw->x_sect_st,PANEL_VALUE,0, NULL);
        xv_set(gd.h_win_horiz_bw->movie_st,PANEL_VALUE,0, NULL);
        xv_set(gd.h_win_horiz_bw->field_sel_st,PANEL_VALUE,0, NULL);

        switch_the_window_expt(0);
        switch_the_window_prds(0);
        break;    
 
    default:
        break;
    }
 
    return(notify_next_event_func(frame,(Notify_event) event,arg,type));
}
/*************************************************************************
 * Event callback function for `cp'.
 */
Notify_value
h_pan_event_proc(win, event, arg, type)
    Xv_window   win;
    Event       *event;
    Notify_arg  arg;
    Notify_event_type type;
{
    int   num_fields;
    int   field_index;
    int   slide_pos;
    double    delta;
    char    label[32];
    met_record_t    *mr;
    
    /*
     * Process keyboard events.  Only process them when the key is
     * released to prevent repeated processing when the key is held
     * down (may want to change this in the future).
     */

    if (event_is_up(event))
    {
        
        /*
         * Pressing a number key selects the corresponding field in the field
         * choice list.
         */

        num_fields = gd.num_menu_fields;
        mr = gd.mrec[gd.h_win.field];
    
        if (event_id(event) >= '1' && event_id(event) <= '1' + num_fields - 1)
        {
            field_index = event_id(event) - '1';
        
            /* simulate the user selecting a new field */
            set_field(field_index);
        }

        /*
         * Arrow keys:
         *    up    - move up one elevation
         *    down  - move down on elevation
         *    left  - move back one movie frame
         *    right - move forward one movie frame
         */

        switch(event_action(event))
        {
        case ACTION_GO_COLUMN_BACKWARD :      /* ACTION_UP */
            slide_pos = (int)xv_get(gd.h_win_horiz_bw->slice_sl,    PANEL_VALUE);
            if(mr->data_format == PPI_DATA_FORMAT) {
                sl_proc(gd.h_win_horiz_bw->slice_sl, slide_pos, (Event *)NULL);
                break;
            }

            /* Calc the ammount to move to */
            if(mr->plane < mr->sects -1 && mr->plane == 0) {
                delta = mr->vert[mr->plane +1].cent - mr->vert[mr->plane].cent;
            } else { 
                delta = gd.h_win.delta;
            }

            slide_pos += (int) (delta/ gd.h_win.delta) + 0.5;
            if(slide_pos > gd.h_win.num_slices) slide_pos = gd.h_win.num_slices -1;
            xv_set(gd.h_win_horiz_bw->slice_sl,    PANEL_VALUE,slide_pos,NULL);

            gd.h_win.cmin_ht = (slide_pos  * gd.h_win.delta) + gd.h_win.min_ht;
            gd.h_win.cmax_ht = gd.h_win.cmin_ht + gd.h_win.delta;

            sprintf(label,"%g %s",((gd.h_win.cmin_ht + gd.h_win.cmax_ht) /2.0),
		mr->units_label_sects);
            xv_set(gd.h_win_horiz_bw->cur_ht_msg,PANEL_LABEL_STRING,label,NULL);
        
            set_redraw_flags(1,1);
            reset_data_valid_flags(1,1);
        
            break;
        
        case ACTION_GO_COLUMN_FORWARD :         /* ACTION_DOWN */
            slide_pos = (int)xv_get(gd.h_win_horiz_bw->slice_sl,    PANEL_VALUE);
            if(mr->data_format == PPI_DATA_FORMAT) {
                sl_proc(gd.h_win_horiz_bw->slice_sl, slide_pos, (Event *)NULL);
                break;
            }

            /* Calc the ammount to move to */
            if(mr->plane > 0  && mr->plane < mr->sects) {
                delta = mr->vert[mr->plane -1].cent - mr->vert[mr->plane].cent;
            } else { 
                delta = -gd.h_win.delta;
            }

            slide_pos += (int) (delta/ gd.h_win.delta) - 0.5;
            if (slide_pos < 0) slide_pos = 0;
            xv_set(gd.h_win_horiz_bw->slice_sl,    PANEL_VALUE,slide_pos,NULL);

            gd.h_win.cmin_ht = (slide_pos  * gd.h_win.delta) + gd.h_win.min_ht;
            gd.h_win.cmax_ht = gd.h_win.cmin_ht + gd.h_win.delta;

            sprintf(label,"%g %s",((gd.h_win.cmin_ht + gd.h_win.cmax_ht) /2.0),
		mr->units_label_sects);
            xv_set(gd.h_win_horiz_bw->cur_ht_msg,PANEL_LABEL_STRING,label,NULL);
        
            set_redraw_flags(1,1);
        
            break;
        
        case ACTION_GO_CHAR_BACKWARD :       /* ACTION_LEFT */
            break;
        
        case ACTION_GO_CHAR_FORWARD :        /* ACTION_RIGHT */
            break;
        
        default:
            break;
        } /* endswitch - event_action(event) */

    } /* endif - event_is_up */


    return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*************************************************************************
 * SETUP_RHI_AREA :   Set the endpoints of the vertical plane for each
 *        data field
 */

setup_rhi_area()
{
    /* get the X,Y coordinates in Km of the two endpoints */
    pixel_to_km(&gd.h_win.margin, r_startx, r_starty,
                &gd.v_win.cmin_x, &gd.v_win.cmin_y);
    pixel_to_km(&gd.h_win.margin, r_lastx, r_lasty,
                &gd.v_win.cmax_x, &gd.v_win.cmax_y);
     
    xv_set(gd.h_win_horiz_bw->x_sect_st,PANEL_VALUE,1,NULL);
    x_sect_ctl_proc(0,1,NULL);
     
    gd.v_win.active = 1;

    set_redraw_flags(1,1);
    reset_data_valid_flags(0,1);
}
 
/*************************************************************************
 * DO_ZOOM : Zoom area
 */

do_zoom()
{
    int index;
    double    km_x1,km_x2,km_y1,km_y2;
    double    dx,dy;
	double  min_zoom_threshold;
    double  theta1, theta2;
    double  r1, r2;
     
    pixel_to_km(&gd.h_win.margin,b_startx,b_starty,&km_x1,&km_y1);
    pixel_to_km(&gd.h_win.margin,b_lastx,b_lasty,&km_x2,&km_y2);

    if(gd.h_win.zoom_level < gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS) {
          index = gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS;
    } else {
      index = gd.h_win.zoom_level +1;
      if(index >= gd.h_win.num_zoom_levels)
          index = gd.h_win.num_zoom_levels -1;
    }

    switch(gd.projection_mode) {
		default:
			min_zoom_threshold = 5.0; /* km */
		break;
	    case LAT_LON_PROJ:
			min_zoom_threshold = 0.01; /* Degrees */
		break;
	}
     
/* Only do additional zooming if request is at least min_zoom_threshold on one side */
if((fabs(km_x2 - km_x1) > min_zoom_threshold) ||
   (fabs(km_y2 - km_y1) > min_zoom_threshold)) {
    if(km_x1 < km_x2)     /* put coords in ascending order */
    {
        gd.h_win.zmin_x[index] = km_x1;
        gd.h_win.zmax_x[index] = km_x2;
    }
    else
    {
        gd.h_win.zmin_x[index] = km_x2;
        gd.h_win.zmax_x[index] = km_x1;
    }
    if(km_y1 < km_y2)
    {     
        gd.h_win.zmin_y[index] = km_y1;
        gd.h_win.zmax_y[index] = km_y2;
    }
    else
    {
        gd.h_win.zmin_y[index] = km_y2;
        gd.h_win.zmax_y[index] = km_y1;
    }
}

    /* Force zoomed display area to be consistant with the window */
    dx = gd.h_win.zmax_x[index] - gd.h_win.zmin_x[index];
    dy = gd.h_win.zmax_y[index] - gd.h_win.zmin_y[index];

    switch(gd.projection_mode) {
	case LAT_LON_PROJ:
	   /* forshorten the Y coords to make things look better */
	   dy /= gd.aspect_correction;
	break;
    }

    /* Force the domain into the aspect ratio */
    dy *= gd.aspect_ratio;

    /* use largest dimension */
    if(dx > dy)
    {
        gd.h_win.zmax_y[index] += (dx - dy) /2.0;
        gd.h_win.zmin_y[index] -= (dx - dy) /2.0;
    }
    else
    {
        gd.h_win.zmax_x[index] += (dy - dx) /2.0;
        gd.h_win.zmin_x[index] -= (dy - dx) /2.0;
    }

    /* make sure coords are within the limits of the display */
    if(gd.h_win.zmax_x[index] > gd.h_win.max_x)
    {
        gd.h_win.zmin_x[index] -= (gd.h_win.zmax_x[index] - gd.h_win.max_x);
        gd.h_win.zmax_x[index] = gd.h_win.max_x;
    }
    if(gd.h_win.zmax_y[index] > gd.h_win.max_y)
    {
        gd.h_win.zmin_y[index] -= (gd.h_win.zmax_y[index] - gd.h_win.max_y);
        gd.h_win.zmax_y[index] = gd.h_win.max_y;
    }
    if(gd.h_win.zmin_x[index] < gd.h_win.min_x)
    {
        gd.h_win.zmax_x[index] += (gd.h_win.min_x - gd.h_win.zmin_x[index]);
        gd.h_win.zmin_x[index] = gd.h_win.min_x;
    }
    if(gd.h_win.zmin_y[index] < gd.h_win.min_y)
    {
        gd.h_win.zmax_y[index] += (gd.h_win.min_y - gd.h_win.zmin_y[index]);
        gd.h_win.zmin_y[index] = gd.h_win.min_y;
    }

    /* set radial global values if radial data being displayed */
    if (gd.mrec[gd.h_win.field]->data_format == PPI_DATA_FORMAT)
        zoom_radial(gd.h_win.zmin_x[index], gd.h_win.zmin_y[index],
                    gd.h_win.zmax_x[index], gd.h_win.zmax_y[index]);

    /* Set current area to our indicated zoom area */
    gd.h_win.cmin_x = gd.h_win.zmin_x[index];
    gd.h_win.cmax_x = gd.h_win.zmax_x[index];
    gd.h_win.cmin_y = gd.h_win.zmin_y[index];
    gd.h_win.cmax_y = gd.h_win.zmax_y[index];

    gd.h_win.cmin_r = gd.h_win.zmin_r;
    gd.h_win.cmax_r = gd.h_win.zmax_r;
    gd.h_win.cmin_deg = gd.h_win.zmin_deg;
    gd.h_win.cmax_deg = gd.h_win.zmax_deg;
     
    set_redraw_flags(1,0);
    if(!gd.always_get_full_domain) reset_data_valid_flags(1,0);

    gd.h_win.zoom_level = index;
    xv_set(gd.zoom_pu->domain_st, PANEL_VALUE, index, NULL);
    return 0;
}

/*************************************************************************
 * ZOOM_RADIAL : Calculate the zoom values for radial data
 */

zoom_radial(min_x, min_y, max_x, max_y)

double min_x;
double min_y;
double max_x;
double max_y;

{
    double theta1, theta2;
    double r1, r2;
    
    if (min_x >= 0 && min_y >= 0)       /* quadrant I */
    {
        /* calculate the angles limiting the zoomed area */
        theta1 = (acos((double)max_y /
                       (double)R(min_x, max_y))) / DEG_TO_RAD;
        theta2 = (acos((double)min_y /
                       (double)R(max_x, min_y))) / DEG_TO_RAD;

        /* calculate the beam limits of the zoomed area */
        r1 = R(min_x, min_y);
        r2 = R(max_x, max_y);
    }
    else if (min_x >= 0 && min_y < 0)
    {
        if (max_y >= 0)                 /* quadrant I/II */
        {
            /* calculate the angles limiting the zoomed area */
            theta1 = (acos((double)max_y /
                           (double)R(min_x, max_y))) / DEG_TO_RAD;
            theta2 = (acos((double)min_y /
                           (double)R(min_x, min_y))) / DEG_TO_RAD;

            /* calculate the beam limits of the zoomed area */
            r1 = min_x;
            if (max_y >= -min_y)
                r2 = R(max_x, max_y);
            else
                r2 = R(max_x, min_y);
        
        }
        else                            /* quadrant II */
        {
            /* calculate the angles limiting the zoomed area */
            theta1 = (acos((double)max_y /
                           (double)R(max_x, max_y))) / DEG_TO_RAD;
            theta2 = (acos((double)min_y /
                           (double)R(min_x, min_y))) / DEG_TO_RAD;

            /* calculate the beam limits of the zoomed area */
            r1 = R(min_x, max_y);
            r2 = R(max_x, min_y);
        
        }
        
    }
    else if (min_x < 0 && min_y < 0)
    {
        if (max_x >= 0 && max_y >= 0)   /* quadrant I/II/III/IV */
        {
            /* calculate the angles limiting the zoomed area */
            theta1 = 0.0;
            theta2 = 360.0;

            /* calculate the beam limits of the zoomed area */
            r1 = 0.0;
            if (max_y >= -min_y)
            {
                if (max_x >= -min_x)
                    r2 = R(max_x, max_y);
                else
                    r2 = R(min_x, max_y);
            }
            else
            {
                if (max_x >= -min_x)
                    r2 = R(max_x, min_y);
                else
                    r2 = R(min_x, min_y);
            }
            

        }
        else if (max_x >= 0)            /* quadrant II/III */
        {
            /* calculate the angles limiting the zoomed area */
            theta1 = (acos((double)max_y /
                           (double)R(max_x, max_y))) / DEG_TO_RAD;
            theta2 = 360.0 - ((acos((double)max_y /
                                    (double)R(min_x, max_y))) / DEG_TO_RAD);

            /* calculate the beam limits of the zoomed area */
            r1 = -max_y;
            if (max_x >= -min_x)
                r2 = R(max_x, min_y);
            else
                r2 = R(min_x, min_y);
        
        }
        else if (max_y >= 0)            /* quadrant III/IV */
        {
            /* calculate the angles limiting the zoomed area */
            theta1 = 360.0 - ((acos((double)min_y /
                                    (double)R(max_x, min_y))) / DEG_TO_RAD);
            theta2 = 360.0 - ((acos((double)max_y /
                                    (double)R(max_x, max_y))) / DEG_TO_RAD);

            /* calculate the beam limits of the zoomed area */
            r1 = -max_x;
            if (max_y >= -min_y)
                r2 = R(min_x, max_y);
            else
                r2 = R(min_x, min_y);

        }
        else                            /* quadrant III */
        {
            /* calculate the angles limiting the zoomed area */
            theta1 = 360.0 - ((acos((double)min_y /
                                    (double)R(max_x, min_y))) / DEG_TO_RAD);
            theta2 = 360.0 - ((acos((double)max_y /
                                    (double)R(min_x, max_y))) / DEG_TO_RAD);

            /* calculate the beam limits of the zoomed area */
            r1 = R(max_x, max_y);
            r2 = R(min_x, min_y);
        
        }
    }
    else
    {
        if (max_x >= 0)                 /* quadrant I/IV */
        {
            /* calculate the angles limiting the zoomed area */
            theta1 = gd.h_win.min_deg;
            theta2 = gd.h_win.max_deg;

            /* calculate the beam limits of the zoomed area */
            r1 = min_y;
            if (max_x >= -min_x)
                r2 = R(max_x, max_y);
            else
                r2 = R(min_x, max_y);
        
        }
        else                            /* quadrant IV */
        {
            /* calculate the angles limiting the zoomed area */
            theta1 = 360.0 - ((acos((double)min_y /
                                    (double)R(min_x, min_y))) / DEG_TO_RAD);
            theta2 = 360.0 - ((acos((double)max_y /
                                    (double)R(max_x, max_y))) / DEG_TO_RAD);

            /* calculate the beam limits of the zoomed area */
            r1 = R(max_x, min_y);
            r2 = R(min_x, max_y);
        
        }
    }
    
    /* update the global radial beam limits */
    gd.h_win.zmin_r = r1;
    gd.h_win.zmax_r = r2;
        
    /* update the global radial angle limits */
    gd.h_win.zmin_deg = theta1;
    gd.h_win.zmax_deg = theta2;
    
} /* end of zoom_radial */

/*************************************************************************
 * DO_ZOOM_PAN : Pan the zoomed area
 */

do_zoom_pan()
{
    int    index;
    double    km_x1,km_x2,km_y1,km_y2;
    double    dx,dy;
     
    pixel_to_km(&gd.h_win.margin,p_startx,p_starty,&km_x1,&km_y1);
    pixel_to_km(&gd.h_win.margin,p_lastx,p_lasty,&km_x2,&km_y2);
     
    dx = km_x1 - km_x2;
    dy = km_y1 - km_y2;

    if(gd.h_win.zoom_level < gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS) {
          index = gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS;
    } else {
      index = gd.h_win.zoom_level;
      if(index >= gd.h_win.num_zoom_levels)
          index = gd.h_win.num_zoom_levels -1;
    }

    gd.h_win.zmin_x[index] = gd.h_win.cmin_x + dx;
    gd.h_win.zmin_y[index] = gd.h_win.cmin_y + dy;
    gd.h_win.zmax_x[index] = gd.h_win.cmax_x + dx;
    gd.h_win.zmax_y[index] = gd.h_win.cmax_y + dy;

    /* make sure coords are within the limits of the display */
    if(gd.h_win.zmax_x[index] > gd.h_win.max_x)
    {
        gd.h_win.zmin_x[index] -= (gd.h_win.zmax_x[index] - gd.h_win.max_x);
        gd.h_win.zmax_x[index] = gd.h_win.max_x;
    }
    if(gd.h_win.zmax_y[index] > gd.h_win.max_y)
    {
        gd.h_win.zmin_y[index] -= (gd.h_win.zmax_y[index] - gd.h_win.max_y);
        gd.h_win.zmax_y[index] = gd.h_win.max_y;
    }
    if(gd.h_win.zmin_x[index] < gd.h_win.min_x)
    {
        gd.h_win.zmax_x[index] += (gd.h_win.min_x - gd.h_win.zmin_x[index]);
        gd.h_win.zmin_x[index] = gd.h_win.min_x;
    }
    if(gd.h_win.zmin_y[index] < gd.h_win.min_y)
    {
        gd.h_win.zmax_y[index] += (gd.h_win.min_y - gd.h_win.zmin_y[index]);
        gd.h_win.zmin_y[index] = gd.h_win.min_y;
    }

    /* set radial global values if radial data being displayed */
    if (gd.mrec[gd.h_win.field]->data_format == PPI_DATA_FORMAT)
        zoom_radial(gd.h_win.zmin_x[index], gd.h_win.zmin_y[index],
                    gd.h_win.zmax_x[index], gd.h_win.zmax_y[index]);

    /* Set current area to our indicated zoom area */
    gd.h_win.cmin_x = gd.h_win.zmin_x[index];
    gd.h_win.cmax_x = gd.h_win.zmax_x[index];
    gd.h_win.cmin_y = gd.h_win.zmin_y[index];
    gd.h_win.cmax_y = gd.h_win.zmax_y[index];
    gd.h_win.cmin_r = gd.h_win.zmin_r;
    gd.h_win.cmax_r = gd.h_win.zmax_r;
    gd.h_win.cmin_deg = gd.h_win.zmin_deg;
    gd.h_win.cmax_deg = gd.h_win.zmax_deg;
     
    set_redraw_flags(1,0);
    if(!gd.always_get_full_domain) reset_data_valid_flags(1,0);

    gd.h_win.zoom_level = index;
    xv_set(gd.zoom_pu->domain_st, PANEL_VALUE, index, NULL);

    return 0;
}


/*************************************************************************
 * ERASE_ZOOM_BOX:  Erases the Zoom box 
 *
 */

void
erase_zoom_box()
{
    int    dx,dy;                /* pixel distances */
    int    sx,sy;

    dx = b_lastx - b_startx;
    dy = b_lasty - b_starty;

    if(dx < 0) { sx = b_startx + dx; dx = -dx; } else {   sx = b_startx; } 
    if(dy < 0) { sy = b_starty + dy; dy = -dy; } else {   sy = b_starty; } 

   if(gd.inten_levels == 0) { /* BIT PLANE MODE */
     XDrawRectangle(gd.dpy,gd.hcan_xid,gd.clear_ol_gc,sx,sy,dx,dy);
   } else {
      XDrawRectangle(gd.dpy,gd.hcan_xid,gd.ol_gc,sx,sy,dx,dy); /* XOR MODE */
   }
}

/*************************************************************************
 * REDRAW_ZOOM_BOX:  Draws the Zoom define box 
 *
 */

void
redraw_zoom_box()
{
    int    dx,dy;                /* pixel distances */
    int    sx,sy;

    dx = b_lastx - b_startx;
    dy = b_lasty - b_starty;

    if(dx < 0) { sx = b_startx + dx; dx = -dx; } else {   sx = b_startx; } 
    if(dy < 0) { sy = b_starty + dy; dy = -dy; } else {   sy = b_starty; } 

   if(gd.inten_levels == 0) { /* BIT PLANE MODE */
     XDrawRectangle(gd.dpy,gd.hcan_xid,gd.ol_gc,sx,sy,dx,dy);
   } else {
      XDrawRectangle(gd.dpy,gd.hcan_xid,gd.ol_gc,sx,sy,dx,dy); /* XOR MODE */
   }
}
 
/*************************************************************************
 * REDRAW_PAN_LINE: Redraw the Pan Define line 
 *
 */

void
redraw_pan_line()
{

/*    XDrawLine(gd.dpy,gd.hcan_xid,gd.clear_ol_gc,p_startx,p_starty,p_lastx,p_lasty); /* BIT PLANE MODE */
    XDrawLine(gd.dpy,gd.hcan_xid,gd.ol_gc,p_startx,p_starty,p_lastx,p_lasty); /* XOR MODE */
}
 
/*************************************************************************
 * REDRAW_RHI_LINE:  Draws the RHI define line 
 *
 */

void
redraw_rhi_line()
{
/*    XDrawLine(gd.dpy,gd.hcan_xid,gd.clear_ol_gc,r_startx,r_starty,r_lastx,r_lasty); /* BIT PLANE MODE */
    XDrawLine(gd.dpy,gd.hcan_xid,gd.ol_gc,r_startx,r_starty,r_lastx,r_lasty); /* XOR MODE */
}
 

/*************************************************************************
 * MANAGE_H_PIXMAPS: Manage the creation/ destruction of pixmaps for the
 *        horizontal display window 
 */

manage_h_pixmaps(mode)
    int    mode;     /* 1= create, 2 = replace, 3 = destroy */
{
    int    i,index;
     
    switch (mode)
    {
    case 1:        /* Create all pixmaps */
    case 2:        /* Replace Pixmaps */
        for(i=0; i < gd.num_datafields; i++)
        {
            if(gd.h_win.field_xid[i]) {
                XFreePixmap(gd.dpy,gd.h_win.field_xid[i]);
                gd.h_win.field_xid[i] = 0;
            }

            /* Create new field Pixmaps -f field updates automatically */
            if(gd.mrec[i]->background_render) {
                gd.h_win.field_xid[i] = XCreatePixmap(gd.dpy, 
                                                  gd.h_win.vis_xid, 
                                                  gd.h_win.can_dim.width,
                                                  gd.h_win.can_dim.height,
                                                  gd.h_win.can_dim.depth);
                gd.h_win.redraw[i] = 1;
                /* clear drawing area */
                XFillRectangle(gd.dpy,gd.h_win.field_xid[i],gd.extras.background_color->gc,
                           0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);
            }

        }
        /* create last stage pixmap */
        if(gd.h_win.can_xid) XFreePixmap(gd.dpy,gd.h_win.can_xid);
        gd.h_win.can_xid = XCreatePixmap(gd.dpy,
                                gd.h_win.vis_xid,
                                gd.h_win.can_dim.width,
                                gd.h_win.can_dim.height,
                                gd.h_win.can_dim.depth);
        XFillRectangle(gd.dpy,gd.h_win.can_xid,gd.extras.background_color->gc,
                           0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);

        /* create temporary area pixmaps */ 
        if(gd.h_win.tmp_xid) XFreePixmap(gd.dpy,gd.h_win.tmp_xid);
        gd.h_win.tmp_xid = XCreatePixmap(gd.dpy,
                                gd.h_win.vis_xid,
                                gd.h_win.can_dim.width,
                                gd.h_win.can_dim.height,
                                gd.h_win.can_dim.depth);
        XFillRectangle(gd.dpy,gd.h_win.tmp_xid,gd.extras.background_color->gc,
                           0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);


        /* release old movie frame pixmaps */
        for(i=0;i < MAX_FRAMES; i++ )
        {
            if(gd.movie.frame[i].h_xid)
            {
                XFreePixmap(gd.dpy,gd.movie.frame[i].h_xid); 
                gd.movie.frame[i].h_xid = 0;
            }
        }
        for(index=(gd.movie.first_index + gd.movie.start_frame),i=0;
            (i < gd.movie.num_pixmaps) && (i < gd.movie.num_frames);
            i++, index++)
        {

            if(index >= MAX_FRAMES) index -= MAX_FRAMES;
            gd.movie.frame[index].h_xid =
                XCreatePixmap(gd.dpy, 
                              gd.h_win.vis_xid, 
                              gd.h_win.can_dim.width,
                              gd.h_win.can_dim.height,
                              gd.h_win.can_dim.depth);
            gd.movie.frame[index].redraw_horiz = 1;
            /* clear drawing area */
            XFillRectangle(gd.dpy,gd.movie.frame[index].h_xid,gd.extras.background_color->gc,
                           0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);
        }

        break;

    case 3:
        for(i=0;i < MAX_FRAMES; i++ ) {
            if(gd.movie.frame[i].h_xid) 
                XFreePixmap(gd.dpy,gd.movie.frame[i].h_xid); 
                gd.movie.frame[i].h_xid = 0;
        }
        for(i=0; i < gd.num_datafields; i++) {
            if(gd.h_win.field_xid[i]) {
                XFreePixmap(gd.dpy,gd.h_win.field_xid[i]);
                gd.h_win.field_xid[i] = 0;
            }
        }
        if(gd.h_win.can_xid) {
            XFreePixmap(gd.dpy,gd.h_win.can_xid);
            gd.h_win.can_xid = 0;
        }
        if(gd.h_win.tmp_xid) {
            XFreePixmap(gd.dpy,gd.h_win.tmp_xid);
            gd.h_win.tmp_xid = 0;
        }
        break;
    }
}
