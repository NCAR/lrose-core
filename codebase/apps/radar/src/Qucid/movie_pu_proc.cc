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
 * MOVIE_PU_PROC.C - Notify and event callback functions for movie controls
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define MOVIE_PU_PROC

#include "cidd.h"

void set_display_time(time_t utime);
void set_end_frame(int num_frames);

/*************************************************************************
 * UPDATE_MOVIE_PU: Update critical displayed values on the movie popup
 */

void update_movie_popup()
{
  int        index;
  char    text[64];
  char    fmt_text[128];
  struct tm tms;

  if(gd.movie.cur_frame < 0) {
    index =  gd.movie.num_frames - 1;
  } else {
    index = gd.movie.cur_frame;
  }
 
  /* Update the Current Frame Begin Time text */
  sprintf(fmt_text,"Frame %d: %%H:%%M %%m/%%d/%%Y",index+1);
  if(_params.use_local_timestamps) {
    strftime (text,64,fmt_text,localtime_r(&gd.movie.frame[index].time_mid,&tms));
  } else {
    strftime (text,64,fmt_text,gmtime_r(&gd.movie.frame[index].time_mid,&tms));
  }
  // xv_set(gd.movie_pu->fr_begin_msg,PANEL_LABEL_STRING,text,NULL);
  
  // Update the movie time start text
  if(_params.use_local_timestamps) {
    strftime (text, 64, _params.moviestart_time_format,
              localtime_r(&gd.movie.start_time,&tms));
  } else {
    strftime (text, 64, _params.moviestart_time_format,
              gmtime_r(&gd.movie.start_time,&tms));
  }
  // xv_set(gd.movie_pu->start_time_tx,PANEL_VALUE,text,NULL);

  // xv_set(gd.movie_pu->movie_type_st,PANEL_VALUE,gd.movie.mode,NULL);

  if(gd.debug1) printf("Time Start: %ld, End: %ld\n",gd.movie.frame[index].time_start,
                       gd.movie.frame[index].time_end);
   
  /* update the time_interval  text */
  switch(gd.movie.climo_mode) {
    case REGULAR_INTERVAL:
      sprintf(text,"%.2f",gd.movie.time_interval);
      // xv_set(gd.movie_pu->time_interval_tx,PANEL_VALUE,text,NULL);
      // xv_set(gd.movie_pu->min_msg,PANEL_LABEL_STRING,"min",NULL);
      break;

    case DAILY_INTERVAL:
      // xv_set(gd.movie_pu->time_interval_tx,PANEL_VALUE,"1",NULL);
      // xv_set(gd.movie_pu->min_msg,PANEL_LABEL_STRING,"day",NULL);
      break;

    case YEARLY_INTERVAL:
      // xv_set(gd.movie_pu->time_interval_tx,PANEL_VALUE,"1",NULL);
      // xv_set(gd.movie_pu->min_msg,PANEL_LABEL_STRING,"yr",NULL);
      break;

  }

  /* update the forecast period text */
  sprintf(text,"%.2f",gd.movie.forecast_interval);
  // xv_set(gd.movie_pu->fcast_per_tx,PANEL_VALUE,text,NULL);

  // xv_set(gd.movie_pu->movie_frame_sl,
  //        PANEL_MIN_VALUE,gd.movie.start_frame + 1,
  //        PANEL_MAX_VALUE,gd.movie.end_frame + 1,
  //        PANEL_VALUE,gd.movie.cur_frame +1,
  //        NULL);

  /* update the start/end frames text */
  sprintf(text,"%d",gd.movie.end_frame +1);
  // xv_set(gd.movie_pu->end_frame_tx,PANEL_VALUE,text,NULL);

  if (gd.prod_mgr) {
    gd.prod_mgr->reset_times_valid_flags();
  }

  if(gd.time_plot)
  {
    gd.time_plot->Set_times((time_t) gd.epoch_start,
                            (time_t) gd.epoch_end,
                            (time_t) gd.movie.frame[gd.movie.cur_frame].time_start,
                            (time_t) gd.movie.frame[gd.movie.cur_frame].time_end,
                            (time_t)((gd.movie.time_interval * 60.0) + 0.5),
                            gd.movie.num_frames);
    gd.time_plot->Draw(); 
  }
    

}

