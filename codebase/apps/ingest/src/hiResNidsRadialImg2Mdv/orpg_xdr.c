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

    Product serialization (xdr) routines.
	
******************************************************************/

/* 
 * RCS info
 * $Author: dixon $
 * $Locker:  $
 * $Date: 2016/03/07 01:23:08 $
 * $Id: orpg_xdr.c,v 1.2 2016/03/07 01:23:08 dixon Exp $
 * $Revision: 1.2 $
 * $State: Exp $
 */  

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <rpc/xdr.h>

#include "orpg_product.h"

#define MAX_ARRAY_SIZE 0x7fffffff	/* there is no limit on array size in C
					*/

static int xdr_RPGP_product_t (XDR *xdrs, RPGP_product_t *prod);
static int xdr_RPGP_parameter_t (XDR *xdrs, RPGP_parameter_t *param);
static int xdr_RPGP_location_t (XDR *xdrs, RPGP_location_t *loc);
static int xdr_RPGP_area_t (XDR *xdrs, RPGP_area_t *area);
static int xdr_RPGP_event_t (XDR *xdrs, RPGP_event_t *event);
static int xdr_RPGP_component_t (XDR *xdrs, char **comp);
static int xdr_RPGP_text_t (XDR *xdrs, RPGP_text_t *text);
static int xdr_RPGP_grid_t (XDR *xdrs, RPGP_grid_t *text);
static int xdr_RPGP_radial_t (XDR *xdrs, RPGP_radial_t *text);
static int xdr_RPGP_data_t (XDR *xdrs, RPGP_data_t *data, int size);
static char *Get_token (char *text, char *buf, int buf_size);
static int Get_data_type (char *attrs, char *buf, int buf_size);
static int xdr_RPGP_string_t (XDR *xdrs, RPGP_string_t *str);
static int xdr_RPGP_table_t (XDR *xdrs, RPGP_table_t *table);
static int Get_n_grid_points (RPGP_grid_t *grid);
static int xdr_RPGP_radial_data_t (XDR *xdrs, RPGP_radial_data_t *radial_d);


/******************************************************************

    Serializes product "prod" and returns the pointer to the serialized
    data with "serial_data". It returns the number of bytes of the 
    serialized data on success or -1 on failure. The caller must
    free the returned pointer if it is not NULL.

******************************************************************/

int RPGP_product_serialize (void *prod, char **serial_data) {
    int size, ret;
    XDR xdrs;
    char *buf;

    *serial_data = NULL;
    size = xdr_sizeof ((xdrproc_t)xdr_RPGP_product_t, prod);
    if (size <= 0)
	return (-1);
    buf = (char *)malloc (size);
    if (buf == NULL)
	return (-1);

    xdrmem_create (&xdrs, buf, size, XDR_ENCODE);
    ret = xdr_RPGP_product_t (&xdrs, ( RPGP_product_t *)prod);
    xdr_destroy (&xdrs);
    if (ret == 0) {
	free (buf);
	return (-1);
    }
    *serial_data = buf;
    return (size);
}

/******************************************************************

    Deserializes the serialized product data in "serial_data" of "size"
    bytes and returns the pointer to the deserialized product struct
    with "prod". It returns 0 on success or -1 on failure. The caller 
    must free the returned product struct, if it is not NULL, with 
    RPGP_product_free.

******************************************************************/

int RPGP_product_deserialize (char *serial_data, int size, void **prod) {
    XDR xdrs;
    RPGP_product_t *p;
    int ret;

    *prod = NULL;
    p = (RPGP_product_t*)  (malloc (sizeof (RPGP_product_t)));
    if (p == NULL)
	return (-1);
    xdrmem_create (&xdrs, serial_data, size, XDR_DECODE);
    ret = xdr_RPGP_product_t (&xdrs, p);
    xdr_destroy (&xdrs);
    if (ret == 0) {
	free (p);
	return (-1);
    }
    *prod = p;
    return (0);
}

