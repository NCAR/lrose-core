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
// Dsr2Rapic.hh
//
// Dsr2Rapic object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////
//
// Dsr2Rapic reads an input radar FMQ, puts the data into a grid, and
// saves it out as an MDV file.
//
///////////////////////////////////////////////////////////////////////

#ifndef Dsr2Rapic_H
#define Dsr2Rapic_H

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "Args.hh"
#include "Params.hh"
#include "FieldInfo.hh"
#include "Beam.hh"
#include "BeamGeom.hh"
#include "PlanTransform.hh"
#include "Antenna.hh"
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>

#include "rdrscan.h"

using namespace std;

typedef struct {
  double elev;
  int nbeams;
} tilt_peak_t;

enum e_scanmode {  
  SCAN_UNKNOWN_MODE = -1,
  SCAN_CALIBRATION_MODE = 0,
  SCAN_SECTOR_MODE = 1,
  SCAN_COPLANE_MODE = 2,
  SCAN_RHI_MODE = 3,
  SCAN_VERTICAL_POINTING_MODE = 4,
  SCAN_TARGET_MODE = 5,
  SCAN_MANUAL_MODE = 6,
  SCAN_IDLE_MODE = 7,
  SCAN_SURVEILLANCE_MODE = 8,
  SCAN_VERTICAL_SWEEP_MODE = 9 };

extern char *scan_mode_strings[];

////////////////////////
// This class

class Dsr2Rapic {
  
public:

  // constructor

  Dsr2Rapic (int argc, char **argv);

  // destructor
  
  ~Dsr2Rapic();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // members

  static const double _smallRange;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  
  vector<DsRadarParams *> _radarParamsVec;
  vector<FieldInfo> _fields;
  int _filterFieldNum;
  int _nBeamsRead;
  vector<Beam *> _beamsStored;
  vector<Beam *> _beamsInterp;
  vector<BeamGeom *> _beamGeomVec;

  // times

  time_t _startTime;
  time_t _endTime;

  // latest beam info

  time_t _latestBeamTime;
  double _latestBeamAz;
  double _latestBeamEl;
  
  // elevation histogram

  int _nElevHist;
  int _elevHistOffset;
  double _elevHistIntv;
  int _elevHistSearchWidth;
  int *_elevHist;
  int _nElev;
  vector<double> _elevTable; // elev table in use
  float _azAngleRes,
    _elAngleRes;
  bool _azAngleIncreasing,
    _elAngleIncreasing;
  float _sectorAz1,
    _sectorAz2;
  float _rhiEl1,
    _rhiEl2;

  // grid params

  double _deltaAz;
  int _nazPer45, _naz;
  int _nxy, _nxyHalf;

  double _predomStartRange;
  double _predomGateSpacing;
  int _predomMaxNGates;
  DsRadarParams _predomRadarParams;

  // radar info;

  bool _prevPrfGood;
  bool _radarInfoSet;
  double _radarLat, _radarLon, _radarAlt;
  double _beamWidth;
  double _nyquist;
  int _radarId;
  int _countryCode;
  string _radarName;

  // antenna object

  Antenna *_antenna;

  // end of volume flag

  bool _endOfVol;
  int  _tiltNum;
  bool _antTransition;
  double _antTransStartTime;

  int _startTiltBeamNum;
  int _lastTiltStartBeam,
    _lastTiltEndBeam;

  int _lastVolTiltCount;
  int _lastScanType;

  // rapic rdrscan instance
  rdr_scan *_rapicScan;
  time_t _lastRapicScanUpdateTime; // last time data was added to _rapicScan
  time_t _rapicScanTimeout;        // if _rapicScan not updating, end it
  // functions
  
  int _run();

  int _readMsg(DsRadarQueue &radarQueue,
	       DsRadarMsg &radarMsg);

  void _processTilt(int tiltnum);
  void _processEndOfVol();
  
  void _processVol();
  int _processRhi();
  
  void _loadRadarParams(const DsRadarMsg &radarMsg);
  void _loadFieldParams(const DsRadarMsg &radarMsg);
  
  bool _loadElevTable(int startbeam = 0, 
		      int endbeam = -1,
		      bool rhimode = false);
  bool _loadElevTableFromHist(int startbeam = 0, 
			      int endbeam = -1,
			      bool rhimode = false);
  int _computeBeamGeom(int startbeam = 0, 
		       int endbeam = -1,
		       bool rhimode = false);
  void _rapicEncodeField(int fieldnum,
			 rdr_scan *fieldscan,
			 int startbeam = 0, 
			 int endbeam = -1);
  void _setPartIDStrings(LevelTable *lvltbl);
  bool _preFilter(const DsRadarMsg &radarMsg);
  void _filterBeams(int startbeam = 0, 
		    int endbeam = -1);
  void _filterNoise(int startbeam = 0, 
		    int endbeam = -1);

  void _saveBeamGeom(const Beam *beam);
  bool _loadPredominantGeom();
  bool _geomMatches(double startRangeNew, double startRangePrev,
		    double gateSpacingNew, double gateSpacingPrev,
		    int nBeamsNew = 0, int nBeamsPrev = 0);

  void _bridgeMissingInAzimuth();
  void _bridgeMissingInElevation();
  Beam *_interpBeam(const Beam *beam1, const Beam *beam2,
		      double weight1);
  
  void _printElevsAndFields();

  void _prepareForVol();
  void _prepareBeamsForVol();
  void _saveBeamsOverlap();
  
  void _reset(const DsRadarMsg &radarMsg);
  void _clearAll();
  void _clearBeams();
  void _clearBeamsStored();
  void _clearBeamsInterp();
  void _clearRadarParamsVec();
  void _clearBeamGeomVec();
  void _clearElevHist();
  void _clearPpiArray();
  void _clearFieldFlags();
  
};

#endif

