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
 * RENDER_WINDS: Render Wind fields for the CIDD display program
 *
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_WINDS

#include "cidd.h"


int render_rect_wind_grid(QPaintDevice *pdev, int k, int font_index);
int render_distorted_wind_grid(QPaintDevice *pdev, int k, int font_index);
/**********************************************************************
 * RENDER_WIND_VECTORS:
 *
 */
int render_wind_vectors(QPaintDevice *pdev, int start_time, int end_time)
{

#ifdef NOTYET
  
    int   i,k;
    int   nx,ny;
    int   xmid,ymid;
    int   stretch_secs;
    int   x1,y1,x2,y2;
    int   font_index;
    int   ref_plotted = 0;
    int   marker_type;

    double    slider_val;

    /* Distance is scaled in increments of wind_time_scale_interval (default = 10) minutes */
    slider_val = (double) gd.layers.wind_scaler * gd.layers.wind_time_scale_interval;

    char    label[64];        /* Label for reference marker */
    unsigned short    *ptr_u;
    unsigned short    *ptr_v;

    Font    font;

    // Pick a font size based on the detail thresholds and domain
    font_index = gd.prod.prod_font_num;
    for(i=NUM_PRODUCT_DETAIL_THRESHOLDS -1; i >=0; i--) {
       if( gd.h_win.km_across_screen <= gd.prod.detail[i].threshold) {
         font_index = gd.prod.prod_font_num + gd.prod.detail[i].adjustment;
       }
    }

    if(font_index < 0) font_index = 0;
    if(font_index >= _params.fonts_n) font_index = _params.fonts_n -1;
     
    for(k=0; k < gd.layers.num_wind_sets; k++ ) {
        if(gd.layers.wind[k].active == 0) continue;
        if(gd.layers.wind[k].wind_u->h_data_valid == 0 || 
           gd.layers.wind[k].wind_v->h_data_valid == 0) continue;

        // Add the list of times to the time plot
        if(gd.layers.wind[k].wind_u->time_list.num_entries > 0) {
            if(gd.time_plot) gd.time_plot->add_grid_tlist(gd.layers.wind[k].wind_u->button_name,
                         gd.layers.wind[k].wind_u->time_list.tim,
                         gd.layers.wind[k].wind_u->time_list.num_entries,
                         gd.layers.wind[k].wind_u->h_date.unix_time);
         } 

        marker_type = gd.layers.wind[k].marker_type;
        nx = gd.layers.wind[k].wind_u->h_fhdr.nx;
        ny = gd.layers.wind[k].wind_u->h_fhdr.ny;

        if(nx == 0 || ny == 0) continue;
        if(nx != gd.layers.wind[k].wind_v->h_fhdr.nx) continue;
        if(ny != gd.layers.wind[k].wind_v->h_fhdr.ny) continue;

        /* get pointers to desired sections */
        ptr_u = (unsigned short *) gd.layers.wind[k].wind_u->h_data;
        ptr_v = (unsigned short *) gd.layers.wind[k].wind_v->h_data;

        /* Make sure the data is valid to plot */
        if((ptr_u == NULL) || ( ptr_v == NULL)) continue;

        stretch_secs =  (int)(60.0 *  gd.layers.wind[k].wind_u->time_allowance);
        if(_params.check_data_times) {
          if(gd.layers.wind[k].wind_u->h_date.unix_time < start_time - stretch_secs) continue;
          if(gd.layers.wind[k].wind_u->h_date.unix_time > end_time + stretch_secs) continue;
        }

        // Set the appropriate font size for labels and line width 
        XSetFont(gd.dpy,gd.layers.wind[k].color->gc,gd.ciddfont[font_index]);
        XSetLineAttributes(gd.dpy, gd.layers.wind[k].color->gc, gd.layers.wind[k].line_width,
		       LineSolid, CapButt, JoinMiter);

	switch(gd.proj.getProjType()) {
	  default:
            if(gd.layers.wind[k].wind_u->h_fhdr.proj_type == gd.proj.getProjType() &&
	      (fabs(gd.h_win.origin_lat - gd.layers.wind[k].wind_u->h_fhdr.proj_origin_lat) < 0.01) &&
	      (fabs(gd.h_win.origin_lon - gd.layers.wind[k].wind_u->h_fhdr.proj_origin_lon) < 0.01)) {

                render_rect_wind_grid(xid,k,font_index);
            } else {
                render_distorted_wind_grid(xid,k,font_index);
        
            }
	  break;

	  case  Mdvx::PROJ_LATLON:
	      if(gd.layers.wind[k].wind_u->h_fhdr.proj_type == Mdvx::PROJ_LATLON ) {
		   render_rect_wind_grid(xid,k,font_index); 
	      } else {
                render_distorted_wind_grid(xid,k,font_index);
	      }
	  break;
	}

        if(ref_plotted == 0 && _params.display_ref_lines) { // only plot one legend
          switch(marker_type) { // Only plot Reference lines for wind features that need it.
          default:
          break;

                  /* Add a reference line */
          case ARROWS:
          case TUFT:
          case VECTOR:
          case TICKVECTOR: {
            double km_pix = gd.h_win.km_across_screen / gd.h_win.img_dim.width;

            // slider value is the time in minutes that the vectors show motion over
            //  distance in meters
            double ref_dist = (slider_val * 60) * gd.layers.wind[k].reference_speed *
                             gd.layers.wind[k].units_scale_factor;


            x1 = (int)(gd.h_win.margin.left + (gd.h_win.can_dim.width /2.0));
            x2 = (int)(x1 + ((ref_dist / 1000) / km_pix));

            y1 = (int)(gd.h_win.margin.top * 1.5);
            y2 = y1;

	    // Draw the reference bar
	    XDrawLine(gd.dpy,xid,gd.layers.wind[k].color->gc,x2,y2,x1,y1);
	    XDrawLine(gd.dpy,xid,gd.layers.wind[k].color->gc,x1,y1-3,x1,y1+3);
	    XDrawLine(gd.dpy,xid,gd.layers.wind[k].color->gc,x2,y2-3,x2,y2+3);
    
            snprintf(label,64,"%g %s",gd.layers.wind[k].reference_speed,gd.layers.wind[k].units_label);
            font = choose_font(label,(int) (gd.h_win.can_dim.width /2.0), gd.h_win.margin.top,
                          &xmid,&ymid);
            XSetFont(gd.dpy,gd.layers.wind[k].color->gc,font);
            if(_params.font_display_mode == 0) 
              XDrawString(gd.dpy,xid,gd.layers.wind[k].color->gc,
                      x2 +4,y1 + ymid,label,strlen(label));
            else
              XDrawImageString(gd.dpy,xid,gd.layers.wind[k].color->gc,
                      x2 +4,y1 + ymid,label,strlen(label));
           }

           ref_plotted = 1;
         break;
         }
      } // If no legend plotted yet
   } // for each wind set

#endif         

    return(0);
}

