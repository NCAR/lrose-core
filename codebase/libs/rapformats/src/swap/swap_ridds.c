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
/******************************************************************
 * swap_ridds.c
 *
 * Byte swapping routines for RIDDS structs
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * May 1997
 *
 */

#include <dataport/bigend.h>
#include <rapformats/swap.h>

void BE_to_RIDDS_vol_number(RIDDS_vol_number *num)
{
  BE_to_array_16(&num->vol_number, 2);
}

void BE_from_RIDDS_vol_number(RIDDS_vol_number *num)
{
  BE_from_array_16(&num->vol_number, 2);
}

void BE_to_RIDDS_vol_title(RIDDS_vol_title *title)
{
  BE_to_array_32(&title->julian_date, 4);
  BE_to_array_32(&title->millisecs_past_midnight, 4);
  BE_to_array_32(&title->filler1, 4);
}

void BE_from_RIDDS_vol_title(RIDDS_vol_title *title)
{
  BE_from_array_32(&title->julian_date, 4);
  BE_from_array_32(&title->millisecs_past_midnight, 4);
  BE_from_array_32(&title->filler1, 4);
}

void BE_to_RIDDS_msg_hdr(RIDDS_msg_hdr *hdr)
{
  BE_to_array_16(&hdr->message_len, 2);
  BE_to_array_16(&hdr->seq_num, 2);
  BE_to_array_16(&hdr->julian_date, 2);
  BE_to_array_32(&hdr->millisecs_past_midnight, 4);
  BE_to_array_16(&hdr->num_message_segs, 2);
  BE_to_array_16(&hdr->message_seg_num, 2);
}

void BE_from_RIDDS_msg_hdr(RIDDS_msg_hdr *hdr)
{
  BE_from_array_16(&hdr->message_len, 2);
  BE_from_array_16(&hdr->seq_num, 2);
  BE_from_array_16(&hdr->julian_date, 2);
  BE_from_array_32(&hdr->millisecs_past_midnight, 4);
  BE_from_array_16(&hdr->num_message_segs, 2);
  BE_from_array_16(&hdr->message_seg_num, 2);
}

void BE_to_RIDDS_data_hdr(RIDDS_data_hdr *data)
{
  BE_to_array_32(&data->millisecs_past_midnight, 4);
  BE_to_array_32(&data->sys_gain_cal_const, 4);
  BE_to_array_16(&data->julian_date, RIDDS_NSI16S_AFTER_JULIAN_DATE * 2);
  BE_to_array_16(&data->ref_ptr, RIDDS_NSI16S_AFTER_REF_PTR * 2);
}

void BE_from_RIDDS_data_hdr(RIDDS_data_hdr *data)
{
  BE_from_array_32(&data->millisecs_past_midnight, 4);
  BE_from_array_32(&data->sys_gain_cal_const, 4);
  BE_from_array_16(&data->julian_date, RIDDS_NSI16S_AFTER_JULIAN_DATE * 2);
  BE_from_array_16(&data->ref_ptr, RIDDS_NSI16S_AFTER_REF_PTR * 2);
}

void BE_to_RIDDS_hdr(RIDDS_hdr *hdr)
{
  BE_to_array_32(hdr, sizeof(RIDDS_hdr));
}

void BE_from_RIDDS_hdr(RIDDS_hdr *hdr)
{
  BE_from_array_32(hdr, sizeof(RIDDS_hdr));
}

void BE_to_RIDDS_status_data(RIDDS_status_data *hdr)
{
  BE_to_array_16(hdr, 2*40);
}

void BE_from_RIDDS_status_data(RIDDS_status_data *hdr)
{
  BE_from_array_16(hdr, 2*40);
}

void BE_to_RIDDS_adaptation_data(RIDDS_adaptation_data *hdr)
{
  BE_to_array_32(&hdr->k1, 2182*4);            /* Easier to do the whole message in 4 byte blocks */
  BE_from_array_32(&hdr->rpg_co_located, 4*4); /* and then undo the few char arrays */
  BE_from_array_32(&hdr->slatdir, 2*4);
  BE_from_array_32(&hdr->site_name, 4);
}

void BE_from_RIDDS_adaptation_data(RIDDS_adaptation_data *hdr)
{
  BE_from_array_32(&hdr->k1, 2182*4);
  BE_to_array_32(&hdr->rpg_co_located, 4*4);
  BE_to_array_32(&hdr->slatdir, 2*4);
  BE_to_array_32(&hdr->site_name, 4);
}

void BE_to_RIDDS_clutter_hdr(RIDDS_clutter_hdr *hdr)
{
  BE_to_array_16(&hdr->julian_date, 2);
  BE_to_array_16(&hdr->minutes_past_midnight, 2);
  BE_to_array_16(&hdr->num_message_segs, 2);
}

void BE_from_RIDDS_clutter_hdr(RIDDS_clutter_hdr *hdr)
{
  BE_from_array_16(&hdr->julian_date, 2);
  BE_from_array_16(&hdr->minutes_past_midnight, 2);
  BE_from_array_16(&hdr->num_message_segs, 2);
}

