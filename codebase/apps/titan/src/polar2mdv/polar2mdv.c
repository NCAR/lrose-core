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
/***************************************************************************
 * polar2mdv.c
 *
 * reads a mile-high beam from shared-memory buffer
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1990
 *
 * Hacked from 'getray' code
 *
 ****************************************************************************/

#define MAIN
#include "polar2mdv.h"
#undef MAIN

int main(int argc, char **argv)

{

  path_parts_t progname_parts;
  rc_table_file_handle_t rc_handle;
  clutter_table_file_handle_t clutter_handle;
  vol_file_handle_t v_handle;


  /*
   * allocate global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));
  memset ((void *)  Glob,
          (int) 0, (size_t) sizeof(global_t));

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
   * setup system function utility
   */

  if (usystem_call_init()) {
    
    fprintf(stderr, "ERROR - %s:main:usystem_call_init\n",
	    Glob->prog_name);
    tidy_and_exit(-1);
    
  }
  
  /*
   * register function to trap termination and interrupts
   */

  PORTsignal(SIGTERM, (void (*)())tidy_and_exit);
  PORTsignal(SIGINT, (void (*)())tidy_and_exit);
  PORTsignal(SIGKILL, (void (*)())tidy_and_exit);
  PORTsignal(SIGQUIT, (void (*)())tidy_and_exit);
  PORTsignal(SIGPIPE, (void (*)())SIG_IGN);

  /*
   * load params data base
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);

  /*
   * read parameters data base to set global variables
   */

  read_params();

  /*
   * parse the command line arguments
   */

  parse_args(argc, argv);

  /*
   * initialize process registration
   */
  
  PMU_auto_init(Glob->prog_name, Glob->instance,
		PROCMAP_REGISTER_INTERVAL);
  
  /*
   * if required, read in radar to cartesian table
   */
  
  RfInitRcTableHandle(&rc_handle,
		      Glob->prog_name,
		      Glob->rc_table_path,
		      (FILE *) NULL);

  if (RfReadRcTable(&rc_handle, "main") != R_SUCCESS)
    tidy_and_exit(-1);
  
  /*
   * if required, read in clutter table
   */
  
  if (Glob->remove_clutter) {
    
    RfInitClutterHandle(&clutter_handle,
			Glob->prog_name,
			Glob->clutter_table_path,
			(FILE *) NULL);

    if (RfReadClutterTable(&clutter_handle,
			   "main") != R_SUCCESS)
      tidy_and_exit(-1);
    
  } /* if (Glob->remove_clutter) */

  /*
   * setup shared memory
   */

  setup_shmem(rc_handle.scan_table);

  /*
   * check that the geometry and other values of the radar, rc_table,
   * and clutter_table agree
   */

  check_geom(&rc_handle, &clutter_handle);

  /*
   * initialize the volume file handle
   */

  init_cart_vol_index(&v_handle, &rc_handle, &clutter_handle);
  
  /*
   * read the beams
   */

  read_shmem(&v_handle, &rc_handle, &clutter_handle);

  return (0);

}

