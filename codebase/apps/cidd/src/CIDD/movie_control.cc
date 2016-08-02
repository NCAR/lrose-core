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
 * MOVIE_CONTROL.C:
 *
 * For the Configurable Interactive Data Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#define MOVIE_CONTROL 1

#include "cidd.h"

/*************************************************************************
 * TIME_FOR_A_NEW_FRAME: Decide if it is time to create a new movie frame
 *    in the Most Recent movie mode
 */

int time_for_a_new_frame()
{
    time_t c_time = time(0);

	// No forward shifting allowed in run_once and exit mode
	// Delay while shifting products 
	if( gd.run_once_and_exit || gd.r_context->offset_x != 0 ||  gd.r_context->offset_y != 0) {
		 return 0;
	}

    if(gd.forecast_mode) c_time += (time_t) (gd.movie.forecast_interval * 60.0);

    if(c_time > (gd.movie.start_time + (gd.movie.num_frames  * gd.movie.time_interval * 60))) {
        return 1;
    }
    
    return 0;
}

/************************************************************************
 * RESET_TIME_POINTS: Calc the beginning and ending time points for
 *        each movie frame
 */

void reset_time_points()
{
    int    i;

    if(gd.debug2) {
        printf("Resetting time points - mode: %d\n",gd.movie.mode);
    }

    switch(gd.movie.climo_mode) {
    case REGULAR_INTERVAL:
      for (i = 0; i < gd.movie.num_frames; i++)
      {
	double time_interval_secs = gd.movie.time_interval * 60.0;
	
	gd.movie.frame[i].time_start =
	  (time_t)(gd.movie.start_time + (i * time_interval_secs) + 0.5);

	gd.movie.frame[i].time_mid =
	  (time_t)(gd.movie.start_time + ((i + 0.5) * time_interval_secs) + 0.5);

	gd.movie.frame[i].time_end =
	  (time_t)(gd.movie.frame[i].time_start + time_interval_secs + 0.5);

      }
    break;

    case DAILY_INTERVAL:
        for (i=0; i < gd.movie.num_frames; i++) {
	    gd.movie.frame[i].time_start = (time_t)
	        (gd.movie.start_time + (i * 1440 * 60.0));

            gd.movie.frame[i].time_mid = (time_t)
                (gd.movie.start_time + (gd.movie.frame_span * 30.0));

            gd.movie.frame[i].time_end =  (time_t)
	        (gd.movie.frame[i].time_start + (gd.movie.frame_span * 60.0));

        }
    break;

    case YEARLY_INTERVAL:
        time_t    ft;
        UTIMstruct t;
	UTIMunix_to_date(gd.movie.start_time,&t);
        for (i=0; i < gd.movie.num_frames; i++, t.year++) {
            ft = UTIMdate_to_unix(&t);
	    gd.movie.frame[i].time_start = (time_t) ft;

            gd.movie.frame[i].time_mid = (time_t)
                (gd.movie.frame[i].time_start + (gd.movie.frame_span * 30.0));

            gd.movie.frame[i].time_end =  (time_t)
	        (gd.movie.frame[i].time_start + (gd.movie.frame_span * 60.0));

        }
    break;
    }
    reset_time_list_valid_flags();

    if (gd.prod_mgr) gd.prod_mgr->reset_times_valid_flags();

    gd.epoch_start = (time_t) gd.movie.start_time;
    gd.epoch_end =  (time_t) (gd.movie.frame[gd.movie.num_frames -1].time_end);

    if(gd.coord_expt != NULL) {
        gd.coord_expt->epoch_start = gd.epoch_start;
        gd.coord_expt->epoch_end = gd.epoch_end;
        gd.coord_expt->click_type = CIDD_OTHER_CLICK;
        gd.coord_expt->pointer_seq_num++;
        gd.coord_expt->time_min = gd.movie.frame[gd.movie.cur_frame].time_start;
        gd.coord_expt->time_max = gd.movie.frame[gd.movie.cur_frame].time_end;
        if(gd.movie.movie_on) { 
	    gd.coord_expt->time_cent = gd.coord_expt->epoch_end;
        } else {
            gd.coord_expt->time_cent = gd.coord_expt->time_min +
              ((gd.coord_expt->time_max - gd.coord_expt->time_min)/2);
        }
    }
}

/************************************************************************
 * ROTATE_MOVIE_FRAMES: Rotate the data in the  movie frames down one-
 * Move the oldest data to the newest slot
 *
 */

