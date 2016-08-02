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
 * send_beam.c
 *
 * Sends a beam packet to the client
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 * Orig code by Mark Shorman
 *
 * Aug 1996
 *
 * returns 0 on success, -1 on failure, 1 if id is incorrect.
 *
 ************************************************************************/

#include "bprp2gate.h"

static double computeDbz(double range,
			 double atten_per_km,
			 double atten_at_100km,
			 double xmt1,
			 double viplo,
			 double plo,
			 double slope,
			 int count,
			 int print);

int send_beam(int client_fd, bprp_beam_t *beam)

{

  static si32 beam_count = 0;
  static si32 volume_no = 0;
  static ui08 buffer [sizeof(gate_data_beam_header_t) + BPRP_GATES_PER_BEAM];
  
  ui08 *gates;
  ui16 raycount;
  
  int i;      
  int shift, mult;
  int radar_id;
  int rc;

  si32 beam_time;
  si32 tilt_no;
  
  double azimuth, elevation;
  
  double slope, viplo, plo, viphi , phi;
  int bin_width, skip;
  double start_range;
  double aperture, radar_constant, range, xmt1;
  double atten_per_km, atten_at_100km;

  gate_data_beam_header_t *beam_header;

  /*
   * swap the beam
   */
  
  BE_to_array_16((ui16 *) beam, sizeof(bprp_beam_t));

  /*
   * if required, check radar id and skip this
   * routine if it does not match the target
   */
  
  radar_id = (int) (beam->hdr.xmt & 0x1f);

  if (Glob->params.check_radar_id) {
    if (radar_id != Glob->params.target_radar_id) {
      return (1);
    }
  }

  /*
   * send params every 90 beams
   */
  
  beam_count++;
  if (beam_count >= 90) {
    if (send_params(client_fd, radar_id, beam)) {
      return (-1);
    }
    beam_count = 0;
  }
  
  if (beam->hdr.date == 0 || beam->hdr.site_blk == 0) {
    return (-1);
  }
  
  if (Glob->params.time_mode == LOCAL_TO_UCT) {

    struct tm date;
	
    date.tm_year = beam->hdr.date / 0x200;
    date.tm_mon = 0;
    date.tm_mday = beam->hdr.date & 0x1ff; /* Strip Julian day	*/
    date.tm_hour = beam->hdr.hour;
    date.tm_min = beam->hdr.min / 60;
    date.tm_sec = beam->hdr.min % 60;
    date.tm_isdst = 0;
    beam_time = mktime(&date);

  } else {

    date_time_t date;

    date.year = beam->hdr.date / 0x200;
    if (date.year < 1900) {
      if (date.year < 50) {
	date.year += 2000;
      } else {
	date.year += 1900;
      }
    }

    date.month = 1;
    date.day = beam->hdr.date & 0x1ff; /* Strip Julian day	*/
    date.hour = beam->hdr.hour;
    date.min = beam->hdr.min / 60;
    date.sec = beam->hdr.min % 60;
    uconvert_to_utime(&date);
    beam_time = date.unix_time;

  }
  
  /*
   * PACER-dependent processing
   *
   * the azimuth and elevation are either binary coded decimals (Bethlehem) or
   * scaled integers (PACER)
   */

  if (Glob->params.pacer) {

    elevation = (double) (beam->hdr.elevation & 0x7fff) * (180.0 / 32768.0);
    azimuth = (double) beam->hdr.azimuth * (180.0 / 32768.0);

    raycount = beam->hdr.raycount;

  } else {

    azimuth = 0.0;
    elevation = 0.0;
  
    for (i = 0, shift = 12, mult = 1000;
	 i < 4;
	 i++, shift -= 4, mult /= 10) {
      azimuth += ((beam->hdr.azimuth >> shift) & 0x0F) * mult; 
      elevation += (((beam->hdr.elevation >> shift) & 0x0F) * mult); 
    }
  
    azimuth /= 10.0;
    elevation /= 10.0;

    raycount = beam->hdr.raycount - 513;

  }
  
  if(raycount == 0) {
    volume_no++;
  }
  tilt_no =  beam->hdr.raycount >> 9;

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    rc = (beam->hdr.raycount & 0x1FF);
    fprintf(stderr,
	    "beam_raycount, raycount, tilt_no, rc, az, el: "
	    "%d, %d, %d, %d, %5.1f %5.1f\n",
	    beam->hdr.raycount, raycount, tilt_no, rc, azimuth, elevation);
  }
  
  if (Glob->params.pacer) {

    skip = (beam->hdr.site_blk & 0x0ff) / 8; 
    viplo = beam->hdr.viplo;
    viphi = beam->hdr.viphi;

  } else {

    skip = (beam->hdr.site_blk & 0x0ff); 
    viplo = beam->hdr.viplo * 8;
    viphi = beam->hdr.viphi * 8;

  }

  atten_at_100km = Glob->params.atmos_attenuation * 100.0;
  atten_per_km = Glob->params.atmos_attenuation;
  radar_constant = Glob->params.radar_constant;
  bin_width = (beam->hdr.site_blk/256) & 0x0f;
  aperture = bin_width * KM_PER_US;
  start_range = skip * KM_PER_US + aperture / 2.0;
  plo = beam->hdr.plo / 32;
  phi = beam->hdr.phi / 32;
  slope = (phi-plo)/ (viphi-viplo);
  xmt1 = radar_constant + beam->hdr.xmt/32.0;

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "time: %s\n", utimstr(beam_time));
    fprintf(stderr, "azimuth: %g\n", azimuth);
    fprintf(stderr, "elevation: %g\n", elevation);
    fprintf(stderr, "volume_no: %d\n", volume_no);
    fprintf(stderr, "tilt_no: %d\n", tilt_no);
    fprintf(stderr, "atten_per_km: %g\n", atten_per_km);
    fprintf(stderr, "radar_constant: %g\n", radar_constant);
    fprintf(stderr, "bin_width: %d\n", bin_width);
    fprintf(stderr, "aperture: %g\n", aperture);
    fprintf(stderr, "start_range: %g\n", start_range);
    fprintf(stderr, "viplo: %g\n", viplo / 8);
    fprintf(stderr, "viphi: %g\n", viphi / 8);
    fprintf(stderr, "plo: %g\n", plo);
    fprintf(stderr, "phi: %g\n", phi);
    fprintf(stderr, "slope: %g\n", slope * 8);
    fprintf(stderr, "xmt1: %g\n", xmt1);
    computeDbz(100.0, atten_per_km, atten_at_100km,
	       xmt1, viplo, plo, slope,
	       beam->vip[0], TRUE);
  }

  /*
   * load beam header, swapping as necessary
   */
  
  beam_header = (gate_data_beam_header_t *) buffer;
  
  beam_header->time = BE_from_si32(beam_time);
  beam_header->azimuth = BE_from_si32((si32) (azimuth * 1000000.0));
  beam_header->elevation = BE_from_si32((si32) (elevation * 1000000.0));
  beam_header->target_elev = BE_from_si32((si32) (elevation * 1000000.0));
  beam_header->vol_num = BE_from_si32(volume_no);
  beam_header->tilt_num = BE_from_si32(tilt_no);
  beam_header->new_scan_limits = BE_from_si32(0L);
  beam_header->end_of_tilt = BE_from_si32((raycount & 0x1ff) == 0);
  beam_header->end_of_volume = BE_from_si32(raycount == 0);

  /*
   * load gates
   */
  
  gates = (ui08 *) (buffer + sizeof(*beam_header));
  range = start_range;

  for(i = 0; i < BPRP_GATES_PER_BEAM; i++, gates++, range += aperture)  {
    
    double dbz;
    int dbz_byte;

    if (range < 0.01) {
      fprintf(stderr, "WARNING - %s:send_beam\n", Glob->prog_name);
      fprintf(stderr, "range for gate %d computes as %g\n", i, range);
    }

    dbz = computeDbz(range, atten_per_km, atten_at_100km,
		     xmt1, viplo, plo, slope,
		     beam->vip[i], FALSE);
    
    dbz_byte = (int)((dbz - DBZ_BIAS) / DBZ_SCALE + 0.5);
    if (dbz_byte > 255) {
      dbz_byte = 255;
    } else if (dbz_byte < 0) {
      dbz_byte = 0;
    }
    *gates = (ui08) (dbz_byte & 0xff);

  } /* i */

  if (client_fd >= 0) {
    if (Glob->params.debug) {
      fprintf(stderr, "Writing beam packet\n");
    }
    if (SKU_write_message(client_fd, GATE_DATA_PACKET_CODE,
			  (char *) buffer,
			  sizeof(gate_data_beam_header_t) +
			  BPRP_GATES_PER_BEAM) < 0) {
      if (Glob->params.debug) {
	fprintf(stderr, "WARNING - %s:send_beam\n", Glob->prog_name);
	fprintf(stderr, "Beam write to client failed\n");
      }
      return (-1);
    } else {
      return (0);
    }

  }

  return (0);

}

static double computeDbz(double range,
			 double atten_per_km,
			 double atten_at_100km,
			 double xmt1,
			 double viplo,
			 double plo,
			 double slope,
			 int count,
			 int print)

{

  double power, range_correction;
  double dbz;

  range_correction =
    20.0 * log10(range / KM_PER_NM) +
    range * atten_per_km - atten_at_100km - xmt1;
  power = (count - viplo) * slope + plo;
  dbz = power + range_correction;
  
  if (print) {
    fprintf(stderr,
	    "range, count, range_corr, power, dbz: "
	    "%g, %d, %g, %g, %g\n",
	    range, count / 8,
	    range_correction, power, dbz);
  }
  
  return dbz;

}
