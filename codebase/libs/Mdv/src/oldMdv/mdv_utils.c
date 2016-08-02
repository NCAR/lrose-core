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
/*****************************************************************
 * MDV_UTILS.C: contains general purpose routines for working
 * with mdv files. 
 * R. Ames March 1996. NCAR, RAP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/param.h>

#include <dataport/bigend.h>
#include <toolsa/mem.h>
#include <toolsa/compress.h>
#include <rapformats/ds_radar.h>
#include <rapformats/var_elev.h>
 
#include <Mdv/mdv/mdv_field_codes.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_utils.h>

#include "mdv_private.h"


/************
 * Static functions.
 */

static void *load_plane_int8(void *buffer,
			     MDV_field_header_t *field_hdr,
			     int data_type,
			     int plane_num,
			     int *return_size);

static void *load_plane_plane_rle8(void *buffer,
				   MDV_field_header_t *field_hdr,
				   int data_type,
				   int plane_num,
				   int *return_size);


/*****************************************************************
 * MDV_RECALLOC: Allocs or reallocs depending on which one is 
 * necessary.   
 * Returns pointer with appropriate space. 
 * --Rachel Ames 3/96, RAP/NCAR
 */

void * MDV_recalloc(void * ptr_to_mem, int number_of_mem, int size_of_mem)
{ 
  if (ptr_to_mem == NULL) {
    ptr_to_mem = ucalloc(number_of_mem, size_of_mem);
  }
  else {
    ptr_to_mem = urealloc(ptr_to_mem, number_of_mem*size_of_mem);
    memset(ptr_to_mem, 0, number_of_mem*size_of_mem);
  }
  return(ptr_to_mem);
}

/*****************************************************************
 * MDV_DATA_ELEMENT_SIZE: Give the size (in bytes) of the data 
 * element given the encoding type integer.
 *
 * --Rachel Ames 3/96
 */

int MDV_data_element_size (int encoding_type)

{

  switch(encoding_type) {
    
  case MDV_INT8 :
    return(INT8_SIZE);
    
  case MDV_INT16 :
    return(INT16_SIZE);
    
  case MDV_FLOAT32 :
    return(FLOAT32_SIZE);
    
  default:
    return(INT8_SIZE);
    
  } /* endswitch - encoding_type */
  
}


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
				int *plane_size_p) 
{
  static char *routine_name = "MDV_get_plane_from_volume()";
  
  int plane_offset;
  int plane_size;
  
  void *return_plane = NULL;
  
  switch (field_hdr->encoding_type)
  {
  case MDV_NATIVE :
    fprintf(stderr, "ERROR: %s\n", routine_name);
    fprintf(stderr,
	    "Cannot extract %s planes\n", MDV_encode2string(MDV_NATIVE));
    return((void *)NULL);
    
  case MDV_INT8 :
  case MDV_INT16 :
  case MDV_FLOAT32 :
    plane_size =
      field_hdr->nx * field_hdr->ny * field_hdr->data_element_nbytes;
    plane_offset = plane_num * plane_size;
    break;
    
  case MDV_PLANE_RLE8 :
  {
    si32 *plane_locs = (si32 *)volume_ptr;
    si32 *plane_sizes = (si32 *)volume_ptr + field_hdr->nz;
    
    plane_offset = plane_locs[plane_num];
    plane_size = plane_sizes[plane_num];

    break;
  }
  
  default:
    fprintf(stderr, "ERROR: %s\n", routine_name);
    fprintf(stderr, "Unrecognized encoding type %d\n",
	    field_hdr->encoding_type);
    return((void *)NULL);
    
  } /* endswitch - field_hdr->encoding_type */
  
  /*
   * Allocate space for the returned plane and copy the data.
   */

  return_plane = umalloc(plane_size);
  memcpy(return_plane, (char *)volume_ptr + plane_offset, plane_size);
  
  *plane_size_p = plane_size;
  return(return_plane);

}


/*****************************************************************
 * MDV_MASTER_HEADER_FROM_BE: Converts master header from big endian
 * format to native format.  Nancy Rehak 6/97
 */

