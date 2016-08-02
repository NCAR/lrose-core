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
//   $Date: 
//   $Id: mad_spdb2symprod.cc,v 1.8 2016/03/07 18:41:15 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * mad_spdb2symprod.cc: mad_spdb2symprod main program.  This server
 *                      serves out microburst polygons from an SPDB
 *                      database in symprod format so that they can be
 *                      easily displayed on the RAP displays.
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

#include <rsbutil/rshape.h>
#include <symprod/spdb_products.h>
#include <symprod/symprod.h>
#include <tdrp/tdrp.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <SpdbServer/SpdbServer.h>

#include "mad_spdb2symprod_tdrp.h"


/**********************************************************************
 * Forward declarations for static functions.
 */

static void add_label_to_buffer(symprod_product_t *prod,
				rshape_polygon_t *polygon,
				double lat,
				double lon);

static void add_polygon_to_buffer(symprod_product_t *prod,
				  rshape_polygon_t *polygon,
				  double *centroid_lat,
				  double *centroid_lon);

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
static mad_spdb2symprod_tdrp_struct Params;

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

  table = mad_spdb2symprod_tdrp_init(&Params);
  
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
 * add_label_to_buffer() - Add the shape label as a SYMPROD text
 *                         object to the indicated buffer.  The label
 *                         is rendered at the given lat/lon position.
 */

static void add_label_to_buffer(symprod_product_t *prod,
				rshape_polygon_t *polygon,
				double lat, double lon)
{
  char label[1024];

  sprintf(label, "%.1f", polygon->magnitude);

  switch(polygon->prod_type)
  {
  case SPDB_MAD_MICROBURST_DATA_TYPE :
    SYMPRODaddText(prod,
		   Params.mb_label_color, "",
		   lat, lon,
		   0, 0,
		   SYMPROD_TEXT_VERT_ALIGN_CENTER,
		   SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		   strlen(label),
		   Params.label_font,
		   label);
    break;

  case SPDB_MAD_CONVERGENCE_DATA_TYPE :
    SYMPRODaddText(prod,
		   Params.conv_label_color, "",
		   lat, lon,
		   0, 0,
		   SYMPROD_TEXT_VERT_ALIGN_CENTER,
		   SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		   strlen(label),
		   Params.label_font,
		   label);
    break;

  case SPDB_MAD_TURBULENCE_DATA_TYPE :
    SYMPRODaddText(prod,
		   Params.turb_label_color, "",
		   lat, lon,
		   0, 0,
		   SYMPROD_TEXT_VERT_ALIGN_CENTER,
		   SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		   strlen(label),
		   Params.label_font,
		   label);
    break;

  default:
    SYMPRODaddText(prod,
		   Params.mb_label_color, "",
		   lat, lon,
		   0, 0,
		   SYMPROD_TEXT_VERT_ALIGN_CENTER,
		   SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		   strlen(label),
		   Params.label_font,
		   label);
    break;
  }

  return;
}


/*********************************************************************
 * add_polygon_to_buffer() - Adds the MAD shape as a SYMPROD polyline
 *                           object to the indicated buffer.  The
 *                           centroid lat and lon are returned for use
 *                           in rendering the label.
 */

