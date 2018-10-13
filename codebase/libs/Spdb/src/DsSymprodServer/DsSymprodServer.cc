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
// DsSymprodServer.cc
//
// Server class for serving out data in the symprod format.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include <cassert>
#include <Spdb/DsSpdb.hh>
#include <Spdb/DsSymprodServer.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/Socket.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <didss/RapDataDir.hh>
#include <dsserver/DsLocator.hh>
using namespace std;

//////////////////////////////////////////
// constructor
//
// Inherits from DsProcessServer

DsSymprodServer::DsSymprodServer(const string &prog_name,
				 const string &instance,
                                 const void *initialParams,
				 int port,
				 int qmax,
				 int max_clients,
				 bool no_threads,
				 bool is_debug,
				 bool is_verbose) :
        DsProcessServer(prog_name,
                        instance,
                        port,
                        qmax,
                        max_clients,
                        is_debug,
                        is_verbose,
                        true, true, true),
        _initialParams(initialParams)
  
{
  assert(initialParams != NULL);
  setNoThreadDebug(no_threads);
  _horizLimitsSet = false;
  _vertLimitsSet = false;
  _unique = Spdb::UniqueOff;
}

/////////////////////////////////////////////////////////
// handle the data command from the parent class
// always return true, so that parent will not exit

int DsSymprodServer::handleDataCommand(Socket * socket,
                                       const void * data, ssize_t dataSize)

{

  if (_isDebug) {
    cerr << "Entering DsSymprodServer::handleDataCommand()." << endl;
    cerr << "  " << DateTime::str() << endl;
  }

  if (_isVerbose) {
    cerr << "Client thread disassembling message..." << endl;
  }

  // disassemble the incoming request

  DsSpdbMsg msg;
  if (msg.disassemble((void *) data, dataSize)) {
    cerr << "ERROR - COMM - DsSymprodServer::handleDataCommand" << endl;
    cerr << "Invalid DsSpdbMsg message" << endl;
    return -1;
  }

  // check message type - can only handle get messages

  if (msg.getSubType() != DsSpdbMsg::DS_SPDB_GET) {
    cerr << "ERROR - DsSymprodServer::handleDataCommand\n"
         << "  Cannot handle request" << endl;
    msg.print(cerr);
    return 0;
  }
  
  // set horiz and vert limits if applicable

  setLimitsFromMsg(msg);

  // set auxiliary XML info

  _auxXml = msg.getAuxXml();
  
  if (_isDebug) {
    cerr << "------------------------------------" << endl;
    msg.print(cerr);
  }
  
  // verify the url and determine the spdb directory path

  string url_str(msg.getUrlStr());
  DsURL url(url_str);
  if (!url.isValid()) {
    cerr << "ERROR - COMM - DsSymprodServer::handleDataCommand" << endl;
    cerr << "   Invalid URL: '" << url_str <<  "'\n";
    return 0;
  }
  string dirPath;
  RapDataDir.fillPath(url, dirPath);

  //
  // Check for local copy of params
  // Override initial params if params file exists in the datatype directory
  //
  void *localParams = (void *) _initialParams;

  // NOTE: the DsLocator will resolve the parameter file name 
  //       and modify the url
  
  bool paramsExist = false;
  
  if (DsLocator.resolveParam(url, _executableName,
                             &paramsExist) != 0) {
    cerr << "ERROR - COMM - DsSpdbServer::handleDataCommand\n"
         << "Cannot resolve parameter specification in url:\n"
         << url.getURLStr()
         << endl;
    return(-1);
  }
  
  //
  // The application-specfic code must load the override parameters
  //
  if (paramsExist) {
    
    if (loadLocalParams(url.getParamFile(), 
                        &localParams) != 0) {
      cerr << "ERROR - COMM - DsSpdbServer::handleDataCommand\n"
           << "Cannot load parameter file:\n"
           << url.getParamFile()
           << endl;
      return(-1);
    }
  }

  // copy the read message

  _readMsg = msg;
  
  // process the get request

  _handleGet(localParams, msg, dirPath, *socket);
  
  if (_isDebug) {
    cerr << "Exiting DsSymprodServer::handleDataCommand()." << endl;
  }
  
  return 0;

}
    
// Override base class on timeout and post handlers
// always return true - i.e. never exit
bool DsSymprodServer::timeoutMethod()
{
  DsProcessServer::timeoutMethod();
  return true; // Continue to wait for clients.
}
bool DsSymprodServer::postHandlerMethod()
{
  DsProcessServer::postHandlerMethod();
  return true; // Continue to wait for clients.
}

/////////////////////////////////////////////////////////////////////
// transformData() - Transform the data from the database into
//                   symprod format.

