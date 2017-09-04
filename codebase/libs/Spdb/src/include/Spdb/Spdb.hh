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
// Spdb/Spdb.hh
//
// Spdb class
//
// This class handles the Spdb operations at the local disk level.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
////////////////////////////////////////////////////////////////
//
// The Symbolic Products Data Base (SPDB) consists of a pair
// of files for each day. Each product type will be stored in
// its own directory.
//
// For each day, there is an index file (extension .indx) and a
// data file (extension .data). The index file header contains
// a product type string and a product information string. The
// info string may be optionally filled out.
//
// The header also has product ID field, which should be set.
//
// Included in the header is a fixed length array of 1440 integers,
// minute_posn, which are used for rapid searching for data at a
// given time. The minute_posn values store the first chunk position
// for each minute of the day. If no chunk corresponds to this minute
// the value is set to -1.
//
// Following the header is an array, n_chunks long, of the
// spdb chunk structs. Each chunk points into the data file,
// giving the offset and length of the data chunk for this
// chunk. The valid time is used for sorting the chunks. The
// expire time is included to allow retrieval of all chunks which
// are valid at a given time.
//
// All values in the header are stored in Big-Endian format. The
// chunk data is stored as it is passed to the library. The calling
// program must make sure the chunks are in BE format.
//
////////////////////////////////////////////////////////////////

#ifndef Spdb_HH
#define Spdb_HH

#define _in_Spdb_hh

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
#include <Spdb/Product_defines.hh>

using namespace std;

///////////////////////////////////////////////////////////////
// class definition

class Spdb

{

  friend class DsSpdbServer;

public:

  // typedefs and constants

#include <Spdb/Spdb_typedefs.hh>

  // constructor

  Spdb();

  // destructor
  
  virtual ~Spdb();
  
  /////////////////////////////////////////////////////////////////////
  //
  // put functions:
  //
  //   The put functions put one or more data chunks to one or more URLs.
  //
  // Put modes:
  //
  //   There are 4 modes for putting:
  //
  //     putModeOver (the default) - a new chunk will overwrite a
  //     previously-stored chunk at the same valid_time.
  //
  //     putModeOnce - you can only put once for a given valid_time.
  //     Trying to put at the same valid_time again will generate an error.
  //
  //     putModeAdd - a chunk will be added to the list of chunks stored
  //     at a given valid_time.
  //
  //     putModeAddUnique - a chunk will be added to the list of chunks stored
  //     at a given valid_time, provided the identical chunk has not already
  //     been added.
  //
  // Use the function setPutMode() to set the mode to one of the following:
  //   putModeOver, putModeAdd, putModeOnce, putModeAddUnique
  //
  // The put functions are overloaded - you need to use the argument
  // list relevant to your application.
  //
  // Decision : one or more chunks?
  //
  //   For putting a single chunk, use the put functions which allow you to
  //   specify data_type, valid_time, expire_time, chunk_len, chunk_data
  //   and optionally data_type2.
  //
  //   For putting multiple chunks:
  //     (a) clear the chunk buffer using clearPutChunks()
  //     (b) add each chunk using addPutChunk()
  //     (c) perform the put using the put functions which do not allow
  //         yout to specify data_type etc.
  //
  // Return values:
  //
  //   The put functions return 0 on success, -1 on error.
  //
  // Error messages:
  //
  //   Error messages are printed to stderr.
  //
  /////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////
  // set the put mode
  // Options are: putModeOver, putModeOnce, putModeAdd, putModeAddUnique
  // Default is putModeOver
  
  void setPutMode(put_mode_t mode);

  ////////////////////////////////////////////////////
  // set the lead time storage
  // If you are dealing with forecast data, you may wish to store
  // the lead time in each chunk header as data_type or data_type2.
  // If you do so, and you set up the storage type using this routine,
  // the latest_data_info file will show the gen time and forecast time
  // instead of the valid time. Also, the header will show this
  // decision.
  //
  // Options are:
  //
  //   LEAD_TIME_NOT_APPLICABLE (the default)
  //   LEAD_TIME_IN_DATA_TYPE
  //   LEAD_TIME_IN_DATA_TYPE2
  
