/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*********************************************************************
 * spr.c
 *
 * Symbolic Product Render routines
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * April 1997
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include <mdv/mdv_macros.h>
#include <rapmath/math_macros.h>
#include <rapplot/gplot.h>
#include "spr_98.h"
#include <symprod98/prod_queue.h>
#include <symprod98/symprod.h>
#include <symprod98/spdb_products.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/str.h>

/*
 * Defines
 */

#define ARRAY_ALLOC_INCR   20

/*
 * Arrays for stippling
 */

#define stipple_bitmap_width  10
#define stipple_bitmap_height 10

static char stipple10_bitmap_bits[] =
{  0x82, 0x00, 0x04, 0x10, 0x40,
   0x00, 0x01, 0x10, 0x02, 0x04,
   0x20, 0x00, 0x00 
};

static char stipple20_bitmap_bits[] =
{  0x92, 0x14, 0x04, 0x90, 0x41,
   0x40, 0x81, 0x12, 0x02, 0x04,
   0x20, 0x42, 0x00 
};

static char stipple30_bitmap_bits[] =
{  0x92, 0x15, 0x14, 0x98, 0x41,
   0x42, 0x85, 0x52, 0x82, 0x44,
   0x24, 0x42, 0x20 
};

static char stipple40_bitmap_bits[] =
{  0x92, 0x95, 0x54, 0x9a, 0x49,
   0x4a, 0xa5, 0x52, 0xa2, 0x45,
   0x24, 0x62, 0xa0 
};

static char stipple50_bitmap_bits[] =
{  0xba, 0x95, 0x54, 0xda, 0xc9,
   0xca, 0xa5, 0x56, 0xaa, 0x4d,
   0x64, 0x66, 0xa0 
};

static char stipple60_bitmap_bits[] =
{  0xba, 0xd5, 0x5d, 0xda, 0xcd,
   0xda, 0xf5, 0x56, 0xaa, 0x5f,
   0x64, 0x6e, 0xa0 
};

static char stipple70_bitmap_bits[] =
{  0xbb, 0xf5, 0x5f, 0xde, 0xbb,
   0xba, 0xf5, 0x77, 0xab, 0x5f,
   0x65, 0x6e, 0xb0 
};

static char stipple80_bitmap_bits[] =
{  0xfb, 0xf7, 0x7f, 0xde, 0xdd,
   0xdb, 0xff, 0x77, 0xeb, 0x5f,
   0x6d, 0x7f, 0xb0 
};

static char stipple90_bitmap_bits[] =
{  0xff, 0xf7, 0x7f, 0xff, 0xff,
   0xfb, 0xff, 0x7f, 0xeb, 0x7f,
   0x6f, 0xff, 0xb0 
};


/*
 * Static variables used for processing.  These values are set by
 * SPR_init(), which must be called by the user before any other
 * SPR routines are called.
 */

static int Spr_initialized = FALSE;

static int Projection;
static double Origin_lat = 0.0;
static double Origin_lon = 0.0;;

static x_color_list_index_t List_index = {NULL,0};
static unsigned long Background_pixel = 0;
static int Background_pixel_init = FALSE;

#ifdef CONSERVE_X_CALLS
static char Last_foreground_color[SYMPROD_COLOR_LEN];
static char Last_background_color[SYMPROD_COLOR_LEN];
#endif

/*
 * Prototypes for static functions
 */

static int convert_capstyle_to_x(int capstyle);

static int convert_joinstyle_to_x(int joinstyle);

static void convert_lat_lon_to_world(double lat,
				     double lon,
				     gframe_t *frame,
				     double *world_x,
				     double *world_y);

static int convert_linetype_to_x(int linetype);

static Pixmap *convert_stipple_to_x(Display *display,
				    int filltype);

static int create_included_stipple(Display *display,
				   char *bitmap,
				   int bitmap_width,
				   int bitmap_height,
				   Pixmap *stipple,
				   unsigned int *width,
				   unsigned int *height);

 
/*********************************************************************
 * SPR_draw_chunk_station_array()
 *
 * Supervises the rendering of the given station report array chunk.
 *
 *********************************************************************/

void SPR_draw_chunk_station_array(int dev,
			    gframe_t *frame,
			    Display *display,
			    GC gc,
			    psgc_t *psgc,
			    void *chunk)
{
    int i;
    int n_stations;
    station_report_t *rep_ptr;
    station_report_array_header_t *sra_ptr;

    /* Pick up pointer to the chunk elements */
    sra_ptr = (station_report_array_header_t *) chunk;
    rep_ptr = chunk + sizeof(station_report_array_header_t);

    /* Convert this chunk to native format */
    station_report_array_from_be(sra_ptr,rep_ptr);

    n_stations = sra_ptr->num_reports;

    for(i=0; i < n_stations; i++, rep_ptr++)
    {
      SPR_draw_chunk_station(dev,
                             frame,
                             display,
                             gc,
                             psgc,
                             rep_ptr);
    }
}

/*********************************************************************
 * SPR_draw_chunk_station()
 *
 * Renders the given station report structure.
 *
 *********************************************************************/

#define BLANK_PIXEL_SPACING 5
#define STRING_LEN 128


