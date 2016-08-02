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
/*************************************************************************
 * area_comps.c
 *
 * Compute areal properties
 *
 * Mike Dixon RAP NCAR Boulder Colorado USA
 *
 * April 1995
 *
 *************************************************************************/

#include "storm_ident.h"
#include <euclid/boundary.h>
#include <euclid/point.h>
#include <euclid/node.h>

/*
 * file scope
 */

static double Dbz_scale, Dbz_bias;
static double Z_p_inverse_coeff, Z_p_inverse_exponent;
static double Z_factor;
static dbz_hist_entry_t *Dbz_hist;

static si32 Min_ix, Min_iy, Max_ix, Max_iy;
static Interval *Intervals = NULL;
static int N_intervals_alloc = 0;
static Row_hdr *Rowh = NULL;
static int Nrows_alloc = 0;
static si32 Num_intervals;
static si32 N, Nx, Ny;
static int Ngrid_alloc = 0;
static ui08 *CompGrid = NULL;
static ui08 *PrecipGrid = NULL;

/*
 * prototypes
 */

static void compute_bdry(Row_hdr  *rowh,
			 si32 ny, si32 nx,
			 Point_d *ref_pt,
			 fl32 *star_ray);

static void compute_proj_polygon(storm_file_global_props_t *gprops,
				 storm_ident_grid_t *grid_params,
				 ui08 *grid);

static void precip_and_area_hist(storm_ident_grid_t *gparams,
				 storm_file_global_props_t *gprops,
				 double darea_at_centroid);
     
/*
 * initialize some statics
 */

void init_area_comps(double dbz_scale,
		     double dbz_bias,
		     dbz_hist_entry_t *dbz_hist,
		     double z_p_inverse_coeff,
		     double z_p_inverse_exponent)

{

  Dbz_scale = dbz_scale;
  Dbz_bias = dbz_bias;
  Dbz_hist = dbz_hist;
  Z_p_inverse_coeff = z_p_inverse_coeff;
  Z_p_inverse_exponent = z_p_inverse_exponent;
  Z_factor = pow(Z_p_inverse_coeff, Z_p_inverse_exponent);
}

/*
 * main
 */

void area_comps(const Clump_order *clump,
		storm_ident_grid_t *gparams,
		storm_file_global_props_t *gprops,
		double darea_at_centroid,
		double darea_ellipse)