  void setLeadTimeStorage(lead_time_storage_t storage_type);

  ////////////////////////////////////////////////////
  // Set chunk compression for put operations.
  // The following compression types can be specified:
  //    Spdb::COMPRESSION_NONE
  //    Spdb::COMPRESSION_GZIP
  //    Spdb::COMPRESSION_BZIP2
  // If set, chunks will be stored compressed and the
  // compression flag will be set in the auxiliary chunk header.
  // The default is COMPRESSION_NONE.

  void setChunkCompressOnPut(compression_t compression);

  ////////////////////////////////////////////////////
  // Set chunk uncompression for get operations.
  // If set, chunks will be uncomrpessed on get, if
  // they are stored using compression.
  // The default is true.

  void setChunkUncompressOnGet(bool state = true);

  /////////////////////////////////////////////////////////////
  // clear put chunks before loading buffer using 'addPutChunk'
  
  void clearPutChunks();
  
  ////////////////////////////////
  // Add a chunk to the put buffer
  // The chunk data will be compressed appropriately if
  // setChunkCompressOnPut() has been called.
  
  void addPutChunk(int data_type,
		   time_t valid_time,
		   time_t expire_time,
		   int chunk_len,
		   const void *chunk_data,
		   int data_type2 = 0,
                   const char *tag = NULL);

  //////////////////////////////////////////////
  // Add an array of chunks to the chunk buffers
  // Note - it is preferable to use addPutChunk
  // in a loop instead.
  // The chunk data will be compressed appropriately if
  // setChunkCompressOnPut() has been called.
  
  void addPutChunks(int n_chunks,
		    const chunk_ref_t *chunk_refs,
		    const void *chunk_data);

  
  //////////////////////////////////////////////
  // Set respect_zero_types on put.
  // By default, if the data_type or data_type2 is 0,
  // it is not used to discriminate between entries in
  // the data base. For example, if the mode is PUT_OVER,
  // a new entry with data_type == 0 will overwrite any
  // entry already in the data base. If respect_zero_types
  // is true, a new entry with data_type == 0 will only
  // overwrite an existing entry with data_type == 0.
  // Similarly for data_type2.

  void setRespectZeroTypesOnPut(bool state = true) {
    _respectZeroTypes = state;
  }

  //////////////////////////////////////////////
  // Enable defragmentation in data files.
  // Off by default.
  // If true, the defragmentation method will be called
  // if a file has fragmented data because of many erase
  // operations. This is rare.

  void setEnableDefragmentation(bool state = true) {
    _enableDefrag = state;
  }

  ///////////////////////
  // number of put chunks

  int nPutChunks() { return (_nPutChunks); }

  //////////////////////////////////////////////////////////
  // put - single chnk
  //
  // Chunk must already be in BE byte order, as appropriate.
  //
  // The chunk data will be compressed appropriately if
  // setChunkCompressOnPut() has been called.
  //
  // Returns 0 on success, -1 on error.
  
  virtual int put(const string &dir,
		  int prod_id,
		  const string &prod_label,
		  int data_type,
		  time_t valid_time,
		  time_t expire_time,
		  int chunk_len,
		  const void *chunk_data,
		  int data_type2 = 0);

  time_t getLatestValidTimePut() const { return(_latestValidTimePut); }
  
  //////////////////////////////////////////////////////////
  // put chunks which have been added with
  // addPutChunk() or addPutChunks()
  //
  // Returns 0 on success, -1 on error

  virtual int put(const string &dir,
		  const int prod_id,
		  const string &prod_label);
  
  ////////////////////////////////////////////////////////////////////
  // Erase data for a given valid time and data type.
  // If the data_type is 0, the data_type is not considered in the erase.
  // Similarly for data_type2.
  // This is a special type of 'put'.
  //
  // Returns 0 on success, -1 on failure
  
