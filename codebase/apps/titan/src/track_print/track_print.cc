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
/*************************************************************************
 * track_print.c :
 *
 * prints track data
 *
 * Mike Dixon
 *
 * RAP NCAR Boulder Colorado USA
 *
 * April 1996
 *
 *************************************************************************/

#define MAIN
#include "track_print.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char **file_paths;
  si32 n_files = 0;
  si32 ifile;
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
   * parse command line arguments
   */

  parse_args(argc, argv, &n_files, &file_paths);

  /*
   * process files
   */

  for (ifile = 0; ifile < n_files; ifile++) {
    process_file(file_paths[ifile]);
  }

  umalloc_verify();
  umalloc_map();

  return(0);

}
