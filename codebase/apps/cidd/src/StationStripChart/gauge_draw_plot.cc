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
 * GAUGE_DRAW_PLOT. Plotting functions 
 *
 */


#define GAUGE_DRAW_PLOT_C

#include "strip_chart.h"
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <didss/LdataInfo.hh>

using namespace std;

#define AXIS_BIT  0x0001 /* Single bit to control printing of an axis */
#define UNITS_BIT 0x0002 /* Single bit to control printing of the field's units */
#define TIME_BIT  0x0004 /* Single bit to control printing of time label*/

void dump_image(Display *dpy, Drawable xid, int width, int height, 
        char* outputDir, char *fname, time_t fileTime, bool writeLDataInfo);

// static prototypes

static double compute_scale_range(double range);
static double compute_tick_interval(double range);
static void compute_min_and_max();
static void compute_field_min_and_max(int src);
static double nearest(double target,double delta);

static void  draw_x_scale( source_info_t* source,
			   int x_start, int x_end,
			   int y_start, int y_end,
			   int label_flag,
			   double &pix_x_unit, double &pix_x_bias);

static void  draw_y_scale( source_info_t* source,
			   int x_start, int x_end,
			   int y_start, int y_end,
			   int label_flag,
			   double &pix_y_unit, double &pix_y_bias);
		    
static void plot_source( source_info_t* source, int x_start, int x_end,
			 int y_start, int y_end, int label_flag);

static void plot_source_data(source_info_t* source,
			     int x_start, int x_end,
			     int y_start, int y_end,
			     double pix_x_unit,double pix_x_bias,
			     double pix_y_unit,double pix_y_bias);
 
static void plot_flight_cat( source_info_t* source, int x_start, int x_end,
			     int y_start, int y_end, int label_flag);

static void plot_flight_cat_data(source_info_t* source,
				 int x_start, int x_end,
				 int y_start, int y_end,
				 double pix_x_unit,double pix_x_bias,
				 int yy_text_bot);

static void plot_fzra( source_info_t* source, int x_start, int x_end,
			     int y_start, int y_end, int label_flag);

static void plot_fzra_data(source_info_t* source,
				 int x_start, int x_end,
				 int y_start, int y_end,
				 double pix_x_unit,double pix_x_bias,
				 int yy_text_bot);
 

static void plot_wx_type( source_info_t* source, int x_start, int x_end,
			     int y_start, int y_end, int label_flag);

static void plot_wx_type_data(source_info_t* source,
				 int x_start, int x_end,
				 int y_start, int y_end,
				 double pix_x_unit,double pix_x_bias,
				 int yy_text_bot);
 
int compute_flight_cat_index(station_report_t &report);

/*****************************************************************
 * DRAW_PLOT: Supervise the plotting of one variable of
 * all sources onto the screen
 */

void draw_plot()
{
  if(gd.ready == 0) return;

  xv_set(gd.Strip_chart_win1->win1,FRAME_BUSY,TRUE,NULL);
  xv_set(gd.Strip_chart_config_pu->config_pu,FRAME_BUSY,TRUE,NULL);

  if(gd.cur_plot < gd.num_active_fields) {
    gd.variable = gd.field_lookup[gd.cur_plot];
    draw_common_plot();
  } else {
    draw_station_plot();
  }

  xv_set(gd.Strip_chart_win1->win1,FRAME_BUSY,FALSE,NULL);
  xv_set(gd.Strip_chart_config_pu->config_pu,FRAME_BUSY,FALSE,NULL);
}

/*****************************************************************
 * DRAW_COMMON_PLOT: Supervise the plotting of all sources, the
 * selected variable onto the screen
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


  int num_stations;
  if(gd.p->max_display_stations < gd.num_sources) {
      num_stations = gd.p->max_display_stations;
  } else {
      num_stations = gd.num_sources;
  }
  each_height = (double) plot_height / num_stations;
  
  gd.data_min = 99999999.0;
  gd.data_max = -99999999.0;
  gd.data_range = 0.0;
  
  PMU_auto_register("Plotting(OK)");


  // plot
  
  XSetForeground(gd.dpy, gd.def_gc, gd.bg_cell);
  XSetBackground(gd.dpy, gd.def_gc, gd.bg_cell);
  XFillRectangle(gd.dpy,gd.back_xid,gd.def_gc, 0,0, gd.win_width,gd.win_height);
  
  /* Draw NOW reference line */
  XSetForeground(gd.dpy, gd.def_gc, gd.now_cell);
  XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,plot_width-1,0,plot_width-1,plot_height);
  
  // compute min and max to make all source's scales identical */
  compute_min_and_max();

  label_flag = UNITS_BIT;
  for(i=0; i < num_stations; i++) {
    y_start = (int) ( i *  each_height + 0.499998);
    y_end =  (int) (((i+1) * each_height) - 0.5001);
    if(i == 0) label_flag |= TIME_BIT;
    if(i == num_stations -1) label_flag |= AXIS_BIT;
    plot_source(gd.sorted_sources[i],0, plot_width,y_start,y_end,label_flag);
  }
  
  XCopyArea(gd.dpy,gd.back_xid,gd.canvas_xid,gd.def_gc,
	    0,0,gd.win_width,gd.win_height,0,0);

  
}

/*****************************************************************
 * DRAW_STATION_PLOT: * Supervise the plotting of a single station,
 * all variables onto the screen
 */

void draw_station_plot()
{

  int i;
  int station;
  int y_start;
  int y_end;
  int plot_width;
  int plot_height;
  int label_flag;
  double each_height;
  
  station = gd.cur_plot - gd.num_active_fields;

  plot_height = gd.win_height - gd.p->bottom_margin;
  plot_width = gd.win_width -  gd.p->right_margin;
  each_height = (double) plot_height / gd.num_active_fields;
  
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

  for(i=0; i < gd.num_active_fields; i++) {
    gd.data_min = 99999999.0;
    gd.data_max = -99999999.0;
    gd.data_range = 0.0;
    
    label_flag = UNITS_BIT;
    y_start = (int) ( i *  each_height + 0.499998);
    y_end =  (int) (((i+1) * each_height) - 0.5001);
    if(i == 0) label_flag |= TIME_BIT;
    if(i == gd.num_active_fields -1 ) label_flag |= AXIS_BIT;
    gd.variable = gd.field_lookup[i];
      
    compute_field_min_and_max(station);

    gd.data_min = gd.sorted_sources[station]->data_min;
    gd.data_max = gd.sorted_sources[station]->data_max;
    gd.data_range = gd.sorted_sources[station]->data_range;
    plot_source(gd.sorted_sources[station],0, plot_width,y_start,y_end,label_flag);
  }
  

  XCopyArea(gd.dpy,gd.back_xid,gd.canvas_xid,gd.def_gc,
	    0,0,gd.win_width,gd.win_height,0,0);
}
 
/*****************************************************************
 * DRAW_STATION_PLOT: Supervise the plotting of all stations
 * onto separate image files . Each plot has all variables for each
 * station.
 */

void draw_station_plot_images()

{

  int i;
  int station;
  int y_start;
  int y_end;
  int plot_width;
  int plot_height;
  int label_flag;
  double each_height;

  plot_height = gd.win_height - gd.p->bottom_margin;
  plot_width = gd.win_width -  gd.p->right_margin;
  each_height = (double) plot_height / gd.num_active_fields;
  
  int num_stations;
  if(gd.p->max_display_stations < gd.num_sources) {
      num_stations = gd.p->max_display_stations;
  } else {
      num_stations = gd.num_sources;
  }
  PMU_auto_register("Plotting(OK)");

  for(station=0; station < num_stations; station++) {

    /* First clear the screen */
    XSetForeground(gd.dpy, gd.def_gc, gd.bg_cell);
    XSetBackground(gd.dpy, gd.def_gc, gd.bg_cell);
    XFillRectangle(gd.dpy,gd.back_xid,gd.def_gc, 0,0,
		   gd.win_width,gd.win_height);

    /* Draw NOW reference line */
    XSetForeground(gd.dpy, gd.def_gc, gd.now_cell);
    XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,plot_width-1,0,
	      plot_width-1,plot_height);

    for(i=0; i < gd.num_active_fields ; i++) {
      gd.data_min = 99999999.0;
      gd.data_max = -99999999.0;
      gd.data_range = 0.0;
      
      label_flag = UNITS_BIT;  // Always add units.
	  if(i == 0) label_flag |= TIME_BIT; // Add the time in for the first.
	  // Draw the time axis for the last.
      if(i == gd.num_active_fields -1 ) label_flag |= AXIS_BIT;

      y_start = (int) ( i *  each_height + 0.499998);
      y_end = (int) ( ((i+1) * each_height) - 0.5001);
      gd.variable = gd.field_lookup[i];

      // compute min and max to make all source's scales identical */
      compute_min_and_max();

      plot_source(&gd.sources[station],0, plot_width,y_start,y_end,label_flag);
    }
    
   // Show progress if window is exposed.
   if(gd.p->show_window) {
	   XCopyArea(gd.dpy,gd.back_xid,gd.canvas_xid,gd.def_gc,
	      0,0,gd.win_width,gd.win_height,0,0);
   }
    
   char time_string[256];
   struct tm res;
   char fname[MAX_PATH_LEN];
   char file_path[MAX_PATH_LEN];
   char cmd[MAX_PATH_LEN];
   
   //  Build the output file name 
   strftime(time_string,256,gd.p->filename_prefix, gmtime_r(&gd.end_time,&res));

   sprintf(fname,"%s_%s.%s", time_string,
		   gd.sources[station].station_info->name,
		   gd.p->image_output_type);

   sprintf(file_path,"%s/%s",gd.p->html_output_dir,fname);
   //
   //
   // Build the post processing command
   sprintf(cmd,"%s %s",gd.p->html_image_post_process_script, file_path);

   dump_image(gd.dpy, gd.back_xid, gd.win_width, gd.win_height,
              gd.p->html_output_dir, fname, gd.end_time, gd.p->write_ldata_info);
   
   safe_system(cmd,gd.p->post_process_timeout);

  } /* End of each station loop */
}
 
