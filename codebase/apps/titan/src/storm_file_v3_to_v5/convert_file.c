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
 * convert_file.c
 *
 * RAP, NCAR, Boulder CO
 *
 * Jan 1996
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_file_v3_to_v5.h"
#include <toolsa/pjg.h>

/*
 * file scope prototypes
 */

static void add_bounding_box_from_polygon(storm_file_handle_t *s_handle,
					  storm_file_global_props_t *gprops);

static void add_bounding_box_from_runs(storm_file_handle_t *s_handle,
				       storm_file_global_props_t *gprops);

static void copy_header(storm_v3_file_header_t *v3_header,
			storm_file_header_t *header);

static void copy_hist(storm_v3_file_params_t *v3_params,
		      storm_v3_file_dbz_hist_t *v3_hist,
		      storm_file_dbz_hist_t *hist);

static void copy_scan_header(storm_v3_file_header_t *v3_header,
			     storm_v3_file_scan_header_t *v3_scan,
			     storm_file_scan_header_t *scan);

static void copy_gprops(storm_v3_file_header_t *v3_header,
			storm_v3_file_global_props_t *v3_gprops,
			storm_file_global_props_t *gprops);

static void copy_layer(storm_v3_file_params_t *v3_params,
		       storm_v3_file_global_props_t *v3_gprops,
		       storm_v3_file_layer_props_t *v3_layer,
		       storm_file_layer_props_t *layer);

/*
 * main
 */

void convert_file(storm_v3_file_index_t *v3_s_handle,
		  storm_file_handle_t *s_handle,
		  char *file_path)

{

  char v5_file_path[MAX_PATH_LEN];

  int iscan, istorm, ilayer, ihist;
  int n_scans;

  storm_v3_file_params_t *v3_params;
  storm_v3_file_scan_header_t *v3_scan;
  storm_file_scan_header_t *scan;
  storm_v3_file_global_props_t *v3_gprops;
  
  path_parts_t v3_parts;

  fprintf(stdout, "Converting file %s\n", file_path);

  /*
   * open v3 files for reading
   */
  
  if (Rfv3OpenStormFiles (v3_s_handle, "r",
			  file_path,
			  (char *) NULL,
			  "convert_file")) {
    fprintf(stderr, "ERROR - %s:convert_file\n", Glob->prog_name);
    fprintf(stderr, "Opening storm file.\n");
    tidy_and_exit(-1);
  }

  /*
   * read in storm properties file header
   */
  
  if (Rfv3ReadStormHeader(v3_s_handle, "convert_file") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:convert_file\n", Glob->prog_name);
    fprintf(stderr, "Reading file %s.\n", file_path);
    tidy_and_exit(-1);
  }
  
  n_scans = v3_s_handle->header->n_scans;
  v3_params = &v3_s_handle->header->params;
  
  /*
   * compute v5 file path
   */

  uparse_path(file_path, &v3_parts);
  sprintf(v5_file_path, "%s%s%s.%s",
	  v3_parts.dir, PATH_DELIM,
	  v3_parts.base, STORM_HEADER_FILE_EXT);
  
  /*
   * open the v5 files for writing
   */
  
  if (RfOpenStormFiles (s_handle, "w", v5_file_path,
			STORM_DATA_FILE_EXT,
			"convert_file")) {
    fprintf(stderr, "ERROR - %s:convert_file\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot open storm file mode 'w'.\n");
    perror(v5_file_path);
    tidy_and_exit(-1);
  }
  
  if (RfAllocStormHeader(s_handle, "convert_file")) {
    tidy_and_exit(-1);
  }
   
  /*
   * copy file header
   */

  copy_header(v3_s_handle->header, s_handle->header);

  /*
   * set the data file at the correct point for data writes - after the
   * label
   */
  
  if (RfSeekStartStormData(s_handle, "convert_file")) {
    tidy_and_exit(-1);
  }
  
  /*
   * loop through scans
   */
  
  for (iscan = 0; iscan < n_scans; iscan++) {

    /*
     * read in scan info
     */
    
    if (Rfv3ReadStormScan(v3_s_handle, iscan, "convert_file") != R_SUCCESS) {
      fprintf(stderr, "ERROR - %s:convert_file\n", Glob->prog_name);
      fprintf(stderr, "Reading file %s.\n", file_path);
      tidy_and_exit(-1);
    }

    v3_scan = v3_s_handle->scan;

    /*
     * Make sure there's memory allocated for the scan and storms,
     * and initialize
     */
    
    RfAllocStormScan(s_handle, v3_scan->nstorms, "convert_file");
    scan = s_handle->scan;

    /*
     * copy over the scan_header and the global props
     */

    copy_scan_header(v3_s_handle->header, v3_scan, scan);

    for (istorm = 0; istorm < v3_scan->nstorms; istorm++) {
      copy_gprops(v3_s_handle->header,
		  v3_s_handle->gprops + istorm,
		  s_handle->gprops + istorm);
    }

    /*
     * loop through the storms
     */

    for (istorm = 0; istorm < v3_scan->nstorms; istorm++) {
      
      v3_gprops = v3_s_handle->gprops + istorm;
      
      if (Rfv3ReadStormProps(v3_s_handle, istorm,
			     "convert_track") != R_SUCCESS) {
	tidy_and_exit(-1);
      }
      
      RfAllocStormProps(s_handle, v3_scan->cart.nz,
			v3_gprops->n_dbz_intervals,
			v3_gprops->n_runs, (si32) 0,
			"convert_file");

      /*
       * copy over the storm data
       */

      for (ilayer = 0; ilayer < v3_gprops->n_layers; ilayer++) {
	copy_layer(&v3_s_handle->header->params, v3_gprops,
		   v3_s_handle->layer + ilayer,
		   s_handle->layer + ilayer);
      }

      for (ihist = 0; ihist < v3_gprops->n_dbz_intervals; ihist++) {
	copy_hist(&v3_s_handle->header->params,
		  v3_s_handle->hist + ihist,
		  s_handle->hist + ihist);
      }

      memcpy((void *) s_handle->runs,
	     (void *) v3_s_handle->runs,
	     (size_t) (v3_gprops->n_runs *
		       sizeof(storm_file_run_t)));

      /*
       * add in the bounding box
       */

      if (s_handle->gprops[istorm].n_runs == 0) {
	
	add_bounding_box_from_polygon(s_handle, s_handle->gprops + istorm);

      } else {

	add_bounding_box_from_runs(s_handle, s_handle->gprops + istorm);

      }

      /*
       * if required, add in projected area runs
       */

      if (Glob->params.add_proj_runs) {
	add_proj_runs(s_handle, s_handle->gprops + istorm);
      }

      /*
       * write storm props
       */
      
      if (RfWriteStormProps(s_handle, istorm, "convert_file")) {
	tidy_and_exit(-1);
      }

    } /* istorm */

    /*
     * write scan
     */

    if (RfWriteStormScan(s_handle, iscan, "convert_file")) {
      tidy_and_exit(-1);
    }
    
  } /* iscan */
  
  /*
   * write storm file header
   */
  
  if (RfWriteStormHeader(s_handle, "convert_file")) {
    tidy_and_exit(-1);
  }
  
  /*
   * close the files
   */

  Rfv3CloseStormFiles(v3_s_handle, "convert_file");
  RfCloseStormFiles(s_handle, "convert_file");
  
}

