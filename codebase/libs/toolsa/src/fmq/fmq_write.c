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
 * fmq_write.c
 *
 * File write routines for FMQ - private
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#include <toolsa/fmq_private.h>
#include <toolsa/compress.h>
#include <toolsa/uusleep.h>

#include <unistd.h>
#include <time.h>
#include <errno.h>

static int _fmq_write(FMQ_handle_t *handle, void *msg, int msg_len, 
		      int msg_type, int msg_subtype,
		      int pre_compressed, int uncompressed_len);

static int free_oldest_slot(FMQ_handle_t *handle);

static int space_avail(FMQ_handle_t *handle, int stored_len);

static int write_msg (FMQ_handle_t *handle, int write_slot, int write_id,
		      void *msg, int msg_len, int stored_len, int offset);
     
/********************
 *  fmq_write()
 *
 *  This function writes a message to an FMQ. 
 *
 *  Provides file locking layer
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

int fmq_write(FMQ_handle_t *handle,
	      void *msg, int msg_len,
	      int msg_type, int msg_subtype)
     
{

  int iret;

  if (fmq_lock_rdwr(handle) != 0) {
    fmq_print_error(handle, "fmq_write",
		    "Error locking file for read/write\n");
    fmq_print_error(handle, "fmq_write",
		    "Stat path: %s\n", handle->stat_path);
    return -1;
  }
  
  iret = _fmq_write(handle, msg, msg_len, msg_type, msg_subtype,
		    FALSE, msg_len);
  fmq_unlock(handle);
  
  return (iret);

}

/**********************************
 *  fmq_write_precompressed()
 *
 *  This function writes a pre-compressed message to an FMQ. 
 *
 *  Provides file locking layer
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

int fmq_write_precompressed(FMQ_handle_t *handle,
			    void *msg, int msg_len,
			    int msg_type, int msg_subtype,
			    int uncompressed_len)

{

  int iret;

  if (fmq_lock_rdwr(handle) != 0)
  {
    fmq_print_error(handle, "fmq_write_precompressed",
		    "Error locking file for read/write\n");
    fmq_print_error(handle, "fmq_write_precompressed",
		    "Stat path: %s\n", handle->stat_path);
    return -1;
  }
  
  iret = _fmq_write(handle, msg, msg_len, msg_type, msg_subtype,
		    TRUE, uncompressed_len);
  fmq_unlock(handle);
  
  return (iret);

}

/******************
 *  fmq_write_stat()
 *
 *  Writes out stat struct
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int fmq_write_stat (FMQ_handle_t *handle)

{

  fmq_stat_t stat;

  /*
   * make local copy of stat struct
   * set byte order to BigEnd
   */
  
  stat = handle->fstat;
  stat.time_written = time(NULL);
  fmq_add_stat_checksum(&stat);
  fmq_be_from_stat(&stat);

  /*
   * seek to start of stat file
   */
  
  if (lseek(handle->stat_fd, 0, SEEK_SET) < 0) {
    perror(handle->stat_path);
    fmq_print_error(handle, "fmq_write_stat",
		    "Cannot seek to start of file\n");
    return -1;
  }
  
  /*
   * write
   */
  
  if (fmq_write_with_retry(handle->stat_fd, &stat, sizeof(fmq_stat_t)) !=
      sizeof(fmq_stat_t)) {
    perror(handle->stat_path);
    fmq_print_error(handle, "fmq_write_stat",
		    "Cannot write stat info.\n");
    return -1;
  }

  return 0;

}

