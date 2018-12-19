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
// KdpFilt.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2012
//
///////////////////////////////////////////////////////////////
//
// Compute KDP using a filtering technique to smooth the phidp
// first
//
///////////////////////////////////////////////////////////////

#ifndef KdpFilt_hh
#define KdpFilt_hh

#include <toolsa/TaArray.hh>
#include <radar/KdpFiltParams.hh>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
class KdpFiltParams;

////////////////////////
// This class

class KdpFilt {
  
public:

  /**
   * Constructor
   */
  KdpFilt();
  
  /**
   * Destructor
   */
  ~KdpFilt();

  /**
   * Set FIR filter length
   * valid lengths are 125, 30, 20, 10
   * the next length down will be used
   * @param[in] len The FIR filter length
   */
  
  // FIR filter options - lengths 125, 30, 20 and 10

  typedef enum {
    FIR_LENGTH_125,
    FIR_LENGTH_60,
    FIR_LENGTH_40,
    FIR_LENGTH_30,
    FIR_LENGTH_20,
    FIR_LENGTH_10
  } fir_filter_len_t;

  void setFIRFilterLen(fir_filter_len_t len);

  /**
   * Set number of iterations over which the filter is applied
   * to unfolded PHIDP
   * default is 2
   */
  void setNFiltIterUnfolded(int n) {
    _nFiltIterUnfolded = n;
  }
  
  /**
   * Set number of iterations over which the filter is applied
   * to constrained PHIDP
   * default is 4
   */
  void setNFiltIterCond(int n) {
    _nFiltIterCond = n;
  }
  
  /**
   * Option to use iterative filtering method.
   * If FALSE, the conditional filtering method will be used.
   * Default is false.
   * See 'setPhidpDiffThreshold'.
   */
  void setUseIterativeFiltering(bool val) {
    _useIterativeFiltering = val;
  }

  /**
   * For iterative filtering only.
   * Set threshold for difference of phidp.
   * We check the difference between the unfolded phidp
   * value and the filtered phidp value.
   * If the difference is less than this value, we use the
   * original value instead of the filtered value.
   * Default is 4.0
   * @param[in] threshold The phidp difference threshold
   */

  void setPhidpDiffThreshold(double threshold) {
    _phidpDiffThreshold = threshold;
  }

  /**
   * Set number of gates for computing phidp stats
   * default is 9
   * @param[in] number of gates for stats
   */
  void setNGatesStats(int n) {
    _nGatesStats = n;
    _nGatesStatsHalf = n / 2 + 1;
  }

  /** 
   * Apply a max range limit in km
   * if true, limit computations to _maxRangeKm
   * useful for avoiding test pulse
   * default is false
   * @param[in] state If true, limit computations to max range
   */

  // option to limit max range
  // can be useful to avoid including the test pulse

  void setMaxRangeKm(bool state, double maxRangeKm) {
    _limitMaxRange = state;
    _maxRangeKm = maxRangeKm;
  }

  /**
   * Check the SNR value in deciding
   * whether PHIDP is valid at a gate.
   * Default is false.
   */
  void checkSnr(bool val) {
    _checkSnr = val;
  }

  /**
   * Set SNR threshold - default is -6
   */
  void setSnrThreshold(double val) {
    _snrThreshold = val;
  }

  /**
   * Check the RHOHV value in deciding
   * whether PHIDP is valid at a gate.
   * Default is false.
   */
  void checkRhohv(bool val) {
    _checkRhohv = val;
  }

  /**
   * Set RHOHV threshold - default is 0.7
   */
  void setRhohvThreshold(double val) {
    _rhohvThreshold = val;
  }

  /**
   * Set max allowable sdev for phidp (deg)
   * sdev is standard deviation of phidp gate-to-gate
   * default is 20
   */
  void setPhidpSdevMax(double val) {
    _phidpSdevMax = val;
  }

