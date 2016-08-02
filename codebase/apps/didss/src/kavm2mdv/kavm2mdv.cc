// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * kavm2mdv.cc
 *
 * Convert Kavouras mosaic grids to MDV format.
 *
 * Nancy Rehak  RAP NCAR Boulder CO USA
 *
 * January 1997
 *
 * Based on kav2dobson.c by Mike Dixon
 *
 **********************************************************************/

#include <toolsa/str.h>

#define MAIN
#include "kavm2mdv.h"
using namespace std;
#undef MAIN

int main(int argc, char **argv)
{
  /*
   * basic declarations
   */

  char *params_file_path = NULL;
  char **input_file_list;
  char *file_path;

  int check_params;
  int print_params;
  int forever = TRUE;
  int i;

  si32 n_input_files = 0;

  path_parts_t progname_parts;
  vol_file_handle_t v_handle;
  tdrp_override_t override;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = STRdup(progname_parts.base);
  
  /*
   * display ucopright message
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
  
  Glob->table = kavm2mdv_tdrp_init(&Glob->params);

  if (TDRP_read(params_file_path,
		Glob->table,
		&Glob->params,
		override.list) == FALSE)
  {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Problems with params file '%s'\n",
	    params_file_path);
    tidy_and_exit(-1);
  }
  
  TDRP_free_override(&override);
  
  if (check_params)
  {
    TDRP_check_is_set(Glob->table, &Glob->params);
    tidy_and_exit(0);
  }
  
  if (print_params)
  {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    tidy_and_exit(0);
  }
  
  if (Glob->params.malloc_debug_level > 0)
  {
    umalloc_debug(Glob->params.malloc_debug_level);
  }

  if (Glob->params.mode == ARCHIVE && n_input_files == 0)
  {
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
   * initialize vol file index
   */

  init_vol_index(&v_handle);

  /*
   * process the files
   */

  if (Glob->params.mode == ARCHIVE)
  {
    for (i = 0; i < n_input_files; i++)
      process_file(&v_handle, input_file_list[i]);
  }
  else
  {
    while (forever)
    {
      /*
       * wait for new data
       */
      
      file_path = get_next_file();

      /*
       * wait if required
       */

      if (Glob->params.processing_delay > 0)
	sleep(Glob->params.processing_delay);

      /*
       * process the file
       */
      
      process_file(&v_handle, file_path);

      umalloc_map();
      
    } /* forever */

  } /* if (Glob->params.mode == ARCHIVE) */

  /*
   * quit
   */
  
  tidy_and_exit(0);
}

