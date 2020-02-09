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
// DsFmq.hh - DsFmq class
//
// Provides server-based access for Fmq class
//
// Mike Dixon, RAL, NCAR, Boulder, CO, 80307, USA
//
// Jan 2009
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _DS_FMQ_INC_
#define _DS_FMQ_INC_

#include <deque>
#include <Fmq/Fmq.hh>
#include <Fmq/DsFmqMsg.hh>
#include <didss/DsURL.hh>
#include <toolsa/Socket.hh>
using namespace std;

// Forward class declarations

class MsgLog;

class DsFmq : public Fmq
{

public:                  

  // constructor
  // note: you must call one of the init() functions
  // before the object is ready to be used.

  DsFmq();                       

  virtual ~DsFmq();

  // The following init functions allow you to set up a DsFmq in 
  // one of the following 4 modes:
  //   CREATE, READ_WRITE, READ_ONLY,
  //   BLOCKING_READ_ONLY, BLOCKING_READ_WRITE
  // These specialized init functions all call the generic init()
  // function. However, they omit the arguments which are not
  // necessary for the particular type of init desired.  If any
  // of these methods are overridden, the underlying DsFmq method
  // must be called.
  // 
  // It is important to note that these apply to the Open()
  // methods only, not the read or write.

  // The following arguments are used in the init functions:
  //
  //   fmqURL: URL for local/remote FMQ
  //   procName: name of your program, for error logging
  //   debug: debug flag, for debug logging
  //   openPosition: for read opens, whether to position the
  //                 queue at the start or end, or ready to
  //                 read last item
  //   compression: for writes, do compression or not?
  //                Compression method defaults to GZIP.
  //                See setCompressionMethod().
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
  
  virtual int initCreate(const char* fmqURL, 
			  const char* procName, 
			  bool debug = false,
			  bool compression = false, 
			  size_t numSlots = 1024, 
			  size_t bufSize = 10000,
			  MsgLog *msgLog = NULL);

  // initReadWrite()
  // If FMQ already exists, it is opened in mode "r+".
  // If not it is created by opening in mode "w+".
  //
  // msecSleep is used for any subsequent blocking reads.
  // If msecSleep is set to -1, 10 msecs will be used for
  //  local disk access and 500 msecs for remote access.
  //
  // Returns 0 on success, -1 on error
 
  virtual int initReadWrite(const char* fmqURL, 
			     const char* procName, 
			     bool debug = false,
			     openPosition position = END,
			     bool compression = false, 
			     size_t numSlots = 1024, 
			     size_t bufSize = 10000,
			     int msecSleep = -1,
			     MsgLog *msgLog = NULL);

  // initReadOnly()
  // Open for reading, in mode "r".
  // If no queue exists, returns an error.
  //
  // msecSleep is used for any subsequent blocking reads.
  // If msecSleep is set to -1, 10 msecs will be used for
  //  local disk access and 500 msecs for remote access.
  //
  // Returns 0 on success, -1 on error
  
  virtual int initReadOnly(const char* fmqURL, 
			    const char* procName, 
			    bool debug = false,
			    openPosition position = END,
			    int msecSleep = -1,
			    MsgLog *msgLog = NULL);

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
  // it, then the openPosition will be set to DsFmq::START no
  // matter what the input argument is set to, since in this case
  // we definitely want to read from the start and not miss anything
  // that was written to the queue when it was initialized.
  // This subtlety was implemented March 2006. Prior to this, the
  // first entry written to a queue may have been missed. Niles Oien.
  //
  virtual int initReadBlocking(const char* fmqURL, 
				const char* procName, 
				bool debug = false,
				openPosition position = END,
				int msecSleep = -1,
				MsgLog *msgLog = NULL);

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
  virtual int initReadWriteBlocking(const char* fmqURL, 
				     const char* procName, 
				     bool debug = false,
				     openPosition position = END,
				     int msecSleep = -1,
				     MsgLog *msgLog = NULL);

  // generic init() - allows full control of the init
  // Returns 0 on success, -1 on error

  virtual int init(const char* fmqURL, 
		    const char* procName, 
		    bool debug = false,
		    openMode mode = READ_WRITE, 
		    openPosition position = END,
		    bool compression = false, 
		    size_t numSlots = 1024, 
		    size_t bufSize = 10000,
		    int msecSleep = -1,
		    MsgLog *msgLog = NULL);

  // Closing the queue
  // returns 0 on success, -1 on failure
  
  virtual int closeMsgQueue();
  
  // setting the compression method - default is GZIP compression
  // Returns 0 on success, -1 on error

