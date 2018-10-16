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
// DsLdataMsg.cc
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

#include <dsserver/DsLdataMsg.hh>
#include <didss/LdataInfo.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsMsgPart.hh>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

DsLdataMsg::DsLdataMsg() :
  DsServerMsg()

{
  clear();
}

////////////////////////////////////////////////////////////
// destructor

DsLdataMsg::~DsLdataMsg()

{

}

////////////////////////////////////////////////////////////
// clear data members

void DsLdataMsg::clear()

{

  DsServerMsg::clearAll();
  _urlStr.clear();
  _displacedDirPath.clear();
  _ldataFileName = LDATA_INFO_FILE_NAME;
  _useXml = true;
  _useAscii = true;
  _saveLatestReadInfo = false;
  _latestReadInfoLabel.clear();
  _useFmq = true;
  _fmqNSlots = LDATA_NSLOTS_DEFAULT;
  _readFmqFromStart = false;
  _maxValidAge = 3600;
  _readForced = false;
  _writeFmqOnly = false;
  _ldataXml.clear();
  _argsXml.clear();
  _errorOccurred = false;
  _errStr.clear();

}

////////////////////////////////////////////////////////////
// set mode

int DsLdataMsg::setMode(int val)

{

  clear();

  switch (val) {
    case DS_LDATA_OPEN:
    case DS_LDATA_SET_DISPLACED_DIR_PATH:
    case DS_LDATA_SET_LDATA_FILE_NAME:
    case DS_LDATA_SET_USE_XML:
    case DS_LDATA_SET_USE_ASCII:
    case DS_LDATA_SET_SAVE_LATEST_READ_INFO:
    case DS_LDATA_SET_USE_FMQ:
    case DS_LDATA_SET_FMQ_NSLOTS:
    case DS_LDATA_SET_READ_FMQ_FROM_START:
    case DS_LDATA_READ:
    case DS_LDATA_WRITE:
    case DS_LDATA_REPLY:
    case DS_LDATA_CLOSE:
      _mode = val;
      return 0;
      break;
    default:
      _errStr = "ERROR - DsLdataMsg::setMode\n";
      TaStr::AddInt(_errStr, "  Unknown mode: ", val);
      TaStr::AddStr(_errStr, "  Setting to DS_LDATA_CLOSE");
      _mode = DS_LDATA_CLOSE;
      return -1;
  }

}

//////////////////////////////////////////////////
// assemble message
  
void *DsLdataMsg::assemble()
  
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_LDATA, 0, _mode);

  // category is end-series if this is a reply to a
  // close request

  setCategory(Generic);
  if (_mode == DS_LDATA_OPEN) {
    setCategory(StartGet);
  } else if (_mode == DS_LDATA_REPLY &&
             _subType == DS_LDATA_CLOSE) {
    setCategory(EndSeries);
  }

  // clear message parts
  
  clearParts();
  
  // add url if appropriate

  if (_urlStr.size() > 0) {
    addURL(_urlStr);
  }

  // mode-specific args

  _argsXml.clear();

  switch (_mode) {
    case DS_LDATA_OPEN:
      _addOpen();
      break;
    case DS_LDATA_SET_DISPLACED_DIR_PATH:
      _addSetDisplacedDirPath();
      break;
    case DS_LDATA_SET_LDATA_FILE_NAME:
      _addSetLdataFileName();
      break;
    case DS_LDATA_SET_USE_XML:
      _addSetUseXml();
      break;
    case DS_LDATA_SET_USE_ASCII:
      _addSetUseAscii();
      break;
    case DS_LDATA_SET_SAVE_LATEST_READ_INFO:
      _addSetSaveLatestReadInfo();
      break;
    case DS_LDATA_SET_USE_FMQ:
      _addSetUseFmq();
      break;
    case DS_LDATA_SET_FMQ_NSLOTS:
      _addSetFmqNSlots();
      break;
    case DS_LDATA_SET_READ_FMQ_FROM_START:
      _addSetReadFmqFromStart();
      break;
    case DS_LDATA_READ:
      _addRead();
      break;
    case DS_LDATA_WRITE:
      _addWrite();
      break;
    case DS_LDATA_REPLY:
      _addReply();
      break;
    case DS_LDATA_CLOSE:
      _addClose();
      break;
    default: {}
  }

  // add ldata info if appropriate

  if (_ldataXml.size() > 0) {
    addPart(DS_LDATA_INFO_XML, _ldataXml.size() + 1, _ldataXml.c_str());
  }

  // add error string if appropriate

  if (_errorOccurred) {
    addErrString(_errStr);
  }

  // base class assemble
  
  return (DsMessage::assemble());

}

