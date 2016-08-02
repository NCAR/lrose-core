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
 * bdry.c: Routines to manipulate boundary (CLD, etc.) data.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/os_config.h>

#include <dataport/port_types.h>
#include <dataport/bigend.h>

#include <rapformats/ascii_shapeio.h>
#include <rapformats/bdry.h>

#include <toolsa/mem.h>
#include <toolsa/membuf.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>


/*
 * Define values used for parsing the input file.
 */

#define PRODUCT_KEY         "PRODUCT "
#define GENERATE_TIME_KEY   "GENERATE_TIME"
#define PRODUCT_TIME_KEY    "PRODUCT_TIME"
#define PRODUCT_DESC_KEY    "PRODUCT_DESCRIPTION"
#define PRODUCT_MOTION_KEY  "PRODUCT_MOTION"
#define DETECTION_ATTR_KEY  "DETECTION_ATTR"
#define POLYLINE_KEY        "POLYLINE"

/*
 * Define values used for parsing data type
 */

#define MASK1               255
#define MASK2               65280
#define MASK3               16711935
#define SUBTYPE_BIAS        99

/*
 * Define tables for converting input line values to structure
 * values.
 */

typedef struct
{
  char parse_string[80];
  int  parse_value;
} parse_table_t;


static parse_table_t Type_table[] =
{ { "BDT",  BDRY_TYPE_BDRY_TRUTH },
  { "BDM",  BDRY_TYPE_BDRY_MIGFA },
  { "BDC",  BDRY_TYPE_BDRY_COLIDE },
  { "ANI",  BDRY_TYPE_COMB_NC_ISSUE },
  { "ANV",  BDRY_TYPE_COMB_NC_VALID },
  { "AXI",  BDRY_TYPE_EXTRAP_ISSUE_ANW },
  { "AXV",  BDRY_TYPE_EXTRAP_VALID_ANW },
  { "FGI",  BDRY_TYPE_FIRST_GUESS_ISSUE },
  { "FGV",  BDRY_TYPE_FIRST_GUESS_VALID },
  { "MMI",  BDRY_TYPE_MINMAX_NC_ISSUE },
  { "MMV",  BDRY_TYPE_MINMAX_NC_VALID } };

static int Type_table_size =
  sizeof(Type_table) / sizeof(parse_table_t);

static parse_table_t Subtype_table[] =
{ { "ALL",  BDRY_SUBTYPE_ALL } };

static int Subtype_table_size =
  sizeof(Subtype_table) / sizeof(parse_table_t);

static parse_table_t Line_type_table[] =
{ { "COMBINED_LINE",      BDRY_LINE_TYPE_COMBINED },
  { "EXTRAPOLATED_LINE",  BDRY_LINE_TYPE_EXTRAPOLATED },
  { "SHEAR_LINE",         BDRY_LINE_TYPE_SHEAR },
  { "THIN_LINE",          BDRY_LINE_TYPE_THIN },
  { "EMPTY_LINE",         BDRY_LINE_TYPE_EMPTY },
  { "GENERIC_LINE",       BDRY_LINE_TYPE_GENERIC} };

static int Line_type_table_size =
  sizeof(Line_type_table) / sizeof(parse_table_t);


/*
 * Prototypes for static functions
 */

static int convert_parse_value(const char *string,
			       parse_table_t *parse_table,
			       int parse_table_size);

static int get_extrapolated_prod_type(const char *type_string);

static void point_from_BE(BDRY_spdb_point_t *point);

static void point_to_BE(BDRY_spdb_point_t *point);

static void polyline_from_BE(BDRY_spdb_polyline_t *polyline);

static void polyline_to_BE(BDRY_spdb_polyline_t *polyline);

static void print_point(FILE *stream, BDRY_spdb_point_t *point);

static void print_polyline(FILE *stream, BDRY_spdb_polyline_t *polyline,
			   int print_points);


  
/************************************************************************
 * BDRY_free_product(): Frees the space used by a BDRY_product_t
 *                      structure.  Assumes that pointers in the structure
 *                      are set to NULL if they don't point to anything.
 */

void BDRY_free_product(BDRY_product_t *product)
{
  int i;
  
  /*
   * Free each polyline in the product.
   */

  for (i = 0; i < product->num_polylines; i++)
  {
    if (product->polylines[i].object_label != (const char *)NULL)
      ufree((void*)product->polylines[i].object_label);
    
    if (product->polylines[i].points != (BDRY_point_t *)NULL)
      ufree((void*)product->polylines[i].points);
    
  } /* endfor - i */
  
  /*
   * Now free the information at the main level.
   */

  if (product->type_string != (const char *)NULL)
    ufree((void*)product->type_string);
  
  if (product->subtype_string != (const char *)NULL)
    ufree((void*)product->subtype_string);
  
  if (product->line_type_string != (const char *)NULL)
    ufree((void*)product->line_type_string);
  
  if (product->desc != (char *)NULL)
    ufree((void*)product->desc);
  
  if (product->polylines != (BDRY_polyline_t *)NULL)
    ufree((void*)product->polylines);
  
  return;
}


