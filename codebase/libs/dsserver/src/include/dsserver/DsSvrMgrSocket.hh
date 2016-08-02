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

//////////////////////////////////////////////////////////
// DsSvrMgrSocket.hh
//
// Paddy McCarthy, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// Jan 1999
//
////////////////////////////////////////////////////////////


#ifndef DSSVRMGRSOCKET_HH
#define DSSVRMGRSOCKET_HH

#include <toolsa/ThreadSocket.hh>
#include <didss/DsURL.hh>
using namespace std;

class DsSvrMgrSocket : public ThreadSocket {

public:

  // Well known executable name for ServerMgr

  static const char* EXEC_NAME;

  DsSvrMgrSocket();
  virtual ~DsSvrMgrSocket();

  // findPortForURL() opens the connection, makes
  // the enquiry and closes the connection

  int findPortForURL(const char *hostname,
		     DsURL & url, int wait_msecs,
		     string & errString);

protected:

  int _doFind(DsURL & url, int wait_msecs, string & errString);

};

#endif