static void add_bounding_box_from_polygon(storm_file_handle_t *s_handle,
					  storm_file_global_props_t *gprops)

{

  int i;
  int n_poly_sides;
  double poly_start_az, poly_delta_az;
  double az;
  double x, y;
  double minx, miny, maxx, maxy;

  titan_grid_t *grid = &s_handle->scan->grid;
  storm_file_params_t *sparams = &s_handle->header->params;

  n_poly_sides = sparams->n_poly_sides;
  poly_start_az = sparams->poly_start_az;
  poly_delta_az = sparams->poly_delta_az;
  
  minx = 1.0e10;
  miny = 1.0e10;
  maxx = -1.0e10;
  maxy = -1.0e10;
  
  for (i = 0; i < n_poly_sides; i++) {

    az = (poly_start_az + (double) i * poly_delta_az) * DEG_TO_RAD;
    x = gprops->proj_area_centroid_x
      + (gprops->proj_area_polygon[i] * sin(az));
    y = gprops->proj_area_centroid_y +
      (gprops->proj_area_polygon[i] * cos(az));
    
    minx = MIN(minx, x);
    miny = MIN(miny, y);
    maxx = MAX(maxx, x);
    maxy = MAX(maxy, y);
    
  }
  
  gprops->bounding_min_ix =
    (si32) floor((minx - grid->minx) / grid->dx - 1.0);
  gprops->bounding_min_iy =
    (si32) floor((miny - grid->miny) / grid->dy - 1.0);
  gprops->bounding_max_ix =
    (si32) floor((maxx - grid->minx) / grid->dx + 2.0);
  gprops->bounding_max_iy =
    (si32) floor((maxy - grid->miny) / grid->dy + 2.0);

  gprops->bounding_min_ix = MAX(gprops->bounding_min_ix, 0);
  gprops->bounding_min_iy = MAX(gprops->bounding_min_iy, 0);
  gprops->bounding_max_ix = MIN(gprops->bounding_max_ix, grid->nx - 1);
  gprops->bounding_max_iy = MIN(gprops->bounding_max_iy, grid->ny - 1);

}

