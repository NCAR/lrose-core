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
/*************************************************************************
 * MDV_FIELD_HANDLE.C
 *
 * Functions for performing field-related actions.
 *
 * Mike Dixon, RAP, NCAR,
 * POBox 3000, Boulder, CO, 80307-3000, USA
 *
 * August 1999
 */

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/membuf.h>
#include <toolsa/compress.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_print.h>
#include <toolsa/toolsa_macros.h>
#include <math.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/***********************************************************************
 * MDV_field_handle_t is a container for field data.
 *
 * MDV_field_handle_t internals are private - they cannot be accessed by
 * external routines.
 *
 * External routines must do a create, and then use the pointer
 * to gain access to the functions.
 */

typedef struct {
  
  MDV_field_header_t hdr;
  MEMbuf *volBuf;
  
} MDV_field_handle_t;

#define _MDV_FIELD_HANDLE_INTERNAL
#include <Mdv/mdv/mdv_field_handle.h>
#undef _MDV_FIELD_HANDLE_INTERNAL

/*
 * file scope prototypes
 */

static void _int8_to_int16(MDV_field_handle_t *handle,
			   int output_scaling,
			   double output_scale,
			   double output_bias);

static void _int16_to_int8(MDV_field_handle_t *handle,
			   int output_scaling,
			   double output_scale,
			   double output_bias);

static void _int8_to_float32(MDV_field_handle_t *handle);

static void _int16_to_float32(MDV_field_handle_t *handle);

static void _float32_to_int8(MDV_field_handle_t *handle,
			     int output_scaling);

static void _float32_to_int8_specified(MDV_field_handle_t *handle,
				       double output_scale,
				       double output_bias);

static void _float32_to_int16(MDV_field_handle_t *handle,
			      int output_scaling);

static void _float32_to_int16_specified(MDV_field_handle_t *handle,
					double output_scale,
					double output_bias);

static int _compress(MDV_field_handle_t *fld,
		     int compression_type);

static int _decompress(MDV_field_handle_t *fld);

static void _set_data_element_nbytes(MDV_field_handle_t *handle);

static int _set_min_and_max(MDV_field_handle_t *handle);

static double _round_up(double z);

static void _print_voldata_verbose(MDV_field_handle_t *handle,
				   FILE *out,
				   int print_labels);

static void _print_voldata_packed(MDV_field_handle_t *handle,
				  FILE *out,
				  int print_labels);

static void _print_int8_packed(FILE *out, int count,
			       ui08 val, ui08 bad, ui08 missing);

static void _print_int16_packed(FILE *out, int count,
				ui16 val, ui16 bad, ui16 missing);

static void _print_float32_packed(FILE *out, int count,
				  fl32 val, fl32 bad, fl32 missing);

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

MDV_field_handle_t *MDV_fhand_create_empty()
     
{

  MDV_field_handle_t *handle = ucalloc(1, sizeof(MDV_field_handle_t));
  handle->volBuf = MEMbufCreate();
  return (handle);
  
}

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

MDV_field_handle_t *MDV_fhand_create_copy(MDV_field_handle_t *rhs)
     
{

  MDV_field_handle_t *handle = ucalloc(1, sizeof(MDV_field_handle_t));
  handle->hdr = rhs->hdr;
  handle->volBuf = MEMbufCreateCopy(rhs->volBuf);
  return (handle);
  
}

/*************************************************************************
 *
 * MDV_fhand_create_from_parts
 *
 * Construct an object from the header and data parts.
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

MDV_field_handle_t
*MDV_fhand_create_from_parts(MDV_field_header_t *rhs_hdr,
			     void *rhs_vol_data)

{
  
  MDV_field_handle_t *handle = MDV_fhand_create_empty();
  handle->hdr = *rhs_hdr;
  if (rhs_vol_data != NULL) {
    MEMbufAdd(handle->volBuf, rhs_vol_data, rhs_hdr->volume_size);
  } else {
    MEMbufPrepare(handle->volBuf, rhs_hdr->volume_size);
  }
  return (handle);
  
}

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

MDV_field_handle_t
*MDV_fhand_create_plane_from_copy(MDV_field_handle_t *rhs,
				  int plane_num)
     
{

  void *plane_ptr;
  int in_nz;
  ui32 be_plane_offset, be_plane_len;
  ui32 in_plane_offset, plane_len;
  ui32 be_out_plane_offset;

  MDV_field_handle_t *handle = MDV_fhand_create_empty();
  in_nz = rhs->hdr.nz;
  handle->hdr = rhs->hdr;
  handle->hdr.nz = 1;
  handle->hdr.grid_minz =
    rhs->hdr.grid_minz + plane_num * rhs->hdr.grid_dz;

  if (!MDV_compressed(handle->hdr.compression_type)) {

    plane_len = 
      handle->hdr.nx * handle->hdr.ny * handle->hdr.data_element_nbytes;
    in_plane_offset = plane_len * plane_num;
    plane_ptr = (ui08 *) MEMbufPtr(rhs->volBuf) + in_plane_offset;
    MEMbufAdd(handle->volBuf, plane_ptr, plane_len);

  } else {

    be_plane_offset = ((ui32 *) MEMbufPtr(rhs->volBuf))[plane_num];
    be_plane_len = ((ui32 *) MEMbufPtr(rhs->volBuf))[in_nz + plane_num];
    in_plane_offset = BE_to_ui32(be_plane_offset);
    plane_len = BE_to_ui32(be_plane_len);

    be_out_plane_offset = BE_from_ui32(0);

    MEMbufAdd(handle->volBuf, &be_out_plane_offset, sizeof(ui32));
    MEMbufAdd(handle->volBuf, &be_plane_len, sizeof(ui32));
    plane_ptr =
      (ui08 *) MEMbufPtr(rhs->volBuf) + 2 * in_nz * sizeof(ui32) +
      in_plane_offset;
    MEMbufAdd(handle->volBuf, plane_ptr, plane_len);

  }

  return (handle);
  
}

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

MDV_field_handle_t
*MDV_fhand_create_plane_from_parts(MDV_field_header_t *rhs_hdr,
				   int plane_num,
				   void *rhs_plane_data,
				   int plane_size)
     
{
  
  MDV_field_handle_t *handle = MDV_fhand_create_empty();
  handle->hdr = *rhs_hdr;
  handle->hdr.nz = 1;
  handle->hdr.grid_minz =
    rhs_hdr->grid_minz + plane_num * rhs_hdr->grid_dz;
  if (rhs_plane_data != NULL) {
    MEMbufAdd(handle->volBuf, rhs_plane_data, plane_size);
  } else {
    MEMbufPrepare(handle->volBuf, plane_size);
  }
  return (handle);
  
}

/*************************************************************************
 *
 * MDV_fhand_delete
 *
 * Destructor
 *
 **************************************************************************/

void MDV_fhand_delete(MDV_field_handle_t *handle)

{

  MEMbufDelete(handle->volBuf);
  ufree(handle);

}

/*************************************************************************
 *
 * MDV_fhand_get_hdr
 *
 * Get pointer to field header
 *
 **************************************************************************/

MDV_field_header_t *MDV_fhand_get_hdr(MDV_field_handle_t *handle)

{
  return (&handle->hdr);
}

/*************************************************************************
 *
 * MDV_fhand_get_vol_ptr
 *
 * Get pointer to vol data
 *
 **************************************************************************/

void *MDV_fhand_get_vol_ptr(MDV_field_handle_t *handle)

{
  return (MEMbufPtr(handle->volBuf));
}

/*************************************************************************
 *
 * MDV_fhand_get_vol_len
 *
 * Get vol data len
 *
 **************************************************************************/

int MDV_fhand_get_vol_len(MDV_field_handle_t *handle)

{
  return (MEMbufLen(handle->volBuf));
}