/************************************************************************
 * BDRY_line_type_string_to_line_type(): Returns the integer line type
 *                                       for a given line type string.
 */

int BDRY_line_type_string_to_line_type(const char *line_type_string)
{
  return(convert_parse_value(line_type_string,
			     Line_type_table,
			     Line_type_table_size));
}


/************************************************************************
 * BDRY_print_spdb_product(): Print an SPDB boundary in ASCII format to
 *                            the given stream.
 */

void BDRY_print_spdb_product(FILE *stream, BDRY_spdb_product_t *prod,
			     int print_points)
{
  int i;
  int polyline;
  int polyline_offset;
  
  fprintf(stream, "\nBoundary Product:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   type = %d\n", prod->type);
  fprintf(stream, "   subtype = %d\n", prod->subtype);
  fprintf(stream, "   sequence_num = %d\n", prod->sequence_num);
  fprintf(stream, "   group_id = %d\n", prod->group_id);
  fprintf(stream, "   generate_time = %s\n", utimstr(prod->generate_time));
  fprintf(stream, "   data_time = %s\n", utimstr(prod->data_time));
  fprintf(stream, "   forecast_time = %s\n", utimstr(prod->forecast_time));
  fprintf(stream, "   expire_time = %s\n", utimstr(prod->expire_time));
  fprintf(stream, "   line_type = %d\n", prod->line_type);
  fprintf(stream, "   bdry_id = %d\n", prod->bdry_id);
  fprintf(stream, "   num_polylines = %d\n", prod->num_polylines);

  for (i = 0; i < BDRY_SPARE_INT_LEN; i++)
    fprintf(stream, "   spare_int[%d] = %d\n", i, prod->spare_int[i]);
  fprintf(stream, "   motion_direction = %f\n", prod->motion_direction);
  fprintf(stream, "   motion_speed = %f\n", prod->motion_speed);
  fprintf(stream, "   line_quality_value = %f\n", prod->line_quality_value);
  fprintf(stream, "   line_quality_thresh = %f\n", prod->line_quality_thresh);
  for (i = 0; i < BDRY_SPARE_FLOAT_LEN; i++)
    fprintf(stream, "   spare_float[%d] = %f\n", i, prod->spare_float[i]);
  fprintf(stream, "   type_string = <%s>\n", prod->type_string);
  fprintf(stream, "   subtype_string = <%s>\n", prod->subtype_string);
  fprintf(stream, "   line_type_string = <%s>\n", prod->line_type_string);
  fprintf(stream, "   desc = <%s>\n", prod->desc);

  polyline_offset = sizeof(BDRY_spdb_product_t) - sizeof(BDRY_spdb_polyline_t);
  
  for (polyline = 0; polyline < prod->num_polylines; polyline++)
  {
    BDRY_spdb_polyline_t *polyline_ptr =
      (BDRY_spdb_polyline_t *)((char *)prod + polyline_offset);
    
    print_polyline(stream, polyline_ptr, print_points);
  
    polyline_offset += sizeof(BDRY_spdb_polyline_t) +
      (polyline_ptr->num_pts - 1) * sizeof(BDRY_spdb_point_t);
    
  }
  
  return;
}


/************************************************************************
 * BDRY_product_to_spdb(): Converts a BDRY_product_t structure to a
 *                         BDRY_spdb_product_t buffer.  Returns the length
 *                         of the SPDB buffer in the argument buffer_len.
 *
 * Note that this routine returns a pointer to static memory that
 * should NOT be freed by the calling routine.
 */

