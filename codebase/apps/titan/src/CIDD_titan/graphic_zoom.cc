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
 * GRAPHIC_ZOOM.C: 
 *
 * For the Cartesian Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define GRAPHIC_ZOOM    1

#include "cidd.h"

extern int    b_lastx,b_lasty;    /* Boundry end point */
extern int    b_startx,b_starty;    /* Boundry start point */
extern int    p_lastx,p_lasty;    /* Pan end point */
extern int    p_startx,p_starty;    /* Pan start point */
extern int    r_lastx,r_lasty;    /* ROUTE end point */
extern int    r_startx,r_starty;    /* ROUTE start point */

/*************************************************************************
 * DO_ZOOM : Zoom area
 */

void do_zoom( void)
{
    int index;
    double    km_x1,km_x2,km_y1,km_y2;
    double    dx,dy;
    static double  min_zoom_threshold = 0.0;
     
	if(min_zoom_threshold == 0.0 ) {
		 min_zoom_threshold = gd.uparams->getDouble( "cidd.min_zoom_threshold", 5.0);
		 if(gd.display_projection == Mdvx::PROJ_LATLON) {
			 min_zoom_threshold /= KM_PER_DEG_AT_EQ;
		 }
	}
    pixel_to_disp_proj(&gd.h_win.margin,b_startx,b_starty,&km_x1,&km_y1);
    pixel_to_disp_proj(&gd.h_win.margin,b_lastx,b_lasty,&km_x2,&km_y2);

    index = gd.h_win.num_zoom_levels -1;

/* Only do additional zooming if request is at least min_zoom_threshold on one side */
if((fabs(km_x2 - km_x1) > min_zoom_threshold) ||
   (fabs(km_y2 - km_y1) > min_zoom_threshold)) {
    if(km_x1 < km_x2)     /* put coords in ascending order */
    {
        gd.h_win.zmin_x[index] = km_x1;
        gd.h_win.zmax_x[index] = km_x2;
    }
    else
    {
        gd.h_win.zmin_x[index] = km_x2;
        gd.h_win.zmax_x[index] = km_x1;
    }
    if(km_y1 < km_y2)
    {     
        gd.h_win.zmin_y[index] = km_y1;
        gd.h_win.zmax_y[index] = km_y2;
    }
    else
    {
        gd.h_win.zmin_y[index] = km_y2;
        gd.h_win.zmax_y[index] = km_y1;
    }
}

    /* Force zoomed display area to be consistant with the window */
    dx = gd.h_win.zmax_x[index] - gd.h_win.zmin_x[index];
    dy = gd.h_win.zmax_y[index] - gd.h_win.zmin_y[index];

    switch(gd.display_projection) {
	case Mdvx::PROJ_LATLON:
	   // Use the current average latitude to set the aspect correction
	   gd.aspect_correction = cos(((gd.h_win.zmax_y[index] + gd.h_win.zmin_y[index])/2.0) * DEG_TO_RAD);

	   /* forshorten the Y coords to make things look better */
	   dy /= gd.aspect_correction;
	break;
    }

    /* Force the domain into the aspect ratio */
    dy *= gd.aspect_ratio;

    /* use largest dimension */
    if(dx > dy)
    {
        gd.h_win.zmax_y[index] += (dx - dy) /2.0;
        gd.h_win.zmin_y[index] -= (dx - dy) /2.0;
    }
    else
    {
        gd.h_win.zmax_x[index] += (dy - dx) /2.0;
        gd.h_win.zmin_x[index] -= (dy - dx) /2.0;
    }

    /* make sure coords are within the limits of the display */
    if(gd.h_win.zmax_x[index] > gd.h_win.max_x)
    {
        gd.h_win.zmin_x[index] -= (gd.h_win.zmax_x[index] - gd.h_win.max_x);
        gd.h_win.zmax_x[index] = gd.h_win.max_x;
    }
    if(gd.h_win.zmax_y[index] > gd.h_win.max_y)
    {
        gd.h_win.zmin_y[index] -= (gd.h_win.zmax_y[index] - gd.h_win.max_y);
        gd.h_win.zmax_y[index] = gd.h_win.max_y;
    }
    if(gd.h_win.zmin_x[index] < gd.h_win.min_x)
    {
        gd.h_win.zmax_x[index] += (gd.h_win.min_x - gd.h_win.zmin_x[index]);
        gd.h_win.zmin_x[index] = gd.h_win.min_x;
    }
    if(gd.h_win.zmin_y[index] < gd.h_win.min_y)
    {
        gd.h_win.zmax_y[index] += (gd.h_win.min_y - gd.h_win.zmin_y[index]);
        gd.h_win.zmin_y[index] = gd.h_win.min_y;
    }

    /* Set current area to our indicated zoom area */
    gd.h_win.cmin_x = gd.h_win.zmin_x[index];
    gd.h_win.cmax_x = gd.h_win.zmax_x[index];
    gd.h_win.cmin_y = gd.h_win.zmin_y[index];
    gd.h_win.cmax_y = gd.h_win.zmax_y[index];

    if(gd.syprod_P->use_domain_limits) {
	 double min_lat,max_lat,min_lon,max_lon;
	 get_bounding_box(min_lat,max_lat,min_lon,max_lon);

	 gd.r_context->set_clip_limits(min_lat,min_lon,gd.h_win.min_ht,
				       max_lat,max_lon,gd.h_win.max_ht);
         gd.prod_mgr->reset_product_valid_flags();
    }

    gd.h_win.zoom_level = index;
    xv_set(gd.zoom_pu->domain_st, PANEL_VALUE, index, NULL);

    set_redraw_flags(1,0);
    if(!gd.always_get_full_domain) {
      reset_time_list_valid_flags();
      reset_data_valid_flags(1,0);
      reset_terrain_valid_flags(1,0);
    }
}

