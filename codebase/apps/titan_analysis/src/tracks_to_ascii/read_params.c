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

#include "tracks_to_ascii.h"

void read_params(void)

{
  
  char *resource_str;
  
  Glob->sample_interval = (si32) floor
    (uGetParamDouble(Glob->prog_name,
		     "sample_interval", SAMPLE_INTERVAL) * 60.0 + 0.5);
  
  /*
   * NOTE - the scan interval is increased by 10% to give a more reliable
   * search for entries (see entry_comps())
   */
  
  Glob->scan_interval = (si32) floor
    (uGetParamDouble(Glob->prog_name,
		     "scan_interval", SCAN_INTERVAL) * 60.0 + 0.5);
  
  Glob->min_duration = uGetParamLong(Glob->prog_name,
				     "min_duration", MIN_DURATION);
  
  Glob->nscans_pre_trend = uGetParamLong(Glob->prog_name,
					 "nscans_pre_trend",
					 NSCANS_PRE_TREND);
  
  Glob->min_nscans_pre_trend = uGetParamLong(Glob->prog_name,
					     "min_nscans_pre_trend",
					     MIN_NSCANS_PRE_TREND);
  
  Glob->nscans_post_trend = uGetParamLong(Glob->prog_name,
					  "nscans_post_trend",
					  NSCANS_POST_TREND);
  
  Glob->min_nscans_post_trend = uGetParamLong(Glob->prog_name,
					      "min_nscans_post_trend",
					      MIN_NSCANS_POST_TREND);
  
  Glob->min_nscans_monotonic =
    uGetParamLong(Glob->prog_name,
		  "min_nscans_monotonic",
		  MIN_NSCANS_MONOTONIC);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "target_entity",
				 TARGET_ENTITY);
  
  uset_triple_param(Glob->prog_name, "read_params",
		    Glob->params_path_name,
		    resource_str, &Glob->target_entity,
		    "complete_track", COMPLETE_TRACK,
		    "track_entry", TRACK_ENTRY,
		    "trends", TRENDS,
		    "target_entity");
  
  resource_str = uGetParamString(Glob->prog_name,
				 "use_simple_tracks",
				 USE_SIMPLE_TRACKS);
  
  uset_true_false_param(Glob->prog_name, "read_params",
			Glob->params_path_name,
			resource_str, &Glob->use_simple_tracks,
			"use_simple_tracks");
  
  resource_str = uGetParamString(Glob->prog_name,
				 "use_complex_tracks",
				 USE_COMPLEX_TRACKS);
  
  uset_true_false_param(Glob->prog_name, "read_params",
			Glob->params_path_name,
			resource_str, &Glob->use_complex_tracks,
			"use_complex_tracks");
  
  resource_str = uGetParamString(Glob->prog_name,
				 "use_box_limits",
				 USE_BOX_LIMITS);
  
  uset_true_false_param(Glob->prog_name, "read_params",
			Glob->params_path_name,
			resource_str, &Glob->use_box_limits,
			"use_box_limits");
  
  resource_str = uGetParamString(Glob->prog_name,
				 "print_polygons",
				 PRINT_POLYGONS);
  
  uset_true_false_param(Glob->prog_name, "read_params",
			Glob->params_path_name,
			resource_str, &Glob->print_polygons,
			"print_polygons");
  
  resource_str = uGetParamString(Glob->prog_name,
				 "nonzero_verification_only",
				 NONZERO_VERIFICATION_ONLY);
  
  uset_true_false_param(Glob->prog_name, "read_params",
			Glob->params_path_name,
			resource_str,
			&Glob->nonzero_verification_only,
			"nonzero_verification_only");
  
  Glob->box_min_x = uGetParamDouble(Glob->prog_name,
				    "box_min_x", BOX_MIN_X);
  Glob->box_min_y = uGetParamDouble(Glob->prog_name,
				    "box_min_y", BOX_MIN_Y);
  Glob->box_max_x = uGetParamDouble(Glob->prog_name,
				    "box_max_x", BOX_MAX_X);
  Glob->box_max_y = uGetParamDouble(Glob->prog_name,
				    "box_max_y", BOX_MAX_Y);
  
  Glob->min_percent_in_box = uGetParamDouble(Glob->prog_name,
					     "min_percent_in_box",
					     MIN_PERCENT_IN_BOX);
  
  Glob->min_nstorms_in_box = uGetParamLong(Glob->prog_name,
					   "min_nstorms_in_box",
					   MIN_NSTORMS_IN_BOX);
  
  Glob->max_top_missing = uGetParamLong(Glob->prog_name,
					"max_top_missing",
					MAX_TOP_MISSING);
  
  Glob->max_range_limited = uGetParamLong(Glob->prog_name,
					  "max_range_limited",
					  MAX_RANGE_LIMITED);
  
  Glob->max_vol_at_start_of_sampling =
    uGetParamLong(Glob->prog_name,
		  "max_vol_at_start_of_sampling",
		  MAX_VOL_AT_START_OF_SAMPLING);
  
  Glob->max_vol_at_end_of_sampling =
    uGetParamLong(Glob->prog_name,
		  "max_vol_at_end_of_sampling",
		  MAX_VOL_AT_END_OF_SAMPLING);
  
  Glob->vol_percentile = uGetParamDouble(Glob->prog_name,
					 "vol_percentile",
					 VOL_PERCENTILE);

  Glob->dbz_for_max_ht = uGetParamDouble(Glob->prog_name,
					 "dbz_for_max_ht",
					 DBZ_FOR_MAX_HT);

  Glob->dbz_for_percent_vol_above =
    uGetParamDouble(Glob->prog_name,
		    "dbz_for_percent_vol_above",
		    DBZ_FOR_PERCENT_VOL_ABOVE);
  
}

