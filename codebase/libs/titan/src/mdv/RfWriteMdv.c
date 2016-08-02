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
 *
 * RfWriteMdv.c
 *
 * part of the rfutil library - radar file access
 *
 * Routines to write the Dobson information in MDV format
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * February 1997
 *
 **************************************************************************/

#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_write.h>
#include <titan/radar.h>
#include <titan/file_io.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pjg_types.h>

#define MAX_SEQ 256

#define DOBSON_CHUNK_VOL_PARAMS_INFO RADAR_VOLUME_FILE_TYPE
#define DOBSON_CHUNK_ELEVATIONS_INFO RADAR_VOLUME_FILE_TYPE

#define UNSCALE_INT(value, scale)  ((double)(value)/(double)(scale))


/*************************************************************************
 *
 * RfCreateMdvData()
 *
 * part of the rfutil library - radar file access
 *
 * the data volume for a field from the data in the Dobson volume file
 * index
 *
 **************************************************************************/

#define THIS_ROUTINE "RfCreateMdvData"

void RfCreateMdvData(vol_file_handle_t *v_handle,
		     void **plane_array,
		     int field,
		     char *calling_routine)
{
  char calling_sequence[MAX_SEQ];

  ui08 *data = NULL;
  long plane_size = v_handle->vol_params->cart.nx *
    v_handle->vol_params->cart.ny;
  long volume_size = plane_size * v_handle->vol_params->cart.nz;
  
  int iplane;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * Allocate space for the data
   */

  data = (ui08 *)umalloc(volume_size * sizeof(ui08));
  
  /*
   * Copy the data for each plane
   */

  for (iplane = 0; iplane < v_handle->vol_params->cart.nz; iplane++)
  {
    plane_array[iplane] = (void *)(data + (iplane * plane_size));
    
    memcpy((char *)(plane_array[iplane]),
	   v_handle->field_plane[field][iplane], plane_size);
  }
  
  return;
}

#undef THIS_ROUTINE


/*************************************************************************
 *
 * RfCreateMdvElevationsChunk()
 *
 * part of the rfutil library - radar file access
 *
 * creates an elevations chunk from the information in a volume
 * file index
 *
 * returns a pointer to the created chunk header and allocates space for
 * and sets the chunk_data pointer in the argument list, returns NULL on
 * failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfCreateMdvElevationsChunk"

MDV_chunk_header_t *RfCreateMdvElevationsChunk(vol_file_handle_t *v_handle,
					       void **data,
					       char *calling_routine)
{
  char calling_sequence[MAX_SEQ];

  MDV_chunk_header_t *chunk_hdr = NULL;
  si32 *chunk_data;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * If the Dobson object doesn't contain any elevation information,
   * return NULL.
   */

  if (v_handle->vol_params->radar.nelevations <= 0)
    return(NULL);
  
  /*
   * Allocate space for the chunk header
   */

  chunk_hdr = (MDV_chunk_header_t *)ucalloc(1, sizeof(MDV_chunk_header_t));
  
  /*
   * Fill in the header parameters
   */

  chunk_hdr->record_len1 = sizeof(MDV_chunk_header_t) - (2 * sizeof(si32));
  chunk_hdr->struct_id = MDV_CHUNK_HEAD_MAGIC_COOKIE;
  
  chunk_hdr->chunk_id = MDV_CHUNK_DOBSON_ELEVATIONS;
  chunk_hdr->chunk_data_offset = 0;
  chunk_hdr->size = v_handle->vol_params->radar.nelevations * sizeof(si32);
  
  strncpy((char *)chunk_hdr->info,
	  DOBSON_CHUNK_ELEVATIONS_INFO,
	  MDV_CHUNK_INFO_LEN);
  chunk_hdr->info[MDV_CHUNK_INFO_LEN-1] = '\0';
  
  chunk_hdr->record_len2 = chunk_hdr->record_len1;
  
  /*
   * Copy the data for the chunk
   */

  chunk_data = (si32 *)umalloc(v_handle->vol_params->radar.nelevations *
			       sizeof(si32));
  memcpy((char *)chunk_data, (char *)v_handle->radar_elevations,
	 chunk_hdr->size);
  
  *data = (void *)chunk_data;
  
  return(chunk_hdr);
}

