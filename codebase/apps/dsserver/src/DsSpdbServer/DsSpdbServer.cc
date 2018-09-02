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
///////////////////////////////////////////////////////////////////////////////
//
//  Server class for DsSpdbServer application
//
///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <cassert>
#include <sys/stat.h>
#include <sys/wait.h>
#include <toolsa/Socket.hh>
#include <toolsa/GetHost.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/TaStr.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DestUrlArray.hh>
#include <dsserver/DsLdataInfo.hh>
#include <dsserver/DsClient.hh>
#include <dsserver/DsSvrMgrSocket.hh>

#include "Driver.hh"
#include "DsSpdbServer.hh"
#include "PutArgs.hh"
#include "Params.hh"
using namespace std;

DsSpdbServer::DsSpdbServer(const string& executableName,
                           const Params *initialParams) :
        DsProcessServer(executableName,
                        initialParams->instance,
                        initialParams->port,
                        initialParams->qmax,
                        initialParams->max_clients,
                        initialParams->debug >= Params::DEBUG_NORM,
                        initialParams->debug >= Params::DEBUG_VERBOSE,
                        initialParams->run_secure,
                        initialParams->run_read_only,
                        initialParams->allow_http),
        _initialParams(initialParams)
{
  
  assert(initialParams != NULL);
  _params = NULL;
  setNoThreadDebug(initialParams->no_threads);
  _nPutChildren = 0;
  
}

DsSpdbServer::~DsSpdbServer()

{
  if (_params != NULL) {
    delete _params;
  }
}

/////////////////////////////////////////////////////////
// Override handleDataCommand from DsProcessServer class.
// Always return true, so that parent will not exit

int DsSpdbServer::handleDataCommand(Socket * socket,
                                    const void * data,
                                    ssize_t dataSize)

{

  if (_isDebug) {
    cerr << "-->> DsSpdbServer::handleDataCommand - entry" << endl;
    cerr << "  " << DateTime::str() << endl;
  }

  if (_isVerbose) {
    cerr << "Client thread disassembling message..." << endl;
  }

  // disassemble the incoming request, and delay uncompression
  // if the data buffer is compressed

  bool delayUncompression = true;
  DsSpdbMsg inMsg;
  if (inMsg.disassemble((void *) data, dataSize, delayUncompression)) {
    cerr << "ERROR - COMM - DsSpdbServer::handleDataCommand" << endl;
    cerr << "Invalid DsSpdbMsg message" << endl;
    return -1;
  }
  
  if (_isDebug) {
    cerr << "-------- RECEIVED MESSAGE ----------" << endl;
    inMsg.print(cerr);
    cerr << "------------------------------------" << endl;
  }
  
  // verify the url and determine the spdb directory path

  string url_str(inMsg.getUrlStr());
  DsURL url(url_str);
  if (!url.isValid()) {
    cerr << "ERROR - COMM - DsSpdbServer::handleDataCommand" << endl;
    cerr << "   Invalid URL: '" << url_str <<  "'\n";
    return 0;
  }

  // Check for local copy of params
  // Override initial params if params file exists in the datatype directory

  _params = new Params(*_initialParams);

  // NOTE: the DsLocator will resolve the parameter file name 
  // and modify the url
  
  bool  paramsExist;
  if (DsLocator.resolveParam(url, _executableName, &paramsExist)) {
    cerr << "ERROR - COMM - DsSpdbServer::handleDataCommand\n"
         << "Cannot resolve parameter specification in url:\n"
         << url.getURLStr()
         << endl;
    return 0;
  }
  
  // The application-specfic code must load the override parameters
  
  if (paramsExist) {
    if (_loadLocalParams(url.getParamFile(), _params)) {
      cerr << "ERROR - DsSpdbServer::handleDataCommand\n"
           << "  Cannot load parameter file:\n"
           << url.getParamFile()
           << endl;
    }
  }
  
  // process the request

  int iret = 0;
  if (inMsg.getSubType() == DsSpdbMsg::DS_SPDB_GET) {

    if (_params->failover_on_read) {
      
      if (_handleGetWithFailover(inMsg, *socket)) {
        iret = -1;
      }

    } else if (_params->use_search_criteria) {

      if (_handleGetWithSearch(inMsg, *socket)) {
        iret = -1;
      }

    } else {
      
      DsSpdbMsg replyMsg;
      DsSpdbMsg::info_t getInfo;
      if (_handleGetSingle(inMsg, *socket, getInfo, true, replyMsg)) {
        iret = -1;
      }

    }

  } else if (inMsg.getSubType() == DsSpdbMsg::DS_SPDB_PUT) {
    
    if (_handlePut(inMsg, *socket)) {
      iret = -1;
    }

  } else {
    
    cerr << "ERROR - DsSpdbServer::handleDataCommand\n"
         << "Unexpected message" << endl;
    iret = -1;

  }
  
  if (_isDebug) {
    cerr << "-->> DsSpdbServer::handleDataCommand - exit, iret: " << iret << endl;
  }
  
  return iret;

}
    
