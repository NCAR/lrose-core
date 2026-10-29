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
// Spdb.cc
//
// Spdb class
//
// This class handles the Spdb operations with both the local
// disk and via the SpdbServer.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
////////////////////////////////////////////////////////////////

#include <spdb/Spdb.hh>
#include <spdb/SpdbMsg.hh>
#include <spdb/SpdbPutArgs.hh>
#include <didss/RapDataDir.hh>
#include <didss/DsMessage.hh>
#include <dsserver/DsSvrMgrSocket.hh>
#include <dsserver/DsLocator.hh>
#include <iostream>
#include <pthread.h>
using namespace std;

// instantiate threadCount

int Spdb::_threadCount = 0;

////////////////////////////////////////////////////////////
// Constructor

Spdb::Spdb()

{

  _nChunks = 0;
  _prodId = 0;
  _putMode = putModeOver;
  _putThreading = true;
  _maxNThreads = 128;

}

////////////////////////////////////////////////////////////
// destructor

Spdb::~Spdb()

{

}

///////////////////////////////////////
// functions to set the put attributes

void Spdb::setPutThreadingOn()
{
  _putThreading = true;
}

void Spdb::setPutThreadingOff()
{
  _putThreading = false;
}

void Spdb::setMaxNThreads(int max_n_threads)
{
  _maxNThreads = max_n_threads;
}

void Spdb::clearUrls()
{
  _urlStrings.erase(_urlStrings.begin(), _urlStrings.end());
}

void Spdb::addUrl(const string &url_str)
{
  _urlStrings.push_back(url_str);
}

void Spdb::setPutMode(const put_mode_t mode)
{
  _putMode = mode;
}

////////////////////////////////////////////////////
// clear chunks before creating buffer using 'addChunk'

void Spdb::clearChunks()
{
  _nChunks = 0;
  _chunkRefBuf.reset();
  _chunkDataBuf.reset();
}

////////////////////////////////////
// Add a chunk to the chunk buffer

void Spdb::addChunk(const int data_type,
		    const time_t valid_time,
		    const time_t expire_time,
		    const int chunk_len,
		    const void *chunk_data)

{

  // load ref
  
  spdb_chunk_ref_t ref;
  ref.data_type = data_type;
  ref.valid_time = valid_time;
  ref.expire_time = expire_time;
  ref.len = chunk_len;
  ref.offset = _chunkDataBuf.getLen();

  // add to buffers
  
  _nChunks++;
  _chunkRefBuf.add(&ref, sizeof(spdb_chunk_ref_t));
  _chunkDataBuf.add(chunk_data, chunk_len);

}

//////////////////////////////////////////////////////////
// put - single chunk to single URL
//
// Chunk must already be in BE byte order, as appropriate.
//
// Returns 0 on success, -1 on error.

int Spdb::put(const string &url_str,
	      const int prod_id,
	      const string &prod_label,
	      const int data_type,
	      const time_t valid_time,
	      const time_t expire_time,
	      const int chunk_len,
	      const void *chunk_data)

{

  clearChunks();
  addChunk(data_type, valid_time, expire_time, chunk_len, chunk_data);
  return (_genericPut(url_str, prod_id, prod_label));

}

///////////////////////////////////////////////////////////
// put - single chunk to multiple URLs
//
// Chunk must already be in BE byte order, as appropriate.
//
// Returns 0 on success, -1 on error

int Spdb::put(const int prod_id,
	      const string &prod_label,
	      const int data_type,
	      const time_t valid_time,
	      const time_t expire_time,
	      const int chunk_len,
	      const void *chunk_data)

{

  clearChunks();
  addChunk(data_type, valid_time, expire_time, chunk_len, chunk_data);

  int iret = 0;
  for (unsigned int i = 0; i < _urlStrings.size(); i++) {
    if (_genericPut(_urlStrings[i], prod_id, prod_label)) {
      iret = -1;
    }
  }
  return (iret);

}

//////////////////////////////////////////////////////////
// put - multiple chunks to single URL
//
// Chunks must already be in BE byte order, as appropriate.
//
// Returns 0 on success, -1 on error

int Spdb::put(const string &url_str,
	      const int prod_id,
	      const string &prod_label)
  
{
  
  return (_genericPut(url_str, prod_id, prod_label));

}

//////////////////////////////////////////////////////////////
// put - multiple chunks to multiple URLs
//
// Chunks must already be in BE byte order, as appropriate.
//
// Returns 0 on success, -1 on error