/**********************************************************************
 * RENDER_RECT_WIND_GRID: Render the winds on a regular rectangular grid
 */
int render_rect_wind_grid(QPaintDevice *pdev, int k, int font_index)
{

#ifdef NOTYET

  int    i,j;
    int    nx,ny;
    int    num_ticks;
    int    width,height;
    int    x1,y1,x2,y2,x3,y3;
    int    xstart,ystart,xend,yend;
    int    x_jump,y_jump;
    int    x_start[MAX_COLS];    /* canvas rectangle begin  coords */
    int    y_start[MAX_ROWS];    /* canvas  rectangle begin coords */
    int    marker_type;

    double  units_scaler;
    double  g_width,g_height;
    double  uspeed,vspeed;
    double  xscale,yscale;
    double  xbias,ybias;
    double  slider_val;
    double  km_x,km_y;
    double  lat,lon;

    unsigned short    u_miss,v_miss;        /* missing data values */
    unsigned short    u_bad,v_bad;        /* bad data values */
    unsigned short    *ptr_u;
    unsigned short    *ptr_v;

    /* get pointers to desired sections - Parent has checked for null */
    ptr_u = (unsigned short *) gd.layers.wind[k].wind_u->h_data;
    ptr_v = (unsigned short *) gd.layers.wind[k].wind_v->h_data;

    // Get barb type and dimensions - Parent has checked for consistancy
    marker_type = gd.layers.wind[k].marker_type;
    nx = gd.layers.wind[k].wind_u->h_fhdr.nx;
    ny = gd.layers.wind[k].wind_u->h_fhdr.ny;

    /* Get Canvas coordinates for wind vector points */
    grid_to_disp_proj(gd.layers.wind[k].wind_v,0,0,&km_x,&km_y);
    disp_proj_to_pixel(&(gd.h_win.margin),km_x,km_y,&xstart,&ystart);
    
    grid_to_disp_proj(gd.layers.wind[k].wind_v,gd.layers.wind[k].wind_v->h_fhdr.nx-1,gd.layers.wind[k].wind_v->h_fhdr.ny-1,&km_x,&km_y);
    disp_proj_to_pixel(&(gd.h_win.margin),km_x,km_y,&xend,&yend);
    
    /* get drawable dimensions */
    width = abs((xstart-xend) + 1);
    height = abs((ystart-yend) + 1);
    
    g_width = (nx > 1) ?  (double)width / (nx -1): 0.0 ;
    g_height = (ny > 1) ? (double)height / (ny -1) : 0.0;
    
    for(i= 0; i < ny; i++) {    /* Calc starting coords for the array */
        y_start[i] = (int)(ystart - ((double)i * g_height));
    }
    for(j=0;j< nx; j++) {
        x_start[j] = (int)(((double) j * g_width) + xstart);
    }
    /* get scalers for data  */

    /* Distance is scaled in increments of wind_time_scale_interval (default = 10) minutes */
    slider_val = (double) gd.layers.wind_scaler * gd.layers.wind_time_scale_interval;

    units_scaler = ((slider_val* 60.0) / 1000.0)/
		     gd.layers.wind[k].wind_u->h_fhdr.grid_dx *
                     gd.layers.wind[k].units_scale_factor;

    // Convert to projection units
    if(gd.display_projection == Mdvx::PROJ_LATLON) 
       units_scaler /= KM_PER_DEG_AT_EQ;


    xscale =  ((double)width / nx) * units_scaler * gd.layers.wind[k].wind_u->h_fhdr.scale;
    xbias = ((double)width / nx) * units_scaler * gd.layers.wind[k].wind_u->h_fhdr.bias;
    
    yscale = ((double)height / ny) * units_scaler * gd.layers.wind[k].wind_v->h_fhdr.scale;
    ybias = ((double)height /ny) * units_scaler * gd.layers.wind[k].wind_v->h_fhdr.bias;
    
     
    u_miss = (unsigned short) gd.layers.wind[k].wind_u->h_fhdr.missing_data_value;
    v_miss = (unsigned short) gd.layers.wind[k].wind_v->h_fhdr.missing_data_value;
    u_bad = (unsigned short) gd.layers.wind[k].wind_u->h_fhdr.bad_data_value;
    v_bad = (unsigned short) gd.layers.wind[k].wind_v->h_fhdr.bad_data_value;
    
    /* Try for approx ideal number of vectors on the screen at one time */
    x_jump = (int)((nx / _params.ideal_x_vectors) + 0.5); 
    if(x_jump < 1) x_jump = 1;

    y_jump = (int)((ny / _params.ideal_y_vectors) + 0.5);
    if(y_jump < 1) y_jump = 1;
     
    /* loop through the array and draw the vectors */
    for(i = 0; i < ny; i++) {
        for(j=0; j < nx; j++) {
          
          uspeed = (*ptr_u * gd.layers.wind[k].wind_u->h_fhdr.scale) + gd.layers.wind[k].wind_u->h_fhdr.bias;
          vspeed = (*ptr_v * gd.layers.wind[k].wind_v->h_fhdr.scale) + gd.layers.wind[k].wind_v->h_fhdr.bias;

        /* If data point has 0 wind or is missing or bad or
         * data grid  needs decimated */

          if((fabs(uspeed) < 0.1 && fabs(vspeed) < 0.1) ||
           (*ptr_u == u_miss) || (*ptr_v == v_miss) ||
           (*ptr_u == u_bad) || (*ptr_v == v_bad) ||
           (i%y_jump != 0) || (j%x_jump != 0)) {
                    /* Do nothing */
          } else {
   
            /* compute vertex position */
            x1 = x_start[j];
            y1 = y_start[i];
   
            switch(marker_type) {
              default :
              case ARROWS:  /* Use (centered) arrows */
    
                /* compute vector head position */
                x2 = (int)(x1 + (((xscale * *ptr_u) + xbias) * 0.5));
                y2 = (int)(y1 - (((yscale * *ptr_v) + ybias) * 0.5));
  
                /* compute vector tail position */
                x3 = (int)(x1 - (((xscale * *ptr_u) + xbias) * 0.5));
                y3 = (int)(y1 + (((yscale * *ptr_v) + ybias) * 0.5));
 
                /* Stop drawing winds when the features are more than
                 * 25 % of the size of the image */
                if((abs(x2-x3) < width/4) && (abs(y2-y3) < height/4))
                XUDRarrow(gd.dpy,xid,gd.layers.wind[k].color->gc,x3,y3,x2,y2,_params.wind_head_size +
						 (3 * (gd.layers.wind[k].line_width -1)) ,_params.wind_head_angle * DEG_TO_RAD);
              break;

              case TUFT: /* Use tufts */

                XUDRcircle(x1, y1, 1, gd.dpy, xid, gd.layers.wind[k].color->gc); /*  */
                x2 = (int)(x1 + ((xscale * *ptr_u) + xbias));
                y2 = (int)(y1 - ((yscale * *ptr_v) + ybias));
                /* Stop drawing winds when the features are more than
                 * 25 % of the size of the image */
                if((abs(x2-x1) < width/4) && (abs(y2-y1) < height/4))
                XDrawLine(gd.dpy,xid,gd.layers.wind[k].color->gc,x2,y2,x1,y1);
              break;

              case BARB: /* Use barbs */

                XUDRwind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,uspeed,vspeed,_params.barb_shaft_len);
              break;

              case VECTOR: /* Use vectors */

                x2 = (int)(x1 + ((xscale * *ptr_u) + xbias));
                y2 = (int)(y1 - ((yscale * *ptr_v) + ybias));
                /* Stop drawing winds when the features are more than
                 * 25 % of the size of the image */
                if((abs(x2-x1) < width/4) && (abs(y2-y1) < height/4))
                    XUDRarrow(gd.dpy,xid,gd.layers.wind[k].color->gc,x1,y1,x2,y2,_params.wind_head_size +
							 (3 * (gd.layers.wind[k].line_width -1)) ,_params.wind_head_angle * DEG_TO_RAD);
              break;

              case TICKVECTOR: /* Use vectors with ticks */
                x2 = (int)(x1 + ((xscale * *ptr_u) + xbias));
                y2 = (int)(y1 - ((yscale * *ptr_v) + ybias));
                num_ticks = (int)((slider_val / gd.layers.wind_time_scale_interval) - 1);
                if (num_ticks <= 0) num_ticks = 1;
                /* Stop drawing winds when the features are more than
                 * 25 % of the size of the image */
                if((abs(x2-x1) < width/4) && (abs(y2-y1) < height/4))
                    XUDRtickarrow(gd.dpy,xid,gd.layers.wind[k].color->gc,x1,y1,x2,y2,
                        _params.wind_head_size + (3 * (gd.layers.wind[k].line_width -1)),
						_params.wind_head_angle * DEG_TO_RAD,num_ticks,4);
              break;

              case LABELEDBARB: /* Use Labeled barbs */

                XUDRwind_barb_labeled(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,uspeed,vspeed,_params.barb_shaft_len);
              break;

              case METBARB: /* Use Meterologists Labeled barbs */

                pixel_to_disp_proj(&gd.h_win.margin, x1, y1, &km_x, &km_y);
                gd.proj.xy2latlon(km_x,km_y,lat,lon);

                XUDRmet_wind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,
                                  uspeed,vspeed,_params.barb_shaft_len,TENS_ONLY_LABEL,lat);
              break;

              case BARB_SH: /* Use S. Hemisphere barbs */

                XUDRmet_wind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,
                                  uspeed,vspeed,_params.barb_shaft_len,0,-1.0);
              break;

              case LABELEDBARB_SH: /* Use S. Hemisphere labeled  barbs */

                XUDRmet_wind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,
                                  uspeed,vspeed,_params.barb_shaft_len,ROUND10_LABEL,-1.0);
              break;
            }
        }  // is a plottable vector
    
        ptr_v++;
        ptr_u++;
      }

    }

