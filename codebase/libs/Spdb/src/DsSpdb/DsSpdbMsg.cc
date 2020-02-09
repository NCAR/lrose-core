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
// DsSpdbMsg.cc
//
// DsSpdbMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////
//
// The DsSpdbMsg object provides the message protocol for
// the Spdb service.
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <Spdb/DsSpdbMsg.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/compress.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

DsSpdbMsg::DsSpdbMsg(memModel_t mem_model /* = CopyMem */ ) :
  DsServerMsg(mem_model)

{
  _appName = "unknown";
  MEM_zero(_info);
  MEM_zero(_info2);
  MEM_zero(_horizLimits);
  MEM_zero(_vertLimits);
  _horizLimitsSet = false;
  _vertLimitsSet = false;
  clearData();
}

/////////////////////////////
// Copy constructor
//

DsSpdbMsg::DsSpdbMsg(const DsSpdbMsg &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

////////////////////////////////////////////////////////////
// destructor

DsSpdbMsg::~DsSpdbMsg()

{
}

//////////////////////////////
// Assignment
//

DsSpdbMsg &DsSpdbMsg::operator=(const DsSpdbMsg &rhs)
  
{
  return _copy(rhs);
}

////////////////////////////////////////////////////////////
// clear data members

void DsSpdbMsg::clearData()

{
  _errorOccurred = false;
  _errorStr.clear();
  _refBuf.free();
  _auxBuf.free();
  _dataBuf.free();
  _timeList.clear();
}

///////////////////////////////////////////////
// assemble a DS_SPDB_PUT message
//
// The chunk_data is passed in uncompressed.
// The desired compression is indicated by data_buf_compression.
  
void *DsSpdbMsg::assemblePut(const string &app_name,
			     const string &url_str,
			     int prod_id,
			     const string &prod_label,
			     mode_enum_t request_mode,
			     Spdb::lead_time_storage_t lead_time_storage,
			     int n_chunks,
                             const MemBuf &ref_buf,
                             const MemBuf &aux_buf,
                             const MemBuf &data_buf,
                             bool respect_zero_types,
                             Spdb::compression_t data_buf_compression)
  
{

  clearData();

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_PUT, request_mode);
  
  // indicate that this is the start of a series of put messages
  
  setCategory(StartPut);
  
  // url

  setUrlStr(url_str);

  // load up _info and _info2

  _info.prod_id = prod_id;
  _info.n_chunks = n_chunks;
  _info.respect_zero_types = respect_zero_types;
  STRncopy(_info.prod_label, prod_label.c_str(), SPDB_LABEL_MAX);
  
  _info2.lead_time_storage = lead_time_storage;
  _info2.data_buf_compression = Spdb::COMPRESSION_NONE;

  // load up refs

  _refBuf.concat(ref_buf);
  _auxBuf.concat(aux_buf);

  // load up data buffer, handle compression as appropriate
  
  _dataBuf.concat(data_buf);
  compressDataBuf(data_buf_compression);
  
  // clear message parts
  
  clearParts();

  // make local copies of relevant parts and byte-swap

  info_t info = _info;
  BEfromInfo(info);

  info2_t info2 = _info2;
  BEfromInfo2(info2);

  MemBuf refBuf(_refBuf);
  Spdb::chunk_refs_to_BE((Spdb::chunk_ref_t *) refBuf.getPtr(), n_chunks);

  MemBuf auxBuf(_auxBuf);
  Spdb::aux_refs_to_BE((Spdb::aux_ref_t *) auxBuf.getPtr(), n_chunks);

  // add parts
  
  addClientHost();
  addClientIpaddr();
  addClientUser();
  if (app_name.size() > 0) {
    addPart(DS_SPDB_APP_NAME_PART, app_name.size() + 1, app_name.c_str());
  }
  addPart(DS_SPDB_URL_PART, url_str.size() + 1, url_str.c_str());
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &info);
  addPart(DS_SPDB_INFO2_PART, sizeof(info2_t), &info2);
  addPart(DS_SPDB_CHUNK_REF_PART, refBuf.getLen(), refBuf.getPtr());
  addPart(DS_SPDB_AUX_REF_PART, auxBuf.getLen(), auxBuf.getPtr());
  if (_auxXml.size() > 0) {
    addPart(DS_SPDB_AUX_XML_PART, _auxXml.size() + 1, _auxXml.c_str());
  }
  addPart(DS_SPDB_CHUNK_DATA_PART, _dataBuf.getLen(), _dataBuf.getPtr());

  // assemble
  
  void *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "------------- DsSpdbMsg::assemblePut --------------" << endl;
    print(cerr);
    cerr << "---------------------------------------------------" << endl;
  }

  return msg;

}

///////////////////////////////////////////////
// assemble put return
//

void *DsSpdbMsg::assemblePutReturn(mode_enum_t request_mode,
                                   const bool errorOccurred /* = false*/,
				   const char *errorStr /* = NULL*/ )
  
