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
// RvDealias.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2002
//
///////////////////////////////////////////////////////////////

#ifndef RvDealias_hh
#define RvDealias_hh

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <netcdf.hh>
#include "Args.hh"
#include "Params.hh"
#include "Complex.hh"

class Moments;
class Verify;

using namespace std;

////////////////////////
// This class

class RvDealias {
  
public:

  // constructor

  RvDealias (int argc, char **argv);

  // destructor
  
  ~RvDealias();

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
  Moments *_moments;
  Verify *_verify;

  int _nSamples;
  int _nTimes, _nGates, _nBeams;
  int _nTimeGates, _nBeamGates, _firstGate;
  float *_I, *_Q, *_Azimuth, *_Elevation;
  int *_Prt, *_SampleNum;
  float *_dbm1, *_power1, *_vel1, *_width1;
  float *_dbm2, *_power2, *_vel2, *_width2;
  double *_Time;

  static const int _phaseCodeN = 8;
  static const int _phaseCodeM = 64;
  Complex_t _phaseCode[_phaseCodeM];

  void _initPhaseCodes();
  void _initDeconMatrix();

  int _processFile(const char *input_path);
  void _processBeam(int beam_num);
  int _loadFromFile(NcFile &ncf);
  void _freeArrays();

  void _printFile(NcFile &ncf);
  void _printAtt(NcAtt *att);
  void _printVarVals(NcVar *var);
  void _printData();

};

#endif