BDRY_spdb_product_t *BDRY_product_to_spdb(BDRY_product_t *product,
					  int *buffer_len)
{
  static MEMbuf *mem_buf = NULL;
  
  BDRY_spdb_product_t spdb_product;
  BDRY_spdb_polyline_t spdb_polyline;
  BDRY_spdb_point_t spdb_point;
  int i, j;
  
  /*
   * Initialize the static memory buffer.
   */

  if (mem_buf == NULL)
    mem_buf = MEMbufCreate();
  else
    MEMbufReset(mem_buf);
  
  /*
   * Put the main product information into the buffer.
   */

  memset(&spdb_product, 0, sizeof(spdb_product));
  
  spdb_product.type          = product->type;
  spdb_product.subtype       = product->subtype;
  spdb_product.sequence_num  = product->sequence_num;
  spdb_product.group_id      = product->group_id;
  spdb_product.generate_time = product->generate_time;
  spdb_product.data_time     = product->data_time;
  spdb_product.forecast_time = product->forecast_time;
  spdb_product.expire_time   = product->expire_time;
  spdb_product.line_type     = product->line_type;
  spdb_product.bdry_id       = product->bdry_id;
  spdb_product.num_polylines = product->num_polylines;
  
  spdb_product.motion_direction     = product->motion_direction;
  spdb_product.motion_speed         = product->motion_speed;
  spdb_product.line_quality_value   = product->line_quality_value;
  spdb_product.line_quality_thresh  = product->line_quality_thresh;
  
  STRcopy(spdb_product.type_string, product->type_string, BDRY_TYPE_LEN);
  STRcopy(spdb_product.subtype_string, product->subtype_string,
	  BDRY_TYPE_LEN);
  STRcopy(spdb_product.line_type_string, product->line_type_string,
	  BDRY_LINE_TYPE_LEN);
  STRcopy(spdb_product.desc, product->desc, BDRY_DESC_LEN);
  
  MEMbufAdd(mem_buf, (void *)&spdb_product,
	    sizeof(BDRY_spdb_product_t) - sizeof(BDRY_spdb_polyline_t));
  
  /*
   * Now add each of the polylines to the buffer.
   */

  for (i = 0; i < product->num_polylines; i++)
  {
    BDRY_polyline_t *polyline = &product->polylines[i];
    
    memset(&spdb_polyline, 0, sizeof(BDRY_spdb_polyline_t));
    
    spdb_polyline.num_pts    = polyline->num_pts;
    spdb_polyline.num_secs   = polyline->num_secs;
    
    STRcopy(spdb_polyline.object_label, polyline->object_label,
	    BDRY_LABEL_LEN);
    
    MEMbufAdd(mem_buf, (void *)&spdb_polyline,
	      sizeof(BDRY_spdb_polyline_t) - sizeof(BDRY_spdb_point_t));
    
    /*
     * Add each of the points in the polyline to the buffer.
     */

    for (j = 0; j < polyline->num_pts; j++)
    {
      BDRY_point_t *point = &polyline->points[j];
      
      memset(&spdb_point, 0, sizeof(BDRY_spdb_point_t));
      
      spdb_point.lat     = point->lat;
      spdb_point.lon     = point->lon;
      spdb_point.u_comp  = point->u_comp;
      spdb_point.v_comp  = point->v_comp;
      spdb_point.value   = point->value;
      
      MEMbufAdd(mem_buf, (void *)&spdb_point, sizeof(BDRY_spdb_point_t));
      
    } /* endfor - j */
    
  } /* endfor - i */
  
  *buffer_len = MEMbufLen(mem_buf);
  
  return((BDRY_spdb_product_t *)MEMbufPtr(mem_buf));
}


/************************************************************************
 * BDRY_spdb_product_from_BE(): Convert a boundary product from big-endian
 *                         format to native format.
 *
 * Returns the number of bytes in the swapped product.  This can be
 * used to easily find the next product in a buffer containing several
 * products.
 */

int BDRY_spdb_product_from_BE(BDRY_spdb_product_t *prod)
{
  int polyline;
  int polyline_offset;
  
  int product_size = 0;
  
  prod->type                = BE_to_si32(prod->type);
  prod->subtype             = BE_to_si32(prod->subtype);
  prod->sequence_num        = BE_to_si32(prod->sequence_num);
  prod->group_id            = BE_to_si32(prod->group_id);
  prod->generate_time       = BE_to_si32(prod->generate_time);
  prod->data_time           = BE_to_si32(prod->data_time);
  prod->forecast_time       = BE_to_si32(prod->forecast_time);
  prod->expire_time         = BE_to_si32(prod->expire_time);
  prod->line_type           = BE_to_si32(prod->line_type);
  prod->bdry_id             = BE_to_si32(prod->bdry_id);
  prod->num_polylines       = BE_to_si32(prod->num_polylines);

  BE_to_array_32(prod->spare_int, BDRY_SPARE_INT_LEN * sizeof(si32));
  
  BE_to_array_32(&prod->motion_direction, 4);
  BE_to_array_32(&prod->motion_speed, 4);
  BE_to_array_32(&prod->line_quality_value, 4);
  BE_to_array_32(&prod->line_quality_thresh, 4);

  BE_to_array_32(prod->spare_float, BDRY_SPARE_FLOAT_LEN * sizeof(fl32));
  
  /* type_string is okay */
  /* subtype_string is okay */
  /* line_type_string is okay */
  /* desc is okay */

  polyline_offset = sizeof(BDRY_spdb_product_t) - sizeof(BDRY_spdb_polyline_t);
  
  product_size += polyline_offset;
  
  for (polyline = 0; polyline < prod->num_polylines; polyline++)
  {
    int polyline_size;
    
    BDRY_spdb_polyline_t *polyline_ptr =
      (BDRY_spdb_polyline_t *)((char *)prod + polyline_offset);
    
    polyline_from_BE(polyline_ptr);
  
    polyline_size = sizeof(BDRY_spdb_polyline_t) +
      (polyline_ptr->num_pts - 1) * sizeof(BDRY_spdb_point_t);
    
    polyline_offset += polyline_size;
    product_size += polyline_size;
  }
  
  return(product_size);
}


