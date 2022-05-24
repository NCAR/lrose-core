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
/**************************************************************************
 * remap_file.c
 *
 * Remaps a dobson volume onto a plane. Only the data between the
 * given height limits in included. Where more than 1 data value
 * maps to a grid point, the max value is taken
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * July 1992
 *
 **************************************************************************/

#include "vol_to_plane.h"

void remap_file(char *file_path)

{

  static int first_call = TRUE;
  static vol_file_handle_t in_index, out_index;

  char note[80];
  long ifield, iplane;
  long nfields, nplanes;
  long start_plane, end_plane;
  long ix, iy, jx, jy;
  long in_pos, out_pos;
  double in_minx, in_miny, in_dx, in_dy;
  double out_minx, out_miny, out_dx, out_dy;
  double x, y;
  
  cart_params_t *in_cart, *out_cart;

  if (first_call) {

    /*
     * initialize file indices
     */
    
    RfInitVolFileHandle(&in_index,
			Glob->prog_name,
			(char *) NULL,
			(FILE *) NULL);
    
    RfInitVolFileHandle(&out_index,
			Glob->prog_name,
			(char *) NULL,
			(FILE *) NULL);
    
    first_call = FALSE;

  }

  /*
   * read in radar volume
   */

  in_index.vol_file_path = file_path;

  if (RfReadVolume(&in_index, "remap_file") != R_SUCCESS)
    tidy_and_exit(-1);

  in_cart = &in_index.vol_params->cart;
  nplanes = in_cart->nz;
  nfields = in_index.vol_params->nfields;

  /*
   * allocate vol params as required
   */
  
  if (RfAllocVolParams(&out_index,
		       "remap_file") != R_SUCCESS)
    tidy_and_exit(-1);
  
  /*
   * copy in vol params
   */

  memcpy ((void *) out_index.vol_params,
          (void *) in_index.vol_params,
          (size_t) sizeof(vol_params_t));

  /*
   * append the note
   */

  sprintf(note, "\nFile composited to plane using '%s'",
	 Glob->prog_name);

  strncat(out_index.vol_params->note, note, 
	  (VOL_PARAMS_NOTE_LEN -
	   strlen(out_index.vol_params->note) - 1));
  
  /*
   * set cart params to indicate there is only 1 plane
   */

  out_cart = &out_index.vol_params->cart;

  out_cart->nx = Glob->nx;
  out_cart->ny = Glob->ny;
  out_cart->nz = 1;

  out_cart->scalex = out_cart->km_scalex;
  out_cart->scaley = out_cart->km_scaley;
  out_cart->scalez = out_cart->km_scalez;

  out_cart->minx =
    (long) floor(Glob->minx * out_cart->scalex + 0.5);

  out_cart->miny =
    (long) floor(Glob->miny * out_cart->scaley + 0.5);

  out_cart->minz =
    (long) floor((Glob->minz + Glob->maxz) * 0.5 * out_cart->scalez + 0.5);

  out_cart->dx =
    (long) floor(Glob->dx * out_cart->scalex + 0.5);

  out_cart->dy =
    (long) floor(Glob->dy * out_cart->scaley + 0.5);

  out_cart->dz =
    (long) floor((Glob->maxz - Glob->minz) * out_cart->scalez + 0.5);

  out_cart->dz_constant = TRUE;

  /*
   * allocate the arrays for the output file
   */

  if (RfAllocVolArrays(&out_index, "remap_file") != R_SUCCESS)
    tidy_and_exit(-1);
  
  /*
   * copy in the radar elevations array and set the plane heights
   * array
   */

  memcpy ((void *) out_index.radar_elevations,
          (void *) in_index.radar_elevations,
          (size_t) (out_index.vol_params->radar.nelevations * sizeof(si32)));

  out_index.plane_heights[0][0] = 
    (si32) floor(Glob->minz * out_cart->scalez + 0.5);

  out_index.plane_heights[0][1] = out_cart->minz;

  out_index.plane_heights[0][2] = 
    (si32) floor(Glob->maxz * out_cart->scalez + 0.5);

  /*
   * copy the field params
   */

  for (ifield = 0; ifield < nfields; ifield++)
    memcpy ((void *) out_index.field_params[ifield],
            (void *) in_index.field_params[ifield],
            (size_t) sizeof(field_params_t));

  /*
   * allocate the field planes
   */

  for (ifield = 0; ifield < nfields; ifield++) {
    
    out_index.field_plane[ifield][0] = (ui08 *) ucalloc
      ((ui32) (Glob->nx * Glob->ny),
       (ui32) sizeof(ui08));

  } /* ifield */

  /*
   * load up the field planes
   */

  in_miny = (double) in_cart->miny / (double) in_cart->scaley;
  in_minx = (double) in_cart->minx / (double) in_cart->scalex;
  in_dy = (double) in_cart->dy / (double) in_cart->scaley;
  in_dx = (double) in_cart->dx / (double) in_cart->scalex;

  out_miny = (double) out_cart->miny / (double) out_cart->scaley;
  out_minx = (double) out_cart->minx / (double) out_cart->scalex;
  out_dy = (double) out_cart->dy / (double) out_cart->scaley;
  out_dx = (double) out_cart->dx / (double) out_cart->scalex;

  start_plane = -1;
  end_plane = -1;

  for (iplane = 0; iplane < nplanes; iplane++) {
    
    if (Glob->minz * in_cart->scalez <= in_index.plane_heights[iplane][1] &&
	Glob->maxz * in_cart->scalez >= in_index.plane_heights[iplane][1]) {
	    
      if (start_plane == -1)
	start_plane = iplane;

      end_plane = iplane;

    }

  } /* iplane */

  if (start_plane < 0 || end_plane < 0) {

    fprintf(stderr, "ERROR - %s:remap_file\n", Glob->prog_name);
    fprintf(stderr, "Check minz ands maxz in params file %s\n",
	    Glob->params_path_name);
    tidy_and_exit(-1);

  }

  printf ("start plane, end plane = %ld, %ld\n",
	  start_plane, end_plane);

  for (ifield = 0; ifield < nfields; ifield++) {

    y = in_miny;

    for (iy = 0; iy < in_cart->ny; iy++) {

      jy = (long) ((y - out_miny) / out_dy + 0.5);

      x = in_minx;

      for (ix = 0; ix < in_cart->nx; ix++) {

	in_pos = iy * in_cart->nx + ix;
	
	jx = (long) ((x - out_minx) / out_dx + 0.5);

	out_pos = jy * out_cart->nx + jx;

	for (iplane = start_plane; iplane <= end_plane; iplane++) {

	  if (out_index.field_plane[ifield][0][out_pos] <
	      in_index.field_plane[ifield][iplane][in_pos]) {
	      
	    out_index.field_plane[ifield][0][out_pos] =
	      in_index.field_plane[ifield][iplane][in_pos];
	      
	      
	  }

	} /* iplane */

	x += in_dx;
	    
      } /* ix */
      
      y += in_dy;

    } /* iy */

  } /* ifield */

  /*
   * write the file
   */

  out_index.vol_file_path = file_path;

  if (RfWriteVolume(&out_index, "remap_file") != R_SUCCESS)
    tidy_and_exit(-1);

  /*
   * free up field planes
   */

  for (ifield = 0; ifield < nfields; ifield++) {
    
    ufree((char *) out_index.field_plane[ifield][0]);

  } /* ifield */
  
}
