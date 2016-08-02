/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            John H. Merritt
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdlib.h>
#include <math.h>

#  define M_PI		3.14159265358979323846

#include <trmm_rsl/rsl.h>
/**********************************************************************/
/*                                                                    */
/*                 RSL_area_of_ray                                    */
/*                 RSL_fractional_area_of_sweep                       */
/*                                                                    */
/*    Compute fractional area of a sweep given a range of dBZ and     */
/*    a maximum range in km.                                          */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/

float get_pixel_area(Ray *r, float min_range, float max_range)
{
  float h1, h2, r1, r2;  /* height and radius in km*/
  float volume;
  float theta;  /* in radian */

  /* returns volume area of ray between min range and max range
   */
  if (r == NULL) return 0.0;

  h1 = min_range;
  h2 = max_range;
  /* convert from degree to radian */
  theta = (r->h.beam_width/2) * 2 * M_PI / 360;

  r1 = h1 * tan(theta);
  r2 = h2 * tan(theta);
  volume = (M_PI/3) * (pow((double)r2, (double) 2) * h2 - pow((double) r1, (double) 2) *h1);

  return volume;
} /* get_pixel_area */


float RSL_area_of_ray(Ray *r, float lo, float hi, float min_range, float max_range)
{
  int i;
  float start_km;
  float xdBZ;
  float r1, r2, area;
  int nbins, bin1;
  float binsize;
  
  if (r == NULL) return 0.0;
  /* Check if min_range is closer to the radar than the first bin.
   * If so, we can't use it.
   */

  if (hi <= 0.0)
	hi = 100.0;  /* default to max dbz*/
  start_km = r->h.range_bin1/1000.0;
  binsize = r->h.gate_size/1000.0;

  nbins = ((max_range - start_km) / binsize);
  bin1 = (min_range - start_km) / binsize;

  if (bin1 < 0) bin1 = 0;
  if (nbins > r->h.nbins) nbins = r->h.nbins;

  /* Compute the number of pixels with lo < dBZ <= hi */
  area = 0.0;

  for(i=bin1; i<nbins-1; i++) {
	xdBZ = r->h.f(r->range[i]);
	if(lo < xdBZ && xdBZ <= hi) { /*MAX_DBZ = hi (typically 70?) */
	  /* get r1 and r2 in km */
	  r1 = i * binsize + start_km;
	  r2 = (i+1) * binsize + start_km;
	  area += get_pixel_area(r, r1, r2);
 	}
	r1 = i * binsize + start_km;
  }

  return area;
}  



float RSL_fractional_area_of_sweep(Sweep *s, float lo, float hi, float min_rng, float max_rng)
{
/*
 *  Compute the fractional area of the Sweep.
 *
 *  This doesn't take care of a sector (window).  The caller will
 *  have to multiply the answer by some appropriate constant.
 *
 */


  float sweep_area;
  float area;
  float frac_area;
  float total_sys_area=0.0;
  int iazm;

  if (s == NULL) return 0.0;

  sweep_area = 0;
  for (iazm=0; iazm<s->h.nrays; iazm++) {
	/* get total system volume area */
	total_sys_area += RSL_area_of_ray(s->ray[iazm], -71, 0.0, min_rng, max_rng);
	area = RSL_area_of_ray(s->ray[iazm], lo, hi, min_rng, max_rng);
	sweep_area += area;
/*
	fprintf(stderr,"iazm = %d, totalsysarea = %f, area = %f, sweep_area = %f\n",
		   iazm, total_sys_area, area, sweep_area);
*/
  }
  frac_area = sweep_area / total_sys_area;
  return (float) frac_area;
}

