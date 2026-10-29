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
 * ac_spdb2symprod.cc: ac_spdb2symprod main program.  This server
 *                     serves out aircraft position data from an SPDB
 *                     database in symprod format so that it can be
 *                     easily displayed on the RAP displays.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/param.h>
#include <sys/types.h>

#include <symprod/spdb_products.h>
#include <symprod/symprod.h>
#include <tdrp/tdrp.h>
#include <rapformats/ac_posn.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/pjg.h>

#include <SpdbServer/AcSpdbServer.h>

#include "ac_spdb2symprod_tdrp.h"


/**********************************************************************
 * Forward declarations for static functions.
 */

static void add_alt_text(symprod_product_t *prod,
			 ac_posn_t *ac_posn,
			 char *color);

static void add_dirn_arrow(symprod_product_t *prod,
			   ac_posn_t *ac_posn,
			   char *color);

static void add_icons(symprod_product_t *prod,
		      ac_posn_t *ac_posn,
		      int num_posn,
		      ui08 *icon,
		      int icon_size,
		      char *color);

static void add_label(symprod_product_t *prod,
		      ac_posn_t *ac_posn,
		      char *color,
		      char *font,
		      int vert_align,
		      int horiz_align,
		      int x_offset,
		      int y_offset);

static void add_polyline(symprod_product_t *prod,
			 ac_posn_t *ac_posn,
			 int num_posn,
			 char *color);

static char *alt_color(double altitude,
		       char *default_color);
  
static void *convert_to_symprod(spdb_chunk_ref_t *chunk_hdr_curr,
				int nchunks_before,
				int nchunks_after,
				void *chunk_data_before,
				void *chunk_data_curr,
				void *chunk_data_after,
				int *symprod_len);

static ui08 *create_icon(long *param_icon,
			 int param_icon_len,
			 int *icon_size);

static void handle_sigpipe(int sig);

static void parse_args(int argc, char **argv,
		       int *print_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override);

static void tidy_and_exit(int sig);

/**********************************************************************
 * Global variables.
 */

static AcSpdbServer *Spdb_server;
static ac_spdb2symprod_tdrp_struct Params;
static int Label_vert_align;
static int Label_horiz_align;
static double Prev_icon_lat;
static double Prev_icon_lon;
static char *Prog_name;

/**********************************************************************
 * Main program.
 */

int main(int argc, char **argv)
{
  // basic declarations

  path_parts_t progname_parts;
  char *params_path_name;
  
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

  parse_args(argc, argv,
	     &print_params, &params_path_name, &override);

  // load up parameters

  table = ac_spdb2symprod_tdrp_init(&Params);
  
  if (TDRP_read(params_path_name,
		table, &Params,
		override.list) == FALSE)
  {
    fprintf(stderr, "ERROR - %s\n",
	    Prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_path_name);
    tidy_and_exit(-1);
  }
  
  TDRP_free_override(&override);
  
  if (print_params)
  {
    TDRP_check_is_set(table, &Params);
    TDRP_print_params(table, &Params, Prog_name, TRUE);
    tidy_and_exit(-1);
  }
  
  if (!TDRP_check_is_set(table, &Params))
    tidy_and_exit(-1);
  
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
  
  // Convert the parameter values to the corresponding SYMPROD
  // constants.

  switch (Params.label_vert_align)
  {
  case VERT_ALIGN_TOP:
    Label_vert_align = SYMPROD_TEXT_VERT_ALIGN_TOP;
    break;

  case VERT_ALIGN_CENTER:
    Label_vert_align = SYMPROD_TEXT_VERT_ALIGN_CENTER;
    break;

  case VERT_ALIGN_BOTTOM:
    Label_vert_align = SYMPROD_TEXT_VERT_ALIGN_BOTTOM;
    break;

  } /* endswitch - Params.label_vert_align */
  
  switch (Params.label_horiz_align)
  {
  case HORIZ_ALIGN_LEFT:
    Label_horiz_align = SYMPROD_TEXT_HORIZ_ALIGN_LEFT;
    break;

  case HORIZ_ALIGN_CENTER:
    Label_horiz_align = SYMPROD_TEXT_HORIZ_ALIGN_CENTER;
    break;

  case HORIZ_ALIGN_RIGHT:
    Label_horiz_align = SYMPROD_TEXT_HORIZ_ALIGN_RIGHT;
    break;

  } /* endswitch - Params.label_vert_align */
  
  // Create the server object

  Spdb_server = new AcSpdbServer(Params.port,
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
				 Params.before_secs,
				 Params.after_secs,
				 Params.wait_msecs,
				 Params.realtime_avail,
				 spdb_debug_level);
  
  // Operate as a server

  Spdb_server->operate();
  
  exit(0);
}


