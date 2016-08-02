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
/************************************************************************
 *                    
 * bdry.h - Header file for the boundary module of the DIDSS library.
 *
 * The boundary module contains routines for manipulating boundaries
 * in the format used to store the boundaries in SPDB.
 *
 * If you are writing boundary data to an SPDB database, the general
 * idea is that you either read a boundary from an ASCII file using
 * SIO_read_data() or SIO_read_record() or you create your own
 * SIO_shape_data structure to be put into SPDB.  You then convert
 * the SIO_shape_data structure to a BDRY_spdb_product_t structure using
 * BDRY_spdb_product_from_SIO_shape(), swap the data to big-endian format
 * using BDRY_spdb_product_to_BE() and then use SPDB routines to put the
 * data into a database.
 *
 * If you are reading boundary data, you reverse the process: you
 * retrieve the data using SPDB routines, swap to native format using
 * BDRY_spdb_product_from_BE() and convert to the internal shape format
 * using BDRY_spdb_product_to_SIO_shape().
 *
 * Nancy Rehak
 * May 1997
 *                   
 *************************************************************************/

#include <dataport/port_types.h>
#include <rapformats/ascii_shapeio.h>
#include <rapformats/bdry_typedefs.h>


#ifdef __cplusplus
 extern "C" {
#endif

#ifndef BDRY_H
#define BDRY_H


/*
 * The following structures are used to conveniently represent boundaries
 * in memory.
 */

typedef struct
{
  fl32 lat;                   /* latitude in degrees */
  fl32 lon;                   /* longitude in degrees */
  fl32 u_comp;                /* horizontal motion dir vector component */
                              /*   in m/s */
  fl32 v_comp;                /* vertical motion dir vector component */
                              /*   in m/s */
  fl32 value;                 /* bdry rel steering flow value for the */
                              /*   initiation zone */
} BDRY_point_t;


typedef struct
{
  si32 num_pts;               /* number of points */
  si32 num_secs;              /* number of seconds extrapolation it is */
  const char *object_label;         /* label associated with this polyline */
  BDRY_point_t *points;       /* array of points in the polyline - occurs */
                              /*   num_pts times */
} BDRY_polyline_t;


typedef struct
{
  si32 type;                  /* product type value as defined above */
  si32 subtype;               /* product subtype value as defined above */
  si32 sequence_num;          /* product counter */
  si32 group_id;              /* group id number */
  si32 generate_time;         /* time of product generation */
  si32 data_time;             /* time of data used to create */
  si32 forecast_time;         /* time of forecast (extrapolation) */
  si32 expire_time;           /* time product becomes invalid */
  si32 line_type;             /* line type value as defined above (for */
                              /*   COLIDE bdrys, extraps) */
  si32 bdry_id;               /* boundary id number */
  si32 num_polylines;         /* number of polylines in this product */
  fl32 motion_direction;      /* motion direction in degrees (all objects */
                              /*   move together).  This value is given in */
                              /*   math coordinates (0 degrees along X axis, */
                              /*   increases in counterclockwise direction). */
  fl32 motion_speed;          /* motion speed in m/s (all objects move */
                              /*   together) */
  fl32 line_quality_value;    /* quality (confidence) value (for COLIDE) */
  fl32 line_quality_thresh;   /* quality threshold (for COLIDE) */
  const char *type_string;    /* product type in string format - used to */
                              /*   determine the type value above */
  const char *subtype_string; /* product subtype in string format - used */
                              /*   to determine the subtype above */
  const char *line_type_string; /* line type (for COLIDE bdrys, extraps) - */
                              /*   used to determine the line_type above */
  const char *desc;           /* label associated with the product */
  BDRY_polyline_t *polylines; /* array of structures for all of the */
                              /*   polylines making up this product - */
                              /*   num_polylines of these */
} BDRY_product_t;

   
  
/************************************************************************
 * Function prototypes.
 ************************************************************************/

/************************************************************************
 * BDRY_free_product(): Frees the space used by a BDRY_product_t
 *                      structure.  Assumes that pointers in the structure
 *                      are set to NULL if they don't point to anything.
 */

void BDRY_free_product(BDRY_product_t *product);

/************************************************************************
 * BDRY_line_type_string_to_line_type(): Returns the integer line type
 *                                       for a given line type string.
 */

int BDRY_line_type_string_to_line_type(const char *line_type_string);

/************************************************************************
 * BDRY_print_spdb_product(): Print an SPDB boundary in ASCII format to
 *                            the given stream.
 */

void BDRY_print_spdb_product(FILE *stream, BDRY_spdb_product_t *prod,
			     int print_points);

/************************************************************************
 * BDRY_product_to_spdb(): Converts a BDRY_product_t structure to a
 *                         BDRY_spdb_product_t buffer.  Returns the length
 *                         of the SPDB buffer in the argument buffer_len.
 *
 * Note that this routine returns a pointer to static memory that
 * should NOT be freed by the calling routine.
 */

BDRY_spdb_product_t *BDRY_product_to_spdb(BDRY_product_t *product,
					  int *buffer_len);

/************************************************************************
 * BDRY_spdb_product_from_BE(): Convert a boundary product from big-endian
 *                         format to native format.
 *
 * Returns the number of bytes in the swapped product.  This can be
 * used to easily find the next product in a buffer containing several
 * products.
 */

int BDRY_spdb_product_from_BE(BDRY_spdb_product_t *prod);

/************************************************************************
 * BDRY_spdb_product_from_SIO_shape(): Converts SIO shape information read in
 *                                from an ASCII file into a boundary
 *                                product buffer.
 *
 * Returns a pointer to a static buffer which should NOT be freed by the
 * calling routine.  Returns NULL if there is an error.
 * Also returns the size in bytes of the product buffer returned.
 *
 * NOTE: This method needs to be tested because we're not currently sure
 * whether shapeio speed values are stored in km/hr or in m/s.
 */

BDRY_spdb_product_t *BDRY_spdb_product_from_SIO_shape(SIO_shape_data_t *shape,
						      int *prod_size);   /* returned */

/************************************************************************
 * BDRY_spdb_product_to_BE(): Convert a boundary product from native format
 *                            to big-endian format. 
 */

void BDRY_spdb_product_to_BE(BDRY_spdb_product_t *prod);

/************************************************************************
 * BDRY_spdb_product_to_SIO_shape(): Converts a boundary product buffer
 *                                   (probably just retrieved from an SPDB
 *                                   database) into the internal SIO shape
 *                                   structure.  Returns a pointer to memory
 *                                   allocated by this routine that should be
 *                                   freed by the calling routine.  Returns
 *                                   NULL if there is an error.
 */

SIO_shape_data_t *BDRY_spdb_product_to_SIO_shape(BDRY_spdb_product_t *bdry);

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
 */

double BDRY_spdb_to_pjg_direction(double spdb_direction);

/************************************************************************
 * BDRY_spdb_to_product(): Converts a BDRY_spdb_product_t buffer to a
 *                         BDRY_product_t structure.
 *
 * Note that this routines allocates space for the BDRY_product_t
 * structure which has to be freed by the calling routine using
 * BDRY_free_product().
 */

BDRY_product_t *BDRY_spdb_to_product(BDRY_spdb_product_t *buffer);

/************************************************************************
 * BDRY_subtype_string_to_subtype(): Returns the integer subtype for a
 *                                   given subtype string.
 */

int BDRY_subtype_string_to_subtype(const char *subtype_string);

/************************************************************************
 * BDRY_type_string_to_type(): Returns the integer type for a given type
 *                             string.
 */

int BDRY_type_string_to_type(const char *type_string);

/************************************************************************
 * BDRY_set_data_type():  Sets data type so that first 8 bits (from lowest
 *                        order bit to highest order bit) are the 
 *                        boundary type, the next 8 bits are the subtype 
 *                        and the last 8 bits are the forecast period.
 *                        A constant value is subtracted from the subtype
 *                        to ensure the value will fit in the 8 bits given
 *                        to it.
 */

si32 BDRY_set_data_type(si32 type, si32 subtype, si32 forecast_period);

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
                          si32 *forecast_period);



# endif     /* BDRY_H */

#ifdef __cplusplus
}
#endif
