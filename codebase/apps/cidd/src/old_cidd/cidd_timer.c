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
/**********************************************************************
 * CIDD_TIMER.C:  Routines that initialize and control timed based events
 *
 *
 * For the Cartesian Radar Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 *
 */

#include <X11/Xlib.h>

#define RD_TIMER 1
#include "cidd.h"

#include <toolsa/utim.h>
#include <rapplot/xrs.h>


static int    redraw_interv = 0;
static int    update_interv = 0;
static int    update_due = 0;
static int    h_copy_flag = 0;
static int    v_copy_flag = 0;
 
void update_ticker(time_t cur_time);
/************************************************************************
 * TIMER_FUNC: This routine supervises animation and rendering operations
 *
 */

void timer_func(client,which)
    Notify_client   client;
    int which;
{
    int    i;
    met_record_t *mr;
    int    index,flag = 0;
    int    msec_diff = 0;
    int    msec_delay = 0;
    long   tm = 0;
     
    Pixmap    h_xid = 0;
    Pixmap    v_xid = 0;
    char    label[32];
     
    struct itimerval timer;
    struct timezone cur_tz;
    static  struct timeval cur_tm;
    static  struct timeval last_frame_tm = {0,0};
    static  struct timeval last_dcheck_tm = {0,0};
    static int h_gather,v_gather;
    static time_t last_tick = 0;

    extern void check_for_io(); 
    
    PMU_auto_register("In Event loop (OK)");
    
    if(gd.io_info.outstanding_request) check_for_io();
    
    /**** Get present frame index *****/
    if (gd.movie.cur_frame < 0) {
        index = gd.movie.first_index +  gd.movie.num_frames - 1;
    } else {
        index = gd.movie.cur_frame + gd.movie.first_index;
    }
    if (index >= MAX_FRAMES) index -= MAX_FRAMES;

    gettimeofday(&cur_tm,&cur_tz);

	/* Update the current time ticker if necessary */
	if(cur_tm.tv_sec > last_tick) {
		update_ticker(cur_tm.tv_sec);
		last_tick = cur_tm.tv_sec;
	}

    msec_delay = gd.movie.display_time_msec;
     
    /* check for incomming events */
    switch(gd.coord_expt->client_event) {
	default: /* Do nothing */
	    gd.coord_expt->epoch_start = gd.movie.start_time.unix_time;
	    gd.coord_expt->epoch_end = gd.movie.start_time.unix_time + ((gd.movie.num_frames +1) * gd.movie.time_interval * 60.0);
            gd.coord_expt->time_min = gd.movie.frame[index].time_start.unix_time;
            gd.coord_expt->time_max = gd.movie.frame[index].time_end.unix_time;
            if(gd.movie.movie_on) {
	       gd.coord_expt->time_cent = gd.coord_expt->epoch_end;
	    } else {
          gd.coord_expt->time_cent = gd.coord_expt->time_min +
	        (gd.coord_expt->time_max - gd.coord_expt->time_min) / 2;
	    }
	break;

    }

    
    if (gd.movie.movie_on ) {
        flag = 1;        /* set OK state */
        if (gd.movie.frame[index].redraw_horiz != 0) flag = 0;
        if (gd.movie.frame[index].redraw_vert != 0 && gd.v_win.active) flag = 0;

	msec_diff = ((cur_tm.tv_sec - last_frame_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_frame_tm.tv_usec) / 1000);
        if (flag && msec_diff > gd.movie.display_time_msec) {
            /* Advance Movie frame */
            gd.movie.cur_frame++;    

            /* reset to beginning of the loop if needed */
            if (gd.movie.cur_frame > gd.movie.end_frame) {
		      gd.movie.cur_frame = gd.movie.start_frame -1;
	    }
            
            if (gd.movie.cur_frame == gd.movie.end_frame) {
		            msec_delay = gd.movie.delay;
	    }

            /**** recalc current frame index *****/
            if (gd.movie.cur_frame < 0) {
                index = gd.movie.first_index +  gd.movie.num_frames - 1;
            } else {
                index = gd.movie.cur_frame + gd.movie.first_index;
            }
            if (index >= MAX_FRAMES) index -= MAX_FRAMES;
        }
    } else {
	/* In Archive Mode or Each Frame Mode, if Prod_sel Changes, we have to redraw everything */
	if(gd.movie.mode == MOVIE_TS || gd.prod.product_time_select == 1) { 
	  if( CSPR_check_pq_update()) {
	       if(gd.debug2) fprintf(stderr, "MOVIE OFF: ARCHIVE/EACH FRAME MODE  Mode: PQ has updated:\n");
		  CSPR_clear_update_flag();
		  set_redraw_flags(1,0);
	  }
	}
    }
	

    gd.movie.cur_index = index;

    
    /* Set up convienient pointer to main met record */
    mr = gd.mrec[gd.h_win.field];

    /* Decide which Pixmaps to use for rendering */
    if (gd.movie.movie_on ) {
        /* set to the movie frame Pixmaps */
        h_xid = gd.movie.frame[index].h_xid;
        if (h_xid == 0) {
            if(mr->background_render) {    
                h_xid = gd.h_win.field_xid[gd.h_win.field];
            } else {
                h_xid = gd.h_win.tmp_xid;
            }
        }

        v_xid = gd.movie.frame[index].v_xid;
        if (v_xid == 0) {
            if(mr->background_render) {
                v_xid = gd.v_win.field_xid[gd.v_win.field];
            } else {
                v_xid = gd.v_win.tmp_xid;
            }
        }
         
    } else {
        /* set to the field Pixmaps */
        if(mr->background_render) {
            h_xid = gd.h_win.field_xid[gd.h_win.field];
        } else {
            h_xid = gd.h_win.tmp_xid;
        }

        if(gd.mrec[gd.v_win.field]->background_render) {
            v_xid = gd.v_win.field_xid[gd.v_win.field];
        } else {
            v_xid = gd.v_win.tmp_xid;
        }
    }
    
    
     /******* Handle Real Time Updating  ********/
    switch (gd.movie.mode) {
    case MOVIE_EL :
    case MOVIE_MR :
        if (time_for_a_new_frame()) {
            move_movie_frame_index(); 
	    /* Vectors must be removed from the (currenntly) last frame if the wind_mode > 0 */
	    if(gd.extras.wind_mode && gd.extras.wind_vectors)  {
		 gd.movie.frame[gd.movie.cur_index].redraw_horiz = 1;
	    }

	    /* Move movie loop to the last frame when aging off old movie frames */
	    gd.movie.cur_frame = gd.movie.end_frame;
            goto return_point;
        }

        tm = time(0);
        /* Check for New data */
        if ( tm >= update_due ) {
            /* only on the last frame */
            if (gd.movie.cur_frame == gd.movie.num_frames -1) {
                update_due = tm + update_interv;
                /* look thru all data fields */
                for (i=0; i < gd.num_datafields; i++) {
                    /* if data has expired or field should be updated */
                    if (gd.mrec[i]->expire_time < tm || gd.mrec[i]->background_render) {
                        gd.mrec[i]->h_data_valid = 0;
                        gd.mrec[i]->v_data_valid = 0;
                    }
                }
                for (i=0; i < gd.extras.num_wind_sets; i++ ) {
                    /* Look through wind field data too */
                    if (gd.extras.wind[i].active) {
                        if (gd.extras.wind[i].wind_u->expire_time < tm) {
                            gd.extras.wind[i].wind_u->h_data_valid = 0;
                            gd.extras.wind[i].wind_u->v_data_valid = 0;
                        }
                        if (gd.extras.wind[i].wind_v->expire_time < tm) {
                            gd.extras.wind[i].wind_v->h_data_valid = 0;
                            gd.extras.wind[i].wind_v->v_data_valid = 0;
                        }
                        if (gd.extras.wind[i].wind_w != NULL && gd.extras.wind[i].wind_w->expire_time < tm) {
                            gd.extras.wind[i].wind_w->h_data_valid = 0;
                            gd.extras.wind[i].wind_w->v_data_valid = 0;
                        }
                    }
                }
                if (mr->h_data_valid == 0) {
                     gd.movie.frame[index].redraw_horiz = 1;
                     gd.h_win.redraw[gd.h_win.field] = 1;
		 }
                if (gd.v_win.active && gd.mrec[gd.v_win.field]->v_data_valid == 0)  {
                    gd.movie.frame[index].redraw_vert = 1;
                    gd.v_win.redraw[gd.v_win.field] = 1;
		 }
            }
        }

        break;

    case MOVIE_TS :
        break;

    default:
        fprintf(stderr,
                "Invalid movie mode %d in timer_func\n",
                gd.movie.mode);
        break;
    } 

    
    /***** Handle Field changes *****/
    if (gd.h_win.field != gd.h_win.last_field) {
        if (gd.movie.movie_on ) {
            set_redraw_flags(1,0);
        } else {
            if (gd.h_win.redraw[gd.h_win.field] == 0) {
                h_copy_flag = 1;
                gd.h_win.last_field = gd.h_win.field;
            }
        }
    }
    if (gd.v_win.field != gd.v_win.last_field ) {
        if (gd.movie.movie_on ) {
            set_redraw_flags(0,1);
        } else {
            if (gd.v_win.redraw[gd.v_win.field] == 0) {
                v_copy_flag = 1;
                gd.v_win.last_field = gd.v_win.field;
            }
        }
    }


    

    /******** Handle Frame changes ********/
    if (gd.movie.last_frame != gd.movie.cur_frame && gd.movie.cur_frame >= 0) {
        reset_data_valid_flags(1,1);

        /* Move the indicators */
        xv_set(gd.movie_pu->movie_frame_sl,
               PANEL_VALUE, gd.movie.cur_frame + 1,
               NULL);
        if (gd.movie.mode == MOVIE_EL) {
            xv_set(gd.h_win_horiz_bw->slice_sl, PANEL_VALUE, gd.movie.cur_frame, NULL);
        }
         
        if(gd.debug2) {
            printf("Moved movie frame, index : %d\n",index);
        }
        gd.coord_expt->time_min = gd.movie.frame[index].time_start.unix_time;
        gd.coord_expt->time_max = gd.movie.frame[index].time_end.unix_time;
        if(gd.movie.movie_on) {
	   gd.coord_expt->time_cent = gd.coord_expt->epoch_end;
	} else {
          gd.coord_expt->time_cent = gd.coord_expt->time_min +
	        (gd.coord_expt->time_max - gd.coord_expt->time_min) / 2;
	}

        /* Change Labels on Frame Begin, End messages */
        sprintf(label,"Frame %2d: %2d:%02d", gd.movie.cur_frame + 1,
            gd.movie.frame[index].time_mid.hour,
            gd.movie.frame[index].time_mid.min);
        xv_set(gd.movie_pu->fr_begin_msg, PANEL_LABEL_STRING, label, NULL);
        xv_set(gd.h_win_horiz_bw->movie_frame_msg, PANEL_LABEL_STRING, label, NULL);
		 
        if (gd.movie.frame[index].redraw_horiz == 0) {
            /* Get Frame */
            retrieve_h_movie_frame(index,h_xid);
            h_copy_flag = 1;
        } else {
            /* Initiate the  Gathering of products */
            CSPR_calc_and_set_time();

	}

        if (gd.v_win.active && gd.movie.frame[index].redraw_vert == 0) {
            retrieve_v_movie_frame(index,v_xid);
            v_copy_flag = 1;
        }
        gd.movie.last_frame = gd.movie.cur_frame;

    }


    /* Draw Selected field - Horizontal for this movie frame */
    if (gd.movie.frame[index].redraw_horiz) {
        /* Draw Frame */ 
        if (gather_hwin_data(gd.h_win.field, gd.movie.frame[index].time_start.unix_time,
                             gd.movie.frame[index].time_end.unix_time) == SUCCESS) {
            if (gd.h_win.redraw[gd.h_win.field]) {
                render_h_movie_frame(index,h_xid);
                save_h_movie_frame(index,h_xid,gd.h_win.field);
            } 

	    /* make sure the horiz window's slider has the correct label */
	    sprintf(label,"%5.1f %s",mr->vert[mr->plane].cent,
			       mr->units_label_sects);
	    xv_set(gd.h_win_horiz_bw->cur_ht_msg, PANEL_LABEL_STRING, label, NULL);

            gd.movie.frame[index].redraw_horiz = 0;
            gd.h_win.redraw[gd.h_win.field] = 0;
            h_copy_flag = 1;
        }
    }
             
    
    /* Draw Selected field - Vertical  for this movie frame */
    if (gd.v_win.active) {
        if (gd.movie.frame[index].redraw_vert) {
            if (gather_vwin_data(gd.v_win.field,gd.movie.frame[index].time_start.unix_time,
                                 gd.movie.frame[index].time_end.unix_time) == SUCCESS) {
                if (gd.v_win.redraw[gd.v_win.field]) {
                    render_v_movie_frame(index,v_xid);
                    save_v_movie_frame(index,v_xid);
                } 
                gd.movie.frame[index].redraw_vert = 0;
                gd.v_win.redraw[gd.v_win.field] = 0;
                v_copy_flag = 1;
            }
        }
    }
         
    
    /***** Selected Field or movie frame has changed - Copy background image onto visible canvas *****/
    if (h_copy_flag) {
      int draw_prod_queue;
      
        if (gd.debug2) fprintf(stderr,"\nCopying Horiz grid image - field %d, index %d xid: %d to xid: %d\n",
				gd.h_win.field,index,h_xid,gd.h_win.can_xid);
    
        XCopyArea(gd.dpy,h_xid,
                gd.h_win.can_xid,
                gd.def_gc,    0,0,
                gd.h_win.can_dim.width,
                gd.h_win.can_dim.height,
                gd.h_win.can_dim.x_pos,
                gd.h_win.can_dim.y_pos);
        gd.h_win.last_field = gd.h_win.field;
        h_copy_flag = 0;

        if (gd.zoom_in_progress) redraw_zoom_box();
        if (gd.pan_in_progress) redraw_pan_line();
        if (gd.rhi_in_progress) redraw_rhi_line();

	  PMU_auto_register("Rendering Products (OK)");

      if(gd.movie.mode == MOVIE_MR && gd.prod.product_time_select == 0) {
	     if(gd.debug2) fprintf(stderr,"Rendering products onto Mid pixmap\n");
	     render_horiz_products(gd.h_win.can_xid,
              gd.movie.frame[gd.movie.cur_frame].time_start.unix_time,
              gd.movie.frame[gd.movie.cur_frame].time_end.unix_time);
	  }

        if (gd.debug2)
	  fprintf(stderr,
	    "\nTimer: Displaying Horiz final image - field %d, index %d xid: %d to xid: %d\n",
		gd.h_win.field,index,gd.h_win.can_xid,gd.h_win.vis_xid);

        /* Now copy last stage pixmap to visible pixmap */
        XCopyArea(gd.dpy,gd.h_win.can_xid,
            gd.h_win.vis_xid,
            gd.def_gc,    0,0,
            gd.h_win.can_dim.width,
            gd.h_win.can_dim.height,
            gd.h_win.can_dim.x_pos,
            gd.h_win.can_dim.y_pos);

        if (gd.zoom_in_progress) redraw_zoom_box();
        if (gd.pan_in_progress) redraw_pan_line();
        if (gd.rhi_in_progress) redraw_rhi_line();


        /* keep track of how much time will elapse showing the current image */
        gettimeofday(&last_frame_tm,&cur_tz);
	 
        if (gd.drawing_mode) redraw_expt_image();
	}

    if (gd.v_win.active && v_copy_flag) {
        if (gd.debug2) fprintf(stderr,"\nCopying Vert grid image - field %d, index %d xid: %d to xid: %d\n",
				gd.v_win.field,index,v_xid,gd.v_win.can_xid);
    
        XCopyArea(gd.dpy,v_xid,
                gd.v_win.can_xid,
                gd.def_gc,    0,0,
                gd.v_win.can_dim.width,
                gd.v_win.can_dim.height,
                gd.v_win.can_dim.x_pos,
                gd.v_win.can_dim.y_pos);
        gd.v_win.last_field  = gd.v_win.field;
        v_copy_flag = 0;

        if (gd.debug2) fprintf(stderr,"\nDisplaying Vert final image - field %d, index %d xid: %d to xid: %d\n",
				gd.v_win.field,index,gd.v_win.can_xid,gd.v_win.vis_xid);

        /* Now copy last stage pixmap to visible pixmap */
        XCopyArea(gd.dpy,gd.v_win.can_xid,
            gd.v_win.vis_xid,
            gd.def_gc,    0,0,
            gd.v_win.can_dim.width,
            gd.v_win.can_dim.height,
            gd.v_win.can_dim.x_pos,
            gd.v_win.can_dim.y_pos);

	rd_h_msg(gd.frame_label,-1);
    }

    
    /***** Handle redrawing background images *****/
    msec_diff = ((cur_tm.tv_sec - last_dcheck_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_dcheck_tm.tv_usec) / 1000);

    if (msec_diff < 0 ||  msec_diff > redraw_interv  && gd.movie.movie_on == 0) {
        check_for_invalid_images(index);

        /* keep track of how much time will elapse since the last check */
        gettimeofday(&last_dcheck_tm,&cur_tz);
	 
    }

return_point:

    
    if (gd.movie.movie_on == 0) {
        if (gd.zoom_in_progress) redraw_zoom_box();
        if (gd.pan_in_progress) redraw_pan_line();
        if (gd.rhi_in_progress) redraw_rhi_line();

	
        if(gd.movie.mode == MOVIE_MR) update_live_products();     /* Rerenders products that have updated */
	    poll_expt(); /* Poll for changes in expt - the drawing utility */
        redraw_expt_image();

        if (gd.zoom_in_progress) redraw_zoom_box();
        if (gd.pan_in_progress) redraw_pan_line();
        if (gd.rhi_in_progress) redraw_rhi_line();

    } else {
        gettimeofday(&cur_tm,&cur_tz);
        msec_diff = ((cur_tm.tv_sec - last_frame_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_frame_tm.tv_usec) / 1000);
        msec_delay = msec_delay - msec_diff;
        if(msec_delay <= 0 || msec_delay > 10000) msec_delay = 100;
    }

    
    /* set up interval timer interval */
    timer.it_value.tv_sec = msec_delay / 1000;
    timer.it_value.tv_usec = ((msec_delay  - (timer.it_value.tv_sec * 1000)) * 1000);
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

 /* Set the interval timer function and start timer */
    notify_set_itimer_func(gd.h_win_horiz_bw->horiz_bw,
                           (Notify_func)timer_func,
                           ITIMER_REAL, &timer, NULL); /*  */

    
}

