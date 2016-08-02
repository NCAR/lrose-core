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
 * RfTables.c
 *
 * part of the rfutil library - radar file access
 *
 * Table file access routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1991
 *
 **************************************************************************/

#include <titan/file_io.h>
#include <titan/radar.h>
#include <dataport/bigend.h>
#include <toolsa/toolsa_macros.h>

#define MAX_SEQ 256

static si32 *Long_array = NULL;
static int Nlongs_alloc = 0;

/*
 * file scope prototypes
 */

static void check_long_array(int nlongs_needed);

static void free_long_array(void);

static int read_scan_table(scan_table_t *stable,
			   si32 nelevations,
			   FILE *index_file,
			   char *file_path,
			   char *prog_name,
			   char *calling_routine);

static int read_plane_heights(si32 nplanes,
			      si32 ***plane_heights_p,
			      FILE *index_file,
			      char *file_path,
			      char *prog_name,
			      char *calling_routine);

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

static int read_scan_table(scan_table_t *stable,
			   si32 nelevations,
			   FILE *index_file,
			   char *file_path,
			   char *prog_name,
			   char *calling_routine)
     
{

  si32 beam_num;
  si32 ielev, iaz;
  si32 *ll;
  scan_table_elev_t *elev;
  
  /*
   * allocate radar elevations arrays
   */
  
  RfAllocScanTableElevs(stable);
  check_long_array(stable->nelevations);
  
  /*
   * read in radar_elevations array as long, decode from network
   * byte order
   */
  
  if (ufread((char *) Long_array,
	     sizeof(si32),
	     (int) stable->nelevations,
	     index_file) != stable->nelevations) {
    
    fprintf(stderr, "ERROR - %s:%s:read_scan_table\n",
	    prog_name, calling_routine);
    fprintf(stderr, "Reading radar elevation array.\n");
    perror(file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) Long_array,
	    (ui32) (stable->nelevations * sizeof(si32)));
  
  /*
   * load up elevation angles
   */
  
  for (ielev = 0; ielev < stable->nelevations; ielev++) {
    stable->elev_angles[ielev] = (double) Long_array[ielev] / 1000000.0;
  }
  
  /*
   * compute elevation limits
   */

  RfComputeScanTableElevLimits(stable);
  RfComputeScanTableExtElev(stable);

  /*
   * if scan table, read in nazimuths and azimuths array
   */

  if (stable->use_azimuth_table) {

    check_long_array(stable->nbeams_vol);
    
    /*
     * read in nazimuths
     */
    
    if (ufread((char *) Long_array,
	       sizeof(si32),
	       (int) stable->nelevations,
	       index_file) != stable->nelevations) {
      fprintf(stderr, "ERROR - %s:%s:read_scan_table\n",
	      prog_name, calling_routine);
      fprintf(stderr, "Reading nazimuths array.\n");
      perror(file_path);
      return (R_FAILURE);
    }
    
    BE_to_array_32((ui32 *) Long_array,
	      (ui32) (nelevations * sizeof(si32)));

    stable->max_azimuths = 0;
    elev = stable->elevs;
    for (ielev = 0; ielev < nelevations; ielev++, elev++) {
      elev->naz = Long_array[ielev];
      stable->max_azimuths = MAX(stable->max_azimuths, elev->naz);
    }
    
    /*
     * read in azimuths
     */
    
    if (ufread((char *) Long_array,
	       sizeof(si32),
	       (int) stable->nbeams_vol,
	       index_file) != stable->nbeams_vol) {
      fprintf(stderr, "ERROR - %s:%s:read_scan_table\n",
	      prog_name, calling_routine);
      fprintf(stderr, "Reading azimuths array.\n");
      perror(file_path);
      return (R_FAILURE);
    }
    
    BE_to_array_32((ui32 *) Long_array,
	      (ui32) (stable->nbeams_vol * sizeof(si32)));
    
    beam_num = 0;
    elev = stable->elevs;
    ll = Long_array;
    for (ielev = 0; ielev < nelevations; ielev++, elev++) {
      RfAllocScanTableAzArrays(stable, ielev);
      elev->start_beam_num = beam_num;
      for (iaz = 0; iaz < elev->naz; iaz++, ll++) {
	elev->azs[iaz].angle = (double) (*ll) / 1000000.0;
	elev->azs[iaz].beam_num = beam_num;
	elev->end_beam_num = beam_num;
	beam_num++;
      } /* iaz */
    } /* ielev */

    if (beam_num != stable->nbeams_vol) {
      fprintf(stderr, "ERROR - %s:%s:read_scan_table\n",
	      prog_name, calling_routine);
      fprintf(stderr, "Reading azimuths array.\n");
      fprintf(stderr, "Incorrect nbeams_vol = %ld, should be %ld\n",
	      (long) beam_num, (long) stable->nbeams_vol);
      perror(file_path);
      return (R_FAILURE);
    }
    
    /*
     * compute the azimuth limits
     */
    
    RfComputeScanTableAzLimits(stable);
    
  } /* if (tparams->use_scan_table) */

  return (R_SUCCESS);

}

