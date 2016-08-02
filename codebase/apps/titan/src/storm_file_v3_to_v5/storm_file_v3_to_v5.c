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
 * storm_file_v3_to_v5.c
 *
 * Convert storm file from version 3 to version 5
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * Jan 1996
 *
 ************************************************************************/

#define MAIN
#include "storm_file_v3_to_v5.h"
#undef MAIN

typedef struct {
  
  char *prog_name;
  char *file_path_0;
  char *file_label_0;
  FILE *file_0;
  char *file_path_1;
  char *file_label_1;
  FILE *file_1;
  int index_initialized;
  
} rf_dual_index_t;

static int RfInitDualIndexV3(rf_dual_index_t *index,
			     int size,
			     char *prog_name,
			     char *file_path_0,
			     char *file_label_0,
			     FILE *file_0,
			     char *file_path_1,
			     char *file_label_1,
			     FILE *file_1);
     
int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char **file_paths;
  char *params_file_path = NULL;
  int check_params;
  int print_params;
  si32 n_files = 0;
  si32 ifile;
  path_parts_t progname_parts;
  tdrp_override_t override;
  storm_v3_file_index_t v3_s_handle;
  storm_file_handle_t s_handle;
  
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

  parse_args(argc, argv,
	     &check_params, &print_params,
	     &override,
	     &params_file_path,
	     &n_files, &file_paths);
  
  /*
   * load up parameters
   */
  
  Glob->table = storm_file_v3_to_v5_tdrp_init(&Glob->params);

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
  
  if (Glob->params.malloc_debug_level > 0)
    umalloc_debug(Glob->params.malloc_debug_level);
  
  /*
   * initialize indices
   */

  RfInitDualIndexV3((rf_dual_index_t *) &v3_s_handle,
		    sizeof(storm_v3_file_index_t),
		    Glob->prog_name,
		    (char *) NULL,
		    STORM_V3_HEADER_FILE_TYPE, 
		    (FILE *) NULL,
		    (char *) NULL,
		    STORM_V3_DATA_FILE_TYPE, 
		    (FILE *) NULL);
  
  RfInitStormFileHandle(&s_handle, Glob->prog_name);

  /*
   * loop through the files
   */

  for (ifile = 0; ifile < n_files; ifile++) {

    if (Glob->params.print_file) {
      print_file(&v3_s_handle, file_paths[ifile]);
    } else {
      convert_file(&v3_s_handle, &s_handle, file_paths[ifile]);
    }
    
    umalloc_verify();
    
  } /* ifile */

  tidy_and_exit(0);

  return(0);

}

/*************************************************************************
 *
 * RfInitDualIndex()
 *
 * initializes the memory associated with a generic file index
 *
 **************************************************************************/

static int RfInitDualIndexV3(rf_dual_index_t *index,
			     int size,
			     char *prog_name,
			     char *file_path_0,
			     char *file_label_0,
			     FILE *file_0,
			     char *file_path_1,
			     char *file_label_1,
			     FILE *file_1)
     
{

  /*
   * set fields in index
   */

  memset ((void *)  index,
          (int) 0, (size_t)  size);

  index->prog_name = (char *) umalloc
    ((ui32) (strlen(prog_name) + 1));

  strcpy(index->prog_name, prog_name);

  /*
   * file paths
   */

  if (file_path_0 != NULL) {

    index->file_path_0 = (char *) umalloc
      ((ui32) (strlen(file_path_0) + 1));

    strcpy(index->file_path_0, file_path_0);

  }

  if (file_path_1 != NULL) {

    index->file_path_1 = (char *) umalloc
      ((ui32) (strlen(file_path_1) + 1));

    strcpy(index->file_path_1, file_path_1);

  }

  /*
   * file labels
   */

  if (file_label_0 != NULL) {

    index->file_label_0 = (char *) ucalloc
      ((ui32) 1, (ui32) R_FILE_LABEL_LEN);

    strcpy(index->file_label_0, file_label_0);
    
  }

  if (file_label_1 != NULL) {

    index->file_label_1 = (char *) ucalloc
      ((ui32) 1, (ui32) R_FILE_LABEL_LEN);

    strcpy(index->file_label_1, file_label_1);
    
  }

  /*
   * file pointers
   */

  index->file_0 = file_0;
  index->file_1 = file_1;

  index->index_initialized = TRUE;

  return (R_SUCCESS);

}

