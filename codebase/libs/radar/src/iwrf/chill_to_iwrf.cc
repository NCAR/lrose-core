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
// chill_to_iwrf.cc
//
// Utility routines for converting chill time series
// data to iwrf time series format
//
// Mike Dixon, RAL, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// April 2009

#include <dataport/swap.h>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <radar/chill_to_iwrf.hh>

using namespace std;
static bool _chillSweepNum0Based = false;

///////////////////////////////////////////
// string representation of chill enums

string chill_scan_type_to_str(int scan_type)

{
  
  switch (scan_type) {
    case SCAN_TYPE_PPI:
      return "SCAN_TYPE_PPI";
    case SCAN_TYPE_RHI:
      return "SCAN_TYPE_RHI";
    case SCAN_TYPE_FIXED:
      return "SCAN_TYPE_FIXED";
    case SCAN_TYPE_MANPPI:
      return "SCAN_TYPE_MANPPI";
    case SCAN_TYPE_MANRHI:
      return "SCAN_TYPE_MANRHI";
    case SCAN_TYPE_IDLE:
      return "SCAN_TYPE_IDLE";
    default:
      return "UNKNOWN";
  }

}

string chill_follow_mode_to_str(int follow_mode)

{
  
  switch (follow_mode) {
    case FOLLOW_MODE_NONE:
      return "FOLLOW_MODE_NONE";
    case FOLLOW_MODE_SUN:
      return "FOLLOW_MODE_SUN";
    case FOLLOW_MODE_VEHICLE:
      return "FOLLOW_MODE_VEHICLE";
    default:
      return "UNKNOWN";
  }

}

string chill_pol_mode_to_str(int pol_mode)

{
  
  switch (pol_mode) {
    case POL_MODE_V:
      return "POL_MODE_V";
    case POL_MODE_H:
      return "POL_MODE_H";
    case POL_MODE_VH:
      return "POL_MODE_VH";
    case POL_MODE_VHS:
      return "POL_MODE_VHS";
    default:
      return "UNKNOWN";
  }

}

string chill_pulse_type_to_str(int pulse_type)

{
  
  switch (pulse_type) {
    case PULSE_TYPE_RECT_1US:
      return "PULSE_TYPE_RECT_1US";
    case PULSE_TYPE_RECT_200NS:
      return "PULSE_TYPE_RECT_200NS";
    case PULSE_TYPE_GAUSSIAN_1US:
      return "PULSE_TYPE_GAUSSIAN_1US";
    default:
      return "UNKNOWN";
  }

}

string chill_test_type_to_str(int test_type)

{
  
  switch (test_type) {
    case TEST_TYPE_NONE:
      return "TEST_TYPE_NONE";
    case TEST_TYPE_CW_CAL:
      return "TEST_TYPE_CW_CAL";
    case TEST_TYPE_SOLAR_CAL_FIXED:
      return "TEST_TYPE_SOLAR_CAL_FIXED";
    case TEST_TYPE_SOLAR_CAL_SCAN:
      return "TEST_TYPE_SOLAR_CAL_SCAN";
    case TEST_TYPE_NOISE_SOURCE_H:
      return "TEST_TYPE_NOISE_SOURCE_H";
    case TEST_TYPE_NOISE_SOURCE_V:
      return "TEST_TYPE_NOISE_SOURCE_V";
    case TEST_TYPE_BLUESKY:
      return "TEST_TYPE_BLUESKY";
    case TEST_TYPE_SAVEPARAMS:
      return "TEST_TYPE_SAVEPARAMS";
    default:
      return "UNKNOWN";
  }

}

string chill_event_notice_cause_to_str(int event_notice_cause)

{
  
  switch (event_notice_cause) {
    case END_NOTICE_CAUSE_DONE:
      return "END_NOTICE_CAUSE_DONE";
    case END_NOTICE_CAUSE_TIMEOUT:
      return "END_NOTICE_CAUSE_TIMEOUT";
    case END_NOTICE_CAUSE_TIMER:
      return "END_NOTICE_CAUSE_TIMER";
    case END_NOTICE_CAUSE_ABORT:
      return "END_NOTICE_CAUSE_ABORT";
    case END_NOTICE_CAUSE_ERROR_ABORT:
      return "END_NOTICE_CAUSE_ERROR_ABORT";
    case END_NOTICE_CAUSE_RESTART:
      return "END_NOTICE_CAUSE_RESTART";
    default:
      return "UNKNOWN";
  }

}

////////////////////////////////////////////////
// print the contents of a generic packet header

void chill_generic_packet_header_print(ostream &out,
                                       const generic_packet_header_t &hdr)

{

  out << "===============================================" << endl;
  out << "CHILL GENERIC PACKET HEADER" << endl;
  out << "  magic_word: " << hdr.magic_word << endl;
  out << "  payload_length: " << hdr.payload_length << endl;

}

