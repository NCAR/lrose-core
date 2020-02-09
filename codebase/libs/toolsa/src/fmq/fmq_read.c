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
 * fmq_read.c
 *
 * File read routines for FMQ - private
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#include <toolsa/fmq_private.h>
#include <toolsa/uusleep.h>
#include <unistd.h>
#include <errno.h>

static int read_next(FMQ_handle_t *handle, int *msg_read);
     
static int read_msg(FMQ_handle_t *handle, int slot_num);

static int update_last_id_read (FMQ_handle_t *handle);

/******************
 * fmq_read() 
 *
 * Reads the next message from FMQ.
 *
 * If type is non-negative, read until the correct message type
 * is found. A type of -1 indicates all types are accepted.
 *
 * Sets msg_read TRUE if msg read, FALSE if not.
 * 
 * Returns 0 on success, -1 on failure.
 */

int fmq_read(FMQ_handle_t *handle, int *msg_read, int type)
     
{

  if (type < 0) {

    return (read_next(handle, msg_read));

  } else {

    while (read_next(handle, msg_read) == 0) {
      if (*msg_read == FALSE) {
	return 0;
      } else if (type == handle->fslot.type) {
	return 0;
      }
    }

  }

  return -1;

}

/****************************
 *  fmq_read_blocking()
 *
 *  This function reads a message from an FMQ - it blocks until
 *  a message is received. Registers with procmap while
 *  waiting.
 *
 *   Parameters:
 *
 *    msecs_sleep - number of millisecs to sleep between reads
 *                  while waiting for a message to arrive.
 *    If set to -1, default of 10 msecs will be used.
 *
 *    type - if type is non-negative, read until the correct message
 *           type is found.
 *           A type of -1 indicates all types are accepted.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int fmq_read_blocking (FMQ_handle_t *handle,
		       int msecs_sleep, int type)
     
{

  int forever = TRUE;
  int msg_read;

  while (forever) {

    if(read_next(handle, &msg_read)) {
    
      return -1; /* error */

    }
    
    if (msg_read) {

      /*
       * message read
       */

      if (type < 0) {

	/* accept all types */

	return 0;

      } else if (type == handle->fslot.type) {

	/* type is as requested */
	
	return 0;
	
      }

    } else {

      if (msecs_sleep < 0) {
	umsleep(10);
      } else if (msecs_sleep > 0) {
	umsleep(msecs_sleep);
      }
    
    } /* if (msg_read) */
      
    /*
     * no message of correct type - send heartbeat
     */
    
    if (handle->heartbeat_func != NULL) {
      handle->heartbeat_func("In FMQ_read_blocking()");
    }
    
  } /* while (forever) */

  return -1; /* to satisfy compiler */

}

/***********************
 *  fmq_seek_end()
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

int fmq_seek_end (FMQ_handle_t *handle)

{

  int last_slot_read;

  if (fmq_find_slot_for_id(handle,
                           handle->fstat.youngest_id,
                           &last_slot_read)) {
    fmq_print_error(handle, "fmq_seek_end",
                    "Cannot find slot for id: %d\n", handle->fstat.youngest_id);
    fmq_print_error(handle, "fmq_seek_end",
                    "Fmq: %s\n", handle->fmq_path);
    return -1;
  }

  handle->last_id_read = handle->fstat.youngest_id;
  handle->last_slot_read = last_slot_read;

  /*
   * in the case of blocking writes, update the last_id_read
   * in the file status block
   */

  if (update_last_id_read (handle)) {
    return -1;
  }

  return 0;

}

