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
// MomentsMgr.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////
//
// MomentsMgr manages the use of the Moments objects, handling 
// the specific parameters for each case.
//
///////////////////////////////////////////////////////////////

#ifndef MomentsMgr_hh
#define MomentsMgr_hh

#include <iostream>
#include <string>
#include <radar/iwrf_data.h>
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class MomentsMgr {
  
public:

  // constructor

  MomentsMgr(const string &prog_name,
	     const Params &params);
  
  // destructor
  
  ~MomentsMgr();
  
  // assignment

  void operator=(const MomentsMgr&);
  
  // initialization

  void init(const Params::moments_params_t &mparams);
  
  // check if this manager is applicable
  // given the scan mode, prf and antenna rate
  
  bool checkSuitable(int scanMode,
                     const string &scanName,
                     double prf,
                     double antennaRate) const;

  // get methods

  inline bool checkScanMode() const {
    return _mparams.check_scan_mode; 
  }
  inline iwrf_scan_mode_t getScanMode() const {
    return _scanMode;
  }
  
  inline bool checkScanName() const { 
    return _mparams.check_scan_name; 
  }
  inline string getScanName() const { 
    return _mparams.scan_name; 
  }

  inline bool checkPrf() const { return _mparams.check_prf; }
  inline double getLowerPrf() const { return _mparams.prf_lower_limit; }
  inline double getUpperPrf() const { return _mparams.prf_upper_limit; }

  inline bool checkAntennaRate() const {
    return _mparams.check_antenna_rate; 
  }
  inline double getLowerAntennaRate() const {
    return _mparams.antenna_rate_lower_limit;
  }
  inline double getUpperAntennaRate() const {
    return _mparams.antenna_rate_upper_limit;
  }

  inline Params::beam_method_t getBeamMethod() const {
    return _mparams.beam_method;
  }

  inline int getNSamples() const { return _nSamples; }
  inline int getNSamplesHalf() const { return _nSamplesHalf; }
  
  double getBeamAngleDeg() const {
    return _mparams.beam_angle_deg;
  }
  
  bool indexTheBeams() const {
    return _mparams.index_the_beams;
  }
  
  double getIndexedResolution() const {
    return _mparams.indexed_resolution;
  }
  
  double getMinAntennaRateForIndexing() const {
    return _mparams.min_antenna_rate_for_indexing;
  }

  inline Params::window_t getWindowType() const {
    return _mparams.window;
  }
  
  inline bool isSwitchingReceiver() const {
    return _mparams.switching_receiver;
  }

  inline iwrf_xmit_rcv_mode_t getXmitRcvMode() const {
    return _xmitRcvMode;
  }
  
  inline bool applyClutterFilter() const {
    if (_params.clutter_filter_type == Params::CLUTTER_FILTER_NONE) {
      return false;
    } else {
      return _mparams.apply_clutter_filter;
    }
  }

  inline bool applyPhaseDecoding() const { return _mparams.apply_sz; }

  inline bool changeVelocitySign() const {
    return _mparams.change_velocity_sign;
  }

  inline bool isDualPrt() const {
    return (_mparams.proc_flags == Params::PROC_FLAGS_DUAL_PRT);
  }

  // print members
  
  void print(ostream &out) const;

protected:
  
private:

  string _progName;
  const Params &_params;
  Params::moments_params_t _mparams;

  // scan mode

  iwrf_scan_mode_t _scanMode;
  
  // number of pulse samples
  
  static const int _nSamplesSz1 = 64;
  int _nSamples;
  int _nSamplesHalf;

  iwrf_xmit_rcv_mode_t _xmitRcvMode;

};

#endif