////////////////////////////////////////////
// print the contents of a radar info struct

void chill_radar_info_print(ostream &out,
                            const radar_info_t &info)

{

  out << "===============================================" << endl;
  out << "CHILL RADAR INFO HSK" << endl;
  // out << "  id: " << info.id << endl;
  // out << "  length: " << info.length << endl;
  out << "  radar_name: " << info.radar_name << endl;
  out << "  latitude_d: " << info.latitude_d << endl;
  out << "  longitude_d: " << info.longitude_d << endl;
  out << "  altitude_m: " << info.altitude_m << endl;
  out << "  beamwidth_d: " << info.beamwidth_d << endl;
  out << "  wavelength_cm: " << info.wavelength_cm << endl;
  out << "  gain_ant_h_db: " << info.gain_ant_h_db << endl;
  out << "  gain_ant_v_db: " << info.gain_ant_v_db << endl;
  out << "  zdr_cal_base_db: " << info.zdr_cal_base_db << endl;
  out << "  phidp_rot_d: " << info.phidp_rot_d << endl;
  out << "  base_radar_constant_db: "
      << info.base_radar_constant_db << endl;
  out << "  power_measurement_loss_h_db: "
      << info.power_measurement_loss_h_db << endl;
  out << "  power_measurement_loss_v_db: "
      << info.power_measurement_loss_v_db << endl;
  out << "  zdr_cal_base_vhs_db: "
      << info.zdr_cal_base_vhs_db << endl;
  out << "  test_power_h_db: " << info.test_power_h_db << endl;
  out << "  test_power_v_db: " << info.test_power_v_db << endl;
  out << "  dc_loss_h_db: " << info.dc_loss_h_db << endl;
  out << "  dc_loss_v_db: " << info.dc_loss_v_db << endl;

}

////////////////////////////////////////////
// print the contents of a scan segment struct

void chill_scan_seg_print(ostream &out,
                          const scan_seg_t &seg)

{

  out << "===============================================" << endl;
  out << "CHILL SCAN SEGMENT HSK" << endl;

  // out << "  id: " << seg.id << endl;
  // out << "  length: " << seg.length << endl;
  out << "  az_manual: " << seg.az_manual << endl;
  out << "  el_manual: " << seg.el_manual << endl;
  out << "  az_start: " << seg.az_start << endl;
  out << "  el_start: " << seg.el_start << endl;
  out << "  scan_rate: " << seg.scan_rate << endl;
  out << "  segname: " << seg.segname << endl;
  out << "  opt.rmax_km: " << seg.opt.rmax_km << endl;
  out << "  opt.htmax_km: " << seg.opt.htmax_km << endl;
  out << "  opt.res_m: " << seg.opt.res_m << endl;
  out << "  follow_mode: " << seg.follow_mode << endl;
  out << "  scan_type: " << chill_scan_type_to_str(seg.scan_type) << endl;
  out << "  scan_flags: " << seg.scan_flags << endl;
  out << "  volume_num: " << seg.volume_num << endl;
  out << "  sweep_num: " << seg.sweep_num << endl;
  out << "  time_limit: " << seg.time_limit << endl;
  out << "  webtilt: " << seg.webtilt << endl;
  out << "  left_limit: " << seg.left_limit << endl;
  out << "  right_limit: " << seg.right_limit << endl;
  out << "  up_limit: " << seg.up_limit << endl;
  out << "  down_limit: " << seg.down_limit << endl;
  out << "  step: " << seg.step << endl;
  out << "  max_sweeps: " << seg.max_sweeps << endl;
  out << "  filter_break_sweep: " << seg.filter_break_sweep << endl;
  out << "  clutter_filter1: " << seg.clutter_filter1 << endl;
  out << "  clutter_filter2: " << seg.clutter_filter2 << endl;
  out << "  project: " << seg.project << endl;
  out << "  current_fixed_angle: " << seg.current_fixed_angle << endl;

}

////////////////////////////////////////////
// print the contents of a proc info struct

void chill_proc_info_print(ostream &out,
                           const processor_info_t &proc)

