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
 * CIDD_COORDS.C: Coordinate conversion routines for the cidd display program.
 *
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RD_COORDS 1

#include "cidd.h"

/**************************************************************************
 * PIXEL_TO_KM: Convert from pixel coords to Km coords 
 *
 */

pixel_to_km(margin,pix_x,pix_y,km_x,km_y)
    margin_t    *margin;
    int    pix_x;
    int    pix_y;
    double    *km_x;    /* RETURN */
    double    *km_y;    /* RETURN */
{
    int    x,y;
    double x_range,y_range;

    x = pix_x - margin->left;
    y = gd.h_win.img_dim.height - (pix_y - margin->top);

    x_range = gd.h_win.cmax_x - gd.h_win.cmin_x;
    y_range = gd.h_win.cmax_y - gd.h_win.cmin_y;
    
    *km_x = (((double) x / gd.h_win.img_dim.width) * x_range) + gd.h_win.cmin_x;
    *km_y = (((double) y / gd.h_win.img_dim.height) * y_range) + gd.h_win.cmin_y;

    return 0;
}
 
/**************************************************************************
 * KM_TO_PIXEL: Convert from Km coords to pixel coords 
 *
 */

km_to_pixel(margin,km_x,km_y,pix_x,pix_y)
    margin_t    *margin;
    double    km_x;
    double    km_y;
    int    *pix_x;    /* RETURN */
    int    *pix_y;    /* RETURN */
{
    double x_range,y_range;

    x_range = gd.h_win.cmax_x - gd.h_win.cmin_x;
    y_range = gd.h_win.cmax_y - gd.h_win.cmin_y;

    km_x -= gd.h_win.cmin_x;
    km_y -= gd.h_win.cmin_y;
    
    *pix_x = (km_x / x_range) * gd.h_win.img_dim.width +  margin->left;
    *pix_y = (km_y / y_range) * gd.h_win.img_dim.height;
    *pix_y = (gd.h_win.img_dim.height - *pix_y) + margin->top;
    return 0;
}

/**************************************************************************
 * PIXEL_TO_LONLAT: Convert from pixel coords to LAT, LON coords 
 *
 */

pixel_to_lonlat(margin,pix_x,pix_y,lon,lat)
    margin_t    *margin;
    int    pix_x;
    int    pix_y;
    double    *lon;    /* RETURN */
    double    *lat;    /* RETURN */
{
    double    x_km,y_km;

    pixel_to_km(margin,pix_x,pix_y,&x_km,&y_km);
    PJGLatLonPlusDxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,x_km,y_km,lat,lon);
    return 0;
}
 
/**************************************************************************
 * LONLAT_TO_PIXEL: Convert from Lat, Lon coords to pixel coords 
 *
 */

lonlat_to_pixel(margin,lon,lat,pix_x,pix_y)
    margin_t    *margin;
    double    lon;
    double    lat;
    int    *pix_x;    /* RETURN */
    int    *pix_y;    /* RETURN */
{
    double    x_km,y_km;

    PJGLatLon2DxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,lat,lon,&x_km,&y_km);
    km_to_pixel(margin,x_km,y_km,pix_x,pix_y);
    return 0;
}

/**************************************************************************
 * PIXEL_TO_GRID: Find the nearest grid point to the given pixel coords
 *
 */

pixel_to_grid(mr,margin,pix_x,pix_y,grid_x,grid_y)
    met_record_t *mr;
    margin_t    *margin;
    int    pix_x;
    int    pix_y;
    int    *grid_x;    /* RETURN */
    int    *grid_y;    /* RETURN */
{
    double    x_km,y_km;

    pixel_to_km(margin,pix_x,pix_y,&x_km,&y_km);

    switch(mr->data_format) {
      case CART_DATA_FORMAT:
         km_to_grid(mr,x_km,y_km,grid_x,grid_y);
      break;

      case  PPI_DATA_FORMAT:
      {
	  double lat,lon;
	  double radius,theta;
	  double range;

	  switch(gd.projection_mode) {
	      case CARTESIAN_PROJ:
		  PJGLatLonPlusDxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,
					  x_km,y_km,&lat,&lon);
	      break;

	      case LAT_LON_PROJ:
		 lat = y_km;
		 lon = x_km;
	      break;
	  }

	  /* Compute R and Theta relative to the grid's origin */
	  PJGLatLon2RTheta(mr->origin_lat,mr->origin_lon,
			   lat,lon,&radius,&theta);

	 /* The Y grid is just the azmiuth */
	 *grid_y = PJGrange360(theta) / mr->dy + 0.5;

	 /* Convert ground based radius  to beam range */
	 range = radius / cos(DEG_TO_RAD * mr->vert[mr->plane].cent);

	 *grid_x = (int) ((range - mr->min_x)/mr->dx + 0.5);
     }
     break;

   }

   return 0;
}
 
