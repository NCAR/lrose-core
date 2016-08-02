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
 * tdata_product.c
 *
 * Routines for handling the tdata product generation.
 *
 * RAP, NCAR, Boulder CO
 *
 * Mike Dixon
 *
 * Sept 1997
 *
 *********************************************************************/

#include <dataport/bigend.h>
#include <titan/tdata_server.h>
#include <toolsa/toolsa_macros.h>

#define LIMIT_VAL(x, low, high) if ((x) < (low)) (x) = (low); else if ((x) > (high)) (x) = (high)

/*
 * file scope
 */

static int Zero_growth_forecast;
static int Send_invalid_forecasts;
static int Plot_current;
static int Plot_forecast;
static int Plot_trend;
static int Speed_knots;
static int Speed_round;
static int Fixed_length_arrows;
static int Forecast_lead_time;

/********
 * init()
 *
 * initialize file scope variables
 */

void tdata_prod_init(int zero_growth_forecast,
		     int send_invalid_forecasts,
		     int forecast_lead_time,
		     int plot_current,
		     int plot_forecast,
		     int plot_trend,
		     int speed_knots,
		     int speed_round,
		     int fixed_length_arrows)

{

  Zero_growth_forecast = zero_growth_forecast;
  Send_invalid_forecasts = send_invalid_forecasts;
  Forecast_lead_time = forecast_lead_time;
  Plot_current = plot_current;
  Plot_forecast = plot_forecast;
  Plot_trend = plot_trend;
  Speed_knots = speed_knots;
  Speed_round = speed_round;
  Fixed_length_arrows = fixed_length_arrows;

}

/*******************************************************************
 * load_header()
 *
 * loads up product header struct
 */

void tdata_prod_load_header(tdata_product_header_t *header,
			    storm_file_params_t *sparams,
			    titan_grid_t *grid,
			    track_file_handle_t *t_handle,
			    si32 dtime,
			    si32 n_sent,
			    int plot_ellipse,
			    int plot_polygon,
			    int ellipse_included,
			    int polygon_included,
			    si32 n_poly_sides)

{

  header->time = dtime;
  header->grid_type = grid->proj_type;

  if (header->grid_type == TITAN_PROJ_FLAT) {
    header->grid_scalex = 10000;
    header->grid_scaley = 10000;
  } else {
    header->grid_scalex = 1000000;
    header->grid_scaley = 1000000;
  }
    
  header->grid_dx =
    (si32) floor (grid->dx * header->grid_scalex + 0.5);
  header->grid_dy =
    (si32) floor (grid->dy * header->grid_scaley + 0.5);

  header->dbz_threshold = (si32) (sparams->low_dbz_threshold + 0.5);
  header->min_storm_size = (si32) (sparams->min_storm_size + 0.5);

  header->forecast_lead_time = Forecast_lead_time;
  
  header->n_complex_tracks = t_handle->header->n_complex_tracks;
  header->n_entries = n_sent;
  header->plot_current = Plot_current;
  header->plot_forecast = Plot_forecast;
  header->plot_ellipse = plot_ellipse;
  header->plot_polygon = plot_polygon;
  header->plot_trend = Plot_trend;
  header->ellipse_included = ellipse_included;
  header->polygon_included = polygon_included;
  header->speed_knots = Speed_knots;
  header->speed_round = Speed_round;
  header->fixed_length_arrows = Fixed_length_arrows;
  header->n_poly_sides = n_poly_sides;

  if (header->grid_type == TITAN_PROJ_FLAT) {
    header->poly_start_az =
      (si32) ((sparams->poly_start_az + grid->proj_params.flat.rotation) *
	      ANGLE_SCALE + 0.5);
  } else {
    header->poly_start_az =
      (si32) (sparams->poly_start_az * ANGLE_SCALE + 0.5);
  }
  header->poly_delta_az =
    (si32) (sparams->poly_delta_az * ANGLE_SCALE + 0.5);

  header->angle_scale = ANGLE_SCALE;
  header->short_angle_scale = SHORT_ANGLE_SCALE;
  header->speed_scale = SPEED_SCALE;
  header->darea_dt_scale = DAREA_DT_SCALE;
  header->flat_proj_ellipse_scale = FLAT_PROJ_ELLIPSE_SCALE;
  header->latlon_proj_ellipse_scale = LATLON_PROJ_ELLIPSE_SCALE;
  header->radial_multiplier_scale = RADIAL_MULTIPLIER_SCALE;

  BE_from_array_32((ui32 *) header,
		   (ui32) sizeof(tdata_product_header_t));
  
}  

/*******************************************************************
 * load_entry()
 *
 * loads up product entry struct
 *
 * Returns 1 if entry valid, 0 otherwise
 */