void rotate_movie_frames()
{
    int i;
    movie_frame_t tmp_frame;

    /* increment start time */
    gd.movie.start_time += (time_t)  (gd.movie.time_interval * 60);

    /* Copy the first frame data  */
    memcpy(&tmp_frame,&gd.movie.frame[0],sizeof(movie_frame_t));

    // Shift each frame down one.
    for(i = 0; i < gd.movie.num_frames - 1; i++) {
	memcpy(&gd.movie.frame[i],&gd.movie.frame[i+1],sizeof(movie_frame_t));

        // Render a new time selector for this frame
        draw_hwin_bot_margin(gd.movie.frame[i].h_xid,gd.h_win.page,
                             gd.movie.frame[i].time_start,
                             gd.movie.frame[i].time_end);
    }

    // Copy the first frame data to the last
    memcpy(&gd.movie.frame[gd.movie.num_frames - 1],&tmp_frame,sizeof(movie_frame_t));

    gd.movie.frame[gd.movie.num_frames - 1].redraw_horiz = 1;
    gd.movie.frame[gd.movie.num_frames - 1].redraw_vert = 1;

    reset_time_points();

    if(! gd.run_unmapped) update_movie_popup();

    if (gd.movie.reset_frames) {
        set_redraw_flags(1, 1);
        reset_data_valid_flags(1, 1);
	if (gd.prod_mgr) {
	  gd.prod_mgr->reset_product_valid_flags();
	  gd.prod_mgr->reset_times_valid_flags();
	}
    }

    /* restarts the sequence of rendering each field at each zoom state */
    if(gd.html_mode) {
        set_domain_proc(gd.zoom_pu->domain_st,0,NULL); /* reset domain to 1st area */
    }

}

/*****************************************************************************
 * ADJUST_PIXMAP_RANGE: Put the pixmaps at the first N Visible frames
 */

void adjust_pixmap_allocation()
{
    int    i;

    /* First Remove any pixmaps that are not in the current movie loop */
    for(i=gd.movie.num_frames ; i < MAX_FRAMES; i++ ) {
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

    /* Now make sure the movie frames all have pixmaps */
    for(i = 0; i < gd.movie.num_frames;i++ ) {
        if(gd.movie.frame[i].h_xid == 0) {
            gd.movie.frame[i].h_xid = XCreatePixmap(gd.dpy,
                gd.h_win.can_xid[gd.h_win.cur_cache_im],
                gd.h_win.can_dim.width,
                gd.h_win.can_dim.height,
                gd.h_win.can_dim.depth);
            gd.movie.frame[i].redraw_horiz = 1;
        }

        if(gd.movie.frame[i].v_xid == 0) {
            gd.movie.frame[i].v_xid = XCreatePixmap(gd.dpy,
                gd.v_win.can_xid[gd.v_win.cur_cache_im],
                gd.v_win.can_dim.width,
                gd.v_win.can_dim.height,
                gd.v_win.can_dim.depth);
            gd.movie.frame[i].redraw_vert = 1;
        }
    }
}

/*************************************************************************
 * PARSE_STRING_INTO_TIME:   Parse string to and time structure
 *
 */
void parse_string_into_time( const char *string, UTIMstruct *tms)
{
    int    num_fields;
    double  field[16];
	time_t now;
	double tdiff;

	now = time(0);
	// Establish the time difference between local and UTC
	tdiff = difftime(mktime(localtime(&now)),mktime(gmtime(&now)));

    num_fields = STRparse_double(string, field, 40, 16);

    switch(num_fields) {
        case 6:     /* hour:min:sec month/day/Year */

            tms->hour = (int)(field[0]) % 24;
            tms->min = (int)(field[1]) % 60;
            tms->sec = (int)(field[2]) % 60;

            tms->month = (int)(field[3]) % 13;
            tms->day = (int)(field[4]) % 32;

            if(field[5] < 50 ) field[5] += 2000;
            if(field[5] < 1900) field[5] += 1900;
            tms->year = (int)(field[5]);
        break;

        case 5:     /* hour:min month/day/year */

            tms->hour = (int)(field[0]) % 24;
            tms->min = (int)(field[1]) % 60;
            tms->sec = 0;

            tms->month = (int)(field[2]) % 13;
            tms->day = (int)(field[3]) % 32;
            if(field[4] < 50 ) field[4] += 2000;
            if(field[4] < 1900) field[4] += 1900;
            tms->year = (int)(field[4]);
        break;

        case 4:     /* hour:min month day */

            tms->hour = (int)(field[0]) % 24;
            tms->min = (int)(field[1]) % 60;
            tms->month = (int)(field[2]) % 13;
            tms->day = (int)(field[3]) % 32;
        break;

        case 2:     /* min:sec */
            tms->min = (int)(field[0]) % 60;
            tms->sec = (int)(field[1]) % 60;
        break;

        case 1:     /* min */
            tms->min = (int)(field[0]) % 60;
        break;
    }

	// Compensate for the user entering Local time
	if(gd.use_local_timestamps) {
	    now = (int) (UTIMdate_to_unix(tms) -  tdiff);
	    UTIMunix_to_date(now,tms);
	}

}
