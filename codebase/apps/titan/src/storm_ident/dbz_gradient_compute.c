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
 * dbz_gradient_compute.c
 *
 * Compute dbz_gradient from  principal component analysis
 *
 * Note: area is in sq km.
 *       dbz_gradient_area is in sq grid_units.
 *
 * Mike Dixon RAP NCAR Boulder Colorado USA
 *
 * April 1995
 *
 *************************************************************************/

#include "storm_ident.h"

void dbz_gradient_compute(si32 nz,
			  si32 n_layers,
			  si32 base_layer,
			  si32 top_layer,
			  double min_valid_z,
			  layer_stats_t *layer,
			  fl32 *dbz_max_gradient,
			  fl32 *dbz_mean_gradient)
     
{
  
  static int first_call = TRUE;
  static int nz_alloc;
  static double *means, *eigenvalues;
  static double **eigenvectors;
  static double **dbz_gradient_data;
  
  si32 i, iz;

  /*
   * check for more than 1 layer
   */

  if (n_layers < 2) {
    return;
  }

  /*
   * alloc on  first call
   */

  if (first_call) {
    
    means = (double *) umalloc ((ui32) 2 * sizeof(double));
    
    eigenvalues =
      (double *) umalloc ((ui32) 2 * sizeof(double));
    
    eigenvectors =
      (double **) umalloc ((ui32) 2 * sizeof(double *));

    for (i =  0; i < 2; i++)
      eigenvectors[i] =
	(double *) umalloc ((ui32) 2 * sizeof(double));

    nz_alloc = nz;
    
    dbz_gradient_data = (double **) umalloc ((ui32) nz_alloc * sizeof(double *));
    
    for (i =  0; i < nz_alloc; i++)
      dbz_gradient_data[i] =
	(double *) umalloc ((ui32) 2 * sizeof(double));

    first_call = FALSE;

  } else {

    if (nz > nz_alloc) {

      for (i =  0; i < nz_alloc; i++) {
	ufree((char *) dbz_gradient_data[i]);
      }

      nz_alloc = nz;
      
      dbz_gradient_data =
	(double **) urealloc ((char *) dbz_gradient_data,
			      (ui32) nz_alloc * sizeof(double *));
    
      for (i =  0; i < nz_alloc; i++)
	dbz_gradient_data[i] =
	  (double *) umalloc ((ui32) 2 * sizeof(double));

    } /* if (Glob->nz > nz_alloc) */
    
  } /* if (first_call) */
    
  /*
   * tilt angle computations
   */
      
  /*
   * load up dbz_gradient_data array with dbz_max.
   * The z values are exaggerated by a factor of 1000.0 to
   * force the vertical to be the direction of dominant variance
   */
      
  for (iz = base_layer; iz <= top_layer; iz++) {
    
    dbz_gradient_data[iz][0] = layer[iz].dbz_max;
    dbz_gradient_data[iz][1] =
      min_valid_z + (double) iz * Glob->delta_z * 1000.0;

  }
      
  /*
   * obtain the principal component transformation for this data - the
   * technique is applicable here because the first principal component
   * will lie along the axis of maximum variance, which is equivalent
   * to fitting a line through the data points, minimizing the sum of
   * the sguared perpendicular distances from the data to the line.
   */
  
  if (upct((ui32) 2, (ui32) n_layers,
	   dbz_gradient_data + base_layer,
	   means, eigenvectors, eigenvalues) != 0) {
    
    fprintf(stderr, "WARNING - %s:dbz_gradient_compute\n",
	    Glob->prog_name);
    fprintf(stderr, "Computing pct for dbz_max_gradient.\n");
    *dbz_max_gradient = MISSING_VAL;
    
  } else {
    
    *dbz_max_gradient = eigenvectors[0][0] * 1000.0;

  } /* if (upct ........ */

  /*
   * Repeat for dbz_mean.
   */
      
  for (iz = base_layer; iz <= top_layer; iz++) {
    
    dbz_gradient_data[iz][0] = layer[iz].dbz_mean;
    dbz_gradient_data[iz][1] =
      min_valid_z + (double) iz * Glob->delta_z * 1000.0;

  }
      
  if (upct((ui32) 2, (ui32) n_layers,
	   dbz_gradient_data + base_layer,
	   means, eigenvectors, eigenvalues) != 0) {
    
    fprintf(stderr, "WARNING - %s:dbz_gradient_compute\n",
	    Glob->prog_name);
    fprintf(stderr, "Computing pct for dbz_mean_gradient.\n");
    *dbz_mean_gradient = MISSING_VAL;
    
  } else {
    
    *dbz_mean_gradient = eigenvectors[0][0] * 1000.0;

  } /* if (upct ........ */

  return;
  
}

