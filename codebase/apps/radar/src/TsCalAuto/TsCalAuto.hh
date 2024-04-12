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
// TsCalAuto.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2009
//
///////////////////////////////////////////////////////////////

#ifndef TsCalAuto_H
#define TsCalAuto_H

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/DateTime.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>

using namespace std;

////////////////////////
// This class

class TsCalAuto {
  
public:

  // constructor

  TsCalAuto (int argc, char **argv);

  // destructor
  
  ~TsCalAuto();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double piCubed;
  static const double lightSpeed;
  static const double kSquared;

  //////////////
  // data members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  IwrfTsReader *_pulseReader;
  bool _fmqMode;
  bool _haveChan1;

  int _nSamples;
  int _nGatesRequested, _startGateRequested;
  int _nGates, _startGate, _endGate;

  double _siggenStartPower;
  time_t _calTime;
  vector<double> _siggenDbm, _waveguideDbmH, _waveguideDbmV;
  vector<double> _hcDbm, _hxDbm, _vcDbm, _vxDbm;
  vector<double> _hcMinusVcDbm, _hxMinusVxDbm;
  vector<double> _hcDbmNs, _hxDbmNs, _vcDbmNs, _vxDbmNs;
  double _radarConstH, _radarConstV;
  double _wavelengthCm, _wavelengthM;

  double _prevFreq;
  
  int _testPulsePid;
  
  // results

  class ChannelResult {
  public:
    double noiseDbm;
    double gainDbm;
    double i0Dbm;
    double slope;
    double corr;
    double stdErrEst;
    double radarConstant;
    double dbz0;
    double satDbm;
    double dynamicRangeDb;
    ChannelResult() {
      noiseDbm = -9999;
      gainDbm = -9999;
      i0Dbm = -9999;
      slope = -9999;
      corr = -9999;
      stdErrEst = -9999;
      radarConstant = -9999;
      dbz0 = -9999;
      satDbm = -9999;
      dynamicRangeDb = -9999;
    }
  };

  ChannelResult _resultHc;
  ChannelResult _resultHx;
  ChannelResult _resultVc;
  ChannelResult _resultVx;

  // methods

  void _clearArrays();
  int _runFmqMode();
  int _runFileMode();
  int _processFile(const string &filePath);
  int _processFiles(const string &hPath, const string &vPath);
  int _readFile(const string &filePath);
  int _readFiles(const string &hPath, const string &vPath);

  void _doCal();
  void _computeCal(const string &label,
                   const vector<double> &inputDbm,
                   const vector<double> &outputDbm,
                   vector<double> &outputDbmNs,
                   ChannelResult &result,
                   double radarConst);

  int _writeResults();

  int _linearFit(const vector<double> &x,
                 const vector<double> &y,
                 double &gain,
                 double &slope,
                 double &xmean,
                 double &ymean,
                 double &xsdev,
                 double &ysdev,
                 double &corr,
                 double &stdErrEst,
                 double &rSquared);

  double _computeRadarConstant(double xmitPowerDbm,
                               double antennaGainDb,
                               double twoWayWaveguideLossDb,
                               double twoWayRadomeLossDb);

  void _setSiggenPower(double powerDbm);
  void _setSiggenFreq(double freqGhz);
  void _sendSiggenCmd(const char* cmd, char* recv_buf, int buf);
  void _setSiggenRF(bool on);

  int _sampleReceivedPowers(double powerDbm);
  IwrfTsPulse *_getNextPulse();
  void _openPulseReader(); 
  void _closePulseReader();
  void _conditionGateRange(const IwrfTsPulse &pulse);

  void _suspendTestPulse();
  void _resumeTestPulse();

};

#endif
