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
/***************************************************************************
 * clutter_compute.c :
 *
 * Computes stats on ground clutter from all of the files in a designated
 * directory and writes the stats to a given file.
 *
 * The clutter file data is the median value at each grid point.
 *
 * Mike Dixon  RAP NCAR November 1990
 *
 ****************************************************************************/

#define MAIN
#include "clutter_compute.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char **clutter_file_names;              /* array of clutter file names */

  si32 nfiles;

  vol_file_handle_t vol_index;                /* radar vol file handle */
  vol_file_handle_t clut_vol_index;           /* clutter vol file handle */

  path_parts_t progname_parts;            /* executble path parts */

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *) umalloc((ui32) sizeof(global_t));

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
   * get list of clutter file names
   */

  get_clutter_files(&nfiles, &clutter_file_names);

  /*
   * set up the volume structs and arrays
   */
     
  initialize(nfiles, clutter_file_names,
	     &vol_index, &clut_vol_index);
		       
  /*
   * compute the median, write to file
   */

  compute_median(nfiles, clutter_file_names,
		 &vol_index, &clut_vol_index);

  /*
   * write the clutter file
   */

  if (RfWriteVolume(&clut_vol_index, "main") != R_SUCCESS)
    return(1);

  fprintf(stdout, "Writing clutter file '%s'\n",
	  clut_vol_index.vol_file_path);
  
  return (0);

}