  virtual int setCompressionMethod(ta_compression_method_t method);
  
  // Setting the write to block if queue is full.
  // Returns 0 on success, -1 on error

  virtual int setBlockingWrite();
 
  // Set flag to indicate that there is only a single writer
  // so the locking is not necessary
  // Returns 0 on success, -1 on error

  virtual int setSingleWriter();
 
  // set the number of messages to buffer up per write
  // this allows for efficiency in write to a server
  // this defaults to 1

  void setNMessagesPerWrite(int n) { _nMessagesPerWrite = n; }
  
  // set data mapper registration - this is off by default
  // specify the registration interval in seconds
  
  virtual int setRegisterWithDmap(bool doReg,
				  int regIntervalSecs);

  // Seek to a position in the queue.
  // Options are:
  //   SEEK_START: start of queue
  //   SEEK_END: end of queue, after last entry
  //   SEEK_LAST: ready to read last entry
  //   SEEK_BACK: go back by 1 entry

  virtual int seek(seekPosition position);

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
  
  virtual int seekToId(int id);

  // Read a message from the fmq.
  // If type is specified, reads next message of the given type.
  // If msecs_sleep is positive, waits up to that number of millisecs
  // for a message.
  // If msecs_sleep is non-positive, does not wait.
  // Sets *gotOne if a message was read.
  // Returns 0 on success, -1 on error

  virtual int readMsg(bool *gotOne, int type = -1, int msecs_sleep = -1);

  // Read a message from the fmq.
  // If type is specified, reads next message of the given type.
  // Blocks until message is available. While blocking registers with
  // procmap if PMU module has been initialized.
  // Returns 0 on success, -1 on error

  virtual int readMsgBlocking(int type = -1);

  // Writes a message to the fmq
  // Returns 0 on success, -1 on error
  
  virtual int writeMsg(int type, int subType=0, const void *msg=NULL, int msgLen=0);

  ////////////////////////
  // using the write cache

  // clear write cache - before adding for later write
  
  void clearWriteCache();

  // Add a message to the write cache,
  // in preparation for a later write
  
  void addToWriteCache(int type, int subType, const void *msg, int msgLen);

  // get the number of messages in the write cache
  
  int getWriteCacheSize() { return (int) _writeQueue.size(); }

  // Writes all data in the cache to the fmq.
  // For remote writes, this is performed in a single action.
  // For local writes, these are written one at a time.
  // Returns 0 on success, -1 on error
  
  int writeTheCache();
  
  // is queue open?
  
  virtual bool isOpen();

  // Get URL info

  inline const string getUrlStr() const { return _urlStr; }
  const DsURL &getURL() const { return _url; }

  // limits on reads from the server
  // We set a max number of messages and max number of bytes
  // per read from the server.
  // If these limits are exceeded, the client will read the remaining
  // data in a subsequent read

  static const int MAX_READ_NMESSAGES = 100;
  static const int MAX_READ_NBYTES = 1000000;

  // eof (-1) is a predefined DsMessage type
  // If you are developing a DsFmq subclass,
  // Do not define a behavior for message type -1 
  // Consider it reserved or you may get unexpected results
  // from applications who read your fmq.
  //
  static const int multMessageType = 888999;
  static const int multMessagePart = 777888;
  static const int eof = -1;
  static const char* FMQ_PROTOCOL;

protected:

  // url

  string _urlStr;
  DsURL _url;

  // flag to show that data will be retrieved from a server

  bool _isServed;
  
  // socket

  DsFmqMsg _socketMsg;
  Socket *_socket;

  // local read queue
  // we read multiple messages at a time from the server
  // and then the client steps through them one at a time

  class readData {
  public:
    DsFmqMsg::msgInfo_t info;
    MemBuf buf;
  };
  deque<readData *> _readQueue;

  // buffer up messages for writing
  // this allows for efficiency in write to a server

  class writeData {
  public:
    int type;
    int subType;
    int msgLen;
    bool compress;
    ta_compression_method_t compressMethod;
    MemBuf buf;
  };
  deque<writeData *> _writeQueue;

  int _nMessagesPerWrite;
  
private:

  // functions

  int _resolveUrl();
  int _doInit();
  int _doFullInit();
  int _doLegacyInit();
  
  int _openClientSocket();
  void _closeClientSocket();
  int _checkClientSocket();
  
  int _contactServer(void *buffer, const size_t bufLen);
  
  void _printDebugLabel(const string &label);

  void _clearReadQueue();
  void _clearWriteQueue();

  int _checkError();
 
};

#endif

