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
/********************************************************************
 * CIDD_RESET.C Routines that reset the display to some default starting point
 *
 * Frank Hage NCAR, Research Applications Program
 */

#define CIDD_RESET 1
#include "cidd.h"

// extern void set_fcast_tm_proc(Panel_item item, int value, Event *event);

//*******************************************************************
// INVALIDATE_ALL_DATA

void invalidate_all_data()
{
  // Invalidate Product data
  if(gd.prod_mgr) {
    gd.prod_mgr->reset_product_valid_flags();
    gd.prod_mgr->reset_times_valid_flags();
  }

  // Invalidate gridded data
  reset_data_valid_flags(1,1);
  reset_terrain_valid_flags(1,1);

  // Invalidate the lists of data times
  reset_time_list_valid_flags();

  // Cancel any pending requests
  cancel_pending_request();
}

//*******************************************************************
// CLOSE_ALL_POPUPS

void close_all_popups()
{
  // Restore the Menu bar - in prep of all popups being closed 
  // gd.menu_bar.last_callback_value &= (gd.menu_bar.winds_onoff_bit |
  //                                     gd.menu_bar.symprods_onoff_bit |
  //                                     gd.menu_bar.landuse_onoff_bit |
  //                                     gd.menu_bar.report_mode_bit |
  //                                     gd.menu_bar.loop_onoff_bit);

  // xv_set(gd.h_win_horiz_bw->main_st,
  //          PANEL_VALUE, gd.menu_bar.last_callback_value,
  //          NULL);
  if(gd.run_unmapped) return;

  // Close all popup panels
  // xv_set(gd.bookmk_pu->bookmk_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.cmd_pu->cmd_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.data_pu->data_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.draw_pu->draw_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.fcast_pu->fcast_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL); 
  // xv_set(gd.fields_pu->fields_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.gen_time_pu->popup1,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.movie_pu->movie_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.over_pu->over_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.page_pu->page_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.past_pu->past_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL); 
  // xv_set(gd.prod_pu->prod_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.route_pu->route_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.status_pu->status_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.v_win_v_win_pu->v_win_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
  // xv_set(gd.zoom_pu->zoom_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);
}

/********************************************************************
 * RESET_DISPLAY: 
 */
 
