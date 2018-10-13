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
/*************************************************************************
 *
 * P2mdv_lookup.c
 *
 * P2mdv lookup table access routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1997
 *
 **************************************************************************/

#include <dataport/bigend.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <rapformats/polar2mdv_lookup.h>

#define MAX_SEQ 256
#define _FILE_LABEL_LEN 40

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

static int read_scan_table(radar_scan_table_t *stable,
			   si32 nelevations,
			   FILE *index_file,
			   char *file_path,
			   char *prog_name,
			   char *calling_routine);

/*************************************************************************
 *
 * FreeP2mdvIndex()
 *
 * frees the memory associated with the file index
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int FreeP2mdvIndex(P2mdv_lookup_file_index_t *index,
		   char *calling_routine)

{

  if (index->index_initialized) {
    
    STRfree(index->prog_name);
    
    if (index->file_path != NULL) {
      STRfree(index->file_path);
      index->file_path = NULL;
    }

    if (index->file_label != NULL) {
      STRfree(index->file_label);
      index->file_label = NULL;
    }

    index->file = (FILE *) NULL;
    index->index_initialized = FALSE;

  }

  return (0);

}

/*************************************************************************
 *
 * InitP2mdvIndex()
 *
 * initializes the memory associated with a generic dual file index
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int InitP2mdvIndex(P2mdv_lookup_file_index_t *index,
		   int size,
		   char *prog_name,
		   char *file_path,
		   char *file_label,
		   FILE *file,
		   char *calling_routine)
     
{

  /*
   * set fields in index
   */

  memset ((void *)  index,
          (int) 0, (size_t)  size);

  index->prog_name = (char *) umalloc
    ((ui32) (strlen(prog_name) + 1));

  strcpy(index->prog_name, prog_name);
  
  if (file_path != NULL) {

    index->file_path = (char *) umalloc
      ((ui32) (strlen(file_path) + 1));

    strcpy(index->file_path, file_path);

  }

  if (file_label != NULL) {

    index->file_label = (char *) umalloc
      ((ui32) _FILE_LABEL_LEN);

    memset ((void *) index->file_label,
            (int) 0, (size_t)  _FILE_LABEL_LEN);

    strcpy(index->file_label, file_label);
    
  }

  index->file = file;

  index->index_initialized = TRUE;

  return (0);

}

/*************************************************************************
 *
 * FreeP2mdvLookup()
 *
 * Frees up the radar to cartesian lookup table.
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

/*ARGSUSED*/

int FreeP2mdvLookup(P2mdv_lookup_file_index_t *index,
		    char *calling_routine)
     
{
  
  ufree(index->lookup_params);
  ufree2((void **) index->lookup_index);
  ufree(index->list);
  RadarFreeScanTableArrays(index->scan_table);
  ufree(index->scan_table);

  index->lookup_params = NULL;
  index->lookup_index = NULL;
  index->list = NULL;
  index->scan_table = NULL;
  index->scan_table = NULL;

  free_float_array();
  free_long_array();

  return(0);

}

