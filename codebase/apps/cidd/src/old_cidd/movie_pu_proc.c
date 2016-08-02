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
 * MOVIE_PU_PROC.C - Notify and event callback functions for movie controls
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#include "cidd.h"

#include "toolsa/str.h"
#include "toolsa/utim.h"


/*************************************************************************
 * Notify callback function for `start_st'.
 */
void
movie_start_proc(item, value, event)
    Panel_item    item;
    int        value;
    Event        *event;
{
    int    i;
    int    index;
    int    sl_value;
    Drawable    xid;
     
    if(value) {    /* Start command */
	xv_set(gd.movie_pu->start_st, PANEL_VALUE,1, NULL);
	xv_set(gd.h_win_horiz_bw->movie_start_st, PANEL_VALUE,1, NULL);
        if(gd.h_win.field != gd.h_win.movie_field) {
            reset_data_valid_flags(1,0);
            set_redraw_flags(1,0); /*  */
        }
        if(gd.v_win.field != gd.v_win.movie_field) {
            reset_data_valid_flags(0,1);
            set_redraw_flags(0,1); /*  */
        }
        gd.h_win.movie_field = gd.h_win.field;
        gd.v_win.movie_field = gd.v_win.field;
        gd.movie.movie_on = 1;
	sl_value = xv_get(gd.movie_pu->movie_speed_sl, PANEL_VALUE);  
	/* Calc delay in 25 msec increments above some minimum time */
	gd.movie.display_time_msec = (MOVIE_SPEED_RANGE - sl_value) * MOVIE_DELAY_INCR + MIN_MOVIE_TIME;

    } else {    /* Stop command */
	xv_set(gd.movie_pu->start_st, PANEL_VALUE,0, NULL);
	xv_set(gd.h_win_horiz_bw->movie_start_st, PANEL_VALUE,0, NULL);
        if(gd.movie.cur_frame < 0 || gd.limited_mode ) gd.movie.cur_frame = gd.movie.num_frames -1;
        index = gd.movie.first_index + gd.movie.cur_frame;
        if(index >= MAX_FRAMES) index -= MAX_FRAMES;

        if(gd.mrec[gd.h_win.field]->background_render) {
            xid = gd.h_win.field_xid[gd.h_win.field];
        } else {
            xid = gd.h_win.tmp_xid;
        }
       if (gd.debug2) fprintf(stderr,"Movie_stop: Copying Horiz movie image index %d xid: %d to xid: %d\n",
		         	index,gd.movie.frame[index].h_xid,xid);
    
        XCopyArea(gd.dpy,gd.movie.frame[index].h_xid,
                xid,
                gd.def_gc,  0,0,
                gd.h_win.can_dim.width,
                gd.h_win.can_dim.height,
                gd.h_win.can_dim.x_pos,
                gd.h_win.can_dim.y_pos);


        if(gd.mrec[gd.v_win.field]->background_render) {
            xid = gd.v_win.field_xid[gd.v_win.field];
        } else {
            xid = gd.v_win.tmp_xid;
        }
       if (gd.debug2) fprintf(stderr,"Movie_stop: Copying Vert movie image index %d xid: %d to xid: %d\n",
		         	index,gd.movie.frame[index].v_xid,xid);
    
        XCopyArea(gd.dpy,gd.movie.frame[index].v_xid,
                xid,
                gd.def_gc,  0,0,
                gd.v_win.can_dim.width,
                gd.v_win.can_dim.height,
                gd.v_win.can_dim.x_pos,
                gd.v_win.can_dim.y_pos);

        /* Indicate all other fields need rendered */
        for(i=0; i < gd.num_datafields; i++) {
            gd.h_win.redraw[i] = 1;
            gd.v_win.redraw[i] = 1;
        }
        gd.h_win.redraw[gd.h_win.field] = 0;
        gd.v_win.redraw[gd.v_win.field] = 0;
        gd.movie.movie_on = 0;
    }
    if(gd.extras.wind_mode && gd.extras.wind_vectors) {
	 gd.movie.frame[gd.movie.cur_index].redraw_horiz = 1;
    }
	    
    start_timer();      /* Xview sometimes "forgets the interval timer" -restart it just in case */
}