/*************************************************************************
 * MDV_fhand_convert()
 *
 * Convert MDV data to an output encoding type, specifying
 * compression and scaling.
 *
 * The handle must contain a complete field header and data.
 *
 * Supported types are:
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
 *   MDV_COMPRESSION_ZLIB - see <toolsa/compress.h>
 *   MDV_COMPRESSION_BZIP - see <toolsa/compress.h>
 *   MDV_COMPRESSION_GZIP - see <toolsa/compress.h>
 *
 * Scaling types apply only to conversions to int types (INT8 and INT16)
 *
 * Supported scaling types are:
 *   MDV_SCALING_ROUNDED
 *   MDV_SCALING_INTEGRAL
 *   MDV_SCALING_DYNAMIC
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

int MDV_fhand_convert(MDV_field_handle_t *handle,
		      int output_encoding,
		      int output_compression,
		      int output_scaling,
		      double output_scale,
		      double output_bias)
     
{

  /*
   * check for consistency
   */
  
  if (handle->hdr.encoding_type != MDV_ASIS &&
      handle->hdr.encoding_type != MDV_INT8 &&
      handle->hdr.encoding_type != MDV_INT16 &&
      handle->hdr.encoding_type != MDV_FLOAT32) {
    fprintf(stderr, "ERROR - MDV_fhand_convert\n");
    fprintf(stderr, "  Input encoding type %d not supported\n",
	    handle->hdr.encoding_type);
    return (-1);
  }

  if (output_encoding != MDV_ASIS &&
      output_encoding != MDV_INT8 &&
      output_encoding != MDV_INT16 &&
      output_encoding != MDV_FLOAT32) {
    fprintf(stderr, "ERROR - MDV_fhand_convert\n");
    fprintf(stderr, "  Output encoding type %d not supported\n",
	    output_encoding);
    return (-1);
  }

  if (output_compression < MDV_COMPRESSION_ASIS ||
      output_compression > MDV_COMPRESSION_BZIP) {
    fprintf(stderr, "ERROR - MDV_fhand_convert\n");
    fprintf(stderr, "  Output compression type %d not supported\n",
	    output_compression);
    return (-1);
  }

  if (output_scaling < MDV_SCALING_NONE ||
      output_scaling > MDV_SCALING_SPECIFIED) {
    fprintf(stderr, "ERROR - MDV_fhand_convert\n");
    fprintf(stderr, "  Output scaling type %d not supported\n",
	    output_scaling);
    return (-1);
  }

  /*
   * for ASIS encoding, set output_encoding accordingly
   */
  
  if (output_encoding == MDV_ASIS) {
    output_encoding = handle->hdr.encoding_type;
  }

  /*
   * for ASIS compression, set output_compression accordingly
   */
  
  if (output_compression == MDV_COMPRESSION_ASIS) {
    output_compression = handle->hdr.compression_type;
  }

  /*
   * set nbytes and min/max in case not set
   */

  _set_data_element_nbytes(handle);

  if (_set_min_and_max(handle)) {
    return (-1);
  }

  /*
   * check for trivial case - input and output types are the same
   */
  
  if (output_encoding == handle->hdr.encoding_type) {

    if (output_compression == handle->hdr.compression_type) {

      return (0);
      
    } else {

      /*
       * just change compression, not encoding type
       */
      
      if (_decompress(handle)) {
	return (-1);
      }
      
      if (_compress(handle, output_compression)) {
	return (-1);
      }

      return (0);

    }

  }

  /*
   * decompress
   */
  
  if (_decompress(handle)) {
    return (-1);
  }

  /*
   * change encoding - switch on types
   */
  
  if (handle->hdr.encoding_type == MDV_INT8) {
    
    if (output_encoding == MDV_INT16) {
      
      _int8_to_int16(handle,
		     output_scaling,
		     output_scale,
		     output_bias);

    } else if (output_encoding == MDV_FLOAT32) {

      _int8_to_float32(handle);
      
    }

  } else if (handle->hdr.encoding_type == MDV_INT16) {

    if (output_encoding == MDV_INT8) {

      _int16_to_int8(handle,
		     output_scaling,
		     output_scale,
		     output_bias);
      
    } else if (output_encoding == MDV_FLOAT32) {

      _int16_to_float32(handle);
      
    }

  } else if (handle->hdr.encoding_type == MDV_FLOAT32) {

    if (output_encoding == MDV_INT8) {
      
      if (output_scaling == MDV_SCALING_SPECIFIED) {
	_float32_to_int8_specified(handle,
				   output_scale,
				   output_bias);
      } else {
	_float32_to_int8(handle, output_scaling);
      }
      
    } else if (output_encoding == MDV_INT16) {

      if (output_scaling == MDV_SCALING_SPECIFIED) {
	_float32_to_int16_specified(handle,
				   output_scale,
				   output_bias);
      } else {
	_float32_to_int16(handle, output_scaling);
      }
      
    }

  }

  /*
   * compress
   */
  
  if (_compress(handle, output_compression)) {
    return (-1);
  }
      
  return (0);

}
		       
/*************************************************************************
 * MDV_fhand_convert_rounded()
 *
 * This routine calls MDV_fhand_convert(), with ROUNDED scaling.
 * 
 * Returns 0 on success, -1 on failure.
 *
 * See MDV_fhand_convert()
 */

int MDV_fhand_convert_rounded(MDV_field_handle_t *handle,
			      int output_encoding,
			      int output_compression)

{
  return (MDV_fhand_convert(handle,
			    output_encoding,
			    output_compression,
			    MDV_SCALING_ROUNDED, 0.0, 0.0));
}
     
/*************************************************************************
 * MDV_fhand_convert_integral()
 *
 * This routine calls MDV_fhand_convert(), with INTEGRAL scaling.
 * 
 * Returns 0 on success, -1 on failure.
 *
 * See MDV_fhand_convert()
 */

int MDV_fhand_convert_integral(MDV_field_handle_t *handle,
			       int output_encoding,
			       int output_compression)

{
  return (MDV_fhand_convert(handle,
			    output_encoding,
			    output_compression,
			    MDV_SCALING_INTEGRAL, 0.0, 0.0));
}
     
/*************************************************************************
 * MDV_fhand_convert_dynamic()
 *
 * This routine calls MDV_fhand_convert(), with DYNAMIC scaling.
 * 
 * Returns 0 on success, -1 on failure.
 *
 * See MDV_fhand_convert()
 */

int MDV_fhand_convert_dynamic(MDV_field_handle_t *handle,
			      int output_encoding,
			      int output_compression)

{
  return (MDV_fhand_convert(handle,
			    output_encoding,
			    output_compression,
			    MDV_SCALING_DYNAMIC, 0.0, 0.0));
}
     
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

int MDV_fhand_composite(MDV_field_handle_t *handle,
			int lower_plane_num,
			int upper_plane_num)

{

  MEMbuf *work_buf;
  int npoints_plane = handle->hdr.nx * handle->hdr.ny;
  int nplanes;
  double lower_ht, upper_ht;

  if (lower_plane_num < 0 || lower_plane_num > handle->hdr.nz - 1) {
    lower_plane_num = 0;
  }
  if (upper_plane_num < 0 || upper_plane_num > handle->hdr.nz - 1) {
    upper_plane_num = handle->hdr.nz - 1;
  }

  if (lower_plane_num > upper_plane_num) {
    int tmp_plane_num;
    fprintf(stderr, "WARNING - MDV_fhand_composite\n");
    fprintf(stderr, "  Lower plane is above upper plane - switching\n");
    tmp_plane_num = lower_plane_num;
    lower_plane_num = upper_plane_num;
    upper_plane_num = tmp_plane_num;
  }

  nplanes = upper_plane_num - lower_plane_num;

  /*
   * make sure we have decompressed data
   */

  if (_decompress(handle)) {
    fprintf(stderr, "ERROR - MDV_fhand_composite\n");
    return (-1);
  }

  /*
   * create work buffer
   */

  work_buf = MEMbufCreate();

  /*
   * create composite depending on type
   */

  switch (handle->hdr.encoding_type) {

  case MDV_INT8:
    {

      ui08 *v, *c;
      ui08 *comp;
      ui08 val;
      int i, j;
      int nbytes_comp;

      nbytes_comp = npoints_plane * sizeof(ui08);
      comp = MEMbufPrepare(work_buf, nbytes_comp);
      memset(comp, 0, nbytes_comp);
      
      v = MEMbufPtr(handle->volBuf);

      for (j = lower_plane_num; j <= upper_plane_num; j++) {
	c = comp;
	for (i = 0; i < npoints_plane; i++, c++, v++) {
	  val = *v;
	  if (val > *c) {
	    *c = val;
	  }
	} /* i */
      } /* j */

    }
    break;

  case MDV_INT16:
    {

      ui16 *v, *c;
      ui16 *comp;
      ui16 val;
      int i, j;
      int nbytes_comp;

      nbytes_comp = npoints_plane * sizeof(ui16);
      comp = MEMbufPrepare(work_buf, nbytes_comp);
      memset(comp, 0, nbytes_comp);
      
      v = MEMbufPtr(handle->volBuf);

      for (j = lower_plane_num; j <= upper_plane_num; j++) {
	c = comp;
	for (i = 0; i < npoints_plane; i++, c++, v++) {
	  val = *v;
	  if (val > *c) {
	    *c = val;
	  }
	} /* i */
      } /* j */

    }
    break;

  case MDV_FLOAT32:
    {

      fl32 *v, *c;
      fl32 *comp;
      fl32 val;
      int i, j;
      int nbytes_comp;

      nbytes_comp = npoints_plane * sizeof(fl32);
      comp = MEMbufPrepare(work_buf, nbytes_comp);
      c = comp;
      for (i = 0; i < npoints_plane; i++, c++) {
	*c = -1.0e33;
      }
      
      v = MEMbufPtr(handle->volBuf);

      for (j = lower_plane_num; j <= upper_plane_num; j++) {
	c = comp;
	for (i = 0; i < npoints_plane; i++, c++, v++) {
	  val = *v;
	  if (val > *c) {
	    *c = val;
	  }
	} /* i */
      } /* j */

    }
    break;

  } /* switch */

  /*
   * copy data from work buf
   */

  MEMbufFree(handle->volBuf);
  MEMbufAdd(handle->volBuf, MEMbufPtr(work_buf), MEMbufLen(work_buf));
  
  /*
   * free up
   */

  MEMbufDelete(work_buf);

  /*
   * adjust header
   */

  lower_ht = handle->hdr.grid_minz + handle->hdr.grid_dz * lower_plane_num;
  upper_ht = handle->hdr.grid_minz + handle->hdr.grid_dz * upper_plane_num;
  
  handle->hdr.grid_dz *= nplanes;
  handle->hdr.grid_minz = (lower_ht + upper_ht) / 2.0;
  handle->hdr.nz = 1;

  STRconcat(handle->hdr.field_name_long, " comp", MDV_LONG_FIELD_LEN);
  STRconcat(handle->hdr.field_name, " comp", MDV_SHORT_FIELD_LEN);

  return (0);

}
     
