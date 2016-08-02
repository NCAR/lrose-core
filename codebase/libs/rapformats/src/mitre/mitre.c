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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* SCCS info
 *   %W% %D% %T%
 *   %F% %E% %U%
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:45:40 $
 *   $Id: mitre.c,v 1.3 2016/03/03 18:45:40 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * mitre.c: Routines to manipulate MITRE mosaic data.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include <toolsa/os_config.h>

#include <dataport/bigend.h>
#include <dataport/port_types.h>
#include <rapformats/mitre.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/umisc.h>

/************************************************************************
 * MITRE_decode_header(): Extracts the header information from a buffer
 *                        read in from a MITRE file.
 */

void MITRE_decode_header(char *file_buf,
			 MITRE_header_t *header)
{
  char *file_ptr = file_buf;
  
  memcpy((void *)&header->product_code,
	 (void *)file_ptr, 2);
  file_ptr += 2;
  
  memcpy((void *)&header->file_size,
	 (void *)file_ptr, 4);
  file_ptr += 4;
  
  memcpy((void *)&header->time,
         (void *)file_ptr, 4);
  file_ptr += 4;
  
  memcpy((void *)&header->num_intensities,
	 (void *)file_ptr, 2);
  file_ptr += 2;
  
  memcpy((void *)header->intensity_table,
	 (void *)file_ptr, MITRE_MAX_INTENSITIES);
  file_ptr += MITRE_MAX_INTENSITIES;
  
  memcpy((void *)&header->n_lat,
	 (void *)file_ptr, 4);
  file_ptr += 4;
  
  memcpy((void *)&header->n_lon,
	 (void *)file_ptr, 4);
  file_ptr += 4;
  
  assert(sscanf(file_ptr, "%f", &header->origin_lat) == 1);
  file_ptr += 15;
  
  assert(sscanf(file_ptr, "%f", &header->origin_lon) == 1);
  file_ptr += 15;
  
  assert(sscanf(file_ptr, "%f", &header->lat_delta) == 1);
  file_ptr += 15;
  
  assert(sscanf(file_ptr, "%f", &header->lon_delta) == 1);
  
  return;
}


/************************************************************************
 * MITRE_flip_grid(): Flips the MITRE grid and data from using the upper
 *                    left corner as the origin to using the lower left
 *                    corner as the origin.  Updates the origin in the
 *                    header to reflect the change in grid definition.
 */

void MITRE_flip_grid(MITRE_header_t *header,
		     ui08 *data)
{
  static ui08 *temp_data = NULL;
  static int   temp_data_alloc = 0;
  
  int data_size = header->n_lat * header->n_lon;
  ui08 *data_ptr;
  ui08 *temp_data_ptr;
  int i;
  
  /*
   * Make sure there is enough space in the temporary data buffer.
   */

  if (data_size > temp_data_alloc)
  {
    if (temp_data == NULL)
      temp_data = (ui08 *)umalloc(data_size);
    else
      temp_data = (ui08 *)urealloc(temp_data, data_size);
    
    temp_data_alloc = data_size;
  }
  
  /*
   * Copy the data to the temporary data buffer.
   */

  memcpy((char *)temp_data, (char *)data, data_size);
  
  /*
   * Loop through the rows, copying them to the appropriate
   * place in the original buffer.
   */

  for (i = 0; i < header->n_lat; i++)
  {
    data_ptr = data + (i * header->n_lon);
    temp_data_ptr = temp_data + ((header->n_lat - i - 1) * header->n_lon);
    
    memcpy((char *)data_ptr, (char *)temp_data_ptr, header->n_lon);
  }
  
  /*
   * Now update the header values.
   */

  header->origin_lat = header->origin_lat -
    (header->n_lat * header->lat_delta);
  
  return;
}


/************************************************************************
 * MITRE_grids_equal(): Compares the grids in two MITRE headers.  Returns
 *                      TRUE if the grids are the same, FALSE otherwise.
 */

int MITRE_grids_equal(MITRE_header_t *header1,
		      MITRE_header_t *header2)
{
  if (header1->n_lat != header2->n_lat)
    return(FALSE);
  
  if (header1->n_lon != header2->n_lon)
    return(FALSE);
  
  if (header1->origin_lat != header2->origin_lat)
    return(FALSE);
  
  if (header1->origin_lon != header2->origin_lon)
    return(FALSE);
  
  if (header1->lat_delta != header2->lat_delta)
    return(FALSE);
  
  if (header1->lon_delta != header2->lon_delta)
    return(FALSE);
  
  return(TRUE);
}


