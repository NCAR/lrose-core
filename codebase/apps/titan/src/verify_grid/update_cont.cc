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
/************************************************************************
 * update_cont.c
 *
 * Performs verification on the file - updates contingency table data
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * July 1992
 *
 ***********************************************************************/

#include "verify_grid.h"

static void cont_specified_grid(contingency_t *cont,
				vol_file_handle_t *truth_index,
				vol_file_handle_t *detect_index,
				FILE* scan_cont_file);

static void cont_native_grid(contingency_t *cont,
			     vol_file_handle_t *truth_index,
			     vol_file_handle_t *detect_index,
			     FILE* scan_cont_file);

static void cont_relaxed(contingency_t *cont,
			 vol_file_handle_t *truth_index,
			 vol_file_handle_t *detect_index);

static FILE *get_cont_file(vol_file_handle_t *vol_index);

void update_cont(char *detect_file_path,
		 contingency_t *cont)

{
  
  static vol_file_handle_t truth_index;
  static vol_file_handle_t detect_index;
  static int first_call = TRUE;
  
  char *truth_file_path;
  double refl_threshold;

  cart_params_t *truth_cart;
  cart_params_t *detect_cart;

  FILE *scan_cont_file = NULL;

  /*
   * initialize
   */
  
  if (first_call) {
    init_indices(&truth_index, &detect_index);
    first_call = FALSE;
  } /* if (first_call) */

  /*
   * get the file path for the truth data - if this returns
   * error, there is no truth data within the time margin of
   * the detect data, so return early
   */

  if ((truth_file_path = get_truth_path(detect_file_path)) == NULL)
    return;

  if (Glob->params.debug >= DEBUG_NORM) {

    fprintf(stderr, "Detection file '%s'\n", detect_file_path);
    fprintf(stderr, "Truth     file '%s'\n", truth_file_path);

  }

  /*
   * read in the data from the truth and detection files
   */

  detect_index.vol_file_path = detect_file_path;

  if (RfReadVolume(&detect_index, "update_cont") != R_SUCCESS)
    tidy_and_exit(-1);

  truth_index.vol_file_path = truth_file_path;
  
  if (RfReadVolume(&truth_index, "update_cont") != R_SUCCESS)
    tidy_and_exit(-1);

  /*
   * get the cont file based on the detect file
   */

  if (Glob->params.output_scan_cont) {
    scan_cont_file = get_cont_file(&detect_index);
  }

  /*
   * check that the cartesian grids match.  This does not need to
   * be done if we are using specified grids and are not relaxing
   * the contingnecy.
   */

  truth_cart = &truth_index.vol_params->cart;
  detect_cart = &detect_index.vol_params->cart;

  if ((Glob->params.relaxed_contingency ||
       Glob->params.use_native_grid) &&
      (truth_cart->nx != detect_cart->nx ||
       truth_cart->ny != detect_cart->ny)) {
    
    fprintf(stderr, "ERROR - %s:update_cont\n", Glob->prog_name);
    fprintf(stderr, "Cartesian grids do not match\n\n");

    fprintf(stderr, "\nFile '%s'\n", detect_file_path);
    print_cart_params(&detect_index.vol_params->cart);

    fprintf(stderr, "\nFile '%s'\n", truth_file_path);
    print_cart_params(&truth_index.vol_params->cart);

    tidy_and_exit(-1);

  }

  /*
   * check that there is only 1 plane
   */

  if (truth_cart->nz != 1) {

    fprintf(stderr, "ERROR - %s:update_cont\n", Glob->prog_name);
    fprintf(stderr, "Truth data has %ld planes - should be 1\n",
	    (long) truth_cart->nz);
    tidy_and_exit(-1);

  }

  if (detect_cart->nz != 1) {

    fprintf(stderr, "ERROR - %s:update_cont\n", Glob->prog_name);
    fprintf(stderr, "Detection data has %ld planes - should be 1\n",
	    (long) detect_cart->nz);
    tidy_and_exit(-1);
    
  }

  /*
   * check that field number is valid
   */

  if (Glob->params.truth_field > truth_index.vol_params->nfields - 1) {

    fprintf(stderr, "ERROR - %s:update_cont\n", Glob->prog_name);
    fprintf(stderr, "Truth data field %ld too large\n",
	    Glob->params.truth_field);
    fprintf(stderr, "Max allowed is %ld\n",
	    (long) (truth_index.vol_params->nfields - 1));
    tidy_and_exit(-1);

  }

  if (Glob->params.detect_field > detect_index.vol_params->nfields - 1) {

    fprintf(stderr, "ERROR - %s:update_cont\n", Glob->prog_name);
    fprintf(stderr, "Detect data field %ld too large\n",
	    Glob->params.detect_field);
    fprintf(stderr, "Max allowed is %ld\n",
	    (long) (detect_index.vol_params->nfields - 1));
    tidy_and_exit(-1);

  }

  /*
   * if required, print out start of line for scan-by-scan
   * contingency data
   */

  if (Glob->params.output_scan_cont) {

    refl_threshold = parse_threshold(&detect_index);
    
    fprintf(scan_cont_file, "%ld %ld %ld %ld %ld %ld %g %g ",
	    (long) detect_index.vol_params->mid_time.year,
	    (long) detect_index.vol_params->mid_time.month,
	    (long) detect_index.vol_params->mid_time.day,
	    (long) detect_index.vol_params->mid_time.hour,
	    (long) detect_index.vol_params->mid_time.min,
	    (long) detect_index.vol_params->mid_time.sec,
	    (double) Glob->params.time_lag / 60.0,
	    refl_threshold);

    fflush(scan_cont_file);

  }

  
  if (Glob->params.relaxed_contingency) {

    cont_relaxed(cont, &truth_index, &detect_index);

  } else {

    if (Glob->params.use_native_grid) {
      cont_native_grid(cont, &truth_index, &detect_index, scan_cont_file);
    } else {
      cont_specified_grid(cont, &truth_index, &detect_index, scan_cont_file);
    }

  }

  if (Glob->params.output_scan_cont) {
    fprintf(scan_cont_file, "\n");
  }

}