#undef THIS_ROUTINE


/*************************************************************************
 *
 * RfCreateMdvFieldHdr()
 *
 * part of the rfutil library - radar file access
 *
 * creates an MDV field header for the indicated field from the
 * information in a volume file index
 *
 * returns a pointer to the created field header, NULL on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfCreateMdvFieldHdr"

MDV_field_header_t *RfCreateMdvFieldHdr(vol_file_handle_t *v_handle,
					int field_num,
					char *calling_routine)
{
  char calling_sequence[MAX_SEQ];

  MDV_field_header_t *field_hdr = NULL;
  
  int i;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * Allocate space for the field header
   */

  field_hdr = (MDV_field_header_t *)ucalloc(1, sizeof(MDV_field_header_t));

  /*
   * Fill in the fields
   */

  field_hdr->record_len1 = sizeof(MDV_field_header_t) - (2 * sizeof(si32));
  field_hdr->struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;
  
  field_hdr->field_code = 0;
  
  field_hdr->forecast_delta = 0;
  field_hdr->forecast_time = Rfrtime2utime(&v_handle->vol_params->mid_time);
  
  field_hdr->nx = v_handle->vol_params->cart.nx;
  field_hdr->ny = v_handle->vol_params->cart.ny;
  field_hdr->nz = v_handle->vol_params->cart.nz;
  
  if (strstr(v_handle->vol_params->cart.unitsy, "deg")) {
    field_hdr->proj_type = MDV_PROJ_POLAR_RADAR;
  } else {
    if (v_handle->vol_params->cart.scalex !=
	v_handle->vol_params->cart.km_scalex) {
      field_hdr->proj_type = MDV_PROJ_LATLON;
    } else {
      field_hdr->proj_type = MDV_PROJ_FLAT;
    }
  }

  field_hdr->encoding_type = MDV_INT8;
  field_hdr->data_element_nbytes = 1;
  
  field_hdr->field_data_offset = 0;
  
  field_hdr->volume_size = field_hdr->nx * field_hdr->ny *
    field_hdr->nz * sizeof(ui08);
  
  field_hdr->proj_origin_lat =
    UNSCALE_INT(v_handle->vol_params->cart.latitude,
		1000000);
  field_hdr->proj_origin_lon =
    UNSCALE_INT(v_handle->vol_params->cart.longitude,
		1000000);
  for (i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    field_hdr->proj_param[i] = 0.0;
  field_hdr->vert_reference = 0.0;
  
  field_hdr->grid_dx = UNSCALE_INT(v_handle->vol_params->cart.dx,
				   v_handle->vol_params->cart.scalex);
  field_hdr->grid_dy = UNSCALE_INT(v_handle->vol_params->cart.dy,
				   v_handle->vol_params->cart.scaley);
  if (v_handle->vol_params->cart.dz == -1)
    field_hdr->grid_dz = -1;
  else
    field_hdr->grid_dz = UNSCALE_INT(v_handle->vol_params->cart.dz,
				     v_handle->vol_params->cart.scalez);
  
  field_hdr->grid_minx = UNSCALE_INT(v_handle->vol_params->cart.minx,
				     v_handle->vol_params->cart.scalex);
  field_hdr->grid_miny = UNSCALE_INT(v_handle->vol_params->cart.miny,
				     v_handle->vol_params->cart.scaley);
  if (v_handle->vol_params->cart.minz == -1)
    field_hdr->grid_minz = -1;
  else
    field_hdr->grid_minz = UNSCALE_INT(v_handle->vol_params->cart.minz,
				       v_handle->vol_params->cart.scalez);
  
  field_hdr->scale =
    UNSCALE_INT(v_handle->field_params[field_num]->scale,
		v_handle->field_params[field_num]->factor);
  field_hdr->bias =
    UNSCALE_INT(v_handle->field_params[field_num]->bias,
		v_handle->field_params[field_num]->factor);
  
  field_hdr->bad_data_value = 0;
  field_hdr->missing_data_value =
    v_handle->field_params[field_num]->missing_val;
  
  field_hdr->proj_rotation =
    UNSCALE_INT(v_handle->vol_params->cart.rotation,
		1000000);
  
  strncpy((char *)field_hdr->field_name_long,
	  v_handle->field_params[field_num]->name,
	  MDV_LONG_FIELD_LEN);
  field_hdr->field_name_long[MDV_LONG_FIELD_LEN-1] = '\0';
  strncpy((char *)field_hdr->field_name,
	  v_handle->field_params[field_num]->name,
	  MDV_SHORT_FIELD_LEN);
  field_hdr->field_name[MDV_SHORT_FIELD_LEN-1] = '\0';
  strncpy((char *)field_hdr->units,
	  v_handle->field_params[field_num]->units,
	  MDV_UNITS_LEN);
  field_hdr->units[MDV_UNITS_LEN-1] = '\0';
  strncpy((char *)field_hdr->transform,
	  v_handle->field_params[field_num]->transform,
	  MDV_TRANSFORM_LEN);
  field_hdr->transform[MDV_TRANSFORM_LEN-1] = '\0';
  
  field_hdr->record_len2 = field_hdr->record_len1;
  
  return(field_hdr);
}