{

  out << "===============================================" << endl;
  out << "CHILL PROCESSOR INFO HSK" << endl;

  // out << "  id: " << proc.id << endl;
  // out << "  length: " << proc.length << endl;
  out << "  polarization_mode: "
      << chill_pol_mode_to_str(proc.polarization_mode) << endl;
  if (proc.processing_mode & PROC_MODE_INDEXEDBEAM_MSK) {
    cerr << "  processing_mode: indexed_beams" << endl;
  }
  if (proc.processing_mode & PROC_MODE_LONGINT_MSK) {
    cerr << "  processing_mode: long int" << endl;
  }
  if (proc.processing_mode & PROC_MODE_DUALPRT_MSK) {
    cerr << "  processing_mode: dual_prt" << endl;
  }
  if (proc.processing_mode & PROC_MODE_PHASECODE_MSK) {
    cerr << "  processing_mode: phasecode" << endl;
  }
  out << "  pulse_type: "
      << chill_pulse_type_to_str(proc.pulse_type) << endl;
  out << "  test_type: "
      << chill_test_type_to_str(proc.test_type) << endl;
  out << "  integration_cycle_pulses: "
      << proc.integration_cycle_pulses << endl;
  out << "  clutter_filter_number: " << proc.clutter_filter_number << endl;
  out << "  range_gate_averaging: " << proc.range_gate_averaging << endl;
  out << "  indexed_beam_width_d: " << proc.indexed_beam_width_d << endl;
  out << "  gate_spacing_m: " << proc.gate_spacing_m << endl;
  out << "  prt_usec: " << proc.prt_usec << endl;
  out << "  range_start_km: " << proc.range_start_km << endl;
  out << "  range_stop_km: " << proc.range_stop_km << endl;
  out << "  max_gate: " << proc.max_gate << endl;
  out << "  test_power_dbm: " << proc.test_power_dbm << endl;
  out << "  test_pulse_range_km: " << proc.test_pulse_range_km << endl;
  out << "  test_pulse_length_usec: " << proc.test_pulse_length_usec << endl;
  out << "  prt2_usec: " << proc.prt2_usec << endl;
  out << "  range_offset_m: " << proc.range_offset_m << endl;

}

///////////////////////////////////////////////
// print the contents of a power_update struct

void chill_power_update_print(ostream &out,
                              const power_update_t &pup)

{

  out << "===============================================" << endl;
  out << "CHILL POWER UPDATE HSK" << endl;

  char tmp[128];
  sprintf(tmp, "  id: %x", pup.id);
  out << "  id: " << tmp << endl;
  out << "  id: " << pup.id << endl;
  out << "  length: " << pup.length << endl;
  out << "  h_power_dbm: " << pup.h_power_dbm << endl;
  out << "  v_power_dbm: " << pup.v_power_dbm << endl;

}

///////////////////////////////////////////////
// print the contents of a xmit_sample struct

void chill_xmit_sample_print(ostream &out,
                             const xmit_sample_t &xmit)

{

  out << "===============================================" << endl;
  out << "CHILL XMIT SAMPLE HSK" << endl;

  out << "  h_power_dbm: " << xmit.h_power_dbm << endl;
  out << "  v_power_dbm: " << xmit.v_power_dbm << endl;
  out << "  offset: " << xmit.offset << endl;
  out << "  samples: " << xmit.samples << endl;

  for (int ii = 0; ii < xmit.samples; ii++) {
    out << "    num, h, v: "
        << ii << ", "
        << xmit.h_samples[ii] << ", "
        << xmit.v_samples[ii] << endl;
  }

}

///////////////////////////////////////////////
// print the contents of a event_notice struct

void chill_event_notice_print(ostream &out,
                              const event_notice_t &event)

{

  out << "===============================================" << endl;
  out << "CHILL EVENT NOTICE HSK" << endl;

  out << "  flags: " << event.flags << endl;
  if (event.flags & EN_START_SWEEP) {
    out << "         start of sweep" << endl;
  }
  if (event.flags & EN_END_SWEEP) {
    out << "         end of sweep" << endl;
  }
  if (event.flags & EN_END_VOLUME) {
    out << "         end of volume" << endl;
  }
  out << "  cause: "
      << chill_event_notice_cause_to_str(event.cause) << endl;

}

///////////////////////////////////////////////
// print the contents of a cal_terms struct

void chill_cal_terms_print(ostream &out,
                           const cal_terms_t &cal)