/*************************************************************************
 * MOVIE_START
 */

void movie_start( u_int value)
{
  int    i;
  int    sl_value;
  Drawable    xid;


  if(value) {    /* Start command */
    // Make sure the main loop on/off button is correct
    // int new_value = gd.menu_bar.last_callback_value | gd.menu_bar.loop_onoff_bit;
    // gd.menu_bar.last_callback_value = new_value;
    // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE, new_value, NULL);

    // Make sure the Movie popup's value is correct
    // xv_set(gd.movie_pu->start_st, PANEL_VALUE,1, NULL);

    if(gd.h_win.page != gd.h_win.movie_page) {
      reset_data_valid_flags(1,0);
      set_redraw_flags(1,0); /*  */
    }
    if(gd.v_win.page != gd.v_win.movie_page) {
      reset_data_valid_flags(0,1);
      set_redraw_flags(0,1); /*  */
    }
    gd.h_win.movie_page = gd.h_win.page;
    gd.v_win.movie_page = gd.v_win.page;
    gd.movie.movie_on = 1;
    // sl_value = xv_get(gd.movie_pu->movie_speed_sl, PANEL_VALUE);
    sl_value = 0;
    /* Calc delay in 25 msec increments above some minimum time */
    gd.movie.display_time_msec = (MOVIE_SPEED_RANGE - sl_value) * MOVIE_DELAY_INCR + MIN_MOVIE_TIME;

  } else {    /* Stop command */
    // Make sure the main loop on/off button is correct
    // int new_value = gd.menu_bar.last_callback_value & ~(gd.menu_bar.loop_onoff_bit);
    // gd.menu_bar.last_callback_value = new_value;
    // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE, new_value, NULL);

    // Make sure the Movie popup's value is correct
    // xv_set(gd.movie_pu->start_st, PANEL_VALUE,0, NULL);

    // Limited Mode always stops on the end frame
    if(gd.movie.cur_frame < 0 || _params.wsddm_mode ) gd.movie.cur_frame = gd.movie.num_frames -1;
    if(gd.mrec[gd.h_win.page]->auto_render) {
      xid = gd.h_win.page_xid[gd.h_win.page];
    } else {
      xid = gd.h_win.tmp_xid;
    }
    if (gd.debug2) fprintf(stderr,"Movie_stop: Copying Horiz movie image frame %d xid: %ld to xid: %ld\n",
                           gd.movie.cur_frame,gd.movie.frame[gd.movie.cur_frame].h_xid,xid);
    
    XCopyArea(gd.dpy,gd.movie.frame[gd.movie.cur_frame].h_xid,
              xid,
              gd.def_gc,  0,0,
              gd.h_win.can_dim.width,
              gd.h_win.can_dim.height,
              gd.h_win.can_dim.x_pos,
              gd.h_win.can_dim.y_pos);


    if(gd.mrec[gd.v_win.page]->auto_render) {
      xid = gd.v_win.page_xid[gd.v_win.page];
    } else {
      xid = gd.v_win.tmp_xid;
    }
    if (gd.debug2) fprintf(stderr,"Movie_stop: Copying Vert movie image frame %d xid: %ld to xid: %ld\n",
                           gd.movie.cur_frame,gd.movie.frame[gd.movie.cur_frame].v_xid,xid);
    
    XCopyArea(gd.dpy,gd.movie.frame[gd.movie.cur_frame].v_xid,
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
    gd.h_win.redraw[gd.h_win.page] = 0;
    gd.v_win.redraw[gd.v_win.page] = 0;
    gd.movie.movie_on = 0;
  }
  if(gd.layers.wind_mode && gd.layers.wind_vectors) {
    gd.movie.frame[gd.movie.cur_frame].redraw_horiz = 1;
  }
	    
  start_timer();      /* Xview sometimes "forgets the interval timer" -restart it just in case */

}

#ifdef NOTNOW

/*************************************************************************
 * Notify callback function for `start_st'.
 */

void movie_start_proc(Panel_item item, int value, Event *event)
{
  // Use unused parameters
  item = 0; event = NULL;
  movie_start(value);
}

/*************************************************************************
 * Notify callback function for `movie_type_st'.
 */
void movie_type_proc(Panel_item item, int value, Event *event)
{
  time_t    clock;
  static time_t    last_time = 0;
     
  // Use unused parameters
  item = 0; event = NULL;
     
  gd.movie.mode = value;
     
  switch(value) {
    case REALTIME_MODE: /* REAL_TIME  */

      gd.movie.cur_frame = gd.movie.num_frames -1;
      gd.movie.end_frame = gd.movie.num_frames -1;

      clock = time(0);
            
      gd.movie.start_time = clock - (time_t) ((gd.movie.num_frames -1) *
                                              gd.movie.time_interval * 60.0);
      gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);
      gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
      gd.coord_expt->time_seq_num++;

      break;

    case ARCHIVE_MODE:    /* Time Span */
      gd.movie.cur_frame = 0;
      gd.movie.end_frame = gd.movie.num_frames -1;
      gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
      gd.coord_expt->time_seq_num++;
      break;
  }

  // If the movie time has changed
  if(gd.movie.start_time != last_time) {
    reset_time_points();
    // Reset gridded and product data validity flags
    invalidate_all_data();
    set_redraw_flags(1,1);
  }

  // Set forecast as past time choosers to "now" 
  xv_set(gd.fcast_pu->fcast_st,PANEL_VALUE,0,NULL);
  xv_set(gd.past_pu->past_hr_st,PANEL_VALUE,0,NULL);

  last_time = gd.movie.start_time;

  update_movie_popup();

}