/**********************************************
 *  fmq_seek_last()
 *
 *  This positions for reading the last entry.
 *  Seek to the end of the FMQ, minus 1 slot.
 *
 *  Sets read pointers only - no affect on writes.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

int fmq_seek_last (FMQ_handle_t *handle)

{
  
  int last_slot_read;

  if (fmq_find_slot_for_id (handle,
			    handle->fstat.youngest_id,
			    &last_slot_read)) {
    fmq_print_error(handle, "fmq_seek_last",
                    "Cannot find slot for id: %d\n", handle->fstat.youngest_id);
    fmq_print_error(handle, "fmq_seek_last",
                    "Fmq: %s\n", handle->fmq_path);
    return -1;
  }

  handle->last_id_read = fmq_prev_id(handle->fstat.youngest_id);
  handle->last_slot_read = fmq_prev_slot(handle, last_slot_read);

  return 0;

}

/*************************
 *  fmq_seek_start()
 *
 *  Seek to the start of the FMQ.
 *
 *  Sets read pointers only - no affect on writes.
 *
 *  Set the read pointer to just before the oldest
 *  record, so that the entire buffer will be
 *  available for reads.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

int fmq_seek_start (FMQ_handle_t *handle)

{

  handle->last_slot_read = fmq_prev_slot(handle,
					 handle->fstat.oldest_slot);
  handle->last_id_read = -1;

  return 0;

}

/*************************
 *  fmq_seek_back()
 *
 *  Seek back by 1 entry.
 *
 *  Sets read pointers only - no affect on writes.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

int fmq_seek_back (FMQ_handle_t *handle)

{

  if (handle->last_id_read == -1) {
    return 0;
  }

  if (handle->last_id_read == 0) {
    handle->last_id_read = -1;
    handle->last_slot_read = -1;
    return 0;
  }
    
  handle->last_id_read = fmq_prev_id(handle->last_id_read);
  handle->last_slot_read = fmq_prev_slot(handle, handle->last_slot_read);

  return 0;

}

/*************************
 *  fmq_seek_to_id()
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

int fmq_seek_to_id (FMQ_handle_t *handle, int id)

{

  /*
   * get slot of requested ID
   */

  int slot;
  if (fmq_find_slot_for_id (handle, id, &slot)) {
    fmq_print_error(handle, "fmq_seek_to_id",
                    "Cannot find slot for id: %d\n", handle->fstat.youngest_id);
    fmq_print_error(handle, "fmq_seek_to_id",
                    "Fmq: %s\n", handle->fmq_path);
    return -1;
  }

  handle->last_id_read = id;
  handle->last_slot_read = slot;

  return 0;

}

     

/******************
 *  fmq_read_stat()
 *
 *  Reads in stat struct, store in handle.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int fmq_read_stat (FMQ_handle_t *handle)

{

  int ii;
  
  /*
   * try 5 times to read the status struct, using the checksum to
   * ensure it is correctly read
   */

  fmq_stat_t status;

  for (ii = 0; ii < 5; ii++) {

    /*
     * seek to start of status file
     */
    
    if (lseek(handle->stat_fd, 0, SEEK_SET) < 0) {
      fmq_print_error(handle, "fmq_read_stat",
                      "Cannot seek to start of status file\n");
      perror(handle->stat_path);
      return -1;
    }

    /*
     * read in status struct
     */
    
    if (fmq_read_with_retry(handle->stat_fd, &status, sizeof(fmq_stat_t)) !=
        sizeof(fmq_stat_t)) {
      fmq_print_error(handle, "fmq_read_stat",
                      "Cannot read status struct\n");
      perror(handle->stat_path);
      return -1;
    }

    /* swap */
    
    fmq_be_to_stat(&status);

    /* copy to handle */

    handle->fstat = status;
    
    /* checksum check */
    
    if (fmq_check_stat_checksum(&status) == 0) {
      return 0;
    }

  } /* ii */

  /*  checksum error */
  
  fmq_print_error(handle, "fmq_read_stat",
                  "stat checksum error\n"
                  "checksum is %d, should be %d\n",
                  status.checksum,
                  fmq_compute_stat_checksum(&status));
  fprintf(stderr, "WARNING - fmq_read_stat\n");
  fprintf(stderr, "  Could not resolve bad checksum, continuing anyway ...\n");
  fmq_print_stat(handle, stderr);

  return 0;
  
}

