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

/*******************************************************************
 * dsserver/DsLdataServerMsg.hh
 *
 * DsMessage class for DsLdataServer
 ******************************************************************/

#ifndef DsLdataServerMsg_HH
#define DsLdataServerMsg_HH

#include <deque>
#include <iostream>
#include <dsserver/DsLdataInfo.hh>
#include <dsserver/DsServerMsg.hh>
#include <toolsa/MemBuf.hh>
using namespace std;

////////////////////////////////////////////////////////////////////a
// Messages to and from the DsLdataServer make use the generic
// ds_message format - see ds_message.h
//
// Messages have a header containing a type and subtype,
// and optinally a body comprising a number of parts.
// The header contains the number of parts, which is 0 for
// a message with no body.
//

///////////////////////////////////////////////////////////////////
// message type definitions
//

////////////////////////////////////////////////////////////////////
// read()
//
// Perform a read. Reads as many info entries as are available
// and returns them all in the return message.
//
// Type: DS_MESSAGE_TYPE_LDATA_SERVER
// SubType: DS_LDATA_SERVER_READ
//
// The message body has the following parts:
//   DS_LDATA_SERVER_URL_PART
//   DS_LDATA_SERVER_DEBUG_PART
//   DS_LDATA_SERVER_FILENAME_PART
//
// A DS_LDATA_SERVER_RETURN message is expected as a reply.
// Mode: set to DS_LDATA_SERVER_READ
// Error: set to -1 if an error occurred, which usually means
//   that we cannot create the directory required.
//   DS_ERRORSTR_PART contains error message.

////////////////////////////////////////////////////////////////////
// readForced()
//
// Perform a read. If no info to read, forces a read on the latest
// one available.
// Reads as many info entries as are available
// and returns them all in the return message.
//
// Type: DS_MESSAGE_TYPE_LDATA_SERVER
// SubType: DS_LDATA_SERVER_READ_FORCED
//
// The message body has the following parts:
//   DS_LDATA_SERVER_URL_PART
//   DS_LDATA_SERVER_DEBUG_PART
//   DS_LDATA_SERVER_FILENAME_PART
//
// A DS_LDATA_SERVER_RETURN message is expected as a reply.
// Mode: set to DS_LDATA_SERVER_READ_FORCED
// Error: set to -1 if an error occurred, which usually means
//   that we cannot create the directory required.
//   DS_ERRORSTR_PART contains error message.

////////////////////////////////////////////////////////////////////
// write()
//
// Perform a write of the latest data info.
//
// Type: DS_MESSAGE_TYPE_LDATA_SERVER
// SubType: DS_LDATA_SERVER_WRITE
//
// The message body has the following parts:
//   DS_LDATA_SERVER_URL_PART
//   DS_LDATA_SERVER_DEBUG_PART
//   DS_LDATA_SERVER_FILENAME_PART
//   DS_LDATA_SERVER_LATEST_TIME_PART
//   DS_LDATA_SERVER_INFO_BUFFER_PART
//
// A DS_LDATA_SERVER_RETURN message is expected as a reply.
// Mode: set to DS_LDATA_SERVER_WRITE
// Error: set to -1 if an error occurred, which usually means
//   that we cannot create the directory required.
//   DS_ERRORSTR_PART contains error message.

////////////////////////////////////////////////////////////////////
// writeFmq()
//
// Perform a write of the latest data info only to the FMQ.
//
// Type: DS_MESSAGE_TYPE_LDATA_SERVER
// SubType: DS_LDATA_SERVER_WRITE_FMQ
//
// The message body has the following parts:
//   DS_LDATA_SERVER_URL_PART
//   DS_LDATA_SERVER_DEBUG_PART
//   DS_LDATA_SERVER_FILENAME_PART
//   DS_LDATA_SERVER_LATEST_TIME_PART
//   DS_LDATA_SERVER_INFO_BUFFER_PART
//
// A DS_LDATA_SERVER_RETURN message is expected as a reply.
// Mode: set to DS_LDATA_SERVER_WRITE_FMQ
// Error: set to -1 if an error occurred, which usually means
//   that we cannot create the directory required.
//   DS_ERRORSTR_PART contains error message.

