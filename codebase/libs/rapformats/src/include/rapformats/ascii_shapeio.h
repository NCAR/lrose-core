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
*                                                                       *
*                             shapeio.h
*                                                                       *
*************************************************************************

                        Mon Mar 20 17:25:50 1995

       Description:     Header file for shapeio software.

       See Also:

       Author:          Dave Albo

       Modification History:

*/

#ifndef SHAPEIO_H
#define SHAPEIO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * System include files
 */
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <ctype.h>

/*
 * Local include files
 */
#include <toolsa/utim.h>

/*
 * Definitions / macros / types
 */

/*
 * List of known file name suffixes.
 */
#define SIO_FILE_PROD        "prod"
/* same file as SIO_TEMP_FILE; broken up by days (for archival purposes) */
#define SIO_FILE_PRODUCTS    "prdsprod"

/*
 * Fixed name for temporary file.
 */
#define SIO_TEMP_FILE        "prod.temp"

/*
 * Max length of various labels in the lines.
 */
#define SIO_LABEL_LEN 80

/*
 * abs(u) or abs(v) must be <= SIO_UV_LIMIT
 * This applies to both wind u,v and bdry u,v.
 * Used for reading in boundaries and extraps.
 * u and v are set to MISSING_UV if they're not in the above range.
 */
#define SIO_UV_LIMIT      3600  /* km per hour */
#define SIO_MISSING_UV    9999 

/*
 * File access modes:
 */
#define SIO_MODE_CLOSEST    0  /* closest shapes to center time*/
#define SIO_MODE_CLOSEST_LE 1  /* closest less or equal */
#define SIO_MODE_CLOSEST_GE 2  /* closest greater or eqaual*/
#define SIO_MODE_ALL        3  /* all shapes in time window */

/*
 * Modes to append to existing files:
 */
#define SIO_APPEND_NORMAL 0       /* open file for append, write */
#define SIO_APPEND_SAFE   1       /* make copy of file, append to that,
			       * overwrite original file */
/*
 * Known product type/subtype names. (up to 3 char)
 * Fixed names:
 */
#define SIO_PRODTYPE_BAD                "???"
#define SIO_BOUNDARY_IDENTIFIER         "BD"
#define SIO_PRODTYPE_BOUNDARY_TRUTH     "BDT"  
#define SIO_PRODTYPE_BOUNDARY_MIGFA     "BDM"  
#define SIO_PRODTYPE_BOUNDARY_COLIDE    "BDC"
#define SIO_PRODTYPE_COMBINED_NC_ISSUE  "ANI"
#define SIO_PRODTYPE_COMBINED_NC_VALID  "ANV"
#define SIO_PRODTYPE_EXTRAP_ISSUE_ANW   "AXI"
#define SIO_PRODTYPE_EXTRAP_VALID_ANW   "AXV"
#define SIO_PRODTYPE_FIRST_GUESS_ISSUE  "FGI"
#define SIO_PRODTYPE_FIRST_GUESS_VALID  "FGV"
#define SIO_PRODTYPE_MINMAX_NC_ISSUE    "MMI"
#define SIO_PRODTYPE_MINMAX_NC_VALID    "MMV"

#define SIO_SUBTYPE_ALL                 "ALL"

/*
 * First letter of extrap names...followed by # of minutes..
 */
#define SIO_PRODTYPE_EXTRAP_ISSUE_MIGFA  "E"
#define SIO_PRODTYPE_EXTRAP_VALID        "V"
#define SIO_PRODTYPE_EXTRAP_ISSUE_COLIDE "X"

#define SIO_NAME_MATCH(s, t)  (strcmp((s), (t)) == 0)
#define SIO_IS_EXTRAP(s,p) ((strncmp((s),p, strlen(p)) == 0) && isdigit(*((s)+strlen(p))))

