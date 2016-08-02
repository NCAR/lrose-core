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
 * write_output_file.c
 *
 * writes the vil data to a file
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "mdv2vil.h"
#include <titan/file_io.h>

void write_output_file(vol_file_handle_t *v_handle)

{
  
  char note[80];

  PMU_auto_register("In write_output_file");

  /*
   * append the note
   */

  sprintf(note, "\nCreated VIL from reflectivity field");

  /*
   * adjust cart params to indicate there is only 1 level
   * and 1 field
   */

  v_handle->vol_params->cart.nz = 1;
  v_handle->vol_params->nfields = 1;

  /*
   * write file
   */
  
  if (RfWriteDobson(v_handle, TRUE,
		    (Glob->params.debug >= DEBUG_NORM),
		    Glob->params.output_dir,
		    Glob->params.output_file_ext,
		    Glob->prog_name,
		    "write_output_file") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:write_output_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot write output file to dir %s\n",
	    Glob->params.output_dir);
  }

}