void MDV_master_header_from_BE(MDV_master_header_t *m_hdr) 
{
  /* swap header si32 and fl32's */
  BE_to_array_32((ui32 *)(&m_hdr->record_len1),
		 MDV_NUM_MASTER_HEADER_32 * sizeof(si32));

  /* swap the last record length */
  m_hdr->record_len2 = BE_to_si32(m_hdr->record_len2);

  return;
}

 
/*****************************************************************
 * MDV_MASTER_HEADER_TO_BE: Converts master header from native
 * format to big endian format.  Nancy Rehak 6/97
 */

void MDV_master_header_to_BE(MDV_master_header_t *m_hdr) 
{
  /* swap header si32 and fl32's */
  BE_from_array_32((ui32 *)(&m_hdr->record_len1),
		   MDV_NUM_MASTER_HEADER_32 * sizeof(si32));

  /* swap the last record length */
  m_hdr->record_len2 = BE_from_si32(m_hdr->record_len2);

  return;
}

 
/*****************************************************************
 * MDV_FIELD_HEADER_FROM_BE: Converts field header from big endian
 * format to native format.  Nancy Rehak 6/97
 */

void MDV_field_header_from_BE(MDV_field_header_t *f_hdr)
{
  /* swap header si32 and fl32's */
  BE_to_array_32((ui32 *)(&f_hdr->record_len1),
		 MDV_NUM_FIELD_HEADER_32 * sizeof(si32));
 
  /* swap the last record length */
  f_hdr->record_len2 = BE_to_si32(f_hdr->record_len2);

  /*
   * make consistent with new encoding and compression types
   */

  if (f_hdr->encoding_type == MDV_PLANE_RLE8) {
    f_hdr->encoding_type = MDV_INT8;
    f_hdr->compression_type = MDV_COMPRESSION_RLE;
  }

  if (f_hdr->compression_type < MDV_COMPRESSION_NONE ||
      f_hdr->compression_type > MDV_COMPRESSION_GZIP) {
    f_hdr->compression_type = MDV_COMPRESSION_NONE;
  }
 
  return;
}

/*****************************************************************
 * MDV_FIELD_HEADER_TO_BE: Converts field header from native
 * format to big endian format.  Nancy Rehak 6/97
 */

void MDV_field_header_to_BE(MDV_field_header_t *f_hdr)
{

  /*
   * make consistent with new encoding and compression types
   */

  if (f_hdr->encoding_type == MDV_PLANE_RLE8) {
    f_hdr->encoding_type = MDV_INT8;
    f_hdr->compression_type = MDV_COMPRESSION_RLE;
  }
 
  /* swap header si32 and fl32's */
  BE_from_array_32((ui32 *)(&f_hdr->record_len1),
		   MDV_NUM_FIELD_HEADER_32 * sizeof(si32));
 
  /* swap the last record length */
  f_hdr->record_len2 = BE_from_si32(f_hdr->record_len2);
 
  return;
}


/*****************************************************************
 * MDV_VLEVEL_HEADER_FROM_BE: Swaps vlevel header from big endian
 * format to native format.  Nancy Rehak 6/97
 */
 
void MDV_vlevel_header_from_BE(MDV_vlevel_header_t *v_hdr)
{
  /* swap header si32 and fl32's. No chars so everything done at once */
  BE_to_array_32((ui32 *)(&v_hdr->record_len1),
		 MDV_NUM_VLEVEL_HEADER_32 * sizeof(si32));
 
  /* swap the last record length */
  v_hdr->record_len2 = BE_to_si32(v_hdr->record_len2);
 
  return;
}


/*****************************************************************
 * MDV_VLEVEL_HEADER_TO_BE: Swaps vlevel header from native
 * format to big endian format.  Nancy Rehak 6/97
 */
 
void MDV_vlevel_header_to_BE(MDV_vlevel_header_t *v_hdr)
{
  /* swap header si32 and fl32's. No chars so everything done at once */
  BE_from_array_32((ui32 *)(&v_hdr->record_len1),
		   MDV_NUM_VLEVEL_HEADER_32 * sizeof(si32));
 
  /* swap the last record length */
  v_hdr->record_len2 = BE_from_si32(v_hdr->record_len2);
 
  return;
}


/*****************************************************************
 * MDV_FIELD_VLEVEL_HEADER_FROM_BE: Converts a field_vlevel header
 * from big endian format to native format.  Nancy Rehak 6/97
 */
 