#define SIO_IS_EXTRAP_ISSUE_COLIDE(s) (SIO_IS_EXTRAP((s), SIO_PRODTYPE_EXTRAP_ISSUE_COLIDE))
#define SIO_IS_EXTRAP_ISSUE(s) (SIO_IS_EXTRAP((s),SIO_PRODTYPE_EXTRAP_ISSUE_MIGFA) || SIO_IS_EXTRAP_ISSUE_COLIDE((s)) || strcmp((s), SIO_PRODTYPE_EXTRAP_ISSUE_ANW) == 0)
#define SIO_IS_EXTRAP_VALID(s) (SIO_IS_EXTRAP((s),SIO_PRODTYPE_EXTRAP_VALID) || strcmp((s), SIO_PRODTYPE_EXTRAP_VALID_ANW) == 0)
#define SIO_IS_BOUNDARY_DETECTION(s) (strncmp((s), SIO_BOUNDARY_IDENTIFIER, strlen(SIO_BOUNDARY_IDENTIFIER)) == 0)

/*
 * A data request to sio consists of this struct
 */
typedef struct
{
    char *directory;  /* where is data*/
    char *suffix;      /* type of file */
    time_t target_time;/* center time (unix time) */
    int delta_time;    /* window allowed (secnds) */
    int extrap;        /* seconds forward (0,1,..) */
    int delta_extrap;  /* windowallowed (secnds) */
    int mode;          /* one of the access modes */
} SIO_read_request_t;


/*
 * Here is our sio data element...it consists of any number of
 * polylines:
 */
typedef struct
{
    char object_label[SIO_LABEL_LEN]; /* label associated with this polyline */
    int npoints;                      /* # of points */
    int nseconds;                     /* # of seconds extrapolation it is */
    float *lat;                       /* array of latitudes */
    float *lon;                       /* array of longitudes */
    float *u_comp;                    /* array of horizontal motion dir */
                                      /*   vector components (in km/h) */
    float *v_comp;                    /* array of vertical motion dir vector */
                                      /*   components (in km/h) */
    float *value;                     /* a value for each point (bdry rel */
                                      /*   steering flow value for the */
                                      /*   initiation zone) */
} SIO_polyline_object_t;

/*
 * with a fixed header portion:
 */
typedef struct
{
    char *type;                       /* product type*/
    char *sub_type;                   /* product subtype */
    int sequence_number;              /* product counter*/
    int group_id;                     /* group id number */
    time_t gen_time;                  /* time of product generation */
    time_t data_time;                 /* time of data used to create*/
    time_t valid_time;                /* time product is valid */
    time_t expire_time;               /* time product becomes invalid*/
    char description[SIO_LABEL_LEN];  /* label associated with the product*/
    float motion_dir;                 /* degreesT (all objects move together)*/
    float motion_speed;               /* km/hr    (all objects move together)*/
    char *line_type;                  /* line type (for colide bdrys, */
                                      /*    extraps) */
    float qual_value;                 /* quality (confidence) value (for */
                                      /*    colide) */
    float qual_thresh;                /* quality threshold (for colide) */
    int num_objects;                  /* # of objects that follows*/
    SIO_polyline_object_t *P;           /* num_objects of them..*/
} SIO_shape_data_t;


/*
 * The index file records consist of:
 */
typedef struct
{
    time_t data_time;       /* time of data used to make objects */
    int    extrap_seconds;  /* delta time (secs) till data is valid */
    int    file_index;      /* offset from file beginning (bytes) */
    char  *type;            /* same as type found in SIO_shape_data_t*/
} SIO_index_data_t;



/*
 * Function prototypes
 */

/*
 * Return array of shapes, satisfying input criteria, NULL for none.
 *
 */

extern SIO_shape_data_t *SIO_read_data(SIO_read_request_t *R, /* request*/
				       int *nshapes,          /* returned # of
							       * shapes.
							       */
				       char *bdry_index_dir); /* dir for bdry
							       * index files
							       */

/*
 * Open indicated shape data file.
 */

extern FILE *SIO_open_data_file(const char   *directory,      /* where */
				const char   *suffix,         /* file suffix */
				time_t file_time,       /* when */
				const char   *mode,           /* "r" or "w"*/
				int    minutes_offset); /* number of minutes to
							 * offset the time;
							 * effectively
							 * adjusting the time
							 * when a new file is
							 * opened
							 */

extern FILE *SIO_open_named_data_file(const char *name,  /* its name */
				      const char *mode); /* "r" or "w"*/

/*
 * Append input data to proper file
 */

extern int SIO_append_data(const char *directory,     /* where */
			   const char *suffix,        /* file suffix */
			   SIO_shape_data_t *S, /* some data products,
						 * filled in
						 */
			   int nshapes,         /* # of data shape products */
			   int method,          /* SIO_APPEND_NORMAL or
						 * SIO_APPEND_SAFE
						 */
			   int minutes_offset); /* number of minutes to offset
						 * the time; effectively
						 * adjusting the time when a
						 * new file is opened
						 */