// Override base class on timeout and post handlers
// always return true - i.e. never exit
bool DsSpdbServer::timeoutMethod()
{
  DsProcessServer::timeoutMethod();
  return true; // Continue to wait for clients.
}
bool DsSpdbServer::postHandlerMethod()
{
  DsProcessServer::postHandlerMethod();
  return true; // Continue to wait for clients.
}

//////////////////////////////////////////////////////////////////////
// load params from local file

int DsSpdbServer::_loadLocalParams(const string &paramFile, Params *params)
  
{

  if (_isDebug) {
    cerr << "Loading new params from file: " << paramFile << endl;
  }

  char **tdrpOverrideList = NULL;
  bool expandEnvVars = true;
  if (params->load(paramFile.c_str(), tdrpOverrideList,
                   expandEnvVars, _isVerbose)) {
    cerr << "ERROR - DsSpdbServer::_loadLocalParams" << endl
         << "  Cannot load parameter file: " << paramFile << endl;
    return -1;
  }
  
  if (_isVerbose) {
    params->print(stderr, PRINT_SHORT);
  }
  
  return 0;

}

//////////////////////////////
// handle get from single URL

int DsSpdbServer::_handleGetSingle(const DsSpdbMsg &inMsg, 
                                   Socket &socket,
                                   DsSpdbMsg::info_t &getInfo,
                                   bool sendReply,
                                   DsSpdbMsg &replyMsg)

