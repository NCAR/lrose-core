/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/**********************************************************************
 * RENDER_EXTRAS.C: Supervise the rendering of all "extra" features 
 *
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#include <X11/Xlib.h>

#define RENDER_EXTRAS
#include "cidd.h"

#ifdef IRIX5
#define M_PI            3.14159265358979323846
#endif

#include <rapplot/xrs.h>
#include <rapplot/xudr.h>
#include <rapplot/rascon.h>

/**********************************************************************
 * RENDER_HORIZ_EXTRAS: Render selected extra features and data onto
 *    the horizontal display - Contours, Wind vectors, range rings and azmith
 *    lines
 */

render_horiz_extras(xid,field,start_time,end_time)
    Drawable xid;
    int    field;
    long    start_time,end_time;
{
    int    i,j;
    int    startx,starty,endx,endy;
    int    x1,y1,x2,y2;
    int    x_grid[MAX_COLS],y_grid[MAX_ROWS];
    int    cont_labels;

    double delt;
    double levels[MAX_CONT_LEVELS];
    GC    gc_array[MAX_CONT_LEVELS];

    double    x_km,y_km;
    double    val;
    int    i_val;
    unsigned char    *ptr;
    unsigned char    bad_data;
    met_record_t *mr;       /* convienence pointer to a data record */

    if( gd.v_win.active) {
            /* render the RHI cross reference line */
        km_to_pixel(&gd.h_win.margin,gd.v_win.cmin_x,gd.v_win.cmin_y,&x1,&y1);
        km_to_pixel(&gd.h_win.margin,gd.v_win.cmax_x,gd.v_win.cmax_y,&x2,&y2);
    
        XDrawLine(gd.dpy,xid,gd.extras.foreground_color->gc,x1,y1,x2,y2);
    }

    /* render contours if selected */
    for(j= 0; j < NUM_CONT_LAYERS; j++) {
      if(gd.extras.cont[j].active) {

	if(gd.extras.cont[j].labels_on) {
	   cont_labels = RASCON_DOLABELS;
	   } else {
	   cont_labels = RASCON_NOLABELS;
	}
        mr = gd.mrec[gd.extras.cont[j].field];
		XSetFont(gd.dpy,gd.extras.cont[j].color->gc,gd.ciddfont[gd.prod.prod_font_num]);

        ptr = (unsigned char *)  mr->h_data;
        if(ptr != NULL) {    /* data is available */

            grid_to_km(mr,mr->x1,mr->y1,&x_km,&y_km);
            km_to_pixel(&gd.h_win.margin,x_km,y_km,&startx,&starty);
        
            grid_to_km(mr,mr->x2,mr->y2,&x_km,&y_km);
            km_to_pixel(&gd.h_win.margin,x_km,y_km,&endx,&endy);
            
            delt = (endx - startx) / (double) (mr->h_nx -1);
            for(i=0; i < mr->h_nx; i++) {
                x_grid[i] = startx + (i * delt);
            }
            
            delt = (endy - starty) / (double) (mr->h_ny -1);
            for(i=0; i < mr->h_ny; i++) {
                y_grid[i] = starty + (i * delt);
            }
    
            gd.extras.cont[j].num_levels = 0;
            val = gd.extras.cont[j].min;
            for(i=0; (gd.extras.cont[j].num_levels < MAX_CONT_LEVELS ) && (val < gd.extras.cont[j].max);i++) {
                val = gd.extras.cont[j].min + ( i * gd.extras.cont[j].interval);
		i_val = (val - mr->h_bias) / mr->h_scale;
		if(i_val > 0 && i_val < 255) {
                    levels[gd.extras.cont[j].num_levels] = i_val;
                    gc_array[gd.extras.cont[j].num_levels] = gd.extras.cont[j].color->gc;
                    gd.extras.cont[j].num_levels++;
		}
             }

            bad_data = mr->missing_val;
	    if(gd.extras.cont[j].num_levels > 0) {
                RASCONinit(gd.dpy,xid,mr->h_nx,mr->h_ny,x_grid,y_grid);
                RASCONcontour(gc_array,ptr,&bad_data,RASCON_CHAR,
                    gd.extras.cont[j].num_levels,
                    levels,
                    RASCON_LINE_CONTOURS, cont_labels,
                    mr->h_scale,mr->h_bias);
	    }
             

        }
      }
    } /* End of for each contour layer */
     
    /* render range_rings if selected */
    if(gd.extras.range) {
        draw_cap_range_rings(xid,field);
    }
     
    /* render azmith lines if selected */
    if(gd.extras.azmiths) {
        draw_cap_azmith_lines(xid,field);
    }
     
    /* render Winds if selected */
    if(gd.extras.wind_vectors) {
	switch(gd.extras.wind_mode) {
	    default:
	    case WIND_MODE_ON:  /* winds get rendered in each frame */
                render_wind_vectors(xid,start_time,end_time);
	    break;

	    case WIND_MODE_LAST: /* Winds get rendered in last farame only */
		if(gd.movie.cur_frame == gd.movie.end_frame)
		    render_wind_vectors(xid,start_time,end_time);
	    break;

	    case WIND_MODE_STILL: /* Winds get rendered in the last frame only
				   * if the movie loop is off 
				   */
		if(!gd.movie.movie_on && gd.movie.cur_frame == gd.movie.end_frame)
		    render_wind_vectors(xid,start_time,end_time);
	    break;
	}
    }

	render_overlays(xid);

    return SUCCESS;
}

