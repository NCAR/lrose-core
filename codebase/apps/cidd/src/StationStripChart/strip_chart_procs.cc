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
 * STRIP_CHART_PROCS.C
 */

#include "strip_chart.h"
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>

using namespace std;

extern void  draw_plot(void);
extern void build_field_menu(void);
extern void update_config_popup(void);
extern int compute_flight_cat_index(station_report_t &report);

char *screen_to_value( int xpos, int ypos);
char *field_value(source_info_t *source, int variable, time_t request_time); 

/*************************************************************************
 *  FIELD_VALUE: Return a printed string value given the source,
 *  the variable to print and the time at the cursor
*/

char *field_value(source_info_t *source, int variable, time_t request_time)
{

	static char string_val[32];
	char *ptr;
	double field_val;
	int i;
	int min_dist = 2000000000;
	int closest_index = 0;
	int accept_codes;

	
	if(source->num_reports <= 0) return "\0";

	// Find the closest data to the request time.
	for(i=0; i < source->num_reports; i++) {
		if(request_time > (int) source->reports[i].time) {
			if(request_time - (int) source->reports[i].time < min_dist) {
				min_dist = request_time - source->reports[i].time;
				closest_index = i;
		    }
        } else {
			if((int) source->reports[i].time - request_time  < min_dist) {
				min_dist = source->reports[i].time - request_time;
				closest_index = i;
		    }
		}
    }

	// Build a String repsentation of the data point
     switch(variable) {
     case Params::RATE:
        field_val = source->reports[closest_index].precip_rate;
        switch (gd.p->units) {
           case Params::METRIC:
           break;
           case Params::ENGLISH:
             if(field_val != STATION_NAN) field_val /= INCHES_TO_MM;
           break;
        }
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
	  break;

     case Params::ACCUMULATION:
	    field_val = source->reports[closest_index].liquid_accum;
        switch (gd.p->units) {
           case Params::METRIC:
           break;
           case Params::ENGLISH:
	        if(field_val != STATION_NAN) field_val /= INCHES_TO_MM;
           break;
	     }
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
	break;

     case Params::ACCUMULATION2:
        field_val = source->reports[closest_index].shared.station.liquid_accum2;
        switch (gd.p->units) {
           case Params::METRIC:
           break;
           case Params::ENGLISH:
	         if(field_val != STATION_NAN) field_val /=  INCHES_TO_MM;
           break;
        }
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
      break;

     case Params::SPARE1:
		field_val = source->reports[closest_index].shared.station.Spare1;
		if(field_val != STATION_NAN) {
			field_val *= gd.p->spare1_scale;
			field_val += gd.p->spare1_bias;
		}
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
		break;

     case Params::SPARE2:
		field_val = source->reports[closest_index].shared.station.Spare2;
		if(field_val != STATION_NAN) {
			field_val *= gd.p->spare2_scale;
			field_val += gd.p->spare2_bias;
		}
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
		break;

     case Params::CEILING:
		field_val = source->reports[closest_index].ceiling;
		if(field_val != STATION_NAN) { 
		    field_val *=  3048.0;  // FT per KM. - Convert to Feet.
	    	// Clamp to  25,000 ft.
	    	if (field_val > 25000.0) field_val = 25000.0;
			if(field_val < 0.0) field_val = 0.0;
		}
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
		break;

     case Params::VISIBILITY:
		field_val = source->reports[closest_index].visibility;
        switch (gd.p->units) {
		  case Params::METRIC:
		  break;
		  case Params::ENGLISH:
		    if(field_val != STATION_NAN) field_val /=  KM_PER_MI;
		  break;
		}
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
	break;

     case Params::DEWPT:
		field_val = source->reports[closest_index].dew_point;
        switch (gd.p->units) {
		  case Params::METRIC:
		  break;
		  case Params::ENGLISH:
		    if(field_val != STATION_NAN) field_val = TEMP_C_TO_F(field_val);
		  break;
		}
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
	break;

     case Params::TEMPERATURE:
		field_val = source->reports[closest_index].temp;
        switch (gd.p->units) {
		  case Params::METRIC:
		  break;
		  case Params::ENGLISH:
		    if(field_val != STATION_NAN) field_val = TEMP_C_TO_F(field_val);
		  break;
		}
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
       }
	break;

     case Params::HUMIDITY:
		field_val = source->reports[closest_index].relhum;
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
       }
	break;

     case Params::WIND_SPEED:
		field_val = source->reports[closest_index].windspd;
        switch (gd.p->units) {
		  case Params::METRIC:
		    if(field_val != STATION_NAN) field_val *=  NMH_PER_MS;
		  break;
		  case Params::ENGLISH:
		    if(field_val != STATION_NAN) field_val /=  MPH_TO_MS ;
		  break;
		}
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
	break;

    case Params::WIND_DIRN:
		field_val = source->reports[closest_index].winddir;
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
	break;

    case Params::PRESSURE:
		field_val = source->reports[closest_index].pres;
		if(field_val != STATION_NAN) {
		 sprintf(string_val,"%g",field_val);
		} else {
		 strcpy(string_val,"-");
        }
	break;

    case Params::FZ_PRECIP:
	      accept_codes = (WT_FZFG | WT_MFZDZ | WT_FZRA | WT_FROST |
				               WT_FZDZ | WT_MFZRA | WT_PFZRA | WT_PFZDZ );

	     ptr = weather_type2string(source->reports[closest_index].weather_type);

		 if((source->reports[closest_index].weather_type & accept_codes) == 0 ) {  
		   strcpy(string_val,"-");
		 } else {
		   strcpy(string_val,ptr);
		 }

	break;

    case Params::PRECIP_TYPE:
	      accept_codes = (WT_RA | WT_UP | WT_FG | WT_HZ |
				              WT_TS | WT_GR | WT_PE | WT_DZ |
							  WT_MRA | WT_PRA | WT_MSN |  WT_SN |  WT_PSN );

	     ptr = weather_type2string(source->reports[closest_index].weather_type);

		 if((source->reports[closest_index].weather_type & accept_codes) == 0 ) {  
		   strcpy(string_val,"-");
		 } else {
		   strcpy(string_val,ptr);
		 }

	break;

    case Params::FLIGHT_CAT:
		int index = compute_flight_cat_index(source->reports[closest_index]);
	    if(index < 0) {
		  strcpy(string_val,"-");
		} else {
		  strcpy(string_val, gd.p->_flight_category[index].label_str);
		}
	break;


    } // switch

    return string_val;
}

