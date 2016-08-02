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
/*********************************************************************
 * mask_low_tops.c
 *
 * masks out radar data which does not have the specified tops
 *
 * RAP, NCAR, Boulder CO
 *
 * July 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include  <string.h>
#include "storm_ident.h"

static ui08 *Tops_grid;
static ui08 *Shallow_grid;
static ui08 *Shallow_edm_grid;
static ui08 *Shallow_mask_grid;

#define HEIGHT_SCALE 10

enum
{
  TOPS_FIELD,
  SHALLOW_FIELD,
  SHALLOW_EDM_FIELD,
  SHALLOW_MASK_FIELD,
  N_TOPS_FIELDS
} tops_fields;

static void write_tops_file(vol_file_handle_t *v_handle)

{

  static int first_call = TRUE;
  static vol_file_handle_t tops_v_handle;
  char note[80];
  cart_params_t *cart;
  field_params_t *fparams;

  cart = &v_handle->vol_params->cart;

  if (!Glob->params.create_tops_files)
    return;

  /*
   * initialize tops vol file handle
   */

  if (first_call) {
    
    RfInitVolFileHandle(&tops_v_handle,
			Glob->prog_name,
			(char *) NULL,
			(FILE *) NULL);
    
    if (RfAllocVolParams(&tops_v_handle,
			 "write_tops_file"))
      tidy_and_exit(-1);
    
    first_call = FALSE;
    
  } /* if (first_call ... */
  
  /*
   * copy in vol params
   */

  memcpy ((void *) tops_v_handle.vol_params,
          (void *) v_handle->vol_params,
          (size_t) sizeof(vol_params_t));
  
  /*
   * append the note
   */
  
  sprintf(note,
	  "\nTops data, dbz threshold %g, "
	  "min_radar_tops %g, tops_edge_margin %g", 
	  Glob->params.low_dbz_threshold, Glob->params.min_radar_tops,
	  Glob->params.tops_edge_margin);
  
  strncat(tops_v_handle.vol_params->note, note, 
	  (VOL_PARAMS_NOTE_LEN -
	   strlen(tops_v_handle.vol_params->note) - 1));
  
  /*
   * adjust cart params to indicate there is only 1 level
   * and N_TOPS_FIELDS
   */
  
  tops_v_handle.vol_params->cart.nz = 1;
  tops_v_handle.vol_params->nfields = N_TOPS_FIELDS;

  /*
   * allocate the arrays for the vol file handle
   */

  if (RfAllocVolArrays(&tops_v_handle,
		       "write_tops_file"))
    tidy_and_exit(-1);
  
  /*
   * copy in the radar elevations array and the first elements
   * of the plane heights array
   */
  
  memcpy ((void *) tops_v_handle.radar_elevations,
          (void *) v_handle->radar_elevations,
          (size_t) (v_handle->vol_params->radar.nelevations * sizeof(si32)));

  memcpy ((void *) *tops_v_handle.plane_heights,
          (void *) *v_handle->plane_heights,
          (size_t) (N_PLANE_HEIGHT_VALUES * sizeof(si32)));
  
  /*
   * set the field params
   */

  fparams = tops_v_handle.field_params[TOPS_FIELD];
  fparams->encoded = TRUE;
  fparams->factor = 1000;
  fparams->scale = 1000 / HEIGHT_SCALE;
  fparams->bias = 0;
  fparams->missing_val = 0;
  fparams->noise = 0;
  strcpy(fparams->name, "Tops");
  strcpy(fparams->units, "km");
  strcpy(fparams->transform, "none");

  fparams = tops_v_handle.field_params[SHALLOW_FIELD];
  *fparams = *tops_v_handle.field_params[TOPS_FIELD];
  strcpy(fparams->name, "Shallow_echo");
  strcpy(fparams->units, "km");

  fparams = tops_v_handle.field_params[SHALLOW_EDM_FIELD];
  *fparams = *tops_v_handle.field_params[TOPS_FIELD];
  fparams->scale = 1000;
  strcpy(fparams->name, "Shallow_edm");
  strcpy(fparams->units, "count");
  
  fparams = tops_v_handle.field_params[SHALLOW_MASK_FIELD];
  *fparams = *tops_v_handle.field_params[SHALLOW_EDM_FIELD];
  strcpy(fparams->name, "Shallow_mask");
  strcpy(fparams->units, "count");
  
  /*
   * set the field data pointers to the grid data
   */
  
  tops_v_handle.field_plane[TOPS_FIELD][0] = Tops_grid;
  tops_v_handle.field_plane[SHALLOW_FIELD][0] = Shallow_grid;
  tops_v_handle.field_plane[SHALLOW_EDM_FIELD][0] = Shallow_edm_grid;
  tops_v_handle.field_plane[SHALLOW_MASK_FIELD][0] = Shallow_mask_grid;
  
  /*
   * write the file
   */
  
  if (RfWriteDobson(&tops_v_handle, TRUE,
		    Glob->params.debug,
		    Glob->params.tops_dir,
		    Glob->params.rdata_file_ext,
		    Glob->prog_name,
		    "write_tops_file")) {
    tidy_and_exit(-1);
  }

  return;

}

/****************************
 * alloc_tops_grids()
 */

static void alloc_tops_grids(void)

