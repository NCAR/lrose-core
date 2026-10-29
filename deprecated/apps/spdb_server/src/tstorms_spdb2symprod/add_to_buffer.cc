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
 * Sept 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "tstorms_spdb2symprod.h"

/*
 * file scope prototypes
 */

static void add_polygon(symprod_product_t *prod,
			tstorm_polygon_t *tstorm_polygon,
			int npoints_polygon,
			char *color,
			int dashed);

/*********************************************************************
 * add_current_polygon()
 *
 *   Add a SYMPROD polyline object for the current storm shape to the
 *   product buffer.
 *
 */

void add_current_polygon(symprod_product_t *prod,
			 tstorm_spdb_header_t *header,
			 tstorm_spdb_entry_t *entry)

{
  
  tstorm_polygon_t tstorm_polygon;
  int npoints_polygon = header->n_poly_sides + 1;

  // load up polygon, 0 lead time (current pos)

  if (Params.storm_shape == POLYGON_SHAPE) {
    if (Params.hull_smooth)
      tstorm_hull_smooth(header, entry,
                         Params.inner_bnd_multiplier,
                         Params.outer_bnd_multiplier,
                         &tstorm_polygon,
                         &npoints_polygon,
                         0,
                         0);
    else
      tstorm_spdb_load_polygon(header, entry,
			       &tstorm_polygon,
			       0.0);

  } else {
    tstorm_spdb_load_ellipse(header, entry,
                             &tstorm_polygon,
                             0.0);
  }

  // add polygon to prod struct  

  add_polygon(prod, &tstorm_polygon,
	      npoints_polygon,
	      Params.current_color, FALSE);

}


/*********************************************************************
 * add_forecast_polygon()
 *
 *   Add a SYMPROD polyline object for the forecast storm shape to the
 *   product buffer.
 *
 */

void add_forecast_polygon(symprod_product_t *prod,
			  tstorm_spdb_header_t *header,
			  tstorm_spdb_entry_t *entry)
  
{
  
  tstorm_polygon_t tstorm_polygon;
  int npoints_polygon = header->n_poly_sides + 1;

  // load up polygon, 0 lead time (current pos)

  if (Params.storm_shape == POLYGON_SHAPE) {
    if (Params.hull_smooth)
      tstorm_hull_smooth(header, entry,
                         Params.inner_bnd_multiplier,
                         Params.outer_bnd_multiplier,
                         &tstorm_polygon,
                         &npoints_polygon,
                         Params.forecast_lead_time,
                         0);
    else
      tstorm_spdb_load_polygon(header, entry,
			       &tstorm_polygon,
			       Params.forecast_lead_time);
  } else {
    tstorm_spdb_load_ellipse(header, entry,
			     &tstorm_polygon,
			     Params.forecast_lead_time);
  }

  // add polygon to prod struct 

  add_polygon(prod, &tstorm_polygon,
	      npoints_polygon,
	      Params.forecast_color, Params.forecast_dashed);

}

/*********************************************************************
 * add_polygon()
 *
 *   Add a SYMPROD polyline object for the storm shape to the
 *   product buffer. Dashed for forecast shapes.
 */

static void add_polygon(symprod_product_t *prod,
			tstorm_polygon_t *tstorm_polygon,
			int npoints_polygon,
			char *color,
			int dashed)

