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
/**********************************************************************
 * GRAPHIC_CHECK.C:  
 *
 * For the Configurable Interactive Data Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */


#define GRAPHIC_CHECK
#include "cidd.h"

// in timer_control.cc
extern int h_copy_flag;
extern int v_copy_flag; 
 
/************************************************************************
 * CHECK_FOR_INVALID_IMAGES: Check for images in which the data 
 * are no longer valid. Look for the "best" invalid  image to 
 *  render.
 *
 */

void check_for_invalid_images(int index)
{
    int    i;
    int    h_image,v_image;
    int    stat;
    int    none_found = 1;
    Drawable    xid;

    h_image = gd.h_win.page + 1;
    v_image = gd.v_win.page + 1;
    if(!gd.run_once_and_exit)  PMU_auto_register("Checking Images (OK)");

    /* look through the rest of the images  */
    for (i=0; i < gd.num_datafields-1; i++) {    
        
        /*
         * Render horizontal image, if necessary.
         */

        if (h_image >= gd.num_datafields) h_image = 0;

        if (gd.mrec[h_image]->currently_displayed && gd.mrec[h_image]->auto_render) {
            
            if (gd.h_win.redraw[h_image] || (gd.mrec[h_image]->h_data_valid == 0)) {
		none_found = 0;
                stat = gather_hwin_data(h_image,
                                        gd.movie.frame[index].time_start,
                                        gd.movie.frame[index].time_end);
                if (stat == CIDD_SUCCESS) {
                    if(gd.mrec[h_image]->auto_render) {
                        xid = gd.h_win.page_xid[h_image];
                    } else {
                        xid = gd.h_win.tmp_xid;
                    }
                    render_horiz_display(xid,h_image,
			gd.movie.frame[index].time_start,
                        gd.movie.frame[index].time_end);

		    save_h_movie_frame(index,xid,h_image);

                    gd.h_win.redraw[h_image] = 0;
                } else {
                    return;
                }
                if (h_image == gd.h_win.last_page && gd.h_win.redraw[h_image] == 0) h_copy_flag = 1;
            } 
        }
        h_image++;

        /*
         * Render vertical image, if necessary.
         */

        if (v_image >= gd.num_datafields) v_image = 0;

        if (gd.mrec[v_image]->currently_displayed && gd.mrec[v_image]->auto_render) {
            if ((gd.v_win.active) && (gd.v_win.redraw[v_image] || (gd.mrec[v_image]->v_data_valid == 0))) {
                stat = gather_vwin_data(v_image, gd.movie.frame[index].time_start,
                                        gd.movie.frame[index].time_end);
                if (stat == CIDD_SUCCESS) {
                    if(gd.mrec[v_image]->auto_render) {
                        xid = gd.v_win.page_xid[v_image];
                    } else {
                        xid = gd.v_win.tmp_xid;
                    }
                    render_vert_display(xid,v_image, gd.movie.frame[index].time_start,
                                        gd.movie.frame[index].time_end);    
                    gd.v_win.redraw[v_image] = 0;
                } else {
                    return;
                }
                if (v_image == gd.v_win.last_page && gd.v_win.redraw[v_image] == 0) v_copy_flag = 1;
            }
        }
        
        v_image++;
    }

    // At this point all background images have been rendered. and nothing else is
    //  happening

    // In html mode, cycle through all zooms and heights
    if(none_found && gd.html_mode && gd.io_info.outstanding_request == 0) {

	/* If more zoom levels to render */
	if(gd.h_win.zoom_level < (gd.h_win.num_zoom_levels -  NUM_CUSTOM_ZOOMS - 2)) {

	     /* Set zoom to next level */
	     gd.h_win.zoom_level++;
	     // set_domain_proc(gd.zoom_pu->domain_st,gd.h_win.zoom_level,NULL);

               // If more heights to render
	} else if (gd.cur_render_height < gd.num_render_heights -1) {

           // Set height to next level
	   gd.cur_render_height++;
	   if(gd.debug) fprintf(stderr,"HTML_MODE: Height now: %g\n",gd.h_win.cur_ht);
	   gd.h_win.cur_ht = gd.height_array[gd.cur_render_height];

           // Reset Zoom back to first  level
	   gd.h_win.zoom_level = 0;
	   // set_domain_proc(gd.zoom_pu->domain_st,gd.h_win.zoom_level,NULL);

	   // Make sure new data gets loaded
	   reset_data_valid_flags(1,0);
	   reset_terrain_valid_flags(1,0);
               
	 // No more heights and no more zooms to render
	 } else if(gd.run_once_and_exit) {
	     if(!gd.quiet_mode)  fprintf(stderr,"Exiting\n");
	     // xv_destroy(gd.h_win_horiz_bw->horiz_bw);
	     exit(-1);
	}
    }
    return;
}
