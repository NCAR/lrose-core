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
/************************************************************************
 * RENDER_VERT.C: Routines that render vertical cross section data 
 *        
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_VERT
#include "cidd.h"
#include <rapplot/rascon.h>

/**********************************************************************
 * RENDER_VERT_DISPLAY: Render the vertical cross section display
 */

render_vert_display(xid,field,start_time,end_time)
    Drawable xid;
    int    field;
    long    start_time,end_time;
{
    int x1,y1,ht,wd;    /* boundries of image area */
    int stat;

    if(xid == 0) return FAILURE;

    rd_h_msg("Rendering",-1);

    if(gd.debug2) fprintf(stderr,"Rendering Vertical Image, field :%d\n",field);
    /* Clear drawing area */
    XFillRectangle(gd.dpy,xid,gd.extras.background_color->gc,
        0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height);

    stat =  render_rhi(xid,field,start_time,end_time);

    render_vert_extras(xid,field,start_time,end_time);

    render_vert_products(xid,field,start_time,end_time);

    /* clear margin areas */
    XFillRectangle(gd.dpy,xid,gd.extras.background_color->gc,
        0,0,gd.v_win.can_dim.width,gd.v_win.margin.top);

    XFillRectangle(gd.dpy,xid,gd.extras.background_color->gc,
        0,gd.v_win.can_dim.height - gd.v_win.margin.bot,
        gd.v_win.can_dim.width,gd.v_win.margin.bot);

    XFillRectangle(gd.dpy,xid,gd.extras.background_color->gc,
        0,0,gd.v_win.margin.left,gd.v_win.can_dim.height);

    XFillRectangle(gd.dpy,xid,gd.extras.background_color->gc,
        gd.v_win.can_dim.width - gd.v_win.margin.right,
        0,gd.v_win.margin.right,gd.v_win.can_dim.height);


    /* Add a border */
    x1 = gd.v_win.margin.left -1;
    y1 = gd.v_win.margin.top -1;
    wd = gd.v_win.img_dim.width +1;
    ht = gd.v_win.img_dim.height +1;
    /* Add a border around the plot */
    XDrawRectangle(gd.dpy,xid,gd.extras.foreground_color->gc,x1,y1,wd,ht);
 
    draw_vwin_right_label(xid,field);
    draw_vwin_left_label(xid,field);
    draw_vwin_top_label(xid,field);
    draw_vwin_bot_label(xid,field);

    rd_h_msg(gd.frame_label,-1);

    return SUCCESS;
}

/**********************************************************************
 * RENDER_VERT_EXTRAS: Render selected extra features that appear on
 *        the vertical cross section display
 */