void MDV_field_vlevel_header_from_BE(MDV_field_vlevel_header_t *fv_head)
{
  MDV_field_header_from_BE(fv_head->fld_hdr);

  if (fv_head->vlv_hdr != NULL)
    MDV_vlevel_header_from_BE(fv_head->vlv_hdr);

  return;
}


/*****************************************************************
 * MDV_FIELD_VLEVEL_HEADER_TO_BE: Converts a field_vlevel header
 * from native format to big endian format.  Nancy Rehak 6/97
 */
 
void MDV_field_vlevel_header_to_BE(MDV_field_vlevel_header_t *fv_head)
{
  MDV_field_header_to_BE(fv_head->fld_hdr);

  if (fv_head->vlv_hdr != NULL)
    MDV_vlevel_header_to_BE(fv_head->vlv_hdr);

  return;
}


/*****************************************************************
 * MDV_CHUNK_HEADER_FROM_BE: Converts a chunk header from big endian
 * format to native format.  Nancy Rehak 6/97
 */
 
void MDV_chunk_header_from_BE(MDV_chunk_header_t *c_hdr)
{
  /* swap header si32 and fl32's */
  BE_to_array_32((ui32 *)(&c_hdr->record_len1),
		 MDV_NUM_CHUNK_HEADER_32 * sizeof(si32));
 
  /* swap the last record length */
  c_hdr->record_len2 = BE_to_si32(c_hdr->record_len2);

  return;
}

/*****************************************************************
 * MDV_CHUNK_HEADER_TO_BE: Converts a chunk header from native
 * format to big endian format.  Nancy Rehak 6/97
 */
 
void MDV_chunk_header_to_BE(MDV_chunk_header_t *c_hdr)
{
  /* swap header si32 and fl32's */
  BE_from_array_32((ui32 *)(&c_hdr->record_len1),
		   MDV_NUM_CHUNK_HEADER_32 * sizeof(si32));
 
  /* swap the last record length */
  c_hdr->record_len2 = BE_from_si32(c_hdr->record_len2);

  return;
}

/*****************************************************************
 * MDV_UNENCODED_VOLUME_FROM_BE: Converts the data in an unencoded
 * data volume from big endian format to native format.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_unencoded_volume_from_BE(void *volume_data,
				 ui32 volume_size,
				 int data_type)
{
  char *routine_name = "MDV_unencoded_volume_from_BE";
  
  switch(data_type)
  {
  case MDV_INT8 :
    /* No data swapping necessary */
    break;
    
  case MDV_INT16 :
    BE_to_array_16((ui16 *)volume_data, volume_size);
    break;
    
  case MDV_FLOAT32 :
    BE_to_array_32((ui32 *)volume_data, volume_size);
    break;
    
  default:
    fprintf(stderr,
	    "%s: Do not know how to byte swap data in %s format\n",
	    routine_name, MDV_encode2string(data_type));
    return(MDV_FAILURE);
    
  } /* endswitch - data_type */

  return(MDV_SUCCESS);
}


/*****************************************************************
 * MDV_UNENCODED_VOLUME_TO_BE: Converts the data in an unencoded
 * data volume from native format to big endian format.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_unencoded_volume_to_BE(void *volume_data,
			       ui32 volume_size,
			       int data_type)
{
  char *routine_name = "MDV_unencoded_volume_to_BE";
  
  switch(data_type)
  {
  case MDV_INT8 :
    /* No data swapping necessary */
    break;
    
  case MDV_INT16 :
    BE_from_array_16((ui16 *)volume_data, volume_size);
    break;
    
  case MDV_FLOAT32 :
    BE_from_array_32((ui32 *)volume_data, volume_size);
    break;
    
  default:
    fprintf(stderr,
	    "%s: Do not know how to byte swap data in %s format\n",
	    routine_name, MDV_encode2string(data_type));
    return(MDV_FAILURE);
    
  } /* endswitch - data_type */

  return(MDV_SUCCESS);
}


