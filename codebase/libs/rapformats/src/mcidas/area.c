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
 * area.c: Routines to manipulate McIDAS AREA data.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/os_config.h>

#include <dataport/port_types.h>
#include <dataport/swap.h>

#include <toolsa/mem.h>

#include <rapformats/mcidas_area.h>

/************************************************************************
 * AREA_nav_type_to_id(): Converts the navigation type string (note that
 *                        this string is NOT null-terminated) into an
 *                        internal id, defined as AREA_NAV_xxx.
 *
 * Returns the appropriate id (AREA_NAV_xxxx) for the string.  If the
 * navigation type was unknown, returns AREA_NAV_UNKNOWN.
 */

int AREA_nav_type_to_id(char *nav_type)
{
  if (nav_type[0] == 'G' &&
      nav_type[1] == 'O' &&
      nav_type[2] == 'E' &&
      nav_type[3] == 'S')
    return(AREA_NAV_GOES);
  
  if (nav_type[0] == 'T' &&
      nav_type[1] == 'I' &&
      nav_type[2] == 'R' &&
      nav_type[3] == 'O')
    return(AREA_NAV_TIRO);
  
  if (nav_type[0] == 'G' &&
      nav_type[1] == 'V' &&
      nav_type[2] == 'A' &&
      nav_type[3] == 'R')
    return(AREA_NAV_GVAR);
  
  if (nav_type[0] == 'P' &&
      nav_type[1] == 'S' &&
      nav_type[2] == ' ' &&
      nav_type[3] == ' ')
    return(AREA_NAV_PS);
  
  if (nav_type[0] == 'L' &&
      nav_type[1] == 'A' &&
      nav_type[2] == 'M' &&
      nav_type[3] == 'B')
    return(AREA_NAV_LAMB);
  
  if (nav_type[0] == 'R' &&
      nav_type[1] == 'A' &&
      nav_type[2] == 'D' &&
      nav_type[3] == 'R')
    return(AREA_NAV_RADR);
  
  if (nav_type[0] == 'R' &&
      nav_type[1] == 'E' &&
      nav_type[2] == 'C' &&
      nav_type[3] == 'T')
    return(AREA_NAV_RECT);
  
  if (nav_type[0] == 'M' &&
      nav_type[1] == 'E' &&
      nav_type[2] == 'R' &&
      nav_type[3] == 'C')
    return(AREA_NAV_MERC);
  
  if (nav_type[0] == 'M' &&
      nav_type[1] == 'E' &&
      nav_type[2] == 'T' &&
      nav_type[3] == ' ')
    return(AREA_NAV_MET);
  
  if (nav_type[0] == ' ' &&
      nav_type[1] == ' ' &&
      nav_type[2] == ' ' &&
      nav_type[3] == ' ')
    return(AREA_NAV_NONE);
  
  return(AREA_NAV_UNKNOWN);
}


/************************************************************************
 * AREA_needs_swap(): Determines whether the data read in from a McIDAS
 *                    AREA file needs to be swapped based on the type
 *                    value in the directory block.  This value should
 *                    always be equal to the AREA_VERSION_NUMBER.
 *
 * Returns TRUE if the data needs to be swapped, FALSE otherwise.
 */

int AREA_needs_swap(AREA_dir_block_t *dir_block)
{
  return(!(dir_block->type == AREA_VERSION_NUMBER));
}


/************************************************************************
 * AREA_print_dir_block(): Print the contents of an AREA file directory
 *                         block in ASCII format to the given stream.
 */