  /**
   * Set max allowable jitter for phidp (deg)
   * Jitter is mean absolute change in phidp gate-to-gate
   * default is 30
   */
  void setPhidpJitterMax(double val) {
    _phidpJitterMax = val;
  }
  
  /**
   * Check the standard deviation of ZDR in deciding
   * whether PHIDP is valid at a gate.
   * Default is false.
   */
  void checkZdrSdev(bool val) {
    _checkZdrSdev = val;
  }

  /**
   * Set threshold for standard deviation of zdr
   * default is 2.5
   * @param[in] threshold The zdr std deviation threshold
   */
  void setZdrSdevMax(double val) {
    _zdrSdevMax = val;
  }

  /**
   * Set minimum valid absolute KDP value
   * default is 0.01
   * If absolute computed value is less than this, it is set to 0.
   * @param[in] val The KDP threshold
   */
  void setMinValidAbsKdp(double val) {
    _minValidAbsKdp = val;
  }

  /**
   * Set flag to indicate we should compute corrections.
   * Uses default coefficients.
   */
  void setComputeAttenCorr(bool val);
  
  /**
   * Set attenuation coefficients
   * also sets flag to indicate we should compute corrections
   */

  void setAttenCoeffs(double dbzCoeff, double dbzExpon,
                      double zdrCoeff, double zdrExpon);


  /**
   * Set KDP threshold for valid run when computing KDP using
   * Z and ZDR self-consistency
   */

  void setKdpMinForSelfConsistency(double val) { _kdpMinForSelfConsistency = val; }
  
  /**
   * Set length for Z and ZDR median filter when estimating
   * KDP from Z and ZDR
   */

  void setMedianFilterLenForKdpZZdr(int val) { _kdpZZdrMedianLen = val; }
  
  /**
   * Set parameters from KdpFiltParams object
   */

  void setFromParams(const KdpFiltParams &params);

  /**
   * Initialize the object arrays for later use.
   * Do this if you need access to the arrays, but have not yet called
   * compute(), and do not plan to do so.
   * For example, you may want to output missing fields that you have
   * not computed, but the memory needs to be there.
   */ 
  void initializeArrays(int nGates);

  /**
   * Compute KDP
   * use get methods for access to KDP array, and other
   * arrays used in the computation
   * @param[in] elevDeg The beam elevation (degrees)
   * @param[in] azDeg The beam azimuth (degrees)
   * @param[in] wavelengthCm Radar wavelength (cm)
   * @param[in] nGates The number of range gates in the beam
   * @param[in] startRangeKm - range to center of first gate
   * @param[in] gateSpacingKm - space between gate centers
   * @param[in] snr Array of SNR values, set to NULL if not available
   * @param[in] dbz Array of dbz values
   * @param[in] zdr Array of zdr values
   * @param[in] phidp Array of phidp values
   * @param[in] missingValue The value to use for missing/bad data
   * @return 0 on success, -1 on error
   */

  int compute(time_t timeSecs,
              double timeFractionSecs,
              double elevDeg,
              double azDeg,
              double wavelengthCm,
              int nGates,
              double startRangeKm,
              double gateSpacingKm,
              const double *snr,
              const double *dbz,
              const double *zdr,
              const double *rhohv,
              const double *phidp,
	      double missingValue);

  // compute PHIDP statistics
  // Computes sdev, jitter at each gate
  // Use getPhidpSdev(), getPhidpJitter() for access to results
  
  int computePhidpStats(int nGates,
                        double startRangeKm,
                        double gateSpacingKm,
                        const double *phidp,
                        double missingValue);
  
  // get fields after calling compute()

  /**
   * Get range details after calling compute()
   * @return start range and gate spacing (in km)
   */
  double getStartRangeKm() const { return _startRangeKm; }
  double getGateSpacingKm() const { return _gateSpacingKm; }

  /**
   * Get dbz & SNR array after calling compute()
   * @return an array of values
   */

  const double *getSnr() const { return _snr; }
  const double *getDbz() const { return _dbz; }
  const double *getZdr() const { return _zdr; }
  const double *getRhohv() const { return _rhohv; }
  
