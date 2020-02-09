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
 *  MDV_CONVERT.C  Subroutines for converting data between the supported
 *                 MDV data formats.
 *
 *  N. Rehak, Feb. 1997
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#include <toolsa/os_config.h>
#include <toolsa/compress.h>

#include <dataport/port_types.h>
#include <toolsa/mem.h>
#include <toolsa/umisc.h>

#include <Mdv/mdv/mdv_macros.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_utils.h>

/*
 * Forward declarations to static routines
 */

/* Converts MDV_FLOAT32 data to MDV_INT8 format, calculating new scale
 * and bias values */

static ui08 *convert_float32_to_int8_scaled(ui08 *input_data,
					    MDV_field_header_t *field_hdr);

/* Converts MDV_PLANE_RLE8 data to MDV_INT8 format */
static ui08 *convert_plane_rle8_to_int8(ui08 *plane_rle8_data,
					int plane_rle8_size,
					int nx,
					int ny,
					int nz,
					int *int8_size);

/******************************************************************************
 * MDV_CONVERT_VOLUME: Allocate space for a data volume (data for all levels)
 * and convert the given volume of data into the specified format. Caller is 
 * responsible for freeing up the returned buffer when done.
 *
 * Inputs: input_volume - pointer to the input volume of data.
 *         input_volume_size - number of bytes in the input volume.
 *         nx - grid size in the x direction.
 *         ny - grid size in the y direction.
 *         nz - grid size in the z direction.
 *         input_volume_format - the format of the input data.
 *         return_volume_format - the desired format for the returned data.
 *
 * Outputs: return_volume_size - number of bytes in returned volume.
 *
 * Returns: returns a pointer to the converted data volume information, or
 *          NULL if there is an error.  The space for the returned data volume
 *          is allocated by this routine and must be freed by the calling
 *          routine.
 */

ui08 *MDV_convert_volume(ui08 *input_volume,
			 int input_volume_size,
			 int nx,
			 int ny,
			 int nz,
			 int input_volume_format,
			 int return_volume_format,
			 int *return_volume_size)
{
  static char *routine_name = "MDV_convert_volume";
  
  ui08 *return_volume = NULL;              /* buffer returned to caller */
  
  /*
   * Make sure there was some input data
   */

  if (input_volume == NULL)
  {
    *return_volume_size = 0;
    return(NULL);
  }
  
  /*
   * If the output format is the same as the input format, just
   * copy the given buffer and return.
   */

  if (input_volume_format == return_volume_format)
  {
    if ((return_volume = (ui08 *)umalloc(input_volume_size)) == NULL)
    {
      fprintf(stderr,
	      "%s: Error allocating %d bytes for return volume\n",
	      routine_name,
	      input_volume_size);
      *return_volume_size = 0;
      return(NULL);
    }
    
    memcpy(return_volume, input_volume, input_volume_size);
    *return_volume_size = input_volume_size;
    return(return_volume);
  } /* endif - return_volume_format == input_volume_format */
  

  /*
   * Now do any of the conversions we have implemented.
   */

  if (input_volume_format == MDV_PLANE_RLE8 &&
      return_volume_format == MDV_INT8)
  {
    return_volume = convert_plane_rle8_to_int8(input_volume,
					       input_volume_size,
					       nx,
					       ny,
					       nz,
					       return_volume_size);
  }
  else
  {
    fprintf(stderr,
	    "%s: Error -- not yet converting %s data to %s format\n",
	    routine_name,
	    MDV_encode2string(input_volume_format),
	    MDV_encode2string(return_volume_format));
    *return_volume_size = 0;
    return(NULL);
  }
   
  return(return_volume); 
}


