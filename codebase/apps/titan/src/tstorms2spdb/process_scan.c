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

#include "tstorms2spdb.h"

static ui08 *allocate_output_buffer(int buffer_len);

static void _load_header(tstorm_spdb_header_t *header,
			 storm_file_params_t *sparams,
			 titan_grid_t *grid,
			 track_file_handle_t *t_handle,
			 si32 dtime,
			 si32 n_entries);
     
static void _load_entry(tstorm_spdb_header_t *header,
			tstorm_spdb_entry_t *entry,
			track_file_entry_t *file_entry,
			storm_file_global_props_t *gprops,
			track_file_forecast_props_t *fprops,
			titan_grid_comps_t *comps);
     
/****************************************************************************
 * process_scan.c
 *
 * Process scan of a given storm file
 *
 * July 1995
 *
 ****************************************************************************/

int process_scan(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle,
		 int this_scan_num,
		 int n_scans,
		 date_time_t *scan_times)

{
  ui08 *output_buffer;

  int n_entries;
  int ientry;
  int buffer_len;

  si32 valid_time, expire_time;
  
  date_time_t *this_scan_time = scan_times + this_scan_num;

  track_file_entry_t *file_entry;
  storm_file_scan_header_t *scan;
  storm_file_global_props_t *gprops;
  track_file_forecast_props_t *fprops;
  storm_file_params_t *sparams = &s_handle->header->params;
  tstorm_spdb_header_t tstorm_header;
  tstorm_spdb_entry_t *tstorm_entry;

  titan_grid_comps_t grid_comps;
  
  PMU_auto_register("In process_scan");

  /*
   * read in track file scan entries
   */

  if (RfReadTrackScanEntries(t_handle, this_scan_num,
			     "process_scan") != R_SUCCESS) {
    return (-1);
  }
  
  /*
   * read in storm file scan
   */
  
  if (RfReadStormScan(s_handle, this_scan_num, "process_scan") != R_SUCCESS) {
    return (-1);
  }
  
  /*
   * initialize projection
   */
  
  TITAN_init_proj(&s_handle->scan->grid, &grid_comps);
  
  /*
   * initialize
   */

  scan = s_handle->scan;
  n_entries = t_handle->scan_index[this_scan_num].n_entries;

  /*
   * load header
   */
  
  _load_header(&tstorm_header, sparams,
	       &s_handle->scan->grid,
	       t_handle,
	       this_scan_time->unix_time,
	       n_entries);

  /*
   * allocate buffer
   */
  
  buffer_len = tstorm_spdb_buffer_len(&tstorm_header);
  output_buffer = allocate_output_buffer(buffer_len);

  /*
   *  copy header into buffer, put into BE ordering
   */
  
  memcpy(output_buffer, &tstorm_header, sizeof(tstorm_spdb_header_t));
  tstorm_spdb_header_to_BE((tstorm_spdb_header_t *) output_buffer);
  
  /*
   * loop through the entries
   */
  
  file_entry = t_handle->scan_entries;
  tstorm_entry =
    (tstorm_spdb_entry_t *) (output_buffer + sizeof(tstorm_spdb_header_t));
  
  for (ientry = 0; ientry < n_entries;
       ientry++, file_entry++, tstorm_entry++) {

    fprops = &file_entry->dval_dt;
    gprops = s_handle->gprops + file_entry->storm_num;
    
    /*
     * load up entry struct
     */
    
    _load_entry(&tstorm_header, tstorm_entry,
		file_entry, gprops, fprops, &grid_comps);
    
    /*
     *  set entry to BE ordering
     */

    tstorm_spdb_entry_to_BE(tstorm_entry);

  } /* ientry */
  
  /*
   * write buffer to output
   */
  
  valid_time = this_scan_time->unix_time;
  
  if (this_scan_num == n_scans - 1) {
    expire_time = valid_time +
      (valid_time - scan_times[this_scan_num - 1].unix_time);
  } else {
    expire_time = scan_times[this_scan_num + 1].unix_time;
  }
  
  if(output_write(valid_time, expire_time, output_buffer, buffer_len)) {
    fprintf(stderr, "ERROR - %s:process_scan\n", Glob->prog_name);
    fprintf(stderr, "Cannot write output buffer\n");
    return (-1);
  }

  return (0);

}

/*****************************************************************
 * allocate_output_buffer()
 */

static ui08 *allocate_output_buffer(int buffer_len)
     
