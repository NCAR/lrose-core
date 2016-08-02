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
// RapidDow2Dsr.hh
//
// RapidDow2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2011
//
///////////////////////////////////////////////////////////////
//
// RapidDow2Dsr reads radial moments data from the RapidDow,
// corrects the data appropriately for pointing angles and
// range, and writes the data to a DsRadarQueue beam by beam.
//
////////////////////////////////////////////////////////////////

#ifndef RapidDow2Dsr_HH
#define RapidDow2Dsr_HH

#include <string>
#include <vector>
#include "Args.hh"
#include "Params.hh"
#include <Fmq/DsRadarQueue.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxFile.hh>
using namespace std;

class DsInputPath;

////////////////////////
// This class

class RapidDow2Dsr {
  
public:

  // constructor

  RapidDow2Dsr (int argc, char **argv);

  // destructor
  
  ~RapidDow2Dsr();

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
  DsInputPath *_input;
  DsRadarQueue _rQueue;

  RadxVol _vol;
  Radx::DataType_t _dataType;
  int _dataByteWidth;

  int _channelNum;
  Params::channel_t _channel;

  time_t _fileStartTimeSecs;
  double _fileStartPartialSecs;
  double _fileDuration;

  int _outputNRays;
  int _outputRayNum;
  double _outputDeltaTime;

  bool _endOfVol;
  double _minEl, _maxEl;
  int _volNum;
  int _nBeamsInVol;
  int _sweepNum;

  int _processFile(const string filePath);
  int _processFileForChannel(const string filePath);
  int _findFileForChannel(const string filePath,
                          string &chanPath);
  void _computeTimes();
  int _processSweep(int sweepIndex);
  void _setupRead(RadxFile &file);
  void _convertFieldsToUniformType();

  int _writeParams();
  int _writeCalibration();
  int _writeRay(const RadxRay &ray);

  int _getDsRadarType(Radx::PlatformType_t ptype);
  int _getDsScanMode(Radx::SweepMode_t mode);
  int _getDsFollowMode(Radx::FollowMode_t mode);
  int _getDsPolarizationMode(Radx::PolarizationMode_t mode);
  int _getDsPrfMode(Radx::PrtMode_t mode, double prtRatio);

  double _constrainAz(double az);
    
};

#endif