/*******************
 *  fmq_read_slots()
 *
 *  Reads in slot structs array
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int fmq_read_slots (FMQ_handle_t *handle)

{

  int nbytes;

  /*
   * seek to start of slots in status file
   */

  if (lseek(handle->stat_fd, sizeof(fmq_stat_t), SEEK_SET) < 0) {
    fmq_print_error(handle, "fmq_read_slots",
		    "Cannot seek to start of slots in status file\n");
    perror(handle->stat_path);
    return -1;
  }
  
  /*
   * read in slots, make sure there are enough slots allocated
   */

  fmq_alloc_slots(handle, handle->fstat.nslots);
  
  nbytes = sizeof(fmq_slot_t) * handle->fstat.nslots;
  if (fmq_read_with_retry(handle->stat_fd, handle->fslots, nbytes) != nbytes) {
    fmq_print_error(handle, "fmq_read_slots",
		    "Cannot read slots\n");
    perror(handle->stat_path);
    return -1;
  }

  /*
   * swap slot byte order
   */

  BE_to_array_32(handle->fslots, handle->fstat.nslots * sizeof(fmq_slot_t));

  return 0;

}

/******************
 *  fmq_read_slot()
 *
 *  Reads in slot struct.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int fmq_read_slot (FMQ_handle_t *handle, int slot_num)

{

  int offset, ii;
  fmq_slot_t *slot;

  /*
   * Make sure we have a valid slot number
   */

  if (slot_num >= handle->fstat.nslots) {
    fmq_print_error(handle, "fmq_read_slot",
                    "Invalid slot number %d, nslots = %d\n",
                    slot_num, handle->fstat.nslots);
    fmq_print_error(handle, "fmq_read_slot",
                    "Fmq: %s\n", handle->fmq_path);
    return -1;
  }
  
  fmq_alloc_slots(handle, handle->fstat.nslots);
  
  /*
   * try 5 times to read the slot struct, using the checksum to
   * ensure it is correctly read
   */

  for (ii = 0; ii < 5; ii++) {
    
    /*
     * seek to given slot
     */
    
    offset = sizeof(fmq_stat_t) + slot_num * sizeof(fmq_slot_t);
    if (lseek(handle->stat_fd, offset, SEEK_SET) < 0) {
      fmq_print_error(handle, "fmq_read_slot",
                      "Cannot seek to slot %d in status file\n",
                      slot_num);
      perror(handle->stat_path);
      return -1;
    }
    slot = handle->fslots + slot_num;

    /*
     * read in slot
     */
    
    
    if (fmq_read_with_retry(handle->stat_fd, slot, sizeof(fmq_slot_t)) !=
        sizeof(fmq_slot_t)) {
      fmq_print_error(handle, "fmq_read_slot",
                      "Cannot read slot %d in status file\n",
                      slot_num);
      perror(handle->stat_path);
      return -1;
    }
    
    /*
     * swap slot byte order
     */
    
    fmq_be_to_slot(slot);

    if (slot->checksum == 0) {
      continue;
    }
    
    /* checksum check */
    
    if (fmq_check_slot_checksum(slot) == 0) {
      return 0;
    }
    
  } /* ii */

  /* checksum error */
  
  fmq_print_error(handle, "fmq_read_slot",
                  "slot checksum error\n"
                  "checksum is %d, should be %d\n",
                  slot->checksum, fmq_compute_slot_checksum(slot));
  fprintf(stderr, "WARNING - fmq_read_slot\n");
  fprintf(stderr, "  Could not resolve bad checksum, continuing anyway ...\n");
  fmq_print_slot(slot_num, slot, stderr);

  return 0;

}

/*******************
 *  load_read_msg
 *
 *  Given a read message, load up the handle.
 *  This is used by the FmqMgrClient class, which
 *  reads from a socket and then loads up the handle.
 */

int fmq_load_read_msg (FMQ_handle_t *handle,
		       int msg_type,
		       int msg_subtype,
		       int msg_id,
		       time_t msg_time,
		       void *msg,
		       int stored_len,
		       int compressed,
		       int uncompressed_len)
     
