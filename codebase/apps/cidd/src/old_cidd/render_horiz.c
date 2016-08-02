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
 * RENDER_HORIZ.C: Render horizontal cross section data, extra features
 *    and labels for the RD program horizontal display. 
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 *
 * Modification History:
 *   N. Rehak   14 Aug 92   Added code to render radial images.
 */

#include <math.h>

#include <rapplot/rascon.h>

#if defined(IRIX5)
extern double rint(double);
#endif

#define RENDER_HORIZ
#include "cidd.h"

int render_horiz_display( Drawable xid, int field, long start_time, long end_time);
int render_grid( Drawable xid, int field, long start_time, long end_time, int is_overlay_field);
int render_cart_grid( Drawable xid, met_record_t *mr, long start_time, long end_time, int is_overlay_field);
int render_polar_grid( Drawable xid, met_record_t *mr, long start_time, long end_time);
void draw_filled_contours( Drawable xid, int x_start[], int y_start[], met_record_t *mr);
int draw_filled_image( Drawable xid, int x_start[], int y_start[], met_record_t *mr);
char * field_label( met_record_t *mr);
void render_horiz_margins(Drawable xid, int field, long start_time, long end_time);
int draw_hwin_interior_labels( Drawable xid, int field, long start_time, long end_time);
int draw_hwin_right_label( Drawable xid, int field, long start_time, long end_time);
int draw_hwin_left_label( Drawable xid, int field, long start_time, long end_time);
int draw_hwin_top_label( Drawable xid, int field, long start_time, long end_time);
int draw_hwin_bot_label( Drawable xid, int field, long start_time, long end_time);

/**********************************************************************
 * RENDER_HORIZ_DISPLAY: Render a complete horizontal plane of data 
 *        and its associated overlays and labels  labels. 
 */

int render_horiz_display( Drawable xid, int field, long start_time,
			 long end_time)
{
    int i;

    if(xid == 0) return FAILURE;

    PMU_auto_register("Rendering (OK)");

    /* Clear drawing area */
    XFillRectangle(gd.dpy,xid,gd.extras.background_color->gc,
         0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);

    rd_h_msg("Rendering",-1);

    if(!gd.draw_main_on_top) render_grid(xid,field,start_time,end_time,0); 
     
    /* Render each of the gridded_overlay fields */
    for(i=0; i < NUM_GRID_LAYERS; i++) {
        if(gd.extras.overlay_field_on[i]) {
            render_grid(xid,gd.extras.overlay_field[i],start_time,end_time,1);
	}
    }

    if(gd.draw_main_on_top) render_grid(xid,field,start_time,end_time,0); 

    render_horiz_extras(xid,field,start_time,end_time);

    render_horiz_margins(xid,field,start_time,end_time);

    return SUCCESS;    /* avaliable data has been rendered */
}


/**********************************************************************
 * RENDER_HORIZ_MARGINS: Render the margins for the horizontal display.
 */

void render_horiz_margins(Drawable xid, int field,
			  long start_time, long end_time)
{
  int    x1, y1, ht, wd;    /* boundries of image area */

  /* clear margin areas */

  XFillRectangle(gd.dpy, xid, gd.extras.background_color->gc,
		 0, 0,
		 gd.h_win.can_dim.width, gd.h_win.margin.top);
         
  XFillRectangle(gd.dpy, xid, gd.extras.background_color->gc,
		 0, gd.h_win.can_dim.height - gd.h_win.margin.bot,
		 gd.h_win.can_dim.width, gd.h_win.margin.bot);
         
  XFillRectangle(gd.dpy, xid, gd.extras.background_color->gc,
		 0, 0,
		 gd.h_win.margin.left, gd.h_win.can_dim.height);
         
  XFillRectangle(gd.dpy, xid, gd.extras.background_color->gc,
		 gd.h_win.can_dim.width - gd.h_win.margin.right, 0,
		 gd.h_win.margin.right, gd.h_win.can_dim.height);
         
  /* Add a border around the plot */
  x1 = gd.h_win.margin.left - 1;
  y1 = gd.h_win.margin.top - 1;
  wd = gd.h_win.img_dim.width + 1;
  ht = gd.h_win.img_dim.height + 1;
     
  XDrawRectangle(gd.dpy, xid, gd.extras.foreground_color->gc,
		 x1, y1,
		 wd, ht);

  draw_hwin_right_label(xid, field, start_time, end_time); /* the color scale */
  draw_hwin_left_label(xid, field, start_time, end_time);
  draw_hwin_top_label(xid, field, start_time, end_time);
  draw_hwin_bot_label(xid, field, start_time, end_time);

  draw_hwin_interior_labels(xid, field, start_time, end_time);

  rd_h_msg(gd.frame_label, -1);
}

#define MESSAGE_LEN   256

/**********************************************************************
 * RENDER_GRID: Render a horizontal plane of  gridded data 
 *    Returns 1 on success, 0 on failure
 */

