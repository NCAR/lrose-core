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

/***********************************************************
 *
 * ds_msg_handle.h
 *
 * This is a C version of the DsMessage class.
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * Jan 1999
 *
 ***********************************************************/

#ifndef _ds_msg_handle_h_
#define _ds_msg_handle_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <dataport/port_types.h>
#include <didss/ds_message.h>

/*
 * magic cookie for checking on init
 */

#define DS_MSG_HANDLE_INIT_DONE 765159872

/*
 *  memory model - the user can choose either to copy memory areas
 *  into memory local to the object, or to point to memory held
 *  by other objects. If the PointToMem model is used, the user
 *  must make sure that the memory pointed to remains valid for
 *  as long as this class needs it. If you are assembling a message,
 *  the memory must be valid until after the assemble. If you are
 *  disassembling a message, the memory must remain valid until 
 *  after you are done with the components of the message.
 * 
 *  The memory is actually managed by the DsMsgPart class, which
 *  holds the pointer to the data buffer for each class.
 */  

typedef enum { DS_MSG_COPY_MEM, DS_MSG_POINT_TO_MEM } ds_msg_mem_model_t;

/*
 * Server message categories
 */

typedef enum {
  DS_MSG_GENERIC = 8389420,
  DS_MSG_SERVER_STATUS
} ds_msg_cat_t;

/*
 * ds_msg_part_t struct
 */

typedef struct {

  int type;
  int length;
  int padded_length;
  int offset;
  ui08 *buf;
  int n_buf_alloc;
  int buf_is_local;

} ds_msg_part_t;

/*
 * ds_msg_handle_t struct
 */

typedef struct {

  int init_flag;
  int mem_model;
  
  /*
   * the header and parts
   */
  
  DsMsgHdr_t hdr;
  ds_msg_part_t *parts;
  int n_parts_alloc;

  /*
   * assembled message
   */

  ui08 *assembled_msg;
  int length_assembled;
  int n_assembled_alloc;

} ds_msg_handle_t;

/*
 * interface
 */

/************************************************
 *
 * ds_msg_init_handle
 *
 * initializes the memory associated with handle
 *
 * Returns 0 on success, -1 on failure.
 *
 */

extern void ds_msg_init_handle(ds_msg_handle_t *handle,
			       ds_msg_mem_model_t mem_model);

/*****************************************
 *
 * ds_msg_free_handle
 *
 * Frees the memory associated with handle
 *
 */

extern void ds_msg_free_handle(ds_msg_handle_t *handle);

/*************************************************************
 * decode a message header
 *
 * This is used if you just want to peek at the header before
 * deciding how to handle the message.
 */

extern void ds_msg_decode_header(ds_msg_handle_t *handle,
				 void *in_msg);

/**********************************************
 * disassemble a message into parts, store in
 * ds_msg_handle.
 *
 * The parts are checked to make sure they do not run over
 * the end of the message.
 *
 * Returns 0 on success, -1 on failure.
 *
 */

extern int ds_msg_disassemble(ds_msg_handle_t *handle,
			      void *in_msg, int msg_len);

/************************************************
 * does a part exist?
 * returns the number of parts of the given type
 */

extern int ds_msg_part_exists(ds_msg_handle_t *handle,
			      int data_type);

/*****************************************
 * Get a part from the parts array, given
 * the index into the parts array.
 *
 * Returns pointer to part, NULL on failure.
 */
 
extern ds_msg_part_t *ds_msg_get_part(ds_msg_handle_t *handle,
				      int index);

/*********************************************************
 * Get a part by type.
 *
 * The index refers only to parts of this type. For example,
 * an index of 2 will return the 3rd part of the given type.
 *
 * Returns pointer to the requested part, NULL on failure.
 */

extern ds_msg_part_t *ds_msg_get_part_by_type(ds_msg_handle_t *handle,
					      int data_type, int index);
  
/*********************************************
 * clear before adding parts.
 *
 * This initializes the number of parts to 0.
 *
 * It does NOT clear the header attributes set using the
 * set() routines.
 */

extern void ds_msg_clear_parts(ds_msg_handle_t *handle);

/************************************************
 * Add a part to the object.
 *
 * The part is added at the end of the part list.
 *
 * The buffer must be in BE byte order.
 */
 
extern void ds_msg_add_part(ds_msg_handle_t *handle,
			    int type, int len, void *data);

/*************************************************
 * assemble the parts into a message
 *
 * Returns pointer to the assembled message.
 */
  
extern ui08 *ds_msg_assemble(ds_msg_handle_t *handle);

/**************************************************
 * print out the message header
 *
 */
  
extern void ds_msg_print_header(ds_msg_handle_t *handle,
				FILE *out, char *spacer);

#ifdef __cplusplus
}
#endif

#endif
