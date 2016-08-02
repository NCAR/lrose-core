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
/*********************************************************************
 * add_to_buffer.cc
 *
 * Routines for adding objects to the buffer
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1998
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "vergrid_spdb2symprod.hh"
#include <toolsa/pjg.h>

// file scope

#define NSEGMENTS 36

static double CenterLat;
static double CenterLon;
static double TimeLat;
static double TimeLon;
static double Radius;
static double SegLat[NSEGMENTS+1];
static double SegLon[NSEGMENTS+1];

static void add_circle(symprod_product_t *prod,
		       int start_index,
		       int end_index,
		       char *color);
		     
static void add_text(symprod_product_t *prod,
		     char *color,
		     int vert_align,
		     char *label,
		     double percent_coverage);

//////////////////////////////////
// compute circle segment geometry

void compute_geometry(double center_lat,
		      double center_lon,
		      double radius)

{

  CenterLat = center_lat;
  CenterLon = center_lon;
  Radius = radius;

  double dtheta = 360.0 / NSEGMENTS;
  
  for (int i = 0; i < NSEGMENTS + 1; i++) {

    double theta = i * dtheta;

    PJGLatLonPlusRTheta(CenterLat, CenterLon,
			Radius, theta,
			&SegLat[i], &SegLon[i]);

  }

  PJGLatLonPlusRTheta(CenterLat, CenterLon,
		      Radius, 180.0,
		      &TimeLat, &TimeLon);

}

////////////////////
// add_truth_circle

void add_truth_circle(symprod_product_t *prod)

{

  add_circle(prod, 0, NSEGMENTS - 1, Params.truth_color);
  
}
		     
//////////////////////
// add_forecast_circle

void add_forecast_circle(symprod_product_t *prod)

{

  add_circle(prod, 1, NSEGMENTS, Params.forecast_color);
  
}
		     
/////////////////
// add_truth_text

void add_truth_text(symprod_product_t *prod,
		    double percent_coverage)

{

  add_text(prod,
	   Params.truth_color,
	   SYMPROD_TEXT_VERT_ALIGN_TOP,
	   "Now",
	   percent_coverage);
  
}
		     
////////////////////
// add_forecast_text

void add_forecast_text(symprod_product_t *prod,
		       double percent_coverage)

{

  add_text(prod,
	   Params.forecast_color,
	   SYMPROD_TEXT_VERT_ALIGN_BOTTOM,
	   "Fcast",
	   percent_coverage);
  
}
		     
//////////////////
// add_time_text()

void add_time_text(symprod_product_t *prod,
		   time_t forecast_time)

{

  // form text string

  char text[64];
  sprintf(text, "Ftime: %s", utimstr(forecast_time));

  SYMPRODaddText(prod, Params.time_color,
		 Params.text_background_color,
		 TimeLat, TimeLon,
		 0, 0,
		 SYMPROD_TEXT_VERT_ALIGN_BOTTOM,
		 SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		 0, Params.text_font,
		 text);

}

/////////////
// add_circle

static void add_circle(symprod_product_t *prod,
		       int start_index,
		       int end_index,
		       char *color)
		     
{

  static MEMbuf *mbuf_points = NULL;

  /*
   * initialize mbuf
   */

  if (mbuf_points == NULL)
    mbuf_points = MEMbufCreate();
  else
    MEMbufReset(mbuf_points);
  
  // load polyline data into mbuf_points
  
  for (int i = start_index; i < end_index; i += 2) {

    symprod_wpt_t point;

    point.lon = SegLon[i];
    point.lat = SegLat[i];
    MEMbufAdd(mbuf_points, &point, sizeof(point));

    point.lon = SegLon[i+1];
    point.lat = SegLat[i+1];
    MEMbufAdd(mbuf_points, &point, sizeof(point));

    point.lon = SYMPROD_WPT_PENUP;
    point.lat = SYMPROD_WPT_PENUP;
    MEMbufAdd(mbuf_points, &point, sizeof(point));

  } // i

  // add the polyline
  
  SYMPRODaddPolyline(prod, color,
		     convert_line_type_param(Params.suggested_line_type),
		     Params.suggested_line_width,
		     convert_capstyle_param(Params.suggested_capstyle),
		     convert_joinstyle_param(Params.suggested_joinstyle),
		     FALSE, SYMPROD_FILL_NONE,
		     (symprod_wpt_t *) MEMbufPtr(mbuf_points),
		     MEMbufLen(mbuf_points) / sizeof(symprod_wpt_t));

  return;
  
}

/////////////
// add_text()

static void add_text(symprod_product_t *prod,
		     char *color,
		     int vert_align,
		     char *label,
		     double percent_coverage)

{

  // form text string

  char text[64];
  sprintf(text, "%s:%.0f%%", label, percent_coverage);

  SYMPRODaddText(prod, color,
		 Params.text_background_color,
		 CenterLat, CenterLon,
		 0, 0,
		 vert_align,
		 SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		 0, Params.text_font,
		 text);

}