/************************************************************************
 * UPDATE_TICKER: Update the current time ticker if mapped - otherwise
 *   Make sure the correct panels are mapped
 */

void
update_ticker(time_t cur_time) 
{
    struct tm *loc_tm;
    char time_string[64];
    struct stat s;
    char *ptr;
    FILE *sfile;

    if(gd.run_unmapped) {
		xv_set(gd.h_win_horiz_bw->horiz_bw,XV_SHOW,FALSE,NULL);
	} else {
	   loc_tm = localtime(&cur_time);

	   strftime(time_string,64,"%T %Z",loc_tm);

	   xv_set(gd.h_win_horiz_bw->cur_time_msg,
		   PANEL_LABEL_STRING,time_string,NULL);
	}

	/* Update the NO DATA Status messages if the file was defined */
	if(gd.status.status_fname != NULL) { /* If its defined */

		if(stat(gd.status.status_fname,&s) >= 0) { /* And it exists */

			  if(s.st_mtime > gd.status.last_accessed) { /* And has been updated */

			      if((sfile = fopen(gd.status.status_fname,"r")) != NULL) { /* And can be read */

					  /* Clear out the old one */
				      memset((void *) gd.status.stat_msg,0,TITLE_LENGTH);
					  /* Get the first line of the file */
					  fgets(gd.status.stat_msg,TITLE_LENGTH,sfile);
					  fclose(sfile);
					  /* Strip off the newline */
					  if((ptr = strchr(gd.status.stat_msg,'\n')) != NULL)
					      *ptr = '\0';
					  set_redraw_flags(1,0); /* force a redraw */
					  /* gd.no_data_message = gd.status.stat_msg; */
					  gd.status.last_accessed = s.st_mtime;
				  }
			  }
		}
	}
}
 
