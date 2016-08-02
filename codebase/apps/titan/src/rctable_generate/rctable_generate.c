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
/*******************************************************************************
 * rctable_generate.c
 *
 * Creates the r_theta_phi to cartesian coord system
 * look-up-table, for taking incoming radar coord data
 * and putting it into a cartesian coordinate array
 *
 * Mike Dixon  RAP NCAR   October 1990
 *
 *******************************************************************************/

#define MAIN
#include "rctable_generate.h"
#undef MAIN

#include <signal.h>

int main(int argc, char **argv)
{

  /*
   * declarations
   */

  si32 *rindex_table;
  si32 n_ext_elev;

  double *cosphi;
  double **beam_ht, **gnd_range;

  scan_table_t scan_table;
  path_parts_t progname_parts;

  /*
   * allocate space for the global structure
   */

  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));

  /*
   * set program name
   */

  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);

  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, exit);
  PORTsignal(SIGTERM, exit);
  PORTsignal(SIGQUIT, exit);
  
  /*
   * read the parameters from the file into the params data base
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);
  
  /*
   * parse the command line arguments
   */

  parse_args(argc, argv);

  /*
   * read the parameters
   */

  read_params();

  /*
   * set up the scan details
   */

  setup_scan(&scan_table);

  /*
   * Override grid params based on mode
   */

  if (Glob->mode == PpiMode || Glob->mode == PolarMode) {
    
    Glob->nz = scan_table.nelevations;
    Glob->minz = 0.0;
    Glob->dz = 1.0;

  }

  if (Glob->mode == PolarMode) {

    if (scan_table.use_azimuth_table) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Cannot use scan table in Polar Mode\n");
      exit(-1);
    }
    
    Glob->ny = scan_table.nazimuths;
    Glob->miny = scan_table.start_azimuth;
    Glob->dy = scan_table.delta_azimuth;

    Glob->nx = scan_table.ngates;
    Glob->minx = scan_table.start_range;
    Glob->dx = scan_table.gate_spacing;

  }

  /*
   * allocate trig of elevation angles
   */

  n_ext_elev = scan_table.nelevations + 2;
  cosphi = (double *) umalloc((ui32) (n_ext_elev * sizeof(double)));

  /*
   * allocate ground range and beam height
   */

  gnd_range = (double **) ucalloc2
    ((ui32) n_ext_elev,
     (ui32) scan_table.ngates,
     sizeof(double));

  beam_ht = (double **) ucalloc2
    ((ui32) n_ext_elev,
     (ui32) scan_table.ngates,
     sizeof(double));

  /*
   * calculate the geometry
   */

  printf("Calculating geometry........\n");

  calc_geom(n_ext_elev, cosphi, beam_ht, gnd_range, &scan_table);

  /*
   * allocate cartesian array with radar coordinate array offsets
   */

  rindex_table = (si32 *)
    umalloc ((ui32) ((Glob->nx * Glob->ny * Glob->nz) * sizeof(si32)));

  /*
   * make the table which looks up the radar coordinate given the cartesian
   * coordinate
   */

  printf("Making initial table..........\n");

  if (Glob->mode == CartMode) {
    make_table_cart(rindex_table, cosphi, beam_ht,
		    gnd_range, &scan_table);
  } else if (Glob->mode == PpiMode) {
    make_table_ppi(rindex_table, cosphi, beam_ht,
		    gnd_range, &scan_table);
  } else if (Glob->mode == PolarMode) {
    make_table_polar(rindex_table);
  }

  /*
   * invert the table, to create a file which enables one to look up the 
   * relevant cartesian coordinates given the radar coordinates
   */

  printf("Inverting table...............\n");

  invert_table_and_write_files(rindex_table, &scan_table);

  return(0);

}

