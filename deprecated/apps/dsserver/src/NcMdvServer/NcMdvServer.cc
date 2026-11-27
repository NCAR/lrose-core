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
// NcMdvServer.cc
//
// Mdv Server Object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////

#include "Params.hh"
#include "NcMdvServer.hh"

#include <toolsa/TaStr.hh>
#include <dsserver/DsLocator.hh>
#include <Mdv/NcfMdvx.hh>
#include <Mdv/DsMdvxMsg.hh>
using namespace std;

NcMdvServer::NcMdvServer(string executableName,
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

NcMdvServer::~NcMdvServer()
{
}

// Handle data commands from the client.
//   Returning failure (-1) from this method makes the server die.
//   So don't ever do that.
// 
// virtual
int NcMdvServer::handleDataCommand(Socket * socket,
                                   const void * data, ssize_t dataSize)
{

  if (_isVerbose) {
    cerr << "Handling data command in NcMdvServer." << endl;
  }
  
  // peek at the message
  
  DsServerMsg msg;
  if (msg.decodeHeader(data, dataSize)) {
    string errMsg  = "Error in NcMdvServer::handleDataCommand(): ";
    errMsg += "Could not decode header.";
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM - NcMdvServer - " << statusString << endl;
    }
    return 0;
  }

  // disassemble the message, to make URL available for params

  if (msg.disassemble(data, dataSize)) {
    string errMsg  = "Error in NcMdvServer::handleDataCommand(): ";
    errMsg += "Could not disassemble message.";
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM - NcMdvServer - " << statusString << endl;
    }
    return 0;
  }

  // check mode and subtype for validity
  
  int subType = msg.getSubType();
  
  if (subType != DsMdvxMsg::MDVP_CONVERT_MDV_TO_NCF &&
      subType != DsMdvxMsg::MDVP_CONVERT_NCF_TO_MDV &&
      subType != DsMdvxMsg::MDVP_READ_NCF &&
      subType != DsMdvxMsg::MDVP_READ_RADX &&
      subType != DsMdvxMsg::MDVP_READ_ALL_HDRS_NCF &&
      subType != DsMdvxMsg::MDVP_READ_ALL_HDRS_RADX &&
      subType != DsMdvxMsg::MDVP_WRITE_TO_DIR &&
      subType != DsMdvxMsg::MDVP_CONSTRAIN_NCF) {

    string errMsg  = "Error in NcMdvServer::handleDataCommand(): ";
    errMsg += "Unsupported subType: ";
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
      cerr << "ERROR - COMM - NcMdvServer - " << statusString << endl;
    }
    return 0;
    
  }
  
  // Check for local copy of params
  // Override initial params if params file exists in the datatype directory

  Params *paramsInUse = _params;
  bool paramsAreLocal = false;
  
  if (_isDebug) {
    umsleep(20);
    cerr << "Checking for local params." << endl;
  }
  
  string errStr = "ERROR - NcMdvServer::handleDataCommand\n";
  if (_checkForLocalParams( msg, &paramsInUse, paramsAreLocal, errStr)) {
    if (_isDebug) {
      cerr << errStr << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errStr, statusString, 10000)) {
      cerr << "ERROR - COMM - NcMdvServer - " << statusString << endl;
    }
    return 0;
    
  } // if (checkForLocalParams( ...
  
  handleMdvxCommand(socket, data, dataSize, paramsInUse);
  
  if (paramsAreLocal) {
    delete paramsInUse;
  }
  
  return 0;

}

// Handle Mdvx data requests