int render_grid( Drawable xid, int field, long start_time, long end_time, int is_overlay_field)
{
    int    out_of_date;    
    int    stretch_secs;
    int    ht,wd;                /* Dims of data rectangles  */
    int    startx,endx;        /* pixel limits of data area */
    int    starty,endy;        /* pixel limits of data area */
    int     x_pix, y_pix;           /* pixel point values */
    int    xmid,ymid;
    char    message[MESSAGE_LEN];    /* Error message area */
    met_record_t *mr;        /* pointer to record for convienence */
    Font    font;

    mr = gd.mrec[field];    /* get pointer to data record */

    if(gd.debug2) fprintf(stderr,"Rendering Horiz Image - Field %d\n",field);
    out_of_date = 0;
    stretch_secs =  60.0 * mr->time_allowance;

    if(gd.check_data_times) {
      if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
      if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
    }


    /* For the Main Field - Clear the screen and  Print a special message 
     * and draw the wall clock time if the data is not availible 
     */
    if(!is_overlay_field) {


	/* Print out an informational message */
      if(gd.status.status_fname != NULL && strlen(gd.status.stat_msg) > 2) {

          font = choose_font(gd.status.stat_msg, gd.h_win.img_dim.width,
	      gd.h_win.img_dim.height/4, &xmid, &ymid);
          XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
          XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
            gd.h_win.margin.left + (gd.h_win.img_dim.width /2) + xmid  ,
            gd.h_win.margin.top + (gd.h_win.img_dim.height /2) + ymid ,
	    gd.status.stat_msg,strlen(gd.status.stat_msg));
      }
      /* If no data in current response or data is way out of date */
      if( mr->h_data == NULL || out_of_date ) {

        /* display "Data Not Available" message */
        if(out_of_date) {
            STRcopy(message, gd.no_data_message,MESSAGE_LEN);
        } else {
            STRcopy(message, gd.no_data_message,MESSAGE_LEN);
            /* STRcopy(message, "DATA NOT ACCESSIBLE",MESSAGE_LEN); /*  */
        }

        font = choose_font(message, gd.h_win.img_dim.width, gd.h_win.img_dim.height, &xmid, &ymid);
        XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
        XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
            gd.h_win.margin.left + (gd.h_win.img_dim.width /2) + xmid  ,
            gd.h_win.margin.top + (gd.h_win.img_dim.height /4) + ymid ,
	    message,strlen(message));
        

        if(gd.debug2) {
          fprintf(stderr, "No data from  server: %s - %d\n",
	                  gd.io_info.mr->data_hostname, gd.io_info.mr->port);
        }
        
	if(gd.show_clock) {
            /* draw a clock */
            ht = gd.h_win.can_dim.height * 0.05;
            startx = gd.h_win.can_dim.width - gd.h_win.margin.right - ht - 5;
            starty = gd.h_win.margin.top + ht + 5;
            XUDRdraw_clock(gd.dpy,xid,gd.extras.foreground_color->gc,
                 startx,starty,ht, ((start_time + end_time) /2),1);
	}
        
        return FAILURE;
      }
    }
     
    set_busy_state(1);

    switch(mr->data_format) {
      case  CART_DATA_FORMAT:
           render_cart_grid( xid, mr,
	                      start_time, end_time,
			      is_overlay_field);
      break;
      
      case  PPI_DATA_FORMAT:
           render_polar_grid( xid, mr, start_time, end_time);
      break;
      
    }
   set_busy_state(0);
   return SUCCESS;
}

/**********************************************************************
 * RENDER_CART_GRID: Render a horizontal plane of  gridded data 
 *    Returns 1 on success, 0 on failure
 */

int render_cart_grid( Drawable xid, met_record_t *mr, long start_time, long end_time, int is_overlay_field)
{
    int    i,j,k;
    int    out_of_date;    
    int    stretch_secs;
    int    ht,wd;               /* Dims of data rectangles  */
    int    startx,endx;        /* pixel limits of data area */
    int    starty,endy;        /* pixel limits of data area */
    int     x_pix, y_pix;     /* pixel point values */
    int    xmid,ymid;
    double    x_km,y_km;
    double val;
    int    x_start[MAX_COLS + 4];    /* canvas rectangle begin  coords */
    int    y_start[MAX_ROWS + 4];    /* canvas  rectangle begin coords */
    double    r_ht,r_wd;        /*  data point rectangle dims */
    unsigned char    *ptr,*ptr1;
    unsigned char    miss;            /* Missing data value */
    char    message[256];    /* Error message area */
    Font    font;

    ptr = (unsigned char *) mr->h_data;
    if(ptr == NULL) return FAILURE;
     
    /* Calculate Data to Screen Mapping */
    grid_to_km(mr,mr->x1,mr->y1,&x_km,&y_km);
    x_km -= mr->dx/2.0;    /* expand grid coord by half width */
    if(mr->order) {
        y_km -= mr->dy/2.0;
    } else {
        y_km += mr->dy/2.0;
    }
    km_to_pixel(&gd.h_win.margin,x_km,y_km,&startx,&starty);

    grid_to_km(mr,mr->x2,mr->y2,&x_km,&y_km);
    x_km += mr->dx/2.0;    /* expand grid coord by half width */
    if(mr->order) {
        y_km += mr->dy/2.0;
    } else {
        y_km -= mr->dy/2.0;
    }
    km_to_pixel(&gd.h_win.margin,x_km,y_km,&endx,&endy);

    /* get data point rectangle size */
    r_ht =  (double)(ABSDIFF(starty,endy))  / (double) mr->h_ny;
    r_wd =  (double)(endx - startx)/ (double) mr->h_nx;    

    /* Calc starting coords for the X,Y array */
    for(j=0;j <= mr->h_nx; j++) {
        x_start[j] = ((double) j * r_wd) + startx;
        if(x_start[j] < 0) x_start[j] = 0;
        if(x_start[j] > gd.h_win.can_dim.width) x_start[j] = gd.h_win.can_dim.width;
    }

    for(i= 0; i <= mr->h_ny; i++) {
        if(mr->order) {
            y_start[i] = starty - ((double) i * r_ht);
        } else {
            y_start[i] = starty + ((double) i * r_ht);
        }
        if(y_start[i] < 0) y_start[i] = 0;
        if(y_start[i] >= gd.h_win.can_dim.height) y_start[i] = gd.h_win.can_dim.height -1;
    }

    if(mr->num_display_pts <=0) mr->num_display_pts = mr->h_ny * mr->h_nx;

    miss = mr->missing_val;
    ht = r_ht + 1.0; 
    wd = r_wd + 1.0;

    if(is_overlay_field) {
        if(gd.debug2) printf("Drawing Rectangle Fill Overlay image field: %s\n",
			    mr->field_label);
        if(mr->order) {
	    for(i= 0; i < mr->h_ny; i++) y_start[i]-= (ht -1);
	}

        mr->num_display_pts = 0;
        for(i= 0; i < mr->h_ny; i++) {
            for(j=0;j< mr->h_nx; j++) {
                if(*ptr != miss) {
                    val =  (mr->h_scale * *ptr) + mr->h_bias;
                    if(val >= mr->overlay_min && val <= mr->overlay_max) 
                        XFillRectangle(gd.dpy,xid,mr->h_vcm.val_gc[*ptr],x_start[j],y_start[i],wd,ht);
                    mr->num_display_pts++;
                }
                ptr++;
            }
        }

    } else {
	 
       switch (mr->render_method) {

	    case RECTANGLES:
              if( gd.inten_levels > 1 && !gd.draw_main_on_top && mr->num_display_pts > gd.image_fill_treshold) {
                draw_filled_image(xid,x_start,y_start,mr);
	      } else {
                if(gd.debug2) printf("Drawing Rectangle Fill image, field: %s \n",
		                        mr->field_label);
                mr->num_display_pts = 0;
                if(mr->order) {
		    for(i= 0; i < mr->h_ny; i++) y_start[i]-= (ht -1);
		}
                for(i= 0; i < mr->h_ny; i++) {
                  for(j=0;j< mr->h_nx; j++) {
                    if(*ptr != miss) {
                        XFillRectangle(gd.dpy,xid,mr->h_vcm.val_gc[*ptr],x_start[j],y_start[i],wd,ht);
                        mr->num_display_pts++;
                    }
                    ptr++;
                  }
                }
              if(gd.debug2) printf("NUM RECTANGLES: %d of %d \n",
				     mr->num_display_pts,mr->h_nx*mr->h_ny);
	      }
	    break;

	    case FILLED_CONTOURS:
              for(j=0;j <= mr->h_nx; j++) {
                  x_start[j] =  x_start[j] + (r_wd / 2);
                  if(x_start[j] > gd.h_win.can_dim.width) x_start[j] = gd.h_win.can_dim.width;
              }

              for(i= 0; i <= mr->h_ny; i++) {    /* Calc starting coords for the array */
                  if(mr->order) {
                       y_start[i] =  y_start[i] + (r_ht / 2.0);
                  } else {
                       y_start[i] =  y_start[i] - (r_ht / 2.0);
                  }
                      if(y_start[i] < 0) y_start[i] = 0;
                      if(y_start[i] >= gd.h_win.can_dim.height) y_start[i] = gd.h_win.can_dim.height -1;
              }
              if(gd.debug2) printf("Drawing Filled Contour image: field %s\n",
			       mr->field_label);
              draw_filled_contours(xid,x_start,y_start,mr);
	    break;
	  }
       }
        
    return SUCCESS;
}


