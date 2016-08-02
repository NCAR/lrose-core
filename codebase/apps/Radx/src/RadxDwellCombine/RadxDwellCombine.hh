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
#include <Fmq/DsRadarQueue.hh>
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

  // fmq mode

  DsRadarQueue _inputFmq;
  DsRadarQueue _outputFmq;
  DsRadarMsg _inputMsg;
  DsRadarMsg _outputMsg;
  DsRadarParams _rparams;
  vector<DsPlatformGeoref> _georefs;
  bool _needWriteParams;
  int _inputContents;
  int _nRaysRead;
  int _nRaysWritten;

  // combining

  RadxVol _statsVol;
  RadxField::StatsMethod_t _dwellStatsMethod;
  RadxTime _dwellStartTime, _dwellEndTime;

  // censoring

  int _nWarnCensorPrint;

  // methods

  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  int _runFmq();

  int _processFile(const string &filePath);
  void _setupRead(RadxFile &file);
  void _applyLinearTransform(RadxVol &vol);
  void _convertFields(RadxVol &vol);
  void _convertAllFields(RadxVol &vol);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  int _writeVol(RadxVol &vol);
  int _combineDwells(RadxVol &vol);
  void _setDwellStatsMethod();

  int _readFmqMsg(bool &gotMsg);
  void _loadRadarParams();
  RadxRay *_createInputRay();
  int _writeParams(const RadxRay *ray);
  int _writeRay(const RadxRay *ray);

  bool _isOutputField(const string &name);

  Radx::SweepMode_t _getRadxSweepMode(int dsrScanMode);
  Radx::PolarizationMode_t _getRadxPolarizationMode(int dsrPolMode);
  Radx::FollowMode_t _getRadxFollowMode(int dsrMode);
  Radx::PrtMode_t _getRadxPrtMode(int dsrMode);

  int _getDsScanMode(Radx::SweepMode_t mode);
    
  void _censorFields(RadxVol &vol);
  void _censorRay(RadxRay *ray);

};

#endif
