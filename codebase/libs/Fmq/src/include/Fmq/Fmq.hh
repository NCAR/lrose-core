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
// Fmq.cc
// Fmq class
//
// Message queue - supports file-based queue and shared-memory-based queue.
// Default behavior is file-based queue.
// If fmqPath has a filename of "shmem_xxxxxx", a shared-memory queue
// is used instead.
//
// Mike Dixon, RAL, NCAR, Boulder, Colorado
//
// Jan 2009
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef _FMQ_HH_INCLUDED_
#define _FMQ_HH_INCLUDED_

#include <toolsa/compress.h>
#include <toolsa/MemBuf.hh>
#include <Fmq/FmqDeviceFile.hh>
#include <Fmq/FmqDeviceShmem.hh>
using namespace std;

// Forward class declarations

class MsgLog;

// class definition

class Fmq

{

  friend class DsFmqMsg;

public:                  

  // enums

  enum openMode { CREATE,
		  READ_WRITE,
		  READ_ONLY,
		  BLOCKING_READ_ONLY,
		  BLOCKING_READ_WRITE };

  enum openPosition { START, END };

  enum seekPosition { FMQ_SEEK_START,
		      FMQ_SEEK_END,
		      FMQ_SEEK_LAST,
		      FMQ_SEEK_BACK };

  static const int Q_MAGIC_STAT = 88008801;
  static const int Q_MAGIC_BUF = 88008802;
  static const int Q_MAX_ID = 1000000000;
  static const int Q_NBYTES_EXTRA = 12;

  // FMQ status struct
  
  typedef struct {
    
    si32 magic_cookie;    /* magic cookie for file type */
    
    si32 youngest_id;     /* message id of last message written */
    si32 youngest_slot;   /* num of slot which contains the
			   * youngest message in the queue */
    si32 oldest_slot;     /* num of slot which contains the
			     oldest message in the queue */
    
    si32 nslots;          /* number of message slots */
    si32 buf_size;        /* size of buffer */
    
    si32 begin_insert;    /* offset to start of insert free region */
    si32 end_insert;      /* offset to end of insert free region */
    si32 begin_append;    /* offset to start of append free region */
    si32 append_mode;     /* TRUE for append mode, FALSE for insert mode */
    
    si32 time_written;    /* time at which the status struct was last
			   * written to file */
    
    /* NOTE - blocking write only supported for 1 reader */
    
    si32 blocking_write;  /* flag to indicate blocking write */
    si32 last_id_read;    /* used for blocking write operation */
    si32 checksum;
    
  } q_stat_t;
  
  // FMQ slot struct
  
  //   Messages are stored in the buffer as follows:
  //        si32         si32                             si32
  //   --------------------------------------------------------
  //   | magic cookie |  slot_num  | -- message -- | pad |  id  |
  //   --------------------------------------------------------
  //   Pad is for 4-byte alignment.
  
  typedef struct {
    
    si32 active;          /* active flag, 1 or 0 */
    si32 id;              /* message id 0 to FMQ_MAX_ID */
    si32 time;            /* Unix time at which the message is written */
    si32 msg_len;         /* message len in bytes */
    si32 stored_len;      /* message len + extra 12 bytes (FMQ_NBYTES_EXTRA)
			   * for magic-cookie and slot num fields,
			   * plus padding out to even 4 bytes */
    si32 offset;          /* message offset in buffer */
    si32 type;            /* message type - user-defined */
    si32 subtype;         /* message subtype - user-defined */
    si32 compress;        /* compress mode - TRUE or FALSE */
    si32 checksum;
    
  } q_slot_t;
  
  // constructor
  // note: you must call one of the init() functions
  // before the object is ready to be used.

  Fmq();                       

  // destructor

  virtual ~Fmq();

  // Set the heartbeat function. This function will be called while
  // the 'blocking' open and read calls are waiting.
  //
  // The default heartbeat_func is PMU_auto_regsister.
  // 
  // If you do not want heartbeat calls to procmap, use this
  // method to set it to NULL.
  
  void setHeartbeat(TA_heartbeat_t heartbeat_func);

  // Sets the flag which indicates the object is being
  // used by a server.
  