{

  static MEMbuf *mbuf_points = NULL;

  int ipt;
  tstorm_pt_t *pt;

  /*
   * initialize mbuf
   */

  if (mbuf_points == NULL)
    mbuf_points = MEMbufCreate();
  else
    MEMbufReset(mbuf_points);
  
  // load polyline data into mbuf_points

  pt = tstorm_polygon->pts;
  for (ipt = 0; ipt < npoints_polygon; ipt++, pt++) {

    symprod_wpt_t point;
    double prev_lat, prev_lon;
    double dlat, dlon;
    
    if (dashed && ipt > 0) {

      dlon = pt->lon - prev_lon;
      dlat = pt->lat - prev_lat;

      point.lon = prev_lon + 0.25 * dlon;
      point.lat = prev_lat + 0.25 * dlat;
      MEMbufAdd(mbuf_points, &point, sizeof(point));

      point.lon = SYMPROD_WPT_PENUP;
      point.lat = SYMPROD_WPT_PENUP;
      MEMbufAdd(mbuf_points, &point, sizeof(point));

      point.lon = prev_lon + 0.75 * dlon;
      point.lat = prev_lat + 0.75 * dlat;
      MEMbufAdd(mbuf_points, &point, sizeof(point));

    }
    
    point.lat = pt->lat;
    point.lon = pt->lon;
    MEMbufAdd(mbuf_points, &point, sizeof(point));
    
    prev_lat = pt->lat;
    prev_lon = pt->lon;

  } /* endfor - ipt */

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

/*********************************************************************
 * add_forecast_vector()
 *
 *   Add an arrow to show forecast movement vector
 *
 */

void add_forecast_vector(symprod_product_t *prod,
			tstorm_spdb_entry_t *entry)
  
{

  double length;

  if (Params.fixed_length_arrows) {
    length = Params.arrow_shaft_length;
  } else {
    length = entry->speed * Params.forecast_lead_time / 3600.0;
  }
  
  SYMPRODaddArrowStartPt(prod,
			 Params.vector_color,
			 convert_line_type_param(Params.suggested_line_type),
			 Params.suggested_line_width,
			 convert_capstyle_param(Params.suggested_capstyle),
			 convert_joinstyle_param(Params.suggested_joinstyle),
			 entry->latitude,
			 entry->longitude,
			 length, entry->direction,
			 Params.arrow_head_len,
			 Params.arrow_head_half_angle);

}

/*********************************************************************
 * add_text()
 *
 *   Add a SYMPROD text object for speed and growth
 */

void add_text(symprod_product_t *prod,
	      tstorm_spdb_entry_t *entry)

{

  if (!Params.plot_trend && !Params.plot_speed) {
    return;
  }
  
  // form text string

  int plot_text = FALSE;
  int plot_top = FALSE;
  char speed_text[64];
  char trend_text[64];
  char top_text[64];
  
  speed_text[0] = '\0';
  trend_text[0] = '\0';
  top_text[0] = '\0';
  
  // load speed text

  double speed_kmh = entry->speed;
  double speed;
  int rounded_speed;

  if (Params.speed_units == SPEED_KNOTS) {
    speed = speed_kmh / 1.852;
  } else if (Params.speed_units == SPEED_MPH) {
    speed = speed_kmh / 1.6;
  } else {
    speed = speed_kmh;
  }
   
  if (Params.speed_round) {
    rounded_speed = (int) ((speed + 2.5) / 5.0) * 5;
  } else {
    rounded_speed = (int) (speed + 0.5);
  }

  if (rounded_speed > 0) {
    sprintf(speed_text, "%d", rounded_speed);
    plot_text = TRUE;
  }
    
  // load trend text
  
  if (Params.plot_trend) {

    // Intensity_trend can be -1 (decreasing), 0, or 1 (increasing).
    // Same for size trend. We switch on the sum of the two.

    int sum_trend = entry->intensity_trend + entry->size_trend;

    if (sum_trend > 0) {
      sprintf(trend_text, "+");
      plot_text = TRUE;
    } else if (sum_trend < 0) {
      sprintf(trend_text, "-");
      plot_text = TRUE;
    }
      
  }

  // Load top text

  if (Params.plot_top && entry->top > 2.0) {
    if (Params.top_km)
      sprintf(top_text, "%d", (int)(entry->top + 0.5));
    else {
      double top_100s_ft =
	entry->top / KM_PER_MI * FT_PER_MI / 100.0;
      
      sprintf(top_text, "%d", (int)(top_100s_ft + 0.5));
    }
    
    plot_text = TRUE;
    plot_top = TRUE;
  }
  
  if (plot_text) {

    char text_string[128];

    if (plot_top)
      sprintf(text_string, "%s%s/%s", speed_text, trend_text, top_text);
    else
      sprintf(text_string, "%s%s", speed_text, trend_text);

    SYMPRODaddText(prod, Params.text_color,
		   Params.text_background_color,
		   entry->latitude, entry->longitude,
		   0, 0,
		   SYMPROD_TEXT_VERT_ALIGN_CENTER,
		   SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		   0, Params.text_font,
		   text_string);

  }

}
