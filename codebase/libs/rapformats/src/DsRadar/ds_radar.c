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
/*********************************************
 * ds_radar.c
 *
 * C routines for DsRadar structs
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * May 1998
 */

#include <dataport/bigend.h>
#include <rapformats/ds_radar.h>
#include <toolsa/umisc.h>
#include <toolsa/mem.h>
#include <assert.h>

/**********************
 * BE swapping routines
 */

/***********************
 * BE_to_DsRadarParams()
 *
 * Convert BE to DsRadarParams_t
 */

void BE_to_DsRadarParams(DsRadarParams_t *params)
     
{
  BE_to_array_32(params, sizeof(DsRadarParams_t) - NCHAR_DS_RADAR_PARAMS);
}

/*************************
 * BE_from_DsRadarParams()
 *
 * Convert DsRadarParams_t to BE
 */

void BE_from_DsRadarParams(DsRadarParams_t *params)

{
  BE_from_array_32(params, sizeof(DsRadarParams_t) - NCHAR_DS_RADAR_PARAMS);
}

/***********************
 * BE_to_DsFieldParams()
 *
 * Convert BE to DsFieldParams_t
 */

void BE_to_DsFieldParams(DsFieldParams_t *field)
     
{
  BE_to_array_32(field, sizeof(DsFieldParams_t) - NCHAR_DS_FIELD_PARAMS);
}

/*************************
 * BE_from_DsFieldParams()
 *
 * Convert DsFieldParams_t to BE
 */

void BE_from_DsFieldParams(DsFieldParams_t *field)

{
  BE_from_array_32(field, sizeof(DsFieldParams_t) - NCHAR_DS_FIELD_PARAMS);
}

/*******************
 * BE_to_DsBeamHdr()
 *
 * Convert BE to DsBeamHdr_t
 */

void BE_to_DsBeamHdr(DsBeamHdr_t *beam)

{
  BE_to_array_32(beam, sizeof(DsBeamHdr_t));
}

/***************************
 * BE_from_DsBeamHdr()
 *
 * Convert DsBeamHdr_t to BE
 */

void BE_from_DsBeamHdr(DsBeamHdr_t *beam)

{
  BE_from_array_32(beam, sizeof(DsBeamHdr_t));
}

/***********************
 * BE_to_DsRadarFlags()
 *
 * Convert BE to DsRadarFlags_t
 */

void BE_to_DsRadarFlags(DsRadarFlags_t *flags)
     
{
  BE_to_array_32(flags, sizeof(DsRadarFlags_t));
}

/*************************
 * BE_from_DsRadarFlags()
 *
 * Convert DsRadarFlags_t to BE
 */

void BE_from_DsRadarFlags(DsRadarFlags_t *flags)

{
  BE_from_array_32(flags, sizeof(DsRadarFlags_t));
}

/*********************************
 * radar elevations array routines
 */

/********************
 * DsRadarElev_init()
 *
 * Initialize radar elevation struct
 */

void DsRadarElev_init(DsRadarElev_t *elev)
{
  elev->n_elev = 0;
  elev->elev_array = NULL;
  elev->chunk_len = 0;
  elev->chunk_buf = NULL;
  elev->init_flag = DSRADAR_ELEV_INIT;
}

/*********************
 * DsRadarElev_alloc()
 *
 * Alloc arrays for radar elevation struct
 */

void DsRadarElev_alloc(DsRadarElev_t *elev, int nelev)
{
  assert(elev->init_flag == DSRADAR_ELEV_INIT);
  if (nelev != elev->n_elev) {
    elev->elev_array = (fl32 *) urealloc(elev->elev_array,
					 nelev * sizeof(fl32));
    elev->n_elev = nelev;
    elev->chunk_len = sizeof(si32) + nelev * sizeof(fl32);
    elev->chunk_buf = (ui08 *) urealloc(elev->chunk_buf,
					elev->chunk_len);
  }
}

/********************
 * DsRadarElev_free()
 *
 * Free arrays for radar elevation struct
 */

void DsRadarElev_free(DsRadarElev_t *elev)
{
  assert(elev->init_flag == DSRADAR_ELEV_INIT);
  if (elev->elev_array) {
    ufree(elev->elev_array);
    elev->elev_array = NULL;
  }
  elev->n_elev = 0;
  if (elev->chunk_buf) {
    ufree(elev->chunk_buf);
    elev->chunk_buf = NULL;
  }
  elev->chunk_len = 0;
}