/////////////////////////////////////////////////
// add modes

void DsLdataMsg::_addOpen()

{

  TaXml::attribute attr("mode", "open");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);

  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);

  _argsXml += TaXml::writeString("displacedDirPath",
                                 1, _displacedDirPath);
  _argsXml += TaXml::writeString("ldataFileName", 1, _ldataFileName);
  _argsXml += TaXml::writeBoolean("useXml", 1, _useXml);
  _argsXml += TaXml::writeBoolean("useAscii", 1, _useAscii);
  _argsXml += TaXml::writeBoolean("saveLatestReadInfo",
                                  1, _saveLatestReadInfo);
  _argsXml += TaXml::writeString("latestReadInfoLabel",
                                 1, _latestReadInfoLabel);
  _argsXml += TaXml::writeBoolean("useFmq", 1, _useFmq);
  _argsXml += TaXml::writeInt("fmqNSlots", 1, _fmqNSlots);
  _argsXml += TaXml::writeBoolean("readFmqFromStart",
                                  1, _readFmqFromStart);

  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);
  
  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addSetSaveLatestReadInfo()

{

  TaXml::attribute attr("mode", "");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addSetDisplacedDirPath()

{

  TaXml::attribute attr("mode", "");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addSetLdataFileName()

{

  TaXml::attribute attr("mode", "setLdataFileName");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);

  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeString("ldataFileName", 1, _ldataFileName);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addSetUseXml()

{

  TaXml::attribute attr("mode", "setUseXml");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeBoolean("useXml", 1, _useXml);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addSetUseAscii()

{

  TaXml::attribute attr("mode", "setUseAscii");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeBoolean("useAscii", 1, _useAscii);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addSetUseFmq()

{

  TaXml::attribute attr("mode", "setUseFmq");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeBoolean("useFmq", 1, _useFmq);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addSetFmqNSlots()

{

  TaXml::attribute attr("mode", "setFmqNSlots");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeInt("fmqNSlots", 1, _fmqNSlots);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addSetReadFmqFromStart()

{

  TaXml::attribute attr("mode", "setReadFmqFromStart");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);

  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeBoolean("readFmqFromStart", 1, _readFmqFromStart);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addRead()

{

  TaXml::attribute attr("mode", "read");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeInt("maxValidAge", 1, _maxValidAge);
  _argsXml += TaXml::writeBoolean("readForced", 1, _readForced);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addWrite()

{

  TaXml::attribute attr("mode", "write");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeBoolean("writeFmqOnly", 1, _writeFmqOnly);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addReply()

{

  TaXml::attribute attr("mode", "reply");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

void DsLdataMsg::_addClose()

{

  TaXml::attribute attr("mode", "close");
  vector<TaXml::attribute> attrs;
  attrs.push_back(attr);
  
  _argsXml += TaXml::writeStartTag("DsLdataMsg", 0, attrs, true);
  _argsXml += TaXml::writeEndTag("DsLdataMsg", 0);

  addPart(DS_LDATA_ARGS_XML, _argsXml.size() + 1, _argsXml.c_str());

}

/////////////////////////////////////
// override the disassemble function
//

int DsLdataMsg::disassemble(const void *inMsg, int msgLen)

{

  clear();

  // peek at the header to make sure we're looking at the
  // right type of message
  
  if (decodeHeader(inMsg, msgLen)) {
    _errStr = "ERROR - DsLdataMsg::disassemble\n";
    TaStr::AddStr(_errStr, "  ", DateTime::str());
    TaStr::AddStr(_errStr, "  Bad message header\n");
    TaStr::AddInt(_errStr, "  Message len: ", msgLen);
    return -1;
  }
  
  if (_type != DS_MESSAGE_TYPE_LDATA) {
    _errStr = "ERROR - DsLdataMsg::disassemble\n";
    TaStr::AddStr(_errStr, "  ", DateTime::str());
    TaStr::AddInt(_errStr, "  Unknown message type: ", _type);
    TaStr::AddInt(_errStr, "  Message len: ", msgLen);
    return -1;
  }

  // disassemble the message using the base-class routine
  
  if (DsMessage::disassemble(inMsg, msgLen)) {
    _errStr = "ERROR - DsLdataMsg::disassemble\n";
    TaStr::AddStr(_errStr, "  ", DateTime::str());
    TaStr::AddStr(_errStr, "ERROR in DsMessage::disassemble()");
    return -1;
  }

  // url

  _urlStr = getFirstURLStr();

  // ldata info as xml

  if (partExists(DS_LDATA_INFO_XML)) {
    _ldataXml = (char *)
      getPartByType(DS_LDATA_INFO_XML)->getBuf();
  }

  // error

  if (partExists(DS_ERR_STRING)) {
    _errorOccurred = true;
    _errStr = getFirstErrString();
  }

  // args as XML

  if (partExists(DS_LDATA_ARGS_XML)) {
    _argsXml = (char *) getPartByType(DS_LDATA_ARGS_XML)->getBuf();
    if (TaXml::readString(_argsXml, "displacedDirPath", _displacedDirPath)) {
      _displacedDirPath.clear();
    }
    if (TaXml::readString(_argsXml, "ldataFileName", _ldataFileName)) {
      _ldataFileName = LDATA_INFO_FILE_NAME;
    }
    if (TaXml::readBoolean(_argsXml, "useXml", _useXml)) {
      _useXml = true;
    }
    if (TaXml::readBoolean(_argsXml, "useAscii", _useAscii)) {
      _useAscii = true;
    }
    if (TaXml::readBoolean(_argsXml, "saveLatestReadInfo", _saveLatestReadInfo)) {
      _saveLatestReadInfo = false;
    }
    if (TaXml::readString(_argsXml, "latestReadInfoLabel", _latestReadInfoLabel)) {
      _latestReadInfoLabel.clear();
    }
    if (TaXml::readBoolean(_argsXml, "useFmq", _useFmq)) {
      _useFmq = true;
    }
    if (TaXml::readInt(_argsXml, "fmqNSlots", _fmqNSlots)) {
      _fmqNSlots = LDATA_NSLOTS_DEFAULT;
    }
    if (TaXml::readBoolean(_argsXml, "readFmqFromStart", _readFmqFromStart)) {
      _readFmqFromStart = false;
    }
  }

  return 0;

}

////////////////
// print message
//

void DsLdataMsg::print(ostream &out, const char *spacer) const

{

  if (_urlStr.size() > 0) {
    out << spacer << "  urlStr: " << _urlStr << endl;
  }

  switch (_mode) {
    case DS_LDATA_OPEN:
      out << spacer << "Message mode: DS_LDATA_OPEN" << endl;
      out << spacer << "  displacedDirPath: " << _displacedDirPath << endl;
      out << spacer << "  fileName: " << _ldataFileName << endl;
      out << spacer << "  useXml: " << (_useXml?"y":"n") << endl;
      out << spacer << "  useAscii: " << (_useAscii?"y":"n") << endl;
      out << spacer << "  saveLatestReadInfo: " << (_saveLatestReadInfo?"y":"n") << endl;
      out << spacer << "  useFmq: " << (_useFmq?"y":"n") << endl;
      out << spacer << "  fmqNSlots: " << _fmqNSlots << endl;
      out << spacer << "  readFmqFromStart: " << (_readFmqFromStart?"y":"n") << endl;
      break;
    case DS_LDATA_SET_DISPLACED_DIR_PATH:
      out << spacer << "Message mode: DS_LDATA_SET_DISPLACED_DIR_PATH" << endl;
      out << spacer << "  displacedDirPath: " << _displacedDirPath << endl;
      break;
    case DS_LDATA_SET_LDATA_FILE_NAME:
      out << spacer << "Message mode: DS_LDATA_SET_LDATA_FILE_NAME" << endl;
      out << spacer << "  fileName: " << _ldataFileName << endl;
      break;
    case DS_LDATA_SET_USE_XML:
      out << spacer << "Message mode: DS_LDATA_SET_USE_XML" << endl;
      out << spacer << "  useXml: " << (_useXml?"y":"n") << endl;
      break;
    case DS_LDATA_SET_USE_ASCII:
      out << spacer << "Message mode: DS_LDATA_SET_USE_ASCII" << endl;
      out << spacer << "  useAscii: " << (_useAscii?"y":"n") << endl;
      break;
    case DS_LDATA_SET_SAVE_LATEST_READ_INFO:
      out << spacer << "Message mode: DS_LDATA_SET_SAVE_LATEST_READ_INFO" << endl;
      out << spacer << "  saveLatestReadInfo: " << (_saveLatestReadInfo?"y":"n") << endl;
      break;
    case DS_LDATA_SET_USE_FMQ:
      out << spacer << "Message mode: DS_LDATA_SET_USE_FMQ" << endl;
      out << spacer << "  useFmq: " << (_useFmq?"y":"n") << endl;
      break;
    case DS_LDATA_SET_FMQ_NSLOTS:
      out << spacer << "Message mode: DS_LDATA_SET_FMQ_NSLOTS" << endl;
      out << spacer << "  fmqNSlots: " << _fmqNSlots << endl;
      break;
    case DS_LDATA_SET_READ_FMQ_FROM_START:
      out << spacer << "Message mode: DS_LDATA_SET_READ_FMQ_FROM_START" << endl;
      out << spacer << "  readFmqFromStart: " << (_readFmqFromStart?"y":"n") << endl;
      break;
    case DS_LDATA_READ:
      out << spacer << "Message mode: DS_LDATA_READ" << endl;
      out << spacer << "  readForced: " << (_readForced?"y":"n") << endl;
      out << spacer << "  maxValidAge: " << _maxValidAge << endl;
      break;
    case DS_LDATA_WRITE:
      out << spacer << "Message mode: DS_LDATA_WRITE" << endl;
      out << spacer << "  writeFmqOnly: " << (_writeFmqOnly?"y":"n") << endl;
      break;
    case DS_LDATA_REPLY:
      out << spacer << "Message mode: DS_LDATA_REPLY" << endl;
      break;
    case DS_LDATA_CLOSE:
      out << spacer << "Message mode: DS_LDATA_CLOSE" << endl;
      break;
    default: {}
  }

  if (_argsXml.size() > 0) {
    out << spacer << "  Args XML:" << endl;
    out << spacer << _argsXml << endl;
  }
  
  if (_ldataXml.size() > 0) {
    out << spacer << "  Ldata XML:" << endl;
    out << spacer << _ldataXml << endl;
  }
  
  if (_errorOccurred) {
    out << spacer << "  Error occurred" << endl;
    out << spacer << "  Error str: " << _errStr << endl;
  }

  DsServerMsg::print(out, "");

}

