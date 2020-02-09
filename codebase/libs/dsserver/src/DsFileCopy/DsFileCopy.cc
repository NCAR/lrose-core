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
// DsFileCopy.cc
//
// DsFileCopy object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////
//
// The DsFileCopy object performs simple buffered file io both directly
// to the local disk and via a DsFileCopyServer.
//
///////////////////////////////////////////////////////////////

#include <dsserver/DsFileCopy.hh>
#include <dsserver/DsClient.hh>
#include <dsserver/DsLocator.hh>
#include <didss/RapDataDir.hh>
#include <toolsa/umisc.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <cstdlib>
#include <cstdarg>
#include <iostream>
#include <sys/stat.h>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

DsFileCopy::DsFileCopy(const DsMessage::memModel_t mem_model) :
  _msg(mem_model)
  
{

  _fileLen = 0;
  _fileBuf = NULL;
  _doPut = false;
  _debug = false;
  _maxFileAge = -1;
  _openTimeoutMsecs = -1;

}

////////////////////////////////////////////////////////////
// destructor

DsFileCopy::~DsFileCopy()

{
  if (_fileBuf) {
    ufree(_fileBuf);
  }
}

/////////////////////////////////////////////////////////////
// enquireForPut()
//
// Enquire whether to put a file.
//
// Afterwards, you can check DoPut() to decide whether to put or not.
//
// Returns 0 on success, -1 on error
// Error message available via getErrorStr()

int DsFileCopy::enquireForPut(const string &data_dir,
			      const DsURL &dest_url,
			      const LdataInfo &ldata_info,
			      const string &file_name,
			      const int overwrite_age /* = -1*/)
  