/*************************************************************************
 * DO_ZOOM_PAN : Pan the zoomed area
 */

void do_zoom_pan(void )
{
    int    index;
    double    km_x1,km_x2,km_y1,km_y2;
    double    dx,dy;
     
    pixel_to_disp_proj(&gd.h_win.margin,p_startx,p_starty,&km_x1,&km_y1);
    pixel_to_disp_proj(&gd.h_win.margin,p_lastx,p_lasty,&km_x2,&km_y2);
     
    dx = km_x1 - km_x2;
    dy = km_y1 - km_y2;

    if(gd.h_win.zoom_level < gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS) {
          index = gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS;
    } else {
      index = gd.h_win.zoom_level;
      if(index >= gd.h_win.num_zoom_levels)
          index = gd.h_win.num_zoom_levels -1;
    }

    gd.h_win.zmin_x[index] = gd.h_win.cmin_x + dx;
    gd.h_win.zmin_y[index] = gd.h_win.cmin_y + dy;
    gd.h_win.zmax_x[index] = gd.h_win.cmax_x + dx;
    gd.h_win.zmax_y[index] = gd.h_win.cmax_y + dy;

    /* make sure coords are within the limits of the display */
    if(gd.h_win.zmax_x[index] > gd.h_win.max_x)
    {
        gd.h_win.zmin_x[index] -= (gd.h_win.zmax_x[index] - gd.h_win.max_x);
        gd.h_win.zmax_x[index] = gd.h_win.max_x;
    }
    if(gd.h_win.zmax_y[index] > gd.h_win.max_y)
    {
        gd.h_win.zmin_y[index] -= (gd.h_win.zmax_y[index] - gd.h_win.max_y);
        gd.h_win.zmax_y[index] = gd.h_win.max_y;
    }
    if(gd.h_win.zmin_x[index] < gd.h_win.min_x)
    {
        gd.h_win.zmax_x[index] += (gd.h_win.min_x - gd.h_win.zmin_x[index]);
        gd.h_win.zmin_x[index] = gd.h_win.min_x;
    }
    if(gd.h_win.zmin_y[index] < gd.h_win.min_y)
    {
        gd.h_win.zmax_y[index] += (gd.h_win.min_y - gd.h_win.zmin_y[index]);
        gd.h_win.zmin_y[index] = gd.h_win.min_y;
    }

    /* Set current area to our indicated zoom area */
    gd.h_win.cmin_x = gd.h_win.zmin_x[index];
    gd.h_win.cmax_x = gd.h_win.zmax_x[index];
    gd.h_win.cmin_y = gd.h_win.zmin_y[index];
    gd.h_win.cmax_y = gd.h_win.zmax_y[index];

	 // Use the current average latitude to set the aspect correction
	 gd.aspect_correction = cos(((gd.h_win.zmax_y[index] + gd.h_win.zmin_y[index])/2.0) * DEG_TO_RAD);
     
    if(gd.syprod_P->use_domain_limits) {
	 double min_lat,max_lat,min_lon,max_lon;
	 get_bounding_box(min_lat,max_lat,min_lon,max_lon);

	 gd.r_context->set_clip_limits(min_lat,min_lon,gd.h_win.min_ht,
				       max_lat,max_lon,gd.h_win.max_ht);
         gd.prod_mgr->reset_product_valid_flags();
    }

    gd.h_win.zoom_level = index;
    xv_set(gd.zoom_pu->domain_st, PANEL_VALUE, index, NULL);

    set_redraw_flags(1,0);
    if(!gd.always_get_full_domain) {
      reset_time_list_valid_flags();
      reset_data_valid_flags(1,0);
      reset_terrain_valid_flags(1,0);
    }

}


