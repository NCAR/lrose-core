/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            David B. Wolff
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
#include <trmm_rsl/rsl.h>
/*
 * Author: David B. Wolff
 * Date:   8/4/94
 */
/**************************************************************************
	Ideally this would be done via a CAPPI, but could also be done via
	the PPI. Here we will send a volume to produce a CAPPI. From the CAPPI(Z)
	we compute the CAPPI(R) given a Z-R relationship. Then convert that to 
	a cartesian array for imaging or later fiddling.
	
    NOTE: 
	In keep with the object oriented approach to the library, the following
    approach is used to deal with the Z-R conversion:
	
	Four functions are produced: RSL_volume_Z_to_R, RSL_sweep_Z_to_R,
    RSL_ray_Z_to_r, z_to_r. RSL_volume_Z_to_R call RSL_sweep_Z_to_R for each
    sweep, RSL_sweep_Z_to_R calls RSL__ray_Z_to_R for each ray, RSL__ray_Z_to_R
    calls z_to_r for each range bin.

***************************************************************************/

Volume  *RSL_volume_z_to_r(Volume *z_volume, float k, float a);
Sweep   *RSL_sweep_z_to_r(Sweep *z_sweep, float k, float a);
Ray     *RSL_ray_z_to_r(Ray *z_ray, float k, float a);
float   RSL_z_to_r(float z, float k, float a);


Volume *RSL_volume_z_to_r(Volume *z_volume, float k, float a)
{
	Volume 	*r_volume;
	int		i;
	if(z_volume == NULL) return NULL;
	r_volume = RSL_new_volume(z_volume->h.nsweeps);
	r_volume->h = z_volume->h;
	for(i=0; i<z_volume->h.nsweeps; i++) {
		r_volume->sweep[i] = RSL_sweep_z_to_r(z_volume->sweep[i], k, a);
	}
	return r_volume;
}

Sweep *RSL_sweep_z_to_r(Sweep *z_sweep, float k, float a)
{
	Sweep 	*r_sweep;
	int		i;
	if(z_sweep == NULL) return NULL;
	r_sweep = RSL_new_sweep(z_sweep->h.nrays);
	r_sweep->h = z_sweep->h;
	for(i=0; i<z_sweep->h.nrays; i++) {
		r_sweep->ray[i] = RSL_ray_z_to_r(z_sweep->ray[i], k, a);
	}
	return r_sweep;
}

Ray *RSL_ray_z_to_r(Ray *z_ray, float k, float a)
{
	Ray 	*r_ray;
	int		i;	
	Range (*invf)(float x);
	float (*f)(Range x);

	if (z_ray == NULL) return NULL;
	r_ray = RSL_new_ray(z_ray->h.nbins);
	r_ray->h = z_ray->h;
	f = r_ray->h.f;
	invf = r_ray->h.invf;
	for(i=0; i<z_ray->h.nbins; i++) {
		r_ray->range[i] = invf(RSL_z_to_r( f(z_ray->range[i]), k, a));
	}
	return r_ray;
	
}

#include <math.h>

float RSL_z_to_r(float dbz, float k, float a) {

	float 	dbr, dbk, r;

	if (dbz >= NOECHO) return dbz; /* Be careful, NOECHO is the smallest,
									  so far, of the reserved numbers. */
	
	dbk = (float)10*log10((double)k);
	dbr = 1./a*(dbz- dbk);
	r   = (float)pow((double)10., (double)(dbr/10.) );
/*	fprintf(stderr,"dbz=%.1f; \tdbr=%.1f \tRate= %.3f\n",dbz,dbr,r);  */
	return r;
}

