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
// RadarMoments.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2007
//
///////////////////////////////////////////////////////////////
//
// RadarMoments computes moments at a gate
//
///////////////////////////////////////////////////////////////

#ifndef RadarMoments_hh
#define RadarMoments_hh

#include <string>
#include <radar/RadarFft.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfCalib.hh>
#include <radar/MomentsFields.hh>
#include <radar/RegressionFilter.hh>
#include <radar/AtmosAtten.hh>
#include <rapformats/DsRadarCalib.hh>
#include <radar/Sz864.hh>
using namespace std;

////////////////////////
// This class

class RadarMoments {
  
public:

  // receiver channel identification

  typedef enum {
    CHANNEL_HC, CHANNEL_VC, CHANNEL_HX, CHANNEL_VX
  } channel_t;

  // default constructor

  RadarMoments();

  // constructor with max gates specified
  
  RadarMoments(int max_gates,
               bool debug = false,
               bool verbose = false);
  
  // destructor
  
  ~RadarMoments();

  // set debugging

  void setDebug(bool state) { _debug = state; }
  void setVerbose(bool state) { _verbose = state; }

  // set max number of gates

  void setMaxGates(int maxGates);

  // initialize based on OpsInfo
  
  void init(double prt, const IwrfTsInfo &opsInfo);

  // initialize based on params passed in

  void init(double prt,
	    double wavelengthMeters,
	    double startRangeKm,
	    double gateSpacingKm);

  // initialize staggered PRT mode based on OpsInfo

  void initStagPrt(double prtShort,
                   double prtLong,
                   int staggeredM,
                   int staggeredN,
                   int nGatesPrtShort,
                   int nGatesPrtLong,
                   const IwrfTsInfo &opsInfo);
  
  // initialize staggered PRT mode based on params passed in

  void initStagPrt(double prtShort,
                   double prtLong,
                   int staggeredM,
                   int staggeredN,
                   int nGatesPrtShort,
                   int nGatesPrtLong,
                   double wavelengthMeters,
                   double startRangeKm,
                   double gateSpacingKm);

  // set number of samples

  void setNSamples(int n);

  // set calibration data
  
  void setCalib(const DsRadarCalib &calib);
  void setCalib(const IwrfCalib &calib);

  // set the estimated noise power based on analysis of the data
  // note that noise is power at the DRX, before subtraction of receiver gain

  void setEstimatedNoiseDbmHc(double val);
  void setEstimatedNoiseDbmVc(double val);
  void setEstimatedNoiseDbmHx(double val);
  void setEstimatedNoiseDbmVx(double val);

  // negate the velocity - i.e. change the sign
  
  void setChangeVelocitySign(bool state);

  // negate the velocity - i.e. change the sign - for staggered ONLY
  // this is used in conjunction with setChangeVelocitySign()
  // so if both are set they cancel out for staggered.
  
  void setChangeVelocitySignStaggered(bool state);

  // negate the phidp - i.e. change the sign
  
  void setChangePhidpSign(bool state);

  // set method for spectrum width computation
  // WIDTH_METHOD_R0R1: use R0R1, or R0Rm for staggered
  // WIDTH_METHOD_R1R2: use R1R2
  // Default is R0

  typedef enum {
    WIDTH_METHOD_R0R1,
    WIDTH_METHOD_R1R2,
    WIDTH_METHOD_HYBRID
  } spectrum_width_method_t;

  void setSpectrumWidthMethod(spectrum_width_method_t method) { _widthMethod = method; }

  // set the window R values, used for width correction

  void setWindowRValues(double windowR1,
                        double windowR2,
                        double windowR3,
                        double windowHalfR1,
                        double windowHalfR2,
                        double windowHalfR3);

  // should we correct for system phidp?
  
  void setCorrectForSystemPhidp(bool status) {
    _correctForSystemPhidp = status;
  }
  
  // should we change the sign of the Aiq?

  void setChangeAiqSign(bool status) {
    _changeAiqSign = status;
  }

  // do you want to compute cpa using the alternative method?
  // See comments for computeCpaAlt

  void setComputeCpaUsingAlt() {
    _computeCpaUsingAlt = true;
  }

  // set notch width for computing time series power smoothness
  
  void setTssNotchWidth(int width) { _tssNotchWidth = width; }
  
  // Clutter filtering options
  // normally adaptive filtering is used

  typedef enum {
    CLUTTER_FILTER_ADAPTIVE,
    CLUTTER_FILTER_REGRESSION,
    CLUTTER_FILTER_NOTCH
  } clutter_filter_type_t;

  // use the adaptive filter (default)
  
  void setUseAdaptiveFilter() {
    _clutterFilterType = CLUTTER_FILTER_ADAPTIVE;
  }

  // choose whether to apply spectral residue correction
  // specify the min SNR (in dB) - for values below this the
  // correction will not be applied.
  
  void setApplySpectralResidueCorrection(bool state, double minSnrDb) {
    _applySpectralResidueCorrection = state;
    _minSnrDbForResidueCorrection = minSnrDb;
  }

  // use polynomial regression filter

  void setUseRegressionFilter(bool interpAcrossNotch) {
    _clutterFilterType = CLUTTER_FILTER_REGRESSION;
    _regrInterpAcrossNotch = interpAcrossNotch;
  }
  
  // use notch filter
  
  void setUseSimpleNotchFilter(double notchWidthMps) {
    _clutterFilterType = CLUTTER_FILTER_NOTCH;
    _notchWidthMps = notchWidthMps;
  }
  
  // Set dB for dB correction in clutter processing.
  // This will turn on the dB for dB correction.
  
  void setDbForDb(double db_for_db_ratio,
                  double db_for_db_threshold);
  
  // set SZ
  // This will cause SZ864 processing to be applied
  
  void setSz(double snr_threshold,
             bool negate_phase_codes,
             double strong_to_weak_power_ratio_threshold,
             double out_of_trip_power_ratio_threshold,
             int out_of_trip_power_n_replicas);

  // set min SNR threshold for computing ZDR and LDR

  void setMinSnrDbForZdr(double val) { 
    _minSnrDbForZdr = val;
  }

