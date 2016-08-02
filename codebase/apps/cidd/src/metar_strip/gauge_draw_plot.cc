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
/*****************************************************************
 * GAUGE_DRAW_PLOT.C : Plotting functions 
 *
 */


#define GAUGE_DRAW_PLOT_C

#include "strip_chart.h"
using namespace std;

#define AXIS_BIT 0x0001  /* Single bit to control printing of an axis */
#define UNITS_BIT 0x0002 /* Single bit to control print units
			  * instead of time label*/
// static prototypes

static double compute_scale_range(double range);
static double compute_tick_interval(double range);
static void compute_min_and_max();
static double nearest(double target,double delta);

static void  draw_x_scale( SourceInfo &source,
			   int x_start, int x_end,
			   int y_start, int y_end,
			   int label_flag,
			   double &pix_x_unit, double &pix_x_bias);

static void  draw_y_scale( SourceInfo &source,
			   int x_start, int x_end,
			   int y_start, int y_end,
			   int label_flag,
			   double &pix_y_unit, double &pix_y_bias);
		    
static void plot_source( SourceInfo &source, int x_start, int x_end,
			 int y_start, int y_end, int label_flag);

static void plot_source_data(SourceInfo &source,
			     int x_start, int x_end,
			     int y_start, int y_end,
			     double pix_x_unit,double pix_x_bias,
			     double pix_y_unit,double pix_y_bias);
 
static void plot_flight_cat( SourceInfo &source, int x_start, int x_end,
			     int y_start, int y_end, int label_flag);

static void plot_flight_cat_data(SourceInfo &source,
				 int x_start, int x_end,
				 int y_start, int y_end,
				 double pix_x_unit,double pix_x_bias,
				 int yy_text_bot);
 
static int 
compute_flight_cat_index(WxObs &report);

/*****************************************************************
 * DRAW_PLOT: Supervise the plotting of all sources onto the screen
 */

void draw_plot()
{
  if(gd.variable <= Params::PRESSURE) {
    draw_common_plot();
  } else {
    draw_station_plot();
  }
}

/*****************************************************************
 * DRAW_COMMON_PLOT: Supervise the plotting of all sources onto the screen
 */

void draw_common_plot()
{

  int i;
  int y_start;
  int y_end;
  int plot_width;
  int plot_height;
  int label_flag;
  double each_height;
  
  plot_height = gd.win_height - gd.p->bottom_margin;
  plot_width = gd.win_width -  gd.p->right_margin;
  each_height = (double) plot_height / gd.num_sources;
  
  gd.data_min = 99999999.0;
  gd.data_max = -99999999.0;
  gd.data_range = 0.0;
  
  PMU_auto_register("Plotting(OK)");

  // compute min and max to make all source's scales identical */

  compute_min_and_max();

  // plot
  
  XSetForeground(gd.dpy, gd.def_gc, gd.bg_cell);
  XSetBackground(gd.dpy, gd.def_gc, gd.bg_cell);
  XFillRectangle(gd.dpy,gd.back_xid,gd.def_gc, 0,0, gd.win_width,gd.win_height);
  
  /* Draw NOW reference line */
  XSetForeground(gd.dpy, gd.def_gc, gd.now_cell);
  XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,plot_width-1,0,plot_width-1,plot_height);
  
  label_flag = 0;
  for(i=0; i < gd.num_sources; i++) {
    y_start = (int) ( i *  each_height + 0.499998);
    y_end =  (int) (((i+1) * each_height) - 0.5001);
    if(i == gd.num_sources -1) label_flag = 1;
    plot_source(gd.sources[i],0, plot_width,y_start,y_end,label_flag);
  }
  
  XCopyArea(gd.dpy,gd.back_xid,gd.canvas_xid,gd.def_gc,
	    0,0,gd.win_width,gd.win_height,0,0);
  
}

/*****************************************************************
 * DRAW_STATION_PLOT:
 * Supervise the plotting of a single station onto the screen
 */

