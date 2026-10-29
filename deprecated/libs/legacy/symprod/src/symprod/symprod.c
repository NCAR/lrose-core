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
 * symprod.c
 *
 * Routines for manipulating SYMPROD objects.
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * Aug 1995
 *
 *********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <toolsa/os_config.h>
#include <dataport/bigend.h>
#include <symprod/symprod.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/umisc.h>


#define OBJECT_ARRAY_INCR    10

/*
 * prototypes for static functions.
 */

static void add_arrow(symprod_product_t *prod,
		      const char *color,
		      int linetype,
		      int linewidth,
		      int capstyle,
		      int joinstyle,
		      double start_lat,
		      double start_lon,
		      double end_lat,
		      double end_lon,
		      double length,
		      double dirn,
		      double head_len,
		      double head_half_angle);

static void print_object_type(FILE *stream,
			      int type);

static void print_vert_align(FILE *stream,
			     int align);

static void print_horiz_align(FILE *stream,
			      int align);

static void print_fill(FILE *stream,
		       int fill);

static void print_linetype(FILE *stream,
			   int linetype);

static void print_capstyle(FILE *stream,
			   int capstyle);

static void print_joinstyle(FILE *stream,
			    int joinstyle);

static void print_line_interp(FILE *stream,
			      int line_interp);


/**********************************************************************
 * Exported routines.
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
					time_t expire_time)
{

  symprod_product_t *prod;
					
  /*
   * alloc
   */

  prod = (symprod_product_t *) umalloc(sizeof(symprod_product_t));
  MEM_zero(*prod);
  
  /*
   * Initialize the values in the structure.
   */

  prod->generate_time = generate_time;
  prod->received_time = received_time;
  prod->start_time    = start_time;
  prod->expire_time   = expire_time;
  
  SYMPRODinitBbox(&prod->bounding_box);

  prod->num_objs = 0;

  prod->label = (char *)NULL;
  
  /*
   * initialize buffer for objects
   */
  
  prod->mbuf_obj = MEMbufCreate();

  return (prod);

}


/**********************************************************************
 * SYMPRODaddLabel() - Adds the optional label to a product.
 */

void SYMPRODaddLabel(symprod_product_t *prod,
		     const char *label)
{
  if (prod->label != (char *)NULL)
    ufree(prod->label);
  
  prod->label = STRdup(label);
  
  return;
}


/**********************************************************************
 * SYMPRODaddObject() - Adds the given object to the internal product
 *                      structure.
 *
 * Memory is allocated for the object info. This memory is freed
 * in SYMPRODfreeProduct().
 */

void SYMPRODaddObject(symprod_product_t *prod,
		      int type,
		      int object_data,
		      int detail_level,
		      const char *color,
		      const char *background_color,
		      symprod_wpt_t *centroid,
		      symprod_box_t *bounding_box,
		      symprod_object_union_t *object_info,
		      int num_bytes)

{

  symprod_object_t obj;
  
  /*
   * Load the object information.
   */

  memset(&obj, 0, sizeof(obj));
  
  obj.object_type = type;
  obj.object_data = object_data;
  obj.num_bytes = num_bytes;
  obj.detail_level = detail_level;
  STRcopy(obj.color, color, SYMPROD_COLOR_LEN);
  STRcopy(obj.background_color, background_color, SYMPROD_COLOR_LEN);
  obj.centroid = *centroid;

  obj.object_info = (symprod_object_union_t *) umalloc(num_bytes);
  memcpy(obj.object_info, object_info, num_bytes);

  /*
   * add object to the buffer
   */

  MEMbufAdd(prod->mbuf_obj, &obj, sizeof(symprod_object_t));
  prod->num_objs++;
  
  /*
   * update the bounding box
   */
  
  SYMPRODupdateBbox(&prod->bounding_box, bounding_box);

  return;

}


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

void SYMPRODaddArrowBothPts(symprod_product_t *prod,
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
			    double head_half_angle)

{

  double length, dirn;

  PJGLatLon2RTheta(start_lat, start_lon, end_lat, end_lon,
		   &length, &dirn);

  add_arrow(prod, color, linetype, linewidth, capstyle, joinstyle,
	    start_lat, start_lon, end_lat, end_lon,
	    length, dirn,
	    head_len, head_half_angle);

}

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

void SYMPRODaddArrowStartPt(symprod_product_t *prod,
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
			    double head_half_angle)

{

  double end_lat, end_lon;

  PJGLatLonPlusRTheta(start_lat, start_lon, length, dirn,
		      &end_lat, &end_lon);

  add_arrow(prod, color, linetype, linewidth, capstyle, joinstyle,
	    start_lat, start_lon, end_lat, end_lon,
	    length, dirn,
	    head_len, head_half_angle);

}

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

void SYMPRODaddArrowEndPt(symprod_product_t *prod,
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
			  double head_half_angle)

{

  double start_lat, start_lon;
  
  PJGLatLonPlusRTheta(end_lat, end_lon, length, dirn + 180.0,
		      &start_lat, &start_lon);

  add_arrow(prod, color, linetype, linewidth, capstyle, joinstyle,
	    start_lat, start_lon, end_lat, end_lon,
	    length, dirn,
	    head_len, head_half_angle);

}

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

void SYMPRODaddArrowMidPt(symprod_product_t *prod,
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
			  double head_half_angle)
     
{

  double start_lat, start_lon;
  double end_lat, end_lon;

  PJGLatLonPlusRTheta(mid_lat, mid_lon, length / 2.0, dirn,
		      &end_lat, &end_lon);

  PJGLatLonPlusRTheta(mid_lat, mid_lon, length / 2.0, dirn + 180.0,
		      &start_lat, &start_lon);

  add_arrow(prod, color, linetype, linewidth, capstyle, joinstyle,
	    start_lat, start_lon, end_lat, end_lon,
	    length, dirn,
	    head_len, head_half_angle);

}

/*********************************************************************
 * SYMPRODaddBitmapIcon()
 *
 * Add an array of SYMPROD bitmap icons to the product buffer.
 */

void SYMPRODaddBitmapIcon(symprod_product_t *prod,
			  const char *color,
			  int bitmap_x_dim,
			  int bitmap_y_dim,
			  int num_icons,
			  symprod_wpt_t *icon_origins,
			  ui08 *bitmap)
{
  static MEMbuf *mbuf_obj = NULL;

  int ipt;
  symprod_wpt_t centroid;
  symprod_box_t bounding_box;
  symprod_const_bitmap_icon_obj_t bitmap_icon_obj;
  
  /*
   * initialize mbuf
   */

  if (mbuf_obj == NULL)
    mbuf_obj = MEMbufCreate();
  else
    MEMbufReset(mbuf_obj);
  
  /*
   * load bounding box & centroid
   */

  SYMPRODinitBbox(&bounding_box);

  for (ipt = 0; ipt < num_icons; ipt++)
    SYMPRODupdateBboxPoint(&bounding_box,
			   icon_origins[ipt].lat, icon_origins[ipt].lon);
  
  centroid.lat =
    (bounding_box.min_lat + bounding_box.max_lat) / 2.0;
  centroid.lon =
    (bounding_box.min_lon + bounding_box.max_lon) / 2.0;
  
  /*
   * load the bitmap icon object data
   */
       
  bitmap_icon_obj.bitmap_x_dim = bitmap_x_dim;
  bitmap_icon_obj.bitmap_y_dim = bitmap_y_dim;
  bitmap_icon_obj.num_icons = num_icons;
  bitmap_icon_obj.spare = 0;
  
  /*
   * load up object mbuf with obj type followed by data
   */

  MEMbufAdd(mbuf_obj, &bitmap_icon_obj,
	    sizeof(symprod_const_bitmap_icon_obj_t));
  MEMbufAdd(mbuf_obj, icon_origins, num_icons * sizeof(symprod_wpt_t));
  MEMbufAdd(mbuf_obj, bitmap, bitmap_x_dim * bitmap_y_dim);
  
  SYMPRODaddObject(prod, SYMPROD_OBJ_BITMAP_ICON,
		   0, 0, color, "", &centroid, &bounding_box,
		   (symprod_object_union_t *) MEMbufPtr(mbuf_obj),
		   MEMbufLen(mbuf_obj));
  
  return;
  
}

/*********************************************************************
 * SYMPRODaddChunk()
 *
 * Add a SYMPROD chunk object to the product buffer.
 */

void SYMPRODaddChunk(symprod_product_t *prod,
		     const char *color,
		     const char *background_color,
		     double min_lat,
		     double min_lon,
		     double max_lat,
		     double max_lon,
		     int chunk_type,
		     int user_data,
		     char *data,
		     int data_len)
{
  static MEMbuf *mbuf_obj = NULL;
  symprod_const_chunk_obj_t chunk_obj;
  symprod_wpt_t centroid;
  symprod_box_t bounding_box;
  
  /*
   * initialize mbuf
   */

  if (mbuf_obj == NULL)
    mbuf_obj = MEMbufCreate();
  else
    MEMbufReset(mbuf_obj);
  
  /*
   * load object
   */

  chunk_obj.chunk_type = chunk_type;
  chunk_obj.user_data = user_data;

  /*
   * set bounding box and centroid
   */

  centroid.lat = (max_lat + min_lat) / 2.0;
  centroid.lon = (max_lon + min_lon) / 2.0;

  bounding_box.min_lat = min_lat;
  bounding_box.max_lat = max_lat;
  bounding_box.min_lon = min_lon;
  bounding_box.max_lon = max_lon;

  /*
   * load up object mbuf with obj type followed by data
   */

  MEMbufAdd(mbuf_obj, &chunk_obj, sizeof(symprod_const_chunk_obj_t));
  MEMbufAdd(mbuf_obj, data, data_len);

  SYMPRODaddObject(prod, SYMPROD_OBJ_CHUNK,
		   0, 0, color, background_color, &centroid, &bounding_box,
		   (symprod_object_union_t *) MEMbufPtr(mbuf_obj),
		   MEMbufLen(mbuf_obj));
  
  return;
  
}

