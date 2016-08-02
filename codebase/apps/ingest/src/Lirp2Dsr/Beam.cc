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
// Beam.cc
//
// Beam object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////
//
// Beam object holds time series and moment data.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <cmath>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include "sz864.h"
#include "Beam.hh"
using namespace std;

bool Beam::_zvFlagReady = false;
int Beam::_zvFlag = 0;
int Beam::_nZdr = 0;
double Beam::_sumZdr = 0.0;

InterestMap *Beam::_dbzTextureInterestMap = NULL;
InterestMap *Beam::_sqrtTextureInterestMap = NULL;
InterestMap *Beam::_dbzSpinInterestMap = NULL;
InterestMap *Beam::_wtSpinInterestMap = NULL;
InterestMap *Beam::_velInterestMap = NULL;
InterestMap *Beam::_widthInterestMap = NULL;
InterestMap *Beam::_velSdevInterestMap = NULL;
InterestMap *Beam::_zdrSdevInterestMap = NULL;
InterestMap *Beam::_rhohvInterestMap = NULL;
InterestMap *Beam::_rhohvSdevInterestMap = NULL;
InterestMap *Beam::_clutRatioNarrowInterestMap = NULL;
InterestMap *Beam::_clutRatioWideInterestMap = NULL;
InterestMap *Beam::_clutWxPeakRatioInterestMap = NULL;
InterestMap *Beam::_clutWxPeakSepInterestMap = NULL;
InterestMap *Beam::_clutWxPeakSepSdevInterestMap = NULL;

////////////////////////////////////////////////////
// Constructor

Beam::Beam(const string &prog_name,
	   const Params &params,
	   int max_trips,
	   const deque<Pulse *> pulse_queue,
	   double az,
	   const MomentsMgr *momentsMgr) :
  _progName(prog_name),
  _params(params),
  _maxTrips(max_trips),
  _az(az),
  _momentsMgr(momentsMgr)

{

  _nSamples = _momentsMgr->getNSamples();

  // load up pulse vector

  int offset = 0;
  if (_momentsMgr->getDualPol()) {
    // for dual pol data, we check the Zv flag to determine whether pulse 0
    // is horizontal or vertical polarized. If vertical, go back by one to
    // start with horizontal
    int seqNum = pulse_queue[_nSamples-1]->getSeqNum();
    if ((seqNum % 2) != _momentsMgr->getZvFlag()) {
      offset = 1;
    }
  }
    
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = _nSamples - ii - 1 + offset;
    Pulse *pulse = pulse_queue[jj];
    _pulses.push_back(pulse);
    pulse->addClient("Beam::Beam");
//      Pulse *prevPulse = pulse_queue[jj + _nSamples];
//      _prevPulses.push_back(prevPulse);
//      prevPulse->addClient("Beam::Beam");
  }
  
  // initialize
  
  _halfNSamples = _nSamples / 2;
  _delta12 = NULL;
  _iq = NULL;
  _iqh = NULL;
  _iqv = NULL;

  // compute mean prt

  double sumPrt = 0.0;
  for (int ii = 0; ii < _nSamples; ii++) {
    sumPrt += _pulses[ii]->getPrt();
  }
  _prt = sumPrt / _nSamples;
  _prf = 1.0 / _prt;

  // set time

  _time = (time_t) (_pulses[_halfNSamples]->getTime() + 0.5);

  // set elevation

  _el = _pulses[_halfNSamples]->getEl();
  if (_el > 180.0) {
    _el -= 360.0;
  }

  // compute number of output gates

  _nGatesPulse = _pulses[_halfNSamples]->getNGates();
  if (_momentsMgr->getApplySz() && !_momentsMgr->getDualPol()) {
    _nGatesOut = _nGatesPulse * 2;
  } else {
    _nGatesOut = _nGatesPulse;
  }

  // alloc field arrays

  _allocArrays();

  // interest maps if needed

  if (_params.apply_rec) {
    createInterestMaps(_params);
  }
  _recVarsReady = false;
  
}

//////////////////////////////////////////////////////////////////
// destructor

Beam::~Beam()

{

  for (int ii = 0; ii < (int) _pulses.size(); ii++) {
    if (_pulses[ii]->removeClient("Beam::~Beam") == 0) {
      delete _pulses[ii];
    }
  }
  _pulses.clear();

  for (int ii = 0; ii < (int) _prevPulses.size(); ii++) {
    if (_prevPulses[ii]->removeClient("Beam::~Beam") == 0) {
      delete _prevPulses[ii];
    }
  }
  _prevPulses.clear();

  if (_delta12) {
    delete[] _delta12;
  }

  if (_iq) {
    ufree2((void **) _iq);
  }
  if (_iqh) {
    ufree2((void **) _iqh);
  }
  if (_iqv) {
    ufree2((void **) _iqv);
  }

  _freeArrays();

}

/////////////////////////////////////////////////////////
// create interest maps
//
// These are static on the class, so are only created once.

int Beam::createInterestMaps(const Params &params)