  /**
   * Get phidp array after calling compute()
   * @return an array of unfolded phidp values
   */
  const double *getPhidp() const { return _phidp; }
  const double *getPhidpUnfold() const { return _phidpUnfold; }
  const double *getPhidpMean() const { return _phidpMean; }
  const double *getPhidpMeanUnfold() const { return _phidpMeanUnfold; }
  const double *getPhidpSdev() const { return _phidpSdev; }
  const double *getPhidpJitter() const { return _phidpJitter; }
  const double *getPhidpFilt() const { return _phidpFilt; }
  const double *getPhidpCond() const { return _phidpCond; }
  const double *getPhidpCondFilt() const {
    return _phidpCondFilt;
  }
  const double *getPhidpAccumFilt() const {
    return _phidpAccumFilt;
  }
  const double *getZdrSdev() const { return _zdrSdev; }

  /**
   * Get flag of valid gates after calling compute()
   * @return an array of flag values
   */
  const bool *getValidForKdp() const { return _validForKdp; }
  const bool *getValidForUnfold() const { return _validForUnfold; }
  
  /**
   * Get phase shift on backscatter (deg) after calling compute()
   * @return an array of psob values
   */
  const double *getPsob() const { return _psob; }

  /**
   * Get kdp array after calling compute()
   * @return an array of kdp values
   */
  const double *getKdp() const { return _kdp; }
  const double *getKdpZZdr() const { return _kdpZZdr; }
  // self-consistency conditioned result
  const double *getKdpSC() const { return _kdpSC; }

  /**
   * Get attenuation correction after calling compute()
   * @return an array of correction values
   */
  const double *getDbzAttenCorr() const { return _dbzAttenCorr; }
  const double *getZdrAttenCorr() const { return _zdrAttenCorr; }
  const double *getDbzCorrected() const { return _dbzCorrected; }
  const double *getZdrCorrected() const { return _zdrCorrected; }

  /**
   * set debug on
   * Debug print output will go to stderr
   */
  void setDebug(bool state = true) { _debug = state; }

  /**
   * set writing of ray file
   * Ray data will be written to the specified dir
   */
  void setWriteRayFile(bool state = true,
                       string dir = "");
  
protected:
  
private:

  double _missingValue; /**< Value for missing or bad data */

  // parameters

  KdpFiltParams _params;

  // time for ray

  time_t _timeSecs;
  double _timeFractionSecs;

  // FIR filter options - lengths 125, 30, 20 and 10
  
  static const int FIR_LEN_125 = 125; /**< FIR filter len 125 */
  static const int FIR_LEN_60 = 60;   /**< FIR filter len 60 */
  static const int FIR_LEN_40 = 40;   /**< FIR filter len 40 */
  static const int FIR_LEN_30 = 30;   /**< FIR filter len 30 */
  static const int FIR_LEN_20 = 20;   /**< FIR filter len 20 */
  static const int FIR_LEN_10 = 10;   /**< FIR filter len 10 */

  static const double firCoeff_125[FIR_LEN_125+1]; /**< FIR len 125 */
  static const double firCoeff_60[FIR_LEN_60+1];   /**< FIR len 60 */
  static const double firCoeff_40[FIR_LEN_40+1];   /**< FIR len 40 */
  static const double firCoeff_30[FIR_LEN_30+1];   /**< FIR len 30 */
  static const double firCoeff_20[FIR_LEN_20+1];   /**< FIR len 20 */
  static const double firCoeff_10[FIR_LEN_10+1];   /**< FIR len 10 */
  
  int _firLength;          /**< The length of the current FIR array */
  int _firLenHalf;         /**< Half the length of the current FIR array */
  const double *_firCoeff; /**< The length of the current FIR array */

  int _nFiltIterUnfolded;  /**< Number of times the filter is
                            * iteratively applied to unfolded phidp */
  
  int _nFiltIterCond;  /**< Number of times the filter is
                        * iteratively applied to filtered constrained */
  
