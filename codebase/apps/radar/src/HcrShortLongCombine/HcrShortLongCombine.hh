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
  vector<string> _readPaths;

  // reading fmq in realtime

  IwrfMomReader *_readerShort;
  IwrfMomReader *_readerLong;

  // Radx output moments queue

  DsFmq *_outputFmq;

  DsRadarParams _rparams;
  vector<DsPlatformGeoref> _georefs;
  bool _needWriteParams;
  int _inputContents;
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

  // combining

  double _dwellLengthSecs;
  double _dwellLengthSecsHalf;
  
  RadxTime _dwellStartTime;
  RadxTime _dwellEndTime;
  RadxTime _dwellMidTime;
  RadxTime _latestRayTime;
  
  RadxRay *_cacheRayShort;
  RadxRay *_cacheRayLong;

  vector<RadxRay *> _dwellRaysShort;
  vector<RadxRay *> _dwellRaysLong;

  RadxVol _dwellVolShort;
  RadxVol _dwellVolLong;
  RadxField::StatsMethod_t _globalMethod;
  vector<RadxField::NamedStatsMethod> _namedMethods;

  // output data on time boundaries

  RadxTime _nextEndOfVolTime;
  RadxVol _splitVol;

  // methods

  int _runRealtime();
  int _runArchive();

  int _openFmqs();
  int _prepareInputRays();
  int _readNextDwell();
  RadxRay *_combineDwellRays();
  void _clearDwellRays();
  void _unfoldVel(RadxRay *rayCombined);
  RadxRay *_readRayShort();
  RadxRay *_readRayLong();
  
  int _openFileReaders();
  int _processFile(const string &filePath);
  void _setupRead(RadxFile &file);
  void _convertFields(RadxVol &vol);
  void _convertAllFields(RadxVol &vol);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);

  int _writeVol(RadxVol &vol);
  int _writeVolOnTimeBoundary(RadxVol &vol);
  int _writeSplitVol();
  void _setNextEndOfVolTime(RadxTime &refTime);

  int _combineDwells(RadxVol &vol);
  int _combineDwellsCentered(RadxVol &vol);
  RadxField::StatsMethod_t
    _getDwellStatsMethod(Params::dwell_stats_method_t method);

  int _writeParams(const RadxRay *ray);
  int _writeRay(const RadxRay *ray);

  bool _isOutputField(const string &name);

  Radx::SweepMode_t _getRadxSweepMode(int dsrScanMode);
  Radx::PolarizationMode_t _getRadxPolarizationMode(int dsrPolMode);
  Radx::FollowMode_t _getRadxFollowMode(int dsrMode);
  Radx::PrtMode_t _getRadxPrtMode(int dsrMode);

  int _getDsScanMode(Radx::SweepMode_t mode);
    
};

#endif
