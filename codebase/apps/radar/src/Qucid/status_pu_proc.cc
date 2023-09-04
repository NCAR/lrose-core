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
 * STATUS_PU_PROC.C - Notify and event callback functions for status popup 
 */
#define STATUS_PU_PROC

#include "cidd.h"
#include <toolsa/TaStr.hh>

#define ST_MAX_LINES 1000
static int num_lines = -1;
// static Xv_opaque scrollbar = 0;
static int view_length = 0;
/*************************************************************************
 *
 */

void add_message_to_status_win(const char *mess, int display_flag)
{

    if( num_lines < 0) {
	// scrollbar = xv_get(gd.status_pu->status_list,PANEL_LIST_SCROLLBAR);
	// view_length = xv_get(scrollbar,SCROLLBAR_VIEW_LENGTH);
	num_lines = 0;
    }

    if(gd.enable_status_window ) {
      if(gd.menu_bar.show_status_win_bit == 0 && display_flag) {
        // xv_set(gd.status_pu->status_pu,FRAME_CMD_PUSHPIN_IN, TRUE,XV_SHOW, TRUE,NULL);
      }
    } else {
      return;
    }

    if(num_lines == ST_MAX_LINES) {
	// Delete first line
      // xv_set(gd.status_pu->status_list,PANEL_LIST_DELETE,0,NULL);
	num_lines--;
    }

    // xv_set( gd.status_pu->status_list, PANEL_LIST_STRING, num_lines, mess, NULL);
    num_lines++;

    if (num_lines > view_length ) {
      // xv_set(scrollbar, SCROLLBAR_VIEW_START, num_lines - view_length + 1, NULL );
    }
}

//////////////////////////////////////
// add a report to the status window
//
// This does not pop up the window - the user must do that manually
// from the main menu

void add_report_to_status_win(const char *mess)
{

  if( num_lines < 0) {
    // scrollbar = xv_get(gd.status_pu->status_list,PANEL_LIST_SCROLLBAR);
    // view_length = xv_get(scrollbar,SCROLLBAR_VIEW_LENGTH);
    num_lines = 0;
  }
  
  if(num_lines == ST_MAX_LINES) {
    // Delete first line
    // xv_set(gd.status_pu->status_list,PANEL_LIST_DELETE,0,NULL);
    num_lines--;
  }
  
  // tokenize the message using line feeds as the break character

  vector<string> lines;
  TaStr::tokenize(mess, "\n", lines);
  
  // copy lines to the status window
  for (int ii = 0; ii < (int) lines.size(); ii++) {
    // xv_set( gd.status_pu->status_list, PANEL_LIST_STRING, num_lines, lines[ii].c_str(), NULL);
    num_lines++;
  }
  
  if (num_lines > view_length ) {
    // xv_set(scrollbar, SCROLLBAR_VIEW_START, num_lines - view_length + 1, NULL );
  }

}

#ifdef NOTNOW

/*************************************************************************
 * Notify callback function for `dismiss_bt'.
 */
void
status_dismiss_proc(Panel_item item, Event *event)
{

    xv_set(gd.status_pu->status_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);

     // Pop back up the Status  button on the menu bar
     gd.menu_bar.last_callback_value &= ~gd.menu_bar.show_status_win_bit;
     xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);  
}
 
/*************************************************************************
 * User-defined action for `status_pu'.
 */
void
status_resize_proc(Xv_window win, Event *event, Notify_arg arg, Notify_event_type type)
{
    int width;
    status_pu_status_pu_objects *ip = (status_pu_status_pu_objects *) xv_get(win, XV_KEY_DATA, INSTANCE);

    width =  xv_get(ip->status_pu,XV_WIDTH);

    xv_set(ip->status_list,PANEL_LIST_WIDTH, width - 40,NULL);
    xv_set(ip->dismiss_bt,XV_X, (width - 10 -  xv_get(ip->dismiss_bt,XV_WIDTH)),NULL);
}
         

/*************************************************************************
 * Event callback function for `status_pu'.
 */
Notify_value
status_pu_status_pu_event_callback(Xv_window win, Event *event, Notify_arg arg, Notify_event_type type)
{
    if (event_action(event) == WIN_RESIZE)
    {
        status_resize_proc(win, event, arg, type);
    }
 
    return notify_next_event_func(win, (Notify_event) event, arg, type);
}
 
/*************************************************************************
 * Notify callback function for `clear_bt'.
 */
void
status_clear_proc(Panel_item item, Event *event)
{
    xv_set(gd.status_pu->status_list,PANEL_LIST_DELETE_ROWS,0,num_lines,NULL);
    num_lines = 0;
} 

#endif