{

  clearData();

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_PUT_RETURN, request_mode,
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
  
void *DsSpdbMsg::assembleGetExact(const string &url_str,
				  time_t request_time,
				  int data_type,
				  int data_type2,
				  int get_refs_only,
				  int respect_zero_types,
				  int get_unique,
                                  bool check_write_time_on_get,
                                  time_t latest_valid_write_time,
                                  Spdb::compression_t data_buf_compression)
{
  
  clearData();
  _urlStr = url_str;

  // load up local _info

  _info.request_time = request_time;
  _info.data_type = data_type;
  _info.data_type2 = data_type2;
  _info.get_refs_only = get_refs_only;
  _info.get_unique = get_unique;
  _info.respect_zero_types = respect_zero_types;

  _info2.check_write_time_on_get = check_write_time_on_get;
  _info2.latest_valid_write_time = latest_valid_write_time;
  _info2.data_buf_compression = data_buf_compression;

  return (_assembleGet(DS_SPDB_GET_MODE_EXACT));

}

void *DsSpdbMsg::assembleGetClosest(const string &url_str,
				    time_t request_time,
				    int time_margin,
				    int data_type,
				    int data_type2,
				    int get_refs_only,
				    int respect_zero_types,
				    int get_unique,
                                    bool check_write_time_on_get,
                                    time_t latest_valid_write_time,
                                    Spdb::compression_t data_buf_compression)
  
{
  
  clearData();
  _urlStr = url_str;

  // load up local _info

  _info.request_time = request_time;
  _info.time_margin = time_margin;
  _info.data_type = data_type;
  _info.data_type2 = data_type2;
  _info.get_refs_only = get_refs_only;
  _info.get_unique = get_unique;
  _info.respect_zero_types = respect_zero_types;

  _info2.check_write_time_on_get = check_write_time_on_get;
  _info2.latest_valid_write_time = latest_valid_write_time;
  _info2.data_buf_compression = data_buf_compression;

  return (_assembleGet(DS_SPDB_GET_MODE_CLOSEST));

}

void *DsSpdbMsg::assembleGetInterval(const string &url_str,
				     time_t start_time,
				     time_t end_time,
				     int data_type,
				     int data_type2,
				     int get_refs_only,
				     int respect_zero_types,
				     int get_unique,
                                     bool check_write_time_on_get,
                                     time_t latest_valid_write_time,
                                     Spdb::compression_t data_buf_compression)
  
{
  
  clearData();
  _urlStr = url_str;

  // load up local _info

  _info.start_time = start_time;
  _info.end_time = end_time;
  _info.data_type = data_type;
  _info.data_type2 = data_type2;
  _info.get_refs_only = get_refs_only;
  _info.get_unique = get_unique;
  _info.respect_zero_types = respect_zero_types;

  _info2.check_write_time_on_get = check_write_time_on_get;
  _info2.latest_valid_write_time = latest_valid_write_time;
  _info2.data_buf_compression = data_buf_compression;

  return (_assembleGet(DS_SPDB_GET_MODE_INTERVAL));

}

void *DsSpdbMsg::assembleGetValid(const string &url_str,
				  time_t request_time,
				  int data_type,
				  int data_type2,
				  int get_refs_only,
				  int respect_zero_types,
				  int get_unique,
                                  bool check_write_time_on_get,
                                  time_t latest_valid_write_time,
                                  Spdb::compression_t data_buf_compression)
  
{
  
  clearData();
  _urlStr = url_str;

  // load up local _info

  _info.request_time = request_time;
  _info.data_type = data_type;
  _info.data_type2 = data_type2;
  _info.get_refs_only = get_refs_only;
  _info.get_unique = get_unique;
  _info.respect_zero_types = respect_zero_types;

  _info2.check_write_time_on_get = check_write_time_on_get;
  _info2.latest_valid_write_time = latest_valid_write_time;
  _info2.data_buf_compression = data_buf_compression;

  return (_assembleGet(DS_SPDB_GET_MODE_VALID));

}

void *DsSpdbMsg::assembleGetLatest(const string &url_str,
				   int time_margin,
				   int data_type,
				   int data_type2,
				   int get_refs_only,
				   int respect_zero_types,
				   int get_unique,
                                   bool check_write_time_on_get,
                                   time_t latest_valid_write_time,
                                   Spdb::compression_t data_buf_compression)
  
{
  
  clearData();
  _urlStr = url_str;

  // load up local _info

  _info.time_margin = time_margin;
  _info.data_type = data_type;
  _info.data_type2 = data_type2;
  _info.get_refs_only = get_refs_only;
  _info.get_unique = get_unique;
  _info.respect_zero_types = respect_zero_types;

  _info2.check_write_time_on_get = check_write_time_on_get;
  _info2.latest_valid_write_time = latest_valid_write_time;
  _info2.data_buf_compression = data_buf_compression;

  return (_assembleGet(DS_SPDB_GET_MODE_LATEST));

}

void *DsSpdbMsg::assembleGetFirstBefore(const string &url_str,
					time_t request_time,
					int time_margin,
					int data_type,
					int data_type2,
					int get_refs_only,
					int respect_zero_types,
					int get_unique,
                                        bool check_write_time_on_get,
                                        time_t latest_valid_write_time,
                                        Spdb::compression_t data_buf_compression)
  
{
  
  clearData();
  _urlStr = url_str;

  // load up local _info

  _info.request_time = request_time;
  _info.time_margin = time_margin;
  _info.data_type = data_type;
  _info.data_type2 = data_type2;
  _info.get_refs_only = get_refs_only;
  _info.get_unique = get_unique;
  _info.respect_zero_types = respect_zero_types;

  _info2.check_write_time_on_get = check_write_time_on_get;
  _info2.latest_valid_write_time = latest_valid_write_time;
  _info2.data_buf_compression = data_buf_compression;
  
  return (_assembleGet(DS_SPDB_GET_MODE_FIRST_BEFORE));

}

void *DsSpdbMsg::assembleGetFirstAfter(const string &url_str,
				       time_t request_time,
				       int time_margin,
				       int data_type,
				       int data_type2,
				       int get_refs_only,
				       int respect_zero_types,
				       int get_unique,
                                       bool check_write_time_on_get,
                                       time_t latest_valid_write_time,
                                       Spdb::compression_t data_buf_compression)
  
{
  
  clearData();
  _urlStr = url_str;

  // load up local _info

  _info.request_time = request_time;
  _info.time_margin = time_margin;
  _info.data_type = data_type;
  _info.data_type2 = data_type2;
  _info.get_refs_only = get_refs_only;
  _info.get_unique = get_unique;

  _info2.check_write_time_on_get = check_write_time_on_get;
  _info2.latest_valid_write_time = latest_valid_write_time;
  _info2.data_buf_compression = data_buf_compression;

  return (_assembleGet(DS_SPDB_GET_MODE_FIRST_AFTER));
  
}

void *DsSpdbMsg::assembleGetTimes(const string &url_str,
                                  bool check_write_time_on_get,
                                  time_t latest_valid_write_time)
  
{
  
  clearData();
  _urlStr = url_str;

  // load up local _info
  
  _info2.check_write_time_on_get = check_write_time_on_get;
  _info2.latest_valid_write_time = latest_valid_write_time;

  return (_assembleGet(DS_SPDB_GET_MODE_TIMES));

}

///////////////////////////////////////////////////////
// assemble a DS_SPDB_GET DATA success return message
//
// The chunk_data is passed in uncompressed.
// The desired compression is indicated by data_buf_compression.


void *DsSpdbMsg::assembleGetDataSuccessReturn(mode_enum_t request_mode,
                                              const info_t &getInfo,
                                              const MemBuf &refBuf,
                                              const MemBuf &auxBuf,
                                              const MemBuf &dataBuf,
                                              Spdb::compression_t data_buf_compression)

{
  
  clearData();

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_GET_RETURN, request_mode);
  
  // indicate that this is the end of a series of messages
  
  setCategory(EndSeries);
  
  // load up _info

  _info = getInfo;
  _info2.data_buf_compression = Spdb::COMPRESSION_NONE;
  
  // load up refs
  
  _refBuf.free();
  _refBuf.concat(refBuf);
  
  _auxBuf.free();
  _auxBuf.concat(auxBuf);

  // load up data

  _dataBuf.free();
  _dataBuf.concat(dataBuf);
  
  // compress as required

  compressDataBuf(data_buf_compression);

  // create local copies, byte-swap

  info_t locInfo = _info;
  BEfromInfo(locInfo);

  info2_t locInfo2 = _info2;
  BEfromInfo2(locInfo2);

  MemBuf refCopy(_refBuf);
  Spdb::chunk_refs_to_BE((Spdb::chunk_ref_t *) refCopy.getPtr(), _info.n_chunks);

  MemBuf auxCopy(_auxBuf);
  Spdb::aux_refs_to_BE((Spdb::aux_ref_t *) auxCopy.getPtr(), _info.n_chunks);

  // clear message parts

  clearParts();
  
  // add parts
  
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &locInfo);
  addPart(DS_SPDB_INFO2_PART, sizeof(info2_t), &locInfo2);
  addPart(DS_SPDB_CHUNK_REF_PART, refCopy.getLen(), refCopy.getPtr());
  addPart(DS_SPDB_AUX_REF_PART, auxCopy.getLen(), auxCopy.getPtr());

  if (!_info.get_refs_only) {
    addPart(DS_SPDB_CHUNK_DATA_PART, _dataBuf.getLen(), _dataBuf.getPtr());
  }
  
  void *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "------ DsSpdbMsg::assembleGetDataSuccessReturn ---------" << endl;
    print(cerr);
    cerr << "--------------------------------------------------------" << endl;
  }

  return msg;

}

