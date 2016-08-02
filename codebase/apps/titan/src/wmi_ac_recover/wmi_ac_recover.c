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
/**********************************************************************
 * Function:       wmi_ac_recover (main)                                          
 *
 * Description:              
 * Reads in SEA MD200 formatted data and gets raw state data 
 * All raw is Little Endian --- therefore, depending on the
 * machine type, one will need to swap either the fortran header
 * blocks or the data, BUT NOT BOTH.
 * After all the data is read in, a genpro file will be made.
 * All data will be written in BE -- both fortran record blocks AND 
 * the data blocks will be BE.
 * 
 **********************************************************************/

#define MAIN
#include "wmi_ac_recover.h"
#undef MAIN

/**********************************************************************/

si32 main(int argc, char **argv)

{

  char *params_file_path = NULL;
  ui08 buf[MAX_REC];

  si32 errflg = OKAY;
  si32 bytes_read = 0; 

  int check_params;
  int print_params;
  int in_fid;

  path_parts_t progname_parts;
  tdrp_override_t override;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *) umalloc((unsigned int) sizeof(global_t));

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((unsigned int) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * parse command line arguments
   */
  
  parse_args(argc, argv,
	     &check_params, &print_params,
	     &override, &params_file_path);

  /*
   * load up parameters
   */
  
  Glob->table = wmi_ac_recover_tdrp_init(&Glob->params);
  
  if (FALSE == TDRP_read(params_file_path,
			 Glob->table,
			 &Glob->params,
			 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_file_path);
    exit(-1);
  }
  
  TDRP_free_override(&override);
  
  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    exit(0);
  }
  
  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    exit(0);
  }
  
  if (Glob->params.malloc_debug_level > 0) {
    umalloc_debug(Glob->params.malloc_debug_level);
  }

  /*
   * read in tables
   */

  read_tables();

  /*
   * open the input dataset
   */
  
  if ((in_fid = open(Glob->params.tape_name, O_RDONLY)) == ERROR) {
    fprintf(stderr, "Error opening %s \n",Glob->params.tape_name);
    exit_str("Error opening input dataset");
  }
  TTAPE_set_var(in_fid);
  
  /*
   * Read in first data buffer
   */
  
  if (Glob->params.debug) {
    fprintf(stderr,"Starting to read from %s\n",Glob->params.tape_name);
  }
  
  bytes_read = read(in_fid, buf, MAX_REC); 
  
  /*
   * now parse through the data
   */
  
  while (bytes_read > 0) {

    if (bytes_read == Glob->state_buf_size) {
      
      /*
       * swap data if necessary
       */
      
      if (Glob->swap_data) {
	errflg = swap_ddir((DataDir *)buf);
	if (errflg == ERROR) exit_str("Could not swap data correctly");
      }
      
      parse_raw_data(buf);
      
    }   /* endif reading a state buffer */
    
    /*
     * read more data
     */
    
    bytes_read = read(in_fid, buf, MAX_REC); 
    
  }  /* end while there is still data loop */
  
  close(in_fid);
  
  if (Glob->params.debug) fprintf(stdout,"No more data.\n");

  if (Glob->params.debug) {
    fprintf(stdout, "\n%s Normal Termination \n", Glob->prog_name);
  }
  
  ufree(Glob->acqtbl);
  ufree(Glob->fc);
  ufree(Glob->cv);
  ufree(Glob);
  
  exit(OKAY);
  
}