/**********************************************************************
 * DRAW_CAP_AZMITH_LINES:
 *
 */

draw_cap_azmith_lines(xid,field)
    Drawable xid;
    int    field;
{
    int    x1,y1,x2,y2;
    double    x_km,y_km;
    double    xstart,ystart;
    double    xend,yend;
    double    cur_angle;
    static double interval;
    static double radius;

    if(interval == 0.0) {
        interval = XRSgetLong(gd.cidd_db, "cidd.azmith_interval", 30.0);
        interval *= (M_PI/180.0); /* convert to radians */
        interval = fabs(interval);
    }

    if(radius == 0.0) {
        radius = XRSgetLong(gd.cidd_db, "cidd.azmith_radius", 200.0);
    }

#ifdef USE_DATA_ORIGIN
    /* get km coords of data origin */
    PJGLatLon2DxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,
        gd.mrec[field]->origin_lat,
        gd.mrec[field]->origin_lon,
        &x_km,&y_km);
#else
    switch(gd.projection_mode) {
      case CARTESIAN_PROJ:
        /* Place at Displays origin */
        x_km = 0.0;
        y_km = 0.0; 
      break;

      case LAT_LON_PROJ:
        /* Place at Specified origin */
        x_km = gd.h_win.origin_lon;
        y_km = gd.h_win.origin_lat;
      break;
    }

#endif

    cur_angle = 0.0;
    while(cur_angle < M_PI) {
        xstart = x_km + (cos(cur_angle) * radius);
        ystart = y_km + (sin(cur_angle) * radius);
        xend = x_km - (cos(cur_angle) * radius);
        yend = y_km - (sin(cur_angle) * radius);
        km_to_pixel(&gd.h_win.margin,xstart,ystart,&x1,&y1);
        km_to_pixel(&gd.h_win.margin,xend,yend,&x2,&y2);

        XUDRline_clip(gd.dpy, xid, gd.extras.range_color->gc,
                      x1, y1, x2, y2,
                      gd.h_win.margin.left,
                      gd.h_win.margin.top,
                      gd.h_win.img_dim.width + gd.h_win.margin.left,
                      gd.h_win.img_dim.height + gd.h_win.margin.top);

        cur_angle += interval;
    }
}

/**********************************************************************
 * DRAW_CAP_RANGE_RINGS:
 *
 */

draw_cap_range_rings(xid,field)
    Drawable xid;
    int    field;
{
    int    x,y;
    int    c_x,c_y,radius;
    int    xmid,ymid;
    double    min_r,max_r;
    double    x_km,y_km;
    double    dist;
    double    cur_rad;
    double    min_rad;
    double    interval;
    double    angle;
    double    sin_ang,cos_ang;
    char    label[LABEL_LENGTH];
    Font    font;
    static int    x_space,y_space;

    static double   ring_spacing = 0.0;
    static double  unit_per_km = 0.0;
    static char *u_label;
     
    if(ring_spacing == 0.0) {
        x_space = XRSgetLong(gd.cidd_db, "cidd.range_ring_x_space", 40);
        y_space = XRSgetLong(gd.cidd_db, "cidd.range_ring_y_space", 15);
        ring_spacing = XRSgetDouble(gd.cidd_db, "cidd.range_ring_spacing", -1.0);
        unit_per_km = XRSgetDouble(gd.cidd_db, "cidd.units_per_km", 1.0);
        u_label = XRSgetString(gd.cidd_db, "cidd.scale_units_label", "km");
    }
     
    if(ring_spacing > 0.0) {
        interval = ring_spacing;
    } else {
        dist = compute_range (gd.h_win.cmin_x,gd.h_win.cmin_y,gd.h_win.cmax_x,gd.h_win.cmax_y);
        dist *= unit_per_km;
        interval = compute_tick_interval(dist);
    }


#ifdef USE_DATA_ORIGIN
    /* get km coords of data origin */
    PJGLatLon2DxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,
        gd.mrec[field]->origin_lat,
        gd.mrec[field]->origin_lon,
        &x_km,&y_km);
