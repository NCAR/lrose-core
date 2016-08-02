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
 * fmq_file.c
 *
 * File operations for FMQ - private
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

/* ifdef below is to get fileno() prototype */

#include <toolsa/fmq_private.h>
#include <toolsa/file_io.h>
#include <toolsa/uusleep.h>
#include <toolsa/umisc.h>

static int close_files(FMQ_handle_t *handle);

static void free_file_mem(FMQ_handle_t *handle);

static int init_files(FMQ_handle_t *handle,
		      int nslots, int buf_size);
     
/**************************
 *  fmq_open_create()
 *
 *  Creates the FMQ by opening in mode "w+".
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_open_create(FMQ_handle_t *handle,
		    int nslots, int buf_size)
     
{

  handle->write = TRUE;

  /*
   * open, creating the files write/read
   */
    
  if (fmq_open(handle, "w+")) {
    return -1;
  }

  /*
   * initialize files
   */
  
  fmq_lock_rdwr(handle);
  init_files(handle, nslots, buf_size);
  fmq_unlock(handle);

  return 0;

}

/************************
 *  fmq_open_rdwr()
 *
 *  If FMQ does not exist, creates it by opening in mode "w+"
 *  If FMQ exists and is valid, opens in mode "r+".
 *  Otherwise, returns error.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_open_rdwr(FMQ_handle_t *handle,
		  int nslots, int buf_size)
     
{

  /*
   * if files do not exist, do create instead
   */

  if (!fmq_exist(handle)) {
    return (fmq_open_create(handle, nslots, buf_size));
  }

  /*
   * open the files
   */

  if (fmq_open(handle, "r+")) {
    return -1;
  }

  /*
   * check the fmq file sizes
   * Side effect - reads in slots array and stats
   */
  
  if (!fmq_check_file_sizes(handle)) {
    return -1;
  }

  handle->write = TRUE;

  return 0;

}

/*****************************
 *  fmq_open_rdwr_nocreate()
 *
 *  If FMQ exists and is valid, opens the files mode "r+".
 *  Otherwise, returns error.
 *
 *  Used for reading in blocking operations.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_open_rdwr_nocreate(FMQ_handle_t *handle)

{

  handle->write = FALSE;
  
  if (!fmq_exist(handle)) {
    return -1;
  }

  /*
   * open the files
   */

  if (fmq_open(handle, "r+")) {
    return -1;
  }

  /*
   * read in status
   */

  if (fmq_read_stat(handle)) {
    return -1;
  }

  /*
   * set last_slot_read to 1 less than oldest_slot
   */

  handle->last_slot_read = fmq_prev_slot(handle, handle->fstat.oldest_slot);
  
  return 0;

}

/**************************
 *  fmq_open_rdonly()
 *
 *  If FMQ exists and is valid, opens the files mode "r".
 *  Otherwise, returns error.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_open_rdonly (FMQ_handle_t *handle)

{

  handle->write = FALSE;
  
  if (!fmq_exist(handle)) {
    return -1;
  }

  /*
   * open the files
   */

  if (fmq_open(handle, "r")) {
    return -1;
  }

  /*
   * read in status
   */

  if (fmq_read_stat(handle)) {
    return -1;
  }

  /*
   * set last_slot_read to 1 less than oldest_slot
   */

  handle->last_slot_read = fmq_prev_slot(handle, handle->fstat.oldest_slot);
  
  return 0;

}

/****************************
 *  fmq_open_blocking()
 *
 *  If valid FMQ does not exist, waits until it does,
 *  and then opens opens the files mode "r", i.e. rdonly.
 * 
 *  While waiting, calls heartbeat function if required.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_open_blocking(FMQ_handle_t *handle, int msecs_sleep)

{
  
  int forever = TRUE;

  handle->write = FALSE;

  while (forever) {
    
    if (fmq_open_rdonly (handle)) {

      if (handle->heartbeat_func != NULL) {
	handle->heartbeat_func("In FMQ_open_blocking()");
      }

      /*
       * no FMQ yet - sleep as requested
       */
      
      if (msecs_sleep < 0) {
	umsleep(1000);
      } else if (msecs_sleep > 0) {
	umsleep(msecs_sleep);
      }

    } else {

      return 0;

    }

  } /* while (forever) */
  
  /*
   * never reach this return - only for compiler warnings
   */
  
  return -1;

}

