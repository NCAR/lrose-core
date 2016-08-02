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
// spdb/Spdb.hh
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

#ifndef Spdb_HH
#define Spdb_HH

#include <string>
#include <vector>
#include <iostream>
#include <didss/DsURL.hh>
#include <toolsa/MemBuf.hh>
#include <spdb/SpdbMsg.hh>
#include <toolsa/ThreadSocket.hh>
using namespace std;

///////////////////////////////////////////////////////////////
// class definition

class Spdb : public ThreadSocket

{

public:

  // chunk struct

  typedef struct {
    time_t valid_time;
    time_t expire_time;
    int data_type;
    int len;
    void *data;
  } chunk_t;

  // mode enum

  typedef enum {putModeOver, putModeAdd, putModeOnce} put_mode_t;
  
  // constructor

  Spdb();

  // destructor
  
  virtual ~Spdb();

  
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
  // Use the function setPutMode() to set the mode to one of the following:
  //   putModeOver, putModeAdd or putModeOnce.
  //
  // Threading:
  //
  //   By default all puts are performed in a separate thread, so that
  //   control is returned to the program immediately after the threads
  //   are created.
  //
  //   To control threading, use the functions:
  //     
  //     setPutThreadingOn() - each put occurs in a separate thread.
  //     setPutThreadingOff() - puts are sequential
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
  //         yout to specify data_type etc.
  //
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
  //   In threaded mode, return code is from thread creation.
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

  //////////////////////////////////////////
  // set threading on or off. Default is on.

  void setPutThreadingOn();
  void setPutThreadingOff();

  /////////////////////////////////////////////
  // set max number of threads - default is 128

  void setMaxNThreads(int max_n_threads);

  ////////////////////////////////////////////////////
  // clear URLs before creating list using 'addUrl'

  void clearUrls();

  ////////////////////////////////////
  // Add a URL to the destination list

  void addUrl(const string &url_str);

  ////////////////////////////////////////////////////
  // set the put mode
  // Options are: putModeOver, putModeAdd, putModeOnce
  // Default is putModeOver
  
  void setPutMode(const put_mode_t mode);

  ////////////////////////////////////////////////////
  // clear chunks before creating buffer using 'addChunk'

  void clearChunks();

  ////////////////////////////////////
  // Add a chunk to the chunk buffer

  void addChunk(const int data_type,
		const time_t valid_time,
		const time_t expire_time,
		const int chunk_len,
		const void *chunk_data);

  //////////////////////////////////////////////////////////
  // put - single chunk to single URL
  //
  // Chunk must already be in BE byte order, as appropriate.
  //
  // Returns 0 on success, -1 on error.
  
  int put(const string &url_str,
	  const int prod_id,
	  const string &prod_label,
	  const int data_type,
	  const time_t valid_time,
	  const time_t expire_time,
	  const int chunk_len,
	  const void *chunk_data);

  ///////////////////////////////////////////////////////////
  // put - single chunk to multiple URLs
  //
  // Chunk must already be in BE byte order, as appropriate.
  //
  // Returns 0 on success, -1 on error
  
  int put(const int prod_id,
	  const string &prod_label,
	  const int data_type,
	  const time_t valid_time,
	  const time_t expire_time,
	  const int chunk_len,
	  const void *chunk_data);

  //////////////////////////////////////////////////////////
  // put - multiple chunks to single URL
  //
  // Chunks must already be in BE byte order, as appropriate.
  //
  // Returns 0 on success, -1 on error

  int put(const string &url_str,
	  const int prod_id,
	  const string &prod_label);

  //////////////////////////////////////////////////////////////
  // put - multiple chunks to multiple URLs
  //
  // Chunks must already be in BE byte order, as appropriate.
  //
  // Returns 0 on success, -1 on error

  int put(const int prod_id,
	  const string &prod_label);
  
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
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  // specifying the product ID.
  //
  // The product ID defaults to 0, which data from any product ID will
  // be returned. If you wish to make sure that only data of a certain
  // product ID is returned, set the required ID before the get.

  void setProdId(int id) { _prodId = id; }

  ///////////////////////////////////////////////////////////////////
  // access to info and chunk refs and data after a get()

  int getProdId() { return (_prodId); }
  string &getProdLabel() { return (_prodLabel); }
  int getNChunks() { return (_nChunks); }
  vector<chunk_t> &getChunks() { return (_chunks); }
  spdb_chunk_ref_t *getChunkRefs() { return (_chunkRefs); }
  void *getChunkData() { return (_chunkData); }