void AREA_print_dir_block(FILE *stream, AREA_dir_block_t *dir_block)
{
  int i;
  
  fprintf(stream, "\nDirectory Block:\n");
  fprintf(stream, "\n");
  fprintf(stream, "rel_position = %d\n", dir_block->rel_position);
  fprintf(stream, "type = %d\n", dir_block->type);
  fprintf(stream, "satellite_id = %d\n", dir_block->satellite_id);
  fprintf(stream, "image_date = %d\n", dir_block->image_date);
  fprintf(stream, "image_time = %d\n", dir_block->image_time);
  fprintf(stream, "y_coord = %d\n", dir_block->y_coord);
  fprintf(stream, "x_coord = %d\n", dir_block->x_coord);
  fprintf(stream, "unused1 = %d\n", dir_block->unused1);
  fprintf(stream, "y_size = %d\n", dir_block->y_size);
  fprintf(stream, "x_size = %d\n", dir_block->x_size);
  fprintf(stream, "z_size = %d\n", dir_block->z_size);
  fprintf(stream, "y_res = %d\n", dir_block->y_res);
  fprintf(stream, "x_res = %d\n", dir_block->x_res);
  fprintf(stream, "z_res = %d\n", dir_block->z_res);
  fprintf(stream, "line_prefix = %d\n", dir_block->line_prefix);
  fprintf(stream, "proj_num = %d\n", dir_block->proj_num);
  fprintf(stream, "create_date = %d\n", dir_block->create_date);
  fprintf(stream, "create_time = %d\n", dir_block->create_time);
  fprintf(stream, "filter_map = %x\n", dir_block->filter_map);
  fprintf(stream, "image_id = %d\n", dir_block->image_id);
  for (i = 0; i < AREA_SENSOR_DATA_LEN; i++)
    fprintf(stream, "sensor_data[%d] = %d\n", i, dir_block->sensor_data[i]);
  fprintf(stream, "memo = ");
  for (i = 0; i < AREA_MEMO_LEN; i++)
    fprintf(stream, "%c", dir_block->memo[i]);
  fprintf(stream, "\n");
  fprintf(stream, "unused2 = %d\n", dir_block->unused2);
  fprintf(stream, "data_offset = %d\n", dir_block->data_offset);
  fprintf(stream, "nav_offset = %d\n", dir_block->nav_offset);
  fprintf(stream, "validity_code = %d\n", dir_block->validity_code);
  for (i = 0; i < AREA_PDL_LEN; i++)
    fprintf(stream, "pdl[%d] = %d\n", i, dir_block->pdl[i]);
  fprintf(stream, "band_8_source = %d\n", dir_block->band_8_source);
  fprintf(stream, "start_date = %d\n", dir_block->start_date);
  fprintf(stream, "start_time = %d\n", dir_block->start_time);
  fprintf(stream, "start_scan = %d\n", dir_block->start_scan);
  fprintf(stream, "prefix_doc_len = %d\n", dir_block->prefix_doc_len);
  fprintf(stream, "prefix_cal_len = %d\n", dir_block->prefix_cal_len);
  fprintf(stream, "prefix_lev_len = %d\n", dir_block->prefix_lev_len);
  fprintf(stream, "source_type = ");
  for (i = 0; i < AREA_SOURCE_TYPE_LEN; i++)
    fprintf(stream, "%c", dir_block->source_type[i]);
  fprintf(stream, "\n");
  fprintf(stream, "calib_type = ");
  for (i = 0; i < AREA_CALIB_TYPE_LEN; i++)
    fprintf(stream, "%c", dir_block->calib_type[i]);
  fprintf(stream, "\n");
  for (i = 0; i < AREA_UNUSED3_LEN; i++)
    fprintf(stream, "unused3[%d] = %d\n", i, dir_block->unused3[i]);
  fprintf(stream, "suppl_offset = %d\n", dir_block->suppl_offset);
  fprintf(stream, "suppl_len = %d\n", dir_block->suppl_len);
  fprintf(stream, "unused4 = %d\n", dir_block->unused4);
  fprintf(stream, "cal_offset = %d\n", dir_block->cal_offset);
  fprintf(stream, "num_comments = %d\n", dir_block->num_comments);
  fprintf(stream, "\n\n");
  
  return;
}


