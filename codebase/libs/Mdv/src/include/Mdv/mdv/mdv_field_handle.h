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
 * MDV_FIELD_HANDLE.H
 *
 * Pseudo-class for performing field-related conversions.
 *
 * Mike Dixon, RAP, NCAR,
 * POBox 3000, Boulder, CO, 80307-3000, USA
 *
 * August 1999
 */

#ifndef MDV_FIELD_HANDLE_H
#define MDV_FIELD_HANDLE_H

#include <Mdv/mdv/mdv_file.h>
#include <stdio.h>

#ifndef _MDV_FIELD_HANDLE_INTERNAL

/*
 * Incomplete definition for public interface
 */

struct MDV_field_handle_t;
typedef struct MDV_field_handle_t MDV_field_handle_t;

#endif

/*
 * function prototypes
 */

/*************************************************************************
 *
 * MDV_fhand_create_empty
 *
 * Default constructor
 *
 * Returns pointer to handle
 *
 * NOTE: for the handle routines to work, the following items in the 
 *       header must be set after construction:
 *
 *    nx, ny, nz, volume_size, data_element_nbytes,
 *    bad_data_value, missing_data_value
 *
 *  In addition, for INT8 and INT16:
 *  
 *    scale, bias
 *
 **************************************************************************/

extern MDV_field_handle_t
  *MDV_fhand_create_empty();

/*************************************************************************
 *
 * MDV_fhand_create_copy
 *
 * Copy constructor
 *
 * Returns pointer to handle
 *
 * NOTE: for the handle routines to work, the following items in the 
 *       header must be set:
 *
 *    nx, ny, nz, volume_size, data_element_nbytes,
 *    bad_data_value, missing_data_value
 *
 *  In addition, for INT8 and INT16:
 *  
 *    scale, bias
 *
 **************************************************************************/

extern MDV_field_handle_t
  *MDV_fhand_create_copy(MDV_field_handle_t *rhs);

/*************************************************************************
 *
 * MDV_fhand_create_from_parts
 *
 * Construct an object form the header and data parts.
 *
 * If the data pointer is not NULL, space is allocated for it and the
 * data is copied in.
 *
 * If the data pointer is NULL, space is allocated for it.
 *
 * Returns pointer to handle
 *
 * NOTE: for the handle routines to work, the following items in the 
 *       header must be set:
 *
 *    nx, ny, nz, volume_size, data_element_nbytes,
 *    bad_data_value, missing_data_value
 *
 *  In addition, for INT8 and INT16:
 *  
 *    scale, bias
 *
 **************************************************************************/

extern MDV_field_handle_t
  *MDV_fhand_create_from_parts(MDV_field_header_t *rhs_hdr,
			      void *rhs_vol_data);

/*************************************************************************
 *
 * MDV_fhand_create_plane_from_copy
 *
 * Constructor for a handle for a single plane, given a handle which
 * has the data volume. The data for the plane is copied from the
 * volume.
 *
 * Returns pointer to handle
 *
 **************************************************************************/

extern MDV_field_handle_t
  *MDV_fhand_create_plane_from_copy(MDV_field_handle_t *rhs,
				    int plane_num);
     
/*************************************************************************
 *
 * MDV_fhand_create_plane_from_parts
 *
 * Constructor for a handle for a single plane, given a field header
 * and the data for a single plane.
 *
 * The buffer passed in contains the data only for the single plane.
 *
 * Returns pointer to handle
 *
 **************************************************************************/

extern MDV_field_handle_t
  *MDV_fhand_create_plane_from_parts(MDV_field_header_t *rhs_hdr,
				     int plane_num,
				     void *rhs_plane_data,
				     int plane_size);
     
/*************************************************************************
 *
 * MDV_fhand_delete
 *
 * Destructor
 *
 **************************************************************************/

extern void
  MDV_fhand_delete(MDV_field_handle_t *handle);

/*************************************************************************
 *
 * MDV_fhand_get_hdr
 *
 * Get pointer to field header
 *
 **************************************************************************/

extern MDV_field_header_t
  *MDV_fhand_get_hdr(MDV_field_handle_t *handle);

/*************************************************************************
 *
 * MDV_fhand_get_vol_ptr
 *
 * Get pointer to vol data
 *
 **************************************************************************/

extern void
  *MDV_fhand_get_vol_ptr(MDV_field_handle_t *handle);

/*************************************************************************
 *
 * MDV_fhand_get_vol_len
 *
 * Get vol data len
 *
 **************************************************************************/

extern int
  MDV_fhand_get_vol_len(MDV_field_handle_t *handle);

/*************************************************************************
 * MDV_fhand_convert()
 *
 * Convert MDV data to an output encoding type, specifying
 * compression and scaling.
 *
 * The handle must contain a complete field header and data.
 *
 * Supported encoding types are:
 *   MDV_NATIVE  - no change
 *   MDV_INT8
 *   MDV_INT16
 *   MDV_FLOAT32
 *
 * Supported compression types are:
 *   MDV_COMPRESSION_ASIS - no change
 *   MDV_COMPRESSION_NONE - uncompressed
 *   MDV_COMPRESSION_RLE - see <toolsa/compress.h>
 *   MDV_COMPRESSION_LZO - see <toolsa/compress.h>
 *   MDV_COMPRESSION_GZIP - see <toolsa/compress.h>
 *   MDV_COMPRESSION_BZIP - see <toolsa/compress.h>
 *
 * Scaling types apply only to conversions to int types (INT8 and INT16)
 *
 * Supported scaling types are:
 *   MDV_SCALING_DYNAMIC
 *   MDV_SCALING_INTEGRAL
 *   MDV_SCALING_SPECIFIED
 *
 * For MDV_SCALING_DYNAMIC, the scale and bias is determined from the
 * dynamic range of the data.
 *
 * For MDV_SCALING_INTEGRAL, the operation is similar to MDV_SCALING_DYNAMIC,
 * except that the scale and bias are constrained to integral values.
 * 
 * For MDV_SCALING_SPECIFIED, the specified scale and bias are used.
 *
 * Output scale and bias are ignored for conversions to float, and
 * for MDV_SCALING_DYNAMIC and MDV_SCALING_INTEGRAL.
 *
 * Returns 0 on success, -1 on failure.
 *
 * On success, the volume data is converted, and the header is adjusted
 * to reflect the changes.
 */