/*****************************************************************
 * DRAW_PLOT_IMAGES: Supervise the plotting of all variables 
 *  into separate plots. Each station is one row in plots.
 */

void draw_common_plot_images()
{

  int i,k;
  int y_start;
  int y_end;
  int plot_width;
  int plot_height;
  int label_flag;
  double each_height;
  
  xv_set(gd.Strip_chart_win1->win1,FRAME_BUSY,TRUE,NULL);
  xv_set(gd.Strip_chart_config_pu->config_pu,FRAME_BUSY,TRUE,NULL);
  
  plot_height = gd.win_height - gd.p->bottom_margin;
  plot_width = gd.win_width -  gd.p->right_margin;
  int num_stations;
  if(gd.p->max_display_stations < gd.num_sources) {
      num_stations = gd.p->max_display_stations;
  } else {
      num_stations = gd.num_sources;
  }
  each_height = (double) plot_height / num_stations;
  
  for(k=0; k < gd.num_active_fields ; k++ ){  /* Plot each Variable */

    gd.data_min = 99999999.0;
    gd.data_max = -99999999.0;
    gd.data_range = 0.0;
    
    gd.variable = gd.field_lookup[k];

    
    /* First clear the pixmap */
    XSetForeground(gd.dpy, gd.def_gc, gd.bg_cell);
    XSetBackground(gd.dpy, gd.def_gc, gd.bg_cell);
    XFillRectangle(gd.dpy,gd.back_xid,gd.def_gc, 0,0,
		   gd.win_width,gd.win_height);  /* Plot each Variable */
    /* Draw NOW reference line */
    XSetForeground(gd.dpy, gd.def_gc, gd.now_cell);
    XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,plot_width-1,
	      0,plot_width-1,plot_height);
    
    compute_min_and_max(); // Find global min& max's

    label_flag = UNITS_BIT;
    for(i=0; i < num_stations; i++) {
      y_start = (int) ( i *  each_height + 0.499998);
      y_end = (int) ( ((i+1) * each_height) - 0.5001);
      if(i == 0) label_flag |= TIME_BIT;
      if(i == num_stations -1) label_flag |= AXIS_BIT;
      plot_source(gd.sorted_sources[i],0, plot_width,y_start,y_end,label_flag);
    }
    
   // Show progress if window is exposed.
   if(gd.p->show_window) {
	   XCopyArea(gd.dpy,gd.back_xid,gd.canvas_xid,gd.def_gc,
	      0,0,gd.win_width,gd.win_height,0,0);
   }
    
   char *fname = NULL;

    switch(gd.variable) {
    case Params::RATE:
      fname = "Rate";
      break;
    case Params::ACCUMULATION:
      fname = "Accum";
      break;
    case Params::ACCUMULATION2:
      fname = "Accum2";
      break;
    case Params::CEILING:
      fname = "Ceil";
      break;
    case Params::VISIBILITY:
      fname = "Vis";
      break;
    case Params::FLIGHT_CAT:
      fname = "FLTCAT";
      break;
    case Params::TEMPERATURE:
      fname = "Temp";
      break;
    case Params::HUMIDITY:
      fname = "Humid";
      break;
    case Params::DEWPT:
      fname = "Dewpt";
      break;
    case Params::WIND_SPEED:
      fname = "Speed";
      break;
    case Params::WIND_DIRN:
      fname = "Wind";
      break;
    case Params::PRESSURE:
      fname = "Pres";
      break;
    case Params::SPARE1:
      fname = "Spare1";
      break;
    case Params::SPARE2:
      fname = "Spare2";
      break;
    case Params::FZ_PRECIP:
      fname = "FZRA";
      break;
    case Params::PRECIP_TYPE:
      fname = "WX";
      break;
    }

   char time_string[256];
   char filename[MAX_PATH_LEN];
   struct tm res;
   char file_path[MAX_PATH_LEN];
   char cmd[MAX_PATH_LEN];
   
   //  Build the output file name 
   strftime(time_string,256,gd.p->filename_prefix, gmtime_r(&gd.end_time,&res));

   sprintf(filename, "%s_%s.%s",time_string,
                   fname,
                   gd.p->image_output_type);   

   sprintf(file_path,"%s/%s",gd.p->html_output_dir, filename);

   // Build the post processing command
   sprintf(cmd,"%s %s",gd.p->html_image_post_process_script, file_path);

   dump_image(gd.dpy, gd.back_xid,gd.win_width, gd.win_height, gd.p->html_output_dir, 
            filename, gd.end_time,gd.p->write_ldata_info);
     
   safe_system(cmd,gd.p->post_process_timeout);

  }

  xv_set(gd.Strip_chart_win1->win1,FRAME_BUSY,FALSE,NULL);
  xv_set(gd.Strip_chart_config_pu->config_pu,FRAME_BUSY,FALSE,NULL);

}

/*****************************************************************
 * PLOT_SOURCE: Supervise the plotting of one source,
 * all enabled variables in the space given
 */