render_vert_extras(xid,field,start_time,end_time)
    Drawable xid;
    int    field;
    long    start_time,end_time;
{
    int    i,j;
    int x1,y1,x2,y2;
    int tmp;
    int startx,starty,endx,endy;
    int    x_grid[MAX_COLS],y_grid[MAX_ROWS];
    double    dist,x_km,y_km,km_ht;
    double    val;
    double delt;
    double levels[MAX_CONT_LEVELS];
    unsigned char    *ptr;
    unsigned char    bad_data;
    met_record_t *mr;       /* convienence pointer to a data record */
    GC    gc_array[MAX_CONT_LEVELS];

    /* render the CAP cross reference line */
    if (gd.mrec[field]->data_format != PPI_DATA_FORMAT)
    {
        km_ht = (gd.h_win.cmin_ht + gd.h_win.cmax_ht) / 2.0; 
        x_km = compute_range(gd.v_win.cmin_x,gd.v_win.cmin_y,gd.v_win.cmax_x,gd.v_win.cmax_y);

        km_to_pixel_v(&gd.h_win.margin,0.0,km_ht,&x1,&y1);    /* from 0.0 km */
        km_to_pixel_v(&gd.h_win.margin,x_km,km_ht,&x2,&y2);    /* to full range */
        XDrawLine(gd.dpy,xid,gd.extras.foreground_color->gc,x1,y1,x2,y2);
    } /* endif */

    /* render contours if selected */
    for(j=0; j < NUM_CONT_LAYERS; j++) {
      if(gd.extras.cont[j].active) {
        mr = gd.mrec[gd.extras.cont[j].field];
        ptr = mr->v_data;

        if(ptr != NULL) {
            /* Calculate data to Pixel mapping */
            grid_to_km(mr,mr->vx1,mr->vy1,&x_km,&y_km);
            dist = compute_range(gd.v_win.cmin_x, gd.v_win.cmin_y,x_km,y_km);
            km_to_pixel_v(&gd.v_win.margin,dist,mr->vert[0].min,&startx,&starty);
    
            grid_to_km(mr,mr->vx2,mr->vy2,&x_km,&y_km);
            dist = compute_range(gd.v_win.cmin_x, gd.v_win.cmin_y,x_km,y_km);
            km_to_pixel_v(&gd.v_win.margin,dist,mr->vert[mr->sects -1].max,&endx,&endy);
    
            if(startx > endx) {
                tmp = startx;
                startx = endx;
                endx = tmp;
            }

	    delt = (endx - startx) / (double) (mr->v_nx -1);
	    for(i=0; i < mr->v_nx; i++) {
		x_grid[i] = startx + (i * delt);
	    }

	    for(i=0; i < mr->v_ny; i++) {
		km_to_pixel_v(&gd.v_win.margin,dist,mr->vert[i].max,&endx,&endy);
		y_grid[i] = endy;
	    }
     
            gd.extras.cont[j].num_levels = 0;
            val = gd.extras.cont[j].min;
            for(i=0; (i < MAX_CONT_LEVELS ) && (val < gd.extras.cont[j].max);i++) {
                   val = gd.extras.cont[j].min + ( i * gd.extras.cont[j].interval);
                   val =  (val - mr->v_bias) / mr->v_scale;
		   if(val  > 0 && val  < 255) {
		       levels[gd.extras.cont[j].num_levels] = val;
		       gc_array[gd.extras.cont[j].num_levels] = gd.extras.cont[j].color->gc;
                       gd.extras.cont[j].num_levels++;
		    }
               }
     
	    bad_data = mr->missing_val;
	    RASCONinit(gd.dpy,xid,mr->v_nx,mr->v_ny,x_grid,y_grid);

            RASCONcontour(gc_array,ptr,&bad_data,RASCON_CHAR,
                    gd.extras.cont[j].num_levels,
                    levels,
                    RASCON_LINE_CONTOURS, RASCON_DOLABELS,
                    mr->v_scale,mr->v_bias);

        }
      }
    }
     
    /* render Winds if selected */
    if(gd.extras.wind_vectors) {
        render_vert_wind_vectors(xid);
    }

}

/**********************************************************************
 * RENDER_VERT_PRODUCTS: Render selected products that can appear on
 *        the vertical cross section display
 */

render_vert_products(xid,field,start_time,end_time)
    Drawable xid;
    int    field;
    long    start_time,end_time;
{
}

/**********************************************************************
 * RENDER_RHI: Render a vertical plane of data 
 */

