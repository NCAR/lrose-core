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
#ifdef __cplusplus
 extern "C" {
#endif
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:15:01 $
 *   $Id: symprod.h,v 1.24 2016/03/03 18:15:01 dixon Exp $
 *   $Revision: 1.24 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/******************************************************************
 * symprod.h: header file for the symbolic products library.
 *
 * This header file describes the buffers and messages used by the
 * symbolic products library.  The idea of this library is to use
 * symbolic product information in a common format so that displays
 * and other processes can operate on and render a single format.
 * The product information is generally stored in an SPDB database
 * in some native format.  Then, a server is written using the
 * SpdbServer class and an output transformation is included to
 * convert the native format to SYMPROD format for sending to the
 * display or other process (like the product selector).
 *
 ******************************************************************/

#ifndef symprod_h
#define symprod_h

#include <stdio.h>
#include <time.h>

#include <dataport/port_types.h>
#include <toolsa/membuf.h>


#define SYMPROD_LABEL_LEN        80
#define SYMPROD_COLOR_LEN        32
#define SYMPROD_ICON_NAME_LEN    32
#define SYMPROD_FONT_NAME_LEN    80


/*****************************************************************************
 * Error values returned by symprod routines.
 */

typedef enum
{
  SYMPROD_SUCCESS,

  SYMPROD_UNKNOWN_ERROR,

  SYMPROD_WRITE_ERROR,
  SYMPROD_READ_ERROR,
  SYMPROD_FILE_OPEN_ERROR
} symprod_error_t;


/*****************************************************************************
 * Defines for field values.
 */

#define SYMPROD_OBJ_TEXT                        1
#define SYMPROD_OBJ_POLYLINE                    2
#define SYMPROD_OBJ_STROKED_ICON                3
#define SYMPROD_OBJ_NAMED_ICON                  4
#define SYMPROD_OBJ_BITMAP_ICON                 5
#define SYMPROD_OBJ_ARC                         6
#define SYMPROD_OBJ_RECTANGLE                   7
#define SYMPROD_OBJ_CHUNK                       8

#define SYMPROD_LINETYPE_SOLID                  1
#define SYMPROD_LINETYPE_DASH                   2
#define SYMPROD_LINETYPE_DOT_DASH               3

#define SYMPROD_CAPSTYLE_BUTT                   1
#define SYMPROD_CAPSTYLE_NOT_LAST               2
#define SYMPROD_CAPSTYLE_PROJECTING             3
#define SYMPROD_CAPSTYLE_ROUND                  4

#define SYMPROD_JOINSTYLE_BEVEL                 1
#define SYMPROD_JOINSTYLE_MITER                 2
#define SYMPROD_JOINSTYLE_ROUND                 3

#define SYMPROD_LINE_INTERP_STRAIGHT            1
#define SYMPROD_LINE_INTERP_BEZIER              2
#define SYMPROD_LINE_INTERP_CUBIC_SPLINE        3

#define SYMPROD_FILL_NONE                       1
#define SYMPROD_FILL_STIPPLE10                  2
#define SYMPROD_FILL_STIPPLE20                  3
#define SYMPROD_FILL_STIPPLE30                  4
#define SYMPROD_FILL_STIPPLE40                  5
#define SYMPROD_FILL_STIPPLE50                  6
#define SYMPROD_FILL_STIPPLE60                  7
#define SYMPROD_FILL_STIPPLE70                  8
#define SYMPROD_FILL_STIPPLE80                  9
#define SYMPROD_FILL_STIPPLE90                 10
#define SYMPROD_FILL_SOLID                     11

#define SYMPROD_TEXT_VERT_ALIGN_TOP             1
#define SYMPROD_TEXT_VERT_ALIGN_CENTER          2
#define SYMPROD_TEXT_VERT_ALIGN_BOTTOM          3

#define SYMPROD_TEXT_HORIZ_ALIGN_LEFT           1
#define SYMPROD_TEXT_HORIZ_ALIGN_CENTER         2
#define SYMPROD_TEXT_HORIZ_ALIGN_RIGHT          3


/*****************************************************************************
 * Defines for penup values.
 */

#define SYMPROD_PPT_PENUP          0x7FFFFFFF
#define SYMPROD_WPT_PENUP          0x7FFFFFFF


/*****************************************************************************
 * Typedefs used for the product buffer.  The product buffer looks like this:
 *
 *            product header
 *            object 1 header
 *            object 1 information
 *            object 2 header
 *            object 2 information
 *                   .
 *                   .
 *            object n header
 *            object n information
 */


typedef struct world_point
{
  fl32        lat;
  fl32        lon;
} symprod_wpt_t;

typedef struct pixel_point
{
  si32        x;
  si32        y;
} symprod_ppt_t;

typedef struct box
{
  fl32        min_lat;
  fl32        max_lat;
  fl32        min_lon;
  fl32        max_lon;
} symprod_box_t;


typedef struct prod_hdr
{
  si32        generate_time;          /* Time product was generated.         */
  si32        received_time;          /* Time product was received.          */
  si32        start_time;             /* Time product starts being valid.    */
  si32        expire_time;            /* Time product expires.               */
  symprod_box_t                       /* Bounding box for displayable        */
    bounding_box;                     /*   objects in product.               */
  si32        num_objs;               /* Number of objects making up the     */
                                      /*   product.                          */
  si32        spare;                  /* Spare field for alignment purposes. */
  char        label[SYMPROD_LABEL_LEN];
                                      /* Optional product label to be        */
                                      /*   displayed in the select list for  */
                                      /*   this instance of the product.     */
  si32        offsets[1];             /* Array of byte offsets into the      */
                                      /*   buffer for each object in the     */
                                      /*   product. There are at least       */
                                      /*   num_objs of these, but there may  */
                                      /*   be extras for alignment purposes  */
                                      /*   or for ease of coding.  Should be */
                                      /*   an even number of these so that   */
                                      /*   things will be aligned properly   */
                                      /*   when sending buffers across       */
                                      /*   architectures.                    */
} symprod_prod_hdr_t;

typedef struct
{
  si32        generate_time;
  si32        received_time;
  si32        start_time;
  si32        expire_time;
  symprod_box_t bounding_box;
  si32        num_objs;
  si32        spare;
  ui08        label[SYMPROD_LABEL_LEN];
} symprod_const_prod_hdr_t;


typedef struct obj_hdr
{
  si32        object_type;            /* Type of object SYMPROD_OBJ_xxx      */
  si32        object_data;            /* Object data.  This field gives more */
                                      /*   identifying info about the object */
                                      /*   that is specific to the creating  */
                                      /*   algorithm.                        */
  si32        num_bytes;              /* Number of bytes in object structure */
                                      /*   excluding object header.          */
  si32        detail_level;           /* Suggested detail level for          */
                                      /*   rendering object.                 */
  char        color[SYMPROD_COLOR_LEN];
                                      /* Suggested foreground color for      */
                                      /*   rendering object.                 */
  char        background_color[SYMPROD_COLOR_LEN];
                                      /* Suggested background color for      */
                                      /*   rendering object.                 */
  symprod_wpt_t centroid;             /* Object centroid, used for picking   */
                                      /*   the object.                       */
} symprod_obj_hdr_t;


/*****************************************************************************
 * Typedefs for individual objects.
 */


typedef struct text_obj
{
  symprod_wpt_t        origin;           /* World coordinate origin of text. */
  symprod_ppt_t        offset;           /* Pixel coordinate offset of text. */
  si32                 vert_alignment;   /* Text vertical alignment          */
                                         /*   SYMPROD_TEXT_VERT_xxx.         */
  si32                 horiz_alignment;  /* Text horizontal alignment        */
                                         /*   SYMPROD_TEXT_HORIZ_xxx.        */
  si32                 size;             /* An indication of the importance  */
                                         /*   of this text string to give    */
                                         /*   the display a hint about what  */
                                         /*   font to use in rendering it.   */
  si32                 length;           /* Text length, not including NULL. */
  char                 fontname[SYMPROD_FONT_NAME_LEN];
                                         /* The name of the font to be used  */
                                         /*   in rendering the text.         */
  char                 string[1];        /* Variable length text string to   */
                                         /*   display, at least length       */
                                         /*   bytes. Can be longer for       */
                                         /*   alignment purposes.            */
} symprod_text_obj_t;

typedef struct
{
  symprod_wpt_t        origin;
  symprod_ppt_t        offset;
  si32                 vert_alignment;
  si32                 horiz_alignment;
  si32                 size;
  si32                 length;
  char                 fontname[SYMPROD_FONT_NAME_LEN];
} symprod_const_text_obj_t;


typedef struct polyline_obj
{
  si32                 close_flag;       /* Flag indicating if the polyline  */
                                         /*   should be closed (i.e. should  */
                                         /*   be a polygon).                 */
  si32                 fill;             /* Polygon fill information         */
                                         /*   SYMPROD_FILL_xxx, only used if */
                                         /*   close_flag is TRUE.            */
  si32                 linetype;         /* Suggested line type for          */
                                         /*   rendering polyline             */
                                         /*   SYMPROD_LINETYPE_xxx.          */
  si32                 linewidth;        /* Suggested line width for         */
                                         /*   rendering polyline.            */
  si32                 capstyle;         /* Suggested cap style for          */
                                         /*   rendering polyline             */
                                         /*   SYMPROD_CAPSTYLE_xxx.          */
  si32                 joinstyle;        /* Suggested join style for         */
                                         /*   rendering polyline             */
                                         /*   SYMPROD_JOINSTYLE_xxx.         */
  si32                 line_interp;      /* Suggested line interpolation     */
                                         /*   method for rendering polyline  */
                                         /*   SYMPROD_LINE_INTERP_xxx.       */
  si32                 num_points;       /* Number of points in polyline.    */
  symprod_wpt_t        points[1];        /* Variable array of polyline pts.  */
                                         /*   There should be num_points of  */
                                         /*   these.                         */
} symprod_polyline_obj_t;

typedef struct
{
  si32                 close_flag;
  si32                 fill;
  si32                 linetype;
  si32                 linewidth;
  si32                 capstyle;
  si32                 joinstyle;
  si32                 line_interp;
  si32                 num_points;
} symprod_const_polyline_obj_t;


typedef struct stroked_icon_obj
{
  si32             num_icon_pts;         /* Number of points making up the   */
                                         /*   icon.                          */
  si32             num_icons;            /* Number of icons to display.      */
  symprod_ppt_t    icon_pts[1];          /* Points making up the icon.       */
                                         /*   There should be num_icon_pts   */
                                         /*   of these.                      */
/*  symprod_wpt_t    icon_origins[1]; */ /* Locations for the icons.  There  */
                                         /*   should be num_icons of these.  */
} symprod_stroked_icon_obj_t;

typedef struct
{
  si32             num_icon_pts;
  si32             num_icons;
} symprod_const_stroked_icon_obj_t;


typedef struct named_icon_obj
{
  ui08             name[SYMPROD_ICON_NAME_LEN];
                                         /* Name of icon to display.         */
  si32             num_icons;            /* Number of icons to display.      */
  si32             spare;                /* Spare field for alignment.       */
  symprod_wpt_t    icon_origins[1];      /* Locations for the icons.  There  */
                                         /*   should be num_icons of these.  */
} symprod_named_icon_obj_t;

typedef struct
{
  ui08             name[SYMPROD_ICON_NAME_LEN];
  si32             num_icons;
  si32             spare;
} symprod_const_named_icon_obj_t;


typedef struct bitmap_icon_obj
{
  si32             bitmap_x_dim;         /* Number of bits in the bitmap in  */
                                         /*   the X direction.               */
  si32             bitmap_y_dim;         /* Number of bits in the bitmap in  */
                                         /*   the Y direction.               */
  si32             num_icons;            /* Number of icons to display.      */
  si32             spare;                /* Spare field for alignment.       */
  symprod_wpt_t    icon_origins[1];      /* Locations for the icons.  There  */
                                         /*   should be num_icons of these.  */
/*  ui08             bitmap[1];  */      /* Bitmap for the icon.  There      */
                                         /*   should at least be             */
                                         /*   bitmap_x_dim * bitmap_y_dim    */
                                         /*   values, each interpreted as    */
                                         /*   TRUE or FALSE. There may be    */
                                         /*   more values for alignment      */
                                         /*   purposes.                      */
} symprod_bitmap_icon_obj_t;

typedef struct
{
  si32             bitmap_x_dim;
  si32             bitmap_y_dim;
  si32             num_icons;
  si32             spare;
} symprod_const_bitmap_icon_obj_t;


typedef struct arc_obj
{
  symprod_wpt_t   origin;                /* Center of rectangle containing   */
                                         /*   the arc.                       */
  si32            height;                /* Height of rectangle containing   */
                                         /*   the arc.                       */
  si32            width;                 /* Width of rectangle containing    */
                                         /*   the arc.                       */
  fl32            angle1;                /* Start of arc relative to the     */
                                         /*   3 o'clock position (counter-   */
                                         /*   clockwise) in degrees.         */
  fl32            angle2;                /* Path and extent of arc relative  */
                                         /*   to the starting position       */
                                         /*   (counter-clockwise) in degrees */
  si32            linetype;              /* Suggested line type for          */
                                         /*   rendering arc                  */
                                         /*   SYMPROD_LINETYPE_xxx.          */
  si32            linewidth;             /* Suggested line width for         */
                                         /*   rendering arc.                 */
  si32            fill;                  /* Arc fill information             */
                                         /*   SYMPROD_FILL_xxx.              */
  si32            capstyle;              /* Suggested cap style for          */
                                         /*   rendering arc                  */
                                         /*   SYMPROD_CAPSTYLE_xxx.          */
  si32            joinstyle;             /* Suggested join style for         */
                                         /*   rendering arc                  */
                                         /*   SYMPROD_JOINSTYLE_xxx.         */
  si32            spare;                 /* Spare field for alignment.       */
} symprod_arc_obj_t;

typedef struct rectangle_obj
{
  symprod_wpt_t   origin;                /* Lower left hand corner of        */
                                         /*   rectangle.                     */
  si32            height;                /* Height of rectangle.             */
  si32            width;                 /* Width of rectangle.              */
  si32            linetype;              /* Suggested line type for          */
                                         /*   rendering rectangle            */
                                         /*   SYMPROD_LINETYPE_xxx.          */
  si32            linewidth;             /* Suggested line width for         */
                                         /*   rendering rectangle.           */
  si32            fill;                  /* Fill information for rectangle   */
                                         /*   SYMPROD_FILL_xxx.              */
  si32            capstyle;              /* Suggested cap style for          */
                                         /*   rendering rectangle            */
                                         /*   SYMPROD_CAPSTYLE_xxx.          */
  si32            joinstyle;             /* Suggested join style for         */
                                         /*   rendering rectangle            */
                                         /*   SYMPROD_JOINSTYLE_xxx.         */
  si32            spare;                 /* Spare field for alignment.       */
} symprod_rectangle_obj_t;

typedef struct chunk_obj
{
  si32            chunk_type;            /* Chunk type identifier.  This can */
                                         /*   be used to identify the type   */
                                         /*   of data in the chunk.          */
  si32            user_data;             /* Chunk data that can be used by   */
                                         /*   the client for any purpose.    */
  ui08            data[1];               /* Chunk data.  Should be at least  */
                                         /*   obj_hdr.num_bytes long. Could  */
                                         /*   be longer for alignment        */
                                         /*   purposes.                      */
} symprod_chunk_obj_t;


typedef struct
{
  si32            chunk_type;
  si32            user_data;
} symprod_const_chunk_obj_t;


/*****************************************************************************
 * Typedefs for internal representations of products and objects.
 */

typedef union
{
  symprod_text_obj_t           text;
  symprod_polyline_obj_t       polyline;
  symprod_stroked_icon_obj_t   stroked_icon;
  symprod_named_icon_obj_t     named_icon;
  symprod_bitmap_icon_obj_t    bitmap_icon;
  symprod_arc_obj_t            arc;
  symprod_rectangle_obj_t      rectangle;
  symprod_chunk_obj_t          chunk;
} symprod_object_union_t;

typedef struct object
{
  int                  object_type;
  int                  object_data;
  int                  num_bytes;         /* Number of bytes in object       */
                                          /*   information (object_info      */
                                          /*   below).                       */
  int                  detail_level;
  char                 color[SYMPROD_COLOR_LEN];
  char                 background_color[SYMPROD_COLOR_LEN];
  symprod_wpt_t        centroid;
  symprod_object_union_t *object_info;
} symprod_object_t;


typedef struct product
{

  time_t        generate_time;
  time_t        received_time;
  time_t        start_time;
  time_t        expire_time;

  symprod_box_t bounding_box;
  
  int           num_objs;
  int           num_objs_alloc;

  char          *label;
  
  MEMbuf        *mbuf_obj;        /* MEMbuf for array of
				   * symprod_object_t structures */
} symprod_product_t;

typedef struct
{
  time_t        generate_time;
  time_t        received_time;
  time_t        start_time;
  time_t        expire_time;
  symprod_box_t bounding_box;
  
  int           num_objs;
  int           num_objs_alloc;

  char          *label;
} symprod_const_product_t;


/******************************************************************
 * Prototypes for exported functions.
 */


/**********************************************************************
 * GENERAL FUNCTIONS
 **********************************************************************/

/**********************************************************************
 * SYMPRODcreateProduct() - Create an internal product structure.
 *                          Allocates space for the returned structure
 *                          which must be freed by the calling routine
 *                          using SYMPRODfreeProduct().
 */

symprod_product_t *SYMPRODcreateProduct(time_t generate_time,
					time_t received_time,
					time_t start_time,
					time_t expire_time);

/**********************************************************************
 * SYMPRODaddLabel() - Adds the optional label to a product.
 */

void SYMPRODaddLabel(symprod_product_t *prod,
		     const char *label);

/**********************************************************************
 * SYMPRODaddObject() - Adds the given object to the internal product
 *                      structure.
 *
 * Memory is allocated for the object info. This memory is freed
 * in SYMPRODfreeProduct().
 */

void SYMPRODaddObject(symprod_product_t *product,
		      int type,
		      int object_data,
		      int detail_level,
		      const char *color,
		      const char *background_color,
		      symprod_wpt_t *centroid,
		      symprod_box_t *bounding_box,
		      symprod_object_union_t *object_info,
		      int num_bytes);

/**********************************************************************
 * SYMPRODaddArrowBothPts()
 *
 * Add an arrow object to the internal product structure.
 *
 * Geometry inputs: both start and end points
 *
 * head_len:        in km
 * head_half_angle: in deg
 */

extern void SYMPRODaddArrowBothPts(symprod_product_t *prod,
				   const char *color,
				   int linetype,
				   int linewidth,
				   int capstyle,
				   int joinstyle,
				   double start_lat,
				   double start_lon,
				   double end_lat,
				   double end_lon,
				   double head_len,
				   double head_half_angle);

/**********************************************************************
 * SYMPRODaddArrowStartPt()
 *
 * Add an arrow object to the internal product structure.
 *
 * Geometry inputs: start pt, length, dirn
 *
 * head_len:        in km
 * head_half_angle: in deg
 */

extern void SYMPRODaddArrowStartPt(symprod_product_t *prod,
				   const char *color,
				   int linetype,
				   int linewidth,
				   int capstyle,
				   int joinstyle,
				   double start_lat,
				   double start_lon,
				   double length,
				   double dirn,
				   double head_len,
				   double head_half_angle);

/**********************************************************************
 * SYMPRODaddArrowEndPt()
 *
 * Add an arrow object to the internal product structure.
 *
 * Geometry inputs: end pt, length, dirn
 *
 * head_len:        in km
 * head_half_angle: in deg
 */

extern void SYMPRODaddArrowEndPt(symprod_product_t *prod,
				 const char *color,
				 int linetype,
				 int linewidth,
				 int capstyle,
				 int joinstyle,
				 double end_lat,
				 double end_lon,
				 double length,
				 double dirn,
				 double head_len,
				 double head_half_angle);

/**********************************************************************
 * SYMPRODaddArrowMidPt()
 *
 * Add an arrow object to the internal product structure.
 *
 * Geometry inputs: mid pt, length, dirn
 *
 * head_len:        in km
 * head_half_angle: in deg
 */

extern void SYMPRODaddArrowMidPt(symprod_product_t *prod,
				 const char *color,
				 int linetype,
				 int linewidth,
				 int capstyle,
				 int joinstyle,
				 double mid_lat,
				 double mid_lon,
				 double length,
				 double dirn,
				 double head_len,
				 double head_half_angle);
     
/*********************************************************************
 * SYMPRODaddBitmapIcon()
 *
 * Add a SYMPROD bitmap icon object to the product buffer.
 */

void SYMPRODaddBitmapIcon(symprod_product_t *prod,
			  const char *color,
			  int bitmap_x_dim,
			  int bitmap_y_dim,
			  int num_icons,
			  symprod_wpt_t *icon_origins,
			  ui08 *bitmap);

/*********************************************************************
 * SYMPRODaddChunk()
 *
 * Add a SYMPROD chunk object to the product buffer.
 */

extern void SYMPRODaddChunk(symprod_product_t *prod,
			    const char *color,
			    const char *background_color,
			    double min_lat,
			    double min_lon,
			    double max_lat,
			    double max_lon,
			    int chunk_type,
			    int user_data,
			    char *data,
			    int data_len);

/*********************************************************************
 * SYMPRODaddPolyline()
 *
 * Add a SYMPROD polyline object to the product buffer.
 */

extern void SYMPRODaddPolyline(symprod_product_t *prod,
			       const char *color,
			       int linetype,
			       int linewidth,
			       int capstyle,
			       int joinstyle,
			       int close_flag,
			       int fill,
			       symprod_wpt_t *pts,
			       int npoints);

/*********************************************************************
 * SYMPRODaddStrokedIcon()
 *
 * Add a SYMPROD stroked icon object to the product buffer.
 */

void SYMPRODaddStrokedIcon(symprod_product_t *prod,
			   const char *color,
			   int num_icon_pts,
			   symprod_ppt_t *icon_pts,
			   int num_icons,
			   symprod_wpt_t *icon_origins);

/*********************************************************************
 * SYMPRODaddText()
 *
 * Add a SYMPROD text object to the product buffer.
 */

extern void SYMPRODaddText(symprod_product_t *prod,
			   const char *color,
			   const char *background_color,
			   double lat, double lon,
			   int offset_x, int offset_y,
			   int vert_alignment,
			   int horiz_alignment,
			   int size,
			   const char *fontname,
			   const char *text_string);

/**********************************************************************
 * SYMPRODproductToBuffer() - Convert internal format to product buffer.
 *                            The buffer will be returned in
 *                            native format (any necessary byte-
 *                            swapping must be done later).
 */

char *SYMPRODproductToBuffer(symprod_product_t *prod,
			     int *buflen);

/**********************************************************************
 * SYMPRODbufferToProduct() - Convert a product buffer to internal
 *                            format.  The buffer is assumed to be in
 *                            native format (any necessary byte-
 *                            swapping has already been done).
 */

symprod_product_t *SYMPRODbufferToProduct(char *buffer);

/**********************************************************************
 * SYMPRODfreeProduct() - Free a product.
 */

void SYMPRODfreeProduct(symprod_product_t *product);


/**********************************************************************
 * PRINTING FUNCTIONS
 **********************************************************************/

/**********************************************************************
 * SYMPRODprintProductHdr() - Prints a product header to the given
 *                            stream in ASCII format.
 */

void SYMPRODprintProductHdr(FILE *stream,
			    symprod_prod_hdr_t *prod_hdr);

/**********************************************************************
 * SYMPRODprintObjectHdr() - Prints an object header to the given
 *                           stream in ASCII format.
 */

void SYMPRODprintObjectHdr(FILE *stream,
			   symprod_obj_hdr_t *obj_hdr);

/**********************************************************************
 * SYMPRODprintProduct() - Print a product structure to the given
 *                         stream in ASCII format.  Also prints all of
 *                         the object structures contained in the
 *                         product structure.
 */

void SYMPRODprintProduct(FILE *stream,
			 symprod_product_t *product);

/**********************************************************************
 * SYMPRODprintProductBuffer() - Print a product buffer to the given
 *                               stream in ASCII format.
 */

void SYMPRODprintProductBuffer(FILE *stream,
			       char *product_buffer);

/**********************************************************************
 * SYMPRODprintObject() - Print an object structure to the given stream
 *                        in ASCII format.
 */

void SYMPRODprintObject(FILE *stream,
			symprod_object_t *object);

/**********************************************************************
 * SYMPRODprintTextObject() - Print a text object to the given stream
 *                            in ASCII format.
 */

void SYMPRODprintTextObject(FILE *stream,
			    symprod_text_obj_t *object);

/**********************************************************************
 * SYMPRODprintPolylineObject() - Print a polyline object to the given
 *                                stream in ASCII format.
 */

void SYMPRODprintPolylineObject(FILE *stream,
				symprod_polyline_obj_t *object);

/**********************************************************************
 * SYMPRODprintStrokedIconObject() - Print a stroked icon object to the
 *                                   given stream in ASCII format.
 */

void SYMPRODprintStrokedIconObject(FILE *stream,
				   symprod_stroked_icon_obj_t *object);

/**********************************************************************
 * SYMPRODprintNamedIconObject() - Print a named icon object to the
 *                                 given stream in ASCII format.
 */

void SYMPRODprintNamedIconObject(FILE *stream,
				 symprod_named_icon_obj_t *object);

/**********************************************************************
 * SYMPRODprintBitmapIconObject() - Print a bitmap icon object to the
 *                                  given stream in ASCII format.
 */

void SYMPRODprintBitmapIconObject(FILE *stream,
				  symprod_bitmap_icon_obj_t *object);

/**********************************************************************
 * SYMPRODprintArcObject() - Print an arc object to the given stream in
 *                           ASCII format.
 */

void SYMPRODprintArcObject(FILE *stream,
			   symprod_arc_obj_t *object);

/**********************************************************************
 * SYMPRODprintRectangleObject() - Print a rectangle object to the given
 *                                 stream in ASCII format.
 */

void SYMPRODprintRectangleObject(FILE *stream,
				 symprod_rectangle_obj_t *object);

/**********************************************************************
 * SYMPRODprintChunkObject() - Print a chunk object to the given stream
 *                             in ASCII format.  Currently, the chunk
 *                             data in the object is printed as a string.
 */

void SYMPRODprintChunkObject(FILE *stream,
			     symprod_chunk_obj_t *object);


/**********************************************************************
 * BOUNDING BOX FUNCTIONS
 **********************************************************************/

/*********************************************************************
 * SYMPRODinitBbox()
 *
 * Initialize bounding box
 */

extern void SYMPRODinitBbox(symprod_box_t *bb);

/*********************************************************************
 * SYMPRODupdateBboxPoint()
 *
 * Update a bounding box with a point
 */

extern void SYMPRODupdateBboxPoint(symprod_box_t *bb,
				   double lat, double lon);

/*********************************************************************
 * SYMPRODupdateBbox()
 *
 * Update one bounding box with another
 */

extern void SYMPRODupdateBbox(symprod_box_t *master_bb,
			      symprod_box_t *bb);

/**********************************************************************
 * SWAPPING FUNCTIONS
 **********************************************************************/

/**********************************************************************
 * SYMPRODproductBufferToBE() - Convert a product buffer to big-endian
 *                              format for writing to disk or trans-
 *                              mission across the network.
 */

void SYMPRODproductBufferToBE(char *buffer);

/**********************************************************************
 * SYMPRODproductBufferFromBE() - Convert a product buffer from big-
 *                                endian format after reading from disk
 *                                or transmission across the network.
 */

void SYMPRODproductBufferFromBE(char *buffer);

/**********************************************************************
 * SYMPRODprodHdrToBE() - Convert a product header structure to big-
 *                        endian format for writing to disk or trans-
 *                        mission across the network.
 */

void SYMPRODprodHdrToBE(char *buffer);

/**********************************************************************
 * SYMPRODprodHdrFromBE() - Convert a product header structure from big-
 *                          endian format after reading from disk or
 *                          transmission across the network.
 */

void SYMPRODprodHdrFromBE(char *buffer);

/**********************************************************************
 * SYMPRODobjHdrToBE() - Convert an object header structure to big-
 *                       endian format for writing to disk or trans-
 *                       mission across the network.
 */

void SYMPRODobjHdrToBE(char *buffer);

/**********************************************************************
 * SYMPRODobjHdrFromBE() - Convert an object header structure from big-
 *                         endian format after reading from disk or
 *                         transmission across the network.
 */

void SYMPRODobjHdrFromBE(char *buffer);

/**********************************************************************
 * SYMPRODobjTextToBE() - Convert a text object structure to big-
 *                        endian format for writing to disk or trans-
 *                        mission across the network.
 */

void SYMPRODobjTextToBE(char *buffer);

/**********************************************************************
 * SYMPRODobjTextFromBE() - Convert a text object structure from big-
 *                          endian format after reading from disk or
 *                          transmission across the network.
 */

void SYMPRODobjTextFromBE(char *buffer);

/**********************************************************************
 * SYMPRODobjPolylineToBE() - Convert a polyline object structure to
 *                            big-endian format for writing to disk or
 *                            transmission across the network.
 */

void SYMPRODobjPolylineToBE(char *buffer);

/**********************************************************************
 * SYMPRODobjPolylineFromBE() - Convert a polyline object structure from
 *                              big-endian format after reading from
 *                              disk or transmission across the network.
 */

void SYMPRODobjPolylineFromBE(char *buffer);

/**********************************************************************
 * SYMPRODobjStrokedIconToBE() - Convert a stroked icon object
 *                               structure to big-endian format for
 *                               writing to disk or transmission across
 *                               the network.
 */

void SYMPRODobjStrokedIconToBE(char *buffer);

/**********************************************************************
 * SYMPRODobjStrokedIconFromBE() - Convert a stroked icon object
 *                                 structure from big-endian format
 *                                 after reading from disk or trans-
 *                                 mission across the network.
 */

void SYMPRODobjStrokedIconFromBE(char *buffer);

/**********************************************************************
 * SYMPRODobjNamedIconToBE() - Convert a named icon object structure to
 *                             big-endian format for writing to disk or
 *                             transmission across the network.
 */

void SYMPRODobjNamedIconToBE(char *buffer);

/**********************************************************************
 * SYMPRODobjNamedIconFromBE() - Convert a named icon object structure
 *                               from big-endian format after reading
 *                               from disk or transmission across the network.
 */

void SYMPRODobjNamedIconFromBE(char *buffer);

/**********************************************************************
 * SYMPRODobjBitmapIconToBE() - Convert a bitmap icon object structure
 *                              to big-endian format for writing to disk
 *                              or transmission across the network.
 */

void SYMPRODobjBitmapIconToBE(char *buffer);

/**********************************************************************
 * SYMPRODobjBitmapIconFromBE() - Convert a bitmap icon object structure
 *                                from big-endian format after reading
 *                                disk or transmission across the network.
 */

void SYMPRODobjBitmapIconFromBE(char *buffer);

/**********************************************************************
 * SYMPRODobjArcToBE() - Convert an arc object structure to big-endian
 *                       format for writing to disk or transmission 
 *                       across the network.
 */

void SYMPRODobjArcToBE(char *buffer);

/**********************************************************************
 * SYMPRODobjArcFromBE() - Convert an arc object structure from big-
 *                         endian format after reading from disk or
 *                         transmission across the network.
 */

void SYMPRODobjArcFromBE(char *buffer);

/**********************************************************************
 * SYMPRODobjRectangleToBE() - Convert a rectangle object structure to
 *                             big-endian format for writing to disk or
 *                             transmission across the network.
 */

void SYMPRODobjRectangleToBE(char *buffer);

/**********************************************************************
 * SYMPRODobjRectangleFromBE() - Convert a rectangle object structure
 *                               from big-endian format after reading
 *                               from disk or transmission across the
 *                               network.
 */

void SYMPRODobjRectangleFromBE(char *buffer);

/**********************************************************************
 * SYMPRODobjChunkToBE() - Convert a chunk object structure to big-
 *                         endian format for writing to disk or trans-
 *                         mission across the network.  Note that the
 *                         chunk data itself is assumed to already be
 *                         in big-endian format since this library
 *                         doesn't know anything about the data format.
 */

void SYMPRODobjChunkToBE(char *buffer);

/**********************************************************************
 * SYMPRODobjChunkFromBE() - Convert a chunk object structure from big-
 *                           endian format after reading from disk or
 *                           transmission across the network.  Note that
 *                           the chunk data itself is assumed to already
 *                           be in big-endian format since this library
 *                           doesn't know anything about the data format.
 */

void SYMPRODobjChunkFromBE(char *buffer);

/**********************************************************************
 * SYMPRODboxToBE() - Convert a bounding box structure to big-endian
 *                    format for writing to disk or transmission across
 *                    the network.
 */

void SYMPRODboxToBE(char *buffer);

/**********************************************************************
 * SYMPRODboxFromBE() - Convert a bounding box structure from big-endian
 *                      format after reading from disk or transmission
 *                      across the network.
 */

void SYMPRODboxFromBE(char *buffer);

/**********************************************************************
 * SYMPRODwptToBE() - Convert a world coordinate point structure to
 *                    big-endian format for writing to disk or trans-
 *                    mission across the network.
 */

void SYMPRODwptToBE(char *buffer);

/**********************************************************************
 * SYMPRODwptFromBE() - Convert a world coordinate point structure from
 *                      big-endian format after reading from disk or
 *                      transmission across the network.
 */

void SYMPRODwptFromBE(char *buffer);

/**********************************************************************
 * SYMPRODpptToBE() - Convert a pixel coordinate point structure to
 *                    big-endian format for writing to disk or trans-
 *                    mission across the network.
 */

void SYMPRODpptToBE(char *buffer);

/**********************************************************************
 * SYMPRODpptFromBE() - Convert a pixel coordinate point structure from
 *                      big-endian format after reading from disk or
 *                      transmission across the network.
 */

void SYMPRODpptFromBE(char *buffer);


/**********************************************************************
 * TRANSFORMATION FUNCTIONS
 **********************************************************************/

/**********************************************************************
 * SYMPRODpptDistance() - Calculate the distance between two pixel
 *                        points.  The value returned is in pixels.
 */

double SYMPRODpptDistance(symprod_ppt_t point1,
			  symprod_ppt_t point2);

/**********************************************************************
 * SYMPRODrotatePptArray() - Rotate an array of pixel points by the
 *                           given angle.  The array is rotated around
 *                           the point (0,0) and the angle is given in
 *                           radians.
 */

void SYMPRODrotatePptArray(symprod_ppt_t *array,
			   int n_points,
			   double angle_rad);

/**********************************************************************
 * SYMPRODrotateWptArray() - Rotate an array of world points by the
 *                           given angle.  The array is rotated around
 *                           the point (0,0) and the angle is given in
 *                           radians.
 */

void SYMPRODrotateWptArray(symprod_wpt_t *array,
			   int n_points,
			   double angle_rad);

/**********************************************************************
 * SYMPRODscalePptArray() - Scale an array of pixel points by the
 *                          given factor.
 */

void SYMPRODscalePptArray(symprod_ppt_t *array,
			  int n_points,
			  double factor);

/**********************************************************************
 * SYMPRODscaleWptArray() - Scale an array of world points by the
 *                          given factor.
 */

void SYMPRODscaleWptArray(symprod_wpt_t *array,
			  int n_points,
			  double factor);

/**********************************************************************
 * SYMPRODtranslatePptArray() - Translate an array of pixel points by the
 *                              given values.  The values are assumed to
 *                              be in pixels.
 */

void SYMPRODtranslatePptArray(symprod_ppt_t *array,
			      int n_points,
			      int trans_x,
			      int trans_y);

/**********************************************************************
 * SYMPRODtranslateWptArray() - Translate an array of world points by the
 *                              given values.  The values are assumed to
 *                              be in degrees.
 */

void SYMPRODtranslateWptArray(symprod_wpt_t *array,
			      int n_points,
			      double trans_lat,
			      double trans_lon);

/**********************************************************************
 * SYMPRODwptDistance() - Calculate the distance between two world
 *                        points.  The value returned is in degrees.
 */

double SYMPRODwptDistance(symprod_wpt_t point1,
			  symprod_wpt_t point2);


#endif

#ifdef __cplusplus
}
#endif