///////////////////////////////////////////////////////////
// assemble a DS_SPDB_GET TIMES-mode success return message
//
  
void *DsSpdbMsg::assembleGetTimesSuccessReturn(mode_enum_t request_mode,
                                               const info_t &info)
  
{
  
  clearData();

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_GET_RETURN, request_mode);
  
  // indicate that this is the end of a series of messages
  
  setCategory(EndSeries);
  
  // load up _info
  // convert copy into BE byte order

  _info = info;
  info_t locInfo(info);
  BEfromInfo(locInfo);

  // clear message parts

  clearParts();

  // add parts
  
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &locInfo);
  
  // assemble
  
  return (DsMessage::assemble());

}


///////////////////////////////////////////////////////////
// assemble a DS_SPDB_GET_MODE_TIME_LIST message
//
  
void *DsSpdbMsg::assembleCompileTimeList(const string &url_str,
					 time_t start_time,
					 time_t end_time,
					 size_t min_time_interval,
                                         bool check_write_time_on_get,
                                         time_t latest_valid_write_time)
  
{
  
  clearData();
  _urlStr = url_str;

  // load up local _info

  _info.start_time = start_time;
  _info.end_time = end_time;
  _info.time_margin = min_time_interval;
  
  _info2.check_write_time_on_get = check_write_time_on_get;
  _info2.latest_valid_write_time = latest_valid_write_time;

  return (_assembleGet(DS_SPDB_GET_MODE_TIME_LIST));
  
}

///////////////////////////////////////////////////////////////
// assemble a DS_SPDB_GET_MODE_TIME_LIST success return message
//
  
void *DsSpdbMsg::assembleCompileTimeListSuccessReturn
  (mode_enum_t request_mode,
   const info_t &info,
   const vector<time_t> &time_list)
  
{
  
  clearData();

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_GET_RETURN, request_mode);
  
  // indicate that this is the end of a series of messages
  
  setCategory(EndSeries);
  
  // load up _info
  // convert copy into BE byte order

  _info = info;
  info_t locInfo(info);
  BEfromInfo(locInfo);
  
  // load up time list in BE byte order
  
  int ntimes = time_list.size();
  TaArray<si32> timesArray;
  si32 *times = timesArray.alloc(ntimes);
  for (int i = 0; i < ntimes; i++) {
    times[i] = (int) time_list[i];
  }
  BE_from_array_32(times, ntimes * sizeof(si32));

  // clear message parts

  clearParts();

  // add parts
  
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &locInfo);
  addPart(DS_SPDB_TIME_LIST_PART, ntimes * sizeof(si32), times);
  
  void *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--- DsSpdbMsg::assembleCompileTimeListSuccessReturn ---" << endl;
    print(cerr);
    cerr << "-------------------------------------------------------" << endl;
  }

  return msg;

}