void draw_station_plot()
{

  int i = 0;
  int station = 0;
  int store_variable = 0;
  int y_start = 0;
  int y_end = 0;
  int plot_width = 0;
  int plot_height = 0;
  int label_flag = 0;
  double each_height = 0.0;
  
  station = gd.variable - (Params::PRESSURE + 1);
  store_variable = gd.variable;
  
  plot_height = gd.win_height - gd.p->bottom_margin;
  plot_width = gd.win_width -  gd.p->right_margin;

  if (gd.p->plot_flight_category) {
    each_height = (double) plot_height / (Params::PRESSURE + 1);
  }
  else {
    each_height = (double) plot_height / (Params::PRESSURE);
  }
  
  PMU_auto_register("Plotting(OK)");
  
  /* First clear the screen */
  XSetForeground(gd.dpy, gd.def_gc, gd.bg_cell);
  XSetBackground(gd.dpy, gd.def_gc, gd.bg_cell);
  XFillRectangle(gd.dpy,gd.back_xid,gd.def_gc, 0,0,
		 gd.win_width,gd.win_height);

  /* Draw NOW reference line */
  XSetForeground(gd.dpy, gd.def_gc, gd.now_cell);
  XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,plot_width-1,0,
	    plot_width-1,plot_height);

  for(i=0; i <= Params::PRESSURE; i++) {

    if ((!gd.p->plot_flight_category) &&
	(i == Params::FLIGHT_CAT)) {
      continue;
    }

    gd.data_min = 99999999.0;
    gd.data_max = -99999999.0;
    gd.data_range = 0.0;
    
    label_flag = UNITS_BIT;
    y_end +=  (int) (each_height - 0.5001);
    if(i == Params::PRESSURE) label_flag |= AXIS_BIT;
    gd.variable = i;
    compute_min_and_max();
    plot_source(gd.sources[station],0, plot_width,y_start,y_end,label_flag);
    y_start += (int) (each_height + 0.499998);
  }
  
  XCopyArea(gd.dpy,gd.back_xid,gd.canvas_xid,gd.def_gc,
	    0,0,gd.win_width,gd.win_height,0,0);

  gd.variable = store_variable;

}
 
/*****************************************************************
 * DRAW_STATION_PLOT: Supervise the plotting of a single station
 * onto an xwd file screen
 */

void draw_station_plot_xwd()

{

  int i = 0;
  int station = 0;
  int store_variable = 0;
  int y_start = 0;
  int y_end = 0;
  int plot_width = 0;
  int plot_height = 0;
  int label_flag = 0;
  double each_height = 0.0;
  FILE *outfile;
  XWindowAttributes win_att;
  Window w;
  
  store_variable = gd.variable;

  plot_height = gd.win_height - gd.p->bottom_margin;
  plot_width = gd.win_width -  gd.p->right_margin;
  
  if (gd.p->plot_flight_category) {
    each_height = (double) plot_height / (Params::PRESSURE + 1);
  }
  else {
    each_height = (double) plot_height / (Params::PRESSURE);
  }

  PMU_auto_register("Plotting(OK)");
  w = xv_get(gd.Strip_chart_win1->win1,XV_XID);
  if(XGetWindowAttributes(gd.dpy,w,&win_att) == 0) {
    fprintf(stderr,"Problem getting window attributes\n");
    return;
  }

  for(station=0; station < gd.num_sources; station++) {

    /* First clear the screen */
    XSetForeground(gd.dpy, gd.def_gc, gd.bg_cell);
    XSetBackground(gd.dpy, gd.def_gc, gd.bg_cell);
    XFillRectangle(gd.dpy,gd.back_xid,gd.def_gc, 0,0,
		   gd.win_width,gd.win_height);

    /* Draw NOW reference line */
    XSetForeground(gd.dpy, gd.def_gc, gd.now_cell);
    XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,plot_width-1,0,
	      plot_width-1,plot_height);

    for(i=0; i <= Params::PRESSURE; i++) {

      if ((!gd.p->plot_flight_category) &&
	  (i == Params::FLIGHT_CAT)) {
	continue;
      }

      gd.data_min = 99999999.0;
      gd.data_max = -99999999.0;
      gd.data_range = 0.0;
      
      label_flag = UNITS_BIT;
      y_end +=  (int) (each_height - 0.5001);
      if(i == Params::PRESSURE) label_flag |= AXIS_BIT;
      gd.variable = i;
      compute_min_and_max();
      plot_source(gd.sources[station],0, plot_width,y_start,y_end,label_flag);
      y_start += (int) (each_height + 0.499998);
    }
    
    XCopyArea(gd.dpy,gd.back_xid,gd.canvas_xid,gd.def_gc,
	      0,0,gd.win_width,gd.win_height,0,0);
    
    if(strlen(gd.sources[station].station_info->xwd_path) > 3) {
      if((outfile =
	  fopen(gd.sources[station].station_info->xwd_path,"w")) == NULL) {
	fprintf(stderr,"Problem opening %s\n",
		gd.sources[station].station_info->xwd_path);
	perror("gauge_draw_plot");
	break;
      }
      
      XwuDumpWindow(gd.dpy,gd.back_xid,&win_att,outfile);
      
      if(fclose(outfile) !=0) {
	fprintf(stderr,"Problem closing %s\n",
		gd.sources[station].station_info->xwd_path);
	perror("strip_chart");
      }
      
      if(strlen(gd.sources[station].station_info->convert_command) > 3) {
	system(gd.sources[station].station_info->convert_command);
      }
    }

  } /* End of each station loop */

  gd.variable = store_variable;

}
 
/*****************************************************************
 * DRAW_PLOT_XWD: Supervise the plotting of all sources into xwd file
 *  Uses the backing XID to draw into
 */

