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
 * dsr2mmq.c
 *
 * Reads DsRadar format, processes the beam and sends it on to an mmq.
 *
 * Jaimi Yee
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1998 
 *
 ****************************************************************************/

#define MAIN
#include "dsr2mmq.h"
using namespace std;
#undef MAIN

int main(int argc, char **argv)

{
  char *params_file_path = NULL;

  int check_params;
  int print_params;
  
  path_parts_t progname_parts;
  tdrp_override_t override;

  /*
   * allocate global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t)); 

  /*
   * initialize radar queue pointer to NULL
   */
  Glob->radarQueue = NULL;

  /*
   * register function to trap termination and interrupts
   */

  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGKILL, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);


  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);

  /*
   * display ucopright message
   */

  ucopyright(Glob->prog_name);

  /*
   * parse the command line arguments
   */
  
  parse_args(argc, argv,
             &check_params, &print_params,
             &override,
             &params_file_path);

  /*
   * load up parameters
   */

  Glob->table = dsr2mmq_tdrp_init(&Glob->params);

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

  /*
   * initialize process registration
   */
  
    PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);

  /*
   * Instantiate and initialize the DsRadar queue and message
   */
  Glob->radarQueue = new DsRadarQueue;
  Glob->radarMsg   = new DsRadarMsg;

  if (Glob->radarQueue->initReadBlocking( Glob->params.fmq_url,
					  Glob->prog_name,
					  (bool)Glob->params.debug,
					  DsFmq::END,
					  50)) {
     fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
     fprintf(stderr, "Could not initialize radar queue '%s'\n",
                     Glob->params.fmq_url);
     tidy_and_exit(-1);
     
  }

  if( Glob->params.use_ngates ) {
     Glob->radarMsg->padBeams( true, Glob->params.ngates );
  }

  /*
   * read in the data and setup rdi message
   */

  process_data();
  
  tidy_and_exit(0);

  return(0);

}