/**********************************************************************
 * RENDER_POLAR_GRID: Render a horizontal plane of  gridded data 
 *    Returns 1 on success, 0 on failure
 */

int render_polar_grid( Drawable xid, met_record_t *mr, long start_time, long end_time)
{
    int    i,j,k;
    int    x_pix, y_pix;           /* pixel point values */
    unsigned char    *ptr;
    unsigned char    miss;            /* Missing data value */
    char    message[256];    /* Error message area */
    
    Font    font;

    int     beam, prev_beam;
    geo_coord_t real_beam;          /* point on beam in grid coordinates */
    XPoint  pixel_beam[2][MAX_COLS+1];
                                    /* points on beams surrounding azimuth */
    int     nxpixels, nypixels;
    int     nxpixels_per_gate, nypixels_per_gate;
    int     xinterval_count, yinterval_count;
    double  xpixel_interval, ypixel_interval;
    double  xpixel_next, ypixel_next;
    XPoint  rad_trap[4];
                                    /* pixel radial trapezoid vertices */
    double  az;                     /* current azimuth angle */
    double  gate;                   /* current gate value */
    double  theta;                  /* angle for trapezoid border */
    double  sin_theta, cos_theta;
    double  delta_x, delta_y;
    double  min_x, min_y;
    double  delta_r;
    double val;

     
    ptr = (unsigned char *) mr->h_data;
    if(ptr == NULL) return FAILURE;

    set_busy_state(1);

/*        FILE *trapfile, *valfile;
/*        
/*        if ((trapfile = fopen("trapezoids.out", "w")) == NULL)
/*            printf("Error opening trapezoid output file\n");
/*
/*        if ((valfile = fopen("values.out", "w")) == NULL)
/*            printf("error opening value output file\n");
*/

        /* save the "missing data" value */
        miss = mr->missing_val;
        mr->num_display_pts = 0;

        /*
         * calculate the radial trapezoids
         */

        /* initialize the first beam used to render trapezoids */
        az = mr->min_y + (((double)mr->y1 - 0.5) * mr->dy);
        theta = (az - mr->dy/2.0) * DEG_TO_RAD;
        sin_theta = sin(theta);
        cos_theta = cos(theta);
        delta_r = mr->dx * cos(mr->elev[mr->plane] * (double)DEG_TO_RAD);
        delta_x = delta_r * sin_theta;
        delta_y = delta_r * cos_theta;
        rad_grid_to_km(mr, (double)mr->x1, (double)mr->x1, &min_x, &min_y);
        min_x = (min_x - mr->dx) * sin_theta;
        min_y = (min_y - mr->dx) * cos_theta;
        
        real_beam.x = min_x;
        real_beam.y = min_y;

        km_to_pixel(&gd.h_win.margin, real_beam.x, real_beam.y, &x_pix, &y_pix);
        
        pixel_beam[0][0].x = x_pix;
        pixel_beam[0][0].y = y_pix;

        real_beam.x = min_x + mr->h_nx * delta_x;
        real_beam.y = min_y + mr->h_nx * delta_y;

        km_to_pixel(&gd.h_win.margin, real_beam.x, real_beam.y, &x_pix, &y_pix);
        
        pixel_beam[0][mr->h_nx].x = x_pix;
        pixel_beam[0][mr->h_nx].y = y_pix;

        nxpixels = ABSDIFF(pixel_beam[0][mr->h_nx].x, pixel_beam[0][0].x);
        nypixels = ABSDIFF(pixel_beam[0][mr->h_nx].y, pixel_beam[0][0].y);
        
        nxpixels_per_gate = nxpixels / mr->h_nx;
        nypixels_per_gate = nypixels / mr->h_nx;

        if ( nxpixels == 0%mr->h_nx )
           xpixel_interval = mr->h_nx;
        else
           xpixel_interval = (double)mr->h_nx /
                             (double)(nxpixels%mr->h_nx);
        if ( nypixels == 0%mr->h_nx )
           ypixel_interval = mr->h_nx;
        else
           ypixel_interval = (double)mr->h_nx /
                             (double)(nypixels%mr->h_nx);

        xinterval_count = 0;
        yinterval_count = 0;
        xpixel_next = xpixel_interval;
        ypixel_next = ypixel_interval;

        if (theta >= 0.0 && theta < RADIAN90)
        {
            for (j = 1; j <= mr->h_nx; j++)
            {
                pixel_beam[0][j].x = pixel_beam[0][j-1].x + nxpixels_per_gate;
                xinterval_count++;
                if (xinterval_count >= (int)(xpixel_next + 0.5))
                {
                    pixel_beam[0][j].x++;
                    xpixel_next += xpixel_interval;
                } /* endif */

                pixel_beam[0][j].y = pixel_beam[0][j-1].y - nypixels_per_gate;
                yinterval_count++;
                if (yinterval_count >= (int)ypixel_next)
                {
                    pixel_beam[0][j].y--;
                    ypixel_next += ypixel_interval;
                } /* endif */
            
            } /* endfor - j */
        }  /* endif - x > 0, y > 0 quadrant */

        else if (theta >= RADIAN90 && theta < RADIAN180)
        {
            for (j = 1; j <= mr->h_nx; j++)
            {
                pixel_beam[0][j].x = pixel_beam[0][j-1].x + nxpixels_per_gate;
                xinterval_count++;
                if (xinterval_count >= (int)(xpixel_next + 0.5))
                {
                    pixel_beam[0][j].x++;
                    xpixel_next += xpixel_interval;
                } /* endif */

                pixel_beam[0][j].y = pixel_beam[0][j-1].y + nypixels_per_gate;
                yinterval_count++;
                if (yinterval_count >= (int)ypixel_next)
                {
                    pixel_beam[0][j].y++;
                    ypixel_next += ypixel_interval;
                } /* endif */
            } /* endfor - j */
        }  /* endif - x > 0, y < 0 quadrant */

        else if (theta >= RADIAN180 && theta < RADIAN270)
        {
            for (j = 1; j <= mr->h_nx; j++)
            {
                pixel_beam[0][j].x = pixel_beam[0][j-1].x - nxpixels_per_gate;
                xinterval_count++;
                if (xinterval_count >= (int)(xpixel_next + 0.5))
                {
                    pixel_beam[0][j].x--;
                    xpixel_next += xpixel_interval;
                } /* endif */

                pixel_beam[0][j].y = pixel_beam[0][j-1].y + nypixels_per_gate;
                yinterval_count++;
                if (yinterval_count >= (int)ypixel_next)
                {
                    pixel_beam[0][j].y++;
                    ypixel_next += ypixel_interval;
                } /* endif */
            } /* endfor - j */
        }  /* endif - x < 0, y < 0 quadrant */

        else  /* theta >= RADIAN270 */
        {
            for (j = 1; j <= mr->h_nx; j++)
            {
                pixel_beam[0][j].x = pixel_beam[0][j-1].x - nxpixels_per_gate;
                xinterval_count++;
                if (xinterval_count >= (int)(xpixel_next + 0.5))
                {
                    pixel_beam[0][j].x--;
                    xpixel_next += xpixel_interval;
                } /* endif */

                pixel_beam[0][j].y = pixel_beam[0][j-1].y - nypixels_per_gate;
                yinterval_count++;
                if (yinterval_count >= (int)ypixel_next)
                {
                    pixel_beam[0][j].y--;
                    ypixel_next += ypixel_interval;
                } /* endif */
            } /* endfor - j */
        }  /* endif - x < 0, y > 0 quadrant */
        
        
        
        /* calculate the rest of the beams used to render trapezoids */
        for (i = 1; i <= mr->h_ny; i++)
        {
            beam = i%2;
            prev_beam = (i+1)%2;
            
            az = mr->min_y +  ((((double)mr->y1 -0.5) + i) * mr->dy);
            theta = (az + mr->dy/2.0) * DEG_TO_RAD;
            sin_theta = sin(theta);
            cos_theta = cos(theta);
            delta_r = mr->dx * cos(mr->elev[mr->plane] * (double)DEG_TO_RAD);
            delta_x = delta_r * sin_theta;
            delta_y = delta_r * cos_theta;
            rad_grid_to_km(mr,
                           (double)mr->x1, (double)mr->x1,
                           &min_x, &min_y);
            min_x = (min_x - mr->dx) * sin_theta;
            min_y = (min_y - mr->dx) * cos_theta;

            real_beam.x = min_x;
            real_beam.y = min_y;

            km_to_pixel(&gd.h_win.margin, real_beam.x, real_beam.y, &x_pix, &y_pix);
            
            pixel_beam[beam][0].x = x_pix;
            pixel_beam[beam][0].y = y_pix;
            
            real_beam.x = min_x + mr->h_nx * delta_x;
            real_beam.y = min_y + mr->h_nx * delta_y;

            km_to_pixel(&gd.h_win.margin, real_beam.x, real_beam.y, &x_pix, &y_pix);
            
            pixel_beam[beam][mr->h_nx].x = x_pix;
            pixel_beam[beam][mr->h_nx].y = y_pix;

            nxpixels = ABSDIFF(pixel_beam[beam][mr->h_nx].x, pixel_beam[beam][0].x);
            nypixels = ABSDIFF(pixel_beam[beam][mr->h_nx].y, pixel_beam[beam][0].y);
            
            nxpixels_per_gate = nxpixels / mr->h_nx;
            nypixels_per_gate = nypixels / mr->h_nx;
            
            if ( nxpixels%mr->h_nx == 0 )
               xpixel_interval = mr->h_nx;
            else
               xpixel_interval = (double)mr->h_nx /
                                 (double)(nxpixels%mr->h_nx);
            if ( nypixels%mr->h_nx == 0 )
               ypixel_interval = mr->h_nx;
            else
               ypixel_interval = (double)mr->h_nx /
                                 (double)(nypixels%mr->h_nx);

            xinterval_count = 0;
            yinterval_count = 0;
            xpixel_next = xpixel_interval;
            ypixel_next = ypixel_interval;

            /* fill the pixel beam incrementally */
            /* the quadrant determines whether to increment or decrement the */
            /* X,Y values for the pixels */
            if (theta >= 0.0 && theta < RADIAN90)
            {
                for (j = 1; j <= mr->h_nx; j++)
                {
                    pixel_beam[beam][j].x = pixel_beam[beam][j-1].x +
                        nxpixels_per_gate;
                    xinterval_count++;
                    if (xinterval_count >= (int)(xpixel_next + 0.5))
                    {
                        pixel_beam[beam][j].x++;
                        xpixel_next += xpixel_interval;
                    } /* endif - extra pixel interval */
                
                    pixel_beam[beam][j].y = pixel_beam[beam][j-1].y -
                        nypixels_per_gate;
                    yinterval_count++;
                    if (yinterval_count >= (int)ypixel_next)
                    {
                        pixel_beam[beam][j].y--;
                        ypixel_next += ypixel_interval;
                    } /* endif - extra pixel interval */
                } /* endfor - j */
            }  /* endif - x > 0, y > 0 quadrant */

            else if (theta >= RADIAN90 && theta < RADIAN180)
            {
                for (j = 1; j <= mr->h_nx; j++)
                {
                    pixel_beam[beam][j].x = pixel_beam[beam][j-1].x +
                        nxpixels_per_gate;
                    xinterval_count++;
                    if (xinterval_count >= (int)(xpixel_next + 0.5))
                    {
                        pixel_beam[beam][j].x++;
                        xpixel_next += xpixel_interval;
                    } /* endif - extra pixel interval */
                
                    pixel_beam[beam][j].y = pixel_beam[beam][j-1].y +
                        nypixels_per_gate;
                    yinterval_count++;
                    if (yinterval_count >= (int)ypixel_next)
                    {
                        pixel_beam[beam][j].y++;
                        ypixel_next += ypixel_interval;
                    } /* endif - extra pixel interval*/
                } /* endfor - j */
            }  /* endif - x > 0, y < 0 quadrant */

            else if (theta >= RADIAN180 && theta < RADIAN270)
            {
                for (j = 1; j <= mr->h_nx; j++)
                {
                    pixel_beam[beam][j].x = pixel_beam[beam][j-1].x -
                        nxpixels_per_gate;
                    xinterval_count++;
                    if (xinterval_count >= (int)(xpixel_next + 0.5))
                    {
                        pixel_beam[beam][j].x--;
                        xpixel_next += xpixel_interval;
                    } /* endif - extra pixel interval */
                
                    pixel_beam[beam][j].y = pixel_beam[beam][j-1].y +
                        nypixels_per_gate;
                    yinterval_count++;
                    if (yinterval_count >= (int)ypixel_next)
                    {
                        pixel_beam[beam][j].y++;
                        ypixel_next += ypixel_interval;
                    } /* endif - extra pixel interval*/
                } /* endfor - j */
            }  /* endif - x < 0, y < 0 quadrant */

            else  /* theta >= RADIAN270 */
            {
                for (j = 1; j <= mr->h_nx; j++)
                {
                    pixel_beam[beam][j].x = pixel_beam[beam][j-1].x -
                        nxpixels_per_gate;
                    xinterval_count++;
                    if (xinterval_count >= (int)(xpixel_next + 0.5))
                    {
                        pixel_beam[beam][j].x--;
                        xpixel_next += xpixel_interval;
                    } /* endif - extra pixel interval */
                
                    pixel_beam[beam][j].y = pixel_beam[beam][j-1].y -
                        nypixels_per_gate;
                    yinterval_count++;
                    if (yinterval_count >= (int)ypixel_next)
                    {
                        pixel_beam[beam][j].y--;
                        ypixel_next += ypixel_interval;
                    } /* endif - extra pixel interval */
                } /* endfor - j */
            }  /* endif - x < 0, y > 0 quadrant */
        
            /* render the trapezoids using the pixels calculated for this beam */
            /* and for the previous beam */
            for (j = 0; j < mr->h_nx; j++)
            {
                rad_trap[0].x = pixel_beam[beam][j].x;
                rad_trap[0].y = pixel_beam[beam][j].y;

                rad_trap[1].x = pixel_beam[beam][j+1].x;
                rad_trap[1].y = pixel_beam[beam][j+1].y;

                rad_trap[2].x = pixel_beam[prev_beam][j+1].x;
                rad_trap[2].y = pixel_beam[prev_beam][j+1].y;

                rad_trap[3].x = pixel_beam[prev_beam][j].x;
                rad_trap[3].y = pixel_beam[prev_beam][j].y;

/*                fprintf(stderr,
/*                        "Output files are %d beams by %d azimuths\n",
/*                        mr->h_nx, mr->h_ny);
/*                
/*                fprintf(trapfile, "%d,%d\t%d,%d\t%d,%d\t%d,%d\n",
/*                        rad_trap[0].x, rad_trap[0].y,
/*                        rad_trap[1].x, rad_trap[1].y,
/*                        rad_trap[2].x, rad_trap[2].y,
/*                        rad_trap[3].x, rad_trap[3].y);
/*
/*                fprintf(valfile, "%d,", *ptr);
*/
                    
                /* render the radial image */
                if (*ptr != miss)
                {
                    XFillPolygon(gd.dpy, xid, mr->h_vcm.val_gc[*ptr],
                                 rad_trap, 4, Convex, CoordModeOrigin);
                    mr->num_display_pts++;
                } /* endif */
                ptr++;
            } /* endfor - j */
            
        } /* endfor - i */

/*        fclose(trapfile);
/*        fprintf(valfile, "\n");
/*        fclose(valfile);
*/


    set_busy_state(0);
    return SUCCESS;
}