/*****************************
 * read in plane heights array
 */

static int read_plane_heights(si32 nplanes,
			      si32 ***plane_heights_p,
			      FILE *index_file,
			      char *file_path,
			      char *prog_name,
			      char *calling_routine)
     
{

  si32 nheights = nplanes * N_PLANE_HEIGHT_VALUES;
  si32 **plane_heights;

  check_long_array(nheights);
  
  /*
   * read in plane_heights array, decode from network
   * byte order
   */
  
  if (ufread((char *) Long_array,
	     sizeof(si32),
	     (int) nheights,
	     index_file) != nheights) {
    
    fprintf(stderr, "ERROR - %s:%s:read_plane_heights\n",
	    prog_name, calling_routine);
    fprintf(stderr, "Reading plane height limits array.\n");
    perror(file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) Long_array,
	    (ui32) (nheights * sizeof(si32)));
  
  /*
   * allocate index plane heights array
   */
  
  plane_heights = (si32 **) ucalloc2
    ((ui32) nplanes, (ui32) N_PLANE_HEIGHT_VALUES,
     (ui32) sizeof(si32));
  *plane_heights_p = plane_heights;

  /*
   * load plane heights array
   */

  memcpy((void *) *plane_heights, (void *) Long_array,
	 (ui32) (nheights * sizeof(si32)));
  
  return (R_SUCCESS);

}


/*************************************************************************
 *
 * RfFreeClutterTable()
 *
 * part of the rfutil library - radar file access
 *
 * Frees up the clutter table memory
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeClutterTable(clutter_table_file_handle_t *clutter_handle,
		       const char *calling_routine)
     
{
  
  ufree(clutter_handle->table_params);
  ufree(clutter_handle->radar_elevations);
  ufree2((void **) clutter_handle->plane_heights);
  ufree(clutter_handle->table_index);
  ufree(clutter_handle->list);
  return(R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeRcTable()
 *
 * part of the rfutil library - radar file access
 *
 * Frees up the radar to cartesian lookup table.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeRcTable(rc_table_file_handle_t *rc_handle,
		  const char *calling_routine)
     
{
  
  ufree(rc_handle->table_params);
  ufree2((void **) rc_handle->plane_heights);
  ufree2((void **) rc_handle->table_index);
  ufree(rc_handle->list);
  RfFreeScanTableArrays(rc_handle->scan_table);
  ufree(rc_handle->scan_table);
  return(R_SUCCESS);

}

/*************************************************************************
 *
 * RfFreeSlaveTable()
 *
 * part of the rfutil library - radar file access
 *
 * Frees up the radar to cartesian slave lookup table.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

/*ARGSUSED*/

int RfFreeSlaveTable(slave_table_file_handle_t *slave_handle,
		     const char *calling_routine)
     
{
  
  ufree(slave_handle->table_params);
  ufree2((void **) slave_handle->plane_heights);
  ufree2((void **) slave_handle->table_index);
  ufree(slave_handle->list);
  RfFreeScanTableArrays(slave_handle->scan_table);
  ufree(slave_handle->scan_table);
  return(R_SUCCESS);

}

