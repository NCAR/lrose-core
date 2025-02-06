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
 * RENDER_POLAR_GRID (TNG) - Projects one beam edge onto lat lon.
 *  - Converts to local coords - Use rotation matrix to compute
 *    polygon corners and then convert to screen coords for filling.
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage May 2000 
 */

#define RENDER_POLAR_GRID 1

#include "cidd.h"

void rotate_points(double theta, double x_cent, double y_cent, double *xarr, double *yarr, int num_points);
/**********************************************************************
 * RENDER_POLAR_GRID: Render a horizontal plane of  gridded data 
 *    Returns 1 on success, 0 on failure
 */

int render_polar_grid( QPaintDevice *pdev, MetRecord *mr, time_t start_time, time_t end_time, int  is_overlay_field)
{

#ifdef NOTYET
  
  int    i,j;
    int    x_pix, y_pix;           /* pixel point values */
    unsigned short    *ptr;

    double x_cent,y_cent;
    double  az;                     /* current azimuth angle */
    double radius;
    double cos_elev;
    
    double lat,lon;
//     double  zero_x_array[MAX_COLS];  // The left edge of the 0 degree beam
//     double  zero_y_array[MAX_COLS];

    double  rot_x1_array[MAX_COLS];  // The left edge of one beam 
    double  rot_y1_array[MAX_COLS];
    double  rot_x2_array[MAX_COLS];  // The right edge of one beam
    double  rot_y2_array[MAX_COLS];

    XPoint  rad_trap[4];

    // Use unused parameters 
    start_time = 0;  end_time = 0;

    ptr = ( unsigned short *) mr->h_data;
    if(ptr == NULL) return CIDD_FAILURE;

    set_busy_state(1);

    mr->num_display_pts = 0;

    // compute the center coordinates.
    gd.proj.latlon2xy( mr->h_fhdr.proj_origin_lat,
			mr->h_fhdr.proj_origin_lon,
			x_cent,y_cent);

    // compute the projection coordinates along the left edge of 0 degree beam
    // Note this is line is 0.0 degrees - 1/2 of the beam width

    if(gd.use_cosine_correction) {
      cos_elev = cos(mr->vert[mr->plane].cent * RAD_PER_DEG);
    } else {
      cos_elev = 1.0;
    }
    
    // compute start gate so that we do not use negative gates

    int startGate = 0;
    if (mr->h_fhdr.grid_minx < 0) {
      startGate = (int) ((-1.0 * mr->h_fhdr.grid_minx) / mr->h_fhdr.grid_dx + 0.5);
    }

    // compute the coords of the left edge of the first beam
    az = mr->h_fhdr.grid_miny - mr->h_fhdr.grid_dy / 2.0;
    for(j=0; j <= mr->h_fhdr.nx; j++) {  // for each gate
	 radius = ((mr->h_fhdr.grid_dx * (j - 0.5)) + mr->h_fhdr.grid_minx) * cos_elev;
	 PJGLatLonPlusRTheta(mr->h_fhdr.proj_origin_lat,
			     mr->h_fhdr.proj_origin_lon,
			     radius, az,
			     &lat,&lon);
	 gd.proj.latlon2xy(lat,lon, rot_x2_array[j], rot_y2_array[j]);
    }

    // Now compute the right edges of the beams and plot each valid  gate
    for(i=0; i <  mr->h_fhdr.ny; i++) {

	// The left edge of the next beam is the right edge of the previous beam
	memcpy(rot_x1_array,rot_x2_array,(mr->h_fhdr.nx +1) * sizeof(double));
	memcpy(rot_y1_array,rot_y2_array,(mr->h_fhdr.nx +1) * sizeof(double));

	az += mr->h_fhdr.grid_dy;

	// Compute the  Edge points 
        for(j=0; j <= mr->h_fhdr.nx; j++) {  // for each gate
	     radius = ((mr->h_fhdr.grid_dx * (j - 0.5)) + mr->h_fhdr.grid_minx) * cos_elev;
	     PJGLatLonPlusRTheta(mr->h_fhdr.proj_origin_lat,
			     mr->h_fhdr.proj_origin_lon,
			     radius, az,
			     &lat,&lon);
	 gd.proj.latlon2xy(lat,lon, rot_x2_array[j], rot_y2_array[j]);
        }

	// Now plot each gate as a filled polygon
	for(j = 0; j < mr->h_fhdr.nx; j++, ptr++) {
           if (mr->h_vcm.val_gc[*ptr] != NULL) {
	      if(is_overlay_field) {
		double val =  (mr->h_fhdr.scale * *ptr) + mr->h_fhdr.bias;
	        if(val < mr->overlay_min || val > mr->overlay_max) continue;
	      }

		disp_proj_to_pixel(&gd.h_win.margin, rot_x1_array[j], rot_y1_array[j], &x_pix, &y_pix);
                rad_trap[0].x = x_pix;
                rad_trap[0].y = y_pix;

		int minx = x_pix;
		int maxx = x_pix;
		int miny = y_pix;
		int maxy = y_pix;

		disp_proj_to_pixel(&gd.h_win.margin, rot_x1_array[j+1], rot_y1_array[j+1], &x_pix, &y_pix);
                rad_trap[1].x = x_pix;
                rad_trap[1].y = y_pix;

		minx = MIN(minx, x_pix);
		maxx = MAX(maxx, x_pix);
		miny = MIN(miny, y_pix);
		maxy = MAX(maxy, y_pix);

		disp_proj_to_pixel(&gd.h_win.margin, rot_x2_array[j+1], rot_y2_array[j+1], &x_pix, &y_pix);
                rad_trap[2].x = x_pix;
                rad_trap[2].y = y_pix;

		minx = MIN(minx, x_pix);
		maxx = MAX(maxx, x_pix);
		miny = MIN(miny, y_pix);
		maxy = MAX(maxy, y_pix);

		disp_proj_to_pixel(&gd.h_win.margin, rot_x2_array[j], rot_y2_array[j], &x_pix, &y_pix);
                rad_trap[3].x = x_pix;
                rad_trap[3].y = y_pix;

		minx = MIN(minx, x_pix);
		maxx = MAX(maxx, x_pix);
		miny = MIN(miny, y_pix);
		maxy = MAX(maxy, y_pix);

  		if (minx <= gd.h_win.can_dim.width &&
  		    maxx >= 0 &&
  		    miny <= gd.h_win.can_dim.height &&
  		    maxy >= 0 &&
                    j >= startGate) {

		  // XDrawLines(gd.dpy,xid,gd.legends.route_path_color->gc,rad_trap,3,CoordModeOrigin); 
		  XFillPolygon(gd.dpy, xid, mr->h_vcm.val_gc[*ptr], rad_trap, 4, Convex, CoordModeOrigin);

  		}

		mr->num_display_pts++;
           } 
           
	}
    }
    set_busy_state(0);

#endif
    
    return 0;
}