/******************************************************************

    Frees all pointers of the product struct "prod". It returns 0 
    on success or -1 on failure. This function can be applied to
    product structs created by RPGP_product_deserialize or any 
    struct with all its pointers memory-allocated.

******************************************************************/

int RPGP_product_free (void *prod) {
    XDR xdrs;
    int ret;

    xdrmem_create (&xdrs, NULL, 0, XDR_FREE);
    ret = xdr_RPGP_product_t (&xdrs, (RPGP_product_t*)prod);
    xdr_destroy (&xdrs);
    free (prod);
    if (ret == 0)
	return (-1);
    return (0);
}

/******************************************************************

    RPGP_product_t XDR-serialization function. A separate function for 
    deserialize the part excluding the compressed area may be useful. 

******************************************************************/

int xdr_RPGP_product_t (XDR *xdrs, RPGP_product_t *prod) {
    int *numof_prod_params_p;
    RPGP_parameter_t **prod_params_p;
    int *numof_components_p;
    void ***components_p;

    if (xdrs->x_op == XDR_DECODE) {
	prod->name = NULL;
	prod->description = NULL;
    }

    if (xdr_string (xdrs, &(prod->name), MAX_ARRAY_SIZE) == 0 ||
	xdr_string (xdrs, &(prod->description), MAX_ARRAY_SIZE) == 0 ||
	xdr_int (xdrs, &(prod->product_id)) == 0 ||
	xdr_int (xdrs, &(prod->type)) == 0 ||
	xdr_u_int (xdrs, &(prod->gen_time)) == 0)
	return (0);

    if (prod->type == RPGP_EXTERNAL) {
	RPGP_ext_data_t *eprod = (RPGP_ext_data_t *)prod;
	if (xdrs->x_op == XDR_DECODE) {
	    eprod->prod_params = NULL;
	    eprod->components = NULL;
	}

	if (xdr_short (xdrs, &(eprod->spare[0])) == 0 ||
	    xdr_short (xdrs, &(eprod->spare[1])) == 0 ||
	    xdr_short (xdrs, &(eprod->spare[2])) == 0 ||
	    xdr_short (xdrs, &(eprod->spare[3])) == 0 ||
	    xdr_short (xdrs, &(eprod->spare[4])) == 0 ||
	    xdr_short (xdrs, &(eprod->compress_type)) == 0 ||
	    xdr_int (xdrs, &(eprod->size_decompressed)) == 0)
	    return (0);
	numof_prod_params_p = &(eprod->numof_prod_params);
	prod_params_p = &(eprod->prod_params);
	numof_components_p = &(eprod->numof_components);
	components_p = &(eprod->components);
    }
    else {
	if (xdrs->x_op == XDR_DECODE) {
	    prod->radar_name = NULL;
	    prod->prod_params = NULL;
	    prod->components = NULL;
	}

	if (xdr_string (xdrs, &(prod->radar_name), MAX_ARRAY_SIZE) == 0 ||
	    xdr_float (xdrs, &(prod->radar_lat)) == 0 ||
	    xdr_float (xdrs, &(prod->radar_lon)) == 0 ||
	    xdr_float (xdrs, &(prod->radar_height)) == 0 ||
	    xdr_u_int (xdrs, &(prod->volume_time)) == 0 ||
	    xdr_u_int (xdrs, &(prod->elevation_time)) == 0 ||
	    xdr_float (xdrs, &(prod->elevation_angle)) == 0 ||
	    xdr_int (xdrs, &(prod->volume_number)) == 0 ||
	    xdr_short (xdrs, &(prod->operation_mode)) == 0 ||
	    xdr_short (xdrs, &(prod->vcp)) == 0 ||
	    xdr_short (xdrs, &(prod->elevation_number)) == 0 ||
	    xdr_short (xdrs, &(prod->compress_type)) == 0 ||
	    xdr_int (xdrs, &(prod->size_decompressed)) == 0)
	    return (0);
	numof_prod_params_p = &(prod->numof_prod_params);
	prod_params_p = &(prod->prod_params);
	numof_components_p = &(prod->numof_components);
	components_p = &(prod->components);
    }

    if (xdr_int (xdrs, numof_prod_params_p) == 0 ||
	(*numof_prod_params_p > 0 &&
	 xdr_array (xdrs, (char **)prod_params_p, 
		(unsigned int *)numof_prod_params_p, MAX_ARRAY_SIZE, 
		sizeof (RPGP_parameter_t), 
		(xdrproc_t)xdr_RPGP_parameter_t) == 0))
	return (0);

    if (xdr_int (xdrs, numof_components_p) == 0)
	return (0);

    /* process decompression */

    if (*numof_components_p > 0 &&
	xdr_array (xdrs, (char **)components_p, 
		(unsigned int *)numof_components_p, MAX_ARRAY_SIZE, 
		sizeof (char *), (xdrproc_t)xdr_RPGP_component_t) == 0)
	return (0);

    return (1);
}