/************************************************************************
 * MITRE_header_from_be(): Swap the MITRE header bytes, if necessary.
 */

void MITRE_header_from_be(MITRE_header_t *header)
{
  header->product_code    = BE_to_si16(header->product_code);
  header->file_size       = BE_to_si32(header->file_size);
  header->time            = BE_to_si32(header->time);
  header->num_intensities = BE_to_si16(header->num_intensities);
  header->n_lat           = BE_to_si32(header->n_lat);
  header->n_lon           = BE_to_si32(header->n_lon);
  
  /*
   * Don't need to swap the intensity table because the data is
   * byte data.
   *
   * Don't need to swap the origin_lat, origin_lon, lat_delta and
   * lon_delta fields because they were stored in the file in ASCII
   * format.
   */

  return;
}


/************************************************************************
 * MITRE_print_header(): Prints a MITRE file header.
 */

void MITRE_print_header(FILE *stream,
			MITRE_header_t *header)
{
  time_t print_time = header->time;
  int i;
  
  fprintf(stream, "\nMITRE header:\n\n");
  fprintf(stream, "   product_code    = %d\n", header->product_code);
  fprintf(stream, "   file_size       = %d\n", header->file_size);
  fprintf(stream, "   time_stamp      = %s",
	  ctime(&print_time));
  fprintf(stream, "   num_intensities = %d\n", header->num_intensities);
  for (i = 0; i < MITRE_MAX_INTENSITIES; i++)
    fprintf(stream, "   intensity[%02d]   = %d\n",
	    i, header->intensity_table[i]);
  fprintf(stream, "   n_lat           = %d\n", header->n_lat);
  fprintf(stream, "   n_lon           = %d\n", header->n_lon);
  fprintf(stream, "   origin_lat      = %f\n", header->origin_lat);
  fprintf(stream, "   origin_lon      = %f\n", header->origin_lon);
  fprintf(stream, "   lat_delta       = %f\n", header->lat_delta);
  fprintf(stream, "   lon_delta       = %f\n", header->lon_delta);
  fprintf(stream, "\n\n");
  
  return;
}


/************************************************************************
 * MITRE_read_file(): Reads in a MITRE file.  Returns the mosaic data
 *                    in intensity level, as it appears in the input file.
 */

ui08 *MITRE_read_file(char *filename,
		      MITRE_header_t *mitre_header)
{
  static char *routine_name = "MITRE_read_file";
  
  FILE *in_file;
  char file_header[MITRE_FILE_HEADER_LEN];
  ui08 *data;
  int data_size;
  
  /*
   * Open the input file
   */

  if ((in_file = ta_fopen_uncompress(filename, "r")) == NULL)
  {
    fprintf(stderr, "ERROR - rapformats:%s\n", routine_name);
    fprintf(stderr, "cant open MITRE file\n");
    perror(filename);
    return(NULL);
  }
  
  /*
   * Read in the file header
   */

  if (ufread(file_header, 1, MITRE_FILE_HEADER_LEN, in_file)
      != MITRE_FILE_HEADER_LEN)
  {
    fprintf(stderr, "ERROR - rapformats:%s\n", routine_name);
    fprintf(stderr, "Can't read in MITRE file header\n");
    perror(filename);
    fclose(in_file);
    return(NULL);
  }

  MITRE_decode_header(file_header, mitre_header);
  MITRE_header_from_be(mitre_header);
  
  /*
   * Allocate the return buffer.
   */

  data_size = mitre_header->file_size - MITRE_FILE_HEADER_LEN;
  data = (ui08 *)umalloc((ui32)data_size);

  /*
   * Read in the data buffer.
   */

  if (ufread(data, 1, data_size, in_file) != data_size)
  {
    fprintf(stderr, "ERROR - rapformats:%s\n", routine_name);
    fprintf(stderr, "Can't read in MITRE data\n");
    perror(filename);
    fclose(in_file);
    return(NULL);
  }
  
  /*
   * Close the input file
   */

  fclose(in_file);

  return(data);
}


/************************************************************************
 * MITRE_read_file_inten(): Reads in a MITRE file.  Returns the mosaic
 *                          data with the intensity table values appied
 *                          to the data (the returned buffer contains
 *                          actual data values calculated using the
 *                          intensity table in the header.
 */

ui08 *MITRE_read_file_inten(char *filename,
			    MITRE_header_t *header)
{
  ui08 *dbz_data = MITRE_read_file(filename, header);
  int data_size = header->n_lat * header->n_lon;
  int i;
  
  for (i = 0; i < data_size; i++)
    dbz_data[i] = header->intensity_table[dbz_data[i]];
  
  return(dbz_data);
}
