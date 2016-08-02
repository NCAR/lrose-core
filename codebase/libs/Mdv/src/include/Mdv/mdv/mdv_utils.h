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
 * MDV_UTILS.H : Contains utility routines for MDV files
 *
 * R. Ames March 1996. NCAR, RAP.
 *
 */

#ifndef MDV_UTILS_H
#define MDV_UTILS_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <dataport/bigend.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_handle.h>

#ifndef MDV_SUCCESS
#define MDV_SUCCESS 0
#endif

#ifndef MDV_FAILURE
#define MDV_FAILURE -1
#endif


/*****************************************************************
 * MDV_RECALLOC: Allocs or reallocs depending on which one is 
 * necessary.   
 * Returns pointer with appropriate space. 
 */

void * MDV_recalloc(void * ptr_to_mem, int number_of_mem, int size_of_mem);

/*****************************************************************
 * MDV_DATA_ELEMENT_SIZE: Give the size (in bytes) of the data 
 * element given the encoding type integer.
 * Returns -1 if encoding type is MDV_NATIVE.
 * --Rachel Ames 3/96
 * Binary encoding of the data in the arrays. 
 * MDV_NATIVE   0     Whatever units the data is already in 
 * MDV_INT8     1     Uncompressed 8 bit unsigned integers 
 * MDV_INT16    2     Uncompressed 16 bit unsigned integers 
 * MDV_FLOAT32  5     Uncompressed 32 bit signed IEEE Floats
 */

int MDV_data_element_size(int encoding_type);

/*****************************************************************
 * MDV_get_plane_from_volume: Extracts the given plane of data from
 *                            the given volume.
 *
 * Returns a pointer to plane data on success, or NULL on failure.
 * On success, plane_size is set in *plane_size_p.
 *
 * NOTE: Allocates space for the returned plane which must be freed
 *       by the calling routine.
 */

void *MDV_get_plane_from_volume(MDV_field_header_t *field_hdr,
				int plane_num,
				void *volume_ptr,
				int *plane_size_p);

/*****************************************************************
 * Swap routines.
 *****************************************************************/

/*****************************************************************
 * MDV_MASTER_HEADER_FROM_BE: Converts master header from big endian
 * format to native format.  Nancy Rehak 6/97
 */

void MDV_master_header_from_BE(MDV_master_header_t *m_hdr) ;

/*****************************************************************
 * MDV_MASTER_HEADER_TO_BE: Converts master header from native
 * format to big endian format.  Nancy Rehak 6/97
 */

void MDV_master_header_to_BE(MDV_master_header_t *m_hdr) ;

/*****************************************************************
 * MDV_FIELD_HEADER_FROM_BE: Converts field header from big endian
 * format to native format.  Nancy Rehak 6/97
 */

void MDV_field_header_from_BE(MDV_field_header_t *f_hdr);

/*****************************************************************
 * MDV_FIELD_HEADER_TO_BE: Converts field header from native
 * format to big endian format.  Nancy Rehak 6/97
 */

void MDV_field_header_to_BE(MDV_field_header_t *f_hdr);

/*****************************************************************
 * MDV_VLEVEL_HEADER_FROM_BE: Conversts vlevel header from big endian
 * format to native format.  Nancy Rehak 6/97
 */
 
void MDV_vlevel_header_from_BE(MDV_vlevel_header_t *v_hdr);

/*****************************************************************
 * MDV_VLEVEL_HEADER_TO_BE: Converts vlevel header from native
 * format to big endian format.  Nancy Rehak 6/97
 */
 
void MDV_vlevel_header_to_BE(MDV_vlevel_header_t *v_hdr);

/*****************************************************************
 * MDV_FIELD_VLEVEL_HEADER_FROM_BE: Converts a field_vlevel header
 * from big endian format to native format.  Nancy Rehak 6/97
 */
 
void MDV_field_vlevel_header_from_BE(MDV_field_vlevel_header_t *fv_head);

/*****************************************************************
 * MDV_FIELD_VLEVEL_HEADER_TO_BE: Converts a field_vlevel header
 * from native format to big endian format.  Nancy Rehak 6/97
 */
 
void MDV_field_vlevel_header_to_BE(MDV_field_vlevel_header_t *fv_head);

/*****************************************************************
 * MDV_CHUNK_HEADER_FROM_BE: Converts a chunk header from big endian
 * format to native format.  Nancy Rehak 6/97
 */
 
void MDV_chunk_header_from_BE(MDV_chunk_header_t *c_hdr);

/*****************************************************************
 * MDV_CHUNK_HEADER_TO_BE: Converts a chunk header from native
 * format to big endian format.  Nancy Rehak 6/97
 */
 
void MDV_chunk_header_to_BE(MDV_chunk_header_t *c_hdr);

