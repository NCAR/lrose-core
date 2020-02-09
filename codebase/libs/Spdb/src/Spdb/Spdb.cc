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

#include <Spdb/Spdb.hh>
#include <dataport/bigend.h>
#include <toolsa/file_io.h>
#include <toolsa/TaFile.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/compress.h>
#include <dsserver/DsLdataInfo.hh>
#include <didss/RapDataDir.hh>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <sys/stat.h>
#include <set>
using namespace std;

// initialize constants

int Spdb::_fileMajorVersion = 1;
int Spdb::_fileMinorVersion = 1;
const char *Spdb::_indxExt = "indx";
const char *Spdb::_dataExt = "data";

////////////////////////////////////////////////////////////
// Constructor

Spdb::Spdb() :

        _appName("unknown"),
        _prodId(0),

        _getRefsOnly(false),
        _respectZeroTypes(false),
        _enableDefrag(false),
        _getUnique(UniqueOff),
        _nGetChunks(0),
        _checkWriteTimeOnGet(false),
        _latestValidWriteTime(0),

        _putMode(putModeOver),
        _nPutChunks(0),
        _latestValidTimePut(0),
        _leadTimeStorage(LEAD_TIME_NOT_APPLICABLE),

        _chunkCompressOnPut(COMPRESSION_NONE),
        _chunkUncompressOnGet(true),

        _locked(false),
        _emptyDay(false),
        _openDay(0),

        _indxFd(-1),
        _dataFd(-1),
        _indxFile(NULL),
        _dataFile(NULL),
        _lockFile(NULL),
        _openMode(ReadMode),
        _filesOpen(false),

        _firstTime(0),
        _lastTime(0),
        _lastValidTime(0)

{

  MEM_zero(_indxPath);
  MEM_zero(_dataPath);
  MEM_zero(_lockPath);
  MEM_zero(_hdr);

}

////////////////////////////////////////////////////////////
// destructor

Spdb::~Spdb()

{
  _closeFiles();
}

///////////////////////////////////////
// functions to set the put attributes

void Spdb::setPutMode(const put_mode_t mode)
{
  _putMode = mode;
}

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

void Spdb::setLeadTimeStorage(lead_time_storage_t storage_type)
{
  _leadTimeStorage = storage_type;
  _hdr.lead_time_storage = storage_type;
}

////////////////////////////////////////////////////
// Set chunk compression for put operations.
// The following compression types can be specified:
//    Spdb::COMPRESSION_NONE
//    Spdb::COMPRESSION_GZIP
//    Spdb::COMPRESSION_BZIP2
// If set, chunks will be stored compressed and the
// compression flag will be set in the auxiliary chunk header.
// The default is COMPRESSION_NONE.

void Spdb::setChunkCompressOnPut(compression_t compression)
{
  _chunkCompressOnPut = compression;
}

////////////////////////////////////////////////////
// Set chunk uncompression for get operations.
// If set, chunks will be uncomrpessed on get, if
// they are stored using compression.
// The default is true.

void Spdb::setChunkUncompressOnGet(bool state /*= true */)

{
  _chunkUncompressOnGet = state;
}

///////////////////////////////////////////////////////////
// clear put chunks before loading buffer using addPutChunk

void Spdb::clearPutChunks()
{
  _nPutChunks = 0;
  _putRefBuf.reset();
  _putAuxBuf.reset();
  _putDataBuf.reset();
  _respectZeroTypes = false;
}

////////////////////////////////////
// Add a chunk to the chunk buffer
// The chunk data will be compressed appropriately if
// setChunkCompressOnPut() has been called.

void Spdb::addPutChunk(int data_type,
		       time_t valid_time,
		       time_t expire_time,
		       int chunk_len,
		       const void *chunk_data,
		       int data_type2 /* = 0*/,
                       const char *tag /* = NULL */)

{

  // If (chunk_len == 0) then chunk_data can be NULL as
  // what is being done is actually erasing data from the SPDB database.
  // Otherwise, though, having chunk_data set to NULL can cause problems with
  // the spdb database. In that case print something as the caller
  // needs to debug what they are doing.
  if ((chunk_data == NULL) && (chunk_len != 0)){
    cerr << "NULL entered as chunk_data to Spdb::addPutChunk() with data length ";
    cerr << chunk_len << ", disregarding" << endl;
    return;
  }

  // handle compression if required

  void *compressedBuf = NULL;
  ui64 nbytesCompressed = chunk_len;
  
  if (_chunkCompressOnPut == COMPRESSION_GZIP) {
    compressedBuf = ta_compress(TA_COMPRESSION_GZIP,
                                chunk_data,
                                chunk_len,
                                &nbytesCompressed);
  } else if (_chunkCompressOnPut == COMPRESSION_BZIP2) {
    compressedBuf = ta_compress(TA_COMPRESSION_BZIP,
                                chunk_data,
                                chunk_len,
                                &nbytesCompressed);
  }

  // ignore compression if it does not reduce the data size

  if (compressedBuf != NULL &&
      nbytesCompressed >= (unsigned int) chunk_len) {
    ta_compress_free(compressedBuf);
    compressedBuf = NULL;
  }

  // set len and pointer to data

  int storedLen = chunk_len;
  const void *data = chunk_data;
  if (compressedBuf != NULL) {
    storedLen = nbytesCompressed;
    data = compressedBuf;
  }
  
  // load ref
  
  chunk_ref_t ref;
  ref.data_type = data_type;
  ref.data_type2 = data_type2;
  ref.valid_time = valid_time;
  ref.expire_time = expire_time;
  ref.len = storedLen;
  ref.offset = _putDataBuf.getLen();

  // load auxiliary ref

  aux_ref_t aux;
  MEM_zero(aux);
  aux.write_time = (ti32) time(NULL);
  if (compressedBuf != NULL) {
    aux.compression = _chunkCompressOnPut;
  }

  if (tag != NULL) {
    int nn = strlen(tag);
    if (nn > TAG_LEN - 1) {
      nn = TAG_LEN - 1;
    }
    strncpy(aux.tag, tag, nn);
  }

  // add to buffers
  
  _nPutChunks++;
  _putRefBuf.add(&ref, sizeof(chunk_ref_t));
  _putAuxBuf.add(&aux, sizeof(aux_ref_t));
  _putDataBuf.add(data, storedLen);

  // free up as required

  if (compressedBuf != NULL) {
    ta_compress_free(compressedBuf);
  }

}

//////////////////////////////////////////////
// Add an array of chunks to the chunk buffers
// Note - it is preferable to use addPutChunk
// in a loop instead.
// The chunk data will be compressed appropriately if
// setChunkCompressOnPut() has been called.

void Spdb::addPutChunks(int n_chunks,
			const chunk_ref_t *chunk_refs,
			const void *chunk_data)

{

  const char *data = (const char *) chunk_data;
  for (int ii = 0; ii < n_chunks; ii++) {
    const chunk_ref_t &ref = chunk_refs[ii];
    addPutChunk(ref.data_type,
                ref.valid_time,
                ref.expire_time,
                ref.len,
                data + ref.offset,
                ref.data_type2);
  }
  
}

//////////////////////////////////////////////
// Add chunks - used by server message classes
// No change in compression.

void Spdb::_addPutChunks(int n_chunks,
                         const chunk_ref_t *chunk_refs,
                         const aux_ref_t *aux_refs,
                         const void *chunk_data)
  
{
  
  aux_ref_t aux;
  MEM_zero(aux);
  aux.write_time = (ti32) time(NULL);

  int dataLen = 0;
  for (int i = 0; i < n_chunks; i++) {
    _nPutChunks++;
    _putRefBuf.add(chunk_refs + i, sizeof(chunk_ref_t));
    if (aux_refs == NULL) {
      _putAuxBuf.add(&aux, sizeof(aux_ref_t));
    } else {
      _putAuxBuf.add(aux_refs + i, sizeof(aux_ref_t));
    }
    dataLen += chunk_refs[i].len;
  }

  _putDataBuf.add(chunk_data, dataLen);
  

}

//////////////////////////////////////////////////////////
// put - single chunk
//
// Chunk must already be in BE byte order, as appropriate.
//
// The chunk data will be compressed appropriately if
// setChunkCompressOnPut() has been called.
//
// Returns 0 on success, -1 on error.

int Spdb::put(const string &dir,
	      int prod_id,
	      const string &prod_label,
	      int data_type,
	      time_t valid_time,
	      time_t expire_time,
	      int chunk_len,
	      const void *chunk_data,
	      int data_type2 /* = 0*/ )

{

  clearPutChunks();
  addPutChunk(data_type, valid_time, expire_time,
	      chunk_len, chunk_data, data_type2);
  return (put(dir, prod_id, prod_label));

}

//////////////////////////////////////////////////////////
// put chunks which have been added with
// addPutChunk() or addPutChunks()
// Returns 0 on success, -1 on error

int Spdb::put(const string &dir,
	      int prod_id,
	      const string &prod_label)
  
{

  if (_nPutChunks < 1) {
    return 0;
  }

  _clearErrStr();
  _errStr += "Spdb::put\n";

  _dir = dir;
  _setLock(WriteMode);
  
  int iret = _put(prod_id, prod_label);

  _clearLock();
  return iret;

}

///////////////////////////////////////////////////////////////////
// erase()
//
// Erase data for a given valid time and data type.
// If the data_type is 0, the data_type is not considered in the erase.
// Similarly for data_type2.
//
// Returns 0 on success, -1 on failure

int Spdb::erase(const string &dir,
		time_t valid_time,
		int data_type /* = 0*/,
		int data_type2 /* = 0*/ )

{

  clearPutChunks();
  addPutChunk(data_type, valid_time, valid_time, 0, NULL, data_type2);
  return (erase(dir));

}

///////////////////////////////////////////////////////////////////
// Erase data for a given set of chunk refs.
// Before calling this function, call clearPutChunks(),
// and addPutChunk() for each time and data_type you want to erase.
// You can call addPutChunk() with 0 length and NULL pointer.
// If the data_type is 0, all data at that time is erased.
// This is a special type of 'put'.
//
// Returns 0 on success, -1 on failure

int Spdb::erase(const string &dir)

{
  
  _clearErrStr();
  _errStr += "Spdb::erase\n";

  _dir = dir;
  _setLock(WriteMode);
  
  int iret = _erase();

  _clearLock();
  return iret;

}

///////////////////////////////////////////////////////////////////
// getExact()
//
// Get data at exactly the given time.
//

int Spdb::getExact(const string &dir,
		   time_t request_time,
		   int data_type /* = 0*/,
		   int data_type2 /* = 0*/,
		   bool get_refs_only /* = false*/,
		   bool respect_zero_types /* = false*/ )

{

  _clearGet();
  _clearErrStr();
  _errStr += "Running Spdb::getExact\n";

  _dir = dir;
  _setLock(ReadMode);

  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;
  
  int iret = _getExact(request_time, data_type, data_type2);
  _loadChunksFromGet();

  _clearLock();
  return iret;

}

///////////////////////////////////////////////////////////////////
// getClosest()
//
// Get data closest to the given time, within the time margin.
//

int Spdb::getClosest(const string &dir,
		     time_t request_time,
		     int time_margin,
		     int data_type /* = 0*/,
		     int data_type2 /* = 0*/,
		     bool get_refs_only /* = false*/,
		     bool respect_zero_types /* = false*/ )

{

  _clearGet();
  _clearErrStr();
  _errStr += "Running Spdb::getClosest\n";

  _dir = dir;
  _setLock(ReadMode);

  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;
  
  int iret = _getClosest(request_time, time_margin,
			 data_type, data_type2);
  _loadChunksFromGet();

  _clearLock();
  return iret;

}

///////////////////////////////////////////////////////////////////
// getFirstBefore()
//
// Get first data at or before the requested time.
//

int Spdb::getFirstBefore(const string &dir,
			 time_t request_time,
			 int time_margin,
			 int data_type /* = 0*/,
			 int data_type2 /* = 0*/,
			 bool get_refs_only /* = false*/,
			 bool respect_zero_types /* = false*/ )

{

  _clearGet();
  _clearErrStr();
  _errStr += "Running Spdb::getFirstBefore\n";

  _dir = dir;
  _setLock(ReadMode);

  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;
  
  int iret = _getFirstBefore(request_time, time_margin,
			     data_type, data_type2);
  _loadChunksFromGet();

  _clearLock();
  return iret;
  
}
  
///////////////////////////////////////////////////////////////////
// getFirstAfter()
//
// Get first data at or after the requested time.
//

int Spdb::getFirstAfter(const string &dir,
			time_t request_time,
			int time_margin,
			int data_type /* = 0*/,
			int data_type2 /* = 0*/,
			bool get_refs_only /* = false*/,
			bool respect_zero_types /* = false*/ )

{

  _clearGet();
  _clearErrStr();
  _errStr += "Running Spdb::getFirstAfter\n";

  _dir = dir;
  _setLock(ReadMode);

  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;
  
  int iret = _getFirstAfter(request_time, time_margin,
			    data_type, data_type2);
  _loadChunksFromGet();

  _clearLock();
  return iret;
  
}

///////////////////////////////////////////////////////////////////
// getInterval()
//
// Get data in the time interval.
//

int Spdb::getInterval(const string &dir,
		      time_t start_time,
		      time_t end_time,
		      int data_type /* = 0*/,
		      int data_type2 /* = 0*/,
		      bool get_refs_only /* = false*/,
		      bool respect_zero_types /* = false*/ )

{

  _clearGet();
  _clearErrStr();
  _errStr += "Running Spdb::getInterval\n";

  _dir = dir;
  _setLock(ReadMode);

  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;
  
  int iret = _getInterval(start_time, end_time,
			  data_type, data_type2);
  _loadChunksFromGet();

  _clearLock();
  return iret;

}
  
///////////////////////////////////////////////////////////////////
// getValid()
//
// Get data valid at the given time.
//

