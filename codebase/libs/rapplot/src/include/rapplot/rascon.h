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
#ifndef RASCON_WAS_INCLUDED
#define RASCON_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

/*****************************************************************************
 * RASCON.H : RASter CONtouring routines for X displays. Provides contour lines
 * and filled contours for both regular, rectangular and irrigular,
 * non-rectangular grids. Based on a simple, fast contouring
 * algorithm which draws all contours that exist between
 * four points (a "box"). Only two rows of data points need be in memory
 * at a time. This algorithm was designed for extremely
 * large data sets which could not be loaded into memory. Since the early 80's
 * When a 1Mb of RAM was $2-5k, this has become less important.
 * Handles Char,Int,Float,and Double type data.  
 * F. Hage  Jan 1991 NCAR - RAP, from Work originally done at UNC, 1986
 * Modified for Variable Type data. F. Hage,  Mar 1993.
 * 2 Row contouring and Filled Contouring -  F Hage. July 1994
 * Added smoothing fuctions for "blocky" low precision data - Oct 1999
 */

/* ======================= include files ========================= */
#include <stdio.h>
   /*#include <sys/param.h> */
#include <sys/types.h>
#include <math.h>
#include <X11/Xlib.h>    /* X11 */

/* ========================= defines ============================= */
#define RASCON_LINE_CONTOURS 0
#define RASCON_FILLED_CONTOURS 1
#define RASCON_BOX_OUTLINE 2

/* This is now a bitwise "features" control flag */
#define RASCON_NOLABELS 0      /* For backwards compatibility - Depreciated */
#define RASCON_DOLABELS 1
#define RASCON_DOSMOOTH 2      /* Apply a smoothing function before contoring */
#define RASCON_DOSMOOTH2 4     /* Smooth twice */
#define RASCON_DATA_IS_LOG 8   /* Data has been transformed using log()  */
#define RASCON_ADDNOISE 16    /* Apply small amount of noise to char data */

#define RASCON_CHAR     1       /* For data typing */
#define RASCON_SHORT    2
#define RASCON_INT      3
#define RASCON_LONG     3
#define RASCON_FLOAT    4
#define RASCON_DOUBLE   5


/* ======================== structures =========================== */
typedef struct { /* Bounding box for row (pixels), where (0,0)
                is the upperleft corner of the window/drawable.
              */
  int x1; /* top row (X,Y) */
  int y1;
  int x2; /* bottom row (X,Y) */
  int y2;
} RASCONBoxEdge;

/* ===================== external function prototypes ============ */

/* Several options are available for contouring grids;
 *  The "original" routines which act on an entire 2D grid and the "new"
 *  routine which acts on 2 rows of a grid at a time.  Both flavors support
 *  line contours and filled contours. The "original" routines are
 *  faster but need data on a orthoginal grid. IE. They assume the grid cells
 *  are rectangular in shape. The grid may have different shape/size rectangles,
 *  but the Y  dimension along a row or the X dimension along a column must remain constant.
 *  Another way to put this is; the grid must be able to be drawn with straight lines.
 *  The lines must be exactly parallel to the edges of the pixmap, but need not be regularly spaced.
 *  As long as you can draw a straight line through each row and column, things work.
 *
 *  The "new, 2 Row" routine should be used for grids that have non rectangular shapes.
 *  These routines rely on  grid cell boundries, in pixmap coordinates, to determine
 *  the shape of each cell. The cells must tessellate for the whole grid to
 *  appear correct, with no gaps.
 *  The "new" routine may also be more appropriate for extreemly large grids.
 *
 *  Each Contour level has an associated GC with it to allow different line and fill styles,
 *  colors, patterns, Logical operations, and plane masks.
 *  
 *
 *   The "original" routines are:
 *
 *   RASCONinit()
 *   RASCONcontour()
 *
 *   The "new" routine is:
 *
 *   RASCONcontour2Rows()
*/


#ifdef __STDC__
extern int RASCONcontour2Rows(Display *disp,Drawable xid, GC *draw_gc,
		  int num_points_along_x, int rownum,
                  caddr_t row1_data, 
                  caddr_t row2_data, caddr_t bad_data,int data_type, int nlevels, 
                  double *levels,
                  RASCONBoxEdge *bedge, int fill_flag, int do_extras,
                  double label_scale,double label_bias);
