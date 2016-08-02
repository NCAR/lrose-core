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
/////////////////////////////////////////////////////////////
// NexradData.cc
//
// Definitions for Nexrad data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/NexradData.hh>
#include <Radx/RadxTime.hh>
#include <Radx/ByteOrder.hh>
#include <cstring>
#include <cstdio>
#include <iostream>
using namespace std;

// const double NexradData::HORIZ_BEAM_WIDTH = 0.95;
// const double NexradData::VERT_BEAM_WIDTH = 0.95;
// const double NexradData::FIXED_ANGLE = 0.005493164063;
// const double NexradData::AZ_RATE = 0.001373291016;
// const double NexradData::NOMINAL_WAVELN = 0.1053;

///////////////////////////////////////////////////////////////
// constructor

NexradData::NexradData()

{

}

///////////////////////////////////////////////////////////////
// destructor

NexradData::~NexradData()

{

}

///////////////////////////////////////////////////////////////
// printing routines

////////////////////////////////////////////////////////////
// Printing

void NexradData::print(const id_rec_t &val, ostream &out)
{
  out << "=============== NEXRAD ID REC ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  filename: " << Radx::makeString(val.filename, 8) << endl;
  out << "=============================================" << endl;
}

void NexradData::print(const vol_number_t &val, ostream &out)
{
  out << "=============== NEXRAD VOL NUMBER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  vol_number: " << val.vol_number << endl;
  out << "=============================================" << endl;
}

void NexradData::print(const vol_title_t &val, ostream &out)
{
  out << "=============== NEXRAD VOL TITLE ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  filetype: " << Radx::makeString(val.filetype, 9) << endl;
  out << "  vol_num: " << Radx::makeString(val.vol_num, 3) << endl;
  out << "  julian_date: " << val.julian_date << endl;
  // out << "  unused1: " << val.unused1 << endl;
  out << "  millisecs_past_midnight: " << val.millisecs_past_midnight << endl;
  printTime(val.julian_date, val.millisecs_past_midnight, out);
  // out << "  unused2: " << val.unused2 << endl;
  out << "================================================" << endl;
}

void NexradData::print(const ctm_info_t &val, ostream &out)
{
  out << "=============== NEXRAD CTM INFO ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  word1: " << val.word1 << endl;
  out << "  word2: " << val.word2 << endl;
  out << "  word3: " << val.word3 << endl;
  out << "  word4: " << val.word4 << endl;
  out << "  word5: " << val.word5 << endl;
  out << "  word6: " << val.word6 << endl;
  out << "===============================================" << endl;
}

void NexradData::print(const msg_hdr_t &val, ostream &out)
{
  out << "=============== NEXRAD MSG HEADER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  message_len: " << val.message_len << endl;
  out << "  channel_id: " << (int) val.channel_id << endl;
  out << "  message_type: " << (int) val.message_type << endl;
  out << "  message_type: " << msgType2Str(val.message_type) << endl;
  out << "  seq_num: " << val.seq_num << endl;
  out << "  julian_date: " << val.julian_date << endl;
  out << "  millisecs_past_midnight: " << val.millisecs_past_midnight << endl;
  printTime(val.julian_date, val.millisecs_past_midnight, out);
  out << "  num_message_segs: " << val.num_message_segs << endl;
  out << "  message_seg_num: " << val.message_seg_num << endl;
  out << "=================================================" << endl;
}

