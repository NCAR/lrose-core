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
/***************************************************************************
 * polar_ingest.c
 *
 * Reads a radar data stream from an input device, reformats it
 * and places the data in shared memory. The radar beams are stored
 * in a revolving buffer. The shared memory buffer is then read by 
 * client programs.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1991
 *
 ****************************************************************************/

#define MAIN
#include "polar_ingest.h"
#undef MAIN

int main(int argc, char **argv)

{
  
  path_parts_t progname_parts;

  /*
   * allocate global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));

  Glob->semaphores_attached = FALSE;
  Glob->shmem_header_attached = FALSE;
  Glob->shmem_buffer_attached = FALSE;

  /*
   * register function to trap termination and interrupts
   */

  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGKILL, tidy_and_exit);
  PORTsignal(SIGPIPE, SIG_IGN);

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
   * parse the command line arguments - to check for -h etc.
   */
  
  parse_args(argc, argv);

  /*
   * load parameters data base from file
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);
  
  /*
   * read params data base into variables
   */
  
  read_params();
  
  /*
   * parse the command line arguments again - to override any
   * relevant params
   */
  
  parse_args(argc, argv);

  /*
   * initialize process registration
   */
  
  PMU_auto_init(Glob->prog_name, Glob->instance,
		PROCMAP_REGISTER_INTERVAL);
  
  /*
   * if filelist arg set, list the files on the tape
   * and then quit
   */

  if (Glob->filelist) {

    list_tape_files();
    return(0);
    
  } 

  /*
   * set up the semaphores and start polar2mdv as required
   */

  setup_sems();

  /*
   * initialize RDI MMQ
   */

  if (Glob->write_rdi_mmq) {
    if (init_rdi_mmq(Glob->rdi_mmq_key, Glob->nfields_in, Glob->ngates_out,
		     Glob->prog_name, Glob->debug)) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Cannot init RDI MMQ\n");
      tidy_and_exit(-1);
    }
  }

  /*
   * process the data
   */

  process_data_stream();

  tidy_and_exit(0);

  return(0);

}

