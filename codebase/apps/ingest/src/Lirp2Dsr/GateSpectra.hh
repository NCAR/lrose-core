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
// GateSpectra.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2005
//
///////////////////////////////////////////////////////////////
//
// Spectra for individual gates.
//
// Used to store computed spectra and other details so that these
// do not need to be recomputed for the clutter filtering step.
//
////////////////////////////////////////////////////////////////

#ifndef GateSpectra_hh
#define GateSpectra_hh

#include <string>
#include <vector>
#include "Complex.hh"

using namespace std;

class GateSpectra {
  
public:

  GateSpectra(int nSamples);
  ~GateSpectra();

  void setTrip1IsStrong(bool state) { _trip1IsStrong = state; }
  void setCensorOnPowerRatio(bool state) { _censorOnPowerRatio = state; }
  void setPrtSecs(double secs) { _prtSecs = secs; }
  void setPowerStrong(double power) { _powerStrong = power; }
  void setVelStrong(double vel) { _velStrong = vel; }
  void setSpectrumOrig(const Complex_t *spec);
  void setSpectrumStrongTrip(const Complex_t *spec);
  void setMagWeakTrip(const double *mag);

  bool getTrip1IsStrong() const { return _trip1IsStrong; }
  bool getCensorOnPowerRatio() const { return _censorOnPowerRatio; }
  double getPrtSecs() const { return _prtSecs; }
  double getPowerStrong() const { return _powerStrong; }
  double getVelStrong() const { return _velStrong; }
  const Complex_t *getSpectrumOrig() const { return _spectrumOrig; }
  const Complex_t *getSpectrumStrongTrip() const { return _spectrumStrongTrip; }
  const double *getMagWeakTrip() const { return _magWeakTrip; }

protected:
private:

  int _nSamples;

  bool _trip1IsStrong;
  bool _censorOnPowerRatio;

  double _prtSecs;
  double _powerStrong;
  double _velStrong;

  Complex_t *_spectrumOrig;
  Complex_t *_spectrumStrongTrip;
  double *_magWeakTrip;
  

};

#endif

