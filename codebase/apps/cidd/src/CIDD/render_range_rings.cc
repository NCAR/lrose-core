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
 * RENDER_RANGE_RINGS.C: 
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_RANGE_RINGS

#include "cidd.h"

#ifdef IRIX5
#define M_PI            3.14159265358979323846
#endif

/**********************************************************************
 * DRAW_CAP_RANGE_RINGS:
 *
 */

void draw_cap_range_rings( Drawable xid)
{
    int    x,y;
    int    xmid,ymid;
    double    min_r,max_r;
    double    origin_lat,origin_lon;
    double    dist,theta;
    double    cur_rad;
    double    min_rad,max_rad;
    double    interval;

    double local_x,local_y;
    double lat,lon;

    char    label[LABEL_LENGTH];
    Font    font;
    static int    x_space,y_space;

    static int    render_labels = 1;
    static double   ring_spacing = 0.0;
    static double   max_ring_range = 1000.0;
    double unit_per_km;
    const char *u_label;

    unit_per_km = gd.scale_units_per_km;
	u_label = gd.scale_units_label;

    if(ring_spacing == 0.0) {
        x_space = gd.uparams->getLong("cidd.range_ring_x_space", 50);
        y_space = gd.uparams->getLong("cidd.range_ring_y_space", 15);
        ring_spacing = gd.uparams->getDouble("cidd.range_ring_spacing", -1.0);
        max_ring_range = gd.uparams->getDouble("cidd.max_ring_range", 1000.0);
        render_labels = gd.uparams->getLong("cidd.range_ring_labels", 1);
    }

    if(ring_spacing > 0.0) {
        interval = ring_spacing;
    } else {
	    interval = 0.0;
        dist = compute_range (gd.h_win.cmin_x,gd.h_win.cmin_y, gd.h_win.cmax_x,gd.h_win.cmax_y);
	    if(gd.display_projection == Mdvx::PROJ_LATLON)  dist *= KM_PER_DEG_AT_EQ;
        dist *= unit_per_km;
        interval = compute_tick_interval(dist);
    }


    if(gd.range_ring_follows_data) {
          origin_lat = gd.mrec[gd.h_win.page]->h_fhdr.proj_origin_lat;
          origin_lon = gd.mrec[gd.h_win.page]->h_fhdr.proj_origin_lon;

    } else {
          origin_lon = gd.h_win.origin_lon;
          origin_lat = gd.h_win.origin_lat;
    }

    /* Don't render the range rings if the origin is at 0.0 lat and 0.0 lon. */
    if (gd.range_ring_follows_data && (
	 (origin_lat == 0.0 && origin_lon == 0.0 ) ||
	 (origin_lat == -90.0 && origin_lon == -180.0)
	)) return;
    
    if(gd.range_ring_for_radar_only) {
      bool renderRings = false;
      Mdvx::projection_type_t projType =
        (Mdvx::projection_type_t) gd.mrec[gd.h_win.page]->h_fhdr.proj_type;
      if (projType == Mdvx::PROJ_POLAR_RADAR ||
          projType == Mdvx::PROJ_FLAT) {
        renderRings = true;
      }
//       Mdvx::vlevel_type_t vlevelType =
//         (Mdvx::vlevel_type_t) gd.mrec[gd.h_win.page]->h_fhdr.vlevel_type;
//       if (vlevelType == Mdvx::VERT_TYPE_ELEV ||
//           vlevelType == Mdvx::VERT_VARIABLE_ELEV ) {
//         renderRings = true;
//       }
      if (!renderRings) {
        return;
      }
    }

    /* find distances to corners - pick the furthest corner for quantities */
    max_r = 0.0;
    min_r = 1000000.0;


	gd.proj.xy2latlon(gd.h_win.cmin_x,gd.h_win.cmin_y,lat,lon);
	PJGLatLon2RTheta(lat,lon,origin_lat,origin_lon, &dist, &theta);
    if(dist < min_r) min_r = dist;
    if(dist > max_r) max_r = dist;

	gd.proj.xy2latlon(gd.h_win.cmax_x,gd.h_win.cmax_y,lat,lon);
	PJGLatLon2RTheta(lat,lon,origin_lat,origin_lon, &dist, &theta);
    if(dist < min_r) min_r = dist;
    if(dist > max_r) max_r = dist;

	gd.proj.xy2latlon(gd.h_win.cmax_x,gd.h_win.cmin_y,lat,lon);
	PJGLatLon2RTheta(lat,lon,origin_lat,origin_lon, &dist, &theta);
    if(dist < min_r) min_r = dist;
    if(dist > max_r) max_r = dist;

	gd.proj.xy2latlon(gd.h_win.cmin_x,gd.h_win.cmax_y,lat,lon);
	PJGLatLon2RTheta(lat,lon,origin_lat,origin_lon, &dist, &theta);
    if(dist < min_r) min_r = dist;
    if(dist > max_r) max_r = dist;

	gd.proj.latlon2xy(origin_lat,origin_lon,local_x,local_y);
    disp_proj_to_pixel(&gd.h_win.margin,local_x,local_y,&x,&y);
	// If the origin is internal
    if(( x < gd.h_win.img_dim.width ) && (x > 0) &&
        (y < gd.h_win.img_dim.height ) && (y > 0)) {
        min_r = interval;
    }

    min_r *= unit_per_km;
    max_r *= unit_per_km;
    if (max_r > max_ring_range) max_r = max_ring_range;

    min_rad = rint(min_r * unit_per_km);
    cur_rad = min_rad - abs((int)min_rad % (int)((interval > 1.0) ? interval:  1.0));
    if(cur_rad < 1.0) min_rad += interval;

	// Find max radius
    for(max_rad = cur_rad; max_rad <= max_r - 0.1; max_rad += interval) {}
    cur_rad = max_rad;
    if(max_rad > 1000.0) return;   // Bail out on very large ranges.

    XPoint xpt[61];  // For 60 segment circle
    int x_lab = 0, y_lab = 0;
    int angle_index = -1;

    while(cur_rad >= min_r) {
	double az = 0.0;
	for(int i = 0; i < 61; i++ ) {
	    PJGLatLonPlusRTheta(origin_lat,origin_lon,(cur_rad/unit_per_km),az,&lat,&lon);
	    gd.proj.latlon2xy(lat,lon,local_x,local_y);
	    disp_proj_to_pixel(&gd.h_win.margin,local_x,local_y,&x,&y);
	    xpt[i].x = x;
	    xpt[i].y = y;

	    if(angle_index >= 0 && i ==  angle_index) {
		x_lab = x;
		y_lab = y;
	    } else  if( angle_index < 0  &&
	        x > gd.h_win.margin.left && x < gd.h_win.img_dim.width + gd.h_win.margin.left &&
	        y > gd.h_win.margin.top && y < gd.h_win.img_dim.height + gd.h_win.margin.top) {

		x_lab = x;
		y_lab = y;
		angle_index = i;
	    }

	    az += 6.0; // Degrees
    	}
	XDrawLines(gd.dpy,xid,gd.legends.range_ring_color->gc,xpt,61,CoordModeOrigin);

	if(interval >= 1.0) {
          sprintf(label,"%.0f %s",cur_rad,u_label);
	} else if (interval >= 0.1) {
          sprintf(label,"%.1f %s",cur_rad,u_label);
	} else {
          sprintf(label,"%.2f %s",cur_rad,u_label);
	}
        font = choose_font(label,x_space,y_space,&xmid,&ymid);
        
        XSetFont(gd.dpy,gd.legends.range_ring_color->gc,font);

        if (render_labels) {
          if(gd.font_display_mode == 0) {
            XDrawString(gd.dpy,xid,gd.legends.range_ring_color->gc,
                        (int)(x_lab + xmid),
                        (int)(y_lab + ymid),
                        label,strlen(label));
          } else {
            XDrawImageString(gd.dpy,xid,gd.legends.range_ring_color->gc,
                             (int)(x_lab + xmid),
                             (int)(y_lab + ymid),
                             label,strlen(label));
          }
        }
        cur_rad -= interval;
    }

    /* render azimuth lines if selected */
    if(gd.legends.azimuths) draw_cap_azimuth_lines(xid);
     
}
