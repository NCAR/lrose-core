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
// WMS2MdvServer.cc
//
// Mdv Server Object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////

#include "Params.hh"
#include "WMS2MdvServer.hh"

#include <dsserver/DsLocator.hh>
#include <Mdv/DsMdvxMsg.hh>
using namespace std;

WMS2MdvServer::WMS2MdvServer(string executableName,
                         Params *params,
			 bool allowParamsOverride)
  : DsProcessServer(executableName,
		    params->instance,
		    params->port,
		    params->qmax,
		    params->max_clients,
		    params->debug >= Params::DEBUG_NORM,
		    params->debug >= Params::DEBUG_VERBOSE,
		    params->run_secure, false, true),
              _params(params),
	      _allowParamsOverride(allowParamsOverride)

{
    setNoThreadDebug(_params->no_threads);

    // Set up the DsDataFile class with this server's messaging state.
    DsDataFile::setDebug(_isDebug);
    DsDataFile::setVerbose(_isVerbose);

    if (_isDebug) {
      cerr << "debug on" << endl;
    }
    if (_isVerbose) {
      cerr << "verbose on" << endl;
    }
}

WMS2MdvServer::~WMS2MdvServer()
{
}

// Handle data commands from the client.
//   Returning failure (-1) from this method makes the server die.
//   So don't ever do that.
// 
// virtual
int WMS2MdvServer::handleDataCommand(Socket * socket,
                                   const void * data, ssize_t dataSize)
{

  if (_isVerbose) {
    cerr << "Handling data command in WMS2MdvServer." << endl;
  }

  // peek at the message

  DsServerMsg msg;
  if (msg.decodeHeader(data, dataSize)) {
    string errMsg  = "Error in WMS2MdvServer::handleDataCommand(): ";
    errMsg += "Could not decode header.";
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM -WMS2MdvServer - " << statusString << endl;
    }
    return 0;
  }

  // disassemble the message - this is for the param override
  // function and can be removed if only Mdvx protocol is supported

  if (msg.disassemble(data, dataSize)) {
    string errMsg  = "Error in WMS2MdvServer::handleDataCommand(): ";
    errMsg += "Could not disassemble message.";
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM -WMS2MdvServer - " << statusString << endl;
    }
    return 0;
  }

  // check mode and subtype for validity

  int subType = msg.getSubType();

  if (subType != DsMdvxMsg::MDVP_READ_ALL_HDRS &&
      subType != DsMdvxMsg::MDVP_READ_VOLUME &&
      subType != DsMdvxMsg::MDVP_READ_VSECTION &&
      subType != DsMdvxMsg::MDVP_WRITE_TO_DIR &&
      subType != DsMdvxMsg::MDVP_WRITE_TO_PATH &&
      subType != DsMdvxMsg::MDVP_COMPILE_TIME_LIST) {

    string errMsg  = "Error in WMS2MdvServer::handleDataCommand(): ";
    errMsg += "Unknown subType: ";
    char str[32];
    sprintf(str, "%d\n", msg.getSubType());
    errMsg += str;
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
	cerr << "ERROR - COMM -WMS2MdvServer - " << statusString << endl;
    }
    return 0;

  }

  //
  // Check for local copy of params
  // Override initial params if params file exists in the datatype directory
  //
  Params *paramsInUse = _params;
  bool paramsAreLocal = false;
  
  if (_allowParamsOverride) {

    if (_isDebug) {
      cerr << "Checking for local params." << endl;
    }

    string errStr = "ERROR - WMS2MdvServer::handleDataCommand\n";
    if (_checkForLocalParams( msg, &paramsInUse, paramsAreLocal, errStr)) {
      if (_isDebug) {
	cerr << errStr << endl;
      }
      // Send error reply to client.
      string statusString;
      if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				     errStr, statusString, 10000)) {
	cerr << "ERROR - COMM -WMS2MdvServer - " << statusString << endl;
      }
      return 0;

    } // if (checkForLocalParams( ...

  } // if (_allowParamsOverride) 


  handleMdvxCommand(socket, data, dataSize, paramsInUse);

  if (paramsAreLocal) {
    delete paramsInUse;
  }
  
  return 0;

}

int
WMS2MdvServer::_checkForLocalParams( DsServerMsg &msg,
				   Params **paramsInUse,
				   bool &paramsAreLocal,
				   string errStr )

{

  //
  // NOTE: the DsLocator will resolve the parameter file name 
  //       and modify the url
  //

  bool paramsExist;
  string urlStr = msg.getFirstURLStr();
  if (urlStr == "") {
    errStr += "No URL in message.\n";
    return -1;
  }
  DsURL url(urlStr);
  if ( DsLocator.resolveParam( url, _executableName,
			       &paramsExist ) != 0 ) {
    errStr += "Cannot resolve parameter specification in url:\n";
    errStr += url.getURLStr();
    errStr += "\n";
    return -1;
  }
  
  //
  // The application-specfic code must load the override parameters
  //
  if ( paramsExist ) {
    if ( _loadLocalParams( url.getParamFile(), paramsInUse ) != 0 ) {
      errStr += "Cannot load parameter file:\n";
      errStr += url.getParamFile();
      errStr += "\n";
      return( -1 );
    }
    paramsAreLocal = true;
  } // if ( paramsExist )

  return 0;

}
  
int
WMS2MdvServer::_loadLocalParams( const string &paramFile, Params **params_p)
{

  char   **tdrpOverrideList = NULL;
  bool     expandEnvVars = true;
  
  if (_isDebug) {
    cerr << "Loading new params from file: " << paramFile << endl;
  }
  
  Params *localParams = new Params(*_params);
  if ( localParams->load( (char*)paramFile.c_str(),
			  tdrpOverrideList,
			  expandEnvVars,
			  false ) != 0 ) {
    delete localParams;
    return -1;
  }

  if (_isVerbose) {
    localParams->print(stderr, PRINT_SHORT);
  }
  
  *params_p = localParams;
  return 0;
}
