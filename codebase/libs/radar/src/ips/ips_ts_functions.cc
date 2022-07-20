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
/////////////////////////////////////////////////////////////////
// ips_ts_functions.cc
//
// Utility routines for ips_ts_data structs
//
// Mike Dixon, RAL, EOL, POBox 3000, Boulder, CO, 80307-3000
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
//
///////////////////////////////////////////////////////////////

#include <dataport/swap.h>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <radar/ips_ts_functions.hh>

using namespace std;

/////////////////////////////
// check for a missing values

bool ips_ts_int_is_missing(si32 val)
{
  if (val == IPS_TS_MISSING_INT) {
    return true;
  } else {
    return false;
  }
}

bool ips_ts_float_is_missing(fl32 val)
{
  if (std::isnan(val)) {
    return true;
  } else if (fabs(val - IPS_TS_MISSING_FLOAT) < 0.001)  {
    return true;
  } else {
    return false;
  }
}

//////////////////////////////////////////////////////////////////
// struct initialization
// sets values to missing as appropriate

//////////////////////////////////////////////////////
// init packet info struct

void ips_ts_packet_info_init(ips_ts_packet_info_t &val)

{

  MEM_zero(val);

}

//////////////////////////////////////////////////////
// init sync struct

void ips_ts_sync_init(ips_ts_sync_t &val)

{

  MEM_zero(val);

  val.packet.id = IPS_TS_SYNC_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);

  val.magik[0] = IPS_TS_SYNC_VAL_00;
  val.magik[1] = IPS_TS_SYNC_VAL_01;

}

//////////////////////////////////////////////////////
// init version struct

void ips_ts_version_init(ips_ts_version_t &val)

{

  MEM_zero(val);

  val.packet.id = IPS_TS_VERSION_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);
  
}

//////////////////////////////////////////////////////
// init radar_info struct

void ips_ts_radar_info_init(ips_ts_radar_info &val)

{

  MEM_zero(val);

  val.packet.id = IPS_TS_RADAR_INFO_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);

  val.latitude_deg = IPS_TS_MISSING_FLOAT;
  val.longitude_deg = IPS_TS_MISSING_FLOAT;
  val.altitude_m = IPS_TS_MISSING_FLOAT;
  val.platform_type = static_cast<si32>(ips_ts_radar_platform_t::NOT_SET);

  val.beamwidth_deg_h = IPS_TS_MISSING_FLOAT;
  val.beamwidth_deg_v = IPS_TS_MISSING_FLOAT;
  val.wavelength_cm = IPS_TS_MISSING_FLOAT;
  
  val.nominal_gain_ant_db_h = IPS_TS_MISSING_FLOAT;
  val.nominal_gain_ant_db_v = IPS_TS_MISSING_FLOAT;

}

//////////////////////////////////////////////////////
// init scan_segment struct

void ips_ts_scan_segment_init(ips_ts_scan_segment &val)

{

  MEM_zero(val);

  val.packet.id = IPS_TS_SCAN_SEGMENT_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);

  val.scan_mode = static_cast<si32>(ips_ts_scan_mode_t::NOT_SET);
  val.volume_num = IPS_TS_MISSING_INT;
  val.sweep_num = IPS_TS_MISSING_INT;

  val.az_start = IPS_TS_MISSING_FLOAT;
  val.el_start = IPS_TS_MISSING_FLOAT;
  val.scan_rate = IPS_TS_MISSING_FLOAT;
  val.left_limit = IPS_TS_MISSING_FLOAT;
  val.right_limit = IPS_TS_MISSING_FLOAT;
  val.up_limit = IPS_TS_MISSING_FLOAT;
  val.down_limit = IPS_TS_MISSING_FLOAT;
  val.step = IPS_TS_MISSING_FLOAT;

  val.current_fixed_angle = IPS_TS_MISSING_FLOAT;

  val.n_sweeps = 0;
  
  val.sun_scan_sector_width_az = IPS_TS_MISSING_FLOAT;  
  val.sun_scan_sector_width_el = IPS_TS_MISSING_FLOAT;  

}

//////////////////////////////////////////////////////
// init ts_processing struct

void ips_ts_processing_init(ips_ts_processing_t &val)

{

  MEM_zero(val);

  val.packet.id = IPS_TS_PROCESSING_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);

  val.pol_mode = static_cast<si32>(ips_ts_pol_mode_t::NOT_SET);
  val.prf_mode = static_cast<si32>(ips_ts_prf_mode_t::NOT_SET);
  val.pulse_shape = static_cast<si32>(ips_ts_pulse_shape_t::NOT_SET);

  val.pulse_width_us = IPS_TS_MISSING_FLOAT;

  val.start_range_m = IPS_TS_MISSING_FLOAT;
  val.gate_spacing_m = IPS_TS_MISSING_FLOAT;

  val.test_pulse_range_km = IPS_TS_MISSING_FLOAT;
  val.test_pulse_length_us = IPS_TS_MISSING_FLOAT;

  val.num_prts = 1;
  for (int ii = 0; ii < IPS_TS_MAX_PRT; ii++) {
    val.prt_us[ii] = IPS_TS_MISSING_FLOAT;
  }

}

//////////////////////////////////////////////////////
// init status_xml struct

void ips_ts_status_xml_init(ips_ts_status_xml_t &val)

{

  MEM_zero(val);

  val.packet.id = IPS_TS_STATUS_XML_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);
  val.xml_len = 0;

}

//////////////////////////////////////////////////////
// init calibration struct

void ips_ts_calibration_init(ips_ts_calibration_t &val)

{

  MEM_zero(val);

  val.packet.id = IPS_TS_CALIBRATION_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);

  val.wavelength_cm = IPS_TS_MISSING_FLOAT;
  val.beamwidth_deg_h = IPS_TS_MISSING_FLOAT;
  val.beamwidth_deg_v = IPS_TS_MISSING_FLOAT;
  val.gain_ant_db_h = IPS_TS_MISSING_FLOAT;
  val.gain_ant_db_v = IPS_TS_MISSING_FLOAT;
  val.pulse_width_us = IPS_TS_MISSING_FLOAT;
  val.xmit_power_dbm_h = IPS_TS_MISSING_FLOAT;
  val.xmit_power_dbm_v = IPS_TS_MISSING_FLOAT;
  val.two_way_waveguide_loss_db_h = IPS_TS_MISSING_FLOAT;
  val.two_way_waveguide_loss_db_v = IPS_TS_MISSING_FLOAT;
  val.two_way_radome_loss_db_h = IPS_TS_MISSING_FLOAT;
  val.two_way_radome_loss_db_v = IPS_TS_MISSING_FLOAT;
  val.receiver_mismatch_loss_db = IPS_TS_MISSING_FLOAT;
  val.k_squared_water = IPS_TS_MISSING_FLOAT;
  val.radar_constant_h = IPS_TS_MISSING_FLOAT;
  val.radar_constant_v = IPS_TS_MISSING_FLOAT;
  val.noise_dbm_hc = IPS_TS_MISSING_FLOAT;
  val.noise_dbm_hx = IPS_TS_MISSING_FLOAT;
  val.noise_dbm_vc = IPS_TS_MISSING_FLOAT;
  val.noise_dbm_vx = IPS_TS_MISSING_FLOAT;
  val.i0_dbm_hc = IPS_TS_MISSING_FLOAT;
  val.i0_dbm_hx = IPS_TS_MISSING_FLOAT;
  val.i0_dbm_vc = IPS_TS_MISSING_FLOAT;
  val.i0_dbm_vx = IPS_TS_MISSING_FLOAT;
  val.receiver_gain_db_hc = IPS_TS_MISSING_FLOAT;
  val.receiver_gain_db_hx = IPS_TS_MISSING_FLOAT;
  val.receiver_gain_db_vc = IPS_TS_MISSING_FLOAT;
  val.receiver_gain_db_vx = IPS_TS_MISSING_FLOAT;
  val.receiver_slope_hc = IPS_TS_MISSING_FLOAT;
  val.receiver_slope_hx = IPS_TS_MISSING_FLOAT;
  val.receiver_slope_vc = IPS_TS_MISSING_FLOAT;
  val.receiver_slope_vx = IPS_TS_MISSING_FLOAT;
  val.dynamic_range_db_hc = IPS_TS_MISSING_FLOAT;
  val.dynamic_range_db_hx = IPS_TS_MISSING_FLOAT;
  val.dynamic_range_db_vc = IPS_TS_MISSING_FLOAT;
  val.dynamic_range_db_vx = IPS_TS_MISSING_FLOAT;
  val.base_dbz_1km_hc = IPS_TS_MISSING_FLOAT;
  val.base_dbz_1km_hx = IPS_TS_MISSING_FLOAT;
  val.base_dbz_1km_vc = IPS_TS_MISSING_FLOAT;
  val.base_dbz_1km_vx = IPS_TS_MISSING_FLOAT;
  val.sun_power_dbm_hc = IPS_TS_MISSING_FLOAT;
  val.sun_power_dbm_hx = IPS_TS_MISSING_FLOAT;
  val.sun_power_dbm_vc = IPS_TS_MISSING_FLOAT;
  val.sun_power_dbm_vx = IPS_TS_MISSING_FLOAT;
  val.noise_source_power_dbm_h = IPS_TS_MISSING_FLOAT;
  val.noise_source_power_dbm_v = IPS_TS_MISSING_FLOAT;
  val.power_meas_loss_db_h = IPS_TS_MISSING_FLOAT;
  val.power_meas_loss_db_v = IPS_TS_MISSING_FLOAT;
  val.coupler_forward_loss_db_h = IPS_TS_MISSING_FLOAT;
  val.coupler_forward_loss_db_v = IPS_TS_MISSING_FLOAT;
  val.test_power_dbm_h = IPS_TS_MISSING_FLOAT;
  val.test_power_dbm_v = IPS_TS_MISSING_FLOAT;
  val.zdr_correction_db = 0.0;
  val.ldr_correction_db_h = 0.0;
  val.ldr_correction_db_v = 0.0;
  val.phidp_rot_deg = 0.0;
  val.dbz_correction = 0.0;
  
}

//////////////////////////////////////////////////////
// init event_notice struct

void ips_ts_event_notice_init(ips_ts_event_notice_t &val)

{

  MEM_zero(val);

  val.packet.id = IPS_TS_EVENT_NOTICE_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);

  val.start_of_sweep = 0;
  val.end_of_sweep = 0;

  val.start_of_volume = 0;
  val.end_of_volume = 0;
  
  val.scan_mode = static_cast<si32>(ips_ts_scan_mode_t::NOT_SET);
  val.volume_num = IPS_TS_MISSING_INT;
  val.sweep_num = IPS_TS_MISSING_INT;
  
  val.current_fixed_angle = IPS_TS_MISSING_FLOAT;

}

//////////////////////////////////////////////////////
// init pulse_header struct

void ips_ts_pulse_header_init(ips_ts_pulse_header_t &val)

{

  MEM_zero(val);

  val.packet.id = IPS_TS_PULSE_HEADER_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);

  val.pulse_seq_num = 0;
  val.dwell_seq_num = 0;

  val.beam_num_in_dwell = 0;
  val.visit_num_in_beam = 0;

  val.scan_mode = static_cast<si32>(ips_ts_scan_mode_t::NOT_SET);
  val.sweep_num = IPS_TS_MISSING_INT;
  val.volume_num = IPS_TS_MISSING_INT;

  val.elevation = IPS_TS_MISSING_FLOAT;
  val.azimuth = IPS_TS_MISSING_FLOAT;
  val.fixed_angle = IPS_TS_MISSING_FLOAT;
  
  val.prt = IPS_TS_MISSING_FLOAT;
  val.prt_next = IPS_TS_MISSING_FLOAT;
  
  val.pulse_width_us = IPS_TS_MISSING_FLOAT;

  val.n_gates = 0;
  val.start_range_m = IPS_TS_MISSING_FLOAT;
  val.gate_spacing_m = IPS_TS_MISSING_FLOAT;

  val.hv_flag = IPS_TS_MISSING_INT;
  val.phase_cohered = IPS_TS_MISSING_INT;

  val.iq_encoding = static_cast<si32>(ips_ts_iq_encoding_t::NOT_SET);
  val.n_channels = 1;
  val.n_data = 0;

  val.scale = 1.0;
  val.offset = 0.0;

  for (int ii = 0; ii < IPS_TS_MAX_CHAN; ii++) {
    val.chan_is_copol[ii] = -1;
  }

  val.status = 0;
  val.event_flags = 0;
  
}