void SPR_draw_chunk_station(int dev,
			    gframe_t *frame,
			    Display *display,
			    GC gc,
			    psgc_t *psgc,
			    station_report_t *station)
{
  static int barb_shaft_len = 33;

  int xoffset, yoffset;
  int len, direct, ascent, descent;
  double world_x ,world_y;
  XCharStruct overall;
  char string[STRING_LEN];
  double u_wind, v_wind;
  
  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }

  /*
   * Draw the report
   */

  /*
   * Find world/projection coordinates
   */

  convert_lat_lon_to_world(station->lat, station->lon, frame,
			   &world_x, &world_y);
  
  memset(string,0,STRING_LEN);/* make sure text string is null terminated */
   
  /*
   * Determine the U & V componets of the wind & PLOT A BARB
   */

  if (station->winddir != STATION_NAN &&
      station->windspd != STATION_NAN)
  {
    u_wind = -sin(station->winddir * RAD_PER_DEG) *
      station->windspd * NMH_PER_MS;
    if(u_wind < 0.1 && u_wind > -0.1) u_wind = 0.0;

    v_wind = -cos(station->winddir * RAD_PER_DEG) *
      station->windspd * NMH_PER_MS;
    if(v_wind < 0.1 && v_wind > -0.1) v_wind = 0.0;
 
    GDrawWindBarb(dev, frame, gc, frame->psgc,
		  world_x, world_y,
		  u_wind, v_wind, barb_shaft_len);
  }

  
  /*
   * Plot TEMPERATURE as text - Upper Left
   */

  if (station->temp != STATION_NAN)
  {
    sprintf(string, "%.0f", station->temp);
    len = strlen(string);
    XTextExtents(frame->x->font, string, len,
		 &direct, &ascent, &descent, &overall);
  
    xoffset =  -(overall.width) - BLANK_PIXEL_SPACING; 
    yoffset =  -overall.ascent  - overall.descent - BLANK_PIXEL_SPACING;

    GDrawImageStringOffset(dev, frame, gc,
			   frame->x->font, frame->psgc,
			   XJ_LEFT, YJ_BELOW,
			   world_x, world_y,
			   xoffset, -yoffset,
			   string);
    }

   string[0] = '\0';

  /*
   * Plot Weather Type indicators as text - Middle Left
   */

   if (station->msg_id == METAR_REPORT) {

     char *space_pos;
     
     /*
      * copy in the metar weather, and truncate the string after the
      * first token, i.e. at the first space
      */
     
     STRncopy(string, station->shared.metar.weather_str, 128);
     space_pos = strchr(string, ' ');
     if (space_pos != NULL) {
       *space_pos = '\0';
     }
     
   } else {

     ui32 wtype = station->weather_type;

    /* 
     * place these in order of priority - Last = highest
     * Priorities set for Aircraft Indutstry
     */
     
     if(wtype & WT_FG) strcpy(string,"FG"); /*  Fog */
     if(wtype & WT_HZ) strcpy(string,"HZ"); /* Haze */
     if(wtype & WT_UP) strcpy(string,"UP"); /* Undet Precip */
     if(wtype & WT_DZ) strcpy(string,"DZ"); /* Drizzle */
     if(wtype & WT_DS) strcpy(string,"DS"); /* Dust Storm */
     if(wtype & WT_MRA) strcpy(string,"-RA"); /* Light Rain */
     if(wtype & WT_RA) strcpy(string,"RA"); /* Rain */
     if(wtype & WT_TS) strcpy(string,"TS"); /* Thunder Storms */
     if(wtype & WT_MSN) strcpy(string,"-SN"); /* Light Snow */
     if(wtype & WT_SG) strcpy(string,"SG"); /* Light Snow */
     if(wtype & WT_BLSN) strcpy(string,"BLSN"); /* Blowing Snow */
     if(wtype & WT_FZFG) strcpy(string,"FZFG"); /* Freezing Fog */
     if(wtype & WT_SQ) strcpy(string,"SQ"); /* Squall */
     if(wtype & WT_MFZRA) strcpy(string,"-FZRA"); /* Freezing Rain */
     if(wtype & WT_SN) strcpy(string,"SN"); /* Snow */
     if(wtype & WT_PRA) strcpy(string,"+RA"); /* Heavy Rain */
     if(wtype & WT_PTS) strcpy(string,"+TS"); /* Heavy Thunderstorms */
     if(wtype & WT_FZDZ) strcpy(string,"FZDZ"); /* Freezing Drizzle */
     if(wtype & WT_PSN) strcpy(string,"+SN"); /*  Heavy Snow */
     if(wtype & WT_PE) strcpy(string,"PE"); /* Ice Pellets */
     if(wtype & WT_GS) strcpy(string,"GS"); /* Small Hail */
     if(wtype & WT_GR) strcpy(string,"GR"); /* Hail */
     if(wtype & WT_FZRA) strcpy(string,"FZRA"); /* Freezing Rain */
     if(wtype & WT_PFZRA) strcpy(string,"+FZRA"); /* Freezing Rain */
     if(wtype & WT_FC) strcpy(string,"FC"); /* Funnel Cloud */

   }
     
     
   len = strlen(string);
   if( len > 1) {
     XTextExtents(frame->x->font, string, len,
		  &direct, &ascent, &descent, &overall);
     xoffset =  -(overall.width) - BLANK_PIXEL_SPACING; 
     yoffset =  - overall.descent;
     GDrawImageStringOffset(dev, frame, gc,
			    frame->x->font, frame->psgc,
			    XJ_LEFT, YJ_BELOW,
			    world_x, world_y,
			    xoffset, -yoffset,
			    string);
   }
   
  /*
   * Plot DEWPOINT as text - Lower Left
   */

  if(station->dew_point != STATION_NAN)
  {
    sprintf(string, "%.0f", station->dew_point);
    len = strlen(string);
    XTextExtents(frame->x->font ,string, len,
		 &direct, &ascent, &descent, &overall);
  
    xoffset =  -(overall.width) - BLANK_PIXEL_SPACING; 
    yoffset =  overall.ascent  +  BLANK_PIXEL_SPACING;

    GDrawImageStringOffset(dev, frame, gc,
			   frame->x->font, frame->psgc,
			   XJ_LEFT, YJ_BELOW,
			   world_x, world_y,
			   xoffset, -yoffset,
			   string);
  }

  /*
   * Plot Site ID as text - Upper Right
   */
   if(strlen(station->station_label) > 1) 
   {

    /* Only display the last part of the label if it starts with 'K' */
    /* Don't ask :) */
    if(station->station_label[0] != 'K') {
        STRcopy(string,station->station_label,STRING_LEN);
    } else {
        STRcopy(string,station->station_label+1,STRING_LEN);
    }
    len = strlen(string);
    XTextExtents(frame->x->font ,string, len,
		 &direct, &ascent, &descent, &overall);
  
    xoffset =  BLANK_PIXEL_SPACING; 
    yoffset =  -overall.ascent  - overall.descent - BLANK_PIXEL_SPACING;

    GDrawImageStringOffset(dev, frame, gc,
			   frame->x->font, frame->psgc,
			   XJ_LEFT, YJ_BELOW,
			   world_x, world_y,
			   xoffset, -yoffset,
			   string);
  }

  /*
   * Plot  GUST as text - Lower Right
   */

    if (station->windgust != STATION_NAN &&
	station->windgust > 2.0)
    {
      sprintf(string,"G%.0f",station->windgust * NMH_PER_MS);

     len = strlen(string);
     XTextExtents(frame->x->font, string, len,
		 &direct, &ascent, &descent, &overall);
  
      xoffset =  BLANK_PIXEL_SPACING; 
      yoffset =  BLANK_PIXEL_SPACING;

      GDrawImageStringOffset(dev, frame, gc,
			   frame->x->font, frame->psgc,
			   XJ_LEFT, YJ_BELOW,
			   world_x, world_y,
			   xoffset, -yoffset,
			   string);

    }

  return;
}