/*********************************************************************
 * SYMPRODaddPolyline()
 *
 * Add a SYMPROD polyline object to the product buffer.
 */

void SYMPRODaddPolyline(symprod_product_t *prod,
			const char *color,
			int linetype,
			int linewidth,
			int capstyle,
			int joinstyle,
			int close_flag,
			int fill,
			symprod_wpt_t *pts,
			int npoints)

{

  static MEMbuf *mbuf_obj = NULL;
  int ipt;
  symprod_wpt_t *pt;
  symprod_wpt_t centroid;
  symprod_box_t bounding_box;
  symprod_const_polyline_obj_t polyline_obj;
  
  /*
   * initialize mbuf
   */

  if (mbuf_obj == NULL)
    mbuf_obj = MEMbufCreate();
  else
    MEMbufReset(mbuf_obj);
  
  /*
   * load bounding box & centroid
   */

  SYMPRODinitBbox(&bounding_box);
  pt = pts;
  for (ipt = 0; ipt < npoints; ipt++, pt++) {
    if (pt->lat != SYMPROD_WPT_PENUP &&
	pt->lon != SYMPROD_WPT_PENUP)
      SYMPRODupdateBboxPoint(&bounding_box, pt->lat, pt->lon);
  }
  
  centroid.lat =
    (bounding_box.min_lat + bounding_box.max_lat) / 2.0;
  centroid.lon =
    (bounding_box.min_lon + bounding_box.max_lon) / 2.0;
  
  /*
   * load the polyline object data
   */
       
  polyline_obj.close_flag = close_flag;
  polyline_obj.fill = fill;
  polyline_obj.linetype = linetype;
  polyline_obj.linewidth = linewidth;
  polyline_obj.capstyle = capstyle;
  polyline_obj.joinstyle = joinstyle;
  polyline_obj.line_interp = SYMPROD_LINE_INTERP_STRAIGHT;
  polyline_obj.num_points = npoints;

  /*
   * load up object mbuf with obj type followed by data
   */

  MEMbufAdd(mbuf_obj, &polyline_obj, sizeof(symprod_const_polyline_obj_t));
  MEMbufAdd(mbuf_obj, pts, npoints * sizeof(symprod_wpt_t));

  SYMPRODaddObject(prod, SYMPROD_OBJ_POLYLINE,
		   0, 0, color, "", &centroid, &bounding_box,
		   (symprod_object_union_t *) MEMbufPtr(mbuf_obj),
		   MEMbufLen(mbuf_obj));
  
  return;
  
}

/*********************************************************************
 * SYMPRODaddStrokedIcon()
 *
 * Add an array of SYMPROD stroked icons to the product buffer.
 */

void SYMPRODaddStrokedIcon(symprod_product_t *prod,
			   const char *color,
			   int num_icon_pts,
			   symprod_ppt_t *icon_pts,
			   int num_icons,
			   symprod_wpt_t *icon_origins)
{
  static MEMbuf *mbuf_obj = NULL;

  int ipt;
  symprod_wpt_t centroid;
  symprod_box_t bounding_box;
  symprod_const_stroked_icon_obj_t stroked_icon_obj;
  
  /*
   * initialize mbuf
   */

  if (mbuf_obj == NULL)
    mbuf_obj = MEMbufCreate();
  else
    MEMbufReset(mbuf_obj);
  
  /*
   * load bounding box & centroid
   */

  SYMPRODinitBbox(&bounding_box);

  for (ipt = 0; ipt < num_icons; ipt++)
    SYMPRODupdateBboxPoint(&bounding_box,
			   icon_origins[ipt].lat, icon_origins[ipt].lon);
  
  centroid.lat =
    (bounding_box.min_lat + bounding_box.max_lat) / 2.0;
  centroid.lon =
    (bounding_box.min_lon + bounding_box.max_lon) / 2.0;
  
  /*
   * load the stroked icon object data
   */
       
  stroked_icon_obj.num_icon_pts = num_icon_pts;
  stroked_icon_obj.num_icons = num_icons;
  
  /*
   * load up object mbuf with obj type followed by data
   */

  MEMbufAdd(mbuf_obj, &stroked_icon_obj,
	    sizeof(symprod_const_stroked_icon_obj_t));
  MEMbufAdd(mbuf_obj, icon_pts, num_icon_pts * sizeof(symprod_ppt_t));
  MEMbufAdd(mbuf_obj, icon_origins, num_icons * sizeof(symprod_wpt_t));
  
  SYMPRODaddObject(prod, SYMPROD_OBJ_STROKED_ICON,
		   0, 0, color, "", &centroid, &bounding_box,
		   (symprod_object_union_t *) MEMbufPtr(mbuf_obj),
		   MEMbufLen(mbuf_obj));
  
  return;
  
}

/*********************************************************************
 * SYMPRODaddText()
 *
 * Add a SYMPROD text object to the product buffer.
 */

void SYMPRODaddText(symprod_product_t *prod,
		    const char *color,
		    const char *background_color,
		    double lat, double lon,
		    int offset_x, int offset_y,
		    int vert_alignment,
		    int horiz_alignment,
		    int size,
		    const char *fontname,
		    const char *text_string)

{

  static MEMbuf *mbuf_obj = NULL;
  symprod_const_text_obj_t text_obj;
  symprod_wpt_t centroid;
  symprod_box_t bounding_box;
  
  /*
   * initialize mbuf
   */

  if (mbuf_obj == NULL)
    mbuf_obj = MEMbufCreate();
  else
    MEMbufReset(mbuf_obj);
  
  /*
   * load object
   */

  text_obj.origin.lat = lat;
  text_obj.origin.lon = lon;

  text_obj.offset.x = offset_x;
  text_obj.offset.y = offset_y;

  text_obj.vert_alignment = vert_alignment;
  text_obj.horiz_alignment = horiz_alignment;

  text_obj.size = size;
  text_obj.length = strlen(text_string);

  STRncopy(text_obj.fontname, fontname, SYMPROD_FONT_NAME_LEN);

  /*
   * set bounding box and centroid
   */

  centroid.lat = lat;
  centroid.lon = lon;

  bounding_box.min_lat = lat - 0.2;
  bounding_box.max_lat = lat + 0.2;
  bounding_box.min_lon = lon - 0.2;
  bounding_box.max_lon = lon + 0.2;

  /*
   * load up object mbuf with obj type followed by data
   */

  MEMbufAdd(mbuf_obj, &text_obj, sizeof(symprod_const_text_obj_t));
  MEMbufAdd(mbuf_obj, (void *) text_string, strlen(text_string) + 1);

  SYMPRODaddObject(prod, SYMPROD_OBJ_TEXT,
		   0, 0, color, background_color, &centroid, &bounding_box,
		   (symprod_object_union_t *) MEMbufPtr(mbuf_obj),
		   MEMbufLen(mbuf_obj));
  
  return;
  
}

/**********************************************************************
 * SYMPRODproductToBuffer() - Convert internal format to product buffer.
 *                            The buffer will be returned in
 *                            native format (any necessary byte-
 *                            swapping must be done later).
 */

char *SYMPRODproductToBuffer(symprod_product_t *prod,
			     int *buflen)

{

  int i;
  int npad;
  int n_is_odd;
  int n_offsets;
  si32 offset;
  char pad[8];

  static MEMbuf *mbuf_main = NULL;
  static MEMbuf *mbuf_obj = NULL;

  symprod_const_prod_hdr_t prod_hdr;
  symprod_obj_hdr_t obj_hdr;
  symprod_object_t *obj;
  
  /*
   * initialize mbufs
   */

  if (mbuf_main == NULL)
    mbuf_main = MEMbufCreate();
  else
    MEMbufReset(mbuf_main);
  
  if (mbuf_obj == NULL)
    mbuf_obj = MEMbufCreate();
  else
    MEMbufReset(mbuf_obj);
  
  /*
   * add main header
   */
  
  MEM_zero(prod_hdr);

  prod_hdr.generate_time = prod->generate_time;
  prod_hdr.received_time = prod->generate_time;
  prod_hdr.start_time = prod->start_time;
  prod_hdr.expire_time = prod->expire_time;
  prod_hdr.bounding_box = prod->bounding_box;
  prod_hdr.num_objs = prod->num_objs;
  if (prod->label == (char *)NULL)
    prod_hdr.label[0] = '\0';
  else
    STRcopy(prod_hdr.label, prod->label, SYMPROD_LABEL_LEN);
  
  MEMbufAdd(mbuf_main, &prod_hdr, sizeof(symprod_const_prod_hdr_t));

  /*
   * compute number of offsets
   */

  n_offsets = prod_hdr.num_objs;
  if ((prod_hdr.num_objs / 2) == 1) {
    n_is_odd = TRUE;
  } else {
    n_is_odd = FALSE;
  }
  if (n_is_odd) {
    /* this extra offset is added for 8-byte alignment */
    n_offsets++;
  }

  /*
   * loop through objects
   */

  obj = (symprod_object_t *) MEMbufPtr(prod->mbuf_obj);

  for (i = 0; i < prod->num_objs; i++, obj++) {

    /*
     * compute offset and add to main buffer
     */

    offset = (sizeof(symprod_const_prod_hdr_t) +
	      n_offsets * sizeof(si32) +
	      MEMbufLen(mbuf_obj));
    MEMbufAdd(mbuf_main, &offset, sizeof(si32));
    
    /*
     * Set the object hdr values.
     */

    obj_hdr.object_type = obj->object_type;
    obj_hdr.object_data = obj->object_data;
    obj_hdr.num_bytes = obj->num_bytes;
    obj_hdr.detail_level = obj->detail_level;
    STRcopy(obj_hdr.color, (char *)obj->color,
	    SYMPROD_COLOR_LEN);
    STRcopy(obj_hdr.background_color, (char *)obj->background_color,
	    SYMPROD_COLOR_LEN);
    obj_hdr.centroid = obj->centroid;

    /*
     * add object hdr to obj membuf
     */
    
    MEMbufAdd(mbuf_obj, &obj_hdr, sizeof(symprod_obj_hdr_t));

    /*
     * add object to obj membuf
     */

    MEMbufAdd(mbuf_obj, obj->object_info, obj->num_bytes);

    /*
     * add padding for alignment as necessary
     */

    npad = 8 - (obj->num_bytes % 8);
    memset(pad, 0, npad);
    MEMbufAdd(mbuf_obj, pad, npad);
    
  } /* i */

  /*
   * if odd number of offsets, add one for alignment
   */

  if (n_is_odd) {
    offset = 0;
    MEMbufAdd(mbuf_main, &offset, sizeof(si32));
  }

  /*
   * add obj buffer to main buffer
   */

  MEMbufAdd(mbuf_main, MEMbufPtr(mbuf_obj), MEMbufLen(mbuf_obj));

  /*
   * set return values
   */

  *buflen = MEMbufLen(mbuf_main);
  return (MEMbufPtr(mbuf_main));

}