/*************************************************************************
 * Notify callback function for `movie_frame_sl'.
 */
void movie_frame_proc(Panel_item item, int value, Event *event)
{
  // Use unused parameters
  item = 0; event = NULL;
    
  gd.movie.cur_frame = value - 1;

}

/*************************************************************************
 * Notify callback function for `movie_speed_sl'.
 */
void movie_speed_proc(Panel_item item, int value, Event *event)
{
  // Use unused parameters
  item = 0; event = NULL;
     
  xv_set(gd.movie_pu->movie_speed_sl,PANEL_VALUE,value,NULL);
     
  /* Calc delay in 25 msec increments above some minimum time */
  gd.movie.display_time_msec = (MOVIE_SPEED_RANGE - value) * MOVIE_DELAY_INCR + MIN_MOVIE_TIME;
}

/*************************************************************************
 * Notify callback function for `movie_dwell_sl'.
 */
void movie_delay_proc(Panel_item item, int value, Event *event)
{
  // Use unused parameters
  item = 0; event = NULL;
     
  gd.movie.delay = value * 1000;
}
void set_display_time(time_t utime);

/*************************************************************************
 * Notify callback function for `start_time_tx'.
 */
Panel_setting start_time_proc(Panel_item item, Event *event)
{
  time_t    utime;
  UTIMstruct    temp_utime;
  char *    cvalue = (char *) xv_get(item, PANEL_VALUE);
    
  // Fill in the current values.
  UTIMunix_to_date(gd.movie.start_time,&temp_utime);


  // Replace values in temp_utime
  parse_string_into_time(cvalue,&temp_utime);

  utime = UTIMdate_to_unix(&temp_utime);

  set_display_time(utime);

  return panel_text_notify(item, event);

}


#endif

/*************************************************************************
 * SET_DISPLAY_TIME
 */
