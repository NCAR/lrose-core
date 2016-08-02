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
 *  MDV_READ.C  Subroutines for reading data from MDV files.
 *  F. Hage.  Dec 1993. RAP, R. Ames 6/96.
 *
 *  Separated out from mdv_user.c.  N. Rehak, Aug. 1996.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <toolsa/os_config.h>

#include <dataport/bigend.h>
#include <dataport/swap.h>

#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_field_handle.h>

#include <rapformats/dobson.h>

#include "mdv_private.h"

/******************************************************************************
 * MDV_VERIFY:
 *
 * Verify that a file is in MDV format
 *
 * Returns: TRUE or FALSE
 */
 
int MDV_verify( char * infile_name) {

  static char *routine_name = "MDV_verify";
  
  FILE *fp;
  MDV_master_header_t header;
  struct stat file_stat;

  /* Do some sanity checking */

  if (infile_name == NULL || infile_name[0] == '\0') {
    fprintf(stderr, "%s: Input file name not specified.\n",
	    routine_name);
    return (FALSE);
  }
  
  /* open file */

  if ((fp = ta_fopen_uncompress(infile_name,"r")) == NULL) {
    fprintf(stderr,"%s: Error opening file: %s\n",
	    routine_name, infile_name);
    return (FALSE);
  }
  
  /* make sure there is enough file for the header */

  if (fstat(fileno(fp), &file_stat) != 0) {
    fprintf(stderr,"%s: Could not stat file input file %s.\n",
	    routine_name, infile_name);
    fclose(fp);
    return (FALSE);
  }
  
  if (file_stat.st_size < sizeof(MDV_master_header_t)) {
    fclose(fp);
    return (FALSE);
  }
  
  /* Read in the header */

  if (ufread(&header, sizeof(MDV_master_header_t),
		1, fp) != 1) {
    fprintf(stderr,
	    "%s: Error reading in file header\n", routine_name);
    perror(infile_name);
    fclose(fp);
    return (FALSE);
  }
  
  fclose(fp);
  
  /*
   * Swap the data in the master header.
   */
  
  MDV_master_header_from_BE(&header);
  
  if (header.struct_id == MDV_MASTER_HEAD_MAGIC_COOKIE) {
    return (TRUE);
  } else {
    return(FALSE);
  }
  
}

  
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

int MDV_load_master_header( FILE *infile, MDV_master_header_t *m_hdr)

{
  /* master headers are always at the beginning */
  if (fseek(infile,0,SEEK_SET)) {
    return MDV_FAILURE;
  }

  if((ufread(m_hdr,sizeof(MDV_master_header_t),1,infile)) != 1) {
    return MDV_FAILURE;
  }

  MDV_master_header_from_BE(m_hdr);
  
  return MDV_SUCCESS;

}


/******************************************************************************
 * MDV_LOAD_FIELD_HEADER: Load mdv field header data into the given structure
 * from disk.  Memory for the field header is assumed to be allocated before
 * this routine is called.  The bytes in the header are swapped if necessary
 * to put the data in native format.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         f_hdr -  pointer to the field header to be loaded.
 *         field_num - number of the field being loaded.  This is used to
 *                     determine the position on disk where the field header
 *                     information is located.
 *
 * Outputs: f_hdr - updated to include the values read in from disk, byte
 *                  swapped as necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_load_field_header(FILE *infile, MDV_field_header_t *f_hdr,
			  int field_num)

{
  long hdr_offset;
  
  hdr_offset = sizeof(MDV_master_header_t) +
    (field_num * sizeof(MDV_field_header_t));

  if (fseek(infile, hdr_offset, SEEK_SET)) {
    return MDV_FAILURE;
  }
  
  if((ufread(f_hdr, sizeof(MDV_field_header_t),1,infile)) != 1) {
    return MDV_FAILURE;
  }
  
  MDV_field_header_from_BE(f_hdr);

  /*
   * check that min/max is set in header. If not, read in 
   * field and do a null conversion, which forces the
   * min and max to be set. Copy the values to the field header.
   */

  if (f_hdr->min_value == 0.0 && f_hdr->max_value == 0.0) {
    MDV_field_handle_t *fhand = MDV_fhand_create_from_parts(f_hdr, NULL);
    if (MDV_fhand_read_vol(fhand, infile)) {
      return MDV_FAILURE;
    }
    MDV_fhand_convert(fhand, MDV_ASIS, MDV_COMPRESSION_ASIS,
		      MDV_SCALING_ROUNDED, 0.0, 0.0);
    f_hdr->min_value = MDV_fhand_get_hdr(fhand)->min_value;
    f_hdr->max_value = MDV_fhand_get_hdr(fhand)->max_value;
    MDV_fhand_delete(fhand);
  }

  return MDV_SUCCESS;
}


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
 
