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
/*-----------------------------------------------------------------*/
/*                                                                 */
/*            ray_sort_compare                                     */
/*            RSL_sort_rays_in_sweep                               */
/*            RSL_sort_rays_in_volume                              */
/*                                                                 */
/*  By: John Merritt                                               */
/*      Space Applications Corporation                             */
/*      August 26, 1994                                            */
/*
 * Update and Revisions:
 *
 *      New Routines:
 *        int     ray_sort_compare_by_time(Ray **r1,Ray **r2);
 *        int     sweep_sort_compare(Sweep **s1, Sweep **s2)
 *        Sweep  *RSL_sort_rays_by_time(Sweep *s);
 *        Volume *RSL_sort_sweeps_in_volume(Volume *v)
 *        Volume *RSL_sort_volume(Volume *v)
 *        Radar  *RSL_sort_radar(Radar *r)
 *
 *      Modifications:
 *         Routines that sort data structures now set the 
 *         number of structures in the parent data structure.
 *    
 *         Routines set the enum sorted variable in the
 *         appropriate data structure.
 *
 *      Dennis Flanigan,Jr.
 *      3/23/95
 */
/*-----------------------------------------------------------------*/

#include <stdlib.h>
#include <trmm_rsl/rsl.h> 

static int ray_sort_compare(Ray **r1, Ray **r2)
   {
   /* Compare azim values.  Return -1, 0, 1 for <, =, > comparison. */
   if (*r1 == NULL) return 1;
   if (*r2 == NULL) return -1;

   if ((*r1)->h.azimuth < (*r2)->h.azimuth) return -1;
   if ((*r1)->h.azimuth > (*r2)->h.azimuth) return 1;
   return 0;
   }


static int ray_sort_compare_by_time(Ray **r1, Ray **r2)
   {
   /* Compare time values.  Return -1, 0, 1 for <, =, > comparison. */

   if (*r1 == NULL) return 1;
   if (*r2 == NULL) return -1;

   /* Compare year value */
   if ((*r1)->h.year < (*r2)->h.year) return -1;
   if ((*r1)->h.year > (*r2)->h.year) return 1;

   /* Compare month value */
   if ((*r1)->h.month < (*r2)->h.month) return -1;
   if ((*r1)->h.month > (*r2)->h.month) return 1;

   /* Compare day value */
   if ((*r1)->h.day < (*r2)->h.day) return -1;
   if ((*r1)->h.day > (*r2)->h.day) return 1;

   /* Compare hour value */
   if ((*r1)->h.hour < (*r2)->h.hour) return -1;
   if ((*r1)->h.hour > (*r2)->h.hour) return 1;

   /* Compare minute value */
   if ((*r1)->h.minute < (*r2)->h.minute) return -1;
   if ((*r1)->h.minute > (*r2)->h.minute) return 1;
   
   /* Compare second value */
   if ((*r1)->h.sec < (*r2)->h.sec) return -1;
   if ((*r1)->h.sec > (*r2)->h.sec) return 1;

   return 0;
   }

static int sweep_sort_compare(Sweep **s1, Sweep **s2)
   {
   /* Compare elevation values.  Return -1, 0, 1 for <, =, > comparison. */
   if (*s1 == NULL) return 1;
   if (*s2 == NULL) return -1;

   if ((*s1)->h.elev < (*s2)->h.elev) return -1;
   if ((*s1)->h.elev > (*s2)->h.elev) return 1;
   return 0;
   }

Sweep *RSL_sort_rays_in_sweep(Sweep *s)
   {
   /* Sort rays by azimuth in passed sweep */
   int a;
   
   if (s == NULL) return NULL;

   qsort((void *)s->ray, s->h.nrays, sizeof(Ray *),
		 (int (*)(const void *, const void *))ray_sort_compare);

   /* Set nrays values to number of non-NULL indexes.
    * After sorting this is highest useable index.
	*/
   for(a=s->h.nrays-1; a>=0 ;a--) {
	 if(s->ray[a] != NULL) {
	   s->h.nrays = a+1;
	   break;
	 }
   }

   return s;
   }

Sweep *RSL_sort_rays_by_time(Sweep *s)
   {
   /* Set rays in passed sweep by time */
   int a;
   
   if (s == NULL) return NULL;

   qsort((void *)s->ray, s->h.nrays, sizeof(Ray *), 
		 (int (*)(const void *, const void *))ray_sort_compare_by_time);

   /* Set nrays values to number of non-NULL indexes.
    * After sorting this is highest useable index.
	*/
   for(a=s->h.nrays-1; a>=0 ;a--) {
	 if(s->ray[a] != NULL) {
	   s->h.nrays = a+1;
	   break;
	 }
   }

   return s;
   }


Volume *RSL_sort_rays_in_volume(Volume *v)
   {
   /* Sort rays in the sweeps pointed to by the volume .
	* (Does not sort the sweeps pointers)
	*/
   int i;
   if (v == NULL) return NULL;
 
   for (i=0; i<v->h.nsweeps; i++)
   v->sweep[i] = RSL_sort_rays_in_sweep(v->sweep[i]);

   return v;
   }

Volume *RSL_sort_sweeps_in_volume(Volume *v)
   {
   /* Sort sweeps pointers in passed volume data structure. 
	* (Does not sort rays in sweeps.)
	*/
   int a;
   
   if (v == NULL) return NULL;
   
   qsort((void *)v->sweep,v->h.nsweeps,sizeof(Sweep *),
		 (int (*)(const void *, const void *))sweep_sort_compare);

   /* Set nsweeps value to number of non-NULL indexes.
    * After sorting, this is the highest usable index.
	*/
   for(a=0;a<v->h.nsweeps;a++)
	  {
	  if(v->sweep[a] == NULL)
		 {
		 v->h.nsweeps = a;
		 break;
		 }
	  }
   
   return v;
   }

Volume *RSL_sort_volume(Volume *v)
   {
   /* Sort sweeps and rays by angle in the Volume data structure
	* passed.
	*/
   
   if(v == NULL) return NULL;

   v = RSL_sort_sweeps_in_volume(v);
   v = RSL_sort_rays_in_volume(v);

   return v;
   }


Radar *RSL_sort_radar(Radar *r)
   {
   /* Sort sweeps and rays in all Volumes of the passed Radar data 
	* structure.
	*/
   int a;
   
   if(r == NULL) return NULL;
   
   for(a=0; a<r->h.nvolumes;a++)
	  {
	  r->v[a] = RSL_sort_volume(r->v[a]);
	  }

   return r;
   }


