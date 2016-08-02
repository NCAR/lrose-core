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
 * trec_gauge_spdb2symprod.cc
 *
 * trec_gauge_spdb2symprod main program.  This server
 * serves out trec gauge data from an SPDB database
 * in symprod format so that it can be easily
 * displayed on the RAP displays.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#define MAIN
#include "trec_gauge_spdb2symprod.h"
#undef MAIN

int main(int argc, char **argv)

{
  // basic declarations

  path_parts_t progname_parts;
  char *params_path_name;
  
  int check_params;
  int print_params;
  tdrp_override_t override;
  TDRPtable *table;
  
  SpdbServerDebugLevel spdb_debug_level;
  
  // set program name

  uparse_path(argv[0], &progname_parts);
  Prog_name = STRdup(progname_parts.base);

  // display ucopyright message

  ucopyright(Prog_name);

  // register function to trap termination and interrupts

  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGPIPE, handle_sigpipe);
  
  // parse the command line arguments, and open files as required

  parse_args(Prog_name, argc, argv,
	     &check_params, &print_params,
	     &params_path_name, &override);

  // load up parameters

  table = trec_gauge_spdb2symprod_tdrp_init(&Params);
  
  if (params_path_name) {
    if (TDRP_read(params_path_name,
		  table, &Params,
		  override.list) == FALSE) {
      fprintf(stderr, "ERROR - %s\n",
	      Prog_name);
      fprintf(stderr, "Cannot read params file '%s'\n",
	      params_path_name);
      tidy_and_exit(-1);
    }
  } else {
    TDRP_set_defaults(table, &Params);
  }
  
  TDRP_free_override(&override);
  
  if (check_params) {
    TDRP_check_is_set(table, &Params);
    tidy_and_exit(-1);
  }
  
  if (print_params) {
    TDRP_print_params(table, &Params, Prog_name, TRUE);
    tidy_and_exit(-1);
  }
  
  // Set the malloc debug level

  umalloc_debug(Params.malloc_debug_level);
  
  // Convert the debug level to the SpdbServerDebugLevel value.

  switch (Params.debug) {
  case DEBUG_OFF:
    spdb_debug_level = SPDB_SERVER_DEBUG_OFF;
    break;
    
  case DEBUG_ERRORS:
    spdb_debug_level = SPDB_SERVER_DEBUG_ERRORS;
    break;
    
  case DEBUG_MSGS:
    spdb_debug_level = SPDB_SERVER_DEBUG_MSGS;
    break;
    
  case DEBUG_ROUTINES:
    spdb_debug_level = SPDB_SERVER_DEBUG_ROUTINES;
    break;
    
  case DEBUG_ALL:
    spdb_debug_level = SPDB_SERVER_DEBUG_ALL;
    break;
    
  } /* endswitch - Params.debug */
  
  // Create the server object

  Spdb_server = new SpdbServer(Params.port,
			       Params.product_label,
			       Params.product_id,
			       Params.database_dir,
			       Prog_name,
			       Params.servmap_type,
			       Params.servmap_subtype,
			       Params.servmap_instance,
			       64,
			       NULL,
			       -1,
			       convert_to_symprod,
			       SPDB_SYMPROD_ID,
			       1000,
			       Params.realtime_avail,
			       spdb_debug_level);
  
  // Operate as a server

  Spdb_server->operate();
  
  return (0);

}