int MDV_load_vlevel_header( FILE *infile, MDV_vlevel_header_t *v_hdr, 
                             MDV_master_header_t *m_hdr, int field_num)

{

  long hdr_offset;
 
  /*
   * Make sure that vlevel headers are included in this data.
   */
  
  if (!m_hdr->vlevel_included)
    return MDV_FAILURE;
     
  hdr_offset = m_hdr->vlevel_hdr_offset + 
    (field_num * sizeof(MDV_vlevel_header_t));
 
  if (fseek(infile,hdr_offset,SEEK_SET)) {
    return MDV_FAILURE;
  }

  if((ufread(v_hdr,sizeof(MDV_vlevel_header_t),1,infile)) != 1) {
    return MDV_FAILURE;
  }
  
  MDV_vlevel_header_from_BE(v_hdr);
  
  return MDV_SUCCESS;

}


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
 
int MDV_load_vlevel_header_offset(FILE *infile,
				  MDV_vlevel_header_t *v_hdr, 
				  int vlevel_offset,
				  int field_num)
{

  long hdr_offset;
  
  hdr_offset = vlevel_offset + 
    (field_num * sizeof(MDV_vlevel_header_t));
  
  if (fseek(infile, hdr_offset, SEEK_SET)) {
    return MDV_FAILURE;
  }

  if ((ufread(v_hdr, sizeof(MDV_vlevel_header_t), 1, infile)) != 1)  {
    return MDV_FAILURE;
  }

  MDV_vlevel_header_from_BE(v_hdr);
  
  return MDV_SUCCESS;

}


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
 
int MDV_load_field_vlevel_header(FILE *infile,
				 MDV_field_vlevel_header_t *fv_hdr,
				 MDV_master_header_t *m_hdr, int field_num)
{

  long hdr_offset;
  
  hdr_offset = m_hdr->field_hdr_offset + 
    (field_num * sizeof(MDV_field_header_t));

  if (fseek(infile,hdr_offset,SEEK_SET)) {
    return MDV_FAILURE;
  }

  /* allocate/realloc space if not already allocated */
  fv_hdr->fld_hdr = (MDV_field_header_t *)
    MDV_recalloc(fv_hdr->fld_hdr,1, sizeof(MDV_field_header_t));

  /* read in info */
  if((ufread(fv_hdr->fld_hdr,sizeof(MDV_field_header_t),1,infile)) != 1) {
    return MDV_FAILURE;
  }

  MDV_field_header_from_BE(fv_hdr->fld_hdr);
  
  /* check if vlevel information included */
  if (m_hdr->vlevel_included) {

    fv_hdr->vlv_hdr = (MDV_vlevel_header_t *)
      MDV_recalloc(fv_hdr->vlv_hdr,1, sizeof(MDV_vlevel_header_t));

    hdr_offset = m_hdr->vlevel_hdr_offset + 
      (field_num*sizeof(MDV_vlevel_header_t));
  
    if (fseek(infile,hdr_offset,SEEK_SET)) {
      return MDV_FAILURE;
    }

    if((ufread(fv_hdr->vlv_hdr,sizeof(MDV_vlevel_header_t),1,infile)) != 1) {
      return MDV_FAILURE;
    }

    MDV_vlevel_header_from_BE(fv_hdr->vlv_hdr);

  } else {  /* vlevel info not included */

    if (fv_hdr->vlv_hdr != NULL) {
      ufree(fv_hdr->vlv_hdr);
    }
      
    fv_hdr->vlv_hdr = NULL;

  }  /*end else there aren't any vlevel headers */
 
  return MDV_SUCCESS;

}


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
 