/******************************************************************************
 * MDV_CONVERT_VOLUME_SCALED: Allocate space for a data volume (data for
 * all levels) and convert the given volume of data into the specified format,
 * calculating scale and bias to scale the data into the new format.  The
 * given field header is updated to reflect the updated volume.  Caller is 
 * responsible for freeing up the returned buffer when done.
 *
 * Inputs: input_volume - pointer to the input volume of data.
 *         return_volume_format - the desired format for the returned data.
 *         field_hdr - field header for the data.  Should match the input
 *                     volume on entry.
 *
 * Outputs: field_hdr -   Will be updated to match the output volume on exit.
 *
 * Returns: returns a pointer to the converted data volume information, or
 *          NULL if there is an error.  The space for the returned data volume
 *          is allocated by this routine and must be freed by the calling
 *          routine.
 *
 * Note: This routine will currently only convert MDV_FLOAT32 volume data
 *       to MDV_INT8 format.
 */

ui08 *MDV_convert_volume_scaled(ui08 *input_volume,
				int return_volume_format,
				MDV_field_header_t *field_hdr)
{
  static char *routine_name = "MDV_convert_volume_scaled()";
  
  if (return_volume_format != MDV_INT8)
  {
    fprintf(stderr,
	    "ERROR: %s\n", routine_name);
    fprintf(stderr,
	    "Conversion to format %s not yet implemented\n",
	    MDV_encode2string(return_volume_format));
    
    return((ui08 *)NULL);
  }
  
  if (field_hdr->encoding_type != MDV_FLOAT32)
  {
    fprintf(stderr,
	    "ERROR: %s\n", routine_name);
    fprintf(stderr,
	    "Conversion from format %s not yet implemented\n",
	    MDV_encode2string(field_hdr->encoding_type));
    
    return((ui08 *)NULL);
  }
  
  switch (field_hdr->encoding_type)
  {
  case MDV_FLOAT32 :
    return(convert_float32_to_int8_scaled(input_volume,
					  field_hdr));
    
  default:
    fprintf(stderr, "ERROR: %s\n", routine_name);
    fprintf(stderr, "Unknown input encoding type %d\n",
	    field_hdr->encoding_type);
    
    return((ui08 *)NULL);
  }
  
}


/********************************************************
 * STATIC ROUTINES
 ********************************************************/

/********************************************************
 * CONVERT_FLOAT32_TO_INT8_SCALED
 * Converts a volume of data in MDV_FLOAT32 format to MDV_INT8
 * format, calculating the scale and bias that will fit the
 * values into the MDV_INT8 range.
 *
 * Returns a pointer to the converted data, or NULL on error.
 * The caller must free the space allocated.
 */

static ui08 *convert_float32_to_int8_scaled(ui08 *input_data,
					    MDV_field_header_t *field_hdr)
{
  static char *routine_name = "convert_float32_to_int8_scaled";

  fl32 *float_data = (fl32 *)input_data;

  int num_values = field_hdr->nx * field_hdr->ny * field_hdr->nz;
  int i;
  
  double data_min = 0;
  double data_max = 0;
  double data_found = FALSE;

  double scale_min = 1.0;
  double scale_max = 255.0;
  double scale_missing = 0.0;
  
  double scale;
  double bias;
  
  ui08 *return_data;
  
  /*
   * Find the data min and max values.
   */

  for (i = 0; i < num_values; i++)
  {
    if (float_data[i] != field_hdr->bad_data_value &&
	float_data[i] != field_hdr->missing_data_value)
    {
      double data_value = (float_data[i] * field_hdr->scale) + field_hdr->bias;
	
      if (data_found)
      {
	if (data_value < data_min)
	  data_min = data_value;

	if (data_value > data_max)
	  data_max = data_value;
      }
      else
      {
	data_min = data_value;
	data_max = data_value;
	
	data_found = TRUE;
      }
    }
    
  } /* endfor - i */
  
  if (!data_found)
  {
    fprintf(stderr,
	    "%s: Could not calculate scale/bias -- no non-missing data found\n",
	    routine_name);
    fprintf(stderr,
	    "         Using 1.0 for scale and 0.0 for bias\n");
  }
  
  /*
   * Calculate the new scale and bias.
   */

  if (!data_found || data_min == data_max)
  {
    scale = 1.0;
    bias = 0.0;
  }
  else
  {
    scale = (data_max - data_min) / (scale_max - scale_min);
    bias = data_min - (scale_min * scale);
  }
  
  /*
   * Create the new data volume.
   */

  return_data = (ui08 *)umalloc(num_values * sizeof(ui08));
  
  for (i = 0; i < num_values; i++)
  {
    if (float_data[i] == field_hdr->bad_data_value ||
	float_data[i] == field_hdr->missing_data_value)
    {
      return_data[i] = (ui08)scale_missing;
    }
    else
    {
      double data_value = (float_data[i] * field_hdr->scale) + field_hdr->bias;
      double scaled_value = (data_value - bias) / scale;
      
      if (scaled_value >= scale_max)
	return_data[i] = (ui08)scale_max;
      else if (scaled_value <= scale_min)
	return_data[i] = (ui08)scale_min;
      else
      {
	if (scaled_value < 0.0)
	  return_data[i] = (ui08)(scaled_value - 0.5);
	else
	  return_data[i] = (ui08)(scaled_value + 0.5);
      }
    }
  } /* endfor - i */
  
  /*
   * Update the field header values.
   */

  field_hdr->scale = scale;
  field_hdr->bias = bias;

  field_hdr->bad_data_value = scale_missing;
  field_hdr->missing_data_value = scale_missing;
  
  field_hdr->encoding_type = MDV_INT8;
  field_hdr->data_element_nbytes = MDV_data_element_size(MDV_INT8);
  field_hdr->volume_size = num_values * field_hdr->data_element_nbytes;
  
  return(return_data);
}


