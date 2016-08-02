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
// Beam.hh
//
// Beam object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////

#ifndef Beam_hh
#define Beam_hh

#include <string>
#include <vector>
#include <cstdio>
#include "Params.hh"
#include "Complex.hh"
#include "Pulse.hh"
#include "MomentsMgr.hh"
#include "Fields.hh"
using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  // constructor

  Beam (const Params &params,
	const deque<Pulse *> pulse_queue,
	double az,
	MomentsMgr *momentsMgr);
  
  // destructor
  
  ~Beam();

  // Create interest maps.
  // These are static on the class, and should be created before any
  // beams are constructed.
 
  static int createInterestMaps(const Params &params);
  
  // compute moments
  
  void computeMoments();
  void computeMomentsSinglePol();
  void computeMomentsDualFastAlt();
  void computeMomentsDualCp2Xband();

  // set methods

  void setTargetEl(double el) { _targetEl = el; }

  // get methods

  int getNSamples() const { return _nSamples; }

  double getEl() const { return _el; }
  double getAz() const { return _az; }
  double getTargetEl() const { return _targetEl; }
  double getPrf() const { return _prf; }
  double getPrt() const { return _prt; }
  double getTime() const { return _time; }

  int getNGatesPulse() const { return _nGatesPulse; }
  int getNGatesOut() const { return _nGatesOut; }

  double getStartRange() const { return _momentsMgr->getStartRange(); }
  double getGateSpacing() const { return _momentsMgr->getGateSpacing(); }

  const MomentsMgr *getMomentsMgr() const { return _momentsMgr; }

  const Fields* getFields() const { return _fields; }

protected:
  
private:

  const Params &_params;
  
  int _nSamples;
  int _halfNSamples;
  
  vector<Pulse *> _pulses;
  Complex_t **_iqc;
  Complex_t **_iqhc;
  Complex_t **_iqhx;
  Complex_t **_iqvc;
  Complex_t **_iqvx;
  
  double _el;
  double _az;
  double _targetEl;

  double _prf;
  double _prt;
  
  double _time;

  int _nGatesPulse;
  int _nGatesOut;

  // manager for computing moments

  MomentsMgr *_momentsMgr;

  // moments fields at each gate

  Fields *_fields;

};

#endif