/*************************************************************************
 *
 * ReadP2mdvLookup()
 *
 * Reads in the radar to cartesian lookup table.
 *
 * If heights_in_km is set to TRUE, the plane heights will
 * be in km. Otherwise they will be in the grid units.
 *
 * Memory allocation is taken care of in this routine. To free up
 * this memory, use DsFreeP2mdvLookup()
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int ReadP2mdvLookup(P2mdv_lookup_file_index_t *index,
		      char *file_path,
		      char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  char file_label[_FILE_LABEL_LEN];
  char *list = NULL;
  
  si32 nbytes_char;
  
  si32 ielev, iaz;
  si32 nelevations;
  si32 nazimuths;
  /* si32 nplanes; */
  si32 nlist;
  
  P2mdv_lookup_params_t *lparams;
  P2mdv_lookup_index_t **lookup_index;
  radar_scan_table_t *stable;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, "ReadP2mdvLookup");

  /*
   * open file
   */
  
  if (index->file_path != NULL) {
    STRfree(index->file_path);
  }
  index->file_path = STRdup(file_path);
  if ((index->file =
       ta_fopen_uncompress(index->file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:%s:ReadP2mdvLookup\n",
	    index->prog_name, calling_routine);
    fprintf(stderr, "Cannot open lookup table file.\n");
    perror(index->file_path);
    return (-1);
  }
  
  /*
   * read in file label
   */
  
  if (ta_fread(file_label, sizeof(char), _FILE_LABEL_LEN,
	       index->file) != _FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:ReadP2mdvLookup\n",
	    index->prog_name, calling_routine);
    fprintf(stderr, "Reading file label.\n");
    perror(index->file_path);
    return (-1);
    
  }
  
  /*
   * Check that the label is correct
   */
  
  if (strcmp(file_label, index->file_label)) {
    
    fprintf(stderr, "ERROR - %s:%s:ReadP2mdvLookup\n",
	    index->prog_name, calling_routine);
    fprintf(stderr, "File label does not match requested label.\n");
    fprintf(stderr, "File label is '%s'\n", file_label);
    fprintf(stderr, "Requested label is '%s'\n",
	    index->file_label);
    return (-1);
    
  }
  
  /*
   * allocate space for table params
   */
  
  index->lookup_params = (P2mdv_lookup_params_t *)
    umalloc((ui32) sizeof(P2mdv_lookup_params_t));
    
  index->scan_table = (radar_scan_table_t *)
    umalloc((ui32) sizeof(radar_scan_table_t));

  lparams = index->lookup_params;
  stable = index->scan_table;
  
  /*
   * read in table params
   */
  
  if (ta_fread((char *) lparams,
	       sizeof(P2mdv_lookup_params_t),
	       1, index->file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:ReadP2mdvLookup\n",
	    index->prog_name, calling_routine);
    fprintf(stderr, "Reading table params.\n");
    perror(index->file_path);
    return (-1);
    
  }
  
  nbytes_char = BE_to_si32(lparams->nbytes_char);
  
  BE_to_array_32((ui32 *) lparams,
		 (sizeof(P2mdv_lookup_params_t) - nbytes_char));

  /*
   * set local variables
   */
  
  nelevations = lparams->nelevations;
  nazimuths = lparams->nazimuths;
  /* nplanes = lparams->grid.nz; */
  nlist = lparams->nlist;
  
  RadarInitScanTable(stable);

  stable->nelevations = lparams->nelevations;
  stable->nazimuths = lparams->nazimuths;
  stable->ngates = lparams->ngates;
  stable->nbeams_vol = lparams->nbeams_vol;
  stable->delta_azimuth = lparams->delta_azimuth;
  stable->start_azimuth = lparams->start_azimuth;
  stable->beam_width = lparams->beam_width;
  stable->gate_spacing = lparams->gate_spacing;
  stable->start_range = lparams->start_range;
  stable->extend_below = lparams->extend_below;
  stable->missing_data_index = lparams->missing_data_index;
  stable->use_azimuth_table = lparams->use_azimuth_table;

  /*
   * read in scan details
   */

  if (read_scan_table(stable, nelevations, index->file,
		      index->file_path, index->prog_name,
		      calling_sequence) != 0) {
    return (-1);
  }

  /*
   * allocate P2mdv_lookup_index array
   */
  
  index->lookup_index = (P2mdv_lookup_index_t **) ucalloc2
    (nelevations, nazimuths, sizeof(P2mdv_lookup_index_t));
  lookup_index = index->lookup_index;
  
  /*
   * read in P2mdv_lookup_index array
   */
  
  for (ielev = 0; ielev < nelevations; ielev++) {
    if (ta_fread((char *) lookup_index[ielev],
		 sizeof(P2mdv_lookup_index_t),
		 nazimuths, index->file) != nazimuths) {
      
      fprintf(stderr, "ERROR - %s:%s:ReadP2mdvLookup\n",
	      index->prog_name, calling_routine);
      fprintf(stderr, "Reading lookup index.\n");
      perror(index->file_path);
      return (-1);
    }
  }
  
  if (lparams->geom != P2MDV_POLAR) {
    
    /*
     * allocate list array
     */
    
    index->list = (char *) umalloc (nlist);
    list = index->list;
    
    /*
     * read in list array
     */
    
    if (ta_fread(list, sizeof(char), nlist, index->file) != nlist) {
      
      fprintf(stderr, "ERROR - %s:%s:ReadP2mdvLookup\n",
	      index->prog_name, calling_routine);
      fprintf(stderr, "Reading rc table list.\n");
      perror(index->file_path);
      return (-1);
      
    }

  }
  
  /*
   * decode the shorts and longs in the index, and set pointers relative to
   * memory instead of offsets relative to the start of the list. The file
   * is stored in network byte order
   */
  
  for (ielev = 0; ielev < nelevations; ielev++) {
    
    for (iaz = 0; iaz < nazimuths; iaz++) {
      
      BE_to_array_16(&lookup_index[ielev][iaz].npoints, sizeof(si16));
      BE_to_array_16(&lookup_index[ielev][iaz].last_gate_active, sizeof(si16));
      BE_to_array_32(&lookup_index[ielev][iaz].u.offset, sizeof(si32));
      if (lparams->geom != P2MDV_POLAR) {
	lookup_index[ielev][iaz].u.entry =
	  (P2mdv_lookup_entry_t *)(list + lookup_index[ielev][iaz].u.offset);
      }
      
    } /* iaz */
    
  } /* ielev */
  
  if (lparams->geom != P2MDV_POLAR) {
    
    /*
     * decode the fields in the list
     */
    
    BE_to_array_32(list, nlist);

  }
  
  /*
   * close the file
   */
  
  fclose(index->file);
  free_long_array();
  
  return(0);
  
}

