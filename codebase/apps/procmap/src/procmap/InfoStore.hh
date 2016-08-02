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
// InfoStore.h: Information store object
/////////////////////////////////////////////////////////////

#ifndef INFOSTORE_H
#define INFOSTORE_H

#include <toolsa/globals.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MemBuf.hh>
#include <string>
#include <map>
#include <pthread.h>

class InfoStore {
  
public:

  InfoStore ();
  ~InfoStore ();
  void setDebug(bool debug = true) { _debug = debug; }
  void GetInfo(const PROCMAP_request_t &req, MemBuf &info_buf);
  void RegisterProc(const PROCMAP_info_t &info);
  void UnRegisterProc(const PROCMAP_info_t &info);
  void Purge(time_t now);
  void print(ostream &out);
  void print(ostream &out, const PROCMAP_info_t &info);
  void printPretty(ostream &out, const PROCMAP_info_t &info);
  
private:

  bool _debug;
  pthread_mutex_t _mutex;
  
  typedef struct {
    bool valid;
    PROCMAP_info_t info;
  } info_entry_t;

  typedef pair<string, PROCMAP_info_t > InfoPair;
  typedef map<string, PROCMAP_info_t, less<string> > InfoMap;
  typedef InfoMap::iterator InfoIter;

  InfoMap _procs;

  bool _matchName(const char *proc_name, const char *want_name);
  bool _matchInstance(const char *proc_instance, const char *want_instance);
  
};

#endif
