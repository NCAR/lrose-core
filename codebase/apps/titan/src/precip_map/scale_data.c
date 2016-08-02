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

#include "precip_map.h"
#include "sys/types.h"
#include "sys/stat.h"

/****************************************************************************
 * scale_data.c
 *
 * scales the data and stores in ui08 plane, sets scale and bias
 *
 * October 1993
 *
 ****************************************************************************/

void scale_data(double *dprecip,
		ui08 *uprecip,
		si32 npoints,
		si32 factor,
		si32 *scale_p,
		si32 *bias_p)
		
{

  ui08 *up;

  int byte_val;
  si32 i;

  double *dp;
  double minval, maxval;
  double range;
  double scale, tmp_scale;
  double bias, tmp_bias;

  /*
   * determine the min and max values in the field
   */

  minval = 1.0e100;
  maxval = -1.0e100;

  dp = dprecip;
  
  for (i = 0; i < npoints; i++) {

    if (*dp > maxval)
      maxval = *dp;
	
    if (*dp != 0.0 && *dp < minval)
      minval = *dp;

    dp++;

  } /* i */

  if (minval == 1.0e100)
    minval = -1.0;

  /*
   * make sure the extreme values are not the same
   */

  if(minval == maxval) {
    minval -= 1.0;
    maxval += 1.0;
  }

  /*
   * extend the range by 5 % on each end
   */
  
  range = maxval - minval;
  maxval += range / 20.0;
  minval -= range / 20.0;

  /*
   * compute the scale and bias - the data must be scaled between
   * 1 and 255.
   */

  tmp_scale = (maxval - minval) / 254.0;
  tmp_bias = minval - 1.0 * tmp_scale;

  *bias_p = (si32) floor (tmp_bias * (double) factor + 0.5);
  bias = (double) *bias_p / (double) factor;
    
  *scale_p = (si32) floor (tmp_scale * (double) factor + 0.5);
  scale = (double) *scale_p / (double) factor;
    
  if (Glob->params.debug >= DEBUG_EXTRA) {
    
    fprintf(stderr, "      Min, max : %g, %g\n", minval, maxval);
    fprintf(stderr, "      Bias, scale : %g, %g\n", bias, scale);
    
  }
  
  /*
   * store the scaled data in the field plane
   */
  
  up = uprecip;
  dp = dprecip;
  
  for (i = 0; i < npoints; i++, dp++, up++) {

    if (*dp < 0.01)
      byte_val = 0;
    else
      byte_val = (ui08) ((*dp - bias) / scale + 0.5);
    
    if (byte_val < 2) {
      byte_val = 0;
    } else if (byte_val > 254) {
      byte_val = 254;
    }

    *up = byte_val;

  } /* i */

}



