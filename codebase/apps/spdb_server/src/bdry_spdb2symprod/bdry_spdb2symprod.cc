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
//   $Id: bdry_spdb2symprod.cc,v 1.20 2016/03/07 18:41:14 dixon Exp $
//   $Revision: 1.20 $
//   $State: Exp $

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * bdry_spdb2symprod.cc: bdry_spdb2symprod main program.  This server
 *                       serves out boundary data from an SPDB database
 *                       in symprod format so that it can be easily
 *                       displayed on the RAP displays.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1997
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

#include <rapformats/bdry.h>
#include <rapformats/bdry_extrap.h>
#include <rapmath/math_macros.h>
#include <symprod/spdb_products.h>
#include <symprod/symprod.h>
#include <tdrp/tdrp.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>


#include <SpdbServer/SpdbServer.h>

#include "bdry_spdb2symprod_tdrp.h"


/**********************************************************************
 * Forward declarations for static functions.
 */

static int convert_capstyle_param(int capstyle);

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
static bdry_spdb2symprod_tdrp_struct Params;

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

  table = bdry_spdb2symprod_tdrp_init(&Params);
  
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
  static BDRY_spdb_product_t *bdry = NULL;
  static int bdry_alloc = 0;
  
  static symprod_wpt_t *points = NULL;
  static int points_alloc = 0;
  
  char *return_buffer;
  
  // Initialize returned values

  *symprod_len = 0;
  
  // Copy the SPDB data to the local buffer

  if (bdry_alloc < spdb_len)
  {
    if (bdry == NULL)
      bdry = (BDRY_spdb_product_t *)umalloc(spdb_len);
    else
      bdry = (BDRY_spdb_product_t *)urealloc(bdry,
					     spdb_len);
    
    bdry_alloc = spdb_len;
  }
  
  memcpy((char *)bdry, (char *)spdb_data, spdb_len);
  
  // Convert the SPDB data to native format so we can use it.

  BDRY_spdb_product_from_BE(bdry);
  
  // Now put it in product format so it's a little easier to access the
  // pieces

  BDRY_product_t *shape = BDRY_spdb_to_product(bdry);
  
  if (Params.debug >= DEBUG_ALL)
  {
    SPDB_print_chunk_ref(spdb_hdr,
			 stdout);
    
    BDRY_print_spdb_product(stdout, bdry, FALSE);

  }
  
  // Shift the location of the boundary vertices, if requested

  if (Params.latitude_shift != 0.0 ||
      Params.longitude_shift != 0.0)
  {
    for (int obj = 0; obj < shape->num_polylines; obj++)
    {
      for (int pt = 0; pt < shape->polylines[obj].num_pts; pt++)
      {
	shape->polylines[obj].points[pt].lat += Params.latitude_shift;
	shape->polylines[obj].points[pt].lon += Params.longitude_shift;
      } /* endfor - pt */
    } /* endfor - obj */
    
  }
  
  // Convert the SPDB data to symprod format

  time_t current_time = time((time_t *)NULL);
  
  symprod_product_t *symprod_prod =
    SYMPRODcreateProduct(current_time,
			 current_time,
			 shape->data_time,
			 shape->expire_time);
  
  bool shape_added = false;
  
  for (int obj = 0; obj < shape->num_polylines; obj++)
  {
    double map_direction;
    int is_detection;
    
    // We never display the prediction lines.  These are just
    // used by colide.

    if (shape->type == BDRY_TYPE_PREDICT_COLIDE)
      continue;
    
    // Convert the database direction to the direction needed by
    // the SPDB routines.

    map_direction = BDRY_spdb_to_pjg_direction(shape->motion_direction);

    if (points_alloc < shape->polylines[obj].num_pts)
    {
      points_alloc = shape->polylines[obj].num_pts;
      
      if (points == (symprod_wpt_t *)NULL)
	points = (symprod_wpt_t *)umalloc(points_alloc *
					  sizeof(symprod_wpt_t));
      else
	points = (symprod_wpt_t *)urealloc(points,
					   points_alloc *
					   sizeof(symprod_wpt_t));
    }
    
    for (int pt = 0; pt < shape->polylines[obj].num_pts; pt++)
    {
      points[pt].lat = shape->polylines[obj].points[pt].lat;
      points[pt].lon = shape->polylines[obj].points[pt].lon;
    }
    
    // Add the polyline to the product buffer

    char *color;
    
    if (shape->type == BDRY_TYPE_BDRY_TRUTH ||
	shape->type == BDRY_TYPE_BDRY_MIGFA ||
	shape->type == BDRY_TYPE_BDRY_COLIDE ||
	shape->type == BDRY_TYPE_COMB_NC_ISSUE ||
	shape->type == BDRY_TYPE_COMB_NC_VALID ||
	shape->type == BDRY_TYPE_FIRST_GUESS_ISSUE ||
	shape->type == BDRY_TYPE_FIRST_GUESS_VALID ||
	shape->type == BDRY_TYPE_MINMAX_NC_ISSUE ||
	shape->type == BDRY_TYPE_MINMAX_NC_VALID)
    {
      color = Params.detection_color;
      is_detection = TRUE;
    }
    else
    {
      // If we are calculating extrapolations, we don't want to
      // display this boundary.

      if (Params.calc_extrapolations)
	continue;
      
      color = Params.extrapolation_color;
      is_detection = FALSE;
    }

    if ((is_detection && Params.display_detections) ||
	(!is_detection && Params.display_extrapolations))
    {
      SYMPRODaddPolyline(symprod_prod,
			 color,
			 convert_line_type_param(Params.display_line_type),
			 Params.display_line_width,
			 convert_capstyle_param(Params.display_capstyle),
			 convert_joinstyle_param(Params.display_joinstyle),
			 FALSE,
			 SYMPROD_FILL_NONE,
			 points,
			 shape->polylines[obj].num_pts);

      shape_added = true;
      
      // Add the optional label to the product buffer

      if (Params.display_label)
      {
	const char *label;
      
	switch (Params.label_source)
	{
	case LABEL_DESCRIP :
	  label = shape->desc;
	  break;
    
	case LABEL_POLYLINE :
	  label = shape->polylines[obj].object_label;
	  break;
	} /* endswitch - Params.label_source */
  
	SYMPRODaddLabel(symprod_prod,
			label);
      
	SYMPRODaddText(symprod_prod,
		       color, "",
		       shape->polylines[obj].points[0].lat,
		       shape->polylines[obj].points[0].lon,
		       0, 0,
		       SYMPROD_TEXT_VERT_ALIGN_BOTTOM,
		       SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		       0,
		       Params.label_font,
		       label);
      
	shape_added = true;
	
      } /* endif - Params.display_label */
    }
    
    // Add the optional motion vector to the product buffer

    if (Params.display_vector &&
	is_detection &&
	shape->motion_speed > 0.0)
    {
      int center_pt = shape->polylines[obj].num_pts / 2;
      double vector_length =
	(shape->motion_speed * Params.extrap_secs) / 3600.0;
      
      SYMPRODaddArrowStartPt(symprod_prod,
			     Params.vector_color,
			     convert_line_type_param(Params.display_line_type),
			     Params.display_line_width,
			     convert_capstyle_param(Params.display_capstyle),
			     convert_joinstyle_param(Params.display_joinstyle),
			     shape->polylines[obj].points[center_pt].lat,
			     shape->polylines[obj].points[center_pt].lon,
			     vector_length,
			     map_direction,
			     Params.head_length,
			     Params.head_half_angle);
      
      shape_added = true;
      
    } /* endif - Params.display_vector */
    
    // Extrapolate this boundary, if requested

    if (Params.display_extrapolations &&
	Params.calc_extrapolations &&
	shape->motion_speed > 0.0)
    {
      if (Params.debug >= DEBUG_MSGS)
	fprintf(stdout,
		"*** Extrapolating %s -- direction = %f, speed = %f\n",
		shape->desc, shape->motion_direction, shape->motion_speed);
      
      if (Params.point_extrapolations)
	 BDRY_extrap_pt_motion(shape, 1, Params.extrap_secs);
      else
         BDRY_extrapolate(shape, 1, Params.extrap_secs);
      
      for (int pt = 0; pt < shape->polylines[obj].num_pts; pt++)
      {
	points[pt].lat = shape->polylines[obj].points[pt].lat;
	points[pt].lon = shape->polylines[obj].points[pt].lon;
      }
    
      SYMPRODaddPolyline(symprod_prod,
			 Params.extrapolation_color,
			 convert_line_type_param(Params.display_line_type),
			 Params.display_line_width,
			 convert_capstyle_param(Params.display_capstyle),
			 convert_joinstyle_param(Params.display_joinstyle),
			 FALSE,
			 SYMPROD_FILL_NONE,
			 points,
			 shape->polylines[obj].num_pts);
    
      shape_added = true;
      
      // Label the extrapolation

      if (Params.display_label)
      {
	char ext_label[BUFSIZ];
      
	switch (Params.label_source)
	{
	case LABEL_DESCRIP :
	  sprintf(ext_label, "%d+%d",
		  shape->bdry_id,
		  (int)(Params.extrap_secs / 60));
	  break;
    
	case LABEL_POLYLINE :
	  sprintf(ext_label, "Boundary_extrap");
	  break;
	} /* endswitch - Params.label_source */
  
	SYMPRODaddLabel(symprod_prod,
			ext_label);
      
	SYMPRODaddText(symprod_prod,
		       Params.extrapolation_color, "",
		       shape->polylines[obj].points[0].lat,
		       shape->polylines[obj].points[0].lon,
		       0, 0,
		       SYMPROD_TEXT_VERT_ALIGN_BOTTOM,
		       SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		       0,
		       Params.label_font,
		       ext_label);
      
	shape_added = true;
	
      } /* endif - Params.display_label */
    
    }

  } /* endfor - obj */
  

  // Free the shape object since it's no longer needed

  umalloc_verify();
  BDRY_free_product(shape);
  umalloc_verify();
  
  // Check for shapes in the buffer.  If no shapes were added
  // to the buffer, we want to return NULL.

  if (!shape_added)
  {
    SYMPRODfreeProduct(symprod_prod);

    *symprod_len = 0;
    return((void *)NULL);
  }
  
  // Create the return buffer

  return_buffer = SYMPRODproductToBuffer(symprod_prod,
					 symprod_len);

  SYMPRODfreeProduct(symprod_prod);
  
  // Check everything

  if (Params.debug >= DEBUG_ALL)
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
