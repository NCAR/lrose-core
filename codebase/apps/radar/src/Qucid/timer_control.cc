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
/**********************************************************************
 * TIMER_CONTROL.C:  Routines that initialize and control timed based events
 *
 *
 * For the Configurable Interactive Data Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 *
 */

#define TIMER_CONTROL 1
#include "cidd.h"
#include <dsserver/DmapAccess.hh> 
#include <toolsa/TaStr.hh> 

int    redraw_interv = 0;
int    update_interv = 0;
int    update_due = 0;
int    h_copy_flag = 0;
int    v_copy_flag = 0;

/**********************************************************************
 * HANDLE_CLIENT_EVENT: 
 */

void handle_client_event()
{
  time_t clock;

  if(gd.debug1) fprintf(stderr,"Found Client Event: %d, Args: %s\n",
			gd.coord_expt->client_event,gd.coord_expt->client_args);

  switch(gd.coord_expt->client_event) {
    case NEW_MDV_AVAIL:
      remote_new_mdv_avail(gd.coord_expt->client_args);
      break;

    case NEW_SPDB_AVAIL:
      remote_new_spdb_avail(gd.coord_expt->client_args);
      break;

    case RELOAD_DATA:
      invalidate_all_data();
      set_redraw_flags(1,1);
      break;

    case SET_FRAME_NUM:
      int	frame;
      if((sscanf(gd.coord_expt->client_args,"%d",&frame)) == 1) {
        /* Sanity Check */
        if(frame <= 0 ) frame = 1;
        if(frame > gd.movie.num_frames ) frame = gd.movie.num_frames ;

        gd.movie.cur_frame = frame - 1;
      } else {
        fprintf(stderr,"Invalid SET_FRAME_NUM: Args: %s\n",gd.coord_expt->client_args);
      }
      break;

    case SET_NUM_FRAMES:
      int	nframes;
      if((sscanf(gd.coord_expt->client_args,"%d",&nframes)) == 1) {
        set_end_frame(nframes);
      } else {
        fprintf(stderr,"Invalid SET_NUM_FRAMES: Args: %s\n",gd.coord_expt->client_args);
      }
      break;

    case SET_REALTIME:
      gd.movie.mode = REALTIME_MODE;
      gd.movie.cur_frame = gd.movie.num_frames -1;
      gd.movie.end_frame = gd.movie.num_frames -1;
      clock = time(0);
      gd.movie.start_time = clock - (time_t) ((gd.movie.num_frames -1) *
                                              gd.movie.time_interval_mins * 60.0);
      gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);
      gd.coord_expt->runtime_mode = RUNMODE_REALTIME;	
      gd.coord_expt->time_seq_num++;

      reset_time_points();
      invalidate_all_data();
      set_redraw_flags(1,1);

      // Set forecast and past time choosers to "now"
      // xv_set(gd.fcast_pu->fcast_st,PANEL_VALUE,0,NULL);
      // xv_set(gd.past_pu->past_hr_st,PANEL_VALUE,0,NULL);
      // Set movie mode widget to REALTIME 
      // xv_set(gd.movie_pu->movie_type_st,PANEL_VALUE,0,NULL);

      break;

    case SET_TIME:
      UTIMstruct ts;
      time_t interest_time;

      if((sscanf(gd.coord_expt->client_args,"%ld %ld %ld %ld %ld %ld",
                 &ts.year,&ts.month,&ts.day, &ts.hour,&ts.min,&ts.sec)) == 6) {
		
        interest_time = UTIMdate_to_unix(&ts);

        set_display_time(interest_time);
        invalidate_all_data();
        set_redraw_flags(1,1);
      } else {
        fprintf(stderr,"Invalid SET_TIME Args: %s\n",gd.coord_expt->client_args);
      }
      break;

    default: {}

  }

  // Reset  the command
  gd.coord_expt->client_event = NO_MESSAGE;
}


/**********************************************************************
 * CHECK_FOR_EXPIRED_DATA: Check all data and determine if 
 * the data's expiration time has been exceeded - Mark it invalid if so.
 */