/*************************************************************************
 * SCREEN_TO_VALUE: Return a String representing the data value plotted
 *  at the cursor position.
 */
char *screen_to_value( int xpos, int ypos)
{
	 static char str_buf[256];
	 static char *val_ptr;
	 time_t req_time;
	 int plot_height =  gd.win_height - gd.p->bottom_margin;

	 double pix_x_bias = (gd.win_width -  gd.p->right_margin) -
			  (gd.pixels_per_sec * (double)gd.end_time) - 3;

	 req_time = (time_t) ((int) ((xpos - pix_x_bias) / gd.pixels_per_sec));


	 if(gd.cur_plot < gd.num_active_fields) { // looking at one field - many stations
	   int num_stations;
	   int cur_station = 0;

	   if(gd.p->max_display_stations < gd.num_sources) {
	     num_stations = gd.p->max_display_stations;
	   } else {
         num_stations = gd.num_sources;
	   }
       double each_height = (double) plot_height / num_stations;
	   cur_station = (int) (ypos / each_height);
	   if(cur_station > num_stations -1) cur_station = num_stations -1;

	   val_ptr = field_value(gd.sorted_sources[cur_station], gd.variable,  req_time);
	   strcpy(str_buf,val_ptr);
	   
	 } else { // Looking at one station many fields


			 // Get which source
		 int cur_station = gd.cur_plot - gd.num_active_fields;

		 // Get which variable the cursor is in
		 double each_height = (double) plot_height / gd.num_active_fields;
		 int variable = (int) (ypos / each_height);
		 if(variable > gd.num_active_fields -1) variable = gd.num_active_fields -1;
		 variable = gd.field_lookup[variable];
		 
	     val_ptr = field_value(gd.sorted_sources[cur_station], variable,  req_time);
	     strcpy(str_buf,val_ptr);
	 }

	 return str_buf;
}