/******************************************************************

    Generic component pointer serialization function. 

******************************************************************/

static int xdr_RPGP_component_t (XDR *xdrs, char **comp) {
    int type, p, ret;

    if (xdrs->x_op == XDR_DECODE) {		/* get type */
	p = xdr_getpos (xdrs);
	if (xdr_int (xdrs, &type) == 0 ||	/* XDR pointer mark */
	    xdr_int (xdrs, &type) == 0)
	    return (0);
	if (xdr_setpos (xdrs, p) == 0)
	    return (0);
	*comp = NULL;
    }
    else
	type = ((RPGP_radial_t *)(*comp))->comp_type;

    switch (type) {
	case RPGP_AREA_COMP:
	    ret = xdr_pointer (xdrs, (char **)comp, 
				sizeof (RPGP_area_t), 
				(xdrproc_t)xdr_RPGP_area_t);
	    break;
	case RPGP_TEXT_COMP:
	    ret = xdr_pointer (xdrs, (char **)comp, 
				sizeof (RPGP_text_t), 
				(xdrproc_t)xdr_RPGP_text_t);
	    break;
	case RPGP_EVENT_COMP:
	    ret = xdr_pointer (xdrs, (char **)comp, 
				sizeof (RPGP_event_t), 
				(xdrproc_t)xdr_RPGP_event_t);
	    break;
	case RPGP_GRID_COMP:
	    ret = xdr_pointer (xdrs, (char **)comp, 
				sizeof (RPGP_grid_t), 
				(xdrproc_t)xdr_RPGP_grid_t);
	    break;
	case RPGP_RADIAL_COMP:
	    ret = xdr_pointer (xdrs, (char **)comp, 
				sizeof (RPGP_radial_t), 
				(xdrproc_t)xdr_RPGP_radial_t);
	    break;
	case RPGP_TABLE_COMP:
	    ret = xdr_pointer (xdrs, (char **)comp, 
				sizeof (RPGP_table_t), 
				(xdrproc_t)xdr_RPGP_table_t);
	    break;
	default:
	    ret = 0;
	    break;
    }
    if (ret == 0)
	return (ret);
    return (1);
}

/******************************************************************

    RPGP_parameter_t serialization function. 

******************************************************************/

static int xdr_RPGP_parameter_t (XDR *xdrs, RPGP_parameter_t *param) {

    if (xdrs->x_op == XDR_DECODE) {
	param->id = NULL;
	param->attrs = NULL;
    }
    if (xdr_string (xdrs, &(param->id), MAX_ARRAY_SIZE) == 0 ||
	xdr_string (xdrs, &(param->attrs), MAX_ARRAY_SIZE) == 0)
	return (0);
    return (1);
}

/******************************************************************

    RPGP_data_t serialization function. 

******************************************************************/