/************************************************************************
 * CHECK_FOR_INVALID_IMAGES: Check for images in which the data 
 * are no longer valid. Look for the "best" invalid  image to 
 *  render.
 *
 */

void check_for_invalid_images(index)
    int    index;    /* the current frame index */
{
    int    i;
    int    h_image,v_image;
    int    stat;
    int    none_found = 1;
    Drawable    xid;

    h_image = gd.h_win.field + 1;
    v_image = gd.v_win.field + 1;
    PMU_auto_register("Checking Images (OK)");

    /* look through the rest of the images  */
    for (i=0; i < gd.num_datafields-1; i++) {    
        
        /*
         * Render horizontal image, if necessary.
         */

        if (h_image >= gd.num_datafields) h_image = 0;

        if (gd.mrec[h_image]->currently_displayed && gd.mrec[h_image]->background_render) {
            
            if (gd.h_win.redraw[h_image] || (gd.mrec[h_image]->h_data_valid == 0)) {
		none_found = 0;
                stat = gather_hwin_data(h_image,
                                        gd.movie.frame[index].time_start.unix_time,
                                        gd.movie.frame[index].time_end.unix_time);
                if (stat == SUCCESS) {
                    if(gd.mrec[h_image]->background_render) {
                        xid = gd.h_win.field_xid[h_image];
                    } else {
                        xid = gd.h_win.tmp_xid;
                    }
                    render_horiz_display(xid,h_image,
			gd.movie.frame[index].time_start.unix_time,
                        gd.movie.frame[index].time_end.unix_time);

		    save_h_movie_frame(index,xid,h_image);

                    gd.h_win.redraw[h_image] = 0;
                } else {
                    return;
                }
                if (h_image == gd.h_win.last_field && gd.h_win.redraw[h_image] == 0) h_copy_flag = 1;
            } 
        }
        h_image++;

        /*
         * Render vertical image, if necessary.
         */

        if (v_image >= gd.num_datafields) v_image = 0;

        if (gd.mrec[v_image]->currently_displayed && gd.mrec[v_image]->background_render) {
            if ((gd.v_win.active) && (gd.v_win.redraw[v_image] || (gd.mrec[v_image]->v_data_valid == 0))) {
                stat = gather_vwin_data(v_image, gd.movie.frame[index].time_start.unix_time,
                                        gd.movie.frame[index].time_end.unix_time);
                if (stat == SUCCESS) {
                    if(gd.mrec[v_image]->background_render) {
                        xid = gd.v_win.field_xid[v_image];
                    } else {
                        xid = gd.v_win.tmp_xid;
                    }
                    render_vert_display(xid,v_image, gd.movie.frame[index].time_start.unix_time,
                                        gd.movie.frame[index].time_end.unix_time);    
                    gd.v_win.redraw[v_image] = 0;
                } else {
                    return;
                }
                if (v_image == gd.v_win.last_field && gd.v_win.redraw[v_image] == 0) v_copy_flag = 1;
            }
        }
        
        v_image++;
    }

    /* if all background images have been rendered. and nothing else is
        happening
    */
    if(none_found && gd.html_mode && gd.io_info.outstanding_request == 0) {

	/* If more zoom levels to render */
	if(gd.h_win.zoom_level < (gd.h_win.num_zoom_levels -  NUM_CUSTOM_ZOOMS -1)) {

	     /* Set zoom to next level */
	     gd.h_win.zoom_level++;
	     set_domain_proc(gd.zoom_pu->domain_st,gd.h_win.zoom_level,NULL);
	}
    }
    return;
}

