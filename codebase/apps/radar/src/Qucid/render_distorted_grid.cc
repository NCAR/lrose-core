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
 * RENDER_DISTORTED_GRID - Render grids that are not
 *  orthagonal in the display's projection
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage June 2002 
 */

#define RENDER_DISTORTED_GRID 1

#include "cidd.h"

void rotate_points(double theta, double x_cent, double y_cent, double *xarr, double *yarr, int num_points);
/**********************************************************************
 * RENDER_DISTORTED_GRID: Render a horizontal plane of  gridded data 
 *    Returns 1 on success, 0 on failure
 */

int render_distorted_grid( Drawable xid, met_record_t *mr, time_t start_time, time_t end_time, int  is_overlay_field)
{
    int    i,j;
    unsigned short    *ptr;
    
    double val;
    double lat,lon;
    double x_km,y_km;
    int    render_method;
    int    num_points;

    int  top_x_array[MAX_COLS];  // The top edge of the row 
    int  top_y_array[MAX_COLS];

    int  bot_x_array[MAX_COLS];  // The Bottom edge of the row 
    int  bot_y_array[MAX_COLS];

    int  *top_row_x,*bot_row_x,*tmp_x;
    int  *top_row_y,*bot_row_y,*tmp_y;

    XPoint  trap[4]; // A trapazoid

    // Use unused parameters 
    start_time = 0;  end_time = 0;

    ptr = ( unsigned short *) mr->h_data;
    if(ptr == NULL) return CIDD_FAILURE;

    set_busy_state(1);

    // Determine the proper rendfering method.
    mr->num_display_pts = 0;
    render_method = mr->render_method; 
    num_points = mr->h_fhdr.nx * mr->h_fhdr.ny;
    if(render_method == DYNAMIC_CONTOURS) {
	if(num_points < _params.dynamic_contour_threshold) {
	    render_method = FILLED_CONTOURS;
	} else {
	    render_method = POLYGONS;
	}
    }
     // Handle case where contours really don't apply (vertical sides) 
    if(mr->h_fhdr.min_value == mr->h_fhdr.max_value ) render_method = POLYGONS; 
    // Contours don't clip - Must render as polygons.

    XStandardColormap best_map;
    // Images don't get countoured
    if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
      render_method = POLYGONS;
      
      if(! XGetStandardColormap(gd.dpy,RootWindow(gd.dpy,0),&best_map,XA_RGB_BEST_MAP)){
        // try to fix the problem
        // safe_system("xstdcmap -best",_params.simple_command_timeout_secs);
        // if(! XGetStandardColormap(gd.dpy,RootWindow(gd.dpy,0),&best_map,XA_RGB_BEST_MAP)){
        // fprintf(stderr,"Can't Render RGB images - Try Running X Server in 24 bit mode\n");
        // fprintf(stderr,"Run 'xstdcmap -best -verbose' to see the problem\n"); 
        // assume 24-bit depth, 8-bit colors
        fprintf(stderr,"WARNING - XGetStandardColormap() failed\n");
        fprintf(stderr,"  Setting base_pixel = 0\n"); 
        fprintf(stderr,"  Setting   red_mult = 65536, green_mult = 256, blue_mult = 1\n"); 
        fprintf(stderr,"  Setting   red_max  = 255,   green_max  = 255, blue_max  = 255\n"); 
        best_map.base_pixel = 0;
        best_map.red_mult = 65536;
        best_map.green_mult = 256;
        best_map.blue_mult = 1;
        best_map.red_max = 255;
        best_map.green_max = 255;
        best_map.blue_max = 255;
        // return CIDD_FAILURE;
        // }
      }
    }

    if(_params.clip_overlay_fields) render_method = POLYGONS; 

    if( render_method  == FILLED_CONTOURS) {

	draw_filled_contours_d(xid,mr);

        set_busy_state(0);
	return 0;
    }

    top_row_x = top_x_array;
    top_row_y = top_y_array;
    bot_row_x = bot_x_array;
    bot_row_y = bot_y_array;

    // Compute the first row of points
    for(i=0; i <= mr->h_fhdr.nx; i++) {  // for each Column
	 mr->proj->xyIndex2latlon(i-0.5,-0.5,lat,lon); // Grid to World
	 gd.proj.latlon2xy(lat,lon,x_km,y_km); // World to Map

	 disp_proj_to_pixel(&(gd.h_win.margin),x_km,y_km,&bot_row_x[i],&bot_row_y[i]);
    }

    // Pointer for RGB Images
    ui32 *uptr;
    uptr = (ui32 *) mr->h_data;
    ui32 pixel;

    for(j = 1; j <= mr->h_fhdr.ny; j++) { // For each row
       // Rotate the old bottom row to the top
        tmp_x = top_row_x; 
        tmp_y = top_row_y; 
	top_row_x = bot_row_x;
	top_row_y = bot_row_y;
	bot_row_x = tmp_x;
	bot_row_y = tmp_y;

	int diffx  = 0;
	int diffy  = 0;


	for(i=0; i <= mr->h_fhdr.nx; i++) { // for each Column
  	    mr->proj->xyIndex2latlon(i-0.5,j-0.5,lat,lon); // Grid to World
	    gd.proj.latlon2xy(lat,lon,x_km,y_km); // World to Map

	    disp_proj_to_pixel(&(gd.h_win.margin),x_km,y_km,&bot_row_x[i],&bot_row_y[i]);

	    if(i > 0) {
	        trap[0].x = bot_row_x[i-1];
	        trap[0].y = bot_row_y[i-1];
	        trap[1].x = top_row_x[i-1];
	        trap[1].y = top_row_y[i-1];

	        trap[2].x = top_row_x[i];
	        trap[2].y = top_row_y[i];
	        trap[3].x = bot_row_x[i];
	        trap[3].y = bot_row_y[i];

		//  If we're an Image, Use RGB
                if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
		  if( MdvGetA(*uptr) != 0) {
                    pixel = best_map.base_pixel +
                      ((ui32) (0.5 + ((_params.image_inten * MdvGetR(*uptr) / 255.0) * best_map.red_max)) * best_map.red_mult) +
                      ((ui32) (0.5 + ((_params.image_inten * MdvGetG(*uptr) / 255.0) * best_map.green_max)) * best_map.green_mult) +
                      ((ui32) (0.5 + ((_params.image_inten * MdvGetB(*uptr) / 255.0) * best_map.blue_max)) * best_map.blue_mult);

                   XSetForeground(gd.dpy,gd.def_gc, pixel);
	           XFillPolygon(gd.dpy, xid, gd.def_gc, trap, 4, Convex, CoordModeOrigin);
		 }
		 uptr++;

                } else {  // Is a data grid - Use the 8 bit data.
		  if(mr->h_vcm.val_gc[*ptr] != NULL) { 
			int render_flag = 1;
            // Check to make sure all points are on the canvas within a 100 pixel buffer.
		    if(_params.check_clipping) {	
			  int minx = -CLIP_BUFFER;
			  int miny = -CLIP_BUFFER;
			  int maxx = gd.h_win.can_dim.width + CLIP_BUFFER;
			  int maxy = gd.h_win.can_dim.height + CLIP_BUFFER;
			  if(trap[0].x < minx  || trap[0].x >  maxx) render_flag = 0;
			  if(trap[1].x < minx  || trap[1].x >  maxx) render_flag = 0;
			  if(trap[2].x < minx  || trap[2].x >  maxx) render_flag = 0;
			  if(trap[3].x < minx  || trap[3].x >  maxx) render_flag = 0;

			  if(trap[0].y < miny  || trap[0].y >  maxy) render_flag = 0;
			  if(trap[1].y < miny  || trap[1].y >  maxy) render_flag = 0;
			  if(trap[2].y < miny  || trap[2].y >  maxy) render_flag = 0;
			  if(trap[3].y < miny  || trap[3].y >  maxy) render_flag = 0;

			  // Now check for Shape - Reject bizare shapes.
			  if(render_flag) {
				minx = trap[0].x;
			    maxx = trap[0].x;
			    miny = trap[0].y;
			    maxy = trap[0].y;
			    for( int k=1; k < 4; k++) {
				    if(trap[k].x < minx) minx = trap[k].x;
				    if(trap[k].x > maxx) maxx = trap[k].x;
				    if(trap[k].y < miny) miny = trap[k].y;
				    if(trap[k].y > maxy) maxy = trap[k].y;
				}
				diffx = abs(maxx - minx) +1;
				diffy = abs(maxy - miny) +1;
				double ratio = (double) diffx / diffy;
				if(ratio > MAX_ASPECT_RATIO) render_flag  = 0;
				if(ratio < 1/MAX_ASPECT_RATIO) render_flag  = 0;
			  }
			}

		    if(is_overlay_field) {
		      val =  (mr->h_fhdr.scale * *ptr) + mr->h_fhdr.bias;
		      if (render_flag  && val >= mr->overlay_min && val <= mr->overlay_max) {
		        XFillPolygon(gd.dpy, xid, mr->h_vcm.val_gc[*ptr],
		                      trap, 4, Convex, CoordModeOrigin);
		      }
		    } else {
		      if(render_flag) {
	            // printf("I,J: %4d,%4d: | %4d %4d %4d %4d | %4d %4d %4d %4d |\n",
				//  i,j, trap[0].x,trap[1].x, trap[2].x,trap[3].x, trap[0].y,trap[1].y, trap[2].y,trap[3].y);
                XFillPolygon(gd.dpy, xid, mr->h_vcm.val_gc[*ptr],
		                  trap, 4, Convex, CoordModeOrigin);
			  }
		    }
		  }
		ptr++;
	      }
	    }   
	}

    }

    set_busy_state(0);


    return 0;
}
