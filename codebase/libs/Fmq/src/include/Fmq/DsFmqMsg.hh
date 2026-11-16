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
#include <cstddef>
#include <cstdint>

class DsMsgPart;
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

  // Structs for initializing an fmq via the server, and for describing
  // each message carried in a read reply or write request.
  //
  // Each has two wire layouts: the legacy 32-bit one, which every
  // DsFmqServer and client from before the 64-bit FMQ understands, and a
  // 64-bit one that carries buffer and message sizes above 2 GB. A sender
  // emits the legacy layout whenever the values fit in it and the 64-bit
  // layout only when they do not, the same way DsMessage chooses 32- or
  // 64-bit part headers by size. A receiver tells the layouts apart by
  // part length, so the two sizes must differ (checked below).
  //
  // In memory the 64-bit layout is always used (initInfo_t, msgInfo_t).

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
  } initInfo_32_t;

  static const int32_t INIT_INFO_64_NUM_SI32 = 8;
  static const int32_t INIT_INFO_64_NUM_SI64 = 5;

  typedef struct {
    si32 debug;
    si32 compress;
    si32 openMode;
    si32 openPosition;
    si32 numSlots;
    si32 spare_si32[3];
    si64 bufSize;
    si64 spare_si64[4];
    char procName[PROC_NAME_LEN];
  } initInfo_64_t;

  typedef struct {
    si32 msgType;
    si32 msgSubtype;
    si32 msgLen; // msg len, compressed len if preCompressed
    si32 msgPreCompressed; // flag to indicate already compressed
    si32 msgUncompressedLen; // length before compression
    si32 msgId;
    ti32 msgTime;
  } msgInfo_32_t;

  static const int32_t MSG_INFO_64_NUM_SI32 = 8;
  static const int32_t MSG_INFO_64_NUM_SI64 = 4;

  typedef struct {
    si32 msgType;
    si32 msgSubtype;
    si32 msgPreCompressed; // flag to indicate already compressed
    si32 msgId;
    si32 spare_si32[4];
    si64 msgTime;
    si64 msgLen; // msg len, compressed len if preCompressed
    si64 msgUncompressedLen; // length before compression
    si64 spare_si64[1];
  } msgInfo_64_t;

  static_assert(offsetof(initInfo_64_t, bufSize) ==
                INIT_INFO_64_NUM_SI32 * sizeof(si32),
                "Unexpected layout of initInfo_64_t");
  static_assert(offsetof(initInfo_64_t, procName) ==
                offsetof(initInfo_64_t, bufSize) + INIT_INFO_64_NUM_SI64 * sizeof(si64),
                "Unexpected layout of initInfo_64_t");
  static_assert(offsetof(msgInfo_64_t, msgTime) ==
                MSG_INFO_64_NUM_SI32 * sizeof(si32),
                "Unexpected layout of msgInfo_64_t");
  static_assert(sizeof(msgInfo_64_t) ==
                offsetof(msgInfo_64_t, msgTime) + MSG_INFO_64_NUM_SI64 * sizeof(si64),
                "Unexpected layout of msgInfo_64_t");
  static_assert(sizeof(initInfo_32_t) != sizeof(initInfo_64_t),
                "init info layouts are told apart by part length");
  static_assert(sizeof(msgInfo_32_t) != sizeof(msgInfo_64_t),
                "msg info layouts are told apart by part length");

  // in-memory form

  typedef initInfo_64_t initInfo_t;
  typedef msgInfo_64_t msgInfo_t;

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
			   int32_t numSlots,
			   int64_t bufSize);
  
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

  void addWriteData(int32_t msgType, int32_t msgSubtype,
		    const void *msg, int64_t msgLen,
		    bool compress,
		    ta_compression_method_t cmethod);
  
  // assemble request write message
  // Assumes clearAll() was called, and data was
  // added with a series of calls to addwriteData()
  
  void assembleRequestWrite();

  // assemble request to write single message

  void assembleRequestWrite(int32_t msgType, int32_t msgSubtype,
			    const void *msg, int64_t msgLen,
			    bool compress,
			    ta_compression_method_t cmethod);
  
  void assembleRequestClose();
  
  void assembleSuccessReply(int32_t msgType);
  
  void assembleErrorReply(int32_t msgType,
			  const string &errorStr);
  
  // disassemble messages
  
  int disassemble(const void *msg, int64_t msgLen, Fmq &fmq);

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
  
  static string msgType2Str(int32_t msgType);
  static string msgPart2Str(int32_t msgPart);

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

  void clearInitInfo();

  // wire format: send a part in the smallest layout that holds its
  // values, decode either layout on receipt

  static bool fitsInSi32(int64_t value);
  static bool initInfoFitsLegacyLayout(const initInfo_t &info);
  static bool msgInfoFitsLegacyLayout(const msgInfo_t &info);
  static void initInfoFromLegacy(const initInfo_32_t &legacy, initInfo_t &info);
  static void initInfoToLegacy(const initInfo_t &info, initInfo_32_t &legacy);
  static void msgInfoFromLegacy(const msgInfo_32_t &legacy, msgInfo_t &info);
  static void msgInfoToLegacy(const msgInfo_t &info, msgInfo_32_t &legacy);

  void addInitInfoPart(const initInfo_t &info);
  void addMsgInfoPart(const msgInfo_t &info);
  int loadInitInfoPart(const DsMsgPart &part, Fmq &fmq);
  int loadMsgInfoPart(const DsMsgPart &part, msgInfo_t &info, Fmq &fmq);

  // byte swapping

  static void BEfromInitInfo_32(initInfo_32_t *info);
  static void BEtoInitInfo_32(initInfo_32_t *info);
  static void BEfromInitInfo_64(initInfo_64_t *info);
  static void BEtoInitInfo_64(initInfo_64_t *info);
  static void BEfromMsgInfo_32(msgInfo_32_t *info);
  static void BEtoMsgInfo_32(msgInfo_32_t *info);
  static void BEfromMsgInfo_64(msgInfo_64_t *info);
  static void BEtoMsgInfo_64(msgInfo_64_t *info);

};

#endif
