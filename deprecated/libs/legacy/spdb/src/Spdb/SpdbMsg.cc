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
// SpdbMsg.cc
//
// SpdbMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////
//
// The SpdbMsg object provides the message protocol for
// the Spdb service.
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <spdb/SpdbMsg.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <rapmath/math_macros.h>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

SpdbMsg::SpdbMsg(memModel_t mem_model /* = CopyMem */) :
  DsServerMsg(mem_model)

{
}

////////////////////////////////////////////////////////////
// destructor

SpdbMsg::~SpdbMsg()

{
}

///////////////////////////////////////////////
// assemble a DS_SPDB_PUT message
//
  
void *SpdbMsg::assemblePut(const string &url_str,
			   const int prod_id,
			   const string &prod_label,
			   const mode_enum_t mode,
			   const int n_chunks,
			   const spdb_chunk_ref_t *refs,
			   const void *chunk_data,
			   const int chunk_data_len)

{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_PUT, mode);

  // indicate that this is the start of a series of put messages
  
  setCategory(StartPut);
  
  // load up local _info to BE byte order

  memset(&_info, 0, sizeof(info_t));
  _info.prod_id = prod_id;
  _info.n_chunks = n_chunks;
  STRncopy(_info.prod_label, prod_label.c_str(), SPDB_LABEL_MAX);
  _BEfromInfo();

  // load up local refs to BE byte order

  int refNbytes = n_chunks * sizeof(spdb_chunk_ref_t);
  _chunkRefs = (spdb_chunk_ref_t *) _refBuf.prepare(refNbytes);
  memcpy(_chunkRefs, refs, refNbytes);
  BE_to_array_32(_chunkRefs, refNbytes);

  // clear message parts

  clearParts();

  // add parts
  
  addPart(DS_SPDB_URL_PART, url_str.size() + 1, url_str.c_str());
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &_info);
  addPart(DS_SPDB_CHUNK_REF_PART, refNbytes, _chunkRefs);
  addPart(DS_SPDB_CHUNK_DATA_PART, chunk_data_len, chunk_data);
  
  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble put return
//

void *SpdbMsg::assemblePutReturn(const bool errorOccurred /* = false*/,
				 const char *errorStr /* = NULL*/ )
  
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_PUT_RETURN, _mode,
	     errorOccurred? -1 : 0);
  
  // indicate that this is the end of a series of messages
  
  setCategory(EndSeries);
  
  // clear message parts

  clearParts();

  // add parts

  if (errorOccurred && errorStr != NULL) {
    addPart(DS_SPDB_ERRORSTR_PART,
	    strlen(errorStr) + 1, errorStr);
  }

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble DS_SPDB_GET messages
//
  
void *SpdbMsg::assembleGetExact(const string &url_str,
				time_t request_time,
				int data_type)
  
{
  
  // load up local _info

  memset(&_info, 0, sizeof(info_t));
  _info.request_time = request_time;
  _info.data_type = data_type;

  return (_assembleGetGeneric(url_str, DS_SPDB_GET_MODE_EXACT));

}

void *SpdbMsg::assembleGetClosest(const string &url_str,
				  time_t request_time,
				  int time_margin,
				  int data_type)
  
{
  
  // load up local _info

  memset(&_info, 0, sizeof(info_t));
  _info.request_time = request_time;
  _info.time_margin = time_margin;
  _info.data_type = data_type;

  return (_assembleGetGeneric(url_str, DS_SPDB_GET_MODE_CLOSEST));

}

void *SpdbMsg::assembleGetInterval(const string &url_str,
				   time_t start_time,
				   time_t end_time,
				   int data_type)
  
{
  
  // load up local _info

  memset(&_info, 0, sizeof(info_t));
  _info.start_time = start_time;
  _info.end_time = end_time;
  _info.data_type = data_type;

  return (_assembleGetGeneric(url_str, DS_SPDB_GET_MODE_INTERVAL));

}

void *SpdbMsg::assembleGetValid(const string &url_str,
				time_t request_time,
				int data_type)
  
{
  
  // load up local _info

  memset(&_info, 0, sizeof(info_t));
  _info.request_time = request_time;
  _info.data_type = data_type;

  return (_assembleGetGeneric(url_str, DS_SPDB_GET_MODE_VALID));

}

