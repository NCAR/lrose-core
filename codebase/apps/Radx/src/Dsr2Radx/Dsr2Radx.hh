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
///////////////////////////////////////////////////////////////
// Dsr2Radx.hh
//
// Dsr2Radx object
// Updated processing
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2018
//
///////////////////////////////////////////////////////////////
//
// Dsr2Radx reads an input radar FMQ, and saves out data files
// in formats supported by Radx.
//
///////////////////////////////////////////////////////////////

#ifndef Dsr2Radx_HH
#define Dsr2Radx_HH

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "Args.hh"
#include "Antenna.hh"
#include "Params.hh"
#include "SweepMgr.hh"
#include <Radx/RadxVol.hh>
#include <Radx/RadxFile.hh>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
class IwrfMomReader;
class RadxRay;

using namespace std;

////////////////////////
// This class

class Dsr2Radx {
  
public:

  // constructor

  Dsr2Radx (int argc, char **argv);

  // destructor
  
  ~Dsr2Radx();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // members

  static const double _smallVal;
  static const double _pseudoDiam;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // reading the incoming FMQ

  IwrfMomReader *_reader;
 
  int _nRaysRead;
  int _nCheckPrint;
  int _nWarnCensorPrint;

  // calibrations

  vector<DsRadarCalib *> _calibs;

  // sweep manager

  SweepMgr *_sweepMgr;
  bool _sweepNumbersMissing;
  Radx::SweepMode_t _sweepMode;
  
  // current scan mode
  
  typedef enum {
    SCAN_MODE_UNKNOWN,
    SCAN_MODE_SURVEILLANCE,
    SCAN_MODE_SECTOR,
    SCAN_MODE_PPI,
    SCAN_MODE_RHI,
    SCAN_MODE_VERT,
    SCAN_MODE_SUNSCAN,
    SCAN_MODE_SUNSCAN_RHI
  } scan_mode_t;
  scan_mode_t _scanMode;
  
  bool _scanInfoFromHeaders;
  bool _endOfVolAutomatic;

  // antenna object

  Antenna *_antenna;

  // previous angles for determining antenna movement

  double _prevAzMoving;
  double _prevElevMoving;

  // radial volume data set

  RadxVol _vol;
  RadxFile *_outFile;
  
  // end of volume condition

  RadxRay *_prevRay;
  RadxRay *_cachedRay;
  time_t _endOfVolTime;
  int _prevVolNum;
  int _prevTiltNum;
  int _prevSweepNum;
  Radx::SweepMode_t _prevSweepMode;
  int _sweepNumOverride;
  bool _sweepNumDecreasing;
  bool _endOfVol;

  // is this a solar scan?

  bool _isSolarScan;

  // lookup table for max ht

  double _lutRadarAltitudeKm;
  double _maxRangeLut[90]; // for each degree elevation

  // functions
  
  int _run();

  int _processRay(RadxRay *ray);
  int _processEndOfVol();
  int _processVol();

  void _setupWrite();
  int _doWrite();

  int _writeLdataInfo(const string &outputDir,
                      const string &outputPath,
                      time_t dataTime,
                      const string &dataType);

  void _updatePlatform();
  void _updateRcalib();

  double _computeDeltaAngle(double a1, double a2);

  void _censorInputRay(RadxRay *ray);

  void _prepareRayForOutput(RadxRay *ray);
  bool _isCensoringField(const string &name);
  bool _isOutputField(const string &name);
  void _applyCensoring();

  void _addRayToVol(RadxRay *ray);
  bool _acceptRay(const RadxRay *ray);
  bool _isAntennaMoving(const RadxRay *ray);
  int _loadCurrentScanMode();
  int _findScanModeFromHeaders();
  void _findScanModeFromAntenna();
  bool _isVertScan();
  
  void _clearData();

  Radx::SweepMode_t _getRadxSweepMode(int dsrScanMode);
  Radx::PolarizationMode_t _getRadxPolarizationMode(int dsrPolMode);
  Radx::FollowMode_t _getRadxFollowMode(int dsrMode);
  Radx::PrtMode_t _getRadxPrtMode(int dsrMode);

  int _computeNgatesForMaxHt(double elev,
                             double radarAltitudeKm,
                             double startRangeKm,
                             double gateSpacingKm);

  void _computeMaxRangeLut(double radarAltitudeKm);
  
  void _computeEndOfVolTime(time_t rayTime);

  bool _checkEndOfVol360(RadxRay *ray);

};

#endif

