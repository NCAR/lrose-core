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

#include "grid_forecast.hh"

void read_params(void)

{

  /*
   * get globals from the parameters
   */
  
  Glob->original_rdata_dir = uGetParamString(Glob->prog_name,
					     "original_rdata_dir",
					     ORIGINAL_RDATA_DIR);

  Glob->forecast_rdata_dir = uGetParamString(Glob->prog_name,
					     "forecast_rdata_dir",
					     FORECAST_RDATA_DIR);

  Glob->dobson_file_ext = uGetParamString(Glob->prog_name,
					  "dobson_file_ext",
					  DOBSON_FILE_EXT);
  
  Glob->min_history_in_secs = uGetParamLong(Glob->prog_name,
					  "min_history_in_secs",
					  MIN_HISTORY_IN_SECS);

  Glob->forecast_lead_time = uGetParamLong(Glob->prog_name,
					   "forecast_lead_time",
					   FORECAST_LEAD_TIME);

  Glob->dbz_field = uGetParamLong(Glob->prog_name,
				  "dbz_field",
				  DBZ_FIELD);

}
