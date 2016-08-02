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
// Message for passing between DsFmq clients and DsFmqServer
//
// Mike Dixon, RAL, NCAR, Boulder, CO, 80307, USA
//
// Jan 2009
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef _DS_FMQ_MSG_INC_
#define _DS_FMQ_MSG_INC_

#include <Fmq/Fmq.hh>
#include <dsserver/DsServerMsg.hh>
using namespace std;
 
class DsFmqMsg : public DsServerMsg
{

public:
 
  // Client/Server message type definitions

  typedef enum {
    DS_FMQ_MESSAGE = 43000,
    DS_FMQ_INIT = 43001,
    DS_FMQ_SET_COMPRESSION_METHOD = 43002,
    DS_FMQ_SET_BLOCKING_WRITE = 43003,
    DS_FMQ_SET_SINGLE_WRITER = 43004,
    DS_FMQ_SET_REG_WITH_DMAP = 43005,
    DS_FMQ_SEEK = 43006,
    DS_FMQ_READ = 43007,
    DS_FMQ_WRITE = 43008,
    DS_FMQ_CLOSE = 43009,
    DS_FMQ_REPLY = 43010,
    DS_FMQ_SEEK_TO_ID = 43011
  } msgType_t;
 
  // Client/Server message part definitions
  // LEGACY_INIT - old style, need init followed by open and seek
  // FULL_INIT - new style, does open and initial seek in one step

  typedef enum {
    DS_FMQ_URL_PART = DsServerMsg::DS_URL,                 // 1
    DS_FMQ_ERR_STRING_PART = DsServerMsg::DS_ERR_STRING,   // 8
    DS_FMQ_INIT_PART = 44001,
    DS_FMQ_INFO_PART = 44002,
    DS_FMQ_DATA_PART = 44003
  } msgPart_t ;

  // Struct for initializing an fmq via the server
 
  static const int PROC_NAME_LEN = 32;
  
  typedef struct {
    si32 debug;
    si32 compress;
    si32 openMode;
    si32 openPosition;
    si32 numSlots;
    si32 bufSize;
    si32 spare[10];
    char procName[PROC_NAME_LEN];
  } initInfo_t;

  // Struct containing message properties

  typedef struct {
    si32 msgType;
    si32 msgSubtype;
    si32 msgLen; // msg len, compressed len if preCompressed
    si32 msgPreCompressed; // flag to indicate already compressed
    si32 msgUncompressedLen; // length before compression
    si32 msgId;
    ti32 msgTime;
  } msgInfo_t;
  
  // constructor

  DsFmqMsg();
  
  // destructor

  virtual ~DsFmqMsg();
  
  // clear all - overriding parent class

  virtual void clearAll();

  // assemble messages
 
  void assembleRequestInit(const string &url,
			   const string &procName,
			   bool debug,
			   Fmq::openMode mode, 
			   Fmq::openPosition position, 
			   bool compress,
			   size_t numSlots, 
			   size_t bufSize);
  
  void assembleSetCompressionMethod(ta_compression_method_t method);

  void assembleSetBlockingWrite();

  void assembleSetSingleWriter();
  
  void assembleSetRegisterWithDmap(bool doReg,
				   int regIntervalSecs);

  void assembleRequestSeek(Fmq::seekPosition pos);
  
  void assembleRequestSeekToId(int id);
  
  void assembleRequestRead(int requestedType, int msecsSleep);

  // Add read data for a single message in
  // preparation for calling assembleReadReply().
  // You need to call clearAll() before the first add.

  void addReadData(const Fmq &fmq);
  
  // Assemble reply after successful read.
  // Assumes clearAll() was called, and data was
  // added with a series of calls to addReadData()
  
  void assembleReadReply(int msgType, const Fmq &fmq);

  // Add write data for a single message, in preparation
  // for calling assembleRequestWrite.
  // You need to call clearAll() before the first add.

  void addWriteData(int msgType, int msgSubtype, 
		    const void *msg, int msgLen,
		    bool compress,
		    ta_compression_method_t cmethod);
  
  // assemble request write message
  // Assumes clearAll() was called, and data was
  // added with a series of calls to addwriteData()
  
  void assembleRequestWrite();

  // assemble request to write single message

  void assembleRequestWrite(int msgType, int msgSubtype, 
			    const void *msg, int msgLen,
			    bool compress,
			    ta_compression_method_t cmethod);
  
  void assembleRequestClose();
  
  void assembleSuccessReply(int msgType);
  
  void assembleErrorReply(int msgType,
			  const string &errorStr);
  
  // disassemble messages
  
  int disassemble(const void *msg, const int msgLen, Fmq &fmq);

  ///////////////////////////////////////////////////////
  // get methods

  inline const string getUrlStr() const { return getFirstURLStr(); }
  inline const DsURL &getUrl() const { return *getFirstURL(); }
  inline const string getErrStr() const { return getFirstErrString(); }
  
  inline const vector<void *> getMsgData() const { return _msgData; }
  inline const vector<msgInfo_t> getMsgInfo() const { return _msgInfo; }

  inline bool initInfoSet() const { return _initInfoSet; }
  inline const initInfo_t &getInitInfo() const { return _initInfo; }

  // interpreting message types
  
  static string msgType2Str(int msgType);
  static string msgPart2Str(int msgPart);

  // printing

  static void printInitInfo(ostream &out, const char *spacer,
			    const initInfo_t &initInfo);
  static void printMsgInfo(ostream &out, const char *spacer,
			   const msgInfo_t &info);
protected:
 
  // data

  bool _initInfoSet;
  initInfo_t _initInfo;

  vector<void *> _msgData;
  vector<msgInfo_t> _msgInfo;

  // methods
 
  void BEfromInfo(msgInfo_t *info);
  void BEtoInfo(msgInfo_t *info);
 
  void clearInitInfo();
  void BEfromInitInfo();
  void BEtoInitInfo();
 
};

#endif