/*********************************************************************
 * add_icons() - Add the given group of icons to the object
 *               buffer.  Returns the number of bytes added
 *               to the buffer.
 */

static void add_icons(symprod_product_t *prod,
		      ac_posn_t *ac_posn,
		      int num_posn,
		      ui08 *icon,
		      int icon_size,
		      char *color)

{

  static MEMbuf *mbuf_origins = NULL;
  
  /*
   * initialize mbuf
   */
  
  if (mbuf_origins == NULL)
    mbuf_origins = MEMbufCreate();
  else
    MEMbufReset(mbuf_origins);

  // load up icon origins

  symprod_wpt_t origin;
  
  for (int posn = 0; posn < num_posn; posn++)
  {
    origin.lat = ac_posn[posn].lat;
    origin.lon = ac_posn[posn].lon;

    if (Params.sparse_icons)
    {
      double r, theta;
      PJGLatLon2RTheta(origin.lat, origin.lon,
		       Prev_icon_lat, Prev_icon_lon,
		       &r, &theta);
      if (r >= Params.sparse_icons_min_distance)
      {
	Prev_icon_lat = origin.lat;
	Prev_icon_lon = origin.lon;
	MEMbufAdd(mbuf_origins, &origin, sizeof(origin));
      }
    }
    else
    {
      MEMbufAdd(mbuf_origins, &origin, sizeof(origin));
    }
    
  }

  // load up product

  if (MEMbufLen(mbuf_origins) > 0) {
    SYMPRODaddBitmapIcon(prod, color, icon_size, icon_size,
			 MEMbufLen(mbuf_origins) / sizeof(symprod_wpt_t),
			 (symprod_wpt_t *) MEMbufPtr(mbuf_origins),
			 icon);
  }

}

/*********************************************************************
 * add_label() - Add the track label to the object buffer.
 */

static void add_label(symprod_product_t *prod,
		      ac_posn_t *ac_posn,
		      char *color,
		      char *font,
		      int vert_align,
		      int horiz_align,
		      int x_offset,
		      int y_offset)

{
  SYMPRODaddText(prod, color, "",
		 ac_posn->lat, ac_posn->lon,
		 x_offset, y_offset,
		 vert_align, horiz_align,
		 0, font, ac_posn->callsign);
}

/*********************************************************************
 * add_alt_text()
 *
 * Add the altitude text centered on the aircraft position to the
 * object buffer.
 */

static void add_alt_text(symprod_product_t *prod,
			 ac_posn_t *ac_posn,
			 char *color)
  
{

  // if icons are to be sparse, check min icon distance.
  // return early if distance from prev is less than the minimum.

  if (Params.sparse_icons) {
    double r, theta;
    PJGLatLon2RTheta(ac_posn->lat, ac_posn->lon,
		     Prev_icon_lat, Prev_icon_lon,
		     &r, &theta);
    if (r < Params.sparse_icons_min_distance) {
      return;
    }
  }
  Prev_icon_lat = ac_posn->lat;
  Prev_icon_lon = ac_posn->lon;

  // create the string

  static char alt_str[128];
  sprintf(alt_str, "%g", ac_posn->alt);
  
  // add text

  SYMPRODaddText(prod, color, "",
		 ac_posn->lat, ac_posn->lon, 0, 0,
		 SYMPROD_TEXT_VERT_ALIGN_CENTER,
		 SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		 0, Params.altitude_font,
		 alt_str);

}

/*********************************************************************
 * add_polyline() - Add the polyline defined by the given
 *                            aircraft positions to the object buffer.
 */

static void add_polyline(symprod_product_t *prod,
			 ac_posn_t *ac_posn,
			 int num_posn,
			 char *color)