//////////////////////////////////////////////////////
// init platform_georef struct

void ips_ts_platform_georef_init(ips_ts_platform_georef_t &val)

{
  MEM_zero(val);
  val.packet.id = IPS_TS_PLATFORM_GEOREF_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
}

//////////////////////////////////////////////////////
// init georef_correction struct

void ips_ts_georef_correction_init(ips_ts_georef_correction_t &val)

{
  MEM_zero(val);
  val.packet.id = IPS_TS_GEOREF_CORRECTION_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  ips_ts_set_packet_time_to_now(val.packet);
}

////////////////////////////////////////////////////////////
// set packet sequence number

void ips_ts_set_packet_seq_num(ips_ts_packet_info_t &packet, si64 seq_num) {
  packet.seq_num = seq_num;
}

////////////////////////////////////////////////////////////
// set packet time

void ips_ts_set_packet_time(ips_ts_packet_info_t &packet,
                            double dtime) {
  time_t secs = (time_t) dtime;
  int nano_secs = (int) ((dtime - secs) * 1.0e9 + 0.5);
  packet.time_secs_utc = secs;
  packet.time_nano_secs = nano_secs;
}

void ips_ts_set_packet_time(ips_ts_packet_info_t &packet,
                            time_t secs, int nano_secs) {
  packet.time_secs_utc = secs;
  packet.time_nano_secs = nano_secs;
}

void ips_ts_set_packet_time_to_now(ips_ts_packet_info_t &packet) {
  struct timeval time;
  gettimeofday(&time, NULL);
  packet.time_secs_utc = time.tv_sec;
  packet.time_nano_secs = time.tv_usec * 1000;
}

//////////////////////////////////////////////////////////////////
// check packet id for validity, swapping as required.
// returns 0 on success, -1 on failure

int ips_ts_check_packet_id(si32 packetId, bool *isSwapped)

{

  if (isSwapped != NULL) {
    *isSwapped = false;
  }
  
  si32 id = packetId;
  if (ipsIdIsSwapped(id)) {
    SWAP_array_32(&id, sizeof(si32));
    if (isSwapped != NULL) {
      *isSwapped = true;
    }
  }

  if (id >= IPS_TS_SYNC_ID &&
      id < IPS_TS_SYNC_ID + 1000) {
    return 0;
  }

  return -1;

}

//////////////////////////////////////////////////////////////////
// check packet id for validity, swapping in-place as required.
// also swaps the packet_len argument.
// returns 0 on success, -1 on failure

int ips_ts_check_packet_id(si32 &packetId, si32 &packetLen, bool *isSwapped)

{

  if (isSwapped != NULL) {
    *isSwapped = false;
  }
  
  if (ipsIdIsSwapped(packetId)) {
    SWAP_array_32(&packetId, sizeof(si32));
    SWAP_array_32(&packetLen, sizeof(si32));
    if (isSwapped != NULL) {
      *isSwapped = true;
    }
  }

  return ips_ts_check_packet_id(packetId);

}

//////////////////////////////////////////////////////////////////
// get packet id, check for validity of this packet
// checks the packet length
// prints error in debug mode
// returns 0 on success, -1 on failure

int ips_ts_get_packet_id(const void* buf, int len, int &packet_id)

{

  if (len < (int) sizeof(si32)) {
    return -1;
  }

  // get packet ID
  
  si32 id;
  memcpy(&id, buf, sizeof(si32));
  if (ipsIdIsSwapped(id)) {
    SWAP_array_32(&id, sizeof(si32));
  }

  packet_id = id;

  int iret = 0;
  switch (packet_id) {

    case IPS_TS_SYNC_ID:
      if (len < (int) sizeof(ips_ts_sync_t)) {
	iret = -1;
      } break;

    case IPS_TS_RADAR_INFO_ID:
      if (len < (int) sizeof(ips_ts_radar_info_t)) {
	iret = -1;
      } break;

    case IPS_TS_SCAN_SEGMENT_ID:
      if (len < (int) sizeof(ips_ts_scan_segment_t)) {
	iret = -1;
      } break;

    case IPS_TS_PROCESSING_ID:
      if (len < (int) sizeof(ips_ts_processing_t)) {
	iret = -1;
      } break;

    case IPS_TS_STATUS_XML_ID:
      if (len < (int) sizeof(ips_ts_status_xml_t)) {
	iret = -1;
      } break;

    case IPS_TS_CALIBRATION_ID:
      if (len < (int) sizeof(ips_ts_calibration_t)) {
	iret = -1;
      } break;

    case IPS_TS_EVENT_NOTICE_ID:
      if (len < (int) sizeof(ips_ts_event_notice_t)) {
	iret = -1;
      } break;

    case IPS_TS_PULSE_HEADER_ID:
      if (len < (int) sizeof(ips_ts_pulse_header_t)) {
	iret = -1;
      } break;

  }

  return iret;

}

//////////////////////////////////////////////////////////////////
// Check if packet buffer has correct radar id.
// Returns true if correct, false if not.
// A specified radarId of 0 will match all messages.

bool ips_ts_check_radar_id(const void *buf,
                           int len,
                           int radarId)
  