static void plot_source( source_info_t* source, int x_start, int x_end,
			 int y_start, int y_end, int label_flag)
{

  // special case for flight cat
  if (gd.variable == Params::FLIGHT_CAT) {
    plot_flight_cat(source, x_start, x_end, y_start, y_end, label_flag);
    return;
  }

  // special case for Freezing Precip 
  if (gd.variable == Params::FZ_PRECIP) {
    plot_fzra(source, x_start, x_end, y_start, y_end, label_flag);
    return;
  }


  // special case for Precip type 
  if (gd.variable == Params::PRECIP_TYPE) {
    plot_wx_type(source, x_start, x_end, y_start, y_end, label_flag);
    return;
  }

  double pix_y_unit,pix_y_bias; /* Slope & Y intercept for
				 * units to pixel space conv */
  double pix_x_unit,pix_x_bias; /* Slope & Y intercept for
				 * units to pixel space conv */
  int direct,ascent,descent;
  XCharStruct overall;
  char string_buf[256];
  station_report_t *report;

  /* Draw Dividng line */
  XSetForeground(gd.dpy, gd.def_gc, gd.fg_cell);
  XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x_start,y_end,x_end,y_end);

  // Draw scales and set up a mapping between pixel space
  // and time/data units 

  draw_x_scale(source, x_start, x_end, y_start, y_end, 
	       label_flag, pix_x_unit, pix_x_bias);
  
  if(source->num_reports == 0) return;  /* bail out if no reports */
  
  /* plot the data using the mapping between pixel space and time/data units */

  draw_y_scale(source, x_start, x_end, y_start, y_end,
	       1, pix_y_unit, pix_y_bias);
  
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  plot_source_data(source, x_start, x_end, y_start, y_end,
		   pix_x_unit, pix_x_bias,
		   pix_y_unit, pix_y_bias);
  
  // Place station name and Units Label in the Upper left corner

  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  
  report = source->reports;
  
  *string_buf = '\0'; // make sure to null terminate 
  if(label_flag & UNITS_BIT) {
	char *ptr;
    switch(gd.variable) {
      default:
      case Params::SPARE1:
	    ptr = "Spare1";
		for(int i = 0; i < gd.p->display_variable_n; i++) {
			  if(gd.p->_display_variable[i].field == gd.variable) {
				  ptr = gd.p->_display_variable[i].label_str;
			  }
		}
        sprintf(string_buf,"%s %s", report->station_label,ptr);
      break;

      case Params::SPARE2:
	    ptr = "Spare2";
		for(int i = 0; i < gd.p->display_variable_n; i++) {
			  if(gd.p->_display_variable[i].field == gd.variable) {
				  ptr = gd.p->_display_variable[i].label_str;
			  }
		}
        sprintf(string_buf,"%s %s", report->station_label,ptr);
      break;

      case Params::ACCUMULATION2:
	    ptr = "Precip Accum2";
		for(int i = 0; i < gd.p->display_variable_n; i++) {
			  if(gd.p->_display_variable[i].field == gd.variable) {
				  ptr = gd.p->_display_variable[i].label_str;
			  }
		}
        sprintf(string_buf,"%s %s", report->station_label,ptr);
      break;

      case Params::RATE:
        switch (gd.p->units) {
	  case Params::METRIC:
            sprintf(string_buf,"%s Precip Rate (mm/hr)", report->station_label);
	  break;
	  case Params::ENGLISH:
            sprintf(string_buf,"%s Precip Rate (in/hr)", report->station_label);
	  break;
	}
      break;

      case Params::ACCUMULATION:
        switch (gd.p->units) {
	  case Params::METRIC:
            sprintf(string_buf,"%s Precip Accumulation (mm)", report->station_label);
	  break;
	  case Params::ENGLISH:
            sprintf(string_buf,"%s Precip Accumulation (in)", report->station_label);
	  break;
	}
      break;

      case Params::CEILING:
        sprintf(string_buf,"%s Ceiling (ft)", report->station_label);
      break;

      case Params::VISIBILITY:
        switch (gd.p->units) {
	  case Params::METRIC:
            sprintf(string_buf,"%s Visibility (Km)", report->station_label);
	  break;
	  case Params::ENGLISH:
            sprintf(string_buf,"%s Visibility (Mi)", report->station_label);
	  break;
	}
      break;

      case Params::TEMPERATURE:
        switch (gd.p->units) {
	  case Params::METRIC:
            sprintf(string_buf,"%s Temperature (C)", report->station_label);
	  break;
	  case Params::ENGLISH:
            sprintf(string_buf,"%s Temperature (F)", report->station_label);
	  break;
	}
      break;

      case Params::DEWPT:
        switch (gd.p->units) {
	  case Params::METRIC:
            sprintf(string_buf,"%s Dew Point (C)", report->station_label);
	  break;
	  case Params::ENGLISH:
            sprintf(string_buf,"%s Dew Point (F)", report->station_label);
	  break;
	}
      break;

      case Params::HUMIDITY:
        sprintf(string_buf,"%s Humidity (%%)", report->station_label);
      break;

      case Params::WIND_SPEED:
        switch (gd.p->units) {
	  case Params::METRIC:
            sprintf(string_buf,"%s Wind Speed (kts)", report->station_label);
	  break;
	  case Params::ENGLISH:
            sprintf(string_buf,"%s Wind Speed (mph)", report->station_label);
	  break;
	}
      break;

      case Params::WIND_DIRN:
        sprintf(string_buf,"%s Wind Direction (Deg)", report->station_label);
      break;

      case Params::PRESSURE:
        sprintf(string_buf,"%s Pressure (mbar)", report->station_label);
      break;
    }        
  }

  if(label_flag & TIME_BIT) {
      time_t plot_start_time;
      time_t plot_end_time; 
      struct tm res;
      char time_string[64];
      char time_string2[64];

      if(source->num_reports > 0) {
	 plot_start_time = report->time;  // from first report
	 plot_end_time = source->reports[source->num_reports -1].time;  // last report
      } else {
	 plot_start_time = (time_t) ((x_start  - pix_x_bias) / pix_x_unit);
	 plot_end_time = (time_t) ((x_end  - pix_x_bias) / pix_x_unit);
      }

      if(plot_start_time < plot_end_time && ((x_end - x_start) > 400) ) {
	  if(gd.p->use_localtime) {
	    strftime(time_string,64," %D %H:%M to ",localtime_r(&plot_start_time,&res));
	  } else {
	    strftime(time_string,64," %D %H:%M to ",gmtime_r(&plot_start_time,&res));
	  }
	  strncat(string_buf,time_string,64);

	  if(gd.p->use_localtime) {
	    strftime(time_string2,64,"%D %H:%M %Z",localtime_r(&plot_end_time,&res));
	  } else {
	    strftime(time_string2,64,"%D %H:%M UTC",gmtime_r(&plot_end_time,&res));
	  }
	  strncat(string_buf,time_string2,64);

      } else if((x_end - x_start) > 250) {
	    if(gd.p->use_localtime) {
	      strftime(time_string,64," %D %H:%M %Z",localtime_r(&plot_end_time,&res));
	    } else {
	      strftime(time_string,64," %D %H:%M UTC",gmtime_r(&plot_end_time,&res));
		}
	  strncat(string_buf,time_string,64);
      } 

  }
  
  XTextExtents(gd.fontst, string_buf, strlen(string_buf),
	       &direct,&ascent,&descent,&overall);
  
  XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		   3,y_start + ascent  + 3,
		   string_buf ,strlen(string_buf));
    
}

/*****************************************************************
 * DRAW_X_SCALE: Set up a mapping between pixel space and time
 * And Draw scales along the bottom
 *
 */

