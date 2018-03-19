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
// Spdb/DsSpdb.hh
//
// DsSpdb class
//
// This class handles the DsSpdb operations with both the local
// disk and via the DsSpdbServer.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
////////////////////////////////////////////////////////////////

#ifndef DsSpdb_HH
#define DsSpdb_HH

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <time.h>
#include <didss/DsURL.hh>
#include <toolsa/MemBuf.hh>
#include <Spdb/Spdb.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <toolsa/ThreadSocket.hh>
using namespace std;

///////////////////////////////////////////////////////////////
// class definition

class DsSpdb : public Spdb

{

public:

  // constructor

  DsSpdb();

  // destructor
  
  virtual ~DsSpdb();

  // set the debugging state

  void setDebug(bool debug = true) { _debug = debug; }

  /////////////////////////////////
  // set or clear horizontal limits
  //
  // Only relevant for get requests to servers which can interpret
  // the SPDB data spatially, e.g. the Symprod servers.
  
  void setHorizLimits(double min_lat,
		      double min_lon,
		      double max_lat,
		      double max_lon);
  
  void clearHorizLimits();
  
  ///////////////////////////////
  // set or clear vertical limits
  //
  // heights will generally be specified in km msl, though there
  // may be instances in which the client and server agree on
  // a different convention.
  //
  // Only relevant for get requests to servers which can interpret
  // the SPDB data spatially, e.g. the Symprod servers.
  
  void setVertLimits(double min_ht,
		     double max_ht);
  
  void clearVertLimits();
  
  /////////////////////////////////
  // set the limits in the message
  
  void setLimitsInMsg(DsSpdbMsg &msg);

  // Set data compression for transfer.
  // The following compression types can be specified:
  //    Spdb::COMPRESSION_NONE
  //    Spdb::COMPRESSION_GZIP
  //    Spdb::COMPRESSION_BZIP2
  // If set, data will be compressed before transmission,
  // and uncompressed on the receiving end. This applies to
  // both putting and getting data.
  // The default is COMPRESSION_NONE.

  void setDataCompressForTransfer(compression_t compression);

  /////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////
  //
  // put functions:
  //
  //   The put functions put one or more data chunks to one or more URLs.
  //
  // Put modes:
  //
  //   There are 3 modes for putting:
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
  //   putModeOver, putModeAdd or putModeOnce.
  //
  // The put functions are overloaded - you need to use the argument
  // list relevant to your application.
  //
  // Decision 1: one or more chunks?
  //
  //   For putting a single chunk, use the put functions which allow you to
  //   specify data_type, valid_time, expire_time, chunk_len and chunk_data.
  //
  //   For putting multiple chunks:
  //     (a) clear the chunk buffer using clear_chunk()
  //     (b) add each chunk using addChunk()
  //     (c) perform the put using the put functions which do not allow
  //         you to specify data_type etc.
  //
  // NOTE: puts to multiple URLs are deprecated, use the DsSpdbServer
  //       for data distribution instead.
  // Decision 2: one or more URLs?
  //
  //   For putting to a single URL, use the put functions which allow you
  //   to specify the URL.
  //
  //   For putting to multiple URLs:
  //     (a) clear the URL list using clearUrls()
  //     (b) add each URL using addUrl()
  //     (c) perform the put using the functions which do not allow 
  //         you to specify a URL.
  //
  // Return values:
  //
  //   The put functions return 0 on success, -1 on error.
  //   In non-threaded mode, return code is from the put(s).
  //   In threaded mode, return code is from child creation.
  //   For puts to multiple URLs, an error is returned if any of the puts
  //   generate an error.
  //
  // Error messages:
  //
  //   Error messages are printed to stderr.
  //
  /////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////

  ////////////////////////////////////
  // set attributes for put functions

  ////////////////////////////////////////////////////
  // clear URLs before creating list using 'addUrl'

  void clearUrls();

  ////////////////////////////////////
  // Add a URL to the destination list

  void addUrl(const string &url_str);