void reset_display()
{
  int i;
  u_int value;
  double world_x,world_y;

  struct timeval tp;
  struct timezone tzp;

  static int first_time = 1;
  static int planview_start_page = 0;
  static int xsect_start_page = 0;
  static double start_height = -1.0;

  if(_params.run_once_and_exit) return;  // Do not reset things in batch mode

  // Set this static first time through
  if(start_height < 0.0) {
    if( gd.num_render_heights == 0 ) {
      start_height = _params.start_ht;
    } else {
      start_height = gd.height_array[0];
    }
    planview_start_page = _params.planview_start_page;
    if(planview_start_page >= gd.num_datafields ||
       planview_start_page < 0) planview_start_page = 0;
       
    xsect_start_page = _params.xsect_start_page;
    if(xsect_start_page >= gd.num_datafields ||
       xsect_start_page < 0) xsect_start_page = 0;

  }

  gettimeofday(&tp, &tzp);

  // Move movie loop to the last frame in real-time mode
  if(gd.movie.demo_mode == 0 || gd.movie.demo_time == 0) {

    // Set back to now if in real time mode real time;
    // set_fcast_tm_proc(0,0,NULL);
     
    // gd.movie.cur_frame = gd.movie.end_frame;
     
    gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
    gd.coord_expt->time_seq_num++;
     
    gd.movie.end_frame = gd.movie.num_frames -1;
    gd.movie.cur_frame = gd.movie.end_frame;
     
  } else {
    gd.movie.cur_frame = gd.movie.start_frame;
    gd.movie.start_time = gd.movie.demo_time;
    gd.movie.mode = ARCHIVE_MODE;
    gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
    gd.coord_expt->time_seq_num++;
     
    gd.h_win.movie_page = gd.h_win.page;
    gd.v_win.movie_page = gd.v_win.page;
    gd.movie.cur_frame = 0;
     
    reset_time_points();
    update_movie_popup();
     
  }

  if(! first_time) {  // Turn off the loop except for the first time to allow
    first_time = 0;  // real time displays to start with the loop on.
     
    gd.movie.movie_on = 0;
    // movie button needs to be released
    // gd.menu_bar.last_callback_value = gd.menu_bar.last_callback_value &
    //    			       ~(gd.menu_bar.loop_onoff_bit);
  }

  // Restore the movie loop
  reset_time_points();
  invalidate_all_data();

  // Mark the images for redrawing 
  set_redraw_flags(1,1);

  // Set back to first page.
  gd.h_win.page = 0;

  // Set forecast as past time choosers to "now"
  // if(! gd.run_unmapped) xv_set(gd.fcast_pu->fcast_st,PANEL_VALUE,0,NULL);
  // if(! gd.run_unmapped) xv_set(gd.past_pu->past_hr_st,PANEL_VALUE,0,NULL);

  // xv_set(gd.v_win_v_win_pu->route_st, PANEL_VALUE,0,NULL);

  gd.v_win.active = 0;  // Indicate cross section is not open
  gd.movie.active = 0;  // Indicate movie control panel is not open

  if(gd.movie.magnify_mode) {
    gd.movie.time_interval_mins /= gd.movie.magnify_factor;
    gd.movie.magnify_mode = 0;  // Indicate movie 
  }

  gd.report_mode = 0;  // Turn off report mode.
  // Release menu bar button
  // gd.menu_bar.last_callback_value = gd.menu_bar.last_callback_value &
  //      			       ~(gd.menu_bar.report_mode_bit);


  // Restore initial state of winds on-off button
  gd.layers.wind_vectors = gd.layers.init_state_wind_vectors;
  if(gd.layers.wind_vectors) {
    // gd.menu_bar.last_callback_value |= gd.menu_bar.winds_onoff_bit;
  } else {
    // gd.menu_bar.last_callback_value =gd.menu_bar.last_callback_value 
    //  & ~(gd.menu_bar.winds_onoff_bit);
  }

  if(! gd.run_unmapped) update_movie_popup();

  close_all_popups();

  // Restore the starting overlay on/off state
  value = 0;
  int total = 0;  // Total unique menu items.
  int used = 0;  // Check for duplicate labels
  for(i=0; i < gd.num_map_overlays; i++) {
    gd.over[i]->active = gd.over[i]->default_on_state;

    used = 0;
    for(int j=0; j < i; j++) {
      if(gd.over[j]->control_label == gd.over[i]->control_label) {
        used = 1;
      }
    }

    // If <= 32 - update the choice widget
    if( gd.num_map_overlays <= 32 && gd.over[i]->active) {
      value |= 1 <<total ;
    }

    // Otherwise update the scrolling list widget
    if(gd.num_map_overlays > 32 && gd.over[i]->active) {
      // if(! gd.run_unmapped) xv_set(gd.over_pu->over_lst, PANEL_LIST_SELECT, total, TRUE, NULL);
    } else {
      // if(! gd.run_unmapped) xv_set(gd.over_pu->over_lst, PANEL_LIST_SELECT, total, FALSE, NULL);
    }
    if(!used) total++;
  }
  // If <= 32 - update the choice widget
  // if(gd.num_map_overlays <= 32) if(! gd.run_unmapped) xv_set(gd.over_pu->over_pu_st, PANEL_VALUE,value,NULL);

  // Restore the Product starting on/off state.
  if(_params.symprod_prod_info_n <= 32) { 
    value = 0;
    for ( i = 0; i < _params.symprod_prod_info_n ; i++) {
      if(_params._symprod_prod_info[i].on_by_default == TRUE) value |= 1 <<i;
      gd.prod_mgr->set_product_active(i,(int) _params._symprod_prod_info[i].on_by_default); 
    }
    // if(! gd.run_unmapped) xv_set(gd.prod_pu->prod_st, PANEL_VALUE, value,NULL);
  } else {
    for ( i = 0; i < _params.symprod_prod_info_n ; i++) { 
      if( _params._symprod_prod_info[i].on_by_default == TRUE) {
        // if(! gd.run_unmapped) xv_set(gd.prod_pu->prod_lst,PANEL_LIST_SELECT, i, TRUE,NULL);
      } else {
        // if(! gd.run_unmapped) xv_set(gd.prod_pu->prod_lst,PANEL_LIST_SELECT, i, FALSE,NULL);
      }
      gd.prod_mgr->set_product_active(i,(int) _params._symprod_prod_info[i].on_by_default);
    }
  }

  // Restore the zoom level
  // set_domain_proc(gd.zoom_pu->domain_st,gd.h_win.start_zoom_level, (Event *) NULL);

  // restore the starting height
  gd.h_win.cur_ht = start_height;

  // DO a pseudo click to reset ancillary displays
  gd.proj.latlon2xy( gd.h_win.reset_click_lat, gd.h_win.reset_click_lon, 
                     world_x,world_y);

  gd.coord_expt->button = 0;
  gd.coord_expt->selection_sec = tp.tv_sec; 
  gd.coord_expt->selection_usec = tp.tv_usec;
  gd.coord_expt->pointer_lon = gd.h_win.reset_click_lon;
  gd.coord_expt->pointer_lat = gd.h_win.reset_click_lat;
  gd.coord_expt->pointer_x = world_x;
  gd.coord_expt->pointer_y = world_y;
  gd.coord_expt->pointer_alt_min = gd.h_win.cur_ht;
  gd.coord_expt->pointer_alt_max = gd.h_win.cur_ht;
  gd.coord_expt->pointer_ht_km = gd.h_win.cur_ht;
  gd.coord_expt->click_type = CIDD_RESET_CLICK;

  gd.coord_expt->pointer_seq_num++; 

  // Restore the initial page
  set_field(planview_start_page);
  set_v_field(gd.field_index[xsect_start_page]);
}