/*****************************************************************
 * MDV_PLANE_TO_BE: Converts the data in a plane of data from big
 * endian format to native format.  N. Rehak 8/98
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_plane_from_BE(MDV_field_header_t *field_hdr, void *plane_ptr)
{
  static char *routine_name = "MDV_plane_from_BE";
  
  switch (field_hdr->encoding_type)
  {
  case MDV_INT8 :
    /*
     * No swapping necessary.
     */

    return(MDV_SUCCESS);
    
  case MDV_INT16 :
    BE_to_array_16(plane_ptr,
		   field_hdr->nx * field_hdr->ny * sizeof(si16));
    return(MDV_SUCCESS);
    
  case MDV_FLOAT32 :
    BE_to_array_32(plane_ptr,
		   field_hdr->nx * field_hdr->ny * sizeof(si32));
    return(MDV_SUCCESS);
    
  case MDV_PLANE_RLE8 :
    return(MDV_plane_rle8_from_BE(plane_ptr));
  
  } /* endswitch */
  
  fprintf(stderr, "ERROR: mdv:%s\n", routine_name);
  fprintf(stderr, "Invalid encoding type %d found in field header.\n",
	  field_hdr->encoding_type);
  
  return(MDV_FAILURE);
}


/*****************************************************************
 * MDV_PLANE_TO_BE: Converts the data in a plane of data from native
 * format to big endian format.  N. Rehak 8/98
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_plane_to_BE(MDV_field_header_t *field_hdr, void *plane_ptr)
{
  static char *routine_name = "MDV_plane_to_BE";
  
  switch (field_hdr->encoding_type)
  {
  case MDV_INT8 :
    /*
     * No swapping necessary.
     */

    return(MDV_SUCCESS);
    
  case MDV_INT16 :
    BE_from_array_16(plane_ptr,
		     field_hdr->nx * field_hdr->ny * sizeof(si16));
    return(MDV_SUCCESS);
    
  case MDV_FLOAT32 :
    BE_from_array_32(plane_ptr,
		     field_hdr->nx * field_hdr->ny * sizeof(si32));
    return(MDV_SUCCESS);
    
  case MDV_PLANE_RLE8 :
    return(MDV_plane_rle8_to_BE(plane_ptr));
  
  } /* endswitch */
  
  fprintf(stderr, "ERROR: mdv:%s\n", routine_name);
  fprintf(stderr, "Invalid encoding type %d found in field header.\n",
	  field_hdr->encoding_type);
  
  return(MDV_FAILURE);
}


/*****************************************************************
 * MDV_PLANE_RLE8_FROM_BE: Converts the data in a plane of data
 * encoded in the MDV_PLANE_RLE8 format from big endian format to
 * native format.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_plane_rle8_from_BE(void *plane_data)
{
  si32 *plane_ptr = (si32 *)plane_data;
  
  plane_ptr[0] = BE_to_si32(plane_ptr[0]);  /* RL8_FLAG */
  plane_ptr[1] = BE_to_si32(plane_ptr[1]);  /* key */
  plane_ptr[2] = BE_to_si32(plane_ptr[2]);  /* nbytes_array */
  plane_ptr[3] = BE_to_si32(plane_ptr[3]);  /* nbytes_full */
  plane_ptr[4] = BE_to_si32(plane_ptr[4]);  /* nbytes_coded */

  /* All of the rest of the data is byte data so doesn't need swapping */

  return(MDV_SUCCESS);
}


/*****************************************************************
 * MDV_PLANE_RLE8_TO_BE: Converts the data in a plane of data
 * encoded in the MDV_PLANE_RLE8 format from native format to
 * big endian format.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE
 */
 
int MDV_plane_rle8_to_BE(void *plane_data)
{
  si32 *plane_ptr = (si32 *)plane_data;
  
  plane_ptr[0] = BE_from_si32(plane_ptr[0]);  /* RL8_FLAG */
  plane_ptr[1] = BE_from_si32(plane_ptr[1]);  /* key */
  plane_ptr[2] = BE_from_si32(plane_ptr[2]);  /* nbytes_array */
  plane_ptr[3] = BE_from_si32(plane_ptr[3]);  /* nbytes_full */
  plane_ptr[4] = BE_from_si32(plane_ptr[4]);  /* nbytes_coded */

  /* All of the rest of the data is byte data so doesn't need swapping */

  return(MDV_SUCCESS);
}


/*****************************************************************
 * MDV_CHUNK_DATA_FROM_BE: Converts chunk data from big endian
 * format to native format if the chunk data type is a known type.
 * The data pointer must point to the record length value preceding
 * the chunk data.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE */
 