#undef THIS_ROUTINE

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

/**********************
 * read in scan details
 */

static int read_scan_table(radar_scan_table_t *stable,
			   si32 nelevations,
			   FILE *index_file,
			   char *file_path,
			   char *prog_name,
			   char *calling_routine)
     
{

  si32 beam_num;
  si32 ielev, iaz;
  fl32 *fl;
  radar_scan_table_elev_t *elev;

  /*
   * allocate radar elevations arrays
   */
  
  RadarAllocScanTableElevs(stable, nelevations);
  
  /*
   * read in radar_elevations array, decode from network
   * byte order
   */
  
  check_float_array(stable->nelevations);
  
  if (ta_fread((char *) Float_array,
	       sizeof(fl32), stable->nelevations,
	       index_file) != stable->nelevations) {
    
    fprintf(stderr, "ERROR - %s:%s:read_scan_table\n",
	    prog_name, calling_routine);
    fprintf(stderr, "Reading radar elevation array.\n");
    perror(file_path);
    free_float_array();
    return (-1);
    
  }
  
  BE_to_array_32((ui32 *) Float_array,
		 (stable->nelevations * sizeof(fl32)));

  memcpy(stable->elev_angles, Float_array,
	 stable->nelevations * sizeof(fl32));
  
  /*
   * compute elevation limits
   */
  
  RadarComputeScanTableElevLimits(stable);
  RadarComputeScanTableExtElev(stable);

  /*
   * if scan table, read in nazimuths and azimuths array
   */

  if (stable->use_azimuth_table) {
    
    /*
     * read in nazimuths
     */
    
    check_long_array(stable->nelevations);
    
    if (ta_fread((char *) Long_array,
		 sizeof(si32),
		 (int) stable->nelevations,
		 index_file) != stable->nelevations) {
      fprintf(stderr, "ERROR - %s:%s:read_scan_table\n",
	      prog_name, calling_routine);
      fprintf(stderr, "Reading nazimuths array.\n");
      perror(file_path);
      free_long_array();
      free_float_array();
      return (-1);
    }
    
    BE_to_array_32((ui32 *) Long_array,
		   (nelevations * sizeof(si32)));
    
    stable->max_azimuths = 0;
    elev = stable->elevs;
    for (ielev = 0; ielev < nelevations; ielev++, elev++) {
      elev->naz = Long_array[ielev];
      stable->max_azimuths = MAX(stable->max_azimuths, elev->naz);
    }
    
    /*
     * read in azimuths
     */
    
    check_float_array(stable->nbeams_vol);
    
    if (ta_fread((char *) Float_array,
		 sizeof(fl32), stable->nbeams_vol,
		 index_file) != stable->nbeams_vol) {
      fprintf(stderr, "ERROR - %s:%s:read_scan_table\n",
	      prog_name, calling_routine);
      fprintf(stderr, "Reading azimuths array.\n");
      perror(file_path);
      free_long_array();
      free_float_array();
      return (-1);
    }
    
    BE_to_array_32((ui32 *) Float_array,
		   (stable->nbeams_vol * sizeof(fl32)));
    
    beam_num = 0;
    elev = stable->elevs;
    fl = Float_array;
    for (ielev = 0; ielev < nelevations; ielev++, elev++) {
      RadarAllocScanTableAzArrays(stable, ielev, elev->naz);
      elev->start_beam_num = beam_num;
      for (iaz = 0; iaz < elev->naz; iaz++, fl++) {
	elev->azs[iaz].angle = *fl;
	elev->azs[iaz].beam_num = beam_num;
	elev->end_beam_num = beam_num;
	beam_num++;
      } /* iaz */
    } /* ielev */
    
    if (beam_num != stable->nbeams_vol) {
      fprintf(stderr, "ERROR - %s:%s:read_scan_table - file '%s'\n",
	      prog_name, calling_routine, file_path);
      fprintf(stderr, "Reading azimuths array.\n");
      fprintf(stderr, "Incorrect nbeams_vol = %ld, should be %ld\n",
	      (long) beam_num, (long) stable->nbeams_vol);
      free_long_array();
      free_float_array();
      return (-1);
    }
    
    /*
     * compute the azimuth limits
     */
    
    RadarComputeScanTableAzLimits(stable);
    
  } /* if (tparams->use_scan_table) */

  free_long_array();
  free_float_array();
  return (0);

}

