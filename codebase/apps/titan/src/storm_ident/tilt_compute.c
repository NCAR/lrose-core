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
 * tilt_compute.c
 *
 * Compute tilt from  principal component analysis
 *
 * Note: area is in sq km.
 *       tilt_area is in sq grid_units.
 *
 * Mike Dixon RAP NCAR Boulder Colorado USA
 *
 * April 1995
 *
 *************************************************************************/

#include "storm_ident.h"

void tilt_compute(si32 nz,
		  si32 n_layers,
		  si32 base_layer,
		  si32 top_layer,
		  double min_valid_z,
		  layer_stats_t *layer,
		  fl32 *tilt_dirn,
		  fl32 *tilt_angle)
     
{

  static int first_call = TRUE;
  static int nz_alloc;
  static double *means, *eigenvalues;
  static double **eigenvectors;
  static double **tilt_data;

  si32 i, iz;
  double tilt_x, tilt_y, tilt_z, tilt_r;

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
    
    means = (double *) umalloc ((ui32) MAX_EIG_DIM * sizeof(double));
    
    eigenvalues =
      (double *) umalloc ((ui32) MAX_EIG_DIM * sizeof(double));
    
    eigenvectors =
      (double **) umalloc ((ui32) MAX_EIG_DIM * sizeof(double *));

    for (i =  0; i < MAX_EIG_DIM; i++)
      eigenvectors[i] =
	(double *) umalloc ((ui32) MAX_EIG_DIM * sizeof(double));

    nz_alloc = nz;
    
    tilt_data = (double **) umalloc ((ui32) nz_alloc * sizeof(double *));
    
    for (i =  0; i < nz_alloc; i++)
      tilt_data[i] =
	(double *) umalloc ((ui32) MAX_EIG_DIM * sizeof(double));

    first_call = FALSE;

  } else {

    if (nz > nz_alloc) {

      for (i =  0; i < nz_alloc; i++) {
	ufree((char *) tilt_data[i]);
      }

      nz_alloc = nz;
      
      tilt_data =
	(double **) urealloc ((char *) tilt_data,
			      (ui32) nz_alloc * sizeof(double *));
    
      for (i =  0; i < nz_alloc; i++)
	tilt_data[i] =
	  (double *) umalloc ((ui32) MAX_EIG_DIM * sizeof(double));

    } /* if (nz > nz_alloc) */
    
  } /* if (first_call) */
    
  /*
   * tilt angle computations
   */
      
  /*
   * load up tilt_data array with the centroid positions for
   * each layer. The z values are exaggerated by a factor of 10.0 to
   * force the vertical to be the direction of dominant variance
   */
      
  for (iz = base_layer; iz <= top_layer; iz++) {
    
    tilt_data[iz][0] = layer[iz].vol_centroid_x;
    tilt_data[iz][1] = layer[iz].vol_centroid_y;
    tilt_data[iz][2] = min_valid_z + (double) iz * Glob->delta_z * 10.0;
    
  }
      
  /*
   * obtain the principal component transformation for this data - the
   * technique is applicable here because the first principal component
   * will lie along the axis of maximum variance, which is equivalent
   * to fitting a line through the data points, minimizing the sum of
   * the sguared perpendicular distances from the data to the line.
   */
  
  if (upct((ui32) 3, (ui32) n_layers,
	   tilt_data + base_layer,
	   means, eigenvectors, eigenvalues) != 0) {
    
    fprintf(stderr, "WARNING - %s:tilt_compute\n", Glob->prog_name);
    fprintf(stderr, "Computing pct for tilt angle and direction.\n");
    *tilt_dirn = MISSING_VAL;
    *tilt_angle = MISSING_VAL;
    
  } else {
    
    /*
     * compute the tilt angle and direction from the first eigenvector
     */
    
    tilt_x = eigenvectors[0][0];
    tilt_y = eigenvectors[1][0];
    tilt_z = eigenvectors[2][0] / 10.0;
    tilt_r = sqrt(tilt_x * tilt_x + tilt_y * tilt_y);
    
    if (tilt_x == 0 && tilt_y == 0) {
      
      *tilt_dirn = 0.0;
      *tilt_angle = 0.0;
      
    } else {
      
      *tilt_dirn = atan2(tilt_x, tilt_y) * RAD_TO_DEG;
      
      if (*tilt_dirn < 0)
	*tilt_dirn += 360.0;
      
      *tilt_angle = 90.0 - atan2(tilt_z, tilt_r) * RAD_TO_DEG;
      
    }
    
  } /* if (upct ........ */

  return;
  
}