///////////////////////////////////////////////
// assemble get error return
//

void *DsSpdbMsg::assembleGetErrorReturn(mode_enum_t request_mode,
                                        const char *errorStr /* = NULL*/ )
  
{

  clearData();

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_GET_RETURN, request_mode, -1);
  
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
  
  void *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--------- DsSpdbMsg::assembleGetErrorReturn -----------" << endl;
    print(cerr);
    cerr << "-------------------------------------------------------" << endl;
  }

  return msg;

}

///////////////////////////////////////////////////////
// generic assemble using member data

void *DsSpdbMsg::assemble(int subtype,
                          int mode,
                          category_t category)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, subtype, mode);
  
  // indicate that this is the end of a series of messages
  
  setCategory(category);
  
  // convert info copy into BE byte order

  info_t locInfo(_info);
  BEfromInfo(locInfo);

  info2_t locInfo2(_info2);
  BEfromInfo2(locInfo2);
  
  // load up local refs to BE byte order
  
  MemBuf locRefBuf(_refBuf);
  Spdb::chunk_refs_to_BE((Spdb::chunk_ref_t *) locRefBuf.getPtr(), _info.n_chunks);

  MemBuf locAuxBuf(_auxBuf);
  Spdb::aux_refs_to_BE((Spdb::aux_ref_t *) locAuxBuf.getPtr(), _info.n_chunks);
  
  // clear message parts

  clearParts();
  
  // add parts

  addClientHost();
  addClientIpaddr();
  addClientUser();

  if (_appName.size() > 0) {
    addPart(DS_SPDB_APP_NAME_PART, _appName.size() + 1, _appName.c_str());
  }
  if (_urlStr.size() > 0) {
    addPart(DS_SPDB_URL_PART, _urlStr.size() + 1, _urlStr.c_str());
  }
  if (_errorOccurred) {
    if (_errorStr.size() > 0) {
      addPart(DS_SPDB_URL_PART, _errorStr.size() + 1, _errorStr.c_str());
    }
  }
  
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &locInfo);
  addPart(DS_SPDB_INFO2_PART, sizeof(info2_t), &locInfo2);
  
  if (locRefBuf.getLen() > 0) {
    addPart(DS_SPDB_CHUNK_REF_PART, locRefBuf.getLen(), locRefBuf.getPtr());
  }
  if (locAuxBuf.getLen() > 0) {
    addPart(DS_SPDB_AUX_REF_PART, locAuxBuf.getLen(), locAuxBuf.getPtr());
  }
  if (_auxXml.size() > 0) {
    addPart(DS_SPDB_AUX_XML_PART, _auxXml.size() + 1, _auxXml.c_str());
  }
  if (_dataBuf.getLen() > 0) {
    addPart(DS_SPDB_CHUNK_DATA_PART, _dataBuf.getLen(), _dataBuf.getPtr());
  }
  
  // assemble
  
  void *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "------------- DsSpdbMsg::assemble --------------" << endl;
    print(cerr);
    cerr << "------------------------------------------------" << endl;
  }

  return msg;

}

/////////////////////////////////////
// override the disassemble function
//

int DsSpdbMsg::disassemble(const void *in_msg, ssize_t msg_len,
                           bool delay_uncompression /* = false */)

