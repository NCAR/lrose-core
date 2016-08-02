/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1995  Dennis F. Flanigan Jr. of Applied Research Corporation,
                        Landover, Maryland, a NASA/GSFC on-site contractor.

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
/* File: cappi.c 
 * 
 * 1/9/95 DFF
 * Version 0.29 updates 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <trmm_rsl/rsl.h> 

extern int radar_verbose_flag;


/*********************************************************************/
/*                                                                   */
/*                RSL_get_value_from_cappi                           */
/*                                                                   */
/*********************************************************************/
float RSL_get_value_from_cappi(Cappi *cappi, float rng, float azm)
   {
   return RSL_get_value_from_sweep(cappi->sweep, azm, rng);
   }

/*********************************************************************/
/*                                                                   */
/*                        RSL_new_cappi                              */
/*                        RSL_free_cappi                             */
/*                                                                   */
/* Dennis Flanigan                                                   */
/* Mods by John Merritt 6/14/95                                      */
/*********************************************************************/

Cappi *RSL_new_cappi(Sweep *sweep, float height)
   {
   /* Modeled after a sweep structure. */
   int   a;
   Cappi *c;
   float grange;
   Ray *ray;
   int num_bin;
   float start_bin, size_bin;

   if((c = (Cappi  *)calloc(1, sizeof(Cappi))) == NULL)
      {
      fprintf(stderr,"RSL_new_cappi: Calloc failed for Cappi data structure.\n");
      return(NULL);
      }

   c->height    = height;
   ray = RSL_get_first_ray_of_sweep(sweep);
   num_bin   = ray->h.nbins;
   start_bin = ray->h.range_bin1/1000.0;
   size_bin  = ray->h.gate_size/1000.0;

   /* Allocate space for elev angle,range array */
   if((c->loc =(Er_loc *)calloc(num_bin,sizeof(Er_loc))) == NULL)
      {
      fprintf(stderr,"RSL_new_cappi: Calloc failed for er_loc array. \n");
      free(c);
      return(NULL);
      }

   /* Calculate elevation angle verse range array */
   for(a=0;a<num_bin;a++)
      {
      grange = start_bin + (a * size_bin);
      RSL_get_slantr_and_elev(grange,height,
                              &c->loc[a].srange,&c->loc[a].elev);
      }

   /* Allocate Space for the data */
   c->sweep = RSL_copy_sweep(sweep);
   RSL_clear_sweep(c->sweep); /* This maintains header info. */

   return c;
   }  

void RSL_free_cappi(Cappi *c)
   {
   if (c == NULL) return;
   RSL_free_sweep(c->sweep);
   free(c->loc);
   free(c);

   return;
   }  


/*********************************************************************/
/*                                                                   */
/*                        RSL_cappi_at_h                             */
/*                                                                   */
/* DFF 10/28/94                                                      */
/* Modified by John Merritt 6/14/95                                  */
/*********************************************************************/
Cappi *RSL_cappi_at_h(Volume  *v, float height, float max_range)
   /*
 * h and max_range in KM.
 */
   {
   int n;
   Cappi *c;
   Sweep *sweep;
   
   if (v == NULL) return NULL;
   sweep     = RSL_get_first_sweep_of_volume(v); /* 1st non-NULL sweep */
   
   if((c = RSL_new_cappi(sweep, height)) == NULL)
      {
      fprintf(stderr,"RSL_cappi_at_h: Vnew_cappi failed\n");
      return (NULL);
      }
   
   if((n = RSL_fill_cappi(v,c,0)) < 0)
      {
      fprintf(stderr,"RSL_cappi_at_h: Vfill_cappi_at_h failed: returned: %d\n",n);
      RSL_free_cappi(c);
      return(NULL);
      }
   
   return(c);
   }

 
/*********************************************************************/
/*                                                                   */
/*                        RSL_fill_cappi                             */
/* DFF 10/28/94                                                      */
/*********************************************************************/
int RSL_fill_cappi(Volume *v, Cappi *cap, int method)
   {
   int a,b;
   float x;
   Ray *ray;
   Sweep *sweep;
   
   if (v == NULL) return(-1);
   if (cap == NULL) return(-1);

   /* get data from frist ray. */
   ray = RSL_get_first_ray_of_volume(v);
   cap->month = ray->h.month;
   cap->day        = ray->h.day;
   cap->year       = ray->h.year;
   cap->hour       = ray->h.hour;
   cap->minute     = ray->h.minute;
   cap->sec        = ray->h.sec;
   cap->field_type = 1; /** default setting  -- PAK **/
   cap->interp_method = method;    /* ?? nearest neighbor */
   
   ray = RSL_get_first_ray_of_sweep(cap->sweep);
   sweep = cap->sweep;
   for(a=0;a < sweep->h.nrays; a++)
      {
      ray = sweep->ray[a];
      for(b=0; b < ray->h.nbins; b++)
         {
         x = RSL_get_value(v,
                           cap->loc[b].elev,
                           cap->sweep->ray[a]->h.azimuth,
                           cap->loc[b].srange);
		   
         ray->range[b] = ray->h.invf(x);
         }
      }

   return 1;
   }