void set_display_time(time_t utime)
{
  int i,j,found;
  time_t    last_time;
  time_t    target_time;

  movie_frame_t    tmp_frame[MAX_FRAMES];    /* info about each frame */

  last_time = gd.movie.start_time;
     
  // Round to the nearest even interval 
  utime -= (utime % gd.movie.round_to_seconds);

  // Already set to this time
  if(utime == gd.movie.start_time) return;

  /* if starting time moves more than 2 volume intervals, assume we want archive mode */
  if(abs(last_time - utime) > (2 * gd.movie.time_interval * 60)) {
    gd.movie.mode = ARCHIVE_MODE;
    gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
    gd.coord_expt->time_seq_num++;
    // xv_set(gd.movie_pu->movie_type_st,PANEL_VALUE,ARCHIVE_MODE,NULL);

  } else {
    if(gd.movie.mode == REALTIME_MODE) {
      gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
      gd.coord_expt->time_seq_num++;

      // Not Sensible to change start times in real time mode - Ignore;
      update_movie_popup();
      return ;
    }
  }

  gd.movie.start_time = utime;

  // Record the time we're currently on
  target_time = gd.movie.frame[gd.movie.cur_frame].time_start;

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

  // Reset gridded and product data validity flags
  invalidate_all_data();

  update_movie_popup();
     
}

#ifdef NOTNOW

/*************************************************************************
 *
 * SET_SWEEP_PROC 
 */
void set_sweep_proc(Panel_item item, int value, Event *event)
{
  static int last_delay = 0;

  if (value) {
    last_delay = gd.movie.delay;
    gd.movie.delay = 0;
    gd.movie.sweep_on = 1;
  } else {
    gd.movie.delay = last_delay;
    gd.movie.sweep_on = 0;
  }
}

/*************************************************************************
 * Notify callback function for `end_frame_tx'.
 */
Panel_setting end_frame_proc(Panel_item item, Event *event)
{

  int value = atoi((char *) xv_get(item, PANEL_VALUE));

  set_end_frame(value);
	
  return panel_text_notify(item, event);
}

#endif

/*************************************************************************
 *  SET_END_FRAME
 */
void set_end_frame(int num_frames)
{
  int i,j;
  time_t    target_time;
  movie_frame_t    tmp_frame[MAX_FRAMES];    /* info about each frame */
  int old_frames;

  gd.movie.end_frame = num_frames -1;

  // Sanity check
  if(gd.movie.end_frame < 0) gd.movie.end_frame = 0;
  if(gd.movie.end_frame >= MAX_FRAMES) gd.movie.end_frame = MAX_FRAMES -1;
  old_frames = gd.movie.num_frames;
  gd.movie.num_frames = gd.movie.end_frame +1;

  if(gd.movie.num_frames > 1) {
    // xv_set(gd.movie_pu->movie_frame_sl,XV_SHOW,TRUE,NULL);
  } else {
    // xv_set(gd.movie_pu->movie_frame_sl,XV_SHOW,FALSE,NULL);
  }

  target_time = gd.movie.frame[gd.movie.cur_frame].time_start;

  if(gd.movie.mode == REALTIME_MODE) {
    gd.movie.cur_frame = gd.movie.end_frame;
    // Make a temporary copy
    memcpy(tmp_frame,gd.movie.frame,sizeof(movie_frame_t) * MAX_FRAMES);

    // Zero out global array
    memset(gd.movie.frame,0,sizeof(movie_frame_t) * MAX_FRAMES);

    // Start point changes
    gd.movie.start_time -= (time_t) ((gd.movie.time_interval * 60.0) *
                                     (gd.movie.num_frames - old_frames));

    reset_time_points();
	 
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
        gd.movie.frame[i].redraw_horiz = 1;
        gd.movie.frame[i].redraw_vert = 1;
      }
    } else {
      for(i = 0, j = old_frames - gd.movie.num_frames ; j < old_frames; i++, j++) {
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
  } else {
    gd.movie.cur_frame = 0;
    // Start point remains the same
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
  }
     
  for(i=0; i < gd.movie.num_frames; i++) {
    if(target_time >= gd.movie.frame[i].time_start &&
       target_time <= gd.movie.frame[i].time_end) {

      gd.movie.cur_frame = i;
    }
  }
  // Reset gridded and product data validity flags
  invalidate_all_data();
     
  update_movie_popup();

  adjust_pixmap_allocation();
     
  return;
}

#ifdef NOTNOW

/*************************************************************************
 * Notify callback function for `time_interval_tx'.
 */