/*************************************************************************
 * Menu handler for `menu1'.
 */
Menu
select_field_proc( Menu	menu, Menu_generate	op)
{
	
  int plot_num;
  Menu_item item = (Menu_item ) xv_get(menu, MENU_SELECTED_ITEM);
  
  switch (op) {
  case MENU_DISPLAY:
    break;
    
  case MENU_DISPLAY_DONE:
    break;
    
  case MENU_NOTIFY:
    
	plot_num = (int) xv_get(item,XV_KEY_DATA,MENU_KEY);

	if(plot_num >= gd.num_active_fields) {
	   // Search through sorted sources for match
	   for(int i=0; i < gd.num_sources; i++) {
		  if(gd.sorted_sources[i] == &gd.sources[plot_num - gd.num_active_fields]) {
			  gd.cur_plot = i + gd.num_active_fields;
		  }
	   }

	 } else {
		 gd.cur_plot = plot_num;
	 }

    gd.data_min = 99999999.0;
    gd.data_max = -99999999.0;
    
    draw_plot();
    
    break;
    
  case MENU_NOTIFY_DONE:
    break;
  }
  return menu;
}

/*************************************************************************
 * Event callback function for `canvas1'.
 */
Notify_value
strip_chart_win1_canvas1_event_callback(Xv_window win,
					Event  *event,
					Notify_arg arg,
					Notify_event_type type)

{
  
  xv_get(xv_get(win, CANVAS_PAINT_CANVAS_WINDOW),
	 XV_KEY_DATA, INSTANCE);
  
  /* gxv_start_connections DO NOT EDIT THIS SECTION */
  
  switch(event_action(event)) {
  case ACTION_MENU:
  case ACTION_SELECT:
    if(event_is_down(event)) {
      Menu    menu = (Menu) xv_get(win, WIN_MENU);
      if(menu) menu_show(menu, win, event, 0);
    }
  break;

  case ACTION_ADJUST:
    if(event_is_up(event)) {
	  update_config_popup();
      xv_set(gd.Strip_chart_config_pu->config_pu,
             FRAME_CMD_PUSHPIN_IN, TRUE,
             XV_SHOW,TRUE,
             NULL);
    }
  break;
    
  case WIN_REPAINT:
    if (gd.p->debug)  cerr << "Repainting window" << endl;
    draw_plot();
  break;
    
  }

  if(event_id(event) == LOC_MOVE || event_id(event) ==  LOC_WINEXIT) {
	  static int report_visible = 0;
	  static int x1 = 0;
	  static int y1 = 0;
	  static int xwidth = gd.win_width;
	  static int yheight = gd.win_height;
	  
	  int out_of_range_flag = 0; // Do not draw value if set

	  // For labels
	  int direct,ascent,descent;
	  int xpos,ypos;
	  XCharStruct overall;
	  char *val_ptr;

	  xpos = event_x(event);
	  ypos = event_y(event);

	  if(report_visible) {  // Restore underlying image
		XCopyArea(gd.dpy,gd.back_xid,gd.canvas_xid,gd.def_gc,
	              x1,y1, xwidth, yheight, x1, y1);
	  }
	  if(event_id(event) ==  LOC_WINEXIT) out_of_range_flag = 1;

	  if(out_of_range_flag != 0 ) {
	     report_visible = 0;
	  } else {
	      report_visible = 1;
	      val_ptr = screen_to_value(xpos,ypos);

	      if(val_ptr != NULL) {
	         int len = strlen(val_ptr);
                 XTextExtents(gd.fontst,val_ptr,len,&direct,&ascent,&descent,&overall);

		 // Don't let label leave the right edge - Shift it left
		 if(xpos + 2 + overall.width > gd.win_width) {
		    xpos -= (xpos + 2 + overall.width) - gd.win_width;
		 }
		 XSetForeground(gd.dpy, gd.def_gc, gd.fg_cell);
		 XDrawImageString(gd.dpy,gd.canvas_xid,gd.def_gc,
				 xpos + 2,
				 ypos-gd.fontst->descent + 2,
				 val_ptr,len);

		 xwidth =  overall.width;
		 yheight = gd.fontst->ascent + gd.fontst->descent;
		 x1 = xpos + 2;
		 y1 = ypos - yheight + 2;

	      }

	  }
  }
  
  /* gxv_end_connections */
  
  return notify_next_event_func(win, (Notify_event) event, arg, type);

}