/**********************************************************************
 * DRAW_FILLED_CONTOURS: Render an image using filled contours 
 *
 */

void draw_filled_contours( Drawable xid, int x_start[], int y_start[], met_record_t *mr)
{
   int i,i_val;
   int    xvals,yvals;    /* number of grid points in each direction */
   GC    gc_array[MAX_COLORS];
   double levels[MAX_COLORS];
   char bad_data;

    if(mr->h_data == NULL) return;

   for(i=0; i < mr->h_vcm.nentries; i++) {
       gc_array[i] =  mr->h_vcm.vc[i]->gc;
       i_val = (mr->h_vcm.vc[i]->min - mr->h_bias) / mr->h_scale;

       if(i_val < 0 ) i_val = 0;
       if(i_val > 255 ) i_val = 255;

       levels[i] = i_val;
   }

   /* Max out the top contour level to avoid "holes" in the graphics */
   levels[mr->h_vcm.nentries] = 255;
   gc_array[mr->h_vcm.nentries] = mr->h_vcm.vc[mr->h_vcm.nentries-1]->gc;

   bad_data = mr->missing_val;

   RASCONinit(gd.dpy,xid,mr->h_nx,mr->h_ny,x_start,y_start);
   RASCONcontour(gc_array,mr->h_data,&bad_data,RASCON_CHAR,
      mr->h_vcm.nentries +1 ,
      levels,
      RASCON_FILLED_CONTOURS, RASCON_NOLABELS,
      mr->h_scale,mr->h_bias);
}

