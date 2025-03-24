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
// RadxDwellCombine.hh
//
// RadxDwellCombine object
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

#ifndef RadxDwellCombine_HH
#define RadxDwellCombine_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <rapformats/DsRadarMsg.hh>
#include <radar/IwrfMomReader.hh>
class RadxFile;
class RadxRay;
using namespace std;

class RadxDwellCombine {
  
public:

  // constructor
  
  RadxDwellCombine (int argc, char **argv);

  // destructor
  
  ~RadxDwellCombine();

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

  // reading input moments

  IwrfMomReader *_momReader;
  int _nRaysRead;

  RadxPlatform _platform;
  vector<RadxRcalib> _calibs;
  string _statusXml;
  vector<RadxEvent> _events;

  // fmq output

  DsFmq *_outputFmq;
  int _nRaysWritten;
  
  double _wavelengthM;
  double _prt;
  double _nyquist;
  
  // combining
  
  RadxVol _dwellVol;
  RadxField::StatsMethod_t _globalMethod;
  vector<RadxField::NamedStatsMethod> _namedMethods;
  RadxTime _dwellStartTime, _dwellEndTime;
  RadxTime _latestRayTime, _dwellMidTime;

  // output data on time boundaries

  RadxTime _nextEndOfVolTime;
  RadxVol _splitVol;
  
  // censoring

  int _nWarnCensorPrint;

  // fmq mode
  
  int _runFmq();
  int _openInputFmq();
  int _openOutputFmq();
  RadxRay *_readFmqRay();
  
  // file-based modes

  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  int _processFile(const string &filePath);
  void _setupRead(RadxFile &file);

  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  int _writeVol(RadxVol &vol);
  int _writeVolOnTimeBoundary(RadxVol &vol);
  int _writeSplitVol();
  void _setNextEndOfVolTime(RadxTime &refTime);

  // processing
  
  void _applyLinearTransform(RadxVol &vol);
  void _setFieldFoldsAttribute(RadxVol &vol);
  void _convertFields(RadxVol &vol);
  void _convertAllFields(RadxVol &vol);

  // combining dwells
  
  int _combineDwells(RadxVol &vol);
  int _combineDwellsCentered(RadxVol &vol);
  RadxField::StatsMethod_t
    _getDwellStatsMethod(Params::dwell_stats_method_t method);

  // censoring
  
  void _censorFields(RadxVol &vol);
  void _censorRay(RadxRay *ray);

  // modify platform metadata
  
  void _setPlatformMetadata(RadxPlatform &platform);

};

#endif
