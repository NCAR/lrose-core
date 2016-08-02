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
 * MDV_WRITE.H : Prototypes and defines for routines used to write MDV files.
 *
 * F. Hage Dec 1993. NCAR, RAP.
 *
 * Separated from mdv_user.h.  N. Rehak, Aug. 1996.
 */

#ifndef MDV_WRITE_H
#define MDV_WRITE_H

#include <stdio.h>

#include <toolsa/os_config.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_dataset.h>

/******************************************************************************
 * MDV_handle_write_to_dir
 *
 * Similar to MDV_write_to_dir(), but with control over the output
 * encoding, compression and scaling.
 *
 * Write out the handle structs and data to a given dirctory. The
 * file path is computed from the centroid time. Output files all
 * have the mdv extension.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         output_dir - output directory
 *         output_encoding_type - format for the output field data.
 *         write_ldata_info - option to write latest data info file.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 *
 */

extern int MDV_handle_write_to_dir(MDV_handle_t *mdv,
				   char *output_dir,
				   int output_encoding,
				   int output_compression,
				   int output_scaling,
				   double output_scale,
				   double output_bias,
				   int write_ldata_info);

/******************************************************************************
 * MDV_write_to_dir
 *
 * Write out the handle structs and data to a given dirctory. The file
 * path is computed from the centroid time. Output files all have the
 * mdv extension.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         output_dir - output directory
 *         output_encoding_type - format for the output field data.
 *         write_ldata_info - option to write latest data info file.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.  */

extern int MDV_write_to_dir(MDV_handle_t *mdv,
			    char *output_dir,
			    int output_encoding_type,
			    int write_ldata_info);

