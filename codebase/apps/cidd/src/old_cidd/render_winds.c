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
 * RENDER_WINDS: Render Wind fields for the CIDD display program
 *
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#include <X11/Xlib.h>

#define RENDER_WINDS
#include "cidd.h"

#include <rapplot/xrs.h>
#include <rapplot/xudr.h>


static double  head_angle = 0.0;
static double w_scale_factor = 0.0;
static int    marker_type = 0;
static int    ideal_x_vects = 0;
static int    ideal_y_vects = 0;
static int    head_size = 0;
static int    shaft_len = 0;
/**********************************************************************
 * RENDER_WIND_VECTORS:
 *
 */
int
render_wind_vectors(Drawable xid, int start_time, int end_time)
{
    int    i,j,k;
    int    nx,ny; /* number of points in data grid */
    int	   num_x,num_y; /* number of grid cells in current area */
    int    num_ticks;
    int    xmid,ymid;
    int    width,height;
    int    stretch_secs;
    int    x1,y1,x2,y2,x3,y3;
    int    xstart,ystart,xend,yend;
    int    x_jump,y_jump;
    int    x_start[MAX_COLS];    /* canvas rectangle begin  coords */
    int    y_start[MAX_ROWS];    /* canvas  rectangle begin coords */
    double    g_width,g_height;
    double    uspeed,vspeed;
    double    xscale,yscale;
    double    xbias,ybias;
    double    slope,val;
    double    head_slope;
    double    km_x,km_y;
    double    speed,ref;        /* Reference marker */
    char    label[16];        /* Label for reference marker */
    char    *type_ptr;
    unsigned char    u_miss,v_miss;        /* missing data values */
    unsigned char    *ptr_u;
    unsigned char    *ptr_v;
    Font    font;

    if(ideal_x_vects == 0) {
        ideal_x_vects = XRSgetLong(gd.cidd_db, "cidd.ideal_x_vectors", 20);
        ideal_y_vects = XRSgetLong(gd.cidd_db, "cidd.ideal_y_vectors", 20);
        head_size = XRSgetLong(gd.cidd_db, "cidd.wind_head_size", 5);
        shaft_len = XRSgetLong(gd.cidd_db, "cidd.barb_shaft_len", 25);
        head_angle = XRSgetDouble(gd.cidd_db, "cidd.wind_head_angle", 45.0);
        type_ptr = XRSgetString(gd.cidd_db, "cidd.wind_marker_type", "arrow");
        marker_type = 1;   /* use centered arrows by default */
        if(strncmp(type_ptr, "tuft", 4) == 0)  marker_type = 2;
        if(strncmp(type_ptr, "barb", 4) == 0)  marker_type = 3;
        if(strncmp(type_ptr, "vector", 4) == 0)  marker_type = 4;
        if(strncmp(type_ptr, "tickvector", 4) == 0)  marker_type = 5;
    }
     
    for(k=0; k < gd.extras.num_wind_sets; k++ ) {
        if(gd.extras.wind[k].active == 0) continue;
        nx = gd.extras.wind[k].wind_u->h_nx;
        ny = gd.extras.wind[k].wind_u->h_ny;
        if(nx == 0 || ny == 0) continue;

        /* get pointers to desired sections */
        ptr_u = (unsigned char *) gd.extras.wind[k].wind_u->h_data;
         
        ptr_v = (unsigned char *) gd.extras.wind[k].wind_v->h_data;

        /* Make sure the data is valid to plot */
        if((ptr_u == NULL) || ( ptr_v == NULL)) continue;
        stretch_secs =  60.0 *  gd.extras.wind[k].wind_u->time_allowance;
	if(gd.check_data_times) {
          if(gd.extras.wind[k].wind_u->h_date.unix_time < start_time - stretch_secs) continue;
          if(gd.extras.wind[k].wind_u->h_date.unix_time > end_time + stretch_secs) continue;
	}

        /* Get Canvas coordinates for wind vector points */
        grid_to_km(gd.extras.wind[k].wind_v,gd.extras.wind[k].wind_v->x1,gd.extras.wind[k].wind_v->y1,&km_x,&km_y);
        km_to_pixel(&(gd.h_win.margin),km_x,km_y,&xstart,&ystart);
    
        grid_to_km(gd.extras.wind[k].wind_v,gd.extras.wind[k].wind_v->x2,gd.extras.wind[k].wind_v->y2,&km_x,&km_y);
        km_to_pixel(&(gd.h_win.margin),km_x,km_y,&xend,&yend);
    
        /* get drawable dimensions */
        width = abs((xstart-xend) + 1);
        height = abs((ystart-yend) + 1);
    
        g_width = (nx > 1) ?  (double)width / (nx -1): 0.0 ;
        g_height = (ny > 1) ? (double)height / (ny -1) : 0.0;
    
        for(i= 0; i < ny; i++) {    /* Calc starting coords for the array */
            if(gd.extras.wind[k].wind_u->order) {
                y_start[i] = ystart - ((double)i * g_height);
            } else {
                y_start[i] = ((double) i * g_height) + ystart;
            }
        }
        for(j=0;j< nx; j++) {
            x_start[j] = ((double) j * g_width) + xstart;
        }
        /* get scalers for data  */

        /* Distance is scaled in increments of N minutes */
        val = (xv_get(gd.extras_pu->wind_sl,PANEL_VALUE) + 1) * gd.extras.wind_time_scale_interval;

        gd.extras.wind[k].wind_ms = ((val* 60.0) / 1000.0)/ gd.extras.wind[k].wind_u->dx;
	if(gd.projection_mode == LAT_LON_PROJ)
              	gd.extras.wind[k].wind_ms /= KM_PER_DEG_AT_EQ;
        xscale =  ((double)width / nx) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_u->h_scale;
        xbias = ((double)width / nx) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_u->h_bias;
    
        yscale = ((double)height / ny) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_v->h_scale;
        ybias = ((double)height /ny) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_v->h_bias;
    
     
        u_miss = gd.extras.wind[k].wind_u->missing_val;
        v_miss = gd.extras.wind[k].wind_v->missing_val;
    
	num_x = (int) ((gd.h_win.cmax_x - gd.h_win.cmin_x) / gd.extras.wind[k].wind_u->dx);
        x_jump = (num_x / ideal_x_vects) + 0.5;    /* Try for approx ideal number of vectors on the screen at one time */
        if(x_jump < 1) x_jump = 1;


	num_y = (int) ((gd.h_win.cmax_y - gd.h_win.cmin_y) / gd.extras.wind[k].wind_u->dy);
        y_jump = (num_y / ideal_y_vects) + 0.5;
        if(y_jump < 1) y_jump = 1;
         
        /* loop through the array and draw the vectors */
        for(i = 0; i < ny; i++) {
            for(j=0; j < nx; j++) {
                /* If data point is missing or data grid  needs decimated */
                if((*ptr_u == u_miss) || (*ptr_v == v_miss) || (i%y_jump != 0) || (j%x_jump != 0)) {
                    /* Do nothing */
                } else {
    
                    /* compute vertex position */
                    x1 = x_start[j];
                    y1 = y_start[i];
    
                    switch(marker_type) {
                      default :
                      case 1:  /* Use (centered) arrows */
        
                        /* compute vector head position */
                        x2 = x1 + (((xscale * *ptr_u) + xbias) * 0.5);
                        y2 = y1 - (((yscale * *ptr_v) + ybias) * 0.5);
        
                        /* compute vector tail position */
                        x3 = x1 - (((xscale * *ptr_u) + xbias) * 0.5);
                        y3 = y1 + (((yscale * *ptr_v) + ybias) * 0.5);
        
						/* Stop drawing winds when the features are more than
						 * 25 % of the size of the image */
                        if((abs(x2-x3) < width/4) && (abs(y2-y3) < height/4))
                        XUDRarrow(gd.dpy,xid,gd.extras.wind[k].color->gc,x3,y3,x2,y2,head_size,head_angle * DEG_TO_RAD);
                      break;

                      case 2: /* Use tufts */
    
                        XUDRcircle(x1, y1, 1, gd.dpy, xid, gd.extras.wind[k].color->gc); /*  */
                        x2 = x1 + ((xscale * *ptr_u) + xbias);
                        y2 = y1 - ((yscale * *ptr_v) + ybias);
						/* Stop drawing winds when the features are more than
						 * 25 % of the size of the image */
                        if((abs(x2-x1) < width/4) && (abs(y2-y1) < height/4))
                        XDrawLine(gd.dpy,xid,gd.extras.wind[k].color->gc,x2,y2,x1,y1);
                      break;

                      case 3: /* Use barbs */
    
                        uspeed = (*ptr_u * gd.extras.wind[k].wind_u->h_scale) + gd.extras.wind[k].wind_u->h_bias;
                        vspeed = (*ptr_v * gd.extras.wind[k].wind_v->h_scale) + gd.extras.wind[k].wind_v->h_bias;
                        XUDRwind_barb(gd.dpy,xid,gd.extras.wind[k].color->gc,x1, y1,uspeed,vspeed,shaft_len);
                      break;

                      case 6: /* Use vectors */
    
                        x2 = x1 + ((xscale * *ptr_u) + xbias);
                        y2 = y1 - ((yscale * *ptr_v) + ybias);
						/* Stop drawing winds when the features are more than
						 * 25 % of the size of the image */
                        if((abs(x2-x1) < width/4) && (abs(y2-y1) < height/4))
							XUDRarrow(gd.dpy,xid,gd.extras.wind[k].color->gc,x1,y1,x2,y2,head_size,head_angle * DEG_TO_RAD);
                      break;
    
                      case 5: /* Use vectors with ticks */
                        x2 = x1 + ((xscale * *ptr_u) + xbias);
                        y2 = y1 - ((yscale * *ptr_v) + ybias);
			num_ticks = (val / gd.extras.wind_time_scale_interval) - 1;
			if (num_ticks <= 0) num_ticks = 1;
						/* Stop drawing winds when the features are more than
						 * 25 % of the size of the image */
                        if((abs(x2-x1) < width/4) && (abs(y2-y1) < height/4))
                            XUDRtickarrow(gd.dpy,xid,gd.extras.wind[k].color->gc,x1,y1,x2,y2,head_size,head_angle * DEG_TO_RAD,num_ticks,4);
                      break;
                    }
                }
    
                ptr_v++;
                ptr_u++;
            }
        }
        /* Add a reference line */
        speed = 5.0;
        ref = (speed - gd.extras.wind[k].wind_u->h_bias) / gd.extras.wind[k].wind_u->h_scale;

        x1 = gd.h_win.margin.left + ((gd.h_win.can_dim.width * (k+1.0))/4.0);
        y1 = gd.h_win.margin.top * 1.25;

        x2 = x1 + ((xscale * ref) + xbias);
        y2 = y1;
        XUDRarrow(gd.dpy,xid,gd.extras.wind[k].color->gc,x1,y1,x2,y2,head_size,head_angle * DEG_TO_RAD);
    
        sprintf(label,"%g m/sec",speed);
        font = choose_font(label,((x2 -x1) * 5), gd.h_win.margin.top,&xmid,&ymid);
        XSetFont(gd.dpy,gd.extras.wind[k].color->gc,font);
	if(gd.font_display_mode == 0) 
          XDrawString(gd.dpy,xid,gd.extras.wind[k].color->gc, x2 +4,y1 + ymid,label,strlen(label));
	else
          XDrawImageString(gd.dpy,xid,gd.extras.wind[k].color->gc, x2 +4,y1 + ymid,label,strlen(label));
    }
         
    return(0);
}