/****************************
 *  fmq_open_blocking_rdwr()
 *
 *  If valid FMQ does not exist, waits until it does,
 *  and then opens opens the files mode "r+", i.e. read-write.
 * 
 *  While waiting, calls heartbeat function if required.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_open_blocking_rdwr(FMQ_handle_t *handle, int msecs_sleep)

{
  
  int forever = TRUE;

  handle->write = FALSE;
  
  while (forever) {
    
    if (fmq_open_rdwr_nocreate(handle)) {
      
      if (handle->heartbeat_func != NULL) {
	handle->heartbeat_func("In FMQ_open_blocking()");
      }

      /*
       * no FMQ yet - sleep 1 sec
       */

      if (msecs_sleep < 0) {
	umsleep(1000);
      } else if (msecs_sleep > 0) {
	umsleep(msecs_sleep);
      }

    } else {

      return 0;

    }

  } /* while (forever) */
  
  /*
   * never reach this return - only for compiler warnings
   */
  
  return -1;

}

/**************
 *  fmq_open ()
 *
 *  Opens the FMQ status and buffer files
 *
 *  Parameters:
 *    handle - FMQ handle
 *    fmq_path - name of the message buffer
 *    mode - uses fopen() style modes.
 *           Valid modes:
 *             "r"  - read only
 *             "r+" - read/write
 *             "w+" - write/read, creates if non-existent
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_open (FMQ_handle_t *handle, char *mode)

{

  /*
   * close files if already open
   */

  if (handle->stat_file != NULL) {
    fclose(handle->stat_file);
    handle->stat_file = NULL;
  }

  if (handle->buf_file != NULL) {
    fclose(handle->buf_file);
    handle->buf_file = NULL;
  }

  /*
   * in "w+" mode, create the directory if needed
   */

  if (!strcmp(mode, "w+")) {
    int iret;
    path_parts_t parts;
    uparse_path(handle->stat_path, &parts);
    iret = ta_makedir_recurse(parts.dir);
    ufree_parsed_path(&parts);
    if (iret) {
      fmq_print_error(handle, "fmq_open",
		      "Cannot create directory, mode '%s'\n", mode);
      perror(handle->stat_path);
      return -1;
    }
  }

  /*
   * Open the stat file
   */
  
  if ((handle->stat_file = fopen(handle->stat_path, mode)) == NULL) {
    fmq_print_error(handle, "fmq_open",
		    "Cannot open stat file, mode '%s'\n", mode);
    perror(handle->stat_path);
    return -1;
  }

  /*
   * get file descriptor for low-level read/writes
   */

  handle->stat_fd = fileno(handle->stat_file);

  /*
   * Open the buf file
   */
  
  if ((handle->buf_file = fopen(handle->buf_path, mode)) == NULL) {
    fmq_print_error(handle, "fmq_open",
		    "Cannot open buf file, mode '%s'\n", mode);
    perror(handle->buf_path);
    return -1;
  }

  /*
   * get file descriptor for low-level read/writes
   */

  handle->buf_fd = fileno(handle->buf_file);

  return 0;

}

/********************
 *  fmq_close()
 *
 *  Close the FMQ status and buffer files.
 *  Free up memory associated with the files. 
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_close(FMQ_handle_t *handle)

{

  free_file_mem(handle);
  return (close_files(handle));

}

/********************
 *  fmq_clear()
 *
 *  Clear the the FMQ status and buffer files.
 *  Free up memory associated with the files. 
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_clear(FMQ_handle_t *handle)

{

  free_file_mem(handle);
  
  fmq_lock_rdwr(handle);

  if (init_files(handle,
                 handle->fstat.nslots,
                 handle->fstat.buf_size)) {
    fmq_unlock(handle);
    return -1;
  }

  fmq_unlock(handle);

  return 0;

}

/**********************
 *  free_file_mem()
 *
 *  Free up memory associated with the files. 
 *
 */

static void free_file_mem(FMQ_handle_t *handle)

{

  fmq_free_slots(handle);
  fmq_free_entry(handle);

}

/****************
 *  close_files()
 *
 *  Closes the FMQ status and buffer files
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

static int close_files(FMQ_handle_t *handle)

{

  int iret = 0;

  /*
   * close stat file
   */
  
  if (handle->stat_file != NULL) {
    if (fclose(handle->stat_file)) {
      fmq_print_error(handle, "FMQ_close",
		      "Cannot close FMQ stat file\n");
      perror(handle->stat_path);
      iret = -1;
    }
    handle->stat_file = NULL;
  }

  /*
   * close buf file
   */

  if (handle->buf_file != NULL) {
    if (fclose(handle->buf_file)) {
      fmq_print_error(handle, "FMQ_close",
		      "Cannot close FMQ buf file\n");
      perror(handle->buf_path);
      iret = -1;
    }
    handle->buf_file = NULL;
  }

  return (iret);

}

/****************
 * fmq_init_buf()
 *
 * Initialize buffer variables in status struct
 */
 
void fmq_init_buf(FMQ_handle_t *handle)

