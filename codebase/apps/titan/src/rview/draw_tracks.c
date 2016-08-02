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
/************************************************************************
 * draw_tracks.c
 *
 * draws tracks to cappi
 *
 * In X, goes to a pixmap. In PS, goes to an output file.
 *
 * rview routine
 * 
 * Mike Dixon, RAP, NCAR, April 1991
 *************************************************************************/

#include "rview.h"
#include <titan/tdata_partial_track.h>

#define START_AZIMUTH 0.0
#define END_AZIMUTH 360.0
#define NSEGMENTS 120
#define ARROW_HEAD_ANGLE 15.0
#define ARROW_HEAD_KM_FRACTION 0.010
#define ARROW_HEAD_KM 1.0
#define DEG_TO_RAD 0.01745329251994372

static tdata_partial_track_t Partial;

/*
 * file scope prototypes
 */

static void convert_xy_coords(titan_grid_comps_t *source_comps,
			      titan_grid_comps_t *target_comps,
			      double source_x, double source_y,
			      double *target_x, double *target_y);

static void draw_annotation(int dev,
			    gframe_t *frame,
			    tdata_basic_track_entry_t *entry,
			    double x, double y);

static void draw_forecast_vectors(int dev,
				  gframe_t *frame,
				  GC forecast_vector_gc,
				  titan_grid_comps_t *storm_comps,
				  tdata_basic_header_t *header,
				  tdata_basic_track_entry_t *entry,
				  double x, double y);

static void draw_runs(int dev,
		      gframe_t *frame,
		      GC storm_gc,
		      psgc_t *storm_psgc,
		      titan_grid_t *storm_grid,
		      double xcorr, double ycorr,
		      tdata_basic_header_t *header,
		      tdata_basic_track_entry_t *entry,
		      storm_file_run_t *runs);
     
static void draw_storm_shape(int dev,
			     gframe_t *frame,
			     GC storm_gc,
			     psgc_t *storm_psgc,
			     titan_grid_t *storm_grid,
			     double xcorr, double ycorr,
			     tdata_basic_header_t *header,
			     tdata_basic_track_entry_t *entry,
			     storm_file_run_t ****storm_runs,
			     int forecast_step, double x, double y,
			     int icomplex, int isimple, int ientry);

static void draw_vectors(int dev,
			 gframe_t *frame,
			 si32 start_scan,
			 si32 current_scan,
			 si32 end_scan,
			 int icomplex,
			 titan_grid_comps_t *storm_comps,
			 tdata_basic_complex_params_t *ct_params,
			 tdata_basic_with_params_index_t *tdata_index);
  
static int find_scan_limits(si32 current_time,
			    tdata_basic_with_params_index_t *tdata_index,
			    tdata_basic_complex_params_t *ct_params,
			    int icomplex,
			    si32 *start_scan,
			    si32 *current_scan,
			    si32 *end_scan,
			    si32 *start_time,
			    si32 *end_time);

static int forecast_valid(tdata_basic_header_t *header,
			  tdata_basic_track_entry_t *entry,
			  double volume, double area);

static tdata_basic_track_entry_t *
next_entry(si32 scan_num,
	   tdata_basic_with_params_index_t *tdata_index,
	   tdata_basic_complex_params_t *ct_params,
	   int icomplex, int *isimple_p, int *ientry_p);

static GC select_gc(time_hist_shmem_t *tshmem,
		    tdata_basic_track_entry_t *entry,
		    GC highlight_gc,
		    GC dim_gc);
     
static void
render_tracks(int dev,
	      gframe_t *frame,
	      date_time_t **track_time_ptr,
	      double *dbz_threshold,
	      si32 *n_tracks_plotted);

static void set_shmem_times(si32 current_time,
			    si32 start_time,
			    si32 end_time);

/* 
 * main routine
 */

void draw_tracks(int dev,
		 gframe_t *frame,
		 date_time_t **track_time_ptr,
		 double *dbz_threshold,
		 si32 *n_tracks_plotted)

{
  
  /*
   * if postscript, save graphics context and set up font size
   */
  
  if (dev == PSDEV) {
    PsGsave(frame->psgc->file);
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_track_annotation_fontsize);
  }
	  
  render_tracks(dev, frame, track_time_ptr,
		dbz_threshold, n_tracks_plotted);
  
  /*
   * restore postscript graphics context
   */
  
  if (dev == PSDEV) {
    PsGrestore(frame->psgc->file);
  }
	  
}

/*************************************************************
 *
 * convert_xy_coords()
 *
 * convert x,y coords from one grid to another
 */

static void convert_xy_coords(titan_grid_comps_t *source_comps,
			      titan_grid_comps_t *target_comps,
			      double source_x, double source_y,
			      double *target_x, double *target_y)
     
{

  double lat, lon;
  
  if (source_comps->proj_type == TITAN_PROJ_LATLON &&
      target_comps->proj_type == TITAN_PROJ_LATLON) {

    *target_x = source_x;
    *target_y = source_y;

  } else if (!memcmp((void *) source_comps,
		     (void *) target_comps,
		     sizeof(titan_grid_comps_t))) {

    *target_x = source_x;
    *target_y = source_y;

  } else {

    TITAN_xy2latlon(source_comps, source_x, source_y,
		  &lat, &lon);
  
    TITAN_latlon2xy(target_comps, lat, lon, target_x, target_y);
    
  }

}

