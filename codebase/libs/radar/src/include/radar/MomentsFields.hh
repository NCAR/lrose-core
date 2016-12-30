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
// MomentsFields.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2008
//
///////////////////////////////////////////////////////////////

#ifndef MomentsFields_HH
#define MomentsFields_HH

#include <radar/RadarComplex.hh>

////////////////////////
// This class

class MomentsFields {
  
public:
  
  MomentsFields();

  // set values to missing

  void init();

public:

  // public data

  double start; // for referencing fields in object using offsets
  
  // reflectivity
  
  double dbz; // corrected for atmospheric attenuation
  double dbz_no_atmos_atten; // no atmospheric attenuation correction
  double dbzhc;
  double dbzhx;
  double dbzvc;
  double dbzvx;

  // alternating mode velocity

  double vel; // best estimate
  double vel_alt; // alternative velocity estimate
  double vel_hv; // velocity from H and V separately
  double vel_h_only; // velocity using H only
  double vel_v_only; // velocity using V only
  double vel_alt_fold_interval; // -1, 0 or 1
  double vel_alt_fold_confidence; // 0.0 to 1.0
  double vel_corrected; // velocity corrected for platform motion

  // staggered PRT

  double vel_prt_short; // for short prt in staggered mode
  double vel_prt_long;  // for long prt in staggered mode
  double vel_diff; // vel_prt_short - vel_prt_long
  double vel_unfold_interval; /* vel unfolding interval
                               * with respect to short prt */

  // spectrum width

  double width;
  double width_r0r1;
  double width_r1r2;
  double width_r1r3;
  double width_ppls;
  double width_h_only; // width using H only
  double width_v_only; // width using V only
  double width_prt_long; // long-prt pulses in staggered mode
  double width_prt_short; // short-prt pulses in staggered mode

  // normalized coherent power
  // also referred to as SQI - signal quality index
  
  double ncp;
  double ncp_h_only;
  double ncp_v_only;
  double ncp_h_minus_v;

  double ncp_trip1;
  double ncp_trip2;
  double ncp_trip3;
  double ncp_trip4;

  double ncp_prt_long; // long-prt pulses in staggered mode
  double ncp_prt_short; // short-prt pulses in staggered mode
  double ncp_trip_flag;
  
  // signal to noise

  double snr;
  double snrhc;
  double snrhx;
  double snrvc;
  double snrvx;

 // uncalibrated power

  double dbm;
  double dbmhc;
  double dbmhx;
  double dbmvc;
  double dbmvx;

  // noise-subtracted uncalibrated power
  
  double dbmhc_ns;
  double dbmhx_ns;
  double dbmvc_ns;
  double dbmvx_ns;

  // dual polarization fields

  double zdrm; // measured - uncalibrated
  double zdr;  // calibrated
  double zdr_bias;  // zdr - ldr_diff

  double ldr;
  double ldrhm; // measured - uncalibrated
  double ldrh; // calibrated
  double ldrvm; // measured - uncalibrated
  double ldrv; // calibrated
  double ldr_diff; // ldrv-ldrh == zdr
  double ldr_mean; // mean ldr using h power as denom

  double rhohv; // noise corrected
  double rhohv_nnc; // no noise correction
  double phidp0; // uncorrected phidp
  double phidp;
  double phidp_cond; // phidp conditioned for kdp
  double phidp_filt; // phidp filtered for kdp
  double phidp_sdev_4kdp; // phidp sdev when computing kdp
  double phidp_jitter_4kdp; // phidp jitter when computing kdp
  double zdr_sdev_4kdp; // ZDR standard deviation when computing kdp
  double kdp;
  double psob; // phase shift on backscatter
  double kdp_hb; // kdp from hubbert bringi method

  // co-cross correlations

  double rho_vchx;
  double rho_hcvx;
  double rho_vxhx;
  double rho_phidp;
  
  // cross polar ratio - CPR

  double cpr_mag; // magnitude of CPR^2 in db
  double cpr_phase; // phase of CPR
  double cpr_ldr; // (mag cpr^2 / LDR) in linear space

  // CMD - Clutter Mitigation Decision

  double cpa;    // clutter phase alignment
  double pratio; // power ratio
  double mvar;   // pulse-to-pulse power variation in dB
  double tss;    // time series power smoothness
  double tpt;    // time series power trend in dB
  double cpd;    // cumulative phase difference
  double tclut;  // max of tpt and cpd
  double ozsnr;  // SNR away from 0 DC
  
  double tdbz;
  double spin;
  double max_tdbz_spin;

  double zdr_sdev;
  double phidp_sdev;

  double dbz_diff_sq;
  double dbz_spin_change;
  
  double cmd;
  double cmd_flag;
  
  double tdbz_interest;
  double spin_interest;
  double cpa_interest;
  double zdr_sdev_interest;
  double phidp_sdev_interest;

  // clutter power and noise residue

  double clut; // total clut power, after corrections for noise floor
  double clut_2_wx_ratio; // as computed by the filter
  double spectral_noise; // dBm power for skirts in spectrum
  double spectral_snr; // SNR for skirts in spectrum

  // refractivity fields

  double aiq_hc;
  double niq_hc;
  double aiq_vc;
  double niq_vc;

  // SZ8-64 phase coding
  
  double sz_trip_flag;
  double sz_leakage;
  
  // censoring flag

  double censoring_flag;

  // covariances linear

  double lag0_hc; // lag 0 (power) h co-polar
  double lag0_hx; // lag 0 (power) h co-polar
  double lag0_vc; // lag 0 (power) v co-polar
  double lag0_vx; // lag 0 (power) v x-polar

