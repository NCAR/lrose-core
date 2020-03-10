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
/*********************************************************************
 * tstorm_spdb.c
 *
 * Routines for handling the titan storm SPDB data.
 *
 * RAP, NCAR, Boulder CO
 *
 * Mike Dixon
 *
 * Sept 1997
 *
 *********************************************************************/

#include <dataport/bigend.h>
#include <rapformats/tstorm_spdb.h>
#include <rapformats/titan_grid.h>
#include <toolsa/pjg.h>
#include <toolsa/udatetime.h>
#include <toolsa/toolsa_macros.h>

/*********************************************
 * function to load forecast centroid location
 */

void tstorm_spdb_load_centroid(tstorm_spdb_header_t *header,
			       tstorm_spdb_entry_t *entry,
			       double *lat_p, double *lon_p,
			       double lead_time)
     
{

  /*
   * compute forecast centroid position
   */

  double dist = entry->speed * lead_time / 3600.0;

  if (header->grid.proj_type == TITAN_PROJ_FLAT) {
    
    /*
     * initialize projection computations
     */
    
    titan_grid_comps_t comps;
    double dirn_in_rad;
    double delta_x;
    double delta_y;
    double centroid_x, centroid_y;
    double forecast_x;
    double forecast_y;

    TITAN_init_flat(header->grid.proj_origin_lat,
                    header->grid.proj_origin_lon, 0, &comps);

    dirn_in_rad = entry->direction * DEG_TO_RAD;
    delta_x = dist * sin(dirn_in_rad);
    delta_y = dist * cos(dirn_in_rad);
    
    TITAN_latlon2xy(&comps, entry->latitude, entry->longitude,
                    &centroid_x, &centroid_y);

    forecast_x = centroid_x + delta_x;
    forecast_y = centroid_y + delta_y;

    *lat_p = forecast_y;
    *lon_p = forecast_x;

  } else {

    double forecast_lat, forecast_lon;
    
    PJGLatLonPlusRTheta(entry->latitude, entry->longitude,
			dist, entry->direction,
			&forecast_lat, &forecast_lon);
    
    *lat_p = forecast_lat;
    *lon_p = forecast_lon;

  }

}

/****************************************
 * function to load ellipse latlon points -
 *   grow argument specifies whether to 
 *   use lineal growth when extrapolating
 *   the storm
 */

void tstorm_spdb_load_growth_ellipse(tstorm_spdb_header_t *header,
			             tstorm_spdb_entry_t *entry,
			             tstorm_polygon_t *polygon,
			             double lead_time,
                                     int grow)
     
{

  int i;

  double dist, dirn_in_rad;
  double forecast_lat, forecast_lon;
  double forecast_x = 0, forecast_y = 0;
  double centroid_x, centroid_y;
  double radius, theta, dtheta;
  double delta_x, delta_y;
  double plot_lon, plot_lat;
  double maj_radius, min_radius;
  double axis_rotation;
  double phi;
  double ratio_factor;
  double wangle1 = 0.0, wangle2 = 360.0;
  double forecast_area;
  double lineal_growth;

  tstorm_pt_t *pt;
  titan_grid_comps_t comps;

  /*
   * initialize projection computations
   */
  
  if (header->grid.proj_type == TITAN_PROJ_FLAT) {
    TITAN_init_flat(header->grid.proj_origin_lat,
		  header->grid.proj_origin_lon, 0, &comps);
  }

  /*
   * compute forecast centroid position
   */

  dist = entry->speed * lead_time / 3600.0;
  dirn_in_rad = entry->direction * DEG_TO_RAD;

  if (header->grid.proj_type == TITAN_PROJ_FLAT) {
    
    delta_x = dist * sin(dirn_in_rad);
    delta_y = dist * cos(dirn_in_rad);

    TITAN_latlon2xy(&comps, entry->latitude, entry->longitude,
		  &centroid_x, &centroid_y);

    forecast_x = centroid_x + delta_x;
    forecast_y = centroid_y + delta_y;

  } else {

    PJGLatLonPlusRTheta(entry->latitude, entry->longitude,
			dist, entry->direction,
			&forecast_lat, &forecast_lon);

  }
  
  /*
   * compute lineal growth
   */

  forecast_area = entry->area + entry->darea_dt * lead_time / 3600.0;

  if (forecast_area < 1.0)
    forecast_area = 1.0;
  
  if( grow )
     lineal_growth = sqrt(forecast_area / entry->area);
  else
     lineal_growth = 1.0;
  
  /*
   * set up ellipse geom
   */
  
  maj_radius = entry->ellipse_major_radius;
  min_radius = entry->ellipse_minor_radius;
  axis_rotation = entry->ellipse_orientation;
  ratio_factor = pow(min_radius / maj_radius, 2.0) - 1.0;

  /*
   * set up start angles
   */
  
  dtheta = (((wangle2 - wangle1) * RAD_PER_DEG) /
	    (double) header->n_poly_sides);
  theta = wangle1 * RAD_PER_DEG;
  phi = theta + axis_rotation * RAD_PER_DEG;
  
  /*
   * load up ellipse latlon points
   */

  pt = polygon->pts;

  for (i = 0; i < header->n_poly_sides + 1;
       i++, pt++, theta += dtheta, phi += dtheta) {
    
    radius = min_radius / sqrt(1.0 + pow(cos(theta), 2.0) *
			       ratio_factor);
    radius *= lineal_growth;
    
    if (header->grid.proj_type == TITAN_PROJ_FLAT) {

      delta_x = radius * sin(phi);
      delta_y = radius * cos(phi);
      TITAN_xy2latlon(&comps,
		    forecast_x + delta_x, forecast_y + delta_y,
		    &plot_lat, &plot_lon);
      
    } else {

      plot_lon = forecast_lon + radius * sin(phi);
      plot_lat = forecast_lat + radius * cos(phi);

    }
    
    pt->lon = plot_lon;
    pt->lat = plot_lat;

  } /* i */

}

