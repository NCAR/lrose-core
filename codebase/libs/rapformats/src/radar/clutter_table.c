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
 * clutter_table.c
 *
 * Clutter table file access routines
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
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <rapformats/clutter_table.h>

#define MAX_SEQ 256
#define _FILE_LABEL_LEN 40

/*************************************************************************
 *
 * FreeClutterIndex()
 *
 * frees the memory associated with the file index
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int FreeClutterIndex(clut_table_file_index_t *index,
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
 * InitClutterIndex()
 *
 * initializes the memory associated with a generic dual file index
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int InitClutterIndex(clut_table_file_index_t *index,
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
 * FreeClutterTable()
 *
 * Frees up the clutter table memory
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

/*ARGSUSED*/

int FreeClutterTable(clut_table_file_index_t *clutter_index,
		     char *calling_routine)
     
{
  
  ufree(clutter_index->table_params);
  ufree(clutter_index->radar_elevations);
  ufree(clutter_index->table_index);
  ufree(clutter_index->list);
  return(0);
  
}

/*************************************************************************
 *
 * ReadClutterTable()
 *
 * Reads in a clutter table
 *
 * Memory allocation is taken care of in this routine. To free up
 * this memory, use FreeClutterTable()
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int ReadClutterTable(clut_table_file_index_t *clutter_index,
		     char *file_path,
		     char *calling_routine)
     
{
  
  char file_label[_FILE_LABEL_LEN];
  char *list;
  
  si32 nbytes_char;
  
  si32 i, ielev, iaz;
  si32 nelevations;
  si32 nazimuths;
  /* si32 nplanes; */
  si32 nlist;
  
  clut_table_params_t *tparams;
  clut_table_index_t **table_index;
  clut_table_entry_t *entry;
  
  /*
   * open file
   */

  if (clutter_index->file_path != NULL) {
    STRfree(clutter_index->file_path);
  }
  clutter_index->file_path = STRdup(file_path);
  if ((clutter_index->file =
       ta_fopen_uncompress(clutter_index->file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:%s:ReadClutterTable\n",
	    clutter_index->prog_name, calling_routine);
    fprintf(stderr, "Cannot open lookup table file.\n");
    perror(clutter_index->file_path);
    return (-1);
  }
  
  /*
   * read in file label
   */
  
  if (ta_fread(file_label,
	       (int) sizeof(char),
	       (int) _FILE_LABEL_LEN,
	       clutter_index->file) != _FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:ReadClutterTable\n",
	    clutter_index->prog_name, calling_routine);
    fprintf(stderr, "ERROR - ReadClutterTable\n");
    fprintf(stderr, "Reading file label.\n");
    perror(clutter_index->file_path);
    return (-1);
    
  }
  
  /*
   * Check that the label is correct
   */
  
  if (strcmp(file_label, clutter_index->file_label)) {
    
    fprintf(stderr, "ERROR - %s:%s:ReadClutterTable\n",
	    clutter_index->prog_name, calling_routine);
    fprintf(stderr, "File label does not match requested label.\n");
    fprintf(stderr, "File label is '%s'\n", file_label);
    fprintf(stderr, "Requested label is '%s'\n",
	    clutter_index->file_label);
    return (-1);
    
  }
  
  /*
   * allocate space for table params
   */
  
  clutter_index->table_params = (clut_table_params_t *)
    umalloc((ui32) sizeof(clut_table_params_t));
    
  tparams = clutter_index->table_params;
  
  /*
   * read in table params
   */
  
  if (ta_fread((char *) tparams,
	       (int) sizeof(clut_table_params_t),
	       1, clutter_index->file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:ReadClutterTable\n",
	    clutter_index->prog_name, calling_routine);
    fprintf(stderr, "Reading table params.\n");
    perror(clutter_index->file_path);
    return (-1);
    
  }
  
  nbytes_char = BE_to_si32(tparams->nbytes_char);

  BE_to_array_32((ui32 *) tparams,
		 (ui32) (sizeof(clut_table_params_t) - nbytes_char));
  
  /*
   * set local variables
   */
  
  nelevations = clutter_index->table_params->lookup_params.nelevations;
  nazimuths = clutter_index->table_params->lookup_params.nazimuths;
  /* nplanes = clutter_index->table_params->lookup_params.grid.nz; */
  nlist = clutter_index->table_params->nlist;
  
  /*
   * allocate index radar elevations array
   */
  
  clutter_index->radar_elevations = (fl32 *) umalloc
    (nelevations * sizeof(fl32));
    
  /*
   * read in local radar_elevations array as long, decode from network
   * byte order
   */
  
  if (ta_fread((char *) clutter_index->radar_elevations,
	       sizeof(fl32), nelevations,
	       clutter_index->file) != nelevations) {
    
    fprintf(stderr, "ERROR - %s:%s:ReadClutterTable\n",
	    clutter_index->prog_name, calling_routine);
    fprintf(stderr, "Reading radar elevation array.\n");
    perror(clutter_index->file_path);
    return (-1);
    
  }
  
  BE_to_array_32(clutter_index->radar_elevations,
		 nelevations * sizeof(fl32));
  
  /*
   * allocate clut_table_index array
   */
  
  clutter_index->table_index = (clut_table_index_t **) ucalloc2
    ((ui32) nelevations,
     (ui32) nazimuths,
     (ui32) sizeof(clut_table_index_t));
  
  table_index = clutter_index->table_index;
  
  /*
   * read in clut_table_index array
   */
  
  for (ielev = 0; ielev < nelevations; ielev++) {
    
    if (ta_fread((char *) table_index[ielev],
		 (int) sizeof(clut_table_index_t),
		 (int) nazimuths,
		 clutter_index->file) != nazimuths) {
      
      fprintf(stderr, "ERROR - %s:%s:ReadClutterTable\n",
	      clutter_index->prog_name, calling_routine);
      fprintf(stderr, "Reading clut_table_index.\n");
      perror(clutter_index->file_path);
      return (-1);
      
    }
    
  }
  
  /*
   * allocate list array
   */
  
  clutter_index->list = (char *) umalloc ((ui32) nlist);
  list = clutter_index->list;
  
  /*
   * read in list array
   */
  
  if (ta_fread(list,
	       (int) sizeof(char) ,
	       (int) nlist,
	       clutter_index->file) != nlist) {
    
    fprintf(stderr, "ERROR - %s:%s:ReadClutterTable\n",
	    clutter_index->prog_name, calling_routine);
    fprintf(stderr, "Reading clutter table list.\n");
    perror(clutter_index->file_path);
    return (-1);
    
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
        (clut_table_entry_t *)(list + table_index[ielev][iaz].u.offset);
      
      /*
       * the fields in the list
       */
      
      for (i = 0; i < table_index[ielev][iaz].nclut_points; i++) {
	
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
  
  fclose(clutter_index->file);
  
  return(0);
  
}


#define BOOL_STR(a) (a == 0 ? "false" : "true")

void PrintClutterTableRadarGrid(FILE *out,
				char *spacer,
				_MDV_radar_grid_t *grid)
     
{

  fprintf(out, "%sMDV radar grid parameters : \n", spacer);

  fprintf(out, "%s  nbytes_char : %ld\n",
	  spacer, (long) grid->nbytes_char);

  fprintf(out, "%s  grid latitude : %g\n",
	  spacer, grid->latitude);
  fprintf(out, "%s  grid longitude : %g\n",
	  spacer, grid->longitude);
  fprintf(out, "%s  grid rotation : %g\n",
	  spacer, grid->rotation);

  fprintf(out, "%s  nx, ny, nz : %d, %d, %d\n",
	  spacer,
	  grid->nx, grid->ny, grid->nz);

  fprintf(out, "%s  minx, miny, minz : %g, %g, %g\n",
	  spacer,
	  grid->minx, grid->miny, grid->minz);
  
  fprintf(out, "%s  dx, dy, dz : %g, %g, %g\n", spacer,
	  grid->dx, grid->dy, grid->dz);
  
  fprintf(out, "%s  radarx, radary, radarz : %g, %g, %g\n",
	  spacer,
	  grid->radarx, grid->radary, grid->radarz);
  
  fprintf(out, "%s  dz_constant: %s\n", spacer,
	  BOOL_STR(grid->dz_constant));

  fprintf(out, "%s  x units : %s\n", spacer, grid->unitsx);
  fprintf(out, "%s  y units : %s\n", spacer, grid->unitsy);
  fprintf(out, "%s  z units : %s\n", spacer, grid->unitsz);


}

/*------------------------------------
 */

void PrintClutterTableElevations(FILE *out,
				 char *spacer,
				 char *label,
				 si32 nelevations,
				 fl32 *radar_elevations)
     
{
  
  si32 ielev;
  
  fprintf(out, "\n%s%s : \n\n", spacer, label);

  for (ielev = 0; ielev < nelevations; ielev++) {
    fprintf(out, "%s  Elev. number %ld = %g deg.\n",
	    spacer, (long) ielev,
	    (double) radar_elevations[ielev]);
  }
  
  fprintf(out, "\n");

}