int MDV_load_chunk_header(FILE *infile, MDV_chunk_header_t *c_hdr, 
			  MDV_master_header_t *m_hdr, int chunk_num)
{

  long hdr_offset;
 
  hdr_offset = m_hdr->chunk_hdr_offset + 
    (chunk_num*sizeof(MDV_chunk_header_t));
 
  if(fseek(infile,hdr_offset,SEEK_SET)) {
    return (MDV_FAILURE);
  }
 
  if((ufread(c_hdr,sizeof(MDV_chunk_header_t),1,infile)) != 1) {
    return MDV_FAILURE;
  }

  MDV_chunk_header_from_BE(c_hdr);
  
  return MDV_SUCCESS;

}


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

void * MDV_read_field_volume(FILE *infile,
			     MDV_field_header_t *f_hdr,
			     int output_encoding,
			     int output_compression,
			     int output_scaling,
			     double output_scale,
			     double output_bias,
			     int *output_volume_len)
     
{

  MDV_field_handle_t *fhand;
  void *out_buf;
  int out_len;

  /*
   * create a field_handle, read in the field
   */

  fhand = MDV_fhand_create_from_parts(f_hdr, NULL);

  /*
   * read in field data
   */

  if (MDV_fhand_read_vol(fhand, infile)) {
    MDV_fhand_delete(fhand);
    return (NULL);
  }

  /*
   * convert the data as appropriate
   */
  
  if (MDV_fhand_convert(fhand, 
			output_encoding, output_compression,
			output_scaling, output_scale, output_bias)) {
    fprintf(stderr, "ERROR - MDV_read_field_volume\n");
    fprintf(stderr, "  Cannot convert field '%s'\n", f_hdr->field_name);
    MDV_fhand_delete(fhand);
    return (NULL);
  }

  /*
   * allocate buffer for data, copy data in
   */

  out_len = MDV_fhand_get_vol_len(fhand);
  out_buf = umalloc(out_len);
  memcpy(out_buf, MDV_fhand_get_vol_ptr(fhand), out_len);
  *f_hdr = *(MDV_fhand_get_hdr(fhand));
  
  /*
   * free up
   */

  MDV_fhand_delete(fhand);

  if (output_volume_len != NULL) {
    *output_volume_len = out_len;
  }

  return(out_buf);

}

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

void * MDV_read_field_plane(FILE *infile,
			    MDV_field_header_t *f_hdr,
			    int output_encoding,
			    int output_compression,
			    int output_scaling,
			    double output_scale,
			    double output_bias,
			    int plane_num,
			    int *output_plane_len)
     
{

  MDV_field_handle_t *fhand;
  void *out_buf;
  int out_len;

  /*
   * create a field_handle, read in the field
   */

  fhand = MDV_fhand_create_from_parts(f_hdr, NULL);

  /*
   * read in field data plane
   */

  if (MDV_fhand_read_plane(fhand, infile, plane_num)) {
    MDV_fhand_delete(fhand);
    return (NULL);
  }

  /*
   * convert the data as appropriate
   */
  
  if (MDV_fhand_convert(fhand, 
			output_encoding, output_compression,
			output_scaling, output_scale, output_bias)) {
    fprintf(stderr, "ERROR - MDV_read_field_plane\n");
    fprintf(stderr, "  Cannot convert field '%s'\n", f_hdr->field_name);
    MDV_fhand_delete(fhand);
    return (NULL);
  }

  /*
   * allocate buffer for data, copy data in
   */

  out_len = MDV_fhand_get_vol_len(fhand);
  out_buf = umalloc(out_len);
  memcpy(out_buf, MDV_fhand_get_vol_ptr(fhand), out_len);
  *f_hdr = *(MDV_fhand_get_hdr(fhand));

  /*
   * free up
   */

  MDV_fhand_delete(fhand);

  *output_plane_len = out_len;
  return(out_buf);

}

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

void * MDV_read_field_composite(FILE *infile,
				MDV_field_header_t *f_hdr,
				int output_encoding,
				int output_compression,
				int output_scaling,
				double output_scale,
				double output_bias,
				int lower_plane_num,
				int upper_plane_num,
				int *output_plane_len)
     
