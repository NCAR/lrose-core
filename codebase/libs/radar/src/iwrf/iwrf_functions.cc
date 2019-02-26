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
// iwrf_functions.cc
//
// Utility routines for iwrf_data structs
//
// Mike Dixon, RAL, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// Feb 2009

#include <dataport/swap.h>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <radar/iwrf_functions.hh>

using namespace std;

/////////////////////////////
// check for a missing values

bool iwrf_int_is_missing(si32 val)
{
  if (val == IWRF_MISSING_INT) {
    return true;
  } else {
    return false;
  }
}

bool iwrf_float_is_missing(fl32 val)
{
  if (isnan(val)) {
    return true;
  } else if (fabs(val - IWRF_MISSING_FLOAT) < 0.001)  {
    return true;
  } else {
    return false;
  }
}

//////////////////////////////////////////////////////////////////
// packet initialization
// sets values to missing as appropriate

//////////////////////////////////////////////////////
// init sync struct

void iwrf_sync_init(iwrf_sync_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_SYNC_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.magik[0] = IWRF_SYNC_VAL_00;
  val.magik[1] = IWRF_SYNC_VAL_01;

}

//////////////////////////////////////////////////////
// init version struct

void iwrf_version_init(iwrf_version_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_VERSION_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);
  
}

//////////////////////////////////////////////////////
// init radar_info struct

void iwrf_radar_info_init(iwrf_radar_info &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_RADAR_INFO_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.latitude_deg = IWRF_MISSING_FLOAT;
  val.longitude_deg = IWRF_MISSING_FLOAT;
  val.altitude_m = IWRF_MISSING_FLOAT;
  val.platform_type = IWRF_RADAR_PLATFORM_NOT_SET;

  val.beamwidth_deg_h = IWRF_MISSING_FLOAT;
  val.beamwidth_deg_v = IWRF_MISSING_FLOAT;
  val.wavelength_cm = IWRF_MISSING_FLOAT;
  
  val.nominal_gain_ant_db_h = IWRF_MISSING_FLOAT;
  val.nominal_gain_ant_db_v = IWRF_MISSING_FLOAT;

}

//////////////////////////////////////////////////////
// init scan_segment struct

void iwrf_scan_segment_init(iwrf_scan_segment &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_SCAN_SEGMENT_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.scan_mode = IWRF_SCAN_MODE_NOT_SET;
  val.follow_mode = IWRF_FOLLOW_MODE_NOT_SET;
  val.volume_num = IWRF_MISSING_INT;
  val.sweep_num = IWRF_MISSING_INT;
  val.time_limit = 0;

  val.az_manual = IWRF_MISSING_FLOAT;
  val.el_manual = IWRF_MISSING_FLOAT;
  val.az_start = IWRF_MISSING_FLOAT;
  val.el_start = IWRF_MISSING_FLOAT;
  val.scan_rate = IWRF_MISSING_FLOAT;
  val.left_limit = IWRF_MISSING_FLOAT;
  val.right_limit = IWRF_MISSING_FLOAT;
  val.up_limit = IWRF_MISSING_FLOAT;
  val.down_limit = IWRF_MISSING_FLOAT;
  val.step = IWRF_MISSING_FLOAT;

  val.current_fixed_angle = IWRF_MISSING_FLOAT;
  val.init_direction_cw = 1;
  val.init_direction_up = 1;

  val.n_sweeps = 0;
  
  val.optimizer_rmax_km = IWRF_MISSING_FLOAT;
  val.optimizer_htmax_km = IWRF_MISSING_FLOAT;
  val.optimizer_res_m = IWRF_MISSING_FLOAT;  

  val.sun_scan_sector_width_az = IWRF_MISSING_FLOAT;  
  val.sun_scan_sector_width_el = IWRF_MISSING_FLOAT;  

  val.timeseries_archive_enable = 0;
  val.waveguide_switch_position = 0;
  val.start_ppi_ccw = 0;

}

//////////////////////////////////////////////////////
// init antenna_correction struct

void iwrf_antenna_correction_init(iwrf_antenna_correction_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_ANTENNA_CORRECTION_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.az_correction = IWRF_MISSING_FLOAT;
  val.el_correction = IWRF_MISSING_FLOAT;

}

//////////////////////////////////////////////////////
// init ts_processing struct

void iwrf_ts_processing_init(iwrf_ts_processing_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_TS_PROCESSING_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.xmit_rcv_mode = IWRF_XMIT_RCV_MODE_NOT_SET;
  val.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_NOT_SET;
  val.prf_mode = IWRF_PRF_MODE_NOT_SET;
  val.pulse_type = IWRF_PULSE_TYPE_NOT_SET;

  val.prt_usec = IWRF_MISSING_FLOAT;
  val.prt2_usec = IWRF_MISSING_FLOAT;

  val.cal_type = IWRF_CAL_TYPE_NOT_SET;
  
  val.burst_range_offset_m = IWRF_MISSING_FLOAT;
  val.pulse_width_us = IWRF_MISSING_FLOAT;
  val.start_range_m = IWRF_MISSING_FLOAT;
  val.gate_spacing_m = IWRF_MISSING_FLOAT;

  val.integration_cycle_pulses = IWRF_MISSING_INT;
  val.clutter_filter_number = IWRF_MISSING_INT;
  val.range_gate_averaging = 1;
  val.max_gate = 0;

  val.test_power_dbm = IWRF_MISSING_FLOAT;
  val.test_pulse_range_km = IWRF_MISSING_FLOAT;
  val.test_pulse_length_usec = IWRF_MISSING_FLOAT;

  val.pol_mode = IWRF_POL_MODE_NOT_SET;

  val.beams_are_indexed = 0;
  val.specify_dwell_width = 1;
  val.indexed_beam_width_deg = 1.0;
  val.indexed_beam_spacing_deg = 1.0;

  val.num_prts = 1;
  val.prt3_usec = 0;
  val.prt4_usec = 0;
  val.block_mode_prt2_pulses = 0;
  val.block_mode_prt3_pulses = 0;
  val.block_mode_prt4_pulses = 0;
  val.pol_sync_mode = 0;
  
}

//////////////////////////////////////////////////////
// init xmit_power struct

void iwrf_xmit_power_init(iwrf_xmit_power_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_XMIT_POWER_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.power_dbm_h = IWRF_MISSING_FLOAT;
  val.power_dbm_v = IWRF_MISSING_FLOAT;

}

//////////////////////////////////////////////////////
// init rx_power struct

void iwrf_rx_power_init(iwrf_rx_power_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_RX_POWER_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.max_power_dbm_hc = IWRF_MISSING_FLOAT;
  val.max_power_dbm_vc = IWRF_MISSING_FLOAT;
  val.max_power_dbm_hx = IWRF_MISSING_FLOAT;
  val.max_power_dbm_vx = IWRF_MISSING_FLOAT;

}

//////////////////////////////////////////////////////
// init xmit_sample struct

void iwrf_xmit_sample_init(iwrf_xmit_sample_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_XMIT_SAMPLE_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.power_dbm_h = IWRF_MISSING_FLOAT;
  val.power_dbm_v = IWRF_MISSING_FLOAT;
  val.offset = 0;
  val.n_samples = 0;

  val.sampling_freq = IWRF_MISSING_FLOAT;

  val.scale_h = IWRF_MISSING_FLOAT;
  val.offset_h = IWRF_MISSING_FLOAT;

  val.scale_v = IWRF_MISSING_FLOAT;
  val.offset_v = IWRF_MISSING_FLOAT;

}

void iwrf_xmit_sample_v2_init(iwrf_xmit_sample_v2_t &val)

{

  MEM_zero(val);
  
  val.packet.id = IWRF_XMIT_SAMPLE_V2_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.n_samples = 0;
  val.n_channels = 0;

}

//////////////////////////////////////////////////////
// init xmit_info struct

void iwrf_xmit_info_init(iwrf_xmit_info_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_XMIT_INFO_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.xmit_0_enabled = 0;
  val.xmit_1_enabled = 0;

  val.xmit_rcv_mode = IWRF_XMIT_RCV_MODE_NOT_SET;
  val.pol_mode = IWRF_POL_MODE_NOT_SET;
  val.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_NOT_SET;
  val.prf_mode = IWRF_PRF_MODE_NOT_SET;
  val.pulse_type = IWRF_PULSE_TYPE_NOT_SET;

  val.prt_usec = IWRF_MISSING_FLOAT;
  val.prt2_usec = IWRF_MISSING_FLOAT;

}

//////////////////////////////////////////////////////
// init burst_iq struct

void iwrf_burst_header_init(iwrf_burst_header_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_BURST_HEADER_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.pulse_seq_num = 0;
  val.n_samples = 0;
  val.channel_id = 0;
  val.iq_encoding = IWRF_MISSING_INT;
  val.scale = IWRF_MISSING_FLOAT;
  val.offset = IWRF_MISSING_FLOAT;
  val.power_dbm = IWRF_MISSING_FLOAT;
  val.phase_deg = IWRF_MISSING_FLOAT;
  val.freq_hz = IWRF_MISSING_FLOAT;
  val.sampling_freq_hz = IWRF_MISSING_FLOAT;
  val.power_max_dbm = IWRF_MISSING_FLOAT;
  val.power_p90_dbm = IWRF_MISSING_FLOAT;
  val.pulse_width_sec = IWRF_MISSING_FLOAT;

}

//////////////////////////////////////////////////////
// init status_xml struct

void iwrf_status_xml_init(iwrf_status_xml_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_STATUS_XML_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);
  val.xml_len = 0;

}

//////////////////////////////////////////////////////
// init antenna_angles struct

void iwrf_antenna_angles_init(iwrf_antenna_angles_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_ANTENNA_ANGLES_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.scan_mode = IWRF_SCAN_MODE_NOT_SET;
  val.follow_mode = IWRF_FOLLOW_MODE_NOT_SET;
  val.sweep_num = IWRF_MISSING_INT;
  val.volume_num = IWRF_MISSING_INT;

  val.fixed_el = IWRF_MISSING_FLOAT;
  val.fixed_az = IWRF_MISSING_FLOAT;
  val.elevation = IWRF_MISSING_FLOAT;
  val.azimuth = IWRF_MISSING_FLOAT;
  
  val.antenna_transition = 0;

}

//////////////////////////////////////////////////////
// init calibration struct

void iwrf_calibration_init(iwrf_calibration_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_CALIBRATION_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.wavelength_cm = IWRF_MISSING_FLOAT;
  val.beamwidth_deg_h = IWRF_MISSING_FLOAT;
  val.beamwidth_deg_v = IWRF_MISSING_FLOAT;
  val.gain_ant_db_h = IWRF_MISSING_FLOAT;
  val.gain_ant_db_v = IWRF_MISSING_FLOAT;
  val.pulse_width_us = IWRF_MISSING_FLOAT;
  val.xmit_power_dbm_h = IWRF_MISSING_FLOAT;
  val.xmit_power_dbm_v = IWRF_MISSING_FLOAT;
  val.two_way_waveguide_loss_db_h = IWRF_MISSING_FLOAT;
  val.two_way_waveguide_loss_db_v = IWRF_MISSING_FLOAT;
  val.two_way_radome_loss_db_h = IWRF_MISSING_FLOAT;
  val.two_way_radome_loss_db_v = IWRF_MISSING_FLOAT;
  val.receiver_mismatch_loss_db = IWRF_MISSING_FLOAT;
  val.k_squared_water = IWRF_MISSING_FLOAT;
  val.radar_constant_h = IWRF_MISSING_FLOAT;
  val.radar_constant_v = IWRF_MISSING_FLOAT;
  val.noise_dbm_hc = IWRF_MISSING_FLOAT;
  val.noise_dbm_hx = IWRF_MISSING_FLOAT;
  val.noise_dbm_vc = IWRF_MISSING_FLOAT;
  val.noise_dbm_vx = IWRF_MISSING_FLOAT;
  val.i0_dbm_hc = IWRF_MISSING_FLOAT;
  val.i0_dbm_hx = IWRF_MISSING_FLOAT;
  val.i0_dbm_vc = IWRF_MISSING_FLOAT;
  val.i0_dbm_vx = IWRF_MISSING_FLOAT;
  val.receiver_gain_db_hc = IWRF_MISSING_FLOAT;
  val.receiver_gain_db_hx = IWRF_MISSING_FLOAT;
  val.receiver_gain_db_vc = IWRF_MISSING_FLOAT;
  val.receiver_gain_db_vx = IWRF_MISSING_FLOAT;
  val.receiver_slope_hc = IWRF_MISSING_FLOAT;
  val.receiver_slope_hx = IWRF_MISSING_FLOAT;
  val.receiver_slope_vc = IWRF_MISSING_FLOAT;
  val.receiver_slope_vx = IWRF_MISSING_FLOAT;
  val.dynamic_range_db_hc = IWRF_MISSING_FLOAT;
  val.dynamic_range_db_hx = IWRF_MISSING_FLOAT;
  val.dynamic_range_db_vc = IWRF_MISSING_FLOAT;
  val.dynamic_range_db_vx = IWRF_MISSING_FLOAT;
  val.base_dbz_1km_hc = IWRF_MISSING_FLOAT;
  val.base_dbz_1km_hx = IWRF_MISSING_FLOAT;
  val.base_dbz_1km_vc = IWRF_MISSING_FLOAT;
  val.base_dbz_1km_vx = IWRF_MISSING_FLOAT;
  val.sun_power_dbm_hc = IWRF_MISSING_FLOAT;
  val.sun_power_dbm_hx = IWRF_MISSING_FLOAT;
  val.sun_power_dbm_vc = IWRF_MISSING_FLOAT;
  val.sun_power_dbm_vx = IWRF_MISSING_FLOAT;
  val.noise_source_power_dbm_h = IWRF_MISSING_FLOAT;
  val.noise_source_power_dbm_v = IWRF_MISSING_FLOAT;
  val.power_meas_loss_db_h = IWRF_MISSING_FLOAT;
  val.power_meas_loss_db_v = IWRF_MISSING_FLOAT;
  val.coupler_forward_loss_db_h = IWRF_MISSING_FLOAT;
  val.coupler_forward_loss_db_v = IWRF_MISSING_FLOAT;
  val.test_power_dbm_h = IWRF_MISSING_FLOAT;
  val.test_power_dbm_v = IWRF_MISSING_FLOAT;
  val.zdr_correction_db = 0.0;
  val.ldr_correction_db_h = 0.0;
  val.ldr_correction_db_v = 0.0;
  val.phidp_rot_deg = 0.0;
  val.dbz_correction = 0.0;
  
}

//////////////////////////////////////////////////////
// init event_notice struct

void iwrf_event_notice_init(iwrf_event_notice_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_EVENT_NOTICE_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.start_of_sweep = 0;
  val.end_of_sweep = 0;

  val.start_of_volume = 0;
  val.end_of_volume = 0;
  
  val.scan_mode = IWRF_SCAN_MODE_NOT_SET;
  val.follow_mode = IWRF_FOLLOW_MODE_NOT_SET;
  val.volume_num = IWRF_MISSING_INT;
  val.sweep_num = IWRF_MISSING_INT;
  
  val.cause = IWRF_EVENT_CAUSE_NOT_SET;
  val.current_fixed_angle = IWRF_MISSING_FLOAT;

  val.antenna_transition = 0;

}

//////////////////////////////////////////////////////
// init phasecode struct

void iwrf_phasecode_init(iwrf_phasecode_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_PHASECODE_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.seq_length = 0;

}

//////////////////////////////////////////////////////
// init pulse_header struct

void iwrf_pulse_header_init(iwrf_pulse_header_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_PULSE_HEADER_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.pulse_seq_num = 0;

  val.scan_mode = IWRF_SCAN_MODE_NOT_SET;
  val.follow_mode = IWRF_FOLLOW_MODE_NOT_SET;
  val.sweep_num = IWRF_MISSING_INT;
  val.volume_num = IWRF_MISSING_INT;

  val.fixed_el = IWRF_MISSING_FLOAT;
  val.fixed_az = IWRF_MISSING_FLOAT;
  val.elevation = IWRF_MISSING_FLOAT;
  val.azimuth = IWRF_MISSING_FLOAT;
  
  val.prt = IWRF_MISSING_FLOAT;
  val.prt_next = IWRF_MISSING_FLOAT;
  
  val.pulse_width_us = IWRF_MISSING_FLOAT;

  val.n_gates = 0;

  val.n_channels = 1;
  val.iq_encoding = IWRF_IQ_ENCODING_NOT_SET;
  val.hv_flag = IWRF_MISSING_INT;

  val.antenna_transition = 0;
  val.phase_cohered = IWRF_MISSING_INT;
  val.status = 0;
  val.n_data = 0;
  
  for (int ichan = 0; ichan < IWRF_MAX_CHAN; ichan++) {
    val.iq_offset[ichan] = 0;
    val.burst_mag[ichan] = IWRF_MISSING_FLOAT;
    val.burst_arg[ichan] = IWRF_MISSING_FLOAT;
    val.burst_arg_diff[ichan] = IWRF_MISSING_FLOAT;
  }

  val.scale = 1.0;
  val.offset = 0.0;
  val.n_gates_burst = 0;

}

//////////////////////////////////////////////////////
// init rvp8_ops_info struct

void iwrf_rvp8_ops_info_init(iwrf_rvp8_ops_info_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_RVP8_OPS_INFO_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.i_version = 0;

  val.f_pwidth_usec = IWRF_MISSING_FLOAT;

  val.f_dbz_calib = IWRF_MISSING_FLOAT;
  val.i_sample_size = IWRF_MISSING_INT;

  val.i_flags = 0;

  val.i_playback_version = IWRF_MISSING_INT;

  val.f_sy_clk_mhz = IWRF_MISSING_FLOAT;
  val.f_wavelength_cm = IWRF_MISSING_FLOAT;
  val.f_saturation_dbm = 6;
  
  val.f_range_mask_res = IWRF_MISSING_FLOAT;

  for (int ichan = 0; ichan < IWRF_MAX_CHAN; ichan++) {
    val.f_noise_dbm[ichan] = IWRF_MISSING_FLOAT;
    val.f_noise_stdv_db[ichan] = IWRF_MISSING_FLOAT;
  }
  val.f_noise_range_km = IWRF_MISSING_FLOAT;
  val.f_noise_prf_hz = IWRF_MISSING_FLOAT;

}

//////////////////////////////////////////////////////
// init rvp8_pulse_header struct

void iwrf_rvp8_pulse_header_init(iwrf_rvp8_pulse_header_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_RVP8_PULSE_HEADER_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.i_version = 0;
  val.i_flags = 0;
  val.i_aq_mode = 0;
  val.i_polar_bits = 0;
  val.i_viq_per_bin = 1;
  val.i_tg_bank = 1;

  val.i_num_vecs = IWRF_MISSING_INT;
  val.i_max_vecs = IWRF_MISSING_INT;
  
  for (int ichan = 0; ichan < IWRF_MAX_CHAN; ichan++) {
    val.i_data_off[ichan] = 0;
    val.f_burst_mag[ichan] = IWRF_MISSING_FLOAT;
  }

}

//////////////////////////////////////////////////////
// init moments_field_header struct

void iwrf_moments_field_header_init(iwrf_moments_field_header_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_MOMENTS_FIELD_HEADER_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.encoding = IWRF_MOMENTS_ENCODING_SCALED_SI16;
  val.byte_width = 2;
  val.sampling_ratio = 1;

}

//////////////////////////////////////////////////////
// init moments_ray_header struct

void iwrf_moments_ray_header_init(iwrf_moments_ray_header_t &val)

{

  MEM_zero(val);

  val.packet.id = IWRF_MOMENTS_RAY_HEADER_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);

  val.volume_num = IWRF_MISSING_INT;
  val.sweep_num = IWRF_MISSING_INT;
  val.scan_mode = IWRF_MISSING_INT;
  val.follow_mode = IWRF_MISSING_INT;
  val.prf_mode = IWRF_MISSING_INT;
  val.polarization_mode = IWRF_MISSING_INT;
  val.elevation = IWRF_MISSING_FLOAT;
  val.azimuth = IWRF_MISSING_FLOAT;
  val.fixed_angle = IWRF_MISSING_FLOAT;
  val.target_scan_rate = IWRF_MISSING_FLOAT;
  val.true_scan_rate = IWRF_MISSING_FLOAT;
  val.is_indexed = IWRF_MISSING_INT;
  val.angle_res = IWRF_MISSING_FLOAT;
  val.antenna_transition = IWRF_MISSING_INT;
  val.prt = IWRF_MISSING_FLOAT;
  val.prt_ratio = IWRF_MISSING_FLOAT;
  val.pulse_width_us = IWRF_MISSING_FLOAT;
  val.n_samples = IWRF_MISSING_INT;
  val.n_fields = IWRF_MISSING_INT;
  val.n_gates = IWRF_MISSING_INT;
  val.start_range_m = IWRF_MISSING_FLOAT;
  val.gate_spacing_m = IWRF_MISSING_FLOAT;
  val.nyquist_mps = IWRF_MISSING_FLOAT;
  val.unambig_range_km = IWRF_MISSING_FLOAT;
  val.meas_xmit_power_dbm_h = IWRF_MISSING_FLOAT;
  val.meas_xmit_power_dbm_v = IWRF_MISSING_FLOAT;
  val.event_flags = IWRF_MISSING_INT;

}

