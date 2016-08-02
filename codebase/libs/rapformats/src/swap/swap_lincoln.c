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
 * swap_lincoln.c
 *
 * Byte swapping routines for lincoln moments data
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1997
 *
 ****************************************************************************/

#include <dataport/bigend.h>
#include <rapformats/swap.h>

void BE_to_ncar_udp_frame_hdr(ncar_udp_frame_hdr_t *hdr)

{

  hdr->frames_per_beam = BE_to_ui16(hdr->frames_per_beam);
  hdr->frame_this_pkt = BE_to_ui16(hdr->frame_this_pkt);
  hdr->frame_seq_num = BE_to_ui32(hdr->frame_seq_num);

}

void BE_from_ncar_udp_frame_hdr(ncar_udp_frame_hdr_t *hdr)

{

  hdr->frames_per_beam = BE_from_ui16(hdr->frames_per_beam);
  hdr->frame_this_pkt = BE_from_ui16(hdr->frame_this_pkt);
  hdr->frame_seq_num = BE_from_ui32(hdr->frame_seq_num);

}

void BE_to_ll_udp_frame_hdr(ll_udp_frame_hdr_t *hdr)

{

  hdr->frame_format = BE_to_ui16(hdr->frame_seq_num);
  hdr->frame_seq_num = BE_to_ui32(hdr->frame_seq_num);
  hdr->frames_per_beam = BE_to_ui16(hdr->frames_per_beam);
  hdr->frame_this_pkt = BE_to_ui16(hdr->frame_this_pkt);

  hdr->frag_hdr.frag_seqn = BE_to_ui32(hdr->frag_hdr.frag_seqn);
  hdr->frag_hdr.msg_bytes = BE_to_ui16(hdr->frag_hdr.msg_bytes);
  hdr->frag_hdr.frag_count = BE_to_ui16(hdr->frag_hdr.frag_count);
  hdr->frag_hdr.frag_nmbr = BE_to_ui16(hdr->frag_hdr.frag_nmbr);
  hdr->frag_hdr.frag_offset = BE_to_ui16(hdr->frag_hdr.frag_offset);
  hdr->frag_hdr.data_bytes = BE_to_ui16(hdr->frag_hdr.data_bytes);
  hdr->frag_hdr.options = BE_to_ui16(hdr->frag_hdr.options);

  hdr->rec_hdr.rec_id = BE_to_ui16(hdr->rec_hdr.rec_id);
  hdr->rec_hdr.rec_len = BE_to_ui16(hdr->rec_hdr.rec_len);
  hdr->rec_hdr.rec_seqn = BE_to_ui32(hdr->rec_hdr.rec_seqn);

}

void BE_from_ll_udp_frame_hdr(ll_udp_frame_hdr_t *hdr)

{

  hdr->frame_format = BE_from_ui16(hdr->frame_seq_num);
  hdr->frame_seq_num = BE_from_ui32(hdr->frame_seq_num);
  hdr->frames_per_beam = BE_from_ui16(hdr->frames_per_beam);
  hdr->frame_this_pkt = BE_from_ui16(hdr->frame_this_pkt);

  hdr->frag_hdr.frag_seqn = BE_from_ui32(hdr->frag_hdr.frag_seqn);
  hdr->frag_hdr.msg_bytes = BE_from_ui16(hdr->frag_hdr.msg_bytes);
  hdr->frag_hdr.frag_count = BE_from_ui16(hdr->frag_hdr.frag_count);
  hdr->frag_hdr.frag_nmbr = BE_from_ui16(hdr->frag_hdr.frag_nmbr);
  hdr->frag_hdr.frag_offset = BE_from_ui16(hdr->frag_hdr.frag_offset);
  hdr->frag_hdr.data_bytes = BE_from_ui16(hdr->frag_hdr.data_bytes);
  hdr->frag_hdr.options = BE_from_ui16(hdr->frag_hdr.options);

  hdr->rec_hdr.rec_id = BE_from_ui16(hdr->rec_hdr.rec_id);
  hdr->rec_hdr.rec_len = BE_from_ui16(hdr->rec_hdr.rec_len);
  hdr->rec_hdr.rec_seqn = BE_from_ui32(hdr->rec_hdr.rec_seqn);

}

void BE_to_ll_params(ll_params_t *params)

