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
/******************************************************************
 * fmq.c
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

#include <toolsa/fmq_private.h>

/*************
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

int FMQ_init(FMQ_handle_t *handle,
	     const char *fmq_path,
	     int debug,
	     const char *prog_name)

{
  
  return (fmq_init_handle(handle, fmq_path, debug, prog_name));

}

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

int FMQ_set_heartbeat(FMQ_handle_t *handle,
		      TA_heartbeat_t heartbeat_func)

{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_set_heartbeat\n");
    return -1;
  }
  
  fmq_set_heartbeat(handle, heartbeat_func);
  return 0;

}

/********************
 * FMQ_set_server()
 *
 * Set server flag on.
 *
 * Returns 0 on success, -1 on error.
 */

int FMQ_set_server(FMQ_handle_t *handle)

{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_set_server\n");
    return -1;
  }

  fmq_set_server(handle);
  return 0;

}

/********************
 * FMQ_set_compress()
 *
 * Set compression on.
 *
 * Returns 0 on success, -1 on error.
 */

int FMQ_set_compress(FMQ_handle_t *handle)

{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_set_compress\n");
    return -1;
  }

  fmq_set_compress(handle);
  return 0;

}

/******************************
 * FMQ_set_compression_method()
 *
 * Set compression on.
 *
 * Returns 0 on success, -1 on error.
 */

int FMQ_set_compression_method(FMQ_handle_t *handle,
			       ta_compression_method_t method)

{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_set_compression_method\n");
    return -1;
  }

  fmq_set_compression_method(handle, method);
  return 0;

}

/**************************
 * FMQ_set_blocking_write()
 *
 * Set blocking_write on.
 *
 * Returns 0 on success, -1 on error.
 */

int FMQ_set_blocking_write(FMQ_handle_t *handle)

{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_set_blocking_write\n");
    return -1;
  }

  fmq_set_blocking_write(handle);
  return 0;

}

/********************
 *  FMQ_open_create()
 *
 *  Creates the FMQ by opening in mode "w+".
 *  Overwrites any existing FMQ.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int FMQ_open_create(FMQ_handle_t *handle,
		    int nslots, int buf_size)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_open_create\n");
    return -1;
  }

  return (fmq_open_create(handle, nslots, buf_size));

}

/******************
 *  FMQ_open_rdwr()
 *
 *  If FMQ exists and is valid, opens in mode "r+".
 *  Otherwise, creates the FMQ by opening in mode "w+".
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int FMQ_open_rdwr(FMQ_handle_t *handle,
		  int nslots, int buf_size)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_open_rdwr\n");
    return -1;
  }

  return (fmq_open_rdwr(handle, nslots, buf_size));

}

/***************************
 *  FMQ_open_rdwr_nocreate()
 *
 *  If FMQ exists and is valid, opens in mode "r+".
 *  Otherwise, returns error.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int FMQ_open_rdwr_nocreate(FMQ_handle_t *handle)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_open_rdwr_nocreate\n");
    return -1;
  }

  return (fmq_open_rdwr_nocreate(handle));

}

/********************
 *  FMQ_open_rdonly()
 *
 *  If FMQ exists and is valid, opens the files mode "r".
 *  Otherwise, returns error.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int FMQ_open_rdonly (FMQ_handle_t *handle)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_open_rdonly\n");
    return -1;
  }

  return (fmq_open_rdonly(handle));

}

/**********************
 *  FMQ_open_blocking()
 *
 *  If valid FMQ does not exist, waits until it does,
 *  and then opens opens the files mode "r", i.e. rdonly.
 * 
 *  While waiting, call heartbeat function if non-NULL
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int FMQ_open_blocking (FMQ_handle_t *handle)
     
{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_open_blocking\n");
    return -1;
  }

  return (fmq_open_blocking(handle, -1));

}

/*************************
 *  FMQ_open_blocking_rdwr()
 *
 *  If valid FMQ does not exist, waits until it does,
 *  and then opens opens the files mode "r+", i.e. rdwr
 * 
 *  While waiting, calls heartbeat function if non-NULL
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int FMQ_open_blocking_rdwr (FMQ_handle_t *handle)
     
{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_open_blocking_rdwr\n");
    return -1;
  }

  return (fmq_open_blocking_rdwr(handle, -1));

}

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

int FMQ_seek_end (FMQ_handle_t *handle)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_seek_end\n");
    return -1;
  }

  return (fmq_seek_end(handle));

}

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

int FMQ_seek_last (FMQ_handle_t *handle)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_seek_last\n");
    return -1;
  }

  return (fmq_seek_last(handle));

}

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

int FMQ_seek_start (FMQ_handle_t *handle)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_seek_start\n");
    return -1;
  }

  return (fmq_seek_start(handle));

}

/*******************
 *  FMQ_seek_back()
 *
 *  Seek back by 1 entry.
 *
 *  Sets read pointers only - no affect on writes.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

int FMQ_seek_back (FMQ_handle_t *handle)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_seek_back\n");
    return -1;
  }

  return (fmq_seek_back(handle));

}

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

int FMQ_seek_to_id (FMQ_handle_t *handle, int id)

{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_seek_to_id\n");
    return -1;
  }

  return (fmq_seek_to_id(handle, id));

}

/*************
 *  FMQ_read()
 *
 *  Reads the next message from FMQ.
 *
 *  Sets msg_read TRUE if msg read, FALSE if not.
 * 
 *  Returns 0 on success, -1 on failure.
 */

