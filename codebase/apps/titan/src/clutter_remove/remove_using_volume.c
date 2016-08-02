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
 * remove_using_volume()
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "clutter_remove.h"

/*
 * prototypes
 */

static void check_valid(vol_file_handle_t *v_handle,
			vol_file_handle_t *cindex);

static void perform_removal(vol_file_handle_t *v_handle,
			    vol_file_handle_t *cindex);

/*
 * main
 */

void remove_using_volume(char *input_file_path)

{

  static int first_call = TRUE;
  static vol_file_handle_t cindex;
  static vol_file_handle_t v_handle; 
  static LDATA_handle_t ldata_handle;
  
  char output_path[MAX_PATH_LEN];
  date_time_t dtime;

  if (first_call) {

    /*
     * initialize volume file handle
     */
    
    RfInitVolFileHandle(&v_handle,
			Glob->prog_name,
			NULL,
			(FILE *) NULL);
    
    /*
     * initialize clutter_table file handle
     */
    
    RfInitVolFileHandle(&cindex,
			Glob->prog_name,
			Glob->params.clutter_volume_file_path,
			(FILE *) NULL);
    
    /*
     * Initialize LDATA handle
     */

    LDATA_init_handle(&ldata_handle,
		      Glob->prog_name,
		      FALSE);
    
    /*
     * read in the clutter volume
     */
    
    if (RfReadVolume(&cindex, "remove_using_volume") != R_SUCCESS) {
      fprintf(stderr, "Cannot read in clutter volume\n");
      tidy_and_exit(-1);
    }

    first_call = FALSE;
    
  }
  
  /*
   * read in radar volume
   */

  v_handle.vol_file_path = input_file_path;
  if (RfReadVolume(&v_handle, "remove_using_volume") != R_SUCCESS)
    tidy_and_exit(-1);

  /*
   * check that the data in the files is consistent
   */
  
  check_valid(&v_handle, &cindex);

  /*
   * remove the clutter and write the output file
   */

  perform_removal(&v_handle, &cindex);

  /*
   * write the radar volume with clutter removed
   */

  Rfrtime2dtime(&v_handle.vol_params->mid_time, &dtime);

  sprintf(output_path, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	  Glob->params.output_dir, PATH_DELIM,
	  dtime.year, dtime.month, dtime.day,
	  PATH_DELIM, dtime.hour, dtime.min,
	  dtime.sec, Glob->params.output_file_ext);

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "Creating output file %s\n", output_path);
  }

  v_handle.vol_file_path = output_path;
  if (RfWriteVolume(&v_handle, "remove_using_volume") != R_SUCCESS)
    tidy_and_exit(-1);
  
  /*
   * Write the LDATA file
   */

  if (LDATA_info_write(&ldata_handle,
		       Glob->params.output_dir,
		       dtime.unix_time,
		       "mdv",
		       "none",
		       "none",
		       0,
		       (int *)NULL) != 0)
  {
    fprintf(stderr,
	    "Error writing LDATA file in directory <%s>\n",
	    Glob->params.output_dir);
  }
  
  return;
  
}

static void check_valid(vol_file_handle_t *v_handle,
			vol_file_handle_t *cindex)
     
