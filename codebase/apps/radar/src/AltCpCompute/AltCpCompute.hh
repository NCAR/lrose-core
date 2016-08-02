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
// AltCpCompute.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#ifndef AltCpCompute_H
#define AltCpCompute_H

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <radar/IwrfTsReader.hh>
#include <radar/RadarComplex.hh>
#include <radar/GateData.hh>
#include <rapformats/DsRadarCalib.hh>

class RadxFile;
class RadxRay;
class RadxField;
class RadxTime;
class RadxVol;

using namespace std;

////////////////////////
// This class

class AltCpCompute {
  
public:

  // constructor

  AltCpCompute (int argc, char **argv);

  // destructor
  
  ~AltCpCompute();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  IwrfTsReader *_reader;
  double _minValidRatioDb, _maxValidRatioDb;

  // switching receiver?

  bool _switching;

  // pulse queue
  
  deque<const IwrfTsPulse *> _pulseQueue;
  int _maxPulseQueueSize;
  long _pulseSeqNum;
  int _totalPulseCount;
  int _nSamples;
  int _nSamplesHalf;
  int _nGates;

  // analysis times

  time_t _latestPulseTime;
  double _calTime;
  double _startTime;
  double _pulseTime;
  double _midPrt;
  double _midEl;
  double _midAz;
  double _prevAz;
  double _azMoved;
  int _nBeams;

  // calibration

  DsRadarCalib _calib;
  double _noiseHc;
  double _noiseHx;
  double _noiseVc;
  double _noiseVx;
  
  // gate data

  vector<GateData *> _gateData;

  // summing up for Vx/Hx ratio

  int _writeCount;
  int _pairCount;

  int _nPairsClutter;
  double _sumRatioHxVxDbClutter;
  double _meanRatioHxVxDbClutter;

  int _nPairsWeather;
  double _sumRatioHxVxDbWeather;
  double _meanRatioHxVxDbWeather;

  // output files
  
  FILE *_ratioFile;

  // status values

  vector<double> _statusVals;
  vector<string> _statusLabels;
  double _siteTempC;

  double _calXmitPowerDbmH;
  double _calXmitPowerDbmV;

  double _measTxPwrRatioHV;

  time_t _timeForSiteTemp;
  double _meanZdrmVert;
  double _sunscanS1S2;
  double _sunscanQuadPowerDbm;
  double _sunscanZdrm;
  
  // methods
  
  int _timeSeriesRun();

  int _processPulse(const IwrfTsPulse *pulse);
  void _addPulseToQueue(const IwrfTsPulse *pulse);
  void _clearPulseQueue();
  int _processBeam();
  bool _checkNGates();
  bool _checkDual();
  void _allocGateData();
  void _freeGateData();
  void _loadGateData();

  int _cfradialRun();
  int _cfradialRunFilelist();
  int _cfradialRunArchive();
  int _cfradialRunRealtimeWithLdata();
  int _cfradialRunRealtimeNoLdata();
  int _cfradialProcessFile(const string &readPath);
  void _cfradialProcessRay(double receiverGainDbHx,
                           double receiverGainDbVx,
                           RadxRay *ray, 
                           RadxField *dbmhxField,
                           RadxField *dbmvxField,
                           RadxField *snrhxField,
                           RadxField *snrvxField,
                           RadxField *cpaField,
                           RadxField *cmdField,
                           RadxField *rhoVxHxField);
  void _cfradialSetupRead(RadxFile &file);
  
  void _addGateData(RadxTime rayTime,
                    double elevationDeg,
                    double azimuthDeg,
                    int gateNum,
                    double rangeKm,
                    double cpa,
                    double cmd,
                    double snrhx,
                    double snrvx,
                    double rhovxhx,
                    double dbmhx,
                    double dbmvx);
  
  void _computeRatio();
  void _clearRatio();
  void _writeRatioHeader();
  int _openRatioFile();
  int _writeRatioToFile(const RadxTime &rtime);
  void _doWriteRatio(const RadxTime &rtime);
  void _getXmitPowers(const RadxTime &rtime);
  
  int _writeRunningSummary();
  void _writePairHeader();
  void _writePairData(RadxTime &rayTime,
                      double el,
                      double az,
                      int gateNum,
                      double rangeKm,
                      double snrHx,
                      double snrVx,
                      double dbmHx,
                      double dbmVx,
                      double cpa,
                      double cmd,
                      double rhoVxHx,
                      double ratioHxVxDb);

  int _writeRatioToSpdb(const RadxTime &rtime);

  void _retrieveValsFromXmlStatus(const RadxVol &vol);

  int _retrieveSiteTempFromSpdb(const RadxVol &vol,
                                double &tempC,
                                time_t &timeForTemp);

  int _retrieveVertPointingFromSpdb(const RadxVol &vol,
                                    double &meanZdrm);
    
  int _retrieveSunscanFromSpdb(const RadxVol &vol,
                               double &S1S2,
                               double &quadPowerDbm);

};

#endif