/**********************************************************************
 * START_TIMER:  Start up the interval timer
 */

void start_timer()
{
    struct  itimerval   timer;

    if(redraw_interv == 0) {
        redraw_interv = XRSgetLong(gd.cidd_db, "cidd.redraw_interval", REDRAW_INTERVAL);
        update_interv =  XRSgetLong(gd.cidd_db, "cidd.update_interval", UPDATE_INTERVAL);
    }
 
    /* set up interval timer interval */
    timer.it_value.tv_sec = 3;
    timer.it_value.tv_usec = 1000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000;

    if(gd.debug) fprintf(stderr,"Starting Interval timer\n"); 

    /* Set the interval timer function and start timer */
    notify_set_itimer_func(gd.h_win_horiz_bw->horiz_bw,
                           (Notify_func)timer_func,
                           ITIMER_REAL, &timer, NULL); /*  */

}

/************************************************************************
 * MOVE_MOVIE_FRAME_INDEX: Advance the frame index
 *
 */

int move_movie_frame_index()
{
    int    index,new_index;
    Pixmap    p_xid;

    UTIMstruct     temp_utime;
     
    /* find end frame index */
    index = gd.movie.first_index +  gd.movie.num_frames - 1;
    if (index >= MAX_FRAMES) index -= MAX_FRAMES;

    new_index = index +1;
    if (new_index >= MAX_FRAMES) new_index -= MAX_FRAMES;

    /* move the pixmaps to the new frame */
    p_xid = gd.movie.frame[gd.movie.first_index].h_xid;
    gd.movie.frame[gd.movie.first_index].h_xid = 0;
    gd.movie.frame[new_index].h_xid = p_xid;

    p_xid = gd.movie.frame[gd.movie.first_index].v_xid;
    gd.movie.frame[gd.movie.first_index].v_xid = 0;
    gd.movie.frame[new_index].v_xid = p_xid;

    gd.movie.first_index++;    /* discard the old frame */
    if (gd.movie.first_index >= MAX_FRAMES)
        gd.movie.first_index -= MAX_FRAMES;

    /* increment start time */
    gd.movie.start_time.unix_time += (gd.movie.time_interval * 60);

    reset_time_points();

    UTIMunix_to_date(gd.movie.start_time.unix_time, &gd.movie.start_time);

    update_movie_popup(1);


    gd.movie.frame[new_index].redraw_horiz = 1;
    gd.movie.frame[new_index].redraw_vert = 1;

    if (gd.movie.reset_frames) {
        set_redraw_flags(1, 1);
        reset_data_valid_flags(1, 1);
    }

    /* restarts the sequence of rendering each field at each zoom state */
    if(gd.html_mode) {
	set_domain_proc(gd.zoom_pu->domain_st,0,NULL); /* reset domain to 1st area */
    }


    return new_index;
    
}