{

  out << "===============================================" << endl;
  out << "CHILL CAL TERMS HSK" << endl;

  out << "  noise_v_rx_1: " << cal.noise_v_rx_1 << endl;
  out << "  noise_h_rx_2: " << cal.noise_h_rx_2 << endl;
  out << "  noise_v_rx_2: " << cal.noise_v_rx_2 << endl;
  out << "  noise_h_rx_1: " << cal.noise_h_rx_1 << endl;
  out << "  zcon_h_db: " << cal.zcon_h_db << endl;
  out << "  zcon_v_db: " << cal.zcon_v_db << endl;
  out << "  zdr_bias_db: " << cal.zdr_bias_db << endl;
  out << "  ldr_bias_h_db: " << cal.ldr_bias_h_db << endl;
  out << "  ldr_bias_v_db: " << cal.ldr_bias_v_db << endl;
  out << "  noise_source_h_db: " << cal.noise_source_h_db << endl;
  out << "  noise_source_v_db: " << cal.noise_source_v_db << endl;
  out << "  gain_v_rx_1_db: " << cal.gain_v_rx_1_db << endl;
  out << "  gain_h_rx_2_db: " << cal.gain_h_rx_2_db << endl;
  out << "  gain_v_rx_2_db: " << cal.gain_v_rx_2_db << endl;
  out << "  gain_h_rx_1_db: " << cal.gain_h_rx_1_db << endl;
  out << "  sun_pwr_v_rx_1_db: " << cal.sun_pwr_v_rx_1_db << endl;
  out << "  sun_pwr_h_rx_2_db: " << cal.sun_pwr_h_rx_2_db << endl;
  out << "  sun_pwr_v_rx_2_db: " << cal.sun_pwr_v_rx_2_db << endl;
  out << "  sun_pwr_h_rx_1_db: " << cal.sun_pwr_h_rx_1_db << endl;
  
}

///////////////////////////////////////////////
// print the contents of a phasecode struct

void chill_phasecode_print(ostream &out,
                           const phasecode_t &code)

{

  out << "===============================================" << endl;
  out << "CHILL PHASECODE HSK" << endl;
  
  out << "  seq_length: " << code.seq_length << endl;

  for (int ii = 0; ii < code.seq_length; ii++) {
    out << "    num, phase_h, phase_v: "
        << ii << ", "
        << code.phase[ii].phase_h_d << ", "
        << code.phase[ii].phase_v_d << endl;
  }

}

///////////////////////////////////////////////
// print the contents of a xmit_info struct

void chill_xmit_info_print(ostream &out,
                           const xmit_info_t &info)

{

  out << "===============================================" << endl;
  out << "CHILL XMIT INFO HSK" << endl;
  
  if (info.xmit_enables & XMIT_H_ENABLE) {
    out << "  H xmit enabled" << endl;
  }
  if (info.xmit_enables & XMIT_V_ENABLE) {
    out << "  V xmit enabled" << endl;
  }
  out << "  polarization_mode: "
      << chill_pol_mode_to_str(info.polarization_mode) << endl;
  out << "  pulse_type: " << info.pulse_type << endl;
  out << "  prt_usec: " << info.prt_usec << endl;
  out << "  prt2_usec: " << info.prt2_usec << endl;

}

/////////////////////////////////////////////////////
// print the contents of a antenna_correction struct

void chill_ant_corr_print(ostream &out,
                          const antenna_correction_t &corr)

{
  
  out << "===============================================" << endl;
  out << "CHILL ANTENNA CORRECTION HSK" << endl;
  
  out << "  azimuth: " << corr.azimuth << endl;
  out << "  elevation: " << corr.elevation << endl;

}

///////////////////////////////////////////////
// print the contents of a version struct

void chill_sdb_version_print(ostream &out,
                             const sdb_version_hdr_t &version)
  
{

  out << "===============================================" << endl;
  out << "CHILL SDB VERSION" << endl;

  char hexStr[128];
  sprintf(hexStr, "%x", version.id);
  out << "  id: " << hexStr << ", " << version.id << endl;
  out << "  length: " << version.length << endl;
  int streamMinor = version.version >> 16;
  int streamMajor = (version.version << 16) >> 16;
  out << "  stream format version: "
      << streamMajor << "." << streamMinor << endl;
  int creatorMinor = version.version >> 16;
  int creatorMajor = (version.version << 16) >> 16;
  out << "  creatorformat version: "
      << creatorMajor << "." << creatorMinor << endl;

}

////////////////////////////////////////////
// load up IWRF radar_info from CHILL types

void chill_iwrf_radar_info_load(const radar_info_t &info,
                                si64 seq_num,
                                iwrf_radar_info_t &iwrf)
  
{
  
  iwrf_radar_info_init(iwrf);
  iwrf.packet.seq_num = seq_num;

  iwrf.latitude_deg = info.latitude_d;
  iwrf.longitude_deg = info.longitude_d;
  iwrf.altitude_m = info.altitude_m;
  iwrf.platform_type = IWRF_RADAR_PLATFORM_FIXED;

  iwrf.beamwidth_deg_h = info.beamwidth_d;
  iwrf.beamwidth_deg_v = info.beamwidth_d;
  iwrf.wavelength_cm = info.wavelength_cm;
  
  iwrf.nominal_gain_ant_db_h = info.gain_ant_h_db;
  iwrf.nominal_gain_ant_db_v = info.gain_ant_v_db;

  STRncopy(iwrf.radar_name, info.radar_name, IWRF_MAX_RADAR_NAME);
  STRncopy(iwrf.site_name, info.radar_name, IWRF_MAX_SITE_NAME);

}