int Spdb::getValid(const string &dir,
		   time_t request_time,
		   int data_type /* = 0*/,
		   int data_type2 /* = 0*/,
		   bool get_refs_only /* = false*/,
		   bool respect_zero_types /* = false*/ )

{

  _clearGet();
  _clearErrStr();
  _errStr += "Running Spdb::getValid\n";

  _dir = dir;
  _setLock(ReadMode);

  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;

  int iret = _getValid(request_time, data_type, data_type2);
  _loadChunksFromGet();

  _clearLock();
  return iret;

}

///////////////////////////////////////////////////////////////////
// getLatest()
//
// Get latest data.
//

int Spdb::getLatest(const string &dir,
		    int time_margin /* = 0*/,
		    int data_type /* = 0*/,
		    int data_type2 /* = 0*/,
		    bool get_refs_only /* = false*/,
		    bool respect_zero_types /* = false*/ )

{

  _clearGet();
  _clearErrStr();
  _errStr += "Running Spdb::getLatest\n";

  _dir = dir;
  _setLock(ReadMode);

  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;

  int iret = _getLatest(time_margin, data_type, data_type2);
  
  _loadChunksFromGet();

  _clearLock();
  return iret;

}

////////////////////////////////////////////////////////////
// get the first, last and last_valid_time in the data base
// In this case, no chunk data is returned.
//
// Returns 0 on success, -1 on failure

int Spdb::getTimes(const string &dir)
  
{

  _clearErrStr();
  _errStr += "Running Spdb::getTimes\n";
  
  _dir = dir;
  _setLock(ReadMode);
  
  int iret = _getTimes();

  _clearLock();
  return iret;
  
}
  
////////////////////////////////////////////////////////////
// get the first, last and last_valid_time in the data base
// In this case, no chunk data is returned.
//
// Returns 0 on success, -1 on failure

int Spdb::getTimes(const string &dir,
		   time_t &first_time,
		   time_t &last_time,
		   time_t &last_valid_time)
  
{

  if (getTimes(dir)) {
    return -1;
  }

  first_time = _firstTime;
  last_time = _lastTime;
  last_valid_time = _lastValidTime;
  
  if (_checkWriteTimeOnGet) {
    if (first_time > _latestValidWriteTime) {
      _errStr += "ERROR - Spdb::getTimes\n";
      _addStrErr("  Checking latest valid write time: ",
                 utimstr(_latestValidWriteTime));
      _errStr += "  No data before this time.\n";
      return -1;
    }
    if (last_time > _latestValidWriteTime) {
      last_time = _latestValidWriteTime;
    }
    if (last_valid_time > _latestValidWriteTime) {
      last_valid_time = _latestValidWriteTime;
    }
  }
 
  return 0;
  
}
  
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

int Spdb::compileTimeList(const string &dir,
			  time_t start_time,
			  time_t end_time,
			  size_t minimum_interval /* = 1*/ )

{
  
  _clearTimeList();
  _clearErrStr();
  _errStr += "Running Spdb::compileTimeList\n";

  _dir = dir;
  _setLock(ReadMode);

  int iret = _compileTimeList(start_time, end_time, minimum_interval);

  _clearLock();
  return iret;

}

////////////////////////////////////////////////////////////
// hash4CharsToInt32()
//
// Convert the first 4 characters of an ID STRING to a int 32.
// If string is empty, returns 0.
// This function guarnteed to never return 0, unless the string is
// empty.

si32 Spdb::hash4CharsToInt32(const char *id_string)

{

  if(id_string == NULL) return -1;

  int len = strlen(id_string);

  if(len == 0) return 0;
  if(len > 4) len = 4;  /* only use the first four characters */

  si32 value = 0;
  for(int i= 0 ; i < len; i++) {
    value |= ((int) id_string[i]) << (i * 8);
  }
  
  if(value == 0) value = 1;
  
  return value;
}

////////////////////////////////////////////////////////////
// dehashInt32To4Chars()
//
// Convert a 32-bit int into a 4 Character ID String
// Returns a string containing the 4-char ASCII ID.

string Spdb::dehashInt32To4Chars(si32 id_int)

{

  char id_string[8]; 
  memset(id_string, 0, 8);

  if (id_int != 0) {

    id_string[3] = (char) ((id_int & 0xff000000) >> 24);
    id_string[2] = (char) ((id_int & 0x00ff0000) >> 16);
    id_string[1] = (char) ((id_int & 0x0000ff00) >> 8);
    id_string[0] = (char) (id_int & 0x000000ff);

  }

  return id_string;

}


////////////////////////////////////////////////////////////
// hashStringToInt32()
//
// Convert the 1-5 character ID STRING to an int 32.
// Does not clash with Spdb::hash4CharsToInt32 or
// any positive integer ID's
// returns -1 or error.

si32 Spdb::hash5CharsToInt32(const char *id_string)

{
  char packed[5];
  memset(packed, 0, 5);

  if(id_string == NULL) return -1;

  int len = strlen(id_string);
  if(len == 0) return -1;

  if(len > 5) len = 5;  /* only use the first five characters */

  // Pack Null,'-','0-9','A-Z,'a-z' into 6 bits.
  for(int i= 0 ; i < len; i++) {
    if(id_string[i] >= 97) {   // a-z
	  packed[i] = id_string[i] - 59;
	} else if (id_string[i] >= 65) { // A-Z
	  packed[i] = id_string[i] - 53;
	} else if (id_string[i] >= 48) { // 0-9
	  packed[i] = id_string[i] - 46;
	} else if (id_string[i] == 45) { // - char
	  packed[i] = id_string[i] - 44;
	} else {  // All others get mapped to the null character.
	  packed[i] = 0;
	}
  }
  si32 value = 0;
  value |= packed[0];
  value |= packed[1] << 6;
  value |= packed[2] << 12;
  value |= packed[3] << 18;
  value |= packed[4] << 24;
  
  return -value;   // Use the negative to indicate this is the 5 char hash.
}

////////////////////////////////////////////////////////////
// dehashInt32To5Chars()
//
// Convert a 32-bit int into a 1-5 Character ID String
// Returns a string containing the 1-5-char ASCII ID, NULL terminated.

string Spdb::dehashInt32To5Chars(si32 id_int)

{
  char id_string[8]; 
  memset(id_string, 0, 8);

  char packed[5];
  memset(packed, 0, 5);

  si32 value = -id_int;  // Unset the upper bit

  // Grab 6 bits per character
  if (id_int != 0) {
    packed[4] = (char) ((value & 0x3f000000) >> 24);
    packed[3] = (char) ((value & 0x00FC0000) >> 18);
    packed[2] = (char) ((value & 0x0003F000) >> 12);
    packed[1] = (char) ((value & 0x00000FC0) >> 6);
    packed[0] = (char)  (value & 0x0000003f);
  }

  // Expand 6 bits to original ASCII;  Null,'-','0-9','A-Z,'a-z' 
  for(int i= 0 ; i < 5; i++) {
    if(packed[i] >= 38) {   // a-z
	  id_string[i] = packed[i] + 59;
	} else if (packed[i] >= 12) { // A-Z
	  id_string[i] = packed[i] + 53;
	} else if (packed[i] >= 2) { // 0-9
	  id_string[i] = packed[i] + 46;
	} else if (packed[i] == 1) { // - char
	  id_string[i] = packed[i] + 44;
	} else {  // All others get mapped to the null character.
	  id_string[i] = 0;
	}
  }
  return id_string;
}

//////////////////////////////////////////////////////////
// put
//
// Returns 0 on success, -1 on error

int Spdb::_put(int prod_id,
	       const string &prod_label)
  
{

  if (_nPutChunks < 1) {
    return 0;
  }

  // open files for first chunk
  
  chunk_ref_t *putRefs = (chunk_ref_t *) _putRefBuf.getPtr();
  time_t validTime = putRefs[0].valid_time;
  time_t latestValidTime = validTime;
  int maxDataType = putRefs[0].data_type;
  int maxDataType2 = putRefs[0].data_type2;
  
  void *chunkData = _putDataBuf.getPtr();
  chunk_ref_t *ref = (chunk_ref_t *) _putRefBuf.getPtr();
  aux_ref_t *aux = (aux_ref_t *) _putAuxBuf.getPtr();
  
  for (int i = 0; i < _nPutChunks; i++, ref++, aux++) {
    
    // check files are open for correct day
    
    validTime = ref->valid_time;
    latestValidTime = MAX(validTime, latestValidTime);
    maxDataType = MAX(maxDataType, ref->data_type);
    maxDataType2 = MAX(maxDataType2, ref->data_type2);

    if (_checkOpen(prod_id, prod_label, validTime, WriteMode)) {
      _errStr += "ERROR - Spdb::put\n";
      _addStrErr("  Cannot open files for chunk in dir: ", _dir);
      _addStrErr("  Valid Time: ", utimstr(validTime));
      return -1;
    }

    void *chunk = (char *) chunkData + ref->offset;

    if (_storeChunk(ref, aux, chunk)) {
      _errStr += "ERROR - Spdb::put\n";
      _addStrErr("  Cannot store chunk in dir: ", _dir);
      _addStrErr("  Valid Time: ", utimstr(ref->valid_time));
      _closeFiles(false);
      return -1;
    }

  } // i

  _latestValidTimePut = latestValidTime;
  _closeFiles();
  
  // write latest data info

  DsLdataInfo ldata;
  ldata.setDir(_path);
  ldata.setDataFileExt(_indxExt);
  ldata.setWriter(_appName.c_str());

  char tmpStr[128];
  DateTime vtime(latestValidTime);
  sprintf(tmpStr, "%.4d%.2d%.2d.%s",
	  vtime.getYear(), vtime.getMonth(), vtime.getDay(), _indxExt);
  ldata.setRelDataPath(tmpStr);

  time_t storeTime = latestValidTime;
  if (_leadTimeStorage == LEAD_TIME_IN_DATA_TYPE) {
    ldata.setIsFcast();
    ldata.setLeadTime(maxDataType);
    storeTime -= maxDataType;
  } else if (_leadTimeStorage == LEAD_TIME_IN_DATA_TYPE2) {
    ldata.setIsFcast();
    ldata.setLeadTime(maxDataType2);
    storeTime -= maxDataType2;
  }

  if (ldata.write(storeTime, "spdb")) {
    _errStr += "ERROR - Spdb::put\n";
    _errStr += "  Cannot write latest data info file.\n";
    _addStrErr("  Dir path: ", _path);
    return -1;
  }
 
  return 0;

}

///////////////////////////////////////////////////////////////////
// Erase data for a given set of chunk refs.
// Before calling this function, call clearPutChunks(),
// and addPutChunk() for each time and data_type you want to erase.
// You can call addPutChunk() with 0 length and NULL pointer.
// If the data_type is 0, all data at that time is erased.
// This is a special type of 'put'.
//
// Returns 0 on success, -1 on failure

int Spdb::_erase()

{
  
  int iret = 0;
  chunk_ref_t *ref = (chunk_ref_t *) _putRefBuf.getPtr();
  for (int i = 0; i < _nPutChunks; i++, ref++) {
    
    if (_checkOpen(0, "", ref->valid_time, WriteMode)) {
      _errStr += "  Problems with database from which to erase.\n";
      _addStrErr("  Dir: ", _dir);
      _addStrErr("  Valid time: ", utimstr(ref->valid_time));
      return -1;
    }
    
    if (_eraseChunks(ref->valid_time, ref->data_type, ref->data_type2)) {
      _closeFiles();
      iret = -1;
    }

  }
  
  _closeFiles();
  return iret;
  
}

////////////////////////////////////////////////////////////
// get the first and last times in the data base
//
// Returns 0 on success, -1 on failure

int Spdb::_getFirstAndLastTimes(time_t &first_time,
				time_t &last_time)
  
{

  first_time = 0;
  last_time = 0;
  
  // compute the path from using RapDataDir

  RapDataDir.fillPath(_dir, _path);
  
  // open data directory file for reading
  
  DIR *dirp;
  if ((dirp = opendir (_path.c_str())) == NULL) {
    _errStr += "ERROR - Spdb::_getFirstAndLastTimes\n";
    _addStrErr("  dir: ", _dir);
    _addStrErr("  Cannot open directory: ", _path);
    return -1;
  }

  // read through the directory,
  // find first and last valid indx names
  
  bool filesFound = false;
  string firstName, lastName;
  struct dirent	*dp = NULL;
  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {

    // exclude dir entries and files beginning with '.'
    if (dp->d_name[0] == '.') {
      continue;
    }

    // check for the indx extension
    if (strstr(dp->d_name, _indxExt) == NULL) {
      continue;
    }
    
    // check that the file name is in the correct format

    int year, month, day;
    if (sscanf(dp->d_name, "%4d%2d%2d",
	       &year, &month, &day) != 3) {
      continue;
    }

    // check that the index and data files exist

    char indxPath[MAX_PATH_LEN];
    sprintf(indxPath, "%s%s%.4d%.2d%.2d.%s",
	    _path.c_str(), PATH_DELIM, year, month, day, _indxExt);
    char dataPath[MAX_PATH_LEN];
    sprintf(dataPath, "%s%s%.4d%.2d%.2d.%s",
	    _path.c_str(), PATH_DELIM, year, month, day, _dataExt);

    if (!ta_stat_exists_compress(indxPath)) {
      continue;
    }
    if (!ta_stat_exists_compress(dataPath)) {
      continue;
    }

    // file name is in correct format, and exists. Therefore, accept it

    string thisName = dp->d_name;
    if (!filesFound) {
      firstName = thisName;
      lastName = thisName;
      filesFound = true;
    } else {
      if (thisName < firstName) {
	firstName = thisName;
      }
      if (thisName > lastName) {
	lastName = thisName;
      }
    }

  } // readdir()///

  // close the directory file

  closedir(dirp);

  if (filesFound) {

    // get first time from first file
    
    date_time_t firstDate;
    sscanf(firstName.c_str(), "%4d%2d%2d",
	   &firstDate.year, &firstDate.month, &firstDate.day);
    firstDate.hour = 12;
    firstDate.min = 0;
    firstDate.sec = 0;
    uconvert_to_utime(&firstDate);

    if (_openFiles(0, "", firstDate.unix_time, ReadMode) == 0) {
      first_time = _hdr.start_valid;
      _closeFiles();
    } else {
      return -1;
    }
    
    // get last time from last file
    
    date_time_t lastDate;
    sscanf(lastName.c_str(), "%4d%2d%2d",
	   &lastDate.year, &lastDate.month, &lastDate.day);
    lastDate.hour = 12;
    lastDate.min = 0;
    lastDate.sec = 0;
    uconvert_to_utime(&lastDate);
    time_t searchTime = lastDate.unix_time;
    
    // find first file, back from this time, which has chunks
    
    for (int ii = 0; ii < 365; ii++, searchTime -= SECS_IN_DAY) {
      if (_checkOpen(0, "", searchTime, ReadMode) == 0) {
        if (_hdr.n_chunks > 0) {
          last_time = _hdr.end_valid;
          break;
        }
      } else {
        break;
      }
    } // ii
    
  } // if (filesFound)

  return 0;

}
  