void BE_to_RIDDS_vcp_hdr(RIDDS_VCP_hdr *hdr)
{
  BE_to_array_16(&hdr->message_len, 11*2);
}

void BE_from_RIDDS_vcp_hdr(RIDDS_VCP_hdr *hdr)
{
  BE_from_array_16(&hdr->message_len, 11*2);
}

void BE_to_RIDDS_elevation_angle(RIDDS_elevation_angle *hdr)
{
  BE_to_array_16(&hdr->elevation_angle, 2);
  BE_to_array_16(&hdr->surveillance_prf_pulse_count, 20*2);
}

void BE_from_RIDDS_elevation_angle(RIDDS_elevation_angle *hdr)
{
  BE_from_array_16(&hdr->elevation_angle, 2);
  BE_from_array_16(&hdr->surveillance_prf_pulse_count, 20*2);
}

void BE_to_RIDDS_data_31_hdr(RIDDS_data_31_hdr *hdr)
{
  BE_to_array_32(&hdr->millisecs_past_midnight, 4);
  BE_to_array_16(&hdr->julian_date, 2);
  BE_to_array_16(&hdr->radial_num, 2);
  BE_to_array_32(&hdr->azimuth, 4);
  BE_to_array_16(&hdr->radial_length, 2);
  BE_to_array_32(&hdr->elevation, 4);
  BE_to_array_16(&hdr->data_blocks, 2);
  BE_to_array_32(&hdr->volume_ptr, 4*9);
}

void BE_from_RIDDS_data_31_hdr(RIDDS_data_31_hdr *hdr)
{
  BE_from_array_32(&hdr->millisecs_past_midnight, 4);
  BE_from_array_16(&hdr->julian_date, 2);
  BE_from_array_16(&hdr->radial_num, 2);
  BE_from_array_32(&hdr->azimuth, 4);
  BE_from_array_16(&hdr->radial_length, 2);
  BE_from_array_32(&hdr->elevation, 4);
  BE_from_array_16(&hdr->data_blocks, 2);
  BE_from_array_32(&hdr->volume_ptr, 4*9);
}

void BE_to_RIDDS_volume_31_hdr(RIDDS_volume_31_hdr *hdr)
{
  BE_to_array_16(&hdr->block_size, 2);
  BE_to_array_32(&hdr->lat, 4);
  BE_to_array_32(&hdr->lon, 4);
  BE_to_array_16(&hdr->height, 2);
  BE_to_array_16(&hdr->feedhorn_height, 2);
  BE_to_array_32(&hdr->dbz0, 4*5);
  BE_to_array_16(&hdr->vol_coverage_pattern, 4);
}

void BE_from_RIDDS_volume_31_hdr(RIDDS_volume_31_hdr *hdr)
{
  BE_from_array_16(&hdr->block_size, 2);
  BE_from_array_32(&hdr->lat, 4);
  BE_from_array_32(&hdr->lon, 4);
  BE_from_array_16(&hdr->height, 2);
  BE_from_array_16(&hdr->feedhorn_height, 2);
  BE_from_array_32(&hdr->dbz0, 4*5);
  BE_from_array_16(&hdr->vol_coverage_pattern, 4);
}

void BE_to_RIDDS_elev_31_hdr(RIDDS_elev_31_hdr *hdr)
{
  BE_to_array_16(&hdr->block_size, 2);
  BE_to_array_16(&hdr->atmos, 2);
  BE_to_array_32(&hdr->dbz0, 4);
}

void BE_from_RIDDS_elev_31_hdr(RIDDS_elev_31_hdr *hdr)
{
  BE_from_array_16(&hdr->block_size, 2);
  BE_from_array_16(&hdr->atmos, 2);
  BE_from_array_32(&hdr->dbz0, 4);
}

void BE_to_RIDDS_radial_31_hdr(RIDDS_radial_31_hdr *hdr)
{
  BE_to_array_16(&hdr->block_size, 2);
  BE_to_array_16(&hdr->unamb_range_x10, 2);
  BE_to_array_32(&hdr->horiz_noise, 4);
  BE_to_array_32(&hdr->vert_noise, 4);
  BE_to_array_16(&hdr->nyquist_vel, 4);
}

void BE_from_RIDDS_radial_31_hdr(RIDDS_radial_31_hdr *hdr)
{
  BE_from_array_16(&hdr->block_size, 2);
  BE_from_array_16(&hdr->unamb_range_x10, 2);
  BE_from_array_32(&hdr->horiz_noise, 4);
  BE_from_array_32(&hdr->vert_noise, 4);
  BE_from_array_16(&hdr->nyquist_vel, 4);
}

void BE_to_RIDDS_data_31(RIDDS_data_31 *data)
{
  BE_to_array_32(&data->reserved_1, 4);
  BE_to_array_16(&data->num_gates, 2*5); 
  BE_to_array_32(&data->scale, 4);
  BE_to_array_32(&data->offset, 4);
}

void BE_from_RIDDS_data_31(RIDDS_data_31 *data)
{
  BE_from_array_32(&data->reserved_1, 4);
  BE_from_array_16(&data->num_gates, 2*5); 
  BE_from_array_32(&data->scale, 4);
  BE_from_array_32(&data->offset, 4);
}