/**********************************************************************
 * DRAW_FILLED_IMAGE: Render an image using pixel fills
 *
 */

int draw_filled_image( Drawable xid, int x_start[], int y_start[], met_record_t *mr)
{
    unsigned char    miss;            /* missing/bad/unmeasured data value */
    unsigned char   *im_ptr,*data_ptr;    /* pointers to the image & data arrays */
    unsigned char    *last_row_ptr;
    unsigned char    *im_data;
    int    yi_index;            /* row index for the image */
    int    flag;
    int    x,y;
    int    len,width;
    XImage *img;
    Visual  *visual;

    /********* Retrieve the Horizontal Display Image *********/
    visual = DefaultVisual(gd.dpy,0);

    if(gd.debug2) printf("Drawing Pixel- filled image, field: %s \n",
		      mr->field_label);
     
    /* get memory areas for header and data */
    im_data = (unsigned char *)  ucalloc(1,gd.h_win.can_dim.width * gd.h_win.can_dim.height);
    if(im_data == NULL) return 0;
    img = XCreateImage(gd.dpy,visual,8,ZPixmap,0,(char *)im_data,
            gd.h_win.can_dim.width,gd.h_win.can_dim.height,8,0);
    
    mr->num_display_pts = 0;
    width = gd.h_win.can_dim.width;
    data_ptr = (unsigned char *) mr->h_data;
    if(data_ptr == NULL) return FAILURE;
    miss = mr->missing_val;

    /**** Fill in blank rows before data area ****/
    if(mr->order) {    
        /* bottom left orientation */
        im_ptr = im_data + (width * (y_start[0] +1));
        len = ((gd.h_win.can_dim.height - y_start[0]) -1) * width;
        if(len > 0) memset((char *)im_ptr,gd.extras.background_color->pixval,len);
        im_ptr -= width;
    } else {    
        /* top left orientation */
        im_ptr = im_data;
        len = (y_start[0] -1) * width;
        if(len > 0) {
            memset((char *)im_ptr,gd.extras.background_color->pixval,len);
            im_ptr += len;
        }
    }

    yi_index = y_start[0];
    
    /*** Fill in data area rows  ***/
    for(y=1; y <= mr->h_ny; y++) {
        /* Determine if this data row needs rendering */
        flag = 0;
        if(mr->order) {
            if(yi_index >= y_start[y])  flag = 1;
        } else {
            if(yi_index <= y_start[y])  flag = 1;
        }
         
        if(flag) {    /* this data row needs rendering */
            last_row_ptr = im_ptr;

            /**** Fill in blank area to the left of the data area */
            len = x_start[0];
            if(len > 0) {
                memset((char *)im_ptr,gd.extras.background_color->pixval,len);
                im_ptr += len;
            }
    
            /**** Fill in one pixel row of data values */
            for(x = 0; x < mr->h_nx; x++) {
                len = x_start[x+1] - x_start[x];
                    if(*data_ptr != miss) {
                        memset((char *)im_ptr,mr->h_vcm.val_pix[*data_ptr],len+1);
                        mr->num_display_pts++;
                    } else {
                        memset((char *)im_ptr,gd.extras.background_color->pixval,len);
                    }
                    im_ptr += len;
                data_ptr++;
            }
    
            /**** fill in blank area to the right of the data area */
            len = width  - x_start[mr->h_nx];  
            if(len > 0) {   
                 memset((char *)im_ptr,gd.extras.background_color->pixval,len);
                im_ptr += len;
            }
    
            /**** copy the pixel row to fill out the data row */
            if(mr->order) {
                yi_index--;
                im_ptr-= (2 * width); /* set image pointer back to previous row */
                while(yi_index > y_start[y]) {
                    memcpy((char *)im_ptr,(char *)last_row_ptr,width);
                    im_ptr-= width;
                    yi_index--;
                }
            }else {
                yi_index++;
                while(yi_index < y_start[y]) {
                    memcpy((char *)im_ptr,(char *)last_row_ptr,width);
                    im_ptr+= width;
                    yi_index++;
                }
            }
    
        } else {    /* skip this row of data */
            data_ptr += mr->h_nx;
        }
    }
    
    /**** Fill in blank rows after data area ****/
    if(mr->order) {
        len =  (y_start[mr->h_ny] +1)  * width;
        if(len > 0) {
            memset((char *)im_data,gd.extras.background_color->pixval,len);
        }
    } else {
        len = (gd.h_win.can_dim.height - y_start[mr->h_ny]) * width;
        if(len > 0) {
            memset((char *)im_ptr,gd.extras.background_color->pixval,len);
        }
    }

    if(gd.debug2) printf("NUM PIXEL FILLED PTS: %d of %d\n",
			   mr->num_display_pts,mr->h_nx*mr->h_ny);
    
    /**** Now copy to the canvas ****/
    XPutImage(gd.dpy,xid,gd.def_gc,img,0,0,0,0,
        gd.h_win.can_dim.width,gd.h_win.can_dim.height);

    XDestroyImage(img);    /* Deallocates both the structure and the image data */
    return SUCCESS;
}