/*******************
 *  fmq_write_slot()
 *
 *  Writes out slot struct to stat file
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int fmq_write_slot (FMQ_handle_t *handle, int slot_num)

{

  int offset;
  fmq_slot_t slot;

  /*
   * Make sure we have a valid slot number
   */

  if (slot_num >= handle->fstat.nslots) {
    fmq_print_error(handle, "fmq_write_slot",
                    "Invalid slot number %d, nslots = %d\n",
                    slot_num, handle->fstat.nslots);
    fmq_print_error(handle, "fmq_write_slot",
		    "Stat path: %s\n", handle->stat_path);
    return -1;
  }
  
  if (slot_num >= handle->nslots_alloc) {
    fmq_print_error(handle, "fmq_write_slot",
                    "Too few slots allocated.  "
                    "allocated = %d, needed = %d\n",
                    handle->nslots_alloc, slot_num);
    fmq_print_error(handle, "fmq_write_slot",
		    "Stat path: %s\n", handle->stat_path);
    return -1;
  }
  
  /*
   * make local copy of slot struct
   * set byte order to BigEnd
   */
  
  slot = handle->fslots[slot_num];
  fmq_add_slot_checksum(&slot);
  fmq_be_from_slot(&slot);

  /*
   * seek to slot
   */

  offset = sizeof(fmq_stat_t) + slot_num * sizeof(fmq_slot_t);
  if (lseek(handle->stat_fd, offset, SEEK_SET) < 0) {
    perror(handle->stat_path);
    fmq_print_error(handle, "fmq_write_slot",
		    "Cannot seek to slot posn, offset %d.\n", offset);
    return -1;
  }

  /*
   * write slot
   */

  if (fmq_write_with_retry(handle->stat_fd, &slot, sizeof(fmq_slot_t)) !=
      sizeof(fmq_slot_t)) {
    perror(handle->stat_path);
    fmq_print_error(handle, "fmq_write_slot",
		    "Cannot write slot info, slot num %d.\n",
		    (int) slot_num);
    return -1;
  }
  
  return 0;

}

/**************
 *  _fmq_write()
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

static int _fmq_write(FMQ_handle_t *handle, void *msg, int msg_len, 
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
  fmq_slot_t *slot;
  
  /*
   * read in status struct 
   */
  
  if (fmq_read_stat(handle)) {
    return -1;
  }

  /*
   * compute the slot position for writing
   */
  
  write_slot = fmq_next_slot (handle, handle->fstat.youngest_slot);
  write_id = fmq_next_id(handle->fstat.youngest_id);
  
  /*
   * in blocking write operation, wait if we have caught up
   */
  
  if (handle->blocking_write) {
  
    int overwrite_id = write_id - handle->fstat.nslots;
    if (overwrite_id < 0) {
      overwrite_id += FMQ_MAX_ID;
    }

    while (overwrite_id == handle->fstat.last_id_read) {

      fmq_unlock(handle);
      if (handle->heartbeat_func != NULL) {
	handle->heartbeat_func("FMQ_write - blocked ...");
      }
      if (handle->debug) {
	fprintf(stderr, "FMQ_write - blocked ...\n");
      }
      umsleep(200);
      fmq_lock_rdwr(handle);
      if (fmq_read_stat(handle)) {
	return -1;
      }

    } /* while */

    handle->fstat.blocking_write = TRUE;

  } /* if (handle->blocking_write) */

  /*
   * if the write_slot is the same as the oldest slot, the slot
   * array is cycling faster than the buffer. Therefore, free
   * up the oldest slot
   */

  if (write_slot == handle->fstat.oldest_slot) {
    if (free_oldest_slot(handle)) {
      return -1;
    }
  }

  /*
   * compress if required
   */
  
  if (handle->compress && !pre_compressed && (msg != NULL)) {
    do_compress = TRUE;
  } else {
    do_compress = FALSE;
  }

  if (do_compress) {
    if ((cmsg = ta_compress(handle->compress_method,
			    msg, msg_len, &clen)) == NULL) {
      fmq_print_error(handle, "_fmq_write",
		      "Message compression failed.\n");
      fmq_print_error(handle, "_fmq_write",
                      "Fmq: %s\n", handle->fmq_path);
      return -1;
    }
  } else {
    clen = msg_len;
    cmsg = msg;
  }

  /*
   * compute padded length, to keep the total length aligned to
   * 32-bit words
   */
  
  nbytes_padded = (((clen - 1) / sizeof(si32)) + 1) * sizeof(si32);
  stored_len = nbytes_padded + FMQ_NBYTES_EXTRA;
  
  /*
   * check that message will fit in buffer
   */
  
  if (stored_len > handle->fstat.buf_size) {
    fmq_print_error(handle, "_fmq_write",
		    "Message size %d bytes too large for FMQ\n"
		    "Max msg len %d\n",
		    clen, handle->fstat.buf_size - FMQ_NBYTES_EXTRA);
    fmq_print_error(handle, "_fmq_write",
                    "Fmq: %s\n", handle->fmq_path);
    if (do_compress) {
      ta_compress_free(cmsg);
    }
    return -1;
  }

  /*
   * make space for message
   */

  while (1) {
    int avail = space_avail(handle, stored_len);
    if (avail < 0) {
      return -1;
    }
    if (avail) {
      break;
    }
  }
  
  if (fmq_alloc_slots(handle, handle->fstat.nslots)) {
    return -1;
  }

  /*
   * write the message
   */

  if (handle->fstat.append_mode) {
    offset = handle->fstat.begin_append;
  } else {
    offset = handle->fstat.begin_insert;
  }

  /* #define DEBUG_PRINT */
