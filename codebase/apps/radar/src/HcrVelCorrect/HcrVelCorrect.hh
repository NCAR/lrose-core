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
// HcrVelCorrect.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2015
//
//////////////////////////////////////////////////////////////////////////
//
// HcrVelCorrect reads in HCR moments, computes the apparent velocity
// of the ground echo, filters the apparent velocity in time to remove
// spurious spikes, and then corrects the weather echo velocity using
// the filtered ground velocity as the correction to be applied.
//
//////////////////////////////////////////////////////////////////////////

#ifndef HcrVelCorrect_HH
#define HcrVelCorrect_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <deque>
#include <cmath>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <radar/HcrSurfaceVel.hh>
#include <radar/HcrVelFirFilt.hh>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
class RadxFile;
class RadxRay;
using namespace std;

class HcrVelCorrect {
  
public:

  // constructor
  
  HcrVelCorrect (int argc, char **argv);

  // destructor
  
  ~HcrVelCorrect();

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

  // storing incoming rays long enough to 
  // write out filtered results

  RadxTime _timeFirstRay;
  HcrSurfaceVel _surfVel;
  HcrVelFirFilt _firFilt;

  // input volume
  
  RadxVol _inVol;
  RadxTime _inEndTime;
  bool _firstInputFile;

  // wave filtering

  class FiltNode {
  public:
    RadxRay *ray;
    double velSurf;
    double dbzSurf;
    double rangeToSurf;
    double velNoiseFiltMean;
    double velNoiseFiltMedian;
    double velWaveFiltMean;
    double velWaveFiltMedian;
    double velWaveFiltPoly;
    bool written;
    FiltNode() {
      ray = NULL;
      velSurf = NAN;
      dbzSurf = NAN;
      rangeToSurf = NAN;
      velNoiseFiltMean = NAN;
      velNoiseFiltMedian = NAN;
      velWaveFiltMean = NAN;
      velWaveFiltMedian = NAN;
      velWaveFiltPoly = NAN;
      written = false;
    }
    RadxTime getTime() { return ray->getRadxTime(); }
  };

  deque<FiltNode> _filtNodes;
  RadxTime _filtStartTime;
  RadxTime _filtMidTime;
  RadxTime _filtEndTime;
  RadxTime _nodesEndTime;
  RadxTime _filtRetrieveTime;
  
  bool _velIsValid;
  double _velFilt;
  RadxRay *_filtRay;
  
  double _noiseFiltSecs;
  double _waveFiltSecs;
  double _filtSecs;

  // filtered volume - output

  RadxVol _filtVol;

  // methods

  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  int _runFmq();

  int _processFile(const string &filePath);
  int _processRay(RadxRay *ray);

  void _initWaveFilt();
  int _applyWaveFilt(RadxRay *ray, 
                     double velSurf,
                     double dbzSurf,
                     double rangeToSurf);

  void _updateNodeStats();
  void _addRayToFiltVol(RadxRay *ray);

  void _setupRead(RadxFile &file);

  void _convertFieldsForOutput(RadxVol &vol);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  int _writeFiltVol();
  
  RadxRay *_readFmqRay();
  void _loadRadarParams();
  RadxRay *_createInputRay();
  int _writeParams(const RadxRay *ray);
  int _writeRay(const RadxRay *ray);

  Radx::SweepMode_t _getRadxSweepMode(int dsrScanMode);
  Radx::PolarizationMode_t _getRadxPolarizationMode(int dsrPolMode);
  Radx::FollowMode_t _getRadxFollowMode(int dsrMode);
  Radx::PrtMode_t _getRadxPrtMode(int dsrMode);

  int _getDsScanMode(Radx::SweepMode_t mode);

  void _correctVelForRay(RadxRay *ray, double surfFilt);
  void _copyVelForRay(RadxRay *ray);

  void _writeResultsToSpdb(const RadxRay *filtRay);


};

#endif