/******************************************************************************
 * MDV_handle_write_to_ds_dir
 *
 * Similar to MDV_write_to_ds_dir(), but with control over the output
 * encoding, compression and scaling.
 *
 * Write out the handle structs and data to a dirctory,
 * using the Didss data naming convention:
 *   output_dir/yyyymmdd/g_hhmmss/f_ssssssss
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         output_dir - output directory
 *         output_encoding_type - format for the output field data.
 *         write_ldata_info - option to write latest data info file.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

extern int MDV_handle_write_to_ds_dir(MDV_handle_t *mdv,
				      char *output_dir,
				      int output_encoding,
				      int output_compression,
				      int output_scaling,
				      double output_scale,
				      double output_bias,
				      int write_ldata_info);

/******************************************************************************
 * MDV_WRITE_TO_DS_DIR: Write out the handle structs and data to a dirctory,
 *                   using the Didss data naming convention:
 *                                                           
 *                       output_dir/yyyymmdd/g_hhmmss/f_ssssssss
 * 
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         output_dir - output directory
 *         output_encoding_type - format for the output field data.
 *         write_ldata_info - option to write latest data info file.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

extern int MDV_write_to_ds_dir(MDV_handle_t *mdv,
			       char *output_dir,
			       int output_encoding_type,
			       int write_ldata_info);

/******************************************************************************
 * MDV_HANDLE_WRITE_ALL
 *
 * Write out the handle structs and data to the given path.
 *
 * This is an up-to-date version of MDV_write_all(), which allows
 * you to specify output encoding, compression and scaling.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         file_path - path of output file. 
 *         output_encoding_type - format for the output field data.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

extern int MDV_handle_write_all(MDV_handle_t *mdv,
				char *file_path,
				int output_encoding,
				int output_compression,
				int output_scaling,
				double output_scale,
				double output_bias);

/******************************************************************************
 * MDV_WRITE_ALL: Write out the handle structs and data to the given path.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         file_path - path of output file. 
 *         output_encoding_type - format for the output field data.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_write_all(MDV_handle_t *mdv,
		  char *file_path,
		  int output_encoding_type);

/******************************************************************************
 * MDV_WRITE_MASTER_HEADER: Write an MDV master header to the file in the
 * appropriate place.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         m_hdr - pointer to master header information.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    master header, byte swapped if necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_write_master_header(FILE *outfile, MDV_master_header_t *m_hdr);

/******************************************************************************
 * MDV_WRITE_FIELD_HEADER: Write an MDV field header from the given 
 * structure into the proper place in the file.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         f_hdr - pointer to field header information.
 *         field_num - the field number for this field.  This is used to
 *                     determine where in the file the header will be
 *                     written.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    field header, byte swapped if necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_write_field_header(FILE *outfile, MDV_field_header_t *f_hdr,
			   int field_num);

/******************************************************************************
 * MDV_WRITE_VLEVEL_HEADER: Write an MDV vlevel header from the given
 * structure into the proper place in the file.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         v_hdr - pointer to vlevel header information.
 *         m_hdr - pointer to associated master header information.  This
 *                 information is used to position the vlevel header in
 *                 the output file.
 *         field_num - the number of the field associated with the vlevel
 *                     information.  This value is also used to position
 *                     the vlevel header in the output file.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    vlevel header, byte swapped if necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
int MDV_write_vlevel_header(FILE *outfile, MDV_vlevel_header_t *v_hdr,
			    MDV_master_header_t *m_hdr, int vlevel_num);

/******************************************************************************
 * MDV_WRITE_FIELD_VLEVEL_HEADER: Write an MDV field header and an MDV vlevel
 * header into a file.  Note, these are not written in the file sequentially.
 * The master header field and vlevel offsets MUST be correct!
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         fv_hdr - pointer to the field/vlevel header structure to be
 *                  written to disk.
 *         m_hdr - pointer to the associated master header information.
 *                 This information is used for positioning the field and
 *                 vlevel headers in the file.
 *         field_num - the number of the associated field.  This information
 *                     is also used for positioning the headers in the file.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
int MDV_write_field_vlevel_header(FILE *outfile,
				  MDV_field_vlevel_header_t *fv_hdr,
				  MDV_master_header_t *m_hdr, int field_num);

/******************************************************************************
 * MDV_WRITE_CHUNK: Write an MDV chunk header and data from the given
 * structures into the proper place in the file.  The data offset for
 * the chunk data must be specified by the caller in case the position
 * on disk is different from the position in memory (for example, if
 * the preceding field data was written in a compressed format but was
 * stored in memory in an uncompressed format.)
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         c_hdr - pointer to chunk header information.
 *         c_data - pointer to chunk data.  (This points to the beginning
 *                  of the actual data, not to any preceding FORTRAN
 *                  record length value unless the record length value
 *                  is to be included as part of the chunk data.  This
 *                  routine will put the FORTRAN record length fields
 *                  around the chunk data for you.)
 *         m_hdr - pointer to the associated master header information.
 *                 This information is used for positioning the chunk
 *                 header in the output file.
 *         chunk_num - the number of the chunk.  This information is also
 *                     used for positioning the chunk header in the file.
 *         chunk_data_offset - the offset into the output file where the
 *                             chunk data should be written.  This value
 *                             will be different from the value in the
 *                             chunk header if the field data in the file
 *                             is in a different format than the field data
 *                             in memory.
 *         swap_chunk_data - flag indicating if this routine should try to
 *                           swap the chunk data.  If TRUE, the data will
 *                           be swapped, if necessary, so that the data on
 *                           disk is written in big-endian format.  If
 *                           FALSE, the data will be written to disk as is.
 *                           Note that this routine can only swap the chunk
 *                           data if it is in a known format.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    chunk header, byte swapped if necessary, and the
 *                    chunk data.  The chunk data offset value on disk
 *                    will be the value passed into the routine rather
 *                    than the value in c_hdr.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
int MDV_write_chunk(FILE *outfile, MDV_chunk_header_t *c_hdr,
		    void *c_data,
		    MDV_master_header_t *m_hdr, int chunk_num,
		    int chunk_data_offset,
		    int swap_chunk_data);

/******************************************************************************
 * MDV_WRITE_CHUNK_HEADER: Write an MDV chunk header from the given
 * structure into the proper place in the file.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         c_hdr - pointer to chunk header information.
 *         m_hdr - pointer to the associated master header information.
 *                 This information is used for positioning the chunk
 *                 header in the file.
 *         chunk_num - the chunk number.  This information is also used for
 *                     positioning the header in the file.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    chunk header, byte swapped if necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
int MDV_write_chunk_header(FILE *outfile, MDV_chunk_header_t *c_hdr,
			   MDV_master_header_t *m_hdr, int chunk_num);

/******************************************************************************
 * MDV_write_field_vol
 *
 * For an open file, write the volume data for the given field.
 *
 * The field header must match the volume data. It must be in
 * host byte order. The volume_size and field_data_offset must
 * be correct in the header.
 *
 * You must specify output_encoding, output_compression and output_scaling
 * types. See <Mdv/mdv/mdv_file.h>
 *
 * A copy of the field header is updated to reflect the output types
 * and sizes and the data offset and is written to the file.
 *
 * Returns: 0 on success, -1 on failure.
 *
 * Side effect: if output_volume_len is not NULL the length of the 
 *              field buffer is copied to it
 */