  virtual int erase(const string &dir,
		    time_t valid_time,
		    int data_type = 0,
		    int data_type2 = 0);

  ////////////////////////////////////////////////////////////////////
  // Erase data for a given set of chunk refs.
  // Before calling this function, call clearPutChunks(),
  // and addPutChunk() for each time and data_type you want to erase.
  // You can call addPutChunk() with 0 length and NULL pointer.
  // If the data_type is 0, all data at that time is erased.
  // This is a special type of 'put'.
  //
  // Returns 0 on success, -1 on failure
  
  virtual int erase(const string &dir);

  ///////////////////////////////////////////////////////////////////
  //
  // get functions
  //
  // The get() functions get spdb data from the database pointed to
  // by the dir.
  //
  // There are a variety of functions, each of which specifies the
  // time search in a different manner.
  //
  // If data_type == 0, all relevant chunks are returned.
  // If data_type != 0, only chunks with that data type are returned. 
  // If data_type2 == 0, all relevant chunks are returned.
  // If data_type2 != 0, only chunks with that data_type2 are returned. 
  //
  // All get functions return 0 on success, -1 on error.
  // Error string is available via getErrorStr().
  //
  // Get functions are not threaded.
  //
  // All of the functions set an array of chunk reference headers and
  // a buffer with the chunk data. Any function may return more that 1
  // chunk - if more than one chunk is stored at the requested time
  // all of the chunks will be returned.
  //
  // The following functions are used to get the chunk details
  // after the get functions:
  //   int getNChunks() - returns the number of chunks
  //   chunk_ref_t *getChunkRefs() - returns the chunk refs
  //   void *getChunkData() - pointer to the chunk data buffer
  //   void *getChunk(int i) - pointer to chunk i
  //
  // The chunk refs are in host-byte order.
  // The chunk data is in BE byte order.
  //
  // You have the option of only getting the chunk refs, and not
  // the data itself. This is more efficient if you only need access
  // to the refs, for example when you are only interested in the
  // times etc.
  //
  // To activate this option, set get_refs_only to true.
  // The default is false.
  //
  // By default, if you specify a data_type of 0, the data_type will
  // be ignored, and you will get all data_types. The same applies
  // to data_type2. You can override this behavior by setting
  // respect_zero_types to true. The default is false.
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  // specifying the product ID.
  //
  // The product ID defaults to 0, which data from any product ID will
  // be returned. If you wish to make sure that only data of a certain
  // product ID is returned, set the required ID before the get.

  void setProdId(int id) { _prodId = id; }

  ///////////////////////////////////////////////////////////////////
  // specifying uniqueness.
  //
  // If you specify 'latest', the get() filters duplicate data from
  // the chunk data, leaving the latest chunk for each data_type.
  // All chunks with data_type of 0 will be left in the buffer.
  //
  // If you specify 'earliest', the get() filters duplicate data from
  // the chunk data, leaving the earliest chunk for each data_type.
  // All chunks with data_type of 0 will be left in the buffer.
  //
  // If you do not specify anything, all data will be returned.

  void setUniqueOff() { _getUnique = UniqueOff; }
  void setUniqueLatest() { _getUnique = UniqueLatest; }
  void setUniqueEarliest() { _getUnique = UniqueEarliest; }

  ///////////////////////////////////////////////////////////////////
  // getExact()
  //
  // Get data at exactly the given time.
  //

  virtual int getExact(const string &dir,
		       time_t request_time,
		       int data_type = 0,
		       int data_type2 = 0,
		       bool get_refs_only = false,
		       bool respect_zero_types = false);
  
  ///////////////////////////////////////////////////////////////////
  // getClosest()
  //
  // Get data closest to the given time, within the time margin.
  //

  virtual int getClosest(const string &dir,
			 time_t request_time,
			 int time_margin,
			 int data_type = 0,
			 int data_type2 = 0,
			 bool get_refs_only = false,
			 bool respect_zero_types = false);

  ///////////////////////////////////////////////////////////////////
  // getInterval()
  //
  // Get data in the time interval.
  //