/*************************************************************************
 *
 * MDV_fhand_data_to_BE
 *
 * Convert data buffer to BE byte ordering
 *
 **************************************************************************/

void MDV_fhand_data_to_BE(MDV_field_handle_t *handle)

{

  if (MDV_compressed(handle->hdr.compression_type)) {
    /* compressed data is already BE */
    return;
  }
  
  switch (handle->hdr.encoding_type) {

  case MDV_INT8:
    /* no need to swap byte data */
    return;
    break;

  case MDV_INT16:
    BE_from_array_16(MEMbufPtr(handle->volBuf), MEMbufLen(handle->volBuf));
    break;

  case MDV_FLOAT32:
    BE_from_array_32(MEMbufPtr(handle->volBuf), MEMbufLen(handle->volBuf));
    break;

  }

}

/*************************************************************************
 *
 * MDV_fhand_data_from_BE
 *
 * Convert data buffer from BE byte ordering
 *
 **************************************************************************/

void MDV_fhand_data_from_BE(MDV_field_handle_t *handle)
     
{

  if (MDV_compressed(handle->hdr.compression_type)) {
    /* compressed data is left in BE */
    return;
  }
  
  switch (handle->hdr.encoding_type) {

  case MDV_INT8:
    /* no need to swap byte data */
    return;
    break;

  case MDV_INT16:
    BE_to_array_16(MEMbufPtr(handle->volBuf), MEMbufLen(handle->volBuf));
    break;

  case MDV_FLOAT32:
    BE_to_array_32(MEMbufPtr(handle->volBuf), MEMbufLen(handle->volBuf));
    break;

  }

}

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

int MDV_fhand_read_vol(MDV_field_handle_t *handle,
		       FILE *infile)

{

  void *buf;
  int volume_size = handle->hdr.volume_size;
  
  fseek(infile, handle->hdr.field_data_offset, SEEK_SET);
  
  buf = MEMbufPrepare(handle->volBuf, volume_size);

  if (ufread(buf, 1, volume_size, infile)
      != volume_size) {
    fprintf(stderr, "ERROR - MDV_fhand_read_vol\n");
    fprintf(stderr, "  Cannot read field '%s'\n",
	    handle->hdr.field_name);
    return (-1);
  }

  MDV_fhand_data_from_BE(handle);

  return (0);

}

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

int MDV_fhand_read_plane(MDV_field_handle_t *handle,
			 FILE *infile,
			 int plane_num)

{

  void *buf;
  int plane_nbytes;
  ui32 be_input_offset, be_input_len;
  ui32 input_offset, input_len;
  ui32 be_output_offset;

  plane_nbytes =
    handle->hdr.nx * handle->hdr.ny * handle->hdr.data_element_nbytes;
  
  if (!MDV_compressed(handle->hdr.compression_type)) {

    input_offset = handle->hdr.field_data_offset + plane_num * plane_nbytes;
    input_len = plane_nbytes;

    /*
     * read in uncompressed plane
     */
    
    fseek(infile, input_offset, SEEK_SET);

    buf = MEMbufPrepare(handle->volBuf, input_len);
    if (ufread(buf, 1, input_len, infile) != input_len) {
      fprintf(stderr, "ERROR - MDV_fhand_read_plane\n");
      fprintf(stderr, "  Cannot read data for plane %d, field '%s'\n",
	      plane_num, handle->hdr.field_name);
      return (-1);
    }

  } else {

    MEMbuf *work_buf;
    int index_array_size = handle->hdr.nz * sizeof(ui32);
    int offset_offset =
      handle->hdr.field_data_offset + plane_num * sizeof(ui32);
    int len_offset = offset_offset + index_array_size;

    /*
     * For compressed data, we need to read in the offset and len
     * of the data from the arrays at the start of the volume.
     * So we seek into the arrays and read just the 2 values needed.
     */
    
    fseek(infile, offset_offset, SEEK_SET);

    if (ufread(&be_input_offset, sizeof(ui32), 1, infile) != 1) {
      fprintf(stderr, "ERROR - MDV_fhand_read_plane\n");
      fprintf(stderr, "  Cannot read offset for plane %d, field '%s'\n",
	      plane_num, handle->hdr.field_name);
      return (-1);
    }

    fseek(infile, len_offset, SEEK_SET);

    if (ufread(&be_input_len, sizeof(ui32), 1, infile) != 1) {
      fprintf(stderr, "ERROR - MDV_fhand_read_plane\n");
      fprintf(stderr, "  Cannot read len for plane %d, field '%s'\n",
	      plane_num, handle->hdr.field_name);
      return (-1);
    }

    /*
     * swap offset and len
     */
    
    input_offset = BE_to_ui32(be_input_offset);
    input_len = BE_to_ui32(be_input_len);

    /*
     * read in compressed plane
     */

    fseek(infile,
	  handle->hdr.field_data_offset + 2 * index_array_size + input_offset,
	  SEEK_SET);

    work_buf = MEMbufCreate();
    buf = MEMbufPrepare(work_buf, input_len);
    
    if (ufread(buf, 1, input_len, infile) != input_len) {
      fprintf(stderr, "ERROR - MDV_fhand_read_plane\n");
      fprintf(stderr, "  Cannot read data for plane %d, field '%s'\n",
	      plane_num, handle->hdr.field_name);
      MEMbufDelete(work_buf);
      return (-1);
    }

    /*
     * assemble the compressed plane in the buffer - this is byte-swapped
     */

    MEMbufFree(handle->volBuf);
    be_output_offset = BE_from_ui32(0);
    MEMbufAdd(handle->volBuf, &be_output_offset, sizeof(ui32));
    MEMbufAdd(handle->volBuf, &be_input_len, sizeof(ui32));
    MEMbufAdd(handle->volBuf, MEMbufPtr(work_buf), input_len);

    /*
     * free up
     */

    MEMbufDelete(work_buf);

  }

  /*
   * make header current
   */

  handle->hdr.nz = 1;
  handle->hdr.grid_minz += plane_num * handle->hdr.grid_dz;
  handle->hdr.volume_size = MEMbufLen(handle->volBuf);
  
  /*
   * swap the data - this only affects uncompressed data
   */
  
  MDV_fhand_data_from_BE(handle);

  return (0);

}

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

int MDV_fhand_write_vol(MDV_field_handle_t *handle,
			FILE *outfile)