#undef THIS_ROUTINE


/*************************************************************************
 *
 * RfCreateMdvMasterHdr()
 *
 * part of the rfutil library - radar file access
 *
 * creates an MDV master header from the information in a volume file index
 *
 * returns a pointer to the created master header, NULL on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfCreateMdvMasterHdr"

MDV_master_header_t *RfCreateMdvMasterHdr(vol_file_handle_t *v_handle,
					  char *calling_routine)
{
  char calling_sequence[MAX_SEQ];

  MDV_master_header_t *master_hdr = NULL;
  int proj_type;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * Determine the projection type for use in calculating other fields.
   */

  if (strstr(v_handle->vol_params->cart.unitsy, "deg"))
    proj_type = MDV_PROJ_POLAR_RADAR;
  else
  {
    if (v_handle->vol_params->cart.scalex != v_handle->vol_params->cart.km_scalex)
      proj_type = MDV_PROJ_LATLON;
    else
      proj_type = MDV_PROJ_FLAT;
  }
  
  /*
   * Allocate space for the master header.
   */

  master_hdr =
    (MDV_master_header_t *)ucalloc(1, sizeof(MDV_master_header_t));
  
  /*
   * Update the fields in the header
   */

  master_hdr->record_len1 = sizeof(MDV_master_header_t) - (2 * sizeof(si32));
  master_hdr->struct_id = MDV_MASTER_HEAD_MAGIC_COOKIE;
  master_hdr->revision_number = MDV_REVISION_NUMBER;
  
  master_hdr->time_gen = Rfrtime2utime(&v_handle->vol_params->file_time);
  master_hdr->time_begin = Rfrtime2utime(&v_handle->vol_params->start_time);
  master_hdr->time_end = Rfrtime2utime(&v_handle->vol_params->end_time);
  master_hdr->time_centroid = Rfrtime2utime(&v_handle->vol_params->mid_time);
  master_hdr->time_expire = master_hdr->time_end;
  
  master_hdr->num_data_times = 0;
  master_hdr->index_number = 0;
  
  master_hdr->data_dimension = 3;
  master_hdr->data_collection_type = MDV_DATA_MEASURED;

  master_hdr->native_vlevel_type = MDV_VERT_TYPE_ELEV;

  if (strstr(v_handle->vol_params->cart.unitsz, "deg")) {
    master_hdr->vlevel_type = MDV_VERT_TYPE_ELEV;
  } else {
    master_hdr->vlevel_type = MDV_VERT_TYPE_Z;
  }

  master_hdr->vlevel_included = TRUE;
  
  master_hdr->grid_order_direction = MDV_ORIENT_SN_WE;
  master_hdr->grid_order_indices = MDV_ORDER_XYZ;
  
  master_hdr->n_fields = v_handle->vol_params->nfields;
  
  master_hdr->max_nx = v_handle->vol_params->cart.nx;
  master_hdr->max_ny = v_handle->vol_params->cart.ny;
  master_hdr->max_nz = v_handle->vol_params->cart.nz;
  
  if (v_handle->vol_params->radar.nelevations <= 0)
    master_hdr->n_chunks = 1;   /* just vol params */
  else
    master_hdr->n_chunks = 2;   /* vol params and elevations */
  
  master_hdr->field_hdr_offset = 0;
  master_hdr->vlevel_hdr_offset = 0;
  master_hdr->chunk_hdr_offset = 0;
  
  if (proj_type == MDV_PROJ_LATLON)
  {
    master_hdr->sensor_lon =
      UNSCALE_INT(v_handle->vol_params->cart.radarx,
		  v_handle->vol_params->cart.scalex);
    master_hdr->sensor_lat =
      UNSCALE_INT(v_handle->vol_params->cart.radary,
		  v_handle->vol_params->cart.scaley);
  }
  else
  {
    double sensor_lat;
    double sensor_lon;
    
    PJGstruct *pjg_struct =
      PJGs_flat_init(UNSCALE_INT(v_handle->vol_params->cart.latitude,
				 1000000),
		     UNSCALE_INT(v_handle->vol_params->cart.longitude,
				 1000000),
		     UNSCALE_INT(v_handle->vol_params->cart.rotation,
				 1000000));
    
    PJGs_flat_xy2latlon(pjg_struct,
			UNSCALE_INT(v_handle->vol_params->cart.radarx,
				    v_handle->vol_params->cart.scalex),
			UNSCALE_INT(v_handle->vol_params->cart.radary,
				    v_handle->vol_params->cart.scaley),
			&(sensor_lat),
			&(sensor_lon));

    master_hdr->sensor_lat = sensor_lat;
    master_hdr->sensor_lon = sensor_lon;
  }
  
  master_hdr->sensor_alt = UNSCALE_INT(v_handle->vol_params->cart.radarz,
				       v_handle->vol_params->cart.scalez);
    
  strncpy((char *)master_hdr->data_set_info,
	  v_handle->vol_params->note,
	  MDV_INFO_LEN);
  master_hdr->data_set_info[MDV_INFO_LEN-1] = '\0';
  strncpy((char *)master_hdr->data_set_name,
	  v_handle->vol_file_label,
	  MDV_NAME_LEN);
  master_hdr->data_set_name[MDV_NAME_LEN-1] = '\0';
  if (v_handle->vol_file_path == (char *)NULL)
    master_hdr->data_set_source[0] = '\0';
  else
    strncpy((char *)master_hdr->data_set_source,
	    v_handle->vol_file_path,
	    MDV_NAME_LEN);
  master_hdr->data_set_source[MDV_NAME_LEN-1] = '\0';
  
  master_hdr->record_len2 = master_hdr->record_len1;
  
  return(master_hdr);
}