////////////////////////////////////////////////////////////
// get the last valid time in the data base,
// limiting the search based on the data types
//
// Returns 0 on success, -1 on failure

int Spdb::_getLastValid(time_t &last_valid_time,
                        int data_type,
                        int data_type2)
  
{
  
  // compute the path from using RapDataDir
  
  RapDataDir.fillPath(_dir, _path);
  
  // Get the latest data time from the latest_data_info file.
  
  time_t lastValidTime = 0;
  LdataInfo ldata(_path);
  
  if (ldata.read() == 0) {
    lastValidTime = ldata.getLatestValidTime();
  }

  if (lastValidTime <= 1) {
    time_t first, last;
    if (_getFirstAndLastTimes(first, last)) {
      _errStr += "ERROR - Spdb::_getLastValid\n";
      return -1;
    }
    lastValidTime = last;
  }

  if (data_type == 0 && data_type2 == 0) {
    // no data type constraints, so return now
    last_valid_time = lastValidTime;
    return 0;
  }

  // save the _getRefsOnly flag to restore later
  // then set to true so we only get the refs

  bool requestedGetRefsOnly = _getRefsOnly;
  _getRefsOnly = true;

  // get all refs from latest time back by 1 day,
  // using the data types for searching
  
  if (_getInterval(lastValidTime - SECS_IN_DAY,
                   lastValidTime,
                   data_type, data_type2)) {
    _errStr += "ERROR - Spdb::_getLastValid\n";
    TaStr::AddInt(_errStr, "  data_type: ", data_type);
    TaStr::AddInt(_errStr, "  data_type2: ", data_type2);
    _errStr += "  Cannot get refs for previous day\n";
    _getRefsOnly = requestedGetRefsOnly;
    return -1;
  }

  // restore the _getRefsOnly flag to the requested value
  
  _getRefsOnly = requestedGetRefsOnly;

  // if entries found, get the time of the last entry

  if (_nGetChunks >= 1) {

    chunk_ref_t *refs = (chunk_ref_t *) _getRefBuf.getPtr();
    chunk_ref_t &latestRef = refs[_nGetChunks - 1];
    lastValidTime = latestRef.valid_time;

  } else {

    // no data found in the past day, return error
    _clearGet();
    last_valid_time = 0;
    return 0;

  }

  // clear the get data

  _clearGet();
  
  // success

  last_valid_time = lastValidTime;
  return 0;

}
  
///////////////////////////////////////////////////////////////////
// _fetchExact()
//
// Get data at exactly the given time. Files are already open
//

int Spdb::_fetchExact(time_t request_time,
		      int data_type,
		      int data_type2)

{

  // get the indx posn for this time
  
  int posn;
  if ((posn = _posnAtTime(request_time, data_type, data_type2)) < 0) {
    return 0;
  }
  
  chunk_ref_t *fileRefs = (chunk_ref_t *) _hdrRefBuf.getPtr();
  aux_ref_t *fileAuxs = (aux_ref_t *) _hdrAuxBuf.getPtr();

  MemBuf readBuf;

  for (int i = posn; i < _hdr.n_chunks; i++) {
    
    if ((time_t) fileRefs[i].valid_time == request_time) {
      
      if (_checkTypeThenReadChunk(data_type, data_type2,
                                  fileRefs[i], fileAuxs[i],
                                  readBuf)) {
        _closeFiles();
        return -1;
      }

    } else {
      
      break;
      
    }

  } // i

  return 0;

}

///////////////////////////////////////////////////////////////////
// _getExact()
//
// Get data at exactly the given time.
//

int Spdb::_getExact(time_t request_time,
		    int data_type,
		    int data_type2)

{

  // open files
  
  if (_openFiles(0, "", request_time, ReadMode)) {
    if (_emptyDay) {
      return 0;
    } else {
      return -1;
    }
  }

  int iret = _fetchExact(request_time, data_type, data_type2);
  _closeFiles();

  if (iret) {
    return -1;
  }

  if (_getUnique == UniqueLatest) {
    makeUniqueLatest();
  } else if (_getUnique == UniqueEarliest) {
    makeUniqueEarliest();
  }
  
  return 0;

}
  
///////////////////////////////////////////////////////////////////
// _getClosest()
//
// Get data closest to the given time, within the time margin.

int Spdb::_getClosest(time_t request_time,
		      int time_margin,
		      int data_type,
		      int data_type2)

{

  // open files
  
  if (_openFiles(0, "", request_time, ReadMode)) {
    if (!_emptyDay) {
      return -1;
    }
  }

  int iret;

  // get the chunk position for the request time - if
  // negative there is no data exactly at the request time,
  // so get the nearest time.
  
  if (_posnAtTime(request_time, data_type, data_type2) >= 0) {

    // there is data exactly at the requested time
    iret = _fetchExact(request_time, data_type, data_type2);

  } else {

    // search for closest time
    
    time_t nearest_time;
    if (_getTimeNearest(request_time, time_margin, data_type, data_type2,
			nearest_time) != 0) {
      _nGetChunks = 0;
      iret = 0;
    } else {
      iret = _getExact(nearest_time, data_type, data_type2);
    }

  }

  _closeFiles();

  if (iret) {
    return -1;
  }

  if (_getUnique == UniqueLatest) {
    makeUniqueLatest();
  } else if (_getUnique == UniqueEarliest) {
    makeUniqueEarliest();
  }

  return 0;
  
}

///////////////////////////////////////////////////////////////////
// _getFirstBefore()
//
// Get first data at or before the requested time.

int Spdb::_getFirstBefore(time_t request_time,
			  int time_margin,
			  int data_type,
			  int data_type2)

{

  // open files
  
  if (_openFiles(0, "", request_time, ReadMode)) {
    if (!_emptyDay) {
      return -1;
    }
  }

  int iret;

  // get the chunk position for the request time - if
  // negative there is no data exactly at the request time,
  // so get the nearest time.
  
  if (_posnAtTime(request_time, data_type, data_type2) >= 0) {

    // there is data exactly at the requested time
    iret = _fetchExact(request_time, data_type, data_type2);

  } else {

    // search for closest time
    
    time_t nearest_time;
    if (_getFirstTimeBefore(request_time, request_time - time_margin,
			    data_type, data_type2, nearest_time) != 0) {
      _nGetChunks = 0;
      iret = 0;
    } else {
      iret = _getExact(nearest_time, data_type, data_type2);
    }

  }

  _closeFiles();

  if (iret) {
    return -1;
  }

  if (_getUnique == UniqueLatest) {
    makeUniqueLatest();
  } else if (_getUnique == UniqueEarliest) {
    makeUniqueEarliest();
  }
  
  return 0;
  
}
  
///////////////////////////////////////////////////////////////////
// _getFirstAfter()
//
// Get first data at or after the requested time.

int Spdb::_getFirstAfter(time_t request_time,
			 int time_margin,
			 int data_type,
			 int data_type2)

{

  // open files
  
  if (_openFiles(0, "", request_time, ReadMode)) {
    if (!_emptyDay) {
      return -1;
    }
  }

  int iret;

  // get the chunk position for the request time - if
  // negative there is no data exactly at the request time,
  // so get the nearest time.
  
  if (_posnAtTime(request_time, data_type, data_type2) >= 0) {

    // there is data exactly at the requested time
    iret = _fetchExact(request_time, data_type, data_type2);

  } else {

    // search for closest time
    
    time_t nearest_time;
    if (_getFirstTimeAfter(request_time, request_time + time_margin,
			   data_type, data_type2, nearest_time) != 0) {
      _nGetChunks = 0;
      iret = 0;
    } else {
      iret = _getExact(nearest_time, data_type, data_type2);
    }

  }

  _closeFiles();

  if (iret) {
    return -1;
  }

  if (_getUnique == UniqueLatest) {
    makeUniqueLatest();
  } else if (_getUnique == UniqueEarliest) {
    makeUniqueEarliest();
  }

  return 0;
  
}
  
///////////////////////////////////////////////////////////////////
// _getInterval()
//
// Get data in the time interval.

int Spdb::_getInterval(time_t start_time,
		       time_t end_time,
		       int data_type,
		       int data_type2)

{


  // constrain request using available data times

  time_t time1 = start_time;
  time_t time2 = end_time;
  time_t first_time, last_time;
  if (_getFirstAndLastTimes(first_time, last_time) == 0) {
    time1 = MAX(start_time, first_time);
    time2 = MIN(end_time, last_time);
  } else {
    _errStr += "ERROR - _getFirstAndLastTimes failed\n";
    return -1;
  }
  time_t file_start_time = (start_time / SECS_IN_DAY) * SECS_IN_DAY;

  
  MemBuf readBuf;

  while (file_start_time <= time2) {
    
    // open files
    
    if (_openFiles(0, "", file_start_time, ReadMode)) {
      if (_emptyDay) {
	file_start_time += SECS_IN_DAY;
	continue;
      } else {
	return -1;
      }
    }
    chunk_ref_t *fileRefs = (chunk_ref_t *) _hdrRefBuf.getPtr();
    aux_ref_t *fileAuxs = (aux_ref_t *) _hdrAuxBuf.getPtr();
    
    // get the first indx posn at or after start time
    
    int posn = _firstPosnAfter(time1);

    if (posn >= 0) {
      
      for (int i = posn; i < _hdr.n_chunks; i++) {
	
	if ((time_t) fileRefs[i].valid_time <= time2) {
	  
          if (_checkTypeThenReadChunk(data_type, data_type2,
                                      fileRefs[i], fileAuxs[i],
                                      readBuf)) {
            _closeFiles();
            return -1;
          }

	} else {
	  
	  break;
	  
	} // if (fileRefs[i].valid_time <= time2)
	
      } // i

    } // if (posn >= 0)
    _closeFiles();

    // move ahead by 1 day
    
    file_start_time = (time_t) _hdr.start_of_day + SECS_IN_DAY;
    time1 = file_start_time;

  } // while
  
  if (_getUnique == UniqueLatest) {
    makeUniqueLatest();
  } else if (_getUnique == UniqueEarliest) {
    makeUniqueEarliest();
  }
  
  return 0;

}
  
///////////////////////////////////////////////////////////////////
// _getValid()
//
// Get data valid at the given time.

int Spdb::_getValid(time_t request_time,
		    int data_type,
		    int data_type2)

{

  // read file header for search time, set start and end times
  // for search
  
  if (_openFiles(0, "", request_time, ReadMode)) {
    return 0;
  }
  time_t time1 = _hdr.earliest_valid;
  time_t time2 = request_time;

  // constrain request using available data times
  
  time_t first_time, last_time;
  if (_getFirstAndLastTimes(first_time, last_time)) {
    _nGetChunks = 0;
    _closeFiles();
    return 0;
  }
  _closeFiles();
  time1 = MAX(time1, first_time);
  time2 = MIN(time2, last_time);
  time_t file_start_time = (time1 / SECS_IN_DAY) * SECS_IN_DAY;
  
  MemBuf readBuf;

  while (file_start_time <= time2) {
    
    // open files
    
    if (_openFiles(0, "", file_start_time, ReadMode)) {
      if (_emptyDay) {
	file_start_time += SECS_IN_DAY;
	continue;
      } else {
	return -1;
      }
    }

    chunk_ref_t *fileRefs = (chunk_ref_t *) _hdrRefBuf.getPtr();
    aux_ref_t *fileAuxs = (aux_ref_t *) _hdrAuxBuf.getPtr();

    // get the first indx posn at or after start time
    
    int posn = _firstPosnAfter(time1);

    if (posn >= 0) {
      
      for (int i = posn; i < _hdr.n_chunks; i++) {
	
	if ((time_t) fileRefs[i].valid_time <= request_time &&
	    (time_t) fileRefs[i].expire_time >= request_time) {

          if (_checkTypeThenReadChunk(data_type, data_type2,
                                      fileRefs[i], fileAuxs[i],
                                      readBuf)) {
            _closeFiles();
            return -1;
          }

	} // if (fileRefs[i].valid_time <= request_time ...
	
      } // i

    } // if (posn >= 0)
    
    _closeFiles();

    // move ahead by 1 day
    
    file_start_time = (time_t) _hdr.start_of_day + SECS_IN_DAY;
    time1 = file_start_time;

  } // while
  
  if (_getUnique == UniqueLatest) {
    makeUniqueLatest();
  } else if (_getUnique == UniqueEarliest) {
    makeUniqueEarliest();
  }
  
  return 0;

}
  
///////////////////////////////////////////////////////////////////
// Get latest data, using the data types for the search

int Spdb::_getLatest(int time_margin,
		     int data_type,
		     int data_type2)
  
