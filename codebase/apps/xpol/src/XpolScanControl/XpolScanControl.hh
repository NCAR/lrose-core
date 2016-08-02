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
// XpolScanControl.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2012
//
///////////////////////////////////////////////////////////////
// XpolScanControl controls the XPOL antenna. It creates PAXI
// files and uploads them to the DRX. From one FMQ it reads
// the data coming from the DRX, to monitor the antenna behavior.
// It then adds the scan information as appropriate, and writes
// the modified data to an output FMQ.
////////////////////////////////////////////////////////////////

#ifndef XpolScanControl_H
#define XpolScanControl_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <rapformats/DsRadarMsg.hh>
#include <rapformats/DsRadarParams.hh>
#include <Fmq/DsRadarQueue.hh>
#include <euclid/SunPosn.hh>
class Field;

using namespace std;

// typedefs for map of active children

typedef pair<pid_t, time_t> activePair_t;
typedef map <pid_t, time_t, less<pid_t> > activeMap_t;

////////////////////////
// This class

class XpolScanControl {
  
public:

  // constructor

  XpolScanControl(int argc, char **argv);

  // destructor
  
  ~XpolScanControl();

  // run 

  int Run();

  // data members

  bool isOK;
  
  // stop scanning

  void stopActivity();

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // lock file to ensure only a single instance is running

  FILE *_lockFile;

  // input queues

  DsRadarQueue _inputQueue;
  DsRadarMsg _inputMsg;
  int _inputContents;
 
  // output queue

  DsRadarQueue _outputQueue;
  DsRadarMsg _outputMsg;
  MemBuf _outBuf;

  // children from running scripts

  activeMap_t _active;

  // paxi file

  FILE *_paxiFile;

  // digital receiver proc num

  int _drxConfId;

  // current scan and beam information

  int _volNum;
  int _sweepNum;
  int _scanMode;
  int _beamTimeSecs;
  bool _transition;
  double _beamTimeDsecs;
  double _prevBeamTimeDsecs;
  double _deltaTimeDsecs;
  double _az;
  double _el;
  double _prevAz;
  double _prevEl;
  double _azDist;
  double _elDist;
  double _azRate;
  double _elRate;
  double _prevAzRate;
  double _prevElRate;
  double _fixedEl;
  double _fixedAz;

  // sun scanning

  SunPosn _sunPosn;

  // functions
  
  int _checkParams();
  int _createLockFile();
  int _run();
  int _init();
  int _getNextBeam(const string &modeStr);

  int _setDrxProc(int drxConfId);

  int _stopAntenna(double nsecsWait,
                   bool dataActive);

  int _runPointing(double azPoint,
                   double elPoint,
                   double azVel,
                   double elVel,
                   double nsecsWait,
                   bool dataActive);

  int _runSurveillance(double startAz,
                       double startEl,
                       double endEl,
                       double deltaEl,
                       double azVel,
                       double elVel,
                       double transitionTolerance);

  int _runSector(int mode,
                 const string &modeStr,
                 double startAz,
                 double endAz,
                 double startEl,
                 double endEl,
                 double deltaEl,
                 double azVel,
                 double elVel,
                 double transitionTolerance);

  int _runRhi(double startAz,
              double endAz,
              double deltaAz,
              double startEl,
              double endEl,
              double azVel,
              double elVel,
              double transitionTolerance);

  int _runVertPoint(double startAz,
                    double startEl,
                    double azVel,
                    double elVel,
                    int nSweeps,
                    double transitionTolerance);

  int _runSunscanRaster(double startAz,
                        double endAz,
                        double startEl,
                        double endEl,
                        double deltaEl,
                        double azVel,
                        double elVel,
                        double transitionTolerance);

  int _runSunscanConstEl(double startAz,
                         double endAz,
                         double startEl,
                         double endEl,
                         double deltaEl,
                         double azVel,
                         double elVel);
  
  int _runSunscanConstAz(double startAz,
                         double endAz,
                         double startEl,
                         double endEl,
                         double deltaAz,
                         double azVel,
                         double elVel);

  int _openOutputFmq();

  int _writeBeam();
  int _writeParams();
  int _writeCalib(const DsRadarCalib &calib);
  int _writeFlags(const DsRadarFlags &flags);

  void _writeStartOfVolume();
  void _writeEndOfVolume();
  void _writeStartOfSweep();
  void _writeEndOfSweep();

  int _openPaxiFile();
  void _closePaxiFile();

  void _uploadPaxiFile();
  void _execScript();
  void _killAsRequired(pid_t pid, time_t terminate_time);
  void _reapChildren();
  void _killRemainingChildren();

  void _printSummary(const DsRadarMsg &msg);

  double _getCorrectedAz(double rawAz);
  double _getRawAz(double correctedAz);
  double _getNormAz(double az);
  double _getAzDiff(double az1, double az2);
  double _getAbsAzDiff(double az1, double az2);
  bool _isStopped();

};


#endif
