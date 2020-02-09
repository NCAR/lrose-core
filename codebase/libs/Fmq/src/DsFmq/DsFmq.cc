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
////////////////////////////////////////////////////////////////////////////////
//
// DsFmq.cc - DsFmq class
//
// Provides server-based access for Fmq class
//
// Mike Dixon, RAL, NCAR, Boulder, CO, 80307, USA
//
// Jan 2009
//
////////////////////////////////////////////////////////////////////////////////
                                 
#include <cassert>
#include <toolsa/MsgLog.hh>
#include <toolsa/Socket.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/uusleep.h>
#include <dsserver/DsLocator.hh>
#include <Fmq/DsFmq.hh>
using namespace std;

// initialize static consts

const char* DsFmq::FMQ_PROTOCOL = "fmqp";

// constructor
   
DsFmq::DsFmq() : Fmq()
{

  _isServed = false;
  _socket = NULL;
  _nMessagesPerWrite = 1;

}

// destructor

DsFmq::~DsFmq()
{
  _clearReadQueue();
  _clearWriteQueue();
  closeMsgQueue();
}

/////////////////////////////////////////////////////////////
// The following init functions allow you to set up a Fmq in 
// one of the following 4 modes:
//   CREATE, READ_WRITE, READ_ONLY,
//   BLOCKING_READ_ONLY, BLOCKING_READ_WRITE
// These specialized init functions all call the generic init()
// function. However, they omit the arguments which are not
// necessary for the particular type of init desired.  If any
// of these methods are overridden, the underlying Fmq method
// must be called.
// 
// It is important to note that these apply to the Open()
// methods only, not the read or write.

// The following arguments are used in the init functions:
//
//   fmqPath: URL for local/remote FMQ
//   procName: name of your program, for error logging
//   debug: debug flag, for debug logging
//   openPosition: for read opens, whether to position the
//                 queue at the start or end, or ready to
//                 read last item
//   compress: for writes, do compression or not?
//             Compression method defaults to GZIP.
//             See setCompressionMethod().
//   numSlots: for creates, number of slots in queue.
//   bufSize: for creates, total size of data buffer.
//   msecSleep: for blocking reads, number of milli-seconds
//              to wait while polling.
//   msgLog: optional pointer to a message log. If NULL, a log
//           is created by this object.

// initCreate()
// Create an FMQ, opening the files in mode "w+". Any existing
// queue is overwritten.
// Returns 0 on success, -1 on error
  
int DsFmq::initCreate(const char* fmqURL, 
		      const char* procName, 
		      bool debug /* = false*/,
		      bool compression /* = false*/, 
		      size_t numSlots /* = 1024*/, 
		      size_t bufSize /* = 10000*/,
		      MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqURL, procName, debug, CREATE, END,
	       compression, numSlots, bufSize, -1, msgLog));
}

/////////////////////////////////////////////////////////////
// initReadWrite()
// If FMQ already exists, it is opened in mode "r+".
// If not it is created by opening in mode "w+".
//
// msecSleep is used for any subsequent blocking reads.
// If msecSleep is set to -1, 10 msecs will be used for
//  local disk access and 500 msecs for remote access.
//
// Returns 0 on success, -1 on error
 
int DsFmq::initReadWrite(const char* fmqURL, 
			 const char* procName, 
			 bool debug /* = false*/,
			 openPosition position /* = END*/,
			 bool compression /* = false*/, 
			 size_t numSlots /* = 1024*/, 
			 size_t bufSize /* = 10000*/,
			 int msecSleep /* = -1*/,
			 MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqURL, procName, debug, READ_WRITE, position,
	       compression, numSlots, bufSize, msecSleep, msgLog));
}

/////////////////////////////////////////////////////////////
// initReadOnly()
// Open for reading, in mode "r".
// If no queue exists, returns an error.
//
// msecSleep is used for any subsequent blocking reads.
// If msecSleep is set to -1, 10 msecs will be used for
//  local disk access and 500 msecs for remote access.
//
// Returns 0 on success, -1 on error
  
int DsFmq::initReadOnly(const char* fmqURL, 
		      const char* procName, 
		      bool debug /* = false*/,
		      openPosition position /* = END*/,
		      int msecSleep /* = -1*/,
		      MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqURL, procName, debug, READ_ONLY, position,
	       false, 1024, 10000, msecSleep, msgLog));
}