//////////////////////////////////////////////////////
// init moments_field_index struct

void iwrf_moments_field_index_init(iwrf_moments_field_index_t &val)

{
  MEM_zero(val);
}

//////////////////////////////////////////////////////
// init platform_georef struct

void iwrf_platform_georef_init(iwrf_platform_georef_t &val)

{
  MEM_zero(val);
  val.packet.id = IWRF_PLATFORM_GEOREF_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  // do not initialize time at this stage
  // iwrf_set_packet_time_to_now(val.packet);
}

//////////////////////////////////////////////////////
// init georef_correction struct

void iwrf_georef_correction_init(iwrf_georef_correction_t &val)

{
  MEM_zero(val);
  val.packet.id = IWRF_GEOREF_CORRECTION_ID;
  val.packet.len_bytes = sizeof(val);
  val.packet.version_num = 1;
  iwrf_set_packet_time_to_now(val.packet);
}

////////////////////////////////////////////////////////////
// set packet sequence number

void iwrf_set_packet_seq_num(iwrf_packet_info_t &packet, si64 seq_num) {
  packet.seq_num = seq_num;
}

////////////////////////////////////////////////////////////
// set packet time

void iwrf_set_packet_time(iwrf_packet_info_t &packet,
			  double dtime) {
  time_t secs = (time_t) dtime;
  int nano_secs = (int) ((dtime - secs) * 1.0e9 + 0.5);
  packet.time_secs_utc = secs;
  packet.time_nano_secs = nano_secs;
}

void iwrf_set_packet_time(iwrf_packet_info_t &packet,
			  time_t secs, int nano_secs) {
  packet.time_secs_utc = secs;
  packet.time_nano_secs = nano_secs;
}

void iwrf_set_packet_time_to_now(iwrf_packet_info_t &packet) {
  struct timeval time;
  gettimeofday(&time, NULL);
  packet.time_secs_utc = time.tv_sec;
  packet.time_nano_secs = time.tv_usec * 1000;
}

//////////////////////////////////////////////////////////////////
// check packet id for validity, swapping as required.
// returns 0 on success, -1 on failure

int iwrf_check_packet_id(si32 packetId, bool *isSwapped)

{

  if (isSwapped != NULL) {
    *isSwapped = false;
  }
  
  si32 id = packetId;
  if (iwrfIdIsSwapped(id)) {
    SWAP_array_32(&id, sizeof(si32));
    if (isSwapped != NULL) {
      *isSwapped = true;
    }
  }

  if (id >= IWRF_SYNC_ID &&
      id < IWRF_SYNC_ID + 1000) {
    return 0;
  }

  return -1;

}

//////////////////////////////////////////////////////////////////
// check packet id for validity, swapping in-place as required.
// also swaps the packet_len argument.
// returns 0 on success, -1 on failure

int iwrf_check_packet_id(si32 &packetId, si32 &packetLen, bool *isSwapped)

{

  if (isSwapped != NULL) {
    *isSwapped = false;
  }
  
  if (iwrfIdIsSwapped(packetId)) {
    SWAP_array_32(&packetId, sizeof(si32));
    SWAP_array_32(&packetLen, sizeof(si32));
    if (isSwapped != NULL) {
      *isSwapped = true;
    }
  }

  return iwrf_check_packet_id(packetId);

}

//////////////////////////////////////////////////////////////////
// get packet id, check for validity of this packet
// checks the packet length
// prints error in debug mode
// returns 0 on success, -1 on failure

int iwrf_get_packet_id(const void* buf, int len, int &packet_id)

{

  if (len < (int) sizeof(si32)) {
    return -1;
  }

  // get packet ID
  
  si32 id;
  memcpy(&id, buf, sizeof(si32));
  if (iwrfIdIsSwapped(id)) {
    SWAP_array_32(&id, sizeof(si32));
  }

  packet_id = id;

  int iret = 0;
  switch (packet_id) {

    case IWRF_SYNC_ID:
      if (len < (int) sizeof(iwrf_sync_t)) {
	iret = -1;
      } break;

    case IWRF_RADAR_INFO_ID:
      if (len < (int) sizeof(iwrf_radar_info_t)) {
	iret = -1;
      } break;

    case IWRF_SCAN_SEGMENT_ID:
      if (len < (int) sizeof(iwrf_scan_segment_t)) {
	iret = -1;
      } break;

    case IWRF_ANTENNA_CORRECTION_ID:
      if (len < (int) sizeof(iwrf_antenna_correction_t)) {
	iret = -1;
      } break;

    case IWRF_TS_PROCESSING_ID:
      if (len < (int) sizeof(iwrf_ts_processing_t)) {
	iret = -1;
      } break;

    case IWRF_XMIT_POWER_ID:
      if (len < (int) sizeof(iwrf_xmit_power_t)) {
	iret = -1;
      } break;

    case IWRF_RX_POWER_ID:
      if (len < (int) sizeof(iwrf_rx_power_t)) {
	iret = -1;
      } break;

    case IWRF_XMIT_SAMPLE_ID:
      if (len < (int) sizeof(iwrf_xmit_sample_t)) {
	iret = -1;
      } break;

    case IWRF_XMIT_SAMPLE_V2_ID:
      if (len < (int) sizeof(iwrf_xmit_sample_v2_t)) {
	iret = -1;
      } break;

    case IWRF_BURST_HEADER_ID:
      if (len < (int) sizeof(iwrf_burst_header_t)) {
	iret = -1;
      } break;

    case IWRF_STATUS_XML_ID:
      if (len < (int) sizeof(iwrf_status_xml_t)) {
	iret = -1;
      } break;

    case IWRF_ANTENNA_ANGLES_ID:
      if (len < (int) sizeof(iwrf_antenna_angles_t)) {
	iret = -1;
      } break;

    case IWRF_CALIBRATION_ID:
      if (len < (int) sizeof(iwrf_calibration_t)) {
	iret = -1;
      } break;

    case IWRF_EVENT_NOTICE_ID:
      if (len < (int) sizeof(iwrf_event_notice_t)) {
	iret = -1;
      } break;

    case IWRF_PHASECODE_ID:
      if (len < (int) sizeof(iwrf_phasecode_t)) {
	iret = -1;
      } break;

    case IWRF_XMIT_INFO_ID:
      if (len < (int) sizeof(iwrf_xmit_info_t)) {
	iret = -1;
      } break;

    case IWRF_PULSE_HEADER_ID:
      if (len < (int) sizeof(iwrf_pulse_header_t)) {
	iret = -1;
      } break;

    case IWRF_RVP8_PULSE_HEADER_ID:
      if (len < (int) sizeof(iwrf_rvp8_pulse_header_t)) {
	iret = -1;
      } break;

    case IWRF_RVP8_OPS_INFO_ID:
      if (len < (int) sizeof(iwrf_rvp8_ops_info_t)) {
	iret = -1;
      } break;
      
    case IWRF_MOMENTS_FIELD_HEADER_ID:
      if (len < (int) sizeof(iwrf_moments_field_header_t)) {
	iret = -1;
      } break;
      
    case IWRF_MOMENTS_RAY_HEADER_ID:
      if (len < (int) sizeof(iwrf_moments_ray_header_t)) {
	iret = -1;
      } break;
      
  }

  return iret;

}

//////////////////////////////////////////////////////////////////
// Check if packet buffer has correct radar id.
// Returns true if correct, false if not.
// A specified radarId of 0 will match all messages.

bool iwrf_check_radar_id(const void *buf,
                         int len,
                         int radarId)
  