void *SpdbMsg::assembleGetLatest(const string &url_str,
				 int time_margin,
				 int data_type)
  
{
  
  // load up local _info

  memset(&_info, 0, sizeof(info_t));
  _info.time_margin = time_margin;
  _info.data_type = data_type;

  return (_assembleGetGeneric(url_str, DS_SPDB_GET_MODE_LATEST));

}

void *SpdbMsg::assembleGetFirstBefore(const string &url_str,
				      time_t request_time,
				      int time_margin,
				      int data_type)
  
{
  
  // load up local _info

  memset(&_info, 0, sizeof(info_t));
  _info.request_time = request_time;
  _info.time_margin = time_margin;
  _info.data_type = data_type;

  return (_assembleGetGeneric(url_str, DS_SPDB_GET_MODE_FIRST_BEFORE));

}

void *SpdbMsg::assembleGetFirstAfter(const string &url_str,
				     time_t request_time,
				     int time_margin,
				     int data_type)
  
{
  
  // load up local _info

  memset(&_info, 0, sizeof(info_t));
  _info.request_time = request_time;
  _info.time_margin = time_margin;
  _info.data_type = data_type;

  return (_assembleGetGeneric(url_str, DS_SPDB_GET_MODE_FIRST_AFTER));

}

void *SpdbMsg::assembleGetTimes(const string &url_str)
  
{
  
  // load up local _info

  memset(&_info, 0, sizeof(info_t));

  return (_assembleGetGeneric(url_str, DS_SPDB_GET_MODE_TIMES));

}

///////////////////////////////////////////////////////
// assemble a DS_SPDB_GET DATA success return message
//
  
void *SpdbMsg::assembleGetDataSuccessReturn(const info_t &info,
					    const spdb_chunk_ref_t *refs,
					    const void *chunk_data)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_GET_RETURN, _mode);
  
  // indicate that this is the end of a series of messages
  
  setCategory(EndSeries);
  
  // load up local _info to BE byte order

  _info = info;
  _BEfromInfo();

  // load up local refs to BE byte order

  int refNbytes = info.n_chunks * sizeof(spdb_chunk_ref_t);
  _chunkRefs = (spdb_chunk_ref_t *) _refBuf.prepare(refNbytes);
  memcpy(_chunkRefs, refs, refNbytes);
  BE_to_array_32(_chunkRefs, refNbytes);

  // compute data length

  int chunkDataLen = 0;
  for (int i = 0; i < info.n_chunks; i++) {
    int endPos = refs[i].offset + refs[i].len;
    chunkDataLen = MAX(chunkDataLen, endPos);
  }

  // clear message parts

  clearParts();

  // add parts
  
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &_info);
  addPart(DS_SPDB_CHUNK_REF_PART, refNbytes, _chunkRefs);
  addPart(DS_SPDB_CHUNK_DATA_PART, chunkDataLen, chunk_data);
  
  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////////////////
// assemble a DS_SPDB_GET TIMES-mode success return message
//
  
void *SpdbMsg::assembleGetTimesSuccessReturn(const info_t &info)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_GET_RETURN, _mode);
  
  // indicate that this is the end of a series of messages
  
  setCategory(EndSeries);
  
  // load up local _info to BE byte order

  _info = info;
  _BEfromInfo();

  // clear message parts

  clearParts();

  // add parts
  
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &_info);
  
  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble get error return
//

void *SpdbMsg::assembleGetErrorReturn(const char *errorStr /* = NULL*/ )
  
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_GET_RETURN, _mode, -1);
  
  // indicate that this is the end of a series of messages
  
  setCategory(EndSeries);
  
  // clear message parts

  clearParts();

  // add parts
  
  if (errorStr != NULL) {
    addPart(DS_SPDB_ERRORSTR_PART,
	    strlen(errorStr) + 1, errorStr);
  }

  // assemble
  
  return (DsMessage::assemble());

}

/////////////////////////////////////
// override the disassemble function
//

int SpdbMsg::disassemble(void *in_msg, const int msg_len)