  RadarComplex_t lag0_vchx; // lag 0 v-co to h-x covariance
  RadarComplex_t lag0_hcvx; // lag 0 h-co to v-x  covariance
  RadarComplex_t lag1_hc;
  RadarComplex_t lag1_vc;
  RadarComplex_t lag1_hcvc; // lag 1 h-co to v-co covariance
  RadarComplex_t lag1_vchc; // lag 1 v-co to h-co covariance
  RadarComplex_t lag1_vxhx; // lag 1 v-x to h-x covariance
  RadarComplex_t lag2_hc;
  RadarComplex_t lag2_vc;
  RadarComplex_t lag3_hc;
  RadarComplex_t lag3_vc;
  RadarComplex_t rvvhh0;

  // covariances staggered

  double lag0_hc_short; // lag 0 (power) h co-polar short prt
  double lag0_vc_short; // lag 0 (power) v co-polar short prt
  double lag0_hc_long; // lag 0 (power) h co-polar long prt
  double lag0_vc_long; // lag 0 (power) v co-polar long prt

  RadarComplex_t lag1_hc_long; // lag 1 h-co long-to-long
  RadarComplex_t lag1_vc_long; // lag 1 v-co long-to-long
  RadarComplex_t lag1_hc_short; // lag 1 h-co short-to-short
  RadarComplex_t lag1_vc_short; // lag 1 v-co short-to-short
  RadarComplex_t lag1_hc_short_to_long; // lag 1 h-co short-to-long
  RadarComplex_t lag1_vc_short_to_long; // lag 1 v-co short-to-long
  RadarComplex_t lag1_hc_long_to_short; // lag 1 h-co long-to-short
  RadarComplex_t lag1_vc_long_to_short; // lag 1 v-co long-to-short

  RadarComplex_t rvvhh0_long;
  RadarComplex_t rvvhh0_short;

  // covariances as log power and phase

  double lag0_hc_db; // lag 0 (power) h co-polar
  double lag0_hx_db; // lag 0 (power) h co-polar
  double lag0_vc_db; // lag 0 (power) v x-polar
  double lag0_vx_db; // lag 0 (power) v x-polar

  double lag0_vchx_db; // lag 0 v-co to h-x covariance
  double lag0_vchx_phase;

  double lag0_hcvx_db; // lag 0 h-co to v-x  covariance
  double lag0_hcvx_phase;

  double lag1_hc_db;
  double lag1_hc_phase;

  double lag1_vc_db;
  double lag1_vc_phase;
  
  double lag1_hcvc_db; // lag 1 h-co to v-co covariance
  double lag1_hcvc_phase;

  double lag1_vchc_db; // lag 1 v-co to h-co covariance
  double lag1_vchc_phase;

  double lag1_vxhx_db; // lag 1 v-x to h-x covariance
  double lag1_vxhx_phase;

  double lag2_hc_db;
  double lag2_hc_phase;

  double lag2_vc_db;
  double lag2_vc_phase;

  double lag3_hc_db;
  double lag3_hc_phase;

  double lag3_vc_db;
  double lag3_vc_phase;

  double rvvhh0_db;
  double rvvhh0_phase;

  double sdev_vv;
  
  double prt;
  double num_pulses;

  double prt_short;
  double prt_long;

  // staggered lag1 covariances as log power and phase

  double lag0_hc_short_db; // lag 0 (power) h co-polar short prt
  double lag0_vc_short_db; // lag 0 (power) v co-polar short prt
  double lag0_hc_long_db; // lag 0 (power) h co-polar long prt
  double lag0_vc_long_db; // lag 0 (power) v co-polar long prt

  double lag1_hc_long_db; // lag 1 h-co long-to-long 
  double lag1_hc_long_phase; // lag 1 h-co long-to-long
  double lag1_vc_long_db; // lag 1 v-co long-to-long
  double lag1_vc_long_phase; // lag 1 v-co long-to-long

  double lag1_hc_short_db; // lag 1 h-co short-to-short 
  double lag1_hc_short_phase; // lag 1 h-co short-to-short
  double lag1_vc_short_db; // lag 1 v-co short-to-short
  double lag1_vc_short_phase; // lag 1 v-co short-to-short

  double lag1_hc_short_to_long_db; // lag 1 h-co short-to-long
  double lag1_hc_short_to_long_phase; // lag 1 h-co short-to-long
  double lag1_vc_short_to_long_db; // lag 1 v-co short-to-long
  double lag1_vc_short_to_long_phase; // lag 1 v-co short-to-long

  double lag1_hc_long_to_short_db; // lag 1 h-co long-to-short
  double lag1_hc_long_to_short_phase; // lag 1 h-co long-to-short
  double lag1_vc_long_to_short_db; // lag 1 h-co long-to-short
  double lag1_vc_long_to_short_phase; // lag 1 v-co long-to-short

  double rvvhh0_long_db;
  double rvvhh0_long_phase;
  double rvvhh0_short_db;
  double rvvhh0_short_phase;

  // identifying noise

  double dbm_for_noise;
  double dbm_sdev;
  double ncp_mean;
  RadarComplex_t phase_for_noise;
  double accum_phase_change;
  double phase_change_error;

  // flag to indicate noise/signal is present at a gate

  double noise_flag;
  double signal_flag;

  // noise_bias relative to calibrated noise
  // these will be constant for all gates in a ray
  
  double noise_bias_db_hc;
  double noise_bias_db_hx;
  double noise_bias_db_vc;
  double noise_bias_db_vx;

  // attenuation correction for refl and zdr

  double dbz_atten_correction;
  double zdr_atten_correction;
  double dbz_atten_corrected;
  double zdr_atten_corrected;

  // for testing

  mutable double test;
  mutable double test2;
  mutable double test3;
  mutable double test4;
  mutable double test5;
  
  static const double missingDouble;

protected:
private:

};

#endif

