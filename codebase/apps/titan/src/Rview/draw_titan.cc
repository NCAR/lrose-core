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

#include "Rview.hh"
#include <titan/TitanPartialTrack.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;

#define START_AZIMUTH 0.0
#define END_AZIMUTH 360.0
#define NSEGMENTS 120
#define ARROW_HEAD_ANGLE 15.0
#define ARROW_HEAD_KM_FRACTION 0.010
#define ARROW_HEAD_KM 1.0

/*
 * file scope prototypes
 */

static void convert_xy_coords(MdvxProj &source_proj,
			      MdvxProj &target_proj,
			      double source_x, double source_y,
			      double &target_x, double &target_y);
     
static double convert_dist(MdvxProj &source_proj,
			   MdvxProj &target_proj,
			   double source_dist);

static void draw_annotation(int dev,
			    gframe_t *frame,
			    MdvxProj &storm_proj,
			    const track_file_entry_t *entry,
			    const storm_file_global_props_t *gprops,
			    const TitanPartialTrack &partial,
			    double x, double y);

static void draw_forecast_vectors(int dev,
				  gframe_t *frame,
				  GC forecast_vector_gc,
				  MdvxProj &storm_proj,
				  const track_file_entry_t *entry,
				  const storm_file_global_props_t *gprops,
				  double x, double y);

static void draw_runs(int dev,
		      gframe_t *frame,
		      GC storm_gc,
		      psgc_t *storm_psgc,
		      MdvxProj &storm_proj,
		      const vector<storm_file_run_t> &runs);
     
static void draw_storm_shape(int dev,
			     gframe_t *frame,
			     GC storm_gc,
			     psgc_t *storm_psgc,
			     MdvxProj &storm_proj,
			     const track_file_entry_t *entry,
			     const storm_file_scan_header_t *scan,
			     const storm_file_global_props_t *gprops,
			     int forecast_step, double x, double y,
			     int icomplex, int isimple, int ientry);

static void draw_vectors(int dev,
			 gframe_t *frame,
			 int start_scan,
			 int current_scan,
			 int end_scan,
			 MdvxProj &storm_proj,
			 const TitanComplexTrack *ctrack,
			 const TitanPartialTrack &partial);
  
static int find_scan_limits(time_t current_time,
			    const TitanComplexTrack *ctrack,
			    int *start_scan,
			    int *current_scan,
			    int *end_scan,
			    time_t *start_time,
			    time_t *end_time);

static int forecast_valid(const track_file_entry_t *entry,
			  double volume, double area);

static const TitanTrackEntry *
next_entry(int scan_num,
	   const TitanComplexTrack *ctrack,
	   int *isimple_p, int *ientry_p);

static GC select_gc(const track_file_entry_t *entry,
		    const TitanPartialTrack &partial,
		    GC highlight_gc,
		    GC dim_gc);
     
static void set_storm_coord(Mdvx::coord_t &coord);

static void
render_tracks(int dev,
	      gframe_t *frame,
	      date_time_t **track_time_ptr,
	      double *dbz_threshold,
	      int *n_tracks_plotted);

static void set_shmem_times(time_t current_time,
			    time_t start_time,
			    time_t end_time);

/* 
 * main routine
 */

void draw_titan(int dev,
		gframe_t *frame,
		date_time_t **track_time_ptr,
		double *dbz_threshold,
		int *n_tracks_plotted)

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

static void convert_xy_coords(MdvxProj &source_proj,
			      MdvxProj &target_proj,
			      double source_x, double source_y,
			      double &target_x, double &target_y)
     
{
  
  if (source_proj.getProjType() == Mdvx::PROJ_LATLON &&
      target_proj.getProjType() == Mdvx::PROJ_LATLON) {

    target_x = source_x;
    target_y = source_y;

  } else {
    
    double lat, lon;
    source_proj.xy2latlon(source_x, source_y, lat, lon);
    target_proj.latlon2xy(lat, lon, target_x, target_y);
    
  }

}

/*************************************************************
 *
 * convert_dist()
 *
 * convert distance from one grid to another
 */

static double convert_dist(MdvxProj &source_proj,
			   MdvxProj &target_proj,
			   double source_dist)
     
{
  
  if (source_proj.getProjType() == target_proj.getProjType()) {

    return source_dist;

  } else {
    
    double dist_km;
    
    dist_km = source_proj.x2km(source_dist);
    return target_proj.km2xGrid(dist_km) * target_proj.getCoord().dx;

  }

}

/*****************************************
 * draw_annotation()
 */

