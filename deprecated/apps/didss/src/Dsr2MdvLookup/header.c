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
 * header.c
 *
 * Handles writing headers to file.
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
#include <rapformats/ds_radar.h>

/*
 * file scope variables
 */

static fl32 *Float_array = NULL;
static int Nfloats_alloc = 0;
static si32 *Long_array = NULL;
static int Nlongs_alloc = 0;

/*
 * file scope prototypes
 */

static void check_float_array(int nfloats_needed);
static void free_float_array(void);
static void check_long_array(int nlongs_needed);
static void free_long_array(void);

/***************
 * header_size()
 *
 * returns size of header
 *
 * header format
 *
 * file_label   : char[DS_FILE_LABEL_LEN]
 *
 * table_params : P2mdv_lookup_params_t
 *
 * elevations : fl32[]
 *  For each elevation: elev angles (deg) (nelevations)
 *
 * If (table_params.use_scan_table == TRUE) {
 *   nazimuths array: si32[]
 *     For each elevation: nazimuths
 *   azimuths array: fl32[]
 *     array of azimuths: azimuths[]
 * }
 *
 * See didss/polarmdv_lookup.h.
 */

int header_size(radar_scan_table_t *scan_table)

{

  int header_size;

  if (Glob->params.use_azimuth_table) {
    header_size = (DS_LABEL_LEN +
		   sizeof(P2mdv_lookup_params_t) +
		   scan_table->nelevations * sizeof(fl32) +
		   scan_table->nelevations * sizeof(si32) +
		   scan_table->nbeams_vol * sizeof(si32));
    
  } else {
    header_size = (DS_LABEL_LEN +
		   sizeof(P2mdv_lookup_params_t) +
		   scan_table->nelevations * sizeof(fl32));
  }

  return (header_size);

}
  
/****************
 * load up params
 */

void load_params(P2mdv_lookup_params_t *lparams,
		 radar_scan_table_t *scan_table,
		 si32 list_offset,
		 si32 header_size,
		 si32 index_size)

{

  si32 nbytes_char;

  /*
   * load up rc_params structure for rc table
   */

  nbytes_char = N_MDV_RADAR_GRID_LABELS * MDV_RADAR_LABEL_LEN;

  lparams->nbytes_char = nbytes_char;

  lparams->geom = Glob->geom;
  lparams->use_azimuth_table = scan_table->use_azimuth_table;
  lparams->extend_below = scan_table->extend_below;
  lparams->missing_data_index = scan_table->missing_data_index;

  lparams->nelevations = scan_table->nelevations;
  lparams->nazimuths = scan_table->nazimuths;
  lparams->ngates = scan_table->ngates;
  lparams->nbeams_vol = scan_table->nbeams_vol;

  lparams->delta_azimuth = scan_table->delta_azimuth;
  lparams->start_azimuth = scan_table->start_azimuth;
  lparams->beam_width = scan_table->beam_width;

  lparams->start_range = scan_table->start_range;
  lparams->gate_spacing = scan_table->gate_spacing;

  lparams->radar_latitude = Glob->params.radar_location.latitude;
  lparams->radar_longitude = Glob->params.radar_location.longitude;

  lparams->ndata = Glob->nx * Glob->ny * Glob->nz;
  lparams->nlist = list_offset;

  lparams->index_offset = header_size;
  lparams->list_offset = lparams->index_offset + index_size;

  lparams->grid.nbytes_char = nbytes_char;

  lparams->grid.latitude = Glob->params.output_grid.origin_lat;
  lparams->grid.longitude = Glob->params.output_grid.origin_lon;
  lparams->grid.rotation = Glob->params.output_grid.rotation;

  lparams->grid.nx = Glob->nx;
  lparams->grid.ny = Glob->ny;
  lparams->grid.nz = Glob->nz;

  lparams->grid.minx = Glob->minx;
  lparams->grid.miny = Glob->miny;
  lparams->grid.minz = Glob->minz;

  lparams->grid.dx = Glob->dx;
  lparams->grid.dy = Glob->dy;
  lparams->grid.dz = Glob->dz;

  lparams->grid.radarx = Glob->radarx;
  lparams->grid.radary = Glob->radary;
  lparams->grid.radarz = Glob->radarz;

  memset (lparams->grid.unitsx, 0, MDV_RADAR_LABEL_LEN);
  memset (lparams->grid.unitsy, 0, MDV_RADAR_LABEL_LEN);
  memset (lparams->grid.unitsz, 0, MDV_RADAR_LABEL_LEN);

  strcpy(lparams->grid.unitsx, "km");

  if (Glob->geom == P2MDV_CART ||
      Glob->geom == P2MDV_PPI) {
    strcpy(lparams->grid.unitsy, "km");
  } else {
    /* PolarGeom */
    strcpy(lparams->grid.unitsy, "deg");
  }

  if (Glob->geom == P2MDV_CART) {
    strcpy(lparams->grid.unitsz, "km");
    lparams->grid.dz_constant = TRUE;
  } else {
    strcpy(lparams->grid.unitsz, "deg");
    lparams->grid.dz_constant = FALSE;
  }

}