{

  int i;

  params->cpi_seq_num = BE_to_si16(params->cpi_seq_num);
  params->rad_seq_num = BE_to_si16(params->rad_seq_num);
  params->tilt_num = BE_to_si16(params->tilt_num);
  params->vol_num = BE_to_si16(params->vol_num);
  params->scan_mode = BE_to_si16(params->scan_mode);
  params->target_elev = BE_to_si16(params->target_elev);
  params->scan_dir = BE_to_si16(params->scan_dir);
  params->range_to_first_gate = BE_to_si16(params->range_to_first_gate);
  params->num_spacing_segs = BE_to_si16(params->num_spacing_segs);

  for (i = 0; i < LL_MAX_RANGE_SEGS; i++) {
    params->range_seg[i].gates_per_beam =
      BE_to_si16(params->range_seg[i].gates_per_beam);
    params->range_seg[i].gate_spacing =
      BE_to_si16(params->range_seg[i].gate_spacing);
  }

  params->prf = BE_to_si16(params->prf);
  params->month = BE_to_si16(params->month);
  params->day = BE_to_si16(params->day);
  params->year = BE_to_si16(params->year);
  params->hour = BE_to_si16(params->hour);
  params->min = BE_to_si16(params->min);
  params->sec = BE_to_si16(params->sec);
  params->azimuth = BE_to_ui16(params->azimuth);
  params->elevation = BE_to_si16(params->elevation);

  params->latitude = BE_to_si32(params->latitude);
  params->longitude = BE_to_si32(params->longitude);

  params->altitude = BE_to_si16(params->altitude);
  params->beamwidth = BE_to_si16(params->beamwidth);
  params->polarization = BE_to_si16(params->polarization);
  params->xmit_pwr = BE_to_si16(params->xmit_pwr);
  params->frequency = BE_to_si16(params->frequency);
  params->pulse_width = BE_to_si16(params->pulse_width);

  for (i = 0; i < LL_NFIELDS; i++) {
    params->scale[i] = BE_to_si16(params->scale[i]);
    params->bias[i] = BE_to_si16(params->bias[i]);
  }


  params->vel_field_size = BE_to_si16(params->vel_field_size);
  params->min_behind_uct = BE_to_si16(params->min_behind_uct);
  params->end_tilt_flag = BE_to_si16(params->end_tilt_flag);

}

void BE_from_ll_params(ll_params_t *params)

{

  int i;

  params->cpi_seq_num = BE_from_si16(params->cpi_seq_num);
  params->rad_seq_num = BE_from_si16(params->rad_seq_num);
  params->tilt_num = BE_from_si16(params->tilt_num);
  params->vol_num = BE_from_si16(params->vol_num);
  params->scan_mode = BE_from_si16(params->scan_mode);
  params->target_elev = BE_from_si16(params->target_elev);
  params->scan_dir = BE_from_si16(params->scan_dir);
  params->range_to_first_gate = BE_from_si16(params->range_to_first_gate);
  params->num_spacing_segs = BE_from_si16(params->num_spacing_segs);

  for (i = 0; i < LL_MAX_RANGE_SEGS; i++) {
    params->range_seg[i].gates_per_beam =
      BE_from_si16(params->range_seg[i].gates_per_beam);
    params->range_seg[i].gate_spacing =
      BE_from_si16(params->range_seg[i].gate_spacing);
  }

  params->prf = BE_from_si16(params->prf);
  params->month = BE_from_si16(params->month);
  params->day = BE_from_si16(params->day);
  params->year = BE_from_si16(params->year);
  params->hour = BE_from_si16(params->hour);
  params->min = BE_from_si16(params->min);
  params->sec = BE_from_si16(params->sec);
  params->azimuth = BE_from_ui16(params->azimuth);
  params->elevation = BE_from_si16(params->elevation);

  params->latitude = BE_from_si32(params->latitude);
  params->longitude = BE_from_si32(params->longitude);

  params->altitude = BE_to_si16(params->altitude);
  params->beamwidth = BE_from_si16(params->beamwidth);
  params->polarization = BE_from_si16(params->polarization);
  params->xmit_pwr = BE_from_si16(params->xmit_pwr);
  params->frequency = BE_from_si16(params->frequency);
  params->pulse_width = BE_from_si16(params->pulse_width);

  for (i = 0; i < LL_NFIELDS; i++) {
    params->scale[i] = BE_from_si16(params->scale[i]);
    params->bias[i] = BE_from_si16(params->bias[i]);
  }


  params->vel_field_size = BE_from_si16(params->vel_field_size);
  params->min_behind_uct = BE_from_si16(params->min_behind_uct);
  params->end_tilt_flag = BE_from_si16(params->end_tilt_flag);

}