  ////////////////////////////////////////////////////
  // See SPdb.hh for:
  //   clearPutChunks()
  //   setPutMode()
  //   addPutChunk()
  
  //////////////////////////////////////////////////////////
  // put - single chunk to single URL
  // Overrides Spdb function.
  //
  // Chunk must already be in BE byte order, as appropriate.
  //
  // Returns 0 on success, -1 on error.
  
  virtual int put(const string &url_str,
		  int prod_id,
		  const string &prod_label,
		  int data_type,
		  time_t valid_time,
		  time_t expire_time,
		  int chunk_len,
		  const void *chunk_data,
		  int data_type2 = 0);
  
  //////////////////////////////////////////////////////////
  // put - multiple chunks to single URL
  //
  // Chunks must already be in BE byte order, as appropriate.
  //
  // Returns 0 on success, -1 on error

  virtual int put(const string &url_str,
		  int prod_id,
		  const string &prod_label);

  ///////////////////////////////////////////////////////////
  // put - single chunk to multiple URLs
  // Overrides Spdb function.
  //
  // Chunk must already be in BE byte order, as appropriate.
  //
  // Returns 0 on success, -1 on error
  
  virtual int put(int prod_id,
		  const string &prod_label,
		  int data_type,
		  time_t valid_time,
		  time_t expire_time,
		  int chunk_len,
		  const void *chunk_data,
		  int data_type2 = 0);

  //////////////////////////////////////////////////////////////
  // put - multiple chunks to multiple URLs
  //
  // Chunks must already be in BE byte order, as appropriate.
  //
  // Returns 0 on success, -1 on error

  virtual int put(int prod_id,
		  const string &prod_label);
  
  ///////////////////////////////////////////////////////////////////
  // erase()
  //
  // Erase data for a given valid time and data type.
  // If the data_type is 0, all data at that time is erased.
  //
  // Returns 0 on success, -1 on failure

  virtual int erase(const string &url_str,
		    time_t valid_time,
		    int data_type = 0,
		    int data_type2 = 0);

  ///////////////////////////////////////////////////////////////////
  // Erase data for a given set of chunk refs.
  // Before calling this function, call clearPutChunks(),
  // and addPutChunk() for each time and data_type you want to erase.
  // You should call addPutChunk() with 0 length and NULL pointer.
  // If the data_type is 0, all data at that time is erased.
  // This is a special type of 'put'.
  //
  // Returns 0 on success, -1 on failure
  
  virtual int erase(const string &url_str);

  // Do the put() as specified in the message
  // This is strictly local operation.
  // Only the dir section of the URL is used.
  // Returns 0 on success, -1 on failure.

  int doMsgPut(const DsSpdbMsg &inMsg);
 
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  //
  // get functions
  //
  // The get() functions get spdb data from the database pointed to
  // by the URL.
  //
  // There are a variety of functions, each of which specifies the
  // time search in a different manner.
  //
  // If data_type == 0, all relevant chunks are returned.
  // If data_type != 0, only chunks with that data type are returned. 
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
  // See Spdb.hh for setProdId()

  ///////////////////////////////////////////////////////////////////
  // access to info and chunk refs and data after a get()
  // See Spdb.hh for:
  //   getProdId()
  //   getProdLabel()
  //   getNChunks()
  //   getChunkRefs()
  //   getChunkData()
  //   getChunks()

  ///////////////////////////////////////////////////////////////////
  // getExact() - overrides Spdb
  //
  // Get data at exactly the given time.
  //

  virtual int getExact(const string &url_str,
		       time_t request_time,
		       int data_type = 0,
		       int data_type2 = 0,
		       bool get_refs_only = false,
		       bool respect_zero_types = false);
  
  ///////////////////////////////////////////////////////////////////
  // getClosest() - overrides Spdb
  //
  // Get data closest to the given time, within the time margin.
  //

  virtual int getClosest(const string &url_str,
			 time_t request_time,
			 int time_margin,
			 int data_type = 0,
			 int data_type2 = 0,
			 bool get_refs_only = false,
			 bool respect_zero_types = false);

