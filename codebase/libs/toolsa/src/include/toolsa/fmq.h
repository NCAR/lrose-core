/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/******************************************************
 * fmq.h
 *
 * File Message Queue - FMQ - utility
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 * See also fmq.doc
 */

#ifndef FMQ_H
#define FMQ_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <toolsa/compress.h>
#include <toolsa/heartbeat.h>
#include <dataport/port_types.h>

#define FMQ_MAGIC_STAT 88008801
#define FMQ_MAGIC_BUF  88008802
#define FMQ_MAX_ID 1000000000
#define FMQ_NBYTES_EXTRA 12

/*
 * FMQ status struct
 */

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

} fmq_stat_t;

/*
 * FMQ slot struct
 */

/*
 * Messages are stored in the buffer as follows:
 *      si32         si32                             si32
 * --------------------------------------------------------
 * | magic cookie |  slot_num  | -- message -- | pad |  id  |
 * --------------------------------------------------------
 * Pad is for 4-byte alignment.
 */

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

} fmq_slot_t;

/*
 * FMQ handle struct
 */

typedef struct {

  /*
   * public interface
   * users gain access to these via the public functions
   */

  char *fmq_path;       /* message queue path */
  void *msg;            /* message */
  void *dmsg;           /* decompressed message */
  fmq_stat_t fstat;     /* status struct */
  fmq_slot_t fslot;     /* copy of last slot read or written */

  /*
   * private interface - callers should not reference any of
   * the fields below.
   */

  char *prog_name;      /* calling program name */
  char *stat_path;      /* status path */
  char *buf_path;       /* buffer path */

  int debug;            /* debug flag */
  int server;           /* indicates that the handle is being used by
			 * a server rather than a client */

  int write;            /* set TRUE for write side, FALSE for read
			 * side */

  int compress;         /* compress mode - TRUE or FALSE */
  ta_compression_method_t compress_method; /* see <toolsa/compress.h>
					    * for definitions */

  int last_id_read;     /* latest id read by FMQ_read() */
  int last_slot_read;   /* latest slot read by FMQ_read() */
  int msg_len;          /* size of latest message read */
  int magic_cookie;
  
  int blocking_write;    /* reserved for blocking write */
  int last_slot_written; /* reserved for blocking write */
  
  int nslots_alloc;     /* Number of slots allocated */
  fmq_slot_t *fslots;   /* slots array */

  void *entry;          /* message entry array - includes extra bytes
			 * msg points into this array */
  int n_entry_alloc;   /* number of bytes allocated for msg entry */

  FILE *stat_file;      /* streams file pointer - for fopen/fclose */
  FILE *buf_file;       /* streams file pointer - for fopen/fclose */

  int stat_fd;          /* file descriptor - used for unbuffered I/O */
  int buf_fd;           /* file descriptor - used for unbuffered I/O */

  TA_heartbeat_t heartbeat_func; /* heartbeat function which is called
                                  * by the blocking functions during wait
                                  * periods. If left NULL, no call is made */

} FMQ_handle_t;

/*
 * prototypes
 */

/*******************
 * FMQ_init()
 *
 * Initialize FMQ handle
 *
 * This must be done before using the handle for any
 * other function.
 *
 * Arg list:
 *  fmq_path:  path of FMQ
 *  debug;     set debugging on
 *  prog_name: program name (for debugging and error messages)
 *
 * Returns 0 on success, -1 on error.
 */

extern int FMQ_init(FMQ_handle_t *handle,
		    const char *fmq_path,
		    int debug,
		    const char *prog_name);

/*********************
 * FMQ_set_heartbeat()
 *
 * Set the heartbeat function. This function will be called while
 * the 'blocking' open and read calls are waiting.
 *
 * If set_heartbeat is not called, no heartbeat function will be
 * called.
 *
 * Returns 0 on success, -1 on error.
 */

extern int FMQ_set_heartbeat(FMQ_handle_t *handle,
			     TA_heartbeat_t heartbeat_func);

/********************
 * FMQ_set_server()
 *
 * Set server flag on.
 *
 * Returns 0 on success, -1 on error.
 */

extern int FMQ_set_server(FMQ_handle_t *handle);

/********************
 * FMQ_set_compress()
 *
 * Set compression on.
 *
 * Returns 0 on success, -1 on error.
 */

extern int FMQ_set_compress(FMQ_handle_t *handle);

/******************************
 * FMQ_set_compression_method()
 *
 * Set compression on.
 *
 * Returns 0 on success, -1 on error.
 */

extern int FMQ_set_compression_method(FMQ_handle_t *handle,
				      ta_compression_method_t method);

/**************************
 * FMQ_set_blocking_write()
 *
 * Set blocking write on.
 *
 * Returns 0 on success, -1 on error.
 */

