/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// DsLdataServerMsg.cc
//
// DsLdataServerMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2002
//
///////////////////////////////////////////////////////////////
//
// The DsLdataServerMsg object provides the message protocol for
// the DsLdataServer service.
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <dsserver/DsLdataServerMsg.hh>
#include <didss/DsMsgPart.hh>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

DsLdataServerMsg::DsLdataServerMsg() :
  DsServerMsg()

{
}

////////////////////////////////////////////////////////////
// destructor

DsLdataServerMsg::~DsLdataServerMsg()

{
  clearLdataQ();
}

//////////////////////////////////////////////////
// assemble a read message
  
void *DsLdataServerMsg::assembleRead(const DsLdataInfo &ldata,
				     int max_valid_age /* = -1*/ )

{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_LDATA_SERVER,
	     DS_LDATA_SERVER_READ);
  
  // indicate that this is the start of a series of get messages
  
  setCategory(StartGet);

  // clear message parts
  
  clearParts();
  
  // add parts

  si32 maxValidAge = max_valid_age;
  BE_from_array_32(&maxValidAge, sizeof(maxValidAge));
  addPart(DS_LDATA_SERVER_MAX_VALID_AGE_PART,
	  sizeof(maxValidAge), &maxValidAge);

  const string &urlStr = ldata._url.getURLStr();
  addPart(DS_LDATA_SERVER_URL_PART,
	  urlStr.size() + 1, urlStr.c_str());

  if (ldata._debug) {
    addPart(DS_LDATA_SERVER_DEBUG_PART, 0, NULL);
  }

  if (ldata._readFmqFromStart) {
    addPart(DS_LDATA_SERVER_READ_FMQ_FROM_START_PART, 0, NULL);
  }

  addPart(DS_LDATA_SERVER_FILENAME_PART,
	  ldata._fileName.size() + 1, ldata._fileName.c_str());

  // assemble
  
  return (DsMessage::assemble());

}

//////////////////////////////////////////////////
// assemble a readForced message
  
void *DsLdataServerMsg::assembleReadForced(const DsLdataInfo &ldata,
					   int max_valid_age /* = -1*/ )

{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_LDATA_SERVER,
	     DS_LDATA_SERVER_READ_FORCED);
  
  // indicate that this is the start of a series of get messages
  
  setCategory(StartGet);

  // clear message parts
  
  clearParts();
  
  // add parts

  si32 maxValidAge = max_valid_age;
  BE_from_array_32(&maxValidAge, sizeof(maxValidAge));
  addPart(DS_LDATA_SERVER_MAX_VALID_AGE_PART,
	  sizeof(maxValidAge), &maxValidAge);

  const string &urlStr = ldata._url.getURLStr();
  addPart(DS_LDATA_SERVER_URL_PART,
	  urlStr.size() + 1, urlStr.c_str());

  if (ldata._debug) {
    addPart(DS_LDATA_SERVER_DEBUG_PART, 0, NULL);
  }

  if (ldata._readFmqFromStart) {
    addPart(DS_LDATA_SERVER_READ_FMQ_FROM_START_PART, 0, NULL);
  }

  addPart(DS_LDATA_SERVER_FILENAME_PART,
	  ldata._fileName.size() + 1, ldata._fileName.c_str());

  // assemble
  
  return (DsMessage::assemble());

}

//////////////////////////////////////////////////
// assemble a write message
  
void *DsLdataServerMsg::assembleWrite(const DsLdataInfo &ldata,
				      time_t latest_time)

{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_LDATA_SERVER,
	     DS_LDATA_SERVER_WRITE);
  
  // indicate that this is the start of a series of get messages
  
  setCategory(StartPut);

  // clear message parts
  
  clearParts();
  
  // add parts

  const string &urlStr = ldata._url.getURLStr();
  addPart(DS_LDATA_SERVER_URL_PART,
	  urlStr.size() + 1, urlStr.c_str());
  if (ldata._debug) {
    addPart(DS_LDATA_SERVER_DEBUG_PART, 0, NULL);
  }
  addPart(DS_LDATA_SERVER_FILENAME_PART,
	  ldata._fileName.size() + 1, ldata._fileName.c_str());

  ui32 ltime = latest_time;
  BE_from_array_32(&ltime, sizeof(ltime));
  addPart(DS_LDATA_SERVER_LATEST_TIME_PART, sizeof(ltime), &ltime);

  ldata.assemble();
  addPart(DS_LDATA_SERVER_INFO_BUFFER_PART,
	  ldata.getBufLen(), ldata.getBufPtr());

  // assemble
  
  return (DsMessage::assemble());

}

//////////////////////////////////////////////////
// assemble a writeFmq message
  
void *DsLdataServerMsg::assembleWriteFmq(const DsLdataInfo &ldata,
					 time_t latest_time)