{

  MDV_field_handle_t *fhand;
  void *out_buf;
  int out_len;

  /*
   * create a field_handle, read in the field
   */

  fhand = MDV_fhand_create_from_parts(f_hdr, NULL);

  /*
   * read in field data
   */

  if (MDV_fhand_read_vol(fhand, infile)) {
    MDV_fhand_delete(fhand);
    return (NULL);
  }

  /*
   * create composite
   */

  if (MDV_fhand_composite(fhand, lower_plane_num, upper_plane_num)) {
    fprintf(stderr, "ERROR - MDV_read_field_composite\n");
    fprintf(stderr, "  Cannot composite field '%s'\n", f_hdr->field_name);
    fprintf(stderr, "  Lower plane %d, upper plane %d\n",
	    lower_plane_num, upper_plane_num);
    MDV_fhand_delete(fhand);
    return (NULL);
  }

  /*
   * convert the data as appropriate
   */
  
  if (MDV_fhand_convert(fhand, 
			output_encoding, output_compression,
			output_scaling, output_scale, output_bias)) {
    fprintf(stderr, "ERROR - MDV_read_field_composite\n");
    fprintf(stderr, "  Cannot convert field '%s'\n", f_hdr->field_name);
    MDV_fhand_delete(fhand);
    return (NULL);
  }

  /*
   * allocate buffer for data, copy data in
   */

  out_len = MDV_fhand_get_vol_len(fhand);
  out_buf = umalloc(out_len);
  memcpy(out_buf, MDV_fhand_get_vol_ptr(fhand), out_len);
  *f_hdr = *(MDV_fhand_get_hdr(fhand));

  /*
   * free up
   */

  MDV_fhand_delete(fhand);

  if (output_plane_len != NULL) {
    *output_plane_len = out_len;
  }

  return(out_buf);

}

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

void * MDV_get_volume(FILE *infile,
		      const MDV_field_header_t *f_hdr,
		      int return_type)
{

  int volume_size;
  return(MDV_get_volume_size(infile, f_hdr, return_type, &volume_size));
  
}

/******************************************************************************
 * MDV_GET_VOLUME_SIZE: Just like MDV_get_volume, but also returns the size in
 *                      bytes of the returned volume.
 */

void * MDV_get_volume_size(FILE *infile,
			   const MDV_field_header_t *f_hdr,
			   int return_type,
			   int *volume_size)
{

  int output_encoding;
  int output_compression;
  MDV_field_header_t loc_fhdr = *f_hdr;

  output_encoding = return_type;
  output_compression = MDV_COMPRESSION_NONE;

  if (return_type == MDV_PLANE_RLE8) {
    output_encoding = MDV_INT8;
    output_compression = MDV_COMPRESSION_RLE;
  }
  
  return (MDV_read_field_volume(infile, &loc_fhdr,
				output_encoding, output_compression,
				MDV_SCALING_ROUNDED, 0.0, 0.0, 
				volume_size));

}

/******************************************************************************
 * MDV_GET_PLANE: Allocate space for a data plane and read the desired plane
 * into the buffer from the Open file. Caller is responsible for freeing
 * up the buffer when done. Caller is responsible for making sure the
 * field_vlevel_header is correct and proper for the open file.
 *
 * Inputs: infile - pointer to the input file.  This is assumed to currently
 *                  be open for read.
 *         f_hdr - field header for the plane to be loaded.  This header
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

void * MDV_get_plane(FILE *infile,
		     const MDV_field_header_t *f_hdr, 
                     int return_type, int plane_num)
{
  int plane_size;
  return(MDV_get_plane_size(infile, f_hdr,
			    return_type, plane_num, &plane_size));
}


/******************************************************************************
 * MDV_GET_PLANE_SIZE: Just like MDV_get_plane, but also returns the size in
 *                     bytes of the returned plane.
 */

void * MDV_get_plane_size(FILE *infile,
			  const MDV_field_header_t *f_hdr, 
			  int return_type,
			  int plane_num,
			  int *plane_size)
{

  int output_encoding;
  int output_compression;
  MDV_field_header_t loc_fhdr = *f_hdr;

  output_encoding = return_type;
  output_compression = MDV_COMPRESSION_NONE;

  if (return_type == MDV_PLANE_RLE8) {
    output_encoding = MDV_INT8;
    output_compression = MDV_COMPRESSION_RLE;
  }
  
  return (MDV_read_field_plane(infile, &loc_fhdr,
			       output_encoding, output_compression,
			       MDV_SCALING_ROUNDED, 0.0, 0.0, 
			       plane_num, plane_size));
}


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