/**************************************************************************
 * KM_TO_GRID: Return the grid coords that contains the given Km coords.
 *
 */

km_to_grid(mr,km_x,km_y,grid_x,grid_y)
    met_record_t *mr;
    double    km_x;
    double    km_y;
    int    *grid_x;    /* RETURN */
    int    *grid_y;    /* RETURN */
{
    switch(mr->order) {
        case 0: /* upper left is first */
            *grid_x = (km_x - mr->min_x) / mr->dx;
            *grid_y = (mr->max_y - km_y) / mr->dy;
        break;

        case 1: /* lower left is first */
            *grid_x = (km_x - mr->min_x) / mr->dx;
            *grid_y = (km_y - mr->min_y) / mr->dy;
        break;
    }
    
    return 0;
}
 
/**************************************************************************
 * GRID_TO_KM: Convert from grid coords to KM coords 
 *
 */

grid_to_km(mr,grid_x,grid_y,km_x,km_y)
    met_record_t *mr;
    int    grid_x;
    int    grid_y;
    double    *km_x;    /* RETURN */
    double    *km_y;    /* RETURN */
{
    *km_x = (mr->dx * grid_x) + mr->min_x + (mr->dx / 2.0);
    if(mr->order) {    /* right handed */
        *km_y = (mr->dy * grid_y) + mr->min_y + (mr->dy / 2.0);
    } else {    /* left handed */
        *km_y = mr->max_y - ((mr->dy * grid_y) + (mr->dy / 2.0));
    }
    return 0;
}

/**************************************************************************
 * GRID_TO_KM_V: Convert from grid coords to KM coords in vertical space
 *
 */

grid_to_km_v(mr,grid_x,grid_y,km_x,km_y)
    met_record_t *mr;
    int    grid_x;
    int    grid_y;
    double    *km_x;    /* RETURN */
    double    *km_y;    /* RETURN */
{
    *km_x = (mr->vdx * grid_x) + mr->vmin_x + (mr->vdx / 2.0);
    if(mr->order) {    /* right handed */
        *km_y = (mr->vdy * grid_y) + mr->vmin_y + (mr->vdy / 2.0);
    } else {    /* left handed */
        *km_y = mr->vmax_y - ((mr->vdy * grid_y) + (mr->vdy / 2.0));
    }
    return 0;
}

/**************************************************************************
 * PIXEL_TO_KM_V: Convert from pixel coords to Km coords (vertical)
 *
 */

pixel_to_km_v(margin,pix_x,pix_y,km_x,km_ht)
    margin_t    *margin;
    int    pix_x;
    int    pix_y;
    double    *km_x;    /* RETURN */
    double    *km_ht;    /* RETURN */
{
    int    x,y;
    double x_range,y_range;

    x = pix_x - margin->left;
    y = gd.v_win.img_dim.height - (pix_y - margin->top);

    x_range = compute_range(gd.v_win.cmin_x,gd.v_win.cmin_y,gd.v_win.cmax_x,gd.v_win.cmax_y);
    y_range = gd.v_win.cmax_ht - gd.v_win.cmin_ht;
    
    *km_x = (((double) x / gd.v_win.img_dim.width) * x_range);
    *km_ht = (((double) y / gd.v_win.img_dim.height) * y_range) + gd.v_win.cmin_ht;

    return 0;
}
 
/**************************************************************************
 * KM_TO_PIXEL_V: Convert from Km coords to pixel coords for vertical display
 *
 */

km_to_pixel_v(margin,km_x,km_ht,pix_x,pix_y)
    margin_t    *margin;
    double    km_x;
    double    km_ht;
    int    *pix_x;    /* RETURN */
    int    *pix_y;    /* RETURN */
{
    double x_range,y_range;

    x_range = compute_range(gd.v_win.cmin_x, gd.v_win.cmin_y, gd.v_win.cmax_x, gd.v_win.cmax_y);
    y_range = gd.v_win.cmax_ht - gd.v_win.cmin_ht;

    km_ht -= gd.v_win.cmin_ht;
     
    *pix_x = (km_x / x_range) * gd.v_win.img_dim.width +  margin->left;
    *pix_y = (km_ht / y_range) * (double) gd.v_win.img_dim.height;
    *pix_y = (gd.v_win.img_dim.height - *pix_y) + margin->top;
    return 0;
}

/**************************************************************************
 * KM_TO_GRID_V: Return the nearest grid coords to the given Km coords.
 *
 */

