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
////////////////////////////////////////////////////////////////////
// PmuInfo.hh
//
// Provides access to PMU info in a thread-safe manner.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// April 2000
//
////////////////////////////////////////////////////////////////////

#ifndef PmuInfo_HH
#define PmuInfo_HH


#include <sys/types.h>
#include <string>
#include <toolsa/pmu.h>
#include <toolsa/MemBuf.hh>
using namespace std;

class PmuInfo {

public:

  // default constructor

  PmuInfo();
  
  // destructor
  
  ~PmuInfo();

  ////////////////////////////////////////////////////////////////
  // Read the info, from procmap on hostname.
  //
  // All processes which match procname, up to its length, and
  // instance, up to its length, are returned.
  // So to get all processes, use empty strings.
  // To get a single process, use full name and instance.
  //
  // Returns 0 on success, -1 on error
  // Get error via getErrStr().
  
  int read(string hostname = "localhost",
	   string procname = "",
	   string instance = "");
  
  ////////////////////////////////
  // get proc info, after the read

  // number of matching processes found  

  int getNProcs() const { return (_nProcs); } 

  // array of process info structs

  const PROCMAP_info_t *getInfoArray() const;

  // uptime of procmap

  time_t getUpTime() const { return (_upTime); }

  // reply time from procmap host

  time_t getReplyTime() const { return (_replyTime); }
  
  // error string
  
  const string &getErrStr() { return (_errStr); }
  
protected:

  string _errStr;
  int _procmapPort;
  int _nProcs;
  MemBuf _infoBuf;
  time_t _upTime;
  time_t _replyTime;

  int _read(const string &hostname,
	    const string &procname,
	    const string &instance);
  
  int _readRelay(const string &hostlist,
		 const string &procname,
		 const string &instance);
  
private:
  
};
  
#endif 