void NexradData::print(const message_1_t &val, ostream &out)
{
  out << "=============== NEXRAD DATA HEADER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  millisecs_past_midnight: " << val.millisecs_past_midnight << endl;
  out << "  julian_date: " << val.julian_date << endl;
  printTime(val.julian_date, val.millisecs_past_midnight, out);
  // out << "  unamb_range_x10: " << val.unamb_range_x10 << endl;
  out << "  unamb_range (km): " << val.unamb_range_x10 / 10.0 << endl;
  out << "  azimuth: " << (val.azimuth / 8.0) * (180.0 / 4096.0) << endl;
  out << "  radial_num: " << val.radial_num << endl;
  out << "  radial_status: " << val.radial_status << endl;
  out << "  elevation: " << (val.elevation / 8.0) * (180.0 / 4096.0) << endl;
  out << "  elev_num: " << val.elev_num << endl;
  out << "  ref_gate1: " << val.ref_gate1 << endl;
  out << "  vel_gate1: " << val.vel_gate1 << endl;
  out << "  ref_gate_width: " << val.ref_gate_width << endl;
  out << "  vel_gate_width: " << val.vel_gate_width << endl;
  out << "  ref_num_gates: " << val.ref_num_gates << endl;
  out << "  vel_num_gates: " << val.vel_num_gates << endl;
  out << "  sector_num: " << val.sector_num << endl;
  out << "  sys_gain_cal_const: " << val.sys_gain_cal_const << endl;
  out << "  ref_ptr: " << val.ref_ptr << endl;
  out << "  vel_ptr: " << val.vel_ptr << endl;
  out << "  sw_ptr: " << val.sw_ptr << endl;
  // out << "  velocity_resolution: " << val.velocity_resolution << endl;
  if (val.velocity_resolution == 2) {
    out << "  velocity_resolution: 0.5" << endl;
  } else {
    out << "  velocity_resolution: 1.0" << endl;
  }
  out << "  vol_coverage_pattern: " << val.vol_coverage_pattern << endl;
  //   out << "  word_23: " << val.word_23 << endl;
  //   out << "  word_24: " << val.word_24 << endl;
  //   out << "  word_25: " << val.word_25 << endl;
  //   out << "  word_26: " << val.word_26 << endl;
  out << "  ref_data_playback: " << val.ref_data_playback << endl;
  out << "  vel_data_playback: " << val.vel_data_playback << endl;
  out << "  sw_data_playback: " << val.sw_data_playback << endl;
  out << "  nyquist_vel (m/s): " << val.nyquist_vel / 100.0 << endl;
  out << "  atmos_atten_factor (dB/km): " << val.atmos_atten_factor / 1000.0 << endl;
  out << "  threshold_param: " << val.threshold_param << endl;
  out << "  spot_blank_status: " << val.spot_blank_status << endl;
  //   out << "  word_34: " << val.word_34 << endl;
  //   out << "  word_35: " << val.word_35 << endl;
  //   out << "  word_36: " << val.word_36 << endl;
  //   out << "  word_37: " << val.word_37 << endl;
  //   out << "  word_38: " << val.word_38 << endl;
  //   out << "  word_39: " << val.word_39 << endl;
  //   out << "  word_40: " << val.word_40 << endl;
  //   out << "  word_41: " << val.word_41 << endl;
  //   out << "  word_42: " << val.word_42 << endl;
  //   out << "  word_43: " << val.word_43 << endl;
  //   out << "  word_44: " << val.word_44 << endl;
  //   out << "  word_45: " << val.word_45 << endl;
  //   out << "  word_46: " << val.word_46 << endl;
  //   out << "  word_47: " << val.word_47 << endl;
  //   out << "  word_48: " << val.word_48 << endl;
  //   out << "  word_49: " << val.word_49 << endl;
  out << "==================================================" << endl;
}

void NexradData::print(const geom_hdr_t &val, ostream &out)
{
  out << "=============== NEXRAD GEOM HEADER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  nazimuths: " << val.nazimuths << endl;
  out << "  ref_ngates: " << val.ref_ngates << endl;
  out << "  vel_ngates: " << val.vel_ngates << endl;
  out << "  ref_resolution: " << val.ref_resolution << endl;
  out << "  vel_resolution: " << val.vel_resolution << endl;
  out << "  elevation: " << val.elevation << endl;
  out << "==================================================" << endl;
}

void NexradData::print(const clutter_hdr_t &val, ostream &out)
{
  out << "=============== NEXRAD CLUTTER HEADER ===============" << endl;
  // out << "  julian_date: " << val.julian_date << endl;
  // out << "  minutes_past_midnight: " << val.minutes_past_midnight << endl;
  // printTime(val.julian_date, val.minutes_past_midnight * 60000, out);
  out << "  num_message_segs: " << val.num_message_segs << endl;
  out << "=====================================================" << endl;
}

void NexradData::print(const VCP_hdr_t &val, ostream &out)
{
  out << "=============== NEXRAD VCP HEADER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  message_len: " << val.message_len << endl;
  out << "  pattern_type: " << val.pattern_type << endl;
  out << "  pattern_number: " << val.pattern_number << endl;
  out << "  num_elevation_cuts: " << val.num_elevation_cuts << endl;
  out << "  clutter_map_group: " << val.clutter_map_group << endl;
  out << "  dop_vel_resolution: " << (int) val.dop_vel_resolution << endl;
  out << "  pulse_width: " << (int) val.pulse_width << endl;
  // out << "  spare1: " << val.spare1 << endl;
  // out << "  spare2: " << val.spare2 << endl;
  // out << "  spare3: " << val.spare3 << endl;
  // out << "  spare4: " << val.spare4 << endl;
  // out << "  spare5: " << val.spare5 << endl;
  out << "=================================================" << endl;
}