/****************************************
 * function to load ellipse latlon points 
 */

void tstorm_spdb_load_ellipse(tstorm_spdb_header_t *header,
			      tstorm_spdb_entry_t *entry,
			      tstorm_polygon_t *polygon,
			      double lead_time)
     
{
   tstorm_spdb_load_growth_ellipse( header, entry, polygon, lead_time, 1 );
}

/****************************************
 * function to load polygon latlon points
 * from ray data -
 *   the grow argument specifies whether
 *   to use lineal growth when extrapolating
 *   the storm
 */

void tstorm_spdb_load_growth_polygon(const tstorm_spdb_header_t *header,
			             const tstorm_spdb_entry_t *entry,
			             tstorm_polygon_t *polygon,
			             const double lead_time,
                                     const int grow)
     
{

  const ui08 *ray;
  int iray;
  double forecast_lat, forecast_lon;
  double forecast_x = 0, forecast_y = 0;
  double centroid_x, centroid_y;
  double range, theta, dtheta;
  double delta_x, delta_y;
  double plot_lon, plot_lat;
  double scale;
  double dist, dirn_in_rad;
  double forecast_area;
  double lineal_growth;
  tstorm_pt_t *pt;
  titan_grid_comps_t comps;

  /*
   * initialize projection computations
   */
  
  if (header->grid.proj_type == TITAN_PROJ_FLAT) {
    TITAN_init_flat(header->grid.proj_origin_lat,
		  header->grid.proj_origin_lon, 0, &comps);
  }

  /*
   * compute forecast centroid position
   */

  dist = entry->speed * lead_time / 3600.0;
  dirn_in_rad = entry->direction * DEG_TO_RAD;

  if (header->grid.proj_type == TITAN_PROJ_FLAT) {
    
    delta_x = dist * sin(dirn_in_rad);
    delta_y = dist * cos(dirn_in_rad);

    TITAN_latlon2xy(&comps, entry->latitude, entry->longitude,
		  &centroid_x, &centroid_y);

    forecast_x = centroid_x + delta_x;
    forecast_y = centroid_y + delta_y;

  } else {

    PJGLatLonPlusRTheta(entry->latitude, entry->longitude,
			dist, entry->direction,
			&forecast_lat, &forecast_lon);

  }

  /*
   * compute lineal growth
   */

  forecast_area = entry->area + entry->darea_dt * lead_time / 3600.0;

  if (forecast_area < 1.0)
    forecast_area = 1.0;
  
  if( grow )
     lineal_growth = sqrt(forecast_area / entry->area);
  else
     lineal_growth = 1.0;
  
  /*
   * load up polygon latlon points from rays
   */

  pt = polygon->pts;
  theta = header->poly_start_az * DEG_TO_RAD;
  dtheta = header->poly_delta_az * DEG_TO_RAD;
  scale = entry->polygon_scale;
  ray = entry->polygon_radials;

  for (iray = 0; iray < header->n_poly_sides;
       iray++, pt++, theta += dtheta, ray++) {
    
    range = ((double) *ray) * scale * lineal_growth;

    if (header->grid.proj_type == TITAN_PROJ_FLAT) {
      
      delta_x = range * sin(theta) * header->grid.dx;
      delta_y = range * cos(theta) * header->grid.dy;
      TITAN_xy2latlon(&comps,
		    forecast_x + delta_x, forecast_y + delta_y,
		    &plot_lat, &plot_lon);

    } else {

      plot_lon = forecast_lon + range * sin(theta) * header->grid.dx;
      plot_lat = forecast_lat + range * cos(theta) * header->grid.dy;

    }

    pt->lon = plot_lon;
    pt->lat = plot_lat;
    
  } /* iray */

  pt->lon = polygon->pts[0].lon;
  pt->lat = polygon->pts[0].lat;

}

