// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/***************************************************************************
 * update_params.c
 *
 * set up the shared memory header and buffer, and load in the
 * relevant parameters
 *
 * Returns 1 if params have changed, 0 otherwise
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 **************************************************************************/

#include "polar2gate.h"
#include <dataport/bigend.h>

int update_params(ui08 *beam_buffer,
		  ui08 *gate_params_pkt,
		  si32 nbytes_params_pkt)

{

  static int first_call = TRUE;
  static si32 nbytes_fparams;

  int params_have_changed = FALSE;

  si32 ifield;
  si32 field_num;

  static gate_data_radar_params_t rparams;
  static gate_data_field_params_t *fparams;

  static gate_data_radar_params_t prev_rparams;
  static gate_data_field_params_t *prev_fparams;

  gate_data_radar_params_t *pkt_rparams;
  gate_data_field_params_t *pkt_fparams;

  ll_params_t *ll_params;
  rp7_params_t *rp7_params;
  chill_params_t *ch_params;
  lass_params_t *lass_param_p;
  alenia_params_t *al_params;

  if (first_call) {

    /*
     * allocate field params, and initialize
     */

    nbytes_fparams = Glob->nfields_out * sizeof(gate_data_field_params_t);

    fparams = (gate_data_field_params_t *) umalloc
      ((ui32) nbytes_fparams);

    prev_fparams = (gate_data_field_params_t *) umalloc
      ((ui32) nbytes_fparams);

    memset ((void *)  &prev_rparams,
            (int) 0, (size_t) sizeof(gate_data_radar_params_t));
    
    memset ((void *)  prev_fparams,
            (int) 0, (size_t) nbytes_fparams);

    rparams.radar_id = Glob->radar_id;
    rparams.altitude = (si32) (Glob->altitude * 1000.0 + 0.5);
    rparams.latitude = (si32) floor (Glob->latitude * 1000000.0 + 0.5);
    rparams.longitude = (si32) floor (Glob->longitude * 1000000.0 + 0.5);
    rparams.ngates = Glob->ngates_out;
    rparams.beam_width = (si32) (Glob->beam_width * 1000000.0 + 0.5);
    rparams.nfields = Glob->nfields_out;
    rparams.data_field_by_field = Glob->data_field_by_field;

  } /* if (first_call) */

  /*
   * set radar params
   */

  /*
   * set field params
   */

  switch (Glob->header_type) {

  case LINCOLN_HEADER:

    ll_params = (ll_params_t *) beam_buffer;
    
    rparams.gate_spacing = (si32) (Glob->gate_spacing * 1000000.0 + 0.5);
    rparams.start_range = (si32) (Glob->start_range * 1000000.0 + 0.5);
    rparams.samples_per_beam = Glob->samples_per_beam;
    rparams.pulse_width = (si32) (Glob->pulse_width * 1000.0 + 0.5);
    rparams.prf = (si32) (Glob->prf_nominal * 1000.0 + 0.5);
    rparams.wavelength = (si32) (Glob->wavelength * 10000.0 + 0.5);
    rparams.scan_mode = Glob->scan_mode;
    rparams.scan_type = 0;

    rparams.field_flag = 0L;

    for (ifield = 0; ifield < rparams.nfields; ifield++) {
      rparams.field_flag <<= 1;
      rparams.field_flag |= 1L;
    } /* ifield */
    
    rparams.nfields_current = rparams.nfields;

    if (Glob->nfields_in != LL_NFIELDS) {
      
      fprintf(stderr, "ERROR - %s:update_params\n", Glob->prog_name);
      fprintf(stderr,
	      "Number of input fields requested - %ld - incorrect.\n",
	      (long) Glob->nfields_in);
      fprintf(stderr, "Number of fields for Lincoln format is %d.\n",
	      LL_NFIELDS);
      tidy_and_exit(-1);
      
    } /* if (Glob->nfields_in > ... */
    
    for (ifield = 0; ifield < Glob->nfields_out; ifield++) {
      
      field_num = Glob->out_field_pos[ifield];

      if (field_num > LL_NFIELDS - 1) {

	fprintf(stderr, "ERROR - %s:update_params\n", Glob->prog_name);
	fprintf(stderr,
		"Output field position - %ld - too great.\n",
		(long) Glob->out_field_pos[ifield]);
	fprintf(stderr, "Number of fields for Lincoln format is %d.\n",
		LL_NFIELDS);
	tidy_and_exit(-1);

      }
      
      fparams[ifield].scale =
	(si32) (((double) ll_params->scale[field_num] * 
		 (double) RDATA_SCALE_AND_BIAS_MULT /
		 (double) LL_SCALE_AND_BIAS_MULT) + 0.5);

      fparams[ifield].bias =
	(si32) (((double) ll_params->bias[field_num] * 
		 (double) RDATA_SCALE_AND_BIAS_MULT /
		 (double) LL_SCALE_AND_BIAS_MULT) + 0.5);

      fparams[ifield].factor = RDATA_SCALE_AND_BIAS_MULT;

    } /* ifield */
  
    break;

  case RP7_HEADER:

    rp7_params = (rp7_params_t *) beam_buffer;
    
    rparams.gate_spacing = (si32) (Glob->gate_spacing * 1000000.0 + 0.5);
    rparams.start_range = (si32) (Glob->start_range * 1000000.0 + 0.5);
    rparams.samples_per_beam = Glob->samples_per_beam;
    rparams.pulse_width = (si32) (Glob->pulse_width * 1000.0 + 0.5);
    rparams.prf = (si32) (Glob->prf_nominal * 1000.0 + 0.5);
    rparams.wavelength = (si32) (Glob->wavelength * 10000.0 + 0.5);
    rparams.scan_mode = Glob->scan_mode;
    rparams.scan_type = 0;

    rparams.field_flag = 0L;

    for (ifield = 0; ifield < rparams.nfields; ifield++) {
      rparams.field_flag <<= 1;
      rparams.field_flag |= 1L;
    } /* ifield */
    
    rparams.nfields_current = rparams.nfields;

    for (ifield = 0; ifield < Glob->nfields_out; ifield++) {
      
      field_num = Glob->out_field_pos[ifield];

      if (field_num > (si32) (rp7_params->nfields - 1)) {

	fprintf(stderr, "ERROR - %s:update_params\n", Glob->prog_name);
	fprintf(stderr,
		"Output field position - %ld - too great.\n",
		(long) Glob->out_field_pos[ifield]);
	fprintf(stderr, "Number of fields in data is %d.\n",
		rp7_params->nfields);
	tidy_and_exit(-1);

      }
      
      fparams[ifield].scale =
	(si32) (((double) rp7_params->lscale[field_num].scale *
		 (double) RDATA_SCALE_AND_BIAS_MULT /
		 (double) RP7_SCALE_AND_BIAS_MULT) + 0.5);

      fparams[ifield].bias =
	(si32) (((double) rp7_params->lscale[field_num].bias *
		 (double) RDATA_SCALE_AND_BIAS_MULT /
		 (double) RP7_SCALE_AND_BIAS_MULT) + 0.5);

      fparams[ifield].factor = RDATA_SCALE_AND_BIAS_MULT;

    } /* ifield */
  
    break;

  case ALENIA_HEADER:

    al_params = (alenia_params_t *) beam_buffer;
   
    rparams.gate_spacing =
      (si32) (al_params->gate_spacing * 1000000.0 + 0.5);
    rparams.start_range =
      (si32) (al_params->start_range * 1000000.0 + 0.5);
    rparams.samples_per_beam = al_params->npulses;
    rparams.pulse_width = (si32) (al_params->pulse_width * 1000.0 + 0.5);
    rparams.prf = (si32) (Glob->prf_nominal * 1000.0 + 0.5);
    rparams.wavelength = (si32) (Glob->wavelength * 10000.0 + 0.5);
    rparams.scan_mode = al_params->scan_mode;
    rparams.scan_type = 0;

    rparams.field_flag = 0L;

    for (ifield = 0; ifield < rparams.nfields; ifield++) 
    {
      rparams.field_flag <<= 1;
      rparams.field_flag |= 1L;
    } /* ifield */
    
    rparams.nfields_current = rparams.nfields;

    for (ifield = 0; ifield < Glob->nfields_out; ifield++) {
      
      field_num = Glob->out_field_pos[ifield];

      if (field_num > N_ALENIA_FIELDS) {
	fprintf(stderr, "ERROR - %s:update_params\n", Glob->prog_name);
	fprintf(stderr,
		"Output field position - %ld - too great.\n",
		(long) Glob->out_field_pos[ifield]);
	fprintf(stderr, "Number of fields in data is %d.\n",
		N_ALENIA_FIELDS);
	tidy_and_exit(-1);
      }
      
      fparams[ifield].factor = RDATA_SCALE_AND_BIAS_MULT;

      fparams[ifield].scale =
	(si32) ((al_params->scale[field_num] *
		 (double) RDATA_SCALE_AND_BIAS_MULT) + 0.5);

      fparams[ifield].bias =
	(si32) ((al_params->bias[field_num] * 
		 (double) RDATA_SCALE_AND_BIAS_MULT) + 0.5);

    } /* ifield */
  
    break;

  case LASS_HEADER:

    lass_param_p = (lass_params_t *) beam_buffer;
   
    rparams.gate_spacing = (si32) (Glob->gate_spacing * 1000000.0 + 0.5);
    rparams.start_range = (si32) (Glob->start_range * 1000000.0 + 0.5);
    rparams.samples_per_beam = Glob->samples_per_beam;
    rparams.pulse_width = (si32) (Glob->pulse_width * 1000.0 + 0.5);
    rparams.prf = (si32) (Glob->prf_nominal * 1000.0 + 0.5);
    rparams.wavelength = (si32) (Glob->wavelength * 10000.0 + 0.5);
    rparams.scan_mode = Glob->scan_mode;
    rparams.scan_type = 0;

    rparams.field_flag = 0L;

    for (ifield = 0; ifield < rparams.nfields; ifield++) 
    {
      rparams.field_flag <<= 1;
      rparams.field_flag |= 1L;
    } /* ifield */
    
    rparams.nfields_current = rparams.nfields;

    for (ifield = 0; ifield < Glob->nfields_out; ifield++) 
    {
      
      field_num = Glob->out_field_pos[ifield];

      if (field_num > (si32) 2) 
      {

	      fprintf(stderr, "ERROR - %s:update_params\n", Glob->prog_name);
	      fprintf(stderr,
		         "Output field position - %ld - too great.\n",
		          (long) Glob->out_field_pos[ifield]);
	      fprintf(stderr, "Number of fields in data is %d.\n",
		          rp7_params->nfields);
	      tidy_and_exit(-1);

      }
      
      fparams[ifield].scale =
	     (si32) (( (double) lass_param_p->scaleFactor[field_num] *
		           (double) RDATA_SCALE_AND_BIAS_MULT /
		           (double) 1.0) + 0.5);

      fparams[ifield].factor = RDATA_SCALE_AND_BIAS_MULT;
      fparams[ifield].bias =
	     (si32) (((double) lass_param_p->biasFactor[field_num] * 
		 (double) RDATA_SCALE_AND_BIAS_MULT /
		 (double) 1.0) + 0.5);

    } /* ifield */
  
    break;


  case CHILL_HEADER:

    ch_params = (chill_params_t *) beam_buffer;

    rparams.gate_spacing = (si32) (ch_params->gate_spacing * 1000000.0 + 0.5);
    rparams.start_range = (si32) (ch_params->start_range * 1000000.0 + 0.5);
    rparams.samples_per_beam = ch_params->chldat1.hits;
    rparams.pulse_width =
      (si32) ((ch_params->pulse_len / 1024.0) * 1000.0 + 0.5);
    rparams.prf = (si32) (ch_params->prf * 1000.0 + 0.5);
    rparams.wavelength = (si32) (ch_params->wavelength * 10000.0 + 0.5);
    rparams.scan_mode = ch_params->scan_mode;
    rparams.scan_type = ch_params->scan_type;

    rparams.field_flag = ch_params->field_flag;
    rparams.nfields_current = ch_params->nfields_current;
    
    memcpy ((void *) fparams,
            (void *) ch_params->fparams,
            (size_t) (Glob->nfields_out *
		 sizeof(gate_data_field_params_t)));

    break;

  } /* switch */

  /*
   * check to see if params have changed
   */

  params_have_changed = FALSE;

  if (memcmp((void *) &rparams, (void *) &prev_rparams,
	     (size_t) sizeof(gate_data_radar_params_t))) {
    params_have_changed = TRUE;
  }

  if (memcmp((void *) fparams, (void *) prev_fparams,
	     (size_t) nbytes_fparams)) {
    params_have_changed = TRUE;
  }

  if (params_have_changed) {

    /*
     * set local pointers
     */
    
    pkt_rparams = (gate_data_radar_params_t *) gate_params_pkt;
    pkt_fparams = (gate_data_field_params_t *)
      (gate_params_pkt + sizeof(gate_data_radar_params_t));

    /*
     * copy params to pkt areas
     */

    memcpy ((void *) pkt_rparams,
            (void *) &rparams,
            (size_t) sizeof(gate_data_radar_params_t));

    memcpy ((void *) pkt_fparams,
            (void *) fparams,
            (size_t) nbytes_fparams);

    /*
     * store in prev locations for later comparison
     */

    memcpy ((void *) &prev_rparams,
            (void *) &rparams,
            (size_t) sizeof(gate_data_radar_params_t));

    memcpy ((void *) prev_fparams,
            (void *) fparams,
            (size_t) nbytes_fparams);

    if (Glob->debug)
      printf("params have changed\n");

    /*
     * code the params into network byte order
     */
    
    BE_from_array_32((ui32 *) gate_params_pkt,
		     (ui32) nbytes_params_pkt);
    
  } /* if (params_have_changed) */
  
  first_call = FALSE;

  return (params_have_changed);

}