#ifdef DEBUG_PRINT
  fprintf(stderr, "bi, ei, ba, am, ln, ys, os, off: %9d %9d %9d %2d %8d %5d %5d %9d\n",
          handle->fstat.begin_insert,
          handle->fstat.end_insert,
          handle->fstat.begin_append,
          handle->fstat.append_mode,
          stored_len,
          handle->fstat.youngest_slot,
          handle->fstat.oldest_slot,
          offset);
#endif
  
  iret = write_msg(handle, write_slot, write_id, cmsg,
		   clen, stored_len, offset);
  if (do_compress) {
    ta_compress_free(cmsg);
  }
  if (iret) {
    return -1;
  }
  
  /*
   * load up slot info and write out
   */
  
  slot = handle->fslots + write_slot;
  slot->active = TRUE;
  slot->id = write_id;
  slot->time = time(NULL);
  slot->msg_len = uncompressed_len;
  slot->stored_len = stored_len;
  slot->offset = offset;
  slot->type = msg_type;
  slot->subtype = msg_subtype;

  if (msg != NULL) {
    slot->compress = handle->compress;
  } else {
    slot->compress = FALSE;
  }

  if (fmq_write_slot(handle, write_slot)) {
    slot->active = FALSE;
    return -1;
  }

  handle->fslot = handle->fslots[write_slot];

  /*
   * update status data and write out
   */

  handle->fstat.youngest_slot = write_slot;
  if (handle->fstat.oldest_slot == -1) {
    handle->fstat.oldest_slot = write_slot;
  }
  if (handle->fstat.append_mode) {
    handle->fstat.begin_append += stored_len;
  } else {
    handle->fstat.begin_insert += stored_len;
  }
  handle->fstat.youngest_id = write_id;
  
  if (fmq_write_stat(handle)) {
    return -1;
  }

  return 0;

}

/**************
 *  write_msg()
 *
 *  Writes out msg for a given slot.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

static int write_msg(FMQ_handle_t *handle, int write_slot, int write_id,
                     void *msg, int msg_len, int stored_len, int offset)
     
{

  int id_posn;
  si32 *iptr;
  si32 magic_cookie;
  si32 slot_num;
  si32 id;

  /*
   * seek to start of message
   */
  
  if (lseek(handle->buf_fd, offset, SEEK_SET) < 0) {
    perror(handle->buf_path);
    fmq_print_error(handle, "write_msg",
		    "Cannot seek to msg in buf file.\n");
    return -1;
  }

  /*
   * alloc space for entry
   */
  
  fmq_alloc_entry(handle, stored_len);

  /*
   * set values in entry
   */
     
  magic_cookie = BE_from_si32(FMQ_MAGIC_BUF);
  slot_num = BE_from_si32(write_slot);
  id = BE_from_si32(write_id);

  iptr = handle->entry;
  iptr[0] = magic_cookie;
  iptr[1] = slot_num;
  id_posn = (stored_len / sizeof(si32)) - 1;
  iptr[id_posn] = id;

  /*
   * copy the message in
   */

  memcpy(iptr + 2, msg, msg_len);

  /*
   * write out entry
   */
  
  if (fmq_write_with_retry(handle->buf_fd, handle->entry, stored_len) != stored_len) {
    perror(handle->buf_path);
    fmq_print_error(handle, "write_msg",
                    "Cannot write message to buf file, "
                    "slot_num, len, offset: %d, %d, %d\n",
                    write_slot, msg_len, offset);
    return -1;
  }
  
  return 0;

}