{

  int volume_size = handle->hdr.volume_size;
  si32 be_volume_size = BE_from_si32(volume_size);

  /*
   * swap to BE byte ordering
   */
  
  MDV_fhand_data_to_BE(handle);

  /*
   * seek to start of fortran rec len
   */
  
  fseek(outfile, handle->hdr.field_data_offset - sizeof(be_volume_size),
	SEEK_SET);

  /*
   * Write the leading FORTRAN record to disk.
   */

  if (ufwrite(&be_volume_size, sizeof(be_volume_size),
              1, outfile) != 1) {
    fprintf(stderr, "ERROR - MDV_fhand_write_vol\n");
    fprintf(stderr, "  Cannot write fortran len for field '%s'\n",
	    handle->hdr.field_name);
    return (-1);
  }

  /*
   * Write out the data.
   */

  if (ufwrite(MEMbufPtr(handle->volBuf), 1, volume_size, outfile)
      != volume_size) {
    fprintf(stderr, "ERROR - MDV_fhand_write_vol\n");
    fprintf(stderr, "  Cannot write data for field '%s'\n",
	    handle->hdr.field_name);
    return (-1);
  }
  
  /*
   * Write the trailing FORTRAN record to disk.
   */

  if (ufwrite(&be_volume_size, sizeof(be_volume_size),
	      1, outfile) != 1) {
    fprintf(stderr, "ERROR - MDV_fhand_write_vol\n");
    fprintf(stderr, "  Cannot write fortran len for field '%s'\n",
	    handle->hdr.field_name);
    return (-1);
  }

  return (0);

}

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

void MDV_fhand_print(MDV_field_handle_t *handle,
		     FILE *out,
		     int print_native,
		     int print_labels,
		     int pack_duplicates)
     
{

  MDV_print_field_header_full(&handle->hdr, out);
  MDV_fhand_print_voldata(handle, out,
			  print_native, print_labels, pack_duplicates);

}

/*************************************************************************
 *
 * MDV_fhand_print_voldata
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

void MDV_fhand_print_voldata(MDV_field_handle_t *handle,
			     FILE *out,
			     int print_native,
			     int print_labels,
			     int pack_duplicates)
     
{

  MDV_field_handle_t *local_handle;

  /*
   * make local copy of handle
   */
  
  local_handle = MDV_fhand_create_copy(handle);

  /*
   * uncompress if needed
   */

  if (MDV_compressed(local_handle->hdr.compression_type)) {
    if  (_decompress(local_handle)) {
      return;
    }
  }

  /*
   * convert if needed
   */

  if (!print_native) {
    MDV_fhand_convert_dynamic(local_handle, MDV_FLOAT32,
			      MDV_COMPRESSION_NONE);
  }

  if (pack_duplicates) {
    _print_voldata_packed(local_handle, out, print_labels);
  } else {
    _print_voldata_verbose(local_handle, out, print_labels);
  }

  MDV_fhand_delete(local_handle);

}

/*
 * static routines
 */


/***************
 * INT8 to INT16
 */

static void _int8_to_int16(MDV_field_handle_t *handle,
			   int output_scaling,
			   double output_scale,
			   double output_bias)
     
{

  _int8_to_float32(handle);

  if (output_scaling == MDV_SCALING_SPECIFIED) {
    _float32_to_int16_specified(handle,
				output_scale,
				output_bias);
  } else {
    _float32_to_int16(handle, output_scaling);
  }
      
}

/***************
 * INT16 to INT8
 */

static void _int16_to_int8(MDV_field_handle_t *handle,
			   int output_scaling,
			   double output_scale,
			   double output_bias)
     
{

  _int16_to_float32(handle);

  if (output_scaling == MDV_SCALING_SPECIFIED) {
    _float32_to_int8_specified(handle,
			       output_scale,
			       output_bias);
  } else {
    _float32_to_int8(handle, output_scaling);
  }
  
}

/*****************
 * INT8 to FLOAT32
 */

static void _int8_to_float32(MDV_field_handle_t *handle)
     
{

  int i;
  int output_size;
  int npoints;
  ui08 *in;
  fl32 outval, *out;
  MEMbuf *copy_buf;
  fl32 scale, bias;
  double threshold_val;

  /*
   * copy the volume buffer
   */

  copy_buf = MEMbufCreateCopy(handle->volBuf);

  /*
   * allocate the output buffer
   */

  npoints = handle->hdr.nx * handle->hdr.ny * handle->hdr.nz;
  output_size = npoints * sizeof(fl32);
  MEMbufPrepare(handle->volBuf, output_size);
  
  /*
   * convert data
   */
  
  in = (ui08 *) MEMbufPtr(copy_buf);
  out = (fl32 *) MEMbufPtr(handle->volBuf);
  scale = handle->hdr.scale;
  bias = handle->hdr.bias;
  if (handle->hdr.min_value == handle->hdr.max_value) {
    threshold_val = 0.0;
  } else {
    threshold_val = fabs(scale * 0.05);
  }
  
  for (i = 0; i < npoints; i++, in++, out++) {
    outval = ((fl32) *in * scale + bias);
    if (fabs(outval) < threshold_val) {
      *out = 0.0;
    } else {
      *out = outval;
    }
  }

  /*
   * adjust header
   */

  handle->hdr.volume_size = output_size;
  handle->hdr.encoding_type = MDV_FLOAT32;
  handle->hdr.scaling_type = MDV_SCALING_NONE;
  handle->hdr.data_element_nbytes = 4;
  handle->hdr.missing_data_value =
    handle->hdr.missing_data_value * scale + bias;
  handle->hdr.bad_data_value =
    handle->hdr.bad_data_value * scale + bias;
  handle->hdr.scale = 0.0;
  handle->hdr.bias = 0.0;
  
  /*
   * free up
   */

  MEMbufDelete(copy_buf);
  
}

/******************
 * INT16 to FLOAT32
 */

static void _int16_to_float32(MDV_field_handle_t *handle)

{

  int i;
  int output_size;
  int npoints;
  ui16 *in;
  fl32 outval, *out;
  MEMbuf *copy_buf;
  fl32 scale, bias;
  double threshold_val;
  
  /*
   * copy the volume buffer
   */

  copy_buf = MEMbufCreateCopy(handle->volBuf);

  /*
   * allocate the output buffer
   */

  npoints = handle->hdr.nx * handle->hdr.ny * handle->hdr.nz;
  output_size = npoints * sizeof(fl32);
  MEMbufPrepare(handle->volBuf, output_size);
  
  /*
   * convert data
   */
  
  in = (ui16 *) MEMbufPtr(copy_buf);
  out = (fl32 *) MEMbufPtr(handle->volBuf);
  scale = handle->hdr.scale;
  bias = handle->hdr.bias;
  if (handle->hdr.min_value == handle->hdr.max_value) {
    threshold_val = 0.0;
  } else {
    threshold_val = fabs(scale * 0.05);
  }
  
  for (i = 0; i < npoints; i++, in++, out++) {
    outval = ((fl32) *in * scale + bias);
    if (fabs(outval) < threshold_val) {
      *out = 0.0;
    } else {
      *out = outval;
    }
  }

  /*
   * adjust header
   */
  
  handle->hdr.volume_size = output_size;
  handle->hdr.encoding_type = MDV_FLOAT32;
  handle->hdr.scaling_type = MDV_SCALING_NONE;
  handle->hdr.data_element_nbytes = 4;
  handle->hdr.missing_data_value =
    handle->hdr.missing_data_value * scale + bias;
  handle->hdr.bad_data_value =
    handle->hdr.bad_data_value * scale + bias;
  handle->hdr.scale = 0.0;
  handle->hdr.bias = 0.0;
  
  /*
   * free up
   */

  MEMbufDelete(copy_buf);
  
}

/******************
 * FLOAT32 to INT8
 *
 * Dynamic or rounded scaling.
 */

static void _float32_to_int8(MDV_field_handle_t *handle,
			     int output_scaling)

