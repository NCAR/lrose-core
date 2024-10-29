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
// HcrShortLongCombine.hh
//
// HcrShortLongCombine object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2014
//
//////////////////////////////////////////////////////////////////////////
//
// Combines multiple dwells from CfRadial files, writes out combined
// dwell files. The goal is to summarize dwells in pointing data - for
// example from vertically-pointing instruments. This can make displaying
// the data in a BSCAN quicker and more efficient.
//
//////////////////////////////////////////////////////////////////////////

#ifndef HcrShortLongCombine_HH
#define HcrShortLongCombine_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <radar/IwrfMomReader.hh>
class RadxFile;
class RadxRay;
using namespace std;

class HcrShortLongCombine {
  
public:

  // constructor
  
  HcrShortLongCombine (int argc, char **argv);

  // destructor
  
  ~HcrShortLongCombine();

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

  // reading fmq in realtime

  IwrfMomReader *_readerShort;
  IwrfMomReader *_readerLong;

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
  
  RadxTime _dwellStartTime;
  RadxTime _dwellEndTime;
  RadxTime _dwellMidTime;
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

  double _meanLatShort, _meanLonShort, _meanAltShort;
  double _meanLatLong, _meanLonLong, _meanAltLong;

  // methods

  int _runRealtime();
  int _runArchive();
  int _computeMeanLocation();

  int _openInputFmqs();
  int _openOutputFmq();
  int _openFileReaders();

  int _prepareInputRays();
  int _readNextDwell();
  int _checkForTimeGap(RadxRay *latestRayShort);
  RadxRay *_combineDwellRays();
  void _clearDwellRays();

  void _unfoldVel(RadxRay *rayCombined);

  void _computeVelCorrectedForVertMotion(RadxRay *ray,
                                         RadxField *velShort,
                                         RadxField *velLong,
                                         RadxField *velUnfolded);
  
  double _correctForNyquist(double vel, double nyquist);
  
  RadxRay *_readRayShort();
  RadxRay *_readRayLong();
  
  RadxField::StatsMethod_t
    _getDwellStatsMethod(Params::dwell_stats_method_t method);

  void _setPlatformMetadata(RadxPlatform &platform);

};

#endif