  void setServer();

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
  // Messages written to msgLog if non-NULL
  
  virtual int initCreate(const char* fmqPath, 
			 const char* procName, 
			 bool debug = false,
			 bool compress = false, 
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
  // Messages written to msgLog if non-NULL
 
  virtual int initReadWrite(const char* fmqPath, 
			    const char* procName, 
			    bool debug = false,
			    openPosition position = END,
			    bool compress = false, 
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
  // Messages written to msgLog if non-NULL
  
  virtual int initReadOnly(const char* fmqPath, 
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
  // Messages written to msgLog if non-NULL

  // Note - if the queue does not exist, and we have to wait for
  // it, then the openPosition will be set to Fmq::START no
  // matter what the input argument is set to, since in this case
  // we definitely want to read from the start and not miss anything
  // that was written to the queue when it was initialized.
  // This subtlety was implemented March 2006. Prior to this, the
  // first entry written to a queue may have been missed. Niles Oien.

  virtual int initReadBlocking(const char* fmqPath, 
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
  // Messages written to msgLog if non-NULL
 
  // See comments above for initReadBlocking() about the setting of
  // the open position - they apply to initReadWriteBlocking as well.

  virtual int initReadWriteBlocking(const char* fmqPath, 
				    const char* procName, 
				    bool debug = false,
				    openPosition position = END,
				    int msecSleep = -1,
				    MsgLog *msgLog = NULL);

  // generic init() - allows full control of the init
  // Returns 0 on success, -1 on error
  // Messages written to msgLog if non-NULL

  virtual int init(const char* fmqPath, 
		   const char* procName, 
		   bool debug = false,
		   openMode mode = READ_WRITE, 
		   openPosition position = END,
		   bool compress = false, 
		   size_t numSlots = 1024, 
		   size_t bufSize = 10000,
		   int msecSleep = -1,
		   MsgLog *msgLog = NULL);

  // Setting a timeout for blocking reads

  void setBlockingReadTimeout(int msecs) { _msecBlockingReadTimeout = msecs; }
 
  // Closing the fmq
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
  
  virtual int writeMsg(int type,
		       int subType = 0,
		       const void *msg = NULL,
		       int msgLen = 0);
  
  // Writing precompressed msg
  // Returns 0 on success, -1 on error

  virtual int writeMsgPreCompressed(int type, int subType, 
				    const void *msg, int msgLen,
				    int uncompressedLen);
  
  ///////////////////////////////////////////////////////////
  // get methods

  // get queue details

  inline const char *getProcName() const
  {
    return _progName.c_str();
  }

  inline const string &getProgName() const
  {
    return _progName;
  }

  // get queue details
  
  virtual bool isOpen() { return _dev != NULL; }
  bool isServer() { return _server; }

  ///////////////////////////////////////
  // get fmq details
  
  inline const string &getFmqPath() const
  {
    return _fmqPath;
  }
  inline int getNumSlots() const
  {
    return _stat.nslots;
  }
  inline int getBufSize() const
  {
    return _stat.buf_size;
  }

  ////////////////////////////////////////////////
  // get details of message returned from read()
  
  // get pointer to message buffer itself

  inline const void *getMsg() const
  { 
    return _msgBuf.getPtr();
  }

  // get message length
  // this is the uncompressed length, except if the 'server' flags is
  // set and compression is on, since the server will pass the data
  // to the client in compressed form

  inline int getMsgLen() const
  {
    return _msgBuf.getLen();
  }

  // is compression active?

  inline bool getCompress() const
  {
    return _compress;
  }

  // what compression method is in use?

  inline ta_compression_method_t getCompressMethod() const
  {
    return _compressMethod;
  }

  // get the stored length of the message

  inline int getStoredLen() const
  {
    return _slot.stored_len;
  }

  // is the stored message compressed?

  inline bool isMsgCompressed() const
  {
    return (bool) _slot.compress;
  }

  // get the compressed length of the message
  // same as stored length

  inline int getCompressedLen() const
  {
    return _slot.stored_len;
  }

  // get the message length for the current slot
  // In compressed data, this is the original message len

  inline int getSlotMsgLen() const
  {
    return _slot.msg_len;
  }

  // get the message ID

  inline int getMsgId() const
  { 
    return _slot.id;
  }

  // get the message time
  
  inline time_t getMsgTime() const
  { 
    return _slot.time;
  }

  // get the message type

  inline int getMsgType() const
  {
    return _slot.type; 
  }

  // get the message sub-type

  inline int getMsgSubtype() const
  { 
    return _slot.subtype; 
  }
  
  // Is this a shared-memory-based queue?
  // inspect fmqPath to find out.
  // If the fileName is of the form "shmem_xxxxxxxx", then this is a
  // shared memory queue with a key of xxxxxxxx.
  // Returns true if key found, false otherwise.

  static bool isShmemQueue(const string &fmqPath);

  // Compute shmem key.
  // Returns 0 on success, -1 on failure

  static int getShmemKey(const string &fmqPath, key_t &key);

  ///////////////////////////////////////////////////////////////////
  // error string is set during open/read/write operations
  // get error string is an error is returned

  void initErrStr() const;
  const string &getErrStr() { return (_errStr); }

  // printing

  ///////////////////////////////////////////////////////////
  //  Print out the status struct.
  
  void print_stat(FILE *out) const; 
  
  ///////////////////////////////////////////////////////////
  //  Print out the latest slot read.
  
  void print_slot_read(FILE *out) const;
  
  ///////////////////////////////////////////////////////////
  // Print out the latest slot written.
  
  void print_slot_written(FILE *out) const;

  ///////////////////////////////////////////////////////////
  //  Debugging printout - will work for invalid
  //  FMQ files.
  //
  //  Print out the status and slot structs.
  //  Opens and closes the files.
  //  Returns 0 on success, -1 on failure.
  
  static int print_debug(const char *fmq_path,
			 const char *prog_name,
			 FILE *out);
  
  // byte swapping

  static void be_from_slot(q_slot_t *slot);
  static void be_from_stat(q_stat_t *stat);
  static void be_to_slot(q_slot_t *slot);
  static void be_to_stat(q_stat_t *stat);

protected:

  // device implementation

  FmqDevice *_dev;

  // initialization

  string _fmqPath;
  string _progName;
  bool _debug;
  openMode _openMode;
  openPosition _openPosition;
  bool _compress;
  int _numSlots;
  int _bufSize;
  int _msecSleep;
  int _msecBlockingReadTimeout;

  // logging
  
  MsgLog *_msgLog;

  // flags
  
  bool _createdOnOpen;
  bool _server;        /* indicates that the object is being used by
			* a server rather than a client */
  
  ta_compression_method_t _compressMethod; /* see <toolsa/compress.h>
					    * for definitions */
  
  // buffer for message
  
  MemBuf _msgBuf;      /* buffer for message */
  
  // copy of latest stat and slot read

  q_stat_t _stat;  /* status struct */
  q_slot_t _slot;  /* copy of last slot read or written */
  
  // counters and flags
  
  int _lastIdRead;     /* latest id read by FMQ_read() */
  int _lastSlotRead;   /* latest slot read by FMQ_read() */
  
  int _blockingWrite;  /* should the write block if the queue is full? */
  int _lastSlotWritten;/* used by blocking write */
  
  int _singleWriter;   /* flag to indicate that only a single writer is running
			* so that locking is not necessary */

  // memory allocation

  int _nslotsAlloc;    /* Number of slots allocated */
  q_slot_t *_slots;/* slots array */
  
  ui08 *_entry;        /* message entry array - includes extra bytes
			* msg points into this array */
  int _nEntryAlloc;    /* number of bytes allocated for msg entry */

  // heartbeat function for procmap registration

  TA_heartbeat_t _heartbeatFunc; /* heartbeat function which is called
				  * by the blocking functions during wait
				  * periods. If left NULL, no call is made */
  
  // data mapper registration
  
  bool _registerWithDmap;   // should we register with the data mapper?
  int _dmapRegIntervalSecs; // how often should we register?
  time_t _dmapPrevRegTime;  // the time of the previous registration

  // errors
  
  mutable string _errStr;

  //////////////////////////////////////////////////////////
  // protected methods

  // initialization

  void _init_status(int nslots, int buf_size);

  // memory management

  int _alloc_slots(int nslots);
  void _free_slots();
  int _free_oldest_slot();
  void _alloc_entry(int msg_len);
  void _free_entry();

  // open

  int _open(openMode mode, size_t numSlots, size_t bufSize);
  int _open_create(int nslots, int buf_size);
  int _open_rdwr(int nslots, int buf_size);
  int _open_rdonly();
  int _open_rdwr_nocreate();
  int _open_blocking(int msecs_sleep);
  int _open_blocking_rdwr(int msecs_sleep);
  int _open_device(const char *mode);

  // close / clear

  void _close();
  void _close_device();
  int _clear();

  // locking for writes

  int _lock_device();
  int _unlock_device();

  // seek

  int _seek_end();
  int _seek_last();
  int _seek_start();
  int _seek_back();
  int _seek_device(FmqDevice::ident_t id, off_t offset);

  // read
  
  int _read(int *msg_read, int type);
  int _read_blocking(int msecs_sleep, int type);
  int _read_non_blocking (int *msg_read, int type, int msecs_sleep);
  int _read_stat();
  int _read_slots();
  int _read_slot(int slot_num);
  int _read_next(int *msg_read);
  int _read_msg_for_slot(int slot_num);
  int _read_msg(int slot_num);

  int _load_read_msg(int msg_type,
		     int msg_subtype,
		     int msg_id,
		     time_t msg_time,
		     void *msg,
		     int stored_len,
		     int compressed,
		     int uncompressed_len);
  
  int _read_device(FmqDevice::ident_t id, void *mess, size_t len);
  int _update_last_id_read();
  int _add_read_msg(void *msg, int msg_size);
  
  // write

  int _prepare_for_writing(int nslots, int buf_size);

  int _write(void *msg, int msg_len,
	     int msg_type, int msg_subtype);

  int _write_precompressed(void *msg, int msg_len,
			   int msg_type, int msg_subtype,
			   int uncompressed_len);
  
  int _write_stat();
  int _write_slot(int slot_num);
  
  int _write_msg(void *msg, int msg_len,
		 int msg_type, int msg_subtype,
		 int pre_compressed, int uncompressed_len);
  
  int _write_msg_to_slot(int write_slot, int write_id,
			 void *msg, int msg_len, int stored_len, int offset);
  
  virtual int _write_device(FmqDevice::ident_t id, const void *mess, size_t len);

  // checking and consistency
  
  int _check();
  int _check_and_clear();

  virtual int _check_device_exists();
  virtual int _check_buffer_sizes();
  
  int _compute_stat_checksum(const q_stat_t *stat);
  int _compute_slot_checksum(const q_slot_t *slot);
  void _add_stat_checksum(q_stat_t *stat);
  void _add_slot_checksum(q_slot_t *slot);
  int _check_stat_checksum(const q_stat_t *stat);
  int _check_slot_checksum(const q_slot_t *slot);

  // search
  
  int _prev_id(int id);
  int _next_id(int id);
  int _next_slot(int slot_num);
  int _prev_slot(int slot_num);
  int _find_slot_for_id(int search_id, int *slot_p);
  int _slot_in_active_region(int slot_num);

  // space used

  int _space_avail(int stored_len);
  int _fraction_used(double *slot_fraction_p,
		     double *buffer_fraction_p);

  // printing

  void _print_slot(FILE *out, const char *label, int num) const;
  void _print_slots(FILE *out) const;
  void _print_active_slots(FILE *out) const;
  void _basic_print_slot(int slot_num, q_slot_t *slot, FILE *out) const;
  void _pretty_print_slot(int slot_num, q_slot_t *slot, FILE *out) const;

  // error printing

  void _print_error(const char *routine, const char *format, ...) const;
  void _print_info(const char *routine, const char *format, ...) const;

  // data mapper registration
  
  int _doRegisterWithDmap();

private:

  // recovery from corrupt buffer - not currently used
  // instead we reinitialize the fmq

  int _check_and_recover();
  int _recover();

};

#endif

