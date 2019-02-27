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
 * fmq_check.c
 *
 * Checking routines for FMQ
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#include <toolsa/fmq_private.h>
#include <sys/types.h>
#include <sys/stat.h>

static int fmq_recover (FMQ_handle_t *handle);

/**************************************
 *  fmq_exist()
 *
 *  Checks that the FMQ files exist.
 *
 *  Return value:
 *    TRUE on success, FALSE on failure
 */

int fmq_exist (FMQ_handle_t *handle)

{

  struct stat file_stat;
  
  /*
   * check status file
   */

  if (stat(handle->stat_path, &file_stat)) {
    return FALSE;
  }

  /*
   * check buf file
   */

  if (stat(handle->buf_path, &file_stat)) {
    return FALSE;
  }

  return TRUE;

}

/********************************************
 *  fmq_check_file_sizes()
 *
 *  Checks that the FMQ files are valid sizes.
 *  Also reads in the slots array.
 *
 *  Assumes the files are already open.
 *
 *  Return value:
 *    TRUE on success, FALSE on failure
 */

int fmq_check_file_sizes (FMQ_handle_t *handle)

{

  int stat_file_size;
  int buf_file_size;

  struct stat file_stat;

  /*
   * read in status struct
   */

  if (fmq_read_stat(handle)) {
    fmq_print_error(handle, "fmq_check_file_sizes",
                    "  Fmq: %s\n", handle->fmq_path);
    return FALSE;
  }

  /*
   * compute sizes of files
   */

  stat_file_size = (sizeof(fmq_stat_t) +
		    handle->fstat.nslots * sizeof(fmq_slot_t));
  buf_file_size = handle->fstat.buf_size;
  
  /*
   * check status file size
   */

  if (stat(handle->stat_path, &file_stat)) {
    fmq_print_error(handle, "fmq_check_file_sizes",
		    "Cannot stat status file\n");
    perror(handle->stat_path);
    return FALSE;
  }
  if (file_stat.st_size < stat_file_size) {
    fmq_print_error(handle, "fmq_check_file_sizes",
                    "  Fmq: %s\n"
                    "  Stat file too small: %d bytes\n"
                    "  Should be: %d bytes\n",
                    handle->fmq_path, file_stat.st_size, stat_file_size);
    return FALSE;
  }

  /*
   * check buf file size
   */

  if (stat(handle->buf_path, &file_stat)) {
    fmq_print_error(handle, "fmq_check_file_sizes",
		    "Cannot stat buffer file\n");
    perror(handle->buf_path);
    return FALSE;
  }
  if (file_stat.st_size < buf_file_size) {
    fmq_print_error(handle, "fmq_check_file_sizes",
                    "  Fmq: %s\n"
                    "  Buf file wrong too small: %d bytes\n"
                    "  Should be: %d bytes\n",
                    handle->fmq_path, file_stat.st_size, buf_file_size);
    return FALSE;
  }

  /*
   * read in slots
   */

  if (fmq_read_slots(handle)) {
    fmq_print_error(handle, "fmq_check_file_sizes",
                    "Cannot read in slots array\n");
    return FALSE;
  }

  return TRUE;

}

/**************************
 *  fmq_check()
 *
 *  Checks that the FMQ files are valid.
 *  Assumes files exist and are open.
 *
 *  Return value:
 *    0 on success, -1 if file is corrupted
 */

int fmq_check (FMQ_handle_t *handle)

{

  int islot;
  int slot_num;
  fmq_slot_t *slot;

  /*
   * are the files the correct sizes?
   */

  if (!fmq_check_file_sizes(handle)) {
    return -1;
  }

  /*
   * check that the youngest id has a slot
   */
  
  if (fmq_find_slot_for_id (handle, handle->fstat.youngest_id,
			    &slot_num)) {
    fmq_print_error(handle, "fmq_check",
                    "Cannot read in slot for youngest_id: %d\n",
                    handle->fstat.youngest_id);
    return -1;
  }
  
  /*
   * check all msgs
   */
  
  slot = handle->fslots;
  for (islot = 0; islot < handle->fstat.nslots; islot++, slot++) {

    /*
     * Check whether this slot should be active or not.
     */
    
    int should_be_active = fmq_slot_in_active_region(handle, islot);
    
    if (slot->active && !should_be_active) {
      fmq_print_error(handle, "fmq_check",
                      "Slot %d is active, should be inactive\n",
                      islot);
      return -1;
    }

    if (!slot->active && should_be_active) {
      fmq_print_error(handle, "fmq_check",
                      "Slot %d is inactive, should be active\n",
                      islot);
      return -1;
    }
    
    if (slot->active && should_be_active) {
      if (fmq_read_msg_for_slot(handle, islot)) {
        fmq_print_error(handle, "fmq_check",
                        "Cannot read slot num: %d\n",
                        islot);
        return -1;
      }
    }
    
  }

  return 0;

}

