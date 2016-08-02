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
 * update_file.c
 *
 * Update the origin values in the specified file.  Note that 
 * this program updates the files in place rather than making copies
 * of the files.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <sys/stat.h>

#include <mdv/mdv_file.h>
#include <mdv/mdv_macros.h>
#include <mdv/mdv_read.h>
#include <mdv/mdv_write.h>
#include <rapformats/dobson.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>

#include "mdv_update_origin.h"


void update_file(char *file_path)
{
  FILE *mdv_file;
  
  MDV_master_header_t master_hdr;
  MDV_field_header_t field_hdr;
  MDV_chunk_header_t chunk_hdr;
  
  int field_num;
  int chunk_num;
  
  fprintf(stderr, "Converting file %s\n", file_path);

  /*
   * Open the file.
   */

  if ((mdv_file = ta_fopen_uncompress(file_path, "r+")) == NULL)
  {
    fprintf(stderr,
	    "  Error opening file <%s> for update -- SKIPPING FILE\n",
	    file_path);

    return;
  }
  
  /*
   * Read in the master header so we can see how many field
   * headers there are.
   */

  if (MDV_load_master_header(mdv_file,
			     &master_hdr) != MDV_SUCCESS)
  {
    fprintf(stderr,
	    "  Error loading master header from file <%s> -- SKIPPING FILE\n",
	    file_path);

    fclose(mdv_file);
    return;
  }
  
  /*
   * Now loop through all of the fields updating the origins as
   * we go along.
   */

  for (field_num = 0; field_num < master_hdr.n_fields; field_num++)
  {
    /*
     * Read in the original field header.
     */

    if (MDV_load_field_header(mdv_file, &field_hdr, field_num)
	!= MDV_SUCCESS)
    {
      fprintf(stderr,
	      "  Error loading field %d header from file <%s> -- SKIPPING FIELD\n",
	      field_num, file_path);
      
      continue;
    }
    
    /*
     * Update the field values.
     */

    field_hdr.proj_origin_lat = Glob->params.new_grid_origin_lat;
    field_hdr.proj_origin_lon = Glob->params.new_grid_origin_lon;
    
    /*
     * Write the updated field header.
     */

    if (MDV_write_field_header(mdv_file, &field_hdr, field_num)
	!= MDV_SUCCESS)
    {
      fprintf(stderr, 
	      "  Error writing updated field %d header to file <%s>\n",
	      field_num, file_path);
    }
    
  } /* endfor - field_num */
  
  /*
   * Finally, see if the file contains a Dobson volume params chunk.
   * The TITAN processes overwrite the MDV information with this
   * information before processing the file.
   */

  for (chunk_num = 0; chunk_num < master_hdr.n_chunks; chunk_num++)
  {
    /*
     * Read in the chunk header.
     */

    if (MDV_load_chunk_header(mdv_file, &chunk_hdr,
			      &master_hdr, chunk_num) != MDV_SUCCESS)
    {
      fprintf(stderr,
	      "  Error loading chunk %d header from file <%s> -- SKIPPING CHUNK\n",
	      chunk_num, file_path);
      
      continue;
    }
    
    if (chunk_hdr.chunk_id == MDV_CHUNK_DOBSON_VOL_PARAMS)
    {
      vol_params_t *vol_params =
	(vol_params_t *)MDV_get_chunk_data(mdv_file, &chunk_hdr);
      
      if (vol_params == NULL)
      {
	fprintf(stderr,
		"  Error loading chunk %d data from file <%s> -- SKIPPING CHUNK\n",
		chunk_num, file_path);
      
	continue;
      } /* endif - vol_params == NULL */

      /*
       * Update the origin.
       */

      vol_params->cart.latitude = Glob->params.new_grid_origin_lat * 1000000;
      vol_params->cart.longitude = Glob->params.new_grid_origin_lon * 1000000;
      
      /*
       * Now write the chunk back to the file.
       */

      if (MDV_write_chunk_data(mdv_file, chunk_hdr.chunk_data_offset,
			       (void *)vol_params, chunk_hdr.size,
			       chunk_hdr.chunk_id, TRUE) != MDV_SUCCESS)
      {
	fprintf(stderr,
		"  Error writing chunk %d data to file <%s> -- SKIPPING CHUNK\n",
		chunk_num, file_path);
      
	continue;
      }
      
      ufree(vol_params);
      
    } /* endif - chunk_hdr.chunk_id == MDV_CHUNK_DOBSON_VOL_PARAMS */
    
  } /* endfor - chunk_num */
  
  /*
   * Close the file.
   */

  fclose(mdv_file);
  
  fprintf(stderr, "Wrote file %s\n", file_path);

}

