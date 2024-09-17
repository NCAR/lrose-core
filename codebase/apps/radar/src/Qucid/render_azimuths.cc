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
 * DRAW_CAP_AZIMUTH_LINES:
 *
 */

void draw_cap_azimuth_lines(QPaintDevice *pdev)
{
  
  static double az_interval = 0.0;
  static double range = 0.0;
  
  if(az_interval == 0.0) {
    az_interval = _params.azimuth_interval;
    az_interval = fabs(az_interval);
  }
  
  if(range == 0.0) {
    range = _params.azimuth_radius;
  }

  double lon1 = gd.h_win.origin_lon;
  double lat1 = gd.h_win.origin_lat;
  if(_params.range_ring_follows_data) {
    // Place at Data's origin
    lon1 = gd.mrec[gd.h_win.page]->h_fhdr.proj_origin_lon;
    lat1 = gd.mrec[gd.h_win.page]->h_fhdr.proj_origin_lat;
  }

  int x1, y1;
  if (gd.display_projection != Mdvx::PROJ_LATLON) {
    lonlat_to_pixel(&gd.h_win.margin,lon1,lat1,&x1,&y1);
  } else {
    disp_proj_to_pixel(&gd.h_win.margin,lon1,lat1,&x1,&y1);
  }
  double az = 0.0;
  while(az < 360) {
    
    double lat2, lon2;
    PJGLatLonPlusRTheta(lat1, lon1, range, az, &lat2, &lon2);
    
    int x2, y2;
    if (gd.display_projection != Mdvx::PROJ_LATLON) {
      lonlat_to_pixel(&gd.h_win.margin,lon2,lat2,&x2,&y2);
    } else {
      disp_proj_to_pixel(&gd.h_win.margin,lon2,lat2,&x2,&y2);
    }

    XUDRline_clip(gd.dpy, xid, gd.legends.range_ring_color->gc,
                  x1, y1, x2, y2,
                  gd.h_win.margin.left,
                  gd.h_win.margin.top,
                  gd.h_win.img_dim.width + gd.h_win.margin.left,
                  gd.h_win.img_dim.height + gd.h_win.margin.top);

    az += az_interval;

  }

}