/**************************
 *  fmq_check_and_clear()
 *
 *  Checks that the FMQ files are not corrupted.
 *  Assumes the files are open.
 *  If corrupted, clears the buffers ready for fresh start.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_check_and_clear(FMQ_handle_t *handle)

{

  /*
   * check the file for corruption
   */

  if (fmq_check(handle)) {

    /*
     * clear the queue buffer and files
     * ready to start clean
     */
    
    if (fmq_clear(handle)) {
      return -1;
    }

    fprintf(stderr, "WARNING  %s:fmq_check_and_clear\n", handle->prog_name);
    fprintf(stderr, "  FMQ path '%s'\n", handle->fmq_path);
    fprintf(stderr, "    FMQ check failed\n");
    fprintf(stderr, "    FMQ re-initialized\n");
    
  }

  return 0;

}

/**************************
 *  fmq_check_and_recover()
 *
 *  Checks that the FMQ files are valid, performs recovery
 *  if possible.
 *
 *  Assumes the files are not open.
 *
 *  Return value:
 *    0 on success, -1 on failure
 */

int fmq_check_and_recover (FMQ_handle_t *handle)

{

  int iret = 0;

  if (!fmq_exist(handle)) {
    return -1;
  }

  /*
   * open the files
   */

  if (fmq_open_rdwr(handle, 0, 0)) {
    return -1;
  }

  /*
   * check the file for corruption
   */

  if (fmq_check(handle)) {

    /*
     * recover the FMQ if possible
     */
    
    fmq_lock_rdwr(handle);
    iret = fmq_recover(handle);
    fmq_unlock(handle);

  } /* if fmq_check(....) */

  fmq_close(handle);

  return iret;

}

/*******************************************
 *  fmq_recover()
 *
 *  Recover the FMQ from a corrupted state.
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

static int fmq_recover (FMQ_handle_t *handle)

{

  int islot;
  int done;
  int append_mode;
  int ids_wrap = FALSE;
  int zero_present;
  int max_present;
  int maxId;

  int nslots = handle->fstat.nslots;
  int youngest_id = 0, oldest_id = 0;
  int youngest_slot_num = 0, oldest_slot_num = 0;
  int begin_insert = 0, end_insert = 0, begin_append = 0;

  fmq_slot_t *slot = NULL;
  
  fmq_print_error(handle, "fmq_recover",
                  "Recovering Fmq: %s\n", handle->fmq_path);
  
  /*
   * do the ID's wrap?
   * They do if both 0 and MAX are present.
   */

  zero_present = FALSE;
  max_present = FALSE;
  slot = handle->fslots; 
  for (islot = 0; islot < nslots; islot++, slot++) {
    if (slot->id == 0) {
      zero_present = TRUE;
    }
    if (slot->id == FMQ_MAX_ID - 1) {
      max_present = TRUE;
    }
  }
  if (zero_present && max_present) {
    ids_wrap = TRUE;
  }

  /*
   * find youngest_id and youngest_slot_num, respecting the fact that the
   * IDs may have wrapped
   */

  maxId = FMQ_MAX_ID;
  if (ids_wrap) {
    maxId = nslots;
  }
  youngest_id = -1;
  slot = handle->fslots;
  for (islot = 0; islot < nslots; islot++, slot++) {
    if (slot->active && slot->id > youngest_id && slot->id < maxId) {
      youngest_id = slot->id;
      youngest_slot_num = islot;
    }
  }

  if (youngest_id == -1) {
    return -1;
  }

  /*
   * Find oldest_slot_num by moving back through the slots, checking
   * that the previous slot number is correct. If this pattern
   * breaks, the oldest valid slot has been found.
   */

  oldest_slot_num = youngest_slot_num;
  done = FALSE;

  while (!done) {
    int prev_slot_num = fmq_prev_slot(handle, oldest_slot_num);
    int prev_id = fmq_prev_id(oldest_id);
    fmq_slot_t *prev_slot = handle->fslots + prev_slot_num;
    if (prev_slot->id != prev_id) {
      done = TRUE;
    } else {
      oldest_slot_num = prev_slot_num;
      oldest_id = prev_id;
    }
  }

  if (handle->debug) {
    fprintf(stderr, "--> Original  youngest, oldest: %d, %d\n",
	    handle->fstat.youngest_slot, handle->fstat.oldest_slot);
    fprintf(stderr, "--> Recovered youngest, oldest: %d, %d\n",
	    youngest_slot_num, oldest_slot_num);
  }

  handle->fstat.youngest_slot = youngest_slot_num;
  handle->fstat.oldest_slot = oldest_slot_num;

  /*
   * set all other messages inactive
   */

  slot = handle->fslots;
  for (islot = 0; islot < nslots; islot++, slot++) {
    if (!fmq_slot_in_active_region(handle, islot) &&
	slot->active) {
      fmq_print_error(handle, "fmq_recover",
                      "Setting slot %d inactive\n", islot);
      MEM_zero(*slot);
      if (fmq_write_slot(handle, islot)) {
	return -1;
      }
    }
  }

  /***********************************************************
   * Determine the begin and end of the insert and append regions,
   * and whether the buffer is in append mode.
   *
   * The following rules are applied:
   *
   *   1. If the youngest slot stores its message closest to the end
   *      of the buffer, the FMQ is in append mode. Otherwise it is
   *      in insert mode (!append_mode).
   *
   *   2. The append region starts at the end of the active slot which
   *      is closest to the end of the buffer file.
   *
   *   3. In insert mode, the insert region starts at the end of the
   *      youngest slot. In append mode, the insert region starts at
   *      that start of the file.
   *
   *   4. The insert region ends at the start of the oldest slot.
   */

  /*
   * check for append mode
   */

  append_mode = TRUE;
  begin_append = 0;
  begin_insert = 0;
  end_insert = 0;

  {
    fmq_slot_t *youngest_slot = handle->fslots + youngest_slot_num;
    fmq_slot_t *oldest_slot = handle->fslots + oldest_slot_num;
    int youngest_end = youngest_slot->offset + youngest_slot->stored_len;
    slot = handle->fslots;
    for (islot = 0; islot < nslots; islot++, slot++) {
      if(slot->active) {
        int end = slot->offset + slot->stored_len;
        if (begin_append > end) {
          begin_append = end;
        }
        if (end > youngest_end) {
          append_mode = FALSE;
          break;
        }
      }
    }
    if (!append_mode) {
      begin_insert = youngest_slot->offset + youngest_slot->stored_len;
    }
    end_insert = oldest_slot->offset;
  }
  
