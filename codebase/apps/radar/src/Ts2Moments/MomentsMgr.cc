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
///////////////////////////////////////////////////////////////
// MomentsMgr.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////
//
// MomentsMgr manages the use of the Moments objects, handling 
// the specific parameters for each case.
//
///////////////////////////////////////////////////////////////

#include <toolsa/mem.h>
#include <radar/iwrf_functions.hh>
#include "MomentsMgr.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

MomentsMgr::MomentsMgr(const string &prog_name,
		       const Params &params) :
        _progName(prog_name),
        _params(params)

{
  MEM_zero(_mparams);
  _scanMode = IWRF_SCAN_MODE_NOT_SET;
  _nSamples = 0;
  _nSamplesHalf = 0;
  _xmitRcvMode = IWRF_XMIT_RCV_MODE_NOT_SET;
}

//////////////////////////////////////////////////////////////////
// destructor

MomentsMgr::~MomentsMgr()

{

}

////////////////////////////////////////////////////
// assignment

void MomentsMgr::operator=(const MomentsMgr &other)
  
{

  if (&other == this) {
    return;
  }

  init(other._mparams);

}
  
////////////////////////////////////////////////////
// initialization

void MomentsMgr::init(const Params::moments_params_t &mparams)
  
{

  _mparams = mparams;
  
  switch (_mparams.scan_mode) {
    
    case Params::SCAN_MODE_SECTOR:
      _scanMode = IWRF_SCAN_MODE_SECTOR;
      break;
    case Params::SCAN_MODE_COPLANE:
      _scanMode = IWRF_SCAN_MODE_COPLANE;
      break;
    case Params::SCAN_MODE_RHI:
      _scanMode = IWRF_SCAN_MODE_RHI;
      break;
    case Params::SCAN_MODE_VERTICAL_POINTING:
      _scanMode = IWRF_SCAN_MODE_VERTICAL_POINTING;
      break;
    case Params::SCAN_MODE_IDLE:
      _scanMode = IWRF_SCAN_MODE_IDLE;
      break;
    case Params::SCAN_MODE_SURVEILLANCE:
      _scanMode = IWRF_SCAN_MODE_AZ_SUR_360;
      break;
    case Params::SCAN_MODE_SUNSCAN:
      _scanMode = IWRF_SCAN_MODE_SUNSCAN;
      break;
    case Params::SCAN_MODE_POINTING:
      _scanMode = IWRF_SCAN_MODE_POINTING;
      break;
    case Params::SCAN_MODE_MANUAL_PPI:
      _scanMode = IWRF_SCAN_MODE_MANPPI;
      break;
    case Params::SCAN_MODE_MANUAL_RHI:
      _scanMode = IWRF_SCAN_MODE_MANRHI;
      break;
    default:
      _scanMode = IWRF_SCAN_MODE_NOT_SET;

  } // switch

  switch (_mparams.xmit_rcv_mode) {
    
    case Params::SINGLE_POL: 
      _xmitRcvMode = IWRF_SINGLE_POL;
      break;
    case Params::SINGLE_POL_V: 
      _xmitRcvMode = IWRF_SINGLE_POL_V;
      break;
    case Params::DP_ALT_HV_CO_ONLY:
      _xmitRcvMode = IWRF_ALT_HV_CO_ONLY;
      break;
    case Params::DP_ALT_HV_CO_CROSS:
      _xmitRcvMode = IWRF_ALT_HV_CO_CROSS;
      break;
    case Params::DP_ALT_HV_FIXED_HV:
      _xmitRcvMode = IWRF_ALT_HV_FIXED_HV;
      break;
    case Params::DP_SIM_HV_FIXED_HV:
      _xmitRcvMode = IWRF_SIM_HV_FIXED_HV;
      break;
    case Params::DP_SIM_HV_SWITCHED_HV:
      _xmitRcvMode = IWRF_SIM_HV_SWITCHED_HV;
      break;
    case Params::DP_H_ONLY_FIXED_HV:
      _xmitRcvMode = IWRF_H_ONLY_FIXED_HV;
      break;
    case Params::DP_V_ONLY_FIXED_HV:
      _xmitRcvMode = IWRF_V_ONLY_FIXED_HV;
      break;
    default:
      _xmitRcvMode = IWRF_SINGLE_POL;

  } // switch

  // make sure we have an even number of samples,
  // with a minimum of 16

  _nSamples = ((_mparams.beam_n_samples - 1) / 2 + 1) * 2;
  if (_nSamples < 8) {
    _nSamples = 8;
  }
  if (_mparams.apply_sz) {
    _nSamples = _nSamplesSz1;
  }
  _nSamplesHalf = _nSamples / 2;

  if (_mparams.apply_sz) {
    if (_nSamples != 64) {
      cerr << "WARNING - Ts2Moments::MomentsMgr" << endl;
      cerr << "  For SZ1 phase coding, you must set n_samples to 64" << endl;
      _nSamples = 64;
    }
    if (_xmitRcvMode != IWRF_SINGLE_POL) {
      cerr << "WARNING - Ts2Moments::MomentsMgr" << endl;
      cerr << "  For SZ phase coding, you must use single pol processing" << endl;
      _xmitRcvMode = IWRF_SINGLE_POL;
    }
  }

}