{

  int multiple_layers = FALSE;
  int intv, offset;
  Interval *intvl;
  storm_ident_grid_t loc_grid;

  /*
   * initialize
   */

  Min_ix = 1000000000;
  Min_iy = 1000000000;
  Max_ix = 0;
  Max_iy = 0;

  /*
   * get the limits of the proj area
   */
  
  for (intv = 0; intv < clump->size; intv++) {
    
    intvl = clump->ptr[intv];
    
    Min_ix = MIN(intvl->begin, Min_ix);
    Max_ix = MAX(intvl->end, Max_ix);
    Min_iy = MIN(intvl->row_in_plane, Min_iy);
    Max_iy = MAX(intvl->row_in_plane, Max_iy);
    
  }

  gprops->bounding_min_ix = Min_ix + gparams->start_ix;
  gprops->bounding_min_iy = Min_iy + gparams->start_iy;
  gprops->bounding_max_ix = Max_ix + gparams->start_ix;
  gprops->bounding_max_iy = Max_iy + gparams->start_iy;

  /*
   * compute grid sizes, and set grid params
   */

  Nx = Max_ix - Min_ix + 1;
  Ny = Max_iy - Min_iy + 1;
  N = Nx * Ny;

  loc_grid.nx = Nx;
  loc_grid.ny = Ny;
  loc_grid.min_x = gparams->min_x + Min_ix * gparams->dx;
  loc_grid.min_y = gparams->min_y + Min_iy * gparams->dy;
  loc_grid.dx = gparams->dx;
  loc_grid.dy = gparams->dy;

  /*
   * check memory allocation
   */

  if (N > Ngrid_alloc) {
    if (CompGrid == NULL) {
      CompGrid = (ui08 *) umalloc(N);
      PrecipGrid = (ui08 *) umalloc(N);
    } else {
      CompGrid = (ui08 *) urealloc(CompGrid, N);
      PrecipGrid = (ui08 *) urealloc(PrecipGrid, N);
    }
    Ngrid_alloc = N;
  }

  /*
   * zero out grid for proj area comps
   */

  memset(CompGrid, 0, N);
  
  /*
   * load up grid with 1's to indicate projected area
   */
  
  for (intv = 0; intv < clump->size; intv++) {
    intvl = clump->ptr[intv];
    if (intvl->plane != 0) {
      multiple_layers = TRUE;
    }
    offset =
      ((intvl->row_in_plane - Min_iy) * Nx) + (intvl->begin - Min_ix);
    memset((CompGrid + offset), 1, intvl->len);
  }
  
  /*
   * projected area comps
   */
  
  ellipse_compute(&loc_grid, CompGrid,
		  darea_at_centroid, darea_ellipse,
		  &gprops->proj_area,
		  &gprops->proj_area_centroid_x,
		  &gprops->proj_area_centroid_y,
		  &gprops->proj_area_orientation,
		  &gprops->proj_area_major_radius,
		  &gprops->proj_area_minor_radius);

  /*
   * compute polygon for projected area
   */

  compute_proj_polygon(gprops, &loc_grid, CompGrid);

  if (multiple_layers) {

    /*
     * zero out grid for precip area comps
     */

    memset(PrecipGrid, 0, N);
  
    /*
     * load up grid with 1's to indicate precip area
     */
  
    for (intv = 0; intv < clump->size; intv++) {
      intvl = clump->ptr[intv];
      if (intvl->plane == 0) {
	offset =
	  ((intvl->row_in_plane - Min_iy) * Nx) + (intvl->begin - Min_ix);
	memset((void *) (PrecipGrid + offset), 1, (int) intvl->len);
      }
    }
    
    /*
     * precip area comps
     */

    ellipse_compute(&loc_grid, PrecipGrid,
		    darea_at_centroid, darea_ellipse,
		    &gprops->precip_area,
		    &gprops->precip_area_centroid_x,
		    &gprops->precip_area_centroid_y,
		    &gprops->precip_area_orientation,
		    &gprops->precip_area_major_radius,
		    &gprops->precip_area_minor_radius);
    
  } else {

    /*
     * only one layer - copy from proj area
     */

    gprops->precip_area = gprops->proj_area;
    gprops->precip_area_centroid_x = gprops->proj_area_centroid_x;
    gprops->precip_area_centroid_y = gprops->proj_area_centroid_y;
    gprops->precip_area_orientation = gprops->proj_area_orientation;
    gprops->precip_area_major_radius = gprops->proj_area_major_radius;
    gprops->precip_area_minor_radius = gprops->proj_area_minor_radius;

  } 

  /*
   * compute precip from composite, and load up
   * dbz histogram for composite area
   */

  precip_and_area_hist(gparams, gprops, darea_at_centroid);

  return;

}

static void compute_proj_polygon(storm_file_global_props_t *gprops,
				 storm_ident_grid_t *loc_grid,
				 ui08 *grid)

{
  
  Point_d ref_pt;

  /*
   * check memory allocation
   */

  EG_alloc_rowh((int) loc_grid->ny, &Nrows_alloc, &Rowh);

  /*
   * get the intervals
   */
  
  Num_intervals = EG_find_intervals(loc_grid->ny, loc_grid->nx,
				    grid, &Intervals, &N_intervals_alloc,
				    Rowh, 1);

  /*
   * compute the proj area centroid as a reference point for
   * the shape star - we add 0.5 because the star is computed
   * relative to the lower-left corner of the grid, while the
   * other computations are relative to the center of the
   * grid rectangles.
   */
  
  ref_pt.x = 0.5 +
    (gprops->proj_area_centroid_x - loc_grid->min_x) / loc_grid->dx;
  ref_pt.y = 0.5 +
    (gprops->proj_area_centroid_y - loc_grid->min_y) / loc_grid->dy;

  /*
   * compute the boundary
   */

  compute_bdry(Rowh, loc_grid->ny, loc_grid->nx,
	       &ref_pt, (fl32 *) gprops->proj_area_polygon);
  
  return;

}