/*************************************************************************
 *
 * RfReadClutterTable()
 *
 * part of the rfutil library - radar file access
 *
 * Reads in a clutter table
 *
 * Memory allocation is taken care of in this routine. To free up
 * this memory, use RfFreeClutterTable()
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfReadClutterTable(clutter_table_file_handle_t *clutter_handle,
		       const char *calling_routine)
     
{
  
  char file_label[R_FILE_LABEL_LEN];
  char *list;
  
  si32 nbytes_char;
  
  si32 i, ielev, iaz;
  si32 nelevations;
  si32 nazimuths;
  si32 nplanes;
  si32 nheights;
  si32 nlist;
  
  clutter_table_params_t *tparams;
  clutter_table_index_t **table_index;
  clutter_table_entry_t *entry;
  
  /*
   * open file
   */
  
  if ((clutter_handle->file =
       Rf_fopen_uncompress(clutter_handle->file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:%s:RfReadClutterTable\n",
	    clutter_handle->prog_name, calling_routine);
    fprintf(stderr, "Cannot open lookup table file.\n");
    perror(clutter_handle->file_path);
    return (R_FAILURE);
  }
  
  /*
   * read in file label
   */
  
  if (ufread(file_label,
	     (int) sizeof(char),
	     (int) R_FILE_LABEL_LEN,
	     clutter_handle->file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadClutterTable\n",
	    clutter_handle->prog_name, calling_routine);
    fprintf(stderr, "ERROR - RfReadClutterTable\n");
    fprintf(stderr, "Reading file label.\n");
    perror(clutter_handle->file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * Check that the label is correct
   */
  
  if (strcmp(file_label, clutter_handle->file_label)) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadClutterTable\n",
	    clutter_handle->prog_name, calling_routine);
    fprintf(stderr, "File label does not match requested label.\n");
    fprintf(stderr, "File label is '%s'\n", file_label);
    fprintf(stderr, "Requested label is '%s'\n",
	    clutter_handle->file_label);
    return (R_FAILURE);
    
  }
  
  /*
   * allocate space for table params
   */
  
  clutter_handle->table_params = (clutter_table_params_t *)
    umalloc((ui32) sizeof(clutter_table_params_t));
    
  tparams = clutter_handle->table_params;
  
  /*
   * read in table params
   */
  
  if (ufread((char *) tparams,
	     (int) sizeof(clutter_table_params_t),
	     1, clutter_handle->file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadClutterTable\n",
	    clutter_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading table params.\n");
    perror(clutter_handle->file_path);
    return (R_FAILURE);
    
  }
  
  nbytes_char = BE_to_si32(tparams->nbytes_char);

  BE_to_array_32((ui32 *) tparams,
		 (ui32) (sizeof(clutter_table_params_t) - nbytes_char));
  
  /*
   * set local variables
   */
  
  nelevations = clutter_handle->table_params->rc_params.nelevations;
  nazimuths = clutter_handle->table_params->rc_params.nazimuths;
  nplanes = clutter_handle->table_params->rc_params.cart.nz;
  nheights = N_PLANE_HEIGHT_VALUES * nplanes;
  nlist = clutter_handle->table_params->nlist;
  
  /*
   * allocate index radar elevations array
   */
  
  clutter_handle->radar_elevations = (si32 *) umalloc
    ((ui32) (nelevations * sizeof(si32)));
    
  /*
   * read in local radar_elevations array as long, decode from network
   * byte order
   */
  
  if (ufread((char *) clutter_handle->radar_elevations,
	     sizeof(si32), (int) nelevations,
	     clutter_handle->file) != nelevations) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadClutterTable\n",
	    clutter_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading radar elevation array.\n");
    perror(clutter_handle->file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) clutter_handle->radar_elevations,
		 (ui32) (nelevations * sizeof(si32)));
  
  /*
   * allocate index plane heights array
   */
  
  clutter_handle->plane_heights = (si32 **) ucalloc2
    ((ui32) nplanes,
     (ui32) N_PLANE_HEIGHT_VALUES,
     (ui32) sizeof(si32));
  
  /*
   * read plane_heights array, decode from network
   * byte order
   */
  
  if (ufread((char *) *clutter_handle->plane_heights,
	     sizeof(si32),
	     (int) nheights,
	     clutter_handle->file) != nheights) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadClutterTable\n",
	    clutter_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading plane height limits array.\n");
    perror(clutter_handle->file_path);
    return (R_FAILURE);
    
  }
  
  BE_to_array_32((ui32 *) *clutter_handle->plane_heights,
		 (ui32) (nheights * sizeof(si32)));
  
  /*
   * allocate clutter_table_index array
   */
  
  clutter_handle->table_index = (clutter_table_index_t **) ucalloc2
    ((ui32) nelevations,
     (ui32) nazimuths,
     (ui32) sizeof(clutter_table_index_t));
  
  table_index = clutter_handle->table_index;
  
  /*
   * read in clutter_table_index array
   */
  
  for (ielev = 0; ielev < nelevations; ielev++) {
    
    if (ufread((char *) table_index[ielev],
	       (int) sizeof(clutter_table_index_t),
	       (int) nazimuths,
	       clutter_handle->file) != nazimuths) {
      
      fprintf(stderr, "ERROR - %s:%s:RfReadClutterTable\n",
	      clutter_handle->prog_name, calling_routine);
      fprintf(stderr, "Reading clutter_table_index.\n");
      perror(clutter_handle->file_path);
      return (R_FAILURE);
      
    }
    
  }
  
  /*
   * allocate list array
   */
  
  clutter_handle->list = (char *) umalloc ((ui32) nlist);
  list = clutter_handle->list;
  
  /*
   * read in list array
   */
  
  if (ufread(list,
	     (int) sizeof(char) ,
	     (int) nlist,
	     clutter_handle->file) != nlist) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadClutterTable\n",
	    clutter_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading clutter table list.\n");
    perror(clutter_handle->file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode the shorts and longs in the index, and set pointers relative
   * to memory instead of offsets relative to the start of the list
   */
  
  for (ielev = 0; ielev < nelevations; ielev++) {
    
    for (iaz = 0; iaz < nazimuths; iaz++) {
      
      /*
       * the fields in the index
       */
      
      BE_to_array_32((ui32 *) &table_index[ielev][iaz].nclut_points,
		     (ui32) sizeof(si32));
      BE_to_array_32((ui32 *) &table_index[ielev][iaz].u.offset,
		     (ui32) sizeof(si32));
      table_index[ielev][iaz].u.entry =
        (clutter_table_entry_t *)(list + table_index[ielev][iaz].u.offset);
      
      /*
       * the fields in the list
       */
      
      for (i = 0; i < (int) table_index[ielev][iaz].nclut_points; i++) {
	
        entry = table_index[ielev][iaz].u.entry + i;
	
        BE_to_array_16((ui16 *) &entry->dbz, (ui32) sizeof(si16));
        BE_to_array_16((ui16 *) &entry->ipoint, (ui32) sizeof(si16));
        BE_to_array_32((ui32 *) &entry->cart_index, (ui32) sizeof(si32));
	
      } /* i */
      
    } /* iaz */
    
  } /* ielev */
  
  /*
   * close the file
   */
  
  fclose(clutter_handle->file);
  
  return(R_SUCCESS);
  
}

