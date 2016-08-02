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
 * GRAPHIC_CANVAS_EVENTS.CC:: Event handling for the Horiz view canvas 
 * - BACKWARD COMPATIBILITY VERSION
 * For the Cartesian Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */


#define GRAPHIC_CANVAS_EVENTS_BKWD  1

#include "cidd.h"

extern int    b_lastx,b_lasty;    /* Boundry end point */
extern int    b_startx,b_starty;    /* Boundry start point */
extern int    p_lastx,p_lasty;    /* Pan end point */
extern int    p_startx,p_starty;    /* Pan start point */
extern int    r_lastx,r_lasty;    /* ROUTE end point */
extern int    r_startx,r_starty;    /* ROUTE start point */
extern int    r_state;
/*************************************************************************
 * Event callback function for `canvas1'.
 */
Notify_value can_event_proc_bkwd(Xv_window win, Event *event,
                            Notify_arg arg, Notify_event_type type)
{
    
    int   x_gd,y_gd;            /* Data grid coords */
    int   field;                /* Data field number */
    int   out_of_range_flag;    /* 1 = no such grid point in visible data */
    int   num_fields;         /* num fields in field choice list */
    int   field_index;        /* field index in field choice list */
    int   curr_frame;
    int   max_frame;
    
     
    double    x_dproj,y_dproj;        /* Km coords */
    double    lat,lon;        /* latitude, longitude coords */
    double    dist,theta;    // distance and angle from last click point
    double value;            /* data value after scaling */

    fl32 *ptr;            /* pointer to fl32 data */
    char    text[256];
    char    lat_string[16];
    char    lon_string[16];
    char    dist_string[16];
    char    dir_string[16];
    met_record_t    *mr;

    struct timeval tp;
    struct timezone tzp;

    static int last_frame = -1;
    static time_t lockout_time = 0;  /* Events will be discarded until this passes */
    static double last_lat = 0.0;
    static double last_lon = 0.0;


    /* Move to front of any motion drag events */
    while (XCheckMaskEvent(gd.dpy, PointerMotionMask | ButtonMotionMask, event->ie_xevent))
        /* NULL BODY */;

   field = gd.h_win.page;
   mr = gd.mrec[field];

    ///////REPORT MODE ////////////////////
    if(gd.report_mode) {
	 if(event_id(event) == LOC_MOVE || event_id(event) ==  LOC_WINEXIT) {
	  if( gd.movie.cur_frame == gd.movie.last_frame &&
	    gd.movie.movie_on == 0 &&
	    mr->h_data_valid == 1 &&
	    mr->h_fl32_data != NULL ) {

	   static int report_visible = 0;
	   static int x1 = gd.h_win.can_dim.x_pos;
	   static int y1 = gd.h_win.can_dim.y_pos;
	   static int xwidth = gd.h_win.can_dim.width;
	   static int yheight = gd.h_win.can_dim.height;

	   int len,direct,ascent,descent;
	   int xpos,ypos;
	   XCharStruct overall;

	   xpos = event_x(event);
	   ypos = event_y(event);
           out_of_range_flag = 0;

           pixel_to_grid(mr, &gd.h_win.margin, xpos, ypos, &x_gd, &y_gd);
           x_gd = CLIP(x_gd,0,(mr->h_fhdr.nx -1),out_of_range_flag);
           y_gd = CLIP(y_gd,0,(mr->h_fhdr.ny -1),out_of_range_flag);

	   if(xpos < gd.h_win.img_dim.x_pos || 
	      xpos > gd.h_win.img_dim.x_pos + gd.h_win.img_dim.width ||
	      ypos < gd.h_win.img_dim.x_pos ||
	      ypos > gd.h_win.img_dim.x_pos + gd.h_win.img_dim.height) 
	      out_of_range_flag = 1;

           ptr = mr->h_fl32_data;
	   // Pick out grid pint in plane
	   ptr += (mr->h_fhdr.nx * (y_gd)) + (x_gd);
	   if(report_visible) {
		   XCopyArea(gd.dpy,gd.h_win.can_xid[gd.h_win.cur_cache_im],
		       gd.h_win.vis_xid,
		       gd.def_gc,    x1, y1,
		       xwidth, yheight, x1, y1);
	   }

	   if(event_id(event) ==  LOC_WINEXIT) out_of_range_flag = 1;
           if(out_of_range_flag != 0 ||
	            *ptr == (mr->h_mdvx->getFieldByNum(0))->getFieldHeader().missing_data_value ||
                *ptr == (mr->h_mdvx->getFieldByNum(0))->getFieldHeader().bad_data_value) {
	       report_visible = 0;
           } else {
		  report_visible = 1;
		  if (mr->h_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
				  value = exp((double) *ptr);
		  } else {
		     value =  *ptr;
		  }

          // Deal with values very close to 0
          if(value  < 0.0001 && value  > -0.0001) value = 0.0;

		  sprintf(text,"%g",value);
		  len = strlen(text);
		  XTextExtents(gd.fontst[gd.prod.prod_font_num],text,len,&direct,&ascent,&descent,&overall);
		  XSetFont(gd.dpy,gd.legends.foreground_color->gc,gd.ciddfont[gd.prod.prod_font_num]);
		  XDrawImageString(gd.dpy,gd.h_win.vis_xid,gd.legends.foreground_color->gc,
		                   xpos + 2,
				   ypos-gd.fontst[gd.prod.prod_font_num]->descent + 2,
				   text,len);

		  xwidth =  overall.width;
		  yheight = gd.fontst[gd.prod.prod_font_num]->ascent +
		            gd.fontst[gd.prod.prod_font_num]->descent;

		  x1 = xpos + 2;
		  y1 = ypos - yheight + 2;
	   }
          }
	 }
    }
    /////// End of report Mode   ////////////

    if((gd.zoom_in_progress | gd.route_in_progress | gd.pan_in_progress) == 0)  {
      // Handle Events in Margins in a seperate event handlers 
      if(event_y(event) < gd.h_win.margin.top) {
        top_margin_event(event);
        return notify_next_event_func(win, (Notify_event) event, arg, type);
      }
      if(event_y(event) > gd.h_win.can_dim.height - gd.h_win.margin.bot) {
        bot_margin_event(event);
        return notify_next_event_func(win, (Notify_event) event, arg, type);
      }
      if(event_x(event) < gd.h_win.margin.left) {
        left_margin_event(event);
        return notify_next_event_func(win, (Notify_event) event, arg, type);
      }
      if(event_x(event) > gd.h_win.can_dim.width - gd.h_win.margin.right) {
        right_margin_event(event);
        return notify_next_event_func(win, (Notify_event) event, arg, type);
      }
    }


   /////////////////// HANDLE CANVAS EVENTS ///////////////////

   gettimeofday(&tp, &tzp);                /* record when this event took place */
   gd.last_event_time = tp.tv_sec;

   if(tp.tv_sec <= lockout_time) {
      if(gd.debug) {
          fprintf(stderr,"Events locked out for %ld more seconds\n",
                  ((lockout_time - tp.tv_sec) + 1));
      }

      // Just skip Processing
      return notify_next_event_func(win, (Notify_event) event, arg, type);
   }

        /* Left button up or down  */
        if(event_action(event) == ACTION_SELECT ) {
             // Cancel the last segment if the left key is pressed
            if(gd.route_in_progress && event_is_up(event)) {
                if(gd.h_win.route.num_segments > 1) {
                   XDrawLine(gd.dpy,gd.hcan_xid,gd.ol_gc,r_startx,r_starty,r_lastx,r_lasty);
                   redraw_route_line(&gd.h_win); // clears the line
                   // Keep track of total length;
                   gd.h_win.route.total_length -= gd.h_win.route.seg_length[gd.h_win.route.num_segments -1];

                   gd.h_win.route.num_segments--;
                   r_startx = gd.h_win.route.x_pos[gd.h_win.route.num_segments];
                   r_starty = gd.h_win.route.y_pos[gd.h_win.route.num_segments];

                   r_lastx = r_startx;
                   r_lasty = r_starty;
                   redraw_route_line(&gd.h_win); // redraws the line
                   sprintf(text,"%d Segments - Last pointed Deleted",
                                     gd.h_win.route.num_segments);
                   gui_label_h_frame(text, 1);
                   add_message_to_status_win(text,0);

                } else {
                    redraw_route_line(&gd.h_win); // clears the line
                    gd.h_win.route.num_segments = 0;
                    gd.route_in_progress = 0;
                    r_state = 0;
                    if(gd.drawing_mode == DRAW_FMQ_MODE) {
                        gui_label_h_frame("Draw Mode Canceled", 1);
                        add_message_to_status_win("Draw Mode Canceled",0);
                    } else {
                        gui_label_h_frame("Route Define Canceled", 1);
                        add_message_to_status_win("Route Define Canceled",0);
                    }
                    set_redraw_flags(1,0);
                }
            } else if(event_is_down(event)) {
                gd.zoom_in_progress = 1;
                b_startx = event_x(event);
                b_starty = event_y(event);
                b_lastx = b_startx;
                b_lasty = b_starty;
            } else {  // UP
                gd.zoom_in_progress = 0;

                pixel_to_disp_proj(&gd.h_win.margin, b_lastx, b_lasty, &x_dproj, &y_dproj);

                gd.proj.xy2latlon(x_dproj,y_dproj,lat,lon);

                PJGLatLon2RTheta(last_lat,last_lon,lat,lon,&dist,&theta);
                last_lat = lat;
                last_lon = lon;

                dist *= gd.scale_units_per_km;
                sprintf(dist_string,"%.2f %s/",dist,gd.scale_units_label);

                if(theta < 0.0) theta += 360;
                sprintf(dir_string,"%.0f deg",theta);

                /* Nicely format the lat lons */
                switch(gd.latlon_mode) {
                    default:
                    case 0:  /* Decimal Degrees */
		        if (lat > 90.0) {
			  sprintf(lat_string, ">90");
			} else if (lat < -90.0) {
			  sprintf(lat_string, "<90");
			} else {
			  sprintf(lat_string,"%.4f",lat);
			}
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
		       if (lat > 90.0) {
			 sprintf(lat_string, ">90");
		       } else if (lat < -90.0) {
			 sprintf(lat_string, "<90");
		       } else {
			 sprintf(lat_string,"%d %d\' %.0f\"",ideg,imin,fsec);
		       }

                       ideg = (int) lon;
                       fmin = fabs(lon - ideg); /* extract decimal fraction */
                       imin = (int) (fmin * 60.0);
                       fsec = (fmin - (imin / 60.0)) * 3600.0; 
                       sprintf(lon_string,"%d %d\' %.0f\"",ideg,imin,fsec);
                       }
                    break;
                }


                if ((abs(b_lastx - b_startx) > 5 || abs(b_lasty - b_starty) > 5)) {
                    do_zoom();

                    gd.coord_expt->button = event->ie_xevent->xbutton.button;
                    gd.coord_expt->selection_sec = tp.tv_sec;
                    gd.coord_expt->selection_usec = tp.tv_usec;
                    gd.coord_expt->pointer_x = (gd.h_win.cmin_x + gd.h_win.cmax_x) / 2.0;
                    gd.coord_expt->pointer_y = (gd.h_win.cmin_y + gd.h_win.cmax_y) / 2.0;

                    gd.proj.xy2latlon(gd.coord_expt->pointer_x,gd.coord_expt->pointer_y,lat,lon);

                    gd.coord_expt->pointer_lon = lon;
                    gd.coord_expt->pointer_lat = lat;
                    gd.coord_expt->pointer_alt_min = gd.h_win.cur_ht;
                    gd.coord_expt->pointer_alt_max = gd.h_win.cur_ht;
                    gd.coord_expt->data_altitude = mr->vert[mr->plane].cent;
                    gd.coord_expt->click_type = CIDD_ZOOM_CLICK;
                    gd.coord_expt->pointer_seq_num++;

                } else {       /* report data value */ 
                    gd.coord_expt->button = event->ie_xevent->xbutton.button;
                    gd.coord_expt->selection_sec = tp.tv_sec;
                    gd.coord_expt->selection_usec = tp.tv_usec;
                    gd.coord_expt->pointer_x = x_dproj;
                    gd.coord_expt->pointer_y = y_dproj;
                    gd.coord_expt->pointer_lon = lon;
                    gd.coord_expt->pointer_lat = lat;
                    gd.coord_expt->pointer_alt_min = gd.h_win.cur_ht;
                    gd.coord_expt->pointer_alt_max = gd.h_win.cur_ht;
                    gd.coord_expt->data_altitude = mr->vert[mr->plane].cent;
                    gd.coord_expt->click_type = CIDD_USER_CLICK;
                    gd.coord_expt->pointer_seq_num++;

                    pixel_to_grid(mr, &gd.h_win.margin, b_lastx, b_lasty, &x_gd, &y_gd);

                    out_of_range_flag = 0;
                    x_gd = CLIP(x_gd,0,(mr->h_fhdr.nx -1),out_of_range_flag);
                    y_gd = CLIP(y_gd,0,(mr->h_fhdr.ny -1),out_of_range_flag);
                    if(out_of_range_flag) {
                        sprintf(text, "X,Y: %.2fkm,%.2fkm  LAT,LON:%s,%s  Outside Data grid",
                                                        x_dproj, y_dproj,
                                                        lat_string,lon_string);
                        // Add the station location
                        if(gd.station_loc != NULL) {
                            strncat(text," : ",256);
                            strncat(text,
                                    gd.station_loc->FindClosest(lat,lon,gd.locator_margin_km).c_str(),
                                    256);
                        }

                        gui_label_h_frame(text,-3);
                        strncat(text,"\n",256);
                        add_message_to_status_win(text,0);
                    } else {
                      if(gd.movie.cur_frame == last_frame) {
                        if(mr->h_fl32_data != NULL) {
                            ptr = mr->h_fl32_data;

			    // Pick out correct grid point
                            ptr += (mr->h_fhdr.nx * (y_gd - 0)) + (x_gd - 0);
                                if( *ptr == (mr->h_mdvx->getFieldByNum(0))->getFieldHeader().missing_data_value ||
                                    *ptr == (mr->h_mdvx->getFieldByNum(0))->getFieldHeader().bad_data_value) {
                                    sprintf(text,
                                    "%s: Bad or Missing Data at X,Y: %.2f, %.2f GX,GY: %d,%d  LAT,LON: %s, %s     Dist: %s%s",
                                    mr->legend_name, 
                                    x_dproj, y_dproj,
									x_gd,y_gd,
                                    lat_string,lon_string,
                                    dist_string,dir_string);
                                } else {

				  // Grab the fl32 data value
				  if (mr->h_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
					 value = exp((double) *ptr);
				  } else {
				     value =  *ptr;
				  }

				  // Deal with (fudge) values very close to 0
				  if(value < 0.0001 && value  > -0.0001) value = 0.0;

                                  if(gd.display_projection != Mdvx::PROJ_LATLON) {
                                    sprintf(text, "%s Value: %g %s   X,Y:%.2f,%.2f  GX,GY: %d,%d  LAT,LON:%s,%s Dist: %s%s",
                                      mr->legend_name,value,mr->field_units, 
                                      x_dproj, y_dproj,
									  x_gd,y_gd,
                                      lat_string,lon_string,
                                      dist_string,dir_string);
                                  } else {  // Is a lat lon projection makeing X,Y redundant
                                    sprintf(text, "%s Value: %g %s GX,GY: %d,%d  LAT,LON:%s,%s    Dist: %s%s",
                                      mr->legend_name,value,mr->field_units, 
									  x_gd,y_gd,
                                      lat_string,lon_string,
                                      dist_string,dir_string);
                                }

                            }

                            // Add the station location
                            if(gd.station_loc != NULL) {
                              strncat(text," : ",256);
                              strncat(text,
                                      gd.station_loc->FindClosest(lat,lon,gd.locator_margin_km).c_str(),
                                      256);
                            }
                            gui_label_h_frame(text,-3);
                            strncat(text,"\n",256);
                            add_message_to_status_win(text,0);
                        }
                      } else {
                            mr->h_data_valid = 0;
                            gather_hwin_data(gd.h_win.page,
                                gd.movie.frame[gd.movie.cur_frame].time_start,
                                gd.movie.frame[gd.movie.cur_frame].time_end);
                                
                            gui_label_h_frame("Gathering correct data - Please Click Again",-1);
                            add_message_to_status_win("Gathering correct data - Please Click Again",0);
                            last_frame = gd.movie.cur_frame;
			    gd.movie.last_frame = gd.movie.cur_frame;
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

                    pixel_to_disp_proj(&gd.h_win.margin, p_lastx, p_lasty, &x_dproj, &y_dproj);

                    gd.proj.xy2latlon(x_dproj,y_dproj,lat,lon);
                

                } else {
                    if (abs(p_lastx - p_startx) > 5 || abs(p_lasty - p_starty) > 5) {
                       do_zoom_pan();

                       gd.coord_expt->button = event->ie_xevent->xbutton.button;
                       gd.coord_expt->selection_sec = tp.tv_sec;
                       gd.coord_expt->selection_usec = tp.tv_usec;
                       gd.coord_expt->pointer_x = (gd.h_win.cmin_x + gd.h_win.cmax_x) / 2.0;
                       gd.coord_expt->pointer_y = (gd.h_win.cmin_y + gd.h_win.cmax_y) / 2.0;

                       gd.proj.xy2latlon(gd.coord_expt->pointer_x,gd.coord_expt->pointer_y,lat,lon);

                       gd.coord_expt->pointer_lon = lon;
                       gd.coord_expt->pointer_lat = lat;
                       gd.coord_expt->pointer_alt_min = gd.h_win.cur_ht;
                       gd.coord_expt->pointer_alt_max = gd.h_win.cur_ht;
                       gd.coord_expt->data_altitude = mr->vert[mr->plane].cent;
                       gd.coord_expt->click_type = CIDD_ZOOM_CLICK;
                       gd.coord_expt->pointer_seq_num++;

                    } else {

                       gd.coord_expt->button = event->ie_xevent->xbutton.button;
                       gd.coord_expt->selection_sec = tp.tv_sec;
                       gd.coord_expt->selection_usec = tp.tv_usec;
                       gd.coord_expt->pointer_x = x_dproj;
                       gd.coord_expt->pointer_y = y_dproj;
                       gd.coord_expt->pointer_lon = lon;
                       gd.coord_expt->pointer_lat = lat;
                       gd.coord_expt->pointer_alt_min = gd.h_win.cur_ht;
                       gd.coord_expt->pointer_alt_max = gd.h_win.cur_ht;
                       gd.coord_expt->data_altitude = mr->vert[mr->plane].cent;
                       gd.coord_expt->click_type = CIDD_USER_CLICK;
                       gd.coord_expt->pointer_seq_num++;

                       gd.save_im_win = PLAN_VIEW;
                       update_save_panel();
                       if(gd.enable_save_image_panel)
			   xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, TRUE,XV_SHOW, TRUE,NULL);
                                        }
                    gd.pan_in_progress = 0;
                }
            }
        }


     
        /* RIGHT Button up or down  */
        if (event_action(event) == ACTION_MENU ) {
            if(event_is_down(event)) {
                switch(r_state) {
                  default:
                  case 0:  // Start anew
                    r_startx = event_x(event);
                    r_starty = event_y(event);
                    r_lastx = r_startx;
                    r_lasty = r_starty;
                    gd.h_win.route.num_segments = 0;
                    gd.h_win.route.total_length = 0;
                    r_state = 1; // Initial Press
                    gd.route_in_progress = 1;
                    if(gd.drawing_mode == DRAW_FMQ_MODE) {
                        gui_label_h_frame("Draw Mode", 1);
                        add_message_to_status_win("Draw Mode",0);
                    } else {
                        gui_label_h_frame("Define Route Mode", 1);
                        add_message_to_status_win("Define Route Mode",0);
                    }
                  break;

                  case 3:
                    if ((abs(r_lastx - event_x(event)) > 5 || abs(r_lasty - event_y(event)) > 5)) {
                       r_state = 2;
                       r_startx = r_lastx; // pick up starting point from last segment
                       r_starty = r_lasty;
                       r_lastx = event_x(event);
                       r_lasty = event_y(event);
                       XDrawLine(gd.dpy,gd.hcan_xid,gd.ol_gc,r_startx,r_starty,r_lastx,r_lasty);
                    } else {
                       r_state = 4;
                    }
                  break;
                }


                pixel_to_disp_proj(&gd.h_win.margin, r_lastx, r_lasty, &x_dproj, &y_dproj);

				// Sanitize Full earth domain limits.
				if (gd.display_projection == Mdvx::PROJ_LATLON) {
				    if(x_dproj  < -360.0) x_dproj = -360.0;
				    if(x_dproj  > 360.0) x_dproj = 360.0;
				    if(y_dproj  < -85.0) y_dproj = -85.0;
				    if(y_dproj  > 85.0) y_dproj = 85.0;
				}

                PJGLatLonPlusDxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,x_dproj,y_dproj,&lat,&lon);
                

            } else { // - BUTTON UP
                  r_lastx = event_x(event);
                  r_lasty = event_y(event);
                  switch(r_state) {
                    default:
                    case 1: // button up with no movement
                      r_state = 0;
                      gd.route_in_progress = 0;
                    break;

                    case 2: // Was just dragging
                      if ((abs(r_lastx - r_startx) > 5 || abs(r_lasty - r_starty) > 5)) {
                        if(gd.h_win.route.num_segments < MAX_ROUTE_SEGMENTS -1) {
			  gd.h_win.route.num_segments++;
			  // fprintf(stderr,"Num Segments: %d\n",gd.h_win.route.num_segments);
			}
			if( gd.one_click_rhi == 0 ) {
                          sprintf(text,"%d Segments Entered - Click to extend - twice at end to finish",
                                     gd.h_win.route.num_segments);
                          gui_label_h_frame(text, 1);
                          add_message_to_status_win(text,0);
			}

                        if(gd.h_win.route.num_segments <= 1) {
                           pixel_to_disp_proj(&gd.h_win.margin, r_startx, r_starty,
                               &gd.h_win.route.x_world[0],
                               &gd.h_win.route.y_world[0]);

						   if (gd.display_projection == Mdvx::PROJ_LATLON) {
						    if(gd.h_win.route.x_world[0]  < -360.0) gd.h_win.route.x_world[0] = -360.0;
						    if(gd.h_win.route.x_world[0]  > 360.0) gd.h_win.route.x_world[0] = 360.0;
						    if(gd.h_win.route.y_world[0]  < -85) gd.h_win.route.y_world[0] = -85;
						    if(gd.h_win.route.y_world[0]  > 85) gd.h_win.route.y_world[0] = 85;
						  }

                          gd.h_win.route.x_pos[0] = r_startx;
                          gd.h_win.route.y_pos[0] = r_starty;
                        }

                        // Compute new segment length
                        pixel_to_disp_proj(&gd.h_win.margin, r_lastx, r_lasty,
                               &gd.h_win.route.x_world[gd.h_win.route.num_segments],
                               &gd.h_win.route.y_world[gd.h_win.route.num_segments]);

						 // Sanitize Full earth domain limits.
						 if (gd.display_projection == Mdvx::PROJ_LATLON) {
						    if(gd.h_win.route.x_world[gd.h_win.route.num_segments]  < -360.0)
						        gd.h_win.route.x_world[gd.h_win.route.num_segments] = -360.0;
						    if(gd.h_win.route.x_world[gd.h_win.route.num_segments]  > 360.0)
						        gd.h_win.route.x_world[gd.h_win.route.num_segments] = 360.0;
						    if(gd.h_win.route.y_world[gd.h_win.route.num_segments]  < -85)
						        gd.h_win.route.y_world[gd.h_win.route.num_segments] = -85;
						    if(gd.h_win.route.y_world[gd.h_win.route.num_segments]  > 85)
						        gd.h_win.route.y_world[gd.h_win.route.num_segments] = 85;
						}

                        gd.h_win.route.seg_length[gd.h_win.route.num_segments -1] =
                                disp_proj_dist(gd.h_win.route.x_world[gd.h_win.route.num_segments],
                                           gd.h_win.route.y_world[gd.h_win.route.num_segments],
                                           gd.h_win.route.x_world[gd.h_win.route.num_segments-1],
                                           gd.h_win.route.y_world[gd.h_win.route.num_segments-1]);

                        // Keep track of total length;
                        gd.h_win.route.total_length +=
                           gd.h_win.route.seg_length[gd.h_win.route.num_segments -1];

                        gd.h_win.route.x_pos[gd.h_win.route.num_segments] = r_lastx;
                        gd.h_win.route.y_pos[gd.h_win.route.num_segments] = r_lasty;

			// Collect/Generate a waypoint label
			if(gd.station_loc != NULL) {
                            const char    *txt_ptr;
			    // Next to last point
			    gd.proj.xy2latlon(gd.h_win.route.x_world[gd.h_win.route.num_segments-1],
					     gd.h_win.route.y_world[gd.h_win.route.num_segments-1],
					     lat,lon); 
                            txt_ptr = gd.station_loc->FindClosest(lat,lon,gd.locator_margin_km).c_str();
			    if(strlen(txt_ptr) ==0)  {
				sprintf(gd.h_win.route.navaid_id[gd.h_win.route.num_segments-1],"%d",gd.h_win.route.num_segments);
			    } else {
				strncpy(gd.h_win.route.navaid_id[gd.h_win.route.num_segments-1],txt_ptr,16);
			    }

			    //  last point
			    gd.proj.xy2latlon(gd.h_win.route.x_world[gd.h_win.route.num_segments],
					     gd.h_win.route.y_world[gd.h_win.route.num_segments],
					     lat,lon); 
                            txt_ptr = gd.station_loc->FindClosest(lat,lon,gd.locator_margin_km).c_str();
			    if(strlen(txt_ptr) ==0)  {
				sprintf(gd.h_win.route.navaid_id[gd.h_win.route.num_segments],"%d",gd.h_win.route.num_segments+1);
			    } else {
				strncpy(gd.h_win.route.navaid_id[gd.h_win.route.num_segments],txt_ptr,16);
			    }
			} else {
			    sprintf(gd.h_win.route.navaid_id[gd.h_win.route.num_segments-1],"%d",gd.h_win.route.num_segments);
			    sprintf(gd.h_win.route.navaid_id[gd.h_win.route.num_segments],"%d",gd.h_win.route.num_segments+1);
			}

			/*   DEBUG
			fprintf(stderr,"navaid_id %d: %s  %d: %s\n",
				gd.h_win.route.num_segments,
				gd.h_win.route.navaid_id[gd.h_win.route.num_segments-1],
				gd.h_win.route.num_segments+1,
				gd.h_win.route.navaid_id[gd.h_win.route.num_segments]);
			*/
			if(gd.drawing_mode != DRAW_FMQ_MODE && gd.one_click_rhi != 0 ) {
                          setup_route_area();
                          strncpy(gd.h_win.route.route_label,"Custom",7);

                          // Copy the route definition into the space reserved for the Custom route 
                          if(gd.layers.route_wind.num_predef_routes > 0 ) {
                            memcpy(gd.layers.route_wind.route+gd.layers.route_wind.num_predef_routes,
                                 &gd.h_win.route,sizeof(route_track_t));
                          }

                          xv_set(gd.route_pu->route_st,PANEL_VALUE,gd.layers.route_wind.num_predef_routes,NULL);
                          gd.route_in_progress = 0;
                          lockout_time = tp.tv_sec+1;  // Lockout for at most 1  seconds
                          r_state = 0;

			} else {

                          r_state = 3;
			}

                      } else { // Only small change after dragging

                        gd.coord_expt->button = event->ie_xevent->xbutton.button;
                        gd.coord_expt->selection_sec = tp.tv_sec;
                        gd.coord_expt->selection_usec = tp.tv_usec;
                        gd.coord_expt->pointer_x = x_dproj;
                        gd.coord_expt->pointer_y = y_dproj;
                        gd.coord_expt->pointer_lon = lon;
                        gd.coord_expt->pointer_lat = lat;
                        gd.coord_expt->pointer_alt_min = gd.h_win.cur_ht;
                        gd.coord_expt->pointer_alt_max = gd.h_win.cur_ht;
                        gd.coord_expt->data_altitude = mr->vert[mr->plane].cent;
                        gd.coord_expt->click_type = CIDD_USER_CLICK;
                        gd.coord_expt->pointer_seq_num++;

                        if(gd.h_win.route.num_segments > 0) {
                          r_state = 3;
                          r_lastx =  r_startx;
                          r_lasty =  r_starty;
                        } else {
                          r_state = 0;
                          gd.route_in_progress = 0;
                          gui_label_h_frame("ROUTE Define Canceled", 1);
                          add_message_to_status_win("ROUTE Define Canceled",0);
                        }
                        
                      }
                    break;


                    case 4: // OK - ALL DONE 
                      if(gd.drawing_mode == DRAW_FMQ_MODE) {
                          show_draw_panel(1);
                      } else {
                          setup_route_area();
                      }
                      r_state = 0;
                      strncpy(gd.h_win.route.route_label,"Custom",7);

                      // Copy the route definition into the space reserved for the Custom route 
                      if(gd.layers.route_wind.num_predef_routes > 0 ) {
                        memcpy(gd.layers.route_wind.route+gd.layers.route_wind.num_predef_routes,
                             &gd.h_win.route,sizeof(route_track_t));
                      }

                      xv_set(gd.route_pu->route_st,PANEL_VALUE,gd.layers.route_wind.num_predef_routes,NULL);
                      gd.route_in_progress = 0;
                      lockout_time = tp.tv_sec+1;  // Lockout for at most 1  seconds
                    break;

                }

            } // End of Button UP section

        } // End of ACTION_MENU section

        /* Mouse is moved while any button is down - LOC DRAG */
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

            if(event_right_is_down(event))    /* ROUTE Define */ {
                switch(r_state) {
                  default:
                  case 1:
                    redraw_route_line(&gd.h_win); // clears the line
                    r_lastx = event_x(event);
                    r_lasty = event_y(event);
                    r_state = 2;
                    redraw_route_line(&gd.h_win); // sets the line
                  break;

                  case 2:
                    redraw_route_line(&gd.h_win); // clears the line
                    r_lastx = event_x(event);
                    r_lasty = event_y(event);
                    r_state = 2;
                    redraw_route_line(&gd.h_win); // sets the line
                  break;

                  case 4:
                    if(abs(event_x(event) - r_lastx) > 5 || abs(event_y(event) - r_lasty) > 5 ) {
                        r_startx = r_lastx;
                        r_starty = r_lasty;
                        r_lastx = event_x(event);
                        r_lasty = event_y(event);
                        r_state = 2;
                        XDrawLine(gd.dpy,gd.hcan_xid,gd.ol_gc,r_startx,r_starty,r_lastx,r_lasty);
                    }
                  break;
                }
            }
        }


    /*
     * Process keyboard events.  Only process them when the key is
     * released to prevent repeated processing when the key is held
     * down (may want to change this in the future).
     */

    if (event_is_down(event)) {

	//fprintf(stderr,"Event id = %d, XKeycode: %u\n",
	//	event_id(event),event->ie_xevent->xkey.keycode);
	 

	if(event_id(event) == '=') {
		next_cache_image();
	}

	if(event_id(event) == '-') {
		prev_cache_image();
	}

	// Extra Keys - Seen by Xview as event_ID 0
	if(event_id(event) == 0) {
	  int keycode = event->ie_xevent->xkey.keycode;

	  switch (keycode) {
	    case 111: // Print Screen Key
	     startup_snapshot(0);
		break;

		case 79: // Keypad 7
		   rotate_route(-10.0);
		break;

		case 81: // Keypad 9
		   rotate_route(+10.0);
		break;

		case 83: // Keypad 4
		   rotate_route(-1.0);
		break;

		case 85: // Keypad 6
		   rotate_route(+1.0);
		break;

		case 87: // Keypad 1
		   rotate_route(-.1);
		break;

		case 89: // Keypad 3
		   rotate_route(.1);
		break;

	 } // End switch on keycode.

	}  // End of event_ID 0 keys

        num_fields = gd.num_menu_fields;
	field_index = -1;
    
        /*
         * Pressing a number key selects the corresponding field in the field
         * choice list.
         */
	// Fields 1-9
        if (event_id(event) >= '1' && event_id(event) <= '9') {
	    field_index = event_id(event) - '1';
	}

	// Field 10
        if (event_id(event) == '0' ) {
	    field_index = 9;
	}

	// Lower case Letters for fields 10-35
        if (event_id(event) >= 'a' && event_id(event) <= 'z' ) {
            field_index = event_id(event) - 'a' + 10;
        }
	// Cap Letters for fields 36-62
        if (event_id(event) >= 'A' && event_id(event) <= 'Z' ) {
            field_index = event_id(event) - 'A' + 36;
        }

	if(field_index >= 0 && field_index < num_fields) {
            /* simulate the user selecting a new field */
            set_field(field_index);
            set_v_field(gd.field_index[field_index]);
	}

	// FLIP Field is '.' (period)
    if (event_id(event) == '.' ) {
	    set_field(-1);
	    set_v_field(-1);
	}

	// Check for Zoom Commands F1 - F10
	if(event_id(event) >= 32605 &&
	   event_id(event) <= (32605 +  gd.h_win.num_zoom_levels)) {
	   set_domain_proc(gd.zoom_pu->domain_st,(event_id(event) - 32605), (Event *)NULL);
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
            if(mr->plane  < mr->ds_fhdr.nz -1)
                 set_height( mr->plane + 1);
        break;
        
        case ACTION_GO_COLUMN_FORWARD :         /* ACTION_DOWN */
            if(mr->plane > 0)
                set_height( mr->plane - 1);
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
