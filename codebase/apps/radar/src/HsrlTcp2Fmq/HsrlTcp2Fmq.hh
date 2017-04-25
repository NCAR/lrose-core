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
// HsrlTcp2Fmq.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////
//
// HsrlTcp2Fmq reads raw HSRL fields from the instrument server via
// TCP/IP. It saves the data out to a file message queue (FMQ), which
// can be read by multiple clients.
//
////////////////////////////////////////////////////////////////

#ifndef HsrlTcp2Fmq_HH
#define HsrlTcp2Fmq_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/Socket.hh>
#include <Fmq/DsFmq.hh>
#include <toolsa/MemBuf.hh>

using namespace std;

////////////////////////
// This class

class HsrlTcp2Fmq {
  
public:

  // constructor

  HsrlTcp2Fmq(int argc, char **argv);

  // destructor
  
  ~HsrlTcp2Fmq();

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

  // heartbeat function for reading

  Socket::heartbeat_t _heartBeatFunc;

  // tcp input socket

  Socket _sock;
  bool _sockTimedOut;
  int _timedOutCount;
  bool _unknownMsgId;
  int _unknownCount;

  // input message

  MemBuf _inputBuf;
  si64 _packetId;
  si64 _packetLen;

  // output FMQ

  DsFmq _outputFmq;
  
  // reading data

  int _readFromServer();
  int _readMessage();
  int _readTcpPacket();

  // write to FMQ

  int _openOutputFmq();
  int _writeToOutputFmq();

};

#endif