void NexradData::print(const ppi_hdr_t &val, ostream &out)
{
  out << "=============== NEXRAD PPI HEADER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  elevation_angle: " << val.elevation_angle << endl;
  out << "  elevation_angle (deg): " << val.elevation_angle * 360.0 / 65536.0 << endl;
  out << "  channel_config: " << (int) val.channel_config << endl;
  out << "  waveform_type: " << (int) val.waveform_type << endl;
  out << "  super_res_control: " << (int) val.super_res_control << endl;
  out << "  surveillance_prf_num: " << (int) val.surveillance_prf_num << endl;
  out << "  surveillance_prf_pulse_count: "
      << val.surveillance_prf_pulse_count << endl;
  out << "  azimuth_rate: " << val.azimuth_rate << endl;
  out << "  azimuth_rate deg/s: " << (val.azimuth_rate * 90 / 65536.0) << endl;
  out << "  reflectivity_thresh (SNR dB): " << val.reflectivity_thresh / 100.0 << endl;
  out << "  velocity_thresh (SNR dB): " << val.velocity_thresh / 100.0 << endl;
  out << "  spectrum_width_thresh (SNR dB): "
      << val.spectrum_width_thresh / 100.0 << endl;
  // out << "  spare1: " << val.spare1 << endl;
  // out << "  spare2: " << val.spare2 << endl;
  // out << "  spare3: " << val.spare3 << endl;
  out << "  edge_angle1: " << (val.edge_angle1 / 8.0) * (180.0 / 4096.0) << endl;
  out << "  doppler_prf_number1: " << val.doppler_prf_number1 << endl;
  out << "  doppler_prf_pulse_count1: " << val.doppler_prf_pulse_count1 << endl;
  // out << "  spare4: " << val.spare4 << endl;
  out << "  edge_angle2: " << (val.edge_angle2 / 8.0) * (180.0 / 4096.0) << endl;
  out << "  doppler_prf_number2: " << val.doppler_prf_number2 << endl;
  out << "  doppler_prf_pulse_count2: " << val.doppler_prf_pulse_count2 << endl;
  // out << "  spare5: " << val.spare5 << endl;
  out << "  edge_angle3: " << (val.edge_angle3 / 8.0) * (180.0 / 4096.0) << endl;
  out << "  doppler_prf_number3: " << val.doppler_prf_number3 << endl;
  out << "  doppler_prf_pulse_count3: " << val.doppler_prf_pulse_count3 << endl;
  // out << "  spare6: " << val.spare6 << endl;
  out << "=================================================" << endl;
}