  virtual int getInterval(const string &dir,
			  time_t start_time,
			  time_t end_time,
			  int data_type = 0,
			  int data_type2 = 0,
			  bool get_refs_only = false,
			  bool respect_zero_types = false);

  ///////////////////////////////////////////////////////////////////
  // getValid()
  //
  // Get data valid at the given time.
  //

  virtual int getValid(const string &dir,
		       time_t request_time,
		       int data_type = 0,
		       int data_type2 = 0,
		       bool get_refs_only = false,
		       bool respect_zero_types = false);
  
  ///////////////////////////////////////////////////////////////////
  // getLatest()
  //
  // Get latest data, within the given time margin. This is the
  // same as getInterval(latestTime - margin, latestTime).
  //

  virtual int getLatest(const string &dir,
			int time_margin = 0,
			int data_type = 0,
			int data_type2 = 0,
			bool get_refs_only = false,
			bool respect_zero_types = false);

  ///////////////////////////////////////////////////////////////////
  // getFirstBefore()
  //
  // Get first data at or before the requested time,
  // within the given time margin.
  //

  virtual int getFirstBefore(const string &dir,
			     time_t request_time,
			     int time_margin,
			     int data_type = 0,
			     int data_type2 = 0,
			     bool get_refs_only = false,
			     bool respect_zero_types = false);
  
  ///////////////////////////////////////////////////////////////////
  // getFirstAfter()
  //
  // Get first data at or after the requested time,
  // within the given time margin.
  //

  virtual int getFirstAfter(const string &dir,
			    time_t request_time,
			    int time_margin,
			    int data_type = 0,
			    int data_type2 = 0,
			    bool get_refs_only = false,
			    bool respect_zero_types = false);

  ////////////////////////////////////////////////////////////
  // get the first, last and last_valid_time in the data base
  //
  // In this case, no chunk data is returned.

  virtual int getTimes(const string &dir,
		       time_t &first_time,
		       time_t &last_time,
		       time_t &last_valid_time);
  
  /////////////////////////////////////////////////////////
  // Option to check write times on get.
  // Spdb records the time at which the data are written.
  // If set, only return data stored at or before
  //   the latest valid write time.

  void setCheckWriteTimeOnGet(time_t latest_valid_time) {
    _checkWriteTimeOnGet = true;
    _latestValidWriteTime = latest_valid_time;
  }

  void clearCheckWriteTimeOnGet() {
    _checkWriteTimeOnGet = false;
    _latestValidWriteTime = 0;
  }

  ////////////////////////////////////////////////////////////
  // get the first, last and last_valid_time in the data base
  // Use getFirstTime(), getLastTime() and getLastValidTime()
  // to access the times after making this call.
  //
  // In this case, no chunk data is returned.

  virtual int getTimes(const string &dir);

  time_t getFirstTime() const { return (_firstTime); }
  time_t getLastTime() const { return (_lastTime); }
  time_t getLastValidTime() const { return (_lastValidTime); }

  ///////////////////////////////////////////////////////////////////
  // access to info and chunk refs and data after a get()

  int getProdId() const { return (_prodId); }
  const string &getProdLabel() const { return (_prodLabel); }
  int getNChunks() const { return (_nGetChunks); }
  chunk_ref_t *getChunkRefs() const {
    return ((chunk_ref_t *) _getRefBuf.getPtr());
  }
  aux_ref_t *getAuxRefs() const {
    return ((aux_ref_t *) _getAuxBuf.getPtr());
  }
  void *getChunkData() const { return (_getDataBuf.getPtr()); }
  bool getRefsOnly() const { return (_getRefsOnly); }

  // After a get(), you can access the chunk data via a
  // vector<chunk_t>.
  // Note: getChunk() sets up the vector on each call, so do not call it
  // frequently. Rather call it once and set a local variable
  // to the returned value.

  const vector<chunk_t> &getChunks() const { return _chunksFromGet; }

  // Uncompress chunks in the get buffers.
  // Normally it is not necessary to use this call.
  // It should be called before using chunk data, if
  // setChunkUnCompressOnGet(false) was called before
  // getting the data.
  
