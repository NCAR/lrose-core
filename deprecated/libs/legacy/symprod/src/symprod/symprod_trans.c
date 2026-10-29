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
/*********************************************************************
 * symprod_trans.c
 *
 * Transformation routines for SYMPROD objects.  These routines do
 * things like rotate objects around an axis.
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * Sept 1997
 *
 * Based on routines in the prodserv library.
 *
 *********************************************************************/

#include <stdio.h>
#include <math.h>

#include <symprod/symprod.h>

/**********************************************************************
 * Exported routines.
 **********************************************************************/


/**********************************************************************
 * SYMPRODpptDistance() - Calculate the distance between two pixel
 *                        points.  The value returned is in pixels.
 */

double SYMPRODpptDistance(symprod_ppt_t point1,
			  symprod_ppt_t point2)
{
  double dx, dy;
    
  dx = (double) (point1.x - point2.x);
  dy = (double) (point1.y - point2.y);
    
  return(sqrt(dx*dx + dy*dy));
}


/**********************************************************************
 * SYMPRODrotatePptArray() - Rotate an array of pixel points by the
 *                           given angle.  The array is rotated around
 *                           the point (0,0) and the angle is given in
 *                           radians.
 */

void SYMPRODrotatePptArray(symprod_ppt_t *array,
			   int n_points,
			   double angle_rad)
{
  double angle_sin = sin(angle_rad);
  double angle_cos = cos(angle_rad);
  
  int i;
  int old_x, old_y;
  
  for (i = 0; i < n_points; i++)
  {
    old_x = array[i].x;
    old_y = array[i].y;
    
    array[i].x = (si32)((old_x * angle_cos - old_y * angle_sin) + 0.5);
    array[i].y = (si32)((old_y * angle_cos + old_x * angle_sin) + 0.5);
  }
  
  return;  
}


/**********************************************************************
 * SYMPRODrotateWptArray() - Rotate an array of world points by the
 *                           given angle.  The array is rotated around
 *                           the point (0,0) and the angle is given in
 *                           radians.
 */

void SYMPRODrotateWptArray(symprod_wpt_t *array,
			   int n_points,
			   double angle_rad)
{
  double angle_sin = sin(angle_rad);
  double angle_cos = cos(angle_rad);
  
  int     i;
  double  old_x, old_y;
    
  for (i = 0; i < n_points; i++)
  {
    old_x = array[i].lon;
    old_y = array[i].lat;

    array[i].lon = (old_x * angle_cos - old_y * angle_sin);
    array[i].lat = (old_y * angle_cos + old_x * angle_sin);
  }

  return;    
}


/**********************************************************************
 * SYMPRODscalePptArray() - Scale an array of pixel points by the
 *                          given factor.
 */

void SYMPRODscalePptArray(symprod_ppt_t *array,
			  int n_points,
			  double factor)
{
  int i;
    
  for (i = 0; i < n_points; i++)
  {
    array[i].x = (int)(((double)array[i].x * factor) + 0.5);
    array[i].y = (int)(((double)array[i].y * factor) + 0.5);
  }

  return;    
}


/**********************************************************************
 * SYMPRODscaleWptArray() - Scale an array of world points by the
 *                          given factor.
 */

void SYMPRODscaleWptArray(symprod_wpt_t *array,
			  int n_points,
			  double factor)
{
  int i;
    
  for (i = 0; i < n_points; i++)
  {
    array[i].lon *= factor;
    array[i].lat *= factor;
  }

  return;    
}


/**********************************************************************
 * SYMPRODtranslatePptArray() - Translate an array of pixel points by the
 *                              given values.  The values are assumed to
 *                              be in pixels.
 */

void SYMPRODtranslatePptArray(symprod_ppt_t *array,
			      int n_points,
			      int trans_x,
			      int trans_y)
{
  int i;
    
  for (i = 0; i < n_points; i++)
  {
    array[i].x += trans_x;
    array[i].y += trans_y;
  }

  return;    
}


/**********************************************************************
 * SYMPRODtranslateWptArray() - Translate an array of world points by the
 *                              given values.  The values are assumed to
 *                              be in degrees.
 */

void SYMPRODtranslateWptArray(symprod_wpt_t *array,
			      int n_points,
			      double trans_lat,
			      double trans_lon)
{
  int i;
    
  for (i = 0; i < n_points; i++)
  {
    array[i].lon += trans_lon;
    array[i].lat += trans_lat;
  }

  return;    
}


/**********************************************************************
 * SYMPRODwptDistance() - Calculate the distance between two world
 *                        points.  The value returned is in degrees.
 */

double SYMPRODwptDistance(symprod_wpt_t point1,
			  symprod_wpt_t point2)
{
  double dx, dy;
    
  dx = (double) (point1.lon - point2.lon);
  dy = (double) (point1.lat - point2.lat);
    
  return(sqrt(dx*dx + dy*dy));
}


/**********************************************************************
 * Static routines.
 **********************************************************************/
