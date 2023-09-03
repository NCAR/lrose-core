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
 * GRAPHIC_CANVAS_EVENTS.CC:: Event handling for the Horiz view margins 
 *
 * For the Cartesian Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */


#define GRAPHIC_MARGIN_EVENTS  1

#include "cidd.h"

//////////////////////////////////////////////////////////////////////////
// SET_HEIGHT : Set all parameters correctly for the given height index
//

void set_height(int index)
{
   met_record_t    *mr;

   static double last_ht =  0.0;

    mr = choose_ht_sel_mr(gd.h_win.page);
  
    if(index >= mr->ds_fhdr.nz) index = mr->ds_fhdr.nz -1;
    if(index < 0) index = 0;

    gd.h_win.cur_ht = mr->vert[index].cent;

    if(gd.h_win.cur_ht !=  last_ht){

        last_ht = gd.h_win.cur_ht;
        reset_data_valid_flags(1,0);
     
        switch(gd.movie.mode) {
        case REALTIME_MODE:
            set_redraw_flags(1,1);
            break;
     
        case ARCHIVE_MODE:
            set_redraw_flags(1,1);
            break;
        }

	gd.prod_mgr->reset_product_valid_flags_vert();
	
    }
     
}

//////////////////////////////////////////////////////////////////////////
// Event handler function for top margin 
//
void top_margin_event(Event *event)
{
    
    int   xpix,ypix;  

    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);  /* record when this event took place */

    if(event_action(event) == ACTION_SELECT ) {
        if(event_is_down(event)) {
            xpix = event_x(event);
            ypix = event_y(event);
         } else {
            xpix = event_x(event);
            ypix = event_y(event);
	 }
    } 

    return;
}

//////////////////////////////////////////////////////////////////////////
// Event handler function for bottom margin 
//
void bot_margin_event(Event *event)
{
    
    int  frame;
    int   xpix,ypix;  

    time_t click_time;
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);  /* record when this event took place */
    gd.last_event_time = tp.tv_sec;

    if(event_action(event) == ACTION_SELECT ) {
        if(event_is_down(event)) {
            xpix = event_x(event);
            ypix = event_y(event);
         } else {
            xpix = event_x(event);
            ypix = event_y(event);
	    click_time = time_from_pixel(xpix);
	    // fprintf(stderr,"Time = %s\n",ctime(&click_time));

	    frame = (int) ((click_time - gd.movie.start_time) /
			  (gd.movie.time_interval * 60.0) + 1.5);
            
	    if(frame < 0) frame = 0;
	    if(frame > gd.movie.num_frames) frame = gd.movie.num_frames;

	    // xv_set(gd.movie_pu->movie_frame_sl, PANEL_VALUE, frame, NULL);
	    movie_frame_proc(gd.movie_pu->movie_frame_sl,frame, (Event*)NULL);
	 }
    } 

    return;
}

//////////////////////////////////////////////////////////////////////////
// Event handler function for left margin 
//
void left_margin_event(Event *event)
{
    
    int   xpix,ypix;  

    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);  /* record when this event took place */

    if(event_action(event) == ACTION_SELECT ) {
        if(event_is_down(event)) {
            xpix = event_x(event);
            ypix = event_y(event);
	    // fprintf(stderr,"Margin click at: %4d,%4d\n",xpix,ypix);
         } else {
            xpix = event_x(event);
            ypix = event_y(event);
	 }
    } 

    return;
}

//////////////////////////////////////////////////////////////////////////
// Event handler function for right margin 
//
void right_margin_event(Event *event)
{
    
    int i;
    int index; 
    int xpix,ypix;  
    double height;
    double dist1,dist2;
    met_record_t    *mr;

    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);  /* record when this event took place */

    if(event_action(event) == ACTION_SELECT ) {
        if(event_is_down(event)) {
            xpix = event_x(event);
            ypix = event_y(event);
         } else {
            xpix = event_x(event);
            ypix = event_y(event);

	    mr = choose_ht_sel_mr(gd.h_win.page);

	    // Search for the closest plane 
	    dist1 = DBL_MAX;
	    index = 0;

	    height = height_from_pixel(ypix,mr);

	    // printf("YPIX: %d,  height_from_pixel: %g\n",ypix,height);
	    for(i=0; i < mr->ds_fhdr.nz; i++ ) {
	      dist2 = fabs(height - mr->vert[i].cent);
	      if(dist2 < dist1) {
		   index = i;
		   dist1 = dist2;
	      }
	    }

	    // Move all of the height related stuff
	    set_height(index);
	    if(gd.debug1) fprintf(stderr,"Height = %.1f - Index: %d\n",height,index);

	 }
    } 

    return;
}