/*************************************************************************
 * ERASE_ZOOM_BOX:  Erases the Zoom box 
 *
 */

void erase_zoom_box(void)
{
    int    dx,dy;                /* pixel distances */
    int    sx,sy;

    dx = b_lastx - b_startx;
    dy = b_lasty - b_starty;

    if(dx < 0) { sx = b_startx + dx; dx = -dx; } else {   sx = b_startx; } 
    if(dy < 0) { sy = b_starty + dy; dy = -dy; } else {   sy = b_starty; } 

   if ( PseudoColor == xv_get(gd.h_win_horiz_bw->horiz_bw,XV_VISUAL_CLASS)) {
     XDrawRectangle(gd.dpy,gd.hcan_xid,gd.clear_ol_gc,sx,sy,dx,dy);
   } else {
      XDrawRectangle(gd.dpy,gd.hcan_xid,gd.ol_gc,sx,sy,dx,dy); /* XOR MODE */
   }
}

/*************************************************************************
 * REDRAW_ZOOM_BOX:  Draws the Zoom define box 
 *
 */

void redraw_zoom_box(void)
{
    int    dx,dy;                /* pixel distances */
    int    sx,sy;

    dx = b_lastx - b_startx;
    dy = b_lasty - b_starty;

    if(dx < 0) { sx = b_startx + dx; dx = -dx; } else {   sx = b_startx; } 
    if(dy < 0) { sy = b_starty + dy; dy = -dy; } else {   sy = b_starty; } 

    XDrawRectangle(gd.dpy,gd.hcan_xid,gd.ol_gc,sx,sy,dx,dy);
}
 
/*************************************************************************
 * REDRAW_PAN_LINE: Redraw the Pan Define line 
 *
 */

void redraw_pan_line(void)
{

//    XDrawLine(gd.dpy,gd.hcan_xid,gd.clear_ol_gc,p_startx,p_starty,p_lastx,p_lasty); /* BIT PLANE MODE */
    XDrawLine(gd.dpy,gd.hcan_xid,gd.ol_gc,p_startx,p_starty,p_lastx,p_lasty); /* XOR MODE */
}