{

  static ui08 *buffer = NULL;
  static int size_allocated = 0;
  
  /*
   * allocate array for entries
   */
  
  if (size_allocated < buffer_len) {

    if (buffer == NULL) {
      buffer = (ui08 *) umalloc ((ui32) buffer_len);
    } else {
      buffer = (ui08 *)
	urealloc ((char *) buffer,(ui32) buffer_len);
    } /* if (buffer == NULL) */

    size_allocated = buffer_len;
    
  } /* if (size_allocated < buffer_len) */
  
  return (buffer);

}


/*******************************************************************
 * load_header()
 *
 * loads up header struct
 */

static void _load_header(tstorm_spdb_header_t *header,
			 storm_file_params_t *sparams,
			 titan_grid_t *grid,
			 track_file_handle_t *t_handle,
			 si32 dtime,
			 si32 n_entries)
     
{

  header->time = dtime;

  header->grid = *grid;

  header->low_dbz_threshold = sparams->low_dbz_threshold;

  header->n_entries = n_entries;
  header->n_poly_sides = sparams->n_poly_sides;

  if (header->grid.proj_type == TITAN_PROJ_FLAT) {
    header->poly_start_az =
      sparams->poly_start_az + grid->proj_params.flat.rotation;
  } else {
    header->poly_start_az = sparams->poly_start_az;
  }
  header->poly_delta_az = sparams->poly_delta_az;

}  

/*******************************************************************
 * load_entry()
 *
 * loads up product entry struct
 *
 */

static void _load_entry(tstorm_spdb_header_t *header,
			tstorm_spdb_entry_t *entry,
			track_file_entry_t *file_entry,
			storm_file_global_props_t *gprops,
			track_file_forecast_props_t *fprops,
			titan_grid_comps_t *comps)
     
{
  
  ui08 *rp;

  int i;

  fl32 *radial;

  double polygon_scale;
  double max_length = 0.0;

  double direction;
  double longitude, latitude;
  double mass, dmass_dt, norm_dmass_dt;
  double area, darea_dt, norm_darea_dt;
  double orientation;
  
  /*
   * lat / lon
   */

  TITAN_xy2latlon(comps,
		gprops->proj_area_centroid_x,
		gprops->proj_area_centroid_y,
		&latitude, &longitude);
  
  entry->longitude = longitude;
  entry->latitude = latitude;

  /*
   * speed
   */

  entry->speed = fprops->smoothed_speed;
  
  /*
   * direction
   */
  
  direction = fmod(fprops->smoothed_direction + comps->rotation,
		   360.0);
  
  if (direction < 0.0) {
    direction += 360.0;
  }

  entry->direction = direction;

  /*
   * track numbers
   */
  
  entry->simple_track_num = file_entry->simple_track_num;
  entry->complex_track_num = file_entry->complex_track_num;

  /*
   * area
   */

  entry->area = gprops->proj_area;
  entry->darea_dt = fprops->proj_area;

  /*
   * top
   */
  
  entry->top = gprops->top;
  
  /*
   * ellipse
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
  entry->ellipse_orientation = orientation;

  entry->ellipse_major_radius = gprops->proj_area_major_radius;
  entry->ellipse_minor_radius = gprops->proj_area_minor_radius;

  /*
   * polygon
   */
  
  radial = gprops->proj_area_polygon;

  for (i = 0; i < header->n_poly_sides; i++, radial++) {
    max_length = MAX(max_length, *radial);
  }
  polygon_scale = max_length / 255.0;
  entry->polygon_scale = polygon_scale;

  radial = gprops->proj_area_polygon;
  rp = entry->polygon_radials;
  for (i = 0; i < header->n_poly_sides; i++, rp++, radial++) {
    *rp = (ui08) (*radial / polygon_scale + 0.5);
  }

  /*
   * valid forecast
   */

  entry->forecast_valid = file_entry->forecast_valid;

  /*
   * dbz_max
   */

  entry->dbz_max = (si08) (gprops->dbz_max + 0.5);

  /*
   * set intensity trend
   */

  mass = gprops->mass;
  dmass_dt = fprops->mass;
  norm_dmass_dt = dmass_dt / mass;
  
  if (norm_dmass_dt < -0.2) {
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
  
  if (norm_darea_dt < -0.2) {
    entry->size_trend = -1;
  } else if (norm_darea_dt > 0.5) {
    entry->size_trend = 1;
  } else {
    entry->size_trend = 0;
  }

}