{

  int i;
  int output_size;
  int npoints;
  ui08 out_missing, out_bad;
  ui08 *out;
  int out_val;
  double dscale, dbias;
  double drange;
  fl32 scale, bias;
  fl32 *in;
  fl32 in_val;
  fl32 in_missing, in_bad;
  MEMbuf *copy_buf;

  /*
   * set missing and bad
   */

  in_missing = handle->hdr.missing_data_value;
  in_bad =  handle->hdr.bad_data_value;
  out_missing = 0;
  if (in_missing == in_bad) {
    out_bad = out_missing;
  } else {
    out_bad = 1;
  }

  /*
   * set scale, bias
   */

  if (handle->hdr.max_value == handle->hdr.min_value) {
    
    scale = 1.0;
    bias = handle->hdr.min_value - 5.0;
    
  } else {
    
    /*
     * compute scale and bias
     */
    
    drange = handle->hdr.max_value - handle->hdr.min_value;
    dscale = drange / 250.0;
    dbias = handle->hdr.min_value - dscale * 4.0;
    
    if (output_scaling == MDV_SCALING_ROUNDED) {
      dscale = _round_up(dscale);
      dbias = handle->hdr.min_value - dscale * 4.0;
      dbias = ((int) (dbias / dscale) - 1.0) * dscale;
    } else if (output_scaling == MDV_SCALING_INTEGRAL) {
      dscale = floor (dscale + 1.0);
      dbias = floor (dbias);
    }
    
    scale = (fl32) dscale;
    bias = (fl32) dbias;
    
  }
  
  /*
   * copy the volume buffer
   */

  copy_buf = MEMbufCreateCopy(handle->volBuf);

  /*
   * allocate the output buffer
   */

  npoints = handle->hdr.nx * handle->hdr.ny * handle->hdr.nz;
  output_size = npoints * sizeof(ui08);
  MEMbufPrepare(handle->volBuf, output_size);

  /*
   * convert data
   */
  
  in = (fl32 *) MEMbufPtr(copy_buf);
  out = (ui08 *) MEMbufPtr(handle->volBuf);

  for (i = 0; i < npoints; i++, in++, out++) {
    in_val = *in;
    if (in_val == in_missing) {
      *out = out_missing;
    } else if (in_val == in_bad) {
      *out = out_bad;
    } else {
      out_val = (int) ((in_val - bias) / scale + 0.49999);
      if (out_val > 254) {
 	*out = 254;
      } else if (out_val < 3) {
 	*out = 3;
      } else {
	*out = (ui08) out_val;
      }
    }
  } /* i */

  /*
   * adjust header
   */
  
  handle->hdr.volume_size = output_size;
  handle->hdr.encoding_type = MDV_INT8;
  handle->hdr.scaling_type = output_scaling;
  handle->hdr.data_element_nbytes = 1;
  handle->hdr.missing_data_value = out_missing;
  handle->hdr.bad_data_value = out_bad;
  handle->hdr.scale = scale;
  handle->hdr.bias = bias;
  
  /*
   * free up
   */

  MEMbufDelete(copy_buf);
  
}

/******************
 * FLOAT32 to INT8
 *
 * Specified scale and bias
 */

static void _float32_to_int8_specified(MDV_field_handle_t *handle,
				       double output_scale,
				       double output_bias)

{

  int i;
  int output_size;
  int npoints;
  int min_byte_val, max_byte_val;
  ui08 out_missing = 0, out_bad = 0;
  ui08 *out;
  int out_val;
  fl32 scale = 1.0, bias = 0.0;
  fl32 *in;
  fl32 in_val;
  fl32 in_missing, in_bad;
  MEMbuf *copy_buf;

  /*
   * check whether to put missing and bad at lower end or
   * upper end
   */

  in_missing = handle->hdr.missing_data_value;
  in_bad =  handle->hdr.bad_data_value;

  min_byte_val =
    (int) ((handle->hdr.min_value - output_bias) / output_scale + 0.49999);
  max_byte_val =
    (int) ((handle->hdr.max_value - output_bias) / output_scale + 0.49999);
  
  if (min_byte_val > 1) {
    
    if (in_missing == in_bad) {
      out_missing = 0;
      out_bad = 0;
    } else {
      out_missing = 0;
      out_bad = 1;
    }

  } else if (max_byte_val < 254) {

    if (in_missing == in_bad) {
      out_missing = 255;
      out_bad = 255;
    } else {
      out_missing = 255;
      out_bad = 254;
    }

  } else {

    fprintf(stderr, "WARNING - MDV_field_handle.\n");
    fprintf(stderr, "  _float32_to_int8_specified().\n");
    fprintf(stderr, "  Inconsistent scale, bias and data values.\n");
    fprintf(stderr, "  output_scale: %g\n", output_scale);
    fprintf(stderr, "  output_bias: %g\n", output_bias);
    fprintf(stderr, "  min_val: %g\n", handle->hdr.min_value);
    fprintf(stderr, "  max_val: %g\n", handle->hdr.max_value);
    fprintf(stderr, "  Using 0 for missing and 1 for bad value\n");
    fprintf(stderr,
	    "  Data which maps to a byte val of < 2 will be set to 2\n");
    
  } /* if (min_byte_val >= 2) */

  /*
   * copy the volume buffer
   */

  copy_buf = MEMbufCreateCopy(handle->volBuf);

  /*
   * allocate the output buffer
   */

  npoints = handle->hdr.nx * handle->hdr.ny * handle->hdr.nz;
  output_size = npoints * sizeof(ui08);
  MEMbufPrepare(handle->volBuf, output_size);

  /*
   * convert data
   */
  
  in = (fl32 *) MEMbufPtr(copy_buf);
  out = (ui08 *) MEMbufPtr(handle->volBuf);

  for (i = 0; i < npoints; i++, in++, out++) {
    in_val = *in;
    if (in_val == in_missing) {
      *out = out_missing;
    } else if (in_val == in_bad) {
      *out = out_bad;
    } else {
      out_val = (int) ((in_val - bias) / scale + 0.49999);
      if (out_missing == 0) {
	if (out_val > 255) {
	  *out = 255;
	} else if (out_val < 2) {
	  *out = 2;
	} else {
	  *out = (ui08) out_val;
	}
      } else {
	if (out_val > 253) {
	  *out = 253;
	} else if (out_val < 0) {
	  *out = 0;
	} else {
	  *out = (ui08) out_val;
	}
      }
    }
  } /* i */

  /*
   * adjust header
   */
  
  handle->hdr.volume_size = output_size;
  handle->hdr.encoding_type = MDV_INT8;
  handle->hdr.scaling_type = MDV_SCALING_SPECIFIED;
  handle->hdr.data_element_nbytes = 1;
  handle->hdr.missing_data_value = out_missing;
  handle->hdr.bad_data_value = out_bad;
  handle->hdr.scale = scale;
  handle->hdr.bias = bias;
  
  /*
   * free up
   */

  MEMbufDelete(copy_buf);
  
}

/******************
 * FLOAT32 to INT16
 *
 * Dynamic or rounded scaling
 */

static void _float32_to_int16(MDV_field_handle_t *handle,
			      int output_scaling)

{

  int i;
  int output_size;
  int npoints;
  ui16 out_missing, out_bad;
  ui16 *out;
  int out_val;
  double dscale, dbias;
  double drange;
  fl32 scale, bias;
  fl32 *in;
  fl32 in_val;
  fl32 in_missing, in_bad;
  MEMbuf *copy_buf;
  
  /*
   * set missing and bad
   */

  in_missing = handle->hdr.missing_data_value;
  in_bad =  handle->hdr.bad_data_value;
  out_missing = 0;
  if (in_missing == in_bad) {
    out_bad = out_missing;
  } else {
    out_bad = 1;
  }

  /*
   * set scale, bias
   */

  if (handle->hdr.max_value == handle->hdr.min_value) {
    
    scale = 1.0;
    bias = handle->hdr.min_value - 20.0;
    
  } else {
    
    /*
     * compute scale and bias
     */
    
    drange = handle->hdr.max_value - handle->hdr.min_value;
    dscale = drange / 65500.0;
    dbias = handle->hdr.min_value - dscale * 20.0;
    
    if (output_scaling == MDV_SCALING_ROUNDED) {
      dscale = _round_up(dscale);
      dbias = handle->hdr.min_value - dscale * 20.0;
      dbias = ((int) (dbias / dscale) - 1.0) * dscale;
    } else if (output_scaling == MDV_SCALING_INTEGRAL) {
      dscale = floor (dscale + 1.0);
      dbias = floor (dbias);
    }
    
    scale = (fl32) dscale;
    bias = (fl32) dbias;
    
  }
  
  /*
   * copy the volume buffer
   */

  copy_buf = MEMbufCreateCopy(handle->volBuf);

  /*
   * allocate the output buffer
   */

  npoints = handle->hdr.nx * handle->hdr.ny * handle->hdr.nz;
  output_size = npoints * sizeof(ui16);
  MEMbufPrepare(handle->volBuf, output_size);

  /*
   * convert data
   */
  
  in = (fl32 *) MEMbufPtr(copy_buf);
  out = (ui16 *) MEMbufPtr(handle->volBuf);

  for (i = 0; i < npoints; i++, in++, out++) {
    in_val = *in;
    if (in_val == in_missing) {
      *out = out_missing;
    } else if (in_val == in_bad) {
      *out = out_bad;
    } else {
      out_val = (int) ((in_val - bias) / scale + 0.49999);
      if (out_val > 65525) {
 	*out = 65525;
      } else if (out_val < 20) {
 	*out = 20;
      } else {
	*out = (ui16) out_val;
      }
    }
  } /* i */

  /*
   * adjust header
   */
  
  handle->hdr.volume_size = output_size;
  handle->hdr.encoding_type = MDV_INT16;
  handle->hdr.scaling_type = output_scaling;
  handle->hdr.data_element_nbytes = 2;
  handle->hdr.missing_data_value = out_missing;
  handle->hdr.bad_data_value = out_bad;
  handle->hdr.scale = scale;
  handle->hdr.bias = bias;
  
  /*
   * free up
   */

  MEMbufDelete(copy_buf);
  
}

