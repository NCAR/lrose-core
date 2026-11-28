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
 * perform_smoothing()
 *
 * process track file
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1993
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "smooth_mdv.h"

static ui08 compute_mean(ui08 *sample, si32 nsamples);

static ui08 compute_median(ui08 *sample,
			     si32 nsamples, si32 mid_pt);

static int compare_data();

void perform_smoothing(vol_file_handle_t *v_handle)

{

  ui08 *filtered, *fil_p;
  ui08 *data_plane, *data_p;
  ui08 *sample, *sample_p;
  
  si32 i, ix, iy;
  si32 half;
  si32 ifield, nfields_smooth;
  si32 field_num;
  si32 nsamples, npoints;
  si32 mid_pt;
  si32 iz;
  si32 index;
  si32 *offset, *offset_p;

  cart_params_t *cart;

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "Performing smoothing\n");
  }

  /*
   * initialize and allocate
   */

  half = Glob->params.kernel_size / 2;
  cart = &v_handle->vol_params->cart;

  npoints = cart->ny * cart->nx;
  
  filtered = (ui08 *) umalloc ((ui32) npoints * sizeof(ui08));

  nfields_smooth = Glob->params.smoothed_fields.len;

  /*
   * set up offset array
   */

  nsamples = Glob->params.kernel_size * Glob->params.kernel_size;
  mid_pt = nsamples / 2;
  
  sample = (ui08 *) umalloc (nsamples * sizeof(ui08));
  offset = (si32 *) umalloc (nsamples * sizeof(si32));

  offset_p = offset;
  
  for (iy = 0; iy < Glob->params.kernel_size; iy++) {
    for (ix = 0; ix < Glob->params.kernel_size; ix++) {
      *offset_p = (iy - half) * cart->nx + (ix - half);
      offset_p++;
    } /* ix */
  } /* iy */
  
  /*
   * loop through fields
   */

  for (ifield = 0; ifield < nfields_smooth; ifield++) {

    field_num = Glob->params.smoothed_fields.val[ifield];
    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf(stderr, "Smoothing field %d\n", field_num);
    }

    /*
     * loop through planes
     */

    for (iz = 0; iz < cart->nz; iz++) {

      data_plane = v_handle->field_plane[field_num][iz];
      
      /*
       * clear filtered array
       */
      
      memset(filtered, 0, cart->ny * cart->nx);

      /*
       * loop through points
       */
      
      for (iy = half; iy < cart->ny - half; iy++) {
	
	index = iy * cart->nx + half;

	for (ix = half; ix < cart->nx - half; ix++) {

	  /*
	   * get index to point
	   */
	  
	  data_p = data_plane + index;
	  fil_p = filtered + index;
	  
	  /*
	   * load up sample array
	   */
	  
	  sample_p = sample;
	  offset_p = offset;
	  
	  for (i = 0; i < nsamples; i++) {
	    
	    *sample_p = *(data_p + *offset_p);
	    offset_p++;
	    sample_p++;
	    
	  } /* i */

	  /*
	   * compute smoothed value, load into filtered array
	   */

	  if (Glob->params.method == MEAN) {
	    *fil_p = compute_mean(sample, nsamples);
	  } else {
	    *fil_p = compute_median(sample, nsamples, mid_pt);
	  }

	  index++;
	  
	} /* ix */
	
      } /* iy */

      /*
       * copy the filtered array in
       */

      memcpy(data_plane, filtered, npoints);

    } /* iz */

  } /* ifield */
  
}
      
/********************************************************
 * compute_mean()
 */

static ui08 compute_mean(ui08 *sample, si32 nsamples)

{

  ui08 *sample_p;
  si32 i;
  double sum = 0.0;
  double mean;

  sample_p = sample;

  for (i = 0; i < nsamples; i++){
    sum += (double) *sample_p;
    sample_p++;
  }

  mean = sum / (double) nsamples;
  
  if (mean < 0.0)
    mean = 0.0;
  
  if (mean > 127.0)
    mean = 127.0;

  return (ui08) (mean + 0.5);

}


/********************************************************
 * compute_median()
 */

static ui08 compute_median(ui08 *sample,
			     si32 nsamples, si32 mid_pt)

{

  qsort(sample, nsamples, sizeof(ui08),
	(int (*)())compare_data);
      
  return (ui08) (*(sample + mid_pt));


}


/*********************************************************
 * define function to be used for sorting values
 */

static int compare_data(u1, u2)

     ui08 *u1, *u2;

{

  return ((int) *u1 - (int) *u2);

}








