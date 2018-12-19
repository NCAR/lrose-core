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
// DsLdataServer.cc
//
// DsLdataServer Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////

#include <sys/wait.h>

#include <toolsa/pmu.h>
#include <toolsa/Socket.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>

#include <didss/DsMsgPart.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DmapAccess.hh>

#include "DsLdataServer.hh"
using namespace std;

extern void tidy_and_exit (int sig);

//////////////////////////////////////////
// constructor
//
// Inherits from DsServer

DsLdataServer::DsLdataServer(string executableName,
			     string instanceName,
			     int port,
			     int maxQuiescentSecs,
			     int maxClients,
			     bool isDebug,
			     bool isVerbose,
			     bool noThreads,
                             bool isSecure,
                             bool isReadOnly,
			     const Params &params)
        : DsProcessServer(executableName,
                          instanceName,
                          port,
                          maxQuiescentSecs,
                          maxClients,
                          isDebug,
                          isVerbose,
                          isSecure,
                          isReadOnly,
                          true),
          _useThreads(!noThreads),
          _params(params)
  
{

  setNoThreadDebug(noThreads);

}

//////////////
// destructor

DsLdataServer::~DsLdataServer()

{

}

// virtual 
int DsLdataServer::handleDataCommand(Socket * socket,
				     const void *msgData,
                                     ssize_t msgLen)
{
  
  // loop until error or close occurs

  while(true) {

    if (_isVerbose) {
      cerr << "DsLdataServer::handleDataCommand() - start handling message" << endl;
    }
  
    DsLdataMsg inMsg;
    if (_isVerbose) {
      cerr << "Client thread disassembling message..." << endl;
      cerr << "  message len: " << msgLen << endl;
    }
    
    if (inMsg.disassemble((void *) msgData, msgLen)) {
      cerr << "ERROR - DsLdataServer::handleDataCommand" << endl;
      cerr << "  Invalid DsLdataMsg message - cannot proceed" << endl;
      return -1;
    }
    
    if (_isVerbose) {
      cerr << "---------- incoming message --------------" << endl;
      inMsg.print(cerr);
      cerr << "------------------------------------------" << endl;
    }
    
    // initialize the reply message
    
    DsLdataMsg outMsg;
    outMsg.setMode(DsLdataMsg::DS_LDATA_REPLY);
    outMsg.setSubType(inMsg.getMode());
    
    switch (inMsg.getMode()) {
      case DsLdataMsg::DS_LDATA_OPEN:
        _handleOpen(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_SET_DISPLACED_DIR_PATH:
        _handleSetDisplacedDirPath(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_SET_LDATA_FILE_NAME:
        _handleSetLdataFileName(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_SET_USE_XML:
        _handleSetUseXml(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_SET_USE_ASCII:
        _handleSetUseAscii(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_SET_SAVE_LATEST_READ_INFO:
        _handleSetSaveLatestReadInfo(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_SET_USE_FMQ:
        _handleSetUseFmq(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_SET_FMQ_NSLOTS:
        _handleSetFmqNSlots(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_SET_READ_FMQ_FROM_START:
        _handleSetReadFmqFromStart(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_READ:
        _handleRead(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_WRITE:
        _handleWrite(inMsg, outMsg);
        break;
      case DsLdataMsg::DS_LDATA_CLOSE:
        _handleClose(inMsg, outMsg);
        break;
      default: {
        outMsg.setErrorOccurred(true);
        string errStr;
        TaStr::AddInt(errStr, "Unknown message type: ", inMsg.getMode());
        outMsg.setErrStr(errStr);
        if (_isDebug) {
          cerr << "============ ERROR =============" << endl;
          cerr << errStr << endl;
          cerr << "================================" << endl;
        }
        return -1;
      }
    }
    
    // send reply
    
    outMsg.assemble();
    if (socket->writeMessage(DsLdataMsg::DsLdataMsg::DS_LDATA_REPLY,
                             outMsg.assembledMsg(),
                             outMsg.lengthAssembled())) {
      cerr << "ERROR - COMM -DsLdataServer::handleDataCommand" << endl;
      cerr << "  Writing reply - cannot continue" << endl;
      return -1;
    }
    
    if (_isVerbose) {
      cerr << "DsLdataServer::handleDataCommand() - done handling message." << endl;
    }
    
    if (inMsg.getMode() == DsLdataMsg::DS_LDATA_CLOSE) {
      if (_isDebug) {
        cerr << "DsLdataServer::handleDataCommand" << endl;
        cerr << "  Close command, child will exit" << endl;
      }
      return 0;
    }

    // Get the next request
    
    if (socket->readMessage()) {
      cerr << "ERROR - DsLdataServer::handleDataCommand" << endl;
      cerr << socket->getErrStr() << endl;
      cerr << "  Error reading incoming request - cannot continue" << endl;
      return -1;
    }
    
    // set the data pointer and length from the socket read

    msgData = socket->getData();
    msgLen = socket->getNumBytes();

  } // while (true)
  
  return 0;

}
    
//////////////////////////////////////////
// handle open request

int DsLdataServer::_handleOpen(const DsLdataMsg &inMsg,
                               DsLdataMsg &outMsg)

{

  _ldata.setDirFromUrl(inMsg.getUrlStr());
  _ldata.setDebug(_isVerbose);
  _ldata.setReadFmqFromStart(inMsg.getReadFmqFromStart());
  _ldata.setLdataFileName(inMsg.getLdataFileName().c_str());
  _ldata.setSaveLatestReadInfo(inMsg.getLatestReadInfoLabel(),
                               inMsg.getMaxValidAge(),
                               inMsg.getSaveLatestReadInfo());
  _ldata.setDisplacedDirPath(inMsg.getDisplacedDirPath());
  _ldata.setUseFmq(inMsg.getUseFmq());
  _ldata.setFmqNSlots(inMsg.getFmqNSlots());
  _ldata.setUseXml(inMsg.getUseXml());
  _ldata.setUseAscii(inMsg.getUseAscii());

  if (_isDebug) {
    cerr << "=========== setting up LdataInfo =============" << endl;
    _ldata.printFull(cerr);
    cerr << "==============================================" << endl;
  }
  
  return 0;

}

//////////////////////////////////////////
// handle setDisplacedDirPath request

int DsLdataServer::_handleSetDisplacedDirPath(const DsLdataMsg &inMsg,
                                              DsLdataMsg &outMsg)
  
{
  
  _ldata.setDisplacedDirPath(inMsg.getDisplacedDirPath());
  return 0;

}

//////////////////////////////////////////
// handle setLdataFileName request

int DsLdataServer::_handleSetLdataFileName(const DsLdataMsg &inMsg,
                                           DsLdataMsg &outMsg)
  
{

  _ldata.setLdataFileName(inMsg.getLdataFileName().c_str());
  return 0;

}

//////////////////////////////////////////
// handle setUseXml request

int DsLdataServer::_handleSetUseXml(const DsLdataMsg &inMsg,
                                    DsLdataMsg &outMsg)

{

  _ldata.setUseXml(inMsg.getUseXml());
  return 0;

}

//////////////////////////////////////////
// handle setUseAscii request

int DsLdataServer::_handleSetUseAscii(const DsLdataMsg &inMsg,
                                      DsLdataMsg &outMsg)

{

  _ldata.setUseAscii(inMsg.getUseAscii());
  return 0;

}

//////////////////////////////////////////
// handle setSaveLatestReadInfo request

int DsLdataServer::_handleSetSaveLatestReadInfo(const DsLdataMsg &inMsg,
                                                DsLdataMsg &outMsg)

{

  _ldata.setSaveLatestReadInfo(inMsg.getLatestReadInfoLabel(),
                               inMsg.getMaxValidAge(),
                               inMsg.getSaveLatestReadInfo());
  return 0;

}

//////////////////////////////////////////
// handle setUseFmq request

int DsLdataServer::_handleSetUseFmq(const DsLdataMsg &inMsg,
                                    DsLdataMsg &outMsg)

{
  
  _ldata.setUseFmq(inMsg.getUseFmq());
  return 0;

}

//////////////////////////////////////////
// handle setFmqNSlots request

int DsLdataServer::_handleSetFmqNSlots(const DsLdataMsg &inMsg,
                                       DsLdataMsg &outMsg)

{

  _ldata.setFmqNSlots(inMsg.getFmqNSlots());
  return 0;

}

//////////////////////////////////////////
// handle setReadFmqFromStart request

int DsLdataServer::_handleSetReadFmqFromStart(const DsLdataMsg &inMsg,
                                              DsLdataMsg &outMsg)

{

  _ldata.setReadFmqFromStart(inMsg.getReadFmqFromStart());
  return 0;

}

//////////////////////////////////////////
// handle read request

int DsLdataServer::_handleRead(const DsLdataMsg &inMsg,
                               DsLdataMsg &outMsg)

{

  int maxValidAge = inMsg.getMaxValidAge();
  bool forced = inMsg.getReadForced();

  // for non-forced read, if no data is available,
  // we do not add the ldata XML to the outgoing
  // message, but return success anyway.
  // The client DsLdataInfo will interpret this as data not
  // available at this time

  if (!forced) {
    if (_ldata.read(maxValidAge)== 0) {
      _ldata.assemble(true);
      outMsg.setLdataXml((char *) _ldata.getBufPtr());
    }
    return 0;
  }

  // in forced mode, we return error if we don't have
  // 
  int iret = 0;
  if (forced) {
    iret = _ldata.readForced(maxValidAge);
  } 

  if (iret) {

    outMsg.setErrorOccurred(true);
    string errStr = "ERROR - DsLdataServer::_handleRead\n";
    errStr += _ldata.getErrStr();
    outMsg.setErrStr(errStr);
    if (_isDebug) {
      cerr << "============ ERROR =============" << endl;
      cerr << errStr << endl;
      cerr << "================================" << endl;
    }
    return -1;
  }

  _ldata.assemble(true);
  outMsg.setLdataXml((char *) _ldata.getBufPtr());
  return 0;

}

//////////////////////////////////////////
// handle write request

int DsLdataServer::_handleWrite(const DsLdataMsg &inMsg,
                                DsLdataMsg &outMsg)
  
{

  bool fmqOnly = inMsg.getWriteFmqOnly();
  string ldataXml = inMsg.getLdataXml();

  // disassemble the XML representation of the LdataInfo, which 
  // sets up the _ldata object accordingly

  if (_ldata.disassemble(ldataXml.c_str(), ldataXml.size())) {
    outMsg.setErrorOccurred(true);
    string errStr = "ERROR - DsLdataServer::_handleWrite\n";
    errStr += "Cannot disassemble LdataInfo XML\n";
    errStr += ldataXml;
    errStr += "\n";
    errStr += _ldata.getErrStr();
    outMsg.setErrStr(errStr);
    if (_isDebug) {
      cerr << "============ ERROR =============" << endl;
      cerr << errStr << endl;
      cerr << "================================" << endl;
    }
    return -1;
  }
  
  int iret = 0;
  if (fmqOnly) {
    iret = _ldata.writeFmq();
  } else {
    iret = _ldata.write();
  }

  if (iret) {

    outMsg.setErrorOccurred(true);
    string errStr = "ERROR - DsLdataServer::_handleWrite\n";
    errStr += _ldata.getErrStr();
    outMsg.setErrStr(errStr);
    if (_isDebug) {
      cerr << "============ ERROR =============" << endl;
      cerr << errStr << endl;
      cerr << "================================" << endl;
    }
    return -1;
  }

  // register with DataMapper

  DmapAccess dmap;
  if (_ldata.isFcast()) {
    dmap.regLatestInfo(_ldata.getLatestTime(),
		       _ldata.getDataDirPath(),
		       _ldata.getDataType(),
		       _ldata.getLeadTime());
  } else {
    dmap.regLatestInfo(_ldata.getLatestTime(),
		       _ldata.getDataDirPath(),
		       _ldata.getDataType());
  }
  
  return 0;

}

//////////////////////////////////////////
// handle close request

int DsLdataServer::_handleClose(const DsLdataMsg &inMsg,
                                DsLdataMsg &outMsg)

{

  return 0;

}

