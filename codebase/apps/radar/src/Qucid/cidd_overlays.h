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
#ifndef CIDD_OVERLAYS_H
#define CIDD_OVERLAYS_H
/**********************************************************************
 * CIDD_OVERLAYS.H: Data structure defns for CIDD
 */

/* geographic feature coordinates & label */

struct geo_feat_label {

  double min_lat; /* latitude, longitude bounding box for text */
  double min_lon;
  double max_lat;
  double max_lon;

  double attach_lat;
  double attach_lon; /* latitude, longitude of the object associated with the label */
  double local_x;
  double local_y;
  double rotation; /* 0 = left to right, 90 = bottom to top, etc */

  char string[NAME_LENGTH]; /* String to display */

};
typedef struct geo_feat_label Geo_feat_label_t;

/* geographic feature Icon & label */

struct geo_feat_icon {

  double lat;
  double lon;
  double local_x;
  double local_y;

  short text_x;
  short text_y;
  char label[LABEL_LENGTH]; /* Label to display */

  struct geo_feat_icondef *icon;

};
typedef struct geo_feat_icon Geo_feat_icon_t;

/* geographic feature Icon & label */

struct geo_feat_icondef {
  int num_points;
  short *x;
  short *y;
  char name[NAME_LENGTH];
};
typedef struct geo_feat_icondef Geo_feat_icondef_t;

/* geographic feature lines */

struct geo_feat_polyline {

  int num_points;

  double min_x; /* Local coords bounding box */
  double max_x;
  double min_y;
  double max_y;

  double *lat;
  double *lon;
  double *local_x;
  double *local_y;
  
  char label[LABEL_LENGTH]; /* Label of polyline */

};
typedef struct geo_feat_polyline Geo_feat_polyline_t;

// main overlay struct

struct overlay { /* Overlay data */
  long active; /* Current on/off state; 1 = active */
  long default_on_state; /* If set to 1, This overlay should appear by default */

  long num_labels; /* number of associated text strings in map overlay */
  long num_icons; /* number of icons in map overlay */
  long num_icondefs; /* number of icons in map overlay */
  long num_polylines; /* number of polylines in map overlay */

  long num_alloc_labels; /* number of allocated pointers for labels*/
  long num_alloc_icons; /* number of allocated pointers for number of icons */
  long num_alloc_icondefs; /* number of allocated pointers for number of icons */
  long num_alloc_polylines; /* number of allocated pointers for number of polylines */

  long pixval; /* X color cell value to use to draw in the proper color */
  long line_width; /* How wide to draw the line */

  double detail_thresh_min; /* Overlays are visible when distance (km) across */
  double detail_thresh_max; /* across the screen is between min and max */

  Geo_feat_label_t **geo_label;
  Geo_feat_icon_t **geo_icon;
  Geo_feat_icondef_t **geo_icondef;
  Geo_feat_polyline_t **geo_polyline;

  char map_file_name[NAME_LENGTH]; /* Name of map file to read */
  char control_label[LABEL_LENGTH]; /* The overlay's GUI label */
  char map_name[NAME_LENGTH]; /* Long ID */
  char map_code[LABEL_LENGTH]; /* SHORT NAME */
  char color_name[NAME_LENGTH]; /* Current Color */

  Color_gc_t *color;
};
typedef struct overlay Overlay_t; 

typedef struct { /* geographic feature coordinates & label */
  double lat;
  double lon; /* latitude, longitude */
  char label[LABEL_LENGTH]; /* String to label coordinate with */
  int lab_pos; /* 1 = rt, 2 = top 3 = left, 4 = bot */
}geo_feat_coor_t;

typedef struct { /* Arbitrary geographic coordinate */
  double x,y;
}geo_coord_t;
 
#endif