int NcMdvServer::handleMdvxCommand(Socket *socket,
                                   const void *data,
                                   ssize_t dataSize,
				   Params *paramsInUse)
{

  // disassemble message
  
  NcfMdvx mdvx;
  DsMdvxMsg msg;
  if (_isVerbose) {
    msg.setDebug(true);
    mdvx.setDebug(true);
  }
  
  if (msg.disassemble(data, dataSize, mdvx)) {
    string errMsg  = "Error in NcMdvServer::handleDataCommand(): ";
    errMsg += "Could not disassemble message.";
    errMsg += msg.getErrStr();
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM - NcMdvServer - " << statusString << endl;
    }
    return 0;
  }

  // get URL

  const string &url = msg.getFirstURLStr();

  if (_isDebug) {
    cerr << "  Path: " << mdvx.getPathInUse() << endl;
    cerr << "  Client host: " << msg.getClientHost() << endl;
    cerr << "  Client ipaddr: " << msg.getClientIpaddr() << endl;
    cerr << "  Client user: " << msg.getClientUser() << endl;
    mdvx.printReadRequest(cerr);
    switch (msg.getSubType()) {
      case DsMdvxMsg::MDVP_CONVERT_MDV_TO_NCF: {
        cerr << "Converting MDV to NCF" << endl;
        mdvx.printConvertMdv2NcfRequest(cerr);
        break;
      }
      case DsMdvxMsg::MDVP_CONVERT_NCF_TO_MDV: {
        cerr << "Converting NCF to MDV" << endl;
        break;
      }
      case DsMdvxMsg::MDVP_READ_NCF: {
        cerr << "Reading NCF file" << endl;
        break;
      }
      case DsMdvxMsg::MDVP_READ_RADX: {
        cerr << "Reading RADX file" << endl;
        break;
      }
      case DsMdvxMsg::MDVP_READ_ALL_HDRS_NCF: {
        cerr << "Reading all headers from NCF file" << endl;
        break;
      }
      case DsMdvxMsg::MDVP_READ_ALL_HDRS_RADX: {
        cerr << "Reading all headers from Radx file" << endl;
        break;
      }
      case DsMdvxMsg::MDVP_WRITE_TO_DIR: {
        cerr << "Writing to directory" << endl;
        if (mdvx.getWriteFormat() == Mdvx::FORMAT_NCF ||
            mdvx.getCurrentFormat() == Mdvx::FORMAT_NCF) {
          mdvx.printConvertMdv2NcfRequest(cerr);
        }
        break;
      }
      case DsMdvxMsg::MDVP_CONSTRAIN_NCF: {
        cerr << "Constraining NCF with read qualifiers" << endl;
        break;
      }
      default: {}
    }
  }

  int iret = 0;
  switch (msg.getSubType()) {
    
    case DsMdvxMsg::MDVP_CONVERT_MDV_TO_NCF: {
      iret = mdvx.convertMdv2Ncf(url);
      if (iret) {
        msg.assembleErrorReturn(msg.getSubType(), mdvx.getErrStr());
      } else {
        msg.assembleConvertMdv2NcfReturn(mdvx);
      }
      break;
    }
      
    case DsMdvxMsg::MDVP_CONVERT_NCF_TO_MDV: {
      iret = mdvx.convertNcf2Mdv(url);
      if (iret) {
        msg.assembleErrorReturn(msg.getSubType(), mdvx.getErrStr());
      } else {
        msg.assembleConvertNcf2MdvReturn(mdvx);
      }
      break;
    }
      
    case DsMdvxMsg::MDVP_READ_NCF: {
      iret = mdvx.readNcf(url);
      if (iret) {
        msg.assembleErrorReturn(msg.getSubType(), mdvx.getErrStr());
      } else {
        msg.assembleReadNcfReturn(mdvx);
      }
      break;
    }
      
    case DsMdvxMsg::MDVP_READ_RADX: {
      iret = mdvx.readRadx(url);
      if (iret) {
        msg.assembleErrorReturn(msg.getSubType(), mdvx.getErrStr());
      } else {
        msg.assembleReadRadxReturn(mdvx);
      }
      break;
    }
      
    case DsMdvxMsg::MDVP_READ_ALL_HDRS_NCF: {
      iret = mdvx.readAllHeadersNcf(url);
      if (iret) {
        msg.assembleErrorReturn(msg.getSubType(), mdvx.getErrStr());
      } else {
        msg.assembleReadAllHdrsNcfReturn(mdvx);
      }
      break;
    }
      
    case DsMdvxMsg::MDVP_READ_ALL_HDRS_RADX: {
      iret = mdvx.readAllHeadersRadx(url);
      if (iret) {
        msg.assembleErrorReturn(msg.getSubType(), mdvx.getErrStr());
      } else {
        msg.assembleReadAllHdrsRadxReturn(mdvx);
      }
      break;
    }
      
    case DsMdvxMsg::MDVP_WRITE_TO_DIR: {
      iret = mdvx.writeToDir(url);
      if (iret) {
        msg.assembleErrorReturn(msg.getSubType(), mdvx.getErrStr());
      } else {
        msg.assembleWriteReturn(DsMdvxMsg::MDVP_WRITE_TO_DIR, mdvx);
      }
      break;
    }
      
    case DsMdvxMsg::MDVP_CONSTRAIN_NCF: {
      iret = mdvx.constrainNcf(url);
      if (iret) {
        msg.assembleErrorReturn(msg.getSubType(), mdvx.getErrStr());
      } else {
        msg.assembleConstrainNcfReturn(mdvx);
      }
      break;
    }
      
    default: {
      string errMsg;
      TaStr::AddInt(errMsg, "NcMdvServer, unknown command: ", msg.getSubType());
      msg.assembleErrorReturn(msg.getSubType(), errMsg);
      break;
    }
    
  } // switch
  
  // send reply

  void *msgToSend = msg.assembledMsg();
  int msgLen = msg.lengthAssembled();
  if (socket->writeMessage(0, msgToSend, msgLen)) {
    cerr << "ERROR - COMM - NcMdvServer::HandleDataCommand." << endl;
    cerr << "  Sending message to server." << endl;
    cerr << socket->getErrStr() << endl;
  } else {
    if (_isDebug) {
      cerr << "SUCCESS - NcMdvServer sent reply to client" << endl;
    }
  }

  return 0;

}

int NcMdvServer::_checkForLocalParams(DsServerMsg &msg,
                                      Params **paramsInUse,
                                      bool &paramsAreLocal,
                                      string errStr )

{

  // NOTE: the DsLocator will resolve the parameter file name 
  //       and modify the url

  bool paramsExist;
  _incomingUrl = msg.getFirstURLStr();
  if (_incomingUrl == "") {
    errStr += "No URL in message.\n";
    return -1;
  }
  DsURL url(_incomingUrl);
  if ( DsLocator.resolveParam( url, _executableName,
			       &paramsExist ) != 0 ) {
    errStr += "Cannot resolve parameter specification in url:\n";
    errStr += url.getURLStr();
    errStr += "\n";
    return -1;
  }
  
  // The application-specfic code must load the override parameters

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
NcMdvServer::_loadLocalParams( const string &paramFile, Params **params_p)
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