/*****************************************
 * draw_annotation()
 */

static void draw_annotation(int dev,
			    gframe_t *frame,
			    tdata_basic_track_entry_t *entry,
			    double x, double y)

{

  char annotation_str[64];

  /*
   * annotation
   */
  
  switch (Glob->annotate_tracks) {
      
  case SPEED_ANNOTATION:
		
    if (entry->forecast_valid) {
      sprintf(annotation_str, "%.0f", entry->speed);
    } else {
      sprintf(annotation_str, "*");
    }
    break;
	  
  case NUMBER_ANNOTATION:
	    
    if (Glob->track_shmem->track_type == PARTIAL_TRACK &&
	Glob->track_shmem->case_num >= 0 &&
	tdata_basic_entry_in_partial(&Partial, entry)) {

      /*
       * label as a case
       */
      
      sprintf(annotation_str, "C%d",
	      Glob->track_shmem->case_num);

    } else if (entry->complex_track_num == entry->simple_track_num) {
      
      /*
       * simple and complex track numbers
       * are the same
       */
      
      sprintf(annotation_str, "%ld",
	      (long) entry->complex_track_num);
      
    } else {
      
      /*
       * simple and complex track numbers differ
       */
      
      sprintf(annotation_str, "%ld/%ld",
	      (long) entry->complex_track_num,
	      (long) entry->simple_track_num);
      
    }
    
    break;
    
  case MAX_DBZ_ANNOTATION:
    sprintf(annotation_str, "%.0f", entry->dbz_max);
    break;
    
  case TOPS_ANNOTATION:
    sprintf(annotation_str, "%.1f", entry->top);
    break;
    
  case VIL_ANNOTATION:
    sprintf(annotation_str, "%.1f", entry->vil_from_maxz);
    break;

  default:
    annotation_str[0] = '\0';
    
  } /* switch */

  if (strlen(annotation_str) > 0) {
    GDrawImageString(dev, frame, Glob->track_annotation_gc,
		     frame->x->font, frame->psgc,
		     XJ_CENTER, YJ_CENTER, x, y, annotation_str);
  }
    
}

/*****************************************
 * draw_forecast_vectors()
 */

static void draw_forecast_vectors(int dev,
				  gframe_t *frame,
				  GC forecast_vector_gc,
				  titan_grid_comps_t *storm_comps,
				  tdata_basic_header_t *header,
				  tdata_basic_track_entry_t *entry,
				  double x, double y)

{

  static int first_call = TRUE;
  static double arrow_head;

  int istep;

  double xx1, yy1, xx2, yy2;
  double move_x, move_y;
  double plot_volume, plot_proj_area;
  double dhr, lead_time_hr;
  
  /*
   * initialize
   */

  if (first_call) {

    /*
     * get arrow head length in world coordinates
     */

    arrow_head =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "arrow_head_km", ARROW_HEAD_KM);
    
    if (Glob->grid.proj_type == TITAN_PROJ_LATLON) {
      arrow_head /= 50.0;
    }

    first_call = FALSE;

  }

  if (!Glob->plot_vectors) {
    return;
  }

  /*
   * compute movement per time step
   */

  dhr = Glob->forecast_interval / 3600.0;
  move_x = entry->proj_area_centroid_dx_dt * dhr;
  move_y = entry->proj_area_centroid_dy_dt * dhr;

  /*
   * set line width if postscript
   */
  
  if (dev == PSDEV) {
    PsSetLineStyle(frame->psgc->file,
		   &Glob->forecast_vector_psgc);
  }
   
  /*
   * loop through forecast steps
   */

  xx2 = x;
  yy2 = y;
  
  for (istep = 1; istep <= Glob->n_forecast_steps; istep++) {

    lead_time_hr = istep * dhr;

    plot_volume = entry->volume + entry->dvolume_dt * lead_time_hr;
    plot_proj_area =
      entry->proj_area + entry->dproj_area_dt * lead_time_hr;
    
    /*
     * do not plot if volume or area is too low
     */

    if (!forecast_valid(header, entry, plot_volume, plot_proj_area)) {
      return;
    }

    /*
     * set arrow end points
     */
    
    xx1 = xx2;
    yy1 = yy2;
    xx2 += move_x;
    yy2 += move_y;
    
    GDrawArrow(dev, frame, forecast_vector_gc, frame->psgc,
	       xx1, yy1, xx2, yy2,
	       ARROW_HEAD_ANGLE, arrow_head);
    
  } /* istep */
  
}

/**************
 * draw_runs()
 */

static void draw_runs(int dev,
		      gframe_t *frame,
		      GC storm_gc,
		      psgc_t *storm_psgc,
		      titan_grid_t *storm_grid,
		      double xcorr, double ycorr,
		      tdata_basic_header_t *header,
		      tdata_basic_track_entry_t *entry,
		      storm_file_run_t *runs)
    
