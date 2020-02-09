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
//  Message for passing between DsFmq clients and DsFmqServer
//
//  Mike Dixon, RAL, NCAR, Boulder, CO, 80307, USA
//
//  Jan 2009
//
////////////////////////////////////////////////////////////////////////////////
                                 
#include <cassert>
#include <dataport/bigend.h>
#include <didss/DsMsgPart.hh>
#include <Fmq/DsFmqMsg.hh>
#include <toolsa/MsgLog.hh>
#include <toolsa/mem.h>

using namespace std;
   

///////////////////////////////////////////////
// constructor
   
DsFmqMsg::DsFmqMsg() : DsServerMsg()
{
  _initInfoSet = false;
}

///////////////////////////////////////////////
// destructor
   
DsFmqMsg::~DsFmqMsg()
{

}

///////////////////////////////////////////////////////////////
// clear everything - overriding parent class

void DsFmqMsg::clearAll()
{
  DsServerMsg::clearAll();
  _initInfoSet = false;
  _msgInfo.clear();
  _msgData.clear();
}

///////////////////////////////////////////////
// assemble request init message
   
void DsFmqMsg::assembleRequestInit(const string &url,
				   const string &procName,
				   bool debug,
				   Fmq::openMode openMode,
				   Fmq::openPosition openPosition,
				   bool compress,
				   size_t numSlots,
				   size_t bufSize)
{

  clearAll();
  setType(DS_FMQ_INIT);
  setCategory(StartGet);

  clearInitInfo();
  STRncopy(_initInfo.procName, procName.c_str(), PROC_NAME_LEN);
  _initInfo.debug = (si32) debug;
  _initInfo.openMode = (si32) openMode;
  _initInfo.openPosition = (si32) openMode;
  _initInfo.compress = (si32) compress;
  _initInfo.numSlots = (si32) numSlots;
  _initInfo.bufSize = (si32) bufSize;

  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleRequestInit" << endl;
    printInitInfo(cerr, "  ", _initInfo);
  }

  BEfromInitInfo();
  addPart(DS_FMQ_INIT_PART, sizeof(initInfo_t), &_initInfo);
  addURL(url);
  assemble();

}

///////////////////////////////////////////////
// assemble set compression method
   
void DsFmqMsg::assembleSetCompressionMethod(ta_compression_method_t method)
{

  clearAll();
  setType(DS_FMQ_SET_COMPRESSION_METHOD);
  setSubType(method);

  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleSetCompressionMethod" << endl;
    cerr << "     method: " << method << endl;
  }

  assemble();

}

///////////////////////////////////////////////
// assemble set blocking write
   
void DsFmqMsg::assembleSetBlockingWrite()
{

  clearAll();
  setType(DS_FMQ_SET_BLOCKING_WRITE);

  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleSetBlockingWrite" << endl;
  }

  assemble();

}

///////////////////////////////////////////////
// assemble set single writer mode
   
void DsFmqMsg::assembleSetSingleWriter()
{

  clearAll();
  setType(DS_FMQ_SET_SINGLE_WRITER);

  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleSetSingleWriter" << endl;
  }

  assemble();

}

///////////////////////////////////////////////
// assemble set register with data mapper
   
void DsFmqMsg::assembleSetRegisterWithDmap(bool doReg,
					   int regIntervalSecs)
{

  clearAll();
  setType(DS_FMQ_SET_REG_WITH_DMAP);
  setSubType(doReg);
  setMode(regIntervalSecs);

  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleSetRegisterWithDmap" << endl;
    cerr << "     doReg: " << doReg << endl;
    cerr << "     regIntervalSecs: " << regIntervalSecs << endl;
  }

  assemble();

}

///////////////////////////////////////////////
// assemble request seek message
   
void DsFmqMsg::assembleRequestSeek(Fmq::seekPosition pos)
{

  clearAll();
  setType(DS_FMQ_SEEK);
  setSubType(pos);

  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleRequestSeek" << endl;
    cerr << "      pos: " << pos << endl;
  }

  assemble();

}

///////////////////////////////////////////////
// assemble request seek-to-id message
   
void DsFmqMsg::assembleRequestSeekToId(int id)
{

  clearAll();
  setType(DS_FMQ_SEEK_TO_ID);
  setSubType(id);

  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleRequestSeekToId" << endl;
    cerr << "      id: " << id << endl;
  }

  assemble();

}

///////////////////////////////////////////////
// assemble request read message
   