  void uncompressGetChunks();

  //////////////////////////////////////////////////////////
  // compile time list
  //
  // Compile a list of available data times in the specified
  // directory between the start and end times.
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
  
  virtual int compileTimeList(const string &dir,
			      time_t start_time,
			      time_t end_time,
			      size_t minimum_interval = 1);

  // access to time list

  const vector<time_t> &getTimeList() const { return (_timeList); }
  int getNTimesInList() const { return (_timeList.size()); }
  time_t getTimeFromList(int i) const { return (_timeList[i]); }

  ////////////////////////////////////////////////////////////////////
  // makeUniqueLatest()
  //
  // This functions filters duplicate data from the chunk data,
  // leaving the latest chunk for each data_type. All chunks with
  // data_type of 0 will be left in the buffer.
  //
  // The intention is that you can call this function after any of
  // the get routines, and filter the output of the get.

  void makeUniqueLatest();

  ////////////////////////////////////////////////////////////////////
  // makeUniqueEarliest()
  //
  // This functions filters duplicate data from the chunk data,
  // leaving the earliest chunk for each data_type. All chunks with
  // data_type of 0 will be left in the buffer.
  //
  // The intention is that you can call this function after any of
  // the get routines, and filter the output of the get.
  
  void makeUniqueEarliest();
  
  ////////////////////////////////////////////////////////////////////
  // makeUnique
  //
  // This functions filters duplicate data from the chunk data,
  // leaving the latest or earliest chunk for each data_type.
  // If latest is true, we get the latest. Same as calling
  // makeUniqueLatest().
  // If latest is false, we get the earliest. Same as calling
  // makeUniqueEarliest().
  //
  // All chunks with data_type of 0 will be left in the buffer.
  //
  // The intention is that you can call this function after any of
  // the get routines, and filter the output of the get.
  
  void makeUnique(bool latest = true);
  
  //////////////////////////////////////////////////////////////////
  // Set/clear the auxiliary XML buffer
  // This may be used to pass extra information from a client
  // to a server
  // The contents of the XML must be agreed upon between the client
  // and server. This is not part of the SPDB protocol.

  void setAuxXml(const string &xml);
  void clearAuxXml();
  
  ///////////////////////////////////////////////////////////////////
  //
  // print functions
  //
  ///////////////////////////////////////////////////////////////////
  
  //////////////////////////////////////////////
  // print header
  // Prints the SPDB headers for a given dir and time
  //
  // Returns 0 on success, -1 on failure

  int printHeader(const string &dir,
		  time_t request_time,
		  ostream &out);
  
  // Print the given chunk reference structure
  
  static void printChunkRef(const chunk_ref_t *chunk_ref, ostream &out,
                            const aux_ref_t *aux_ref = NULL);

  // Prints the given chunk object
  
  void printChunk(const chunk_t &chunk, ostream &out);
  
  ////////////////////////////////////////////////////////////
  // hash4CharsToInt32()
  //
  // Convert the first 4 characters of an ID STRING to a int 32.
  // On error Returns -1.
  // This function guarnteed to never return 0
  
  static si32 hash4CharsToInt32(const char *id_string);

  ////////////////////////////////////////////////////////////
  // dehashInt32To4Chars()
  //
  // Convert a 32-bit int into a 4 Character ID String
  // Returns a string containing the 4-char ASCII ID.
  
  static string dehashInt32To4Chars(si32 id_int);


  ////////////////////////////////////////////////////////////
  // hash5CharsToInt32()
  //
  // Packs 5 or fewer ID characters into an int 32.
  // Valid chars: NULL,'-','0-9',A-Z','a-z'
  // On error Returns -1.
  // This will not clash with values from hash4CharsToInt32() or
  // positive integers.
  // This function guarnteed to never return 0
  
  static si32 hash5CharsToInt32(const char *id_string);

