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
 * spdb/SpdbMsg.hh
 *
 * DsMessage class for Spdb
 *
 * This file defines the Spdb server protocol.
 ******************************************************************/

#ifndef SpdbMsg_HH
#define SpdbMsg_HH

#include <toolsa/MemBuf.hh>
#include <dsserver/DsServerMsg.hh>
#include <spdb/spdb.h>
#include <iostream>
using namespace std;

////////////////////////////////////////////////////////////////////a
// Messages to and from the SpdbServer make use the generic
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

///////////////
// PUT Message
//
// Type: DS_MESSAGE_TYPE_SPDB
//
// SubType: DS_SPDB_PUT
//
// Modes: DS_SPDB_PUT_MODE_OVER
//        DS_SPDB_PUT_MODE_ADD
//        DS_SPDB_PUT_MODE_ONCE
//
// Category: StartPut (8389422 - see DsServerMsg.hh)
//
// The message body has the following parts:
//   DS_SPDB_URL_PART
//   DS_SPDB_INFO_PART (info_t struct)
//   DS_SPDB_CHUNK_REF_PART (array of spdb_chunk_ref_t)
//   DS_SPDB_CHUNK_DATA_PART (data buffer)
//
// The URL specifies the destination location.
//
// The info part specifies the following:
//   product_label
//   product_id
//   n_chunks
//
// The url specifies the destination.
//
// Data_type and times are set in the chunk refs.
//
// The chunk refs and chunk data must be in BE byte order.
//
// A DS_SPDB_PUT_RETURN message is expected as a reply.
//

/////////////////////
// PUT_RETURN Message
//
// Type: DS_MESSAGE_TYPE_SPDB
// SubType: DS_SPDB_PUT_RETURN
// Mode: set to mode of PUT message
// Flags: 0 on success, -1 on error.
// Category: EndSeries (8389424 - see DsServerMsg.hh)
//
// If an error occurred, the return message will contain a
// DS_SPDB_ERRORSTR_PART containing the error message.
//

///////////////
// GET Message
//
// Type: DS_MESSAGE_TYPE_SPDB
//
// SubType: DS_SPDB_GET
//
// Modes: DS_SPDB_GET_MODE_EXACT
//        DS_SPDB_GET_MODE_CLOSEST
//        DS_SPDB_GET_MODE_INTERVAL
//        DS_SPDB_GET_MODE_VALID
//        DS_SPDB_GET_MODE_LATEST
//        DS_SPDB_GET_MODE_FIRST_BEFORE
//        DS_SPDB_GET_MODE_FIRST_AFTER
//        DS_SPDB_GET_MODE_TIMES
//
// Category: StartGet (8389423 - see DsServerMsg.hh)
//
// The message body has the following parts:
//   DS_SPDB_URL_PART
//   DS_SPDB_INFO_PART (info_t struct)
//
// The URL specifies the requested location.
//
// The info part specifies the following, depending on the mode:
//
//   DS_SPDB_GET_MODE_EXACT: request_time, data_type
//   DS_SPDB_GET_MODE_CLOSEST: request_time, time_margin, data_type
//   DS_SPDB_GET_MODE_INTERVAL: start_time, end_time, data_type
//   DS_SPDB_GET_MODE_VALID: request_time, data_type
//   DS_SPDB_GET_MODE_LATEST: time_margin, data_type
//   DS_SPDB_GET_MODE_FIRST_BEFORE: request_time, time_margin, data_type
//   DS_SPDB_GET_MODE_FIRST_AFTER: request_time, time_margin, data_type
//   DS_SPDB_GET_MODE_TIMES: nothing
//
// A DS_SPDB_GET_RETURN message is expected as a reply.
//

/////////////////////
// GET_RETURN Message
//
// Type: DS_MESSAGE_TYPE_SPDB
// SubType: DS_SPDB_GET_RETURN
// Mode: set to mode of GET message
// Flags: 0 on success, -1 on error.
// Category: EndSeries (8389424 - see DsServerMsg.hh)
//
// Some of the following parts will be present:
//
//   DS_SPDB_INFO_PART (info_t struct)
//   DS_SPDB_CHUNK_REF_PART (array of spdb_chunk_ref_t)
//   DS_SPDB_CHUNK_DATA_PART (data buffer)
//   DS_SPDB_ERRORSTR_PART (error msg)
//
// On success, the return message has a DS_SPDB_INFO_PART containing
// the relevant information.
//
// If an error occurred, the return message will contain only a
// DS_SPDB_ERRORSTR_PART containing the error message.
//
// On success:
//
// For mode DS_SPDB_GET_MODE_TIMES the returned times are in the info part.
//   first_time is stored as start_time.
//   last_time is stored as end_time.
//   last_valid_time is stored as last_valid_time.
//
// For all other get modes, n_chunks is in the DS_SPDB_INFO_PART.
// In addition, they will have a DS_SPDB_CHUNK_REF_PART and a
// DS_SPDB_CHUNK_DATA_PART.
//

///////////////////////////////////////////////////////////////////
// class definition

