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
 * RfReadMDV.c
 *
 * part of the rfutil library - radar file access
 *
 * MDV file reading routines.  These routines read information from
 * Dobson or MDV files (the routine determines the type of file 
 * being read) and returns the information in Dobson format to the
 * calling program.
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1996
 *
 **************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <titan/file_io.h>

#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_user.h>

#include <titan/radar.h>

#include <toolsa/pjg_flat.h>
#include <toolsa/str.h>

#define INT_SCALE 1000
#define FPARAMS_INT_SCALE 10000

/*************************************************************************
 * Declare static routines.
 */

static int load_chunk_info(MDV_master_header_t *master_hdr,
			   MDV_field_header_t *field_hdr,
			   vol_file_handle_t *vindex,
			   int *vol_params_loaded_p);

static void update_dobson_volume_params(MDV_master_header_t *master_hdr,
					MDV_field_header_t *field_hdr,
					vol_params_t *vol_params);

static void update_dobson_radar_params(MDV_master_header_t *master_hdr,
				       radar_params_t *radar_params);

static void update_dobson_cart_params(MDV_master_header_t *master_hdr,
				      MDV_field_header_t *field_hdr,
				      cart_params_t *cart_params);

static void update_dobson_field_params(MDV_field_header_t *field_hdr,
				       field_params_t *field_params,
				       char *prog_name,
				       char *calling_routine);


