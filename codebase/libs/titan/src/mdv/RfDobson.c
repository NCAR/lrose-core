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
 * RfDobson.c
 *
 * part of the rfutil library - radar file access
 *
 * Dobson file access routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * October 1991
 *
 **************************************************************************/

#include <toolsa/compress.h>
#include <titan/radar.h>
#include <titan/file_io.h>
#include <rapformats/titan_grid.h>
#include <titan/mdv.h>
#include <dataport/bigend.h>
#include <toolsa/pjg.h>
#include <toolsa/toolsa_macros.h>

#define MAX_SEQ 256

/*
 * time manipulation - to and from date_time structs
 */

void Rfdtime2rtime(date_time_t *dtime,
		   radtim_t *rtime)

{
  rtime->year = dtime->year;
  rtime->month = dtime->month;
  rtime->day = dtime->day;
  rtime->hour = dtime->hour;
  rtime->min = dtime->min;
  rtime->sec = dtime->sec;
}

void Rfrtime2dtime(radtim_t *rtime,
		   date_time_t *dtime)
{
  dtime->year = rtime->year;
  dtime->month = rtime->month;
  dtime->day = rtime->day;
  dtime->hour = rtime->hour;
  dtime->min = rtime->min;
  dtime->sec = rtime->sec;
  uconvert_to_utime(dtime);
}

si32 Rfrtime2utime(radtim_t *rtime)

{
  date_time_t dtime;

  Rfrtime2dtime(rtime, &dtime);
  return (dtime.unix_time);
}

void Rfutime2rtime(time_t unix_time,
		   radtim_t *rtime)
{
  date_time_t dtime;
  dtime.unix_time = unix_time;
  uconvert_from_utime(&dtime);
  rtime->year = dtime.year;
  rtime->month = dtime.month;
  rtime->day = dtime.day;
  rtime->hour = dtime.hour;
  rtime->min = dtime.min;
  rtime->sec = dtime.sec;
}

/***********************
 * RfInitVolFileHandle()
 *
 * Initialize the vol file handle
 */

void RfInitVolFileHandle(vol_file_handle_t *handle,
			 const char *prog_name,
			 const char *file_name,
			 FILE *fd)
     
{

  /*
   * set fields in handle
   */

  memset (handle, 0, sizeof(vol_file_handle_t));

  handle->prog_name = (char *) umalloc (strlen(prog_name) + 1);
  strcpy(handle->prog_name, prog_name);
  
  if (file_name != NULL) {
    handle->vol_file_path = (char *) umalloc (strlen(file_name) + 1);
    strcpy(handle->vol_file_path, file_name);
  }

  handle->vol_file_label = (char *) umalloc (R_FILE_LABEL_LEN);
  memset (handle->vol_file_label, 0, R_FILE_LABEL_LEN);
  strcpy(handle->vol_file_label, RADAR_VOLUME_FILE_TYPE);
  
  handle->vol_file = fd;
  handle->handle_initialized = TRUE;

}

/***********************
 * RfFreeVolFileHandle()
 *
 * Free the vol file handle
 */

void RfFreeVolFileHandle(vol_file_handle_t *handle)

{

  RfFreeVolArrays(handle, "RfFreeVolFileHandle");
  RfFreeVolParams(handle, "RfFreeVolFileHandle");
  RfFreeVolPlanes(handle, "RfFreeVolFileHandle");

  if (handle->handle_initialized) {
    ufree(handle->prog_name);
    if (handle->vol_file_path != NULL) {
      ufree(handle->vol_file_path);
      handle->vol_file_path = NULL;
    }
    if (handle->vol_file_label != NULL) {
      ufree(handle->vol_file_label);
      handle->vol_file_label = NULL;
    }
    handle->vol_file = (FILE *) NULL;
    handle->handle_initialized = FALSE;
  }

}

/*************************************************************************
 *
 * RfAllocVolArrays()
 *
 * part of the rfutil library - radar file access
 *
 * allocates space for the arrays associated with the
 * vol_file_handle_t struct
 *
 **************************************************************************/

#define THIS_ROUTINE "RfAllocVolArrays"

int RfAllocVolArrays(vol_file_handle_t *v_handle, const char *calling_routine)
     