void check_for_expired_data(time_t tm)
{
  int i;

  // Mark all data past the expiration data as invalid
  /* look thru primary data fields */
  for (i=0; i < gd.num_datafields; i++) {
    /* if data has expired or field should be updated */
    if (gd.mrec[i]->h_mhdr.time_expire < tm ) {
      //        if(gd.debug1) fprintf(stderr,"Field: %s expired at %s\n",
      //		 gd.mrec[i]->button_name,
      //		 asctime(gmtime(&((time_t) gd.mrec[i]->h_mhdr.time_expire))));
      gd.mrec[i]->h_data_valid = 0;
      gd.mrec[i]->v_data_valid = 0;
    }
  }

  /* Look through wind field data too */
  for (i=0; i < gd.layers.num_wind_sets; i++ ) {
    if (gd.layers.wind[i].active) {
      if (gd.layers.wind[i].wind_u->h_mhdr.time_expire < tm) {
        gd.layers.wind[i].wind_u->h_data_valid = 0;
        gd.layers.wind[i].wind_u->v_data_valid = 0;
      }
      if (gd.layers.wind[i].wind_v->h_mhdr.time_expire < tm) {
        gd.layers.wind[i].wind_v->h_data_valid = 0;
        gd.layers.wind[i].wind_v->v_data_valid = 0;
      }
      if (gd.layers.wind[i].wind_w != NULL && gd.layers.wind[i].wind_w->h_mhdr.time_expire < tm) {
        gd.layers.wind[i].wind_w->h_data_valid = 0;
        gd.layers.wind[i].wind_w->v_data_valid = 0;
      }
    }
  }
}

/**********************************************************************
 * CHECK_FOR_DATA_UPDATES: Check all data and determine if 
 * new data has arrived - Mark it invalid if so.
 */

void check_for_data_updates(time_t tm)
{
  DmapAccess dmap;
  int i,j;
  char *start_ptr, *end_ptr;
  char dir_buf[MAX_PATH_LEN];

  if( strlen(_params.datamap_host) < 2 || dmap.reqAllInfo(_params.datamap_host) != 0) {

    // Force a reload of the data
    reset_data_valid_flags(1,1);
    if (gd.prod_mgr) {
      gd.prod_mgr->reset_product_valid_flags();
      gd.prod_mgr->reset_times_valid_flags();
    }

  } else {   // Got valid  data back from the Data Mapper

    int nsets = dmap.getNInfo();


    if(gd.debug1) fprintf(stderr,"Found %d Datamapper info sets\n",nsets);

    /* look thru all data fields */
    for (i=0; i < gd.num_datafields; i++) {
      // pull out dir from URL

      end_ptr = strrchr(gd.mrec[i]->url,'&');
      if(end_ptr == NULL) continue;  // broken URL.

      start_ptr =  strrchr(gd.mrec[i]->url,':');
      if(start_ptr == NULL) {
	start_ptr =  gd.mrec[i]->url; // Must be a local file/dir based URL
      } else {
	start_ptr++;  // Move up one character
      }

      strncpy(dir_buf,start_ptr,(size_t) (end_ptr - start_ptr + 1));
      dir_buf[(size_t) (end_ptr - start_ptr)] = '\0'; // Null terminate
	
      // Look through the data mapper info
      for(j = 0;  j < nsets; j++) {
	const DMAP_info_t &info = dmap.getInfo(j);

	// See if any data matches
	if(strstr(info.dir,dir_buf) != NULL) {
          // Note unix_time is signed (time_t)  and info.end_time is unsigned int
          if (gd.mrec[i]->h_date.unix_time < (int) info.end_time) {
            gd.mrec[i]->h_data_valid = 0;
            gd.mrec[i]->v_data_valid = 0;
          }
	}
      }
    }

    for (i=0; i < gd.layers.num_wind_sets; i++ ) {
      /* Look through wind field data too */
      if (gd.layers.wind[i].active) {
	// pull out dir from URL

	end_ptr = strrchr(gd.layers.wind[i].wind_u->url,'&');
	if(end_ptr == NULL) continue;  // broken URL.

	start_ptr =  strrchr(gd.layers.wind[i].wind_u->url,':');
	if(start_ptr == NULL) {
          start_ptr =  gd.mrec[i]->url; // Must be a local file/dir based URL
	} else {
          start_ptr++;  // Move up one character
	}

	strncpy(dir_buf,start_ptr,(size_t) (end_ptr - start_ptr + 1));
	dir_buf[(size_t) (end_ptr - start_ptr)] = '\0'; // Null terminate
	
	// Look through the data mapper info
	for(j = 0;  j < nsets; j++) {
          const DMAP_info_t &info = dmap.getInfo(j);
          // See if any data matches
          if(strstr(info.dir,dir_buf) != NULL) {
            // Check if that data is more current
            if (gd.layers.wind[i].wind_u->h_date.unix_time < (int) info.end_time) {
              gd.layers.wind[i].wind_u->h_data_valid = 0;
              gd.layers.wind[i].wind_u->v_data_valid = 0;
              gd.layers.wind[i].wind_v->h_data_valid = 0;
              gd.layers.wind[i].wind_v->v_data_valid = 0;
              if (gd.layers.wind[i].wind_w != NULL) {
		gd.layers.wind[i].wind_w->h_data_valid = 0;
		gd.layers.wind[i].wind_w->v_data_valid = 0;
              }
            }
          }  // If a match
	} // For all data mapper info
      }   // If wind layer is active
    }     // For all wind layers

    gd.prod_mgr->check_product_validity(tm, dmap);
  } // If data mapper info is availible
}