/*************************************************************************
 *
 * RfReadMDVVolume()
 *
 * part of the rfutil library - radar file access
 *
 * reads an MDV radar volume from a file and returns information in
 * dobson format
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define MAX_SEQ  256

int RfReadMDVVolume(vol_file_handle_t *vindex,
		    const char *calling_routine)
{
  static char *this_routine = "RfReadMDVVolume";
  
  char calling_sequence[MAX_SEQ];

  int i,j;
  int nz;
  int vol_params_loaded;

  double z, scalez;
    
  MDV_master_header_t master_hdr;
  MDV_field_header_t  field_hdr;
  MDV_vlevel_header_t vlevel_hdr;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, this_routine);
  
  /*
   * Initialize the information in the Dobson structure and update
   * the volume header.  This must be done before allocating space
   * for the rest of the Dobson arrays since the array sizes are
   * kept in the volume header.
   */

  if (!vindex->handle_initialized)
  {
    vindex->vol_params = (vol_params_t *)calloc(1,
						sizeof(vol_params_t));
    vindex->handle_initialized = TRUE;
  }

  /*
   * Open the MDV file.
   */

  if ((vindex->vol_file =
       Rf_fopen_uncompress(vindex->vol_file_path, "r")) == NULL)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    vindex->prog_name, calling_sequence);
    fprintf(stderr, "Cannot open MDV volume file for reading.\n");
    perror(vindex->vol_file_path);
    return(R_FAILURE);
  }
  
  /*
   * Read in the master header information from the mdv file.
   */

  if (MDV_load_master_header(vindex->vol_file,
			     &master_hdr) != MDV_SUCCESS)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    vindex->prog_name, calling_sequence);
    fprintf(stderr, "Cannot load MDV master header from file %s.\n",
	    vindex->vol_file_path);
    fclose(vindex->vol_file);
    return(R_FAILURE);
  }
  
  /*
   * Update the vindex information.
   */

  if (vindex->vol_file_label == NULL) {
    vindex->vol_file_label = STRdup(RADAR_VOLUME_FILE_TYPE);
  } else {
    strcpy(vindex->vol_file_label, RADAR_VOLUME_FILE_TYPE);
  }
    
  /*
   * Update the volume parameters.  (This initializes the data.  It will
   * be updated again later if the complete volume parameters are kept as
   * chunk data.)  Send in the field header for the first field, if there
   * is one.
   */

  if (RfAllocVolParams(vindex, calling_sequence) != R_SUCCESS)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    vindex->prog_name, calling_sequence);
    fprintf(stderr, "Cannot allocate space for vindex vol params\n");
    fclose(vindex->vol_file);
    return(R_FAILURE);
  }

  /*
   * load chunk info as appropriate
   */
  
  vol_params_loaded = FALSE;
  if(load_chunk_info(&master_hdr, &field_hdr, vindex,
		     &vol_params_loaded) != MDV_SUCCESS) {
    fclose(vindex->vol_file);
    return(R_FAILURE);
  }

  if (master_hdr.n_fields > 0)
  {
    if (MDV_load_field_header(vindex->vol_file, &field_hdr, 0)
	!= MDV_SUCCESS)
    {
      fprintf(stderr, "%s:%s\n",
	      vindex->prog_name, calling_sequence);
      fprintf(stderr,
	      "Cannot load MDV field header for field 0 from file %s.\n",
	      vindex->vol_file_path);
      fclose(vindex->vol_file);
      return(R_FAILURE);
    }
    
    if (!vol_params_loaded)
      update_dobson_volume_params(&master_hdr, &field_hdr,
				  vindex->vol_params);
  }
  else
  {
    if (!vol_params_loaded)
      update_dobson_volume_params(&master_hdr, NULL, vindex->vol_params);
  }
    
  /*
   * Allocate space for the arrays in the Dobson structure.
   */

  if (RfAllocVolArrays(vindex, calling_sequence) != R_SUCCESS)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    vindex->prog_name, calling_sequence);
    fprintf(stderr, "Cannot allocate space for vindex arrays\n");
    fclose(vindex->vol_file);
    return(R_FAILURE);
  }
    
  /*
   * Get the Dobson field params information from the MDV file.
   * Get the field data at the same time so we don't have to read
   * the field headers twice.
   */

  for (i = 0; i < master_hdr.n_fields; i++)
  {
    if (MDV_load_field_header(vindex->vol_file, &field_hdr, i)
	!= MDV_SUCCESS)
    {
      fprintf(stderr, "ERROR - %s:%s\n",
	      vindex->prog_name, calling_sequence);
      fprintf(stderr, "Cannot read field %d header from MDV file.\n",
	      i);
      fclose(vindex->vol_file);
      return(MDV_FAILURE);
    }
    
    update_dobson_field_params(&field_hdr, vindex->field_params[i],
			       vindex->prog_name, calling_sequence);

    /*
     * get the field plane data
     */

    for (j = 0; j < field_hdr.nz; j++) {

      if (vindex->plane_allocated[i][j]) {
	free(vindex->field_plane[i][j]);
	vindex->plane_allocated[i][j] = FALSE;
      }

      if ((vindex->field_plane[i][j] = (ui08 *)
	   MDV_get_plane(vindex->vol_file, &field_hdr,
			 MDV_INT8, j)) == NULL)
	{
	  fprintf(stderr, "ERROR - %s:%s\n",
		  vindex->prog_name, calling_sequence);
	  fprintf(stderr, "Cannot read field %d, plane %d from MDV file.\n",
		  i, j);
	  fclose(vindex->vol_file);
	  return(MDV_FAILURE);
	}

      vindex->plane_allocated[i][j] = TRUE;

    } /* j */

  } /* i */
    
  /*
   * Get the plane heights from the vlevel record for the first
   * field in the MDV file.
   */

  nz = vindex->vol_params->cart.nz;
  scalez = (double) vindex->vol_params->cart.scalez;
    
  if (master_hdr.vlevel_included && master_hdr.n_fields > 0)
    {
    if (MDV_load_vlevel_header(vindex->vol_file, &vlevel_hdr,
			       &master_hdr, 0) != MDV_SUCCESS)
    {
      fprintf(stderr, "ERROR - %s:%s\n",
	      vindex->prog_name, calling_sequence);
      fprintf(stderr, "Cannot read first vlevel header from MDV file.\n");
      fclose(vindex->vol_file);
      return(MDV_FAILURE);
    }

    if (nz == 1) {

      z = vlevel_hdr.vlevel_params[0] - field_hdr.grid_dz / 2.0;
      vindex->plane_heights[0][PLANE_BASE_INDEX] =
	(int)(z * scalez + 0.5);

      z = vlevel_hdr.vlevel_params[0];
      vindex->plane_heights[0][PLANE_MIDDLE_INDEX] =
	(int)(z * scalez + 0.5);

      z = vlevel_hdr.vlevel_params[0] + field_hdr.grid_dz / 2.0;
      vindex->plane_heights[0][PLANE_TOP_INDEX] =
	(int)(z * scalez + 0.5);

    } else {

      /* copy in mid plane heights */

      for (i = 0; i < nz; i++)
	{
	  vindex->plane_heights[i][PLANE_MIDDLE_INDEX] =
	    (int)(vlevel_hdr.vlevel_params[i] * scalez + 0.5);
	}

      /* compute top plane heights */
      
      for (i = 0; i < nz - 1; i++)
	{
	  vindex->plane_heights[i][PLANE_TOP_INDEX] =
	    (int) (((vlevel_hdr.vlevel_params[i] + 
		     vlevel_hdr.vlevel_params[i + 1]) / 2.0) * scalez + 0.5);
	}
      vindex->plane_heights[nz - 1][PLANE_TOP_INDEX] =
	(int) ((vlevel_hdr.vlevel_params[nz - 1] +
		(vlevel_hdr.vlevel_params[nz - 1] - 
		 vlevel_hdr.vlevel_params[nz - 2]) / 2.0) * scalez + 0.5);
      
      /* compute base plane heights */
      
      for (i = 1; i < nz; i++)
	{
	  vindex->plane_heights[i][PLANE_BASE_INDEX] =
	    (int) (((vlevel_hdr.vlevel_params[i] + 
		     vlevel_hdr.vlevel_params[i - 1]) / 2.0) * scalez + 0.5);
	}
      vindex->plane_heights[0][PLANE_BASE_INDEX] =
	(int) ((vlevel_hdr.vlevel_params[0] -
		(vlevel_hdr.vlevel_params[1] - 
		 vlevel_hdr.vlevel_params[0]) / 2.0) * scalez + 0.5);

    } /* if (nz == 1) */
    
  }
  else
  {
    for (i = 0; i < nz; i++)
    {
      if (master_hdr.n_fields > 0)
      {
	/*
	 * We still have the last field header so we can use
	 * that for our calculations.
	 */

	vindex->plane_heights[i][PLANE_MIDDLE_INDEX] =
	  (int) (((field_hdr.grid_minz + i * field_hdr.grid_dz) *
		  scalez) + 0.5); 
	vindex->plane_heights[i][PLANE_BASE_INDEX] =
	  (int) (((field_hdr.grid_minz + i * field_hdr.grid_dz) -
		  field_hdr.grid_dz / 2.0) * scalez + 0.5);
	vindex->plane_heights[i][PLANE_TOP_INDEX] =
	  (int) (((field_hdr.grid_minz + i * field_hdr.grid_dz) +
		  field_hdr.grid_dz / 2.0) * scalez + 0.5);
      }
      else
	{
	  vindex->plane_heights[i][PLANE_BASE_INDEX] = 0;
	  vindex->plane_heights[i][PLANE_MIDDLE_INDEX] = 0;
	  vindex->plane_heights[i][PLANE_TOP_INDEX] = 0;
	}
    }
  }

  /*
   * close
   */

  fclose(vindex->vol_file);
  
  return (R_SUCCESS);
}