//////////////////////////////////////////////
// load up IWRF scan_segment from CHILL types

void chill_iwrf_scan_seg_load(const scan_seg_t &scan,
                              si64 seq_num,
                              iwrf_scan_segment_t &iwrf)
  
{
  
  iwrf_scan_segment_init(iwrf);
  iwrf.packet.seq_num = seq_num;
  
  switch (scan.scan_type) {
    case SCAN_TYPE_PPI:
      if (scan.scan_flags & SF_SECTOR) {
        iwrf.scan_mode = IWRF_SCAN_MODE_SECTOR;
      } else {
        iwrf.scan_mode = IWRF_SCAN_MODE_AZ_SUR_360;
      }
      break;
    case SCAN_TYPE_RHI:
      iwrf.scan_mode = IWRF_SCAN_MODE_RHI;
      break;
    case SCAN_TYPE_FIXED:
      iwrf.scan_mode = IWRF_SCAN_MODE_POINTING;
      break;
    case SCAN_TYPE_MANPPI:
      iwrf.scan_mode = IWRF_SCAN_MODE_MANPPI;
      break;
    case SCAN_TYPE_MANRHI:
      iwrf.scan_mode = IWRF_SCAN_MODE_MANRHI;
      break;
    case SCAN_TYPE_IDLE:
      iwrf.scan_mode = IWRF_SCAN_MODE_IDLE;
      break;
    default:
      iwrf.scan_mode = IWRF_SCAN_MODE_AZ_SUR_360;
  }

  switch (scan.follow_mode) {
    case FOLLOW_MODE_NONE:
      iwrf.follow_mode = IWRF_FOLLOW_MODE_NONE;
      break;
    case FOLLOW_MODE_SUN:
      iwrf.follow_mode = IWRF_FOLLOW_MODE_SUN;
      break;
    case FOLLOW_MODE_VEHICLE:
      iwrf.follow_mode = IWRF_FOLLOW_MODE_VEHICLE;
      break;
    default:
      iwrf.follow_mode = IWRF_FOLLOW_MODE_NOT_SET;
  }

  iwrf.volume_num = scan.volume_num;

  if (scan.sweep_num == 0) {
    _chillSweepNum0Based = true;
  }
  if (_chillSweepNum0Based) {
    iwrf.sweep_num = scan.sweep_num;
  } else {
    iwrf.sweep_num = scan.sweep_num - 1;
  }

  iwrf.time_limit = scan.time_limit;
  iwrf.az_manual = scan.az_manual;
  iwrf.el_manual = scan.el_manual;
  iwrf.az_start = scan.az_start;
  iwrf.el_start = scan.el_start;
  iwrf.scan_rate = scan.scan_rate;
  iwrf.left_limit = scan.left_limit;
  iwrf.right_limit = scan.right_limit;
  iwrf.up_limit = scan.up_limit;
  iwrf.down_limit = scan.down_limit;
  iwrf.step = scan.step;
  iwrf.current_fixed_angle = scan.current_fixed_angle;

  if (scan.scan_flags & 0x02) {
    iwrf.init_direction_cw = true;
  } else {
    iwrf.init_direction_cw = false;
  }
  iwrf.init_direction_up = true;
  iwrf.n_sweeps = scan.max_sweeps;
  iwrf.optimizer_rmax_km = scan.opt.rmax_km;
  iwrf.optimizer_htmax_km = scan.opt.htmax_km;
  iwrf.optimizer_res_m = scan.opt.res_m;
  STRncopy(iwrf.segment_name, scan.segname, IWRF_MAX_SEGMENT_NAME);
  STRncopy(iwrf.project_name, scan.project, IWRF_MAX_PROJECT_NAME);

  // iwrf.fixed_angles[IWRF_MAX_FIXED_ANGLES];

}

////////////////////////////////////////////////////
// load up IWRF antenna_correction from CHILL types

void chill_iwrf_ant_corr_load(const antenna_correction_t &corr,
                              si64 seq_num,
                              iwrf_antenna_correction_t &iwrf)
  
{
  
  iwrf_antenna_correction_init(iwrf);
  iwrf.packet.seq_num = seq_num;

  iwrf.az_correction = corr.azimuth;
  iwrf.el_correction = corr.elevation;

}

////////////////////////////////////////////////////
// load up IWRF ts_processing from CHILL types

void chill_iwrf_ts_proc_load(const processor_info_t &proc,
                             si64 seq_num,
                             iwrf_ts_processing_t &iwrf)
  