/************************************************************************
 * BDRY_spdb_product_from_SIO_shape(): Converts SIO shape information read in
 *                                from an ASCII file into a boundary
 *                                product buffer.
 *
 * Returns a pointer to a static buffer which should NOT be freed by the
 * calling routine.  Returns NULL if there is an error.
 * Also returns the size in bytes of the product buffer returned.
 */

BDRY_spdb_product_t *BDRY_spdb_product_from_SIO_shape(SIO_shape_data_t *shape,
						      int *prod_size)   /* returned */
{
  static BDRY_spdb_product_t *buffer = NULL;
  static int buffer_alloc = 0;
  
  int obj;
  int polyline_offset;
  
  /*
   * Determine how big the return buffer needs to be.  Note that we
   * subtract one when adding the sizes of the structures in arrays
   * since there is one of these structures already included in the
   * size of the parent structure.
   */

  *prod_size = sizeof(BDRY_spdb_product_t) +
    (shape->num_objects - 1) * sizeof(BDRY_spdb_polyline_t);
  
  for (obj = 0; obj < shape->num_objects; obj++)
    *prod_size += (shape->P->npoints - 1) * sizeof(BDRY_spdb_point_t);
  
  /*
   * Make sure the return buffer is big enough.
   */

  if (*prod_size > buffer_alloc)
  {
    if (buffer == NULL)
      buffer = (BDRY_spdb_product_t *)umalloc(*prod_size);
    else
      buffer = (BDRY_spdb_product_t *)urealloc(buffer,
					  *prod_size);
    
    buffer_alloc = *prod_size;
  }
  
  /*
   * Update the main product structure.
   */

  buffer->type = BDRY_type_string_to_type(shape->type);
  buffer->subtype = BDRY_subtype_string_to_subtype(shape->sub_type);
  buffer->sequence_num = shape->sequence_number;
  buffer->group_id = shape->group_id;
  buffer->generate_time = shape->gen_time;
  buffer->data_time = shape->data_time;
  buffer->forecast_time = shape->valid_time;
  buffer->expire_time = shape->expire_time;
  buffer->line_type =
    BDRY_line_type_string_to_line_type(shape->line_type);
  buffer->num_polylines = shape->num_objects;
  memset(buffer->spare_int, 0, sizeof(buffer->spare_int));
  
  buffer->motion_direction = shape->motion_dir;
  buffer->motion_speed = shape->motion_speed;
  buffer->line_quality_value = shape->qual_value;
  buffer->line_quality_thresh = shape->qual_thresh;
  memset(buffer->spare_float, 0, sizeof(buffer->spare_float));
  
  STRcopy(buffer->type_string, shape->type, BDRY_TYPE_LEN);
  STRcopy(buffer->subtype_string, shape->sub_type, BDRY_TYPE_LEN);
  STRcopy(buffer->line_type_string, shape->line_type, BDRY_LINE_TYPE_LEN);
  STRcopy(buffer->desc, shape->description, BDRY_DESC_LEN);

  buffer->bdry_id = atoi(buffer->desc);
  
  /*
   * Update the data for each of the polylines.
   */

  polyline_offset = sizeof(BDRY_spdb_product_t) - sizeof(BDRY_spdb_polyline_t);
  
  for (obj = 0; obj < shape->num_objects; obj++)
  {
    int pt;
    
    BDRY_spdb_polyline_t *polyline =
      (BDRY_spdb_polyline_t *)((char *)buffer + polyline_offset);
    
    polyline->num_pts = shape->P[obj].npoints;
    polyline->num_secs = shape->P[obj].nseconds;
    memset(polyline->spare, 0, sizeof(polyline->spare));
    
    STRcopy(polyline->object_label, shape->P[obj].object_label,
	    BDRY_LABEL_LEN);
    
    for (pt = 0; pt < polyline->num_pts; pt++)
    {
      polyline->points[pt].lat = shape->P[obj].lat[pt];
      polyline->points[pt].lon = shape->P[obj].lon[pt];

      if (shape->P[obj].u_comp == NULL)
	polyline->points[pt].u_comp = BDRY_VALUE_UNKNOWN;
      else if (shape->P[obj].u_comp[pt] == SIO_MISSING_UV)
	 polyline->points[pt].u_comp = BDRY_VALUE_UNKNOWN;
      else
	polyline->points[pt].u_comp = shape->P[obj].u_comp[pt];

      if (shape->P[obj].v_comp == NULL)
	polyline->points[pt].v_comp = BDRY_VALUE_UNKNOWN;
      else if (shape->P[obj].v_comp[pt] == SIO_MISSING_UV)
	 polyline->points[pt].v_comp = BDRY_VALUE_UNKNOWN;
      else
	polyline->points[pt].v_comp = shape->P[obj].v_comp[pt];

      if (shape->P[obj].value == NULL)
	polyline->points[pt].value = 0;
      else
	polyline->points[pt].value = shape->P[obj].value[pt];

      memset(polyline->points[pt].spare, 0,
	     sizeof(polyline->points[pt].spare));
    } /* endfor - pt */
    
    polyline_offset += sizeof(BDRY_spdb_polyline_t) +
      (polyline->num_pts - 1) * sizeof(BDRY_spdb_point_t);
    
  } /* endfor - obj */
  
  return(buffer);
}


