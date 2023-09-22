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
/****************************************************************
 * RENDER_PRODUCTS.c
 *
 *****************************************************************/

#include "cidd.h"

/**********************************************************************
 * RENDER__PRODUCTS: Render selected products onto the given pixmap
 */

void render_products( Drawable xid, time_t start_time, time_t end_time)
{
    int font_index;
    int i;
    time_t tm;    /* current time */
    double x_dproj_per_pixel;
    double y_dproj_per_pixel;
    double top_km,bot_km;
    double right_km,left_km;

    tm = time(0);

    /* allow overlays to be active through at least one time interval */
    if((start_time < tm) && (tm - start_time) < (gd.movie.time_interval * 60.0)) {
        start_time = (time_t)(tm - (gd.movie.time_interval * 60.0));
    }

    // Render Native SYMPRODS Objects
    if(gd.prod.products_on) {
	x_dproj_per_pixel = (gd.h_win.cmax_x - gd.h_win.cmin_x) /
	    (double)gd.h_win.img_dim.width;

	y_dproj_per_pixel = (gd.h_win.cmax_y - gd.h_win.cmin_y) /
	    (double)gd.h_win.img_dim.height;  

        left_km = (double)gd.h_win.margin.left * x_dproj_per_pixel;
	right_km = (double)gd.h_win.margin.right * x_dproj_per_pixel;
	top_km = (double)gd.h_win.margin.top * y_dproj_per_pixel;
	bot_km = (double)gd.h_win.margin.bot * y_dproj_per_pixel;  

        // Pick a font size based on the detail thresholds and domain
        font_index = gd.prod.prod_font_num;
        for(i=NUM_PRODUCT_DETAIL_THRESHOLDS -1; i >=0; i--) {
           if( gd.h_win.km_across_screen <= gd.prod.detail[i].threshold) {
             font_index = gd.prod.prod_font_num + gd.prod.detail[i].adjustment;
           }
        }
        if(font_index <0) font_index = 0;
        if(font_index >= gd.num_fonts) font_index = gd.num_fonts -1;


	gd.r_context->reset_fgbg(); // Reset the color "memory"

	// Set icon scaling
	gd.r_context->set_iconscale(gd.h_win.km_across_screen);

	// Set which canvas to draw on
	gd.r_context->set_drawable(xid);
	gd.r_context->set_xid(xid);

	// Set the Zoom/ Domain params.
	gd.r_context->set_domain(gd.h_win.cmin_x - left_km,
				gd.h_win.cmax_x + right_km,
				gd.h_win.cmin_y - bot_km,
				gd.h_win.cmax_y + top_km,
				gd.h_win.can_dim.width,
				gd.h_win.can_dim.height,
                                gd.fontst[font_index]);

	gd.r_context->set_times((time_t) gd.epoch_start,
	                       (time_t) gd.epoch_end,
			       (time_t) start_time,
			       (time_t) end_time,
			       gd.mrec[gd.h_win.page]->h_date.unix_time);



 	// if(gd.status_pu) xv_set(gd.status_pu->status_list,XV_SHOW,FALSE,NULL);

	// Do the Work!
	gd.prod_mgr->draw();

 	// if(gd.status_pu) xv_set(gd.status_pu->status_list,XV_SHOW,TRUE,NULL);
    }

}   
