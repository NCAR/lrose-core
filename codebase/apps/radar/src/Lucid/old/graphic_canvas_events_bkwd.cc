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
 * GRAPHIC_CANVAS_EVENTS_BKWD.CC:: Event handling for the Horiz view canvas 
 * - BACKWARD COMPATIBILITY VERSION
 * For the Cartesian Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */


#define GRAPHIC_CANVAS_EVENTS_BKWD  1

#include "cidd.h"

// file scope variables

extern int b_lastx,b_lasty;    /* Boundry end point */
extern int b_startx,b_starty;    /* Boundry start point */
extern int p_lastx,p_lasty;    /* Pan end point */
extern int p_startx,p_starty;    /* Pan start point */
extern int r_lastx,r_lasty;    /* ROUTE end point */
extern int r_startx,r_starty;    /* ROUTE start point */
extern int r_state;

#ifdef NOTNOW

static fl32 _missingVal = -9999;
static fl32 _badVal = -9999;
static MetRecord *_mr = NULL;
static double pseudo_diam = 17066.0; /* 4/3 earth diam for radar beam corrections */

static int last_frame = -1;
static time_t lockout_time = 0;  /* Events will be discarded until this passes */
static double last_lat = 0.0;
static double last_lon = 0.0;

static struct timeval _tp;

static int report_visible = 0;
static int xx1 = gd.h_win.can_dim.x_pos;
static int yy1 = gd.h_win.can_dim.y_pos;
static int xwidth = gd.h_win.can_dim.width;
static int yheight = gd.h_win.can_dim.height;

#endif

// file scope functions

// static void draw_report_text(Event *event);
// static void respond_to_select(Event *event);
// static void respond_to_adjust(Event *event);
// static void respond_to_menu(Event *event);
// static void respond_to_drag(Event *event);
// static void respond_to_keyboard(Event *event);
// static double constrain_az(double az);

#ifdef NOTNOW

/*************************************************************************
 * Event callback function for `canvas1'.
 */