#else
    switch(gd.projection_mode) {
      case CARTESIAN_PROJ:
        /* Place at Displays origin */
        x_km = 0.0;
        y_km = 0.0; 
      break;

      case LAT_LON_PROJ:
        /* Place at Specified origin */
        x_km = gd.h_win.origin_lon;
        y_km = gd.h_win.origin_lat;
      break;
    }

#endif

    if((gd.h_win.cmin_x < x_km) && (gd.h_win.cmax_x > x_km) &&
        (gd.h_win.cmin_y < y_km) && (gd.h_win.cmax_y > y_km)) {
        min_r = interval;
    } else {
        min_r = interval * 1000.0;
        dist = ABSDIFF(gd.h_win.cmin_x,x_km);
        if(dist < min_r) min_r = dist;
        dist = ABSDIFF(gd.h_win.cmax_x,x_km);
        if(dist < min_r) min_r = dist;
        dist = ABSDIFF(gd.h_win.cmin_y,y_km);
        if(dist < min_r) min_r = dist;
        dist = ABSDIFF(gd.h_win.cmax_y,y_km);
        if(dist < min_r) min_r = dist;
    }
    min_r *= unit_per_km;

    /* find distances to corners - pick the furthest corner for quantities */
    max_r = interval;
    dist = compute_range (x_km,y_km,gd.h_win.cmin_x,gd.h_win.cmin_y);
    dist *= unit_per_km;
    if(dist < min_r) min_r = dist;
    if(dist > max_r){
        max_r = dist;
        angle = atan2((gd.h_win.cmin_y - y_km),(gd.h_win.cmin_x - x_km));
    }

    dist = compute_range (x_km,y_km,gd.h_win.cmax_x,gd.h_win.cmax_y);
    dist *= unit_per_km;
    if(dist < min_r) min_r = dist;
    if(dist > max_r){
        max_r = dist;
        angle = atan2((gd.h_win.cmax_y - y_km),(gd.h_win.cmax_x - x_km));
    }

    dist = compute_range (x_km,y_km,gd.h_win.cmin_x,gd.h_win.cmax_y);
    dist *= unit_per_km;
    if(dist < min_r) min_r = dist;
    if(dist > max_r) {
        max_r = dist;
        angle = atan2((gd.h_win.cmax_y - y_km),(gd.h_win.cmin_x - x_km));
    }

    dist = compute_range (x_km,y_km,gd.h_win.cmax_x,gd.h_win.cmin_y);
    dist *= unit_per_km;
    if(dist < min_r) min_r = dist;
    if(dist > max_r) {
        max_r = dist;
        angle = atan2((gd.h_win.cmin_y - y_km),(gd.h_win.cmax_x - x_km));
    }

    sin_ang = -sin(angle);
    cos_ang = cos(angle);

    /* Get pixel coords of center of coord system */
    km_to_pixel(&gd.h_win.margin,x_km,y_km,&c_x,&c_y);

    min_rad = rint(min_r);
    cur_rad = min_rad - abs((int)min_rad % (int)((interval > 1.0) ? interval:  1.0));
    if(cur_rad < 1.0) cur_rad += interval;

    while(cur_rad <= max_r) {
        km_to_pixel(&gd.h_win.margin,x_km + cur_rad/unit_per_km,y_km,&x,&y);
        radius = ABSDIFF(c_x,x);
        XUDRcircle_segment(gd.dpy, xid, gd.extras.range_color->gc,
                           c_x, c_y, radius,
                           gd.h_win.margin.left,
                           gd.h_win.margin.top,
                           gd.h_win.img_dim.width + gd.h_win.margin.left,
                           gd.h_win.img_dim.height + gd.h_win.margin.top);


        sprintf(label,"%.0f %s",cur_rad,u_label);
        font = choose_font(label,x_space,y_space,&xmid,&ymid);
         
        /* clear background behind range label */
        XFillRectangle(gd.dpy,xid,gd.extras.background_color->gc,
            (int)(c_x + xmid  + (cos_ang * radius)),
            (int)(c_y - ymid  + (sin_ang * radius)),
            (-xmid*2),(ymid*2));
         
        XSetFont(gd.dpy,gd.extras.range_color->gc,font);
        XDrawImageString(gd.dpy,xid,gd.extras.range_color->gc,
            (int)(c_x + xmid + (cos_ang * radius)),
            (int)(c_y + ymid + (sin_ang * radius)),
            label,strlen(label));

        cur_rad += interval;
    }

}
