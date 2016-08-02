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
// Procmap.h: procmap class
/////////////////////////////////////////////////////////////

#ifndef PROCMAP_H
#define PROCMAP_H

#include "Args.hh"
#include "InfoStore.hh"
#include <toolsa/pmu.h>
#include <toolsa/sockutil.h>
#include <pthread.h>
#include <string>

class threadArgs {
public:
  threadArgs(int sockFd, bool debug, InfoStore *store, time_t startTime);
  int _sockFd;
  bool _debug;
  InfoStore *_store;
  time_t _startTime;
};

class Procmap {
  
public:

  typedef struct {
    SKU_header_t header;
    union {
      PROCMAP_info_t info;
      PROCMAP_request_t req;
    } u;
  } msg_t;

  // constructor

  Procmap (int argc, char **argv);

  // destructor
  
 ~Procmap();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  InfoStore _store;
  int _port;
  int _protoFd;
  int _nThreads;
  time_t _startTime;
  time_t _lastPurge;

  static void *_handleIncoming(void *thread_args);

  static void _handleGetInfo(int sockFd,
			     bool debug,
			     InfoStore *store,
			     time_t startTime,
			     const SKU_header_t header,
			     void *msg_in);
  
  static void _handleGetInfoRelay(int sockFd,
				  bool debug,
				  InfoStore *store,
				  time_t startTime,
				  const SKU_header_t header,
				  void *msg_in);
  
  static void _handleRegister(int sockFd,
			      bool debug,
			      InfoStore *store,
			      time_t startTime,
			      const SKU_header_t &header,
			      void *msg_in);

  static void _sendReply(int sockFd, bool debug,
			 time_t startTime, int return_code);
  
};

#endif
