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
 * do_print.c
 *
 * Print out prod array
 *
 * RAP, NCAR, Boulder CO
 *
 * march 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "tprod_print.h"

typedef struct {
  double lon;
  double lat;
} LonLat;

#define NPOINTS_ELLIPSE 36

static LonLat *
compute_ellipse(tdata_product_ellipse_t *ellipse,
		double centerx, double centery,
		double growth, int flat_proj,
		double ellipse_scale, double short_angle_scale,
		double *maj_radius_p,
		double *min_radius_p, double *axis_rotation_p);

static LonLat *
compute_polygon(tdata_product_polygon_t *polygon, int n_sides, 
		double start_az, double delta_az,
		double grid_dx, double grid_dy,
		double centerx, double centery,
		double growth, int flat_proj,
		double radial_multiplier_scale);
     
static void print_ellipse(LonLat *pts,
			  double maj_radius,
			  double min_radius,
			  double axis_rotation,
			  int comments);

static void print_polygon(LonLat *pts,
			  int n_poly_sides, int comments);

void do_print(void *data_buf)

{

  char *mess_ptr;

  int ientry;
  int polygon_size;
  int n_poly_sides;
  int flat_proj;
  int comments = Glob->comments;

  double poly_start_az;
  double poly_delta_az;
  double grid_dx;
  double grid_dy;
  double speed_kmh, dist_km;
  double dirn_true;
  double currentlon, currentlat;
  double forecastlon, forecastlat;
  double forecast_lead_time_hr;
  double area_factor, linear_factor;
  double angle_scale, short_angle_scale;
  double ellipse_scale;
  double radial_multiplier_scale;
  double maj_radius, min_radius;
  double axis_rotation;
  
  tdata_product_header_t *header;
  tdata_product_entry_t *entry;
  tdata_product_ellipse_t *ellipse;
  tdata_product_polygon_t *polygon;

  LonLat *pts;
  
  /*
   * set pointers
   */
  
  mess_ptr = data_buf;
  header = (tdata_product_header_t *) mess_ptr;
  mess_ptr += sizeof(tdata_product_header_t);
  
  XDRU_tohl((u_long *) header,
	    (u_long) sizeof(tdata_product_header_t));

  /* initialize */

  if (Glob->lead_time >= 0) {
    forecast_lead_time_hr = Glob->lead_time / 3600.0;
  } else {
    forecast_lead_time_hr = (double) header->forecast_lead_time / 3600.0;
  }

  polygon_size = sizeof(long) + header->n_poly_sides * sizeof(u_char);

  angle_scale = (double) header->angle_scale;
  short_angle_scale = (double) header->short_angle_scale;
  radial_multiplier_scale = (double) header->radial_multiplier_scale;

  if (header->grid_type == MDV_PROJ_FLAT) {
    flat_proj = TRUE;
    ellipse_scale = (double) header->flat_proj_ellipse_scale;
  } else {
    flat_proj = FALSE;
    ellipse_scale = (double) header->latlon_proj_ellipse_scale;
  }

  if (header->plot_polygon) {

    n_poly_sides = header->n_poly_sides;
    poly_start_az =
      ((double) header->poly_start_az / angle_scale) * RAD_PER_DEG;
    poly_delta_az =
      ((double) header->poly_delta_az / angle_scale) * RAD_PER_DEG;
    grid_dx = (double) header->grid_dx / (double) header->grid_scalex;
    grid_dy = (double) header->grid_dy / (double) header->grid_scaley;
    
  } /* if (header->draw_polygon) */

  /*
   * draw for each storm entry
   */

  if (comments) {
    fprintf(stdout, "###################################################\n");
    fprintf(stdout, "##  Time: %s\n", utimstr(header->time));
  }
  fprintf(stdout, "    %ld\n", (long) header->time);

  if (comments) {
    fprintf(stdout, "##  Forecast lead time:\n");
  }
  fprintf(stdout, "    %g\n", forecast_lead_time_hr * 60.0);

  if (comments) {
    fprintf(stdout, "##  Nstorms:\n");
  }
  fprintf(stdout, "    %d\n", (int) header->n_entries);
  
  for (ientry = 0; ientry < header->n_entries; ientry++)  {
    
    if (comments) {
      fprintf(stdout, "##---------------\n");
      fprintf(stdout, "##  Storm number:\n");
    }
    fprintf(stdout, "    %d\n", ientry);
    
    /*
     * set pointers into buffer, incrementing buffer 
     * pointer accordingly
     */

    entry = (tdata_product_entry_t *) mess_ptr;
    mess_ptr += sizeof(tdata_product_entry_t);

    XDRU_tohl((u_long *) &entry->longitude,
	      (u_long) sizeof(long));

    XDRU_tohl((u_long *) &entry->latitude,
	      (u_long) sizeof(long));

    XDRU_tohs((u_short *) &entry->direction,
	      (u_short) sizeof(u_short));

    XDRU_tohs((u_short *) &entry->speed,
	      (u_short) sizeof(u_short));

    ellipse = (tdata_product_ellipse_t *) NULL;
    if (header->ellipse_included) {

      ellipse = (tdata_product_ellipse_t *) mess_ptr;
      mess_ptr += sizeof(tdata_product_ellipse_t);

      XDRU_tohs((u_short *) &ellipse->norm_darea_dt,
		(u_short) sizeof(u_short));
      XDRU_tohs((u_short *) &ellipse->orientation,
		(u_short) sizeof(u_short));
      XDRU_tohs((u_short *) &ellipse->minor_radius,
		(u_short) sizeof(u_short));
      XDRU_tohs((u_short *) &ellipse->major_radius,
		(u_short) sizeof(u_short));

    }

    polygon = (tdata_product_polygon_t *) NULL;
    if (header->polygon_included) {

      polygon = (tdata_product_polygon_t *) mess_ptr;
      mess_ptr += polygon_size;

      XDRU_tohl((u_long *) &polygon->radial_multiplier,
		(u_long) sizeof(long));
      
    }

    /*
     * compute speed and direction
     */

    currentlon = (double) entry->longitude / angle_scale;
    currentlat = (double) entry->latitude / angle_scale;
    
    speed_kmh = (double) entry->speed / (double) header->speed_scale;
    dist_km = speed_kmh * forecast_lead_time_hr;
    
    dirn_true = (double) entry->direction / short_angle_scale;

    PJGLatLonPlusRTheta(currentlat, currentlon,
			dist_km, dirn_true,
			&forecastlat, &forecastlon);

    if (comments) {
      fprintf(stdout, "##  Simple track number, complex track number:\n");
    }
    fprintf(stdout, "    %d %d\n",
	    entry->simple_track_num,
	    entry->complex_track_num);

    if (comments) {
      fprintf(stdout, "##  Top (km):\n");
    }
    fprintf(stdout, "    %d\n", entry->top);

    if (comments) {
      fprintf(stdout, "##  Current lon, lat:\n");
    }
    fprintf(stdout, "    %g %g\n", currentlon, currentlat);

    if (comments) {
      fprintf(stdout, "##  speed kmh, dist km:\n");
    }
    fprintf(stdout, "    %g %g\n", speed_kmh, dist_km);

    if (comments) {
      fprintf(stdout, "##  dirn_true:\n");
    }
    fprintf(stdout, "    %g\n", dirn_true);

    if (comments) {
      fprintf(stdout, "##  Forecast lon, lat\n");
    }
    fprintf(stdout, "    %g %g\n", forecastlon, forecastlat);
    
    /*
     * print current ellipse position as applicable
     */
    
    if (header->plot_current && header->plot_ellipse && ellipse) {
      
      /*
       * compute and print ellipse
       */
      
      pts = compute_ellipse(ellipse,
			    currentlon, currentlat,
			    1.0, flat_proj,
			    ellipse_scale, short_angle_scale,
			    &maj_radius, &min_radius, &axis_rotation);
      
      if (comments) {
	fprintf(stdout, "##  Current ellipse:\n");
      }
      fprintf(stdout, "    1\n");
      
      print_ellipse(pts, maj_radius, min_radius, axis_rotation,
		    comments);

    } else {
      
      if (comments) {
	fprintf(stdout, "##  Current ellipse:\n");
      }
      fprintf(stdout, "    0\n");
      
    } /* if (header->plot_current ... ) */
    
    /*
     * print current polygon position as applicable
     */
    
    if (header->plot_current && header->plot_polygon && polygon) {

      /*
       * compute and print polygon
       */
      
      pts = compute_polygon(polygon,
			    n_poly_sides, poly_start_az, poly_delta_az,
			    grid_dx, grid_dy,
			    currentlon, currentlat,
			    1.0, flat_proj,
			    radial_multiplier_scale);
      
      if (comments) {
	fprintf(stdout, "##  Current polygon:\n");
      }
      fprintf(stdout, "    1\n");
      
      print_polygon(pts, n_poly_sides, comments);
      
    } else {

      if (comments) {
	fprintf(stdout, "##  Current polygon:\n");
      }
      fprintf(stdout, "    0\n");
      
    } /* if (header->plot_current ... */

    if (ellipse) {
      /*
       * compute growth factor
       */
      area_factor = 1.0 +
	(((double) ellipse->norm_darea_dt /
	  (double) header->darea_dt_scale) * forecast_lead_time_hr);
      linear_factor = sqrt(area_factor);
    } else {
      area_factor = -1.0;
    }

    /*
     * print forecast ellipse as applicable
     */
    
    if (header->plot_forecast && (area_factor > 0.0) &&
	header->plot_ellipse) {
	  
      /*
       * compute and print ellipse
       */
      
      pts = compute_ellipse(ellipse,
			    forecastlon, forecastlat,
			    linear_factor, flat_proj,
			    ellipse_scale, short_angle_scale,
			    &maj_radius, &min_radius, &axis_rotation);
      
      if (comments) {
	fprintf(stdout, "##  Forecast ellipse:\n");
      }
      fprintf(stdout, "    1\n");
      
      print_ellipse(pts, maj_radius, min_radius, axis_rotation,
		    comments);

    } else {
      
      if (comments) {
	fprintf(stdout, "##  Forecast ellipse:\n");
      }
      fprintf(stdout, "    0\n");

    } /* if (header->plot_forecast ... */
    
    if (header->plot_forecast && (area_factor > 0.0) &&
	header->plot_polygon) {
      
      /*
       * compute and print polygon
       */
      
      pts = compute_polygon(polygon,
			    n_poly_sides, poly_start_az, poly_delta_az,
			    grid_dx, grid_dy,
			    forecastlon, forecastlat,
			    linear_factor, flat_proj,
			    radial_multiplier_scale);
      
      if (comments) {
	fprintf(stdout, "##  Forecast polygon:\n");
      }
      fprintf(stdout, "    1\n");
	
      print_polygon(pts, n_poly_sides, comments);

    } else {
    
      if (comments) {
	fprintf(stdout, "##  Forecast polygon:\n");
      }
      fprintf(stdout, "    0\n");
    
    } /* if (header->plot_forecast .... ) */

  } /* ientry */

  fflush(stdout);
  
}