/**********************************************************************
 * DRAW_HWIN_RIGHT_LABEL: Displays a labeled color scale in the right 
 *        margin if it is set large enough
 */

int draw_hwin_right_label( Drawable xid, int    field, long start_time, long end_time)
{
    int i;
    int x_start,y_start;
    int have_overlay_fields;
    int    ht,wd;
    met_record_t *mr;        /* pointer to record for convienence */

    if(gd.h_win.margin.right <= 20) return 0;

    mr = gd.mrec[field];    /* get pointer to data record */

    have_overlay_fields = 0;
    for(i=0; i < NUM_GRID_LAYERS; i++) {
	if(gd.extras.overlay_field_on[i]) have_overlay_fields = 1;
    }
     
    /* split into halfs - & Layer 0 on top, Highest layer on Bottom */
    if(have_overlay_fields) { /* split into halfs - & Draw Two scales */

        /* calc dimensions of drawable area */
        x_start = gd.h_win.can_dim.width - gd.h_win.margin.right;
        y_start =  gd.h_win.margin.top;

        /* Use half the left margin minus 1/2 a top margin width */
        ht = (gd.h_win.can_dim.height - (gd.h_win.margin.top * 2) - gd.h_win.margin.bot) / 2;
        wd = gd.h_win.margin.right;
    
        draw_colorbar(gd.dpy,xid,gd.extras.foreground_color->gc,x_start,y_start,wd,ht,
	    mr->h_vcm.nentries,mr->h_vcm.vc,0,mr->field_units);

        /* separate the colorscales by a top margin width */
        y_start =  y_start + ht + gd.h_win.margin.top;
    
	for(i=0; i < NUM_GRID_LAYERS; i++) {
	  if(gd.extras.overlay_field_on[i]) {
            mr = gd.mrec[gd.extras.overlay_field[i]];    /* get pointer to data record */
            draw_colorbar(gd.dpy,xid,gd.extras.foreground_color->gc,x_start,y_start,wd,ht,
		mr->h_vcm.nentries,mr->h_vcm.vc,0,mr->field_units);
	  }
	}

    } else {  /* Draw the primary colorscale only */
        mr = gd.mrec[field];    /* get pointer to data record */
        /* calc dimensions of drawable area */
        x_start = gd.h_win.can_dim.width - gd.h_win.margin.right;
        y_start =  gd.h_win.margin.top;
        ht = gd.h_win.can_dim.height - gd.h_win.margin.top - gd.h_win.margin.bot;
        wd = gd.h_win.margin.right;
    
        draw_colorbar(gd.dpy,xid,gd.extras.foreground_color->gc,
		     x_start,y_start,wd,ht,mr->h_vcm.nentries,mr->h_vcm.vc,0,mr->field_units);
    }

    return 1;
}

/**********************************************************************
 * DRAW_LEFT_LABEL: Draw an axis in the left margin & label it
 */