void draw_plot_xwd()
{

  int i,k;
  int y_start;
  int y_end;
  int plot_width;
  int plot_height;
  int label_flag;
  int store_variable;
  double each_height;
  char *fname = NULL;
  char *cmd = NULL;
  FILE *outfile;
  XWindowAttributes win_att;
  Window w;
  
  xv_set(gd.Strip_chart_win1->win1,FRAME_BUSY,TRUE,NULL);
  
  store_variable = gd.variable;
  w = xv_get(gd.Strip_chart_win1->win1,XV_XID);
  if(XGetWindowAttributes(gd.dpy,w,&win_att) == 0) {
    fprintf(stderr,"Problem getting window attributes\n");
    return;
  }
  
  plot_height = gd.win_height - gd.p->bottom_margin;
  plot_width = gd.win_width -  gd.p->right_margin;
  each_height = (double) plot_height / gd.num_sources;
  
  for(k=0; k <= Params::PRESSURE; k++ ){  /* Plot each Variable */

    gd.data_min = 99999999.0;
    gd.data_max = -99999999.0;
    gd.data_range = 0.0;
    gd.variable = k;
    
    // compute min and max to make all source's scales identical */
    
    compute_min_and_max();
    
    /* First clear the pixmap */
    XSetForeground(gd.dpy, gd.def_gc, gd.bg_cell);
    XSetBackground(gd.dpy, gd.def_gc, gd.bg_cell);
    XFillRectangle(gd.dpy,gd.back_xid,gd.def_gc, 0,0,
		   gd.win_width,gd.win_height);  /* Plot each Variable */
    /* Draw NOW reference line */
    XSetForeground(gd.dpy, gd.def_gc, gd.now_cell);
    XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,plot_width-1,
	      0,plot_width-1,plot_height);
    
    label_flag = 0;
    for(i=0; i < gd.num_sources; i++) {
      y_start = (int) ( i *  each_height + 0.499998);
      y_end = (int) ( ((i+1) * each_height) - 0.5001);
      if(i == gd.num_sources -1) label_flag = 1;
      plot_source(gd.sources[i],0, plot_width,y_start,y_end,label_flag);
    }

    switch(gd.variable) {
    case Params::CEILING:
      fname = gd.p->ceiling_html.xwd_path;
      cmd =   gd.p->ceiling_html.convert_command;
      break;
    case Params::VISIBILITY:
      fname = gd.p->visibility_html.xwd_path;
      cmd =   gd.p->visibility_html.convert_command;
      break;
    case Params::FLIGHT_CAT:
      fname = gd.p->flight_cat_html.xwd_path;
      cmd =   gd.p->flight_cat_html.convert_command;
      break;
    case Params::TEMPERATURE:
      fname = gd.p->temperature_html.xwd_path;
      cmd =   gd.p->temperature_html.convert_command;
      break;
    case Params::HUMIDITY:
      fname = gd.p->humidity_html.xwd_path;
      cmd =   gd.p->humidity_html.convert_command;
      break;
    case Params::WIND_SPEED:
      fname = gd.p->wind_speed_html.xwd_path;
      cmd =   gd.p->wind_speed_html.convert_command;
      break;
    case Params::WIND_DIRN:
      fname = gd.p->wind_dirn_html.xwd_path;
      cmd =   gd.p->wind_dirn_html.convert_command;
      break;
    case Params::PRESSURE:
      fname = gd.p->pressure_html.xwd_path;
      cmd =   gd.p->pressure_html.convert_command;
      break;
    }

    if(strlen(fname) > 3) {
      if((outfile = fopen(fname,"w")) == NULL) {
	fprintf(stderr,"Problem opening %s\n",fname);
	perror("strip_chart");
	break;
      }

      XwuDumpWindow(gd.dpy,gd.back_xid,&win_att,outfile);

      if(fclose(outfile) !=0) {
	fprintf(stderr,"Problem closing %s\n",fname);
	perror("strip_chart");
      }
      
      if(strlen(cmd) > 3) {
	system(cmd);
      }
    }
  }

  /* restore the original variable */
  gd.variable = store_variable;
  
  xv_set(gd.Strip_chart_win1->win1,FRAME_BUSY,FALSE,NULL);

}

/*****************************************************************
 * PLOT_SOURCE: Supervise the plotting of one source in the space given
 *
 *
 */