{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_LDATA_SERVER,
	     DS_LDATA_SERVER_WRITE_FMQ);
  
  // indicate that this is the start of a series of get messages
  
  setCategory(StartPut);

  // clear message parts
  
  clearParts();
  
  // add parts

  const string &urlStr = ldata._url.getURLStr();
  addPart(DS_LDATA_SERVER_URL_PART,
	  urlStr.size() + 1, urlStr.c_str());
  if (ldata._debug) {
    addPart(DS_LDATA_SERVER_DEBUG_PART, 0, NULL);
  }
  addPart(DS_LDATA_SERVER_FILENAME_PART,
	  ldata._fileName.size() + 1, ldata._fileName.c_str());
  ui32 ltime = latest_time;
  BE_from_array_32(&ltime, sizeof(ltime));
  addPart(DS_LDATA_SERVER_LATEST_TIME_PART, sizeof(ltime), &ltime);

  ldata.assemble();
  addPart(DS_LDATA_SERVER_INFO_BUFFER_PART,
	  ldata.getBufLen(), ldata.getBufPtr());

  // assemble
  
  return (DsMessage::assemble());
  
}

//////////////////////////////////////////////////
// assemble a generic return message
  
void *DsLdataServerMsg::assembleReturn(bool errorOccurred /* = false*/,
				       const string &errorStr /* = ""*/ )

{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_LDATA_SERVER,
	     DS_LDATA_SERVER_RETURN,
	     _subType);

  if (errorOccurred) {
    setError(-1);
  }
  
  // indicate that this is the end of a series
  
  setCategory(EndSeries);

  // clear message parts
  
  clearParts();
  
  // add parts

  if (errorOccurred) {
    addPart(DS_ERR_STRING, errorStr.size() + 1, errorStr.c_str());
  }

  // assemble
  
  return (DsMessage::assemble());

}

//////////////////////////////////////////////////
// assemble a return with info message
  
void *DsLdataServerMsg::assembleInfoReturn(const vector<MemBuf> &info)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_LDATA_SERVER,
	     DS_LDATA_SERVER_RETURN,
	     _subType);
  
  // indicate that this is the end of a series
  
  setCategory(EndSeries);

  // clear message parts
  
  clearParts();
  
  // add parts

  for (size_t ii = 0; ii < info.size(); ii++) {
    addPart(DS_LDATA_SERVER_INFO_BUFFER_PART,
	    info[ii].getBufLen(), info[ii].getBufPtr());
  }

  // assemble
  
  return (DsMessage::assemble());

}

/////////////////////////////////////
// override the disassemble function
//

int DsLdataServerMsg::disassemble(void *inMsg, const int msgLen)

{

  char tmpStr[128];
  // initialize

  _debug = false;
  _errorOccurred = false;
  _readFmqFromStart = false;
  _identStr.erase();
  _urlStr.erase();
  _fileName.erase();
  _errorStr.erase();
  clearLdataQ();
  _latestTime = 0;
  _maxValidAge = -1;

  _isRead = false;
  _isReadForced = false;
  _isWrite = false;
  _isWriteFmq = false;
  _isReturn = false;

  // peek at the header to make sure we're looking at the
  // right type of message
  
  if (decodeHeader(inMsg, msgLen)) {
    _errorStr = "ERROR - DsLdataServerMsg::disassemble\n";
    TaStr::AddStr(_errorStr, "  ", DateTime::str());
    _errorStr += "  Bad message header\n";
    _errorStr += "  Message len: ";
    sprintf(tmpStr, "%d\n", msgLen);
    _errorStr += tmpStr;
    return (-1);
  }

  if (_type != DS_MESSAGE_TYPE_LDATA_SERVER) {
    _errorStr = "ERROR - DsLdataServerMsg::disassemble\n";
    TaStr::AddStr(_errorStr, "  ", DateTime::str());
    _errorStr += "  Unknown message type: \n";
    sprintf(tmpStr, "%d\n", _type);
    _errorStr += tmpStr;
    _errorStr += "  Message len: ";
    sprintf(tmpStr, "%d\n", msgLen);
    _errorStr += tmpStr;
    return (-1);
  }

  switch (_subType) {
  case DS_LDATA_SERVER_READ:
    _isRead = true;
    break;
  case DS_LDATA_SERVER_READ_FORCED:
    _isReadForced = true;
    break;
  case DS_LDATA_SERVER_WRITE:
    _isWrite = true;
  case DS_LDATA_SERVER_WRITE_FMQ:
    _isWriteFmq = true;
    break;
  case DS_LDATA_SERVER_RETURN:
    _isReturn = true;
    switch (_mode) {
    case DS_LDATA_SERVER_READ:
      _isRead = true;
      break;
    case DS_LDATA_SERVER_READ_FORCED:
      _isReadForced = true;
      break;
    case DS_LDATA_SERVER_WRITE:
      _isWrite = true;
    case DS_LDATA_SERVER_WRITE_FMQ:
      _isWriteFmq = true;
      break;
    } // switch (_mode)
    break;
  } // switch (_subType)

  // disassemble the message using the base-class routine
  
  if (DsMessage::disassemble(inMsg, msgLen)) {
    _errorStr = "ERROR - DsLdataServerMsg::disassemble\n";
    TaStr::AddStr(_errorStr, "  ", DateTime::str());
    _errorStr += "ERROR in DsMessage::disassemble()\n";
    return (-1);
  }

  // set data members to parts
  
  // debug
  if (partExists(DS_LDATA_SERVER_DEBUG_PART)) {
    _debug = true;
  }

  // read FMQ from start
  if (partExists(DS_LDATA_SERVER_READ_FMQ_FROM_START_PART)) {
    _readFmqFromStart = true;
  }

  // error
  if (partExists(DS_ERR_STRING)) {
    _errorOccurred = true;
    _errorStr = (char *)
      getPartByType(DS_ERR_STRING)->getBuf();
  }

  // url
  if (partExists(DS_LDATA_SERVER_URL_PART)) {
    _urlStr = (char *) getPartByType(DS_LDATA_SERVER_URL_PART)->getBuf();
  }

  // file name
  if (partExists(DS_LDATA_SERVER_FILENAME_PART)) {
    _fileName = (char *)
      getPartByType(DS_LDATA_SERVER_FILENAME_PART)->getBuf();
  }

  // ident string
  if (partExists(DS_LDATA_SERVER_IDENT_STR_PART)) {
    _identStr = (char *)
      getPartByType(DS_LDATA_SERVER_IDENT_STR_PART)->getBuf();
  }

  // latest time
  if (partExists(DS_LDATA_SERVER_LATEST_TIME_PART)) {
    ui32 ltime;
    memcpy(&ltime,
	   getPartByType(DS_LDATA_SERVER_LATEST_TIME_PART)->getBuf(),
	   sizeof(ltime));
    BE_to_array_32(&ltime, sizeof(ltime));
    _latestTime = ltime;
  }
  
  // max valid age
  if (partExists(DS_LDATA_SERVER_MAX_VALID_AGE_PART)) {
    si32 max_valid_age;
    memcpy(&max_valid_age,
	   getPartByType(DS_LDATA_SERVER_MAX_VALID_AGE_PART)->getBuf(),
	   sizeof(max_valid_age));
    BE_to_array_32(&max_valid_age, sizeof(max_valid_age));
    _maxValidAge = max_valid_age;
  }
  
  // ldata info

  int nLdata = partExists(DS_LDATA_SERVER_INFO_BUFFER_PART);
  for (int ii = 0; ii < nLdata; ii++) {
    DsMsgPart *infoPart =
      getPartByType(DS_LDATA_SERVER_INFO_BUFFER_PART, ii);
    LdataInfo *ldata = new LdataInfo;
    ldata->disassemble(infoPart->getBuf(), infoPart->getLength());
    _ldataQ.push_back(ldata);
  }

  return (0);

}

