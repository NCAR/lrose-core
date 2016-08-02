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
 * process_ppis()
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "ppi2comp.h"

void process_ppis(char *input_file_path)

{
  
  static int first_call = TRUE;
  static ppi_t *ppi;

  int i;
  int all_valid;
  int nppis = Glob->params.input.len;
  ppi2comp_input *input = Glob->params.input.val;

  PMU_auto_register("Processing ppi");

  /*
   * init on first call
   */

  if (first_call) {

    ppi = (ppi_t *) umalloc(nppis * sizeof(ppi_t));

    for (i = 0; i < nppis; i++) {
      RfInitVolFileHandle(&ppi[i].v_handle,
			  Glob->prog_name,
			  NULL,
			  (FILE *) NULL);

    } /* i */

    first_call = FALSE;

  }

  /*
   * read in trigger ppi file
   */

  strcpy(ppi[0].file_path, input_file_path);
  ppi[0].v_handle.vol_file_path = ppi[0].file_path;
  if (RfReadVolume(&ppi[0].v_handle, "process_ppis") != R_SUCCESS) {
    fprintf(stderr, "Cannot read in trigger ppi volume\n");
    return;
  }
  ppi[0].valid = TRUE;

  /*
   * get other ppi files
   */

  all_valid = TRUE;
  for (i = 1; i < nppis; i++) {
    find_ppi_file(&ppi[0], &ppi[i], &input[i]);
    if (!ppi[i].valid) {
      all_valid = FALSE;
    }
  }
  
  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "Trigger PPI file: %s\n", ppi[0].file_path);
    for (i = 1; i < nppis; i++) {
      if (ppi[i].valid) {
	fprintf(stderr, "PPI %d, file: %s\n", i, ppi[i].file_path);
      } else {
	fprintf(stderr, "PPI %d NOT VALID, input dir %s\n",
		i, input[i].dir);
      }
    }
    if (!all_valid) {
      fprintf(stderr, "***** NOT PROCESSING THIS PPI SET *****\n");
    }

  }

  if (all_valid) {
    create_comp(nppis, input, ppi);
  }

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "------------\n");
  }

  return;

}