render_rhi(xid,field,start_time,end_time)
    Drawable xid;
    int    field;
    long    start_time,end_time;
{
    int    i,j;
    int    ht,wd;                /* Dims of drawing canvas */
    int    startx,starty;        /* Pixel limits of data area */
    int    endx,endy;            /* Pixel limits of data area */
    int    xmid,ymid;
    int    x_start[MAX_COLS];    /* canvas rectangle begin  coords */
    int    y_start[MAX_ROWS];    /* canvas  rectangle begin coords */
    int    y_end[MAX_ROWS];    /* canvas  rectangle end coords */
    double    dist;            /* distance between corner of vert section and grid point */
    double    span;            /* distance between corners of vert section */
    double    height;
    double    x_km,y_km;
    double    r_ht,r_wd;        /*  data point rectangle dims */
    unsigned char    *ptr;
    unsigned char    miss;            /* Missing data value */
    met_record_t *mr;        /* pointer to record for convienence */
    char    message[32];
    Font    font;

    if(xid == 0) return FAILURE;
    mr = gd.mrec[field];    /* get pointer to data record */
     
    ptr =  (unsigned char *) mr->v_data;
     
    if(ptr == NULL) {    /* If no data - Draw warning message */
        sprintf(message,"Sadly, Data is Not Available :(");
        font = choose_font(message,gd.v_win.img_dim.width,gd.v_win.img_dim.height,&xmid,&ymid);
        XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
        XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
            gd.v_win.margin.left *2 ,gd.v_win.margin.top *2,
            message,strlen(message));

        fprintf(stderr, "Message with no data received from server\n");
        fprintf(stderr, "  default server: %s - %d\n",
                gd.io_info.mr->data_hostname, gd.io_info.mr->port);
        fflush(stderr);
        
        return SUCCESS;
    }

    if (mr->data_format == CART_DATA_FORMAT)
    {
        
        grid_to_km(mr,mr->vx1,mr->vy1,&x_km,&y_km);
        if(mr->vx1 > mr->vx2) {    /* adjust by half width */
            x_km += mr->dx/2.0;
        } else {
            x_km -= mr->dx/2.0;
        }
        if(mr->vy1 > mr->vy2) {    /* adjust by half width */
            y_km += mr->dy/2.0;
        } else {
            y_km -= mr->dy/2.0;
        }
        span = compute_range(gd.v_win.cmin_x, gd.v_win.cmin_y,gd.v_win.cmax_x, gd.v_win.cmax_y);
        dist = compute_range(gd.v_win.cmax_x, gd.v_win.cmax_y,x_km,y_km);
        height = mr->vert[mr->z1].min;
        km_to_pixel_v(&gd.v_win.margin,(span - dist),height,&startx,&starty);

        grid_to_km(mr,mr->vx2,mr->vy2,&x_km,&y_km);
        if(mr->vx1 > mr->vx2) {    /* adjust by half width */
            x_km -= mr->dx/2.0;
        } else {
            x_km += mr->dx/2.0;
        }
        if(mr->vy1 > mr->vy2) {    /* adjust by half width */
            y_km -= mr->dy/2.0;
        } else {
            y_km += mr->dy/2.0;
        }
        dist = compute_range(gd.v_win.cmin_x, gd.v_win.cmin_y,x_km,y_km);

        height = mr->vert[mr->z2].max;
        km_to_pixel_v(&gd.v_win.margin,dist,height,&endx,&endy);
     
        r_wd =  (double)ABSDIFF(endx,startx) / (double) mr->v_nx;        /* get data point rectangle width */

        for(i= 0; i < mr->v_ny; i++) {    /* Calc starting/ending coords for the array */
            height = mr->vert[mr->z1 + i].min;
            km_to_pixel_v(&gd.v_win.margin,dist,height,&endx,&(y_start[i]));
            height = mr->vert[mr->z1 + i].max;
            km_to_pixel_v(&gd.v_win.margin,dist,height,&endx,&(y_end[i]));
        }

        for(j=0;j< mr->v_nx; j++) {
            x_start[j] = ((double) j * r_wd) + startx;
        }
    }
    else    /* PPI_DATA_FORMAT */
    {
        grid_to_km_v(mr, mr->vx1, mr->vy1, &x_km, &y_km);
        if (mr->vx1 > mr->vx2)
            x_km += mr->vdx / 2.0;
        else
            x_km -= mr->vdx / 2.0;
        if (mr->vy1 > mr->vy2)
            y_km += mr->vdy / 2.0;
        else
            y_km -= mr->vdy / 2.0;
        
        km_to_pixel_v(&gd.v_win.margin, x_km, y_km, &startx, &starty);
        
        grid_to_km_v(mr, mr->vx2, mr->vy2, &x_km, &y_km);
        if (mr->vx1 > mr->vx2)
            x_km -= mr->vdx / 2.0;
        else
            x_km += mr->vdx / 2.0;
        if (mr->vy1 > mr->vy2)
            y_km -= mr->vdy / 2.0;
        else
            y_km += mr->vdy / 2.0;
        
        km_to_pixel_v(&gd.v_win.margin, x_km, y_km, &endx, &endy);
        

        r_wd = (double)ABSDIFF(endx, startx) / (double) mr->v_nx;
        r_ht = (double)ABSDIFF(endy, starty) / (double) mr->v_ny;
        
        for (i = 0; i < mr->v_nx; i++)
        {
            x_start[i] = startx + (long)((double)i * r_wd);
        } /* endfor - i */

        for (i = 0; i < mr->v_ny; i++)
        {
            y_start[i] = starty - (long)((double)i * r_ht);
            y_end[i] = starty - (long)((double)(i + 1) * r_ht);
        } /* endfor - i */
        
        
    }
    
     
    miss = mr->missing_val;
    wd = r_wd + 1.0;
     
    for(i= 0; i < mr->v_ny; i++) {
        ht = y_start[i] - y_end[i];
        for(j=0;j< mr->v_nx; j++) {
            if(*ptr != miss) {
                XFillRectangle(gd.dpy,xid,mr->v_vcm.val_gc[*ptr],x_start[j],y_end[i],wd,ht);
            }
            ptr++;
        }
    }
    
    return SUCCESS;
}