{

  handle->fstat.youngest_slot = -1;
  handle->fstat.oldest_slot = -1;
  handle->fstat.begin_insert = 0;
  handle->fstat.end_insert = 0;
  handle->fstat.begin_append = 0;
  handle->fstat.append_mode = TRUE;

}

/*******************
 *  init_files()
 *
 *  Writes arrays and buffers to files.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

static int init_files (FMQ_handle_t *handle,
		       int nslots, int buf_size)
     

{

  si08 last_byte;
  int islot;
  si32 magic_cookie;

  /*
   * allocate status and slots memory
   */

  if (fmq_alloc_slots(handle, nslots)) {
    return -1;
  }

  /*
   * initialize stat struct and handle
   */

  MEM_zero(handle->fstat);
  handle->fstat.magic_cookie = FMQ_MAGIC_STAT;
  handle->fstat.youngest_id = -1;
  handle->fstat.youngest_slot = -1;
  handle->fstat.oldest_slot = -1;
  handle->fstat.nslots = nslots;
  handle->fstat.buf_size = buf_size;
  handle->fstat.begin_insert = 0;
  handle->fstat.end_insert = 0;
  handle->fstat.begin_append = 0;
  handle->fstat.append_mode = TRUE;

  handle->last_id_read = -1;
  handle->last_slot_read = -1;
  handle->last_slot_written = -1;

  /*
   * initialize slots
   */
  
  memset(handle->fslots, 0, nslots * sizeof(fmq_slot_t));

  /*
   * seek to start of buf file
   */

  if (lseek(handle->buf_fd, 0, SEEK_SET) < 0) {
    fmq_print_error(handle, "init_files",
		    "Cannot seek to start of buf file\n");
    perror(handle->buf_path);
    return -1;
  }

  /*
   * write magic cookie
   */

  magic_cookie = BE_from_si32(FMQ_MAGIC_BUF);
  if (fmq_write_with_retry(handle->buf_fd, &magic_cookie, sizeof(si32)) !=
      sizeof(si32)) {
    fmq_print_error(handle, "init_files",
		    "Cannot write magic cookie at start of buf file\n");
    perror(handle->buf_path);
    return -1;
  }
  
  /*
   * seek to 1 byte from end of file
   */

  if (lseek(handle->buf_fd, buf_size - 1, SEEK_SET) < 0) {
    fmq_print_error(handle, "init_files",
		    "Cannot seek to end of buf file\n");
    perror(handle->buf_path);
    return -1;
  }

  /*
   * write byte at end of file to set file size
   */

  last_byte = -1;
  if (fmq_write_with_retry(handle->buf_fd, &last_byte, sizeof(si08)) !=
      sizeof(si08)) {
    fmq_print_error(handle, "init_files",
		    "Cannot write byte at end of buf file\n");
    perror(handle->buf_path);
    return -1;
  }

  /*
   * write out slots and status
   */

  for (islot = 0; islot < nslots; islot++) {
    if (fmq_write_slot(handle, islot)) {
      fmq_print_error(handle, "init_files",
		      "Cannot write slot struct %d\n", islot);
      perror(handle->stat_path);
      return -1;
    }
  } /* islot */

  if (fmq_write_stat(handle)) {
    fmq_print_error(handle, "init_files",
		    "Cannot write stat struct\n");
    perror(handle->stat_path);
    return -1;
  }
  
  return 0;

}

/********************
 *  fmq_lock_rdonly()
 *
 *  Locks the stat file for reading - blocks until file
 *  available for reading.
 *
 *  Call heartbeat function while waiting, if non-NULL.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

int fmq_lock_rdonly (FMQ_handle_t *handle)

{

  if (ta_lock_file_heartbeat(handle->stat_path,
                             handle->stat_file, "r",
                             handle->heartbeat_func)) {
    return -1;
  } else {
    return 0;
  }

}

/******************
 *  fmq_lock_rdwr()
 *
 *  Locks the stat file for reading - blocks until file
 *  available for writing and reading.
 *
 *  Call heartbeat function while waiting, if non-NULL.
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

int fmq_lock_rdwr (FMQ_handle_t *handle)
     
{

  if (ta_lock_file_heartbeat(handle->stat_path,
                             handle->stat_file, "w",
                             handle->heartbeat_func)) {
    return -1;
  } else {
    return 0;
  }

}

/***************
 *  fmq_unlock()
 *
 *  Unlocks the stat file
 *
 *  Return value:
 *    0 on success, -1 on error.
 */

int fmq_unlock (FMQ_handle_t *handle)

{

  if (ta_unlock_file(handle->stat_path,
		     handle->stat_file)) {
    return -1;
  } else {
    return 0;
  }

}