static void plot_source( SourceInfo &source, int x_start, int x_end,
			 int y_start, int y_end, int label_flag)
{

  // special case for flight cat

  if (gd.variable == Params::FLIGHT_CAT && gd.p->plot_flight_category) {
    plot_flight_cat(source, x_start, x_end, y_start, y_end, label_flag);
    return;
  }

  double pix_y_unit,pix_y_bias; /* Slope & Y intercept for
				 * units to pixel space conv */
  double pix_x_unit,pix_x_bias; /* Slope & Y intercept for
				 * units to pixel space conv */
  int direct,ascent,descent;
  XCharStruct overall;
  char string_buf[128];

  /* Draw Dividng line */
  XSetForeground(gd.dpy, gd.def_gc, gd.fg_cell);
  XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x_start,y_end,x_end,y_end);

  // Draw scales and set up a mapping between pixel space
  // and time/data units 

  draw_x_scale(source, x_start, x_end, y_start, y_end, 
	       label_flag, pix_x_unit, pix_x_bias);
  
  if(source.obsArray.size() == 0) return;  /* bail out if no reports */
  
  draw_y_scale(source, x_start, x_end, y_start, y_end,
	       1, pix_y_unit, pix_y_bias);
  
  /* plot the data using the mapping between pixel space and time/data units */

  XSetForeground(gd.dpy, gd.def_gc, source.color_cell);
  plot_source_data(source, x_start, x_end, y_start, y_end,
		   pix_x_unit, pix_x_bias,
		   pix_y_unit, pix_y_bias);
  
  // Place station name and Units Label in the Upper left corner

  XSetForeground(gd.dpy, gd.def_gc, source.color_cell);

  const WxObs &obs = source.obsArray[0];
  
  switch(gd.variable) {
  case Params::CEILING:
    sprintf(string_buf,"%s Ceiling (ft)", obs.getStationId().c_str());
    break;
  case Params::VISIBILITY:
    sprintf(string_buf,"%s Visibility (Km)", obs.getStationId().c_str());
    break;
  case Params::TEMPERATURE:
    sprintf(string_buf,"%s Temperature (C)", obs.getStationId().c_str());
    break;
  case Params::HUMIDITY:
    sprintf(string_buf,"%s Humidity (%%)", obs.getStationId().c_str());
    break;
  case Params::WIND_SPEED:
    sprintf(string_buf,"%s Wind Speed (kts)", obs.getStationId().c_str());
    break;
  case Params::WIND_DIRN:
    sprintf(string_buf,"%s Wind Direction (T)", obs.getStationId().c_str());
    break;
  case Params::PRESSURE:
    sprintf(string_buf,"%s Pressure (hPa)", obs.getStationId().c_str());
    break;
  }        
  
  XTextExtents(gd.fontst, string_buf, strlen(string_buf),
	       &direct,&ascent,&descent,&overall);
  
  XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		   3,y_start + ascent  + 3,
		   string_buf ,strlen(string_buf));
    
}

/*****************************************************************
 * DRAW_X_SCALE: Set up a mapping between pixel space and time
 * And Draw scalesat along the bottom
 *
 */

static void draw_x_scale( SourceInfo &source,
			  int x_start, int x_end,
			  int y_start, int y_end,
			  int label_flag,
			  double &pix_x_unit, double &pix_x_bias)

{

  int x1;
  int text_period;
  int tick_period;
  time_t value;
  int direct,ascent,descent;
  XCharStruct overall;
  char string_buf[128];
  
  /* Determine mapping
   * use size of window, gd.p->pixels_per_minuts  to set range */

  double pixels_per_minute = gd.pixels_per_sec * 60.0;

  if (pixels_per_minute >= 2.0) {
    text_period = 1800;
  } else if (pixels_per_minute >= 1.0) {
    text_period = 3600;
  } else if (pixels_per_minute >= 0.5) {
    text_period = 7200;
  } else {
    text_period = 14400;
  }
  tick_period = text_period / 2;
  
  /* Compute the slope and intercept (bias) of the mapping function */
  pix_x_unit = gd.pixels_per_sec;  /* pixels per second */
  pix_x_bias = x_end - (pix_x_unit * (double)gd.end_time) - 3;

  /* Plot a tick every x minutes */
  value = gd.end_time - (gd.end_time % tick_period);
  while ( value > gd.start_time) {
    x1 = (int) (pix_x_unit * value + pix_x_bias);
    XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x1,y_end -3,x1,y_end);
    if(label_flag & AXIS_BIT) {
      /* Plot a time label every hour */
      if((value %text_period) == 0) { /* place a time label
				       *every now and then */
	date_time_t tt;
	tt.unix_time = value;
	uconvert_from_utime(&tt);
	sprintf(string_buf, "%.2d:%.2d", tt.hour, tt.min);
	XTextExtents(gd.fontst,string_buf,strlen(string_buf),
		     &direct,&ascent,&descent,&overall);
	
	if(gd.p->bottom_margin <=0) {  /* Draw the time lable in the
					* plotting area */
	  XDrawImageString(gd.dpy,gd.back_xid, gd.def_gc,
			   x1 - (overall.width/2),
			   y_end -3 ,string_buf ,strlen(string_buf));
	} else {  /* Draw the labels below - in the margin */
	  XDrawImageString(gd.dpy,gd.back_xid, gd.def_gc,
			   x1 - (overall.width/2),
			   y_end + 3 + ascent ,string_buf,
			   strlen(string_buf));
	}
      }
    }
    value -= tick_period;  /* every 15 minutes */
  } // while

  if(label_flag & AXIS_BIT && (gd.p->bottom_margin > 30)) {
    XSetForeground(gd.dpy, gd.def_gc, gd.fg_cell);
    strcpy(string_buf,"TIME:");
    XTextExtents(gd.fontst,string_buf,
		 strlen(string_buf),&direct,&ascent,&descent,&overall);
    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     0 ,y_end + 2 * (ascent + 3),
		     string_buf ,strlen(string_buf));
    
    sprintf(string_buf, "Ends at: %s", utimstr(gd.end_time));
    XTextExtents(gd.fontst,string_buf,
		 strlen(string_buf),&direct,&ascent,&descent,&overall);
    XSetForeground(gd.dpy, gd.def_gc, gd.now_cell);
    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     gd.win_width - overall.width -2 ,
		     y_end + 2 * (ascent + 3),
		     string_buf ,strlen(string_buf));
    XSetForeground(gd.dpy, gd.def_gc, gd.fg_cell);
  }
  
}