static int xdr_RPGP_data_t (XDR *xdrs, RPGP_data_t *data, int size) {
    char type[32];
    int ret;
    size_t s;
    xdrproc_t f;

    if (xdrs->x_op == XDR_DECODE) {
	data->attrs = NULL;
	data->data = NULL;
    }
    else {
	if (Get_data_type (data->attrs, type, 32) <= 0) {
	    fprintf (stderr, "type not defined in RPGP_data_t\n");
	    return (0);
	}
    }

    if (xdr_string (xdrs, &(data->attrs), MAX_ARRAY_SIZE) == 0)
	return (0);

    if (xdrs->x_op == XDR_DECODE) {
	if (Get_data_type (data->attrs, type, 32) <= 0) {
	    fprintf (stderr, "type not defined in RPGP_data_t\n");
	    return (0);
	}
    }

    if (size <= 0)
	return (1);

    ret = 0;
    s = 0;
    f = (xdrproc_t)0;
    if (strcmp (type, "short") == 0) {
	s = sizeof (short);
	f = (xdrproc_t)xdr_short;
    }
    else if (strcmp (type, "byte") == 0) {
	s = sizeof (char);
	f = (xdrproc_t)xdr_char;
    }
    else if (strcmp (type, "int") == 0) {
	s = sizeof (int);
	f = (xdrproc_t)xdr_int;
    }
    else if (strcmp (type, "float") == 0) {
	s = sizeof (float);
	f = (xdrproc_t)xdr_float;
    }
    else if (strcmp (type, "double") == 0) {
	s = sizeof (double);
	f = (xdrproc_t)xdr_double;
    }
    else if (strcmp (type, "uint") == 0) {
	s = sizeof (unsigned int);
	f = (xdrproc_t)xdr_u_int;
    }
    else if (strcmp (type, "ushort") == 0) {
	s = sizeof (unsigned short);
	f = (xdrproc_t)xdr_u_short;
    }
    else if (strcmp (type, "ubyte") == 0) {
	s = sizeof (unsigned char);
	f = (xdrproc_t)xdr_u_char;
    }
    else
	fprintf (stderr, "orpg_xdr: type (%s) not supported\n", type);

    if (s > 0)
	ret = xdr_array (xdrs, (char **)&(data->data), 
		(unsigned int *)&(size), MAX_ARRAY_SIZE, s, f);
    if (ret == 0)
	return (0);

    return (1);
}

/******************************************************************

    RPGP_location_t serialization function. This is shared by 
    RPGP_xy_location_t and RPGP_azran_location_t.

******************************************************************/

static int xdr_RPGP_location_t (XDR *xdrs, RPGP_location_t *loc) {

    if (xdr_float (xdrs, &(loc->lat)) == 0 ||
	xdr_float (xdrs, &(loc->lon)) == 0)
	return (0);
    return (1);
}

/******************************************************************

    RPGP_area_t serialization function. 

******************************************************************/

static int xdr_RPGP_area_t (XDR *xdrs, RPGP_area_t *area) {

    if (xdrs->x_op == XDR_DECODE) {
	area->comp_params = NULL;
	area->points = NULL;
    }
    if (xdr_int (xdrs, &(area->comp_type)) == 0 ||
	xdr_int (xdrs, &(area->numof_comp_params)) == 0 ||
	(area->numof_comp_params > 0 &&
	 xdr_array (xdrs, (char **)&(area->comp_params), 
		(unsigned int *)&(area->numof_comp_params), MAX_ARRAY_SIZE, 
		sizeof (RPGP_parameter_t), 
		(xdrproc_t)xdr_RPGP_parameter_t) == 0) ||
	xdr_int (xdrs, &(area->area_type)) == 0 ||
	xdr_int (xdrs, &(area->numof_points)) == 0)
	return (0);

    if (area->numof_points > 0) {
	int ret;

	switch (RPGP_LOCATION_TYPE (area->area_type)) {
	    case RPGP_LATLON_LOCATION:
	    case RPGP_XY_LOCATION:
	    case RPGP_AZRAN_LOCATION:
	 	ret = xdr_array (xdrs, (char **)&(area->points), 
			(unsigned int *)&(area->numof_points), MAX_ARRAY_SIZE, 
			sizeof (RPGP_location_t), 
			(xdrproc_t)xdr_RPGP_location_t);
		break;
	    default:
		fprintf (stderr, "Unexpected Location type (%x)\n", 
				RPGP_LOCATION_TYPE (area->area_type));
		return (0);
	}
	if (ret == 0)
	    return (0);
    }
    return (1);
}

