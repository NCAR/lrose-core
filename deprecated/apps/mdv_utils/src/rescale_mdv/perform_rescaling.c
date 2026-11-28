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
 * perform_rescaling()
 *
 * Rescale given file.
 *
 * Returns 0 on success, -1 on failure.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rescale_mdv.h"

int perform_rescaling(char *file_path)

{

  static int first_call = TRUE;
  static vol_file_handle_t v_handle;

  ui08 lut[256];
  ui08 *d;
  
  int iz, i;
  int final_byte;
  int nbytes_per_plane;

  double orig_scale, orig_bias;
  double final_scale, final_bias;
  double float_val;

  field_params_t *fparams;
  cart_params_t *cart;

  fprintf(stdout, "Rescaling file %s\n", file_path);
  fflush(stdout);

  if (first_call) {
    
    /*
     * initialize index
     */
    
    RfInitVolFileHandle(&v_handle,
			Glob->prog_name,
			(char *) NULL,
			(FILE *) NULL);

    first_call = FALSE;

  }
  
  /*
   * read in the file
   */

  fprintf(stderr, "111111\n");

  umalloc_verify();

  fprintf(stderr, "222222\n");

  v_handle.vol_file_path = file_path;
  fprintf(stderr, "25252525252\n");
  if (RfReadVolume(&v_handle, "perform_rescaling") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:perform_rescaling\n", Glob->prog_name);
    fprintf(stderr, "Cannot read in file %s.\n", file_path);
    return(-1);
  }

  fprintf(stderr, "333333\n");

  umalloc_verify();

  fprintf(stderr, "444444\n");

  /*
   * compute look-up table
   */

  fparams = v_handle.field_params[Glob->params.field_num];
  orig_scale = (double) fparams->scale / (double) fparams->factor;
  orig_bias = (double) fparams->bias / (double) fparams->factor;
  final_scale = Glob->params.output_scale;
  final_bias = Glob->params.output_bias;
  
  memset(lut, 0, 256);
  
  for (i = 2; i < 256; i++) {
    float_val = orig_bias + (double) i * orig_scale;
    final_byte = (int) (((float_val - final_bias) / final_scale) + 0.5);
    if (final_byte < 0) {
      final_byte = 0;
    }
    if (final_byte > 255) {
      final_byte = 255;
    }
    lut[i] = (ui08) final_byte;
  }

  /*
   * rescale the data in the chosen field
   */

  cart = &v_handle.vol_params->cart;
  nbytes_per_plane = cart->nx * cart->ny;

  for (iz = 0; iz < cart->nz; iz++) {

    d = v_handle.field_plane[Glob->params.field_num][iz];

    for (i = 0; i < nbytes_per_plane; i++, d++) {
      *d = lut[*d];
    }

  } /* iz */

  /*
   * amend the field params
   */

  fparams->scale = (int) floor(final_scale * fparams->factor + 0.5);
  fparams->bias = (int) floor(final_bias * fparams->factor + 0.5);
  
  /*
   * write the file
   */
  
  if (RfWriteVolume(&v_handle, "perform_rescaling") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:perform_rescaling\n", Glob->prog_name);
    fprintf(stderr, "Cannot write out file %s.\n", file_path);
    return(-1);
  }

  return 0;

}