/************************************************************************
 * RESET_TIME_POINTS: Calc the beginning and ending time points for
 *        each movie frame 
 */

reset_time_points()
{
    int    i;
    int    frame;
    long    st_time,e_time;

    UTIMstruct      temp_utime;
    

    if(gd.debug2) {
        printf("Resetting time points - mode: %d\n",gd.movie.mode);
    }

    switch(gd.movie.mode) {
    case MOVIE_MR:
    case MOVIE_TS:
        frame = gd.movie.first_index;
        for (i=0; i < gd.movie.num_frames; i++) {

            st_time = gd.movie.start_time.unix_time + (i * gd.movie.time_interval * 60.0) - (gd.movie.time_interval * 30.0);
            UTIMunix_to_date(st_time, &gd.movie.frame[frame].time_start);

            st_time = gd.movie.start_time.unix_time + (i * gd.movie.time_interval * 60.0);
            UTIMunix_to_date(st_time, &gd.movie.frame[frame].time_mid);

            e_time = gd.movie.frame[frame].time_start.unix_time + (gd.movie.time_interval * 60.0);
            UTIMunix_to_date(e_time, &gd.movie.frame[frame].time_end);

            frame++;

            if (frame >= MAX_FRAMES) frame -= MAX_FRAMES;
        }
        break;

    case MOVIE_EL:
        frame = gd.movie.first_index;
        for (i=0; i < gd.movie.num_frames; i++) {
            st_time = gd.movie.start_time.unix_time;
            UTIMunix_to_date(st_time, &gd.movie.frame[frame].time_start);

            e_time = gd.movie.start_time.unix_time + (gd.movie.time_interval * 60.0);
            UTIMunix_to_date(e_time, &gd.movie.frame[frame].time_end);

            frame++;
            if (frame >= MAX_FRAMES) frame -= MAX_FRAMES;
        }
        break;
    }

    CSPR_calc_and_set_time();
    
}

