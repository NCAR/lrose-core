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
 * perform_adjustment()
 *
 * Adjust given file.
 *
 * Returns 0 on success, -1 on failure.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1998
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "mdv_adjust_data.h"

int perform_adjustment(char *file_path)

{

  static int first_call = TRUE;
  static vol_file_handle_t v_handle;

  double orig_bias;
  double final_bias;

  field_params_t *fparams;

  fprintf(stdout, "Adjusting file %s\n", file_path);
  fflush(stdout);

  if (first_call) {
    
    /*
     * initialize index
     */
    
    RfInitVolFileHandle(&v_handle,
			Glob->prog_name,
			(char *) NULL,
			(FILE *) NULL);

    first_call = FALSE;

  }
  
  /*
   * read in the file
   */

  v_handle.vol_file_path = file_path;
  if (RfReadVolume(&v_handle, "perform_adjustment") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:perform_adjustment\n", Glob->prog_name);
    fprintf(stderr, "Cannot read in file %s.\n", file_path);
    return(-1);
  }

  /*
   * compute new bias
   */

  fparams = v_handle.field_params[Glob->params.field_num];
  orig_bias = (double) fparams->bias / (double) fparams->factor;
  final_bias = orig_bias + Glob->params.data_adjustment;
  
  /*
   * amend the field params
   */

  fparams->bias = (int) floor(final_bias * fparams->factor + 0.5);
  
  /*
   * write the file
   */
  
  if (RfWriteVolume(&v_handle, "perform_adjustment") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:perform_adjustment\n", Glob->prog_name);
    fprintf(stderr, "Cannot write out file %s.\n", file_path);
    return(-1);
  }

  return (0);

}