int draw_hwin_left_label( Drawable xid, int    field, long start_time, long end_time)
{
    int    x_start;
    int    tick_xstart,tick_xend;
    int    tick_ystart;
    int    xmid,ymid;
    double    range;
    double    min_val;
    double    tick_spacing;
    double    current_tick;
    char    label[16];
    Font    font;
     
    static double unit_per_km = 0.0;
    static char *u_label;

    if(unit_per_km == 0.0) {
    unit_per_km = XRSgetDouble(gd.cidd_db, "cidd.units_per_km", 1.0);
    u_label = XRSgetString(gd.cidd_db, "cidd.scale_units_label", "km");
    }

    if(gd.h_win.margin.left == 0) return 0;
    /* calc dimensions of drawable area */
    x_start = 0;
    tick_xstart = gd.h_win.margin.left * 0.75;

    range = gd.h_win.cmax_y - gd.h_win.cmin_y;    
    range *= unit_per_km;

    tick_spacing = compute_tick_interval(range);
    min_val = rint(gd.h_win.cmin_y * unit_per_km);
    current_tick = min_val + abs((int)min_val %  (int)((tick_spacing > 1.0) ? tick_spacing:  1.0));

    while(current_tick < gd.h_win.cmax_y * unit_per_km) {
        km_to_pixel(&gd.h_win.margin,gd.h_win.cmin_x ,(current_tick / unit_per_km) ,&tick_xend,&tick_ystart);
        XDrawLine(gd.dpy,xid,gd.extras.foreground_color->gc,tick_xstart,tick_ystart,tick_xend-1,tick_ystart);

        sprintf(label,"%.0f",current_tick);
        font = choose_font(label,tick_xstart,gd.h_win.margin.left,&xmid,&ymid);
        XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
        XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
            x_start,(tick_ystart + ymid),label,strlen(label));

        current_tick += tick_spacing;
    };

    sprintf(label,"%s",u_label);
    font = choose_font(label,tick_xstart,gd.h_win.margin.left,&xmid,&ymid);
    XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
    XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc, x_start,(4*ymid),label,strlen(label));

    return 1;
}

/**********************************************************************
 * FIELD_LABEL: Return a Label string for a field 
 */

char * field_label( met_record_t *mr)
{
    time_t  now;
    struct tm tms;
    char   tlabel[256];
    static char   label[256];

    /* Fill the  tm struct */
    ZERO_STRUCT(&tms);
    tms.tm_sec = mr->h_date.sec;
    tms.tm_min = mr->h_date.min;
    tms.tm_hour = mr->h_date.hour;
    tms.tm_mday = mr->h_date.day;
    tms.tm_mon = mr->h_date.month -1;
    tms.tm_year = mr->h_date.year - 1900;
    tms.tm_wday = 0;
    tms.tm_yday = 0;
    tms.tm_isdst = -1;

    strftime(tlabel,256,gd.label_time_format,&tms); /* Convert to a string */

    now = time(NULL);
    label[0] = '\0';
     
    if(mr->composite_mode == FALSE) {
      if(!gd.limited_mode) {
           sprintf(label,"%s ELEV: %g %s %s GMT",
                mr->field_name,
                mr->vert[mr->plane].cent,
                mr->units_label_sects,
                tlabel);
      } else {
           sprintf(label,"%s %s GMT",
                mr->field_name,
                tlabel);
      }
    } else {
        sprintf(label,"%s Composite %s GMT",
                mr->field_name,
                tlabel);
    }

    /* If data is stamped more than a minute in the future, add a FORECAST label */
    if((now + 60) < mr->h_date.unix_time) strcat(label," FORECAST");

    return label;
}

#define LABEL_LEN  256

/**********************************************************************
 * DRAW_HWIN_INTERIOR_LABELS: Label the interior of the horizontal image
 *
 */

int draw_hwin_interior_labels( Drawable xid, int    field, long start_time, long end_time)
{
    int    i;
    int    out_of_date;    
    int    stretch_secs;
    int    x_start,y_start;
    int    xmid,ymid;
    int    ht,wd;
    met_record_t *mr;        /* pointer to record for convienence */
    char    label[LABEL_LEN];
    Font    font;

    if(gd.h_win.margin.top == 0) return 0;

    label[0] = '\0';
     
  mr = gd.mrec[field];    /* get pointer to main gridded field record */
  stretch_secs =  60.0 * mr->time_allowance;
  out_of_date = 0;
  if(gd.check_data_times) {
      if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
      if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
  }

  gd.num_field_labels = 0;
  if(mr->h_data != NULL && !out_of_date) {    /* If data appears to be valid - use its time for a clock */
    /* draw a clock */
    if (gd.show_clock)
    {
      ht = gd.h_win.can_dim.height * 0.05;
      x_start = gd.h_win.can_dim.width - gd.h_win.margin.right - ht - 5;
      y_start = gd.h_win.margin.top + ht + 5;
      XUDRdraw_clock(gd.dpy,xid, gd.extras.foreground_color->gc,
		     x_start, y_start, ht, mr->h_date.unix_time,
		     gd.draw_clock_local);
    }
    
  }
     
  /* calc dimensions of drawable area */
  ht = gd.h_win.margin.top;
  wd = gd.h_win.can_dim.width - gd.h_win.margin.left - gd.h_win.margin.right;
  x_start = gd.h_win.margin.left + 5;
  y_start = gd.h_win.margin.top * 2;

   for(i=0 ; i < NUM_GRID_LAYERS; i++) {   /* Add a label for each active overlay field */
     if(gd.extras.overlay_field_on[i]) {   /* Add a label for the overlay field */
	mr = gd.mrec[gd.extras.overlay_field[i]]; 
	if(gd.check_data_times) {
	  if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
	  if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
	}

	if(mr->h_data != NULL && !out_of_date) {
	  sprintf(label,"Layer %d : ",i+1);
	  strncat(label,field_label(mr),256);
          font = choose_font(label,wd,ht,&xmid,&ymid);
          XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);

	  if(gd.font_display_mode == 0)
            XDrawString(gd.dpy,xid,gd.extras.foreground_color->gc, x_start,y_start + ymid,label,strlen(label));
	  else
            XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc, x_start,y_start + ymid,label,strlen(label));

          gd.num_field_labels++;
          y_start += gd.h_win.margin.top;
	}
    }
  }

  for(i=0; i < NUM_CONT_LAYERS; i++) {
    if(gd.extras.cont[i].active) {     /* Add a label for the contour field */
	mr = gd.mrec[gd.extras.cont[i].field];
	if(gd.check_data_times) {
	   if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
	   if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
	}

	if(mr->h_data != NULL && !out_of_date) {
          sprintf(label,"Contour layer %d ",i);
	  strncat(label,field_label(mr),LABEL_LEN);
          font = choose_font(label,wd,ht,&xmid,&ymid);
          XSetFont(gd.dpy,gd.extras.cont[i].color->gc,font);
	  if(gd.font_display_mode == 0)
            XDrawString(gd.dpy,xid,gd.extras.cont[i].color->gc, x_start,y_start + ymid,label,strlen(label));
	  else 
            XDrawImageString(gd.dpy,xid,gd.extras.cont[i].color->gc, x_start,y_start + ymid,label,strlen(label));

          gd.num_field_labels++;
          y_start += gd.h_win.margin.top;
	}
    }
  }

    if(gd.extras.wind_vectors ) {    /* Add wind labels */
        for(i=0; i < gd.extras.num_wind_sets; i++) {
            if(gd.extras.wind[i].active == 0) continue;
            mr = gd.extras.wind[i].wind_u;
            out_of_date = 0;
            stretch_secs =  60.0 * mr->time_allowance;
	    if(gd.check_data_times) {
               if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
               if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
	    }

            if(gd.extras.wind[i].wind_u->h_data_valid  && gd.extras.wind[i].wind_v->h_data_valid &&
			        gd.extras.wind[i].wind_u->h_data != NULL && !out_of_date) {
                sprintf(label,"%s Vectors: %d/%d %d:%02d",
                    mr->source_name,
                    mr->h_date.month,
                    mr->h_date.day,
                    mr->h_date.hour,
                    mr->h_date.min);
            } else {
	      if(gd.limited_mode) {
		    sprintf(label,"");
	      } else {
                if(out_of_date) {
		    sprintf(label,"%s Vectors: No Data ",mr->source_name);
		} else {
		    sprintf(label,"%s Vectors: N/A ",mr->source_name);
	        }
	      }
            }
                
         font = choose_font(label,wd,ht,&xmid,&ymid);
         XSetFont(gd.dpy,gd.extras.wind[i].color->gc,font);

	if(gd.font_display_mode == 0)
           XDrawString(gd.dpy,xid,gd.extras.wind[i].color->gc, x_start,y_start + ymid,label,strlen(label));
	else 
           XDrawImageString(gd.dpy,xid,gd.extras.wind[i].color->gc, x_start,y_start + ymid,label,strlen(label));

	 gd.num_field_labels++;
	 y_start += gd.h_win.margin.top;
        }
    }
    return SUCCESS;
}
 