{

  char calling_sequence[MAX_SEQ];

  si32 ifield;
  si32 nelevations;
  si32 nplanes;
  si32 nfields;

  nelevations = v_handle->vol_params->radar.nelevations;
  nplanes = v_handle->vol_params->cart.nz;
  nfields = v_handle->vol_params->nfields;

  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  if (!v_handle->arrays_allocated) {

    /*
     * initial allocation
     */

    v_handle->radar_elevations = (si32 *) umalloc
      ((ui32) (nelevations * sizeof(si32)));

    v_handle->plane_heights = (si32 **) ucalloc2
      ((ui32) nplanes,
       (ui32) N_PLANE_HEIGHT_VALUES,
       (ui32) sizeof(si32));

    v_handle->plane_offset = (si32 **) ucalloc2
      ((ui32) nfields, (ui32) nplanes,
       (ui32) sizeof(si32));

    v_handle->plane_allocated = (si32 **) ucalloc2
      ((ui32) nfields, (ui32) nplanes,
       (ui32) sizeof(si32));
    
    v_handle->field_params_offset = (si32 *) umalloc
      ((ui32) (nfields * sizeof(si32)));

    v_handle->field_params = (field_params_t **) ucalloc2
      ((ui32) nfields, (ui32) 1,
       (ui32) sizeof(field_params_t));
    
    v_handle->field_plane = (ui08 ***) umalloc
      ((ui32) (nfields * sizeof(ui08 **)));
    
    for (ifield = 0; ifield < nfields; ifield++)
      v_handle->field_plane[ifield] = (ui08 **) umalloc
	((ui32) (nplanes * sizeof(ui08 *)));
    
    v_handle->nelevations_allocated = nelevations;
    v_handle->nplanes_allocated = nplanes;
    v_handle->nfields_allocated = nfields;

    v_handle->arrays_allocated = TRUE;

  } else {

    /*
     * reallocation as necessary
     */

    if (nelevations != v_handle->nelevations_allocated) {

      v_handle->radar_elevations = (si32 *) urealloc
	((char *) v_handle->radar_elevations,
	 (ui32) (nelevations * sizeof(si32)));

    }

    if (nplanes != v_handle->nplanes_allocated) {

      ufree2((void **) v_handle->plane_heights);

      v_handle->plane_heights = (si32 **) ucalloc2
	((ui32) nplanes,
	 (ui32) N_PLANE_HEIGHT_VALUES,
	 (ui32) sizeof(si32));

    }

    if (nfields != v_handle->nfields_allocated) {

      v_handle->field_params_offset = (si32 *) urealloc
	((char *) v_handle->field_params_offset,
	 (ui32) (nfields * sizeof(si32)));

      ufree2((void **) v_handle->field_params);

      v_handle->field_params = (field_params_t **) ucalloc2
	((ui32) nfields, (ui32) 1,
	 (ui32) sizeof(field_params_t));

    }

    if (nplanes != v_handle->nplanes_allocated ||
	nfields != v_handle->nfields_allocated) {

      if (RfFreeVolPlanes(v_handle, calling_sequence))
	return(R_FAILURE);

      ufree2((void **) v_handle->plane_offset);
      ufree2((void **) v_handle->plane_allocated);
      
      v_handle->plane_offset = (si32 **) ucalloc2
	((ui32) nfields, (ui32) nplanes,
	 (ui32) sizeof(si32));

      v_handle->plane_allocated = (si32 **) ucalloc2
	((ui32) nfields, (ui32) nplanes,
	 (ui32) sizeof(si32));

      for (ifield = 0;
	   ifield < v_handle->nfields_allocated; ifield++)
	ufree(v_handle->field_plane[ifield]);

      v_handle->field_plane = (ui08 ***) urealloc
	((char *) v_handle->field_plane,
	 (ui32) (nfields * sizeof(ui08 **)));
      
      for (ifield = 0; ifield < nfields; ifield++)
	v_handle->field_plane[ifield] = (ui08 **) umalloc
	  ((ui32) (nplanes * sizeof(ui08 *)));
    
    }
    
    v_handle->nelevations_allocated = nelevations;
    v_handle->nplanes_allocated = nplanes;
    v_handle->nfields_allocated = nfields;

  } /* if (!v_handle->arrays_allocated) */

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfAllocVolParams()
 *
 * part of the rfutil library - radar file access
 *
 * allocates space for the vol_params_t structure
 *
 **************************************************************************/

/*ARGSUSED*/

int RfAllocVolParams(vol_file_handle_t *v_handle, const char *calling_routine)
     
{

  if (!v_handle->params_allocated) {

    v_handle->vol_params = (vol_params_t *)
      umalloc ((ui32) sizeof(vol_params_t));

    v_handle->params_allocated = TRUE;

  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfCartParams2TITANGrid()
 *
 * Copies a cart_params_t struct to titan_grid_t struct
 *
 **************************************************************************/

void RfCartParams2TITANGrid(cart_params_t *cart,
			  titan_grid_t *grid,
			  si32 grid_type)
     
{

  double range, azimuth;
  double lat, lon;

  memset((void *) grid, 0, sizeof(titan_grid_t));
  
  grid->nbytes_char = TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN;
  grid->proj_type = grid_type;
  grid->proj_origin_lat = (double) cart->latitude / DEG2LONG;
  grid->proj_origin_lon = (double) cart->longitude / DEG2LONG;
  grid->proj_params.flat.rotation = (double) cart->rotation / DEG2LONG;
  grid->nx = cart->nx;
  grid->ny = cart->ny;
  grid->nz = cart->nz;
  grid->minx = (double) cart->minx / (double) cart->scalex;
  grid->miny = (double) cart->miny / (double) cart->scaley;
  grid->minz = (double) cart->minz / (double) cart->scalez;
  grid->dx = (double) cart->dx / (double) cart->scalex;
  grid->dy = (double) cart->dy / (double) cart->scaley;
  grid->dz = (double) cart->dz / (double) cart->scalez;
  grid->sensor_x = (double) cart->radarx / (double) cart->scalex;
  grid->sensor_y = (double) cart->radary / (double) cart->scaley;
  grid->sensor_z = (double) cart->radarz / (double) cart->scalez;
  grid->dz_constant = cart->dz_constant;
  
  if (grid_type == TITAN_PROJ_LATLON) {
    
    grid->sensor_lat = grid->sensor_y;
    grid->sensor_lon = grid->sensor_x;
    
  } else if (grid_type == TITAN_PROJ_FLAT) {
    
    if (grid->sensor_x == 0.0 && grid->sensor_y == 0.0) {
      
      grid->sensor_lat = grid->proj_origin_lat;
      grid->sensor_lon = grid->proj_origin_lon;
      
    } else {
      
      range = sqrt(grid->sensor_x * grid->sensor_x +
		   grid->sensor_y * grid->sensor_y);
      
      azimuth = (atan2(grid->sensor_x, grid->sensor_y) +
		 grid->proj_params.flat.rotation * DEG_TO_RAD);

      PJGLatLonPlusRTheta(grid->proj_origin_lat, grid->proj_origin_lon,
			  range, azimuth,
			  &lat, &lon);
      
      grid->sensor_lat = lat;
      grid->sensor_lon = lon;
      
    } /* if (grid->sensor_x == 0.0 ...*/

  } else {

    grid->sensor_lat = 0.0;
    grid->sensor_lon = 0.0;
    
  } /* if (grid_type == TITAN_PROJ_LATLON) */
      
  strncpy(grid->unitsx, cart->unitsx, TITAN_GRID_UNITS_LEN);
  strncpy(grid->unitsy, cart->unitsy, TITAN_GRID_UNITS_LEN);
  strncpy(grid->unitsz, cart->unitsz, TITAN_GRID_UNITS_LEN);

  return;

}

/*************************************************************************
 *
 * RfDecodeCartParams()
 *
 * Decodes cart params into floating point
 *
 **************************************************************************/

void RfDecodeCartParams(cart_params_t *cart,
			cart_float_params_t *fl_cart)
     
{

  fl_cart->latitude = (double) cart->latitude / 1000000.0;
  fl_cart->longitude = (double) cart->longitude / 1000000.0;
  fl_cart->rotation = (double) cart->rotation / 1000000.0;

  fl_cart->nx = cart->nx;
  fl_cart->ny = cart->ny;
  fl_cart->nz = cart->nz;

  fl_cart->minx = (double) cart->minx / (double) cart->scalex;
  fl_cart->miny = (double) cart->miny / (double) cart->scaley;
  fl_cart->minz = (double) cart->minz / (double) cart->scalez;

  fl_cart->dx = (double) cart->dx / (double) cart->scalex;
  fl_cart->dy = (double) cart->dy / (double) cart->scaley;
  fl_cart->dz = (double) cart->dz / (double) cart->scalez;

  fl_cart->radarx = (double) cart->radarx / (double) cart->scalex;
  fl_cart->radary = (double) cart->radary / (double) cart->scaley;
  fl_cart->radarz = (double) cart->radarz / (double) cart->scalez;

  return;

}

/*************************************************************************
 *
 * RfFreeVolArrays()
 *
 * part of the rfutil library - radar file access
 *
 * Frees space for the arrays associated with the
 * vol_file_handle_t struct
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeVolArrays(vol_file_handle_t *v_handle, const char *calling_routine)
{

  si32 ifield;

  if (v_handle->arrays_allocated) {

    RfFreeVolPlanes(v_handle, "RfFreeVolArrays");

    ufree(v_handle->radar_elevations);
    ufree2((void **) v_handle->plane_heights);
    ufree2((void **) v_handle->plane_offset);
    ufree(v_handle->field_params_offset);
    ufree2((void **) v_handle->field_params);
    
    for (ifield = 0;
	 ifield < v_handle->nfields_allocated; ifield++)
      ufree(v_handle->field_plane[ifield]);
      
    ufree(v_handle->field_plane);
    
    v_handle->nelevations_allocated = 0;
    v_handle->nplanes_allocated = 0;
    v_handle->nfields_allocated = 0;

    v_handle->arrays_allocated = FALSE;

  } /* if (v_handle->arrays_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeVolParams()
 *
 * part of the rfutil library - radar file access
 *
 * frees space for the vol_params_t structure
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeVolParams(vol_file_handle_t *v_handle, const char *calling_routine)
{

  if (v_handle->params_allocated) {

    ufree ((char *) v_handle->vol_params);
    v_handle->params_allocated = FALSE;

  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeVolPlanes()
 *
 * part of the rfutil library - radar file access
 *
 * Frees space for the field plane arrays
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeVolPlanes(vol_file_handle_t *v_handle, const char *calling_routine)
{

  si32 ifield, iplane;

  if (v_handle->arrays_allocated) {

    for (ifield = 0;
	 ifield < v_handle->nfields_allocated; ifield++) {
      
      for (iplane = 0; iplane < v_handle->nplanes_allocated; iplane++) {

	if (v_handle->plane_allocated[ifield][iplane]) {
	  ufree ((char *) v_handle->field_plane[ifield][iplane]);
	  v_handle->plane_allocated[ifield][iplane] = FALSE;
	}

      } /* iplane */

    } /* ifield */
      
  } /* if (v_handle->arrays_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * RfReadVolFparams()
 *
 * part of the rfutil library - radar file access
 *
 * reads in the field params from a volume scan file
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfReadVolFparams(vol_file_handle_t *v_handle, si32 field_num,
		     const char *calling_routine)
     
{
  
  si32 field_offset;
  si32 nbytes_char;
  si32 nplanes;
  field_params_t *fparams;
  
  fparams = v_handle->field_params[field_num];
  nplanes = v_handle->vol_params->cart.nz;
  
  /*
   * check that field number is valid
   */
  
  if (field_num > v_handle->vol_params->nfields - 1) {
    fprintf(stderr, "ERROR - %s:%s:RfReadVolFparams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr,
	    "Requested field number %d exceeds the %d fields available.\n",
	    field_num + 1, v_handle->vol_params->nfields);
    return (R_FAILURE);
  }
  
  /*
   * seek to field params structure
   */
  
  field_offset = v_handle->field_params_offset[field_num];
  fseek(v_handle->vol_file, field_offset, SEEK_SET);
  
  /*
   * read in field params
   */
  
  if (ufread((char *) fparams,
	     (int) sizeof(field_params_t),
	     1, v_handle->vol_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadVolFparams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading field params, field %ld.\n", (long) field_num);
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode si32s in field params - nbytes_char is the number of bytes
   * of character data at the end of the struct
   */
  
  memcpy ((void *) &nbytes_char,
          (void *) &fparams->nbytes_char,
          (size_t) sizeof(si32));
  
  BE_to_array_32((ui32 *) &nbytes_char,
	    (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) fparams,
	    (ui32) (sizeof(field_params_t) - nbytes_char));
  
  /*
   * read in plane offset array
   */
  
  if (ufread((char *) v_handle->plane_offset[field_num],
	     sizeof(si32), (int) nplanes,
	     v_handle->vol_file) != nplanes) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadVolFparams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading data plane offset array.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) v_handle->plane_offset[field_num],
	    (ui32) (nplanes * sizeof(si32)));
  
  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfReadVolParams()
 *
 * part of the rfutil library - radar file access
 *
 * reads in the VOL_PARAMS structure from a volume scan file
 *
 * If heights_in_km is set to TRUE, the plane heights will
 * be in km. Otherwise they will be in the grid units.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadVolParams"

int RfReadVolParams(vol_file_handle_t *v_handle, const char *calling_routine)
{
  
  char calling_sequence[MAX_SEQ];

  char file_label[R_FILE_LABEL_LEN];
  
  si32 nbytes_char;
  si32 nelevations;
  si32 nfields;
  si32 nplanes;
  si32 nheights;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * allocate space for vol params if needed
   */
  
  if (!v_handle->params_allocated)
    if (RfAllocVolParams(v_handle, calling_sequence))
      return(R_FAILURE);
  
  /*
   * rewind file
   */
  
  fseek(v_handle->vol_file, 0L, SEEK_SET);
  
  /*
   * read in file label
   */
  
  if (ufread(file_label, (int) sizeof(char),
	     (int) R_FILE_LABEL_LEN,
	     v_handle->vol_file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading file label.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * check that this is a radar volume scan file
   */
  
  if (strcmp(file_label, v_handle->vol_file_label)) {
    fprintf(stderr, "ERROR - %s:%s:RfReadVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "File label does not match requested label.\n");
    fprintf(stderr, "File label is '%s'\n", file_label);
    fprintf(stderr, "Requested label is '%s'\n",
	    v_handle->vol_file_label);
    return (R_FAILURE);
  }
  
  /*
   * read in vol params
   */
  
  if (ufread((char *) v_handle->vol_params,
	     (int) sizeof(vol_params_t),
	     1, v_handle->vol_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading vol_params_t structure.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  
  /*
   * decode the parts of the structure which contain longs - 
   * the file contains longs which are coded with the
   * most-significant byte first. This may need changing to
   * match the machine byte-ordering.
   */
  
  BE_to_array_32((ui32 *) &v_handle->vol_params->file_time,
	    (ui32) (N_VOL_PARAMS_TIMES * sizeof(radtim_t)));
  
  nbytes_char = v_handle->vol_params->radar.nbytes_char;
  
  BE_to_array_32((ui32 *) &nbytes_char,
	    (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) &v_handle->vol_params->radar,
	    (ui32) (sizeof(radar_params_t) - nbytes_char));
  
  memcpy ((void *) &nbytes_char,
          (void *) &v_handle->vol_params->cart.nbytes_char,
          (size_t) sizeof(si32));
  
  BE_to_array_32((ui32 *) &nbytes_char,
	    (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) &v_handle->vol_params->cart,
	    (ui32) (sizeof(cart_params_t) - nbytes_char));
  
  BE_to_array_32((ui32 *) &v_handle->vol_params->nfields,
	    (ui32) sizeof(si32));
  
  /*
   * set local variables
   */
  
  nelevations = v_handle->vol_params->radar.nelevations;
  nplanes = v_handle->vol_params->cart.nz;
  nfields = v_handle->vol_params->nfields;
  nheights = N_PLANE_HEIGHT_VALUES * nplanes;
  
  /*
   * allocate space for arrays as required
   */
  
  if (RfAllocVolArrays(v_handle, calling_sequence))
    return (R_FAILURE);
  
  /*
   * read in radar_elevations array, decode from network
   * byte order
   */
  
  if (ufread((char *) v_handle->radar_elevations,
	     sizeof(si32), (int) nelevations,
	     v_handle->vol_file) != nelevations) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading radar elevation array.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) v_handle->radar_elevations,
	    (ui32) (nelevations * sizeof(si32)));
  
  /*
   * read in plane_heights array, decode from network
   * byte order
   */
  
  if (ufread((char *) *v_handle->plane_heights,
	     sizeof(si32),
	     (int) nheights,
	     v_handle->vol_file) != nheights) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading plane limits array.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) *v_handle->plane_heights,
	    (ui32) (nheights * sizeof(si32)));
  
  /*
   * read in field params offset array and decode from
   * network byte order
   */
  
  if (ufread((char *) v_handle->field_params_offset,
	     sizeof(si32), (int) nfields,
	     v_handle->vol_file) != nfields) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading field params offset array.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) v_handle->field_params_offset,
	    (ui32) (nfields * sizeof(si32)));
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadVolPlane()
 *
 * part of the rfutil library - radar file access
 *
 * reads in the data for a given field and plane -
 * allocates space for the data array
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfReadVolPlane(vol_file_handle_t *v_handle, si32 field_num,
		   si32 plane_num, const char *calling_routine)
     
{

  int eight_bit;

  ui32 nbytes_array;
  ui32 nbytes_per_plane;
  ui32 nbytes_full;
  
  ui08 *cdata, *fdata;
  
  ui08 leading_bytes[RL8_NBYTES_EXTRA];
  
  field_params_t *fparams;
  
  /*
   * set field params pointer
   */
  
  fparams = v_handle->field_params[field_num];
  
  /*
   * set number of bytes per plane
   */
  
  nbytes_per_plane = (v_handle->vol_params->cart.nx *
		      v_handle->vol_params->cart.ny);
  
  /*
   * move to plane data offset
   */
  
  fseek(v_handle->vol_file,
	v_handle->plane_offset[field_num][plane_num],
	SEEK_SET);
  
  /*
   * read in data - it may be runlength encoded, and this is checked
   * before reading. Space is allocated for the data array
   */
  
  if (fparams->encoded) {
    
    /*
     * read in leading bytes
     */
    
    if (ufread((char *) leading_bytes, (int) sizeof(ui08),
	       RL8_NBYTES_EXTRA, v_handle->vol_file) != RL8_NBYTES_EXTRA) {
      
      fprintf(stderr, "ERROR - %s:%s:RfReadVolPlane\n",
	      v_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "Reading leaading bytes in coded array, field %ld, plane %ld.\n",
	      (long) field_num, (long) plane_num);
      perror(v_handle->vol_file_path);
      return (R_FAILURE);
      
    }
    
    /*
     * check on type of encoding
     */

    if (uRLCheck(leading_bytes,
		 (ui32) RL8_NBYTES_EXTRA,
		 &eight_bit,
		 &nbytes_array))
      return (R_FAILURE);

    /*
     * seek back
     */
    
    fseek(v_handle->vol_file, -((si32) RL8_NBYTES_EXTRA), SEEK_CUR);
    
    /*
     * allocate memory for coded data array
     */
    
    cdata = (ui08 *) umalloc
      ((ui32) (nbytes_array * sizeof(ui08)));
    
    /*
     * read in coded data
     */
    
    if (ufread((char *) cdata, (int) sizeof(ui08),
	       (int) nbytes_array,
	       v_handle->vol_file) != (int) nbytes_array) {
      
      fprintf(stderr, "ERROR - %s:%s:RfReadVolPlane\n",
	      v_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "Reading encoded data array, field %ld, plane %ld.\n",
	      (long) field_num, (long) plane_num);
      perror(v_handle->vol_file_path);
      return (R_FAILURE);
      
    }
    
    /*
     * decode the leading bytes which are longs, and unpack the data
     */

    if (eight_bit) {
      fdata = uRLDecode8(cdata, &nbytes_full);
    } else {
      fdata = uRLDecode(cdata, &nbytes_full);
    }
    
    /*
     * check that there are the correct number of bytes
     */
    
    if (nbytes_full != nbytes_per_plane) {
      fprintf(stderr, "ERROR - %s:%s:RfReadVolPlane\n",
	      v_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "Reading encoded data array, field %ld, plane %ld.\n",
	      (long) field_num, (long) plane_num);
      fprintf(stderr,
	      "Uncompressed to %ld bytes, should be %ld bytes.\n",
	      (long) nbytes_full, (long) nbytes_per_plane);
      return (R_FAILURE);
    }
    
    /*
     * free up coded array
     */
    
    ufree(cdata);
    
  } else {
    
    /*
     * non-encoded file
     */
    
    /*
     * allocate memory for field data array
     */
    
    fdata = (ui08 *)
      umalloc ((ui32) (nbytes_per_plane * sizeof(ui08)));
    
    /*
     * read in data
     */
    
    if (ufread((char *) fdata, (int) sizeof(ui08),
	       (int) nbytes_per_plane,
	       v_handle->vol_file) != (int) nbytes_per_plane) {
      
      fprintf(stderr, "ERROR - %s:%s:RfReadVolPlane\n",
	      v_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "Reading full data array, field %ld, plane %ld.\n",
	      (long) field_num, (long) plane_num);
      perror(v_handle->vol_file_path);
      return (R_FAILURE);
      
    }
    
  } /* if (fparams->encoded) */
  
  /*
   * if this plane has been previously allocated, free it
   */
  
  if (v_handle->plane_allocated[field_num][plane_num])
    ufree(v_handle->field_plane[field_num][plane_num]);
  
  /*
   * set field_plane pointer
   */
  
  v_handle->field_plane[field_num][plane_num] = fdata;
  
  /*
   * set flag to indicate that this plane has been allocated
   */
  
  v_handle->plane_allocated[field_num][plane_num] = TRUE;
  
  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfReadVolume.c
 *
 * part of the rfutil library - radar file access
 *
 * reads a radar volume from file.  The radar file may be in either
 * Dobson or MDV format.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadVolume"

int RfReadVolume(vol_file_handle_t *v_handle, const char *calling_routine)
{
  char calling_sequence[MAX_SEQ];

  si32 file_record_len;
  si32 file_struct_id;
  
  int mdv_file;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * open the radar volume file, uncompressing if necessary
   */

  if ((v_handle->vol_file =
       Rf_fopen_uncompress(v_handle->vol_file_path, "r")) == NULL)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    v_handle->prog_name, calling_sequence);
    fprintf(stderr, "Cannot open radar volume file for reading.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
  }

  /*
   * Determine if this is a Dobson file or an MDV file.  We assume it
   * is an MDV file, read in the first two longs and check to see if
   * we have the MDV magic number.  If we do, treat it as an MDV file.
   * Otherwise, treat it as a Dobson file.
   */

  if (fread(&file_record_len, sizeof(si32), 1, v_handle->vol_file) != 1)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    v_handle->prog_name, calling_sequence);
    fprintf(stderr, "Cannot read first si32 value from radar volume file.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
  }
  
  if (fread(&file_struct_id, sizeof(si32), 1, v_handle->vol_file) != 1)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    v_handle->prog_name, calling_sequence);
    fprintf(stderr, "Cannot read second si32 value from radar volume file.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
  }
  
  if (BE_to_si32(file_struct_id) == MDV_MASTER_HEAD_MAGIC_COOKIE ||
      file_struct_id == MDV_MASTER_HEAD_MAGIC_COOKIE)
  {
    mdv_file = TRUE;
  }
  else
  {
    mdv_file = FALSE;
  }
  
  /*
   * Close the input file since the reading routines open
   * it for themselves.
   */

  fclose(v_handle->vol_file);
    
  if (mdv_file)
  {
    /*
     * Read the Dobson file.
     */

    if (RfReadMDVVolume(v_handle, calling_sequence) != R_SUCCESS)
    {
      fprintf(stderr, "ERROR - %s:%s\n",
	      v_handle->prog_name, calling_sequence);
      fprintf(stderr, "Cannot read MDV file %s.\n",
	      v_handle->vol_file_path);
      return (R_FAILURE);
    } /* endif - read volume */
    
  } /* endif - MDV input file */
  else /* Dobson format */
  {
    /*
     * Set the Dobson file type so we can read the file.
     */
    
    if (v_handle->vol_file_label == NULL) {
      v_handle->vol_file_label = (char *) ucalloc (R_FILE_LABEL_LEN, 1);
    }
    strcpy(v_handle->vol_file_label, RADAR_VOLUME_FILE_TYPE);
    
    /*
     * Read the Dobson file.
     */

    if (RfReadDobsonVolume(v_handle, calling_sequence) != R_SUCCESS)
    {
      fprintf(stderr, "ERROR - %s:%s\n",
	      v_handle->prog_name, calling_sequence);
      fprintf(stderr, "Cannot read dobson file %s.\n",
	      v_handle->vol_file_path);
      return (R_FAILURE);
    } /* endif - read volume */
    
  } /* endelse - dobson input file */
  
  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadDobsonVolume()
 *
 * part of the rfutil library - radar file access
 *
 * reads a Dobson radar volume from file
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadDobsonVolume"

