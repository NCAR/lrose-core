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
////////////////////////////////////////////////////////////////////////////////
//
// DsFmqServer.cc
// DsFmqServer class
//
// Mike Dixon, RAL, NCAR, Boulder, CO, USA
// Jan 2009
//
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <string>
#include <didss/RapDataDir.hh>
#include <didss/DsMessage.hh>
#include <Fmq/DsFmq.hh>
#include <Fmq/DsFmqMsg.hh>
#include <toolsa/Socket.hh>
#include <toolsa/TaStr.hh>
#include "DsFmqServer.hh"
using namespace std;

DsFmqServer::DsFmqServer(const string& progName,
			 const Params& params) :
	
	DsProcessServer(progName, 
			params.instance,
			params.port, 
			params.qmax, 
			params.max_clients,
			params.debug, 
			params.verbose,
                        params.run_secure, 
                        false, 
                        true),
	_progName(progName),
	_params(params)
  
{
  if (_params.no_threads) {
    setNoThreadDebug(true);
  }
}

int DsFmqServer::handleDataCommand(Socket* socket,
				   const void* data,
				   ssize_t dataSize)
{

  _socket = socket;

  // loop until error or close occurs

  while(true) {

    // Disassemble the message request
    
    DsFmqMsg msg;
    if (_isVerbose) {
      msg.setDebug();
    }
    if (msg.disassemble((void *)_socket->getData(), _socket->getNumBytes(), _fmq)) {
      // error on disassemble
      if (_isVerbose) {
	cerr << _fmq.getErrStr() << endl;
      }
      msg.assembleErrorReply(DsFmqMsg::DS_FMQ_MESSAGE, _fmq.getErrStr());
      if (_socket->writeMessage(0, msg.assembledMsg(), msg.lengthAssembled())) {
	cerr << "ERROR - DsFmqServer::handleDataCommand." << endl;
	cerr << "  Error on disassemble" << endl;
	cerr << "  Sending reply to client" << endl;
	cerr << _socket->getErrStr() << endl;
	return -1;
      }
    }

    // Handle each request type as necessary

    switch(msg.getType()) {
      
      case DsFmqMsg::DS_FMQ_INIT: {
	_performInit(msg);
      } break;
	
      case DsFmqMsg::DS_FMQ_SET_COMPRESSION_METHOD: {
	_setCompressionMethod(msg);
      }	break;

      case DsFmqMsg::DS_FMQ_SET_BLOCKING_WRITE: {
	_setBlockingWrite(msg);
      }	break;

      case DsFmqMsg::DS_FMQ_SET_SINGLE_WRITER: {
	_setSingleWriter(msg);
      }	break;

      case DsFmqMsg::DS_FMQ_SET_REG_WITH_DMAP: {
	_setRegisterWithDmap(msg);
      }	break;
	
      case DsFmqMsg::DS_FMQ_SEEK: {
	_performSeek(msg);
      }	break;
	
      case DsFmqMsg::DS_FMQ_READ: {
	_performRead(msg);
      }	break;

      case DsFmqMsg::DS_FMQ_WRITE: {
	_performWrite(msg);
      }	break;

      case DsFmqMsg::DS_FMQ_CLOSE: {
	_performClose(msg);
	return 0;
      }	break;

    } // case

    // Get the next request
    
    if (_socket->readMessage()) {
      cerr << "ERROR - DsFmqServer::handleDataCommand" << endl;
      cerr << _socket->getErrStr() << endl;
      return 0;
    }

  } // while

  return 0;

}

////////////////////////////////////////////
// perform initialization

int DsFmqServer::_performInit(DsFmqMsg &msg)