  void setMinSnrDbForLdr(double val) { 
    _minSnrDbForLdr = val;
  }

  // compute ZDR using using SNR instead of LAG0 power
  // this compensates for difference in noise power in H and V

  void setComputeZdrUsingSnr(bool val = true) { 
    _computeZdrUsingSnr = val;
  }

  // Set adjustment for transmitter power.
  // If measured transmitter power is available, it can be
  // used to adjust DBZ for changes in power relative to
  // the power used during calibration

  void setAdjustDbzForMeasXmitPower(bool val = true) { 
    _adjustDbzForMeasXmitPower = val;
  }

  void setAdjustZdrForMeasXmitPower(bool val = true) { 
    _adjustZdrForMeasXmitPower = val;
  }

  void setMeasXmitPowerDbmH(double val) {
    _measXmitPowerDbmH = val;
  }

  void setMeasXmitPowerDbmV(double val) {
    _measXmitPowerDbmV = val;
  }

  /////////////////////////////////////////////////////////////
  // load up the atmospheric attenuation table,
  // given the elevation angle

  void loadAtmosAttenCorrection(int nGates,
                                double elevationDeg,
                                const AtmosAtten &atmosAtten);

  /////////////////////////////////////////////////////////////
  // covariance computations

  // Single polarization
  // Horizontal channel
  
  void computeCovarSinglePolH(RadarComplex_t *iqhc,
                              MomentsFields &fields);
  
  // Single polarization
  // Vertical channel
  
  void computeCovarSinglePolV(RadarComplex_t *iqvc,
                              MomentsFields &fields);
  
  // Compute covariances
  // DP_ALT_HV_CO_ONLY
  // Transmit alternating, receive copolar only
  
  void computeCovarDpAltHvCoOnly(RadarComplex_t *iqhc,
                                 RadarComplex_t *iqvc,
                                 MomentsFields &fields);

  // Compute covariances
  // DP_ALT_HV_CO_CROSS
  // Transmit alternating, receive co/cross
  
  void computeCovarDpAltHvCoCross(RadarComplex_t *iqhc,
                                  RadarComplex_t *iqvc,
                                  RadarComplex_t *iqhx,
                                  RadarComplex_t *iqvx,
                                  MomentsFields &fields);

  // Compute covariances
  // DP_SIM_HV
  // Dual pol, transmit simultaneous, receive fixed channels 
  
  void computeCovarDpSimHv(RadarComplex_t *iqhc,
                           RadarComplex_t *iqvc,
                           MomentsFields &fields);
    
  // Compute covariances
  // Dual pol, transmit H, receive co/cross
  
  void computeCovarDpHOnly(RadarComplex_t *iqhc,
                           RadarComplex_t *iqvx,
                           MomentsFields &fields);

  // Compute covariances
  // Dual pol, transmit V, receive co/cross
  
  void computeCovarDpVOnly(RadarComplex_t *iqvc,
                           RadarComplex_t *iqhx,
                           MomentsFields &fields);
  
  /////////////////////////////////////////////////////////////
  // moments computations

  // Single polarization
  // IQ passed in
  // Horizontal channel
  
  void singlePolH(RadarComplex_t *iqhc,
                  int gateNum,
                  bool isFiltered,
                  MomentsFields &fields);
  
  // Single polarization
  // IQ passed in
  // Vertical channel
  
  void singlePolV(RadarComplex_t *iqvc,
                  int gateNum,
                  bool isFiltered,
                  MomentsFields &fields);

  // Single polarization
  // Horizontal channel
  // covariances passed in
  
  void computeMomSinglePolH(double lag0_hc,
                            RadarComplex_t lag1_hc,
                            RadarComplex_t lag2_hc,
                            RadarComplex_t lag3_hc,
                            int gateNum,
                            MomentsFields &fields);
  
  // Single polarization
  // Vertical channel
  // covariances passed in

  void computeMomSinglePolV(double lag0_vc,
                            RadarComplex_t lag1_vc,
                            RadarComplex_t lag2_vc,
                            RadarComplex_t lag3_vc,
                            int gateNum,
                            MomentsFields &fields);
  
  // Compute reflectivity only
  
  double computeDbz(const RadarComplex_t *iq,
                    int nSamples,
                    double noisePower,
                    double baseDbz1km,
                    int gateNum);
  
  // DP_ALT_HV_CO_ONLY
  // Dual pol, transmit alternating, receive copolar only
  // IQ passed in
  
  void dpAltHvCoOnly(RadarComplex_t *iqhc,
                     RadarComplex_t *iqvc,
                     int gateNum,
                     bool isFiltered,
                     MomentsFields &fields);
  
  // DP_ALT_HV_CO_ONLY
  // Dual pol, transmit alternating, receive copolar only
  // covariances passed in

  void computeMomDpAltHvCoOnly(double lag0_hc,
                               double lag0_vc,
                               RadarComplex_t lag1_vchc,
                               RadarComplex_t lag1_hcvc,
                               RadarComplex_t lag2_hc,
                               RadarComplex_t lag2_vc,
                               int gateNum,
                               MomentsFields &fields);
  
  // DP_ALT_HV_CO_CROSS
  // Dual pol, transmit alternating, receive co/cross
  // IQ passed in
  
  void dpAltHvCoCross(RadarComplex_t *iqhc,
                      RadarComplex_t *iqvc,
                      RadarComplex_t *iqhx,
                      RadarComplex_t *iqvx,
                      int gateNum,
                      bool isFiltered,
                      MomentsFields &fields);
  
  // DP_ALT_HV_CO_CROSS
  // Dual pol, transmit alternating, receive co/cross
  // covariances passed in
  
  void computeMomDpAltHvCoCross(double lag0_hc,
                                double lag0_hx,
                                double lag0_vc,
                                double lag0_vx,
                                RadarComplex_t lag0_vchx,
                                RadarComplex_t lag0_hcvx,
                                RadarComplex_t lag0_hxvx,
                                RadarComplex_t lag1_vchc,
                                RadarComplex_t lag1_hcvc,
                                RadarComplex_t lag2_hc,
                                RadarComplex_t lag2_vc,
                                int gateNum,
                                MomentsFields &fields);
  
