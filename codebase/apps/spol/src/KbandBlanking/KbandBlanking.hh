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
///////////////////////////////////////////////////////////////////////////
// KbandBlanking.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2011
//
///////////////////////////////////////////////////////////////////////////
//
// KbandBlanking reads angles from the SPOL angle FMQ, and determines
// whether the azimith falls into a blanking region or not. It then
// issues blanking/not blanking commands to the kadrx. kadrx will then
// disable the transmit triggers while blanked.
//
///////////////////////////////////////////////////////////////////////////

#ifndef KbandBlanking_HH
#define KbandBlanking_HH

#include <string>
#include <vector>
#include <cstdio>
#include <xmlrpcpp/XmlRpc.h>

#include "Args.hh"
#include "Params.hh"
#include <didss/DsMessage.hh>
#include <Fmq/DsFmq.hh>
#include <radar/spol_angles.hh>

using namespace std;

////////////////////////
// This class

class KbandBlanking {
  
public:

  // constructor

  KbandBlanking(int argc, char **argv);

  // destructor
  
  ~KbandBlanking();

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

  // blanking lookup table

  double _azRes; // azimuth resolution
  int _nLut;
  int *_blankedLut;
  double *_lowerElLimit;
  double *_upperElLimit;

  // blanking condition

  double _prevRpcTime;
  bool _blanked;

  // angle input message and FMQ

  DsFmq _angleFmq;
  DsMessage _angleMsg;
  DsMsgPart *_anglePart;
  int _anglePos, _angleNParts;
  spol_short_angle_t _latestAngle;
  spol_short_angle_t _prevAngle;

  // RPC client

  XmlRpc::XmlRpcClient *_rpcClient;

  // reading in the angle data

  int _runOps();
  int _runSim();
  int _openAngleFmq();
  int _getNextAngle();
  void _computeBlankingLut();
  double _getCurrentTime();

  int _setBlankingOn(double time, double el, double az, bool changingState);
  int _setBlankingOff(double time, double el, double az, bool changingState);

};

#endif
