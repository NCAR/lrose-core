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
 * check_geom.c: check that the radar has the same geometry as
 *               that used to create the lookup table
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "cart_convert.h"

void check_geom(vol_file_handle_t *rv_handle,
		rc_table_file_handle_t *rc_handle)

{

  int error_flag = FALSE;
  int i;

  rc_table_params_t *rcparams;
  double *rc_elevations;

  rcparams = rc_handle->table_params;
  rc_elevations = rc_handle->scan_table->elev_angles;

  /*
   * check the two sets of parameters against each other to ensure match
   */

  if(rv_handle->vol_params->radar.altitude != rcparams->cart.radarz) {

    fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Radar altitude does not match.\n");
    fprintf(stderr, "Data radar altitude = %d\n",
	    rv_handle->vol_params->radar.altitude);
    fprintf(stderr, "Lookup table radar altitude = %d\n",
	    rcparams->cart.radarz);
    error_flag = TRUE;

  }

  if(rv_handle->vol_params->radar.nazimuths != rcparams->nazimuths) {

    fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "No of azimuths do not match.\n");
    fprintf(stderr, "Radar volume nazimuths = %d\n",
	    rv_handle->vol_params->radar.nazimuths);
    fprintf(stderr, "Lookup table nazimuths = %d\n",
	    rcparams->nazimuths);
    error_flag = TRUE;

  }

  if(rv_handle->vol_params->radar.nelevations != rcparams->nelevations) {

    fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "No of elevations do not match.\n");
    fprintf(stderr, "Radar volume nelevations = %d\n",
	    rv_handle->vol_params->radar.nelevations);
    fprintf(stderr, "Lookup table nelevations = %d\n",
	    rcparams->nelevations);
    error_flag = TRUE;

  }

  if(rv_handle->vol_params->radar.ngates != rcparams->ngates) {

    fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "No of gates do not match.\n");
    fprintf(stderr, "Radar volume ngates = %d\n",
	    rv_handle->vol_params->radar.ngates);
    fprintf(stderr, "Lookup table ngates = %d\n",
	    rcparams->ngates);
    error_flag = TRUE;

  }

  if(rv_handle->vol_params->radar.gate_spacing != rcparams->gate_spacing) {

    fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Gate spacing does not match.\n");
    fprintf(stderr, "Radar volume gate_spacing = %d\n",
	    rv_handle->vol_params->radar.gate_spacing);
    fprintf(stderr, "Lookup table gate_spacing = %d\n",
	    rcparams->gate_spacing);
    error_flag = TRUE;

  }

  if(rv_handle->vol_params->radar.start_range != rcparams->start_range) {

    fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Start range does not match.\n");
    fprintf(stderr, "Radar volume start_range = %d\n",
	    rv_handle->vol_params->radar.start_range);
    fprintf(stderr, "Lookup table start_range = %d\n",
	    rcparams->start_range);
    error_flag = TRUE;

  }

  if(rv_handle->vol_params->radar.start_azimuth != rcparams->start_azimuth) {

    fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Start azimuth does not match.\n");
    fprintf(stderr, "Radar volume start_azimuth = %g\n",
	    (float) rv_handle->vol_params->radar.start_azimuth / 1000.0);
    fprintf(stderr, "Lookup table start_azimuth = %g\n",
	    (float) rcparams->start_azimuth / 1000.0);
    error_flag = TRUE;

  }

  if(rv_handle->vol_params->radar.delta_azimuth != rcparams->delta_azimuth) {

    fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Delta does not match.\n");
    fprintf(stderr, "Radar volume delta_azimuth = %g\n",
	    (float) rv_handle->vol_params->radar.delta_azimuth / 1000.0);
    fprintf(stderr, "Lookup table delta_azimuth = %g\n",
	    (float) rcparams->delta_azimuth / 1000.0);
    error_flag = TRUE;

  }

  if(rv_handle->vol_params->radar.beam_width != rcparams->beam_width) {

    fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr, "Beam width does not match.\n");
    fprintf(stderr, "Radar volume beam_width = %g\n",
	    (float) rv_handle->vol_params->radar.beam_width / 1000.0);
    fprintf(stderr, "Lookup table beam_width = %g\n",
	    (float) rcparams->beam_width / 1000.0);
    error_flag = TRUE;

  }

  /*
   * check elevations
   */

  if (rv_handle->vol_params->radar.nelevations == rcparams->nelevations) {

    for (i = 0; i < rv_handle->vol_params->radar.nelevations; i++) {

      if (rv_handle->radar_elevations[i] / DEG_FACTOR
	  != rc_elevations[i]) {

	fprintf(stderr, "ERROR - %s:check_geom\n", Glob->prog_name);
	fprintf(stderr, "Elevation %d does not match.\n", i);
	fprintf(stderr, "Radar volume elevation[%d] = %g\n",
		i, (double) rv_handle->radar_elevations[i] / DEG_FACTOR);
	fprintf(stderr, "Lookup table elevation[%d] = %g\n",
		i, (double) rc_elevations[i] / DEG_FACTOR);
	error_flag = TRUE;
	
      } /* if (rindex ... */

    } /* i */

  } /* if (rindex ... */

  if (error_flag == TRUE)
    exit(1);

}
