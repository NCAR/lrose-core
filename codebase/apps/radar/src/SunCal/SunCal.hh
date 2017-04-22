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
// SunCal.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#ifndef SunCal_H
#define SunCal_H

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include "MomentsSun.hh"
#include <toolsa/DateTime.hh>
#include <euclid/SunPosn.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/GateData.hh>
#include <radar/MomentsFields.hh>
#include <radar/RadarMoments.hh>
#include <radar/IwrfCalib.hh>
#include <Mdv/DsMdvx.hh>

class DsInputPath;
class RadxRay;
class RadxField;
class RadxVol;
using namespace std;

////////////////////////
// This class

class SunCal {
  
public:

  // constructor

  SunCal (int argc, char **argv);

  // destructor
  
  ~SunCal();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // private class for computing stats

  class Stats {

  public:

    double solidAngle;
    int nBeamsUsed;

    double meanDbm;
    double minDbm;

    double meanRatioDbmVcHc;
    double meanRatioDbmVxHx;
    double meanRatioDbmVcHx;
    double meanRatioDbmVxHc;

    double meanCorr00Hc;
    double meanCorr00Vc;
    double meanCorr00;

    RadarComplex_t sumRvvhh0Hc;
    RadarComplex_t sumRvvhh0Vc;
    RadarComplex_t sumRvvhh0;

    double meanS1S2;
    double sdevS1S2;
    double meanSS;
    double sdevSS;
    double SdevOfMean;

    double ratioMeanVcHcDb;
    double ratioMeanVxHxDb;
    double ratioMeanS1S2Db;
    double ratioMeanSSDb;

    Stats() {

      solidAngle = -9999;
      nBeamsUsed = -9999;
      meanDbm = -9999;
      minDbm = -9999;
      meanRatioDbmVcHc = -9999;
      meanRatioDbmVxHx = -9999;
      meanRatioDbmVcHx = -9999;
      meanRatioDbmVxHc = -9999;
      meanCorr00Hc = -9999;
      meanCorr00Vc = -9999;
      meanCorr00 = -9999;
      sumRvvhh0Hc.re = 0.0;
      sumRvvhh0Hc.im = 0.0;
      sumRvvhh0Vc.re = 0.0;
      sumRvvhh0Vc.im = 0.0;
      sumRvvhh0.re = 0.0;
      sumRvvhh0.im = 0.0;
      meanS1S2 = -9999;
      sdevS1S2 = -9999;
      meanSS = -9999;
      sdevSS = -9999;
      SdevOfMean = -9999;
      ratioMeanVcHcDb = -9999;
      ratioMeanVxHxDb = -9999;
      ratioMeanS1S2Db = -9999;
      ratioMeanSSDb = -9999;

    }

  };

  // private class for computing xpol ratio
  
  class Xpol {
  public:
    double powerHx;
    double powerVx;
    double rhoVxHx;
    Xpol(double phx, double pvx, double rvxhx) {
      powerHx = phx;
      powerVx = pvx;
      rhoVxHx = rvxhx;
    }
  };

  // private class for computing test pulse

  class TestPulse {
  public:
    double powerHc;
    double powerVc;
    double powerHx;
    double powerVx;
    TestPulse(double hc, double vc, double hx, double vx) {
      powerHc = hc;
      powerVc = vc;
      powerHx = hx;
      powerVx = vx;
    }
  };

  // enum for control of centroid computation

  typedef enum {
    channelHc, channelVc, channelMeanc
  } power_channel_t;

  //////////////
  // data members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  IwrfTsReader *_tsReader;
  DsInputPath *_covarReader;
  SunPosn _sunPosn;

  // radar location

  double _radarLat;
  double _radarLon;
  double _radarAltKm;

  // is the data in RHI mode

  bool _isRhi;

  // receiver flags

  bool _switching;
  bool _dualPol;
  bool _alternating;

  // pulse queue
  
  deque<const IwrfTsPulse *> _pulseQueue;
  int _maxPulseQueueSize;
  long _pulseSeqNum;
  int _totalPulseCount;
  double _prevAngleOffset;

  // gate data

  int _nGates;
  double _startRangeKm, _gateSpacingKm;
  int _nSamples, _nSamplesHalf;
  vector<GateData *> _gateData;
  vector<MomentsFields> _fields;
  int _startGateSun;
  int _endGateSun;