{  

  // dbz texture

  if (_dbzTextureInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("DbzTexture",
				    params._dbz_texture_interest_map,
				    params.dbz_texture_interest_map_n,
				    pts)) {
      return -1;
    }
    _dbzTextureInterestMap =
      new InterestMap("DbzTexture", pts,
		      params.dbz_texture_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _dbzTextureInterestMap" << endl;
    }
  } // if (!_dbzTextureInterestMap)

  // sqrt dbz texture
  
  if (_sqrtTextureInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("SqrtTexture",
				    params._sqrt_texture_interest_map,
				    params.sqrt_texture_interest_map_n,
				    pts)) {
      return -1;
    }
    _sqrtTextureInterestMap =
      new InterestMap("SqrtTexture", pts,
		      params.sqrt_texture_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _sqrtTextureInterestMap" << endl;
    }
  } // if (!_dbzTextureInterestMap)

  // dbz spin

  if (_dbzSpinInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("DbzSpin",
				    params._dbz_spin_interest_map,
				    params.dbz_spin_interest_map_n,
				    pts)) {
      return -1;
    }
    _dbzSpinInterestMap =
      new InterestMap("DbzSpin", pts,
		      params.dbz_spin_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _dbzSpinInterestMap" << endl;
    }
  } // if (!_dbzSpinInterestMap)

  // weighted dbz spin

  if (_wtSpinInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("WtSpin",
				    params._wt_spin_interest_map,
				    params.wt_spin_interest_map_n,
				    pts)) {
      return -1;
    }
    _wtSpinInterestMap =
      new InterestMap("WtSpin", pts,
		      params.wt_spin_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _wtSpinInterestMap" << endl;
    }
  } // if (!_wtSpinInterestMap)

  // velocity

  if (_velInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("velocity",
				    params._vel_interest_map,
				    params.vel_interest_map_n,
				    pts)) {
      return -1;
    }
    _velInterestMap =
      new InterestMap("velocity", pts, params.vel_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _velInterestMap" << endl;
    }
  } // if (!_velInterestMap)

  // width

  if (_widthInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("spectrum width",
				    params._width_interest_map,
				    params.width_interest_map_n,
				    pts)) {
      return -1;
    }
    _widthInterestMap =
      new InterestMap("spectrum width", pts, params.width_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _widthInterestMap" << endl;
    }
  } // if (!_widthInterestMap)

  // sdev of velocity

  if (_velSdevInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("velocity sdev",
				    params._vel_sdev_interest_map,
				    params.vel_sdev_interest_map_n,
				    pts)) {
      return -1;
    }
    _velSdevInterestMap =
      new InterestMap("velocity sdev", pts, params.vel_sdev_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _velSdevInterestMap" << endl;
    }
  } // if (!_velSdevInterestMap)

  // sdev of zdr

  if (_zdrSdevInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("zdr sdev",
				    params._zdr_sdev_interest_map,
				    params.zdr_sdev_interest_map_n,
				    pts)) {
      return -1;
    }
    _zdrSdevInterestMap =
      new InterestMap("zdr sdev", pts, params.zdr_sdev_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _zdrSdevInterestMap" << endl;
    }
  } // if (!_zdrSdevInterestMap)

  // rhohv

  if (_rhohvInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("rhohv",
				    params._rhohv_interest_map,
				    params.rhohv_interest_map_n,
				    pts)) {
      return -1;
    }
    _rhohvInterestMap =
      new InterestMap("rhohv", pts, params.rhohv_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _rhohvInterestMap" << endl;
    }
  } // if (!_rhohvInterestMap)

  // sdev of rhohv

  if (_rhohvSdevInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("rhohv sdev",
				    params._rhohv_sdev_interest_map,
				    params.rhohv_sdev_interest_map_n,
				    pts)) {
      return -1;
    }
    _rhohvSdevInterestMap =
      new InterestMap("rhohv sdev", pts, params.rhohv_sdev_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _rhohvSdevInterestMap" << endl;
    }
  } // if (!_rhohvSdevInterestMap)

  // clutter probability fields

  if (_clutRatioNarrowInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("clutter ratio narrow",
				    params._clut_ratio_narrow_interest_map,
				    params.clut_ratio_narrow_interest_map_n,
				    pts)) {
      return -1;
    }
    _clutRatioNarrowInterestMap =
      new InterestMap("clutter ration narrow",
                      pts, params.clut_ratio_narrow_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _clutRatioNarrowInterestMap" << endl;
    }
  } // if (!_clutRatioNarrowInterestMap)

  if (_clutRatioWideInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("clutter ratio wide",
				    params._clut_ratio_wide_interest_map,
				    params.clut_ratio_wide_interest_map_n,
				    pts)) {
      return -1;
    }
    _clutRatioWideInterestMap =
      new InterestMap("clutter ratio wide",
                      pts, params.clut_ratio_wide_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _clutRatioWideInterestMap" << endl;
    }
  } // if (!_clutRatioWideInterestMap)

  if (_clutWxPeakRatioInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("clut-wx-peak ratio",
				    params._clut_wx_peak_ratio_interest_map,
				    params.clut_wx_peak_ratio_interest_map_n,
				    pts)) {
      return -1;
    }
    _clutWxPeakRatioInterestMap =
      new InterestMap("clut-wx-peak ratio",
                      pts, params.clut_wx_peak_ratio_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _clutWxPeakRatioInterestMap" << endl;
    }
  } // if (!_clutWxPeakRatioInterestMap)

  if (_clutWxPeakSepInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("clut-wx-peak sep",
				    params._clut_wx_peak_sep_interest_map,
				    params.clut_wx_peak_sep_interest_map_n,
				    pts)) {
      return -1;
    }
    _clutWxPeakSepInterestMap =
      new InterestMap("clut-wx-peak sep",
                      pts, params.clut_wx_peak_sep_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _clutWxPeakSepInterestMap" << endl;
    }
  } // if (!_clutWxPeakSepInterestMap)

  if (_clutWxPeakSepSdevInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector
        ("clut-wx-peak sep",
         params._clut_wx_peak_sep_sdev_interest_map,
         params.clut_wx_peak_sep_sdev_interest_map_n,
         pts)) {
      return -1;
    }
    _clutWxPeakSepSdevInterestMap =
      new InterestMap("clut-wx-peak sep sdev",
                      pts, params.clut_wx_peak_sep_sdev_interest_weight);
    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> Creating _clutWxPeakSepSdevInterestMap" << endl;
    }
  } // if (!_clutWxPeakSepSdevInterestMap)

  return 0;
  
}

/////////////////////////////////////////////////
// compute moments
//
// Returns 0 on success, -1 on failure
    
int Beam::computeMoments(int &combinedPrintCount,
			 FILE *combinedSpectraFile)
  
{

  int iret = 0;

  if (_momentsMgr->getDualPol()) {
    
    if (computeMomentsDualPol(combinedPrintCount, combinedSpectraFile)) {
      iret = -1;
    }

  } else {

    if (computeMomentsSinglePol(combinedPrintCount, combinedSpectraFile)) {
      iret = -1;
    }

  }

  _recVarsReady = false;

  return iret;

}

/////////////////////////////////////////////////
// compute moments - single pol
//
// Returns 0 on success, -1 on failure
    
int Beam::computeMomentsSinglePol(int &combinedPrintCount,
				  FILE *combinedSpectraFile)
  
{

  // compute delta phases for SZ, if required
  
  if (_momentsMgr->getApplySz()) {
    _delta12 = new Complex_t[_nSamples];
    for (int i = 0; i < _nSamples; i++) {
      _delta12[i].re = cos(_pulses[i]->getPhaseDiff0() * DEG_TO_RAD);
      _delta12[i].im = -1.0 * sin(_pulses[i]->getPhaseDiff0() * DEG_TO_RAD);
    }
  } // if (_applySz) 
  
  // get unpacked pulses
  
  const FLT4* iqData[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    iqData[ii] = _pulses[ii]->getIq();
  }
  
  // load up IQ data array
  
  if (_iq) {
    ufree2((void **) _iq);
  }
  _iq = (Complex_t **) ucalloc2(_nGatesPulse, _nSamples, sizeof(Complex_t));
  int posn = 0;
  for (int igate = 0; igate < _nGatesPulse; igate++, posn += 2) {
    Complex_t *iq = _iq[igate];
    for (int isamp = 0; isamp < _nSamples; isamp++, iq++) {
      iq->re = iqData[isamp][posn];
      iq->im = iqData[isamp][posn + 1];
    } // isamp
  } // igate
  
  if (_momentsMgr->getApplySz()) {
    _momentsMgr->computeMomentsSz(_time, _el, _az, _prt,
				  _nGatesPulse, _iq, _delta12,
				  combinedPrintCount, combinedSpectraFile,
				  _powerDbm, _snr, _dbz, _dbzt,
				  _vel, _width, _leakage,
				  _flags, _tripFlag,
				  _gateSpectra);
  } else {
    _momentsMgr->computeMoments(_time, _el, _az, _prt,
				_nGatesPulse, _iq,
				combinedPrintCount, combinedSpectraFile,
				_powerDbm, _snr, _dbz,
				_noiseDbm, _snrn, _dbzn,
				_vel, _width, _flags,
                                _recClutDbzNarrow,
                                _recClutRatioNarrow,
                                _recClutRatioWide,
                                _recClutWxPeakRatio,
                                _recClutWxPeakSep,
				_gateSpectra);
  }
  
  return 0;

}

/////////////////////////////////////////////////
// compute moments - dual pol case
//
// Returns 0 on success, -1 on failure
    
int Beam::computeMomentsDualPol(int &combinedPrintCount,
				FILE *combinedSpectraFile)
  