{

  int iz, irun;
  double dx, dy, dz;
  double minx, miny, minz;
  double startx, starty, width, height;
  storm_file_run_t *run = runs;

  /*
   * grid params
   */
  
  dx = storm_grid->dx;
  dy = storm_grid->dy;
  dz = storm_grid->dz;
  minx = storm_grid->minx;
  miny = storm_grid->miny;
  minz = storm_grid->minz;

  iz = (si32) ((Glob->z_cappi - minz) / dz + 0.5);
    
  for (irun = 0; irun < entry->n_runs; irun++, run++) {
      
    if (run->iz == iz || Glob->plot_composite) {
	
      startx = minx + ((double) run->ix -0.5) * dx + xcorr;
      starty = miny + ((double) run->iy -0.5) * dy + ycorr;
      width = (double) run->n * dx;
      height = dy;

      if (Glob->fill_runs) {
	GFillRectangle(dev, frame, storm_gc, storm_psgc,
		       startx, starty, width, height);
      } else {
	GDrawRectangle(dev, frame, storm_gc, storm_psgc,
		       startx, starty, width, height);
      }
      
    } /* if (run->iz ... ) */
    
  }
    
}

/*****************************************
 * draw_storm_shape()
 *
 * Draw ellipse or polygon as appropriate
 */

static void draw_storm_shape(int dev,
			     gframe_t *frame,
			     GC storm_gc,
			     psgc_t *storm_psgc,
			     titan_grid_t *storm_grid,
			     double xcorr, double ycorr,
			     tdata_basic_header_t *header,
			     tdata_basic_track_entry_t *entry,
			     storm_file_run_t ****storm_runs,
			     int forecast_step, double x, double y,
			     int icomplex, int isimple, int ientry)

{

  int iray;
  
  double dx, dy;
  double lead_time_hr;
  double plot_x, plot_y;
  double plot_major_radius, plot_minor_radius;
  double plot_volume, plot_proj_area;
  double poly_delta_az;
  double theta;
  double range;
  double plot_storm_scale;

  GPoint ray[N_POLY_SIDES + 1];
  GPoint polygon[N_POLY_SIDES + 1];

  /*
   * compute plotting posn depending on forecast_step
   */

  if (forecast_step == 0) {

    plot_x = x;
    plot_y = y;

    plot_volume = entry->volume;
    plot_proj_area = entry->proj_area;

    plot_storm_scale = 1.0;

  } else {

    lead_time_hr = ((double) forecast_step *
		    Glob->forecast_interval / 3600.0);
    
    plot_x = x + entry->proj_area_centroid_dx_dt * lead_time_hr;
    plot_y = y + entry->proj_area_centroid_dy_dt * lead_time_hr;

    plot_volume = entry->volume + entry->dvolume_dt * lead_time_hr;
    plot_proj_area =
      entry->proj_area + entry->dproj_area_dt * lead_time_hr;
    
    /*
     * do not plot if volume or area is too low
     */

    if (!forecast_valid(header, entry, plot_volume, plot_proj_area)) {
      return;
    }

    plot_storm_scale = sqrt(plot_proj_area / entry->proj_area);
	
  }

  if (dev == PSDEV) {
    PsSetLineStyle(frame->psgc->file, storm_psgc);
  }
    
  if (Glob->track_graphic == ELLIPSES) {
    
    if (forecast_step == 0) {
      
      plot_minor_radius = entry->proj_area_minor_radius;
      plot_major_radius = entry->proj_area_major_radius;

    } else {
      
      plot_minor_radius =
	entry->proj_area_minor_radius * plot_storm_scale;
      plot_major_radius =
	entry->proj_area_major_radius * plot_storm_scale;

    }
      
    /*
     * draw in ellipse
     */

    if (Glob->fill_graphic) {
      GFillArc(dev, frame, storm_gc, storm_psgc,
	       plot_x, plot_y, plot_major_radius, plot_minor_radius,
	       START_AZIMUTH, END_AZIMUTH,
	       (90.0 - entry->proj_area_orientation), NSEGMENTS);
    } else {
      GDrawArc(dev, frame, storm_gc, storm_psgc,
	       plot_x, plot_y, plot_major_radius, plot_minor_radius,
	       START_AZIMUTH, END_AZIMUTH,
	       (90.0 - entry->proj_area_orientation), NSEGMENTS);
    }
    
  } else if (Glob->track_graphic == POLYGONS) {
    
    poly_delta_az = header->poly_delta_az * DEG_TO_RAD;
    theta = header->poly_start_az * DEG_TO_RAD;
    dx = header->grid.dx;
    dy = header->grid.dy;
    
    for (iray = 0; iray < header->n_poly_sides; iray++) {
      
      range = entry->proj_area_polygon[iray] * plot_storm_scale;
      ray[iray].x = range * sin(theta) * dx;
      ray[iray].y = range * cos(theta) * dy;
      polygon[iray].x = plot_x + ray[iray].x;
      polygon[iray].y = plot_y + ray[iray].y;
      
      theta += poly_delta_az;
      
    } /* iray */
    
    polygon[header->n_poly_sides].x = polygon[0].x;
    polygon[header->n_poly_sides].y = polygon[0].y;
    
    if (Glob->fill_graphic) {
      GFillPolygon(dev, frame, storm_gc, storm_psgc,
		   polygon, (header->n_poly_sides + 1),
		   CoordModeOrigin);
    } else {
      GDrawLines(dev, frame, storm_gc, storm_psgc,
		 polygon, (header->n_poly_sides + 1),
		 CoordModeOrigin);
    }
      
  } /* if (Glob->track_graphic == ELLIPSES) */

  /*
   * if required, draw in storm runs
   */

  if (Glob->plot_runs && storm_runs != NULL) {

    draw_runs(dev, frame, storm_gc, storm_psgc,
	      storm_grid, xcorr, ycorr,
	      header, entry,
	      storm_runs[icomplex][isimple][ientry]);
    
  } /* if (Glob->plot_runs ... */

}
  