extern int FMQ_set_blocking_write(FMQ_handle_t *handle);

/********************
 *  FMQ_open_create()
 *
 *  Creates the FMQ by opening in mode "w+".
 *  Overwrites any existing FMQ.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

extern int FMQ_open_create(FMQ_handle_t *handle,
			   int nslots, int buf_size);

/******************
 *  FMQ_open_rdwr()
 *
 *  If FMQ exists and is valid, opens in mode "r+".
 *  Otherwise, creates the FMQ by opening in mode "w+".
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

extern int FMQ_open_rdwr(FMQ_handle_t *handle,
			 int nslots, int buf_size);

/***************************
 *  FMQ_open_rdwr_nocreate()
 *
 *  If FMQ exists and is valid, opens in mode "r+".
 *  Otherwise, returns error.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

extern int FMQ_open_rdwr_nocreate(FMQ_handle_t *handle);

/********************
 *  FMQ_open_rdonly()
 *
 *  If FMQ exists and is valid, opens the files mode "r".
 *  Otherwise, returns error.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

extern int FMQ_open_rdonly (FMQ_handle_t *handle);

/**********************
 *  FMQ_open_blocking()
 *
 *  If valid FMQ does not exist, waits until it does,
 *  and then opens opens the files mode "r", i.e. rdonly.
 * 
 *  While waiting, registers with procmap if PMU module has
 *  been initialized.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

extern int FMQ_open_blocking (FMQ_handle_t *handle);

/*************************
 *  FMQ_open_blocking_rdwr()
 *
 *  If valid FMQ does not exist, waits until it does,
 *  and then opens opens the files mode "r+", i.e. rdwr
 * 
 *  While waiting, registers with procmap if PMU module has
 *  been initialized.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

extern int FMQ_open_blocking_rdwr (FMQ_handle_t *handle);
     
/*****************
 *  FMQ_seek_end()
 *
 *  Seek to the end of the FMQ.
 *
 *  Sets read pointers only - no affect on writes.
 *
 *  Only messages written after this call
 *  will be available for read.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

extern int FMQ_seek_end (FMQ_handle_t *handle);

/******************
 *  FMQ_seek_last()
 *
 *  Position for reading the last entry.
 *  Seek to the end of the FMQ, minus 1 slot.
 *
 *  Sets read pointers only - no affect on writes.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

extern int FMQ_seek_last (FMQ_handle_t *handle);

/*******************
 *  FMQ_seek_start()
 *
 *  Seek to the start of the FMQ.
 *
 *  Sets read pointers only - no affect on writes.
 *
 *  Set the read pointer to just before the oldest
 *  record, so that the entire buffer will be
 *  available for reads
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

extern int FMQ_seek_start (FMQ_handle_t *handle);

/******************
 *  FMQ_seek_back()
 *
 *  Move backwards by one entry.
 *
 *  Sets read pointers only - no affect on writes.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

extern int FMQ_seek_back (FMQ_handle_t *handle);

/********************
 *  FMQ_seek_to_id()
 *
 *  Seeks to a given ID.
 *  Positions the FMQ so that the next read will be the first
 *  slot after the specified ID. To read the actual ID, call
 *  this function followed by fmq_seek_back().
 *
 *  Sets read pointers only - no affect on writes.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

extern int FMQ_seek_to_id (FMQ_handle_t *handle, int id);

/*************
 *  FMQ_read()
 *
 *  Reads the next message from FMQ.
 *
 *  Sets msg_read TRUE if msg read, FALSE if not.
 * 
 *  Returns 0 on success, -1 on failure.
 */

extern int FMQ_read (FMQ_handle_t *handle, int *msg_read);

/*********************
 *  FMQ_load_read_msg()
 *
 *  Load the given message into the FMQ handle.
 *
 *  Returns 0 on success, -1 on failure.
 */

extern int FMQ_load_read_msg (FMQ_handle_t *handle,
			      int msg_type,
			      int msg_subtype,
			      int msg_id,
			      time_t msg_time,
			      void *msg,
			      int stored_len,
			      int compressed,
			      int uncompressed_len);

/******************
 *  FMQ_read_type()
 *
 *  Reads the next message of the given type from FMQ.
 *  Note: type must be non-negative.
 *
 *  Sets msg_read TRUE if msg read, FALSE if not.
 * 
 *  Returns 0 on success, -1 on failure.
 */

extern int FMQ_read_type (FMQ_handle_t *handle, int *msg_read,
			  unsigned int type);
     
