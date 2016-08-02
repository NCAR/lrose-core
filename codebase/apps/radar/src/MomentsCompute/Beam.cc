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
#include "Beam.hh"
#include "umalloc.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

Beam::Beam(const Params &params,
           const deque<Pulse *> pulse_queue,
           double az,
           MomentsMgr *momentsMgr) :
        _params(params),
        _az(az),
        _momentsMgr(momentsMgr)
  
{

  _nSamples = _momentsMgr->getNSamples();
  
  // load up pulse vector

  int offset = 0;
  if (_momentsMgr->getMode() == Params::DUAL_FAST_ALT) {
    // for alternating dual pol data, we check the HV flag to
    // determine whether pulse 0 is horizontally or vertically polarized.
    // If vertical, go back by one to start with horizontal
    const Pulse *pulse = pulse_queue[_nSamples-1];
    if (!pulse->isHoriz()) {
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
  _iqc = NULL;
  _iqhc = NULL;
  _iqhx = NULL;
  _iqvc = NULL;
  _iqvx = NULL;

  // compute mean prt

  double sumPrt = 0.0;
  for (int ii = 0; ii < _nSamples; ii++) {
    sumPrt += _pulses[ii]->getPrt();
  }
  _prt = sumPrt / _nSamples;
  _prf = 1.0 / _prt;

  // set time
  _time = _pulses[_halfNSamples]->getTime();

  // set elevation

  _el = _pulses[_halfNSamples]->getEl();
  if (_el > 180.0) {
    _el -= 360.0;
  }
  _targetEl = _el;

  // compute number of output gates

  _nGatesPulse = _pulses[_halfNSamples]->getNGates();
  _nGatesOut = _nGatesPulse;

  // fields at each gate

  _fields = new Fields[_nGatesOut];

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

  if (_iqc) {
    Umalloc::ufree2((void **) _iqc);
  }

  if (_iqhc) {
    Umalloc::ufree2((void **) _iqhc);
  }

  if (_iqhx) {
    Umalloc::ufree2((void **) _iqhx);
  }

  if (_iqvc) {
    Umalloc::ufree2((void **) _iqvc);
  }
  
  if (_iqvx) {
    Umalloc::ufree2((void **) _iqvx);
  }
  
  if (_fields) {
    delete[] _fields;
  }

}

/////////////////////////////////////////////////
// compute moments
//
// Returns 0 on success, -1 on failure

void Beam::computeMoments()
  
{
  
  if (_momentsMgr->getMode() == Params::SINGLE_POL) {
    
    computeMomentsSinglePol();

  } else if (_momentsMgr->getMode() == Params::DUAL_FAST_ALT) {
    
    computeMomentsDualFastAlt();

  } else if (_momentsMgr->getMode() == Params::DUAL_CP2_XBAND) {
    
    computeMomentsDualCp2Xband();

  }

}

/////////////////////////////////////////////////
// compute moments - single pol
    
void Beam::computeMomentsSinglePol()
  
{

  // get pulse IQ copol data
  
  const float **IQC = new const float *[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    IQC[ii] = _pulses[ii]->getIqc();
  }
  
  // load up IQ data array
  
  if (_iqc) {
    Umalloc::ufree2((void **) _iqc);
  }
  _iqc = (Complex_t **)
    Umalloc::ucalloc2(_nGatesPulse, _nSamples, sizeof(Complex_t));
  int posn = 0;
  for (int igate = 0; igate < _nGatesPulse; igate++, posn += 2) {
    Complex_t *iqc = _iqc[igate];
    for (int isamp = 0; isamp < _nSamples; isamp++, iqc++) {
      iqc->re = IQC[isamp][posn];
      iqc->im = IQC[isamp][posn + 1];
    } // isamp
  } // igate
  
  // compute moments

  _momentsMgr->computeSingle(_time, _el, _az, _prt,
                             _nGatesPulse, _iqc, _fields);

  // clean up

  delete[] IQC;

}

/////////////////////////////////////////////////
// compute moments - dual pol fast alternating
//
// Returns 0 on success, -1 on failure
void 
Beam::computeMomentsDualFastAlt()
{

  // get pulse IQ copol data
  
  const float **IQC = new const float *[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    IQC[ii] = _pulses[ii]->getIqc();
  }
  
  // load up IQH and IQV data arrays
  
  int nHalf = _nSamples / 2;
  if (_iqhc) {
    Umalloc::ufree2((void **) _iqhc);
  }
  if (_iqvc) {
    Umalloc::ufree2((void **) _iqvc);
  }
  _iqhc = (Complex_t **)
    Umalloc::ucalloc2(_nGatesPulse, nHalf, sizeof(Complex_t));
  _iqvc = (Complex_t **)
    Umalloc::ucalloc2(_nGatesPulse, nHalf, sizeof(Complex_t));

for (int igate = 0, posn = 0; igate < _nGatesPulse; igate++, posn += 2) {
    Complex_t *iqhc = _iqhc[igate];
    Complex_t *iqvc = _iqvc[igate];
    for (int isamp = 0; isamp < _nSamples; iqhc++, iqvc++) {
      iqhc->re = IQC[isamp][posn];
	  iqhc->im = IQC[isamp][posn + 1];
      isamp++;
      iqvc->re = IQC[isamp][posn];
      iqvc->im = IQC[isamp][posn + 1];
      isamp++;
    } // isamp
  } // igate

  _momentsMgr->computeDualFastAlt(_time, _el, _az, _prt,
                                  _nGatesPulse, _iqhc, _iqvc, _fields);
  
  delete[] IQC;

}

/////////////////////////////////////////////////
// compute moments - CP2 Xband data
//
// Returns 0 on success, -1 on failure
    
void Beam::computeMomentsDualCp2Xband()
  
{

  // get pulse IQC copol data
  
  const float **IQC = new const float *[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    IQC[ii] = _pulses[ii]->getIqc();
  }
  
  // get pulse IQX xpol data
  
  const float **IQX = new const float *[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    IQX[ii] = _pulses[ii]->getIqx();
  }
  
  // load up IQHC data array
  
  if (_iqhc) {
    Umalloc::ufree2((void **) _iqhc);
  }
  _iqhc = (Complex_t **)
    Umalloc::ucalloc2(_nGatesPulse, _nSamples, sizeof(Complex_t));
  for (int igate = 0, posn = 0; igate < _nGatesPulse; igate++, posn += 2) {
    Complex_t *iqhc = _iqhc[igate];
    for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++) {
      iqhc->re = IQC[isamp][posn];
      iqhc->im = IQC[isamp][posn + 1];
    } // isamp
  } // igate
  
  // load up IQVX data array
  
  if (_iqvx) {
    Umalloc::ufree2((void **) _iqvx);
  }
  _iqvx = (Complex_t **)
    Umalloc::ucalloc2(_nGatesPulse, _nSamples, sizeof(Complex_t));
  for (int igate = 0, posn = 0; igate < _nGatesPulse; igate++, posn += 2) {
    Complex_t *iqvx = _iqvx[igate];
    for (int isamp = 0; isamp < _nSamples; isamp++, iqvx++) {
      iqvx->re = IQX[isamp][posn];
      iqvx->im = IQX[isamp][posn + 1];
    } // isamp
  } // igate
  
  _momentsMgr->computeDualCp2Xband(_time, _el, _az, _prt,
                                   _nGatesPulse, _iqhc, _iqvx, _fields);
  
  delete[] IQC;
  delete[] IQX;

}