Notify_value can_event_proc_bkwd(Window win, Event *event,
				 Notify_arg arg, Notify_event_type type)
{
    
  /* Move to front of any motion drag events */
  while (XCheckMaskEvent(gd.dpy, PointerMotionMask | ButtonMotionMask, event->ie_xevent)) {
    /* NULL BODY */
  }

  // set up field reference

  _mr = gd.mrec[gd.h_win.page];
  if (_mr == NULL || _mr->h_mdvx == NULL || _mr->h_mdvx->getFieldByNum(0) == NULL) {
    // Just skip Processing
    return notify_next_event_func(win, (Notify_event) event, arg, type);
  }
  
  const Mdvx::field_header_t &fhdr = (_mr->h_mdvx->getFieldByNum(0))->getFieldHeader();
  _missingVal = fhdr.missing_data_value;
  _badVal = fhdr.bad_data_value;
  
  ///////REPORT MODE ////////////////////

  if(gd.report_mode) {
    if(event_id(event) == LOC_MOVE || event_id(event) ==  LOC_WINEXIT) {
      if( gd.movie.cur_frame == gd.movie.last_frame &&
	  gd.movie.movie_on == 0 &&
	  _mr->h_data_valid == 1 &&
	  _mr->h_fl32_data != NULL ) {
	draw_report_text(event);
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

  struct timezone tzp;
  gettimeofday(&_tp, &tzp);                /* record when this event took place */
  gd.last_event_time = _tp.tv_sec;

  if(_tp.tv_sec <= lockout_time) {
    if(gd.debug) {
      fprintf(stderr,"Events locked out for %ld more seconds\n",
	      ((lockout_time - _tp.tv_sec) + 1));
    }

    // Just skip Processing
    return notify_next_event_func(win, (Notify_event) event, arg, type);
  }

  /* Left button up or down  */
  if(event_action(event) == ACTION_SELECT ) {
    respond_to_select(event);
  }

  /* Middle button up or down */
  if(event_action(event) == ACTION_ADJUST ) {
    respond_to_adjust(event);
  }
     
  /* RIGHT Button up or down  */
  if (event_action(event) == ACTION_MENU ) {
    respond_to_menu(event);
  }

  /* Mouse is moved while any button is down - LOC DRAG */

  if (event_action(event) == LOC_DRAG ) {
    respond_to_drag(event);
  }

  /*
   * Process keyboard events.  Only process them when the key is
   * released to prevent repeated processing when the key is held
   * down (may want to change this in the future).
   */

  respond_to_keyboard(event);

  return notify_next_event_func(win, (Notify_event) event, arg, type);

}

////////////////////////////////////////
// draw the report text at the cursor

static void draw_report_text(Event *event)

{

  int len,direct,ascent,descent;
  int xpos,ypos;
  XCharStruct overall;
  
  xpos = event_x(event);
  ypos = event_y(event);
  
  int x_gd, y_gd;
  pixel_to_grid(_mr, &gd.h_win.margin, xpos, ypos, &x_gd, &y_gd);

  int out_of_range_flag = 0;
  x_gd = CLIP(x_gd,0,(_mr->h_fhdr.nx -1),out_of_range_flag);
  y_gd = CLIP(y_gd,0,(_mr->h_fhdr.ny -1),out_of_range_flag);
  
  if(xpos < gd.h_win.img_dim.x_pos || 
     xpos > gd.h_win.img_dim.x_pos + gd.h_win.img_dim.width ||
     ypos < gd.h_win.img_dim.x_pos ||
     ypos > gd.h_win.img_dim.x_pos + gd.h_win.img_dim.height) 
    out_of_range_flag = 1;
  
  fl32 *ptr = _mr->h_fl32_data;
  // Pick out grid pint in plane
  ptr += (_mr->h_fhdr.nx * (y_gd)) + (x_gd);
  if(report_visible) {
    XCopyArea(gd.dpy,gd.h_win.can_xid[gd.h_win.cur_cache_im],
	      gd.h_win.vis_xid,
	      gd.def_gc,    xx1, yy1,
	      xwidth, yheight, xx1, yy1);
  }
  
  if(event_id(event) ==  LOC_WINEXIT) out_of_range_flag = 1;

  if(out_of_range_flag != 0 || *ptr == _missingVal || *ptr == _badVal) {

    report_visible = 0;

  } else {

    report_visible = 1;
    double value;
    if (_mr->h_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
      value = exp((double) *ptr);
    } else {
      value =  *ptr;
    }
    
    // Deal with values very close to 0
    if(value  < 0.0001 && value  > -0.0001) value = 0.0;

    char text[256];
    snprintf(text,"%g",value);
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
    
    xx1 = xpos + 2;
    yy1 = ypos - yheight + 2;
  }

}

////////////////////////////////////////
// respond to a select event

static void respond_to_select(Event *event)

{

  int x_gd,y_gd; /* Data grid coords */
     
  double x_dproj,y_dproj;/* Km coords */
  double lat,lon;        /* latitude, longitude coords */
  double dist_from_origin, az_from_origin; // click point relative to origin
  double dist_moved,az_moved;  // distance and angle from last click point

  // data

  double value; /* data value after scaling */
  fl32 *ptr; /* pointer to fl32 data */

  // text buffers

  char singleLineText[1024];
  char multiLineText[8192];
  char xy_string[256];
  char lat_string[256];
  char lon_string[256];
  char beam_ht_str[256];
  
  xy_string[0] = '\0';
  beam_ht_str[0] = '\0';

  // set up radar parameters, if relevant

  double radarElevDeg = -9999;
  double beamHt = -9999;
  double radarOriginLat = 0.0;
  double radarOriginLon = 0.0;
  double radarOriginAlt = 0.0;
  const Mdvx::field_header_t &fhdr = (_mr->h_mdvx->getFieldByNum(0))->getFieldHeader();
  if (fhdr.vlevel_type == Mdvx::VERT_TYPE_ELEV) {
    radarElevDeg = gd.h_win.cur_ht;
    radarOriginLat = fhdr.proj_origin_lat;
    radarOriginLon = fhdr.proj_origin_lon;
    radarOriginAlt = fhdr.user_data_fl32[0];
  }

  if (gd.route_in_progress && event_is_up(event)) {
    
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
      snprintf(singleLineText,"%d Segments - Last pointed Deleted",
	      gd.h_win.route.num_segments);
      gui_label_h_frame(singleLineText, 1);
      add_message_to_status_win(singleLineText,0);

    } else {
      redraw_route_line(&gd.h_win); // clears the line
      gd.h_win.route.num_segments = 0;
      gd.route_in_progress = 0;
      r_state = 0;
      if(_params.drawing_mode == DRAW_FMQ_MODE) {
	gui_label_h_frame("Draw Mode Canceled", 1);
	add_message_to_status_win("Draw Mode Canceled",0);
      } else {
	gui_label_h_frame("Route Define Canceled", 1);
	add_message_to_status_win("Route Define Canceled",0);
      }
      set_redraw_flags(1,0);
    }

  } else if (event_is_down(event)) {

    gd.zoom_in_progress = 1;
    b_startx = event_x(event);
    b_starty = event_y(event);
    b_lastx = b_startx;
    b_lasty = b_starty;

  } else {  // UP
    
    gd.zoom_in_progress = 0;

    // compute locations

    pixel_to_disp_proj(&gd.h_win.margin, b_lastx, b_lasty, &x_dproj, &y_dproj);
    gd.proj.xy2latlon(x_dproj,y_dproj,lat,lon);
    PJGLatLon2RTheta(gd.h_win.origin_lat,gd.h_win.origin_lon,
		     lat,lon,&dist_from_origin,&az_from_origin);
    PJGLatLon2RTheta(last_lat,last_lon,lat,lon,&dist_moved,&az_moved);
    snprintf(xy_string, "X,Y: %.2f,%.2fkm  ", x_dproj, y_dproj);
    
    last_lat = lat;
    last_lon = lon;

    // compute radar beam ht if relevant, set beam ht str

    if (radarElevDeg > -999) {
      double rangeFromRadar, azFromRadar;
      PJGLatLon2RTheta(radarOriginLat,radarOriginLon,lat,lon,&rangeFromRadar,&azFromRadar);
      double zcorr = (rangeFromRadar * rangeFromRadar) / pseudo_diam;
      beamHt = radarOriginAlt + rangeFromRadar * sin(radarElevDeg * DEG_TO_RAD) + zcorr;
      snprintf(beam_ht_str, "  Beam height: %.3f km, %.0f ft\n", beamHt, (beamHt * 1000.0 / 0.3048));
    }

    /* Nicely format the lat lons */

    switch(_params.latlon_mode) {
      default:
      case 0:  /* Decimal Degrees */
	if (lat > 90.0) {
	  snprintf(lat_string, ">90");
	} else if (lat < -90.0) {
	  snprintf(lat_string, "<90");
	} else {
	  snprintf(lat_string,"%.4f",lat);
	}
	snprintf(lon_string,"%.4f",lon);
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
	    snprintf(lat_string, ">90");
	  } else if (lat < -90.0) {
	    snprintf(lat_string, "<90");
	  } else {
	    snprintf(lat_string,"%d %d\' %.0f\"",ideg,imin,fsec);
	  }

	  ideg = (int) lon;
	  fmin = fabs(lon - ideg); /* extract decimal fraction */
	  imin = (int) (fmin * 60.0);
	  fsec = (fmin - (imin / 60.0)) * 3600.0; 
	  snprintf(lon_string,"%d %d\' %.0f\"",ideg,imin,fsec);
	}
	break;
    }


    if ((abs(b_lastx - b_startx) > 5 || abs(b_lasty - b_starty) > 5)) {

      do_zoom();

      double clickX = (gd.h_win.cmin_x + gd.h_win.cmax_x) / 2.0;
      double clickY = (gd.h_win.cmin_y + gd.h_win.cmax_y) / 2.0;
      gd.proj.xy2latlon(clickX, clickY,lat,lon);
      handle_click_h(event, b_lastx, b_lasty,
                     clickX, clickY, lat, lon, CIDD_ZOOM_CLICK);

    } else {       /* report data value */ 

      double clickX = x_dproj;
      double clickY = y_dproj;
      handle_click_h(event, b_lastx, b_lasty,
                     clickX, clickY, lat, lon, CIDD_USER_CLICK);
      
      render_click_marks();
      
      pixel_to_grid(_mr, &gd.h_win.margin, b_lastx, b_lasty, &x_gd, &y_gd);
      
      int out_of_range_flag = 0;
      x_gd = CLIP(x_gd,0,(fhdr.nx -1),out_of_range_flag);
      y_gd = CLIP(y_gd,0,(fhdr.ny -1),out_of_range_flag);

      if(out_of_range_flag) {

	// outsize data grid
	
	snprintf(singleLineText,
		"%s: Outside grid at "
		"LAT,LON: %s,%s  Az/dist:%.1fdegT,%.1fkm  %s",
		_mr->legend_name, 
		lat_string, lon_string,
		constrain_az(az_from_origin), dist_from_origin,
		xy_string);
	      
	// Add the station location
	if(gd.station_loc != NULL) {
	  strncat(singleLineText," : ",256);
	  strncat(singleLineText,
		  gd.station_loc->FindClosest(lat,lon,_params.locator_margin_km).c_str(),
		  256);
	}

	gui_label_h_frame(singleLineText,-3);
	strncat(singleLineText,"\n",256);
	add_message_to_status_win(singleLineText, 0);

	// status window
	
	if (_params.report_clicks_in_status_window) {
	  snprintf(multiLineText,
		  "=====================================\n"
		  "===== Clicked outside data grid =====\n"
		  "  LAT,LON: %s, %s\n"
		  "  From origin: %.1f degT, %.1f km\n"
		  "  From previous: %.1f degT, %.1f km\n"
		  "  X,Y: %.2fkm, %.2fkm\n",
		  lat_string,lon_string,
		  constrain_az(az_from_origin), dist_from_origin,
		  constrain_az(az_moved), dist_moved,
		  x_dproj, y_dproj);
	  add_report_to_status_win(multiLineText);
	  if (_params.report_clicks_in_degM_and_nm) {
	    snprintf(multiLineText,
		    "===== Click point loc in degM/nm =====\n"
		    "  From origin: %.1f degM, %.1f nm\n"
		    "  From previous: %.1f degM, %.1f nm\n",
		    constrain_az(az_from_origin - _params.magnetic_variation_deg),
		    dist_from_origin / KM_PER_NM,
		    constrain_az(az_moved - _params.magnetic_variation_deg),
		    dist_moved / KM_PER_NM);
	    add_report_to_status_win(multiLineText);
	  }
	}

      } else {

	if(gd.movie.cur_frame == last_frame) {
	  
	  if(_mr->h_fl32_data != NULL) {
	    ptr = _mr->h_fl32_data;
	    
	    // Pick out correct grid point
	    ptr += (fhdr.nx * (y_gd - 0)) + (x_gd - 0);

	    if(*ptr == _missingVal || *ptr == _badVal) {
	      
	      snprintf(singleLineText,
		      "%s: Missing at "
		      "LAT,LON: %s,%s  Az/dist:%.1fdegT,%.1fkm  %sGX,GY: %d,%d",
		      _mr->legend_name, 
		      lat_string, lon_string,
		      constrain_az(az_from_origin), dist_from_origin,
		      xy_string,
		      x_gd, y_gd);
	      
	      if (_params.report_clicks_in_status_window) {
		snprintf(multiLineText,
			"=======================================\n"
			"===== Data missing at click point =====\n"
			"  Time: %s\n"
			"  LAT,LON: %s, %s\n"
			"  From origin: %.1f degT, %.1f km\n"
			"  From previous: %.1f degT, %.1f km\n"
			"%s"
			"  X,Y: %.2fkm, %.2fkm\n"
			"  IndexX,IndexY: %d, %d\n",
			DateTime::strm(_mr->h_mhdr.time_centroid).c_str(),
			lat_string,lon_string,
			constrain_az(az_from_origin), dist_from_origin,
			constrain_az(az_moved), dist_moved,
			beam_ht_str,
			x_dproj, y_dproj,
			x_gd, y_gd);
		add_report_to_status_win(multiLineText);
	      }

	    } else {
	      
	      // Grab the fl32 data value
	      if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
		value = exp((double) *ptr);
	      } else {
		value =  *ptr;
	      }

	      // Deal with (fudge) values very close to 0
	      if(value < 0.0001 && value  > -0.0001) value = 0.0;

	      if(fhdr.proj_type == Mdvx::PROJ_LATLON) {
		// for latlon text, no XY
		xy_string[0] = '\0';
	      }
	      
	      snprintf(singleLineText,
		      "%s %g %s  "
		      "LAT,LON:%s,%s  Az/dist:%.1fdegT,%.1fkm  %sGX,GY: %d,%d",
		      _mr->legend_name, value, _mr->field_units, 
		      lat_string, lon_string,
		      constrain_az(az_from_origin), dist_from_origin,
		      xy_string,
		      x_gd, y_gd);

	      if (_params.report_clicks_in_status_window) {
		snprintf(multiLineText,
			"======================================\n"
			"====== Data found at click point =====\n"
			"  Time: %s\n"
			"  %s: %g %s\n"
			"  LAT,LON: %s, %s\n"
			"  From origin: %.1f degT, %.1f km\n"
			"  From previous: %.1f degT, %.1f km\n"
			"%s"
			"  X,Y: %.2fkm, %.2fkm\n"
			"  IndexX,IndexY: %d, %d\n",
			DateTime::strm(_mr->h_mhdr.time_centroid).c_str(),
			_mr->legend_name,value,_mr->field_units, 
			lat_string,lon_string,
			constrain_az(az_from_origin), dist_from_origin,
			constrain_az(az_moved), dist_moved,
			beam_ht_str,
			x_dproj, y_dproj,
			x_gd, y_gd);
		add_report_to_status_win(multiLineText);
	      }
	      
	    }

	    if (_params.report_clicks_in_degM_and_nm) {
	      snprintf(multiLineText,
		      "===== Click point loc in degM/nm =====\n"
		      "  From origin: %.1f degM, %.1f nm\n"
		      "  From previous: %.1f degM, %.1f nm\n",
		      constrain_az(az_from_origin - _params.magnetic_variation_deg),
		      dist_from_origin / KM_PER_NM,
		      constrain_az(az_moved - _params.magnetic_variation_deg),
		      dist_moved / KM_PER_NM);
	      add_report_to_status_win(multiLineText);
	    }

	    // Add the station location
	    if(gd.station_loc != NULL) {
	      strncat(singleLineText," : ",256);
	      strncat(singleLineText,
		      gd.station_loc->FindClosest(lat,lon,_params.locator_margin_km).c_str(),
		      256);
	    }
	    gui_label_h_frame(singleLineText,-3);
	    strncat(singleLineText,"\n",256);
	    add_message_to_status_win(singleLineText, 0);
	  }

	} else {

	  _mr->h_data_valid = 0;
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

////////////////////////////////////////
// respond to an adjust event

static void respond_to_adjust(Event *event)

{

  double x_dproj,y_dproj;        /* Km coords */
  double lat,lon;        /* latitude, longitude coords */

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
        
        double clickX = (gd.h_win.cmin_x + gd.h_win.cmax_x) / 2.0;
        double clickY = (gd.h_win.cmin_y + gd.h_win.cmax_y) / 2.0;
        gd.proj.xy2latlon(clickX,clickY,lat,lon);
        handle_click_h(event, p_lastx, p_lasty,
                       clickX, clickY, lat, lon, CIDD_ZOOM_CLICK);

      } else {
	
        double clickX = x_dproj;
        double clickY = y_dproj;
        handle_click_h(event, p_lastx, p_lasty,
                       clickX, clickY, lat, lon, CIDD_USER_CLICK);

        render_click_marks();
	
	gd.save_im_win = PLAN_VIEW;
	update_save_panel();
	if(_params.enable_save_image_panel)
	  // xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, TRUE,XV_SHOW, TRUE,NULL);
      }
      gd.pan_in_progress = 0;
    }
  }

}

////////////////////////////////////////
// respond to a menu event

static void respond_to_menu(Event *event)

{

  double    x_dproj,y_dproj;        /* Km coords */
  double    lat,lon;        /* latitude, longitude coords */
  char text[1024];

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
	if(_params.drawing_mode == DRAW_FMQ_MODE) {
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
	  if( _params.one_click_rhi == 0 ) {
	    snprintf(text,"%d Segments Entered - Click to extend - twice at end to finish",
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
	    gd.proj.xy2latlon(gd.h_win.route.x_world[0], gd.h_win.route.y_world[0],
			      gd.h_win.route.lat[0], gd.h_win.route.lon[0]);
                          
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
	  gd.proj.xy2latlon(gd.h_win.route.x_world[gd.h_win.route.num_segments],
			    gd.h_win.route.y_world[gd.h_win.route.num_segments],
			    gd.h_win.route.lat[gd.h_win.route.num_segments],
			    gd.h_win.route.lon[gd.h_win.route.num_segments]);

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
	    txt_ptr = gd.station_loc->FindClosest(lat,lon,_params.locator_margin_km).c_str();
	    if(strlen(txt_ptr) ==0)  {
	      snprintf(gd.h_win.route.navaid_id[gd.h_win.route.num_segments-1],"%d",gd.h_win.route.num_segments);
	    } else {
	      strncpy(gd.h_win.route.navaid_id[gd.h_win.route.num_segments-1],txt_ptr,16);
	    }

	    //  last point
	    gd.proj.xy2latlon(gd.h_win.route.x_world[gd.h_win.route.num_segments],
			      gd.h_win.route.y_world[gd.h_win.route.num_segments],
			      lat,lon); 
	    txt_ptr = gd.station_loc->FindClosest(lat,lon,_params.locator_margin_km).c_str();
	    if(strlen(txt_ptr) ==0)  {
	      snprintf(gd.h_win.route.navaid_id[gd.h_win.route.num_segments],"%d",gd.h_win.route.num_segments+1);
	    } else {
	      strncpy(gd.h_win.route.navaid_id[gd.h_win.route.num_segments],txt_ptr,16);
	    }
	  } else {
	    snprintf(gd.h_win.route.navaid_id[gd.h_win.route.num_segments-1],"%d",gd.h_win.route.num_segments);
	    snprintf(gd.h_win.route.navaid_id[gd.h_win.route.num_segments],"%d",gd.h_win.route.num_segments+1);
	  }

	  /*   DEBUG
	       fprintf(stderr,"navaid_id %d: %s  %d: %s\n",
	       gd.h_win.route.num_segments,
	       gd.h_win.route.navaid_id[gd.h_win.route.num_segments-1],
	       gd.h_win.route.num_segments+1,
	       gd.h_win.route.navaid_id[gd.h_win.route.num_segments]);
	  */
	  if(_params.drawing_mode != DRAW_FMQ_MODE && _params.one_click_rhi != 0 ) {
	    setup_route_area(1);
	    strncpy(gd.h_win.route.route_label,"Custom",7);

	    // Copy the route definition into the space reserved for the Custom route 
	    if(gd.layers.route_wind.num_predef_routes > 0 ) {
	      memcpy(gd.layers.route_wind.route+gd.layers.route_wind.num_predef_routes,
		     &gd.h_win.route,sizeof(route_track_t));
	    }

	    // xv_set(gd.route_pu->route_st,PANEL_VALUE,gd.layers.route_wind.num_predef_routes,NULL);
	    gd.route_in_progress = 0;
	    lockout_time = _tp.tv_sec+1;  // Lockout for at most 1  seconds
	    r_state = 0;

	  } else {

	    r_state = 3;
	  }

	} else { // Only small change after dragging
          
          double clickX = x_dproj;
          double clickY = y_dproj;
          handle_click_h(event, r_lastx, r_lasty,
                         clickX, clickY, lat, lon, CIDD_USER_CLICK);

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
	if(_params.drawing_mode == DRAW_FMQ_MODE) {
	  show_draw_panel(1);
	} else {
	  setup_route_area(1);
	}
	r_state = 0;
	strncpy(gd.h_win.route.route_label,"Custom",7);

	// Copy the route definition into the space reserved for the Custom route 
	if(gd.layers.route_wind.num_predef_routes > 0 ) {
	  memcpy(gd.layers.route_wind.route+gd.layers.route_wind.num_predef_routes,
		 &gd.h_win.route,sizeof(route_track_t));
	}

	// xv_set(gd.route_pu->route_st,PANEL_VALUE,gd.layers.route_wind.num_predef_routes,NULL);
	gd.route_in_progress = 0;
	lockout_time = _tp.tv_sec+1;  // Lockout for at most 1  seconds
	break;

    }

  } // End of Button UP section

}

////////////////////////////////////////
// respond to a drag event

static void respond_to_drag(Event *event)

{

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

////////////////////////////////////////
// respond to a keyboard event

static void respond_to_keyboard(Event *event)

{

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
    if(event_id(event) == 0 && event->ie_xevent->xkey.keycode == 111) startup_snapshot(0);

    process_rotate_keys(event);

    int num_fields = gd.num_menu_fields;
    int field_index = -1;
    
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

      case ACTION_GO_COLUMN_BACKWARD : {      /* ACTION_UP */
	if(_mr->plane  < _mr->ds_fhdr.nz -1)
	  set_height( _mr->plane + 1);
      } break;
        
      case ACTION_GO_COLUMN_FORWARD : {         /* ACTION_DOWN */
	if(_mr->plane > 0)
	  set_height( _mr->plane - 1);
      } break;

      case ACTION_GO_CHAR_BACKWARD : {       /* ACTION_LEFT */
	// int curr_frame = (int)xv_get(gd.movie_pu->movie_frame_sl,
	// 			     PANEL_VALUE);
	int curr_frame = 0;
	
	if (curr_frame > 1) {
	  curr_frame--;
          
	  // xv_set(gd.movie_pu->movie_frame_sl,
	  //        PANEL_VALUE, curr_frame,
	  //        NULL);
	  
	  movie_frame_proc(gd.movie_pu->movie_frame_sl,
			   curr_frame,
			   (Event *)NULL);
	}
      } break;
        
      case ACTION_GO_CHAR_FORWARD : {         /* ACTION_RIGHT */
	// int curr_frame = (int)xv_get(gd.movie_pu->movie_frame_sl,
	// 			     PANEL_VALUE);
	// int max_frame = (int)xv_get(gd.movie_pu->movie_frame_sl,
        //                             PANEL_MAX_VALUE);
	int curr_frame = 0;
	int max_frame = 0;
	
	if (curr_frame < max_frame) {
	  curr_frame++;
          
	  // xv_set(gd.movie_pu->movie_frame_sl,
	  //        PANEL_VALUE, curr_frame,
	  //        NULL);
	  
	  movie_frame_proc(gd.movie_pu->movie_frame_sl,
			   curr_frame,
			   (Event *)NULL);
	}
	
      } break;
        
      default: {}

    } /* endswitch - event_action(event) */

  } /* endif - event_is_up */

}

// constrain azimuth to between 0 and 360 degrees

static double constrain_az(double az)

{
  if(az < 0.0) {
    return az + 360;
  } else if (az >= 360) {
    return az - 360;
  }
  return az;
}

#endif