{

  ui08 *clut_dbz_p;
  
  si32 npoints;
  si32 dbz_field = Glob->params.dbz_field;
  si32 ipoint, iplane;
  si32 nplanes;
  si32 dbz_byte_val;

  double clut_dbz;
  double clutter_scale, clutter_bias, vol_scale, vol_bias;
  double scale_diff, bias_diff;
  
  nplanes = v_handle->vol_params->cart.nz;
  npoints =
    v_handle->vol_params->cart.nx * v_handle->vol_params->cart.ny;

  /*
   * check that the dbz field set by the user is valid
   */

  if (Glob->params.dbz_field > v_handle->vol_params->nfields -1 ) {
    fprintf(stderr, "ERROR - %s:remove_using_volume.\n",
	    Glob->prog_name);
    fprintf(stderr, "Dbz_field %ld too high - only nfields of %ld.\n",
	    Glob->params.dbz_field,
	    (long) v_handle->vol_params->nfields);
    fprintf(stderr, "Field numbers start at 0\n");
    tidy_and_exit(-1);
  }

  /*
   * check the cartesian parameters
   */
  
  if (memcmp((void *) &v_handle->vol_params->cart,
	     (void *) &cindex->vol_params->cart,
	     (size_t) sizeof(cart_params_t))) {
      
    fprintf(stderr, "\nERROR - %s:remove_using_volume\n",
	    Glob->prog_name);
    fprintf(stderr,
	    "Cartesian params in volume do not match clutter volume.\n");
    fprintf(stderr, "Generate a new clutter volume.\n");
    tidy_and_exit(-1);
  }

  /*
   * check that the scale and bias of the dbz field matches that used
   * to generate the clutter table. If not, adjust the clutter volume
   * values accordingly.
   */
  
  clutter_scale = ((double) cindex->field_params[dbz_field]->scale /
		   (double) cindex->field_params[dbz_field]->factor);
  clutter_bias = ((double) cindex->field_params[dbz_field]->bias /
		  (double) cindex->field_params[dbz_field]->factor);
  
  vol_scale = ((double) v_handle->field_params[dbz_field]->scale /
		(double) v_handle->field_params[dbz_field]->factor);
  vol_bias = ((double) v_handle->field_params[dbz_field]->bias /
	       (double) v_handle->field_params[dbz_field]->factor);
  
  scale_diff = fabs(clutter_scale - vol_scale);
  bias_diff = fabs(clutter_bias - vol_bias);
  
  /*
   * if the scale or bias differ, adjust the clutter volume
   * to use the same scale as the data
   */
  
  if (scale_diff > 0.01 || bias_diff > 0.01) {

    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr,
	      "remove_using_volume: adjusting scale and bias\n");
      fprintf(stderr, "clutter scale, bias: %g, %g\n",
	      clutter_scale, clutter_bias);
      fprintf(stderr, "vol scale, bias: %g, %g\n",
	      vol_scale, vol_bias);
    }

    for (iplane = 0; iplane < nplanes; iplane++) {

      clut_dbz_p = cindex->field_plane[dbz_field][iplane];

      for (ipoint = 0; ipoint < npoints; ipoint++, clut_dbz_p++) {
	
	clut_dbz = ((*clut_dbz_p) * clutter_scale) + clutter_bias;

	dbz_byte_val =
	  (si32) (((clut_dbz - vol_bias) / vol_scale) + 0.5);
	
	if (dbz_byte_val < 0)
	  dbz_byte_val = 0;
	if (dbz_byte_val > 255)
	  dbz_byte_val = 255;

	if (Glob->params.debug >= DEBUG_EXTRA) {
	  if (*clut_dbz_p != 0) {
	    fprintf(stderr, "orig, dbz, final: %d, %g, %d\n",
		    *clut_dbz_p, clut_dbz, dbz_byte_val);
	  }
	}
	
	*clut_dbz_p = dbz_byte_val;

      } /* ipoint */
      
    } /* iplane */

    cindex->field_params[dbz_field]->scale =
      v_handle->field_params[dbz_field]->scale;
    
    cindex->field_params[dbz_field]->bias =
      v_handle->field_params[dbz_field]->bias;
    
    cindex->field_params[dbz_field]->factor =
      v_handle->field_params[dbz_field]->factor;
    
  } /* if (scale_diff....... */

  return;
  
}