/********************************************************************
 * cont_specified_grid()
 */

static void cont_specified_grid(contingency_t *cont,
				vol_file_handle_t *truth_index,
				vol_file_handle_t *detect_index,
				FILE* scan_cont_file)

{
  static const char *routine_name = "cont_specified_grid";
  
  static int first_call = TRUE;
  static ui08 *truth_grid, *detect_grid;
  static si32 npoints_grid;

  ui08 *truth_plane, *detect_plane;
  ui08 *d, *t;
  ui08 *truth_grid_ptr, *detect_grid_ptr;
  
  int truth, detect;
  int accept;
  int truth_byte_upper, truth_byte_lower;
  int detect_byte_upper, detect_byte_lower;
  int truth_nx, truth_ny;
  int detect_nx, detect_ny;
  int truth_ix, truth_iy, truth_i;
  int detect_ix, detect_iy, detect_i;
  
  si32 i, ix, iy;
  si32 n_detect = 0;
  si32 n_truth = 0;
  si32 n_success = 0;
  si32 n_failure = 0;
  si32 n_false_alarm = 0;
  si32 n_non_event = 0;

  double truth_scale, truth_bias;
  double detect_scale, detect_bias;
  double x, y, range;
  double truth_dx, truth_dy, truth_minx, truth_miny;
  double detect_dx, detect_dy, detect_minx, detect_miny;
  double rx, ry;
  double radarx, radary;

  cart_params_t *truth_cart;
  cart_params_t *detect_cart;

  field_params_t *truth_field;
  field_params_t *detect_field;

  /*
   * alloc
   */
  
  if (first_call) {
  
    npoints_grid = Glob->params.grid.ny * Glob->params.grid.nx;
    
    truth_grid = (ui08 *) umalloc
      ((ui32) (npoints_grid * sizeof(ui08)));
    
    detect_grid = (ui08 *) umalloc
      ((ui32) (npoints_grid * sizeof(ui08)));
    
    first_call = FALSE;
    
  } /* if (first_call) */

  /*
   * zero out grids
   */

  memset(truth_grid, 0, npoints_grid * sizeof(ui08));
  memset(detect_grid, 0, npoints_grid * sizeof(ui08));
  
  /*
   * initialize constants
   */

  truth_cart = &truth_index->vol_params->cart;

  truth_field = truth_index->field_params[Glob->params.truth_field];
  truth_scale =
    (double) truth_field->scale / (double) truth_field->factor;
  truth_bias =
    (double) truth_field->bias / (double) truth_field->factor;

  detect_cart = &detect_index->vol_params->cart;

  detect_field = detect_index->field_params[Glob->params.detect_field];
  detect_scale =
    (double) detect_field->scale / (double) detect_field->factor;
  detect_bias =
    (double) detect_field->bias / (double) detect_field->factor;

  truth_byte_upper =
    (int) ((Glob->params.truth_level_upper - truth_bias) /
	   truth_scale + 0.5);
  truth_byte_lower =
    (int) ((Glob->params.truth_level_lower - truth_bias) /
	   truth_scale + 0.5);

  detect_byte_upper =
    (int) ((Glob->params.detect_level_upper - detect_bias) /
	   detect_scale + 0.5);
  detect_byte_lower =
    (int) ((Glob->params.detect_level_lower - detect_bias) /
	   detect_scale + 0.5);

  truth_dx = (double) truth_cart->dx / (double) truth_cart->scalex;
  truth_dy = (double) truth_cart->dy / (double) truth_cart->scaley;
  truth_minx = (double) truth_cart->minx / (double) truth_cart->scalex;
  truth_miny = (double) truth_cart->miny / (double) truth_cart->scaley;
  truth_nx = truth_cart->nx;
  truth_ny = truth_cart->ny;
  
  detect_dx = (double) detect_cart->dx / (double) detect_cart->scalex;
  detect_dy = (double) detect_cart->dy / (double) detect_cart->scaley;
  detect_minx = (double) detect_cart->minx / (double) detect_cart->scalex;
  detect_miny = (double) detect_cart->miny / (double) detect_cart->scaley;
  detect_nx = detect_cart->nx;
  detect_ny = detect_cart->ny;
  
  radarx = (double) truth_cart->radarx / (double) truth_cart->scalex;
  radary = (double) truth_cart->radary / (double) truth_cart->scaley;

  /*
   * load up grids
   */

  truth_plane = truth_index->field_plane[Glob->params.truth_field][0];
  detect_plane = detect_index->field_plane[Glob->params.detect_field][0];
  
  /*
   * Fill in the truth and detect grids by going through the specified
   * contingency grid and filling it with the truth/detect flag for the
   * truth/detect grid square containing the center of the contingency
   * grid square.
   */

  y = Glob->params.grid.miny;
  ry = y - radary;

  truth_grid_ptr = truth_grid;
  detect_grid_ptr = detect_grid;
  
  for (iy = 0; iy < Glob->params.grid.ny;
       iy++, y += Glob->params.grid.dy, ry += Glob->params.grid.dy) {
    
    x = Glob->params.grid.minx;
    rx = x - radarx;
    
    for (ix = 0; ix < Glob->params.grid.nx;
	 ix++, x += Glob->params.grid.dx, rx += Glob->params.grid.dx) {

      if (Glob->params.check_range) {
	range = sqrt(rx * rx + ry * ry);
	if (range <= Glob->params.max_range) {
	  accept = TRUE;
	} else {
	  accept = FALSE;
	}
      } else {
	accept = TRUE;
      }

      if (accept) {
 
	truth_ix = (int)(((x - truth_minx) / truth_dx) + 0.5);
	truth_iy = (int)(((y - truth_miny) / truth_dy) + 0.5);
	truth_i = truth_iy * truth_nx + truth_ix;
	
	if (truth_ix >= 0 && truth_ix < truth_nx &&
	    truth_iy >= 0 && truth_iy < truth_ny &&
	    truth_plane[truth_i] >= truth_byte_lower &&
	    truth_plane[truth_i] <= truth_byte_upper) {
	  *truth_grid_ptr = 1;
	}

	detect_ix = (int)(((x - detect_minx) / detect_dx) + 0.5);
	detect_iy = (int)(((y - detect_miny) / detect_dy) + 0.5);
	detect_i = detect_iy * detect_nx + detect_ix;
	
	if (detect_ix >= 0 && detect_ix < detect_nx &&
	    detect_iy >= 0 && detect_iy < detect_ny &&
	    detect_plane[detect_i] >= detect_byte_lower &&
	    detect_plane[detect_i] <= detect_byte_upper) {
	  *detect_grid_ptr = 1;
	}

      } /* if (accept) */

      truth_grid_ptr++;
      detect_grid_ptr++;

    } /* ix */

  } /* iy */

  /*
   * accumulate contingency data
   */

  t = truth_grid;
  d = detect_grid;
  
  for (i = 0; i < npoints_grid; i++, t++, d++) {

    if (*t) {
      truth = TRUE;
      n_truth++;
    } else {
      truth = FALSE;
    }

    if (*d) {
      detect = TRUE;
      n_detect++;
    } else {
      detect = FALSE;
    }

    if (truth && detect) {
      n_success++;
    } else if (truth && !detect) {
      n_failure++;
    } else if (!truth && detect) {
      n_false_alarm++;
    } else {
      n_non_event++;
    }  /* if (truth ... */
    
  } /* i */

  cont->n_detect += n_detect;
  cont->n_truth += n_truth;
  cont->n_success += n_success;
  cont->n_failure += n_failure;
  cont->n_false_alarm += n_false_alarm;
  cont->n_non_event += n_non_event;

  if (Glob->params.output_scan_cont) {
    fprintf(scan_cont_file, "%ld %ld %ld %ld %ld %ld",
	    (long) n_truth, (long) n_detect,
	    (long) n_success, (long) n_failure,
	    (long) n_false_alarm, (long) n_non_event);
  }

  if (Glob->params.output_intermediate_grids)
  {
    MDV_master_header_t *master_hdr = Glob->inter_dataset.master_hdr;
    
    UTIMstruct time_struct;
    char output_directory[MAX_PATH_LEN];
    char output_filename[MAX_PATH_LEN];
    
    /*
     * Fill in master header information.
     */

    master_hdr->time_gen = time(NULL);
    master_hdr->time_begin =
      Rfrtime2utime(&truth_index->vol_params->start_time);
    master_hdr->time_end =
      Rfrtime2utime(&truth_index->vol_params->end_time);
    master_hdr->time_centroid =
      Rfrtime2utime(&truth_index->vol_params->mid_time);
    master_hdr->time_expire = master_hdr->time_end;
    
    /*
     * Set the data pointers.
     */

    Glob->inter_dataset.field_plane[0][0] = truth_grid;
    Glob->inter_dataset.field_plane[1][0] = detect_grid;
    
    /*
     * Create the output file.
     */

    UTIMunix_to_date(master_hdr->time_centroid, &time_struct);
    
    sprintf(output_directory,
	    "%s/%04ld%02ld%02ld",
	    Glob->params.intermediate_dir,
	    time_struct.year,
	    time_struct.month,
	    time_struct.day);
    sprintf(output_filename,
	    "%02ld%02ld%02ld.mdv",
	    time_struct.hour,
	    time_struct.min,
	    time_struct.sec);
    
    if (MDV_write_dataset_remote(&Glob->inter_dataset,
				 MDV_PLANE_RLE8,
				 FALSE,
				 "local",
				 output_directory,
				 output_filename,
				 "./tmp") != MDV_SUCCESS)
    {
      fprintf(stderr, "ERROR - %s:%s\n",
	      Glob->prog_name, routine_name);
      fprintf(stderr, "Error writing intermediate grids to MDV file <%s/%s>\n",
	      output_directory, output_filename);
    }
    
  } /* endif - Glob->params.output_intermediate_grids */
  
}