{
  
  ui64 nfull;

  handle->fslot.type    = msg_type;
  handle->fslot.subtype = msg_subtype;
  handle->fslot.id      = msg_id;
  handle->fslot.time    = msg_time;

  if (handle->dmsg != NULL) {
    ufree(handle->dmsg);
    handle->dmsg = NULL;
  }

  if (compressed) {

    handle->dmsg = ta_decompress(msg, &nfull);
    
    if (handle->dmsg == NULL || nfull != uncompressed_len) {
      fmq_print_error(handle, "load_read_msg",
		      "Error on decompression, expected %d bytes, "
		      "got %d bytes\n", (int) uncompressed_len, (int) nfull);
      fmq_print_error(handle, "load_read_msg",
                      "Fmq: %s\n", handle->fmq_path);
      return -1;
    }

    handle->msg_len = uncompressed_len;
    handle->fslot.msg_len = uncompressed_len;
    handle->fslot.stored_len = stored_len;
    handle->msg =  handle->dmsg;
    
  } else {

    handle->msg_len = stored_len;
    handle->fslot.msg_len = stored_len;
    handle->fslot.stored_len = stored_len;
    handle->dmsg = umalloc(handle->msg_len);
    memcpy(handle->dmsg, msg, handle->msg_len);
    handle->msg =  handle->dmsg;
    
  }
  
  return 0;

}

/**************
 *  read_next()
 *
 *  Reads next message, if available
 *
 *  Sets msg_read TRUE if msg read, FALSE if not.
 * 
 *  Returns 0 on success, -1 on failure.
 */

static int read_next(FMQ_handle_t *handle, int *msg_read)
     