/**********************************************************************
 * RENDER_VERT_WIND_VECTORS: Render vertical Cross section winds 
 */

render_vert_wind_vectors(xid)
    Drawable xid;
{
    int i,j,k;
    int ht,wd;              /* Dims of drawing canvas */
    int xmid,ymid;
    int nx,ny;
    int x1,y1,x2,y2,x3,y3;
    int width,x_jump,y_jump;
    int endx,endy;          /* Pixel limits of data area */
    int startx,starty;      /* Pixel limits of data area */
    int x_start[MAX_COLS];  /* canvas rectangle begin  coords */
    int y_start[MAX_ROWS];  /* canvas  rectangle begin coords */
    int y_end[MAX_ROWS];    /* canvas  rectangle end coords */
    double  g_width,g_height;
    double  xscale,yscale,zscale;
    double  xbias,ybias,zbias;
    double  slope,val;
    double  head_slope;
    double    x_proj,y_proj;
    double  speed,ref;      /* Reference marker */
    char    label[16];      /* Label for reference marker */
    double  dist;           /* distance between corner of vert section and grid point */
    double  span;           /* distance between corners of vert section */
    double  height;
    double  x_km,y_km;
    double  r_ht,r_wd;      /*  data point rectangle dims */
    double theta,sin_theta,cos_theta;
    double u_proj,v_proj;

    unsigned char   *ptr_u;
    unsigned char   *ptr_v;
    unsigned char   *ptr_w;
    unsigned char   miss_u;           /* Missing data value */
    unsigned char   miss_v;           /* Missing data value */
    unsigned char   miss_w;           /* Missing data value */
    met_record_t *mr_u;       /* pointer to U record for convienence */
    met_record_t *mr_v;       /* pointer to V record for convienence */
    met_record_t *mr_w;       /* pointer to W record for convienence */
    char    message[32];
    Font    font;

    if(xid == 0) return FAILURE;
    if(w_scale_factor == 0.0) {
        w_scale_factor = XRSgetDouble(gd.cidd_db, "cidd.wind_w_scale_factor", 10.0);
    }

    /* Calc coefficients for projection of U and V onto the cross section plane */
    theta = atan2((gd.v_win.cmax_y - gd.v_win.cmin_y),(gd.v_win.cmax_x - gd.v_win.cmin_x));
    sin_theta = sin(theta);
    cos_theta = cos(theta);

    for(k=0; k < gd.extras.num_wind_sets; k++ ) {
        if(gd.extras.wind[k].active == 0) continue;
        if(gd.extras.wind[k].wind_w == NULL) continue;

        /* get pointers to desired sections */
        ptr_u = (unsigned char *) gd.extras.wind[k].wind_u->v_data;
        ptr_v = (unsigned char *) gd.extras.wind[k].wind_v->v_data;
        ptr_w = (unsigned char *) gd.extras.wind[k].wind_w->v_data;
        if((ptr_u == NULL) || ( ptr_v == NULL) || ( ptr_w == NULL)) continue;
         
        mr_u = gd.extras.wind[k].wind_u;    /* get pointer to U data record */
        mr_v = gd.extras.wind[k].wind_v;    /* get pointer to V data record */
        mr_w = gd.extras.wind[k].wind_w;    /* get pointer to W data record */

        nx = mr_u->v_nx;
        ny = mr_u->v_ny;

        if(mr_v->v_nx != nx) continue;    /* Consistancy check */
        if(mr_v->v_ny != ny) continue;
        if(mr_w->v_nx != nx) continue;
        if(mr_w->v_ny != ny) continue;


        /* Compute distances in  this cross section */
        grid_to_km(mr_u,mr_u->vx1,mr_u->vy1,&x_km,&y_km);
        span = compute_range(gd.v_win.cmin_x, gd.v_win.cmin_y,gd.v_win.cmax_x, gd.v_win.cmax_y);
        dist = compute_range(gd.v_win.cmax_x, gd.v_win.cmax_y,x_km,y_km);
        height = mr_u->vert[mr_u->z1].min;
        km_to_pixel_v(&(gd.v_win.margin),(span - dist),height,&startx,&starty);
    
        grid_to_km(mr_u,mr_u->vx2,mr_u->vy2,&x_km,&y_km);
        dist = compute_range(gd.v_win.cmin_x, gd.v_win.cmin_y,x_km,y_km);
        height = mr_u->vert[mr_u->z2].max;
        km_to_pixel_v(&(gd.v_win.margin),dist,height,&endx,&endy);

        r_wd =  (double)ABSDIFF(endx,startx) / (double) nx;       /* get data point rectangle width */
        for(j=0;j< nx; j++) {
            x_start[j] = ((double) j * r_wd) + startx;
        }
    
        for(i= 0; i < mr_u->v_ny; i++) {  /* Calc starting/ending coords for the array */
            height = mr_u->vert[mr_u->z1 + i].min;
            km_to_pixel_v(&(gd.v_win.margin),dist,height,&endx,&(y_start[i]));
            height = mr_u->vert[mr_u->z1 + i].max;
            km_to_pixel_v(&(gd.v_win.margin),dist,height,&endx,&(y_end[i]));
        }
    
        /* get drawable dimensions */
        width = abs((startx-endx)) + 1;
        height = abs((y_start[0] - y_end[mr_u->v_ny -1])) + 1;
    
        miss_u = mr_u->missing_val;
        miss_v = mr_v->missing_val;
        miss_w = mr_w->missing_val;
        wd = r_wd + 1.0;

        /* get scalers for data  */
        if(gd.extras.wind[k].wind_ms <= 0.0) {
            /* Distance is scaled in increments of 10 minutes */
            val = (xv_get(gd.extras_pu->wind_sl,PANEL_VALUE) + 1) * 10.0 ;
            gd.extras.wind[k].wind_ms = ((val* 60.0) / 1000.0)/ gd.extras.wind[k].wind_u->dx;
        }
        xscale =  (width / nx) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_u->v_scale;
        xbias = (width / nx) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_u->v_bias;
    
        yscale = (width / nx) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_v->v_scale;
        ybias = (width / nx) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_v->v_bias;
    
        zscale = (height / ny) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_w->v_scale;
        zbias = (height /ny) * gd.extras.wind[k].wind_ms * gd.extras.wind[k].wind_w->v_bias;
    
        x_jump = (nx / ideal_x_vects) + 0.5;    /* Try for approx ideal number of vectors on the screen at one time */
        if(x_jump < 1) x_jump = 1;
        y_jump = (ny / ideal_y_vects) + 0.5;
        if(y_jump < 1) y_jump = 1;

        /* loop through the array and draw the vectors */
        for(i = 0; i < ny; i++) {
            for(j=0; j < nx; j++) {
                /* If data point is missing or data grid  needs decimated */
                if((*ptr_u == miss_u) || (*ptr_v == miss_v) || (*ptr_w == miss_w) || (i%y_jump != 0) || (j%x_jump != 0)) {
                    /* Do nothing */
                } else {

                    /* compute vertex position */
                    x1 = x_start[j];
                    y1 = (y_start[i] + y_end[i]) / 2;
   
                    if( marker_type > 0) { /* Use arrows */

                        /* compute vector head position */
			 
			u_proj = cos_theta * ((xscale * *ptr_u) + xbias);
			v_proj = sin_theta * ((yscale * *ptr_v) + ybias);
			 
                        x2 = x1 + ((u_proj + v_proj) * 0.5);

                        y2 = y1 - (((zscale * *ptr_w) + zbias) * 0.5 * w_scale_factor);

                        /* compute vector tail position */
                        x3 = x1 - ((u_proj + v_proj) * 0.5);

                        y3 = y1 + (((zscale * *ptr_w) + zbias) * 0.5 * w_scale_factor);

                        XUDRarrow(gd.dpy,xid,gd.extras.wind[k].color->gc,x3,y3,x2,y2,head_size,head_angle * DEG_TO_RAD);

                    } else {        /* Use tufts */

                        XUDRcircle(x1, y1, 1, gd.dpy, xid, gd.extras.wind[k].color->gc); /*  */
                        x2 = x1 + (cos_theta * (((xscale * *ptr_u) + xbias) * 0.5)) + 
                                  (sin_theta * (((yscale * *ptr_v) + ybias) * 0.5));

                        y2 = y1 - ((zscale * *ptr_w) + zbias * w_scale_factor);
                        XDrawLine(gd.dpy,xid,gd.extras.wind[k].color->gc,x2,y2,x1,y1);
                    }
                }

                ptr_v++;
                ptr_u++;
                ptr_w++;
            }
        }

    }
    /* Add a reference line */
    speed = 5.0;
    /* Distance is scaled in increments of 10 minutes */
    val = (xv_get(gd.extras_pu->wind_sl,PANEL_VALUE) + 1) * 10.0;

    /* pixels = m/sec * minutes * 60 seconds/minute / 1km/1000meters / km_span of window * pixels wide the image is */
    dist = speed *  val * 60.0 / 1000.0 / span * gd.h_win.can_dim.width;

    x1 = gd.v_win.margin.left * 2;
    y1 = gd.v_win.margin.top * 1.55;

    x2 = x1 + dist;
    y2 = y1;
     
    XUDRarrow(gd.dpy,xid,gd.extras.wind[0].color->gc,x1,y1,x2,y2,head_size,head_angle * DEG_TO_RAD);

    sprintf(label,"%g m/sec (W * %g)",speed,w_scale_factor);
    font = choose_font(label,((x2 -x1) * 5), gd.v_win.margin.top,&xmid,&ymid);
    XSetFont(gd.dpy,gd.extras.wind[0].color->gc,font);
    if(gd.font_display_mode == 0) 
        XDrawString(gd.dpy,xid,gd.extras.wind[0].color->gc, x2 +4,y1 + ymid,label,strlen(label));
    else
        XDrawImageString(gd.dpy,xid,gd.extras.wind[0].color->gc, x2 +4,y1 + ymid,label,strlen(label));
     
    return SUCCESS;
}
