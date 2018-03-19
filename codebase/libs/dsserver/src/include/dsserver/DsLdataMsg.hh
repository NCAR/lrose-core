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
// DsLdataMsg.hh
//
// DsLdataMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////
//
// The DsLdataMsg object provides the message protocol for
// the DsLdataServer service.
//
///////////////////////////////////////////////////////////////

#ifndef DsLdataMsg_HH
#define DsLdataMsg_HH

#include <deque>
#include <iostream>
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
// message composition
//
// type: DS_MESSAGE_TYPE_LDATA
//
// subtype: set to 0 except for REPLY, in which case the
//          subtype echoes the mode of the request message
// 
// mode: set to one of the following
//
//   DS_LDATA_OPEN
//   DS_LDATA_SET_DISPLACED_DIR_PATH
//   DS_LDATA_SET_LDATA_FILE_NAME
//   DS_LDATA_SET_USE_XML
//   DS_LDATA_SET_USE_ASCII
//   DS_LDATA_SET_SAVE_LATEST_READ_INFO
//   DS_LDATA_SET_USE_FMQ
//   DS_LDATA_SET_FMQ_NSLOTS
//   DS_LDATA_SET_READ_FMQ_FROM_START
//   DS_LDATA_READ
//   DS_LDATA_WRITE
//   DS_LDATA_REPLY (reply for all modes)
//   DS_LDATA_CLOSE
//
// parts list:
//
//   OPEN, SET, READ and WRITE messages will include the
//     DS_LDATA_ARGS_XML_PART
//
//   OPEN and REPLY will include the
//     DS_URL part
//
//   READ and WRITE will include the
//     DS_LDATA_INFO_XML_PART
//
//   REPLY subtype will be set to the type of the request
//
//   If an error occurs, REPLY will include the
//     DS_ERR_STRING part

///////////////////////////////////////////////////////////////////
// class definition

class DsLdataMsg : public DsServerMsg

{

public:
  
  // message type definition

  typedef enum {
    DS_MESSAGE_TYPE_LDATA = 717000
  } type_enum_t;

  // message mode definitions

  typedef enum {
    DS_LDATA_OPEN = 717100,
    DS_LDATA_SET_DISPLACED_DIR_PATH = 717110,
    DS_LDATA_SET_LDATA_FILE_NAME = 717120,
    DS_LDATA_SET_USE_XML = 717130,
    DS_LDATA_SET_USE_ASCII = 717140,
    DS_LDATA_SET_SAVE_LATEST_READ_INFO = 717150,
    DS_LDATA_SET_USE_FMQ = 717160,
    DS_LDATA_SET_FMQ_NSLOTS = 717170,
    DS_LDATA_SET_READ_FMQ_FROM_START = 717180,
    DS_LDATA_READ = 717200,
    DS_LDATA_WRITE = 717220,
    DS_LDATA_CLOSE = 717230,
    DS_LDATA_REPLY = 717240
  } mode_enum_t;

  // part definitions
  
  typedef enum {
    DS_LDATA_ARGS_XML = 717500,
    DS_LDATA_INFO_XML = 717510
  } part_enum_t;

  // constructor
  
  DsLdataMsg();
  
  // destructor
  
  virtual ~DsLdataMsg();

  // clear the members

  void clear();

  // set methods
  
  int setMode(int mode);

  void setUrlStr(const string &urlStr) { _urlStr = urlStr; }
  void setDisplacedDirPath(const string &val) { _displacedDirPath = val; }
  void setLdataFileName(const string &val) { _ldataFileName = val; }
  
  void setUseXml(bool val) { _useXml = val; }
  void setUseAscii(bool val) { _useAscii = val; }
  void setSaveLatestReadInfo(bool val) { _saveLatestReadInfo = val; }
  void setLatestReadInfoLabel(const string &val) {
    _latestReadInfoLabel = val;
  }

  void setUseFmq(bool val) { _useFmq = val; }
  void setFmqNSlots(int val) { _fmqNSlots = val; }
  void setReadFmqFromStart(bool val) { _readFmqFromStart = val; }

  void setMaxValidAge(int val) { _maxValidAge = val; }
  void setReadForced(bool val) { _readForced = val; }

  void setWriteFmqOnly(bool val) { _writeFmqOnly = val; }

  void setLdataXml(const string &val) { _ldataXml = val; }

  void setErrorOccurred(bool val) { _errorOccurred = val; }
  void setErrStr(const string &val) { _errStr = val; }
  
  // assemble message
  
  virtual void *assemble();
  
  // disassemble message
  
  virtual int disassemble(const void *inMsg, int msgLen);
  
  // print
  
  virtual void print(ostream &out, const char *spacer = "") const;
  
  // get methods
  
  const string &getUrlStr() const { return _urlStr; }
  const string &getDisplacedDirPath() const { return _displacedDirPath; }
  const string &getLdataFileName() const { return _ldataFileName; }

  bool getUseXml() const { return _useXml; }
  bool getUseAscii() const { return _useAscii; }
  bool getSaveLatestReadInfo() const { return _saveLatestReadInfo; }
  const string &getLatestReadInfoLabel() const {
    return _latestReadInfoLabel;
  }

  bool getUseFmq() const { return _useFmq; }
  int getFmqNSlots() const { return _fmqNSlots; }
  bool getReadFmqFromStart() const { return _readFmqFromStart; }

  int getMaxValidAge() const { return _maxValidAge; }
  bool getReadForced() const { return _readForced; }

  bool getWriteFmqOnly() const { return _writeFmqOnly; }

  const string &getLdataXml() const { return _ldataXml; }

  bool getErrorOccurred() const { return _errorOccurred; }
  const string &getErrorStr() const { return _errStr; }

protected:

private:

  // url

  string _urlStr;

  // args for set

  string _displacedDirPath; // Directory of displaced data set.
  string _ldataFileName; // ldata_info file name
  
  bool _useXml; // use XML file
  bool _useAscii; // use ASCII file
  bool _saveLatestReadInfo; // save latest read info, for restart
  string _latestReadInfoLabel;

  bool _useFmq; // use an FMQ
  int _fmqNSlots; // how many slots in the FMQ?
  bool _readFmqFromStart; // start reading from start of FMQ

  // read args
  
  int _maxValidAge;
  bool _readForced;

  // write args
  
  bool _writeFmqOnly;

  // latest data info as XML

  string _ldataXml;

  // args as XML

  string _argsXml;

  // errors

  bool _errorOccurred;
  string _errStr;

  // methods
  
  void _addOpen();
  void _addSetDisplacedDirPath();
  void _addSetLdataFileName();
  void _addSetUseXml();
  void _addSetUseAscii();
  void _addSetSaveLatestReadInfo();
  void _addSetUseFmq();
  void _addSetFmqNSlots();
  void _addSetReadFmqFromStart();
  void _addRead();
  void _addWrite();
  void _addReply();
  void _addClose();
  
};

#endif
