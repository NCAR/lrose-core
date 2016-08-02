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
// PointingCpCompute.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2010
//
///////////////////////////////////////////////////////////////

#ifndef PointingCpCompute_H
#define PointingCpCompute_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include "GateData.hh"
#include <radar/IwrfTsReader.hh>
#include <radar/RadarComplex.hh>
#include <rapformats/DsRadarCalib.hh>

using namespace std;

////////////////////////
// This class

class PointingCpCompute {
  
public:

  // constructor

  PointingCpCompute (int argc, char **argv);

  // destructor
  
  ~PointingCpCompute();

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
  
  double _startSecsVData;
  double _endSecsVData;
  
  // input pulses
  
  IwrfTsReader *_reader;
  int _nPulsesH, _nPulsesV;
  string _inputPath;
  
  // geometry

  double _sumAzXH, _sumAzYH;
  double _sumElXH, _sumElYH;
  double _sumAzXV, _sumAzYV;
  double _sumElXV, _sumElYV;

  double _meanAzH;
  double _meanElH;
  double _meanAzV;
  double _meanElV;

  // analysis in range
  
  int _analysisStartGate, _analysisEndGate;
  int _analysisNGates;

  bool _gateSpacingWarningPrinted;
  bool _startRangeWarningPrinted;

  // calibration

  DsRadarCalib _calib;
  
  // GateData

  vector<GateData> _gatesH;
  vector<GateData> _gatesV;

  // times for H and V data

  double _startTimeH;
  double _endTimeH;
  
  double _startTimeV;
  double _endTimeV;
  
  // results

  int _nGatesUsed;

  double _cpPowerRatio;
  double _cpPowerRatioDb;

  double _cpNormRatio;
  double _cpNormRatioDb;


  
  // methods
  
  void _processPulse(const IwrfTsPulse *pulse);
  void _computeCp();

  int _writeResults();
  int _writeGlobalResults();

};

#endif