/************************************************************************
 * BDRY_spdb_product_to_BE(): Convert a boundary product from native format
 *                       to big-endian format.
 */

void BDRY_spdb_product_to_BE(BDRY_spdb_product_t *prod)
{
  int polyline;
  int num_polylines = prod->num_polylines;
  int polyline_offset;
  
  prod->type                = BE_from_si32(prod->type);
  prod->subtype             = BE_from_si32(prod->subtype);
  prod->sequence_num        = BE_from_si32(prod->sequence_num);
  prod->group_id            = BE_from_si32(prod->group_id);
  prod->generate_time       = BE_from_si32(prod->generate_time);
  prod->data_time           = BE_from_si32(prod->data_time);
  prod->forecast_time       = BE_from_si32(prod->forecast_time);
  prod->expire_time         = BE_from_si32(prod->expire_time);
  prod->line_type           = BE_from_si32(prod->line_type);
  prod->bdry_id             = BE_from_si32(prod->bdry_id);
  prod->num_polylines       = BE_from_si32(prod->num_polylines);
  BE_from_array_32(prod->spare_int, BDRY_SPARE_INT_LEN * sizeof(si32));
  
  BE_from_array_32(&prod->motion_direction, 4);
  BE_from_array_32(&prod->motion_speed, 4);
  BE_from_array_32(&prod->line_quality_value, 4);
  BE_from_array_32(&prod->line_quality_thresh, 4);
  BE_from_array_32(prod->spare_float, BDRY_SPARE_FLOAT_LEN * sizeof(fl32));
  
  /* type_string is okay */
  /* subtype_string is okay */
  /* line_type_string is okay */
  /* desc is okay */

  polyline_offset = sizeof(BDRY_spdb_product_t) - sizeof(BDRY_spdb_polyline_t);
  
  for (polyline = 0; polyline < num_polylines; polyline++)
  {
    int num_points;
    
    BDRY_spdb_polyline_t *polyline_ptr =
      (BDRY_spdb_polyline_t *)((char *)prod + polyline_offset);
    
    num_points = polyline_ptr->num_pts;
    
    polyline_to_BE(polyline_ptr);
  
    polyline_offset += sizeof(BDRY_spdb_polyline_t) +
      (num_points - 1) * sizeof(BDRY_spdb_point_t);
  }
  
  return;
}


/************************************************************************
 * BDRY_spdb_product_to_SIO_shape(): Converts a boundary product buffer
 *                              (probably just retrieved from an SPDB
 *                              database) into the internal SIO shape
 *                              structure.  Returns a pointer to memory
 *                              allocated by this routine that should be
 *                              freed by the calling routine.  Returns
 *                              NULL if there is an error.
 */

