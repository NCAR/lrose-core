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
/****************************************************************************
 * set_derived_params()
 *
 * Sets those params which are derived from the main ones.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * March 1996
 *
 ****************************************************************************/

#include "storm_track.h"

void set_derived_params(int n_files)

{

  if (Glob->params.malloc_debug_level > 0)
    umalloc_debug(Glob->params.malloc_debug_level);
  
  if (n_files > 0) {
    Glob->params.mode = RE_TRACK;
  }

  if (Glob->params.forecast_weights.len > MAX_NWEIGHTS_FORECAST) {
    Glob->params.forecast_weights.len = MAX_NWEIGHTS_FORECAST;
  }

  /*
   * set forecast type to match typedefs in dix_track.h
   */

  if (Glob->params.forecast_type == TREND) {
    Glob->forecast_type = FORECAST_BY_TREND;
  } else if (Glob->params.forecast_type == PARABOLIC) {
    Glob->forecast_type = FORECAST_BY_PARABOLA;
  } else {
    Glob->forecast_type = FORECAST_BY_REGRESSION;
  }

}