/********************************************************************
 * cont_native_grid()
 */

static void cont_native_grid(contingency_t *cont,
			     vol_file_handle_t *truth_index,
			     vol_file_handle_t *detect_index,
			     FILE* scan_cont_file)

{

  ui08 *truth_plane, *detect_plane;

  int truth, detect;

  si32 ix, iy;
  si32 n_detect = 0;
  si32 n_truth = 0;
  si32 n_success = 0;
  si32 n_failure = 0;
  si32 n_false_alarm = 0;
  si32 n_non_event = 0;

  double truth_scale, truth_bias;
  double detect_scale, detect_bias;
  double truth_val, detect_val;
  double x, y, dx, dy, minx, miny, range;
  double radarx, radary;

  cart_params_t *truth_cart;

  field_params_t *truth_field;
  field_params_t *detect_field;

  /*
   * accumulate contingency data
   */

  truth_cart = &truth_index->vol_params->cart;

  truth_field = truth_index->field_params[Glob->params.truth_field];
  truth_scale =
    (double) truth_field->scale / (double) truth_field->factor;
  truth_bias =
    (double) truth_field->bias / (double) truth_field->factor;

  detect_field = detect_index->field_params[Glob->params.detect_field];
  detect_scale =
    (double) detect_field->scale / (double) detect_field->factor;
  detect_bias =
    (double) detect_field->bias / (double) detect_field->factor;
  
  truth_plane = truth_index->field_plane[Glob->params.truth_field][0];
  detect_plane = detect_index->field_plane[Glob->params.detect_field][0];
  
  dx = (double) truth_cart->dx / (double) truth_cart->scalex;
  dy = (double) truth_cart->dy / (double) truth_cart->scaley;
  minx = (double) truth_cart->minx / (double) truth_cart->scalex;
  miny = (double) truth_cart->miny / (double) truth_cart->scaley;
  radarx = (double) truth_cart->radarx / (double) truth_cart->scalex;
  radary = (double) truth_cart->radary / (double) truth_cart->scaley;

  y = miny - radary;

  for (iy = 0; iy < truth_cart->ny; iy++) {

    x = minx - radarx;

    for (ix = 0; ix < truth_cart->nx; ix++) {

      range = sqrt(x * x + y * y);

      if (!Glob->params.check_range ||
	  range <= Glob->params.max_range) {
 
	truth_val = (double) *truth_plane * truth_scale + truth_bias;
	detect_val = (double) *detect_plane * detect_scale + detect_bias;
	
	if (truth_val >= Glob->params.truth_level_lower &&
	    truth_val <= Glob->params.truth_level_upper) {
	  truth = TRUE;
	  n_truth++;
	} else {
	  truth = FALSE;
	}

	if (detect_val >= Glob->params.detect_level_lower &&
	    detect_val <= Glob->params.detect_level_upper) {
	  detect = TRUE;
	  n_detect++;
	} else {
	  detect = FALSE;
	}

	if (truth && detect) {
	  
	  n_success++;
	  
	} else if (truth && !detect) {
	  
	  n_failure++;
	  
	} else if (!truth && detect) {
	  
	  n_false_alarm++;

	} else {

	  n_non_event++;
	  
	}  /* if (truth ... */

      } /* if (range ... */

      truth_plane++;
      detect_plane++;
      x += dx;

    } /* ix */

    y += dy;

  } /* iy */

  cont->n_detect += n_detect;
  cont->n_truth += n_truth;
  cont->n_success += n_success;
  cont->n_failure += n_failure;
  cont->n_false_alarm += n_false_alarm;
  cont->n_non_event += n_non_event;

  if (Glob->params.output_scan_cont) {
    fprintf(scan_cont_file, "%ld %ld %ld %ld %ld %ld",
	    (long) n_truth, (long) n_detect,
	    (long) n_success, (long) n_failure,
	    (long) n_false_alarm, (long) n_non_event);
  }

}