/**********************************************************************
 * SYMPRODbufferToProduct() - Convert a product buffer to internal
 *                            format.  The buffer is assumed to be in
 *                            native format (any necessary byte-
 *                            swapping has already been done).
 */

symprod_product_t *SYMPRODbufferToProduct(char *buffer)

{

  symprod_prod_hdr_t *prod_hdr_in;
  symprod_obj_hdr_t  *obj_hdr_in;
  
  symprod_product_t *prod = NULL;
  symprod_object_t obj;
  
  int i;
  
  /*
   * Set pointers in the incoming buffer.
   */

  prod_hdr_in = (symprod_prod_hdr_t *)buffer;
  
  /*
   * create product
   */

  prod = SYMPRODcreateProduct(0, 0, 0, 0);

  /*
   * Set the values in the main product structure.
   */

  prod->generate_time  = prod_hdr_in->generate_time;
  prod->received_time  = prod_hdr_in->received_time;
  prod->start_time     = prod_hdr_in->start_time;
  prod->expire_time    = prod_hdr_in->expire_time;
  prod->bounding_box   = prod_hdr_in->bounding_box;
  prod->num_objs       = prod_hdr_in->num_objs;
  if (prod_hdr_in->label[0] == '\0')
    prod->label = (char *)NULL;
  else
    prod->label        = STRdup(prod_hdr_in->label);
  
  /*
   * Now set all of the object information
   */

  for (i = 0; i < prod_hdr_in->num_objs; i++) {

    /*
     * Set the input and output pointers.
     */
    
    obj_hdr_in = (symprod_obj_hdr_t *)(buffer +
				       prod_hdr_in->offsets[i]);
    
    /*
     * Set the output object values.
     */

    obj.object_type = obj_hdr_in->object_type;
    obj.object_data = obj_hdr_in->object_data;
    obj.num_bytes   = obj_hdr_in->num_bytes;
    obj.detail_level = obj_hdr_in->detail_level;
    STRcopy(obj.color, (char *)obj_hdr_in->color,
	    SYMPROD_COLOR_LEN);
    STRcopy(obj.background_color, (char *)obj_hdr_in->background_color,
	    SYMPROD_COLOR_LEN);
    obj.centroid     = obj_hdr_in->centroid;

    /*
     * Now pull out the individual object information
     */

    obj.object_info =
      (symprod_object_union_t *)umalloc(obj.num_bytes);
    
    memcpy(obj.object_info,
	   (char *)obj_hdr_in + sizeof(symprod_obj_hdr_t),
	   obj.num_bytes);
    
    /*
     * add to prod
     */

    MEMbufAdd(prod->mbuf_obj, &obj, sizeof(symprod_object_t));
    
  } /* endfor - i */
  
  return(prod);

}


/**********************************************************************
 * SYMPRODfreeProduct() - Free a product.
 */

void SYMPRODfreeProduct(symprod_product_t *prod)
{

  int i;
  symprod_object_t *obj;
  
  /*
   * Free the space used for the object specific information.
   */

  obj = (symprod_object_t *) MEMbufPtr(prod->mbuf_obj);
  for (i = 0; i < prod->num_objs; i++, obj++) {
    ufree(obj->object_info);
  }

  /*
   * free object membuf
   */

  MEMbufDelete(prod->mbuf_obj);

  /*
   * free struct
   */
  
  if (prod->label != (char *)NULL)
    ufree(prod->label);
  ufree(prod);
  
  return;
}


/**********************************************************************
 * SYMPRODproductBufferToBE() - Convert a product buffer to big-endian
 *                              format for writing to disk or trans-
 *                              mission across the network.
 */

void SYMPRODproductBufferToBE(char *buffer)
{
  char *swap_ptr = buffer;
  symprod_prod_hdr_t *prod_hdr = (symprod_prod_hdr_t *)buffer;
  symprod_obj_hdr_t  *obj_hdr;
  
  int obj;
  
  /*
   * Swap each of the objects in the buffer.  Do this before swapping
   * the product header since we need some of the product header values
   * to find our way around in the objects.
   */

  for (obj = 0; obj < prod_hdr->num_objs; obj++)
  {
    /*
     * Set the buffer pointer for swapping
     */

    obj_hdr = (symprod_obj_hdr_t *)(buffer + prod_hdr->offsets[obj]);
    
    /*
     * Swap the object information
     */

    swap_ptr = (char *)obj_hdr + sizeof(symprod_obj_hdr_t);
    
    switch(obj_hdr->object_type)
    {
    case SYMPROD_OBJ_TEXT :
      SYMPRODobjTextToBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_POLYLINE :
      SYMPRODobjPolylineToBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_STROKED_ICON :
      SYMPRODobjStrokedIconToBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_NAMED_ICON :
      SYMPRODobjNamedIconToBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_BITMAP_ICON :
      SYMPRODobjBitmapIconToBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_ARC :
      SYMPRODobjArcToBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_RECTANGLE :
      SYMPRODobjRectangleToBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_CHUNK :
      SYMPRODobjChunkToBE(swap_ptr);
      break;
      
    default:
      fprintf(stderr,
	      "ERROR: symprod:SYMPRODproductBufferToBE\n");
      fprintf(stderr,
	      "Unknown object type %d in for object %d product buffer.\n",
	      obj_hdr->object_type, obj);
      break;
      
    } /* endswitch - obj_hdr->object_type */
    
    /*
     * Swap the object header
     */

    SYMPRODobjHdrToBE((char *)obj_hdr);
    
  } /* endfor - obj */
  
  /*
   * Now swap the product header
   */

  SYMPRODprodHdrToBE((char *)prod_hdr);
  
  return;
}


/**********************************************************************
 * SYMPRODproductBufferFromBE() - Convert a product buffer from big-
 *                                endian format after reading from disk
 *                                or transmission across the network.
 */

void SYMPRODproductBufferFromBE(char *buffer)
{
  char *swap_ptr = buffer;

  symprod_prod_hdr_t *prod_hdr;
  symprod_obj_hdr_t  *obj_hdr;
  
  int obj;
  
  /*
   * Swap the product header
   */

  SYMPRODprodHdrFromBE(swap_ptr);
  
  prod_hdr = (symprod_prod_hdr_t *)buffer;
  
  /*
   * Swap each of the objects in the buffer.
   */

  for (obj = 0; obj < prod_hdr->num_objs; obj++)
  {
    /*
     * Set the buffer pointer for swapping
     */

    swap_ptr = buffer + prod_hdr->offsets[obj];
    
    /*
     * Swap the object header
     */

    SYMPRODobjHdrFromBE(swap_ptr);
    
    obj_hdr = (symprod_obj_hdr_t *)swap_ptr;
    
    /*
     * Swap the object information
     */

    swap_ptr += sizeof(symprod_obj_hdr_t);
    
    switch(obj_hdr->object_type)
    {
    case SYMPROD_OBJ_TEXT :
      SYMPRODobjTextFromBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_POLYLINE :
      SYMPRODobjPolylineFromBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_STROKED_ICON :
      SYMPRODobjStrokedIconFromBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_NAMED_ICON :
      SYMPRODobjNamedIconFromBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_BITMAP_ICON :
      SYMPRODobjBitmapIconFromBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_ARC :
      SYMPRODobjArcFromBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_RECTANGLE :
      SYMPRODobjRectangleFromBE(swap_ptr);
      break;
      
    case SYMPROD_OBJ_CHUNK :
      SYMPRODobjChunkFromBE(swap_ptr);
      break;
      
    default:
      fprintf(stderr,
	      "ERROR: symprod:SYMPRODproductBufferFromBE\n");
      fprintf(stderr,
	      "Unknown object type %d in for object %d product buffer.\n",
	      obj_hdr->object_type, obj);
      fprintf(stderr,
	      "Object not being swapped\n");
      break;
      
    } /* endswitch - obj_hdr->object_type */
    
  } /* endfor - obj */
  
  return;
}


/**********************************************************************
 * SYMPRODprodHdrToBE() - Convert a product header structure to big-
 *                        endian format for writing to disk or trans-
 *                        mission across the network.
 */

void SYMPRODprodHdrToBE(char *buffer)
{
  symprod_prod_hdr_t *prod_hdr =
    (symprod_prod_hdr_t *)buffer;
  
  prod_hdr->generate_time = BE_from_si32(prod_hdr->generate_time);
  prod_hdr->received_time = BE_from_si32(prod_hdr->received_time);
  prod_hdr->start_time    = BE_from_si32(prod_hdr->start_time);
  prod_hdr->expire_time   = BE_from_si32(prod_hdr->expire_time);
  
  SYMPRODboxToBE((char *)&prod_hdr->bounding_box);
  
  BE_from_array_32((void *)prod_hdr->offsets,
		   prod_hdr->num_objs * sizeof(si32));
  
  prod_hdr->num_objs      = BE_from_si32(prod_hdr->num_objs);
  prod_hdr->spare         = BE_from_si32(prod_hdr->spare);
  
  /* prod_hdr->label is okay */

  return;
}