/*****************************************************************
 * ROTATE_POINTS : Rotate the array of points around x_cent, y_cent
 *  Uses the rotation matrix: 
 *   |  cos_theta  -sin_theta (-xc * cos_theta + yc * sin_theta + xc) |
 *   |  sin_theta   cos_theta (-xc * sin_theta - yc * cos_theta + yc) |
 *   |     0           0                     1                        |
 */
void rotate_points(double theta, double x_cent, double y_cent, double *xarr, double *yarr, int num_points)
{
   int i;
   double xnew, ynew;
   double matrix[6];   // Homogenous matrix for rotation

   if(num_points <=0) return;

   double cos_t = cos(-theta * RAD_PER_DEG);
   double sin_t = sin(-theta * RAD_PER_DEG);

   // Build the rotation matrix 
   // First row
   matrix[0] = cos_t;
   matrix[1] = -sin_t;
   matrix[2] = (-x_cent * cos_t) + (y_cent * sin_t) + x_cent;

   // second row
   matrix[3] = sin_t;
   matrix[4] = cos_t;
   matrix[5] = (-x_cent * sin_t) - (y_cent * cos_t) + y_cent;

   // third row is unity. -> 0,0,1 so not used.

   for(i=0; i < num_points; i++) {
      xnew =  (xarr[i] * matrix[0]) + (yarr[i] *  matrix[1]) + matrix[2];

      ynew =  (xarr[i] * matrix[3]) + (yarr[i] *  matrix[4]) + matrix[5];

      xarr[i] = xnew;
      yarr[i] = ynew;
   }
}