/*************************************************************************
 *
 * RfReadRcTable()
 *
 * part of the rfutil library - radar file access
 *
 * Reads in the radar to cartesian lookup table.
 *
 * If heights_in_km is set to TRUE, the plane heights will
 * be in km. Otherwise they will be in the grid units.
 *
 * Memory allocation is taken care of in this routine. To free up
 * this memory, use RfFreeRcTable()
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadRcTable"

int RfReadRcTable(rc_table_file_handle_t *rc_handle,
		  const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  char file_label[R_FILE_LABEL_LEN];
  char *list;
  
  si32 nbytes_char;
  
  si32 ielev, iaz;
  si32 nelevations;
  si32 nazimuths;
  si32 nplanes;
  // si32 nheights;
  si32 nlist;
  
  rc_table_params_t *tparams;
  rc_table_index_t **table_index;
  scan_table_t *stable;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * open file
   */
  
  if ((rc_handle->file =
       Rf_fopen_uncompress(rc_handle->file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:%s:RfReadRcTable\n",
	    rc_handle->prog_name, calling_routine);
    fprintf(stderr, "Cannot open lookup table file.\n");
    perror(rc_handle->file_path);
    return (R_FAILURE);
  }
  
  /*
   * read in file label
   */
  
  if (ufread(file_label,
	     (int) sizeof(char),
	     (int) R_FILE_LABEL_LEN,
	     rc_handle->file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadRcTable\n",
	    rc_handle->prog_name, calling_routine);
    fprintf(stderr, "ERROR - RfReadRcTable\n");
    fprintf(stderr, "Reading file label.\n");
    perror(rc_handle->file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * Check that the label is correct
   */
  
  if (strcmp(file_label, rc_handle->file_label)) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadRcTable\n",
	    rc_handle->prog_name, calling_routine);
    fprintf(stderr, "File label does not match requested label.\n");
    fprintf(stderr, "File label is '%s'\n", file_label);
    fprintf(stderr, "Requested label is '%s'\n",
	    rc_handle->file_label);
    return (R_FAILURE);
    
  }
  
  /*
   * allocate space for table params
   */
  
  rc_handle->table_params = (rc_table_params_t *)
    umalloc((ui32) sizeof(rc_table_params_t));
    
  rc_handle->scan_table = (scan_table_t *)
    umalloc((ui32) sizeof(scan_table_t));

  tparams = rc_handle->table_params;
  stable = rc_handle->scan_table;
  
  /*
   * read in table params
   */
  
  if (ufread((char *) tparams,
	     (int) sizeof(rc_table_params_t),
	     1, rc_handle->file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadRcTable\n",
	    rc_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading table params.\n");
    perror(rc_handle->file_path);
    return (R_FAILURE);
    
  }
  
  memcpy ((void *)  &nbytes_char,
          (void *)  &tparams->nbytes_char,
          (size_t)  sizeof(si32));
  
  BE_to_array_32((ui32 *) &nbytes_char,
	    (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) tparams,
	    (ui32) (sizeof(rc_table_params_t) - nbytes_char));

  /*
   * set local variables
   */
  
  nelevations = tparams->nelevations;
  nazimuths = tparams->nazimuths;
  nplanes = tparams->cart.nz;
  // nheights = N_PLANE_HEIGHT_VALUES * nplanes;
  nlist = tparams->nlist;
  
  stable->nelevations = tparams->nelevations;
  stable->nazimuths = tparams->nazimuths;
  stable->ngates = tparams->ngates;
  stable->nbeams_vol = tparams->nbeams_vol;
  stable->delta_azimuth = (double) tparams->delta_azimuth / 1000000.0;
  stable->start_azimuth = (double) tparams->start_azimuth / 1000000.0;
  stable->beam_width = (double) tparams->beam_width / 1000000.0;
  stable->gate_spacing = (double) tparams->gate_spacing / 1000000.0;
  stable->start_range = (double) tparams->start_range / 1000000.0;
  stable->extend_below = tparams->extend_below;
  stable->missing_data_index = tparams->missing_data_index;
  stable->use_azimuth_table = tparams->use_azimuth_table;

  /*
   * read in scan details
   */

  if (read_scan_table(stable,
		      nelevations,
		      rc_handle->file,
		      rc_handle->file_path,
		      rc_handle->prog_name,
		      calling_sequence) != R_SUCCESS) {
    return (R_FAILURE);
  }

  /*
   * read in plane_heights array
   */
  
  if (read_plane_heights(nplanes,
			 &rc_handle->plane_heights,
			 rc_handle->file,
			 rc_handle->file_path,
			 rc_handle->prog_name,
			 calling_sequence) != R_SUCCESS) {
    return (R_FAILURE);
  }
  
  /*
   * allocate rc_table_index array
   */
  
  rc_handle->table_index = (rc_table_index_t **) ucalloc2
    ((ui32) nelevations,
     (ui32) nazimuths,
     (ui32) sizeof(rc_table_index_t));
  table_index = rc_handle->table_index;
  
  /*
   * read in rc_table_index array
   */
  
  for (ielev = 0; ielev < nelevations; ielev++) {
    
    if (ufread((char *) table_index[ielev],
	       (int) sizeof(rc_table_index_t),
	       (int) nazimuths,
	       rc_handle->file) != nazimuths) {
      
      fprintf(stderr, "ERROR - %s:%s:RfReadRcTable\n",
	      rc_handle->prog_name, calling_routine);
      fprintf(stderr, "Reading rc_table_index.\n");
      perror(rc_handle->file_path);
      return (R_FAILURE);
      
    }
    
  }
  
  /*
   * allocate list array
   */
  
  rc_handle->list = (char *) umalloc ((ui32) nlist);
  list = rc_handle->list;
  
  /*
   * read in list array
   */
  
  if (ufread(list,
	     (int) sizeof(char) ,
	     (int) nlist,
	     rc_handle->file) != nlist) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadRcTable\n",
	    rc_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading rc table list.\n");
    perror(rc_handle->file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode the shorts and longs in the index, and set pointers relative to
   * memory instead of offsets relative to the start of the list. The file
   * is stored in network byte order
   */
  
  for (ielev = 0; ielev < nelevations; ielev++) {
    
    for (iaz = 0; iaz < nazimuths; iaz++) {
      
      BE_to_array_16((ui16 *) &table_index[ielev][iaz].npoints,
		(ui32) sizeof(si16));
      BE_to_array_16((ui16 *) &table_index[ielev][iaz].last_gate_active,
		(ui32) sizeof(si16));
      BE_to_array_32((ui32 *) &table_index[ielev][iaz].u.offset,
		(ui32) sizeof(si32));
      table_index[ielev][iaz].u.entry =
	(rc_table_entry_t *)(list + table_index[ielev][iaz].u.offset);
      
    } /* iaz */
    
  } /* ielev */
  
  /*
   * decode the fields in the list
   */
  
  BE_to_array_32((ui32 *) list, (ui32) nlist);
  
  /*
   * close the file
   */
  
  fclose(rc_handle->file);
  free_long_array();
  
  return(R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfReadSlaveTable()
 *
 * part of the rfutil library - radar file access
 *
 * Reads in the radar to cartesian lookup table.
 *
 * If heights_in_km is set to TRUE, the plane heights will
 * be in km. Otherwise they will be in the grid units.
 *
 * Memory allocation is taken care of in this routine. To free up
 * this memory, use RfFreeSlaveTable()
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadSlaveTable"

int RfReadSlaveTable(slave_table_file_handle_t *slave_handle,
		     const char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  char file_label[R_FILE_LABEL_LEN];
  char *list;
  
  si32 nbytes_char;
  
  si32 ielev, iaz;
  si32 nelevations;
  si32 nazimuths;
  si32 nplanes;
  // si32 nheights;
  si32 nlist;
  
  rc_table_params_t *tparams;
  slave_table_index_t **table_index;
  scan_table_t *stable;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * open file
   */
  
  if ((slave_handle->file =
       Rf_fopen_uncompress(slave_handle->file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:%s:RfReadSlaveTable\n",
	    slave_handle->prog_name, calling_routine);
    fprintf(stderr, "Cannot open lookup table file.\n");
    perror(slave_handle->file_path);
    return (R_FAILURE);
  }
  
  /*
   * read in file label
   */
  
  if (ufread(file_label,
	     (int) sizeof(char),
	     (int) R_FILE_LABEL_LEN,
	     slave_handle->file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadSlaveTable\n",
	    slave_handle->prog_name, calling_routine);
    fprintf(stderr, "ERROR - RfReadSlaveTable\n");
    fprintf(stderr, "Reading file label.\n");
    perror(slave_handle->file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * Check that the label is correct
   */
  
  if (strcmp(file_label, slave_handle->file_label)) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadSlaveTable\n",
	    slave_handle->prog_name, calling_routine);
    fprintf(stderr, "File label does not match requested label.\n");
    fprintf(stderr, "File label is '%s'\n", file_label);
    fprintf(stderr, "Requested label is '%s'\n",
	    slave_handle->file_label);
    return (R_FAILURE);
    
  }
  
  /*
   * allocate space for table params
   */
  
  slave_handle->table_params = (rc_table_params_t *)
    umalloc((ui32) sizeof(rc_table_params_t));
  
  slave_handle->scan_table = (scan_table_t *)
    umalloc((ui32) sizeof(scan_table_t));
  
  tparams = slave_handle->table_params;
  stable = slave_handle->scan_table;
  
  /*
   * read in table params
   */
  
  if (ufread((char *) tparams,
	     (int) sizeof(rc_table_params_t),
	     1, slave_handle->file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadSlaveTable\n",
	    slave_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading table params.\n");
    perror(slave_handle->file_path);
    return (R_FAILURE);
    
  }
  
  memcpy ((void *)  &nbytes_char,
          (void *)  &tparams->nbytes_char,
          (size_t)  sizeof(si32));
  
  BE_to_array_32((ui32 *) &nbytes_char,
	    (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) tparams,
	    (ui32) (sizeof(rc_table_params_t) - nbytes_char));
  
  /*
   * set local variables
   */
  
  nelevations = slave_handle->table_params->nelevations;
  nazimuths = slave_handle->table_params->nazimuths;
  nplanes = slave_handle->table_params->cart.nz;
  // nheights = N_PLANE_HEIGHT_VALUES * nplanes;
  nlist = slave_handle->table_params->nlist;
  
  stable->nelevations = tparams->nelevations;
  stable->nazimuths = tparams->nazimuths;
  stable->ngates = tparams->ngates;
  stable->nbeams_vol = tparams->nbeams_vol;
  stable->delta_azimuth = (double) tparams->delta_azimuth / 1000000.0;
  stable->start_azimuth = (double) tparams->start_azimuth / 1000000.0;
  stable->beam_width = (double) tparams->beam_width / 1000000.0;
  stable->gate_spacing = (double) tparams->gate_spacing / 1000000.0;
  stable->start_range = (double) tparams->start_range / 1000000.0;
  stable->extend_below = tparams->extend_below;
  stable->use_azimuth_table = tparams->use_azimuth_table;
  stable->missing_data_index = tparams->missing_data_index;

  /*
   * read in scan details
   */

  if (read_scan_table(stable,
		      nelevations,
		      slave_handle->file,
		      slave_handle->file_path,
		      slave_handle->prog_name,
		      calling_sequence) != R_SUCCESS) {
    return (R_FAILURE);
  }

  /*
   * read in plane_heights array
   */
  
  if (read_plane_heights(nplanes,
			 &slave_handle->plane_heights,
			 slave_handle->file,
			 slave_handle->file_path,
			 slave_handle->prog_name,
			 calling_sequence) != R_SUCCESS) {
    return (R_FAILURE);
  }
  
  /*
   * allocate slave_table_index array
   */
  
  slave_handle->table_index = (slave_table_index_t **) ucalloc2
    ((ui32) nelevations,
     (ui32) nazimuths,
     (ui32) sizeof(slave_table_index_t));
  table_index = slave_handle->table_index;
  
  /*
   * read in slave_table_index array
   */
  
  for (ielev = 0; ielev < nelevations; ielev++) {
    
    if (ufread((char *) table_index[ielev],
	       (int) sizeof(slave_table_index_t),
	       (int) nazimuths,
	       slave_handle->file) != nazimuths) {
      
      fprintf(stderr, "ERROR - %s:%s:RfReadSlaveTable\n",
	      slave_handle->prog_name, calling_routine);
      fprintf(stderr, "Reading slave_table_index.\n");
      perror(slave_handle->file_path);
      return (R_FAILURE);
      
    }
    
  }
  
  /*
   * allocate list array
   */
  
  slave_handle->list = (char *) umalloc ((ui32) nlist);
  list = slave_handle->list;
  
  /*
   * read in list array
   */
  
  if (ufread(list,
	     (int) sizeof(char) ,
	     (int) nlist,
	     slave_handle->file) != nlist) {
    
    fprintf(stderr, "ERROR - %s:%s:RfReadSlaveTable\n",
	    slave_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading slave table list.\n");
    perror(slave_handle->file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode the shorts and longs in the index, and set pointers relative
   * to memory instead of offsets relative to the start of the list
   */
  
  for (ielev = 0; ielev < tparams->nelevations; ielev++) {
    
    for (iaz = 0; iaz < tparams->nazimuths; iaz++) {
      
      /*
       * the fields in the index
       */
      
      BE_to_array_32((ui32 *) &table_index[ielev][iaz].npoints,
		(ui32) sizeof(si32));
      BE_to_array_32((ui32 *) &table_index[ielev][iaz].u.offset,
		(ui32) sizeof(si32));
      table_index[ielev][iaz].u.index =
        (si32 *)(list + table_index[ielev][iaz].u.offset);
      
    } /* iaz */
    
  } /* ielev */
  
  /*
   * decode the fields in the list
   */
  
  BE_to_array_32((ui32 *) list, (ui32) nlist);
  
  /*
   * close the file
   */
  
  fclose(slave_handle->file);
  free_long_array();
  
  return(R_SUCCESS);
  
}

#undef THIS_ROUTINE
