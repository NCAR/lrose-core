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
 * tdrp_test.c
 *
 * Test of use of tdrp library in C
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000, USA
 *
 * Sept 1998
 *
 **********************************************************************/

#include "tdrp_test.h"
#include <stdlib.h>

int main(int argc, char **argv)

{
  
  /*
   * basic declarations
   */

  char *prog_name;
  char *params_file_path = NULL;
  tdrp_override_t override;
  _tdrp_struct params;   /* parameter struct */
  FILE *out;

  /*
   * set program name
   */
  
  prog_name = strrchr(argv[0], '/');
  if (prog_name == NULL) {
    prog_name = argv[0];
  }

  /* Add the next line to test the warning suppresion
   * TDRP_warn_if_extra_params(FALSE);
   */

  /*
   * initialize the override list
   */

  TDRP_init_override(&override);

  /*
   * parse command line arguments
   */
  
  parse_args(argc, argv, prog_name, &override);
  
  /*
   * load up parameters
   */
  
  if (_tdrp_load_from_args(argc, argv, &params,
			   override.list, &params_file_path)) {
    fprintf(stderr, "ERROR - %s:main\n", prog_name);
    fprintf(stderr, "Problems with params file '%s'\n",
	    params_file_path);
    exit(-1);
  }

  /*
   * free up override list
   */
  
  TDRP_free_override(&override);
  
  /*
   * print out the parameters from user space
   */
  
  do_printout(&params, stdout);

  /*
   * print table to params.in
   */

  if ((out = fopen("params.in", "w")) == NULL) {
    fprintf(stderr, "ERROR - %s\n", prog_name);
    fprintf(stderr,
	    "Cannot open file params.in for writing table.\n");
    return (-1);
  }
  _tdrp_print(out, PRINT_SHORT);
  fclose(out);

  /*
   * change a few params from within the program
   */

  params.your_age = 22;
  params._our_ages[3] = 27;
  params.__item_count[2][4] = 77;
  params._storm_volume[0] = 99.99;
  params.__rain_accumulation[3][1] = 3.3;
  params.mass_coefficient = 8.8e-8;
  params.debug = TRUE;
  params.__compute_length[2][2] = TRUE;
  TDRP_str_replace(&params.input_file_ext, "dob");
  TDRP_str_replace(&params._input_file_paths[1],
		   "New_file_path");
  params.grid.dy = 3.9;
  params._surface_stations[1].lat = 39.9998;
  params._surface_stations[2].gauge_make = ETI;
  TDRP_str_replace(&params._data_field[1].name, "Spectral width");

  /*
   * realloc some arrays
   */
  
  _tdrp_array_realloc("our_ages", 7);
  params._our_ages[5] = 55;
  params._our_ages[6] = 70;
  
  _tdrp_array2D_realloc("rain_accumulation", 6, 8);
  _tdrp_array2D_realloc("output_file_paths", 4, 3);
  _tdrp_array_realloc("data_field", 3);

  params._data_field[2].scale = 1.1;
  params._data_field[2].bias = -25.0;
  TDRP_str_replace(&params._data_field[2].name, "Signal-to-noise");
  TDRP_str_replace(&params._data_field[2].units, "dB");
  
  /*
   * copy params back to table
   */
  
  _tdrp_sync();

  /*
   * print out table to params.out
   */

  if ((out = fopen("params.out", "w")) == NULL) {
    fprintf(stderr, "ERROR - %s\n", prog_name);
    fprintf(stderr,
	    "Cannot open file params.out for writing table.\n");
    return (-1);
  }
  _tdrp_print(out, PRINT_SHORT);
  fclose(out);

  /*
   * Free up
   */

  _tdrp_free_all();

  return(0);

}