/**********************************************************************
 * DRAW_HWIN_TOP_LABEL: Label the top of the horizontal image
 */

int draw_hwin_top_label( Drawable xid, int    field, long start_time, long end_time)
{
    int    i;
    int    out_of_date;    
    int    stretch_secs;
    int    x_start,y_start;
    int    xmid,ymid;
    int    ht,wd;
    met_record_t *mr;        /* pointer to record for convienence */
    char    label[256];
    Font    font;
    GC      gc;

    if(gd.h_win.margin.top == 0) return 0;

    label[0] = '\0';
     
  mr = gd.mrec[field];    /* get pointer to main gridded field record */
  stretch_secs =  60.0 * mr->time_allowance;
  out_of_date = 0;
  if(gd.check_data_times) {
    if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
    if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
  }

    
  gd.num_field_labels = 0;
     
  /* calc dimensions of drawable area */
  ht = gd.h_win.margin.top;
  wd = gd.h_win.can_dim.width - gd.h_win.margin.left - gd.h_win.margin.right;
  x_start =  gd.h_win.margin.left + 5;
  y_start =  ht/2;

  if(mr->h_data != NULL && !out_of_date) {    /* If primary data appears to be valid - Label it */
    strncat(label,field_label(mr),256);
  } else {
    sprintf(label,"%s Not Avail. ",mr->field_name);
  }
               
  font = choose_font(label,wd,ht,&xmid,&ymid);
  XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
  XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc, x_start,y_start + ymid,label,strlen(label));
  gd.num_field_labels++;

  return SUCCESS;
}
 
/**********************************************************************
 * DRAW_HWIN_BOT_LABEL: Draw an axis and label the bottom margin
 */

int draw_hwin_bot_label( Drawable xid, int    field, long start_time, long end_time)
{
    int    tick_xstart;
    int    tick_ystart,tick_yend;
    int    xmid,ymid,tmp;
    int    label_width;
    int    label_height;
    double    range;
    double    min_val;
    double    tick_spacing;
    double    current_tick;
    char    label[16];
    Font    font;

    static double unit_per_km = 0.0;
    static char *u_label;
    
    if(unit_per_km == 0.0) {
        unit_per_km = XRSgetDouble(gd.cidd_db, "cidd.units_per_km", 1.0);
        u_label = XRSgetString(gd.cidd_db, "cidd.scale_units_label", "km");
    }

    if(gd.h_win.margin.bot == 0) return 0;
    /* calc dimensions of drawable area */
    tick_yend = gd.h_win.can_dim.height - (gd.h_win.margin.bot * 0.75);

    range = gd.h_win.cmax_x - gd.h_win.cmin_x;    
    range *= unit_per_km;
    
    tick_spacing = compute_tick_interval(range);
    min_val = rint(gd.h_win.cmin_x * unit_per_km);
    current_tick = min_val + abs((int)min_val %  (int)((tick_spacing > 1.0) ? tick_spacing:  1.0));

    label_width = gd.h_win.img_dim.width  * (tick_spacing / range) * 0.6;
    label_height = gd.h_win.margin.bot * 0.75;
    while(current_tick < gd.h_win.cmax_x * unit_per_km) {
        km_to_pixel(&gd.h_win.margin,(current_tick / unit_per_km),gd.h_win.cmin_y,&tick_xstart,&tick_ystart);
        XDrawLine(gd.dpy,xid,gd.extras.foreground_color->gc,tick_xstart,tick_ystart,tick_xstart,tick_yend);

        sprintf(label,"%.0f",current_tick);
        font = choose_font(label,label_width,label_height,&xmid,&ymid);
        XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
        XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
            (tick_xstart + xmid),gd.h_win.can_dim.height -2,label,strlen(label));

        current_tick += tick_spacing;
    };

    sprintf(label,"%s",u_label);
    font = choose_font(label,label_width,label_height,&tmp,&ymid);
    XSetFont(gd.dpy,gd.extras.foreground_color->gc,font);
    XDrawImageString(gd.dpy,xid,gd.extras.foreground_color->gc,
        (tick_xstart - (2 * xmid) + 1),(gd.h_win.can_dim.height - 2),label,strlen(label));

    return 1;
}
