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
 * cart_convert.c
 *
 * converts a file in radar coords to cartesian coords
 * using a look-up-table created by 'rctable_generate'
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * November 1990
 *
 *******************************************************************************/

#define MAIN
#include "cart_convert.h"
#undef MAIN

#include <signal.h>
#include <string.h>

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  path_parts_t progname_parts;

  vol_file_handle_t rv_handle, cv_handle;
  rc_table_file_handle_t rc_handle;

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
   * read the parameters into the parameters data base
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, exit);
  PORTsignal(SIGTERM, exit);
  PORTsignal(SIGQUIT, exit);
  
  /*
   * parse the command line arguments
   */

  parse_args(argc, argv);

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
   * initialize radar coords volume file handle
   */

  RfInitVolFileHandle(&rv_handle,
		      Glob->prog_name,
		      Glob->rdata_path,
		      (FILE *) NULL);

  /*
   * initialize cart coords volume file handle
   */

  RfInitVolFileHandle(&cv_handle,
		      Glob->prog_name,
		      Glob->rdata_path,
		      (FILE *) NULL);

  /*
   * read in radar volume
   */
  
  if (RfReadVolume(&rv_handle, "volume_view") != R_SUCCESS)
    exit(1);

  /*
   * check that the geometry is correct
   */
  
  check_geom(&rv_handle, &rc_handle);

  /*
   * set up the cartesian radar volume
   */

  setup_cart_volume(&rv_handle, &cv_handle, &rc_handle);

  /*
   * transform data and write to file
   */
  
  transform(&rv_handle, &cv_handle, &rc_handle);

  /*
   * write out cartesian file
   */

  if (RfWriteVolume(&cv_handle, "main") != R_SUCCESS)
    exit(1);

  return(0);
  
}
