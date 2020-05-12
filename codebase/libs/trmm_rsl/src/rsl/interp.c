/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996  Dennis F. Flanigan Jr. of Applied Research Corporation,
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
/*
 * Interpolation functions
 *
 * Dennis Flanigan, Jr.
 *
 * 7/7/95
 * Finished testing interpolation routine.
 *
 * 7/6/95
 * Finished up bilinear interpolation routine.
 * Routine calculates values using four surrounding
 * values with the same slant range.
 *
 * 6/29/95
 * Added bilinear interpolation value routine.   Rewrote 
 * get_surrounding sweeps so that it returns Sweeps instead of
 * Sweep indexes.
 *
 * 6/28/95 
 * Added internal ray searching routine. Replaces
 * RSL_get_next_closest_ray.
 *
 * 6/25/95
 * Added internal sweep searching routine designed for use
 * by interpolation routines.  Replaced RSL_get_next_closest_sweep.
 *
 * 6/23/95 
 * This file was created.  Started adding internal routines
 * needed for calculating distences between two points
 * in space.
 */

#include <stdio.h>
#include <math.h>
#include <trmm_rsl/rsl.h>


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

extern double hypot(double, double);

/***************************************************/
/*                                                 */
/*          get_xyz_coord                          */
/*                                                 */
/*                                                 */
/***************************************************/
void get_xyz_coord(double *x,double *y,double *z,
                   double range,double azim,double elev)
   {
   /* Return x,y,z coordinates given range, azimuth angle and 
    * elevation angle.  Memory allocation for x,y and z are
    * not provided by this routine!
	*/
   double azim_rad,elev_rad;

#ifndef M_PI
#  define M_PI		3.14159265358979323846
#endif
   
   azim_rad = azim *  M_PI / 180.0;
   elev_rad = elev *  M_PI / 180.0;
   
   *x = cos(elev_rad) * cos(azim_rad) * range;
   *y = cos(elev_rad) * sin(azim_rad) * range;
   *z = sin(elev_rad) * range;
   }

/***************************************************/
/*                                                 */
/*          get_dist                               */
/*                                                 */
/*                                                 */
/***************************************************/
float get_dist(float r1,float a1,float e1,
               float r2,float a2,float e2)
   {
   /* Give two points described by range, azimuth angle
    * and elevation angle, return the distence between
    * the two.
	*/
   double x1,y1,z1,x2,y2,z2;
   
   get_xyz_coord(&x1,&y1,&z1,(double)r1,(double)a1,(double)e1);
   get_xyz_coord(&x2,&y2,&z2,(double)r2,(double)a2,(double)e2);

   return (float) sqrt(pow(x1 - x2,2) + pow(y1 - y2,2) + pow(z1 - z2,2));
   }

/***************************************************/
/*                                                 */
/*          get_surrounding_sweeps                 */
/*                                                 */
/*                                                 */
/***************************************************/
void get_surrounding_sweep(Sweep **below,Sweep **above, Volume *v,
                           float elev)
   {
   /* Return the pointers of the sweeps that are above and
    * and below the elevation angle in the parameter list.
    *
    * Assume at least one non-NULL sweep exist in Volume.
    *
    * A value of NULL is set to  above or below in cases
    * where there is no sweep above or below.
    */
   int a;

   /* look for above index first */
   a = 0;
   while(a < v->h.nsweeps)
      {
      if(v->sweep[a] != NULL)
         {
         if(v->sweep[a]->h.elev >= elev) 
            {
            *above = v->sweep[a];
            break;
            }
         }
      a++;
      }
   
   /* Was above ever set ?
    * If not, set above to counter.
    */
   if(a == v->h.nsweeps)
      {
      *above = NULL;
      }
   
   /* Look for index just below elev */
   a--;
   while(a >= 0)
      {
      if(v->sweep[a] != NULL)
         {
         if(v->sweep[a]->h.elev <= elev)
            {
            *below = v->sweep[a];
            break;
            }
         }
      a--;
      }
   
   /* Was a below found ? */
   if (a == -1)
      {
      *below = NULL;  
      }
   
   /* Bye */
   }

/******************************************
 *                                        *
 * dir_angle_diff                         *
 *                                        *
 * Dennis Flanigan,Jr. 4/29/95            *
 ******************************************/
double dir_angle_diff(float x,float y)
   {
   /* returns difference between angles x and y.  Returns
    * positive value if y > x, negitive value if
    * y < x.
    * 
    */
   double d;
   d = (double)(y - x);
   
   while (d >= 180) d = -1 * (360 - d);
   while (d < -180) d =  (360 + d);
   
   return d;
   }