{

  // get the latest time
  
  time_t last_valid_time;
  if (_getLastValid(last_valid_time, data_type, data_type2)) {
    _errStr += "ERROR - Spdb::getLatest\n";
    return -1;
  }

  if (last_valid_time == 0) {
    _nGetChunks = 0;
    return 0;
  }
  
  int iret = _getInterval(last_valid_time - time_margin,
			  last_valid_time + time_margin,
			  data_type, data_type2);

  if (iret) {
    return -1;
  }

  if (_getUnique == UniqueLatest) {
    makeUniqueLatest();
  } else if (_getUnique == UniqueEarliest) {
    makeUniqueEarliest();
  }
  
  return 0;

}

////////////////////////////////////////////////////////////
// get the first, last and last_valid_time in the data base
// In this case, no chunk data is returned.
//
// Returns 0 on success, -1 on failure

int Spdb::_getTimes()
  
{

  if (_getFirstAndLastTimes(_firstTime, _lastTime)) {
    return -1;
  }

  if (_getLastValid(_lastValidTime, 0, 0)) {
    return -1;
  }

  return 0;
  
}
  
//////////////////////////////////////////////////////////
// compile time list

int Spdb::_compileTimeList(time_t start_time,
			   time_t end_time,
			   size_t minimum_interval)
  
{
  
  // loop through the days
  
  int iret = -1;

  int startDay = start_time / SECS_IN_DAY;
  int endDay = end_time / SECS_IN_DAY;

  for (int iday = startDay; iday <= endDay; iday++) {

    // compute midday time

    time_t midday = iday * SECS_IN_DAY + SECS_IN_DAY / 2;

    // open files
    
    if (_openFiles(0, "", midday, ReadMode) == 0) {
      
      iret = 0;

      time_t lastAdded = -1;
      chunk_ref_t *ref = (chunk_ref_t *) _hdrRefBuf.getPtr();
      aux_ref_t *aux = (aux_ref_t *) _hdrAuxBuf.getPtr();
      for (int i = 0; i < _hdr.n_chunks; i++, ref++, aux++) {
        if ((time_t) ref->valid_time >= start_time &&
	    (time_t) ref->valid_time <= end_time) {
          if (_checkWriteTimeOnGet &&
              (aux->write_time > _latestValidWriteTime)) {
            continue;
          }
	  if (i == 0) {
	    _timeList.push_back(ref->valid_time);
	    lastAdded = ref->valid_time;
	  } else {
	    if (ref->valid_time - lastAdded >= (int) minimum_interval) {
	      _timeList.push_back(ref->valid_time);
	      lastAdded = ref->valid_time;
	    }
	  } // if (i == 0)
	} // if (ref->valid_time ...
      } // i
      
    } // if (_openFiles ...
    
  } // iday

  return iret;

}

///////////////////////////////////////
// _clearGet()
//
// Clear the get buffers
  
void Spdb::_clearGet()

{

  _nGetChunks = 0;
  _getRefBuf.free();
  _getAuxBuf.free();
  _getDataBuf.free();
  _chunksFromGet.clear();
  _storedCompression.clear();

}

///////////////////////////////////////
// _clearTimeList()
//
// Clear the time list vector
  
void Spdb::_clearTimeList()

{

  _timeList.clear();

}

///////////////////////////////////////
// Load up the chunk vector

void Spdb::_loadChunksFromGet()

{

  _chunksFromGet.clear();

  chunk_ref_t *ref = (chunk_ref_t *) _getRefBuf.getPtr();
  aux_ref_t *aux = (aux_ref_t *) _getAuxBuf.getPtr();
  int nchunksAux = _getAuxBuf.getLen() / sizeof(aux_ref_t);
  bool haveAux = true;
  if (nchunksAux != _nGetChunks) {
    haveAux = false;
  }
  for (int i = 0; i < _nGetChunks; i++, ref++, aux++) {
    chunk_t chunk;
    chunk.valid_time = ref->valid_time;
    chunk.expire_time = ref->expire_time;
    if (haveAux) {
      chunk.write_time = aux->write_time;
      chunk.current_compression = (compression_t) aux->compression;
      if (i < (int) _storedCompression.size()) {
        chunk.stored_compression = _storedCompression[i];
      } else {
        chunk.stored_compression = (compression_t) aux->compression;
      }
      chunk.tag = aux->tag;
    } else {
      chunk.write_time = 0;
      chunk.stored_compression = COMPRESSION_NONE;
    }
    chunk.data_type = ref->data_type;
    chunk.data_type2 = ref->data_type2;
    chunk.len = ref->len;
    chunk.data = (void *) ((char *) getChunkData() + ref->offset);
    _chunksFromGet.push_back(chunk);
  }

}

////////////////////////////////////////
// uncompress chunks in the get buffers
//
// This should be called before using chunks, if
// setChunkUnCompressOnGet(false) was called before
// getting the data.

void Spdb::uncompressGetChunks()

{

  // first check if we need to uncompress
  
  {
    aux_ref_t *aux  = (aux_ref_t *) _getAuxBuf.getPtr();
    bool needUncompress = false;
    for (int i = 0; i < _hdr.n_chunks; i++, aux++) {
      if (aux->compression != 0) {
        needUncompress = true;
        break;
      }
    }
    if (!needUncompress) {
      // no chunks are compressed
      return;
    }
  }

  // we have some compressed chunks, so proceed with uncompression

  int nChunks = 0;
  MemBuf refBuf;
  MemBuf auxBuf;
  MemBuf dataBuf;
  vector<compression_t> storedCompression;

  chunk_ref_t *ref  = (chunk_ref_t *) _getRefBuf.getPtr();
  aux_ref_t *aux  = (aux_ref_t *) _getAuxBuf.getPtr();
  char *data = (char *) _getDataBuf.getPtr();

  for (int i = 0; i < _nGetChunks; i++, ref++, aux++) {
    
    // make local copies of the references
    
    chunk_ref_t refCopy(*ref);
    aux_ref_t auxCopy(*aux);
    void *chunk = data + refCopy.offset;
    compression_t compression = (compression_t) auxCopy.compression;
    
    // uncompress chunk if it is compressed
    // otherwise add it unchanged to the buffers
    
    if (ta_is_compressed(chunk, refCopy.len)) {
      ui64 nbytesUncompressed;
      void *uncompressed  = ta_decompress(chunk, &nbytesUncompressed);
      if (uncompressed != NULL) {
        memcpy(chunk, uncompressed, nbytesUncompressed);
        refCopy.len = nbytesUncompressed;
        refCopy.offset = dataBuf.getLen();
        auxCopy.compression = 0;
        ta_compress_free(uncompressed);
      } else {
        cerr << "WARNING - Spdb::uncompressGetChunks" << endl;
        cerr << "  Cannot uncompress chunk, offset, len: "
             << refCopy.offset << ", " << refCopy.len << endl;
      }
    }

    nChunks++;
    refBuf.add(&refCopy, sizeof(chunk_ref_t));
    auxBuf.add(&auxCopy, sizeof(aux_ref_t));
    dataBuf.add(chunk, refCopy.len);
    storedCompression.push_back(compression);

  } // i

  // copy to main variables

  _nGetChunks = nChunks;
  _getRefBuf = refBuf;
  _getAuxBuf = auxBuf;
  _getDataBuf = dataBuf;
  _storedCompression = storedCompression;

  // load up chunks

  _loadChunksFromGet();

}

/////////////////////////////////////////////////////
// _openFiles()
//
// If the files exist, read in header.
//
// Returns 0 on success, -1 on failure.
//
// If mode is Read, and there are no files to be read, returns -1
// and sets the flag _emptyDay

int Spdb::_openFiles(int prod_id,
		     const string &prod_label,
		     time_t valid_time,
		     open_mode_t mode,
		     bool read_chunk_refs /* = true*/ )
  
{

  _emptyDay = false;

  // compute the path from using RapDataDir

  RapDataDir.fillPath(_dir, _path);

  // close files if open
  
  _closeFiles();

  // make directory if needed
  
  if (mode == WriteMode) {
    if (ta_makedir_recurse(_path.c_str())) {
      _errStr += "ERROR - Spdb::_openFiles\n";
      _addStrErr("  Cannot make dir: ", _path);
      return -1;
    }
  }

  // compute the path names
  
  date_time_t vtime;
  vtime.unix_time = valid_time;
  uconvert_from_utime(&vtime);
  
  sprintf(_indxPath, "%s%s%.4d%.2d%.2d.%s",
	  _path.c_str(), PATH_DELIM,
	  vtime.year, vtime.month, vtime.day,
	  _indxExt);

  sprintf(_dataPath, "%s%s%.4d%.2d%.2d.%s",
	  _path.c_str(), PATH_DELIM,
	  vtime.year, vtime.month, vtime.day,
	  _dataExt);

  // decide if files exist
  
  bool indx_file_exists = false;
  bool data_file_exists = false;
  struct stat stat_buf;

  if (ta_stat_uncompress(_indxPath, &stat_buf) == 0) {
    // file exists - does its size exceed the hdr size?
    if ((int) stat_buf.st_size >= (int) sizeof(header_t)) {
      indx_file_exists = true;
    } else {
      indx_file_exists = false;
    }
  } else {
    indx_file_exists = false;
  }

  if (ta_stat_uncompress(_dataPath, &stat_buf) == 0) {
    data_file_exists = true;
  } else {
    data_file_exists = false;
  }

  if (indx_file_exists && data_file_exists) {
    
    if (_openReadWrite(prod_id, mode, read_chunk_refs)) {
      return -1;
    }

  } else {

    // no files

    if (mode == ReadMode) {

      // set up an empty object, set _emptyDay flag, return error

      _emptyDay = true;

      if (_prodId == 0 && prod_id != 0) {
	_prodId = prod_id;
	_prodLabel = prod_label;
      }
      _initHdr(_prodId, _prodLabel, valid_time);

      return -1;

    }

    // create it in write mode

    if (_openCreate(prod_id, prod_label, valid_time, mode)) {
      return -1;
    }
    
  }

  // successful, so set valid day

  _openMode = mode;
  _openDay = valid_time / SECS_IN_DAY;

  return 0;
  
}

/////////////////////////////////////////////////////
// _openReadWrite()
//
// Open files in read/write mode.
//
// Returns 0 on success, -1 on failure.

int Spdb::_openReadWrite(int prod_id,
			 open_mode_t mode,
			 bool read_chunk_refs)
     
{

  // open files as appropriate
  
  const char *open_mode;
  if (mode == ReadMode) {
    open_mode = "rb";  // read only
  } else {
    open_mode = "rb+"; // read/write
  }
    
  // open files for read/write
  
  if ((_indxFile =
       ta_fopen_uncompress(_indxPath, open_mode)) == NULL) {

    int errNum = errno;
    cerr << "ERROR - Spdb::_openReadWrite" << endl;
    cerr << "  Failure to open file read/write" << endl;
    cerr << "  File: " << _indxPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    cerr << "  Failing over to read-only mode" << endl;
    
    // failure - try read-only
    
    if ((_indxFile =
	 ta_fopen_uncompress(_indxPath, "rb")) == NULL) {
      errNum = errno;
      _errStr += "ERROR - Spdb::_openReadWrite\n";
      _addStrErr("  Product: ", _hdr.prod_label);
      _errStr += "  Cannot open data indx for read.\n";
      _addStrErr("  _indxPath: ", strerror(errNum));
      return -1;
    }
    
  }
  _indxFd = fileno(_indxFile);
  
  if ((_dataFile =
       ta_fopen_uncompress(_dataPath, open_mode)) == NULL) {
    
    // failure - try read-only
    
    int errNum = errno;
    cerr << "ERROR - Spdb::_openReadWrite" << endl;
    cerr << "  Failure to open file read/write" << endl;
    cerr << "  File: " << _dataPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    cerr << "  Failing over to read-only mode" << endl;
    
    if ((_dataFile =
	 ta_fopen_uncompress(_dataPath, "r")) == NULL) {
      int errNum = errno;
      _errStr += "ERROR - Spdb::_openReadWrite\n";
      _addStrErr("  Product: ", _hdr.prod_label);
      _errStr += "  Cannot open data file for read.\n";
      _addStrErr("  _dataPath: ", strerror(errNum));
      fclose(_indxFile);
      return -1;
    }
    
  }
  _dataFd = fileno(_dataFile);
  _filesOpen = true;
  
  // read in index header
  
  if (ta_fread(&_hdr, sizeof(header_t), 1, _indxFile) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Spdb::_openReadWrite\n";
    _addStrErr("  Product: ", _hdr.prod_label);
    _errStr += "  Cannot read indx file header.\n";
    _addStrErr("  _indxPath: ", strerror(errNum));
    _closeFiles(false);
    return -1;
  }
  
  // swap header as required, except for label which is char
  
  BE_to_array_32(((char *) &_hdr + SPDB_LABEL_MAX),
		 sizeof(header_t) - SPDB_LABEL_MAX);
  
  // check that the ID is correct - if ID of 0 is passed in
  // we accept any data ID
  
  if (prod_id > 0) {
    if (_hdr.prod_id == 0) {
      _hdr.prod_id = prod_id;
    } else if (_hdr.prod_id != prod_id) {
      _errStr += "ERROR - Spdb::_openReadWrite\n";
      _addStrErr("  Product: ", _hdr.prod_label);
      _errStr += "  Incorrect indx file ID.\n";
      _addIntErr("  ID found: ", _hdr.prod_id);
      _addIntErr("  Should be: ", prod_id);
      _closeFiles(false);
      return -1;
    }
  }
  if (_hdr.prod_id != 0) {
    _prodId = _hdr.prod_id;
    _prodLabel = _hdr.prod_label;
  }
  if (_hdr.lead_time_storage != 0) {
    _leadTimeStorage = (lead_time_storage_t) _hdr.lead_time_storage;
  }
  
  if (read_chunk_refs) {
    _readChunkRefs();
  }

  return 0;

}

/////////////////////////////////////////////////////
// _openCreate()
//
// Files do not exist, so open in write+ mode.
//
// Returns 0 on success, -1 on failure.