{

  // set up data pointer array
  
  const FLT4* iqData[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    iqData[ii] = _pulses[ii]->getIq();
  }
  
  // load up IQ data array
  
  if (_iq) {
    ufree2((void **) _iq);
  }
  _iq = (Complex_t **) ucalloc2(_nGatesPulse, _nSamples, sizeof(Complex_t));
  for (int igate = 0, posn = 0; igate < _nGatesPulse; igate++, posn += 2) {
    Complex_t *iq = _iq[igate];
    for (int isamp = 0; isamp < _nSamples; isamp++, iq++) {
      iq->re = iqData[isamp][posn];
      iq->im = iqData[isamp][posn + 1];
    } // isamp
  } // igate
  
  // load up IQH and IQV data arrays
  
  int nDual = _nSamples / 2;
  if (_iqh) {
    ufree2((void **) _iqh);
  }
  if (_iqv) {
    ufree2((void **) _iqv);
  }
  _iqh = (Complex_t **) ucalloc2(_nGatesPulse, nDual, sizeof(Complex_t));
  _iqv = (Complex_t **) ucalloc2(_nGatesPulse, nDual, sizeof(Complex_t));
  for (int igate = 0, posn = 0; igate < _nGatesPulse; igate++, posn += 2) {
    Complex_t *iqh = _iqh[igate];
    Complex_t *iqv = _iqv[igate];
    for (int isamp = 0; isamp < _nSamples; iqh++, iqv++) {
      iqh->re = iqData[isamp][posn];
      iqh->im = iqData[isamp][posn + 1];
      isamp++;
      iqv->re = iqData[isamp][posn];
      iqv->im = iqData[isamp][posn + 1];
      isamp++;
    } // isamp
  } // igate

  _momentsMgr->computeMomentsDualPol(_time, _el, _az, _prt,
				     _nGatesPulse, _iq, _iqh, _iqv,
				     combinedPrintCount, combinedSpectraFile,
				     _powerDbm, _snr, _dbz,
				     _vel, _width, _flags,
				     _zdr, _rhohv, _phidp, _kdp,
                                     _recClutRatioNarrow,
                                     _recClutRatioWide,
                                     _recClutWxPeakRatio,
                                     _recClutWxPeakSep,
				     _gateSpectra);
  
  return 0;

}

/////////////////////////////////////////////////
// compute REC

void Beam::computeRec(const deque<Beam *> beams,
		      int midBeamIndex)
  
{
  
  const Beam *midBeam = beams[midBeamIndex];
  
  // vel and width

  for (int igate = 0; igate < _nGatesOut; igate++) {
    _recVel[igate] = beams[midBeamIndex]->_vel[igate];
    _recWidth[igate] = beams[midBeamIndex]->_width[igate];
  }

  // set limits for REC computations by checking the surrounding beams
  
  int minBeamIndex, maxBeamIndex;
  _computeRecBeamLimits(beams, midBeamIndex, minBeamIndex, maxBeamIndex);
  
  // make sure all beams have computed the rec variables

  for (int ii = minBeamIndex; ii <= maxBeamIndex; ii++) {
    beams[ii]->_computeRecVars();
  }

  // compute number of gates in kernel, making sure there is an odd number

  int nGatesKernel = (int)
    (_params.rec_kernel_range_len / _momentsMgr->getGateSpacing() + 0.5);
  if ((nGatesKernel % 2) == 0) {
    nGatesKernel++;
  }
  int nGatesHalf = nGatesKernel / 2;

  // set up gate limits

  vector<int> startGate;
  vector<int> endGate;
  for (int igate = 0; igate < _nGatesOut; igate++) {
    int start = igate - nGatesHalf;
    if (start < 0) {
      start = 0;
    }
    startGate.push_back(start);
    int end = igate + nGatesHalf;
    if (end > _nGatesOut - 1) {
      end = _nGatesOut - 1;
    }
    endGate.push_back(end);
  } // igate
  
  // tdbz and spin
  // are based on spatial variation in range and azimuth

  for (int igate = 0; igate < _nGatesOut; igate++) {
    
    // compute sums etc. for stats over the kernel space
    
    double nTexture = 0.0;
    double sumDbzDiffSq = 0.0;
    
    double nSpinChange = 0.0;
    double nSpinTotal = 0.0;
    double sumSpinDbz = 0.0;
    
    for (int ibeam = minBeamIndex; ibeam <= maxBeamIndex; ibeam++) {
      
      const Beam *beam = beams[ibeam];
      const double *dBZ = beam->_dbz;
      const double *dbzDiffSq = beam->_recDbzDiffSq;
      const double *dbzSpinChange = beam->_recDbzSpinChange;

      for (int jj = startGate[igate]; jj <= endGate[igate]; jj++) {

	// dbz texture

	double dds = dbzDiffSq[jj];
	if (dds != _missingDbl) {
	  sumDbzDiffSq += dds;
	  nTexture++;
	}

	// spin
        
	double dsc = dbzSpinChange[jj];
        double dbz = dBZ[jj];
	if (dsc != _missingDbl && dbz != _missingDbl) {
	  nSpinChange += dsc;
          sumSpinDbz += dbz;
	  nSpinTotal++;
	}

      } // jj
      
    } // ibeam
    
    // texture
    
    if (nTexture > 0) {
      double texture = sumDbzDiffSq / nTexture;
      _recDbzTexture[igate] = texture;
      _recSqrtTexture[igate] = sqrt(texture);
    }

    // spin
    
    if (nSpinTotal > 0) {
      _recDbzSpin[igate] = (nSpinChange / nSpinTotal) * 100.0;
      double meanDbz = sumSpinDbz / nSpinTotal;
      _recWtSpin[igate] = (nSpinChange / nSpinTotal) * meanDbz;
    }

  } // igate

  // vel sdev and clutter peak separation coefficient of variation
  // computed in range only

  for (int igate = 0; igate < _nGatesOut; igate++) {

    // compute sums etc. for stats over the kernel space

    double nVel = 0.0;
    double sumVel = 0.0;
    double sumVelSq = 0.0;
    
    double nSep = 0.0;
    double sumSep = 0.0;
    double sumSepSq = 0.0;
    
    const double *vel = midBeam->_vel;
    const double *sep = midBeam->_recClutWxPeakSep;
    
    // const double *width = beam->_width;
    
    for (int jj = startGate[igate]; jj <= endGate[igate]; jj++) {

      // vel
      
      double vv = vel[jj];
      if (vv != _missingDbl) {
        sumVel += vv;
        sumVelSq += (vv * vv);
        nVel++;
      }
      
      // peak separation
      
      double ss = sep[jj];
      if (ss != _missingDbl) {
        sumSep += ss;
        sumSepSq += (ss * ss);
        nSep++;
      }
      
    } // jj
    
    // vel sdev
    
    if (nVel > 0) {
      double meanVel = sumVel / nVel;
      if (nVel > 2) {
	double term1 = sumVelSq / nVel;
	double term2 = meanVel * meanVel;
	if (term1 >= term2) {
	  _recVelSdev[igate] = sqrt(term1 - term2);
	}
      }
    }

    // sep coefficient of variation
    
    if (nSep > 0) {
      double meanSep = sumSep / nSep;
      if (nSep > 2) {
	double term1 = sumSepSq / nSep;
	double term2 = meanSep * meanSep;
	if (term1 >= term2) {
	  _recClutWxPeakSepSdev[igate] = sqrt(term1 - term2) / meanSep;
	}
      }
    }
    
  } // igate

  // dual pol fields

  if (_momentsMgr->getDualPol()) {

    for (int igate = 0; igate < _nGatesOut; igate++) {

      // compute sums etc. for stats over the kernel space
      
      double nZdr = 0.0;
      double sumZdr = 0.0;
      double sumZdrSq = 0.0;
      
      double nRhohv = 0.0;
      double sumRhohv = 0.0;
      double sumRhohvSq = 0.0;
      
      const Beam *beam = beams[midBeamIndex];
      const double *zdr = beam->_zdr;
      const double *rhohv = beam->_rhohv;
      
      for (int jj = startGate[igate]; jj <= endGate[igate]; jj++) {
	
	// zdr
	
	double zz = zdr[jj];
	if (zz != _missingDbl) {
	  sumZdr += zz;
	  sumZdrSq += (zz * zz);
	  nZdr++;
	}

	// rhohv
	
	double rr = rhohv[jj];
	if (rr != _missingDbl) {
	  sumRhohv += rr;
	  sumRhohvSq += (rr * rr);
	  nRhohv++;
	}
	
      } // jj
      
      // zdr
      
      if (nZdr > 0) {
	double meanZdr = sumZdr / nZdr;
	if (nZdr > 2) {
	  double term1 = sumZdrSq / nZdr;
	  double term2 = meanZdr * meanZdr;
	  if (term1 >= term2) {
	    _recZdrSdev[igate] = sqrt(term1 - term2);
	  }
	}
      }
      
      // rhohv
      
      if (nRhohv > 0) {
	double meanRhohv = sumRhohv / nRhohv;
	if (nRhohv > 2) {
	  double term1 = sumRhohvSq / nRhohv;
	  double term2 = meanRhohv * meanRhohv;
	  if (term1 >= term2) {
	    _recRhohvSdev[igate] = sqrt(term1 - term2);
	  }
	}
      }
      
    } // igate
    
  } // if (dualPol ...
  
  // compute rec clutter field

  const double *snr = _snr;
  const double *dbz = _dbz;
  const double *dbzTexture = _recDbzTexture;
  const double *sqrtTexture = _recSqrtTexture;
  const double *dbzSpin = _recDbzSpin;
  const double *wtSpin = _recWtSpin;
  const double *vel = _recVel;
  const double *velSdev = _recVelSdev;
  const double *width = _recWidth;
  const double *zdrSdev = _recZdrSdev;
  const double *rhohv = _rhohv;
  const double *rhohvSdev = _recRhohvSdev;
  const double *ratioNarrow = _recClutRatioNarrow;
  const double *ratioWide = _recClutRatioWide;
  const double *clutWxRatio = _recClutWxPeakRatio;
  const double *clutWxSep = _recClutWxPeakSep;
  const double *clutWxSepSdev = _recClutWxPeakSepSdev;
  double *rec = _rec;
  double *recFlag = _recFlag;
  double probThreshold = _params.rec_threshold_for_clutter;
  double ratioNarrowThreshold = _params.min_ratio_narrow_clutter;

  for (int igate = 0; igate < _nGatesOut;
       igate++, snr++, dbz++, dbzTexture++, dbzSpin++, wtSpin++,
         vel++, velSdev++, width++,
	 zdrSdev++, rhohv++, rhohvSdev++,
         ratioNarrow++, ratioWide++, clutWxRatio++, clutWxSep++,
         rec++, recFlag++) {

    if (*dbz == _missingDbl) {
      continue;
    }

    double sumInterest = 0.0;
    double sumWeights = 0.0;
    
    _dbzTextureInterestMap->accumInterest(*dbzTexture,
                                          sumInterest, sumWeights);
    _sqrtTextureInterestMap->accumInterest(*sqrtTexture,
                                           sumInterest, sumWeights);
    _dbzSpinInterestMap->accumInterest(*dbzSpin,
                                       sumInterest, sumWeights);
    _wtSpinInterestMap->accumInterest(*wtSpin,
                                      sumInterest, sumWeights);
    _velInterestMap->accumInterest(*vel,
                                   sumInterest, sumWeights);
    _widthInterestMap->accumInterest(*width, sumInterest, sumWeights);
    _velSdevInterestMap->accumInterest(*velSdev, sumInterest, sumWeights);
    _zdrSdevInterestMap->accumInterest(*zdrSdev, sumInterest, sumWeights);
    _rhohvInterestMap->accumInterest(*rhohv, sumInterest, sumWeights);
    _rhohvSdevInterestMap->accumInterest(*rhohvSdev, sumInterest, sumWeights);
    _clutRatioNarrowInterestMap->accumInterest(*ratioNarrow,
                                               sumInterest, sumWeights);
    _clutRatioWideInterestMap->accumInterest(*ratioWide,
                                             sumInterest, sumWeights);
    _clutWxPeakRatioInterestMap->accumInterest(*clutWxRatio,
                                               sumInterest, sumWeights);
    _clutWxPeakSepInterestMap->accumInterest(*clutWxSep,
                                             sumInterest, sumWeights);
    _clutWxPeakSepSdevInterestMap->accumInterest(*clutWxSepSdev,
                                                 sumInterest, sumWeights);
    
    *rec = sumInterest / sumWeights;

    if (*rec >= probThreshold &&
        *ratioNarrow >= ratioNarrowThreshold) {
      *recFlag = 1.0;
    } else {
      *recFlag = 0.0;
    }

  } // igate

}