extern int MDV_write_field_vol(FILE *outfile,
			       MDV_field_header_t *f_hdr,
			       int field_num,
			       void *field_data,
			       int output_encoding,
			       int output_compression,
			       int output_scaling,
			       double output_scale,
			       double output_bias,
			       int *output_volume_size);
     
/******************************************************************************
 * MDV_WRITE_FIELD: Write the MDV field header and data from the given 
 * structure into the proper place in the file.  The position of the
 * field header is calculated from the field number and the position of
 * the field data is passed in since this position may be different from
 * the position of the data in the dataset in memory if the data is being
 * output in a format different from that in memory.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         f_hdr - pointer to the field header information.
 *         f_data - pointer to the field data.  The format of this data
 *                  must match the description given in the field header.
 *         field_num - the field number for this field information.  This
 *                     is used for positioning the field header in the
 *                     output file and for error messages.
 *         field_data_offset - offset in the output file where the field
 *                             data should begin.  This may differ from
 *                             the field data offset in the header if the
 *                             data is being written in a different format
 *                             than it is being stored in memory.
 *         output_encoding_type - format for the output field data.
 *
 * Outputs: outfile - updated on disk to conatine the information in the
 *                    field header and the field data.  The field header
 *                    information on disk contains the field data offset
 *                    and encoding type of the data as it is written to
 *                    disk rather than that in f_hdr.
 *
 * Returns: the number of bytes written to the file for the field data, or
 *          -1 on error.
 */

int MDV_write_field(FILE *outfile, MDV_field_header_t *f_hdr,
		    void *f_data,
		    int field_num,
		    int field_data_offset,
		    int output_encoding_type);

/******************************************************************************
 * MDV_WRITE_FIELD_DATA: Write a whole volume of field data.
 *
 * Inputs: fhdr - pointer to the field header information.
 *         field_num - the number of the field being written.
 *         field_data_offset - the offset into the output file where the
 *                             field data should be written.  This value
 *                             may be different from the value in the field
 *                             header if the data is being written in a format
 *                             different from its format in memory.
 *         fld_data_ptr - pointer to the beginning of the field data.
 *         output_encoding_type - format to use for the output data.
 *         outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *
 * Outputs: outfile - updated on disk to contain the field information in
 *                    the indicated format, byte swapped if necessary.
 *
 * Returns: the output size for the field data written or -1 on error.
 */
 
int MDV_write_field_data(MDV_field_header_t *fhdr, int field_num,
			 int field_data_offset,
			 void *fld_data_ptr, int output_encoding_type,
			 FILE *outfile);

/******************************************************************************
 * MDV_WRITE_CHUNK_DATA: Write out the chunk data to the given file.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         offset - offset into the output file where the chunk data should
 *                  be written.  This offset should point to the position
 *                  of the chunk data itself.  FORTRAN record length values
 *                  will written before and after this data on disk.
 *         c_data - pointer to the chunk data to be written.
 *         size - size in bytes of the chunk data.  This value does not
 *                include the surrounding FORTRAN record length fields.
 *         chunk_id - chunk identifier.  If this is a chunk type known to
 *                    the library, then the chunk information may be byte
 *                    swapped when written, if necessary.
 *         swap_flag - flag indicating whether this routine should try to
 *                     byte swap the chunk data before writing.  If TRUE,
 *                     the data will be byte swapped, if necessary, if the
 *                     chunk type is known by the library.  If FALSE, the
 *                     chunk data is written as is to the file.
 *
 * Outputs: outfile - updated on disk to contain the chunk information,
 *                    byte swapped if necessary and requested.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_write_chunk_data(FILE *outfile, long offset,
			 void *c_data, long size,
			 long chunk_id, int swap_flag);

#endif /* MDV_WRITE_H */

#ifdef __cplusplus
}
#endif
