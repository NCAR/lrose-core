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
 * RENDER_AZMIUTHS.C: 
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */


#define RENDER_AZMIUTHS

#include "cidd.h"

#ifdef IRIX5
#define M_PI            3.14159265358979323846
#endif

/**********************************************************************
 * DRAW_CAP_AZMITH_LINES:
 *
 */

void draw_cap_azmith_lines(Drawable xid)
{
    int    x1,y1,x2,y2;
    double    x_dproj,y_dproj;
    double    xstart,ystart;
    double    xend,yend;
    double    cur_angle;
    static double interval = 0.0;
    static double radius = 0.0;

    if(interval == 0.0) {
        interval = gd.uparams->getDouble( "cidd.azmith_interval", 30.0);
        interval = gd.uparams->getDouble( "cidd.azimuth_interval", interval);
        interval *= (M_PI/180.0); /* convert to radians */
        interval = fabs(interval);
    }

    if(radius == 0.0) {
        radius = gd.uparams->getDouble( "cidd.azmith_radius", 200.0);
        radius = gd.uparams->getDouble( "cidd.azimuth_radius", radius);
	if(gd.display_projection == Mdvx::PROJ_LATLON) radius /= KM_PER_DEG_AT_EQ;
    }

      switch(gd.display_projection) {
        default:
        case Mdvx::PROJ_FLAT:
        case Mdvx::PROJ_LAMBERT_CONF:
          if(gd.range_ring_follows_data) {
            /* use km coords of data origin */
            PJGLatLon2DxDy(gd.h_win.origin_lat,gd.h_win.origin_lon,
              gd.mrec[gd.h_win.page]->h_fhdr.proj_origin_lat,
              gd.mrec[gd.h_win.page]->h_fhdr.proj_origin_lon,
              &x_dproj,&y_dproj);
          } else {
            /* Place at Displays origin */
            x_dproj = 0.0;
            y_dproj = 0.0; 
		  }
        break;

        case Mdvx::PROJ_LATLON:
          if(gd.range_ring_follows_data) {
			// Place at Data's origin
			x_dproj = gd.mrec[gd.h_win.page]->h_fhdr.proj_origin_lon;
			y_dproj = gd.mrec[gd.h_win.page]->h_fhdr.proj_origin_lat;
		  } else {
            /* Place at Display origin */
            x_dproj = gd.h_win.origin_lon;
            y_dproj = gd.h_win.origin_lat;
		  }
        break;
      }

    cur_angle = 0.0;
    while(cur_angle < M_PI) {
        xstart = x_dproj + (cos(cur_angle) * radius);
        ystart = y_dproj + (sin(cur_angle) * radius);
        xend = x_dproj - (cos(cur_angle) * radius);
        yend = y_dproj - (sin(cur_angle) * radius);
        disp_proj_to_pixel(&gd.h_win.margin,xstart,ystart,&x1,&y1);
        disp_proj_to_pixel(&gd.h_win.margin,xend,yend,&x2,&y2);

        XUDRline_clip(gd.dpy, xid, gd.legends.range_ring_color->gc,
                      x1, y1, x2, y2,
                      gd.h_win.margin.left,
                      gd.h_win.margin.top,
                      gd.h_win.img_dim.width + gd.h_win.margin.left,
                      gd.h_win.img_dim.height + gd.h_win.margin.top);

        cur_angle += interval;
    }
}