/////////////////////////////////////////////////////////////////////////
// filter clutter, using rec values to decide whether to apply the filter
    
void Beam::filterClutterUsingRec()
  
{

  // load up flags indicating the likely gates for clutter

  const double *recFlag = _recFlag;
  bool hasClutter[_nGatesOut];
  memset(hasClutter, 0, _nGatesOut * sizeof(bool));

  for (int igate = 0; igate < _nGatesOut; igate++, recFlag++) {
    if (*recFlag != _missingDbl && *recFlag > 0.99) {
      hasClutter[igate] = true;
    }
  }

  // compute filtered moments

  if (_momentsMgr->getDualPol()) {
    
    _momentsMgr->filterClutterDualPol(_prt, _nGatesPulse, _iq, _iqh, _iqv, hasClutter,
				      _dbz, _vel, _width, _gateSpectra,
				      _clut, _dbzf, _velf, _widthf);
    
  } else {
    
    if (_momentsMgr->getApplySz()) {
      _momentsMgr->filterClutterSz(_prt, _nGatesPulse,
				   _gateSpectra, _delta12, hasClutter,
				   _dbz, _vel, _width, _tripFlag,
				   _clut, _dbzf, _velf, _widthf);
    } else {
      _momentsMgr->filterClutter(_prt, _nGatesPulse, _iq, hasClutter,
				 _dbz, _vel, _width, _gateSpectra,
				 _clut, _dbzf, _velf, _widthf);
    }
    
  }


}

/////////////////////////////////////////////////////////
// alloc arrays

void Beam::_allocArrays()

