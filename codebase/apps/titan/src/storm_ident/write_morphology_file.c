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
 * write_morphology_file.c
 *
 * writes the morphology data to a file
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include  <string.h>
#include "storm_ident.h"

static ui08 *Refl_margin_grid;
static ui08 *Edm_grid;
static ui08 *Morphology_grid;
static ui08 *Eroded_grid;

enum
{
  REFL_MARGIN_FIELD,
  EDM_FIELD,
  MORPHOLOGY_FIELD,
  ERODED_FIELD,
  N_MORPH_FIELDS
} morph_fields;

void write_morphology_file(vol_file_handle_t *v_handle)

{

  static int first_call = TRUE;
  static vol_file_handle_t morph_v_handle;
  char note[80];
  cart_params_t *cart;
  field_params_t *fparams;

  cart = &v_handle->vol_params->cart;

  if (!Glob->params.create_morphology_files)
    return;

  /*
   * initialize morph vol file handle
   */

  if (first_call) {
    
    RfInitVolFileHandle(&morph_v_handle,
			Glob->prog_name,
			(char *) NULL,
			(FILE *) NULL);
    
    if (RfAllocVolParams(&morph_v_handle,
			 "write_morphology_file"))
      tidy_and_exit(-1);
    
    first_call = FALSE;
    
  } /* if (first_call ... */

  /*
   * copy in vol params
   */

  memcpy ((void *) morph_v_handle.vol_params,
          (void *) v_handle->vol_params,
          (size_t) sizeof(vol_params_t));
  
  /*
   * append the note
   */
  
  sprintf(note,
	  "\nMorphology data, dbz threshold %g, "
	  "erosion threshold %g, refl_divisor %g", 
	  Glob->params.low_dbz_threshold,
	  Glob->params.morphology_erosion_threshold,
	  Glob->params.morphology_refl_divisor);
  
  strncat(morph_v_handle.vol_params->note, note, 
	  (VOL_PARAMS_NOTE_LEN -
	   strlen(morph_v_handle.vol_params->note) - 1));
  
  /*
   * adjust cart params to indicate there is only 1 level
   * and N_MORPH_FIELDS
   */
  
  morph_v_handle.vol_params->cart.nz = 1;
  morph_v_handle.vol_params->nfields = N_MORPH_FIELDS;

  /*
   * allocate the arrays for the vol file handle
   */

  if (RfAllocVolArrays(&morph_v_handle,
		       "write_morphology_file"))
    tidy_and_exit(-1);

  /*
   * copy in the radar elevations array and the first elements
   * of the plane heights array
   */

  memcpy ((void *) morph_v_handle.radar_elevations,
          (void *) v_handle->radar_elevations,
          (size_t) (v_handle->vol_params->radar.nelevations * sizeof(si32)));

  memcpy ((void *) *morph_v_handle.plane_heights,
          (void *) *v_handle->plane_heights,
          (size_t) (N_PLANE_HEIGHT_VALUES * sizeof(si32)));
  
  /*
   * set the field params
   */

  fparams = morph_v_handle.field_params[REFL_MARGIN_FIELD];
  fparams->encoded = TRUE;
  fparams->factor = 1;
  fparams->scale = 1;
  fparams->bias = 0;
  fparams->missing_val = 0;
  fparams->noise = 0;
  strcpy(fparams->name, "Refl margin");
  strcpy(fparams->units, "count");
  strcpy(fparams->transform, "none");

  fparams = morph_v_handle.field_params[EDM_FIELD];
  *fparams = *morph_v_handle.field_params[REFL_MARGIN_FIELD];
  strcpy(fparams->name, "EDM");
  strcpy(fparams->units, "km");

  fparams = morph_v_handle.field_params[MORPHOLOGY_FIELD];
  *fparams = *morph_v_handle.field_params[REFL_MARGIN_FIELD];
  strcpy(fparams->name, "Morphology");
  strcpy(fparams->units, "count");
  
  fparams = morph_v_handle.field_params[ERODED_FIELD];
  *fparams = *morph_v_handle.field_params[REFL_MARGIN_FIELD];
  strcpy(fparams->name, "Eroded");
  strcpy(fparams->units, "count");
  
  /*
   * set the field data pointers to the grid data
   */
  
  morph_v_handle.field_plane[REFL_MARGIN_FIELD][0] = Refl_margin_grid;
  morph_v_handle.field_plane[EDM_FIELD][0] = Edm_grid;
  morph_v_handle.field_plane[MORPHOLOGY_FIELD][0] = Morphology_grid;
  morph_v_handle.field_plane[ERODED_FIELD][0] = Eroded_grid;
  
  /*
   * write the file
   */
  
  if (RfWriteDobson(&morph_v_handle, TRUE,
		    Glob->params.debug,
		    Glob->params.morphology_dir,
		    Glob->params.rdata_file_ext,
		    Glob->prog_name,
		    "write_morphology_file")) {
    tidy_and_exit(-1);
  }

  return;

}

/****************************
 * alloc_morphology_grids()
 */

void alloc_morphology_grids(void)

{

  static int first_call = TRUE;
  static si32 nbytes_alloc;
  si32 nbytes_needed;

  nbytes_needed = Glob->nx * Glob->ny * sizeof(ui08);
  
  if (first_call) {

    Refl_margin_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    Morphology_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    Edm_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    Eroded_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    nbytes_alloc = nbytes_needed;

    first_call = FALSE;
    
  } else {

    /*
     * adjust grid allocation as necessary
     */
  
    if (nbytes_needed < nbytes_alloc) {
	
      Refl_margin_grid = (ui08 *) urealloc ((char *) Refl_margin_grid,
					     (ui32) nbytes_needed);
      Morphology_grid = (ui08 *) urealloc ((char *) Morphology_grid,
					     (ui32) nbytes_needed);
      Edm_grid = (ui08 *) urealloc ((char *) Edm_grid,
				      (ui32) nbytes_needed);
      Eroded_grid = (ui08 *) urealloc ((char *) Eroded_grid,
					 (ui32) nbytes_needed);
      
      nbytes_alloc = nbytes_needed;

    } /* if (nbytes_needed < nbytes_alloc) */
    
  } /* if (first_call) */

  /*
   * zero out grids
   */

  memset((void *) Refl_margin_grid, 0, (int) nbytes_alloc);
  memset((void *) Morphology_grid, 0, (int) nbytes_alloc);
  memset((void *) Edm_grid, 0, (int) nbytes_alloc);
  memset((void *) Eroded_grid, 0, (int) nbytes_alloc);

  return;

}

/***************************
 * update_morphology_grids()
 */

void update_morphology_grids(storm_ident_grid_t *grid_params,
			     ui08 *edm, ui08 *refl_margin,
			     ui08 *morph, ui08 *eroded,
			     int min_ix, int min_iy)

{

  int iy, ix;
  int offset;
  
  for (iy = 0; iy < grid_params->ny; iy++) {
  
    offset = (iy + min_iy) * Glob->nx + min_ix;

    for (ix = 0; ix < grid_params->nx;
	 ix++, offset++, edm++, refl_margin++, morph++, eroded++) {
      
      if (*edm) {
	Edm_grid[offset] = *edm;
	Refl_margin_grid[offset] = *refl_margin;
	Morphology_grid[offset] = *morph;
	Eroded_grid[offset] = *eroded;
      }
      
    } /* ix */
    
  } /* iy */

  return;

}