/**********************************************************************
 * SYMPRODprodHdrFromBE() - Convert a product header structure from big-
 *                          endian format after reading from disk or
 *                          transmission across the network.
 */

void SYMPRODprodHdrFromBE(char *buffer)
{
  symprod_prod_hdr_t *prod_hdr =
    (symprod_prod_hdr_t *)buffer;
  
  prod_hdr->generate_time = BE_to_si32(prod_hdr->generate_time);
  prod_hdr->received_time = BE_to_si32(prod_hdr->received_time);
  prod_hdr->start_time    = BE_to_si32(prod_hdr->start_time);
  prod_hdr->expire_time   = BE_to_si32(prod_hdr->expire_time);
  
  SYMPRODboxFromBE((char *)&prod_hdr->bounding_box);
  
  prod_hdr->num_objs      = BE_to_si32(prod_hdr->num_objs);
  prod_hdr->spare         = BE_to_si32(prod_hdr->spare);
  
  BE_to_array_32((void *)prod_hdr->offsets,
		 prod_hdr->num_objs * sizeof(si32));
  
  /* prod_hdr->label is okay */

  return;
}


/**********************************************************************
 * SYMPRODobjHdrToBE() - Convert an object header structure to big-
 *                       endian format for writing to disk or trans-
 *                       mission across the network.
 */

void SYMPRODobjHdrToBE(char *buffer)
{
  symprod_obj_hdr_t *obj_hdr =
    (symprod_obj_hdr_t *)buffer;
  
  obj_hdr->object_type  = BE_from_si32(obj_hdr->object_type);
  obj_hdr->object_data  = BE_from_si32(obj_hdr->object_data);
  obj_hdr->num_bytes    = BE_from_si32(obj_hdr->num_bytes);
  obj_hdr->detail_level = BE_from_si32(obj_hdr->detail_level);
  
  SYMPRODwptToBE((char *)&obj_hdr->centroid);
  
  return;
}


/**********************************************************************
 * SYMPRODobjHdrFromBE() - Convert an object header structure from big-
 *                         endian format after reading from disk or
 *                         transmission across the network.
 */

void SYMPRODobjHdrFromBE(char *buffer)
{
  symprod_obj_hdr_t *obj_hdr =
    (symprod_obj_hdr_t *)buffer;
  
  obj_hdr->object_type  = BE_to_si32(obj_hdr->object_type);
  obj_hdr->object_data  = BE_to_si32(obj_hdr->object_data);
  obj_hdr->num_bytes    = BE_to_si32(obj_hdr->num_bytes);
  obj_hdr->detail_level = BE_to_si32(obj_hdr->detail_level);
  
  SYMPRODwptFromBE((char *)&obj_hdr->centroid);
  
  return;
}


/**********************************************************************
 * SYMPRODobjTextToBE() - Convert a text object structure to big-
 *                        endian format for writing to disk or trans-
 *                        mission across the network.
 */

void SYMPRODobjTextToBE(char *buffer)
{
  symprod_text_obj_t *obj =
    (symprod_text_obj_t *)buffer;
  
  SYMPRODwptToBE((char *)&obj->origin);
  SYMPRODpptToBE((char *)&obj->offset);
  
  obj->vert_alignment  = BE_from_si32(obj->vert_alignment);
  obj->horiz_alignment = BE_from_si32(obj->horiz_alignment);
  obj->size            = BE_from_si32(obj->size);
  obj->length          = BE_from_si32(obj->length);
  
  return;
}


/**********************************************************************
 * SYMPRODobjTextFromBE() - Convert a text object structure from big-
 *                          endian format after reading from disk or
 *                          transmission across the network.
 */

void SYMPRODobjTextFromBE(char *buffer)
{
  symprod_text_obj_t *obj =
    (symprod_text_obj_t *)buffer;
  
  SYMPRODwptFromBE((char *)&obj->origin);
  SYMPRODpptFromBE((char *)&obj->offset);
  
  obj->vert_alignment  = BE_to_si32(obj->vert_alignment);
  obj->horiz_alignment = BE_to_si32(obj->horiz_alignment);
  obj->size            = BE_to_si32(obj->size);
  obj->length          = BE_to_si32(obj->length);
  
  return;
}


/**********************************************************************
 * SYMPRODobjPolylineToBE() - Convert a polyline object structure to
 *                            big-endian format for writing to disk or
 *                            transmission across the network.
 */

void SYMPRODobjPolylineToBE(char *buffer)
{
  symprod_polyline_obj_t *obj =
    (symprod_polyline_obj_t *)buffer;
  
  int pt;
  
  for (pt = 0; pt < obj->num_points; pt++)
    SYMPRODwptToBE((char *)&(obj->points[pt]));
       
  obj->close_flag  = BE_from_si32(obj->close_flag);
  obj->fill        = BE_from_si32(obj->fill);
  obj->linetype    = BE_from_si32(obj->linetype);
  obj->linewidth   = BE_from_si32(obj->linewidth);
  obj->capstyle    = BE_from_si32(obj->capstyle);
  obj->joinstyle   = BE_from_si32(obj->joinstyle);
  obj->line_interp = BE_from_si32(obj->line_interp);
  obj->num_points  = BE_from_si32(obj->num_points);
  
  return;
}


/**********************************************************************
 * SYMPRODobjPolylineFromBE() - Convert a polyline object structure from
 *                              big-endian format after reading from
 *                              disk or transmission across the network.
 */

void SYMPRODobjPolylineFromBE(char *buffer)
{
  symprod_polyline_obj_t *obj =
    (symprod_polyline_obj_t *)buffer;
  
  int pt;
  
  obj->close_flag  = BE_to_si32(obj->close_flag);
  obj->fill        = BE_to_si32(obj->fill);
  obj->linetype    = BE_to_si32(obj->linetype);
  obj->linewidth   = BE_to_si32(obj->linewidth);
  obj->capstyle    = BE_to_si32(obj->capstyle);
  obj->joinstyle   = BE_to_si32(obj->joinstyle);
  obj->line_interp = BE_to_si32(obj->line_interp);
  obj->num_points  = BE_to_si32(obj->num_points);
  
  for (pt = 0; pt < obj->num_points; pt++)
    SYMPRODwptFromBE((char *)&(obj->points[pt]));
       
  return;
}


/**********************************************************************
 * SYMPRODobjStrokedIconToBE() - Convert a stroked icon object
 *                               structure to big-endian format for
 *                               writing to disk or transmission across
 *                               the network.
 */

void SYMPRODobjStrokedIconToBE(char *buffer)
{
  symprod_stroked_icon_obj_t *obj =
    (symprod_stroked_icon_obj_t *)buffer;
  
  int pt;
  int icon;
  
  symprod_wpt_t *icon_ptr;
  
  for (pt = 0; pt < obj->num_icon_pts; pt++)
    SYMPRODpptToBE((char *)&(obj->icon_pts[pt]));
  
  icon_ptr = (symprod_wpt_t *)(buffer + 2 * sizeof(si32) +
			       obj->num_icon_pts * sizeof(symprod_ppt_t));
  
  for (icon = 0; icon < obj->num_icons; icon++)
  {
    SYMPRODwptToBE((char *)icon_ptr);
    icon_ptr++;
  } /* endfor - icon */
  
  obj->num_icon_pts = BE_from_si32(obj->num_icon_pts);
  obj->num_icons    = BE_from_si32(obj->num_icons);
  
  return;
}


/**********************************************************************
 * SYMPRODobjStrokedIconFromBE() - Convert a stroked icon object
 *                                 structure from big-endian format
 *                                 after reading from disk or trans-
 *                                 mission across the network.
 */

void SYMPRODobjStrokedIconFromBE(char *buffer)
{
  symprod_stroked_icon_obj_t *obj =
    (symprod_stroked_icon_obj_t *)buffer;
  
  int pt;
  int icon;
  
  symprod_wpt_t *icon_ptr;
  
  obj->num_icon_pts = BE_to_si32(obj->num_icon_pts);
  obj->num_icons    = BE_to_si32(obj->num_icons);
  
  for (pt = 0; pt < obj->num_icon_pts; pt++)
    SYMPRODpptFromBE((char *)&(obj->icon_pts[pt]));
  
  icon_ptr = (symprod_wpt_t *)(buffer + 2 * sizeof(si32) +
			       obj->num_icon_pts * sizeof(symprod_ppt_t));
  
  for (icon = 0; icon < obj->num_icons; icon++)
  {
    SYMPRODwptFromBE((char *)icon_ptr);
    icon_ptr++;
  } /* endfor - icon */
  
  return;
}


/**********************************************************************
 * SYMPRODobjNamedIconToBE() - Convert a named icon object structure to
 *                             big-endian format for writing to disk or
 *                             transmission across the network.
 */

void SYMPRODobjNamedIconToBE(char *buffer)
{
  symprod_named_icon_obj_t *obj =
    (symprod_named_icon_obj_t *)buffer;
  
  obj->num_icons = BE_from_si32(obj->num_icons);
  obj->spare     = BE_from_si32(obj->spare);
  
  return;
}


/**********************************************************************
 * SYMPRODobjNamedIconFromBE() - Convert a named icon object structure
 *                               from big-endian format after reading
 *                               from disk or transmission across the network.
 */

void SYMPRODobjNamedIconFromBE(char *buffer)
{
  symprod_named_icon_obj_t *obj =
    (symprod_named_icon_obj_t *)buffer;
  
  obj->num_icons = BE_to_si32(obj->num_icons);
  obj->spare     = BE_to_si32(obj->spare);
  
  return;
}


