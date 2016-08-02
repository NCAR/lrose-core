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
 * process_file()
 *
 * process the kav file
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "mdv2plain.h"

#include <sys/stat.h>

#include <toolsa/globals.h>
#include <toolsa/ldata_info.h>


#define TIME_STR_LEN 12
#define MON_STR_LEN 3

int process_file(vol_file_handle_t *v_handle, char *input_file_path,
		 LDATA_handle_t *output_ldata)

{

  si32 nfields, nplanes;

  fprintf(stdout, "Processing file %s\n", input_file_path);

  /*
   * read in radar volume
   */

  v_handle->vol_file_path = input_file_path;
  
  if (RfReadVolume(v_handle, "process_file") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot read in file %s\n", input_file_path);
    return(-1);
  }
  
  nfields = v_handle->vol_params->nfields;
  nplanes = v_handle->vol_params->cart.nz;

  /*
   * check the requested plane nums are OK
   */

  if (Glob->params.start_plane > nplanes - 1) {
    fprintf(stderr, "Requested plane num %ld too high\n",
	    Glob->params.start_plane);
    fprintf(stderr, "Only %d planes in file '%s'\n",
	    nplanes, input_file_path);
    return (-1);
  }

  if (Glob->params.end_plane > nplanes - 1) {
    fprintf(stderr, "Requested plane num %ld too high\n",
	    Glob->params.end_plane);
    fprintf(stderr, "Only %d planes in file '%s'\n",
	    nplanes, input_file_path);
    return (-1);
  }

  /*
   * check the requested field num is OK
   */

  if (Glob->params.field_num > nfields - 1) {
    fprintf(stderr, "Requested field num %ld too high\n",
	    Glob->params.field_num);
    fprintf(stderr, "Only %d fields in file '%s'\n",
	    nfields, input_file_path);
    return (-1);
  }

  /*
   * write the output
   */

  return (write_output(v_handle, output_ldata));

}