{
  if (radarId == 0) {
    // matches all packets
    return true;
  }
  if (len < (int) sizeof(iwrf_packet_info_t)) {
    // too small for a valid packet
    return false;
  }
  iwrf_packet_info_t info;
  memcpy(&info, buf, sizeof(info));
  iwrf_packet_info_swap(info);
  if (info.radar_id == 0 ||
      info.radar_id == radarId) {
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////
// get packet time as a double

double iwrf_get_packet_time_as_double(const iwrf_packet_info_t &packet)

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

int iwrf_packet_swap(void *buf, int len)
{

  int packet_id;
  if (iwrf_get_packet_id(buf, len, packet_id)) {
    return -1;
  }

  switch (packet_id) {

    case IWRF_SYNC_ID:
      iwrf_sync_swap(*((iwrf_sync_t *) buf));
      break;

    case IWRF_RADAR_INFO_ID:
      iwrf_radar_info_swap(*((iwrf_radar_info_t *) buf));
      break;

    case IWRF_SCAN_SEGMENT_ID:
      iwrf_scan_segment_swap(*((iwrf_scan_segment_t *) buf));
      break;

    case IWRF_ANTENNA_CORRECTION_ID:
      iwrf_antenna_correction_swap(*((iwrf_antenna_correction_t *) buf));
      break;

    case IWRF_TS_PROCESSING_ID:
      iwrf_ts_processing_swap(*((iwrf_ts_processing_t *) buf));
      break;

    case IWRF_XMIT_POWER_ID:
      iwrf_xmit_power_swap(*((iwrf_xmit_power_t *) buf));
      break;

    case IWRF_RX_POWER_ID:
      iwrf_rx_power_swap(*((iwrf_rx_power_t *) buf));
      break;

    case IWRF_XMIT_SAMPLE_ID:
      iwrf_xmit_sample_swap(*((iwrf_xmit_sample_t *) buf));
      break;

    case IWRF_XMIT_SAMPLE_V2_ID:
      iwrf_xmit_sample_v2_swap(*((iwrf_xmit_sample_v2_t *) buf));
      break;

    case IWRF_BURST_HEADER_ID:
      iwrf_burst_header_swap(*((iwrf_burst_header_t *) buf));
      break;

    case IWRF_STATUS_XML_ID:
      iwrf_status_xml_swap(*((iwrf_status_xml_t *) buf));
      break;

    case IWRF_ANTENNA_ANGLES_ID:
      iwrf_antenna_angles_swap(*((iwrf_antenna_angles_t *) buf));
      break;

    case IWRF_CALIBRATION_ID:
      iwrf_calibration_swap(*((iwrf_calibration_t *) buf));
      break;

    case IWRF_EVENT_NOTICE_ID:
      iwrf_event_notice_swap(*((iwrf_event_notice_t *) buf));
      break;

    case IWRF_PHASECODE_ID:
      iwrf_phasecode_swap(*((iwrf_phasecode_t *) buf));
      break;

    case IWRF_XMIT_INFO_ID:
      iwrf_xmit_info_swap(*((iwrf_xmit_info_t *) buf));
      break;

    case IWRF_PULSE_HEADER_ID:
      iwrf_pulse_header_swap(*((iwrf_pulse_header_t *) buf));
      break;

    case IWRF_RVP8_PULSE_HEADER_ID:
      iwrf_rvp8_pulse_header_swap(*((iwrf_rvp8_pulse_header_t *) buf));
      break;

    case IWRF_RVP8_OPS_INFO_ID:
      iwrf_rvp8_ops_info_swap(*((iwrf_rvp8_ops_info_t *) buf));
      break;
      
  }

  return 0;

}

//////////////////////////////////////////////////////
// swap packet header
// returns true is swapped, false if already in native

bool iwrf_packet_info_swap(iwrf_packet_info_t &packet)
  
{
  if (iwrfIdIsSwapped(packet.id)) {
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

bool iwrf_version_swap(iwrf_version_t &version)

{
  bool swap = iwrf_packet_info_swap(version.packet);
  if (swap) {
    ui08 *start = (ui08 *) &version + sizeof(iwrf_packet_info_t);
    ui08 *end = (ui08 *) &version.version_name;
    int nbytes = end - start;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap sync
// returns true is swapped, false if already in native

bool iwrf_sync_swap(iwrf_sync_t &sync)

{
  // only swap the header
  // no data to be swapped since all bytes are identical
  return iwrf_packet_info_swap(sync.packet);
}

//////////////////////////////////////////////////////
// swap radar_info
// returns true is swapped, false if already in native

bool iwrf_radar_info_swap(iwrf_radar_info_t &radar_info)

{
  bool swap = iwrf_packet_info_swap(radar_info.packet);
  if (swap) {
    ui08 *start = (ui08 *) &radar_info + sizeof(iwrf_packet_info_t);
    ui08 *end = (ui08 *) &radar_info.radar_name;
    int nbytes = end - start;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap scan_segment
// returns true is swapped, false if already in native

bool iwrf_scan_segment_swap(iwrf_scan_segment_t &segment)

{
  bool swap = iwrf_packet_info_swap(segment.packet);
  if (swap) {
    ui08 *start = (ui08 *) &segment + sizeof(iwrf_packet_info_t);
    ui08 *end = (ui08 *) &segment.segment_name;
    int nbytes = end - start;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap antenna_correction
// returns true is swapped, false if already in native

bool iwrf_antenna_correction_swap(iwrf_antenna_correction_t &correction)

{
  bool swap = iwrf_packet_info_swap(correction.packet);
  if (swap) {
    ui08 *start = (ui08 *) &correction + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_antenna_correction_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap ts_processing
// returns true is swapped, false if already in native

bool iwrf_ts_processing_swap(iwrf_ts_processing_t &processing)

{
  bool swap = iwrf_packet_info_swap(processing.packet);
  if (swap) {
    ui08 *start = (ui08 *) &processing + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_ts_processing_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap xmit_power
// returns true is swapped, false if already in native

bool iwrf_xmit_power_swap(iwrf_xmit_power_t &power)

{
  bool swap = iwrf_packet_info_swap(power.packet);
  if (swap) {
    ui08 *start = (ui08 *) &power + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_xmit_power_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap rx_power
// returns true is swapped, false if already in native

bool iwrf_rx_power_swap(iwrf_rx_power_t &power)

{
  bool swap = iwrf_packet_info_swap(power.packet);
  if (swap) {
    ui08 *start = (ui08 *) &power + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_rx_power_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap xmit_sample
// returns true is swapped, false if already in native

bool iwrf_xmit_sample_swap(iwrf_xmit_sample_t &sample)

{
  bool swap = iwrf_packet_info_swap(sample.packet);
  if (swap) {
    ui08 *start = (ui08 *) &sample + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_xmit_sample_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

bool iwrf_xmit_sample_v2_swap(iwrf_xmit_sample_v2_t &sample)

{
  bool swap = iwrf_packet_info_swap(sample.packet);
  if (swap) {
    ui08 *start = (ui08 *) &sample + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_xmit_sample_v2_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap xmit_info
// returns true is swapped, false if already in native

bool iwrf_xmit_info_swap(iwrf_xmit_info_t &info)

{
  bool swap = iwrf_packet_info_swap(info.packet);
  if (swap) {
    ui08 *start = (ui08 *) &info + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_xmit_info_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap burst_iq
// returns true is swapped, false if already in native

bool iwrf_burst_header_swap(iwrf_burst_header_t &val)
  
{
  bool swap = iwrf_packet_info_swap(val.packet);
  if (swap) {
    SWAP_array_64(&val.pulse_seq_num, sizeof(si64));
    int nn = sizeof(iwrf_packet_info_t) + sizeof(si64);
    ui08 *start = (ui08 *) &val + nn;
    int nbytes = sizeof(iwrf_burst_header_t) - nn;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap status_xml
// returns true is swapped, false if already in native

bool iwrf_status_xml_swap(iwrf_status_xml_t &val)
  
{
  bool swap = iwrf_packet_info_swap(val.packet);
  if (swap) {
    ui08 *start = (ui08 *) &val + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_status_xml_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap antenna_angles
// returns true is swapped, false if already in native

bool iwrf_antenna_angles_swap(iwrf_antenna_angles_t &val)
  
{
  bool swap = iwrf_packet_info_swap(val.packet);
  if (swap) {
    ui08 *start = (ui08 *) &val + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_antenna_angles_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap calibration
// returns true is swapped, false if already in native

bool iwrf_calibration_swap(iwrf_calibration_t &calib)

{
  bool swap = iwrf_packet_info_swap(calib.packet);
  if (swap) {
    ui08 *start = (ui08 *) &calib + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_calibration_t)
      - sizeof(iwrf_packet_info_t) - IWRF_MAX_RADAR_NAME;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap event_notice
// returns true is swapped, false if already in native

bool iwrf_event_notice_swap(iwrf_event_notice_t &notice)

{
  bool swap = iwrf_packet_info_swap(notice.packet);
  if (swap) {
    ui08 *start = (ui08 *) &notice + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_event_notice_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap phasecode
// returns true is swapped, false if already in native

bool iwrf_phasecode_swap(iwrf_phasecode_t &code)

{
  bool swap = iwrf_packet_info_swap(code.packet);
  if (swap) {
    ui08 *start = (ui08 *) &code + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_phasecode_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap pulse_header
// returns true is swapped, false if already in native

bool iwrf_pulse_header_swap(iwrf_pulse_header_t &pulse)

{
  bool swap = iwrf_packet_info_swap(pulse.packet);
  if (swap) {
    SWAP_array_64(&pulse.pulse_seq_num, sizeof(si64));
    int nn = sizeof(iwrf_packet_info_t) + sizeof(si64);
    ui08 *start = (ui08 *) &pulse + nn;
    int nbytes = sizeof(iwrf_pulse_header_t) - nn;
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap rvp8_pulse_header
// returns true is swapped, false if already in native

bool iwrf_rvp8_pulse_header_swap(iwrf_rvp8_pulse_header_t &pulse)

{
  bool swap = iwrf_packet_info_swap(pulse.packet);

  if (swap) {

    pulse.i_tx_phase = SWAP_ui16(pulse.i_tx_phase);
    pulse.i_az = SWAP_ui16(pulse.i_az);
    pulse.i_el = SWAP_ui16(pulse.i_el);
    pulse.i_num_vecs = SWAP_si16(pulse.i_num_vecs);
    pulse.i_max_vecs = SWAP_si16(pulse.i_max_vecs);
    pulse.i_tg_wave = SWAP_ui16(pulse.i_tg_wave);

    pulse.i_btime_api = SWAP_ui32(pulse.i_btime_api);
    pulse.i_sys_time = SWAP_ui32(pulse.i_sys_time);
    pulse.i_prev_prt = SWAP_ui32(pulse.i_prev_prt);
    pulse.i_next_prt = SWAP_ui32(pulse.i_next_prt);
    pulse.i_seq_num = SWAP_ui32(pulse.i_seq_num);

    SWAP_array_32(pulse.uiq_perm, 2 * sizeof(ui32));
    SWAP_array_32(pulse.uiq_once, 2 * sizeof(ui32));

    SWAP_array_32(pulse.i_data_off, IWRF_MAX_CHAN * sizeof(si32));
    SWAP_array_32(pulse.f_burst_mag, IWRF_MAX_CHAN * sizeof(fl32));
    SWAP_array_16(pulse.i_burst_arg, IWRF_MAX_CHAN * sizeof(si16));
    SWAP_array_16(pulse.i_wrap_iq, IWRF_MAX_CHAN * sizeof(si16));

    SWAP_array_32(pulse.unused2, 23 * sizeof(si32));

  }
  return swap;
}

//////////////////////////////////////////////////////
// swap rvp8_ops_info
// returns true is swapped, false if already in native

bool iwrf_rvp8_ops_info_swap(iwrf_rvp8_ops_info_t &info)

{
  bool swap = iwrf_packet_info_swap(info.packet);

  if (swap) {

    info.i_version = SWAP_si32(info.i_version);


    info.i_major_mode = SWAP_ui32(info.i_major_mode);
    info.i_polarization = SWAP_ui32(info.i_polarization);
    info.i_phase_mode_seq = SWAP_ui32(info.i_phase_mode_seq);

    info.i_task_sweep = SWAP_ui16(info.i_task_sweep);
    info.i_task_aux_num = SWAP_ui16(info.i_task_aux_num);

    info.i_task_scan_type = SWAP_si32(info.i_task_scan_type);
    SWAP_array_32(info.unused1, 3 * sizeof(si32));

    info.i_aq_mode = SWAP_ui32(info.i_aq_mode);
    info.i_unfold_mode = SWAP_ui32(info.i_unfold_mode);

    info.i_pwidth_code = SWAP_ui32(info.i_pwidth_code);
    info.f_pwidth_usec = SWAP_fl32(info.f_pwidth_usec);

    info.f_dbz_calib = SWAP_fl32(info.f_dbz_calib);

    info.i_sample_size = SWAP_si32(info.i_sample_size);

    info.i_mean_angle_sync = SWAP_ui32(info.i_mean_angle_sync);
    info.i_flags = SWAP_ui32(info.i_flags);

    info.i_playback_version = SWAP_si32(info.i_playback_version);

    info.f_sy_clk_mhz = SWAP_fl32(info.f_sy_clk_mhz);
    info.f_wavelength_cm = SWAP_fl32(info.f_wavelength_cm);
    info.f_saturation_dbm = SWAP_fl32(info.f_saturation_dbm);
    info.f_range_mask_res = SWAP_fl32(info.f_range_mask_res);

    SWAP_array_16(info.i_range_mask, IWRF_RVP8_GATE_MASK_LEN * sizeof(si16));

    SWAP_array_32(info.f_noise_dbm, IWRF_MAX_CHAN * sizeof(fl32));
    SWAP_array_32(info.f_noise_stdv_db, IWRF_MAX_CHAN * sizeof(fl32));

    info.f_noise_range_km = SWAP_fl32(info.f_noise_range_km);
    info.f_noise_prf_hz = SWAP_fl32(info.f_noise_prf_hz);

    SWAP_array_16(info.i_gparm_latch_sts, 2 * sizeof(si16));
    SWAP_array_16(info.i_gparm_immed_sts, 6 * sizeof(si16));
    SWAP_array_16(info.i_gparm_diag_bits, 4 * sizeof(si16));

    SWAP_array_32(info.unused2, 185 * sizeof(si32));

  }
  return swap;
}

//////////////////////////////////////////////////////
// swap field_header
// returns true is swapped, false if already in native

bool iwrf_moments_field_header_swap(iwrf_moments_field_header_t &val)

{
  bool swap = iwrf_packet_info_swap(val.packet);
  if (swap) {
    ui08 *startVal = (ui08 *) &val;
    ui08 *startSwap = (ui08 *) &val.encoding;
    int nbytes = sizeof(val) - (startSwap - startVal);
    SWAP_array_32(startSwap, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap ray_header
// returns true is swapped, false if already in native

bool iwrf_moments_ray_header_swap(iwrf_moments_ray_header_t &val)

{
  bool swap = iwrf_packet_info_swap(val.packet);
  if (swap) {
    ui08 *startVal = (ui08 *) &val;
    ui08 *startSwap = (ui08 *) &val.volume_num;
    int nbytes = sizeof(val) - (startSwap - startVal);
    SWAP_array_32(startSwap, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap field_index
// returns true is swapped, false if already in native

bool iwrf_moments_field_index_swap(iwrf_moments_field_index_t &val)
  
{
  if (iwrfIdIsSwapped(val.id)) {
    int nbytes = sizeof(val) - IWRF_MAX_MOMENTS_FIELD_NAME;
    SWAP_array_32(&val.id, nbytes);
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////
// swap field_index
// returns true is swapped, false if already in native

bool iwrf_platform_georef_swap(iwrf_platform_georef_t &val)
  
{
  bool swap = iwrf_packet_info_swap(val.packet);
  if (swap) {
    ui08 *start = (ui08 *) &val + sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, 20 * sizeof(fl32));
    SWAP_array_64(&val.longitude, 2 * sizeof(fl64));
  }
  return swap;
}

//////////////////////////////////////////////////////
// swap field_index
// returns true is swapped, false if already in native

bool iwrf_georef_correction_swap(iwrf_georef_correction_t &val)
  
{
  bool swap = iwrf_packet_info_swap(val.packet);
  if (swap) {
    ui08 *start = (ui08 *) &val + sizeof(iwrf_packet_info_t);
    int nbytes = sizeof(iwrf_georef_correction_t) - sizeof(iwrf_packet_info_t);
    SWAP_array_32(start, nbytes);
  }
  return swap;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// string representation of enums

// string representation of packet_id 

string iwrf_packet_id_to_str(int packet_id)

{
  
  switch (packet_id) {
    case IWRF_SYNC_ID: return "IWRF_SYNC_ID";
    case IWRF_RADAR_INFO_ID: return "IWRF_RADAR_INFO_ID";
    case IWRF_SCAN_SEGMENT_ID: return "IWRF_SCAN_SEGMENT_ID";
    case IWRF_ANTENNA_CORRECTION_ID: return "IWRF_ANTENNA_CORRECTION_ID";
    case IWRF_TS_PROCESSING_ID: return "IWRF_TS_PROCESSING_ID";
    case IWRF_XMIT_POWER_ID: return "IWRF_XMIT_POWER_ID";
    case IWRF_RX_POWER_ID: return "IWRF_RX_POWER_ID";
    case IWRF_XMIT_SAMPLE_ID: return "IWRF_XMIT_SAMPLE_ID";
    case IWRF_XMIT_SAMPLE_V2_ID: return "IWRF_XMIT_SAMPLE_V2_ID";
    case IWRF_BURST_HEADER_ID: return "IWRF_BURST_HEADER_ID";
    case IWRF_STATUS_XML_ID: return "IWRF_STATUS_XML_ID";
    case IWRF_ANTENNA_ANGLES_ID: return "IWRF_ANTENNA_ANGLES_ID";
    case IWRF_CALIBRATION_ID: return "IWRF_CALIBRATION_ID";
    case IWRF_EVENT_NOTICE_ID: return "IWRF_EVENT_NOTICE_ID";
    case IWRF_PHASECODE_ID: return "IWRF_PHASECODE_ID";
    case IWRF_XMIT_INFO_ID: return "IWRF_XMIT_INFO_ID";
    case IWRF_PULSE_HEADER_ID: return "IWRF_PULSE_HEADER_ID";
    case IWRF_VERSION_ID: return "IWRF_VERSION_ID";
    case IWRF_UI_OPERATIONS_ID: return "IWRF_UI_OPERATIONS_ID";
    case IWRF_ANT_CONTROL_CONSTANTS_ID: return "IWRF_ANT_CONTROL_CONSTANTS_ID";
    case IWRF_RVP8_PULSE_HEADER_ID: return "IWRF_RVP8_PULSE_HEADER_ID";
    case IWRF_RVP8_OPS_INFO_ID: return "IWRF_RVP8_OPS_INFO_ID";
    case IWRF_MOMENTS_FIELD_HEADER_ID: return "IWRF_MOMENTS_FIELD_HEADER_ID";
    case IWRF_MOMENTS_RAY_HEADER_ID: return "IWRF_MOMENTS_RAY_HEADER_ID";
    case IWRF_MOMENTS_FIELD_INDEX_ID: return "IWRF_MOMENTS_FIELD_INDEX_ID";
    case IWRF_PLATFORM_GEOREF_ID: return "IWRF_PLATFORM_GEOREF_ID";
    case IWRF_GEOREF_CORRECTION_ID: return "IWRF_GEOREF_CORRECTION_ID";
    default: return "UNKNOWN";
  }

}

// string representation of xmit_rcv_mode

string iwrf_xmit_rcv_mode_to_str(int xmit_rcv_mode)

{
  
  switch (xmit_rcv_mode) {
    case IWRF_SINGLE_POL: return "IWRF_SINGLE_POL";
    case IWRF_ALT_HV_CO_ONLY: return "IWRF_ALT_HV_CO_ONLY";
    case IWRF_ALT_HV_CO_CROSS: return "IWRF_ALT_HV_CO_CROSS";
    case IWRF_ALT_HV_FIXED_HV: return "IWRF_ALT_HV_FIXED_HV";
    case IWRF_SIM_HV_FIXED_HV: return "IWRF_SIM_HV_FIXED_HV";
    case IWRF_SIM_HV_SWITCHED_HV: return "IWRF_SIM_HV_SWITCHED_HV";
    case IWRF_H_ONLY_FIXED_HV: return "IWRF_H_ONLY_FIXED_HV";
    case IWRF_V_ONLY_FIXED_HV: return "IWRF_V_ONLY_FIXED_HV";
    case IWRF_ALT_HHVV_FIXED_HV: return "IWRF_ALT_HHVV_FIXED_HV";
    case IWRF_SINGLE_POL_V: return "IWRF_SINGLE_POL_V";
    default: return "UNKNOWN";
  }

}

// string representation of iwrf_xmit_phase_mode

string iwrf_xmit_phase_mode_to_str(int xmit_phase_mode)

{
  
  switch (xmit_phase_mode) {
    case IWRF_XMIT_PHASE_MODE_FIXED: return "IWRF_XMIT_PHASE_MODE_FIXED";
    case IWRF_XMIT_PHASE_MODE_RANDOM: return "IWRF_XMIT_PHASE_MODE_RANDOM";
    case IWRF_XMIT_PHASE_MODE_SZ864: return "IWRF_XMIT_PHASE_MODE_SZ864";
    default: return "UNKNOWN";
  }

}

// string representation of prf_mode

string iwrf_prf_mode_to_str(int prf_mode)

{
  
  switch (prf_mode) {
    case IWRF_PRF_MODE_FIXED: return "IWRF_PRF_MODE_FIXED";
    case IWRF_PRF_MODE_STAGGERED_2_3: return "IWRF_PRF_MODE_STAGGERED_2_3";
    case IWRF_PRF_MODE_STAGGERED_3_4: return "IWRF_PRF_MODE_STAGGERED_3_4";
    case IWRF_PRF_MODE_STAGGERED_4_5: return "IWRF_PRF_MODE_STAGGERED_4_5";
    case IWRF_PRF_MODE_MULTI_PRT: return "IWRF_PRF_MODE_MULTI_PRT";
    case IWRF_PRF_MODE_BLOCK_MODE: return "IWRF_PRF_MODE_BLOCK_MODE";
    default: return "UNKNOWN";
  }

}

// string representation of pulse_type

string iwrf_pulse_type_to_str(int pulse_type)
  
{
  
  switch (pulse_type) {
    case IWRF_PULSE_TYPE_RECT: return "IWRF_PULSE_TYPE_RECT";
    case IWRF_PULSE_TYPE_GAUSSIAN: return "IWRF_PULSE_TYPE_GAUSSIAN";
    default: return "UNKNOWN";
  }

}

// string representation of pulse_polarization

string iwrf_pol_mode_to_str(int pol_mode)

{
  
  switch (pol_mode) {
    case IWRF_POL_MODE_H: return "IWRF_POL_MODE_H";
    case IWRF_POL_MODE_V: return "IWRF_POL_MODE_V";
    case IWRF_POL_MODE_HV_ALT: return "IWRF_POL_MODE_HV_ALT";
    case IWRF_POL_MODE_HV_SIM: return "IWRF_POL_MODE_HV_SIM";
    default: return "UNKNOWN";
  }

}

// string representation of scan_mode

string iwrf_scan_mode_to_str(int scan_mode)

{
  
  switch (scan_mode) {
    case IWRF_SCAN_MODE_SECTOR: return "IWRF_SCAN_MODE_SECTOR";
    case IWRF_SCAN_MODE_COPLANE: return "IWRF_SCAN_MODE_COPLANE";
    case IWRF_SCAN_MODE_RHI: return "IWRF_SCAN_MODE_RHI";
    case IWRF_SCAN_MODE_VERTICAL_POINTING:
      return "IWRF_SCAN_MODE_VERTICAL_POINTING";
    case IWRF_SCAN_MODE_IDLE: return "IWRF_SCAN_MODE_IDLE";
    case IWRF_SCAN_MODE_AZ_SUR_360: return "IWRF_SCAN_MODE_AZ_SUR_360";
    case IWRF_SCAN_MODE_EL_SUR_360: return "IWRF_SCAN_MODE_EL_SUR_360";
    case IWRF_SCAN_MODE_SUNSCAN: return "IWRF_SCAN_MODE_SUNSCAN";
    case IWRF_SCAN_MODE_POINTING: return "IWRF_SCAN_MODE_POINTING";
    case IWRF_SCAN_MODE_FOLLOW_VEHICLE: return "IWRF_SCAN_MODE_FOLLOW_VEHICLE";
    case IWRF_SCAN_MODE_EL_SURV: return "IWRF_SCAN_MODE_EL_SURV";
    case IWRF_SCAN_MODE_MANPPI: return "IWRF_SCAN_MODE_MANPPI";
    case IWRF_SCAN_MODE_MANRHI: return "IWRF_SCAN_MODE_MANRHI";
    case IWRF_SCAN_MODE_SUNSCAN_RHI: return "IWRF_SCAN_MODE_SUNSCAN_RHI";
    default: return "UNKNOWN";
  }

}

string iwrf_scan_mode_to_short_str(int scan_mode)

{
  
  switch (scan_mode) {
    case IWRF_SCAN_MODE_SECTOR: return "SECTOR";
    case IWRF_SCAN_MODE_COPLANE: return "COPLANE";
    case IWRF_SCAN_MODE_RHI: return "RHI";
    case IWRF_SCAN_MODE_VERTICAL_POINTING: return "VERT";
    case IWRF_SCAN_MODE_IDLE: return "IDLE";
    case IWRF_SCAN_MODE_AZ_SUR_360: return "AZ_SUR";
    case IWRF_SCAN_MODE_EL_SUR_360: return "EL_SUR";
    case IWRF_SCAN_MODE_SUNSCAN: return "SUN";
    case IWRF_SCAN_MODE_POINTING: return "POINT";
    case IWRF_SCAN_MODE_FOLLOW_VEHICLE: return "VEHICLE";
    case IWRF_SCAN_MODE_EL_SURV: return "EL_SUR";
    case IWRF_SCAN_MODE_MANPPI: return "MANPPI";
    case IWRF_SCAN_MODE_MANRHI: return "MANRHI";
    case IWRF_SCAN_MODE_SUNSCAN_RHI: return "SUN_RHI";
    default: return "UNKNOWN";
  }

}

// string representation of follow_mode

string iwrf_follow_mode_to_str(int follow_mode)

{
  
  switch (follow_mode) {
    case IWRF_FOLLOW_MODE_NONE: return "IWRF_FOLLOW_MODE_NONE";
    case IWRF_FOLLOW_MODE_SUN: return "IWRF_FOLLOW_MODE_SUN";
    case IWRF_FOLLOW_MODE_VEHICLE: return "IWRF_FOLLOW_MODE_VEHICLE";
    case IWRF_FOLLOW_MODE_AIRCRAFT: return "IWRF_FOLLOW_MODE_AIRCRAFT";
    case IWRF_FOLLOW_MODE_TARGET: return "IWRF_FOLLOW_MODE_TARGET";
    case IWRF_FOLLOW_MODE_MANUAL: return "IWRF_FOLLOW_MODE_MANUAL";
    default: return "UNKNOWN";
  }

}

// string representation of radar_platform

string iwrf_radar_platform_to_str(int radar_platform)

{
  
  switch (radar_platform) {
    case IWRF_RADAR_PLATFORM_FIXED: return "IWRF_RADAR_PLATFORM_FIXED";
    case IWRF_RADAR_PLATFORM_VEHICLE: return "IWRF_RADAR_PLATFORM_VEHICLE";
    case IWRF_RADAR_PLATFORM_SHIP: return "IWRF_RADAR_PLATFORM_SHIP";
    case IWRF_RADAR_PLATFORM_AIRCRAFT: return "IWRF_RADAR_PLATFORM_AIRCRAFT";
    default: return "UNKNOWN";
  }

}

// string representation of cal_type
  
string iwrf_cal_type_to_str(int cal_type)

{
  
  switch (cal_type) {
    case IWRF_CAL_TYPE_NONE: return "IWRF_CAL_TYPE_NONE";
    case IWRF_CAL_TYPE_CW_CAL: return "IWRF_CAL_TYPE_CW_CAL";
    case IWRF_CAL_TYPE_SOLAR_CAL_FIXED: return "IWRF_CAL_TYPE_SOLAR_CAL_FIXED";
    case IWRF_CAL_TYPE_SOLAR_CAL_SCAN: return "IWRF_CAL_TYPE_SOLAR_CAL_SCAN";
    case IWRF_CAL_TYPE_NOISE_SOURCE_H: return "IWRF_CAL_TYPE_NOISE_SOURCE_H";
    case IWRF_CAL_TYPE_NOISE_SOURCE_V: return "IWRF_CAL_TYPE_NOISE_SOURCE_V";
    case IWRF_CAL_TYPE_NOISE_SOURCE_HV: return "IWRF_CAL_TYPE_NOISE_SOURCE_HV";
    case IWRF_CAL_TYPE_BLUESKY: return "IWRF_CAL_TYPE_BLUESKY";
    case IWRF_CAL_TYPE_SAVEPARAMS: return "IWRF_CAL_TYPE_SAVEPARAMS";
    default: return "UNKNOWN";
  }

}

// string representation of event_cause

string iwrf_event_cause_to_str(int event_cause)

{
  
  switch (event_cause) {
    case IWRF_EVENT_CAUSE_DONE: return "IWRF_EVENT_CAUSE_DONE";
    case IWRF_EVENT_CAUSE_TIMEOUT: return "IWRF_EVENT_CAUSE_TIMEOUT";
    case IWRF_EVENT_CAUSE_TIMER: return "IWRF_EVENT_CAUSE_TIMER";
    case IWRF_EVENT_CAUSE_ABORT: return "IWRF_EVENT_CAUSE_ABORT";
    case IWRF_EVENT_CAUSE_SCAN_ABORT: return "IWRF_EVENT_CAUSE_SCAN_ABORT";
    case IWRF_EVENT_CAUSE_RESTART: return "IWRF_EVENT_CAUSE_RESTART";
    default: return "UNKNOWN";
  }

}

// string representation of iq_encoding

string iwrf_iq_encoding_to_str(int iq_encoding)

{
  
  switch (iq_encoding) {
    case IWRF_IQ_ENCODING_FL32: return "IWRF_IQ_ENCODING_FL32";
    case IWRF_IQ_ENCODING_SCALED_SI16: return "IWRF_IQ_ENCODING_SCALED_SI16";
    case IWRF_IQ_ENCODING_DBM_PHASE_SI16:
      return "IWRF_IQ_ENCODING_DBM_PHASE_SI16";
    case IWRF_IQ_ENCODING_SIGMET_FL16: return "IWRF_IQ_ENCODING_SIGMET_FL16";
    case IWRF_IQ_ENCODING_SCALED_SI32: return "IWRF_IQ_ENCODING_SCALED_SI32";
    default: return "UNKNOWN";
  }

}

// string representation of moments_encoding

string iwrf_moments_encoding_to_str(int moments_encoding)

{
  
  switch (moments_encoding) {
    case IWRF_MOMENTS_ENCODING_FL64: return "IWRF_MOMENTS_ENCODING_FL64";
    case IWRF_MOMENTS_ENCODING_FL32: return "IWRF_MOMENTS_ENCODING_FL32";
    case IWRF_MOMENTS_ENCODING_SCALED_SI32: return "IWRF_MOMENTS_ENCODING_SCALED_SI32";
    case IWRF_MOMENTS_ENCODING_SCALED_SI16: return "IWRF_MOMENTS_ENCODING_SCALED_SI16";
    case IWRF_MOMENTS_ENCODING_SCALED_SI08: return "IWRF_MOMENTS_ENCODING_SCALED_SI08";
    default: return "UNKNOWN";
  }

}

// string representation of ui_error

string iwrf_ui_error_to_str(int ui_error)

{
  
  switch (ui_error) {

    case IWRF_UI_ERROR_DELETE_FAILED: 
      return "IWRF_UI_ERROR_DELETE_FAILED";
    case IWRF_UI_ERROR_TASKLIST_SIZE:
      return "IWRF_UI_ERROR_TASKLIST_SIZE";
    case IWRF_UI_ERROR_MISSING_TASK_NAME:
      return "IWRF_UI_ERROR_MISSING_TASK_NAME";
    case IWRF_UI_ERROR_TASKLIST_INDEX_RANGE:
      return "IWRF_UI_ERROR_TASKLIST_INDEX_RANGE";
    case IWRF_UI_ERROR_TASKLIST_FULL:
      return "IWRF_UI_ERROR_TASKLIST_FULL";
    case IWRF_UI_ERROR_APPEND_TASK_UNDEFINED:
      return "IWRF_UI_ERROR_APPEND_TASK_UNDEFINED";
    case IWRF_UI_ERROR_APPEND_TASK_NONAME:
      return "IWRF_UI_ERROR_APPEND_TASK_NONAME";
    case IWRF_UI_ERROR_CANT_SCHEDULE:
      return "IWRF_UI_ERROR_CANT_SCHEDULE";
    case IWRF_UI_ERROR_REPEAT_CYCLE_RANGE:
      return "IWRF_UI_ERROR_REPEAT_CYCLE_RANGE";
    case IWRF_UI_ERROR_TASKLIST_ENTRY_UNKNOWN:
      return "IWRF_UI_ERROR_TASKLIST_ENTRY_UNKNOWN";
    case IWRF_UI_ERROR_TASKLIST_ENTRY_UNDEFINED:
      return "IWRF_UI_ERROR_TASKLIST_ENTRY_UNDEFINED";
    case IWRF_UI_ERROR_CANT_SCHEDULE_TASK_NOT_IN_TASKLIST:
      return "IWRF_UI_ERROR_CANT_SCHEDULE_TASK_NOT_IN_TASKLIST";
    case IWRF_ANTCON_NOT_CONNECTED:
      return "IWRF_ANTCON_NOT_CONNECTED";
    case IWRF_ANTENA_FAULTED:
      return "IWRF_ANTENA_FAULTED";
    case IWRF_TXCTRL_NOT_CONNECTED:
      return "IWRF_TXCTRL_NOT_CONNECTED";
    case IWRF_SCAN_STATE_TIMEOUT:
      return "IWRF_SCAN_STATE_TIMEOUT";
    case IWRF_SCAN_SEGMENT_LIST_ERROR:
      return "IWRF_SCAN_SEGMENT_LIST_ERROR";
    default:
      return "UNKNOWN";
  }

}

// string representation of ui_error

string iwrf_ui_opcode_to_str(int ui_opcode)

{
  
  switch (ui_opcode) {

    case IWRF_UI_UPD_SCAN_SEGMENT:
      return "IWRF_UI_UPD_SCAN_SEGMENT";
    case IWRF_UI_UPD_TS_PROCESSING:
      return "IWRF_UI_UPD_TS_PROCESSING";
    case IWRF_UI_UPD_DELETE_TASK:
      return "IWRF_UI_UPD_DELETE_TASK";
    case IWRF_UI_UPD_SEGMENT_SCHEDULE:
      return "IWRF_UI_UPD_SEGMENT_SCHEDULE";
    case IWRF_UI_GO_IDLE:
      return "IWRF_UI_GO_IDLE";
    case IWRF_UI_TASKLIST_UPD_LIST:
      return "IWRF_UI_TASKLIST_UPD_LIST";
    case IWRF_UI_TASKLIST_GET_UNUSED_LIST:
      return "IWRF_UI_TASKLIST_GET_UNUSED_LIST";
    case IWRF_UI_TASKLIST_GET_CURRENT_INDEX:
      return "IWRF_UI_TASKLIST_GET_CURRENT_INDEX";
    case IWRF_UI_TASKLIST_UPD_NEXT_INDEX:
      return "IWRF_UI_TASKLIST_UPD_NEXT_INDEX";
    case IWRF_UI_TASKLIST_SET_INDEX_IMMEDIATE:
      return "IWRF_UI_TASKLIST_SET_INDEX_IMMEDIATE";
    case IWRF_UI_TASKLIST_UPD_LIST_SCHEDULE:
      return "IWRF_UI_TASKLIST_UPD_LIST_SCHEDULE";
    case IWRF_UI_TASKLIST_GET_LIST_SCHEDULE:
      return "IWRF_UI_TASKLIST_GET_LIST_SCHEDULE";
    case IWRF_UI_TASKLIST_REMOVE_ALL:
      return "IWRF_UI_TASKLIST_REMOVE_ALL";
    case IWRF_UI_TASKLIST_REMOVE_ONE:
      return "IWRF_UI_TASKLIST_REMOVE_ONE";
    case IWRF_UI_TASKLIST_APPEND:
      return "IWRF_UI_TASKLIST_APPEND";
    case IWRF_UI_TASKLIST_EXCHANGE:
      return "IWRF_UI_TASKLIST_EXCHANGE";
    case IWRF_UI_UPD_GLOBAL:
      return "IWRF_UI_UPD_GLOBAL";
    case IWRF_UI_UPD_RSM_PACKET:
      return "IWRF_UI_UPD_RSM_PACKET";
    case IWRF_UI_RCS_STATUS:
      return "IWRF_UI_RCS_STATUS";
    case IWRF_UI_SHUTDOWN_CONNECTION:
      return "IWRF_UI_SHUTDOWN_CONNECTION";
    case IWRF_UI_SET_POL_MODE:
      return "IWRF_UI_SET_POL_MODE";
    case IWRF_UI_CONFIRM_POL_MODE:
      return "IWRF_UI_CONFIRM_POL_MODE";
    default:
      return "UNKNOWN";
  }

}

// string representation of iwrf_xmit_phase_mode

string iwrf_sample_units_to_str(int sample_units)

{
  
  switch (sample_units) {
    case IWRF_SAMPLE_UNITS_COUNTS: return "IWRF_SAMPLE_UNITS_COUNTS";
    case IWRF_SAMPLE_UNITS_VOLTS: return "IWRF_SAMPLE_UNITS_VOLTS";
    default: return "UNKNOWN";
  }

}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// printing routines

////////////////////////////////
// print depending on packet type

void iwrf_packet_print(FILE *out, const void *buf, int len)

{
  
  int packet_id;
  if (iwrf_get_packet_id(buf, len, packet_id)) {
    return;
  }

  switch (packet_id) {

    case IWRF_SYNC_ID:
      iwrf_sync_print(out, *((iwrf_sync_t *) buf));
      break;

    case IWRF_RADAR_INFO_ID:
      iwrf_radar_info_print(out, *((iwrf_radar_info_t *) buf));
      break;

    case IWRF_SCAN_SEGMENT_ID:
      iwrf_scan_segment_print(out, *((iwrf_scan_segment_t *) buf));
      break;

    case IWRF_ANTENNA_CORRECTION_ID:
      iwrf_antenna_correction_print(out, *((iwrf_antenna_correction_t *) buf));
      break;

    case IWRF_TS_PROCESSING_ID:
      iwrf_ts_processing_print(out, *((iwrf_ts_processing_t *) buf));
      break;

    case IWRF_XMIT_POWER_ID:
      iwrf_xmit_power_print(out, *((iwrf_xmit_power_t *) buf));
      break;

    case IWRF_RX_POWER_ID:
      iwrf_rx_power_print(out, *((iwrf_rx_power_t *) buf));
      break;

    case IWRF_XMIT_SAMPLE_ID:
      iwrf_xmit_sample_print(out, *((iwrf_xmit_sample_t *) buf));
      break;

    case IWRF_XMIT_SAMPLE_V2_ID:
      iwrf_xmit_sample_v2_print(out, *((iwrf_xmit_sample_v2_t *) buf));
      break;

    case IWRF_BURST_HEADER_ID:
      iwrf_burst_header_print(out, *((iwrf_burst_header_t *) buf));
      break;

    case IWRF_STATUS_XML_ID:
      iwrf_status_xml_print(out, buf);
      break;

    case IWRF_ANTENNA_ANGLES_ID:
      iwrf_antenna_angles_print(out, *((iwrf_antenna_angles_t *) buf));
      break;

    case IWRF_CALIBRATION_ID:
      iwrf_calibration_print(out, *((iwrf_calibration_t *) buf));
      break;

    case IWRF_EVENT_NOTICE_ID:
      iwrf_event_notice_print(out, *((iwrf_event_notice_t *) buf));
      break;

    case IWRF_PHASECODE_ID:
      iwrf_phasecode_print(out, *((iwrf_phasecode_t *) buf));
      break;

    case IWRF_XMIT_INFO_ID:
      iwrf_xmit_info_print(out, *((iwrf_xmit_info_t *) buf));
      break;

    case IWRF_PULSE_HEADER_ID:
      iwrf_pulse_header_print(out, *((iwrf_pulse_header_t *) buf));
      break;

    case IWRF_RVP8_PULSE_HEADER_ID:
      iwrf_rvp8_pulse_header_print(out, *((iwrf_rvp8_pulse_header_t *) buf));
      break;

    case IWRF_RVP8_OPS_INFO_ID:
      iwrf_rvp8_ops_info_print(out, *((iwrf_rvp8_ops_info_t *) buf));
      break;
      
    case IWRF_MOMENTS_FIELD_HEADER_ID:
      iwrf_moments_field_header_print(out,
                                      *((iwrf_moments_field_header_t *) buf));
      break;
      
    case IWRF_MOMENTS_RAY_HEADER_ID:
      iwrf_moments_ray_header_print(out, 
                                    *((iwrf_moments_ray_header_t *) buf));
      break;
      
    case IWRF_MOMENTS_FIELD_INDEX_ID:
      iwrf_moments_field_index_print(out,
                                     *((iwrf_moments_field_index_t *) buf));
      break;
      
    case IWRF_PLATFORM_GEOREF_ID:
      iwrf_platform_georef_print(out, *((iwrf_platform_georef_t *) buf));
      break;
      
    case IWRF_GEOREF_CORRECTION_ID:
      iwrf_georef_correction_print(out, *((iwrf_georef_correction_t *) buf));
      break;
      
  }

}

//////////////////////////////////////////////////////
// print packet header

void iwrf_packet_info_print(FILE *out,
			    const iwrf_packet_info_t &packet)

{

  iwrf_packet_info_t copy = packet;
  iwrf_packet_info_swap(copy);
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
// print radar_info

void iwrf_radar_info_print(FILE *out,
			   const iwrf_radar_info_t &info)

{

  iwrf_radar_info_t copy = info;
  iwrf_radar_info_swap(copy);
  fprintf(out, "==================== iwrf_radar_info ============================\n");
  iwrf_packet_info_print(out, copy.packet);

  fprintf(out, "  latitude_deg: %g\n", copy.latitude_deg);
  fprintf(out, "  longitude_deg: %g\n", copy.longitude_deg);
  fprintf(out, "  altitude_m: %g\n", copy.altitude_m);
  fprintf(out, "  platform_type: %s\n",
	  iwrf_radar_platform_to_str(copy.platform_type).c_str());
  fprintf(out, "  beamwidth_deg_h: %g\n", copy.beamwidth_deg_h);
  fprintf(out, "  beamwidth_deg_v: %g\n", copy.beamwidth_deg_v);
  fprintf(out, "  wavelength_cm: %g\n", copy.wavelength_cm);
  fprintf(out, "  nominal_gain_ant_db_h: %g\n", copy.nominal_gain_ant_db_h);
  fprintf(out, "  nominal_gain_ant_db_v: %g\n", copy.nominal_gain_ant_db_v);
  fprintf(out, "  radar_name: %s\n",
	  iwrf_safe_str(copy.radar_name, IWRF_MAX_RADAR_NAME).c_str());
  fprintf(out, "  site_name: %s\n",
	  iwrf_safe_str(copy.site_name, IWRF_MAX_SITE_NAME).c_str());
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print sync packet

void iwrf_sync_print(FILE *out,
		     const iwrf_sync_t &sync)

{

  iwrf_sync_t copy = sync;
  iwrf_sync_swap(copy);

  fprintf(out, "==================== iwrf_sync ==================================\n");
  iwrf_packet_info_print(out, copy.packet);
  fprintf(out, "  magik[0]: 0x%x\n", copy.magik[0]);
  fprintf(out, "  magik[1]: 0x%x\n", copy.magik[1]);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print version packet

void iwrf_version_print(FILE *out,
                        const iwrf_version_t &version)

{

  iwrf_version_t copy = version;
  iwrf_version_swap(copy);
  fprintf(out, "==================== iwrf_version ===============================\n");
  iwrf_packet_info_print(out, copy.packet);
  fprintf(out, "  major_version_num: %d\n", copy.major_version_num);
  fprintf(out, "  minor_version_num: %d\n", copy.minor_version_num);
  fprintf(out, "  version_name: %s\n", copy.version_name);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print scan_segment

void iwrf_scan_segment_print(FILE *out,
                             const iwrf_scan_segment_t &seg)

{

  iwrf_scan_segment_t copy = seg;
  iwrf_scan_segment_swap(copy);
  fprintf(out, "==================== iwrf_scan_segment ==========================\n");
  iwrf_packet_info_print(out, copy.packet);

  fprintf(out, "  scan_mode: %s\n", iwrf_scan_mode_to_str(copy.scan_mode).c_str());
  fprintf(out, "  follow_mode: %s\n", iwrf_follow_mode_to_str(copy.follow_mode).c_str());
  fprintf(out, "  volume_num: %d\n", copy.volume_num);
  fprintf(out, "  sweep_num: %d\n", copy.sweep_num);
  fprintf(out, "  time_limit: %d\n", copy.time_limit);
  fprintf(out, "  az_manual: %g\n", copy.az_manual);
  fprintf(out, "  el_manual: %g\n", copy.el_manual);
  fprintf(out, "  az_start: %g\n", copy.az_start);
  fprintf(out, "  el_start: %g\n", copy.el_start);
  fprintf(out, "  scan_rate: %g\n", copy.scan_rate);
  fprintf(out, "  left_limit: %g\n", copy.left_limit);
  fprintf(out, "  right_limit: %g\n", copy.right_limit);
  fprintf(out, "  up_limit: %g\n", copy.up_limit);
  fprintf(out, "  down_limit: %g\n", copy.down_limit);
  fprintf(out, "  step: %g\n", copy.step);
  fprintf(out, "  current_fixed_angle: %g\n", copy.current_fixed_angle);
  fprintf(out, "  init_direction_cw: %d\n", copy.init_direction_cw);
  fprintf(out, "  init_direction_up: %d\n", copy.init_direction_up);
  fprintf(out, "  n_sweeps: %d\n", copy.n_sweeps);

  fprintf(out, "  fixed_angles:");
  int nSweeps = copy.n_sweeps;
  if (nSweeps > IWRF_MAX_FIXED_ANGLES) {
    fprintf(out, " Bad number of sweeps\n");
  } else {
    for (int ii = 0; ii < nSweeps; ii++) {
      fprintf(out, " %g", copy.fixed_angles[ii]);
    }
    fprintf(out, "\n");
  }

  fprintf(out, "  optimizer_rmax_km: %g\n", copy.optimizer_rmax_km);
  fprintf(out, "  optimizer_htmax_km: %g\n", copy.optimizer_htmax_km);
  fprintf(out, "  optimizer_res_m: %g\n", copy.optimizer_res_m);

  fprintf(out, "  sun_scan_sector_width_az: %g\n", copy.sun_scan_sector_width_az);
  fprintf(out, "  sun_scan_sector_width_el: %g\n", copy.sun_scan_sector_width_el);

  fprintf(out, "  timeseries_archive_enable: %d\n", copy.timeseries_archive_enable);
  fprintf(out, "  waveguide_switch_position: %d\n", copy.waveguide_switch_position);
  fprintf(out, "  start_ppi_ccw: %d\n", copy.start_ppi_ccw);

  fprintf(out, "  segment_name: %s\n",
	  iwrf_safe_str(copy.segment_name, IWRF_MAX_SEGMENT_NAME).c_str());
  fprintf(out, "  project_name: %s\n",
	  iwrf_safe_str(copy.project_name, IWRF_MAX_PROJECT_NAME).c_str());
  
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print antenna_correction

void iwrf_antenna_correction_print(FILE *out,
				   const iwrf_antenna_correction_t &corr)

{

  iwrf_antenna_correction_t copy = corr;
  iwrf_antenna_correction_swap(copy);
  fprintf(out, "==================== iwrf_antenna_correction ====================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  az_correction: %g\n", copy.az_correction);
  fprintf(out, "  el_correction: %g\n", copy.el_correction);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print ts_processing

void iwrf_ts_processing_print(FILE *out,
			      const iwrf_ts_processing_t &proc)

{

  iwrf_ts_processing_t copy = proc;
  iwrf_ts_processing_swap(copy);
  fprintf(out, "==================== iwrf_ts_processing =========================\n");
  iwrf_packet_info_print(out, copy.packet);

  fprintf(out, "  xmit_rcv_mode: %s\n",
	  iwrf_xmit_rcv_mode_to_str(copy.xmit_rcv_mode).c_str());
  fprintf(out, "  xmit_phase_mode: %s\n",
	  iwrf_xmit_phase_mode_to_str(copy.xmit_phase_mode).c_str());
  fprintf(out, "  prf_mode: %s\n",
          iwrf_prf_mode_to_str(copy.prf_mode).c_str());
  fprintf(out, "  pulse_type: %s\n",
          iwrf_pulse_type_to_str(copy.pulse_type).c_str());

  fprintf(out, "  prt_usec: %g\n", copy.prt_usec);
  fprintf(out, "  prt2_usec: %g\n", copy.prt2_usec);

  fprintf(out, "  cal_type: %s\n",
          iwrf_cal_type_to_str(copy.cal_type).c_str());
  
  fprintf(out, "  burst_range_offset_m: %g\n", copy.burst_range_offset_m);
  fprintf(out, "  pulse_width_us: %g\n", copy.pulse_width_us);
  fprintf(out, "  start_range_m: %g\n", copy.start_range_m);
  fprintf(out, "  gate_spacing_m: %g\n", copy.gate_spacing_m);

  fprintf(out, "  integration_cycle_pulses: %d\n",
          copy.integration_cycle_pulses);
  fprintf(out, "  clutter_filter_number: %d\n", copy.clutter_filter_number);
  fprintf(out, "  range_gate_averaging: %d\n", copy.range_gate_averaging);
  fprintf(out, "  max_gate: %d\n", copy.max_gate);

  fprintf(out, "  test_power_dbm: %g\n", copy.test_power_dbm);

  fprintf(out, "  test_pulse_range_km: %g\n", copy.test_pulse_range_km);
  fprintf(out, "  test_pulse_length_usec: %g\n", copy.test_pulse_length_usec);
  fprintf(out, "  pol_mode: %s\n", iwrf_pol_mode_to_str(copy.pol_mode).c_str());

  fprintf(out, "  beams_are_indexed: %s\n", copy.beams_are_indexed? "Y":"N");
  fprintf(out, "  specify_dwell_width: %s\n", copy.specify_dwell_width? "Y":"N");
  fprintf(out, "  indexed_beam_width_deg: %g\n", copy.indexed_beam_width_deg);
  fprintf(out, "  indexed_beam_spacing_deg: %g\n", copy.indexed_beam_spacing_deg);

  fprintf(out, "  num_prts: %d\n", copy.num_prts);
  fprintf(out, "  prt3_usec: %g\n", copy.prt3_usec);
  fprintf(out, "  prt4_usec: %g\n", copy.prt4_usec);
  fprintf(out, "  block_mode_prt2_pulses: %d\n", copy.block_mode_prt2_pulses);
  fprintf(out, "  block_mode_prt3_pulses: %d\n", copy.block_mode_prt3_pulses);
  fprintf(out, "  block_mode_prt4_pulses: %d\n", copy.block_mode_prt4_pulses);
  fprintf(out, "  pol_sync_mode: %d\n", copy.pol_sync_mode);

}

//////////////////////////////////////////////////////
// print xmit_power

void iwrf_xmit_power_print(FILE *out,
			   const iwrf_xmit_power_t &pwr)

{

  iwrf_xmit_power_t copy = pwr;
  iwrf_xmit_power_swap(copy);
  fprintf(out, "==================== iwrf_xmit_power ============================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  power_dbm_h: %g\n", copy.power_dbm_h);
  fprintf(out, "  power_dbm_v: %g\n", copy.power_dbm_v);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print rx_power

void iwrf_rx_power_print(FILE *out,
                         const iwrf_rx_power_t &pwr)
  
{

  iwrf_rx_power_t copy = pwr;
  iwrf_rx_power_swap(copy);
  fprintf(out, "==================== iwrf_rx_power ============================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  max_power_dbm_hc: %g\n", copy.max_power_dbm_hc);
  fprintf(out, "  max_power_dbm_vc: %g\n", copy.max_power_dbm_vc);
  fprintf(out, "  max_power_dbm_hx: %g\n", copy.max_power_dbm_hx);
  fprintf(out, "  max_power_dbm_vx: %g\n", copy.max_power_dbm_vx);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print xmit_sample

void iwrf_xmit_sample_print(FILE *out,
			    const iwrf_xmit_sample_t &samp)

{

  iwrf_xmit_sample_t copy = samp;
  iwrf_xmit_sample_swap(copy);
  fprintf(out, "==================== iwrf_xmit_sample ===========================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  power_dbm_h: %g\n", copy.power_dbm_h);
  fprintf(out, "  power_dbm_v: %g\n", copy.power_dbm_v);
  fprintf(out, "  offset: %d\n", copy.offset);
  fprintf(out, "  n_samples: %d\n", copy.n_samples);
  fprintf(out, "  sampling_freq: %g\n", copy.sampling_freq);
  fprintf(out, "  scale_h: %g\n", copy.scale_h);
  fprintf(out, "  offset_h: %g\n", copy.offset_h);
  fprintf(out, "  scale_v: %g\n", copy.scale_v);
  fprintf(out, "  offset_v: %g\n", copy.offset_v);

  int n_samples = copy.n_samples;
  if (n_samples > IWRF_N_TXSAMP) {
    n_samples = IWRF_N_TXSAMP;
  }
 
  fprintf(out, "  samples_h:\n");
  for (int ii = 0; ii < n_samples; ii++) {
    fprintf(out, "    index, val: %d, %d\n", ii,
            copy.samples_h[ii]);
  }

  fprintf(out, "  samples_v:\n");
  for (int ii = 0; ii < n_samples; ii++) {
    fprintf(out, "    index, val: %d, %d\n", ii,
            copy.samples_v[ii]);
  }
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print xmit_sample

void iwrf_xmit_sample_v2_print(FILE *out,
                               const iwrf_xmit_sample_v2_t &samp)
  
{
  
  iwrf_xmit_sample_v2_t copy = samp;
  iwrf_xmit_sample_v2_swap(copy);
  fprintf(out, "================= iwrf_xmit_v2_sample =====================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  n_samples: %d\n", copy.n_samples);
  fprintf(out, "  n_channels: %d\n", copy.n_channels);

  fprintf(out, "===========================================================\n");

}

//////////////////////////////////////////////////////
// print xmit_info

void iwrf_xmit_info_print(FILE *out,
			  const iwrf_xmit_info_t &info)

{
  
  iwrf_xmit_info_t copy = info;
  iwrf_xmit_info_swap(copy);
  fprintf(out, "==================== iwrf_xmit_info =============================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  xmit_0_enabled: %d\n", copy.xmit_0_enabled);
  fprintf(out, "  xmit_1_enabled: %d\n", copy.xmit_1_enabled);
  fprintf(out, "  xmit_rcv_mode: %d\n", copy.xmit_rcv_mode);
  fprintf(out, "  pol_mode: %s\n", iwrf_pol_mode_to_str(copy.pol_mode).c_str());
  fprintf(out, "  xmit_phase_mode: %s\n",
	  iwrf_xmit_phase_mode_to_str(copy.xmit_phase_mode).c_str());
  fprintf(out, "  prf_mode: %s\n", iwrf_prf_mode_to_str(copy.prf_mode).c_str());
  fprintf(out, "  pulse_type: %s\n", iwrf_pulse_type_to_str(copy.pulse_type).c_str());
  fprintf(out, "  prt_usec: %g\n", copy.prt_usec);
  fprintf(out, "  prt2_usec: %g\n", copy.prt2_usec);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print burst_iq

void iwrf_burst_header_print(FILE *out,
                             const iwrf_burst_header_t &val)
  
{

  iwrf_burst_header_t copy = val;
  iwrf_burst_header_swap(copy);
  fprintf(out, "===================== iwrf_burst_header =========================\n");
  iwrf_packet_info_print(out, copy.packet);
  fprintf(out, "  pulse_seq_num: %lld\n", (long long) copy.pulse_seq_num);
  fprintf(out, "  n_samples: %d\n", copy.n_samples);
  fprintf(out, "  channel_id: %d\n", copy.channel_id);
  fprintf(out, "  iq_encoding: %d\n", copy.iq_encoding);
  fprintf(out, "  scale: %g\n", copy.scale);
  fprintf(out, "  offset: %g\n", copy.offset);
  fprintf(out, "  power_dbm: %g\n", copy.power_dbm);
  fprintf(out, "  phase_deg: %g\n", copy.phase_deg);
  fprintf(out, "  freq_hz: %g\n", copy.freq_hz);
  fprintf(out, "  sampling_freq_hz: %g\n", copy.sampling_freq_hz);
  fprintf(out, "  power_max_dbm: %g\n", copy.power_max_dbm);
  fprintf(out, "  power_p90_dbm: %g\n", copy.power_p90_dbm);
  fprintf(out, "  pulse_width_sec: %g\n", copy.pulse_width_sec);
  
  fprintf(out, "=================================================================\n");
  
}

//////////////////////////////////////////////////////
// print status_xml

void iwrf_status_xml_print(FILE *out,
                           const iwrf_status_xml_t &val,
                           const string &xmlStr)
  
{
  
  iwrf_status_xml_t copy = val;
  iwrf_status_xml_swap(copy);
  fprintf(out, "===================== iwrf_status_xml ===========================\n");
  iwrf_packet_info_print(out, copy.packet);
  fprintf(out, "  xml_len: %d\n", copy.xml_len);
  if (copy.xml_len > (int) sizeof(copy) + 1) {
    fprintf(out, "%s\n", xmlStr.c_str());
  }
  fprintf(out, "=================================================================\n");
  
}

void iwrf_status_xml_print(FILE *out, const void *buf)
{
  iwrf_status_xml_t hdr;
  memcpy(&hdr, buf, sizeof(iwrf_status_xml_t));
  const char *str = (const char *) buf + sizeof(iwrf_status_xml_t);
  string statusStr(str);
  iwrf_status_xml_print(out, hdr, statusStr);
}

//////////////////////////////////////////////////////
// print antenna_angles

void iwrf_antenna_angles_print(FILE *out,
                               const iwrf_antenna_angles_t &val)
  
{
  
  iwrf_antenna_angles_t copy = val;
  iwrf_antenna_angles_swap(copy);
  fprintf(out, "================ iwrf_antenna_angles ======================\n");
  iwrf_packet_info_print(out, copy.packet);
  fprintf(out, "  scan_mode: %s\n",
          iwrf_scan_mode_to_str(copy.scan_mode).c_str());
  fprintf(out, "  follow_mode: %s\n",
          iwrf_follow_mode_to_str(copy.follow_mode).c_str());
  fprintf(out, "  sweep_num: %d\n", copy.sweep_num);
  fprintf(out, "  volume_num: %d\n", copy.volume_num);
  fprintf(out, "  fixed_el: %g\n", copy.fixed_el);
  fprintf(out, "  fixed_az: %g\n", copy.fixed_az);
  fprintf(out, "  elevation: %g\n", copy.elevation);
  fprintf(out, "  azimuth: %g\n", copy.azimuth);
  fprintf(out, "  antenna_transition: %d\n", copy.antenna_transition);
  fprintf(out, "  status: %d\n", copy.status);
  if (copy.event_flags & IWRF_END_OF_SWEEP) {
    fprintf(out, "  event: end_of_sweep\n");
  }
  if (copy.event_flags & IWRF_START_OF_SWEEP) {
    fprintf(out, "  event: start_of_sweep\n");
  }
  if (copy.event_flags & IWRF_END_OF_VOLUME) {
    fprintf(out, "  event: end_of_volume\n");
  }
  if (copy.event_flags & IWRF_START_OF_VOLUME) {
    fprintf(out, "  event: start_of_volume\n");
  }
  fprintf(out, "===========================================================\n");
  
}

//////////////////////////////////////////////////////
// print calibration

void iwrf_calibration_print(FILE *out,
			    const iwrf_calibration_t &calib)
  
{

  iwrf_calibration_t copy = calib;
  iwrf_calibration_swap(copy);
  fprintf(out, "==================== iwrf_calibration ===========================\n");
  iwrf_packet_info_print(out, copy.packet);
  
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

void iwrf_event_notice_print(FILE *out,
			     const iwrf_event_notice_t &note)

{

  iwrf_event_notice_t copy = note;
  iwrf_event_notice_swap(copy);
  fprintf(out, "==================== iwrf_event_notice ==========================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  start_of_sweep: %d\n", copy.start_of_sweep);
  fprintf(out, "  end_of_sweep: %d\n", copy.end_of_sweep);
  fprintf(out, "  start_of_volume: %d\n", copy.start_of_volume);
  fprintf(out, "  end_of_volume: %d\n", copy.end_of_volume);
  fprintf(out, "  scan_mode: %s\n", iwrf_scan_mode_to_str(copy.scan_mode).c_str());
  fprintf(out, "  follow_mode: %s\n", iwrf_follow_mode_to_str(copy.follow_mode).c_str());
  fprintf(out, "  volume_num: %d\n", copy.volume_num);
  fprintf(out, "  sweep_num: %d\n", copy.sweep_num);
  fprintf(out, "  cause: %s\n", iwrf_event_cause_to_str(copy.cause).c_str());
  fprintf(out, "  current_fixed_angle: %g\n", copy.current_fixed_angle);
  fprintf(out, "  antenna_transition: %d\n", copy.antenna_transition);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print iwrf_phasecode

void iwrf_phasecode_print(FILE *out,
			  const iwrf_phasecode_t &code)
  
{

  iwrf_phasecode_t copy = code;
  iwrf_phasecode_swap(copy);
  fprintf(out, "==================== iwrf_phasecode =============================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  int seq_length = copy.seq_length;
  if (seq_length > IWRF_MAX_PHASE_SEQ_LEN) {
    seq_length = IWRF_MAX_PHASE_SEQ_LEN;
  }
  fprintf(out, "  seq_length: %d\n", seq_length);
  
  for (int ii = 0; ii < seq_length; ii++) {
    fprintf(out, "  Sequence[%d]: phase_deg_h, phase_deg_v: %g, %g",
	    ii,
	    copy.phase[ii].phase_deg_h,
	    copy.phase[ii].phase_deg_v);
  }
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print pulse_header

void iwrf_pulse_header_print(FILE *out,
			     const iwrf_pulse_header_t &pulse,
                             const iwrf_platform_georef_t *georef /* = NULL*/)

{
  
  iwrf_pulse_header_t copy = pulse;
  iwrf_pulse_header_swap(copy);
  fprintf(out, "==================== iwrf_pulse_header ==========================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  pulse_seq_num: %lld\n", (long long) copy.pulse_seq_num);
  fprintf(out, "  scan_mode: %s\n", iwrf_scan_mode_to_str(copy.scan_mode).c_str());
  fprintf(out, "  follow_mode: %s\n", iwrf_follow_mode_to_str(copy.follow_mode).c_str());
  fprintf(out, "  sweep_num: %d\n", copy.sweep_num);
  fprintf(out, "  volume_num: %d\n", copy.volume_num);
  fprintf(out, "  fixed_el: %g\n", copy.fixed_el);
  fprintf(out, "  fixed_az: %g\n", copy.fixed_az);
  fprintf(out, "  elevation: %g\n", copy.elevation);
  fprintf(out, "  azimuth: %g\n", copy.azimuth);
  fprintf(out, "  prt: %g\n", copy.prt);
  fprintf(out, "  prt_next: %g\n", copy.prt_next);
  fprintf(out, "  pulse_width_us: %g\n", copy.pulse_width_us);
  fprintf(out, "  n_gates: %d\n", copy.n_gates);
  fprintf(out, "  n_channels: %d\n", copy.n_channels);
  fprintf(out, "  iq_encoding: %s\n",
          iwrf_iq_encoding_to_str(copy.iq_encoding).c_str());  
  fprintf(out, "  hv_flag: %d\n", copy.hv_flag);
  fprintf(out, "  antenna_transition: %d\n", copy.antenna_transition);
  fprintf(out, "  phase_cohered: %d\n", copy.phase_cohered);
  fprintf(out, "  status: %d\n", copy.status);
  fprintf(out, "  n_data: %d\n", copy.n_data);

  fprintf(out, "  iq_offset:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %d", copy.iq_offset[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  burst_mag:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %g", copy.burst_mag[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  burst_arg:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %g", copy.burst_arg[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  burst_arg_diff:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %g", copy.burst_arg_diff[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  scale: %g\n", copy.scale);
  fprintf(out, "  offset: %g\n", copy.offset);
  fprintf(out, "  n_gates_burst: %d\n", copy.n_gates_burst);

  fprintf(out, "  start_range_m: %g\n", copy.start_range_m);
  fprintf(out, "  gate_spacing_m: %g\n", copy.gate_spacing_m);

  if (copy.event_flags & IWRF_END_OF_SWEEP) {
    fprintf(out, "  event: end_of_sweep\n");
  }
  if (copy.event_flags & IWRF_START_OF_SWEEP) {
    fprintf(out, "  event: start_of_sweep\n");
  }
  if (copy.event_flags & IWRF_END_OF_VOLUME) {
    fprintf(out, "  event: end_of_volume\n");
  }
  if (copy.event_flags & IWRF_START_OF_VOLUME) {
    fprintf(out, "  event: start_of_volume\n");
  }

  if (copy.txrx_state & IWRF_TXRX_LONG_PRT) {
    fprintf(out, "  txrx state: long PRT\n");
  }
  if (copy.txrx_state & IWRF_TXRX_SHORT_PRT) {
    fprintf(out, "  txrx state: short PRT\n");
  }

  if (georef != NULL) {
    iwrf_platform_georef_t gcopy = *georef;
    iwrf_platform_georef_swap(gcopy);
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
// print rvp8_pulse_header

void iwrf_rvp8_pulse_header_print(FILE *out,
				  const iwrf_rvp8_pulse_header_t &pulse)

{
  
  iwrf_rvp8_pulse_header_t copy = pulse;
  iwrf_rvp8_pulse_header_swap(copy);
  fprintf(out, "==================== iwrf_rvp8_pulse_header =====================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  i_flags: %d\n", copy.i_flags);
  fprintf(out, "  i_aq_mode: %d\n", copy.i_aq_mode);
  fprintf(out, "  i_polar_bits: %d\n", copy.i_polar_bits);
  fprintf(out, "  i_viq_per_bin: %d\n", copy.i_viq_per_bin);
  fprintf(out, "  i_tg_bank: %d\n", copy.i_tg_bank);
  fprintf(out, "  i_tx_phase: %d\n", copy.i_tx_phase);
  fprintf(out, "  i_az: %d\n", copy.i_az);
  fprintf(out, "  i_el: %d\n", copy.i_el);
  fprintf(out, "  i_num_vecs: %d\n", copy.i_num_vecs);
  fprintf(out, "  i_max_vecs: %d\n", copy.i_max_vecs);
  fprintf(out, "  i_tg_wave: %d\n", copy.i_tg_wave);
  fprintf(out, "  i_btime_api: %d\n", copy.i_btime_api);
  fprintf(out, "  i_sys_time: %d\n", copy.i_sys_time);
  fprintf(out, "  i_prev_prt: %d\n", copy.i_prev_prt);
  fprintf(out, "  i_next_prt: %d\n", copy.i_next_prt);
  fprintf(out, "  i_seq_num: %d\n", copy.i_seq_num);

  fprintf(out, "  uiq_perm:");
  for (int ii = 0; ii < 2; ii++) {
    fprintf(out, " %d", copy.uiq_perm[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  i_data_off:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %d", copy.i_data_off[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  f_burst_mag:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %g", copy.f_burst_mag[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  i_burst_arg:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %d", copy.i_burst_arg[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  i_wrap_iq:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %d", copy.i_wrap_iq[ii]);
  }
  fprintf(out, "\n");
  fprintf(out, "=================================================================\n");
}

//////////////////////////////////////////////////////
// print rvp8_ops_info

void iwrf_rvp8_ops_info_print(FILE *out,
			      const iwrf_rvp8_ops_info_t &info)

{
  
  iwrf_rvp8_ops_info_t copy = info;
  iwrf_rvp8_ops_info_swap(copy);
  fprintf(out, "==================== iwrf_rvp8_ops_info =========================\n");
  iwrf_packet_info_print(out, copy.packet);
  
  fprintf(out, "  i_version: %d\n", copy.i_version);
  fprintf(out, "  i_major_mode: %d\n", copy.i_major_mode);
  fprintf(out, "  i_polarization: %d\n", copy.i_polarization);
  fprintf(out, "  i_phase_mode_seq: %d\n", copy.i_phase_mode_seq);
  fprintf(out, "  i_task_sweep: %d\n", copy.i_task_sweep);
  fprintf(out, "  i_task_aux_num: %d\n", copy.i_task_aux_num);
  fprintf(out, "  i_task_scan_type: %d\n", copy.i_task_scan_type);

  
  fprintf(out, "  task_name: %s\n",
	  iwrf_safe_str(copy.s_task_name, 32).c_str());
  fprintf(out, "  site_name: %s\n",
	  iwrf_safe_str(copy.s_site_name, 32).c_str());

  fprintf(out, "  i_aq_mode: %d\n", copy.i_aq_mode);
  fprintf(out, "  i_unfold_mode: %d\n", copy.i_unfold_mode);
  fprintf(out, "  i_pwidth_code: %d\n", copy.i_pwidth_code);
  fprintf(out, "  f_pwidth_usec: %g\n", copy.f_pwidth_usec);
  fprintf(out, "  f_dbz_calib: %g\n", copy.f_dbz_calib);
  fprintf(out, "  i_sample_size: %d\n", copy.i_sample_size);
  fprintf(out, "  i_mean_angle_sync: %d\n", copy.i_mean_angle_sync);
  fprintf(out, "  i_flags: %d\n", copy.i_flags);
  fprintf(out, "  i_playback_version: %d\n", copy.i_playback_version);
  fprintf(out, "  f_sy_clk_mhz: %g\n", copy.f_sy_clk_mhz);
  fprintf(out, "  f_wavelength_cm: %g\n", copy.f_wavelength_cm);
  fprintf(out, "  f_saturation_dbm: %g\n", copy.f_saturation_dbm);
  fprintf(out, "  f_range_mask_res: %g\n", copy.f_range_mask_res);

  fprintf(out, "  i_range_mask:");
  for (int ii = 0; ii < IWRF_RVP8_GATE_MASK_LEN; ii++) {
    fprintf(out, " %d", copy.i_range_mask[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  f_noise_dbm:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %g", copy.f_noise_dbm[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  f_noise_stdv_db:");
  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    fprintf(out, " %g", copy.f_noise_stdv_db[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  f_noise_range_km: %g\n", copy.f_noise_range_km);
  fprintf(out, "  f_noise_prf_hz: %g\n", copy.f_noise_prf_hz);

  fprintf(out, "  i_gparm_latch_sts:");
  for (int ii = 0; ii < 2; ii++) {
    fprintf(out, " %d", copy.i_gparm_latch_sts[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  i_gparm_immed_sts:");
  for (int ii = 0; ii < 6; ii++) {
    fprintf(out, " %d", copy.i_gparm_immed_sts[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  i_gparm_diag_bits:");
  for (int ii = 0; ii < 4; ii++) {
    fprintf(out, " %d", copy.i_gparm_diag_bits[ii]);
  }
  fprintf(out, "\n");

  fprintf(out, "  version_string: %s\n",
	  iwrf_safe_str(copy.s_version_string, 12).c_str());

  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print moments_field_header

void iwrf_moments_field_header_print(FILE *out,
                                     const iwrf_moments_field_header_t &val)
  
{

  iwrf_moments_field_header_t copy = val;
  iwrf_moments_field_header_swap(copy);
  fprintf(out, "================ iwrf_moments_field_header ======================\n");
  iwrf_packet_info_print(out, copy.packet);

  fprintf(out, "  name: %s\n",
	  iwrf_safe_str(copy.name, IWRF_MAX_MOMENTS_FIELD_NAME).c_str());
  fprintf(out, "  long_name: %s\n",
	  iwrf_safe_str(copy.long_name, IWRF_MAX_MOMENTS_FIELD_NAME_LONG).c_str());
  fprintf(out, "  standard_name: %s\n",
	  iwrf_safe_str(copy.standard_name, IWRF_MAX_MOMENTS_FIELD_NAME_STANDARD).c_str());
  fprintf(out, "  units: %s\n",
	  iwrf_safe_str(copy.units, IWRF_MAX_MOMENTS_FIELD_UNITS).c_str());
  fprintf(out, "  threshold_field_name: %s\n",
	  iwrf_safe_str(copy.threshold_field_name, IWRF_MAX_MOMENTS_FIELD_NAME).c_str());

  fprintf(out, "  encoding: %s\n",
	  iwrf_moments_encoding_to_str(copy.encoding).c_str());
  fprintf(out, "  byte_width: %d\n", copy.byte_width);
  fprintf(out, "  scale: %g\n", copy.scale);
  fprintf(out, "  offset: %g\n", copy.offset);
  fprintf(out, "  sampling_ratio: %g\n", copy.sampling_ratio);
  fprintf(out, "  threshold_value: %g\n", copy.threshold_value);
  fprintf(out, "  n_samples: %d\n", copy.n_samples);
  fprintf(out, "  nyquist_mps: %g\n", copy.nyquist_mps);
  

  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print moments_ray_header

void iwrf_moments_ray_header_print(FILE *out,
                                     const iwrf_moments_ray_header_t &val)
  
{

  iwrf_moments_ray_header_t copy = val;
  iwrf_moments_ray_header_swap(copy);
  fprintf(out, "================ iwrf_moments_ray_header ======================\n");
  iwrf_packet_info_print(out, copy.packet);

  fprintf(out, "  volume_num: %d\n", copy.volume_num);
  fprintf(out, "  sweep_num: %d\n", copy.sweep_num);

  fprintf(out, "  scan_mode: %s\n", 
          iwrf_scan_mode_to_str(copy.scan_mode).c_str());
  fprintf(out, "  follow_mode: %s\n", 
          iwrf_follow_mode_to_str(copy.follow_mode).c_str());
  fprintf(out, "  prf_mode: %s\n",
          iwrf_prf_mode_to_str(copy.prf_mode).c_str());
  fprintf(out, "  polarization_mode: %s\n", 
          iwrf_pol_mode_to_str(copy.polarization_mode).c_str());

  fprintf(out, "  elevation: %g\n", copy.elevation);
  fprintf(out, "  azimuth: %g\n", copy.azimuth);
  fprintf(out, "  fixed_angle: %g\n", copy.fixed_angle);
  fprintf(out, "  target_scan_rate: %g\n", copy.target_scan_rate);
  fprintf(out, "  true_scan_rate: %g\n", copy.true_scan_rate);
  fprintf(out, "  is_indexed: %d\n", copy.is_indexed);
  fprintf(out, "  angle_res: %g\n", copy.angle_res);
  fprintf(out, "  antenna_transition: %d\n", copy.antenna_transition);
  fprintf(out, "  prt: %g\n", copy.prt);
  fprintf(out, "  prt_ratio: %g\n", copy.prt_ratio);
  fprintf(out, "  pulse_width_us: %g\n", copy.pulse_width_us);
  fprintf(out, "  n_samples: %d\n", copy.n_samples);
  fprintf(out, "  n_fields: %d\n", copy.n_fields);
  fprintf(out, "  n_gates: %d\n", copy.n_gates);
  fprintf(out, "  start_range_m: %g\n", copy.start_range_m);
  fprintf(out, "  gate_spacing_m: %g\n", copy.gate_spacing_m);
  fprintf(out, "  nyquist_mps: %g\n", copy.nyquist_mps);
  fprintf(out, "  unambig_range_km: %g\n", copy.unambig_range_km);
  fprintf(out, "  meas_xmit_power_dbm_h: %g\n", copy.meas_xmit_power_dbm_h);
  fprintf(out, "  meas_xmit_power_dbm_v: %g\n", copy.meas_xmit_power_dbm_v);
  fprintf(out, "  event_flags: %d\n", copy.event_flags);

  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print moments_field_index

void iwrf_moments_field_index_print(FILE *out,
                                    const iwrf_moments_field_index &val)

{

  iwrf_moments_field_index_t copy = val;
  iwrf_moments_field_index_swap(copy);
  fprintf(out, "================ iwrf_moments_field_index =======================\n");
  fprintf(out, "  field_name: %s\n",
	  iwrf_safe_str(copy.field_name, IWRF_MAX_MOMENTS_FIELD_NAME).c_str());
  fprintf(out, "  id: %d\n", copy.id);
  fprintf(out, "  offset: %d\n", copy.offset);
  fprintf(out, "  len: %d\n", copy.len);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print platform_georef

void iwrf_platform_georef_print(FILE *out,
                                const iwrf_platform_georef &val)

{
  
  iwrf_platform_georef_t copy = val;
  iwrf_platform_georef_swap(copy);
  fprintf(out, "====================== iwrf_platform_georef =====================\n");
  iwrf_packet_info_print(out, copy.packet);
  fprintf(out, "  unit_num: %d\n", copy.unit_num);
  fprintf(out, "  unit_id: %d\n", copy.unit_id);
  fprintf(out, "  longitude: %g\n", copy.longitude);
  fprintf(out, "  latitude: %g\n", copy.latitude);
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
  fprintf(out, "  drive_angle_1_deg: %g\n", copy.drive_angle_1_deg);
  fprintf(out, "  drive_angle_2_deg: %g\n", copy.drive_angle_2_deg);
  fprintf(out, "=================================================================\n");

}

//////////////////////////////////////////////////////
// print georef_correction

void iwrf_georef_correction_print(FILE *out,
                                  const iwrf_georef_correction &val)

{

  iwrf_georef_correction_t copy = val;
  iwrf_georef_correction_swap(copy);
  fprintf(out, "==================== iwrf_georef_correction =====================\n");
  fprintf(out, "  azimuth_corr_deg: %g\n", copy.azimuth_corr_deg);
  fprintf(out, "  elevation_corr_deg: %g\n", copy.elevation_corr_deg);
  fprintf(out, "  range_delay_corr_mps: %g\n", copy.range_delay_corr_mps);
  fprintf(out, "  longitude_corr_deg: %g\n", copy.longitude_corr_deg);
  fprintf(out, "  latitude_corr_deg: %g\n", copy.latitude_corr_deg);
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
// print UI task list

void iwrf_ui_tasklist_print(FILE *out, const iwrf_ui_tasklist_full_t &tlp)

{
  int i;
  fprintf(out, "Tasklist Contents:\n");
  for(i=0; i < tlp.num_list_items; ++i) {
    fprintf(out,"%3d %s\n", i, tlp.tasks[i].name);
  }
  fprintf(out, "Tasklist Schedule:\n");
  iwrf_ui_schedule_print(out, tlp.tasklist_schedule);
  fprintf(out, "\n");
}

//////////////////////////////////////////////////////
// print UI schedule header

void iwrf_ui_schedule_print(FILE *out,
			    const iwrf_ui_schedule_info_t &schedule)
{
  fprintf(out, "begin time: %s\n", 
          iwrf_time_str((time_t*) &schedule.begin_time_utc,
                        NULL).c_str());
  fprintf(out, "repeat cycle %d secs\n", schedule.repeat_cycle_secs);
  fprintf(out, "priority: %d\n", schedule.priority);
  if(schedule.last_run_time_utc==0) {
    fprintf(out, "last run at: never run\n");
  } else {
    fprintf(out, "last run at: %s\n", 
            iwrf_time_str((time_t*) &schedule.last_run_time_utc,
                          NULL).c_str());
  }
}

//////////////////////////////////////////////////////
// print ui task operations

void iwrf_ui_task_operations_print(FILE *out,
                                   const iwrf_ui_task_operations_t &val)
{

  iwrf_ui_task_operations_t copy = val;
  // iwrf_task_operations_swap(copy);
  fprintf(out, "=========== iwrf_ui_task_operations ====================\n");
  iwrf_packet_info_print(out, copy.packet);
  fprintf(out, "  task_name: %s\n",
          iwrf_safe_str(copy.task_name, IWRF_MAX_SEGMENT_NAME).c_str());
  fprintf(out, "  owner_name: %s\n",
          iwrf_safe_str(copy.owner_name, IWRF_UI_MAX_OPERATOR_NAME).c_str());
  fprintf(out, "  opcode: %s\n",
          iwrf_ui_opcode_to_str(copy.op_code).c_str());
  fprintf(out, "========================================================\n");

}

//////////////////////////////////////////////////////
// Return a string formed safely from a char* array
// Null-termination of the input string is guaranteed.

string iwrf_safe_str(const char *str, int maxLen)

{

  char *safechar = new char[maxLen + 1];
  memcpy(safechar, str, maxLen);
  safechar[maxLen] = '\0';
  string safestr(safechar);
  delete[] safechar;

  return safestr;

}

string iwrf_time_str(const time_t *ptime, si32 *nano_secs)
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
/// Print format for IWRF structs

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
/// Print format of all IWRF structs

static void _print_format_header(FILE *out)
{
  fprintf(out, _hform, "type", "name", "size", "offset");
  fprintf(out, _hform, "----", "----", "----", "------");
}

void iwrf_print_all_formats(FILE *out)
{

  fprintf(out, "===================== IWRF FORMAT ======================\n");

  {
    iwrf_packet_info_t val;
    iwrf_packet_info_print_format(out, val);
  }

  {
    iwrf_sync_t val;
    iwrf_sync_print_format(out, val);
  }

  {
    iwrf_radar_info_t val;
    iwrf_radar_info_print_format(out, val);
  }
  
  {
    iwrf_scan_segment_t val;
    iwrf_scan_segment_print_format(out, val);
  }
  
  {
    iwrf_antenna_correction_t val;
    iwrf_antenna_correction_print_format(out, val);
  }
  
  {
    iwrf_ts_processing_t val;
    iwrf_ts_processing_print_format(out, val);
  }
  
  {
    iwrf_xmit_power_t val;
    iwrf_xmit_power_print_format(out, val);
  }
  
  {
    iwrf_rx_power_t val;
    iwrf_rx_power_print_format(out, val);
  }
  
  {
    iwrf_xmit_sample_t val;
    iwrf_xmit_sample_print_format(out, val);
  }
  
  {
    iwrf_xmit_sample_v2_t val;
    iwrf_xmit_sample_v2_print_format(out, val);
  }
  
  {
    iwrf_burst_header_t val;
    iwrf_burst_header_print_format(out, val);
  }
  
  {
    iwrf_status_xml_t val;
    iwrf_status_xml_print_format(out, val);
  }
  
  {
    iwrf_antenna_angles_t val;
    iwrf_antenna_angles_print_format(out, val);
  }
  
  {
    iwrf_calibration_t val;
    iwrf_calibration_print_format(out, val);
  }
  
  {
    iwrf_event_notice_t val;
    iwrf_event_notice_print_format(out, val);
  }
  
  {
    iwrf_phasecode_t val;
    iwrf_phasecode_print_format(out, val);
  }
  
  {
    iwrf_xmit_info_t val;
    iwrf_xmit_info_print_format(out, val);
  }
  
  {
    iwrf_pulse_header_t val;
    iwrf_pulse_header_print_format(out, val);
  }
  
  {
    iwrf_rvp8_pulse_header_t val;
    iwrf_rvp8_pulse_header_print_format(out, val);
  }
  
  {
    iwrf_rvp8_ops_info_t val;
    iwrf_rvp8_ops_info_print_format(out, val);
  }
  
  {
    iwrf_ui_task_operations_t val;
    iwrf_ui_task_operations_print_format(out, val);
  }
  
  {
    iwrf_moments_field_header_t val;
    iwrf_moments_field_header_print_format(out, val);
  }
  
  {
    iwrf_moments_ray_header_t val;
    iwrf_moments_ray_header_print_format(out, val);
  }
  
  {
    iwrf_moments_field_index_t val;
    iwrf_moments_field_index_print_format(out, val);
  }
  
  {
    iwrf_platform_georef_t val;
    iwrf_platform_georef_print_format(out, val);
  }
  
  {
    iwrf_georef_correction_t val;
    iwrf_georef_correction_print_format(out, val);
  }
  
  _print_format_divider('=', out);

}

// print basic packet info

void _print_packet_format(FILE *out,
                          const iwrf_packet_info_t &pkt)

{
  
  const char *id = (char *) &pkt.id;
  fprintf(out, _dform, "si32", "id", sizeof(pkt.id), (char *) &pkt.id - id);
  fprintf(out, _dform, "si32", "len_bytes", sizeof(pkt.len_bytes), (char *) &pkt.len_bytes - id);
  fprintf(out, _dform, "si64", "seq_num", sizeof(pkt.seq_num), (char *) &pkt.seq_num - id);
  fprintf(out, _dform, "si32", "version_num", sizeof(pkt.version_num), (char *) &pkt.version_num - id);
  fprintf(out, _dform, "si32", "radar_id", sizeof(pkt.radar_id), (char *) &pkt.radar_id - id);
  fprintf(out, _dform, "si64", "time_secs_utc", sizeof(pkt.time_secs_utc), (char *) &pkt.time_secs_utc - id);
  fprintf(out, _dform, "si32", "time_nano_secs", sizeof(pkt.time_nano_secs), (char *) &pkt.time_nano_secs - id);
  fprintf(out, _dform, "si32", "reserved[5]", sizeof(pkt.reserved), (char *) pkt.reserved - id);

}

// print format of packet info

void iwrf_packet_info_print_format(FILE *out,
                                   const iwrf_packet_info_t &val)

{
  
  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_packet_info_t'\n");
  fprintf(out, "  included in all packet types\n");
  fprintf(out, "  size: %d\n\n", (int) sizeof(val));
  _print_format_header(out);
  _print_packet_format(out, val);
  _print_format_divider('-', out);

}


// print format of sync packet

void iwrf_sync_print_format(FILE *out, const iwrf_sync_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_sync_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_SYNC_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  fprintf(out, _dform, "si32", "magik[2]", sizeof(val.magik), (char *) val.magik - id);
  
  _print_format_divider('-', out);

}

// print format of radar_info

void iwrf_radar_info_print_format(FILE *out, const iwrf_radar_info_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_radar_info_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_RADAR_INFO_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;

  fprintf(out, _dform, "fl32", "latitude_deg", sizeof(val.latitude_deg), (char *) &val.latitude_deg - id);
  fprintf(out, _dform, "fl32", "longitude_deg", sizeof(val.longitude_deg), (char *) &val.longitude_deg - id);
  fprintf(out, _dform, "fl32", "altitude_m", sizeof(val.altitude_m), (char *) &val.altitude_m - id);
  fprintf(out, _dform, "si32", "platform_type", sizeof(val.platform_type), (char *) &val.platform_type - id);
  fprintf(out, _dform, "fl32", "beamwidth_deg_h", sizeof(val.beamwidth_deg_h), (char *) &val.beamwidth_deg_h - id);
  fprintf(out, _dform, "fl32", "beamwidth_deg_v", sizeof(val.beamwidth_deg_v), (char *) &val.beamwidth_deg_v - id);
  fprintf(out, _dform, "fl32", "wavelength_cm", sizeof(val.wavelength_cm), (char *) &val.wavelength_cm - id);
  fprintf(out, _dform, "fl32", "nominal_gain_ant_db_h", sizeof(val.nominal_gain_ant_db_h), (char *) &val.nominal_gain_ant_db_h - id);
  fprintf(out, _dform, "fl32", "nominal_gain_ant_db_v", sizeof(val.nominal_gain_ant_db_v), (char *) &val.nominal_gain_ant_db_v - id);
  fprintf(out, _dform, "fl32", "unused[25]", sizeof(val.unused), (char *) val.unused - id);
  fprintf(out, _dform, "char", "radar_name[32]", sizeof(val.radar_name), (char *) val.radar_name - id);
  fprintf(out, _dform, "char", "site_name[32]", sizeof(val.site_name), (char *) val.site_name - id);

  _print_format_divider('-', out);

}

// print format of scan_segment

void iwrf_scan_segment_print_format(FILE *out, const iwrf_scan_segment_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_scan_segment_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_SCAN_SEGMENT_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "scan_mode", sizeof(val.scan_mode), (char *) &val.scan_mode - id);
  fprintf(out, _dform, "si32", "follow_mode", sizeof(val.follow_mode), (char *) &val.follow_mode - id);
  fprintf(out, _dform, "si32", "volume_num", sizeof(val.volume_num), (char *) &val.volume_num - id);
  fprintf(out, _dform, "si32", "sweep_num", sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "si32", "time_limit", sizeof(val.time_limit), (char *) &val.time_limit - id);
  fprintf(out, _dform, "fl32", "az_manual", sizeof(val.az_manual), (char *) &val.az_manual - id);
  fprintf(out, _dform, "fl32", "el_manual", sizeof(val.el_manual), (char *) &val.el_manual - id);
  fprintf(out, _dform, "fl32", "az_start", sizeof(val.az_start), (char *) &val.az_start - id);
  fprintf(out, _dform, "fl32", "el_start", sizeof(val.el_start), (char *) &val.el_start - id);
  fprintf(out, _dform, "fl32", "scan_rate", sizeof(val.scan_rate), (char *) &val.scan_rate - id);
  fprintf(out, _dform, "fl32", "left_limit", sizeof(val.left_limit), (char *) &val.left_limit - id);
  fprintf(out, _dform, "fl32", "right_limit", sizeof(val.right_limit), (char *) &val.right_limit - id);
  fprintf(out, _dform, "fl32", "up_limit", sizeof(val.up_limit), (char *) &val.up_limit - id);
  fprintf(out, _dform, "fl32", "down_limit", sizeof(val.down_limit), (char *) &val.down_limit - id);
  fprintf(out, _dform, "fl32", "step", sizeof(val.step), (char *) &val.step - id);
  fprintf(out, _dform, "fl32", "current_fixed_angle", sizeof(val.current_fixed_angle), (char *) &val.current_fixed_angle - id);
  fprintf(out, _dform, "si32", "init_direction_cw", sizeof(val.init_direction_cw), (char *) &val.init_direction_cw - id);
  fprintf(out, _dform, "si32", "init_direction_up", sizeof(val.init_direction_up), (char *) &val.init_direction_up - id);
  fprintf(out, _dform, "si32", "n_sweeps", sizeof(val.n_sweeps), (char *) &val.n_sweeps - id);
  fprintf(out, _dform, "fl32", "fixed_angles[512]", sizeof(val.fixed_angles), (char *) val.fixed_angles - id);
  fprintf(out, _dform, "fl32", "optimizer_rmax_km", sizeof(val.optimizer_rmax_km), (char *) &val.optimizer_rmax_km - id);
  fprintf(out, _dform, "fl32", "optimizer_htmax_km", sizeof(val.optimizer_htmax_km), (char *) &val.optimizer_htmax_km - id);
  fprintf(out, _dform, "fl32", "optimizer_res_m", sizeof(val.optimizer_res_m), (char *) &val.optimizer_res_m - id);
  fprintf(out, _dform, "fl32", "sun_scan_sector_width_az", sizeof(val.sun_scan_sector_width_az), (char *) &val.sun_scan_sector_width_az - id);
  fprintf(out, _dform, "fl32", "sun_scan_sector_width_el", sizeof(val.sun_scan_sector_width_el), (char *) &val.sun_scan_sector_width_el - id);

  fprintf(out, _dform, "si32", "timeseries_archive_enable", sizeof(val.timeseries_archive_enable), (char *) &val.timeseries_archive_enable - id);
  fprintf(out, _dform, "si32", "waveguide_switch_position", sizeof(val.waveguide_switch_position), (char *) &val.waveguide_switch_position - id);
  fprintf(out, _dform, "si32", "start_ppi_ccw", sizeof(val.start_ppi_ccw), (char *) &val.start_ppi_ccw - id);

  fprintf(out, _dform, "fl32", "unused[455]", sizeof(val.unused), (char *) val.unused - id);
  fprintf(out, _dform, "char", "segment_name[32]", sizeof(val.segment_name), (char *) val.segment_name - id);
  fprintf(out, _dform, "char", "project_name[32]", sizeof(val.project_name), (char *) val.project_name - id);

  _print_format_divider('-', out);

}


// print format of antenna_correction

void iwrf_antenna_correction_print_format(FILE *out, const iwrf_antenna_correction_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_antenna_correction_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_ANTENNA_CORRECTION_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "fl32", "az_correction", sizeof(val.az_correction), (char *) &val.az_correction - id);
  fprintf(out, _dform, "fl32", "el_correction", sizeof(val.el_correction), (char *) &val.el_correction - id);
  fprintf(out, _dform, "fl32", "unused[16]", sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}


// print format of ts_processing

void iwrf_ts_processing_print_format(FILE *out, const iwrf_ts_processing_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_ts_processing_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_TS_PROCESSING_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "xmit_rcv_mode", sizeof(val.xmit_rcv_mode), (char *) &val.xmit_rcv_mode - id);
  fprintf(out, _dform, "si32", "xmit_phase_mode", sizeof(val.xmit_phase_mode), (char *) &val.xmit_phase_mode - id);
  fprintf(out, _dform, "si32", "prf_mode", sizeof(val.prf_mode), (char *) &val.prf_mode - id);
  fprintf(out, _dform, "si32", "pulse_type", sizeof(val.pulse_type), (char *) &val.pulse_type - id);
  fprintf(out, _dform, "fl32", "prt_usec", sizeof(val.prt_usec), (char *) &val.prt_usec - id);
  fprintf(out, _dform, "fl32", "prt2_usec", sizeof(val.prt2_usec), (char *) &val.prt2_usec - id);
  fprintf(out, _dform, "si32", "cal_type", sizeof(val.cal_type), (char *) &val.cal_type - id);
  fprintf(out, _dform, "fl32", "burst_range_offset_m", sizeof(val.burst_range_offset_m), (char *) &val.burst_range_offset_m - id);
  fprintf(out, _dform, "fl32", "pulse_width_us", sizeof(val.pulse_width_us), (char *) &val.pulse_width_us - id);
  fprintf(out, _dform, "fl32", "start_range_m", sizeof(val.start_range_m), (char *) &val.start_range_m - id);
  fprintf(out, _dform, "fl32", "gate_spacing_m", sizeof(val.gate_spacing_m), (char *) &val.gate_spacing_m - id);
  fprintf(out, _dform, "si32", "integration_cycle_pulses", sizeof(val.integration_cycle_pulses), (char *) &val.integration_cycle_pulses - id);
  fprintf(out, _dform, "si32", "clutter_filter_number", sizeof(val.clutter_filter_number), (char *) &val.clutter_filter_number - id);
  fprintf(out, _dform, "si32", "range_gate_averaging", sizeof(val.range_gate_averaging), (char *) &val.range_gate_averaging - id);
  fprintf(out, _dform, "si32", "max_gate", sizeof(val.max_gate), (char *) &val.max_gate - id);
  fprintf(out, _dform, "fl32", "test_power_dbm", sizeof(val.test_power_dbm), (char *) &val.test_power_dbm - id);
  fprintf(out, _dform, "fl32", "test_pulse_range_km", sizeof(val.test_pulse_range_km), (char *) &val.test_pulse_range_km - id);
  fprintf(out, _dform, "fl32", "test_pulse_length_usec", sizeof(val.test_pulse_length_usec), (char *) &val.test_pulse_length_usec - id);
  fprintf(out, _dform, "si32", "pol_mode", sizeof(val.pol_mode), (char *) &val.pol_mode - id);
  fprintf(out, _dform, "si32", "xmit_flag[2]", sizeof(val.xmit_flag), (char *) val.xmit_flag - id);
  fprintf(out, _dform, "si32", "beams_are_indexed", sizeof(val.beams_are_indexed), (char *) &val.beams_are_indexed - id);
  fprintf(out, _dform, "si32", "specify_dwell_width", sizeof(val.specify_dwell_width), (char *) &val.specify_dwell_width - id);
  fprintf(out, _dform, "fl32", "indexed_beam_width_deg", sizeof(val.indexed_beam_width_deg), (char *) &val.indexed_beam_width_deg - id);
  fprintf(out, _dform, "fl32", "indexed_beam_spacing_deg", sizeof(val.indexed_beam_spacing_deg), (char *) &val.indexed_beam_spacing_deg - id);

  fprintf(out, _dform, "si32", "indexed_beam_spacing_deg", sizeof(val.indexed_beam_spacing_deg), (char *) &val.indexed_beam_spacing_deg - id);

  fprintf(out, _dform, "si32", "num_prts", sizeof(val.num_prts), (char *) &val.num_prts - id);
  fprintf(out, _dform, "fl32", "prt3_usec", sizeof(val.prt3_usec), (char *) &val.prt3_usec - id);
  fprintf(out, _dform, "fl32", "prt4_usec", sizeof(val.prt4_usec), (char *) &val.prt4_usec - id);
  fprintf(out, _dform, "si32", "block_mode_prt2_pulses", sizeof(val.block_mode_prt2_pulses), (char *) &val.block_mode_prt2_pulses - id);
  fprintf(out, _dform, "si32", "block_mode_prt3_pulses", sizeof(val.block_mode_prt3_pulses), (char *) &val.block_mode_prt3_pulses - id);
  fprintf(out, _dform, "si32", "block_mode_prt4_pulses", sizeof(val.block_mode_prt4_pulses), (char *) &val.block_mode_prt4_pulses - id);
  fprintf(out, _dform, "ui32", "pol_sync_mode", sizeof(val.pol_sync_mode), (char *) &val.pol_sync_mode - id);

  fprintf(out, _dform, "fl32", "unused[18]", sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}


// print format of xmit_power

void iwrf_xmit_power_print_format(FILE *out, const iwrf_xmit_power_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_xmit_power_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_XMIT_POWER_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "fl32", "power_dbm_h", sizeof(val.power_dbm_h), (char *) &val.power_dbm_h - id);
  fprintf(out, _dform, "fl32", "power_dbm_v", sizeof(val.power_dbm_v), (char *) &val.power_dbm_v - id);
  fprintf(out, _dform, "si32", "unused[16]", sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}


// print format of rx_power

void iwrf_rx_power_print_format(FILE *out, const iwrf_rx_power_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_rx_power_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_RX_POWER_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "fl32", "max_power_dbm_hc", sizeof(val.max_power_dbm_hc), (char *) &val.max_power_dbm_hc - id);
  fprintf(out, _dform, "fl32", "max_power_dbm_vc", sizeof(val.max_power_dbm_vc), (char *) &val.max_power_dbm_vc - id);
  fprintf(out, _dform, "fl32", "max_power_dbm_hx", sizeof(val.max_power_dbm_hx), (char *) &val.max_power_dbm_hx - id);
  fprintf(out, _dform, "fl32", "max_power_dbm_vx", sizeof(val.max_power_dbm_vx), (char *) &val.max_power_dbm_vx - id);
  fprintf(out, _dform, "si32", "unused[14]", sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}


// print format of xmit_sample

void iwrf_xmit_sample_print_format(FILE *out, const iwrf_xmit_sample_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_xmit_sample_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_XMIT_SAMPLE_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "fl32", "power_dbm_h", sizeof(val.power_dbm_h), (char *) &val.power_dbm_h - id);
  fprintf(out, _dform, "fl32", "power_dbm_v", sizeof(val.power_dbm_v), (char *) &val.power_dbm_v - id);
  fprintf(out, _dform, "si32", "offset", sizeof(val.offset), (char *) &val.offset - id);
  fprintf(out, _dform, "si32", "n_samples", sizeof(val.n_samples), (char *) &val.n_samples - id);
  fprintf(out, _dform, "fl32", "sampling_freq", sizeof(val.sampling_freq), (char *) &val.sampling_freq - id);
  fprintf(out, _dform, "fl32", "scale_h", sizeof(val.scale_h), (char *) &val.scale_h - id);
  fprintf(out, _dform, "fl32", "offset_h", sizeof(val.offset_h), (char *) &val.offset_h - id);
  fprintf(out, _dform, "fl32", "scale_v", sizeof(val.scale_v), (char *) &val.scale_v - id);
  fprintf(out, _dform, "fl32", "offset_v", sizeof(val.offset_v), (char *) &val.offset_v - id);
  fprintf(out, _dform, "si32", "samples_h[512]", sizeof(val.samples_h), (char *) val.samples_h - id);
  fprintf(out, _dform, "si32", "samples_v[512]", sizeof(val.samples_v), (char *) val.samples_v - id);
  fprintf(out, _dform, "si32", "unused[1001]", sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}

// print format of xmit_sample_v2

void iwrf_xmit_sample_v2_print_format(FILE *out, const iwrf_xmit_sample_v2_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_xmit_sample_v2_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_XMIT_SAMPLE_V2_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "n_samples", sizeof(val.n_samples), (char *) &val.n_samples - id);
  fprintf(out, _dform, "si32", "n_channels", sizeof(val.n_channels), (char *) &val.n_channels - id);

  _print_format_divider('-', out);

}


// print format of burst_iq

void iwrf_burst_header_print_format(FILE *out, const iwrf_burst_header_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_burst_header_t'\n  size: %d\n  id: 0x%x\n\n",
          (int) sizeof(val), IWRF_BURST_HEADER_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si64", "pulse_seq_num", sizeof(val.pulse_seq_num), (char *) &val.pulse_seq_num - id);
  fprintf(out, _dform, "si32", "n_samples", sizeof(val.n_samples), (char *) &val.n_samples - id);
  fprintf(out, _dform, "si32", "channel_id", sizeof(val.channel_id), (char *) &val.channel_id - id);
  fprintf(out, _dform, "si32", "iq_encoding", sizeof(val.iq_encoding), (char *) &val.iq_encoding - id);
  fprintf(out, _dform, "fl32", "scale", sizeof(val.scale), (char *) &val.scale - id);
  fprintf(out, _dform, "fl32", "offset", sizeof(val.offset), (char *) &val.offset - id);
  fprintf(out, _dform, "fl32", "power_dbm", sizeof(val.power_dbm), (char *) &val.power_dbm - id);
  fprintf(out, _dform, "fl32", "phase_deg", sizeof(val.phase_deg), (char *) &val.phase_deg - id);
  fprintf(out, _dform, "fl32", "freq_hz", sizeof(val.freq_hz), (char *) &val.freq_hz - id);
  fprintf(out, _dform, "fl32", "sampling_freq_hz", sizeof(val.sampling_freq_hz), (char *) &val.sampling_freq_hz - id);
  fprintf(out, _dform, "fl32", "power_max_dbm", sizeof(val.power_max_dbm), (char *) &val.power_max_dbm - id);
  fprintf(out, _dform, "fl32", "power_p90_dbm", sizeof(val.power_p90_dbm), (char *) &val.power_p90_dbm - id);
  fprintf(out, _dform, "fl32", "pulse_width_sec", sizeof(val.pulse_width_sec), (char *) &val.pulse_width_sec - id);

  _print_format_divider('-', out);

}

  
// print format of status_xml

void iwrf_status_xml_print_format(FILE *out, const iwrf_status_xml_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_status_xml_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_STATUS_XML_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);
  
  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);
  
  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "xml_len", sizeof(val.xml_len), (char *) &val.xml_len - id);

  _print_format_divider('-', out);

}

// print format of antenna_angles

void iwrf_antenna_angles_print_format(FILE *out, const iwrf_antenna_angles_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_antenna_angles_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_ANTENNA_ANGLES_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);
  
  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);
  
  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "scan_mode", sizeof(val.scan_mode), (char *) &val.scan_mode - id);
  fprintf(out, _dform, "si32", "follow_mode", sizeof(val.follow_mode), (char *) &val.follow_mode - id);
  fprintf(out, _dform, "si32", "volume_num", sizeof(val.volume_num), (char *) &val.volume_num - id);
  fprintf(out, _dform, "si32", "sweep_num", sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "fl32", "fixed_el", sizeof(val.fixed_el), (char *) &val.fixed_el - id);
  fprintf(out, _dform, "fl32", "fixed_az", sizeof(val.fixed_az), (char *) &val.fixed_az - id);
  fprintf(out, _dform, "fl32", "elevation", sizeof(val.elevation), (char *) &val.elevation - id);
  fprintf(out, _dform, "fl32", "azimuth", sizeof(val.azimuth), (char *) &val.azimuth - id);
  fprintf(out, _dform, "si32", "antenna_transition", sizeof(val.antenna_transition), (char *) &val.antenna_transition - id);
  fprintf(out, _dform, "si32", "status", sizeof(val.status), (char *) &val.status - id);
  fprintf(out, _dform, "si32", "event_flags", sizeof(val.event_flags), (char *) &val.event_flags - id);
  fprintf(out, _dform, "si32", "unused[7]", sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}

// print format of calibration

void iwrf_calibration_print_format(FILE *out, const iwrf_calibration_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_calibration_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_CALIBRATION_ID);
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
  fprintf(out, _dform, "si32", "unused[49]", sizeof(val.unused), (char *) val.unused - id);
  fprintf(out, _dform, "char", "radar_name[32]", sizeof(val.radar_name), (char *) val.radar_name - id);
  
  _print_format_divider('-', out);

}

  
// print format of event_notice

void iwrf_event_notice_print_format(FILE *out, const iwrf_event_notice_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_event_notice_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_EVENT_NOTICE_ID);
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
  fprintf(out, _dform, "si32", "follow_mode", sizeof(val.follow_mode), (char *) &val.follow_mode - id);
  fprintf(out, _dform, "si32", "volume_num", sizeof(val.volume_num), (char *) &val.volume_num - id);
  fprintf(out, _dform, "si32", "sweep_num", sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "si32", "cause", sizeof(val.cause), (char *) &val.cause - id);
  fprintf(out, _dform, "fl32", "current_fixed_angle", sizeof(val.current_fixed_angle), (char *) &val.current_fixed_angle - id);
  fprintf(out, _dform, "si32", "antenna_transition", sizeof(val.antenna_transition), (char *) &val.antenna_transition - id);
  fprintf(out, _dform, "si32", "unused[7]", sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}


// print format of iwrf_phasecode

void iwrf_phasecode_print_format(FILE *out, const iwrf_phasecode_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_phasecode_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_PHASECODE_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "seq_length", sizeof(val.seq_length), (char *) &val.seq_length - id);
  fprintf(out, _dform, "si32", "spare", sizeof(val.spare), (char *) &val.spare - id);

  fprintf(out, "  phases:\n");
  int offset = (char *) val.phase - id;
  int size = sizeof(fl32);
  for (int ii = 0; ii < 256; ii++) {
    if (ii < 3 || ii > 252) {
      char label[32];
      sprintf(label, "phase[%d].phase_deg_h", ii);
      fprintf(out, _dform, "char", label, size, offset);
      offset += size;
      sprintf(label, "phase[%d].phase_deg_v", ii);
      fprintf(out, _dform, "char", label, size, offset);
      offset += size;
    } else if (ii < 6){
      fprintf(out, "     ....\n");
      offset += 2 * size;
    } else {
      offset += 2 * size;
    }
  }


  fprintf(out, _dform, "fl32", "unused[496]", sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}

  
// print format of xmit_info

void iwrf_xmit_info_print_format(FILE *out, const iwrf_xmit_info_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_xmit_info_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_XMIT_INFO_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "xmit_0_enabled", sizeof(val.xmit_0_enabled), (char *) &val.xmit_0_enabled - id);
  fprintf(out, _dform, "si32", "xmit_1_enabled", sizeof(val.xmit_1_enabled), (char *) &val.xmit_1_enabled - id);
  fprintf(out, _dform, "si32", "xmit_rcv_mode", sizeof(val.xmit_rcv_mode), (char *) &val.xmit_rcv_mode - id);
  fprintf(out, _dform, "si32", "pol_mode", sizeof(val.pol_mode), (char *) &val.pol_mode - id);
  fprintf(out, _dform, "si32", "xmit_phase_mode", sizeof(val.xmit_phase_mode), (char *) &val.xmit_phase_mode - id);
  fprintf(out, _dform, "si32", "prf_mode", sizeof(val.prf_mode), (char *) &val.prf_mode - id);
  fprintf(out, _dform, "si32", "pulse_type", sizeof(val.pulse_type), (char *) &val.pulse_type - id);
  fprintf(out, _dform, "fl32", "prt_usec", sizeof(val.prt_usec), (char *) &val.prt_usec - id);
  fprintf(out, _dform, "fl32", "prt2_usec", sizeof(val.prt2_usec), (char *) &val.prt2_usec - id);
  fprintf(out, _dform, "fl32", "unused[9]", sizeof(val.unused), (char *) val.unused - id);
  
  _print_format_divider('-', out);

}


// print format of pulse_header

void iwrf_pulse_header_print_format(FILE *out, const iwrf_pulse_header_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_pulse_header_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_PULSE_HEADER_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si64", "pulse_seq_num", sizeof(val.pulse_seq_num), (char *) &val.pulse_seq_num - id);
  fprintf(out, _dform, "si32", "scan_mode", sizeof(val.scan_mode), (char *) &val.scan_mode - id);
  fprintf(out, _dform, "si32", "follow_mode", sizeof(val.follow_mode), (char *) &val.follow_mode - id);
  fprintf(out, _dform, "si32", "volume_num", sizeof(val.volume_num), (char *) &val.volume_num - id);
  fprintf(out, _dform, "si32", "sweep_num", sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "fl32", "fixed_el", sizeof(val.fixed_el), (char *) &val.fixed_el - id);
  fprintf(out, _dform, "fl32", "fixed_az", sizeof(val.fixed_az), (char *) &val.fixed_az - id);
  fprintf(out, _dform, "fl32", "elevation", sizeof(val.elevation), (char *) &val.elevation - id);
  fprintf(out, _dform, "fl32", "azimuth", sizeof(val.azimuth), (char *) &val.azimuth - id);
  fprintf(out, _dform, "fl32", "prt", sizeof(val.prt), (char *) &val.prt - id);
  fprintf(out, _dform, "fl32", "prt_next", sizeof(val.prt_next), (char *) &val.prt_next - id);
  fprintf(out, _dform, "fl32", "pulse_width_us", sizeof(val.pulse_width_us), (char *) &val.pulse_width_us - id);
  fprintf(out, _dform, "si32", "n_gates", sizeof(val.n_gates), (char *) &val.n_gates - id);
  fprintf(out, _dform, "si32", "n_channels", sizeof(val.n_channels), (char *) &val.n_channels - id);
  fprintf(out, _dform, "si32", "iq_encoding", sizeof(val.iq_encoding), (char *) &val.iq_encoding - id);
  fprintf(out, _dform, "si32", "hv_flag", sizeof(val.hv_flag), (char *) &val.hv_flag - id);
  fprintf(out, _dform, "si32", "antenna_transition", sizeof(val.antenna_transition), (char *) &val.antenna_transition - id);
  fprintf(out, _dform, "si32", "phase_cohered", sizeof(val.phase_cohered), (char *) &val.phase_cohered - id);
  fprintf(out, _dform, "si32", "status", sizeof(val.status), (char *) &val.status - id);
  fprintf(out, _dform, "si32", "n_data", sizeof(val.n_data), (char *) &val.n_data - id);
  fprintf(out, _dform, "si32", "iq_offset[4]", sizeof(val.iq_offset), (char *) val.iq_offset - id);
  fprintf(out, _dform, "fl32", "burst_mag[4]", sizeof(val.burst_mag), (char *) val.burst_mag - id);
  fprintf(out, _dform, "fl32", "burst_arg[4]", sizeof(val.burst_arg), (char *) val.burst_arg - id);
  fprintf(out, _dform, "fl32", "burst_arg_diff[4]", sizeof(val.burst_arg_diff), (char *) val.burst_arg_diff - id);
  fprintf(out, _dform, "fl32", "scale", sizeof(val.scale), (char *) &val.scale - id);
  fprintf(out, _dform, "fl32", "offset", sizeof(val.offset), (char *) &val.offset - id);
  fprintf(out, _dform, "si32", "n_gates_burst", sizeof(val.n_gates_burst), (char *) &val.n_gates_burst - id);
  fprintf(out, _dform, "fl32", "start_range_m", sizeof(val.start_range_m), (char *) &val.start_range_m - id);
  fprintf(out, _dform, "fl32", "gate_spacing_m", sizeof(val.gate_spacing_m), (char *) &val.gate_spacing_m - id);
  fprintf(out, _dform, "si32", "event_flags", sizeof(val.event_flags), (char *) &val.event_flags - id);
  fprintf(out, _dform, "si32", "txrx_state", sizeof(val.txrx_state), (char *) &val.txrx_state - id);
  fprintf(out, _dform, "si32", "unused[6]", sizeof(val.unused), (char *) val.unused - id);

  _print_format_divider('-', out);

}


// print format of rvp8_pulse_header

void iwrf_rvp8_pulse_header_print_format(FILE *out, const iwrf_rvp8_pulse_header_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_rvp8_pulse_header_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_RVP8_PULSE_HEADER_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "i_version", sizeof(val.i_version), (char *) &val.i_version - id);
  fprintf(out, _dform, "ui08", "i_flags", sizeof(val.i_flags), (char *) &val.i_flags - id);
  fprintf(out, _dform, "ui08", "i_aq_mode", sizeof(val.i_aq_mode), (char *) &val.i_aq_mode - id);
  fprintf(out, _dform, "ui08", "i_polar_bits", sizeof(val.i_polar_bits), (char *) &val.i_polar_bits - id);
  fprintf(out, _dform, "ui08", "i_viq_per_bin", sizeof(val.i_viq_per_bin), (char *) &val.i_viq_per_bin - id);
  fprintf(out, _dform, "ui08", "i_tg_bank", sizeof(val.i_tg_bank), (char *) &val.i_tg_bank - id);
  fprintf(out, _dform, "ui08", "unused1[3]", sizeof(val.unused1), (char *) val.unused1 - id);
  fprintf(out, _dform, "ui16", "i_tx_phase", sizeof(val.i_tx_phase), (char *) &val.i_tx_phase - id);
  fprintf(out, _dform, "ui16", "i_az", sizeof(val.i_az), (char *) &val.i_az - id);
  fprintf(out, _dform, "ui16", "i_el", sizeof(val.i_el), (char *) &val.i_el - id);
  fprintf(out, _dform, "si16", "i_num_vecs", sizeof(val.i_num_vecs), (char *) &val.i_num_vecs - id);
  fprintf(out, _dform, "si16", "i_max_vecs", sizeof(val.i_max_vecs), (char *) &val.i_max_vecs - id);
  fprintf(out, _dform, "ui16", "i_tg_wave", sizeof(val.i_tg_wave), (char *) &val.i_tg_wave - id);
  fprintf(out, _dform, "ui32", "i_btime_api", sizeof(val.i_btime_api), (char *) &val.i_btime_api - id);
  fprintf(out, _dform, "ui32", "i_sys_time", sizeof(val.i_sys_time), (char *) &val.i_sys_time - id);
  fprintf(out, _dform, "ui32", "i_prev_prt", sizeof(val.i_prev_prt), (char *) &val.i_prev_prt - id);
  fprintf(out, _dform, "ui32", "i_next_prt", sizeof(val.i_next_prt), (char *) &val.i_next_prt - id);
  fprintf(out, _dform, "ui32", "i_seq_num", sizeof(val.i_seq_num), (char *) &val.i_seq_num - id);
  fprintf(out, _dform, "ui32", "uiq_perm[2]", sizeof(val.uiq_perm), (char *) val.uiq_perm - id);
  fprintf(out, _dform, "ui32", "uiq_once[2]", sizeof(val.uiq_once), (char *) val.uiq_once - id);
  fprintf(out, _dform, "si32", "i_data_off[4]", sizeof(val.i_data_off), (char *) val.i_data_off - id);
  fprintf(out, _dform, "fl32", "f_burst_mag[4]", sizeof(val.f_burst_mag), (char *) val.f_burst_mag - id);
  fprintf(out, _dform, "ui16", "i_burst_arg[4]", sizeof(val.i_burst_arg), (char *) val.i_burst_arg - id);
  fprintf(out, _dform, "ui16", "i_wrap_iq[4]", sizeof(val.i_wrap_iq), (char *) val.i_wrap_iq - id);
  fprintf(out, _dform, "si32", "unused2[23]", sizeof(val.unused2), (char *) val.unused2 - id);

  _print_format_divider('-', out);

}


// print format of rvp8_ops_info

void iwrf_rvp8_ops_info_print_format(FILE *out, const iwrf_rvp8_ops_info_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_rvp8_ops_info_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_RVP8_OPS_INFO_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "si32", "i_version", sizeof(val.i_version), (char *) &val.i_version - id);
  fprintf(out, _dform, "ui32", "i_major_mode", sizeof(val.i_major_mode), (char *) &val.i_major_mode - id);
  fprintf(out, _dform, "ui32", "i_polarization", sizeof(val.i_polarization), (char *) &val.i_polarization - id);
  fprintf(out, _dform, "ui32", "i_phase_mode_seq", sizeof(val.i_phase_mode_seq), (char *) &val.i_phase_mode_seq - id);
  fprintf(out, _dform, "ui16", "i_task_sweep", sizeof(val.i_task_sweep), (char *) &val.i_task_sweep - id);
  fprintf(out, _dform, "ui16", "i_task_aux_num", sizeof(val.i_task_aux_num), (char *) &val.i_task_aux_num - id);
  fprintf(out, _dform, "si32", "i_task_scan_type", sizeof(val.i_task_scan_type), (char *) &val.i_task_scan_type - id);
  fprintf(out, _dform, "si32", "unused1[3]", sizeof(val.unused1), (char *) val.unused1 - id);
  fprintf(out, _dform, "char", "s_task_name[32]", sizeof(val.s_task_name), (char *) val.s_task_name - id);
  fprintf(out, _dform, "char", "s_site_name[32]", sizeof(val.s_site_name), (char *) val.s_site_name - id);
  fprintf(out, _dform, "ui32", "i_aq_mode", sizeof(val.i_aq_mode), (char *) &val.i_aq_mode - id);
  fprintf(out, _dform, "ui32", "i_unfold_mode", sizeof(val.i_unfold_mode), (char *) &val.i_unfold_mode - id);
  fprintf(out, _dform, "ui32", "i_pwidth_code", sizeof(val.i_pwidth_code), (char *) &val.i_pwidth_code - id);
  fprintf(out, _dform, "fl32", "f_pwidth_usec", sizeof(val.f_pwidth_usec), (char *) &val.f_pwidth_usec - id);
  fprintf(out, _dform, "fl32", "f_dbz_calib", sizeof(val.f_dbz_calib), (char *) &val.f_dbz_calib - id);
  fprintf(out, _dform, "si32", "i_sample_size", sizeof(val.i_sample_size), (char *) &val.i_sample_size - id);
  fprintf(out, _dform, "ui32", "i_mean_angle_sync", sizeof(val.i_mean_angle_sync), (char *) &val.i_mean_angle_sync - id);
  fprintf(out, _dform, "ui32", "i_flags", sizeof(val.i_flags), (char *) &val.i_flags - id);
  fprintf(out, _dform, "si32", "i_playback_version", sizeof(val.i_playback_version), (char *) &val.i_playback_version - id);
  fprintf(out, _dform, "fl32", "f_sy_clk_mhz", sizeof(val.f_sy_clk_mhz), (char *) &val.f_sy_clk_mhz - id);
  fprintf(out, _dform, "fl32", "f_wavelength_cm", sizeof(val.f_wavelength_cm), (char *) &val.f_wavelength_cm - id);
  fprintf(out, _dform, "fl32", "f_saturation_dbm", sizeof(val.f_saturation_dbm), (char *) &val.f_saturation_dbm - id);
  fprintf(out, _dform, "fl32", "f_range_mask_res", sizeof(val.f_range_mask_res), (char *) &val.f_range_mask_res - id);
  fprintf(out, _dform, "ui16", "i_range_mask[512]", sizeof(val.i_range_mask), (char *) val.i_range_mask - id);
  fprintf(out, _dform, "fl32", "f_noise_dbm[4]", sizeof(val.f_noise_dbm), (char *) val.f_noise_dbm - id);
  fprintf(out, _dform, "fl32", "f_noise_stdv_db[4]", sizeof(val.f_noise_stdv_db), (char *) val.f_noise_stdv_db - id);
  fprintf(out, _dform, "fl32", "f_noise_range_km", sizeof(val.f_noise_range_km), (char *) &val.f_noise_range_km - id);
  fprintf(out, _dform, "fl32", "f_noise_prf_hz", sizeof(val.f_noise_prf_hz), (char *) &val.f_noise_prf_hz - id);
  fprintf(out, _dform, "ui16", "i_gparm_latch_sts[2]", sizeof(val.i_gparm_latch_sts), (char *) val.i_gparm_latch_sts - id);
  fprintf(out, _dform, "ui16", "i_gparm_immed_sts[6]", sizeof(val.i_gparm_immed_sts), (char *) val.i_gparm_immed_sts - id);
  fprintf(out, _dform, "ui16", "i_gparm_diag_bits[4]", sizeof(val.i_gparm_diag_bits), (char *) val.i_gparm_diag_bits - id);
  fprintf(out, _dform, "char", "s_version_string[12]", sizeof(val.s_version_string), (char *) val.s_version_string - id);
  fprintf(out, _dform, "si32", "unused2[185]", sizeof(val.unused2), (char *) val.unused2 - id);

  _print_format_divider('-', out);

}

// print format of moments_field_header

void iwrf_moments_field_header_print_format(FILE *out, const iwrf_moments_field_header_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_moments_field_header_t'\n  size: %d\n  id: 0x%x\n\n", 
          (int) sizeof(val), IWRF_MOMENTS_FIELD_HEADER_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;

  fprintf(out, _dform, "char", "name[32]",
          sizeof(val.name), (char *) val.name - id);
  fprintf(out, _dform, "char", "long_name[128]",
          sizeof(val.long_name), (char *) val.long_name - id);
  fprintf(out, _dform, "char", "standard_name[128]", 
          sizeof(val.standard_name), (char *) val.standard_name - id);
  fprintf(out, _dform, "char", "units[16]", 
          sizeof(val.units), (char *) val.units - id);
  fprintf(out, _dform, "char", "threshold_field_name[32]", 
          sizeof(val.threshold_field_name), (char *) val.threshold_field_name - id);

  fprintf(out, _dform, "si32", "encoding", sizeof(val.encoding), (char *) &val.encoding - id);
  fprintf(out, _dform, "si32", "byte_width", sizeof(val.byte_width), (char *) &val.byte_width - id);
  fprintf(out, _dform, "fl32", "scale", sizeof(val.scale), (char *) &val.scale - id);
  fprintf(out, _dform, "fl32", "offset", sizeof(val.offset), (char *) &val.offset - id);
  fprintf(out, _dform, "fl32", "sampling_ratio", sizeof(val.sampling_ratio), (char *) &val.sampling_ratio - id);
  fprintf(out, _dform, "fl32", "threshold_value", sizeof(val.threshold_value), (char *) &val.threshold_value - id);
  fprintf(out, _dform, "si32", "n_samples", sizeof(val.n_samples), (char *) &val.n_samples - id);
  fprintf(out, _dform, "fl32", "nyquist_mps", sizeof(val.nyquist_mps), (char *) &val.nyquist_mps - id);

  _print_format_divider('-', out);

}

// print format of moments_ray_header

void iwrf_moments_ray_header_print_format(FILE *out, const iwrf_moments_ray_header_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_moments_ray_header_t'\n  size: %d\n  id: 0x%x\n\n", 
          (int) sizeof(val), IWRF_MOMENTS_RAY_HEADER_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;

  fprintf(out, _dform, "si32", "volume_num", sizeof(val.volume_num), (char *) &val.volume_num - id);
  fprintf(out, _dform, "si32", "sweep_num", sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "si32", "scan_mode", sizeof(val.scan_mode), (char *) &val.scan_mode - id);
  fprintf(out, _dform, "si32", "follow_mode", sizeof(val.follow_mode), (char *) &val.follow_mode - id);
  fprintf(out, _dform, "si32", "prf_mode", sizeof(val.prf_mode), (char *) &val.prf_mode - id);
  fprintf(out, _dform, "si32", "polarization_mode", sizeof(val.polarization_mode), (char *) &val.polarization_mode - id);
  fprintf(out, _dform, "fl32", "elevation", sizeof(val.elevation), (char *) &val.elevation - id);
  fprintf(out, _dform, "fl32", "azimuth", sizeof(val.azimuth), (char *) &val.azimuth - id);
  fprintf(out, _dform, "fl32", "fixed_angle", sizeof(val.fixed_angle), (char *) &val.fixed_angle - id);
  fprintf(out, _dform, "fl32", "target_scan_rate", sizeof(val.target_scan_rate), (char *) &val.target_scan_rate - id);
  fprintf(out, _dform, "fl32", "true_scan_rate", sizeof(val.true_scan_rate), (char *) &val.true_scan_rate - id);
  fprintf(out, _dform, "si32", "is_indexed", sizeof(val.is_indexed), (char *) &val.is_indexed - id);
  fprintf(out, _dform, "fl32", "angle_res", sizeof(val.angle_res), (char *) &val.angle_res - id);
  fprintf(out, _dform, "si32", "antenna_transition", sizeof(val.antenna_transition), (char *) &val.antenna_transition - id);
  fprintf(out, _dform, "fl32", "prt", sizeof(val.prt), (char *) &val.prt - id);
  fprintf(out, _dform, "fl32", "prt_ratio", sizeof(val.prt_ratio), (char *) &val.prt_ratio - id);
  fprintf(out, _dform, "fl32", "pulse_width_us", sizeof(val.pulse_width_us), (char *) &val.pulse_width_us - id);
  fprintf(out, _dform, "si32", "n_samples", sizeof(val.n_samples), (char *) &val.n_samples - id);
  fprintf(out, _dform, "si32", "n_fields", sizeof(val.n_fields), (char *) &val.n_fields - id);
  fprintf(out, _dform, "si32", "n_gates", sizeof(val.n_gates), (char *) &val.n_gates - id);
  fprintf(out, _dform, "fl32", "start_range_m", sizeof(val.start_range_m), (char *) &val.start_range_m - id);
  fprintf(out, _dform, "fl32", "gate_spacing_m", sizeof(val.gate_spacing_m), (char *) &val.gate_spacing_m - id);
  fprintf(out, _dform, "fl32", "nyquist_mps", sizeof(val.nyquist_mps), (char *) &val.nyquist_mps - id);
  fprintf(out, _dform, "fl32", "unambig_range_km", sizeof(val.unambig_range_km), (char *) &val.unambig_range_km - id);
  fprintf(out, _dform, "fl32", "meas_xmit_power_dbm_h", 
          sizeof(val.meas_xmit_power_dbm_h), (char *) &val.meas_xmit_power_dbm_h - id);
  fprintf(out, _dform, "fl32", "meas_xmit_power_dbm_v", 
          sizeof(val.meas_xmit_power_dbm_v), (char *) &val.meas_xmit_power_dbm_v - id);
  fprintf(out, _dform, "si32", "event_flags", sizeof(val.event_flags), (char *) &val.event_flags - id);

  _print_format_divider('-', out);

}

// print format of moments_field_index

void iwrf_moments_field_index_print_format(FILE *out, const iwrf_moments_field_index_t &val)
{
  
  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_moments_field_index_t'\n  size: %d\n  id: 0x%x\n\n", 
          (int) sizeof(val), IWRF_MOMENTS_FIELD_INDEX_ID);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *start = (char *) val.field_name;

  fprintf(out, _dform, "char", "field_name[32]",
          sizeof(val.field_name), (char *) val.field_name - start);

  fprintf(out, _dform, "si32", "id", sizeof(val.id), (char *) &val.id - start);
  fprintf(out, _dform, "si32", "offset", sizeof(val.offset), (char *) &val.offset - start);
  fprintf(out, _dform, "si32", "len", sizeof(val.len), (char *) &val.len - start);

  _print_format_divider('-', out);

}


// print format of platform_georef

void iwrf_platform_georef_print_format(FILE *out, const iwrf_platform_georef_t &val)
{
  
  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_platform_georef_t'\n  size: %d\n  id: 0x%x\n\n", 
          (int) sizeof(val), IWRF_PLATFORM_GEOREF_ID);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *start = (char *) &val.unit_num;

  fprintf(out, _dform, "si32", "unit_num",
          sizeof(val.unit_num), (char *) &val.unit_num - start);
  fprintf(out, _dform, "si32", "id",
          sizeof(val.unit_id), (char *) &val.unit_id - start);
  fprintf(out, _dform, "fl32", "altitude_msl_km",
          sizeof(val.altitude_msl_km), (char *) &val.altitude_msl_km - start);
  fprintf(out, _dform, "fl32", "altitude_agl_km",
          sizeof(val.altitude_agl_km), (char *) &val.altitude_agl_km - start);
  fprintf(out, _dform, "fl32", "ew_velocity_mps",
          sizeof(val.ew_velocity_mps), (char *) &val.ew_velocity_mps - start);
  fprintf(out, _dform, "fl32", "ns_velocity_mps",
          sizeof(val.ns_velocity_mps), (char *) &val.ns_velocity_mps - start);
  fprintf(out, _dform, "fl32", "vert_velocity_mps",
          sizeof(val.vert_velocity_mps), (char *) &val.vert_velocity_mps - start);
  fprintf(out, _dform, "fl32", "heading_deg",
          sizeof(val.heading_deg), (char *) &val.heading_deg - start);
  fprintf(out, _dform, "fl32", "roll_deg",
          sizeof(val.roll_deg), (char *) &val.roll_deg - start);
  fprintf(out, _dform, "fl32", "pitch_deg",
          sizeof(val.pitch_deg), (char *) &val.pitch_deg - start);
  fprintf(out, _dform, "fl32", "drift_angle_deg",
          sizeof(val.drift_angle_deg), (char *) &val.drift_angle_deg - start);
  fprintf(out, _dform, "fl32", "rotation_angle_deg",
          sizeof(val.rotation_angle_deg), (char *) &val.rotation_angle_deg - start);
  fprintf(out, _dform, "fl32", "tilt_deg",
          sizeof(val.tilt_deg), (char *) &val.tilt_deg - start);
  fprintf(out, _dform, "fl32", "ew_horiz_wind_mps",
          sizeof(val.ew_horiz_wind_mps), (char *) &val.ew_horiz_wind_mps - start);
  fprintf(out, _dform, "fl32", "ns_horiz_wind_mps",
          sizeof(val.ns_horiz_wind_mps), (char *) &val.ns_horiz_wind_mps - start);
  fprintf(out, _dform, "fl32", "vert_wind_mps",
          sizeof(val.vert_wind_mps), (char *) &val.vert_wind_mps - start);
  fprintf(out, _dform, "fl32", "heading_rate_dps",
          sizeof(val.heading_rate_dps), (char *) &val.heading_rate_dps - start);
  fprintf(out, _dform, "fl32", "pitch_rate_dps",
          sizeof(val.pitch_rate_dps), (char *) &val.pitch_rate_dps - start);
  fprintf(out, _dform, "fl32", "drive_angle_1_deg",
          sizeof(val.drive_angle_1_deg), (char *) &val.drive_angle_1_deg - start);
  fprintf(out, _dform, "fl32", "drive_angle_1_deg",
          sizeof(val.drive_angle_2_deg), (char *) &val.drive_angle_2_deg - start);

  fprintf(out, _dform, "fl64", "longitude64",
          sizeof(val.longitude), (char *) &val.longitude - start);
  fprintf(out, _dform, "fl64", "latitude64",
          sizeof(val.latitude), (char *) &val.latitude - start);

  fprintf(out, _dform, "fl32", "track_deg",
          sizeof(val.track_deg), (char *) &val.track_deg - start);
  fprintf(out, _dform, "fl32", "roll_rate_dps",
          sizeof(val.roll_rate_dps), (char *) &val.roll_rate_dps - start);

  _print_format_divider('-', out);

}


// print format of georef_correction

void iwrf_georef_correction_print_format(FILE *out, const iwrf_georef_correction_t &val)
{
  
  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_georef_correction_t'\n  size: %d\n  id: 0x%x\n\n", 
          (int) sizeof(val), IWRF_GEOREF_CORRECTION_ID);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);
  
  const char *start = (char *) &val.azimuth_corr_deg;

  fprintf(out, _dform, "fl32", "azimuth_corr_deg",
          sizeof(val.azimuth_corr_deg), (char *) &val.azimuth_corr_deg - start);
  fprintf(out, _dform, "fl32", "elevation_corr_deg",
          sizeof(val.elevation_corr_deg), (char *) &val.elevation_corr_deg - start);
  fprintf(out, _dform, "fl32", "range_delay_corr_mps",
          sizeof(val.range_delay_corr_mps), (char *) &val.range_delay_corr_mps - start);
  fprintf(out, _dform, "fl32", "longitude_corr_deg",
          sizeof(val.longitude_corr_deg), (char *) &val.longitude_corr_deg - start);
  fprintf(out, _dform, "fl32", "latitude_corr_deg",
          sizeof(val.latitude_corr_deg), (char *) &val.latitude_corr_deg - start);
  fprintf(out, _dform, "fl32", "pressure_alt_corr_km",
          sizeof(val.pressure_alt_corr_km), (char *) &val.pressure_alt_corr_km - start);
  fprintf(out, _dform, "fl32", "radar_alt_corr_km",
          sizeof(val.radar_alt_corr_km), (char *) &val.radar_alt_corr_km - start);
  fprintf(out, _dform, "fl32", "ew_gndspd_corr_mps",
          sizeof(val.ew_gndspd_corr_mps), (char *) &val.ew_gndspd_corr_mps - start);
  fprintf(out, _dform, "fl32", "ns_gndspd_corr_mps",
          sizeof(val.ns_gndspd_corr_mps), (char *) &val.ns_gndspd_corr_mps - start);
  fprintf(out, _dform, "fl32", "vert_vel_corr_mps",
          sizeof(val.vert_vel_corr_mps), (char *) &val.vert_vel_corr_mps - start);
  fprintf(out, _dform, "fl32", "heading_corr_deg",
          sizeof(val.heading_corr_deg), (char *) &val.heading_corr_deg - start);
  fprintf(out, _dform, "fl32", "roll_corr_deg",
          sizeof(val.roll_corr_deg), (char *) &val.roll_corr_deg - start);
  fprintf(out, _dform, "fl32", "pitch_corr_deg",
          sizeof(val.pitch_corr_deg), (char *) &val.pitch_corr_deg - start);
  fprintf(out, _dform, "fl32", "drift_corr_deg",
          sizeof(val.drift_corr_deg), (char *) &val.drift_corr_deg - start);
  fprintf(out, _dform, "fl32", "rot_angle_corr_deg",
          sizeof(val.rot_angle_corr_deg), (char *) &val.rot_angle_corr_deg - start);
  fprintf(out, _dform, "fl32", "tilt_corr_deg",
          sizeof(val.tilt_corr_deg), (char *) &val.tilt_corr_deg - start);

  _print_format_divider('-', out);

}


// print format of user interface task-list

void iwrf_ui_task_operations_print_format(FILE *out, const iwrf_ui_task_operations_t &val)
{

  _print_format_divider('-', out);
  fprintf(out, "  struct: 'iwrf_ui_task_operations_t'\n  size: %d\n  id: 0x%x\n\n", (int) sizeof(val), IWRF_UI_OPERATIONS_ID);
  fprintf(out, "  packet info:\n");
  _print_format_header(out);
  _print_packet_format(out, val.packet);

  fprintf(out, "\n");
  fprintf(out, "  meta-data:\n");
  _print_format_header(out);

  const char *id = (char *) &val.packet.id;
  
  fprintf(out, _dform, "char", "task_name[32]", sizeof(val.task_name[32]), (char *) &val.task_name[32] - id);
  fprintf(out, _dform, "char", "owner_name[32]", sizeof(val.owner_name[32]), (char *) &val.owner_name[32] - id);
  fprintf(out, _dform, "si32", "op_code", sizeof(val.op_code), (char *) &val.op_code - id);
  fprintf(out, _dform, "ui32", "pad1", sizeof(val.pad1), (char *) &val.pad1 - id);
  fprintf(out, _dform, "union", "un", sizeof(val.un), (char *) &val.un - id);

  _print_format_divider('-', out);

}