static void add_polygon_to_buffer(symprod_product_t *prod,
				  rshape_polygon_t *polygon,
				  double *centroid_lat,
				  double *centroid_lon)
{
  static symprod_wpt_t *pts = NULL;
  static int pts_alloc = 0;
  
  rshape_xy_t *pt_array;

  double min_lat, max_lat;
  double min_lon, max_lon;

  int pt_size = polygon->npt * sizeof(symprod_wpt_t);
  
  if (pts_alloc < pt_size)
  {
    if (pts == NULL)
      pts = (symprod_wpt_t *)umalloc(pt_size);
    else
      pts = (symprod_wpt_t *)urealloc(pts, pt_size);
    
    pts_alloc = pt_size;
  }
  
  PJGflat_init(polygon->latitude, polygon->longitude, 0.0);
  
  pt_array = (rshape_xy_t *)((char *)polygon + sizeof(rshape_polygon_t));
  
  for (int pt = 0; pt < (int)polygon->npt; pt++)
  {
    double lat, lon;
    
    PJGflat_xy2latlon(pt_array[pt].x / 1000.0,
		      pt_array[pt].y / 1000.0,
		      &lat, &lon);

    pts[pt].lat = lat;
    pts[pt].lon = lon;

    if (pt == 0)
    {
      min_lat = max_lat = lat;
      min_lon = max_lon = lon;
    }
    else
    {
      if (lat < min_lat)
	min_lat = lat;
      if (lat > max_lat)
	max_lat = lat;
      if (lon < min_lon)
	min_lon = lon;
      if (lon > max_lon)
	max_lon = lon;
    }
  }
  
  *centroid_lat = (min_lat + max_lat) / 2.0;
  *centroid_lon = (min_lon + max_lon) / 2.0;

  switch(polygon->prod_type)
  {
  case SPDB_MAD_MICROBURST_DATA_TYPE :
    SYMPRODaddPolyline(prod,
		       Params.mb_polygon_color,
		       convert_line_type_param(Params.mb_polygon_linetype),
		       Params.mb_polygon_line_width,
		       convert_capstyle_param(Params.display_capstyle),
		       convert_joinstyle_param(Params.display_joinstyle),
		       TRUE,
		       convert_fill_param(Params.mb_polygon_fill),
		       pts,
		       polygon->npt);
    break;

  case SPDB_MAD_CONVERGENCE_DATA_TYPE :
    SYMPRODaddPolyline(prod,
		       Params.conv_polygon_color,
		       convert_line_type_param(Params.conv_polygon_linetype),
		       Params.conv_polygon_line_width,
		       convert_capstyle_param(Params.display_capstyle),
		       convert_joinstyle_param(Params.display_joinstyle),
		       TRUE,
		       convert_fill_param(Params.conv_polygon_fill),
		       pts,
		       polygon->npt);
    break;

  case SPDB_MAD_TURBULENCE_DATA_TYPE :
    SYMPRODaddPolyline(prod,
		       Params.turb_polygon_color,
		       convert_line_type_param(Params.turb_polygon_linetype),
		       Params.turb_polygon_line_width,
		       convert_capstyle_param(Params.display_capstyle),
		       convert_joinstyle_param(Params.display_joinstyle),
		       TRUE,
		       convert_fill_param(Params.turb_polygon_fill),
		       pts,
		       polygon->npt);
    break;

  default:
    SYMPRODaddPolyline(prod,
		       Params.mb_polygon_color,
		       convert_line_type_param(Params.mb_polygon_linetype),
		       Params.mb_polygon_line_width,
		       convert_capstyle_param(Params.display_capstyle),
		       convert_joinstyle_param(Params.display_joinstyle),
		       TRUE,
		       convert_fill_param(Params.mb_polygon_fill),
		       pts,
		       polygon->npt);
    break;
  }

  return;
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
  static rshape_polygon_t *polygon = NULL;
  static int polygon_alloc = 0;

  symprod_product_t *prod;
  time_t now;
  
  if (Params.debug >= DEBUG_ALL)
  {
    fprintf(stdout, "---- Entering convert_to_symprod\n");
  }
  
  // Initialize returned values

  *symprod_len = 0;
  
  // Copy the SPDB data to the local buffer

  if (polygon_alloc < spdb_len)
  {
    if (polygon == NULL)
      polygon = (rshape_polygon_t *)umalloc(spdb_len);
    else
      polygon = (rshape_polygon_t *)urealloc(polygon,
					     spdb_len);
    
    polygon_alloc = spdb_len;
  }
  
  memcpy((char *)polygon, (char *)spdb_data, spdb_len);
  
  // Convert the SPDB data to native format so we can use it.

  RSHAPE_polygon_from_be((char *)polygon);
  
  if (Params.debug >= DEBUG_ALL)
  {
    SPDB_print_chunk_ref(spdb_hdr,
			 stdout);
    
    RSHAPE_polygon_print(stdout, polygon);
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

  double centroid_lat, centroid_lon;

  add_polygon_to_buffer(prod, polygon, &centroid_lat, &centroid_lon);
  
  if (Params.render_label)
    add_label_to_buffer(prod, polygon, centroid_lat, centroid_lon);

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
  // delete the server object

  if (Spdb_server != NULL)
    delete Spdb_server;
  
  // verify mallocs

  umalloc_map();
  umalloc_verify();

  // Unregister from the process mapper

  PMU_auto_unregister();
  
  // exit with code sig

  exit(sig);

}
