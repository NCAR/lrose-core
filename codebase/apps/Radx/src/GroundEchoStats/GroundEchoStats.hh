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
// GroundEchoStats object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2012
//
///////////////////////////////////////////////////////////////
//
// GroundEchoStats prints out radar moments in a variety of ways
//
///////////////////////////////////////////////////////////////////////

#ifndef GroundEchoStats_HH
#define GroundEchoStats_HH

#include <string>
#include <Radx/Radx.hh>
#include <toolsa/ServerSocket.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

class IwrfMomReader;
class RadxRay;

////////////////////////
// This class

class GroundEchoStats {
  
public:

  // constructor

  GroundEchoStats (int argc, char **argv);

  // destructor
  
  ~GroundEchoStats();

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
  IwrfMomReader *_reader;
  
  // server mode

  ServerSocket _server;
  Socket *_sock;

  // statistics

  double _sumDbz;
  double _sumVrPlatform;
  double _sumVrMeasured;
  double _sumSqVrMeasured;
  double _sumVrCorrected;
  double _sumRange;
  int _count;
  double _globalStartTime;
  double _statsStartTime, _statsEndTime;

  vector<double> _vrCorrected;
  vector<double> _tiltError;

  double _globalSumVrCorrected;
  double _globalSumSqVrCorrected;
  double _globalSumTiltError;
  double _globalSumSqTiltError;
  int _globalCount;

  // methods

  int _run();
  
  void _debugPrint(const RadxRay *ray);
  void _initStats();
  void _computeStats(const RadxRay *ray);
  double _computeSdev(double sum,
                      double sumSq,
                      int count);


};

#endif