static void compute_bdry(Row_hdr  *rowh,
			 si32 ny,
			 si32 nx,
			 Point_d *ref_pt,
			 fl32 *proj_area_polygon)
{
  
  static int *Bdry_list = NULL;
  static Point_d *Bdry_pts = NULL;
  static Node *Nodes = NULL;
  static int Nnodes_allocated = 0;

  int i;
  int num_nodes;
  int bdry_size;
  
  double theta;
  Star_point star_ray[N_POLY_SIDES + 1];
  Point_d ray[N_POLY_SIDES];

  num_nodes = 4 * Num_intervals;
  
  /*
   * allocate memory for arrays and initialize
   */

  EG_alloc_nodes(num_nodes, &Nnodes_allocated,
		 &Bdry_list, &Bdry_pts, &Nodes);

  /*
   * call bdry_graph to set up the graph for the boundary
   */
  
  if (EG_bdry_graph(rowh, ny, nx, Nodes, num_nodes, 0)) {
    fprintf(stderr, "ERROR - %s:compute_bdry\n", Glob->prog_name);
    fprintf(stderr, "Cannot compute boundary\n");
    return;
  }

  /*
   * traverse the graph to determine the boundary
   */
  
  bdry_size = EG_traverse_bdry_graph(Nodes, 2L, Bdry_list);

  /*
   * initialize the array of rays
   */

  theta = EG_init_ray_TN(ray, (int) N_POLY_SIDES, ref_pt);

  /*
   * generate the array of boundary points from the array of
   * boundary nodes and a bdry_list
   */
  
  EG_gen_bdry(Bdry_pts, Nodes, Bdry_list, bdry_size);
  
  /*
   * determine the intersections of the rays with the boundary and
   * store this information in star_ray
   */
  
  EG_make_star_TN(Bdry_pts, bdry_size, ray,
		  N_POLY_SIDES, theta, ref_pt,
		  star_ray);

  for (i = 0; i < N_POLY_SIDES; i++) {
    proj_area_polygon[i] = star_ray[i].r;
  }

  return;

}

/*******************
 * store_proj_runs()
 */

si32 store_proj_runs(storm_file_handle_t *s_handle,
		     storm_ident_grid_t *gparams)

{

  int irun;
  int start_ix = gparams->start_ix + Min_ix;
  int start_iy = gparams->start_iy + Min_iy;
  Interval *intvl;
  storm_file_run_t *run;
  
  RfAllocStormProps(s_handle, Glob->nz,
		    Glob->n_dbz_hist_intervals,
		    0L, Num_intervals,
		    "props_compute");
  
  run = s_handle->proj_runs;
  intvl = Intervals;
  
  for (irun = 0; irun < Num_intervals; irun++, run++, intvl++) {
    
    run->ix = intvl->begin + start_ix;
    run->iy = intvl->row_in_plane + start_iy;
    run->iz = 0;
    run->n = intvl->len;
	
  } /* intv */

  return (Num_intervals);

}

/******************************************************
 * compute precip and area dbz histogram
 */

static void precip_and_area_hist(storm_ident_grid_t *gparams,
				 storm_file_global_props_t *gprops,
				 double darea_at_centroid)
     
{

  ui08 *comp_dbz, *dbz;
  ui08 *cg;
  int iy, ix, index;
  int dbz_intvl;
  double r_dbz, refl, precip_flux_factor;
  double sum_factor;
  double n = 0.0;

  sum_factor = 0.0;
  comp_dbz = get_comp_grid();
  cg = CompGrid;
  
  for (iy = 0; iy < Ny; iy++) {
    
    index = (iy + Min_iy + gparams->start_iy) * Glob->nx +
      Min_ix + gparams->start_ix;
    dbz = comp_dbz + index;

    for (ix = 0; ix < Nx; ix++, cg++, dbz++) {
      
      /*
       * if this point is in the composite shape, add
       * in the precip factor
       */
      
      if (*cg) {

	r_dbz = ((double) *dbz) * Dbz_scale + Dbz_bias;
	refl = pow(10.0, r_dbz / 10.0);
	precip_flux_factor = pow(refl, Z_p_inverse_exponent);
	sum_factor += precip_flux_factor;
	
	/*
	 * load up area dbz histogram counts
	 */
	
	dbz_intvl = Glob->dbz_interval[*dbz];
	if (dbz_intvl >= 0) {
	  Dbz_hist[dbz_intvl].n_area++;
	  n++;
	}
	
      }

    } /* ix */

  } /* iy */
  
  /*
   * compute precip flox in m/s
   */
  
  gprops->precip_flux = ((sum_factor * darea_at_centroid * Z_factor) / 3.6);
  
  /*
   * load area dbz histograms
   */
  
  for (dbz_intvl = 0; dbz_intvl < Glob->n_dbz_hist_intervals; dbz_intvl++) {
    if (Dbz_hist[dbz_intvl].n_area > 0) {
      Dbz_hist[dbz_intvl].percent_area = 100.0 *
	(double) Dbz_hist[dbz_intvl].n_area / n;
    } else {
      Dbz_hist[dbz_intvl].percent_area = 0.0;
    }
  }
      
  return;

}


