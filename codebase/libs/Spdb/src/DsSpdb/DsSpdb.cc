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
////////////////////////////////////////////////////////////////
// DsSpdb.cc
//
// DsSpdb class
//
// This class handles the Spdb operations with both the local
// disk and via the DsSpdbServer.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
////////////////////////////////////////////////////////////////

#include <Spdb/DsSpdb.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <didss/DsMessage.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DsClient.hh>
#include <dsserver/DsSvrMgrSocket.hh>
#include <dsserver/DsLocator.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/str.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
using namespace std;

///////////////////////////////////////////////////////////
// Constructor

DsSpdb::DsSpdb() : Spdb()

{

  _putInChild = false;
  _childCount = 0;
  _maxNChildren = 64;
  _childTimeoutSecs = 60;
  _registerWhileCleaning = true;
  _isLocal = false;
  _debug = false;
  clearHorizLimits();
  clearVertLimits();
  _dataCompressForTransfer = COMPRESSION_NONE;

}

////////////////////////////////////////////////////////////
// destructor

DsSpdb::~DsSpdb()

{
  cleanThreads(WAIT_TO_COMPLETE);
}

///////////////////////////////////////
// functions to set the put attributes

void DsSpdb::setPutThreadingOn()
{
  _putInChild = true;
}

void DsSpdb::setPutThreadingOff()
{
  _putInChild = false;
}

void DsSpdb::setMaxNThreads(int max_n_threads)
{
  _maxNChildren = max_n_threads;
}

void DsSpdb::clearUrls()
{
  _urlStrings.erase(_urlStrings.begin(), _urlStrings.end());
}

void DsSpdb::addUrl(const string &url_str)
{
  _urlStrings.push_back(url_str);
}

/////////////////////////////////
// set or clear horizontal limits
//
// Only relevant for requests to servers which can interpret
// the SPDB data, e.g. the Symprod servers.
  
void DsSpdb::setHorizLimits(double min_lat,
			    double min_lon,
			    double max_lat,
			    double max_lon)
  
{
  
  _minLat = min_lat;
  _minLon = min_lon;
  _maxLat = max_lat;
  _maxLon = max_lon;
  _horizLimitsSet = true;

}

void DsSpdb::clearHorizLimits()

{
  _horizLimitsSet = false;
}

///////////////////////////////
// set or clear vertical limits
//
// heights will generally be specified in km msl, though there
// may be instances in which the client and server agree on
// a different convention.
//
// Only relevant for requests to servers which can interpret
// the SPDB data, e.g. the Symprod servers.

void DsSpdb::setVertLimits(double min_ht,
                           double max_ht)
  
{
  _minHt = min_ht;
  _maxHt = max_ht;
  _vertLimitsSet = true;
}

void DsSpdb::clearVertLimits()

{
  _vertLimitsSet = false;
}

/////////////////////////////////
// set the limits in the message

void DsSpdb::setLimitsInMsg(DsSpdbMsg &msg)

{

  if (_horizLimitsSet) {
    msg.setHorizLimits(_minLat, _minLon, _maxLat, _maxLon);
  }
  if (_vertLimitsSet) {
    msg.setVertLimits(_minHt, _maxHt);
  }

}  

////////////////////////////////////////////////////
// Set data compression for transfer.
// Data can be compressed with either GZIP or BZIP2.
// If set, data will be compressed before transmission,
// and uncompressed on the receiving end. This applies to
// both putting and getting data.
// The default is COMPRESSION_NONE.

void DsSpdb::setDataCompressForTransfer(compression_t compression)
{
  _dataCompressForTransfer = compression;
}

//////////////////////////////////////////////////////////
// put - single chunk to single URL
//
// Chunk must already be in BE byte order, as appropriate.
//
// Returns 0 on success, -1 on error.

int DsSpdb::put(const string &url_str,
		int prod_id,
		const string &prod_label,
		int data_type,
		time_t valid_time,
		time_t expire_time,
		int chunk_len,
		const void *chunk_data,
		int data_type2 /* = 0*/)

{

  clearPutChunks();
  addPutChunk(data_type, valid_time, expire_time,
	      chunk_len, chunk_data, data_type2);
  return (put(url_str, prod_id, prod_label));
  
}

//////////////////////////////////////////////////////////
// put - multiple chunks to single URL
//
// Chunks must already be in BE byte order, as appropriate.
//
// Returns 0 on success, -1 on error

int DsSpdb::put(const string &url_str,
		int prod_id,
		const string &prod_label)
  