  // volume number etc

  int _volNum, _prevVolNum;

  // calibration times

  double _startTime;
  double _endTime;
  double _calTime;

  // grid

  int _gridNAz, _gridNEl;
  double _gridMinAz, _gridMinEl;
  double _gridMaxAz, _gridMaxEl;
  double _gridDeltaAz, _gridDeltaEl;

  // beam location and time details
  
  double _midTime, _midPrt;
  double _midAz, _midEl;
  double _targetEl, _targetAz;
  double _offsetAz, _offsetEl;

  // storing moments

  vector<vector<MomentsSun *> > _rawMoments;
  vector<vector<MomentsSun *> > _interpMoments;
  vector<Xpol> _xpolMoments;
  vector<TestPulse> _testPulseMoments;

  MomentsSun _prevRawMoments;
  double _prevOffsetEl;
  double _prevOffsetAz;

  // noise
  
  IwrfCalib _calib;
  double _nBeamsNoise;
  double _noiseDbmHc, _noiseDbmHx;
  double _noiseDbmVc, _noiseDbmVx;

  // max power

  double _maxValidSunPowerDbm;

  double _maxPowerDbm;
  double _quadPowerDbm;
  
  double _maxPowerDbmHc;
  double _quadPowerDbmHc;
  
  double _maxPowerDbmVc;
  double _quadPowerDbmVc;
  
  // sun centroid

  bool _validCentroid;
  double _meanSunEl, _meanSunAz;
  double _sunCentroidAzOffset, _sunCentroidElOffset;
  double _sunCentroidAzOffsetHc, _sunCentroidElOffsetHc;
  double _sunCentroidAzOffsetVc, _sunCentroidElOffsetVc;

  // beam pattern and pattern ratio H-V
  // fitting parabolas to solar pattern

  double _ccAz, _bbAz, _aaAz, _errEstAz, _rSqAz;
  double _ccEl, _bbEl, _aaEl, _errEstEl, _rSqEl;

  double _widthRatioElAzHc;
  double _widthRatioElAzVc;
  double _widthRatioElAzDiffHV;
  double _zdrDiffElAz;

  // mean correlations
  
  double _meanCorrSun;
  double _meanCorr00H;
  double _meanCorr00V;
  double _meanCorr00;
  
  // stats

  vector<Stats> _stats;

  // volume identification

  int _nBeamsThisVol;
  double _volMinEl, _volMaxEl;
  int _volCount;
  bool _endOfVol;
  bool _volInProgress;
  double _prevEl;

  // S1S2 global results
  
  double _S1S2Sdev;
  vector<double> _S1S2Results;

  double _SSSdev;
  vector<double> _SSResults;

  // xpol results

  int _nXpolPoints;
  double _meanXpolRatioDb;
  double _zdrCorr;
  Stats _statsForZdrBias;

  // test pulse results

  double _meanTestPulsePowerHcDbm;
  double _meanTestPulsePowerVcDbm;
  double _meanTestPulsePowerHxDbm;
  double _meanTestPulsePowerVxDbm;

  // transmit powers

  double _sumXmitPowerHDbm;
  double _sumXmitPowerVDbm;
  double _countXmitPowerH;
  double _countXmitPowerV;
  double _meanXmitPowerHDbm;
  double _meanXmitPowerVDbm;

  // counter for printing headers

  int _globalPrintCount;

  // xpol ratio and temperature from SPDB

  time_t _timeForXpolRatio;
  double _xpolRatioDbFromSpdb;

  time_t _timeForSiteTemp;
  double _siteTempFromSpdb;

  // methods

  void _initMembers();
  int _runForTimeSeries();
  int _runForCovar();
  int _createReaders();

  int _processPulse(const IwrfTsPulse *pulse);
  void _addPulseToQueue(const IwrfTsPulse *pulse);
  int _addPulseToNexradQueue(const IwrfTsPulse *iwrfPulse);
  void _clearPulseQueue();
  void _deletePulseQueue();

  int _processCovarFile(const char *filePath);
  int _processCovarRay(size_t rayIndex, RadxRay *ray);
  int _computeCovarMoments(RadxRay *ray, MomentsSun &mom);

  double _computeFieldMean(const RadxField *field);
  RadarComplex_t _computeRvvhh0Mean(const RadxField *rvvhh0_db,
                                    const RadxField *rvvhh0_phase,
                                    double &corrMag);

