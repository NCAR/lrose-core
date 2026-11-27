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
 * Dsr2MdvLookup.c
 *
 * Creates the r_theta_phi to cartesian coord system
 * look-up-table, for taking incoming radar coord data
 * and putting it into a cartesian coordinate array
 *
 * Mike Dixon  RAP NCAR   October 1990
 *
 *******************************************************************************/

#define MAIN
#include "Dsr2MdvLookup.h"
#undef MAIN

#include <signal.h>

int main(int argc, char **argv)
{

  /*
   * declarations
   */

  char *params_file_path = NULL;

  int check_params;
  int print_params;

  si32 *rindex_table;
  si32 n_ext_elev;

  double *cosphi;
  double **beam_ht, **gnd_range;

  radar_scan_table_t scan_table;
  path_parts_t progname_parts;
  tdrp_override_t override;

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
   * display ucopright message
   */

  ucopyright(Glob->prog_name);

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
  
  /*
   * parse command line arguments
   */

  parse_args(argc, argv,
             &check_params, &print_params,
             &override,
             &params_file_path);

  /*
   * load up parameters
   */
  
  Glob->table = Dsr2MdvLookup_tdrp_init(&Glob->params);
  
  if (FALSE == TDRP_read(params_file_path,
                         Glob->table,
                         &Glob->params,
                         override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
            params_file_path);
    tidy_and_exit(-1);
  }
  TDRP_free_override(&override);

  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    tidy_and_exit(0);
  }

  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    tidy_and_exit(0);
  }
  
  /*
   * set up the scan details
   */

  setup_scan(&scan_table);

  /*
   * Set up the derived parameters
   */

  set_derived_params(&scan_table);

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

  if (Glob->geom == P2MDV_POLAR) {

    make_table_polar(rindex_table);

  } else if (Glob->geom == P2MDV_PPI) {

    make_table_ppi(rindex_table, cosphi, beam_ht,

		   gnd_range, &scan_table);
  } else {

    make_table_cart(rindex_table, cosphi, beam_ht,
		    gnd_range, &scan_table);

  }

  if (Glob->geom == P2MDV_POLAR) {

    write_polar(rindex_table, &scan_table);
    
  } else { /* P2MDV_CART or P2MDV_PPI geom */
    
    /*
     * invert the table, to create a file which enables one to look up the 
     * relevant cartesian coordinates given the radar coordinates
     */
    
    printf("Inverting table...............\n");
    
    write_cart_or_ppi(rindex_table, &scan_table);

  }

  return(0);

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

