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
 * send_params.c
 *
 * Sends a parameter packet to the client
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 * Orig code by Mark Shorman
 *
 * Aug 1996
 *
 * returns 0 on success, -1 on failure.
 *
 ************************************************************************/

#include "bprp2gate.h"

int send_params(int client_fd, int radar_id, bprp_beam_t *beam)

{

  int bin_width, skip;
  double gate_spacing;
  double start_range;
  ui08 buffer[sizeof(gate_data_radar_params_t) + 
		sizeof(gate_data_field_params_t)];

  gate_data_radar_params_t *radar_params;
  gate_data_field_params_t *field_params;

  if (Glob->params.pacer) {
    skip = ((beam->hdr.site_blk) & 0x0ff) / 8;
  } else {
    skip = ((beam->hdr.site_blk) & 0x0ff);
  }

  bin_width = ((beam->hdr.site_blk)/256) & 0x0f;
  gate_spacing = (double) bin_width * KM_PER_US;
  start_range = (double) skip * KM_PER_US + gate_spacing / 2.0;
  
  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "siteblk, width, skip, spacing, start_range: "
	    "%d, %d, %d, %g, %g\n",
	    beam->hdr.site_blk, bin_width, skip, gate_spacing, start_range);
  }
  
  radar_params = (gate_data_radar_params_t *) buffer;
  field_params = (gate_data_field_params_t *) (buffer + sizeof(*radar_params));
  
  radar_params->radar_id =
    BE_from_si32((si32) radar_id);
  radar_params->altitude =
    BE_from_si32((si32) floor(Glob->params.radar_altitude * 1000.0 + 0.5));
  radar_params->latitude =
    BE_from_si32((si32) floor(Glob->params.radar_latitude * 1000000.0 + 0.5));
  radar_params->longitude =
    BE_from_si32((si32) floor(Glob->params.radar_longitude * 1000000.0 + 0.5));
  radar_params->ngates = BE_from_si32(BPRP_GATES_PER_BEAM);
  radar_params->gate_spacing =
    BE_from_si32((si32) floor(gate_spacing * 1000000.0 + 0.5));
  radar_params->start_range =
    BE_from_si32((si32) floor(start_range * 1000000.0 + 0.5));
  radar_params->beam_width =
    BE_from_si32((si32) floor(Glob->params.beam_width * 1000000.0 + 0.5));
  radar_params->samples_per_beam =
    BE_from_si32(Glob->params.samples_per_beam);
  radar_params->pulse_width =
    BE_from_si32((si32) floor(Glob->params.pulse_width * 1000.0 + 0.5));
  radar_params->prf =
    BE_from_si32((si32) floor(Glob->params.prf * 1000.0 + 0.5));
  radar_params->wavelength =
    BE_from_si32((si32) floor(Glob->params.wavelength * 10000.0 + 0.5));
  radar_params->nfields = BE_from_si32(1L);
  radar_params->scan_type = BE_from_si32(0L);
  radar_params->scan_mode = BE_from_si32((si32) GATE_DATA_SURVEILLANCE_MODE);
  radar_params->data_field_by_field = BE_from_si32(1L);
  radar_params->nfields_current = BE_from_si32(1L);
  radar_params->field_flag = BE_from_si32(1L);
  
  field_params->factor = BE_from_si32(DBZ_FACTOR);
  field_params->scale = BE_from_si32((si32) (DBZ_SCALE * DBZ_FACTOR + 0.5));
  field_params->bias = BE_from_si32((si32) (DBZ_BIAS * DBZ_FACTOR + 0.5));
  
  if (client_fd >= 0) {
    if (Glob->params.debug) {
      fprintf(stderr, "Writing params packet\n");
    }
    if (SKU_write_message(client_fd, GATE_PARAMS_PACKET_CODE,
			  (char *) buffer, sizeof(buffer)) < 0) {
      if (Glob->params.debug) {
	fprintf(stderr, "WARNING - %s:send_params\n", Glob->prog_name);
	fprintf(stderr, "Params write to client failed\n");
      }
      return (-1);
    } else {
      return (0);
    }
  }
  
  return (0);

}

