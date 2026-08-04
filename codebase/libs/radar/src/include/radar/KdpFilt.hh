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

#include <radar/KdpFiltParams.hh>
#include <radar/KdpFirFilt.hh>
#include <radar/KdpQuadFilt.hh>
#include <radar/RadarFft.hh>
#include <rapmath/ForsytheFit.hh>
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

  // option to limit max range
  // can be useful to avoid including the test pulse

  void setMaxRangeKm(bool state, double maxRangeKm) {
    _limitMaxRange = state;
    _maxRangeKm = maxRangeKm;
  }
  
  /**
   * Set parameters from KdpFiltParams object
   */

  void setParams(const KdpFiltParams &params);

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

  // number of gates in use

  int getNGates() const { return _nGates; }
  
  /**
   * Get dbz & SNR array after calling compute()
   * @return an array of values
   */

  const double *getSnr() const { return _snr.data(); }
  const double *getDbz() const { return _dbz.data(); }
  const double *getZdr() const { return _zdr.data(); }
  const double *getRhohv() const { return _rhohv.data(); }
  
  /**
   * Get phidp array after calling compute()
   * @return an array of unfolded phidp values
   */
  const double *getPhidp() const { return _phidp.data(); }
  const double *getPhidpMean() const { return _phidpMean.data(); }
  const double *getPhidpUnfold() const { return _phidpUnfold.data(); }
  const double *getPhidpUnfoldFilled() const { return _phidpUnfoldFilled.data(); }
  const double *getPhidpSdev() const { return _phidpSdev.data(); }
  const double *getPhidpJitter() const { return _phidpJitter.data(); }
  const double *getPhidpFilt() const { return _phidpFilt.data(); }

  /**
   * Get flag of valid gates after calling compute()
   * @return an array of flag values
   */

  const int *getValidForKdp() const { return _validForKdp.data(); }
  
  /**
   * Get kdp array after calling compute()
   * @return an array of kdp values
   */

  const double *getKdp() const { return _kdp.data(); }
  const vector<double> &getKdpVec() const { return _kdp; }

  // theoretical estimate from Z and ZDR

  const double *getKdpZZdr() const { return _kdpZZdr.data(); }
  const vector<double> &getKdpZZdrVec() const { return _kdpZZdr; }

  // self-consistency conditioned result
  
  const double *getKdpSC() const { return _kdpSC.data(); }
  const vector<double> &getKdpSCVec() const { return _kdpSC; }

  /**
   * Get phase shift on backscatter (deg) after calling compute()
   * @return an array of delta values
   */
  const double *getDelta() const { return _delta.data(); }
  const vector<double> &getDeltaVec() const { return _delta; }

  /**
   * Get attenuation correction after calling compute()
   * @return an array of correction values
   */
  const double *getDbzAttenCorr() const { return _dbzAttenCorr.data(); }
  const double *getZdrAttenCorr() const { return _zdrAttenCorr.data(); }
  const double *getDbzCorrected() const { return _dbzCorrected.data(); }
  const double *getZdrCorrected() const { return _zdrCorrected.data(); }

  /**
   * set writing of ray file
   * Ray data will be written to the specified dir
   */
  void setWriteRayFile(bool state = true,
                       string dir = "");
  
  // runs of valid phidp

  class PhidpRun {
  public:
    int ibegin;
    int iend;
    PhidpRun() {
      ibegin = 0;
      iend = 0;
    }
    PhidpRun(int begin, int end) {
      ibegin = begin;
      iend = end;
    }
    int len() const { return (iend - ibegin + 1); }
    void print(int irun, ostream &out) {
      out << "irun, ibegin, iend: "
          << irun << ","
          << ibegin << ","
          << iend << endl;
    }
  };

  const vector<PhidpRun> &getValidRuns() const { return _validRuns; }
  const vector<PhidpRun> &getGapRuns() const { return _gapRuns; }

protected:
  