  bool _useIterativeFiltering;
  double _phidpDiffThreshold;

  int _nGates;          /**< n gates in input array */
  int _nGatesStats;     /**< n gates for computing phidp stats
                         * default is 9 */
  int _nGatesStatsHalf; /**< half of _nGatesPhidpStats, truncated */

  // option to limit max range
  // can be useful to avoid including the test pulse
  
  bool _limitMaxRange;
  double _maxRangeKm;

  // wavelength

  double _wavelengthCm;

  // start range and gate spacing

  double _startRangeKm;
  double _gateSpacingKm;

  // beam pointing

  double _elevDeg;         /**< The current beam elevation */
  double _azDeg;           /**< The current beam azimuth */

  // thresholds for filtering

  bool _checkSnr; /**< should we check SNR? */
  double _snrThreshold;

  bool _checkRhohv; /**< should we check RHOHV? */
  double _rhohvThreshold;

  bool _checkZdrSdev; /**< should we check SDEV of ZDR? */
  double _zdrSdevMax;  /**< Max sdev of zdr for valid phidp */

  double _phidpJitterMax; /**< Max jitter for valid phidp */
  double _phidpSdevMax; /**< Max sdev for valid phidp */

  // min valid KDP, default 0.05
  // absolute values less than this are set to 0

  double _minValidAbsKdp;

  // phidp state for unfolding

  class GateState {
  public:
    void init(double missingValue) {
      missing = true;
      phidp = missingValue;
      xx = 0.0;
      yy = 0.0;
      meanxx = 0.0;
      meanyy = 0.0;
      distFromPrev = 0.0;
      phidpMean = missingValue;
      phidpSdev = missingValue;
      phidpJitter = missingValue;
    }
    bool missing;
    double phidp;
    double xx;
    double yy;
    double meanxx;
    double meanyy;
    double distFromPrev;
    double phidpMean;
    double phidpSdev;
    double phidpJitter;
  };

  TaArray<GateState> _gateStates_;
  GateState *_gateStates;
  
  bool _foldsAt90;
  double _foldVal, _foldRange;
  int _firstValidGate;
  int _lastValidGate;
  
  // runs of valid phidp

  class PhidpRun {
  public:
    int ibegin;
    int iend;
    double phidpBegin;
    double phidpEnd;
    PhidpRun() {
      ibegin = 0;
      iend = 0;
      phidpBegin = 0.0;
      phidpEnd = 0.0;
    }
    PhidpRun(int begin, int end) {
      ibegin = begin;
      iend = end;
      phidpBegin = 0.0;
      phidpEnd = 0.0;
    }
    int len() const { return (iend - ibegin + 1); }
    void print(int irun, ostream &out) {
      out << "irun, ibegin, iend: "
          << irun << ","
          << ibegin << ","
          << iend << endl;
    }
  };

  vector<PhidpRun> _validRuns;
  vector<PhidpRun> _gaps;
  
  // arrays for input and computed data
  // and pointers to those arrays

  int _arrayExtra;
  int _arrayLen;
  
  bool _snrAvailable;
  TaArray<double> _snr_;
  double *_snr;
  
  TaArray<double> _dbz_;
  double *_dbz;

  TaArray<double> _dbzMedian_;
  double *_dbzMedian;

  TaArray<double> _dbzMax_;
  double *_dbzMax;
  
  bool _rhohvAvailable;
  TaArray<double> _rhohv_;
  double *_rhohv;

  bool _zdrAvailable;
  TaArray<double> _zdr_;
  double *_zdr;

  TaArray<double> _zdrSdev_;
  double *_zdrSdev;

  TaArray<double> _zdrMedian_;
  double *_zdrMedian;

  TaArray<double> _phidp_;
  double *_phidp;
  
  TaArray<double> _phidpMean_;
  double *_phidpMean;
  
  TaArray<double> _phidpMeanValid_;
  double *_phidpMeanValid;
  