int Spdb::_openCreate(int prod_id,
		      const string &prod_label,
		      time_t valid_time,
		      open_mode_t mode)
		      

{

  // open files for write/read
    
  if ((_indxFile =
       ta_fopen_uncompress(_indxPath, "wb+")) == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Spdb::_openCreate\n";
    _addStrErr("  Product: ", _hdr.prod_label);
    _errStr += "  Cannot open data indx for write/read.\n";
    _addStrErr("  _indxPath: ", strerror(errNum));
    return -1;
  }
  _indxFd = fileno(_indxFile);
  
  if ((_dataFile =
       ta_fopen_uncompress(_dataPath, "wb+")) == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Spdb::_openCreate\n";
    _addStrErr("  Product: ", _hdr.prod_label);
    _errStr += "  Cannot open data data for write/read.\n";
    _addStrErr("  _dataPath: ", strerror(errNum));
    fclose(_indxFile);
    return -1;
  }
  _dataFd = fileno(_dataFile);
  _filesOpen = true;
  
  // initialize header
  
  _initHdr(prod_id, prod_label, valid_time);

  if (_writeIndxFile()) {
    _closeFiles(false);
    return -1;
  }

  _prodId = prod_id;
  _prodLabel = prod_label;

  return 0;
  
}

/////////////////////  
// initialize header
  
void Spdb::_initHdr(int prod_id,
		    const string &prod_label,
		    time_t valid_time)
  
{
  
  MEM_zero(_hdr);
  STRncopy(_hdr.prod_label, prod_label.c_str(),
	   SPDB_LABEL_MAX);
  _hdr.major_version = _fileMajorVersion;
  _hdr.minor_version = _fileMinorVersion;
  _hdr.prod_id = prod_id;
  _hdr.n_chunks = 0;
  _hdr.nbytes_frag = 0;
  _hdr.nbytes_data = 0;
  _hdr.max_duration = 0;
  _hdr.start_of_day = (valid_time / SECS_IN_DAY) * SECS_IN_DAY;
  _hdr.end_of_day = (time_t) _hdr.start_of_day + 86399;
  _hdr.start_valid = (time_t) _hdr.end_of_day;
  _hdr.end_valid = (time_t) _hdr.start_of_day;
  _hdr.latest_expire = (time_t) _hdr.start_of_day;
  _hdr.earliest_valid = (time_t) _hdr.end_of_day;
  _hdr.lead_time_storage = _leadTimeStorage;
  memset(_hdr.spares, 0, sizeof(_hdr.spares));
  
  for (int i = 0; i < MINS_IN_DAY; i++) {
    _hdr.minute_posn[i] = -1;
  }

}
  
/////////////////////////////////////////////////////
// _checkOpen()
//
// Check that the files are open for the relevant day.
// If incorrect day, files are closed and the correct files opened.
//
// If mode is Read, and there are no files to be read, returns -1
// and sets the flag _emptyDay
//
// Returns 0 on success, -1 on failure.

int Spdb::_checkOpen(int prod_id,
		     const string &prod_label,
		     time_t valid_time,
		     open_mode_t mode,
		     bool read_chunk_refs /* = true*/ )
  
{

  int day = valid_time / SECS_IN_DAY;
  
  if (day != _openDay) {
    _closeFiles();
  }

  if (!_filesOpen) {

    return _openFiles(prod_id, prod_label,
		      valid_time, mode, read_chunk_refs);
    
  } else {

    return 0;

  }

}

///////////////////
// close the files
//
// If sync is true and mode is write, we write the index file.
// Defragmentation is carried out if enabled.
//

void Spdb::_closeFiles(bool sync /* = true*/ )

{

  if (!_filesOpen) {
    return;
  }

  if (sync && _openMode == WriteMode) {
    
    // defrag if necessary

    if (_enableDefrag && _defrag()) {
      _errStr += "ERROR - Spdb::_closeFiles\n";
    }
    
    // write index file
    
    if (_indxFile != NULL) {
      if (_writeIndxFile()) {
	_errStr += "ERROR - Spdb::_closeFiles\n";
	_errStr += "  Cannot write indx file.\n";
	_addStrErr("  Product label: ", _hdr.prod_label);
      }
    }

  } // if (sync && _openMode == WriteMode)

  if (_indxFile != NULL) {
    fclose(_indxFile);
    _indxFile = NULL;
  }

  if (_dataFile != NULL) {
    fclose(_dataFile);
    _dataFile = NULL;
  }

  _filesOpen = false;
  _openDay = 0;

  return;


}

//////////////////////
// write the indx file
// Returns 0 on success, -1 on failure.

int Spdb::_writeIndxFile(bool write_refs /* = true*/ )
{

  // copy header and put into BE order
  
  header_t tmp_hdr = _hdr;
  BE_from_array_32(((char *) &tmp_hdr + SPDB_LABEL_MAX),
		   sizeof(header_t) - SPDB_LABEL_MAX);
  
  // write out index header
  
  fseek(_indxFile, 0, SEEK_SET);

  if (ta_fwrite(&tmp_hdr, sizeof(header_t), 1, _indxFile) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Spdb::_writeIndxFile\n";
    _addStrErr("  Product: ", _hdr.prod_label);
    _errStr += "  Cannot write indx file hdr.\n";
    _addStrErr("  _indxPath: ", strerror(errNum));
    return -1;
  }

  if (_hdr.n_chunks > 0 && write_refs) {
    
    // copy chunk refs and aux refs to work buffer

    MemBuf refBuf;
    refBuf.add(_hdrRefBuf.getPtr(), _hdr.n_chunks * sizeof(chunk_ref_t));
    chunk_refs_to_BE((chunk_ref_t *) refBuf.getPtr(), _hdr.n_chunks);

    MemBuf auxBuf;
    auxBuf.add(_hdrAuxBuf.getPtr(), _hdr.n_chunks * sizeof(aux_ref_t));
    aux_refs_to_BE((aux_ref_t *) auxBuf.getPtr(), _hdr.n_chunks);
    
    MemBuf workBuf;
    workBuf.add(refBuf.getPtr(), refBuf.getLen());
    workBuf.add(auxBuf.getPtr(), auxBuf.getLen());

    // write out refs
    
    if (ta_fwrite(workBuf.getPtr(), 1,
                  workBuf.getLen(), _indxFile) != workBuf.getLen()) {
      int errNum = errno;
      _errStr += "ERROR - Spdb::_writeIndxFile\n";
      _addStrErr("  Product: ", _hdr.prod_label);
      _errStr += "  Cannot write indx file chunk refs.\n";
      _addStrErr("  _indxPath: ", strerror(errNum));
      return -1;
    }
    fflush(_indxFile);
  }
  
  // Flush the index file stream to make sure the data is written.

  fflush(_indxFile);
  
  return 0;

}

////////////////////////////////////////////
// _setLock()
//
// Sets up locks on files.
//
// Returns 0 on success, -1 on failure.

int Spdb::_setLock(open_mode_t mode)

{

  // create the lock file

  string fullDir;
  RapDataDir.fillPath(_dir, fullDir);
  sprintf(_lockPath, "%s%s%s", fullDir.c_str(), PATH_DELIM, "_lock");
  if ((_lockFile = fopen(_lockPath, "w+")) == NULL) {
    _errStr += "ERROR - Spdb::_setLocks\n";
    _addStrErr("  Cannot create lock file, dir: ", fullDir);
    return -1;
  }
  
  const char *modeStr = "w";
  if (mode == ReadMode) {
    modeStr = "r";
  }
  
  if (ta_lock_file_threaded(_lockPath, _lockFile, modeStr)) {
    _errStr += "ERROR - Spdb::_setLocks\n";
    _errStr += "  File: ";
    _errStr += _lockPath;
    _errStr += "\n";
    _errStr += strerror(errno);
    _errStr += "\n";
    _errStr += "  File may be read-only.\n";
    _errStr += "  Also check for NFS mounts and file protections.\n";
    if (mode == WriteMode || !_ignoreLock()) {
      return -1;
    } else {
      cerr << _errStr << endl;
    }
  }
  
  _locked = true;
  
  return 0;

}

////////////////////////////////////////
// clear the file locks
//
// Returns 0 on success, -1 on failure.

int Spdb::_clearLock()

{

  int iret = 0;

  if (_locked) {
    _locked = false;
    if (ta_unlock_file_threaded(_lockPath, _lockFile)) {
      _errStr += "ERROR - Spdb::_clearLocks\n";
      _errStr += "  File: ";
      _errStr += _lockPath;
      _errStr += "\n";
      _errStr += strerror(errno);
      _errStr += "\n";
      iret = -1;
    }
    fclose(_lockFile);
  }

  return iret;

}

/////////////////////////////////////
// check whether we can ignore lock

bool Spdb::_ignoreLock()

{
  char *lock_str = getenv("SPDB_ALLOW_NO_LOCK");
  
  if (lock_str && STRequal(lock_str, "true")) {
    return true;
  }

  return false;
}

/////////////////////////////////////
// add error string with int argument

void Spdb::_addIntErr(const char *err_str, int iarg)
{
  _errStr += err_str;
  char str[32];
  sprintf(str, "%d\n", iarg);
  _errStr += str;
}

////////////////////////////////////////
// add error string with string argument

void Spdb::_addStrErr(const char *err_str, const string &sarg)
{
  _errStr += err_str;
  _errStr += ": ";
  _errStr += sarg;
  _errStr += "\n";
}


//////////////////////////////////////////////////////////////////
// set the auxiliary XML buffer
// This may be used to pass extra information from a client
// to a server
// The contents of the XML must be agreed upon between the client
// and server. This is not part of the SPDB protocol.

void Spdb::setAuxXml(const string &xml)
{
  _auxXml = xml;
}

void Spdb::clearAuxXml()
{
  _auxXml.clear();
}

//////////////////////////////////////////////
// print header
// Prints the SPDB headers for a given dir and time
//
// Returns 0 on success, -1 on failure

int Spdb::printHeader(const string &dir,
		      time_t request_time,
		      ostream &out)
  
{

  _clearErrStr();
  
  if (_openFiles(0, "", request_time, ReadMode)) {
    _errStr += "ERROR - Spdb::printHeader\n";
    _addStrErr("  dir: ", dir);
    _addStrErr("  Cannot open file for time: ", utimstr(request_time));
    return -1;
  }

  out << endl;
  out << "SPDB FILE HEADER" << endl << endl;
  out << "  dir : " << _path << endl;
  out << "  indx path: " << _indxPath << endl;
  out << "  data path: " << _dataPath << endl;
  out << endl;

  _printIndxHeader(_hdr, out, true);
  
  out << endl;
  out << "Chunk reference array:" << endl << endl;
  out << setw(8) << "Chunk"
      << setw(15) << "Data Type"
      << setw(15) << "Data Type2"
      << setw(15) << "Valid"
      << setw(15) << "Expire"
      << setw(15) << "Write"
      << setw(10) << "Compress"
      << setw(8) << "Len"
      << " " << "Tag"
      << endl;
  

  chunk_ref_t *refs = (chunk_ref_t *) _hdrRefBuf.getPtr();
  aux_ref_t *auxs = (aux_ref_t *) _hdrAuxBuf.getPtr();
  for (int i = 0; i < _hdr.n_chunks; i++, refs++, auxs++) {
    out << setw(8) << i
	<< setw(15) << refs->data_type
	<< setw(15) << refs->data_type2
	<< setw(15) << utimstr(refs->valid_time)
	<< setw(15) << utimstr(refs->expire_time)
	<< setw(15) << utimstr(auxs->write_time);
    compression_t compress = (compression_t) auxs->compression;
    if (compress == COMPRESSION_NONE) {
      out << setw(10) << "none";
    } else if (compress == COMPRESSION_GZIP) {
      out << setw(10) << "gzip";
    } else if (compress == COMPRESSION_BZIP2) {
      out << setw(10) << "bzip2";
    }
    out << setw(8) << refs->len
        << " " << auxs->tag
        << endl;
  }

  _closeFiles();

  return 0;

}

//////////////////////////////////////
// Print the index header

void Spdb::_printIndxHeader(header_t &hdr,
			    ostream &out,
			    bool print_min_posn /* = false*/ )
     
{

  out << "prod_label: " << hdr.prod_label << endl;
  out << "major_version: " << hdr.major_version << endl;
  out << "minor_version: " << hdr.minor_version << endl;
  out << "prod_id: " << hdr.prod_id << endl;
  out << "n_chunks: " << hdr.n_chunks << endl;
  out << "nbytes_frag: " << hdr.nbytes_frag << endl;
  out << "nbytes_data: " << hdr.nbytes_data << endl;
  out << "max_duration: " << hdr.max_duration << endl;
  out << "start_of_day: " << utimstr(hdr.start_of_day) << endl;
  out << "end_of_day: " << utimstr(hdr.end_of_day) << endl;
  out << "start_valid: " << utimstr(hdr.start_valid) << endl;
  out << "end_valid: " << utimstr(hdr.end_valid) << endl;
  out << "latest_expire: " << utimstr(hdr.latest_expire) << endl;
  out << "earliest_valid: " << utimstr(hdr.earliest_valid) << endl;

  if (hdr.lead_time_storage == LEAD_TIME_IN_DATA_TYPE) {
    out << "Lead time: stored in data_type" << endl;
  } else {
    out << "Lead time: stored in data_type2" << endl;
  }

  out << endl;
  
  if (print_min_posn) {
    out << "Minute position array:" << endl << endl;
    out << setw(8) << "Minute"
	<< setw(9) << "Tod"
	<< setw(8) << "Posn" << endl;
    for (int i = 0; i < MINS_IN_DAY; i++) {
      if (hdr.minute_posn[i] >= 0) {
	int hour = i / 60;
	int min = i - (hour * 60);
	out << setw(8) << i
	    << setw(6) << hour << ":"
	    << setw(2) << min
	    << setw(8) << hdr.minute_posn[i] << endl;
      }
    }
  }
  
  out << endl;

}

