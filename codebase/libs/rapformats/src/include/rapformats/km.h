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
 * km.h - Header file for the km (Kavouras Mosaic) module of the
 *        rapformats library.
 *
 * Nancy Rehak
 * Janurary 1997
 *                   
 *************************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef KM_H
#define KM_H

#include <dataport/port_types.h>
#include <rapformats/kav_grid.h>
#include <toolsa/udatetime.h>


/*
 * Output types for KM_vip2dbz_init.
 */

#define KM_OUTPUT_PLAIN         0
#define KM_OUTPUT_DOBSON        1

/*
 * Debug levels for KM_decode_header
 */

#define KM_DEBUG_OFF             0
#define KM_DEBUG_WARNINGS        1
#define KM_DEBUG_NORM            2
#define KM_DEBUG_EXTRA           3

/*
 * Kavouras mosaic header structure
 */

typedef struct
{
  double   lat0;
  double   lon0;
  double   lon_width;
  double   aspect;

  date_time_t time;

  int	nx;
  int	ny;
  int	nsites;
  int	site_x[MAX_SITES];
  int	site_y[MAX_SITES];
  int	site_status[MAX_SITES];

  char  site_id[MAX_SITES][3];

  char    day[3];
  char    mon[4];
  char    year[3];
  char    hour[3];
  char    min[3];
  
  char	  filename[13];
  char    validdate[7];
  char    validtime[5];
  char    projection[3];	/* Inconsistancy in docs; could be [5] */
  
} KM_header_t;

/************************************************************************
 * Function prototypes.
 ************************************************************************/

/************************************************************************
 * KM_decode_header()
 */

void KM_decode_header(char *file_buf,
		      int file_size,
		      KM_header_t *header,
		      int debug_level);

/************************************************************************
 * KM_fix_header_time(): Reset the header time so that it is within 12
 *                       hours of the file time.
 */

void KM_fix_header_time(time_t file_time,
			KM_header_t *header);

/************************************************************************
 * KM_set_header_time(): Set the Kavouras header time fields to the
 *                       given time.
 */

void KM_set_header_time(time_t new_time,
			KM_header_t *header);

/************************************************************************
 * KM_unpack_init(): 
 */

void KM_unpack_init(char *file_buf, int file_len);

/************************************************************************
 * KM_unpack_raw_line(): Repack the buffin; return the output buffer and
 *                       its length.
 */

ui08 *KM_unpack_raw_line(void);

/************************************************************************
 * KM_vip2dbz_init(): 
 */

void KM_vip2dbz_init(double scale, double bias,
		     double *vip2dbz, int vip2dbz_len,
		     int output_type, double dbz_threshold);



# endif     /* KM_H */

#ifdef __cplusplus
}
#endif