static void draw_x_scale( source_info_t* source,
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
  struct tm res;
  
  /* Determine mapping
   * use size of window, gd.p->pixels_per_minuts  to set range */

  double pixels_per_minute = gd.pixels_per_sec * 60.0;

  if (pixels_per_minute >= 2.0) {
    text_period = 1800;
  } else if (pixels_per_minute >= 1.0) {
    text_period = 3600;
  } else if (pixels_per_minute >= 0.5) {
    text_period = 7200;
  } else if (pixels_per_minute >= 0.25) {
    text_period = 14400;
  } else if (pixels_per_minute >= 0.125) {
    text_period = 28800;
  } else if (pixels_per_minute >= 0.0625) {
    text_period = 86400;
  } else if (pixels_per_minute >= 0.03125) {
    text_period = 172800;
  } else {
    text_period = 345600;
  }
  tick_period = text_period / 2;
  
  /* Compute the slope and intercept (bias) of the mapping function */
  pix_x_unit = gd.pixels_per_sec;  /* pixels per second */
  pix_x_bias = x_end - (pix_x_unit * (double)gd.end_time) - 3;

  struct tm *v_tm;
  int zone_offset = 0;
  if(gd.p->use_localtime && (text_period >= 14400)) {
	  v_tm = localtime(&value);
	  zone_offset = v_tm->tm_gmtoff;
  }
  /* Plot a tick every x minutes - Pick the latest point to label */
  value = gd.end_time - (gd.end_time % tick_period) - zone_offset;
  

  while ( value > gd.start_time) {
    x1 = (int) (pix_x_unit * value + pix_x_bias);
    XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x1,y_end -3,x1,y_end);
    if(label_flag & AXIS_BIT) {
     
      if((value  % text_period) == abs(zone_offset)) { // place a time label

    	if(gd.p->use_localtime) {
		v_tm = localtime_r(&value,&res);
	} else {
		v_tm = gmtime_r(&value,&res);
    	}

	if(v_tm->tm_hour  == 0 && v_tm->tm_min == 0) {
		strftime(string_buf,128,"%m/%d",v_tm);
	} else {
		strftime(string_buf,128,"%H:%M",v_tm);
	}
	XTextExtents(gd.fontst,string_buf,strlen(string_buf),
		     &direct,&ascent,&descent,&overall);
	
	if(gd.p->bottom_margin <=0) {  /* Draw the time label in the
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
    value -= tick_period;  
  } // while (value > gd.start_time)

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

#define TOO_BIG 200000.0
#define TOO_SMALL -1000.0

static void compute_min_and_max()
		    
{

  gd.data_min = 99999999.0;
  gd.data_max = -99999999.0;
  gd.n_points = 0;

  switch(gd.variable) {

    case Params::WIND_DIRN:
      gd.data_max = 360.0;
      gd.data_min = 0.0;
      gd.data_range = 360.0;
      gd.n_points = 1;
      return;
    break;

    case Params::Params::HUMIDITY:
      gd.data_max = 100.0;
      gd.data_min = 0.0;
      gd.data_range = 100.0;
      gd.n_points = 1;
      return;
    break;

    default: // Do nothing.
    break;
  };


  int num_stations;
  if(gd.p->max_display_stations < gd.num_sources) {
      num_stations = gd.p->max_display_stations;
  } else {
      num_stations = gd.num_sources;
  }
  
  for(int src = 0; src <  num_stations; src++) {

     compute_field_min_and_max(src);

     source_info_t *source = gd.sorted_sources[src];

     if (source->data_max > gd.data_max) { gd.data_max = source->data_max; }
     if (source->data_min < gd.data_min) { gd.data_min = source->data_min; }

     if((source->data_max - source->data_min) >  gd.data_range) {
          gd.data_range  = gd.data_max - gd.data_min;
     }
      
  } // src
}
    
/*****************************************************************
 * COMPUTE_FIELD_MIN_AND_MAX: compute min and max values to get the
 * plots all the same if required.
 *
 */

/* Data must be between these vaues to be valid */

#define TOO_BIG 200000.0
#define TOO_SMALL -1000.0

static void compute_field_min_and_max(int src)
		    
{

  source_info_t *source = gd.sorted_sources[src];
  source->data_min = 99999999.0;
  source->data_max = -99999999.0;

  // Return fixed values for these fields.
  switch(gd.variable) {
    case Params::WIND_DIRN:
      source->data_max = 360.0;
      source->data_min = 0.0;
      source->data_range = 360.0;
      return;
    break;

    case Params::Params::HUMIDITY:
      source->data_max = 100.0;
      source->data_min = 0.0;
      source->data_range = 100.0;
      return;
    break;

    default: // Do nothing.
    break;
  };
   
  int first_valid = source->num_reports - 1;
  fl32 first_val = 0.0;

    for(int i = 0; i < source->num_reports; i++) {

      if ((time_t) source->reports[i].time < gd.start_time ||
	  (time_t) source->reports[i].time > gd.end_time) {
	continue;
      }

      fl32 field_val = 0.0;
      
      switch(gd.variable) {
      case Params::RATE:
	field_val = source->reports[i].precip_rate;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val /= INCHES_TO_MM;
	  break;
	}
	break;

      case Params::ACCUMULATION:
	field_val = source->reports[i].liquid_accum;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val /= INCHES_TO_MM;
	  break;
	}
	if(i < first_valid && field_val != STATION_NAN) {
	    first_valid = i;
	    first_val = field_val;  // Record the starting val
	}
	// Bias so left edge is 0.0
	if(field_val != STATION_NAN) {
	  field_val -= first_val;
	  // Accum should never go negative, so clamp to 0.0
	  if(field_val < 0.0) field_val = 0.0;
	}
	break;

      case Params::ACCUMULATION2:
	field_val = source->reports[i].shared.station.liquid_accum2;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val /=  INCHES_TO_MM;
	  break;
	}
	break;

      case Params::SPARE1:
	field_val = source->reports[i].shared.station.Spare1;
	if(field_val != STATION_NAN) {
		field_val *= gd.p->spare1_scale;
		field_val += gd.p->spare1_bias;
	}
	break;

      case Params::SPARE2:
	field_val = source->reports[i].shared.station.Spare2;
	if(field_val != STATION_NAN) {
		field_val *= gd.p->spare2_scale;
		field_val += gd.p->spare2_bias;
	}
	break;

      case Params::CEILING:
	field_val = source->reports[i].ceiling;
	if(field_val != STATION_NAN) { 
	    field_val *=  3048.0;  // FT per KM. - Convert to Feet.
	    // Clamp to  25,000 ft.
	    if (field_val > 25000.0) field_val = 25000.0;
		if(field_val < 0.0) field_val = 0.0;
	}
	break;

      case Params::VISIBILITY:
	field_val = source->reports[i].visibility;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val /=  KM_PER_MI;
	  break;
	}
	break;

      case Params::DEWPT:
	field_val = source->reports[i].dew_point;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val = TEMP_C_TO_F(field_val);
	  break;
	}
	break;

      case Params::TEMPERATURE:
	field_val = source->reports[i].temp;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val = TEMP_C_TO_F(field_val);
	  break;
	}
	break;

      case Params::HUMIDITY:
	field_val = source->reports[i].relhum;
	break;

      case Params::WIND_SPEED:
	field_val = source->reports[i].windspd;
        switch (gd.p->units) {
	  case Params::METRIC:
	    if(field_val != STATION_NAN) field_val *=  NMH_PER_MS;
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val /=  MPH_TO_MS ;
	  break;
	}
	break;

      case Params::WIND_DIRN:
	field_val = source->reports[i].winddir;
	break;

      case Params::PRESSURE:
	field_val = source->reports[i].pres;
	if (field_val > 1500) continue;
	if (field_val < 500) continue;
	break;

      } // switch
      
      if(field_val == STATION_NAN) continue;
      if(field_val >= TOO_BIG) continue;
      if(field_val <= TOO_SMALL) continue;

      gd.n_points++;

      if (field_val > source->data_max) { source->data_max = field_val; }
      if (field_val < source->data_min) { source->data_min = field_val; }
      
    } // i

   // Some Variables have pre-defined min's max's
   switch(gd.variable) {
    case Params::CEILING:
    case Params::WIND_SPEED:
    case Params::VISIBILITY:
    case Params::ACCUMULATION:
      source->data_min = 0.0;
      if(source->data_max == -99999999.0)source->data_max = 1.0;
    break;

    case Params::RATE:
  	source->data_min = 0.0;
        switch (gd.p->units) {
	  case Params::METRIC:
	    if(source->data_max < gd.p->min_prate_range)  source->data_max = gd.p->min_prate_range;
	  break;
	  case Params::ENGLISH:
	    if(source->data_max < gd.p->min_prate_range/INCHES_TO_MM)
	          source->data_max = gd.p->min_prate_range / INCHES_TO_MM;
	  break;
	}
	 
    default: 
      if(source->data_min == 99999999.0 && source->data_max == -99999999.0){
	      source->data_min = 0.0;
	      source->data_max = 1.0;
      }
    break;
  };


  source->data_range = source->data_max - source->data_min;
   if(gd.p->debug >= Params::DEBUG_VERBOSE) {
       fprintf(stderr,"Source Min: %g, Max: %g Range: %g\n",
              source->data_min, source->data_max, source->data_range);
   }

}
    
/*****************************************************************
 * DRAW_Y_SCALE: Set up a mapping between pixel space and data units
 * And Draw vertical scales at the center and right edge.
 *
 */

static void draw_y_scale( source_info_t* source,
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

  /* Use GLobal ranges to keep all sources on same scale *
   * Based on the range of the data
   * Pick a  nice even range that span the data */
  
  scale_range = compute_scale_range(range);
  
  /* Pick a nice even tick delta */
  tick_delta = compute_tick_interval(scale_range);

  if(gd.data_min == 0.0) {
     tick_base = 0.0;
  } else {
      double average = (gd.data_min + gd.data_max) / 2.0;
      /* center the range over the average */
      tick_base = average - (scale_range / 2.0);
      if(tick_base > gd.data_min) tick_base = gd.data_min;  
  }
  
  
  /* adjust the scale bottom to the nearest tick_delta units */
  tick_base  = nearest(tick_base,tick_delta);
  if(tick_base > gd.data_min) tick_base -= tick_delta;  
  if(gd.data_min == 0.0 ) tick_base = 0.0;  
  
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
    if (source->num_reports > 0) {
      fprintf(stderr,"\nSource: %s %d points\n",
	      source->reports[0].station_label, source->num_reports);
      fprintf(stderr,"Scale_range: %g Range: %g\n", scale_range, range);
    } else {
      fprintf(stderr, "no reports\n");
    }
    fprintf(stderr,"Tick_base: %g, tick_delta: %g, Scale_range: %g\n",
	    tick_base,tick_delta,scale_range);
  }
  
  /* Compute the slope and intercept (bias) of the mapping function */

  pix_y_unit = -(y_end - y_start -1)/ scale_range;
  pix_y_bias = y_end - (pix_y_unit * tick_base);

  if(gd.n_points)  {
    /* Draw two sets of tick marks; right, and left */
    for(value = tick_base; value < (tick_base + scale_range);
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
    if(fabs(scale_range) < 0.01) {
        sprintf(string_buf,"%0.3f",(tick_base + scale_range));
    } else if(fabs(scale_range) < 0.1) {
        sprintf(string_buf,"%.2f",(tick_base + scale_range));
    } else if(scale_range < 1.0) {
        sprintf(string_buf,"%.1f",(tick_base + scale_range));
    } else {
        sprintf(string_buf,"%.0f",(tick_base + scale_range));
    }
    XTextExtents(gd.fontst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);
    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     (x_end - 3 - overall.width),
		     (y_start + 2 + overall.ascent),
		     string_buf ,strlen(string_buf));
  } else {
    //  Add a no data label
    sprintf(string_buf,"No Sensor/Data");
    XTextExtents(gd.fontst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);

    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     (x_end - 3 - overall.width),
		     ((y_start + y_end) / 2  - 1),
		     string_buf ,strlen(string_buf));
  }

}