{  

  // field arrays
  
  _flags = new int[_nGatesOut];
  _tripFlag = new int[_nGatesOut];
  _vcensor = new int[_nGatesOut];
  _censor = new int[_nGatesOut];
  _zinfill = new int[_nGatesOut];

  _powerDbm = new double[_nGatesOut];
  _snr = new double[_nGatesOut];

  _noiseDbm = new double[_nGatesOut];
  _snrn = new double[_nGatesOut];
  _dbzn = new double[_nGatesOut];

  _dbzt = new double[_nGatesOut];
  _dbz = new double[_nGatesOut];
  _vel = new double[_nGatesOut];
  _width = new double[_nGatesOut];

  _clut = new double[_nGatesOut];
  _dbzf = new double[_nGatesOut];
  _velf = new double[_nGatesOut];
  _widthf = new double[_nGatesOut];

  _dbzi = new double[_nGatesOut];
  _veli = new double[_nGatesOut];
  _widthi = new double[_nGatesOut];

  _leakage = new double[_nGatesOut];
  _vtexture = new double[_nGatesOut];
  _vspin = new double[_nGatesOut];
  _fcensor = new double[_nGatesOut];
  _ztexture = new double[_nGatesOut];

  _phidp = new double[_nGatesOut];
  _kdp = new double[_nGatesOut];
  _zdr = new double[_nGatesOut];
  _rhohv = new double[_nGatesOut];

  // initialize integer arrays to 0

  memset(_flags, 0, _nGatesOut * sizeof(int));
  memset(_tripFlag, 0, _nGatesOut * sizeof(int));
  memset(_vcensor, 0, _nGatesOut * sizeof(int));
  memset(_censor, 0, _nGatesOut * sizeof(int));
  memset(_zinfill, 0, _nGatesOut * sizeof(int));

  // initialize double arrays to missing

  for (int ii = 0; ii < _nGatesOut; ii++) {
    _powerDbm[ii] = _missingDbl;
  }

  memcpy(_snr, _powerDbm, _nGatesOut * sizeof(double));

  memcpy(_noiseDbm, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_snrn, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_dbzn, _powerDbm, _nGatesOut * sizeof(double));

  memcpy(_dbzt, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_dbz, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_vel, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_width, _powerDbm, _nGatesOut * sizeof(double));

  memcpy(_clut, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_dbzf, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_velf, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_widthf, _powerDbm, _nGatesOut * sizeof(double));

  memcpy(_dbzi, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_veli, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_widthi, _powerDbm, _nGatesOut * sizeof(double));

  memcpy(_leakage, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_vtexture, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_vspin, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_fcensor, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_ztexture, _powerDbm, _nGatesOut * sizeof(double));

  memcpy(_phidp, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_kdp, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_zdr, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_rhohv, _powerDbm, _nGatesOut * sizeof(double));

  // REC variables

  _recVarsReady = false;
  if (_params.apply_rec) {

    _recDbzDiffSq = new double[_nGatesOut];
    _recDbzSpinChange = new double[_nGatesOut];
    _recDbzTexture = new double[_nGatesOut];
    _recSqrtTexture = new double[_nGatesOut];
    _recDbzSpin = new double[_nGatesOut];
    _recWtSpin = new double[_nGatesOut];
    _recVel = new double[_nGatesOut];
    _recVelSdev = new double[_nGatesOut];
    _recWidth = new double[_nGatesOut];
    _recClutDbzNarrow = new double[_nGatesOut];
    _recClutRatioNarrow = new double[_nGatesOut];
    _recClutRatioWide = new double[_nGatesOut];
    _recClutWxPeakRatio = new double[_nGatesOut];
    _recClutWxPeakSep = new double[_nGatesOut];
    _recClutWxPeakSepSdev = new double[_nGatesOut];
    _recZdrSdev = new double[_nGatesOut];
    _recRhohvSdev = new double[_nGatesOut];
    _rec = new double[_nGatesOut];
    _recFlag = new double[_nGatesOut];
    
    memcpy(_recDbzDiffSq, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recDbzSpinChange, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recDbzTexture, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recSqrtTexture, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recDbzSpin, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recWtSpin, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recVel, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recVelSdev, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recWidth, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recClutDbzNarrow, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recClutRatioNarrow, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recClutRatioWide, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recClutWxPeakRatio, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recClutWxPeakSep, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recClutWxPeakSepSdev, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recZdrSdev, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recRhohvSdev, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_rec, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_recFlag, _powerDbm, _nGatesOut * sizeof(double));

  } else {

    _recDbzDiffSq = NULL;
    _recDbzSpinChange = NULL;
    _recDbzTexture = NULL;
    _recSqrtTexture = NULL;
    _recDbzSpin = NULL;
    _recWtSpin = NULL;
    _recVel = NULL;
    _recVelSdev = NULL;
    _recWidth = NULL;
    _recClutDbzNarrow = NULL;
    _recClutRatioNarrow = NULL;
    _recClutRatioWide = NULL;
    _recClutWxPeakRatio = NULL;
    _recClutWxPeakSep = NULL;
    _recClutWxPeakSepSdev = NULL;
    _recZdrSdev = NULL;
    _recRhohvSdev = NULL;
    _rec = NULL;
    _recFlag = NULL;

  }

  for (int ii = 0; ii < _nGatesPulse; ii++) {
    GateSpectra *spec = new GateSpectra(_nSamples);
    _gateSpectra.push_back(spec);
  }

}

/////////////////////////////////////////////////////////
// free arrays

void Beam::_freeArrays()

{  

  _freeArray(_flags);
  _freeArray(_tripFlag);
  _freeArray(_vcensor);
  _freeArray(_censor);
  _freeArray(_zinfill);

  _freeArray(_powerDbm);
  _freeArray(_snr);

  _freeArray(_noiseDbm);
  _freeArray(_snrn);
  _freeArray(_dbzn);

  _freeArray(_dbzt);
  _freeArray(_dbz);
  _freeArray(_vel);
  _freeArray(_width);

  _freeArray(_clut);
  _freeArray(_dbzf);
  _freeArray(_velf);
  _freeArray(_widthf);

  _freeArray(_dbzi);
  _freeArray(_veli);
  _freeArray(_widthi);

  _freeArray(_leakage);
  _freeArray(_vtexture);
  _freeArray(_vspin);
  _freeArray(_fcensor);
  _freeArray(_ztexture);

  _freeArray(_phidp);
  _freeArray(_kdp);
  _freeArray(_zdr);
  _freeArray(_rhohv);

  _freeArray(_recDbzDiffSq);
  _freeArray(_recDbzSpinChange);
  _freeArray(_recDbzTexture);
  _freeArray(_recSqrtTexture);
  _freeArray(_recDbzSpin);
  _freeArray(_recWtSpin);
  _freeArray(_recVel);
  _freeArray(_recVelSdev);
  _freeArray(_recWidth);
  _freeArray(_recClutDbzNarrow);
  _freeArray(_recClutRatioNarrow);
  _freeArray(_recClutRatioWide);
  _freeArray(_recClutWxPeakRatio);
  _freeArray(_recClutWxPeakSep);
  _freeArray(_recClutWxPeakSepSdev);
  _freeArray(_recZdrSdev);
  _freeArray(_recRhohvSdev);
  _freeArray(_rec);
  _freeArray(_recFlag);

  for (int ii = 0; ii < _nGatesPulse; ii++) {
    delete _gateSpectra[ii];
  }

}

/////////////////////////////////////////////////////////
// free array

void Beam::_freeArray(int* &array)

{  
  if (array) {
    delete[] array;
    array = NULL;
  }
}

/////////////////////////////////////////////////////////
// free double array

void Beam::_freeArray(double* &array)

{  
  if (array) {
    delete array;
    array = NULL;
  }
}

/////////////////////////////////////////////////////////
// compute REC beam limits by checking surrounding beams

void Beam::_computeRecBeamLimits(const deque<Beam *> beams,
				 int midBeamIndex,
				 int &minBeamIndex,
				 int &maxBeamIndex) const
  
{
  
  minBeamIndex = midBeamIndex;
  maxBeamIndex = midBeamIndex;
  
  double midEl = getEl();
  double maxElDiff = 0.2;
  
  double prevAz = getAz();
  double maxAzDiff = _params.azimuth_resolution * 2.0;
  for (int ii = midBeamIndex - 1; ii >= 0; ii--) {
    const Beam *bb = beams[ii];
    if (bb->getMomentsMgr() != getMomentsMgr() ||
	bb->getNGatesOut() != getNGatesOut()) {
      break;
    }
    double az = bb->getAz();
    if (fabs(_computeAzDiff(az, prevAz)) > maxAzDiff) {
      break;
    }
    double el = bb->getEl();
    if (fabs(el - midEl) > maxElDiff) {
      break;
    }
    minBeamIndex = ii;
    prevAz = az;
  }
  
  prevAz = getAz();
  for (int ii = midBeamIndex + 1; ii < (int) beams.size(); ii++) {
    const Beam *bb = beams[ii];
    if (bb->getMomentsMgr() != getMomentsMgr() ||
	bb->getNGatesOut() != getNGatesOut()) {
      break;
    }
    double az = bb->getAz();
    if (fabs(_computeAzDiff(az, prevAz)) > maxAzDiff) {
      break;
    }
    double el = bb->getEl();
    if (fabs(el - midEl) > maxElDiff) {
      break;
    }
    maxBeamIndex = ii;
    prevAz = az;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "REC, el, az, minBeamIndex, maxBeamIndex: "
	 << getEl() << ", "
	 << getAz() << ", "
	 << minBeamIndex << ", " << maxBeamIndex << endl;
  }
    
}