int MDV_chunk_data_from_BE(void *c_data, int size, int chunk_id)

{
  si32 *rec_len1 = (si32 *)c_data;
  void *data = (void *)((char *)c_data + sizeof(si32));
  si32 *rec_len2 = (si32 *)((char *)c_data + sizeof(si32) + size);
  
  /*
   * Swap the initial record length.
   */

  *rec_len1 = BE_to_si32(*rec_len1);
  
  /*
   * Swap data.
   */

  switch(chunk_id)
  {
  case MDV_CHUNK_DOBSON_VOL_PARAMS :
    dobson_vol_params_from_BE((vol_params_t *)data);
    break;
    
  case MDV_CHUNK_DOBSON_ELEVATIONS :
    dobson_elevations_from_BE(data, size);
    break;
    
  case MDV_CHUNK_DSRADAR_PARAMS :
    BE_to_DsRadarParams((DsRadarParams_t *)data);
    break;
    
  case MDV_CHUNK_DSRADAR_ELEVATIONS :
    BE_to_array_32(data, size);
    break;

  case MDV_CHUNK_VARIABLE_ELEV :
    VAR_ELEV_variable_elev_from_BE(data, size);
    break;

  default:
    /*
     * Return the buffer unswapped.  We don't recognize this
     * chunk type.
     */
    break;
  } /* endswitch - chunk_id */
  
  /*
   * Swap the last record length
   */

  *rec_len2 = BE_to_si32(*rec_len2);

  return(MDV_SUCCESS);
}


/*****************************************************************
 * MDV_CHUNK_DATA_TO_BE: Converts chunk data from native format to
 * big endian format if the chunk data type is a known type.
 * The data pointer must point to the record length value preceding
 * the chunk data.  N. Rehak 6/97
 *
 * returns MDV_SUCCESS or MDV_FAILURE */
 
int MDV_chunk_data_to_BE(void *c_data, int size, int chunk_id)

{
  si32 *rec_len1 = (si32 *)c_data;
  void *data = (void *)((char *)c_data + sizeof(si32));
  si32 *rec_len2 = (si32 *)((char *)c_data + sizeof(si32) + size);
  
  /*
   * Swap the initial record length.
   */

  *rec_len1 = BE_from_si32(*rec_len1);
  
  /*
   * Swap data.
   */

  switch(chunk_id)
  {
  case MDV_CHUNK_DOBSON_VOL_PARAMS :
    dobson_vol_params_to_BE((vol_params_t *)data);
    break;
    
  case MDV_CHUNK_DOBSON_ELEVATIONS :
    dobson_elevations_to_BE(data, size);
    break;
    
  case MDV_CHUNK_DSRADAR_PARAMS :
    BE_from_DsRadarParams((DsRadarParams_t *)data);
    break;
    
  case MDV_CHUNK_DSRADAR_ELEVATIONS :
    BE_from_array_32(data, size);
    break;

  case MDV_CHUNK_VARIABLE_ELEV :
    VAR_ELEV_variable_elev_to_BE(data, size);
    break;
      
  default:
    /*
     * Return the buffer unswapped.  We don't recognize this
     * chunk type.
     */
    break;
  } /* endswitch - chunk_id */
  
  /*
   * Swap the last record length
   */

  *rec_len2 = BE_from_si32(*rec_len2);

  return(MDV_SUCCESS);
}


/*****************************************************************
 * MDV_get_field_name: Returns pointer to field name - NULL on error
 *
 */

char *MDV_get_field_name(int field_code)
{
    if(field_code < 0 || field_code > MDV_MAX_FIELD_CODE) return (char *) NULL;
	/* Make sure static array is not screwed up */
	assert(field_code == mdv_field_code_info[field_code].code);
	return mdv_field_code_info[field_code].name;
}
 
/*****************************************************************
 * MDV_get_field_units: Returns pointer to field units - NULL on error
 *
 */

char *MDV_get_field_units(int field_code)
{
    if(field_code < 0 || field_code > MDV_MAX_FIELD_CODE) return (char *) NULL;
	/* Make sure static array is not screwed up */
	assert(field_code == mdv_field_code_info[field_code].code);
	return mdv_field_code_info[field_code].units;
}
 
