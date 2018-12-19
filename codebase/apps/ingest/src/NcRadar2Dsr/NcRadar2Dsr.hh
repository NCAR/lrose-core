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
// NcRadar2Dsr.hh
//
// NcRadar2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////

#ifndef NcRadar2Dsr_hh
#define NcRadar2Dsr_hh

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <Fmq/DsRadarQueue.hh>
#include <Ncxx/Nc3File.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class NcRadar2Dsr {
  
public:

  // constructor

  NcRadar2Dsr (int argc, char **argv);

  // destructor
  
  ~NcRadar2Dsr();

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

  int _prevVolNum;
  int _volNum;
  int _tiltNum;
  time_t _startTime, _endTime;
  
  int _processFile(const char *input_path);
  int _checkFile(Nc3File &ncf);
  
  void _printFile(Nc3File &ncf);
  void _printAtt(Nc3Att *att);
  void _printVarVals(Nc3Var *var);

};

#endif