#undef THIS_ROUTINE


/*************************************************************************
 *
 * RfCreateMdvVlevelHdr()
 *
 * part of the rfutil library - radar file access
 *
 * creates an MDV vlevel header from the information in a volume file
 * index.  The vlevel header is the same for every field in a volume
 * file.
 *
 * returns a pointer to the created vlevel header, NULL on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfCreateMdvVlevelHdr"

MDV_vlevel_header_t *RfCreateMdvVlevelHdr(vol_file_handle_t *v_handle,
					  char *calling_routine)
{
  char calling_sequence[MAX_SEQ];

  MDV_vlevel_header_t *vlevel_hdr = NULL;
  int level;
  int vlevel_type;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * set vlevel type
   */
  
  if (strstr(v_handle->vol_params->cart.unitsz, "deg")) {
    vlevel_type = MDV_VERT_TYPE_ELEV;
  } else {
    vlevel_type = MDV_VERT_TYPE_Z;
  }

  /*
   * Allocate space for the vlevel header
   */

  vlevel_hdr = (MDV_vlevel_header_t *)ucalloc(1, sizeof(MDV_vlevel_header_t));
  
  /*
   * Fill in the fields in the header
   */

  vlevel_hdr->record_len1 = sizeof(MDV_vlevel_header_t) - (2 * sizeof(si32));
  vlevel_hdr->struct_id = MDV_VLEVEL_HEAD_MAGIC_COOKIE;
  
  for (level = 0; level < v_handle->vol_params->cart.nz; level++)
  {
    vlevel_hdr->vlevel_type[level] = vlevel_type;
    vlevel_hdr->vlevel_params[level] =
      (float) v_handle->plane_heights[level][PLANE_MIDDLE_INDEX] /
      (float) v_handle->vol_params->cart.km_scalez;
  }
  
  for (level = v_handle->vol_params->cart.nz;
       level < MDV_MAX_VLEVELS; level++)
  {
    vlevel_hdr->vlevel_type[level] = 0;
    vlevel_hdr->vlevel_params[level] = 0;
  }
  
  vlevel_hdr->record_len2 = vlevel_hdr->record_len1;
 
  return(vlevel_hdr);
}