{

  // initialize
  
  _urlStr = "\0";
  _errorStr = "\0";
  _errorOccurred = false;
  _clearInfo();
  _nChunks = 0;
  _chunkRefs = NULL;
  _chunkData = NULL; 

  // peek at the header to make sure we're looking at the
  // right type of message

  if (decodeHeader(in_msg, msg_len)) {
    cerr << "ERROR - SpdbMsg::disassemble" << endl;
    cerr << "Bad message header" << endl;
    cerr << "Message len: " << msg_len << endl;
    return (-1);
  }

  if (_type != DS_MESSAGE_TYPE_SPDB) {
    cerr << "ERROR - SpdbMsg::disassemble" << endl;
    cerr << "Unknown message type: " << _type << endl;
    cerr << "Message len: " << msg_len << endl;
    printHeader(&cerr, "");
    return (-1);
  }

  // disassemble the message using the base-class routine
  
  if (DsMessage::disassemble(in_msg, msg_len)) {
    cerr << "ERROR - SpdbMsg::disassemble" << endl;
    cerr << "ERROR in DsMessage::disassemble()" << endl;
    return (-1);
  }
  if (_flags) {
    _errorOccurred = true;
  }

  // set data members to parts

  if (partExists(DS_SPDB_URL_PART)) {
    _urlStr = (char *) getPartByType(DS_SPDB_URL_PART)->getBuf();
  }
  if (partExists(DS_SPDB_ERRORSTR_PART)) {
    _errorStr = (char *) getPartByType(DS_SPDB_ERRORSTR_PART)->getBuf();
  }
  if (partExists(DS_SPDB_INFO_PART)) {
    memcpy(&_info, getPartByType(DS_SPDB_INFO_PART)->getBuf(),
	   sizeof(info_t));
    _BEtoInfo();
    _nChunks = _info.n_chunks;
  }
  if (partExists(DS_SPDB_CHUNK_REF_PART)) {
    _chunkRefs = (spdb_chunk_ref_t *)
      getPartByType(DS_SPDB_CHUNK_REF_PART)->getBuf();
    BE_from_array_32(_chunkRefs, _info.n_chunks * sizeof(spdb_chunk_ref_t));
  }
  if (partExists(DS_SPDB_CHUNK_DATA_PART)) {
    DsMsgPart *part = getPartByType(DS_SPDB_CHUNK_DATA_PART);
    _chunkData = (void *) part->getBuf();
    _chunkDataLen = part->getLength();
  }

  return (0);

}

////////////////
// print message
//

void SpdbMsg::print(ostream &out)

