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
 * fmq_print.c
 *
 * Printing routines for FMQ
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#include <toolsa/fmq_private.h>
#include <toolsa/udatetime.h>
#define BOOL_STR(a) ((a)? "true" : "false")

/*******************
 *  fmq_print_stat()
 *
 *  Prints out the status struct.
 *
 *  Returns 0 on success, -1 on failure.
 *
 */

int fmq_print_stat(FMQ_handle_t *handle, FILE *out)

{

  /*
   * print status
   */

  fprintf(out, "\n");
  fprintf(out, "FMQ STATUS - %s\n", handle->fmq_path);
  fprintf(out, "==========\n");
  fprintf(out, "\n");
  
  fprintf(out, "  magic_cookie: %d\n", handle->fstat.magic_cookie);
  fprintf(out, "  youngest_id: %d\n", handle->fstat.youngest_id);
  fprintf(out, "  youngest_slot: %d\n", handle->fstat.youngest_slot);
  fprintf(out, "  oldest_slot: %d\n", handle->fstat.oldest_slot);
  fprintf(out, "  nslots: %d\n", handle->fstat.nslots);
  fprintf(out, "  buf_size: %d\n", handle->fstat.buf_size);
  fprintf(out, "  begin_insert: %d\n", handle->fstat.begin_insert);
  fprintf(out, "  end_insert: %d\n", handle->fstat.end_insert);
  fprintf(out, "  begin_append: %d\n", handle->fstat.begin_append);
  fprintf(out, "  append_mode: %s\n", BOOL_STR(handle->fstat.append_mode));
  fprintf(out, "  time_written: %s\n", utimstr(handle->fstat.time_written));
  fprintf(out, "  blocking_write: %d\n", handle->fstat.blocking_write);
  fprintf(out, "  last_id_read: %d\n", handle->fstat.last_id_read);
  fprintf(out, "  checksum: %d\n", handle->fstat.checksum);

  fprintf(out, "\n");
  fprintf(out, "\n");

  return 0;

}

/***************
 *  print_slot()
 *
 *  Prints out a slot.
 */

static void print_slot(FMQ_handle_t *handle, FILE *out,
		       char *label, int latest_num)
     
{

  fprintf(out,
	  "%s - %d: active %d, id %d, time %s, msg_len %d, "
	  "stored_len %d, offset %d, type %d, subtype %d, "
          "compress %s, checksum %d\n",
	  label, latest_num,
	  handle->fslot.active, 
	  handle->fslot.id, 
	  utimstr(handle->fslot.time), 
	  handle->fslot.msg_len, 
	  handle->fslot.stored_len, 
	  handle->fslot.offset, 
	  handle->fslot.type, 
	  handle->fslot.subtype,
	  BOOL_STR(handle->fslot.compress),
          handle->fslot.checksum);

}

/************************
 *  fmq_print_slot_read()
 *
 *  Prints out the latest slot read.
 */

int fmq_print_slot_read(FMQ_handle_t *handle, FILE *out)
     
{

  print_slot(handle, out, "LATEST SLOT READ",
	     handle->last_slot_read);
  return 0;

}

/***************************
 *  fmq_print_slot_written()
 *
 *  Prints out the latest slot written.
 */

int fmq_print_slot_written(FMQ_handle_t *handle, FILE *out)
     
{
  
  print_slot(handle, out, "LATEST SLOT WRITTEN",
	     handle->fstat.youngest_slot);
  return 0;

}

/********************
 *  fmq_print_debug()
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

int fmq_print_debug(char *fmq_path,
		    char *prog_name,
		    FILE *out)
     
{

  int islot;
  fmq_slot_t *slot;
  FMQ_handle_t handle;
  
  if (fmq_init_handle(&handle, fmq_path, TRUE, prog_name)) {
    fprintf(stderr, "fmq_init_handle failed.\n");
    return -1;
  }

  fprintf(out, "\n");
  fprintf(out, "FMQ STATUS\n");
  fprintf(out, "==========\n");
  fprintf(out, "\n");

  /*
   * open the files if they exist
   */
  
  if (!fmq_exist(&handle)) {
    return -1;
  }

  if (fmq_open(&handle, "r")) {
    return -1;
  }

  /*
   * check if valid
   */
  
  if (fmq_check_file_sizes(&handle)) {
    fprintf(out, "FMQ %s is valid\n\n", handle.fmq_path);
  } else {
    fprintf(out, "FMQ %s is not valid\n", handle.fmq_path);
    if (fmq_open(&handle, "r")) {
      fmq_print_error(&handle, "fmq_print_debug",
		      "Cannot open files\n");
      return -1;
    }
  }
  
  /*
   * read in status struct
   */
  
  if (fmq_read_stat(&handle)) {
    fmq_print_error(&handle, "fmq_print_debug",
		    "Cannot read in stat struct\n");
    perror(handle.stat_path);
    return -1;
  }
  
  /*
   * read in slots
   */
  
  if (fmq_alloc_slots(&handle, handle.fstat.nslots)) {
    return -1;
  }
  
  if (fmq_read_slots(&handle)) {
    fmq_print_error(&handle, "fmq_print_debug ",
		    "Cannot read in slot structs array\n");
    perror(handle.stat_path);
    return -1;
  }
  
  /*
   * print status
   */

  fmq_print_stat(&handle, out);

  /*
   * print slots
   */
  
  fprintf(out,
	  "*** slot_num, active, id, time, msg_len, "
	  "stored_len, offset, type, subtype ***\n");

  slot = handle.fslots;
  for (islot = 0; islot < handle.fstat.nslots; islot++, slot++) {
    fmq_print_slot(islot, slot, out);
  } /* islot */

  fprintf(out, "\n");
  fprintf(out, "\n");
  
  fmq_close(&handle);
  fmq_free_handle(&handle);

  return 0;

}

/********************
 *  Print slots
 */

void fmq_print_slots(FMQ_handle_t *handle,
                     FILE *out)
     
{
  fmq_slot_t *slot = handle->fslots;
  int islot;
  for (islot = 0; islot < handle->fstat.nslots; islot++, slot++) { 
    fmq_print_slot(islot, slot, out);
  } /* islot */
}

/********************
 *  Print slots
 */

void fmq_print_active_slots(FMQ_handle_t *handle, FILE *out)
     
{
  fmq_slot_t *slot = handle->fslots;
  int islot;
  for (islot = 0; islot < handle->fstat.nslots; islot++, slot++) { 
    if (slot->active) {
      fmq_print_slot(islot, slot, out);
    }
  } /* islot */
}

/********************
 *  Print slot
 */

void fmq_print_slot(int slot_num,
                    fmq_slot_t *slot,
                    FILE *out)
     
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

void fmq_pretty_print_slot(int slot_num,
                           fmq_slot_t *slot,
                           FILE *out)
     
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