static void add_bounding_box_from_runs(storm_file_handle_t *s_handle,
				       storm_file_global_props_t *gprops)

{
  
  int irun;
  int min_ix, min_iy, max_ix, max_iy;
  storm_file_run_t *run;
  
  min_ix = 1000000;
  min_iy = 1000000;
  max_ix = -1000000;
  max_iy = -1000000;
  
  run = s_handle->runs;
  
  for (irun = 0; irun < gprops->n_runs; irun++, run++) {
    
    min_ix = MIN(min_ix, run->ix);
    max_ix = MAX(max_ix, run->ix + run->n - 1);
    min_iy = MIN(min_iy, run->iy);
    max_iy = MAX(max_iy, run->iy);
    
  }
  
  gprops->bounding_min_ix = min_ix;
  gprops->bounding_min_iy = min_iy;
  gprops->bounding_max_ix = max_ix;
  gprops->bounding_max_iy = max_iy;

} 

static void copy_header(storm_v3_file_header_t *v3_header,
			storm_file_header_t *header)

{

  storm_v3_file_params_t *v3_params = &v3_header->params;
  storm_file_params_t *params = &header->params;
  storm_v3_float_params_t fl_params;

  Rfv3DecodeStormParams(v3_params, &fl_params);
  
  memset((void *) header, 0, sizeof(storm_file_header_t));
  
  header->n_scans = v3_header->n_scans;
  header->start_time = uunix_time((date_time_t *) &v3_header->start_time);
  header->end_time = uunix_time((date_time_t *) &v3_header->end_time);

  params->low_dbz_threshold = fl_params.low_dbz_threshold;
  params->high_dbz_threshold = fl_params.high_dbz_threshold;
  params->dbz_hist_interval = fl_params.dbz_hist_interval;
  params->hail_dbz_threshold = fl_params.hail_dbz_threshold;
  params->base_threshold = fl_params.base_threshold;
  params->top_threshold = fl_params.top_threshold;
  params->min_storm_size = fl_params.min_storm_size;
  params->max_storm_size = fl_params.max_storm_size;
  params->z_p_coeff = fl_params.z_p_coeff;
  params->z_p_exponent = fl_params.z_p_exponent;
  params->z_m_coeff = fl_params.z_m_coeff;
  params->z_m_exponent = fl_params.z_m_exponent;
  params->sectrip_vert_aspect = fl_params.sectrip_vert_aspect;
  params->sectrip_horiz_aspect = fl_params.sectrip_horiz_aspect;
  params->sectrip_orientation_error = fl_params.sectrip_orientation_error;
  params->vel_available = v3_params->vel_available;
  params->n_poly_sides = N_POLY_SIDES;
  params->poly_start_az = 0;
  params->poly_delta_az = 360.0 / (double) N_POLY_SIDES;

}

static void copy_scan_header(storm_v3_file_header_t *v3_header,
			     storm_v3_file_scan_header_t *v3_scan,
			     storm_file_scan_header_t *scan)

{

  int pjg_type, titan_type;
  storm_v3_float_scan_header_t fl_scan;

  Rfv3DecodeStormScan(&v3_header->params,
		      v3_scan, &fl_scan);
  
  memset((void *) scan, 0, sizeof(storm_file_scan_header_t));

  scan->scan_num = v3_scan->scan_num;
  scan->nstorms = v3_scan->nstorms;
  scan->time = uunix_time((date_time_t *) &v3_scan->time);
  scan->min_z = fl_scan.min_z;
  scan->delta_z = fl_scan.delta_z;

  pjg_type = Rfv3StormGridType(&v3_header->params);
  titan_type = pjg2mdv_type(pjg_type);

  RfCartParams2TITANGrid(&v3_scan->cart, &scan->grid, titan_type);

}


static void copy_gprops(storm_v3_file_header_t *v3_header,
			storm_v3_file_global_props_t *v3_gprops,
			storm_file_global_props_t *gprops)