{
  static MEMbuf *mbuf_points = NULL;
  
  if (Params.debug >= DEBUG_ALL)
    fprintf(stderr, "Entering add_polyline\n");
  
  /*
   * initialize mbuf
   */
  
  if (mbuf_points == NULL)
    mbuf_points = MEMbufCreate();
  else
    MEMbufReset(mbuf_points);

  // load up polyline

  symprod_wpt_t vertex;
  
  for (int posn = 0; posn < num_posn; posn++)
  {
    vertex.lat = ac_posn[posn].lat;
    vertex.lon = ac_posn[posn].lon;
    MEMbufAdd(mbuf_points, &vertex, sizeof(vertex));
  }

  // load up product

  SYMPRODaddPolyline(prod, color,
		     SYMPROD_LINETYPE_SOLID,
		     Params.polyline_width,
		     SYMPROD_CAPSTYLE_BUTT,
		     SYMPROD_JOINSTYLE_BEVEL,
		     FALSE, FALSE,
		     (symprod_wpt_t *) MEMbufPtr(mbuf_points),
		     MEMbufLen(mbuf_points) / sizeof(symprod_wpt_t));

  if (Params.debug >= DEBUG_ALL)
    fprintf(stderr, "Leaving add_polyline\n");
  
}

/*********************************************************************
 * add_dirn_arrow() - Add the dirn arrow for the given
 *                    aircraft position to the object buffer.
 */

static void add_dirn_arrow(symprod_product_t *prod,
			   ac_posn_t *ac_posn,
			   char *color)

{

  SYMPRODaddArrowBothPts(prod, color,
			 SYMPROD_LINETYPE_SOLID,
			 Params.polyline_width,
			 SYMPROD_CAPSTYLE_BUTT,
			 SYMPROD_JOINSTYLE_BEVEL,
			 ac_posn[-1].lat, ac_posn[-1].lon,
			 ac_posn[0].lat, ac_posn[0].lon,
			 Params.dirn_arrow_head_len, 30.0);

}

/*********************************************************************
 * convert_to_symprod() - Convert the data from the SPDB database to
 *                        symprod format.
 */

static void *convert_to_symprod(spdb_chunk_ref_t *chunk_hdr_curr,
				int nchunks_before,
				int nchunks_after,
				void *chunk_data_before,
				void *chunk_data_curr,
				void *chunk_data_after,
				int *symprod_len)