void DsSymprodServer::transformData(const void *localParams,
				    const string &dir_path,
				    int prod_id,
				    const string &prod_label,
				    int n_chunks_in,
				    const Spdb::chunk_ref_t *chunk_refs_in,
				    const Spdb::aux_ref_t *aux_refs_in,
				    const void *chunk_data_in,
				    int &n_chunks_out,
				    MemBuf &refBufOut,
				    MemBuf &auxBufOut,
				    MemBuf &dataBufOut)
{

  // initialize the buffers
  
  if (_isDebug) cerr << "DsSymprodServer.transformData: entry n_chunks_in: "
    << n_chunks_in << "  prod_id: " << prod_id << endl;


  refBufOut.free();
  auxBufOut.free();
  dataBufOut.free();

  // Transform each chunk and add it to the memory buffers

  n_chunks_out = 0;
  MemBuf symprodBuf;
  
  for (int i = 0; i < n_chunks_in; i++) {

    Spdb::chunk_ref_t ref = chunk_refs_in[i];
    Spdb::aux_ref_t aux = aux_refs_in[i];
    void *chunk_data = (void *)((char *)chunk_data_in + ref.offset);

    if (_isDebug) cerr << "DsSymprodServer.transformData: chunk "
      << i << " of " << n_chunks_in << "  data_type: " << ref.data_type
      << "  data_type2: " << ref.data_type2 << "  len: " << ref.len << endl;

    
    symprodBuf.free();
    if (convertToSymprod(localParams, dir_path, prod_id, prod_label,
			 ref, aux, chunk_data, ref.len,
			 symprodBuf) == 0) {

      ref.offset = dataBufOut.getLen();
      ref.len = symprodBuf.getLen();
      refBufOut.add(&ref, sizeof(ref));
      auxBufOut.add(&aux, sizeof(aux));
      dataBufOut.add(symprodBuf.getPtr(), symprodBuf.getLen());
      n_chunks_out++;

    }
    
  }
  
  if (_isDebug) cerr << "DsSymprodServer.transformData: exit\n";
}

/////////////////////////////////////////////////////////////
// Handle a get request -- sets the prod_id and prod_label 
// in the get info appropriately

int DsSymprodServer::_handleGet(const void *localParams,
                                const DsSpdbMsg &inMsg, 
				const string &dirPath,
                                Socket &socket)
  
{
  if (_isDebug) cerr << "DsSymprodServer.handleGet: entry\n";

  DsSpdbMsg::info_t getInfo;
  DsSpdb spdb;
  DsSpdbMsg replyMsg;

  if (spdb.doMsgGet(inMsg, getInfo)) {
    
    string errStr = "ERROR - DsSypmrodServer::_handleGet()\n";
    errStr += spdb.getErrStr();
    errStr += "  URL: ";
    errStr += inMsg.getUrlStr();
    errStr += "\n";
    replyMsg.assembleGetErrorReturn(inMsg.getSpdbMode(), errStr.c_str());
    if (_isDebug) {
      cerr << errStr << endl;
    }

  } else {

    if (inMsg.getMode() == DsSpdbMsg::DS_SPDB_GET_MODE_TIMES) {

      replyMsg.assembleGetTimesSuccessReturn(inMsg.getSpdbMode(), getInfo);

    } else {

      if (_unique == Spdb::UniqueLatest) {
        spdb.makeUniqueLatest();
      } else if (_unique == Spdb::UniqueEarliest) {
        spdb.makeUniqueEarliest();
      }

      MemBuf refBufOut;
      MemBuf auxBufOut;
      MemBuf dataBufOut;
      int nChunksOut;
      
      transformData(localParams,
		    dirPath,
		    spdb.getProdId(), spdb.getProdLabel(),
		    spdb.getNChunks(),
		    spdb.getChunkRefs(), spdb.getAuxRefs(),
                    spdb.getChunkData(),
		    nChunksOut, refBufOut, auxBufOut, dataBufOut);
      
      if (_isDebug) {
        cerr << "Found data, nChunks: " << spdb.getNChunks() << endl;
      }
      _setProductId(getInfo, localParams);
      getInfo.n_chunks = nChunksOut;
      replyMsg.assembleGetDataSuccessReturn
        (inMsg.getSpdbMode(),
         getInfo, refBufOut,
         auxBufOut, dataBufOut,
         inMsg.getDataBufCompression());

    }
    
  }
    
  // send reply

  void *replyMsgBuf = replyMsg.assembledMsg();
  ssize_t replyBuflen = replyMsg.lengthAssembled();

  if (_isDebug) {
    cerr << "==== reply message ====" << endl;
    replyMsg.print(cerr);
    cerr << "=======================" << endl;
  }

  if (socket.writeMessage(DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
			  replyMsgBuf, replyBuflen, 1000)) {
    cerr << "ERROR - COMM - DsSpdbServer::_handleGet" << endl;
    cerr << "  Writing reply" << endl;
    return -1;
  }

  if (_isDebug) cerr << "DsSymprodServer.handleGet: exit\n";
  return 0;

}

///////////////////////////////////
// set the product id and label to SYMPROD

void DsSymprodServer::_setProductId(DsSpdbMsg::info_t &info, const void *localParams) {
      info.prod_id = SPDB_SYMPROD_ID;
      STRncopy(info.prod_label, SPDB_SYMPROD_LABEL, SPDB_LABEL_MAX);

}

///////////////////////////////////
// set the limits from the message

void DsSymprodServer::setLimitsFromMsg(const DsSpdbMsg &inMsg)

{

  if (inMsg.horizLimitsSet()) {
    const DsSpdbMsg::horiz_limits_t &hlimits = inMsg.getHorizLimits();
    _minLat = hlimits.min_lat;
    _minLon = hlimits.min_lon;
    _maxLat = hlimits.max_lat;
    _maxLon = hlimits.max_lon;
    _horizLimitsSet = true;
  }
  if (inMsg.vertLimitsSet()) {
    const DsSpdbMsg::vert_limits_t &vlimits = inMsg.getVertLimits();
    _minHt = vlimits.min_ht;
    _maxHt = vlimits.max_ht;
    _vertLimitsSet = true;
  }

}  

