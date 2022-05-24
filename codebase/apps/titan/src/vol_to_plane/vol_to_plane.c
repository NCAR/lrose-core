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
/**************************************************************************
 * vol_to_plane.c
 *
 * Remaps a dobson volume file onto a plane. The original file is
 * replaced by the plane file. 
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * July 1992
 *
 **************************************************************************/

#define MAIN
#include "vol_to_plane.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  path_parts_t progname_parts;

  long n_files, ifile;

  char **file_paths;

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
   * load up parameters data base
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);

  /*
   * set variables from data base
   */

  read_params();

  /*
   * parse command line arguments
   */

  parse_args(argc, argv, &n_files, &file_paths);

  /*
   * loop through the files
   */

  for (ifile = 0; ifile < n_files; ifile++) {

    fprintf(stderr, "\n%s : processing file %s\n",
	    Glob->prog_name, file_paths[ifile]);
    
    /*
     * remap the file
     */
    
    remap_file(file_paths[ifile]);
  
    umalloc_count();

  } /* ifile */

  /*
   * quit
   */
  
  tidy_and_exit(0);

  return(0);

}