#define OFFSET_ALLOC_INCR    10

/*********************************************************************
 * SPR_draw_obj_bitmap_icon()
 *
 * Renders the given bitmap icon object structure.
 *
 *********************************************************************/

void SPR_draw_obj_bitmap_icon(int dev,
			      gframe_t *frame,
			      Display *display,
			      GC gc,
			      psgc_t *psgc,
			      symprod_bitmap_icon_obj_t *object)
{
  static XPoint *offsets = NULL;
  static int offsets_alloc = 0;
  
  int icon;
  int bit;
  ui08 *bit_ptr;
  
  int num_offsets = 0;
  
  double world_loc_x, world_loc_y;

  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }
  
  /*
   * Accumulate the points in the icon.
   */

  bit_ptr = (ui08 *)((char *)object + sizeof(symprod_const_bitmap_icon_obj_t) +
    object->num_icons * sizeof(symprod_wpt_t));
  
  for (bit = 0;
       bit < object->bitmap_x_dim * object->bitmap_y_dim;
       bit++)
  {
    if (bit_ptr[bit])
    {
      num_offsets++;
      
      if (num_offsets >= offsets_alloc)
      {
	offsets_alloc += OFFSET_ALLOC_INCR;
	
	if (offsets == NULL)
	  offsets = (XPoint *)umalloc(offsets_alloc * sizeof(XPoint));
	else
	  offsets = (XPoint *)urealloc(offsets,
				       offsets_alloc * sizeof(XPoint));
      }
      
      offsets[num_offsets-1].x = -(object->bitmap_x_dim / 2 -
	bit % object->bitmap_x_dim);

      offsets[num_offsets-1].y = object->bitmap_y_dim / 2 -
	bit / object->bitmap_x_dim;
      
    }

  } /* endfor - bit */
    
  /*
   * Draw the icon at each of the given positions.
   */

  for (icon = 0; icon < object->num_icons; icon++)
  {
    symprod_wpt_t *origin =
      (symprod_wpt_t *)((char *)object +
			sizeof(symprod_const_bitmap_icon_obj_t) +
			icon * sizeof(symprod_wpt_t));
    
    /*
     * Determine the world coordinates for the icon.
     */

    convert_lat_lon_to_world(origin->lat,
			     origin->lon,
			     frame,
			     &world_loc_x,
			     &world_loc_y);
      
    
    /*
     * Now draw the icon.
     */

    GDrawPoints(dev,
		frame,
		gc,
		psgc,
		world_loc_x,
		world_loc_y,
		offsets,
		num_offsets);
    
  } /* endfor - icon */
  
  return;
}


