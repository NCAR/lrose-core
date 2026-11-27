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
 * print_polar2mdv_lookup.c :
 *
 * Mike Dixon
 *
 * RAP NCAR Boulder Colorado USA
 *
 * April 1997
 *
 *******************************************************************************/

#define MAIN
#include "print_polar2mdv_lookup.h"
#undef MAIN

#define BOOL_STR(a) (((a) == 0)? "false" : "true")

static void _PrintRadarGrid(FILE *out,
			    char *spacer,
			    _MDV_radar_grid_t *grid);
     
int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char *file_path;
  path_parts_t progname_parts;
  long iaz, ielev, ipoint;
  P2mdv_lookup_file_index_t lookup_index;
  P2mdv_lookup_index_t *lindex;
  P2mdv_lookup_entry_t *lentry;
  P2mdv_lookup_params_t *lparams;

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
   * initialize P2mdv_lookup file index
   */

  InitP2mdvIndex(&lookup_index,
		 sizeof(P2mdv_lookup_file_index_t),
		 Glob->prog_name,
		 NULL,
		 POLAR2MDV_LOOKUP_LABEL,
		 (FILE *) NULL,
		 "print_polar2mdv_lookup");

  /*
   * read in the radar-to-cart table
   */

  if (ReadP2mdvLookup(&lookup_index, file_path, "P2mdv_lookup_view")) {
    return(-1);
  }
  lparams = lookup_index.lookup_params;

  /*
   * print out info
   */

  printf("\nPOLAR2MDV LOOKUP TABLE FILE INFORMATION\n\n");
  
  switch (lparams->geom) {
  case P2MDV_CART:
    printf("table geometry: CART\n");
    break;
  case P2MDV_PPI:
    printf("table geometry: PPI\n");
    break;
  case P2MDV_POLAR:
    printf("table geometry: POLAR\n");
    break;
  }
  
  printf("use_azimuth_table : %s\n",
	 BOOL_STR(lparams->use_azimuth_table));
  printf("extend_below : %s\n",
	 BOOL_STR(lparams->extend_below));

  printf("nelevations : %d\n", lparams->nelevations);
  printf("nazimuths : %d\n", lparams->nazimuths);
  printf("ngates : %d\n", lparams->ngates);
  printf("nbeams_vol : %d\n", lparams->nbeams_vol);

  printf("delta_azimuth (deg) : %g\n", lparams->delta_azimuth);
  printf("start_azimuth (deg) : %g\n", lparams->start_azimuth);
  printf("beam_width (deg) : %g\n\n", lparams->beam_width);

  printf("start_range (km) : %g\n", lparams->start_range);
  printf("gate_spacing (km) : %g\n", lparams->gate_spacing);

  printf("radar latitude : %g\n", lparams->radar_latitude);
  printf("radar longitude : %g\n", lparams->radar_longitude);

  printf("ndata - number of points in grid : %d\n",
	 lparams->ndata);
  printf("nlist - size of table list : %d\n\n", lparams->nlist);

  printf("file index offset in bytes : %d\n\n", lparams->index_offset);
  printf("file list offset in bytes  : %d\n\n", lparams->list_offset);

  _PrintRadarGrid(stdout, "  ", &lparams->grid);
  
  RadarPrintScanTable(stdout, "    ", lookup_index.scan_table);
  
  if (!Glob->summary_printout) {

    for (ielev = 0; ielev < lparams->nelevations; ielev++) {

      for (iaz = 0; iaz < lparams->nazimuths; iaz++) {

	lindex = lookup_index.lookup_index[ielev] + iaz;

	if (lparams->geom == P2MDV_POLAR) {
	  printf("  ielev, iaz, npoints, last_gate_active, offset: "
		 "%ld, %ld, %d, %d, %d\n",
		 ielev, iaz, lindex->npoints,
		 lindex->last_gate_active, lindex->u.offset);
	} else {
	  printf("  ielev, iaz, npoints, last_gate_active: "
		 "%ld, %ld, %d, %d\n",
		 ielev, iaz, lindex->npoints,
		 lindex->last_gate_active);
	}

	if (Glob->full_printout && lparams->geom != P2MDV_POLAR) {
	  for (ipoint = 0; ipoint < lindex->npoints; ipoint++) {
	    lentry = lindex->u.entry + ipoint;
	    printf("    gate, index: %d, %d\n",
		   lentry->gate, lentry->index);
	  } /* ipoint */
	} /* if (full_printout)*/

      } /* iaz */

    } /* ielev */

  } /* if (!summary_printout) */

  FreeP2mdvLookup(&lookup_index, "print_polar2mdv_lookup");

  return (0);

}

static void _PrintRadarGrid(FILE *out,
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