void NexradData::print(const adaptation_data_t &val,
                       ostream &out)
{

  out << "============== NEXRAD ADAPTATION DATA HEADER ==============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  adap_file_name: "
      << Radx::makeString(val.adap_file_name, 12) << endl;
  out << "  adap_format: " << Radx::makeString(val.adap_format, 4) << endl;
  out << "  adap_revision: " << Radx::makeString(val.adap_revision, 4) << endl;
  out << "  adap_date: " << Radx::makeString(val.adap_date, 12) << endl;
  out << "  adap_time: " << Radx::makeString(val.adap_time, 12) << endl;
  out << "  k1: " << val.k1 << endl;
  out << "  az_lat: " << val.az_lat << endl;
  out << "  k3: " << val.k3 << endl;
  out << "  el_lat: " << val.el_lat << endl;
  out << "  parkaz: " << val.parkaz << endl;
  out << "  parkel: " << val.parkel << endl;
  for (int ii = 0; ii < 11; ii++) {
    out << "  a_fuel_conv[" << ii << "]: " << val.a_fuel_conv[ii] << endl;
  }
  out << "  a_min_shelter_temp: " << val.a_min_shelter_temp << endl;
  out << "  a_max_shelter_temp: " << val.a_max_shelter_temp << endl;
  out << "  a_min_shelter_ac_temp_diff: "
      << val.a_min_shelter_ac_temp_diff << endl;
  out << "  a_max_xmtr_air_temp: " << val.a_max_xmtr_air_temp << endl;
  out << "  a_max_rad_temp: " << val.a_max_rad_temp << endl;
  out << "  a_max_rad_temp_rise: " << val.a_max_rad_temp_rise << endl;
  out << "  ped_28v_reg_lim: " << val.ped_28v_reg_lim << endl;
  out << "  ped_5v_reg_lim: " << val.ped_5v_reg_lim << endl;
  out << "  ped_15v_reg_lim: " << val.ped_15v_reg_lim << endl;
  out << "  a_min_gen_room_temp: " << val.a_min_gen_room_temp << endl;
  out << "  a_max_gen_room_temp: " << val.a_max_gen_room_temp << endl;
  out << "  dau_5v_reg_lim: " << val.dau_5v_reg_lim << endl;
  out << "  dau_15v_reg_lim: " << val.dau_15v_reg_lim << endl;
  out << "  dau_28v_reg_lim: " << val.dau_28v_reg_lim << endl;
  out << "  en_5v_reg_lim: " << val.en_5v_reg_lim << endl;
  out << "  en_5v_nom_volts: " << val.en_5v_nom_volts << endl;
  out << "  rpg_co_located: " << Radx::makeString(val.rpg_co_located, 8) << endl;
  out << "  spec_filter_installed: "
      << Radx::makeString(val.spec_filter_installed, 8) << endl;
  out << "  tps_installed: " << Radx::makeString(val.tps_installed, 8) << endl;
  out << "  rms_installed: " << Radx::makeString(val.rms_installed, 8) << endl;
  out << "  a_hvdl_tst_int: " << val.a_hvdl_tst_int << endl;
  out << "  a_rpg_lt_int: " << val.a_rpg_lt_int << endl;
  out << "  a_min_stab_util_pwr_time: " << val.a_min_stab_util_pwr_time << endl;
  out << "  a_gen_auto_exer_interval: " << val.a_gen_auto_exer_interval << endl;
  out << "  a_util_pwr_sw_req_interval: "
      << val.a_util_pwr_sw_req_interval << endl;
  out << "  a_low_fuel_level: " << val.a_low_fuel_level << endl;
  out << "  config_chan_number: " << val.config_chan_number << endl;
  out << "  a_rpg_link_type: " << val.a_rpg_link_type << endl;
  out << "  redundant_chan_config: " << val.redundant_chan_config << endl;
  for (int ii = 0; ii < 104; ii++) {
    if (val.atten_table[ii] != 0) {
      out << "  atten_table[" << ii << "]: " << val.atten_table[ii] << endl;
    }
  }
  for (int ii = 0; ii < 69; ii++) {
    if (val.path_losses[ii] != 0) {
      out << "  path_losses[" << ii << "]: " << val.path_losses[ii] << endl;
    }
  }
  out << "  chan_cal_diff: " << val.chan_cal_diff << endl;
  out << "  path_losses_70_71: " << val.path_losses_70_71 << endl;
  out << "  log_amp_factor_scale: " << val.log_amp_factor_scale << endl;
  out << "  log_amp_factor_bias: " << val.log_amp_factor_bias << endl;
  out << "  spare_1: " << val.spare_1 << endl;
  for (int ii = 0; ii < 13; ii++) {
    out << "  rnscale[" << ii << "]: " << val.rnscale[ii] << endl;
  }
  for (int ii = 0; ii < 13; ii++) {
    out << "  atmos[" << ii << "]: " << val.atmos[ii] << endl;
  }
  for (int ii = 0; ii < 12; ii++) {
    out << "  el_index[" << ii << "]: " << val.el_index[ii] << endl;
  }
  out << "  tfreq_mhz: " << val.tfreq_mhz << endl;
  out << "  base_data_tcn: " << val.base_data_tcn << endl;
  out << "  refl_data_tover: " << val.refl_data_tover << endl;
  out << "  tar_dbz0_lp: " << val.tar_dbz0_lp << endl;
  out << "  spare_2: " << val.spare_2 << endl;
  out << "  spare_3: " << val.spare_3 << endl;
  out << "  spare_4: " << val.spare_4 << endl;
  out << "  lx_lp: " << val.lx_lp << endl;
  out << "  lx_sp: " << val.lx_sp << endl;
  out << "  meteor_param: " << val.meteor_param << endl;
  out << "  beamwidth: " << val.beamwidth << endl;
  out << "  antenna_gain: " << val.antenna_gain << endl;
  out << "  spare_5: " << val.spare_5 << endl;
  out << "  vel_maint_limit: " << val.vel_maint_limit << endl;
  out << "  wth_maint_limit: " << val.wth_maint_limit << endl;
  out << "  vel_degrad_limit: " << val.vel_degrad_limit << endl;
  out << "  wth_degrad_limit: " << val.wth_degrad_limit << endl;
  out << "  noisetemp_dgrad_limit: " << val.noisetemp_dgrad_limit << endl;
  out << "  noisetemp_maint_limit: " << val.noisetemp_maint_limit << endl;
  out << "  spare_6: " << val.spare_6 << endl;
  out << "  spare_7: " << val.spare_7 << endl;
  out << "  kly_degrade_limit: " << val.kly_degrade_limit << endl;
  out << "  ts_coho: " << val.ts_coho << endl;
  out << "  ts_cw: " << val.ts_cw << endl;
  out << "  ts_rf_sp: " << val.ts_rf_sp << endl;
  out << "  ts_rf_lp: " << val.ts_rf_lp << endl;
  out << "  ts_stalo: " << val.ts_stalo << endl;
  out << "  ts_noise: " << val.ts_noise << endl;
  out << "  xmtr_peak_pwr_high_limit: " << val.xmtr_peak_pwr_high_limit << endl;
  out << "  xmtr_peak_pwr_low_limit: " << val.xmtr_peak_pwr_low_limit << endl;
  out << "  dbz0_delta_limit: " << val.dbz0_delta_limit << endl;
  out << "  threshold1: " << val.threshold1 << endl;
  out << "  threshold2: " << val.threshold2 << endl;
  out << "  clut_supp_dgrad_lim: " << val.clut_supp_dgrad_lim << endl;
  out << "  clut_supp_maint_lim: " << val.clut_supp_maint_lim << endl;
  out << "  range0_value: " << val.range0_value << endl;
  out << "  xmtr_pwr_mtr_scale: " << val.xmtr_pwr_mtr_scale << endl;
  out << "  n_smooth: " << val.n_smooth << endl;
  out << "  tar_dbz0_sp: " << val.tar_dbz0_sp << endl;
  out << "  spare_8: " << val.spare_8 << endl;
  out << "  deltaprf: " << val.deltaprf << endl;
  out << "  spare_9: " << val.spare_9 << endl;
  out << "  spare_10: " << val.spare_10 << endl;
  out << "  tau_sp: " << val.tau_sp << endl;
  out << "  tau_lp: " << val.tau_lp << endl;
  out << "  nc_dead_value: " << val.nc_dead_value << endl;
  out << "  tau_rf_sp: " << val.tau_rf_sp << endl;
  out << "  tau_rf_lp: " << val.tau_rf_lp << endl;
  out << "  seg1lim: " << val.seg1lim << endl;
  out << "  slatsec: " << val.slatsec << endl;
  out << "  slonsec: " << val.slonsec << endl;
  out << "  spare_11: " << val.spare_11 << endl;
  out << "  slatdeg: " << val.slatdeg << endl;
  out << "  slatmin: " << val.slatmin << endl;
  out << "  slondeg: " << val.slondeg << endl;
  out << "  slonmin: " << val.slonmin << endl;
  out << "  slatdir: " << Radx::makeString(val.slatdir, 4) << endl;
  out << "  slondir: " << Radx::makeString(val.slondir, 4) << endl;
  out << "  spare_12: " << val.spare_12 << endl;
  for (int ii = 0; ii < 293; ii++) {
    if (val.vcpat11[ii] != 0) {
      out << "  vcpat11[" << ii << "]: " << val.vcpat11[ii] << endl;
    }
  }
  for (int ii = 0; ii < 293; ii++) {
    if (val.vcpat21[ii] != 0) {
      out << "  vcpat21[" << ii << "]: " << val.vcpat21[ii] << endl;
    }
  }
  for (int ii = 0; ii < 293; ii++) {
    if (val.vcpat31[ii] != 0) {
      out << "  vcpat31[" << ii << "]: " << val.vcpat31[ii] << endl;
    }
  }
  for (int ii = 0; ii < 293; ii++) {
    if (val.vcpat32[ii] != 0) {
      out << "  vcpat32[" << ii << "]: " << val.vcpat32[ii] << endl;
    }
  }
  for (int ii = 0; ii < 293; ii++) {
    if (val.vcpat300[ii] != 0) {
      out << "  vcpat300[" << ii << "]: " << val.vcpat300[ii] << endl;
    }
  }
  for (int ii = 0; ii < 293; ii++) {
    if (val.vcpat301[ii] != 0) {
      out << "  vcpat301[" << ii << "]: " << val.vcpat301[ii] << endl;
    }
  }
  out << "  az_correction_factor: " << val.az_correction_factor << endl;
  out << "  el_correction_factor: " << val.el_correction_factor << endl;
  out << "  site_name: " << Radx::makeString(val.site_name, 4) << endl;
  out << "  ant_manual_setup_ielmin: " << val.ant_manual_setup_ielmin << endl;
  out << "  ant_manual_setup_ielmax: " << val.ant_manual_setup_ielmax << endl;
  out << "  ant_manual_setup_fazvelmax: "
      << val.ant_manual_setup_fazvelmax << endl;
  out << "  ant_manual_setup_felvelmax: "
      << val.ant_manual_setup_felvelmax << endl;
  out << "  ant_manual_setup_ignd_hgt: " << val.ant_manual_setup_ignd_hgt << endl;
  out << "  ant_manual_setup_irad_hgt: " << val.ant_manual_setup_irad_hgt << endl;
  out << "  RVP8NV_iwaveguide_length: " << val.RVP8NV_iwaveguide_length << endl;
  out << "  vel_data_tover: " << val.vel_data_tover << endl;
  out << "  width_data_tover: " << val.width_data_tover << endl;
  out << "  doppler_range_start: " << val.doppler_range_start << endl;
  out << "  max_el_index: " << val.max_el_index << endl;
  out << "  seg2lim: " << val.seg2lim << endl;
  out << "  seg3lim: " << val.seg3lim << endl;
  out << "  seg4lim: " << val.seg4lim << endl;
  out << "  nbr_el_segments: " << val.nbr_el_segments << endl;
  out << "  noise_long: " << val.noise_long << endl;
  out << "  ant_noise_temp: " << val.ant_noise_temp << endl;
  out << "  noise_short: " << val.noise_short << endl;
  out << "  noise_tolerance: " << val.noise_tolerance << endl;
  out << "  min_dyn_range: " << val.min_dyn_range << endl;
  out << "===========================================================" << endl;

}

