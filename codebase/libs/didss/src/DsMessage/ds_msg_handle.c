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
 * ds_msg_handle.c
 *
 * This is a C version of the DsMessage class.
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * Jan 1999
 *
 ***********************************************************/

#include <dataport/bigend.h>      
#include <didss/ds_msg_handle.h>
#include <toolsa/mem.h>
#include <assert.h>

/*
 * file scope functions
 */
  
static void _alloc_assembled_msg(ds_msg_handle_t *handle);

static void _check_init(ds_msg_handle_t *handle);

static void _alloc_parts(ds_msg_handle_t *handle, int n_parts);

static void _alloc_part_buf(ds_msg_part_t *part);

static int ds_msg_load_part_from_msg(ds_msg_part_t *part,
				     int part_num,
				     void *in_msg,
				     int msg_len);

static void ds_msg_load_part_from_mem(ds_msg_part_t *part,
				      int data_type,
				      int len,
				      void *in_mem);

/************************************************
 *
 * ds_msg_init_handle
 *
 * initializes the memory associated with handle
 *
 */

void ds_msg_init_handle(ds_msg_handle_t *handle,
			ds_msg_mem_model_t mem_model)
     
{

  memset(handle, 0, sizeof(ds_msg_handle_t));

  handle->mem_model = mem_model;
  
  handle->hdr.type = -1;
  handle->hdr.subType = -1;
  handle->hdr.mode = -1;
  handle->hdr.flags = 0;
  handle->hdr.majorVersion = 1;
  handle->hdr.minorVersion = 0;
  handle->hdr.serialNum = -1;
  handle->hdr.category = 0;
  handle->hdr.nParts = 0;

  handle->init_flag = DS_MSG_HANDLE_INIT_DONE; 

}

/*****************************************
 *
 * ds_msg_free_handle
 *
 * Frees the memory associated with handle
 *
 */

void ds_msg_free_handle(ds_msg_handle_t *handle)

{

  _check_init(handle);

  if (handle->parts) {
    int i;
    for (i = 0; i < handle->hdr.nParts; i++) {
      if (handle->parts[i].buf) {
	ufree(handle->parts[i].buf);
	handle->parts[i].buf = NULL;
	handle->parts[i].n_buf_alloc = 0;
      }
    }
    ufree(handle->parts);
    handle->parts = NULL;
    handle->n_parts_alloc = 0;
  }
  
  if (handle->assembled_msg) {
    ufree (handle->assembled_msg);
    handle->assembled_msg = NULL;
    handle->n_assembled_alloc = 0;
  }

  handle->init_flag = 0;

}

/************************************************************
 * decode a message header
 *
 * This is used if you just want to peek at the header before
 * deciding how to handle the message.
 */

void ds_msg_decode_header(ds_msg_handle_t *handle,
			  void *in_msg)
     
{
  
  DsMsgHdr_t hdr;

  _check_init(handle);
  memcpy(&hdr, in_msg, sizeof(DsMsgHdr_t));
  BE_to_DsMsgHdr(&hdr);

  handle->hdr.type = hdr.type;
  handle->hdr.subType = hdr.subType;
  handle->hdr.mode = hdr.mode;
  handle->hdr.flags = hdr.flags;
  handle->hdr.majorVersion = hdr.majorVersion;
  handle->hdr.minorVersion = hdr.minorVersion;
  handle->hdr.serialNum = hdr.serialNum;
  handle->hdr.category = hdr.category;
  handle->hdr.nParts = hdr.nParts;
  
}

/********************************************************
 * disassemble a message into parts, store in
 * DsMessage object.
 *
 * The parts are checked to make sure they do not run over
 * the end of the message.
 *
 * Returns 0 on success, -1 on failure.
 */

int ds_msg_disassemble(ds_msg_handle_t *handle,
		       void *in_msg, int msg_len)
     
{
  
  int i;

  _check_init(handle);

  ds_msg_decode_header(handle, in_msg);
  
  _alloc_parts(handle, handle->hdr.nParts);
  for (i = 0; i < handle->hdr.nParts; i++) {
    if (ds_msg_load_part_from_msg(handle->parts + i,
				  i, in_msg, msg_len)) {
      ds_msg_print_header(handle, stderr, "  ");
      return (-1);
    }
  }
  return (0);
}

/***********************************************
 * does a part exist?
 *
 * returns the number of parts of the given type
 */

int ds_msg_part_exists(ds_msg_handle_t *handle,
		       int data_type)

{

  int count = 0;
  int i;

  _check_init(handle);

  for (i = 0; i < handle->hdr.nParts; i++) {
    if (handle->parts[i].type == data_type) {
      count++;
    }
  }
  return (count);
}

/********************************************
 * Get a part from the parts array, given
 * the index into the array.
 *
 * Returns pointer to part, NULL on failure.
 */

