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
// PrecipRate.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////
//
// PrecipRate computes precip rate
//
///////////////////////////////////////////////////////////////////////

#ifndef PrecipRate_HH
#define PrecipRate_HH

#include <toolsa/TaArray.hh>
#include <radar/PrecipRateParams.hh>
#include <radar/NcarParticleId.hh>

using namespace std;

////////////////////////
// This class

class PrecipRate {
  
public:

  // constructor

  PrecipRate();

  // destructor
  
  ~PrecipRate();

  // Set processing options from params object

  void setFromParams(const PrecipRateParams &params);

  // set the wavelength in cm

  void setWavelengthCm(double cm) { _wavelengthCm = cm; }

  // Set the SNR threshold for censoring
  // The threshold in Db, gates below this threshold are censored

  void setSnrThresholdDb(double val) { _snrThreshold = val; }

  // RATE_ZH = zh_aa * (ZH ** zh_bb).

  void setZhAa(double aa) { _zh_aa = aa; }
  void setZhBb(double bb) { _zh_bb = bb; }

  // RATE_ZH_SNOW = zh_aa_snow * (ZH ** zh_bb_snow).

  void setZhAaSnow(double aa) { _zh_aa_snow = aa; }
  void setZhBbSnow(double bb) { _zh_bb_snow = bb; }

  // RATE_ZH_MIXED

  // set the correction to apply to reflectivity in the brightband
  // before computing mixed precip rate based on RATE_ZH
  // The correctoion should be negative
  
  void setBrightBandDbzCorrection(double val) {
    _brightBandDbzCorrection = val;
  }

  // RATE_Z_ZDR = zzdr_aa * (ZH ** zzdr_bb) * (ZDR ** zzdr_cc).
  
  void setZzdrAa(double aa) { _zzdr_aa = aa; }
  void setZzdrBb(double bb) { _zzdr_bb = bb; }
  void setZzdrCc(double cc) { _zzdr_cc = cc; }
  
  // RATE_KDP = sign(KDP) * kdp_aa * (|KDP| ** kdp_bb).
  
  void setKdpAa(double aa) { _kdp_aa = aa; }
  void setKdpBb(double bb) { _kdp_bb = bb; }

  // RATE_KDP_ZDR = sign(KDP) * kdpzdr_aa *
  //    (|KDP| ** kdpzdr_bb) * (ZDR ** kdpzdr_cc).
  
  void setKdpZdrAa(double aa) { _kdpzdr_aa = aa; }
  void setKdpZdrBb(double bb) { _kdpzdr_bb = bb; }
  void setKdpZdrCc(double cc) { _kdpzdr_cc = cc; }
  
  // The PID rate is based on a weighted combination of rates.
  // The weights are based on the PID interest values.
  //
  //   RATE_LIGHT_RAIN = RATE_ZH
  //   RATE_MOD_RAIN = RATE_Z_ZDR
  //   RATE_SNOW = RATE_ZH_SNOW
  //   RATE_MIXED = RATE_ZH_MIXED
  //   if (kdp > threshold && RATE_KDP is valid)
  //     RATE_HVY_RAIN = RATE_KDP
  //     RATE_HAIL = RATE_KDP
  //   else
  //     RATE_HVY_RAIN = RATE_ZZDR
  //     RATE_HAIL = RATE_ZZDR
  //   RATE = 0
  //   RATE += RATE_LIGHT_RAIN * WT_LIGHT_RAIN
  //   RATE += RATE_MOD_RAIN * WT_MOD_RAIN
  //   RATE += RATE_HVY_RAIN * WT_HVY_RAIN
  //   RATE += RATE_SNOW * WT_SNOW
  //   RATE += RATE_MIXED * WT_MIXED
  //   RATE += RATE_HAIL * WT_HAIL

  void setPidKdpThreshold(double val) { _pid_kdp_threshold = val; }

  // The HYBRID rate is a combination based on PID and partly based on
  // the BRINGI algorithm (Bringi, Williams, Thurai, May, 2009).
  // 
  // if hail or heavy rain
  //   if KDP > threhsold and rateKDP is valid
  //      use RATE_KDP
  //   else
  //      use RATE_KDP
  // else if snow/ice
  //    use RATE_ZH_SNOW
  // else if mixed precip
  //    use RATE_ZH_MIXED
  // else if rain or SLD
  //   if dBZ > theshold and KDP > threhsold
  //     if rateKDP is valid
  //       use RATE_KDP
  //     else
  //       use RATE_ZZDR
  //   else
  //      if ZDR >= threshold
  //        use RATE_Z
  //      else
  //        use RATE_ZZDR
  
