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
/***************************************************************************
 * perform_analysis.c
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1993
 *
 **************************************************************************/

#include "mhr_analysis.h"

void perform_analysis (char *beam_buffer)

{

  ui08 *refl_bytes;
  ui08 *vel_bytes;
  ui08 *width_bytes;
  
  int i;
  int refl_field = 0;
  int vel_field = 1;
  int width_field = 2;

  si32 ngates;
  si32 nfields;

  double cf = 360.0 / 65535.0;
  double start_range;
  double gate_spacing;
  
  double refl_scale, refl_bias;
  double vel_scale, vel_bias;
  double width_scale, width_bias;

  double *range_p;
  double *refl_p;
  double *vel_p;
  double *width_p;

  static si32 prev_ngates = 0;
  static double *refl = NULL;
  static double *vel = NULL;
  static double *width = NULL;
  static double *range = NULL;

  rp7_params_t *rp7_params = (rp7_params_t *) beam_buffer;
  
  /*
   * get parameters
   */

  ngates = rp7_params->gates_per_beam;
  gate_spacing = (double) rp7_params->gate_spacing / 1000.0;
  start_range = gate_spacing / 2.0;
  nfields = rp7_params->nfields;

  refl_scale = ((double) rp7_params->lscale[refl_field].scale /
		(double) RP7_SCALE_AND_BIAS_MULT);

  refl_bias = ((double) rp7_params->lscale[refl_field].bias /
		(double) RP7_SCALE_AND_BIAS_MULT);

  vel_scale = ((double) rp7_params->lscale[vel_field].scale /
		(double) RP7_SCALE_AND_BIAS_MULT);

  vel_bias = ((double) rp7_params->lscale[vel_field].bias /
		(double) RP7_SCALE_AND_BIAS_MULT);

  width_scale = ((double) rp7_params->lscale[width_field].scale /
		(double) RP7_SCALE_AND_BIAS_MULT);

  width_bias = ((double) rp7_params->lscale[width_field].bias /
		(double) RP7_SCALE_AND_BIAS_MULT);

  /*
   * allocate arrays
   */

  if (prev_ngates == 0) {

    refl = (double *) malloc ((ui32) (ngates * sizeof(double)));
    vel = (double *) malloc ((ui32) (ngates * sizeof(double)));
    width = (double *) malloc ((ui32) (ngates * sizeof(double)));
    range = (double *) malloc ((ui32) (ngates * sizeof(double)));
    
  } else if (prev_ngates != ngates) {

    refl = (double *) realloc((void *) refl,
			      (ui32) (ngates * sizeof(double)));

    vel = (double *) realloc((void *) vel,
			      (ui32) (ngates * sizeof(double)));

    width = (double *) realloc((void *) width,
			      (ui32) (ngates * sizeof(double)));

    range = (double *) realloc((void *) range,
			      (ui32) (ngates * sizeof(double)));

    prev_ngates = ngates;


  } /* if (prev_ngates == 0) */

  if (refl == NULL ||
      vel == NULL ||
      width == NULL ||
      range == NULL) {
    
    fprintf(stderr, "ERROR - %s:perform_analysis\n", Glob->prog_name);
    fprintf(stderr, "Could not allocate arrays\n");
    tidy_and_exit(-1);

  }

  /*
   *  set arrays
   */

  refl_bytes =
    (ui08 *) beam_buffer + sizeof(rp7_params_t) + refl_field;

  vel_bytes =
    (ui08 *) beam_buffer + sizeof(rp7_params_t) + vel_field;

  width_bytes =
    (ui08 *) beam_buffer + sizeof(rp7_params_t) + width_field;

  range_p = range;
  refl_p = refl;
  vel_p = vel;
  width_p = width;

  for (i = 0; i < ngates; i++) {

    *range_p = start_range + gate_spacing * (double) i;

    *refl_p = (double) *refl_bytes * refl_scale + refl_bias;

    *vel_p = (double) *vel_bytes * vel_scale + vel_bias;

    *width_p = (double) *width_bytes * width_scale + width_bias;

    range_p++;
    refl_p++;
    vel_p++;
    width_p++;

    refl_bytes += nfields;
    vel_bytes += nfields;
    width_bytes += nfields;

  }

  printf("***************\n");
  printf("perform_analysis\n");
  printf("date                    %.4d/%.2d/%.2d\n",
	 rp7_params->year, rp7_params->month, rp7_params->day);
  printf("time                    %.2d:%.2d:%.2d\n",
	 rp7_params->hour,rp7_params->min, rp7_params->sec);
  
  printf("azimuth (deg)           %g\n",
	 (double) rp7_params->azimuth * cf);
  printf("elevation (deg)         %g\n",
	 (double) rp7_params->elevation * cf);
    
  printf("ngates                  %ld\n", (long) ngates);
  printf("range to first gate(km) %g\n", start_range);
  printf("gate spacing(km)        %g\n", gate_spacing);

  for (i = 0; i < ngates; i++) {

    printf("range: %10g      refl: %10g      vel: %10g      width: %10g\n",
	   range[i], refl[i], vel[i], width[i]);

  }
      
  fflush(stdout);
  
}