{

  _errStr = "ERROR - COMM - DsSpdb::put\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // decode URL

  if (_setUrl(url_str)) {
    return -1;
  }

  int iret;

  if (_isLocal) {
    iret = _localPut(prod_id, prod_label);
  } else {
    iret = _remotePut(prod_id, prod_label);
  }

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

///////////////////////////////////////////////////////////
// put - single chunk to multiple URLs
//
// Chunk must already be in BE byte order, as appropriate.
//
// Returns 0 on success, -1 on error

int DsSpdb::put(int prod_id,
		const string &prod_label,
		int data_type,
		time_t valid_time,
		time_t expire_time,
		int chunk_len,
		const void *chunk_data,
		int data_type2 /* = 0*/)

{

  clearPutChunks();
  addPutChunk(data_type, valid_time, expire_time,
	      chunk_len, chunk_data, data_type2);
  return (put(prod_id, prod_label));

}

//////////////////////////////////////////////////////////////
// put - multiple chunks to multiple URLs
//
// Chunks must already be in BE byte order, as appropriate.
//
// Returns 0 on success, -1 on error

int DsSpdb::put(int prod_id,
		const string &prod_label)

{

  _errStr = "ERROR - COMM - DsSpdb::put\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  
  int iret = 0;
  for (size_t i = 0; i < _urlStrings.size(); i++) {

    // decode URL
  
    if (_setUrl(_urlStrings[i])) {
      return -1;
    }
  
    if (_isLocal) {

      if (_localPut(prod_id, prod_label)) {
	iret = -1;
      }

    } else {

      if (_remotePut(prod_id, prod_label)) {
	iret = -1;
      }

    }

  } // i

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

///////////////////////////////////////////////////////////////////
// erase()
//
// Erase data for a given valid time and data type.
// If the data_type is 0, all data at that time is erased.
//
// Returns 0 on success, -1 on failure

int DsSpdb::erase(const string &url_str,
		  time_t valid_time,
		  int data_type /* = 0*/,
		  int data_type2 /* = 0*/)
  
{
  
  clearPutChunks();
  addPutChunk(data_type, valid_time, valid_time, 0, NULL, data_type2);
  return (erase(url_str));

}

///////////////////////////////////////////////////////////////////
// Erase data for a given set of chunk refs.
// Before calling this function, call clearPutChunks(),
// and addPutChunk() for each time and data_type you want to erase.
// You should call addPutChunk() with 0 length and NULL pointer.
// If the data_type is 0, all data at that time is erased.
// This is a special type of 'put'.
//
// Returns 0 on success, -1 on failure

int DsSpdb::erase(const string &url_str)

{

  _errStr = "ERROR - COMM - DsSpdb::erase\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // set put mode to erase

  setPutMode(putModeErase);

  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }
  
  int iret;

  if (_isLocal) {
    iret = Spdb::erase(_url.getFile());
  } else {
    iret = _remotePut(0, "");
  }
  
  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////
// Do the put() as specified in the message
// This is strictly local operation.
// Only the dir section of the URL is used.
// Returns 0 on success, -1 on failure.

int DsSpdb::doMsgPut(const DsSpdbMsg &inMsg)
  
{
  
  if (inMsg.getSubType() != DsSpdbMsg::DS_SPDB_PUT) {
    _errStr += "ERROR - DsSpdb::doMsgPut\n";
    TaStr::AddStr(_errStr, "   ", "Message not for put mode");
    TaStr::AddStr(_errStr, "   subtype: ",
                  DsSpdbMsg::subtype2Str(inMsg.getSubType()));
    TaStr::AddStr(_errStr, "   mode: ",
                  DsSpdbMsg::mode2Str(inMsg.getMode()));
    return -1;
  }

  // make a copy of the message

  DsSpdbMsg putMsg(inMsg);
  if (_debug) {
    putMsg.setDebug(true);
  }

  // get the directory location for the put operation
  
  DsURL url(putMsg.getUrlStr());
  if (!url.isValid()) {
    _errStr += "ERROR - DsSpdb::doMsgPut\n";
    TaStr::AddStr(_errStr, "   Invalid URL: ", putMsg.getUrlStr());
    return -1;
  }
  string dir = url.getFile();

  // uncompress data if needed

  putMsg.uncompressDataBuf();
  
  // set up the put args

  int mode = putMsg.getMode();
  Spdb::lead_time_storage_t lead_time_storage =
    (Spdb::lead_time_storage_t) putMsg.getInfo2().lead_time_storage;
  int prod_id = putMsg.getInfo().prod_id;
  const string &prod_label = putMsg.getInfo().prod_label;
  int n_chunks = putMsg.getNChunks();
  const Spdb::chunk_ref_t *chunk_refs = putMsg.getChunkRefs();
  const Spdb::aux_ref_t *aux_refs = putMsg.getAuxRefs();
  const void *chunk_data = putMsg.getChunkData();
  bool respect_zero_types = putMsg.getInfo().respect_zero_types;

  // set up the object for the put
  
  if (lead_time_storage != Spdb::LEAD_TIME_NOT_APPLICABLE) {
    setLeadTimeStorage((Spdb::lead_time_storage_t) lead_time_storage);
  }
  setRespectZeroTypesOnPut(respect_zero_types);

  clearPutChunks();
  _addPutChunks(n_chunks, chunk_refs, aux_refs, chunk_data);

  switch (mode) {
    
    case DsSpdbMsg::DS_SPDB_PUT_MODE_OVER:
      setPutMode(Spdb::putModeOver);
      if (Spdb::put(dir, prod_id, prod_label)) {
        _errStr += "  DsSpdb::doMsgPut.\n";
        _errStr += getErrStr();
        return -1;
      }
      break;
      
    case DsSpdbMsg::DS_SPDB_PUT_MODE_ADD:
      setPutMode(Spdb::putModeAdd);
      if (Spdb::put(dir, prod_id, prod_label)) {
        _errStr += "  DsSpdb::doMsgPut.\n";
        _errStr += getErrStr();
        return -1;
      }
      break;
      
    case DsSpdbMsg::DS_SPDB_PUT_MODE_ADD_UNIQUE:
      setPutMode(Spdb::putModeAddUnique);
      if (Spdb::put(dir, prod_id, prod_label)) {
        _errStr += "  DsSpdb::doMsgPut.\n";
        _errStr += getErrStr();
        return -1;
      }
      break;
      
    case DsSpdbMsg::DS_SPDB_PUT_MODE_ONCE:
      setPutMode(Spdb::putModeOnce);
      if (Spdb::put(dir, prod_id, prod_label)) {
        _errStr += "  DsSpdb::doMsgPut.\n";
        _errStr += getErrStr();
        return -1;
      }
      break;
      
    case DsSpdbMsg::DS_SPDB_PUT_MODE_ERASE:
      
      if (Spdb::erase(dir)) {
        _errStr += "  DsSpdb::doMsgPut.\n";
        _errStr += getErrStr();
        return -1;
      }
      break;
      
    default:
      break;
      
  } // switch (put_mode)
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////
// getExact()
//
// Get data at exactly the given time.
//

int DsSpdb::getExact(const string &url_str,
		     time_t request_time,
		     int data_type /* = 0*/,
		     int data_type2 /* = 0*/,
		     bool get_refs_only /* = false*/,
		     bool respect_zero_types /* = false*/)

{

  _errStr = "ERROR - COMM - DsSpdb::getExact\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);
  
  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }
  
  if (_isLocal) {
    
    int iret = Spdb::getExact(_url.getFile(), request_time,
			      data_type, data_type2,
			      get_refs_only, respect_zero_types);
    if (iret) {
      return -1;
    }
    
  } else {

    // create and assemble message
    
    DsSpdbMsg msg(DsMessage::CopyMem);
    setLimitsInMsg(msg);
    msg.setAuxXml(_auxXml);
    msg.setDebug(_debug);
    
    msg.assembleGetExact(url_str, request_time,
                         data_type, data_type2,
                         get_refs_only, respect_zero_types,
                         _getUnique,
                         _checkWriteTimeOnGet,
                         _latestValidWriteTime,
                         _dataCompressForTransfer);
    
    // communicate with server.

    if (_communicateGet(msg, _url)) {
      return -1;
    }

  }

  return 0;

}
  
///////////////////////////////////////////////////////////////////
// getClosest()
//
// Get data closest to the given time, within the time margin.
//

int DsSpdb::getClosest(const string &url_str,
		       time_t request_time,
		       int time_margin,
		       int data_type /* = 0*/,
		       int data_type2 /* = 0*/,
		       bool get_refs_only /* = false*/,
		       bool respect_zero_types /* = false*/)

{

  _errStr = "ERROR - COMM - DsSpdb::getClosest\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }

  if (_isLocal) {

    int iret = Spdb::getClosest(_url.getFile(), request_time,
				time_margin, data_type, data_type2,
				get_refs_only, respect_zero_types);
    if (iret) {
      return -1;
    }
    
  } else {
    
    // create and assemble message
    
    DsSpdbMsg msg(DsMessage::CopyMem);
    setLimitsInMsg(msg);
    msg.setAuxXml(_auxXml);
    msg.setDebug(_debug);
    msg.assembleGetClosest(url_str, request_time, time_margin,
                           data_type, data_type2,
                           get_refs_only, respect_zero_types,
                           _getUnique,
                           _checkWriteTimeOnGet,
                           _latestValidWriteTime,
                           _dataCompressForTransfer);
    
    // communicate with server.

    if (_communicateGet(msg, _url)) {
      return -1;
    }

  }

  return 0;

}
  
///////////////////////////////////////////////////////////////////
// getInterval()
//
// Get data in the time interval.
//

int DsSpdb::getInterval(const string &url_str,
			time_t start_time,
			time_t end_time,
			int data_type /* = 0*/,
			int data_type2 /* = 0*/,
			bool get_refs_only /* = false*/,
			bool respect_zero_types /* = false*/)

{

  _errStr = "ERROR - COMM - DsSpdb::getInterval\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }

  if (_isLocal) {

    int iret =
      Spdb::getInterval(_url.getFile(),
			start_time, end_time, data_type, data_type2,
			get_refs_only, respect_zero_types);
    if (iret) {
      return -1;
    }
    
  } else {

    // create and assemble message

    DsSpdbMsg msg(DsMessage::CopyMem);
    setLimitsInMsg(msg);
    msg.setAuxXml(_auxXml);
    msg.setDebug(_debug);
    msg.assembleGetInterval(url_str, start_time, end_time,
                            data_type, data_type2,
                            get_refs_only, respect_zero_types,
                            _getUnique,
                            _checkWriteTimeOnGet,
                            _latestValidWriteTime,
                            _dataCompressForTransfer);

    // communicate with server.

    if (_communicateGet(msg, _url)) {
      return -1;
    }

  }

  return 0;

}
  
///////////////////////////////////////////////////////////////////
// getValid()
//
// Get data valid at the given time.
//

int DsSpdb::getValid(const string &url_str,
		     time_t request_time,
		     int data_type /* = 0*/,
		     int data_type2 /* = 0*/,
		     bool get_refs_only /* = false*/,
		     bool respect_zero_types /* = false*/)

{

  _errStr = "ERROR - COMM - DsSpdb::getValid\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }

  if (_isLocal) {

    int iret = Spdb::getValid(_url.getFile(), request_time,
			      data_type, data_type2,
			      get_refs_only, respect_zero_types);
    if (iret) {
      return -1;
    }

  } else {

    // create and assemble message

    DsSpdbMsg msg(DsMessage::CopyMem);
    setLimitsInMsg(msg);
    msg.setAuxXml(_auxXml);
    msg.setDebug(_debug);
    msg.assembleGetValid(url_str, request_time,
                         data_type, data_type2,
                         get_refs_only, respect_zero_types,
                         _getUnique,
                         _checkWriteTimeOnGet, _latestValidWriteTime,
                         _dataCompressForTransfer);
    
    // communicate with server.

    if (_communicateGet(msg, _url)) {
      return -1;
    }

  }

  return 0;

}
  
///////////////////////////////////////////////////////////////////
// getLatest()
//
// Get latest data.
//

int DsSpdb::getLatest(const string &url_str,
		      int time_margin /* = 0*/,
		      int data_type /* = 0*/,
		      int data_type2 /* = 0*/,
		      bool get_refs_only /* = false*/,
		      bool respect_zero_types /* = false*/)

{

  _errStr = "ERROR - COMM - DsSpdb::getLatest\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }

  if (_isLocal) {

    int iret = Spdb::getLatest(_url.getFile(), time_margin,
			       data_type, data_type2,
			       get_refs_only, respect_zero_types);
    if (iret) {
      return -1;
    }

  } else {

    // create and assemble message
    
    DsSpdbMsg msg(DsMessage::CopyMem);
    setLimitsInMsg(msg);
    msg.setAuxXml(_auxXml);
    msg.setDebug(_debug);
    msg.assembleGetLatest(url_str, time_margin,
                          data_type, data_type2,
                          get_refs_only, respect_zero_types,
                          _getUnique,
                          _checkWriteTimeOnGet,
                          _latestValidWriteTime,
                          _dataCompressForTransfer);

    // communicate with server.

    if (_communicateGet(msg, _url)) {
      return -1;
    }

  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// getFirstBefore()
//
// Get first data at or before the requested time.
//

int DsSpdb::getFirstBefore(const string &url_str,
			   time_t request_time,
			   int time_margin,
			   int data_type /* = 0*/,
			   int data_type2 /* = 0*/,
			   bool get_refs_only /* = false*/,
			   bool respect_zero_types /* = false*/)

{

  _errStr = "ERROR - COMM - DsSpdb::getFirstBefore\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }

  if (_isLocal) {

    int iret = Spdb::getFirstBefore(_url.getFile(), request_time,
				    time_margin, data_type, data_type2,
				    get_refs_only, respect_zero_types);
    if (iret) {
      return -1;
    }

  } else {

    // create and assemble message

    DsSpdbMsg msg(DsMessage::CopyMem);
    setLimitsInMsg(msg);
    msg.setAuxXml(_auxXml);
    msg.setDebug(_debug);
    msg.assembleGetFirstBefore(url_str, request_time,
                               time_margin,
                               data_type, data_type2,
                               get_refs_only, respect_zero_types,
                               _getUnique,
                               _checkWriteTimeOnGet,
                               _latestValidWriteTime,
                               _dataCompressForTransfer);
    
    // communicate with server.

    if (_communicateGet(msg, _url)) {
      return -1;
    }

  }

  return 0;

}
  
///////////////////////////////////////////////////////////////////
// getFirstAfter()
//
// Get first data at or after the requested time.
//

int DsSpdb::getFirstAfter(const string &url_str,
			  time_t request_time,
			  int time_margin,
			  int data_type /* = 0*/,
			  int data_type2 /* = 0*/,
			  bool get_refs_only /* = false*/,
			  bool respect_zero_types /* = false*/)

{

  _errStr = "ERROR - COMM - DsSpdb::getFirstAfter\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }

  if (_isLocal) {

    int iret = Spdb::getFirstAfter(_url.getFile(), request_time,
				   time_margin, data_type, data_type2,
				   get_refs_only, respect_zero_types);
    if (iret) {
      return -1;
    }

  } else {

    // create and assemble message

    DsSpdbMsg msg(DsMessage::CopyMem);
    setLimitsInMsg(msg);
    msg.setAuxXml(_auxXml);
    msg.setDebug(_debug);
    msg.assembleGetFirstAfter(url_str, request_time,
                              time_margin,
                              data_type, data_type2,
                              get_refs_only, respect_zero_types,
                              _getUnique,
                              _checkWriteTimeOnGet,
                              _latestValidWriteTime,
                              _dataCompressForTransfer);
    
    // communicate with server.

    if (_communicateGet(msg, _url)) {
      return -1;
    }

  }

  return 0;

}
  
////////////////////////////////////////////////////////////
// get the first, last and last_valid_time in the data base
//
// In this case, no chunk data is returned.

int DsSpdb::getTimes(const string &url_str)
  
{

  _errStr = "ERROR - COMM - DsSpdb::getTimes\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }

  if (_isLocal) {
    
    int iret = Spdb::getTimes(_url.getFile());

    if (iret) {
      return -1;
    }
    
  } else {

    // create and assemble message
    
    DsSpdbMsg getMsg(DsMessage::CopyMem);
    getMsg.setAuxXml(_auxXml);
    getMsg.setDebug(_debug);
    getMsg.assembleGetTimes(url_str,
                            _checkWriteTimeOnGet,
                            _latestValidWriteTime);
    
    // communicate with server.

    DsSpdbMsg replyMsg;
    if (_communicate(getMsg, _url, replyMsg)) {
      _errStr += "ERROR - COMM - DsSpdb::getTimes\n";
      _errStr += " Communicating with server\n";
      TaStr::AddStr(_errStr, "  URL: ", _url.getURLStr());
      return -1;
    }

    // check for error
    
    if (replyMsg.errorOccurred()) {
      _errStr += replyMsg.getErrorStr();
      return -1;
    }

    // set times

    const DsSpdbMsg::info_t &info = replyMsg.getInfo();
    _firstTime = info.start_time;
    _lastTime = info.end_time;
    _lastValidTime = info.last_valid_time;

    // set prod id and label

    _prodId = replyMsg.getInfo().prod_id;
    _prodLabel = replyMsg.getInfo().prod_label;

  }

  return 0;

}
  
////////////////////////////////////////////////////////////
// get the first, last and last_valid_time in the data base
//
// In this case, no chunk data is returned.

int DsSpdb::getTimes(const string &url_str,
		     time_t &first_time,
		     time_t &last_time,
		     time_t &last_valid_time)
  
{

  if (getTimes(url_str)) {
    return -1;
  }

  first_time = _firstTime;
  last_time = _lastTime;
  last_valid_time = _lastValidTime;

  return 0;

}
  
//////////////////////////////////////////////////////////
// compile time list
//
// Compile a list of available data times at the specified
// url between the start and end times.
//
// The optional minimum_interval arg specifies the minimum
// interval in secs between times in the time list. This
// allows you to cull the list to suit your needs.
// minimum_interval default to 1 sec, which will return a 
// full list with no duplicates. If you set to to 0, you
// will get duplicates if there is more than one entry at
// the same time. If you set it to > 1, you will cull the
// list to the required sparseness.
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.
//
// After a successful call to compileTimeList(), access the
// time list via the following functions:
//   getTimeList(): vector of time_t
//   getNTimesInList(): n entries in list
//   getTimeFromList(int i): time_t from list
  
int DsSpdb::compileTimeList(const string &url_str,
			    time_t start_time,
			    time_t end_time,
			    size_t minimum_interval /* = 1*/)
{

  _errStr = "ERROR - COMM - DsSpdb::compileTimeList\n";
  TaStr::AddStr(_errStr, "  Time: ", DateTime::str());
  TaStr::AddStr(_errStr, "  URL: ", url_str);

  // decode URL
  
  if (_setUrl(url_str)) {
    return -1;
  }
  
  if (_isLocal) {
    
    int iret =
      Spdb::compileTimeList(_url.getFile(),
			    start_time, end_time, minimum_interval);
    if (iret) {
      return -1;
    }
    
  } else {
    
    // create and assemble message

    DsSpdbMsg getMsg(DsMessage::CopyMem);
    getMsg.setDebug(_debug);
    getMsg.assembleCompileTimeList(url_str, start_time, end_time,
                                   minimum_interval,
                                   _checkWriteTimeOnGet,
                                   _latestValidWriteTime);
    
    // communicate with server.

    DsSpdbMsg replyMsg;
    if (_communicate(getMsg, _url, replyMsg)) {
      _errStr += "ERROR - COMM - DsSpdb::getTimeList\n";
      _errStr += " Communicating with server\n";
      TaStr::AddStr(_errStr, "  URL: ", _url.getURLStr());
      return -1;
    }

    // check for error
    
    if (replyMsg.errorOccurred()) {
      _errStr += replyMsg.getErrorStr();
      return -1;
    }
    
    // make local copies of the data
    
    _prodId = replyMsg.getInfo().prod_id;
    _prodLabel = replyMsg.getInfo().prod_label;
    _timeList = replyMsg.getTimeList();

  }

  return 0;

}
  
/////////////////////////////////////////////
// Do the get() as specified in the message.
// Loads the getInfo struct.
// This is a strictly local operation.
// Only the dir section of the URL is used.
// Returns 0 on success, -1 on failure

int DsSpdb::doMsgGet(const DsSpdbMsg &inMsg, DsSpdbMsg::info_t &getInfo)
  
{

  MEM_zero(getInfo);

  if (inMsg.getSubType() != DsSpdbMsg::DS_SPDB_GET) {
    _errStr += "ERROR - DsSpdb::doMsgGet\n";
    TaStr::AddStr(_errStr, "   ", "Message not for get mode");
    TaStr::AddStr(_errStr, "   subtype: ",
                  DsSpdbMsg::subtype2Str(inMsg.getSubType()));
    TaStr::AddStr(_errStr, "   mode: ",
                  DsSpdbMsg::mode2Str(inMsg.getMode()));
    return -1;
  }
  
  // get the directory location for the get operation
  
  DsURL url(inMsg.getUrlStr());
  if (!url.isValid()) {
    _errStr += "ERROR - DsSpdb::doMsgGet\n";
    TaStr::AddStr(_errStr, "   Invalid URL: ", inMsg.getUrlStr());
    return -1;
  }
  string dir = url.getFile();
  
  // set up for the get operation, from the message parts

  const DsSpdbMsg::info_t &info = inMsg.getInfo();
  if (info.get_unique == Spdb::UniqueLatest) {
    setUniqueLatest();
  } else if (info.get_unique == Spdb::UniqueEarliest) {
    setUniqueEarliest();
  } else {
    setUniqueOff();
  }

  const DsSpdbMsg::info2_t &info2 = inMsg.getInfo2();
  if (info2.check_write_time_on_get) {
    setCheckWriteTimeOnGet(info2.latest_valid_write_time);
  } else {
    clearCheckWriteTimeOnGet();
  }

  if (inMsg.horizLimitsSet()) {
    const DsSpdbMsg::horiz_limits_t &hlimits = inMsg.getHorizLimits();
    setHorizLimits(hlimits.min_lat, hlimits.min_lon,
                   hlimits.max_lat, hlimits.max_lon);
  } else {
    clearHorizLimits();
  }

  if (inMsg.vertLimitsSet()) {
    const DsSpdbMsg::vert_limits_t &vlimits = inMsg.getVertLimits();
    setVertLimits(vlimits.min_ht, vlimits.max_ht);
  } else {
    clearVertLimits();
  }

  // perform the get specified by the mode

  switch (inMsg.getMode()) {
    
    case DsSpdbMsg::DS_SPDB_GET_MODE_EXACT:
      if (Spdb::getExact(dir, info.request_time,
                         info.data_type, info.data_type2,
                         info.get_refs_only, info.respect_zero_types)) {
        _errStr += "ERROR - DsSpdb::doMsgGet\n";
        return -1;
      }
      getInfo.n_chunks = getNChunks();
      getInfo.prod_id = getProdId();
      getInfo.get_refs_only = info.get_refs_only;
      STRncopy(getInfo.prod_label, getProdLabel().c_str(),
               SPDB_LABEL_MAX);
      break;
      
    case DsSpdbMsg::DS_SPDB_GET_MODE_CLOSEST:
      if (Spdb::getClosest(dir, info.request_time,
                           info.time_margin,
                           info.data_type, info.data_type2,
                           info.get_refs_only, info.respect_zero_types)) {
        _errStr += "ERROR - DsSpdb::doMsgGet\n";
        return -1;
      }
      getInfo.n_chunks = getNChunks();
      getInfo.prod_id = getProdId();
      getInfo.get_refs_only = info.get_refs_only;
      STRncopy(getInfo.prod_label, getProdLabel().c_str(),
               SPDB_LABEL_MAX);
      break;
      
    case DsSpdbMsg::DS_SPDB_GET_MODE_INTERVAL:
      if (Spdb::getInterval(dir,
                            info.start_time, info.end_time,
                            info.data_type, info.data_type2,
                            info.get_refs_only, info.respect_zero_types)) {
        _errStr += "ERROR - DsSpdb::doMsgGet\n";
        return -1;
      }
      getInfo.n_chunks = getNChunks();
      getInfo.prod_id = getProdId();
      getInfo.get_refs_only = info.get_refs_only;
      STRncopy(getInfo.prod_label, getProdLabel().c_str(),
               SPDB_LABEL_MAX);
      break;
      
    case DsSpdbMsg::DS_SPDB_GET_MODE_VALID:
      if (Spdb::getValid(dir, info.request_time,
                         info.data_type, info.data_type2,
                         info.get_refs_only, info.respect_zero_types)) {
        _errStr += "ERROR - DsSpdb::doMsgGet\n";
        return -1;
      }
      getInfo.n_chunks = getNChunks();
      getInfo.prod_id = getProdId();
      getInfo.get_refs_only = info.get_refs_only;
      STRncopy(getInfo.prod_label, getProdLabel().c_str(),
               SPDB_LABEL_MAX);
      break;
      
    case DsSpdbMsg::DS_SPDB_GET_MODE_LATEST:
      if (Spdb::getLatest(dir, info.time_margin,
                          info.data_type, info.data_type2,
                          info.get_refs_only, info.respect_zero_types)) {
        _errStr += "ERROR - DsSpdb::doMsgGet\n";
        return -1;
      }
      getInfo.n_chunks = getNChunks();
      getInfo.prod_id = getProdId();
      getInfo.get_refs_only = info.get_refs_only;
      STRncopy(getInfo.prod_label, getProdLabel().c_str(),
               SPDB_LABEL_MAX);
      break;
      
    case DsSpdbMsg::DS_SPDB_GET_MODE_FIRST_BEFORE:
      if (Spdb::getFirstBefore(dir, info.request_time,
                               info.time_margin,
                               info.data_type, info.data_type2,
                               info.get_refs_only, info.respect_zero_types)) {
        _errStr += "ERROR - DsSpdb::doMsgGet\n";
        return -1;
      }
      getInfo.n_chunks = getNChunks();
      getInfo.prod_id = getProdId();
      getInfo.get_refs_only = info.get_refs_only;
      STRncopy(getInfo.prod_label, getProdLabel().c_str(),
               SPDB_LABEL_MAX);
      break;
      
    case DsSpdbMsg::DS_SPDB_GET_MODE_FIRST_AFTER:
      if (Spdb::getFirstAfter(dir, info.request_time,
                              info.time_margin,
                              info.data_type, info.data_type2,
                              info.get_refs_only, info.respect_zero_types)) {
        _errStr += "ERROR - DsSpdb::doMsgGet\n";
        return -1;
      }
      getInfo.n_chunks = getNChunks();
      getInfo.prod_id = getProdId();
      getInfo.get_refs_only = info.get_refs_only;
      STRncopy(getInfo.prod_label, getProdLabel().c_str(),
               SPDB_LABEL_MAX);
      break;
      
    case DsSpdbMsg::DS_SPDB_GET_MODE_TIMES:
      time_t first, last, lastValid;
      if (Spdb::getTimes(dir, first, last, lastValid)) {
        _errStr += "ERROR - DsSpdb::doMsgGet\n";
        return -1;
      }
      getInfo.start_time = first;
      getInfo.end_time = last;
      getInfo.last_valid_time = lastValid;
      break;
      
    case DsSpdbMsg::DS_SPDB_GET_MODE_TIME_LIST:
      if (Spdb::compileTimeList(dir,
                                info.start_time, info.end_time,
                                info.time_margin)) {
        _errStr += "ERROR - DsSpdb::doMsgGet\n";
        return -1;
      }
      getInfo.prod_id = getProdId();
      STRncopy(getInfo.prod_label, getProdLabel().c_str(),
               SPDB_LABEL_MAX);
      break;

    default:
      break;
      
  } // switch

  return 0;

}

//////////////////////////////////////////////////////////////
// cleanThreads
//
// clean up the threads

void DsSpdb::cleanThreads(thread_cleanup_mode_t mode /* = WAIT_TO_COMPLETE*/)

{

  if (mode == CLEAN_COMPLETED_ONLY) {

    _reapChildren();

  } else if (mode == CANCEL_INCOMPLETE) {

    _reapChildren(true);

  } else {

    while (_childList.size() > 0) {
      if (_registerWhileCleaning) {
	PMU_auto_register("Cleaning up threads");
      }
      _reapChildren();
      if (_childList.size() > 0) {
	umsleep(100);
      }
    }
    
  }

}

////////////////////////////////////////////////////////////////
// Purge completed put children
// Returns the number of threads purged

int DsSpdb::_reapChildren(bool cancel_uncompleted /* = false*/)

{

  bool done = false;
  int count = 0;

  while (!done) {
    
    done = true;
    
    list<PutArgs *>::iterator ii;
    for (ii = _childList.begin(); ii != _childList.end(); ii++) {
      PutArgs *args = (*ii);

      // check if done

      if (waitpid(args->childPid,
		  (int *) NULL,
		  (int) (WNOHANG | WUNTRACED)) == (pid_t) args->childPid) {
	
	_childCount--;
	delete args;
	_childList.erase(ii);
	done = false;
	count++;
	break;

      }

      // check for hung child, kill if timed out
      // waitpid() will get it later

      if (cancel_uncompleted) {
	time_t now = time(NULL);
	if (now > args->childExpireTime) {
	  if (kill((pid_t) args->childPid, SIGKILL)) {
	    cerr << "ERROR - DsSpdb::_reapChildren" << endl;
	    cerr << "  " << DateTime::str() << endl;
	    cerr << "  Cannot kill pid: " << args->childPid << endl;
	    cerr << "  url: " << args->urlStr << endl;
	  }
	}
      }

    } // ii

  } // while

  return count;

}

//////////////////////////////////////////////////////////////
// _localPut
//
// All local puts come through here
//
// Returns 0 on success, -1 on failure

int DsSpdb::_localPut(int prod_id,
		      const string &prod_label)

{

  string dir_str = _url.getFile();

  int iret = 0;

  if (Spdb::put(dir_str, prod_id, prod_label)) {
    iret = -1;
  }

  return iret;
  
}


//////////////////////////////////////////////////////////////
// _remotePut
//
// All remote puts come through here

int DsSpdb::_remotePut(int prod_id,
		       const string &prod_label)

{

  // create args object for thread
  
  DsSpdbMsg::mode_enum_t mode = DsSpdbMsg::DS_SPDB_PUT_MODE_ADD;
  if (_putMode == putModeOver) {
    mode = DsSpdbMsg::DS_SPDB_PUT_MODE_OVER;
  } else if (_putMode == putModeAdd) {
    mode = DsSpdbMsg::DS_SPDB_PUT_MODE_ADD;
  } else if (_putMode == putModeAddUnique) {
    mode = DsSpdbMsg::DS_SPDB_PUT_MODE_ADD_UNIQUE;
  } else if (_putMode == putModeOnce) {
    mode = DsSpdbMsg::DS_SPDB_PUT_MODE_ONCE;
  } else if (_putMode == putModeErase) {
    mode = DsSpdbMsg::DS_SPDB_PUT_MODE_ERASE;
  }
  
  // assemble the message

  DsSpdbMsg putMsg;
  putMsg.setAuxXml(_auxXml);
  putMsg.setDebug(_debug);
  
  putMsg.assemblePut(_appName,
                     _url.getURLStr(),
                     prod_id, prod_label, mode,
                     _leadTimeStorage,
                     _nPutChunks,
                     _putRefBuf,
                     _putAuxBuf,
                     _putDataBuf,
                     _respectZeroTypes,
                     _dataCompressForTransfer);

  PutArgs *putArgs = new PutArgs(_url.getURLStr(), _childTimeoutSecs);
  
  if (_putInChild) {
    
    if (_childCount > _maxNChildren) {
      cerr << "ERROR - SpdbPut" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Too many children, max allowed: " << _maxNChildren << endl;
      delete (putArgs);
      return -1;
    }
    
    // spawn a child for doing the put
      
    putArgs->childRunning = true;
    putArgs->childPid = fork();
    
    // check for parent or child
    
    if (putArgs->childPid == 0) {
      
      // child - do the put and exit

      _doRemotePut(putArgs, putMsg);
      _exit(0);
      
    } else {

      // parent
      
      _childCount++;
      _childList.push_back(putArgs);

    }

  } else {
    
    // no threading call function directly
    
    if(_doRemotePut(putArgs, putMsg) != NULL) {
      delete (putArgs);
      return -1;
    } else {
      delete (putArgs);
    }
    
  }
  
  return 0;

}

////////////////////////////////////////////////
// function for processing remote put in thread

void *DsSpdb::_doRemotePut(PutArgs *pArgs,
                           const DsSpdbMsg &putMsg)
  
{

  // URL object interprets the URL
  
  DsURL url(pArgs->urlStr);
  
  // resolve the port if needed

  if (url.getPort() < 0) {
    DsSvrMgrSocket mgrSock;
    string errorStr;
    if (mgrSock.findPortForURL(url.getHost().c_str(), url,
			       -1, errorStr)) {
      cerr << "ERROR - COMM - DsSpdb::_doRemotePut" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Cannot resolve port from ServerMgr" << endl;
      cerr << "  " << errorStr << endl;
      pArgs->childDone = true;
      return ((void *) -1);
    }
  }

  // communicate with server, read reply

  DsSpdbMsg replyMsg;
  if (_communicate(putMsg, url, replyMsg)) {
    cerr << "ERROR - COMM - DsSpdb::_doRemotePut" << endl;
    cerr << "  Communicating with server" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << _errStr << endl;
    pArgs->childDone = true;
    return ((void *) -1);
  }

  // check for error

  if (replyMsg.errorOccurred()) {
    cerr << "ERROR - COMM - DsSpdb::_doRemotePut" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  " << replyMsg.getErrorStr() << endl;
    pArgs->childDone = true;
    return ((void *) -1);
  }

  pArgs->childDone = true;
  return (NULL);

}

 
/////////////////////////////////////////////
// communicate put message to server,
// and read reply.
//
// Returns 0 on success, -1 on error.

int DsSpdb::_communicatePut(const DsSpdbMsg &inMsg,
			    const DsURL &url,
                            DsSpdbMsg &replyMsg)
  
{

  if (_communicate(inMsg, url, replyMsg)) {
    _errStr += "ERROR - DsSpdb::_communicatePut\n";
    // print error for puts since it is in a child process
    cerr << _errStr << endl;
    return -1;
  }
  return 0;

}

/////////////////////////////////////////////
// communicate get message to server,
// and read reply
//
// Returns 0 on success, -1 on error.

int DsSpdb::_communicateGet(const DsSpdbMsg &inMsg,
			    const DsURL &url)

{

  // communicate with server.

  DsSpdbMsg replyMsg;
  if (_communicate(inMsg, url, replyMsg)) {
    _errStr += "ERROR - DsSpdb::_communicateGet\n";
    return -1;
  }
  
  // check for error
    
  if (replyMsg.errorOccurred()) {
    _errStr += replyMsg.getErrorStr();
    return -1;
  }

  // make local copies of the data

  _loadChunkData(replyMsg.getInfo().prod_id,
                 replyMsg.getInfo().prod_label,
                 replyMsg.getNChunks(),
                 replyMsg.getChunkRefs(),
                 replyMsg.getAuxRefs(),
                 replyMsg.getInfo().get_refs_only,
                 replyMsg.getChunkData(),
                 replyMsg.getChunkDataLen());

  return 0;

}

/////////////////////////////////////////////////
// communicate message to server, and read reply
//
// Load up error string
//
// Returns 0 on success, -1 on error.

int DsSpdb::_communicate(const DsSpdbMsg &inMsg,
			 const DsURL &url,
                         DsSpdbMsg &replyMsg)

{

  DsClient client;
  if (_debug) {
    client.setDebug(true);
  }
  
  if (_debug) {
    cerr << "------------------- DsSpdb::_communicate -------------------" << endl;
    cerr << "Request message: " << endl;
    inMsg.print(cerr);
    cerr << "------------------------------------------------------------" << endl;
  }
  
  if (client.communicateAutoFwd(url, DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
				inMsg.assembledMsg(), inMsg.lengthAssembled())) {
    _errStr += "ERROR - DsSpdb::_communicate\n";
    _errStr += "ERROR - COMM - communicating with server.\n";
    _errStr += client.getErrStr();
    TaStr::AddStr(_errStr, "  URL: ", url.getURLStr());
    return -1;
  }
  
  // disassemble the reply message
  
  if (replyMsg.disassemble(client.getReplyBuf(), client.getReplyLen())) {
    _errStr += "ERROR - DsSpdb::_communicate\n";
    _errStr += "  Invalid reply - cannot disassemble.\n";
    TaStr::AddStr(_errStr, "  URL: ", url.getURLStr());
    return-1;
  }
  
  if (_debug) {
    cerr << "------------------- DsSpdb::_communicate -------------------" << endl;
    cerr << "Reply message: " << endl;
    replyMsg.print(cerr);
    cerr << "------------------------------------------------------------" << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////
// _setUrl
//
// Check the URL, then set it in the class.
// 
// Side effects:
//
//  * decodes the URL, sets _url
//  * decides if it is local or not, sets _isLocal accordingly.
//  * determines the default port
//
// Returns 0 on success, -1 on failure.

int DsSpdb::_setUrl(const string &url_str)

{

  // Operate off of the fully resolved url, not just the one passed in
  // Do not contact the ServerMgr at this stage, just get the default port
  
  _url.setURLStr(url_str);

  bool  contactServer;
  if (DsLocator.resolve(_url, &contactServer, false) != 0) {
    _errStr += "ERROR - COMM - DsSpdb::_setUrl\n";
    TaStr::AddStr(_errStr, "  Cannot resolve URL: ", url_str);
    return -1;
  }
  
  // determine if host is local
  _isLocal = !contactServer;
  
  return 0;
}


///////////////////////////////////////
// copy handle members to local memory

void DsSpdb::_loadChunkData(int prod_id,
                            const char *prod_label,
                            int n_chunks,
                            const Spdb::chunk_ref_t *chunk_refs,
                            const Spdb::aux_ref_t *aux_refs,
                            bool get_refs_only,
                            const void *chunk_data,
                            int chunk_data_len)

{

  _prodId = prod_id;
  _prodLabel = prod_label;

  _nGetChunks = n_chunks;
  
  _getRefBuf.free();
  if (chunk_refs != NULL) {
    _getRefBuf.add(chunk_refs, _nGetChunks * sizeof(Spdb::chunk_ref_t));
  }
  
  _getAuxBuf.free();
  if (aux_refs != NULL) {
    _getAuxBuf.add(aux_refs, _nGetChunks * sizeof(Spdb::aux_ref_t));
  }
  
  _getRefsOnly = get_refs_only;
  _getDataBuf.free();
  if (!get_refs_only) {
    _getDataBuf.add(chunk_data, chunk_data_len);
  }

  _loadChunksFromGet();

}


