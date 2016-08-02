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
/***********************************************************************
 * output.c
 *
 * Composes output for sending to the client
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 * Orig code by Mark Shorman
 *
 * Aug 1996
 *
 ************************************************************************/

#include "rdacs2gate.h"
#include <assert.h>
#include <time.h>

#define SCALE_FACTOR 10000.0

static int InitDone = FALSE;

static ui08 *RayBuf = NULL;
static int RayBufAlloc = 0;
static int RayBufSize;
static char *ParamBuf = NULL;
static int ParamBufSize = 0;
static gate_data_radar_params_t *Radar_params;
static gate_data_field_params_t *Field_params;
static int GotRayParams = FALSE;
static int GotSiteParams = FALSE;
static int Ngates;
static int Nfields;

static int VolNum = 0;
static int TiltNum = 0;
static int PrevTiltNum = 0;
static int OnLastEl = FALSE;
static int EndOfVol = FALSE;
static int EndOfTilt = FALSE;
static double TargetEl;

static void check_buffer_alloc(void);

static int get_tilt_num(double elevation);

void init_param_buffer(void)

{

  int i;
  int field_flag;
  
  assert (!InitDone);

  ParamBufSize = sizeof(gate_data_radar_params_t) +
    Glob->params.fields.len * sizeof(gate_data_field_params_t);

  ParamBuf = (char *) umalloc(ParamBufSize);

  Radar_params = (gate_data_radar_params_t *) ParamBuf;

  Field_params = (gate_data_field_params_t *)
    (ParamBuf + sizeof(gate_data_radar_params_t));

  Nfields = Glob->params.fields.len;

  for (i = 0; i < Nfields; i++) {
    Field_params[i].factor = SCALE_FACTOR;
    Field_params[i].scale =
      (int) (Glob->params.fields.val[i].scale * SCALE_FACTOR + 0.5);
    Field_params[i].bias =
      (int) (Glob->params.fields.val[i].bias * SCALE_FACTOR + 0.5);
    Field_params[i].factor = BE_from_si32(Field_params[i].factor);
    Field_params[i].scale = BE_from_si32(Field_params[i].scale);
    Field_params[i].bias = BE_from_si32(Field_params[i].bias);
  }

  Radar_params->radar_id =
    BE_from_si32((si32) Glob->params.radar_id);
  Radar_params->beam_width =
    BE_from_si32((si32) floor(Glob->params.beam_width * 1000000.0 + 0.5));
  Radar_params->samples_per_beam =
    BE_from_si32(Glob->params.samples_per_beam);
  Radar_params->pulse_width =
    BE_from_si32((si32) floor(Glob->params.pulse_width * 1000.0 + 0.5));
  Radar_params->prf =
    BE_from_si32((si32) floor(Glob->params.prf * 1000.0 + 0.5));
  Radar_params->wavelength =
    BE_from_si32((si32) floor(Glob->params.wavelength * 10000.0 + 0.5));

  Radar_params->nfields = BE_from_si32(Nfields);
  Radar_params->scan_type = BE_from_si32(0L);
  Radar_params->scan_mode = BE_from_si32((si32) GATE_DATA_SURVEILLANCE_MODE);
  Radar_params->data_field_by_field = BE_from_si32(1L);
  Radar_params->nfields_current = BE_from_si32(Nfields);

  field_flag = 0;
  for (i = 0; i < Nfields; i++) {
    field_flag |= 1 << i;
  }
  Radar_params->field_flag = BE_from_si32(field_flag);

  InitDone = TRUE;

}

void store_ray_params(RDP_GetRayParms_R *rparams)