int FMQ_read (FMQ_handle_t *handle, int *msg_read)
     
{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_read\n");
    return -1;
  }

  return (fmq_read(handle, msg_read, -1));

}

/**********************
 *  FMQ_load_read_msg()
 *
 *  Load the given message into the FMQ handle.
 *
 *  Returns 0 on success, -1 on failure.
 */

int FMQ_load_read_msg (FMQ_handle_t *handle,
		       int msg_type,
		       int msg_subtype,
		       int msg_id,
		       time_t msg_time,
		       void *msg,
		       int stored_len,
		       int compressed,
		       int uncompressed_len)
     
{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_load_read_msg\n");
    return -1;
  }

  return (fmq_load_read_msg(handle, msg_type, msg_subtype,
			    msg_id, msg_time, msg,
			    stored_len, compressed, uncompressed_len));

}

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

int FMQ_read_type (FMQ_handle_t *handle, int *msg_read,
		   unsigned int type)
     
{
  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_read_type\n");
    return -1;
  }

  return (fmq_read(handle, msg_read, (int) type));

}

/**********************
 *  FMQ_read_blocking()
 *
 *  This function reads a message of the specific type from an FMQ.
 *  It blocks until a message is received.
 *  Note: type must be non-negative.
 *
 *  Parameters:
 *    msecs_sleep - number of millisecs to sleep between reads
 *                  while waiting for a message to arrive.
 *
 *  If set to -1, default of 10 msecs will be used.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int FMQ_read_blocking (FMQ_handle_t *handle,
		       int msecs_sleep)

{

  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_read_blocking\n");
    return -1;
  }

  return (fmq_read_blocking(handle, msecs_sleep, -1));
  
}

/***************************
 *  FMQ_read_type_blocking()
 *
 *  This function reads a message of a given type from an FMQ.
 *  It blocks until a message is received.
 *
 *  Parameters:
 *    msecs_sleep - number of millisecs to sleep between reads
 *                  while waiting for a message to arrive.
 *
 *  If set to -1, default of 10 msecs will be used.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int FMQ_read_type_blocking (FMQ_handle_t *handle,
			    int msecs_sleep,
			    unsigned int type)

{

  
  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_read_type_blocking\n");
    return -1;
  }

  return (fmq_read_blocking(handle, msecs_sleep, (int) type));
  
}

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

int FMQ_write (FMQ_handle_t *handle, void *msg, int msg_len,
	       int msg_type, int msg_subtype)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_write\n");
    return -1;
  }

  return (fmq_write(handle, msg, msg_len, msg_type, msg_subtype));
  
}

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

int FMQ_write_precompressed(FMQ_handle_t *handle,
			    void *msg, int msg_len,
			    int msg_type, int msg_subtype,
			    int uncompressed_len)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_write_precompressed\n");
    return -1;
  }

  return (fmq_write_precompressed(handle, msg, msg_len,
				  msg_type, msg_subtype, uncompressed_len));
  
}

/**************************
 *  fmq_check_and_recover()
 *
 *  Checks that the FMQ files are valid, performs recovery
 *  if possible.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int FMQ_check_and_recover(FMQ_handle_t *handle)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_check_and_recover\n");
    return -1;
  }

  return (fmq_check_and_recover(handle));

}

/*********************
 * FMQ_fraction_used()
 *
 * Computes the fraction of the available
 * space used in terms of slots and buffer.
 *
 * returns 0 on success, -1 on failure.
 */

