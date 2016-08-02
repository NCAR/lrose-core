/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996  Thuy Nguyen of International Database Systems
                        a NASA/GSFC on-site contractor.

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

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <trmm_rsl/rsl.h> 

extern int radar_verbose_flag;


/***************************************************************************
 *                         RSL_get_window_from_radar
 *                         RSL_get_window_from_volume
 *                         RSL_get_window_from_sweep
 *                         RSL_get_window_from_ray
 *                         
 * These routines get window (area) defined by minimum range, maximum range, 
 * low azimuth, and hi azimuth.
 *
 * By:  Thuy Nguyen
 *      International Database Systems
 * 
 y***************************************************************************/

Radar *RSL_get_window_from_radar(Radar *r, float min_range, float max_range,
								 float low_azim, float hi_azim)
{
  int i;
  Radar *new_radar;
  
  if (min_range > max_range || min_range < 0 || max_range < 0){
	if (radar_verbose_flag)
	fprintf(stderr,"Get win from radar: given invalid min range (%f) or max range (%f)\n",
		   min_range, max_range);
	return NULL;
  }
  if (!r) return NULL;

  if ((new_radar = RSL_new_radar(r->h.nvolumes)) == NULL) return NULL;
  new_radar->h = r->h;

  for (i = 0; i < r->h.nvolumes; i++) {
	if (radar_verbose_flag)
	  fprintf(stderr,"Getting window from volume for v[%d] out of %d volumes\n",
			 i,r->h.nvolumes );

	new_radar->v[i] = RSL_get_window_from_volume(r->v[i], min_range, max_range,
										  low_azim, hi_azim);
  }
  return new_radar;
}


Volume *RSL_get_window_from_volume(Volume *v, float min_range, float max_range,
								 float low_azim, float hi_azim)
{
  int i;
  Volume *new_volume;
  Sweep  *new_sweep;

  if (min_range > max_range || min_range < 0 || max_range < 0){
	if (radar_verbose_flag)
	fprintf(stderr,"Get win from volume: given invalid min range (%f) or max range (%f)\n",
		   min_range, max_range);
	return NULL;
  }
  if (!v) return NULL;

  if ((new_volume = RSL_new_volume(v->h.nsweeps)) == NULL) return NULL;
  new_volume->h = v->h;

  for (i = 0; i < v->h.nsweeps; i++) {
	if (radar_verbose_flag)
	  fprintf(stderr,"Getting window from sweep for s[%d] out of %d sweeps\n", 
			 i,v->h.nsweeps); 

	new_sweep = RSL_get_window_from_sweep(v->sweep[i], min_range, max_range,
										  low_azim, hi_azim);
	new_volume->sweep[i] = new_sweep;
  }

  if (radar_verbose_flag)
	fprintf(stderr,"Got win from volume: orig volume has %d sweeps, new "
		   "volume has %d sweeps\n",v->h.nsweeps,new_volume->h.nsweeps);
  
  return new_volume;
}

Sweep *RSL_get_window_from_sweep(Sweep *s, float min_range, float max_range,
								 float low_azim, float hi_azim)
{
  int   i;
  Sweep *new_sweep;
  Ray   *new_ray;

  if (min_range > max_range || min_range < 0 || max_range < 0){
	if (radar_verbose_flag)
	fprintf(stderr,"Get win from sweep: given invalid min range (%f) or max range (%f)\n",
		   min_range, max_range);
	return NULL;
  }
  if (s == NULL) return NULL;

  if ((new_sweep = RSL_new_sweep(s->h.nrays)) == NULL)
        return NULL;

  new_sweep->h = s->h;

  for (i = 0; i < s->h.nrays; i++) {
        new_ray = RSL_get_window_from_ray(s->ray[i], min_range, max_range,
										  low_azim, hi_azim);
		new_sweep->ray[i] = new_ray;

  }


  if (radar_verbose_flag)
	fprintf(stderr,"Got win from sweep: orig sweep has %d rays, new sweep "
		   "has %d rays.\n",s->h.nrays,new_sweep->h.nrays);

  return new_sweep;
}


Ray *RSL_get_window_from_ray(Ray *r, float min_range, float max_range,
								 float low_azim, float hi_azim)
{
  float start_km, binsize;
  int start_index, end_index;
  Ray *new_ray;
  int i;

  if (min_range > max_range || min_range < 0 || max_range < 0){
	if (radar_verbose_flag)
	fprintf(stderr,"Get win from ray: given invalid min range (%f) or max range (%f)\n",
		   min_range, max_range);
	return NULL;
  }

  if (r == NULL || r->h.azimuth < low_azim || r->h.azimuth >= hi_azim) 
	return NULL;

  /* convert from meter to km */
  start_km = r->h.range_bin1/1000.0;
  binsize = r->h.gate_size/1000.0;

  end_index = (int) ( (max_range - start_km) / binsize) + 1;
  if (end_index > r->h.nbins)
	end_index = r->h.nbins;

  if (min_range == 0.0)
        start_index = 0;
  else
        start_index = (int) ( (min_range - start_km) / binsize);


  if ((new_ray = RSL_copy_ray(r)) == NULL) return NULL;
  if ((new_ray = RSL_clear_ray(new_ray)) == NULL) return NULL;


  for (i = start_index; i < end_index; i++) {
	new_ray->range[i] = r->range[i];
  }
  return new_ray;
}



