  // DP_SIM_HV
  // Dual pol, transmit simultaneous
  // IQ passed in
  
  void dpSimHv(RadarComplex_t *iqhc,
               RadarComplex_t *iqvc,
               int gateNum,
               bool isFiltered,
               MomentsFields &fields);

  // DP_SIM_HV
  // Dual pol, transmit simultaneous
  // covariances passed in
  
  void computeMomDpSimHv(double lag0_hc,
                         double lag0_vc,
                         RadarComplex_t Rvvhh0,
                         RadarComplex_t lag1_hc,
                         RadarComplex_t lag1_vc,
                         RadarComplex_t lag2_hc,
                         RadarComplex_t lag2_vc,
                         RadarComplex_t lag3_hc,
                         RadarComplex_t lag3_vc,
                         int gateNum,
                         MomentsFields &fields);
  
  // DP_H_ONLY
  // Dual pol, transmit H only, receive H and V
  // IQ passed in
  
  void dpHOnly(RadarComplex_t *iqhc,
               RadarComplex_t *iqvx,
               int gateNum,
               bool isFiltered,
               MomentsFields &fields);
  
  // DP_H_ONLY
  // Dual pol, transmit H only, receive H and V
  // covariances passed in
  
  void computeMomDpHOnly(double lag0_hc,
                         double lag0_vx,
                         RadarComplex_t lag1_hc,
                         RadarComplex_t lag2_hc,
                         RadarComplex_t lag3_hc,
                         int gateNum,
                         MomentsFields &fields);

  // DP_V_ONLY
  // Dual pol, transmit V only, receive H and V
  // IQ passed in
  
  void dpVOnly(RadarComplex_t *iqvc,
               RadarComplex_t *iqhx,
               int gateNum,
               bool isFiltered,
               MomentsFields &fields);

  // DP_V_ONLY
  // Dual pol, transmit V only, receive H and V
  
  void computeMomDpVOnly(double lag0_vc,
                         double lag0_hx,
                         RadarComplex_t lag1_vc,
                         RadarComplex_t lag2_vc,
                         RadarComplex_t lag3_vc,
                         int gateNum,
                         MomentsFields &fields);
  
  // Single polarization
  // Staggered-PRT
  
  void singlePolHStagPrt(RadarComplex_t *iqhc,
                         RadarComplex_t *iqhcShort,
                         RadarComplex_t *iqhcLong,
                         int gateNum,
                         bool isFiltered,
                         MomentsFields &fields);
  
  // Single polarization Staggered-PRT
  // Assumes data is in horizontal channel
  // covariances passed in

  void singlePolHStagPrt(double lag0_hc_long,
                         double lag0_hc_short,
                         RadarComplex_t &lag1_hc_long,
                         RadarComplex_t &lag1_hc_short,
                         RadarComplex_t &lag1_hc_short_to_long,
                         RadarComplex_t &lag1_hc_long_to_short,
                         int gateNum,
                         bool isFiltered,
                         MomentsFields &fields);

  // single pol stag prt power

  void singlePolHStagPrtPower(double lag0_hc,
                              int gateNum,
                              bool isFiltered,
                              MomentsFields &fields);
  
  // Dual pol, transmit simultaneous, receive fixed channels
  // Staggered-PRT
  
  void dpSimHvStagPrt(RadarComplex_t *iqhc,
                      RadarComplex_t *iqvc,
                      RadarComplex_t *iqhcShort,
                      RadarComplex_t *iqvcShort,
                      RadarComplex_t *iqhcLong,
                      RadarComplex_t *iqvcLong,
                      int gateNum,
                      bool isFiltered,
                      MomentsFields &fields);
  
  // Dual pol, transmit simultaneous, receive fixed channels
  // Staggered-PRT
  // Covariances passed in

  void dpSimHvStagPrt(double lag0_hc_long,
                      double lag0_vc_long,
                      double lag0_hc_short,
                      double lag0_vc_short,
                      RadarComplex_t &lag1_hc_long,
                      RadarComplex_t &lag1_vc_long,
                      RadarComplex_t &lag1_hc_short,
                      RadarComplex_t &lag1_vc_short,
                      RadarComplex_t &lag1_hc_short_to_long,
                      RadarComplex_t &lag1_vc_short_to_long,
                      RadarComplex_t &lag1_hc_long_to_short,
                      RadarComplex_t &lag1_vc_long_to_short,
                      RadarComplex_t &rvvhh0_long,
                      RadarComplex_t &rvvhh0_short,
                      int gateNum,
                      bool isFiltered,
                      MomentsFields &fields);

  // dual pol stag prt power

  void dpSimHvStagPrtPower(double lag0_hc,
                           double lag0_vc,
                           int gateNum,
                           bool isFiltered,
                           MomentsFields &fields);

  // Dual pol, transmit H only, receive fixed channels
  // Staggered-PRT
  
  void dpHOnlyStagPrt(RadarComplex_t *iqhc,
                      RadarComplex_t *iqvx,
                      RadarComplex_t *iqhcShort,
                      RadarComplex_t *iqvxShort,
                      RadarComplex_t *iqhcLong,
                      RadarComplex_t *iqvxLong,
                      int gateNum,
                      bool isFiltered,
                      MomentsFields &fields);
  
  void dpHOnlyStagPrtPower(double lag0_hc,
                           double lag0_vx,
                           int gateNum,
                           bool isFiltered,
                           MomentsFields &fields);

  // Dual pol, transmit V only, receive fixed channels
  // Staggered-PRT
  
  void dpVOnlyStagPrt(RadarComplex_t *iqvc,
                      RadarComplex_t *iqhx,
                      RadarComplex_t *iqvcShort,
                      RadarComplex_t *iqhxShort,
                      RadarComplex_t *iqvcLong,
                      RadarComplex_t *iqhxLong,
                      int gateNum,
                      bool isFiltered,
                      MomentsFields &fields);
  