/************************************************************************
 * AREA_print_lamb_nav_block(): Print the contents of an AREA file LAMB
 *                              navigation block in ASCII format to the
 *                              given stream.
 */

void AREA_print_lamb_nav_block(FILE *stream,
			       AREA_lamb_nav_block_t *nav_block)
{
  int i;
  
  fprintf(stream, "\nNavigation block:\n");
  fprintf(stream, "\n");
  fprintf(stream, "nav_type = ");
  for (i = 0; i < AREA_NAV_TYPE_LEN; i++)
    fprintf(stream, "%c", nav_block->nav_type[i]);
  fprintf(stream, "\n");
  fprintf(stream, "np_line = %d\n", nav_block->np_line);
  fprintf(stream, "np_element = %d\n", nav_block->np_element);
  fprintf(stream, "latitude_1 = %d\n", nav_block->latitude_1);
  fprintf(stream, "latitude_2 = %d\n", nav_block->latitude_2);
  fprintf(stream, "lat_spacing = %d\n", nav_block->lat_spacing);
  fprintf(stream, "longitude = %d\n", nav_block->longitude);
  fprintf(stream, "planet_radius = %d M\n", nav_block->planet_radius);
  fprintf(stream, "planet_ecc = %d (%f)\n",
	  nav_block->planet_ecc,
	  (double)nav_block->planet_ecc / 1000000.0);
  fprintf(stream, "coord_type = %d\n", nav_block->coord_type);
  fprintf(stream, "long_conv = %d\n", nav_block->long_conv);
  for (i = 0; i < AREA_NAV_LAMB_UNUSED_LEN; i++)
    fprintf(stream, "unused[%d] = %d\n", i, nav_block->unused[i]);
  fprintf(stream, "memo = ");
  for (i = 0; i < AREA_NAV_LAMB_MEMO_LEN; i++)
    fprintf(stream, "%c", nav_block->memo[i]);
  fprintf(stream, "\n");
  fprintf(stream, "\n");
  
  return;
}


/************************************************************************
 * AREA_print_radr_nav_block(): Print the contents of an AREA file RADR
 *                              navigation block in ASCII format to the
 *                              given stream.
 */

void AREA_print_radr_nav_block(FILE *stream,
			       AREA_radr_nav_block_t *nav_block)
{
  int i;
  
  fprintf(stream, "\nNavigation block:\n");
  fprintf(stream, "\n");
  fprintf(stream, "nav_type = ");
  for (i = 0; i < AREA_NAV_TYPE_LEN; i++)
    fprintf(stream, "%c", nav_block->nav_type[i]);
  fprintf(stream, "\n");
  fprintf(stream, "row = %d\n", nav_block->row);
  fprintf(stream, "column = %d\n", nav_block->column);
  fprintf(stream, "latitude = %d\n", nav_block->latitude);
  fprintf(stream, "longitude = %d\n", nav_block->longitude);
  fprintf(stream, "resolution = %d\n", nav_block->resolution);
  fprintf(stream, "rotation = %d (%f degrees)\n",
	  nav_block->rotation,
	  (double)nav_block->rotation / 1000.0);
  fprintf(stream, "lon_resolution = %d\n", nav_block->lon_resolution);
  
  return;
}


/************************************************************************
 * AREA_print_rect_nav_block(): Print the contents of an AREA file RECT
 *                              navigation block in ASCII format to the
 *                              given stream.
 */