////////////////////////////////////////////////////
// printChunkRef()
//
// Prints the given chunk reference structure

void Spdb::printChunkRef(const chunk_ref_t *chunk_ref,
			 ostream &out,
                         const aux_ref_t *aux_ref /* = NULL*/)
  
{

  out << endl;
  out << "SPDB Chunk Reference" << endl << endl;
  out << "  valid_time: " << utimstr(chunk_ref->valid_time) << endl;
  out << "  expire_time: " << utimstr(chunk_ref->expire_time) << endl;
  if (aux_ref != NULL) {
    if (aux_ref->write_time != 0) {
      out << "  write_time: " << utimstr(aux_ref->write_time) << endl;
    }
    compression_t compress = (compression_t) aux_ref->compression;
    if (compress == COMPRESSION_GZIP) {
      out << "  compression: gzip" << endl;
    } else if (compress == COMPRESSION_BZIP2) {
      out << "  compression: bzip2" << endl;
    }
    if (strlen(aux_ref->tag) != 0) {
      out << "  tag: " << aux_ref->tag << endl;
    }
  }
  out << "  data_type: " << chunk_ref->data_type << endl;
  out << "  data_type2: " << chunk_ref->data_type2 << endl;
  out << "  offset: " << chunk_ref->offset << endl;
  out << "  len: " << chunk_ref->len << endl;
  out << endl;

}

////////////////////////////////////////////////////
// printChunk()
// Prints the given chunk object

void Spdb::printChunk(const chunk_t &chunk, ostream &out)
  
{

  out << endl;
  out << "SPDB chunk_t" << endl << endl;
  out << "  valid_time: " << utimstr(chunk.valid_time) << endl;
  out << "  expire_time: " << utimstr(chunk.expire_time) << endl;
  if (chunk.write_time != 0) {
    out << "  write_time: " << utimstr(chunk.write_time) << endl;
  }
  if (chunk.current_compression == COMPRESSION_GZIP) {
    out << "  current_compression: gzip" << endl;
  } else if (chunk.current_compression == COMPRESSION_BZIP2) {
    out << "  current_compression: bzip2" << endl;
  }
  if (chunk.tag.size() > 0) {
    out << "tag: " << chunk.tag << endl;
  }
  out << "  data_type: " << chunk.data_type << endl;
  out << "  data_type2: " << chunk.data_type2 << endl;
  out << "  len: " << chunk.len << endl;
  out << endl;

}

////////////////////////////////////////////////////////////////////
// makeUniqueLatest()
//
// This functions filters duplicate data from the chunk data,
// leaving the latest chunk for each data_type. All chunks with
// data_type of 0 will be left in the buffer.
//
// The intention is that you can call this function after any of
// the get routines, and filter the output of the get.

void Spdb::makeUniqueLatest()

{
  makeUnique(true);
}

////////////////////////////////////////////////////////////////////
// makeUniqueEarliest()
//
// This functions filters duplicate data from the chunk data,
// leaving the earliest chunk for each data_type. All chunks with
// data_type of 0 will be left in the buffer.
//
// The intention is that you can call this function after any of
// the get routines, and filter the output of the get.

void Spdb::makeUniqueEarliest()

{
  makeUnique(false);
}
  
////////////////////////////////////////////////////////////////////
// makeUnique
//
// This functions filters duplicate data from the chunk data,
// leaving the latest or earliest chunk for each data_type.
// If latest is true, we get the latest.
// If latest is false, we get the earliest.
//
// All chunks with data_type of 0 will be left in the buffer.
//
// The intention is that you can call this function after any of
// the get routines, and filter the output of the get.

void Spdb::makeUnique(bool latest /* = true*/ )

{
  
  // Allocate vector for unique_flags

  vector<bool> uniqueFlags;
  uniqueFlags.reserve(_nGetChunks);

  // Loop through the incoming chunks to find the unique ones.
  // check for uniqueness in the existing data

  chunk_ref_t *refs = (chunk_ref_t *) _getRefBuf.getPtr();
  aux_ref_t *auxs = (aux_ref_t *) _getAuxBuf.getPtr();
  bool isUnique = true;
  
  for (int i = 0; i < _nGetChunks; i++) {
    
    uniqueFlags[i] = true;
    
    if (refs[i].data_type == 0) {
      continue;
    }

    if (latest) {

      // Mode: latest
      // Since the chunks arrive in chronological order, a chunk
      // is unique if there is no following chunk with the same
      // data_type (or if the chunk's data_type is 0).
      
      for (int j = i + 1; j < _nGetChunks; j++) {
	if (refs[i].data_type == refs[j].data_type) {
          isUnique = false;
	  uniqueFlags[i] = false;
	  break;
	}
      } // j

    } else {

      // Mode: earliest
      // Since the chunks arrive in chronological order, a chunk
      // is unique if there is no previous chunk with the same
      // data_type (or if the chunk's data_type is 0).

      for (int j = 0; j < i; j++) {
	if (refs[i].data_type == refs[j].data_type) {
          isUnique = false;
	  uniqueFlags[i] = false;
	  break;
	}
      } // j

    }
    
  } // i

  if (isUnique) {
    // already unique, return now
    return;
  }
  
  // Load up unique chunks to temporary buffers

  MemBuf refBuf;
  MemBuf auxBuf;
  MemBuf dataBuf;

  int unique_offset = 0;
  int num_unique = 0;
  vector<compression_t> storedCompression;
  
  for (int i = 0; i < _nGetChunks; i++) {
    if (uniqueFlags[i]) {
      chunk_ref_t unique_hdr;
      num_unique++;
      unique_hdr = refs[i];
      unique_hdr.offset = unique_offset;
      unique_offset += refs[i].len;
      refBuf.add(&unique_hdr, sizeof(chunk_ref_t));
      auxBuf.add(&auxs[i], sizeof(aux_ref_t));
      if (!_getRefsOnly) {
	dataBuf.add(((char *) getChunkData() + refs[i].offset), refs[i].len);
      }
      if(i < (int) _storedCompression.size())
	storedCompression.push_back(_storedCompression[i]);
    }
  } // i

  // copy to main variables

  _nGetChunks = num_unique;
  _getRefBuf = refBuf;
  _getAuxBuf = auxBuf;
  _getDataBuf = dataBuf;
  _storedCompression = storedCompression;

  // load up chunks

  _loadChunksFromGet();

}

/////////////////////////////////////////////////////////
// _firstPosnAfter()
//
// Returns the first chunk ref posn at or after the
// requested time, on the given day.
//
// Returns -1 if no chunk has been stored at or after
// this time.

int Spdb::_firstPosnAfter(time_t start_time)
     
{

  // constrain start time

  if (start_time < (time_t) _hdr.start_of_day) {
    start_time = _hdr.start_of_day;
  } else if (start_time > (time_t) _hdr.end_of_day) {
    start_time = _hdr.end_of_day;
  }

  int smin = (start_time % SECS_IN_DAY) / SECS_IN_MIN;

  int start_posn = -1;
  si32 *min_posn = _hdr.minute_posn + smin;
  while (start_posn < 0 && smin < MINS_IN_DAY) {
    start_posn = *min_posn;
    smin++;
    min_posn++;
  }

  if (start_posn < 0) {
    // no data found at or after the start time
    return -1;
  }
  
  chunk_ref_t *ref = (chunk_ref_t *) _hdrRefBuf.getPtr() + start_posn;

  for (int i = start_posn; i < _hdr.n_chunks; i++, ref++) {
    if ((time_t) ref->valid_time >= start_time) {
      return i;
    }
  } // i

  return -1;

}

//////////////////////////////////////////////////////
// _getFirstTimeAfter()
//
// Returns the first chunk time at or after the
// requested time, and before the end_time.
//
// Returns 0 on success, -1 on failure

int Spdb::_getFirstTimeAfter(time_t request_time,
			     time_t end_time,
			     int data_type,
			     int data_type2,
			     time_t &data_time)
  
{

  time_t search_time = request_time;

  while (search_time <= end_time) {

    if (_openFiles(_prodId, _prodLabel,
		   search_time, ReadMode)) {
      if (!_emptyDay) {
	return -1;
      }
    }

    // Find the first minute value for which there is data.
    
    int rmin = (search_time % SECS_IN_DAY) / SECS_IN_MIN;
    int emin = rmin + ((end_time - search_time) / SECS_IN_MIN) + 1;
    emin = MAX(emin, MINS_IN_DAY - 1);
    
    int posn = -1;
    si32 *min_posn = _hdr.minute_posn + rmin;
    while (posn < 0 && rmin <= emin) {
      posn = *min_posn;
      rmin++;
      min_posn++;
    }
    
    // Check the valid time for the found data.  Look for the
    // first data with an appropriate valid time.  We must do
    // this because the data found above could be earlier than
    // the requested time by a few seconds.  (e.g. data for
    // 13:31:15 when requesting data after 13:31:30, both appear
    // under minute 13:31).  Also make sure we find the appropriate
    // data_type.
    
    if (posn >= 0) {
      
      chunk_ref_t *ref = (chunk_ref_t *) _hdrRefBuf.getPtr() + posn;
      aux_ref_t *aux = (aux_ref_t *) _hdrAuxBuf.getPtr() + posn;
      for (int i = posn; i < _hdr.n_chunks; i++, ref++) {
	if ((time_t) ref->valid_time >= search_time &&
	    (time_t) ref->valid_time <= end_time &&
	    _acceptRef(data_type, data_type2, *ref, *aux)) {
	  data_time = ref->valid_time;
	  return 0;
	} else if ((time_t) ref->valid_time > end_time) {
	  return -1;
	}
      } // i
      
    }
  
    // no valid time found after the request time - move on
    // to the next day
  
    date_time_t next_time;
    next_time.unix_time = search_time + 86400;
    uconvert_from_utime(&next_time);
    next_time.hour = 0;
    next_time.min = 0;
    next_time.sec = 0;
    uconvert_to_utime(&next_time);
    if (next_time.unix_time == search_time) {
      return -1;
    }
    search_time = next_time.unix_time;
    
  } // while

  return -1;
    
}

/////////////////////////////////////////////////////
// _getFirstTimeBefore()
//
// Returns the first chunk time at or before the
// requested time, and after the start_time.
//
// Returns 0 on success, -1 on failure

int Spdb::_getFirstTimeBefore(time_t request_time,
			      time_t start_time,
			      int data_type,
			      int data_type2,
			      time_t &data_time)
  
{

  time_t search_time = request_time;

  while (search_time >= start_time) {

    if (_openFiles(_prodId, _prodLabel,
		   search_time, ReadMode)) {
      if (!_emptyDay) {
	return -1;
      }
    }
    
    // Find the first minute value for which there is data.
    
    int rmin = ((search_time % SECS_IN_DAY) / SECS_IN_MIN);
    int smin = (rmin - ((search_time - start_time) / SECS_IN_MIN)) - 1;
    smin = MAX(smin, 0);
  
    int posn = -1;
    si32 *min_posn = _hdr.minute_posn + rmin;
    while (posn < 0 && rmin >= smin) {
      posn = *min_posn;
      rmin--;
      min_posn--;
    }
    
    // Check the valid time for the found data.  Look for the
    // first data with an appropriate valid time.  We must do
    // this because there could be data later than the data
    // found above, but still earlier than the requested time
    // (e.g. data existing for 13:31:15 and 13:31:30 when
    // requesting data before 13:31:45, all appear under
    // minute 13:31).  Also, make sure we find the appropriate
    // data type.
    ///
    
    if (posn >= 0) {
      
      int posn_ahead;
      
      if ((posn_ahead = _posn1MinAhead(posn)) < 0) {
	return -1;
      }
      
      chunk_ref_t *ref = (chunk_ref_t *) _hdrRefBuf.getPtr() + posn_ahead;
      aux_ref_t *aux = (aux_ref_t *) _hdrAuxBuf.getPtr() + posn_ahead;
      for (int i = posn_ahead; i >= 0; i--, ref--) {
	if ((time_t) ref->valid_time <= search_time &&
	    (time_t) ref->valid_time >= start_time &&
	    _acceptRef(data_type, data_type2, *ref, *aux)) {
	  data_time = ref->valid_time;
	  return 0;
	} else if ((time_t) ref->valid_time < start_time) {
	  return -1;
	}
      } // i
      
    } // if (posn >= 0)
    
    // no valid time found after the request time - move back to
    // the previous day
  
    date_time_t prev_time;
    prev_time.unix_time = search_time - 86400;
    uconvert_from_utime(&prev_time);
    prev_time.hour = 23;
    prev_time.min = 59;
    prev_time.sec = 59;
    uconvert_to_utime(&prev_time);
    if (prev_time.unix_time == search_time) {
      return -1;
    }
    search_time = prev_time.unix_time;
    
  } // while

  return -1;
    
}

#define UNIX_TIME_MAX 2147483647

///////////////////////////////////////////////////////////
// _getTimeNearest()
//
// Gets the nearest time with data within the time_margin.
//
// Returns 0 on success, -1 on failure

int Spdb::_getTimeNearest(time_t request_time,
			  int time_margin,
			  int data_type,
			  int data_type2,
			  time_t &time_nearest)
     
{

  time_nearest = 0;

  time_t end_time;
  if (time_margin >= 0) {
    end_time = request_time + time_margin;
  } else {
    end_time = UNIX_TIME_MAX;
  }

  time_t nearest_after;
  bool after_found;
  if (_getFirstTimeAfter(request_time, end_time,
			 data_type, data_type2,
			 nearest_after) == 0) {
    after_found = true;
  } else {
    after_found = false;
  }
    
  time_t start_time;
  if (time_margin >= 0) {
    start_time = request_time - time_margin;
  } else {
    start_time = 0;
  }

  time_t nearest_before;
  bool before_found;
  if (_getFirstTimeBefore(request_time,
			  start_time, data_type, data_type2,
			  nearest_before) == 0) {
    before_found = true;
  } else {
    before_found = false;
  }
    
  if (before_found && after_found) {

    if ((nearest_after - request_time) <
	(request_time - nearest_before)) {
      time_nearest = nearest_after;
    } else {
      time_nearest = nearest_before;
    }
    
  } else if (before_found) {
    
    time_nearest = nearest_before;
    
  } else if (after_found) {
    
    time_nearest = nearest_after;
    
  } else {
    
    return -1;
    
  }
  
  return 0;

}

