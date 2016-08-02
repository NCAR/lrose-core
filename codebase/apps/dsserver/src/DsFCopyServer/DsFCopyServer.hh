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
// DsFCopyServer.hh
//
// FileServerobject
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////


#ifndef _DsFCopyServer_HH
#define _DsFCopyServer_HH

#include <dsserver/DsProcessServer.hh>
#include <dsserver/DsFileCopyMsg.hh>
#include <string>
using namespace std;

class Params;

class DsFCopyServer : public DsProcessServer {

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              
  //   
  DsFCopyServer(string executableName,
		string instanceName,                                         
		int port,
		int maxQuiescentSecs,                                  
		int maxClients,
		Params *params);

  // destructor

  virtual ~DsFCopyServer();
  
protected:

  // Handle a client's request for data. The passed
  //   message is a decoded DsResourceMsg that is
  //   *not* a server command.
  //
  // Look at the URL and determine if a port is needed.
  // Spawn the necessary server.
  // 
  virtual int handleDataCommand(Socket * socket,
				const void * data, ssize_t dataSize);
    
private:

  // Private methods with no bodies. DO NOT USE!
  // 
  DsFCopyServer();
  DsFCopyServer(const DsFCopyServer & orig);
  DsFCopyServer & operator = (const DsFCopyServer & other);

  Params *_params;
  bool _useLock;

  int _handleEnquireByTime(Socket * clientSocket,
			   DsFileCopyMsg &copyMsg,
			   bool &doPut,
			   string &putDir,
			   string &putName,
			   string &putPath,
			   string &ldataDir);
  
  int _handleEnquireForPut(Socket * clientSocket,
			   DsFileCopyMsg &copyMsg,
			   bool &doPut,
			   string &putDir,
			   string &putName,
			   string &putPath,
			   string &ldataDir,
			   time_t &fileModTime);

  int _handlePutByTime(Socket * clientSocket,
		       DsFileCopyMsg &copyMsg);
  
  int _handlePutForced(Socket * clientSocket,
		       DsFileCopyMsg &copyMsg);
  
  int _checkPutDir(DsFileCopyMsg &copyMsg,
		   bool by_time,
		   string &putDir,
		   string &ldataDir,
		   string &errorStr);

  void _setDoPut(DsFileCopyMsg &copyMsg,
		 bool &doPut, const string &_putPath);

  int _readPutMessage(Socket * clientSocket,
		      DsFileCopyMsg &copyMsg,
		      const string &putPath);
  
  int _handlePut(Socket * clientSocket,
		 DsFileCopyMsg &copyMsg,
		 const string &putName,
		 const string &putPath,
		 const string &ldataDir,
		 time_t fileModTime,
		 string &errorStr);

  int _enquireForPutReturn(Socket * clientSocket,
			   DsFileCopyMsg &copyMsg,
			   bool &doPut,
			   const bool errorOccurred = false,
			   string errorStr = "");

  int _putAfterEnquireReturn(Socket * clientSocket,
			     DsFileCopyMsg &copyMsg,
			     const bool errorOccurred = false,
			     string errorStr = "");

  int _putForcedReturn(Socket * clientSocket,
		       DsFileCopyMsg &copyMsg,
		       const bool errorOccurred = false,
		       string errorStr = "");

  int _writeLdataInfo(DsFileCopyMsg &copyMsg,
		      const string &putName,
		      const string &ldataDir,
		      time_t fileModTime);

  int _writeZeroLenFile(const string &path);
  void _removeFile(const string &path);

};

#endif