/*****************
 * draw_vectors()
 *
 */

static void draw_vectors(int dev,
			 gframe_t *frame,
			 si32 start_scan,
			 si32 current_scan,
			 si32 end_scan,
			 int icomplex,
			 titan_grid_comps_t *storm_comps,
			 tdata_basic_complex_params_t *ct_params,
			 tdata_basic_with_params_index_t *tdata_index)
  
{

  static int first_call = TRUE;
  static double arrow_head;

  int isimple, jsimple, ientry;
  int child_found;

  si32 ichild, child_track_num;

  double xx1, yy1, xx2, yy2;
  
  tdata_basic_simple_params_t *st_params;
  tdata_basic_track_entry_t *entry, *child_entry;
  time_hist_shmem_t *tshmem = Glob->track_shmem;
  GC vector_gc;

  /*
   * initialize
   */

  if (first_call) {

    /*
     * get arrow head length in world coordinates
     */

    arrow_head =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "arrow_head_km", ARROW_HEAD_KM);

    if (Glob->grid.proj_type == TITAN_PROJ_LATLON) {
      arrow_head /= 50.0;
    }

    first_call = FALSE;

  }

  if (!Glob->plot_vectors) {
    return;
  }

  /*
   * loop through the simple tracks which make up this complex track
   */
    
  st_params = tdata_index->simple_params[icomplex];
    
  for (isimple = 0;
       isimple < ct_params->n_simple_tracks; isimple++, st_params++) {
    
    /*
     * loop through the entries
     */
    
    entry = tdata_index->track_entry[icomplex][isimple] + 1;
    
    for (ientry = 1; ientry < st_params->duration_in_scans;
	 ientry++, entry++) {
      
      if (entry->scan_num > start_scan &&
	  entry->scan_num <= end_scan) {

	convert_xy_coords(storm_comps, &Glob->grid_comps,
			  (entry - 1)->proj_area_centroid_x,
			  (entry - 1)->proj_area_centroid_y,
			  &xx1, &yy1);
      
	convert_xy_coords(storm_comps, &Glob->grid_comps,
			  entry->proj_area_centroid_x,
			  entry->proj_area_centroid_y,
			  &xx2, &yy2);

	if (entry->scan_num <= current_scan) {
	  /*
	   * past vectors
	   */
	  if ((vector_gc =
	       select_gc(tshmem, entry,
			 Glob->past_vector_gc,
			 Glob->past_vector_dim_gc)) != NULL) {
	    if (dev == PSDEV) {
	      PsSetLineStyle(frame->psgc->file,
			     &Glob->past_vector_psgc);
	    }
	    GDrawArrow(dev, frame, vector_gc, frame->psgc,
		       xx1, yy1, xx2, yy2,
		       ARROW_HEAD_ANGLE, arrow_head);
	  }
	} else {
	  /*
	   * future vectors
	   */
	  if ((vector_gc =
	       select_gc(tshmem, entry,
			 Glob->future_vector_gc,
			 Glob->future_vector_dim_gc)) != NULL) {
	    if (dev == PSDEV) {
	      PsSetLineStyle(frame->psgc->file,
			     &Glob->future_vector_psgc);
	    }
	    GDrawArrow(dev, frame, vector_gc, frame->psgc,
		       xx1, yy1, xx2, yy2,
		       ARROW_HEAD_ANGLE, arrow_head);
	  }
	} /* if (entry->scan <= current_scan) */

      } /* if (entry->scan > start_scan ... */

    } /* ientry */

    /*
     * if this simple track has children,
     * draw vectors to each child
     */
	  
    if (st_params->nchildren > 0) {

      entry = (tdata_index->track_entry[icomplex][isimple] +
	       st_params->duration_in_scans - 1);

      if (entry->scan_num >= end_scan ||
	  entry->scan_num < start_scan) {

	/*
	 * this is the last scan to be plotted, or later,
	 * so do not draw vectors to the children
	 */

	continue;

      }
    
      convert_xy_coords(storm_comps, &Glob->grid_comps,
			entry->proj_area_centroid_x,
			entry->proj_area_centroid_y,
			&xx1, &yy1);

      for (ichild = 0; ichild < st_params->nchildren; ichild++) {
	      
	child_track_num = st_params->child[ichild];
	      
	/*
	 * find global props which match the child track num
	 */
	
	child_found = FALSE;

	for (jsimple = 0;
	     jsimple < ct_params->n_simple_tracks; jsimple++) {
		
	  child_entry = tdata_index->track_entry[icomplex][jsimple];
	  if (child_entry->simple_track_num == child_track_num) {
	    child_found = TRUE;
	    break;
	  }
		
	} /* jsimple */
	      
	if (child_found) {
		
	  convert_xy_coords(storm_comps, &Glob->grid_comps,
			    child_entry->proj_area_centroid_x,
			    child_entry->proj_area_centroid_y,
			    &xx2, &yy2);
	  
	  if (entry->scan_num < current_scan) {
	    /*
	     * past vectors
	     */
	    if ((vector_gc =
		 select_gc(tshmem, entry,
			   Glob->past_vector_gc,
			   Glob->past_vector_dim_gc)) != NULL) {
	      if (dev == PSDEV) {
		PsSetLineStyle(frame->psgc->file,
			       &Glob->past_vector_psgc);
	      }
	      GDrawArrow(dev, frame, vector_gc, frame->psgc,
			 xx1, yy1, xx2, yy2,
			 ARROW_HEAD_ANGLE, arrow_head);
	    }
	  } else {
	    /*
	     * future vectors
	     */
	    if ((vector_gc =
		 select_gc(tshmem, entry,
			   Glob->future_vector_gc,
			   Glob->future_vector_dim_gc)) != NULL) {
	      if (dev == PSDEV) {
		PsSetLineStyle(frame->psgc->file,
			       &Glob->future_vector_psgc);
	      }
	      GDrawArrow(dev, frame, vector_gc, frame->psgc,
			 xx1, yy1, xx2, yy2,
			 ARROW_HEAD_ANGLE, arrow_head);
	    }
	  } /* if (entry->scan_num < current_scan) */


	} /* if (child_found) */
	      
      } /* ichild */
	    
    } /* if (st_params->nchildren ... */
	  
  } /* isimple */

}
    