void NexradData::print(const message_31_hdr_t &val, ostream &out)
{
  out << "=============== NEXRAD DATA 31 HEADER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  radar_icao: " << Radx::makeString(val.radar_icao, 4) << endl;
  out << "  millisecs_past_midnight: " << val.millisecs_past_midnight << endl;
  out << "  julian_date: " << val.julian_date << endl;
  printTime(val.julian_date, val.millisecs_past_midnight, out);
  out << "  radial_num: " << val.radial_num << endl;
  out << "  azimuth: " << val.azimuth << endl;
  out << "  compression: " << (int) val.compression << endl;
  out << "  spare_1: " << (int) val.spare_1 << endl;
  out << "  radial_length: " << val.radial_length << endl;
  out << "  azimuth_spacing: " << (int) val.azimuth_spacing << endl;
  out << "  radial_status: " << (int) val.radial_status << endl;
  out << "  elev_num: " << (int) val.elev_num << endl;
  out << "  sector_num: " << (int) val.sector_num << endl;
  out << "  elevation: " << val.elevation << endl;
  out << "  spot_blank: " << (int) val.spot_blank << endl;
  out << "  azimuth_index: " << (int) val.azimuth_index << endl;
  out << "  n_data_blocks: " << val.n_data_blocks << endl;
  for (int ii = 0; ii < MAX_DATA_BLOCKS; ii++) {
    out << "  data_block_offset[" << ii << "]: " << val.data_block_offset[ii] << endl;
  }
  out << "=====================================================" << endl;
}