/***************************************************/
/*                                                 */
/*          get_surrounding_rays                   */
/*                                                 */
/*                                                 */
/***************************************************/
void get_surrounding_ray(Ray **ccwise,Ray **cwise,Sweep *s,
                         float ray_angle)
   {
   /*  Return the pointers to the rays that are counterclockwise
    *  and clockwise to the ray_angle in the parameter list.
    *  Memory space for the variables ccwise and cwise
    *  is not allocated by this routine.
    *
    *  Assume at least two rays exist and that a hash
    *  table has been created.
    *
    *  Will need to add test for first ray and last
    *  ray if this routine is going to be used for
    *  RHI scans.
	*/
   int hindex;
   double close_diff;
   Hash_table *hash_table;
   
   Azimuth_hash *closest;

   /* Find hash index close to hindex we want.  This will
    * used as a starting point for a search to the closest
    * ray.
    */
   hash_table = hash_table_for_sweep(s);
   if (hash_table == NULL) return; /* Nada. */

   hindex = hash_bin(hash_table,ray_angle); 

   /* Find hash entry with closest Ray */
   closest = the_closest_hash(hash_table->indexes[hindex],ray_angle);

   close_diff = dir_angle_diff(ray_angle,closest->ray->h.azimuth);
   
   if(close_diff < 0)
      {
      /* Closest ray is counterclockwise to ray_angle */
      *ccwise = closest->ray;
      *cwise = closest->ray_high->ray;
      }
   else
      {
      /* Closest ray is clockwise to ray_angle. */
      *cwise = closest->ray;
      *ccwise = closest->ray_low->ray;
      }

   }

/***************************************************/
/*                                                 */
/*          from_dB, to_dB                         */
/*                                                 */
/***************************************************/
double from_dB(double db)
   {
   return pow(10,db/10.0);
   }

double to_dB(double value)
   {
   return 10.0 * log10(value);
   }

/***************************************************/
/*                                                 */
/*        get_linear_value_from_sweep              */
/*                                                 */
/***************************************************/
double get_linear_value_from_sweep(Sweep *sweep,float srange,float azim,
                                   float limit)
   {
   float  ccw_db_value,cw_db_value;
   double ccw_value,cw_value,value;
   double delta_angle;
   Ray    *ccw_ray,*cw_ray;

   get_surrounding_ray(&ccw_ray,&cw_ray,sweep,azim);
   
   /* Assume that ccw_ray and cw_ray will be non_NULL */
   
   if((azim - ccw_ray->h.azimuth) > limit)
      {
      ccw_value = -1;
      }
   else
      {
      ccw_db_value = RSL_get_value_from_ray(ccw_ray,srange);
	  
      /* Check to make sure this is a valid value */
      if (ccw_db_value == BADVAL)
         {
         ccw_value = 0;
         }
      else
         {
         ccw_value = from_dB(ccw_db_value);
         }
      }
   
   if((cw_ray->h.azimuth - azim) > limit)
      {
      cw_value = -1;
      }
   else
      {
      cw_db_value = RSL_get_value_from_ray(cw_ray,srange);
      /* Check to make sure this is a valid value */
      if (cw_db_value == BADVAL)
         {
         cw_value = 0;
         }
      else
         {
         cw_value = from_dB(cw_db_value);
         }
      }
   
   if((cw_value != -1) && (ccw_value != -1))
      {
      /* Both the clockwise ray and the counterclockwise
       * ray is valid.
       */
      delta_angle = angle_diff(ccw_ray->h.azimuth,cw_ray->h.azimuth);
	  
      value=((angle_diff(azim,cw_ray->h.azimuth)/delta_angle)*ccw_value)
         + ((angle_diff(azim,ccw_ray->h.azimuth)/delta_angle) * cw_value);
      }
   else if((cw_value == -1) && (ccw_value == -1))
      {
      /* Neither ray is valid. */
      value = -1;
      }
   else if(cw_value != -1)
      {
      /* cw_ray is only ray that is within limit. */
      value = cw_value;
      }
   else
      {
      /* ccw_ray is only ray that is within limit. */
      value = ccw_value;
      }
   
   return value;
   }

/***************************************************/
/*                                                 */
/*          RSL_get_linear_value                   */
/*                                                 */
/*                                                 */
/***************************************************/
float RSL_get_linear_value(Volume *v,float srange,float azim,
                           float elev,float limit)
   {
   /* Compute bilinear value from four surrounding values
    * in Volume v.  The four surrounding values
    * are at a constant range.
    *
    * Limit is an angle used only to reject values
    * in the azimuth plane.
	*/
   float  db_value;
   double value = 0;
   double up_value, down_value;
   double delta_angle;
   Sweep  *up_sweep,*down_sweep;

   get_surrounding_sweep(&down_sweep,&up_sweep,v,elev);

   /* Calculate interpolated value in sweep above 
	* requested point.
	*/
   if(up_sweep == NULL)
      {
      up_value = -1;
      }
   else
      {
      up_value = get_linear_value_from_sweep(up_sweep,srange,azim,limit);
      }

   /* Calculate interpolated value in sweep below requested point.
	*/
   if(down_sweep == NULL)
      {
      down_value = -1;
      }
   else
      {
      down_value = get_linear_value_from_sweep(down_sweep,srange,azim,limit);
      }
   /* Using the interpolated values calculated at the elevation
    * angles in the above and below sweeps, interpolate a value
    * for the elvation angle at the requested point.
    */
   if((up_value != -1) && (down_value != -1))
      {
      delta_angle = angle_diff(up_sweep->h.elev,down_sweep->h.elev);

      value =((angle_diff(elev,up_sweep->h.elev)/delta_angle) * down_value) +
         ((angle_diff(elev,down_sweep->h.elev)/delta_angle) * up_value);
      }
   else if((up_value == -1) && (down_value == -1))
      {
      value = -1;
      }
   else if(up_value != -1)
      {
      value = up_value;
      }
   else
      {
      value = down_value;
      }

   /* Convert back to dB value and return. */
   if(value > 0)
      {
      db_value = (float)to_dB(value);
      return db_value;
      }
   else
      {
      return BADVAL;
      }
   }