#endif

    /* Input:
    *       Display    disp;   The X window Display to render to
    *       Drawable xid:      The Pixmap to draw into
    *       GC *draw_gc:       Graphics context array to use, dimensioned to [nlevels]
    *       int num_points_along_x: The number of data points along x in the rowX arrays
    *       int rownum;        Number of current row to contour (row1)
    *       caddr_t row1_data: Pointer to first row of data, dimensioned to [ncols]
    *       caddr_t row2_data: Pointer to second row of data, dimensioned to [ncols]
    *       caddr_t bad_data:     Value indicating bad or missing data
    *       int    data_type:  An Enumerated data type.
    *       int nlevels:       Number of contour levels to draw
    *       double *levels:    Values to draw contours at, dimensioned to [nlevels]
    *       RASCONBoxEdge *bedge: Array of x,y locations for row to be drawn (pixels),
    *                             dimensioned to [num_points_along_x
    *       int fill_flag:     flag to draw line or filled contours, flag values are:
    *                              RASCON_LINE_CONTOURS or  RASCON_FILLED_CONTOURS
    *       int do_extras:     flag to turn on extra features like draw labels on
    *                            contour lines, flag values are a bitwise OR'ing of:
    *                              RASCON_DOLABELS or RASCON_DOSMOOTH
    *      double label_scale: amount to scale labels by
    *      double label_bias:   amount to bias labels by
    *
    *       Contours one row of cells bounded by two rows of data.  A "cell" is defined
    *       as the area surrounded by four data points. The location of all data points in the
    *       two arrays of data are specified using the RASCONBoxEdge structures.
    *       To contour an entire grid, each row of cells must be passed in using the
    *       row1_data[] and row2_data[] arrays. Except for the first and last row of cells,
    *       each row of data points is used twice; once as the upper row1_data and then again as
    *       the lower row2_data. Remember you can only contour a cell of it has four good data
    *       points surrounding it.
    *
    *       The associated levels[] and draw_gc[] are used to define the values and the line or
    *       fill patterns/colors for each contour. Line contours are drawn using XDrawLine().
    *       Filled contours are drawn using XFillPolygon(), assuming complex polygons. 
    *
    *       A very simplistic, and crude labeling algorithm is used.
    *
    *       Returns -1 on error.
    *
    *       The bedge[] entries define the bounds of each grid box (in pixels) to
    *       be drawn for the input data.
    *
    *   bedge[0]    bedge[1]    bedge[...]  bedge[ncols-1]
    *  x1,y1       x1,y1       x1,y1       x1,y1
    *   -----------------------------------|
    *    |           |           |          |
    *    |           |           |          |
    *    |           |           |          |
    *    |           |           |          |
    *    -----------------------------------|
    *  x2,y2       x2,y2       x2,y2       x2,y2
    *
    * returns -1 on error.
    */

#ifdef __STDC__
extern int RASCONinit(Display *disp, Drawable draw_xid, int ncols, int nrows, int *x_grid, int *y_grid);
#endif

    /* Input:
    *       Display *disp: X display to draw to
    *       Drawable draw_xid: Window or pixmap to draw to
    *       int ncols:  number of data points in x direction
    *       int nrows:  number of data points in Y direction
    *       int *x_grid: X Screen coordinates in display window for drawing contouring grid (in pixels)
    *       int *y_grid: Y Screen coordinates in display window for drawing contouring grid (in pixels)
    *
    *      Initialize the data grid mapping to pixels from cell unit normal space.
    *      Save global variables. Must be called before RASCONcontour()
    */

#ifdef __STDC__
extern int RASCONcontour(GC *draw_gc, void * data, void * bad_data, int data_type,
             int nlevels, double *levels,
             int fill_flag, int do_extras,double label_scale,double label_bias);
#endif

    /* Input:
    *       GC *draw_gc:        Graphics context array to use, dimensioned to [nlevels]
    *       caddr_t data: pointer to 2-D array of data, dimensioned to [ncols * nrows]
    *       caddr_t bad_data: Value indicating bad or missing data
    *       int data_type;  The Enumerated type of data to be contoured.
    *       int nlevels:   Number of contour levels to draw
    *       double *levels:  Values to draw contours at, dimensioned to [nlevels]
    *       int fill_flag:     flag to draw line or filled contours, flag values are:
    *                              RASCON_LINE_CONTOURS or  RASCON_FILLED_CONTOURS
    *       int do_extras:     bitwise flag to draw labels on contour lines, flag values are:
    *                              RASCON_DOLABELS or RASCON_DOSMOOTH
    *      double label_scale: amount to scale labels by
    *      double label_bias: amount to scale labels by
    *
    *    Contours the input data[] array using the specified levels[]. The grid
    *    boxes are drawn in the region specified in RASCONinit().
    *
    *    The associated levels[] and draw_gc[] are used to define the value and the line or
    *    fill patterns/colors for each contour. Line contours are drawn using XDrawLine().
    *    Filled contours are drawn using XFillPolygon(). The display and drawable
    *    defined in RASCONinit() are used to draw to.
    *
    *    A very simplistic, and crude labeling algorithm is used.
    *    Returns -1 on error.
    *
    */

#ifdef __cplusplus
}
#endif
#endif
