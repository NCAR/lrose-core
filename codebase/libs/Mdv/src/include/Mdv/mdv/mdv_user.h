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
#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 * MDV_USER.H : Defines necessary for using general MDV routines.
 *
 * F. Hage Dec 1993. NCAR, RAP.
 *
 */

#ifndef MDV_USER_H
#define MDV_USER_H

#include <toolsa/os_config.h>

#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_write.h>
#include <Mdv/mdv/mdv_dataset.h>


/******************************************************************************
 * MDV_FREE_FIELD_VLEVEL_HEADER: Frees space for field_vlevel header fields.
 *
 * Inputs: fv_hdr - pointer to field/vlevel header structure to be freed.
 *
 * Outputs: fv_hdr - internal pointers are freed and set to NULL.  The
 *                   MDV_field_vlevel_header_t pointer is NOT freed by
 *                   this routine.
 */

void MDV_free_field_vlevel_header( MDV_field_vlevel_header_t *fv_hdr);

/******************************************************************************
 * MDV_INIT_MASTER_HEADER: Initializes an MDV_master_header_t struct.
 *
 * Inputs: master_hdr - pointer to master header to be initialized.
 *
 * Outputs: master_hdr - values are set to "initialized" values.  Generally,
 *                       this means 0, but sets things like the FORTRAN
 *                       record lengths appropriately.
 */

void MDV_init_master_header(MDV_master_header_t *master_hdr);

/******************************************************************************
 * MDV_INIT_FIELD_HEADER: Initializes an MDV_field_header_t struct.
 *
 * Inputs: field_hdr - pointer to field header to be initialized.
 *
 * Outputs: field_hdr - values are set to "initialized" values.  Generally,
 *                      this means 0, but sets things like the FORTRAN
 *                      record lengths appropriately.
 */

void MDV_init_field_header(MDV_field_header_t *field_hdr);

/******************************************************************************
 * MDV_INIT_VLEVEL_HEADER: Initializes an MDV_vlevel_header_t struct.
 *
 * Inputs: vlevel_hdr - pointer to vlevel header to be initialized.
 *
 * Outputs: vlevel_hdr - values are set to "initialized" values.  Generally,
 *                       this means 0, but sets things like the FORTRAN
 *                       record lengths appropriately.
 */

void MDV_init_vlevel_header(MDV_vlevel_header_t *vlevel_hdr);

/******************************************************************************
 * MDV_INIT_CHUNK_HEADER: Initializes an MDV_chunk_header_t struct.
 *
 * Inputs: chunk_hdr - pointer to chunk header to be initialized.
 *
 * Outputs: chunk_hdr - values are set to "initialized" values.  Generally,
 *                      this means 0, but sets things like the FORTRAN
 *                      record lengths appropriately.
 */

void MDV_init_chunk_header(MDV_chunk_header_t *chunk_hdr);

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
 
void MDV_set_master_hdr_offsets(MDV_master_header_t *m_hdr);

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
 
si32 MDV_get_first_field_offset(MDV_master_header_t *master_hdr);

#endif /* MDV_USER_H */

#ifdef __cplusplus
}
#endif