/////////////////////////////////////////////////////////////
// initReadBlocking()
// If queue exists, opens for reading, in mode "r".
// If the queue does not exist, blocks while waiting for it to be
// created by another process. While waiting, registers with procmap
// if PMU module has been initialized.
//
// msecSleep is used for any blocking reads.
// If msecSleep is set to -1, 10 msecs will be used for
//  local disk access and 500 msecs for remote access.
//
// Returns 0 on success, -1 on error

//
// Note - if the queue does not exist, and we have to wait for
// it, then the openPosition will be set to Fmq::START no
// matter what the input argument is set to, since in this case
// we definitely want to read from the start and not miss anything
// that was written to the queue when it was initialized.
// This subtlety was implemented March 2006. Prior to this, the
// first entry written to a queue may have been missed. Niles Oien.
//

int DsFmq::initReadBlocking(const char* fmqURL, 
			    const char* procName, 
			    bool debug /* = false*/,
			    openPosition position /* = END*/,
			    int msecSleep /* = -1*/,
			    MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqURL, procName, debug, BLOCKING_READ_ONLY, position,
	       false, 1024, 10000, msecSleep, msgLog));
}

/////////////////////////////////////////////////////////////
// initReadWriteBlocking()
// If queue exists, opens for reading, in mode "r+".
// If the queue does not exist, blocks while waiting for it to be
// created by another process. While waiting, registers with procmap
// if PMU module has been initialized.
//
// msecSleep is used for any blocking reads.
// If msecSleep is set to -1, 10 msecs will be used for
//  local disk access and 500 msecs for remote access.
//
// Returns 0 on success, -1 on error
 
//
// See comments above for initReadBlocking() about the setting of
// the open position - they apply to initReadWriteBlocking as well.
//

int DsFmq::initReadWriteBlocking(const char* fmqURL, 
				 const char* procName, 
				 bool debug /* = false*/,
				 openPosition position /* = END*/,
				 int msecSleep /* = -1*/,
				 MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqURL, procName, debug, BLOCKING_READ_WRITE, position,
	       false, 1024, 10000, msecSleep, msgLog));
}

/////////////////////////////////////////////////////////////
// generic init() - allows full control of the init
// Returns 0 on success, -1 on error

int DsFmq::init(const char* fmqURL,
		const char *procName,
		bool debug,
		openMode mode, 
		openPosition position, 
		bool compress,
		size_t numSlots, 
		size_t bufSize,
		int msecSleep,
		MsgLog *msgLog)

{

  _urlStr = fmqURL;
  _progName = procName;
  _debug = debug;
  _openMode = mode;
  _openPosition = position;
  _compress = compress;
  _numSlots = numSlots;
  _bufSize = bufSize;
  _msecSleep = msecSleep;
  _msgLog = msgLog;
  if (_debug) {
    _socketMsg.setDebug();
  }
      
  // resolve the URL
  
  if (_resolveUrl()) {
    _print_error("DsFmq::init",
		 "Cannot resolve URL: %s", _urlStr.c_str());
    return -1;
  }

  if (_debug) {
    cerr << "DsFmq::init, URL: " << _url.getURLStr() << endl;
  }

  _fmqPath =  _url.getFile();

  if (!_isServed) {
    // local instance - use Fmq method instead
    return Fmq::init(_fmqPath.c_str(), procName, debug, mode, position,
		     compress, numSlots, bufSize, msecSleep, msgLog);
  }

  // do the init, depending on open mode

  switch(_openMode) {
    
    case BLOCKING_READ_ONLY: 
    case BLOCKING_READ_WRITE: {
      while( true ) {
	if (_doInit() == 0) {
	  return 0;
	}
	if (_heartbeatFunc != NULL ) {
	  _heartbeatFunc("DsFmq blocking on open");
	}
	umsleep(1000);
      } // while
      break;
    }

    default: {
      if (_doInit() == 0) {
	return 0;
      }
    }

  }
   
  return -1;

}

///////////////////////////////////////////////////////////////////
// Closing the fmq
// returns 0 on success, -1 on failure
  
