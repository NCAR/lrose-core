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
// Dsr2Vol.hh
//
// Dsr2Vol object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////
//
// Dsr2Vol reads an input radar FMQ, puts the data into a grid, and
// saves it out as an MDV file.
//
///////////////////////////////////////////////////////////////////////

#ifndef Dsr2Vol_H
#define Dsr2Vol_H

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "Args.hh"
#include "Params.hh"
#include "FieldInfo.hh"
#include "Beam.hh"
#include "BeamGeomMgr.hh"
#include "PlanTransform.hh"
#include "PpiMgr.hh"
#include "Antenna.hh"
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>

using namespace std;

////////////////////////
// This class

class Dsr2Vol {
  
public:

  // constructor

  Dsr2Vol (int argc, char **argv);

  // destructor
  
  ~Dsr2Vol();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // members

  static const double _smallVal;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  
  vector<DsRadarParams *> _radarParamsVec;
  DsRadarParams *_latestRadarParams;
  DsRadarCalib *_calib;
  string _statusXml;
  vector<FieldInfo> _fields;

  int _dbzFieldNum;
  int _snrFieldNum;
  int _thresholdFieldNum;
  int _nBeamsRead;

  // storing the beams

  vector<Beam *> _beamsStored;

  // times

  time_t _startTime;
  time_t _endTime;

  // latest beam info

  time_t _latestBeamTime;
  double _latestBeamAz;
  double _latestBeamEl;

  // current scan mode
  
  scan_mode_t _scanMode;
  bool _scanInfoFromHeaders;
  bool _endOfVolAutomatic;

  // previous beam

  double _prevAzMoving;
  double _prevElevMoving;

  // beam geometry

  BeamGeomMgr *_beamGeomMgr;
  DsRadarParams _predomRadarParams;

  // radar info;

  bool _prevPrfGood;
  double _radarLat, _radarLon, _radarAlt;
  double _beamWidth;
  double _nyquist;

  // antenna object

  Antenna *_antenna;

  // PPI manager

  PpiMgr *_ppiMgr;

  // end of volume condition

  int _volNum;
  int _prevVolNum;
  bool _endOfVol;

  // functions
  
  int _run();

  int _readMsg(DsRadarQueue &radarQueue, DsRadarMsg &radarMsg, bool &gotMsg);

  void _processVol();
  void _processRhi(const DsRadarParams &predomRadarParams);
  
  void _loadRadarParams(const DsRadarMsg &radarMsg);
  void _loadRadarCalib(const DsRadarMsg &radarMsg);
  void _loadFieldParams(const DsRadarMsg &radarMsg);
  
  bool _preFilter(const DsRadarMsg &radarMsg);
  void _filterOnGeom();
  void _censorGateData();
  void _filterUsingThresholds();
  void _filterUsingDbz();
  void _filterUsingSnr();

  void _saveLatestBeamInfo(const Beam *beam);
  bool _isAntennaMoving(const DsBeamHdr_t *beam);
  int _loadCurrentScanMode();
  int _findScanModeFromHeaders();
  void _findScanModeFromAntenna();
  bool _isVertScan();

  void _printFields();

  void _prepareForVol();
  void _prepareBeamsForVol();
  void _saveBeamsOverlap();
  
  void _reset(const DsRadarMsg &radarMsg);
  void _clearAll();
  void _clearBeamsStored();
  void _clearRadarParamsVec();
  void _clearFieldFlags();
  
};

#endif