/******************************************************************

    RPGP_text_t serialization function. 

******************************************************************/

static int xdr_RPGP_text_t (XDR *xdrs, RPGP_text_t *text) {
    int len;

    if (xdrs->x_op == XDR_DECODE) {
	text->comp_params = NULL;
	text->text = NULL;
    }
    else
	len = strlen (text->text) + 1;
    if (xdr_int (xdrs, &(text->comp_type)) == 0 ||
	xdr_int (xdrs, &(text->numof_comp_params)) == 0 ||
	(text->numof_comp_params > 0 &&
	 xdr_array (xdrs, (char **)&(text->comp_params), 
		(unsigned int *)&(text->numof_comp_params), MAX_ARRAY_SIZE, 
		sizeof (RPGP_parameter_t), 
		(xdrproc_t)xdr_RPGP_parameter_t) == 0) ||
	xdr_bytes (xdrs, (char **)&(text->text), (unsigned int *)&len, 
					MAX_ARRAY_SIZE) == 0)
	return (0);
    return (1);
}

/******************************************************************

    RPGP_table_t serialization function. 

******************************************************************/

static int xdr_RPGP_table_t (XDR *xdrs, RPGP_table_t *table) {
    int len;

    if (xdrs->x_op == XDR_DECODE) {
	table->comp_params = NULL;
	table->column_labels = NULL;
	table->row_labels = NULL;
	table->entries = NULL;
    }

    if (xdr_int (xdrs, &(table->comp_type)) == 0 ||
	xdr_int (xdrs, &(table->numof_comp_params)) == 0 ||
	(table->numof_comp_params > 0 &&
	 xdr_array (xdrs, (char **)&(table->comp_params), 
		(unsigned int *)&(table->numof_comp_params), MAX_ARRAY_SIZE, 
		sizeof (RPGP_parameter_t), 
		(xdrproc_t)xdr_RPGP_parameter_t) == 0) ||
	xdr_RPGP_string_t (xdrs, &(table->title)) == 0 ||
	xdr_short (xdrs, &(table->n_columns)) == 0 ||
	xdr_short (xdrs, &(table->n_rows)) == 0)
	return (0);

    len = table->n_columns;
    if (len > 0 &&
	xdr_array (xdrs, (char **)&(table->column_labels), 
		(unsigned int *)&len, MAX_ARRAY_SIZE, 
		sizeof (RPGP_string_t), 
		(xdrproc_t)xdr_RPGP_string_t) == 0)
	return (0);
    len = table->n_rows;
    if (len > 0 &&
	xdr_array (xdrs, (char **)&(table->row_labels), 
		(unsigned int *)&len, MAX_ARRAY_SIZE, 
		sizeof (RPGP_string_t), 
		(xdrproc_t)xdr_RPGP_string_t) == 0)
	return (0);
    len = table->n_rows * table->n_columns;
    if (len > 0 &&
	xdr_array (xdrs, (char **)&(table->entries), 
		(unsigned int *)&len, MAX_ARRAY_SIZE, 
		sizeof (RPGP_string_t), 
		(xdrproc_t)xdr_RPGP_string_t) == 0)
	return (0);

    return (1);
}

/******************************************************************

    RPGP_string_t serialization function. 

******************************************************************/

static int xdr_RPGP_string_t (XDR *xdrs, RPGP_string_t *str) {
    int len;

    if (xdrs->x_op == XDR_DECODE)
	str->text = NULL;
    else
	len = strlen (str->text) + 1;
    if (xdr_bytes (xdrs, (char **)&(str->text), (unsigned int *)&len, 
					MAX_ARRAY_SIZE) == 0)
	return (0);
    return (1);
}