  void _initForAnalysis();
  int _performAnalysis(bool force);

  void _computeMomentsAllGates();
  int _computeMomentsSun(MomentsSun *moments, int startGate, int endGate);
  int _computeMomentsSunDualAlt(MomentsSun *moments, int startGate, int endGate);
  int _computeMomentsSunDualSim(MomentsSun *moments, int startGate, int endGate);
  int _computeMomentsSunSinglePol(MomentsSun *moments, int startGate, int endGate);
  void _addToXpol(int startGate, int nGates);
  void _addToTestPulse();
  void _addToXmitPowers(const IwrfTsPulse *pulse);
  
  void _checkForNorthCrossing(double &az0, double &az1);

  void _createRawMomentsArray();
  void _clearRawMomentsArray();
  void _deleteRawMomentsArray();

  void _createInterpMomentsArray();
  void _clearInterpMomentsArray();
  void _deleteInterpMomentsArray();

  double _computeDist(MomentsSun *moments, double el, double az);

  void _deleteXpolMomentsArray();
  void _deleteTestPulseMomentsArray();

  void _printRawMomentsArray(ostream &out);

  void _sortRawMomentsByEl();
  void _sortRawMomentsByAz();
  static bool _compareMomentsEl(MomentsSun *lhs, MomentsSun *rhs);
  static bool _compareMomentsAz(MomentsSun *lhs, MomentsSun *rhs);

  void _interpMomentsPpi();
  void _interpMomentsRhi();
  void _getNoiseFromCalFile();
  void _getNoiseFromTimeSeries();
  void _computeMeanNoise();
  void _computeNoiseFromMinPower();
  void _correctPowersForNoise();
  void _computePowerRatios();
  void _computeMaxPower();
  void _computeSunCorr();
  void _computeSunCentroid(power_channel_t channel);

  void _computeSSUsingSolidAngle();
  void _computeSS(const vector<MomentsSun> &selected,
                  double solidAngle,
                  Stats &stats);

  void _computeEllipsePowerDiffsUsingSolidAngle();

  void _computeXpolRatio(const vector<Xpol> &xpolMoments);
  void _computeTestPulse();

  void _printRunDetails(ostream &out);
  void _printOpsInfo(ostream &out);
  void _printMomentsLabels(ostream &out);

  int _quadFit(int n,
               const vector<double> &x,
               const vector<double> &y,
               double &a0,
               double &a1,
               double &a2,
               double &std_error_est,
               double &r_squared);
  
  void _checkEndOfVol(double el);

  void _allocGateData();
  void _freeGateData();

  int _loadGateData();
  int _loadGateDataDualPolAlt();
  int _loadGateDataDualPolSim();
  int _loadGateDataSinglePol();

  int _checkAlternatingStartsOnH();
  bool _isAlternating();
  bool _startsOnH();
  bool _isDualPol();
  
  int _retrieveXpolRatioFromSpdb(time_t scanTime,
                                 double &xpolRatio,
                                 time_t &timeForXpolRatio);

  int _retrieveSiteTempFromSpdb(time_t scanTime,
                                double &tempC,
                                time_t &timeForTemp);

  int _writeSummaryText();
  void _writeSummaryText(FILE *out);
  int _appendToGlobalResults();
  void _writeGlobalHeader(FILE *out);
  void _appendFloatToFile(FILE *out, double val, int width = 10);
  int _writeGriddedTextFiles();
  int _writeGriddedField(const string &dirPath,
                         const string &field, int offset);
  int _writeGriddedFieldDebug(const string &dirPath,
                              const string &field, int offset);
  int _writeToMdv();
  void _initMdvMasterHeader(DsMdvx &mdvx, time_t dataTime);
  void _addMdvField(DsMdvx &mdvx,
                    const string &fieldName,
                    const string &units,
                    const string &transform,
                    int memOffset);
  
  int _writeSummaryToSpdb();

  int _writeNexradToMdv();
  void _initNexradMdvMasterHeader(DsMdvx &mdvx, time_t dataTime);
  void _addNexradMdvField(DsMdvx &mdvx,
                          const string &fieldName,
                          const string &units,
                          const string &transform,
                          int memOffset);

  int _writeNexradSummaryToSpdb();

};

#endif