#undef THIS_ROUTINE


/*************************************************************************
 *
 * RfCreateMdvVolParamsChunk()
 *
 * part of the rfutil library - radar file access
 *
 * creates a volume parameters chunk from the information in a volume
 * file index
 *
 * returns a pointer to the created chunk header and allocates space for
 * and sets the chunk_data pointer in the argument list, returns NULL on
 * failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfCreateMdvVolParamsChunk"

MDV_chunk_header_t *RfCreateMdvVolParamsChunk(vol_file_handle_t *v_handle,
					      void **data,
					      char *calling_routine)
{
  char calling_sequence[MAX_SEQ];

  MDV_chunk_header_t *chunk_hdr = NULL;
  vol_params_t *chunk_data;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * Allocate space for the chunk header
   */

  chunk_hdr = (MDV_chunk_header_t *)ucalloc(1, sizeof(MDV_chunk_header_t));
  
  /*
   * Fill in the header parameters
   */

  chunk_hdr->record_len1 = sizeof(MDV_chunk_header_t) - (2 * sizeof(si32));
  chunk_hdr->struct_id = MDV_CHUNK_HEAD_MAGIC_COOKIE;
  
  chunk_hdr->chunk_id = MDV_CHUNK_DOBSON_VOL_PARAMS;
  chunk_hdr->chunk_data_offset = 0;
  chunk_hdr->size = sizeof(vol_params_t);
  
  strncpy((char *)chunk_hdr->info,
	  DOBSON_CHUNK_VOL_PARAMS_INFO,
	  MDV_CHUNK_INFO_LEN);
  chunk_hdr->info[MDV_CHUNK_INFO_LEN-1] = '\0';
  
  chunk_hdr->record_len2 = chunk_hdr->record_len1;
  
  /*
   * Copy the data for the chunk
   */

  chunk_data = (vol_params_t *)umalloc(sizeof(vol_params_t));
  *chunk_data = *v_handle->vol_params;
  
  *data = (void *)chunk_data;
  
  return(chunk_hdr);
}

#undef THIS_ROUTINE


