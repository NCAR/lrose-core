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
 * remove_using_table()
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

static void check_geom(vol_file_handle_t *v_handle,
		       clutter_table_file_handle_t *clutter_handle);

static void perform_removal(vol_file_handle_t *v_handle,
			    clutter_table_file_handle_t *clutter_handle);

/*
 * main
 */

void remove_using_table(char *input_file_path)

{

  static int first_call = TRUE;
  static clutter_table_file_handle_t clutter_handle;
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
    
    RfInitClutterHandle(&clutter_handle,
			Glob->prog_name,
			Glob->params.clutter_table_file_path,
			(FILE *) NULL);
    
    /*
     * Initialize LDATA handle
     */

    LDATA_init_handle(&ldata_handle,
		      Glob->prog_name,
		      FALSE);
    
    /*
     * read in the clutter table
     */
    
    if (RfReadClutterTable(&clutter_handle,
			   "remove_using_table") != R_SUCCESS)
      tidy_and_exit(-1);
    
    first_call = FALSE;
    
  }

  /*
   * read in radar volume
   */

  v_handle.vol_file_path = input_file_path;
  if (RfReadVolume(&v_handle, "remove_using_table") != R_SUCCESS)
    tidy_and_exit(-1);

  /*
   * check that the geometry in the files is consistent
   */
  
  check_geom(&v_handle, &clutter_handle);

  /*
   * remove the clutter and write the output file
   */

  perform_removal(&v_handle, &clutter_handle);

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
  if (RfWriteVolume(&v_handle, "remove_using_table") != R_SUCCESS)
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

static void check_geom(vol_file_handle_t *v_handle,
		       clutter_table_file_handle_t *clutter_handle)

{

  int error_flag = FALSE;

  long ielev, iaz, ipoint, iplane, ilimit;

  si32 dbz_field = Glob->params.dbz_field;
  si32 nplanes;
  si32 nelevations, nazimuths;
  si32 data_dbz_val, nclut_points;
  si32 **vol_plane_heights;
  si32 **clutter_plane_heights;
  
  double clutter_dbz;
  double scalez;
  double clutter_scale, clutter_bias, data_scale, data_bias;
  double scale_diff, bias_diff;
  
  clutter_table_entry_t *clutter_entry;
  clutter_table_params_t *clut_params;
  clutter_table_index_t **clut_index;
  
  /*
   * set local pointers and variables
   */
    
  clut_params = clutter_handle->table_params;
  clut_index = clutter_handle->table_index;
    
  vol_plane_heights = v_handle->plane_heights;
  clutter_plane_heights = clutter_handle->plane_heights;

  nplanes = v_handle->vol_params->cart.nz;
  scalez = (double) v_handle->vol_params->cart.scalez;

  nelevations = v_handle->vol_params->radar.nelevations;
  nazimuths = v_handle->vol_params->radar.nazimuths;

  /*
   * check the cartesian parameters
   */
  
  if (memcmp((void *) &v_handle->vol_params->cart,
	     (void *) &clutter_handle->table_params->rc_params.cart,
	     (size_t) sizeof(cart_params_t))) {
      
    fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
    fprintf(stderr,
	    "Cartesian params in volume do not match clutter table.\n");
    fprintf(stderr,
	    "Generate a new clutter map using the current rc table.\n");
    fprintf(stderr, "Run program 'clutter_table_generate'.\n");
    tidy_and_exit(-1);
  }

  /*
   * check that the plane height limits are the same for the
   * radar-to-cart table and the clutter table
   */
  
  for (iplane = 0; iplane < nplanes; iplane++) {
    
    for (ilimit = 0; ilimit < N_PLANE_HEIGHT_VALUES; ilimit++) {
      
      if(vol_plane_heights[iplane][ilimit] !=
	 clutter_plane_heights[iplane][ilimit]) {
	
	fprintf(stderr, "\nERROR - %s:check_geom\n", Glob->prog_name);
	fprintf(stderr, "Plane height [%ld][%ld] does not match.\n",
		iplane, ilimit);
	fprintf(stderr, "Radar volume plane height = %g\n",
		(double) vol_plane_heights[iplane][ilimit] / scalez);
	fprintf(stderr, "Clutter table height = %g\n",
		(double) clutter_plane_heights[iplane][ilimit] / scalez);
	error_flag = TRUE;
	
      }
      
    } /* ilimit */
    
  } /* iplane */
  
  /*
   * check that the scale and bias of the dbz field matches that used
   * to generate the clutter table. If not, adjust the
   * values accordingly.
   */
  
  clutter_scale = ((double) clut_params->dbz_scale /
		   (double) clut_params->factor);
  clutter_bias = ((double) clut_params->dbz_bias /
		  (double) clut_params->factor);

  data_scale = ((double) v_handle->field_params[dbz_field]->scale /
		(double) v_handle->field_params[dbz_field]->factor);
  data_bias = ((double) v_handle->field_params[dbz_field]->bias /
	       (double) v_handle->field_params[dbz_field]->factor);
  
  scale_diff = fabs(clutter_scale - data_scale);
  bias_diff = fabs(clutter_bias - data_bias);
  
  /*
   * if the scale or bias differ, adjust the clutter table
   * to use the same scale as the data
   */
  
  if (scale_diff > 0.01 || bias_diff > 0.01) {
    
    for (ielev = 0; ielev < nelevations; ielev++) {
      
      for (iaz = 0; iaz < nazimuths; iaz++) {
	
	clutter_entry = clut_index[ielev][iaz].u.entry;
	nclut_points = clut_index[ielev][iaz].nclut_points;
	
	for (ipoint = 0; ipoint < nclut_points; ipoint++) {
	  
	  clutter_dbz = (double) clutter_entry->dbz * clutter_scale +
	    clutter_bias;
	  
	  data_dbz_val = (si32) (((clutter_dbz - data_bias) /
				  data_scale) + 0.5);
	  
	  if (data_dbz_val < 0)
	    data_dbz_val = 0;
	  if (data_dbz_val > 255)
	    data_dbz_val = 255;
	  
	  clutter_entry->dbz = data_dbz_val;
	  
	  clutter_entry++;
	  
	} /* ipoint */
	
      } /* iaz */
      
    } /* ielev */
    
  } /* if (scale_diff....... */
  
  if (error_flag == TRUE)
    tidy_and_exit(-1);
  
}

