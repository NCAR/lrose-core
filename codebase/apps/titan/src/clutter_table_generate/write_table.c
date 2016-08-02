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
 * write_table.c
 *
 * generates the clutter table and writes it to file
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "clutter_table_generate.h"
#include <toolsa/xdru.h>
#include <dataport/bigend.h>
#include <time.h>

void write_table(vol_file_handle_t *clut_vol_index,
		 rc_table_file_handle_t *rc_handle)

{

  char file_label[R_FILE_LABEL_LEN];
  ui08 **dbz_data;

  si32 ielev, iaz, ipoint, iplane;
  si32 field_index, plane_index;
  si32 nelevations, nazimuths, nbeams;
  si32 nplanes, nheights;
  si32 nbytes_per_plane;
  si32 headers_size;
  si32 index_size;
  si32 nclut_points, ncart_points;
  si32 list_offset = 0;
  si32 dbz_scaled_margin, dbz_scaled_threshold;

  double dbz_scale, dbz_bias;

  rc_table_params_t *rc_table_params;
  rc_table_index_t **rc_table_index;
  rc_table_entry_t *rc_entry;

  clutter_table_params_t clutter_table_params;
  clutter_table_index_t **clutter_table_index;
  clutter_table_entry_t clutter_entry;

  field_params_t *dbz_fparams;

  date_time_t dtime;

  FILE *clut_file;

  /*
   * set local pointers
   */

  rc_table_params = rc_handle->table_params;
  rc_table_index = rc_handle->table_index;

  /*
   * open clutter table file
   */

  if ((clut_file = fopen(Glob->clutter_table_path, "w")) == NULL) {

    fprintf(stderr, "ERROR - %s:write_table.\n", Glob->prog_name);
    fprintf(stderr, "Cannot open clutter table file for writing.\n");
    perror(Glob->clutter_table_path);
    exit(1);

  }

  /*
   * compute size of the file headers
   */

  nelevations = rc_table_params->nelevations;
  nazimuths = rc_table_params->nazimuths;
  nbeams = nazimuths * nelevations;
  nbytes_per_plane = rc_table_params->cart.nx * rc_table_params->cart.ny;
  nplanes = rc_table_params->cart.nz;
  nheights = nplanes * N_PLANE_HEIGHT_VALUES;

  headers_size =
    (R_FILE_LABEL_LEN +
     sizeof(clutter_table_params_t) +
     nelevations * sizeof(si32) +
     nheights * sizeof(si32));

  index_size =
    nbeams * sizeof(clutter_table_index_t);

  /*
   * space fwd over area for clutter_table_params header
   * and the clutter_table_index array
   */

  fseek(clut_file, headers_size + index_size, SEEK_SET);

  /*
   * allocate space for the clutter_table_index array
   */

  clutter_table_index = (clutter_table_index_t **) umalloc2
    ((ui32) nelevations,
     (ui32) nazimuths,
     sizeof(clutter_table_index_t));

  /*
   * compute the dbz margin in terms of the byte value
   */

  dbz_fparams = clut_vol_index->field_params[Glob->dbz_field];
  dbz_data = clut_vol_index->field_plane[Glob->dbz_field];
  
  dbz_bias =
    (double) dbz_fparams->bias / (double) dbz_fparams->factor;

  dbz_scale =
    (double) dbz_fparams->scale / (double) dbz_fparams->factor;

  dbz_scaled_margin = (si32) (Glob->dbz_margin / dbz_scale + 0.5);
  dbz_scaled_threshold =
    (si32) ((Glob->dbz_threshold - dbz_bias) / dbz_scale + 0.5);

  /*
   * loop through elevations and azimuths
   */

  for (ielev = 0; ielev < nelevations; ielev++) {

    for (iaz = 0; iaz < nazimuths; iaz++) {

      nclut_points = 0;

      /*
       * loop through cartesian points
       */

      ncart_points = rc_table_index[ielev][iaz].npoints;
      rc_entry = rc_table_index[ielev][iaz].u.entry;

      for (ipoint = 0; ipoint < ncart_points; ipoint++)  {

	field_index = rc_entry->index;

	iplane = field_index / nbytes_per_plane;
	plane_index = field_index - iplane * nbytes_per_plane;

	if ((si32) dbz_data[iplane][plane_index] > dbz_scaled_threshold) {

	  nclut_points++;

	  clutter_entry.ipoint = ipoint;
	  clutter_entry.cart_index = field_index;
	  clutter_entry.dbz =
	    dbz_data[iplane][plane_index] + dbz_scaled_margin;
	  if (clutter_entry.dbz > 255) clutter_entry.dbz = 255;

	  /*
	   * put into network byte order
	   */

	  BE_from_array_16((ui16 *) &clutter_entry.dbz,
			   (ui32) sizeof(si16));
	  BE_from_array_16((ui16 *) &clutter_entry.ipoint,
			   (ui32) sizeof(si16));
	  BE_from_array_32((ui32 *) &clutter_entry.cart_index,
			   (ui32) sizeof(si32));
	  
	  if (ufwrite((char *) &clutter_entry, sizeof(clutter_table_entry_t),
		      1, clut_file) != 1) {

	    fprintf(stderr, "ERROR - %s:write_table\n", Glob->prog_name);
	    fprintf(stderr, "Writing clutter entry.\n");
	    perror(Glob->clutter_table_path);
	    exit(1);

	  } /* if (ufwrite.... */

	}

	rc_entry++;

      } /* ipoint */

      /*
       * load index values
       */

      clutter_table_index[ielev][iaz].nclut_points = nclut_points;
      clutter_table_index[ielev][iaz].u.offset = list_offset;

      /*
       * put into network byte order
       */

      BE_from_array_32((ui32 *) &clutter_table_index[ielev][iaz].u.offset,
		       (ui32) sizeof(si32));
      BE_from_array_32((ui32 *) &clutter_table_index[ielev][iaz].nclut_points,
		       (ui32) sizeof(si32));
      
      /*
       * update list offset
       */

      list_offset += nclut_points * sizeof(clutter_table_entry_t);

    } /* iaz */

  } /* ielev */

  /*
   * load up clutter table params structure
   */

  clutter_table_params.nbytes_char = rc_table_params->nbytes_char;
  clutter_table_params.file_time = time(NULL);
  
  Rfrtime2dtime(&clut_vol_index->vol_params->start_time, &dtime);
  clutter_table_params.start_time = uunix_time(&dtime);
  Rfrtime2dtime(&clut_vol_index->vol_params->mid_time, &dtime);
  clutter_table_params.mid_time = uunix_time(&dtime);
  Rfrtime2dtime(&clut_vol_index->vol_params->end_time, &dtime);
  clutter_table_params.end_time = uunix_time(&dtime);

  clutter_table_params.factor = dbz_fparams->factor;
  clutter_table_params.dbz_scale = dbz_fparams->scale;
  clutter_table_params.dbz_bias = dbz_fparams->bias;
  clutter_table_params.dbz_margin = (si32)
    (Glob->dbz_margin * clutter_table_params.factor + 0.5);

  clutter_table_params.nlist = list_offset;

  clutter_table_params.index_offset = headers_size;

  clutter_table_params.list_offset = 
    clutter_table_params.index_offset + index_size;

  memcpy ((void *) &clutter_table_params.rc_params,
          (void *) rc_table_params,
          (size_t) sizeof(rc_table_params_t));

  /*
   * put params into network byte order
   */
  
  BE_from_array_32((ui32 *) &clutter_table_params, 
		   (ui32) (sizeof(clutter_table_params_t) -
			   rc_table_params->nbytes_char));
  
  /*
   * set file label
   */

  memset ((void *) file_label,
          (int) 0, (size_t) R_FILE_LABEL_LEN);

  strncpy(file_label, CLUTTER_TABLE_FILE, R_FILE_LABEL_LEN);

  /*
   * seek to start of file
   */

  fseek(clut_file, (si32) 0, SEEK_SET);

  /*
   * write file label
   */

  if (ufwrite(file_label,
	      sizeof(char),
	      R_FILE_LABEL_LEN,
	      clut_file) != R_FILE_LABEL_LEN) {

    fprintf(stderr, "ERROR - %s:write_table\n", Glob->prog_name);
    fprintf(stderr, "Writing clutter file label.\n");
    perror(Glob->clutter_file_path);
    exit(1);
    
  }
  
  /*
   * write params
   */

  if (ufwrite((char *) &clutter_table_params,
	      sizeof(clutter_table_params_t),
	      1, clut_file) != 1) {

    fprintf(stderr, "ERROR - %s:write_table\n", Glob->prog_name);
    fprintf(stderr, "Writing clutter file params.\n");
    perror(Glob->clutter_file_path);
    exit(1);
    
  }

  /*
   * code the radar_elevations array into network
   * byte order and write to file
   */

  BE_from_array_32((ui32 *) clut_vol_index->radar_elevations,
		   (ui32) (nelevations * sizeof(si32)));
  
  if (ufwrite((char *) clut_vol_index->radar_elevations,
	      sizeof(si32),
	      (int) nelevations,
	      clut_file) != nelevations) {

    fprintf(stderr, "ERROR - %s:write_table\n", Glob->prog_name);
    fprintf(stderr, "Writing radar elevations.\n");
    perror(Glob->clutter_file_path);
    exit(1);

  }

  /*
   * code the plane_heights array into network
   * byte order and write to file
   */

  BE_from_array_32((ui32 *) *clut_vol_index->plane_heights,
		   (ui32) (nheights * sizeof(si32)));
  
  if (ufwrite((char *) *clut_vol_index->plane_heights,
	      sizeof(si32), (int) nheights,
	      clut_file) != nheights) {

    fprintf(stderr, "ERROR - %s:write_table\n", Glob->prog_name);
    fprintf(stderr, "Writing plane height limits.\n");
    perror(Glob->clutter_file_path);
    exit(1);

  }

  /*
   * write index
   */

  for (ielev = 0; ielev < nelevations; ielev++) {
    
    if (ufwrite((char *) clutter_table_index[ielev],
		sizeof(clutter_table_index_t),
		(int) nazimuths,
		clut_file) != nazimuths) {
      
      fprintf(stderr, "ERROR - %s:write_table\n", Glob->prog_name);
      fprintf(stderr, "Writing index, elen num %ld.\n", (long) ielev);
      perror(Glob->clutter_file_path);
      exit(1);
      
    }
    
  } /* ielev */

  /*
   * close file
   */

  fclose(clut_file);

}