/**********************************************************************
 * SYMPRODobjBitmapIconToBE() - Convert a bitmap icon object structure
 *                              to big-endian format for writing to disk
 *                              or transmission across the network.
 */

void SYMPRODobjBitmapIconToBE(char *buffer)
{
  symprod_bitmap_icon_obj_t *obj =
    (symprod_bitmap_icon_obj_t *)buffer;
  
  int icon;
  
  for (icon = 0; icon < obj->num_icons; icon++)
    SYMPRODwptToBE((char *)&(obj->icon_origins[icon]));
  
  /* bitmap is okay */

  obj->bitmap_x_dim = BE_from_si32(obj->bitmap_x_dim);
  obj->bitmap_y_dim = BE_from_si32(obj->bitmap_y_dim);
  obj->num_icons    = BE_from_si32(obj->num_icons);
  obj->spare        = BE_from_si32(obj->spare);
  
  return;
}


/**********************************************************************
 * SYMPRODobjBitmapIconFromBE() - Convert a bitmap icon object structure
 *                                from big-endian format after reading
 *                                disk or transmission across the network.
 */

void SYMPRODobjBitmapIconFromBE(char *buffer)
{
  symprod_bitmap_icon_obj_t *obj =
    (symprod_bitmap_icon_obj_t *)buffer;
  
  int icon;
  
  obj->bitmap_x_dim = BE_to_si32(obj->bitmap_x_dim);
  obj->bitmap_y_dim = BE_to_si32(obj->bitmap_y_dim);
  obj->num_icons    = BE_to_si32(obj->num_icons);
  obj->spare        = BE_to_si32(obj->spare);
  
  for (icon = 0; icon < obj->num_icons; icon++)
    SYMPRODwptFromBE((char *)&(obj->icon_origins[icon]));
  
  /* bitmap is okay */

  return;
}


/**********************************************************************
 * SYMPRODobjArcToBE() - Convert an arc object structure to big-endian
 *                       format for writing to disk or transmission 
 *                       across the network.
 */

void SYMPRODobjArcToBE(char *buffer)
{
  symprod_arc_obj_t *obj =
    (symprod_arc_obj_t *)buffer;
  
  SYMPRODwptToBE((char *)&obj->origin);
  
  obj->height      = BE_from_si32(obj->height);
  obj->width       = BE_from_si32(obj->width);
  BE_from_array_32(&obj->angle1, 4);
  BE_from_array_32(&obj->angle2, 4);
  obj->linetype    = BE_from_si32(obj->linetype);
  obj->linewidth   = BE_from_si32(obj->linewidth);
  obj->fill        = BE_from_si32(obj->fill);
  obj->capstyle    = BE_from_si32(obj->capstyle);
  obj->joinstyle   = BE_from_si32(obj->joinstyle);
  obj->spare       = BE_from_si32(obj->spare);
  
  return;
}


/**********************************************************************
 * SYMPRODobjArcFromBE() - Convert an arc object structure from big-
 *                         endian format after reading from disk or
 *                         transmission across the network.
 */

void SYMPRODobjArcFromBE(char *buffer)
{
  symprod_arc_obj_t *obj =
    (symprod_arc_obj_t *)buffer;
  
  SYMPRODwptFromBE((char *)&obj->origin);
  
  obj->height      = BE_to_si32(obj->height);
  obj->width       = BE_to_si32(obj->width);
  BE_to_array_32(&obj->angle1, 4);
  BE_to_array_32(&obj->angle2, 4);
  obj->linetype    = BE_to_si32(obj->linetype);
  obj->linewidth   = BE_to_si32(obj->linewidth);
  obj->fill        = BE_to_si32(obj->fill);
  obj->capstyle    = BE_to_si32(obj->capstyle);
  obj->joinstyle   = BE_to_si32(obj->joinstyle);
  obj->spare       = BE_to_si32(obj->spare);
  
  return;
}


/**********************************************************************
 * SYMPRODobjRectangleToBE() - Convert a rectangle object structure to
 *                             big-endian format for writing to disk or
 *                             transmission across the network.
 */

void SYMPRODobjRectangleToBE(char *buffer)
{
  symprod_rectangle_obj_t *obj =
    (symprod_rectangle_obj_t *)buffer;
  
  SYMPRODwptToBE((char *)&obj->origin);
  
  obj->height    = BE_from_si32(obj->height);
  obj->width     = BE_from_si32(obj->width);
  obj->linetype  = BE_from_si32(obj->linetype);
  obj->linewidth = BE_from_si32(obj->linewidth);
  obj->fill      = BE_from_si32(obj->fill);
  obj->capstyle  = BE_from_si32(obj->capstyle);
  obj->joinstyle = BE_from_si32(obj->joinstyle);
  obj->spare     = BE_from_si32(obj->spare);
  
  return;
}


/**********************************************************************
 * SYMPRODobjRectangleFromBE() - Convert a rectangle object structure
 *                               from big-endian format after reading
 *                               from disk or transmission across the
 *                               network.
 */

void SYMPRODobjRectangleFromBE(char *buffer)
{
  symprod_rectangle_obj_t *obj =
    (symprod_rectangle_obj_t *)buffer;
  
  SYMPRODwptFromBE((char *)&obj->origin);
  
  obj->height    = BE_to_si32(obj->height);
  obj->width     = BE_to_si32(obj->width);
  obj->linetype  = BE_to_si32(obj->linetype);
  obj->linewidth = BE_to_si32(obj->linewidth);
  obj->fill      = BE_to_si32(obj->fill);
  obj->capstyle  = BE_to_si32(obj->capstyle);
  obj->joinstyle = BE_to_si32(obj->joinstyle);
  obj->spare     = BE_to_si32(obj->spare);
  
  return;
}


/**********************************************************************
 * SYMPRODobjChunkToBE() - Convert a chunk object structure to big-
 *                         endian format for writing to disk or trans-
 *                         mission across the network.  Note that the
 *                         chunk data itself is assumed to already be
 *                         in big-endian format since this library
 *                         doesn't know anything about the data format.
 */

void SYMPRODobjChunkToBE(char *buffer)
{
  symprod_chunk_obj_t *obj =
    (symprod_chunk_obj_t *)buffer;
  
  obj->chunk_type = BE_from_si32(obj->chunk_type);
  obj->user_data  = BE_from_si32(obj->user_data);
  
  /* data must be swapped by the user */

  return;
}


/**********************************************************************
 * SYMPRODobjChunkFromBE() - Convert a chunk object structure from big-
 *                           endian format after reading from disk or
 *                           transmission across the network.  Note that
 *                           the chunk data itself is assumed to already
 *                           be in big-endian format since this library
 *                           doesn't know anything about the data format.
 */

void SYMPRODobjChunkFromBE(char *buffer)
{
  symprod_chunk_obj_t *obj =
    (symprod_chunk_obj_t *)buffer;
  
  obj->chunk_type = BE_to_si32(obj->chunk_type);
  obj->user_data  = BE_to_si32(obj->user_data);
  
  /* data must be swapped by the user */

  return;
}


/**********************************************************************
 * SYMPRODboxToBE() - Convert a bounding box structure to big-endian
 *                    format for writing to disk or transmission across
 *                    the network.
 */

void SYMPRODboxToBE(char *buffer)
{
  symprod_box_t *box = (symprod_box_t *)buffer;
  
  BE_from_array_32(&box->min_lat, 4);
  BE_from_array_32(&box->max_lat, 4);
  BE_from_array_32(&box->min_lon, 4);
  BE_from_array_32(&box->max_lon, 4);
  
  return;
}


/**********************************************************************
 * SYMPRODboxFromBE() - Convert a bounding box structure from big-endian
 *                      format after reading from disk or transmission
 *                      across the network.
 */

void SYMPRODboxFromBE(char *buffer)
{
  symprod_box_t *box = (symprod_box_t *)buffer;
  
  BE_to_array_32(&box->min_lat, 4);
  BE_to_array_32(&box->max_lat, 4);
  BE_to_array_32(&box->min_lon, 4);
  BE_to_array_32(&box->max_lon, 4);
  
  return;
}


/**********************************************************************
 * SYMPRODwptToBE() - Convert a world coordinate point structure to
 *                    big-endian format for writing to disk or trans-
 *                    mission across the network.
 */

void SYMPRODwptToBE(char *buffer)
{
  symprod_wpt_t *wpt = (symprod_wpt_t *)buffer;
  
  BE_from_array_32(&wpt->lat, 4);
  BE_from_array_32(&wpt->lon, 4);
  
  return;
}


/**********************************************************************
 * SYMPRODwptFromBE() - Convert a world coordinate point structure from
 *                      big-endian format after reading from disk or
 *                      transmission across the network.
 */

void SYMPRODwptFromBE(char *buffer)
{
  symprod_wpt_t *wpt = (symprod_wpt_t *)buffer;
  
  BE_to_array_32(&wpt->lat, 4);
  BE_to_array_32(&wpt->lon, 4);
  
  return;
}


/**********************************************************************
 * SYMPRODpptToBE() - Convert a pixel coordinate point structure to
 *                    big-endian format for writing to disk or trans-
 *                    mission across the network.
 */

void SYMPRODpptToBE(char *buffer)
{
  symprod_ppt_t *ppt = (symprod_ppt_t *)buffer;
  
  ppt->x = BE_from_si32(ppt->x);
  ppt->y = BE_from_si32(ppt->y);
  
  return;
}


/**********************************************************************
 * SYMPRODpptFromBE() - Convert a pixel coordinate point structure from
 *                      big-endian format after reading from disk or
 *                      transmission across the network.
 */

void SYMPRODpptFromBE(char *buffer)
{
  symprod_ppt_t *ppt = (symprod_ppt_t *)buffer;
  
  ppt->x = BE_to_si32(ppt->x);
  ppt->y = BE_to_si32(ppt->y);
  
  return;
}


/**********************************************************************
 * SYMPRODprintProductHdr() - Prints a product header to the given
 *                            stream in ASCII format.
 */

