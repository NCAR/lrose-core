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
 * CIDD_COORDS.C: Coordinate conversion routines for the cidd display program.
 *
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define CIDD_COORDS 1

#include "cidd.h"

/**************************************************************************
 *  GET_BOUNDING_BOX: Return the current lat,lon bounding box of data on the display
 */

void get_bounding_box(double &min_lat, double &max_lat, double &min_lon, double &max_lon)
{

  // condition the longitudes for this zoom

  double meanx = (gd.h_win.cmin_x + gd.h_win.cmax_x) / 2.0;
  double meany = (gd.h_win.cmin_y + gd.h_win.cmax_y) / 2.0;
  double meanLat, meanLon;
  double lon1,lon2,lat1,lat2;

  gd.proj.xy2latlon(meanx,meany,meanLat,meanLon);
  // Make sure meanLon makes since.
  if (meanLon > gd.h_win.max_x) {
  		meanLon -= 360.0;
  } else if (meanLon < gd.h_win.min_x) {
		meanLon += 360.0;
  }
  gd.proj.setConditionLon2Ref(true, meanLon);
  
    if(_params.always_get_full_domain) {
            gd.proj.xy2latlon(gd.h_win.min_x,gd.h_win.min_y,min_lat,min_lon);
            gd.proj.xy2latlon(gd.h_win.max_x,gd.h_win.max_y,max_lat,max_lon);
     } else {
        switch(gd.display_projection) {
          default:
			lon1 = gd.h_win.cmin_x;
			lon2 = gd.h_win.cmax_x;

			if((lon2 - lon1) > 360.0) {
			   lon1 = gd.h_win.min_x;
			   lon2 = gd.h_win.max_x; 
			}


			lat1 = gd.h_win.cmin_y;
			lat2 = gd.h_win.cmax_y;
			if((lat2 - lat1) > 360.0) {
			   lat1 = gd.h_win.min_y;
			   lat2 = gd.h_win.max_y; 
			}

            gd.proj.xy2latlon(lon1, lat1,min_lat,min_lon);
            gd.proj.xy2latlon(lon2, lat2,max_lat,max_lon);

          break;
 
		  case Mdvx::PROJ_FLAT :
          case Mdvx::PROJ_LAMBERT_CONF:
		  case Mdvx::PROJ_POLAR_STEREO:
		  case Mdvx::PROJ_OBLIQUE_STEREO:
		  case Mdvx::PROJ_MERCATOR:
            double lat,lon;
 
            // Compute the bounding box
            max_lon = -360.0;
            min_lon = 360.0;
            max_lat = -180.0;
            min_lat = 180.0;
 
            // Check each corner of the projection + 4 mid points, top, bottom
			// Left and right 

            // Lower left
            gd.proj.xy2latlon(gd.h_win.cmin_x , gd.h_win.cmin_y ,lat,lon);
            if(lon > max_lon) max_lon = lon;
            if(lon < min_lon) min_lon = lon;
            if(lat > max_lat) max_lat = lat;
            if(lat < min_lat) min_lat = lat;
 
            // Lower midpoint
            gd.proj.xy2latlon((gd.h_win.cmin_x +gd.h_win.cmax_x)/2 , gd.h_win.cmin_y,lat,lon);
            if(lon > max_lon) max_lon = lon;
            if(lon < min_lon) min_lon = lon;
            if(lat > max_lat) max_lat = lat;
            if(lat < min_lat) min_lat = lat;
 
            // Lower right
            gd.proj.xy2latlon(gd.h_win.cmax_x , gd.h_win.cmin_y ,lat,lon);
            if(lon > max_lon) max_lon = lon;
            if(lon < min_lon) min_lon = lon;
            if(lat > max_lat) max_lat = lat;
            if(lat < min_lat) min_lat = lat;
 
            // Right midpoint
            gd.proj.xy2latlon(gd.h_win.cmax_x , (gd.h_win.cmin_y + gd.h_win.cmax_y)/2,lat,lon);
            if(lon > max_lon) max_lon = lon;
            if(lon < min_lon) min_lon = lon;
            if(lat > max_lat) max_lat = lat;
            if(lat < min_lat) min_lat = lat;

            // Upper right
            gd.proj.xy2latlon(gd.h_win.cmax_x , gd.h_win.cmax_y ,lat,lon);
            if(lon > max_lon) max_lon = lon;
            if(lon < min_lon) min_lon = lon;
            if(lat > max_lat) max_lat = lat;
            if(lat < min_lat) min_lat = lat;

            // Upper midpoint
            gd.proj.xy2latlon((gd.h_win.cmin_x +gd.h_win.cmax_x)/2 , gd.h_win.cmax_y,lat,lon);
            if(lon > max_lon) max_lon = lon;
            if(lon < min_lon) min_lon = lon;
            if(lat > max_lat) max_lat = lat;
            if(lat < min_lat) min_lat = lat;
 
            // Upper left
            gd.proj.xy2latlon(gd.h_win.cmin_x , gd.h_win.cmax_y ,lat,lon);
            if(lon > max_lon) max_lon = lon;
            if(lon < min_lon) min_lon = lon;
            if(lat > max_lat) max_lat = lat;
            if(lat < min_lat) min_lat = lat;

            // Left midpoint
            gd.proj.xy2latlon(gd.h_win.cmin_x , (gd.h_win.cmin_y + gd.h_win.cmax_y)/2,lat,lon);
            if(lon > max_lon) max_lon = lon;
            if(lon < min_lon) min_lon = lon;
            if(lat > max_lat) max_lat = lat;
            if(lat < min_lat) min_lat = lat;
			 
          break;
       }
   }

   if(gd.display_projection == Mdvx::PROJ_LATLON ) {
     double originLon = (min_lon + max_lon) / 2.0;
     gd.proj.initLatlon(originLon);
   }
}