ds_msg_part_t *ds_msg_get_part(ds_msg_handle_t *handle,
			       int index) 
  
{

  _check_init(handle);

  if (index > handle->hdr.nParts - 1) {
    return (NULL);
  }

  return (handle->parts + index);

}

/********************************************************
 * get a part by type.
 *
 * The index refers only to parts of this type. For example,
 * an index of 2 will return the 3rd part of the given type.
 *
 * Returns pointer to the requested part, NULL on failure.
 */

ds_msg_part_t *ds_msg_get_part_by_type(ds_msg_handle_t *handle,
				       int data_type,
				       int index) 
  
{

  int i;
  int count = 0;

  _check_init(handle);

  for (i = 0; i < handle->hdr.nParts; i++) {
    if (handle->parts[i].type == data_type) {
      if (count == index) {
	return(handle->parts + i);
      }
      count++;
    }
  }

  return (NULL);

}
  
/********************************************************
 * clear before adding parts.
 *
 * This sets the number of parts to 0.
 *
 */

void ds_msg_clear_parts(ds_msg_handle_t *handle)
     
{
  _check_init(handle);
  handle->hdr.nParts = 0;
}

/*********************************************************
 * Add a part to the object.
 *
 * The part is added at the end of the part list.
 *
 * The buffer must be in BE byte order.
 */

void ds_msg_add_part(ds_msg_handle_t *handle,
		     int type, int len, void *data)
     
{

  _check_init(handle);
  
  _alloc_parts(handle, handle->hdr.nParts + 1);

  ds_msg_load_part_from_mem(handle->parts + handle->hdr.nParts,
			    type, len, data);
 
  handle->hdr.nParts++;

}

/********************************************
 * assemble the parts into a message
 *
 * Returns pointer to the assembled message.
 */

ui08 *ds_msg_assemble(ds_msg_handle_t *handle)

{
  
  int i;
  int partHdrOffset;
  int partDataOffset;
  DsMsgHdr_t header;

  _check_init(handle);

  /* compute total message length */

  handle->length_assembled = 0;
  handle->length_assembled += sizeof(DsMsgHdr_t);
  handle->length_assembled += handle->hdr.nParts * sizeof(DsMsgPart_t);

  for (i = 0; i < handle->hdr.nParts; i++) {
    handle->length_assembled += handle->parts[i].padded_length;
  }

  /* allocate memory */

  _alloc_assembled_msg(handle);

  /* load up header */

  memset(&header, 0, sizeof(DsMsgHdr_t));
  header.type = handle->hdr.type;
  header.subType = handle->hdr.subType;
  header.mode = handle->hdr.mode;
  header.flags = handle->hdr.flags;
  header.majorVersion = handle->hdr.majorVersion;
  header.minorVersion = handle->hdr.minorVersion;
  header.serialNum = handle->hdr.serialNum;
  header.category = handle->hdr.category;
  header.nParts = handle->hdr.nParts;
  BE_from_DsMsgHdr(&header);
  memcpy(handle->assembled_msg, &header, sizeof(DsMsgHdr_t));

  /* load up parts */

  partHdrOffset = sizeof(DsMsgHdr_t);
  partDataOffset = partHdrOffset + handle->hdr.nParts * sizeof(DsMsgPart_t);

  for (i = 0; i < handle->hdr.nParts; i++) {
    DsMsgPart_t msgPart;
    memset(&msgPart, 0, sizeof(DsMsgPart_t));
    msgPart.dataType = handle->parts[i].type;
    msgPart.len = handle->parts[i].length;
    msgPart.offset = partDataOffset;
    BE_from_DsMsgPart(&msgPart);
    memcpy(handle->assembled_msg + partHdrOffset,
	   &msgPart, sizeof(DsMsgPart_t));
    memcpy(handle->assembled_msg + partDataOffset,
	   handle->parts[i].buf, handle->parts[i].length);
    partHdrOffset += sizeof(DsMsgPart_t);
    partDataOffset += handle->parts[i].padded_length;
  }

  return (handle->assembled_msg);

}

/******************************
 * print out the message header
 *
 */

void ds_msg_print_header(ds_msg_handle_t *handle,
			 FILE *out,  char *spacer)
     
