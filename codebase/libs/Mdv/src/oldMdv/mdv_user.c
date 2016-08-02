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
/******************************************************************************
 *  MDV_USER.C  Subroutines useful for accessing MDV data.
 *  F. Hage.  Dec 1993. RAP, R. Ames 6/96.
 *
 *  Divided into mdv_user.c, mdv_read.c and mdv_write.c.
 *  N. Rehak, Aug. 1996.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <memory.h>

#include <toolsa/os_config.h>
#include <Mdv/mdv/mdv_user.h>
#include <toolsa/mem.h>


/******************************************************************************
 * MDV_FREE_FIELD_VLEVEL_HEADER: Frees space for field_vlevel header fields.
 *
 * Inputs: fv_hdr - pointer to field/vlevel header structure to be freed.
 *
 * Outputs: fv_hdr - internal pointers are freed and set to NULL.  The
 *                   MDV_field_vlevel_header_t pointer is NOT freed by
 *                   this routine.
 */

void MDV_free_field_vlevel_header( MDV_field_vlevel_header_t *fv_hdr)
{
 
    ufree(fv_hdr->fld_hdr);
    fv_hdr->fld_hdr = NULL;

    ufree(fv_hdr->vlv_hdr);
    fv_hdr->vlv_hdr = NULL;
 
}

/******************************************************************************
 * MDV_INIT_MASTER_HEADER: Initializes an MDV_master_header_t struct.
 *
 * Inputs: master_hdr - pointer to master header to be initialized.
 *
 * Outputs: master_hdr - values are set to "initialized" values.  Generally,
 *                       this means 0, but sets things like the FORTRAN
 *                       record lengths appropriately.
 */

void MDV_init_master_header(MDV_master_header_t *master_hdr)
{
  /*
   * Initialize the values to 0.
   */

  memset((void *)master_hdr, 0, sizeof(MDV_master_header_t));
  
  /*
   * Now set any values that we can.
   */

  master_hdr->record_len1 = sizeof(MDV_master_header_t) -
    (2 * sizeof(si32));

  master_hdr->struct_id = MDV_MASTER_HEAD_MAGIC_COOKIE;
  master_hdr->revision_number = MDV_REVISION_NUMBER;
  
  master_hdr->record_len2 = master_hdr->record_len1;
  
  return;
}


/******************************************************************************
 * MDV_INIT_FIELD_HEADER: Initializes an MDV_field_header_t struct.
 *
 * Inputs: field_hdr - pointer to field header to be initialized.
 *
 * Outputs: field_hdr - values are set to "initialized" values.  Generally,
 *                      this means 0, but sets things like the FORTRAN
 *                      record lengths appropriately.
 */

void MDV_init_field_header(MDV_field_header_t *field_hdr)
{
  /*
   * Initialize the values to 0.
   */

  memset((void *)field_hdr, 0, sizeof(MDV_field_header_t));
  
  /*
   * Now set any values that we can.
   */

  field_hdr->record_len1 = sizeof(MDV_field_header_t) -
    (2 * sizeof(si32));

  field_hdr->struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;
  
  field_hdr->record_len2 = field_hdr->record_len1;
  
  return;
}


/******************************************************************************
 * MDV_INIT_VLEVEL_HEADER: Initializes an MDV_vlevel_header_t struct.
 *
 * Inputs: vlevel_hdr - pointer to vlevel header to be initialized.
 *
 * Outputs: vlevel_hdr - values are set to "initialized" values.  Generally,
 *                       this means 0, but sets things like the FORTRAN
 *                       record lengths appropriately.
 */

void MDV_init_vlevel_header(MDV_vlevel_header_t *vlevel_hdr)
{
  /*
   * Initialize the values to 0.
   */

  memset((void *)vlevel_hdr, 0, sizeof(MDV_vlevel_header_t));
  
  /*
   * Now set any values that we can.
   */

  vlevel_hdr->record_len1 = sizeof(MDV_vlevel_header_t) -
    (2 * sizeof(si32));

  vlevel_hdr->struct_id = MDV_VLEVEL_HEAD_MAGIC_COOKIE;
  
  vlevel_hdr->record_len2 = vlevel_hdr->record_len1;
  
  return;
}


/******************************************************************************
 * MDV_INIT_CHUNK_HEADER: Initializes an MDV_chunk_header_t struct.
 *
 * Inputs: chunk_hdr - pointer to chunk header to be initialized.
 *
 * Outputs: chunk_hdr - values are set to "initialized" values.  Generally,
 *                      this means 0, but sets things like the FORTRAN
 *                      record lengths appropriately.
 */

void MDV_init_chunk_header(MDV_chunk_header_t *chunk_hdr)
{
  /*
   * Initialize the values to 0.
   */

  memset((void *)chunk_hdr, 0, sizeof(MDV_chunk_header_t));
  
  /*
   * Now set any values that we can.
   */

  chunk_hdr->record_len1 = sizeof(MDV_chunk_header_t) -
    (2 * sizeof(si32));

  chunk_hdr->struct_id = MDV_CHUNK_HEAD_MAGIC_COOKIE;
  
  chunk_hdr->record_len2 = chunk_hdr->record_len1;
  
  return;
}


/******************************************************************************
 * MDV_SET_MASTER_HDR_OFFSETS: Set all of the offset values the master
 * header.  The offset values are set based on the values of the other
 * fields in the header, so these must be set before this routine is 
 * called.
 *
 * Inputs: m_hdr - pointer to master header to be updated.
 *
 * Outputs: m_hdr - all offset values are set appropriately.
 */
 
void MDV_set_master_hdr_offsets(MDV_master_header_t *m_hdr)
{
  m_hdr->field_hdr_offset = sizeof(MDV_master_header_t);

  m_hdr->vlevel_hdr_offset = m_hdr->field_hdr_offset +
    (m_hdr->n_fields * sizeof(MDV_field_header_t));
  
  if (m_hdr->vlevel_included)
    m_hdr->chunk_hdr_offset = m_hdr->vlevel_hdr_offset +
      (m_hdr->n_fields * sizeof(MDV_vlevel_header_t));
  else
    m_hdr->chunk_hdr_offset = m_hdr->vlevel_hdr_offset;
  
  return;
}

/******************************************************************************
 * MDV_GET_FIRST_FIELD_OFFSET: Get the file offset for the first field in
 * the MDV file.
 *
 * Inputs: dataset - pointer to the dataset whose chunk header offset values
 *                   are to be updated.
 *
 * Outputs: dataset - all offset values in the chunk headers are set
 *                    appropriately.  Othere dataset information is used
 *                    to determine these offsets and must be accurate
 *                    before this routine is called.
 */
 
si32 MDV_get_first_field_offset(MDV_master_header_t *master_hdr)
{
  if (master_hdr->vlevel_included)
    return(sizeof(MDV_master_header_t) +
	   (master_hdr->n_fields * sizeof(MDV_field_header_t)) +
	   (master_hdr->n_fields * sizeof(MDV_vlevel_header_t)) +
	   (master_hdr->n_chunks * sizeof(MDV_chunk_header_t)) +
	   sizeof(si32));
  else
    return(sizeof(MDV_master_header_t) +
	   (master_hdr->n_fields * sizeof(MDV_field_header_t)) +
	   (master_hdr->n_chunks * sizeof(MDV_chunk_header_t)) +
	   sizeof(si32));
}