  void setHybridDbzThreshold(double val) { _hybrid_dbz_threshold = val; }
  void setHybridKdpThreshold(double val) { _hybrid_kdp_threshold = val; }
  void setHybridZdrThreshold(double val) { _hybrid_zdr_threshold = val; }

  // The HIDRO rate is a combination of the other rates
  // based on the CSU-HIDRO algorithm (Cifelli et al, 2011).
  // For ice:
  //       RATE_HIDRO = MISSING
  // For hail or mixed:
  //    if KDP >= threshold and rateKdp is valid
  //       RATE_HIDRO = RATE_KDP
  //    else
  //       RATE_HIDRO = RATE_ZR
  // For rain or sld:
  //    if KDP >= threshold and DBZ >= threhold
  //      if ZDR >= threshold and rateKdpZdr is valid
  //        RATE_HIDRO = RATE_KDPZDR
  //      else if rateKdp is valid
  //        RATE_HIDRO = RATE_KDP
  //      else
  //        RATE_HIDRO = RATE_ZZDR
  //    else
  //      if ZDR >= threshold
  //        RATE_HIDRO = RATE_Z
  //      else
  //        RATE_HIDRO = RATE_ZZDR
  
  void setHidroDbzThreshold(double val) { _hidro_dbz_threshold = val; }
  void setHidroKdpThreshold(double val) { _hidro_kdp_threshold = val; }
  void setHidroZdrThreshold(double val) { _hidro_zdr_threshold = val; }

  // The BRINGI rate is a combination of the other rates
  // based on the BRINGI algorithm (Bringi, Williams, Thurai, May, 2009).
  //
  // if HAIL or RAIN/HAIL mixture and rateKdp is valid
  //     RATE_BRINGI = RATE_KDP
  // else if dBZ > theshold and KDP > threhsold and rateKdp is valid
  //    RATE_BRINGI = RATE_KDP
  // else
  //    if ZDR >= threshold
  //      RATE_BRINGI = RATE_ZZDR
  //    else
  //      RATE_BRINGI = RATE_Z
  
  void setBringiDbzThreshold(double val) { _bringi_dbz_threshold = val; }
  void setBringiKdpThreshold(double val) { _bringi_kdp_threshold = val; }
  void setBringiZdrThreshold(double val) { _bringi_zdr_threshold = val; }

  // set median filtering - default is off

  void setApplyMedianFilterToDbz(int filter_len) {
    _applyMedianFilterToDbz = true;
    _dbzMedianFilterLen = filter_len;
  }

  void setApplyMedianFilterToZdr(int filter_len) {
    _applyMedianFilterToZdr = true;
    _zdrMedianFilterLen = filter_len;
  }

  // compute precip rate
  //
  // cflag: censor flag
  // dbz, zdr, kdp: input
  // rateZ, rateZSnow, rateKdp, rateKdpZdr, rateZZdr: simple rates, output
  // rateHybrid: rate based on simple rates
  // rateHidro: rate based on CSU HIDRO algorithm
  // rateBringi: rate based on BRINGI algorithm
  
  void computePrecipRates(int nGates,
                          const double *snr,
			  const double *dbz,
                          const double *zdr,
                          const double *kdp,
			  double missingVal,
                          const NcarParticleId *pid = NULL);
  
  // get input fields after calling computePrecipRates()

  const double *getSnr() const { return _snr; }
  const double *getDbz() const { return _dbz; }
  const double *getZdr() const { return _zdr; }
  const double *getKdp() const { return _kdp; }

  // Get censor flag applied to PID during call to computePidBeam()
  // If flag is true, censoring was applied to that gate.

  const bool *getCensorFlags() const { return _cflags; }
  
  // get rate fields after calling computePrecipRates()