int Spdb::put(const int prod_id,
	      const string &prod_label)

{

  int iret = 0;
  for (unsigned int i = 0; i < _urlStrings.size(); i++) {
    if (_genericPut(_urlStrings[i], prod_id, prod_label)) {
      iret = -1;
    }
  }
  return (iret);

}

///////////////////////////////////////////////////////////////////
// getExact()
//
// Get data at exactly the given time.
//

int Spdb::getExact(const string &url_str,
		   const time_t request_time,
		   const int data_type /* = 0*/ )

{

  // decode URL
  
  if (_checkUrl(url_str)) {
    return (-1);
  }

  if (_isLocal) {

    spdb_handle_t handle;
    if (SPDB_init(&handle, NULL, _prodId, (char *) _dirPath.c_str())) {
      cerr << "ERROR - Spdb::getExact" << endl;
      cerr << "Cannot init SPDB_handle." << endl;
      return (-1);
    }
  
    si32 nChunks;
    spdb_chunk_ref_t *chunkRefs;
    void *chunkData;

    if (SPDB_fetch(&handle, data_type, request_time,
		   &nChunks, &chunkRefs, &chunkData)) {
      cerr << "ERROR - Spdb::getExact" << endl;
      cerr << "Fetch failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    // make local copies of the data
    
    _copyChunkMembers(handle.prod_id, handle.prod_label,
		      nChunks, chunkRefs,
		      chunkData, SPDB_chunk_data_len(&handle));

    // free up handle

    SPDB_free(&handle);

  } else {

    // create and assemble message

    SpdbMsg msg(DsMessage::PointToMem);
    void *buf =
      msg.assembleGetExact(url_str, request_time, data_type);

    // communicate with server. Message memory is owned by socket,
    // so this must stay in scope until after the copy

    ThreadSocket sock;
    if (_communicateGet(sock, msg, buf, msg.lengthAssembled())) {
      cerr << "ERROR - Spdb::getExact" << endl;
      cerr << "  Communicating with server." << endl;
      return (-1);
    }

    // check for error
    
    if (msg.errorOccurred()) {
      cerr << "ERROR - Spdb::getExact" << endl;
      cerr << "  " << msg.getErrorStr() << endl;
      return (-1);
    }

    // make local copies of the data

    _copyChunkMembers(msg.getInfo().prod_id, msg.getInfo().prod_label,
		      msg.getNChunks(), msg.getChunkRefs(),
		      msg.getChunkData(), msg.getChunkDataLen());


  }

  return (0);

}
  
///////////////////////////////////////////////////////////////////
// getClosest()
//
// Get data closest to the given time, within the time margin.
//

int Spdb::getClosest(const string &url_str,
		     const time_t request_time,
		     const int time_margin,
		     const int data_type /* = 0*/ )

{

  // decode URL
  
  if (_checkUrl(url_str)) {
    return (-1);
  }

  if (_isLocal) {

    spdb_handle_t handle;
    if (SPDB_init(&handle, NULL, _prodId, (char *) _dirPath.c_str())) {
      cerr << "ERROR - Spdb::getClosest" << endl;
      cerr << "Cannot init SPDB_handle." << endl;
      return (-1);
    }
  
    si32 nChunks;
    spdb_chunk_ref_t *chunkRefs;
    void *chunkData;

    if (SPDB_fetch_closest(&handle, data_type, request_time, time_margin,
			   &nChunks, &chunkRefs, &chunkData)) {
      cerr << "ERROR - Spdb::getClosest" << endl;
      cerr << "Fetch failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    // make local copies of the data
    
    _copyChunkMembers(handle.prod_id, handle.prod_label,
		      nChunks, chunkRefs,
		      chunkData, SPDB_chunk_data_len(&handle));

    // free up handle

    SPDB_free(&handle);

  } else {

    // create and assemble message

    SpdbMsg msg(DsMessage::PointToMem);
    void *buf =
      msg.assembleGetClosest(url_str, request_time, time_margin, data_type);

    // communicate with server. Message memory is owned by socket,
    // so this must stay in scope until after the copy

    ThreadSocket sock;
    if (_communicateGet(sock, msg, buf, msg.lengthAssembled())) {
      cerr << "ERROR - Spdb::getClosest" << endl;
      cerr << "  Communicating with server." << endl;
      return (-1);
    }

    // check for error
    
    if (msg.errorOccurred()) {
      cerr << "ERROR - Spdb::getClosest" << endl;
      cerr << "  " << msg.getErrorStr() << endl;
      return (-1);
    }

    // make local copies of the data

    _copyChunkMembers(msg.getInfo().prod_id, msg.getInfo().prod_label,
		      msg.getNChunks(), msg.getChunkRefs(),
		      msg.getChunkData(), msg.getChunkDataLen());

  }

  return (0);

}
  
///////////////////////////////////////////////////////////////////
// getInterval()
//
// Get data in the time interval.
//

int Spdb::getInterval(const string &url_str,
		      const time_t start_time,
		      const time_t end_time,
		      const int data_type /* = 0*/ )

{

  // decode URL
  
  if (_checkUrl(url_str)) {
    return (-1);
  }

  if (_isLocal) {

    spdb_handle_t handle;
    if (SPDB_init(&handle, NULL, _prodId, (char *) _dirPath.c_str())) {
      cerr << "ERROR - Spdb::getInterval" << endl;
      cerr << "Cannot init SPDB_handle." << endl;
      return (-1);
    }
  
    si32 nChunks;
    spdb_chunk_ref_t *chunkRefs;
    void *chunkData;

    if (SPDB_fetch_interval(&handle, data_type, start_time, end_time,
			   &nChunks, &chunkRefs, &chunkData)) {
      cerr << "ERROR - Spdb::getInterval" << endl;
      cerr << "Fetch failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    // make local copies of the data
    
    _copyChunkMembers(handle.prod_id, handle.prod_label,
		      nChunks, chunkRefs,
		      chunkData, SPDB_chunk_data_len(&handle));

    // free up handle

    SPDB_free(&handle);

  } else {

    // create and assemble message

    SpdbMsg msg(DsMessage::PointToMem);
    void *buf =
      msg.assembleGetInterval(url_str, start_time, end_time, data_type);

    // communicate with server. Message memory is owned by socket,
    // so this must stay in scope until after the copy

    ThreadSocket sock;
    if (_communicateGet(sock, msg, buf, msg.lengthAssembled())) {
      cerr << "ERROR - Spdb::getInterval" << endl;
      cerr << "  Communicating with server." << endl;
      return (-1);
    }

    // check for error
    
    if (msg.errorOccurred()) {
      cerr << "ERROR - Spdb::getInterval" << endl;
      cerr << "  " << msg.getErrorStr() << endl;
      return (-1);
    }

    // make local copies of the data

    _copyChunkMembers(msg.getInfo().prod_id, msg.getInfo().prod_label,
		      msg.getNChunks(), msg.getChunkRefs(),
		      msg.getChunkData(), msg.getChunkDataLen());

  }

  return (0);

}
  
///////////////////////////////////////////////////////////////////
// getValid()
//
// Get data valid at the given time.
//

int Spdb::getValid(const string &url_str,
		   const time_t request_time,
		   const int data_type /* = 0*/ )

{

  // decode URL
  
  if (_checkUrl(url_str)) {
    return (-1);
  }

  if (_isLocal) {

    spdb_handle_t handle;
    if (SPDB_init(&handle, NULL, _prodId, (char *) _dirPath.c_str())) {
      cerr << "ERROR - Spdb::getValid" << endl;
      cerr << "Cannot init SPDB_handle." << endl;
      return (-1);
    }
  
    si32 nChunks;
    spdb_chunk_ref_t *chunkRefs;
    void *chunkData;

    if (SPDB_fetch_valid(&handle, data_type, request_time,
			 &nChunks, &chunkRefs, &chunkData)) {
      cerr << "ERROR - Spdb::getValid" << endl;
      cerr << "Fetch failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    // make local copies of the data
    
    _copyChunkMembers(handle.prod_id, handle.prod_label,
		      nChunks, chunkRefs,
		      chunkData, SPDB_chunk_data_len(&handle));

    // free up handle

    SPDB_free(&handle);

  } else {

    // create and assemble message

    SpdbMsg msg(DsMessage::PointToMem);
    void *buf =
      msg.assembleGetValid(url_str, request_time, data_type);
    
    // communicate with server. Message memory is owned by socket,
    // so this must stay in scope until after the copy

    ThreadSocket sock;
    if (_communicateGet(sock, msg, buf, msg.lengthAssembled())) {
      cerr << "ERROR - Spdb::getValid" << endl;
      cerr << "  Communicating with server." << endl;
      return (-1);
    }

    // check for error
    
    if (msg.errorOccurred()) {
      cerr << "ERROR - Spdb::getValid" << endl;
      cerr << "  " << msg.getErrorStr() << endl;
      return (-1);
    }

    // make local copies of the data

    _copyChunkMembers(msg.getInfo().prod_id, msg.getInfo().prod_label,
		      msg.getNChunks(), msg.getChunkRefs(),
		      msg.getChunkData(), msg.getChunkDataLen());

  }

  return (0);

}
  
///////////////////////////////////////////////////////////////////
// getLatest()
//
// Get latest data.
//

int Spdb::getLatest(const string &url_str,
		    const int time_margin /* = 0*/,
		    const int data_type /* = 0*/ )

{

  // decode URL
  
  if (_checkUrl(url_str)) {
    return (-1);
  }

  if (_isLocal) {

    spdb_handle_t handle;
    if (SPDB_init(&handle, NULL, _prodId, (char *) _dirPath.c_str())) {
      cerr << "ERROR - Spdb::getLatest" << endl;
      cerr << "Cannot init SPDB_handle." << endl;
      return (-1);
    }
  
    si32 lastValid;
    if (SPDB_last_valid_time(&handle, &lastValid)) {
      cerr << "ERROR - Spdb::getLatest" << endl;
      cerr << "SPDB_last_valid_time failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    si32 nChunks;
    spdb_chunk_ref_t *chunkRefs;
    void *chunkData;

    if (SPDB_fetch_interval(&handle, data_type,
			    lastValid - time_margin,
			    lastValid + time_margin,
			    &nChunks, &chunkRefs, &chunkData)) {
      cerr << "ERROR - Spdb::getLatest" << endl;
      cerr << "Fetch failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    // make local copies of the data
    
    _copyChunkMembers(handle.prod_id, handle.prod_label,
		      nChunks, chunkRefs,
		      chunkData, SPDB_chunk_data_len(&handle));

    // free up handle

    SPDB_free(&handle);

  } else {

    // create and assemble message
    
    SpdbMsg msg(DsMessage::PointToMem);
    void *buf =
      msg.assembleGetLatest(url_str, time_margin, data_type);

    // communicate with server. Message memory is owned by socket,
    // so this must stay in scope until after the copy

    ThreadSocket sock;
    if (_communicateGet(sock, msg, buf, msg.lengthAssembled())) {
      cerr << "ERROR - Spdb::getLatest" << endl;
      cerr << "  Communicating with server." << endl;
      return (-1);
    }

    // check for error
    
    if (msg.errorOccurred()) {
      cerr << "ERROR - Spdb::getLatest" << endl;
      cerr << "  " << msg.getErrorStr() << endl;
      return (-1);
    }

    // make local copies of the data
    
    _copyChunkMembers(msg.getInfo().prod_id, msg.getInfo().prod_label,
		      msg.getNChunks(), msg.getChunkRefs(),
		      msg.getChunkData(), msg.getChunkDataLen());

  }

  return (0);

}

///////////////////////////////////////////////////////////////////
// getFirstBefore()
//
// Get first data at or before the requested time.
//

int Spdb::getFirstBefore(const string &url_str,
			 const time_t request_time,
			 const int time_margin,
			 const int data_type /* = 0*/ )

{

  // decode URL
  
  if (_checkUrl(url_str)) {
    return (-1);
  }

  if (_isLocal) {

    spdb_handle_t handle;
    if (SPDB_init(&handle, NULL, _prodId, (char *) _dirPath.c_str())) {
      cerr << "ERROR - Spdb::getFirstBefore" << endl;
      cerr << "Cannot init SPDB_handle." << endl;
      return (-1);
    }
  
    si32 nChunks;
    spdb_chunk_ref_t *chunkRefs;
    void *chunkData;

    if (SPDB_fetch_first_before(&handle, data_type, request_time, time_margin,
				&nChunks, &chunkRefs, &chunkData)) {
      cerr << "ERROR - Spdb::getFirstBefore" << endl;
      cerr << "Fetch failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    // make local copies of the data
    
    _copyChunkMembers(handle.prod_id, handle.prod_label,
		      nChunks, chunkRefs,
		      chunkData, SPDB_chunk_data_len(&handle));

    // free up handle

    SPDB_free(&handle);

  } else {

    // create and assemble message

    SpdbMsg msg(DsMessage::PointToMem);
    void *buf = msg.assembleGetFirstBefore(url_str, request_time,
					   time_margin, data_type);
    
    // communicate with server. Message memory is owned by socket,
    // so this must stay in scope until after the copy

    ThreadSocket sock;
    if (_communicateGet(sock, msg, buf, msg.lengthAssembled())) {
      cerr << "ERROR - Spdb::getFirstBefore" << endl;
      cerr << "  Communicating with server." << endl;
      return (-1);
    }

    // check for error
    
    if (msg.errorOccurred()) {
      cerr << "ERROR - Spdb::getFirstBefore" << endl;
      cerr << "  " << msg.getErrorStr() << endl;
      return (-1);
    }

    // make local copies of the data

    _copyChunkMembers(msg.getInfo().prod_id, msg.getInfo().prod_label,
		      msg.getNChunks(), msg.getChunkRefs(),
		      msg.getChunkData(), msg.getChunkDataLen());

  }

  return (0);

}
  
///////////////////////////////////////////////////////////////////
// getFirstAfter()
//
// Get first data at or after the requested time.
//

int Spdb::getFirstAfter(const string &url_str,
			const time_t request_time,
			const int time_margin,
			const int data_type /* = 0*/ )

{

  // decode URL
  
  if (_checkUrl(url_str)) {
    return (-1);
  }

  if (_isLocal) {

    spdb_handle_t handle;
    if (SPDB_init(&handle, NULL, _prodId, (char *) _dirPath.c_str())) {
      cerr << "ERROR - Spdb::getFirstAfter" << endl;
      cerr << "Cannot init SPDB_handle." << endl;
      return (-1);
    }
  
    si32 nChunks;
    spdb_chunk_ref_t *chunkRefs;
    void *chunkData;
    
    if (SPDB_fetch_first_after(&handle, data_type, request_time, time_margin,
			       &nChunks, &chunkRefs, &chunkData)) {
      cerr << "ERROR - Spdb::getFirstAfter" << endl;
      cerr << "Fetch failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    // make local copies of the data
    
    _copyChunkMembers(handle.prod_id, handle.prod_label,
		      nChunks, chunkRefs,
		      chunkData, SPDB_chunk_data_len(&handle));

    // free up handle

    SPDB_free(&handle);

  } else {

    // create and assemble message

    SpdbMsg msg(DsMessage::PointToMem);
    void *buf = msg.assembleGetFirstAfter(url_str, request_time,
					  time_margin, data_type);
    
    // communicate with server. Message memory is owned by socket,
    // so this must stay in scope until after the copy

    ThreadSocket sock;
    if (_communicateGet(sock, msg, buf, msg.lengthAssembled())) {
      cerr << "ERROR - Spdb::getFirstAfter" << endl;
      cerr << "  Communicating with server." << endl;
      return (-1);
    }

    // check for error
    
    if (msg.errorOccurred()) {
      cerr << "ERROR - Spdb::getFirstAfter" << endl;
      cerr << "  " << msg.getErrorStr() << endl;
      return (-1);
    }

    // make local copies of the data

    _copyChunkMembers(msg.getInfo().prod_id, msg.getInfo().prod_label,
		      msg.getNChunks(), msg.getChunkRefs(),
		      msg.getChunkData(), msg.getChunkDataLen());

  }

  return (0);

}
  
////////////////////////////////////////////////////////////
// get the first, last and last_valid_time in the data base
//
// In this case, no chunk data is returned.

int Spdb::getTimes(const string &url_str,
		   time_t &first_time,
		   time_t &last_time,
		   time_t &last_valid_time)
  
{

  // decode URL
  
  if (_checkUrl(url_str)) {
    return (-1);
  }

  if (_isLocal) {

    spdb_handle_t handle;
    if (SPDB_init(&handle, NULL, _prodId, (char *) _dirPath.c_str())) {
      cerr << "ERROR - Spdb::getTimes" << endl;
      cerr << "Cannot init SPDB_handle." << endl;
      return (-1);
    }
  
    si32 first, last;
    si32 lastValid;

    if (SPDB_first_and_last_times(&handle, &first, &last)) {
      cerr << "ERROR - Spdb::getTimes" << endl;
      cerr << "SPDB_first_and_last_times failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    if (SPDB_last_valid_time(&handle, &lastValid)) {
      cerr << "ERROR - Spdb::getTimes" << endl;
      cerr << "SPDB_last_valid_time failed." << endl;
      SPDB_free(&handle);
      return (-1);
    }

    // set times

    first_time = first;
    last_time = last;
    last_valid_time = lastValid;

    // set prod id and label

    _prodId = handle.prod_id;
    _prodLabel = handle.prod_label;

    // free up handle

    SPDB_free(&handle);

  } else {

    // create and assemble message

    SpdbMsg msg(DsMessage::PointToMem);
    void *buf = msg.assembleGetTimes(url_str);
    
    // communicate with server. Message memory is owned by socket,
    // so this must stay in scope until after the copy

    ThreadSocket sock;
    if (_communicateGet(sock, msg, buf, msg.lengthAssembled())) {
      cerr << "ERROR - Spdb::getTimes" << endl;
      cerr << "  Communicating with server." << endl;
      return (-1);
    }

    // check for error
    
    if (msg.errorOccurred()) {
      cerr << "ERROR - Spdb::getTimes" << endl;
      cerr << "  " << msg.getErrorStr() << endl;
      return (-1);
    }

    // set times

    SpdbMsg::info_t &info = msg.getInfo();
    first_time = info.start_time;
    last_time = info.end_time;
    last_valid_time = info.last_valid_time;

    // set prod id and label

    _prodId = msg.getInfo().prod_id;
    _prodLabel = msg.getInfo().prod_label;

  }

  return (0);

}
  
//////////////////////////////////////////////////////////////
// _genericPut
//
// Generic put routine - all puts call this routine

int Spdb::_genericPut(const string &url_str,
		      const int prod_id,
		      const string &prod_label)

{

  spdb_chunk_ref_t *chunk_refs =
    (spdb_chunk_ref_t *) _chunkRefBuf.getBufPtr();

  void *chunk_data = _chunkDataBuf.getBufPtr();

  // decode URL

  if (_checkUrl(url_str)) {
    return (-1);
  }

  if (_isLocal) {

    if (_doLocalPut(prod_id, prod_label,
		    _nChunks, chunk_refs, chunk_data)) {
      return (-1);
    }

  } else {

    // not local

    // create args object for thread
    
    SpdbMsg::mode_enum_t mode;
    if (_putMode == putModeOver) {
      mode = SpdbMsg::DS_SPDB_PUT_MODE_OVER;
    } else if (_putMode == putModeAdd) {
      mode = SpdbMsg::DS_SPDB_PUT_MODE_ADD;
    } else {
      mode = SpdbMsg::DS_SPDB_PUT_MODE_ONCE;
    }

    SpdbPutArgs *putArgs =
      new SpdbPutArgs(url_str, prod_id, prod_label, mode,
		      _nChunks, chunk_refs, chunk_data);

    if (_putThreading) {

      if (_threadCount > _maxNThreads) {
	cerr << "ERROR - SpdbPut" << endl;
	cerr << "  Too many threads, max allowed: " << _maxNThreads << endl;
	delete (putArgs);
	return (-1);
      }

      // spawn a thread for doing the put
      
      pthread_t thread;
      int iret;
      if ((iret = pthread_create(&thread, NULL,
				 _doRemotePut, (void *) putArgs))) {
	cerr << "ERROR - SpdbPut" << endl;
	cerr << "  Cannot create thread." << endl;
	cerr << "  " << strerror(iret) << endl;
	delete (putArgs);
	return (-1);
      }
      pthread_detach(thread);
      
    } else {
      
      // no threading call function directly
      
      if(_doRemotePut(putArgs) != NULL) {
	return (-1);
      }
      
    }

  }

  return (0);

}

/////////////////////////////////////////
// function for processing put locally

int Spdb::_doLocalPut(const int prod_id,
		      const string &prod_label,
		      const int n_chunks,
		      const spdb_chunk_ref_t *chunk_refs,
		      const void *chunk_data)
  
{

  spdb_handle_t handle;
  if (SPDB_init(&handle, (char *) prod_label.c_str(),
		prod_id, (char *) _dirPath.c_str())) {
    cerr << "ERROR - SpdbPut" << endl;
    cerr << "Cannot init SPDB_handle." << endl;
    return (-1);
  }
  
  spdb_chunk_ref_t *ref = (spdb_chunk_ref_t *) chunk_refs;
  for (int i = 0; i < n_chunks; i++, ref++) {
    if (_putMode == putModeOver) {
      if (SPDB_store_over(&handle, ref->data_type,
			  ref->valid_time, ref->expire_time,
			  ((char *) chunk_data + ref->offset),
			  ref->len)) {
	SPDB_free(&handle);
	return (-1);
      }
    } else if (_putMode == putModeAdd) {
      if (SPDB_store_add(&handle, ref->data_type,
			 ref->valid_time, ref->expire_time,
			 ((char *) chunk_data + ref->offset),
			 ref->len)) {
	SPDB_free(&handle);
	return (-1);
      }
    } else {
      if (SPDB_store(&handle, ref->data_type,
		     ref->valid_time, ref->expire_time,
		     ((char *) chunk_data + ref->offset),
		     ref->len)) {
	SPDB_free(&handle);
	return (-1);
      }
    }
  } // i
  
  SPDB_free(&handle);

  return (0);

}

 
////////////////////////////////////////////////
// function for processing remote put in thread

void *Spdb::_doRemotePut(void *args)
  
{

  _threadCount++;
  
  // args is owned by the thread, and must be deleted
  // before exiting to avoid a leak.
  
  SpdbPutArgs *pArgs = (SpdbPutArgs *) args;

  // URL object interprets the URL

  DsURL url(pArgs->urlStr);
  
  // resolve the port if needed
  
  if (url.getPort() < 0) {
    DsSvrMgrSocket mgrSock;
    string errorStr;
    if (mgrSock.findPortForURL(url.getHost().c_str(), url,
			       -1, errorStr)) {
      cerr << "ERROR - Spdb::_doRemotePut" << endl;
      cerr << "  Cannot resolve port from ServerMgr" << endl;
      cerr << "  " << errorStr << endl;
      delete (pArgs);
      _threadCount--;
      return ((void *) -1);
    }
  }

  // open socket

  ThreadSocket sock;
  if (sock.open(url.getHost().c_str(), url.getPort())) {
    cerr << "ERROR - Spdb::_doRemotePut" << endl;
    cerr << "  Trying to connect to server" << endl;
    cerr << "  host: " << url.getHost() << endl;
    cerr << "  port: " << url.getPort() << endl;
    cerr << sock.getErrString() << endl;
    delete (pArgs);
    _threadCount--;
    return ((void *) -1);
  }

  // assemble the message

  SpdbMsg msg(DsMessage::PointToMem);

  void *buf = msg.assemblePut(pArgs->urlStr,
			      pArgs->prodId,
			      pArgs->prodLabel,
			      pArgs->putMode,
			      pArgs->nChunks,
			      pArgs->chunkRefs,
			      pArgs->chunkData,
			      pArgs->chunkDataLen);

  // communicate with server, read reply

  if (_communicatePut(sock, msg, buf, msg.lengthAssembled(), url)) {
    cerr << "ERROR - Spdb::_doRemotePut" << endl;
    cerr << "  Cannot communicate with server" << endl;
    delete (pArgs);
    sock.close();
    _threadCount--;
    return ((void *) -1);
  }

  // check for error

  if (msg.errorOccurred()) {
    cerr << "ERROR - Spdb::_doRemotePut" << endl;
    cerr << "  " << msg.getErrorStr() << endl;
    delete (pArgs);
    sock.close();
    _threadCount--;
    return ((void *) -1);
  }

  delete (pArgs);
  sock.close();
  _threadCount--;
  return (NULL);

}

 
/////////////////////////////////////////////
// communicate put message to server,
// and read reply.
//
// Returns 0 on success, -1 on error.

int Spdb::_communicatePut(ThreadSocket &sock,
			  SpdbMsg &msg,
			  void *buf,
			  const int buflen,
			  DsURL &url)

{

  // write the message
  
  if (sock.writeMessage(SpdbMsg::DS_MESSAGE_TYPE_SPDB,
		   buf, buflen, 10000)) {
    cerr << "ERROR - Spdb::_communicatePut" << endl;
    cerr << "  Cannot send message to server." << endl;
    cerr << "  host: " << url.getHost() << endl;
    cerr << "  port: " << url.getPort() << endl;
    cerr << sock.getErrString() << endl;
    return(-1);
  }
  
  // read the reply

  if (sock.readMessage()) {
    cerr << "ERROR - Spdb::_communicatePut" << endl;
    cerr << "  Cannot read reply from server." << endl;
    cerr << "  host: " << url.getHost() << endl;
    cerr << "  port: " << url.getPort() << endl;
    cerr << sock.getErrString() << endl;
    return(-1);
  }

  // disassemble the reply
  
  if (msg.disassemble((void *) sock.getData(), sock.getNumBytes())) {
    cerr << "ERROR - Spdb::_communicatePut" << endl;
    cerr << "Invalid reply" << endl;
    return(-1);
  }
  
  return (0);

}

/////////////////////////////////////////////
// communicate get message to server,
// and read reply
//
// Returns 0 on success, -1 on error.

int Spdb::_communicateGet(ThreadSocket &sock,
			  SpdbMsg &msg,
			  void *buf,
			  const int buflen)

{

  // open socket
  
  if (sock.open(_url.getHost().c_str(),	_url.getPort())) {
    cerr << "ERROR - Spdb::_communicateGet" << endl;
    cerr << "  Trying to connect to server" << endl;
    cerr << "  host: " << _url.getHost() << endl;
    cerr << "  port: " << _url.getPort() << endl;
    cerr << sock.getErrString() << endl;
    return (-1);
  }

  // write the message
  
  if (sock.writeMessage(SpdbMsg::DS_MESSAGE_TYPE_SPDB,
			buf, buflen, 10000)) {
    cerr << "ERROR - Spdb::_communicateGet" << endl;
    cerr << "  Cannot send message to server." << endl;
    cerr << "  host: " << _url.getHost() << endl;
    cerr << "  port: " << _url.getPort() << endl;
    cerr << sock.getErrString() << endl;
    sock.close();
    return(-1);
  }
  
  // read the reply

  if (sock.readMessage()) {
    cerr << "ERROR - Spdb::_communicateGet" << endl;
    cerr << "  Cannot read reply from server." << endl;
    cerr << "  host: " << _url.getHost() << endl;
    cerr << "  port: " << _url.getPort() << endl;
    cerr << sock.getErrString() << endl;
    sock.close();
    return(-1);
  }

  // close socket
  
  sock.close();

  // disassemble the reply
  
  if (msg.disassemble((void *) sock.getData(), sock.getNumBytes())) {
    cerr << "ERROR - Spdb::_communicateGet" << endl;
    cerr << "Invalid reply" << endl;
    return(-1);
  }
  
  return (0);

}

//////////////////////////////////////////////////////////////
// _checkUrl
//
// Decode and check the URL.
// 
// Side effects:
//
//  * decides if it is local or not, sets _isLocal accordingly.
//  * if local, sets the _dirPath from the URL.
//
// Returns 0 on success, -1 on failure.

int Spdb::_checkUrl(const string &url_str)

{
  bool  contactServer;

  //
  // Operate off of the fully resolved url, not just the one passed in
  //
  _url.setURLStr(url_str);

  if ( DsLocator.resolve( _url, &contactServer ) != 0 ) {
    cerr << "ERROR - Spdb" << endl;
    cerr << "Invalid URL: '" << url_str << "'" << endl;
    return (-1);
  }
  
  // determine if host is local
  _isLocal = !contactServer;
  _dirPath = _url.getFile();

  if (_isLocal) {
    RapDataDir.fillPath(_url.getFile(), _dirPath);
  }
  
  return (0);
}


///////////////////////////////////////
// copy handle members to local memory

void Spdb::_copyChunkMembers(int prod_id,
			     char *prod_label,
			     int n_chunks,
			     spdb_chunk_ref_t *chunk_refs,
			     void *chunk_data,
			     int chunk_data_len)

{

  _prodId = prod_id;
  _prodLabel = prod_label;

  _nChunks = n_chunks;

  _chunkRefBuf.reset();
  _chunkRefs = (spdb_chunk_ref_t *)
    _chunkRefBuf.add(chunk_refs, _nChunks * sizeof(spdb_chunk_ref_t));

  _chunkDataBuf.reset();
  _chunkData =
    _chunkDataBuf.add(chunk_data, chunk_data_len);

  _chunks.reserve(_nChunks);
  spdb_chunk_ref_t *ref = _chunkRefs;
  for (int i = 0; i < _nChunks; i++, ref++) {
    chunk_t &chunk = _chunks[i];
    chunk.valid_time = ref->valid_time;
    chunk.expire_time = ref->expire_time;
    chunk.data_type = ref->data_type;
    chunk.len = ref->len;
    chunk.data = (void *) ((char *) _chunkData + ref->offset);
  }

}