/*************************************************************************
 * Notify callback function for `movie_type_st'.
 */
void
movie_type_proc(item, value, event)
    Panel_item    item;
    int        value;
    Event        *event;
{
    int    index;
    long    clock;
    static long    last_time;

    UTIMstruct  temp_utime;
     
    gd.movie.mode = value;
     
    switch(value) {
        case MOVIE_MR: /* Most recent */

            gd.movie.num_frames = gd.movie.num_pixmaps;
            gd.movie.cur_frame = gd.movie.num_pixmaps -1;
            gd.movie.start_frame = 0;
            gd.movie.end_frame = gd.movie.num_frames -1;

            clock = time(0);
            clock += (gd.movie.forecast_interval * 60.0);
            
            last_time = clock - ((gd.movie.num_frames -1) * gd.movie.time_interval * 60.0);
	    last_time -= (last_time % gd.movie.round_to_seconds);

            UTIMunix_to_date(last_time,&temp_utime);

            gd.movie.start_time.year = temp_utime.year;
            gd.movie.start_time.month = temp_utime.month;
            gd.movie.start_time.day = temp_utime.day;
            gd.movie.start_time.hour = temp_utime.hour;
            gd.movie.start_time.min = temp_utime.min;
            gd.movie.start_time.sec = temp_utime.sec;
            gd.movie.start_time.unix_time = last_time;
            
        break;

        case MOVIE_TS:    /* Time Span */
            if(last_time == 0) {
                last_time = gd.movie.start_time.unix_time;
            }
            /* Restore the original start_time */
            gd.movie.start_time.unix_time = last_time;
            UTIMunix_to_date(last_time, &temp_utime);

            gd.movie.start_time.year = temp_utime.year;
            gd.movie.start_time.month = temp_utime.month;
            gd.movie.start_time.day = temp_utime.day;
            gd.movie.start_time.hour = temp_utime.hour;
            gd.movie.start_time.min = temp_utime.min;
            gd.movie.start_time.sec = temp_utime.sec;
            gd.movie.start_time.unix_time = last_time;
            
            gd.movie.num_frames = gd.movie.num_pixmaps;
            gd.movie.cur_frame = 0;
            gd.movie.start_frame = 0;
            gd.movie.end_frame = gd.movie.num_frames -1;
        break;

        case MOVIE_EL: /* Elevation */
            last_time = gd.movie.start_time.unix_time;    /* Save the Starting time of previous mode */
            if(gd.movie.cur_frame < 0) {
                index = gd.movie.first_index + gd.movie.num_frames - 1;
            } else {
                index = gd.movie.first_index + gd.movie.cur_frame;
            }
            if(index >= MAX_FRAMES) index -= MAX_FRAMES;

            gd.movie.start_time.unix_time = gd.movie.frame[index].time_mid.unix_time;
            UTIMunix_to_date(gd.movie.start_time.unix_time,
                             &temp_utime);

            gd.movie.start_time.year = temp_utime.year;
            gd.movie.start_time.month = temp_utime.month;
            gd.movie.start_time.day = temp_utime.day;
            gd.movie.start_time.hour = temp_utime.hour;
            gd.movie.start_time.min = temp_utime.min;
            gd.movie.start_time.sec = temp_utime.sec;

            gd.movie.cur_frame = 0;
            gd.movie.num_frames = (gd.h_win.max_ht - gd.h_win.min_ht) / gd.h_win.delta;
            gd.movie.start_frame = 0;
            gd.movie.end_frame = gd.movie.num_frames -1;
        break;
    }

    reset_time_points();
     
    update_movie_popup();

    set_redraw_flags(1,1);

    reset_data_valid_flags(1,1);
}