/********************************************************************
 * cont_relaxed()
 */

static void cont_relaxed(contingency_t *cont,
			 vol_file_handle_t *truth_index,
			 vol_file_handle_t *detect_index)

{

  ui08 *truth_plane, *detect_plane;
  ui08 **truth, **detect;
  ui08 *p_truth, *p_detect;

  si32 ix, iy;
  si32 jx, jy;
  si32 max_truth, n_truth;
  si32 startx, endx;
  si32 starty, endy;
  si32 margin;

  double truth_scale, truth_bias;
  double detect_scale, detect_bias;
  double truth_val, detect_val;
  double x, y, dx, dy, minx, miny, range;
  double radarx, radary;

  cart_params_t *truth_cart;

  field_params_t *truth_field;
  field_params_t *detect_field;

  /*
   * accumulate contingency data
   */

  margin = Glob->params.relaxed_margin;

  truth_cart = &truth_index->vol_params->cart;

  truth_field = truth_index->field_params[Glob->params.truth_field];
  truth_scale =
    (double) truth_field->scale / (double) truth_field->factor;
  truth_bias =
    (double) truth_field->bias / (double) truth_field->factor;

  detect_field = detect_index->field_params[Glob->params.detect_field];
  detect_scale =
    (double) detect_field->scale / (double) detect_field->factor;
  detect_bias =
    (double) detect_field->bias / (double) detect_field->factor;
  
  dx = (double) truth_cart->dx / (double) truth_cart->scalex;
  dy = (double) truth_cart->dy / (double) truth_cart->scaley;
  minx = (double) truth_cart->minx / (double) truth_cart->scalex;
  miny = (double) truth_cart->miny / (double) truth_cart->scaley;
  radarx = (double) truth_cart->radarx / (double) truth_cart->scalex;
  radary = (double) truth_cart->radary / (double) truth_cart->scaley;

  truth = (ui08 **) ucalloc2
    ((ui32) truth_cart->ny,
     (ui32) truth_cart->nx,
     (ui32) sizeof(ui08));

  detect = (ui08 **) ucalloc2
    ((ui32) truth_cart->ny,
     (ui32) truth_cart->nx,
     (ui32) sizeof(ui08));

  p_truth = *truth;
  p_detect = *detect;

  truth_plane = truth_index->field_plane[Glob->params.truth_field][0];
  detect_plane = detect_index->field_plane[Glob->params.detect_field][0];
  
  y = miny - radary;

  for (iy = 0; iy < truth_cart->ny; iy++) {

    x = minx - radarx;

    for (ix = 0; ix < truth_cart->nx; ix++) {

      range = sqrt(x * x + y * y);

      if (!Glob->params.check_range ||
	  range <= Glob->params.max_range) {
 
	truth_val = (double) *truth_plane * truth_scale + truth_bias;
	detect_val = (double) *detect_plane * detect_scale + detect_bias;
	
	if (truth_val >= Glob->params.truth_level_lower &&
	    truth_val <= Glob->params.truth_level_upper)
	  *p_truth = TRUE;

	if (detect_val >= Glob->params.detect_level_lower &&
	    detect_val <= Glob->params.detect_level_upper)
	  *p_detect = TRUE;

      }

      p_truth++;
      p_detect++;
      truth_plane++;
      detect_plane++;

      x += dx;

    } /* ix */

    y += dy;

  } /* iy */

  startx = margin;
  endx = truth_cart->nx - margin - 1;

  starty = margin;
  endy = truth_cart->ny - margin - 1;

  max_truth = (1 + 2 * margin) * (1 + 2 * margin);

  for (iy = starty; iy <= endy; iy++) {

    p_detect = detect[iy] + startx;

    for (ix = startx; ix <= endx; ix++) {

      n_truth = 0;
      
      for (jy = iy - margin; jy <= iy + margin; jy++)
	for (jx = ix - margin; jx <= ix + margin; jx++)
	  n_truth += truth[jy][jx];

      if (n_truth > 0) {
	cont->n_truth++;
      }

      if (*p_detect > 0) {
	cont->n_detect++;
      }

      if (n_truth > 0 && *p_detect) {
	
	cont->n_success++;
	
      } else if (n_truth == max_truth && !*p_detect) {
	
	cont->n_failure++;
	
      } else if (n_truth  == 0 && *p_detect) {
	
	cont->n_false_alarm++;
	
      }  /* if (truth ... */
      
      p_detect++;

    } /* ix */
  
    y += dy;
  
  } /* iy */

  ufree2((void **) truth);
  ufree2((void **) detect);

}

