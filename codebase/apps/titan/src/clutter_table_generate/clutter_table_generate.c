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
 * clutter_table_generate.c
 *
 * generates a clutter table given a clutter data file and a radar-to-cartesian
 * lookup table file
 *
 * Mike Dixon  RAP, NCAR, Boulder, CO, USA
 *
 * October 1990
 *
 *******************************************************************************/

#define MAIN
#include "clutter_table_generate.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  vol_file_handle_t clut_vol_index;
  rc_table_file_handle_t rc_handle;

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
  Glob->prog_name = (char *) umalloc
    ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);

  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * load the paramters into the data base
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);

  /*
   * read the parameters from the data base into the global struct
   */

  read_params();

  /*
   * parse command line arguments
   */

  parse_args(argc, argv);

  /*
   * initialize the clutter vol file handle
   */

  RfInitVolFileHandle(&clut_vol_index,
		      Glob->prog_name,
		      Glob->clutter_file_path,
		      (FILE *) NULL);
  
  /*
   * read in the clutter data
   */

  if (RfReadVolume(&clut_vol_index, "main") != R_SUCCESS)
    return(1);

  /*
   * initialize rc_table file handle
   */

  RfInitRcTableHandle(&rc_handle,
		      Glob->prog_name,
		      Glob->rc_table_path,
		      (FILE *) NULL);

  /*
   * read in the radar-to-cart table
   */

  if (RfReadRcTable(&rc_handle,
		    "main") != R_SUCCESS)
    exit(1);

  /*
   * check the geometry
   */

  check_geom(&clut_vol_index, &rc_handle);

  /*
   * write the clutter table
   */

  write_table(&clut_vol_index, &rc_handle);

  return (0);

}

