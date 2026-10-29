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
//   $Id: sigmet_spdb2symprod.cc,v 1.5 2016/03/07 18:41:15 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * sigmet_spdb2symprod.cc: sigmet_spdb2symprod main program.  This
 *                         server serves out SIGMET polygons from an
 *                         SPDB database in symprod format so that it
 *                         can be easily displayed on the RAP displays.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 1997
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

#include <rapformats/fos.h>
#include <toolsa/toolsa_macros.h>
#include <symprod/spdb_products.h>
#include <symprod/symprod.h>
#include <tdrp/tdrp.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <SpdbServer/SpdbServer.h>

#include "sigmet_spdb2symprod_tdrp.h"


/**********************************************************************
 * Forward declarations for static functions.
 */

static void add_polygon_to_buffer(symprod_product_t *prod,
				  SIGMET_spdb_t *sigmet);

static int convert_capstyle_param(int capstyle);

static int convert_fill_param(int fill);

static int convert_joinstyle_param(int joinstyle);

static int convert_line_type_param(int line_type);

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
static sigmet_spdb2symprod_tdrp_struct Params;

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

  table = sigmet_spdb2symprod_tdrp_init(&Params);
  
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
			       spdb_debug_level);
  
  // Operate as a server

  Spdb_server->operate();
  
  exit(0);
}


/*********************************************************************
 * add_polyline_to_buffer() - Add a SYMPROD polyline object to the
 *                            indicated buffer.  Returns the size in
 *                            bytes of the buffer object created.
 */

static void add_polygon_to_buffer(symprod_product_t *prod,
				  SIGMET_spdb_t *sigmet)
{
  static symprod_wpt_t *pts = NULL;
  static int pts_alloc = 0;
  
  int pt_size = sigmet->num_vertices * sizeof(symprod_wpt_t);
  
  if (pts_alloc < pt_size)
  {
    if (pts == NULL)
      pts = (symprod_wpt_t *)umalloc(pt_size);
    else
      pts = (symprod_wpt_t *)urealloc(pts, pt_size);
    
    pts_alloc = pt_size;
  }
  
  for (int pt = 0; pt < sigmet->num_vertices; pt++)
  {
    pts[pt].lat = sigmet->vertices[pt].lat;
    pts[pt].lon = sigmet->vertices[pt].lon;
  }
  
  SYMPRODaddPolyline(prod,
		     Params.polygon_color,
		     convert_line_type_param(Params.display_linetype),
		     Params.display_line_width,
		     convert_capstyle_param(Params.display_capstyle),
		     convert_joinstyle_param(Params.display_joinstyle),
		     TRUE,
		     convert_fill_param(Params.polygon_fill),
		     pts,
		     sigmet->num_vertices);
}


/*********************************************************************
 * convert_capstyle_param() - Convert the TDRP capstyle parameter to
 *                            the matching symprod value.
 */

static int convert_capstyle_param(int capstyle)
{
  switch(capstyle)
  {
  case CAPSTYLE_BUTT :
    return(SYMPROD_CAPSTYLE_BUTT);
    
  case CAPSTYLE_NOT_LAST :
    return(SYMPROD_CAPSTYLE_NOT_LAST);
    
  case CAPSTYLE_PROJECTING :
    return(SYMPROD_CAPSTYLE_PROJECTING);

  case CAPSTYLE_ROUND :
    return(SYMPROD_CAPSTYLE_ROUND);
  }
  
  return(SYMPROD_CAPSTYLE_BUTT);
}


/*********************************************************************
 * convert_fill_param() - Convert the TDRP fill parameter to the
 *                        matching symprod value.
 */

static int convert_fill_param(int fill)
{
  switch(fill)
  {
  case FILL_NONE :
    return(SYMPROD_FILL_NONE);
    
  case FILL_STIPPLE10 :
    return(SYMPROD_FILL_STIPPLE10);
    
  case FILL_STIPPLE20 :
    return(SYMPROD_FILL_STIPPLE20);
    
  case FILL_STIPPLE30 :
    return(SYMPROD_FILL_STIPPLE30);
    
  case FILL_STIPPLE40 :
    return(SYMPROD_FILL_STIPPLE40);
    
  case FILL_STIPPLE50 :
    return(SYMPROD_FILL_STIPPLE50);
    
  case FILL_STIPPLE60 :
    return(SYMPROD_FILL_STIPPLE60);
    
  case FILL_STIPPLE70 :
    return(SYMPROD_FILL_STIPPLE70);
    
  case FILL_STIPPLE80 :
    return(SYMPROD_FILL_STIPPLE80);
    
  case FILL_STIPPLE90 :
    return(SYMPROD_FILL_STIPPLE90);
    
  case FILL_SOLID :
    return(SYMPROD_FILL_SOLID);
  }
  
  return(SYMPROD_FILL_NONE);
}