extern int
  MDV_fhand_convert(MDV_field_handle_t *handle,
		    int output_encoding,
		    int output_compression,
		    int output_scaling,
		    double output_scale,
		    double output_bias);

/*************************************************************************
 * MDV_fhand_convert_rounded()
 *
 * This routine calls MDV_fhand_convert(), with ROUNDED scaling.
 * 
 * Returns 0 on success, -1 on failure.
 *
 * See MDV_fhand_convert()
 */

extern int
  MDV_fhand_convert_rounded(MDV_field_handle_t *handle,
			    int output_encoding,
			    int output_compression);
     
/*************************************************************************
 * MDV_fhand_convert_integral()
 *
 * This routine calls MDV_fhand_convert(), with INTEGRAL scaling.
 * 
 * Returns 0 on success, -1 on failure.
 *
 * See MDV_fhand_convert()
 */

extern int
  MDV_fhand_convert_integral(MDV_field_handle_t *handle,
			     int output_encoding,
			     int output_compression);

/*************************************************************************
 * MDV_fhand_convert_dynamic()
 *
 * This routine calls MDV_fhand_convert(), with DYNAMIC scaling.
 * 
 * Returns 0 on success, -1 on failure.
 *
 * See MDV_fhand_convert()
 */

extern int
  MDV_fhand_convert_dynamic(MDV_field_handle_t *handle,
			    int output_encoding,
			    int output_compression);
     
/*************************************************************************
 * MDV_fhand_composite()
 *
 * Compute the composite (max at multiple levels) for the data volume.
 *
 * If lower_plane_num is -1, it is set to the lowest plane in the vol.
 * If upper_plane_num is -1, it is set to the highest plane in the vol.
 * 
 * Returns 0 on success, -1 on failure.
 *
 */

extern int MDV_fhand_composite(MDV_field_handle_t *handle,
			       int lower_plane_num,
			       int upper_plane_num);

/*************************************************************************
 *
 * MDV_fhand_data_to_BE
 *
 * Convert data buffer to BE byte ordering
 *
 **************************************************************************/

extern void MDV_fhand_data_to_BE(MDV_field_handle_t *handle);
     
/*************************************************************************
 *
 * MDV_fhand_data_from_BE
 *
 * Convert data buffer from BE byte ordering
 *
 **************************************************************************/

extern void MDV_fhand_data_from_BE(MDV_field_handle_t *handle);
     
/*************************************************************************
 *
 * MDV_fhand_read_vol
 *
 * Read a field volume from a file.
 *
 * The volume data is read into the volBuf, and then swapped if appropriate.
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

extern int MDV_fhand_read_vol(MDV_field_handle_t *handle,
			      FILE *infile);

/*************************************************************************
 *
 * MDV_fhand_read_plane
 *
 * Read a field plane from a file.
 *
 * The plane data is read into the volBuf, and then swapped if appropriate.
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

extern int MDV_fhand_read_plane(MDV_field_handle_t *handle,
				FILE *infile,
				int plane_num);

/*************************************************************************
 *
 * MDV_fhand_write_vol
 *
 * Write a field volume to a file.
 *
 * The volume data is swapped as appropriate, and them written to
 * the file.
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

extern int MDV_fhand_write_vol(MDV_field_handle_t *handle,
			       FILE *outfile);

/*************************************************************************
 *
 * MDV_fhand_print
 *
 * Print the contents of a field_handle container.
 *
 * print_native: if TRUE, type is preserved.
 *               if FALSE, printed as floats.
 *
 * print_labels: if TRUE, a label will be printed for each plane.
 *
 * pack_duplicates: if TRUE, duplicates will be printed once with a 
 *                           repeat count.
 *                  if FALSE, all values will be printed.
 *
 **************************************************************************/

extern void MDV_fhand_print(MDV_field_handle_t *handle,
			    FILE *out,
			    int print_native,
			    int print_labels,
			    int pack_duplicates);

/*************************************************************************
 *
 * MDV_fhand_print_data
 *
 * Print the volume data part of a field_handle container.
 *
 * print_native: if TRUE, type is preserved.
 *               if FALSE, printed as floats.
 *
 * print_labels: if TRUE, a label will be printed for each plane.
 *
 * pack_duplicates: if TRUE, duplicates will be printed once with a 
 *                           repeat count.
 *                  if FALSE, all values will be printed.
 *
 **************************************************************************/

extern void MDV_fhand_print_voldata(MDV_field_handle_t *handle,
				    FILE *out,
				    int print_native,
				    int print_labels,
				    int pack_duplicates);
     
#endif

#ifdef __cplusplus
}
#endif