/*********************************************************************
 * SPR_draw_obj_chunk()
 *
 * Renders the given chunk object structure.
 *
 *********************************************************************/

void SPR_draw_obj_chunk(int dev,
			gframe_t *frame,
			Display *display,
			GC gc,
			psgc_t *psgc,
			symprod_chunk_obj_t *object)
{
  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }

  /*
   * Render the chunk based on its type.
   */

  switch (object->chunk_type)
  {
  case SPDB_STATION_REPORT_ID :
    /*
     * Swap the report from big-endian to native format.
     */

    station_report_from_be((station_report_t *)object->data);

    SPR_draw_chunk_station(dev,
			   frame,
			   display,
			   gc,
			   psgc,
			   (station_report_t *)object->data);
    break;

  case SPDB_STATION_REPORT_ARRAY_ID :

    SPR_draw_chunk_station_array(dev,
			   frame,
			   display,
			   gc,
			   psgc,
			   object->data);
    break;
    
  default :
    fprintf(stderr,
	    "WARNING:  Don't know how to render chunk type %d -- skipping\n",
	    object->chunk_type);
    break;
    
  } /* endswitch - object->chunk_type */

  return;
}


/*********************************************************************
 * SPR_draw_obj_polyline()
 *
 * Renders the given polyline object structure.
 *
 *********************************************************************/

void SPR_draw_obj_polyline(int dev,
			   gframe_t *frame,
			   Display *display,
			   GC gc,
			   psgc_t *psgc,
			   symprod_polyline_obj_t *object)
{
  static GPoint *gpts = NULL;
  static int gpts_alloc = 0;
  
  int gpts_used = 0;
  
  int i;
  double x_world;
  double y_world;
  
  symprod_wpt_t *pt;
  
  Pixmap *stipple;
  
  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }
  
  /*
   * Set the line attributes.
   */

  XSetLineAttributes(frame->x->display, gc,
		     object->linewidth,
		     convert_linetype_to_x(object->linetype),
		     convert_capstyle_to_x(object->capstyle),
		     convert_joinstyle_to_x(object->joinstyle));
  
  if (object->close_flag &&
      object->fill >= SYMPROD_FILL_STIPPLE10 &&
      object->fill <= SYMPROD_FILL_STIPPLE90)
  {
    stipple = convert_stipple_to_x(frame->x->display,
				   object->fill);
  
    XSetStipple(frame->x->display, gc, *stipple);
  }
  
  /*
   * Render the line.
   */

  pt = object->points;
  for (i = 0; i < object->num_points; i++, pt++)
  {
    if (pt->lat == SYMPROD_WPT_PENUP ||
	pt->lon == SYMPROD_WPT_PENUP)
    {
      if (gpts_used > 0)
      {
	XSetFillStyle(frame->x->display, gc,
		      FillSolid);
  
	GDrawLines(dev, frame, gc, psgc,
		   gpts, gpts_used, CoordModeOrigin);

	if (object->close_flag &&
	    object->fill != SYMPROD_FILL_NONE)
	{
	  XSetFillStyle(frame->x->display, gc,
			FillStippled);
  
	  GFillPolygon(dev, frame, gc, psgc,
		       gpts, gpts_used, CoordModeOrigin);
	}
	
	gpts_used = 0;
      }
    }
    else
    {
      convert_lat_lon_to_world(pt->lat, pt->lon,
			       frame, &x_world, &y_world);

      if (gpts_used >= gpts_alloc)
      {
	gpts_alloc += ARRAY_ALLOC_INCR;
	
	if (gpts == NULL)
	  gpts = (GPoint *)umalloc(gpts_alloc * sizeof(GPoint));
	else
	  gpts = (GPoint *)urealloc(gpts, gpts_alloc * sizeof(GPoint));
      }
      
      gpts[gpts_used].x = x_world;
      gpts[gpts_used].y = y_world;
      
      gpts_used++;
    }

  } /* endfor - i */
  
  /*
   * Readd the last point if the polygon should be closed
   */

  if (object->close_flag &&
      gpts_used > 0 &&
      object->points[0].lat != SYMPROD_WPT_PENUP &&
      object->points[0].lon != SYMPROD_WPT_PENUP)
  {
    convert_lat_lon_to_world(object->points[0].lat,
			     object->points[0].lon,
			     frame,
			     &x_world,
			     &y_world);
  
    if (gpts_used >= gpts_alloc)
    {
      gpts_alloc += ARRAY_ALLOC_INCR;
	
      if (gpts == NULL)
	gpts = (GPoint *)umalloc(gpts_alloc * sizeof(GPoint));
      else
	gpts = (GPoint *)urealloc(gpts, gpts_alloc * sizeof(GPoint));
    }
      
    gpts[gpts_used].x = x_world;
    gpts[gpts_used].y = y_world;
      
    gpts_used++;
  }
  
  /*
   * Now render any remaining line
   */

  if (gpts_used > 0)
  {
    XSetFillStyle(frame->x->display, gc,
		  FillSolid);
  
    GDrawLines(dev, frame, gc, psgc,
	       gpts, gpts_used, CoordModeOrigin);

    if (object->close_flag &&
	object->fill != SYMPROD_FILL_NONE)
    {
      XSetFillStyle(frame->x->display, gc,
		    FillStippled);
  
      GFillPolygon(dev, frame, gc, psgc,
		   gpts, gpts_used, CoordModeOrigin);
    }
    
  }
  
  return;
}


