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
/*******************************************************************************
 * ppi2comp.c
 *
 * Creates a composite cartesian file from a number of
 * cartesian ppi files.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Jan 1996
 *
 *******************************************************************************/

#define MAIN
#include "ppi2comp.h"
#undef MAIN

int main(int argc, char **argv)

{

  char *params_file_path = NULL;
  char **input_file_list;
  char file_path[MAX_PATH_LEN];

  int check_params;
  int print_params;
  int forever = TRUE;
  int i;

  si32 n_input_files = 0;

  path_parts_t progname_parts;
  tdrp_override_t override;
  LDATA_handle_t ldata;

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);

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
	     &params_file_path, &override,
	     &n_input_files, &input_file_list);

  /*
   * load up parameters
   */
  
  Glob->table = ppi2comp_tdrp_init(&Glob->params);
  
  if (FALSE == TDRP_read(params_file_path,
			 Glob->table,
			 &Glob->params,
			 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Problems with params file '%s'\n",
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

  if (Glob->params.mode == ARCHIVE && n_input_files == 0) {
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "For ARCHIVE mode must specify input file list\n");
    tidy_and_exit(-1);
  }

  /*
   * initialize process registration
   */

  PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  /*
   * process the files
   */

  if (Glob->params.mode == ARCHIVE) {
    
    for (i = 0; i < n_input_files; i++) {
      
      process_ppis(input_file_list[i]);
      umalloc_map();

    }

  } else {

    /*
     * REALTIME
     */
    
    LDATA_init_handle(&ldata, Glob->prog_name, Glob->params.debug);

    while (forever) {
      
      /*
       * get latest data time, set end time to this
       */
      
      LDATA_info_read_blocking(&ldata, Glob->params.input.val[0].dir,
			       Glob->params.input.val[0].max_valid_age,
			       2000, PMU_auto_register);

      sprintf(file_path, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	      Glob->params.input.val[0].dir,
	      PATH_DELIM,
	      ldata.ltime.year, ldata.ltime.month, ldata.ltime.day,
	      PATH_DELIM,
	      ldata.ltime.hour, ldata.ltime.min, ldata.ltime.sec,
	      ldata.info.file_ext);
      
      /*
       * process the file
       */
      
      process_ppis(file_path);
      umalloc_map();
      
    } /* forever */

    LDATA_free_handle(&ldata);
    
  } /* if (Glob->params.mode == ARCHIVE) */
  
  /*
   * quit
   */
  
  tidy_and_exit(0);

  return (0);

}