{

  if (_isDebug) {
    cerr << "-->> DsSpdbServer::_handleGetSingle - entry" << endl;
  }

  replyMsg.clearData();
  DsSpdb spdb;
  if (spdb.doMsgGet(inMsg, getInfo)) {
    
    string errStr = "ERROR - DsSpdbServer::_handleGetSingle()\n";
    errStr += spdb.getErrStr();
    TaStr::AddStr(errStr, "  URL: ", inMsg.getUrlStr());
    replyMsg.assembleGetErrorReturn(inMsg.getSpdbMode(), errStr.c_str());
    if (_isDebug) {
      cerr << errStr << endl;
    }

  } else {
    
    if (_isDebug) {
      cerr << "DsSpdbServer.handleGetSingle: getProdId: "
           << spdb.getProdId() << "  getProdLabel: "
           << spdb.getProdLabel() << endl;
    }
    
    if (inMsg.getMode() == DsSpdbMsg::DS_SPDB_GET_MODE_TIMES) {

      replyMsg.assembleGetTimesSuccessReturn(inMsg.getSpdbMode(), getInfo);
      
    } else if (inMsg.getMode() == DsSpdbMsg::DS_SPDB_GET_MODE_TIME_LIST) {

      replyMsg.assembleCompileTimeListSuccessReturn
        (inMsg.getSpdbMode(),
         getInfo, spdb.getTimeList());
      
    } else {
      
      MemBuf refBufOut;
      MemBuf auxBufOut;
      MemBuf dataBufOut;
      int nChunksOut;
      
      if (!getInfo.get_refs_only) {

	_selectGetData(spdb.getNChunks(),
                       spdb.getChunkRefs(), spdb.getAuxRefs(),
                       spdb.getChunkData(),
                       nChunksOut, refBufOut, auxBufOut, dataBufOut);

      } else {
        
	nChunksOut = spdb.getNChunks();
	refBufOut.free();
	refBufOut.add(spdb.getChunkRefs(),
		      spdb.getNChunks() * sizeof(Spdb::chunk_ref_t));
	auxBufOut.free();
	auxBufOut.add(spdb.getAuxRefs(),
		      spdb.getNChunks() * sizeof(Spdb::aux_ref_t));

      }

      getInfo.n_chunks = nChunksOut;
      replyMsg.assembleGetDataSuccessReturn
        (inMsg.getSpdbMode(),
         getInfo, refBufOut, auxBufOut,
         dataBufOut, inMsg.getDataBufCompression());

    }
    
  }
    
  // send reply

  if (_isDebug) {
    cerr << "-------- REPLY MESSAGE ----------" << endl;
    replyMsg.print(cerr);
    cerr << "---------------------------------" << endl;
  }

  void *replyBuf = replyMsg.assembledMsg();
  int replyLen = replyMsg.lengthAssembled();
  
  if (sendReply) {
    if (socket.writeMessage(DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
                            replyBuf, replyLen, 1000)) {
      cerr << "ERROR - DsSpdbServer::_handleGet" << endl;
      cerr << "  Cannot write reply" << endl;
      return -1;
    }
  }

  if (_isDebug) {
    cerr << "-->> DsSpdbServer.handleGetSingle: exit" << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////
// Handle failover on read
  
int DsSpdbServer::_handleGetWithFailover(const DsSpdbMsg &inMsg,
                                         Socket &socket)
  
{

  if (_isDebug) {
    cerr << "-->> DsSpdbServer.handleGetWithFailover: entry" << endl;
  }
  
  int iret = 0;
  
  bool found = false;
  DsSpdbMsg replyMsg;
  
  for(int ii = 0; !found && ii < _params->failover_entries_n; ii++) {
    
    // modify the message according to the params

    string url = _params->_failover_entries[ii].url;
    DsSpdbMsg::info_t info = inMsg.getInfo();
    int time_margin = _params->_failover_entries[ii].search_margin_secs;
    if (time_margin >= 0) {
      info.time_margin = time_margin;
    }
    
    DsSpdbMsg getMsg(inMsg);
    getMsg.setUrlStr(url);
    getMsg.setInfo(info);
    
    DsSpdbMsg::info_t getInfo = inMsg.getInfo();
    iret = _handleGetSingle(getMsg, socket, getInfo,
                            false, replyMsg);
    
    if (iret == 0 && getInfo.n_chunks > 0) {
      found = true;
      cerr << "Found data for url: " << url << endl;
    }
    
  }

  // Now send the reply
  
  if (_isDebug) {
    cerr << "-------- REPLY MESSAGE ----------" << endl;
    replyMsg.print(cerr);
    cerr << "---------------------------------" << endl;
  }

  void *replyBuf = replyMsg.assembledMsg();
  int replyLen = replyMsg.lengthAssembled();
  
  if (socket.writeMessage(DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
                          replyBuf, replyLen, 1000)) {
    cerr << "ERROR - DsSpdbServer::_handleGetWithFailover" << endl;
    cerr << "  Cannot write reply" << endl;
    return( -1 );
  }
  
  if (_isDebug) {
    cerr << "-->> DsSpdbServer.handleGetWithFailover: exit" << endl;
  }
  
  return iret;
  
}

/////////////////////////////////////////////////////////////////////
// Handle specialized search criteria
  
int DsSpdbServer::_handleGetWithSearch(const DsSpdbMsg &inMsg,
                                       Socket &socket)
  
{

  if (_isDebug) {
    cerr << "-->> DsSpdbServer.handleGetWithSearch: entry" << endl;
  }
  
  int iret = 0;
  
  bool found = false;
  DsSpdbMsg replyMsg;

  for(int i = 0; !found && i < _params->search_criteria_n; i++) {
    
    // modify the message according to the params

    string url = inMsg.getUrlStr();
    url += PATH_DELIM;
    url += _params->_search_criteria[i].subdir;

    DsSpdbMsg::info_t info = inMsg.getInfo();
    info.time_margin = _params->_search_criteria[i].time_margin_min * 60;

    DsSpdbMsg getMsg(inMsg);
    getMsg.setUrlStr(url);
    getMsg.setInfo(info);

    DsSpdbMsg::info_t getInfo = inMsg.getInfo();
    iret = _handleGetSingle(getMsg, socket, getInfo,
                            false, replyMsg);

    if (iret == 0 && getInfo.n_chunks > 0) {
      found = true;
      cerr << "Found data for url: " << url << endl;
    }
    
  }

  // Now send the reply
  
  if (_isDebug) {
    cerr << "-------- REPLY MESSAGE ----------" << endl;
    replyMsg.print(cerr);
    cerr << "---------------------------------" << endl;
  }

  void *replyBuf = replyMsg.assembledMsg();
  int replyLen = replyMsg.lengthAssembled();
  
  if (socket.writeMessage(DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
                          replyBuf, replyLen, 1000)) {
    cerr << "ERROR - DsSpdbServer::_handleGetWithSearch" << endl;
    cerr << "  Cannot write reply" << endl;
    return( -1 );
  }
  
  if (_isDebug) {
    cerr << "-->> DsSpdbServer.handleGetWithSearch: exit" << endl;
  }
  
  return iret;
  
}

///////////////////////////////////////////////////////////
// perform selections on the get data

void DsSpdbServer::_selectGetData(const int n_chunks_in,
                                  const Spdb::chunk_ref_t *chunk_refs_in,
                                  const Spdb::aux_ref_t *aux_refs_in,
                                  const void *chunk_data_in,
                                  int &n_chunks_out,
                                  MemBuf &refBufOut,
                                  MemBuf &auxBufOut,
                                  MemBuf &dataBufOut)
  
{
  int i, dataType;
  int selectIndex = 0;
  
  // Allow for selection of multiple chunks based on data type
  
  switch ( _params->data_type_select ) {
    
    case Params::ALL:
      // default spdb behavior
      break;
      
    case Params::FIRST:
      // only use the first chunk
      selectIndex = 0;
      break;
      
    case Params::LAST:
      // only use the last chunk
      selectIndex = n_chunks_in - 1;
      break;
      
    case Params::MIN:
      // only use the chunk with the smallest data_type value
      selectIndex = 0;
      dataType = chunk_refs_in[0].data_type;
      for( i=1; i < n_chunks_in; i++ ) {
        if ( chunk_refs_in[i].data_type < dataType ) {
          selectIndex = i;
          dataType = chunk_refs_in[i].data_type;
        }
      }
      break;
      
    case Params::MAX:
      // only use the chunk with the largest data_type value
      selectIndex = 0;
      dataType = chunk_refs_in[0].data_type;
      for( i=1; i < n_chunks_in; i++ ) {
        if ( chunk_refs_in[i].data_type > dataType ) {
          selectIndex = i;
          dataType = chunk_refs_in[i].data_type;
        }
      }
      break;
  }
  
  if ( n_chunks_in > 1  &&  _params->data_type_select != Params::ALL ) {
    
    if (_isVerbose) {
      cerr << "DsSpdbServer::transformData() - searching for chunk." << endl;
      cerr << "  n_chunks_in: " << n_chunks_in << endl;
      cerr << "  selectIndex: " << selectIndex << endl;
    }
    
    // We're only going to serve one of the input chunks
    
    char   *dataPos;
    size_t  dataLen;
    
    // Calculate the position of the selected chunk
    
    dataPos = (char*)chunk_data_in;
    for( i=0; i < selectIndex; i++ ) {
      dataPos += chunk_refs_in[i].len;
    }
    
    // Copy the single selected input chunk to the output chunk
    
    Spdb::chunk_ref_t outRef = chunk_refs_in[selectIndex];
    n_chunks_out = 1;
    dataLen = outRef.len;
    outRef.offset = 0;
    refBufOut.free();
    refBufOut.add(&outRef, sizeof(Spdb::chunk_ref_t));
    Spdb::aux_ref_t outAux = aux_refs_in[selectIndex];
    auxBufOut.add(&outAux, sizeof(Spdb::aux_ref_t));
    dataBufOut.free();
    dataBufOut.add(dataPos, dataLen);
    
  } else {
    
    // Default behavior - copy the data unchanged
    
    n_chunks_out = n_chunks_in;
    
    refBufOut.free();
    refBufOut.add(chunk_refs_in, n_chunks_in * sizeof(Spdb::chunk_ref_t));
    
    auxBufOut.free();
    auxBufOut.add(aux_refs_in, n_chunks_in * sizeof(Spdb::aux_ref_t));
    
    int dataLen = 0;
    for (int i = 0; i < n_chunks_in; i++) {
      dataLen += chunk_refs_in[i].len;
    }
    dataBufOut.free();
    dataBufOut.add(chunk_data_in, dataLen);
    
  } // if ( n_chunks_in ...
  
}

////////////////
// _handlePut()

int DsSpdbServer::_handlePut(const DsSpdbMsg &inMsg, 
                             Socket &socket)
  
{

  if (_isDebug) {
    cerr << "-->> DsSpdbServer::_handlePut - entry" << endl;
  }

  // check read-only
  
  if (_isReadOnly) {
    cerr << "ERROR - DsSpdbServer::_handlePut" << endl;
    cerr << "  Cannot put data - running in read-only mode." << endl;
    cerr << "  URL: " << inMsg.getUrlStr() << endl;
    return -1;
  }
   
  // check security
  
  if (_isSecure) {
    string securityErr;
    if (!DsServerMsg::urlIsSecure(inMsg.getUrlStr(), securityErr)) {
      cerr << "ERROR - DsSpdbServer::_handlePut" << endl;
      cerr << "  Running in secure mode." << endl;
      cerr << securityErr;
      cerr << "  URL: " << inMsg.getUrlStr() << endl;
      return -1;
    }
  }
   
  int iret = 0;
    
  if (_params->forward) {

    // reply immediately - do not wait for result of forwarding

    DsSpdbMsg replyMsg;
    replyMsg.assemblePutReturn(inMsg.getSpdbMode());

    if (_isDebug) {
      cerr << "------ FORWARDING - EARLY REPLY TO CLIENT ----------" << endl;
      replyMsg.print(cerr);
      cerr << "----------------------------------------------------" << endl;
    }

    void *replyBuf = replyMsg.assembledMsg();
    int replyLen = replyMsg.lengthAssembled();
    
    if (socket.writeMessage(DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
			    replyBuf, replyLen, 1000)) {
      cerr << "ERROR - DsSpdbServer::_handlePut" << endl;
      cerr << "  Forwarding active" << endl;
      cerr << "  Cannot write reply" << endl;
      iret = -1;
    }
    
    // forward the data

    string errStr;
    if (_putForward(inMsg, errStr)) {
      cerr << "ERROR - DsSpdbServer::_handlePut" << endl;
      cerr << "  Forwarding active" << endl;
      cerr << "  URL: " << inMsg.getUrlStr() << endl;
      cerr << errStr << endl;
      iret = -1;
    }

  } else {

    // local put
    
    DsSpdbMsg replyMsg;
    DsSpdb spdb;
    if (spdb.doMsgPut(inMsg)) {
      string errStr = "ERROR - DsSpdbServer::_handlePut\n";
      TaStr::AddStr(errStr, "  URL: ", inMsg.getUrlStr());
      errStr += spdb.getErrStr();
      replyMsg.assemblePutReturn(inMsg.getSpdbMode(), true, errStr.c_str());
    } else {
      replyMsg.assemblePutReturn(inMsg.getSpdbMode());
    }
    
    // send reply
    
    if (_isDebug) {
      cerr << "------ FORWARDING - LOCAL PUT REPLY -------" << endl;
      replyMsg.print(cerr);
      cerr << "-------------------------------------------" << endl;
    }

    void *replyBuf = replyMsg.assembledMsg();
    int replyLen = replyMsg.lengthAssembled();
    
    if (socket.writeMessage(DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
                            replyBuf, replyLen, 1000)) {
      cerr << "ERROR - DsSpdbServer::_handlePut" << endl;
      cerr << "  Cannot write reply" << endl;
      iret = -1;
    }
    
  } // if (_params->forward)
  
  if (_isDebug) {
    cerr << "-->> DsSpdbServer::_handlePut - exit" << endl;
  }

  return iret;

}

////////////////////////////////////////////////
// function for forwarding data to multiple URLs

int DsSpdbServer::_putForward(const DsSpdbMsg &inMsg, 
                              string &errStr)
  
{

  if (_isDebug) {
    cerr << "-> Doing data forwarding." << endl;
    inMsg.print(cerr);
  }

  // create destination URL list
  
  DestUrlArray urlArray("DsSpdbServer", false);
  if (_isDebug) {
    urlArray.setDebugOn();
  }
  
  if (_params->use_dest_host_list_file) {
    if (urlArray.load(_params->dest_host_list_file_path,
		      _params->dest_url_template)) {
      errStr += "  DsSpdbServer::_forward.\n";
      errStr += urlArray.getErrStr();
      return -1;
    }
  } else {
    for (int i = 0; i < _params->dest_url_list_n; i++) {
      if (urlArray.add(_params->_dest_url_list[i])) {
	errStr += "  DsSpdbServer::_forward.\n";
	errStr += urlArray.getErrStr();
	return -1;
      }
    } // i
  }

  if (_isDebug >= Params::DEBUG_VERBOSE) {
    urlArray.print(cerr);
  }
  
  // clear put list

  _putList.erase(_putList.begin(), _putList.end());

  // compute the dir path from the incoming url
  
  string url_str(inMsg.getUrlStr());
  DsURL url(url_str);
  string dirPath;
  RapDataDir.fillPath(url, dirPath);

  // loop through the URLs in the array

  GetHost getHost;
  int iret = 0;

  for (size_t ii = 0; ii < urlArray.size(); ii++) {

    // directory path

    DsURL url(urlArray[ii]);
    string fwdPath;
    if (url.getFile().size() == 0) {
      // no dir specified, set to current dir
      string urlFile;
      RapDataDir.stripPath(dirPath, urlFile);
      fwdPath = dirPath;
      url.setFile(urlFile);
      url.getURLStr();
      if (_isDebug) {
	cerr << "** dir not provided, using current dir: " << urlFile << endl;
      }
    } else {
      // use dir in URL
      RapDataDir.fillPath(url.getFile(), fwdPath);
    }
    
    if (_isDebug) {
      cerr << "---> Forwarding for URL: " << url.getURLStr() << endl;
      cerr << "     _port: " << _port << endl;
      cerr << "     url.getPort(): " << url.getPort() << endl;
      cerr << "     url.getHost(): " << url.getHost() << endl;
      cerr << "     fwdPath: " << fwdPath << endl;
      cerr << "     dirPath: " << dirPath << endl;
    }

    // There are 3 put modes:
    //   TO_SERVER, LOCAL_DIR, LOCAL_FILE_MSG
    //
    //   LOCAL_FILE_MSG: triggered if hostname == "localfile".
    //     In this mode the message buffer is put to a file, not to the server.
    //
    //   TO_SERVER: put to server
    //
    //   LOCAL_DIR: local file write
    
    enum putMode_t {PUT_LOCAL_DIR, PUT_TO_SERVER, PUT_LOCAL_FILE_MSG};
    
    // default is to go to a server
    
    putMode_t putMode = PUT_TO_SERVER;
    
    // Look for the special host name; "localfile"
    
    if (!strcmp(url.getHost().c_str(), "localfile")) {

      putMode = PUT_LOCAL_FILE_MSG; 

    } else if (getHost.hostIsLocal(url.getHost()) &&
               (url.getPort() == _port || url.getPort() == -1)) {
      
      putMode = PUT_LOCAL_DIR; // do not recurse to this server
      if (_isDebug) {
        cerr << "  putting to local dir: " << url.getFile() << endl;
      }
      
    } // else if (getHost.hostIsLocal ...
    
    switch (putMode) {
      
      case PUT_LOCAL_DIR:
        if (_forwardLocal(url, inMsg)) {
          iret = -1;
        }
        break;
    
      case PUT_LOCAL_FILE_MSG:
        if (_writeMsgBufToFile(url, inMsg)) {
          iret = -1;
        }
        break;
      
      case PUT_TO_SERVER:
        if (_queueForwardingToServer(url)) {
          iret = -1;
        }
        break;
      
    } // switch

  } // ii

  // perform the pending puts in the list
  
  _performQueuedPuts(inMsg);

  // purge the children
  
  while (_nPutChildren > 0) {
    _purgeCompletedPuts();
    umsleep(1000);
  }

  return iret;

}

/////////////////////////////////////////////////
// write the message to a local URL specified
// in the forwarding list

int DsSpdbServer::_forwardLocal(const DsURL &url,
                                const DsSpdbMsg &inMsg)
  
{
  
  if (_isDebug) {
    cerr << "-----> DsSpdbServer::_forwardLocal, doing put to url: "
         << inMsg.getUrlStr() << endl;
  }

  // copy the input message, and set the URL

  DsSpdbMsg putMsg(inMsg);
  putMsg.setUrlStr(url.getURLStr());

  // put locally
  
  DsSpdb spdb;
  if (spdb.doMsgPut(putMsg)) {
    cerr << "ERROR - DsSpdbServer::_forward" << endl;
    cerr << "  URL: " << url.getURLStr() << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////
// write the entire message buffer to a local file
//
// This file will then be sent to a remote host, where
// it will be read and the message will be acted on
// to update a remote data base.
  
int DsSpdbServer::_writeMsgBufToFile(DsURL url,
                                     const DsSpdbMsg &inMsg)

{

  if (_isDebug) {
    cerr << "DEBUG - DsSpdbServer::_writeMsgBufToFile" << endl;
    cerr << "-----> Writing message file to dir: "
	 << _params->localfile_dest_dir << endl;
  }
  
  // Substitute a real hostname for "localfile" in the URL
  // so the far side can use it.
  
  url.setHost(_params->localfile_host);
  
  // Copy the input message to create the put message

  DsSpdbMsg putMsg(inMsg);
  DsSpdb::compression_t compression = inMsg.getDataBufCompression();
  if (_params->compress_for_forwarding) {
    if (_params->compression_method == Params::COMPRESSION_BZIP2) {
      compression = Spdb::COMPRESSION_BZIP2;
    } else {
      compression = Spdb::COMPRESSION_GZIP;
    }
  }
  putMsg.compressDataBuf(compression);
  putMsg.setUrlStr(url.getURLStr());
  putMsg.assemble(DsSpdbMsg::DS_SPDB_PUT, inMsg.getMode(), inMsg.getMessageCat());

  void *fileBuf = putMsg.assembledMsg();
  int fileBufLen = putMsg.lengthAssembled();
  
  // make sure directory exists
  
  if (ta_makedir_recurse(_params->localfile_dest_dir)) {
    int errNum = errno;
    cerr << "ERROR - DsSpdbServer::_writeMsgBufToFile" << endl;
    cerr << "  Cannot create dir: " << _params->localfile_dest_dir << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  
  // compute name and path of local file
      
  char fname[MAX_PATH_LEN];
  char fpath[MAX_PATH_LEN];
  FILE *fd;
  
  sprintf(fname,"ds_spdbp_%ld_%d.msg", time(NULL), getpid());
  sprintf(fpath,"%s%s%s",
	  _params->localfile_dest_dir, PATH_DELIM, fname);

  if(( fd= fopen(fpath,"w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - DsSpdbServer::_writeMsgBufToFile" << endl;
    cerr << "  Cannot open file for writing: " << fpath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // write the file
  
  if((int) fwrite(fileBuf,1,fileBufLen,fd) != fileBufLen) {
    int errNum = errno;
    cerr << "ERROR - DsSpdbServer::_writeMsgBufToFile" << endl;
    cerr << "  Cannot write to file: " << fpath << endl;
    cerr << strerror(errNum) << endl;
    fclose(fd);
    return -1;
  }
  fclose(fd);
  
  // Update LdataInfo file & FMQ
  
  DsLdataInfo ldatainfo;
  ldatainfo.setDir(_params->localfile_dest_dir);
  ldatainfo.setRelDataPath(fname);
  ldatainfo.setDataFileExt("msg");
  ldatainfo.setWriter("DsSpdbServer");
  ldatainfo.setUserInfo1("SPDB put message");
  ldatainfo.write(time(NULL));

  // execute a command if specified

  if (strlen(_params->localfile_command) > 0) {
    char cmd[MAX_PATH_LEN *2];
    sprintf(cmd, "%s %s", _params->localfile_command, fpath);
    safe_system(cmd, _params->localfile_command_timeout);
  }

  return 0;

}

////////////////////////////////////////////////
// queue information for forwarding to server

int DsSpdbServer::_queueForwardingToServer(const DsURL &url)
  
{
  
  if (_isDebug) {
    cerr << "-----> Forwarding to server" << endl;
    cerr << "       url: " << url.getURLStr() << endl;
  }

  // create DsSpdbPutArgs object for this put
  
  PutArgs *putArgs =
    new PutArgs(url.getURLStr(),
                _params->put_timeout_secs);
  
  // add to put list
  
  _putList.push_back(putArgs);
  
  return 0;

}

/////////////////////////////////////////////////////////////
// Perform any pending puts
// Called from the main thread.

void DsSpdbServer::_performQueuedPuts(const DsSpdbMsg &inMsg)

{

  // set up the put message
  
  DsSpdbMsg putMsg(inMsg);
  DsSpdb::compression_t compression = inMsg.getDataBufCompression();
  if (_params->compress_for_forwarding) {
    if (_params->compression_method == Params::COMPRESSION_BZIP2) {
      compression = Spdb::COMPRESSION_BZIP2;
    } else {
      compression = Spdb::COMPRESSION_GZIP;
    }
  }
  putMsg.compressDataBuf(compression);

  list<PutArgs *>::iterator ii;
  for (ii = _putList.begin(); ii != _putList.end(); ii++) {
    
    PutArgs *putArgs = (*ii);
    
    if (putArgs->childPid == 0) {

      // not put yet

      // set URL, assemble message

      putMsg.setUrlStr(putArgs->urlStr);
      putMsg.assemble(DsSpdbMsg::DS_SPDB_PUT,
                      inMsg.getMode(),
                      DsServerMsg::StartPut);

      // do the put

      _doPutInChild(putArgs, putMsg);
      _nPutChildren++;
      if (_isVerbose) {
	cerr << "Doing put in child, pid: " << putArgs->childPid << endl;
	cerr << "  " << DateTime::str() << endl;
	cerr << "  URL: " << putArgs->urlStr << endl;
      }
      
    } // if (putArgs->childPid == 0) 

  } // ii
    
}
	
/////////////////////////////////////////////////////////////
// Function for actually putting the data in a child process
// Called from the main thread.
// Note that the putMsg is not altered in the parent, only
// the child.

void DsSpdbServer::_doPutInChild(PutArgs *putArgs,
                                 DsSpdbMsg &putMsg)

{

  if (!_params->no_threads) {
    
    // spawn a child for doing the put
    
    putArgs->childPid = fork();
    
    // check for parent or child
    
    if (putArgs->childPid != 0) {
      // parent - return now
      return;
    }

  }
  
  // child

  // set the URL

  putMsg.setUrlStr(putArgs->urlStr);
  
  string modeStr;
  switch(putMsg.getMode()) {
    case DsSpdbMsg::DS_SPDB_PUT_MODE_OVER:
      modeStr = "over";
      break;
    case DsSpdbMsg::DS_SPDB_PUT_MODE_ADD:
      modeStr = "add";
      break;
    case DsSpdbMsg::DS_SPDB_PUT_MODE_ADD_UNIQUE:
      modeStr = "add-unique";
      break;
    case DsSpdbMsg::DS_SPDB_PUT_MODE_ONCE:
      modeStr = "once";
      break;
    case DsSpdbMsg::DS_SPDB_PUT_MODE_ERASE:
      modeStr = "erase";
      break;
    default:
      modeStr = "unknown";
  }

  // communicate with server, read reply

  DsURL url(putArgs->urlStr);
  string errStr;
  DsSpdbMsg replyMsg;
  int iret = _communicate(putMsg, url, replyMsg, errStr);
  if (iret) {
    cerr << "ERROR - COMM - DsSpdbServer::_doPutInChild" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Cannot communicate to put data" << endl;
    cerr << errStr;
    _logRemoteForwarding("error", modeStr, url, *putArgs, putMsg);
  } else {
    _logRemoteForwarding("success", modeStr, url, *putArgs, putMsg);
  }

  putArgs->childDone = true;

  _exit(0);

}

/////////////////////////////////////////////////
// communicate message to server, and read reply
//
// Load up error string
//
// Returns 0 on success, -1 on error.

int DsSpdbServer::_communicate(const DsSpdbMsg &outMsg,
                               DsURL url,
                               DsSpdbMsg &replyMsg,
                               string &errStr)
  
{

  if (_isDebug) {
    cerr << "-->> DsSpdbServer::_communicate - entry" << endl;
  }

  // resolve the port if needed

  if (url.getPort() < 0) {
    string locErrStr;
    if (DsLocator.resolvePort(url, NULL, false, &locErrStr, true)) {
      errStr += "ERROR - DsSpdbServer::_communicate\n";
      TaStr::AddStr(errStr, "  URL: ", url.getURLStr());
      errStr += "  Cannot resolve port from ServerMgr\n";
      errStr += locErrStr;
      return -1;
    }
  }

  // communicate with client - a DsSpdbServer

  DsClient client;
  if (_isDebug) {
    client.setDebug(true);
  }
  client.setErrStr("ERROR - DsSpdbServer::_communicate\n");
  
  if (_isDebug) {
    cerr << "-------------- SENDING REQUEST -------------" << endl;
    outMsg.print(cerr);
    cerr << "--------------------------------------------" << endl;
  }

  if (client.communicateAutoFwd(url, DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
				outMsg.assembledMsg(),
                                outMsg.lengthAssembled())) {
    errStr += client.getErrStr();
    TaStr::AddStr(errStr, "  URL: ", url.getURLStr());
    return -1;
  }
  
  // disassemble the reply
  
  if (replyMsg.disassemble(client.getReplyBuf(), client.getReplyLen())) {
    errStr += "  Invalid reply - cannot disassemble.\n";
    TaStr::AddStr(errStr, "  URL: ", url.getURLStr());
    return-1;
  }
  
  if (_isDebug) {
    cerr << "-------------- RECEIVING REPLY -------------" << endl;
    replyMsg.print(cerr);
    cerr << "--------------------------------------------" << endl;
  }

  if (_isDebug) {
    cerr << "-->> DsSpdbServer::_communicate - exit" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// Purge completed put children

void DsSpdbServer::_purgeCompletedPuts()

{

  bool done = false;
  while (!done) {
    
    done = true;
    
    list<PutArgs *>::iterator ii;
    for (ii = _putList.begin(); ii != _putList.end(); ii++) {
      PutArgs *putArgs = (*ii);

      // check if done
      
      if (waitpid(putArgs->childPid,
		  (int *) NULL,
		  (int) (WNOHANG | WUNTRACED)) == putArgs->childPid) {
	
	if (_isVerbose) {
	  cerr << "++++++++   pid done: " << putArgs->childPid << endl;
	  cerr << "++++++++   url: " << putArgs->urlStr << endl;
	}
	_nPutChildren--;
	delete putArgs;
	_putList.erase(ii);
	done = false;
	break;

      }

      // check for hung child, kill if timed out
      // waitpid() will get it later

      time_t now = time(NULL);
      if (now > putArgs->childExpireTime) {
	if (_isVerbose) {
	  cerr << "++++++++   killing pid: " << putArgs->childPid << endl;
	  cerr << "++++++++   url: " << putArgs->urlStr << endl;
	}
	if (kill(putArgs->childPid, SIGKILL)) {
	  cerr << "ERROR - DsSpdbServer::_purgeCompletedPuts()" << endl;
	  cerr << "  " << DateTime::str() << endl;
	  cerr << "  Cannot kill pid: " << putArgs->childPid << endl;
	  cerr << "  url: " << putArgs->urlStr << endl;
	}
      }

    } // ii

  } // while

}

//////////////////////////////////
// log remote forwarding

void DsSpdbServer::_logRemoteForwarding(const string &label,
                                        const string &modeStr,
                                        const DsURL &url,
                                        const PutArgs &putArgs,
                                        const DsSpdbMsg &putMsg)
  
{

  char *logDir = getenv("DATA_DISTRIB_LOG_DIR");
  if (logDir == NULL) {
    return;
  }

  string logName("DsSpdbServer");
  char *logSuffix = getenv("DATA_DISTRIB_LOG_SUFFIX");
  if (logSuffix != NULL) {
    logName += ".";
    logName += logSuffix;
  }
  int nchunks = putMsg.getNChunks();
  int nbytes = putMsg.lengthAssembled();
  time_t valid_time = putMsg.getChunkRefs()[nchunks-1].valid_time;
  time_t now = time(NULL);

  MsgLog log(logName);
  log.setDayMode();
  log.setAppendMode();
  log.setSuffix("log");
  log.setOutputDir(logDir);
  
  log.postMsg("%s, %7s, %8s, %18s, %8d, %s, %s",
	      DateTime::str(now, false).c_str(),
	      label.c_str(),
	      modeStr.c_str(),
	      url.getHost().c_str(),
	      nbytes,
	      DateTime::str(valid_time, false).c_str(),
	      url.getFile().c_str());

}