#ifdef NOTNOW
  islot = oldest_slot_num;
  done = FALSE;
  while (!done) {
    
    slot = handle->fslots + islot;

    if (islot == oldest_slot_num) {
      end_insert = slot->offset;
    }
    
    end = slot->offset + slot->stored_len;

    if (append_mode) {
      if (end > begin_append) {
	begin_append = end;
      } else {
	append_mode = FALSE;
	begin_insert = end;
      }
    } else {
      begin_insert = end;
    }
    
    if (islot == youngest_slot_num) {
      done = TRUE;
    }

    islot = fmq_next_slot(handle, islot);

  } /* while */
#endif

  if (handle->debug) {
    fprintf(stderr, "--> Original BI, EI, BA, AM: %d, %d, %d, %d\n",
	    handle->fstat.begin_insert, handle->fstat.end_insert,
	    handle->fstat.begin_append, handle->fstat.append_mode);
    fprintf(stderr, "--> Recovered BI, EI, BA, AM: %d, %d, %d, %d\n",
	    begin_insert, end_insert, begin_append, append_mode);
  }

  handle->fstat.begin_insert = begin_insert;
  handle->fstat.end_insert = end_insert;
  handle->fstat.begin_append = begin_append;
  handle->fstat.append_mode = append_mode;

  if (fmq_write_stat(handle)) {
    return -1;
  }

  return 0;

}

/*******************************************
 *  Compute checksum for stat struct
 *
 *  Returns checksum
 */

int fmq_compute_stat_checksum(const fmq_stat_t *stat)

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

/*******************************************
 *  Compute checksum for slot struct
 *
 *  Returns checksum
 */

int fmq_compute_slot_checksum(const fmq_slot_t *slot)

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

/*******************************************
 *  Add a checksum to stat struct
 */

void fmq_add_stat_checksum(fmq_stat_t *stat)

{

  int sum = fmq_compute_stat_checksum(stat);
  stat->checksum = sum;

}

/*******************************************
 *  Add a checksum to slot struct
 */

void fmq_add_slot_checksum(fmq_slot_t *slot)

{

  int sum = fmq_compute_slot_checksum(slot);
  slot->checksum = sum;

}

/*******************************************
 *  Check checksum on stat struct
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int fmq_check_stat_checksum(const fmq_stat_t *stat)

{

  int sum;

  if (stat->checksum == 0) {
    /* backward compatibility */
    return 0;
  }

  sum = fmq_compute_stat_checksum(stat);

  if (sum != stat->checksum) {
    return -1;
  }

  return 0;

}

/*******************************************
 *  Check checksum on slot struct
 *
 *  Return value:
 *    0 on success, -1 on failure.
 */

int fmq_check_slot_checksum(const fmq_slot_t *slot)

{

  int sum;

  if (slot->checksum == 0) {
    /* backward compatibility */
    return 0;
  }

  sum = fmq_compute_slot_checksum(slot);

  if (sum != slot->checksum) {
    return -1;
  }

  return 0;

}