private:
  
  double _missingValue; /**< Value for missing or bad data */

  // parameters

  KdpFiltParams _params;

  // time for ray

  time_t _timeSecs;
  double _timeFractionSecs;

  int _nGates;          /**< n gates in input array */
  int _nGatesStats;     /**< n gates for computing phidp stats
                         * default is 9 */
  int _nGatesStatsHalf; /**< half of _nGatesPhidpStats, truncated */
  int _nGatesPad;       /**< padding at each end for regr and fft filters */
  int _nGatesPadded;    /**< gates including padding */
  
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

  // parameters for KDP conditioned by ZZDR

  double _kdpZExpon;
  double _kdpZdrExpon;
  double _kdpZZdrCoeff;
  int _kdpZZdrMedianLen;

  // phidp state for unfolding

  class GateProps {
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

  vector<GateProps> _gateProps;
  
  bool _foldsAt90;
  double _foldVal, _foldRange;
  int _firstValidGate;
  int _lastValidGate;
  
  // runs of valid phidp

  vector<PhidpRun> _validRuns;
  vector<PhidpRun> _gapRuns;
  
  // arrays for input and computed data
  // and pointers to those arrays

  vector<int> _validForKdp;
  
  bool _snrAvailable;
  vector<double> _snr;
  
  vector<double> _dbz;
  vector<double> _dbzMedian;

  bool _rhohvAvailable;
  vector<double> _rhohv;

  bool _zdrAvailable;
  vector<double> _zdr;
  vector<double> _zdrMedian;

  vector<double> _phidp;
  vector<double> _phidpMean;
  vector<double> _phidpMeanFilled;
  vector<double> _phidpJitter;
  vector<double> _phidpSdev;
  vector<double> _phidpUnfold;
  vector<double> _phidpUnfoldFilled;
  
  vector<double> _phidpFilt;
  vector<double> _phidpFiltTrend;
  vector<double> _phidpFirFilt;
  vector<double> _phidpQuadFilt;
  vector<double> _kdpQuadFilt;
  vector<double> _phidpFftFilt;
  vector<double> _phidpRegrFilt;

  vector<double> _kdp;
  vector<double> _kdpZZdr;
  vector<double> _kdpSC;
  vector<double> _phidpSC;

  vector<double> _delta;
  vector<double> _deltaMean;

  vector<double> _dbzAttenCorr;
  vector<double> _zdrAttenCorr;
  vector<double> _dbzCorrected;
  vector<double> _zdrCorrected;

  vector<double> _xxVals;
  vector<double> _scBlock;
  
  // debug printing and writing ray files

  bool _writeRayFile;
  string _rayFileDir;

  // FIR filter
  
  KdpFirFilt _firFilt;
  
  // quadratic filter
  
  KdpQuadFilt _quadFilt;
  
  // FFT for filtering
  
  RadarFft _fft;

  //////////////////////////////////////////
  // methods

  // Set number of gates, and padding

  void _setNGates(int n) {
    _nGates = n;
    _nGatesPadded = _nGates + 2 * _nGatesPad;
  }

  // Set number of gates for computing phidp stats
  
  void _setNGatesStats(int n) {
    _nGatesStats = n;
    _nGatesStatsHalf = n / 2 + 1;
  }

  // get number of valid gates
  
  int _getNGatesMaxValid();
    
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

  void _filterPhidp();

  // worker methods
  
  void _computeKdp();
  void _computeAttenCorrection();

  /// Compute the folding range by inspecting the phidp data

  void _computeFoldingRange();
  
  // adjust input phidp for folding range

  void _adjustPhidpBeforeUnfolding(vector<double> &phidp);
  void _adjustPhidpAfterUnfolding(vector<double> &phidp);

  /// Load runs with valid gates

  int _findValidRuns();

  /// is this gate valid?

  bool _isGateValid(int igate);

  /// Initialize the properties at each gate

  void _gatePropsInit();

  /// To calculate the mean phidp, standard deviation, and jitter
  /// in phidp at a gate, using stats on the circle
  
  void _computePhidpStats(int gateNum);

  /// Compute estimated kdp from Z and ZDR using power law

  double _computeKdpFromZZdr(double dbz, double zdr);

  /// load up conditional kdp from computed kdp and kdpZZdr

  void _loadKdpSC();
  void _loadKdpSCRun(int startGate, int endGate);
  
  /// apply FIR filter
  
  void _applyFirFilter();
    
  /// filter phidp using quadratic fit
  
  void _applyQuadFilter();
    
  /// filter phidp using FFT
  
  void _applyFftFilter();
    
  /// filter phidp using regression polynomial
  
  void _applyRegrFilter();
  void _applyPhidpRegrFilt(int runNum);
  void _applyPhidpRegrFiltGlobal();
    
  /// fill phidp missing gates with noise

  void _fillPhidpMissingGates();

  /// censor non valid kdp results
  
  void _censorNonValidKdp();

  /// get quality based on rhohv
  
  double _rhohvQuality(double rhohv);
  
  /// moving mean along a vector
  
  void _movingMean(const std::vector<double>& xx,
                   size_t filtLen,
                   std::vector<double>& filt);
  
  
  /// Write ray data to a file
  
  void _writeRayDataToFile();
  double _getPlotVal(double val, double valIfMissing);

  // get attenuation parameters
  
  double _getDbzAttenCoeff();
  double _getDbzAttenExpon();
  double _getZdrAttenCoeff();
  double _getZdrAttenExpon();
  
};

#endif