/******************
 * FLOAT32 to INT16
 *
 * Specified scaling
 */

static void _float32_to_int16_specified(MDV_field_handle_t *handle,
					double output_scale,
					double output_bias)

{

  int i;
  int output_size;
  int npoints;
  int min_byte_val, max_byte_val;
  ui16 out_missing = 0, out_bad = 0;
  ui16 *out;
  int out_val;

  fl32 scale = 1.0, bias = 0.0;
  fl32 *in;
  fl32 in_val;
  fl32 in_missing, in_bad;
  MEMbuf *copy_buf;

  /*
   * check whether to put missing and bad at lower end or
   * upper end
   */

  in_missing = handle->hdr.missing_data_value;
  in_bad =  handle->hdr.bad_data_value;
  
  min_byte_val =
    (int) ((handle->hdr.min_value - output_bias) / output_scale + 0.49999);
  max_byte_val =
    (int) ((handle->hdr.max_value - output_bias) / output_scale + 0.49999);

  if (min_byte_val > 1) {

    if (in_missing == in_bad) {
      out_missing = 0;
      out_bad = 0;
    } else {
      out_missing = 0;
      out_bad = 1;
    }

  } else if (max_byte_val < 65534) {

    if (in_missing == in_bad) {
      out_missing = 65535;
      out_bad = 65535;
    } else {
      out_missing = 65535;
      out_bad = 65534;
    }

  } else {

    fprintf(stderr, "WARNING - MDV_field_handle.\n");
    fprintf(stderr, "  _float32_to_int16_specified().\n");
    fprintf(stderr, "  Inconsistent scale, bias and data values.\n");
    fprintf(stderr, "  output_scale: %g\n", output_scale);
    fprintf(stderr, "  output_bias: %g\n", output_bias);
    fprintf(stderr, "  min_val: %g\n", handle->hdr.min_value);
    fprintf(stderr, "  max_val: %g\n", handle->hdr.max_value);
    fprintf(stderr, "  Using 0 for missing and 1 for bad value\n");
    fprintf(stderr,
	    "  Data which maps to a byte val of < 2 will be set to 2\n");
    
  } /* if (min_byte_val >= 2)  */

  /*
   * copy the volume buffer
   */

  copy_buf = MEMbufCreateCopy(handle->volBuf);

  /*
   * allocate the output buffer
   */

  npoints = handle->hdr.nx * handle->hdr.ny * handle->hdr.nz;
  output_size = npoints * sizeof(ui16);
  MEMbufPrepare(handle->volBuf, output_size);

  /*
   * convert data
   */
  
  in = (fl32 *) MEMbufPtr(copy_buf);
  out = (ui16 *) MEMbufPtr(handle->volBuf);

  for (i = 0; i < npoints; i++, in++, out++) {
    in_val = *in;
    if (in_val == in_missing) {
      *out = out_missing;
    } else if (in_val == in_bad) {
      *out = out_bad;
    } else {
      out_val = (int) ((in_val - bias) / scale + 0.49999);
      if (out_missing == 0) {
	if (out_val > 65535) {
	  *out = 65535;
	} else if (out_val < 2) {
	  *out = 2;
	} else {
	  *out = (ui16) out_val;
	}
      } else {
	if (out_val > 65533) {
	  *out = 65533;
	} else if (out_val < 0) {
	  *out = 0;
	} else {
	  *out = (ui16) out_val;
	}
      }
    }
  } /* i */

  /*
   * adjust header
   */
  
  handle->hdr.volume_size = output_size;
  handle->hdr.encoding_type = MDV_INT16;
  handle->hdr.scaling_type = MDV_SCALING_SPECIFIED;
  handle->hdr.data_element_nbytes = 2;
  handle->hdr.missing_data_value = out_missing;
  handle->hdr.bad_data_value = out_bad;
  handle->hdr.scale = scale;
  handle->hdr.bias = bias;
  
  /*
   * free up
   */

  MEMbufDelete(copy_buf);
  
}

/**************************************
 * compression / decompression routines
 *
 * The compressed buffer comprises the following in order:
 *   Array of nz ui32s: compressed plane offsets
 *   Array of nz ui32s: compressed plane sizes
 *   All of the compressed planes packed together.
 *
 * Compressed buffer is in BE byte order.
 */

static int _compress(MDV_field_handle_t *handle,
		     int compression_type)

{

  unsigned int iz, nz;
  unsigned int npoints_plane;
  unsigned int nbytes_plane, nbytes_vol;
  unsigned int index_array_size;
  unsigned int next_offset;
  ui32 plane_offsets[MDV_MAX_VLEVELS];
  ui32 plane_sizes[MDV_MAX_VLEVELS];
  MEMbuf *work_buf;
  
  if (compression_type == handle->hdr.compression_type) {
    return (0);
  }
  
  if (MDV_compressed(handle->hdr.compression_type)) {
    if (_decompress(handle)) {
      return (-1);
    }
  }

  if (compression_type == MDV_COMPRESSION_NONE) {
    return (0);
  }

  nz = handle->hdr.nz;
  npoints_plane = handle->hdr.nx * handle->hdr.ny;
  nbytes_plane = npoints_plane * handle->hdr.data_element_nbytes;
  nbytes_vol = nbytes_plane * nz;
  index_array_size = nz * sizeof(ui32);
  next_offset = 0;

  /*
   * swap volume data to BE as appropriate
   */

  if (handle->hdr.encoding_type == MDV_INT16) {
    BE_from_array_16(MEMbufPtr(handle->volBuf), nbytes_vol);
  } else if (handle->hdr.encoding_type == MDV_FLOAT32) {
    BE_from_array_32(MEMbufPtr(handle->volBuf), nbytes_vol);
  }

  /*
   * create working buffer
   */
  
  work_buf = MEMbufCreate();

  /*
   * compress plane-by-plane
   */
  
  for (iz = 0; iz < nz; iz++) {
    
    void *compressed_plane;
    void *uncompressed_plane;
    unsigned int nbytes_compressed;
    
    uncompressed_plane = ((char *) MEMbufPtr(handle->volBuf) +
			  iz * nbytes_plane);
    
    switch (compression_type) {
      
    case MDV_COMPRESSION_RLE:
      compressed_plane = rle_compress(uncompressed_plane, nbytes_plane,
				      &nbytes_compressed);
      break;

    case MDV_COMPRESSION_LZO:
      compressed_plane = lzo_compress(uncompressed_plane, nbytes_plane,
				      &nbytes_compressed);
      break;

    case MDV_COMPRESSION_GZIP:
      compressed_plane = gzip_compress(uncompressed_plane, nbytes_plane,
				       &nbytes_compressed);
      break;
      
    case MDV_COMPRESSION_BZIP:
      compressed_plane = bzip_compress(uncompressed_plane, nbytes_plane,
				       &nbytes_compressed);
      break;

    case MDV_COMPRESSION_ZLIB:
      compressed_plane = zlib_compress(uncompressed_plane, nbytes_plane,
				       &nbytes_compressed);
      break;
      
    default:
      fprintf(stderr, "ERROR - MDV_compress_field\n");
      fprintf(stderr, "  Unknown compression type: %d\n", compression_type);
      MEMbufDelete(work_buf);
      return (-1);
    }
    
    if (compressed_plane == NULL) {
      fprintf(stderr, "ERROR - MDV_compress_field\n");
      fprintf(stderr, "  Compression failed\n");
      MEMbufDelete(work_buf);
      return (-1);
    }

    plane_offsets[iz] = next_offset;
    plane_sizes[iz] = nbytes_compressed;

    MEMbufAdd(work_buf, compressed_plane, nbytes_compressed);

    ta_compress_free(compressed_plane);

    next_offset += nbytes_compressed;

  } /* iz */

  /*
   * swap plane offset and size arrays
   */

  BE_from_array_32(plane_offsets, index_array_size);
  BE_from_array_32(plane_sizes, index_array_size);

  /*
   * assemble compressed buffer
   */
  
  MEMbufFree(handle->volBuf);
  MEMbufAdd(handle->volBuf, plane_offsets, index_array_size);
  MEMbufAdd(handle->volBuf, plane_sizes, index_array_size);
  MEMbufAdd(handle->volBuf, MEMbufPtr(work_buf), MEMbufLen(work_buf));

  /*
   * adjust header
   */

  handle->hdr.compression_type = compression_type;
  handle->hdr.volume_size = next_offset + 2 * index_array_size;
;

  /*
   * free up
   */
  
  MEMbufDelete(work_buf);
  
  return (0);

}

