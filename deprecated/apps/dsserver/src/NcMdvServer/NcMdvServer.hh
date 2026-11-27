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
// NcMdvServer.hh
//
// Mdv Server Object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////

#ifndef NcMdvServerINCLUDED
#define NcMdvServerINCLUDED

#include <didss/DsDataFile.hh>
#include <dsserver/DsProcessServer.hh>
#include <Mdv/MdvxField.hh>

#include <string>
using namespace std;

class NcMdvServer : public DsProcessServer {
  
public:

  static const string DataTypeString;
  
  NcMdvServer(string executableName,
	      Params *params,
	      bool allowParamsOverride);

  virtual ~NcMdvServer();
  
protected:

  virtual int handleDataCommand(Socket * socket,
                                const void * data, ssize_t dataSize);

private:

  Params *_params;
  bool _allowParamsOverride;
  string _incomingUrl;
  
  // Private methods with no bodies. DO NOT USE!
  //
  NcMdvServer();
  NcMdvServer(const NcMdvServer & orig);
  NcMdvServer & operator = (const NcMdvServer & other);

  // Handle Mdvx data commands from the client.
  int handleMdvxCommand(Socket * socket,
                        const void * data, ssize_t dataSize,
                        Params *paramsInUse);

  // load the local params
  
  int _checkForLocalParams( DsServerMsg &msg,
                            Params **paramsInUse,
                            bool &paramsAreLocal,
                            string errStr);
  
  int _loadLocalParams(const string &paramFile, Params **params_p);
  
};
#endif