/**********************************************************************
 * DRAW_VWIN_RIGHT_LABEL:     Displays a labeled color scale if the
 * margin is set large enough
 */

draw_vwin_right_label(xid,field)
    Drawable xid;
    int    field;
{
    int x_start,y_start;
    int ht,wd;
    int bar_width;
    met_record_t *mr;       /* pointer to record for convienence */

    if(gd.v_win.margin.right <= 20) return;

    mr = gd.mrec[field];    /* get pointer to data record */
 
    /* calc dimensions of drawable area */
    y_start = gd.v_win.margin.top;
    x_start = gd.v_win.can_dim.width - gd.v_win.margin.right;
    ht = gd.v_win.can_dim.height - gd.v_win.margin.top - gd.v_win.margin.bot;
    wd = gd.v_win.margin.right;

    draw_colorbar(gd.dpy,xid,gd.extras.foreground_color->gc,
       x_start,y_start,wd,ht,mr->v_vcm.nentries,mr->v_vcm.vc,0,mr->field_units);
}
 
/**********************************************************************
 * DRAW_LEFT_LABEL: Draw an axis in the left margin & label it
 */

draw_vwin_left_label(xid,field)
    Drawable xid;
    int    field;
{
    int x_start,y_start;
    int tick_xstart,tick_xend;
    int tick_ystart;
    int xmid,ymid;
    int    label_space;
    double  range;
    double  min_val;
    double  tick_spacing;
    double  current_tick;
    char    label[16];
    Font    font;
 
    if(gd.v_win.margin.left == 0) return;
    /* calc dimensions of drawable area */
    x_start = 0;
    y_start =  gd.v_win.margin.top;
    tick_xstart = gd.v_win.margin.left * 0.75;
 
    range = gd.v_win.max_ht - gd.v_win.min_ht;
    tick_spacing = compute_tick_interval(range);
    label_space = gd.v_win.img_dim.height * (tick_spacing / range);
    min_val = rint(gd.v_win.min_ht);
    current_tick = min_val + abs((int)min_val %  (int)((tick_spacing > 1.0) ? tick_spacing:  1.0));
 
    while(current_tick <= gd.v_win.max_ht) {
        km_to_pixel_v(&gd.v_win.margin,(double)0.0,current_tick,&tick_xend,&tick_ystart);
        XDrawLine(gd.dpy,xid,gd.extras.foreground_color->gc,tick_xstart,tick_ystart,tick_xend-1,tick_ystart);
 
        sprintf(label,"%.0f",current_tick);
        font = choose_font(label,tick_xstart,label_space,&xmid,&ymid);
        XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
        XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
            x_start,(tick_ystart + ymid),label,strlen(label));
 
        current_tick += tick_spacing;
    };

    if (gd.mrec[field]->data_format == PPI_DATA_FORMAT)
        sprintf(label,"%s",gd.mrec[field]->vunits_label_rows);
    else
        sprintf(label,"%s",gd.mrec[field]->units_label_sects);
    font = choose_font(label,gd.v_win.margin.left,gd.v_win.margin.left,&xmid,&ymid);
    XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
    XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
        x_start +2 ,gd.v_win.can_dim.height -2,label,strlen(label));

}

