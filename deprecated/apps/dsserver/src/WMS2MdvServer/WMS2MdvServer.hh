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
// WMS2MdvServer.hh
//
// Mdv Server Object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////

#ifndef WMS2MdvServerINCLUDED
#define WMS2MdvServerINCLUDED

#include <didss/DsDataFile.hh>
#include <dsserver/DsProcessServer.hh>
#include <string>
using namespace std;

class Grid;
class DsURL;
class MdvField;
class DsMdvMsg;
class MdvFile;
class DsMdvSocket;
class DsMdvx;

class WMS2MdvServer : public DsProcessServer {
  
public:
  static const string DataTypeString;
  
  WMS2MdvServer(string executableName,
	      Params *params,
	      bool allowParamsOverride);

    virtual ~WMS2MdvServer();

  protected:
    virtual int handleDataCommand(Socket * socket,
                                  const void * data, ssize_t dataSize);

  private:

    Params *_params;
    bool _allowParamsOverride;

    // Private methods with no bodies. DO NOT USE!
    //
    WMS2MdvServer();
    WMS2MdvServer(const WMS2MdvServer & orig);
    WMS2MdvServer & operator = (const WMS2MdvServer & other);

    // Handle Mdv data commands from the client.
    int handleMdvCommand(Socket * socket,
                         const void * data, ssize_t dataSize,
                         Params *paramsInUse);

    // Handle Mdvx data commands from the client.
    int handleMdvxCommand(Socket * socket,
                          const void * data, ssize_t dataSize,
                          Params *paramsInUse);

    // Top-level request handlers. 
    //   Each of these (or it's called methods) handles all replies to clients.
    // 
    int handleGetData(int requestType,
		      Socket * socket,
		      const DsMdvMsg & msg,
		      const DsURL & url,
		      Params *paramsInUse);

    int handlePutData(Socket * socket,
		      const DsMdvMsg & msg,
		      const DsURL & url);

    int handleGetTimes(Socket * socket,
                       const DsMdvMsg & msg,
		       const DsURL & url);

    // Message reading methods.
    int getProtoGridFromMsg(const DsMdvMsg & msg,
                            Grid ** protoGrid,
                            string & errString);

    int getProtoFieldFromMsg(const DsMdvMsg & msg,
                             MdvField & protoField,
                             string & errString);

    // Data Get request handlers -- called by handleGetData(...).
    //    Note that each of these methods takes care of replying
    //    to the client on error.
    // 

    int handleGetVolume(Socket * socket,
			const DsMdvMsg & msg,
			const DsURL & url,
			DsDataFile *diskFile,
			Params *paramsInUse);

    // load the local params

    int _checkForLocalParams( DsServerMsg &msg,
			      Params **paramsInUse,
			      bool &paramsAreLocal,
			      string errStr);

    int _loadLocalParams(const string &paramFile, Params **params_p);

	// Wrappers for reading WMS
	int _readWMServer(DsMdvx *mdvx, Params *paramsInUse);

	void _buildBBOX(DsMdvx *mdvx, Params *paramsInUse, double &x1, double &y1,
	                 double &x2, double &y2, double &delta_x, double &delta_y);

};

#endif