{
  static int num_calls = 0;
  
  int next_slot, slot_read;

  num_calls++;
  
  *msg_read = FALSE;

  if (fmq_read_stat(handle)) {
    return -1;
  }

  /*
   * special case - no messages written yet
   */
  
  if (handle->fstat.youngest_id == -1) {
    handle->last_id_read = -1;
    handle->last_slot_read = -1;
    return 0;
  }

  /*
   * check if a new message has been written since the
   * last read
   */

  if (handle->last_id_read == handle->fstat.youngest_id) {
    return 0;
  }
  
  /*
   * get msg for next logical slot
   */

  next_slot = fmq_next_slot(handle, handle->last_slot_read);

  if (fmq_read_msg_for_slot(handle, next_slot)) {

    /*
     * no valid message for the next logical slot, so the buffer has
     * probably overflowed. The best option is to move ahead to
     * the youngest slot and start reading from there
     */

    if (fmq_read_msg_for_slot(handle, handle->fstat.youngest_slot)) {
      return -1;
    }

    slot_read = handle->fstat.youngest_slot;

  } else {

    slot_read = next_slot;
    
  }

  /*
   * In blocked mode, if we have skipped data tell the user about it.
   */

  if (handle->fstat.blocking_write && handle->fslot.id >= 0) {
    int prev_id = fmq_prev_id(handle->fslot.id);
    if (prev_id != handle->last_id_read) {
      fprintf(stderr, "!!!!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!\n"
	      "Data was skipped even though the fmq is in blocking mode.\n"
	      "You should either increase the buffer size or decrease\n"
	      "the number of slots.\n"
	      "You must ensure that the slots wrap BEFORE the\n"
	      "buffer wraps.\n");
      fmq_print_error(handle, "read_next",
                      "Fmq: %s\n", handle->fmq_path);
      fmq_print_stat(handle, stderr);
      fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
  }
  
  handle->last_slot_read = slot_read;
  handle->last_id_read = handle->fslot.id;
  *msg_read = TRUE;

  /*
   * in the case of blocking writes, update the last_id_read
   * in the file status block
   */

  if (update_last_id_read (handle)) {
    return -1;
  }

  return 0;

}

/****************************************
 * fmq_read_msg_for_slot()
 *
 * Reads the message for the given slot.
 *
 * Returns 0 on success, -1 on failure.
 *
 */

int fmq_read_msg_for_slot(FMQ_handle_t *handle, int slot_num)

{

  int prev_id;
  fmq_slot_t *slot;

  if (fmq_read_slot(handle, slot_num)) {
    return -1;
  }

  slot = handle->fslots + slot_num;
  prev_id = fmq_prev_id(slot->id);

  if (slot->active &&
      (handle->last_id_read == -1 || prev_id == handle->last_id_read)) {

    /*
     * read in message
     */
    
    if (read_msg(handle, slot_num)) {
      /* failed */
      handle->last_slot_read = slot_num;
      handle->last_id_read = slot->id;
      return -1;
    }

  } else {

    handle->last_slot_read = slot_num;
    handle->last_id_read = -1;
    return -1;

  }

  return 0;

}

/*************
 *  read_msg()
 *
 *  Reads in msg for a given slot.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

static int read_msg (FMQ_handle_t *handle, int slot_num)

{

  fmq_slot_t *slot;
  int id_posn;
  ui64 nfull;
  si32 magic_cookie;
  si32 slot_num_check;
  si32 id_check;
  si32 *iptr;
  void *compressed_msg;
  
  /*
   * Make sure we have a valid slot number.
   */

  if (slot_num >= handle->fstat.nslots) {
    fmq_print_error(handle, "read_msg",
                    "Fmq: %s\n", handle->fmq_path);
    fmq_print_error(handle, "read_msg",
                    "Invalid slot number %d, nslots = %d\n",
                    slot_num, handle->fstat.nslots);
    return -1;
  }
  
  slot = handle->fslots + slot_num;

  /*
   * seek to start of message
   */

  if (lseek(handle->buf_fd, slot->offset, SEEK_SET) < 0) {
    fmq_print_error(handle, "fmq_read_msg",
		    "Cannot seek to msg in buf file.\n");
    return -1;
  }

  /*
   * alloc space for entry
   */

  fmq_alloc_entry(handle, slot->stored_len);
     
  /*
   * read in message
   */

  if (fmq_read_with_retry(handle->buf_fd, handle->entry, slot->stored_len) !=
      slot->stored_len) {
    fmq_print_error(handle, "read_msg",
                    "Fmq: %s\n", handle->fmq_path);
    fmq_print_error(handle, "read_msg",
		    "Cannot read message from buf file, "
		    "slot, len, offset: %d, %d, %d\n",
		    slot_num, slot->stored_len, slot->offset);
    return -1;
  }
  
  /*
   * check the magic cookie and slot number fields.
   *
   * Entry is comprised of following:
   *      si32         si32                             si32
   * --------------------------------------------------------
   * | magic cookie |  slot_num  | -- message -- | pad |  id  |
   * --------------------------------------------------------
   *
   * Pad is for si32 alignment.
   */

  /*
   * check magic cookie
   */
  
  iptr = handle->entry;

  magic_cookie = BE_to_si32(iptr[0]);

  if (magic_cookie != FMQ_MAGIC_BUF) {
    fmq_print_error(handle, "read_msg",
                    "Fmq: %s\n", handle->fmq_path);
    fmq_print_error(handle, "fmq_read_msg",
                    "Magic cookie not correct in message area, "
                    "slot_num, len, offset, magic_cookie, desired magic_cookie: "
                    "%d, %d, %d, %d, %d\n",
                    slot_num, slot->stored_len, slot->offset,
                    magic_cookie, FMQ_MAGIC_BUF);
    return -1;
  }

  /*
   * check slot_num
   */

  slot_num_check = BE_to_si32(iptr[1]);

  if (slot_num_check != slot_num) {
    fmq_print_error(handle, "read_msg",
                    "Fmq: %s\n", handle->fmq_path);
    fmq_print_error(handle, "fmq_read_msg",
                    "Start check slot_num not correct in message area, "
                    "len, offset: %d, %d, "
                    "expected slot_num %d, "
                    "slot_num in file %d\n",
                    slot->stored_len, slot->offset,
                    slot_num, slot_num_check);
    return -1;
  }

  /*
   * check id
   */

  id_posn = (slot->stored_len / sizeof(si32)) - 1;
  id_check = BE_to_si32(iptr[id_posn]);
  
  if (id_check != slot->id) {
    fmq_print_error(handle, "read_msg",
                    "Fmq: %s\n", handle->fmq_path);
    fmq_print_error(handle, "fmq_read_msg",
                    "End check id not correct in message area, "
                    "len, offset: %d, %d, "
                    "expected id %d, "
                    "id in file %d\n",
                    slot->stored_len, slot->offset,
                    slot->id, id_check);
    return -1;
  }

  /*
   * set msg pointer, uncompressing as necessary
   * If this is a server, decompression is not done becuse this is
   * done at the client end.
   */

  if (slot->compress) {
    if (handle->server) {
      /* leave data compressed for server to pass on */
      handle->msg = (void *) (iptr + 2);
      handle->msg_len = slot->stored_len - 2 * sizeof(si32);
    } else {
      /* uncompress the data */
      if (handle->dmsg != NULL) {
	ufree(handle->dmsg);
	handle->dmsg = NULL;
      }
      compressed_msg = (void *) (iptr + 2);
      handle->dmsg = ta_decompress(compressed_msg, &nfull);
      if (handle->dmsg == NULL || nfull != slot->msg_len) {
        fmq_print_error(handle, "read_msg",
                        "Fmq: %s\n", handle->fmq_path);
	fmq_print_error(handle, "read_msg",
			"Error on decompression, expected %d bytes, "
			"got %d bytes\n", (int) slot->msg_len, (int) nfull);
	return -1;
      }
      handle->msg =  handle->dmsg;
      handle->msg_len = slot->msg_len;
    }
  } else {
    /* data not compressed */
    handle->msg = (void *) (iptr + 2);
    handle->msg_len = slot->msg_len;
  }

  /*
   * set len and latest slot read.
   */

  handle->fslot = handle->fslots[slot_num];

  return 0;

}