/*********************************************************************
 * convert_joinstyle_param() - Convert the TDRP joinstyle parameter to
 *                             the matching symprod value.
 */

static int convert_joinstyle_param(int joinstyle)
{
  switch(joinstyle)
  {
  case JOINSTYLE_BEVEL :
    return(SYMPROD_JOINSTYLE_BEVEL);
    
  case JOINSTYLE_MITER :
    return(SYMPROD_JOINSTYLE_MITER);
    
  case JOINSTYLE_ROUND :
    return(SYMPROD_JOINSTYLE_ROUND);
  }
  
  return(SYMPROD_JOINSTYLE_BEVEL);
}


/*********************************************************************
 * convert_line_type_param() - Convert the TDRP line type parameter to
 *                             the matching symprod value.
 */

static int convert_line_type_param(int line_type)
{
  switch(line_type)
  {
  case LINETYPE_SOLID :
    return(SYMPROD_LINETYPE_SOLID);
    
  case LINETYPE_DASH :
    return(SYMPROD_LINETYPE_DASH);
    
  case LINETYPE_DOT_DASH :
    return(SYMPROD_LINETYPE_DOT_DASH);
  }
  
  return(SYMPROD_LINETYPE_SOLID);
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
  static SIGMET_spdb_t *sigmet = NULL;
  static int sigmet_alloc = 0;

  symprod_product_t *prod;
  time_t now;
  
  if (Params.debug >= DEBUG_ALL)
  {
    fprintf(stdout, "---- Entering convert_to_symprod\n");
  }
  
  // Initialize returned values

  *symprod_len = 0;
  
  // Copy the SPDB data to the local buffer

  if (sigmet_alloc < spdb_len)
  {
    if (sigmet == NULL)
      sigmet = (SIGMET_spdb_t *)umalloc(spdb_len);
    else
      sigmet = (SIGMET_spdb_t *)urealloc(sigmet,
					 spdb_len);
    
    sigmet_alloc = spdb_len;
  }
  
  memcpy((char *)sigmet, (char *)spdb_data, spdb_len);
  
  // Convert the SPDB data to native format so we can use it.

  SIGMET_spdb_from_BE(sigmet);
  
  if (Params.debug >= DEBUG_ALL)
  {
    SPDB_print_chunk_ref(spdb_hdr,
			 stdout);
    
    SIGMET_print_spdb(stdout, sigmet);
  }
  
  // Create the structure for the internal representation of the
  // product.

  now = time(NULL);
  if ((prod = SYMPRODcreateProduct(now, now,
				   spdb_hdr->valid_time,
				   spdb_hdr->expire_time)) == NULL)
  {
    return(NULL);
  }
  
  // Convert the SPDB data to symprod format

  add_polygon_to_buffer(prod, sigmet);
  
  // Convert the internal representation of the buffer to a flat
  // buffer to be returned to the caller.

  void *return_buffer = SYMPRODproductToBuffer(prod, symprod_len);;

  SYMPRODfreeProduct(prod);
  
  // Check everything

  if (Params.debug >= DEBUG_ALL)
  {
    fprintf(stdout, "*******************\n");
    
    SYMPRODprintProductBuffer(stdout, (char *)return_buffer);
  
    fprintf(stdout, "====================\n");
  }
  
  // Put the product buffer in BE format for transmission

  SYMPRODproductBufferToBE((char *)return_buffer);
  
  if (Params.debug >= DEBUG_ALL)
  {
    fprintf(stdout, "---- Leaving convert_to_symprod\n");
  }
  
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
  // Delete the server object

  if (Spdb_server != NULL)
    delete Spdb_server;
  
  // verify mallocs

  umalloc_map();
  umalloc_verify();

  // exit with code sig

  exit(sig);

}