/**************************************************************************
 * PIXEL_TO_KM: Convert from pixel coords to Km coords 
 *
 */

void pixel_to_disp_proj( margin_t *margin, int pix_x, int pix_y, 
             double *km_x,    /* RETURN */
	     double *km_y)    /* RETURN */
{
    int    x,y;
    double x_range,y_range;

    x = pix_x - margin->left;
    y = gd.h_win.img_dim.height - (pix_y - margin->top);

    x_range = gd.h_win.cmax_x - gd.h_win.cmin_x;
    y_range = gd.h_win.cmax_y - gd.h_win.cmin_y;
    
    *km_x = (((double) x / gd.h_win.img_dim.width) * x_range) + gd.h_win.cmin_x;
    *km_y = (((double) y / gd.h_win.img_dim.height) * y_range) + gd.h_win.cmin_y;

}
 
/**************************************************************************
 * KM_TO_PIXEL: Convert from Km coords to pixel coords 
 *
 */

void disp_proj_to_pixel( margin_t *margin, double km_x, double km_y,
             int    *pix_x,    /* RETURN */
             int    *pix_y)    /* RETURN */
{
    double x_range,y_range;

    x_range = gd.h_win.cmax_x - gd.h_win.cmin_x;
    y_range = gd.h_win.cmax_y - gd.h_win.cmin_y;

    km_x -= gd.h_win.cmin_x;
    km_y -= gd.h_win.cmin_y;
    
    *pix_x = (int) ((km_x / x_range) * gd.h_win.img_dim.width) +  margin->left;
    *pix_y = (int) ((km_y / y_range) * gd.h_win.img_dim.height);
    *pix_y = (gd.h_win.img_dim.height - *pix_y) + margin->top;
}

/**************************************************************************
 * PIXEL_TO_LONLAT: Convert from pixel coords to LAT, LON coords 
 *
 */

void pixel_to_lonlat( margin_t *margin, int pix_x, int pix_y, 
                 double *lon,    /* RETURN */
                 double *lat)    /* RETURN */
{
    double    x_dproj,y_dproj;

    pixel_to_disp_proj(margin,pix_x,pix_y,&x_dproj,&y_dproj);
    PJGLatLonPlusDxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,x_dproj,y_dproj,lat,lon);
}
 
/**************************************************************************
 * LONLAT_TO_PIXEL: Convert from Lat, Lon coords to pixel coords 
 *
 */

void lonlat_to_pixel( margin_t    *margin, double    lon, double    lat,
    int    *pix_x,    /* RETURN */
    int    *pix_y)    /* RETURN */
{
    double    x_dproj,y_dproj;

    PJGLatLon2DxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,lat,lon,&x_dproj,&y_dproj);
    disp_proj_to_pixel(margin,x_dproj,y_dproj,pix_x,pix_y);

}
 
/**************************************************************************
 * KM_TO_GRID: Return the grid coords that contains the given Km coords.
 *
 */