void NexradData::print(const message_31_vol_t &val, ostream &out)
{
  out << "=============== NEXRAD VOLUME 31 HEADER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  block_type: " << val.block_type << endl;
  out << "  block_name: " << Radx::makeString(val.block_name, 3) << endl;
  out << "  block_size: " << val.block_size << endl;
  out << "  ver_major: " << (int) val.ver_major << endl;
  out << "  ver_minor: " << (int) val.ver_minor << endl;
  out << "  lat: " << val.lat << endl;
  out << "  lon: " << val.lon << endl;
  out << "  height: " << val.height << endl;
  out << "  feedhorn_height: " << val.feedhorn_height << endl;
  out << "  dbz0: " << val.dbz0 << endl;
  out << "  horiz_power: " << val.horiz_power << endl;
  out << "  vert_power: " << val.vert_power << endl;
  out << "  system_zdr: " << val.system_zdr << endl;
  out << "  system_phi: " << val.system_phi << endl;
  out << "  vol_coverage_pattern: " << val.vol_coverage_pattern << endl;
  // out << "  spare_1: " << val.spare_1 << endl;
  out << "=======================================================" << endl;
}

void NexradData::print(const message_31_elev_t &val, ostream &out)
{
  out << "=============== NEXRAD ELEV 31 HEADER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  block_type: " << val.block_type << endl;
  out << "  block_name: " << Radx::makeString(val.block_name, 3) << endl;
  out << "  block_size: " << val.block_size << endl;
  out << "  atmos: " << val.atmos << endl;
  out << "  dbz0: " << val.dbz0 << endl;
  out << "=====================================================" << endl;
}