/*************************************************************************
 * COMPUTE_FLIGHT_CAT_INDEX
 */
int compute_flight_cat_index(station_report_t &report)

{

  if(report.ceiling == STATION_NAN ||
     report.visibility == STATION_NAN) {
    return -1;
  }

  for (int i = 0; i < gd.p->flight_category_n; i++) {
    double ceiling = report.ceiling * 3048.0;
    double visibility = report.visibility;
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
 *        Range Assumed to be > .10 && < 100000.0
 */

static double
compute_tick_interval(double range)
{
    double    arange = fabs(range);

    if(arange <= 0.01) return (0.002);
    if(arange <= 0.02) return (0.005);
    if(arange <= 0.05) return (0.01);
    if(arange <= 0.1) return (0.02);
    if(arange <= 0.2) return (0.05);
    if(arange <= 0.5) return (0.1);
    if(arange <= 1.0) return (0.25);
    if(arange <= 2.0) return (0.5);
    if(arange <= 5.0) return (1.0);
    if(arange <= 10.0) return (2.0);
    if(arange <= 30.0) return (5.0);
    if(arange <= 50.0) return (10.0);
    if(arange <= 100.0) return (20.0);
    if(arange <= 300.0) return (50.0);
    if(arange == 360.0) return (90.0);
    if(arange <= 1500.0) return (100.0);
    if(arange <= 3000.0) return (200.0);
    if(arange <= 7500.0) return (500.0);
    if(arange <= 15000.0) return (1000.0);
    if(arange <= 30000.0) return (2000.0);
    return(5000.0);
}
 
/*************************************************************************
 * COMPUTE_SCALE_RANGE: Return a range for a scale based on the dynimic range 
 *        Range Assumed to be > .10 && < 50000.0
 */

static double
compute_scale_range(double range)
{
    double    arange = fabs(range);

    if(arange <= .010) return (.010);
    if(arange <= .020) return (.020);
    if(arange <= .050) return (.050);
    if(arange <= .10) return (.10);
    if(arange <= .20) return (.20);
    if(arange <= .50) return (.50);
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
    if(arange <= 1000.0) return (1000.0);
    if(arange <= 2000.0) return (2000.0);
    if(arange <= 5000.0) return (5000.0);
    if(arange <= 10000.0) return (10000.0);
    if(arange <= 20000.0) return (20000.0);
    if(arange <= 25000.0) return (25000.0);
    if(arange <= 30000.0) return (30000.0);
    if(arange <= 40000.0) return (40000.0);
    return(50000.0);
}

/*************************************************************************
 * PLOT_SOURCE_DATA: Plot the data on the pixmap given the data to pixel transform
 *   values 
 *
 */

static void plot_source_data(source_info_t* source,
			     int x_start, int x_end,
			     int y_start, int y_end,
			     double pix_x_unit,double pix_x_bias,
			     double pix_y_unit,double pix_y_bias)
 
{
  int i;
  int n_points;
  int first_valid = source->num_reports -1;
  XPoint *bpt = new XPoint[source->num_reports];
    
  n_points = 0;
  fl32 field_val = 0.0;
  fl32 last_field_val = 0.0;
  fl32 first_val = 0.0;

  for(i=0; i < source->num_reports; i++) {

    if ((time_t) source->reports[i].time < gd.start_time ||
	(time_t) source->reports[i].time > gd.end_time) {
      continue;
    }


    switch(gd.variable) {
      case Params::RATE:
	field_val = source->reports[i].precip_rate;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val /=  INCHES_TO_MM;
	  break;
	}
	break;
      case Params::ACCUMULATION:
	field_val = source->reports[i].liquid_accum;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val /= INCHES_TO_MM;
	  break;
	}
	if(i < first_valid && field_val != STATION_NAN) {
	    first_valid = i;
	    first_val = field_val;  // Record the starting val
	}
	// Bias so left edge is 0.0
	if(field_val != STATION_NAN) {
	  field_val -= first_val;
	  // Accum should never go negative, so clamp to 0.0
	  if(field_val < 0.0) field_val = 0.0;
	}
	break;
	
      case Params::ACCUMULATION2:
	field_val = source->reports[i].shared.station.liquid_accum2;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val /= INCHES_TO_MM;
	  break;
	}
	break;
      case Params::SPARE1:
	field_val = source->reports[i].shared.station.Spare1;
	if(field_val != STATION_NAN) {
		field_val *= gd.p->spare1_scale;
		field_val += gd.p->spare1_bias;
	}
	break;
      case Params::SPARE2:
	field_val = source->reports[i].shared.station.Spare2;
	if(field_val != STATION_NAN) {
		field_val *= gd.p->spare2_scale;
		field_val += gd.p->spare2_bias;
	}
	break;

    case Params::CEILING:
      field_val = source->reports[i].ceiling * 3048.0;
      if (field_val > 12500.0) { field_val = 12500.0; }
      break;
    case Params::VISIBILITY:
	field_val = source->reports[i].visibility;
	// added to offset pwd data output irregularities occasionally causing "20000" to be read in as "0000" -AG
	//if (field_val == 0.0) { field_val = 20.0; }
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val /=  KM_PER_MI;
	  break;
	}
      break;
    case Params::TEMPERATURE:
	field_val = source->reports[i].temp;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val = TEMP_C_TO_F(field_val);
	  break;
	}
      break;
    case Params::DEWPT:
	field_val = source->reports[i].dew_point;
        switch (gd.p->units) {
	  case Params::METRIC:
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN) field_val = TEMP_C_TO_F(field_val);
	  break;
	}
      break;
    case Params::HUMIDITY:
      field_val = source->reports[i].relhum;
      break;
    case Params::WIND_SPEED:
	field_val = source->reports[i].windspd;
        switch (gd.p->units) {
	  case Params::METRIC:
	    if(field_val != STATION_NAN)
	    field_val *= NMH_PER_MS;
	  break;
	  case Params::ENGLISH:
	    if(field_val != STATION_NAN)
	    field_val /=  MPH_TO_MS;
	  break;
	}
      break;
    case Params::WIND_DIRN:
      field_val = source->reports[i].winddir;
	  // If the wind shifts across the scale - Plot what we have.
	  // Start a new line with the new point. - Avoids connecting the line as
	  // it crosses 0/360.
	  if(i > 0 && fabs(field_val - last_field_val) > 270.0) {
		  if(n_points == 1) { // If it's a single point, give it some dimension
             bpt[n_points].x = (short int) ((source->reports[i].time) * pix_x_unit + pix_x_bias);
             bpt[n_points].y = bpt[n_points -1].y;
			 n_points++;
		  }
          XSetLineAttributes(gd.dpy, gd.def_gc, gd.p->trace_line_width, LineSolid, CapButt, JoinBevel);
          XDrawLines(gd.dpy,gd.back_xid,gd.def_gc,bpt,n_points,CoordModeOrigin);
          XSetLineAttributes(gd.dpy, gd.def_gc, 1, LineSolid, CapButt, JoinBevel);
		  n_points = 0;
	  }
	  last_field_val = field_val;

      break;
    case Params::PRESSURE:
      field_val = source->reports[i].pres;
      if (field_val > 1500) continue;
      if (field_val < 500) continue;
      break;
    } // switch

    if(field_val == STATION_NAN) continue;
    if(field_val >= TOO_BIG) continue;
    if(field_val <= TOO_SMALL) continue;

    last_field_val = field_val;

    bpt[n_points].x = (short int) ((source->reports[i].time) * pix_x_unit + pix_x_bias);
    bpt[n_points].y = (short int) (field_val * pix_y_unit + pix_y_bias);
    n_points++;

  } // i



  if(n_points > 0) {
    gd.n_points = n_points;
    XSetLineAttributes(gd.dpy, gd.def_gc, gd.p->trace_line_width, LineSolid, CapButt, JoinBevel);
    XDrawLines(gd.dpy,gd.back_xid,gd.def_gc,bpt,n_points,CoordModeOrigin);
    XSetLineAttributes(gd.dpy, gd.def_gc, 1, LineSolid, CapButt, JoinBevel);
  }

  if(gd.p->right_margin > 20) { // Plot the data value in the margin.
    int direct,ascent,descent;
    int ypos;
    XCharStruct overall;
    char string_buf[128];

    if(fabs(last_field_val) < 0.1) {
        sprintf(string_buf,"%.3f",last_field_val);
	// Strip off leading 0
	if(strncmp(string_buf,"0.0",3) == 0 ) {
		strcpy(string_buf,string_buf+1);
	}
	
    } else if(fabs(last_field_val) < 1.0) {
        sprintf(string_buf,"%.2f",last_field_val);
    } else if(last_field_val < 100.0) {
        sprintf(string_buf,"%.1f",last_field_val);
    } else {
        sprintf(string_buf,"%.0f",last_field_val);
    }

    XTextExtents(gd.fontst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);

    // Place the label mid point at the data - Y value
    ypos = (int)((field_val * pix_y_unit + pix_y_bias) + (ascent / 2));
    //
    // Make sure lebel stays in our area.
    if(ypos + descent > y_end) ypos = y_end - descent;
    if(ypos - ascent < y_start) ypos = y_start + ascent;

    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     (x_end + 1), ypos,
		     string_buf,strlen(string_buf));
  }

  delete[] bpt;

}