/*****************************************************************
 * MDV_get_field_abbrev: Returns pointer to field abbrev - NULL on error
 *
 */

char *MDV_get_field_abbrev(int field_code)
{
    if(field_code < 0 || field_code > MDV_MAX_FIELD_CODE) return (char *) NULL;
	/* Make sure static array is not screwed up */
	assert(field_code == mdv_field_code_info[field_code].code);
	return mdv_field_code_info[field_code].abbrev;
}
 

/*****************************************************************
 * MDV_get_field_code_from_name()
 *
 * Returns field code for field name
 *
 * Returns code on success, -1 on failure (no match).
 */

int MDV_get_field_code_from_name(char *name)
{
  int ncodes;
  int i;
  
  ncodes = sizeof(mdv_field_code_info) / sizeof(mdv_field_code_t);

  for (i = 0; i < ncodes; i++) {
    if (!strcmp(name, mdv_field_code_info[i].name)) {
      return (i);
    }
  }
  return(-1);

}


/*****************************************************************
 * MDV_get_field_code_from_abbrev()
 *
 * Returns field code for field abbrev
 *
 * Returns code on success, -1 on failure (no match).
 */

int MDV_get_field_code_from_abbrev(char *abbrev)
{
  int ncodes;
  int i;
  
  ncodes = sizeof(mdv_field_code_info) / sizeof(mdv_field_code_t);

  for (i = 0; i < ncodes; i++) {
    if (!strcmp(abbrev, mdv_field_code_info[i].abbrev)) {
      return (i);
    }
  }
  return(-1);

}


/******************************************************************************
 * MDV_CALC_PLANE_SIZE: Calculates the number of bytes used to store the
 *                      data in the indicated plane.
 *
 * Returns the plane size on success, -1 on failure.
 */

int MDV_calc_plane_size(MDV_field_header_t *field_hdr,
			int plane_num,
			void *plane_ptr)
{
  static char *routine_name = "MDV_calc_plane_size";
  
  /*
   * Check for errors.
   */

  if (plane_num >= field_hdr->nz)
  {
    fprintf(stderr, "ERROR: mdv:%s\n", routine_name);
    fprintf(stderr, "Invalid plane number %d requested\n", plane_num);
    fprintf(stderr, "Field only has %d planes\n", field_hdr->nz);
    
    return(-1);
  }
  
  /*
   * Calculate the plane size.
   */

  if (!MDV_compressed(field_hdr->compression_type)) {

    /*
     * compute field size from grid dimension
     */
    
    return(field_hdr->nx * field_hdr->ny *
	   MDV_data_element_size(field_hdr->encoding_type));
    
  } else {

    /*
     * for compressed types, the compressed plane is preceded
     * by two ui32s: offset (always 0) and plane size.
     */

    si32 array_size = ((si32 *)plane_ptr)[1];
    return(array_size);

  }
  
}

/******************************************************************************
 * MDV_LOAD_PLANE: Loads the indicated plane from the given buffer.  The
 *                 buffer is expected to point to the beginning of the volume
 *                 data for the field.
 */

void *MDV_load_plane(void *buffer, MDV_field_header_t *fld_hdr, 
		     int return_type, int plane_num, int *plane_size)
{
  static char *routine_name = "MDV_load_plane";
  
  char *buf_ptr = (char *)buffer;
  
  void *plane_ptr;
  
  /*
   * Initialize return values
   */

  *plane_size = 0;
  
  /*
   * Do some sanity checking
   */

  if (fld_hdr == NULL)
  {
    fprintf(stderr,
	    "%s: Invalid pointers in parameter list.\n",
	    routine_name);
    return(NULL);
  }
  
  if (plane_num < 0 || (plane_num > fld_hdr->nz - 1))
  {
    fprintf(stderr,
	    "%s: Invalide plane number %d requested.\n",
	    routine_name, plane_num);
    return NULL;
  }
  
  /*
   * Skip the record size.
   */

  buf_ptr += sizeof(si32);
  
  /*
   * Load the data and convert it based on the encoding type.
   */

  switch(fld_hdr->encoding_type)
  {
  case MDV_INT8 :
    plane_ptr = load_plane_int8(buf_ptr, fld_hdr,
				return_type, plane_num, plane_size);
    break;
    
  case MDV_PLANE_RLE8 :
    plane_ptr = load_plane_plane_rle8(buf_ptr, fld_hdr,
				      return_type, plane_num, plane_size);
    break;
    
  default:
    fprintf(stderr,
	    "%s: Cannot load plane in %s format -- not yet implemented\n",
	    routine_name, MDV_encode2string(fld_hdr->encoding_type));
    return NULL;
    break;
  } /* endswitch - fld_hdr->encoding_type */
  
  return(plane_ptr);
}