/****************************
 * DsRadarElev_load_chunk()
 *
 * Load up chunk data
 */

void DsRadarElev_load_chunk(DsRadarElev_t *elev)
{
  memcpy(elev->chunk_buf, &elev->n_elev, sizeof(si32));
  memcpy(elev->chunk_buf + sizeof(si32), elev->elev_array,
	 elev->n_elev * sizeof(fl32));
}

/****************************
 * DsRadarElev_unload_chunk()
 *
 * Unload chunk data into struct
 */

void DsRadarElev_unload_chunk(DsRadarElev_t *elev,
			      ui08 *chunk, int chunk_len)
{
  int nelev;
  nelev = (chunk_len / sizeof(si32)) - 1;
  DsRadarElev_alloc(elev, nelev);
  memcpy(elev->chunk_buf, chunk, elev->chunk_len);
  memcpy(&elev->n_elev, elev->chunk_buf, sizeof(si32));
  memcpy(elev->elev_array, elev->chunk_buf + sizeof(si32),
	 elev->n_elev * sizeof(fl32));
}

/****************
 * print routines
 */


void DsRadarParams_print(FILE *out, char *spacer,
			 DsRadarParams_t *rparams)

{

  fprintf(out, "%s DsRadarParams\n", spacer);
  fprintf(out, "%s -------------\n", spacer);

  fprintf(out, "%s radar_id: %d\n", spacer, (int) rparams->radar_id);
  fprintf(out, "%s radar_type: %d\n", spacer, (int) rparams->radar_type);
  fprintf(out, "%s nfields: %d\n", spacer, (int) rparams->nfields);
  fprintf(out, "%s ngates: %d\n", spacer, (int) rparams->ngates);
  fprintf(out, "%s samples_per_beam: %d\n", spacer,
	  (int) rparams->samples_per_beam);
  fprintf(out, "%s scan_type: %d\n", spacer, (int) rparams->scan_type);
  fprintf(out, "%s scan_mode: %d\n", spacer, (int) rparams->scan_mode);
  fprintf(out, "%s follow_mode: %d\n", spacer, (int) rparams->follow_mode);
  fprintf(out, "%s nfields_current: %d\n", spacer,
	  (int) rparams->nfields_current);
  fprintf(out, "%s field_flag: %d\n", spacer, (int) rparams->field_flag);
  fprintf(out, "%s polarization: %d\n", spacer, (int) rparams->polarization);
  fprintf(out, "%s prf mode %d\n", spacer, (int) rparams->prf_mode);

  fprintf(out, "%s radar_constant: %g\n", spacer, rparams->radar_constant);
  fprintf(out, "%s altitude: %g\n", spacer, rparams->altitude);
  fprintf(out, "%s latitude: %g\n", spacer, rparams->latitude);
  fprintf(out, "%s longitude: %g\n", spacer, rparams->longitude);
  fprintf(out, "%s gate_spacing: %g\n", spacer, rparams->gate_spacing);
  fprintf(out, "%s start_range: %g\n", spacer, rparams->start_range);
  fprintf(out, "%s horiz_beam_width: %g\n", spacer, rparams->horiz_beam_width);
  fprintf(out, "%s vert_beam_width: %g\n", spacer, rparams->vert_beam_width);
  fprintf(out, "%s pulse_width: %g\n", spacer, rparams->pulse_width);
  fprintf(out, "%s prf: %g\n", spacer, rparams->prf);
  fprintf(out, "%s prt: %g\n", spacer, rparams->prt);
  fprintf(out, "%s prt2: %g\n", spacer, rparams->prt2);
  fprintf(out, "%s wavelength: %g\n", spacer, rparams->wavelength);
  fprintf(out, "%s xmit_peak_pwr: %g\n", spacer, rparams->xmit_peak_pwr);
  fprintf(out, "%s receiver_mds: %g\n", spacer, rparams->receiver_mds);
  fprintf(out, "%s receiver_gain: %g\n", spacer, rparams->receiver_gain);
  fprintf(out, "%s antenna_gain: %g\n", spacer, rparams->antenna_gain);
  fprintf(out, "%s system_gain: %g\n", spacer, rparams->system_gain);
  fprintf(out, "%s unambig_vel: %g\n", spacer, rparams->unambig_vel);
  fprintf(out, "%s unambig_range: %g\n", spacer, rparams->unambig_range);
   
  fprintf(out, "%s radar_name: %s\n", spacer, rparams->radar_name);
  fprintf(out, "%s scan_type_name: %s\n", spacer, rparams->scan_type_name);
  
}

