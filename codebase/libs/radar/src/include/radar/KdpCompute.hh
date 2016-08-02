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
// KdpCompute.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2009
//
///////////////////////////////////////////////////////////////
//
// Kdp for SBand
//
////////////////////////////////////////////////////////////////

#ifndef KdpCompute_hh
#define KdpCompute_hh

#include <toolsa/TaArray.hh>
#include <string>
#include <vector>
using namespace std;

////////////////////////
// This class

class KdpCompute {
  
public:

  // constructor
  
  KdpCompute();
  
  // destructor
  
  ~KdpCompute();

  // set transmit pulsing mode
  // default is AltHv

  void setSlant45();
  void setAltHv();

  // set median filtering for phidp
  // default is off, median filter len is 5

  void setApplyMedianFilterToPhidp(int filter_len) {
    _applyMedianFilterToPhidp = true;
    _phidpMedianFilterLen = filter_len;
  }

  // set number of gates over which to compute the slope of phidp
  // default is 20

  void setPhidpSlopeFitLen(int len) {
    _phidpSlopeFitLen = len;
  }

  // set threshold for standard deviation of phidp
  // default is 12.0

  void setPhidpSdevThreshold(double threshold) {
    _phidpSdevThreshold = threshold;
  }

  // set threshold for standard deviation of zdr
  // default is 1.8

  void setZdrSdevThreshold(double threshold) {
    _zdrSdevThreshold = threshold;
  }

  // set threshold for rhohv in weather
  // default is 0.7

  void setRhohvWxThreshold(double threshold) {
    _rhohvWxThreshold = threshold;
  }
  
  // compute KDP
  // use get methods for access to KDP array, and other
  // arrays used in the computation
  
  int compute(int nGates,
              const double *range,
              const double *dbz,
              const double *zdr,
              const double *phidp,
              const double *rhohv,
              const double *snr,
	      double missingValue);

  // get fields after calling compute()

  const double *getRange() const { return _range; }
  const double *getDbz() const { return _dbz; }
  const double *getZdr() const { return _zdr; }
  const double *getPhidp() const { return _phidp; }
  const double *getRhohv() const { return _rhohv; }
  const double *getSnr() const { return _snr; }
  const double *getSdphidp() const { return _sdphidp; }
  const double *getSdZdr() const { return _sdzdr; }
  const double *getKdp() const { return _kdp; }

protected:
  
private:

  static const int EXTRA_GATES = 500; // for padding at each end of arrays
  static const int EXTRA_HALF = 250; // for padding at each end of arrays
  static const int mgood = 10;
  static const int unfoldLen = 5;
  
  double _missingValue;

  // range for phidp folding (degrees)

  double _phidpFoldRange;

  // thresholds for filtering phidp

  int _phidpSlopeFitLen;      // number of gates for fitting slope to phidp
  double _phidpSdevThreshold; // threshold for sdev of phidp
  double _zdrSdevThreshold;   // threshold for sdev of zdr
  double _rhohvWxThreshold;   // threshold for rhohv in weather
  
  // median filtering
  
  bool _applyMedianFilterToPhidp;
  int _phidpMedianFilterLen;

  // store input data in local arrays
  // this data is censored and filtered
  
  TaArray<double> _range_;
  double *_range;
  
  TaArray<double> _dbz_;
  double *_dbz;
  
  TaArray<double> _zdr_;
  double *_zdr;
  
  TaArray<double> _phidp_;
  double *_phidp;
  
  TaArray<double> _rhohv_;
  double *_rhohv;
  
  TaArray<double> _snr_;
  double *_snr;

  TaArray<double> _sdphidp_;
  double *_sdphidp;

  TaArray<double> _sdzdr_;
  double *_sdzdr;
  
  TaArray<double> _kdp_;
  double *_kdp;

  TaArray<bool> _valid_;
  bool *_valid;

  // methods
  
  void _msr(double &ymn, double &sd, const double *y, int n);
  void _lse(double &a, double &b, const double *x, const double *y, int n);
  void _unfoldPhidp(int nGates);
  double _computePhidpSlope(int index);

};

#endif