/******************************************************************************
 * MDV_CALC_BUFFER_SIZE: Calculates the size of a flat buffer needed to
 *                       store this MDV data.
 */

int MDV_calc_buffer_size(MDV_handle_t *mdv)
{
  int buffer_size = 0;
  
  int n_fields = mdv->master_hdr.n_fields;
  int n_chunks = mdv->master_hdr.n_chunks;
  
  int i;
  
  /*
   * Add the master header.
   */

  buffer_size += sizeof(MDV_master_header_t);
  
  /*
   * Add the field headers.
   */

  buffer_size += n_fields * sizeof(MDV_field_header_t);
  
  /*
   * Add the vlevel headers.
   */

  if (mdv->master_hdr.vlevel_included)
    buffer_size += n_fields * sizeof(MDV_vlevel_header_t);
  
  /*
   * Add the chunk headers.
   */

  buffer_size += n_chunks * sizeof(MDV_chunk_header_t);
  
  /*
   * Add the field data.  Note that the FORTRAN record lengths are
   * not included in the volume size.
   */

  for (i = 0; i < n_fields; i++)
    buffer_size += mdv->fld_hdrs[i].volume_size + (2 * sizeof(si32));
  
  /*
   * Add the chunk data.  Note that the FORTRAN record lengths are
   * not included in the chunk size.
   */

  for (i = 0; i < n_chunks; i++)
    buffer_size += mdv->chunk_hdrs[i].size + (2 * sizeof(si32));
      
  return(buffer_size);
}

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

int MDV_compressed(int compression_type)

{

  if (compression_type < MDV_COMPRESSION_RLE ||
      compression_type > MDV_COMPRESSION_GZIP) {
    return (FALSE);
  } else {
    return (TRUE);
  }

}

/********************************************************
 * STATIC ROUTINES
 ********************************************************/

/********************************************************
 * LOAD_PLANE_INT8
 * Routine for loading a plane of data in MDV_INT8 format
 * from the given buffer and returning it in the specified
 * format. The buffer pointer is assumed to point to the
 * beginning of the volume data, after the FORTRAN record
 * length. All necessary byte swapping is done in this routine.
 *
 * This routine assumes that the plane number is valid for
 * this data.
 *
 * Returns pointer to data buffer or NULL on error.  Note
 * that the calling routine must free the data pointer
 * returned.
 */

static void *load_plane_int8(void *buffer,
			     MDV_field_header_t *field_hdr,
			     int data_type,
			     int plane_num,
			     int *return_size)
{
  static char *routine_name = "load_plane_int8";
  
  char *buf_ptr;
  
  void *return_buf;
  
  int plane_size;
  int plane_loc;
  
  /*
   * Compute size of returned plane.
   */

  plane_size = field_hdr->nx * field_hdr->ny * sizeof(ui08);

  /*
   * Compute where the data is located in the buffer.
   */

  plane_loc = plane_num * plane_size;
    
  /*
   * Move to the data position.
   */

  buf_ptr = (char *)buffer + plane_loc;
  
  /*
   * No swapping necessary - byte data
   */

  /*
   * Convert the data to the desired return type.
   */

  switch(data_type)
  {
  case MDV_INT8 :
    return_buf = umalloc(plane_size);
    memcpy(return_buf, buf_ptr, plane_size);
    *return_size = plane_size;
    break;
    
  default:
    fprintf(stderr,
	    "%s: Cannot convert %s plane to %s format -- not yet implemented\n",
	    routine_name, MDV_encode2string(MDV_INT8),
	    MDV_encode2string(data_type));
    *return_size = 0;
    return(NULL);
  } /* endswitch - data_type */
  
  return(return_buf);
  
} /* end read_plane_int8 */