/**************
 * write_header
 */

void write_header(P2mdv_lookup_params_t *params,
		  radar_scan_table_t *scan_table,
		  char *label,
		  FILE *out_file,
		  char *file_path)

{

  char file_label[DS_LABEL_LEN];
  int ielev, iaz;
  fl32 *fl;
  si32 nbytes_char = N_MDV_RADAR_FIELD_LABELS * MDV_RADAR_LABEL_LEN;
  P2mdv_lookup_params_t tmp_params;
  radar_scan_table_elev_t *elev;

  /*
   * write file label
   */

  memset ((void *) file_label,
          (int) 0, (size_t)  DS_LABEL_LEN);
  strncpy(file_label, label, DS_LABEL_LEN);

  fseek(out_file, 0L, SEEK_SET);

  if (ufwrite(file_label, sizeof(char),
	      DS_FILE_LABEL_LEN, out_file) != DS_FILE_LABEL_LEN) {
    fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing file label %s.\n", label);
    perror(file_path);
    tidy_and_exit(-1);
  }
  
  /*
   * code params into network byte order
   */

  tmp_params = *params;
  BE_from_array_32((ui32 *) &tmp_params,
		   (ui32) (sizeof(P2mdv_lookup_params_t) - nbytes_char));
  
  /*
   * write table params
   */

  if (ufwrite((char *) &tmp_params,
	      sizeof(P2mdv_lookup_params_t), 1, out_file) != 1) {
    fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing radar-to-cart params.\n");
    perror(file_path);
    tidy_and_exit(-1);
  }
  
  /*
   * write elevation angles
   */

  check_float_array(scan_table->nelevations);
  memcpy(Float_array, scan_table->elev_angles,
	 scan_table->nelevations * sizeof(fl32));
  BE_from_array_32((ui32 *) Float_array,
		   (scan_table->nelevations * sizeof(fl32)));
  
  if (ufwrite((char *) Float_array, sizeof(fl32),
	      (int) scan_table->nelevations, out_file) !=
      scan_table->nelevations) {
    fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	    Glob->prog_name);
    fprintf(stderr, "Writing elevations array.\n");
    perror(file_path);
    tidy_and_exit(-1);
  }

  if (scan_table->use_azimuth_table) {
    
    /*
     * write out nazimuths array
     */

    check_long_array(scan_table->nelevations);
    elev = scan_table->elevs;
    for (ielev = 0; ielev < scan_table->nelevations; ielev++, elev++) {
      Long_array[ielev] = elev->naz;
    }
    BE_from_array_32((ui32 *) Long_array,
		     (scan_table->nelevations * sizeof(si32)));
    
    if (ufwrite((char *) Long_array, sizeof(si32),
		(int) scan_table->nelevations, out_file) !=
	scan_table->nelevations) {
      fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	      Glob->prog_name);
      fprintf(stderr, "Writing nazimuths array.\n");
      perror(file_path);
      tidy_and_exit(-1);
    }

    /*
     * write out azimuths array
     */

    check_float_array(scan_table->nbeams_vol);
    elev = scan_table->elevs;
    fl = Float_array;
    for (ielev = 0; ielev < scan_table->nelevations; ielev++, elev++) {
      for (iaz = 0; iaz < elev->naz; iaz++, fl++) {
	*fl = elev->azs[iaz].angle;
      } /* iaz */
    } /* ielev */

    BE_from_array_32((ui32 *) Float_array,
		     (scan_table->nbeams_vol * sizeof(fl32)));
    
    if (ufwrite((char *) Float_array, sizeof(fl32),
		(int) scan_table->nbeams_vol, out_file) !=
	scan_table->nbeams_vol) {
      fprintf(stderr, "ERROR - %s:invert_table_and_write_files\n",
	      Glob->prog_name);
      fprintf(stderr, "Writing azimuths array.\n");
      perror(file_path);
      tidy_and_exit(-1);
    }

  } /* if (scan_table->use_azimuth_table) */

  free_long_array();
  free_float_array();
  
}

/*******************************
 * alloc array for reading floats
 */

static void check_float_array(int nfloats_needed)
{

  if (nfloats_needed > Nfloats_alloc) {
    if (Float_array == NULL) {
      Float_array = (fl32 *) umalloc (nfloats_needed * sizeof(fl32));
    } else {
      Float_array = (fl32 *) urealloc ((char *) Float_array,
				       nfloats_needed * sizeof(fl32));
    }
    Nfloats_alloc = nfloats_needed;
  }

}

/******************************
 * free array for reading floats
 */

static void free_float_array(void)
{
  ufree(Float_array);
  Float_array = NULL;
  Nfloats_alloc = 0;
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
  ufree(Long_array);
  Long_array = NULL;
  Nlongs_alloc = 0;
}

