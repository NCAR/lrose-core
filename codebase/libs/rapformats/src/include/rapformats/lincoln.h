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
#ifdef __cplusplus
 extern "C" {
#endif
/**********************************************************************
 * lincoln.h
 *
 * data structures for the lincoln labs ethernet header
 *
 * First packet for beam:
 *     ll_frame_hdr_t
 *     ll_params_t
 *     data
 *
 * Subsequent packets for beam:
 *     ll_frame_hdr_t
 *     ll_subseq_params_t
 *     data
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 **********************************************************************/

#ifndef _lincoln_h
#define _lincoln_h

#include <dataport/port_types.h>
#include <sys/types.h>

/*
 * number of fields
 */

#define LL_NFIELDS 4

/*
 * factor by which scale and bias values have been multiplied
 * before storage
 */

#define LL_SCALE_AND_BIAS_MULT 100

/*
 * scan modes
 */

#define LL_SURVEILLANCE_MODE 1
#define LL_RHI_MODE 2
#define LL_SECTOR_MODE 3

/*
 * packet header code
 */

#define LL_BEAM_PACKET_CODE 0x6001

/*
 * packet types
 */

# define FIRST_PKT 1
# define CONTINUE_PKT 2

/*
 * ll udp fragmentation header
 */
   
typedef struct {
  ui32 frag_seqn;   /* incremental fragment sequence number       */
  ui16 msg_bytes;   /* total message length (bytes), w/o hdr      */
  ui16 frag_count;  /* total number of fragments in message       */
  ui16 frag_nmbr;   /* current fragment index (1,2,..,frag_count) */
  ui16 frag_offset; /* offset from message start (bytes)          */
  ui16 data_bytes;  /* length of packet minus this header         */
  ui16 options;     /* optional flag bits                         */
} ll_frag_hdr_t;

typedef struct {
  ui16 rec_id;    /* Always "150" = 0x96 for BDE records */
  ui16 rec_len;   /* Record halfwords (16-bits), including SC_PAC hdr     */
  ui32 rec_seqn;  /* Increments by one for each frame, rolls over to zero */
} ll_rec_hdr_t;

/*
 * struct for range_segs
 */

typedef struct {
  si16 gates_per_beam;
  si16 gate_spacing;
} ll_range_seg_t;

#define LL_MAX_RANGE_SEGS 10

/*
 * lincoln parameters for first packet
 */

typedef struct {

  si16 cpi_seq_num;
  si16 rad_seq_num;
  si16 tilt_num;
  si16 vol_num;
  si16 scan_mode;
  si16 target_elev;
  si16 scan_dir;
  si16 range_to_first_gate;
  si16 num_spacing_segs;
  ll_range_seg_t range_seg[LL_MAX_RANGE_SEGS];
  si16 prf;
  si16 month;
  si16 day;
  si16 year;
  si16 hour;
  si16 min;
  si16 sec;
  ui16 azimuth;
  si16 elevation;
  char radar_name[16];
  char site_name[16];
  char proj_name[16];
  si32 latitude;
  si32 longitude;
  si16 altitude;
  si16 beamwidth;
  si16 polarization;
  si16 xmit_pwr;
  si16 frequency;
  si16 pulse_width;
  char prod_name[LL_NFIELDS][8];
  si16 scale[LL_NFIELDS];
  si16 bias[LL_NFIELDS];
  si16 vel_field_size;
  si16 min_behind_uct;
  si16 end_tilt_flag;
  si16 vcp;
  si16 vel_gpb;
  si16 ref_gpb;
  si16 ref_spacing;
  si16 vel_spacing;
  si16 ref_gate1;
  si16 vel_gate1;
  si16 unused[6];

} ll_params_t;

/*
 * lincoln parameters for subsequent packets
 */

typedef struct {

  ui16 pkt_start_gate;
  ui16 pkt_ngates;
  
} ll_pkt_gate_t;

/*
 * checksum at end of base data packets
 */

typedef si32 ll_checksum_t;

typedef struct {

  ui08 edst[6];              /* destination address */
  ui08 esrc[6];              /* source address */
  ui16 nit_frame_type;
  ui16 frame_format;
  ui32 frame_seq_num;
  ui16 frames_per_beam;
  ui16 frame_this_pkt;
  ui16 unused[4];

} ll_nit_frame_hdr_t;

typedef struct {

  ll_frag_hdr_t frag_hdr;
  ll_rec_hdr_t rec_hdr;
  ui08 unused1[14];
  ui16 frame_format;
  ui32 frame_seq_num;
  ui16 frames_per_beam;
  ui16 frame_this_pkt;
  ui16 unused[4];

} ll_udp_frame_hdr_t;

typedef struct {

  ui16 frames_per_beam;
  ui16 frame_this_pkt;
  ui32 frame_seq_num;
  
} ncar_udp_frame_hdr_t;

#define LL_MAX_NGATES 1538

typedef struct {
  ll_params_t hdr;
  ll_pkt_gate_t pkt;
  ui08 data[LL_NFIELDS * LL_MAX_NGATES];
} ll_beam_rec_t;

#endif
#ifdef __cplusplus
}
#endif
