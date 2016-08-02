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
 * mdv2ascii.c :
 *
 * Prints out a mdv grid in ascii format
 *
 * Mike Dixon
 *
 * RAP NCAR Boulder Colorado USA
 *
 * August 1994
 *
 *******************************************************************************/

#define MAIN
#include "mdv2ascii.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  si32 col_count;
  si32 nfields, nplanes;
  double val;
  vol_file_handle_t v_handle;
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
   * parse command line arguments
   */

  parse_args(argc, argv);

  /*
   * initialize volume file index
   */

  RfInitVolFileHandle(&v_handle,
		      Glob->prog_name,
		      Glob->file_name,
		      (FILE *) NULL);

  /*
   * read in radar volume
   */
  
  if (RfReadVolume(&v_handle, "main") != R_SUCCESS)
    exit(-1);

  nfields = v_handle.vol_params->nfields;
  nplanes = v_handle.vol_params->cart.nz;

  /*
   * check the requested plane num is OK
   */

  if (Glob->plane_num > nplanes - 1) {
    fprintf(stderr, "Requested plane num %d too high\n",
	    Glob->plane_num);
    fprintf(stderr, "Only %d planes in file '%s'\n",
	    nplanes, Glob->file_name);
    exit (-1);
  }

  /*
   * check the requested field num is OK
   */

  if (Glob->field_num > nfields - 1) {
    fprintf(stderr, "Requested field num %d too high\n",
	    Glob->field_num);
    fprintf(stderr, "Only %d fields in file '%s'\n",
	    nfields, Glob->file_name);
    exit (-1);
  }

  /*
   * init the retrieval of data from plane
   */

  if (Glob->vol) {
    init_retrieval_vol(&v_handle,
		       Glob->field_num);
  } else if(Glob->composite) {
    init_retrieval_comp(&v_handle,
			Glob->field_num);
  } else {
    init_retrieval(&v_handle,
		   Glob->plane_num,
		   Glob->field_num);
  }

  /*
   * retrieve points and print out
   */

  col_count = 0;
  
  while (retrieve_next(&val)) {

    fprintf(stdout, Glob->format, val);
    col_count++;

    if (col_count == Glob->ncolumns) {
      fprintf(stdout, "\n");
      col_count = 0;
    }

  }

  if (col_count != 0) {
    fprintf(stdout, "\n");
  }

  if (RfFreeVolArrays(&v_handle, "main")) {
    exit (-1);
  }

  return (0);

}
