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
// MomentsFields.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#include <radar/MomentsFields.hh>
#include <cmath>
#include <cstring>

const double MomentsFields::missingDouble = -9999;

// constructor

MomentsFields::MomentsFields()

{

  init();

}

// Initialize to missing

void MomentsFields::init()

{
  
  // reflectivity
  
  dbz = missingDouble;
  dbz_no_atmos_atten = missingDouble;
  dbzhc = missingDouble;
  dbzhx = missingDouble;
  dbzvc = missingDouble;
  dbzvx = missingDouble;

  // alternating mode velocity

  vel = missingDouble;
  vel_alt = missingDouble;
  vel_hv = missingDouble;
  vel_h_only = missingDouble;
  vel_v_only = missingDouble;
  vel_alt_fold_interval = missingDouble;
  vel_alt_fold_confidence = missingDouble;
  vel_corr_vert = missingDouble;
  vel_corr_motion = missingDouble;

  // staggered PRT

  vel_prt_short = missingDouble;
  vel_prt_long = missingDouble;
  vel_diff = missingDouble;
  vel_unfold_interval = missingDouble;
                                 
  // spectrum width

  width = missingDouble;
  width_r0r1 = missingDouble;
  width_r1r2 = missingDouble;
  width_r1r3 = missingDouble;
  width_ppls = missingDouble;
  width_h_only = missingDouble;
  width_v_only = missingDouble;
  width_prt_long = missingDouble;
  width_prt_short = missingDouble;
  width_corr_motion = missingDouble;

  // normalized coherent power
  // also referred to as SQI - signal quality index
  
  ncp = missingDouble;
  ncp_h_only = missingDouble;
  ncp_v_only = missingDouble;
  ncp_h_minus_v = missingDouble;

  ncp_trip1 = missingDouble;
  ncp_trip2 = missingDouble;
  ncp_trip3 = missingDouble;
  ncp_trip4 = missingDouble;
  
  ncp_prt_long = missingDouble;
  ncp_prt_short = missingDouble;
  ncp_trip_flag = missingDouble;

  // signal to noise

  snr = missingDouble;
  snrhc = missingDouble;
  snrhx = missingDouble;
  snrvc = missingDouble;
  snrvx = missingDouble;

 // uncalibrated power

  dbm = missingDouble;
  dbmhc = missingDouble;
  dbmhx = missingDouble;
  dbmvc = missingDouble;
  dbmvx = missingDouble;

  // noise-subtracted uncalibrated power
  
  dbmhc_ns = missingDouble;
  dbmhx_ns = missingDouble;
  dbmvc_ns = missingDouble;
  dbmvx_ns = missingDouble;

  // dual polarization fields

  zdrm = missingDouble;
  zdr = missingDouble;
  zdr_bias = missingDouble;
  
  ldr = missingDouble;
  ldrhm = missingDouble;
  ldrh = missingDouble;
  ldrvm = missingDouble;
  ldrv = missingDouble;
  ldr_diff = missingDouble;
  ldr_mean = missingDouble;

  rhohv = missingDouble;
  rhohv_nnc = missingDouble;
  phidp0 = missingDouble;
  phidp = missingDouble;
  phidp_cond = missingDouble;
  phidp_filt = missingDouble;
  phidp_sdev_4kdp = missingDouble;
  phidp_jitter_4kdp = missingDouble;
  zdr_sdev_4kdp = missingDouble;
  kdp = missingDouble;
  psob = missingDouble;
  kdp_hb = missingDouble;

  // co-cross correlations

  rho_vchx = missingDouble;
  rho_hcvx = missingDouble;
  rho_vxhx = missingDouble;
  rho_phidp = missingDouble;
  
  // cross polar ratio - CPR

  cpr_mag = missingDouble;
  cpr_phase = missingDouble;
  cpr_ldr = missingDouble;

  // CMD - Clutter Mitigation Decision

  cpa = missingDouble;
  pratio = missingDouble;
  mvar = missingDouble;
  tss = missingDouble;
  tpt = missingDouble;
  cpd = missingDouble;
  tclut = missingDouble;
  ozsnr = missingDouble;
  
  tdbz = missingDouble;
  spin = missingDouble;
  max_tdbz_spin = missingDouble;

  zdr_sdev = missingDouble;
  phidp_sdev = missingDouble;

  dbz_diff_sq = missingDouble;
  dbz_spin_change = missingDouble;
  
  cmd = missingDouble;
  cmd_flag = missingDouble;
  
  tdbz_interest = missingDouble;
  spin_interest = missingDouble;
  cpa_interest = missingDouble;
  zdr_sdev_interest = missingDouble;
  phidp_sdev_interest = missingDouble;

  // clutter power and noise residue

  clut = missingDouble;
  clut_2_wx_ratio = missingDouble;
  spectral_noise = missingDouble;
  spectral_snr = missingDouble;
  regr_filt_poly_order = missingDouble;
  regr_filt_cnr_db = missingDouble;

  // refractivity fields

  aiq_hc = missingDouble;
  niq_hc = missingDouble;
  aiq_vc = missingDouble;
  niq_vc = missingDouble;

  // SZ8-64 phase coding
  
  sz_trip_flag = missingDouble;
  sz_leakage = missingDouble;
  
  // censoring flag

  censoring_flag = missingDouble;

  // covariances linear

  lag0_hc = missingDouble;
  lag0_hx = missingDouble;
  lag0_vc = missingDouble;
  lag0_vx = missingDouble;

  lag0_vchx.clear();
  lag0_hcvx.clear();
  lag1_hc.clear();
  lag1_vc.clear();
  lag1_hcvc.clear();
  lag1_vchc.clear();
  lag1_vxhx.clear();
  lag2_hc.clear();
  lag2_vc.clear();
  lag3_hc.clear();
  lag3_vc.clear();
  rvvhh0.clear();

  // covariances staggered

  lag0_hc_short = missingDouble;
  lag0_vc_short = missingDouble;
  lag0_hc_long = missingDouble;
  lag0_vc_long = missingDouble;

  lag1_hc_long.clear();
  lag1_vc_long.clear();
  lag1_hc_short.clear();
  lag1_vc_short.clear();

  lag1_hc_short_to_long.clear();
  lag1_vc_short_to_long.clear();
  lag1_hc_long_to_short.clear();
  lag1_vc_long_to_short.clear();

  rvvhh0_long.clear();
  rvvhh0_short.clear();

  // covariances as log power and phase

  lag0_hc_db = missingDouble;
  lag0_hx_db = missingDouble;
  lag0_vc_db = missingDouble;
  lag0_vx_db = missingDouble;

  lag0_vchx_db = missingDouble;
  lag0_vchx_phase = missingDouble;

  lag0_hcvx_db = missingDouble;
  lag0_hcvx_phase = missingDouble;

  lag1_hc_db = missingDouble;
  lag1_hc_phase = missingDouble;

  lag1_vc_db = missingDouble;
  lag1_vc_phase = missingDouble;
  
  lag1_hcvc_db = missingDouble;
  lag1_hcvc_phase = missingDouble;

  lag1_vchc_db = missingDouble;
  lag1_vchc_phase = missingDouble;

  lag1_vxhx_db = missingDouble;
  lag1_vxhx_phase = missingDouble;

  lag2_hc_db = missingDouble;
  lag2_hc_phase = missingDouble;

  lag2_vc_db = missingDouble;
  lag2_vc_phase = missingDouble;

  lag3_hc_db = missingDouble;
  lag3_hc_phase = missingDouble;

  lag3_vc_db = missingDouble;
  lag3_vc_phase = missingDouble;

  rvvhh0_db = missingDouble;
  rvvhh0_phase = missingDouble;

  sdev_vv = missingDouble;
  
  prt = missingDouble;
  num_pulses = missingDouble;

  prt_short = missingDouble;
  prt_long = missingDouble;

  // staggered lag1 covariances as log power and phase

  lag0_hc_short_db = missingDouble;
  lag0_vc_short_db = missingDouble;
  lag0_hc_long_db = missingDouble;
  lag0_vc_long_db = missingDouble;

  lag1_hc_short_db = missingDouble;
  lag1_hc_short_phase = missingDouble;
  lag1_vc_short_db = missingDouble;
  lag1_vc_short_phase = missingDouble;

  lag1_hc_long_db = missingDouble;
  lag1_hc_long_phase = missingDouble;
  lag1_vc_long_db = missingDouble;
  lag1_vc_long_phase = missingDouble;

  lag1_hc_short_to_long_db = missingDouble;
  lag1_hc_short_to_long_phase = missingDouble;
  lag1_vc_short_to_long_db = missingDouble;
  lag1_vc_short_to_long_phase = missingDouble;

  lag1_hc_long_to_short_db = missingDouble;
  lag1_hc_long_to_short_phase = missingDouble;
  lag1_vc_long_to_short_db = missingDouble;
  lag1_vc_long_to_short_phase = missingDouble;

  rvvhh0_long_db = missingDouble;
  rvvhh0_long_phase = missingDouble;
  rvvhh0_short_db = missingDouble;
  rvvhh0_short_phase = missingDouble;

  // identifying noise

  dbm_for_noise = missingDouble;
  dbm_sdev = missingDouble;
  ncp_mean = missingDouble;
  phase_for_noise.clear();
  accum_phase_change = 0.0;
  phase_change_error = 0.0;
  
  // flag to indicate noise/signal is present at a gate

  noise_flag = missingDouble;
  noise_interest = missingDouble;
  signal_flag = missingDouble;
  signal_interest = missingDouble;

  // noise_bias relative to calibrated noise
  // these will be constant for all gates in a ray
  
  noise_bias_db_hc = missingDouble;
  noise_bias_db_hx = missingDouble;
  noise_bias_db_vc = missingDouble;
  noise_bias_db_vx = missingDouble;

  // attenuation correction for refl and zdr

  dbz_atten_correction = missingDouble;
  zdr_atten_correction = missingDouble;
  dbz_atten_corrected = missingDouble;
  zdr_atten_corrected = missingDouble;

  // fields for testing

  test0 = missingDouble;
  test1 = missingDouble;
  test2 = missingDouble;
  test3 = missingDouble;
  test4 = missingDouble;
  test5 = missingDouble;
  test6 = missingDouble;
  test7 = missingDouble;
  test8 = missingDouble;
  test9 = missingDouble;
  
}

