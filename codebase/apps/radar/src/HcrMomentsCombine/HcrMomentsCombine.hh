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
//////////////////////////////////////////////////////////////////////////
// HcrMomentsCombine.hh
//
// HcrMomentsCombine object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2026
//
//////////////////////////////////////////////////////////////////////////
//
// HCR has the capability to transmit blocks of pulses with varying PRTs 
//   and pulse lengths.
//
// The latest version supports 3 block types:
//
//      (1) short-pulse and short-PRT
//      (2) long-pulse and long-PRT
//      (3) long-pulse and short-PRT.
//
// This sequence is repeated in time.
//
// HcrTs2Moments reads this interleaved time series, and computes the 
//   relevant moments for each block. Those moments are then written, in 
//   sequence, to a single output FMQ in Radx moments format.
//
// HcrMomentsCombine reads the Radx moments data stream, and combines the 
//   three blocks into a single block, naming the fields appropriately, 
//   and unfolding the velocity fields as appropriate. This allows us to 
//   unfold the velocity field using the staggered-PRT technique.
//
//////////////////////////////////////////////////////////////////////////

#ifndef HcrMomentsCombine_HH
#define HcrMomentsCombine_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <radar/IwrfMomReader.hh>
class RadxFile;
class RadxRay;
using namespace std;

class HcrMomentsCombine {
  
public:

  // constructor
  
  HcrMomentsCombine (int argc, char **argv);

  // destructor
  
  ~HcrMomentsCombine();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // reading input moments

  IwrfMomReader *_momReader;

  // Radx output moments queue

  DsFmq *_outputFmq;

  int _nRaysRead;
  int _nRaysWritten;

  RadxPlatform _platformShort;
  RadxPlatform _platformLong;
  vector<RadxRcalib> _calibsShort;
  vector<RadxRcalib> _calibsLong;
  string _statusXmlShort;
  string _statusXmlLong;
  vector<RadxEvent> _eventsShort;
  vector<RadxEvent> _eventsLong;

  double _wavelengthM;
  double _prtShort;
  double _prtLong;
  double _nyquistShort;
  double _nyquistLong;
  double _nyquistUnfolded;

  int _stagM;
  int _stagN;
  int _LL;
  int _PP_[32];
  
  // combining

  double _dwellLengthSecs;
  double _dwellLengthSecsHalf;
  
  RadxTime _nextDwellStartTime;
  RadxTime _nextDwellEndTime;
  RadxTime _nextDwellMidTime;
  RadxTime _thisDwellMidTime;

  RadxTime _latestRayTime;
  RadxTime _prevTimeShort;
  
  RadxRay *_cacheRayShort;
  RadxRay *_cacheRayLong;
  
  vector<RadxRay *> _dwellRaysShort;
  vector<RadxRay *> _dwellRaysLong;

  RadxVol _dwellVolShort;
  RadxVol _dwellVolLong;
  RadxField::StatsMethod_t _globalMethod;
  vector<RadxField::NamedStatsMethod> _namedMethods;

  double _meanLat, _meanLon, _meanAlt;

  // methods

  int _runRealtime();
  int _runArchive();
  int _computeMeanLocation();

  int _openInputFmqs();
  int _openOutputFmq();
  int _openFileReader();

  int _prepareInputRays();
  int _readNextDwell();
  int _checkForTimeGap(RadxRay *latestRayShort);
  RadxRay *_combineDwellRays();
  void _clearDwellRays();

  void _unfoldVel(RadxRay *rayCombined);

  void _computeVelCorrectedForVertMotion(RadxRay *ray,
                                         RadxField *velRawShort,
                                         RadxField *velRawLong,
                                         RadxField *velUnfolded);
  
  double _correctForNyquist(double vel, double nyquist);
  
  RadxRay *_readRayNext();
  RadxRay *_readRayLong();
  
  RadxField::StatsMethod_t
    _getDwellStatsMethod(Params::dwell_stats_method_t method);

};

#endif