{
  static char *routine_name = "convert_to_symprod()";
  
  static ui08 *before_icon = NULL;
  static int before_icon_size = 0;
  
  static ui08 *current_icon = NULL;
  static int current_icon_size = 0;
  
  static ui08 *after_icon = NULL;
  static int after_icon_size = 0;
  
  static ac_posn_t *ac_posn = NULL;
  static int n_posn_alloc = 0;
  
  // Initialize
  *symprod_len = 0;
  Prev_icon_lat = 0.0;
  Prev_icon_lon = 0.0;
  
  // Initialize any icons we may need to use.

  if (Params.display_before_icon)
    before_icon = create_icon(Params.before_icon.val,
			      Params.before_icon.len,
			      &before_icon_size);
  
  if (Params.display_current_icon)
    current_icon = create_icon(Params.current_icon.val,
			       Params.current_icon.len,
			       &current_icon_size);
  
  if (Params.display_after_icon)
    after_icon = create_icon(Params.after_icon.val,
			     Params.after_icon.len,
			     &after_icon_size);
  
  // alloc space for local buffer

  int n_posn = nchunks_before + nchunks_after + 1;
  if (n_posn_alloc < n_posn)
  {
    if (ac_posn == NULL)
      ac_posn = (ac_posn_t *) umalloc(n_posn * sizeof(ac_posn_t));
    else
      ac_posn = (ac_posn_t *)urealloc(ac_posn,
				      n_posn * sizeof(ac_posn_t));
    n_posn_alloc = n_posn;
  }
  
  // Copy the SPDB data to local buffer and byte swap
  
  memcpy(ac_posn, chunk_data_before,
	 nchunks_before * sizeof(ac_posn_t));
  memcpy(ac_posn + nchunks_before, chunk_data_curr,
	 sizeof(ac_posn_t));
  memcpy(ac_posn + nchunks_before + 1, chunk_data_after,
	 nchunks_after * sizeof(ac_posn_t));

  if (Params.debug >= DEBUG_ALL)
    fprintf(stderr,
	    "Received data for %d positions (%d before, %d after):\n",
	    n_posn, nchunks_before, nchunks_after);
  
  for (int i = 0; i < n_posn; i++)
  {
    BE_to_ac_posn(ac_posn + i);
    
    if (Params.debug >= DEBUG_ALL)
    {
      ac_posn_print(stderr, "--- ", ac_posn + i);
      fprintf(stderr, "\n");
    }
    
  }
  
  // See if we are skipping GA aircraft

  if (!Params.display_ga)
  {
    if (isdigit(ac_posn[nchunks_before].callsign[0]) ||
	isdigit(ac_posn[nchunks_before].callsign[1]) ||
	isdigit(ac_posn[nchunks_before].callsign[2]))
    {
      if (Params.debug >= DEBUG_MSGS)
	fprintf(stderr, "--->  Skipping GA aircraft: %s\n",
		ac_posn[nchunks_before].callsign);
      
      return(NULL);
    }
  }
  
  // create struct for internal representation of product

  symprod_product_t *prod;
  time_t now = time(NULL);
  if ((prod = SYMPRODcreateProduct(now, now,
				   chunk_hdr_curr->valid_time,
				   chunk_hdr_curr->expire_time)) == NULL)
  {
    if (Params.debug >= DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR: %s::%s\n", Prog_name, routine_name);
      fprintf(stderr, "Error creating SPDB product object\n");
    }
    
    return (NULL);
  }

  // Add the callsign as the product label so it will appear in the
  // prod_sel window.

  SYMPRODaddLabel(prod, ac_posn[nchunks_before].callsign);
  
  if (Params.color_by_altitude)
  {
    // Add polylines for track colored by altitude

    for (int i = 1; i < n_posn; i++)
    {
      double alt = (ac_posn[i].alt + ac_posn[i-1].alt) / 2.0;
      char *color = alt_color(alt, Params.before_icon_color);
      add_polyline(prod, ac_posn + i - 1, 2, color);
    }
  }
  else
  {
    // add polyline for track before current pos

    if (Params.display_before_polyline && nchunks_before > 0)
    {
      add_polyline(prod, ac_posn,
		   nchunks_before + 1,
		   Params.before_polyline_color);
    }
    
    // add polyline for track after current pos
    
    if (Params.display_after_polyline && nchunks_after > 0)
    {
      add_polyline(prod, ac_posn + nchunks_before,
		   nchunks_after + 1,
		   Params.after_polyline_color);
    }

  } // if (Params.color_by_altitude) 
  
  // Add icons for track before the current position

  if (before_icon != NULL && nchunks_before > 0)
  {
    if (Params.color_by_altitude) {
      for (int i = 0; i < nchunks_before; i++) {
	char *color = alt_color(ac_posn[i].alt,
				Params.before_icon_color);
	if (Params.plot_altitude_text) {
	  add_alt_text(prod, ac_posn + i, color);
	} else {
	  add_icons(prod, ac_posn + i, 1,
			      before_icon, before_icon_size, color);
	}
      } // i
    } else {
      add_icons(prod, ac_posn, nchunks_before,
			  before_icon, before_icon_size,
			  Params.before_icon_color);
    }
  }
  
  // Add icons for track at the current position
  
  if (current_icon != NULL)
  {
    char *color;
    if (Params.color_by_altitude) {
      color = alt_color(ac_posn[nchunks_before].alt,
			Params.current_icon_color);
    } else {
      color = Params.current_icon_color;
    }
    if (Params.plot_altitude_text) {
      add_alt_text(prod, ac_posn + nchunks_before, color);
    } else {
      add_icons(prod, ac_posn + nchunks_before, 1,
		current_icon, current_icon_size, color);
    }
  }
  
  // Add icons for track after the current position
  
  if (after_icon != NULL && nchunks_after > 0)
  {
    int start = nchunks_before + 1;
    if (Params.color_by_altitude) {
      for (int i = 0; i < nchunks_after; i++) {
	char *color = alt_color(ac_posn[start + i].alt,
				Params.after_icon_color);
	if (Params.plot_altitude_text) {
	  add_alt_text(prod,
		       ac_posn + start + i, color);
	} else {
	  add_icons(prod,
		    ac_posn + start + i, 1,
		    after_icon, after_icon_size, color);
	}
      } // i
    } else {
      add_icons(prod, ac_posn + start, nchunks_after,
		after_icon, after_icon_size,
		Params.after_icon_color);
    } // if (Params.color_by_altitude)
  }

  // add label

  if (Params.display_label)
  {
    add_label(prod,
	      ac_posn + nchunks_before,
	      Params.label_color,
	      Params.label_font,
	      Label_vert_align,
	      Label_horiz_align,
	      Params.label_offset.x_offset,
	      Params.label_offset.y_offset);
  }

  // add arrow if required

  if (Params.plot_dirn_arrow) {

    if (Params.display_after_polyline) {
      add_dirn_arrow(prod, ac_posn + nchunks_before + nchunks_after,
		     Params.after_polyline_color);
    } else if (Params.display_before_polyline) {
      add_dirn_arrow(prod, ac_posn + nchunks_before,
		     Params.before_polyline_color);
    }

  }
  
  // copy internal representation of product to output buffer

  void *return_buffer = SYMPRODproductToBuffer(prod, symprod_len);

  if (Params.debug >= DEBUG_ALL) {
    SYMPRODprintProductBuffer(stderr, (char *) return_buffer);
  }

  // Put the product buffer in BE format for transmission
  
  SYMPRODproductBufferToBE((char *) return_buffer);

  // free up internal representation of product

  SYMPRODfreeProduct(prod);
  
  return(return_buffer);

}