void * MDV_get_composite(FILE *infile,
			 const MDV_field_header_t *f_hdr, 
			 int composite_alg,
			 int return_type)
{

  int output_encoding;
  int output_compression;
  int volume_size;
  MDV_field_header_t loc_fhdr = *f_hdr;

  output_encoding = return_type;
  output_compression = MDV_COMPRESSION_NONE;

  if (return_type == MDV_PLANE_RLE8) {
    output_encoding = MDV_INT8;
    output_compression = MDV_COMPRESSION_RLE;
  }
  
  return (MDV_read_field_composite(infile,
				   &loc_fhdr,
				   output_encoding,
				   output_compression,
				   MDV_SCALING_ROUNDED, 0.0, 0.0, 
				   -1, -1, &volume_size));

}

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
 
void *MDV_get_chunk_data(FILE *infile, MDV_chunk_header_t *c_hdr)

{

  static char *routine_name = "MDV_get_chunk_data";
  
  void *chunk_data;
  void *return_chunk_data;
  int bytes_read;
  
  /*
   * Seek to the proper position in the input file.  Remember to get
   * the FORTRAN record lengths.
   */

  if (fseek(infile, c_hdr->chunk_data_offset - sizeof(si32),
	    SEEK_SET) != 0) {
    fprintf(stderr,
	    "%s: Error seeking to chunk data position (offset %d) "
	    "in input file.\n",
	    routine_name, c_hdr->chunk_data_offset);
    return(NULL);
  }
  
  /*
   * Allocate space for the returned chunk data.  Be sure to include
   * the surrounding FORTRAN record lengths.
   */
  
  if ((chunk_data = (void *)umalloc(c_hdr->size +
				    2 * sizeof(si32))) == NULL) {
    fprintf(stderr,
	    "%s: Error allocating %d bytes for chunk data\n",
	    routine_name, c_hdr->size);
    return(NULL);
  }
  
  /*
   * Read the chunk data from the file.
   */

  if ((bytes_read = ufread(chunk_data, 1,
			   c_hdr->size + 2 * sizeof(si32), infile))
      != c_hdr->size + 2 * sizeof(si32)) {
    fprintf(stderr,
	    "%s: Error reading chunk data "
	    "(expected bytes = %ld, read bytes = %ld)\n",
	    routine_name,
	    (long int) (c_hdr->size + 2 * sizeof(si32)),
	    (long int) bytes_read);
    ufree(chunk_data);
    return(NULL);
  }
  
  /*
   * Swap the chunk data.
   */
  
  if (MDV_chunk_data_from_BE(chunk_data, c_hdr->size,
			     c_hdr->chunk_id) != MDV_SUCCESS) {
    fprintf(stderr,
	    "%s: Error swapping chunk data\n",
	    routine_name);
  }
  
  /*
   * Now take off the record lengths.  The record lengths were needed
   * for the swap routine.
   */

  if ((return_chunk_data = (void *)umalloc(c_hdr->size)) == NULL) {
    fprintf(stderr,
	    "%s: Error allocating %d bytes for returned chunk data\n",
	    routine_name, c_hdr->size);
    ufree(chunk_data);
    return(NULL);
  }
  
  memcpy((char *)return_chunk_data,
	 (char *)chunk_data + sizeof(si32),
	 c_hdr->size);
  
  ufree(chunk_data);
  
  return(return_chunk_data);

}


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

int MDV_handle_read_all(MDV_handle_t *mdv, char *file_path,
			int output_encoding,
			int output_compression,
			int output_scaling,
			double output_scale,
			double output_bias)

