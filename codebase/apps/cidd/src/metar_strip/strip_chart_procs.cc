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
using namespace std;

extern void  draw_plot(void);
/*************************************************************************
 * Menu handler for `menu1'.
 */
Menu
select_field_proc( Menu	menu, Menu_generate	op)
{
	
  int i;
  char *string;
  Menu_item item = (Menu_item ) xv_get(menu, MENU_SELECTED_ITEM);
  
  switch (op) {
  case MENU_DISPLAY:
    break;
    
  case MENU_DISPLAY_DONE:
    break;
    
  case MENU_NOTIFY:
    
    gd.variable = -1;
		  
    string = (char *)  xv_get(item,MENU_STRING);

    if (gd.p->debug >= Params::DEBUG_VERBOSE) {
      cerr << "selection string: " << string << endl;
    }

    /* Set a numerical value for which variable we are dealing with */
    if(strncmp(string,"Ceil",4) == 0) {
      gd.variable = Params::CEILING;
    }
    if(strncmp(string,"Vis",3) == 0) {
      gd.variable = Params::VISIBILITY;
    }
    if(strncmp(string,"Flight",6) == 0) {
      gd.variable = Params::FLIGHT_CAT;
    }
    if(strncmp(string,"Temperature",11) == 0) {
      gd.variable = Params::TEMPERATURE;
    }
    if(strncmp(string,"Humidity",8) == 0) {
      gd.variable = Params::HUMIDITY;
    }
    if(strncmp(string,"Wind Speed",10) == 0) {
      gd.variable = Params::WIND_SPEED;
    }
    if((strncmp(string,"Wind Direction",15) == 0) || 
       (strncmp(string,"Wind Dirn",10) == 0)) {
      gd.variable = Params::WIND_DIRN;
    }
    if(strncmp(string,"Pressure",8) == 0) {
      gd.variable = Params::PRESSURE;
    }
    
    if(gd.variable == -1) {
      for(i = 0; i < gd.num_sources; i++) {
	if(strcmp(string, gd.sources[i].station_info->name) == 0) {
	  gd.variable = i + Params::PRESSURE + 1;
	}
      }
    }

    if (gd.p->debug >= Params::DEBUG_VERBOSE) {
      cerr << "gd.variable: " << gd.variable << endl;
    }
    
    gd.data_min = 99999999.0;
    gd.data_max = -99999999.0;
    
    draw_plot();
    
    /* gxv_start_connections DO NOT EDIT THIS SECTION */
    
    /* gxv_end_connections */
    
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
    if(event_is_down(event)) {
      Menu    menu = (Menu) xv_get(win, WIN_MENU);
      if(menu) menu_show(menu, win, event, 0);
    }
    break;
    
  case WIN_REPAINT:
    draw_plot();
    break;
    
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

  xv_get(win, XV_KEY_DATA, INSTANCE);
  
  switch(event_action(event)) {

  case WIN_RESIZE:

    // Release backing Pixmap

    if(gd.back_xid) XFreePixmap(gd.dpy,gd.back_xid);
    
    // recompute sizes

    gd.win_height =  xv_get(gd.Strip_chart_win1->win1,WIN_HEIGHT);
    gd.win_width =  xv_get(gd.Strip_chart_win1->win1,WIN_WIDTH);
    gd.plot_height = gd.win_height - gd.p->bottom_margin;
    gd.plot_width = gd.win_width -  gd.p->right_margin;

    // force a recalc on the times, and a retrieve

    check_retrieve(true);
    
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

    // create new backing Pixmap

    gd.back_xid =  XCreatePixmap(gd.dpy, gd.canvas_xid,
				 gd.win_width, gd.win_height,
				 DefaultDepth(gd.dpy,0));

    do_draw();

    break;

  default: {}

  }
  
  
  /* gxv_start_connections DO NOT EDIT THIS SECTION */
  
  /* gxv_end_connections */
  
  return notify_next_event_func(win, (Notify_event) event, arg, type);

}