int tdata_prod_load_entry(tdata_product_entry_t *entry,
			  track_file_entry_t *file_entry,
			  storm_file_global_props_t *gprops,
			  track_file_forecast_props_t *fprops,
			  titan_grid_comps_t *comps,
			  double lead_time_hr,
			  double vol_threshold)

{
  
  double speed;
  double direction;
  double longitude, latitude;
  double proj_area;
  double forecast_area;
  double dproj_area_dt;
  double dvol_dt, volume, forecast_volume;
  double mass, dmass_dt, norm_dmass_dt;
  double area, darea_dt, norm_darea_dt;
  
  /*
   * check for valid forecast
   */

  entry->forecast_valid = TRUE;

  if (!file_entry->forecast_valid) {
    if (!Send_invalid_forecasts)
      return (0);
    entry->forecast_valid = FALSE;
  }

  entry->simple_track_num = file_entry->simple_track_num;
  entry->complex_track_num = file_entry->complex_track_num;
  entry->top = (si08) (gprops->top + 0.5);
  
  /*
   * check to see if volume at half of forecast time will be below
   * threshold
   */

  volume = gprops->volume;
  dvol_dt = fprops->volume;
  forecast_volume = volume + dvol_dt * lead_time_hr * 0.5;

  if (forecast_volume < 0.0) {
    entry->forecast_valid = FALSE;
    if (!Send_invalid_forecasts)
      return (0);
  }

  /*
   * check area forecast
   */

  proj_area = gprops->proj_area;
  dproj_area_dt = fprops->proj_area;

  forecast_area =
    proj_area + dproj_area_dt * lead_time_hr * 0.5;

  if (forecast_area <= 0.0) {
    entry->forecast_valid = FALSE;
    if (!Send_invalid_forecasts)
      return (0);
  }

  /*
   * lat / lon
   */

  TITAN_xy2latlon(comps,
		gprops->proj_area_centroid_x,
		gprops->proj_area_centroid_y,
		&latitude, &longitude);
  
  entry->longitude = (si32) (longitude * ANGLE_SCALE + 0.5);
  entry->latitude = (si32) (latitude * ANGLE_SCALE + 0.5);

  /*
   * speed
   */

  speed = fprops->smoothed_speed;
  speed = MIN(speed, 65535 / SPEED_SCALE);
  entry->speed = (ui16) (speed * SPEED_SCALE + 0.5);
  
  /*
   * direction
   */
  
  direction = fmod(fprops->smoothed_direction + comps->rotation,
		   360.0);
  
  if (direction < 0.0) {
    direction += 360.0;
  }

  entry->direction = (ui16) (direction * SHORT_ANGLE_SCALE + 0.5);

  /*
   * set intensity trend
   */

  mass = gprops->mass;
  dmass_dt = fprops->mass;
  norm_dmass_dt = dmass_dt / mass;
  
  if (norm_dmass_dt < -0.5) {
    entry->intensity_trend = -1;
  } else if (norm_dmass_dt > 0.5) {
    entry->intensity_trend = 1;
  } else {
    entry->intensity_trend = 0;
  }

  /*
   * set size trend
   */

  area = gprops->proj_area;
  darea_dt = fprops->proj_area;
  norm_darea_dt = darea_dt / area;
  
  if (norm_darea_dt < -0.5) {
    entry->size_trend = -1;
  } else if (norm_darea_dt > 0.5) {
    entry->size_trend = 1;
  } else {
    entry->size_trend = 0;
  }

  /*
   *  set to network byte order
   */

  BE_from_array_32((ui32 *) &entry->longitude,
		   (ui32) sizeof(si32));
  BE_from_array_32((ui32 *) &entry->latitude,
		   (ui32) sizeof(si32));
  BE_from_array_16((ui16 *) &entry->direction,
		   (ui16) sizeof(ui16));
  BE_from_array_16((ui16 *) &entry->speed,
		   (ui16) sizeof(ui16));
  BE_from_array_16((ui16 *) &entry->simple_track_num,
		   (ui16) sizeof(ui16));
  BE_from_array_16((ui16 *) &entry->complex_track_num,
		   (ui16) sizeof(ui16));
  
  return (1);
  
}

/*******************************************************************
 * load_ellipse()
 *
 * loads up product ellipse struct
 *
 */

void tdata_prod_load_ellipse(tdata_product_ellipse_t *ellipse,
			     storm_file_global_props_t *gprops,
			     track_file_forecast_props_t *fprops,
			     titan_grid_comps_t *comps)

