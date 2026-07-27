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
// KdpFirFilt.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2026
//
///////////////////////////////////////////////////////////////
//
// Apply FIR filter to phidp, for computing KDP
//
///////////////////////////////////////////////////////////////

#ifndef KdpFirFilt_hh
#define KdpFirFilt_hh

#include <radar/KdpFirFilt.hh>
#include <vector>
using namespace std;

////////////////////////
// This class

class KdpFirFilt {
  
public:

  /**
   * Constructor
   */

  KdpFirFilt();
  
  /**
   * Destructor
   */

  ~KdpFirFilt();

  // phidp feature length for filtering
  // also selects the FIR length
  
  void setFeatureLength(double featureLengthKm, double gateSpacingKm);
  
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
   * default is 2
   */
  
  void setNFiltIterUnfolded(int n) {
    _nFiltIterUnfolded = n;
  }
  
  /**
   * Set number of iterations over which the filter is applied
   * to mitigate phase shift on backscatter
   * default is 4
   */
  void setNFiltIterPsob(int n) {
    _nFiltIterPsob = n;
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
   * Set number of gates, and padding
   */

  void setNGates(int n) {
    _nGates = n;
    _nGatesPadded = _nGates + 2 * _nGatesPad;
  }

  /**
   * Initialize the object arrays for later use.
   * Do this if you need access to the arrays, but have not yet called
   * compute(), and do not plan to do so.
   * For example, you may want to output missing fields that you have
   * not computed, but the memory needs to be there.
   */ 
  void initializeArrays(int nGates);

  /////////////////////////////////////////////
  // filter the input PHIDP array
  
  void filterPhidp(const vector<double> &phidp);

  // get fields after calling compute()
  
  /**
   * Get range details after calling compute()
   * @return start range and gate spacing (in km)
   */

  double getGateSpacingKm() const { return _gateSpacingKm; }
  double getFeatureLengthKm() const { return _featureLengthKm; }
  int getNGatesFeature() const { return _nGatesFeature; }

  /**
   * Get phidp array after calling compute()
   * @return an array of unfolded phidp values
   */
  const double *getPhidp() const { return _phidp; }
  const double *getPhidpFilt() const { return _phidpFilt; }
  const double *getPhidpCond() const { return _phidpCond; }
  const double *getPhidpCondFilt() const {
    return _phidpCondFilt;
  }

  /**
   * set debug on
   * Debug print output will go to stderr
   */
  void setDebug(bool state = true) { _debug = state; }

protected:
  
private:

  double _missingValue; /**< Value for missing or bad data */
  
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
  
  int _nFiltIter;  /**< Number of times the filter is
                    * iteratively applied to filter phidp */
  
  int _nFiltIterPsob;  /**< Number of times the filter is
                        * iteratively applied to mitigate backscatter phase shift */
  
  bool _useIterativeFiltering; /* for phase shift on backscatter removal */
  double _phidpDiffThreshold; /* for phase shift on backscatter removal */
  
  int _nGates;          /**< n gates in input array */
  int _nGatesPad;       /**< padding at each end for regr and fft filters */
  int _nGatesPadded;    /**< gates including padding */
  
  // start range and gate spacing

  double _gateSpacingKm;

  // nominal length of a feature in PHIDP
  
  double _featureLengthKm;
  int _nGatesFeature;
  
  // arrays for input and computed data
  // and pointers to those arrays

  vector<double> _phidp_;
  double *_phidp;
  
  vector<double> _phidpFilt_;
  double *_phidpFilt;
  
  vector<double> _phidpCond_;
  double *_phidpCond;
  
  vector<double> _phidpCondFilt_;
  double *_phidpCondFilt;
  
  // debug printing

  bool _debug;

  // worker methods
  
  void _applyIterativeFir(double *out, const double *in, int nIterations);
  void _applyIterativeFirCond(double *out, const double *in, int nIterations);
  void _copyArray(double *out, const double *in);
  void _copyArrayCond(double *out, const double *in, const double *original);
  void _padArray(double *array);

  void _applyFirFilter(double *out, const double *in);
  double _getFirFilterGain();

  /// is this gate valid?

  bool _isGateValid(int igate);
  
};

#endif

