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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 18:41:14 $
//   $Id: chunk_spdb2symprod.cc,v 1.10 2016/03/07 18:41:14 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * chunk_spdb2symprod.cc: chunk_spdb2symprod main program.  This server
 *                        serves out chunk data from an SPDB database
 *                        in symprod format so that it can be easily
 *                        displayed on the RAP displays.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/param.h>
#include <sys/types.h>

#include <symprod/spdb_products.h>
#include <symprod/symprod.h>
#include <tdrp/tdrp.h>
#include <toolsa/mem.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <SpdbServer/SpdbServer.h>

#include "chunk_spdb2symprod_tdrp.h"


/**********************************************************************
 * Forward declarations for static functions.
 */

static void *convert_to_symprod(spdb_chunk_ref_t *spdb_hdr,
				void *spdb_data,
				int spdb_len,
				int *symprod_len);

static void handle_sigpipe(int sig);

static void parse_args(char *prog_name, int argc, char **argv,
		       int *check_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override);

static void tidy_and_exit(int sig);


/**********************************************************************
 * Global variables.
 */

static SpdbServer *Spdb_server = NULL;
static chunk_spdb2symprod_tdrp_struct Params;

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
  tdrp_override_t override;
  TDRPtable *table;
  
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
	     &check_params, &params_path_name, &override);

  // load up parameters

  table = chunk_spdb2symprod_tdrp_init(&Params);
  
  if (params_path_name)
  {
    if (TDRP_read(params_path_name,
		  table, &Params,
		  override.list) == FALSE)
    {
      fprintf(stderr, "ERROR - %s\n",
	      prog_name);
      fprintf(stderr, "Cannot read params file '%s'\n",
	      params_path_name);
      tidy_and_exit(-1);
    }
  }
  else
  {
    TDRP_set_defaults(table, &Params);
  }
  
  TDRP_free_override(&override);
  
  if (check_params)
  {
    TDRP_check_is_set(table, &Params);
    TDRP_print_params(table, &Params, prog_name, TRUE);
    tidy_and_exit(-1);
  }
  
  // Set the malloc debug level

  umalloc_debug(Params.malloc_debug_level);
  
  // Convert the debug level to the SpdbServerDebugLevel value.

  switch (Params.debug)
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

  } /* endswitch - Params.debug */
  
  // Create the server object

  Spdb_server = new SpdbServer(Params.port,
			       Params.product_label,
			       Params.product_id,
			       Params.database_dir,
			       prog_name,
			       Params.servmap_type,
			       Params.servmap_subtype,
			       Params.servmap_instance,
			       64,
			       NULL,
			       -1,
			       convert_to_symprod,
			       SPDB_SYMPROD_ID,
			       Params.wait_msecs,
			       Params.realtime_avail,
			       spdb_debug_level,
 			       Params.latest_data_only_for_type);
  
  // Operate as a server

  Spdb_server->operate();
  
  exit(0);
}


/*********************************************************************
 * convert_to_symprod() - Convert the data from the SPDB database to
 *                        symprod format.
 */

void *convert_to_symprod(spdb_chunk_ref_t *spdb_hdr,
			 void *spdb_data,
			 int spdb_len,
			 int *symprod_len)
{
  char *return_buffer;
  
  // Initialize returned values

  *symprod_len = 0;
  
  // Convert the SPDB chunk to SYMPROD format

  time_t current_time = time((time_t *)NULL);
  
  symprod_product_t *symprod_prod =
    SYMPRODcreateProduct(current_time,
			 current_time,
			 spdb_hdr->valid_time,
			 spdb_hdr->expire_time);
  
  SYMPRODaddChunk(symprod_prod,
		  Params.color,
		  "",
		  -1.0,
		  -1.0,
		  -1.0,
		  -1.0,
		  Params.chunk_type,
		  0,
		  (char *) spdb_data,
		  spdb_len);
  
  // Create the return buffer

  return_buffer = SYMPRODproductToBuffer(symprod_prod,
					 symprod_len);
  
  SYMPRODfreeProduct(symprod_prod);
  
  // Check everything

  if (Params.debug >= DEBUG_ALL)
    SYMPRODprintProductBuffer(stderr, return_buffer);
  
  // Put the product buffer in BE format for transmission

  SYMPRODproductBufferToBE(return_buffer);
  
  return(return_buffer);
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

static void parse_args(char *prog_name, int argc, char **argv,
		       int *check_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override)
{
  int error_flag = 0;
  int warning_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];
  
  // Initialize the returned values

  *check_params_p = FALSE;
  *params_file_path_p = NULL;
  
  // load up usage string

  sprintf(usage, "%s%s%s",
	  "Usage:\n\n", prog_name, " [options] as below:\n\n"
	  "       [ --, -help, -man] produce this list.\n"
	  "       [ -check_params] check parameter usage\n"
	  "       [ -print_params] print parameter usage\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -mdebug level] set malloc debug level\n"
	  "       [ -params name] parameters file name\n"
	  "       [ -data data_dir] input data directory\n"
	  "       [ -port port] output port number\n"
	  "\n");

  // initialize

  *check_params_p = FALSE;
  TDRP_init_override(override);
  
  // search for command options
  
  for (i = 1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      printf("%s", usage);
      tidy_and_exit(1);
    }
    else if (STRequal_exact(argv[i], "-check_params") ||
	     STRequal_exact(argv[i], "-print_params"))
    {
      *check_params_p = TRUE;
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = DEBUG_MSGS;");
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

  if (error_flag || warning_flag)
  {
    fprintf(stderr, "%s\n", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    *params_file_path_p);
  }

  if (error_flag)
    tidy_and_exit(1);

  return;
}


/*********************************************************************
 * tidy_and_exit() - Clean up any memory, etc. and exit the program.
 */

void tidy_and_exit(int sig)
{
  // delete the server object

  if (Spdb_server != NULL)
    delete Spdb_server;
  
  // verify mallocs

  umalloc_map();
  umalloc_verify();

  // exit with code sig

  exit(sig);

}
