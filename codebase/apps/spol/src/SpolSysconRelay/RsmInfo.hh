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
// RsmInfo.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////
//
// RsmInfo reads RSM - status packets, from a SysCon server
// and loads up a shared memory segment with selected parts
// of that data
//
////////////////////////////////////////////////////////////////

#ifndef RsmInfo_HH
#define RsmInfo_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Params.hh"
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/iwrf_data.h>
#include <radar/iwrf_user_interface.h>
#include <radar/iwrf_functions.hh>
#include <radar/rsm_functions.hh>
#include <radar/syscon_to_spol.h>

using namespace std;

////////////////////////
// This class

class RsmInfo {
  
public:

  // constructor

  RsmInfo(const string &progName,
          const Params &params);

  // destructor
  
  ~RsmInfo();

  // check for data
  // waiting specified time
  
  int checkForData(int waitMsecs);

  // read data from server
  // returns 0 on success, -1 on failure
  // also sets id of iwrf data read
  
  int read();
  
protected:
  
private:

  string _progName;
  const Params &_params;

  // socket

  Socket _sock;
  
  // sequence number of structure

  int _seqNum;
  
  // functions

  int _openSocket(const char *host, int port);
  void _closeSocket();

  int _peekAtBuffer(void *buf, int nbytes);
  int _readPacket(int &id, int &len, MemBuf &buf);

  void _handleScanSegment(iwrf_scan_segment_t &scan);
  void _handleTsProcessing(iwrf_ts_processing_t &proc);
  void _handleRsmPacket(const rsm_pkt_t &rsm_pkt);
  void _handleRsmInsPm(const rsm_ins_pm_t &pm);
  void _handleRsmSyscon(const rsm_syscon_t &syscon);
  void _handleRsmAntcon(const rsm_antcon_t &antcon);
  
};

#endif