/*********************************************************************
 * SPR_draw_obj_stroked_icon()
 *
 * Renders the given stroked icon object structure.
 *
 *********************************************************************/

void SPR_draw_obj_stroked_icon(int dev,
			       gframe_t *frame,
			       Display *display,
			       GC gc,
			       psgc_t *psgc,
			       symprod_stroked_icon_obj_t *object)
{
  int icon;
  int icon_pt;
  
  double world_loc_x, world_loc_y;
  
  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }
  
  /*
   * Draw the icon at each of the given positions.
   */

  for (icon = 0; icon < object->num_icons; icon++)
  {
    symprod_wpt_t *origin =
      (symprod_wpt_t *)((char *)object +
			sizeof(symprod_const_stroked_icon_obj_t) +
			object->num_icon_pts * sizeof(symprod_ppt_t) +
			icon * sizeof(symprod_wpt_t));

#ifdef DEBUG    
    fprintf(stderr,
	    "Creating stroked icon at lat = %f, lon = %f\n",
	    origin->lat, origin->lon);
#endif
  
    /*
     * Determine the world coordinates for the icon.
     */

    convert_lat_lon_to_world(origin->lat,
			     origin->lon,
			     frame,
			     &world_loc_x,
			     &world_loc_y);
      
    
    /*
     * Draw each stroke in the icon.  The strokes are drawn individually
     * from icon_pt-1 to icon_pt.
     */

    for (icon_pt = 1; icon_pt < object->num_icon_pts; icon_pt++)
    {
      double world_x1, world_y1;
      double world_x2, world_y2;
      
      if (object->icon_pts[icon_pt - 1].x == SYMPROD_PPT_PENUP ||
	  object->icon_pts[icon_pt - 1].y == SYMPROD_PPT_PENUP ||
	  object->icon_pts[icon_pt].x == SYMPROD_PPT_PENUP ||
	  object->icon_pts[icon_pt].y == SYMPROD_PPT_PENUP)
	continue;
      
      world_x1 = GXWorldx(frame, GXWindowx(frame, world_loc_x) +
			  object->icon_pts[icon_pt - 1].x);
      world_y1 = GXWorldy(frame, GXWindowy(frame, world_loc_y) +
			  object->icon_pts[icon_pt - 1].y);
      
      world_x2 = GXWorldx(frame, GXWindowx(frame, world_loc_x) +
			  object->icon_pts[icon_pt].x);
      world_y2 = GXWorldy(frame, GXWindowy(frame, world_loc_y) +
			  object->icon_pts[icon_pt].y);
      
      GDrawLine(dev,
		frame,
		gc,
		psgc,
		world_x1,
		world_y1,
		world_x2,
		world_y2);
      
    } /* endfor - icon_pt */
    
  } /* endfor - icon */
  
  return;
}


/*********************************************************************
 * SPR_draw_obj_text()
 *
 * Renders the given text object structure.
 *
 *********************************************************************/

void SPR_draw_obj_text(int dev,
		       gframe_t *frame,
		       Display *display,
		       GC gc,
		       psgc_t *psgc,
		       symprod_text_obj_t *object)
{
  double x_world;
  double y_world;
  int x_just = XJ_LEFT;
  int y_just = YJ_BELOW;
  
  
  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }
  
  /*
   * Convert the justification values.
   */

  switch(object->horiz_alignment)
  {
  case SYMPROD_TEXT_HORIZ_ALIGN_LEFT :
    x_just = XJ_LEFT;
    break;
    
  case SYMPROD_TEXT_HORIZ_ALIGN_CENTER :
    x_just = XJ_CENTER;
    break;
    
  case SYMPROD_TEXT_HORIZ_ALIGN_RIGHT :
    x_just = XJ_RIGHT;
    break;
  }
  
  switch(object->vert_alignment)
  {
  case SYMPROD_TEXT_VERT_ALIGN_TOP :
    y_just = YJ_BELOW;
    break;
    
  case SYMPROD_TEXT_VERT_ALIGN_CENTER :
    y_just = YJ_CENTER;
    break;
    
  case SYMPROD_TEXT_VERT_ALIGN_BOTTOM :
    y_just = YJ_ABOVE;
    break;
  }
  
  /*
   * Draw the text where indicated
   */

#ifdef DEBUG
  fprintf(stderr, "Drawing label \"%s\" at lat = %f, lon = %f\n",
	  object->string, object->origin.lat, object->origin.lon);
#endif
  
  convert_lat_lon_to_world(object->origin.lat,
			   object->origin.lon,
			   frame,
			   &x_world,
			   &y_world);


  GDrawImageStringOffset(dev,
			 frame,
			 gc,
			 frame->x->font,
			 psgc,
			 x_just,
			 y_just,
			 x_world,
			 y_world,
			 object->offset.x,
			 object->offset.y,
			 object->string);
		   
  return;
}