/*****************************
 * function to compute ellipse
 *
 * Loads up maj_radius_p, min_radius_p and axis_rotation_p.
 *
 * Returns boundary pts array, size NPOINTS_ELLIPSE.
 */

static LonLat *
compute_ellipse(tdata_product_ellipse_t *ellipse,
		double centerx, double centery,
		double growth, int flat_proj,
		double ellipse_scale, double short_angle_scale,
		double *maj_radius_p,
		double *min_radius_p, double *axis_rotation_p)
     
{
  
  long i;
  
  double delta_theta;
  double theta;
  double phi;
  double deltax, deltay;
  double radius;
  double ratio_factor;
  double wangle1 = 0.0, wangle2 = 360.0;
  
  LonLat *pt;
  static LonLat pts[NPOINTS_ELLIPSE];

  *maj_radius_p =
    ((double) ellipse->major_radius / ellipse_scale) * growth;
  *min_radius_p =
    ((double) ellipse->minor_radius / ellipse_scale) * growth;
  *axis_rotation_p =
    (90.0 - ((double) ellipse->orientation / short_angle_scale));

  ratio_factor = pow(*min_radius_p / *maj_radius_p, 2.0) - 1.0;
  
  /* set up start angles  */
  
  delta_theta = (((wangle2 - wangle1) * RAD_PER_DEG) /
		 (double) NPOINTS_ELLIPSE);

  theta = wangle1 * RAD_PER_DEG;
  phi = theta + *axis_rotation_p * RAD_PER_DEG;
  
  pt = pts;
  
  /*
   * generate the ellipse segments
   */
  
  if (flat_proj) {
    PJGflat_init(centery, centerx, 0.0);
  }
  
  for (i = 0; i < NPOINTS_ELLIPSE;
       i++, pt++, theta += delta_theta, phi += delta_theta) {
    
    radius = *min_radius_p / sqrt(1.0 + pow(cos(theta), 2.0) *
				  ratio_factor);
    
    if (flat_proj) {
      deltax = radius * cos(phi);
      deltay = radius * sin(phi);
      PJGflat_xy2latlon(deltax, deltay, &pt->lat, &pt->lon);
    } else {
      pt->lon = centerx + radius * cos(phi);
      pt->lat = centery + radius * sin(phi);
    }
    
  } /* i */

  return (pts);
  
}


