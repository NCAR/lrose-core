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
//////////////////////////////////////////////////////////
// SweepData - class that contains and manages data
//            specific to a single tilt or sweep
//
// $Id: SweepData.hh,v 1.10 2016/03/07 01:23:01 dixon Exp $
//
/////////////////////////////////////////////////////////

#ifndef _SWEEP_DATA_
#define _SWEEP_DATA_

#include <vector>
#include <deque>

#include "Params.hh"
#include "Gemtronik2Netcdf.hh"
#include "ReadGemtronik.hh"

//
// Forward class declarations
//
class Beam;

class SweepData 
{
public:

   //
   // Constructor
   //   tdrpParams = reference to tdrp parameters
   //
   SweepData( Params *tdrpParams );

   //
   // Destructor
   //
   ~SweepData();


  void clear();

  void setInfo(int elevNum, float elevation, const char *date, const char *time, int nrays,
	       float bWidth, float wLen);

  void setData(ReadGemtronik::VolumeEnum_t dataType, unsigned char* data, unsigned long int dataSize,
	       float *rays, int nrays, int nbins, float datamin, float datascale,
	       float startrange, float rangestep, const char *date, const char *time,
	       int pulsecount, float stagger, int prf, float snr_thresh, int long_pulse);

  void mergeDbz(SweepData *prevSweep);

  bool complete();

  void finishSweep();

  //
  // Get functions
  //
  vector<Beam*>& getBeams(){ return _beams; }
  int            getNBeams(){ return _beams.size(); }
  int            getVCP(){ return _vcp; }
  bool           getCalcSnr(){ return _calcSnr; }
  bool           getCalcPr(){ return _calcPr; }
  bool           getCalcRec(){ return _calcRec; }
  float          getFixedAngle(){ return _fixedAngle; }
  float          getElevation() const { return _fixedAngle; }
  double         getTime() const { return _scantime; }
  float          getNyquistVelocity(){ return _nyquistVelocity; }
  float          getUnambiguousRange(){ return _unambiguousRange; }
  float          getBeamWidth() const { return _beamWidth; }
  float          getWaveLength() const { return _waveLen; }
  int            getLongPulse() const { return _long_pulse; }
  float          getStagger() { return _stagger; }
  float          getPrf() { return _dop_prf; }
  float          getSurPrf() { return _sur_prf; }
  float          getPulseCount() { return _dop_pulsecount; }
  float          getSurPulseCount() { return _sur_pulsecount; }
  float          getDbz0() { return _dbz0; }
  bool           merged() { return _merged; }

  //
  // Constants
  //  SPEED_OF_LIGHT  = speed of light in m/s
  static const double SPEED_OF_LIGHT;

private:

  void computeRec();

  void computeMinDbz0();

  void fillRec();

  Beam* getClosestBeam(float currentAz);

  time_t date2utime(const char *date, const char *time);


  //
  // Reference to tdrp parameters
  //
  Params *_params;

  double _scantime;
  int _nbeams;
  float _beamWidth;
  float _waveLen;

  int   _elevIndex;
  int   _vcp;
  float _unambiguousRange;
  float _fixedAngle;
  float _nyquistVelocity;
  bool  _sur;
  float _sur_prf;
  int   _sur_antspeed;
  int   _sur_pulsecount;
  bool  _dop;
  float _dop_prf;
  int   _dop_antspeed;
  int   _dop_pulsecount;
  float _dbz0;
  float _stagger;
  float _horiz_noise;
  float _vert_noise;
  bool  _sw;
  int   _long_pulse;

  //
  // User request to calc Rec, Snr, and Pr
  bool _calcRec;
  bool _calcSnr;
  bool _calcPr;
  bool _merged;
  //
  // Vector of beams
  //
  vector< Beam* > _beams;

  //
  // Rec computation 
  //   applyRec         = compute the rec or not?
  //   maxBeamQueueSize = maximum number of beams allowd in the queue
  //   beamQueue        = list of beams needed for rec computation
  //
  bool          _applyRec;
  int           _midBeamIndex;
  int           _maxBeamQueueSize;
  deque<Beam *> _beamQueue;

  //
  // Azimuth, elevation and time tolerances
  //   
  double _azTolerance;
  double _elevTolerance;
  double _timeTolerance;

};

#endif
