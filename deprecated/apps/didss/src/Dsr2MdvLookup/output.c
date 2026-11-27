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
 * output.c
 *
 * File output
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Dsr2MdvLookup.h"
#include <dataport/bigend.h>
#include <toolsa/file_io.h>

/*
 * file scope prototypes
 */

static si32 get_maxpergate(si32 *rindex_table,
			   ppi_table_t **ppi_table,
			   radar_scan_table_t *scan_table);

/*********************************************************************
 * write_cart_or_ppi()
 *
 * For geom types P2mdv_CART and P2mdv_PPI
 *
 * Inverts the lookup table to allow one to look up the series
 * of relevant cartesian points given the radar coordinate.
 * Writes the polar2mdv lookup tables to file.
 *
 *********************************************************************/

void write_cart_or_ppi(si32 *rindex_table,
		       radar_scan_table_t *scan_table)

{

  ui16 ngates_active, last_gate_active;

  si32 i, ielev, iaz, igate, icart, ipoint;
  si32 npoints, tot_points;
  si32 maxpergate;
  si32 lookup_list_offset = 0;
  si32 lookup_index_size;
  si32 ncart, start_index, end_index, offset;
  si32 nbeams_array;
  si32 *index;

  P2mdv_lookup_index_t **P2mdv_lookup_index;
  ppi_table_t **ppi_table;
  P2mdv_lookup_entry_t *lookup_entry;
  P2mdv_lookup_params_t lookup_params;
  radar_scan_table_elev_t *elev;
  path_parts_t parts;

  FILE *lookup_file;

  /*
   * allocate array for the P2mdv_lookup structures
   */

  P2mdv_lookup_index = (P2mdv_lookup_index_t **) ucalloc2
    ((ui32) scan_table->nelevations,
     (ui32) scan_table->max_azimuths,
     (ui32) sizeof(P2mdv_lookup_index_t));

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

  nbeams_array = scan_table->nazimuths * scan_table->nelevations;
  lookup_index_size = nbeams_array * sizeof(P2mdv_lookup_index_t);

  /*
   * make dir if necessary
   */

  uparse_path(Glob->params.lookup_table_path, &parts);
  if (ta_makedir_recurse(parts.dir)) {
    fprintf(stderr, "ERROR - %s:write_cart_or_ppi.\n", Glob->prog_name);
    fprintf(stderr, "  Unable to create tabel dir: %s\n", parts.dir);
    tidy_and_exit(-1);
  }
  ufree_parsed_path(&parts);
  
  /*
   * open file
   */

  if ((lookup_file = fopen(Glob->params.lookup_table_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:write_cart_or_ppi.\n", Glob->prog_name);
    fprintf(stderr, "Unable to create rc table file.\n");
    perror(Glob->params.lookup_table_path);
    tidy_and_exit(-1);
  }

  /*
   * space fwd over area for rctable_params header and the index array
   */

  fseek(lookup_file, header_size(scan_table) + lookup_index_size, SEEK_SET);

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
	umalloc((ui32) (maxpergate * sizeof(ui32)));
    }
  }

  /*
   * main loop through the elevations
   */

  elev = scan_table->elevs;
  for (ielev = 0; ielev < scan_table->nelevations; ielev++, elev++) {

    printf("write_cart_or_ppi: %s%d\n",
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

    printf("write_cart_or_ppi: %s\n",
	   "loading P2mdv_lookup structs, writing to file.");

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

      P2mdv_lookup_index[ielev][iaz].npoints = tot_points;
      P2mdv_lookup_index[ielev][iaz].last_gate_active = last_gate_active;
      P2mdv_lookup_index[ielev][iaz].u.offset = lookup_list_offset;
      
      BE_from_array_16(&P2mdv_lookup_index[ielev][iaz].npoints,
		       (ui32) sizeof(si16));
      BE_from_array_16(&P2mdv_lookup_index[ielev][iaz].last_gate_active,
		       (ui32) sizeof(si16));
      BE_from_array_32(&P2mdv_lookup_index[ielev][iaz].u.offset,
		       (ui32) sizeof(si32));
      
      /*
       * if there are active gates for this azimuth, gather details about
       * the points
       */

      if ((si32) ngates_active > 0) {

	/*
	 * allocate memory for the table entries for this beam
	 */

	lookup_entry = (P2mdv_lookup_entry_t *)
	  umalloc((ui32) tot_points * sizeof(P2mdv_lookup_entry_t));
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
	      lookup_entry[ipoint].index = ppi_table[iaz][igate].index[i];
	      lookup_entry[ipoint].gate = igate;
	      ipoint++;

	    }

	  } /* if (npoints > 0) */

	} /* igate */

	/*
	 * code lookup_entry and index arrays into network byte order
	 */

	BE_from_array_32((ui32 *) lookup_entry,
			 (ui32) (tot_points * sizeof(P2mdv_lookup_entry_t)));
	BE_from_array_32((ui32 *) index,
			 (ui32) (tot_points * sizeof(si32)));
	
	/*
	 * write out lookup_entry array
	 */

	fwrite((char *) lookup_entry, sizeof(P2mdv_lookup_entry_t),
	       (int) tot_points, lookup_file);

	/*
	 * increment list offsets
	 */

	lookup_list_offset += tot_points * sizeof(P2mdv_lookup_entry_t);

	/*
	 * free up arrays
	 */

	ufree((char *) lookup_entry);
	ufree((char *) index);

      } /* if (ngates_active > 0) */

    } /* iaz */

  } /* ielev */

  printf("write_cart_or_ppi: %s\n",
	 "writing table parameters and index arrays to file.");

  /*
   * load up lookup_params structure
   */

  load_params(&lookup_params, scan_table,
	      lookup_list_offset, header_size(scan_table), lookup_index_size);

  /*
   * write file headers
   */

  write_header(&lookup_params, scan_table,
	       POLAR2MDV_LOOKUP_LABEL,
	       lookup_file, Glob->params.lookup_table_path);

  /*
   * write P2mdv_lookup_index
   */

  if (ufwrite((char *) *P2mdv_lookup_index,
	      sizeof(P2mdv_lookup_index_t),
	      (int) nbeams_array,
	      lookup_file) != nbeams_array) {
    fprintf(stderr, "ERROR - %s:write_cart_or_ppi\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing radar-to-cart table index.\n");
    perror(Glob->params.lookup_table_path);
    tidy_and_exit(-1);
  }
  
  /*
   * close file
   */

  fclose(lookup_file);

  /*
   * free up
   */

  ufree2((void **) P2mdv_lookup_index);

  for (iaz = 0; iaz < scan_table->max_azimuths; iaz++) {
    for (igate = 0; igate < scan_table->ngates; igate++) {
      ufree((char *) ppi_table[iaz][igate].index);
    }
  }
  ufree2((void **) ppi_table);

}