  ///////////////////////////////////////////////////////////////////
  // getInterval() - overrides Spdb
  //
  // Get data in the time interval.
  //
  
  virtual int getInterval(const string &url_str,
			  time_t start_time,
			  time_t end_time,
			  int data_type = 0,
			  int data_type2 = 0,
			  bool get_refs_only = false,
			  bool respect_zero_types = false);

  ///////////////////////////////////////////////////////////////////
  // getValid() - overrides Spdb
  //
  // Get data valid at the given time.
  //

  virtual int getValid(const string &url_str,
		       time_t request_time,
		       int data_type = 0,
		       int data_type2 = 0,
		       bool get_refs_only = false,
		       bool respect_zero_types = false);
  
  ///////////////////////////////////////////////////////////////////
  // getLatest() - overrides Spdb
  //
  // Get latest data, within the given time margin. This is the
  // same as getInterval(latestTime - margin, latestTime).
  //

  virtual int getLatest(const string &url_str,
			int time_margin = 0,
			int data_type = 0,
			int data_type2 = 0,
			bool get_refs_only = false,
			bool respect_zero_types = false);

  ///////////////////////////////////////////////////////////////////
  // getFirstBefore() - overrides Spdb
  //
  // Get first data at or before the requested time,
  // within the given time margin.
  //

  virtual int getFirstBefore(const string &url_str,
			     time_t request_time,
			     int time_margin,
			     int data_type = 0,
			     int data_type2 = 0,
			     bool get_refs_only = false,
			     bool respect_zero_types = false);
  
  ///////////////////////////////////////////////////////////////////
  // getFirstAfter() - overrides Spdb
  //
  // Get first data at or after the requested time,
  // within the given time margin.
  //

  virtual int getFirstAfter(const string &url_str,
			    time_t request_time,
			    int time_margin,
			    int data_type = 0,
			    int data_type2 = 0,
			    bool get_refs_only = false,
			    bool respect_zero_types = false);

  ////////////////////////////////////////////////////////////
  // get the first, last and last_valid_time in the data base
  // verrides Spdb
  //
  // In this case, no chunk data is returned.

  virtual int getTimes(const string &url_str,
		       time_t &first_time,
		       time_t &last_time,
		       time_t &last_valid_time);

  ////////////////////////////////////////////////////////////
  // get the first, last and last_valid_time in the data base
  // Use getFirstTime(), getLastTime() and getLastValidTime()
  // to access the times after making this call.
  //
  // In this case, no chunk data is returned.

  virtual int getTimes(const string &dir);
  
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
  
  virtual int compileTimeList(const string &url_str,
			      time_t start_time,
			      time_t end_time,
			      size_t minimum_interval = 1);
  
  // Do the get() as specified in the message.
  // Loads the getInfo struct.
  // This is a strictly local operation.
  // Only the dir section of the URL is used.
  // Returns 0 on success, -1 on failure

  int doMsgGet(const DsSpdbMsg &inMsg, DsSpdbMsg::info_t &getInfo);
  
  // horizontal and vertical limits

  bool areHorizLimitsSet() const { return _horizLimitsSet; }
  double getMinLat() const { return _minLat; }
  double getMinLon() const { return _minLon; }
  double getMaxLat() const { return _maxLat; }
  double getMaxLon() const { return _maxLon; }

  bool areVertLimitsSet() const { return _vertLimitsSet; }
  double getMinHt() const { return _minHt; }
  double getMaxHt() const { return _maxHt; }

  ////////////////////////////////////////////////////////////////////
  // makeUnique() - see Spdb.hh
  
  ///////////////////////////////////////////////////////////////////
  // print functions
  //
  // See Spdb.hh for:
  //   printHeader()
  //   printChunkRef()
  
  ///////////////////////////////////////////////////////////////////
  // error string
  //
  // See Spdb.hh for:
  //   getErrStr()
  //   getErrorStr()
  //   getErrorString()

protected:

  // inner class for put arguments