  ///////////////////////////////////////////////////////////////////
  // getExact()
  //
  // Get data at exactly the given time.
  //

  int getExact(const string &url_str,
	       const time_t request_time,
	       const int data_type = 0);
  
  ///////////////////////////////////////////////////////////////////
  // getClosest()
  //
  // Get data closest to the given time, within the time margin.
  //

  int getClosest(const string &url_str,
		 const time_t request_time,
		 const int time_margin,
		 const int data_type = 0);

  ///////////////////////////////////////////////////////////////////
  // getInterval()
  //
  // Get data in the time interval.
  //

  int getInterval(const string &url_str,
		  const time_t start_time,
		  const time_t end_time,
		  const int data_type = 0);

  ///////////////////////////////////////////////////////////////////
  // getValid()
  //
  // Get data valid at the given time.
  //

  int getValid(const string &url_str,
	       const time_t request_time,
	       const int data_type = 0);
  
  ///////////////////////////////////////////////////////////////////
  // getLatest()
  //
  // Get latest data, within the given time margin. This is the
  // same as getInterval(latestTime - margin, latestTime).
  //

  int getLatest(const string &url_str,
		const int time_margin = 0,
		const int data_type = 0);

  ///////////////////////////////////////////////////////////////////
  // getFirstBefore()
  //
  // Get first data at or before the requested time,
  // within the given time margin.
  //

  int getFirstBefore(const string &url_str,
		     const time_t request_time,
		     const int time_margin,
		     const int data_type = 0);
  
  ///////////////////////////////////////////////////////////////////
  // getFirstAfter()
  //
  // Get first data at or after the requested time,
  // within the given time margin.
  //

  int getFirstAfter(const string &url_str,
		    const time_t request_time,
		    const int time_margin,
		    const int data_type = 0);

  ////////////////////////////////////////////////////////////
  // get the first, last and last_valid_time in the data base
  //
  // In this case, no chunk data is returned.

  int getTimes(const string &url_str,
	       time_t &first_time,
	       time_t &last_time,
	       time_t &last_valid_time);

  ////////////////////////////////////////////////////////////////////
  // makeUnique()
  //
  // This functions filters duplicate data from the chunk data,
  // leaving the latest chunk for each data_type. All chunks with
  // data_type of 0 will be left in the buffer.
  //
  // The intention is that you can call this function after any of
  // the get routines, and filter the output of the get.
  //

  void makeUnique();
  
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  //
  // print functions
  //
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  
  ////////////////////////////
  // print given chunk header

  void printChunkRef(spdb_chunk_ref_t &chunk_ref, ostream &out);

  /////////////////////////////////////
  // print chunk header i after a get()

  void printChunkRef(int i, ostream &out);

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  //
  // error functions
  //
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  
  const string &getErrorStr() { return (_errorStr); }

protected:

private:

  // product info

  string _prodLabel;
  int _prodId;

  // url
  
  string _urlStr;
  string _dirPath;
  DsURL _url;

  // chunk refs and data

  int _nChunks;
  spdb_chunk_ref_t *_chunkRefs;
  void *_chunkData;
  MemBuf _chunkRefBuf;
  MemBuf _chunkDataBuf;
  vector<chunk_t> _chunks;

  // put attributes

  bool _putThreading;
  int _maxNThreads;
  put_mode_t _putMode;
  vector<string> _urlStrings;

  // is the URL local?

  bool _isLocal;

  // errors

  string _errorStr;

  // private functions

  int _genericPut(const string &url_str,
		  const int prod_id,
		  const string &prod_label);

  int _doLocalPut(const int prod_id,
		  const string &prod_label,
		  const int n_chunks,
		  const spdb_chunk_ref_t *chunk_refs,
		  const void *chunk_data);

  int _checkUrl(const string &url_str);

  int _communicateGet(ThreadSocket &sock,
		      SpdbMsg &msg,
		      void *buf,
		      const int buflen);

  void _copyChunkMembers(int prod_id,
			 char *prod_label,
			 int n_chunks,
			 spdb_chunk_ref_t *chunk_refs,
			 void *chunk_data,
			 int chunk_data_len);

  // Static members and functions for threading
    
  static int _threadCount;

  static void *_doRemotePut(void *args);
  
  static int _communicatePut(ThreadSocket &sock,
			     SpdbMsg &msg,
			     void *buf,
			     const int buflen,
			     DsURL &url);

};

#endif


