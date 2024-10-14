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
 * GRAPHIC_PANEL.CC
 *
 * For the Cartesian Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */


#define GRAPHIC_PANEL    1
#include "cidd.h"

/* These are all shared within the "Graphic..." modules */
int    b_lastx,b_lasty;    /* Boundry end point */
int    b_startx,b_starty;    /* Boundry start point */
int    p_lastx,p_lasty;    /* Pan end point */
int    p_startx,p_starty;    /* Pan start point */
int    r_lastx,r_lasty;    /* ROUTE end point */
int    r_startx,r_starty;    /* ROUTE start point */
int    r_state;              // ROUTE multi segment define state
// State 0: Starting Point - Nothing clicked - nothing in progress.
// State 1: Button has been pressed, but not moved
// State 2. Button has been dragged in the down position to a new location
// State 3: Button has been released after being moved
// State 4: Button has been pressed again near the latest release point 
//   Note: State 4 can only be reached from state 3.
//         State 3 can only be reached from state 2.
//         State 2 is reached from either state 1, 3 or 4
//         State 1 can only be reached from state 0
//         State 0 is reached from either state 1, 2 or 4


// State 0 only goes to state 1. - DOWN

// State 1 moves to state 2 if cursor moves with the button down - DRAG - ROUTE EXTEND
// State 1 reverts to state 0 if cursor hasn't moved on button up - UP - REPORT
//   Transitioning from 1 to 2 - Starts and extends a new segment
//   Transitioning from 1 to 0 - Reports the data value

// State 2 reverts to state 0 if cursor hasn't moved much on button up - UP - REPORT
// State 2 moves to state 3 if cursor has moved significantly on button up - UP - ADD SEGMENT
//   Transitioning from 2 to 3 Adds a route segment
//   Transitioning from 2 to 0 - Reports the data value

// State 3 moves to state 4 if the button goes down again close by - DOWN
// State 3 reverts to state 2 if button goes down agian a ways away. - DOWN - ROUTE EXTEND
//   Transitioning from 3 to 2 Starts a new  route segment
//   Transitioning from 3 to 4 Indicates route will be either extended by dragging or
//     will be teminated and completed on button up.

// State 4 moves to 0 if the button goes up again close by - UP -  COMPLETE ROUTE- DONE.
// State 4 moves to state 2 if cursor moves with the button down - DRAG - ROUTE EXTEND
//   Transitioning from 4 to 0 Completes the route.
//   Transitioning from 4 to 2 Starts and extends a new segment

// The process of defining a route path mean that in one way or another
// one must reach 4 from state 3, which can only be reached when the
// user clicks the first time and then drags the cursor away. In this
// way, simple mistaken key clicks do not erase a route.
// Once the route definition process is clearly underway, these state
// transistion rules allow the user to extend the route by either
// clicking near the last end point and dragging 
// to the next way point. The process terminates when the user
// single clicks and releases on the end way point after dragging out
// at least one of the segments. To put it another way start the
// route definition by dragging out a segment. Then extend it by
// clicking and dragging to the next
// way point. Terminate the route by explictly clicking again on
// the last way point. 

#ifdef NOTNOW
/*************************************************************************
 * Event callback function for `cp'.
 */
Notify_value h_pan_event_proc( Window   win, Event *event,
                               Notify_arg  arg, Notify_event_type type)
{
    int   num_fields;
    int   field_index;
    int   height_index;
    met_record_t    *mr;
    
    /*
     * Process keyboard events.  Only process them when the key is
     * released to prevent repeated processing when the key is held
     * down (may want to change this in the future).
     */

    if (event_is_up(event))
    {
        
        /*
         * Pressing a number key selects the corresponding field in the field
         * choice list.
         */

        num_fields = gd.num_menu_fields;
        mr = gd.mrec[gd.h_win.page];
    
        if (event_id(event) >= '1' && event_id(event) <= '1' + num_fields - 1)
        {
            field_index = event_id(event) - '1';
        
            /* simulate the user selecting a new field */
            set_field(field_index);
            set_v_field(gd.field_index[field_index]);
        }

        /*
         * Arrow keys:
         *    up    - move up one elevation
         *    down  - move down on elevation
         *    left  - move back one movie frame
         *    right - move forward one movie frame
         */

        switch(event_action(event))
        {
        case ACTION_GO_COLUMN_BACKWARD :      /* ACTION_UP */
	    
            if(mr->plane < mr->ds_fhdr.nz -1 && mr->plane >= 0) {
                height_index = mr->plane +1;
                set_height(height_index);
	        set_height_label();
                set_redraw_flags(1,1);
                reset_data_valid_flags(1,1);
            }
        
            break;
        
        case ACTION_GO_COLUMN_FORWARD :         /* ACTION_DOWN */

            if(mr->plane > 0  && mr->plane < mr->ds_fhdr.nz) {
                height_index = mr->plane - 1;
                set_height(height_index);
	        set_height_label();
                set_redraw_flags(1,1);
                reset_data_valid_flags(1,1);
            }

            break;
        
        case ACTION_GO_CHAR_BACKWARD :       /* ACTION_LEFT */
            break;
        
        case ACTION_GO_CHAR_FORWARD :        /* ACTION_RIGHT */
            break;
        
        default:
            break;
        } /* endswitch - event_action(event) */

    } /* endif - event_is_up */


    return notify_next_event_func(win, (Notify_event) event, arg, type);
}
#endif