/*****************************************************************
 *  PLOT_FLIGHT_CAT
 */

static void plot_flight_cat( source_info_t* source, int x_start, int x_end,
			     int y_start, int y_end, int label_flag)

{

  double pix_x_unit,pix_x_bias; /* Slope & Y intercept for
				 * units to pixel space conv */

  int direct,ascent,descent;
  XCharStruct overall;
  char string_buf[128];
  station_report_t *report;

  /* Draw Dividng line */
  XSetForeground(gd.dpy, gd.def_gc, gd.fg_cell);
  XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x_start,y_end,x_end,y_end);

  // Draw scales and set up a mapping between pixel space
  // and time/data units 

  draw_x_scale(source, x_start, x_end, y_start, y_end, 
	       label_flag, pix_x_unit, pix_x_bias);
  
  if(source->num_reports == 0) return;  /* bail out if no reports */
  
  // Place station name and Units Label in the Upper left corner

  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  report = source->reports;
  int xx = 3;

  sprintf(string_buf,"%s Flight Category", report->station_label);
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
    XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
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

static void plot_flight_cat_data(source_info_t* source,
				 int x_start, int x_end,
				 int y_start, int y_end,
				 double pix_x_unit,double pix_x_bias,
				 int yy_text_bot)
  
{

  int nn = source->num_reports;
  int last_fcat_index = -1;
  if (nn < 2) {
    //  Add a no data label
    int direct,ascent,descent;
    XCharStruct overall;

    XTextExtents(gd.fontst,"No Data",7,
		 &direct,&ascent,&descent,&overall);

    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     (x_end - 3 - overall.width),
		     ((y_start + y_end) / 2  - 1),
		     "No Data" ,7);
    return;
  }

  // load up flight cat data

  int fcat[nn];
  for(int i = 0; i < nn; i++) {
    fcat[i] = compute_flight_cat_index(source->reports[i]);
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
      t1 = source->reports[0].time;
      t2 = source->reports[0].time + (source->reports[1].time - source->reports[0].time) / 2;
    } else if (i == nn - 1) {
      t1 = source->reports[nn - 2].time + (source->reports[nn - 1].time - source->reports[nn - 2].time) / 2;
      t2 = source->reports[nn - 1].time;
    } else {
      t1 = source->reports[i - 1].time + ( source->reports[i].time - source->reports[i - 1].time) / 2;
      t2 = source->reports[i].time + (source->reports[i + 1].time - source->reports[i].time) / 2;
    }

    XSetForeground(gd.dpy, gd.def_gc, gd.fcat_cells[fcat[i]]);

    short int x1 = (short int) (t1 * pix_x_unit + pix_x_bias);
    short int x2 = (short int) (t2 * pix_x_unit + pix_x_bias);
    
    XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, x1, yy, x2, yy);

    last_fcat_index = fcat[i];

  } // i

  XSetLineAttributes(gd.dpy, gd.def_gc,
		     1, LineSolid, CapButt, JoinBevel);
    
  if(gd.p->right_margin > 20) { // Plot the data value in the margin.
    int direct,ascent,descent;
    int ypos;
    XCharStruct overall;
    char string_buf[128];

    strcpy(string_buf," ");

    if(last_fcat_index >= 0) {
	    strcpy(string_buf,gd.p->_flight_category[last_fcat_index].label_str);
    }
    
    XTextExtents(gd.fontst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);

    // Place the label at the mid point
    ypos = (int)(((y_start + y_end) / 2) + (ascent  - descent)  / 2);

    XSetForeground(gd.dpy, gd.def_gc, source->color_cell);

    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     (x_end + 1), ypos,
		     string_buf,strlen(string_buf));
  }
}

/*****************************************************************
 *  PLOT_FZRA
 */

static void plot_fzra( source_info_t* source, int x_start, int x_end,
			     int y_start, int y_end, int label_flag)