/*************************************************************************
 * Event callback function for `win1'.
 */

Notify_value
win1_event_proc( Xv_window  win, Event *event,
		 Notify_arg arg, Notify_event_type type)
{
	static int last_width = 0;
	static int last_height = 0;

  xv_get(win, XV_KEY_DATA, INSTANCE);
  /* Move to front of any motion drag or resize events */
  while (XCheckMaskEvent(gd.dpy, PointerMotionMask | ButtonMotionMask | ResizeRequest,
			  event->ie_xevent))
     /* NULL BODY */;

  
  switch(event_action(event)) {

  case WIN_RESIZE:

    // recompute sizes

    gd.win_height =  xv_get(gd.Strip_chart_win1->win1,WIN_HEIGHT);
    gd.win_width =  xv_get(gd.Strip_chart_win1->win1,WIN_WIDTH);

    // Avoid redundant resize events.
    if(gd.win_height != last_height || gd.win_width != last_width) {
      gd.plot_height = gd.win_height - gd.p->bottom_margin;
      gd.plot_width = gd.win_width -  gd.p->right_margin;

      last_height = gd.win_height;
      last_width = gd.win_width;
      
      if(!gd.p->keep_period_fixed) {
		check_retrieve(true); // force a recalc on the times, and a retrieve
	  } else {
	    gd.pixels_per_sec = (double) gd.plot_width  / (double) gd.p->plot_period_secs;
	  }
    
      if (gd.p->debug) {
        cerr << "Resizing window" << endl;
        cerr << "  pixels_per_sec: " << gd.pixels_per_sec << endl;
        cerr << "  win_height: " << gd.win_height << endl;
        cerr << "  win_width: " << gd.win_width << endl;
        cerr << "  plot_height: " << gd.plot_height << endl;
        cerr << "  plot_width: " << gd.plot_width << endl;
        cerr << "  Start time: " << utimstr(gd.start_time) << endl;
        cerr << "  End   time: " << utimstr(gd.end_time) << endl;
      }

      // Release backing Pixmap
      if(gd.back_xid) XFreePixmap(gd.dpy,gd.back_xid);
    
      // create new backing Pixmap
      gd.back_xid =  XCreatePixmap(gd.dpy, gd.canvas_xid,
				 gd.win_width, gd.win_height,
				 DefaultDepth(gd.dpy,0));

     draw_new_data();
   }

   break;

  default: {}

  }
  
  return notify_next_event_func(win, (Notify_event) event, arg, type);

}

/*************************************************************************
 * Notify callback function for `run_mode_st'.
 */
void
run_mode_proc(Panel_item item, int value, Event *event)
{
   gd.p->mode = (Params::mode_t)  value;
   switch(gd.p->mode) {
	   case Params::REALTIME: gd.want_live_updates = 1;
	   case Params::ARCHIVE: gd.want_live_updates = 0;
	   case Params::SLAVE:  
			if(gd.cidd_mem->runtime_mode == RUNMODE_REALTIME) {
				gd.want_live_updates = 1;
			} else {
				gd.want_live_updates = 0;
			}
	}

   check_retrieve(true);

   update_config_popup();

   draw_plot();
}

/*************************************************************************
 * Notify callback function for `archive_time_tx'.
 */
