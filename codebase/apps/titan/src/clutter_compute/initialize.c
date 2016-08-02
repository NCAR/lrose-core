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
 * initialize.c
 *
 * reads in the first and last radar volumes, initializes structs
 * and creates arrays
 *
 * RAP, NCAR, Boulder CO
 *
 * November 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "clutter_compute.h"

void initialize(si32 nfiles,
		char **clutter_file_names,
		vol_file_handle_t *vol_index,
		vol_file_handle_t *clut_vol_index)

{

  ui08 *cdata;
  char add_note[VOL_PARAMS_NOTE_LEN];

  long ifield, iplane;
  si32 nfields, nplanes, npoints_plane, npoints_vol;

  date_time_t start, mid, end;

  /*
   * initialize the vol file indices
   */

  RfInitVolFileHandle(vol_index,
		      Glob->prog_name,
		      (char *) NULL,
		      (FILE *) NULL);

  RfInitVolFileHandle(clut_vol_index,
		      Glob->prog_name,
		      Glob->clutter_file_path,
		      (FILE *) NULL);
  
  /*
   * allocate vol params for the clutter file handle
   */

  if (RfAllocVolParams(clut_vol_index, "initialize") != R_SUCCESS)
    exit(1);

  /*
   * read in first radar volume
   */

  vol_index->vol_file_path = clutter_file_names[0];

  if (RfReadVolume(vol_index, "initialize") != R_SUCCESS)
    exit(1);

  /*
   * copy the vol_params to the clutter file handle
   */

  *clut_vol_index->vol_params = *vol_index->vol_params;

  /*
   * amend the note
   */

  sprintf(add_note, "Median of clutter data in directory '%s'.\n",
	  Glob->clutter_dir);

  strncat(clut_vol_index->vol_params->note, add_note,
	  (int) (VOL_PARAMS_NOTE_LEN -
		 strlen(clut_vol_index->vol_params->note)));

  /*
   * set start time of clutter volume
   */

  clut_vol_index->vol_params->start_time =
    vol_index->vol_params->start_time;

  /*
   * read in last radar volume
   */

  vol_index->vol_file_path = clutter_file_names[nfiles - 1];

  if (RfReadVolume(vol_index, "initialize") != R_SUCCESS)
    exit(1);

  /*
   * set end time of clutter volume
   */

  clut_vol_index->vol_params->end_time = vol_index->vol_params->end_time;

  /*
   * copy scan times to date_time_t structures
   */

  Rfrtime2dtime(&clut_vol_index->vol_params->start_time, &start);
  Rfrtime2dtime(&clut_vol_index->vol_params->end_time, &end);

  /*
   * compute mid time, and convert back
   */

  mid.unix_time = start.unix_time + (end.unix_time - start.unix_time) / 2;
  uconvert_from_utime(&mid);

  /*
   * copy mid date and time to clutter vol index
   */

  Rfdtime2rtime(&mid, &clut_vol_index->vol_params->mid_time);

  /*
   * allocate arrays for the clutter file handle
   */

  if (RfAllocVolArrays(clut_vol_index, "initialize") != R_SUCCESS)
    exit(0);

  /*
   * copy the radar elevation angles, plane heights and field parameters
   * to the clutter vol index
   */

  memcpy ((void *) clut_vol_index->radar_elevations,
          (void *) vol_index->radar_elevations,
          (size_t) (vol_index->vol_params->radar.nelevations * sizeof(si32)));

  memcpy ((void *) *clut_vol_index->plane_heights,
          (void *) *vol_index->plane_heights,
          (size_t) (vol_index->vol_params->cart.nz *
	       N_PLANE_HEIGHT_VALUES * sizeof(si32)));

  nfields = vol_index->vol_params->nfields;

  for (ifield = 0; ifield < nfields; ifield++) {
    
    memcpy ((void *) clut_vol_index->field_params[ifield],
            (void *) vol_index->field_params[ifield],
            (size_t) sizeof(field_params_t));

  } /* ifield */

  /*
   * set up the array for the clutter field data, and point
   * the field_plane array into it
   */

  nplanes = vol_index->vol_params->cart.nz;

  npoints_plane = (vol_index->vol_params->cart.nx *
		   vol_index->vol_params->cart.ny);

  npoints_vol = nplanes * npoints_plane;

  cdata = (ui08 *) umalloc
    ((ui32) (nfields * npoints_vol));
  
  for (ifield = 0; ifield < nfields; ifield++) {

    for (iplane = 0; iplane < nplanes; iplane++) {

      clut_vol_index->field_plane[ifield][iplane] = cdata;

      cdata += npoints_plane;

    } /* iplane */

  } /* ifield */

}