/****************************
 * find_scan_limits()
 *
 * Determines the start, current and end scans to be
 * rendered for this complex track
 *
 * returns 0 on success, -1 on failure
 */

static int find_scan_limits(si32 current_time,
			    tdata_basic_with_params_index_t *tdata_index,
			    tdata_basic_complex_params_t *ct_params,
			    int icomplex,
			    si32 *start_scan,
			    si32 *current_scan,
			    si32 *end_scan,
			    si32 *start_time,
			    si32 *end_time)

{

  int isimple, ientry;
  int duration;
  int start_set = FALSE;
  si32 start_search, end_search, entry_time;
  tdata_basic_simple_params_t *st_params;
  tdata_basic_track_entry_t *entry;

  /*
   * initialize
   */

  *current_scan = 0;

  /*
   * compute start search time for past track data, and end time for
   * future track data
   */
    
  if (Glob->plot_past == FALSE) {
    start_search = current_time;
  } else if (Glob->plot_past == PAST_LIMITED) {
    start_search = current_time - Glob->past_plot_period;
  } else if (Glob->plot_past == PAST_ALL) {
    start_search = ct_params->start_time;
  }
  
  if (Glob->plot_future == FALSE) {
    end_search = current_time;
  } else if (Glob->plot_future == FUTURE_LIMITED) {
    end_search = current_time + Glob->future_plot_period;
  } else if (Glob->plot_future == FUTURE_ALL) {
    end_search = ct_params->end_time;
  }
  
  /*
   * loop through the simple tracks which make up this complex track
   */
  
  st_params = tdata_index->simple_params[icomplex];
  
  for (isimple = 0;
       isimple < ct_params->n_simple_tracks; isimple++, st_params++) {
    
    duration = st_params->duration_in_scans;
      
    /*
     * loop through the scans
     */
      
    entry = tdata_index->track_entry[icomplex][isimple];

    for (ientry = 0; ientry < duration; ientry++, entry++) {

      /*
       * get time for track
       */
      
      entry_time = entry->time;

      if (!start_set) {
	if (entry_time >= start_search) {
	  *start_scan = entry->scan_num;
	  *start_time = entry_time;
	  start_set = TRUE;
	}
      }
      
      if (entry_time == current_time) {
	*current_scan = entry->scan_num;
      }

      *end_scan = entry->scan_num;
      *end_time = entry_time;

      if (entry_time >= end_search) {
	if (*current_scan > 100000) {
	  fprintf(stderr, "Scan num too high\n");
	}
	return (0);
      }

    } /* ientry */

  } /* isimple */

  return (-1);

}

/***********************************************
 * forecast_valid()
 *
 * Checks if forecast is valid
 *
 * Returns 1 if valid, 0 if not.
 */

static int forecast_valid(tdata_basic_header_t *header,
			  tdata_basic_track_entry_t *entry,
			  double volume, double area)

{

  if (!entry->forecast_valid) {
    return (0);
  } else {
    if (volume < 1.0 || area < 1.0) {
      return (0);
    } else {
      return (1);
    }
  }

}