void DsFmqMsg::assembleRequestRead(int requestedType, int msecsSleep)
{

  clearAll();
  setType(DS_FMQ_READ);
  setSubType(requestedType);
  setMode(msecsSleep);

  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleRequestRead" << endl;
    if (requestedType >= 0) {
      cerr << "     Requested message type: " << requestedType << endl;
    }
    if (msecsSleep >= 0) {
      cerr << "     Requested msecs sleep: " << msecsSleep << endl;
    }
  }

  assemble();

}

///////////////////////////////////////////////
// Add read data for a single message in
// preparation for calling assembleReadReply().
// You need to call clearAll() before the first add.

void DsFmqMsg::addReadData(const Fmq &fmq)

{

  msgInfo_t info;
  MEM_zero(info);
  
  info.msgType = fmq.getMsgType();
  info.msgSubtype = fmq.getMsgSubtype();
  info.msgId = fmq.getMsgId();
  info.msgTime = fmq.getMsgTime();
  if (fmq.isMsgCompressed()) {
    info.msgLen = fmq.getMsgLen();
    info.msgPreCompressed = true;
    info.msgUncompressedLen = fmq.getSlotMsgLen();
  } else {
    info.msgLen = fmq.getMsgLen();
    info.msgPreCompressed = false;
    info.msgUncompressedLen = fmq.getMsgLen();
  }
  
  if (_debug) {
    cerr << "==>> DsFmqMsg::addMessageData" << endl;
    printMsgInfo(cerr, "  ", info);
  }

  // add it to the message
  
  BEfromInfo(&info);
  addPart(DS_FMQ_INFO_PART, sizeof(msgInfo_t), &info);
  addPart(DS_FMQ_DATA_PART,
	  fmq.getMsgLen(), fmq.getMsg());

}

///////////////////////////////////////////////
// Assemble reply after successful read.
// Assumes clearAll() was called, and data was
// added with a series of calls to addReadData()

void DsFmqMsg::assembleReadReply(int msgType, const Fmq &fmq)
  
{
  
  setType(DS_FMQ_REPLY);
  setSubType(msgType);

  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleReadReply" << endl;
  }
  
  assemble();

}

///////////////////////////////////////////////////////
// Add write data for a single message, in preparation
// for calling assembleRequestWrite.
// You need to call clearAll() before the first add.

void DsFmqMsg::addWriteData(int msgType, int msgSubtype, 
			    const void *msg, int msgLen,
			    bool compress,
			    ta_compression_method_t cmethod)

{

  msgInfo_t info;
  MEM_zero(info);
  info.msgType = msgType;
  info.msgSubtype = msgSubtype;
  info.msgLen = msgLen;
  info.msgPreCompressed = false;
  info.msgUncompressedLen = msgLen;
  
  if (_debug) {
    cerr << "==>> DsFmqMsg::addWriteData" << endl;
    printMsgInfo(cerr, "  ", info);
  }

  if (msg == NULL) {

    // dummy message
    // set to zero length
    info.msgLen = 0;
    info.msgPreCompressed = false;
    info.msgUncompressedLen = 0;
    BEfromInfo(&info);
    addPart(DS_FMQ_INFO_PART, sizeof(msgInfo_t), &info);

    // add part with no length
    char dummy;
    addPart(DS_FMQ_DATA_PART, 0, &dummy);

  } else {
    
    ui64 clen = msgLen;
    const void *cmsg = msg;

    if (compress) {
      if ((cmsg = ta_compress(cmethod, msg, msgLen, &clen)) == NULL) {
	cerr << "WARNING - DsFmqMsg::addWriteData" << endl;
	cerr << "  Compression failed - cannot compress message" << endl;
	compress = false;
      } else {
	info.msgPreCompressed = true;
	info.msgLen = clen;
	info.msgUncompressedLen = msgLen;
      }
    }
    BEfromInfo(&info);
    addPart(DS_FMQ_INFO_PART, sizeof(msgInfo_t), &info);
    addPart(DS_FMQ_DATA_PART, clen, cmsg);

    if (compress) {
      ta_compress_free((void *) cmsg);
    }

  } // if (msg == NULL)
  
}

///////////////////////////////////////////////
// assemble request write message
// Assumes clearAll() was called, and data was
// added with a series of calls to addwriteData()
   
void DsFmqMsg::assembleRequestWrite()
{
  
  setType(DS_FMQ_WRITE);
  
  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleRequestWrite" << endl;
  }

  assemble();

}