//////////////////////////////////////////
// _posn1MinAhead()
//
// Return posn ahead by 1 minute, or
// the end of the array if that is less than
// 1 min ahead.
//
// Returns -1 on error.
///

int Spdb::_posn1MinAhead(int start_posn)

{

  chunk_ref_t *refs = (chunk_ref_t *) _hdrRefBuf.getPtr();
  time_t start_time = refs[start_posn].valid_time;
  time_t target_time = start_time + SECS_IN_MIN;

  for (int i = start_posn + 1; i < _hdr.n_chunks; i++) {
    if ((time_t) refs[i].valid_time >= target_time) {
      return i;
    }
  }

  return (_hdr.n_chunks - 1);

}

//////////////////////////////////////////////////////
// _posnAtTime()
//
// Returns the posn of the first chunk ref stored for
// data_types at the valid time.
// If data_type or data_type2 is 0, the data_type check is not done.
//
// This is -1 if no chunk has been stored for this time.

int Spdb::_posnAtTime(time_t valid_time,
		      int data_type, int data_type2)

{
  
  int vmin = (valid_time % SECS_IN_DAY) / SECS_IN_MIN;
  int minute_posn = _hdr.minute_posn[vmin];

  if (minute_posn < 0) {
    return -1;
  }

  chunk_ref_t *ref = (chunk_ref_t *) _hdrRefBuf.getPtr() + minute_posn;
  aux_ref_t *aux = (aux_ref_t *) _hdrAuxBuf.getPtr() + minute_posn;

  for (int i = minute_posn; i < _hdr.n_chunks; i++, ref++, aux++) {
    if (((time_t) ref->valid_time - valid_time) > SECS_IN_MIN) {
      return -1;
    }
    if ((time_t) ref->valid_time == valid_time) {
      if (_acceptRef(data_type, data_type2, *ref, *aux)) {
	return i;
      }
    }
  } // i

  return -1;

}

/////////////////////////////////////////////////////
// _readChunkRefs()
//
// Read in the chunk refs from the indx file
//
// If there are not enough chunk headers in the file, update the
// n_chunks value in the index header.  This is done so we can recover
// files which have been corrupted by a process dying before the chunk
// header is written.  Because we are updating the index header value,
// the file will no longer be corrupted after the next write by the
// writing process so we don't need to rewrite the index header here.

void Spdb::_readChunkRefs()

{     
  
  // chunk refs

  _hdrRefBuf.reserve(_hdr.n_chunks * sizeof(chunk_ref_t));
  
  int n_chunks_read = ta_fread(_hdrRefBuf.getPtr(), sizeof(chunk_ref_t),
                               _hdr.n_chunks, _indxFile);
  chunk_refs_from_BE((chunk_ref_t *) _hdrRefBuf.getPtr(), n_chunks_read);

  if (n_chunks_read < _hdr.n_chunks) {
    // trim down to size
    _hdr.n_chunks = n_chunks_read;
    _hdrRefBuf.reserve(_hdr.n_chunks * sizeof(chunk_ref_t));
  }
  
  // aux refs
  // If these are missing, they are set to 0
  
  _hdrAuxBuf.reserve(_hdr.n_chunks * sizeof(aux_ref_t));
  memset(_hdrAuxBuf.getPtr(), 0, _hdrAuxBuf.getLen());
  
  int n_aux_read = ta_fread(_hdrAuxBuf.getPtr(), sizeof(aux_ref_t),
                            _hdr.n_chunks, _indxFile);
  
  if (n_aux_read < _hdr.n_chunks) {
    memset(_hdrAuxBuf.getPtr(), 0, _hdrAuxBuf.getLen());
  } else {
    aux_refs_from_BE((aux_ref_t *) _hdrAuxBuf.getPtr(), n_aux_read);
  }
  
}

/////////////////////////////////////////////////
// do the chunk read if the data type is correct
// Appends to the ref, aux and data buffers, and
// updates the _nGetChunks counter
// Returns 0 on success, -1 on failure.

int Spdb::_checkTypeThenReadChunk(int data_type,
                                  int data_type2,
                                  const chunk_ref_t &ref,
                                  const aux_ref_t &aux,
                                  MemBuf &readBuf)
  
{

  // check the data types are acceptable

  if (!_acceptRef(data_type, data_type2, ref, aux)) {
    return 0;
  }
    
  // copy the references
  
  chunk_ref_t refCopy(ref);
  aux_ref_t auxCopy(aux);
      
  if (_getRefsOnly) {
    
    // add to the get buffers
    
    _getRefBuf.add(&refCopy, sizeof(chunk_ref_t));
    _getAuxBuf.add(&auxCopy, sizeof(aux_ref_t));
    
  } else {
    
    // read the chunk

    if (_readChunk(refCopy, auxCopy,
                   readBuf, _chunkUncompressOnGet)) {
      return -1;
    }
    
    // set the offset for the chunk reference in the
    // buffer we are building
    
    refCopy.offset = _getDataBuf.getLen();

    // compression?

    if (_chunkUncompressOnGet) {
    }
    
    // add to the get buffers
    
    _getRefBuf.add(&refCopy, sizeof(chunk_ref_t));
    _getAuxBuf.add(&auxCopy, sizeof(aux_ref_t));

    // add the chunk to our buffer
    _getDataBuf.concat(readBuf);
    
  }
  
  _storedCompression.push_back((compression_t) aux.compression);
  _nGetChunks++;
  return 0;
  
}
      
//////////////////////////////////////////
// _readChunk()
//
// Reads chunk into buffer provided.
// Uncompresses if needed, adjusts the values
// in ref and aux accordingly.
//
// Returns 0 on success, -1 on failure.

int Spdb::_readChunk(chunk_ref_t &ref,
                     aux_ref_t &aux,
                     MemBuf &readBuf,
                     bool doUncompress)
  
{
  
  // allocate space in buffer for chunk data
  
  void *chunk = readBuf.reserve(ref.len);
  
  // seek to offset
  
  if (fseek(_dataFile, ref.offset, SEEK_SET) < 0) {
    int errNum = errno;
    _errStr += "ERROR - Spdb::_readChunk\n";
    _addStrErr(" Prod label: ", _hdr.prod_label);
    _addIntErr(" Cannot seek to data offset: ", ref.offset);
    _addStrErr(_dataPath, strerror(errNum));
    return -1;
  }
  
  // read data
  
  if ((ui32) ta_fread(chunk, 1, ref.len, _dataFile) != ref.len) {
    int errNum = errno;
    _errStr += "ERROR - Spdb::_readChunk\n";
    _addStrErr(" Prod label: ", _hdr.prod_label);
    _addIntErr(" Cannot read chunk of len: ", ref.len);
    _addIntErr(" Data offset: ", ref.offset);
    _addStrErr(_dataPath, strerror(errNum));
    return -1;
  }

  // uncompress chunk if it is compressed

  if (doUncompress && ta_is_compressed(chunk, ref.len)) {
    ui64 nbytesUncompressed;
    void *uncompressed  = ta_decompress(chunk, &nbytesUncompressed);
    if (uncompressed == NULL) {
      _errStr += "WARNING - Spdb::_readChunk\n";
      _addStrErr(" Prod label: ", _hdr.prod_label);
      _addIntErr(" Cannot uncompress chunk of len: ", ref.len);
      _addIntErr(" Data offset: ", ref.offset);
    } else {
      chunk = readBuf.reserve(nbytesUncompressed);
      memcpy(chunk, uncompressed, nbytesUncompressed);
      ref.len = nbytesUncompressed;
      aux.compression = 0;
      ta_compress_free(uncompressed);
    }
  }

  return 0;

}

////////////////////////////////////////////////////////
// _setEarliestValid()
//
// Set the earliest_valid_time for any files which
// have times which overlap the valid period of
// this product.
//
// Returns 0 on success, -1 on failure

int Spdb::_setEarliestValid(time_t valid_time, time_t expire_time)

{

  int valid_day = valid_time / SECS_IN_DAY;
  int expire_day = expire_time / SECS_IN_DAY;
  
  // limit the expire day to 3 days ahead

  expire_day = MIN(expire_day, valid_day + 3);

  if (expire_day > valid_day) {

    // create local object, set directory

    Spdb dayObj;
    dayObj._dir = _dir;

    for (int iday = valid_day + 1; iday <= expire_day; iday++) {

      // compute midday time

      time_t midday_time = iday * SECS_IN_DAY + SECS_IN_DAY / 2;
      
      // read in headers

      if (dayObj._openFiles(_prodId, _prodLabel,
			    midday_time, WriteMode, false)) {
	if (!_emptyDay) {
	  _errStr += "ERROR - Spdb::_setEarliestValid\n";
	  return -1;
	}
      }

      // adjust earliest valid time as necessary
      
      if (valid_time < (time_t) dayObj._hdr.earliest_valid) {
	dayObj._hdr.earliest_valid = valid_time;
	dayObj._writeIndxFile(false);
      }
      
      dayObj._closeFiles(false);

    } // iday

  }

  return 0;

}

///////////////////////////////////////////////////////////
// _storeChunk()
//
// Stores a chunk in the data base. There are 3 modes:
//   putModeOnce - no overwrite
//   putModeOver - will place the new chunk over the
//     old one if it fits, otherwise at the end of the file
//   putModeAdd - adds the chunk, allows multiple chunks at
//     the same valid time.
//   putModeAdd - adds the chunk if identical chunk does not
//     already exist. Allows multiple chunks at
//     the same valid time.
//
// Returns 0 on success, -1 on failure

int Spdb::_storeChunk(const chunk_ref_t *input_ref,
                      const aux_ref_t *input_aux,
		      const void *input_data)
  
{

  bool new_ref = true, append = true;
  int posn = 0;
  chunk_ref_t inref = *input_ref;
  aux_ref_t inaux = *input_aux;

  // set new_ref flag depending on mode

  switch (_putMode) {
    
  case putModeOnce:
    posn = _storedPosn(inref.valid_time,
		       inref.data_type, inref.data_type2);
    if (posn >= 0) {
      return -1;
    } else {
      new_ref = true;
    }
    break;

  case putModeAdd:
    new_ref = true;
    break;

  case putModeAddUnique:
    if (_checkStored(inref, input_data)) {
      return 0;
    } else {
      new_ref = true;
    }
    break;

  case putModeOver:
    posn = _storedPosn(inref.valid_time,
		       inref.data_type, inref.data_type2);
    if (posn >= 0) {
      new_ref = false;
    } else {
      new_ref = true;
    }
    break;

  default:
    new_ref = true;
    
  } // switch (mode)

  if (new_ref) {
    
    append = true;
    
  } else {
    
    chunk_ref_t *existRef = (chunk_ref_t *) _hdrRefBuf.getPtr() + posn;
    
    if (existRef->len >= inref.len) {

      // chunk will fit in previous location. Fragment
      // is created at end of slot if they are not
      // the same size.
      
      int nfrag = existRef->len - inref.len;
      _hdr.nbytes_frag += nfrag;
      _hdr.nbytes_data -= nfrag;
      inref.offset = existRef->offset;
      append = false;
      
    } else {

      // chunk will not fit in previous location,
      // store at end of file. Entire slot
      // becomes a fragment.
      
      _hdr.nbytes_frag += existRef->len;
      _hdr.nbytes_data -= existRef->len;
      append = true;
      
    } // if (existRef->len >= len)

  } // if (append)

  // write chunk data - inref.offset is set as a side effect
  
  if (_writeChunk(inref, input_data, append)) {
    _errStr += "ERROR - Spdb::_storeChunk\n";
    _errStr += "  Cannot write chunk to data file.\n";
    _addStrErr("  Product label: ", _hdr.prod_label);
    return -1;
  }
  
  if (new_ref) {
    
    // add chunk reference to list

    _addChunkRef(inref, inaux);

  } else {

    // store chunk ref at previous location

    chunk_ref_t *ref = (chunk_ref_t *) _hdrRefBuf.getPtr() + posn;
    *ref = inref;

    aux_ref_t *aux = (aux_ref_t *) _hdrAuxBuf.getPtr() + posn;
    *aux = inaux;

  }

  // keep stats up to date
  
  _hdr.max_duration =
    MAX(_hdr.max_duration,
	((time_t) inref.expire_time - (time_t) inref.valid_time));
  _hdr.start_valid =
    MIN(((time_t) _hdr.start_valid), ((time_t) inref.valid_time));
  _hdr.end_valid = 
    MAX(((time_t) _hdr.end_valid), ((time_t) inref.valid_time));
  _hdr.latest_expire =
    MAX(((time_t) _hdr.latest_expire), ((time_t) inref.expire_time));
  _hdr.earliest_valid =
    MIN(((time_t) _hdr.earliest_valid), ((time_t) inref.valid_time));

  _setEarliestValid(inref.valid_time,
		    inref.expire_time);

  return 0;

}

////////////////////////////////////////////////////
// _addChunkRef()
//
// returns posn of chunk in ref array

int Spdb::_addChunkRef(const chunk_ref_t &inref,
                       const aux_ref_t &inaux)
  
