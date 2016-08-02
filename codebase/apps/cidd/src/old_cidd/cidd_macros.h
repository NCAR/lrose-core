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
 * CIDD_MACROS.H : Macro Definitions for the RD display program
 */

#define APP_DATABASE    "cidd_defaults"     /* the default prarmeter database */
 
#define MAX_COLORS    1024   /* maximum number of colors in images*/
#define MAX_FRAMES    256    /* maximum number of movie frames  */
#define MAX_ROWS      4096   /* maximum number of rows in data fields */
#define MAX_COLS      4096   /* maximum number of columns in data fields */
#define MAX_SECTS     128    /* maximum number of sections (planes) in data fields */
#define MAX_DATA_VALS    256    /* maximum number of data values (dynamic range) */
#define MAX_FONTS        16      /* maximum number of fonts */
#define MAX_DATA_FIELDS  256    /* maximum number of data fields  */
#define MAX_OVERLAYS     256     /* maximum number of map overlays */
#define NUM_GRID_LAYERS  5      /* number of layers possible overlaid grids  */
#define NUM_CONT_LAYERS  5      /* number layers possible overlaid contours  */
#define NUM_CUSTOM_ZOOMS 3      /* number of Custom Zoom domains */

#define REDRAW_INTERVAL  1000    /* DefaultNumber of milliseconds between background redraw ops */
#define UPDATE_INTERVAL    120  /* DefaultNumber of seconds intervals between realtime update ops */
#define MIN_MOVIE_TIME 40    /* Minimum number of milliseconds to display a movie frame  */
#define MOVIE_SPEED_RANGE 20 /* number of speed settings possible */
#define MOVIE_DELAY_INCR 25 /* Delay increments by this number of msec on slider action */

#define SUCCESS 0        /* return status indicators for subroutines */
#define FAILURE 1
#define INCOMPLETE 2

#define CHAR            1       /* For data typing */
#define SHORT           2
#define INT             3
#define LONG            3
#define FLOAT           4
#define DOUBLE          5

#define OVERLAY_COLOR    "White" /* default overlay color */

#define LABEL_LENGTH       16    /* Space reserved for unit labels, etc */
#define NAME_LENGTH        128   /* Space reserved for names */
#define TITLE_LENGTH       128   /* Space reserved for titles */
 
#define     LIVE_DATA    1       /* bits selected for data operation mode */
#define     STATIC_DATA  2

#define        SHMEM_1        32        /* dobson format shared memory */

#define BGND    8        /* color index to background color */
#define FGND    9        /* color index to foreground color */
#define GEO    10        /* color index to geographical features color */
#define CROSS  11        /* Color index for the cross reference lines on the CAP/RHI */
#define WIND1  12        /* Color index for the Wind vectors */
#define WIND2  13        /* Color index for the Wind vectors */
#define WIND3  14        /* Color index for the Wind vectors */
#define CONT   15        /* Color index for contours */
#define DRAW   16        /* Color index for user drawings */

#define CURRENT_TIME 0x7fffffff    /* time indicated for most recent */
#define MOVIE_MR 0        /* Most recent mode */
#define MOVIE_TS 1        /* Time span mode */
#define MOVIE_EL 2        /* Elevation mode */

#define DEG_RAD 57.29577951    /* degrees per radian */
#define KM_PER_DEG_AT_EQ 111.198487

#define CIDD_TO_EXPT_DATA   100

#define CART_DATA_FORMAT  0            /* for data format flag */
#define PPI_DATA_FORMAT   1

#define CARTESIAN_PROJ    0
#define LAT_LON_PROJ      1

/* Methods of rendering false colored data */
#define RECTANGLES  0
#define FILLED_CONTOURS 1
 
/*    OR's flag with 1 if cliped and "returns" a value >= l && <= u */
#define CLIP(x,l,u,flag) (x < l) ? flag|=1,l: ((x > u)? flag|=1,u:x) 
#define ABSDIFF(a,b) (((a) < (b))? (b)-(a): (a)-(b))
#define ZERO_STRUCT(p)      memset((void *)(p),0,sizeof (*(p)))
