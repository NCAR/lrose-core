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
using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  // constructor

  Beam (const string &prog_name,
	const Params &params,
	const deque<Pulse *> pulse_queue,
	double az,
	MomentsMgr *momentsMgr);
  
  // destructor
  
  ~Beam();

  // compute moments
  //
  // Returns 0 on success, -1 on failure
  
  int computeMoments();

  // set methods

  void setTargetEl(double el) { _targetEl = el; }

  // get methods

  int getNSamples() const { return _nSamples; }
  int get_maxTrips() const { return _maxTrips; }
  double getEl() const { return _el; }
  double getTargetEl() const { return _targetEl; }
  double getAz() const { return _az; }
  double getPrf() const { return _prf; }
  double getPrt() const { return _prt; }
  time_t getTime() const { return _time; }
  double getDoubleTime() const { return _dtime; }
  int getNGatesPulse() const { return _nGatesPulse; }
  int getNGatesOut() const { return _nGatesOut; }
  MomentsMgr *getMomentsMgr() const { return _momentsMgr; }
  
  const int* getFlags() const { return _flags; }
  const double* getPowerDbm() const { return _powerDbm; }
  const double* getSnr() const { return _snr; }

  const double* getDbz() const { return _dbz; }
  const double* getVel() const { return _vel; }
  const double* getWidth() const { return _width; }

  const double* getAiq() const { return _aiq; }
  const double* getNiq() const { return _niq; }
  const double* getMeanI() const { return _meanI; }
  const double* getMeanQ() const { return _meanQ; }

  const double* getAiqVpol() const { return _aiqVpol; }
  const double* getNiqVpol() const { return _niqVpol; }
  const double* getMeanIVpol() const { return _meanIVpol; }
  const double* getMeanQVpol() const { return _meanQVpol; }

protected:
  
private:

  static const double _missingDbl;

  string _progName;
  const Params &_params;
  int _maxTrips;
  
  int _nSamples;
  int _halfNSamples;
  
  vector<Pulse *> _pulses;
  Complex_t **_iq;
  Complex_t **_iqh;
  Complex_t **_iqv;

  bool _dualPolAlternating;
  
  double _el;
  double _targetEl;
  double _az;

  double _prf;
  double _prt;
  
  time_t _time;
  double _dtime;

  int _nGatesPulse;
  int _nGatesOut;

  MomentsMgr *_momentsMgr;
  
  int *_flags;

  double *_powerDbm;
  double *_snr;
  
  double *_dbz;
  double *_vel;
  double *_width;

  double *_aiq;
  double *_niq;
  double *_meanI;
  double *_meanQ;

  double *_aiqVpol;
  double *_niqVpol;
  double *_meanIVpol;
  double *_meanQVpol;

  // private functions
  
  void _allocArrays();
  void _freeArrays();
  void _freeArray(int* &array);
  void _freeArray(double* &array);

  int _computeMomentsSinglePol();
  int _computeMomentsAlternating();

};

#endif