void AREA_print_rect_nav_block(FILE *stream,
			       AREA_rect_nav_block_t *nav_block)
{
  int i;
  
  fprintf(stream, "\nNavigation block:\n");
  fprintf(stream, "\n");
  fprintf(stream, "nav_type = ");
  for (i = 0; i < AREA_NAV_TYPE_LEN; i++)
    fprintf(stream, "%c", nav_block->nav_type[i]);
  fprintf(stream, "\n");
  fprintf(stream, "row_num = %d\n", nav_block->row_num);
  fprintf(stream, "row_latitude = %d (%f degrees)\n",
	  nav_block->row_latitude,
	  (double)nav_block->row_latitude / 10000.0);
  fprintf(stream, "column_num = %d\n", nav_block->column_num);
  fprintf(stream, "column_longitude = %d (%f degrees)\n",
	  nav_block->column_longitude,
	  (double)nav_block->column_longitude / 10000.0);
  fprintf(stream, "degrees_lat = %d (%f degrees)\n",
	  nav_block->degrees_lat,
	  (double)nav_block->degrees_lat / 10000.0);
  fprintf(stream, "degrees_lon = %d (%f degrees)\n",
	  nav_block->degrees_lon,
	  (double)nav_block->degrees_lon / 10000.0);
  fprintf(stream, "planet_radius = %d m\n", nav_block->planet_radius);
  fprintf(stream, "planet_ecc = %d (%f)\n",
	  nav_block->planet_ecc,
	  (double)nav_block->planet_ecc / 1000000.0);
  fprintf(stream, "coord_type = %d\n", nav_block->coord_type);
  fprintf(stream, "long_conv = %d\n", nav_block->long_conv);
  
  return;
}


/************************************************************************
 * AREA_swap_dir_block(): Swap the bytes in all of the integers in an
 *                        AREA file directory block.
 */

void AREA_swap_dir_block(AREA_dir_block_t *dir_block)
{
  dir_block->rel_position     = SWAP_si32(dir_block->rel_position);
  dir_block->type             = SWAP_si32(dir_block->type);
  dir_block->satellite_id     = SWAP_si32(dir_block->satellite_id);
  dir_block->image_date       = SWAP_si32(dir_block->image_date);
  dir_block->image_time       = SWAP_si32(dir_block->image_time);
  dir_block->y_coord          = SWAP_si32(dir_block->y_coord);
  dir_block->x_coord          = SWAP_si32(dir_block->x_coord);
  dir_block->unused1          = SWAP_si32(dir_block->unused1);
  dir_block->y_size           = SWAP_si32(dir_block->y_size);
  dir_block->x_size           = SWAP_si32(dir_block->x_size);
  dir_block->z_size           = SWAP_si32(dir_block->z_size);
  dir_block->y_res            = SWAP_si32(dir_block->y_res);
  dir_block->x_res            = SWAP_si32(dir_block->x_res);
  dir_block->z_res            = SWAP_si32(dir_block->z_res);
  dir_block->line_prefix      = SWAP_si32(dir_block->line_prefix);
  dir_block->proj_num         = SWAP_si32(dir_block->proj_num);
  dir_block->create_date      = SWAP_si32(dir_block->create_date);
  dir_block->create_time      = SWAP_si32(dir_block->create_time);
  dir_block->filter_map       = SWAP_si32(dir_block->filter_map);
  dir_block->image_id         = SWAP_si32(dir_block->image_id);
  SWAP_array_32((ui32 *) dir_block->sensor_data, AREA_SENSOR_DATA_LEN * sizeof(si32));
  /* memo is okay */
  dir_block->unused2          = SWAP_si32(dir_block->unused2);
  dir_block->data_offset      = SWAP_si32(dir_block->data_offset);
  dir_block->nav_offset       = SWAP_si32(dir_block->nav_offset);
  dir_block->validity_code    = SWAP_si32(dir_block->validity_code);
  SWAP_array_32((ui32 *) dir_block->pdl, AREA_PDL_LEN * sizeof(si32));
  dir_block->band_8_source    = SWAP_si32(dir_block->band_8_source);
  dir_block->start_date       = SWAP_si32(dir_block->start_date);
  dir_block->start_time       = SWAP_si32(dir_block->start_time);
  dir_block->start_scan       = SWAP_si32(dir_block->start_scan);
  dir_block->prefix_doc_len   = SWAP_si32(dir_block->prefix_doc_len);
  dir_block->prefix_cal_len   = SWAP_si32(dir_block->prefix_cal_len);
  dir_block->prefix_lev_len   = SWAP_si32(dir_block->prefix_lev_len);
  /* source_type is okay */
  /* calib_type is 0kay */
  SWAP_array_32((ui32 *) dir_block->unused3, AREA_UNUSED3_LEN * sizeof(si32));
  dir_block->suppl_offset     = SWAP_si32(dir_block->suppl_offset);
  dir_block->suppl_len        = SWAP_si32(dir_block->suppl_len);
  dir_block->unused4          = SWAP_si32(dir_block->unused4);
  dir_block->cal_offset       = SWAP_si32(dir_block->cal_offset);
  dir_block->num_comments     = SWAP_si32(dir_block->num_comments);
  
  return;
}


