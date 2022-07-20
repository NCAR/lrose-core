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
// IpsMoments.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2019
//
///////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
//
// IpsMoments computes moments at a gate
//
///////////////////////////////////////////////////////////////

#ifndef IpsMoments_hh
#define IpsMoments_hh

#include <string>
#include <radar/IpsTsInfo.hh>
#include <radar/IpsTsCalib.hh>
#include <radar/IpsMomFields.hh>
#include <radar/RadarComplex.hh>
#include <radar/RadarFft.hh>
#include <radar/AtmosAtten.hh>
#include <rapformats/DsRadarCalib.hh>
class IpsTsPulse;
using namespace std;

////////////////////////
// This class

class IpsMoments {
  
public:

  // receiver channel identification

  enum class channel_t {
    CHANNEL_HC,
    CHANNEL_VC,
    CHANNEL_HX,
    CHANNEL_VX,
  };

  // default constructor

  IpsMoments();

  // constructor with max gates specified
  
  IpsMoments(int max_gates,
              bool debug = false,
              bool verbose = false);
  
  // destructor
  
  ~IpsMoments();

  // set debugging

  void setDebug(bool state) { _debug = state; }
  void setVerbose(bool state) { _verbose = state; }

  // set max number of gates

  void setMaxGates(int maxGates);

  // initialize based on OpsInfo
  
  void init(double prt, const IpsTsInfo &opsInfo);
  
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
                   const IpsTsInfo &opsInfo);
  
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
  
  void setCalib(const IpsTsCalib &calib);

  // set the estimated noise power based on analysis of the data
  // note that noise is power at the DRX, before subtraction of receiver gain

  void setNoiseDbmHc(double val);
  void setNoiseDbmVc(double val);
  void setNoiseDbmHx(double val);
  void setNoiseDbmVx(double val);

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

  enum class spectrum_width_method_t {
    R0R1,
    R1R2,
    HYBRID
  };

  void setSpectrumWidthMethod(spectrum_width_method_t method) {
    _widthMethod = method;
  }

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
  // Compute moments, for pulses from a single channel
  // store results in fields object
  
  void computeMoments(vector<const IpsTsPulse *> &pulses,
                      RadarComplex_t *iq0,
                      int gateNum,
                      IpsMomFields &fields);

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

  // compute percentile power value in spectrum
  
  double computePowerPercentile(int nSamples,
                                double *powerSpec,
                                double percentile);

  // get the calibrated noise power given the channel
  
  double getCalNoisePower(channel_t channel);

  // get the noise power in use given the channel
  
  double getNoisePower(channel_t channel);

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
                           IpsMomFields &fields);
  
  void singlePolVNoisePrep(double lag0_vc,
                           RadarComplex_t lag1_vc,
                           IpsMomFields &fields);
  
  void dpAltHvCoOnlyNoisePrep(double lag0_hc,
                              double lag0_vc,
                              RadarComplex_t lag2_hc,
                              RadarComplex_t lag2_vc,
                              IpsMomFields &fields);
  
  void dpAltHvCoCrossNoisePrep(double lag0_hc,
                               double lag0_hx,
                               double lag0_vc,
                               double lag0_vx,
                               RadarComplex_t lag2_hc,
                               RadarComplex_t lag2_vc,
                               IpsMomFields &fields);
  
  void dpSimHvNoisePrep(double lag0_hc,
                        double lag0_vc,
                        RadarComplex_t lag1_hc,
                        RadarComplex_t lag1_vc,
                        IpsMomFields &fields);
  
  void dpHOnlyNoisePrep(double lag0_hc,
                        double lag0_vx,
                        RadarComplex_t lag1_hc,
                        IpsMomFields &fields);
  
  void dpVOnlyNoisePrep(double lag0_vc,
                        double lag0_hx,
                        RadarComplex_t lag1_vc,
                        IpsMomFields &fields);
  
  void singlePolHStagPrtNoisePrep(RadarComplex_t *iqhc,
                                  RadarComplex_t *iqhcShort,
                                  RadarComplex_t *iqhcLong,
                                  IpsMomFields &fields);
  
  void dpSimHvStagPrtNoisePrep(RadarComplex_t *iqhc,
                               RadarComplex_t *iqvc,
                               RadarComplex_t *iqhcShort,
                               RadarComplex_t *iqvcShort,
                               RadarComplex_t *iqhcLong,
                               RadarComplex_t *iqvcLong,
                               IpsMomFields &fields);


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

  // noise power for moments computations
  
  double _noisePowerHc;
  double _noisePowerHx;
  double _noisePowerVc;
  double _noisePowerVx;

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

  // working summations for covariances

  ScalarSum _sum_lag0_hc;
  ScalarSum _sum_lag0_vc;
  ScalarSum _sum_lag0_hx;
  ScalarSum _sum_lag0_vx;

  ComplexSum _sum_lag0_vchx; // lag 0 v-co to h-x covariance
  ComplexSum _sum_lag0_hcvx; // lag 0 h-co to v-x  covariance
  ComplexSum _sum_lag1_hc;
  ComplexSum _sum_lag1_vc;
  ComplexSum _sum_lag1_hcvc; // lag 1 h-co to v-co covariance
  ComplexSum _sum_lag1_vchc; // lag 1 v-co to h-co covariance
  ComplexSum _sum_lag1_vxhx; // lag 1 v-x to h-x covariance
  ComplexSum _sum_lag2_hc;
  ComplexSum _sum_lag2_vc;
  ComplexSum _sum_lag3_hc;
  ComplexSum _sum_lag3_vc;
  ComplexSum _sumrvvhh0;

  ScalarSum _sum_lag0_hc_short; // lag 0 (power) h co-polar short prt
  ScalarSum _sum_lag0_vc_short; // lag 0 (power) v co-polar short prt
  ScalarSum _sum_lag0_hc_long; // lag 0 (power) h co-polar long prt
  ScalarSum _sum_lag0_vc_long; // lag 0 (power) v co-polar long prt

  ComplexSum _sum_lag1_hc_long; // lag 1 h-co long-to-long
  ComplexSum _sum_lag1_vc_long; // lag 1 v-co long-to-long
  ComplexSum _sum_lag1_hc_short; // lag 1 h-co short-to-short
  ComplexSum _sum_lag1_vc_short; // lag 1 v-co short-to-short
  ComplexSum _sum_lag1_hc_short_to_long; // lag 1 h-co short-to-long
  ComplexSum _sum_lag1_vc_short_to_long; // lag 1 v-co short-to-long
  ComplexSum _sum_lag1_hc_long_to_short; // lag 1 h-co long-to-short
  ComplexSum _sum_lag1_vc_long_to_short; // lag 1 v-co long-to-short

  ComplexSum _sum_rvvhh0_long;
  ComplexSum _sum_rvvhh0_short;

  // functions

  void _init();

  void _initForCovar();

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

  static void _compute3PtMedian(const RadarComplex_t *iq,
                                RadarComplex_t &median);

  double _conditionSnr(double snr);
  double _adjustDbzForPwrH(double dbz);
  double _adjustDbzForPwrV(double dbz);
  double _adjustZdrForPwr(double zdr);

  void _setFieldMetaData(IpsMomFields &fields);

  void _allocRangeCorr();
  void _allocAtmosAttenCorr();

  // constrain a value between limits

  inline static double _constrain(double xx, double min, double max)
  {
    if (!std::isfinite(xx)) return min;
    if (xx < min) return min;
    if (xx > max) return max;
    return xx;
  }

};

#endif