{
  
  // get URL

  const string &urlStr = msg.getUrlStr();
  if (urlStr.size() == 0) {
    string errStr = "ERROR - DsFmqServer::_performFullInit\n";
    errStr += "  DS_FMQ_URL_PART missing from message\n";
    if (_sendReply(msg.getType(), -1, errStr)) {
      return -1;
    }
    return -1;
  }
  DsURL url(urlStr);
  if (!url.isValid()) {
    string errStr = "ERROR - DsFmqServer::_performFullInit\n";
    TaStr::AddStr(errStr, "  Invalid URL: ", urlStr);
    if (_sendReply(msg.getType(), -1, errStr)) {
      return -1;
    }
    return -1;
  }
  string fmqPath;
  RapDataDir.fillPath(url.getFile(), fmqPath);

  // get info

  if (!msg.initInfoSet()) {
    string errStr = "ERROR - DsFmqServer::_performFullInit\n";
    errStr += "  DS_FMQ_INIT_PART missing from message\n";
    if (_sendReply(msg.getType(), -1, errStr)) {
      return -1;
    }
    return -1;
  }
  const DsFmqMsg::initInfo_t &info = msg.getInitInfo();
  
  // perform init

  if (_fmq.init(fmqPath.c_str(),
		info.procName,
		_isVerbose,
		(Fmq::openMode) info.openMode,
		(Fmq::openPosition) info.openPosition,
		info.compress,
		info.numSlots,
		info.bufSize)) {
    if (_sendReply(msg.getType(), -1, _fmq.getErrStr())) {
      return -1;
    }
    return -1;
  }

  // set flag to indicate to Fmq object that this is a server
  // so that it can handle compression appropriately

  _fmq.setServer();

  // send reply to indicate success

  if (_sendReply(msg.getType(), 0, "")) {
    return -1;
  }

  return 0;

}

// set compression method

int DsFmqServer::_setCompressionMethod(DsFmqMsg &msg)
{

  // set local object
  
  ta_compression_method_t method = (ta_compression_method_t) msg.getSubType();
  _fmq.setCompressionMethod(method);

  // send reply to client

  if (_sendReply(msg.getType(), 0, "")) {
    return -1;
  }

  return 0;

}

// set write to block if queue is full

int DsFmqServer::_setBlockingWrite(DsFmqMsg &msg)
{

  // set local object
  
  _fmq.setBlockingWrite();

  // send reply to client
  
  if (_sendReply(msg.getType(), 0, "")) {
    return -1;
  }
  
  return 0;

}

// set flag that only a single writer is in operation
// so that locking is not required

int DsFmqServer::_setSingleWriter(DsFmqMsg &msg)
{

  // set local object
  
  _fmq.setSingleWriter();

  // send reply to client
  
  if (_sendReply(msg.getType(), 0, "")) {
    return -1;
  }
  
  return 0;

}

// set register with DataMapper