/**********************************************************************
 * DRAW_VWIN_TOP_LABEL: Label the top of the horizontal image
 */

draw_vwin_top_label(xid,field)
    Drawable xid;
    int    field;
{
    int    x_start,y_start;
    int    xmid,ymid;
    int    ht,wd;
    met_record_t *mr;        /* pointer to record for convienence */
    char    label[TITLE_LENGTH];
    Font    font;

    if(gd.v_win.margin.top == 0) return;
    mr = gd.mrec[field];    /* get pointer to data record */
     
    /* calc dimensions of drawable area */
    ht = gd.v_win.margin.top;
    wd = gd.v_win.can_dim.width - gd.v_win.margin.left - gd.v_win.margin.right;
    x_start =  (wd/2) + gd.v_win.margin.left;
    y_start =  ht/2;

    sprintf(label,"%s Cross Section %d/%d  %d:%02d",
        mr->field_name,
        mr->v_date.month,
        mr->v_date.day,
        mr->v_date.hour,
        mr->v_date.min);

    font = choose_font(label,wd,ht,&xmid,&ymid);
    XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
    XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
        x_start + xmid,y_start + ymid,label,strlen(label));

}

/**********************************************************************
 * DRAW_VWIN_BOT_LABEL: Draw an axis and lable the bottom margin
 */

draw_vwin_bot_label(xid,field)
    Drawable xid;
    int    field;
{
    int tick_xstart;
    int tick_ystart,tick_yend;
    int xmid,ymid;
    int label_space;
    double  range;
    double  tick_spacing;
    double  current_tick;
    char    label[16];
    Font    font;
 
    if(gd.v_win.margin.bot == 0) return;
    /* calc dimensions of drawable area */
    tick_yend = gd.v_win.can_dim.height - (gd.v_win.margin.bot * 0.75);
 
    range = compute_range(gd.v_win.cmin_x,gd.v_win.cmin_y,gd.v_win.cmax_x,gd.v_win.cmax_y);
    tick_spacing = compute_tick_interval(range);
    current_tick = 0.0;
 
    label_space = gd.v_win.img_dim.width * (tick_spacing / range);
    while(current_tick <= range) {
        km_to_pixel_v(&gd.v_win.margin,current_tick,gd.v_win.cmin_ht,&tick_xstart,&tick_ystart);
        XDrawLine(gd.dpy,xid,gd.extras.foreground_color->gc,tick_xstart,tick_ystart,tick_xstart,tick_yend);
 
        sprintf(label,"%.0f",current_tick);
        font = choose_font(label,label_space,gd.v_win.margin.bot,&xmid,&ymid);
        XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
        XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
            (tick_xstart + xmid),gd.v_win.can_dim.height -2,label,strlen(label));         
 
        current_tick += tick_spacing;
    };

    if (gd.mrec[field]->data_format == PPI_DATA_FORMAT)
        sprintf(label,"%s",gd.mrec[field]->vunits_label_cols);
    else
        sprintf(label,"%s",gd.mrec[field]->units_label_cols);
    font = choose_font(label,label_space,gd.h_win.margin.bot,&xmid,&ymid);
    XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
    XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
        tick_xstart + label_space + xmid,gd.v_win.can_dim.height -2,label,strlen(label));

}

#ifndef LINT
static char RCS_id[] = "$Id: render_vert.c,v 1.15 2016/03/07 18:28:26 dixon Exp $";
static char SCCS_id[] = "%W% %D% %T%";
#endif /* not LINT */