{
  
  iwrf_ts_processing_init(iwrf);
  iwrf.packet.seq_num = seq_num;

  switch (proc.polarization_mode) {
    case POL_MODE_V:
      iwrf.xmit_rcv_mode = IWRF_SINGLE_POL;
      iwrf.pol_mode = IWRF_POL_MODE_V;
      break;
    case POL_MODE_H:
      iwrf.xmit_rcv_mode = IWRF_SINGLE_POL;
      iwrf.pol_mode = IWRF_POL_MODE_H;
      break;
    case POL_MODE_VH:
      iwrf.xmit_rcv_mode = IWRF_ALT_HV_CO_CROSS;
      iwrf.pol_mode = IWRF_POL_MODE_HV_ALT;
      break;
    case POL_MODE_VHS:
      iwrf.xmit_rcv_mode = IWRF_SIM_HV_FIXED_HV;
      iwrf.pol_mode = IWRF_POL_MODE_HV_SIM;
      break;
    default:
      iwrf.xmit_rcv_mode = IWRF_SINGLE_POL;
      iwrf.pol_mode = IWRF_POL_MODE_H;
  }

  iwrf.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_FIXED;
  iwrf.prf_mode = IWRF_PRF_MODE_FIXED;

  switch (proc.pulse_type) {
    case PULSE_TYPE_RECT_1US:
      iwrf.pulse_type = IWRF_PULSE_TYPE_RECT;
      iwrf.pulse_width_us = 1.0;
      break;
    case PULSE_TYPE_RECT_200NS:
      iwrf.pulse_type = IWRF_PULSE_TYPE_RECT;
      iwrf.pulse_width_us = 0.2;
      break;
    case PULSE_TYPE_GAUSSIAN_1US:
      iwrf.pulse_type = IWRF_PULSE_TYPE_GAUSSIAN;
      iwrf.pulse_width_us = 1.0;
      break;
    default:
      iwrf.pulse_type = IWRF_PULSE_TYPE_RECT;
      iwrf.pulse_width_us = 1.0;
  }

  iwrf.prt_usec = proc.prt_usec;
  iwrf.prt2_usec = proc.prt_usec;

  iwrf.cal_type = IWRF_CAL_TYPE_CW_CAL;

  iwrf.burst_range_offset_m = proc.range_offset_m;
  iwrf.start_range_m = -proc.range_offset_m;
  iwrf.gate_spacing_m = proc.gate_spacing_m;

  iwrf.integration_cycle_pulses = proc.integration_cycle_pulses;
  iwrf.clutter_filter_number = proc.clutter_filter_number;
  iwrf.range_gate_averaging = proc.range_gate_averaging;
  iwrf.max_gate = proc.max_gate;
  iwrf.test_power_dbm = proc.test_power_dbm;
  iwrf.test_pulse_range_km = proc.test_pulse_range_km;
  iwrf.test_pulse_length_usec = proc.test_pulse_length_usec;

}

////////////////////////////////////////////
// load up IWRF xmit_power from CHILL types

void chill_iwrf_xmit_power_load(const power_update_t &pwr,
                                si64 seq_num,
                                iwrf_xmit_power_t &iwrf)

{

  iwrf_xmit_power_init(iwrf);
  iwrf.packet.seq_num = seq_num;
  
  iwrf.power_dbm_h = pwr.h_power_dbm;
  iwrf.power_dbm_v = pwr.v_power_dbm;
  
}

////////////////////////////////////////////
// load up IWRF xmit_sample from CHILL types

void chill_iwrf_xmit_sample_load(const xmit_sample_t &sample,
                                 si64 seq_num,
                                 iwrf_xmit_sample_t &iwrf)

{

  iwrf_xmit_sample_init(iwrf);
  iwrf.packet.seq_num = seq_num;

  iwrf.power_dbm_h = sample.h_power_dbm;
  iwrf.power_dbm_v = sample.v_power_dbm;
  iwrf.offset = sample.offset;
  iwrf.n_samples = sample.samples;
  if (iwrf.n_samples > IWRF_N_TXSAMP) {
    iwrf.n_samples = IWRF_N_TXSAMP;
  }
  iwrf.sampling_freq = 4.0e7;
  iwrf.scale_h = 1.0;
  iwrf.offset_h = 0.0;
  iwrf.scale_v = 1.0;
  iwrf.offset_v = 0.0;

  for (int ii = 0; ii < iwrf.n_samples; ii++) {
    iwrf.samples_h[ii] = sample.h_samples[ii];
    iwrf.samples_v[ii] = sample.v_samples[ii];
  }

}

////////////////////////////////////////////
// load up IWRF xmit_info from CHILL types

void chill_iwrf_xmit_info_load(const xmit_info_t &info,
                               si64 seq_num,
                               iwrf_xmit_info_t &iwrf)

