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

/******************************************************************
 * spr.h: header file for Symbolic Product Render module
 *
 * This module contains routines for rendering symbolic products in
 * the SYMPROD format on a display.
 *
 * SPR_init() must be called before any other routine in this
 * module is called.
 *
 ******************************************************************/

#ifndef spr_h
#define spr_h


#include <mdv/mdv_macros.h>
#include <rapplot/gplot.h>
#include <rapformats/station_reports.h>
#include <symprod98/prod_queue.h>
#include <symprod98/symprod.h>


#ifdef __cplusplus
extern "C" {
#endif

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
	      double origin_lon);

/*********************************************************************
 * SPR_set_background_color()
 *
 * Sets the backgroud color for SPR rendering.
 *
 *********************************************************************/

void SPR_set_background_color(Display *display,
			      Colormap cmap,
			      x_color_list_index_t *list_index,
			      char *color_name);

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
			      symprod_bitmap_icon_obj_t *object);

/*********************************************************************
 * SPR_draw_chunk_station()
 *
 * Renders the given station report structure.
 *
 *********************************************************************/

void SPR_draw_chunk_station(int dev,
			    gframe_t *frame,
			    Display *display,
			    GC gc,
			    psgc_t *psgc,
			    station_report_t *station);

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
			symprod_chunk_obj_t *object);

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
			   symprod_polyline_obj_t *object);

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
			       symprod_stroked_icon_obj_t *object);

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
		       symprod_text_obj_t *object);

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
		     symprod_object_t *object);

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
			 pq_handle_t *handle);

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
		      symprod_product_t *product);


#ifdef __cplusplus
}
#endif

#endif