void disp_proj_to_grid( met_record_t *mr, double km_x, double km_y,
    int    *grid_x,    /* RETURN */
    int    *grid_y)    /* RETURN */
{
    *grid_x = (int) ((km_x - mr->h_fhdr.grid_minx) / mr->h_fhdr.grid_dx + 0.5);
    *grid_y = (int) ((km_y - mr->h_fhdr.grid_miny) / mr->h_fhdr.grid_dy + 0.5);
}
 
/**************************************************************************
 * GRID_TO_KM: Convert from grid coords to KM coords 
 *
 */

void grid_to_disp_proj( met_record_t *mr, int grid_x, int grid_y,
    double    *km_x,    /* RETURN */
    double    *km_y)    /* RETURN */
{
    *km_x = (mr->h_fhdr.grid_dx * (double) grid_x) + mr->h_fhdr.grid_minx;
    *km_y = (mr->h_fhdr.grid_dy * (double) grid_y) + mr->h_fhdr.grid_miny;
}


/**************************************************************************
 * PIXEL_TO_GRID: Find the nearest grid point to the given pixel coords
 *
 */

void pixel_to_grid( met_record_t *mr, margin_t *margin, int pix_x, int pix_y,
    int *grid_x,    /* RETURN */
    int *grid_y)    /* RETURN */
{

    if((!_params.use_cosine_correction) && 
       (mr->proj->getProjType() == Mdvx::PROJ_POLAR_RADAR)) {
      // for radar projection without cosine correction, we need
      // a special method
      pixel_to_grid_radar_no_cosine(mr, margin, pix_x, pix_y, grid_x, grid_y);
      return;
    }
    
    double lat,lon;
    double    x_dproj,y_dproj;

    // Screen to Map
    pixel_to_disp_proj(margin,pix_x,pix_y,&x_dproj,&y_dproj);

    // Map to World
    gd.proj.xy2latlon( x_dproj,y_dproj,lat,lon);

    // World to grid
    double current_ht = mr->h_vhdr.level[0];
    mr->proj->latlon2xyIndex(lat,lon,*grid_x,*grid_y, false, current_ht);
    
}

/**************************************************************************
 * PIXEL_TO_GRID_RADAR_NO_COSINE
 * Find the nearest grid point to the given pixel coords, for a radar
 * projection without cosine correction e.g. used for vertical
 * pointing data.
 */

void pixel_to_grid_radar_no_cosine(met_record_t *mr,
                                   margin_t *margin, 
                                   int pix_x, int pix_y,
                                   int *grid_x,    /* RETURN */
                                   int *grid_y)    /* RETURN */
{

  // Screen to Map
  double x_dproj, y_dproj;
  pixel_to_disp_proj(margin,pix_x,pix_y,&x_dproj,&y_dproj);

  // range/az

  double range = sqrt(x_dproj * x_dproj + y_dproj * y_dproj); 
  double az = atan2(x_dproj, y_dproj) * RAD_TO_DEG;
  if (az < 0) az += 360.0;

  // index in range and azimuth

  const Mdvx::coord_t &coord = mr->proj->getCoord();
  int irange = (int) ((range - coord.minx) / coord.dx + 0.5);
  int iaz = (int) ((az - coord.miny) / coord.dy + 0.5);

#ifdef NOTNOW  
  cerr << "pix_x, pix_y, x_dproj, y_dproj, range, az, irange, iaz: "
       << pix_x << ", " << pix_y << ", "
       << x_dproj << ", " << y_dproj << ", "
       << range << ", " << az << ", "
       << irange << ", " << iaz << endl;
#endif

  *grid_x = irange;
  *grid_y = iaz;
  
}

/**************************************************************************
 * PIXEL_TO_KM_V: Convert from pixel coords to Km coords (vertical)
 *
 */

void pixel_to_disp_proj_v( margin_t    *margin, int    pix_x, int    pix_y,
    double    *km_x,    /* RETURN */
    double    *km_ht)    /* RETURN */
{
    int    x,y;
    double x_range,y_range;

    x = pix_x - margin->left;
    y = gd.v_win.img_dim.height - (pix_y - margin->top);

    //x_range = gd.h_win.route.total_length;
    //y_range = gd.v_win.max_ht - gd.v_win.min_ht;
    x_range = gd.v_win.cmax_x - gd.v_win.cmin_x;
    y_range = gd.v_win.cmax_y - gd.v_win.cmin_y;
    
    *km_x = (((double) x / gd.v_win.img_dim.width) * x_range) + gd.v_win.cmin_x;
    //*km_ht = (((double) y / gd.v_win.img_dim.height) * y_range) + gd.v_win.min_ht;
    *km_ht = (((double) y / gd.v_win.img_dim.height) * y_range) + gd.v_win.cmin_y;
}
 