////////////////////////////////////////////
// check if this manager is applicable
// given the scan mode, prf and antenna rate

bool MomentsMgr::checkSuitable(int scanMode,
                               const string &scanName,
                               double prf,
                               double antennaRate) const
  
{

  if (checkScanMode()) {
    if (scanMode != _scanMode) {
      return false;
    }
  }

  if (checkScanName()) {
    if (scanName != getScanName()) {
      return false;
    }
  }

  if (checkPrf()) {
    if (prf < getLowerPrf() || prf > getUpperPrf()) {
      return false;
    }
  }

  if (checkAntennaRate()) {
    if (fabs(antennaRate) < getLowerAntennaRate() ||
        fabs(antennaRate) > getUpperAntennaRate()) {
      return false;
    }
  }

  return true;

}

// print members

void MomentsMgr::print(ostream &out) const

{

  out << "========= MomentsMgr ==========" << endl;
  out << "  checkScanMode: " << checkScanMode() << endl;
  out << "  scanMode: " << iwrf_scan_mode_to_str(getScanMode()) << endl;
  out << "  checkScanName: " << checkScanName() << endl;
  out << "  scanName: " << getScanName() << endl;
  out << "  checkPrf: " << checkPrf() << endl;
  out << "  lowerPrf: " << getLowerPrf() << endl;
  out << "  upperPrf: " << getUpperPrf() << endl;
  out << "  checkAntennaRate: " << checkAntennaRate() << endl;
  out << "  lowerAntennaRate: " << getLowerAntennaRate() << endl;
  out << "  upperAntennaRate: " << getUpperAntennaRate() << endl;
  if (getBeamMethod() == Params::BEAM_SPECIFY_N_SAMPLES) {
    cerr << "  beam method: specify n_samples" << endl;
  } else {
    cerr << "  beam method: specify angle" << endl;
  }
  out << "  nSamples: " << getNSamples() << endl;
  out << "  nSamplesHalf: " << getNSamplesHalf() << endl;
  out << "  beamAngleDeg: " << getBeamAngleDeg() << endl;
  out << "  indexTheBeams: " << indexTheBeams() << endl;
  out << "  indexedResolution: " << getIndexedResolution() << endl;
  out << "  minAntennRateForIndexing: "
      << getMinAntennaRateForIndexing() << endl;
  if (getWindowType() == Params::WINDOW_VONHANN) {
    cerr << "  window: Von Hann" << endl;
  } else if (getWindowType() == Params::WINDOW_BLACKMAN) {
    cerr << "  window: Blackman" << endl;
  } else if (getWindowType() == Params::WINDOW_BLACKMAN) {
    cerr << "  window: Blackman-Nuttall" << endl;
  } else {
    cerr << "  window: Rectangular" << endl;
  }
  out << "  switchingReceiver: " << isSwitchingReceiver() << endl;
  out << "  xmitRcvMode: " << iwrf_xmit_rcv_mode_to_str(getXmitRcvMode()) << endl;
  out << "  applyClutterFilter: " << applyClutterFilter() << endl;
  out << "  applyPhaseDecoding: " << applyPhaseDecoding() << endl;

}