/**************************
 * Check space availability
 *
 * Returns TRUE if space available, FALSE otherwise if not,
 * -1 on error.
 *
 * Toggles between append and insert mode as applicable.
 * In append mode, the space to be used occurs after the
 * begin_append offset. In insert mode, the space to be
 * used occurs between the begin_insert and end_insert
 * offsets.
 */

static int space_avail(FMQ_handle_t *handle, int stored_len)

{

  int space;

  if (handle->fstat.append_mode) {

    space = handle->fstat.buf_size - handle->fstat.begin_append;

    if (space >= stored_len) {
      return TRUE;
    } else {
      handle->fstat.append_mode = FALSE;
      return FALSE;
    }

  } else {

    space = handle->fstat.end_insert - handle->fstat.begin_insert;

    if (space >= stored_len) {
      return TRUE;
    } else {
      if (free_oldest_slot(handle)) {
	return -1;
      }
      return FALSE;
    }

  } /* if (handle->fstat.append_mode) */

}

/********************
 * free_oldest_slot()
 *
 * Free up the oldest slot, and increase the free space in
 * the insert region.
 */

static int free_oldest_slot(FMQ_handle_t *handle)
     
{

  int oldest_slot = handle->fstat.oldest_slot;
  fmq_slot_t *oldest_ptr = handle->fslots + oldest_slot;

  /*
   * update oldest slot
   */

  fmq_read_slot (handle, oldest_slot);

  /*
   * consistency check
   */

  if (oldest_ptr->offset != handle->fstat.end_insert) {

    fprintf(stderr, "===============================\n");
    fmq_print_error(handle, "free_oldest_slot",
                    "Fmq: %s\n", handle->fmq_path);
    fmq_print_error(handle, "free_oldest_slot",
		    "Offset mismatch: end_insert %d, "
		    "oldset_slot offset %d\n",
		    handle->fstat.end_insert, oldest_ptr->offset);
    fprintf(stderr, "\n");
    fmq_print_stat(handle, stderr);
    fprintf(stderr, "\n");
    fmq_pretty_print_slot(oldest_slot, oldest_ptr, stderr);
    fprintf(stderr, "===============================\n");

    /*
     * re-initialize the queue
     */
    fmq_clear(handle);
    return -1;

  }

  handle->fstat.end_insert += oldest_ptr->stored_len;

  /*
   * Check whether the insert region has merged with the append
   * region. If so, merge both into the append region, and
   * set the insert region to length 0 at the start of the file.
   */

  if (handle->fstat.end_insert >= handle->fstat.begin_append) {

    handle->fstat.begin_append = handle->fstat.begin_insert;
    handle->fstat.begin_insert = 0;
    handle->fstat.end_insert = 0;
    handle->fstat.append_mode = TRUE;
    
  }

  handle->fstat.oldest_slot =
    fmq_next_slot(handle, handle->fstat.oldest_slot);

  /*
   * Zero out the slot and save it to file.
   */
  
  MEM_zero(*oldest_ptr);
  if (fmq_write_slot(handle, oldest_slot)) {
    fmq_print_error(handle, "free_oldest_slot",
                    "Fmq: %s\n", handle->fmq_path);
    fmq_print_error(handle, "free_oldest_slot",
		    "Cannot write slot %d\n", oldest_slot);
  }

  return 0;

}

/*************************************************************************
 * write with retries
 *
 * returns number of bytes written, -1 on error
 */

int fmq_write_with_retry(int fd, const void *mess, size_t len)

{

  int bytes_written;
  int target_len = len;
  char *ptr = (char *) mess;
  int retries = 100;
  int total = 0;
  int err_count = 0;
  
  while(target_len > 0) {
    
    errno = 0;
    bytes_written = write(fd,ptr,target_len);

    if(bytes_written <= 0) {
      
      if (errno != EINTR) { /* system call was not interrupted */
	err_count++;
      }
      
      if(err_count >= retries) {
	return total;
      }

      /*
       * Block for 1 millisecond
       */

      uusleep(1000);
      
    } else {

      err_count = 0;
      
    }
    
    if (bytes_written > 0) {
      target_len -= bytes_written;
      ptr += bytes_written;
      total += bytes_written;
    }

  }
  
  return (total);

}

