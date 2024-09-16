/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016 */
/* ** University Corporation for Atmospheric Research (UCAR) */
/* ** National Center for Atmospheric Research (NCAR) */
/* ** Boulder, Colorado, USA */
/* ** BSD licence applies - redistribution and use in source and binary */
/* ** forms, with or without modification, are permitted provided that */
/* ** the following conditions are met: */
/* ** 1) If the software is modified to produce derivative works, */
/* ** such modified software should be clearly marked, so as not */
/* ** to confuse it with the version available from UCAR. */
/* ** 2) Redistributions of source code must retain the above copyright */
/* ** notice, this list of conditions and the following disclaimer. */
/* ** 3) Redistributions in binary form must reproduce the above copyright */
/* ** notice, this list of conditions and the following disclaimer in the */
/* ** documentation and/or other materials provided with the distribution. */
/* ** 4) Neither the name of UCAR nor the names of its contributors, */
/* ** if any, may be used to endorse or promote products derived from */
/* ** this software without specific prior written permission. */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifndef CIDD_WINDOWS_H
#define CIDD_WINDOWS_H

/**********************************************************************
 * CIDD_WINDOWS.H: Data structure defns for CIDD
 */

#include "cidd_layers.h"
#include <QPaintDevice>

typedef struct { /* Margin settings */

 int left; /* pixels at left to avoid */
 int top; /* pixels at top to avoid */
 int right; /* pixels at right to avoid */
 int bot; /* pixels at bottom to avoid */

} margin_t ;

typedef struct { /* Drawable dimensions */

 int x_pos;
 int y_pos;
 int width;
 int height;
 int depth;
 int closed;

} draw_dim_t;

typedef struct zoom_t {

 double zmin_x;
 double zmin_y;
 double zmax_x;
 double zmax_y;

 char *label;

 zoom_t *parent;
 zoom_t *child;

} zoom_t;
 
typedef struct { /* WINDOW Data - for each display window */

 int *ip; /* instance pointer; hook to XView object structures*/
 int active; /* set to 1 if window is currently active */
 int page; /* Current page selected in theis windowr */
 int last_page; /* The last page viewed */
 int movie_page; /* The page currently viewed in movie loops */
 int min_height; /* minimum height window is allowed to get to */
 int min_width; /* minimum width window is allowed to get to */
 int zoom_level; /* index to use for zoom boundary coords */
 int prev_zoom_level; /* index to use for zoom boundary coords */
 int cur_cache_im; /* index to current XID Cache */
 int last_cache_im; /* index to last XID Cache */
 int num_zoom_levels; /* number of stored zoom levels */
 int start_zoom_level; /* The starting zoom level - for reset function */
 int redraw[MAX_DATA_FIELDS]; /* set to 1 when field needs re-rendered */

 int legends_start_x; /* the X value for the first legend character */
 int legends_start_y; /* the Y value for the first legend */
 int legends_delta_y; /* the Y spacing between the legends */

 double origin_lat; /* Latitude origin of the window's coord system */
 double origin_lon; /* Longitude origin of the window's coord system */
 double reset_click_lon; /* Latitude */
 double reset_click_lat; /* Longitude */

 double min_x,max_x; /* Km limits of full display area */
 double min_y,max_y; /* Km limits of full display area */
 double min_lat,max_lat; /* Globe limits of full display area */
 double min_lon,max_lon; /* Globe limits of full display area */
 double min_ht,max_ht; /* Km limits of full display area */
 double min_r,max_r; /* Radial r limits of full display area */

 double min_deg,max_deg;/* Radial deg limits of full display area */

 double cmin_x,cmax_x; /* X limits of current display area */
 double cmin_y,cmax_y; /* Y limits of current display area */
 double cur_ht; /* Z Height of current display area */

 double *zmin_x,*zmax_x; /* X limits of zoomed display area */
 double *zmin_y,*zmax_y; /* Y limits of zoomed display area */
 double zmin_ht,zmax_ht; /* Z limits of zoomed display area */

 double km_across_screen; /* Approx. distance across the window (KM) */

 route_track_t route;

 Drawable vis_xid; /* X ID of Visible canvas */
 Drawable *can_xid; /* X ID of last stage canvases to draw products on top of */
 Drawable tmp_xid; /* X ID of area to drap fields that aren't updated */
 Drawable page_xid[MAX_DATA_FIELDS]; /* draw Pixmap for each field */

 QPaintDevice vis_qpd; /* X ID of Visible canvas */
 QPaintDevice *can_qpd; /* X ID of last stage canvases to draw products on top of */
 QPaintDevice tmp_qpd; /* X ID of area to drap fields that aren't updated */
 QPaintDevice page_qpd[MAX_DATA_FIELDS]; /* draw Pixmap for each field */

 draw_dim_t win_dim; /* Window dimensions and position */
 draw_dim_t can_dim; /* Canvas dimensions and position */
 draw_dim_t img_dim; /* Image Pixmaps dimensions and positions */
 margin_t margin; /* Canvas Margin dimensions */

 char title[TITLE_LENGTH];

 char image_dir[MAX_PATH_LEN];
 char image_fname[MAX_PATH_LEN];
 char image_command[MAX_PATH_LEN];
	 
} win_param_t;

#endif
