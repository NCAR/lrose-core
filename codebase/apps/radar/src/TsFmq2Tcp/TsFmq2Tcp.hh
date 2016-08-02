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
////////////////////////////////////////////////////////////////////////////////
//
// TsFmq2Tcp.hh
// TsFmq2Tcp class
//
// Mike Dixon, RAL, NCAR, Boulder, CO, USA
// Feb 2009
//
// TsFmq2Tcp listens for clients. When a client connects, it spawns a child
// to handle the client. The child opens the time-series FMQ, reads a message
// at a time from the FMQ and writes this data, unchanged,
// to the client in a continuous stream.
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef  _TS_FMQ2TCP_HH_
#define  _TS_FMQ2TCP_HH_

#include <Fmq/Fmq.hh>
#include <string>
#include <dsserver/ProcessServer.hh>
#include <didss/DsMessage.hh>
#include "Params.hh"
using namespace std;

class TsFmq2Tcp : public ProcessServer
{
public:
  
  TsFmq2Tcp(const string& progName,
	    const Params &params);
  
  virtual ~TsFmq2Tcp();
  
protected:
  
  string _progName;
  const Params &_params;
  
  // provide method missing in base class to handle client
  // Returns 0 on success, -1 on failure

  virtual int handleClient(Socket* socket);
    
private:

  Fmq _fmq;
  DsMessage _msg;
  int _msgNParts;
  int _msgPos;
  MemBuf _pktBuf;

  int _processFmq(Fmq &fmq, Socket* socket);
  const DsMsgPart *_getNextFromFmq(Fmq &fmq);

};

#endif

