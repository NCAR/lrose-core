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
// TsOverlay.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2012
//
///////////////////////////////////////////////////////////////
//
// TsOverlay reads raw time-series data from two sets of files. It
// combines these time series by summing the I and Q data, to create
// an overlaid data set. Typically this is used for combining clutter
// and weather data together, for testing the clutter mitigation
// algorithms.
//
////////////////////////////////////////////////////////////////

#ifndef TsOverlay_H
#define TsOverlay_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <Fmq/DsFmq.hh>
#include <toolsa/MemBuf.hh>
#include <didss/DsMessage.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/RadarComplex.hh>
#include <rapformats/DsRadarCalib.hh>
#include <Radx/RadxTime.hh>

using namespace std;

////////////////////////
// This class

class TsOverlay {
  
public:

  // constructor

  TsOverlay(int argc, char **argv);

  // destructor
  
  ~TsOverlay();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;

  // pulse reading
  
  IwrfTsReader *_primaryReader;
  IwrfTsReader *_secondaryReader;
  IwrfTsPulse *_primaryPulse;
  IwrfTsPulse *_secondaryPulse;
  IwrfTsPulse *_combinedPulse;

  bool _prevPrimaryWithinLimits;
  bool _prevSecondaryWithinLimits;
  
  DsRadarCalib _primaryCalib;
  DsRadarCalib _secondaryCalib;

  int _primaryStartGate;
  int _secondaryStartGate;
  bool _alternatingMode;
  
  // metadata

  int _volNum;
  int _sweepNum;

  int _nGates;
  double _startRangeKm;
  double _gateSpacingKm;

  RadxTime _startTime;
  RadxTime _firstPulseTime;
  RadxTime _pulseTime;
  RadxTime _outputTime;
  double _deltaTime;

  double _prt;
  si64 _pulseSeqNum;
  
  double _outputAzimuthStart;
  double _outputAzimuth;
  double _deltaAz;

  // output files

  FILE *_outPrimary;
  FILE *_outSecondary;
  FILE *_outCombined;

  string _primaryOutputPath;
  string _secondaryOutputPath;
  string _combinedOutputPath;

  // debug prints

  double _prevAzPrint;

  // functions

  int _openReaders();
  int _checkAlternating(IwrfTsReader *reader);

  IwrfTsPulse *_getNextPulsePrimary();

  IwrfTsPulse *_getNextPulseSecondary(int requiredHvFlag);

  int _overlayTimeSeries();
  int _overlayPulse();

  void _setPulseMetaData();

  void _combineIq(vector<RadarComplex_t> &primaryIq,
                  vector<RadarComplex_t> &secondaryIq,
                  double primaryNoiseDbm,
                  double secondaryNoiseDbm,
                  vector<RadarComplex_t> &comboIq);

  void _computeGateGeometry();

  void _printPulseInfo();

  int _writeToOutput();
  int _writeEndOfSweep();
  int _writeEndOfVol();
  int _addEndOfVol();

  int _writeToFiles(const void *buf, int nbytes);

  int _openOutputFiles(IwrfTsPulse *pulse);

  FILE *_openOutputFile(IwrfTsPulse *pulse,
                        const string &output_dir,
                        string &outputPath);

  void _closeOutputFiles();
  
  void _closeOutputFile(FILE* &out,
                        const string &outputDir,
                        const string &outputPath);

};

#endif
