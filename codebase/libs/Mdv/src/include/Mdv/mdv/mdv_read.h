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
 * MDV_READ.H : Prototypes and defines for routines for reading MDV data
 *              from files.
 *
 * F. Hage Dec 1993. NCAR, RAP.
 *
 * Separated from mdv_user.h.  N. Rehak, Aug. 1996.
 */

#ifndef MDV_READ_H
#define MDV_READ_H

#include <stdio.h>

#include <toolsa/os_config.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_dataset.h>

/******************************************************************************
 * MDV_VERIFY:
 *
 * Verify that a file is in MDV format
 *
 * Returns: TRUE or FALSE
 */
 
extern int MDV_verify( char * infile_name);

/******************************************************************************
 * MDV_LOAD_MASTER_HEADER: Load mdv data file header into given area from
 * disk.  Memory for the master header is assumed to be allocated before
 * this routine is called.  The bytes in the header are swapped if necessary
 * to put the data in native format.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         m_hdr - pointer to the master header to be loaded.
 *
 * Outputs: m_hdr - updated to include the values read in from disk,
 *                  byte swapped as necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

extern int MDV_load_master_header(FILE *infile, MDV_master_header_t *m_hdr);
 
/******************************************************************************
 * MDV_LOAD_FIELD_HEADER: Load mdv field header data into the given structure
 * from disk.  Memory for the field header is assumed to be allocated before
 * this routine is called.  The bytes in the header are swapped if necessary
 * to put the data in native format.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         f_hdr - pointer to the field header to be loaded.
 *         field_num - number of the field being loaded.  This is used to
 *                     determine the position on disk where the field header
 *                     information is located.
 *
 * Outputs: f_hdr - updated to include the values read in from disk, byte
 *                  swapped as necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

extern int MDV_load_field_header(FILE *infile, MDV_field_header_t *f_hdr,
				 int field_num);

/******************************************************************************
 * MDV_LOAD_VLEVEL_HEADER: Load mdv vlevel header data into the given
 * structure from disk.  Memory for the vlevel header is assumed to be
 * allocated before this routine is called.  The bytes in the header are
 * swapped if necessary to put the data in native format.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         v_hdr - pointer to the vlevel header to be loaded.
 *         m_hdr - pointer to the associated master header information.  This
 *                 information is used to determine the position on disk
 *                 where the vlevel header information is located.
 *         field_num - the field number.  This information is also used to
 *                     determine the disk location.
 *
 * Output: v_hdr - updated to include the values read in from disk,
 *                 byte swapped as necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
extern int MDV_load_vlevel_header(FILE *infile, MDV_vlevel_header_t *v_hdr,
				  MDV_master_header_t *m_hdr, int vlevel_num);

/******************************************************************************
 * MDV_LOAD_VLEVEL_HEADER_OFFSET: Load mdv vlevel header data into the given
 * structure from disk given the offset for the first vlevel header in the
 * file rather than the master header.  Memory for the vlevel header is
 * assumed to be allocated before this routine is called.  The bytes in the
 * header are swapped if necessary to put the data in native format.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         v_hdr - pointer to the vlevel header to be loaded.
 *         vlevel_offset - offset in the file to the first vlevel header.
 *                         This information is used to determine the disk
 *                         location of the desired vlevel header.
 *         field_num - the field number.  This information is also used to
 *                     determine the disk location.
 *
 * Output: v_hdr - updated to include the values read in from disk,
 *                 byte swapped as necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
extern int MDV_load_vlevel_header_offset(FILE *infile,
					 MDV_vlevel_header_t *v_hdr, 
					 int vlevel_offset,
					 int field_num);

/******************************************************************************
 * MDV_LOAD_FIELD_VLEVEL_HEADER: Load mdv field_vlevel header data into the 
 * given structure from disk.  Memory for the field/vlevel header is assumed
 * to be allocated before this routine is called.  Memory for the individual
 * structures pointed to by the field/vlevel header are allocated by this
 * routine.
 *
 * Inputs:  infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *          fv_hdr - pointer to the field/vlevel header to be loaded.
 *          m_hdr - pointer to the associated master header information.
 *                  This information is used to determine the position on
 *                  disk where the field and vlevel headers are located.
 *          field_num - the field number.  This information is also used to
 *                      determine the header locations.
 *
 * Outputs:  fv_hdr - updated to contain the field and vlevel header
 *                    information.  Memory is allocated for both loaded
 *                    headers.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
extern int MDV_load_field_vlevel_header(FILE *infile,
					MDV_field_vlevel_header_t *fv_hdr,
					MDV_master_header_t *m_hdr,
					int field_num);

/******************************************************************************
 * MDV_LOAD_CHUNK_HEADER: Load mdv chunk header data into the given structure
 * from disk.  Memory for the chunk header is assumed to be allocated before
 * this routine is called.  The bytes in the header are swapped if necessary.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         c_hdr - pointer to the chunk header to be loaded.
 *         m_hdr - pointer to the assocated master header information.  This
 *                 information is used to determine the position on disk
 *                 where the chunk header information is located.
 *         chunk_num - the chunk number.  This information is also used to
 *                     determine the chunk header location.
 *
 * Outputs: c_hdr - updated to include the values read in from disk, byte
 *                  swapped as necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
extern int MDV_load_chunk_header(FILE *infile, MDV_chunk_header_t *c_hdr,
				 MDV_master_header_t *m_hdr, int chunk_num);

/******************************************************************************
 * MDV_read_field_volume
 *
 * From an open file, read the volume data for the given field.
 *
 * The field header must already have been read an swapped into host
 * byte order. You are responsible for making sure the field_header is
 * correct for the open file
 *
 * You must specify output_encoding, output_compression and output_scaling
 * types. See <Mdv/mdv/mdv_file.h>
 *
 * The field header is updated to reflect the output types and sizes.
 *
 * Returns: returns a pointer to the data volume, byte swapped if
 *          necessary, or NULL if there is an error.  This space for this
 *          data volume is allocated by this routine and must be freed
 *          by the calling routine using ufree().
 *
 *          Also, if output_volume_len is not NULL the length of the 
 *          field buffer is copied to it.
 */

