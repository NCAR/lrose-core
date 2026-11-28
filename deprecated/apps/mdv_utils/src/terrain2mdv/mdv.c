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
 * mdv.c
 *
 * Routines to write the McIdas information in MDV format
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * February 1997
 *
 **************************************************************************/

#include "terrain2mdv.h"
#include <mdv/mdv_user.h>
#include <mdv/mdv_write.h>
#include <titan/file_io.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pjg_types.h>
#include <toolsa/str.h>

static MDV_master_header_t Master_hdr;
static MDV_field_header_t Field_hdr;

/*************************************************************************
 *
 * LoadMdvFieldHdr()
 *
 * loads MDV field header
 *
 **************************************************************************/

void LoadMdvFieldHdr(time_t image_time)

{

  int i;

  /*
   * Fill in the fields
   */

  Field_hdr.record_len1 = sizeof(MDV_field_header_t) - (2 * sizeof(si32));
  Field_hdr.struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;
  
  Field_hdr.field_code = 0;
  
  Field_hdr.forecast_time = image_time;
  Field_hdr.forecast_delta = 0;
  
  
  if ( Glob->params.proj == FLAT ) {
    Field_hdr.proj_type = MDV_PROJ_FLAT;
    Field_hdr.nx = Glob->nx;
    Field_hdr.ny = Glob->ny;
    Field_hdr.nz = 1;
  } else {
    Field_hdr.proj_type = MDV_PROJ_LATLON;
    Field_hdr.nx = Map.nlons;
    Field_hdr.ny = Map.nlats;
    Field_hdr.nz = 1;
  }

  Field_hdr.encoding_type = MDV_INT8;
  Field_hdr.data_element_nbytes = 1;
  
  Field_hdr.field_data_offset = 0;
  
  Field_hdr.volume_size = Field_hdr.nx * Field_hdr.ny *
    Field_hdr.nz * sizeof(ui08);
  
  memset(Field_hdr.unused_si32, 0, sizeof(Field_hdr.unused_si32));
  
  /* Map. fields are all in arc seconds, since the input file is in arc seconds */

  Field_hdr.proj_origin_lat = Glob->params.lat;
                              /*(Map.south + (Map.north-Map.south)/2.0) / 3600.0;*/
  Field_hdr.proj_origin_lon = Glob->params.lon;
                              /*(Map.west + (Map.east-Map.west)/2.0) / 3600.0;*/
  
  for (i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    Field_hdr.proj_param[i] = 0.0;
  Field_hdr.vert_reference = 0.0;
  
  if ( Glob->params.proj == FLAT ){
    Field_hdr.grid_dx = Glob->dx;
    Field_hdr.grid_dy = Glob->dy;
    Field_hdr.grid_dz = 1;

    Field_hdr.grid_minx = Glob->params.xmin;
    Field_hdr.grid_miny = Glob->params.ymin;
    Field_hdr.grid_minz = 0;
  } else {
    Field_hdr.grid_dx = (Map.lon_spacing) / 3600.0;
    Field_hdr.grid_dy = (Map.lat_spacing) / 3600.0;
    Field_hdr.grid_dz = 1;

    Field_hdr.grid_minx = (Map.west) / 3600.0;
    Field_hdr.grid_miny = (Map.south) / 3600.0;
    Field_hdr.grid_minz = 0;
  }

  Field_hdr.scale = Glob->params.scale;
  Field_hdr.bias = Glob->params.bias;
  
  Field_hdr.bad_data_value = BAD_VALUE;
  Field_hdr.missing_data_value = BAD_VALUE;
  
  Field_hdr.proj_rotation = 0.0;
  Field_hdr.field_code = 5;
  
  memset(Field_hdr.unused_fl32, 0, sizeof(Field_hdr.unused_fl32));
  
  STRncopy((char *)Field_hdr.field_name_long,
	   "Elevation",
	   MDV_LONG_FIELD_LEN);

  STRncopy((char *)Field_hdr.field_name,
	  "Elevation",
	  MDV_SHORT_FIELD_LEN);

  STRncopy((char *)Field_hdr.units,
	  "m",
	  MDV_UNITS_LEN);

  STRncopy((char *)Field_hdr.transform, "none",
	  MDV_TRANSFORM_LEN);
  
  memset(Field_hdr.unused_char, 0, sizeof(Field_hdr.unused_char));
  
  Field_hdr.record_len2 = Field_hdr.record_len1;
  
}

/*************************************************************************
 *
 * LoadMdvMasterHdr()
 *
 * loads an MDV master header
 *
 **************************************************************************/

void LoadMdvMasterHdr(time_t time)

{

  /*
   * Update the fields in the header
   */
  
  Master_hdr.record_len1 = sizeof(MDV_master_header_t) - (2 * sizeof(si32));
  Master_hdr.struct_id = MDV_MASTER_HEAD_MAGIC_COOKIE;
  Master_hdr.revision_number = MDV_REVISION_NUMBER;
  
  Master_hdr.time_gen = time;
  Master_hdr.time_begin = time;
  Master_hdr.time_end = time;
  Master_hdr.time_centroid = time;
  Master_hdr.time_expire = Master_hdr.time_end;
  
  Master_hdr.num_data_times = 0;
  Master_hdr.index_number = 0;
  
  Master_hdr.data_dimension = 2;
  Master_hdr.data_collection_type = MDV_DATA_MEASURED;
  Master_hdr.native_vlevel_type = MDV_VERT_TYPE_Z;
  Master_hdr.vlevel_type = MDV_VERT_TYPE_Z;
  Master_hdr.vlevel_included = FALSE;
  
  Master_hdr.grid_order_direction = MDV_ORIENT_SN_WE;
  Master_hdr.grid_order_indices = MDV_ORDER_XYZ;
  
  Master_hdr.n_fields = 1;
  
  if ( Glob->params.proj == FLAT ) {
    Master_hdr.max_nx = Glob->nx;
    Master_hdr.max_ny = Glob->ny;
    Master_hdr.max_nz = 1;
  } else {
    Master_hdr.max_nx = Map.nlons;
    Master_hdr.max_ny = Map.nlats;
    Master_hdr.max_nz = 1;
  }
  
  Master_hdr.n_chunks = 0;

  Master_hdr.field_hdr_offset = 0;
  Master_hdr.vlevel_hdr_offset = 0;
  Master_hdr.chunk_hdr_offset = 0;
  
  memset(Master_hdr.unused_si32, 0, sizeof(Master_hdr.unused_si32));
  
  Master_hdr.sensor_lon = 0.0;
  Master_hdr.sensor_lat = 0.0;
  Master_hdr.sensor_alt = 0.0;
    
  memset(Master_hdr.unused_fl32, 0, sizeof(Master_hdr.unused_fl32));
  
  STRncopy((char *)Master_hdr.data_set_info, "DEM Elevation", MDV_INFO_LEN);
  
  STRncopy((char *)Master_hdr.data_set_name, "DEM Elevation", MDV_NAME_LEN);
  
  STRncopy((char *)Master_hdr.data_set_source, "DEM Elevation", MDV_NAME_LEN);

  Master_hdr.record_len2 = Master_hdr.record_len1;

}

/*************************************************************************
 *
 * WriteMdv()
 *
 * writes an MDV file
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int WriteMdv(date_time_t *image_time, ui08 *data)

{

  char dir_path[MAX_PATH_LEN];
  char mdv_file_path[MAX_PATH_LEN];

  si32 ifield;

  mode_t mode;
  path_parts_t fparts;

  int write_size;
  int next_offset;

  FILE *mdv_file;
  
  /*
   * compute file path
   */

  sprintf(mdv_file_path, "%s%s%s",
	  Glob->params.output_dir, PATH_DELIM,
	  Glob->params.outfname);

  fprintf(stdout, "  Writing output MDV file:\n");
  fprintf(stdout, "    %s\n", mdv_file_path);

  /*
   * try to open the file
   */
    
  if ((mdv_file = fopen(mdv_file_path, "w")) == NULL) {

    /*
     * File cannot be opened - probably the directory does
     * not exist. Parse the file path to get at the directory name.
     */

    uparse_path(mdv_file_path, &fparts);
    memset ((void *) dir_path,
            (int) 0, (size_t)  MAX_PATH_LEN);
    memcpy ((void *) dir_path,
            (void *) fparts.dir,
            (size_t) strlen(fparts.dir) - 1);
    
    /*
     * create directory
     */
    
    mode = 0xffff;

    if (mkdir(dir_path, mode) != 0) {
      fprintf(stderr, "ERROR - %s:write_mdv.\n", Glob->prog_name);
      fprintf(stderr, "Cannot create data subdirectory.\n");
      perror(dir_path);
      return (-1);
    }
      
    /*
     * try opening file again
     */
    
    if ((mdv_file = fopen(mdv_file_path, "w")) == NULL) {
      fprintf(stderr, "ERROR - %s:write_mdv.\n", Glob->prog_name);
      fprintf(stderr, "Creating file.\n");
      perror(mdv_file_path);
      return (-1);
    }
    
  } /* if ((mdv_file = fopen(vol_file_path, "w")) == NULL) */

  MDV_set_master_hdr_offsets(&Master_hdr);
  
  if (MDV_write_master_header(mdv_file, &Master_hdr)
      != MDV_SUCCESS) {
    fprintf(stderr, "ERROR - %s:write_mdv.\n", Glob->prog_name);
    fprintf(stderr, "Cannot write master header to outfile.\n");
    fclose(mdv_file);
    return(-1);
  }
  
  /*
   * Convert and write each of the field headers and the associated
   * field data
   */

  next_offset = MDV_get_first_field_offset(&Master_hdr);
  
  ifield = 0;
  if ((write_size = MDV_write_field(mdv_file,
				    &Field_hdr, data,
				    ifield, next_offset,
				    MDV_INT8)) < 0) {
    fprintf(stderr, "ERROR - %s:write_mdv.\n", Glob->prog_name);
    fprintf(stderr, "Error writing field information.\n");
    fclose(mdv_file);
    return(-1);
  }
    
  next_offset += write_size + 2 * sizeof(si32);
    
  /*
   * close file and return
   */
  
  fclose(mdv_file);
  return (R_SUCCESS);

}

