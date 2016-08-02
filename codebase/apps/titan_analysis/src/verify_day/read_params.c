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

#include "verify_day.h"

void read_params(void)

{

  char *resource_str;

  /*
   * get globals from the parameters
   */
  
  Glob->verify_dir = uGetParamString(Glob->prog_name,
				     "verify_dir",
				     VERIFY_DIR);

  Glob->verify_file_ext = uGetParamString(Glob->prog_name,
					  "verify_file_ext",
					  VERIFY_FILE_EXT);

  Glob->min_valid_history = uGetParamLong(Glob->prog_name,
					  "min_valid_history",
					  MIN_VALID_HISTORY);

  Glob->ellipse_radius_ratio =
   uGetParamDouble(Glob->prog_name,
		    "ellipse_radius_ratio",
		    ELLIPSE_RADIUS_RATIO);

  Glob->forecast_lead_time =
    uGetParamDouble(Glob->prog_name, "forecast_lead_time",
		    FORECAST_LEAD_TIME) * 60.0;

  Glob->forecast_lead_time_margin =
    uGetParamDouble(Glob->prog_name,
		    "forecast_lead_time_margin",
		    FORECAST_LEAD_TIME_MARGIN) * 60.0;
  
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
   * set verification_field option
   */
  
  resource_str =
    uGetParamString(Glob->prog_name,
		    "verification_field",
		    VERIFICATION_FIELD);
  
  if (uset_double_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str,
			&Glob->verification_field,
			"all_storms", ALL_STORMS_FIELD,
			"valid_storms", VALID_STORMS_FIELD,
			"verification_field"))
    tidy_and_exit(-1);
  
}
