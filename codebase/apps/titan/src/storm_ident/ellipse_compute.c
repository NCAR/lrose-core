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
 * ellipse_compute.c
 *
 * Compute ellipse from  principal component analysis
 *
 * Note: area is in sq km.
 *       ellipse_area is in sq grid_units.
 *
 * Mike Dixon RAP NCAR Boulder Colorado USA
 *
 * April 1995
 *
 *************************************************************************/

#include "storm_ident.h"

void ellipse_compute(storm_ident_grid_t *grid_params,
		     ui08 *grid,
		     double darea_at_centroid,
		     double darea_ellipse,
		     fl32 *area,
		     fl32 *area_centroid_x,
		     fl32 *area_centroid_y,
		     fl32 *area_orientation,
		     fl32 *area_major_radius,
		     fl32 *area_minor_radius)
     
{

  static int first_call = TRUE;
  static double *means, *eigenvalues;
  static double **eigenvectors;
  static double **area_coords = NULL;
  static si32 n_coords_alloc = 0;
  double area_major_sd;
  double area_minor_sd;

  si32 i, ix, iy;
  si32 n_coords;
  si32 max_coords;
  double area_u, area_v;
  double scale_factor;
  double area_ellipse;

  /*
   * alloc on  first call
   */

  if (first_call) {
    
    means = (double *) umalloc ((ui32) MAX_EIG_DIM * sizeof(double));
    
    eigenvalues =
      (double *) umalloc ((ui32) MAX_EIG_DIM * sizeof(double));
    
    eigenvectors =
      (double **) umalloc ((ui32) MAX_EIG_DIM * sizeof(double *));

    for (i =  0; i < MAX_EIG_DIM; i++)
      eigenvectors[i] =
	(double *) umalloc ((ui32) MAX_EIG_DIM * sizeof(double));

    first_call = FALSE;

  } 

  /*
   * check coords allocation
   */

  max_coords = grid_params->nx * grid_params->ny;
  if (max_coords > n_coords_alloc) {
    alloc_area_coords(max_coords,
		      &area_coords,
		      &n_coords_alloc);
  }

  /*
   * load up coords
   */

  n_coords = 0;
  for (iy = 0; iy < grid_params->ny; iy++) {
    for (ix = 0; ix < grid_params->nx; ix++) {
      if (*grid) {
	area_coords[n_coords][0] =
	  grid_params->min_x + (double) ix * grid_params->dx;
	area_coords[n_coords][1] =
	  grid_params->min_y + (double) iy * grid_params->dy;
	n_coords++;
      }
      grid++;
    } /* ix */
  } /* iy */
    
  if (n_coords == 0) {
   
    *area = 0.0;
    *area_centroid_x = 0.0;
    *area_centroid_y = 0.0;
    *area_orientation = 0.0;
    area_major_sd =  0.0;
    area_minor_sd = 0.0;
    *area_major_radius = 0.0;
    *area_minor_radius = 0.0;
    
  } else {
    
    *area = (double) n_coords * darea_at_centroid;  
    area_ellipse = (double) n_coords * darea_ellipse;
    
    /*
     * obtain the principal component transformation for the coord data
     * The technique is applicable here because the first principal
     * component will lie along the axis of maximum variance, which
     * is equivalent to fitting a line through the data points,
     * minimizing the sum of the sguared perpendicular distances
     * from the data to the line.
     */
    
    if (upct((ui32) 2, (ui32) n_coords,
	     area_coords,
	     means, eigenvectors, eigenvalues) != 0) {
      
      fprintf(stderr, "WARNING - %s:ellipse_compute\n", Glob->prog_name);
      fprintf(stderr, "Computing pct for precip area shape.\n");
      
      *area_orientation = MISSING_VAL;
      area_major_sd = MISSING_VAL;
      area_minor_sd = MISSING_VAL;
      *area_major_radius = MISSING_VAL;
      *area_minor_radius = MISSING_VAL;
      
    } else {
      
      area_u = eigenvectors[0][0];
      area_v = eigenvectors[1][0];
      
      if (area_u == 0 && area_v == 0) {
	
	*area_orientation = 0.0;
	
      } else {
	
	*area_orientation = atan2(area_u, area_v) * RAD_TO_DEG;
	
	if (*area_orientation < 0)
	  *area_orientation += 180.0;
	
      }
      
      *area_centroid_x = means[0];
      *area_centroid_y = means[1];
      
      if (eigenvalues[0] > 0)
	area_major_sd = sqrt(eigenvalues[0]);
      else
	area_major_sd = 0.1;
      
      if (eigenvalues[1] > 0)
	area_minor_sd = sqrt(eigenvalues[1]);
      else
	area_minor_sd = 0.1;
      
      scale_factor =
	sqrt(area_ellipse /
	     (M_PI * area_minor_sd * area_major_sd));
      
      *area_major_radius = area_major_sd * scale_factor;
      *area_minor_radius = area_minor_sd * scale_factor;
      
      /*
       * check for ridiculous results, which occur when all points
       * line up in a straight line
       */
      
      if (*area_major_radius / *area_minor_radius > 1000.0) {
	
	*area_major_radius  = sqrt(area_ellipse / M_PI);
	*area_minor_radius  = sqrt(area_ellipse / M_PI);
	
      } /* if (*area_major_radius ... */
      
    } /* if (upct..... */
    
  } /* if (n_coords == 0) */

  return;
  
}

