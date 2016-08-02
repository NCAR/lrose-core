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
 * fos.h - Header file for the Family of Services module of the
 *         rapformats library.
 *
 * Nancy Rehak
 * March 1997
 *                   
 *************************************************************************/

#include <dataport/port_types.h>
#include <stdio.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef FOS_H
#define FOS_H

/************************************************************************
 * SIGMET types
 ************************************************************************/

/*
 * Define the values used in the SPDB structure.
 */

typedef enum
{
  SIGMET_TYPE_UNKNOWN = -1,

  SIGMET_UNPARSABLE = 1,

  SIGMET_CONVECTIVE = 10,
  SIGMET_TURB       = 11,
  SIGMET_ICING      = 12,
  SIGMET_MISC       = 13
} SIGMET_type_t;


typedef enum
{
  SIGMET_REPORT_NEW,
  SIGMET_REPORT_CORRECTION,
  SIGMET_REPORT_AMENDMENT
} SIGMET_report_type_t;


/*
 * Define the structure used for storing the SIGMET data in the SPDB
 * files.
 */

typedef struct
{
  fl32 lat;
  fl32 lon;

  /*
   * Keep slope/intercept information for line segment connection this
   * vertex with the next vertex here.  This information is used in the
   * point-and-click algorithm.
   */

  fl32 slope;
  fl32 intercept;
} SIGMET_vertex_t;


typedef struct
{
  fl32 min_lat;
  fl32 min_lon;
  fl32 max_lat;
  fl32 max_lon;
} SIGMET_box_t;

#define SIGMET_ID_LEN           8
#define FORECAST_REGION_LEN     4

typedef struct
{
  ui08            sigmet_id[SIGMET_ID_LEN];   /* NULL terminated */
  si32            sigmet_type;                /* SIGMET_type_t */
  si32            report_type;                /* SIGMET_report_type_t */
  si32            cancel_flag;
  si32            start_time;                 /* time_t */
  si32            end_time;                   /* time_t */
  ui08            forecast_region[FORECAST_REGION_LEN];   /* NULL terminated */
  ui08            forecast_letter;
  ui08            forecast_alignment[3];
  SIGMET_box_t    bounding_box;
  si32            num_vertices;
  si32            text_len;
  si32            text_offset;
  SIGMET_vertex_t vertices[1];      /* num_vertices of these */
/*ui08            text[1];*/        /* text_len characters, NULL terminated */
} SIGMET_spdb_t;

typedef struct
{
  char            sigmet_id[SIGMET_ID_LEN];   /* NULL terminated */
  SIGMET_type_t   sigmet_type;
  SIGMET_report_type_t  report_type;
  int             cancel_flag;
  time_t          start_time;                 /* time_t */
  time_t          end_time;                   /* time_t */
  char            forecast_region[FORECAST_REGION_LEN];   /* NULL terminated */
  char            forecast_letter;
  SIGMET_box_t    bounding_box;
  int             num_vertices;
  SIGMET_vertex_t *vertices;                  /* num_vertices of these */
} SIGMET_decoded_t;


/************************************************************************
 * WXADV types
 ************************************************************************/

#define FIX_ID_LEN         4

typedef struct
{
  char   id[FIX_ID_LEN];
  double lat;
  double lon;
} WXADV_fix_t;

typedef struct
{
  int   num_lines;
  int   curr_line;
  char  **lines;
} WXADV_text_t;

/************************************************************************
 * Function prototypes for SIGMET functions.
 ************************************************************************/

/************************************************************************
 * SIGMET_box_from_BE() - Convert the SIGMET box information from big-
 *                         endian format to native format.
 */

void SIGMET_box_from_BE(SIGMET_box_t *box);

/************************************************************************
 * SIGMET_box_to_BE() - Convert the SIGMET box information from native
 *                      format to big-endian format.
 */

void SIGMET_box_to_BE(SIGMET_box_t *box);

/************************************************************************
 * SIGMET_decoded_to_spdb() - Convert a decoded SIGMET to the SPDB format.
 */

SIGMET_spdb_t *SIGMET_decoded_to_spdb(WXADV_text_t *sigmet_text,
				      SIGMET_decoded_t *decoded_sigmet);

/************************************************************************
 * SIGMET_print_box() - Print the SIGMET box information to the indicated
 *                      stream in ASCII format.
 */

void SIGMET_print_box(FILE *stream, SIGMET_box_t *box);

/************************************************************************
 * SIGMET_print_decoded() - Print the decoded SIGMET information to the
 *                          indicated stream in ASCII format.
 */

void SIGMET_print_decoded(FILE *stream, SIGMET_decoded_t *sigmet);

/************************************************************************
 * SIGMET_print_spdb() - Print the SPDB SIGMET information to the
 *                       indicated stream in ASCII format.
 */

void SIGMET_print_spdb(FILE *stream, SIGMET_spdb_t *sigmet);

/************************************************************************
 * SIGMET_print_vertex() - Print the SIGMET vertex information to the
 *                         indicated stream in ASCII format.
 */

void SIGMET_print_vertex(FILE *stream, SIGMET_vertex_t *vertex);

/************************************************************************
 * SIGMET_report_type_to_string() - Convert the SIGMET report type to a
 *                                  string, generally for printing.
 */

char *SIGMET_report_type_to_string(SIGMET_report_type_t report_type);

/************************************************************************
 * SIGMET_spdb_from_BE() - Convert the SPDB SIGMET information from big-
 *                         endian format to native format.
 */

void SIGMET_spdb_from_BE(SIGMET_spdb_t *sigmet);

/************************************************************************
 * SIGMET_spdb_to_BE() - Convert the SPDB SIGMET information from native
 *                       format to big-endian format.
 */

void SIGMET_spdb_to_BE(SIGMET_spdb_t *sigmet);

/************************************************************************
 * SIGMET_type_to_string() - Convert the SIGMET type to a string,
 *                           generally for printing.
 */

char *SIGMET_type_to_string(SIGMET_type_t type);

/************************************************************************
 * SIGMET_vertex_from_BE() - Convert the SIGMET vertex information from big-
 *                         endian format to native format.
 */

void SIGMET_vertex_from_BE(SIGMET_vertex_t *vertex);

/************************************************************************
 * SIGMET_vertex_to_BE() - Convert the SIGMET vertex information from
 *                         native format to big-endian format.
 */

void SIGMET_vertex_to_BE(SIGMET_vertex_t *vertex);

/************************************************************************
 * SIGMET_wxadv_to_decoded() - Converts a textual SIGMET stored in the
 *                             WXADV_text_t format into an array of
 *                             decoded SIGMETS.
 */

SIGMET_decoded_t *SIGMET_wxadv_to_decoded(WXADV_text_t *sigmet,
					  time_t sigmet_date,
					  WXADV_fix_t *fixes,
					  int num_fixes,
					  int *num_sigmets);


/************************************************************************
 * Function prototypes for SIGMET functions.
 ************************************************************************/

/************************************************************************
 * WXADV_read_fixes() - Read the fixes from a NWS fixes file and return
 *                      them in an array of structures.  These fixes are
 *                      used in describing the polygon vertices in
 *                      AIRMETs and SIGMETs.
 */

WXADV_fix_t *WXADV_read_fixes(char *fixes_file,
			      int *num_fixes);


#endif     /* SIGMET_H */

#ifdef __cplusplus
}
#endif