/*********************************************************************
 * SPR_draw_object()
 *
 * Renders the given object structure.
 *
 *********************************************************************/

void SPR_draw_object(int dev,
		     gframe_t *frame,
		     Display *display,
		     Colormap cmap,
		     x_color_list_index_t *list_index,
		     symprod_object_t *object)
{
  XColor xcolor;

  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }
  
  
  if (dev == XDEV)
  {
#ifdef CONSERVE_X_CALLS
    /* If the Color has changed, Set the GC */

    if (!STRequal_exact(object->color, Last_foreground_color))
    {
#endif
        /*
         * Set the Foreground color in the GC for rendering.
         */

        if (XParseColor(display, cmap, object->color, &xcolor) == 0)
        {
          fprintf(stderr, "ERROR - SPR_draw_object\n");
          fprintf(stderr, "Cannot match foreground color '%s'\n",
		  object->color);
          return;
        } 
        XAllocColor(display, cmap, &xcolor);
        XSetForeground(display, frame->x->gc , xcolor.pixel);
        
#ifdef CONSERVE_X_CALLS
	STRcopy(Last_foreground_color, object->color,
		SYMPROD_COLOR_LEN);
    }
#endif

    if (object->background_color[0] == '\0')
    {
      if (Background_pixel_init)
        XSetBackground(display, frame->x->gc, Background_pixel);
    }
    else
#ifdef CONSERVE_X_CALLS
      if (!STRequal_exact(object->background_color, Last_background_color))
#endif
    {
        /*
         * Set the Background color in the GC for rendering.
         */

        if (XParseColor(display, cmap,
			object->background_color, &xcolor) == 0)
        {
          fprintf(stderr, "ERROR - SPR_draw_object\n");
          fprintf(stderr, "Cannot match background color '%s'\n",
		  object->background_color);
          return;
        } 
        XAllocColor(display, cmap, &xcolor);
        XSetBackground(display, frame->x->gc , xcolor.pixel);
        
#ifdef CONSERVE_X_CALLS
	STRcopy(Last_background_color, object->background_color,
		SYMPROD_COLOR_LEN);
#endif
    }


  }
    
  /*
   * Render the object based on its type.
   */

  switch (object->object_type)
  {
  case SYMPROD_OBJ_TEXT :
    SPR_draw_obj_text(dev,
		      frame,
		      display,
		      frame->x->gc ,
		      frame->psgc,
		      &object->object_info->text);
    return;
    
  case SYMPROD_OBJ_POLYLINE :
    SPR_draw_obj_polyline(dev,
			  frame,
			  display,
			  frame->x->gc ,
			  frame->psgc,
			  &object->object_info->polyline);
    return;
    
  case SYMPROD_OBJ_STROKED_ICON :
    SPR_draw_obj_stroked_icon(dev,
			      frame,
			      display,
			      frame->x->gc ,
			      frame->psgc,
			      &object->object_info->stroked_icon);
    return;
    
  case SYMPROD_OBJ_NAMED_ICON :
    fprintf(stderr,
	    "ERROR: Named icon object rendering not yet implemented.\n");
    return;
    
  case SYMPROD_OBJ_BITMAP_ICON :
    SPR_draw_obj_bitmap_icon(dev,
			     frame,
			     display,
			     frame->x->gc ,
			     frame->psgc,
			     &object->object_info->bitmap_icon);
    return;
    
  case SYMPROD_OBJ_ARC :
    fprintf(stderr,
	    "ERROR: Arc object rendering not yet implemented.\n");
    return;
    
  case SYMPROD_OBJ_RECTANGLE :
    fprintf(stderr,
	    "ERROR: Rectangle object rendering not yet implemented.\n");
    return;
    
  case SYMPROD_OBJ_CHUNK :
    SPR_draw_obj_chunk(dev,
		       frame,
		       display,
		       frame->x->gc ,
		       frame->psgc,
		       &object->object_info->chunk);
    return;
    
  default:
    fprintf(stderr,
	    "ERROR: Invalid object type %d encountered.\n",
	    object->object_type);
    return;
    
  } /* endswitch - object->object_type */

  return;
}


/*********************************************************************
 * SPR_draw_prod_queue()
 *
 * Renders all of the active, non-expired, selected for display 
 * symbolic products currently in the product queue.
 *
 *********************************************************************/