////////////////////////////////////////////////////////////////// 
// check_what_needs_rendering:

void check_what_needs_rendering(int frame_index)
{
  int i;

  // If data used to draw plan view is invalid - Indicate plane view image needs rerendering
  if (gd.mrec[gd.h_win.page]->h_data_valid == 0 || gd.prod_mgr->num_products_invalid() > 0) {
    gd.movie.frame[frame_index].redraw_horiz = 1;
    gd.h_win.redraw[gd.h_win.page] = 1;
  }

  for ( i=0; i < gd.layers.num_wind_sets; i++ ) {
    /* Look through wind field data too */
    if (gd.layers.wind[i].active) {
      if (gd.layers.wind[i].wind_u->h_data_valid == 0) {
        gd.movie.frame[frame_index].redraw_horiz = 1;
        gd.h_win.redraw[gd.h_win.page] = 1;
      }
      if (gd.layers.wind[i].wind_v->h_data_valid == 0) {
        gd.movie.frame[frame_index].redraw_horiz = 1;
        gd.h_win.redraw[gd.h_win.page] = 1;
      }
      if (gd.layers.wind[i].wind_w != NULL && gd.layers.wind[i].wind_w->h_data_valid == 0) {
        gd.movie.frame[frame_index].redraw_horiz = 1;
        gd.h_win.redraw[gd.h_win.page] = 1;
      }
    }
  }

  /* Check overlay contours if active */
  for(i= 0; i < NUM_CONT_LAYERS; i++) {
    if(gd.layers.cont[i].active) {
      if(gd.mrec[gd.layers.cont[i].field]->h_data_valid == 0) {
	gd.movie.frame[frame_index].redraw_horiz = 1;
	gd.h_win.redraw[gd.h_win.page] = 1;
      }
    }
  } 

  // If data used to draw cross section is invalid - Indicate pcross section image needs rerendering
  if (gd.v_win.active ) {
    if(gd.mrec[gd.v_win.page]->v_data_valid == 0)  {
      gd.movie.frame[frame_index].redraw_vert = 1;
      gd.v_win.redraw[gd.v_win.page] = 1;
    }

    for ( i=0; i < gd.layers.num_wind_sets; i++ ) {
      /* Look through wind field data too */
      if (gd.layers.wind[i].active) {
	if (gd.layers.wind[i].wind_u->v_data_valid == 0) {
          gd.movie.frame[frame_index].redraw_vert = 1;
          gd.v_win.redraw[gd.v_win.page] = 1;
	}
	if (gd.layers.wind[i].wind_v->v_data_valid == 0) {
          gd.movie.frame[frame_index].redraw_vert = 1;
          gd.v_win.redraw[gd.v_win.page] = 1;
	}
	if (gd.layers.wind[i].wind_w != NULL && gd.layers.wind[i].wind_w->v_data_valid == 0) {
          gd.movie.frame[frame_index].redraw_vert = 1;
          gd.v_win.redraw[gd.v_win.page] = 1;
	}
      }
    }

    /* Check overlay contours if active */
    for(i= 0; i < NUM_CONT_LAYERS; i++) {
      if(gd.layers.cont[i].active) {
	if(gd.mrec[gd.layers.cont[i].field]->v_data_valid == 0) {
          gd.movie.frame[frame_index].redraw_vert = 1;
          gd.v_win.redraw[gd.v_win.page] = 1;
	}
      }
    } 
  }
}

/************************************************************************
 * TIMER_FUNC: This routine acts as the main branch point for most of the
 * threads involved in the display. This function gets called through
 * XView's notifier mechanism, which is built on sigalarm().
 * This is parameterized, and usually runs on a 5-100 msec interval.
 * First, any pending IO is handled.
 * Current time tickers, and the display's shmem communications are updated
 * Checks for out-of date images and expired Data are initiated periodically
 * Animation and Rendering are handled.
 * Finally Background Rendering is scheduled
 *
 */

#ifdef NOTNOW