{

  clearData();

  // peek at the header to make sure we're looking at the
  // right type of message

  if (decodeHeader(in_msg, msg_len)) {
    cerr << "ERROR - DsSpdbMsg::disassemble" << endl;
    cerr << "Bad message header" << endl;
    cerr << "Message len: " << msg_len << endl;
    return (-1);
  }

  if (_type != DS_MESSAGE_TYPE_SPDB) {
    cerr << "ERROR - DsSpdbMsg::disassemble" << endl;
    cerr << "Unknown message type: " << _type << endl;
    cerr << "Message len: " << msg_len << endl;
    printHeader(&cerr, "");
    return (-1);
  }

  // disassemble the message using the base-class routine
  
  if (DsMessage::disassemble(in_msg, msg_len)) {
    cerr << "ERROR - DsSpdbMsg::disassemble" << endl;
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
  if (partExists(DS_SPDB_APP_NAME_PART)) {
    _appName = (char *) getPartByType(DS_SPDB_APP_NAME_PART)->getBuf();
  }
  if (partExists(DS_SPDB_ERRORSTR_PART)) {
    _errorStr = (char *) getPartByType(DS_SPDB_ERRORSTR_PART)->getBuf();
  }
  if (partExists(DS_SPDB_INFO_PART)) {
    memcpy(&_info, getPartByType(DS_SPDB_INFO_PART)->getBuf(),
	   sizeof(info_t));
    BEtoInfo(_info);
  }
  if (partExists(DS_SPDB_INFO2_PART)) {
    memcpy(&_info2, getPartByType(DS_SPDB_INFO2_PART)->getBuf(),
	   sizeof(info2_t));
    BEtoInfo2(_info2);
  }
  if (partExists(DS_SPDB_HORIZ_LIMITS_PART)) {
    memcpy(&_horizLimits, getPartByType(DS_SPDB_HORIZ_LIMITS_PART)->getBuf(),
	   sizeof(horiz_limits_t));
    BE_to_array_32(&_horizLimits, sizeof(_horizLimits));
    _horizLimitsSet = true;
  }
  if (partExists(DS_SPDB_VERT_LIMITS_PART)) {
    memcpy(&_vertLimits, getPartByType(DS_SPDB_VERT_LIMITS_PART)->getBuf(),
	   sizeof(vert_limits_t));
    BE_to_array_32(&_vertLimits, sizeof(_vertLimits));
    _vertLimitsSet = true;
  }
  if (partExists(DS_SPDB_CHUNK_REF_PART)) {
    DsMsgPart *part = getPartByType(DS_SPDB_CHUNK_REF_PART);
    const void *buf = part->getBuf();
    ssize_t len = part->getLength();
    _refBuf.free();
    _refBuf.add(buf, len);
    Spdb::chunk_refs_from_BE((Spdb::chunk_ref_t *) _refBuf.getPtr(),
                             len / sizeof(Spdb::chunk_ref_t));
  }
  if (partExists(DS_SPDB_AUX_REF_PART)) {
    DsMsgPart *part = getPartByType(DS_SPDB_AUX_REF_PART);
    const void *buf = part->getBuf();
    ssize_t len = part->getLength();
    _auxBuf.free();
    _auxBuf.add(buf, len);
    Spdb::aux_refs_from_BE((Spdb::aux_ref_t *) _auxBuf.getPtr(),
                           len / sizeof(Spdb::aux_ref_t));
  }
  if (partExists(DS_SPDB_CHUNK_DATA_PART)) {
    DsMsgPart *part = getPartByType(DS_SPDB_CHUNK_DATA_PART);
    _dataBuf.free();
    _dataBuf.add(part->getBuf(), part->getLength());
    if (!delay_uncompression) {
      // uncompress now
      uncompressDataBuf();
    }
  }
  if (partExists(DS_SPDB_TIME_LIST_PART)) {
    DsMsgPart *part = getPartByType(DS_SPDB_TIME_LIST_PART);
    ssize_t nTimes = part->getLength() / sizeof(si32);
    si32 *times = (si32 *) part->getBuf();
    BE_from_array_32(times, part->getLength());
    for (ssize_t i = 0; i < nTimes; i++) {
      _timeList.push_back(times[i]);
    }
  }
  if (partExists(DS_SPDB_AUX_XML_PART)) {
    _auxXml = (char *) getPartByType(DS_SPDB_AUX_XML_PART)->getBuf();
  }

  if (_debug) {
    cerr << "------------- DsSpdbMsg::disassemble --------------" << endl;
    print(cerr);
    cerr << "---------------------------------------------------" << endl;
  }

  return (0);

}

/////////////////////////////////////////////////////////////
// compress the data buffer

void DsSpdbMsg::compressDataBuf(Spdb::compression_t compression)

{

  if (compression == Spdb::COMPRESSION_NONE) {
    uncompressDataBuf();
    return;
  }

  Spdb::compression_t currentCompression = dataBufCompression();
  
  if (compression == currentCompression) {
    // already compressed correctly
    return;
  }

  if (currentCompression != Spdb::COMPRESSION_NONE) {
    uncompressDataBuf();
  }
  
  // determine toolsa compression method
  
  ta_compression_method_t compress_method = TA_COMPRESSION_GZIP;
  if (compression == Spdb::COMPRESSION_BZIP2) {
    compress_method = TA_COMPRESSION_BZIP;
  }
  
  // compress

  ui64 nbytesCompressed;
  void *compressedData = ta_compress(compress_method,
                                     _dataBuf.getPtr(),
                                     _dataBuf.getLen(),
                                     &nbytesCompressed);
  if (compressedData == NULL) {
    // failed to compress
    return;
  }

  // success

  _dataBuf.free();
  _dataBuf.add(compressedData, nbytesCompressed);
  
  // free up

  ta_compress_free(compressedData);

  // set compression status

  _info2.data_buf_compression = compression;

}

/////////////////////////////////////
// uncompress the data buffer

void DsSpdbMsg::uncompressDataBuf()

{

  Spdb::compression_t currentCompression = dataBufCompression();

  // check if it is compressed
  
  if (currentCompression == Spdb::COMPRESSION_NONE) {
    // nothing to do
    return;
  }

  // uncompress

  ui64 nbytesUncompressed;
  void *uncompressed  = ta_decompress(_dataBuf.getPtr(), &nbytesUncompressed);
  if (uncompressed == NULL) {
    cerr << "WARNING - DsSpdbMsg::uncompressDataBuf" << endl;
    cerr << "  Cannot uncompress data buffer" << endl;
    _info2.data_buf_compression = Spdb::COMPRESSION_NONE;
    return;
  }

  _dataBuf.free();
  _dataBuf.add(uncompressed, nbytesUncompressed);
  ta_compress_free(uncompressed);

  // set compression status

  _info2.data_buf_compression = Spdb::COMPRESSION_NONE;

}

/////////////////////////////////////
// get data buffer compression state

Spdb::compression_t DsSpdbMsg::dataBufCompression() const
  
{

  return (Spdb::compression_t) _info2.data_buf_compression;

}

////////////////
// print message
//

void DsSpdbMsg::print(ostream &out, const char *spacer) const

{

  out << spacer << "====== DsSpdbMsg ======" << endl;
  out << spacer << "clientHost: " << getClientHost() << endl;
  out << spacer << "clientIpaddr: " << getClientIpaddr() << endl;
  out << spacer << "clientUser: " << getClientUser() << endl;
  out << spacer << "  Message subType: " << subtype2Str(_subType) << endl;
  out << spacer << "  url: " << _urlStr << endl;
  out << spacer << "  mode: " << mode2Str(_mode) << endl;

  switch (_subType) {
    
    case DS_SPDB_PUT:
      if (_info2.lead_time_storage == Spdb::LEAD_TIME_IN_DATA_TYPE) {
        out << spacer << "  Lead time: set in data_type" << endl;
      } else if (_info2.lead_time_storage == Spdb::LEAD_TIME_IN_DATA_TYPE2) {
        out << spacer << "  Lead time: set in data_type2" << endl;
      }
      switch (_mode) {
        case DS_SPDB_PUT_MODE_OVER:
        case DS_SPDB_PUT_MODE_ADD:
        case DS_SPDB_PUT_MODE_ADD_UNIQUE:
        case DS_SPDB_PUT_MODE_ONCE:
          out << spacer << "  nChunks: " << _info.n_chunks << endl;
          break;
        default: {}
      }
      break;
      
    case DS_SPDB_PUT_RETURN:
      if (_flags) {
        out << spacer << "  error occurred." << endl;
        out << spacer << _errorStr << endl;
      }
      break;
      
    case DS_SPDB_GET:
      switch (_mode) {
        case DS_SPDB_GET_MODE_EXACT:
          out << spacer << "  request_time: " << utimstr(_info.request_time) << endl;
          break;
        case DS_SPDB_GET_MODE_CLOSEST:
          out << spacer << "  request_time: " << utimstr(_info.request_time) << endl;
          out << spacer << "  time_margin: " << _info.time_margin << endl;
          break;
        case DS_SPDB_GET_MODE_INTERVAL:
          out << spacer << "  start_time: " << utimstr(_info.start_time) << endl;
          out << spacer << "  end_time: " << utimstr(_info.end_time) << endl;
          break;
        case DS_SPDB_GET_MODE_VALID:
          out << spacer << "  request_time: " << utimstr(_info.request_time) << endl;
          break;
        case DS_SPDB_GET_MODE_LATEST:
          out << spacer << "  time_margin: " << _info.time_margin << endl;
          break;
        case DS_SPDB_GET_MODE_FIRST_BEFORE:
          out << spacer << "  request_time: " << utimstr(_info.request_time) << endl;
          out << spacer << "  time_margin: " << _info.time_margin << endl;
          break;
        case DS_SPDB_GET_MODE_FIRST_AFTER:
          out << spacer << "  request_time: " << utimstr(_info.request_time) << endl;
          out << spacer << "  time_margin: " << _info.time_margin << endl;
          break;
        case DS_SPDB_GET_MODE_TIMES:
          break;
        case DS_SPDB_GET_MODE_TIME_LIST:
          out << spacer << "  start_time: " << utimstr(_info.start_time) << endl;
          out << spacer << "  end_time: " << utimstr(_info.end_time) << endl;
          out << spacer << "  min_interval: " << _info.time_margin << endl;
          break;
        default:
          break;
      }
      out << spacer << "  dataType: " << _info.data_type << endl;
      out << spacer << "  dataType2: " << _info.data_type2 << endl;
      if (_info.get_unique == Spdb::UniqueOff) {
        out << spacer << "  Get unique: off" << endl;
      } else if (_info.get_unique == Spdb::UniqueLatest) {
        out << spacer << "  Get unique: latest" << endl;
      } else if (_info.get_unique == Spdb::UniqueEarliest) {
        out << spacer << "  Get unique: earliest" << endl;
      } else {
        out << spacer << "  Get unique: unknown" << endl;
      }
      if (_info.get_refs_only) {
        out << spacer << "  Get refs only: true" << endl;
      } else {
        out << spacer << "  Get refs only: false" << endl;
      }
      if (_info.respect_zero_types) {
        out << spacer << "  Respect zero types: true" << endl;
      } else {
        out << spacer << "  Respect zero types: false" << endl;
      }
      if (_info2.check_write_time_on_get) {
        out << spacer << "  Check write time: true" << endl;
        out << spacer << "  Latest valid write time: "
            << utimstr(_info2.latest_valid_write_time) << endl;
      }
      if (_info2.check_write_time_on_get) {
        out << spacer << "  Check write time: true" << endl;
        out << spacer << "  Latest valid write time: "
            << utimstr(_info2.latest_valid_write_time) << endl;
      }
      if (_info2.data_buf_compression == Spdb::COMPRESSION_NONE) {
        out << spacer << "  Data buf compression: none" << endl;
      } else if (_info2.data_buf_compression == Spdb::COMPRESSION_GZIP) {
        out << spacer << "  Data buf compression: gzip" << endl;
      } else if (_info2.data_buf_compression == Spdb::COMPRESSION_BZIP2) {
        out << spacer << "  Data buf compression: bzip2" << endl;
      }
      if (_horizLimitsSet) {
        out << spacer << "  Horiz limits:" << endl;
        out << spacer << "    Min lat: " << _horizLimits.min_lat << endl;
        out << spacer << "    Min lon: " << _horizLimits.min_lon << endl;
        out << spacer << "    Max lat: " << _horizLimits.max_lat << endl;
        out << spacer << "    Max lon: " << _horizLimits.max_lon << endl;
      }
      if (_vertLimitsSet) {
        out << spacer << "  Vert limits:" << endl;
        out << spacer << "    Min ht: " << _vertLimits.min_ht << endl;
        out << spacer << "    Max ht: " << _vertLimits.max_ht << endl;
      }
      break;
      
    case DS_SPDB_GET_RETURN:
      switch (_mode) {
        case DS_SPDB_GET_MODE_TIMES:
          out << spacer << "  start_time: " << utimstr(_info.start_time) << endl;
          out << spacer << "  end_time: " << utimstr(_info.end_time) << endl;
          out << spacer << "  last_valid_time: " << utimstr(_info.last_valid_time) << endl;
          break;
        case DS_SPDB_GET_MODE_EXACT:
        case DS_SPDB_GET_MODE_CLOSEST:
        case DS_SPDB_GET_MODE_INTERVAL:
        case DS_SPDB_GET_MODE_VALID:
        case DS_SPDB_GET_MODE_LATEST:
        case DS_SPDB_GET_MODE_FIRST_BEFORE:
        case DS_SPDB_GET_MODE_FIRST_AFTER:
          out << spacer << "  nChunks: " << _info.n_chunks << endl;
          if (_info2.data_buf_compression == Spdb::COMPRESSION_NONE) {
            out << spacer << "  Data buf compression: none" << endl;
          } else if (_info2.data_buf_compression == Spdb::COMPRESSION_GZIP) {
            out << spacer << "  Data buf compression: gzip" << endl;
          } else if (_info2.data_buf_compression == Spdb::COMPRESSION_BZIP2) {
            out << spacer << "  Data buf compression: bzip2" << endl;
          }
          break;
        default:
          break;
      }
      if (_flags) {
        out << spacer << "  error occurred." << endl;
        out << spacer << _errorStr << endl;
      }
      break;
      
    default:
      break;
      
  } // switch (_subType)

  if (_auxXml.size() > 0) {
    out << spacer << "  auxXml:" << endl;
    out << spacer << _auxXml << endl;
  }

  if (_refBuf.getLen() > 0) {
    out << spacer << "  Chunk ref buf size: " << _refBuf.getLen() << endl;
  }

  if (_auxBuf.getLen() > 0) {
    out << spacer << "  Aux ref buf size: " << _auxBuf.getLen() << endl;
  }

  if (_dataBuf.getLen() > 0) {
    out << spacer << "  Data buf size: " << _dataBuf.getLen() << endl;
  }

  if (lengthAssembled() > 0) {
    out << spacer << "  Assembled message size: " << lengthAssembled() << endl;
  }

  out << spacer << "=======================" << endl;

  printHeader(out, spacer);
  // printPartHeaders(out, spacer);

  out << spacer << "=======================" << endl;

}

///////////////////////////////////////////////
// generic assemble for a DS_SPDB_GET message
//
  
void *DsSpdbMsg::_assembleGet(const mode_enum_t request_mode)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_SPDB, DS_SPDB_GET, request_mode);

  // indicate that this is the start of a series of get messages
  
  setCategory(StartGet);
  
  // convert local info to BE byte order

  info_t info(_info);
  BEfromInfo(info);
  info2_t info2(_info2);
  BEfromInfo2(info2);

  // clear message parts

  clearParts();

  // add parts
  
  addClientHost();
  addClientIpaddr();
  addClientUser();
  addPart(DS_SPDB_URL_PART, _urlStr.size() + 1, _urlStr.c_str());
  addPart(DS_SPDB_INFO_PART, sizeof(info_t), &info);
  addPart(DS_SPDB_INFO2_PART, sizeof(info2_t), &info2);
  if (_auxXml.size() > 0) {
    addPart(DS_SPDB_AUX_XML_PART, _auxXml.size() + 1, _auxXml.c_str());
  }
  
  // horizontal limits

  if (_horizLimitsSet) {
    horiz_limits_t hlimits = _horizLimits;
    BE_from_array_32(&hlimits, sizeof(hlimits));
    addPart(DS_SPDB_HORIZ_LIMITS_PART, sizeof(hlimits), &hlimits);
  }

  // vertical limits

  if (_vertLimitsSet) {
    vert_limits_t vlimits = _vertLimits;
    BE_from_array_32(&vlimits, sizeof(vlimits));
    addPart(DS_SPDB_VERT_LIMITS_PART, sizeof(vlimits), &vlimits);
  }

  // assemble
  
  void *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "------------- DsSpdbMsg::_assembleGet --------------" << endl;
    print(cerr);
    cerr << "----------------------------------------------------" << endl;
  }

  return msg;

}