/****************************************
 * function to load polygon latlon points
 * from ray data.
 */

void tstorm_spdb_load_polygon(const tstorm_spdb_header_t *header,
			      const tstorm_spdb_entry_t *entry,
			      tstorm_polygon_t *polygon,
			      const double lead_time)
     
{
   tstorm_spdb_load_growth_polygon( header, entry, polygon, lead_time, 1 );
}

/*******************************************************************
 * buffer_len()
 *
 * Compute tstorm_spdb buffer len
 *
 */

int tstorm_spdb_buffer_len(tstorm_spdb_header_t *header)

{

  int buffer_len;

  buffer_len = sizeof(tstorm_spdb_header_t) +
    header->n_entries * sizeof(tstorm_spdb_entry_t);

  return (buffer_len);

}

/*****************************
 * BigEndian swapping routines
 */

/****************************
 * tstorm_spdb_header_to_BE
 */

void tstorm_spdb_header_to_BE(tstorm_spdb_header_t *header)

{

  BE_from_array_32(header, sizeof(tstorm_spdb_header_t));

}

/****************************
 * tstorm_spdb_header_from_BE
 */

void tstorm_spdb_header_from_BE(tstorm_spdb_header_t *header)

{

  BE_to_array_32(header, sizeof(tstorm_spdb_header_t));

}

/*************************
 * tstorm_spdb_entry_to_BE
 */

void tstorm_spdb_entry_to_BE(tstorm_spdb_entry_t *entry)

{

  BE_from_array_32(entry, TSTORM_SPDB_ENTRY_NBYTES_32);

}

/***************************
 * tstorm_spdb_entry_from_BE
 */

void tstorm_spdb_entry_from_BE(tstorm_spdb_entry_t *entry)

{

  BE_to_array_32(entry, TSTORM_SPDB_ENTRY_NBYTES_32);

}

/****************************
 * tstorm_spdb_buffer_to_BE
 *
 * Swap on entire tstorm buffer to big-endian ordering
 */

void tstorm_spdb_buffer_to_BE(ui08 *buffer)

{

  int i;
  tstorm_spdb_header_t *header;
  tstorm_spdb_entry_t *entry;

  header = (tstorm_spdb_header_t *) buffer;

  entry =
    (tstorm_spdb_entry_t *) (buffer + sizeof(tstorm_spdb_header_t));

  for (i = 0; i < header->n_entries; i++, entry++) {
    tstorm_spdb_entry_to_BE(entry);
  }

  tstorm_spdb_header_to_BE(header);
}

/****************************
 * tstorm_spdb_buffer_from_BE
 *
 * Swap on entire tstorm buffer to host byte ordering
 */

void tstorm_spdb_buffer_from_BE(ui08 *buffer)

{

  int i;
  tstorm_spdb_header_t *header;
  tstorm_spdb_entry_t *entry;

  header = (tstorm_spdb_header_t *) buffer;
  tstorm_spdb_header_from_BE(header);

  entry =
    (tstorm_spdb_entry_t *) (buffer + sizeof(tstorm_spdb_header_t));

  for (i = 0; i < header->n_entries; i++, entry++) {
    tstorm_spdb_entry_from_BE(entry);
  }

}

/****************************
 * tstorm_spdb_print_header()
 *
 * Print out header
 */

void tstorm_spdb_print_header(FILE *out,
			      const char *spacer,
			      tstorm_spdb_header_t *header)