/********************************************************
 * LOAD_PLANE_PLANE_RLE8
 * Routine for loading a plane of data in MDV_PLANE_RLE8
 * format from the given buffer and returning it in the
 * specified format. The buffer pointer is assumed to point
 * to the beginning of the volume data, after the FORTRAN
 * record length. All necessary byte swapping is done in
 * this routine.
 *
 * This routine assumes that the plane number is valid for
 * this data.
 *
 * Returns pointer to data buffer or NULL on error.  Note
 * that the calling routine must free the data pointer
 * returned.
 */

static void *load_plane_plane_rle8(void *buffer,
				   MDV_field_header_t *field_hdr,
				   int data_type,
				   int plane_num,
				   int *return_size)
{
  static char *routine_name = "load_plane_plane_rle8";
  
  char *buf_ptr = (char *)buffer;
  
  void *return_buf;
  void *encode_buf;

  si32 *vlevel_locs;
  si32 *vlevel_sizes;
  int array_size = field_hdr->nz * sizeof(si32);
  ui32 nbytes;
  si32 *lptr;
  
  /*
   * Allocate memory for location array
   */

  if ((vlevel_locs = (si32 *)umalloc(array_size)) == NULL)
  {
    fprintf(stderr,
	    "%s: Error allocating %d bytes for vlevel_locs array\n",
	    routine_name, array_size);
    return(NULL);
  }

  /*
   * Allocate memory for size array
   */

  if ((vlevel_sizes = (si32 *)umalloc(array_size)) == NULL)
  {
    fprintf(stderr,
	    "%s: Error allocating %d bytes for vlevel_sizes array\n",
	    routine_name, array_size);
    ufree(vlevel_locs);
    return(NULL);
  }

  /*
   * Load the location information.
   */
  
  memcpy(vlevel_locs, buf_ptr, array_size);
  buf_ptr += array_size;
  
  /*
   * Load the size information.
   */

  memcpy(vlevel_sizes, buf_ptr, array_size);
  buf_ptr += array_size;
  
  /*
   * Swap the plane information, if necessary.
   */

  BE_to_array_32(vlevel_locs, array_size);
  BE_to_array_32(vlevel_sizes, array_size);
  
  /*
   * Go to beginning of this plane data.
   */

  encode_buf = buf_ptr + vlevel_locs[plane_num];
  
  /*
   * Swap the encoded plane.
   */

  lptr = encode_buf;
    
  lptr[0] = BE_to_si32(lptr[0]);   /* RL8_FLAG */
  lptr[1] = BE_to_si32(lptr[1]);   /* key */
  lptr[2] = BE_to_si32(lptr[2]);   /* nbytes_array */
  lptr[3] = BE_to_si32(lptr[3]);   /* nbytes_full */
  lptr[4] = BE_to_si32(lptr[4]);   /* nbytes_coded */
  
  /*
   * Put the data into the desired format.
   */

  switch(data_type)
  {
  case MDV_INT8:
    /*
     * Decode this plane of data
     */

    return_buf = (void *)uRLDecode8(encode_buf, &nbytes);

    if (return_buf == NULL ||
	nbytes != field_hdr->nx * field_hdr->ny * INT8_SIZE)
    {
      fprintf(stderr,
	      "%s: Error decoding buffer into %s format\n",
	      routine_name, MDV_encode2string(MDV_INT8));

      if (return_buf != NULL)
      {
	ufree(return_buf);
	*return_size = 0;
	return_buf = NULL;
      }
    }
    
    *return_size = nbytes;
    break;
    
  case MDV_PLANE_RLE8 :
    return_buf = encode_buf;
    *return_size = lptr[2];
    break;
    
  default:
    fprintf(stderr,
	    "%s: Cannot convert %s data into %s format -- not yet implemented\n",
	    routine_name, MDV_encode2string(MDV_PLANE_RLE8),
	    MDV_encode2string(data_type));
    ufree(vlevel_locs);
    ufree(vlevel_sizes);
    *return_size = 0;
    return(NULL);
    break;
  } /* endswitch - data_type */
  
  /*
   * Clean up
   */

  ufree(vlevel_locs);
  ufree(vlevel_sizes);

  return(return_buf);
  
} /* end load_plane_plane_rle8 */