km_to_grid_v(mr,km_x,km_ht,grid_x,grid_y)
    met_record_t *mr;
    double    km_x;
    double    km_ht;
    int    *grid_x;    /* RETURN */
    int    *grid_y;    /* RETURN */
{
    int    i;
    double    x1_km,y1_km;
    double    x2_km,y2_km;
    double dist;
    double x_del;
    double span;
    double xstart_km,xend_km;

    /* calc total span of window */
    span = compute_range(gd.v_win.cmin_x, gd.v_win.cmin_y,gd.v_win.cmax_x, gd.v_win.cmax_y);

    /* calc where data grid starts */
    grid_to_km(mr,mr->vx1,mr->vy1,&x1_km,&y1_km);
    if(mr->vx1 > mr->vx2) { /* adjust by half width */
        x1_km += mr->dx/2.0;
    } else {
        x1_km -= mr->dx/2.0;
    }     
    if(mr->vy1 > mr->vy2) { /* adjust by half width */
        y1_km += mr->dy/2.0;
    } else {
        y1_km -= mr->dy/2.0;
    } 
    dist = compute_range(gd.v_win.cmax_x, gd.v_win.cmax_y,x1_km,y1_km);
    xstart_km = span - dist;

    /* calc end position */
    grid_to_km(mr,mr->vx2,mr->vy2,&x2_km,&y2_km);
    if(mr->vx1 > mr->vx2) { /* adjust by half width */
        x2_km -= mr->dx/2.0;
    } else {
        x2_km += mr->dx/2.0;
    }  
    if(mr->vy1 > mr->vy2) { /* adjust by half width */
        y2_km -= mr->dy/2.0;
    } else {
        y2_km += mr->dy/2.0;
    }  
    xend_km = compute_range(gd.v_win.cmin_x, gd.v_win.cmin_y,x2_km,y2_km);

    dist = xend_km - xstart_km;
    x_del = dist / (double) mr->v_nx;

    *grid_x = (km_x - xstart_km) / x_del;
    *grid_y = 0;
    for(i=0; i < mr->sects; i++) {
        if(km_ht > mr->vert[i].min && km_ht < mr->vert[i].max) {
            *grid_y = i;
            return -1;
        }
    }
    return 0;
}

/**************************************************************************
 * RAD_GRID_TO_PIXEL: Convert from radial grid coords to pixel coords 
 *                    This routine uses the X grid limits for calculating
 *                    both the X and Y coordinates because of the circular
 *                    nature of radial data.
 *
 */

rad_grid_to_pixel(margin,mr,grid_x,grid_y,pix_x,pix_y)
    margin_t    *margin;
        met_record_t    *mr;
    double    grid_x;
    double    grid_y;
    int    *pix_x;    /* RETURN */
    int    *pix_y;    /* RETURN */
{
    double    km_x;
    double    km_y;
    double x_range, y_range;

    /* convert grid coordinates to "world" coordinates */
    rad_grid_to_km(mr, grid_x, grid_y, &km_x, &km_y);

    /* determine the "world" coordinate width of the display */
    x_range = gd.h_win.cmax_x - gd.h_win.cmin_x;
    y_range = gd.h_win.cmax_y - gd.h_win.cmin_y;

    /* convert "world" coordinates to "world" offset on display */
    km_x -= gd.h_win.cmin_x;
    km_y -= gd.h_win.cmin_y;

    /* convert "world" offset value to pixel */
    *pix_x = (km_x / x_range) * gd.h_win.img_dim.width + margin->left;
    *pix_y = (km_y / y_range) * gd.h_win.img_dim.height;

    /* the pixel origin is at the upper left-hand corner of the display, so */
    /* convert the Y value to reflect this */
    *pix_y = (gd.h_win.img_dim.height - *pix_y) + margin->top;

    return 0;
}

/**************************************************************************
 * RAD_GRID_TO_KM: Convert from radial grid coords to KM coords 
 *                 This routine uses the X grid limits for calculating
 *                 both the X and Y coordinates because of the circular
 *                 nature of radial data.
 *
 */

rad_grid_to_km(mr,grid_x,grid_y,km_x,km_y)
    met_record_t *mr;
    double    grid_x;
    double    grid_y;
    double    *km_x;    /* RETURN */
    double    *km_y;    /* RETURN */
{
    double offset_x, offset_y;

    if(gd.h_win.origin_lat == mr->origin_lat &&
       gd.h_win.origin_lon == mr->origin_lon) {
        offset_x = 0.0;
        offset_y = 0.0;
    } else {               
        PJGLatLon2DxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,
                       mr->origin_lat,mr->origin_lon,
                       &offset_x,&offset_y);    
    }

    *km_x = ((mr->dx * grid_x) + mr->min_x + (mr->dx / 2.0)) + offset_x;
    *km_y = ((mr->dx * grid_y) + mr->min_x + (mr->dx / 2.0)) + offset_y;
}