{
  if (radarId == 0) {
    // matches all packets
    return true;
  }
  if (len < (int) sizeof(ips_ts_packet_info_t)) {
    // too small for a valid packet
    return false;
  }
  ips_ts_packet_info_t info;
  memcpy(&info, buf, sizeof(info));
  ips_ts_packet_info_swap(info);
  if (info.radar_id == 0 ||
      info.radar_id == radarId) {
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////
// get packet time as a double

double ips_ts_get_packet_time_as_double(const ips_ts_packet_info_t &packet)

{
  return (packet.time_secs_utc + packet.time_nano_secs / 1.0e9);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// swapping routines
//
// swap to native as required
// swapping is the responsibility of the user, data is always
// written in native

////////////////////////////////
// swap depending on packet type
// returns 0 on success, -1 on failure

int ips_ts_packet_swap(void *buf, int len)
{

  int packet_id;
  if (ips_ts_get_packet_id(buf, len, packet_id)) {
    return -1;
  }

  switch (packet_id) {

    case IPS_TS_SYNC_ID:
      ips_ts_sync_swap(*((ips_ts_sync_t *) buf));
      break;

    case IPS_TS_RADAR_INFO_ID:
      ips_ts_radar_info_swap(*((ips_ts_radar_info_t *) buf));
      break;

    case IPS_TS_SCAN_SEGMENT_ID:
      ips_ts_scan_segment_swap(*((ips_ts_scan_segment_t *) buf));
      break;

    case IPS_TS_PROCESSING_ID:
      ips_ts_processing_swap(*((ips_ts_processing_t *) buf));
      break;

    case IPS_TS_STATUS_XML_ID:
      ips_ts_status_xml_swap(*((ips_ts_status_xml_t *) buf));
      break;

    case IPS_TS_CALIBRATION_ID:
      ips_ts_calibration_swap(*((ips_ts_calibration_t *) buf));
      break;

    case IPS_TS_EVENT_NOTICE_ID:
      ips_ts_event_notice_swap(*((ips_ts_event_notice_t *) buf));
      break;

    case IPS_TS_PULSE_HEADER_ID:
      ips_ts_pulse_header_swap(*((ips_ts_pulse_header_t *) buf));
      break;

  }

  return 0;

}

//////////////////////////////////////////////////////
// swap packet header
// returns true is swapped, false if already in native

bool ips_ts_packet_info_swap(ips_ts_packet_info_t &packet)
  
{
  if (ipsIdIsSwapped(packet.id)) {
    SWAP_array_32(&packet.id, 2 * sizeof(si32));
    SWAP_array_64(&packet.seq_num, sizeof(si64));
    SWAP_array_32(&packet.version_num, 2 * sizeof(si32));
    SWAP_array_64(&packet.time_secs_utc, sizeof(si64));
    SWAP_array_32(&packet.time_nano_secs, 6 * sizeof(si32));
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////
// swap version
// returns true is swapped, false if already in native

bool ips_ts_version_swap(ips_ts_version_t &version)

{
  bool swap = ips_ts_packet_info_swap(version.packet);
  if (swap) {
    ui08 *start = (ui08 *) &version + sizeof(ips_ts_packet_info_t);
    ui08 *end = (ui08 *) &version.version_name;
    int nbytes = end - start;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap sync
// returns true is swapped, false if already in native

bool ips_ts_sync_swap(ips_ts_sync_t &sync)

{
  // only swap the header
  // no data to be swapped since all bytes are identical
  return ips_ts_packet_info_swap(sync.packet);
}

//////////////////////////////////////////////////////
// swap radar_info
// returns true is swapped, false if already in native

bool ips_ts_radar_info_swap(ips_ts_radar_info_t &radar_info)

{
  bool swap = ips_ts_packet_info_swap(radar_info.packet);
  if (swap) {
    ui08 *start = (ui08 *) &radar_info + sizeof(ips_ts_packet_info_t);
    ui08 *end = (ui08 *) &radar_info.radar_name;
    int nbytes = end - start;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap scan_segment
// returns true is swapped, false if already in native

bool ips_ts_scan_segment_swap(ips_ts_scan_segment_t &segment)

{
  bool swap = ips_ts_packet_info_swap(segment.packet);
  if (swap) {
    ui08 *start = (ui08 *) &segment + sizeof(ips_ts_packet_info_t);
    ui08 *end = (ui08 *) &segment.segment_name;
    int nbytes = end - start;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap ts_processing
// returns true is swapped, false if already in native

bool ips_ts_processing_swap(ips_ts_processing_t &processing)

{
  bool swap = ips_ts_packet_info_swap(processing.packet);
  if (swap) {
    ui08 *start = (ui08 *) &processing + sizeof(ips_ts_packet_info_t);
    int nbytes = sizeof(ips_ts_processing_t) - sizeof(ips_ts_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap status_xml
// returns true is swapped, false if already in native

bool ips_ts_status_xml_swap(ips_ts_status_xml_t &val)
  
{
  bool swap = ips_ts_packet_info_swap(val.packet);
  if (swap) {
    ui08 *start = (ui08 *) &val + sizeof(ips_ts_packet_info_t);
    int nbytes = sizeof(ips_ts_status_xml_t) - sizeof(ips_ts_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap calibration
// returns true is swapped, false if already in native

bool ips_ts_calibration_swap(ips_ts_calibration_t &calib)

{
  bool swap = ips_ts_packet_info_swap(calib.packet);
  if (swap) {
    ui08 *start = (ui08 *) &calib + sizeof(ips_ts_packet_info_t);
    int nbytes = sizeof(ips_ts_calibration_t)
      - sizeof(ips_ts_packet_info_t) - IPS_TS_MAX_RADAR_NAME;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap event_notice
// returns true is swapped, false if already in native

bool ips_ts_event_notice_swap(ips_ts_event_notice_t &notice)

{
  bool swap = ips_ts_packet_info_swap(notice.packet);
  if (swap) {
    ui08 *start = (ui08 *) &notice + sizeof(ips_ts_packet_info_t);
    int nbytes = sizeof(ips_ts_event_notice_t) - sizeof(ips_ts_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap pulse_header
// returns true is swapped, false if already in native

bool ips_ts_pulse_header_swap(ips_ts_pulse_header_t &pulse)

{
  bool swap = ips_ts_packet_info_swap(pulse.packet);
  if (swap) {
    SWAP_array_64(&pulse.pulse_seq_num, 2 * sizeof(si64));
    int nn = sizeof(ips_ts_packet_info_t) + 2 * sizeof(si64);
    ui08 *start32 = (ui08 *) &pulse + nn;
    int nbytes32 = sizeof(ips_ts_pulse_header_t) - nn;
    SWAP_array_32(start32, nbytes32);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap field_index
// returns true is swapped, false if already in native

bool ips_ts_platform_georef_swap(ips_ts_platform_georef_t &val)
  
{
  bool swap = ips_ts_packet_info_swap(val.packet);
  if (swap) {
    ui08 *start = (ui08 *) &val + sizeof(ips_ts_packet_info_t);
    SWAP_array_32(start, 20 * sizeof(fl32));
    SWAP_array_64(&val.longitude, 2 * sizeof(fl64));
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap field_index
// returns true is swapped, false if already in native

bool ips_ts_georef_correction_swap(ips_ts_georef_correction_t &val)
  
{
  bool swap = ips_ts_packet_info_swap(val.packet);
  if (swap) {
    ui08 *start = (ui08 *) &val + sizeof(ips_ts_packet_info_t);
    int nbytes = sizeof(ips_ts_georef_correction_t) - sizeof(ips_ts_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// string representation of enums

// string representation of packet_id 

string ips_ts_packet_id_to_str(int packet_id)

{

  switch (packet_id) {
    case IPS_TS_SYNC_ID: return "IPS_TS_SYNC_ID";
    case IPS_TS_RADAR_INFO_ID: return "IPS_TS_RADAR_INFO_ID";
    case IPS_TS_SCAN_SEGMENT_ID: return "IPS_TS_SCAN_SEGMENT_ID";
    case IPS_TS_PROCESSING_ID: return "IPS_TS_PROCESSING_ID";
    case IPS_TS_STATUS_XML_ID: return "IPS_TS_STATUS_XML_ID";
    case IPS_TS_CALIBRATION_ID: return "IPS_TS_CALIBRATION_ID";
    case IPS_TS_EVENT_NOTICE_ID: return "IPS_TS_EVENT_NOTICE_ID";
    case IPS_TS_PULSE_HEADER_ID: return "IPS_TS_PULSE_HEADER_ID";
    case IPS_TS_VERSION_ID: return "IPS_TS_VERSION_ID";
    case IPS_TS_PLATFORM_GEOREF_ID: return "IPS_TS_PLATFORM_GEOREF_ID";
    case IPS_TS_GEOREF_CORRECTION_ID: return "IPS_TS_GEOREF_CORRECTION_ID";
    default: return "UNKNOWN";
  }

}

// string representation of prf_mode

string ips_ts_prf_mode_to_str(ips_ts_prf_mode_t prf_mode)

{
  
  switch (prf_mode) {
    case ips_ts_prf_mode_t::FIXED:
      return "FIXED";
    case ips_ts_prf_mode_t::STAGGERED_2_3:
      return "STAGGERED_2_3";
    case ips_ts_prf_mode_t::STAGGERED_3_4:
      return "STAGGERED_3_4";
    case ips_ts_prf_mode_t::STAGGERED_4_5:
      return "STAGGERED_4_5";
    case ips_ts_prf_mode_t::MULTI_PRT:
      return "MULTI_PRT";
    default:
      return "UNKNOWN";
  }

}

string ips_ts_prf_mode_to_str(int prf_mode_int)

{
  ips_ts_prf_mode_t prf_mode = static_cast<ips_ts_prf_mode_t>(prf_mode_int);
  return ips_ts_prf_mode_to_str(prf_mode);
}

// string representation of pulse_shape

string ips_ts_pulse_shape_to_str(ips_ts_pulse_shape_t pulse_shape)
  
{
  
  switch (pulse_shape) {
    case ips_ts_pulse_shape_t::RECT:
      return "RECT";
    case ips_ts_pulse_shape_t::GAUSSIAN:
      return "GAUSSIAN";
    case ips_ts_pulse_shape_t::CUSTOM:
      return "CUSTOM";
    default:
      return "UNKNOWN";
  }

}

string ips_ts_pulse_shape_to_str(int pulse_shape_int)

{
  ips_ts_pulse_shape_t pulse_shape =
    static_cast<ips_ts_pulse_shape_t>(pulse_shape_int);
  return ips_ts_pulse_shape_to_str(pulse_shape);
}

// string representation of pulse_polarization

string ips_ts_pol_mode_to_str(ips_ts_pol_mode_t pol_mode)

{
  
  switch (pol_mode) {
    case ips_ts_pol_mode_t::H:
      return "H";
    case ips_ts_pol_mode_t::V:
      return "V";
    case ips_ts_pol_mode_t::MIXED:
      return "MIXED";
    default:
      return "UNKNOWN";
  }

}

string ips_ts_pol_mode_to_str(int pol_mode_int)

{
  ips_ts_pol_mode_t pol_mode = static_cast<ips_ts_pol_mode_t>(pol_mode_int);
  return ips_ts_pol_mode_to_str(pol_mode);
}

// string representation of scan_mode

string ips_ts_scan_mode_to_str(ips_ts_scan_mode_t scan_mode)

{
  
  switch (scan_mode) {
    case ips_ts_scan_mode_t::PPI:
      return "PPI";
    case ips_ts_scan_mode_t::RHI:
      return "RHI";
    case ips_ts_scan_mode_t::CALIBRATION:
      return "CALIBRATION";
    case ips_ts_scan_mode_t::VPOINT:
      return "VPOINT";
    case ips_ts_scan_mode_t::SUNSCAN:
      return "SUNSCAN";
    case ips_ts_scan_mode_t::POINTING:
      return "POINTING";
    case ips_ts_scan_mode_t::IDLE:
      return "IDLE";
    default: return "UNKNOWN";
  }

}

string ips_ts_scan_mode_to_str(int scan_mode_int)

{
  ips_ts_scan_mode_t scan_mode = static_cast<ips_ts_scan_mode_t>(scan_mode_int);
  return ips_ts_scan_mode_to_str(scan_mode);
}

string ips_ts_scan_mode_to_short_str(ips_ts_scan_mode_t scan_mode)

{
  switch (scan_mode) {
    case ips_ts_scan_mode_t::PPI:
      return "PPI";
    case ips_ts_scan_mode_t::RHI:
      return "RHI";
    case ips_ts_scan_mode_t::CALIBRATION:
      return "CAL";
    case ips_ts_scan_mode_t::VPOINT:
      return "VPOINT";
    case ips_ts_scan_mode_t::SUNSCAN:
      return "SUN";
    case ips_ts_scan_mode_t::POINTING:
      return "POINT";
    case ips_ts_scan_mode_t::IDLE:
      return "IDLE";
    default:
      return "UNKNOWN";
  }

}

string ips_ts_scan_mode_to_short_str(int scan_mode_int)
{
  ips_ts_scan_mode_t scan_mode = static_cast<ips_ts_scan_mode_t>(scan_mode_int);
  return ips_ts_scan_mode_to_short_str(scan_mode);
}

// string representation of radar_platform

string ips_ts_radar_platform_to_str(ips_ts_radar_platform_t radar_platform)

{
  
  switch (radar_platform) {
    case ips_ts_radar_platform_t::FIXED:
      return "FIXED";
    case ips_ts_radar_platform_t::VEHICLE:
      return "VEHICLE";
    case ips_ts_radar_platform_t::SHIP:
      return "SHIP";
    case ips_ts_radar_platform_t::AIRCRAFT:
      return "AIRCRAFT";
    default:
      return "UNKNOWN";
  }

}

string ips_ts_radar_platform_to_str(int radar_platform_int)

{
  ips_ts_radar_platform_t radar_platform =
    static_cast<ips_ts_radar_platform_t>(radar_platform_int);
  return ips_ts_radar_platform_to_str(radar_platform);
}

// string representation of iq_encoding

string ips_ts_iq_encoding_to_str(ips_ts_iq_encoding_t iq_encoding)

{
  
  switch (iq_encoding) {
    case ips_ts_iq_encoding_t::FL32:
      return "FL32";
    case ips_ts_iq_encoding_t::SCALED_SI16:
      return "SCALED_SI16";
    case ips_ts_iq_encoding_t::DBM_PHASE_SI16:
      return "DBM_PHASE_SI16";
    case ips_ts_iq_encoding_t::SCALED_SI32:
      return "SCALED_SI32";
    default:
      return "UNKNOWN";
  }

}

string ips_ts_iq_encoding_to_str(int iq_encoding_int)

{
  ips_ts_iq_encoding_t iq_encoding = static_cast<ips_ts_iq_encoding_t>(iq_encoding_int);
  return ips_ts_iq_encoding_to_str(iq_encoding);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// printing routines

////////////////////////////////
// print depending on packet type

void ips_ts_packet_print(FILE *out, const void *buf, int len)

{
  
  int packet_id;
  if (ips_ts_get_packet_id(buf, len, packet_id)) {
    return;
  }

  switch (packet_id) {

    case IPS_TS_SYNC_ID:
      ips_ts_sync_print(out, *((ips_ts_sync_t *) buf));
      break;

    case IPS_TS_RADAR_INFO_ID:
      ips_ts_radar_info_print(out, *((ips_ts_radar_info_t *) buf));
      break;

    case IPS_TS_SCAN_SEGMENT_ID:
      ips_ts_scan_segment_print(out, *((ips_ts_scan_segment_t *) buf));
      break;

    case IPS_TS_PROCESSING_ID:
      ips_ts_processing_print(out, *((ips_ts_processing_t *) buf));
      break;

    case IPS_TS_STATUS_XML_ID:
      ips_ts_status_xml_print(out, buf);
      break;

    case IPS_TS_CALIBRATION_ID:
      ips_ts_calibration_print(out, *((ips_ts_calibration_t *) buf));
      break;

    case IPS_TS_EVENT_NOTICE_ID:
      ips_ts_event_notice_print(out, *((ips_ts_event_notice_t *) buf));
      break;

    case IPS_TS_PULSE_HEADER_ID:
      ips_ts_pulse_header_print(out, *((ips_ts_pulse_header_t *) buf));
      break;

    case IPS_TS_PLATFORM_GEOREF_ID:
      ips_ts_platform_georef_print(out, *((ips_ts_platform_georef_t *) buf));
      break;
      
    case IPS_TS_GEOREF_CORRECTION_ID:
      ips_ts_georef_correction_print(out, *((ips_ts_georef_correction_t *) buf));
      break;
      
  }

}

//////////////////////////////////////////////////////
// print packet header

void ips_ts_packet_info_print(FILE *out,
                              const ips_ts_packet_info_t &packet)

{

  ips_ts_packet_info_t copy = packet;
  ips_ts_packet_info_swap(copy);
  fprintf(out, "  id: 0x%x (%d)\n", copy.id, copy.id);
  fprintf(out, "  len_bytes: %d\n", copy.len_bytes);
  fprintf(out, "  seq_num: %lld\n", (long long) copy.seq_num);
  fprintf(out, "  version_num: %d\n", copy.version_num);
  fprintf(out, "  radar_id: %d\n", copy.radar_id);
  fprintf(out, "  time_secs_utc: %lld\n", (long long) copy.time_secs_utc);
  fprintf(out, "  time_nano_secs: %d\n", copy.time_nano_secs);
  
  time_t ptime = copy.time_secs_utc;
  fprintf(out, "  time UTC: %s.%.9d\n",
	  DateTime::strm(ptime).c_str(), copy.time_nano_secs);

}

//////////////////////////////////////////////////////
// print sync packet

void ips_ts_sync_print(FILE *out,
                       const ips_ts_sync_t &sync)

{

  ips_ts_sync_t copy = sync;
  ips_ts_sync_swap(copy);

  fprintf(out, "==================== ips_ts_sync ==================================\n");
  ips_ts_packet_info_print(out, copy.packet);
  fprintf(out, "  magik[0]: 0x%x\n", copy.magik[0]);
  fprintf(out, "  magik[1]: 0x%x\n", copy.magik[1]);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print version packet

void ips_ts_version_print(FILE *out,
                          const ips_ts_version_t &version)

{

  ips_ts_version_t copy = version;
  ips_ts_version_swap(copy);
  fprintf(out, "==================== ips_ts_version ===============================\n");
  ips_ts_packet_info_print(out, copy.packet);
  fprintf(out, "  major_version_num: %d\n", copy.major_version_num);
  fprintf(out, "  minor_version_num: %d\n", copy.minor_version_num);
  fprintf(out, "  version_name: %s\n", copy.version_name);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print radar_info

void ips_ts_radar_info_print(FILE *out,
                             const ips_ts_radar_info_t &info)

{

  ips_ts_radar_info_t copy = info;
  ips_ts_radar_info_swap(copy);
  fprintf(out, "==================== ips_ts_radar_info ============================\n");
  ips_ts_packet_info_print(out, copy.packet);

  fprintf(out, "  latitude_deg: %g\n", copy.latitude_deg);
  fprintf(out, "  longitude_deg: %g\n", copy.longitude_deg);
  fprintf(out, "  altitude_m: %g\n", copy.altitude_m);
  fprintf(out, "  platform_type: %s\n",
	  ips_ts_radar_platform_to_str(copy.platform_type).c_str());
  fprintf(out, "  beamwidth_deg_h: %g\n", copy.beamwidth_deg_h);
  fprintf(out, "  beamwidth_deg_v: %g\n", copy.beamwidth_deg_v);
  fprintf(out, "  wavelength_cm: %g\n", copy.wavelength_cm);
  fprintf(out, "  nominal_gain_ant_db_h: %g\n", copy.nominal_gain_ant_db_h);
  fprintf(out, "  nominal_gain_ant_db_v: %g\n", copy.nominal_gain_ant_db_v);
  fprintf(out, "  radar_name: %s\n",
	  ips_ts_safe_str(copy.radar_name, IPS_TS_MAX_RADAR_NAME).c_str());
  fprintf(out, "  site_name: %s\n",
	  ips_ts_safe_str(copy.site_name, IPS_TS_MAX_SITE_NAME).c_str());
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print scan_segment

void ips_ts_scan_segment_print(FILE *out,
                               const ips_ts_scan_segment_t &seg)

{

  ips_ts_scan_segment_t copy = seg;
  ips_ts_scan_segment_swap(copy);
  fprintf(out, "==================== ips_ts_scan_segment ==========================\n");
  ips_ts_packet_info_print(out, copy.packet);

  fprintf(out, "  scan_mode: %s\n", ips_ts_scan_mode_to_str(copy.scan_mode).c_str());
  fprintf(out, "  volume_num: %d\n", copy.volume_num);
  fprintf(out, "  sweep_num: %d\n", copy.sweep_num);
  fprintf(out, "  az_start: %g\n", copy.az_start);
  fprintf(out, "  el_start: %g\n", copy.el_start);
  fprintf(out, "  scan_rate: %g\n", copy.scan_rate);
  fprintf(out, "  left_limit: %g\n", copy.left_limit);
  fprintf(out, "  right_limit: %g\n", copy.right_limit);
  fprintf(out, "  up_limit: %g\n", copy.up_limit);
  fprintf(out, "  down_limit: %g\n", copy.down_limit);
  fprintf(out, "  step: %g\n", copy.step);
  fprintf(out, "  current_fixed_angle: %g\n", copy.current_fixed_angle);
  fprintf(out, "  n_sweeps: %d\n", copy.n_sweeps);

  fprintf(out, "  fixed_angles:");
  int nSweeps = copy.n_sweeps;
  if (nSweeps > IPS_TS_MAX_FIXED_ANGLES) {
    fprintf(out, " WARNING - bad number of sweeps: %d\n", nSweeps);
    nSweeps = IPS_TS_MAX_FIXED_ANGLES;
  } else {
    for (int ii = 0; ii < nSweeps; ii++) {
      fprintf(out, " %g", copy.fixed_angles[ii]);
    }
    fprintf(out, "\n");
  }

  fprintf(out, "  sun_scan_sector_width_az: %g\n", copy.sun_scan_sector_width_az);
  fprintf(out, "  sun_scan_sector_width_el: %g\n", copy.sun_scan_sector_width_el);

  fprintf(out, "  segment_name: %s\n",
	  ips_ts_safe_str(copy.segment_name, IPS_TS_MAX_SEGMENT_NAME).c_str());
  fprintf(out, "  project_name: %s\n",
	  ips_ts_safe_str(copy.project_name, IPS_TS_MAX_PROJECT_NAME).c_str());
  
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print ts_processing

void ips_ts_processing_print(FILE *out,
                             const ips_ts_processing_t &proc)

{

  ips_ts_processing_t copy = proc;
  ips_ts_processing_swap(copy);
  fprintf(out, "==================== ips_ts_processing =========================\n");
  ips_ts_packet_info_print(out, copy.packet);

  fprintf(out, "  pol_mode: %s\n",
          ips_ts_pol_mode_to_str(copy.pol_mode).c_str());
  fprintf(out, "  prf_mode: %s\n",
          ips_ts_prf_mode_to_str(copy.prf_mode).c_str());
  fprintf(out, "  pulse_shape: %s\n",
          ips_ts_pulse_shape_to_str(copy.pulse_shape).c_str());
  
  fprintf(out, "  pulse_width_us: %g\n", copy.pulse_width_us);
  fprintf(out, "  start_range_m: %g\n", copy.start_range_m);
  fprintf(out, "  gate_spacing_m: %g\n", copy.gate_spacing_m);

  fprintf(out, "  test_pulse_range_km: %g\n", copy.test_pulse_range_km);
  fprintf(out, "  test_pulse_length_us: %g\n", copy.test_pulse_length_us);

  fprintf(out, "  num_prts: %d\n", copy.num_prts);
  for (int ii = 0; ii < copy.num_prts; ii++) {
    fprintf(out, "  prt_us[%d]: %g\n", ii, copy.prt_us[ii]);
  }


}

//////////////////////////////////////////////////////
// print status_xml

void ips_ts_status_xml_print(FILE *out,
                             const ips_ts_status_xml_t &val,
                             const string &xmlStr)
  
{
  
  ips_ts_status_xml_t copy = val;
  ips_ts_status_xml_swap(copy);
  fprintf(out, "===================== ips_ts_status_xml ===========================\n");
  ips_ts_packet_info_print(out, copy.packet);
  fprintf(out, "  xml_len: %d\n", copy.xml_len);
  if (copy.xml_len > (int) sizeof(copy) + 1) {
    fprintf(out, "%s\n", xmlStr.c_str());
  }
  fprintf(out, "=================================================================\n");
  
}

void ips_ts_status_xml_print(FILE *out, const void *buf)
{
  ips_ts_status_xml_t hdr;
  memcpy(&hdr, buf, sizeof(ips_ts_status_xml_t));
  const char *str = (const char *) buf + sizeof(ips_ts_status_xml_t);
  string statusStr(str);
  ips_ts_status_xml_print(out, hdr, statusStr);
}

//////////////////////////////////////////////////////
// print calibration

void ips_ts_calibration_print(FILE *out,
                              const ips_ts_calibration_t &calib)
  
{

  ips_ts_calibration_t copy = calib;
  ips_ts_calibration_swap(copy);
  fprintf(out, "==================== ips_ts_calibration ===========================\n");
  ips_ts_packet_info_print(out, copy.packet);
  
  fprintf(out, "  radar_name: %s\n", copy.radar_name);
  fprintf(out, "  wavelength_cm: %g\n", copy.wavelength_cm);
  fprintf(out, "  beamwidth_deg_h: %g\n", copy.beamwidth_deg_h);
  fprintf(out, "  beamwidth_deg_v: %g\n", copy.beamwidth_deg_v);
  fprintf(out, "  gain_ant_db_h: %g\n", copy.gain_ant_db_h);
  fprintf(out, "  gain_ant_db_v: %g\n", copy.gain_ant_db_v);
  fprintf(out, "  pulse_width_us: %g\n", copy.pulse_width_us);
  fprintf(out, "  xmit_power_dbm_h: %g\n", copy.xmit_power_dbm_h);
  fprintf(out, "  xmit_power_dbm_v: %g\n", copy.xmit_power_dbm_v);
  fprintf(out, "  two_way_waveguide_loss_db_h: %g\n", copy.two_way_waveguide_loss_db_h);
  fprintf(out, "  two_way_waveguide_loss_db_v: %g\n", copy.two_way_waveguide_loss_db_v);
  fprintf(out, "  two_way_radome_loss_db_h: %g\n", copy.two_way_radome_loss_db_h);
  fprintf(out, "  two_way_radome_loss_db_v: %g\n", copy.two_way_radome_loss_db_v);
  fprintf(out, "  receiver_mismatch_loss_db: %g\n", copy.receiver_mismatch_loss_db);
  fprintf(out, "  k_squared_water: %g\n", copy.k_squared_water);
  fprintf(out, "  radar_constant_h: %g\n", copy.radar_constant_h);
  fprintf(out, "  radar_constant_v: %g\n", copy.radar_constant_v);
  fprintf(out, "  noise_dbm_hc: %g\n", copy.noise_dbm_hc);
  fprintf(out, "  noise_dbm_hx: %g\n", copy.noise_dbm_hx);
  fprintf(out, "  noise_dbm_vc: %g\n", copy.noise_dbm_vc);
  fprintf(out, "  noise_dbm_vx: %g\n", copy.noise_dbm_vx);
  fprintf(out, "  i0_dbm_hc: %g\n", copy.i0_dbm_hc);
  fprintf(out, "  i0_dbm_hx: %g\n", copy.i0_dbm_hx);
  fprintf(out, "  i0_dbm_vc: %g\n", copy.i0_dbm_vc);
  fprintf(out, "  i0_dbm_vx: %g\n", copy.i0_dbm_vx);
  fprintf(out, "  receiver_gain_db_hc: %g\n", copy.receiver_gain_db_hc);
  fprintf(out, "  receiver_gain_db_hx: %g\n", copy.receiver_gain_db_hx);
  fprintf(out, "  receiver_gain_db_vc: %g\n", copy.receiver_gain_db_vc);
  fprintf(out, "  receiver_gain_db_vx: %g\n", copy.receiver_gain_db_vx);
  fprintf(out, "  receiver_slope_hc: %g\n", copy.receiver_slope_hc);
  fprintf(out, "  receiver_slope_hx: %g\n", copy.receiver_slope_hx);
  fprintf(out, "  receiver_slope_vc: %g\n", copy.receiver_slope_vc);
  fprintf(out, "  receiver_slope_vx: %g\n", copy.receiver_slope_vx);
  fprintf(out, "  dynamic_range_db_hc: %g\n", copy.dynamic_range_db_hc);
  fprintf(out, "  dynamic_range_db_hx: %g\n", copy.dynamic_range_db_hx);
  fprintf(out, "  dynamic_range_db_vc: %g\n", copy.dynamic_range_db_vc);
  fprintf(out, "  dynamic_range_db_vx: %g\n", copy.dynamic_range_db_vx);
  fprintf(out, "  base_dbz_1km_hc: %g\n", copy.base_dbz_1km_hc);
  fprintf(out, "  base_dbz_1km_hx: %g\n", copy.base_dbz_1km_hx);
  fprintf(out, "  base_dbz_1km_vc: %g\n", copy.base_dbz_1km_vc);
  fprintf(out, "  base_dbz_1km_vx: %g\n", copy.base_dbz_1km_vx);
  fprintf(out, "  sun_power_dbm_hc: %g\n", copy.sun_power_dbm_hc);
  fprintf(out, "  sun_power_dbm_hx: %g\n", copy.sun_power_dbm_hx);
  fprintf(out, "  sun_power_dbm_vc: %g\n", copy.sun_power_dbm_vc);
  fprintf(out, "  sun_power_dbm_vx: %g\n", copy.sun_power_dbm_vx);
  fprintf(out, "  noise_source_power_dbm_h: %g\n", copy.noise_source_power_dbm_h);
  fprintf(out, "  noise_source_power_dbm_v: %g\n", copy.noise_source_power_dbm_v);
  fprintf(out, "  power_meas_loss_db_h: %g\n", copy.power_meas_loss_db_h);
  fprintf(out, "  power_meas_loss_db_v: %g\n", copy.power_meas_loss_db_v);
  fprintf(out, "  coupler_forward_loss_db_h: %g\n", copy.coupler_forward_loss_db_h);
  fprintf(out, "  coupler_forward_loss_db_v: %g\n", copy.coupler_forward_loss_db_v);
  fprintf(out, "  test_power_dbm_h: %g\n", copy.test_power_dbm_h);
  fprintf(out, "  test_power_dbm_v: %g\n", copy.test_power_dbm_v);
  fprintf(out, "  zdr_correction_db: %g\n", copy.zdr_correction_db);
  fprintf(out, "  ldr_correction_db_h: %g\n", copy.ldr_correction_db_h);
  fprintf(out, "  ldr_correction_db_v: %g\n", copy.ldr_correction_db_v);
  fprintf(out, "  phidp_rot_deg: %g\n", copy.phidp_rot_deg);
  fprintf(out, "  dbz_correction: %g\n", copy.dbz_correction);
  fprintf(out, "=================================================================\n");
  
}

//////////////////////////////////////////////////////
// print event_notice

void ips_ts_event_notice_print(FILE *out,
                               const ips_ts_event_notice_t &note)

{

  ips_ts_event_notice_t copy = note;
  ips_ts_event_notice_swap(copy);
  fprintf(out, "==================== ips_ts_event_notice ==========================\n");
  ips_ts_packet_info_print(out, copy.packet);
  
  fprintf(out, "  start_of_sweep: %d\n", copy.start_of_sweep);
  fprintf(out, "  end_of_sweep: %d\n", copy.end_of_sweep);
  fprintf(out, "  start_of_volume: %d\n", copy.start_of_volume);
  fprintf(out, "  end_of_volume: %d\n", copy.end_of_volume);
  fprintf(out, "  scan_mode: %s\n", ips_ts_scan_mode_to_str(copy.scan_mode).c_str());
  fprintf(out, "  volume_num: %d\n", copy.volume_num);
  fprintf(out, "  sweep_num: %d\n", copy.sweep_num);
  fprintf(out, "  current_fixed_angle: %g\n", copy.current_fixed_angle);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print pulse_header

void ips_ts_pulse_header_print(FILE *out,
                               const ips_ts_pulse_header_t &pulse,
                               const ips_ts_platform_georef_t *georef /* = NULL*/)

{
  
  ips_ts_pulse_header_t copy = pulse;
  ips_ts_pulse_header_swap(copy);
  fprintf(out, "==================== ips_ts_pulse_header ==========================\n");
  ips_ts_packet_info_print(out, copy.packet);
  
  fprintf(out, "  pulse_seq_num: %lld\n", (long long) copy.pulse_seq_num);
  fprintf(out, "  dwell_seq_num: %lld\n", (long long) copy.dwell_seq_num);

  fprintf(out, "  beam_num_in_dwell: %d\n", copy.beam_num_in_dwell);
  fprintf(out, "  visit_num_in_beam: %d\n", copy.visit_num_in_beam);

  fprintf(out, "  scan_mode: %s\n", ips_ts_scan_mode_to_str(copy.scan_mode).c_str());
  fprintf(out, "  volume_num: %d\n", copy.volume_num);
  fprintf(out, "  sweep_num: %d\n", copy.sweep_num);

  fprintf(out, "  elevation: %g\n", copy.elevation);
  fprintf(out, "  azimuth: %g\n", copy.azimuth);
  fprintf(out, "  fixed_angle: %g\n", copy.fixed_angle);

  fprintf(out, "  prt: %g\n", copy.prt);
  fprintf(out, "  prt_next: %g\n", copy.prt_next);
  fprintf(out, "  pulse_width_us: %g\n", copy.pulse_width_us);

  fprintf(out, "  n_gates: %d\n", copy.n_gates);
  fprintf(out, "  start_range_m: %g\n", copy.start_range_m);
  fprintf(out, "  gate_spacing_m: %g\n", copy.gate_spacing_m);

  fprintf(out, "  hv_flag: %d\n", copy.hv_flag);
  fprintf(out, "  phase_cohered: %d\n", copy.phase_cohered);

  fprintf(out, "  iq_encoding: %s\n",
          ips_ts_iq_encoding_to_str(copy.iq_encoding).c_str());  
  fprintf(out, "  n_channels: %d\n", copy.n_channels);
  fprintf(out, "  n_data: %d\n", copy.n_data);
  fprintf(out, "  scale: %g\n", copy.scale);
  fprintf(out, "  offset: %g\n", copy.offset);

  for (int ii = 0; ii < copy.n_channels; ii++) {
    fprintf(out, "  chan_is_copol[%d]: %d\n", ii, copy.chan_is_copol[ii]);
  }

  fprintf(out, "  status: %d\n", copy.status);
  
  if (copy.event_flags & IPS_TS_END_OF_SWEEP) {
    fprintf(out, "  event: end_of_sweep\n");
  }
  if (copy.event_flags & IPS_TS_START_OF_SWEEP) {
    fprintf(out, "  event: start_of_sweep\n");
  }
  if (copy.event_flags & IPS_TS_END_OF_VOLUME) {
    fprintf(out, "  event: end_of_volume\n");
  }
  if (copy.event_flags & IPS_TS_START_OF_VOLUME) {
    fprintf(out, "  event: start_of_volume\n");
  }

  if (georef != NULL) {
    ips_ts_platform_georef_t gcopy = *georef;
    ips_ts_platform_georef_swap(gcopy);
    fprintf(out, "  Pulse is using georef:\n");
    fprintf(out, "    georef time_secs_utc: %lld\n",
            (long long) gcopy.packet.time_secs_utc);
    fprintf(out, "    georef time_nano_secs: %d\n", gcopy.packet.time_nano_secs);
    fprintf(out, "    georef unit_num: %d\n", gcopy.unit_num);
    fprintf(out, "    georef unit_id: %d\n", gcopy.unit_id);
  }
  
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print platform_georef

void ips_ts_platform_georef_print(FILE *out,
                                  const ips_ts_platform_georef &val)

{
  
  ips_ts_platform_georef_t copy = val;
  ips_ts_platform_georef_swap(copy);
  fprintf(out, "====================== ips_ts_platform_georef =====================\n");
  ips_ts_packet_info_print(out, copy.packet);
  fprintf(out, "  longitude: %g\n", copy.longitude);
  fprintf(out, "  latitude: %g\n", copy.latitude);
  fprintf(out, "  unit_num: %d\n", copy.unit_num);
  fprintf(out, "  unit_id: %d\n", copy.unit_id);
  fprintf(out, "  altitude_msl_km: %g\n", copy.altitude_msl_km);
  fprintf(out, "  altitude_agl_km: %g\n", copy.altitude_agl_km);
  fprintf(out, "  ew_velocity_mps: %g\n", copy.ew_velocity_mps);
  fprintf(out, "  ns_velocity_mps: %g\n", copy.ns_velocity_mps);
  fprintf(out, "  vert_velocity_mps: %g\n", copy.vert_velocity_mps);
  fprintf(out, "  heading_deg: %g\n", copy.heading_deg);
  fprintf(out, "  track_deg: %g\n", copy.track_deg);
  fprintf(out, "  roll_deg: %g\n", copy.roll_deg);
  fprintf(out, "  pitch_deg: %g\n", copy.pitch_deg);
  fprintf(out, "  drift_angle_deg: %g\n", copy.drift_angle_deg);
  fprintf(out, "  rotation_angle_deg: %g\n", copy.rotation_angle_deg);
  fprintf(out, "  tilt_deg: %g\n", copy.tilt_deg);
  fprintf(out, "  ew_horiz_wind_mps: %g\n", copy.ew_horiz_wind_mps);
  fprintf(out, "  ns_horiz_wind_mps: %g\n", copy.ns_horiz_wind_mps);
  fprintf(out, "  vert_wind_mps: %g\n", copy.vert_wind_mps);
  fprintf(out, "  heading_rate_dps: %g\n", copy.heading_rate_dps);
  fprintf(out, "  pitch_rate_dps: %g\n", copy.pitch_rate_dps);
  fprintf(out, "  roll_rate_dps: %g\n", copy.roll_rate_dps);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print georef_correction

void ips_ts_georef_correction_print(FILE *out,
                                    const ips_ts_georef_correction &val)

{

  ips_ts_georef_correction_t copy = val;
  ips_ts_georef_correction_swap(copy);
  fprintf(out, "==================== ips_ts_georef_correction =====================\n");
  fprintf(out, "  longitude_corr_deg: %g\n", copy.longitude_corr_deg);
  fprintf(out, "  latitude_corr_deg: %g\n", copy.latitude_corr_deg);
  fprintf(out, "  azimuth_corr_deg: %g\n", copy.azimuth_corr_deg);
  fprintf(out, "  elevation_corr_deg: %g\n", copy.elevation_corr_deg);
  fprintf(out, "  range_delay_corr_mps: %g\n", copy.range_delay_corr_mps);
  fprintf(out, "  pressure_alt_corr_km: %g\n", copy.pressure_alt_corr_km);
  fprintf(out, "  radar_alt_corr_km: %g\n", copy.radar_alt_corr_km);
  fprintf(out, "  ew_gndspd_corr_mps: %g\n", copy.ew_gndspd_corr_mps);
  fprintf(out, "  ns_gndspd_corr_mps: %g\n", copy.ns_gndspd_corr_mps);
  fprintf(out, "  vert_vel_corr_mps: %g\n", copy.vert_vel_corr_mps);
  fprintf(out, "  heading_corr_deg: %g\n", copy.heading_corr_deg);
  fprintf(out, "  roll_corr_deg: %g\n", copy.roll_corr_deg);
  fprintf(out, "  pitch_corr_deg: %g\n", copy.pitch_corr_deg);
  fprintf(out, "  drift_corr_deg: %g\n", copy.drift_corr_deg);
  fprintf(out, "  rot_angle_corr_deg: %g\n", copy.rot_angle_corr_deg);
  fprintf(out, "  tilt_corr_deg: %g\n", copy.tilt_corr_deg);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// Return a string formed safely from a char* array
// Null-termination of the input string is guaranteed.

string ips_ts_safe_str(const char *str, int maxLen)

{

  char *safechar = new char[maxLen + 1];
  memcpy(safechar, str, maxLen);
  safechar[maxLen] = '\0';
  string safestr(safechar);
  delete[] safechar;

  return safestr;

}

string ips_ts_time_str(const time_t *ptime, si32 *nano_secs)
{

  char str1[30]={"                             "};
  char str2[50];
  
  strncpy(str1,asctime(gmtime(ptime)),29);
  strncpy(str2, str1+4, 6);	// Month Day	
  strncpy(str2+6, str1+19, 5);	// Year
  *(str2+11)= ' ';
  strncpy(str2+12, str1+11, 8);	// HH:MM:SS
  *(str2+20)= '.';
  *(str2+21)= 0;
  if(nano_secs) sprintf(str2+21, "%.9d", *nano_secs);	// fractions of sec
  return str2;
}

////////////////////////////////////////////////////////
/// Print format for IPS structs

static const char *_hform = "%9s %30s %7s %7s\n";
static const char *_dform = "%9s %30s %7d %7d\n";
static const int _formDividerLen = 56;

static void _print_format_divider(char val, FILE *out)
{
  for (int ii = 0; ii < _formDividerLen; ii++) {
    fprintf(out, "%c", val);
  }
  fprintf(out, "\n");
}
  
////////////////////////////////////////////////////////
/// Print format of all IPS structs

static void _print_format_header(FILE *out)
{
  fprintf(out, _hform, "type", "name", "size", "offset");
  fprintf(out, _hform, "----", "----", "----", "------");
}

void ips_ts_print_all_formats(FILE *out)
{

  fprintf(out, "============= IPS TIME SERIES FORMAT ==================\n");

  {
    ips_ts_packet_info_t val;
    ips_ts_packet_info_print_format(out, val);
  }

  {
    ips_ts_sync_t val;
    ips_ts_sync_print_format(out, val);
  }

  {
    ips_ts_version_t val;
    ips_ts_version_print_format(out, val);
  }

  {
    ips_ts_radar_info_t val;
    ips_ts_radar_info_print_format(out, val);
  }
  
  {
    ips_ts_scan_segment_t val;
    ips_ts_scan_segment_print_format(out, val);
  }
  
  {
    ips_ts_processing_t val;
    ips_ts_processing_print_format(out, val);
  }
  
  {
    ips_ts_status_xml_t val;
    ips_ts_status_xml_print_format(out, val);
  }
  
  {
    ips_ts_calibration_t val;
    ips_ts_calibration_print_format(out, val);
  }
  
  {
    ips_ts_event_notice_t val;
    ips_ts_event_notice_print_format(out, val);
  }
  
  {
    ips_ts_pulse_header_t val;
    ips_ts_pulse_header_print_format(out, val);
  }
  
  {
    ips_ts_platform_georef_t val;
    ips_ts_platform_georef_print_format(out, val);
  }
  
  {
    ips_ts_georef_correction_t val;
    ips_ts_georef_correction_print_format(out, val);
  }
  
  _print_format_divider('=', out);

}

// print basic packet info

void _print_packet_format(FILE *out,
                          const ips_ts_packet_info_t &pkt)

{
  
  const char *id = (char *) &pkt.id;
  fprintf(out, _dform, "si32", "id", sizeof(pkt.id), (char *) &pkt.id - id);
  fprintf(out, _dform, "si32", "len_bytes", sizeof(pkt.len_bytes), (char *) &pkt.len_bytes - id);
  fprintf(out, _dform, "si64", "seq_num", sizeof(pkt.seq_num), (char *) &pkt.seq_num - id);
  fprintf(out, _dform, "si32", "version_num", sizeof(pkt.version_num), (char *) &pkt.version_num - id);
  fprintf(out, _dform, "si32", "radar_id", sizeof(pkt.radar_id), (char *) &pkt.radar_id - id);
  fprintf(out, _dform, "si64", "time_secs_utc", sizeof(pkt.time_secs_utc), (char *) &pkt.time_secs_utc - id);
  fprintf(out, _dform, "si32", "time_nano_secs", sizeof(pkt.time_nano_secs), (char *) &pkt.time_nano_secs - id);
  char tmpStr[1024];
  snprintf(tmpStr, 1024, "reserved[%d]", (int) (sizeof(pkt.reserved) / sizeof(si32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(pkt.reserved), (char *) pkt.reserved - id);

}

// print format of packet info

void ips_ts_packet_info_print_format(FILE *out,
                                     const ips_ts_packet_info_t &val)

{
  
  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_packet_info_t'\n");
  fprintf(out, "  included in all packet types\n");
  fprintf(out, "  size: %d\n\n", (int) sizeof(val));
  _print_format_header(out);
  _print_packet_format(out, val);
  _print_format_divider('-', out);

}


// print format of sync packet

void ips_ts_sync_print_format(FILE *out, const ips_ts_sync_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_sync_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IPS_TS_SYNC_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  char tmpStr[1024];
  snprintf(tmpStr, 1024, "magik[%d]", (int) (sizeof(val.magik) / sizeof(si32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.magik), (char *) val.magik - id);
  
  _print_format_divider('-', out);

}

// print format of version packet

void ips_ts_version_print_format(FILE *out,
                                 const ips_ts_version_t &val)

{

  _print_format_divider('-', out);

  fprintf(out, "  struct: 'ips_ts_version_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IPS_TS_VERSION_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  fprintf(out, _dform, "si32", "major_version_num", sizeof(val.major_version_num), (char *) &val.major_version_num - id);
  fprintf(out, _dform, "si32", "minor_version_num", sizeof(val.minor_version_num), (char *) &val.minor_version_num - id);
  char tmpStr[1024];
  snprintf(tmpStr, 1024, "version_name[%d]", (int) sizeof(val.version_name));
  fprintf(out, _dform, "char", tmpStr, sizeof(val.version_name), (char *) val.version_name - id);

  _print_format_divider('-', out);

}

// print format of radar_info

void ips_ts_radar_info_print_format(FILE *out, const ips_ts_radar_info_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_radar_info_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IPS_TS_RADAR_INFO_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;

  fprintf(out, _dform, "fl64", "latitude_deg", sizeof(val.latitude_deg), (char *) &val.latitude_deg - id);
  fprintf(out, _dform, "fl64", "longitude_deg", sizeof(val.longitude_deg), (char *) &val.longitude_deg - id);
  fprintf(out, _dform, "fl32", "altitude_m", sizeof(val.altitude_m), (char *) &val.altitude_m - id);
  fprintf(out, _dform, "si32", "platform_type", sizeof(val.platform_type), (char *) &val.platform_type - id);
  fprintf(out, _dform, "fl32", "beamwidth_deg_h", sizeof(val.beamwidth_deg_h), (char *) &val.beamwidth_deg_h - id);
  fprintf(out, _dform, "fl32", "beamwidth_deg_v", sizeof(val.beamwidth_deg_v), (char *) &val.beamwidth_deg_v - id);
  fprintf(out, _dform, "fl32", "wavelength_cm", sizeof(val.wavelength_cm), (char *) &val.wavelength_cm - id);
  fprintf(out, _dform, "fl32", "nominal_gain_ant_db_h", sizeof(val.nominal_gain_ant_db_h), (char *) &val.nominal_gain_ant_db_h - id);
  fprintf(out, _dform, "fl32", "nominal_gain_ant_db_v", sizeof(val.nominal_gain_ant_db_v), (char *) &val.nominal_gain_ant_db_v - id);
  char tmpStr[1024];
  snprintf(tmpStr, 1024, "unused[%d]", (int) (sizeof(val.unused) / sizeof(fl32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.unused), (char *) val.unused - id);
  snprintf(tmpStr, 1024, "radar_name[%d]", (int) sizeof(val.radar_name));
  fprintf(out, _dform, "char", tmpStr, sizeof(val.radar_name), (char *) val.radar_name - id);
  snprintf(tmpStr, 1024, "site_name[%d]", (int) sizeof(val.site_name));
  fprintf(out, _dform, "char", tmpStr, sizeof(val.site_name), (char *) val.site_name - id);

  _print_format_divider('-', out);

}

// print format of scan_segment

void ips_ts_scan_segment_print_format(FILE *out, const ips_ts_scan_segment_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_scan_segment_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IPS_TS_SCAN_SEGMENT_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "scan_mode", sizeof(val.scan_mode), (char *) &val.scan_mode - id);
  fprintf(out, _dform, "si32", "volume_num", sizeof(val.volume_num), (char *) &val.volume_num - id);
  fprintf(out, _dform, "si32", "sweep_num", sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "fl32", "az_start", sizeof(val.az_start), (char *) &val.az_start - id);
  fprintf(out, _dform, "fl32", "el_start", sizeof(val.el_start), (char *) &val.el_start - id);
  fprintf(out, _dform, "fl32", "scan_rate", sizeof(val.scan_rate), (char *) &val.scan_rate - id);
  fprintf(out, _dform, "fl32", "left_limit", sizeof(val.left_limit), (char *) &val.left_limit - id);
  fprintf(out, _dform, "fl32", "right_limit", sizeof(val.right_limit), (char *) &val.right_limit - id);
  fprintf(out, _dform, "fl32", "up_limit", sizeof(val.up_limit), (char *) &val.up_limit - id);
  fprintf(out, _dform, "fl32", "down_limit", sizeof(val.down_limit), (char *) &val.down_limit - id);
  fprintf(out, _dform, "fl32", "step", sizeof(val.step), (char *) &val.step - id);
  fprintf(out, _dform, "fl32", "current_fixed_angle", sizeof(val.current_fixed_angle), (char *) &val.current_fixed_angle - id);
  fprintf(out, _dform, "si32", "n_sweeps", sizeof(val.n_sweeps), (char *) &val.n_sweeps - id);

  char tmpStr[1024];
  snprintf(tmpStr, 1024, "fixed_angles[%d]", (int) (sizeof(val.fixed_angles) / sizeof(fl32)));
  fprintf(out, _dform, "fl32", tmpStr, sizeof(val.fixed_angles), (char *) val.fixed_angles - id);

  fprintf(out, _dform, "fl32", "sun_scan_sector_width_az", sizeof(val.sun_scan_sector_width_az), (char *) &val.sun_scan_sector_width_az - id);
  fprintf(out, _dform, "fl32", "sun_scan_sector_width_el", sizeof(val.sun_scan_sector_width_el), (char *) &val.sun_scan_sector_width_el - id);

  snprintf(tmpStr, 1024, "unused[%d]", (int) (sizeof(val.unused) / sizeof(fl32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.unused), (char *) val.unused - id);

  snprintf(tmpStr, 1024, "segment_name[%d]", (int) sizeof(val.segment_name));
  fprintf(out, _dform, "char", tmpStr, sizeof(val.segment_name), (char *) val.segment_name - id);
  snprintf(tmpStr, 1024, "project_name[%d]", (int) sizeof(val.project_name));
  fprintf(out, _dform, "char", tmpStr, sizeof(val.project_name), (char *) val.project_name - id);

  _print_format_divider('-', out);

}


// print format of ts_processing

void ips_ts_processing_print_format(FILE *out, const ips_ts_processing_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_processing_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IPS_TS_PROCESSING_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "pol_mode", sizeof(val.pol_mode), (char *) &val.pol_mode - id);
  fprintf(out, _dform, "si32", "prf_mode", sizeof(val.prf_mode), (char *) &val.prf_mode - id);
  fprintf(out, _dform, "si32", "pulse_shape", sizeof(val.pulse_shape), (char *) &val.pulse_shape - id);
  fprintf(out, _dform, "fl32", "pulse_width_us", sizeof(val.pulse_width_us), (char *) &val.pulse_width_us - id);
  fprintf(out, _dform, "fl32", "start_range_m", sizeof(val.start_range_m), (char *) &val.start_range_m - id);
  fprintf(out, _dform, "fl32", "gate_spacing_m", sizeof(val.gate_spacing_m), (char *) &val.gate_spacing_m - id);
  fprintf(out, _dform, "fl32", "test_pulse_range_km", sizeof(val.test_pulse_range_km), (char *) &val.test_pulse_range_km - id);
  fprintf(out, _dform, "fl32", "test_pulse_length_us", sizeof(val.test_pulse_length_us), (char *) &val.test_pulse_length_us - id);
  fprintf(out, _dform, "si32", "num_prts", sizeof(val.num_prts), (char *) &val.num_prts - id);

  char tmpStr[1024];
  snprintf(tmpStr, 1024, "prt_us[%d]", IPS_TS_MAX_PRT);
  fprintf(out, _dform, "fl32", tmpStr, sizeof(val.prt_us), (char *) val.prt_us - id);

  snprintf(tmpStr, 1024, "unused[%d]", (int) (sizeof(val.unused) / sizeof(fl32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}


// print format of status_xml

void ips_ts_status_xml_print_format(FILE *out, const ips_ts_status_xml_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_status_xml_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IPS_TS_STATUS_XML_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);
  
  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);
  
  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "xml_len", sizeof(val.xml_len), (char *) &val.xml_len - id);

  char tmpStr[1024];
  snprintf(tmpStr, 1024, "unused[%d]", (int) (sizeof(val.unused) / sizeof(fl32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}

// print format of calibration

void ips_ts_calibration_print_format(FILE *out, const ips_ts_calibration_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_calibration_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IPS_TS_CALIBRATION_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "fl32", "wavelength_cm", sizeof(val.wavelength_cm), (char *) &val.wavelength_cm - id);
  fprintf(out, _dform, "fl32", "beamwidth_deg_h", sizeof(val.beamwidth_deg_h), (char *) &val.beamwidth_deg_h - id);
  fprintf(out, _dform, "fl32", "beamwidth_deg_v", sizeof(val.beamwidth_deg_v), (char *) &val.beamwidth_deg_v - id);
  fprintf(out, _dform, "fl32", "gain_ant_db_h", sizeof(val.gain_ant_db_h), (char *) &val.gain_ant_db_h - id);
  fprintf(out, _dform, "fl32", "gain_ant_db_v", sizeof(val.gain_ant_db_v), (char *) &val.gain_ant_db_v - id);
  fprintf(out, _dform, "fl32", "pulse_width_us", sizeof(val.pulse_width_us), (char *) &val.pulse_width_us - id);
  fprintf(out, _dform, "fl32", "xmit_power_dbm_h", sizeof(val.xmit_power_dbm_h), (char *) &val.xmit_power_dbm_h - id);
  fprintf(out, _dform, "fl32", "xmit_power_dbm_v", sizeof(val.xmit_power_dbm_v), (char *) &val.xmit_power_dbm_v - id);
  fprintf(out, _dform, "fl32", "two_way_waveguide_loss_db_h", sizeof(val.two_way_waveguide_loss_db_h), (char *) &val.two_way_waveguide_loss_db_h - id);
  fprintf(out, _dform, "fl32", "two_way_waveguide_loss_db_v", sizeof(val.two_way_waveguide_loss_db_v), (char *) &val.two_way_waveguide_loss_db_v - id);
  fprintf(out, _dform, "fl32", "two_way_radome_loss_db_h", sizeof(val.two_way_radome_loss_db_h), (char *) &val.two_way_radome_loss_db_h - id);
  fprintf(out, _dform, "fl32", "two_way_radome_loss_db_v", sizeof(val.two_way_radome_loss_db_v), (char *) &val.two_way_radome_loss_db_v - id);
  fprintf(out, _dform, "fl32", "receiver_mismatch_loss_db", sizeof(val.receiver_mismatch_loss_db), (char *) &val.receiver_mismatch_loss_db - id);
  fprintf(out, _dform, "fl32", "k_squared_water", sizeof(val.k_squared_water), (char *) &val.k_squared_water - id);
  fprintf(out, _dform, "fl32", "radar_constant_h", sizeof(val.radar_constant_h), (char *) &val.radar_constant_h - id);
  fprintf(out, _dform, "fl32", "radar_constant_v", sizeof(val.radar_constant_v), (char *) &val.radar_constant_v - id);
  fprintf(out, _dform, "fl32", "noise_dbm_hc", sizeof(val.noise_dbm_hc), (char *) &val.noise_dbm_hc - id);
  fprintf(out, _dform, "fl32", "noise_dbm_hx", sizeof(val.noise_dbm_hx), (char *) &val.noise_dbm_hx - id);
  fprintf(out, _dform, "fl32", "noise_dbm_vc", sizeof(val.noise_dbm_vc), (char *) &val.noise_dbm_vc - id);
  fprintf(out, _dform, "fl32", "noise_dbm_vx", sizeof(val.noise_dbm_vx), (char *) &val.noise_dbm_vx - id);
  fprintf(out, _dform, "fl32", "i0_dbm_hc", sizeof(val.i0_dbm_hc), (char *) &val.i0_dbm_hc - id);
  fprintf(out, _dform, "fl32", "i0_dbm_hx", sizeof(val.i0_dbm_hx), (char *) &val.i0_dbm_hx - id);
  fprintf(out, _dform, "fl32", "i0_dbm_vc", sizeof(val.i0_dbm_vc), (char *) &val.i0_dbm_vc - id);
  fprintf(out, _dform, "fl32", "i0_dbm_vx", sizeof(val.i0_dbm_vx), (char *) &val.i0_dbm_vx - id);
  fprintf(out, _dform, "fl32", "receiver_gain_db_hc", sizeof(val.receiver_gain_db_hc), (char *) &val.receiver_gain_db_hc - id);
  fprintf(out, _dform, "fl32", "receiver_gain_db_hx", sizeof(val.receiver_gain_db_hx), (char *) &val.receiver_gain_db_hx - id);
  fprintf(out, _dform, "fl32", "receiver_gain_db_vc", sizeof(val.receiver_gain_db_vc), (char *) &val.receiver_gain_db_vc - id);
  fprintf(out, _dform, "fl32", "receiver_gain_db_vx", sizeof(val.receiver_gain_db_vx), (char *) &val.receiver_gain_db_vx - id);
  fprintf(out, _dform, "fl32", "receiver_slope_hc", sizeof(val.receiver_slope_hc), (char *) &val.receiver_slope_hc - id);
  fprintf(out, _dform, "fl32", "receiver_slope_hx", sizeof(val.receiver_slope_hx), (char *) &val.receiver_slope_hx - id);
  fprintf(out, _dform, "fl32", "receiver_slope_vc", sizeof(val.receiver_slope_vc), (char *) &val.receiver_slope_vc - id);
  fprintf(out, _dform, "fl32", "receiver_slope_vx", sizeof(val.receiver_slope_vx), (char *) &val.receiver_slope_vx - id);
  fprintf(out, _dform, "fl32", "dynamic_range_db_hc", sizeof(val.dynamic_range_db_hc), (char *) &val.dynamic_range_db_hc - id);
  fprintf(out, _dform, "fl32", "dynamic_range_db_hx", sizeof(val.dynamic_range_db_hx), (char *) &val.dynamic_range_db_hx - id);
  fprintf(out, _dform, "fl32", "dynamic_range_db_vc", sizeof(val.dynamic_range_db_vc), (char *) &val.dynamic_range_db_vc - id);
  fprintf(out, _dform, "fl32", "dynamic_range_db_vx", sizeof(val.dynamic_range_db_vx), (char *) &val.dynamic_range_db_vx - id);
  fprintf(out, _dform, "fl32", "base_dbz_1km_hc", sizeof(val.base_dbz_1km_hc), (char *) &val.base_dbz_1km_hc - id);
  fprintf(out, _dform, "fl32", "base_dbz_1km_hx", sizeof(val.base_dbz_1km_hx), (char *) &val.base_dbz_1km_hx - id);
  fprintf(out, _dform, "fl32", "base_dbz_1km_vc", sizeof(val.base_dbz_1km_vc), (char *) &val.base_dbz_1km_vc - id);
  fprintf(out, _dform, "fl32", "base_dbz_1km_vx", sizeof(val.base_dbz_1km_vx), (char *) &val.base_dbz_1km_vx - id);
  fprintf(out, _dform, "fl32", "sun_power_dbm_hc", sizeof(val.sun_power_dbm_hc), (char *) &val.sun_power_dbm_hc - id);
  fprintf(out, _dform, "fl32", "sun_power_dbm_hx", sizeof(val.sun_power_dbm_hx), (char *) &val.sun_power_dbm_hx - id);
  fprintf(out, _dform, "fl32", "sun_power_dbm_vc", sizeof(val.sun_power_dbm_vc), (char *) &val.sun_power_dbm_vc - id);
  fprintf(out, _dform, "fl32", "sun_power_dbm_vx", sizeof(val.sun_power_dbm_vx), (char *) &val.sun_power_dbm_vx - id);
  fprintf(out, _dform, "fl32", "noise_source_power_dbm_h", sizeof(val.noise_source_power_dbm_h), (char *) &val.noise_source_power_dbm_h - id);
  fprintf(out, _dform, "fl32", "noise_source_power_dbm_v", sizeof(val.noise_source_power_dbm_v), (char *) &val.noise_source_power_dbm_v - id);
  fprintf(out, _dform, "fl32", "power_meas_loss_db_h", sizeof(val.power_meas_loss_db_h), (char *) &val.power_meas_loss_db_h - id);
  fprintf(out, _dform, "fl32", "power_meas_loss_db_v", sizeof(val.power_meas_loss_db_v), (char *) &val.power_meas_loss_db_v - id);
  fprintf(out, _dform, "fl32", "coupler_forward_loss_db_h", sizeof(val.coupler_forward_loss_db_h), (char *) &val.coupler_forward_loss_db_h - id);
  fprintf(out, _dform, "fl32", "coupler_forward_loss_db_v", sizeof(val.coupler_forward_loss_db_v), (char *) &val.coupler_forward_loss_db_v - id);
  fprintf(out, _dform, "fl32", "test_power_dbm_h", sizeof(val.test_power_dbm_h), (char *) &val.test_power_dbm_h - id);
  fprintf(out, _dform, "fl32", "test_power_dbm_v", sizeof(val.test_power_dbm_v), (char *) &val.test_power_dbm_v - id);
  fprintf(out, _dform, "fl32", "zdr_correction_db", sizeof(val.zdr_correction_db), (char *) &val.zdr_correction_db - id);
  fprintf(out, _dform, "fl32", "ldr_correction_db_h", sizeof(val.ldr_correction_db_h), (char *) &val.ldr_correction_db_h - id);
  fprintf(out, _dform, "fl32", "ldr_correction_db_v", sizeof(val.ldr_correction_db_v), (char *) &val.ldr_correction_db_v - id);
  fprintf(out, _dform, "fl32", "phidp_rot_deg", sizeof(val.phidp_rot_deg), (char *) &val.phidp_rot_deg - id);

  fprintf(out, _dform, "fl32", "receiver_slope_hc", sizeof(val.receiver_slope_hc), (char *) &val.receiver_slope_hc - id);
  fprintf(out, _dform, "fl32", "receiver_slope_hx", sizeof(val.receiver_slope_hx), (char *) &val.receiver_slope_hx - id);
  fprintf(out, _dform, "fl32", "receiver_slope_vc", sizeof(val.receiver_slope_vc), (char *) &val.receiver_slope_vc - id);
  fprintf(out, _dform, "fl32", "receiver_slope_vx", sizeof(val.receiver_slope_vx), (char *) &val.receiver_slope_vx - id);

  fprintf(out, _dform, "fl32", "i0_dbm_hc", sizeof(val.i0_dbm_hc), (char *) &val.i0_dbm_hc - id);
  fprintf(out, _dform, "fl32", "i0_dbm_hx", sizeof(val.i0_dbm_hx), (char *) &val.i0_dbm_hx - id);
  fprintf(out, _dform, "fl32", "i0_dbm_vc", sizeof(val.i0_dbm_vc), (char *) &val.i0_dbm_vc - id);
  fprintf(out, _dform, "fl32", "i0_dbm_vx", sizeof(val.i0_dbm_vx), (char *) &val.i0_dbm_vx - id);

  fprintf(out, _dform, "fl32", "dynamic_range_db_hc", sizeof(val.dynamic_range_db_hc), (char *) &val.dynamic_range_db_hc - id);
  fprintf(out, _dform, "fl32", "dynamic_range_db_hx", sizeof(val.dynamic_range_db_hx), (char *) &val.dynamic_range_db_hx - id);
  fprintf(out, _dform, "fl32", "dynamic_range_db_vc", sizeof(val.dynamic_range_db_vc), (char *) &val.dynamic_range_db_vc - id);
  fprintf(out, _dform, "fl32", "dynamic_range_db_vx", sizeof(val.dynamic_range_db_vx), (char *) &val.dynamic_range_db_vx - id);

  fprintf(out, _dform, "fl32", "k_squared_water", sizeof(val.k_squared_water), (char *) &val.k_squared_water - id);

  fprintf(out, _dform, "fl32", "dbz_correction", sizeof(val.dbz_correction), (char *) &val.dbz_correction - id);

  char tmpStr[1024];
  snprintf(tmpStr, 1024, "unused[%d]", (int) (sizeof(val.unused) / sizeof(fl32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.unused), (char *) val.unused - id);

  snprintf(tmpStr, 1024, "radar_name[%d]", (int) sizeof(val.radar_name));
  fprintf(out, _dform, "char", tmpStr, sizeof(val.radar_name), (char *) val.radar_name - id);

  _print_format_divider('-', out);

}

  
// print format of event_notice

void ips_ts_event_notice_print_format(FILE *out, const ips_ts_event_notice_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_event_notice_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IPS_TS_EVENT_NOTICE_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "start_of_sweep", sizeof(val.start_of_sweep), (char *) &val.start_of_sweep - id);
  fprintf(out, _dform, "si32", "end_of_sweep", sizeof(val.end_of_sweep), (char *) &val.end_of_sweep - id);
  fprintf(out, _dform, "si32", "start_of_volume", sizeof(val.start_of_volume), (char *) &val.start_of_volume - id);
  fprintf(out, _dform, "si32", "end_of_volume", sizeof(val.end_of_volume), (char *) &val.end_of_volume - id);
  fprintf(out, _dform, "si32", "scan_mode", sizeof(val.scan_mode), (char *) &val.scan_mode - id);
  fprintf(out, _dform, "si32", "volume_num", sizeof(val.volume_num), (char *) &val.volume_num - id);
  fprintf(out, _dform, "si32", "sweep_num", sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "fl32", "current_fixed_angle", sizeof(val.current_fixed_angle), (char *) &val.current_fixed_angle - id);

  char tmpStr[1024];
  snprintf(tmpStr, 1024, "unused[%d]", (int) (sizeof(val.unused) / sizeof(fl32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}


// print format of pulse_header

void ips_ts_pulse_header_print_format(FILE *out, const ips_ts_pulse_header_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_pulse_header_t'\n  size: %d\n  id: 0x%x\n\n", 
          (int) sizeof(val), IPS_TS_PULSE_HEADER_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si64", "pulse_seq_num", sizeof(val.pulse_seq_num), (char *) &val.pulse_seq_num - id);
  fprintf(out, _dform, "si64", "dwell_seq_num", sizeof(val.dwell_seq_num), (char *) &val.dwell_seq_num - id);
  fprintf(out, _dform, "si32", "beam_num_in_dwell", sizeof(val.beam_num_in_dwell), (char *) &val.beam_num_in_dwell - id);
  fprintf(out, _dform, "si32", "visit_num_in_beam", sizeof(val.visit_num_in_beam), (char *) &val.visit_num_in_beam - id);
  fprintf(out, _dform, "si32", "scan_mode", sizeof(val.scan_mode), (char *) &val.scan_mode - id);
  fprintf(out, _dform, "si32", "volume_num", sizeof(val.volume_num), (char *) &val.volume_num - id);
  fprintf(out, _dform, "si32", "sweep_num", sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "fl32", "elevation", sizeof(val.elevation), (char *) &val.elevation - id);
  fprintf(out, _dform, "fl32", "azimuth", sizeof(val.azimuth), (char *) &val.azimuth - id);
  fprintf(out, _dform, "fl32", "fixed_angle", sizeof(val.fixed_angle), (char *) &val.fixed_angle - id);
  fprintf(out, _dform, "fl32", "prt", sizeof(val.prt), (char *) &val.prt - id);
  fprintf(out, _dform, "fl32", "prt_next", sizeof(val.prt_next), (char *) &val.prt_next - id);
  fprintf(out, _dform, "fl32", "pulse_width_us", sizeof(val.pulse_width_us), (char *) &val.pulse_width_us - id);
  fprintf(out, _dform, "si32", "n_gates", sizeof(val.n_gates), (char *) &val.n_gates - id);
  fprintf(out, _dform, "fl32", "start_range_m", sizeof(val.start_range_m), (char *) &val.start_range_m - id);
  fprintf(out, _dform, "fl32", "gate_spacing_m", sizeof(val.gate_spacing_m), (char *) &val.gate_spacing_m - id);
  fprintf(out, _dform, "si32", "hv_flag", sizeof(val.hv_flag), (char *) &val.hv_flag - id);
  fprintf(out, _dform, "si32", "phase_cohered", sizeof(val.phase_cohered), (char *) &val.phase_cohered - id);
  fprintf(out, _dform, "si32", "iq_encoding", sizeof(val.iq_encoding), (char *) &val.iq_encoding - id);
  fprintf(out, _dform, "si32", "n_channels", sizeof(val.n_channels), (char *) &val.n_channels - id);
  fprintf(out, _dform, "si32", "n_data", sizeof(val.n_data), (char *) &val.n_data - id);
  fprintf(out, _dform, "fl32", "scale", sizeof(val.scale), (char *) &val.scale - id);
  fprintf(out, _dform, "fl32", "offset", sizeof(val.offset), (char *) &val.offset - id);

  char tmpStr[1024];
  snprintf(tmpStr, 1024, "chan_is_copol[%d]", IPS_TS_MAX_CHAN);
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.chan_is_copol), (char *) val.chan_is_copol - id);

  fprintf(out, _dform, "si32", "status", sizeof(val.status), (char *) &val.status - id);
  fprintf(out, _dform, "si32", "event_flags", sizeof(val.event_flags), (char *) &val.event_flags - id);

  snprintf(tmpStr, 1024, "unused[%d]", (int) (sizeof(val.unused) / sizeof(fl32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}

// print format of platform_georef

void ips_ts_platform_georef_print_format(FILE *out, const ips_ts_platform_georef_t &val)
{
  
  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_platform_georef_t'\n  size: %d\n  id: 0x%x\n\n", 
          (int) sizeof(val), IPS_TS_PLATFORM_GEOREF_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "fl64", "longitude",
          sizeof(val.longitude), (char *) &val.longitude - id);
  fprintf(out, _dform, "fl64", "latitude",
          sizeof(val.latitude), (char *) &val.latitude - id);

  fprintf(out, _dform, "si32", "unit_num",
          sizeof(val.unit_num), (char *) &val.unit_num - id);
  fprintf(out, _dform, "si32", "id",
          sizeof(val.unit_id), (char *) &val.unit_id - id);

  fprintf(out, _dform, "fl32", "altitude_msl_km",
          sizeof(val.altitude_msl_km), (char *) &val.altitude_msl_km - id);
  fprintf(out, _dform, "fl32", "altitude_agl_km",
          sizeof(val.altitude_agl_km), (char *) &val.altitude_agl_km - id);

  fprintf(out, _dform, "fl32", "ew_velocity_mps",
          sizeof(val.ew_velocity_mps), (char *) &val.ew_velocity_mps - id);
  fprintf(out, _dform, "fl32", "ns_velocity_mps",
          sizeof(val.ns_velocity_mps), (char *) &val.ns_velocity_mps - id);
  fprintf(out, _dform, "fl32", "vert_velocity_mps",
          sizeof(val.vert_velocity_mps), (char *) &val.vert_velocity_mps - id);

  fprintf(out, _dform, "fl32", "heading_deg",
          sizeof(val.heading_deg), (char *) &val.heading_deg - id);
  fprintf(out, _dform, "fl32", "track_deg",
          sizeof(val.track_deg), (char *) &val.track_deg - id);
  fprintf(out, _dform, "fl32", "roll_deg",
          sizeof(val.roll_deg), (char *) &val.roll_deg - id);
  fprintf(out, _dform, "fl32", "pitch_deg",
          sizeof(val.pitch_deg), (char *) &val.pitch_deg - id);
  fprintf(out, _dform, "fl32", "drift_angle_deg",
          sizeof(val.drift_angle_deg), (char *) &val.drift_angle_deg - id);

  fprintf(out, _dform, "fl32", "rotation_angle_deg",
          sizeof(val.rotation_angle_deg), (char *) &val.rotation_angle_deg - id);
  fprintf(out, _dform, "fl32", "tilt_deg",
          sizeof(val.tilt_deg), (char *) &val.tilt_deg - id);

  fprintf(out, _dform, "fl32", "ew_horiz_wind_mps",
          sizeof(val.ew_horiz_wind_mps), (char *) &val.ew_horiz_wind_mps - id);
  fprintf(out, _dform, "fl32", "ns_horiz_wind_mps",
          sizeof(val.ns_horiz_wind_mps), (char *) &val.ns_horiz_wind_mps - id);
  fprintf(out, _dform, "fl32", "vert_wind_mps",
          sizeof(val.vert_wind_mps), (char *) &val.vert_wind_mps - id);

  fprintf(out, _dform, "fl32", "heading_rate_dps",
          sizeof(val.heading_rate_dps), (char *) &val.heading_rate_dps - id);
  fprintf(out, _dform, "fl32", "pitch_rate_dps",
          sizeof(val.pitch_rate_dps), (char *) &val.pitch_rate_dps - id);
  fprintf(out, _dform, "fl32", "roll_rate_dps",
          sizeof(val.roll_rate_dps), (char *) &val.roll_rate_dps - id);
  
  char tmpStr[1024];
  snprintf(tmpStr, 1024, "unused[%d]", (int) (sizeof(val.unused) / sizeof(fl32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.unused), (char *) val.unused - id);
  
  _print_format_divider('-', out);

}


// print format of georef_correction

void ips_ts_georef_correction_print_format(FILE *out, const ips_ts_georef_correction_t &val)
{
  
  _print_format_divider('-', out);
  fprintf(out, "  struct: 'ips_ts_georef_correction_t'\n  size: %d\n  id: 0x%x\n\n", 
          (int) sizeof(val), IPS_TS_GEOREF_CORRECTION_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "fl32", "longitude_corr_deg",
          sizeof(val.longitude_corr_deg), (char *) &val.longitude_corr_deg - id);
  fprintf(out, _dform, "fl32", "latitude_corr_deg",
          sizeof(val.latitude_corr_deg), (char *) &val.latitude_corr_deg - id);

  fprintf(out, _dform, "fl32", "azimuth_corr_deg",
          sizeof(val.azimuth_corr_deg), (char *) &val.azimuth_corr_deg - id);
  fprintf(out, _dform, "fl32", "elevation_corr_deg",
          sizeof(val.elevation_corr_deg), (char *) &val.elevation_corr_deg - id);

  fprintf(out, _dform, "fl32", "range_delay_corr_mps",
          sizeof(val.range_delay_corr_mps), (char *) &val.range_delay_corr_mps - id);
  fprintf(out, _dform, "fl32", "pressure_alt_corr_km",
          sizeof(val.pressure_alt_corr_km), (char *) &val.pressure_alt_corr_km - id);

  fprintf(out, _dform, "fl32", "radar_alt_corr_km",
          sizeof(val.radar_alt_corr_km), (char *) &val.radar_alt_corr_km - id);

  fprintf(out, _dform, "fl32", "ew_gndspd_corr_mps",
          sizeof(val.ew_gndspd_corr_mps), (char *) &val.ew_gndspd_corr_mps - id);
  fprintf(out, _dform, "fl32", "ns_gndspd_corr_mps",
          sizeof(val.ns_gndspd_corr_mps), (char *) &val.ns_gndspd_corr_mps - id);

  fprintf(out, _dform, "fl32", "vert_vel_corr_mps",
          sizeof(val.vert_vel_corr_mps), (char *) &val.vert_vel_corr_mps - id);

  fprintf(out, _dform, "fl32", "heading_corr_deg",
          sizeof(val.heading_corr_deg), (char *) &val.heading_corr_deg - id);
  fprintf(out, _dform, "fl32", "roll_corr_deg",
          sizeof(val.roll_corr_deg), (char *) &val.roll_corr_deg - id);
  fprintf(out, _dform, "fl32", "pitch_corr_deg",
          sizeof(val.pitch_corr_deg), (char *) &val.pitch_corr_deg - id);
  fprintf(out, _dform, "fl32", "drift_corr_deg",
          sizeof(val.drift_corr_deg), (char *) &val.drift_corr_deg - id);
  fprintf(out, _dform, "fl32", "rot_angle_corr_deg",
          sizeof(val.rot_angle_corr_deg), (char *) &val.rot_angle_corr_deg - id);
  fprintf(out, _dform, "fl32", "tilt_corr_deg",
          sizeof(val.tilt_corr_deg), (char *) &val.tilt_corr_deg - id);

  char tmpStr[1024];
  snprintf(tmpStr, 1024, "unused[%d]", (int) (sizeof(val.unused) / sizeof(fl32)));
  fprintf(out, _dform, "si32", tmpStr, sizeof(val.unused), (char *) val.unused - id);
  
  _print_format_divider('-', out);

}
