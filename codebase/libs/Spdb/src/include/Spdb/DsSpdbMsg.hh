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
 * Spdb/DsSpdbMsg.hh
 *
 * DsMessage class for Spdb
 *
 * This file defines the Spdb server protocol.
 ******************************************************************/

#ifndef DsSpdbMsg_HH
#define DsSpdbMsg_HH

#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <dsserver/DsServerMsg.hh>
#include <Spdb/Spdb.hh>
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
//        DS_SPDB_PUT_MODE_ERASE
//        DS_SPDB_PUT_MODE_ADD_UNIQUE
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
//        DS_SPDB_GET_MODE_TIME_LIST
//
// Category: StartGet (8389423 - see DsServerMsg.hh)
//
// The message body has the following parts:
//   DS_SPDB_URL_PART
//   DS_SPDB_INFO_PART (info_t struct)
//
// The URL specifies the requested location.
//
// The info parts specify the following, depending on the mode:
//
//   DS_SPDB_GET_MODE_EXACT: request_time, data_types
//   DS_SPDB_GET_MODE_CLOSEST: request_time, time_margin, data_types
//   DS_SPDB_GET_MODE_INTERVAL: start_time, end_time, data_types
//   DS_SPDB_GET_MODE_VALID: request_time, data_types
//   DS_SPDB_GET_MODE_LATEST: time_margin, data_types
//   DS_SPDB_GET_MODE_FIRST_BEFORE: request_time, time_margin, data_types
//   DS_SPDB_GET_MODE_FIRST_AFTER: request_time, time_margin, data_types
//   DS_SPDB_GET_MODE_TIMES: nothing
//   DS_SPDB_GET_MODE_TIME_LIST: start_time, end_time, time_margin
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
//   DS_SPDB_TIME_LIST_PART (time_list)
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