{
  
  _check_init(handle);

  fprintf(out, "%sDsMessage:\n", spacer);
  fprintf(out, "%s  type: %d\n", spacer, handle->hdr.type);
  fprintf(out, "%s  subType: %d\n", spacer, handle->hdr.subType);
  fprintf(out, "%s  mode: %d\n", spacer, handle->hdr.mode);
  fprintf(out, "%s  flags: %d\n", spacer, handle->hdr.flags);
  fprintf(out, "%s  majorVersion: %d\n", spacer, handle->hdr.majorVersion);
  fprintf(out, "%s  minorVersion: %d\n", spacer, handle->hdr.minorVersion);
  fprintf(out, "%s  serialNum: %d\n", spacer, handle->hdr.serialNum);
  fprintf(out, "%s  nParts: %d\n", spacer, handle->hdr.nParts);
  
  if (handle->hdr.category == DS_MSG_GENERIC) {
    fprintf(out, "%s        category: Generic\n", spacer);
  } else if (handle->hdr.category == DS_MSG_SERVER_STATUS) {
    fprintf(out, "%s        category: ServerStatus\n", spacer);
  } else {
    fprintf(out, "%s        category: Unknown\n", spacer);
  }
  
}

/*******************************
 * check that init has been done
 */

static void _check_init(ds_msg_handle_t *handle)

{
  assert(handle->init_flag == DS_MSG_HANDLE_INIT_DONE);
}

/************************************************
 * allocate the buffer for the assembled message
 */

static void _alloc_assembled_msg(ds_msg_handle_t *handle)
     
{
  if (handle->length_assembled > handle->n_assembled_alloc) {
    handle->assembled_msg =
      (ui08 *) urealloc(handle->assembled_msg, handle->length_assembled);
    handle->n_assembled_alloc = handle->length_assembled;
  }
  memset(handle->assembled_msg, 0, handle->length_assembled);
}


/************************************************
 * allocate the parts
 */

static void _alloc_parts(ds_msg_handle_t *handle,
			 int n_parts)
     
{
  int i;
  ds_msg_part_t *part;

  if (n_parts > handle->n_parts_alloc) {

    /* alloc parts */

    handle->parts = (ds_msg_part_t *)
      urealloc(handle->parts, n_parts * sizeof(ds_msg_part_t));

    /* initialize new parts */

    for (i = handle->n_parts_alloc; i < n_parts; i++) {
      part = handle->parts + i;
      part->type = -1;
      part->length = 0;
      part->padded_length = 0;
      part->buf = NULL;
      part->n_buf_alloc = 0;
      if (handle->mem_model == DS_MSG_COPY_MEM) {
	part->buf_is_local = 1;
      } else {
	part->buf_is_local = 0;
      }
    } /* i */
    
    handle->n_parts_alloc = n_parts;

  }

}

/************************************
 * allocate the local buffer for part
 */

static void _alloc_part_buf(ds_msg_part_t *part)
     
{
  assert(part->buf_is_local);
  if ((part->length > part->n_buf_alloc) ||
      (part->length < part->n_buf_alloc/2)) {
    part->buf = (ui08 *) urealloc(part->buf, part->length);
    part->n_buf_alloc = part->length;
  }
}

/*********************************************************
 * load a part from an incoming message which is assumed to
 * be in BE byte order
 *
 * The part is checked to make sure it does not run over
 * the end of the message.
 *
 * Returns 0 on success, -1 on error
 * Error occurs if end of part is beyond end of message.
 */

static int ds_msg_load_part_from_msg(ds_msg_part_t *part,
				     int part_num,
				     void *in_msg,
				     int msg_len)

{

  ui08 *inBuf = (ui08 *) in_msg;
  DsMsgPart_t msgPart;
  
  memcpy(&msgPart,
	 inBuf + sizeof(DsMsgHdr_t) + part_num * sizeof(DsMsgPart_t),
	 sizeof(DsMsgPart_t));

  BE_to_DsMsgPart(&msgPart);
  
  part->type = msgPart.dataType;
  part->length = msgPart.len;
  part->padded_length = ((part->length / 8) + 1) * 8;
  part->offset = msgPart.offset;
  
  if (msg_len > 0 && (part->offset + part->length) > msg_len) {
    fprintf(stderr, "ds_msg_load_part_from_msg.\n");
    fprintf(stderr, "  End of part %d is beyond end of message.\n",
	    part_num);
    fprintf(stderr, "  End of part offset: %d\n",
	    part->offset + part->length);
    fprintf(stderr, "  End of message offset: %d\n", msg_len);
    return (-1);
  }

  if (part->buf_is_local) {
    _alloc_part_buf(part);
    memcpy(part->buf, inBuf + part->offset, part->length);
  } else {
    part->buf = inBuf + part->offset;
  }
  
  return (0);

}

/*****************************************************
 * load a part from a memory buffer which is assumed to
 * be in host byte order
 */

static void ds_msg_load_part_from_mem(ds_msg_part_t *part,
				      int data_type,
				      int len,
				      void *in_mem)

{

  part->type = data_type;
  part->length = len;
  part->padded_length = ((part->length / 8) + 1) * 8;
  
  if (part->buf_is_local) {
    _alloc_part_buf(part);
    memcpy(part->buf, in_mem, part->length);
  } else {
    part->buf = (ui08 *) in_mem;
  }

}