  void dpVOnlyStagPrtPower(double lag0_vc,
                           double lag0_hx,
                           int gateNum,
                           bool isFiltered,
                           MomentsFields &fields);

  // Single polarization, range unfolding using SZ864
  
  void singlePolHSz864(GateData &gateData,
                      RadarComplex_t *delta12,
                      int gateNum,
                      int ngatesPulse,
                      const RadarFft &fft);
  
  // Single polarization, SZ864, Filtered
  
  void singlePolHSz864Filtered(GateData &gateData,
                              int gateNum,
                              int ngatesPulse);
  
  // apply clutter filter to IQ time series
  //
  // Inputs:
  //   nSamples
  //   fft: object to be used for FFT computations
  //   regr: object to be used for regression filtering
  //   window: window used to create iqWindowed
  //   iqOrig: unfltered time series, not windowed
  //   iqWindowed: unfiltered time series,
  //               windowed using VONHANN, BLACKMAN or BLACKMAN_NUTTALL
  //   specWindowed: if not NULL, contains the spectrum of iqWindowed
  //   calibratedNoise: noise level at digitizer, from cal
  //
  //  Outputs:
  //    iqFiltered: filtered time series
  //    iqNotched: if non-NULL, notched time series
  //    filterRatio: ratio of raw to unfiltered power, before applying correction
  //    spectralNoise: spectral noise estimated from the spectrum
  //    clutResidueRatio: ratio of spectral noise to calibrated noise
  //    specRatio: ratio of filtered to unfiltered in spectrum, if non-NULL
  
  void applyClutterFilter(int nSamples,
                          const RadarFft &fft,
                          const RegressionFilter &regr,
                          const double *window, // active window
                          const RadarComplex_t *iqOrig, // non-windowed
                          const RadarComplex_t *iqWindowed, // windowed
                          const RadarComplex_t *specWindowed, // windowed
                          double calibratedNoise,
                          RadarComplex_t *iqFiltered,
                          RadarComplex_t *iqNotched,
                          double &filterRatio,
                          double &spectralNoise,
                          double &spectralSnr,
                          double *specRatio = NULL);
  
  void applyAdaptiveFilter(int nSamples,
                           const RadarFft &fft,
                           const RadarComplex_t *iqWindowed,
                           const RadarComplex_t *specWindowed,
                           double calibratedNoise,
                           RadarComplex_t *iqFiltered,
                           RadarComplex_t *iqNotched,
                           double &filterRatio,
                           double &spectralNoise,
                           double &clutResidueRatio,
                           double *specRatio = NULL);
  
  void applyNotchFilter(int nSamples,
                        const RadarFft &fft,
                        const RadarComplex_t *iqWindowed,
                        const RadarComplex_t *specWindowed,
                        double calibratedNoise,
                        RadarComplex_t *iqFiltered,
                        double &filterRatio,
                        double &spectralNoise,
                        double &spectralSnr,
                        double *specRatio = NULL);
  
  // apply polynomial regression clutter filter to IQ time series
  //
  // NOTE: IQ input data should not be windowed.
  //       Output filtered data is not windowed.
  //
  // Inputs:
  //   nSamples
  //   fft: object to be used for filling in notch
  //   regr: object to be used for polynomial computations
  //   iqOrig: unfiltered time series, not windowed
  //   calibratedNoise: noise value from calibration in linear units
  //
  //  Outputs:
  //    iqFiltered: filtered time series
  //    iqNotched: if non-NULL, notched time series
  //    filterRatio: ratio of raw to unfiltered power,
  //                 before applying correction
  //    spectralNoise: spectral noise estimated from the spectrum
  //    spectralSnr: ratio of spectral noise to noise power
  //    specRatio: if non-NULL, contains ratio of filtered to
  //               unfiltered spectrum

  // for regression filter, the input data and filtered result is
  // not windowed. The window is passed in for use in the FFTs
  
  void applyRegressionFilter(int nSamples,
                             const RadarFft &fft,
                             const RegressionFilter &regr,
                             const double *window,
                             const RadarComplex_t *iqOrig, // non-windowed
                             double calibratedNoise,
                             bool interpAcrossNotch,
                             RadarComplex_t *iqFiltered,
                             RadarComplex_t *iqNotched,
                             double &filterRatio,
                             double &spectralNoise,
                             double &spectralSnr,
                             double *specRatio = NULL);
  
  // apply adaptive clutter filter to staggered PRT
  // IQ time series
  //
  // The following is assumed:
  //
  //   1. nSamplesHalf refers to short and long prt sequences.
  //      nSamples = nSamplesHalf * 2
  //   2. The combined sequence starts with short PRT.
  //   3. Memory has been allocated as follows:
  //        iqOrigShort[nSamplesHalf]
  //        iqOrigLong[nSamplesHalf]
  //        iqFiltShort[nSamplesHalf]
  //        iqFiltLong[nSamplesHalf]
  //        spectralRatioShort[nSamplesHalf]
  //        spectralRatioLong[nSamplesHalf]
  //   4. Input and output data is windowed appropriately for FFTs.
  //
  // The short and long sequences are filtered separately.
  // The notch is not filled in.
  //
  // Inputs:
  //   fftHalf: object to be used for FFT computations
  //   iqOrigShort: unfiltered short-prt time series
  //   iqOrigLong: unfiltered long-prt time series
  //   channel: which channel - used to determine calibrated noise
  //   interpAcrossNotch: whether to fill in notch
  //
  //  Outputs:
  //    iqFiltShort: filtered short-prt time series
  //    iqFiltLong: filtered long-prt time series
  //    filterRatio: ratio of raw to unfiltered power,
  //    before applying correction
  //    spectralNoise: spectral noise estimated from the spectrum
  //    spectralSnr: ratio of spectral noise to noise power
  //    specRatioShort: filtered/unfiltered ratio, short PRT, if non-NULL
  //    specRatioLong: filtered/unfiltered ratio, long PRT, if non-NULL
  