  class PutArgs {
    
  public:
    
    // constructor
    
    PutArgs(const string &url_str,
            int child_timeout_secs) :
            urlStr(url_str),
            childTimeoutSecs(child_timeout_secs),
            childExpireTime(time(NULL) + childTimeoutSecs),
            childRunning(false),
            childDone(false) {}
    
    string urlStr;
    int childTimeoutSecs;
    time_t childExpireTime;
    pid_t childPid;
    bool childRunning;
    bool childDone;
    
  protected:
  private:
    
  };

  // debugging

  bool _debug;

  // url
  
  string _urlStr;
  DsURL _url;

  // put attributes

  bool _putInChild;
  int _maxNChildren;
  int _childTimeoutSecs;
  int _childCount;
  bool _registerWhileCleaning;
  list<PutArgs *> _childList;

  vector<string> _urlStrings;

  // is the URL local?

  bool _isLocal;

  // socket to server
  
  ThreadSocket _sock;

  // horizontal and vertical limits

  bool _horizLimitsSet;
  double _minLat, _minLon, _maxLat, _maxLon;

  bool _vertLimitsSet;
  double _minHt, _maxHt;
  
  // compression control
  // should we compress data for transfer?

  compression_t _dataCompressForTransfer;

  // protected functions

  int _reapChildren(bool cancel_uncompleted = false);

  int _remotePut(int prod_id,
		 const string &prod_label);

  int _localPut(int prod_id,
		const string &prod_label);

  int _setUrl(const string &url_str);

  int _communicateGet(const DsSpdbMsg &inMsg,
                      const DsURL &url);

  int _communicatePut(const DsSpdbMsg &inMsg,
		      const DsURL &url,
                      DsSpdbMsg &replyMsg);
  
  int _communicate(const DsSpdbMsg &inMsg,
		   const DsURL &url,
                   DsSpdbMsg &replyMsg);

  void *_doRemotePut(PutArgs *pArgs,
                     const DsSpdbMsg &putMsg);
  
  void _loadChunkData(int prod_id,
                      const char *prod_label,
                      int n_chunks,
                      const Spdb::chunk_ref_t *chunk_refs,
                      const Spdb::aux_ref_t *aux_refs,
                      bool get_refs_only,
                      const void *chunk_data,
                      int chunk_data_len);

private:

  // the following will be deprecated

  // Threading:
  //
  //   By default threading is not used for puts.
  //
  //   To control threading, use the functions:
  //     
  //     setPutThreadingOn() - each put occurs in a separate child.
  //     setPutThreadingOff() - puts are sequential
  //
  //   If threading is turned on, each put is performed in a separate
  //   child, so that control is returned to the program immediately
  //   after the children are created. The function cleanThreads() will
  //   clean up after the children. This function will optinally wait
  //   until all children have completed. The function is called in the
  //   wait mode in the destructor. It also has a mode which will
  //   cancel remaining children.
  //

  //////////////////////////////////////////////////
  // set threading for puts on or off. Default is on.
  // If on, puts are done in child processes.

  void setPutThreadingOn();
  void setPutThreadingOff();
  void setPutThreadTimeout(int secs) { _childTimeoutSecs = secs; }

  typedef enum {
    CLEAN_COMPLETED_ONLY,
    WAIT_TO_COMPLETE,
    CANCEL_INCOMPLETE
  } thread_cleanup_mode_t;
  
  // cleanup - cleanThreads with the default args is called in
  // the destructor. To change the behavior, call cleanThreads 
  // before the destructor ins invoked.
  //
  // By default, cleanThreads registers with procmap if waiting
  // for threads to complete. Use setNoregisterWhileCleaning()
  // to turn this off.

  void cleanThreads(thread_cleanup_mode_t mode = WAIT_TO_COMPLETE);

  void setNoRegisterWhileCleaning() { _registerWhileCleaning = false; }

  /////////////////////////////////////////////
  // set max number of threads - default is 128

  void setMaxNThreads(int max_n_threads);

};

#endif


