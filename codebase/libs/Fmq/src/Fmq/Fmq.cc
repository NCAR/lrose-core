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
// Fmq.hh
// Fmq class - File Message Queue
//
// Mike Dixon, RAL, NCAR, Boulder, CO, 80307, USA
//
// Jan 2009
//
////////////////////////////////////////////////////////////////////////////////
                                 
#include <cassert>
#include <cstdarg>
#include <dataport/bigend.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/uusleep.h>
#include <dsserver/DmapAccess.hh>
#include <Fmq/Fmq.hh>
#include <climits>

using namespace std;

#define BOOL_STR(a) ((a)? "true" : "false")

// constructor
   
Fmq::Fmq()   
{

  _debug = false;
  _openMode = READ_WRITE;
  _openPosition = END;
  _compress = false;
  _numSlots = 0;
  _bufSize = 0;
  _msecSleep = -1;
  _msecBlockingReadTimeout = -1;

  _dev = NULL;
  _msgLog = NULL;

  _createdOnOpen = false;
  _server = false;
  _compressMethod = TA_COMPRESSION_GZIP;

  MEM_zero(_stat);
  MEM_zero(_slot);

  _slots = NULL;
  _nslotsAlloc = 0;

  _entry = NULL;
  _nEntryAlloc = 0;

  _lastIdRead = -1;
  _lastSlotRead = -1; 
  _blockingWrite = false;
  _singleWriter = false;
  _lastSlotWritten = 0;

  setHeartbeat(PMU_auto_register);

  _registerWithDmap = false;
  _dmapRegIntervalSecs = 5;
  _dmapPrevRegTime = 0;

}

// destructor

Fmq::~Fmq()
{

  if (_dev) {
    closeMsgQueue();
  }

  _free_slots();
  _free_entry();

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
  
int Fmq::initCreate(const char* fmqPath, 
		    const char* procName, 
		    bool debug /* = false*/,
		    bool compress /* = false*/, 
		    size_t numSlots /* = 1024*/, 
		    size_t bufSize /* = 10000*/,
		    MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqPath, procName, debug, CREATE, END,
	       compress, numSlots, bufSize, -1, msgLog));
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
 
int Fmq::initReadWrite(const char* fmqPath, 
		       const char* procName, 
		       bool debug /* = false*/,
		       openPosition position /* = END*/,
		       bool compress /* = false*/, 
		       size_t numSlots /* = 1024*/, 
		       size_t bufSize /* = 10000*/,
		       int msecSleep /* = -1*/,
		       MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqPath, procName, debug, READ_WRITE, position,
	       compress, numSlots, bufSize, msecSleep, msgLog));
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
  