/*************************************************************************
 * Notify callback function for `movie_frame_sl'.
 */
void
movie_frame_proc(item, value, event)
    Panel_item    item;
    int        value;
    Event        *event;
{
    long start_time, end_time;
    
    gd.movie.cur_frame = value - 1;
    reset_data_valid_flags(1,1);

    /* get the appropriate products from the product selector */
    start_time = gd.movie.frame[gd.movie.cur_frame].time_start.unix_time;
    end_time = gd.movie.frame[gd.movie.cur_frame].time_end.unix_time;
    
    CSPR_calc_and_set_time();
    
    if ((time(0) - start_time) <
        (gd.movie.time_interval * 60.0 * gd.movie.mr_stretch_factor))
        set_import_times(0, 0);
    else
        set_import_times(start_time, end_time);
     
}

/*************************************************************************
 * Notify callback function for `movie_speed_sl'.
 */
void
movie_speed_proc(item, value, event)
    Panel_item    item;
    int        value;
    Event        *event;
{
    xv_set(gd.h_win_horiz_bw->movie_spd_sl,PANEL_VALUE,value,NULL);
    xv_set(gd.movie_pu->movie_speed_sl,PANEL_VALUE,value,NULL);
     
    /* Calc delay in 25 msec increments above some minimum time */
    gd.movie.display_time_msec = (MOVIE_SPEED_RANGE - value) * MOVIE_DELAY_INCR + MIN_MOVIE_TIME;
}

/*************************************************************************
 * Notify callback function for `movie_dwell_sl'.
 */
void
movie_delay_proc(item, value, event)
    Panel_item    item;
    int        value;
    Event        *event;
{
    gd.movie.delay = value * 1000;
}

/*************************************************************************
 * Notify callback function for `start_time_tx'.
 */
Panel_setting
start_time_proc(item, event)
    Panel_item    item;
    Event        *event;
{
    long    utime;
    long    last_time;
    long    start_time;
    long    end_time;

    UTIMstruct    temp_utime;
    
    char *    value = (char *) xv_get(item, PANEL_VALUE);
    
    last_time = gd.movie.start_time.unix_time;
    parse_string_into_time(value,&temp_utime);

    utime = UTIMdate_to_unix(&temp_utime);
     
    utime -= (utime % gd.movie.round_to_seconds);

    UTIMunix_to_date(utime,&temp_utime);

    gd.movie.start_time.year = temp_utime.year;
    gd.movie.start_time.month = temp_utime.month;
    gd.movie.start_time.day = temp_utime.day;
    gd.movie.start_time.hour = temp_utime.hour;
    gd.movie.start_time.min = temp_utime.min;
    gd.movie.start_time.sec = temp_utime.sec;
    gd.movie.start_time.unix_time = utime;

    /* if starting time moves more than 2 volume intervals, assume we want a time series */
    if(abs(last_time - utime) > (2 * gd.movie.time_interval * 60)) {
        gd.movie.mode = MOVIE_TS;
        gd.movie.num_frames = gd.movie.num_pixmaps;
        gd.movie.cur_frame = 0;
        gd.movie.start_frame = 0;
        gd.movie.end_frame = gd.movie.num_frames -1;
        xv_set(gd.movie_pu->movie_type_st,PANEL_VALUE,MOVIE_TS,NULL);
    }


    switch(gd.movie.mode) {
        case MOVIE_MR: /* calc number of frames it will take */
            utime = time(0);
            utime += (gd.movie.forecast_interval * 60.0);
            
            gd.movie.num_frames = (utime - gd.movie.start_time.unix_time) /
            (gd.movie.time_interval * 60);
            if(gd.movie.num_frames >= MAX_FRAMES) 
                gd.movie.num_frames = MAX_FRAMES;
        break;

        case MOVIE_TS: 
        break;

        case MOVIE_EL:
        break;

    }

    reset_time_points();

    reset_data_valid_flags(1,1);

    /* get the appropriate products from the product selector */
    start_time = gd.movie.frame[gd.movie.cur_frame].time_start.unix_time;
    end_time = gd.movie.frame[gd.movie.cur_frame].time_end.unix_time;
    
    if ((time(0) - start_time) <
        (gd.movie.time_interval * 60.0 * gd.movie.mr_stretch_factor))
        set_import_times(0, 0);
    else
        set_import_times(start_time, end_time);
     
    update_movie_popup();

    set_redraw_flags(1,1);
     
    return panel_text_notify(item, event);
}
 
