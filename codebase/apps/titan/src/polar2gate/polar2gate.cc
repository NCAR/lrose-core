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
/**********************************************************************
 * polar2gate.c
 *
 * Reads a mile high radar tape, extracts the data for the relevant
 * data structs and writes the data beam-by-beam to an output
 * socket. This socket is intended to be read by polar_ingest.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * September 1992
 *
 **********************************************************************/

#define MAIN
#include "polar2gate.h"
#undef MAIN

#include <signal.h>

int main(int argc, char **argv)

{
  
  path_parts_t progname_parts;

  /*
   * allocate global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));

  /*
   * register function to trap termination and interrupts
   */

  signal(SIGQUIT, tidy_and_exit);
  signal(SIGTERM, tidy_and_exit);
  signal(SIGINT, tidy_and_exit);
  signal(SIGPIPE, SIG_IGN);

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
   * load parameters data base from file
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);
  
  /*
   * read params data base into variables
   */
  
  read_params();
  
  /*
   * parse the command line arguments
   */
  
  parse_args(argc, argv);

  /*
   * if filelist arg set, list the files on the tape
   * and then quit
   */

  if (Glob->filelist) {

    list_tape_files();
    return(0);

  } 

  /*
   * Process the data
   */

  process_data_stream();

  tidy_and_exit(0);

  return(0);

}