/******************************************************************

    RPGP_radial_t serialization function. 

******************************************************************/

static int xdr_RPGP_radial_t (XDR *xdrs, RPGP_radial_t *radial) {

    if (xdrs->x_op == XDR_DECODE) {
	radial->description = NULL;
	radial->comp_params = NULL;
	radial->radials = NULL;
    }

    if (xdr_int (xdrs, &(radial->comp_type)) == 0 ||
	xdr_string (xdrs, &(radial->description), MAX_ARRAY_SIZE) == 0 ||
	xdr_float (xdrs, &(radial->bin_size)) == 0 ||
	xdr_float (xdrs, &(radial->first_range)) == 0 ||
	xdr_int (xdrs, &(radial->numof_comp_params)) == 0 ||
	(radial->numof_comp_params > 0 &&
	 xdr_array (xdrs, (char **)&(radial->comp_params), 
		(unsigned int *)&(radial->numof_comp_params), MAX_ARRAY_SIZE, 
		sizeof (RPGP_parameter_t), 
		(xdrproc_t)xdr_RPGP_parameter_t) == 0) ||
	xdr_int (xdrs, &(radial->numof_radials)) == 0)
	return (0);

    if (radial->numof_radials > 0) {
	if (xdr_array (xdrs, (char **)&(radial->radials), 
			(unsigned int *)&(radial->numof_radials), MAX_ARRAY_SIZE, 
			sizeof (RPGP_radial_data_t), 
			(xdrproc_t)xdr_RPGP_radial_data_t) == 0)
	    return (0);
    }

    return (1);
}

/******************************************************************

    RPGP_radial_data_t serialization function. 

******************************************************************/

static int xdr_RPGP_radial_data_t (XDR *xdrs, RPGP_radial_data_t *radial_d) {

    if (xdr_float (xdrs, &(radial_d->azimuth)) == 0 ||
	xdr_float (xdrs, &(radial_d->elevation)) == 0 ||
	xdr_float (xdrs, &(radial_d->width)) == 0 ||
	xdr_int (xdrs, &(radial_d->n_bins)) == 0)
	return (0);

    if (radial_d->n_bins > 0) {
	if (xdr_RPGP_data_t (xdrs, &(radial_d->bins), radial_d->n_bins) == 0)
	    return (0);
    }

    return (1);
}

/******************************************************************

    RPGP_grid_t serialization function. 

******************************************************************/

static int xdr_RPGP_grid_t (XDR *xdrs, RPGP_grid_t *grid) {
    int len;

    len = 0;
    if (xdrs->x_op == XDR_DECODE) {
	grid->comp_params = NULL;
	grid->dimensions = NULL;
    }
    else
	len = Get_n_grid_points (grid);

    if (xdr_int (xdrs, &(grid->comp_type)) == 0 ||
	xdr_int (xdrs, &(grid->n_dimensions)) == 0 ||
	(grid->n_dimensions > 0 &&
	 xdr_array (xdrs, (char **)&(grid->dimensions), 
		(unsigned int *)&(grid->n_dimensions), MAX_ARRAY_SIZE, 
		sizeof (int), 
		(xdrproc_t)xdr_int) == 0) ||
	xdr_int (xdrs, &(grid->grid_type)) == 0 ||
	xdr_int (xdrs, &(grid->numof_comp_params)) == 0 ||
	(grid->numof_comp_params > 0 &&
	 xdr_array (xdrs, (char **)&(grid->comp_params), 
		(unsigned int *)&(grid->numof_comp_params), MAX_ARRAY_SIZE, 
		sizeof (RPGP_parameter_t), 
		(xdrproc_t)xdr_RPGP_parameter_t) == 0))
	return (0);

    if (xdrs->x_op == XDR_DECODE)
	len = Get_n_grid_points (grid);

    if (xdr_RPGP_data_t (xdrs, &(grid->data), len) == 0)
	return (0);
    return (1);
}

/******************************************************************

    Calculate total number of grid points in "grid". 

******************************************************************/