void timer_func( Notify_client   client, int which)
{
  met_record_t *mr;
  int    index,flag = 0;
  int    msec_diff = 0;
  int    msec_delay = 0;
  long   tm = 0;

  Pixmap    h_xid = 0;
  Pixmap    v_xid = 0;

  struct itimerval timer;
  struct timezone cur_tz;
  static  struct timeval cur_tm;
  static  struct timeval last_frame_tm = {0,0};
  static  struct timeval last_dcheck_tm = {0,0};
  static time_t last_tick = 0;
  static long client_seq_num = 0;

  // Use unused parameters
  client = 0; which = 0;
  extern void check_for_io(); 

  if(gd.io_info.outstanding_request) {
    check_for_io();
  }

  if(gd.coord_expt->client_event != NO_MESSAGE) {
    handle_client_event();
  }

  gettimeofday(&cur_tm,&cur_tz);

  /* Update the current time ticker if necessary */
  if(cur_tm.tv_sec > last_tick) {
    if(!_params.run_once_and_exit) {
      char buf[128];
      sprintf(buf,"Idle %d secs, Req: %d, Mode: %d, Type: %d",
              (int) (cur_tm.tv_sec - gd.last_event_time),
              gd.io_info.outstanding_request,
              gd.io_info.mode,
              gd.io_info.request_type);
      if(gd.debug || gd.debug1 || gd.debug2) {
        PMU_force_register(buf);
      } else {
        PMU_auto_register(buf);
      }

    }
    update_ticker(cur_tm.tv_sec);
    last_tick = cur_tm.tv_sec;
    if(gd.last_event_time < (last_tick - _params.idle_reset_seconds)) {
      reset_display();
    }
  }

  msec_delay = gd.movie.display_time_msec;

  /**** Get present frame index *****/
  if (gd.movie.cur_frame < 0) {
    index = gd.movie.num_frames - 1;
  } else {
    index = gd.movie.cur_frame;
  }
  // If no images or IO are pending - Check for remote commands
  if(gd.image_needs_saved == 0 &&
     gd.movie.frame[index].redraw_horiz == 0 &&
     gd.io_info.outstanding_request == 0) { 

    if(gd.v_win.active == 0 || gd.movie.frame[index].redraw_vert == 0) {
      if(gd.remote_ui != NULL) ingest_remote_commands();
    }
  }

  // Update the times in the Coordinate SHMEM
  gd.coord_expt->epoch_start = gd.epoch_start;
  gd.coord_expt->epoch_end = gd.epoch_end; 
  gd.coord_expt->time_min = gd.movie.frame[index].time_start;
  gd.coord_expt->time_max = gd.movie.frame[index].time_end;
  if(gd.movie.movie_on) { 
    gd.coord_expt->time_cent = gd.coord_expt->epoch_end;
  } else {
    gd.coord_expt->time_cent = gd.coord_expt->time_min +
      (gd.coord_expt->time_max - gd.coord_expt->time_min) / 2;
  }
  gd.coord_expt->time_current_field = gd.mrec[gd.h_win.page]->h_mhdr.time_centroid;

  if (gd.movie.movie_on ) {
    flag = 1;        /* set OK state */
    if (gd.movie.frame[index].redraw_horiz != 0) flag = 0;
    if (gd.movie.frame[index].redraw_vert != 0 && gd.v_win.active) flag = 0;

    msec_diff = ((cur_tm.tv_sec - last_frame_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_frame_tm.tv_usec) / 1000);
    if (flag && msec_diff > gd.movie.display_time_msec) {
      /* Advance Movie frame */
      gd.movie.cur_frame += gd.movie.sweep_dir;    

      /* reset to beginning of the loop if needed */
      if (gd.movie.cur_frame > gd.movie.end_frame) {
	if(gd.series_save_active) { // End of the Series Save - Turn off
          char cmd[4096];
          gd.series_save_active = 0;
          gd.movie.movie_on = 0;
          if(_params.series_convert_script !=NULL) {
            STRncopy(cmd,_params.series_convert_script,4096);
            for(int ii= gd.movie.start_frame; ii <= gd.movie.end_frame; ii++) {
              STRconcat(cmd," ",4096);
              STRconcat(cmd,gd.movie.frame[ii].fname,4096);
            }
            printf("Running: %s\n",cmd);
          }
          set_busy_state(1);
          safe_system(cmd,_params.complex_command_timeout_secs);

          if(gd.v_win.active) {
            if(_params.series_convert_script !=NULL) {
              STRncopy(cmd,_params.series_convert_script,4096);
              for(int ii= gd.movie.start_frame; ii <= gd.movie.end_frame; ii++) {
                STRconcat(cmd," ",4096);
                STRconcat(cmd,gd.movie.frame[ii].vfname,4096);
              }
              printf("Running: %s\n",cmd);
            }
            set_busy_state(1);
            safe_system(cmd,_params.complex_command_timeout_secs);
          }

          set_busy_state(0);
          gd.movie.cur_frame = gd.movie.end_frame;
	} else {
          if(gd.movie.sweep_on) {
            gd.movie.sweep_dir = -1;
            gd.movie.cur_frame = gd.movie.end_frame -1;
          } else {
            gd.movie.cur_frame = gd.movie.start_frame -1;
          }
	}
      }

      if(gd.movie.cur_frame < gd.movie.start_frame) { 
        gd.movie.sweep_dir = 1;
        if(gd.movie.sweep_on) {
          gd.movie.cur_frame = gd.movie.start_frame+1;
        }
      }
	
      if (gd.movie.cur_frame == gd.movie.end_frame) {
        msec_delay = gd.movie.delay;
      }

      /**** recalc current frame index *****/
      if (gd.movie.cur_frame < 0) {
        index =  gd.movie.num_frames - 1;
      } else {
        index = gd.movie.cur_frame;
      }
    }
  }
	
  /* Set up convienient pointer to main met record */
  mr = gd.mrec[gd.h_win.page];

  /* Decide which Pixmaps to use for rendering */
  if (gd.movie.movie_on ) {
    /* set to the movie frame Pixmaps */
    h_xid = gd.movie.frame[index].h_xid;
    if (h_xid == 0) {
      if(mr->auto_render) {    
        h_xid = gd.h_win.page_xid[gd.h_win.page];
      } else {
        h_xid = gd.h_win.tmp_xid;
      }
    }

    v_xid = gd.movie.frame[index].v_xid;
    if (v_xid == 0) {
      if(mr->auto_render) {
        v_xid = gd.v_win.page_xid[gd.v_win.page];
      } else {
        v_xid = gd.v_win.tmp_xid;
      }
    }
	
  } else {
    /* set to the field Pixmaps */
    if(mr->auto_render) {
      h_xid = gd.h_win.page_xid[gd.h_win.page];
    } else {
      h_xid = gd.h_win.tmp_xid;
    }

    if(gd.mrec[gd.v_win.page]->auto_render) {
      v_xid = gd.v_win.page_xid[gd.v_win.page];
    } else {
      v_xid = gd.v_win.tmp_xid;
    }
  }


  /******* Handle Real Time Updating  ********/
  switch (gd.movie.mode) {
    case REALTIME_MODE :
      if (time_for_a_new_frame()) {
	rotate_movie_frames(); 
	/* Vectors must be removed from the (currently) last frame if the wind_mode > 0 */
	if(gd.layers.wind_mode && gd.layers.wind_vectors)  {
          gd.movie.frame[gd.movie.cur_frame].redraw_horiz = 1;
	}

	// All product data must be reloaded - Set all to invalid
	if (gd.prod_mgr) {
          gd.prod_mgr->reset_product_valid_flags();
          gd.prod_mgr->reset_times_valid_flags();
	}

	/* Move movie loop to the last frame when aging off old movie frames */
	gd.movie.cur_frame = gd.movie.end_frame;
	goto return_point;
      }

      tm = time(0);
      /* CHECK FOR NEW DATA */
      if ( tm >= update_due ) {

	/* Check only on the last frame - Because its the only "live/realtime" one */
	if (gd.movie.cur_frame == gd.movie.num_frames -1) {
          update_due = tm + update_interv;

          check_for_expired_data(tm);  // Look for old data

          check_for_data_updates(tm);  // Look for data that's newly updated

          check_what_needs_rendering(index);

          // Auto click to get ancillary displays to update too.
          gd.coord_expt->click_type = CIDD_OTHER_CLICK;
          gd.coord_expt->pointer_seq_num++;
	}
      }

      break;

    case ARCHIVE_MODE :
      break;

    default:
      fprintf(stderr,
              "Invalid movie mode %d in timer_func\n",
              gd.movie.mode);
      break;
  } 


  /***** Handle Field changes *****/
  if (gd.h_win.page != gd.h_win.last_page) {
    if (gd.movie.movie_on ) {
      set_redraw_flags(1,0);
    } else {
      if (gd.h_win.redraw[gd.h_win.page] == 0) {
        h_copy_flag = 1;
        gd.h_win.last_page = gd.h_win.page;
      }
    }
  }
  if (gd.v_win.page != gd.v_win.last_page ) {
    if (gd.movie.movie_on ) {
      set_redraw_flags(0,1);
    } else {
      if (gd.v_win.redraw[gd.v_win.page] == 0) {
        v_copy_flag = 1;
        gd.v_win.last_page = gd.v_win.page;
      }
    }
  }




  /******** Handle Frame changes ********/
  if (gd.movie.last_frame != gd.movie.cur_frame && gd.movie.cur_frame >= 0) {
    reset_data_valid_flags(1,1);

    if(_params.symprod_short_requests) {
      // All product data must be reloaded - Set all to invalid
      gd.prod_mgr->reset_product_valid_flags();
    }

    /* Move the indicators */
    // xv_set(gd.movie_pu->movie_frame_sl,
    //        PANEL_VALUE, gd.movie.cur_frame + 1,
    //        NULL);
	
    if(gd.debug2) {
      printf("Moved movie frame, index : %d\n",index);
    }
    gd.coord_expt->epoch_start = gd.epoch_start;
    gd.coord_expt->epoch_end = gd.epoch_end;

    if(gd.movie.movie_on) {
      gd.coord_expt->time_cent = gd.coord_expt->epoch_end;
    } else {
      gd.coord_expt->time_min = gd.movie.frame[index].time_start;
      gd.coord_expt->time_max = gd.movie.frame[index].time_end;
      gd.coord_expt->time_cent = gd.coord_expt->time_min +
	(gd.coord_expt->time_max - gd.coord_expt->time_min) / 2;
      gd.coord_expt->click_type = CIDD_OTHER_CLICK;
      gd.coord_expt->pointer_seq_num++;
    }
    gd.coord_expt->time_current_field = gd.mrec[gd.h_win.page]->h_mhdr.time_centroid;

    /* Change Labels on Frame Begin, End messages */
    update_frame_time_msg(index);
		
    if (gd.movie.frame[index].redraw_horiz == 0) {
      /* Get Frame */
      retrieve_h_movie_frame(index,h_xid);
      h_copy_flag = 1;
    }

    if (gd.v_win.active && gd.movie.frame[index].redraw_vert == 0) {
      retrieve_v_movie_frame(index,v_xid);
      v_copy_flag = 1;
    }

    gd.movie.last_frame = gd.movie.cur_frame;
  }


  /* Draw Selected field - Vertical  for this movie frame */
  if (gd.v_win.active) {
    if (gd.movie.frame[index].redraw_vert) {
      if (gather_vwin_data(gd.v_win.page,gd.movie.frame[index].time_start,
                           gd.movie.frame[index].time_end) == CIDD_SUCCESS) {
        if (gd.v_win.redraw[gd.v_win.page]) {
          render_v_movie_frame(index,v_xid);
          save_v_movie_frame(index,v_xid);
        } 
        gd.movie.frame[index].redraw_vert = 0;
        gd.v_win.redraw[gd.v_win.page] = 0;
        v_copy_flag = 1;
      }
    }
  }

  // generate vert section images as required

  if (gd.images_P->generate_vsection_images) {

    if (gd.images_P->debug) {
      cerr << "============>> generating specified vsection images" << endl;
    }

    for (int ii = 0; ii < gd.images_P->vsection_spec_n; ii++) {

      Cimages_P::vsection_spec_t vsect = gd.images_P->_vsection_spec[ii];
      if (gd.images_P->debug) {
        cerr << "=================>> generating vsection ii: " << ii << endl;
        cerr << "    vsection_label: " << vsect.vsection_label << endl;
        cerr << "    n_waypts: " << vsect.n_waypts << endl;
        cerr << "    waypt_locs: " << vsect.waypt_locs << endl;
      }
      cerr << "=====================================" << endl;
      
      // get waypts

      string waypts_locs(vsect.waypt_locs);
      vector<string> toks;
      TaStr::tokenize(waypts_locs, "(", toks);

      int npts_found = 0;
      for (size_t jj = 0; jj < toks.size(); jj++) {
        double xx, yy;
        if (sscanf(toks[jj].c_str(), "%lg, %lg", &xx, &yy) == 2) {
          gd.h_win.route.x_world[jj] = xx;
          gd.h_win.route.y_world[jj] = yy;
          npts_found++;
        }
      }

      if (npts_found == vsect.n_waypts && npts_found > 1) {
        gd.h_win.route.total_length = 0.0;
        gd.h_win.route.num_segments = npts_found - 1;
        for (int iseg = 0; iseg < gd.h_win.route.num_segments; iseg++) {
          gd.h_win.route.seg_length[iseg] =
            disp_proj_dist(gd.h_win.route.x_world[iseg],gd.h_win.route.y_world[iseg],
                           gd.h_win.route.x_world[iseg+1],gd.h_win.route.y_world[iseg+1]);
          gd.h_win.route.total_length += gd.h_win.route.seg_length[iseg];
        }
      }
      
      if (gather_vwin_data(gd.v_win.page,gd.movie.frame[index].time_start,
                           gd.movie.frame[index].time_end) == CIDD_SUCCESS) {
        gd.series_save_active = 1;
        render_v_movie_frame(index,v_xid);
        save_v_movie_frame(index,v_xid);
      }

    } // ii

  } // if (gd.images_P->generate_vsection_images)

  /* Draw Selected field - Horizontal for this movie frame */
  if (gd.movie.frame[index].redraw_horiz) {
    /* Draw Frame */ 
    if (gather_hwin_data(gd.h_win.page, gd.movie.frame[index].time_start,
                         gd.movie.frame[index].time_end) == CIDD_SUCCESS) {
      if (gd.h_win.redraw[gd.h_win.page]) {
        render_h_movie_frame(index,h_xid);
        save_h_movie_frame(index,h_xid,gd.h_win.page);
      } 

      /* make sure the horiz window's slider has the correct label */
      set_height_label();

      gd.movie.frame[index].redraw_horiz = 0;
      gd.h_win.redraw[gd.h_win.page] = 0;
      h_copy_flag = 1;
    }
  }
	

  /***** Selected Field or movie frame has changed - Copy background image onto visible canvas *****/
  if (h_copy_flag || (gd.h_win.cur_cache_im != gd.h_win.last_cache_im)) {

    if (gd.debug2) {
      fprintf(stderr,
              "\nCopying Horiz grid image - "
              "field %d, index %d xid: %ld to xid: %ld\n",
              gd.h_win.page,index,h_xid,gd.h_win.can_xid[gd.h_win.cur_cache_im]);

    if(gd.h_win.cur_cache_im == gd.h_win.last_cache_im) {
      XCopyArea(gd.dpy,h_xid,
		gd.h_win.can_xid[gd.h_win.cur_cache_im],
		gd.def_gc,    0,0,
		gd.h_win.can_dim.width,
		gd.h_win.can_dim.height,
		gd.h_win.can_dim.x_pos,
		gd.h_win.can_dim.y_pos);
    } else {
      gd.h_win.last_cache_im = gd.h_win.cur_cache_im; 
    }
    gd.h_win.last_page = gd.h_win.page;
    h_copy_flag = 0;

    if (gd.zoom_in_progress == 1) redraw_zoom_box();
    if (gd.pan_in_progress) redraw_pan_line();
    if (gd.route_in_progress) redraw_route_line(&gd.h_win);

    if(!_params.run_once_and_exit) PMU_auto_register("Rendering Products (OK)");

    if (gd.debug2) {
      fprintf(stderr,
              "\nTimer: Displaying Horiz final image - "
              "field %d, index %d xid: %ld to xid: %ld\n",
              gd.h_win.page,index,
              gd.h_win.can_xid[gd.h_win.cur_cache_im],
              gd.h_win.vis_xid);

    /* Now copy last stage pixmap to visible pixmap */
    XCopyArea(gd.dpy,gd.h_win.can_xid[gd.h_win.cur_cache_im],
              gd.h_win.vis_xid,
              gd.def_gc,    0,0,
              gd.h_win.can_dim.width,
              gd.h_win.can_dim.height,
              gd.h_win.can_dim.x_pos,
              gd.h_win.can_dim.y_pos);

    if (gd.zoom_in_progress == 1) redraw_zoom_box();
    if (gd.pan_in_progress) redraw_pan_line();
    if (gd.route_in_progress) redraw_route_line(&gd.h_win);

    // Render a time indicator plot in the movie popup
    if(gd.time_plot)
    {
      gd.time_plot->Set_times((time_t) gd.epoch_start,
			      (time_t) gd.epoch_end,
			      (time_t) gd.movie.frame[gd.movie.cur_frame].time_start,
			      (time_t) gd.movie.frame[gd.movie.cur_frame].time_end,
			      (time_t)((gd.movie.time_interval_mins * 60.0) + 0.5),
			      gd.movie.num_frames); 

      if(gd.movie.active) gd.time_plot->Draw();
    }
    
    /* keep track of how much time will elapse showing the current image */
    gettimeofday(&last_frame_tm,&cur_tz);
	
  }

  if(gd.v_win.cur_cache_im != gd.v_win.last_cache_im) {
    v_copy_flag = 1;
    gd.v_win.last_cache_im = gd.v_win.cur_cache_im;
  }

  if (gd.v_win.active && v_copy_flag) {
    if (gd.debug2) fprintf(stderr,"\nCopying Vert grid image - field %d, index %d xid: %ld to xid: %ld\n",
                           gd.v_win.page,index,v_xid,gd.v_win.can_xid[gd.v_win.cur_cache_im]);

    XCopyArea(gd.dpy,v_xid,
              gd.v_win.can_xid[gd.v_win.cur_cache_im],
              gd.def_gc,    0,0,
              gd.v_win.can_dim.width,
              gd.v_win.can_dim.height,
              gd.v_win.can_dim.x_pos,
              gd.v_win.can_dim.y_pos);
    gd.v_win.last_page  = gd.v_win.page;
    v_copy_flag = 0;

    if (gd.debug2) fprintf(stderr,"\nDisplaying Vert final image - field %d, index %d xid: %ld to xid: %ld\n",
                           gd.v_win.page,index,gd.v_win.can_xid[gd.v_win.cur_cache_im],gd.v_win.vis_xid);

    /* Now copy last stage pixmap to visible pixmap */
    XCopyArea(gd.dpy,gd.v_win.can_xid[gd.v_win.cur_cache_im],
              gd.v_win.vis_xid,
              gd.def_gc,    0,0,
              gd.v_win.can_dim.width,
              gd.v_win.can_dim.height,
              gd.v_win.can_dim.x_pos,
              gd.v_win.can_dim.y_pos);

  }

  // If remote driven image(s) needs saved - do it.
  if(gd.image_needs_saved ) {
    int ready = 1;

    // Check to make sure the images are done
    if((gd.save_im_win & PLAN_VIEW) && gd.h_win.redraw[gd.h_win.page] != 0) ready = 0;
    if((gd.save_im_win & XSECT_VIEW) && gd.v_win.redraw[gd.v_win.page] != 0) ready = 0;
    if(ready) {
      dump_cidd_image(gd.save_im_win,0,0,gd.h_win.page);
      gd.image_needs_saved = 0;
    }
  }


  /***** Handle redrawing background images *****/
  msec_diff = ((cur_tm.tv_sec - last_dcheck_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_dcheck_tm.tv_usec) / 1000);

  if (msec_diff < 0 ||  (msec_diff > redraw_interv  && gd.movie.movie_on == 0)) {
    check_for_invalid_images(index);

    /* keep track of how much time will elapse since the last check */
    gettimeofday(&last_dcheck_tm,&cur_tz);
	
  }

 return_point:


  if (gd.movie.movie_on == 0) {
    if (gd.zoom_in_progress == 1) redraw_zoom_box();
    if (gd.pan_in_progress) redraw_pan_line();
    if (gd.route_in_progress) redraw_route_line(&gd.h_win);

    if (gd.zoom_in_progress == 1) redraw_zoom_box();
    if (gd.pan_in_progress) redraw_pan_line();
    if (gd.route_in_progress) redraw_route_line(&gd.h_win);

  } else {
    gettimeofday(&cur_tm,&cur_tz);
    msec_diff = ((cur_tm.tv_sec - last_frame_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_frame_tm.tv_usec) / 1000);
    msec_delay = msec_delay - msec_diff;
    if(msec_delay <= 0 || msec_delay > 10000) msec_delay = 100;
  }

  if (client_seq_num != gd.coord_expt->client_seq_num) {
    render_click_marks();
    client_seq_num = gd.coord_expt->client_seq_num;
  }

  /* set up interval timer interval */
  timer.it_value.tv_sec = msec_delay / 1000;
  timer.it_value.tv_usec = ((msec_delay  - (timer.it_value.tv_sec * 1000)) * 1000);
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 10000;

  /* Set the interval timer function and start timer */
  notify_set_itimer_func(gd.h_win_horiz_bw->horiz_bw,
                         (Notify_func)timer_func,
                         ITIMER_REAL, &timer, NULL); /*  */


}

#endif

/**********************************************************************
 * STOP_TIMER:  Stop the interval timer
 */

void stop_timer()
{
#ifdef NOTNOW
  struct  itimerval   timer;

  /* set up interval timer interval */
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
#endif

  /* Set the interval timer function and start timer */
  // notify_set_itimer_func(gd.h_win_horiz_bw->horiz_bw,
  //                        (Notify_func)timer_func,
  //                        ITIMER_REAL, &timer, NULL); /*  */
}

/**********************************************************************
 * START_TIMER:  Start up the interval timer
 */

void start_timer()
{

  struct itimerval timer;
  
  if(redraw_interv == 0) {
    redraw_interv = _params.redraw_interval;
    update_interv = _params.update_interval;
  }

  /* set up interval timer interval */
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 10000;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 10000;
  
  if(gd.debug) fprintf(stderr,"Starting Interval timer\n"); 
  if(gd.debug) fprintf(stderr,"tv.sec: %ld\n", timer.it_value.tv_sec); 
  if(gd.debug) fprintf(stderr,"tv.usec: %ld\n", timer.it_value.tv_usec); 
  
  /* Set the interval timer function and start timer */
  // notify_set_itimer_func(gd.h_win_horiz_bw->horiz_bw,
  //                        (Notify_func)timer_func,
  //                        ITIMER_REAL, &timer, NULL); /*  */

  gd.coord_expt->shmem_ready = 1;  // Display should be up and running */

}

