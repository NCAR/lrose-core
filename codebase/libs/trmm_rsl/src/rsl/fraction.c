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
/**********************************************************************/
/*  By: John Merritt                                                 */
/*      Space Applications Corporation                               */
/*      June 12, 1994                                                */
/**********************************************************************/


#include <stdio.h>
#include <trmm_rsl/rsl.h>

/**********************************************************************/
/*                                                                    */
/*                   RSL_fraction_of_ray                              */
/*                   RSL_fraction_of_sweep                            */
/*                   RSL_fraction_of_volume                           */
/*                                                                    */
/*   Compute fraction of dBz within a specified range.                */
/*   Loops are unrolled in each higher function because we are        */
/*   computing fractiion.  I thought about a returning a Ratio        */
/*   structure, but dismissed it.                                     */
/*                                                                    */
/**********************************************************************/
typedef struct {
  int n;
  int ntotal;
} Frac_ratio;


Frac_ratio RSL_ratio_of_ray(Ray *r, float lo, float hi, float range)
{
  int i;
  int ibin_range;  /* Maximum bin include, based on range. */
  Frac_ratio fr;

  fr.n = fr.ntotal = 0;

  if (r == NULL) return fr;
  fr.n = 0;
  ibin_range = range /( (float)r->h.gate_size / 1000.0 );
  if (ibin_range > r->h.nbins) ibin_range = r->h.nbins;
  for (i=0; i<ibin_range; i++) {
	if (lo <= r->h.f(r->range[i]) && r->h.f(r->range[i]) <= hi) fr.n++;
	fr.ntotal++;
  }
  return fr;
}

float RSL_fraction_of_ray(Ray *r, float lo, float hi, float range)
{
  Frac_ratio fr;

  fr = RSL_ratio_of_ray(r, lo, hi, range);
  return (float)fr.n / (float)fr.ntotal;
}


Frac_ratio RSL_ratio_of_sweep(Sweep *s, float lo, float hi, float range)
{
  Frac_ratio total_ratio;
  Frac_ratio ray_ratio;
  int i;

  total_ratio.n = total_ratio.ntotal = 0;
  if (s == NULL) return total_ratio;
  for (i = 0; i<s->h.nrays; i++) {
	ray_ratio = RSL_ratio_of_ray(s->ray[i], lo, hi, range);
	total_ratio.n      += ray_ratio.n;
	total_ratio.ntotal += ray_ratio.ntotal;
  }
  return total_ratio;
}


float RSL_fraction_of_sweep(Sweep *s, float lo, float hi, float range)
{
  Frac_ratio fr;

  fr = RSL_ratio_of_sweep(s, lo, hi, range);
  return (float)fr.n / (float)fr.ntotal;
}



Frac_ratio RSL_ratio_of_volume(Volume *v, float lo, float hi, float range)
{
  Frac_ratio total_ratio;
  Frac_ratio sweep_ratio;
  int i;

  total_ratio.n = total_ratio.ntotal = 0;
  if (v == NULL) return total_ratio;
  for (i = 0; i<v->h.nsweeps; i++) {
	sweep_ratio = RSL_ratio_of_sweep(v->sweep[i], lo, hi, range);
	total_ratio.n      += sweep_ratio.n;
	total_ratio.ntotal += sweep_ratio.ntotal;
  }
  return total_ratio;
}


float RSL_fraction_of_volume(Volume *v, float lo, float hi, float range)
{
  Frac_ratio fr;

  fr = RSL_ratio_of_volume(v, lo, hi, range);
  return (float)fr.n / (float)fr.ntotal;
}