int FMQ_fraction_used(FMQ_handle_t *handle,
		      double *slot_fraction_p,
		      double *buffer_fraction_p)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_fraction_used\n");
    return -1;
  }

  return (fmq_fraction_used(handle,
			    slot_fraction_p,
			    buffer_fraction_p));

}

/**************
 *  FMQ_close()
 *
 *  Closes the FMQ status and buffer files
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int FMQ_close (FMQ_handle_t *handle)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_close\n");
    return -1;
  }

  return (fmq_close(handle));

}

/**************
 *  FMQ_clear()
 *
 *  Clears the FMQ status and buffer files
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int FMQ_clear (FMQ_handle_t *handle)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_clear\n");
    return -1;
  }

  return (fmq_clear(handle));

}

/********************
 *  FMQ_free()
 *
 *  Free memory associated with the FMQ
 *
 *  Returns 0 on success, -1 on error.
 *
 */

int FMQ_free(FMQ_handle_t *handle)

{

  int iret;
  
  if (!fmq_check_init_no_error_message(handle)) {
    return -1;
  }

  iret = fmq_close(handle);
  
  fmq_free_handle(handle);

  return (iret);

}

/******************************
 * Message info access routines
 *
 * Users should use these routines to access members of
 * the structs. Users should not use the structs directly,
 * since the implementation may change.
 */

/* msg pointer */
void *FMQ_msg(FMQ_handle_t *handle)
{
  return (handle->msg);
}

/* msg length */
int FMQ_msg_len(FMQ_handle_t *handle)
{
  return (handle->msg_len);
}

/* stored length in slot */
int FMQ_slot_stored_len(FMQ_handle_t *handle)
{
  return (handle->fslot.stored_len);
}

/* message length in slot - uncompressed len */
int FMQ_slot_msg_len(FMQ_handle_t *handle)
{
  return (handle->fslot.msg_len);
}

/* slot compressed? */
int FMQ_slot_compress(FMQ_handle_t *handle)
{
  return (handle->fslot.compress);
}

/* msg id */
int FMQ_msg_id(FMQ_handle_t *handle)
{
  return (handle->fslot.id);
}

/* msg time */
time_t FMQ_msg_time(FMQ_handle_t *handle)
{
  return (handle->fslot.time);
}

/* msg type */
int FMQ_msg_type(FMQ_handle_t *handle)
{
  return (handle->fslot.type);
}

/* msg subtype */
int FMQ_msg_subtype(FMQ_handle_t *handle)
{
  return (handle->fslot.subtype);
}

/* last modification time */
time_t FMQ_last_mod_time(FMQ_handle_t *handle)
{
  return (handle->fstat.time_written);
}

/*******************
 *  FMQ_print_stat()
 *
 *  Prints out the status struct.
 *
 *  Returns 0 on success, -1 on failure.
 *
 */

int FMQ_print_stat(FMQ_handle_t *handle, FILE *out)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_print_stat\n");
    return -1;
  }

  return (fmq_print_stat(handle, out));

}

/************************
 *  FMQ_print_slot_read()
 *
 *  Prints out the latest slot read.
 */

int FMQ_print_slot_read(FMQ_handle_t *handle, FILE *out)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_print_slot_read\n");
    return -1;
  }

  return (fmq_print_slot_read(handle, out));


}

/***************************
 *  FMQ_print_slot_written()
 *
 *  Prints out the latest slot written.
 */

int FMQ_print_slot_written(FMQ_handle_t *handle, FILE *out)

{

  if (!fmq_check_init(handle)) {
    fprintf(stderr, "  FMQ_print_slot_written\n");
    return -1;
  }

  return (fmq_print_slot_written(handle, out));

}

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

int FMQ_print_debug(char *fmq_path, char *prog_name, FILE *out)

{
  return (fmq_print_debug(fmq_path, prog_name, out));
}