/*****************************************************************
 * COMPUTE_MIN_AND_MAX: compute min and max values to get the
 * plots all the same if required.
 *
 */

/* Data must be between these vaues to be valid */

#define TOO_BIG 20000.0
#define TOO_SMALL -1000.0

static void compute_min_and_max()
		    
{

  gd.data_min = 99999999.0;
  gd.data_max = -99999999.0;
  
  for(int isrc = 0; isrc < gd.num_sources; isrc++) {

    const SourceInfo &source = gd.sources[isrc];

    for(int i = 0; i < (int) source.obsArray.size(); i++) {

      if ((time_t) source.obsArray[i].getObservationTime() < gd.start_time ||
	  (time_t) source.obsArray[i].getObservationTime() > gd.end_time) {
	continue;
      }

      fl32 field_val = 0.0;
      
      switch(gd.variable) {
      case Params::CEILING:
	field_val = source.obsArray[i].getCeilingKm() * 3048.0;
	if (gd.p->apply_ceiling && field_val > gd.p->ceiling_threshold) {
	  field_val = gd.p->ceiling_threshold;
	}
	break;
      case Params::VISIBILITY:
	field_val = source.obsArray[i].getVisibilityKm();
	if (gd.p->apply_visibility && field_val > gd.p->visibility_threshold) {
	  field_val = gd.p->visibility_threshold;
	}
	break;
      case Params::TEMPERATURE:
	field_val = source.obsArray[i].getTempC();
	break;
      case Params::HUMIDITY:
	field_val = source.obsArray[i].getRhPercent();
	break;
      case Params::WIND_SPEED:
	field_val = source.obsArray[i].getWindSpeedMps();
	break;
      case Params::WIND_DIRN:
	field_val = source.obsArray[i].getWindDirnDegt();
	break;
      case Params::PRESSURE:
	field_val = source.obsArray[i].getSeaLevelPressureMb();
	if (field_val > 1500) continue;
	if (field_val < 500) continue;
	break;
      } // switch
      
      if(field_val == WxObs::missing) continue;
      if(field_val >= TOO_BIG) continue;
      if(field_val <= TOO_SMALL) continue;

      if (field_val > gd.data_max) {
	gd.data_max = field_val;
      }
      if (field_val < gd.data_min) {
	gd.data_min = field_val;
      }
      
    } // i
    
    if (gd.variable == Params::CEILING) {
      gd.data_min = 0.0;
    } else if (gd.variable == Params::VISIBILITY) {
      gd.data_min = 0.0;
    } else if (gd.variable == Params::WIND_DIRN) {
      gd.data_max = 360.0;
      gd.data_min = 0.0;
    } else if (gd.variable == Params::WIND_SPEED) {
      gd.data_min = 0.0;
    }

  } // isrc

  gd.data_range = gd.data_max - gd.data_min;

}
    
/*****************************************************************
 * DRAW_Y_SCALE: Set up a mapping between pixel space and data units
 * And Draw vertical scales at the center and right edge.
 *
 */

static void draw_y_scale( SourceInfo &source,
			  int x_start, int x_end,
			  int y_start, int y_end,
			  int label_flag,
			  double &pix_y_unit, double &pix_y_bias)
		    