Panel_setting
set_arch_time_proc(Panel_item item, Event *event)
{
		int year,month,day;
		int hour,min,sec;
		int nfields;

        char *  value = (char *) xv_get(item, PANEL_VALUE);

        nfields = sscanf(value,"%d %d %d %d %d %d",
						 &year, &month,&day,&hour,&min,&sec);
        
		DateTime atime(year,month,day,hour,min,sec);
        
		gd.archive_time = atime.utime();
		if(gd.p->use_localtime) {
		  struct tm *t;
		  t = localtime(&gd.archive_time);
		  gd.archive_time -= t->tm_gmtoff;
		}
				        

		check_retrieve(true);

        update_config_popup();

		draw_plot();

        return panel_text_notify(item, event);
}

/* ************************************************************************
 * Notify callback function for `url_tx'.
 */
Panel_setting
set_url_proc(Panel_item item, Event *event)
{
        char *  value = (char *) xv_get(item, PANEL_VALUE);
        
		gd.p->input_url = value;

		retrieve_data();

		draw_plot();

        return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `update_interval_tx'.
 */
Panel_setting
update_interval_tx(Panel_item item, Event *event)
{
        char *    value = (char *) xv_get(item, PANEL_VALUE);
        
		gd.p->update_interval_min = atoi(value);

		retrieve_data();

		draw_plot();
        
        return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `delay_tx'.
 */
Panel_setting
delay_proc(Panel_item item, Event *event)
{
        char *  value = (char *) xv_get(item, PANEL_VALUE);

		gd.p->seconds_delay = atoi(value);

		retrieve_data();

		draw_plot();
        
        return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `trace_width_tx'.
 */
Panel_setting
trace_width_proc(Panel_item item, Event *event)
{
        char * value =  (char *) xv_get(item, PANEL_VALUE);
        
		gd.p->trace_line_width = atoi(value);

        draw_plot();

        return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `scale_tx'.
 * gd.pixels_per_sec
 */
Panel_setting
scale_proc(Panel_item item, Event *event)
{
        char *  value = (char *) xv_get(item, PANEL_VALUE);
		double secs;

		if(gd.p->keep_period_fixed) {
			secs = atof(value) * 3600.0;
			gd.pixels_per_sec = gd.plot_width  / secs;
			gd.p->plot_period_secs = (int) secs;
		} else {
		    double scale = 1.0 / atof(value);
		    gd.pixels_per_sec = scale;
		}

		check_retrieve(true);

        update_config_popup();

		draw_plot();

        return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `field_st'.
 */
void
active_field_st(Panel_item item, int value, Event *event)
{
//   strip_chart_config_pu_objects *ip =
//     (strip_chart_config_pu_objects *) xv_get(item,
//                                              XV_KEY_DATA, INSTANCE);
  short   i;
        
        for (i = 0; i < MAX_FIELDS; i++) {
                if (value & 01) {
				    gd.field_active[i] = 1;
				} else {
				    gd.field_active[i] = 0;
				}

                value >>= 1;
        }

		build_field_menu();
}


/*************************************************************************
 * Notify callback function for `config_dismiss_bt'.
 */
void
dismiss_config_proc(Panel_item item, Event *event)
{
//         strip_chart_config_pu_objects *ip = (strip_chart_config_pu_objects *) xv_get(item,
//  XV_KEY_DATA, INSTANCE);
        
      xv_set(gd.Strip_chart_config_pu->config_pu,
             FRAME_CMD_PUSHPIN_IN, FALSE,
             XV_SHOW,FALSE,
             NULL);
}

/*************************************************************************
 * Notify callback function for `units_st'.
 */
void
set_units_proc(Panel_item item, int value, Event *event)
{
    switch(value) {
     case 0: gd.p->units = Params::METRIC; break;
     case 1: gd.p->units = Params::ENGLISH; break;
    }

	draw_plot();
}


/*************************************************************************
 * Notify callback function for `zone_st'.
 */
void
zone_proc(Panel_item item, int value, Event *event)
{
    switch(value) {
		case 0: gd.p->use_localtime =  pFALSE; break;
		case 1: gd.p->use_localtime = pTRUE; break;
    }

	draw_plot();

}