{

  iwrf_xmit_info_init(iwrf);
  iwrf.packet.seq_num = seq_num;

  iwrf.xmit_0_enabled = info.xmit_enables & XMIT_H_ENABLE;
  iwrf.xmit_1_enabled = info.xmit_enables & XMIT_V_ENABLE;

  switch (info.polarization_mode) {
    case POL_MODE_V:
      iwrf.xmit_rcv_mode = IWRF_SINGLE_POL;
      iwrf.pol_mode = IWRF_POL_MODE_V;
      break;
    case POL_MODE_H:
      iwrf.xmit_rcv_mode = IWRF_SINGLE_POL;
      iwrf.pol_mode = IWRF_POL_MODE_H;
      break;
    case POL_MODE_VH:
      iwrf.xmit_rcv_mode = IWRF_ALT_HV_CO_CROSS;
      iwrf.pol_mode = IWRF_POL_MODE_HV_ALT;
      break;
    case POL_MODE_VHS:
      iwrf.xmit_rcv_mode = IWRF_SIM_HV_FIXED_HV;
      iwrf.pol_mode = IWRF_POL_MODE_HV_SIM;
      break;
    default:
      iwrf.xmit_rcv_mode = IWRF_SINGLE_POL;
      iwrf.pol_mode = IWRF_POL_MODE_H;
  }

  iwrf.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_FIXED;
  iwrf.prf_mode = IWRF_PRF_MODE_FIXED;

  switch (info.pulse_type) {
    case PULSE_TYPE_RECT_1US:
      iwrf.pulse_type = IWRF_PULSE_TYPE_RECT;
      break;
    case PULSE_TYPE_RECT_200NS:
      iwrf.pulse_type = IWRF_PULSE_TYPE_RECT;
      break;
    case PULSE_TYPE_GAUSSIAN_1US:
      iwrf.pulse_type = IWRF_PULSE_TYPE_GAUSSIAN;
      break;
    default:
      iwrf.pulse_type = IWRF_PULSE_TYPE_RECT;
  }

  iwrf.prt_usec = info.prt_usec;
  iwrf.prt2_usec = info.prt2_usec;

}

////////////////////////////////////////////
// load up IWRF calibration from CHILL types

void chill_iwrf_calibration_load(const radar_info_t &rinfo,
                                 const cal_terms_t &cal,
                                 const power_update_t &pwr,
                                 si64 seq_num,
                                 iwrf_calibration_t &iwrf)

{

  iwrf_calibration_init(iwrf);
  iwrf.packet.seq_num = seq_num;

  iwrf.wavelength_cm = rinfo.wavelength_cm;
  iwrf.beamwidth_deg_h = rinfo.beamwidth_d;
  iwrf.beamwidth_deg_v = rinfo.beamwidth_d;
  iwrf.gain_ant_db_h = rinfo.gain_ant_h_db;
  iwrf.gain_ant_db_v = rinfo.gain_ant_v_db;
  iwrf.pulse_width_us = 1.0;
  iwrf.xmit_power_dbm_h = pwr.h_power_dbm;
  iwrf.xmit_power_dbm_v = pwr.v_power_dbm;
  iwrf.two_way_waveguide_loss_db_h = IWRF_MISSING_FLOAT;
  iwrf.two_way_waveguide_loss_db_v = IWRF_MISSING_FLOAT;
  iwrf.two_way_radome_loss_db_h = IWRF_MISSING_FLOAT;
  iwrf.two_way_radome_loss_db_v = IWRF_MISSING_FLOAT;
  iwrf.receiver_mismatch_loss_db = IWRF_MISSING_FLOAT;
  iwrf.radar_constant_h = cal.zcon_h_db + cal.gain_h_rx_1_db - cal.noise_h_rx_1;
  iwrf.radar_constant_v = cal.zcon_v_db + cal.gain_v_rx_1_db - cal.noise_v_rx_1;
  iwrf.noise_dbm_hc = cal.noise_h_rx_1;
  iwrf.noise_dbm_hx = cal.noise_h_rx_2;
  iwrf.noise_dbm_vc = cal.noise_v_rx_1;
  iwrf.noise_dbm_vx = cal.noise_v_rx_2;
  iwrf.receiver_gain_db_hc = cal.gain_h_rx_1_db;
  iwrf.receiver_gain_db_hx = cal.gain_h_rx_2_db;
  iwrf.receiver_gain_db_vc = cal.gain_v_rx_1_db;
  iwrf.receiver_gain_db_vx = cal.gain_v_rx_2_db;
  iwrf.base_dbz_1km_hc = cal.zcon_h_db;
  iwrf.base_dbz_1km_hx = cal.zcon_h_db;
  iwrf.base_dbz_1km_vc = cal.zcon_v_db;
  iwrf.base_dbz_1km_vx = cal.zcon_v_db;
  iwrf.sun_power_dbm_hc = cal.sun_pwr_h_rx_1_db;
  iwrf.sun_power_dbm_hx = cal.sun_pwr_h_rx_2_db;
  iwrf.sun_power_dbm_vc = cal.sun_pwr_v_rx_1_db;
  iwrf.sun_power_dbm_vx = cal.sun_pwr_v_rx_2_db;
  iwrf.noise_source_power_dbm_h = cal.noise_source_h_db;
  iwrf.noise_source_power_dbm_v = cal.noise_source_v_db;
  iwrf.power_meas_loss_db_h = rinfo.power_measurement_loss_h_db;
  iwrf.power_meas_loss_db_v = rinfo.power_measurement_loss_v_db;
  iwrf.coupler_forward_loss_db_h = rinfo.dc_loss_h_db;
  iwrf.coupler_forward_loss_db_v = rinfo.dc_loss_v_db;
  iwrf.test_power_dbm_h = rinfo.test_power_h_db;
  iwrf.test_power_dbm_v = rinfo.test_power_v_db;
  iwrf.zdr_correction_db = - cal.zdr_bias_db;
  iwrf.ldr_correction_db_h = - cal.ldr_bias_h_db;
  iwrf.ldr_correction_db_v = - cal.ldr_bias_v_db;
  iwrf.phidp_rot_deg = rinfo.phidp_rot_d;
  
}