{

  // alloc space

  _hdrRefBuf.reserve((_hdr.n_chunks + 1) * sizeof(chunk_ref_t));
  _hdrAuxBuf.reserve((_hdr.n_chunks + 1) * sizeof(aux_ref_t));
  
  // find posn at which to store the chunk - they
  // are stored in valid_time order

  int store_posn = 0;
  bool latest = false;
  {
    chunk_ref_t *ref  =
      (chunk_ref_t *) _hdrRefBuf.getPtr() + (_hdr.n_chunks - 1);
    int i;
    for (i = _hdr.n_chunks - 1; i >= 0; i--, ref--) {
      if ((time_t) inref.valid_time >= (time_t) ref->valid_time) {
        break;
      }
    } // i
    
    store_posn = i + 1;
    if (store_posn == _hdr.n_chunks) {
      latest = true;
    } else {
      latest = false;
    }
  }

  // move the entries below down one slot
  
  if (!latest) {

    chunk_ref_t *ref =
      (chunk_ref_t *) _hdrRefBuf.getPtr() + (_hdr.n_chunks - 1);
    chunk_ref_t *ref2 = ref + 1;
    for (int i = _hdr.n_chunks; i > store_posn; i--, ref--, ref2--) {
      *ref2 = *ref;
    }

    aux_ref_t *aux =
      (aux_ref_t *) _hdrAuxBuf.getPtr() + (_hdr.n_chunks - 1);
    aux_ref_t *aux2 = aux + 1;
    for (int i = _hdr.n_chunks; i > store_posn; i--, aux--, aux2--) {
      *aux2 = *aux;
    }

  } // if (!latest)

  // store the refs in the header

  chunk_ref_t *ref = (chunk_ref_t *) _hdrRefBuf.getPtr() + store_posn;
  *ref = inref;
  
  aux_ref_t *aux = (aux_ref_t *) _hdrAuxBuf.getPtr() + store_posn;
  *aux = inaux;
  
  // increment the number of chunks and nbytes_data
  
  _hdr.n_chunks++;
  _hdr.nbytes_data += inref.len;

  // amend the minute posn array

  int vmin = ((time_t) inref.valid_time % SECS_IN_DAY) / SECS_IN_MIN;
  si32 *min_posn = _hdr.minute_posn + vmin;

  if (*min_posn == -1) {
    *min_posn = store_posn;
  } else if (*min_posn > store_posn) {
    *min_posn = store_posn;
  }

  if (!latest) {
    min_posn = _hdr.minute_posn + vmin + 1;
    for (int i = vmin + 1; i < MINS_IN_DAY; i++, min_posn++) {
      if (*min_posn != -1) {
	*min_posn = *min_posn + 1;
      }
    } // i
  } // if (!latest)

  return store_posn;
  
}

///////////////////////////////////////////////////////////
// _eraseChunks()
//
// Erase chunks from the data base, given the valid time and
// data_type2. If either data_type is 0, it is ignored.
//
// Returns 0 on success, -1 on failure

int Spdb::_eraseChunks(time_t valid_time, int data_type, int data_type2)
  
{

  int posn = _storedPosn(valid_time, data_type, data_type2);
  if (posn < 0) {
    _errStr += "ERROR - Spdb::_eraseChunk\n";
    _errStr += "  No relevant chunk to erase.\n";
    _addStrErr("  Product label: ", _hdr.prod_label);
    _addStrErr("  Valid time: ", utimstr(valid_time));
    _addIntErr("  Data type: ", data_type);
    _addIntErr("  Data type2: ", data_type2);
    return -1;
  }

  while (posn < _hdr.n_chunks) {

    chunk_ref_t *ref = (chunk_ref_t *) _hdrRefBuf.getPtr() + posn;
    aux_ref_t *aux = (aux_ref_t *) _hdrAuxBuf.getPtr() + posn;

    if ((time_t) ref->valid_time != valid_time ||
	!_acceptRef(data_type, data_type2, *ref, *aux)) {
      break;
    }

    // adjust byte counts

    _hdr.nbytes_frag += ref->len;
    _hdr.nbytes_data -= ref->len;

    // erase the chunk ref, packing the rest of them

    _eraseChunkRef(valid_time, posn);

    _hdr.n_chunks--;
    
  } // while

  return 0;

}

////////////////////////////////////////////////////
// _eraseChunkRef()
//
// erase the given chunk ref

void Spdb::_eraseChunkRef(time_t valid_time, int posn)
  
{

  // amend the minute posn array
  
  int vmin = (valid_time % SECS_IN_DAY) / SECS_IN_MIN;
  bool reset = true;
  
  if (posn > 0) {
    chunk_ref_t *prevRef  = (chunk_ref_t *) _hdrRefBuf.getPtr() + (posn - 1);
    int prevVmin = ((time_t) prevRef->valid_time % SECS_IN_DAY) / SECS_IN_MIN;
    if (prevVmin == vmin) {
      reset = false;
    }
  }
  
  if (reset && posn < (_hdr.n_chunks - 1)) {
    chunk_ref_t *nextRef  = (chunk_ref_t *) _hdrRefBuf.getPtr() + (posn + 1);
    int nextVmin = ((time_t) nextRef->valid_time % SECS_IN_DAY) / SECS_IN_MIN;
    if (nextVmin == vmin) {
      reset = false;
    }
  }

  if (reset) {
    _hdr.minute_posn[vmin] = -1;
  }
  
  si32 *min_posn = _hdr.minute_posn + vmin + 1;
  for (int i = vmin + 1; i < MINS_IN_DAY; i++, min_posn++) {
    if (*min_posn != -1) {
      *min_posn = *min_posn - 1;
    }
  } // i

  // move the refs below up one slot

  chunk_ref_t *ref  = (chunk_ref_t *) _hdrRefBuf.getPtr() + posn;
  for (int ii = posn; ii < _hdr.n_chunks - 1; ii++, ref++) {
    *ref = *(ref + 1);
  }

  aux_ref_t *aux  = (aux_ref_t *) _hdrAuxBuf.getPtr() + posn;
  for (int ii = posn; ii < _hdr.n_chunks - 1; ii++, aux++) {
    *aux = *(aux + 1);
  }

}

///////////////////////////////////////////////////////
// _storedPosn()
//
// Finds posn of data of this type at the valid time.
// If data_type is 0, type check is ignored.
//
// Returns posn on success, -1 on failure.

int Spdb::_storedPosn(time_t valid_time,
		      int data_type, int data_type2)
  
{

  if (_hdr.n_chunks == 0) {
    return -1;
  }
  
  if (valid_time < (time_t) _hdr.start_valid ||
      valid_time > (time_t) _hdr.end_valid) {
    return -1;
  }

  int posn;
  if ((posn = _posnAtTime(valid_time, data_type, data_type2)) < 0) {
    return -1;
  } else {
    return posn;
  }

}

///////////////////////////////////////////////////////
// _checkStored()
//
// Check if given chunk had already been stored.
//
// Returns true if stored, false if not.

int Spdb::_checkStored(const chunk_ref_t &inref,
		       const void *input_data)

{

  // fetch the data matching the input ref - we need to make
  // sure that we get the data

  _getRefsOnly = false;
  int iret = _fetchExact(inref.valid_time,
			 inref.data_type,
			 inref.data_type2);
  if (iret) {
    return false;
  }
  
  // check through the returned chunks for a match

  if (_nGetChunks == 0) {
    return false;
  }

  chunk_ref_t *stored_ref = (chunk_ref_t *) _getRefBuf.getPtr();
  for (int i = 0; i < _nGetChunks; i++, stored_ref++) {
    if (inref.valid_time == stored_ref->valid_time &&
	inref.expire_time == stored_ref->expire_time &&
	inref.data_type == stored_ref->data_type &&
	inref.data_type2 == stored_ref->data_type2 &&
	inref.len == stored_ref->len) {
      void *stored_data = (void *)
	((char *) _getDataBuf.getPtr() + stored_ref->offset);
      if (memcmp(input_data, stored_data, inref.len) == 0) {
	return true;
      }
    }
  } // i

  return false;

}

//////////////////////////////////////////
// _writeChunk()
//
// Side effect: if ref has negative offset, this is filled in.
//
// Returns 0 on success, -1 on failure.

int Spdb::_writeChunk(chunk_ref_t &inref,
		      const void *input_data,
		      bool append)
  
{
  
  if (append) {

    // for appending, seek to end of data file, set ref offset
    
    fseek(_dataFile, 0, SEEK_END);
    inref.offset = ftell(_dataFile);

  } else {

    // seek to offset
    
    fseek(_dataFile, inref.offset, SEEK_SET);

  }
  
  // write data
  
  if (ta_fwrite(input_data, 1, inref.len, _dataFile)
      != inref.len) {
    int errNum = errno;
    _errStr += "ERROR - Spdb::_writeChunk\n";
    _addStrErr("  Product label: ", _hdr.prod_label);
    _addStrErr("  Cannot write chunk for time: ",
	       utimstr(inref.valid_time));
    _addStrErr(_dataPath, strerror(errNum));
    return -1;
  }

  return 0;

}

////////////////////////////
// defragment the data file

int Spdb::_defrag()

{

  // check if we need to defrag
  // only defrag if:
  //   nbytesFrag > 10000 and fragFract > 0.05, or
  //   fragFract > 0.3

  if (_hdr.nbytes_data == 0) return 0;

  double fragFract =
    (double) _hdr.nbytes_frag / (double) _hdr.nbytes_data; 
  int nbytesFrag = _hdr.nbytes_frag;

  if ((nbytesFrag < 10000 || fragFract < 0.05) && fragFract < 0.3) {
    return 0;
  }

  // open a temporary file

  string defragPath = _dataPath;
  defragPath += ".defrag";

  FILE *defragFile;
  if ((defragFile = fopen(defragPath.c_str(), "w")) == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Spdb::_defrag\n";
    _addStrErr("  Prod label: ", _hdr.prod_label);
    _addStrErr("  Cannot open tmp data path: ", defragPath);
    _addStrErr("  ", strerror(errNum));
    return -1;
  }

  // read all entries, saving changed references and writing the 
  // data to the data path

  MemBuf readBuf;
  MemBuf refBuf;
  MemBuf auxBuf;

  chunk_ref_t *ref  = (chunk_ref_t *) _hdrRefBuf.getPtr();
  aux_ref_t *aux  = (aux_ref_t *) _hdrAuxBuf.getPtr();

  for (int i = 0; i < _hdr.n_chunks; i++, ref++, aux++) {

    // make local copies of the references

    chunk_ref_t refCopy(*ref);
    aux_ref_t auxCopy(*aux);
   
    // read the chunk, do not uncompress

    if (_readChunk(refCopy, auxCopy, readBuf, false) == 0) {

      // add to reference buffers, setting the offset first
      
      refCopy.offset = (ui32) ftell(defragFile);
      refBuf.add(&refCopy, sizeof(chunk_ref_t));
      auxBuf.add(&auxCopy, sizeof(aux_ref_t));

      // write data to defrag file
      
      if (ta_fwrite(readBuf.getPtr(), readBuf.getLen(), 1, defragFile) != 1) {
        int errNum = errno;
        _errStr += "ERROR - Spdb::_defrag\n";
        _addStrErr("  Prod label: ", _hdr.prod_label);
        _errStr += "  Cannot write data to truncated data file.\n";
        _addStrErr("  File path: ", defragPath);
        _addIntErr("  Data len: ", readBuf.getLen());
        _addStrErr("  ", strerror(errNum));
        fclose(defragFile);
        return -1;
      }

    }
    
  } // i

  // close the defrag file and the data file

  fclose(defragFile);
  fclose(_dataFile);
  _dataFile = NULL;

  // rename the defrag path to the data path

  if (rename(defragPath.c_str(), _dataPath)) {
    int errNum = errno;
    _errStr += "ERROR - Spdb::_defrag\n";
    _addStrErr("  Prod label: ", _hdr.prod_label);
    _errStr += "  Cannot rename defrag file to data file.\n";
    _addStrErr("  Defrag path: ", defragPath);
    _addStrErr("  Data   path: ", _dataPath);
    _addStrErr("  ", strerror(errNum));
    return -1;
  }

  // set the headers and ref and aux buffers

  _hdr.nbytes_frag = 0;
  _hdrRefBuf = refBuf;
  _hdrAuxBuf = auxBuf;

  return 0;

}

bool Spdb::_acceptRef(int data_type,
		      int data_type2,
		      const chunk_ref_t &ref,
		      const aux_ref_t &aux)

{
  
  if (_checkWriteTimeOnGet) {
    if (aux.write_time > _latestValidWriteTime) {
      return false;
    }
  }

  if (_respectZeroTypes) {
    if (data_type == ref.data_type &&
	data_type2 == ref.data_type2) {
      return true;
    } else {
      return false;
    }
  }

  if (data_type2 == 0) {
    if (data_type == 0 || data_type == ref.data_type) {
      return true;
    }
  } else if (data_type == 0) {
    if (data_type2 == ref.data_type2) {
      return true;
    }
  } else if (data_type == ref.data_type &&
	     data_type2 == ref.data_type2) {
    return true;
  }
  
  return false;

}

////////////////////
// clear error string

void Spdb::_clearErrStr() const
{
  TaStr::AddStr(_errStr, "Time for following error: ", DateTime::str());
}

////////////////////////////////
// byte swapping for chunk refs

void Spdb::chunk_refs_to_BE(chunk_ref_t *refs, int nn)
{
  BE_from_array_32(refs, nn * sizeof(chunk_ref_t));
}

void Spdb::chunk_refs_from_BE(chunk_ref_t *refs, int nn)
{
  BE_to_array_32(refs, nn * sizeof(chunk_ref_t));
}

void Spdb::aux_refs_to_BE(aux_ref_t *auxs, int nn)
{
  aux_ref_t *aux = auxs;
  for (int ii = 0; ii < nn; ii++, aux++) {
    BE_from_array_32(aux, sizeof(aux_ref_t) - TAG_LEN);
  }
}

void Spdb::aux_refs_from_BE(aux_ref_t *auxs, int nn)
{
  aux_ref_t *aux = auxs;
  for (int ii = 0; ii < nn; ii++, aux++) {
    BE_to_array_32(aux, sizeof(aux_ref_t) - TAG_LEN);
  }
}