#endif
    
    return(0);
}

/**********************************************************************
 * RENDER_DISTORTED_WIND_GRID
 */
int render_distorted_wind_grid(QPaintDevice *pdev, int k, int font_index)
{

#ifdef NOTYET
  
    int    i,j;
    int    nx,ny;
    int    num_ticks;
    int    x1,y1,x2,y2,x3,y3;
    int    x_jump,y_jump;
    int    marker_type;

    double    wind_scale;
    double    uspeed,vspeed;
    double    slider_val;
    double    km_x,km_y;
    double    km_x2,km_y2;
    double    lat,lon;
    double    lat2,lon2;

    unsigned short    u_miss,v_miss;        /* missing data values */
    unsigned short    u_bad,v_bad;        /* bad data values */
    unsigned short    *ptr_u;
    unsigned short    *ptr_v;

    MetRecord *mr_u;
    // MetRecord *mr_v;

    /* get pointers to desired sections - Parent has checked for null */
    ptr_u = (unsigned short *) gd.layers.wind[k].wind_u->h_data;
    ptr_v = (unsigned short *) gd.layers.wind[k].wind_v->h_data;

    // Get barb type and dimensions - Parent has checked for consistancy
    marker_type = gd.layers.wind[k].marker_type;
    nx = gd.layers.wind[k].wind_u->h_fhdr.nx;
    ny = gd.layers.wind[k].wind_u->h_fhdr.ny;

    // Get convienient pointers 
    mr_u = gd.layers.wind[k].wind_u;
    // mr_v = gd.layers.wind[k].wind_v;

    u_miss = (unsigned short) gd.layers.wind[k].wind_u->h_fhdr.missing_data_value;
    v_miss = (unsigned short) gd.layers.wind[k].wind_v->h_fhdr.missing_data_value;
    u_bad = (unsigned short) gd.layers.wind[k].wind_u->h_fhdr.bad_data_value;
    v_bad = (unsigned short) gd.layers.wind[k].wind_v->h_fhdr.bad_data_value;
    

    /* Distance is scaled in increments of wind_time_scale_interval (default = 10) minutes */
    slider_val = (double) gd.layers.wind_scaler * gd.layers.wind_time_scale_interval;

    wind_scale = ((slider_val* 60.0) / 1000.0);  // km per m/sec


    /* Try for approx ideal number of vectors on the screen at one time */
    x_jump = (int)((nx / _params.ideal_x_vectors) + 0.5); 
    if(x_jump < 1) x_jump = 1;
    y_jump = (int)((ny / _params.ideal_y_vectors) + 0.5);
    if(y_jump < 1) y_jump = 1;

    for(j= 0; j < ny; j++) { 
      for(i=0;i< nx; i++) {
          uspeed = (*ptr_u * gd.layers.wind[k].wind_u->h_fhdr.scale) + 
		    gd.layers.wind[k].wind_u->h_fhdr.bias;
	  uspeed *= gd.layers.wind[k].units_scale_factor;

          vspeed = (*ptr_v * gd.layers.wind[k].wind_v->h_fhdr.scale) +
		    gd.layers.wind[k].wind_v->h_fhdr.bias;
	  vspeed *= gd.layers.wind[k].units_scale_factor;

          // If data point has 0 wind or is missing or bad or point needs decimated 
          if((fabs(uspeed) < 0.1 && fabs(vspeed) < 0.1) ||
            (*ptr_u == u_miss) || (*ptr_v == v_miss) ||
            (*ptr_u == u_bad) || (*ptr_v == v_bad) ||
            (i%y_jump != 0) || (j%x_jump != 0)) { /* Do nothing */ 

          } else {

	    // Compute point position
	    km_x =  mr_u->h_fhdr.grid_minx + (mr_u->h_fhdr.grid_dx * i);
	    km_y =  mr_u->h_fhdr.grid_miny + (mr_u->h_fhdr.grid_dy * j);
	    mr_u->proj->xy2latlon(km_x,km_y,lat,lon); // Grid to World

	    gd.proj.latlon2xy(lat,lon,km_x2,km_y2); // World to Map
	    disp_proj_to_pixel(&(gd.h_win.margin),km_x2,km_y2,&x1,&y1); // Map to Screen

            switch(marker_type) {
              default :
              case ARROWS:  /* Use (centered) arrows */
    
		if(mr_u->h_fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {
		  km_x = -uspeed * 0.5 * wind_scale;
		  km_y = -vspeed * 0.5 * wind_scale;
		  PJGLatLonPlusDxDy(lat,lon,km_x, km_y,&lat2,&lon2);
		} else {
	           // compute Tail position - Back half way
	           km_x -= uspeed * 0.5 * wind_scale;
	           km_y -= vspeed * 0.5  * wind_scale;

	          mr_u->proj->xy2latlon(km_x,km_y,lat2,lon2); // Grid to World
		}

		gd.proj.latlon2xy(lat2,lon2,km_x2,km_y2); // World to Map
	        disp_proj_to_pixel(&(gd.h_win.margin),km_x2,km_y2,&x2,&y2); // Map to Screen
    
	        // compute Head posiition
		if(mr_u->h_fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {
		  km_x = uspeed * 0.5 * wind_scale;
		  km_y = vspeed * 0.5 * wind_scale;
		  PJGLatLonPlusDxDy(lat,lon,km_x, km_y,&lat2,&lon2);

		} else {
	          km_x += uspeed * wind_scale;
	          km_y += vspeed * wind_scale;
	          mr_u->proj->xy2latlon(km_x,km_y,lat2,lon2); // Grid to World
		}

		gd.proj.latlon2xy(lat2,lon2,km_x2,km_y2); // World to Map
	        disp_proj_to_pixel(&(gd.h_win.margin),km_x2,km_y2,&x3,&y3); // Map to Screen

                /* Stop drawing winds when the features are more than
                 * 25 % of the size of the image */
                if((abs(x2-x3) < gd.h_win.img_dim.width/4) && 
		   (abs(y2-y3) < gd.h_win.img_dim.height/4))
                    XUDRarrow(gd.dpy,xid,gd.layers.wind[k].color->gc,x2,y2,x3,y3,
			      _params.wind_head_size + (3 * (gd.layers.wind[k].line_width -1)),_params.wind_head_angle * DEG_TO_RAD);
              break;

              case TUFT: /* Use tufts */

                XUDRcircle(x1, y1, 1, gd.dpy, xid, gd.layers.wind[k].color->gc); /*  */
	        // compute Head posiition
	        km_x += uspeed * wind_scale;
	        km_y += vspeed * wind_scale;
	        mr_u->proj->xy2latlon(km_x,km_y,lat2,lon2); // Grid to World
		gd.proj.latlon2xy(lat2,lon2,km_x2,km_y2); // World to Map
	        disp_proj_to_pixel(&(gd.h_win.margin),km_x2,km_y2,&x2,&y2); // Map to Screen

                /* Stop drawing winds when the features are more than
                 * 25 % of the size of the image */
                if((abs(x2-x1) < gd.h_win.img_dim.width/4) && 
		   (abs(y2-y1) < gd.h_win.img_dim.height/4))
                    XDrawLine(gd.dpy,xid,gd.layers.wind[k].color->gc,x2,y2,x1,y1);
              break;

              case BARB: /* Use barbs */
		 // KLUDGE  - FIX ME
		 // Warning - Gives BARB on GRID - NOT TRUE E-W 
                 XUDRwind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,uspeed,vspeed,_params.barb_shaft_len);
              break;

              case VECTOR: /* Use vectors */

		if(mr_u->h_fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {
		  km_x = uspeed * wind_scale;
		  km_y = vspeed * wind_scale;
		  PJGLatLonPlusDxDy(lat,lon,km_x, km_y,&lat2,&lon2);

		} else {
	          // compute Head posiition
	          km_x += uspeed * wind_scale;
	          km_y += vspeed * wind_scale;
	          mr_u->proj->xy2latlon(km_x,km_y,lat2,lon2); // Grid to World
		}
		gd.proj.latlon2xy(lat2,lon2,km_x2,km_y2); // World to Map
	        disp_proj_to_pixel(&(gd.h_win.margin),km_x2,km_y2,&x2,&y2); // Map to Screen

                /* Stop drawing winds when the features are more than
                 * 25 % of the size of the image */
                if((abs(x2-x1) < gd.h_win.img_dim.width/4) && 
		   (abs(y2-y1) < gd.h_win.img_dim.height/4))
                     XUDRarrow(gd.dpy,xid,gd.layers.wind[k].color->gc,x1,y1,x2,y2,
							 _params.wind_head_size + (3 * (gd.layers.wind[k].line_width -1)),
							 _params.wind_head_angle * DEG_TO_RAD);
              break;

              case TICKVECTOR: /* Use vectors with ticks */
	        // compute Head posiition
		if(mr_u->h_fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {
		  km_x = uspeed * wind_scale;
		  km_y = vspeed * wind_scale;
		  PJGLatLonPlusDxDy(lat,lon,km_x, km_y,&lat2,&lon2);

		} else {
	          km_x += uspeed * wind_scale;
	          km_y += vspeed * wind_scale;
	          mr_u->proj->xy2latlon(km_x,km_y,lat2,lon2); // Grid to World
		}
		gd.proj.latlon2xy(lat2,lon2,km_x2,km_y2); // World to Map
	        disp_proj_to_pixel(&(gd.h_win.margin),km_x2,km_y2,&x2,&y2); // Map to Screen

                num_ticks = (int)((slider_val / gd.layers.wind_time_scale_interval) - 1);
                if (num_ticks <= 0) num_ticks = 1;
                /* Stop drawing winds when the features are more than
                 * 25 % of the size of the image */
                if((abs(x2-x1) < gd.h_win.img_dim.width/4) && 
		   (abs(y2-y1) < gd.h_win.img_dim.height/4))
                    XUDRtickarrow(gd.dpy,xid,gd.layers.wind[k].color->gc,x1,y1,x2,y2,
                        _params.wind_head_size + (3 * (gd.layers.wind[k].line_width -1)),
						_params.wind_head_angle * DEG_TO_RAD,num_ticks,4);
              break;

              case LABELEDBARB: /* Use Labeled barbs */
		 // KLUDGE  - FIX ME
		 // Warning - Gives BARB on GRID - NOT TRUE E-W 
                XUDRwind_barb_labeled(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,uspeed,vspeed,_params.barb_shaft_len);
              break;

              case METBARB: /* Use Meterologists Labeled barbs */
		 // KLUDGE  - FIX ME
		 // Warning - Gives BARB on GRID - NOT TRUE E-W 
                 XUDRmet_wind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,
                                  uspeed,vspeed,_params.barb_shaft_len,TENS_ONLY_LABEL,lat);
              break;

              case BARB_SH: /* Use S. Hemisphere barbs */
		 // KLUDGE  - FIX ME
		 // Warning - Gives BARB on GRID - NOT TRUE E-W 
                XUDRmet_wind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,
                                  uspeed,vspeed,_params.barb_shaft_len,0,-1.0);
              break;

              case LABELEDBARB_SH: /* Use S. Hemisphere labeled  barbs */
		 // KLUDGE  - FIX ME
		 // Warning - Gives BARB on GRID - NOT TRUE E-W 
                XUDRmet_wind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,
                                  uspeed,vspeed,_params.barb_shaft_len,ROUND10_LABEL,-1.0);
              break;
            }
        }  // is a plottable vector
    
        ptr_v++;
        ptr_u++;
      } // Each column
    }   // Each Row

#endif
    
    return(0);
}