{

  int ngates;
  double start_range, gate_spacing;

  assert(InitDone);

  ngates = rparams->NumberBins;
  gate_spacing = rparams->BinTime * 1.5e-4;
  start_range = (rparams->SkipTime * 1.5e-4) + (gate_spacing / 2.0);

  Ngates = ngates;
  Radar_params->ngates = BE_from_si32(ngates);
  Radar_params->gate_spacing =
    BE_from_si32((si32) floor(gate_spacing * 1000000.0 + 0.5));
  Radar_params->start_range =
    BE_from_si32((si32) floor(start_range * 1000000.0 + 0.5));

  GotRayParams = TRUE;

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "***** store_ray_params *****\n");
    fprintf(stderr, "  ngates: %d\n", ngates);
    fprintf(stderr, "  start_range: %g\n", start_range);
    fprintf(stderr, "  gate_spacing: %g\n", gate_spacing);
  }

}

void store_site_params(SITECFG *site)

{

  double lat, lon, alt;

  assert(InitDone);

  lat = site->Latitude / 1000000.0;  /* deg */
  lon = site->Longitude / 1000000.0; /* deg */
  alt = site->Altitude_ASL / 1000.0; /* km */

  Radar_params->altitude =
    BE_from_si32((si32) floor(alt * 1000.0 + 0.5));
  Radar_params->latitude =
    BE_from_si32((si32) floor(lat * 1000000.0 + 0.5));
  Radar_params->longitude =
    BE_from_si32((si32) floor(lon * 1000000.0 + 0.5));

  GotSiteParams = TRUE;

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "***** store_site_params *****\n");
    fprintf(stderr, "  alt: %g\n", alt);
    fprintf(stderr, "  lat: %g\n", lat);
    fprintf(stderr, "  lon: %g\n", lon);
  }

}

void send_params(void)

{

  assert(InitDone);

  if (GotRayParams && GotSiteParams) {
    send_buffer(ParamBuf, ParamBufSize, GATE_PARAMS_PACKET_CODE);
  }

}

/*
 * handle_step()
 *
 * This handles elevation steps - it keeps TiltNum and VolNum
 * up to date. The last_el_in_vol is a parameter which allows
 * this routine to determine the end-of-vol.
 */

void handle_step(RADPGM_STEP *s)

{
  
  EndOfTilt = TRUE;
  TiltNum++;

  if (OnLastEl) {
    EndOfVol = TRUE;
    TiltNum = 0;
    VolNum++;
    OnLastEl = FALSE;
  }

  TargetEl = (s->El / 16384.0) * 360.0;

  if (fabs(TargetEl - Glob->params.last_el_in_vol) < 0.1) {
    OnLastEl = TRUE;
  }

  if (Glob->params.debug) {
    fprintf(stderr, "TiltNum, VolNum, OnLastEl: %d, %d, %d, %g\n",
	    TiltNum, VolNum, OnLastEl, TargetEl);
  }

}

static void check_buffer_alloc(void)

{

  RayBufSize = sizeof(gate_data_beam_header_t) +
    Ngates * Nfields;

  if (RayBufSize > RayBufAlloc) {
    if (RayBuf == NULL) {
      RayBuf = umalloc(RayBufSize);
    } else {
      RayBuf = urealloc(RayBuf, RayBufSize);
    }
    RayBufAlloc = RayBufSize;
  }

}

void load_beam(RDP_HDR *reply_hdr, RDP_GetRayData_R *rayd)