/***************************
 * function to print polygon
 */

static LonLat *
compute_polygon(tdata_product_polygon_t *polygon, int n_sides, 
		double start_az, double delta_az,
		double grid_dx, double grid_dy,
		double centerx, double centery,
		double growth, int flat_proj,
		double radial_multiplier_scale)
     
{

  static int npts_alloc = 0;
  static LonLat * pts = NULL;
  
  u_char *ray;
  int iray;
  double range, theta;
  double deltax, deltay;
  double plot_x, plot_y;
  double scale;
  LonLat *pt;
  
  /*
   * alloc space for pts array
   */

  if (npts_alloc == 0) {
    npts_alloc = n_sides;
    pts = (LonLat *) umalloc(npts_alloc * sizeof(LonLat));
  } else {
    if (npts_alloc < n_sides) {
      npts_alloc = n_sides;
      pts = (LonLat *) urealloc(pts, npts_alloc * sizeof(LonLat));
    }
  }
  
  pt = pts;
  theta = start_az;
  scale = ((double) polygon->radial_multiplier /
	   radial_multiplier_scale) * growth;
  ray = polygon->radials;

  if (flat_proj) {
    PJGflat_init(centery, centerx, 0.0);
  }
  
  for (iray = 0; iray < n_sides;
       iray++, pt++, theta += delta_az, ray++) {
    
    range = ((double) *ray) * scale;

    if (flat_proj) {

      deltax = range * sin(theta) * grid_dx;
      deltay = range * cos(theta) * grid_dy;
      PJGflat_xy2latlon(deltax, deltay, &plot_y, &plot_x);

    } else {

      plot_x = centerx + range * sin(theta) * grid_dx;
      plot_y = centery + range * cos(theta) * grid_dy;

    }

    pt->lon = plot_x;
    pt->lat = plot_y;
    
  } /* iray */

  return (pts);
  
}