{

  int i;
  storm_v3_float_global_props_t fl_gprops;

  Rfv3DecodeStormProps(&v3_header->params, v3_gprops,
		       &fl_gprops, STORM_V3_ALL_PROPS);

  memset((void *) gprops, 0, sizeof(storm_file_global_props_t));

  gprops->storm_num = v3_gprops->storm_num;
  gprops->n_layers = v3_gprops->n_layers;
  gprops->base_layer = v3_gprops->base_layer;
  gprops->n_dbz_intervals = v3_gprops->n_dbz_intervals;
  gprops->n_runs = v3_gprops->n_runs;
  gprops->n_proj_runs = 0;
  gprops->top_missing = v3_gprops->top_missing;
  gprops->range_limited = v3_gprops->range_limited;
  gprops->second_trip = v3_gprops->second_trip;
  gprops->hail_present = v3_gprops->hail_present;
  gprops->vol_centroid_x = fl_gprops.vol_centroid_x;
  gprops->vol_centroid_y = fl_gprops.vol_centroid_y;
  gprops->vol_centroid_z = fl_gprops.vol_centroid_z;
  gprops->refl_centroid_x = fl_gprops.refl_centroid_x;
  gprops->refl_centroid_y = fl_gprops.refl_centroid_y;
  gprops->refl_centroid_z = fl_gprops.refl_centroid_z;
  gprops->top = fl_gprops.top;
  gprops->base = fl_gprops.base;
  gprops->volume = fl_gprops.volume;
  gprops->area_mean = fl_gprops.area_mean;
  gprops->precip_flux = fl_gprops.precip_flux;
  gprops->mass = fl_gprops.mass;
  gprops->tilt_angle = fl_gprops.tilt_angle;
  gprops->tilt_dirn = fl_gprops.tilt_dirn;
  gprops->dbz_max = fl_gprops.dbz_max;
  gprops->dbz_mean = fl_gprops.dbz_mean;
  gprops->ht_of_dbz_max = fl_gprops.ht_of_dbz_max;
  gprops->rad_vel_mean = 0;
  gprops->vorticity = fl_gprops.vorticity;
  gprops->precip_area = fl_gprops.precip_area;
  gprops->precip_area_centroid_x = fl_gprops.precip_area_centroid_x;
  gprops->precip_area_centroid_y = fl_gprops.precip_area_centroid_y;
  gprops->precip_area_orientation = fl_gprops.precip_area_orientation;
  gprops->precip_area_minor_radius = fl_gprops.precip_area_minor_radius;
  gprops->precip_area_major_radius = fl_gprops.precip_area_major_radius;
  gprops->proj_area = fl_gprops.proj_area;
  gprops->proj_area_centroid_x = fl_gprops.proj_area_centroid_x;
  gprops->proj_area_centroid_y = fl_gprops.proj_area_centroid_y;
  gprops->proj_area_orientation = fl_gprops.proj_area_orientation;
  gprops->proj_area_minor_radius = fl_gprops.proj_area_minor_radius;
  gprops->proj_area_major_radius = fl_gprops.proj_area_major_radius;

  for (i = 0; i < V3_N_POLY_SIDES; i++) {
    gprops->proj_area_polygon[i*2] = fl_gprops.proj_area_polygon[i];
  }

  for (i = 0; i < V3_N_POLY_SIDES - 1; i++) {
    gprops->proj_area_polygon[i*2 + 1] =
      (fl_gprops.proj_area_polygon[i] + 
       fl_gprops.proj_area_polygon[i + 1]) / 2;
  }

  gprops->proj_area_polygon[N_POLY_SIDES - 1] =
    (fl_gprops.proj_area_polygon[0] + 
     fl_gprops.proj_area_polygon[V3_N_POLY_SIDES - 1]) / 2;

}


static void copy_layer(storm_v3_file_params_t *v3_params,
		       storm_v3_file_global_props_t *v3_gprops,
		       storm_v3_file_layer_props_t *v3_layer,
		       storm_file_layer_props_t *layer)

{

  storm_v3_float_layer_props_t fl_layer;

  Rfv3DecodeStormLayer(v3_params, v3_gprops, v3_layer, &fl_layer);

  memset((void *) layer, 0, sizeof(storm_file_layer_props_t));
  
  layer->vol_centroid_x = fl_layer.vol_centroid_x;
  layer->vol_centroid_y = fl_layer.vol_centroid_y;
  layer->refl_centroid_x = fl_layer.refl_centroid_x;
  layer->refl_centroid_y = fl_layer.refl_centroid_y;
  layer->area = fl_layer.area;
  layer->dbz_max = fl_layer.dbz_max;
  layer->dbz_mean = fl_layer.dbz_mean;
  layer->mass = fl_layer.mass;
  layer->rad_vel_mean = 0;
  layer->vorticity = fl_layer.vorticity;

}

static void copy_hist(storm_v3_file_params_t *v3_params,
		      storm_v3_file_dbz_hist_t *v3_hist,
		      storm_file_dbz_hist_t *hist)

{

  storm_v3_float_dbz_hist_t fl_hist;

  Rfv3DecodeStormHist(v3_params, v3_hist, &fl_hist);

  hist->percent_volume = fl_hist.percent_volume;
  hist->percent_area = fl_hist.percent_precip_area;

}