  void applyAdapFilterStagPrt(int nSamplesHalf,
                              const RadarFft &fftHalf,
                              const RadarComplex_t *iqShort,
                              const RadarComplex_t *iqLong,
                              double calibratedNoise,
                              RadarComplex_t *iqFiltShort,
                              RadarComplex_t *iqFiltLong,
                              double &filterRatio,
                              double &spectralNoise,
                              double &spectralSnr,
                              double *spectralRatioShort = NULL,
                              double *spectralRatioLong = NULL);
  
  // apply polynomial regression clutter filter to IQ time series
  //
  // NOTE: IQ data should not be windowed.
  //
  // Inputs:
  //   nSamples
  //   fftHalf: fft object for short and long half time series, length nSamples/2
  //   regr: object to be used for polynomial computations
  //   iqOrig: unfiltered time series, not windowed
  //   channel: which channel - used to determine calibrated noise
  //
  //  Outputs:
  //    iqFiltered: filtered time series
  //    filterRatio: ratio of raw to unfiltered power, before applying correction
  //    spectralNoise: spectral noise estimated from the spectrum
  //    spectralSnr: ratio of spectral noise to noise power
  //    specRatio: if non-NULL, contains ratio of filtered to unfiltered spectrum
  //
  //  Memory allocation by calling routine:
  //    regr - initialized to size nSamples
  //    iqOrig[nSamples]
  //    iqFiltered[nSamples]
  //    specRatio[nSamples] - if non-NULL
  
  void applyRegrFilterStagPrt(int nSamples,
                              const RadarFft &fftHalf,
                              const RegressionFilter &regr,
                              const RadarComplex_t *iqOrig,
                              double calibratedNoise,
                              bool interpAcrossNotch,
                              RadarComplex_t *iqFiltered,
                              double &filterRatio,
                              double &spectralNoise,
                              double &spectralSnr,
                              double *specRatio = NULL);
  
  // apply polynomial regression clutter filter to IQ time series
  //
  // NOTE: IQ data should not be windowed.
  //
  // Inputs:
  //   nSamples
  //   nExpanded = length of expanded time series with inserted 0's
  //             = (nSamples / 2) * (m + n)
  //   fftExp: object to be used for FFT computations
  //   regr: object to be used for polynomial computations
  //   iqOrig: unfiltered time series, not windowed
  //   channel: which channel - used to determine calibrated noise
  //
  //  Outputs:
  //    iqFiltered: filtered time series
  //    filterRatio: ratio of raw to unfiltered power,
  //                 before applying correction
  //    spectralNoise: spectral noise estimated from the spectrum
  //    spectralSnr: ratio of spectral noise to noise power
  //    specRatio: if non-NULL, contains ratio of filtered
  //               to unfiltered spectrum
  //
  //  Memory allocation by calling routine:
  //    fftExp - initialized to size nExpanded
  //    regr - initialized to size nSamples
  //    iqOrig[nSamples]
  //    iqFiltered[nSamples]
  //    specRatio[nSamples] - if non-NULL

  void applyRegrFilterStagPrt(int nSamples,
                              int nExpanded,
                              const RadarFft &fftExp,
                              const RegressionFilter &regr,
                              const RadarComplex_t *iqOrig,
                              double calibratedNoise,
                              bool interpAcrossNotch,
                              RadarComplex_t *iqFiltered,
                              double &filterRatio,
                              double &spectralNoise,
                              double &spectralSnr,
                              double *specRatio = NULL);

  //////////////////////////////////////////////////////////////////////
  // Compute the spectral snr
  //
  // Side effects: the following are computed:
  //  specWindowed (from forward fft)
  //  spectralNoise
  //  spectralSnr
  
  void computeSpectralSnr(int nSamples,
                          const RadarFft &fft,
                          const RadarComplex_t *iqWindowed,
                          RadarComplex_t *specWindowed,
                          double calibratedNoise,
                          double &spectralNoise,
                          double &spectralSnr);
  
  // Separate a staggered time series into two
  // interleaved sequences, short PRT and long PRT.
  //
  // The following is assumed:
  //
  //   1. nSamples refers to the combined length.
  //   2. nSamples is even.
  //   4. The sequence starts with short PRT.
  //   4. Memory has been allocated as follows:
  //        iq[nSamples]
  //        iqShort[nSamples/2]
  //        iqLong[nSamples/2]

  static void separateStagIq(int nSamples,
                             const RadarComplex_t *iq,
                             RadarComplex_t *iqShort,
                             RadarComplex_t *iqLong);
  
  // Combine short and long time series sequences into
  // a single sequences, starting with short PRT.
  //
  // The following is assumed:
  //
  //   1. nSamples refers to the combined length.
  //   2. nSamples is even.
  //   4. The sequence starts with short PRT.
  //   4. Memory has been allocated as follows:
  //        iq[nSamples]
  //        iqShort[nSamples/2]
  //        iqLong[nSamples/2]
  
  static void combineStagIq(int nSamples,
                            const RadarComplex_t *iqShort,
                            const RadarComplex_t *iqLong,
                            RadarComplex_t *iq);
  
  // Expand a staggered time series into a series
  // with 0's inserted, to create a constant-prt
  // time series
  //
  // The following is assumed:
  //
  //   1. nSamples refers to the original length.
  //   2. nExpanded is the expanded length.
  //      nExpanded = (nSamples / 2) * (m + n)
  //   3. The sequence starts with short PRT.
  //   4. Memory has been allocated as follows:
  //        iq[nSamples]
  //        iqExpanded[nExpanded]
  
  static void expandStagIq(int nSamples,
                           int nExpanded,
                           int staggeredM,
                           int staggeredN,
                           const RadarComplex_t *iq,
                           RadarComplex_t *iqExpanded);
  
  // Condense and expanded staggered time series to
  // match the times of the the original series
  // by removing the extra pulses
  //
  // The following is assumed:
  //
  //   1. nSamples refers to the original length.
  //   2. nExpanded is the expanded length.
  //      nExpanded = (nSamples / 2) * (m + n)
  //   3. The sequence starts with short PRT.
  //   4. Memory has been allocated as follows:
  //        iqExpanded[nExpanded]
  //        iqCondensed[nSamples]
  