extern void * MDV_read_field_volume(FILE *infile,
				    MDV_field_header_t *f_hdr,
				    int output_encoding,
				    int output_compression,
				    int output_scaling,
				    double output_scale,
				    double output_bias,
				    int *output_volume_len);
     
/******************************************************************************
 * MDV_read_field_plane
 *
 * From an open file, read the plane data for the given field.
 *
 * The field header must already have been read an swapped into host
 * byte order. You are responsible for making sure the field_header is
 * correct for the open file
 *
 * You must specify output_encoding, output_compression and output_scaling
 * types. See <Mdv/mdv/mdv_file.h>
 *
 * The field header is updated to reflect the output types and sizes.
 *
 * Returns: returns a pointer to the data plane, byte swapped if
 *          necessary, or NULL if there is an error.  This space for this
 *          data volume is allocated by this routine and must be freed
 *          by the calling routine using ufree().
 *
 *          Also, if output_plane_len is not NULL the length of the 
 *          field buffer is copied to it.
 */

extern void * MDV_read_field_plane(FILE *infile,
				   MDV_field_header_t *f_hdr,
				   int output_encoding,
				   int output_compression,
				   int output_scaling,
				   double output_scale,
				   double output_bias,
				   int plane_num,
				   int *output_plane_len);
     