static void perform_removal(vol_file_handle_t *v_handle,
			    vol_file_handle_t *cindex)
{

  char notebuf[BUFSIZ];

  ui08 *clut_dbz, *vol_dbz;

  si32 dbz_field = Glob->params.dbz_field;
  si32 ipoint, ifield, iplane;
  si32 nfields, npoints, nplanes;
  si32 dbz_scaled_margin, dbz_scaled_threshold;
  
  double dbz_scale, dbz_bias;

  field_params_t *dbz_fparams;

  /*
   * set some local variables
   */
  
  nfields = v_handle->vol_params->nfields;
  nplanes = v_handle->vol_params->cart.nz;
  npoints =
    v_handle->vol_params->cart.nx * v_handle->vol_params->cart.ny;

  /*
   * compute the dbz margin in terms of the byte value
   */

  dbz_fparams = cindex->field_params[dbz_field];
  
  dbz_bias =
    (double) dbz_fparams->bias / (double) dbz_fparams->factor;

  dbz_scale =
    (double) dbz_fparams->scale / (double) dbz_fparams->factor;

  dbz_scaled_margin = (si32) (Glob->params.dbz_margin / dbz_scale + 0.5);
  dbz_scaled_threshold =
    (si32) ((Glob->params.dbz_threshold - dbz_bias) / dbz_scale + 0.5);

  if (Glob->params.debug >= DEBUG_EXTRA) {
    fprintf(stderr, "dbz_scale, dbz_bias: %g, %g\n",
	    dbz_scale, dbz_bias);
    fprintf(stderr, "dbz_scaled_margin, dbz_scaled_threshold: %ld, %ld\n",
	    (long) dbz_scaled_margin, (long) dbz_scaled_threshold);
  }

  /*
   * Loop through planes, check points for clutter.
   */

  for (iplane = 0; iplane < nplanes; iplane++) {
    
    clut_dbz = cindex->field_plane[dbz_field][iplane];
    vol_dbz = v_handle->field_plane[dbz_field][iplane];
    
    for (ipoint = 0; ipoint < npoints;
	 ipoint++, clut_dbz++, vol_dbz++) {
      
      if (*vol_dbz > dbz_scaled_threshold) {
	
	if ((*vol_dbz - *clut_dbz) < dbz_scaled_margin) {

	  if (Glob->params.debug >= DEBUG_EXTRA) {
	    
	    int ix, iy;
	    double x, y;
	    double vdbz, cdbz;
	    cart_float_params_t fl_cart;
	    
	    RfDecodeCartParams(&v_handle->vol_params->cart, &fl_cart);

	    iy = ipoint / fl_cart.nx;
	    ix = ipoint - iy * fl_cart.nx;

	    x = ix * fl_cart.dx + fl_cart.minx;
	    y = iy * fl_cart.dy + fl_cart.miny;

	    vdbz = *vol_dbz * dbz_scale + dbz_bias;
	    cdbz = *clut_dbz * dbz_scale + dbz_bias;

	    fprintf(stderr, "x,y,vol_dbz,clut_dbz: %g, %g, %g, %g\n",
		    x, y, vdbz, cdbz);

	  }
	  
	  /*
	   * clutter point - remove all field data for
	   * this point
	   */
	  
	  for (ifield = 0; ifield < nfields; ifield++) {
	    v_handle->field_plane[ifield][iplane][ipoint] = 0;
	  }

	} /* if ((*vol_dbz - *clut_dbz) < dbz_scaled_margin) */

      } /* if (*vol_dbz > dbz_scaled_threshold) */

    } /* ipoint */

  } /* iplane */

  /*
   * append a message to the file note, indicating that the clutter
   * has been removed, giving details
   */
  
  sprintf(notebuf,
	  "Clutter removed\n"
	  "  dbz_margin %g dBZ\n"
	  "  dbz_threshold %g dBZ\n"
	  "  clutter_volume_file '%s'\n",
	  Glob->params.dbz_margin,
	  Glob->params.dbz_threshold,
	  Glob->params.clutter_volume_file_path);
  
  strncat(v_handle->vol_params->note,
	  notebuf,
	  (int) VOL_PARAMS_NOTE_LEN - strlen(v_handle->vol_params->note));

}