/*************************************************************************
 *
 * load_chunk_info()
 *
 * loads info from chunk headers if they exist
 *
 **************************************************************************/

static int load_chunk_info(MDV_master_header_t *master_hdr,
			   MDV_field_header_t *field_hdr,
			   vol_file_handle_t *vindex,
			   int *vol_params_loaded_p)

{
  
  int i;
  MDV_chunk_header_t  chunk_hdr;
  void *chunk_data;
  
  /*
   * Get any chunk information that's appropriate.
   */

  for (i = 0; i < master_hdr->n_chunks; i++)
  {
    if (MDV_load_chunk_header(vindex->vol_file, &chunk_hdr,
			      master_hdr, i) != MDV_SUCCESS)
    {
      fprintf(stderr,
	      "ERROR - %s:RfReadMDVVolume:load_chunk_info\n",
	      vindex->prog_name);
      fprintf(stderr, "Error reading chunk %d header from MDV file.\n",
	      i);
      return(MDV_FAILURE);
    }
    
    switch(chunk_hdr.chunk_id)
    {
    case MDV_CHUNK_DOBSON_VOL_PARAMS :
      if ((chunk_data = MDV_get_chunk_data(vindex->vol_file, &chunk_hdr))
	  == NULL)
      {
	fprintf(stderr, "ERROR - %s:RfReadMDVVolume:load_chunk_info\n",
		vindex->prog_name);
	fprintf(stderr,
		"Cannot read Dobson vol params chunk from MDV file.\n");
	return(MDV_FAILURE);
      }
      
      *(vindex->vol_params) = *((vol_params_t *)chunk_data);
      *vol_params_loaded_p = TRUE;

      ufree(chunk_data);
      break;
      
    case MDV_CHUNK_DOBSON_ELEVATIONS :
      if ((chunk_data = MDV_get_chunk_data(vindex->vol_file, &chunk_hdr))
	  == NULL)
      {
	fprintf(stderr, "ERROR - %s:RfReadMDVVolume:load_chunk_info\n",
		vindex->prog_name);
	fprintf(stderr,
		"Cannot read Dobson elevations chunk from MDV file.\n");
	return(MDV_FAILURE);
      }
      
      if (vindex->radar_elevations != NULL)
	free(vindex->radar_elevations);
      
      vindex->radar_elevations = (si32*) malloc(chunk_hdr.size);
      memcpy((char *)vindex->radar_elevations,
	     (char *)chunk_data,
	     chunk_hdr.size);
      
      ufree(chunk_data);
      break;
    } /* endswitch - chunk_id */
    
  } /* endfor - i */
  
  return (MDV_SUCCESS);

}