int RfReadDobsonVolume(vol_file_handle_t *v_handle, const char *calling_routine)
{

  char calling_sequence[MAX_SEQ];

  si32 nfields, ifield;
  si32 nplanes, iplane;

  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * open the radar volume file, uncompressing if necessary
   */

  if ((v_handle->vol_file =
       Rf_fopen_uncompress(v_handle->vol_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:%s:RfReadVolume\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Cannot open radar volume file for reading.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
  }

  /*
   * read in the vol params
   */

  if (RfReadVolParams(v_handle, calling_sequence)) {
    fclose(v_handle->vol_file);
    return (R_FAILURE);
  }
  
  nfields = v_handle->vol_params->nfields;
  nplanes = v_handle->vol_params->cart.nz;

  /*
   * read in the field params and planes
   */

  for (ifield = 0; ifield < nfields; ifield++) {

    if (RfReadVolFparams(v_handle, ifield, calling_sequence)) {
      fclose(v_handle->vol_file);
      return (R_FAILURE);
    }

    for (iplane = 0; iplane < nplanes; iplane++) {

      if (RfReadVolPlane(v_handle, ifield, iplane, calling_sequence)) {
	fclose(v_handle->vol_file);
	return (R_FAILURE);
      }
      
    } /* iplane */

  } /* ifield */

  /*
   * close file
   */

  fclose(v_handle->vol_file);

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfSeekEndVolHeader.c
 *
 * part of the rfutil library - radar file access
 *
 * seeks to the end of the cartesian volume file header
 *
 **************************************************************************/

int RfSeekEndVolHeader(vol_file_handle_t *v_handle, const char *calling_routine)
{

  if (fseek(v_handle->vol_file,
	    (si32) (R_FILE_LABEL_LEN +
		    sizeof(vol_params_t) +
		    v_handle->vol_params->radar.nelevations * sizeof(si32) +
		    (N_PLANE_HEIGHT_VALUES *
		     v_handle->vol_params->cart.nz * sizeof(si32)) +
		    v_handle->vol_params->nfields * sizeof(si32)),
	    SEEK_SET) != 0) {

    fprintf(stderr, "ERROR - %s:%s:RfSeekEndVolHeader\n",
	    v_handle->prog_name, calling_routine);
    
    fprintf(stderr, "Failed on seek.\n");

    perror(v_handle->vol_file_path);
    
    return (R_FAILURE);

  } else {

    return (R_SUCCESS);

  }

}

/*************************************************************************
 *
 * RfWriteVolFparams.c
 *
 * part of the rfutil library - radar file access
 *
 * writes field params to a radar volume file
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfWriteVolFparams(vol_file_handle_t *v_handle, si32 field_num,
		      const char *calling_routine)
     
{
  
  si32 nplanes;
  si32 *plane_offset;
  field_params_t tmp_field_params, *fparams;
  
  nplanes = v_handle->vol_params->cart.nz;
  fparams = v_handle->field_params[field_num];
  
  /*
   * allocate local array
   */
  
  plane_offset = (si32 *) umalloc
    ((ui32) (nplanes * sizeof(si32)));
  
  /*
   * set field params offset
   */
  
  v_handle->field_params_offset[field_num] =
    ftell(v_handle->vol_file);
  
  /*
   * set number of character bytes in field params
   */
  
  fparams->nbytes_char = N_FIELD_PARAMS_LABELS * R_LABEL_LEN;
  
  /*
   * make local copy of field params, encode and
   * write to file
   */
  
  memcpy ((void *) &tmp_field_params,
          (void *) fparams,
          (size_t) sizeof(field_params_t));
  
  ustr_clear_to_end(tmp_field_params.transform, R_LABEL_LEN);
  ustr_clear_to_end(tmp_field_params.name, R_LABEL_LEN);
  ustr_clear_to_end(tmp_field_params.units, R_LABEL_LEN);

  BE_from_array_32((ui32 *) &tmp_field_params,
	      (ui32) (sizeof(field_params_t) -
		      fparams->nbytes_char));
  
  if (ufwrite((char *) &tmp_field_params,
	      sizeof(field_params_t), 1,
	      v_handle->vol_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteVolFparams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing field params, field %ld.\n", (long) field_num);
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * make copy of plane offset array and code into network byte order
   */
  
  memcpy ((void *) plane_offset,
          (void *) v_handle->plane_offset[field_num],
          (size_t) (nplanes * sizeof(si32)));
  
  BE_from_array_32((ui32 *) plane_offset,
	      (ui32) (nplanes * sizeof(si32)));
  
  /*
   * write to file
   */
  
  if (ufwrite((char *) plane_offset,
	      sizeof(si32),
	      (int) nplanes, v_handle->vol_file) != nplanes) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteVolFparams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing plane offset array, field %ld.\n",
	    (long) field_num);
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  ufree(plane_offset);
  
  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfWriteVolParams.c
 *
 * part of the rfutil library - radar file access
 *
 * writes vol params to a radar volume file
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfWriteVolParams(vol_file_handle_t *v_handle, const char *calling_routine)
{
  
  si32 nelevations;
  si32 nfields;
  si32 nplanes;
  si32 nheights;
  
  si32 *radar_elevations;
  si32 *plane_heights;
  si32 *field_params_offset;
  
  vol_params_t tmp_vol_params;
  
  /*
   * set constants
   */
  
  nelevations = v_handle->vol_params->radar.nelevations;
  nfields = v_handle->vol_params->nfields;
  nplanes = v_handle->vol_params->cart.nz;
  nheights = nplanes * N_PLANE_HEIGHT_VALUES;
  
  /*
   * allocate local arrays
   */
  
  radar_elevations = (si32 *) umalloc
    ((ui32) (nelevations * sizeof(si32)));
  
  plane_heights = (si32 *) umalloc
    ((ui32) (nheights * sizeof(si32)));
  
  field_params_offset = (si32 *) umalloc
    ((ui32) (nfields * sizeof(si32)));
  
  /*
   * seek to start of file
   */
  
  fseek(v_handle->vol_file, (si32) 0, SEEK_SET);
  
  /*
   * write file label
   */
  
  if (ufwrite(v_handle->vol_file_label,
	      sizeof(char),
	      R_FILE_LABEL_LEN,
	      v_handle->vol_file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing volume file label.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * make a local copy of the vol params, encode and write to file
   */
  
  v_handle->vol_params->radar.nbytes_char =
    N_RADAR_PARAMS_LABELS * R_LABEL_LEN;
  
  v_handle->vol_params->cart.nbytes_char =
    N_CART_PARAMS_LABELS * R_LABEL_LEN;
  
  memcpy ((void *) &tmp_vol_params,
          (void *) v_handle->vol_params,
          (size_t) sizeof(vol_params_t));
  
  ustr_clear_to_end(tmp_vol_params.note, VOL_PARAMS_NOTE_LEN);
  ustr_clear_to_end(tmp_vol_params.radar.name, R_LABEL_LEN);
  ustr_clear_to_end(tmp_vol_params.cart.unitsx, R_LABEL_LEN);
  ustr_clear_to_end(tmp_vol_params.cart.unitsy, R_LABEL_LEN);
  ustr_clear_to_end(tmp_vol_params.cart.unitsz, R_LABEL_LEN);

  BE_from_array_32((ui32 *) &tmp_vol_params.file_time,
		   (ui32) (N_VOL_PARAMS_TIMES * sizeof(radtim_t)));
  
  BE_from_array_32((ui32 *) &tmp_vol_params.radar,
		   (ui32) (sizeof(radar_params_t) -
			   v_handle->vol_params->radar.nbytes_char));
  
  BE_from_array_32((ui32 *) &tmp_vol_params.cart,
		   (ui32) (sizeof(cart_params_t) -
			   v_handle->vol_params->cart.nbytes_char));
  
  BE_from_array_32((ui32 *) &tmp_vol_params.nfields, (ui32) sizeof(si32));
  
  if (ufwrite((char *) &tmp_vol_params,
	      sizeof(vol_params_t),
	      1, v_handle->vol_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing volume params.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * make local copy of the radar_elevations array, code into network
   * byte order and write to file
   */
  
  memcpy ((void *) radar_elevations,
          (void *) v_handle->radar_elevations,
          (size_t) (nelevations * sizeof(si32)));
  
  BE_from_array_32((ui32 *) radar_elevations,
	      (ui32) (nelevations * sizeof(si32)));
  
  if (ufwrite((char *) radar_elevations,
	      sizeof(si32),
	      (int) nelevations,
	      v_handle->vol_file) != nelevations) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing radar elevation array.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * make local copy of the plane_heights array, code into network
   * byte order and write to file
   */
  
  memcpy ((void *) plane_heights,
          (void *) *v_handle->plane_heights,
          (size_t) (nheights * sizeof(si32)));
  
  BE_from_array_32((ui32 *) plane_heights,
	      (ui32) (nheights * sizeof(si32)));
  
  if (ufwrite((char *) plane_heights,
	      sizeof(si32), (int) nheights,
	      v_handle->vol_file) != nheights) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing plane limits array.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * make local copy of the field params offset array, code into
   * network byte order and write to file
   */
  
  memcpy ((void *) field_params_offset,
          (void *) v_handle->field_params_offset,
          (size_t) (nfields * sizeof(si32)));
  
  BE_from_array_32((ui32 *) field_params_offset,
	      (ui32) (nfields * sizeof(si32)));
  
  if (ufwrite((char *) field_params_offset,
	      sizeof(si32),
	      (int) nfields,
	      v_handle->vol_file) != nfields) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteVolParams\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing field params offset array.\n");
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * free local arrays
   */
  
  ufree(radar_elevations);
  ufree(plane_heights);
  ufree(field_params_offset);
  
  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfWriteVolPlane.c
 *
 * part of the rfutil library - radar file access
 *
 * writes a plane of data to a radar volume file
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfWriteVolPlane(vol_file_handle_t *v_handle, si32 field_num,
		    si32 plane_num, const char *calling_routine)
     
{
  
  ui08 *fdata, *file_data;
  
  si32 npoints_per_plane;
  si32 nbytes;
  
  field_params_t *fparams;
  
  /*
   * set local variables
   */
  
  npoints_per_plane =
    v_handle->vol_params->cart.nx * v_handle->vol_params->cart.ny;
  
  fdata = v_handle->field_plane[field_num][plane_num];
  
  fparams = v_handle->field_params[field_num];
  
  /*
   * if required, run-length encode the field data
   */
  
  if (fparams->encoded) {
    
    /*
     * run-length encode
     */

    file_data =
      uRLEncode8(fdata,
		 (ui32) npoints_per_plane,
		 (ui32) 255,
		 (ui32 *) &nbytes);
    
  } else {
    
    file_data = fdata;
    nbytes = npoints_per_plane;
    
  } /* if (fparams->encoded) */
  
  /*
   * set plane offset
   */
  
  v_handle->plane_offset[field_num][plane_num] = ftell(v_handle->vol_file);
  
  /*
   * write the plane data array to file
   */
  
  if (ufwrite((char *) file_data, sizeof(char),
	      (int) nbytes, v_handle->vol_file) != nbytes) {
    
    fprintf(stderr, "ERROR - %s:%s:RfWriteVolPlane\n",
	    v_handle->prog_name, calling_routine);
    fprintf(stderr,
	    "Writing field data plane, field %ld, plane %ld.\n",
	    (long) field_num, (long) plane_num);
    perror(v_handle->vol_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * free up file data as needed
   */
  
  if (fparams->encoded && file_data != NULL)
    ufree(file_data);
  
  return (R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfWriteVolume.c
 *
 * part of the rfutil library - radar file access
 *
 * writes a radar volume to file
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfWriteVolume"

int RfWriteVolume(vol_file_handle_t *v_handle, const char *calling_routine)
     
{

  char calling_sequence[MAX_SEQ];

#ifdef OBSOLETE
  char dir_path[MAX_PATH_LEN];

  si32 ifield;
  si32 nplanes, iplane;

  mode_t mode;
  path_parts_t fparts;
  date_time_t date_time;
#endif

  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * see if the output should be in Dobson format or in
   * MDV foramt
   */

#ifdef OBSOLETE
  if (getenv("TITAN_OUTPUT_AS_DOBSON") == NULL) {
#endif

    /*
     * MDV output
     */

    return (RfWriteVolumeMdv(v_handle, MDV_PLANE_RLE8,
			     calling_sequence));
#ifdef OBSOLETE
  }
  
  /*
   * set constants
   */
  
  nplanes = v_handle->vol_params->cart.nz;

  /*
   * set file time to gmt
   */

  ugmtime(&date_time);
  Rfdtime2rtime(&date_time, &v_handle->vol_params->file_time);

  /*
   * try to open the file
   */
    
  if ((v_handle->vol_file = fopen(v_handle->vol_file_path, "w")) == NULL) {

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

    if (mkdir(dir_path, mode) != 0) {
      fprintf(stderr, "ERROR - %s:%s:RfWriteVolume.\n",
	      v_handle->prog_name, calling_routine);
      fprintf(stderr, "Cannot create radar data subdirectory.\n");
      perror(dir_path);
      return (R_FAILURE);
    }
      
    /*
     * try opening file again
     */
    
    if ((v_handle->vol_file = fopen(v_handle->vol_file_path, "w")) == NULL) {
      fprintf(stderr, "ERROR - %s:%s:RfWriteVolume.\n",
	      v_handle->prog_name, calling_routine);
      fprintf(stderr, "Creating rdata file.\n");
      perror(v_handle->vol_file_path);
      return (R_FAILURE);
    }
    
  } /* if ((v_handle->vol_file = fopen(vol_file_path, "w")) == NULL) */

  /*
   * seek ahead to the point after the label, header and
   * header arrays
   */

  if(RfSeekEndVolHeader(v_handle, calling_sequence)) {
    fclose(v_handle->vol_file);
    return(R_FAILURE);
  }

  /*
   * loop through fields
   */
  
  for (ifield =  0; ifield < v_handle->vol_params->nfields; ifield++) {

    /*
     * loop through the planes
     */

    for (iplane = 0; iplane < nplanes; iplane++) {

      /*
       * write plane data
       */

      if(RfWriteVolPlane(v_handle, ifield,
			 iplane, calling_sequence)) {
	fclose(v_handle->vol_file);
	return(R_FAILURE);
      }

    } /* iplane */

    /*
     * write field params
     */

    if(RfWriteVolFparams(v_handle, ifield,
			 calling_sequence)) {
      fclose(v_handle->vol_file);
      return(R_FAILURE);
    }

  } /* ifield */

  /*
   * write vol params
   */

  if(RfWriteVolParams(v_handle, calling_sequence)) {
    fclose(v_handle->vol_file);
    return(R_FAILURE);
  }

  /*
   * close file and return
   */
  
  fclose(v_handle->vol_file);
  return (R_SUCCESS);

#endif

}

#undef THIS_ROUTINE

#ifdef JUNK

/**********************************************************************
 * RLDecode() - performs run-length decoding on byte data which was
 *              compressed using RLEncode
 *
 * Returns the full data array. The size of the array is passed back via
 * nbytes_full.
 *
 **********************************************************************/

static ui08 *RLDecode(ui08 *coded_data, ui32 *nbytes_full)
     
{

  register int runcount;
  ui32 nbytes_coded;
  ui32 nbytes_extra;

  register ui08 byteval;
  ui08 *full_data;
  register ui08 *last_data;
  register ui08 *fdata, *cdata;

  if (coded_data != NULL) {

    /*
     * get number of bytes for the coded and full data
     */

    *nbytes_full = *((si32 *) coded_data + 1);
    nbytes_coded = *((si32 *) coded_data + 2);
    nbytes_extra = RL7_NBYTES_EXTRA;

    /*
     * get space for full data
     */

    full_data = (ui08 *) umalloc(*nbytes_full);
    
    fdata = full_data;
    cdata = coded_data + nbytes_extra;

    last_data = cdata + (nbytes_coded - 1);

    while (cdata < last_data) {

      byteval = *cdata;

      if ((byteval & 0x80) == 0x80) {

	/*
	 * if most significant bit set, mask off lower 7 bits and
	 * use as the count on the next byte value
	 */

	runcount = byteval & 0x7f;
	cdata++;
	byteval  = *cdata;

	/*
	 * set runcount values
	 */

	memset((char *) fdata, (int) byteval, (int) runcount);
	fdata += runcount;

      } else {

	/*
	 * if most significant bit not set, set single byte
	 */

	*fdata = byteval;
	fdata++;

      } /* if ((byteval & 0x80) == 0x80) */

      cdata++;

    } /* while (cdata < last_data) */

    return (full_data);

  } else {

    return ((ui08 *) NULL);

  }

}

/**********************************************************************
 * RLEncode() - performs run-length encoding on byte data which uses
 *              only the lower 7 bits
 *
 * In the coded data, the first 12 bytes are as follows:
 *
 * (si32) nbytes_array, (si32) nbytes_full, (si32) nbytes_coded.
 *
 * The coded data follows these 12 bytes. The coded data is padded out
 * to end on a 4-byte boundary.
 *
 * The memory for the encoded array is allocated by this routine.
 *
 * Returns pointer to the encoded array. The number of bytes in the
 * encodeded data array is returned via nbytes_array.
 *
 **********************************************************************/

static ui08 *RLEncode(ui08 *full_data, ui32 nbytes_full,
		      ui32 *nbytes_array)
     
{

  register ui08 byteval;
  register ui08 *coded_data;
  register ui08 *fdata, *cdata, *last_data;

  register ui32 runcount;

  ui32 nbytes_coded;
  ui32 nbytes_extra;
  ui32 nbytes_unpadded;

  /*
   * full_data is original array
   * fdata is pointer into original array
   * cdata is pointer into coded array
   * last_data is pointer to last byte in original array
   */

  /*
   * initial allocation of encoded array, the size of the original array
   * plus the extra bytes at the start for the nbyte values, plus enough
   * bytes to pass a word boundary. This will be sufficient for the
   * worst case in which there is no compression
   */

  nbytes_extra = RL7_NBYTES_EXTRA;
  
  coded_data = (ui08 *) umalloc
    ((ui32) (nbytes_full + nbytes_extra + NBYTES_WORD));

  /*
   * set the number of bytes in the full data, and the pointer to the
   * number of encoded bytes
   */

  /*
   * set pointers to data arrays
   */

  fdata = full_data;
  cdata = coded_data + nbytes_extra;
  
  /*
   * set pointer to last data byte
   */

  last_data = fdata + (nbytes_full - 1);

  if (full_data != NULL) {

    while (fdata < last_data) {

      /*
       * get byte value
       */

      byteval = *fdata;

      /*
       * return with NULL pointer if data exceeds 127
       */

      if (byteval > 127) {

	fprintf(stderr, "ERROR - RLEncode\n");
	fprintf(stderr, "Byte value exceeds 127.\n");
	return ((ui08 *) NULL);

      } /* if (byteval .... */
  
      runcount = 1;

      while ((fdata < last_data) && (runcount < 127) &&
	     (*(fdata + 1) == byteval)) {

	/*
	 * count up adjacent bytes of same value
	 */

	fdata++;
	runcount++;
	
      }

      if (runcount == 1) {

	/*
	 * count = 1, store as single byte
	 */

	*cdata = byteval;
	cdata++;

      } else {

	/*
	 *  count > 1, store as count then byte value byte
	 */

	*cdata = 0x80 | runcount;
	*(cdata + 1) = byteval;
	cdata += 2;

      }

      fdata++;

    } /* while (fdata < last_data) */

    /*
     * compute the number of bytes in the encoded data, including the 
     * leading 8 bytes and the padding to go to a word boundary
     */

    nbytes_coded = cdata - coded_data - nbytes_extra;
    nbytes_unpadded = nbytes_coded + nbytes_extra;
    *nbytes_array =
      ((ui32) ((nbytes_unpadded - 1) / NBYTES_WORD) + 1) * NBYTES_WORD;

    /*
     * realloc the coded_data array
     */

    coded_data = (ui08 *) urealloc
      ((char *) coded_data, (ui32) *nbytes_array);

    /*
     * set the bytes counts
     */

    *((si32 *) coded_data) = *nbytes_array;
    *((si32 *) coded_data + 1) = nbytes_full;
    *((si32 *) coded_data + 2) = nbytes_coded;

    return (coded_data);

  } else {

    return ((ui08 *) NULL);

  } /* if (full_data != NULL) */

}

#endif