int Fmq::initReadOnly(const char* fmqPath, 
		      const char* procName, 
		      bool debug /* = false*/,
		      openPosition position /* = END*/,
		      int msecSleep /* = -1*/,
		      MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqPath, procName, debug, READ_ONLY, position,
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

int
Fmq::initReadBlocking(const char* fmqPath, 
		      const char* procName, 
		      bool debug /* = false*/,
		      openPosition position /* = END*/,
		      int msecSleep /* = -1*/,
		      MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqPath, procName, debug, BLOCKING_READ_ONLY, position,
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

int
Fmq::initReadWriteBlocking(const char* fmqPath, 
			   const char* procName, 
			   bool debug /* = false*/,
			   openPosition position /* = END*/,
			   int msecSleep /* = -1*/,
			   MsgLog *msgLog /* = NULL */)
  
{
  return (init(fmqPath, procName, debug, BLOCKING_READ_WRITE, position,
	       false, 1024, 10000, msecSleep, msgLog));
}

/////////////////////////////////////////////////////////////
// generic init() - allows full control of the init
// Returns 0 on success, -1 on error

int
Fmq::init(const char* fmqPath,
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

  //
  // Limit the value of bufSize.
  //
  // As of June 2011, using values larger than LONG_MAX-1 for
  // bufSize is problematic. The problems arise because this value is
  // represented as being of types size_t, int and (in the _stat
  // structure) si32. Large values get mangled in type conversion.
  // The resulting behavior can be *very* hard to debug.
  //
  // The long term solution is probably going to be to represent
  // the value as a consistent type, probably si64 (and possibly
  // do the same for the number of slots, too). For now, though,
  // we apply this upper limit.
  //
  // Niles Oien working with Mike Dixon, June 2011.
  //
  if (bufSize > LONG_MAX-1){
    cerr << "WARNING : Upper limit of FMQ buffer size exceeded, using "
         << LONG_MAX-1 << " instead of " << bufSize << endl;
    bufSize = LONG_MAX-1;
  }

  // initialize 

  _fmqPath = fmqPath;
  _progName = procName;
  _debug = debug;
  _openMode = mode;
  _openPosition = position;
  _compress = compress;
  _numSlots = numSlots;
  _bufSize = bufSize;
  _msecSleep = msecSleep;
  _msgLog = msgLog;

  _init_status(_numSlots, _bufSize);
  initErrStr();

  // create the device for low-level read/write
  
  if (_dev) {
    delete _dev;
    _dev = NULL;
  }

  _print_info("init", "initializing FMQ, path: %s", _fmqPath.c_str());
  if (isShmemQueue(fmqPath)) {
    _print_info("init", "Using shared memory for message queue");
    _dev = new FmqDeviceShmem(fmqPath, numSlots, bufSize, _heartbeatFunc);
  } else {
    _dev = new FmqDeviceFile(fmqPath, numSlots, bufSize, _heartbeatFunc);
  }
  
  // open the queue

  _createdOnOpen = false;
  if (_open(mode, numSlots, bufSize)) {
    delete _dev;
    _dev = NULL;
    return -1;
  }
  
  // Set the starting postion. If the queue did not exist previously,
  // then we force the position at the start.
  
  if ((_openPosition == START) || _createdOnOpen) {
    if (seek(FMQ_SEEK_START)) {
      return -1;
    }
  } else {
    if (seek(FMQ_SEEK_END)) {
      return -1;
    }
  }

  return 0;

}

// Closing the fmq
// returns 0 on success, -1 on failure
  
int
Fmq::closeMsgQueue()
{

  if (_dev) {
    delete _dev;
    _dev = NULL;
  }
  return 0;

}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// set methods
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// setting the compression method
// only use GZIP compression, others are deprecated
// Returns 0 on success, -1 on error

int Fmq::setCompressionMethod(ta_compression_method_t method)
{
  if (method == TA_COMPRESSION_NONE) {
    _compressMethod = TA_COMPRESSION_NONE;
  } else {
    _compressMethod = TA_COMPRESSION_GZIP;
  }
  return 0;
}

/////////////////////////////////////////////////////////
// Set the write to block when queue is full.
// Returns 0 on success, -1 on error

int Fmq::setBlockingWrite()
{
  _blockingWrite = true;
  return 0;
}

/////////////////////////////////////////////////////////
// Setting flag to indicate that there is only a single writer
// so the locking is not necessary
// Returns 0 on success, -1 on error

int Fmq::setSingleWriter()
{
  _singleWriter = true;
  return 0;
}

/////////////////////////////////////////////////////////////////
// Set the heartbeat function. This function will be called while
// the 'blocking' open and read calls are waiting.
//
// If set_heartbeat is not called, no heartbeat function will be
// called.

void Fmq::setHeartbeat(TA_heartbeat_t heartbeat_func)
  
{
  _heartbeatFunc = heartbeat_func;
}

// Sets the flag which indicates the object is being
// used by a server.

void Fmq::setServer()
{
  _server = true;
}

/////////////////////////////////////////////////////////
// set data mapper registration - this is off by default
// specify the registration interval in seconds

int Fmq::setRegisterWithDmap(bool doReg,
			     int regIntervalSecs)

{
  _registerWithDmap = doReg;
  _dmapRegIntervalSecs = regIntervalSecs;
  _dmapPrevRegTime = 0;
  return 0;
}
 
////////////////////////////////////////////////////////////////
// Seek to a position in the queue.
// Options are:
//   SEEK_START: start of queue
//   SEEK_END: end of queue, after last entry
//   SEEK_LAST: ready to read last entry
//   SEEK_BACK: go back by 1 entry

int Fmq::seek(Fmq::seekPosition position)
{

  initErrStr();

  if (!_dev) {
    cerr << "ERROR - FmqQueue::seek" << endl;
    cerr << "Queue not open, must call init functions: " << _fmqPath << endl;
    return -1;
  }

  int status = 0;
  
  switch(position) {
    case FMQ_SEEK_START:
      status = _seek_start();
      break;
    case FMQ_SEEK_END:
      status = _seek_end();
      break;
    case FMQ_SEEK_LAST:
      status = _seek_last();
      break;
    case FMQ_SEEK_BACK:
      status = _seek_back();
      break;
  }

  return(status);

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

int Fmq::seekToId(int id)

{

  // get slot of requested ID

  int slot;
  if (_find_slot_for_id (id, &slot)) {
    _print_error("_seek_to_id",
		 "Cannot find slot for id: %d", _stat.youngest_id);
    return -1;
  }

  _lastIdRead = id;
  _lastSlotRead = slot;

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

int Fmq::readMsg(bool *gotOne, int type /*= -1*/, int msecs_sleep /* = -1 */)

{

  initErrStr();

  if (!_dev) {
    cerr << "ERROR - Fmq::readMsg" << endl;
    cerr << "  Fmq path: " << _fmqPath << endl;
    cerr << "  Queue not open, must call init functions" << endl;
    return -1;
  }
  
  assert(gotOne);
  
  int status;
  int readFlag;
  if (msecs_sleep > 0) {
    status = _read_non_blocking(&readFlag, type, msecs_sleep);
  } else {
    status = _read(&readFlag, type);
  }
  *gotOne = readFlag ? true : false;
  
  return(status);

}

////////////////////////////////////////////////////////////////////
// Read a message from the fmq.
// If type is specified, reads next message of the given type.
// Blocks until message is available. While blocking registers with
// procmap if PMU module has been initialized.
// Returns 0 on success, -1 on error

int Fmq::readMsgBlocking(int type)
{

  initErrStr();

  if (!_dev) {
    cerr << "ERROR - Fmq::readMsgBlocking" << endl;
    cerr << "  Fmq path: " << _fmqPath << endl;
    cerr << "  Queue not open, must call init functions" << endl;
    return -1;
  }

  int iret = _read_blocking(_msecSleep, type);
  if (iret == 0) {
    _doRegisterWithDmap();
  }
  return iret;

}

//////////////////////////////////////////////////////
// Writes a message to the fmq
// Returns 0 on success, -1 on error
  
int Fmq::writeMsg(int type, int subType, 
		  const void *msg, int msgLen)
{

  initErrStr();

  if (!_dev) {
    cerr << "ERROR - Fmq::writeMsg" << endl;
    cerr << "  Fmq path: " << _fmqPath << endl;
    cerr << "  Queue not open, must call init functions" << endl;
    return -1;
  }

  int iret = _write((void *) msg, msgLen, type, subType);
  if (iret == 0) {
    _doRegisterWithDmap();
  }
  return iret;

}

/////////////////////////////////////////////////////////
// Writing precompressed msg
// Returns 0 on success, -1 on error

int Fmq::writeMsgPreCompressed(int type, int subType, 
			       const void *msg, int msgLen,
			       int uncompressedLen)
{

  initErrStr();

  if (!_dev) {
    cerr << "ERROR - Fmq::writeMsgPreCompressed" << endl;
    cerr << "  Fmq path: " << _fmqPath << endl;
    cerr << "  Queue not open, must call init functions" << endl;
    return -1;
  }

  return (_write_precompressed((void *) msg, msgLen,
			       type, subType,
			       uncompressedLen));
}

//////////////////////////////////////////////////////////////
// Is this a shared-memory-based queue?
// inspect fmqPath to find out.
// If the fileName is of the form "shmem_xxxxxxxx", then this
// is a shared memory queue with a key of xxxxxxxx.
// If so, compute the key and return 0.
// Otherwise return -1.

int Fmq::getShmemKey(const string &fmqPath, key_t &key)

{

  Path path(fmqPath);
  string fileName = path.getFile();

  // sanity check

  if(fileName.size() < 1) {
    return -1;
  }

  // check for 'shmem' substring
  
  if (fileName.find("shmem") == string::npos) {
    return -1;
  }

  // tokenize, breaking on '_'

  vector<string> toks;
  TaStr::tokenize(fileName, "_", toks);
  if (toks.size() < 1) {
    return -1;
  }

  // try to get keyval from last token

  int keyval;
  string lastTok = toks[toks.size()-1];
  if (sscanf(lastTok.c_str(), "%d", &keyval) == 1) {
    key = keyval;
    return 0;
  } else {
    return -1;
  }

}

bool Fmq::isShmemQueue(const string &fmqPath)

{
  key_t key;
  if (getShmemKey(fmqPath, key) == 0) {
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////
// PRINTING
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Print out the status struct.

void Fmq::print_stat(FILE *out) const
  
{

  fprintf(out, "\n");
  fprintf(out, "FMQ STATUS - %s\n", _fmqPath.c_str());
  fprintf(out, "==========\n");
  fprintf(out, "\n");
  
  fprintf(out, "  magic_cookie: %d\n", _stat.magic_cookie);
  fprintf(out, "  youngest_id: %d\n", _stat.youngest_id);
  fprintf(out, "  youngest_slot: %d\n", _stat.youngest_slot);
  fprintf(out, "  oldest_slot: %d\n", _stat.oldest_slot);
  fprintf(out, "  nslots: %d\n", _stat.nslots);
  fprintf(out, "  buf_size: %d\n", _stat.buf_size);
  fprintf(out, "  begin_insert: %d\n", _stat.begin_insert);
  fprintf(out, "  end_insert: %d\n", _stat.end_insert);
  fprintf(out, "  begin_append: %d\n", _stat.begin_append);
  fprintf(out, "  append_mode: %s\n", BOOL_STR(_stat.append_mode));
  fprintf(out, "  time_written: %s\n", utimstr(_stat.time_written));
  fprintf(out, "  blocking_write: %d\n", _stat.blocking_write);
  fprintf(out, "  last_id_read: %d\n", _stat.last_id_read);
  fprintf(out, "  checksum: %d\n", _stat.checksum);

  fprintf(out, "\n");
  fprintf(out, "\n");

}

////////////////////////////////////////////////////////////
// Print out the latest slot read.

void Fmq::print_slot_read(FILE *out) const
     
{
  _print_slot(out, "LATEST SLOT READ", _lastSlotRead);
}

////////////////////////////////////////////////////////////
// Print out the latest slot written.

void Fmq::print_slot_written( FILE *out) const
     
{
  _print_slot(out, "LATEST SLOT WRITTEN", _stat.youngest_slot);
}

///////////////////////////////////////////////////////////
//  Debugging printout - will work for invalid
//  FMQ files.
//
//  Print out the status and slot structs.
//  Opens and closes the files.
//  Returns 0 on success, -1 on failure.


int Fmq::print_debug(const char *fmqPath,
		     const char *progName,
		     FILE *out)
     
{

  Fmq qq;
  if (qq.initReadOnly(fmqPath, progName)) {
    cerr << "ERROR - Fmq::print_debug" << endl;
    cerr << "  Cannot  initialize read-only" << endl;
    cerr << "  fmqPath: " << fmqPath << endl;
    cerr << "  app: " << progName << endl;
    return -1;
  }

  
  fprintf(out, "\n");
  fprintf(out, "FMQ STATUS\n");
  fprintf(out, "==========\n");
  fprintf(out, "\n");

  /*
   * check if valid
   */
  
  if (qq._check_buffer_sizes()) {
    fprintf(out, "FMQ %s is not valid\n", fmqPath);
    if (qq._open_device("r")) {
      qq._print_error("fmq_print_debug",
		      "Cannot open files\n");
      return -1;
    }
  } else {
    fprintf(out, "FMQ %s is valid\n\n", fmqPath);
  }
  
  /*
   * read in status struct
   */
  
  if (qq._read_stat()) {
    qq._print_error("fmq_print_debug",
		     "Cannot read in stat struct\n");
    return -1;
  }
  
  /*
   * read in slots
   */
  
  if (qq._alloc_slots(qq._stat.nslots)) {
    return -1;
  }
  
  if (qq._read_slots()) {
    qq._print_error("fmq_print_debug ",
		     "Cannot read in slot structs array\n");
    return -1;
  }
  
  /*
   * print status
   */

  qq.print_stat(out);

  /*
   * print slots
   */
  
  fprintf(out,
	  "*** slot_num, active, id, time, msg_len, "
	  "stored_len, offset, type, subtype ***\n");

  Fmq::q_slot_t *slot = qq._slots;
  for (int islot = 0; islot < qq._stat.nslots; islot++, slot++) {
    qq._basic_print_slot(islot, slot, out);
  }
  
  fprintf(out, "\n");
  fprintf(out, "\n");
  
  return 0;

}

////////////////////////////////////////////////////////////
// INITIALIZATION
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// initialize status
//

void Fmq::_init_status(int nslots, int buf_size)

{
  
  MEM_zero(_stat);
  _stat.magic_cookie = Q_MAGIC_STAT;
  _stat.youngest_id = -1;
  _stat.youngest_slot = -1;
  _stat.oldest_slot = -1;
  _stat.nslots = nslots;
  _stat.buf_size = buf_size;
  _stat.begin_insert = 0;
  _stat.end_insert = 0;
  _stat.begin_append = 0;
  _stat.append_mode = 1;

}

////////////////////////////////////////////////////////////
// MEMORY MANAGEMENT
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Fmq::_alloc_slots()
//
// Memory alloc for slots.
//
///

int Fmq::_alloc_slots(int nslots)
     
{

  if (_nslotsAlloc < nslots) {

    if (_slots != NULL) {
      delete[] _slots;
    }
    _slots = new q_slot_t[nslots];

    _nslotsAlloc = nslots;
    
  } // if (_nslotsAlloc < nslots)

  return 0;

}

////////////////////////////////////////////////////////////
// Fmq::_free_slots()
//
// Memory free for slots.
//
///

void Fmq::_free_slots()
     
{

  if (_slots != NULL) {

    delete[] _slots;
    _slots = NULL;
    _nslotsAlloc = 0;

  }

}

////////////////////////////////////////////////////////////
// free_oldest_slot()
//
// Free up the oldest slot, and increase the free space in
// the insert region.
///

int Fmq::_free_oldest_slot()
  
{

  int oldest_slot = _stat.oldest_slot;
  q_slot_t *oldest_ptr = _slots + oldest_slot;

  // update oldest slot

  _read_slot(oldest_slot);

  // consistency check

  if (oldest_ptr->offset != _stat.end_insert) {

    fprintf(stderr, "===============================\n");
    fprintf(stderr,
	    "free_oldest_slot: "
	    "Offset mismatch: end_insert %d, "
	    "oldset_slot offset %d\n",
	    _stat.end_insert, oldest_ptr->offset);
    fprintf(stderr, "\n");
    print_stat(stderr);
    fprintf(stderr, "\n");
    _pretty_print_slot(oldest_slot, oldest_ptr, stderr);
    fprintf(stderr, "===============================\n");
    
    // re-initialize the queue
    _clear();
    return -1;

  }

  _stat.end_insert += oldest_ptr->stored_len;

  // Check whether the insert region has merged with the append
  // region. If so, merge both into the append region, and
  // set the insert region to length 0 at the start of the file.

  if (_stat.end_insert >= _stat.begin_append) {

    _stat.begin_append = _stat.begin_insert;
    _stat.begin_insert = 0;
    _stat.end_insert = 0;
    _stat.append_mode = 1;
    
  }

  _stat.oldest_slot =
    _next_slot(_stat.oldest_slot);

  // Zero out the slot and save it to file.
  
  MEM_zero(*oldest_ptr);
  if (_write_slot(oldest_slot)) {
    _print_error("free_oldest_slot",
		 "Cannot write slot %d\n", oldest_slot);
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Fmq::_alloc_entry
//
// Memory alloc for msg entry
//
///

void Fmq::_alloc_entry(int msg_len)
     
{

  int nbytes_padded;
  int nbytes_needed;

  // compute padded length, to keep the total length aligned to
  // 32-bit words
  
  nbytes_padded = (((msg_len - 1) / sizeof(si32)) + 1) * sizeof(si32);
  nbytes_needed = nbytes_padded + Q_NBYTES_EXTRA;
  
  if (_nEntryAlloc == 0) {
    _entry = new ui08[nbytes_needed];
    _nEntryAlloc = nbytes_needed;
  } else if (nbytes_needed > _nEntryAlloc) {
    delete[] _entry;
    _entry = new ui08[nbytes_needed];
    _nEntryAlloc = nbytes_needed;
  }
  memset(_entry, 0, nbytes_needed);
  
}

////////////////////
// Fmq::_free_entry
//
// Memory free for msg entry.
//
///

void Fmq::_free_entry()
     
{

  if (_entry != NULL) {
    delete[] _entry;
    _entry = NULL;
    _nEntryAlloc = 0;
  }
  
}

////////////////////////////////////////////////////////////
// OPEN
////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// open()
//
// This is the open method used when the call is truly local - i.e.
// called directly by the application itself. The blocking opens
// called here will register with procmap.

int
  Fmq::_open(Fmq::openMode mode, size_t numSlots, size_t bufSize)
{
  
  int status = 0;

  switch(mode) {
    case Fmq::CREATE:
      status = _open_create(numSlots, bufSize);
      break;
    case Fmq::READ_WRITE:
      status = _open_rdwr(numSlots, bufSize);
      break;
    case Fmq::READ_ONLY:
      status = _open_rdonly();
      break;
    case Fmq::BLOCKING_READ_ONLY: 
      status = _open_blocking(-1);
      break;
    case Fmq::BLOCKING_READ_WRITE: 
      status = _open_blocking_rdwr(-1);
      break;
  }

  if (status) {
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////
//  _open_create()
//
//  Creates the FMQ by opening in mode "w+".
//
//  Return value:
//    0 on success, -1 on failure
///

int Fmq::_open_create(int nslots, int buf_size)
     
{

  // open, creating the files write/read
    
  if (_open_device("w+")) {
    return -1;
  }

  // initialize files
  
  _lock_device();
  _prepare_for_writing(nslots, buf_size);
  _unlock_device();

  _createdOnOpen = true;

  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_open_rdwr()
//
//  If FMQ exists and is valid, opens in mode "r+".
//  If FMQ does not exist, creates it by opening in mode "w+"
//  Otherwise, returns error.
//
//  Return value:
//    0 on success, -1 on failure
///

int Fmq::_open_rdwr(int nslots, int buf_size)
     
{

  // if files do not exist, do create instead

  if (_check_device_exists()) {
    return (_open_create(nslots, buf_size));
  }

  // open the files

  if (_open_device("r+")) {
    return -1;
  }

  // read in status and slots

  if (_read_stat()) {
    return -1;
  }
  if (_read_slots()) {
    return -1;
  }

  // check the fmq files, and clear as required
  
  if (_check_and_clear()) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_open_rdonly()
//
//  If FMQ exists and is valid, opens the files mode "r".
//  Otherwise, returns error.
//
//  Return value:
//    0 on success, -1 on failure
///

int Fmq::_open_rdonly ()

{

  if (_check_device_exists()) {
    _print_error("_open_rdonly",
		 "FMQ does not yet exist: %s\n", _fmqPath.c_str());
    if (_msecSleep <= 1000) {
      umsleep(1000);
    } else {
      umsleep(_msecSleep);
    }
    return -1;
  }

  // open the files

  if (_open_device("r")) {
    return -1;
  }

  // read in status and slots

  if (_read_stat()) {
    return -1;
  }
  if (_read_slots()) {
    return -1;
  }

  // set last_slot_read to 1 less than oldest_slot

  _lastSlotRead = _prev_slot(_stat.oldest_slot);
  
  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_open_rdwr_nocreate()
//
//  If FMQ exists and is valid, opens the files mode "r+".
//  Otherwise, returns error.
//
//  Used for reading in blocking operations.
//
//  Return value:
//    0 on success, -1 on failure
///

int Fmq::_open_rdwr_nocreate()

{

  if (_check_device_exists()) {
    return -1;
  }

  // open the files

  if (_open_device("r+")) {
    return -1;
  }

  // read in status and slots

  if (_read_stat()) {
    return -1;
  }
  if (_read_slots()) {
    return -1;
  }

  // set last_slot_read to 1 less than oldest_slot

  _lastSlotRead = _prev_slot(_stat.oldest_slot);
  
  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_open_blocking()
//
//  If valid FMQ does not exist, waits until it does,
//  and then opens opens the files mode "r", i.e. rdonly.
//
//  While waiting, calls heartbeat function if required.
//
//  Return value:
//    0 on success, -1 on failure
///

int Fmq::_open_blocking(int msecs_sleep)

{
  
  while (true) {
    
    if (_open_rdonly ()) {

      if (_heartbeatFunc != NULL) {
	_heartbeatFunc("In FMQ::_open_blocking()");
      }

      // no FMQ yet - sleep as requested
      
      if (msecs_sleep < 0) {
	umsleep(1000);
      } else if (msecs_sleep > 0) {
	umsleep(msecs_sleep);
      }

    } else {

      return 0;

    }

  } // while (forever)
  
  // never reach this return - only for compiler warnings
  
  return -1;

}

////////////////////////////////////////////////////////////
/////////////////////////////
//  Fmq::_open_blocking_rdwr()
//
//  If valid FMQ does not exist, waits until it does,
//  and then opens opens the files mode "r+", i.e. read-write.
//
//  While waiting, calls heartbeat function if required.
//
//  Return value:
//    0 on success, -1 on failure
///

int Fmq::_open_blocking_rdwr(int msecs_sleep)

{
  
  while (true) {
    
    if (_open_rdwr_nocreate()) {
      
      if (_heartbeatFunc != NULL) {
	_heartbeatFunc("In FMQ::_open_blocking()");
      }

      // no FMQ yet - sleep 1 sec

      if (msecs_sleep < 0) {
	umsleep(1000);
      } else if (msecs_sleep > 0) {
	umsleep(msecs_sleep);
      }

    } else {

      return 0;

    }

  } // while (forever)
  
  // never reach this return - only for compiler warnings
  
  return -1;

}

/////////////////////////////////////////////////////////////////
// FILE CLOSE
/////////////////////////////////////////////////////////////////     

////////////////////////////////////////////////////////////
//  Fmq::_close()
//
//  Close the FMQ status and buffer files.
//  Free up memory associated with the files.
//
//  Return value:
//    0 on success, -1 on failure
///

void Fmq::_close()

{

  _free_slots();
  _free_entry();
  _close_device();

}

////////////////////////////////////////////////////////////
//  Fmq::_clear()
//
//  Clear the the FMQ status and buffer files.
//  Free up memory associated with the files.
//
//  Return value:
//    0 on success, -1 on failure
///

int Fmq::_clear()

{

  _free_slots();
  _free_entry();
  
  _lock_device();
  if (_prepare_for_writing(_stat.nslots, _stat.buf_size)) {
    _unlock_device();
    return -1;
  }
  _unlock_device();

  return 0;

}

/////////////////////////////////////////////////////////////////
// SEEK
/////////////////////////////////////////////////////////////////     

////////////////////////////////////////////////////////////
//  Fmq::_seek_end()
//
//  Seek to the end of the FMQ.
//
//  Sets read pointers only - no affect on writes.
//
//  Only messages written after this call
//  will be available for read.
//
//  Return value:
//    0 on success, -1 on error.
///

int Fmq::_seek_end ()

{

  int last_slot_read;

  if (_find_slot_for_id(_stat.youngest_id,
			&last_slot_read)) {
    _print_error("_seek_end",
		 "Cannot find slot for id: %d", _stat.youngest_id);
    return -1;
  }

  _lastIdRead = _stat.youngest_id;
  _lastSlotRead = last_slot_read;

  // in the case of blocking writes, update the last_id_read
  // in the file status block

  if (_stat.blocking_write) {
    if (_update_last_id_read()) {
      return -1;
    }
  }

  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_seek_last()
//
//  This positions for reading the last entry.
//  Seek to the end of the FMQ, minus 1 slot.
//
//  Sets read pointers only - no affect on writes.
//
//  Return value:
//    0 on success, -1 on error.
///

int Fmq::_seek_last ()

{
  
  int last_slot_read;

  if (_find_slot_for_id (_stat.youngest_id,
			 &last_slot_read)) {
    _print_error("_seek_last",
		 "Cannot find slot for id: %d", _stat.youngest_id);
    return -1;
  }

  _lastIdRead = _prev_id(_stat.youngest_id);
  _lastSlotRead = _prev_slot(last_slot_read);

  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_seek_start()
//
//  Seek to the start of the FMQ.
//
//  Sets read pointers only - no affect on writes.
//
//  Set the read pointer to just before the oldest
//  record, so that the entire buffer will be
//  available for reads.
//
//  Return value:
//    0 on success, -1 on error.
///

int Fmq::_seek_start ()

{

  _lastSlotRead = _prev_slot(_stat.oldest_slot);
  _lastIdRead = -1;

  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_seek_back()
//
//  Seek back by 1 entry.
//
//  Sets read pointers only - no affect on writes.
//
//  Return value:
//    0 on success, -1 on error.
///

int Fmq::_seek_back ()

{

  if (_lastIdRead == -1) {
    return 0;
  }

  if (_lastIdRead == 0) {
    _lastIdRead = -1;
    _lastSlotRead = -1;
    return 0;
  }
    
  _lastIdRead = _prev_id(_lastIdRead);
  _lastSlotRead = _prev_slot(_lastSlotRead);

  return 0;

}

/////////////////////////////////////////////////////////////////
// READ
/////////////////////////////////////////////////////////////////     

////////////////////////////////////////////////////////////
// Fmq::_read()
//
// Reads the next message from FMQ.
//
// If type is non-negative, read until the correct message type
// is found. A type of -1 indicates all types are accepted.
//
// Sets msg_read true if msg read, false if not.
//
// Returns 0 on success, -1 on failure.
///

int Fmq::_read(int *msg_read, int type)
     
{

  if (type < 0) {

    return (_read_next(msg_read));

  } else {

    while (_read_next(msg_read) == 0) {
      if (*msg_read == false) {
	return 0;
      } else if (type == _slot.type) {
	return 0;
      }
    }

  }

  return -1;

}

////////////////////////////////////////////////////////////
//  Fmq::_read_blocking()
//
//  This function reads a message from an FMQ - it blocks until
//  a message is received. Registers with procmap while
//  waiting.
//
//   Parameters:
//
//    msecs_sleep - number of millisecs to sleep between reads
//                  while waiting for a message to arrive.
//    If set to -1, default of 10 msecs will be used.
//
//    type - if type is non-negative, read until the correct message
//           type is found.
//           A type of -1 indicates all types are accepted.
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_read_blocking (int msecs_sleep, int type)
     
{

  int msg_read;
  int sleepTotalMsecs = 0;
  if (msecs_sleep < 0) {
    msecs_sleep = 10;
  }

  while (true) {

    if(_read_next(&msg_read)) {
      return -1; // error
    }
    
    if (msg_read) {

      // message read
      if (type < 0) {
	// accept all types
	return 0;
      } else if (type == _slot.type) {
	// type is as requested
	return 0;
      }

      sleepTotalMsecs = 0;

    } else {

      umsleep(msecs_sleep);
      sleepTotalMsecs += msecs_sleep;

      if (_msecBlockingReadTimeout > 0 && 
          sleepTotalMsecs > _msecBlockingReadTimeout) {
        _errStr += "Fmq _read_blocking timed out\n";
        return -1;
      }
    
    } // if (msg_read)
      
    // no message of correct type - send heartbeat
    
    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("In FMQ::_read_blocking()");
    }
    
  } // while (forever)

  return -1; // to satisfy compiler

}


////////////////////////////////////////////////////////////
//  Fmq::_read_non_blocking()
//
//  This function reads a message from an FMQ - waiting 
//  msecs_sleep until a message arrives.
//
//   Parameters:
//
//    msecs_sleep - number of millisecs to sleep between reads
//                  while waiting for a message to arrive.
//    If set to -1, default of 10 msecs will be used.
//
//    type - if type is non-negative, read until the correct message
//           type is found.
//           A type of -1 indicates all types are accepted.
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_read_non_blocking (int *msg_read, int type, int msecs_sleep)
     
{

  assert(msg_read);
  *msg_read = 0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double startTime = tv.tv_sec + (double) tv.tv_usec / 1.0e6;
  double endTime = startTime + msecs_sleep / 1.0e3;
  int count = 0;
  
  while (true) {
    
    if (count > 0) {
      gettimeofday(&tv, NULL);
      double now = tv.tv_sec + (double) tv.tv_usec / 1.0e6;
      if (now > endTime) {
        return 0;
      }
    }
    count++;

    *msg_read = 0;
    if(_read_next(msg_read)) {
      return -1; // error
    }
    
    if (*msg_read) {
      
      // message read
      
      if (type < 0) {

	// accept all types
        
	return 0;
        
      } else if (type == _slot.type) {
        
	// type is as requested
	
	return 0;
	
      }

    } else {
      
      umsleep(10);
      
      if (_heartbeatFunc != NULL) {
        _heartbeatFunc("In FMQ::_read_blocking()");
      }
      
    } // if (msg_read)
    
  } // while (forever)

  return -1; // to satisfy compiler

}


////////////////////////////////////////////////////////////
//  Fmq::_read_stat()
//
//  Reads in stat struct, store in handle.
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_read_stat ()

{

  int ii;
  
  // try 5 times to read the status struct, using the checksum to
  // ensure it is correctly read

  q_stat_t status;

  for (ii = 0; ii < 5; ii++) {

    // seek to start of status file
    
    if (_seek_device(FmqDevice::STAT_IDENT, 0)) {
      _print_error("_read_stat",
		   "Cannot seek to start of status buffer");
      return -1;
    }

    // read in status struct
    
    if (_read_device(FmqDevice::STAT_IDENT, &status, sizeof(q_stat_t))) {
      _print_error("_read_stat",
		   "Cannot read status struct");
      return -1;
    }

    // swap
    
    be_to_stat(&status);

    // copy to handle

    _stat = status;
    
    // checksum check
    
    if (_check_stat_checksum(&status) == 0) {
      return 0;
    }

  } // ii

  //  checksum error
  
  fprintf(stderr,
	  "WARNING - _read_stat, stat checksum error\n"
	  "checksum is %d, should be %d\n",
	  status.checksum,
	  _compute_stat_checksum(&status));
  fprintf(stderr, "  Could not resolve bad checksum, continuing anyway ...\n");
  print_stat(stderr);

  return 0;
  
}

////////////////////////////////////////////////////////////
//  Fmq::_read_slots()
//
//  Reads in slot structs array
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_read_slots ()

{

  int nbytes;

  // seek to start of slots in status file

  if (_seek_device(FmqDevice::STAT_IDENT, sizeof(q_stat_t))) {
    _print_error("_read_slots",
		 "Cannot seek to start of slots in status buffer");
    return -1;
  }
  
  // read in slots, make sure there are enough slots allocated

  _alloc_slots(_stat.nslots);
  
  nbytes = sizeof(q_slot_t) * _stat.nslots;
  if (_read_device(FmqDevice::STAT_IDENT, _slots, nbytes)) {
    _print_error("_read_slots", "Cannot read slots");
    return -1;
  }

  // swap slot byte order

  BE_to_array_32(_slots, _stat.nslots * sizeof(q_slot_t));

  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_read_slot()
//
//  Reads in slot struct.
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_read_slot ( int slot_num)

{

  int offset, ii;
  q_slot_t *slot;

  // Make sure we have a valid slot number

  if (slot_num >= _stat.nslots) {
    _print_error("_read_slot",
		 "Invalid slot number %d, nslots = %d\n"
		 "Make sure writer is not re-creating FMQ repeatedly",
		 slot_num, _stat.nslots);
    print_stat(stderr);
    return -1;
  }
  
  _alloc_slots(_stat.nslots);
  
  // try 5 times to read the slot struct, using the checksum to
  // ensure it is correctly read

  for (ii = 0; ii < 5; ii++) {
    
    // seek to given slot
    
    offset = sizeof(q_stat_t) + slot_num * sizeof(q_slot_t);
    if (_seek_device(FmqDevice::STAT_IDENT, offset)) {
      _print_error("_read_slot",
		   "Cannot seek to slot %d in status file",
		   slot_num);
      return -1;
    }
    slot = _slots + slot_num;

    // read in slot
    
    
    if (_read_device(FmqDevice::STAT_IDENT, slot, sizeof(q_slot_t))) {
      _print_error("_read_slot",
		   "Cannot read slot %d in status file",
		   slot_num);
      return -1;
    }
    
    // swap slot byte order
    
    be_to_slot(slot);

    if (slot->checksum == 0) {
      continue;
    }
    
    // checksum check
    
    if (_check_slot_checksum(slot) == 0) {
      return 0;
    }
    
  } // ii

  // checksum error
  
  fprintf(stderr,
	  "WARNING - _read_slot - slot checksum error\n"
	  "checksum is %d, should be %d\n",
	  slot->checksum, _compute_slot_checksum(slot));
  fprintf(stderr, "  Could not resolve bad checksum, continuing anyway ...\n");
  _basic_print_slot(slot_num, slot, stderr);

  return 0;

}

////////////////////////////////////////////////////////////
//  read_next()
//
//  Reads next message, if available
//
//  Sets msg_read true if msg read, false if not.
//
//  Returns 0 on success, -1 on failure.
///

int Fmq::_read_next(int *msg_read)
     
{
  static int num_calls = 0;
  
  int next_slot, slot_read;

  num_calls++;
  
  *msg_read = false;

  if (_read_stat()) {
    return -1;
  }

  // special case - no messages written yet
  
  if (_stat.youngest_id == -1) {
    _lastIdRead = -1;
    _lastSlotRead = -1;
    return 0;
  }

  // check if a new message has been written since the
  // last read

  if (_lastIdRead == _stat.youngest_id) {
    return 0;
  }
  
  // get msg for next logical slot

  next_slot = _next_slot(_lastSlotRead);

  if (_read_msg_for_slot(next_slot)) {

    // no valid message for the next logical slot, so the buffer has
    // probably overflowed. The best option is to move ahead to
    // the youngest slot and start reading from there

    if (_read_msg_for_slot(_stat.youngest_slot)) {
      return -1;
    }

    slot_read = _stat.youngest_slot;

  } else {

    slot_read = next_slot;
    
  }

  // In blocked mode, if we have skipped data tell the user about it.

  if (_stat.blocking_write && _slot.id >= 0) {
    int prev_id = _prev_id(_slot.id);
    if (prev_id != _lastIdRead) {
      fprintf(stderr, "!!!!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!\n"
	      "Data was skipped even though the fmq is in blocking mode.\n"
	      "You should decrease the number of slots, possibly to be as low as 10.\n"
	      "Only increase buffer size if problems persist after minimizing the number of slots.\n"
	      "You must ensure that the slots wrap BEFORE the\n"
	      "buffer wraps. Almost always the number of slots needs to\n"
	      "be decreased to achieve this, rather than increasing the buffer size.\n");
      fprintf(stderr, "read_next - Fmq: %s\n", _fmqPath.c_str());
      print_stat(stderr);
      fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
  }
  
  _lastSlotRead = slot_read;
  _lastIdRead = _slot.id;
  *msg_read = true;

  // in the case of blocking writes, update the last_id_read
  // in the file status block

  if (_stat.blocking_write) {
    if (_update_last_id_read()) {
      return -1;
    }
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Fmq::_read_msg_for_slot()
//
// Reads the message for the given slot.
//
// Returns 0 on success, -1 on failure.
//
///

int Fmq::_read_msg_for_slot( int slot_num)

{

  int prev_id;
  q_slot_t *slot;

  if (_read_slot(slot_num)) {
    return -1;
  }

  slot = _slots + slot_num;
  prev_id = _prev_id(slot->id);

  if (slot->active &&
      (_lastIdRead == -1 || prev_id == _lastIdRead)) {

    // read in message
    
    if (_read_msg(slot_num)) {
      // failed
      _lastSlotRead = slot_num;
      _lastIdRead = slot->id;
      return -1;
    }

  } else {

    _lastSlotRead = slot_num;
    _lastIdRead = -1;
    return -1;

  }

  return 0;

}

////////////////////////////////////////////////////////////
//  read_msg()
//
//  Reads in msg for a given slot.
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_read_msg (int slot_num)

{

  q_slot_t *slot;
  int id_posn;
  ui64 nfull;
  si32 magic_cookie;
  si32 slot_num_check;
  si32 id_check;
  si32 *iptr;
  void *compressed_msg;
  
  // Make sure we have a valid slot number.

  if (slot_num >= _stat.nslots) {
    _print_error("read_msg",
		 "Invalid slot number %d, nslots = %d\n"
		 "Make sure writer is not re-creating FMQ repeatedly",
		 slot_num, _stat.nslots);
    return -1;
  }
  
  slot = _slots + slot_num;

  // seek to start of message

  if (_seek_device(FmqDevice::BUF_IDENT, slot->offset)) {
    _print_error("_read_msg",
		 "Cannot seek to msg in buf file.");
    return -1;
  }

  // alloc space for entry

  _alloc_entry(slot->stored_len);
     
  // read in message

  if (_read_device(FmqDevice::BUF_IDENT, _entry, slot->stored_len)) {
    _print_error("read_msg",
		 "Cannot read message from buf file, "
		 "slot, len, offset: %d, %d, %d",
		 slot_num, slot->stored_len, slot->offset);
    return -1;
  }
  
  // check the magic cookie and slot number fields.
  //
  // Entry is comprised of following:
  //      si32         si32                             si32
  // --------------------------------------------------------
  // | magic cookie |  slot_num  | -- message -- | pad |  id  |
  // --------------------------------------------------------
  //
  // Pad is for si32 alignment.

  // check magic cookie
  
  iptr = (si32 *) _entry;

  magic_cookie = BE_to_si32(iptr[0]);

  if (magic_cookie != Q_MAGIC_BUF) {
    _print_error("_read_msg",
		 "Magic cookie not correct in message area, "
		 "slot_num, len, offset, magic_cookie, desired magic_cookie: "
		 "%d, %d, %d, %d, %d",
		 slot_num, slot->stored_len, slot->offset,
		 magic_cookie, Q_MAGIC_BUF);
    return -1;
  }

  // check slot_num

  slot_num_check = BE_to_si32(iptr[1]);

  if (slot_num_check != slot_num) {
    _print_error("_read_msg",
		 "Start check slot_num not correct in message area, "
		 "len, offset: %d, %d, "
		 "expected slot_num %d, "
		 "slot_num in file %d",
		 slot->stored_len, slot->offset,
		 slot_num, slot_num_check);
    // COMMENTING OUT ERROR RETURN - CHECK ON THIS - MIKE
    // return -1;
  }

  // check id

  id_posn = (slot->stored_len / sizeof(si32)) - 1;
  id_check = BE_to_si32(iptr[id_posn]);
  
  if (id_check != slot->id) {
    _print_error("_read_msg",
		 "End check id not correct in message area, "
		 "len, offset: %d, %d, "
		 "expected id %d, "
		 "id in file %d",
		 slot->stored_len, slot->offset,
		 slot->id, id_check);
    return -1;
  }

  // set msg pointer, uncompressing as necessary
  // If this is a server, decompression is not done becuse this is
  // done at the client end.

  if (slot->compress) {
    if (_server) {
      // leave data compressed for server to pass on
      _msgBuf.free();
      int msg_len = slot->stored_len - 2 * sizeof(si32);
      if (_add_read_msg(iptr + 2, msg_len)) {
        cerr << "ERROR - _read_msg" << endl;
        return -1;
      }
    } else {
      // uncompress the data
      _msgBuf.free();
      compressed_msg = (void *) (iptr + 2);
      void *umsg = ta_decompress(compressed_msg, &nfull);
      if (umsg == NULL || (int) nfull != slot->msg_len) {
	_print_error("read_msg",
		     "Error on decompression, expected %d bytes, "
		     "got %d bytes",
		     (int) slot->msg_len, (int) nfull);
	if (umsg != NULL) {
	  ta_compress_free(umsg);
	}
	return -1;
      }
      int msg_len = slot->msg_len;
      if (_add_read_msg(umsg, msg_len)) {
        cerr << "ERROR - _read_msg" << endl;
        return -1;
      }
      ta_compress_free(umsg);
    }
  } else {
    // data not compressed
    _msgBuf.free();
    int msg_len = slot->msg_len;
    if (_add_read_msg(iptr + 2, msg_len)) {
      cerr << "ERROR - _read_msg" << endl;
      return -1;
    }
  }

  // set len and latest slot read.

  _slot = _slots[slot_num];

  return 0;

}

////////////////////////////////////////////////////////////
//  load_read_msg
//
//  Given a read message, load up the object
//  This is used by the FmqMgrClient class, which
//  reads from a socket and then loads up the handle.
//

int Fmq::_load_read_msg(int msg_type,
			int msg_subtype,
			int msg_id,
			time_t msg_time,
			void *msg,
			int stored_len,
			int compressed,
			int uncompressed_len)
     
{

  ui64 nfull;

  _slot.type    = msg_type;
  _slot.subtype = msg_subtype;
  _slot.id      = msg_id;
  _slot.time    = msg_time;

  if (compressed) {

    void *dmsg = ta_decompress(msg, &nfull);
    
    if (dmsg == NULL || (int) nfull != uncompressed_len) {
      _print_error("load_read_msg",
		   "Error on decompression, expected %d bytes, "
		   "got %d bytes",
		   (int) uncompressed_len, (int) nfull);
      if (dmsg != NULL) {
	ta_compress_free(dmsg);
      }
      return -1;
    }

    _slot.msg_len = uncompressed_len;
    _slot.stored_len = stored_len;
    _msgBuf.free();
    _msgBuf.add(dmsg, uncompressed_len);
    ta_compress_free(dmsg);
    
  } else {

    _slot.msg_len = stored_len;
    _slot.stored_len = stored_len;
    _msgBuf.free();
    _msgBuf.add(msg, stored_len);

  }
  
  return 0;

}

////////////////////////////////////////////////////////////
//  Add a message to the msg buffer on read
//  Checks for valid length
//  Return value:
//    0 on success, -1 on failure

int Fmq::_add_read_msg(void *msg, int msg_size)
{
  if (msg_size < 0 || msg_size > _bufSize) {
    cerr << "ERROR - Fmq::_add_to_msg" << endl;
    cerr << "  fmq path: " << _fmqPath << endl;
    cerr << "  bad message size on read: " << msg_size << endl;
    cerr << "  max message size: " << _bufSize << endl;
    return -1;
  }
  _msgBuf.add(msg, msg_size);
  return 0;
}

////////////////////////////////////////////////////////////
// WRITE
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
//  prepare for writing
//
//  Return value:
//    0 on success, -1 on failure
///

int Fmq::_prepare_for_writing(int nslots, int buf_size)
     

{

  si08 last_byte;
  int islot;
  si32 magic_cookie;

  // allocate status and slots memory

  if (_alloc_slots(nslots)) {
    return -1;
  }

  _init_status(nslots, buf_size);

  _lastIdRead = -1;
  _lastSlotRead = -1;
  _lastSlotWritten = -1;

  // initialize slots
  
  memset(_slots, 0, nslots * sizeof(q_slot_t));

  // seek to start of buf file

  if (_seek_device(FmqDevice::BUF_IDENT, 0)) {
    _print_error("init_files",
		 "Cannot seek to start of buf file");
    return -1;
  }

  // write magic cookie

  magic_cookie = BE_from_si32(Q_MAGIC_BUF);
  if (_write_device(FmqDevice::BUF_IDENT, &magic_cookie, sizeof(si32))) {
    _print_error("init_files",
		 "Cannot write magic cookie at start of buf file");
    return -1;
  }
  
  // seek to 1 byte from end of file
  
  if (_seek_device(FmqDevice::BUF_IDENT, buf_size - 1)) {
    _print_error("init_files",
		 "Cannot seek to end of buf file");
    return -1;
  }

  // write byte at end of file to set file size

  last_byte = -1;
  if (_write_device(FmqDevice::BUF_IDENT, &last_byte, sizeof(si08))) {
    _print_error("init_files",
		 "Cannot write byte at end of buf file");
    return -1;
  }

  // write out slots and status

  for (islot = 0; islot < nslots; islot++) {
    if (_write_slot(islot)) {
      _print_error("init_files",
		   "Cannot write slot struct %d", islot);
      return -1;
    }
  } // islot

  if (_write_stat()) {
    _print_error("init_files",
		 "Cannot write stat struct");
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_write()
//
//  This function writes a message to an FMQ.
//
//  Provides file locking layer
//
//  Return value:
//    0 on success, -1 on error.
//

int Fmq::_write(void *msg, int msg_len,
		int msg_type, int msg_subtype)
  
{

  int iret;
  
  if (_lock_device() != 0) {
    _print_error("_write", "Error locking for read/write");
    return -1;
  }
  
  iret = _write_msg(msg, msg_len, msg_type, msg_subtype,
		    false, msg_len);
  _unlock_device();
  
  return (iret);
  
}

////////////////////////////////////////////////////////////
//  Fmq::_write_precompressed()
//
//  This function writes a pre-compressed message to an FMQ.
//
//  Provides file locking layer
//
//  Return value:
//    0 on success, -1 on error.
///

int Fmq::_write_precompressed(void *msg, int msg_len,
			      int msg_type, int msg_subtype,
			      int uncompressed_len)
  
{

  int iret;

  if (_lock_device() != 0) {
    _print_error("_write_precompressed",
		 "Error locking for read/write");
    return -1;
  }
  
  iret = _write_msg(msg, msg_len, msg_type, msg_subtype,
		    true, uncompressed_len);
  _unlock_device();
  
  return (iret);

}

////////////////////////////////////////////////////////////
//  Fmq::_write_stat()
//
//  Writes out stat struct
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_write_stat()

{

  q_stat_t stat;

  // make local copy of stat struct
  // set byte order to BigEnd
  
  stat = _stat;
  stat.time_written = time(NULL);
  _add_stat_checksum(&stat);
  be_from_stat(&stat);

  // seek to start of stat file
  
  if (_seek_device(FmqDevice::STAT_IDENT, 0)) {
    return -1;
  }
  
  // write
  
  if (_write_device(FmqDevice::STAT_IDENT, &stat, sizeof(q_stat_t))) {
    _print_error("_write_stat", "Cannot write stat info.");
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_write_slot()
//
//  Writes out slot struct to stat file
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_write_slot ( int slot_num)

{

  int offset;
  q_slot_t slot;

  // Make sure we have a valid slot number

  if (slot_num >= _stat.nslots) {
    _print_error("_write_slot",
		 "Invalid slot number %d, nslots = %d",
		 slot_num, _stat.nslots);
    return -1;
  }
  
  if (slot_num >= _nslotsAlloc) {
    _print_error("_write_slot",
		 "Too few slots allocated.  "
		 "allocated = %d, needed = %d",
		 _nslotsAlloc, slot_num);
    return -1;
  }
  
  // make local copy of slot struct
  // set byte order to BigEnd
  
  slot = _slots[slot_num];
  _add_slot_checksum(&slot);
  be_from_slot(&slot);

  // seek to slot

  offset = sizeof(q_stat_t) + slot_num * sizeof(q_slot_t);
  if (_seek_device(FmqDevice::STAT_IDENT, offset)) {
    _print_error("_write_slot",
		 "Cannot seek to slot posn, offset %d.", offset);
    return -1;
  }

  // write slot

  if (_write_device(FmqDevice::STAT_IDENT, &slot, sizeof(q_slot_t))) {
    _print_error("_write_slot",
		 "Cannot write slot info, slot num %d.",
		 (int) slot_num);
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////
//  _write_msg()
//
//  This function writes a message to an FMQ.
//
//  Parameters:
//    handle - FMQ handle
//    msg - message array
//    msg_len - message length in bytes
//    msg_type - user-defined and used message type
//    msg_subtype - user-defined and used message subtype
//
//  Note: the type and subtype are not used by the FMQ module but
//        are passed through so the reading routine can determine
//        something about the message from the header.
//
//  Return value:
//    0 on success, -1 on error.
///

int Fmq::_write_msg(void *msg, int msg_len, 
		    int msg_type, int msg_subtype,
		    int pre_compressed, int uncompressed_len)

{

  int stored_len;
  int write_slot;
  int write_id;
  int offset;
  int nbytes_padded;
  int iret;
  int do_compress;
  ui64 clen;
  void *cmsg;
  q_slot_t *slot;
  
  // read in status struct 
  
  if (_read_stat()) {
    return -1;
  }

  // compute the slot position for writing
  
  write_slot = _next_slot (_stat.youngest_slot);
  write_id = _next_id(_stat.youngest_id);
  
  // in blocking write operation, wait if we have caught up
  
  if (_blockingWrite) {
  
    int overwrite_id = write_id - _stat.nslots;
    if (overwrite_id < 0) {
      overwrite_id += Q_MAX_ID;
    }

    while (overwrite_id == _stat.last_id_read) {

      _unlock_device();
      if (_heartbeatFunc != NULL) {
	_heartbeatFunc("_write - blocked ...");
      }
      umsleep(200);
      _lock_device();
      if (_read_stat()) {
	return -1;
      }

    } // while

    _stat.blocking_write = true;

  } // if (_blockingWrite)

  // if the write_slot is the same as the oldest slot, the slot
  // array is cycling faster than the buffer. Therefore, free
  // up the oldest slot

  if (write_slot == _stat.oldest_slot) {
    if (_free_oldest_slot()) {
      return -1;
    }
  }

  // compress if required
  
  if (_compress && !pre_compressed && (msg != NULL)) {
    do_compress = true;
  } else {
    do_compress = false;
  }

  if (do_compress) {
    if ((cmsg = ta_compress(_compressMethod,
			    msg, msg_len, &clen)) == NULL) {
      _print_error("_write_msg",
		   "Message compression failed.");
      return -1;
    }
  } else {
    clen = msg_len;
    cmsg = msg;
  }

  // compute padded length, to keep the total length aligned to
  // 32-bit words
  
  nbytes_padded = (((clen - 1) / sizeof(si32)) + 1) * sizeof(si32);
  stored_len = nbytes_padded + Q_NBYTES_EXTRA;
  
  // check that message will fit in buffer
  
  if (stored_len > _stat.buf_size) {
    _print_error("_write_msg",
		 "Message size %d bytes too large for FMQ\n"
		 "Max msg len %d",
		 clen, _stat.buf_size - Q_NBYTES_EXTRA);
    if (do_compress) {
      ta_compress_free(cmsg);
    }
    return -1;
  }

  // make space for message

  while (1) {
    int avail = _space_avail(stored_len);
    if (avail < 0) {
      return -1;
    }
    if (avail) {
      break;
    }
  }
  
  if (_alloc_slots(_stat.nslots)) {
    return -1;
  }

  // write the message

  if (_stat.append_mode) {
    offset = _stat.begin_append;
  } else {
    offset = _stat.begin_insert;
  }

  // #define DEBUG_PRINT
#ifdef DEBUG_PRINT
  fprintf(stderr, "bi, ei, ba, am, ln, ys, os, off: %9d %9d %9d %2d %8d %5d %5d %9d\n",
          _stat.begin_insert,
          _stat.end_insert,
          _stat.begin_append,
          _stat.append_mode,
          stored_len,
          _stat.youngest_slot,
          _stat.oldest_slot,
          offset);
#endif
  
  iret = _write_msg_to_slot(write_slot, write_id, cmsg,
			    clen, stored_len, offset);
  if (do_compress) {
    ta_compress_free(cmsg);
  }
  if (iret) {
    return -1;
  }
  
  // load up slot info and write out
  
  slot = _slots + write_slot;
  slot->active = true;
  slot->id = write_id;
  slot->time = time(NULL);
  slot->msg_len = uncompressed_len;
  slot->stored_len = stored_len;
  slot->offset = offset;
  slot->type = msg_type;
  slot->subtype = msg_subtype;

  if (msg != NULL) {
    slot->compress = _compress;
  } else {
    slot->compress = false;
  }

  if (_write_slot(write_slot)) {
    slot->active = false;
    return -1;
  }

  _slot = _slots[write_slot];

  // update status data and write out

  _stat.youngest_slot = write_slot;
  if (_stat.oldest_slot == -1) {
    _stat.oldest_slot = write_slot;
  }
  if (_stat.append_mode) {
    _stat.begin_append += stored_len;
  } else {
    _stat.begin_insert += stored_len;
  }
  _stat.youngest_id = write_id;
  
  if (_write_stat()) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
//  write_msg()
//
//  Writes out msg for a given slot.
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_write_msg_to_slot(int write_slot, int write_id,
			    void *msg, int msg_len, int stored_len, int offset)
     
{

  int id_posn;
  si32 *iptr;
  si32 magic_cookie;
  si32 slot_num;
  si32 id;

  // seek to start of message
  
  if (_seek_device(FmqDevice::BUF_IDENT, offset)) {
    _print_error("write_msg",
		 "Cannot seek to msg in buf file.");
    return -1;
  }

  // alloc space for entry
  
  _alloc_entry(stored_len);

  // set values in entry
     
  magic_cookie = BE_from_si32(Q_MAGIC_BUF);
  slot_num = BE_from_si32(write_slot);
  id = BE_from_si32(write_id);

  iptr = (si32 *) _entry;
  iptr[0] = magic_cookie;
  iptr[1] = slot_num;
  id_posn = (stored_len / sizeof(si32)) - 1;
  iptr[id_posn] = id;

  // copy the message in

  memcpy(iptr + 2, msg, msg_len);

  // write out entry
  
  if (_write_device(FmqDevice::BUF_IDENT, _entry, stored_len)) {
    _print_error("write_msg",
		 "Cannot write message to buf file, "
		 "slot_num, len, offset: %d, %d, %d",
		 write_slot, msg_len, offset);
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////////////////
// DEVICE-LEVEL methods

// open at the device level
// returns 0 on success, -1 on failure

int Fmq::_open_device(const char *mode)
{

  if (_dev == NULL) {
    _print_error("_open_device", "Device object NULL");
    return -1;
  }
  
  if (_dev->do_open(mode)) {
    _print_error("_open_device", _dev->getErrStr().c_str());
    return -1;
  }

  _bufSize = _dev->get_size(FmqDevice::BUF_IDENT);

  return 0;

}

// close at the device level

void Fmq::_close_device()
{
  if (_dev == NULL) {
    _print_error("_close_device", "Device object NULL");
    return;
  }
  
  _dev->do_close();

}

// seek at the device level
// returns 0 on success, -1 on failure

int Fmq::_seek_device(FmqDevice::ident_t id, off_t offset)
{
  if (_dev == NULL) {
    _print_error("_seek_device", "Device object NULL");
    cerr << "  Device object NULL" << endl;
    return 0;
  }
  
  off_t result = _dev->do_seek(id, offset);
  if (result != offset) {
    _print_error("_seek_device", _dev->getErrStr().c_str());
    return -1;
  }
  return 0;

}

// read at the device level
// returns 0 on success, -1 on failure

int Fmq::_read_device(FmqDevice::ident_t id, void *mess, size_t len)
{
  if (_dev == NULL) {
    _print_error("_read_device", "Device object NULL");
    return -1;
  }
  size_t result = _dev->do_read(id, mess, len);
  if (result != len) {
    _print_error("_read_device", _dev->getErrStr().c_str());
    return -1;
  }
  return 0;

}

// write at the device level
// returns 0 on success, -1 on failure

int Fmq::_write_device(FmqDevice::ident_t id, const void *mess, size_t len)
{

  if (_dev == NULL) {
    _print_error("_write_device", "Device object NULL");
    return -1;
  }

  size_t result = _dev->do_write(id, mess, len);
  if (result != len) {
    _print_error("_write_device", _dev->getErrStr().c_str());
    return -1;
  }
  return 0;
  
}

// lock at the device level
// returns 0 on success, -1 on failure

int Fmq::_lock_device()
{

  if (_singleWriter) {
    // locking not required
    return 0;
  }

  if (_dev == NULL) {
    _print_error("_lock_device", "Device object NULL");
    return -1;
  }

  int result = _dev->lock();
  if (result) {
    _print_error("_lock_device", _dev->getErrStr().c_str());
    return -1;
  }
  return 0;

}

int Fmq::_unlock_device()
{

  if (_singleWriter) {
    // locking not actie
    return 0;
  }

  if (_dev == NULL) {
    _print_error("_unlock_device", "Device object NULL");
    return -1;
  }
  
  return _dev->unlock();

}

// checking at the device level
// returns 0 on success, -1 on failure

int Fmq::_check_device_exists()
{
  
  if (_dev == NULL) {
    _print_error("_device_exists", "Device object NULL");
    return -1;
  }
  
  return _dev->check_exists();

}

// are the buffers the correct size?
// returns 0 on success, -1 on failure

int Fmq::_check_buffer_sizes()
{

  if (_dev == NULL) {
    _print_error("_check_device_sizes", "Device object NULL");
    return -1;
  }
  
  int nslots = _stat.nslots;
  size_t expectedStatSize = sizeof(q_stat_t) + nslots * sizeof(q_slot_t);
  size_t expectedBufSize = _stat.buf_size;

  if (_dev->check_size(FmqDevice::STAT_IDENT, expectedStatSize)) {
    _errStr += _dev->getErrStr();
    return -1;
  }

  if (_dev->check_size(FmqDevice::BUF_IDENT, expectedBufSize)) {
    _errStr += _dev->getErrStr();
    return -1;
  }

  return 0;

}

// update last id read at the device level

int Fmq::_update_last_id_read()
{
  
  if (_dev == NULL) {
    _print_error("_update_last_id_read", "Device object NULL");
    return -1;
  }
  
  return _dev->update_last_id_read(_lastIdRead);

}

////////////////////////////////////////////////////////////
//  Fmq::_check()
//
//  Checks that the FMQ files are valid.
//  Assumes files exist and are open.
//
//  Return value:
//    0 on success, -1 if file is corrupted
///

int Fmq::_check()

{

  int islot;
  int slot_num;
  q_slot_t *slot;

  // are the buffers the correct size?

  if (_check_buffer_sizes()) {
    _print_error("_check", "Buffer sizes incorrect");
    return -1;
  }

  // check that the youngest id has a slot
  
  if (_find_slot_for_id (_stat.youngest_id,
			 &slot_num)) {
    _print_error("_check",
		 "Cannot read in slot for youngest_id: %d",
		 _stat.youngest_id);
    return -1;
  }
  
  // check all msgs

  if (_slots == NULL) {
    _print_error("_check",
		 "_slots array not yet allocated");
    return -1;
  }

  slot = _slots;
  for (islot = 0; islot < _stat.nslots; islot++, slot++) {

    // Check whether this slot should be active or not.
    
    int should_be_active = _slot_in_active_region(islot);
    
    if (slot->active && !should_be_active) {
      _print_error("_check",
		   "Slot %d is active, should be inactive",
		   islot);
      return -1;
    }
    
    if (!slot->active && should_be_active) {
      _print_error("_check",
		   "Slot %d is inactive, should be active",
		   islot);
      return -1;
    }
    
    if (slot->active && should_be_active) {
      if (_read_msg_for_slot(islot)) {
        _print_error("_check",
		     "Cannot read slot num: %d",
		     islot);
        return -1;
      }
    }
    
  }

  return 0;

}

////////////////////////////////////////////////////////////
//  Fmq::_check_and_clear()
//
//  Checks that the FMQ files are not corrupted.
//  Assumes the files are open.
//  If corrupted, clears the buffers ready for fresh start.
//
//  Return value:
//    0 on success, -1 on failure
//

int Fmq::_check_and_clear()

{

  // check the file for corruption

  if (_check()) {

    // clear the queue buffer and files
    // ready to start clean
    
    if (_clear()) {
      return -1;
    }

    fprintf(stderr, "WARNING  %s:Fmq::_check_and_clear\n", _progName.c_str());
    fprintf(stderr, "  FMQ path '%s'\n", _fmqPath.c_str());
    fprintf(stderr, "  FMQ check failed\n");
    fprintf(stderr, "  FMQ re-initialized\n");
    
  }

  return 0;

}

////////////////////////////////////////////////////////////
//  Compute checksum for stat struct
//
//  Returns checksum
///

int Fmq::_compute_stat_checksum(const q_stat_t *stat)

{

  int sum = 0;
  
  sum += stat->magic_cookie;
  sum += ~stat->youngest_id;
  sum += stat->youngest_slot;
  sum += ~stat->oldest_slot;
  sum += stat->nslots;
  sum += stat->buf_size;
  sum += ~stat->begin_insert;
  sum += stat->end_insert;
  sum += ~stat->begin_append;
  sum += stat->append_mode;
  sum += stat->time_written;
  sum += ~stat->blocking_write;

  return sum;

}

////////////////////////////////////////////////////////////
//  Compute checksum for slot struct
//
//  Returns checksum
///

int Fmq::_compute_slot_checksum(const q_slot_t *slot)

{

  int sum = 0;
  
  sum += slot->active;
  sum += ~slot->id;
  sum += slot->time;
  sum += ~slot->msg_len;
  sum += slot->stored_len;
  sum += slot->offset;
  sum += ~slot->type;
  sum += slot->subtype;
  sum += ~slot->compress;

  return sum;

}

////////////////////////////////////////////////////////////
//  Add a checksum to stat struct
///

void Fmq::_add_stat_checksum(q_stat_t *stat)

{

  int sum = _compute_stat_checksum(stat);
  stat->checksum = sum;

}

////////////////////////////////////////////////////////////
//  Add a checksum to slot struct
///

void Fmq::_add_slot_checksum(q_slot_t *slot)

{

  int sum = _compute_slot_checksum(slot);
  slot->checksum = sum;

}

////////////////////////////////////////////////////////////
//  Check checksum on stat struct
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_check_stat_checksum(const q_stat_t *stat)

{

  int sum;

  if (stat->checksum == 0) {
    // backward compatibility
    return 0;
  }

  sum = _compute_stat_checksum(stat);

  if (sum != stat->checksum) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
//  Check checksum on slot struct
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_check_slot_checksum(const q_slot_t *slot)

{

  int sum;

  if (slot->checksum == 0) {
    // backward compatibility
    return 0;
  }

  sum = _compute_slot_checksum(slot);

  if (sum != slot->checksum) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// SEARCH
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Fmq::_prev_slot()
//
// Returns the slot number before the one given.
//
///

int Fmq::_prev_slot ( int slot_num)

{

  if (slot_num == 0) {
    return (_stat.nslots - 1);
  } else {
    return (slot_num - 1);
  }

}

////////////////////////////////////////////////////////////
// Fmq::_next_slot()
//
// Returns the slot number following the one given.
//
///

int Fmq::_next_slot ( int slot_num)

{

  if (slot_num >= _stat.nslots - 1) {
    return 0;
  } else {
    return (slot_num + 1);
  }

}

////////////////////////////////////////////////////////////
// Fmq::_prev_id()
//
// Returns the id number before the one given.
//
///

int Fmq::_prev_id (int id)

{
  if (id == 0) {
    return (Q_MAX_ID - 1);
  } else {
    return (id - 1);
  }
}

////////////////////////////////////////////////////////////
// Fmq::_next_id()
//
// Returns the id number following the one given.
//
///

int Fmq::_next_id (int id)

{

  if (id == Q_MAX_ID - 1) {
    return 0;
  } else {
    return (id + 1);
  }

}

////////////////////////////////////////////////////////////
// Fmq::_find_slot_for_id()
//
// Loads the slot number for the given id.
//
// Returns 0 on success, -1 on error.
//
///

int Fmq::_find_slot_for_id(int search_id, int *slot_p)
     
{

  int islot;
  q_slot_t *slot;

  // special case - search_id is -1, no messages yet.
  // return slot num of -1.

  if (search_id == -1) {
    *slot_p = -1;
    return 0;
  }

  if (_read_slots()) {
    return -1;
  }

  slot = _slots;
  for (islot = 0; islot < _stat.nslots; islot++, slot++) {
    
    if (search_id == slot->id) {
      *slot_p = islot;
      return 0;
    }

  } 

  return -1;

}

////////////////////////////////////////////////////////////
// Fmq::_slot_in_active_region()
//
// Checks if the given slot is within the active
// region or not.
//
// This is based upon the settings of oldest_slot and youngest_slot.
// If oldest_slot is less than youngest_slot, then the active area
// is between the two, inclusive of the two.
// Otherwise, it is the inactive area which is between them.
// See fmq.doc.
//
// Return value:
//   true if YES, false if NO.
///

int Fmq::_slot_in_active_region (int slot_num)

{

  // case in which no slots have yet been used - both oldest
  // and youngest are -1
  
  if (_stat.youngest_slot < 0 ||
      _stat.oldest_slot < 0) {
    return false;
  }

  if (_stat.youngest_slot >= _stat.oldest_slot) {
    
    if (slot_num >= _stat.oldest_slot &&
	slot_num <= _stat.youngest_slot) {
      return true;
    } else {
      return false;
    }
    
  } else {
    
    if (slot_num >= _stat.oldest_slot ||
	slot_num <= _stat.youngest_slot) {
      return true;
    } else {
      return false;
    }
    
  }

}

////////////////////////////////////////////////////////////
// SPACE AVAILABILITY
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Check space availability
//
// Returns true if space available, false otherwise if not,
// -1 on error.
//
// Toggles between append and insert mode as applicable.
// In append mode, the space to be used occurs after the
// begin_append offset. In insert mode, the space to be
// used occurs between the begin_insert and end_insert
// offsets.
///

int Fmq::_space_avail( int stored_len)

{

  int space;

  if (_stat.append_mode) {

    space = _stat.buf_size - _stat.begin_append;

    if (space >= stored_len) {
      return true;
    } else {
      _stat.append_mode = false;
      return false;
    }

  } else {

    space = _stat.end_insert - _stat.begin_insert;

    if (space >= stored_len) {
      return true;
    } else {
      if (_free_oldest_slot()) {
	return -1;
      }
      return false;
    }

  } // if (_stat.append_mode)

}

////////////////////////////////////////////////////////////
// Fmq::_fraction_used()
//
// Computes the fraction of the available
// space used in terms of slots and buffer.
//
// returns 0 on success, -1 on failure.
///

int Fmq::_fraction_used(double *slot_fraction_p,
			      double *buffer_fraction_p)

{

  int islot;
  int nslots_active;
  int nbytes_active;
  double slot_fraction;
  double buffer_fraction;
  q_slot_t *slot;

  // read in status struct 
  
  if (_read_stat()) {
    return -1;
  }

  // read in slots array
  
  if (_read_slots()) {
    return -1;
  }

  // count the active slots

  nslots_active = 0;
  nbytes_active = 0;
  slot = _slots;
  for (islot = 0; islot < _stat.nslots; islot++, slot++) {
    if (slot->active) {
      nslots_active++;
      nbytes_active += slot->stored_len;
    }
  } 

  slot_fraction =
    (double) nslots_active / (double) _stat.nslots;

  buffer_fraction =
    (double) nbytes_active / (double) _stat.buf_size;

  *slot_fraction_p = slot_fraction;
  *buffer_fraction_p = buffer_fraction;

  return 0;

}

////////////////////////////////////////////////////////////
// PRINTING
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
//  print_slot()
//
//  Prints out a slot.
///

void Fmq::_print_slot(FILE *out, const char *label, int num) const
  
{
  
  fprintf(out,
	  "%s - %d: active %d, id %d, time %s, msg_len %d, "
	  "stored_len %d, offset %d, type %d, subtype %d, "
          "compress %s, checksum %d\n",
	  label, num,
	  _slot.active, 
	  _slot.id, 
	  utimstr(_slot.time), 
	  _slot.msg_len, 
	  _slot.stored_len, 
	  _slot.offset, 
	  _slot.type, 
	  _slot.subtype,
	  BOOL_STR(_slot.compress),
          _slot.checksum);
  
}

////////////////////////////////////////////////////////////
//  Print slots
///

void Fmq::_print_slots(FILE *out) const
     
{
  q_slot_t *slot = _slots;
  int islot;
  for (islot = 0; islot < _stat.nslots; islot++, slot++) { 
    _basic_print_slot(islot, slot, out);
  }
}

////////////////////////////////////////////////////////////
//  Print slots
///

void Fmq::_print_active_slots( FILE *out) const
     
{
  q_slot_t *slot = _slots;
  int islot;
  for (islot = 0; islot < _stat.nslots; islot++, slot++) { 
    if (slot->active) {
      _basic_print_slot(islot, slot, out);
    }
  }
}

////////////////////////////////////////////////////////////
//  Print slot
///

void Fmq::_basic_print_slot(int slot_num, q_slot_t *slot, FILE *out) const
     
{

  fprintf(out, "%d ", slot_num);
  fprintf(out, "%d ", slot->active);
  fprintf(out, "%d ", slot->id);
  fprintf(out, "%s ", utimstr(slot->time));
  fprintf(out, "%d ", slot->msg_len);
  fprintf(out, "%d ", slot->stored_len);
  fprintf(out, "%d ", slot->offset);
  fprintf(out, "%d ", slot->type);
  fprintf(out, "%d ", slot->subtype);
  fprintf(out, "%d ", slot->compress);
  fprintf(out, "%d ", slot->checksum);
  fprintf(out, "\n");
    
}

void Fmq::_pretty_print_slot(int slot_num, q_slot_t *slot, FILE *out) const
     
{

  fprintf(out, "======== slot num %d ========\n", slot_num);
  fprintf(out, "  active: %d \n", slot->active);
  fprintf(out, "  id: %d \n", slot->id);
  fprintf(out, "  time: %s \n", utimstr(slot->time));
  fprintf(out, "  msg_len: %d \n", slot->msg_len);
  fprintf(out, "  stored_len: %d \n", slot->stored_len);
  fprintf(out, "  offset: %d \n", slot->offset);
  fprintf(out, "  type: %d \n", slot->type);
  fprintf(out, "  subtype: %d \n", slot->subtype);
  fprintf(out, "  compress: %d \n", slot->compress);
  fprintf(out, "  checksum: %d \n", slot->checksum);
  fprintf(out, "\n");
    
}

////////////////////////////////////////////////////////////
// ERROR PRINTING
////////////////////////////////////////////////////////////

///////////////////////////
// initialize error string

void Fmq::initErrStr() const

{

  _errStr.clear();

  char errTxt[8192];

  sprintf(errTxt, "ERROR - FMQ, fmq path: %s\n", _fmqPath.c_str());
  _errStr += errTxt;

}

////////////////////////////////////////////////////////////
// Handle error message.
// (a) Add to _errStr.
// (b) If _debug is true, print to msgLog if non-null,
//     otherwise to stderr.

void Fmq::_print_error(const char *routine, const char *format, ...) const
     
{

  va_list args;
  char errTxt[8192];

  if (routine != NULL) {
    sprintf(errTxt, "ERROR - %s:Fmq::%s\n", _progName.c_str(), routine);
    _errStr += errTxt;
    sprintf(errTxt, "Fmq path: %s\n", _fmqPath.c_str());
    _errStr += errTxt;
  }

  if (format != NULL) {
    va_start (args, format);
    vsprintf (errTxt, format, args);
    _errStr += errTxt;
    _errStr += "\n";
    va_end (args);
  }

  if (_debug) {
    if (_msgLog != NULL) {
      _msgLog->postMsg(ERROR, "%s\n", _errStr.c_str());
    } else {
      fprintf(stderr, "%s\n", _errStr.c_str());
    }
  }

}

////////////////////////////////////////////////////////////
// Fmq::_print_debug_msg
//
// Prints message to stderr if debug is set.
//
// If format is not NULL, it is used to print the remaining
// args in the var_args list.
//
///

void Fmq::_print_info(const char *routine, const char *format, ...) const
  
{

  if (!_debug) {
    return;
  }

  if (routine != NULL) {
    if (_msgLog == NULL) {
      fprintf(stderr, "INFO: %s:Fmq::%s\n", _progName.c_str(), routine);
      fprintf(stderr, "INFO: fmqPath: %s\n", _fmqPath.c_str());
    } else {
      _msgLog->postMsg(INFO, "%s:Fmq::%s\n", _progName.c_str(), routine);
      _msgLog->postMsg(INFO, "fmqPath: %s\n", _fmqPath.c_str());
    }
  }

  va_list args;
  char infoTxt[8192];
  
  if (format != NULL) {
    va_start (args, format);
    vsprintf (infoTxt, format, args);
    va_end (args);
  }

  if (_msgLog == NULL) {
    fprintf(stderr, "INFO: %s\n", infoTxt);
  } else {
    _msgLog->postMsg(INFO, "%s\n", infoTxt);
  }

}

////////////////////////////////////////////////////////////
// BYTE SWAPPING
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// BE to host for Fmq::_stat
///

void Fmq::be_to_stat(q_stat_t *stat)
{
  BE_to_array_32(stat, sizeof(q_stat_t));
}

////////////////////////////////////////////////////////////
// BE from host for Fmq::_stat
///

void Fmq::be_from_stat(q_stat_t *stat)
{
  BE_from_array_32(stat, sizeof(q_stat_t));
}

////////////////////////////////////////////////////////////
// BE to host for Fmq::_slot
///

void Fmq::be_to_slot(q_slot_t *slot)
{
  BE_to_array_32(slot, sizeof(q_slot_t));
}

////////////////////////////////////////////////////////////
// BE from host for Fmq::_slot
///

void Fmq::be_from_slot(q_slot_t *slot)
{
  BE_from_array_32(slot, sizeof(q_slot_t));
}

////////////////////////////////////////////////////////////
// DATA MAPPER
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// do registration with data mapper

int Fmq::_doRegisterWithDmap()
  
{
  
  if (!_registerWithDmap) {
    return 0;
  }

  time_t now = time(NULL);
  double secsSinceLastReg = (double) now - (double) _dmapPrevRegTime;
  int iret = 0;
  if (secsSinceLastReg >= _dmapRegIntervalSecs) {
    DmapAccess dmap;
    dmap.regLatestInfo(now, _fmqPath, "fmq");
    _dmapPrevRegTime = now;
  }

  return iret;
  
}

////////////////////////////////////////////////////////////
// RECOVERY FROM CORRUPTED BUFFERS - not currently used
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
//  Fmq::_check_and_recover()
//
//  Checks that the FMQ files are valid, performs recovery
//  if possible.
//
//  Assumes the files are not open.
//
//  Return value:
//    0 on success, -1 on failure
///

int Fmq::_check_and_recover ()

{

  int iret = 0;

  if (_check_device_exists()) {
    return -1;
  }

  // open the files

  if (_open_rdwr(0, 0)) {
    return -1;
  }

  // check the file for corruption

  if (_check()) {

    // recover the FMQ if possible
    
    _lock_device();
    iret = _recover();
    _unlock_device();

  } // if _check(....)

  _close();

  return iret;

}

////////////////////////////////////////////////////////////
//  Fmq::_recover()
//
//  Recover the FMQ from a corrupted state.
//
//  Return value:
//    0 on success, -1 on failure.
///

int Fmq::_recover ()

{

  int islot;
  int done;
  int append_mode;
  int ids_wrap = false;
  int zero_present;
  int max_present;
  int maxId;

  int nslots = _stat.nslots;
  int youngest_id = 0, oldest_id = 0;
  int youngest_slot_num = 0, oldest_slot_num = 0;
  int begin_insert = 0, end_insert = 0, begin_append = 0;

  q_slot_t *slot = NULL;
  
  _print_info("_recover",
	      "Recovering Fmq: %s", _fmqPath.c_str());
  
  // do the ID's wrap?
  // They do if both 0 and MAX are present.

  zero_present = false;
  max_present = false;
  for (islot = 0; islot < nslots; islot++, slot++) {
    if (slot->id == 0) {
      zero_present = true;
    }
    if (slot->id == Q_MAX_ID - 1) {
      max_present = true;
    }
  }
  if (zero_present && max_present) {
    ids_wrap = true;
  }

  // find youngest_id and youngest_slot_num, respecting the fact that the
  // IDs may have wrapped

  maxId = Q_MAX_ID;
  if (ids_wrap) {
    maxId = nslots;
  }
  youngest_id = -1;
  slot = _slots;
  for (islot = 0; islot < nslots; islot++, slot++) {
    if (slot->active && slot->id > youngest_id && slot->id < maxId) {
      youngest_id = slot->id;
      youngest_slot_num = islot;
    }
  }

  if (youngest_id == -1) {
    return -1;
  }

  // Find oldest_slot_num by moving back through the slots, checking
  // that the previous slot number is correct. If this pattern
  // breaks, the oldest valid slot has been found.

  oldest_slot_num = youngest_slot_num;
  done = false;

  while (!done) {
    int prev_slot_num = _prev_slot(oldest_slot_num);
    int prev_id = _prev_id(oldest_id);
    q_slot_t *prev_slot = _slots + prev_slot_num;
    if (prev_slot->id != prev_id) {
      done = true;
    } else {
      oldest_slot_num = prev_slot_num;
      oldest_id = prev_id;
    }
  }
  
  if (_debug) {
    _print_info(NULL, "--> Original  youngest, oldest: %d, %d",
		_stat.youngest_slot, _stat.oldest_slot);
    _print_info(NULL, "--> Recovered youngest, oldest: %d, %d",
		youngest_slot_num, oldest_slot_num);
  }

  _stat.youngest_slot = youngest_slot_num;
  _stat.oldest_slot = oldest_slot_num;

  // set all other messages inactive

  slot = _slots;
  for (islot = 0; islot < nslots; islot++, slot++) {
    if (!_slot_in_active_region(islot) && slot->active) {
      _print_error("_recover",
		   "Setting slot %d inactive", islot);
      MEM_zero(*slot);
      if (_write_slot(islot)) {
	return -1;
      }
    }
  }

  ////////////////////////////////////////////////////////////
  // Determine the begin and end of the insert and append regions,
  // and whether the buffer is in append mode.
  //
  // The following rules are applied:
  //
  //   1. If the youngest slot stores its message closest to the end
  //      of the buffer, the FMQ is in append mode. Otherwise it is
  //      in insert mode (!append_mode).
  //
  //   2. The append region starts at the end of the active slot which
  //      is closest to the end of the buffer file.
  //
  //   3. In insert mode, the insert region starts at the end of the
  //      youngest slot. In append mode, the insert region starts at
  //      that start of the file.
  //
  //   4. The insert region ends at the start of the oldest slot.
  ///

  // check for append mode

  append_mode = true;
  begin_append = 0;
  begin_insert = 0;
  end_insert = 0;

  {
    q_slot_t *youngest_slot = _slots + youngest_slot_num;
    q_slot_t *oldest_slot = _slots + oldest_slot_num;
    int youngest_end = youngest_slot->offset + youngest_slot->stored_len;
    slot = _slots;
    for (islot = 0; islot < nslots; islot++, slot++) {
      if(slot->active) {
        int end = slot->offset + slot->stored_len;
        if (begin_append > end) {
          begin_append = end;
        }
        if (end > youngest_end) {
          append_mode = false;
          break;
        }
      }
    }
    if (!append_mode) {
      begin_insert = youngest_slot->offset + youngest_slot->stored_len;
    }
    end_insert = oldest_slot->offset;
  }
  
  _print_info(NULL, "--> Original BI, EI, BA, AM: %d, %d, %d, %d",
	      _stat.begin_insert, _stat.end_insert,
	      _stat.begin_append, _stat.append_mode);
  _print_info(NULL, "--> Recovered BI, EI, BA, AM: %d, %d, %d, %d",
	      begin_insert, end_insert, begin_append, append_mode);

  _stat.begin_insert = begin_insert;
  _stat.end_insert = end_insert;
  _stat.begin_append = begin_append;
  _stat.append_mode = append_mode;

  if (_write_stat()) {
    return -1;
  }

  return 0;

}