  static void condenseStagIq(int nSamples,
                             int nExpanded,
                             int staggeredM,
                             int staggeredN,
                             const RadarComplex_t *iqExpanded,
                             RadarComplex_t *iqCondensed);
  
  // Applies previously computed filter ratios, in the spectral
  // domain, to a time series
  //
  // Inputs:
  //   nSamples
  //   fft: object to be used for FFT computations
  //   iq: input time series to be adjusted for filtering
  //   specRatio: ratio of filtered to unfiltered in spectrum
  //
  //  Outputs:
  //    iqFiltered: filtered time series
  //    iqNotched: if not NULL, notched time series
  
  void applyFilterRatio(int nSamples,
                        const RadarFft &fft,
                        const RadarComplex_t *iq,
                        const double *specRatio,
                        RadarComplex_t *iqFiltered,
                        RadarComplex_t *iqNotched);
  
  // apply clutter filter for SZ 864
  
  void applyClutterFilterSz(int nSamples,
                            const RadarFft &fft,
                            GateData &gateData,
                            double calibratedNoise,
                            double &filterRatio,
                            double &spectralNoise,
                            double &clutResidueRatio);

  // compute the size of the expanded time series for
  // staggered prt

  static int computeNExpandedStagPrt(int nSamples,
                                     int staggeredM,
                                     int staggeredN);

  int computeNExpandedStagPrt(int nSamples);

  // Clutter phase alignment - single pol

  static double computeCpa(const RadarComplex_t *iq, int nSamples);

  // Clutter phase alignment - dual pol
  
  static double computeCpa(const RadarComplex_t *iqh,
                           const RadarComplex_t *iqv,
                           int nSamples);
  
  // Clutter phase alignment - alternative - single pol.
  // Alternative formulation where we look for the
  // minimum 5-pt running CPA and then compute the CPA
  // values on each side of the minimum. The mean of
  // these two values is returned.
  //
  // This formulation works well for time series in
  // which the CPA value is high, then becomes low for
  // a short period, and then returns to high values
  // for the rest of the series.

  static double computeCpaAlt(const RadarComplex_t *iq, int nSamples);
  
  // compute CPA alt - dual pol

  static double computeCpaAlt(const RadarComplex_t *iqh,
                              const RadarComplex_t *iqv,
                              int nSamples);
  
  // compute refractivity

  static void computeRefract(const RadarComplex_t *iq,
                             int nSamples,
                             double &aiq,
                             double &niq,
                             bool changeAiqSign = false);
  
  // compute CPR - cross-polar ratio
  
  static void computeCpr(const RadarComplex_t *iqhc,
                         const RadarComplex_t *iqvx,
                         int nSamples,
                         double &cprPowerDb,
                         double &cprPhaseDeg);
  
  // compute sdev of magnitudes of a time series

  static double computeMagSdev(const RadarComplex_t *iq,
                               int nSamples);

  // compute ratio of DC power to total power
  // Note - this is inefficient, since it initializes an FFT object
  // on every call and compute the spectrum
  
  static double computePowerRatio(const RadarComplex_t *iq,
                                  int nSamples);
  
  static double computePowerRatio(const RadarComplex_t *iqh,
                                  const RadarComplex_t *iqv,
                                  int nSamples);
  
  // compute mean pulse-to-pulse power variation
  // in dB space
  
  static double computeMvar(const RadarComplex_t *iq,
                            int nSamples,
                            double prt);
  
  // mvar for H/V simultaneous
  
  static double computeMvarSim(const RadarComplex_t *iqh,
                               const RadarComplex_t *iqv,
                               int nSamples,
                               double prt);
  
  // mvar for H/V alternating

  static double computeMvarAlt(const RadarComplex_t *iqh,
                               const RadarComplex_t *iqv,
                               int nSamplesHalf,
                               double prt);
  
  // compute time-series power trend between
  // max-power and min-power halves of the series,
  // in dB
  
  static double computeTpt(const RadarComplex_t *iq,
                           int nSamples);
  
  // compute number of times I and Q data cross 0
  // returns number of crossings
  
  static int computeIqz(const RadarComplex_t *iq,
                        int nSamples);
  
  // compute the Cumulative Phase Difference
  // This is the maximum absolute change in cumulative
  // for the time series
  
  double computeCpd(const RadarComplex_t *iq,
                    int nSamples);
  
  // Compute the off-zero SNR, in dB.
  // This is the SNR away from 0 DC.
  // It should computed using a relatively wide notch (say 8 m/s),
  // to make sure no clutter power is included in the result.
  // Pass in a suitable window for the IQ data. This should be
  // a VonHann, Blackman or BlackmanNuttall window, or NULL if the IQ data
  // has already been windowed.
  // notchWidthMps is specified in m/s.
  
  double computeOzSnr(const RadarComplex_t *iq,
                      const double *window,
                      int nSamples,
                      const RadarFft &fft,
                      double notchWidthMps,
                      double noisePower);

  // compute NCP from time series

  double computeNcp(RadarComplex_t *iq);

  // initialize rectangular window

  static void initWindowRect(int nSamples, double *window);

  // initialize vonHann window

  static void initWindowVonhann(int nSamples, double *window);
  
  // initialize Blackman window
  
  static void initWindowBlackman(int nSamples, double *window);
  
  // initialize Blackman-Nuttall window
  
  static void initWindowBlackmanNuttall(int nSamples, double *window);
  
  // initialize Tukey window
  // alpha can vary between 0 and 1
  // alpha == 1 implies a VonHann window
  // alpha == 0 implies a rectangular window

  static void initWindowTukey(double alpha, int nSamples, double *window);

  // create rectangular window
  // Allocates memory and returns window
  
  static double *createWindowRect(int nSamples);

  // create vonHann window
  // Allocates memory and returns window

  static double *createWindowVonhann(int nSamples);
  
  // create Blackman window
  // Allocates memory and returns window

  static double *createWindowBlackman(int nSamples);

  // create Blackman-Nuttall window
  // Allocates memory and returns window

  static double *createWindowBlackmanNuttall(int nSamples);

  // create Tukey window
  // Allocates memory and returns window

  static double *createWindowTukey(double alpha, int nSamples);

