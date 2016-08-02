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
#include <toolsa/mem.h>
#include "Beam.hh"
using namespace std;

const double Beam::_missingDbl = -9999.0;

////////////////////////////////////////////////////
// Constructor

Beam::Beam(const string &prog_name,
	   const Params &params,
	   const deque<Pulse *> pulse_queue,
	   double az,
	   MomentsMgr *momentsMgr) :
  _progName(prog_name),
  _params(params),
  _az(az),
  _momentsMgr(momentsMgr)

{

  _maxTrips = 4;
  _nSamples = _momentsMgr->getNSamples();
  _dualPolAlternating = false;

  // optionally check for dual pol alternating mode
  
  if (_params.check_for_dual_pol_alternating_mode) {
    int nh = 0, nv = 0;
    for (int ii = 0; ii < _nSamples; ii++) {
      if (pulse_queue[ii]->getHvFlag()) {
        nv++;
      } else {
        nh++;
      }
    }
    if (nv == nh) {
      _dualPolAlternating = true;
    }
  }
  
  if (_dualPolAlternating && _params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "------>> Dual pol alternating mode" << endl;
  }

  // load up pulse vector

  int offset = 0;
  if (_dualPolAlternating) {
    // for dual pol data, we check the HV flag to determine whether pulse 0
    // is horizontal or vertical polarized. If vertical, go back by one to
    // start with horizontal.
    const Pulse *pulse = pulse_queue[_nSamples-1];
    if (!pulse->getHvFlag()) { // HV flag is 1 for vertical
      offset = 1;
    }
  }
  
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = _nSamples - ii - 1 + offset;
    Pulse *pulse = pulse_queue[jj];
    _pulses.push_back(pulse);
    pulse->addClient("Beam::Beam");
  }
  
  // initialize
  
  _halfNSamples = _nSamples / 2;
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
  _dtime = _pulses[_halfNSamples]->getTime();

  // set elevation

  _el = _pulses[_halfNSamples]->getEl();
  if (_el > 180.0) {
    _el -= 360.0;
  }
  _targetEl = _el;

  // compute number of output gates

  _nGatesPulse = _pulses[_halfNSamples]->getNGates();
  _nGatesOut = _nGatesPulse;

  // alloc field arrays
  
  _flags = NULL;
  _powerDbm = NULL;
  _snr = NULL;
  _dbz = NULL;
  _vel = NULL;
  _width = NULL;
  _aiq = NULL;
  _niq = NULL;
  _meanI = NULL;
  _meanQ = NULL;
  _aiqVpol = NULL;
  _niqVpol = NULL;
  _meanIVpol = NULL;
  _meanQVpol = NULL;

  _allocArrays();

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

/////////////////////////////////////////////////
// compute moments
//
// Returns 0 on success, -1 on failure
    
int Beam::computeMoments()
  
{

  int iret = 0;

  if (_dualPolAlternating) {
    
    if (_computeMomentsAlternating()) {
      iret = -1;
    }

  } else {

    if (_computeMomentsSinglePol()) {
      iret = -1;
    }

  }

  return iret;

}

/////////////////////////////////////////////////
// compute moments - single pol mode
//
// Returns 0 on success, -1 on failure
    
int Beam::_computeMomentsSinglePol()
  
{

  // get unpacked pulses
  
  const fl32* iqData[_nSamples];
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
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "-->> Computing single pol moments, el, az, nGatesPulse, time: "
  	 << _el << ", " << _az << ", " << _nGatesPulse << ", "
  	 << DateTime::str(_time) << endl;
  }

  _momentsMgr->computeMoments(_prt, _nGatesPulse, _iq,
                              _powerDbm, _snr, _dbz,
                              _vel, _width, _flags);

  _momentsMgr->computeAiqNiq(_nGatesPulse, _nSamples,
                             _iq, _aiq, _niq, _meanI, _meanQ);
  
  return 0;

}

/////////////////////////////////////////////////
// compute moments in alternating mode
//
// Returns 0 on success, -1 on failure
    
int Beam::_computeMomentsAlternating()
  
{

  // get unpacked pulses
  
  const fl32* iqData[_nSamples];
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

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "-->> Computing alternating moments, el, az, nGatesPulse, time: "
  	 << _el << ", " << _az << ", " << _nGatesPulse << ", "
  	 << DateTime::str(_time) << endl;
  }

  _momentsMgr->computeAlternating(_prt, _nGatesPulse, _iq, _iqh, _iqv,
				  _powerDbm, _snr, _dbz,
				  _vel, _width, _flags);
  
  _momentsMgr->computeAiqNiq(_nGatesPulse, nDual, _iqh,
                             _aiq, _niq, _meanI, _meanQ);
  
  _momentsMgr->computeAiqNiq(_nGatesPulse, nDual, _iqv,
                             _aiqVpol, _niqVpol, _meanIVpol, _meanQVpol);
  
  return 0;

}

/////////////////////////////////////////////////////////
// alloc arrays

void Beam::_allocArrays()

{  

  // field arrays
  
  _flags = new int[_nGatesOut];
  _powerDbm = new double[_nGatesOut];
  _snr = new double[_nGatesOut];
  _dbz = new double[_nGatesOut];
  _vel = new double[_nGatesOut];
  _width = new double[_nGatesOut];
  _aiq = new double[_nGatesOut];
  _niq = new double[_nGatesOut];
  _meanI = new double[_nGatesOut];
  _meanQ = new double[_nGatesOut];
  if (_dualPolAlternating) {
    _aiqVpol = new double[_nGatesOut];
    _niqVpol = new double[_nGatesOut];
    _meanIVpol = new double[_nGatesOut];
    _meanQVpol = new double[_nGatesOut];
  }

  // initialize integer arrays to 0
  
  memset(_flags, 0, _nGatesOut * sizeof(int));
  
  // initialize double arrays to missing
  
  for (int ii = 0; ii < _nGatesOut; ii++) {
    _powerDbm[ii] = _missingDbl;
  }

  memcpy(_snr, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_dbz, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_vel, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_width, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_aiq, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_niq, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_meanI, _powerDbm, _nGatesOut * sizeof(double));
  memcpy(_meanQ, _powerDbm, _nGatesOut * sizeof(double));
  if (_dualPolAlternating) {
    memcpy(_aiqVpol, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_niqVpol, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_meanIVpol, _powerDbm, _nGatesOut * sizeof(double));
    memcpy(_meanQVpol, _powerDbm, _nGatesOut * sizeof(double));
  }

}

/////////////////////////////////////////////////////////
// free arrays

void Beam::_freeArrays()

{  

  _freeArray(_flags);
  _freeArray(_powerDbm);
  _freeArray(_snr);
  _freeArray(_dbz);
  _freeArray(_vel);
  _freeArray(_width);
  _freeArray(_aiq);
  _freeArray(_niq);
  _freeArray(_meanI);
  _freeArray(_meanQ);
  _freeArray(_aiqVpol);
  _freeArray(_niqVpol);
  _freeArray(_meanIVpol);
  _freeArray(_meanQVpol);

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
    delete[] array;
    array = NULL;
  }
}