////////////////////////////////////////////////
// Function to return the reference time.

time_t DsSpdbMsg::getRefTime() const {
  //
  // Only relevant for get functions - return 0 otherwise.
  //
  if (_subType != DS_SPDB_GET) return 0L;
 
  switch (_mode) {
    //
    // For LATEST and INTERVAL, return 'now' and end_time respectively.
    // Also return end_time for TIME_LIST mode.
    //
  case DS_SPDB_GET_MODE_LATEST:
    return time(NULL);
    break;
    //
  case DS_SPDB_GET_MODE_INTERVAL:
  case DS_SPDB_GET_MODE_TIME_LIST:
      return _info.end_time;
      break;
      //
      // Mostly, we return the request time.
      // This is the default, but I'll list
      // all cases before I give the default label, just
      // so that the circumstances are clear.
      //
  case DS_SPDB_GET_MODE_VALID:
  case DS_SPDB_GET_MODE_CLOSEST:
  case DS_SPDB_GET_MODE_EXACT:
  case DS_SPDB_GET_MODE_FIRST_BEFORE:
  case DS_SPDB_GET_MODE_FIRST_AFTER:
  case DS_SPDB_GET_MODE_TIMES:
  default :
    return _info.request_time;
    break;
  }
  //
  // Should never get here.
  //
  return 0L;
}