{

  switch (_subType) {
    
  case DS_SPDB_PUT:
    out << "Message subType: DS_SPDB_PUT" << endl;
    out << "  url: " << _urlStr << endl;
    switch (_mode) {
    case DS_SPDB_PUT_MODE_OVER:
      out << "  mode: DS_SPDB_PUT_MODE_OVER" << endl;
      break;
    case DS_SPDB_PUT_MODE_ADD:
      out << "  mode: DS_SPDB_PUT_MODE_ADD" << endl;
      break;
    case DS_SPDB_PUT_MODE_ONCE:
      out << "  mode: DS_SPDB_PUT_MODE_ONCE" << endl;
      break;
    default:
      break;
    }
    out << "  nChunks: " << _info.n_chunks << endl;
    break;

  case DS_SPDB_PUT_RETURN:
    out << "Message subType: DS_SPDB_PUT_RETURN" << endl;
    if (_flags) {
      out << "  error occurred." << endl;
      out << _errorStr << endl;
    }
    break;

  case DS_SPDB_GET:
    out << "Message subType: DS_SPDB_GET" << endl;
    out << "  url: " << _urlStr << endl;
    switch (_mode) {
    case DS_SPDB_GET_MODE_EXACT:
      out << "  mode: DS_SPDB_GET_MODE_EXACT" << endl;
      out << "  request_time: " << utimstr(_info.request_time) << endl;
      break;
    case DS_SPDB_GET_MODE_CLOSEST:
      out << "  mode: DS_SPDB_GET_MODE_CLOSEST" << endl;
      out << "  request_time: " << utimstr(_info.request_time) << endl;
      out << "  time_margin: " << _info.time_margin << endl;
      break;
    case DS_SPDB_GET_MODE_INTERVAL:
      out << "  mode: DS_SPDB_GET_MODE_INTERVAL" << endl;
      out << "  start_time: " << utimstr(_info.start_time) << endl;
      out << "  end_time: " << utimstr(_info.end_time) << endl;
      break;
    case DS_SPDB_GET_MODE_VALID:
      out << "  mode: DS_SPDB_GET_MODE_VALID" << endl;
      out << "  request_time: " << utimstr(_info.request_time) << endl;
      break;
    case DS_SPDB_GET_MODE_LATEST:
      out << "  mode: DS_SPDB_GET_MODE_LATEST" << endl;
      out << "  time_margin: " << _info.time_margin << endl;
      break;
    case DS_SPDB_GET_MODE_FIRST_BEFORE:
      out << "  mode: DS_SPDB_GET_MODE_FIRST_BEFORE" << endl;
      out << "  request_time: " << utimstr(_info.request_time) << endl;
      break;
    case DS_SPDB_GET_MODE_FIRST_AFTER:
      out << "  mode: DS_SPDB_GET_MODE_FIRST_AFTER" << endl;
      out << "  request_time: " << utimstr(_info.request_time) << endl;
      break;
    case DS_SPDB_GET_MODE_TIMES:
      out << "  mode: DS_SPDB_GET_MODE_TIMES" << endl;
      break;
    default:
      break;
    }
    out << "  dataType: " << _info.data_type << endl;
    break;

  case DS_SPDB_GET_RETURN:
    out << "Message subType: DS_SPDB_GET_RETURN" << endl;
    switch (_mode) {
    case DS_SPDB_GET_MODE_EXACT:
      out << "  mode: DS_SPDB_GET_MODE_EXACT" << endl;
      break;
    case DS_SPDB_GET_MODE_CLOSEST:
      out << "  mode: DS_SPDB_GET_MODE_CLOSEST" << endl;
      break;
    case DS_SPDB_GET_MODE_INTERVAL:
      out << "  mode: DS_SPDB_GET_MODE_INTERVAL" << endl;
      break;
    case DS_SPDB_GET_MODE_VALID:
      out << "  mode: DS_SPDB_GET_MODE_VALID" << endl;
      break;
    case DS_SPDB_GET_MODE_LATEST:
      out << "  mode: DS_SPDB_GET_MODE_LATEST" << endl;
      break;
    case DS_SPDB_GET_MODE_FIRST_BEFORE:
      out << "  mode: DS_SPDB_GET_MODE_FIRST_BEFORE" << endl;
      break;
    case DS_SPDB_GET_MODE_FIRST_AFTER:
      out << "  mode: DS_SPDB_GET_MODE_FIRST_AFTER" << endl;
      break;
    case DS_SPDB_GET_MODE_TIMES:
      out << "  mode: DS_SPDB_GET_MODE_TIMES" << endl;
      out << "  start_time: " << utimstr(_info.start_time) << endl;
      out << "  end_time: " << utimstr(_info.end_time) << endl;
      out << "  last_valid_time: " << utimstr(_info.last_valid_time) << endl;
      break;
    default:
      break;
    }
    if (_flags) {
      out << "  error occurred." << endl;
      out << _errorStr << endl;
    }
    break;
    
  default:
    break;
    
  } // switch (_subType)

}

///////////////////////////////////////////////
// generic assemble for a DS_SPDB_GET message
//
  
void *SpdbMsg::_assembleGetGeneric(const string &url_str,
				   const mode_enum_t mode)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_GET, mode);

  // indicate that this is the start of a series of get messages
  
  setCategory(StartGet);
  
  // convert local _info to BE byte order

  _BEfromInfo();

  // clear message parts

  clearParts();

  // add parts
  
  addPart(DS_SPDB_URL_PART, url_str.size() + 1, url_str.c_str());
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &_info);
  
  // assemble
  
  return (DsMessage::assemble());

}

//////////////////////////////////
// Convert to BE from info struct
//

void SpdbMsg::_BEfromInfo()

{
  BE_from_array_32(&_info, sizeof(info_t) - SPDB_LABEL_MAX);
}
  
//////////////////////////////////
// Convert from BE to info struct
//

void SpdbMsg::_BEtoInfo()

{
  BE_to_array_32(&_info, sizeof(info_t) - SPDB_LABEL_MAX);
}

/////////////////////////  
// clear the info struct

void SpdbMsg::_clearInfo()

{
  memset(&_info, 0, sizeof(info_t));
}