{

  static si32 beam_count = 0;
  static ui16 prev_az = 0;
  static ui16 prev_el = 0;
  static int nfields_ray = 0;
  
  si08 *igate, *in_gates;
  ui08 *ogate, *out_gates;
  ui16 this_az, this_el;
  int i;
  time_t now;
  struct tm *ut;
  si32 beam_time;
  date_time_t dtime;
  double azimuth, elevation;
  gate_data_beam_header_t *beam_header;

  /*
   * send params every 90 beams
   */
  
  beam_count++;
  if (beam_count >= 90) {
    send_params();
    beam_count = 0;
  }

  check_buffer_alloc();

  /*
   * check for change of angle - if it has changed, send
   * out ray
   */
  
  this_az = rayd->Azimuth;
  this_el = rayd->Elevation;

  if ((prev_az != this_az) || (prev_el != this_el)) {
    if (nfields_ray > 0) {
      send_buffer(RayBuf, RayBufSize,
		  GATE_DATA_PACKET_CODE);
    }
    memset(RayBuf, 0, RayBufSize);
    nfields_ray = 0;
  }

  /*
   * ray full ?
   */

  if (nfields_ray >= Nfields) {
    return;
  }

  /*  if (nfields_ray == 0) { */

    now = time(NULL);
    if (Glob->params.time_mode == LOCAL) {
      ut = localtime(&now);
      dtime.year = 1900 + ut->tm_year;
      dtime.month = ut->tm_mon + 1;
      dtime.day = ut->tm_mday;
      dtime.hour = ut->tm_hour;
      dtime.min = ut->tm_min;
      dtime.sec = ut->tm_sec;
      uconvert_to_utime(&dtime);
      beam_time = dtime.unix_time;
    } else {
      beam_time = now;
    }

    azimuth = (rayd->Azimuth / 16384.0) * 360.0;
    elevation = (rayd->Elevation / 16384.0) * 360.0;

    if (Glob->params.use_elev_table) {
      TiltNum = get_tilt_num(elevation);
      if (TiltNum != PrevTiltNum && TiltNum == 0) {
	VolNum++;
      }
      PrevTiltNum = TiltNum;
    }

    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf(stderr, "time, az, el, vol, tilt, nfields: "
	      "%s %7.1f %7.1f %d %d %d\n",
	      utimstr(beam_time),
	      azimuth, elevation, VolNum, TiltNum, nfields_ray);
    }
    
    /*
     * load beam header, swapping as necessary
     */
    
    beam_header = (gate_data_beam_header_t *) RayBuf;
    
    beam_header->time = BE_from_si32(beam_time);
    beam_header->azimuth = BE_from_si32((si32) (azimuth * 1000000.0));
    beam_header->elevation = BE_from_si32((si32) (elevation * 1000000.0));
    beam_header->target_elev = BE_from_si32((si32) (TargetEl * 1000000.0));
    beam_header->vol_num = BE_from_si32(VolNum);
    beam_header->tilt_num = BE_from_si32(TiltNum);
    beam_header->new_scan_limits = BE_from_si32(0L);
    if (Glob->params.use_elev_table) {
      beam_header->end_of_tilt = BE_from_si32(-1);
      beam_header->end_of_volume = BE_from_si32(-1);
    } else {
      beam_header->end_of_tilt = BE_from_si32(EndOfTilt);
      beam_header->end_of_volume = BE_from_si32(EndOfVol);
    }

    /*  } */ /* if (nfields_ray == 0) */

  /*
   * clear the end of event flags
   */
  
  EndOfTilt = FALSE;
  EndOfVol = FALSE;

  /*
   * load output gates
   */
  
  out_gates = (RayBuf + sizeof(gate_data_beam_header_t) +
	       Ngates * nfields_ray);
  in_gates = (si08 *) rayd + rayd->OffsetToData - sizeof(RDP_HDR);
  
  /*
   * copy in_gates to out_gates,
   * changing bytes from signed to unsigned
   */

  igate = in_gates;
  ogate = out_gates;
  for (i = 0; i < Ngates; i++, igate++, ogate++) {
    *ogate = *igate + 127;
  } /* i */

  if (Glob->params.debug >= DEBUG_EXTRA) {
    igate = in_gates;
    fprintf(stderr, "Ngates: %d\n", Ngates);
    for (i = 0; i < Ngates; i++, igate++) {
      fprintf(stderr, "%d ", *igate);
    } 
    fprintf(stderr, "\n");
  }

  nfields_ray++;
  prev_az = this_az;
  prev_el = this_el;

}

static int get_tilt_num(double elevation)

{

  int i;
  si32 tilt_num;
  double min_diff = 1000.0;
  double diff;

  for (i = 0; i < Glob->params.elev_table.len; i++) {

    diff = fabs(Glob->params.elev_table.val[i] - elevation);
    
    if (diff < min_diff) {
      tilt_num = i;
      min_diff = diff;
    }

  } /* i */

  return (tilt_num);

}