void SPR_draw_prod_queue(int dev,
			 gframe_t *frame,
			 Display *display,
			 Colormap cmap,
			 x_color_list_index_t *list_index,
			 pq_handle_t *pq_handle)
{
  int slot;
  
#ifdef DEBUG  
  fprintf(stderr,
	  "Entering SPR_draw_prod_queue\n");
#endif
  
  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }

  /*
   * Set the font in the GC
   */

  XSetFont(frame->x->display, frame->x->gc , frame->x->font->fid);

  /*
   * Set the background color in the GC
   */

  if (!Background_pixel_init)
  {
    Background_pixel = BlackPixel(frame->x->display,
				  DefaultScreen(frame->x->display));
  
    Background_pixel_init = TRUE;
  }
      
  XSetBackground(display, frame->x->gc , Background_pixel);
  
  /*
   * Set the product queue semaphores.
   */

  PQ_set_semaphores(pq_handle);
  
  /*
   * Look at each of the slots, rendering the product if it is
   * active, if it isn't expired and if the user has chosen to display
   * it.
   */

  for (slot = 0; slot < pq_handle->status_ptr->total_slots; slot++)
  {
    if (pq_handle->slot_ptr[slot].active &&
	!pq_handle->slot_ptr[slot].expired &&
	pq_handle->slot_ptr[slot].display)
    {
      symprod_product_t *product;
      
#ifdef DEBUG
      fprintf(stdout,
	      "*** Displaying product in slot %d\n", slot);
#endif

      product = SYMPRODbufferToProduct(pq_handle->buffer_ptr +
				       pq_handle->slot_ptr[slot].offset +
				       sizeof(pq_buffer_info_t));
      
      SPR_draw_product(dev,
		       frame,
		       display,
		       cmap,
		       list_index,
		       product);
      
      SYMPRODfreeProduct(product);
      
    } /* endif - pq_handle->slot_ptr[slot].active && ... */
    
  } /* endfor - slot */
  
  /*
   * Clear the product queue semaphores.
   */

  PQ_clear_semaphores(pq_handle);

#ifdef DEBUG  
  fprintf(stderr,
	  "Leaving SPR_draw_prod_queue\n");
#endif
  
  return;
}


/*********************************************************************
 * SPR_draw_product()
 *
 * Renders the given product structure.
 *
 *********************************************************************/

void SPR_draw_product(int dev,
		      gframe_t *frame,
		      Display *display,
		      Colormap cmap,
		      x_color_list_index_t *list_index,
		      symprod_product_t *product)
{

  int i;
  symprod_object_t *obj;
  
  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }
  
  /*
   * Render each object in the product.
   */

  obj = (symprod_object_t *) MEMbufPtr(product->mbuf_obj);
  for (i = 0; i < product->num_objs; i++, obj++)
  {
    SPR_draw_object(dev, frame, display,
		    cmap, list_index, obj);
    
  } /* endfor - i */
  
  return;
}


/*********************************************************************
 * SPR_init()
 *
 * Initializes some constants use by the SPR routines.
 *
 * This routine MUST be called before any other SPR routine is called.
 * It may be called multiple times if the projection changes.
 *
 * The origin_lat and origin_lon values are not used in the
 * MDV_PROJ_LATLON projection, so they can be set to anything in this
 * case.
 *
 * Currently only MDV_PROJ_FLAT and MDV_PROJ_LATLON projections are
 * supported.
 *
 *********************************************************************/

void SPR_init(int projection,             /* MDV_PROJ_xxx */
	      double origin_lat,
	      double origin_lon)
{
  Projection = projection;
  
  Origin_lat = origin_lat;
  Origin_lon = origin_lon;
  
  Spr_initialized = TRUE;

#ifdef CONSERVE_X_CALLS
  memset(Last_foreground_color, 0, SYMPROD_COLOR_LEN);
  memset(Last_background_color, 0, SYMPROD_COLOR_LEN);
#endif

  return;
}


/*********************************************************************
 * SPR_set_background_color()
 *
 * Sets the backgroud color for SPR rendering.
 *
 *********************************************************************/

void SPR_set_background_color(Display *display,
			      Colormap cmap,
			      x_color_list_index_t *list_index,
			      char *color_name)
{
  XColor xcolor;
  
  /*
   * Make sure SPR_init() has been called.
   */

  if (!Spr_initialized)
  {
    fprintf(stderr,
	    "ERROR: SPR_init() not yet called\n");
    
    return;
  }
  
  /*
   * Get the pixel value for the color.
   */
  if (XParseColor(display, cmap, color_name, &xcolor) == 0) {
    fprintf(stderr, "ERROR - SPR_set_background_color\n");
    fprintf(stderr, "Cannot match color '%s'\n", color_name);
    return;

  }

  XAllocColor(display, cmap, &xcolor);

  Background_pixel = xcolor.pixel;

  Background_pixel_init = TRUE;

#ifdef DEBUG
  fprintf(stderr, 
	  "*** Initializing background to color %s, pixel %d\n",
	  color_name, xcolor.pixel);
#endif

  return;
}


/*********************************************************************
 * STATIC FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * convert_capstyle_to_x()
 *
 * Convert the SYMPROD capstyle to the value used by X.
 *********************************************************************/

static int convert_capstyle_to_x(int capstyle)
{
  switch (capstyle)
  {
  case SYMPROD_CAPSTYLE_BUTT :
    return(CapButt);
    
  case SYMPROD_CAPSTYLE_NOT_LAST :
    return(CapNotLast);
    
  case SYMPROD_CAPSTYLE_PROJECTING :
    return(CapProjecting);
    
  case SYMPROD_CAPSTYLE_ROUND :
    return(CapRound);
  } /* endswitch - capstyle */
  
  return(CapButt);
}


/*********************************************************************
 * convert_joinstyle_to_x()
 *
 * Convert the SYMPROD joinstyle to the value used by X.
 *********************************************************************/