static int Get_n_grid_points (RPGP_grid_t *grid) {
    int len, i;
    len = 1;
    for (i = 0; i < grid->n_dimensions; i++)
	len *= grid->dimensions[i];
    if (grid->n_dimensions == 0)
	len = 0;
    return (len);
}

/******************************************************************

    RPGP_event_t serialization function. 

******************************************************************/

static int xdr_RPGP_event_t (XDR *xdrs, RPGP_event_t *event) {

    if (xdrs->x_op == XDR_DECODE) {
	event->event_params = NULL;
	event->components = NULL;
    }
    if (xdr_int (xdrs, &(event->comp_type)) == 0 ||
	xdr_int (xdrs, &(event->numof_event_params)) == 0 ||
	(event->numof_event_params > 0 &&
	 xdr_array (xdrs, (char **)&(event->event_params), 
		(unsigned int *)&(event->numof_event_params), MAX_ARRAY_SIZE, 
		sizeof (RPGP_parameter_t), 
		(xdrproc_t)xdr_RPGP_parameter_t) == 0) ||
	xdr_int (xdrs, &(event->numof_components)) == 0 ||
	(event->numof_components > 0 &&
	 xdr_array (xdrs, (char **)&(event->components), 
		(unsigned int *)&(event->numof_components), MAX_ARRAY_SIZE, 
		sizeof (char *), (xdrproc_t)xdr_RPGP_component_t)) == 0)
	return (0);

    return (1);
}

/******************************************************************

    Searches for the type value in attribute string "attrs". If the
    type value is found, it is returned in "buf" of size "buf_size" 
    and the function returns the length of the type value. Returns 
    -1 if the type is not found. The type value must be non-empty 
    and single token.  

******************************************************************/

static int Get_data_type (char *attrs, char *buf, int buf_size) {
    char t1[16], t2[16], t3[16], t4[16], *p;
    int new_attr;

    buf[0] = '\0';
    if (attrs == NULL)
	return (-1);
    p = attrs;
    new_attr = 1;
    while (1) {
	p = Get_token (p, t1, 16);
	if (t1[0] == '\0')
	    break;
	if (strcmp (t1, ";") == 0) {
	    new_attr = 1;
	    continue;
	}
	if (!new_attr)
	    continue;
	if (strcasecmp (t1, "type") == 0) {
	    char *pp = Get_token (p, t2, 16);
	    if (strcmp (t2, "=") == 0) {
		int len;
		pp = Get_token (pp, t3, 16);
		pp = Get_token (pp, t4, 16);
		if (t3[0] != '\0' &&
		    (t4[0] == '\0' || strcmp (t4, ";") == 0)) {
		    len = strlen (t3);
		    if (len >= buf_size)
			len = buf_size - 1;
		    strncpy (buf, t3, len);
		    buf[len] = '\0';
		    return (len);
		}
	    }
	}
	new_attr = 0;
    }
    return (-1);
}

/******************************************************************

    Finds the first token of "text" in "buf" of size "buf_size". The
    returned token is always null-terminated and possibly truncated.
    Returns the pointer after the token. A token is a word separated
    by space, tab or line return. "=" and ";" is considered as a
    token even if they are not separated by space. If "text" is an
    empty string, an empty string is returned in "buf" and the return
    value is "text".
	
******************************************************************/

static char *Get_token (char *text, char *buf, int buf_size) {
    char *p, *st, *next;
    int len;

    p = text;
    while (*p == ' ' || *p == '\t' || *p == '\n')
	p++;
    st = p;
    if (*p == '=' || *p == ';')
	len = 1;
    else if (*p == '\0')
	len = 0;
    else {
	while (*p != '\0' && *p != ' ' && *p != '\t' && 
				*p != '\n' && *p != '='  && *p != ';')
	    p++;
	len = p - st;
    }
    next = st + len;
    if (len >= buf_size)
	len = buf_size - 1;
    strncpy (buf, st, len);
    buf[len] = '\0';
    return (next);
}