////////////////
// print message
//

void DsLdataServerMsg::print(ostream &out)

{

  switch (_subType) {
    
  case DS_LDATA_SERVER_READ:
    out << "Message subType: DS_LDATA_SERVER_READ" << endl;
    break;

  case DS_LDATA_SERVER_READ_FORCED:
    out << "Message subType: DS_LDATA_SERVER_READ_FORCED" << endl;
    break;

  case DS_LDATA_SERVER_WRITE:
    out << "Message subType: DS_LDATA_SERVER_WRITE" << endl;
    break;

  case DS_LDATA_SERVER_WRITE_FMQ:
    out << "Message subType: DS_LDATA_SERVER_WRITE_FMQ" << endl;
    break;

  case DS_LDATA_SERVER_RETURN:
    out << "Message subType: DS_LDATA_SERVER_RETURN" << endl;
    break;

  } // switch

  if (_debug) {
    out << "  debug: true" << endl;
  }

  if (_errorOccurred) {
    out << "  Error occurred" << endl;
    out << "  Error str: " << _errorStr << endl;
  }

  if (_urlStr.size() > 0) {
    out << "  urlStr: " << _urlStr << endl;
  }

  if (_fileName.size() > 0) {
    out << "  fileName: " << _fileName << endl;
  }
    
  if (_identStr.size() > 0) {
    out << "  fileName: " << _identStr << endl;
  }
    
  if (_ldataQ.size() > 0) {
    
    for (size_t ii = 0; ii << _ldataQ.size(); ii++) {
      out << "===========================================" << endl;
      out << "LdataInfo number: ii" << endl;
      _ldataQ[ii]->print(out);
    } // ii
    
  }

}

////////////////////////////////////////////////////////////
// clear queue

void DsLdataServerMsg::clearLdataQ()

{
  for (size_t ii = 0; ii < _ldataQ.size(); ii++) {
    delete _ldataQ[ii];
  }
  _ldataQ.clear();
  resetLdataQ();
}

////////////////////////////////////////////////////////////
// reset the queue

void DsLdataServerMsg::resetLdataQ()

{
  _ldataPos = 0;
}

////////////////////////////////////////////////////////////
// get the next element off the queue
//
// Returns NULL if none left

const LdataInfo *DsLdataServerMsg::getNextLdataInfo() const

{

  if (_ldataPos == _ldataQ.size()) {
    return NULL;
  }
  return _ldataQ[_ldataPos++];
  
}

