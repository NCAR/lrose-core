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
/************************************************************************
 * RENDER_MARGINS
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_MARGINS 1

#include "cidd.h"

/**********************************************************************
 * RENDER_HORIZ_MARGINS: Render the margins for the horizontal display.
 */

void render_horiz_margins(QPaintDevice *pdev, int page,
			  time_t start_time, time_t end_time)
{

#ifdef NOTYET
  
  int    x1, y1, ht, wd;    /* boundries of image area */

         
  /* Add a border around the plot */
  x1 = gd.h_win.margin.left;
  y1 = gd.h_win.margin.top;
  wd = gd.h_win.img_dim.width -1;
  ht = gd.h_win.img_dim.height -1;
     
  if(!(gd.h_win.margin.left == 0 &&
      gd.h_win.margin.top == 0 &&
	  gd.h_win.margin.right == 0 &&
	  gd.h_win.margin.bot == 0)) {

      XDrawRectangle(gd.dpy, xid, gd.legends.foreground_color->gc,
		 x1, y1,
		 wd, ht);
  }

  draw_hwin_right_margin(xid, page); /* the color scale */

  draw_hwin_top_margin(xid);

  draw_hwin_bot_margin(xid, page, start_time, end_time); // Time scale

  draw_hwin_left_margin(xid);

  draw_hwin_interior_labels(xid, page, start_time, end_time);

  if(_params.show_data_messages) gui_label_h_frame(gd.frame_label, -1);

#endif
  
}