/**********************
 *  FMQ_read_blocking()
 *
 *  This function reads a message from an FMQ - it blocks until
 *  a message is received.
 *
 *  Parameters:
 *    msecs_sleep - number of millisecs to sleep between reads
 *                  while waiting for a message to arrive.
 *  If msecs_sleep is set to -1, a default value of 10 msecs will
 *  be used.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

extern int FMQ_read_blocking (FMQ_handle_t *handle,
			      int msecs_sleep);

/***************************
 *  FMQ_read_type_blocking()
 *
 *  This function reads a message of a given type from an FMQ.
 *  It blocks until a message is received.
 *
 *  Parameters:
 *    msecs_sleep - number of millisecs to sleep between reads
 *                  while waiting for a message to arrive.
 *  If msecs_sleep is set to -1, a default value of 10 msecs will
 *  be used.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

extern int FMQ_read_type_blocking (FMQ_handle_t *handle,
				   int msecs_sleep,
				   unsigned int type);

/**************
 *  FMQ_write()
 *
 *  This function writes a message to an FMQ. 
 *
 *  Parameters:
 *    handle - FMQ handle
 *    msg - message array
 *    msg_len - message length in bytes
 *    msg_type - user-defined and used message type
 *    msg_subtype - user-defined and used message subtype
 *
 *  Note: the type and subtype are not used by the FMQ module but
 *        are passed through so the reading routine can determine
 *        something about the message from the header.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

extern int FMQ_write (FMQ_handle_t *handle, void *msg, int msg_len,
		      int msg_type, int msg_subtype);

/**********************************
 *  FMQ_write_precompressed()
 *
 *  This function writes a pre-compressed message to an FMQ. 
 *
 *  Provides file locking layer
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

extern int FMQ_write_precompressed(FMQ_handle_t *handle,
				   void *msg, int msg_len,
				   int msg_type, int msg_subtype,
				   int uncompressed_len);

/**********************
 * FMQ_fraction_used_()
 *
 * Computes the fraction of the available
 * space used in terms of slots and buffer.
 *
 * returns 0 on success, -1 on failure.
 */

extern int FMQ_fraction_used(FMQ_handle_t *handle,
			     double *slot_fraction_p,
			     double *buffer_fraction_p);

/**************
 *  FMQ_close()
 *
 *  Closes the FMQ status and buffer files
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

extern int FMQ_close (FMQ_handle_t *handle);

/**************
 *  FMQ_clear()
 *
 *  Clears the FMQ status and buffer files
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

extern int FMQ_clear (FMQ_handle_t *handle);

/********************
 *  FMQ_free()
 *
 *  Free memory associated with the FMQ
 *
 *  Returns 0 on success, -1 on error.
 *
 */

extern int FMQ_free(FMQ_handle_t *handle);

/******************************
 * Message info access routines
 *
 * Users should use these routines to access members of
 * the structs. Users should not use the structs directly,
 * since the implementation may change.
 */

extern void *FMQ_msg(FMQ_handle_t *handle); /* msg in handle */
extern int FMQ_msg_len(FMQ_handle_t *handle); /* msg_len in handle */
extern int FMQ_slot_stored_len(FMQ_handle_t *handle); /* stored len in slot */
extern int FMQ_slot_msg_len(FMQ_handle_t *handle); /* uncompressed len */
extern int FMQ_slot_compress(FMQ_handle_t *handle); /* is slot compressed? */
extern int FMQ_msg_id(FMQ_handle_t *handle); /* msg id in slot */
extern time_t FMQ_msg_time(FMQ_handle_t *handle); /* msg time in slot */
extern int FMQ_msg_type(FMQ_handle_t *handle); /* msg type in slot */
extern int FMQ_msg_subtype(FMQ_handle_t *handle); /* msg subtype in slot */
extern time_t FMQ_last_mod_time(FMQ_handle_t *handle);

/*******************
 *  FMQ_print_stat()
 *
 *  Prints out the status struct.
 *
 *  Returns 0 on success, -1 on failure.
 *
 */

extern int FMQ_print_stat(FMQ_handle_t *handle, FILE *out);

/************************
 *  FMQ_print_slot_read()
 *
 *  Prints out the latest slot read.
 */

extern int FMQ_print_slot_read(FMQ_handle_t *handle, FILE *out);

/***************************
 *  FMQ_print_slot_written()
 *
 *  Prints out the latest slot written.
 */

extern int FMQ_print_slot_written(FMQ_handle_t *handle, FILE *out);

/********************
 *  FMQ_print_debug()
 *
 *  Debugging printout - will work for invalid
 *  FMQ files.
 *
 *  Prints out the status and slot structs.
 *
 *  Opens and closes the files.
 *
 *  Returns 0 on success, -1 on failure.
 *
 */

extern int FMQ_print_debug(char *fmq_path,
			   char *prog_name,
			   FILE *out);

#ifdef __cplusplus
}
#endif

#endif