/************************************************************************
 * AREA_swap_lamb_nav_block(): Swap the bytes in all of the integers in an
 *                             AREA file LAMB navigation block.
 */

void AREA_swap_lamb_nav_block(AREA_lamb_nav_block_t *nav_block)
{
  /* nav_type is okay */
  nav_block->np_line          = SWAP_si32(nav_block->np_line);
  nav_block->np_element       = SWAP_si32(nav_block->np_element);
  nav_block->latitude_1       = SWAP_si32(nav_block->latitude_1);
  nav_block->latitude_2       = SWAP_si32(nav_block->latitude_2);
  nav_block->lat_spacing      = SWAP_si32(nav_block->lat_spacing);
  nav_block->longitude        = SWAP_si32(nav_block->longitude);
  nav_block->planet_radius    = SWAP_si32(nav_block->planet_radius);
  nav_block->planet_ecc       = SWAP_si32(nav_block->planet_ecc);
  nav_block->coord_type       = SWAP_si32(nav_block->coord_type);
  nav_block->long_conv        = SWAP_si32(nav_block->long_conv);
  SWAP_array_32((ui32 *) nav_block->unused, AREA_NAV_LAMB_UNUSED_LEN * sizeof(si32));
  /* memo is okay */

  return;
}


/************************************************************************
 * AREA_swap_radr_nav_block(): Swap the bytes in all of the integers in an
 *                             AREA file RADR navigation block.
 */

void AREA_swap_radr_nav_block(AREA_radr_nav_block_t *nav_block)
{
  /* nav_type is okay */
  nav_block->row              = SWAP_si32(nav_block->row);
  nav_block->column           = SWAP_si32(nav_block->column);
  nav_block->latitude         = SWAP_si32(nav_block->latitude);
  nav_block->longitude        = SWAP_si32(nav_block->longitude);
  nav_block->resolution       = SWAP_si32(nav_block->resolution);
  nav_block->rotation         = SWAP_si32(nav_block->rotation);
  nav_block->lon_resolution   = SWAP_si32(nav_block->lon_resolution);
  
  return;
}


/************************************************************************
 * AREA_swap_rect_nav_block(): Swap the bytes in all of the integers in an
 *                             AREA file RECT navigation block.
 */

void AREA_swap_rect_nav_block(AREA_rect_nav_block_t *nav_block)
{
  /* nav_type is okay */
  nav_block->row_num          = SWAP_si32(nav_block->row_num);
  nav_block->row_latitude     = SWAP_si32(nav_block->row_latitude);
  nav_block->column_num       = SWAP_si32(nav_block->column_num);
  nav_block->column_longitude = SWAP_si32(nav_block->column_longitude);
  nav_block->degrees_lat      = SWAP_si32(nav_block->degrees_lat);
  nav_block->degrees_lon      = SWAP_si32(nav_block->degrees_lon);
  nav_block->planet_radius    = SWAP_si32(nav_block->planet_radius);
  nav_block->planet_ecc       = SWAP_si32(nav_block->planet_ecc);
  nav_block->coord_type       = SWAP_si32(nav_block->coord_type);
  nav_block->long_conv        = SWAP_si32(nav_block->long_conv);
  
  return;
}
