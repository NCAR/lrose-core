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
 * write_verification_file.c
 *
 * writes the verification data to a file
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

#define N_VERIFICATION_FIELDS 2
#define ALL_STORMS_FIELD 0
#define VALID_STORMS_FIELD 1

static ui08 *All_storms_grid;
static ui08 *Valid_storms_grid;

void write_verification_file(vol_file_handle_t *v_handle)

{

  static int first_call = TRUE;
  static vol_file_handle_t verify_v_handle;

  char note[80];

  ui08 *composite_grid;
  ui08 *comp;
  ui08 *all, *valid;

  int i;

  cart_params_t *cart;

  cart = &v_handle->vol_params->cart;

  if (!Glob->params.create_verification_files)
    return;

  /*
   * initialize verify vol file handle
   */

  if (first_call) {

    RfInitVolFileHandle(&verify_v_handle,
			Glob->prog_name,
			(char *) NULL,
			(FILE *) NULL);

    if (RfAllocVolParams(&verify_v_handle,
			 "write_verification_file"))
      tidy_and_exit(-1);
  
    first_call = FALSE;
    
  } /* if (first_call ... */

  /*
   * copy in vol params
   */

  memcpy ((void *) verify_v_handle.vol_params,
          (void *) v_handle->vol_params,
          (size_t) sizeof(vol_params_t));

  /*
   * append the note
   */

  sprintf(note, "\nVerification data, dbz threshold %g", 
	  Glob->params.low_dbz_threshold);

  strncat(verify_v_handle.vol_params->note, note, 
	  (VOL_PARAMS_NOTE_LEN -
	   strlen(verify_v_handle.vol_params->note) - 1));
  
  /*
   * adjust cart params to indicate there is only 1 level
   * and 2 fields
   */

  verify_v_handle.vol_params->cart.nz = 1;
  verify_v_handle.vol_params->nfields = N_VERIFICATION_FIELDS;

  /*
   * allocate the arrays for the vol file handle
   */

  if (RfAllocVolArrays(&verify_v_handle,
		       "write_verification_file"))
    tidy_and_exit(-1);

  /*
   * copy in the radar elevations array and the first elements
   * of the plane heights array
   */

  memcpy ((void *) verify_v_handle.radar_elevations,
          (void *) v_handle->radar_elevations,
          (size_t) (v_handle->vol_params->radar.nelevations * sizeof(si32)));

  memcpy ((void *) *verify_v_handle.plane_heights,
          (void *) *v_handle->plane_heights,
          (size_t) (N_PLANE_HEIGHT_VALUES * sizeof(si32)));
  
  /*
   * set the field params
   */

  memcpy ((void *) verify_v_handle.field_params[ALL_STORMS_FIELD],
          (void *) v_handle->field_params[Glob->params.dbz_field],
          (size_t) sizeof(field_params_t));

  memcpy ((void *) verify_v_handle.field_params[VALID_STORMS_FIELD],
          (void *) v_handle->field_params[Glob->params.dbz_field],
          (size_t) sizeof(field_params_t));

  strcpy(verify_v_handle.field_params[ALL_STORMS_FIELD]->name,
	 "Verify refl, all storms");
  
  strcpy(verify_v_handle.field_params[VALID_STORMS_FIELD]->name,
	 "Verify refl, valid storms");

  /*
   * get the composite reflectivity grid - this is the
   * max reflectivity at any height
   */

  composite_grid = get_comp_grid();
  
  /*
   * load up the verification grids with the composite reflectivity,
   * masked using the storm runs as applicable
   */

  comp = composite_grid;
  all = All_storms_grid;
  valid = Valid_storms_grid;
  
  for (i = 0; i < cart->nx * cart->ny; i++, all++, valid++, comp++) {
    
    if (*all) {
      *all = *comp;
    }

    if (*valid) {
      *valid = *comp;
    }

  } /* i */
  
  /*
   * set the field data pointers to the grid data
   */

  verify_v_handle.field_plane[ALL_STORMS_FIELD][0] = All_storms_grid;
  verify_v_handle.field_plane[VALID_STORMS_FIELD][0] = Valid_storms_grid;

  /*
   * write the file
   */

  if (RfWriteDobson(&verify_v_handle,
		    TRUE,
		    Glob->params.debug,
		    Glob->params.verify_dir,
		    Glob->params.rdata_file_ext,
		    Glob->prog_name,
		    "write_verification_file")) {
    tidy_and_exit(-1);
  }
  
}

/****************************
 * alloc_verification_grids()
 */

void alloc_verification_grids(void)

{

  static int first_call = TRUE;
  static si32 nbytes_alloc;
  si32 nbytes_needed;

  nbytes_needed = Glob->nx * Glob->ny * sizeof(ui08);
  
  if (first_call) {

    All_storms_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    Valid_storms_grid = (ui08 *) umalloc ((ui32) nbytes_needed);
    nbytes_alloc = nbytes_needed;

    first_call = FALSE;
    
  } else {

    /*
     * adjust grid allocation as necessary
     */
  
    if (nbytes_needed > nbytes_alloc) {

      All_storms_grid = (ui08 *) urealloc ((char *) All_storms_grid,
					     (ui32) nbytes_needed);
      Valid_storms_grid = (ui08 *) urealloc ((char *) Valid_storms_grid,
					       (ui32) nbytes_needed);
      nbytes_alloc = nbytes_needed;
      
    } /* if (Glob->nx != nx_alloc ... */
    
  } /* if (first_call) */

  /*
   * zero out grids
   */

  memset((void *) All_storms_grid, 0, (int) nbytes_alloc);
  memset((void *) Valid_storms_grid, 0, (int) nbytes_alloc);

  return;

}

/***************
 * update_grid()
 */

void update_all_storms_grid(Clump_order *clump)

{

  ui08 *grid;
  int intv;
  int offset;
  Interval *intvl;

  grid = All_storms_grid;
  
  for (intv = 0; intv < clump->size; intv++) {
    
    intvl = clump->ptr[intv];
    offset = intvl->row_in_plane * Glob->nx + intvl->begin;
    memset((void *) (grid + offset), 1, intvl->len);
    
  } /* intv */

}

void update_valid_storms_grid(Clump_order *clump,
			      storm_ident_grid_t *grid_params)

{

  ui08 *grid;
  int ix, iy;
  int intv;
  int offset;
  Interval *intvl;

  grid = Valid_storms_grid;

  for (intv = 0; intv < clump->size; intv++) {
    
    intvl = clump->ptr[intv];
    iy = intvl->row_in_plane + grid_params->start_iy;
    ix = intvl->begin + grid_params->start_ix;
    offset = iy * Glob->nx + ix;
    memset((void *) (grid + offset), 1, intvl->len);
    
  } /* intv */

}




