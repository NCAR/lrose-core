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
 * RENDER_XSECT_CONTROL.C: 
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_XSECT_CONTROL
#include "cidd.h"

/**********************************************************************
 * RENDER_VERT_DISPLAY: Render the vertical cross section display
 */

int render_vert_display( QPaintDevice *pdev, int page, time_t start_time, time_t end_time)
{
    int i;
    int x1,y1,ht,wd;    /* boundries of image area */
    // int stat;
    contour_info_t cont; // contour params

    if(xid == 0) return CIDD_FAILURE;

    if(_params.show_data_messages) {
       gui_label_h_frame("Rendering",-1);
    } else {
       set_busy_state(1);
    }

    if(gd.debug2) fprintf(stderr,"Rendering Vertical Image, page :%d\n",page);
    /* Clear drawing area */
    XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
        0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height);

    if(!_params.draw_main_on_top) { 
        if(gd.mrec[page]->render_method == LINE_CONTOURS) {
            cont.min = gd.mrec[page]->cont_low;
            cont.max = gd.mrec[page]->cont_high;
            cont.interval = gd.mrec[page]->cont_interv;
            cont.active = 1;
	    cont.field = page;
            cont.labels_on = _params.label_contours;
            cont.color = gd.legends.foreground_color;
	    cont.vcm = &gd.mrec[page]->v_vcm;
	    if (0) {   // Taiwan HACK - Buggy - do not use RenderLineContours()
	    //if (gd.layers.use_alt_contours) {
	      RenderLineContours(xid, &cont, true);
 	    } else {
 	      render_xsect_line_contours(xid,&cont);
 	    }
        } else {
          render_xsect_grid(xid,gd.mrec[page],start_time,end_time,0);
          // stat =  render_xsect_grid(xid,gd.mrec[page],start_time,end_time,0);
	}
    }
    
    /* Render each of the gridded_overlay fields */
    for(i=0; i < NUM_GRID_LAYERS; i++) {           
        if(gd.layers.overlay_field_on[i]) {
            render_xsect_grid(xid,gd.mrec[gd.layers.overlay_field[i]],start_time,end_time,1);
        }
    } 

    if(_params.draw_main_on_top) { 
        if(gd.mrec[page]->render_method == LINE_CONTOURS) {
            cont.min = gd.mrec[page]->cont_low;
            cont.max = gd.mrec[page]->cont_high;
            cont.interval = gd.mrec[page]->cont_interv;
            cont.active = 1;
	    cont.field = page;
            cont.labels_on = _params.label_contours;
            cont.color = gd.legends.foreground_color;
	    cont.vcm = &gd.mrec[page]->v_vcm;
 	    if (0) {   // Taiwan HACK - Buggy - do not use RenderLineContours()
 	    // if (gd.layers.use_alt_contours) {
	      RenderLineContours(xid, &cont, true);
 	    } else {
 	      render_xsect_line_contours(xid,&cont);
 	    }
        } else {
          // stat =  render_xsect_grid(xid,gd.mrec[page],start_time,end_time,0);
          render_xsect_grid(xid,gd.mrec[page],start_time,end_time,0);
        }
    }

    /* render contours if selected */
    for(i=0; i < NUM_CONT_LAYERS; i++) {
      if(gd.layers.cont[i].active) {    
 	if (0) {   // Taiwan HACK - Buggy - do not use RenderLineContours()
 	// if (gd.layers.use_alt_contours) {
	  RenderLineContours(xid, &(gd.layers.cont[i]), true);
 	} else {
 	  render_xsect_line_contours(xid, &(gd.layers.cont[i]));
 	}
      }
    }

    /* render Winds if selected */
    if(gd.layers.wind_vectors) {
        render_vert_wind_vectors(xid);
    }

    // Render masking terrain
    if(gd.layers.earth.terrain_active) {
       render_v_terrain(xid);
    }

    render_xsect_top_layers(xid,page);

    render_vert_products(xid);

    /* clear margin areas */
    XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
        0,0,gd.v_win.can_dim.width,gd.v_win.margin.top);

    XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
        0,gd.v_win.can_dim.height - gd.v_win.margin.bot,
        gd.v_win.can_dim.width,gd.v_win.margin.bot);

    XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
        0,0,gd.v_win.margin.left,gd.v_win.can_dim.height);

    XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
        gd.v_win.can_dim.width - gd.v_win.margin.right,
        0,gd.v_win.margin.right,gd.v_win.can_dim.height);


    draw_vwin_right_margin(xid,page);
    draw_vwin_top_margin(xid,page);
    draw_vwin_left_margin(xid,page);
    draw_vwin_bot_margin(xid,page);

    /* Add a border */
    x1 = gd.v_win.margin.left -1;
    y1 = gd.v_win.margin.top -1;
    wd = gd.v_win.img_dim.width +1;
    ht = gd.v_win.img_dim.height +1;
    /* Add a border around the plot */
    XDrawRectangle(gd.dpy,xid,gd.legends.foreground_color->gc,x1,y1,wd,ht);
 

    if(_params.show_data_messages) {
	gui_label_h_frame(gd.frame_label,-1);
    } else {
	set_busy_state(0); 
    }

    return CIDD_SUCCESS;
}
