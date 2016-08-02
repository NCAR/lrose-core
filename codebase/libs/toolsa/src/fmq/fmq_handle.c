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
 * fmq_handle.c
 *
 * Routines for FMQ handle.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#include <toolsa/fmq_private.h>

/******************
 * fmq_check_init()
 *
 * Returns TRUE if handle has been initialized,
 * FALSE otherwise;
 */

int fmq_check_init(FMQ_handle_t *handle)
     
{

  if (handle->magic_cookie == FMQ_MAGIC_STAT) {
    return TRUE;
  } else {
    fprintf(stderr, "ERROR - FMQ handle has not been initialized\n");
    return FALSE;
  }

}

int fmq_check_init_no_error_message(FMQ_handle_t *handle)
     
{
  
  if (handle->magic_cookie == FMQ_MAGIC_STAT) {
    return TRUE;
  } else {
    return FALSE;
  }

}

/*******************
 * fmq_init_handle()
 *
 * Initialize FMQ handle
 *
 * This must be done before using the handle for any
 * other function.
 *
 * Returns 0 on success, -1 on error.
 */

int fmq_init_handle(FMQ_handle_t *handle,
		    const char *fmq_path,
		    int debug,
		    const char *prog_name)
     
{

  MEM_zero(*handle);
  handle->prog_name = umalloc(strlen(prog_name) + 1);
  strcpy(handle->prog_name, prog_name);
  handle->debug = debug;
  handle->last_id_read = -1;
  handle->last_slot_read = -1;
  handle->magic_cookie = FMQ_MAGIC_STAT;
  handle->compress_method = TA_COMPRESSION_ZLIB;

  /*
   * local FMQ set file paths
   */
  
  if (handle->fmq_path == NULL) {
    handle->fmq_path = umalloc(strlen(fmq_path) + 1);
  } else {
    handle->fmq_path = urealloc(handle->fmq_path, strlen(fmq_path) + 1);
  }
  strcpy(handle->fmq_path, fmq_path);
    
  if (handle->stat_path == NULL) {
    handle->stat_path = umalloc(strlen(fmq_path) + 6);
  } else {
    handle->stat_path = urealloc(handle->stat_path, strlen(fmq_path) + 6);
  }
  sprintf(handle->stat_path, "%s.%s", handle->fmq_path, "stat");
    
  if (handle->buf_path == NULL) {
    handle->buf_path = umalloc(strlen(fmq_path) + 5);
  } else {
    handle->buf_path = urealloc(handle->buf_path, strlen(fmq_path) + 5);
  }
  sprintf(handle->buf_path, "%s.%s", handle->fmq_path, "buf");
  
  return 0;

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

void fmq_set_heartbeat(FMQ_handle_t *handle,
		       TA_heartbeat_t heartbeat_func)

{
  handle->heartbeat_func = heartbeat_func;
}

/********************
 * fmq_set_compress()
 *
 * Set compression on.
 */

void fmq_set_compress(FMQ_handle_t *handle)

{
  handle->compress = TRUE;
}

/*****************************
 * fmq_set_compression_method()
 *
 * Set the compression methos - defaults to ZLIB compression
 */

void fmq_set_compression_method(FMQ_handle_t *handle,
				ta_compression_method_t method)

{
  handle->compress_method = method;
}

/*****************************
 * fmq_set_server()
 *
 * Sets the flag which indicates the handle is being
 * used by a server.
 */

void fmq_set_server(FMQ_handle_t *handle)
{
  handle->server = TRUE;
}

/***************************
 * fmq_set_blocking_write()
 *
 * Set blocking write on. NOTE - not yet supported.
 *
 * Returns 0 on success, -1 on error.
 */

void fmq_set_blocking_write(FMQ_handle_t *handle)

{
  handle->blocking_write = TRUE;
}

/********************
 *  fmq_free_handle()
 *
 *  Free memory associated with the handle, i.e. that
 *  which was allocated by fmq_init_handle().
 *
 *  Zero out the handle struct.
 *
 */

void fmq_free_handle (FMQ_handle_t *handle)

{

  if (handle->magic_cookie != FMQ_MAGIC_STAT) {
    return;
  }

  if (handle->prog_name != NULL) {
    ufree (handle->prog_name);
  }
  
  if (handle->fmq_path != NULL) {
    ufree (handle->fmq_path);
  }
  
  if (handle->stat_path != NULL) {
    ufree (handle->stat_path);
  }
  
  if (handle->buf_path != NULL) {
    ufree (handle->buf_path);
  }
  
  if (handle->dmsg != NULL) {
    ufree (handle->dmsg);
  }
  
  MEM_zero(*handle);

}