//////////////////////////////////
// Convert to BE from info struct
//

void DsSpdbMsg::BEfromInfo(info_t &info)

{
  BE_from_array_32(&info, sizeof(info_t) - SPDB_LABEL_MAX);
}
  
//////////////////////////////////
// Convert from BE to info struct
//

void DsSpdbMsg::BEtoInfo(info_t &info)

{
  BE_to_array_32(&info, sizeof(info_t) - SPDB_LABEL_MAX);
}

//////////////////////////////////
// Convert to BE from info2 struct
//

void DsSpdbMsg::BEfromInfo2(info2_t &info2)

{
  BE_from_array_32(&info2, sizeof(info2_t));
}
  
//////////////////////////////////
// Convert from BE to info2 struct
//

void DsSpdbMsg::BEtoInfo2(info2_t &info2)

{
  BE_to_array_32(&info2, sizeof(info2_t));
}

/////////////////////////////////
// set or clear horizontal limits
//
// Only relevant for requests to servers which can interpret
// the SPDB data, e.g. the Symprod servers.
  
void DsSpdbMsg::setHorizLimits(double min_lat,
			       double min_lon,
			       double max_lat,
			       double max_lon)
  
{
  _horizLimits.min_lat = min_lat;
  _horizLimits.min_lon = min_lon;
  _horizLimits.max_lat = max_lat;
  _horizLimits.max_lon = max_lon;
  _horizLimitsSet = true;
}