SIO_shape_data_t *BDRY_spdb_product_to_SIO_shape(BDRY_spdb_product_t *bdry)
{
  SIO_shape_data_t *shape;
  
  int obj;
  int polyline_offset;
  
  /*
   * Allocate space for the returned structure.
   */

  shape = (SIO_shape_data_t *)umalloc(sizeof(SIO_shape_data_t));
  
  /*
   * Update the main shape structure.
   */

  shape->type = STRdup(bdry->type_string);
  shape->sub_type = STRdup(bdry->subtype_string);
  shape->sequence_number = bdry->sequence_num;
  shape->group_id = bdry->group_id;
  shape->gen_time = bdry->generate_time;
  shape->data_time = bdry->data_time;
  shape->valid_time = bdry->forecast_time;
  shape->expire_time = bdry->expire_time;
  STRcopy(shape->description, bdry->desc, SIO_LABEL_LEN);
  shape->motion_dir = bdry->motion_direction;
  shape->motion_speed = bdry->motion_speed;
  shape->line_type = STRdup(bdry->line_type_string);
  shape->qual_value = bdry->line_quality_value;
  shape->qual_thresh = bdry->line_quality_thresh;
  shape->num_objects = bdry->num_polylines;
  
  /*
   * Allocate space for the object structures.
   */

  shape->P =
    (SIO_polyline_object_t *)umalloc(shape->num_objects *
				     sizeof(SIO_polyline_object_t));
  
  /*
   * Update the data for each of the objects.
   */

  polyline_offset = sizeof(BDRY_spdb_product_t) - sizeof(BDRY_spdb_polyline_t);
  
  for (obj = 0; obj < shape->num_objects; obj++)
  {
    int pt;
    SIO_polyline_object_t *object = &(shape->P[obj]);
    BDRY_spdb_polyline_t *polyline =
      (BDRY_spdb_polyline_t *)((char *)bdry + polyline_offset);
    
    STRcopy(object->object_label, polyline->object_label,
	    SIO_LABEL_LEN);
    object->npoints = polyline->num_pts;
    object->nseconds = polyline->num_secs;

    /*
     * Allocate space for the point arrays
     */

    object->lat = (float *)umalloc(polyline->num_pts * sizeof(float));
    object->lon = (float *)umalloc(polyline->num_pts * sizeof(float));
    object->u_comp = (float *)umalloc(polyline->num_pts * sizeof(float));
    object->v_comp = (float *)umalloc(polyline->num_pts * sizeof(float));
    object->value = (float *)umalloc(polyline->num_pts * sizeof(float));
    
    /*
     * Set the point array values.
     */

    for (pt = 0; pt < polyline->num_pts; pt++)
    {
      object->lat[pt] = polyline->points[pt].lat;
      object->lon[pt] = polyline->points[pt].lon;
      object->u_comp[pt] = polyline->points[pt].u_comp;
      object->v_comp[pt] = polyline->points[pt].v_comp;
      object->value[pt] = polyline->points[pt].value;
      
    } /* endfor - pt */
    
    polyline_offset += sizeof(BDRY_spdb_polyline_t) +
      (polyline->num_pts - 1) * sizeof(BDRY_spdb_point_t);
    
  } /* endfor - obj */
  
  return(shape);
}


/************************************************************************
 * BDRY_spdb_to_pjg_direction(): Converts the SPDB direction value to the
 *                               value used by PJG routines.
 *
 * Colide stores the direction using cartesian coordinates where
 * 0 degrees is along the X axis and degrees increase counter-
 * clockwise.  PJG uses map coordinates, where 0 degrees is
 * north and degrees increase clockwise.  Also, colide gives the
 * angle we are moving FROM rather than the angle we are moving
 * TO.
 *
 * Wrong again (maybe).
 * Colide (hopefully) gives the angle TO, so we revert to the
 * standard 90 - angle to convert from math (colide) to "map".
 */

double BDRY_spdb_to_pjg_direction(double spdb_direction)
{
  double pjg_direction;
  
/*  pjg_direction = -90.0 - spdb_direction; */
/*  pjg_direction = spdb_direction;*/
  pjg_direction = 90.0 - spdb_direction;
  while (pjg_direction < 0.0)
    pjg_direction += 360.0;
  while (pjg_direction >= 360.0)
    pjg_direction -= 360.0;
    
  return(pjg_direction);
}


/************************************************************************
 * BDRY_spdb_to_product(): Converts a BDRY_spdb_product_t buffer to a
 *                         BDRY_product_t structure.
 *
 * Note that this routines allocates space for the BDRY_product_t
 * structure which has to be freed by the calling routine using
 * BDRY_free_product().
 */