//////////
// Return
//
// Type: DS_MESSAGE_TYPE_LDATA_SERVER
// SubType: DS_LDATA_SERVER_RETURN
// Mode: set to subType of original message
// Error: 0 on success, -1 on error.
//
// If no error occurred, then:
// On read, message contains number of
//   DS_LDATA_SERVER_INFO_BUFFER_PARTs.
// On write, message has no body.
//
// If an error occurred, the return message will contain a
// DS_ERR_STRING containing the error message string.

///////////////////////////////////////////////////////////////////
// class definition

class DsLdataServerMsg : public DsServerMsg

{

public:
  
  // message type definition

  typedef enum {
    DS_MESSAGE_TYPE_LDATA_SERVER = 717000
  } type_enum_t;

  // message sub-type definitions

  typedef enum {
    DS_LDATA_SERVER_READ = 717100,
    DS_LDATA_SERVER_READ_FORCED = 717104,
    DS_LDATA_SERVER_WRITE = 717106,
    DS_LDATA_SERVER_WRITE_FMQ = 717108,
    DS_LDATA_SERVER_RETURN = 717110
  } subtype_enum_t;

  // part definitions
  
  typedef enum {
    DS_LDATA_SERVER_URL_PART = 717200,
    DS_LDATA_SERVER_DEBUG_PART = 717202,
    DS_LDATA_SERVER_FILENAME_PART = 717204,
    DS_LDATA_SERVER_LATEST_TIME_PART = 717206,
    DS_LDATA_SERVER_MAX_VALID_AGE_PART = 717208,
    DS_LDATA_SERVER_INFO_BUFFER_PART = 717210,
    DS_LDATA_SERVER_IDENT_STR_PART = 717212,
    DS_LDATA_SERVER_READ_FMQ_FROM_START_PART = 717214
  } part_enum_t;

  // constructor

  DsLdataServerMsg();

  // destructor
  
  virtual ~DsLdataServerMsg();
  
  // assemble messages
  
  void *assembleRead(const DsLdataInfo &ldata,
		     int max_valid_age = -1);
  void *assembleReadForced(const DsLdataInfo &ldata,
			   int max_valid_age = -1);
  void *assembleWrite(const DsLdataInfo &ldata,
		      time_t latest_time);
  void *assembleWriteFmq(const DsLdataInfo &ldata,
			 time_t latest_time);
  void *assembleReturn(bool errorOccurred = false,
					 const string &errorStr = "");
  void *assembleInfoReturn(const vector<MemBuf> &info);
  
  // overload disassemble
  
  int disassemble(void *inMsg, const int msgLen);

  // print
  
  void print(ostream &out);

  // access to message data

  bool isDebug() { return _debug; }
  bool errorOccurred() { return _errorOccurred; }
  const string &getUrlStr() const { return _urlStr; }
  const string &getIdentStr() const { return _identStr; }
  const string &getFileName() const { return _fileName; }
  const string &getErrorStr() const { return _errorStr; }
  time_t getLatestTime() const { return _latestTime; }
  int getMaxValidAge() const { return _maxValidAge; }

  // message types

  bool isRead() { return _isRead; }
  bool isReadForced() { return _isReadForced; }
  bool isWrite() { return _isWrite; }
  bool isWriteFmq()  { return _isWriteFmq; }
  bool isReturn() { return _isReturn; }

  // ldata info

  int getLdataQSize() { return _ldataQ.size(); }
  void clearLdataQ();
  void resetLdataQ();

  // get next LdataInfo - returns NULL if none left in Queue
  const LdataInfo *getNextLdataInfo() const;
  
  // error str

protected:

private:

  bool _debug;
  bool _errorOccurred;
  bool _readFmqFromStart;

  bool _isRead;
  bool _isReadForced;
  bool _isWrite;
  bool _isWriteFmq;
  bool _isReturn;

  string _errorStr;
  string _urlStr;
  string _fileName;
  string _identStr;

  time_t _latestTime;
  int _maxValidAge;

  deque<LdataInfo *> _ldataQ;
  mutable size_t _ldataPos;

};

#endif