/******************************************************************************
 * MDV_read_field_composite
 *
 * From an open file, read the vol data for the given field,
 * create a composite and return the composite plane.
 *
 * If lower_plane_num is -1, it is set to the lowest plane in the vol.
 * If upper_plane_num is -1, it is set to the highest plane in the vol.
 * 
 * The field header must already have been read an swapped into host
 * byte order. You are responsible for making sure the field_header is
 * correct for the open file
 *
 * You must specify output_encoding, output_compression and output_scaling
 * types. See <Mdv/mdv/mdv_file.h>
 *
 * The field header is updated to reflect the output types and sizes.
 *
 * Returns: returns a pointer to the data plane, byte swapped if
 *          necessary, or NULL if there is an error.  This space for this
 *          data volume is allocated by this routine and must be freed
 *          by the calling routine using ufree().
 *
 *          Also, if output_plane_len is not NULL the length of the 
 *          field buffer is copied to it.
 */

extern void * MDV_read_field_composite(FILE *infile,
				       MDV_field_header_t *f_hdr,
				       int output_encoding,
				       int output_compression,
				       int output_scaling,
				       double output_scale,
				       double output_bias,
				       int lower_plane_num,
				       int upper_plane_num,
				       int *output_plane_len);
     
/******************************************************************************
 * MDV_GET_VOLUME: Allocate space for a data volume (data for all levels) and 
 * read the desired volume into the buffer from the Open file. Caller is 
 * responsible for freeing up the buffer when done. Caller is responsible 
 * for making sure the field_header is correct and proper for the open file.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         f_hdr - field header for the volume to be loaded.  This header
 *                 must contain the correct encoding_type, volume_size and
 *                 field_data_offset for the data.
 *         return_type - format in memory for the returned data volume.
 *
 * Outputs: None.
 *
 * Returns: returns a pointer to the data volume information, byte swapped if
 *          necessary, or NULL if there is an error.  This space for this
 *          data volume is allocated by this routine and must be freed by the
 *          calling routine.
 */

extern void *MDV_get_volume(FILE *infile,
			    const MDV_field_header_t *f_hdr,
			    int return_type);

/******************************************************************************
 * MDV_GET_VOLUME_SIZE: Allocate space for a data volume (data for all levels)
 * and read the desired volume into the buffer from the Open file. Returns the
 * size of the volume returned.  Caller is responsible for freeing up the
 * buffer when done. Caller is responsible for making sure the field_header is
 * correct and proper for the open file.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         f_hdr - field header for the volume to be loaded.  This header
 *                 must contain the correct encoding_type, volume_size and
 *                 field_data_offset for the data.
 *         return_type - format in memory for the returned data volume.
 *
 * Outputs: volume_size - number of bytes in the volume of data returned.
 *
 * Returns: returns a pointer to the data volume information, byte swapped if
 *          necessary, or NULL if there is an error.  This space for this
 *          data volume is allocated by this routine and must be freed by the
 *          calling routine.
 */

extern void * MDV_get_volume_size(FILE *infile,
				  const MDV_field_header_t *f_hdr,
				  int return_type,
				  int *volume_size);

/******************************************************************************
 * MDV_GET_PLANE: Allocate space for a data plane and read the desired plane
 * into the buffer from the Open file. Caller is responsible for freeing
 * up the buffer when done. Caller is responsible for making sure the
 * field_vlevel_header is correct and proper for the open file.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         fld_hdr - field header for the plane to be loaded.  This header
 *                   must contain the correct encoding_type and
 *                   field_data_offset for the data.
 *         return_type - format in memory for the returned data plane.
 *         plane_num - number of the field plane to be loaded.
 *
 * Outputs: None.
 *
 * Returns: returns a pointer to the data plane information, byte swapped if
 *          necessary, or NULL if there is an error.  This space for this
 *          data plane is allocated by this routine and must be freed by the
 *          calling routine.
 */

extern void * MDV_get_plane(FILE *infile,
			    const MDV_field_header_t *f_hdr,
			    int return_type, int plane_num);

/******************************************************************************
 * MDV_GET_PLANE_SIZE: Just like MDV_get_plane, but also returns the size in
 *                     bytes of the returned plane.
 */