  ////////////////////////////////////////////////////////////
  // dehashInt32To5Chars()
  //
  // Convert a 32-bit int into a 5 or fewer Character ID String
  // Returns a string containing the 1-5 char ASCII ID.
  
  static string dehashInt32To5Chars(si32 id_int);

  ///////////////////////////////////////////////////////////////////
  // error string
  
  const string &getErrStr() const { return (_errStr); }
  const string &getErrorStr() const { return (_errStr); }
  const string &getErrorString() const { return (_errStr); }

  ///////////////////////////
  // name of application

  const string &getAppName() const { return _appName; }
  void setAppName(const string &app_name) const { _appName = app_name; }
 
  // byte swapping for chunk refs

  static void chunk_refs_to_BE(chunk_ref_t *refs, int nn);
  static void chunk_refs_from_BE(chunk_ref_t *refs, int nn);
  static void aux_refs_to_BE(aux_ref_t *auxs, int nn);
  static void aux_refs_from_BE(aux_ref_t *auxs, int nn);

 protected:

  static int _fileMajorVersion;
  static int _fileMinorVersion;
  static const char *_indxExt;
  static const char *_dataExt;
  
  // name of application
  
  mutable string _appName;
  
  // product info
  
  string _prodLabel;
  int _prodId;
  
  // file names
  
  string _dir;
  string _path;
  char _indxPath[SPDB_PATH_MAX];
  char _dataPath[SPDB_PATH_MAX];
  char _lockPath[SPDB_PATH_MAX];
  
  // file header and chunk refs
  
  header_t _hdr;
  MemBuf _hdrRefBuf;
  MemBuf _hdrAuxBuf;
  
  // get() chunk refs and data
  
  bool _getRefsOnly;
  bool _respectZeroTypes;
  bool _enableDefrag;
  get_unique_t _getUnique;
  int _nGetChunks;
  MemBuf _getRefBuf;  // buffer for chunk refs for gets
  MemBuf _getAuxBuf;  // buffer for aux refs for gets
  MemBuf _getDataBuf; // buffer for data chunks for gets
  vector<chunk_t> _chunksFromGet;
  vector<compression_t> _storedCompression;
  
  // Option to check write times on get.
  // Spdb records the time at which the data are written.
  // If set, only return data stored at or before
  //   the latest valid write time.
  
  bool _checkWriteTimeOnGet;
  time_t _latestValidWriteTime;
  
  // put attributes
  
  put_mode_t _putMode;
  int _nPutChunks;
  MemBuf _putRefBuf;  // buffer for chunk refs for puts
  MemBuf _putAuxBuf;  // buffer for aux refs for puts
  MemBuf _putDataBuf; // buffer for data chunks for puts
  time_t _latestValidTimePut;
  lead_time_storage_t _leadTimeStorage;

  // compression control

  compression_t _chunkCompressOnPut;
  bool _chunkUncompressOnGet;

  // auxiliary XML buffer
  // may be used to pass extra information from a client
  // to a server

  string _auxXml;
  
  // flags etc
  
  bool _locked;
  bool _emptyDay;
  int _openDay;
  
  // file ops
  
  int _indxFd;
  int _dataFd;
  FILE *_indxFile;
  FILE *_dataFile;
  FILE *_lockFile;
  open_mode_t _openMode;
  bool _filesOpen;
  
  // times from getTimes()
  
  time_t _firstTime;
  time_t _lastTime;
  time_t _lastValidTime;
  
  // time list
  
  vector<time_t> _timeList;
  
  // errors
  
  mutable string _errStr;
  
  // protected functions
  
  // special add used by DsSpdbMsg
  // no compression performed
  
  void _addPutChunks(int n_chunks,
                     const chunk_ref_t *chunk_refs,
                     const aux_ref_t *aux_refs,
                     const void *chunk_data);
  
  int _put(int prod_id, const string &prod_label);
  
  int _erase();
  
  int _getFirstAndLastTimes(time_t &first_time,
                            time_t &last_time);

  int _getLastValid(time_t &last_valid_time,
                    int data_type,
                    int data_type2);