/*********************************************************************
 * create_icon() - Create an icon array for use in the SYMPROD object
 *                 from the icon array as specified in the parameter
 *                 file.
 */

static ui08 *create_icon(long *param_icon,
			 int param_icon_len,
			 int *icon_size_p)
{
  ui08 *icon = NULL;
  int i;
  
  // Initialize return values

  *icon_size_p = 0;

  // Make sure the icon is long enough
  // required to be 16 x 16.

  if (param_icon_len < 1)
  {
    fprintf(stderr, "ERROR: ac_spdb2symprod::create_icon\n");
    fprintf(stderr,
	    "Invalid icon size in parameter file.\n"
	    "Icon should be at least 1 bit long.\n");
    return(NULL);
  }
  
  // compute the icon size - this is the sqrt of the length

  int icon_size = (int) (sqrt((double) param_icon_len));

  // Allocate space for the returned icon
  
  int icon_nbytes = icon_size * icon_size;
  
  icon = (ui08 *)umalloc(icon_nbytes);
  
  // Update the icon values
  
  for (i = 0; i < icon_nbytes; i++)
  {
    if (param_icon[i])
      icon[i] = 1;
    else
      icon[i] = 0;
  }

  *icon_size_p = icon_size;
  return(icon);
}

/******************************************
 * alt_color()
 *
 * Returns the color for a given altitude. If
 * there is no relevant color the default is
 * returned.
 */

static char *alt_color(double altitude,
		       char *default_color)
  
{

  for (int i = 0; i < Params.altitude_color_scale.len; i++) {

    if (Params.altitude_color_scale.val[i].min_val <= altitude &&
	Params.altitude_color_scale.val[i].max_val >= altitude) {

      return(Params.altitude_color_scale.val[i].color);
    }

  } // i

  return (default_color);

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

  // Close the AcSpdbServer sockets
  Spdb_server->handleSigpipe();
  
  return;
}


/*******************************************************************
 * parse_args() - Parse the command line arguments.
 */

static void parse_args(int argc, char **argv,
		       int *print_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override)
{
  int error_flag = 0;
  int warning_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];
  
  // Initialize the returned values

  *print_params_p = FALSE;
  *params_file_path_p = NULL;
  
  // load up usage string

  sprintf(usage, "%s%s%s",
	  "Usage:\n\n", Prog_name, " [options] as below:\n\n"
	  "       [ --, -help, -man] produce this list.\n"
	  "       [ -print_params] print TDRP parameter values\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -mdebug level] set malloc debug level\n"
	  "       [ -params name] parameters file name\n"
	  "       [ -data data_dir] input data directory\n"
	  "       [ -port port] output port number\n"
	  "\n");

  // initialize

  *print_params_p = FALSE;
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


/*********************************************************************
 * tidy_and_exit() - Clean up any memory, etc. and exit the program.
 */

void tidy_and_exit(int sig)
{
  // delete the AcSpdbServer object

  delete Spdb_server;
  
  // verify mallocs

  umalloc_map();
  umalloc_verify();

  // exit with code sig

  exit(sig);

}
