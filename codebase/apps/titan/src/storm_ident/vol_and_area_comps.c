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
 * vol_and_area_comps.c
 *
 * Compute clump volume
 *
 * Mike Dixon RAP NCAR Boulder Colorado USA
 *
 * April 1995
 *
 *************************************************************************/

#include "storm_ident.h"

static double Dx, Dy, Dx_at_equator;
static double Dvol_flat, Dvol_at_equator;
static double Darea_flat, Darea_at_equator;
static double Darea_ellipse;
static int Latlon;

void init_vol_and_area_comps(void)

{

  if (Glob->projection == MDV_PROJ_FLAT) {
    
    /*
     * flat grid
     */

    Latlon = FALSE;

    Dx = Glob->delta_x;
    Dy = Glob->delta_y;
    Darea_flat = Dx * Dy;
    Dvol_flat = Dx * Dy * Glob->delta_z;
    Darea_ellipse = Darea_flat;
    
  } else {
    
    /*
     * latlon grid
     *
     * latlon data has a lat/lon grid, so we need to multiply by
     * a (111.12 squared) to get km2 for area. The delta_z is
     * set nominally to 1.0, so area and volume will be the same.
     * The volume and area computations are adjusted later for the
     * latitude of the storm.
     */
    
    Latlon = TRUE;

    Dy = Glob->delta_y * KM_PER_DEG_AT_EQ;
    Dx_at_equator = Glob->delta_y * KM_PER_DEG_AT_EQ;

    Dvol_at_equator =
      (Glob->delta_x * Glob->delta_y * Glob->delta_z * 
       KM_PER_DEG_AT_EQ * KM_PER_DEG_AT_EQ);
    
    Darea_at_equator =
      Glob->delta_x * Glob->delta_y * KM_PER_DEG_AT_EQ * KM_PER_DEG_AT_EQ;
    
    Darea_ellipse = Glob->delta_x * Glob->delta_y;
    
  } /* if (!latlon) */

  return;
  
}
  
void vol_and_area_comps(const Clump_order *clump,
			double *size_p,
			double *dvol_at_centroid_p,
			double *darea_at_centroid_p,
			double *darea_ellipse_p)
     
{

  

  int intv;
  Interval *intvl;
  double sumy = 0.0, n = 0.0;
  double vol_centroid_y;
  double latitude_factor;

  if (!Latlon) {

    *dvol_at_centroid_p = Dvol_flat;
    *darea_at_centroid_p = Darea_flat;
    *darea_ellipse_p = Darea_ellipse;

    if (Glob->nz <= 1)
      *size_p = clump->pts * Darea_flat;
    else
      *size_p = clump->pts * Dvol_flat;

  } else {

    /*
     * compute the volumetric y centroid
     */

    for (intv = 0; intv < clump->size; intv++) {

      intvl = clump->ptr[intv];
      sumy += (double) intvl->row_in_plane * (double) intvl->len;
      n += (double) intvl->len;

    }

    vol_centroid_y = (sumy / n) * Glob->delta_y + Glob->min_y;
    latitude_factor = cos(vol_centroid_y * DEG_TO_RAD);

    *dvol_at_centroid_p = Dvol_at_equator * latitude_factor;
    *darea_at_centroid_p = Darea_at_equator * latitude_factor;
    *darea_ellipse_p = Darea_ellipse;

    if (Glob->nz <= 1)
      *size_p = clump->pts * *darea_at_centroid_p;
    else
      *size_p = clump->pts * *dvol_at_centroid_p;

  }

  return;

}

double km_per_grid_unit(si32 min_iy, si32 max_iy)

{

  double mid_iy, mid_lat;
  double latitude_factor;

  if (Glob->projection == MDV_PROJ_FLAT) {
    return ((Dx + Dy) / 2.0);
  } else {
    mid_iy = ((double) max_iy + (double) min_iy) / 2.0;
    mid_lat = Glob->min_y + Glob->delta_y * mid_iy;
    latitude_factor = cos(mid_lat * DEG_TO_RAD);
    return ((Dx_at_equator * latitude_factor + Dy) / 2.0);
  }

}