class SpdbMsg : public DsServerMsg

{

public:
  
  //
  // message type definition
  //

  typedef enum {
    DS_MESSAGE_TYPE_SPDB = 77000
  } type_enum_t;

  //
  // message sub-type definitions
  //
  
  typedef enum {
    DS_SPDB_PUT = 77200,
    DS_SPDB_GET = 77201,
    DS_SPDB_PUT_RETURN = 77202,
    DS_SPDB_GET_RETURN = 77203
  } subtype_enum_t;

  //
  // put and get mode definitions
  //

  typedef enum {
    DS_SPDB_PUT_MODE_OVER = 77300,
    DS_SPDB_PUT_MODE_ADD = 77301,
    DS_SPDB_PUT_MODE_ONCE = 77302,
    DS_SPDB_GET_MODE_EXACT = 77303,
    DS_SPDB_GET_MODE_CLOSEST = 77304,
    DS_SPDB_GET_MODE_INTERVAL = 77305,
    DS_SPDB_GET_MODE_VALID = 77306,
    DS_SPDB_GET_MODE_LATEST = 77307,
    DS_SPDB_GET_MODE_FIRST_BEFORE = 77308,
    DS_SPDB_GET_MODE_FIRST_AFTER = 77309,
    DS_SPDB_GET_MODE_TIMES = 77310
  } mode_enum_t;

  //
  // part definitions
  //
  
  typedef enum {
    DS_SPDB_URL_PART = 77500,
    DS_SPDB_INFO_PART = 77501,
    DS_SPDB_CHUNK_REF_PART = 77502,
    DS_SPDB_CHUNK_DATA_PART = 77503,
    DS_SPDB_ERRORSTR_PART = 77504
  } part_enum_t;

  ///////////////////
  // spdb info struct
  
  typedef struct {
    si32 prod_id;
    si32 n_chunks;
    si32 data_type;
    ti32 start_time;
    ti32 end_time;
    ti32 request_time;
    ti32 last_valid_time;
    si32 time_margin;
    si32 spare[4];
    char prod_label[SPDB_LABEL_MAX];
  } info_t;
  
  // constructor

  SpdbMsg(memModel_t mem_model = CopyMem);

  // destructor
  
  virtual ~SpdbMsg();

  // assemble a DS_SPDB_PUT message
  
  void *assemblePut(const string &url_str,
		    const int prod_id,
		    const string &prod_label,
		    const mode_enum_t mode,
		    const int n_chunks,
		    const spdb_chunk_ref_t *refs,
		    const void *chunk_data,
		    const int chunk_data_len);
  
  // assemble a DS_SPDB_PUT_RETURN message
  
  void *assemblePutReturn(const bool errorOccurred = false,
			  const char *errorStr = NULL);
  
  // assemble a DS_SPDB_GET messages
  
  void *assembleGetExact(const string &url_str,
			 time_t request_time,
			 int data_type);
  
  void *assembleGetClosest(const string &url_str,
			   time_t request_time,
			   int time_margin,
			   int data_type);

  void *assembleGetInterval(const string &url_str,
			    time_t start_time,
			    time_t end_time,
			    int data_type);

  void *assembleGetValid(const string &url_str,
			 time_t request_time,
			 int data_type);

  void *assembleGetLatest(const string &url_str,
			  int time_margin,
			  int data_type);

  void *assembleGetFirstBefore(const string &url_str,
			       time_t request_time,
			       int time_margin,
			       int data_type);

  void *assembleGetFirstAfter(const string &url_str,
			      time_t request_time,
			      int time_margin,
			      int data_type);

  void *assembleGetTimes(const string &url_str);
  
  // assemble DS_SPDB_GET_RETURN messages
  
  void *assembleGetDataSuccessReturn(const info_t &info,
				     const spdb_chunk_ref_t *refs,
				     const void *chunk_data);

  void *assembleGetTimesSuccessReturn(const info_t &info);

  void *assembleGetErrorReturn(const char *errorStr);

  // overload disassemble
  
  int disassemble(void *in_msg, const int msg_len);

  // print

  void print(ostream &out);

  // access to parts

  string &getUrlStr() { return (_urlStr); }
  bool errorOccurred() { return (_errorOccurred); }
  string &getErrorStr() { return (_errorStr); }
  info_t &getInfo() { return (_info); }
  int getNChunks() { return (_nChunks); }
  spdb_chunk_ref_t *getChunkRefs() { return (_chunkRefs); }
  void *getChunkData() { return (_chunkData); }
  int getChunkDataLen() { return (_chunkDataLen); }

protected:

private:

  string _urlStr;
  bool _errorOccurred;
  string _errorStr;

  // info struct
  info_t _info;

  // chunk refs and data

  int _nChunks;
  spdb_chunk_ref_t *_chunkRefs;
  MemBuf _refBuf;
  void *_chunkData;
  int _chunkDataLen;

  // functions

  void *_assembleGetGeneric(const string &url_str,
			    const mode_enum_t mode);
  void _clearInfo();
  void _BEfromInfo();
  void _BEtoInfo();

};

#endif