void SYMPRODprintProductHdr(FILE *stream,
			    symprod_prod_hdr_t *prod_hdr)
{
  time_t print_time;
  int i;
  
  fprintf(stream, "\n");
  fprintf(stream, "\n");
  fprintf(stream, "Product Header:\n");
  fprintf(stream, "\n");

  print_time = prod_hdr->generate_time;
  fprintf(stream, "   generate time = %s\n",
	  utimstr(print_time));

  print_time = prod_hdr->received_time;
  fprintf(stream, "   received time = %s\n",
	  utimstr(print_time));

  print_time = prod_hdr->start_time;
  fprintf(stream, "   start time = %s\n",
	  utimstr(print_time));

  print_time = prod_hdr->expire_time;
  fprintf(stream, "   expire time = %s\n",
	  utimstr(print_time));

  fprintf(stream, "   bounding box:\n");
  fprintf(stream, "      min lat = %f\n", prod_hdr->bounding_box.min_lat);
  fprintf(stream, "      max lat = %f\n", prod_hdr->bounding_box.max_lat);
  fprintf(stream, "      min lon = %f\n", prod_hdr->bounding_box.min_lon);
  fprintf(stream, "      max lon = %f\n", prod_hdr->bounding_box.max_lon);
  fprintf(stream, "   num objects = %ld\n", (long)prod_hdr->num_objs);
  fprintf(stream, "   spare = %ld\n", (long)prod_hdr->spare);
  fprintf(stream, "   label = <%s>\n", prod_hdr->label);
  
  fprintf(stream, "   object offsets:\n");
  for (i = 0; i < prod_hdr->num_objs; i++)
    fprintf(stream, "      %ld\n", (long)prod_hdr->offsets[i]);
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintObjectHdr() - Prints an object header to the given
 *                           stream in ASCII format.
 */

void SYMPRODprintObjectHdr(FILE *stream,
			   symprod_obj_hdr_t *obj_hdr)
{
  fprintf(stream, "\n");
  fprintf(stream, "Object Header information:\n");
  fprintf(stream, "\n");

  fprintf(stream, "   object type = ");
  print_object_type(stream, obj_hdr->object_type);
  fprintf(stream, "\n");
  
  fprintf(stream, "   object data = %ld\n", (long)obj_hdr->object_data);
  fprintf(stream, "   num bytes = %ld\n", (long)obj_hdr->num_bytes);
  fprintf(stream, "   detail level = %ld\n", (long)obj_hdr->detail_level);
  fprintf(stream, "   color = '%s'\n", obj_hdr->color);
  fprintf(stream, "   background color = '%s'\n", obj_hdr->background_color);
  fprintf(stream, "   centroid lat = %f\n", obj_hdr->centroid.lat);
  fprintf(stream, "   centroid lon = %f\n", obj_hdr->centroid.lon);
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintProduct() - Print a product structure to the given
 *                         stream in ASCII format.  Also prints all of
 *                         the object structures contained in the
 *                         product structure.
 */

void SYMPRODprintProduct(FILE *stream,
			 symprod_product_t *prod)
{

  int i;
  symprod_object_t *obj;
  
  fprintf(stream, "\n");
  fprintf(stream, "\n");
  fprintf(stream, "SYMPROD product information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   generate time = %s\n",
	  utimstr(prod->generate_time));
  fprintf(stream, "   received time = %s\n",
	  utimstr(prod->received_time));
  fprintf(stream, "   start time = %s\n",
	  utimstr(prod->start_time));
  fprintf(stream, "   expire time = %s\n",
	  utimstr(prod->expire_time));
  fprintf(stream, "\n");
  fprintf(stream, "   bounding box:\n");
  fprintf(stream, "      min lat = %.10f\n", prod->bounding_box.min_lat);
  fprintf(stream, "      max lat = %.10f\n", prod->bounding_box.max_lat);
  fprintf(stream, "      min lon = %.10f\n", prod->bounding_box.min_lon);
  fprintf(stream, "      max lon = %.10f\n", prod->bounding_box.max_lon);
  fprintf(stream, "\n");
  fprintf(stream, "   num objects = %d\n",
	  prod->num_objs);
  fprintf(stream, "   num objects allocated = %d\n",
	  prod->num_objs_alloc);
  fprintf(stream, "   label = <%s>\n", prod->label);
  
  obj = (symprod_object_t *) MEMbufPtr(prod->mbuf_obj);

  for (i = 0; i < prod->num_objs; i++, obj++) {
    SYMPRODprintObject(stream, obj);
  }
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintProductBuffer() - Print a product buffer to the given
 *                               stream in ASCII format.
 */

void SYMPRODprintProductBuffer(FILE *stream,
			       char *product_buffer)
{
  int i;
  
  symprod_prod_hdr_t *prod_hdr =
    (symprod_prod_hdr_t *)product_buffer;
  
  fprintf(stream, "\n");
  fprintf(stream, "\n");
  fprintf(stream, "SYMPROD product buffer information:\n");
  fprintf(stream, "\n");

  SYMPRODprintProductHdr(stream, prod_hdr);

  for (i = 0; i < prod_hdr->num_objs; i++)
  {
    symprod_obj_hdr_t *obj_hdr =
      (symprod_obj_hdr_t *)(product_buffer + prod_hdr->offsets[i]);
    char *obj_data =
      product_buffer + prod_hdr->offsets[i] + sizeof(symprod_obj_hdr_t);
    
    SYMPRODprintObjectHdr(stream, obj_hdr);

    switch(obj_hdr->object_type)
    {
    case SYMPROD_OBJ_TEXT :
      SYMPRODprintTextObject(stream, 
			     (symprod_text_obj_t *)obj_data);
      break;
    
    case SYMPROD_OBJ_POLYLINE :
      SYMPRODprintPolylineObject(stream, 
				 (symprod_polyline_obj_t *)obj_data);
      break;
    
    case SYMPROD_OBJ_STROKED_ICON :
      SYMPRODprintStrokedIconObject(stream, 
				    (symprod_stroked_icon_obj_t *)obj_data);
      break;
    
    case SYMPROD_OBJ_NAMED_ICON :
      SYMPRODprintNamedIconObject(stream, 
				  (symprod_named_icon_obj_t *)obj_data);
      break;
    
    case SYMPROD_OBJ_BITMAP_ICON :
      SYMPRODprintBitmapIconObject(stream, 
				   (symprod_bitmap_icon_obj_t *)obj_data);
      break;
    
    case SYMPROD_OBJ_ARC :
      SYMPRODprintArcObject(stream, 
			    (symprod_arc_obj_t *)obj_data);
      break;
    
    case SYMPROD_OBJ_RECTANGLE :
      SYMPRODprintRectangleObject(stream, 
				  (symprod_rectangle_obj_t *)obj_data);
      break;
    
    case SYMPROD_OBJ_CHUNK :
      SYMPRODprintChunkObject(stream, 
			      (symprod_chunk_obj_t *)obj_data);
      break;
    } /* endswitch - obj_hdr->object_type */
  
  } /* endfor - i */
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintObject() - Print an object structure to the given stream
 *                        in ASCII format.
 */

void SYMPRODprintObject(FILE *stream,
			symprod_object_t *object)
{
  fprintf(stream, "\n");
  fprintf(stream, "SYMPROD object information:\n");
  fprintf(stream, "\n");

  fprintf(stream, "   object type = ");
  print_object_type(stream, object->object_type);
  fprintf(stream, "\n");
  
  fprintf(stream, "   num bytes = %d\n", object->num_bytes);
  fprintf(stream, "   detail level = %d\n", object->detail_level);
  fprintf(stream, "   color = '%s'\n", object->color);
  fprintf(stream, "   background color = '%s'\n", object->background_color);
  fprintf(stream, "   centroid lat = %f\n", object->centroid.lat);
  fprintf(stream, "   centroid lon = %f\n", object->centroid.lon);
  
  switch(object->object_type)
  {
  case SYMPROD_OBJ_TEXT :
    SYMPRODprintTextObject(stream, &(object->object_info->text));
    break;
    
  case SYMPROD_OBJ_POLYLINE :
    SYMPRODprintPolylineObject(stream, &(object->object_info->polyline));
    break;
    
  case SYMPROD_OBJ_STROKED_ICON :
    SYMPRODprintStrokedIconObject(stream, &(object->object_info->stroked_icon));
    break;
    
  case SYMPROD_OBJ_NAMED_ICON :
    SYMPRODprintNamedIconObject(stream, &(object->object_info->named_icon));
    break;
    
  case SYMPROD_OBJ_BITMAP_ICON :
    SYMPRODprintBitmapIconObject(stream, &(object->object_info->bitmap_icon));
    break;
    
  case SYMPROD_OBJ_ARC :
    SYMPRODprintArcObject(stream, &(object->object_info->arc));
    break;
    
  case SYMPROD_OBJ_RECTANGLE :
    SYMPRODprintRectangleObject(stream, &(object->object_info->rectangle));
    break;
    
  case SYMPROD_OBJ_CHUNK :
    SYMPRODprintChunkObject(stream, &(object->object_info->chunk));
    break;
  }
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintTextObject() - Print a text object to the given stream
 *                            in ASCII format.
 */

void SYMPRODprintTextObject(FILE *stream,
			    symprod_text_obj_t *object)
{
  fprintf(stream, "\n");
  fprintf(stream, "Text Object information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   origin lat = %f\n", object->origin.lat);
  fprintf(stream, "   origin lon = %f\n", object->origin.lon);
  fprintf(stream, "   offset X = %ld\n", (long)object->offset.x);
  fprintf(stream, "   offset Y = %ld\n", (long)object->offset.y);
  
  fprintf(stream, "   vertical alignment = ");
  print_vert_align(stream, object->vert_alignment);
  fprintf(stream, "\n");
  
  fprintf(stream, "   horizontal alignment = ");
  print_horiz_align(stream, object->horiz_alignment);
  fprintf(stream, "\n");
  
  fprintf(stream, "   size = %ld\n", (long)object->size);
  fprintf(stream, "   length = %ld\n", (long)object->length);
  fprintf(stream, "   string = '%s'\n", object->string);
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintPolylineObject() - Print a polyline object to the given
 *                                stream in ASCII format.
 */

void SYMPRODprintPolylineObject(FILE *stream,
				symprod_polyline_obj_t *object)
{
  int i;
  
  fprintf(stream, "\n");
  fprintf(stream, "Polyline Object information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   close flag = %ld\n", (long)object->close_flag);

  fprintf(stream, "   fill type = ");
  print_fill(stream, object->fill);
  fprintf(stream, "\n");
  
  fprintf(stream, "   line type = ");
  print_linetype(stream, object->linetype);
  fprintf(stream, "\n");
  
  fprintf(stream, "   line width = %ld\n", (long)object->linewidth);
  
  fprintf(stream, "   cap style = ");
  print_capstyle(stream, object->capstyle);
  fprintf(stream, "\n");
  
  fprintf(stream, "   join style = ");
  print_joinstyle(stream, object->joinstyle);
  fprintf(stream, "\n");
  
  fprintf(stream, "   line interpolation = ");
  print_line_interp(stream, object->line_interp);
  fprintf(stream, "\n");
  
  fprintf(stream, "   num points = %ld\n", (long)object->num_points);
  
  fprintf(stream, "   points:\n");
  for (i = 0; i < object->num_points; i++)
    fprintf(stream, "      %10.5f %10.5f\n",
	    object->points[i].lat, object->points[i].lon);
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintStrokedIconObject() - Print a stroked icon object to the
 *                                   given stream in ASCII format.
 */

void SYMPRODprintStrokedIconObject(FILE *stream,
				  symprod_stroked_icon_obj_t *object)
{
  int i;
  symprod_wpt_t *wpt;
  
  fprintf(stream, "\n");
  fprintf(stream, "Stroked Icon Object information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   num icon points = %ld\n", (long)object->num_icon_pts);
  fprintf(stream, "   num icons = %ld\n", (long)object->num_icons);
  
  fprintf(stream, "   icon points:\n");
  for (i = 0; i < object->num_icon_pts; i++)
    fprintf(stream, "      %4ld %4ld\n",
	    (long)object->icon_pts[i].x, (long)object->icon_pts[i].y);
  fprintf(stream, "\n");
  
  fprintf(stream, "   icon locations:\n");
  wpt = (symprod_wpt_t *)((char *)object +
			  sizeof(symprod_const_stroked_icon_obj_t) +
			  (object->num_icon_pts * sizeof(symprod_ppt_t)));
  for (i = 0; i < object->num_icons; i++)
  {
    fprintf(stream, "      %10.5f %10.5f\n",
	    wpt->lat, wpt->lon);
    wpt++;
  }
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintNamedIconObject() - Print a named icon object to the
 *                                 given stream in ASCII format.
 */

void SYMPRODprintNamedIconObject(FILE *stream, 
				symprod_named_icon_obj_t *object)
{
  int i;
  
  fprintf(stream, "\n");
  fprintf(stream, "Named Icon Object information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   icon name = '%s'\n", object->name);
  fprintf(stream, "   num icons = %ld\n", (long)object->num_icons);
  fprintf(stream, "   spare = %ld\n", (long)object->spare);
  
  fprintf(stream, "   icon locations:\n");
  for (i = 0; i < object->num_icons; i++)
    fprintf(stream, "      %10.5f %10.5f\n",
	    object->icon_origins[i].lat, object->icon_origins[i].lon);
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintBitmapIconObject() - Print a bitmap icon object to the
 *                                  given stream in ASCII format.
 */

void SYMPRODprintBitmapIconObject(FILE *stream,
				 symprod_bitmap_icon_obj_t *object)
{
  int i, j;
  ui08 *bit_val;
  
  fprintf(stream, "\n");
  fprintf(stream, "Bitmap Icon Object information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   bitmap X dimension = %ld\n",
	  (long)object->bitmap_x_dim);
  fprintf(stream, "   bitmap Y dimension = %ld\n",
	  (long)object->bitmap_y_dim);
  fprintf(stream, "   num icons = %ld\n",
	  (long)object->num_icons);
  fprintf(stream, "   spare = %ld\n", (long)object->spare);
  
  fprintf(stream, "   icon locations:\n");
  for (i = 0; i < object->num_icons; i++)
    fprintf(stream, "      %10.5f %10.5f\n",
	    object->icon_origins[i].lat,
	    object->icon_origins[i].lon);
  
  fprintf(stream, "   bitmap:\n");
  bit_val = (ui08 *)((char *)object +
		     sizeof(symprod_const_bitmap_icon_obj_t) +
		     object->num_icons * sizeof(symprod_wpt_t));
  for (i = 0; i < object->bitmap_y_dim; i++)
  {
    fprintf(stream, "      ");
    for (j = 0; j < object->bitmap_x_dim; j++)
    {
      fprintf(stream, "%1d ", *bit_val);
      bit_val++;
    } /* endfor - j */
    fprintf(stream, "\n");
  } /* endfor - i */
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintArcObject() - Print an arc object to the given stream in
 *                           ASCII format.
 */

void SYMPRODprintArcObject(FILE *stream, 
			  symprod_arc_obj_t *object)
{
  fprintf(stream, "\n");
  fprintf(stream, "Arc Object information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   origin lat = %f\n", object->origin.lat);
  fprintf(stream, "   origin lon = %f\n", object->origin.lon);
  fprintf(stream, "   height = %ld\n", (long)object->height);
  fprintf(stream, "   width = %ld\n", (long)object->width);
  fprintf(stream, "   angle1 = %f\n", object->angle1);
  fprintf(stream, "   angle2 = %f\n", object->angle2);
  
  fprintf(stream, "   line type = ");
  print_linetype(stream, object->linetype);
  fprintf(stream, "\n");
  
  fprintf(stream, "   line width = %ld\n", (long)object->linewidth);
  
  fprintf(stream, "   fill = ");
  print_fill(stream, object->fill);
  fprintf(stream, "\n");
  
  fprintf(stream, "   cap style = ");
  print_capstyle(stream, object->capstyle);
  fprintf(stream, "\n");
  
  fprintf(stream, "   join style = ");
  print_joinstyle(stream, object->joinstyle);
  fprintf(stream, "\n");
  
  fprintf(stream, "   spare = %ld\n", (long)object->spare);
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintRectangleObject() - Print a rectangle object to the given
 *                                 stream in ASCII format.
 */

void SYMPRODprintRectangleObject(FILE *stream,
				symprod_rectangle_obj_t *object)
{
  fprintf(stream, "\n");
  fprintf(stream, "Rectangle Object information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   origin lat = %f\n", object->origin.lat);
  fprintf(stream, "   origin lon = %f\n", object->origin.lon);
  fprintf(stream, "   height = %ld\n", (long)object->height);
  fprintf(stream, "   width = %ld\n", (long)object->width);
  
  fprintf(stream, "   line type = ");
  print_linetype(stream, object->linetype);
  fprintf(stream, "\n");
  
  fprintf(stream, "   line width = %ld\n", (long)object->linewidth);
  
  fprintf(stream, "   fill type = ");
  print_fill(stream, object->fill);
  fprintf(stream, "\n");
  
  fprintf(stream, "   cap style = ");
  print_capstyle(stream, object->capstyle);
  fprintf(stream, "\n");
  
  fprintf(stream, "   join style = ");
  print_joinstyle(stream, object->joinstyle);
  fprintf(stream, "\n");
  
  fprintf(stream, "   spare = %ld\n", (long)object->spare);
  
  fprintf(stream, "\n");
  
  return;
}


/**********************************************************************
 * SYMPRODprintChunkObject() - Print a chunk object to the given stream
 *                             in ASCII format.  Currently, the chunk
 *                             data in the object is printed as a string.
 */

void SYMPRODprintChunkObject(FILE *stream,
			    symprod_chunk_obj_t *object)
{
  fprintf(stream, "\n");
  fprintf(stream, "Chunk Object information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   chunk_type = %d\n", object->chunk_type);
  fprintf(stream, "   user_data = %d\n", object->user_data);
  fprintf(stream, "\n");
/*  fprintf(stream, "   data as a string = %s\n", object->data); */
  
  fprintf(stream, "\n");
  
  return;
}


/*********************************************************************
 * SYMPRODinitBbox()
 *
 * Initialize bounding box
 */

void SYMPRODinitBbox(symprod_box_t *bb)

{
  bb->min_lat = 360.0;
  bb->max_lat = -360.0;
  bb->min_lon = 720.0;
  bb->max_lon = -720.0;
}

/*********************************************************************
 * SYMPRODupdateBboxPoint()
 *
 * Update a bounding box with a point
 */

void SYMPRODupdateBboxPoint(symprod_box_t *bb,
			    double lat, double lon)
     
{

  if (lat < bb->min_lat) {
    bb->min_lat = lat;
  }

  if (lat > bb->max_lat) {
    bb->max_lat = lat;
  }

  if (lon < bb->min_lon) {
    bb->min_lon = lon;
  }

  if (lon > bb->max_lon) {
    bb->max_lon = lon;
  }

}

/*********************************************************************
 * SYMPRODupdateBbox()
 *
 * Update one bounding box with another
 */

void SYMPRODupdateBbox(symprod_box_t *master_bb,
		       symprod_box_t *bb)

{

  if (bb->min_lat < master_bb->min_lat) {
    master_bb->min_lat = bb->min_lat;
  }

  if (bb->max_lat > master_bb->max_lat) {
    master_bb->max_lat = bb->max_lat;
  }

  if (bb->min_lon < master_bb->min_lon) {
    master_bb->min_lon = bb->min_lon;
  }

  if (bb->max_lon > master_bb->max_lon) {
    master_bb->max_lon = bb->max_lon;
  }

}

/**********************************************************************
 * Static routines.
 **********************************************************************/


/**********************************************************************
 * print_object_type
 */

static void print_object_type(FILE *stream, int type)
{
  switch(type)
  {
  case SYMPROD_OBJ_TEXT :
    fprintf(stream, "SYMPROD_OBJ_TEXT");
    break;
    
  case SYMPROD_OBJ_POLYLINE :
    fprintf(stream, "SYMPROD_OBJ_POLYLINE");
    break;
    
  case SYMPROD_OBJ_STROKED_ICON :
    fprintf(stream, "SYMPROD_OBJ_STROKED_ICON");
    break;
    
  case SYMPROD_OBJ_NAMED_ICON :
    fprintf(stream, "SYMPROD_OBJ_NAMED_ICON");
    break;
    
  case SYMPROD_OBJ_BITMAP_ICON :
    fprintf(stream, "SYMPROD_OBJ_BITMAP_ICON");
    break;
    
  case SYMPROD_OBJ_ARC :
    fprintf(stream, "SYMPROD_OBJ_ARC");
    break;
    
  case SYMPROD_OBJ_RECTANGLE :
    fprintf(stream, "SYMPROD_OBJ_RECTANGLE");
    break;
    
  case SYMPROD_OBJ_CHUNK :
    fprintf(stream, "SYMPROD_OBJ_CHUNK");
    break;
    
  default:
    fprintf(stream, "UNKNOWN OBJECT TYPE %d", type);
    break;
  }
  
  return;
}


/**********************************************************************
 * print_vert_align
 */

static void print_vert_align(FILE *stream, int align)
{
  switch(align)
  {
  case SYMPROD_TEXT_VERT_ALIGN_TOP :
    fprintf(stream, "SYMPROD_TEXT_VERT_ALIGN_TOP");
    break;
    
  case SYMPROD_TEXT_VERT_ALIGN_CENTER :
    fprintf(stream, "SYMPROD_TEXT_VERT_ALIGN_CENTER");
    break;
    
  case SYMPROD_TEXT_VERT_ALIGN_BOTTOM :
    fprintf(stream, "SYMPROD_TEXT_VERT_ALIGN_BOTTOM");
    break;
    
  default:
    fprintf(stream, "UNKNOWN VERTICAL ALIGNMENT %d", align);
    break;
  }
  
  return;
}


/**********************************************************************
 * print_horiz_align
 */

static void print_horiz_align(FILE *stream, int align)
{
  switch(align)
  {
  case SYMPROD_TEXT_HORIZ_ALIGN_LEFT :
    fprintf(stream, "SYMPROD_TEXT_HORIZ_ALIGN_LEFT");
    break;
    
  case SYMPROD_TEXT_HORIZ_ALIGN_CENTER :
    fprintf(stream, "SYMPROD_TEXT_HORIZ_ALIGN_CENTER");
    break;
    
  case SYMPROD_TEXT_HORIZ_ALIGN_RIGHT :
    fprintf(stream, "SYMPROD_TEXT_HORIZ_ALIGN_RIGHT");
    break;
    
  default:
    fprintf(stream, "UNKNOWN HORIZONTAL ALIGNMENT %d", align);
    break;
  }
  
  return;
}


/**********************************************************************
 * print_fill
 */

static void print_fill(FILE *stream, int fill)
{
  switch(fill)
  {
  case SYMPROD_FILL_NONE :
    fprintf(stream, "SYMPROD_FILL_NONE");
    break;
    
  case SYMPROD_FILL_STIPPLE10 :
    fprintf(stream, "SYMPROD_FILL_STIPPLE10");
    break;
    
  case SYMPROD_FILL_STIPPLE20 :
    fprintf(stream, "SYMPROD_FILL_STIPPLE20");
    break;
    
  case SYMPROD_FILL_STIPPLE30 :
    fprintf(stream, "SYMPROD_FILL_STIPPLE30");
    break;
    
  case SYMPROD_FILL_STIPPLE40 :
    fprintf(stream, "SYMPROD_FILL_STIPPLE40");
    break;
    
  case SYMPROD_FILL_STIPPLE50 :
    fprintf(stream, "SYMPROD_FILL_STIPPLE50");
    break;
    
  case SYMPROD_FILL_STIPPLE60 :
    fprintf(stream, "SYMPROD_FILL_STIPPLE60");
    break;
    
  case SYMPROD_FILL_STIPPLE70 :
    fprintf(stream, "SYMPROD_FILL_STIPPLE70");
    break;
    
  case SYMPROD_FILL_STIPPLE80 :
    fprintf(stream, "SYMPROD_FILL_STIPPLE80");
    break;
    
  case SYMPROD_FILL_STIPPLE90 :
    fprintf(stream, "SYMPROD_FILL_STIPPLE90");
    break;
    
  case SYMPROD_FILL_SOLID :
    fprintf(stream, "SYMPROD_FILL_SOLID");
    break;
    
  default:
    fprintf(stream, "UNKNOWN FILL TYPE %d", fill);
    break;
  }
  
  return;
}


/**********************************************************************
 * print_linetype
 */

static void print_linetype(FILE *stream, int linetype)
{
  switch(linetype)
  {
  case SYMPROD_LINETYPE_SOLID :
    fprintf(stream, "SYMPROD_LINETYPE_SOLID");
    break;
    
  case SYMPROD_LINETYPE_DASH :
    fprintf(stream, "SYMPROD_LINETYPE_DASH");
    break;
    
  case SYMPROD_LINETYPE_DOT_DASH :
    fprintf(stream, "SYMPROD_LINETYPE_DOT_DASH");
    break;
    
  default:
    fprintf(stream, "UNKNOWN LINETYPE %d", linetype);
    break;
  }
  
  return;
}


/**********************************************************************
 * print_capstyle
 */

static void print_capstyle(FILE *stream, int capstyle)
{
  switch(capstyle)
  {
  case SYMPROD_CAPSTYLE_BUTT :
    fprintf(stream, "SYMPROD_CAPSTYLE_BUTT");
    break;
    
  case SYMPROD_CAPSTYLE_NOT_LAST :
    fprintf(stream, "SYMPROD_CAPSTYLE_NOT_LAST");
    break;
    
  case SYMPROD_CAPSTYLE_PROJECTING :
    fprintf(stream, "SYMPROD_CAPSTYLE_PROJECTING");
    break;
    
  case SYMPROD_CAPSTYLE_ROUND :
    fprintf(stream, "SYMPROD_CAPSTYLE_ROUND");
    break;
    
  default:
    fprintf(stream, "UNKNOWN CAPSTYLE %d", capstyle);
    break;
  }
  
  return;
}


/**********************************************************************
 * print_joinstyle
 */

static void print_joinstyle(FILE *stream, int joinstyle)
{
  switch(joinstyle)
  {
  case SYMPROD_JOINSTYLE_BEVEL :
    fprintf(stream, "SYMPROD_JOINSTYLE_BEVEL");
    break;
    
  case SYMPROD_JOINSTYLE_MITER :
    fprintf(stream, "SYMPROD_JOINSTYLE_MITER");
    break;
    
  case SYMPROD_JOINSTYLE_ROUND :
    fprintf(stream, "SYMPROD_JOINSTYLE_ROUND");
    break;
    
  default:
    fprintf(stream, "UNKNOWN JOINSTYLE %d", joinstyle);
    break;
  }
  
  return;
}


/**********************************************************************
 * print_line_interp
 */

static void print_line_interp(FILE *stream, int line_interp)
{
  switch(line_interp)
  {
  case SYMPROD_LINE_INTERP_STRAIGHT :
    fprintf(stream, "SYMPROD_LINE_INTERP_STRAIGHT");
    break;
    
  case SYMPROD_LINE_INTERP_BEZIER :
    fprintf(stream, "SYMPROD_LINE_INTERP_BEZIER");
    break;
    
  case SYMPROD_LINE_INTERP_CUBIC_SPLINE :
    fprintf(stream, "SYMPROD_LINE_INTERP_CUBIC_SPLINE");
    break;
    
  default:
    fprintf(stream, "UNKNOWN LINE INTERPOLATION %d", line_interp);
    break;
  }
  
  return;
}

/**********************************************************************
 * add_arrow() - Add an arrow object to the internal product
 *               structure.
 *
 * head_len:        in km
 * head_half_angle: in deg
 */

static void add_arrow(symprod_product_t *prod,
		      const char *color,
		      int linetype,
		      int linewidth,
		      int capstyle,
		      int joinstyle,
		      double start_lat,
		      double start_lon,
		      double end_lat,
		      double end_lon,
		      double length,
		      double dirn,
		      double head_len,
		      double head_half_angle)

{


  symprod_wpt_t arrow[5];
  
  double angle;
  double lat, lon;

  /*
   * compute the arrow points
   */

  arrow[0].lat = start_lat;
  arrow[0].lon = start_lon;

  arrow[1].lat = end_lat;
  arrow[1].lon = end_lon;

  PJGLatLon2RTheta(start_lat, start_lon, end_lat, end_lon,
		   &length, &dirn);

  angle = dirn + 180.0 + head_half_angle;

  PJGLatLonPlusRTheta(end_lat, end_lon, head_len, angle,
		      &lat, &lon);

  arrow[2].lat = lat;
  arrow[2].lon = lon;

  arrow[3].lat = end_lat;
  arrow[3].lon = end_lon;

  angle = dirn + 180.0 - head_half_angle;

  PJGLatLonPlusRTheta(end_lat, end_lon, head_len, angle,
		      &lat, &lon);

  arrow[4].lat = lat;
  arrow[4].lon = lon;

  /*
   * add polyline object
   */
  
  SYMPRODaddPolyline(prod, color, linetype, linewidth,
		     capstyle, joinstyle,
		     FALSE, SYMPROD_FILL_NONE,
		     arrow, 5);

  return;

}