extern void * MDV_get_plane_size(FILE *infile,
				 const MDV_field_header_t *fld_hdr, 
				 int return_type, int plane_num,
				 int *plane_size);

/******************************************************************************
 * MDV_GET_COMPOSITE: Read the data volume for the given field and composite
 * it using the given algorithm.  Space for the returned composite plane is
 * allocated by this routine.  Caller is responsible for freeing up the buffer
 * when done. Caller is responsible for making sure the field_header is
 * correct and proper for the open file.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         field_hdr - field header for the data to be loaded.  This header
 *                     must contain the correct encoding_type and
 *                     field_data_offset for the data in the file.
 *         composite_alg - algorithm to be used in the composite.  Currently
 *                         only MDV_COMPOSITE_MAX is supported.
 *         return_type - format in memory for the returned data plane.
 *                       Must be an uncompressed format.  Currently only
 *                       MDV_INT8 format is supported.
 *
 * Outputs: None.
 *
 * Returns: returns a pointer to the composite plane information, byte swapped
 *          if necessary, or NULL if there is an error.  This space for this
 *          data plane is allocated by this routine and must be freed by the
 *          calling routine.
 */

extern void * MDV_get_composite(FILE *infile,
				const MDV_field_header_t *field_hdr, 
				int composite_alg, int return_type);

/******************************************************************************
 * MDV_GET_CHUNK_DATA: Get the chunk data for the given chunk.  The chunk
 * data is swapped if the library knows about that type of chunk data.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         c_hdr - pointer to the chunk header to be loaded.
 *
 * Outputs: None.
 *
 * Returns: Pointer to the chunk data read from the file, byte swapped if
 *          necessary and if of a type known by the library.  Returns NULL
 *          on error.
 */
 
extern void *MDV_get_chunk_data(FILE *infile, MDV_chunk_header_t *c_hdr);

/*************************************************************************
 *
 * MDV_handle_read_all
 *
 * This is a more up-to-date version of MDV_read_all(). It allows you
 * to specify the output encoding type, compression and scaling.
 *
 * Reads all fields and planes in volume.
 * Handle members and pointers are loaded up with the data.
 *
 * Must call MDV_init_handle() first.
 *
 * This routine may be called repeatedly - memory allocation is
 * handled within the routine.
 *
 * When done, call MDV_free_handle() to free memory. 
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

extern int MDV_handle_read_all(MDV_handle_t *mdv, char *file_path,
			       int output_encoding,
			       int output_compression,
			       int output_scaling,
			       double output_scale,
			       double output_bias);

/*************************************************************************
 *
 * MDV_handle_read_field
 *
 * This is just like MDV_handle_read_all() except that only one field
 * is read from the file.  The field is specified by its 0-based index
 * in the file. After this call, all of the headers in the MDV_handle_t
 * structure are internally consistent.
 *
 * Reads all planes in volume for the indicated field.
 * Handle members and pointers are loaded up with the data.
 *
 * Must call MDV_init_handle() first.
 *
 * This routine may be called repeatedly - memory allocation is
 * handled within the routine.
 *
 * When done, call MDV_free_handle() to free memory. 
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

extern int MDV_handle_read_field(MDV_handle_t *mdv, char *file_path,
				 int field_num,
				 int output_encoding,
				 int output_compression,
				 int output_scaling,
				 double output_scale,
				 double output_bias);

/*************************************************************************
 *
 * MDV_read_all
 *
 * Reads all fields and planes in volume.
 * Handle members and pointers are loaded up with the data.
 *
 * Must call MDV_init_handle() first.
 *
 * This routine may be called repeatedly - memory allocation is
 * handled within the routine.
 *
 * When done, call MDV_free_handle() to free memory. 
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

extern int MDV_read_all(MDV_handle_t *mdv, char *file_path, int return_type);

#endif

#ifdef __cplusplus
}
#endif