/********************************************************
 * CONVERT_PLANE_RLE8_TO_INT8
 * Converts a volume of data in MDV_PLANE_RLE8 format to MDV_INT8
 * format.
 *
 * Returns a pointer to the converted data, or NULL on error.
 * The caller must free the space allocated.
 */

static ui08 *convert_plane_rle8_to_int8(ui08 *plane_rle8_data,
					int plane_rle8_size,
					int nx,
					int ny,
					int nz,
					int *int8_size)
{
  static char *routine_name = "convert_plane_rle8_to_int8";

  si32 *vlevel_loc;
    
  ui08 *int8_data;
  
  ui08 *encoded_volume;
  ui08 *decoded_plane;
  ui08 *buf_ptr;
  int plane_size = nx * ny * sizeof(ui08);
  int vlevel;
      
  /*
   * Allocate space for the return buffer.
   */

  if ((int8_data = (ui08 *)umalloc(nz * plane_size)) == NULL)
  {
    fprintf(stderr,
	    "%s: Error allocating %d bytes for decoded volume\n",
	    routine_name, nz * plane_size);
    *int8_size = 0;
    return(NULL);
  }
   
  *int8_size = nz * plane_size;
  
  /*
   * Set the vlevel locations and sizes and the data location pointer.
   */

  vlevel_loc = (si32 *)plane_rle8_data;
  encoded_volume = plane_rle8_data + (2 * nz * sizeof(si32));
  
  /*
   * Decode each plane and copy it into the return buffer.
   */

  buf_ptr = int8_data;
      
  for (vlevel = 0; vlevel < nz; vlevel++)
  {
    ui32 nbytes_decoded;
	
    /*
     * Decode the plane of data.
     */

    if ((decoded_plane = uRLDecode8(encoded_volume + vlevel_loc[vlevel],
				    &nbytes_decoded)) == NULL)
    {
      fprintf(stderr,
	      "%s: Error decoding plane %d\n",
	      routine_name, vlevel);
      ufree(int8_data);
      *int8_size = 0;
      return(NULL);
    }
	
    if (nbytes_decoded != plane_size)
    {
      fprintf(stderr,
	      "%s: Error in decoding, decoded plane has %d bytes, should have %d bytes\n",
	      routine_name, nbytes_decoded, plane_size);
      ufree(int8_data);
      ufree(decoded_plane);
      *int8_size = 0;
      return(NULL);
    }
	
    /*
     * Copy the plane of data into the volume buffer.
     */

    memcpy((char *)buf_ptr, (char *)decoded_plane, plane_size);
    buf_ptr += plane_size;
    ufree(decoded_plane);
	
  } /* endfor - vlevel */
      
  return(int8_data);
  
} /* end convert_plane_rle8_to_int8 */


