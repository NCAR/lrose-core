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
// Radx2Fmq.hh
//
// Radx2Fmq object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2025
//
///////////////////////////////////////////////////////////////
//
// Radx2Fmq reads radial moments data from Radx-supported files
// and writes the data to a DsRadarQueue beam by beam in Radx format.
//
////////////////////////////////////////////////////////////////

#ifndef Radx2Fmq_HH
#define Radx2Fmq_HH

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

class Radx2Fmq {
  
public:

  // constructor

  Radx2Fmq (int argc, char **argv);

  // destructor
  
  ~Radx2Fmq();

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

  Radx::DataType_t _dataType;
  int _dataByteWidth;
  vector<string> _readPaths;
  
  int _prevNGates;
  int _prevSweepNum;
  vector<string> _prevFieldNames;

  int _runSimulate();
  int _processFile(const string filePath);
  int _processSweep(const RadxVol &vol, int sweepIndex);
  void _setupRead(RadxFile &file);
  void _convertFieldsToUniformType(RadxVol &vol);

  int _writeParams(const RadxVol &vol, const RadxSweep &sweep, const RadxRay &ray);
  int _writeStatusXml(const RadxVol &vol);
  int _writeCalibration(const RadxVol &vol);
  int _writeBeam(const RadxVol &vol,
                 const RadxSweep &sweep,
                 int rayNumInSweep, const RadxRay &ray);

  int _getDsRadarType(Radx::PlatformType_t ptype);
  int _getDsScanMode(Radx::SweepMode_t mode);
  int _getDsFollowMode(Radx::FollowMode_t mode);
  int _getDsPolarizationMode(Radx::PolarizationMode_t mode);
  int _getDsPrfMode(Radx::PrtMode_t mode, double prtRatio);
    
};

#endif
