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
/*****************************************************************************
 * ascii_to_xgraph.c
 *
 * takes stdin input, writes xgraph format to stdout
 *
 * Mike Dixon  RAP NCAR   August 1990
 *
 *****************************************************************************/

#define MAIN
#include "ascii_to_xgraph.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  path_parts_t progname_parts;
  FILE *log_file;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    umalloc(sizeof(global_t));
  
  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc (strlen(progname_parts.base) + 1);
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * load up parameters data base
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);

  /*
   * set variables from data base
   */

  read_params();

  /*
   * parse command line arguments
   */

  parse_args(argc, argv);

  /*
   * open log file
   */

  if (!strcmp(Glob->log_file_path, "stdout")) {
    log_file = stdout;
  } else if (!strcmp(Glob->log_file_path, "stderr")) {
    log_file = stderr;
  } else if ((log_file = fopen(Glob->log_file_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Unable to open log file\n");
    perror(Glob->log_file_path);
    return (-1);
  }

  fprintf(log_file, "ASCII_TO_XGRAPH LOG FILE\n\n");
  
  /*
   * filter the data
   */

  if (filter(log_file))
    return (-1);

  /*
   * close log file
   */

  fclose(log_file);

  /*
   * check mallocs
   */

  umalloc_map();
  umalloc_verify();

  return(0);

}