  // apply window to IQ samples, in place
  
  static void applyWindow(RadarComplex_t *iq,
                          const double *window,
                          int nSamples);

  // apply window to IQ samples, not in place
  
  static void applyWindow(const RadarComplex_t *iqOrig,
                          const double *window,
                          RadarComplex_t *iqWindowed,
                          int nSamples);

  // invert (undo) window to IQ samples, not in place
  
  static void invertWindow(const RadarComplex_t *iqWindowed,
                           const double *window,
                           RadarComplex_t *iqOrig,
                           int nSamples);
  
  // compute serial correlation value for a window

  static double computeWindowCorrelation(int lag,
                                         double *window,
                                         int nSamples);

  // compute time series power smoothness
  
  double computeTss(const RadarComplex_t *iqWindowed,
                    int nSamples,
                    const RadarFft &fft);

  // compute percentile power value in spectrum
  
  double computePowerPercentile(int nSamples,
                                double *powerSpec,
                                double percentile);

  // get the calibrated noise power given the channel
  
  double getCalNoisePower(channel_t channel);

  // get the estimated noise power given the channel
  
  double getEstNoisePower(channel_t channel);

  // get the receiver gain given the channel
  
  double getReceiverGain(channel_t channel);

  // get the base dbz at 1km given the channel
  
  double getBaseDbz1km(channel_t channel);

  // get nyquist after object has been initialized/used

  double getNyquist() const { return _nyquist; }
  double getNyquistPrtShort() const { return _nyquistPrtShort; }
  double getNyquistPrtLong() const { return _nyquistPrtLong; }

  // De-trend a time series in preparation
  // for windowing and FFT.
  //
  // The end-point linear trend is removed and the residual remains.
  //
  // This mitigates the raised noise floor problem which can
  // occur when a window is applied to a time series with
  // ends which are considerably different.
  //
  // The I and Q series are filtered indendently.
  //
  // A line is computed between the start and end median values, using
  // 3 points for the median.
  // The de-trended time series is computed as the redidual difference
  // between the original values and the computed line.
  
  static void detrendTs(const RadarComplex_t *iq,
                        int nSamples,
                        RadarComplex_t *iqDeTrended);

  // prepare for noise detection by computing the
  // lag0 power and the velocity phase

  void singlePolHNoisePrep(double lag0_hc,
                           RadarComplex_t lag1_hc,
                           MomentsFields &fields);
  
  void singlePolVNoisePrep(double lag0_vc,
                           RadarComplex_t lag1_vc,
                           MomentsFields &fields);
  
  void dpAltHvCoOnlyNoisePrep(double lag0_hc,
                              double lag0_vc,
                              RadarComplex_t lag2_hc,
                              RadarComplex_t lag2_vc,
                              MomentsFields &fields);
  
  void dpAltHvCoCrossNoisePrep(double lag0_hc,
                               double lag0_hx,
                               double lag0_vc,
                               double lag0_vx,
                               RadarComplex_t lag2_hc,
                               RadarComplex_t lag2_vc,
                               MomentsFields &fields);
  
  void dpSimHvNoisePrep(double lag0_hc,
                        double lag0_vc,
                        RadarComplex_t lag1_hc,
                        RadarComplex_t lag1_vc,
                        MomentsFields &fields);
  
  void dpHOnlyNoisePrep(double lag0_hc,
                        double lag0_vx,
                        RadarComplex_t lag1_hc,
                        MomentsFields &fields);
  
  void dpVOnlyNoisePrep(double lag0_vc,
                        double lag0_hx,
                        RadarComplex_t lag1_vc,
                        MomentsFields &fields);
  
  void singlePolHStagPrtNoisePrep(RadarComplex_t *iqhc,
                                  RadarComplex_t *iqhcShort,
                                  RadarComplex_t *iqhcLong,
                                  MomentsFields &fields);
  
  void dpSimHvStagPrtNoisePrep(RadarComplex_t *iqhc,
                               RadarComplex_t *iqvc,
                               RadarComplex_t *iqhcShort,
                               RadarComplex_t *iqvcShort,
                               RadarComplex_t *iqhcLong,
                               RadarComplex_t *iqvcLong,
                               MomentsFields &fields);


  // compute ripple from mitch switch
  //
  // The Mitch switch presents a different face to the transmit pulse
  // every alternate H and V pulse. This can lead to a difference in
  // power or phase for each alternate pulse, which in turn can lead
  // to spikes in the spectrum at 0.25 of the main PRF (or 0.5 of the
  // H PRF).
  
  void computeMitchSwitchRipple(RadarComplex_t *iqhc,
                                RadarComplex_t *iqvc,
                                RadarComplex_t *iqhx,
                                RadarComplex_t *iqvx,
                                RadarComplex_t &rippleHc,
                                RadarComplex_t &rippleVc,
                                RadarComplex_t &rippleHx,
                                RadarComplex_t &rippleVx,
                                double &snrHc,
                                double &snrVc,
                                double &snrHx,
                                double &snrVx);

protected:
private:
  
  static const double _missing;
  static const double _c1;
  static const double _c2;
  static const double _c3;

  bool _debug;
  bool _verbose;

  // max number of gates
  
  int _maxGates;

  // number of pulse samples
  
  int _nSamples;
  int _nSamplesHalf;
  
  // moments parameters

  bool _correctForSystemPhidp;
  bool _changeAiqSign;
  bool _computeCpaUsingAlt; // use alternative cpa method

  // clutter filtering parameters
  
  clutter_filter_type_t _clutterFilterType;

  bool _applySpectralResidueCorrection;
  double _minSnrDbForResidueCorrection; // if SNR is below this, do not correct
  
  bool _applyDbForDbCorrection;
  double _dbForFbRatio;
  double _dbForDbThreshold;

  int _notchStart;  // start of filter notch in spectral domain
  int _notchEnd;    // end   of filter notch in spectral domain

  double _spectralNoise;
  int _weatherPos;
  int _clutterPos;

  bool _regrInterpAcrossNotch; // interpolate across the notch - regression filter
  double _notchWidthMps;       // notch width in meters per sec - notch filter

