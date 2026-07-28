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
#include <limits>
#include <vector>
#include <cassert>
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
  
  /////////////////////////////////////////////
  // apply the filter, save in filt.
  // filt must have the same size as unfilt.
  
  void applyFilter(const vector<double> &unfilt,
                   vector<double> &filt,
                   int nIterations);
  
  ///////////////////////////////////////////////
  // filter array for phase shift on backscatter
  // filt must have the same size as unfilt.
  
  void applyPsobFilter(const vector<double> &unfilt,
                       vector<double> &filt,
                       int nIterations,
                       double diffThreshold);

  // get filter-specific details

  int getFirLength() const { return _firLength; }
  double getGateSpacingKm() const { return _gateSpacingKm; }
  double getFeatureLengthKm() const { return _featureLengthKm; }
  int getNGatesFeature() const { return _nGatesFeature; }

  // missing value
  
  static constexpr double missingValue() {
    return std::numeric_limits<double>::quiet_NaN();
  }
  
protected:
  
private:

  // FIR filter options - lengths 125, 30, 20 and 10
  
  typedef enum {
    FIR_LENGTH_125,
    FIR_LENGTH_60,
    FIR_LENGTH_40,
    FIR_LENGTH_30,
    FIR_LENGTH_20,
    FIR_LENGTH_10
  } fir_filter_len_t;
  
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
  
  int _nGates;          /**< n gates in input array */
  int _nGatesPad;       /**< padding at each end for regr and fft filters */
  int _nGatesPadded;    /**< gates including padding */
  
  // nominal length of a feature in PHIDP
  
  double _featureLengthKm;
  double _gateSpacingKm;
  int _nGatesFeature;
  
  // set number of gates
  
  void _setNGates(int n);

  // Set FIR filter length
  // valid lengths are 125, 30, 20, 10
  // the next length down will be used
  // FIR filter options - lengths 125, 30, 20 and 10

  void _setFilterLen(fir_filter_len_t len);

  // worker methods
  
  void _initializeArray(vector<double> &vals);
  void _applyIterativeFir(const double *in, double *out,
                          int nIterations);
  void _applyIterativeFirCond(const double *in, double *out,
                              int nIterations, double diffThreshold);
  void _copyArray(const double *in, double *out);
  void _copyArrayCond(const double *in, double *out,
                      const double *original, double diffThreshold);
  void _padArray(double *array);
  void _applyFirFilter(const double *in, double *out);
  double _getFirFilterGain();

};

#endif

