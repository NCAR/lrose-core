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
/**********************************************************************
 * rapformats/clutter_table.h
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * April 1997
 *
 **********************************************************************/

#ifndef clutter_table_h
#define clutter_table_h

#ifdef __cplusplus
extern "C" {
#endif

#include <rapformats/polar2mdv_lookup.h>

#define CLUTTER_TABLE_LABEL "Clutter table file"

/*
 * clutter table params
 */

typedef struct {

  si32 nbytes_char;		/* number of bytes of character data
				 * at the end of this struct */

  si32 file_time;

  si32 start_time;              /* start, mid and end times for data */
  si32 mid_time;
  si32 end_time;
  
  fl32 dbz_scale;		/* scale for dbz values */
  fl32 dbz_bias;		/* bias for dbz values */
  fl32 dbz_margin;		/* margin between clutter dbz data
				 * and the acceptable value from
				 * a data file */

  si32 nlist;			/* length of list */

  si32 index_offset;		/* byte offset of table_index array */
  si32 list_offset;		/* byte offset of list array */

  P2mdv_lookup_params_t lookup_params;  /* copy of the lookup params struct
					 * used to create the clutter table */

} clut_table_params_t;

/*
 * the clut_table entries hold the dbz value for the point,
 * and the cartesian point number for the beam
 */

typedef struct {

  ui16 dbz;
  ui16 ipoint;
  ui32 cart_index;

} clut_table_entry_t;

/*
 * clut_table_index array holds an index of the clut_table
 * entries in the clutter table array
 */

typedef struct {

  ui32 nclut_points;

  union {
    ui32 offset;
    clut_table_entry_t *entry;
  } u;

} clut_table_index_t;

/*
 * clut_table_file_index_t is a convenience structure which may be used
 * for referring to any or all component(s) of a clutter table file
 */

typedef struct {

  char *prog_name;
  char *file_path;
  char *file_label;
  FILE *file;
  int index_initialized;

  clut_table_params_t *table_params;
  clut_table_index_t **table_index;
  char *list;

  fl32 *radar_elevations;  /* deg */

  /*
   * the following are used to keep track of allocated memory
   */

  int params_allocated;
  int elev_allocated;
  int index_allocated;
  int list_allocated;

  si32 nelevations_allocated;
  si32 nazimuths_allocated;
  si32 nlist_allocated;

} clut_table_file_index_t;

/*
 * prototypes
 */

extern int FreeClutterIndex(clut_table_file_index_t *index,
			    char *calling_routine);

extern int InitClutterIndex(clut_table_file_index_t *index,
			    int size,
			    char *prog_name,
			    char *file_path,
			    char *file_label,
			    FILE *file,
			    char *calling_routine);

extern int FreeClutterTable(clut_table_file_index_t *clutter_index,
			    char *calling_routine);

extern int ReadClutterTable(clut_table_file_index_t *clutter_index,
			    char *file_path,
			    char *calling_routine);

extern void PrintClutterTableRadarGrid(FILE *out,
				       char *spacer,
				       _MDV_radar_grid_t *grid);
     
extern void PrintClutterTableElevations(FILE *out,
					char *spacer,
					char *label,
					si32 nelevations,
					fl32 *radar_elevations);
     
#ifdef __cplusplus
}
#endif

#endif
