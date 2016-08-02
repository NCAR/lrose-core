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
 * area_compute.c
 *
 * Example of use of tdrp library in C
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000, USA
 *
 * Sept 1998
 *
 **********************************************************************/

#include "area_compute.h"

int main(int argc, char **argv)

{
  
  /*
   * basic declarations
   */

  char *prog_name;
  char *params_file_path = NULL;
  tdrp_override_t override;
  _tdrp_struct params;   /* parameter struct */
  double area;

  /*
   * set program name
   */
  
  prog_name = strrchr(argv[0], '/');
  if (prog_name == NULL) {
    prog_name = argv[0];
  }

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
    if (params_file_path) {
      fprintf(stderr, "Problems with params file '%s'\n",
	      params_file_path);
    }
    exit(-1);
  }

  /*
   * free up override list
   */
  
  TDRP_free_override(&override);

  /*
   * compute area
   */

  switch (params.shape) {

  case SQUARE:
    area = params.size * params.size;
    break;

  case CIRCLE:
    area = params.size * params.size * (3.14159 / 4.0);
    break;

  case EQ_TRIANGLE:
    area = params.size * params.size * (0.866 / 2.0);
    break;

  } /* switch */

  /*
   * debug message
   */

  if (params.debug) {
    fprintf(stderr, "Size is: %g\n", params.size);
    switch (params.shape) {
    case SQUARE:
      fprintf(stderr, "Shape is SQUARE\n");
      break;
    case CIRCLE:
      fprintf(stderr, "Shape is CIRCLE\n");
      break;
    case EQ_TRIANGLE:
      fprintf(stderr, "Shape is EQ_TRIANGLE\n");
      break;
    } /* switch */
    fprintf(stderr, "Area is: %g\n", area);
  }
  
  /*
   * write out the result
   */
  
  write_result(&params, area);

  /*
   * Free up
   */

  _tdrp_free_all();

  return(0);

}

