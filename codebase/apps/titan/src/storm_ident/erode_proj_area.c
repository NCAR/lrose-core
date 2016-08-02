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
 * erode_proj_area.c
 *
 * Erode projected area based on morphology.
 *
 * Return pointer to eroded grid.
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

static int Ngrid_alloc = 0;
static ui08 *Comp_grid = NULL;
static ui08 *Edm_grid = NULL;
static ui08 *Refl_margin_grid = NULL;
static ui08 *Morph_grid = NULL;
static ui08 *Eroded_grid = NULL;

static void check_alloc(si32 n, si32 ny);

/*
 * main
 */

ui08 *erode_proj_area(vol_file_handle_t *v_handle,
			storm_ident_grid_t *grid_params,
			si32 nplanes,
			const Clump_order *clump,
			double darea_at_centroid)

{

  static Row_hdr *Rowh = NULL;
  static int Nrows_alloc = 0;
  static Interval *Intervals = NULL;
  static int N_intervals_alloc = 0;

  ui08 *comp, *dbz, *morph, *refl_margin;
  ui08 low_dbz_byte = Glob->low_dbz_byte;
  ui08 *edm;
  
  int intv;
  int i;
  int refl_divisor;
  int erosion_threshold;
  int value;
  
  si32 min_ix = 1000000000;
  si32 min_iy = 1000000000;
  si32 max_ix = 0;
  si32 max_iy = 0;
  si32 n, nx, ny;
  si32 num_intervals;
  si32 n_projected;

  double km_per_grid;
  double projected_area;
  double area_based_threshold;
  double morph_erosion_threshold;
 
  Interval *intvl;

  /*
   * get the limits of the proj area
   */
  
  for (intv = 0; intv < clump->size; intv++) {
    
    intvl = clump->ptr[intv];
    
    min_ix = MIN(intvl->begin, min_ix);
    max_ix = MAX(intvl->end, max_ix);
    min_iy = MIN(intvl->row_in_plane, min_iy);
    max_iy = MAX(intvl->row_in_plane, max_iy);
    
  }

  /*
   * compute grid sizes, and set grid params. Grid must have a
   * row of zeros around it
   */

  min_ix -= 1;
  min_iy -= 1;
  max_ix += 1;
  max_iy += 1;

  nx = max_ix - min_ix + 1;
  ny = max_iy - min_iy + 1;
  n = nx * ny;

  grid_params->nx = nx;
  grid_params->ny = ny;
  grid_params->min_x = Glob->min_x + min_ix * Glob->delta_x;
  grid_params->min_y = Glob->min_y + min_iy * Glob->delta_y;
  grid_params->dx = Glob->delta_x;
  grid_params->dy = Glob->delta_y;
  grid_params->start_ix = min_ix;
  grid_params->end_ix = max_ix;
  grid_params->start_iy = min_iy;
  grid_params->end_iy = max_iy;

  /*
   * compute km per grid
   */

  km_per_grid = km_per_grid_unit(min_iy, max_iy);

  /*
   * check memory allocation
   */

  EG_alloc_rowh((int) ny, &Nrows_alloc, &Rowh);
  check_alloc(n, ny);

  /*
   * zero out grids
   */

  memset((void *) Edm_grid, 0, (int) (n * sizeof(ui08)));
  memset((void *) Comp_grid, 0, (int) (n * sizeof(ui08)));
  memset((void *) Eroded_grid, 0, (int) (n * sizeof(ui08)));
  memset((void *) Morph_grid, 0, (int) (n * sizeof(ui08)));
  memset((void *) Refl_margin_grid, 0, (int) (n * sizeof(ui08)));
  
  /*
   * compute the composite reflectivity grid - this is the
   * max reflectivity at any height
   */

  n_projected = 0;
  for (intv = 0; intv < clump->size; intv++) {
    intvl = clump->ptr[intv];
    dbz = (v_handle->field_plane[Glob->params.dbz_field][intvl->plane +
						Glob->min_valid_layer] +
	   intvl->row_in_plane * Glob->nx + intvl->begin);
    comp = Comp_grid +
      ((intvl->row_in_plane - min_iy) * nx) + intvl->begin - min_ix;
    for (i = 0; i < intvl->len; i++, dbz++, comp++) {
      if (!*comp) {
	n_projected++;
      }
      if (*dbz > *comp) {
	*comp = *dbz;
      }
    } /* i */
  } /* intvl */
      
  /*
   * compute projected area and the morphology threshold
   * The area-based threshold is the square root of the projected
   * area / 2.0. This is the max erosion threshold. The actual
   * threshold used is the min of this and the parameter threshold.
   */

  projected_area = n_projected * darea_at_centroid;
  area_based_threshold = sqrt(projected_area) / 2.0;
  morph_erosion_threshold =
    MIN(Glob->params.morphology_erosion_threshold, area_based_threshold);

  /*
   * get the intervals in comp grid
   */
  
  num_intervals = EG_find_intervals(grid_params->ny, grid_params->nx,
				    Comp_grid,
				    &Intervals, &N_intervals_alloc,
				    Rowh, 1);
  
  /*
   * compute the edm
   */

  
  EG_edm_2d(Rowh, Edm_grid,
	    (int) grid_params->nx, (int) grid_params->ny, 1);

  /*
   * compute derived grids
   */

  refl_divisor = (int)
    (Glob->params.morphology_refl_divisor * km_per_grid + 0.5);
  
  edm = Edm_grid;
  comp = Comp_grid;
  refl_margin = Refl_margin_grid;
  morph = Morph_grid;

  for (i = 0; i < nx * ny; i++, comp++, edm++, morph++, refl_margin++) {
    if (*edm > 100) {
      *edm = 100;
    };
    if (*edm) {
      *refl_margin = (*comp - low_dbz_byte) / refl_divisor;
      *morph = *edm + *refl_margin;
    }
  } /* i */

  /*
   * erode the grid
   */
  
  erosion_threshold = (int)
    (morph_erosion_threshold / km_per_grid + 0.5);

  memcpy((void *) Eroded_grid, (void *) Morph_grid, n);
    
  if (erosion_threshold > 0) {
    EG_erode_lesser_2d(Rowh, Eroded_grid, nx, ny, erosion_threshold);
  }
    
  for (value = erosion_threshold - 1; value > 0; value--) {
    EG_erode_lesser_or_equal_2d(Rowh, Eroded_grid, nx, ny, value);
/*  EG_erode_below_score_2d(Rowh, Eroded_grid, nx, ny, value, 6); */
  }

  EG_erode_bridges_2d(Rowh, Eroded_grid, nx, ny);

  /*
   * update the grids
   */

  if (Glob->params.create_morphology_files) {
    update_morphology_grids(grid_params,
			    Edm_grid, Refl_margin_grid,
			    Morph_grid, Eroded_grid,
			    min_ix, min_iy);
  }
  
  return (Eroded_grid);

}

