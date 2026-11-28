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
/***********************************************************************
 * smooth_mdv.c
 *
 * Smooth the data in a mdv file, using either a mean or median
 * filter.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Oct 1993
 *
 ************************************************************************/

#define MAIN
#include "smooth_mdv.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char *params_file_path = NULL;
  int check_params;
  int print_params;
  path_parts_t progname_parts;
  vol_file_handle_t v_handle;
  tdrp_override_t override;
  char notebuf[VOL_PARAMS_NOTE_LEN];

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

  parse_args(argc, argv, &check_params, &print_params,
	     &params_file_path, &override);

   /*
   * load up parameters
   */
  
  Glob->table = smooth_mdv_tdrp_init(&Glob->params);

  if (FALSE == TDRP_read(params_file_path,
			 Glob->table,
			 &Glob->params,
			 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_file_path);
    tidy_and_exit(-1);
  }
  TDRP_free_override(&override);
  
  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    tidy_and_exit(0);
  }
  
  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    tidy_and_exit(0);
  }
  
  if (Glob->params.malloc_debug_level > 0) {
    umalloc_debug(Glob->params.malloc_debug_level);
  }
  
  if (Glob->params.kernel_size % 2 == 0) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Kernel size (set to %ld) must be odd\n",
	    Glob->params.kernel_size);
    tidy_and_exit(-1);
  }
  
  /*
   * initialize index
   */

  RfInitVolFileHandle(&v_handle,
		      Glob->prog_name,
		      Glob->params.input_file_path,
		      (FILE *) NULL);

  /*
   * read in the file
   */

  if (RfReadVolume(&v_handle, "main"))
    tidy_and_exit(-1);

  /*
   * smooth the data
   */

  perform_smoothing(&v_handle);

  /*
   * append a message to the file note, indicating that smoothing
   * has been performed
   */

  if (Glob->params.method == MEAN) {
    sprintf(notebuf, "Mean filter smoothing performed, kernel size %d.\n",
	    (int) Glob->params.kernel_size);
  } else {
    sprintf(notebuf, "Median filter smoothing performed, kernel size %d.\n",
	    (int) Glob->params.kernel_size);
  }
  
  strncat(v_handle.vol_params->note, notebuf,
	  VOL_PARAMS_NOTE_LEN - strlen(v_handle.vol_params->note));

  /*
   * write the file
   */

  fprintf(stderr, "Writing smoothed file %s\n", 
	  Glob->params.output_file_path);

  v_handle.vol_file_path = Glob->params.output_file_path;
      
  if (RfWriteVolume(&v_handle, "main"))
    tidy_and_exit(-1);

  /*
   * quit
   */
  
  tidy_and_exit(0);

  return(0);

}