/*************************************************************************
 *
 * RfWriteVolumeMdv()
 *
 * part of the rfutil library - radar file access
 *
 * writes a radar volume to an MDV file
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfWriteVolumeMdv"

int RfWriteVolumeMdv(vol_file_handle_t *v_handle,
		     int mdv_encoding_type,
		     char *calling_routine)
{
  char calling_sequence[MAX_SEQ];

  char dir_path[MAX_PATH_LEN];

  si32 ifield;

  mode_t mode;
  path_parts_t fparts;
  date_time_t date_time;

  MDV_master_header_t *master_hdr;
  MDV_field_header_t *field_hdr;
  MDV_vlevel_header_t *vlevel_hdr;
  MDV_chunk_header_t *chunk_hdr;
  void **plane_array;
  void **chunk_data;
  
  int next_offset;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * set file time to gmt
   */

  ugmtime(&date_time);
  Rfdtime2rtime(&date_time, &v_handle->vol_params->file_time);

  /*
   * try to open the file
   */
    
  if ((v_handle->vol_file = fopen(v_handle->vol_file_path, "w")) == NULL)
  {
    /*
     * File cannot be opened - probably the directory does
     * not exist. Parse the file path to get at the directory name.
     */

    uparse_path(v_handle->vol_file_path, &fparts);
    memset ((void *) dir_path,
            (int) 0, (size_t)  MAX_PATH_LEN);
    memcpy ((void *) dir_path,
            (void *) fparts.dir,
            (size_t) strlen(fparts.dir) - 1);
    
    /*
     * create directory
     */
    
    mode = 0xffff;

    if (mkdir(dir_path, mode) != 0)
    {
      fprintf(stderr, "ERROR - %s:%s:%s.\n",
	      v_handle->prog_name, calling_routine, THIS_ROUTINE);
      fprintf(stderr, "Cannot create radar data subdirectory.\n");
      perror(dir_path);
      return (R_FAILURE);
    }
      
    /*
     * try opening file again
     */
    
    if ((v_handle->vol_file = fopen(v_handle->vol_file_path, "w")) == NULL) {
      fprintf(stderr, "ERROR - %s:%s:%s.\n",
	      v_handle->prog_name, calling_routine, THIS_ROUTINE);
      fprintf(stderr, "Creating rdata file.\n");
      perror(v_handle->vol_file_path);
      return (R_FAILURE);
    }
    
  } /* if ((v_handle->vol_file = fopen(vol_file_path, "w")) == NULL) */

  /*
   * Convert and write the master header information
   */

  if ((master_hdr = RfCreateMdvMasterHdr(v_handle,
					 calling_sequence)) == NULL)
  {
    fprintf(stderr, "ERROR - %s:%s:%s.\n",
	    v_handle->prog_name, calling_routine, THIS_ROUTINE);
    fprintf(stderr, "Error creating master header from v_handle\n");
    fclose(v_handle->vol_file);
    return(R_FAILURE);
  }
  
  MDV_set_master_hdr_offsets(master_hdr);
  
  if (MDV_write_master_header(v_handle->vol_file, master_hdr)
      != MDV_SUCCESS)
  {
    fprintf(stderr, "ERROR - %s:%s:%s.\n",
	    v_handle->prog_name, calling_routine, THIS_ROUTINE);
    fprintf(stderr, "Cannot write master header to outfile.\n");
    ufree(master_hdr);
    fclose(v_handle->vol_file);
    return(R_FAILURE);
  }
  
  /*
   * Convert and write each of the field headers and the associated
   * field data
   */

  plane_array = (void **)umalloc(v_handle->vol_params->cart.nz *
				 sizeof(void *));
    
  next_offset = MDV_get_first_field_offset(master_hdr);
  
  for (ifield = 0; ifield < v_handle->vol_params->nfields; ifield++)
  {
    int write_size;
    
    if ((field_hdr = RfCreateMdvFieldHdr(v_handle,
					 ifield,
					 calling_sequence)) == NULL)
    {
      fprintf(stderr, "ERROR - %s:%s:%s.\n",
	      v_handle->prog_name, calling_routine, THIS_ROUTINE);
      fprintf(stderr, "Error creating field header for field %d.\n",
	      ifield);
      ufree(master_hdr);
      fclose(v_handle->vol_file);
      return(R_FAILURE);
    }
	
    RfCreateMdvData(v_handle,
		    plane_array,
		    ifield,
		    calling_sequence);
    
    if ((write_size = MDV_write_field(v_handle->vol_file,
				      field_hdr, plane_array[0],
				      ifield, next_offset,
				      mdv_encoding_type)) < 0)
    {
      fprintf(stderr, "ERROR - %s:%s:%s.\n",
	      v_handle->prog_name, calling_routine, THIS_ROUTINE);
      fprintf(stderr, "Error writing field %d information.\n",
	      ifield);

      ufree(master_hdr);
      ufree(field_hdr);
      ufree(plane_array[0]);
      ufree(plane_array);
      fclose(v_handle->vol_file);
      return(R_FAILURE);
    }
    
    next_offset += write_size + 2 * sizeof(si32);
    
    ufree(field_hdr);
    ufree(plane_array[0]);
  }
  
  ufree(plane_array);
  
  /*
   * Create and write the vlevel headers.  Note that each field will
   * have the same vlevel header so we only need to create it once.
   */

  if ((vlevel_hdr = RfCreateMdvVlevelHdr(v_handle,
					 calling_sequence)) == NULL)
  {
    fprintf(stderr, "ERROR - %s:%s:%s.\n",
	    v_handle->prog_name, calling_routine, THIS_ROUTINE);
    fprintf(stderr, "Error creating vlevel header for field %d.\n",
	    ifield);
    ufree(master_hdr);
    fclose(v_handle->vol_file);
    return(R_FAILURE);
  }
    
  for (ifield = 0; ifield < v_handle->vol_params->nfields; ifield++)
  {
    if (MDV_write_vlevel_header(v_handle->vol_file, vlevel_hdr,
				master_hdr, ifield) != MDV_SUCCESS)
    {
      fprintf(stderr, "ERROR - %s:%s:%s.\n",
	      v_handle->prog_name, calling_routine, THIS_ROUTINE);
      fprintf(stderr, "Cannot write vlevel header for field %d to MDV file.\n",
	      ifield);

      ufree(vlevel_hdr);
      ufree(master_hdr);
      fclose(v_handle->vol_file);
      return(R_FAILURE);
    }

  } /* endfor - ifield */

  ufree(vlevel_hdr);
  
  /*
   * Create and write the chunk information.
   */

  if ((chunk_hdr = RfCreateMdvVolParamsChunk(v_handle,
					     (void **)(&chunk_data),
					     calling_sequence)) == NULL)
  {
    fprintf(stderr, "ERROR - %s:%s:%s.\n",
	    v_handle->prog_name, calling_routine, THIS_ROUTINE);
    fprintf(stderr, "Error creating volume params chunk.\n");
    
    ufree(master_hdr);
    fclose(v_handle->vol_file);
    return(R_FAILURE);
  }
  
  if (MDV_write_chunk(v_handle->vol_file, chunk_hdr, chunk_data,
		      master_hdr, 0, next_offset, TRUE) != MDV_SUCCESS)
  {
    fprintf(stderr, "ERROR - %s:%s:%s.\n",
	    v_handle->prog_name, calling_routine, THIS_ROUTINE);
    fprintf(stderr, "Cannot write vol params chunk to MDV file.\n");

    ufree(chunk_data);
    ufree(chunk_hdr);
    ufree(master_hdr);
    fclose(v_handle->vol_file);
    return(R_FAILURE);
  }
  
  ufree(chunk_data);
  ufree(chunk_hdr);
  
  next_offset += sizeof(vol_params_t) + 2 * sizeof(si32);
  
  if ((chunk_hdr = RfCreateMdvElevationsChunk(v_handle,
					      (void **)(&chunk_data),
					      calling_sequence))
      != NULL)
  {
    if (MDV_write_chunk(v_handle->vol_file, chunk_hdr, chunk_data,
			master_hdr, 1, next_offset, TRUE) != MDV_SUCCESS)
    {
      fprintf(stderr, "ERROR - %s:%s:%s.\n",
	      v_handle->prog_name, calling_routine, THIS_ROUTINE);
      fprintf(stderr, "Cannot write elevations chunk to MDV file.\n");

      ufree(chunk_data);
      ufree(chunk_hdr);
      ufree(master_hdr);
      fclose(v_handle->vol_file);
      return(R_FAILURE);
    }

    ufree(chunk_data);
    ufree(chunk_hdr);
  }
  
  /*
   * Free the rest of the memory used by this routine
   */

  ufree(master_hdr);
  
  /*
   * close file and return
   */
  
  fclose(v_handle->vol_file);
  return (R_SUCCESS);

}

#undef THIS_ROUTINE