  TaArray<double> _phidpJitter_;
  double *_phidpJitter;
  
  TaArray<double> _phidpSdev_;
  double *_phidpSdev;
  
  TaArray<double> _phidpMeanUnfold_;
  double *_phidpMeanUnfold;
  
  TaArray<double> _phidpUnfold_;
  double *_phidpUnfold;
  
  TaArray<double> _phidpFilt_;
  double *_phidpFilt;
  
  TaArray<double> _phidpCond_;
  double *_phidpCond;
  
  TaArray<double> _phidpCondFilt_;
  double *_phidpCondFilt;
  
  TaArray<double> _phidpAccumFilt_;
  double *_phidpAccumFilt;
  
  TaArray<bool> _validForKdp_;
  bool *_validForKdp;
  
  TaArray<bool> _validForUnfold_;
  bool *_validForUnfold;
  
  TaArray<double> _kdp_;
  double *_kdp;

  TaArray<double> _kdpZZdr_;
  double *_kdpZZdr;

  TaArray<double> _kdpSC_;
  double *_kdpSC;

  TaArray<double> _psob_;
  double *_psob;

  TaArray<double> _dbzAttenCorr_;
  double *_dbzAttenCorr;

  TaArray<double> _zdrAttenCorr_;
  double *_zdrAttenCorr;

  TaArray<double> _dbzCorrected_;
  double *_dbzCorrected;

  TaArray<double> _zdrCorrected_;
  double *_zdrCorrected;

  // Z and ZDR attenuation correction

  bool _doComputeAttenCorr;
  bool _attenCoeffsSpecified;
  double _dbzAttenCoeff;
  double _dbzAttenExpon;
  double _zdrAttenCoeff;
  double _zdrAttenExpon;
  
  // debug printing and writing ray files

  bool _debug;
  bool _writeRayFile;
  string _rayFileDir;

  // parameters for KDP conditioned by ZZDR

  double _kdpZExpon;
  double _kdpZdrExpon;
  double _kdpZZdrCoeff;
  double _kdpMinForSelfConsistency;
  int _kdpZZdrMedianLen;

  // methods
 
  /**
   * Initialize local arrays and copy input data for filtering,
   * manipulation, etc.
   */ 

  void _initArrays(const double *snr,
                   const double *dbz,
                   const double *zdr,
                   const double *rhohv,
                   const double *phidp,
                   int nGatesMaxValid);
  
  /**
   * Load up conditioned phidp array, by interpolating
   * phidp between valid runs
   */
  int _unfoldPhidp();

  /// filter the unfolded phidp array and compute kdp
  
  void _computeKdp();
  void _copyArray(double *array, const double *vals);
  void _copyArrayCond(double *array, const double *vals,
                      const double *original);
  void _padArray(double *array);
  void _loadKdp();
  void _loadPhidpAccumFilt(const double *phidp, double *accum);
  void _computeAttenCorrection();
  void _applyFirFilter(const double *in, double *out);
  double _getFirFilterGain();
  void _computeDbzMax();
  void _computePhidpConditioned();

  /// Compute the folding range by inspecting the phidp data

  void _computeFoldingRange();
  
  /// Load runs with valid gates

  int _findValidRuns();

  /// is this gate valid?

  bool _isGateValid(int igate);

  /// Initialize the state at each gate

  void _gateStatesInit();

  /// To calculate the mean phidp, standard deviation, and jitter
  /// in phidp at a gate, using stats on the circle
  
  void _computePhidpStats(int gateNum);

  // compute sdev of zdr

  void _computeZdrSdev(int igate);

  /// Write ray data to a file
  
  void _writeRayDataToFile();
  double _getPlotVal(double val, double valIfMissing);

  /// Compute estimated kdp from Z and ZDR using power law

  double _computeKdpFromZZdr(double dbz, double zdr);

  /// load up conditional kdp from computed kdp and kdpZZdr

  void _loadKdpSC();
  void _loadKdpSCRun(int startGate, int endGate);

};

#endif

