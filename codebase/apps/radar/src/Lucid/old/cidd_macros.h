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
/*****************************************************************
 * CIDD_MACROS.H : Macro Definitions for Cidd
 *  Frank Hage NCAR/RAP 1991 - 2009
 *  Mike Dixon NCAR/RAP 1994 -
 */

#ifndef CIDD_MACROS_H
#define CIDD_MACROS_H

#define LUCID_VERSION    "1.0 2024/09/01"
#define LUCID_COPYRIGHT  "(c) 1991-2024,  National Center for Atmospheric Research"
 
#define MAX_COLORS    1024    /* maximum number of colors in images*/
#define MAX_FRAMES    512     /* maximum number of movie frames  */
#define MAX_ROWS      16384   /* maximum number of rows in data fields */
#define MAX_COLS      16384   /* maximum number of columns in data fields */
#define MAX_SECTS     128     /* maximum number of sections (planes) in data fields */
#define MAX_ROUTE_SEGMENTS 128 /* maximum number of sections along one route */
#define MAX_XSECT_POINTS 256
#define MAX_COLOR_CELLS  65536  /* maximum cells in data to color lookup table  */
#define MAX_FONTS        16      /* maximum number of fonts */
#define MAX_DATA_FIELDS  4096    /* maximum number of data fields  */
#define MAX_CACHE_PIXMAPS 32     /* maximum number of Cache'd pixmaps  */
#define MAX_OVERLAYS     512     /* maximum number of map overlays */
#define NUM_GRID_LAYERS  16      /* number of layers possible overlaid grids  */
#define NUM_CONT_LAYERS  16      /* number layers possible overlaid contours  */
#define NUM_CUSTOM_ZOOMS 3      /* number of Custom Zoom domains */

#define REDRAW_INTERVAL  1000    /* DefaultNumber of milliseconds between background redraw ops */
#define UPDATE_INTERVAL    120  /* DefaultNumber of seconds intervals between realtime update ops */
#define MIN_MOVIE_TIME 40    /* Minimum number of milliseconds to display a movie frame  */
#define MOVIE_SPEED_RANGE 20 /* number of speed settings possible */
#define MOVIE_DELAY_INCR 25 /* Delay increments by this number of msec on slider action */
#define URL_MIN_SIZE 8   // Must be at least http://h

#define CLIP_BUFFER 100  // Dont try to render past this point in pixels from the screen edge.
#define MAX_ASPECT_RATIO 200.0 // Dont try to render grid points which have aspect ratios outside this range.

#define CIDD_SUCCESS 0        /* return status indicators for subroutines */
#define CIDD_FAILURE 1
#define INCOMPLETE 2

#define PLAN_VIEW 1
#define XSECT_VIEW 2
#define BOTH_VIEWS 3

#define CHAR            1       /* For data typing */
#define SHORT           2
#define INT             3
#define LONG            3
#define FLOAT           4
#define DOUBLE          5

#define OVERLAY_COLOR    "White" /* default overlay color */

#define LABEL_LENGTH       128   /* Space reserved for unit labels, etc */
#define NAME_LENGTH        128   /* Space reserved for names */
#define TITLE_LENGTH       128   /* Space reserved for titles */
#define URL_LENGTH         1024  /* Space reserved for URL's */
 
#define     LIVE_DATA    1       /* data request operation mode */
#define     STATIC_DATA  2
#define     DSMDVX_DATA  3
#define     SYMPROD_DATA  4

#define NO_REQUEST 0
#define HORIZ_REQUEST 1
#define VERT_REQUEST 2
#define SYMPROD_REQUEST 3
#define TIMELIST_REQUEST 4

#define     NO_DRAW_MODE 0
#define     DRAW_ROUTE_MODE 1
#define     DRAW_FMQ_MODE 2
#define     PICK_PROD_MODE 3

#define        SHMEM_1        32        /* dobson format shared memory */

#define BGND    8        /* color index to background color */
#define FGND    9        /* color index to foreground color */
#define GEO    10        /* color index to geographical features color */
#define CROSS  11        /* Color index for the cross reference lines on the CAP/ROUTE */
#define WIND1  12        /* Color index for the Wind vectors */
#define WIND2  13        /* Color index for the Wind vectors */
#define WIND3  14        /* Color index for the Wind vectors */
#define CONT   15        /* Color index for contours */
#define DRAW   16        /* Color index for user drawings */

#define CURRENT_TIME 0x7fffffff    /* time indicated for most recent */
#define REALTIME_MODE 0        /* Real-time mode */
#define ARCHIVE_MODE 1        /* Archive span mode */

#define REGULAR_INTERVAL 0      /* Climo mode off */
#define DAILY_INTERVAL 1        /* Climo mode */
#define YEARLY_INTERVAL 2       /* Climo mode */

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.017453292519943295 // degrees to radians conversion
#endif

#define DEG_RAD 57.29577951308232  // degrees per radian

#define RADIAN90  1.5707963267948966 // radian value for 90 degrees 
#define RADIAN180 3.1415926535897930 // radian value for 180 degrees 
#define RADIAN270 4.7123889803846900 // radian value for 270 degrees 

#ifndef KM_PER_DEG_AT_EQ
#define KM_PER_DEG_AT_EQ 111.198487
#endif

#define PSEUDO_RADIUS 8533.0 // 4/3 earth radius

/* Methods of rendering Gridded false colored data */

typedef enum
  {
   POLYGONS = 0,
   FILLED_CONTOURS = 1,
   DYNAMIC_CONTOURS = 2,
   LINE_CONTOURS = 3
  } render_method_t;
 
/*    OR's flag with 1 if cliped and "returns" a value >= l && <= u */

#define CLIP(x,l,u,flag) (x < l) ? flag|=1,l: ((x > u)? flag|=1,u:x) 

#ifndef ABS
#define ABS(a) (a)>=0 ? (a) : -(a)
#endif

#define ABSDIFF(a,b) (((a) < (b))? (b)-(a): (a)-(b))

#define ZERO_STRUCT(p) memset((void *)(p),0,sizeof (*(p)))

#endif