{

  FILE *infile;
  int ichunk, ifield, ilevel;
  MDV_field_header_t *fld;
  MDV_field_header_t loc_fld;

  /*
   * check that file is an MDV file
   */

  if (!MDV_verify(file_path)) {
    fprintf(stderr, "ERROR - MDV_read_all\n");
    fprintf(stderr, "File %s is not MDV format\n", file_path);
    return (-1);
  }
  
  /*
   * Open the input file.
   */

  if ((infile = ta_fopen_uncompress(file_path, "r")) == NULL) {
    fprintf(stderr,
	    "ERROR - MDV_read_all:  Cannot open input file\n");
    perror(file_path);
    return(-1);
  }
  
  /*
   * Read the master header.
   */
  
  if (MDV_load_master_header(infile, &mdv->master_hdr) != MDV_SUCCESS) {
    fprintf(stderr,
	    "ERROR: MDV_read_all - reading master header from "
	    "input file <%s>\n", file_path);
    fclose(infile);
    return(-1);
  }

  /*
   * alloc arrays
   */

  MDV_alloc_handle_arrays(mdv,
			  mdv->master_hdr.n_fields,
			  mdv->master_hdr.max_nz,
			  mdv->master_hdr.n_chunks);
  
  /*
   * Read field headers.
   */

  for (ifield = 0; ifield < mdv->master_hdr.n_fields; ifield++) {

    MDV_field_header_t *fhdr = mdv->fld_hdrs + ifield;
    
    if (MDV_load_field_header(infile, fhdr,
			      ifield) != MDV_SUCCESS) {
      fprintf(stderr,
	      "ERROR: MDV_read_all - reading field %d header from "
	      "input file <%s>\n", ifield, file_path);
      fclose(infile);
      return(-1);
    }

    /*
     * Some older files do not have the vlevel types in the
     * field header. In those cases, make the field header
     * consistent with the master header.
     */
    
    if (fhdr->native_vlevel_type == 0 &&
	mdv->master_hdr.native_vlevel_type != 0) {
      fhdr->native_vlevel_type = mdv->master_hdr.native_vlevel_type;
    }
    
    if (fhdr->vlevel_type == 0 &&
	mdv->master_hdr.vlevel_type != 0) {
      fhdr->vlevel_type = mdv->master_hdr.vlevel_type;
    }
      
  }

  /*
   * Read each vlevel header.
   */
  
  if (mdv->master_hdr.vlevel_included) {
    
    for (ifield = 0; ifield < mdv->master_hdr.n_fields; ifield++) {
      if (MDV_load_vlevel_header(infile, mdv->vlv_hdrs + ifield,
				 &mdv->master_hdr, ifield) != MDV_SUCCESS) {
	fprintf(stderr,
		"ERROR: MDV_read_all - reading vlevel %d header from "
		"input file <%s>\n", ifield, file_path);
	fclose(infile);
	return(-1);
      }
	
    } /* ifield */
      
  } /* endif - vlevel_included */

  /*
   * load up the grid
   */

  if (mdv->master_hdr.n_fields > 0) {
    MDV_load_grid_from_hdrs(&mdv->master_hdr, mdv->fld_hdrs,
			    &mdv->grid);
  }
  
  /*
   * Read each chunk header and the associated data.
   */

  for (ichunk = 0; ichunk < mdv->master_hdr.n_chunks; ichunk++) {

    if (MDV_load_chunk_header(infile, mdv->chunk_hdrs + ichunk,
			      &mdv->master_hdr,
			      ichunk) != MDV_SUCCESS) {
      fprintf(stderr,
	      "ERROR: MDV_read_all - loading chunk %d header from "
	      "input file <%s>\n", ichunk, file_path);
      fclose(infile);
      return(-1);
    }

    if (mdv->chunk_data_allocated && mdv->chunk_data[ichunk]) {
      ufree(mdv->chunk_data[ichunk]);
      mdv->chunk_data[ichunk] = NULL;
    }
    if ((mdv->chunk_data[ichunk] =
	 MDV_get_chunk_data(infile, mdv->chunk_hdrs + ichunk)) == NULL)	{
      fprintf(stderr,
	      "ERROR: MDV_read_all - reading chunk %d data from "
	      "input file <%s>\n", ichunk, file_path);
      fclose(infile);
      return(-1);
    }
	
  } /* ichunk */

  if (mdv->master_hdr.n_chunks > 0) {
    mdv->chunk_data_allocated = TRUE;
  }
  
  /*
   * load radar data structs if applicable
   */

  MDV_handle_load_radar_structs(mdv);

  /*
   * read in planes
   */

  for (ifield = 0; ifield < mdv->master_hdr.n_fields; ifield++) {

    fld = mdv->fld_hdrs + ifield;

    fld->volume_size = 0;
    
    for (ilevel = 0; ilevel < fld->nz; ilevel++) {

      int plane_size;
      void *plane;
    
      loc_fld = *fld;
      plane = MDV_read_field_plane(infile, &loc_fld,
				   output_encoding, output_compression,
				   output_scaling, output_scale, output_bias,
				   ilevel, &plane_size);
    
      if (plane == NULL) {
	fprintf(stderr,
		"ERROR: MDV_read_all - reading field %d, level %d from "
		"input file <%s>\n", ifield, ilevel, file_path);
	fclose(infile);
	return(-1);
      }
      
      if (mdv->field_plane[ifield][ilevel] && mdv->field_planes_allocated) {
	ufree(mdv->field_plane[ifield][ilevel]);
      }

      mdv->field_plane[ifield][ilevel] = plane;
      mdv->field_plane_len[ifield][ilevel] = plane_size;
      fld->volume_size += plane_size;

    } /* ilevel */
	
    if (output_encoding != MDV_ASIS)
      fld->encoding_type = output_encoding;

    if (output_compression != MDV_COMPRESSION_ASIS) {
      fld->compression_type = output_compression;
    }

  } /* ifield */

  if (mdv->master_hdr.n_fields > 0) {
    mdv->field_planes_allocated = TRUE;
  }

  fclose(infile);

  mdv->read_all_done = TRUE;
  return (0);

}

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