void NexradData::print(const message_31_radial_t &val, ostream &out)
{
  out << "=============== NEXRAD RADIAL 31 HEADER ===============" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  block_type: " << val.block_type << endl;
  out << "  block_name: " << Radx::makeString(val.block_name, 3) << endl;
  out << "  block_size: " << val.block_size << endl;
  // out << "  unamb_range_x10: " << val.unamb_range_x10 << endl;
  out << "  unamb_range (km): " << val.unamb_range_x10 / 10.0 << endl;
  out << "  horiz_noise: " << val.horiz_noise << endl;
  out << "  vert_noise: " << val.vert_noise << endl;
  out << "  nyquist_vel (m/s): " << val.nyquist_vel / 100.0 << endl;
  // out << "  spare_1: " << val.spare_1 << endl;
  out << "=======================================================" << endl;
}

void NexradData::print(const message_31_field_t &val, ostream &out)
{
  out << "============ NEXRAD DATA 31 FIELD HEADER ===========" << endl;
  out << "  size in bytes: " << sizeof(val) << endl;
  out << "  block_type: " << val.block_type << endl;
  out << "  block_name: " << Radx::makeString(val.block_name, 3) << endl;
  // out << "  reserved_1: " << val.reserved_1 << endl;
  out << "  num_gates: " << val.num_gates << endl;
  out << "  gate1: " << val.gate1 << endl;
  out << "  gate_width: " << val.gate_width << endl;
  out << "  tover: " << val.tover << endl;
  out << "  snr_threshold: " << val.snr_threshold << endl;
  out << "  control_flags: " << (int) val.control_flags << endl;
  out << "  data_size: " << (int) val.data_size << endl;
  out << "  scale: " << val.scale << endl;
  out << "  offset: " << val.offset << endl;
  out << "===================================================" << endl;
}

void NexradData::printTime(int julianDate, int msecsInDay, ostream &out)
{
  int secs = msecsInDay / 1000;
  int msecs = msecsInDay - secs * 1000;
  time_t utime = (julianDate-1) * 86400 + secs;
  RadxTime rtime(utime);
  char msecsStr[16];
  sprintf(msecsStr, "%.3d", msecs);
  out << "  time: " << RadxTime::strm(utime) << "." << msecsStr << endl;
}

////////////////////////////////////////////////////////////
// Byte swapping

void NexradData::swap(vol_number_t &val)
{
  ByteOrder::swap16(&val.vol_number, 2);
}

void NexradData::swap(vol_title_t &val)
{
  ByteOrder::swap16(&val.julian_date, 4);
  ByteOrder::swap32(&val.millisecs_past_midnight, 8);
}

void NexradData::swap(ctm_info_t &val)
{
  ByteOrder::swap16(&val.word1, 12);
}

void NexradData::swap(msg_hdr_t &val)
{
  ByteOrder::swap16(&val.message_len, 2);
  ByteOrder::swap16(&val.seq_num, 4);
  ByteOrder::swap32(&val.millisecs_past_midnight, 4);
  ByteOrder::swap16(&val.num_message_segs, 4);
}

void NexradData::swap(message_1_t &val)
{
  ByteOrder::swap32(&val.millisecs_past_midnight, 4);
  ByteOrder::swap16(&val.julian_date, 28);
  ByteOrder::swap32(&val.sys_gain_cal_const, 4);
  ByteOrder::swap16(&val.ref_ptr, 64);
}

void NexradData::swap(geom_hdr_t &val)
{
  ByteOrder::swap32(&val.nazimuths, 24);
}
  
void NexradData::swap(clutter_hdr_t &val)
{
  ByteOrder::swap16(&val.julian_date, 6);
}
  
void NexradData::swap(VCP_hdr_t &val)
{
  ByteOrder::swap16(&val.message_len, 10);
  ByteOrder::swap16(&val.spare1, 10);
}
  
void NexradData::swap(ppi_hdr_t &val)
{
  ByteOrder::swap16(&val.elevation_angle, 2);
  ByteOrder::swap16(&val.surveillance_prf_pulse_count, 40);
}
  
void NexradData::swap(adaptation_data_t &val)
{
  ByteOrder::swap32(&val.k1, 132);
  ByteOrder::swap32(&val.a_hvdl_tst_int, 1124);
  ByteOrder::swap32(&val.spare_12, 7044);
  ByteOrder::swap32(&val.ant_manual_setup_ielmin, 436);
}
  
void NexradData::swap(message_31_hdr_t &val)
{
  ByteOrder::swap32(&val.millisecs_past_midnight, 4);
  ByteOrder::swap16(&val.julian_date, 4);
  ByteOrder::swap32(&val.azimuth, 4);
  ByteOrder::swap16(&val.radial_length, 2);
  ByteOrder::swap32(&val.elevation, 4);
  ByteOrder::swap16(&val.n_data_blocks, 2);
  ByteOrder::swap32(val.data_block_offset, 4 * MAX_DATA_BLOCKS);
}
  