static FILE *get_cont_file(vol_file_handle_t *vol_index)

{
  
  static FILE *scan_cont_file = NULL;
  static long file_date = 0;
  long vol_date;
  char file_path[MAX_PATH_LEN];

  /*
   * compute vol date
   */

  vol_date = (vol_index->vol_params->mid_time.year * 10000 +
	      vol_index->vol_params->mid_time.month * 100 +
	      vol_index->vol_params->mid_time.day);

  if (vol_date != file_date) {

    if (scan_cont_file == NULL) {
      fclose(scan_cont_file);
      scan_cont_file = NULL;
    }

    file_date =  vol_date;
    sprintf(file_path, "%s%s%ld.%s",
	    Glob->params.scan_cont_dir, PATH_DELIM,
	    vol_date, Glob->params.scan_cont_ext);

    if ((scan_cont_file =
	 fopen(file_path, "w")) == NULL) {
      fprintf(stderr, "ERROR - %s:update_cont\n", Glob->prog_name);
      fprintf(stderr, "Cannot open scan stats file for writing\n");
      perror(file_path);
      tidy_and_exit(-1);
    }
    
    fprintf(scan_cont_file,
	    "year month day hour min sec "
	    "lead_time(mins) threshold(dBZ) "
	    "n_measured n_forecast n_success "
	    "n_failure n_false_alarm n_non_event"
	    "\n");

  }
  
  return (scan_cont_file);

}