/*************************************************************************
 * Notify callback function for `st_frame_tx'.
 */
Panel_setting
start_frame_proc(item, event)
    Panel_item    item;
    Event        *event;
{
    int    value = (int) xv_get(item, PANEL_VALUE);

    gd.movie.start_frame = value -1;
    if(gd.movie.end_frame < gd.movie.start_frame) {
        gd.movie.end_frame = gd.movie.start_frame;
        xv_set(gd.movie_pu->end_frame_tx,PANEL_VALUE,gd.movie.end_frame,NULL);
    }
    gd.movie.cur_frame = gd.movie.start_frame;
    gd.movie.num_frames = gd.movie.end_frame - gd.movie.start_frame + 1;

    reset_time_points();
     
    update_movie_popup();

    adjust_pixmap_range();

    return panel_text_notify(item, event);
}
 
/*************************************************************************
 * Notify callback function for `end_frame_tx'.
 */
Panel_setting
end_frame_proc(item, event)
    Panel_item    item;
    Event        *event;
{
    int    value = (int) xv_get(item, PANEL_VALUE);

    gd.movie.end_frame = value -1;
    if(gd.movie.end_frame < gd.movie.start_frame) {
        gd.movie.start_frame = gd.movie.end_frame;
        xv_set(gd.movie_pu->st_frame_tx,PANEL_VALUE,gd.movie.start_frame,NULL);
    }
    gd.movie.num_frames = gd.movie.end_frame - gd.movie.start_frame + 1;
    gd.movie.cur_frame = gd.movie.end_frame;

    reset_time_points();
     
    update_movie_popup();

    adjust_pixmap_range();
     
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `time_interval_tx'.
 */
Panel_setting
time_interv_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);
    long    utime;

    gd.movie.time_interval = atof(value);

    switch(gd.movie.mode) {
        case MOVIE_MR: /* calc number of frames it will take */
            utime = time(0);
            gd.movie.start_time.unix_time = (utime - (gd.movie.time_interval * 60 * gd.movie.num_frames));
        break;

        case MOVIE_TS: 
        break;

        case MOVIE_EL:
        break;

    }

    reset_time_points();    /* Fill in the time values for each frame */

    reset_data_valid_flags(1,1);

    update_movie_popup();

    set_redraw_flags(1,1);
    
    return panel_text_notify(item, event);
}

/*************************************************************************
 * PARSE_STRING_INTO_TIME:   Parse string to and time structure 
 *
 */

parse_string_into_time(string,time)
    char    *string;
    UTIMstruct    *time;    
{
    int    num_fields;
    double  field[16];


    num_fields = STRparse_double(string, field, 40, 16);

    switch(num_fields) {
        case 6:     /* hour:min:sec month/day/Year */
             
            time->hour = (int)(field[0]) % 24;
            time->min = (int)(field[1]) % 60;
            time->sec = (int)(field[2]) % 60;

            time->month = (int)(field[3]) % 13;
            time->day = (int)(field[4]) % 32;

            if(field[5] < 50 ) field[5] += 2000;
            if(field[5] < 1900) field[5] += 1900;
            time->year = (int)(field[5]);
        break;

        case 5:     /* hour:min month/day/year */
             
            time->hour = (int)(field[0]) % 24;
            time->min = (int)(field[1]) % 60;
            time->sec = 0;

            time->month = (int)(field[2]) % 13;
            time->day = (int)(field[3]) % 32;
            if(field[4] < 50 ) field[4] += 2000;
            if(field[4] < 1900) field[4] += 1900;
            time->year = (int)(field[4]);
        break;

        case 4:     /* hour:min month day */
             
            time->hour = (int)(field[0]) % 24;
            time->min = (int)(field[1]) % 60;
            time->month = (int)(field[2]) % 13;
            time->day = (int)(field[3]) % 32;
        break;

        case 2:     /* min:sec */
            time->min = (int)(field[0]) % 60;
            time->sec = (int)(field[1]) % 60;
        break;

        case 1:     /* min */
            time->min = (int)(field[0]) % 60;
        break;
    }
}

/*************************************************************************
 * UPDATE_MOVIE_PU: Update critical displayed values on the movie popup
 */

update_movie_popup()
{
    int        index;
    char    text[32];

    if(gd.movie.cur_frame < 0) {
        index = gd.movie.first_index +  gd.movie.num_frames - 1;
    } else {
        index = gd.movie.cur_frame + gd.movie.first_index;
    }
    if(index >= MAX_FRAMES) index -= MAX_FRAMES;
 
    /* Update the Current Frame Begin Time text */
    sprintf(text,"Frame %d: %d:%02d %d/%d/%d",
	gd.movie.cur_frame +1 ,
        gd.movie.frame[index].time_mid.hour,
        gd.movie.frame[index].time_mid.min,
        gd.movie.frame[index].time_mid.month,
        gd.movie.frame[index].time_mid.day,
        gd.movie.frame[index].time_mid.year);
         
    xv_set(gd.movie_pu->fr_begin_msg,PANEL_LABEL_STRING,text,NULL);

    /* Update the start Time text */
    sprintf(text,"%d:%02d %d/%d/%d",
        gd.movie.start_time.hour,
        gd.movie.start_time.min,
        gd.movie.start_time.month,
        gd.movie.start_time.day,
        gd.movie.start_time.year);
    xv_set(gd.movie_pu->start_time_tx,PANEL_VALUE,text,NULL);

    if(gd.debug1) printf("Time Start: %d, End: %d\n",gd.movie.frame[index].time_start.unix_time,
                gd.movie.frame[index].time_end.unix_time);

    /* update the time_interval  text */
    sprintf(text,"%.2f",gd.movie.time_interval);
    xv_set(gd.movie_pu->time_interval_tx,PANEL_VALUE,text,NULL);

    /* update the forecast period text */
    sprintf(text,"%.2f",gd.movie.forecast_interval);
    xv_set(gd.movie_pu->fcast_per_tx,PANEL_VALUE,text,NULL);

    xv_set(gd.movie_pu->movie_frame_sl,
        PANEL_MIN_VALUE,gd.movie.start_frame + 1,
        PANEL_MAX_VALUE,gd.movie.end_frame + 1,
        PANEL_VALUE,gd.movie.cur_frame +1,
        NULL);

    /* update the start/end frames text */
    xv_set(gd.movie_pu->st_frame_tx,PANEL_VALUE,gd.movie.start_frame +1,
        PANEL_MAX_VALUE,MAX_FRAMES,NULL);
    xv_set(gd.movie_pu->end_frame_tx,PANEL_VALUE,gd.movie.end_frame +1,
        PANEL_MAX_VALUE,MAX_FRAMES,NULL);
}

/*****************************************************************************
 * ADJUST_PIXMAP_RANGE: Put the pixmaps at the first N Visible frames
 */

adjust_pixmap_range()
{
    int    i,index,num_pmaps;

    /* First Remove any pixmaps that are not in the current movie loop */
    for(i=0; i < MAX_FRAMES; i++ ) {
        if(i < (gd.movie.first_index + gd.movie.start_frame) ||
            i > (gd.movie.first_index + gd.movie.end_frame)) {

            if(gd.movie.frame[i].h_xid) {
                XFreePixmap(gd.dpy,gd.movie.frame[i].h_xid);
                gd.movie.frame[i].h_xid = 0;
                gd.movie.frame[i].redraw_horiz = 1;
            }

            if(gd.movie.frame[i].v_xid) {
                XFreePixmap(gd.dpy,gd.movie.frame[i].v_xid);
                gd.movie.frame[i].v_xid = 0;
                gd.movie.frame[i].redraw_vert = 1;
            }
        }
    }

    /* Now make sure the chosen frames have pixmaps */
    num_pmaps = 0;
    for(i = 0, index=(gd.movie.first_index + gd.movie.start_frame);
        i < gd.movie.num_frames; index++,i++ ) {

        if(index >= MAX_FRAMES) index -= MAX_FRAMES;

        if(num_pmaps < gd.movie.num_pixmaps) {
            if(gd.movie.frame[index].h_xid == 0) {
                gd.movie.frame[index].h_xid = XCreatePixmap(gd.dpy,
                    gd.h_win.can_xid,
                    gd.h_win.can_dim.width,
                    gd.h_win.can_dim.height,
                    gd.h_win.can_dim.depth);
                gd.movie.frame[index].redraw_horiz = 1;
            } 

            if(gd.movie.frame[index].v_xid == 0) {
                gd.movie.frame[index].v_xid = XCreatePixmap(gd.dpy,
                    gd.v_win.can_xid,
                    gd.v_win.can_dim.width,
                    gd.v_win.can_dim.height,
                    gd.v_win.can_dim.depth);
                gd.movie.frame[index].redraw_vert = 1;
            }
        } else {
            if(gd.movie.frame[i].h_xid) {
                XFreePixmap(gd.dpy,gd.movie.frame[i].h_xid);
                gd.movie.frame[i].h_xid = 0;
            }

            if(gd.movie.frame[i].v_xid) {
                XFreePixmap(gd.dpy,gd.movie.frame[i].v_xid);
                gd.movie.frame[i].v_xid = 0;
            }
        }

        num_pmaps++;    /* count the active pixmap sets */
    }
}

/*************************************************************************
 * Notify callback function for `fcast_per_tx'.
 */
Panel_setting
set_fcast_period_proc(item, event)
        Panel_item      item;
        Event           *event;
{
    static long    last_time;
    long    clock;
    UTIMstruct  temp_utime;
    char *  value = (char *) xv_get(item, PANEL_VALUE);
     
     
    gd.movie.forecast_interval = atof(value);
     
    switch(gd.movie.mode) {
        case MOVIE_MR: /* Most recent */

            clock = time(0);
            clock += (gd.movie.forecast_interval * 60.0);
            
            last_time = clock - ((gd.movie.num_frames -1) * gd.movie.time_interval * 60.0);
	    last_time -= (last_time % gd.movie.round_to_seconds);

            UTIMunix_to_date(last_time,&temp_utime);

            gd.movie.start_time.year = temp_utime.year;
            gd.movie.start_time.month = temp_utime.month;
            gd.movie.start_time.day = temp_utime.day;
            gd.movie.start_time.hour = temp_utime.hour;
            gd.movie.start_time.min = temp_utime.min;
            gd.movie.start_time.sec = temp_utime.sec;
            gd.movie.start_time.unix_time = last_time;
            
            reset_time_points();
     
            update_movie_popup();

            set_redraw_flags(1,1);

            reset_data_valid_flags(1,1);
        break;

    }
    return panel_text_notify(item, event);
}

#ifndef LINT
static char RCS_id[] = "$Id: movie_pu_proc.c,v 1.12 2016/03/07 18:28:26 dixon Exp $";
static char SCCS_id[] = "%W% %D% %T%";
#endif /* not LINT */