static void print_ellipse(LonLat *pts,
			  double maj_radius,
			  double min_radius,
			  double axis_rotation,
			  int comments)

{

  int i;
  
  if (comments) {
    fprintf(stdout, "##    Major_radius, minor_radius, rotation:\n");
  }
  fprintf(stdout, "      %g %g %g\n",
	  maj_radius, min_radius, axis_rotation);
  
  if (comments) {
    fprintf(stdout, "##    Npoints ellipse:\n");
  }
  fprintf(stdout, "      %d\n", NPOINTS_ELLIPSE);
  if (comments) {
    fprintf(stdout, "##    Ellipse points:\n");
  }
  if (Glob->full) {
    for (i = 0; i < NPOINTS_ELLIPSE; i++) {
      fprintf(stdout, "%g %g\n", pts[i].lon, pts[i].lat);
    }
  } else {
    for (i = 0; i < NPOINTS_ELLIPSE; i++) {
      fprintf(stdout, "(%g %g) ", pts[i].lon, pts[i].lat);
    }
    fprintf(stdout, "\n");
  }

}

static void print_polygon(LonLat *pts,
			  int n_poly_sides, int comments)

{

  int i;

  if (comments) {
    fprintf(stdout, "##    Npoints polygon:\n");
  }
  fprintf(stdout, "      %d\n", n_poly_sides);
  
  if (comments) {
    fprintf(stdout, "##    Polygon points:\n");
  }
  if (Glob->full) {
    for (i = 0; i < n_poly_sides; i++) {
      fprintf(stdout, "%g %g\n", pts[i].lon, pts[i].lat);
    }
  } else {
    for (i = 0; i < n_poly_sides; i++) {
      fprintf(stdout, "(%g %g) ", pts[i].lon, pts[i].lat);
    }
    fprintf(stdout, "\n");
  }

}