/*
 * Write input data to named file.
 */

extern int SIO_write_to_named_file(char *directory,     /* where */
				   char *name,          /* file name */
				   SIO_shape_data_t *S, /* products, filled
							 * in
							 */
				   int nshapes,         /* # of data shape
							 * products
							 */
				   char *mode,          /* "w", "a" */
				   int print_shapes);   /* print shape info or
							 * not
							 */


/*
 * Write input data to file
 */
extern int SIO_write_data(FILE *fd,               /* opened file */
			  SIO_shape_data_t *S,    /* some data */
			  int print_shapes);      /* print shape info or not */


/*
 * Print contents of input data struct.
 */

extern void SIO_print_shape(SIO_shape_data_t *s,
			    int level); /* 0 = 1line,
					 * 1 = 1 line of 0th polyline.
					 * 2 = 1 line of all polylines.
					 * 3 = full, 0th polyline.
					 * 4 = full, all polylines.
					 */

/*
 * Free memory allocated in input shape structs.
 * Should be called after done using the shapes.
 */

extern void SIO_free_shapes(SIO_shape_data_t *s, /* the shapes */
			    int nshapes);        /* how many */


/*
 * Print contents of an entire SIO data file.
 */
extern void SIO_file_dump(const char *directory,   /* where is data */
			  const char *suffix,      /* file suffix */
			  time_t t,          /* time of file */
			  int level);        /* 0 = 1line,
					      * 1 = 1 line of 0th polyline.
					      * 2 = 1 line of all polylines.
					      * 3 = full, 0th polyline.
					      * 4 = full, all polylines.
					      */

/*
 * Return standard sio file name for the inputs.  Returns a pointer
 * to static memory which should NOT be freed by the calling routine.
 */

extern char *SIO_file_name(const char *suffix,    /* file suffix */
			   time_t time);    /* time of data*/

/*
 * Return the file name for the inputs.  Returns a pointer
 * to static memory which should NOT be freed by the calling routine.
 */

extern char *SIO_file_name_full(const char *directory,   /* where */
				const char *suffix,      /* file suffix */
				time_t time,             /* time of data*/
				int    minutes_offset);  /* number of minutes
							  * to offset the time;
							  * effectively 
							  * adjusting the time
							  * when a new file is
							  * opened
							  */

extern void SIO_clear_read_buf();

/*
 * Reads the next shape record from the input file.  You need to call
 * SIO_clear_read_buf() before using this routine on a new input file.
 * Returns 0 on error or EOF
 */

extern int SIO_read_record(FILE *fp,	        /* opened file */
			   SIO_shape_data_t *S, /* returned data */
			   int *pos);           /* returned bytes from
						 * beginning of file to start
						 * of next record
						 */

extern char *SIO_issue_to_valid_extrap(const char *type);

/*
 * If needed, recreate an index file.
 * 
 * On exit, index file has been made current and:
 *          Return pointer to array of structs containing index data.
 *
 *          Return NULL for no data or error making the index file or
 *          index list.
 */

extern SIO_index_data_t *SIO_rebuild_index_file(const char *directory,  /* where */
						const char *suffix,     /* what */
						time_t time,      /* when */
						int *nindex,      /* how many (returned)*/
						const char *bdry_index_dir,
						/* dir for bdry index file */
						int yesterday_file,   /* flag to look
								       * at yesterday's bdry file */
						char **bdry_file     /* name of bdry
								      * file to use */
						);


extern SIO_index_data_t *SIO_rebuild_named_index_file(const char *dir,    /* output dir */
						      const char *name,   /* name */
						      int *nindex   /* how many (returned)*/
						      );


/*
 * Skip to and read data from file indicated by input index record.
 * Returns a pointer to a static buffer that will be overwritten on
 * the next call to this routine.
 */

extern SIO_shape_data_t *SIO_extract_indexed_data(FILE *fd,
						  SIO_index_data_t *I);

extern
int SIO_clear_data_file(const char *directory,     /* where */
			const char *suffix,         /* file name suffix*/
			time_t time
			);

#ifdef __cplusplus
}
#endif

#endif