/////////////////////////////////////////////////
// compute REC variables
//
// Assumes moments have been computed
    
void Beam::_computeRecVars()
  
{

  if (_recVarsReady) {
    return;
  }

  double spinThresh = _params.rec_spin_dbz_threshold;
  bool useDbzNarrow = _params.use_dbz_narrow_for_tdbz_and_spin;
  int spinSense = 0;
  
  for (int ii = 1; ii < _nGatesOut; ii++) {

    double dbz0 = 0, dbz1 = 0;

    if (_recClutDbzNarrow != NULL && useDbzNarrow) {
      dbz0 = _recClutDbzNarrow[ii-1];
      dbz1 = _recClutDbzNarrow[ii];
    } else {
      dbz0 = _dbz[ii-1];
      dbz1 = _dbz[ii];
    }
    
    if (dbz0 != _missingDbl && dbz1 != _missingDbl) {
      double dbzDiff = dbz1 - dbz0;
      _recDbzDiffSq[ii] = dbzDiff * dbzDiff;
      _recDbzSpinChange[ii] = 0;
      if (dbzDiff >= spinThresh) {
        if (spinSense != 1) {
          spinSense = 1;
          _recDbzSpinChange[ii] = 1;
        }
      } else if (dbzDiff <= -spinThresh) {
        if (spinSense != -1) {
          spinSense = -1;
          _recDbzSpinChange[ii] = 1;
        }
      }
    }

  } // ii

  _recDbzDiffSq[0] = _recDbzDiffSq[1];
  _recDbzSpinChange[0] = _recDbzSpinChange[1];

  _recVarsReady = true;

}

////////////////////////////////////////////////////////
// compute azimuth difference, taking account of wrapping
    
double Beam::_computeAzDiff(double az1, double az2) const
  
{

  double diff = az1 - az2;
  if (diff > 180.0) {
    diff -= 360.0;
  } else if (diff < -180.0) {
    diff += 360.0;
  }
  return diff;

}
  
///////////////////////////////////////
// compute mean velocity

double Beam::_computeMeanVelocity(double vel1, double vel2, double nyquist)

{

  double diff = fabs(vel1 - vel2);
  if (diff < nyquist) {
    return (vel1 + vel2) / 2.0;
  } else {
    if (vel1 > vel2) {
      vel2 += 2.0 * nyquist;
    } else {
      vel1 += 2.0 * nyquist;
    }
    double vel = (vel1 + vel2) / 2.0;
    if (vel > nyquist) {
      vel -= 2.0 * nyquist;
    } else if (vel < -nyquist) {
      vel += 2.0 * nyquist;
    }
    return vel;
  }

}

/////////////////////////////////////////
// compute velocity difference as an angle

double Beam::_velDiff2Angle(double vel1, double vel2, double nyquist)

{
  
  double ang1 = (vel1 / nyquist) * 180.0;
  double ang2 = (vel2 / nyquist) * 180.0;
  double adiff = (ang1 - ang2) - _params.H2V_phase_differential;
  if (adiff > 180.0) {
    adiff -= 360.0;
  } else if (adiff < -180.0) {
    adiff += 360.0;
  }
  return adiff;
}

/////////////////////////////////////////
// multiply complex values

Complex_t Beam::_complexProduct(Complex_t c1, Complex_t c2)

{
  Complex_t product;
  product.re = (c1.re * c2.re) - (c1.im - c2.im);
  product.im = (c1.im * c2.re) + (c1.re * c2.im);
  return product;
}

/////////////////////////////////////////////
// compute mean conjugate product of series

Complex_t Beam::_meanConjugateProduct(const Complex_t *c1,
				      const Complex_t *c2,
				      int len)

{
  
  double sumRe = 0.0;
  double sumIm = 0.0;

  for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
    sumRe = (c1->re * c2->re) + (c1->im * c2->im);
    sumIm = (c1->im * c2->re) - (c1->re * c2->im);
  }

  Complex_t product;
  product.re = sumRe / len;
  product.im = sumIm / len;

  return product;

}

/////////////////////////////////////////
// get velocity from angle of complex val

double Beam::_velFromComplexAng(Complex_t cVal,
				double nyquist)
  
{
  double angle = 0.0;
  if (cVal.re != 0.0 || cVal.im != 0.0) {
    angle = atan2(cVal.im, cVal.re);
  }
  double vel = (angle / M_PI) * nyquist;
  return vel;
}

#ifdef JUNK

////////////////////////////////////////////////////////////////////////
// Filter fields for spikes in dbz
//
// This routine filters the reflectivity data according to the
// NEXRAD specification DV1208621F, section 3.2.1.2.2, page 3-15.
//
// The algorithm is stated as follows:
//
// Clutter detection:
//
// The nth bin is declared to be a point clutter cell if its power value
// exceeds those of both its second nearest neighbors by a threshold
// value TCN. In other words:
//
//    if   P(n) exceeds TCN * P(n-2)
//    and  P(n) exceeds TCN * p(n+2)
//
//  where
//
//   TCN is the point clutter threshold factor, which is always
//       greater than 1, and typically has a value of 8 (9 dB)
//
//   P(n) if the poiwer sum value for the nth range cell
//
//   n is the range gate number
//
// Clutter censoring:
//
// The formulas for censoring detected strong point clutter in an
// arbitrary array A via data substitution are as follows. If the nth
// range cell is an isolated clutter cell (i.e., it si a clutter cell but
// neither of its immediate neighboring cells is a clutter cell) then the 
// replacement scheme is as follows:
//
//   Replace A(n-1) with  A(n-2)
//   Replace A(n)   with  0.5 * A(n-2) * A(n+2)
//   Replace A(n+1) with  A(n+2)
//
// If the nth and (n+1)th range bins constitute an isolated clutter pair,
// the bin replacement scheme is as follows:
//
//   Replace A(n-1) with  A(n-2)
//   Replace A(n)   with  A(n+2)
//   Replace A(n+1) with  A(n+3)
//   Replace A(n+2) with  A(n+3)
//
// Note that runs of more than 2 successive clutter cells cannot occur
// because of the nature of the algorithm.
 
void Beam::_filterSpikes(double *dbzf,
			 double *velf,
			 double *widthf)
  
{
  
  // set clutter threshold

  double tcn = 9.0;

  // loop through gates

  for (int ii = 2; ii < _nGatesOut - 3; ii++) {
    
    // check for clutter at ii and ii + 1

    bool this_gate = false, next_gate = false;
    
    if ((dbzf[ii] - dbzf[ii - 2]) > tcn &&
	(dbzf[ii] - dbzf[ii + 2]) > tcn) {
      this_gate = true;
    }
    if ((dbzf[ii + 1] - dbzf[ii - 1]) > tcn &&
	(dbzf[ii + 1] - dbzf[ii + 3]) > tcn) {
      next_gate = true;
    }

    if (this_gate) {

      if (!next_gate) {

	// only gate ii has clutter, substitute accordingly
	
	dbzf[ii - 1] = dbzf[ii - 2];
	dbzf[ii + 1] = dbzf[ii + 2];
	if (dbzf[ii - 2] == _missingDbl || dbzf[ii + 2] == _missingDbl) {
	  dbzf[ii] = _missingDbl;
	  velf[ii] = _missingDbl;
	  widthf[ii] = _missingDbl;
	} else {
	  // dbzf[ii] = (dbzf[ii - 2] + dbzf[ii + 2]) / 2;
	  dbzf[ii] = dbzf[ii - 2];
	  velf[ii] = velf[ii - 2];
	  widthf[ii] = widthf[ii - 2];
	}
	
      } else {

	// both gate ii and ii+1 has clutter, substitute accordingly

	dbzf[ii - 1] = dbzf[ii - 2];
	dbzf[ii]     = dbzf[ii - 2];
	dbzf[ii + 1] = dbzf[ii + 3];
	dbzf[ii + 2] = dbzf[ii + 3];

	velf[ii - 1] = velf[ii - 2];
	velf[ii]     = velf[ii - 2];
	velf[ii + 1] = velf[ii + 3];
	velf[ii + 2] = velf[ii + 3];

	widthf[ii - 1] = widthf[ii - 2];
	widthf[ii]     = widthf[ii - 2];
	widthf[ii + 1] = widthf[ii + 3];
	widthf[ii + 2] = widthf[ii + 3];

      }

    }
    
  } // ii

}