static void draw_annotation(int dev,
			    gframe_t *frame,
			    MdvxProj &storm_proj,
			    const track_file_entry_t *entry,
			    const storm_file_global_props_t *gprops,
			    const TitanPartialTrack &partial,
			    double x, double y)
  
{

  char annotation_str[64];

  /*
   * annotation
   */
  
  switch (Glob->annotate_tracks) {
      
  case SPEED_ANNOTATION:
		
    if (entry->forecast_valid || Glob->plot_forecast == FORECAST_ALL) {
      sprintf(annotation_str, "%.0f", entry->dval_dt.smoothed_speed);
    } else {
      sprintf(annotation_str, "*");
    }
    break;
	  
  case NUMBER_ANNOTATION:
	    

    if (Glob->track_shmem->track_type == PARTIAL_TRACK &&
	Glob->track_shmem->case_num >= 0 &&
	Glob->track_shmem->complex_track_num >= 0 &&
	Glob->track_shmem->simple_track_num >= 0 &&
	partial.entryIncluded(*entry)) {

      /*
       * label as a case
       */
      
      sprintf(annotation_str, "C-%d",
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
      
      sprintf(annotation_str, "%d/%d",
	      entry->complex_track_num,
	      entry->simple_track_num);
      
    }
    
    break;
    
  case MAX_DBZ_ANNOTATION:
    sprintf(annotation_str, "%.0f", gprops->dbz_max);
    break;
    
  case TOPS_ANNOTATION:
    sprintf(annotation_str, "%.1f", gprops->top);
    break;
    
  case VIL_ANNOTATION:
    sprintf(annotation_str, "%.1f", gprops->vil_from_maxz);
    break;

  case HAIL_CAT_ANNOTATION:
    sprintf(annotation_str, "%d", gprops->add_on.hail_metrics.FOKRcategory);
    break;

  case HAIL_PROB_ANNOTATION:
    sprintf(annotation_str, "%.1f", gprops->add_on.hail_metrics.waldvogelProbability);
    break;

  case HAIL_MASS_ALOFT_ANNOTATION:
    sprintf(annotation_str, "%.1f", gprops->add_on.hail_metrics.hailMassAloft);
    break;

  case HAIL_VIHM_ANNOTATION:
    sprintf(annotation_str, "%.1f", gprops->add_on.hail_metrics.vihm);
    break;


  default:
    annotation_str[0] = '\0';
    
  } /* switch */

  double plot_x, plot_y;
  
  convert_xy_coords(storm_proj, Glob->proj,
		    x, y, plot_x, plot_y);
  
  if (strlen(annotation_str) > 0) {
    if (strncmp(annotation_str, "C-", 2) == 0) {
      GDrawImageString(dev, frame, Glob->track_case_gc,
                       frame->x->font, frame->psgc,
                       XJ_CENTER, YJ_CENTER, plot_x, plot_y, annotation_str);
    } else {
      GDrawImageString(dev, frame, Glob->track_annotation_gc,
                       frame->x->font, frame->psgc,
                       XJ_CENTER, YJ_CENTER, plot_x, plot_y, annotation_str);
    }
  }
    
}

/*****************************************
 * draw_forecast_vectors()
 */

static void draw_forecast_vectors(int dev,
				  gframe_t *frame,
				  GC forecast_vector_gc,
				  MdvxProj &storm_proj,
				  const track_file_entry_t *entry,
				  const storm_file_global_props_t *gprops,
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
      uGetParamDouble(Glob->prog_name,
		      "arrow_head_km", ARROW_HEAD_KM);
    
    if (Glob->proj.getProjType() == Mdvx::PROJ_LATLON) {
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
  move_x = entry->dval_dt.smoothed_proj_area_centroid_x * dhr;
  move_y = entry->dval_dt.smoothed_proj_area_centroid_y * dhr;

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
    
    plot_volume = gprops->volume + entry->dval_dt.volume * lead_time_hr;
    plot_proj_area =
      gprops->proj_area + entry->dval_dt.proj_area * lead_time_hr;
    
    /*
     * do not plot if volume or area is too low
     */
    
    if (!forecast_valid(entry, plot_volume, plot_proj_area)) {
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
		      MdvxProj &storm_proj,
		      const vector<storm_file_run_t> &runs)
    
{

  /*
   * grid params
   */
  
  Mdvx::coord_t storm_coord = storm_proj.getCoord();
  
  double dx = storm_coord.dx;
  double dy = storm_coord.dy;
  double dz = storm_coord.dz;
  double minx = storm_coord.minx;
  double miny = storm_coord.miny;
  double minz = storm_coord.minz;

  int iz = (int) ((Glob->z_cappi - minz) / dz + 0.5);
    
  for (size_t ii = 0; ii < runs.size(); ii++) {
      
    if (runs[ii].iz == iz || Glob->plot_composite) {
	
      double startx, starty;
      
      convert_xy_coords(storm_proj, Glob->proj,
			minx + ((double) runs[ii].ix -0.5) * dx,
			miny + ((double) runs[ii].iy -0.5) * dy,
			startx, starty);
      
      double width = convert_dist(storm_proj, Glob->proj, runs[ii].n * dx);
      double height = convert_dist(storm_proj, Glob->proj, dy);
      
      if (Glob->fill_runs) {
	GFillRectangle(dev, frame, storm_gc, storm_psgc,
		       startx, starty, width, height);
      } else {
	GDrawRectangle(dev, frame, storm_gc, storm_psgc,
		       startx, starty, width, height);
      }
      
    } /* if (run[ii].iz ... ) */
    
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
			     MdvxProj &storm_proj,
			     const track_file_entry_t *entry,
			     const storm_file_scan_header_t *scan,
			     const storm_file_global_props_t *gprops,
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

  int n_sides = Glob->_dsTitan.storm_params().n_poly_sides;

  TaArray<GPoint> ray_, polygon_;
  GPoint *ray = ray_.alloc(n_sides + 1);
  GPoint *polygon = polygon_.alloc(n_sides + 1);

  /*
   * compute plotting posn depending on forecast_step
   */

  if (forecast_step == 0) {

    convert_xy_coords(storm_proj, Glob->proj,
		      x, y, plot_x, plot_y);
    
    plot_volume = gprops->volume;
    plot_proj_area = gprops->proj_area;

    plot_storm_scale = 1.0;

  } else {

    lead_time_hr = ((double) forecast_step *
		    Glob->forecast_interval / 3600.0);
    
    convert_xy_coords(storm_proj, Glob->proj,
		      x + entry->dval_dt.smoothed_proj_area_centroid_x * lead_time_hr,
		      y + entry->dval_dt.smoothed_proj_area_centroid_y * lead_time_hr,
		      plot_x, plot_y);
    
    plot_volume = gprops->volume + entry->dval_dt.volume * lead_time_hr;
    plot_proj_area =
      gprops->proj_area + entry->dval_dt.proj_area * lead_time_hr;
    
    /*
     * do not plot if volume or area is too low
     */

    if (!forecast_valid(entry, plot_volume, plot_proj_area)) {
      return;
    }

    plot_storm_scale = sqrt(plot_proj_area / gprops->proj_area);
	
  }

  if (dev == PSDEV) {
    PsSetLineStyle(frame->psgc->file, storm_psgc);
  }
    
  if (Glob->track_graphic == ELLIPSES) {

    // render ellipses
    
    if (forecast_step == 0) {
      
      plot_minor_radius =
	convert_dist(storm_proj, Glob->proj, gprops->proj_area_minor_radius);
      plot_major_radius =
	convert_dist(storm_proj, Glob->proj, gprops->proj_area_major_radius);

    } else {
      
      plot_minor_radius =
	convert_dist(storm_proj, Glob->proj,
		     gprops->proj_area_minor_radius * plot_storm_scale);
      plot_major_radius =
	convert_dist(storm_proj, Glob->proj,
		     gprops->proj_area_major_radius * plot_storm_scale);

    }
      
    /*
     * draw in ellipse
     */

    if (Glob->fill_graphic) {
      GFillArc(dev, frame, storm_gc, storm_psgc,
	       plot_x, plot_y, plot_major_radius, plot_minor_radius,
	       START_AZIMUTH, END_AZIMUTH,
	       (90.0 - gprops->proj_area_orientation), NSEGMENTS);
    } else {
      GDrawArc(dev, frame, storm_gc, storm_psgc,
 	       plot_x, plot_y, plot_major_radius, plot_minor_radius,
 	       START_AZIMUTH, END_AZIMUTH,
 	       (90.0 - gprops->proj_area_orientation), NSEGMENTS);
    }
    
  } else if (Glob->track_graphic == POLYGONS) {
    
    // render polygons
    
    poly_delta_az = Glob->_dsTitan.storm_params().poly_delta_az * DEG_TO_RAD;
    theta = Glob->_dsTitan.storm_params().poly_start_az * DEG_TO_RAD;
    dx = scan->grid.dx;
    dy = scan->grid.dy;
    
    for (iray = 0; iray < n_sides; iray++) {
      
      range =
	convert_dist(storm_proj, Glob->proj,
		     gprops->proj_area_polygon[iray] * plot_storm_scale);
      ray[iray].x = range * sin(theta) * dx;
      ray[iray].y = range * cos(theta) * dy;
      polygon[iray].x = plot_x + ray[iray].x;
      polygon[iray].y = plot_y + ray[iray].y;
      
      theta += poly_delta_az;
      
    } /* iray */
    
    polygon[n_sides].x = polygon[0].x;
    polygon[n_sides].y = polygon[0].y;
    
    if (Glob->fill_graphic) {
      GFillPolygon(dev, frame, storm_gc, storm_psgc,
		   polygon, (n_sides + 1),
		   CoordModeOrigin);
    } else {
      GDrawLines(dev, frame, storm_gc, storm_psgc,
		 polygon, (n_sides + 1),
		 CoordModeOrigin);
    }
      
  } else {

    // render small circles to show each node

    double node_radius = Glob->node_icon_diam_km / 2.0;
    
    plot_minor_radius = convert_dist(storm_proj, Glob->proj, node_radius);
    plot_major_radius = plot_minor_radius;
    
    if (Glob->fill_graphic) {
      GFillArc(dev, frame, storm_gc, storm_psgc,
	       plot_x, plot_y, plot_major_radius, plot_minor_radius,
	       START_AZIMUTH, END_AZIMUTH,
	       (90.0 - gprops->proj_area_orientation), NSEGMENTS);
    } else {
      GDrawArc(dev, frame, storm_gc, storm_psgc,
 	       plot_x, plot_y, plot_major_radius, plot_minor_radius,
 	       START_AZIMUTH, END_AZIMUTH,
 	       (90.0 - gprops->proj_area_orientation), NSEGMENTS);
    }
    
  } /* if (Glob->track_graphic == ELLIPSES) */

  /*
   * if required, draw in storm runs
   */
  
  if (Glob->plot_runs) {

    const TitanComplexTrack *ctrack =
      Glob->_dsTitan.complex_tracks()[icomplex];
    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    const TitanTrackEntry *tentry = strack->entries()[ientry];

    draw_runs(dev, frame, storm_gc, storm_psgc,
	      storm_proj, tentry->runs());
    
  } /* if (Glob->plot_runs ... */

  Glob->tracks_plotted = true;

}
  
/*****************
 * draw_vectors()
 *
 */

static void draw_vectors(int dev,
			 gframe_t *frame,
			 int start_scan,
			 int current_scan,
			 int end_scan,
			 MdvxProj &storm_proj,
			 const TitanComplexTrack *ctrack,
			 const TitanPartialTrack &partial)
  
{

  static int first_call = TRUE;
  static double arrow_head;

  GC vector_gc;

  /*
   * initialize
   */

  if (first_call) {

    /*
     * get arrow head length in world coordinates
     */

    arrow_head =
      uGetParamDouble(Glob->prog_name,
		      "arrow_head_km", ARROW_HEAD_KM);

    if (Glob->proj.getProjType() == Mdvx::PROJ_LATLON) {
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
    
  for (size_t isimple = 0;
       isimple < ctrack->simple_tracks().size(); isimple++) {

    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    
    /*
     * loop through the entries, starting at 1 instead of 0
     */
    
    for (size_t ientry = 1; ientry < strack->entries().size(); ientry++) {
      
      const TitanTrackEntry *tentry = strack->entries()[ientry];
      const TitanTrackEntry *prev_tentry = strack->entries()[ientry - 1];
      
      if (tentry->entry().scan_num > start_scan &&
	  tentry->entry().scan_num <= end_scan) {
	
	Glob->tracks_plotted = true;

	double xx1, yy1, xx2, yy2;
  
	convert_xy_coords(storm_proj, Glob->proj,
			  prev_tentry->gprops().proj_area_centroid_x,
			  prev_tentry->gprops().proj_area_centroid_y,
			  xx1, yy1);
      
	convert_xy_coords(storm_proj, Glob->proj,
			  tentry->gprops().proj_area_centroid_x,
			  tentry->gprops().proj_area_centroid_y,
			  xx2, yy2);

	if (tentry->entry().scan_num <= current_scan) {
	  /*
	   * past vectors
	   */
	  if ((vector_gc =
	       select_gc(&tentry->entry(), partial,
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
	       select_gc(&tentry->entry(), partial, 
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
	} /* if (tentry->entry().scan <= current_scan) */

      } /* if (tentry->entry().scan > start_scan ... */

    } /* ientry */

    /*
     * if this simple track has children, draw vectors to each child
     */
	  
    if (strack->simple_params().nchildren > 0) {
      
      const TitanTrackEntry *last_entry =
	strack->entries()[strack->entries().size() - 1];
      
      if (last_entry->entry().scan_num >= end_scan ||
	  last_entry->entry().scan_num < start_scan) {

	/*
	 * this is the last scan to be plotted, or later,
	 * so do not draw vectors to the children
	 */

	continue;

      }
    
      double xx1, yy1, xx2, yy2;
  
      convert_xy_coords(storm_proj, Glob->proj,
			last_entry->gprops().proj_area_centroid_x,
			last_entry->gprops().proj_area_centroid_y,
			xx1, yy1);
      
      for (int ichild = 0;
	   ichild < strack->simple_params().nchildren; ichild++) {
	      
	int child_track_num = strack->simple_params().child[ichild];
	      
	/*
	 * find global props which match the child track num
	 */
	
	for (size_t jsimple = 0;
	     jsimple < ctrack->simple_tracks().size(); jsimple++) {
	  
	  const TitanSimpleTrack *jstrack = ctrack->simple_tracks()[jsimple];
    	  const TitanTrackEntry *child_entry = jstrack->entries()[0];
	  
	  if (child_entry->entry().simple_track_num == child_track_num) {

	    convert_xy_coords(storm_proj, Glob->proj,
			      child_entry->gprops().proj_area_centroid_x,
			      child_entry->gprops().proj_area_centroid_y,
			      xx2, yy2);
	    
	    if (child_entry->entry().scan_num < current_scan) {
	      /*
	       * past vectors
	       */
	      if ((vector_gc =
		   select_gc(&last_entry->entry(), partial,
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
		   select_gc(&last_entry->entry(), partial,
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

	  } // if (child_entry->entry().simple_track_num == child_track_num) {
	  
	} /* jsimple */
	
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

static int find_scan_limits(time_t current_time,
			    const TitanComplexTrack *ctrack,
			    int *start_scan,
			    int *current_scan,
			    int *end_scan,
			    time_t *start_time,
			    time_t *end_time)

{

  /*
   * compute start search time for past track data, and end time for
   * future track data
   */
    
  time_t start_search = 0, end_search = 0;

  if (Glob->plot_past == FALSE) {
    start_search = current_time;
  } else if (Glob->plot_past == PAST_LIMITED) {
    start_search = current_time - Glob->past_plot_period;
  } else if (Glob->plot_past == PAST_ALL) {
    start_search = ctrack->complex_params().start_time;
  }
  
  if (Glob->plot_future == FALSE) {
    end_search = current_time;
  } else if (Glob->plot_future == FUTURE_LIMITED) {
    end_search = current_time + Glob->future_plot_period;
  } else if (Glob->plot_future == FUTURE_ALL) {
    end_search = ctrack->complex_params().end_time;
  }

  *current_scan = 0;
  if (current_time < ctrack->complex_params().start_time) {
    *current_scan = ctrack->complex_params().start_scan - 1;
  }
  if (current_time > ctrack->complex_params().end_time) {
    *current_scan = ctrack->complex_params().end_scan + 1;
  }
  
  /*
   * loop through the simple tracks which make up this complex track
   */
  
  bool start_set = false;

  for (size_t isimple = 0;
       isimple < ctrack->simple_tracks().size(); isimple++) {

    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    int duration = strack->entries().size();
      
    /*
     * loop through the scans
     */
      
    for (int ientry = 0; ientry < duration; ientry++) {
      
      const TitanTrackEntry *tentry = strack->entries()[ientry];

      /*
       * get time for track entry
       */
      
      time_t entry_time = tentry->entry().time;

      if (!start_set) {
	if (entry_time >= start_search) {
	  *start_scan = tentry->entry().scan_num;
	  *start_time = entry_time;
	  start_set = true;
	}
      }
      
      if (entry_time == current_time) {
	*current_scan = tentry->entry().scan_num;
      }

      *end_scan = tentry->entry().scan_num;
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

static int forecast_valid(const track_file_entry_t *entry,
			  double volume, double area)

{

  if (Glob->plot_forecast == FORECAST_ALL) {
    return 1;
  }

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

static const TitanTrackEntry *
next_entry(int scan_num,
	   const TitanComplexTrack *ctrack,
	   int *isimple_p, int *ientry_p)

{

  static int current_scan_num = -1;
  static int start_simple = 0;
  static int start_entry = 0;
  
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
  
  for (size_t isimple = start_simple;
       isimple < ctrack->simple_tracks().size(); isimple++) {

    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    int duration = strack->entries().size();
    
    /*
     * loop through the scans
     */
      
    for (int ientry = start_entry; ientry < duration; ientry++) {
      
      const TitanTrackEntry *tentry = strack->entries()[ientry];

      if (tentry->entry().scan_num == scan_num) {

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

	return (tentry);

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

static GC select_gc(const track_file_entry_t *entry,
		    const TitanPartialTrack &partial,
		    GC highlight_gc,
		    GC dim_gc)
     
{

  int highlight;
  int track_selected = 0;
  time_hist_shmem_t *tshmem = Glob->track_shmem;

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
    if (tshmem->complex_track_num >= 0 &&
	tshmem->simple_track_num >= 0 &&
	partial.entryIncluded(*entry)) {
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

static void set_storm_coord(Mdvx::coord_t &coord)

{

  MEM_zero(coord);
  titan_grid_t &grid = Glob->titan_grid;
  
  coord.proj_origin_lat = grid.proj_origin_lat;
  coord.proj_origin_lon = grid.proj_origin_lon;
  
  coord.minx = grid.minx;
  coord.miny = grid.miny;
  coord.minz = grid.minz;

  coord.dx = grid.dx;
  coord.dy = grid.dy;
  coord.dz = grid.dz;

  coord.sensor_x = grid.sensor_x;
  coord.sensor_y = grid.sensor_y;
  coord.sensor_z = grid.sensor_z;

  coord.sensor_lat = grid.sensor_lat;
  coord.sensor_lon = grid.sensor_lon;

  coord.proj_type = grid.proj_type;
  coord.dz_constant = grid.dz_constant;

  coord.nx = grid.nx;
  coord.ny = grid.ny;
  coord.nz = grid.nz;
  
  coord.nbytes_char = 3 * MDV_COORD_UNITS_LEN;

  memcpy(coord.unitsx, grid.unitsx,
	 MIN(TITAN_GRID_UNITS_LEN, MDV_COORD_UNITS_LEN));
  memcpy(coord.unitsy, grid.unitsy,
	 MIN(TITAN_GRID_UNITS_LEN, MDV_COORD_UNITS_LEN));
  memcpy(coord.unitsz, grid.unitsz,
	 MIN(TITAN_GRID_UNITS_LEN, MDV_COORD_UNITS_LEN));
  
  if (grid.proj_type == TITAN_PROJ_FLAT) {
    coord.proj_params.flat.rotation = grid.proj_params.flat.rotation;
  } else if (grid.proj_type == TITAN_PROJ_LAMBERT_CONF) {
    coord.proj_params.lc2.lat1 = grid.proj_params.lc2.lat1;
    coord.proj_params.lc2.lat2 = grid.proj_params.lc2.lat2;
//     coord.proj_params.lc2.SW_lat = grid.proj_params.lc2.SW_lat;
//     coord.proj_params.lc2.SW_lon = grid.proj_params.lc2.SW_lon;
//     coord.proj_params.lc2.origin_x = grid.proj_params.lc2.origin_x;
//     coord.proj_params.lc2.origin_y = grid.proj_params.lc2.origin_y;
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
	      int *n_tracks_plotted)
     
{

  static date_time_t track_time;

  int plot_forecast;
  int icolor;
  
  time_t start_time = 0, end_time;
  time_t min_start_time = LARGE_LONG;
  time_t max_end_time = 0;
  int start_scan = 0, current_scan, end_scan;

  double x, y;

  GC forecast_storm_gc;
  GC forecast_vector_gc;
  GC storm_gc;

  time_hist_shmem_t *tshmem = Glob->track_shmem;;
  Mdvx::coord_t storm_coord;
  MdvxProj storm_proj;

  if (Glob->verbose) {
    fprintf(stderr, "** render_tracks **\n");
  }

  /*
   * initialize
   */

  set_storm_coord(storm_coord);

  /*
   * find the partial track if required
   */
  
  TitanPartialTrack partial(Glob->debug);
  
  if (tshmem->track_type == PARTIAL_TRACK) {

    if (Glob->debug) {
      fprintf(stderr, "Partial track:\n");
      fprintf(stderr, "  ref_time: %s\n",
	      utimstr(tshmem->partial_track_ref_time));
      fprintf(stderr, "  past_period: %d\n",
	      tshmem->partial_track_past_period);
      fprintf(stderr, "  future_period: %d\n",
	      tshmem->partial_track_future_period);
      fprintf(stderr, "  complex_track_num: %d\n",tshmem->complex_track_num);
      fprintf(stderr, "  simple_track_num: %d\n",tshmem->simple_track_num);
    }

    if (tshmem->complex_track_num >= 0 &&
	tshmem->simple_track_num >= 0) {
      if (partial.identify(tshmem->partial_track_ref_time,
			   tshmem->partial_track_past_period,
			   tshmem->partial_track_future_period,
			   tshmem->complex_track_num,
			   tshmem->simple_track_num,
			   Glob->_dsTitan)) {
	/*
	 * partial track not found - beep
	 */
	/* fprintf(stderr, "\a"); */
      }
    }

  }

  /*
   * set return values
   */
  
  track_time.unix_time = Glob->_dsTitan.getTimeInUse();
  uconvert_from_utime(&track_time);
  *track_time_ptr = &track_time;
  *dbz_threshold = (double) Glob->_dsTitan.storm_params().low_dbz_threshold;
  *n_tracks_plotted = 0;

  /*
   * remove rotation from the storm grid - because the image
   * will not be displayed rotated
   */

  if (storm_coord.proj_type == Mdvx::PROJ_FLAT)
    storm_coord.proj_params.flat.rotation = 0.0;
  storm_proj.init(storm_coord);

  /*
   * loop through the complex tracks
   */

  for (size_t icomplex = 0;
       icomplex < Glob->_dsTitan.complex_tracks().size(); icomplex++) {

    const TitanComplexTrack *ctrack =
      Glob->_dsTitan.complex_tracks()[icomplex];
    
    *n_tracks_plotted += 1;
    int complex_track_num = ctrack->complex_params().complex_track_num;

    /*
     * if only selected track is to be displayed, continue now
     * if track number is not selected track
     */
    
    if (Glob->plot_tracks == SELECTED_TRACK &&
	tshmem->complex_track_num != complex_track_num) {
      continue;
    }
    
    if (find_scan_limits(Glob->_dsTitan.getTimeInUse(), ctrack,
			 &start_scan, &current_scan, &end_scan,
			 &start_time, &end_time)) {
      continue;
    }

    min_start_time = MIN(min_start_time, start_time);
    max_end_time = MAX(max_end_time, end_time);

    /*
     * for past scans, draw in the storm shapes
     */

    next_entry(-1, ctrack, NULL, NULL);
    for (int iscan = start_scan; iscan < current_scan; iscan++) {

      if (Glob->fill_graphic) {
	icolor = iscan % 2;
      } else {
	icolor = 0;
      }

      const TitanTrackEntry *tentry;
      int isimple, ientry;
      while ((tentry = next_entry(iscan, ctrack,
				  &isimple, &ientry)) != NULL) {

	if ((storm_gc =
	     select_gc(&tentry->entry(), partial,
		       Glob->past_storm_gc[icolor],
		       Glob->past_storm_dim_gc[icolor])) != NULL) {

	  x = tentry->gprops().proj_area_centroid_x;
	  y = tentry->gprops().proj_area_centroid_y;

	  draw_storm_shape(dev, frame, storm_gc,
			   &Glob->past_storm_psgc[icolor],
			   storm_proj,
			   &tentry->entry(), 
			   &tentry->scan(),
			   &tentry->gprops(),
			   0, x, y,
			   icomplex, isimple, ientry);
	  
	}

      } /* while */
      
    } /* iscan */

    /*
     * for future scans, draw in the storm shapes
     */

    next_entry(-1, ctrack, NULL, NULL);
    for (int iscan = end_scan; iscan > current_scan; iscan--) {

      if (Glob->fill_graphic) {
	icolor = iscan % 2;
      } else {
	icolor = 0;
      }

      const TitanTrackEntry *tentry;
      int isimple, ientry;
      while ((tentry = next_entry(iscan, ctrack,
				  &isimple, &ientry)) != NULL) {

	if ((storm_gc =
	     select_gc(&tentry->entry(), partial,
		       Glob->future_storm_gc[icolor],
		       Glob->future_storm_dim_gc[icolor])) != NULL) {

	  x = tentry->gprops().proj_area_centroid_x;
	  y = tentry->gprops().proj_area_centroid_y;
	  
	  draw_storm_shape(dev, frame, storm_gc,
			   &Glob->future_storm_psgc[icolor],
			   storm_proj,
			   &tentry->entry(), 
			   &tentry->scan(),
			   &tentry->gprops(),
			   0, x, y,
			   icomplex, isimple, ientry);
	  
	}

      } /* while */
      
    } /* iscan */

    /*
     * draw in the forecast and current storm shape,
     * and forecast vectors
     */

    next_entry(-1, ctrack, NULL, NULL);
    
    const TitanTrackEntry *tentry;
    int isimple, ientry;
    while ((tentry = next_entry(current_scan, ctrack,
				&isimple, &ientry)) != NULL) {

      /*
       * get storm (x, y)
       */

      x = tentry->gprops().proj_area_centroid_x;
      y = tentry->gprops().proj_area_centroid_y;
      
      /*
       * decide whether to plot forecast
       */
      
      if (Glob->plot_forecast == FORECAST_ALL) {
	plot_forecast = TRUE;
      } else if (Glob->plot_forecast == FORECAST_LIMITED &&
		 tentry->entry().forecast_valid) {
	plot_forecast = TRUE;
      } else {
	plot_forecast = FALSE;
      }

      if (plot_forecast) {

	for (int istep = Glob->n_forecast_steps; istep >= 1; istep--) {
	  
	  if (Glob->fill_graphic) {
	    icolor = istep % 2;
	  } else {
	    icolor = 0;
	  }

	  if ((forecast_storm_gc =
	       select_gc(&tentry->entry(), partial,
			 Glob->forecast_storm_gc[icolor],
			 Glob->forecast_storm_dim_gc[icolor])) != NULL) {
	    
	    draw_storm_shape(dev, frame,
			     forecast_storm_gc,
			     &Glob->forecast_storm_psgc[icolor],
			     storm_proj,
			     &tentry->entry(), 
			     &tentry->scan(),
			     &tentry->gprops(),
			     istep, x, y,
			     icomplex, isimple, ientry);
	    
	  }
	  
	} /* istep */
	
      } /* if (plot_forecast) */

      /*
       * draw current shape
       */
      
      if (Glob->plot_current) {
	if ((storm_gc = select_gc(&tentry->entry(), partial,
				  Glob->current_storm_gc,
				  Glob->current_storm_dim_gc)) != NULL) {
	  draw_storm_shape(dev, frame, storm_gc,
			   &Glob->current_storm_psgc,
			   storm_proj,
			   &tentry->entry(), 
			   &tentry->scan(),
			   &tentry->gprops(),
			   0, x, y,
			   icomplex, isimple, ientry);
	}
      } /* if (Glob->plot_current) */
	
      /*
       * draw in forecast vectors
       */
      
      if (plot_forecast) {
	if ((forecast_vector_gc =
	     select_gc(&tentry->entry(), partial,
		       Glob->forecast_vector_gc,
		       Glob->forecast_vector_dim_gc)) != NULL) {
	  draw_forecast_vectors(dev, frame, forecast_vector_gc,
				storm_proj,
				&tentry->entry(), &tentry->gprops(),
				x, y);
	}
      }
      
    } /* while */

    /*
     * draw in vectors
     */
    
    draw_vectors(dev, frame,
		 start_scan, current_scan, end_scan,
		 storm_proj,
		 ctrack, partial);
    
    /*
     * add annotation
     */
      
    next_entry(-1, ctrack, NULL, NULL);

    for (int iscan = start_scan; iscan <= end_scan; iscan++) {

      const TitanTrackEntry *tentry;
      while ((tentry = next_entry(iscan, ctrack,
				  NULL, NULL)) != NULL) {
	
	/*
	 * get storm (x, y)
	 */
	
	x = tentry->gprops().proj_area_centroid_x;
	y = tentry->gprops().proj_area_centroid_y;
	
	if (select_gc(&tentry->entry(), partial,
		      Glob->current_storm_gc,
		      Glob->current_storm_dim_gc) != NULL) {
	  draw_annotation(dev, frame, storm_proj,
			  &tentry->entry(),
			  &tentry->gprops(),
			  partial, x, y);
	}
	
      } /* while */

    } /* iscan */

  } /* icomplex */

  /*
   * set track shmem start time for past track data, and end time for
   * future track data
   */
  
  set_shmem_times(Glob->_dsTitan.getTimeInUse(),
		  min_start_time, max_end_time);
  
}

/***********************
 * set_shmem_times()
 *
 * Setup times in shmem so that time_hist can display the
 * time bars correctly
 *
 */


static void set_shmem_times(time_t current_time,
			    time_t start_time,
			    time_t end_time)

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