BDRY_product_t *BDRY_spdb_to_product(BDRY_spdb_product_t *buffer)
{
  BDRY_product_t *product;
  int i, j;
  BDRY_spdb_polyline_t *spdb_polyline;
  BDRY_spdb_point_t *spdb_point;
  
  /*
   * Allocate space for the product information and copy it from
   * the buffer.
   */

  product = (BDRY_product_t *)umalloc(sizeof(BDRY_product_t));
  
  product->type          = buffer->type;
  product->subtype       = buffer->subtype;
  product->sequence_num  = buffer->sequence_num;
  product->group_id      = buffer->group_id;
  product->generate_time = buffer->generate_time;
  product->data_time     = buffer->data_time;
  product->forecast_time = buffer->forecast_time;
  product->expire_time   = buffer->expire_time;
  product->line_type     = buffer->line_type;
  product->bdry_id       = buffer->bdry_id;
  product->num_polylines = buffer->num_polylines;
  
  product->motion_direction    = buffer->motion_direction;
  product->motion_speed        = buffer->motion_speed;
  product->line_quality_value  = buffer->line_quality_value;
  product->line_quality_thresh = buffer->line_quality_thresh;
  
  product->type_string      = STRdup(buffer->type_string);
  product->subtype_string   = STRdup(buffer->subtype_string);
  product->line_type_string = STRdup(buffer->line_type_string);
  product->desc             = STRdup(buffer->desc);
  
  /*
   * Now allocate space for each of the polylines and copy the
   * information from the buffer.
   */

  product->polylines = (BDRY_polyline_t *)umalloc(product->num_polylines *
						  sizeof(BDRY_polyline_t));
  
  spdb_polyline =
    (BDRY_spdb_polyline_t *)((char *)buffer +
			     sizeof(BDRY_spdb_product_t) -
			     sizeof(BDRY_spdb_polyline_t));
  
  for (i = 0; i < product->num_polylines; i++)
  {
    BDRY_polyline_t *polyline = &product->polylines[i];
    
    polyline->num_pts  = spdb_polyline->num_pts;
    polyline->num_secs = spdb_polyline->num_secs;
    
    polyline->object_label = STRdup(spdb_polyline->object_label);
    
    polyline->points = (BDRY_point_t *)umalloc(polyline->num_pts *
					       sizeof(BDRY_point_t));
    
    spdb_point =
      (BDRY_spdb_point_t *)((char *)spdb_polyline +
			    sizeof(BDRY_spdb_polyline_t) -
			    sizeof(BDRY_spdb_point_t));
    
    for (j = 0; j < polyline->num_pts; j++)
    {
      BDRY_point_t *point = &polyline->points[j];
      
      point->lat    = spdb_point->lat;
      point->lon    = spdb_point->lon;
      point->u_comp = spdb_point->u_comp;
      point->v_comp = spdb_point->v_comp;
      point->value  = spdb_point->value;
      
      spdb_point++;
      
    } /* endfor - j */
    
    spdb_polyline = (BDRY_spdb_polyline_t *)(spdb_point - 1);
			       
  } /* endfor - i */
  
  return(product);
}


/************************************************************************
 * BDRY_subtype_string_to_subtype(): Returns the integer subtype for a
 *                                   given subtype string.
 */

int BDRY_subtype_string_to_subtype(const char *subtype_string)
{
  return(convert_parse_value(subtype_string,
			     Subtype_table,
			     Subtype_table_size));
}


/************************************************************************
 * BDRY_type_string_to_type(): Returns the integer type for a given type
 *                             string.
 */

int BDRY_type_string_to_type(const char *type_string)
{
  int type;
  
  type = convert_parse_value(type_string,
			     Type_table,
			     Type_table_size);
  
  if (type == -1)
    type = get_extrapolated_prod_type(type_string);
  
  return(type);
}

/************************************************************************
 * BDRY_set_data_type():  Sets data type so that first 8 bits (from lowest
 *                        order bit to highest order bit) are the 
 *                        boundary type, the next 8 bits are the subtype 
 *                        and the last 8 bits are the forecast period.
 *                        A constant value is subtracted from the subtype
 *                        to ensure the value will fit in the 8 bits given
 *                        to it.
 */

si32 BDRY_set_data_type(si32 type, si32 subtype, si32 forecast_period)
{
   int word = 0;
   si32 data_type = 0;
   
   word = (forecast_period << 16);
   data_type = ((data_type) | word);
   
   word = ((subtype - SUBTYPE_BIAS) << 8);
   data_type = ((data_type) | word);
   
   word = type;
   data_type = ((data_type) | word);

   return(data_type);
}

/************************************************************************
 * BDRY_parse_data_type(): Parses the data type field into type, subtype
 *                         and forecast period, assuming that the data type
 *                         was created such that the first 8 bits (from 
 *                         lowest order bit to highest order bit) are the
 *                         boundary type, the next 8 bits are the subtype
 *                         and the last 8 bits are the forecast period.
 *                         A constant value is added back to the subtype
 *                         since it should have been subtracted off in
 *                         the creation of the data type to ensure that the
 *                         subtype value would fit into the 8 bits assigned
 *                         to it.
 */

void BDRY_parse_data_type(si32 data_type, si32 *type, si32 *subtype, 
                          si32 *forecast_period)
{
   *type = (MASK1 & data_type);
   *subtype = ((MASK2 & data_type) >> 8) + SUBTYPE_BIAS;
   *forecast_period = (MASK3 & data_type) >> 16;
}

/************************************************************************
 * STATIC ROUTINES
 ************************************************************************/

/************************************************************************
 * convert_parse_value(): Convert a string value read in from the input
 *                        file into the corresponding integer value using
 *                        the given parse table.
 */