/*****************************************************************
 * MDV_UNENCODED_VOLUME_FROM_BE: Converts the data in an unencoded
 * data volume from big endian format to native format.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_unencoded_volume_from_BE(void *volume_data,
				 ui32 volume_size,
				 int data_type);

/*****************************************************************
 * MDV_UNENCODED_VOLUME_TO_BE: Converts the data in an unencoded
 * data volume from native format to big endian format.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_unencoded_volume_to_BE(void *volume_data,
			       ui32 volume_size,
			       int data_type);

/*****************************************************************
 * MDV_PLANE_TO_BE: Converts the data in a plane of data from big
 * endian format to native format.  N. Rehak 8/98
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_plane_from_BE(MDV_field_header_t *field_hdr, void *plane_ptr);

/*****************************************************************
 * MDV_PLANE_TO_BE: Converts the data in a plane of data from native
 * format to big endian format.  N. Rehak 8/98
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_plane_to_BE(MDV_field_header_t *field_hdr, void *plane_ptr);

/*****************************************************************
 * MDV_PLANE_RLE8_FROM_BE: Converts the data in a plane of data
 * encoded in the MDV_PLANE_RLE8 format from big endian format to
 * native format.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_plane_rle8_from_BE(void *plane_data);

/*****************************************************************
 * MDV_PLANE_RLE8_TO_BE: Converts the data in a plane of data
 * encoded in the MDV_PLANE_RLE8 format from native format to
 * big endian format.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_plane_rle8_to_BE(void *plane_data);

/*****************************************************************
 * MDV_ROW_RLE8_FROM_BE: Converts the data in a plane of data
 * encoded in the MDV_ROW_RLE8 format from big endian format to
 * native format.  N. Rehak 8/98
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_row_rle8_from_BE(void *plane_data, int num_rows);

/*****************************************************************
 * MDV_ROW_RLE8_TO_BE: Converts the data in a plane of data
 * encoded in the MDV_ROW_RLE8 format from native format to big
 * endian format.  N. Rehak 8/98
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_row_rle8_to_BE(void *plane_data, int num_rows);

/*****************************************************************
 * MDV_CHUNK_DATA_FROM_BE: Converts chunk data from big endian
 * format to native format if the chunk data type is a known type.
 * The data pointer must point to the record length value preceding
 * the chunk data.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE */
 
int MDV_chunk_data_from_BE(void *c_data, int size, int chunk_id);

/*****************************************************************
 * MDV_CHUNK_DATA_TO_BE: Converts chunk data from native format to
 * big endian format if the chunk data type is a known type.
 * The data pointer must point to the record length value preceding
 * the chunk data.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE */
 
int MDV_chunk_data_to_BE(void *c_data, int size, int chunk_id);

/*****************************************************************
 * Routines for accessing the mdv_field_code_info array (defined
 * in mdv/mdv_field_codes.h>
 *****************************************************************/

/*****************************************************************
 * MDV_get_field_name: Returns pointer to field name - NULL on error
 *
 */

char *MDV_get_field_name(int field_code);

/*****************************************************************
 * MDV_get_field_units: Returns pointer to field units - NULL on error
 *
 */

char *MDV_get_field_units(int field_code);

/*****************************************************************
 * MDV_get_field_abbrev: Returns pointer to field abbrev - NULL on error
 *
 */

char *MDV_get_field_abbrev(int field_code);


/*****************************************************************
 * MDV_get_field_code_from_name()
 *
 * Returns field code for field name
 *
 * Returns code on success, -1 on failure (no match).
 */

extern int MDV_get_field_code_from_name(char *name);

/*****************************************************************
 * MDV_get_field_code_from_abbrev()
 *
 * Returns field code for field abbrev
 *
 * Returns code on success, -1 on failure (no match).
 */

extern int MDV_get_field_code_from_abbrev(char *abbrev);

/******************************************************************************
 * MDV_CALC_PLANE_SIZE: Calculates the number of bytes used to store the
 *                      data in the indicated plane.
 *
 * Returns the plane size on success, -1 on failure.
 */

extern int MDV_calc_plane_size(MDV_field_header_t *field_hdr, int plane_num,
			       void *plane_ptr);

/******************************************************************************
 * MDV_LOAD_PLANE: Loads the indicated plane from the given buffer.  The
 *                 buffer is expected to point to the beginning of the volume
 *                 data for the field.
 */

extern void *MDV_load_plane(void *buffer, MDV_field_header_t *fld_hdr, 
			    int return_type, int plane_num, int *plane_size);

/******************************************************************************
 * MDV_CALC_BUFFER_SIZE: Calculates the size of a flat buffer needed to
 *                       store this MDV data.
 */

int MDV_calc_buffer_size(MDV_handle_t *mdv);

/******************************************************************************
 * MDV_compressed()
 *
 * Returns TRUE if the compression_type is in the compressed range,
 *         FALSE otherwise.
 *
 * This test is needed because some files have headers which did not
 * zero out the spares, so the new compression stuff does not work
 * properly.
 */

extern int MDV_compressed(int compression_type);

#endif /* MDV_UTILS_H */

#ifdef __cplusplus
}
#endif
