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
 * GRAPHIC_CROSS_SECTION.CC: Event handling for the Horiz. view window 
 *
 * For the Cartesian Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 *
 * Modification History:
 *   26 Aug 92   N. Rehak   Added code for zooming radial format data.
 */

#include <math.h>
// #include <X11/Xlib.h>

#define GRAPHIC_CROSS_SECTION    1

#include "cidd.h"

extern int    r_state;              /* Route define state variable */
extern int    r_lastx,r_lasty;      /* ROUTE end point */
extern int    r_startx,r_starty;    /* ROUTE start point */

/*************************************************************************
 * SETUP_ROUTE_AREA :   Indicate a New Route or RHI has been defined. 
 *   If clear_flag is set, then the view is reset 
 *   Pop up the panel and make indicate data needs loaded and images need redrawn.
 */

void setup_route_area(int clear_flag )
{

  if(clear_flag) {
    gd.v_win.cmin_x = 0.0;
    gd.v_win.cmax_x = gd.h_win.route.total_length;
    gd.v_win.cmin_y = gd.v_win.min_ht;
    gd.v_win.cmax_y = gd.v_win.max_ht;
  }
  
  // int value = 0;
  // Turn on Cross section bit
  // value = xv_get(gd.h_win_horiz_bw->main_st,PANEL_VALUE);
  //     value |= gd.menu_bar.show_xsect_panel_bit;
  // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,value,NULL);
  // gd.menu_bar.last_callback_value = value;
  
  // show_xsect_panel((u_int)1);
  
  gd.v_win.active = 1;
  set_redraw_flags(1,1);
  reset_data_valid_flags(0,1);
  reset_terrain_valid_flags(0,1);
}
 
/*************************************************************************
 * REDRAW_ROUTE_LINE:  Draws the ROUTE define line
 *
 */

void redraw_route_line(win_param_t * win)
{

  // int i;
  
  // DEBUG
  /*
    static int last_startx = -1;
    if(last_startx != r_startx) 
    fprintf(stderr,"Num Segs: %d \t State: %d\t  start XY: %4d,%4d\t Last XY: %4d,%4d\n",
    win->route.num_segments, r_state,
    r_startx,r_starty,r_lastx,r_lasty);
  */
  
#ifdef NOTYET
  for(int i=0; i < win->route.num_segments; i++) {
    XDrawLine(gd.dpy,gd.hcan_xid,gd.ol_gc,win->route.x_pos[i],win->route.y_pos[i],
              win->route.x_pos[i+1],win->route.y_pos[i+1]);
#endif
    
    // DEBUG
    /*
      if(last_startx != r_startx) {
      fprintf(stderr,"\tX,Y POS: %4d,%4d  X,Y World: %8.4f,%8.4f\n",
      win->route.x_pos[i],win->route.y_pos[i],
      win->route.x_world[i],win->route.y_world[i]);
      
      }
    */
    
    // DEBUG
     /*
       if(last_startx != r_startx) {
       fprintf(stderr,"\tX,Y POS: %4d,%4d  X,Y World: %8.4f,%8.4f\n",
       win->route.x_pos[i],win->route.y_pos[i],
	    win->route.x_world[i],win->route.y_world[i]);
            last_startx = r_startx;
            }
     */
    
#ifdef NOTYET
    XDrawLine(gd.dpy,gd.hcan_xid,gd.ol_gc,r_startx,r_starty,r_lastx,r_lasty);
#endif
    
}   
