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
// KdpBringi.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////

/**
 * @file KdpBringi.hh
 * @brief Kdp for SBand - based on Bringi code
 * @class KdpBringi
 */

#ifndef KdpBringi_hh
#define KdpBringi_hh

#include <toolsa/TaArray.hh>
#include <string>
#include <vector>
using namespace std;

////////////////////////
// This class

class KdpBringi {
  
public:

  /**
   * Constructor
   */
  KdpBringi();
  
  /**
   * Destructor
   */
  ~KdpBringi();

  /**
   * Set the beam wavelength
   * @param[in] cm The wavelength, in cm
   */
  void setWavelengthCm(double cm) { _wavelengthCm = cm; }

  /**
   * Set median filtering for phidp
   * default is off, median filter len is 5
   * @param[in] filter_len The numbere of gates to use for the median filter
   */
  void setApplyMedianFilterToPhidp(int filter_len) {
    _applyMedianFilterToPhidp = true;
    _phidpMedianFilterLen = filter_len;
  }

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
   * Set threshold for SNR.
   * If the SNR is below this value, we mark this gate as
   * not suitable for using the phidp value.
   * default is 0.0
   * @param[in] threshold The SNR threshold for good PHIDP.
   */
  void setSnrThreshold(double threshold) {
    _snrThreshold = threshold;
  }

  /**
   * Set threshold for LDR.
   * If the LDR is above this value, we mark this gate as
   * not suitable, becaue second trip is present.
   * default is 0.0
   * @param[in] threshold The LDR threshold for good PHIDP.
   */
  void setLdrThreshold(double threshold) {
    _ldrThreshold = threshold;
  }

  /**
   * Set threshold for difference of phidp
   * This tests the difference between the unfolded phidp
   * value and the filtered phidp value.
   * If the difference exceeds this value, we use the original value
   * instead of the filtered value.
   * default is 4.0
   * @param[in] threshold The phidp difference threshold
   */
  void setPhidpDiffThreshold(double threshold) {
    _phidpDiffThreshold = threshold;
  }

  /**
   * Set threshold for standard deviation of phidp
   * default is 12.0
   * @param[in] threshold The phidp std deviation threshold
   */
  void setPhidpSdevThreshold(double threshold) {
    _phidpSdevThreshold = threshold;
  }

  /**
   * Set threshold for standard deviation of zdr
   * default is 1.8
   * @param[in] threshold The zdr std deviation threshold
   */
  void setZdrSdevThreshold(double threshold) {
    _zdrSdevThreshold = threshold;
  }

