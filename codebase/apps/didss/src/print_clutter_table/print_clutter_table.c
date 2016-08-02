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
/******************************************************************************
 * print_clutter_table.c :
 *
 * Mike Dixon
 *
 * RAP NCAR Boulder Colorado USA
 *
 * April 1997
 *
 *******************************************************************************/

#define MAIN
#include "print_clutter_table.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char *file_path;
  path_parts_t progname_parts;
  int i, ielev, iaz;
  clut_table_file_index_t clutter_handle;
  clut_table_params_t *tparams;
  clut_table_index_t **table_index;
  clut_table_entry_t *entry;


  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *) umalloc((ui32) sizeof(global_t));
  
  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * display ucopright message
   */

  ucopyright(Glob->prog_name);

  /*
   * parse command line arguments
   */

  parse_args(argc, argv, &file_path);

  /*
   * initialize clut_table file index
   */

  InitClutterIndex(&clutter_handle,
		   sizeof(clut_table_file_index_t),
		   Glob->prog_name, NULL,
		   CLUTTER_TABLE_LABEL, (FILE *) NULL,
		   "clutter_table_view");
  
  /*
   * read in the radar-to-cart table
   */

  if (ReadClutterTable(&clutter_handle, file_path,
		       "clutter_table_view")) {
    return(-1);
  }
  
  tparams = clutter_handle.table_params;

  /*
   * print out info
   */

  printf("\nCLUTTER TABLE FILE INFORMATION\n\n");

  fprintf(stdout, "Dates and times : \n");
  fprintf(stdout, "  File:  %s\n", utimstr(tparams->file_time));
  fprintf(stdout, "  Start: %s\n", utimstr(tparams->start_time));
  fprintf(stdout, "  Mid:   %s\n", utimstr(tparams->mid_time));
  fprintf(stdout, "  End:   %s\n", utimstr(tparams->end_time));
  fprintf(stdout, "\n");

  printf("\nTable parameters : \n\n");

  printf("Dbz scale : %g\n\n", tparams->dbz_scale);
  printf("Dbz bias  : %g\n\n", tparams->dbz_bias);
  printf("Dbz margin : %g\n\n", tparams->dbz_margin);

  printf("nlist - size of clutter list : %d\n\n", tparams->nlist);

  printf("Radar-to-cart table data\n");
  printf("nelevations : %d\n", tparams->lookup_params.nelevations);
  printf("nazimuths : %d\n", tparams->lookup_params.nazimuths);
  printf("ngates : %d\n", tparams->lookup_params.ngates);
  printf("ndata - number of points in cartesian grid : %d\n",
	 tparams->lookup_params.ndata);
  printf("nlist - size of rc table list : %d\n\n",
	 tparams->lookup_params.nlist);
  printf("file index offset in bytes : %d\n\n",
	 tparams->lookup_params.index_offset);
  printf("file list offset in bytes  : %d\n\n",
	 tparams->lookup_params.list_offset);

  printf("gate_spacing (km) : %g\n", tparams->lookup_params.gate_spacing);
  printf("start_range (km) : %g\n", tparams->lookup_params.start_range);
  printf("delta_azimuth (deg) : %g\n", tparams->lookup_params.delta_azimuth);
  printf("start_azimuth (deg) : %g\n", tparams->lookup_params.start_azimuth);
  printf("beam_width (deg) : %g\n\n", tparams->lookup_params.beam_width);

  PrintClutterTableRadarGrid(stdout, "  ", &tparams->lookup_params.grid);
  
  PrintClutterTableElevations(stdout, "  ",
			      "Elevations",
			      tparams->lookup_params.nelevations,
			      clutter_handle.radar_elevations);

  fprintf(stdout, "  Table data:\n");

  table_index = clutter_handle.table_index;
  
  for (ielev = 0; ielev < tparams->lookup_params.nelevations; ielev++) {
    
    for (iaz = 0; iaz < tparams->lookup_params.nazimuths; iaz++) {
      
      /*
       * the fields in the index
       */
      
      fprintf(stdout,
	      "    ielev, iaz, nclut_points: "
	      "%d, %d, %d\n", ielev, iaz,
	      (int) table_index[ielev][iaz].nclut_points);
      
      /*
       * the fields in the list
       */
      
      fprintf(stdout, "      Points as (dbz, ipoint, cart_index): ");
      
      for (i = 0; i < table_index[ielev][iaz].nclut_points; i++) {
	
        entry = table_index[ielev][iaz].u.entry + i;

	fprintf(stdout, "(%d,%d,%d)", (int) entry->dbz,
		(int) entry->ipoint, (int) entry->cart_index);
	
      } /* i */
      
      fprintf(stdout, "\n");

    } /* iaz */
    
  } /* ielev */
  
  FreeClutterTable(&clutter_handle, "clutter_table_view");

  return (0);

}

void tidy_and_exit(int sig)

{

  /*
   * check memory allocation
   */
  
  umalloc_map();
  umalloc_verify();
  
  /*
   * exit with code sig
   */

  exit(sig);

}