  int _fetchExact(time_t request_time,
                  int data_type, int data_type2);
  
  int _getExact(time_t request_time,
                int data_type,
                int data_type2);
  
  int _getClosest(time_t request_time,
                  int time_margin,
                  int data_type,
                  int data_type2);
  
  int _getFirstBefore(time_t request_time,
                      int time_margin,
                      int data_type,
                      int data_type2);
  
  int _getFirstAfter(time_t request_time,
                     int time_margin,
                     int data_type,
                     int data_type2);
  
  int _getInterval(time_t start_time,
                   time_t end_time,
                   int data_type,
                   int data_type2);
  
  int _getValid(time_t request_time,
                int data_type,
                int data_type2);
  
  int _getLatest(int time_margin,
                 int data_type,
                 int data_type2);
  
  int _getTimes();
  
  int _compileTimeList(time_t start_time,
                       time_t end_time,
                       size_t minimum_interval);
  
  int _openFiles(int prod_id,
                 const string &prod_label,
                 time_t valid_time,
                 open_mode_t mode,
                 bool read_chunk_refs = true);
  
  int _openReadWrite(int prod_id,
                     open_mode_t mode,
                     bool read_chunk_refs);
  
  int _openCreate(int prod_id,
                  const string &prod_label,
                  time_t valid_time,
                  open_mode_t mode);
  
  int _checkOpen(int prod_id,
                 const string &prod_label,
                 time_t valid_time,
                 open_mode_t mode,
                 bool read_chunk_refs = true);

  void _initHdr(int prod_id,
                const string &prod_label,
                time_t valid_time);

  void _closeFiles(bool sync = true);

  int _setLock(open_mode_t mode);

  bool _ignoreLock();

  int _clearLock();

  void _addIntErr(const char *err_str, int iarg);

  void _addStrErr(const char *err_str, const string &sarg);

  void _printIndxHeader(header_t &hdr, ostream &out,
                        bool print_min_posn = false);

  void _clearErrStr() const;

  int _firstPosnAfter(time_t start_time);

  int _getFirstTimeAfter(time_t request_time,
                         time_t end_time,
                         int data_type,
                         int data_type2,
                         time_t &data_time);

  int _getFirstTimeBefore(time_t request_time,
                          time_t start_time,
                          int data_type,
                          int data_type2,
                          time_t &data_time);

  int _getTimeNearest(time_t request_time,
                      int time_margin,
                      int data_type,
                      int data_type2,
                      time_t &time_nearest);

  void _clearGet();

  void _clearTimeList();

  void _loadChunksFromGet();

  int _posn1MinAhead(int start_posn);

  int _posnAtTime(time_t valid_time,
                  int data_type,
                  int data_type2);

  void _readChunkRefs();

  int _checkTypeThenReadChunk(int data_type,
                              int data_type2,
                              const chunk_ref_t &ref,
                              const aux_ref_t &aux,
                              MemBuf &readBuf);
 
  int _readChunk(chunk_ref_t &ref, aux_ref_t &aux,
                 MemBuf &buf, bool doUncompress);

  int _setEarliestValid(time_t valid_time, time_t expire_time);

  int _storeChunk(const chunk_ref_t *input_ref,
                  const aux_ref_t *input_aux,
                  const void *input_data);

  int _storedPosn(time_t valid_time,
                  int data_type, 
                  int data_type2);

  int _checkStored(const chunk_ref_t &inref,
                   const void *input_data);

  int _addChunkRef(const chunk_ref_t &inref,
                   const aux_ref_t &inaux);

  int _eraseChunks(time_t valid_time, int data_type, int data_type2);

  void _eraseChunkRef(time_t valid_time, int posn);

  int _writeIndxFile(bool write_refs = true);

  int _writeChunk(chunk_ref_t &inref,
                  const void *input_data,
                  bool append);

  int _defrag();

  bool _acceptRef(int data_type,
                  int data_type2,
                  const chunk_ref_t &ref,
                  const aux_ref_t &aux);

private:

};

#undef _in_Spdb_hh

#endif