static int _decompress(MDV_field_handle_t *handle)

{

  unsigned int iz, nz;
  unsigned int npoints_plane, nbytes_plane, nbytes_vol;
  unsigned int index_array_size;
  ui32 *plane_offsets;
  ui32 *plane_sizes;
  MEMbuf *work_buf;

  if (!MDV_compressed(handle->hdr.compression_type)) {
    return (0);
  }

  nz = handle->hdr.nz;
  npoints_plane = handle->hdr.nx * handle->hdr.ny;
  nbytes_plane = npoints_plane * handle->hdr.data_element_nbytes;
  nbytes_vol = nz * nbytes_plane;
  index_array_size =  nz * sizeof(ui32);

  plane_offsets = (ui32 *) MEMbufPtr(handle->volBuf);
  plane_sizes = plane_offsets + nz;

  /*
   * swap offsets and sizes
   */

  BE_to_array_32(plane_offsets, index_array_size);
  BE_to_array_32(plane_sizes, index_array_size);

  /*
   * allocate work buffer
   */

  work_buf = MEMbufCreate();
  
  for (iz = 0; iz < nz; iz++) {

    void *compressed_plane;
    void *uncompressed_plane;
    ui64 nbytes_uncompressed;
    ui32 this_offset = plane_offsets[iz] + 2 * index_array_size;
    
    compressed_plane = ((char *) MEMbufPtr(handle->volBuf) + this_offset);
    
    uncompressed_plane =
      ta_decompress(compressed_plane, &nbytes_uncompressed);
    
    if (uncompressed_plane == NULL) {
      fprintf(stderr, "ERROR - MDV_decompress_field().\n");
      fprintf(stderr, "  Field not compressed.\n");
      MEMbufDelete(work_buf);
      return (-1);
    }

    if (nbytes_uncompressed != nbytes_plane) {
      fprintf(stderr, "ERROR - MDV_decompress_field().\n");
      fprintf(stderr, "  Wrong number of bytes in plane.\n");
      fprintf(stderr, "  %ld expected, %ld found.\n",
	      (long) nbytes_plane, (long) nbytes_uncompressed);
      MEMbufDelete(work_buf);
      return (-1);
    }
    
    MEMbufAdd(work_buf, uncompressed_plane, nbytes_plane);
    
    ta_compress_free(uncompressed_plane);

  } /* iz */

  /*
   * check
   */
  
  if (MEMbufLen(work_buf) != nbytes_vol) {
    fprintf(stderr, "ERROR - MDV_decompress_field().\n");
    fprintf(stderr, "  Wrong number of bytes in vol.\n");
    fprintf(stderr, "  %d expected, %ld found.\n",
	    nbytes_vol, MEMbufLen(work_buf));
    MEMbufDelete(work_buf);
    return (-1);
  }
  
  /*
   * copy work buf to volume buf
   */
  
  MEMbufReset(handle->volBuf);
  MEMbufAdd(handle->volBuf, MEMbufPtr(work_buf), nbytes_vol);

  /*
   * swap volume data from BE as appropriate
   */

  if (handle->hdr.encoding_type == MDV_INT16) {
    BE_to_array_16(MEMbufPtr(handle->volBuf), nbytes_vol);
  } else if (handle->hdr.encoding_type == MDV_FLOAT32) {
    BE_to_array_32(MEMbufPtr(handle->volBuf), nbytes_vol);
  }

  /*
   * update header
   */

  handle->hdr.compression_type = MDV_COMPRESSION_NONE;
  handle->hdr.volume_size = nbytes_vol;

  /*
   * free up
   */

  MEMbufDelete(work_buf);
  
  return (0);

}


/**************************
 * _set_data_element_nbytes
 */

static void _set_data_element_nbytes(MDV_field_handle_t *handle)

{

  if (handle->hdr.encoding_type == MDV_INT8) {
    handle->hdr.data_element_nbytes = 1;
  } else if (handle->hdr.encoding_type == MDV_INT16) {
    handle->hdr.data_element_nbytes = 2;
  } else if (handle->hdr.encoding_type == MDV_FLOAT32) {
    handle->hdr.data_element_nbytes = 4;
  }

}

/******************
 * _set_min_and_max
 */

static int _set_min_and_max(MDV_field_handle_t *handle)

{

  int i;
  int npoints;

  /*
   * don't do it if already done
   */
  
  if (handle->hdr.min_value != 0.0 || handle->hdr.max_value != 0.0) {
    return (0);
  }

  /*
   * decompress data if necessary
   */
  
  if (_decompress(handle)) {
    return (-1);
  }

  npoints = handle->hdr.nx * handle->hdr.ny * handle->hdr.nz;

  if (handle->hdr.encoding_type == MDV_INT8) {

    ui08 *val = (ui08 *) MEMbufPtr(handle->volBuf);
    ui08 min_val = 255;
    ui08 max_val = 0;
    ui08 this_val;
    ui08 missing = (ui08) handle->hdr.missing_data_value;
    ui08 bad = (ui08) handle->hdr.bad_data_value;

    for (i = 0; i < npoints; i++, val++) {
      this_val = *val;
      if (this_val != missing && this_val != bad) {
	min_val = MIN(min_val, this_val);
	max_val = MAX(max_val, this_val);
      }
    }

    if (min_val <= max_val) {
      handle->hdr.min_value =
	min_val * handle->hdr.scale + handle->hdr.bias;
      handle->hdr.max_value =
	max_val * handle->hdr.scale + handle->hdr.bias;
    }

  } else if (handle->hdr.encoding_type == MDV_INT16) {

    ui16 *val = (ui16 *) MEMbufPtr(handle->volBuf);
    ui16 min_val = 255;
    ui16 max_val = 0;
    ui16 this_val;
    ui16 missing = (ui16) handle->hdr.missing_data_value;
    ui16 bad = (ui16) handle->hdr.bad_data_value;
   
    for (i = 0; i < npoints; i++, val++) {
      this_val = *val;
      if (this_val != missing && this_val != bad) {
	min_val = MIN(min_val, this_val);
	max_val = MAX(max_val, this_val);
      }
    }
    
    if (min_val <= max_val) {
      handle->hdr.min_value =
	min_val * handle->hdr.scale + handle->hdr.bias;
      handle->hdr.max_value =
	max_val * handle->hdr.scale + handle->hdr.bias;
    }

  } else if (handle->hdr.encoding_type == MDV_FLOAT32) {

    fl32 *val = (fl32 *) MEMbufPtr(handle->volBuf);
    fl32 min_val = 1.0e33;
    fl32 max_val = -1.0e33;
    fl32 this_val;
    fl32 missing = handle->hdr.missing_data_value;
    fl32 bad = handle->hdr.bad_data_value;
    
    for (i = 0; i < npoints; i++, val++) {
      this_val = *val;
      if (this_val != missing && this_val != bad) {
	min_val = MIN(min_val, this_val);
	max_val = MAX(max_val, this_val);
      }
    }
    
    if (min_val <= max_val) {
      handle->hdr.min_value = min_val;
      handle->hdr.max_value = max_val;
    }

  }

  return (0);

}

/*****************************************
 * round a value up to the next 2, 5 or 10
 */

static double _round_up(double z)

