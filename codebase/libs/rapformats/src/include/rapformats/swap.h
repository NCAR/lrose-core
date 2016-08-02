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

/*******************************************************************
 * swap.h
 *
 * Swapping prototypes
 ******************************************************************/

#ifndef DsSwap_h
#define DsSwap_h

#ifdef __cplusplus
extern "C" {
#endif

#include <rapformats/lincoln.h>
#include <rapformats/rp7.h>
#include <rapformats/ridds.h>

/*
 * lincoln labs moments
 */
  
extern void BE_to_ncar_udp_frame_hdr(ncar_udp_frame_hdr_t *hdr);
extern void BE_to_ll_udp_frame_hdr(ll_udp_frame_hdr_t *hdr);
extern void BE_to_ll_params(ll_params_t *params);

extern void BE_from_ncar_udp_frame_hdr(ncar_udp_frame_hdr_t *hdr);
extern void BE_from_ll_udp_frame_hdr(ll_udp_frame_hdr_t *hdr);
extern void BE_from_ll_params(ll_params_t *params);

/*
 * rp7 moments
 */

extern void BE_to_rp7_params(rp7_params_t *params);
extern void BE_from_rp7_params(rp7_params_t *params);

/*
 * RIDDS moments
 */

extern void BE_to_RIDDS_vol_number(RIDDS_vol_number *num);
extern void BE_from_RIDDS_vol_number(RIDDS_vol_number *num);
extern void BE_to_RIDDS_vol_title(RIDDS_vol_title *title);
extern void BE_from_RIDDS_vol_title(RIDDS_vol_title *title);
extern void BE_to_RIDDS_msg_hdr(RIDDS_msg_hdr *hdr);
extern void BE_from_RIDDS_msg_hdr(RIDDS_msg_hdr *hdr);
extern void BE_to_RIDDS_data_hdr(RIDDS_data_hdr *data);
extern void BE_from_RIDDS_data_hdr(RIDDS_data_hdr *data);
extern void BE_to_RIDDS_hdr(RIDDS_hdr *hdr);
extern void BE_from_RIDDS_hdr(RIDDS_hdr *hdr);
extern void BE_to_RIDDS_status_data(RIDDS_status_data *hdr);
extern void BE_from_RIDDS_status_data(RIDDS_status_data *hdr);
extern void BE_to_RIDDS_adaptation_data(RIDDS_adaptation_data *hdr);
extern void BE_from_RIDDS_adaptation_data(RIDDS_adaptation_data *hdr);
extern void BE_to_RIDDS_clutter_hdr(RIDDS_clutter_hdr *hdr);
extern void BE_from_RIDDS_clutter_hdr(RIDDS_clutter_hdr *hdr);
extern void BE_to_RIDDS_vcp_hdr(RIDDS_VCP_hdr *hdr);
extern void BE_from_RIDDS_vcp_hdr(RIDDS_VCP_hdr *hdr);
extern void BE_to_RIDDS_elevation_angle(RIDDS_elevation_angle *hdr);
extern void BE_from_RIDDS_elevation_angle(RIDDS_elevation_angle *hdr);
extern void BE_to_RIDDS_data_31_hdr(RIDDS_data_31_hdr *hdr);
extern void BE_from_RIDDS_data_31_hdr(RIDDS_data_31_hdr *hdr);
extern void BE_to_RIDDS_volume_31_hdr(RIDDS_volume_31_hdr *hdr);
extern void BE_from_RIDDS_volume_31_hdr(RIDDS_volume_31_hdr *hdr);
extern void BE_to_RIDDS_elev_31_hdr(RIDDS_elev_31_hdr *hdr);
extern void BE_from_RIDDS_elev_31_hdr(RIDDS_elev_31_hdr *hdr);
extern void BE_to_RIDDS_radial_31_hdr(RIDDS_radial_31_hdr *);
extern void BE_from_RIDDS_radial_31_hdr(RIDDS_radial_31_hdr *hdr);
extern void BE_to_RIDDS_data_31(RIDDS_data_31 *data);
extern void BE_from_RIDDS_data_31(RIDDS_data_31 *data);

#ifdef __cplusplus
}
#endif

#endif