  // SNR thresholds for ZDR and LDR
  
  double _minSnrDbForZdr; // min SNR in both channels required for ZDR
  double _minSnrDbForLdr; // min SNR in both channels required for LDR

  // nyquist etc.
  
  double _wavelengthMeters;
  double _prt;
  double _nyquist;

  // staggered PRT

  double _prtShort;
  double _prtLong;
  int _staggeredM;
  int _staggeredN;
  int _nGatesPrtShort;
  int _nGatesPrtLong;
  double _nyquistPrtShort;
  double _nyquistPrtLong;
  double _nyquistShortPlusLong;
  double _nyquistStagNominal;
  int _LL;
  int _PP_[32];
  int *_PP;

  // change the velocity sign - negation - for those cases
  // in which the phase sense is negative

  double _velSign;
  double _velSignStaggered;

  // change the phidp sign - negation - for those cases
  // in which the phase sense is negative

  double _phidpSign;

  // computing width

  spectrum_width_method_t _widthMethod;

  // R values, at various lags, for the windows
  // used to correct R values used in width computations

  double _windowR1;
  double _windowR2;
  double _windowR3;
  double _windowHalfR1;
  double _windowHalfR2;
  double _windowHalfR3;

  // range geometry

  double _startRangeKm;
  double _gateSpacingKm;

  // range correction

  double *_rangeCorr;
  bool _rangeCorrInit;

  // atmospheric attenuation correction

  double *_atmosAttenCorr;

  // calibration
  
  double _calNoisePowerHc;
  double _calNoisePowerHx;
  double _calNoisePowerVc;
  double _calNoisePowerVx;

  double _baseDbz1kmHc;
  double _baseDbz1kmHx;
  double _baseDbz1kmVc;
  double _baseDbz1kmVx;

  double _receiverGainDbHc;
  double _receiverGainDbHx;
  double _receiverGainDbVc;
  double _receiverGainDbVx;

  double _dbzCorrection;
  double _zdrCorrectionDb;
  double _ldrCorrectionDbH;
  double _ldrCorrectionDbV;
  double _systemPhidpDeg;

  double _calibXmitPowerDbmH;
  double _calibXmitPowerDbmV;

  // estimated noise power
  
  double _estNoisePowerHc;
  double _estNoisePowerHx;
  double _estNoisePowerVc;
  double _estNoisePowerVx;

  // compute ZDR using using SNR instead of LAG0 power
  // this compensates for difference in noise power in H and V

  bool _computeZdrUsingSnr;

  // adjusting for measured transmitter power

  bool _adjustDbzForMeasXmitPower;
  bool _adjustZdrForMeasXmitPower;

  double _measXmitPowerDbmH;
  double _measXmitPowerDbmV;
  
  // moments

  static const double _minDetectableSnr;

  // phidp offset

  static const double _phidpPhaseLimitAlt;
  static const double _phidpPhaseLimitSim;
  RadarComplex_t _phidpOffsetAlt;
  RadarComplex_t _phidpOffsetSim;

  // apply SZ864

  bool _applySz;
  Sz864 *_sz;

  // computing time series power smoothness
  
  int _tssNotchWidth;
  
  // functions

  void _init();

  void _computeRangeCorrection(double startRangeKm,
                               double gateSpacingKm);

  double _computeR0R1Width(double r0,
                           double r1,
                           double nyquist) const;
  

  double _computeR1R2Width(double r1,
                           double r2,
                           double nyquist) const;
  

  double _computeR1R3Width(double r1,
                           double r3,
                           double nyquist) const;

  double _computePplsWidth(double r0,
                           double r1,
                           double r2,
                           double nyquist) const;

  double _computeHybridWidth(double r0,
                             double r1,
                             double r2,
                             double r3,
                             double nyquist) const;

  double _computeStagWidth(double rA,
                           double rB,
                           int lagA,
                           int lagB,
                           double nyquist) const;

  double _computePwrCorrectionRatio(int nSamples,
				    double spectralSnr,
				    double rawPower,
				    double filteredPower,
				    double powerRemoved,
                                    double calibratedNoise);

  void _fillNotchRegrFilter(int nSamples,
                            const RadarFft &fft,
                            const RadarComplex_t *iqOrig,
                            const RadarComplex_t *iqRegr,
                            RadarComplex_t *iqFiltered,
                            double *specRatio);

  void _adapFiltHalfTseries(int nSamplesHalf,
                            const RadarFft &fftHalf,
                            const RadarComplex_t *iq,
                            double calibratedNoise,
                            double nyquist,
                            bool adjustForPowerResidue,
                            RadarComplex_t *iqFiltered,
                            double &filterRatio,
                            double &spectralNoise,
                            double &spectralSnr,
                            double *specRatio);
  
  static void _interpAcrossNotch(int nSamples, double *regrSpec);

  static void _interpAcrossStagNotches(int nSamples,
                                       int nStaggered,
                                       int staggeredM,
                                       int staggeredN,
                                       double *regrSpec);

  void _adjustRegressionFilter(int nSamples,
                               const RadarFft &fft,
                               const double *window,
                               const RadarComplex_t *iqOrig,
                               const RadarComplex_t *iqRegr,
                               double calibratedNoise,
                               bool interpAcrossNotch,
                               RadarComplex_t *iqFiltered,
                               double &filterRatio,
                               double &spectralNoise,
                               double &spectralSnr,
                               double *specRatio);
  
  static void _compute3PtMedian(const RadarComplex_t *iq,
                                RadarComplex_t &median);

  double _conditionSnr(double snr);
  double _adjustDbzForPwrH(double dbz);
  double _adjustDbzForPwrV(double dbz);
  double _adjustZdrForPwr(double zdr);

  void _setFieldMetaData(MomentsFields &fields);

  void _allocRangeCorr();
  void _allocAtmosAttenCorr();

  // constrain a value between limits

  inline static double _constrain(double xx, double min, double max)
  {
    if (!isfinite(xx)) return min;
    if (xx < min) return min;
    if (xx > max) return max;
    return xx;
  }

};

#endif