static int convert_joinstyle_to_x(int joinstyle)
{
  switch (joinstyle)
  {
  case SYMPROD_JOINSTYLE_BEVEL :
    return(JoinBevel);
    
  case SYMPROD_JOINSTYLE_MITER :
    return(JoinMiter);
    
  case SYMPROD_JOINSTYLE_ROUND :
    return(JoinRound);
  } /* endswitch - joinstyle */
  
  return(JoinBevel);
}


/*********************************************************************
 * convert_lat_lon_to_world()
 *
 * Convert the lat/lon values in a SYMPROD object to the correct
 * world coordinates for the display.
 *********************************************************************/

static void convert_lat_lon_to_world(double lat,
				     double lon,
				     gframe_t *frame,
				     double *world_x,
				     double *world_y)
{
  switch(Projection)
  {
  case MDV_PROJ_FLAT :
    PJGLatLon2DxDy(Origin_lat, Origin_lon,
		   lat, lon,
		   world_x, world_y);
    return;
    
  case MDV_PROJ_LATLON :
    *world_x = lon;
    *world_y = lat;
    return;
    
  } /* endswitch - projection */
  
}


/*********************************************************************
 * convert_linetype_to_x()
 *
 * Convert the SYMPROD linetype to the value used by X.
 *********************************************************************/

static int convert_linetype_to_x(int linetype)
{
  switch (linetype)
  {
  case SYMPROD_LINETYPE_SOLID :
    return(LineSolid);
    
  case SYMPROD_LINETYPE_DASH :
    return(LineOnOffDash);
    
  case SYMPROD_LINETYPE_DOT_DASH :
    return(LineDoubleDash);
  } /* endswitch - linetype */
  
  return(LineSolid);
}


/*********************************************************************
 * convert_stipple_to_x()
 *
 * Convert the SYMPROD stipple fill type to the value used by X.
 *********************************************************************/

static Pixmap *convert_stipple_to_x(Display *display,
				    int filltype)
{
  static Pixmap stipple;
  
  unsigned int stipple_width, stipple_height;
  
  switch (filltype)
  {
  case SYMPROD_FILL_STIPPLE10 :
    create_included_stipple(display,
			    stipple10_bitmap_bits,
			    stipple_bitmap_width,
			    stipple_bitmap_height,
			    &stipple,
			    &stipple_width,
			    &stipple_height);
    break;
    
  case SYMPROD_FILL_STIPPLE20 :
    create_included_stipple(display,
			    stipple20_bitmap_bits,
			    stipple_bitmap_width,
			    stipple_bitmap_height,
			    &stipple,
			    &stipple_width,
			    &stipple_height);
    break;
    
  case SYMPROD_FILL_STIPPLE30 :
    create_included_stipple(display,
			    stipple30_bitmap_bits,
			    stipple_bitmap_width,
			    stipple_bitmap_height,
			    &stipple,
			    &stipple_width,
			    &stipple_height);
    break;
    
  case SYMPROD_FILL_STIPPLE40 :
    create_included_stipple(display,
			    stipple40_bitmap_bits,
			    stipple_bitmap_width,
			    stipple_bitmap_height,
			    &stipple,
			    &stipple_width,
			    &stipple_height);
    break;
    
  case SYMPROD_FILL_STIPPLE50 :
    create_included_stipple(display,
			    stipple50_bitmap_bits,
			    stipple_bitmap_width,
			    stipple_bitmap_height,
			    &stipple,
			    &stipple_width,
			    &stipple_height);
    break;
    
  case SYMPROD_FILL_STIPPLE60 :
    create_included_stipple(display,
			    stipple60_bitmap_bits,
			    stipple_bitmap_width,
			    stipple_bitmap_height,
			    &stipple,
			    &stipple_width,
			    &stipple_height);
    break;
    
  case SYMPROD_FILL_STIPPLE70 :
    create_included_stipple(display,
			    stipple70_bitmap_bits,
			    stipple_bitmap_width,
			    stipple_bitmap_height,
			    &stipple,
			    &stipple_width,
			    &stipple_height);
    break;
    
  case SYMPROD_FILL_STIPPLE80 :
    create_included_stipple(display,
			    stipple80_bitmap_bits,
			    stipple_bitmap_width,
			    stipple_bitmap_height,
			    &stipple,
			    &stipple_width,
			    &stipple_height);
    break;
    
  case SYMPROD_FILL_STIPPLE90 :
    create_included_stipple(display,
			    stipple90_bitmap_bits,
			    stipple_bitmap_width,
			    stipple_bitmap_height,
			    &stipple,
			    &stipple_width,
			    &stipple_height);
    break;
    
  }
  
  return(&stipple);
}


/*********************************************************************
 * create_included_stipple()
 *
 * Create a stipple pattern from one of the bit maps defined above.
 *********************************************************************/

static int create_included_stipple(Display *display,
				   char *bitmap,
				   int bitmap_width,
				   int bitmap_height,
				   Pixmap *stipple,
				   unsigned int *width,
				   unsigned int *height)
{
  if ((*stipple = XCreateBitmapFromData(display,
					RootWindow(display, 0),
					bitmap,
					bitmap_width, bitmap_height))
      == FALSE)
    return(FALSE);
  
  *width = bitmap_width;
  *height = bitmap_height;
  
  return(TRUE);
}
