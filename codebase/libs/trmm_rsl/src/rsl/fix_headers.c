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
/*
 * Fix header fields in ray headers.
 *
 * This routine is initially written to support 1C-51.
 * It has been noticed that several radar files contain bad header
 * information.  Herein, we correct it by simple linear interpolation.
 *
 * By: John Merritt
 *     Space Applications Corporation
 *     Copyright 7/16/96
 *
 */

#include <stdio.h>
#include <trmm_rsl/rsl.h> 

Ray *RSL_fix_ray_header(Ray *ray)
{
  return ray;
}

Sweep *RSL_fix_sweep_header(Sweep *sweep)
{
  int i;
  int nfixed = 0;
  int needed_to_fix = 0;
  Ray *ray;

  if (sweep == NULL) return sweep;

  for (i=0; i<sweep->h.nrays; i++) {
	/* Here, we check and use more than one ray. */
	ray = sweep->ray[i];

	if (ray == NULL) continue;
	if (ray->h.month < 1 || ray->h.month > 12) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.month = %d\n", i, ray->h.month);
	}
	if (ray->h.day < 1 || ray->h.day > 31) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.day   = %d\n", i, ray->h.day);
	}
	if (ray->h.year < 1980 || ray->h.year > 2020) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.year  = %d\n", i, ray->h.year);
	}
	if (ray->h.hour < 0 || ray->h.hour > 23) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.hour  = %d\n", i, ray->h.hour);
	}
	if (ray->h.minute < 0 || ray->h.minute > 59) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.minute= %d\n", i, ray->h.minute);
	}
	if (ray->h.sec < 0 || ray->h.sec > 59) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.sec   = %f\n", i, ray->h.sec);
	}
	if (ray->h.elev < 0 || ray->h.elev > 90) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.elev  = %f\n", i, ray->h.elev);
	}
	if (ray->h.range_bin1 < 0 || ray->h.range_bin1 > 150000) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.range_bin1  = %d\n", i, ray->h.range_bin1);
	}
	if (ray->h.gate_size < 0 || ray->h.gate_size > 100000) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.gate_size   = %d\n", i, ray->h.gate_size);
	}
	if (ray->h.beam_width <= 0 || ray->h.beam_width > 10) {
	  needed_to_fix = 1;
	  fprintf(stderr, "ray[%3.3d]->h.beam_width  = %f\n", i, ray->h.beam_width);
	}
	if (needed_to_fix) {
	  needed_to_fix = 0;
	  nfixed++;
	}
  }

  fprintf(stderr, "Repaired %d rays in this sweep.\n", nfixed);

  return sweep;
}

Volume *RSL_fix_volume_header(Volume *v)
{
  int i;
  if (v == NULL) return v;

  for (i=0; i<v->h.nsweeps; i++)
	RSL_fix_sweep_header(v->sweep[i]);

  return v;
}

Radar *RSL_fix_radar_header(Radar *radar)
{
  int i;
  if (radar == NULL) return radar;

  for (i=0; i<radar->h.nvolumes; i++)
	RSL_fix_volume_header(radar->v[i]);

  return radar;
}


