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
  _spectrumOrig = NULL;
  _spectrumStrongTrip = NULL;
  _magWeakTrip = NULL;
}

// destructor

GateSpectra::~GateSpectra()

{
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