/**************************************************************************
 * KM_TO_PIXEL_V: Convert from Km coords to pixel coords for vertical display
 *
 */

void disp_proj_to_pixel_v( margin_t    *margin, double    km_x, double    km_ht,
    int    *pix_x,    /* RETURN */
    int    *pix_y)    /* RETURN */
{
    double x_range,y_range;

    //x_range = gd.h_win.route.total_length;
    //y_range = gd.v_win.max_ht - gd.v_win.min_ht;
    // km_ht -= gd.v_win.min_ht;
    x_range = gd.v_win.cmax_x - gd.v_win.cmin_x;
    y_range = gd.v_win.cmax_y - gd.v_win.cmin_y;
    km_ht -= gd.v_win.cmin_y;
     
    *pix_x = (int) (((km_x - gd.v_win.cmin_x) / x_range) * gd.v_win.img_dim.width +  margin->left);
    *pix_y = (int) (((km_ht / y_range) * (double) gd.v_win.img_dim.height));
    *pix_y = (int) ((gd.v_win.img_dim.height - *pix_y) + margin->top);
}

/**************************************************************************
 * KM_TO_GRID_V: Return the nearest grid coords to the given Km coords.
 *
 */

void disp_proj_to_grid_v( met_record_t *mr, double    km_x, double    km_ht,
    int    *grid_x,    /* RETURN */
    int    *grid_y)    /* RETURN */
{
    int    i;
    double dist = 9999999.9;

	// Radial projection RHI's 
    if (mr->v_fhdr.vlevel_type == Mdvx::VERT_TYPE_AZ) {
	double twiceRad = PSEUDO_RADIUS * 2.0;
        double radarHt = mr->v_mhdr.sensor_alt;
        double htcorr = (km_x * km_x) / twiceRad; // curveature of earth
        double rel_ht = km_ht - radarHt - htcorr;

	double angle  = (90.0 - (atan2(km_x,rel_ht) * DEG_PER_RAD));

        *grid_y = (int) ((angle -  (mr->v_fhdr.grid_miny - (0.5 * mr->v_fhdr.grid_dy))) /mr->v_fhdr.grid_dy);
	dist = sqrt((km_x * km_x) + (km_ht * km_ht));
        *grid_x = (int) ((dist / mr->v_fhdr.grid_dx) + 0.5);
	return;
    }

    *grid_x = (int) ((km_x / mr->v_fhdr.grid_dx) + 0.5);
    *grid_y = -1;
    for(i=0; i < mr->v_fhdr.nz; i++) {
        if(fabs(km_ht - mr->v_vhdr.level[i])  < dist) {
            *grid_y = i;
			dist = fabs(km_ht - mr->v_vhdr.level[i]);
        }
    }

   return;
}

/**************************************************************************
 *  DISP_PROJ_DIST; Compute Distance between two display projection coordinates
 */

double disp_proj_dist(double x1, double y1, double x2, double y2)
{
  double dist,theta;
  double diff_x;
  double diff_y;

  switch(gd.display_projection) {
      default:
          diff_x = (x2 - x1);
	  diff_y = (y2 - y1);
	  dist =  sqrt((diff_y * diff_y) + (diff_x * diff_x));
      break;

      case Mdvx::PROJ_LATLON:
	  PJGLatLon2RTheta(y1,x1, y2, x2, &dist, &theta);
      break;
  }
   return dist;

}

/**************************************************************************
 * ELAPSED_TIME: Compute the elapsed time in seconds
 */

double elapsed_time(struct timeval &tm1, struct timeval &tm2) 
{
    long t1,t2;

    t1 = tm2.tv_sec - tm1.tv_sec;
    t2 = tm2.tv_usec - tm1.tv_usec; 
    if(t2 < 0) {  // Must borrow a million microsecs from the seconds column  
        t1--;
        t2 += 1000000;
    }  

    return (double) t1 + ((double) t2 / 1000000.0);
}