{

  double pix_x_unit,pix_x_bias; /* Slope & Y intercept for
				 * units to pixel space conv */

  int direct,ascent,descent;
  XCharStruct overall;
  char string_buf[128];
  station_report_t *report;

  /* Draw Dividing line */
  XSetForeground(gd.dpy, gd.def_gc, gd.fg_cell);
  XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x_start,y_end,x_end,y_end);

  // Draw scales and set up a mapping between pixel space
  // and time/data units 

  draw_x_scale(source, x_start, x_end, y_start, y_end, 
	       label_flag, pix_x_unit, pix_x_bias);
  
  if(source->num_reports == 0) return;  /* bail out if no reports */
  
  // Place station name and Units Label in the Upper left corner

  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  report = source->reports;
  int xx = 3;

  sprintf(string_buf,"%s FZ Precip ", report->station_label);
  XTextExtents(gd.fontst, string_buf, strlen(string_buf),
	       &direct,&ascent,&descent,&overall);
  XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc, xx, y_start + ascent  + 3,
		   string_buf ,strlen(string_buf));
  xx += XTextWidth(gd.fontst, string_buf, strlen(string_buf));

  // place legends

  XSetLineAttributes(gd.dpy, gd.def_gc, ascent, LineSolid, CapButt, JoinBevel);

  int yy_text_mid = y_start + 3 + ascent / 2;
  int yy_text_bot = y_start + 3 + ascent + ascent;
  
  xx += 15;
  char label[128];

  sprintf(label, "Nil");
  XSetForeground(gd.dpy, gd.def_gc, gd.no_fzra_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx+5, yy_text_mid, xx + 20, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx += 35;

  sprintf(label, "FR");
  XSetForeground(gd.dpy, gd.def_gc, gd.frost_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=25;

  sprintf(label, "FZFG");
  XSetForeground(gd.dpy, gd.def_gc, gd.fzfg_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx+10, yy_text_mid, xx + 25, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=45;

  sprintf(label, " -FZDZ+ ");
  XSetForeground(gd.dpy, gd.def_gc, gd.lt_fzdz_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx+15, yy_text_mid, xx + 30, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=15;

  XSetForeground(gd.dpy, gd.def_gc, gd.fzdz_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx+15, yy_text_mid, xx + 30, yy_text_mid);
  xx +=15;
  XSetForeground(gd.dpy, gd.def_gc, gd.hv_fzdz_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx+15, yy_text_mid, xx + 30, yy_text_mid);
  xx += 50;

  sprintf(label, " -FZRA+ ");
  XSetForeground(gd.dpy, gd.def_gc, gd.lt_fzra_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx+15, yy_text_mid, xx + 30, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));

  xx +=15;
  XSetForeground(gd.dpy, gd.def_gc, gd.fzra_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx+15, yy_text_mid, xx + 30, yy_text_mid);
  xx +=15;
  XSetForeground(gd.dpy, gd.def_gc, gd.hv_fzra_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx+15, yy_text_mid, xx + 30, yy_text_mid);
  xx += 25;
  

  /* plot the data using the mapping between pixel space and time/data units */
  XSetLineAttributes(gd.dpy, gd.def_gc, 1, LineSolid, CapButt, JoinBevel);
  plot_fzra_data(source, x_start, x_end, y_start, y_end,
		       pix_x_unit, pix_x_bias, yy_text_bot);
  
}

/*************************************************************************
 * PLOT_FZRA_DATA:
 *
 * Plot the data on the pixmap given the data to pixel transform values 
 *
 */

static void plot_fzra_data(source_info_t* source,
				 int x_start, int x_end,
				 int y_start, int y_end,
				 double pix_x_unit,double pix_x_bias,
				 int yy_text_bot)
  
{

  int last_weather_type = 0;
  int last_weather = 0;
  int cur_weather = 0;
  int last_x_pix = 0;

  int nn = source->num_reports;

  if (nn < 2) {
    //  Add a no data label
    int direct,ascent,descent;
    XCharStruct overall;

    XTextExtents(gd.fontst,"No Data",7,
		 &direct,&ascent,&descent,&overall);

    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     (x_end - 3 - overall.width),
		     ((y_start + y_end) / 2  - 1),
		     "No Data" ,7);
    return;
  }

  // load up precip data

  // line position and width

  int yy = (yy_text_bot + y_end) / 2;
  XSetLineAttributes(gd.dpy, gd.def_gc, gd.p->flight_category_line_width,
		     LineSolid, CapButt, JoinBevel);
  
  // plot data

  for(int i = 0; i < nn; i++) {

    time_t t1, t2;
    if (i == 0) {
      t1 = source->reports[0].time;
      t2 = source->reports[0].time + (source->reports[1].time - source->reports[0].time) / 2;
    } else if (i == nn - 1) {
      t1 = source->reports[nn - 2].time + (source->reports[nn - 1].time - source->reports[nn - 2].time) / 2;
      t2 = source->reports[nn - 1].time;
    } else {
      t1 = source->reports[i - 1].time + ( source->reports[i].time - source->reports[i - 1].time) / 2;
      t2 = source->reports[i].time + (source->reports[i + 1].time - source->reports[i].time) / 2;
    }

    if ((time_t) source->reports[i].time < gd.start_time ||
	(time_t) source->reports[i].time > gd.end_time) {
      continue;
    }

	double data_val = STATION_NAN;
    switch(source->station_info->fz_precip_field) {
    case Params::RATE:
		data_val = source->reports[i].precip_rate;
      break;
    case Params::ACCUMULATION:
		data_val = source->reports[i].liquid_accum;
      break;
    case Params::ACCUMULATION2:
		data_val = source->reports[i].shared.station.liquid_accum2;
      break;
    case Params::CEILING:
		data_val = source->reports[i].ceiling;
      break;
    case Params::VISIBILITY:
		data_val = source->reports[i].visibility;
      break;
    case Params::TEMPERATURE:
		data_val = source->reports[i].temp;
      break;
    case Params::HUMIDITY:
		data_val = source->reports[i].relhum;
      break;
    case Params::DEWPT:
		data_val = source->reports[i].dew_point;
      break;
    case Params::WIND_SPEED:
		data_val = source->reports[i].windspd;
      break;
    case Params::WIND_DIRN:
		data_val = source->reports[i].winddir;
      break;
    case Params::PRESSURE:
		data_val = source->reports[i].pres;
      break;
    case Params::SPARE1:
		data_val = source->reports[i].shared.station.Spare1;
      break;
    case Params::SPARE2:
		data_val = source->reports[i].shared.station.Spare2;
      break;
    case Params::FZ_PRECIP:
		data_val = 1.0;
      break;
    case Params::PRECIP_TYPE:
		if(source->reports[i].weather_type) data_val = 1.0;
      break;
      case Params::FLIGHT_CAT: {}
      break;
    }

	if(data_val == STATION_NAN) continue;

	XSetForeground(gd.dpy, gd.def_gc, gd.no_fzra_cell);
	if(source->reports[i].weather_type & WT_FROST) {
		XSetForeground(gd.dpy, gd.def_gc, gd.frost_cell);
		cur_weather = 1;
	}
	if(source->reports[i].weather_type & WT_FZFG) {
		XSetForeground(gd.dpy, gd.def_gc, gd.fzfg_cell);
		cur_weather = 2;
	}
	if(source->reports[i].weather_type & WT_MFZDZ) {
		XSetForeground(gd.dpy, gd.def_gc, gd.lt_fzdz_cell);
		cur_weather = 3;
	}
	if(source->reports[i].weather_type & WT_FZDZ) {
		XSetForeground(gd.dpy, gd.def_gc, gd.fzdz_cell);
		cur_weather = 4;
	}
	if(source->reports[i].weather_type & WT_PFZDZ) {
		XSetForeground(gd.dpy, gd.def_gc, gd.hv_fzdz_cell);
		cur_weather = 5;
	}

	if(source->reports[i].weather_type & WT_MFZRA) {
		XSetForeground(gd.dpy, gd.def_gc, gd.lt_fzra_cell);
		cur_weather = 6;
	}
	if(source->reports[i].weather_type & WT_FZRA) {
		XSetForeground(gd.dpy, gd.def_gc, gd.fzra_cell);
		cur_weather = 7;
	}
	if(source->reports[i].weather_type & WT_PFZRA) {
		XSetForeground(gd.dpy, gd.def_gc, gd.hv_fzra_cell);
		cur_weather = 8;
	}

	last_weather_type = source->reports[i].weather_type;

    short int x1 = (short int) (t1 * pix_x_unit + pix_x_bias);
    short int x2 = (short int) (t2 * pix_x_unit + pix_x_bias);

	// Values fill the same pixel
	if(x2 ==  last_x_pix ) {

	  // Only plot values that are worse than previously rendered.
	  if(cur_weather > last_weather)  {
            XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, x1, yy, x2, yy);
	        last_weather = cur_weather;
	  }

	} else { // Values plot on new pixels.

        XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, x1, yy, x2, yy);
	    last_weather = cur_weather;
	}
	last_x_pix = x2;

  } // i

  XSetLineAttributes(gd.dpy, gd.def_gc,
		     1, LineSolid, CapButt, JoinBevel);
    
  if(gd.p->right_margin > 20) { // Plot the data value in the margin.
    int direct,ascent,descent;
    int ypos;
    XCharStruct overall;
    char string_buf[128];

    strcpy(string_buf,"Nil");
    // If multiple are detected, use the worst.

    if(last_weather_type & WT_FROST) strcpy(string_buf,"FR");
    if(last_weather_type & WT_MFZDZ)strcpy(string_buf,"-FZDZ");
    if(last_weather_type & WT_FZDZ) strcpy(string_buf,"FZDZ");
    if(last_weather_type & WT_PFZDZ)strcpy(string_buf,"+FZDZ");
        
    if(last_weather_type & WT_MFZRA)strcpy(string_buf,"-FZRA");
    if(last_weather_type & WT_FZRA) strcpy(string_buf,"FZRA");
    if(last_weather_type & WT_PFZRA)strcpy(string_buf,"+FZRA");
    
    XTextExtents(gd.fontst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);

    // Place the label at the mid point
    ypos = (int)(((y_start + y_end) / 2) + (ascent  - descent)  / 2);

    XSetForeground(gd.dpy, gd.def_gc, source->color_cell);

    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     (x_end + 1), ypos,
		     string_buf,strlen(string_buf));
  }
}


/*****************************************************************
 *  PLOT_WX_TYPE
 */

static void plot_wx_type( source_info_t* source, int x_start, int x_end,
			     int y_start, int y_end, int label_flag)