{

  fprintf(out, "%sTITAN SPDB STORM HEADER\n", spacer);

  fprintf(out, "%s  Time: %s\n", spacer, utimstr(header->time));

  fprintf(out, "%s  n_entries: %d\n", spacer, header->n_entries);

  if (header->grid.proj_type == TITAN_PROJ_FLAT) {
    fprintf(out, "%s  Proj_type: %s\n", spacer, "FLAT");
  } else {
    fprintf(out, "%s  Proj_type: %s\n", spacer, "LATLON");
  }
    
  fprintf(out, "%s  grid_dx: %g\n", spacer, header->grid.dx);
  fprintf(out, "%s  grid_dy: %g\n", spacer, header->grid.dy);
  fprintf(out, "%s  low_dbz_threshold: %g\n",
	  spacer, header->low_dbz_threshold);

  fprintf(out, "%s  n_poly_sides: %d\n", spacer, (int) header->n_poly_sides);
  fprintf(out, "%s  poly_start_az: %g\n", spacer, header->poly_start_az);
  fprintf(out, "%s  poly_delta_az: %g\n", spacer, header->poly_delta_az);

}

/****************************
 * tstorm_spdb_print_entry()
 *
 * Print out entry
 */

void tstorm_spdb_print_entry(FILE *out,
			     const char *spacer,
			     tstorm_spdb_header_t *header,
			     tstorm_spdb_entry_t *entry)
     
{

  int i;

  fprintf(out, "%sTITAN SPDB STORM ENTRY\n", spacer);
  
  fprintf(out, "%s  longitude: %g\n", spacer, entry->longitude);
  fprintf(out, "%s  latitude: %g\n", spacer, entry->latitude);
  fprintf(out, "%s  direction: %g\n", spacer, entry->direction);
  fprintf(out, "%s  speed: %g\n", spacer, entry->speed);

  fprintf(out, "%s  simple_track_num: %d\n",
	  spacer, (int) entry->simple_track_num);
  fprintf(out, "%s  complex_track_num: %d\n",
	  spacer, (int) entry->complex_track_num);

  fprintf(out, "%s  area: %g\n", spacer, entry->area);
  fprintf(out, "%s  darea_dt: %g\n", spacer, entry->darea_dt);

  fprintf(out, "%s  top: %g\n", spacer, entry->top);
  
  fprintf(out, "%s  ellipse_orientation: %g\n",
	  spacer, entry->ellipse_orientation);
  fprintf(out, "%s  ellipse_minor_radius: %g\n",
	  spacer, entry->ellipse_minor_radius);
  fprintf(out, "%s  ellipse_major_radius: %g\n",
	  spacer, entry->ellipse_major_radius);
  
  fprintf(out, "%s  forecast_valid: %s\n", spacer,
	  entry->forecast_valid? "TRUE" : "FALSE");

  fprintf(out, "%s  dbz_max: %d\n", spacer, entry->dbz_max);

  fprintf(out, "%s  intensity_trend: %d\n", spacer, entry->intensity_trend);
  fprintf(out, "%s  size_trend: %d\n", spacer, entry->size_trend);

  fprintf(out, "%s  algorithm_value: %f\n", spacer, entry->algorithm_value);
  
  fprintf(out, "%s  Polygon rays:\n", spacer);
  for (i = 0; i < header->n_poly_sides; i++) {
    fprintf(out, "%s    ray %d: %g\n", spacer, i,
	    entry->polygon_radials[i] * entry->polygon_scale);
  }

}

/*****************************
 * tstorm_spdb_print_polygon()
 *
 * Print out polygon
 */

void tstorm_spdb_print_polygon(FILE *out,
			       const char *spacer,
			       tstorm_polygon_t *polygon,
			       int n_poly_sides)

{

  int i;

  fprintf(out, "%sPolygon for TITAN storm\n", spacer);
  fprintf(out, "%s  Npoints : %d\n", spacer, n_poly_sides);
  fprintf(out, "%s  Polygon points:\n", spacer);
  for (i = 0; i < n_poly_sides; i++) {
    fprintf(out, "%s    %g %g\n", spacer, 
	    polygon->pts[i].lon, polygon->pts[i].lat);
  }
  fprintf(out, "\n");

}

/**************************
 * tstorm_spdb_print_buffer
 *
 * Print an entire buffer
 */

void tstorm_spdb_print_buffer(FILE *out,
			      const char *spacer,
			      ui08 *buffer)

{

  int i;
  tstorm_spdb_header_t *header;
  tstorm_spdb_entry_t *entry;

  header = (tstorm_spdb_header_t *) buffer;
  tstorm_spdb_print_header(out, spacer, header);

  entry =
    (tstorm_spdb_entry_t *) (buffer + sizeof(tstorm_spdb_header_t));

  for (i = 0; i < header->n_entries; i++, entry++) {
    tstorm_spdb_print_entry(out, spacer, header, entry);
  }

}