{

  _errStr = "";
  if (_debug) {
    _errStr = "DsFileCopy::enquireForPut\n";
    TaStr::AddStr(_errStr, "  ", DateTime::str());
    TaStr::AddStr(_errStr, "  dir: ", data_dir);
    TaStr::AddStr(_errStr, "  dest_url: ", dest_url.getURLStr());
  }

  _dataDir = data_dir;
  _destUrl = dest_url;
  string dataDirPath;
  RapDataDir.fillPath(_dataDir, dataDirPath);

  // set the file path

  _filePath = ldata_info.getDataPath();
  
  // stat the file
  
  struct stat fileStat;
  if (ta_stat(_filePath.c_str(), &fileStat)) {
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::enquireForPut", "");
    TaStr::AddStr(_errStr, "Cannot stat file: ", _filePath);
    return -1;
  }
  _fileLen = fileStat.st_size;

  // check file age if appropriate

  if (_maxFileAge > 0) {
    time_t now = time(NULL);
    int fileAge = now - fileStat.st_mtime;
    if (fileAge > _maxFileAge) {
      TaStr::AddStr(_errStr, "ERROR - DsFileCopy::enquireForPut", "");
      TaStr::AddStr(_errStr, "  File too old to send: ", _filePath);
      TaStr::AddInt(_errStr, "    File age (secs): ", fileAge);
      TaStr::AddInt(_errStr, "    Max age allowed (secs): ", _maxFileAge);
      return -1;
    }
  }
  
  // add dataDir to url, if it is not already there
  
  DsURL dataUrl = _destUrl;
  if (dataUrl.getFile().size() == 0) {
    if (_debug) {
      TaStr::AddStr(_errStr, "Setting URL file to dir: ", data_dir);
    }
    dataUrl.setFile(data_dir);
  } else {
    if (_debug) {
      TaStr::AddStr(_errStr, "Using URL file for dir: ", dataUrl.getFile());
    }
  }
  if (_debug) {
    TaStr::AddStr(_errStr, "URL: ", dataUrl.getURLStr());
  }
  
  // assemble the message
  
  void *buf = _msg.assembleEnquireForPut(dataUrl,
					 ldata_info,
					 file_name,
					 fileStat.st_mtime,
					 _fileLen,
					 overwrite_age);
  
  // communicate with server, read reply

  if (_enquireCommunicate(buf, _msg.lengthAssembled())) {
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::enquireForPut", "");
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////////
// putAfterEnquire()
//
// Put the file, if doPut is true.
//
// Returns 0 on success, -1 on error
// Error message available via getErrorStr()

int DsFileCopy::putAfterEnquire(ta_compression_method_t
				compression_type /* = TA_COMPRESSION_NONE*/,
				bool remove_after_copy /* = false*/ )
  
{

  _errStr = "";
  if (_debug) {
    _errStr = "DsFileCopy::putAfterEnquire\n";
    TaStr::AddStr(_errStr, "  Error time:", DateTime::str());
    TaStr::AddStr(_errStr, "  File path:", _filePath);
    TaStr::AddInt(_errStr, "  File len:", _fileLen);
  }

  if (!_doPut) {
    if (_debug) {
      TaStr::AddStr(_errStr, "  ", "-->> declined");
    }
    return 0;
  }

  if (_debug) {
    TaStr::AddStr(_errStr, "  ", "-->> putting file");
  }
  
  // stat the file
  
  struct stat fileStat;
  if (ta_stat(_filePath.c_str(), &fileStat)) {
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::putAfterEnquire", "");
    TaStr::AddStr(_errStr, "  Cannot stat file: ", _filePath);
    return -1;
  }
  _fileLen = fileStat.st_size;
  
  // allocate the file buffer
  
  _fileBuf = urealloc(_fileBuf, _fileLen);
  
  // read in file buffer
  
  FILE *fp;
  if ((fp = fopen(_filePath.c_str(), "r")) == NULL) {
    int errNo = errno;
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::putAfterEnquire", "");
    TaStr::AddStr(_errStr, "  Cannot openfile for reading: ", _filePath);
    TaStr::AddStr(_errStr, "  : ", strerror(errNo));
    return -1;
  }
  
  if (ufread(_fileBuf, 1, _fileLen, fp) != _fileLen) {
    int errNo = errno;
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::putAfterEnquire", "");
    TaStr::AddStr(_errStr, "  Cannot read file: ", _filePath);
    TaStr::AddStr(_errStr, "  : ", strerror(errNo));
    fclose(fp);
    return -1;
  }
  
  fclose(fp);

  // assemble the message
  
  void *buf;
  if (compression_type == TA_COMPRESSION_NONE) {
    buf = _msg.assemblePutAfterEnquire(_fileBuf, _fileLen);
  } else {
    ui64 nbytes_compressed;
    void *cbuf = ta_compress(compression_type, _fileBuf, _fileLen,
			     &nbytes_compressed);
    if (_debug){
      ta_compression_debug( cbuf );
    }
    if (cbuf != NULL) {
      buf = _msg.assemblePutAfterEnquire(cbuf, nbytes_compressed);
      ta_compress_free(cbuf);
    } else {
      buf = _msg.assemblePutAfterEnquire(_fileBuf, _fileLen);
    }
  }
  
  // communicate with server, read reply
  
  if (_communicate(buf, _msg.lengthAssembled())) {
    // poke the ServerMgr, in case server needs restarting
    _destUrl.setPort(0);
    DsLocator.resolve(_destUrl, NULL, true, &_errStr);
    return -1;
  }

  // check for error
  
  if (_msg.getError()) {
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::putAfterEnquire", "");
    _errStr += _msg.getErrorStr();
    return -1;
  }

  // remove the file if requested

  if (remove_after_copy) {
    remove(_filePath.c_str());
  }

  return 0;
 

}

/////////////////////////////////////////////////////////////
// putForced()
//
// Put a file without enquiring
//
// Returns 0 on success, -1 on error
// Error message available via getErrorStr()

int DsFileCopy::putForced(const string &data_dir,
			  const DsURL &dest_url,
			  const LdataInfo &ldata_info,
			  const string &file_name,
			  ta_compression_method_t
			  compression_type /* = TA_COMPRESSION_NONE*/,
			  bool remove_after_copy /* = false*/ )
  
{

  _errStr = "";
  if (_debug) {
    _errStr = "DsFileCopy::putForced\n";
    TaStr::AddStr(_errStr, "  ", DateTime::str());
    TaStr::AddStr(_errStr, "  dir: ", data_dir);
    TaStr::AddStr(_errStr, "  dest_url: ", dest_url.getURLStr());
  }
    
  _dataDir = data_dir;
  _destUrl = dest_url;
  string dataDirPath;
  RapDataDir.fillPath(_dataDir, dataDirPath);

  // set the file path

  _filePath = ldata_info.getDataPath();
  
  // stat the file
  
  struct stat fileStat;
  if (ta_stat(_filePath.c_str(), &fileStat)) {
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::putForced", "");
    TaStr::AddStr(_errStr, "  Cannot stat file: ", _filePath);
    return -1;
  }
  _fileLen = fileStat.st_size;
  
  // check file age if appropriate

  if (_maxFileAge > 0) {
    time_t now = time(NULL);
    int fileAge = now - fileStat.st_mtime;
    if (fileAge > _maxFileAge) {
      TaStr::AddStr(_errStr, "ERROR - DsFileCopy::putForced", "");
      TaStr::AddStr(_errStr, "  File too old to send: ", _filePath);
      TaStr::AddInt(_errStr, "    File age (secs): ", fileAge);
      TaStr::AddInt(_errStr, "    Max age allowed (secs): ", _maxFileAge);
      return -1;
    }
  }
  
  if (_debug) {
    TaStr::AddStr(_errStr, "DsFileCopy::put - putting file", "");
    TaStr::AddStr(_errStr, "  ", DateTime::str());
    TaStr::AddStr(_errStr, "    file path: ", _filePath);
    TaStr::AddInt(_errStr, "    file len: ", _fileLen);
  }
  
  // allocate the file buffer
  
  _fileBuf = urealloc(_fileBuf, _fileLen);
  
  // read in file buffer
  
  FILE *fp;
  if ((fp = fopen(_filePath.c_str(), "r")) == NULL) {
    int errNo = errno;
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::putForced", "");
    TaStr::AddStr(_errStr, "  Cannot openfile for reading: ", _filePath);
    TaStr::AddStr(_errStr, "  : ", strerror(errNo));
    return -1;
  }
  
  if (ufread(_fileBuf, 1, _fileLen, fp) != _fileLen) {
    int errNo = errno;
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::putForced", "");
    TaStr::AddStr(_errStr, "  Cannot read file: ", _filePath);
    TaStr::AddStr(_errStr, "  : ", strerror(errNo));
    fclose(fp);
    return -1;
  }
  
  fclose(fp);

  // add dataDir to url, if it is not already there
  
  DsURL dataUrl = _destUrl;
  if (dataUrl.getFile().size() == 0) {
    if (_debug) {
      TaStr::AddStr(_errStr, "Setting URL file to dir: ", data_dir);
    }
    dataUrl.setFile(data_dir);
  } else {
    if (_debug) {
      TaStr::AddStr(_errStr, "Using URL file for dir: ", dataUrl.getFile());
    }
  }
  if (_debug) {
    TaStr::AddStr(_errStr, "URL: ", dataUrl.getURLStr());
  }
  
  // assemble the message
  
  void *buf;
  if (compression_type == TA_COMPRESSION_NONE) {
    buf = _msg.assemblePutForced(dataUrl, ldata_info, file_name,
				 fileStat.st_mtime, _fileLen,
				 _fileBuf, _fileLen);
  } else {
    ui64 nbytes_compressed;
    void *cbuf = ta_compress(compression_type, _fileBuf, _fileLen,
			     &nbytes_compressed);
    if (_debug){
      ta_compression_debug( cbuf );
    }

    if (cbuf != NULL) {
      buf = _msg.assemblePutForced(dataUrl, ldata_info, file_name,
				   fileStat.st_mtime, _fileLen,
				   cbuf, nbytes_compressed);
      ta_compress_free(cbuf);
    } else {
      buf = _msg.assemblePutForced(dataUrl, ldata_info, file_name,
				   fileStat.st_mtime, _fileLen,
				   _fileBuf, _fileLen);
    }
  }
  
  // communicate with server, read reply
  
  if (_communicate(buf, _msg.lengthAssembled())) {
    return -1;
  }

  // check for error
  
  if (_msg.getError()) {
    TaStr::AddStr(_errStr, "ERROR - DsFileCopy::putForced", "");
    _errStr += _msg.getErrorStr();
    return -1;
  }

  // remove the file if requested

  if (remove_after_copy) {
    remove(_filePath.c_str());
  }

  return 0;

}

///////////////////////////////////////
// sendBufferToSocket()
//
// Sends the buffer to the server directly.
//
// Returns 0 on success, -1 on error
// Error message available via getErrorStr()

int DsFileCopy::sendBufferToSocket(DsURL destURL,
				   void *buf,
				   int bufLen)

{
  
  // Set the URL

  _destUrl = destURL;

  // communicate with server, read reply
  
  if (_communicate(buf, bufLen)) {
    return -1;
  }

  // check for error
  
  if (_msg.getError()) {
    _errStr += _msg.getErrorStr();
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////
// Enquire communicate with server
//
// Returns 0 on success, -1 on error.

int DsFileCopy::_enquireCommunicate(void *buf, const int buflen)

{

  _doPut = false;

  // communicate with server, read reply
  
  if (_communicate(buf, buflen)) {

    _errStr += "ERROR - COMM - DsFileCopy::_enquireCommunicate\n";
    _errStr += "  Error communicating with server.\n";

    // poke the ServerMgr, in case server needs restarting
    _destUrl.setPort(0);
    DsLocator.resolve(_destUrl, NULL, true, &_errStr);

    return -1;

  }

  // check for error
  
  if (_msg.getError()) {
    _errStr += _msg.getErrorStr();
    return -1;
  }

  if (_msg.DoPut()) {
    _doPut = true;
  }

  return 0;

}

/////////////////////////////////////////////
// communicate with server
//
// Returns 0 on success, -1 on error.

int DsFileCopy::_communicate(void *buf,
			     const int buflen)
  
{

  // resolve the URL, not using the ServerMgr at this stage
  
  bool contactServer;
  if (DsLocator.resolve(_destUrl, &contactServer, false, &_errStr, true)) {
    _errStr += "ERROR - COMM - DsFileCopy::_communicate.\n";
    TaStr::AddStr(_errStr, "  Cannot resolve URL: ", _destUrl.getURLStr());
    return -1;
  }

  DsClient client;
  client.setDebug(_debug);
  client.setMergeDebugWithErrStr(true);
  if (_openTimeoutMsecs > 0) {
    client.setOpenTimeoutMsecs(_openTimeoutMsecs);
  }

  if (client.communicateAutoFwd(_destUrl,
                                DsFileCopyMsg::DS_MESSAGE_TYPE_FILECOPY,
				buf, buflen)) {
    _errStr += "ERROR - DsFileCopy::_communicate\n";
    _errStr += client.getErrStr();
    return -1;
  }
  
  // disassemble the reply
  
  if (_debug) {
    TaStr::AddStr(_errStr, "----> DsFileCopy::_communicate()", 
                  " dissasembling reply");
  }
  
  // disassemble the reply
  
  if (_msg.disassemble((void *) client.getReplyBuf(), client.getReplyLen())) {
    _errStr += "ERROR - COMM - DsFileCopy::_communicate\n";
    _errStr += "Invalid reply\n";
    return -1;
  }
  
  return 0;

}