/*********************************************************************
 * write_polar()
 *
 * For geom types P2mdv_POLAR
 *
 *********************************************************************/

void write_polar(si32 *rindex_table,
		 radar_scan_table_t *scan_table)

{

  si32 ielev, iaz;
  si32 lookup_list_offset = 0;
  si32 lookup_index_size;
  si32 nbeams_array;

  P2mdv_lookup_index_t **P2mdv_lookup_index;
  P2mdv_lookup_params_t lookup_params;
  radar_scan_table_elev_t *elev;
  path_parts_t parts;

  FILE *lookup_file;

  /*
   * allocate array for the P2mdv_lookup structures
   */

  P2mdv_lookup_index = (P2mdv_lookup_index_t **) ucalloc2
    ((ui32) scan_table->nelevations,
     (ui32) scan_table->max_azimuths,
     (ui32) sizeof(P2mdv_lookup_index_t));

  /*
   * compute constants
   */

  nbeams_array = scan_table->nazimuths * scan_table->nelevations;
  lookup_index_size = nbeams_array * sizeof(P2mdv_lookup_index_t);

  /*
   * make dir if necessary
   */

  uparse_path(Glob->params.lookup_table_path, &parts);
  if (ta_makedir_recurse(parts.dir)) {
    fprintf(stderr, "ERROR - %s:write_polar.\n", Glob->prog_name);
    fprintf(stderr, "  Unable to create tabel dir: %s\n", parts.dir);
    tidy_and_exit(-1);
  }
  ufree_parsed_path(&parts);
  
  /*
   * open file
   */

  if ((lookup_file = fopen(Glob->params.lookup_table_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:write_polar.\n", Glob->prog_name);
    fprintf(stderr, "Unable to create rc table file.\n");
    perror(Glob->params.lookup_table_path);
    tidy_and_exit(-1);
  }

  /*
   * main loop through the elevations
   */
  
  elev = scan_table->elevs;
  lookup_list_offset = 0;
  
  for (ielev = 0; ielev < scan_table->nelevations; ielev++, elev++) {
    
    printf("write_polar: %s%d\n",
	   "loading up ppi table, elev number ", ielev);
    
    /*
     * loop through the azimuths
     */
    
    printf("write_polar: %s\n",
	   "loading P2mdv_lookup structs, writing to file.");
    
    for (iaz = 0; iaz < scan_table->max_azimuths;
	 iaz++, lookup_list_offset += scan_table->ngates) {
      
      /*
       * load up index structures, code into network byte order
       */
      
      P2mdv_lookup_index[ielev][iaz].npoints = scan_table->ngates;
      P2mdv_lookup_index[ielev][iaz].last_gate_active = scan_table->ngates;
      P2mdv_lookup_index[ielev][iaz].u.offset = lookup_list_offset;
      
      BE_from_array_16(&P2mdv_lookup_index[ielev][iaz].npoints,
		       (ui32) sizeof(si16));
      BE_from_array_16(&P2mdv_lookup_index[ielev][iaz].last_gate_active,
		       (ui32) sizeof(si16));
      BE_from_array_32(&P2mdv_lookup_index[ielev][iaz].u.offset,
		       (ui32) sizeof(si32));
      
    } /* iaz */

  } /* ielev */

  printf("write_polar: %s\n",
	 "writing table parameters and index arrays to file.");

  /*
   * load up lookup_params structure
   */

  load_params(&lookup_params, scan_table,
	      0, header_size(scan_table), lookup_index_size);

  /*
   * write file headers
   */

  write_header(&lookup_params, scan_table,
	       POLAR2MDV_LOOKUP_LABEL,
	       lookup_file, Glob->params.lookup_table_path);

  /*
   * write P2mdv_lookup_index
   */

  if (ufwrite((char *) *P2mdv_lookup_index,
	      sizeof(P2mdv_lookup_index_t),
	      (int) nbeams_array,
	      lookup_file) != nbeams_array) {
    fprintf(stderr, "ERROR - %s:write_polar\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing radar-to-cart table index.\n");
    perror(Glob->params.lookup_table_path);
    tidy_and_exit(-1);
  }
  
  /*
   * close file
   */

  fclose(lookup_file);

  /*
   * free up
   */

  ufree2((void **) P2mdv_lookup_index);

}

/******************
 * get_maxpergate()
 */

static si32 get_maxpergate(si32 *rindex_table,
			   ppi_table_t **ppi_table,
			   radar_scan_table_t *scan_table)

{

  si32 ielev, iaz, igate, icart;
  si32 ncart;
  si32 maxpergate = 0;
  si32 start_index, end_index;
  si32 offset;
  radar_scan_table_elev_t *elev;

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

    if (Glob->params.debug) {
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

    printf("  max number of points per gate "
	   "%ld for elev %ld\n", (long) maxpergate, (long) ielev);

  } /* ielev */

  return (maxpergate);

}