static void perform_removal(vol_file_handle_t *v_handle,
			    clutter_table_file_handle_t *clutter_handle)
{

  char notebuf[VOL_PARAMS_NOTE_LEN];

  si32 dbz_field = Glob->params.dbz_field;
  si32 ielev, iaz, ipoint, ifield, iplane;
  si32 field_index, plane_index;
  si32 nbytes_per_plane;
  si32 nelevations, nazimuths, nfields;
  si32 nclut_points;

  clutter_table_entry_t *clutter_entry;

  /*
   * set some local variables
   */

  nfields = v_handle->vol_params->nfields;
  nelevations = v_handle->vol_params->radar.nelevations;
  nazimuths = v_handle->vol_params->radar.nazimuths;
  nbytes_per_plane = (v_handle->vol_params->cart.nx *
		      v_handle->vol_params->cart.ny);

  /*
   * check that the dbz field set by the user
   * are valid
   */

  if (dbz_field > nfields -1 ) {
    fprintf(stderr, "ERROR - %s:perform_removal.\n", Glob->prog_name);
    fprintf(stderr, "Dbz_field %ld too high - only nfields of %ld.\n",
	    (long) dbz_field, (long) nfields);
    fprintf(stderr, "Field numbers start at 0\n");
    tidy_and_exit(-1);
  }

  /*
   * loop through elevations and azimuths, checking for clutter
   */
  
  for (ielev = 0; ielev < nelevations; ielev++) {

    for (iaz = 0; iaz < nazimuths; iaz++) {

      /*
       * get number of clutter points, and the address of the clutter
       * entries in the clutter list
       */

      nclut_points = clutter_handle->table_index[ielev][iaz].nclut_points;
      clutter_entry = clutter_handle->table_index[ielev][iaz].u.entry;

      /*
       * loop through the points assicaited with this beam
       */

      for (ipoint = 0; ipoint < nclut_points; ipoint++) {

	field_index = clutter_entry->cart_index;

	iplane = field_index / nbytes_per_plane;
	plane_index = field_index - iplane * nbytes_per_plane;
	
	if (v_handle->field_plane[dbz_field][iplane][plane_index]
	    <= clutter_entry->dbz) {

	  for (ifield = 0; ifield < nfields; ifield++)
	    v_handle->field_plane[ifield][iplane][plane_index] = 0;
	  
	}

	clutter_entry++;

      } /* ipoint */

    } /* iaz */

  } /* ielev */

  /*
   * append a message to the file note, indicating that the clutter
   * has been removed, giving details
   */

  sprintf(notebuf, "%s %g dbz, clutter file '%s'.\n",
	  "Clutter removed, dbz margin ",
	  ((double) clutter_handle->table_params->dbz_margin /
	   (double) clutter_handle->table_params->factor),
	  Glob->params.clutter_table_file_path);
  
  strncat(v_handle->vol_params->note,
	  notebuf,
	  (int) VOL_PARAMS_NOTE_LEN - strlen(v_handle->vol_params->note));

}