// Initialize to zero

void MomentsFields::initToZero()

{
  
  // reflectivity
  
  dbz = 0.0;
  dbz_no_atmos_atten = 0.0;
  dbzhc = 0.0;
  dbzhx = 0.0;
  dbzvc = 0.0;
  dbzvx = 0.0;

  // alternating mode velocity

  vel = 0.0;
  vel_alt = 0.0;
  vel_hv = 0.0;
  vel_h_only = 0.0;
  vel_v_only = 0.0;
  vel_alt_fold_interval = 0.0;
  vel_alt_fold_confidence = 0.0;
  vel_corr_vert = 0.0;
  vel_corr_motion = 0.0;

  // staggered PRT

  vel_prt_short = 0.0;
  vel_prt_long = 0.0;
  vel_diff = 0.0;
  vel_unfold_interval = 0.0;
                                 
  // spectrum width

  width = 0.0;
  width_r0r1 = 0.0;
  width_r1r2 = 0.0;
  width_r1r3 = 0.0;
  width_ppls = 0.0;
  width_h_only = 0.0;
  width_v_only = 0.0;
  width_prt_long = 0.0;
  width_prt_short = 0.0;
  width_corr_motion = 0.0;

  // normalized coherent power
  // also referred to as SQI - signal quality index
  
  ncp = 0.0;
  ncp_h_only = 0.0;
  ncp_v_only = 0.0;
  ncp_h_minus_v = 0.0;

  ncp_trip1 = 0.0;
  ncp_trip2 = 0.0;
  ncp_trip3 = 0.0;
  ncp_trip4 = 0.0;
  
  ncp_prt_long = 0.0;
  ncp_prt_short = 0.0;
  ncp_trip_flag = 0.0;

  // signal to noise

  snr = 0.0;
  snrhc = 0.0;
  snrhx = 0.0;
  snrvc = 0.0;
  snrvx = 0.0;

 // uncalibrated power

  dbm = 0.0;
  dbmhc = 0.0;
  dbmhx = 0.0;
  dbmvc = 0.0;
  dbmvx = 0.0;

  // noise-subtracted uncalibrated power
  
  dbmhc_ns = 0.0;
  dbmhx_ns = 0.0;
  dbmvc_ns = 0.0;
  dbmvx_ns = 0.0;

  // dual polarization fields

  zdrm = 0.0;
  zdr = 0.0;
  zdr_bias = 0.0;
  
  ldr = 0.0;
  ldrhm = 0.0;
  ldrh = 0.0;
  ldrvm = 0.0;
  ldrv = 0.0;
  ldr_diff = 0.0;
  ldr_mean = 0.0;

  rhohv = 0.0;
  rhohv_nnc = 0.0;
  phidp0 = 0.0;
  phidp = 0.0;
  phidp_cond = 0.0;
  phidp_filt = 0.0;
  phidp_sdev_4kdp = 0.0;
  phidp_jitter_4kdp = 0.0;
  zdr_sdev_4kdp = 0.0;
  kdp = 0.0;
  psob = 0.0;
  kdp_hb = 0.0;

  // co-cross correlations

  rho_vchx = 0.0;
  rho_hcvx = 0.0;
  rho_vxhx = 0.0;
  rho_phidp = 0.0;
  
  // cross polar ratio - CPR

  cpr_mag = 0.0;
  cpr_phase = 0.0;
  cpr_ldr = 0.0;

  // CMD - Clutter Mitigation Decision

  cpa = 0.0;
  pratio = 0.0;
  mvar = 0.0;
  tss = 0.0;
  tpt = 0.0;
  cpd = 0.0;
  tclut = 0.0;
  ozsnr = 0.0;
  
  tdbz = 0.0;
  spin = 0.0;
  max_tdbz_spin = 0.0;

  zdr_sdev = 0.0;
  phidp_sdev = 0.0;

  dbz_diff_sq = 0.0;
  dbz_spin_change = 0.0;
  
  cmd = 0.0;
  cmd_flag = 0.0;
  
  tdbz_interest = 0.0;
  spin_interest = 0.0;
  cpa_interest = 0.0;
  zdr_sdev_interest = 0.0;
  phidp_sdev_interest = 0.0;

  // clutter power and noise residue

  clut = 0.0;
  clut_2_wx_ratio = 0.0;
  spectral_noise = 0.0;
  spectral_snr = 0.0;
  regr_filt_poly_order = 0.0;
  regr_filt_cnr_db = 0.0;

  // refractivity fields

  aiq_hc = 0.0;
  niq_hc = 0.0;
  aiq_vc = 0.0;
  niq_vc = 0.0;

  // SZ8-64 phase coding
  
  sz_trip_flag = 0.0;
  sz_leakage = 0.0;
  
  // censoring flag

  censoring_flag = 0.0;

  // covariances linear

  lag0_hc = 0.0;
  lag0_hx = 0.0;
  lag0_vc = 0.0;
  lag0_vx = 0.0;

  lag0_vchx.clear();
  lag0_hcvx.clear();
  lag1_hc.clear();
  lag1_vc.clear();
  lag1_hcvc.clear();
  lag1_vchc.clear();
  lag1_vxhx.clear();
  lag2_hc.clear();
  lag2_vc.clear();
  lag3_hc.clear();
  lag3_vc.clear();
  rvvhh0.clear();

  // covariances staggered

  lag0_hc_short = 0.0;
  lag0_vc_short = 0.0;
  lag0_hc_long = 0.0;
  lag0_vc_long = 0.0;

  lag1_hc_long.clear();
  lag1_vc_long.clear();
  lag1_hc_short.clear();
  lag1_vc_short.clear();

  lag1_hc_short_to_long.clear();
  lag1_vc_short_to_long.clear();
  lag1_hc_long_to_short.clear();
  lag1_vc_long_to_short.clear();

  rvvhh0_long.clear();
  rvvhh0_short.clear();

  // covariances as log power and phase

  lag0_hc_db = 0.0;
  lag0_hx_db = 0.0;
  lag0_vc_db = 0.0;
  lag0_vx_db = 0.0;

  lag0_vchx_db = 0.0;
  lag0_vchx_phase = 0.0;

  lag0_hcvx_db = 0.0;
  lag0_hcvx_phase = 0.0;

  lag1_hc_db = 0.0;
  lag1_hc_phase = 0.0;

  lag1_vc_db = 0.0;
  lag1_vc_phase = 0.0;
  
  lag1_hcvc_db = 0.0;
  lag1_hcvc_phase = 0.0;

  lag1_vchc_db = 0.0;
  lag1_vchc_phase = 0.0;

  lag1_vxhx_db = 0.0;
  lag1_vxhx_phase = 0.0;

  lag2_hc_db = 0.0;
  lag2_hc_phase = 0.0;

  lag2_vc_db = 0.0;
  lag2_vc_phase = 0.0;

  lag3_hc_db = 0.0;
  lag3_hc_phase = 0.0;

  lag3_vc_db = 0.0;
  lag3_vc_phase = 0.0;

  rvvhh0_db = 0.0;
  rvvhh0_phase = 0.0;

  sdev_vv = 0.0;
  
  prt = 0.0;
  num_pulses = 0.0;

  prt_short = 0.0;
  prt_long = 0.0;

  // staggered lag1 covariances as log power and phase

  lag0_hc_short_db = 0.0;
  lag0_vc_short_db = 0.0;
  lag0_hc_long_db = 0.0;
  lag0_vc_long_db = 0.0;

  lag1_hc_short_db = 0.0;
  lag1_hc_short_phase = 0.0;
  lag1_vc_short_db = 0.0;
  lag1_vc_short_phase = 0.0;

  lag1_hc_long_db = 0.0;
  lag1_hc_long_phase = 0.0;
  lag1_vc_long_db = 0.0;
  lag1_vc_long_phase = 0.0;

  lag1_hc_short_to_long_db = 0.0;
  lag1_hc_short_to_long_phase = 0.0;
  lag1_vc_short_to_long_db = 0.0;
  lag1_vc_short_to_long_phase = 0.0;

  lag1_hc_long_to_short_db = 0.0;
  lag1_hc_long_to_short_phase = 0.0;
  lag1_vc_long_to_short_db = 0.0;
  lag1_vc_long_to_short_phase = 0.0;

  rvvhh0_long_db = 0.0;
  rvvhh0_long_phase = 0.0;
  rvvhh0_short_db = 0.0;
  rvvhh0_short_phase = 0.0;

  // identifying noise

  dbm_for_noise = 0.0;
  dbm_sdev = 0.0;
  ncp_mean = 0.0;
  phase_for_noise.clear();
  accum_phase_change = 0.0;
  phase_change_error = 0.0;
  
  // flag to indicate noise/signal is present at a gate

  noise_flag = 0.0;
  noise_interest = 0.0;
  signal_flag = 0.0;
  signal_interest = 0.0;

  // noise_bias relative to calibrated noise
  // these will be constant for all gates in a ray
  
  noise_bias_db_hc = 0.0;
  noise_bias_db_hx = 0.0;
  noise_bias_db_vc = 0.0;
  noise_bias_db_vx = 0.0;

  // attenuation correction for refl and zdr

  dbz_atten_correction = 0.0;
  zdr_atten_correction = 0.0;
  dbz_atten_corrected = 0.0;
  zdr_atten_corrected = 0.0;

  // fields for testing

  test0 = 0.0;
  test1 = 0.0;
  test2 = 0.0;
  test3 = 0.0;
  test4 = 0.0;
  test5 = 0.0;
  test6 = 0.0;
  test7 = 0.0;
  test8 = 0.0;
  test9 = 0.0;
  
}

