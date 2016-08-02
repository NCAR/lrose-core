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
 * read_verification_file.c
 *
 * Opens the files, reads in the headers
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 *************************************************************************/

#include "verify_day.h"

#define NO_MAP -1

static void load_map_arrays(void);

static cart_params_t Cart;
static si32 *X_map, *Y_map;

int read_verification_file(storm_file_scan_header_t *scan,
			   time_t scan_time,
			   ui08 **verify_grid)

{

  static int first_call = TRUE;
  static vol_file_handle_t v_handle;

  char *file_path;
  ui08 *radar_grid, *rgrid;
  si32 m_y;
  si32 ix, iy;

  date_time_t file_time;

  file_time.unix_time = scan_time;
  uconvert_from_utime(&file_time);

  if (Glob->debug)
    fprintf(stderr,
	    "reading verification file, time = %d/%d/%d_%d:%d:%d\n",
	    file_time.year, file_time.month, file_time.day,
	    file_time.hour, file_time.min, file_time.sec);

  /*
   * if first call, initialize vol file handle
   */

  if (first_call == TRUE)
    RfInitVolFileHandle(&v_handle,
			Glob->prog_name,
			(char *) NULL,
			(FILE *) NULL);

  /*
   * compute the verification data file path
   */

  file_path = (char *)
    umalloc ((ui32) (strlen(Glob->verify_dir) +
		      2 * strlen(PATH_DELIM) +
		      strlen(Glob->verify_file_ext) + 16));
  
  sprintf(file_path, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	  Glob->verify_dir, PATH_DELIM,
	  file_time.year, file_time.month, file_time.day,
	  PATH_DELIM,
	  file_time.hour, file_time.min, file_time.sec,
	  Glob->verify_file_ext);

  v_handle.vol_file_path = file_path;

  /*
   * read in verification file
   */

  if (RfReadVolume(&v_handle, "read_verification_file") != R_SUCCESS) {

    ufree((char *) file_path);
    return (R_FAILURE);

  }

  if (first_call == TRUE) {

    /*
     * make static copy of cart params
     */

    Cart = v_handle.vol_params->cart;

    /*
     * load up map arrays
     */

    load_map_arrays();

    first_call = FALSE;
    
  } else {
    
    /*
     * if cartesian dimensions have changed, alter the memory allocation
     * accordingly
     */
    
    if (memcmp((void *) &v_handle.vol_params->cart,
	       (void *) &Cart,
	       (size_t) sizeof(cart_params_t)) != 0) {

      ufree((char *) X_map);
      ufree((char *) Y_map);

      Cart = v_handle.vol_params->cart;
      load_map_arrays();

    } /* if (memcmp((char *) &v_handle.vol_params->cart ... */

  } /* if (first_call ..... */

  /*
   * zero out verification grid
   */

  memset ((void *) *verify_grid,
          (int) 0, (size_t) (Glob->ny * Glob->nx * sizeof(ui08)));

  /*
   * map the radar cartesian grid onto the verification grid
   */

  radar_grid = v_handle.field_plane[Glob->verification_field][0];

  for (iy = 0; iy < Cart.ny; iy++) {

    if (Y_map[iy] != NO_MAP) {

      rgrid = radar_grid + (iy * Cart.nx);

      m_y = Y_map[iy];

      for (ix = 0; ix < Cart.nx; ix++) {
	
	if (X_map[ix] != NO_MAP)
	  verify_grid[m_y][X_map[ix]] += *rgrid;

	rgrid++;
	
      } /* ix */

    } /* if (Y_map[iy] != NO_MAP) */

  } /* iy */

  /*
   * free up file path string
   */

  ufree((char *) file_path);

  return (R_SUCCESS);

}

/****************************************************************************
 * load_map_arrays()
 *
 * loads up the arrays used to map the radar cartesian grid onto
 * the verification grid.
 */

static void load_map_arrays(void)

{

  si32 ix, iy;
  si32 nx, ny;
  si32 verify_grid_ix, verify_grid_iy;

  double dx, dy;
  double minx, miny;
  double radar_cart_x, radar_cart_y;

  nx = Cart.nx;
  ny = Cart.ny;

  X_map = (si32 *) umalloc ((ui32) nx * sizeof(si32));
  Y_map = (si32 *) umalloc ((ui32) ny * sizeof(si32));

  dx = (double) Cart.dx / (double) Cart.km_scalex;
  dy = (double) Cart.dy / (double) Cart.km_scaley;
  
  minx = (double) Cart.minx / (double) Cart.km_scalex;
  miny = (double) Cart.miny / (double) Cart.km_scaley;

  for (ix = 0; ix < nx; ix++) {

    radar_cart_x = minx + ix * dx;

    verify_grid_ix = (si32) ((radar_cart_x - Glob->minx) / Glob->dx + 0.5);

    if (verify_grid_ix < 0)
      X_map[ix] = NO_MAP;
    else if (verify_grid_ix > Glob->nx - 1)
      X_map[ix] = NO_MAP;
    else
      X_map[ix] = verify_grid_ix;

  } /* ix */

  for (iy = 0; iy < ny; iy++) {

    radar_cart_y = miny + iy * dy;

    verify_grid_iy = (si32) ((radar_cart_y - Glob->miny) / Glob->dy + 0.5);

    if (verify_grid_iy < 0)
      Y_map[iy] = NO_MAP;
    else if (verify_grid_iy > Glob->ny - 1)
      Y_map[iy] = NO_MAP;
    else
      Y_map[iy] = verify_grid_iy;

  } /* iy */

  return;

}