{

  double r, R;
  double l, L;
  double integral, fract;
  double rfract;

  double _log2 = log10(2.0);
  double _log5 = log10(5.0);

  l = log10(z);
  L = l - (-1000.0);

  fract = modf(L, &integral);

  if (fract == 0.0) {
    rfract = 0.0;
  } else if (fract <= _log2) {
    rfract = _log2;
  } else if (fract <= _log5) {
    rfract = _log5;
  } else {
    rfract = 1.0;
  }

  R = integral + rfract - 1000.0;
  
  r = pow(10.0, R);

  return (r);

}

static void _print_voldata_verbose(MDV_field_handle_t *handle,
				   FILE *out,
				   int print_labels)

{     

  int npoints_plane = handle->hdr.nx * handle->hdr.ny;

  switch (handle->hdr.encoding_type) {

  case MDV_INT8:
    {
      ui08 *val = MEMbufPtr(handle->volBuf);
      ui08 this_val;
      int i, iz;
      ui08 missing = (ui08) handle->hdr.missing_data_value;
      ui08 bad = (ui08) handle->hdr.bad_data_value;

      for (iz = 0; iz < handle->hdr.nz; iz++) {
	if (print_labels) {
	  fprintf(out, "INT8 data for plane %d:\n", iz);
	}
	for (i = 0; i < npoints_plane; i++, val++) {
	  this_val = *val;
	  if (this_val == bad) {
	    fprintf(out, "BAD ");
	  } else if (this_val == missing) {
	    fprintf(out, "MISS ");
	  } else {
	    fprintf(out, "%d ", *val);
	  }
	}
	fprintf(out, "\n\n");
      } /* iz */

    }
  break;

  case MDV_INT16:
    {
      ui16 *val = MEMbufPtr(handle->volBuf);
      int i, iz;
      ui16 missing = (ui16) handle->hdr.missing_data_value;
      ui16 bad = (ui16) handle->hdr.bad_data_value;
      ui16 this_val;

      for (iz = 0; iz < handle->hdr.nz; iz++) {
	if (print_labels) {
	  fprintf(out, "INT16 data for plane %d:\n", iz);
	}
	for (i = 0; i < npoints_plane; i++, val++) {
	  this_val = *val;
	  if (this_val == bad) {
	    fprintf(out, "BAD ");
	  } else if (this_val == missing) {
	    fprintf(out, "MISS ");
	  } else {
	    fprintf(out, "%d ", *val);
	  }
	}
	fprintf(out, "\n\n");
      } /* iz */

    }
  break;

  case MDV_FLOAT32:
    {
      fl32 *val = MEMbufPtr(handle->volBuf);
      int i, iz;
      fl32 missing = handle->hdr.missing_data_value;
      fl32 bad = handle->hdr.bad_data_value;
      fl32 this_val;

      for (iz = 0; iz < handle->hdr.nz; iz++) {
	if (print_labels) {
	  fprintf(out, "FLOAT32 data for plane %d:\n", iz);
	}
	for (i = 0; i < npoints_plane; i++, val++) {
	  this_val = *val;
	  if (this_val == bad) {
	    fprintf(out, "BAD ");
	  } else if (this_val == missing) {
	    fprintf(out, "MISS ");
	  } else {
	    if (fabs(*val) > 0.01) {
	      fprintf(out, "%.3f ", *val);
	    } else {
	      fprintf(out, "%.3e ", *val);
	    }
	  }
	}
	fprintf(out, "\n\n");
      } /* iz */

    }
  break;

  } /* switch */

}

static void _print_voldata_packed(MDV_field_handle_t *handle,
				  FILE *out,
				  int print_labels)

{     

  int npoints_plane = handle->hdr.nx * handle->hdr.ny;

  switch (handle->hdr.encoding_type) {

  case MDV_INT8:
    {
      int i, iz, count;
      ui08 *val = MEMbufPtr(handle->volBuf);
      ui08 missing = (ui08) handle->hdr.missing_data_value;
      ui08 bad = (ui08) handle->hdr.bad_data_value;
      ui08 this_val;
      ui08 prev_val = 0;
      int printed;

      for (iz = 0; iz < handle->hdr.nz; iz++) {
	if (print_labels) {
	  fprintf(out, "INT8 data for plane %d:\n", iz);
	}
	printed = 0;
	count = 1;
	prev_val = *val;
	val++;
	for (i = 1; i < npoints_plane; i++, val++) {
	  this_val = *val;
	  if (this_val != prev_val) {
	    _print_int8_packed(out, count, prev_val, bad, missing);
	    printed++;
	    if (printed > 8) {
	      fprintf(out, "\n");
	      printed = 0;
	    }
	    prev_val = this_val;
	    count = 1;
	  } else {
	    count++;
	  }
	} /* i */
	_print_int8_packed(out, count, prev_val, bad, missing);
	fprintf(out, "\n\n");
      } /* iz */

    }
  break;

  case MDV_INT16:
    {
      int i, iz, count;
      ui16 *val = MEMbufPtr(handle->volBuf);
      ui16 missing = (ui16) handle->hdr.missing_data_value;
      ui16 bad = (ui16) handle->hdr.bad_data_value;
      ui16 this_val;
      ui16 prev_val = 0;
      int printed;

      for (iz = 0; iz < handle->hdr.nz; iz++) {
	if (print_labels) {
	  fprintf(out, "INT16 data for plane %d:\n", iz);
	}
	printed = 0;
	count = 1;
	prev_val = *val;
	val++;
	for (i = 1; i < npoints_plane; i++, val++) {
	  this_val = *val;
	  if (this_val != prev_val) {
	    _print_int16_packed(out, count, prev_val, bad, missing);
	    printed++;
	    if (printed > 7) {
	      fprintf(out, "\n");
	      printed = 0;
	    }
	    prev_val = this_val;
	    count = 1;
	  } else {
	    count++;
	  }
	} /* i */
	_print_int16_packed(out, count, prev_val, bad, missing);
	fprintf(out, "\n\n");
      } /* iz */

    }
  break;

  case MDV_FLOAT32:
    {
      int i, iz, count;
      fl32 *val = MEMbufPtr(handle->volBuf);
      fl32 missing = handle->hdr.missing_data_value;
      fl32 bad = handle->hdr.bad_data_value;
      fl32 this_val;
      fl32 prev_val = 0.0;
      int printed;

      for (iz = 0; iz < handle->hdr.nz; iz++) {
	if (print_labels) {
	  fprintf(out, "FLOAT32 data for plane %d:\n", iz);
	}
	printed = 0;
	count = 1;
	prev_val = *val;
	val++;
	for (i = 1; i < npoints_plane; i++, val++) {
	  this_val = *val;
	  if (this_val != prev_val) {
	    _print_float32_packed(out, count, prev_val, bad, missing);
	    printed++;
	    if (printed > 6) {
	      fprintf(out, "\n");
	      printed = 0;
	    }
	    prev_val = this_val;
	    count = 1;
	  } else {
	    count++;
	  }
	} /* i */
	_print_float32_packed(out, count, prev_val, bad, missing);
	fprintf(out, "\n\n");
      } /* iz */
      
    }
  break;

  } /* switch */

}

static void _print_int8_packed(FILE *out, int count,
			       ui08 val, ui08 bad, ui08 missing)

{

  if (count > 1) {
    fprintf(out, "%d*", count);
  }
  if (val == missing) {
    fprintf(out, "MISS ");
  } else if (val == bad) {
    fprintf(out, "BAD ");
  } else {
    fprintf(out, "%.3d ", val);
  }

}

static void _print_int16_packed(FILE *out, int count,
				ui16 val, ui16 bad, ui16 missing)

{

  if (count > 1) {
    fprintf(out, "%d*", count);
  }
  if (val == missing) {
    fprintf(out, "MISS ");
  } else if (val == bad) {
    fprintf(out, "BAD ");
  } else {
    fprintf(out, "%.5d ", val);
  }

}

static void _print_float32_packed(FILE *out, int count,
				  fl32 val, fl32 bad, fl32 missing)

{

  if (count > 1) {
    fprintf(out, "%d*", count);
  }
  if (val == missing) {
    fprintf(out, "MISS ");
  } else if (val == bad) {
    fprintf(out, "BAD ");
  } else {
    if (fabs(val) > 0.01) {
      fprintf(out, "%.3f ", val);
    } else if (val == 0.0) {
      fprintf(out, "0.0 ");
    } else {
      fprintf(out, "%.3e ", val);
    }
  }

}

