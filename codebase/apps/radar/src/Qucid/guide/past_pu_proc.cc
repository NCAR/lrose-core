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
 * PAST_PU_PROC.C - Notify and event callback functions for the Past time
 *   Selector
 */

#define PAST_PU_PROC

#include "cidd.h"


/*************************************************************************
 * Notify callback function for `past_hr_st'.
 */
void past_hr_proc(Panel_item item, int value, Event *event)
{
    // Use unused parameters
    item = 0; event = NULL;

    // Scale this the same as in gui_init.cc to handle forecast intervals over 24 hours
    int interval = (int) (gd.movie.past_interval/ 25.0 + 1.0); 
    time_t now = time(0);
    
    // Magnify the movie frame  interval when setting forecast or past time mode
    if(gd.movie.magnify_mode == 0 && value != 0) {
	  gd.movie.time_interval *= gd.movie.magnify_factor;
	  gd.movie.magnify_mode = 1;
    }

    //  Put movie frame interval back.
    if(gd.movie.magnify_mode != 0 && value == 0) {
	  gd.movie.time_interval /= gd.movie.magnify_factor;
	  gd.movie.magnify_mode = 0;
    }

    if(value == 0 ) {
        gd.movie.mode = REALTIME_MODE;
		gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
		gd.coord_expt->time_seq_num++;
    } else {
        gd.movie.mode = ARCHIVE_MODE;
		gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
		gd.coord_expt->time_seq_num++;
    }

    // Set the beginning of the loop 
    gd.movie.start_time = now - (time_t) ((gd.movie.num_frames -1) * gd.movie.time_interval * 60.0);
    gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);

    gd.movie.start_time -= value * 3600 * interval;

    xv_set(gd.past_pu->past_pu,FRAME_CMD_PUSHPIN_IN, FALSE, XV_SHOW, FALSE, NULL); 

    // Pop back up the button 
    gd.menu_bar.last_callback_value &= ~gd.menu_bar.show_past_menu_bit; 
    xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,gd.menu_bar.last_callback_value,NULL); 

    // Move to last frame in loop to go to the time selected by the user.
    gd.movie.cur_frame = gd.movie.end_frame; 

    reset_time_points();
    update_movie_popup();
    update_frame_time_msg(gd.movie.cur_frame);
    reset_data_valid_flags(1,1);
    if (gd.prod_mgr) {
      gd.prod_mgr->reset_product_valid_flags();
      gd.prod_mgr->reset_times_valid_flags();
    }
    set_redraw_flags(1,1);
}