void NexradData::swap(message_31_vol_t &val)
{
  ByteOrder::swap16(&val.block_size, 2);
  ByteOrder::swap32(&val.lat, 8);
  ByteOrder::swap16(&val.height, 4);
  ByteOrder::swap32(&val.dbz0, 20);
  ByteOrder::swap16(&val.vol_coverage_pattern, 4);
}
  
void NexradData::swap(message_31_elev_t &val)
{
  ByteOrder::swap16(&val.block_size, 4);
  ByteOrder::swap32(&val.dbz0, 4);
}
  
void NexradData::swap(message_31_radial_t &val)
{
  ByteOrder::swap16(&val.block_size, 4);
  ByteOrder::swap32(&val.horiz_noise, 8);
  ByteOrder::swap16(&val.nyquist_vel, 4);
}
  
void NexradData::swap(message_31_field_t &val)
{
  ByteOrder::swap32(&val.reserved_1, 4);
  ByteOrder::swap16(&val.num_gates, 10);
  ByteOrder::swap32(&val.scale, 8);
}

///////////////////////////////////
string NexradData::msgType2Str(int msgType)

{
 
  switch (msgType) {
    case UNKNOWN_DATA:
      return "UNKNOWN";
    case DIGITAL_RADAR_DATA_1:
      return "DIGITAL_RADAR_DATA_1";
    case RDA_STATUS_DATA:
      return "RDA_STATUS_DATA";
    case PERFORMANCE_MAINTENANCE_DATA:
      return "PERFORMANCE_MAINTENANCE_DATA";
    case CONSOLE_MESSAGE_A2G:
      return "CONSOLE_MESSAGE_A2G";
    case VOLUME_COVERAGE_PATTERN:
      return "VOLUME_COVERAGE_PATTERN";
    case RDA_CONTROL_COMMANDS:
      return "RDA_CONTROL_COMMANDS";
    case VOLUME_COVERAGE_PATTERN2:
      return "VOLUME_COVERAGE_PATTERN2";
    case CLUTTER_SENSOR_ZONES:
      return "CLUTTER_SENSOR_ZONES";
    case REQUEST_FOR_DATA:
      return "REQUEST_FOR_DATA";
    case CONSOLE_MESSAGE_G2A:
      return "CONSOLE_MESSAGE_G2A";
    case LOOPBACK_TEST_RDA_RPG:
      return "LOOPBACK_TEST_RDA_RPG";
    case LOOPBACK_TEST_RGD_RPA:
      return "LOOPBACK_TEST_RGD_RPA";
    case CLUTTER_FILTER_BYPASS_MAP:
      return "CLUTTER_FILTER_BYPASS_MAP";
    case EDITED_CLUTTER_FILTER_MAP:
      return "EDITED_CLUTTER_FILTER_MAP";
    case CLUTTER_FILTER_MAP:
      return "CLUTTER_FILTER_MAP";
    case RDA_ADAPTATION_DATA:
      return "RDA_ADAPTATION_DATA";
    case DIGITAL_RADAR_DATA_31:
      return "DIGITAL_RADAR_DATA_31";
    default:
      return "ILLEGAL_DATA";
  }

}

/////////////////////////////////////////////
//
// Returns true if this is a valid message type,
// false otherwise

bool NexradData::msgTypeIsValid(int msgType)

{
 
  switch (msgType) {
    case UNKNOWN_DATA:
    case DIGITAL_RADAR_DATA_1:
    case RDA_STATUS_DATA:
    case PERFORMANCE_MAINTENANCE_DATA:
    case CONSOLE_MESSAGE_A2G:
    case VOLUME_COVERAGE_PATTERN:
    case RDA_CONTROL_COMMANDS:
    case VOLUME_COVERAGE_PATTERN2:
    case CLUTTER_SENSOR_ZONES:
    case REQUEST_FOR_DATA:
    case CONSOLE_MESSAGE_G2A:
    case LOOPBACK_TEST_RDA_RPG:
    case LOOPBACK_TEST_RGD_RPA:
    case CLUTTER_FILTER_BYPASS_MAP:
    case EDITED_CLUTTER_FILTER_MAP:
    case CLUTTER_FILTER_MAP:
    case RDA_ADAPTATION_DATA:
    case DIGITAL_RADAR_DATA_31:
      return true;
    default:
      return false;
  }

}