///////////////////////////////////////////////
// assemble write request for single message
   
void DsFmqMsg::assembleRequestWrite(int msgType, int msgSubtype, 
				    const void *msg, int msgLen,
				    bool compress,
				    ta_compression_method_t cmethod)
{

  clearAll();
  addWriteData(msgType, msgSubtype, msg, msgLen, compress, cmethod);
  assembleRequestWrite();

}

///////////////////////////////////////////////
// assemble request close
   
void DsFmqMsg::assembleRequestClose()
{

  clearAll();
  setType(DS_FMQ_CLOSE);
  setCategory(EndSeries);
  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleRequestClose" << endl;
  }

  assemble();

}

///////////////////////////////////////////////
// assemble reply to successful request

void DsFmqMsg::assembleSuccessReply(int msgType)

{

   // Initialize the return message
  
  clearAll();
  setType(DS_FMQ_REPLY);
  setSubType(msgType);
   
  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleSuccessReply" << endl;
  }

  assemble();

}

///////////////////////////////////////////////
// assemble error reply

void DsFmqMsg::assembleErrorReply(int msgType,
				  const string &errorStr)

{

  // Initialize the return message
  
  clearAll();
  setType(DS_FMQ_REPLY);
  setSubType(msgType);
  setError(-1);
  addErrString(errorStr);
   
  if (_debug) {
    cerr << "==>> DsFmqMsg::assembleErrorReply" << endl;
    cerr << "     errorStr: " << errorStr << endl;
  }
  
  assemble();

}

///////////////////////////////////////////////
// disassemble the message
// returns 0 on success, -1 on error

int DsFmqMsg::disassemble(const void *msg,
			  const int msgLen,
			  Fmq &fmq)
  
{
  
  // Disassemble the message and see what happened
  
  if (DsServerMsg::disassemble(msg, msgLen)) {
    fmq._print_error("DsFmqMsg::disassemble",
		     "  Failed in call to DsMessage::disassemble()");
    return -1;
  }
  
  if (_type < DS_FMQ_INIT ||
      _type > DS_FMQ_REPLY) {
    fmq._print_error("DsFmqMsg::disassemble",
		     "Unknown reply type: %d\n"
		     "IMPORTANT:   You have probably contacted an old DsFmqServer\n"
		     "     NOTE: You need to upgrade to latest version of DsFmqServer",
		     _type);
    return -1;
  }

  if (_debug) {
    cerr << "==>> DsFmqMsg::disassemble, message type: " << msgType2Str(_type) << endl;
  }
  
  // init request

  _initInfoSet = false;
  if (partExists(DS_FMQ_INIT_PART)) {
    clearInitInfo();
    memcpy(&_initInfo,
	   (initInfo_t *)getPartByType(DS_FMQ_INIT_PART)->getBuf(),
	   sizeof(initInfo_t));
    BEtoInitInfo();
    fmq._progName = _initInfo.procName;
    fmq._debug = _initInfo.debug;
    fmq._compress = _initInfo.compress;
    fmq._openMode = (Fmq::openMode) _initInfo.openMode;
    fmq._openPosition = (Fmq::openPosition) _initInfo.openPosition;
    fmq._numSlots = _initInfo.numSlots;
    fmq._bufSize = _initInfo.bufSize;
    if (_debug) {
      cerr << "  Found full init" << endl;
      printInitInfo(cerr, "  ", _initInfo);
    }
    _initInfoSet = true;
  }

  // info and data parts

  int nInfoParts = partExists(DS_FMQ_INFO_PART);
  int nDataParts = partExists(DS_FMQ_DATA_PART);
  if (nInfoParts != nDataParts) {
    fmq._print_error("DsFmqMsg::disassemble",
		     "  nInfoParts != nDataParts\n"
		     "  nInfoParts = %d\n"
		     "  nDataparts = %d\n",
		     nInfoParts, nDataParts);
    return -1;
  }

  if (_debug && nInfoParts > 0) {
      cerr << "Found n info parts: " << nInfoParts << endl;
  }

  _msgInfo.clear();
  if (nInfoParts > 0) {
    for (int i = 0; i < nInfoParts; i++) {
      DsMsgPart *part = getPartByType(DS_FMQ_INFO_PART, i);
      msgInfo_t info;
      memcpy(&info, (msgInfo_t *)part->getBuf(), sizeof(info));
      BEtoInfo(&info);
      _msgInfo.push_back(info);
    }
  }
  _msgData.clear();
  if (nDataParts > 0) {
    for (int i = 0; i < nDataParts; i++) {
      DsMsgPart *part = getPartByType(DS_FMQ_DATA_PART, i);
      _msgData.push_back((void *) part->getBuf());
    }
  }

  return 0;

}