Panel_setting time_interv_proc(Panel_item item, Event *event)
{
  char *  value = (char *) xv_get(item, PANEL_VALUE);
  time_t    utime;
  int last_climo_mode = gd.movie.climo_mode;


  if(strncasecmp(value,"1 d",3) == 0) {
    gd.movie.climo_mode = DAILY_INTERVAL;
  } else if(strncasecmp(value,"1d",2) == 0) {
    gd.movie.climo_mode = DAILY_INTERVAL;
  } else if(strncasecmp(value,"1 y",3) == 0) {
    gd.movie.climo_mode = YEARLY_INTERVAL;
  } else if(strncasecmp(value,"1y",2) == 0) {
    gd.movie.climo_mode = YEARLY_INTERVAL;
  } else { 
    gd.movie.climo_mode = REGULAR_INTERVAL;
  }

  switch(gd.movie.climo_mode) {
    case REGULAR_INTERVAL:
      gd.movie.time_interval = atof(value);

      if(gd.movie.mode == REALTIME_MODE) {
        utime = time(0);
        gd.movie.start_time = (time_t) ((utime - (gd.movie.time_interval * 60 * gd.movie.num_frames)));
      }
      break;

    case DAILY_INTERVAL:
      if(last_climo_mode != gd.movie.climo_mode) {
        gd.movie.start_time = gd.movie.frame[gd.movie.cur_frame].time_start;
        gd.movie.cur_frame = 0;

      }
      gd.movie.time_interval = 1440.0;
      break;

    case YEARLY_INTERVAL:
      if(last_climo_mode != gd.movie.climo_mode) {
        gd.movie.start_time = gd.movie.frame[gd.movie.cur_frame].time_start;
        gd.movie.cur_frame = 0;
      }
      gd.movie.time_interval = 525600.0;
      break;

  }

  reset_time_points();    /* Fill in the time values for each frame */

  // Reset gridded and product data validity flags
  invalidate_all_data();

  reset_time_allowances();

  update_movie_popup();

  set_redraw_flags(1,1);
    
  return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `fcast_per_tx'.
 */
Panel_setting set_fcast_period_proc(Panel_item item, Event *event)
{
  static time_t    last_time;
  time_t    clock;
  char *  value = (char *) xv_get(item, PANEL_VALUE);
     
     
  gd.movie.forecast_interval = atof(value);
     
  switch(gd.movie.mode) {
    case REALTIME_MODE: /* Most recent */

      clock = time(0);
      if(gd.forecast_mode) clock += (time_t) (gd.movie.forecast_interval * 60.0);
            
      last_time = (time_t) (clock - ((gd.movie.num_frames -1) * gd.movie.time_interval * 60.0));
      last_time -= (last_time % gd.movie.round_to_seconds);

      gd.movie.start_time = last_time;
            
      reset_time_points();

      // Reset gridded and product data validity flags
      invalidate_all_data();
     
      update_movie_popup();

      set_redraw_flags(1,1);

      break;

  }
  return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `save_loop_bt'.
 */
void
  save_loop_proc(Panel_item item, Event *event)
{
  gd.movie.cur_frame = 0;

  set_redraw_flags(1, 1);
  invalidate_all_data();

  gd.series_save_active = 1;
  gd.movie.movie_on = 1;
}

/*************************************************************************
 * Event callback function for `time_can'.
 */
Notify_value
  time_canvas_event_proc(Xv_window win, Event *event, Notify_arg arg, Notify_event_type type)
{
  //fprintf(stderr, "cidd_gui: time_canvas_event_proc: event %d\n", event_id(event));

  if(event_action(event) == ACTION_SELECT || event_id(event) == LOC_MOVEWHILEBUTDOWN ) {
    if(gd.time_plot) gd.time_plot->Event_handler(event_x(event), event_y(event),
                                                 event_id(event), event_is_up(event));
  }

  return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*************************************************************************
 * Repaint callback function for `time_can'.
 */
void
  time_canvas_repaint(Canvas canvas, Xv_window paint_window, Display *display, Window xid, Xv_xrectlist *rects)
{
  //fputs("cidd_gui: time_canvas_repaint\n", stderr);
  if(gd.time_plot) gd.time_plot->Draw();

}  

#endif