/************************
 *  update_last_id_read()
 *
 *  Update the last_id_read in the status struct if the
 *  write mode is blocking.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

static int update_last_id_read (FMQ_handle_t *handle)

{
  
  FILE *stat_file;

  if (!handle->fstat.blocking_write) {
    return 0;
  }

  /*
   * in the case of blocking writes, update the last_id_read
   * in the file status block
   */
  
  handle->fstat.last_id_read = handle->last_id_read;

  if ((stat_file = fopen(handle->stat_path, "r+")) != NULL) {

    int stat_fd = fileno(stat_file);
    long offset = (char *) &handle->fstat.last_id_read -
      (char *) &handle->fstat.magic_cookie;

    if (lseek(stat_fd, offset, SEEK_SET) == offset) {

      si32 last_id_read = handle->last_id_read;
      BE_from_array_32(&last_id_read, sizeof(si32));

      if (fmq_write_with_retry(stat_fd, &last_id_read, sizeof(si32)) == sizeof(si32)) {
	fclose(stat_file);
	return 0;
      } /* write */

    } /* lseek */

    fclose(stat_file);

  } /* fopen */

  return -1;

}


/***************************************************************************
 * Reads with retries
 *
 * Returns nbytes actually read, -1 on error
 */

int fmq_read_with_retry(int fd, const void *mess, size_t len)

{
  
  long bytes_read;
  long target_len = len;
  char *ptr = (char *) mess;
  int retries = 100;
  int total = 0;
  int err_count = 0;
  
  while(target_len) {
    errno = 0;
    bytes_read = read(fd, ptr, target_len);
    if(bytes_read <= 0) {
      if (errno != EINTR) { /* system call was not interrupted */
	err_count++;
      }
      if(err_count >= retries) return total;
      /* Block for 1 millisecond */
      uusleep(1000);
    } else {
      err_count = 0;
    }
    if (bytes_read > 0) {
      target_len -= bytes_read;
      ptr += bytes_read;
      total += bytes_read;
    }
  }
  
  return total;
}

