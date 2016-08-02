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
/*****************************************************************************
 * setup_shmem.c
 *
 * set up the shared memory header and buffer, and load in the
 * relevant parameters
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 ****************************************************************************/

#include "polar_ingest.h"

void setup_shmem(void)

{

  si32 field_params_size;
  si32 beam_header_size;
  si32 beam_data_size;
  si32 ifield, ibeam;

  char *shmem_buffer;
  ui08 *data;

  field_params_t *fparams;
  rdata_shmem_header_t *shdr;
  radar_params_t *rparams;
  ll_params_t *ll_params;
  rp7_params_t *rp7_params;
  bprp_params_t *bprp_params;
  bprp_data_t *bprp_data;
  gate_data_radar_params_t *gate_rparams;
  gate_data_field_params_t *gate_fparams;
  gate_data_beam_header_t *gate_hdr;

  /*
   * attach shared memory for the header
   */
  
  if ((shdr = (rdata_shmem_header_t *)
       ushm_create(Glob->shmem_key + 1, sizeof(rdata_shmem_header_t),
		   S_PERMISSIONS)) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr, "Cannot attach shared memory for header, key = %x\n",
	    Glob->shmem_key + 1);
    tidy_and_exit(-1);
  }

  rparams = &shdr->radar;
  Glob->shmem_header = shdr;
  Glob->shmem_header_attached = TRUE;

  /*
   * set the relevant parts of the shared mem header - the remainder
   * will be set depending on the type of header
   */

  memset ((void *)  shdr,
          (int) 0, (size_t) sizeof(rdata_shmem_header_t));

  shdr->nfields_in = Glob->nfields_in;
  shdr->ngates = Glob->ngates_out;
  shdr->nbeams_buffer = Glob->nbeams_buffer;
  shdr->late_end_of_vol = FALSE;

  /*
   * ensure double alignment
   */

  field_params_size =
    ((sizeof(field_params_t) / sizeof(double) + 1) * sizeof(double));

  field_params_size *= Glob->nfields_in;

  beam_header_size =
    ((sizeof(rdata_shmem_beam_header_t) /
      sizeof(double) + 1) * sizeof(double));

  beam_header_size *= Glob->nbeams_buffer;

  beam_data_size = (Glob->nbeams_buffer * Glob->nfields_in *
		    Glob->ngates_out * sizeof(ui08));

  shdr->buffer_size = 
    field_params_size + beam_header_size + beam_data_size;

  shdr->field_params_offset = 0;
  shdr->beam_headers_offset = shdr->field_params_offset + field_params_size;

  shdr->dbz_field_pos = Glob->dbz_field_pos;

  shdr->radar.radar_id = Glob->radar_id;
  
  strcpy(shdr->radar.name, Glob->radar_name);

  shdr->noise_dbz_at_100km = Glob->noise_dbz_at_100km;

  strcpy(shdr->note, Glob->note);

  /*
   * attach shmem buffer
   */
  
  if ((shmem_buffer = (char *)
       ushm_create(Glob->shmem_key + 2,
		   (int) shdr->buffer_size, S_PERMISSIONS)) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_shmem.\n", Glob->prog_name);
    fprintf(stderr, "Cannot attach shared memory for buffer, key = %x\n",
	    Glob->shmem_key + 2);
    tidy_and_exit(-1);
  }
  
  Glob->shmem_buffer = shmem_buffer;
  Glob->shmem_buffer_attached = TRUE;

  memset ((void *)  shmem_buffer,
          (int) 0, (size_t) shdr->buffer_size);

  Glob->beam_headers = (rdata_shmem_beam_header_t *)
    (shmem_buffer + shdr->beam_headers_offset);

  /*
   * copy in the relevant parts of the field params
   */

  fparams = (field_params_t *)
    (shmem_buffer + shdr->field_params_offset);

  for (ifield = 0; ifield < Glob->nfields_in; ifield++) {

    strncpy (fparams[ifield].name,
	     Glob->field_name[ifield],
	     (size_t) (R_LABEL_LEN - 1));

    fparams[ifield].name[R_LABEL_LEN - 1] = '\0';

    strncpy (fparams[ifield].units,
	     Glob->field_units[ifield],
	     (size_t) (R_LABEL_LEN - 1));

    fparams[ifield].units[R_LABEL_LEN - 1] = '\0';

    strncpy (fparams[ifield].transform,
	     Glob->field_transform[ifield],
	     (size_t) (R_LABEL_LEN - 1));

    fparams[ifield].transform[R_LABEL_LEN - 1] = '\0';
    
  } /* ifield */

  /*
   * load up shmem buffer offsets in the beam headers
   */

  for (ibeam = 0; ibeam < Glob->nbeams_buffer; ibeam++) {

    Glob->beam_headers[ibeam].field_data_offset = 
      (shdr->beam_headers_offset +
       Glob->nbeams_buffer * sizeof(rdata_shmem_beam_header_t) +
       ibeam * Glob->nfields_in * Glob->ngates_out);

  } /* ibeam */

  /*
   * switch on the header type, and set the relevant
   * portions of the shared memory area which are dependent
   * on the incoming data
   */

  switch (Glob->header_type) {

  case LINCOLN_HEADER:

    get_lincoln_ptrs(&ll_params, &data);

    Glob->data_field_by_field = FALSE;

    rparams->altitude = ll_params->altitude;

    rparams->latitude = (si32)
      ((double) Glob->ns_factor *
       fabs(((double) (ll_params->latitude) * 10.0) + 0.5));

    rparams->longitude = (si32)
      ((double) Glob->ew_factor *
       fabs(((double) (ll_params->longitude) * 10.0) + 0.5));
  
    rparams->gate_spacing = ll_params->range_seg[0].gate_spacing * 1000;

    rparams->start_range =
      (si32) floor(((double) ll_params->range_to_first_gate +
		    ((double) ll_params->range_seg[0].gate_spacing / 2.0)) *
		   1000.0 + 0.5);

    rparams->beam_width = ll_params->beamwidth * 10000;
    rparams->samples_per_beam = Glob->samples_per_beam;
    rparams->pulse_width = ll_params->pulse_width;
    rparams->prf = ll_params->prf * 1000;
    rparams->wavelength = (si32)
      (3.0e8 / (double) ll_params->frequency + 0.5);

    if (Glob->nfields_in != LL_NFIELDS) {
	
      fprintf(stderr, "ERROR - %s:setup_shmem\n", Glob->prog_name);
      fprintf(stderr,
	      "Number of input fields requested - %ld - incorrect.\n",
	      (long) Glob->nfields_in);
      fprintf(stderr, "Number of fields for Lincoln format is %d.\n",
	      LL_NFIELDS);
      tidy_and_exit(-1);
      
    } /* if (Glob->nfields_in > ... */

    for (ifield = 0; ifield < Glob->nfields_in; ifield++) {

      fparams[ifield].scale =
	(si32) (((double) ll_params->scale[ifield] * 
		 (double) RDATA_SCALE_AND_BIAS_MULT /
		 (double) LL_SCALE_AND_BIAS_MULT) + 0.5);

      fparams[ifield].bias =
	(si32) (((double) ll_params->bias[ifield] * 
		 (double) RDATA_SCALE_AND_BIAS_MULT /
		 (double) LL_SCALE_AND_BIAS_MULT) + 0.5);

      fparams[ifield].factor = RDATA_SCALE_AND_BIAS_MULT;

    } /* ifield */
  
    break;

  case RP7_HEADER:

    get_rp7_ptrs(&rp7_params, &data);

    Glob->data_field_by_field = FALSE;

    rparams->altitude = rp7_params->altitude;
    
    rparams->latitude = (si32)
      ((double) Glob->ns_factor *
       fabs(((double) (rp7_params->latitude) *
	     (90.0 / 65536.0) * 1000000.0) + 0.5));
    
    rparams->longitude = (si32)
      ((double) Glob->ew_factor *
       fabs(((double) (rp7_params->longitude) *
	     (180.0 / 65536.0) * 1000000.0) + 0.5));
    
    rparams->gate_spacing = rp7_params->gate_spacing * 1000;
    
    rparams->start_range =
      (si32) floor(((double) rp7_params->rhozero1 +
		    (double) rp7_params->rhozero2 +
		    ((double) rp7_params->gate_spacing / 2.0))
		   * 1000.0 + 0.5);
    
    rparams->beam_width = rp7_params->beamwidth * 10000;
    rparams->samples_per_beam = rp7_params->samples_per_beam;
    rparams->pulse_width = rp7_params->pulse_width;
    rparams->prf = rp7_params->prfx10 * 100;
    rparams->wavelength = rp7_params->wavelength * 100;
    
    if (Glob->nfields_in != rp7_params->nfields) {
	
      fprintf(stderr, "ERROR - %s:setup_shmem\n", Glob->prog_name);
      fprintf(stderr,
	      "Number of input fields requested - %ld - incorrect.\n",
	      (long) Glob->nfields_in);
      fprintf(stderr, "Number of fields in data is %d.\n",
	      rp7_params->nfields);
      /* tidy_and_exit(-1); */
      
    } /* if (Glob->nfields_in > ... */

    for (ifield = 0; ifield < Glob->nfields_in; ifield++) {

      fparams[ifield].scale =
	(si32) (((double) rp7_params->lscale[ifield].scale *
		 (double) RDATA_SCALE_AND_BIAS_MULT /
		 (double) LL_SCALE_AND_BIAS_MULT) + 0.5);

      fparams[ifield].bias =
	(si32) (((double) rp7_params->lscale[ifield].bias *
		 (double) RDATA_SCALE_AND_BIAS_MULT /
		 (double) LL_SCALE_AND_BIAS_MULT) + 0.5);

      fparams[ifield].factor = RDATA_SCALE_AND_BIAS_MULT;

    } /* ifield */
  
    break;

  case BPRP_HEADER:

    get_bprp_ptrs(&bprp_params, &bprp_data);
  
    Glob->data_field_by_field = FALSE;

    rparams->altitude = (si32) (BPRP_ALTITUDE * 1000.0 + 0.5);
    rparams->latitude = (si32) floor (BPRP_LATITUDE * 1000000.0 + 0.5);
    rparams->longitude = (si32) floor (BPRP_LONGITUDE *1000000.0 + 0.5);
    rparams->gate_spacing = (si32) (BPRP_GATE_SPACING * 1000000.0 + 0.5); 
    rparams->start_range =
      (si32) ((BPRP_START_RANGE +
	       BPRP_GATE_SPACING / 2.0)* 1000000.0 + 0.5);
    rparams->beam_width = (si32) (BPRP_BEAM_WIDTH * 1000000.0 + 0.5);
    rparams->samples_per_beam = BPRP_SAMPLES_PER_BEAM;            
    rparams->pulse_width = (si32) (BPRP_PULSE_WIDTH * 1000.0 + 0.5);           
    rparams->prf = (si32) (BPRP_PRF_NOMINAL * 1000.0 + 0.5);
    rparams->wavelength = (si32) (BPRP_WAVELENGTH * 10000.0 + 0.5);
    
    if (Glob->nfields_in != BPRP_MAX_NFIELDS) {
	
      fprintf(stderr, "ERROR - %s:setup_shmem\n", Glob->prog_name);
      fprintf(stderr,
	      "Number of input fields requested - %ld - incorrect.\n",
	      (long) Glob->nfields_in);
      fprintf(stderr, "Number of fields in data is %d.\n",
	      BPRP_MAX_NFIELDS);
      tidy_and_exit(-1);
      
    } /* if (Glob->nfields_in > ... */

    for (ifield = 0; ifield < Glob->nfields_in; ifield++) {

      fparams[ifield].scale =
	(si32) ((double) bprp_params->lscale[ifield].scale *
		(double) RDATA_SCALE_AND_BIAS_MULT + 0.5);

      fparams[ifield].bias =
	(si32) ((double) bprp_params->lscale[ifield].bias *
		(double) RDATA_SCALE_AND_BIAS_MULT + 0.5);
      
      fparams[ifield].factor = RDATA_SCALE_AND_BIAS_MULT;

    } /* ifield */
  
    break;

  case GATE_DATA_HEADER:

    get_gate_data_ptrs(&gate_rparams, &gate_fparams,
		       &gate_hdr, &data);
    
    Glob->data_field_by_field = gate_rparams->data_field_by_field;

    rparams->altitude = gate_rparams->altitude;
    rparams->latitude = gate_rparams->latitude;
    rparams->longitude = gate_rparams->longitude;
    rparams->gate_spacing = gate_rparams->gate_spacing;
    rparams->start_range = gate_rparams->start_range;
    rparams->beam_width = gate_rparams->beam_width;
    rparams->samples_per_beam = gate_rparams->samples_per_beam;
    rparams->pulse_width = gate_rparams->pulse_width;
    rparams->prf = gate_rparams->prf;
    rparams->wavelength = gate_rparams->wavelength;
    
    if (Glob->nfields_in != gate_rparams->nfields) {
	
      fprintf(stderr, "ERROR - %s:setup_shmem\n", Glob->prog_name);
      fprintf(stderr,
	      "Number of input fields requested - %ld - incorrect.\n",
	      (long) Glob->nfields_in);
      fprintf(stderr, "Number of fields in data is %d.\n",
	      gate_rparams->nfields);
      /* tidy_and_exit(-1); */
      
    } /* if (Glob->nfields_in > ... */

    for (ifield = 0; ifield < Glob->nfields_in; ifield++) {

      fparams[ifield].scale = gate_fparams[ifield].scale;
      fparams[ifield].bias = gate_fparams[ifield].bias;
      fparams[ifield].factor = gate_fparams[ifield].factor;

    } /* ifield */
  
    break;

  } /* switch */

  shdr->data_field_by_field = Glob->data_field_by_field;

  return;

}
