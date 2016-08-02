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
 * mitre.h - Header file for the mitre module of the rapformats library.
 *
 * Nancy Rehak
 * March 1997
 *                   
 *************************************************************************/

#include <stdio.h>

#include <dataport/port_types.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef MITRE_H
#define MITRE_H


/*
 * Number of bytes in the MITRE file header as it actually appears
 * in the file.
 */

#define MITRE_FILE_HEADER_LEN      96


/*
 * Define the MITRE header.  The MITRE header must be decoded
 * from the file because the information in the file is written
 * in a format that can't be cast to a structure on 32-bit
 * architectures.
 */

#define MITRE_MAX_INTENSITIES      16


typedef struct
{
  si16   product_code;        /* 300 */
  si32   file_size;           /* bytes (96 to MAXINT) */
  si32   time;
  si16   num_intensities;     /* 2 to 16 */
  ui08   intensity_table[MITRE_MAX_INTENSITIES];
  si32   n_lat;
  si32   n_lon;
  fl32   origin_lat;         /* top left lat of grid */
  fl32   origin_lon;         /* top left lon of grid */
  fl32   lat_delta;
  fl32   lon_delta;
} MITRE_header_t;

/************************************************************************
 * Function prototypes.
 ************************************************************************/

/************************************************************************
 * MITRE_decode_header(): Extracts the header information from a buffer
 *                        read in from a MITRE file.
 */

void MITRE_decode_header(char *file_buf,
			 MITRE_header_t *header);

/************************************************************************
 * MITRE_flip_grid(): Flips the MITRE grid and data from using the upper
 *                    left corner as the origin to using the lower left
 *                    corner as the origin.  Updates the origin in the
 *                    header to reflect the change in grid definition.
 */

void MITRE_flip_grid(MITRE_header_t *header,
		     ui08 *data);

/************************************************************************
 * MITRE_grids_equal(): Compares the grids in two MITRE headers.  Returns
 *                      TRUE if the grids are the same, FALSE otherwise.
 */

int MITRE_grids_equal(MITRE_header_t *header1,
		      MITRE_header_t *header2);

/************************************************************************
 * MITRE_header_from_be(): Swap the MITRE header bytes, if necessary.
 */

void MITRE_header_from_be(MITRE_header_t *header);

/************************************************************************
 * MITRE_print_header(): Prints a MITRE file header.
 */

void MITRE_print_header(FILE *stream,
			MITRE_header_t *header);

/************************************************************************
 * MITRE_read_file(): Reads in a MITRE file.  Returns the mosaic data
 *                    in intensity level, as it appears in the input file.
 */

ui08 *MITRE_read_file(char *filename,
		      MITRE_header_t *mitre_header);

/************************************************************************
 * MITRE_read_file_inten(): Reads in a MITRE file.  Returns the mosaic
 *                          data with the intensity table values appied
 *                          to the data (the returned buffer contains
 *                          actual data values calculated using the
 *                          intensity table in the header.
 */

ui08 *MITRE_read_file_inten(char *filename,
			    MITRE_header_t *header);


#endif     /* MITRE_H */

#ifdef __cplusplus
}
#endif