/*************************************************************************
 *
 * update_dobson_volume_params()
 *
 * updates a dobson volume parameters structure from the information in
 * an MDV master header.
 *
 **************************************************************************/

static void update_dobson_volume_params(MDV_master_header_t *master_hdr,
					MDV_field_header_t *field_hdr,
					vol_params_t *vol_params)
{
  strncpy(vol_params->note, "", VOL_PARAMS_NOTE_LEN);

  Rfutime2rtime(master_hdr->time_gen,
		&(vol_params->file_time));
  Rfutime2rtime(master_hdr->time_begin,
		&(vol_params->start_time));
  Rfutime2rtime(master_hdr->time_centroid,
		&(vol_params->mid_time));
  Rfutime2rtime(master_hdr->time_end,
		&(vol_params->end_time));

  update_dobson_radar_params(master_hdr, &(vol_params->radar));
  update_dobson_cart_params(master_hdr, field_hdr, &(vol_params->cart));
  
  vol_params->nfields = master_hdr->n_fields;
  
  return;
}


/*************************************************************************
 *
 * update_dobson_radar_params()
 *
 * updates a dobson radar parameters structure from the information in
 * an MDV master header.
 *
 **************************************************************************/

static void update_dobson_radar_params(MDV_master_header_t *master_hdr,
				       radar_params_t *radar_params)
{
  radar_params->nbytes_char = N_RADAR_PARAMS_LABELS * R_LABEL_LEN;
  radar_params->radar_id = 0; /* ??? */
  radar_params->altitude = (si32) master_hdr->sensor_alt;
  radar_params->latitude =
    (int)floor(master_hdr->sensor_lat * 1000000.0 + 0.5);
  radar_params->longitude =
    (int)floor(master_hdr->sensor_lon * 1000000.0 + 0.5);
  radar_params->nelevations = 0;
  radar_params->nazimuths = 0; /* ??? */
  radar_params->ngates = 0; /* ??? */
  radar_params->gate_spacing = 0; /* ??? */
  radar_params->start_range = 0; /* ??? */
  radar_params->delta_azimuth = 0; /* ??? */
  radar_params->start_azimuth = 0; /* ??? */
  radar_params->beam_width = 0; /* ??? */
  radar_params->samples_per_beam = 0; /* ??? */
  radar_params->pulse_width = 0; /* ??? */
  radar_params->prf = 0; /* ??? */
  radar_params->wavelength = 0; /* ???*/
  radar_params->nmissing = 0; /* ??? */
  radar_params->name[0] = '\0'; /* ??? */
  
  return;
}


/*************************************************************************
 *
 * update_dobson_cart_params()
 *
 * updates a dobson cart parameters structure from the information in
 * an MDV master header.
 *
 **************************************************************************/

