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
///////////////////////////////////////////////////////////////
// DsTitanServer.hh
//
// Server object for DsTitanServer
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////

#ifndef __DsTitanServer_INCLUDED
#define __DsTitanServer_INCLUDED

#include <dsserver/DsProcessServer.hh>
using namespace std;

class DsTitanServer : public DsProcessServer {
  
public:

  DsTitanServer(string executableName,
		Params *params,
		bool allowParamsOverride);
  
  virtual ~DsTitanServer();
  
protected:
  virtual int handleDataCommand(Socket * socket,
				const void * data, ssize_t dataSize);
  
private:
  
  Params *_params;
  bool _allowParamsOverride;
  
  // Private methods with no bodies. DO NOT USE!
  //
  DsTitanServer();
  DsTitanServer(const DsTitanServer & orig);
  DsTitanServer & operator = (const DsTitanServer & other);
  
  // load the local params
  
  int _checkForLocalParams( DsServerMsg &msg,
			    Params **paramsInUse,
			    bool &paramsAreLocal,
			    string errStr);
  
  int _loadLocalParams(const string &paramFile, Params **params_p);
  
};

#endif
