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

/**************************************************************************

    Description: This file contains the definitions for the generic ORPG 
	product format represented in C structures.

 **************************************************************************/

/*
 * RCS info
 * $Author: dixon $
 * $Locker:  $
 * $Date: 2019/03/04 00:13:38 $
 * $Id: orpg_product.hh,v 1.1 2019/03/04 00:13:38 dixon Exp $
 * $Revision: 1.1 $
 * $State: Exp $
 */

#ifndef ORPG_PRODUCT_H
#define ORPG_PRODUCT_H

#ifdef __cplusplus
extern "C"
{
#endif

enum {				/* product types */
    RPGP_VOLUME = 1,		/* volume based product */
    RPGP_ELEVATION,		/* elevation based product */
    RPGP_TIME,			/* time based product */
    RPGP_ON_DEMAND,		/* product generated on demand */
    RPGP_ON_REQUEST,		/* product generated on request */
    RPGP_RADIAL,		/* radial based product */
    RPGP_EXTERNAL		/* product external to ORPG */
};

enum {				/* operation modes - See ICD */
    RPGP_OP_MAINTENANCE = 1,
    RPGP_OP_CLEAR_AIR,
    RPGP_OP_WEATHER
};

enum {				/* component types */
    RPGP_RADIAL_COMP = 1,
    RPGP_GRID_COMP,
    RPGP_AREA_COMP,
    RPGP_TEXT_COMP,
    RPGP_TABLE_COMP,
    RPGP_EVENT_COMP
};


typedef struct {		/* parameter struct */
    char *id;			/* identifier of the parameter */
    char *attrs;	 	/* parameter attributes in text format */
} RPGP_parameter_t;

/* A product and each of its components may have a number of parameters. Each
   parameter is identified by an identifier. A parameter is described by a
   subset of the following attributes: "name", "type", "unit", "range" (valid
   data range), "value" (or values), "default" (default value or values),
   "accuracy" and "description". The data attributes are specified in an
   platform-independent and self-descriptive ASCII-text based format (See Note
   2). Examples are

	"name = Storm Cell ID; type = string; value = A0; range = [A0, Z9]; 
			description = Two character ID. The sequence is 
			recycled following Z9.;"

	"name = Mesocyclone Azimuth; type = float; unit = degree; 
			value = 124.0; range = [0, 360]; accuracy = 1;"

   If a parameter exists in multiple components, identical attributes need to
   be specified only in the first occurrence of the parameter.
*/

typedef struct {		/* struct for binary data */
    char *attrs;		/* data attributes in text format (See Note 2).
				   The following attributes may be specified: 
				   "name", "type", "unit", "range", 
				   "description", "conversion" (data conversion
				   method), and "exception" (exceptional data 
				   values and meanings). Attribute "type" 
				   (Refer to Note 2) is required. Type "string"
				   is not supported in RPGP_data_t. */
    void *data;			/* pointer to the data */
} RPGP_data_t;			/* This is the struct for representing binary 
				   product data. The data is fully described by
				   the data attributes "attrs". The attributes
				   allow us to correctly interpret the raw data
				   stored in "data". They also allow automatic
				   serialization and byte swapping. */

typedef struct {		/* product struct */
    char *name;			/* product name */
    char *description;		/* product description (may contain version 
				   info) */
    int product_id;		/* product id (code) */
    int type;			/* product type (RPGP_VOLUME... except 
				   RPGP_EXTERNAL) */
    unsigned int gen_time;	/* product generation time */

    char *radar_name;		/* radar name. NULL or empty string indicates
				   the radar info is not applicable. The radar
				   info is applicable for products based on 
				   single radar data. The following three 
				   fields are used only if radar_name is 
				   specified. If not used, 0 is assigned. */
    float radar_lat;		/* radar latitude location (in degrees) */
    float radar_lon;		/* radar longitude location (in degrees) */
    float radar_height;		/* radar height location (in meters) */

    unsigned int volume_time;	/* volume scan start time. This and the 
				   following 6 fields are used only for single
				   radar based products. If not used, 0 is 
				   assigned. */
    unsigned int elevation_time;/* elevation scan start time. Used only for 
				   elevation based products. */ 
    float elevation_angle;	/* elevation angle in degrees. Used 
				   only for elevation based products. */
    int volume_number;		/* volume scan number */
    short operation_mode;	/* operation mode (RPGP_OP_MAINTENANCE...) */
    short vcp;			/* VCP number */
    short elevation_number;	/* elevation number within volume scan. Used 
				   only for elevation based products. */

    short compress_type;	/* compression type (currently not used and 
				   must set to 0) */
    int size_decompressed;	/* size after decompressing (currently not used
				   and must set to 0) */

    int numof_prod_params;	/* number of specific product parameters */
    RPGP_parameter_t *prod_params;
				/* specific product parameter list */

    int numof_components;	/* number of components or events */
    void **components;		/* component or event list. See Note 0. */
} RPGP_product_t;

/* A product is a struct of type "RPGP_product_t". The field of
   "component_list" is compressed if a compression method is chosen for the
   product. A product represented in such a C struct can be
   serialized/deserialized with product-independent routines for storage and 
   data communication.
*/

typedef struct {		/* product struct */
    char *name;                 /* product name */
    char *description;          /* product description (may contain version 
				   info) */
    int product_id;             /* product id (code) */
    int type;                   /* product type (must be RPGP_EXTERNAL) */
    unsigned int gen_time;      /* product generation time */
    short spare[5];		/* fields reserved for future use (must set to
				   0) */

    short compress_type;	/* compression type (currently not used and 
				   must set to 0) */
    int size_decompressed;	/* size after decompressing (currently not used
				   and must set to 0) */

    int numof_prod_params;      /* number of specific product parameters */
    RPGP_parameter_t *prod_params;
				/* specific product parameter list */

    int numof_components;       /* number of components or events */
    void **components;          /* component or event list. See Note 0. */
} RPGP_ext_data_t;

/* 
    Note 0: When the product contains multiple detected events, this is an 
    array of pointers to RPGP_event_t. A product can have any number of events.
    If there is only one event, this is an array of pointers each of which 
    points to one of the product component structures (RPGP_radial_t, 
    RPGP_grid_t, RPGP_area_t, RPGP_text_t and RPGP_table_t). A product can have
    any number of components of mixed types.

    DATE        VERSION   PROGRAMMER          NOTES
    --------    -------   ----------          -----
    20100107    0001      James Ward          Fix for CCR NA09-00054

    CHANGE PACKET 28 RADIAL ANGLE DEFINITION

    "The radial angle definition for the radial component specifies center angle.
     This is inconsistent with all other radial-based packet definitions.
     Reason for Change: Change the definition for radial angle from 'center' to 'leading edge'.
     Leading edge is defined as the counter-clockwise edge angle for the start of the radial."
*/

typedef struct {
    float azimuth;		/* azimuth of the leading edge of the radial in degrees */
    float elevation;		/*  */
    float width;		/* radial width in degrees */
    int n_bins;			/* number of bins */
    RPGP_data_t bins;    	/* the radial data */
} RPGP_radial_data_t;

typedef struct {		/* radial data product component */
    int comp_type;		/* must be "RPGP_RADIAL_COMP" */
    char *description;		/* component description - e.g. Data field name */
    float bin_size;		/* size of each range bin in meters */
    float first_range;		/* range of the center of the first bin in 
				   meters */

    int numof_comp_params;	/* number of component parameters */
    RPGP_parameter_t *comp_params;
				/* component parameters - See Note 1 */

    int numof_radials;		/* number of radials */
    RPGP_radial_data_t *radials;/* list of radials */
} RPGP_radial_t;		/* RPGP_radial_t is a special case of the
				   two dimensional gridded data. We define it 
				   as a separate component type so we can 
				   customize it for conveniently representing 
				   various base radar data products. */

enum {				/* for RPGP_grid_t.grid_type */
    RPGP_GT_ARRAY = 1,		/* non-geographical array */
    RPGP_GT_EQUALLY_SPACED,	/* flat equally spaced grid */
    RPGP_GT_LAT_LON,		/* equally spaced latitude-longitude grid */
    RPGP_GT_POLAR		/* rotated pole grid */
};				/* more types to be identified and added */

typedef struct {		/* gridded data product component */
    int comp_type;		/* must be "RPGP_GRID_COMP" */
    int n_dimensions;		/* number of dimensions (1, 2, 3, or 4) */
    int *dimensions;            /* size of each dimension, order should be
                                   fastest changing dimension to slowest
                                   changing */
    int grid_type;
    int numof_comp_params;	/* number of component parameters */
    RPGP_parameter_t *comp_params; /* component parameters - See Note 1. Should
                                   specify which dimension(s) the attribute
                                   applies to */
    RPGP_data_t data;    	/* the grid data. The gridded data are 
				   stored here as a one dimensional array with
				   the index for the first dimension moving 
				   the fastest. */
} RPGP_grid_t;    		/* Grid origin is defined by component parameter.
                                   For equally spaced dimensions, we use
                                   component parameters for specifying the step
                                   sizes. For each unequally spaced grid
                                   dimension, we use an additional 1-D grid
                                   component to specify the grid pointer
                                   locations in that dimension. */


/*
    Note 1: Component parameters are either definitive or descriptive.
    Definitive component parameters are required and predefined. Examples are

	The location of the origin and the coordinate orientation for certain
	grids.

	For equally spaced grid, the step size for each dimension. 

	The altitude of a geo-area if the altitude is relevant.

    The definitive component parameters must be predefined so the user of the 
    product can interpret and display the data product-independently.

    Descriptive component parameters, on the other hand, provide additional 
    descriptions of the product component. Examples are the data field name, 
    the intensity of the event, the forecast position and so on.
*/

typedef struct {		/* defining a geographical location */
    float lat;			/* latitude location (in degrees) */
    float lon;			/* longitude location (in degrees) */
} RPGP_location_t;

typedef struct {		/* defining a geographical location. The
				   origin is the radar. X points to the East 
				   and Y points to the North. */
    float x;			/* X coordinate (in kilo-meters) */
    float y;			/* Y coordinate (in kilo-meters) */
} RPGP_xy_location_t;

typedef struct {		/* defining a geographical location. The
				   origin is the radar. For 0 Azimuth, the true
				   north is used. */
    float range;		/* Range (in kilo-meters) */
    float azi;			/* Azimuth (in degrees) */
} RPGP_azran_location_t;

enum {				/* values for RPGP_area_t.area_type */
    RPGP_AT_POINT = 1,		/* a geographical point (0-D) */
    RPGP_AT_AREA,		/* a closed geographical area (2-D) */
    RPGP_AT_POLYLINE		/* a geographical polyline (1-D) */
};

typedef struct {		/* area product component */
    int comp_type;		/* must be "RPGP_AREA_COMP" */

    int numof_comp_params;	/* number of component parameters */
    RPGP_parameter_t *comp_params;
				/* component parameters - Refer to Note 1. */

    int area_type;		/* See comments after this struct. */
    int numof_points;		/* size of the array "points" */
    void *points;		/* list of geographical points that define the
				   area. See comments after this struct. */
} RPGP_area_t;			/* RPGP_area_t represents an open polyline, a 
				   closed area or a single geographical point.
				   One can use multiple components of 2-D area
				   to represent slices of a 3-D area. */

/*
   The lower two bytes of RPGP_area_t.area_type represent the area type
   (RPGP_AT_*) and the upper two bytes of this field are used for the location
   type (type used for RPGP_area_t.points). Currently defined location types
   are RPGP_location_t, RPGP_xy_location_t and RPGP_azran_location_t. To
   specify the location type, use one of the following macros. It should be
   OR'd with the area type (RPGP_AT_*).
*/
#define RPGP_LATLON_LOCATION	0
#define RPGP_XY_LOCATION	0x10000
#define RPGP_AZRAN_LOCATION	0x20000

/* Macros to get respectively the location type and the area type. */
#define RPGP_LOCATION_TYPE(area_type)	((area_type) & 0xffff0000)
#define RPGP_AREA_TYPE(area_type)	((area_type) & 0xffff)

typedef struct {		/* text product component */
    int comp_type;		/* must be "RPGP_TEXT_COMP" */

    int numof_comp_params;	/* number of component parameters */
    RPGP_parameter_t *comp_params;
				/* component parameters - Refer to Note 1. */

    char *text;			/* text content */
} RPGP_text_t;			/* RPGP_text_t represents a non-geographical 
				   text. For representing geographical text, 
				   one can use RPGP_area_t. */

typedef struct {		/* string component */
    char *text;			
} RPGP_string_t;		

typedef struct {		/* table product component */
    int comp_type;		/* must be "RPGP_TABLE_COMP" */

    int numof_comp_params;	/* number of component parameters */
    RPGP_parameter_t *comp_params;
				/* component parameters - Refer to Note 1. */

    RPGP_string_t title;	/* table title */
    short n_columns;		/* number of columns */
    short n_rows;		/* number of rows */

    RPGP_string_t *column_labels;/* column labels (must be n_columns strings). 
				   Empty if column labels are not used. */
    RPGP_string_t *row_labels;	/* row labels (must be n_rows strings). Empty 
				   if row labels are not used.*/
    RPGP_string_t *entries;	/* table entries (must be an n_rows * n_columns
				   array with row index moving the fastest) */
} RPGP_table_t;

typedef struct {		/* event struct */
    int comp_type;		/* must be "RPGP_EVENT_COMP" */

    int numof_event_params;	/* number of event parameters */
    RPGP_parameter_t *event_params;
				/* event parameters. These are component 
				   parameters shared by all components in this 
				   event struct. */

    int numof_components;	/* number of components */
    void **components;		/* component list - An array of pointers each
				   of which points to one of the product
				   component structures. An event can have any
				   number of components of mixed types. 
				   Possible types are RPGP_radial_t, 
				   RPGP_grid_t, RPGP_area_t, RPGP_text_t and 
				   RPGP_table_t */
} RPGP_event_t;			/* RPGP_event_t is used for containing all 
				   product components that belong to the same
				   detected event. */

/*
    Note 2: Format description of the ASCII-text attribute specifications.

    1. The attributes are represented by an ASCII string. The string consists
    of a number of sections terminated by ";", each of which specifies an
    applicable attribute. ";" after the last section is optional. Each section
    must be in the form of "attribute name = attribute description" where
    "attribute name" must be one of the following: "name", "type", "unit",
    "range", "value", "default", "accuracy", "description", "conversion" and
    "exception". The attribute name is case-insensitive. That is, for example,
    "name", "Name" and "NAME" are all valid and identical. "attribute
    description" is a character string that describes the value of the
    attribute as explained in the following.

    2. Attribute description:

	"name": The name of the parameter. An example is 
	"name = 2D feature altitude".

	"type": One of the following type names: "int", "short", "byte"
	(4-byte, 2-byte and 1-byte integer respectively),"float", "double" 
	(4-byte and 8-byte IEEE floating point numbers respectively), "string"
	(ASCII character string), "uint", "ushort" and "ubyte" (unsigned 
	versions of int, short and byte). An example is "type = int". The type 
	name is case-insensitive.

	"unit": The physical unit of the data value. Standard unit names are to
	be defined. Examples are "unit = meter" and "unit = percent".

	"range": The set of all valid values for the parameter. The range can
	be specified with one of the following three formats:

	    a. Single interval specification defined by "[min, max]" where
	    "min" and "max" are respectively the minimum and maximum values.
	    "[" and "]" can be replaced by "(" and ")" respectively if the
	    boundary is not inclusive. Unlimited boundary is specified by "-".
	    Examples are "range = [1, 2]", "range = (1, 2]", "range = [1., -)",
	    "range = [A, Z]" (character string type), and "range = (-, -)".

	    b. A list of valid values: { v1, v2, ...}. Examples are "range =
	    {1, 2, 3}" and "range = {reflectivity, velocity, spectrum width}.

	    c. A named method that checks the range. The method name is
	    enclosed by "<" and ">". The method must be described elsewhere.

	"value" and "default": A value or a list of values separated by ",".
	Examples are "value = 1", "value = 1.0, 2., 3.0" and "value = Yes, No".

	"accuracy": The accuracy of the data. [max_error] is used for the
	absolute maximum error and (max_error) for the relative maximum error.

	"description": A text description of the data.

	"conversion": The way to convert binary data stored externally. The
	conversion can be specified with one of the following formats:

	    a. Format [scale, offset] is used for scale-offset type of
	    conversion: value = data * scale + offset. An example is
	    "conversion = [2., 64.]".

	    b. Format {valueMap, data1, value1, data2, value2, ...} for data
	    mapping conversions. Where "valueMap" is a reserved key word.
	    "data1", "data2" ... are the data and "value1", "value2" ... are
	    the values to convert to. An example is "conversion = {valueMap, 1,
	    -5., 2, 0., 3, 50., 4, 100.}".

	    c. Format <method> is used for named conversion method. The method 
	    must be described elsewhere.

	    Elements of binary data array are assumed to be stored one after
	    another in the local byte order for types other than "bit" and
	    "string". For type "bit", we assume that the elements are stored in
	    a byte array each of which holds 8 elements. The first bit element
	    is stored in the left-most bit in the bytes. For type "string",
	    elements are null-terminated strings and stored one after another
	    with the null terminator.

	"exception": A list of the exceptional data values and their meanings.
	An example is "exception = 0, below threshold, 1, missing data".
	Standard vocabulary for describing exceptional values needs to be
	established in the future.

    3. When characters ";", "=" and "," are used for formatting purpose,
    characters "space", "tab" and "line return" surrounding them are
    insignificant. That is, for example, "name = short", "name=short" and 
    "name   =short" are all identical. Non-formatting use of ";" and "," are
    allowed if no ambiguity is introduced. In case of ambiguity, "\" can be
    used in front of characters ";" and "," to indicate that they are not
    interpreted as formatting characters. The part of "Attribute description"
    is case-sensitive except otherwise specified.
*/

int RPGP_product_serialize (void *prod, char **serial_data);
int RPGP_product_deserialize (char *serial_data, int size, void **prod);
int RPGP_product_free (void *prod);

#ifdef __cplusplus
}
#endif

#endif			/* #ifndef ORPG_PRODUCT_H */