  const double *getRateZ() const { return _rateZ; }
  const double *getRateZSnow() const { return _rateZSnow; }
  const double *getRateZMixed() const { return _rateZMixed; }
  const double *getRateKdp() const { return _rateKdp; }
  const double *getRateKdpZdr() const { return _rateKdpZdr; }
  const double *getRateZZdr() const { return _rateZZdr; }
  const double *getRateHybrid() const { return _rateHybrid; }
  const double *getRatePid() const { return _ratePid; }
  const double *getRateHidro() const { return _rateHidro; }
  const double *getRateBringi() const { return _rateBringi; }
  
  // set minimum valid rate

  void setMinValidRate(double rate) { _min_valid_rate = rate; }

  // set maximum valid rate

  void setMaxValidRate(double rate) { _max_valid_rate = rate; }

  // set maximum valid dbz

  void setMaxValidDbz(double val) { _max_valid_dbz = val; }

protected:
private:

  PrecipRateParams _params;
  double _missingVal;
  int _nGates;

  // wavelength

  double _wavelengthCm;

  // rate coefficients

  double _zh_aa;
  double _zh_bb;
  double _zh_aa_snow;
  double _zh_bb_snow;
  double _zzdr_aa;
  double _zzdr_bb;
  double _zzdr_cc;
  double _kdp_aa;
  double _kdp_bb;
  double _kdpzdr_aa;
  double _kdpzdr_bb;
  double _kdpzdr_cc;
  double _pid_kdp_threshold;
  double _hybrid_dbz_threshold;
  double _hybrid_zdr_threshold;
  double _hybrid_kdp_threshold;
  double _hidro_dbz_threshold;
  double _hidro_zdr_threshold;
  double _hidro_kdp_threshold;
  double _bringi_dbz_threshold;
  double _bringi_zdr_threshold;
  double _bringi_kdp_threshold;

  // correction for reflectivity enhancement in bright-band
  // should be negative

  double _brightBandDbzCorrection;

  // minimum valid rate

  double _min_valid_rate;

  // maximum valid rate

  double _max_valid_rate;

  // maximum valid reflectivity for use in precip calculations

  double _max_valid_dbz;

  // median filtering

  bool _applyMedianFilterToDbz;
  int _dbzMedianFilterLen;

  bool _applyMedianFilterToZdr;
  int _zdrMedianFilterLen;

  // store input data in local arrays
  // this data is censored and filtered
  
  TaArray<double> _snr_;
  double *_snr;
  
  TaArray<double> _dbz_;
  double *_dbz;
  
  TaArray<double> _zdr_;
  double *_zdr;
  
  TaArray<double> _kdp_;
  double *_kdp;
  
  // censoring based on SNR

  double _snrThreshold;    /**< Gates with SNR less than this are censored */
  TaArray<bool> _cflags_;  /**< Array of censoring flags */
  bool *_cflags;           /**< Pointer to the array of censoring flags */
  
  // store computed fields in local arrays

  TaArray<double> _rateZ_;
  double *_rateZ;
  
  TaArray<double> _rateZSnow_;
  double *_rateZSnow;
  
  TaArray<double> _rateZMixed_;
  double *_rateZMixed;
  
  TaArray<double> _rateKdp_;
  double *_rateKdp;
  
  TaArray<double> _rateKdpZdr_;
  double *_rateKdpZdr;
  
  TaArray<double> _rateZZdr_;
  double *_rateZZdr;
  
  TaArray<double> _rateHybrid_;
  double *_rateHybrid;
  
  TaArray<double> _ratePid_;
  double *_ratePid;
  
  TaArray<double> _rateHidro_;
  double *_rateHidro;
  
  TaArray<double> _rateBringi_;
  double *_rateBringi;
  
  // functions
  
  void _allocArrays();
  void _initArrays();

  void _computeRates(double dbz, 
                     double zdrdb,
                     double kdp,
                     NcarParticleId::category_t category,
		     double &rateZh, 
                     double &rateZhSnow,
                     double &rateZhMixed,
                     double &rateKdp,
                     double &rateKdpZdr, 
                     double &rateZZdr,
		     double &rateHybrid,
		     double &rateHidro,
		     double &rateBringi);
  
  void _computePidFuzzyRate(const NcarParticleId *pid);
  void _computeHybridRates(const NcarParticleId *pid);
  double _computeRainRateRef();

};

#endif