/**********************************************************************
 * SET_BUSY_STATE: Provide visual feedback that the application is busy
 *
 */

set_busy_state(state)
    int    state;    /* 0 = not busy, 1 = busy */
{
    static int last_state = -1;
     
    if(state == last_state) return;
    last_state = state;
     
    if (state) {
        xv_set(gd.h_win_horiz_bw->horiz_bw,
               FRAME_BUSY, TRUE,
               NULL);
        xv_set(gd.v_win_v_win_pu->v_win_pu,
               FRAME_BUSY, TRUE,
               NULL);
        xv_set(gd.extras_pu->extras_pu,
               FRAME_BUSY, TRUE,
               NULL);
/*        xv_set(gd.movie_pu->movie_pu,
/*               FRAME_BUSY, TRUE,
/*               NULL);
*/
    } else {
        xv_set(gd.h_win_horiz_bw->horiz_bw,
               FRAME_BUSY, FALSE,
               NULL);
        xv_set(gd.v_win_v_win_pu->v_win_pu,
               FRAME_BUSY, FALSE,
               NULL);
        xv_set(gd.extras_pu->extras_pu,
               FRAME_BUSY, FALSE,
               NULL);
/*        xv_set(gd.movie_pu->movie_pu,
/*               FRAME_BUSY, FALSE,
/*               NULL);
*/
    }

}

#ifndef LINT
static char RCS_id[] = "$Id: cidd_timer.c,v 1.47 2016/03/07 18:28:26 dixon Exp $";
#endif /* not LINT */