  /**
   * Set threshold for rhohv in weather
   * default is 0.7
   * @param[in] threshold The rhohv threshold
   */
  void setRhohvWxThreshold(double threshold) {
    _rhohvWxThreshold = threshold;
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
   * Set minimum valid absolute KDP value
   * default is 0.01
   * If absolute computed value is less than this, it is set to 0.
   * @param[in] val The KDP threshold
   */
  void setMinValidAbsKdp(double val) {
    _minValidAbsKdp = val;
  }

  /**
   * Compute KDP
   * use get methods for access to KDP array, and other
   * arrays used in the computation
   * @param[in] elev The beam elevation (degrees)
   * @param[in] az The beam azimuth (degrees)
   * @param[in] nGates The number of range gates in the beam
   * @param[in] range Array of gate ranges (in km)
   * @param[in] dbz Array of reflectivity values
   * @param[in] zdr Array of zdr values
   * @param[in] phidp Array of phidp values
   * @param[in] rhohv Array of rhohv values
   * @param[in] snr Array of snr values
   * @param[in] missingValue The value to use for missing/bad data
   * @param[in] ldr - array of ldr values, NULL if LDR not available
   * @return 0 on success, -1 on error
   */
  int compute(double elev,
              double az,
              int nGates,
              const double *range,
              const double *dbz,
              const double *zdr,
              const double *phidp,
              const double *rhohv,
              const double *snr,
	      double missingValue,
              const double *ldr = NULL);

  // get fields after calling compute()

  /**
   * Get range array after calling compute()
   * @return an array of gate ranges (in km)
   */
  const double *getRange() const { return _range; }

  /**
   * Get dbz array after calling compute()
   * @return an array of reflectivity values
   */
  const double *getDbz() const { return _dbz; }

  /**
   * Get zdr array after calling compute()
   * @return an array of zdr values
   */
  const double *getZdr() const { return _zdr; }

  /**
   * Get phidp array after calling compute()
   * @return an array of phidp values
   */
  const double *getPhidp() const { return _phidp; }

  /**
   * Get conditioned phidp array after calling compute()
   * @return an array of conditioned phidp values
   */
  const double *getPhidpCond() const { return _phidpCond; }

  /**
   * Get filtered phidp array after calling compute()
   * @return an array of filtered phidp values
   */
  const double *getPhidpFilt() const { return _phidpFilt; }

  /**
   * Get phase shift on backscatter (deg) after calling compute()
   * @return an array of psob values
   */
  const double *getPsob() const { return _psob; }

  /**
   * Get rhohv array after calling compute()
   * @return an array of rhohv values
   */
  const double *getRhohv() const { return _rhohv; }

  /**
   * Get snr array after calling compute()
   * @return an array of snr values
   */
  const double *getSnr() const { return _snr; }

  /**
   * Get ldr array after calling compute()
   * @return an array of ldr values
   */
  const double *getLdr() const { return _ldr; }

  /**
   * Get phidp std deviation array after calling compute()
   * @return an array of phidp std deviation values
   */
  const double *getSdPhidp() const { return _sdphidp; }

  /**
   * Get zdr std deviation array after calling compute()
   * @return an array of zdr std deviation values
   */
  const double *getSdZdr() const { return _sdzdr; }

  /**
   * Get flag of good gates after calling compute()
   * @return an array of flag values
   */
  const bool *getGoodFlag() const { return _goodFlag; }
  
  /**
   * Get kdp array after calling compute()
   * @return an array of kdp values
   */
  const double *getKdp() const { return _kdp; }

protected:
  
private:

  // static const int EXTRA_GATES = 500; /**< for padding at each end of arrays */
  // static const int EXTRA_HALF = 250;  /**< for padding at each end of arrays */

  // FIR filter options - lengths 125, 30, 20 and 10

  static const int FIR_LEN_125 = 125; /**< FIR filter length constant */
  static const int FIR_LEN_60 = 60;   /**< FIR filter length constant */
  static const int FIR_LEN_40 = 40;   /**< FIR filter length constant */
  static const int FIR_LEN_30 = 30;   /**< FIR filter length constant */
  static const int FIR_LEN_20 = 20;   /**< FIR filter length constant */
  static const int FIR_LEN_10 = 10;   /**< FIR filter length constant */

  static const double firGain_125; /**< FIR gain constant */
  static const double firGain_60;  /**< FIR gain constant */
  static const double firGain_40;  /**< FIR gain constant */
  static const double firGain_30;  /**< FIR gain constant */
  static const double firGain_20;  /**< FIR gain constant */
  static const double firGain_10;  /**< FIR gain constant */

  static const double firCoeff_125[FIR_LEN_125+1]; /**< array of FIR coeffients for specified length */
  static const double firCoeff_60[FIR_LEN_60+1];   /**< array of FIR coeffients for specified length */
  static const double firCoeff_40[FIR_LEN_40+1];   /**< array of FIR coeffients for specified length */
  static const double firCoeff_30[FIR_LEN_30+1];   /**< array of FIR coeffients for specified length */
  static const double firCoeff_20[FIR_LEN_20+1];   /**< array of FIR coeffients for specified length */
  static const double firCoeff_10[FIR_LEN_10+1];   /**< array of FIR coeffients for specified length */

  int _firLength;          /**< The length of the current FIR array */
  int _firLenHalf;         /**< Half the length of the current FIR array */
  const double *_firCoeff; /**< The length of the current FIR array */
  double _firGain;         /**< The current FIR gain value */

  int _arrayExtra;
  int _arrayLen;

  static const int N_GOOD = 10; /**< Length (in gates) of phidp subset where the 
                                ** std dev is less than the threshold */
  static const int N_BAD = 5;   /**< After this many gates have been labeled 'bad',
                                ** insert a test to preserve hail/BB signal */
  static const int N_MEAN = 5;  /**< Number of gates for computing mean for phidp */

  double _missingValue; /**< Value for missing or bad data */
  double _wavelengthCm; /**< The beam wavelength (in cm) */
  double _elev;         /**< The current beam elevation */
  double _az;           /**< The current beam azimuth */

  // thresholds for filtering phidp

  double _snrThreshold;         /**< Threshold for SNR */
  double _ldrThreshold;         /**< Threshold for LDR */
  double _phidpDiffThreshold;   /**< Threshold for phidp difference */
  double _phidpSdevThreshold;   /**< Threshold for sdev of phidp */
  double _zdrSdevThreshold;     /**< Threshold for sdev of zdr */
  double _rhohvWxThreshold;     /**< Threshold for rhohv in weather */
  
  // median filtering

  bool _applyMedianFilterToPhidp;  /**< If true, filter phidp using median filter */
  int _phidpMedianFilterLen;       /**< Number of gates to include when finding median phidp value */

  // option to limit max range
  // can be useful to avoid including the test pulse

  bool _limitMaxRange;
  double _maxRangeKm;
  int _nGatesMaxRange;

  // min valid KDP
  // absolute values less than this are set to 0

  double _minValidAbsKdp;

  // store input data in local arrays
  // this data is censored and filtered
  
  TaArray<double> _range_;     /**< Local array to hold censored and filtered range values */
  double *_range;              /**< Pointer to local array of range values */
  
  TaArray<double> _dbz_;       /**< Local array to hold censored and filtered dbz values */
  double *_dbz;                /**< Pointer to local array of dbz values */
  
  TaArray<double> _zdr_;       /**< Local array to hold censored and filtered zdr values */
  double *_zdr;                /**< Pointer to local array of zdr values */
  
  TaArray<double> _phidp_;     /**< Local array to hold censored and filtered phidp values */
  double *_phidp;              /**< Pointer to local array of phidp values */
  
  TaArray<double> _phidpCond_; /**< Local array to hold conditioned phidp values */
  double *_phidpCond;          /**< Pointer to local array of conditioned phidp values */
  
  TaArray<double> _phidpFilt_; /**< Local array to hold phidp Adaptive Filtered values */
  double *_phidpFilt;          /**< Pointer to local array of Adaptive Filtered phidp values */
  
  TaArray<double> _psob_;      /**< Local array to hold phase shift on backscatter */
  double *_psob;               /**< PSOB = (phidp - phidpFilt) */
  
  TaArray<double> _rhohv_;     /**< Local array to hold censored and filtered rhohv values */
  double *_rhohv;              /**< Pointer to local array of rhohv values */
  
  TaArray<double> _snr_;       /**< Local array to hold censored and filtered snr values */
  double *_snr;                /**< Pointer to local array of snr values */

  TaArray<double> _ldr_;       /**< Local array to hold censored and filtered ldr values */
  double *_ldr;                /**< Pointer to local array of ldr values */

  TaArray<double> _sdphidp_;   /**< Local array to hold std dev of phidp values */
  double *_sdphidp;            /**< Pointer to local array of std dev of phidp values */

  TaArray<double> _sdzdr_;     /**< Local array to hold std dev of zdr values */
  double *_sdzdr;              /**< Pointer to local array of std dev of zdr values */
  
  TaArray<bool> _goodFlag_;    /**< Local array to hold flag for good gates */
  bool *_goodFlag;             /**< Pointer to local array of good flags */

  TaArray<double> _kdp_;      /**< Local array to hold calculated kdp values */
  double *_kdp;               /**< Pointer to local array of calculated kdp values */

  // working arrays

  TaArray<double> _xxx_, _yyy_, _zzz_;
  double *_xxx, *_yyy, *_zzz;

  // runs of good phidp

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
  };

