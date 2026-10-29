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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// SCCS info
//   %W% %D% %T%
//   %F% %E% %U%
//
// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 18:41:15 $
//   $Id: spdb_server.cc,v 1.14 2016/03/07 18:41:15 dixon Exp $
//   $Revision: 1.14 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * spdb_server.cc: spdb_server main program.  This is a generic data
 *                 server that serves out data from spdb files without
 *                 transforming the data in any way.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 1996
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>

#include <tdrp/tdrp.h>

#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "SpdbServer.h"

#include "spdb_server.h"
#include "spdb_server_tdrp.h"


/**********************************************************************
 * Forward declarations for static functions.
 */

static void parse_args(char *prog_name, int argc, char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override);


/**********************************************************************
 * Global variables.
 */

static SpdbServer *Spdb_server;

/**********************************************************************
 * Main program.
 */

int main(int argc, char **argv)
{
  // basic declarations

  path_parts_t progname_parts;
  char *prog_name;
  char *params_path_name;
  
  int check_params;
  int print_params;
  tdrp_override_t override;
  TDRPtable *table;
  spdb_server_tdrp_struct params;
  
  SpdbServerDebugLevel spdb_debug_level;
  
  // set program name

  uparse_path(argv[0], &progname_parts);
  prog_name = STRdup(progname_parts.base);

  // display ucopyright message

  ucopyright(prog_name);

  // register function to trap termination and interrupts

  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);

  PORTsignal(SIGPIPE, handle_sigpipe);
  
  // parse the command line arguments, and open files as required

  parse_args(prog_name, argc, argv,
	     &check_params, &print_params,
	     &params_path_name, &override);

  // load up parameters

  table = spdb_server_tdrp_init(&params);
  
  if (FALSE == TDRP_read(params_path_name,
			 table,
			 &params,
			 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_path_name);
    tidy_and_exit(-1);
  }
  
  TDRP_free_override(&override);
  
  if (check_params)
  {
    TDRP_check_is_set(table, &params);
    tidy_and_exit(-1);
  }
  
  if (print_params)
  {
    TDRP_print_params(table, &params, prog_name, TRUE);
    tidy_and_exit(-1);
  }
  
  // Set the malloc debug level

  umalloc_debug(params.malloc_debug_level);
  
  // Convert the debug level to the SpdbServerDebugLevel value.

  switch (params.debug)
  {
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

  } /* endswitch - params.debug */
  
  // Create the server object

  Spdb_server = new SpdbServer(params.port,
			       params.product_label,
			       params.product_id,
			       params.database_dir,
			       prog_name,
			       params.servmap_type,
			       params.servmap_subtype,
			       params.servmap_instance,
			       64,
			       NULL,
			       params.product_id,
			       NULL,
			       params.product_id,
			       params.wait_msecs,
			       params.realtime_avail,
			       spdb_debug_level,
			       params.latest_data_only_for_type);
  
  // Operate as a server

  Spdb_server->operate();
  
  exit(0);
}


/*********************************************************************
 * tidy_and_exit() - Clean up any memory, etc. and exit the program.
 */

void tidy_and_exit(int sig)
{

  // verify mallocs

  umalloc_map();
  umalloc_verify();

  // exit with code sig

  exit(sig);

}


/*********************************************************************
 * handle_sigpipe() - Close all open sockets when a SIGPIPE signal is
 *                    received.
 */

void handle_sigpipe(int sig)
{
  // fool compiler into not printing unused arg warning

  int i;
  i = sig;

  // Close the SpdbServer sockets
  Spdb_server->handleSigpipe();
  
  return;
}


/*******************************************************************
 * parse_args() - Parse the command line arguments.
 */

#define TMP_STR_LEN 8192

static void parse_args(char *prog_name, int argc, char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override)
{
  int error_flag = 0;
  int warning_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[TMP_STR_LEN];
  
  // Initialize the returned values

  *check_params_p = FALSE;
  *print_params_p = FALSE;
  *params_file_path_p = NULL;
  TDRP_init_override(override);
  
  // load up usage string

  sprintf(usage, "%s%s%s",
	  "Usage:\n\n", prog_name, " [options] as below:\n\n"
	  "       [ --, -help, -man] produce this list.\n"
	  "       [ -check_params] check parameter usage\n"
	  "       [ -data data_dir] input data directory\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -mdebug level] set malloc debug level\n"
	  "       [ -params name] parameters file name\n"
	  "       [ -print_params] print parameter usage\n"
	  "       [ -port port] output port number\n"
	  "\n");

  // search for command options
  
  for (i = 1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      printf("%s", usage);
      tidy_and_exit(1);
    }
    else if (STRequal_exact(argv[i], "-check_params"))
    {
      *check_params_p = TRUE;
    }
    else if (STRequal_exact(argv[i], "-print_params"))
    {
      *print_params_p = TRUE;
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = DEBUG_ALL;");
      TDRP_add_override(override, tmp_str);
    }
    else if (i < argc - 1)
    {
      if (STRequal_exact(argv[i], "-params"))
      {
	*params_file_path_p = argv[i+1];
	i++;
      }
      else if (STRequal_exact(argv[i], "-mdebug"))
      {
	sprintf(tmp_str, "malloc_debug_level = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;
      }
      else if (STRequal_exact(argv[i], "-data"))
      {
	sprintf(tmp_str, "database_dir = \"%s\";", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;
      }
      else if (STRequal_exact(argv[i], "-port"))
      {
	sprintf(tmp_str, "port = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;
      }
      
    } /* if (i < argc - 1) */
    
  } /* i */

  // print usage if there was an error

  if(error_flag || warning_flag)
  {
    fprintf(stderr, "%s\n", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    *params_file_path_p);
  }

  if (error_flag)
    tidy_and_exit(1);

  return;
}
