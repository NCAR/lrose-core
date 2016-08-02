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
 * process_this_clump.c
 *
 * Process the storm(s) in a given clump
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_ident.h"

/*
 * file scope
 */

static ui08 *alloc_grid(storm_ident_grid_t *grid_params,
			  si32 nplanes);

static void load_masked_grid(Clump_order *clump,
			     storm_ident_grid_t *grid_params,
			     ui08 *eroded_grid,
			     ui08 *masked_grid);

/*
 * main
 */

si32
process_this_clump(si32 nstorms,
		   Clump_order *clump,
		   vol_file_handle_t *v_handle,
		   storm_file_handle_t *s_handle,
		   si32 nplanes,
		   double clump_vol,
		   double dvol_at_centroid,
		   double darea_at_centroid,
		   double darea_ellipse)
      
{

  static Interval *Intervals = NULL;
  static int N_intervals_alloc = 0;

  static int N_ints_alloc_clump = 0;
  static Clump_order *Sub_clumps = NULL;
  static Interval **Interval_order = NULL;

  static Row_hdr *Rowh = NULL;
  static int Nrows_alloc = 0;

  ui08 *eroded_grid;
  ui08 *masked_grid;

  int nrows_per_plane;
  int nrows_per_vol;
  int ncols;
  int num_intervals;
  int iclump, num_clumps;

  si32 n_added = 0;

  Clump_order *sub_clump;
  storm_ident_grid_t grid_params;

  /*
   * initialize
   */

  memset((void *) &grid_params, 0, sizeof(storm_ident_grid_t));

  /*
   * do we perform morphology check?
   */
  
  if (Glob->params.check_morphology) {

    /*
     * yes, so erode projected area based on morphology
     */

    eroded_grid = erode_proj_area(v_handle, &grid_params, nplanes,
				  clump, darea_at_centroid);

    if (nplanes > 1) {
      
      /*
       * build up 3-d grid using the eroded grid as a mask
       */

      masked_grid = alloc_grid(&grid_params, nplanes);

      /*
       * load masked grid, excluding points in locations which
       * were eroded away
       */
      
      load_masked_grid(clump, &grid_params,
		       eroded_grid, masked_grid);

    } else {

      /*
       * just use eroded grid
       */

      masked_grid = eroded_grid;
      
    }

    /*
     * alloc for rowh structs
     */

    nrows_per_plane = grid_params.ny;
    nrows_per_vol = nrows_per_plane * nplanes;
    ncols = grid_params.nx;
    EG_alloc_rowh((int) nrows_per_vol, &Nrows_alloc, &Rowh);
    
    /*
     * get the intervals in the masked grid
     */
    
    num_intervals = EG_find_intervals_3d(nplanes,
					 nrows_per_vol,
					 nrows_per_plane,
					 ncols,
					 masked_grid,
					 &Intervals, &N_intervals_alloc,
					 Rowh, 1);

    /*
     * allocate space for intervals and sub clumps
     */
    
    EG_alloc_clumps(num_intervals, &N_ints_alloc_clump,
		    &Sub_clumps, &Interval_order);
    
    /*
     * set clump ids to NULL_ID
     */
    
    EG_reset_clump_id(Intervals, num_intervals);
    
    /*
     * clump
     */
    
    num_clumps = EG_rclump_3d(Rowh, nrows_per_plane, nplanes, TRUE, 1,
			      Interval_order, Sub_clumps);
    
    /*
     * compute props for each clump
     */

    sub_clump = Sub_clumps + 1;

    for (iclump = 0; iclump < num_clumps; iclump++, sub_clump++) {

      double sub_clump_size; 
      double sub_dvol_at_centroid;
      double sub_darea_at_centroid;
      double sub_darea_ellipse;

      /*
       * check if clump volume exceeds min, otherwise
       * go to end of loop
       */
      
      vol_and_area_comps(sub_clump, &sub_clump_size, &sub_dvol_at_centroid,
			 &sub_darea_at_centroid, &sub_darea_ellipse);
      
      if (sub_clump_size >= Glob->params.min_storm_size &&
	  sub_clump_size <= Glob->params.max_storm_size) {
      
	n_added += props_compute(nstorms + n_added, sub_clump,
				 v_handle, s_handle, &grid_params,
				 sub_clump_size, sub_dvol_at_centroid,
				 sub_darea_at_centroid, sub_darea_ellipse);

      }

    } /* iclump */
    
  } else {

    /*
     * no morphology check, just pass clump through to props_compute()
     */
    
    grid_params.nx = Glob->nx;
    grid_params.ny = Glob->ny;
    
    grid_params.min_x = Glob->min_x;
    grid_params.min_y = Glob->min_y;
    
    grid_params.dx = Glob->delta_x;
    grid_params.dy = Glob->delta_y;
    
    grid_params.start_ix = 0;
    grid_params.start_iy = 0;
    
    grid_params.end_ix = 0;
    grid_params.end_iy = 0;
    
    n_added = props_compute(nstorms, clump,
			    v_handle, s_handle, &grid_params,
			    clump_vol, dvol_at_centroid,
			    darea_at_centroid, darea_ellipse);

  }
    
  return (n_added);

}

/**************************
 * allocate the masked grid
 */
  
static ui08 *alloc_grid(storm_ident_grid_t *grid_params,
			  si32 nplanes)

{

  static int Ngrid_alloc = 0;
  static ui08 *Masked_grid;
  int n;

  n = grid_params->nx * grid_params->ny * nplanes;

  if (n > Ngrid_alloc) {
    if (Ngrid_alloc == 0) {
      Masked_grid = (ui08 *) umalloc((ui32) n);
    } else {
      Masked_grid = (ui08 *) urealloc((char *) Masked_grid, (ui32) n);
    }
    Ngrid_alloc = n;
  }

  memset((void *) Masked_grid, 0, n);

  return (Masked_grid);

}

/*************************
 * load up the masked grid
 *
 * Mask out those points in the original clump which have
 * been eroded and are no longer in eroded_grid[].
 */

static void load_masked_grid(Clump_order *clump,
			     storm_ident_grid_t *grid_params,
			     ui08 *eroded_grid,
			     ui08 *masked_grid)

{
  
  ui08 *eroded;
  ui08 *masked;
  int intv, ix, iy;
  int nx = grid_params->nx;
  int plane_offset, vol_offset;
  int start_ix = grid_params->start_ix;
  int start_iy = grid_params->start_iy;
  si32 nbytes_plane;
  Interval *intvl;

  nbytes_plane = grid_params->nx * grid_params->ny;
  
  for (intv = 0; intv < clump->size; intv++) {
    
    intvl = clump->ptr[intv];
    iy = intvl->row_in_plane - start_iy;
    plane_offset = iy * nx + intvl->begin - start_ix;
    vol_offset = intvl->plane * nbytes_plane + plane_offset;
    eroded = eroded_grid + plane_offset;
    masked = masked_grid + vol_offset;

    for (ix = intvl->begin; ix <= intvl->end;
	 ix++, eroded++, masked++) {

      if (*eroded) {
	*masked = 1;
      }

    } /* ix */
    
  } /* intv */

}