int DsFmq::closeMsgQueue()
{
  
  if (!_isServed) {
    // local
    Fmq::closeMsgQueue();
    return 0;
  }

  _socketMsg.assembleRequestClose();
  _printDebugLabel("closeMsgQueue");

  // Send the client's request

  if (_socket != NULL) {
    if (_socket->writeMessage(DsFmqMsg::DS_FMQ_MESSAGE,
			      _socketMsg.assembledMsg(),
			      _socketMsg.lengthAssembled())) {
      _print_error("COMM: DsFmq::closeMsgQueue()",
		   "Failed writing close request to server\n"
		   "%s\n",
		   _socket->getErrString().c_str());
    }
    _closeClientSocket();
  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// is queue open?
// returns true if open, false otherwise
  
bool DsFmq::isOpen()
{

  if (!_isServed) {
    // local
    return Fmq::isOpen();
  }
  
  return _socket != NULL; 

}

/////////////////////////////////////////////////////////////////
// setting the compression method - default is GZIP compression
// Returns 0 on success, -1 on error

int DsFmq::setCompressionMethod(ta_compression_method_t method)
{ 

  Fmq::setCompressionMethod(method);
  if (!_isServed) {
    // local
    return 0;
  }
  
  _socketMsg.assembleSetCompressionMethod(Fmq::getCompressMethod());
  _printDebugLabel("setCompressionMethod");
  if (_contactServer(_socketMsg.assembledMsg(),
		     _socketMsg.lengthAssembled())) {
    return  -1;
  }

  // check for error
  if (_checkError()) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Setting the write to block if queue is full.
// Returns 0 on success, -1 on error

int DsFmq::setBlockingWrite()
{ 

  Fmq::setBlockingWrite();
  if (!_isServed) {
    // local
    return 0;
  }

  _socketMsg.assembleSetBlockingWrite();
  _printDebugLabel("setBlockingWrite");
  if (_contactServer(_socketMsg.assembledMsg(),
		     _socketMsg.lengthAssembled())) {
    return  -1;
  }

  // check for error
  if (_checkError()) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Set flag to indicate that there is only a single writer
// so the locking is not necessary
// Returns 0 on success, -1 on error

int DsFmq::setSingleWriter()
{ 

  Fmq::setSingleWriter();
  if (!_isServed) {
    // local
    return 0;
  }

  _socketMsg.assembleSetSingleWriter();
  _printDebugLabel("setSingleWriter");
  if (_contactServer(_socketMsg.assembledMsg(),
		     _socketMsg.lengthAssembled())) {
    return  -1;
  }

  // check for error
  if (_checkError()) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// set data mapper registration - this is off by default
// specify the registration interval in seconds

int DsFmq::setRegisterWithDmap(bool doReg,
			       int regIntervalSecs)
  
{
  
  Fmq::setRegisterWithDmap(doReg, regIntervalSecs);
  if (!_isServed) {
    // local
    return 0;
  }

  _socketMsg.assembleSetRegisterWithDmap(doReg, regIntervalSecs);
  _printDebugLabel("setRegisterWithDmap");
  if (_contactServer(_socketMsg.assembledMsg(),
		     _socketMsg.lengthAssembled())) {
    return  -1;
  }

  // check for error
  if (_checkError()) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// Seek to a position in the queue.
// Options are:
//   SEEK_START: start of queue
//   SEEK_END: end of queue, after last entry
//   SEEK_LAST: ready to read last entry
//   SEEK_BACK: go back by 1 entry

int DsFmq::seek(seekPosition pos)
{ 

  if (!_isServed) {
    // local
    return Fmq::seek(pos);
  }

  _socketMsg.assembleRequestSeek(pos);
  _printDebugLabel("seek");
  if (_contactServer(_socketMsg.assembledMsg(),
		     _socketMsg.lengthAssembled())) {
    return  -1;
  }

  // check for error
  if (_checkError()) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
//  Seek to a given ID.
//  Positions the FMQ so that the next read will be the first
//  slot AFTER the specified ID.
//
//  To read the actual ID, call this function followed by a
//  call to Fmq::seek(Fmq::FMQ_SEEK_BACK).
//
//  Sets read pointers only - no affect on writes.
//
//  Return value:
//    0 on success, -1 on error.

int DsFmq::seekToId(int id)

{

  if (!_isServed) {
    // local
    return Fmq::seekToId(id);
  }

  _socketMsg.assembleRequestSeekToId(id);
  _printDebugLabel("seekToId");
  if (_contactServer(_socketMsg.assembledMsg(),
		     _socketMsg.lengthAssembled())) {
    return  -1;
  }

  // check for error
  if (_checkError()) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
// Read a message from the fmq.
// If type is specified, reads next message of the given type.
// If msecs_sleep is positive, waits up to that number of millisecs
// for a message.
// If msecs_sleep is non-positive, does not wait.
// Sets *gotOne if a message was read.
// Returns 0 on success, -1 on error

int DsFmq::readMsg(bool *gotOne, int type /* = -1*/, int msecs_sleep /* = -1*/)

{ 
  
  if (!_isServed) {
    // local
    return Fmq::readMsg(gotOne, type, msecs_sleep);
  }

  assert(gotOne);
  *gotOne = false;

  // first check if we have a message in the queue
  
  if (_readQueue.size() == 0) {
    
    // out of data, do a new read
    
    _socketMsg.assembleRequestRead(type, msecs_sleep);
    _printDebugLabel("readMsg");
    if (_contactServer(_socketMsg.assembledMsg(),
		       _socketMsg.lengthAssembled())) {
      return -1;
    }

    // check for error
    if (_checkError()) {
      return -1;
    }
    
    // add data to queue, checking message type as applicable
    
    for (int ii = 0; ii < (int) _socketMsg.getMsgInfo().size(); ii++) {
      if (type < 0 || type == _socketMsg.getMsgInfo()[ii].msgType) {
	readData *rdata = new readData();
	rdata->info = _socketMsg.getMsgInfo()[ii];
	rdata->buf.add(_socketMsg.getMsgData()[ii],
		       rdata->info.msgLen);
	_readQueue.push_back(rdata);
      }
    }

  } // if (_readQueue.size() == 0)
  
  // check if we have any data

  if (_readQueue.size() == 0) {
    // no data available yet
    return 0;
  }
  
  // pop entry from the front of the queue
  
  readData *rdata = _readQueue.front();
  _readQueue.pop_front();

  // Load the read message into this object
  
  if (_load_read_msg(rdata->info.msgType,
		     rdata->info.msgSubtype,
		     rdata->info.msgId,
		     rdata->info.msgTime,
		     rdata->buf.getPtr(),
		     rdata->info.msgLen,
		     rdata->info.msgPreCompressed,
		     rdata->info.msgUncompressedLen)) {
    // decompression error
    delete rdata;
    return -1;
  }
  
  *gotOne = true;
  delete rdata;

  return 0;
  
}

////////////////////////////////////////////////////////////////////
// Read a message from the fmq.
// If type is specified, reads next message of the given type.
// Blocks until message is available. While blocking registers with
// procmap if PMU module has been initialized.
// Returns 0 on success, -1 on error

int DsFmq::readMsgBlocking(int type)
{ 

  if (!_isServed) {
    // local
    return Fmq::readMsgBlocking(type);
  }

  int sleepTotalMsecs = 0;

  while(true) {

    // perform a read
    
    bool gotOne = false;
    if (readMsg(&gotOne, type)) {
      return -1;
    }
    if (gotOne) {
      return 0;
    }

    // No message of correct type was read
    // sleep for a bit then heartbeat
    
    if (_msecSleep < 0) {
      umsleep(500);
      sleepTotalMsecs += 500;
    } else if (_msecSleep > 0) {
      umsleep( _msecSleep );
      sleepTotalMsecs += _msecSleep;
    }
    if (_msecBlockingReadTimeout > 0 && 
        sleepTotalMsecs > _msecBlockingReadTimeout) {
      return -1;
    }
    if (_heartbeatFunc != NULL ) {
      _heartbeatFunc("Blocking on read");
    }

  } // while (true)
  
  return 0;

}

//////////////////////////////////////////////////////
// Writes a message to the fmq
// Returns 0 on success, -1 on error
  
int DsFmq::writeMsg(int type, int subType, const void *msg, int msgLen)
{ 

  if (!_isServed) {
    // local
    return Fmq::writeMsg(type, subType, msg, msgLen);
  }

  // assemble write message
  
  if (_nMessagesPerWrite < 2) {

    // unbuffered - write every message
    _socketMsg.assembleRequestWrite(type, subType, msg, msgLen,
				    _compress, _compressMethod);
    
  } else {

    // cached write
    
    addToWriteCache(type, subType, msg, msgLen);
    if ((int) _writeQueue.size() < _nMessagesPerWrite) {
      // not enough messages yet
      return 0;
    }

    // assemble the write message from the queue
    
    _socketMsg.clearAll();
    while (_writeQueue.size() > 0) {
      writeData *wdata = _writeQueue.front();
      _socketMsg.addWriteData(wdata->type,
			      wdata->subType,
			      wdata->buf.getPtr(),
			      wdata->buf.getLen(),
			      wdata->compress,
			      wdata->compressMethod);
      
      delete wdata;
      _writeQueue.pop_front();
    }

    _socketMsg.assembleRequestWrite();

  }

  _printDebugLabel("writeMsg");
  if (_contactServer(_socketMsg.assembledMsg(),
		     _socketMsg.lengthAssembled())) {
    return  -1;
  }
  
  // check for error
  if (_checkError()) {
    return -1;
  }

  return 0;
}

///////////////////////////////////////////////////////
// clear write cache - before adding for later write

void DsFmq::clearWriteCache()
{
  _clearWriteQueue();
}

//////////////////////////////////////////////////////
// Add a message to the write cache,
// in preparation for a later write

void DsFmq::addToWriteCache(int type, int subType,
			    const void *msg, int msgLen)
{ 

  writeData *wdata = new writeData();
  wdata->type = type;
  wdata->subType = subType;
  wdata->msgLen = msgLen;
  wdata->compress = _compress;
  wdata->compressMethod = _compressMethod;
  wdata->buf.add(msg, msgLen);
  _writeQueue.push_back(wdata);

}

//////////////////////////////////////////////////////
// Writes all data in the cache to the fmq.
// For remote writes, this is performed in a single action.
// For local writes, these are written one at a time.
// Returns 0 on success, -1 on error
  
int DsFmq::writeTheCache()
{ 
  
  if (!_isServed) {
    // local
    int iret = 0;
    while (_writeQueue.size() > 0) {
      if (_debug) {
        cerr << "writing cache, size: " << _writeQueue.size() << endl;
      }
      writeData *wdata = _writeQueue.front();
      if (Fmq::writeMsg(wdata->type,
			wdata->subType,
			wdata->buf.getPtr(),
			wdata->buf.getLen())) {
	iret = -1;
      }
      delete wdata;
      _writeQueue.pop_front();
    }
    return iret;
  }

  // assemble the write message from the queue
  
  _socketMsg.clearAll();
  while (_writeQueue.size() > 0) {
    writeData *wdata = _writeQueue.front();
    _socketMsg.addWriteData(wdata->type,
			    wdata->subType,
			    wdata->buf.getPtr(),
			    wdata->buf.getLen(),
			    wdata->compress,
			    wdata->compressMethod);
    
    delete wdata;
    _writeQueue.pop_front();
  }
  _socketMsg.assembleRequestWrite();
  
  _printDebugLabel("writeBuffer");
  if (_contactServer(_socketMsg.assembledMsg(),
		     _socketMsg.lengthAssembled())) {
    return  -1;
  }
  
  // check for error
  if (_checkError()) {
    return -1;
  }

  return 0;
}


/////////////////////////////////////////////////////////////
// resolve the URL
// set _isServer flag if we need to communicate via server
// Returns 0 on success, -1 on error

int DsFmq::_resolveUrl()

{

  // Check for validity on the url
  
  _url.setURLStr(_urlStr);
  if (!_url.isValid()) {
    _print_error("DsFmq::_resolveUrl",
		 "Invalid URL specification: %s", _urlStr.c_str());
    return -1;
  }
  
  // Make sure the host name is user-specified
  // since the fmq file cannot be located via the DsLocator/DataMapper.
  // The client must know a priori on what host the fmq is located.

  string host = _url.getHost();
  if (host.empty()) {
    _print_error("DsFmq::_resolveUrl",
		 "Host name must be provided in the URL.");
    return -1;
  }
  
  // Determine if this is a local or remote request - do not
  // contact the server manager at this stage
  
  if (DsLocator.resolve(_url, &_isServed, false) != 0) {
    _print_error("DsFmq::_resolveUrl",
		 "Cannot resolve URL: %s.", _urlStr.c_str());
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////
// Perform the initialization.
// Returns 0 on success, -1 on error

int DsFmq::_doInit()
  
{
  
  // Open the socket to the server
  
  if (_openClientSocket()) {
    return -1;
  }

  // try doing a with full init - updated server
  
  _socketMsg.assembleRequestInit(_url.getURLStr(), _progName, _debug, _openMode,
				 _openPosition, _compress, _numSlots, _bufSize);
  _printDebugLabel("_doInit - assembleRequestInit");
  if (_contactServer(_socketMsg.assembledMsg(),
		     _socketMsg.lengthAssembled())) {
    return -1;
  }

  // check for error

  if (_checkError()) {
    _closeClientSocket();
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////////////////////
// Utility methods for contacting the server
////////////////////////////////////////////////////////////////////////////////

/////////////////////////
// open socket to server

int DsFmq::_openClientSocket()
{

  // close socket if already open
  
  _closeClientSocket();

  // set the url from the fmq path, in case the URL has been
  // reset by contacting the server manager
  
  _url.setURLStr(_urlStr);
  
  // resolve the URL

  if (_resolveUrl()) {
    _print_error("DsFmq::_openClientSocket",
		 "Cannot resolve URL: %s", _urlStr.c_str());
    return -1;
  }

  // ping server to check it is available. If the server is up on
  // the specified or default port, fine. If not, the server manager
  // will be contacted to start the server.
  // This will resolve the port as required.
  
  if (DsLocator.pingServer(_url)) {
    _print_error("COMM - DsFmq::openClientSocket",
		 "Cannot ping server, host: %s, port: %d",
		 _url.getHost().c_str(), _url.getPort());
    return -1;
  }
  
  // open

  _socket = new Socket;
  if (_socket->open(_url.getHost().c_str(), _url.getPort())) {
    _print_error("COMM - DsFmq::openClientSocket",
		 "Failed to open client socket");
    _closeClientSocket();
    return -1;
  }
  
  return 0;

}

/////////////////////////
// close socket

void DsFmq::_closeClientSocket()
{
  
  if (_socket != NULL) {
    _socket->close();
    delete _socket;
    _socket = NULL;
  }

}

/////////////////////////
// send request to client

int DsFmq::_contactServer(void *buffer, const size_t bufLen)
{

  // Send the client's request
  
  if (_socket->writeMessage(DsFmqMsg::DS_FMQ_MESSAGE, buffer, bufLen)) {
    _print_error("COMM: DsFmq::contactServer()",
		 "Failed writing request to client socket\n"
		 "%s\n",
		 _socket->getErrString().c_str());
    _closeClientSocket();
    return -1;
  }
  
  // Read the server's reply

  if (_socket->readMessage()) {
    _print_error("COMM: DsFmq::contactServer()",
		 "Failed reading reply from client socket\n",
		 "%s\n",
		 _socket->getErrString().c_str());
    _closeClientSocket();
    return -1;
  }
  
  // Disassemble the reply

  if (_socketMsg.disassemble(_socket->getData(), _socket->getNumBytes(), *this)) {
    _print_error("COMM: DsFmq::contactServer()",
		 "Cannot disassemble reply");
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// check socket status, reconnect as appropriate

int DsFmq::_checkClientSocket()

{
  
  // Make sure the last request did not result in an error state

  if (_socket != NULL) {
    int socketState = _socket->getState();
    if (!(socketState & SockUtil::STATE_ERROR)) {
      return 0;
    }
  }

  // close the existing socket
  
  _closeClientSocket();

  // redo the init - this reopens the socket

  if (_doInit()) {
    _print_error("DsFmq::_verifyClientSocket",
		 "Failed to re-initialize fmq.");
    return -1;
  }

  _print_info("DsFmq::_verifyClientSocket",
	      "Re-established socket to server at port %d",
	      _url.getPort());
  
  return 0;

}

/////////////////////////////////////////////
// print out details of socket message object

void DsFmq::_printDebugLabel(const string &label)
  
{

  if (_debug) {
    cerr << "==>> DsFmq(" << label << ")" << endl;
  }

}

/////////////////////////////////////////////
// clear the read data

void DsFmq::_clearReadQueue()
  
{
  while (_readQueue.size() > 0) {
    readData *data = _readQueue.front();
    delete data;
    _readQueue.pop_front();
  }
}

/////////////////////////////////////////////
// clear the write data

void DsFmq::_clearWriteQueue()
  
{
  while (_writeQueue.size() > 0) {
    writeData *data = _writeQueue.front();
    delete data;
    _writeQueue.pop_front();
  }
}

/////////////////////////////////////////////
// check for error
// if error, add in error string from client

int DsFmq::_checkError()
  
{
  if (_socketMsg.getError()) {
    _errStr += _socketMsg.getErrStr();
    return -1;
  }
  return 0;
}