/****************************
 * next_entry()
 *
 * Gets next entry for a specific scan number.
 * 
 * The sequence must be initialized by calling this routine with
 * a scan number of -1.
 *
 * Subsequent calls re-initialize if the scan number has changed.
 *
 * Returns pointer to entry, NULL when no entries left.
 */

static tdata_basic_track_entry_t *
next_entry(si32 scan_num,
	   tdata_basic_with_params_index_t *tdata_index,
	   tdata_basic_complex_params_t *ct_params,
	   int icomplex, int *isimple_p, int *ientry_p)

{

  static si32 current_scan_num = -1;
  static si32 start_simple = 0;
  static si32 start_entry = 0;
  
  int isimple, ientry;
  int duration;
  tdata_basic_simple_params_t *st_params;
  tdata_basic_track_entry_t *entry;

  if (scan_num != current_scan_num) {
    current_scan_num = scan_num;
    start_simple = 0;
    start_entry = 0;
  }

  if (scan_num < 0) {
    return (NULL);
  }

  /*
   * loop through the simple tracks which make up this complex track
   */
  
  st_params = tdata_index->simple_params[icomplex] + start_simple;
  
  for (isimple = start_simple;
       isimple < ct_params->n_simple_tracks; isimple++, st_params++) {

    duration = st_params->duration_in_scans;
      
    /*
     * loop through the scans
     */
      
    entry = tdata_index->track_entry[icomplex][isimple] + start_entry;

    for (ientry = start_entry; ientry < duration; ientry++, entry++) {
      
      if (entry->scan_num == scan_num) {

	/*
	 * entry found - next search can start at next
	 * simple track
	 */
	
	start_simple = isimple + 1;
	start_entry = 0;

	if (isimple_p != NULL) {
	  *isimple_p = isimple;
	}

	if (ientry_p != NULL) {
	  *ientry_p = ientry;
	}

	return (entry);

      }

    } /* ientry */

    start_entry = 0;

  } /* isimple */

  /*
   * no more entries for this scan_num - reset for next search
   */
  
  current_scan_num = -1;
  start_simple = 0;
  start_entry = 0;

  return (NULL);

}

/*************
 * select_gc()
 *
 * Selects the plotting GC based upon whether the simple
 * track has been selected.
 *
 * Returns GC if entry is to be plotted, NULL if not.
 * 
 */

static GC select_gc(time_hist_shmem_t *tshmem,
		    tdata_basic_track_entry_t *entry,
		    GC highlight_gc,
		    GC dim_gc)
     
{

  int highlight;
  int track_selected;

  /*
   * decide if track is selected
   */

  if (tshmem->track_type == COMPLEX_TRACK) {
    if (tshmem->complex_track_num == entry->complex_track_num) {
      track_selected = TRUE;
    } else {
      track_selected = FALSE;
    }
  } else if (tshmem->track_type == SIMPLE_TRACK) {
    if (tshmem->simple_track_num == entry->simple_track_num) {
      track_selected = TRUE;
    } else {
      track_selected = FALSE;
    }
  } else if (tshmem->track_type == PARTIAL_TRACK) {
    if (tdata_basic_entry_in_partial(&Partial, entry)) {
      track_selected = TRUE;
    } else {
      track_selected = FALSE;
    }
  }
  
  if (Glob->plot_tracks == SELECTED_TRACK && !track_selected) {
    return (NULL);
  }
  
  /*
   * set the GCs for plotting the tracks, dim or not depending
   * on the track number and whether time_hist is active. If time_hist
   * is active, then the time_hist track is plotted in normal
   * intensity, and all other tracks are dimmed.
   */
  
  if (!tshmem->time_hist_active) {
    highlight = TRUE;
  } else if (track_selected) {
    highlight = TRUE;
  } else {
    highlight = FALSE;
  }

  if (highlight) {
    return (highlight_gc);
  } else {
    return (dim_gc);
  }

}

/********************************************************
 * render_tracks()
 *
 * do the track rendering
 */

static void
render_tracks(int dev,
	      gframe_t *frame,
	      date_time_t **track_time_ptr,
	      double *dbz_threshold,
	      si32 *n_tracks_plotted)
     