{

  int y1;
  double range, scale_range; 
  double tick_base,tick_delta;
  double value;
  int direct,ascent,descent;
  XCharStruct overall;
  char string_buf[128];
  
  range = gd.data_max - gd.data_min;
  if (range < 5.0) {
    range = 5.0;
  }

  /* Use GLobal ranges to keep all sources on same scale *
   * Based on the range of the data
   * Pick a  nice even range that span the data */
  
  scale_range = compute_scale_range(range);
  
  /* Pick a nice even tick delta */
  tick_delta = compute_tick_interval(range);

  double average = (gd.data_min + gd.data_max) / 2.0;
    
  /* center the range over the average */
  tick_base = average - (scale_range / 2.0);
  
  if(tick_base > gd.data_min) tick_base = gd.data_min;  
  
  /* adjust the scale bottom to the nearest tick_delta units */
  tick_base  = nearest(tick_base,tick_delta);
  if(tick_base > gd.data_min) tick_base -= tick_delta;  
  if (gd.variable == Params::CEILING ||
      gd.variable == Params::VISIBILITY ||
      gd.variable == Params::WIND_DIRN ||
      gd.variable == Params::WIND_SPEED) {
    tick_base = 0;
  }
  
  while(tick_base + scale_range < gd.data_max)  {
    scale_range += tick_delta;
  }
  if(scale_range > gd.data_range) gd.data_range = scale_range;
  if(scale_range < gd.data_range) scale_range = gd.data_range;
  
  if(gd.p->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr,"Min: %g, Max: %g Range: %g\n",
	    gd.data_min,gd.data_max,range);
  }
  
  if(gd.p->debug >= Params::DEBUG_VERBOSE) {

    fprintf(stderr,"\nSource: %s %d points\n",
	    source.obsArray[0].getStationId().c_str(), source.obsArray.size());
    fprintf(stderr,"Scale_range: %g Range: %g\n", scale_range, range);

    string fieldName;
    if (gd.variable == Params::CEILING) {
      fieldName = "";
    } else if (gd.variable == Params::VISIBILITY) {
      fieldName = "VISIBILITY";
    } else if (gd.variable == Params::FLIGHT_CAT) {
      fieldName = "FLIGHT_CAT";
    } else if (gd.variable == Params::TEMPERATURE) {
      fieldName = "TEMPERATURE";
    } else if (gd.variable == Params::HUMIDITY) {
      fieldName = "HUMIDITY";
    } else if (gd.variable == Params::WIND_SPEED) {
      fieldName = "WIND_SPEED";
    } else if (gd.variable == Params::WIND_DIRN) {
      fieldName = "WIND_DIRN";
    } else if (gd.variable == Params::PRESSURE) {
      fieldName = "PRESSURE";
    }

    fprintf(stderr,"Field name: %s\n", fieldName.c_str());
    fprintf(stderr,"Tick_base: %g, tick_delta: %g, Scale_range: %g\n",
	    tick_base,tick_delta,scale_range);
  }
  
  /* Compute the slope and intercept (bias) of the mapping function */

  pix_y_unit = -(y_end - y_start -1)/ scale_range;
  pix_y_bias = y_end - (pix_y_unit * tick_base);

  /* Draw two sets of tick marks; right, and left */
  for(value = tick_base; value <= (tick_base + scale_range);
      value += tick_delta) {
    y1 = (int) (pix_y_unit * value + pix_y_bias);
    XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x_end,y1,x_end-3,y1);
    XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x_start,y1,x_start+3,y1);
  }
  
  /* Label base of scale */
  sprintf(string_buf,"%.0f",tick_base);
  XTextExtents(gd.fontst,string_buf,strlen(string_buf),
	       &direct,&ascent,&descent,&overall);
  XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		   (x_end - 3 - overall.width),y_end - 2 ,
		   string_buf ,strlen(string_buf));

  /* Label top of scale */
  sprintf(string_buf,"%.0f",(tick_base + scale_range));
  XTextExtents(gd.fontst,string_buf,strlen(string_buf),
	       &direct,&ascent,&descent,&overall);
  XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		   (x_end - 3 - overall.width),
		   (y_start + 2 + overall.ascent),
		   string_buf ,strlen(string_buf));
 
}

static int 
compute_flight_cat_index(WxObs &report)

{

  if(report.getCeilingKm() == WxObs::missing ||
     report.getVisibilityKm() == WxObs::missing) {
    return -1;
  }

  for (int i = 0; i < gd.p->flight_category_n; i++) {
    double ceiling = report.getCeilingKm() * 3048.0;
    double visibility = report.getVisibilityKm();
    if (ceiling >= gd.p->_flight_category[i].ceiling_threshold &&
	visibility >= gd.p->_flight_category[i].visibility_threshold) {
      return i;
    }
  }
  
  return -1;

}

/*************************************************************************
 * NEAREST: Compute the value nearest the target which is divisible by
 *         the absolute value of delta
 */

static double
nearest(double target,double delta)
{
    double answer;
    double rem;

    delta = fabs(delta);
    rem = fmod(target,delta);

    if(target >= 0.0) {
        if(rem > (delta / 2.0)) {
    	   answer = target + (delta - rem);
        } else {
          answer = target -  rem;
        }
    } else {
        if(fabs(rem) > (delta / 2.0)) {
    	   answer = target - (delta + rem);
        } else {
          answer = target -  rem;
        }
    }
      
    return answer;
}
 