void DsSpdbMsg::clearHorizLimits()
  
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

void DsSpdbMsg::setVertLimits(double min_ht,
			      double max_ht)
  
{
  _vertLimits.min_ht = min_ht;
  _vertLimits.max_ht = max_ht;
  _vertLimitsSet = true;
}

void DsSpdbMsg::clearVertLimits()

{
  _vertLimitsSet = false;
}


//////////////////////////////////////////////////////////////////
// set the auxiliary XML buffer
// This may be used to pass extra information from a client
// to a server
// The contents of the XML must be agreed upon between the client
// and server. This is not part of the SPDB protocol.

void DsSpdbMsg::setAuxXml(const string &xml)
{
  _auxXml = xml;
}

void DsSpdbMsg::clearAuxXml()
{
  _auxXml.clear();
}

////////////////////////////////////////////////
// return string representation of subtype

string DsSpdbMsg::subtype2Str(int subtype)

{

  switch(subtype)  {
    case DS_SPDB_PUT:
      return "DS_SPDB_PUT";
    case DS_SPDB_GET:
      return "DS_SPDB_GET";
    case DS_SPDB_PUT_RETURN:
      return "DS_SPDB_PUT_RETURN";
    case DS_SPDB_GET_RETURN:
      return "DS_SPDB_GET_RETURN";
    default:
      return "UNKNOWN"; 
  }

}

////////////////////////////////////////////////
// return string representation of mode

string DsSpdbMsg::mode2Str(int mode)

{

  switch(mode)  {
    case DS_SPDB_PUT_MODE_ADD_UNIQUE:
      return "DS_SPDB_PUT_MODE_ADD_UNIQUE";
    case DS_SPDB_PUT_MODE_ERASE:
      return "DS_SPDB_PUT_MODE_ERASE";
    case DS_SPDB_PUT_MODE_OVER:
      return "DS_SPDB_PUT_MODE_OVER";
    case DS_SPDB_PUT_MODE_ADD:
      return "DS_SPDB_PUT_MODE_ADD";
    case DS_SPDB_PUT_MODE_ONCE:
      return "DS_SPDB_PUT_MODE_ONCE";
    case DS_SPDB_GET_MODE_EXACT:
      return "DS_SPDB_GET_MODE_EXACT";
    case DS_SPDB_GET_MODE_CLOSEST:
      return "DS_SPDB_GET_MODE_CLOSEST";
    case DS_SPDB_GET_MODE_INTERVAL:
      return "DS_SPDB_GET_MODE_INTERVAL";
    case DS_SPDB_GET_MODE_VALID:
      return "DS_SPDB_GET_MODE_VALID";
    case DS_SPDB_GET_MODE_LATEST:
      return "DS_SPDB_GET_MODE_LATEST";
    case DS_SPDB_GET_MODE_FIRST_BEFORE:
      return "DS_SPDB_GET_MODE_FIRST_BEFORE";
    case DS_SPDB_GET_MODE_FIRST_AFTER:
      return "DS_SPDB_GET_MODE_FIRST_AFTER";
    case DS_SPDB_GET_MODE_TIMES:
      return "DS_SPDB_GET_MODE_TIMES";
    case DS_SPDB_GET_MODE_TIME_LIST:
      return "DS_SPDB_GET_MODE_TIME_LIST";
    default:
      return "UNKNOWN"; 
  }

}

////////////////////////////////////////////////
// return string representation of part

string DsSpdbMsg::part2Str(int part)

{

  switch(part)  {
    case DS_SPDB_URL_PART:
      return "DS_SPDB_URL_PART";
    case DS_SPDB_INFO_PART:
      return "DS_SPDB_INFO_PART";
    case DS_SPDB_CHUNK_REF_PART:
      return "DS_SPDB_CHUNK_REF_PART";
    case DS_SPDB_CHUNK_DATA_PART:
      return "DS_SPDB_CHUNK_DATA_PART";
    case DS_SPDB_ERRORSTR_PART:
      return "DS_SPDB_ERRORSTR_PART";
    case DS_SPDB_HORIZ_LIMITS_PART:
      return "DS_SPDB_HORIZ_LIMITS_PART";
    case DS_SPDB_VERT_LIMITS_PART:
      return "DS_SPDB_VERT_LIMITS_PART";
    case DS_SPDB_INFO2_PART:
      return "DS_SPDB_INFO2_PART";
    case DS_SPDB_TIME_LIST_PART:
      return "DS_SPDB_TIME_LIST_PART";
    case DS_SPDB_APP_NAME_PART:
      return "DS_SPDB_APP_NAME_PART";
    case DS_SPDB_AUX_REF_PART:
      return "DS_SPDB_AUX_REF_PART";
    case DS_SPDB_AUX_XML_PART:
      return "DS_SPDB_AUX_XML_PART";
    default:
      return "UNKNOWN"; 
  }

}

//////////////////////////////
// copy contents

DsSpdbMsg &DsSpdbMsg::_copy(const DsSpdbMsg &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }

  // base class copy

  DsServerMsg::_copy(rhs);

  // copy members

  _appName = rhs._appName;
  _urlStr = rhs._urlStr;
  _errorOccurred = rhs._errorOccurred;
  _errorStr = rhs._errorStr;
  _info = rhs._info;
  _info2 = rhs._info2;
  _horizLimits = rhs._horizLimits;
  _vertLimits = rhs._vertLimits;
  _horizLimitsSet = rhs._horizLimitsSet;
  _vertLimitsSet = rhs._vertLimitsSet;
  _refBuf = rhs._refBuf;
  _auxBuf = rhs._auxBuf;
  _dataBuf = rhs._dataBuf;
  _auxXml = rhs._auxXml;
  _timeList = rhs._timeList;

  return *this;
  
}

