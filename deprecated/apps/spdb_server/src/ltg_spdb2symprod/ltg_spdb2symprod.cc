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
//   $Date: 2016/03/07 18:41:15 $
//   $Id: ltg_spdb2symprod.cc,v 1.11 2016/03/07 18:41:15 dixon Exp $
//   $Revision: 1.11 $
//   $State: Exp $

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ltg_spdb2symprod.cc: ltg_spdb2symprod main program.  This server
 *                      serves out lightning data from an SPDB database
 *                      in symprod format so that it can be easily
 *                      displayed on the RAP displays.
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 1997
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

#include <rapformats/ltg.h>
#include <symprod/spdb_products.h>
#include <symprod/symprod.h>
#include <tdrp/tdrp.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/smu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <SpdbServer/SpdbServer.h>

#include "Params.hh"


/**********************************************************************
 * Forward declarations for static functions.
 */

static void *convert_to_symprod(spdb_chunk_ref_t *spdb_hdr,
				void *spdb_data,
				int spdb_len,
				int *symprod_len);

static void handle_sigpipe(int sig);

static void parse_args(char *prog_name,
		       int argc, char **argv,
		       tdrp_override_t *override);

static void tidy_and_exit(int sig);


/**********************************************************************
 * Global variables.
 */

static SpdbServer *Spdb_server = NULL;
static Params *_params;

/**********************************************************************
 * Main program.
 */