static void check_alloc(si32 n, si32 ny)

{
  
  /*
   * check memory allocation
   */

  if (n > Ngrid_alloc) {
    if (Ngrid_alloc == 0) {
      Edm_grid = (ui08 *) umalloc((ui32) (n * sizeof(ui08)));
      Comp_grid = (ui08 *) umalloc((ui32) (n * sizeof(ui08)));
      Refl_margin_grid = (ui08 *) umalloc((ui32) (n * sizeof(ui08)));
      Morph_grid = (ui08 *) umalloc((ui32) (n * sizeof(ui08)));
      Eroded_grid = (ui08 *) umalloc((ui32) (n * sizeof(ui08)));
    } else {
      Edm_grid = (ui08 *) urealloc((char *) Edm_grid,
				      (ui32) (n * sizeof(ui08)));
      Comp_grid = (ui08 *) urealloc((char *) Comp_grid,
				      (ui32) (n * sizeof(ui08)));
      Refl_margin_grid = (ui08 *) urealloc((char *) Refl_margin_grid,
					     (ui32) (n * sizeof(ui08)));
      Morph_grid = (ui08 *) urealloc((char *) Morph_grid,
				       (ui32) (n * sizeof(ui08)));
      Eroded_grid = (ui08 *) urealloc((char *) Eroded_grid,
					(ui32) (n * sizeof(ui08)));
    }
    Ngrid_alloc = n;
  }

}
  