/*************************************************************************
 * COMPUTE_TICK_INTERVAL: Return the tick interval given a range
 *        Range Assumed to be > 1.0 && < 10000.0
 */

static double
compute_tick_interval(double range)
{
    double    arange = fabs(range);

    if(arange <= 0.5) return (0.1);
    if(arange <= 1.0) return (0.25);
    if(arange <= 2.0) return (0.5);
    if(arange <= 10.0) return (2.0);
    if(arange <= 10.0) return (2.0);
    if(arange <= 30.0) return (5.0);
    if(arange <= 50.0) return (10.0);
    if(arange <= 100.0) return (20.0);
    if(arange <= 300.0) return (50.0);
    if(arange == 360.0) return (90.0);
    if(arange <= 1500.0) return (100.0);
    if(arange <= 3000.0) return (200.0);
    if(arange <= 7500.0) return (500.0);
    return(1000.0);
}
 
/*************************************************************************
 * COMPUTE_SCALE_RANGE: Return a range for a scale based on the dynimic range 
 *        Range Assumed to be > 1.0 && < 10000.0
 */

static double
compute_scale_range(double range)
{
    double    arange = fabs(range);

    if(arange <= 1.0) return (1.0);
    if(arange <= 2.0) return (2.0);
    if(arange <= 5.0) return (5.0);
    if(arange <= 10.0) return (10.0);
    if(arange <= 20.0) return (20.0);
    if(arange <= 25.0) return (25.0);
    if(arange <= 40.0) return (40.0);
    if(arange <= 50.0) return (50.0);
    if(arange <= 100.0) return (100.0);
    if(arange <= 200.0) return (200.0);
    if(arange <= 360.0) return (360.0);
    if(arange <= 400.0) return (400.0);
    if(arange <= 500.0) return (500.0);
    return(1000.0);
}

/*************************************************************************
 * PLOT_SOURCE_DATA: Plot the data on the pixmap given the data to pixel transform
 *   values 
 *
 */

static void plot_source_data(SourceInfo &source,
			     int x_start, int x_end,
			     int y_start, int y_end,
			     double pix_x_unit,double pix_x_bias,
			     double pix_y_unit,double pix_y_bias)
 
{

    int i;
    int n_points;
    XPoint *bpt = new XPoint[source.obsArray.size()];
    
    n_points = 0;

    for(i=0; i < (int) source.obsArray.size(); i++) {

    if ((time_t) source.obsArray[i].getObservationTime() < gd.start_time ||
	(time_t) source.obsArray[i].getObservationTime() > gd.end_time) {
      continue;
    }

    fl32 field_val = 0.0;

    switch(gd.variable) {
    case Params::CEILING:
	field_val = source.obsArray[i].getCeilingKm() * 3048.0;
	if (gd.p->apply_ceiling && field_val > gd.p->ceiling_threshold) {
	  field_val = gd.p->ceiling_threshold;
	}
	break;
      case Params::VISIBILITY:
	field_val = source.obsArray[i].getVisibilityKm();
	if (gd.p->apply_visibility && field_val > gd.p->visibility_threshold) {
	  field_val = gd.p->visibility_threshold;
	}
	break;
    case Params::TEMPERATURE:
      field_val = source.obsArray[i].getTempC();
      break;
    case Params::HUMIDITY:
      field_val = source.obsArray[i].getRhPercent();
      break;
    case Params::WIND_SPEED:
      field_val = source.obsArray[i].getWindSpeedMps();
      break;
    case Params::WIND_DIRN:
      field_val = source.obsArray[i].getWindDirnDegt();
      break;
    case Params::PRESSURE:
      field_val = source.obsArray[i].getSeaLevelPressureMb();
      if (field_val > 1500) continue;
      if (field_val < 500) continue;
      break;
    } // switch

    if(field_val == WxObs::missing) continue;
    if(field_val >= TOO_BIG) continue;
    if(field_val <= TOO_SMALL) continue;

    bpt[n_points].x = (short int)
      ((source.obsArray[i].getObservationTime()) * pix_x_unit + pix_x_bias);
    bpt[n_points].y = (short int) (field_val * pix_y_unit + pix_y_bias);
    n_points++;

  } // i

  if(n_points > 0) {
    XSetLineAttributes(gd.dpy, gd.def_gc, gd.p->trace_line_width,
		       LineSolid, CapButt, JoinBevel);
    XDrawLines(gd.dpy,gd.back_xid,gd.def_gc,bpt,n_points,CoordModeOrigin);
    XSetLineAttributes(gd.dpy, gd.def_gc, 1,
		       LineSolid, CapButt, JoinBevel);
  }
  
  delete[] bpt;

}

/*****************************************************************
 * PLOT_SOURCE: Supervise the plotting of one source in the space given
 *
 *
 */

static void plot_flight_cat( SourceInfo &source, int x_start, int x_end,
			     int y_start, int y_end, int label_flag)

