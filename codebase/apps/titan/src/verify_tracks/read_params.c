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
 * read_params.c: reads the parameters, loads up the globals
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "verify_tracks.h"

void read_params(void)

{

  char *resource_str;

  /*
   * get globals from the parameters
   */
  
  Glob->forecast_lead_time =
    uGetParamDouble(Glob->prog_name,
		    "forecast_lead_time",
		    FORECAST_LEAD_TIME) * 60.0;

  Glob->forecast_lead_time_margin =
    uGetParamDouble(Glob->prog_name,
		    "forecast_lead_time_margin",
		    FORECAST_LEAD_TIME_MARGIN) * 60.0;

  Glob->forecast_min_history = (long) floor
    (uGetParamDouble(Glob->prog_name,
		     "forecast_min_history",
		     FORECAST_MIN_HISTORY) * 60.0 + 0.5);
  
  Glob->forecast_scale_factor =
    uGetParamDouble(Glob->prog_name,
		    "forecast_scale_factor",
		    FORECAST_SCALE_FACTOR);

  /*
   * grid geometry
   */

  Glob->nx = uGetParamLong(Glob->prog_name, "nx", NX);
  Glob->ny = uGetParamLong(Glob->prog_name, "ny", NY);

  Glob->minx = uGetParamDouble(Glob->prog_name, "minx", MINX);
  Glob->miny = uGetParamDouble(Glob->prog_name, "miny", MINY);

  Glob->dx = uGetParamDouble(Glob->prog_name, "dx", DX);
  Glob->dy = uGetParamDouble(Glob->prog_name, "dy", DY);


  /*
   * set option for zero growth
   */

  resource_str =
    uGetParamString(Glob->prog_name, "zero_growth", ZERO_GROWTH);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->zero_growth,
			    "zero_growth"))
    exit(-1);

  /*
   * set parabolic growth option
   */
  
  resource_str =
    uGetParamString(Glob->prog_name,
		    "parabolic_growth",
		    PARABOLIC_GROWTH);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->parabolic_growth,
			    "parabolic_growth"))
    exit(-1);
  
  Glob->forecast_growth_period =
    uGetParamDouble(Glob->prog_name,
		    "forecast_growth_period",
		    FORECAST_GROWTH_PERIOD) * 60.0;

  /*
   * set verification scope options
   */
  
  resource_str =
    uGetParamString(Glob->prog_name,
		    "verify_before_forecast_time",
		    VERIFY_BEFORE_FORECAST_TIME);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->verify_before_forecast_time,
			    "verify_before_forecast_time"))
    exit(-1);

  resource_str =
    uGetParamString(Glob->prog_name,
		    "verify_after_track_dies",
		    VERIFY_AFTER_TRACK_DIES);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->verify_after_track_dies,
			    "verify_after_track_dies"))
    exit(-1);

  /*
   * set option for forcing xy fit
   */

  resource_str =
    uGetParamString(Glob->prog_name,
		    "force_xy_fit",
		    FORCE_XY_FIT);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->force_xy_fit,
			    "force_xy_fit"))
    exit(-1);

  /*
   * set option for forecast method
   */

  resource_str =
    uGetParamString(Glob->prog_name,
		    "forecast_method",
		    FORECAST_METHOD);
  
  if (uset_double_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str,
			&Glob->forecast_method,
			"ellipse", STORM_ELLIPSE,
			"polygon", STORM_POLYGON,
			"forecast_method"))
    exit(-1);
  
  /*
   * set option for verify method
   */

  resource_str =
    uGetParamString(Glob->prog_name,
		    "verify_method",
		    VERIFY_METHOD);
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str,
			&Glob->verify_method,
			"ellipse", STORM_ELLIPSE,
			"polygon", STORM_POLYGON,
			"runs", STORM_RUNS,
			"verify_method"))
    exit(-1);
  
  /*
   * set option for activity criterion
   */

  resource_str = uGetParamString(Glob->prog_name,
				 "activity_criterion",
				 ACTIVITY_CRITERION);
  
  if (uset_double_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str,
			&Glob->activity_criterion,
			"point", POINT_ACTIVITY,
			"area", AREA_ACTIVITY,
			"activity_criterion"))
    exit(-1);

  Glob->activity_radius =
    uGetParamDouble(Glob->prog_name, "activity_radius", ACTIVITY_RADIUS);

  Glob->activity_fraction =
    uGetParamDouble(Glob->prog_name, "activity_fraction", ACTIVITY_FRACTION);

}