static void update_dobson_cart_params(MDV_master_header_t *master_hdr,
				      MDV_field_header_t *field_hdr,
				      cart_params_t *cart_params)
{
  /*
   * Make sure we have a field header -- it contains a lot of the
   * necessary information.
   */

  if (field_hdr == NULL)
  {
    fprintf(stderr,
	    "update_dobson_cart_params:  No field header given\n");
    exit(-1);
  }
  
  cart_params->nbytes_char = N_CART_PARAMS_LABELS * R_LABEL_LEN;

  if (field_hdr == NULL)
  {
    cart_params->latitude =
      (int)floor(master_hdr->sensor_lat * 1000000 + 0.5);
    cart_params->longitude =
      (int)floor(master_hdr->sensor_lon * 1000000 + 0.5);
  }
  else
  {
    cart_params->latitude =
      (int)floor(field_hdr->proj_origin_lat * 1000000 + 0.5);
    cart_params->longitude =
      (int)floor(field_hdr->proj_origin_lon * 1000000 + 0.5);
  }
  
  cart_params->rotation = 0; /* ??? */
  
  cart_params->nx = master_hdr->max_nx;
  cart_params->ny = master_hdr->max_ny;
  cart_params->nz = master_hdr->max_nz;
  
  switch (field_hdr->proj_type)
  {
  case MDV_PROJ_FLAT :
  case MDV_PROJ_POLAR_RADAR :
  {
    double radarx, radary;
    
    cart_params->scalex = INT_SCALE;
    cart_params->scaley = INT_SCALE;
    cart_params->scalez = INT_SCALE;
    
    cart_params->km_scalex = INT_SCALE;
    cart_params->km_scaley = INT_SCALE;
    cart_params->km_scalez = INT_SCALE;
    
    cart_params->minx =
      (int)floor(field_hdr->grid_minx *
		 (double)cart_params->scalex + 0.5);
    cart_params->miny =
      (int)floor(field_hdr->grid_miny *
		 (double)cart_params->scaley + 0.5);
    cart_params->minz =
      (int)floor(field_hdr->grid_minz *
		 (double)cart_params->scalez + 0.5);
  
    cart_params->dx =
      (int)floor(field_hdr->grid_dx *
		 (double)cart_params->scalex + 0.5);
    cart_params->dy =
      (int)floor(field_hdr->grid_dy *
		 (double)cart_params->scaley + 0.5);
    cart_params->dz =
      (int)floor(field_hdr->grid_dz *
		 (double)cart_params->scalez + 0.5);
    
    PJGLatLon2DxDy(cart_params->latitude / 1000000.0,
		   cart_params->longitude / 1000000.0,
		   master_hdr->sensor_lat,
		   master_hdr->sensor_lon,
		   &radarx, &radary);

    cart_params->radarx =
      (int)floor(radarx * (double)cart_params->scalex + 0.5);
    cart_params->radary =
      (int)floor(radary * (double)cart_params->scaley + 0.5);
    cart_params->radarz =
      (int)floor(master_hdr->sensor_alt *
	    (double)cart_params->scalez + 0.5);
    
    strncpy(cart_params->unitsx, "km",
	    R_LABEL_LEN);
    cart_params->unitsx[R_LABEL_LEN-1] = '\0';
    strncpy(cart_params->unitsy, "km",
	    R_LABEL_LEN);
    cart_params->unitsy[R_LABEL_LEN-1] = '\0';
    if (master_hdr->vlevel_type == MDV_VERT_TYPE_ELEV ||
	master_hdr->vlevel_type == MDV_VERT_VARIABLE_ELEV ||
	master_hdr->vlevel_type == MDV_VERT_FIELDS_VAR_ELEV) {
      strncpy(cart_params->unitsz, "deg",
	      R_LABEL_LEN);
    } else {
      strncpy(cart_params->unitsz, "km",
	      R_LABEL_LEN);
    }
    cart_params->unitsz[R_LABEL_LEN-1] = '\0';
  
    break;
  } /* endcase - MDV_PROJ_FLAT */

  case MDV_PROJ_LATLON :
  {
    cart_params->scalex = 1000000;
    cart_params->scaley = 1000000;
    cart_params->scalez = INT_SCALE;
    
    cart_params->km_scalex = 10000;
    cart_params->km_scaley = 10000;
    cart_params->km_scalez = INT_SCALE;
    
    cart_params->minx =
      (int)floor(field_hdr->grid_minx *
	    (double)cart_params->scalex + 0.5);
    cart_params->miny =
      (int)floor(field_hdr->grid_miny *
	    (double)cart_params->scaley + 0.5);
    cart_params->minz =
      (int)floor(field_hdr->grid_minz *
	    (double)cart_params->scalez + 0.5);
  
    cart_params->dx =
      (int)floor(field_hdr->grid_dx *
		 (double)cart_params->scalex + 0.5);
    cart_params->dy =
      (int)floor(field_hdr->grid_dy *
		 (double)cart_params->scaley + 0.5);
    cart_params->dz =
      (int)floor(field_hdr->grid_dz *
		 (double)cart_params->scalez + 0.5);
    
    cart_params->radarx =
      (int)floor(master_hdr->sensor_lon *
		 (double)cart_params->scalex + 0.5);
    cart_params->radary =
      (int)floor(master_hdr->sensor_lat *
		 (double)cart_params->scaley + 0.5);
    cart_params->radarz =
      (int)floor(master_hdr->sensor_alt *
		 (double)cart_params->scalez + 0.5);
    
    strncpy(cart_params->unitsx, "lon",
	    R_LABEL_LEN);
    cart_params->unitsx[R_LABEL_LEN-1] = '\0';
    strncpy(cart_params->unitsy, "lat",
	    R_LABEL_LEN);
    cart_params->unitsy[R_LABEL_LEN-1] = '\0';
    if (master_hdr->vlevel_type == MDV_VERT_TYPE_ELEV ||
	master_hdr->vlevel_type == MDV_VERT_VARIABLE_ELEV ||
	master_hdr->vlevel_type == MDV_VERT_FIELDS_VAR_ELEV) {
      strncpy(cart_params->unitsz, "deg",
	      R_LABEL_LEN);
    } else {
      strncpy(cart_params->unitsz, "km",
	      R_LABEL_LEN);
    }
    cart_params->unitsz[R_LABEL_LEN-1] = '\0';
  
    break;
  } /* endcase - MDV_PROJ_LATLON */

  default:
    fprintf(stderr,
	    "update_dobson_cart_params:  Unknown proj_type %d in MDV file.\n",
	    field_hdr->proj_type);
    exit(-1);
  } /* endswitch - proj_type */
  
  if (master_hdr->vlevel_included)
    cart_params->dz_constant = 0;
  else
    cart_params->dz_constant = 1;
  
  return;
}