class DsSpdbMsg : public DsServerMsg

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
    DS_SPDB_GET_RETURN = 77203,
  } subtype_enum_t;

  //
  // put and get mode definitions
  //

  typedef enum {
    DS_SPDB_PUT_MODE_ADD_UNIQUE = 77298,
    DS_SPDB_PUT_MODE_ERASE = 77299,
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
    DS_SPDB_GET_MODE_TIMES = 77310,
    DS_SPDB_GET_MODE_TIME_LIST = 77311
  } mode_enum_t;

  //
  // part definitions
  //
  
  typedef enum {
    DS_SPDB_URL_PART = 77500,
    DS_SPDB_INFO_PART = 77501,
    DS_SPDB_CHUNK_REF_PART = 77502,
    DS_SPDB_CHUNK_DATA_PART = 77503,
    DS_SPDB_ERRORSTR_PART = 77504,
    DS_SPDB_HORIZ_LIMITS_PART = 77505,
    DS_SPDB_VERT_LIMITS_PART = 77506,
    DS_SPDB_INFO2_PART = 77507,
    DS_SPDB_TIME_LIST_PART = 77508,
    DS_SPDB_APP_NAME_PART = 77509,
    DS_SPDB_AUX_REF_PART = 77510,
    DS_SPDB_AUX_XML_PART = 77511
  } part_enum_t;

  ///////////////////
  // info structs
  
  typedef struct {
    si32 prod_id;
    si32 n_chunks;
    si32 data_type;
    ti32 start_time;
    ti32 end_time;
    ti32 request_time;
    ti32 last_valid_time;
    si32 time_margin;
    si32 get_unique;
    si32 data_type2;
    si32 get_refs_only;
    si32 respect_zero_types;
    char prod_label[SPDB_LABEL_MAX];
  } info_t;

  // info2 was added as we needed more features and we did
  // not want to break the existing protocol

  typedef struct {
    si32 lead_time_storage;
    si32 check_write_time_on_get;
    si32 latest_valid_write_time;
    si32 data_buf_compression; // see Spdb::compression_t
    si32 spare[12];
  } info2_t;

  typedef struct {
    fl32 min_lat;
    fl32 min_lon;
    fl32 max_lat;
    fl32 max_lon;
    fl32 spare[2];
  } horiz_limits_t;

  typedef struct {
    fl32 min_ht;
    fl32 max_ht;
    fl32 spare[2];
  } vert_limits_t;

  // constructor

  DsSpdbMsg(memModel_t mem_model = CopyMem);

  // copy constructor
  
  DsSpdbMsg(const DsSpdbMsg &rhs);

   // destructor
  
  virtual ~DsSpdbMsg();

  // assignment
  
  DsSpdbMsg & operator=(const DsSpdbMsg &rhs);

  // clear data members

  void clearData();

  ///////////////////////////////////////////////////////////
  // Assemble methods
  // These methods completely set up the object, ready for
  // transmission

  // assemble a DS_SPDB_PUT message
  
  void *assemblePut(const string &app_name,
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
                    Spdb::compression_t data_buf_compression);
  
  // assemble a DS_SPDB_PUT_RETURN message
  
  void *assemblePutReturn(mode_enum_t request_mode,
                          const bool errorOccurred = false,
			  const char *errorStr = NULL);
  
  // assemble a DS_SPDB_GET messages
  
  void *assembleGetExact(const string &url_str,
			 time_t request_time,
			 int data_type,
			 int data_type2,
			 int get_refs_only,
			 int respect_zero_types,
			 int get_unique,
                         bool check_write_time_on_get,
                         time_t latest_valid_write_time,
                         Spdb::compression_t data_buf_compression);
  
  void *assembleGetClosest(const string &url_str,
			   time_t request_time,
			   int time_margin,
			   int data_type,
			   int data_type2,
			   int get_refs_only,
			   int respect_zero_types,
			   int get_unique,
                           bool check_write_time_on_get,
                           time_t latest_valid_write_time,
                           Spdb::compression_t data_buf_compression);

  void *assembleGetInterval(const string &url_str,
			    time_t start_time,
			    time_t end_time,
			    int data_type,
			    int data_type2,
			    int get_refs_only,
			    int respect_zero_types,
			    int get_unique,
                            bool check_write_time_on_get,
                            time_t latest_valid_write_time,
                            Spdb::compression_t data_buf_compression);

  void *assembleGetValid(const string &url_str,
			 time_t request_time,
			 int data_type,
			 int data_type2,
			 int get_refs_only,
			 int respect_zero_types,
			 int get_unique,
                         bool check_write_time_on_get,
                         time_t latest_valid_write_time,
                         Spdb::compression_t data_buf_compression);

  void *assembleGetLatest(const string &url_str,
			  int time_margin,
			  int data_type,
			  int data_type2,
			  int get_refs_only,
			  int respect_zero_types,
			  int get_unique,
                          bool check_write_time_on_get,
                          time_t latest_valid_write_time,
                          Spdb::compression_t data_buf_compression);

  void *assembleGetFirstBefore(const string &url_str,
			       time_t request_time,
			       int time_margin,
			       int data_type,
			       int data_type2,
			       int get_refs_only,
			       int respect_zero_types,
			       int get_unique,
                               bool check_write_time_on_get,
                               time_t latest_valid_write_time,
                               Spdb::compression_t data_buf_compression);

  void *assembleGetFirstAfter(const string &url_str,
			      time_t request_time,
			      int time_margin,
			      int data_type,
			      int data_type2,
			      int get_refs_only,
			      int respect_zero_types,
			      int get_unique,
                              bool check_write_time_on_get,
                              time_t latest_valid_write_time,
                              Spdb::compression_t data_buf_compression);

  void *assembleGetTimes(const string &url_str,
                         bool check_write_time_on_get,
                         time_t latest_valid_write_time);

  void *assembleCompileTimeList(const string &url_str,
				time_t start_time,
				time_t end_time,
				size_t min_time_interval,
                                bool check_write_time_on_get,
                                time_t latest_valid_write_time);
  
  // assemble DS_SPDB_GET_RETURN messages
  
  void *assembleGetDataSuccessReturn(mode_enum_t request_mode,
                                     const info_t &getInfo,
                                     const MemBuf &ref_buf,
                                     const MemBuf &aux_buf,
                                     const MemBuf &data_buf,
                                     Spdb::compression_t data_buf_compression);
  
  void *assembleGetTimesSuccessReturn(mode_enum_t request_mode,
                                      const info_t &info);

  void *assembleCompileTimeListSuccessReturn
    (mode_enum_t request_mode,
     const info_t &info, const vector<time_t> &time_list);
  
  void *assembleGetErrorReturn(mode_enum_t request_mode,
                               const char *errorStr);
  
  ///////////////////////////////////////////////////////
  // generic assemble using member data
  
  void *assemble(int subtype, int mode, category_t category);
  
  ///////////////////////////////////////////////////////
  // disassemble a message buffer to set the object
  
  int disassemble(const void *in_msg, ssize_t msg_len,
                  bool delay_uncompression = false);

  ///////////////////////////////////////////////////////
  // handling compression

  // compress data buffer
  
  void compressDataBuf(Spdb::compression_t compression);
  
  // uncompress data buffer
  // performed in the case of delayed uncompression
  
  void uncompressDataBuf();
  
  // get data buffer compression state
  
  Spdb::compression_t dataBufCompression() const;
  
  ///////////////////////////////////////////////////////////
  // Individual set methods
  // If you set members using these methods, remember to call
  // the 'assemble' method before using for transmission.

  void setAppName(const string &appName) { _appName = appName; }

  mode_enum_t getSpdbMode() const { return (mode_enum_t) _mode; }

  // set url and info

  void setUrlStr(const string &urlStr) { _urlStr = urlStr; }
  void setInfo(const info_t &info) { _info = info; }
  void setInfo2(const info2_t &info2) { _info2 = info2; }

  // set info fields

  void setProdId(int id) { _info.prod_id = id; }
  void setNChunks(int n) { _info.n_chunks = n; }
  void setDataType(int dtype) { _info.data_type = dtype; }
  void setStartTime(time_t time) { _info.start_time = time; }
  void setEndTime(time_t time) { _info.end_time = time; }
  void setRequestTime(time_t time) { _info.request_time = time; }
  void setLastValidTime(time_t time) { _info.last_valid_time = time; }
  void setTimeMargin(int margin) { _info.time_margin = margin; }
  void setGetUnique(bool state) { _info.get_unique = state; }
  void setDataType2(int dtype) { _info.data_type2 = dtype; }
  void setGetRefsOnly(bool state) { _info.get_refs_only = state; }
  void setRespectZeroTypes(bool state) { _info.respect_zero_types = state; }
  void setProdLabel(const string &label) {
    STRncopy(_info.prod_label, label.c_str(), SPDB_LABEL_MAX);
  }

  // set info2 fields

  void setLeadTimeStorage(Spdb::lead_time_storage_t storage) {
    _info2.lead_time_storage = storage;
  }
  void setSheckWriteTimeOnGet(bool state) {
    _info2.check_write_time_on_get = state;
  }
  void setLatestValidWriteTime(time_t time) {
    _info2.latest_valid_write_time = time;
  }

  // Set compression for data transmission
  // Options are Spdb::COMPRESSION_NONE
  //             Spdb::COMPRESSION_GZIP
  //             Spdb::COMPRESSION_BZIP2

  void setDataCompression(Spdb::compression_t compression) { 
    _info2.data_buf_compression = compression;
  }

  /////////////////////////////////
  // set or clear horizontal limits
  //
  // Only relevant for requests to servers which can interpret
  // the SPDB data, e.g. the Symprod servers.
  
  void setHorizLimits(double min_lat,
		      double min_lon,
		      double max_lat,
		      double max_lon);

  void clearHorizLimits();

  ///////////////////////////////
  // set or clear vertical limits
  //
  // heights will generally be specified in the native units for
  // the grid, though there may be instances in which the client
  // and server agree on a different convention.
  //
  // Only relevant for requests to servers which can interpret
  // the SPDB data, e.g. the Symprod servers.
  
  void setVertLimits(double min_ht,
		     double max_ht);

  void clearVertLimits();

  // set the memory buffers

  void setRefBuf(const MemBuf &buf) { _refBuf = buf; }
  void setAuxBuf(const MemBuf &buf) { _auxBuf = buf; }
  void setDataBuf(const MemBuf &buf) { _dataBuf = buf; }

  //////////////////////////////////////////////////////////////////
  // Set/clear the auxiliary XML buffer
  // This may be used to pass extra information from a client
  // to a server
  // The contents of the XML must be agreed upon between the client
  // and server. This is not part of the SPDB protocol.

  void setAuxXml(const string &xml);
  void clearAuxXml();

  // set the time list

  void setTimeList(const vector<time_t> &list) { _timeList = list; }

  /////////////////////////////////////////////////////////
  // access to parts

  const string &getAppName() const { return (_appName); }
  const string &getUrlStr() const { return (_urlStr); }

  const info_t &getInfo() const { return _info; }
  const info2_t &getInfo2() const { return _info2; }

  // info fields

  int getProdId() const { return _info.prod_id; }
  int getNChunks() const { return _info.n_chunks; }
  int getDataType() const { return _info.data_type; }
  time_t getStartTime() const { return _info.start_time; }
  time_t getEndTime() const { return _info.end_time; }
  time_t getRequestTime() const { return _info.request_time; }
  time_t getLastValidTime() const { return _info.last_valid_time; }
  int getTimeMargin() const { return _info.time_margin; }
  bool getGetUnique() const { return _info.get_unique; }
  int getDataType2() const { return _info.data_type2; }
  bool getGetRefsOnly() const { return _info.get_refs_only; }
  bool getRespectZeroTypes() const { return _info.respect_zero_types; }
  string getProdLabel(const string &label) const { return _info.prod_label; }

  // info 2 fields

  Spdb::lead_time_storage_t getLeadTimeStorage() const {
    return (Spdb::lead_time_storage_t) _info2.lead_time_storage;
  }
  bool getSheckWriteTimeOnGet(bool state) const {
    return _info2.check_write_time_on_get;
  }
  time_t getLatestValidWriteTime(time_t time) const {
    return _info2.latest_valid_write_time;
  }
  Spdb::compression_t getDataBufCompression() const {
    return (Spdb::compression_t) _info2.data_buf_compression;
  }
  
  // horizontal and vertical limits

  bool horizLimitsSet() const { return _horizLimitsSet; }
  const horiz_limits_t &getHorizLimits() const { return _horizLimits; }

  bool vertLimitsSet() const { return _vertLimitsSet; }
  const vert_limits_t &getVertLimits() const { return _vertLimits; }

  // chunk references

  const Spdb::chunk_ref_t *getChunkRefs() const {
    return (Spdb::chunk_ref_t *) _refBuf.getPtr();
  }

  const Spdb::aux_ref_t *getAuxRefs() const {
    return (Spdb::aux_ref_t *) _auxBuf.getPtr();
  }

  // data

  const void *getChunkData() const { return _dataBuf.getPtr(); }
  int getChunkDataLen() const { return _dataBuf.getLen(); }

  // time list
  
  const vector<time_t> &getTimeList() const { return (_timeList); }
  
  // Get the reference time.

  time_t getRefTime() const;

  // auxiliary XML

  const string &getAuxXml() const { return _auxXml; }

  // error status

  bool errorOccurred() const { return (_errorOccurred); }
  const string &getErrorStr() const { return (_errorStr); }

  // print object

  virtual void print(ostream &out, const char *spacer = "") const;

  // byte swapping

  static void BEfromInfo(info_t &info);
  static void BEtoInfo(info_t &info);

  static void BEfromInfo2(info2_t &info2);
  static void BEtoInfo2(info2_t &info2);

  // enums to strings

  static string subtype2Str(int subtype);
  static string mode2Str(int mode);
  static string part2Str(int part);

protected:

 virtual DsSpdbMsg &_copy(const DsSpdbMsg &rhs);

private:

  string _appName;
  string _urlStr;
  bool _errorOccurred;
  string _errorStr;

  // info structs

  info_t _info;
  info2_t _info2;

  horiz_limits_t _horizLimits;
  vert_limits_t _vertLimits;
  bool _horizLimitsSet;
  bool _vertLimitsSet;

  // chunk refs and data

  MemBuf _refBuf;
  MemBuf _auxBuf;
  MemBuf _dataBuf;
  string _auxXml;

  vector<time_t> _timeList;

  // functions

  void *_assembleGet(const mode_enum_t request_mode);
};

#endif