/////////////////////////////////////////////////////
// Struct initialization

void DsFmqMsg::clearInitInfo()
{
  memset(&_initInfo, 0, sizeof(initInfo_t));
}

/////////////////////////////////////////////////////
// Byte swapping

void DsFmqMsg::BEfromInitInfo()
{
  BE_from_array_32(&_initInfo, sizeof(initInfo_t) - PROC_NAME_LEN);
}

void DsFmqMsg::BEtoInitInfo()
{
  BE_to_array_32(&_initInfo, sizeof(initInfo_t) - PROC_NAME_LEN);
}

void DsFmqMsg::BEfromInfo(msgInfo_t *info)
{
  BE_from_array_32(info, sizeof(msgInfo_t));
}

void DsFmqMsg::BEtoInfo(msgInfo_t *info)
{
  BE_to_array_32(info, sizeof(msgInfo_t));
}

/////////////////////////////////////////////////////
// String representation

string DsFmqMsg::msgType2Str(int msgType)

{

  switch(msgType) {

    case DS_FMQ_MESSAGE:
      return "DS_FMQ_MESSAGE";

    case DS_FMQ_INIT:
      return "DS_FMQ_INIT";

    case DS_FMQ_SET_COMPRESSION_METHOD:
      return "DS_FMQ_SET_COMPRESSION_METHOD";

    case DS_FMQ_SET_BLOCKING_WRITE:
      return "DS_FMQ_SET_BLOCKING_WRITE";

    case DS_FMQ_SET_SINGLE_WRITER:
      return "DS_FMQ_SET_SINGLE_WRITER";

    case DS_FMQ_SET_REG_WITH_DMAP:
      return "DS_FMQ_SET_REG_WITH_DMAP";

    case DS_FMQ_SEEK:
      return "DS_FMQ_SEEK";

    case DS_FMQ_READ:
      return "DS_FMQ_READ";

    case DS_FMQ_WRITE:
      return "DS_FMQ_WRITE";

    case DS_FMQ_CLOSE:
      return "DS_FMQ_CLOSE";

    case DS_FMQ_REPLY:
      return "DS_FMQ_REPLY";

  }

  return "DS_FMQ_MSG_TYPE_UNKNOWN";

}  
 
string DsFmqMsg::msgPart2Str(int msgPart)

{

  switch(msgPart) {

    case DS_FMQ_URL_PART:
      return "DS_FMQ_URL_PART";
      
    case DS_FMQ_ERR_STRING_PART:
      return "DS_FMQ_ERR_STRING_PART";
      
    case DS_FMQ_INIT_PART:
      return "DS_FMQ_INIT_PART";

    case DS_FMQ_INFO_PART:
      return "DS_FMQ_INFO_PART";
      
    case DS_FMQ_DATA_PART:
      return "DS_FMQ_DATA_PART";
      
  }

  return "DS_FMQ_MSG_PART_UNKNOWN";

}  
 
/////////////////////////////////////////////////////
// Printing

void DsFmqMsg::printInitInfo(ostream &out, const char *spacer,
			     const initInfo_t &init)
{
  
  out << spacer << "debug: " << init.debug << endl;
  out << spacer << "procName: " << init.procName << endl;
  out << spacer << "openMode: " << init.openMode << endl;
  out << spacer << "openPos: " << init.openPosition << endl;
  out << spacer << "compress: " << init.compress << endl;
  out << spacer << "numSlots: " << init.numSlots << endl;
  out << spacer << "bufSize: " << init.bufSize << endl;
  
}

void DsFmqMsg::printMsgInfo(ostream &out, const char *spacer,
			    const msgInfo_t &info)
{
  
  out << spacer << "msgType: " << info.msgType << endl;
  out << spacer << "msgSubtype: " << info.msgSubtype << endl;
  out << spacer << "msgLen: " << info.msgLen << endl;
  out << spacer << "msgPreCompressed: " << info.msgPreCompressed << endl;
  out << spacer << "msgUncompressedLen: " << info.msgUncompressedLen << endl;
  out << spacer << "msgId: " << info.msgId << endl;
  out << spacer << "msgTime: " << info.msgTime << endl;

}

