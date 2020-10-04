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
// AparMomFields.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#include <radar/AparMomFields.hh>
#include <cmath>
#include <cstring>

const double AparMomFields::missingDouble = -9999;

// constructor

AparMomFields::AparMomFields()

{

  init();

}

// Initialize to missing

void AparMomFields::init()

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
  vel_corrected = missingDouble;

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

  // normalized coherent power
  // also referred to as SQI - signal quality index
  
  ncp = missingDouble;
  ncp_h_only = missingDouble;
  ncp_v_only = missingDouble;
  ncp_h_minus_v = missingDouble;

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
  signal_flag = missingDouble;

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

  test = missingDouble;
  test2 = missingDouble;
  test3 = missingDouble;
  test4 = missingDouble;
  test5 = missingDouble;
  
}

