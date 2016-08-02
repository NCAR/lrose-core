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
 * invert_table_and_write_files.c
 *
 * Inverts the lookup table to allow one to look up the series
 * of relevant cartesian points given the radar coordinate.
 * Writes the radar_to_cart and slave-radar-to-cart lookup
 * tables to file.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rctable_generate.h"
#include <toolsa/xdru.h>
#include <dataport/bigend.h>

static si32 *Long_array = NULL;
static int Nlongs_alloc = 0;

static void check_long_array(int nlongs_needed);

static void free_long_array(void);

static si32 get_maxpergate(si32 *rindex_table,
			   ppi_table_t **ppi_table,
			   scan_table_t *scan_table);

static void load_params(rc_table_params_t *params,
			scan_table_t *scan_table,
			si32 list_offset,
			si32 header_size,
			si32 index_size);

static void write_header(rc_table_params_t *params,
			 scan_table_t *scan_table,
			 char *label,
			 FILE *out_file,
			 char *file_path);

void invert_table_and_write_files(si32 *rindex_table,
				  scan_table_t *scan_table)

{

  ui16 ngates_active, last_gate_active;

  si32 i, ielev, iaz, igate, icart, ipoint;
  si32 npoints, tot_points;
  si32 maxpergate;
  si32 rc_list_offset = 0, slave_list_offset = 0;
  si32 header_size, rc_handle_size, slave_handle_size;
  si32 ncart, start_index, end_index, offset;
  si32 nheights;
  si32 nbeams_array;
  si32 *index;

  rc_table_index_t **rc_table_index;
  ppi_table_t **ppi_table;
  slave_table_index_t **slave_table_index;
  rc_table_entry_t *rc_entry;
  rc_table_params_t rc_params, slave_params;
  scan_table_elev_t *elev;

  FILE *rc_file, *slave_file;

  /*
   * allocate array for the rc_table and slave_table structures
   */

  rc_table_index = (rc_table_index_t **) ucalloc2
    ((ui32) scan_table->nelevations,
     (ui32) scan_table->max_azimuths,
     (ui32) sizeof(rc_table_index_t));

  if (Glob->create_slave_table) {
    slave_table_index = (slave_table_index_t **) ucalloc2
      ((ui32) scan_table->nelevations,
       (ui32) scan_table->max_azimuths,
       (ui32) sizeof(slave_table_index_t));
  }
       
  /*
   * allocate array (for a given elevation number) containing the number of
   * cartesian points and their offsets for each gate and azimuth
   */

  ppi_table = (ppi_table_t **) ucalloc2
    ((ui32) scan_table->max_azimuths,
     (ui32) scan_table->ngates,
     sizeof(ppi_table_t));

  /*
   * compute constants
   */

  nheights = Glob->nz * N_PLANE_HEIGHT_VALUES;

  if (Glob->use_azimuth_table) {
    header_size = (R_LABEL_LEN +
		   sizeof(rc_table_params_t) +
		   scan_table->nelevations * sizeof(si32) +
		   scan_table->nelevations * sizeof(si32) +
		   scan_table->nbeams_vol * sizeof(si32) +
		   nheights * sizeof(si32));
  } else {
    header_size = (R_LABEL_LEN +
		   sizeof(rc_table_params_t) +
		   scan_table->nelevations * sizeof(si32) +
		   nheights * sizeof(si32));
  }
  
  nbeams_array = scan_table->nazimuths * scan_table->nelevations;
  rc_handle_size = nbeams_array * sizeof(rc_table_index_t);
  slave_handle_size = nbeams_array * sizeof(slave_table_index_t);

  /*
   * open files
   */

  if ((rc_file = fopen(Glob->rc_table_file_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:invert_table.\n", Glob->prog_name);
    fprintf(stderr, "Unable to create rc table file.\n");
    perror(Glob->rc_table_file_path);
    exit(-1);
  }

  if (Glob->create_slave_table) {
    if ((slave_file = fopen(Glob->slave_table_file_path, "w")) == NULL) {
      fprintf(stderr, "ERROR - %s:invert_table.\n", Glob->prog_name);
      fprintf(stderr, "Unable to create slave table file.\n");
      perror(Glob->slave_table_file_path);
      exit(-1);
    }
  }

  /*
   * space fwd over area for rctable_params header and the index array
   */

  fseek(rc_file, header_size + rc_handle_size, SEEK_SET);
  if (Glob->create_slave_table) {
    fseek(slave_file, header_size + slave_handle_size, SEEK_SET);
  }

  /*
   * determine max number of cartesian points per gate
   */

  maxpergate = get_maxpergate(rindex_table, ppi_table, scan_table);

  /*
   * compute number of data points in the cartesian grid
   */

  ncart = Glob->nx * Glob->ny * Glob->nz;

  /*
   * allocate enough space in the ppi_table for the
   * max number of points found per gate
   */

  for (iaz = 0; iaz < scan_table->max_azimuths; iaz++) {
    for (igate = 0; igate < scan_table->ngates; igate++) {
      ppi_table[iaz][igate].index = (ui32 *)
	umalloc((ui32) (maxpergate * sizeof(si32)));
    }
  }

  /*
   * main loop through the elevations
   */

  elev = scan_table->elevs;
  for (ielev = 0; ielev < scan_table->nelevations; ielev++, elev++) {

    printf("invert_table: %s%d\n",
	   "loading up ppi table, elev number ", ielev);

    /*
     * initialize counters in Ppi Table
     */

    for (iaz = 0; iaz < scan_table->max_azimuths; iaz++)
      for (igate = 0; igate < scan_table->ngates; igate++)
	ppi_table[iaz][igate].npoints = 0;

    /*
     * search through the rindex_table array, loading up the Ppi Table structs
     * with any indices which apply to this elevation number
     */

    start_index = elev->start_beam_num * scan_table->ngates;
    end_index = ((elev->end_beam_num + 1) * scan_table->ngates) -1;
    
    for (icart = 0; icart < ncart; icart++) {

      if (rindex_table[icart] >= start_index &&
	  rindex_table[icart] <= end_index) {

	offset = rindex_table[icart] - start_index;
	iaz = offset / scan_table->ngates;
	igate = offset - iaz * scan_table->ngates;

	npoints = ppi_table[iaz][igate].npoints;
	ppi_table[iaz][igate].npoints++;

	ppi_table[iaz][igate].index[npoints] = icart;

      }

    } /* icart */

    /*
     * loop through the azimuths
     */

    printf("invert_table: %s\n",
	   "loading Rc_table structs, writing to file.");

    for (iaz = 0; iaz < scan_table->max_azimuths; iaz++) {

      /*
       * get the number of active gates and total number of cartesian
       * points
       */

      last_gate_active = 0;
      ngates_active = 0;
      tot_points = 0;

      for (igate = 0; igate < scan_table->ngates; igate++) {
	if (ppi_table[iaz][igate].npoints > 0) {
	  ngates_active++;
	  last_gate_active = igate;
	  tot_points += ppi_table[iaz][igate].npoints;
	}
      }

      /*
       * load up index structures, code into network byte order
       */

      rc_table_index[ielev][iaz].npoints = tot_points;
      rc_table_index[ielev][iaz].last_gate_active = last_gate_active;
      rc_table_index[ielev][iaz].u.offset = rc_list_offset;
      
      if (Glob->create_slave_table) {
	slave_table_index[ielev][iaz].npoints = tot_points;
	slave_table_index[ielev][iaz].u.offset = slave_list_offset;
      }

      BE_from_array_32(&rc_table_index[ielev][iaz].u.offset,
		       (ui32) sizeof(si32));
      BE_from_array_16(&rc_table_index[ielev][iaz].npoints,
		       (ui32) sizeof(si16));
      BE_from_array_16(&rc_table_index[ielev][iaz].last_gate_active,
		       (ui32) sizeof(si16));
      
      if (Glob->create_slave_table) {
	slave_table_index[ielev][iaz].npoints = tot_points;
	slave_table_index[ielev][iaz].u.offset = slave_list_offset;
	BE_from_array_32(&slave_table_index[ielev][iaz].npoints,
			 (ui32) sizeof(si32));
	BE_from_array_32(&slave_table_index[ielev][iaz].u.offset,
			 (ui32) sizeof(si32));
      }
      
      /*
       * if there are active gates for this azimuth, gather details about
       * the points
       */

      if ((si32) ngates_active > 0) {

	/*
	 * allocate memory for the table entries for this beam
	 */

	rc_entry = (rc_table_entry_t *)
	  umalloc((ui32) tot_points * sizeof(rc_table_entry_t));
	index = (si32 *) umalloc ((ui32) (tot_points * sizeof(si32)));

	/*
	 * loop through gates
	 */

	ipoint = 0;

	for (igate = 0; igate < scan_table->ngates; igate++) {

	  /*
	   * check if there are cartesian points associated with this gate
	   */

	  npoints = ppi_table[iaz][igate].npoints;

	  if (npoints > 0) {

	    for (i = 0; i < npoints; i++) {

	      index[ipoint] = ppi_table[iaz][igate].index[i];
	      rc_entry[ipoint].index = ppi_table[iaz][igate].index[i];
	      rc_entry[ipoint].gate = igate;
	      ipoint++;

	    }

	  } /* if (npoints > 0) */

	} /* igate */

	/*
	 * code rc_entry and index arrays into network byte order
	 */

	BE_from_array_32((ui32 *) rc_entry,
			 (ui32) (tot_points * sizeof(rc_table_entry_t)));
	BE_from_array_32((ui32 *) index,
			 (ui32) (tot_points * sizeof(si32)));
	
	/*
	 * write out rc_entry array
	 */

	fwrite((char *) rc_entry, sizeof(rc_table_entry_t),
	       (int) tot_points, rc_file);

	if (Glob->create_slave_table) {

	  /*
	   * write out slave table indices
	   */
	  
	  fwrite((char *) index, sizeof(si32), (int) tot_points, slave_file);

	}

	/*
	 * increment list offsets
	 */

	rc_list_offset += tot_points * sizeof(rc_table_entry_t);
	slave_list_offset += tot_points * sizeof(si32);

	/*
	 * free up arrays
	 */

	ufree((char *) rc_entry);
	ufree((char *) index);

      } /* if (ngates_active > 0) */

    } /* iaz */

  } /* ielev */

  printf("invert_table: %s\n",
	 "writing table parameters and index arrays to files.");

  /*
   * load up rc_params and slave_params structures
   */

  load_params(&rc_params, scan_table,
	      rc_list_offset, header_size, rc_handle_size);

  if (Glob->create_slave_table) {
    load_params(&slave_params, scan_table,
		slave_list_offset, header_size, slave_handle_size);
  }

  /*
   * write file headers
   */

  write_header(&rc_params, scan_table,
	       RADAR_TO_CART_TABLE_FILE,
	       rc_file, Glob->rc_table_file_path);

  if (Glob->create_slave_table) {
    write_header(&slave_params, scan_table,
		 RADAR_TO_CART_SLAVE_TABLE_FILE,
		 slave_file, Glob->slave_table_file_path);
  }

  /*
   * write rc_table_index
   */

  if (ufwrite((char *) *rc_table_index,
	      sizeof(rc_table_index_t),
	      (int) nbeams_array,
	      rc_file) != nbeams_array) {
    fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing radar-to-cart table index.\n");
    perror(Glob->rc_table_file_path);
    exit(-1);
  }
  
  if (Glob->create_slave_table) {

    /*
     * write slave_table_index
     */
    
    if (ufwrite((char *) *slave_table_index,
		sizeof(slave_table_index_t),
		(int) nbeams_array,
		slave_file) != nbeams_array) {
      fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	      Glob->prog_name);
      fprintf(stderr, "Writing radar-to-cart slave table index.\n");
      perror(Glob->slave_table_file_path);
      exit(-1);
    }
  }
  
  /*
   * close files
   */

  fclose(rc_file);
  if (Glob->create_slave_table) {
    fclose(slave_file);
  }

  /*
   * free up
   */

  ufree2((void **) rc_table_index);
  if (Glob->create_slave_table) {
    ufree2((void **) slave_table_index);
  }

  for (iaz = 0; iaz < scan_table->max_azimuths; iaz++) {
    for (igate = 0; igate < scan_table->ngates; igate++) {
      ufree((char *) ppi_table[iaz][igate].index);
    }
  }
  ufree2((void **) ppi_table);

  free_long_array();

}

/******************
 * get_maxpergate()
 */

static si32 get_maxpergate(si32 *rindex_table,
			   ppi_table_t **ppi_table,
			   scan_table_t *scan_table)

{

  si32 ielev, iaz, igate, icart;
  si32 ncart;
  si32 maxpergate = 0;
  si32 start_index, end_index;
  si32 offset;
  scan_table_elev_t *elev;

  /*
   * compute number of data points in the cartesian grid
   */

  ncart = Glob->nx * Glob->ny * Glob->nz;

  /*
   * determine max number of cartesian points per gate
   */

  elev = scan_table->elevs;
  for (ielev = 0; ielev < scan_table->nelevations; ielev++, elev++) {
    
    /*
     * initialize counters in ppi table array
     */

    for (iaz = 0; iaz < scan_table->max_azimuths; iaz++) 
      for (igate = 0; igate < scan_table->ngates; igate++)
	ppi_table[iaz][igate].npoints = 0;

    /*
     * search through the Rindex_table array, loading up the ppi table structs
     * with any indices which apply to this elevation number
     */

    start_index = elev->start_beam_num * scan_table->ngates;
    end_index = ((elev->end_beam_num + 1) * scan_table->ngates) -1;

    if (Glob->debug) {
      fprintf(stderr, "start_index, end_index: %ld, %ld\n",
	      (long) start_index, (long) end_index);
    }
    
    for (icart = 0; icart < ncart; icart++) {

      if (rindex_table[icart] >= start_index
	  && rindex_table[icart] <= end_index) {

	offset = rindex_table[icart] - start_index;
	iaz = offset / scan_table->ngates;
	igate = offset - iaz * scan_table->ngates;

	ppi_table[iaz][igate].npoints++;
	if (ppi_table[iaz][igate].npoints > maxpergate) {
	  maxpergate = ppi_table[iaz][igate].npoints;
	}

      }

    } /* icart */

    printf("invert_table: max number of points per gate "
	   "%ld for elev %ld\n",
	   (long) maxpergate, (long) ielev);

  } /* ielev */

  return (maxpergate);

}

/****************
 * load up params
 */

static void load_params(rc_table_params_t *params,
			scan_table_t *scan_table,
			si32 list_offset,
			si32 header_size,
			si32 index_size)

{

  si32 nbytes_char;

  /*
   * load up rc_params structure for rc table
   */

  nbytes_char = N_CART_PARAMS_LABELS * R_LABEL_LEN;

  params->nbytes_char = nbytes_char;

  params->use_azimuth_table = scan_table->use_azimuth_table;
  params->extend_below = scan_table->extend_below;
  params->missing_data_index = scan_table->missing_data_index;

  params->nelevations = scan_table->nelevations;
  params->nazimuths = scan_table->nazimuths;
  params->ngates = scan_table->ngates;
  params->nbeams_vol = scan_table->nbeams_vol;

  params->delta_azimuth =
    (si32) (scan_table->delta_azimuth * DEG_FACTOR + 0.5);
  params->start_azimuth =
    (si32) (scan_table->start_azimuth * DEG_FACTOR + 0.5);
  params->beam_width =
    (si32) (scan_table->beam_width * DEG_FACTOR + 0.5);

  params->start_range = (si32) (scan_table->start_range * 1000000 + 0.5);
  params->gate_spacing = (si32) (scan_table->gate_spacing * 1000000 + 0.5);

  params->radar_latitude =
    (si32) floor(Glob->radar_latitude * DEG_FACTOR + 0.5);
  params->radar_longitude =
    (si32) floor(Glob->radar_longitude * DEG_FACTOR + 0.5);

  params->ndata = Glob->nx * Glob->ny * Glob->nz;
  params->nlist = list_offset;

  params->index_offset = header_size;
  params->list_offset = params->index_offset + index_size;

  params->cart.nbytes_char = nbytes_char;

  params->cart.latitude =
    (si32) floor(Glob->cart_latitude * DEG_FACTOR + 0.5);
  params->cart.longitude =
    (si32) floor(Glob->cart_longitude * DEG_FACTOR + 0.5);
  params->cart.rotation =
    (si32) floor(Glob->cart_rotation * DEG_FACTOR + 0.5);

  params->cart.nx = Glob->nx;
  params->cart.ny = Glob->ny;
  params->cart.nz = Glob->nz;

  params->cart.minx =
    (si32) (Glob->minx * KM_SCALEX + 0.5 * usign(Glob->minx));
  params->cart.miny =
    (si32) (Glob->miny * KM_SCALEY + 0.5 * usign(Glob->miny));
  params->cart.minz =
    (si32) (Glob->minz * KM_SCALEZ + 0.5 * usign(Glob->minz));

  params->cart.dx = (si32) (Glob->dx * KM_SCALEX + 0.5);
  params->cart.dy = (si32) (Glob->dy * KM_SCALEY + 0.5);
  params->cart.dz = (si32) (Glob->dz * KM_SCALEZ + 0.5);

  params->cart.radarx = (si32)
    (Glob->radarx * KM_SCALEX + 0.5 * usign(Glob->radarx));
  params->cart.radary = (si32)
    (Glob->radary * KM_SCALEY + 0.5 * usign(Glob->radary));
  params->cart.radarz = (si32)
    (Glob->radarz * KM_SCALEZ + 0.5 * usign(Glob->radarz));

  memset ((void *) params->cart.unitsx,
          (int) 0, (size_t)  R_LABEL_LEN);
  memset ((void *) params->cart.unitsy,
          (int) 0, (size_t)  R_LABEL_LEN);
  memset ((void *) params->cart.unitsz,
          (int) 0, (size_t)  R_LABEL_LEN);

  params->cart.scalex = (si32) (KM_SCALEX + 0.5);
  params->cart.scaley = (si32) (KM_SCALEY + 0.5);
  params->cart.scalez = (si32) (KM_SCALEZ + 0.5);
  
  params->cart.km_scalex = (si32) (KM_SCALEX + 0.5);
  params->cart.km_scaley = (si32) (KM_SCALEY + 0.5);
  params->cart.km_scalez = (si32) (KM_SCALEZ + 0.5);

  strcpy(params->cart.unitsx, "km");

  if (Glob->mode == CartMode ||
      Glob->mode == PpiMode) {
    strcpy(params->cart.unitsy, "km");
  } else {
    /* PolarMode */
    strcpy(params->cart.unitsy, "deg");
  }

  if (Glob->mode == CartMode) {
    strcpy(params->cart.unitsz, "km");
    params->cart.dz_constant = TRUE;
  } else {
    strcpy(params->cart.unitsz, "deg");
    params->cart.dz_constant = FALSE;
  }

}


/**************
 * write_header
 */

static void write_header(rc_table_params_t *params,
			 scan_table_t *scan_table,
			 char *label,
			 FILE *out_file,
			 char *file_path)

{

  char file_label[R_LABEL_LEN];
  int ielev, iaz, iplane;
  si32 **plane_heights, *ll;
  si32 nheights;
  si32 nbytes_char = N_CART_PARAMS_LABELS * R_LABEL_LEN;
  double scalez, minz, dz;
  rc_table_params_t tmp_params;
  scan_table_elev_t *elev;

  /*
   * write file label
   */

  memset ((void *) file_label,
          (int) 0, (size_t)  R_LABEL_LEN);
  strncpy(file_label, label, R_LABEL_LEN);

  fseek(out_file, 0L, SEEK_SET);

  if (ufwrite(file_label, sizeof(char),
	      R_FILE_LABEL_LEN, out_file) != R_FILE_LABEL_LEN) {
    fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing file label %s.\n", label);
    perror(file_path);
    exit(-1);
  }
  
  /*
   * code params into network byte order
   */

  tmp_params = *params;
  BE_from_array_32((ui32 *) &tmp_params,
		   (ui32) (sizeof(rc_table_params_t) - nbytes_char));
  
  /*
   * write table params
   */

  if (ufwrite((char *) &tmp_params,
	      sizeof(rc_table_params_t), 1, out_file) != 1) {
    fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing radar-to-cart params.\n");
    perror(file_path);
    exit(-1);
  }
  
  /*
   * write elevation angles
   */

  check_long_array(scan_table->nelevations);

  /*
   * put target elevations into longs and code into
   * network byte order
   */

  for (ielev = 0; ielev < scan_table->nelevations; ielev++)
    Long_array[ielev] =
      (si32) (scan_table->elev_angles[ielev] * DEG_FACTOR + 0.5);

  BE_from_array_32((ui32 *) Long_array,
		   (ui32) (scan_table->nelevations * sizeof(si32)));
  
  if (ufwrite((char *) Long_array, sizeof(si32),
	      (int) scan_table->nelevations, out_file) !=
      scan_table->nelevations) {
    fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing elevations array.\n");
    perror(file_path);
    exit(-1);
  }

  if (scan_table->use_azimuth_table) {
    
    check_long_array(scan_table->nbeams_vol);
    
    /*
     * write out nazimuths array
     */

    elev = scan_table->elevs;
    for (ielev = 0; ielev < scan_table->nelevations; ielev++, elev++) {
      Long_array[ielev] = elev->naz;
    }
    
    BE_from_array_32((ui32 *) Long_array,
		     (ui32) (scan_table->nelevations * sizeof(si32)));
    
    if (ufwrite((char *) Long_array, sizeof(si32),
		(int) scan_table->nelevations, out_file) !=
	scan_table->nelevations) {
      fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	      Glob->prog_name);
      fprintf(stderr, "Writing nazimuths array.\n");
      perror(file_path);
      exit(-1);
    }

    /*
     * write out azimuths array
     */

    elev = scan_table->elevs;
    ll = Long_array;
    for (ielev = 0; ielev < scan_table->nelevations; ielev++, elev++) {
      for (iaz = 0; iaz < elev->naz; iaz++, ll++) {
	*ll = (si32) (elev->azs[iaz].angle * DEG_FACTOR + 0.5);
      } /* iaz */
    } /* ielev */

    BE_from_array_32((ui32 *) Long_array,
		     (ui32) (scan_table->nbeams_vol * sizeof(si32)));
    
    if (ufwrite((char *) Long_array, sizeof(si32),
		(int) scan_table->nbeams_vol, out_file) !=
	scan_table->nbeams_vol) {
      fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	      Glob->prog_name);
      fprintf(stderr, "Writing azimuths array.\n");
      perror(file_path);
      exit(-1);
    }

  } /* if (scan_table->use_azimuth_table) */
  
  /*
   * write plane heights
   */
  
  nheights = Glob->nz * N_PLANE_HEIGHT_VALUES;
  
  plane_heights = (si32 **) ucalloc2
    ((ui32) Glob->nz, (ui32) N_PLANE_HEIGHT_VALUES,
     sizeof(si32));
  
  scalez = (double) params->cart.scalez;
  minz = (double) params->cart.minz / scalez;	  
  dz = (double) params->cart.dz / scalez;

  if (Glob->mode == CartMode) {
  
    for (iplane = 0; iplane < Glob->nz; iplane++) {

      plane_heights[iplane][PLANE_BASE_INDEX] = (si32)
	((minz + ((double) iplane - 0.5) * dz) * scalez + 0.5);
      
      plane_heights[iplane][PLANE_MIDDLE_INDEX] = (si32)
	((minz + ((double) iplane) * dz) * scalez + 0.5);
      
      plane_heights[iplane][PLANE_TOP_INDEX] = (si32)
	((minz + ((double) iplane + 0.5) * dz) * scalez + 0.5);
      
    } /* iplane */
  
  } else {

    /* PpiMode or PolarMode */

    /*
     * middle heights from elevation angles
     */
    
    for (iplane = 0; iplane < Glob->nz; iplane++) {
      plane_heights[iplane][PLANE_MIDDLE_INDEX] = (si32)
	(si32) (scan_table->elev_angles[iplane] * KM_SCALEZ + 0.5);
    } /* iplane */
    
    /*
     * base heights
     */
    
    for (iplane = 1; iplane < Glob->nz; iplane++) {
      plane_heights[iplane][PLANE_BASE_INDEX] =
	(plane_heights[iplane - 1][PLANE_MIDDLE_INDEX] +
	 plane_heights[iplane][PLANE_MIDDLE_INDEX]) / 2;
    }

    plane_heights[0][PLANE_BASE_INDEX] =
      plane_heights[0][PLANE_MIDDLE_INDEX] -
      (plane_heights[1][PLANE_MIDDLE_INDEX] -
       plane_heights[0][PLANE_MIDDLE_INDEX]) / 2;

    /*
     * top heights
     */
    
    for (iplane = 0; iplane < Glob->nz - 1; iplane++) {
      plane_heights[iplane][PLANE_TOP_INDEX] =
	(plane_heights[iplane][PLANE_MIDDLE_INDEX] +
	 plane_heights[iplane + 1][PLANE_MIDDLE_INDEX]) / 2;
    }

    plane_heights[Glob->nz - 1][PLANE_TOP_INDEX] =
      plane_heights[Glob->nz - 1][PLANE_MIDDLE_INDEX] +
      (plane_heights[Glob->nz - 1][PLANE_MIDDLE_INDEX] -
       plane_heights[Glob->nz - 2][PLANE_MIDDLE_INDEX]) / 2;

  } /* if (Glob->mode == CartMode) */
    
  BE_from_array_32((ui32 *) *plane_heights,
		   (ui32) (nheights * sizeof(si32)));
  
  if (ufwrite((char *) *plane_heights, sizeof(si32),
	      (int) nheights, out_file) != nheights) {
    fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing plane height limits.\n");
    perror(file_path);
    exit(-1);
  }

  ufree2((void **) plane_heights);

}

/*******************************
 * alloc array for reading longs
 */

static void check_long_array(int nlongs_needed)
{

  if (nlongs_needed > Nlongs_alloc) {
    if (Long_array == NULL) {
      Long_array = (si32 *) umalloc (nlongs_needed * sizeof(si32));
    } else {
      Long_array = (si32 *) urealloc ((char *) Long_array,
				      nlongs_needed * sizeof(si32));
    }
    Nlongs_alloc = nlongs_needed;
  }

}

/******************************
 * free array for reading longs
 */

static void free_long_array(void)
{
  ufree((char *) Long_array);
  Long_array = NULL;
  Nlongs_alloc = 0;
}
  