{

  static date_time_t track_time;
  static int first_call = TRUE;

  int plot_forecast;
  int icolor;
  
  si32 istep, icomplex, isimple, ientry, iscan;
  si32 complex_track_num;

  si32 start_time, end_time;
  si32 min_start_time = LARGE_LONG;
  si32 max_end_time = 0;
  si32 start_scan, current_scan, end_scan;

  double x, y;
  double xx1, yy1, xx2, yy2;
  double xcorr, ycorr;

  GC forecast_storm_gc;
  GC forecast_vector_gc;
  GC storm_gc;

  time_hist_shmem_t *tshmem;
  tdata_basic_with_params_index_t *tdata_index;
  tdata_basic_header_t *header;
  tdata_basic_complex_params_t *ct_params;
  tdata_basic_track_entry_t *entry;
  titan_grid_t *storm_grid;
  titan_grid_comps_t storm_comps;

  if (Glob->debug) {
    fprintf(stderr, "** render_tracks **\n");
  }

  /*
   * initialize
   */

  tshmem = Glob->track_shmem;
  tdata_index = &Glob->tdata_index.basic_p;
  header = &tdata_index->header;
  storm_grid = &header->grid;

  if (first_call) {

    tdata_init_partial_track(&Partial,
			     Glob->prog_name, Glob->debug);
    first_call = FALSE;

  }
  
  /*
   * find the partial track if required
   */
  
  if (tshmem->track_type == PARTIAL_TRACK) {
    if (tdata_find_basic_partial_track(&Partial,
				       tshmem->partial_track_ref_time,
				       tshmem->partial_track_past_period,
				       tshmem->partial_track_future_period,
				       tshmem->complex_track_num,
				       tshmem->simple_track_num,
				       &Glob->tdata_index.basic_p)) {
      /*
       * partial track not found - beep
       */
      /* fprintf(stderr, "\a"); */
    }
  }

  /*
   * set return values
   */
  
  track_time.unix_time = header->time;
  uconvert_from_utime(&track_time);
  *track_time_ptr = &track_time;
  *dbz_threshold = (double) header->dbz_threshold;
  *n_tracks_plotted = 0;

  /*
   * remove rotation from the storm grid - because the image
   * will not be displayed rotated
   */

  storm_grid->proj_params.flat.rotation = 0.0;
  TITAN_init_proj(storm_grid, &storm_comps);

  /*
   * compute grid offsets (x, y)
   */
  
  if (storm_comps.proj_type == TITAN_PROJ_LATLON ||
      Glob->grid_comps.proj_type == TITAN_PROJ_LATLON) {
    
    xcorr = 0.0;
    ycorr = 0.0;

  } else {
    
    TITAN_latlon2xy(&Glob->grid_comps,
		  Glob->grid.proj_origin_lat,
		  Glob->grid.proj_origin_lon,
		  &xx1, &yy1);
    
    TITAN_latlon2xy(&Glob->grid_comps,
		  storm_grid->proj_origin_lat,
		  storm_grid->proj_origin_lon,
		  &xx2, &yy2);
    
    xcorr = xx2 - xx1;
    ycorr = yy2 - yy1;

  }

  /*
   * loop through the complex tracks
   */

  for (icomplex = 0;
       icomplex < header->n_complex_tracks; icomplex++) {

    *n_tracks_plotted += 1;
    ct_params = tdata_index->complex_params + icomplex;
    complex_track_num = ct_params->complex_track_num;

    /*
     * if only selected track is to be displayed, continue now
     * if track number is not selected track
     */
    
    if (Glob->plot_tracks == SELECTED_TRACK &&
	tshmem->complex_track_num != complex_track_num) {
      continue;
    }
    
    if (find_scan_limits(header->time, tdata_index, ct_params,
			 icomplex, &start_scan, &current_scan, &end_scan,
			 &start_time, &end_time)) {
      continue;
    }

    min_start_time = MIN(min_start_time, start_time);
    max_end_time = MAX(max_end_time, end_time);

    /*
     * for past scans, draw in the storm shapes
     */

    next_entry(-1, tdata_index, ct_params, icomplex, NULL, NULL);
    for (iscan = start_scan; iscan < current_scan; iscan++) {

      if (Glob->fill_graphic) {
	icolor = iscan % 2;
      } else {
	icolor = 0;
      }

      while ((entry = next_entry(iscan, tdata_index,
				 ct_params, icomplex,
				 &isimple, &ientry)) != NULL) {

	if ((storm_gc =
	     select_gc(tshmem, entry,
		       Glob->past_storm_gc[icolor],
		       Glob->past_storm_dim_gc[icolor])) != NULL) {

	  x = entry->proj_area_centroid_x + xcorr;
	  y = entry->proj_area_centroid_y + ycorr;
	  
	  draw_storm_shape(dev, frame, storm_gc,
			   &Glob->past_storm_psgc[icolor],
			   storm_grid, xcorr, ycorr,
			   header, entry, 
			   tdata_index->storm_runs,
			   0, x, y,
			   icomplex, isimple, ientry);
	  
	}

      } /* while */
      
    } /* iscan */

    /*
     * for future scans, draw in the storm shapes
     */

    next_entry(-1, tdata_index, ct_params, icomplex, NULL, NULL);
    for (iscan = end_scan; iscan > current_scan; iscan--) {

      if (Glob->fill_graphic) {
	icolor = iscan % 2;
      } else {
	icolor = 0;
      }

      while ((entry = next_entry(iscan, tdata_index,
				 ct_params, icomplex,
				 &isimple, &ientry)) != NULL) {

	if ((storm_gc =
	     select_gc(tshmem, entry,
		       Glob->future_storm_gc[icolor],
		       Glob->future_storm_dim_gc[icolor])) != NULL) {

	  x = entry->proj_area_centroid_x + xcorr;
	  y = entry->proj_area_centroid_y + ycorr;
	  
	  draw_storm_shape(dev, frame, storm_gc,
			   &Glob->future_storm_psgc[icolor],
			   storm_grid, xcorr, ycorr, 
			   header, entry, 
			   tdata_index->storm_runs,
			   0, x, y,
			   icomplex, isimple, ientry);
	  
	}

      } /* while */
      
    } /* iscan */

    /*
     * draw in the forecast and current storm shape,
     * and forecast vectors
     */

    next_entry(-1, tdata_index, ct_params, icomplex, NULL, NULL);

    while ((entry = next_entry(current_scan, tdata_index,
			       ct_params, icomplex,
			       &isimple, &ientry)) != NULL) {

      /*
       * get storm (x, y)
       */

      x = entry->proj_area_centroid_x + xcorr;
      y = entry->proj_area_centroid_y + ycorr;
      
      /*
       * decide whether to plot forecast
       */
      
      if (Glob->plot_forecast == FORECAST_ALL) {
	plot_forecast = TRUE;
      } else if (Glob->plot_forecast == FORECAST_LIMITED &&
		 entry->forecast_valid) {
	plot_forecast = TRUE;
      } else {
	plot_forecast = FALSE;
      }
      
      if (plot_forecast) {

	for (istep = Glob->n_forecast_steps; istep >= 1; istep--) {
	  
	  if (Glob->fill_graphic) {
	    icolor = istep % 2;
	  } else {
	    icolor = 0;
	  }

	  if ((forecast_storm_gc =
	       select_gc(tshmem, entry,
			 Glob->forecast_storm_gc[icolor],
			 Glob->forecast_storm_dim_gc[icolor])) != NULL) {
	  
	    draw_storm_shape(dev, frame,
			     forecast_storm_gc,
			     &Glob->forecast_storm_psgc[icolor],
			     storm_grid, xcorr, ycorr, 
			     header, entry, 
			     tdata_index->storm_runs,
			     istep, x, y,
			     icomplex, isimple, ientry);

	  }

	} /* istep */

      } /* if (plot_forecast) */

      /*
       * draw current shape
       */
      
      if (Glob->plot_current) {
	if ((storm_gc = select_gc(tshmem, entry,
				  Glob->current_storm_gc,
				  Glob->current_storm_dim_gc)) != NULL) {
	  draw_storm_shape(dev, frame, storm_gc,
			   &Glob->current_storm_psgc,
			   storm_grid, xcorr, ycorr, 
			   header, entry, 
			   tdata_index->storm_runs,
			   0, x, y,
			   icomplex, isimple, ientry);
	}
      } /* if (Glob->plot_current) */
	
      /*
       * draw in forecast vectors
       */
      
      if (plot_forecast) {
	if ((forecast_vector_gc =
	     select_gc(tshmem, entry,
		       Glob->forecast_vector_gc,
		       Glob->forecast_vector_dim_gc)) != NULL) {
	  draw_forecast_vectors(dev, frame, forecast_vector_gc,
				&storm_comps, header, entry, x, y);
	}
      }
      
    } /* while */

    /*
     * draw in vectors
     */
    
    draw_vectors(dev, frame,
		 start_scan, current_scan, end_scan,
		 icomplex, &storm_comps,
		 ct_params, tdata_index);

    /*
     * add annotation
     */
      
    next_entry(-1, tdata_index, ct_params, icomplex, NULL, NULL);

    for (iscan = start_scan; iscan <= end_scan; iscan++) {

      while ((entry = next_entry(iscan, tdata_index,
				 ct_params, icomplex,
				 NULL, NULL)) != NULL) {
	
	/*
	 * get storm (x, y)
	 */
	
	x = entry->proj_area_centroid_x + xcorr;
	y = entry->proj_area_centroid_y + ycorr;
	
	if (select_gc(tshmem, entry,
		      Glob->current_storm_gc,
		      Glob->current_storm_dim_gc) != NULL) {
	  draw_annotation(dev, frame, entry, x, y);
	}
	
      } /* while */

    } /* iscan */

  } /* icomplex */
  
  /*
   * set track shmem start time for past track data, and end time for
   * future track data
   */
  
  set_shmem_times(header->time, min_start_time, max_end_time);
  
}

/***********************
 * set_shmem_times()
 *
 * Setup times in shmem so that time_hist can display the
 * time bars correctly
 *
 */


static void set_shmem_times(si32 current_time,
			    si32 start_time,
			    si32 end_time)

{

  time_hist_shmem_t *tshmem = Glob->track_shmem;
  
  /*
   * set track shmem start time for past track data, and end time for
   * future track data
   */
  
  if (Glob->plot_past == FALSE) {
    tshmem->past_plot_period = 0;
  } else if (Glob->plot_past == PAST_LIMITED) {
    tshmem->past_plot_period =
      MIN((current_time - start_time), Glob->past_plot_period);
  } else if (Glob->plot_past == PAST_ALL) {
    tshmem->past_plot_period = current_time - start_time;
  }
  
  if (Glob->plot_future == FALSE) {
    tshmem->future_plot_period = 0;
  } else if (Glob->plot_future == FUTURE_LIMITED) {
    tshmem->future_plot_period =
      MIN((end_time - current_time), Glob->future_plot_period);
  } else if (Glob->plot_future == FUTURE_ALL) {
    tshmem->future_plot_period = end_time - current_time;
  }

}