{

  static int first_call = TRUE;
  static si32 nbytes_alloc;
  si32 nbytes_needed;

  nbytes_needed = Glob->nx * Glob->ny * sizeof(ui08);
  
  if (first_call) {

    Tops_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    Shallow_edm_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    Shallow_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    Shallow_mask_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    nbytes_alloc = nbytes_needed;

    first_call = FALSE;
    
  } else {

    /*
     * adjust grid allocation as necessary
     */
  
    if (nbytes_needed < nbytes_alloc) {
	
      Tops_grid = (ui08 *) urealloc ((char *) Tops_grid,
					     (ui32) nbytes_needed);
      Shallow_edm_grid = (ui08 *) urealloc ((char *) Shallow_edm_grid,
					     (ui32) nbytes_needed);
      Shallow_grid = (ui08 *) urealloc ((char *) Shallow_grid,
				      (ui32) nbytes_needed);
      Shallow_mask_grid = (ui08 *) urealloc ((char *) Shallow_mask_grid,
					 (ui32) nbytes_needed);
      
      nbytes_alloc = nbytes_needed;

    } /* if (nbytes_needed < nbytes_alloc) */
    
  } /* if (first_call) */

  /*
   * zero out grids
   */

  memset((void *) Tops_grid, 0, (int) nbytes_alloc);
  memset((void *) Shallow_edm_grid, 0, (int) nbytes_alloc);
  memset((void *) Shallow_grid, 0, (int) nbytes_alloc);
  memset((void *) Shallow_mask_grid, 0, (int) nbytes_alloc);

  return;

}

/***************************
 * mask_low_tops()
 */

void mask_low_tops(vol_file_handle_t *v_handle)

{

  static Row_hdr *Rowh = NULL;
  static int Nrows_alloc = 0;
  static Interval *Intervals = NULL;
  static int N_intervals_alloc = 0;

  ui08 *dbz;
  ui08 *tops, *shallow;
  ui08 *shallow_edm, *shallow_mask;

  si32 i, iz;
  si32 npoints_per_plane = Glob->nx * Glob->ny;
  si32 dbz_threshold = Glob->low_dbz_byte;
  si32 height_byte_val;
  si32 height_byte_threshold;
  si32 edm_grid_threshold;
  si32 num_intervals;
  
  double plane_height;
  double mean_grid_res;
  
  /*
   * check memory allocation
   */

  EG_alloc_rowh((int) Glob->ny, &Nrows_alloc, &Rowh);
  alloc_tops_grids();
  
  /*
   * load tops grid
   */

  for (iz = 0; iz < Glob->nz; iz++) {

    plane_height = Glob->min_z + Glob->delta_z * iz;
    
    height_byte_val = (si32) (plane_height * HEIGHT_SCALE + 0.5);
    if (height_byte_val > 255) {
      height_byte_val = 255;
    }
    
    tops = Tops_grid;
    dbz = v_handle->field_plane[Glob->params.dbz_field][iz];
    
    for (i = 0; i < npoints_per_plane; i++, tops++, dbz++) {
      if (*dbz > dbz_threshold) {
	*tops = height_byte_val;
      }
    } /* i */
    
  } /* iz */
  
  /*
   * load up grid with only shallow tops
   */

  height_byte_threshold = (si32) (Glob->params.min_radar_tops * HEIGHT_SCALE + 0.5);
  if (height_byte_threshold > 255) {
    height_byte_threshold = 255;
  }
  
  tops = Tops_grid;
  shallow = Shallow_grid;
  for (i = 0; i < npoints_per_plane; i++, tops++, shallow++) {
    if (*tops < height_byte_threshold) {
      *shallow = *tops;
    }
  } /* i */
  
  /*
   * get the intervals in shallow grid
   */
  
  num_intervals = EG_find_intervals(Glob->ny, Glob->nx,
				    Shallow_grid,
				    &Intervals, &N_intervals_alloc,
				    Rowh, 1);
  /*
   * compute the edm of the shallow field
   */
  
  EG_edm_2d(Rowh, Shallow_edm_grid,
	    (int) Glob->nx, (int) Glob->ny, 1);
  
  /*
   * threshold the edm field to get mask field
   */

  mean_grid_res = (Glob->delta_x + Glob->delta_y) / 2.0;
  edm_grid_threshold =
    (si32) ((Glob->params.tops_edge_margin / mean_grid_res) + 0.5);

  shallow_edm = Shallow_edm_grid;
  shallow_mask = Shallow_mask_grid;
  for (i = 0; i < npoints_per_plane; i++, shallow_edm++, shallow_mask++) {
    if (*shallow_edm >= edm_grid_threshold) {
      *shallow_mask = 1;
    }
  } /* i */

  /*
   * mask out reflectivity data in areas of low tops
   */

  for (iz = 0; iz < Glob->nz; iz++) {

    dbz = v_handle->field_plane[Glob->params.dbz_field][iz];
    shallow_mask = Shallow_mask_grid;
    
    for (i = 0; i < npoints_per_plane; i++, dbz++, shallow_mask++) {
      if (*shallow_mask) {
	*dbz = 0;
      }
    } /* i */
    
  } /* iz */
  
  /*
   * if required, write out tops files
   */

  if (Glob->params.create_tops_files) {
    write_tops_file(v_handle);
  } /* if (Glob->params.create_tops_files) */

  return;

}