void DsFieldParams_print(FILE *out, char *spacer,
			 DsFieldParams_t *fparams)

{


  fprintf(out, "%s DsFieldParams\n", spacer);
  fprintf(out, "%s -------------\n", spacer);

  fprintf(out, "%s byte_width: %d\n", spacer,
	  (int) fparams->byte_width);
  fprintf(out, "%s missing_data_value: %d\n", spacer,
	  (int) fparams->missing_data_value);
   
  fprintf(out, "%s scale: %g\n", spacer, fparams->scale);
  fprintf(out, "%s bias: %g\n", spacer, fparams->bias);

  fprintf(out, "%s name: %s\n", spacer, fparams->name);
  fprintf(out, "%s units: %s\n", spacer, fparams->units);

}

void DsBeamHdr_print(FILE *out, char *spacer,
		     DsBeamHdr_t *bhdr)

{

  fprintf(out, "%s DsBeamHdr\n", spacer);
  fprintf(out, "%s ---------\n", spacer);
  
  fprintf(out, "%s time: %s\n", spacer, utimstr(bhdr->time));
  fprintf(out, "%s nano_secs: %d\n", spacer, (int) bhdr->nano_secs);
  fprintf(out, "%s reference time: %s\n", spacer, 
          utimstr(bhdr->reference_time));

  fprintf(out, "%s vol_num: %d\n", spacer, (int) bhdr->vol_num);
  fprintf(out, "%s tilt_num: %d\n", spacer, (int) bhdr->tilt_num);
  fprintf(out, "%s byte_width: %d\n", spacer, (int) bhdr->byte_width);
  fprintf(out, "%s scan_mode: %d\n", spacer, (int) bhdr->scan_mode);
  fprintf(out, "%s beam_is_indexed: %d\n", spacer,
          (int) bhdr->beam_is_indexed);
  fprintf(out, "%s antenna_transition: %d\n", spacer,
          (int) bhdr->antenna_transition);
  fprintf(out, "%s n_samples: %d\n", spacer,
          (int) bhdr->n_samples);
  fprintf(out, "%s azimuth: %g\n", spacer, bhdr->azimuth);
  fprintf(out, "%s elevation: %g\n", spacer, bhdr->elevation);
  fprintf(out, "%s target_elev: %g\n", spacer, bhdr->target_elev);
  fprintf(out, "%s target_az: %g\n", spacer, bhdr->target_az);
  fprintf(out, "%s angular_resolution: %g\n", spacer,
          bhdr->angular_resolution);

}

void DsRadarFlags_print(FILE *out, char *spacer,
			DsRadarFlags_t *flags)

{
  
  fprintf(out, "\n");
  fprintf(out, "%s DsRadarFlags\n", spacer);
  fprintf(out, "%s ------------\n", spacer);
  
  fprintf(out, "%s time: %s\n", spacer, utimstr(flags->time));

  fprintf(out, "%s vol_num: %d\n", spacer, (int) flags->vol_num);
  fprintf(out, "%s tilt_num: %d\n", spacer, (int) flags->tilt_num);
  fprintf(out, "%s scan_type: %d\n", spacer, (int) flags->scan_type);
  
  fprintf(out, "%s start_of_tilt: %d\n", spacer,
	  (int) flags->start_of_tilt);
  fprintf(out, "%s end_of_tilt: %d\n", spacer,
	  (int) flags->end_of_tilt);

  fprintf(out, "%s start_of_volume: %d\n", spacer,
	  (int) flags->start_of_volume);
  fprintf(out, "%s end_of_volume: %d\n", spacer,
	  (int) flags->end_of_volume);

  fprintf(out, "%s new_scan_type: %d\n", spacer,
	  (int) flags->new_scan_type);

}

void DsRadarElev_print(FILE *out, char *spacer,
		       DsRadarElev_t *elev)
{

  int i;

  assert(elev->init_flag == DSRADAR_ELEV_INIT);

  fprintf(out, "%s DsRadarElevs\n", spacer);
  fprintf(out, "%s ------------\n", spacer);
  
  for (i = 0; i < elev->n_elev; i++) {
    fprintf(out, "%s Elev[%d]: %g\n", spacer, i, elev->elev_array[i]);
  }

}