/*************************************************************************
 *
 * update_dobson_field_params()
 *
 * updates a dobson field parameters structure from the information in
 * an MDV dataset.
 *
 **************************************************************************/

static void update_dobson_field_params(MDV_field_header_t *field_hdr,
				       field_params_t *field_params,
				       char *prog_name,
				       char *calling_routine)
{
  static char *this_routine = "update_dobson_field_params";
  
  char calling_sequence[MAX_SEQ];
  
  /*
   * Set up calling sequence.
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, this_routine);
  
  field_params->nbytes_char = N_FIELD_PARAMS_LABELS * R_LABEL_LEN;
  switch (field_hdr->encoding_type)
  {
  case MDV_NATIVE :
  case MDV_INT8 :
  case MDV_INT16 :
  case MDV_FLOAT32 :
    field_params->encoded = FALSE;
    break;
    
  case MDV_PLANE_RLE8 :
    field_params->encoded = TRUE;
    break;
    
  default:
    fprintf(stderr, "ERROR - %s:%s\n",
	    prog_name, calling_sequence);
    fprintf(stderr,
	    "Unknown encoding_type %d in MDV file.\n",
	    field_hdr->encoding_type);
    fprintf(stderr,
	    "Setting Dobson encoded flag to FALSE.\n");
    
    field_params->encoded = FALSE;
    break;
  } /* endswitch - encoding_type */
  
  field_params->factor = FPARAMS_INT_SCALE;
  field_params->scale = (int)floor(field_hdr->scale *
				   (double)field_params->factor + 0.5);
  field_params->bias = (int)floor(field_hdr->bias *
				  (double)field_params->factor + 0.5);
  field_params->missing_val = (si32) field_hdr->missing_data_value;

  field_params->noise = 0; /* ??? */
  strncpy(field_params->transform, field_hdr->transform,
	  R_LABEL_LEN);
  field_params->transform[R_LABEL_LEN-1] = '\0';
  strncpy(field_params->name, field_hdr->field_name,
	  R_LABEL_LEN);
  field_params->name[R_LABEL_LEN-1] = '\0';
  strncpy(field_params->units, field_hdr->units,
	  R_LABEL_LEN);
  field_params->units[R_LABEL_LEN-1] = '\0';
  
  return;
}
