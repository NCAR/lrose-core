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
 * ROUTE_PU_PROC.C - Notify and event callback functions for route
 * selection
 */

#define ROUTE_PU_PROC

#include "cidd.h"

/*************************************************************************
 * Notify callback function for `route_st'.
 */
void
set_route_proc(Panel_item, int value, Event *)
{
    // Copy the proper route definition into the main struct
    if(gd.layers.route_wind.num_predef_routes > 0 )
	memcpy(&gd.h_win.route,gd.layers.route_wind.route+value,sizeof(route_track_t));

    // Turns on cross section window, sets proper redraw, and data flags.
    setup_route_area(1);

    // Hide the menu
    xv_set(gd.route_pu->route_pu,FRAME_CMD_PUSHPIN_IN, FALSE, XV_SHOW, FALSE, NULL);
    // Pop the button back up
    xv_set(gd.v_win_v_win_pu->route_st, PANEL_VALUE,0,NULL);
}     