////////////////////////////////////////////////////////////////////////
// Filter dregs remaining after other filters
 
void Beam::_filterDregs(double nyquist,
			double *dbzf,
			double *velf,
			double *widthf)
  
{
  
  // compute vel diff array
  
  double veld[_nGatesOut];
  veld[0] = 0.0;
  
  for (int ii = 1; ii < _nGatesOut; ii++) {
    double diff;
    if (velf[ii] == _missingDbl || velf[ii -1] == _missingDbl) {
      diff = nyquist / 2.0;
    } else {
      diff = velf[ii] - velf[ii -1];
      if (diff > nyquist) {
	diff -= 2.0 * nyquist;
      } else if (diff < -nyquist) {
	diff += 2.0 * nyquist;
      }
    }
    veld[ii] = diff;
  } // ii
  
  for (int len = 20; len >= 6; len--) {

    int nHalf = len / 2;

    for (int ii = nHalf; ii < _nGatesOut - nHalf; ii++) {
      
      int istart = ii - nHalf;
      int iend = istart + len;
      
      if (dbzf[istart] != _missingDbl || dbzf[iend] != _missingDbl) {
	continue;
      }
      
      double sum = 0.0;
      double count = 0.0;
      int ndata = 0;
      for (int jj = istart; jj <= iend; jj++) {
	sum += veld[jj] * veld[jj];
	count++;
	if (dbzf[jj] != _missingDbl) {
	  ndata++;
	}
      } // jj
      
      if (ndata == 0) {
	continue;
      }

      double texture = sqrt(sum / count) / nyquist;
      double interest = _computeInterest(texture, 0.0, 0.75);

      if (interest > 0.5) {
	// cerr << " " << istart << "--" << iend << " ";
	for (int jj = istart; jj <= iend; jj++) {
	  //  	  if (dbzf[jj] != _missingDbl) {
	  //  	    cerr << " **" << jj << "** ";
	  //  	  }
	  dbzf[jj] = _missingDbl;
	  velf[jj] = _missingDbl;
	  widthf[jj] = _missingDbl;
	} // jj
      }

    } // ii
    
  } // len
  
}


////////////////////////////////////////////////////////////////////////
// Compute vel censoring interest value

void Beam::_computeVelCensoring(const double *vel,
				double nyquist,
				double *vtexture,
				double *vspin,
				int *vcensor)
  
{
  
  // compute vel diff array
  
  double veld[_nGatesOut];
  veld[0] = 0.0;

  for (int ii = 1; ii < _nGatesOut; ii++) {
    double diff;
    if (vel[ii] == _missingDbl || vel[ii -1] == _missingDbl) {
      diff = nyquist / 2.0;
    } else {
      diff = vel[ii] - vel[ii -1];
      if (diff > nyquist) {
	diff -= 2.0 * nyquist;
      } else if (diff < -nyquist) {
	diff += 2.0 * nyquist;
      }
    }
    veld[ii] = diff;
  } // ii
  
  int nGates = 5;
  int nGatesHalf = nGates / 2;

  memset(vtexture, 0, _nGatesOut * sizeof(double));
  for (int ii = nGatesHalf; ii < _nGatesOut - nGatesHalf; ii++) {
    double sum = 0.0;
    double count = 0.0;
    for (int jj = ii - nGatesHalf; jj <= ii + nGatesHalf; jj++) {
      sum += veld[jj] * veld[jj];
      count++;
    } // jj
    double texture = sqrt(sum / count) / nyquist;
    // double interest = _computeInterest(texture, 40.0, 70.0);
    double interest = _computeInterest(texture, 0.0, 0.75);
    // double interest = texture;
    vtexture[ii] = interest;
  } // ii
  
  double dtest = nyquist / 4.0;
  memset(vspin, 0, _nGatesOut * sizeof(double));
  for (int ii = nGatesHalf; ii < _nGatesOut - nGatesHalf; ii++) {
    double sum = 0.0;
    double count = 0.0;
    for (int jj = ii - nGatesHalf + 1; jj <= ii + nGatesHalf; jj++) {
      if (vel[ii] == _missingDbl || vel[ii -1] == _missingDbl) {
	sum++;
      } else {
	double mult = veld[jj-1] * veld[jj];
	if (mult < 0 && (fabs(veld[jj-1]) > dtest || fabs(veld[jj]) > dtest)) {
	  sum++;
	}
      }
      count++;
    } // jj
    double spin = sum / count;
    double interest = _computeInterest(spin, 0.0, 1.0);
    // double interest = spin;
    vspin[ii] = interest;
  } // ii

  memset(vcensor, 0, _nGatesOut * sizeof(int));
  for (int ii = 0; ii < _nGatesOut; ii++) {
    if (vtexture[ii] * vspin[ii] > 0.5) {
      vcensor[ii] = 1;
    }
  } // ii

#ifdef JUNK  
  // remove runs which are less than half the nGates

  int nCensor = 0;
  for (int ii = 0; ii < _nGatesOut; ii++) {
    if (ii == _nGatesOut - 1 || vcensor[ii] < 0.5) {
      if (nCensor > 0 && nCensor < nGatesHalf) {
	int iStart = ii - nCensor;
	int iEnd = ii - 1;
	for (int jj = iStart; jj <= iEnd; jj++) {
	  vcensor[jj] = 0.01;
	}
      }
      nCensor = 0;
    } else {
      nCensor++;
    }
  }
#endif
  
}

#endif

////////////////////////////////////////////////////////////////////////
// Compute Z texture

void Beam::_computeZTexture(const double *dbz,
			    double *ztexture)
  
{
  
  // compute dbz diff array
  
  double dbzd[_nGatesOut];
  dbzd[0] = 0.0;
  int dbzBase = -40;
  
  for (int ii = 1; ii < _nGatesOut; ii++) {
    if (dbz[ii - 1] != _missingDbl && dbz[ii] != _missingDbl) {
      double dbz0 = dbz[ii-1] - dbzBase;
      double dbz1 = dbz[ii] - dbzBase;
      double dbzMean = (dbz0 + dbz1) / 2.0;
      double diff = (dbz1 - dbz0) / dbzMean;
      dbzd[ii] = diff;
    } else {
      dbzd[ii] = _missingDbl;
    }
  } // ii
  
  // compute the z texture

  int nGates = 64;
  int nGatesHalf = nGates / 2;

  // int nGatesThreeQuarters = (nGates * 3) / 4;
  
  for (int ii = 0; ii < nGatesHalf - 1; ii++) {
    ztexture[ii] = _missingDbl;
  }
  for (int ii = _nGatesOut - nGatesHalf; ii < _nGatesOut; ii++) {
    ztexture[ii] = _missingDbl;
  }
  
  for (int ii = nGatesHalf; ii < _nGatesOut - nGatesHalf; ii++) {
    double sum = 0.0;
    double count = 0.0;
    for (int jj = ii - nGatesHalf; jj <= ii + nGatesHalf; jj++) {
      if (dbzd[jj] != _missingDbl) {
	sum += (dbzd[jj] * dbzd[jj]);
	count++;
      } // jj
    }
    double texture;
    if (count < nGatesHalf) {
      texture = _missingDbl;
    } else {
      texture = sqrt(sum / count);
    }
    // double interest = _computeInterest(texture, 40.0, 70.0);
    double interest = texture;
    ztexture[ii] = interest;
  } // ii

}