int MDV_handle_read_field(MDV_handle_t *mdv, char *file_path,
			  int field_num,
			  int output_encoding,
			  int output_compression,
			  int output_scaling,
			  double output_scale,
			  double output_bias)
{
  const char *routine_name = "MDV_handle_read_field";
  
  FILE *infile = NULL;
  int ichunk = 0, ifield = 0, ilevel = 0;
  MDV_field_header_t *fld = NULL;
  MDV_field_header_t loc_fld;

  /*
   * check that file is an MDV file
   */

  if (!MDV_verify(file_path)) {
    fprintf(stderr, "ERROR - %s\n", routine_name);
    fprintf(stderr, "File %s is not MDV format\n", file_path);
    return (-1);
  }
  
  /*
   * Open the input file.
   */

  if ((infile = ta_fopen_uncompress(file_path, "r")) == NULL) {
    fprintf(stderr,
	    "ERROR - %s:  Cannot open input file\n", routine_name);
    perror(file_path);
    return(-1);
  }
  
  /*
   * Read the master header.
   */
  
  if (MDV_load_master_header(infile, &mdv->master_hdr) != MDV_SUCCESS) {
    fprintf(stderr,
	    "ERROR: %s - reading master header from input file <%s>\n",
	    routine_name, file_path);
    fclose(infile);
    return(-1);
  }

  /*
   * Verify the indicated field number.
   */

  if (field_num >= mdv->master_hdr.n_fields)
  {
    fprintf(stderr,
	    "ERROR: %s - input file <%s> does not contain field number %d\n",
	    routine_name, file_path, field_num);
    fclose(infile);
    return(-1);
  }
  
  /*
   * Update the internal master header to show that we will only have
   * one field.
   */

  mdv->master_hdr.n_fields = 1;
  
  /*
   * alloc arrays
   */

  MDV_alloc_handle_arrays(mdv,
			  mdv->master_hdr.n_fields,
			  mdv->master_hdr.max_nz,
			  mdv->master_hdr.n_chunks);
  
  /*
   * Read field header.
   */

  if (MDV_load_field_header(infile, mdv->fld_hdrs,
			    field_num) != MDV_SUCCESS) {
    fprintf(stderr,
	    "ERROR: %s - reading field %d header from input file <%s>\n",
	    routine_name, ifield, file_path);
    fclose(infile);
    return(-1);
  }
      
  /*
   * Read the vlevel header.
   */
  
  if (mdv->master_hdr.vlevel_included) {
    
    if (MDV_load_vlevel_header(infile, mdv->vlv_hdrs,
			       &mdv->master_hdr, field_num) != MDV_SUCCESS) {
      fprintf(stderr,
	      "ERROR: %s - reading vlevel %d header from input file <%s>\n",
	      routine_name, ifield, file_path);
      fclose(infile);
      return(-1);
    }
	
  } /* endif - vlevel_included */

  /*
   * load up the grid
   */

  MDV_load_grid_from_hdrs(&mdv->master_hdr, mdv->fld_hdrs,
			  &mdv->grid);
  
  /*
   * Read each chunk header and the associated data.
   */

  for (ichunk = 0; ichunk < mdv->master_hdr.n_chunks; ichunk++) {

    if (MDV_load_chunk_header(infile, mdv->chunk_hdrs + ichunk,
			      &mdv->master_hdr,
			      ichunk) != MDV_SUCCESS) {
      fprintf(stderr,
	      "ERROR: %s - loading chunk %d header from input file <%s>\n",
	      routine_name, ichunk, file_path);
      fclose(infile);
      return(-1);
    }

    if (mdv->chunk_data_allocated && mdv->chunk_data[ichunk]) {
      ufree(mdv->chunk_data[ichunk]);
      mdv->chunk_data[ichunk] = NULL;
    }
    if ((mdv->chunk_data[ichunk] =
	 MDV_get_chunk_data(infile, mdv->chunk_hdrs + ichunk)) == NULL)	{
      fprintf(stderr,
	      "ERROR: %s - reading chunk %d data from input file <%s>\n",
	      routine_name, ichunk, file_path);
      fclose(infile);
      return(-1);
    }
	
  } /* ichunk */

  if (mdv->master_hdr.n_chunks > 0) {
    mdv->chunk_data_allocated = TRUE;
  }
  
  /*
   * load radar data structs if applicable
   */

  MDV_handle_load_radar_structs(mdv);

  /*
   * read in planes
   */

  fld = mdv->fld_hdrs;

  fld->volume_size = 0;
    
  for (ilevel = 0; ilevel < fld->nz; ilevel++) {

    int plane_size;
    void *plane;

    loc_fld = *fld;
    plane = MDV_read_field_plane(infile, &loc_fld,
				 output_encoding, output_compression,
				 output_scaling, output_scale, output_bias,
				 ilevel, &plane_size);
    
    if (plane == NULL) {
      fprintf(stderr,
	      "ERROR: %s - reading field %d, level %d from input file <%s>\n",
	      routine_name, field_num, ilevel, file_path);
      fclose(infile);
      return(-1);
    }
      
    if (mdv->field_plane[0][ilevel] && mdv->field_planes_allocated) {
      ufree(mdv->field_plane[0][ilevel]);
    }

    mdv->field_plane[0][ilevel] = plane;
    mdv->field_plane_len[0][ilevel] = plane_size;
    fld->volume_size += plane_size;

  } /* ilevel */
	
  /*
   * Update the field header information to keep things internally
   * consistent.
   */

  if (mdv->master_hdr.vlevel_included)
    fld->field_data_offset =
      sizeof(MDV_master_header_t) + sizeof(MDV_field_header_t) +
      sizeof(MDV_vlevel_header_t) +
      (mdv->master_hdr.n_chunks * sizeof(MDV_chunk_header_t));
  else
    fld->field_data_offset =
      sizeof(MDV_master_header_t) + sizeof(MDV_field_header_t) +
      (mdv->master_hdr.n_chunks * sizeof(MDV_chunk_header_t));

  if (output_encoding != MDV_ASIS)
    fld->encoding_type = output_encoding;

  if (output_compression != MDV_COMPRESSION_ASIS)
    fld->compression_type = output_compression;

  mdv->field_planes_allocated = TRUE;

  fclose(infile);

  mdv->read_all_done = TRUE;
  return (0);

}

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

int MDV_read_all(MDV_handle_t *mdv, char *file_path, int return_type)

{

  int output_encoding;
  int output_compression;
  
  output_encoding = return_type;
  output_compression = MDV_COMPRESSION_NONE;

  if (return_type == MDV_PLANE_RLE8) {
    output_encoding = MDV_INT8;
    output_compression = MDV_COMPRESSION_RLE;
  }
  
  return (MDV_handle_read_all(mdv, file_path,
			      output_encoding, output_compression,
			      MDV_SCALING_ROUNDED, 0.0, 0.0));

}