int main(int argc, char **argv)
{
  // basic declarations

  path_parts_t progname_parts;
  char *prog_name;
  char *params_path_name;
  
  tdrp_override_t override;
  
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

  parse_args(prog_name, argc, argv, &override);
  
  // get TDRP params
  
  _params = new Params();
  params_path_name = "unknown";
  if (_params->loadFromArgs(argc, argv,
			    override.list,
			    &params_path_name)) {
    fprintf(stderr, "ERROR - %s\n", prog_name);
    fprintf(stderr, "Problem with TDRP parameters\n");
    tidy_and_exit(-1);
  }
  TDRP_free_override(&override);
  
  // Set the malloc debug level

  umalloc_debug(_params->malloc_debug_level);
  
  // Convert the debug level to the SpdbServerDebugLevel value.

  switch (_params->debug)
  {
  case Params::DEBUG_OFF:
    spdb_debug_level = SPDB_SERVER_DEBUG_OFF;
    break;

  case Params::DEBUG_ERRORS:
    spdb_debug_level = SPDB_SERVER_DEBUG_ERRORS;
    break;

  case Params::DEBUG_MSGS:
    spdb_debug_level = SPDB_SERVER_DEBUG_MSGS;
    break;

  case Params::DEBUG_ROUTINES:
    spdb_debug_level = SPDB_SERVER_DEBUG_ROUTINES;
    break;

  case Params::DEBUG_ALL:
    spdb_debug_level = SPDB_SERVER_DEBUG_ALL;
    break;

  } /* endswitch - _params->debug */
  
  // Create the server object

  Spdb_server = new SpdbServer(_params->port,
			       _params->product_label,
			       _params->product_id,
			       _params->database_dir,
			       prog_name,
			       _params->servmap_type,
			       _params->servmap_subtype,
			       _params->servmap_instance,
			       64,
			       NULL,
			       -1,
			       convert_to_symprod,
			       SPDB_SYMPROD_ID,
			       _params->wait_msecs,
			       _params->realtime_avail,
			       spdb_debug_level);
  
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
  static symprod_ppt_t ltg_icon[5] = { { -4,  0 },
				       {  4,  0 }, 
				       {  0,  0 }, 
				       {  0, -4 }, 
				       {  0,  4 } };

  static symprod_wpt_t *icon_origins = NULL;
  static int icon_origins_alloc = 0;

  // set the icon values

  ltg_icon[0].x = -(_params->icon_size);
  ltg_icon[1].x = _params->icon_size;
  ltg_icon[3].y = -(_params->icon_size);
  ltg_icon[4].y = _params->icon_size;
  
  LTG_strike_t ltg_strike;
  
  char *return_buffer;
  
  // Initialize returned values

  *symprod_len = 0;
  
  // Make sure the data received from the spdb database is of the
  // correct length.

  assert(spdb_len % sizeof(LTG_strike_t) == 0);
  
  // Determine how many lightning strikes are in the buffer

  int num_strikes = spdb_len / sizeof(LTG_strike_t);

  // Convert the SPDB data to SYMPROD format

  time_t current_time = time((time_t *)NULL);
  
  symprod_product_t *symprod_prod =
    SYMPRODcreateProduct(current_time,
			 current_time,
			 spdb_hdr->valid_time,
			 spdb_hdr->expire_time);
  
  // Make sure the origins buffer is big enough

  if (icon_origins_alloc < num_strikes)
  {
    icon_origins_alloc = num_strikes;
    
    if (icon_origins == (symprod_wpt_t *)NULL)
      icon_origins = (symprod_wpt_t *)umalloc(icon_origins_alloc *
					      sizeof(symprod_wpt_t));
    else
      icon_origins = (symprod_wpt_t *)urealloc(icon_origins,
					       icon_origins_alloc *
					       sizeof(symprod_wpt_t));
  }
  
  // Process each strike to get the origins

  for (int strike = 0; strike < num_strikes; strike++)
  {
    // Copy the SPDB data to the local "buffer"

    memcpy((char *)&ltg_strike,
	   (char *)spdb_data + (strike * sizeof(LTG_strike_t)),
	   sizeof(LTG_strike_t));
  
    // Convert the SPDB data to native format so we can use it.

    LTG_from_BE(&ltg_strike);
  
    if (_params->debug >= Params::DEBUG_ALL)
    {
      SPDB_print_chunk_ref(spdb_hdr,
			   stdout);
    
      LTG_print_strike(stdout, &ltg_strike);
    }
  
    // Save the origin in the origin buffer

    icon_origins[strike].lat = ltg_strike.latitude;
    icon_origins[strike].lon = ltg_strike.longitude;
  
  } /* endfor - strike */
  
  SYMPRODaddStrokedIcon(symprod_prod,
			_params->display_color,
			sizeof(ltg_icon) / sizeof(symprod_ppt_t),
			ltg_icon,
			num_strikes,
			icon_origins);
  
  // Create the return buffer

  return_buffer = SYMPRODproductToBuffer(symprod_prod, symprod_len);
  
  SYMPRODfreeProduct(symprod_prod);
  
  // Check everything

  if (_params->debug >= Params::DEBUG_ALL)
    SYMPRODprintProductBuffer(stdout, return_buffer);
  
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

static void parse_args(char *prog_name,
		       int argc, char **argv,
		       tdrp_override_t *override)
{
  int error_flag = 0;
  int warning_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];
  
  // load up usage string

  sprintf(usage, "%s%s%s",
	  "Usage:\n\n", prog_name, " [options] as below:\n\n"
	  "       [ --, -help, -h, -man] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -mdebug level] set malloc debug level\n"
	  "       [ -data data_dir] input data directory\n"
	  "       [ -port port] output port number\n"
	  "\n");

  // initialize

  TDRP_init_override(override);
  
  // search for command options
  
  for (i = 1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      printf("%s", usage);
      TDRP_usage(stdout);
      tidy_and_exit(1);
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = DEBUG_ALL;");
      TDRP_add_override(override, tmp_str);
    }
    else if (i < argc - 1)
    {
      if (STRequal_exact(argv[i], "-mdebug"))
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
    TDRP_usage(stderr);
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
  // Unregister with the process mapper

  PMU_auto_unregister();
  
  // Unregister with the server mapper

  SMU_auto_unregister();
  
  // Delete the server object

  if (Spdb_server != NULL)
    delete Spdb_server;
  
  // verify mallocs

  umalloc_map();
  umalloc_verify();

  // exit with code sig

  exit(sig);

}