int DsFmqServer::_setRegisterWithDmap(DsFmqMsg &msg)
{

  // set local object
  
  bool doReg = (bool) msg.getSubType();
  int regIntervalSecs = msg.getMode();
  _fmq.setRegisterWithDmap(doReg, regIntervalSecs);

  // send reply to client
  
  if (_sendReply(msg.getType(), 0, "")) {
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////////
// perform seek

int DsFmqServer::_performSeek(DsFmqMsg &msg)

{

  Fmq::seekPosition seekPos = (Fmq::seekPosition) msg.getSubType();
  if (_fmq.seek(seekPos)) {
    if (_sendReply(msg.getType(), -1, _fmq.getErrStr())) {
      return -1;
    }
    return -1;
  }

  // send reply to indicate success

  if (_sendReply(msg.getType(), 0, "")) {
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////
// perform seek to id

int DsFmqServer::_performSeekToId(DsFmqMsg &msg)

{

  int id = msg.getSubType();
  if (_fmq.seekToId(id)) {
    if (_sendReply(msg.getType(), -1, _fmq.getErrStr())) {
      return -1;
    }
    return -1;
  }

  // send reply to indicate success

  if (_sendReply(msg.getType(), 0, "")) {
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////
// perform read

int DsFmqServer::_performRead(DsFmqMsg &msg)

{

  // set requested type from the incoming message subtype

  int requestedType = msg.getSubType();

  // create an output message, clear it

  DsFmqMsg outMsg;
  if (_isVerbose) {
    outMsg.setDebug();
  }
  outMsg.clearAll();

  // loop until we run out of data, we get more than
  // 100 messages, or the total len exceeds a Mbyte

  int count = 0;
  int totLen = 0;
  bool done = false;

  while (!done) {

    // read message

    bool gotOne = false;
    if (_fmq.readMsg(&gotOne, requestedType)) {
      if (_sendReply(msg.getType(), -1, _fmq.getErrStr())) {
	return -1;
      }
      return -1;
    }

    if (gotOne) {
      outMsg.addReadData(_fmq);
      count++;
      totLen += _fmq.getMsgLen();
    }

    if (!gotOne ||
	count > DsFmq::MAX_READ_NMESSAGES ||
	totLen > DsFmq::MAX_READ_NBYTES) {
      done = true;
    }

  } // while
  
  // assemble outgoing message

  outMsg.assembleReadReply(msg.getType(), _fmq);

  // send reply containing read data

  assert(_socket);
  if (_socket->writeMessage(DsFmqMsg::DS_FMQ_MESSAGE,
			    outMsg.assembledMsg(),
			    outMsg.lengthAssembled())) {
    cerr << "ERROR - FmqMgrServer::sendReadReply" << endl;
    cerr << "  Sending read reply" << endl;
    cerr << _socket->getErrString() << endl;
    return -1;
  }

  return 0;

}

// perform write

int DsFmqServer::_performWrite(DsFmqMsg &msg)
{

  const vector<void *> msgData = msg.getMsgData();
  const vector<DsFmqMsg::msgInfo_t> msgInfo = msg.getMsgInfo();

  int iret = 0;
    
  for (int ii = 0; ii < (int) msgInfo.size(); ii++) {

    if (msgInfo[ii].msgPreCompressed) {
      int uncompLen = msgInfo[ii].msgUncompressedLen;
      if (_fmq.writeMsgPreCompressed(msgInfo[ii].msgType,
				     msgInfo[ii].msgSubtype,
				     msgData[ii],
				     msgInfo[ii].msgLen,
				     uncompLen)) {
	iret = -1;
      }
    } else {
      if (_fmq.writeMsg(msgInfo[ii].msgType,
			msgInfo[ii].msgSubtype,
			msgData[ii],
			msgInfo[ii].msgLen)) {
	iret = -1;
      }
    }

  } // ii

  if (iret == 0) {
    // success
    if (_sendReply(msg.getType(), 0, "")) {
      return -1;
    }
  } else {
    // failure
    if (_sendReply(msg.getType(), -1, _fmq.getErrStr())) {
      return -1;
    }
    return -1;
  }
    
  return 0;

}

// close queue

int DsFmqServer::_performClose(DsFmqMsg &msg)
{

  // close local object

  _fmq.closeMsgQueue();
  
  return 0;

}


//////////////////////////////////////////////////////////////////
// generic reply to client

int DsFmqServer::_sendReply(int msgType,
			    int status,
			    const string &errorStr)
  
{

  assert(_socket);
  
  DsFmqMsg replyMsg;
  if (_isVerbose) {
    replyMsg.setDebug();
  }
  if (status == 0) {
    replyMsg.assembleSuccessReply(msgType);
  } else {
    replyMsg.assembleErrorReply(msgType, errorStr);
  }
  
  if (_socket->writeMessage(DsFmqMsg::DS_FMQ_MESSAGE,
			    replyMsg.assembledMsg(),
			    replyMsg.lengthAssembled())) {
    cerr << "ERROR - FmqMgrServer::sendReply" << endl;
    cerr << "  Sending reply" << endl;
    cerr << "  Message type: " << DsFmqMsg::msgType2Str(msgType) << endl;
    cerr << _socket->getErrString() << endl;
    return -1;
  }

  return 0;

}

