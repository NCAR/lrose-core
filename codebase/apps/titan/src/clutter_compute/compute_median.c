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
 * compute_median.c
 *
 * loops through the clutter files and computes the median value
 * for each data field in turn. Writes the median values to the
 * median files
 *
 * RAP, NCAR, Boulder CO
 *
 * November 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "clutter_compute.h"

static int int_compare();

void compute_median(si32 nfiles,
		    char **clutter_file_names,
		    vol_file_handle_t *vol_index,
		    vol_file_handle_t *clut_vol_index)

{

  ui08 *median_data;
  ui08 **raw_data;

  int *sort_array;

  long ifield, iplane, ifile, ipoint;
  si32 nhalf;
  si32 nplanes, nfields;
  si32 npoints_plane;

  /*
   * set constants
   */

  nhalf = nfiles / 2;
  nfields = vol_index->vol_params->nfields;
  nplanes = vol_index->vol_params->cart.nz;
  npoints_plane = (vol_index->vol_params->cart.nx *
		   vol_index->vol_params->cart.ny);

  /*
   * allocate arrays
   */

  median_data = (ui08 *) umalloc ((ui32) npoints_plane);

  raw_data = (ui08 **) umalloc2
    ((ui32) nfiles, (ui32) npoints_plane,
     (ui32) sizeof(ui08));
  
  sort_array = (int *) umalloc ((ui32) (nfiles * sizeof(int)));

  /*
   * loop through fields
   */

  for (ifield = 0; ifield < nfields; ifield++) {

    /*
     *  loop through planes
     */

    for (iplane = 0; iplane < nplanes; iplane++) {

      /*
       * check the malloc's
       */

      umalloc_verify();
      
      printf("%s - field, plane = %ld, %ld.\n", Glob->prog_name,
	     ifield, iplane);
      
      /*
       * loop through files
       */

      for (ifile = 0; ifile < nfiles; ifile++) {

	/*
	 * read in the volume
	 */

	if (RfReadVolume(vol_index, 
			 "compute_median") != R_SUCCESS) {
	  fclose(vol_index->vol_file);
	  exit(1);
	}
  
	/*
	 * check that the cartesian params are the same as for the
	 * clutter file (i.e. the same as the first file read
	 */

	if (memcmp((void *) &vol_index->vol_params->cart,
		   (void *) &clut_vol_index->vol_params->cart,
		   (size_t) sizeof(cart_params_t))) {

	  fprintf(stderr, "ERROR - %s:compute_median\n", Glob->prog_name);
	  fprintf(stderr,
		  "Cartesian params for file '%s' differ from file '%s'.\n",
		  clutter_file_names[ifile],
		  clutter_file_names[0]);
	  exit(1);

	} /* if(memcmp.... */

	/*
	 * check field params - first set the noise values equal, since
	 * these may change from file to file
	 */

	vol_index->field_params[ifield]->noise = 
	  clut_vol_index->field_params[ifield]->noise;
	
	if (memcmp((void *) vol_index->field_params[ifield],
		   (void *) clut_vol_index->field_params[ifield],
		   (size_t) sizeof(field_params_t))) {

	  fprintf(stderr, "ERROR - %s:compute_median\n", Glob->prog_name);
	  fprintf(stderr,
		  "Field params, fld %ld, file '%s' differ from file '%s'.\n",
		  ifield,
		  clutter_file_names[ifile],
		  clutter_file_names[0]);
	  exit(1);
	  
	} /* if(memcmp.... */

	/*
	 * copy the plane to the raw_data array
	 */

	memcpy ((void *) raw_data[ifile],
		(void *) vol_index->field_plane[ifield][iplane],
		(size_t) npoints_plane);

      } /* ifile */

      /*
       * for each point in the plane, compute median
       */

      for (ipoint = 0; ipoint < npoints_plane; ipoint++) {

	for (ifile = 0; ifile < nfiles; ifile++)
	  sort_array[ifile] = raw_data[ifile][ipoint];

	/*
	 * sort
	 */

	qsort((char *) sort_array,
	      (int) nfiles, sizeof(int), int_compare);

	median_data[ipoint] = sort_array[nhalf];

      } /* ipoint */

      /*
       * copy the median data to the field_plane array
       */

      memcpy ((void *) clut_vol_index->field_plane[ifield][iplane],
              (void *) median_data,
              (size_t) npoints_plane);

    } /* iplane */

    /*
     * append ' - median clutter' to the field name
     */

    strncat((char *) clut_vol_index->field_params[ifield]->name,
	    (char *) " - median clutter",
	    (int) (R_LABEL_LEN -
		   strlen(clut_vol_index->field_params[ifield]->name)));

  } /* ifield */

} 
/*****************************************************************************
 * define function to be used for sorting
 */

static int int_compare(i, j)

     int *i, *j;

{

  return (int) (*i - *j);

}