////////////////////////////////////////////
// load up IWRF event_notice from CHILL types

void chill_iwrf_event_notice_load(const event_notice_t &event,
                                  const scan_seg_t &scan,
                                  si64 seq_num,
                                  iwrf_event_notice_t &iwrf)

{

  iwrf_event_notice_init(iwrf);
  iwrf.packet.seq_num = seq_num;

  if (event.flags & EN_START_SWEEP) {
    iwrf.start_of_sweep = 1;
  }
  if (event.flags & EN_END_SWEEP) {
    iwrf.end_of_sweep = 1;
  }
  iwrf.start_of_volume = 0;
  if (event.flags & EN_END_VOLUME) {
    iwrf.end_of_volume = 1;
  }

  iwrf.cause = event.cause;

  switch (scan.scan_type) {
    case SCAN_TYPE_PPI:
      if (scan.scan_flags & SF_SECTOR) {
        iwrf.scan_mode = IWRF_SCAN_MODE_SECTOR;
      } else {
        iwrf.scan_mode = IWRF_SCAN_MODE_AZ_SUR_360;
      }
      break;
    case SCAN_TYPE_RHI:
      iwrf.scan_mode = IWRF_SCAN_MODE_RHI;
      break;
    case SCAN_TYPE_FIXED:
      iwrf.scan_mode = IWRF_SCAN_MODE_POINTING;
      break;
    case SCAN_TYPE_MANPPI:
      iwrf.scan_mode = IWRF_SCAN_MODE_MANPPI;
      break;
    case SCAN_TYPE_MANRHI:
      iwrf.scan_mode = IWRF_SCAN_MODE_MANRHI;
      break;
    case SCAN_TYPE_IDLE:
      iwrf.scan_mode = IWRF_SCAN_MODE_IDLE;
      break;
    default:
      iwrf.scan_mode = IWRF_SCAN_MODE_AZ_SUR_360;
  }

  switch (scan.follow_mode) {
    case FOLLOW_MODE_NONE:
      iwrf.follow_mode = IWRF_FOLLOW_MODE_NONE;
      break;
    case FOLLOW_MODE_SUN:
      iwrf.follow_mode = IWRF_FOLLOW_MODE_SUN;
      break;
    case FOLLOW_MODE_VEHICLE:
      iwrf.follow_mode = IWRF_FOLLOW_MODE_VEHICLE;
      break;
    default:
      iwrf.follow_mode = IWRF_FOLLOW_MODE_NOT_SET;
  }

  iwrf.volume_num = scan.volume_num;

  if (scan.sweep_num == 0) {
    _chillSweepNum0Based = true;
  }
  if (_chillSweepNum0Based) {
    iwrf.sweep_num = scan.sweep_num;
  } else {
    iwrf.sweep_num = scan.sweep_num - 1;
  }

}

////////////////////////////////////////////
// load up IWRF phasecode from CHILL types

void chill_iwrf_phasecode_load(const phasecode_t &code,
                               si64 seq_num,
                               iwrf_phasecode_t &iwrf)

{

  iwrf_phasecode_init(iwrf);
  iwrf.packet.seq_num = seq_num;

  iwrf.seq_length = code.seq_length;

  for (int ii = 0; ii < iwrf.seq_length; ii++) {
    iwrf.phase[ii].phase_deg_h = code.phase[ii].phase_h_d;
    iwrf.phase[ii].phase_deg_v = code.phase[ii].phase_v_d;
  }

}