{
  
  double major_radius, minor_radius;
  double orientation;
  double proj_area;
  double dproj_area_dt;
  double norm_darea_dt;
  
  /*
   * area
   */

  if (Zero_growth_forecast) {

    ellipse->norm_darea_dt = 0;

  } else {
    
    proj_area = gprops->proj_area;
    dproj_area_dt = fprops->proj_area;
    
    norm_darea_dt = dproj_area_dt / proj_area;
    LIMIT_VAL(norm_darea_dt,
		  -32767/DAREA_DT_SCALE, 32767/DAREA_DT_SCALE);
    
    ellipse->norm_darea_dt = (si16) (norm_darea_dt *
				      DAREA_DT_SCALE + 0.5);

  } /* if (Zero_growth_forecast) */

  /*
   * orientation
   */

  if (comps->proj_type == TITAN_PROJ_FLAT) {
    orientation = (fmod(gprops->proj_area_orientation, 360.0) +
		   comps->rotation);
  } else {
    orientation = fmod(gprops->proj_area_orientation, 360.0);
  }

  if (orientation < 0.0) {
    orientation += 360.0;
  }

  ellipse->orientation = (ui16) (orientation * SHORT_ANGLE_SCALE + 0.5);

  /*
   * radii
   */
  
  major_radius = gprops->proj_area_major_radius;
  minor_radius = gprops->proj_area_minor_radius;

  if (comps->proj_type == TITAN_PROJ_FLAT) {
    major_radius = MIN(major_radius, 65535/FLAT_PROJ_ELLIPSE_SCALE);
    minor_radius = MIN(minor_radius, 65535/FLAT_PROJ_ELLIPSE_SCALE);
    ellipse->major_radius =
      (ui16) (major_radius * FLAT_PROJ_ELLIPSE_SCALE + 0.5);
    ellipse->minor_radius =
      (ui16) (minor_radius * FLAT_PROJ_ELLIPSE_SCALE + 0.5);
  } else {
    major_radius = MIN(major_radius, 65535/LATLON_PROJ_ELLIPSE_SCALE);
    minor_radius = MIN(minor_radius, 65535/LATLON_PROJ_ELLIPSE_SCALE);
    ellipse->major_radius =
      (ui16) (major_radius * LATLON_PROJ_ELLIPSE_SCALE + 0.5);
    ellipse->minor_radius =
      (ui16) (minor_radius * LATLON_PROJ_ELLIPSE_SCALE + 0.5);
  }

  /*
   * set to network byte order
   */

  BE_from_array_16((ui16 *) &ellipse->norm_darea_dt,
		   (ui16) sizeof(ui16));
  BE_from_array_16((ui16 *) &ellipse->orientation,
		   (ui16) sizeof(ui16));
  BE_from_array_16((ui16 *) &ellipse->minor_radius,
		   (ui16) sizeof(ui16));
  BE_from_array_16((ui16 *) &ellipse->major_radius,
		   (ui16) sizeof(ui16));
  
  return;
  
}

/*******************************************************************
 * load_polygon()
 *
 * loads up product polygon struct
 *
 */

void tdata_prod_load_polygon(tdata_product_polygon_t *polygon,
			     storm_file_global_props_t *gprops,
			     si32 n_poly_sides)

{

  ui08 *rp;
  si32 i;
  fl32 *radial;
  double length_scale;
  double max_length = 0.0;
  
  /*
   * get max radial length
   */

  radial = gprops->proj_area_polygon;

  for (i = 0; i < n_poly_sides; i++, radial++) {
    max_length = MAX(max_length, *radial);
  }
  
  length_scale = max_length / 255.0;

  /*
   * set vals in struct
   */

  polygon->radial_multiplier =
    (si32) (length_scale * RADIAL_MULTIPLIER_SCALE + 0.5);
  BE_from_array_32((ui32 *) &polygon->radial_multiplier,
		   (ui32) sizeof(si32));
  
  radial = gprops->proj_area_polygon;
  rp = polygon->radials;

  for (i = 0; i < n_poly_sides; i++, rp++, radial++) {
    *rp = (ui08) (*radial / length_scale + 0.5);
  }

  return;

}

/*******************************************************************
 * buffer_len()
 *
 * Compute tdata_product buffer len
 *
 */

int tdata_prod_buffer_len(int plot_ellipse, int plot_polygon,
			  int n_poly_sides, int n_entries)

{

  int entry_len;
  int polygon_product_len;
  int buffer_len;
  int ellipse_included = FALSE;

  /*
   * set lens of buffer elements
   */

  entry_len = sizeof(tdata_product_entry_t);
  polygon_product_len = sizeof(si32) + n_poly_sides * sizeof(ui08);
  
  if (plot_ellipse) {
    entry_len += sizeof(tdata_product_ellipse_t);
    ellipse_included = TRUE;
  }
  
  if (plot_polygon) {
    if (!ellipse_included) {
      entry_len += sizeof(tdata_product_ellipse_t);
    }
    entry_len += polygon_product_len;
  }

  buffer_len = sizeof(tdata_product_header_t) + n_entries * entry_len;

  return (buffer_len);

}