static int convert_parse_value(const char *string,
			       parse_table_t *parse_table,
			       int parse_table_size)
{
  int i;
  
  for (i = 0; i < parse_table_size; i++)
    if (STRequal_exact(string, parse_table[i].parse_string))
      return(parse_table[i].parse_value);
    
  return(BDRY_VALUE_UNKNOWN);
}


/************************************************************************
 * get_extrapolated_prod_type(): Parse the product type line to see if
 *                               it's an extrapolated product.
 */

static int get_extrapolated_prod_type(const char *type_string)
{
  int prod_type;

  switch(type_string[0])
  {
  case 'E' :
    prod_type = BDRY_TYPE_EXTRAP_ISSUE_MIGFA;
    break;
    
  case 'V' :
    prod_type = BDRY_TYPE_EXTRAP_VALID;
    break;
    
  case 'X' :
    prod_type = BDRY_TYPE_EXTRAP_ISS_COLIDE;
    break;
    
  default :
    return(BDRY_VALUE_UNKNOWN);
  } /* endswitch */
  
  return(prod_type);
}


/************************************************************************
 * point_from_BE(): Convert a boundary point structure from big-endian
 *                  format to native format.
 */

static void point_from_BE(BDRY_spdb_point_t *point)
{
  /*
   * This structure contains only fl32 values, so convert as an
   * array.
   */

  BE_to_array_32(point, sizeof(BDRY_spdb_point_t));
  
  return;
}


/************************************************************************
 * point_to_BE(): Convert a boundary point structure from native format
 *                to big-endian format.
 */

static void point_to_BE(BDRY_spdb_point_t *point)
{
  /*
   * This structure contains only fl32 values, so convert as an
   * array.
   */

  BE_from_array_32(point, sizeof(BDRY_spdb_point_t));
  
  return;
}


/************************************************************************
 * polyline_from_BE(): Convert a boundary polyline structure from big-endian
 *                     format to native format.
 */

static void polyline_from_BE(BDRY_spdb_polyline_t *polyline)
{
  int pt;
  
  polyline->num_pts       = BE_to_si32(polyline->num_pts);
  polyline->num_secs      = BE_to_si32(polyline->num_secs);
  BE_to_array_32(polyline->spare, sizeof(polyline->spare));
  /* object_label is okay */

  for (pt = 0; pt < polyline->num_pts; pt++)
    point_from_BE(&(polyline->points[pt]));
  
  return;
}


/************************************************************************
 * polyline_to_BE(): Convert a boundary polyline structure from native
 *                   format to big-endian format.
 */

static void polyline_to_BE(BDRY_spdb_polyline_t *polyline)
{
  int pt;
  int num_pts = polyline->num_pts;
  
  polyline->num_pts       = BE_from_si32(polyline->num_pts);
  polyline->num_secs      = BE_from_si32(polyline->num_secs);
  BE_from_array_32(polyline->spare, sizeof(polyline->spare));
  /* object_label is okay */

  for (pt = 0; pt < num_pts; pt++)
    point_to_BE(&(polyline->points[pt]));
  
  return;
}


/************************************************************************
 * print_point(): Print a boundary point structure in ASCII format
 *                to the given stream.
 */

static void print_point(FILE *stream, BDRY_spdb_point_t *point)
{
  int i;
  
  fprintf(stream, "\nBoundary Point:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   lat = %f\n", point->lat);
  fprintf(stream, "   lon = %f\n", point->lon);
  fprintf(stream, "   u_comp = %f\n", point->u_comp);
  fprintf(stream, "   v_comp = %f\n", point->v_comp);
  fprintf(stream, "   value = %f\n", point->value);
  for (i = 0; i < BDRY_POINT_SPARE_LEN; i++)
    fprintf(stream, "   spare[%d] = %f\n", i, point->spare[i]);
  
  return;
}


/************************************************************************
 * print_polyline(): Print a boundary polyline structure in ASCII format
 *                   to the given stream.
 */

static void print_polyline(FILE *stream, BDRY_spdb_polyline_t *polyline,
			   int print_points)
{
  int i;
  int pt;
  
  fprintf(stream, "\nBoundary Polyline:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   num_pts = %d\n", polyline->num_pts);
  fprintf(stream, "   num_secs = %d\n", polyline->num_secs);
  for (i = 0; i < BDRY_POLYLINE_SPARE_LEN; i++)
    fprintf(stream, "   spare[%d] = %d\n", i, polyline->spare[i]);
  fprintf(stream, "   object_label = <%s>\n", polyline->object_label);

  if (print_points)
    for (pt = 0; pt < polyline->num_pts; pt++)
      print_point(stream, &(polyline->points[pt]));
  
  return;
}