#ifdef JUNK

////////////////////////////////////////////////////////////////////////
// Perform infilling

void Beam::_performInfilling(const double *dbzf,
			     const double *velf,
			     const double *widthf,
			     const double *ztexture,
			     int *zinfill,
			     double *dbzi,
			     double *veli,
			     double *widthi)
  
{
  
  // set the infill flag

  for (int ii = 0; ii < _nGatesOut; ii++) {
    if (dbzf[ii] == _missingDbl &&
	ztexture[ii] != _missingDbl &&
	ztexture[ii] < 0.1) {
      zinfill[ii] = 1;
    } else {
      zinfill[ii] = 0;
    }
    // }
  }

  // perform infilling

  memcpy(dbzi, dbzf, _nGatesOut * sizeof(double));
  memcpy(veli, velf, _nGatesOut * sizeof(double));
  memcpy(widthi, widthf, _nGatesOut * sizeof(double));

  int nInfill = 0;
  for (int ii = 0; ii < _nGatesOut; ii++) {
    if (ii == _nGatesOut - 1 || zinfill[ii] == 0) {
      if (nInfill > 0) {
	int nHalf = nInfill / 2;
	int iLowStart = ii - nInfill;
	int iLowEnd = iLowStart + nHalf - 1;
	int iHighStart = iLowEnd + 1;
	int iHighEnd = ii - 1;
	for (int jj = iLowStart; jj <= iLowEnd; jj++) {
	  dbzi[jj] = dbzf[iLowStart - 1];
	  veli[jj] = velf[iLowStart - 1];
	  widthi[jj] = widthf[iLowStart - 1];
	}
	for (int jj = iHighStart; jj <= iHighEnd; jj++) {
	  dbzi[jj] = dbzf[ii];
	  veli[jj] = velf[ii];
	  widthi[jj] = widthf[ii];
	}
      }
      nInfill = 0;
    } else {
      nInfill++;
    }
  }  

}

#endif

////////////////////////////////////////////////////////////////////////
// Compute interest value

double Beam::_computeInterest(double xx,
			      double x0, double x1)
  
{

  if (xx <= x0) {
    return 0.01;
  }
  
  if (xx >= x1) {
    return 0.99;
  }
  
  double xbar = (x0 + x1) / 2.0;
  
  if (xx <= xbar) {
    double yy = (xx - x0) / (x1 - x0);
    double yy2 = yy * yy;
    double interest = 2.0 * yy2;
    return interest;
  } else {
    double yy = (x1 - xx) / (x1 - x0);
    double yy2 = yy * yy;
    double interest = 1.0 - 2.0 * yy2;
    return interest;
  }

}

#ifdef JUNK

////////////////////////////////////////////////////////////////////////
// Compute and print phase drift

void Beam::_printPhaseDrift(const Pulse *pulse)
  
{

  double burstI = pulse->getUnpacked()[0];
  double burstQ = pulse->getUnpacked()[1];
  double burstPhase = 0.0;
  if (burstI != 0.0 && burstQ != 0.0) {
    burstPhase = atan2(burstQ, burstI) * RAD_TO_DEG;
  }
  double phaseDiff = _prevBurstPhase - burstPhase;
  if (phaseDiff > 360.0) {
    phaseDiff -= 360.0;
  } else if (phaseDiff < 0.0) {
    phaseDiff += 360.0;
  }
  _prevBurstPhase = burstPhase;

  // phaseDiff = pulse->phaseDiff;
    
  double tmod = 0.0;
  if (phaseDiff > 350.0 && phaseDiff < 10.0) {
    tmod = 0.0;
  } else if (phaseDiff > 15.0 && phaseDiff < 30.0) {
    tmod = 22.5;
  } else if (phaseDiff > 80.0 && phaseDiff < 100.0) {
    tmod = 90.0;
  } else if (phaseDiff > 190.0 && phaseDiff < 220.0) {
    tmod = 202.5;
  }
  double terror = phaseDiff - tmod;
  if (terror > 180.0) {
    terror -= 360.0;
  } else if (terror < -180.0) {
    terror += 360.0;
  }
    
  if (_phaseCount > 0) {
    _phaseDrift += terror;
  }
  _phaseCount++;
    
  if (_params.debug) {
    cout << "el, az, phaseDiff, tmod, terror: " << pulse->getEl() << ", "
	 << pulse->getAz() << ", " << phaseDiff << ", " << tmod << ", " << terror << endl;
  }
    
  cout << _phaseCount << "  " << _phaseDrift << "  " << terror << "  "
       << tmod << "  " << pulse->getPhaseDiff() << endl;
  
  cout.flush();

}

#endif
    
////////////////////////////////////////////////////////////////////////
// Add spectrum to combined spectra output file

void Beam::_addSpectrumToFile(FILE *specFile, int count, time_t beamTime,
			      double el, double az, int gateNum,
			      double snr, double vel, double width,
			      int nSamples, const Complex_t *iq)
  
{

  date_time_t btime;
  btime.unix_time = beamTime;
  uconvert_from_utime(&btime);
  
  fprintf(specFile,
	  "%d %d %d %d %d %d %d %g %g %d %g %g %g %d",
	  count,
	  btime.year, btime.month, btime.day,
	  btime.hour, btime.min, btime.sec,
	  el, az, gateNum,
	  snr, vel, width, nSamples);

  for (int ii = 0; ii < nSamples; ii++) {
    fprintf(specFile, " %g %g", iq[ii].re, iq[ii].im);
  }
  
  fprintf(specFile, "\n");

}

////////////////////////////////////////////////////////////////////////
// Compute guess on power, to be used for clutter filtering

void Beam::_computeIsWx(Complex_t **IQ, bool *isWx)

{

  // fixed noise value

  double noiseFixed = pow(10.0, _params.radar.noise_value / 10.0);

  // compute dbz for each gate
  
  const double *rangeCorr = _momentsMgr->getRangeCorr();
  double dbz[_nGatesPulse];
  for (int igate = 0; igate < _nGatesPulse; igate++) {
    // const Complex_t *iq = IQ[igate];
    // double power = _moments.computePower(iq);
    double power = 0.0;
    if (power > noiseFixed) {
      double dbm = 10.0 * log10((power - noiseFixed) / noiseFixed);
      dbz[igate] = dbm + rangeCorr[igate];
    } else {
      dbz[igate] = _missingDbl;
    }
  } // igate

  // compute Z texture

  double zTexture[_nGatesPulse];
  _computeZTexture(dbz, zTexture);

  // set weather flag

  memset(isWx, 0, _nGatesPulse * sizeof(bool));
  for (int igate = 0; igate < _nGatesPulse; igate++) {
    // if (dbz[igate] != _missingDbl && zTexture[igate] < 0.1) {
    isWx[igate] = true;
    // }
  }

}

////////////////////////////////////////////////////////////////////////
// Convert interest map points to vector
//
// Returns 0 on success, -1 on failure

int Beam::_convertInterestMapToVector(const string &label,
				      const Params::interest_map_point_t *map,
				      int nPoints,
				      vector<InterestMap::ImPoint> &pts)

{

  pts.clear();

  double prevVal = -1.0e99;
  for (int ii = 0; ii < nPoints; ii++) {
    if (map[ii].value <= prevVal) {
      cerr << "ERROR - Beam::_convertInterestMapToVector" << endl;
      cerr << "  Map label: " << label << endl;
      cerr << "  Map values must increase monotonically" << endl;
      return -1;
    }
    InterestMap::ImPoint pt(map[ii].value, map[ii].interest);
    pts.push_back(pt);
  } // ii
  
  return 0;

}

