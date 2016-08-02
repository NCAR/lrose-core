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
// GateSpectra.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2005
//
////////////////////////////////////////////////////////////////
//
// Spectra for individual gates.
//
// Used to store computed spectra and other details so that these
// do not need to be recomputed for the clutter filtering step.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cstring>
#include "GateSpectra.hh"
using namespace std;

// Constructor

GateSpectra::GateSpectra(int nSamples) :
  _nSamples(nSamples)
  
{
  _trip1IsStrong = true;
  _censorOnPowerRatio = false;
  _prtSecs = 0.001;
  _powerStrong = 0.0;
  _velStrong = 0.0;
  _iq = NULL;
  _iqWindowed = NULL;
  _iqH = NULL;
  _iqHWindowed = NULL;
  _iqV = NULL;
  _iqVWindowed = NULL;
  _spectrumOrig = NULL;
  _spectrumStrongTrip = NULL;
  _magWeakTrip = NULL;
}

// destructor

GateSpectra::~GateSpectra()

{
  if (_iq) {
    delete[] _iq;
  }
  if (_iqWindowed) {
    delete[] _iqWindowed;
  }
  if (_iqH) {
    delete[] _iqH;
  }
  if (_iqHWindowed) {
    delete[] _iqHWindowed;
  }
  if (_iqV) {
    delete[] _iqV;
  }
  if (_iqVWindowed) {
    delete[] _iqVWindowed;
  }
  if (_spectrumOrig) {
    delete[] _spectrumOrig;
  }
  if (_spectrumStrongTrip) {
    delete[] _spectrumStrongTrip;
  }
  if (_magWeakTrip) {
    delete[] _magWeakTrip;
  }
}

//////////////////
// set spectra

void GateSpectra::setIq(const Complex_t *iq)

{
  if (_iq == NULL) {
    _iq = new Complex_t[_nSamples];
  }
  memcpy(_iq, iq, _nSamples * sizeof(Complex_t));
}

void GateSpectra::setIqWindowed(const Complex_t *iq)

{
  if (_iqWindowed == NULL) {
    _iqWindowed = new Complex_t[_nSamples];
  }
  memcpy(_iqWindowed, iq, _nSamples * sizeof(Complex_t));
}

void GateSpectra::setIqH(const Complex_t *iq)

{
  if (_iqH == NULL) {
    _iqH = new Complex_t[_nSamples];
  }
  memcpy(_iqH, iq, _nSamples * sizeof(Complex_t));
}

void GateSpectra::setIqHWindowed(const Complex_t *iq)

{
  if (_iqHWindowed == NULL) {
    _iqHWindowed = new Complex_t[_nSamples];
  }
  memcpy(_iqHWindowed, iq, _nSamples * sizeof(Complex_t));
}

void GateSpectra::setIqV(const Complex_t *iq)

{
  if (_iqV == NULL) {
    _iqV = new Complex_t[_nSamples];
  }
  memcpy(_iqV, iq, _nSamples * sizeof(Complex_t));
}

void GateSpectra::setIqVWindowed(const Complex_t *iq)

{
  if (_iqVWindowed == NULL) {
    _iqVWindowed = new Complex_t[_nSamples];
  }
  memcpy(_iqVWindowed, iq, _nSamples * sizeof(Complex_t));
}

void GateSpectra::setSpectrumOrig(const Complex_t *spec)

{
  if (_spectrumOrig == NULL) {
    _spectrumOrig = new Complex_t[_nSamples];
  }
  memcpy(_spectrumOrig, spec, _nSamples * sizeof(Complex_t));
}

void GateSpectra::setSpectrumStrongTrip(const Complex_t *spec)

{
  if (_spectrumStrongTrip == NULL) {
    _spectrumStrongTrip = new Complex_t[_nSamples];
  }
  memcpy(_spectrumStrongTrip, spec, _nSamples * sizeof(Complex_t));
}

void GateSpectra::setMagWeakTrip(const double *mag)

{
  if (_magWeakTrip == NULL) {
    _magWeakTrip = new double[_nSamples];
  }
  memcpy(_magWeakTrip, mag, _nSamples * sizeof(double));
}