{

  double pix_x_unit,pix_x_bias; /* Slope & Y intercept for
				 * units to pixel space conv */

  int direct,ascent,descent;
  XCharStruct overall;
  char string_buf[128];
  station_report_t *report;

  /* Draw Dividing line */
  XSetForeground(gd.dpy, gd.def_gc, gd.fg_cell);
  XDrawLine(gd.dpy,gd.back_xid,gd.def_gc,x_start,y_end,x_end,y_end);

  // Draw scales and set up a mapping between pixel space
  // and time/data units 

  draw_x_scale(source, x_start, x_end, y_start, y_end, 
	       label_flag, pix_x_unit, pix_x_bias);
  
  if(source->num_reports == 0) return;  /* bail out if no reports */
  
  // Place station name and Units Label in the Upper left corner

  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  report = source->reports;
  int xx = 3;

  sprintf(string_buf,"%s Wx Type", report->station_label);
  XTextExtents(gd.fontst, string_buf, strlen(string_buf),
	       &direct,&ascent,&descent,&overall);
  XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc, xx, y_start + ascent  + 3,
		   string_buf ,strlen(string_buf));
  xx += XTextWidth(gd.fontst, string_buf, strlen(string_buf));

  // place legends

  XSetLineAttributes(gd.dpy, gd.def_gc, ascent, LineSolid, CapButt, JoinBevel);

  int yy_text_mid = y_start + 3 + ascent / 2;
  int yy_text_bot = y_start + 3 + ascent + ascent;
  
  xx += 15;
  char label[128];

  sprintf(label, "Nil");
  XSetForeground(gd.dpy, gd.def_gc, gd.no_fzra_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=35;

  sprintf(label, "FG");
  XSetForeground(gd.dpy, gd.def_gc, gd.fog_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=25;

  sprintf(label, "HZ");
  XSetForeground(gd.dpy, gd.def_gc, gd.hz_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=25;

  sprintf(label, "DZ");
  XSetForeground(gd.dpy, gd.def_gc, gd.dz_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=25;

  sprintf(label, "UP");
  XSetForeground(gd.dpy, gd.def_gc, gd.up_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=25;


  sprintf(label, " -RA+ ");
  XSetForeground(gd.dpy, gd.def_gc, gd.lt_ra_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=15;

  XSetForeground(gd.dpy, gd.def_gc, gd.ra_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  xx +=15;
  XSetForeground(gd.dpy, gd.def_gc, gd.hv_ra_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  xx += 25;
  
  sprintf(label, "TS");
  XSetForeground(gd.dpy, gd.def_gc, gd.ts_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=25;


  sprintf(label, "-SN+  ");
  XSetForeground(gd.dpy, gd.def_gc, gd.lt_sn_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=15;

  XSetForeground(gd.dpy, gd.def_gc, gd.sn_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  xx +=15;

  XSetForeground(gd.dpy, gd.def_gc, gd.hv_sn_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  xx += 25;

  sprintf(label, "PE");
  XSetForeground(gd.dpy, gd.def_gc, gd.pe_cell);
  XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_mid, xx + 15, yy_text_mid);
  XSetForeground(gd.dpy, gd.def_gc, source->color_cell);
  XDrawImageString(gd.dpy, gd.back_xid, gd.def_gc, xx, yy_text_bot, label, strlen(label));
  xx +=25;


  /* plot the data using the mapping between pixel space and time/data units */
  XSetLineAttributes(gd.dpy, gd.def_gc, 1, LineSolid, CapButt, JoinBevel);
  plot_wx_type_data(source, x_start, x_end, y_start, y_end,
	       pix_x_unit, pix_x_bias, yy_text_bot);
  
}

/*************************************************************************
 * PLOT_WX_TYPE_DATA:
 *
 * Plot the data on the pixmap given the data to pixel transform values 
 *
 */

static void plot_wx_type_data(source_info_t* source,
				 int x_start, int x_end,
				 int y_start, int y_end,
				 double pix_x_unit,double pix_x_bias,
				 int yy_text_bot)
  
{

  int last_weather_type = 0;
  int last_weather = 0;
  int cur_weather = 0;
  int last_x_pix = 0;

  int nn = source->num_reports;

  if (nn < 2) {
    //  Add a no data label
    int direct,ascent,descent;
    XCharStruct overall;

    XTextExtents(gd.fontst,"No Data",7,
		 &direct,&ascent,&descent,&overall);

    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     (x_end - 3 - overall.width),
		     ((y_start + y_end) / 2  - 1),
		     "No Data" ,7);
    return;
  }

  // load up precip data

  // line position and width

  int yy = (yy_text_bot + y_end) / 2;
  XSetLineAttributes(gd.dpy, gd.def_gc, gd.p->flight_category_line_width,
		     LineSolid, CapButt, JoinBevel);
  
  // plot data

  for(int i = 0; i < nn; i++) {

    time_t t1, t2;
    if (i == 0) {
      t1 = source->reports[0].time;
      t2 = source->reports[0].time + (source->reports[1].time - source->reports[0].time) / 2;
    } else if (i == nn - 1) {
      t1 = source->reports[nn - 2].time + (source->reports[nn - 1].time - source->reports[nn - 2].time) / 2;
      t2 = source->reports[nn - 1].time;
    } else {
      t1 = source->reports[i - 1].time + ( source->reports[i].time - source->reports[i - 1].time) / 2;
      t2 = source->reports[i].time + (source->reports[i + 1].time - source->reports[i].time) / 2;
    }

    if ((time_t) source->reports[i].time < gd.start_time ||
	(time_t) source->reports[i].time > gd.end_time) {
      continue;
    }


	XSetForeground(gd.dpy, gd.def_gc, gd.bg_cell);
	if(source->reports[i].weather_type & WT_HZ) {
		XSetForeground(gd.dpy, gd.def_gc, gd.hz_cell);
		cur_weather = 1;
	}
	else if(source->reports[i].weather_type & WT_FG) {
		XSetForeground(gd.dpy, gd.def_gc, gd.fog_cell);
		cur_weather = 2;
	}
	else if(source->reports[i].weather_type & WT_DZ) {
		XSetForeground(gd.dpy, gd.def_gc, gd.dz_cell);
		cur_weather = 3;
	}
	else if(source->reports[i].weather_type & WT_UP) {
		XSetForeground(gd.dpy, gd.def_gc, gd.up_cell);
		cur_weather = 4;
	}
	else if(source->reports[i].weather_type & WT_MRA) {
		XSetForeground(gd.dpy, gd.def_gc, gd.lt_ra_cell);
		cur_weather = 5;
	}
	else if(source->reports[i].weather_type & WT_RA) {
		XSetForeground(gd.dpy, gd.def_gc, gd.ra_cell);
		cur_weather = 6;
	}

	else if(source->reports[i].weather_type & WT_PRA) {
		XSetForeground(gd.dpy, gd.def_gc, gd.hv_ra_cell);
		cur_weather = 7;
	}
	else if(source->reports[i].weather_type & WT_TS) {
		XSetForeground(gd.dpy, gd.def_gc, gd.ts_cell);
		cur_weather = 8;
	}
	else if(source->reports[i].weather_type & WT_MSN) {
		XSetForeground(gd.dpy, gd.def_gc, gd.lt_sn_cell);
		cur_weather = 9;
	}
	else if(source->reports[i].weather_type & WT_SN) {
		XSetForeground(gd.dpy, gd.def_gc, gd.sn_cell);
		cur_weather = 10;
	}
	else if(source->reports[i].weather_type & WT_PSN) {
		XSetForeground(gd.dpy, gd.def_gc, gd.hv_sn_cell);
		cur_weather = 11;
	}
	else if(source->reports[i].weather_type & WT_PE) {
		XSetForeground(gd.dpy, gd.def_gc, gd.pe_cell);
		cur_weather = 12;
	}
	// show 'nil' unless visibility is also at zero
	else if(source->reports[i].visibility) {
		XSetForeground(gd.dpy, gd.def_gc, gd.no_fzra_cell);
	        cur_weather = 0;
	}

	last_weather_type = source->reports[i].weather_type;

    short int x1 = (short int) (t1 * pix_x_unit + pix_x_bias);
    short int x2 = (short int) (t2 * pix_x_unit + pix_x_bias);

	// Values fill the same pixel
	if(x2 ==  last_x_pix ) {

	  // Only plot values that are worse than previously rendered.
	  if(cur_weather > last_weather)  {
            XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, x1, yy, x2, yy);
	        last_weather = cur_weather;
	  }

	} else { // Values plot on new pixels.

        XDrawLine(gd.dpy, gd.back_xid, gd.def_gc, x1, yy, x2, yy);
	    last_weather = cur_weather;
	}
	last_x_pix = x2;

  } // i

  XSetLineAttributes(gd.dpy, gd.def_gc,
		     1, LineSolid, CapButt, JoinBevel);
    
  if(gd.p->right_margin > 20) { // Plot the data value in the margin.
    int direct,ascent,descent;
    int ypos;
    XCharStruct overall;
    char string_buf[128];

    strcpy(string_buf,"Nil");
    // If multiple are detected, use the worst.

    if(last_weather_type & WT_FG)strcpy(string_buf,"FG");
    if(last_weather_type & WT_HZ)strcpy(string_buf,"HZ");
    if(last_weather_type & WT_UP) strcpy(string_buf,"UP");
        
    if(last_weather_type & WT_MRA)strcpy(string_buf,"-RA");
    if(last_weather_type & WT_RA) strcpy(string_buf,"RA");
    if(last_weather_type & WT_PRA)strcpy(string_buf,"+RA");
    if(last_weather_type & WT_TS) strcpy(string_buf,"TS");
    
    if(last_weather_type & WT_MSN)strcpy(string_buf,"-SN");
    if(last_weather_type & WT_SN) strcpy(string_buf,"SN");
    if(last_weather_type & WT_PSN)strcpy(string_buf,"+SN");
    if(last_weather_type & WT_PE)strcpy(string_buf,"PE");
        
    XTextExtents(gd.fontst,string_buf,strlen(string_buf),
		 &direct,&ascent,&descent,&overall);

    // Place the label at the mid point
    ypos = (int)(((y_start + y_end) / 2) + (ascent  - descent)  / 2);

    XSetForeground(gd.dpy, gd.def_gc, source->color_cell);

    XDrawImageString(gd.dpy,gd.back_xid,gd.def_gc,
		     (x_end + 1), ypos,
		     string_buf,strlen(string_buf));
  }
}