{

  double pix_x_unit,pix_x_bias; /* Slope & Y intercept for
				 * units to pixel space conv */

  int direct,ascent,descent;
  XCharStruct overall;
  char string_buf[128];

  /* Draw Dividng line */
  XSetForeground(gd.dpy, gd.def_gc, gd.fg_cell);
  XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x_start,y_end,x_end,y_end);

  // Draw scales and set up a mapping between pixel space
  // and time/data units 

  draw_x_scale(source, x_start, x_end, y_start, y_end, 
	       label_flag, pix_x_unit, pix_x_bias);
  
  if(source.obsArray.size() == 0) return;  /* bail out if no reports */
  
  // Place station name and Units Label in the Upper left corner

  XSetForeground(gd.dpy, gd.def_gc, source.color_cell);
  WxObs &obs = source.obsArray[0];
  int xx = 3;

  sprintf(string_buf,"%s Flight Category", obs.getStationId().c_str());
  XTextExtents(gd.fontst, string_buf, strlen(string_buf),
	       &direct,&ascent,&descent,&overall);
  XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		   xx, y_start + ascent  + 3,
		   string_buf ,strlen(string_buf));
  xx += XTextWidth(gd.fontst, string_buf, strlen(string_buf));

  // place legends

  XSetLineAttributes(gd.dpy, gd.def_gc, ascent,
		     LineSolid, CapButt, JoinBevel);

  int yy_text_mid = y_start + 3 + ascent / 2;
  int yy_text_bot = y_start + 3 + ascent;
  
  for (int i = 0; i < gd.p->flight_category_n; i++) {

    xx += 15;
    char label[128];
    sprintf(label, "%s:", gd.p->_flight_category[i].label_str);
    XSetForeground(gd.dpy, gd.def_gc, source.color_cell);
    XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc,
		     xx, yy_text_bot,
		     label, strlen(label));
    xx += XTextWidth(gd.fontst, label, strlen(label));

    xx += 2;
    XSetForeground(gd.dpy, gd.def_gc, gd.fcat_cells[i]);
    XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid,
	      xx + 15, yy_text_mid);
    xx += 15;
    
  } // i
    
  XSetLineAttributes(gd.dpy, gd.def_gc, 1,
		     LineSolid, CapButt, JoinBevel);
  
  /* plot the data using the mapping between pixel space and time/data units */

  plot_flight_cat_data(source, x_start, x_end, y_start, y_end,
		       pix_x_unit, pix_x_bias, yy_text_bot);
  
}

/*************************************************************************
 * PLOT_FLIGHT_CAT_DATA:
 *
 * Plot the data on the pixmap given the data to pixel transform values 
 *
 */

static void plot_flight_cat_data(SourceInfo &source,
				 int x_start, int x_end,
				 int y_start, int y_end,
				 double pix_x_unit,double pix_x_bias,
				 int yy_text_bot)
  
{

  int nn = source.obsArray.size();
  if (nn < 2) {
    return;
  }

  // load up flight cat data

  int fcat[nn];
  for(int i = 0; i < nn; i++) {
    fcat[i] = compute_flight_cat_index(source.obsArray[i]);
  }

  // line position and width

  int yy = (yy_text_bot + y_end) / 2;
  XSetLineAttributes(gd.dpy, gd.def_gc, gd.p->flight_category_line_width,
		     LineSolid, CapButt, JoinBevel);
  
  // plot data

  for(int i = 0; i < nn; i++) {

    if (fcat[i] < 0) {
      continue;
    }
    
    time_t t1, t2;
    if (i == 0) {
      t1 = source.obsArray[0].getObservationTime();
      t2 = source.obsArray[0].getObservationTime() + (source.obsArray[1].getObservationTime() - source.obsArray[0].getObservationTime()) / 2;
    } else if (i == nn - 1) {
      t1 = source.obsArray[nn - 2].getObservationTime() + (source.obsArray[nn - 1].getObservationTime() - source.obsArray[nn - 2].getObservationTime()) / 2;
      t2 = source.obsArray[nn - 1].getObservationTime();
    } else {
      t1 = source.obsArray[i - 1].getObservationTime() + ( source.obsArray[i].getObservationTime() - source.obsArray[i - 1].getObservationTime()) / 2;
      t2 = source.obsArray[i].getObservationTime() + (source.obsArray[i + 1].getObservationTime() - source.obsArray[i].getObservationTime()) / 2;
    }

    XSetForeground(gd.dpy, gd.def_gc, gd.fcat_cells[fcat[i]]);

    short int x1 = (short int) (t1 * pix_x_unit + pix_x_bias);
    short int x2 = (short int) (t2 * pix_x_unit + pix_x_bias);
    
    XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, x1, yy, x2, yy);

  } // i

  XSetLineAttributes(gd.dpy, gd.def_gc,
		     1, LineSolid, CapButt, JoinBevel);
    
}

