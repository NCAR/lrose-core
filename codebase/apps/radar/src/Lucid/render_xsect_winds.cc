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
 * RENDER_XSECT_WINDS: Render Wind fields for the CIDD display program
 *
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */


#define RENDER_XSECT_WINDS
#include "cidd.h"


double w_scale_factor = 0.0;
/**********************************************************************
 * RENDER_VERT_WIND_VECTORS: Render vertical Cross section winds 
 */

int render_vert_wind_vectors( QPaintDevice *pdev)
{

#ifdef NOTYET
  
    int i,j,k;
    // int wd;              /* Dims of drawing canvas */
    int xmid,ymid;
    int nx,nz;
    int x1,y1,x2,y2,x3,y3;
    int width,x_jump,y_jump;
    int endx,endy;          /* Pixel limits of data area */
    int startx,starty;      /* Pixel limits of data area */
    int x_start[MAX_COLS];  /* canvas rectangle begin  coords */
    int y_start[MAX_ROWS];  /* canvas  rectangle begin coords */
    int y_end[MAX_ROWS];    /* canvas  rectangle end coords */
    int use_w_field;        // Flag to indicate the presence of valid W data
    int num_ticks;          // number of ticks (division lines) to place on vectors
    int marker_type = 0;
    int cur_seg = 0;
    int num_way_points = gd.h_win.route.num_segments + 1;

    double  units_scaler;
    double  xscale,yscale,zscale;
    double  xbias,ybias,zbias;
    double  val;
    double  speed;          /* Reference marker */
    char    label[64];      /* Label for reference marker */
    double  dist;           /* distance between corner of vert section and grid point */
    double  dx;
    double  height;
    double  r_wd;      /*  data point rectangle dims */
    double u_proj,v_proj;
    double uspeed,vspeed;   // Speed in standard units

    double seg_dist;
    double total_dist; // Total distance along  segment  double lat[MAX_ROUTE_SEGMENTS]; // Lat lons of  way points
    double seg_len[MAX_ROUTE_SEGMENTS]; // Lengths and direction of each segment
    double seg_theta[MAX_ROUTE_SEGMENTS];
    double lat[MAX_ROUTE_SEGMENTS];  // Array of points - World Coords
    double lon[MAX_ROUTE_SEGMENTS];
    double seg_sin_theta[MAX_ROUTE_SEGMENTS];
    double seg_cos_theta[MAX_ROUTE_SEGMENTS];

    int wpindex;    // Way point index.
    double wp_lat;     // Way point latitude.
    double pt_dist;    // Distance to point

    unsigned short   *ptr_u;
    unsigned short   *ptr_v;
    unsigned short   *ptr_w;
    unsigned short   miss_u;           /* Missing data value */
    unsigned short   miss_v;           /* Missing data value */
    unsigned short   miss_w;           /* Missing data value */
    unsigned short   bad_u;           /* Missing data value */
    unsigned short   bad_v;           /* Missing data value */
    unsigned short   bad_w;           /* Missing data value */

    met_record_t *mr_u;       /* pointer to U record for convienence */
    met_record_t *mr_v;       /* pointer to V record for convienence */
    met_record_t *mr_w;       /* pointer to W record for convienence */

    Font    font;


    if(xid == 0) return CIDD_FAILURE;
    if(w_scale_factor == 0.0) {
      w_scale_factor = _params.wind_w_scale_factor;
    }
    total_dist = 0.0;
    for(i=0; i < num_way_points; i++) {
      gd.proj.xy2latlon( gd.h_win.route.x_world[i], gd.h_win.route.y_world[i], lat[i],lon[i]);
      if(i > 0) {
	PJGLatLon2RTheta(lat[i-1],lon[i-1],
		 lat[i],lon[i],
	         &seg_len[i-1],&seg_theta[i-1]);

        total_dist += seg_len[i-1];
        seg_sin_theta[i-1] = sin(seg_theta[i-1] * DEG_TO_RAD);
        seg_cos_theta[i-1] = cos(seg_theta[i-1] * DEG_TO_RAD);
        //printf("Segment %d, Theta: %g, sin_theta: %g, cos_theta: %g\n",
        // i,seg_theta[i-1],seg_sin_theta[i-1],seg_cos_theta[i-1]); 
      }
    }


    for(k=0; k < gd.layers.num_wind_sets; k++ ) {
        if(gd.layers.wind[k].active == 0) continue;


        marker_type = gd.layers.wind[k].marker_type;

        /* get pointers to desired sections */
        ptr_u = (unsigned short *) gd.layers.wind[k].wind_u->v_data;
        ptr_v = (unsigned short *) gd.layers.wind[k].wind_v->v_data;

	if(ptr_u == NULL || ptr_v== NULL) continue;
         
        mr_u = gd.layers.wind[k].wind_u;    /* get pointer to U data record */
        mr_v = gd.layers.wind[k].wind_v;    /* get pointer to V data record */

        nx = mr_u->v_fhdr.nx;
        dx = mr_u->v_fhdr.grid_dx;
        nz = mr_u->v_fhdr.nz;

	if(nx == 0 || nz == 0) continue;

        if(mr_v->v_fhdr.nx != nx) continue;    /* Consistancy check */
        if(mr_v->v_fhdr.nz != nz) continue;

	// Choose a small font for cross sections
	XSetFont(gd.dpy,gd.layers.wind[k].color->gc,gd.ciddfont[gd.prod.prod_font_num]);

        height = mr_u->vert[0].min;
        disp_proj_to_pixel_v(&(gd.v_win.margin),0.0,height,&startx,&starty);
    
        height = mr_u->vert[mr_u->v_fhdr.nz-1].max;
        disp_proj_to_pixel_v(&(gd.v_win.margin),gd.h_win.route.total_length,height,&endx,&endy);

        r_wd =  (double)ABSDIFF(endx,startx) / (double) nx;       /* get data point rectangle width */
        for(j=0;j< nx; j++) {
            x_start[j] = (int)(((double) j * r_wd) + startx);
        }
    
        for(i= 0; i < mr_u->v_fhdr.nz; i++) {  /* Calc starting/ending coords for the array */
            height = mr_u->vert[i].min;
            disp_proj_to_pixel_v(&(gd.v_win.margin),gd.h_win.route.total_length,height,&endx,&(y_start[i]));
            height = mr_u->vert[i].max;
            disp_proj_to_pixel_v(&(gd.v_win.margin),gd.h_win.route.total_length,height,&endx,&(y_end[i]));
        }
    
        /* get drawable dimensions */
        width = abs((startx-endx)) + 1;
        height = abs((y_start[0] - y_end[mr_u->v_fhdr.nz -1])) + 1;
    
        miss_u =(unsigned short) mr_u->v_fhdr.missing_data_value;
        miss_v =(unsigned short) mr_v->v_fhdr.missing_data_value;
        bad_u =(unsigned short) mr_u->v_fhdr.bad_data_value;
        bad_v =(unsigned short) mr_v->v_fhdr.bad_data_value;
        // wd = (int)(r_wd + 1.0);

        /* get scalers for data  */
	/* Distance is scaled in increments of wind_time_scale_interval (default = 10) minutes */
	val = (double) gd.layers.wind_scaler * gd.layers.wind_time_scale_interval;
	units_scaler = ((val* 60.0) / 1000.0)/ gd.layers.wind[k].wind_u->v_fhdr.grid_dx;

        xscale =  (width / nx) * units_scaler * gd.layers.wind[k].wind_u->v_fhdr.scale;
        xbias = (width / nx) * units_scaler * gd.layers.wind[k].wind_u->v_fhdr.bias;
    
        yscale = (width / nx) * units_scaler * gd.layers.wind[k].wind_v->v_fhdr.scale;
        ybias = (width / nx) * units_scaler * gd.layers.wind[k].wind_v->v_fhdr.bias;
    
    
        /* Try for approx ideal number of vectors on the screen at one time */
        x_jump = (int)((nx / _params.ideal_x_vectors) + 0.5); 
        if(x_jump < 1) x_jump = 1;
        y_jump = (int)((nz / _params.ideal_y_vectors) + 0.5);
        if(y_jump < 1) y_jump = 1;


	// Set up info about the vertical componet
       if(gd.layers.wind[k].wind_w == NULL ||  gd.layers.wind[k].wind_w->v_data == NULL) {
            ptr_w = NULL;
	    use_w_field = 0;
            mr_w = NULL;
            miss_w = 0;
            bad_w = 0;
            zscale = 1.0;
            zbias = 0.0;

	} else {
	    use_w_field = 1;
            ptr_w = (unsigned short *) gd.layers.wind[k].wind_w->v_data;
            mr_w = gd.layers.wind[k].wind_w;    /* get pointer to W data record */

	    // Sanity check
            if(mr_w->v_fhdr.nx != nx) use_w_field = 0;
            if(mr_w->v_fhdr.nz != nz) use_w_field = 0;
            miss_w =(unsigned short) mr_w->v_fhdr.missing_data_value;
            bad_w =(unsigned short) mr_w->v_fhdr.bad_data_value;

            zscale = (height / nz) * units_scaler * gd.layers.wind[k].wind_w->v_fhdr.scale;
            zbias = (height /nz) * units_scaler * gd.layers.wind[k].wind_w->v_fhdr.bias;
	}


        /* loop through the array and draw the vectors */
	int ioff2 = y_jump / 2;
	int joff2 = x_jump / 2;
        for(i = 0; i < nz; i++) {
            cur_seg = 0;
            seg_dist = 0.0;

            for(j=0; j < nx; j++) {

              seg_dist += dx;
              // Check to see if we're past the segment
              if(seg_len[cur_seg] <= seg_dist) {
                   seg_dist =  - (seg_len[cur_seg] - seg_dist);
                   cur_seg++;
              }
              uspeed = (*ptr_u * gd.layers.wind[k].wind_u->v_fhdr.scale) +
		gd.layers.wind[k].wind_u->v_fhdr.bias;
	      vspeed = (*ptr_v * gd.layers.wind[k].wind_v->v_fhdr.scale) +
		gd.layers.wind[k].wind_v->v_fhdr.bias;

                /* If data point is missing or data grid  needs decimated */
		  if((fabs(uspeed) < 0.1 && fabs(vspeed) < 0.1) ||
		   (*ptr_u == miss_u) || (*ptr_v == miss_v) ||
		   (*ptr_u == bad_u) || (*ptr_v == bad_v) ||
		   ((i+ioff2)%y_jump != 0) || ((j+joff2)%x_jump != 0)) {
                    /* Do nothing */
                } else {

		    if(use_w_field) {
		       // Skip this vector if W is bad or missing too.
		       if(*ptr_w == miss_w || *ptr_w == bad_w) continue;
		    }
                    /* compute vertex position */
                    x1 = x_start[j];
                    y1 = (y_start[i] + y_end[i]) / 2;

   
                    switch( marker_type) {
		    default:
                    case ARROWS: /* Use centered arrows */

                        /* compute vector head position */
			 
			u_proj = seg_sin_theta[cur_seg] * ((xscale * *ptr_u) + xbias);
			v_proj = seg_cos_theta[cur_seg] * ((yscale * *ptr_v) + ybias);
			 
                        x2 = (int)(x1 + ((u_proj + v_proj) * 0.5));

                        if(use_w_field) {
			    y2 = (int)(y1 - (((zscale * *ptr_w) + zbias) * 0.5 * w_scale_factor));
			} else {
			    y2 = y1;
			}

                        /* compute vector tail position */
                        x3 = (int)(x1 - ((u_proj + v_proj) * 0.5));

                        if(use_w_field) {
                            y3 = (int)(y1 + (((zscale * *ptr_w) + zbias) * 0.5 * w_scale_factor));
			} else {
			    y3 = y2;
			}

                        XUDRarrow(gd.dpy,xid,gd.layers.wind[k].color->gc,x3,y3,x2,y2,
								_params.wind_head_size + (3 * (gd.layers.wind[k].line_width -1)),
								_params.wind_head_angle * DEG_TO_RAD);

		    break;

                    case TUFT:        /* Use tufts */

                        XUDRcircle(x1, y1, 1, gd.dpy, xid, gd.layers.wind[k].color->gc); /*  */
                        x2 = (int)(x1 + (seg_sin_theta[cur_seg] * (((xscale * *ptr_u) + xbias) * 0.5)) + 
                                  (seg_cos_theta[cur_seg] * (((yscale * *ptr_v) + ybias) * 0.5)));

                        if(use_w_field) {
                           y2 = (int)(y1 - ((zscale * *ptr_w) + zbias * w_scale_factor));
			} else {
			   y2 = y1;
			}
                        XDrawLine(gd.dpy,xid,gd.layers.wind[k].color->gc,x2,y2,x1,y1);
                    break;

                    case BARB:        /* Use barbs */

			XUDRwind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,
				      x1,y1,uspeed,vspeed,_params.barb_shaft_len);
                    break;

		    case VECTOR: /* Use vectors - originating at the grid point  */

                        /* compute vector head position */
			 
			u_proj = seg_sin_theta[cur_seg] * ((xscale * *ptr_u) + xbias);
			v_proj = seg_cos_theta[cur_seg] * ((yscale * *ptr_v) + ybias);
			 
                        x2 = (int)(x1 + (u_proj + v_proj));

                        if(use_w_field) {
			    y2 = (int)(y1 - (((zscale * *ptr_w) + zbias) * 0.5 * w_scale_factor));
			} else {
			    y2 = y1;
			}

                        XUDRarrow(gd.dpy,xid,gd.layers.wind[k].color->gc,x1,y1,x2,y2,
                                  _params.wind_head_size + (3 * (gd.layers.wind[k].line_width -1)),
                                  _params.wind_head_angle * DEG_TO_RAD);

		    break;

		    case TICKVECTOR: /* Use vectors with ticks - originating at the grid point */

                        /* compute vector head position */
			 
			u_proj = seg_sin_theta[cur_seg] * ((xscale * *ptr_u) + xbias);
			v_proj = seg_cos_theta[cur_seg] * ((yscale * *ptr_v) + ybias);
			 
                        x2 = (int)(x1 + ((u_proj + v_proj) * 0.5));

                        if(use_w_field) {
			    y2 = (int)(y1 - (((zscale * *ptr_w) + zbias) * 0.5 * w_scale_factor));
			} else {
			    y2 = y1;
			}

                        /* compute vector tail position */
                        x3 = (int)(x1 - ((u_proj + v_proj) * 0.5));

                        if(use_w_field) {
                          y3 = (int)(y1 + (((zscale * *ptr_w) + zbias) * 0.5 * w_scale_factor));
			} else {
			  y3 = y2;
			}

			num_ticks = (int)((val / gd.layers.wind_time_scale_interval) - 1);
			if (num_ticks <= 0) num_ticks = 1;
			XUDRtickarrow(gd.dpy,xid,gd.layers.wind[k].color->gc,
			              x3,y3,x2,y2,
						  _params.wind_head_size + (3 * (gd.layers.wind[k].line_width -1)),
						  _params.wind_head_angle * DEG_TO_RAD,num_ticks,4);

		    break;

                    case LABELEDBARB:        /* Use Labeled barbs */

			XUDRwind_barb_labeled(gd.dpy,xid,gd.layers.wind[k].color->gc,
				      x1,y1,uspeed,vspeed,_params.barb_shaft_len);
                    break;

		    case METBARB: /* Use Meterologists Labeled barbs */

			wpindex = 0;
			total_dist = 0.0;
			pt_dist = dx * (j + 0.5);


			while(total_dist < pt_dist ) {
		    	total_dist += gd.h_win.route.seg_length[wpindex];
		    	 wpindex++;
			}
			wpindex--;  // The precceeding way point
			if(wpindex >= gd.h_win.route.num_segments) wpindex = gd.h_win.route.num_segments - 1;

			// Distance to preceeding way point;
			total_dist -= gd.h_win.route.seg_length[wpindex];

			// Bi-linear interpolate the Latitude of this point.
			wp_lat = (1 - ((pt_dist - total_dist)/gd.h_win.route.seg_length[wpindex])) * gd.h_win.route.y_world[wpindex];
			wp_lat += ((pt_dist - total_dist)/gd.h_win.route.seg_length[wpindex]) * gd.h_win.route.y_world[wpindex +1];

                        XUDRmet_wind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,
                                          uspeed,vspeed,_params.barb_shaft_len,TENS_ONLY_LABEL,wp_lat);
                    break;

		    case BARB_SH: /* Use S. Hemisphere barbs */
                        XUDRmet_wind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,
                                          uspeed,vspeed,_params.barb_shaft_len,0,-1.0);
                    break;

		    case LABELEDBARB_SH: /* Use S. Hemisphere labeled barbs */
                        XUDRmet_wind_barb(gd.dpy,xid,gd.layers.wind[k].color->gc,x1, y1,
                                          uspeed,vspeed,_params.barb_shaft_len,ROUND10_LABEL,-1.0);
                    break;
 
                }

	      } // Bottom of plot this grid point
              ptr_v++;
              ptr_u++;
              if(use_w_field) ptr_w++;

            } // For each column

        } // for each row

    } // for each wind set ...

    /* Add a reference line */
    switch( marker_type) {
    default:
	break;

	case 1:
	case 2:
	case 4:
	  {
        speed = gd.layers.wind[0].reference_speed;
        /* Distance is scaled in increments of 10 minutes */
        val = (double) gd.layers.wind_scaler * 10.0;

        /* pixels = m/sec * untis_conversion * minutes * 60 seconds/minute / 1km/1000meters / km_span of image * pixels wide the image is */
        dist = speed * gd.layers.wind[0].units_scale_factor *  val * 60.0 / 1000.0 / gd.h_win.route.total_length * gd.v_win.img_dim.width;

        x1 = gd.v_win.margin.left * 2;
        y1 = (int)(gd.v_win.margin.top * 1.55);

        x2 = (int)(x1 + dist);
        y2 = y1;
     
        XUDRarrow(gd.dpy,xid,gd.layers.wind[0].color->gc,x1,y1,x2,y2,
				_params.wind_head_size + (3 * (gd.layers.wind[0].line_width -1)),
				_params.wind_head_angle * DEG_TO_RAD);

        snprintf(label,64,"%g %s (W * %g)",speed,gd.layers.wind[0].units_label,w_scale_factor);
        font = choose_font(label,((x2 -x1) * 5), gd.v_win.margin.top,&xmid,&ymid);
        XSetFont(gd.dpy,gd.layers.wind[0].color->gc,font);
        if(_params.font_display_mode == 0) 
            XDrawString(gd.dpy,xid,gd.layers.wind[0].color->gc, x2 +4,y1 + ymid,label,strlen(label));
        else
            XDrawImageString(gd.dpy,xid,gd.layers.wind[0].color->gc, x2 +4,y1 + ymid,label,strlen(label));
     
	  }
    break;

    case 3: // Barbs
    case 6: 
    case 7:
    case 8:
    case 9:
       // do nothing
    break;

    }  // Reference line switch

#endif
    
    return CIDD_SUCCESS;
}