  vector<PhidpRun> _goodRuns;
  PhidpRun *_runFirst;
  PhidpRun *_runLast;
  
  // methods
 
  /**
   * Initialize local arrays and copy input data for filtering, manipulation, etc.
   * @param[in] nGates The number of gates represented in each data array
   * @param[in] range Pointer to an array of gate ranges
   * @param[in] dbz Pointer to an array of dbz values 
   * @param[in] zdr Pointer to an array of zdr values
   * @param[in] phidp Pointer to an array of phidp values
   * @param[in] rhohv Pointer to an array of rhohv values
   * @param[in] snr Pointer to an array of snr values 
   */ 
  void _initArrays(int nGates,
                   const double *range,
                   const double *dbz,
                   const double *zdr,
                   const double *phidp,
                   const double *rhohv,
                   const double *snr,
                   const double *ldr);

  
  /**
   * Find runs with good gates
   */
  void _findGoodRuns();

  /**
   * This is Yanting's modified unfold code
   * @param[in] nGates The number of gates in the beam
   */
  void _loadConditionedPhidp(int nGates);

  /**
   * Load up conditioned phidp array, by interpolating
   * phidp between good runs
   */
  void _unfoldPhidp(int nGates);

  /**
   * Calculate the mean of the array y(i) (i=0,...,n-1).
   * Checks that values are in range -1.e35 to +1.e35
   * @param[in] y Pointer to an array of values to average
   * @param[in] n The number of values in the array
   * @return The mean value
   */
  double _mean(const double *y, int n);

  /**
   * Calculate the mean (ymn) and standard deviation (sd, or,
   * mean square root, msr) of the array y(i) (i=1,...,n).
   * @param[out] ymn The mean of the array
   * @param[out] The standard deviation of the array
   * @param[in] y Pointer to an array of values to find the std dev of
   * @param[in] n The number of values in the array
   */
  void _msr(double &ymn, double &sd, const double *y, int n);

  /**
   * This is a Linear Least Square Estimate subroutine to fit a linear
   * equation for (xi,yi) (i=1,...,n), so that
   *                         yi = a * xi + b
   * INPUTs: x(i), y(i), n, (i=1,...,n ).
   * OUTPUTs: a ( slope ), b ( intercept ).
   * @param[out] a The slope of the line fitting the points
   * @param[out] b The intercept  of the line fitting the points
   * @param[in] y Pointer to an array of values to fit
   * @param[in] n The number of values in the array
   */   
  void _lse(double &a, double &b, const double *x, const double *y, int n);

};

#endif

