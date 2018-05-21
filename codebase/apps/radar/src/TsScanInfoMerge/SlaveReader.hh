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
// SlaveReader
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////

#ifndef SlaveReader_hh
#define SlaveReader_hh

#include <string>
#include <toolsa/pmu.h>
#include <Fmq/DsFmq.hh>
#include <didss/DsMessage.hh>
#include <didss/DsMsgPart.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include "Params.hh"
using namespace std;

////////////////////////
// Base class

class SlaveReader {
  
public:
  
  // constructor
  
  SlaveReader(const Params &params);
  
  // destructor
  
  ~SlaveReader();

  // get next message part
  // returns NULL at end of data, or error
  
  const DsMsgPart *getNextPart();

  // get methods
  
  IwrfTsInfo &getOpsInfo() { return _opsInfo; }
  
  // get latest pulse packet

  inline const void *getLatestPulsePacket() const { 
    return _latestPulsePacket; 
  }

  inline int getLatestPulsePacketLen() const {
    return _latestPulsePacketLen; 
  }
  
  inline const iwrf_pulse_header_t &getLatestPulseHdr() const {
    return _latestPulseHdr;
  }

protected:
private:
  
  const Params &_params;

  // input queue

  DsFmq _fmq;
  bool _fmqIsOpen;

  // input message
  
  DsMessage _msg;
  DsMsgPart *_part;
  int _nParts;
  int _pos;
  IwrfTsInfo _opsInfo;

  // save latest pulse packet
  
  char *_latestPulsePacket;
  int _latestPulsePacketLen;
  iwrf_pulse_header_t _latestPulseHdr;
  
  int _getNextPart();

};

#endif

