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
 * init_intermediate_grid.c
 *
 * Initialize the MDV structures which contain the intermediate
 * grids for output.
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

#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_user.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>

#include "verify_grid.h"

void init_intermediate_grid(void)
{
  MDV_master_header_t *master_hdr;
  MDV_field_header_t  *field_hdr;
  
  int field;
  
  /*
   * Initialize the dataset.
   */

  MDV_init_dataset(&Glob->inter_dataset);
  
  /*
   * Initialize the master header.
   */

  master_hdr =
    (MDV_master_header_t *)umalloc(sizeof(MDV_master_header_t));

  MDV_init_master_header(master_hdr);
  
  master_hdr->data_dimension = 2;
  master_hdr->data_collection_type = MDV_DATA_MIXED;
  master_hdr->native_vlevel_type = MDV_VERT_TYPE_MIXED;
  master_hdr->vlevel_type = MDV_VERT_TYPE_SURFACE;
  master_hdr->vlevel_included = FALSE;
  master_hdr->n_fields = 2;
  master_hdr->max_nx = Glob->params.grid.nx;
  master_hdr->max_ny = Glob->params.grid.ny;
  master_hdr->max_nz = 1;
  master_hdr->n_chunks = 0;
  
  STRcopy(master_hdr->data_set_info,
	  "Contingency table intermediate results",
	  MDV_INFO_LEN);
  STRcopy(master_hdr->data_set_name,
	  "Contingency table intermediate results",
	  MDV_NAME_LEN);
  
  MDV_set_master_hdr_offsets(master_hdr);
  
  Glob->inter_dataset.master_hdr = master_hdr;
  
  /*
   * Initialize the field headers.  There will be 2 fields:  the
   * derived truth flags on the contingency grid and the derived
   * detection flags on the contingency grid.
   */

  Glob->inter_dataset.fld_hdrs =
    (MDV_field_header_t **)umalloc(2 * sizeof(MDV_field_header_t *));
  
  Glob->inter_dataset.field_plane =
    (void ***)umalloc(2 * sizeof(void **));
  
  for (field = 0; field < 2; field++)
  {
    field_hdr = (MDV_field_header_t *)umalloc(sizeof(MDV_field_header_t));
    
    MDV_init_field_header(field_hdr);
    
    field_hdr->nx = Glob->params.grid.nx;
    field_hdr->ny = Glob->params.grid.ny;
    field_hdr->nz = 1;
    if (Glob->params.projection == PROJ_LATLON)
      field_hdr->proj_type = MDV_PROJ_LATLON;
    else
      field_hdr->proj_type = MDV_PROJ_FLAT;
    field_hdr->encoding_type = MDV_INT8;
    field_hdr->data_element_nbytes = 1;
    field_hdr->volume_size = field_hdr->nx * field_hdr->ny;
    
    field_hdr->proj_origin_lat = Glob->params.grid.miny;
    field_hdr->proj_origin_lon = Glob->params.grid.minx;
    field_hdr->grid_dx = Glob->params.grid.dx;
    field_hdr->grid_dy = Glob->params.grid.dy;
    field_hdr->grid_dz = 1.0;
    field_hdr->grid_minx = Glob->params.grid.minx;
    field_hdr->grid_miny = Glob->params.grid.miny;
    field_hdr->grid_minz = 1.0;
    field_hdr->scale = 1.0;
    field_hdr->bias = 0.0;
    field_hdr->bad_data_value = -1.0;
    field_hdr->missing_data_value = -1.0;
    field_hdr->proj_rotation = 0.0;
    
    if (field == 0)
    {
      field_hdr->field_data_offset = MDV_get_first_field_offset(master_hdr);
      
      STRcopy(field_hdr->field_name_long,
	      "Truth Contingency Grid",
	      MDV_LONG_FIELD_LEN);
      STRcopy(field_hdr->field_name,
	      "Truth Grid",
	      MDV_SHORT_FIELD_LEN);
    }
    else
    {
      field_hdr->field_data_offset =
	MDV_get_first_field_offset(master_hdr) + field_hdr->volume_size;
      
      STRcopy(field_hdr->field_name_long,
	      "Detection Contingency Grid",
	      MDV_LONG_FIELD_LEN);
      STRcopy(field_hdr->field_name,
	      "Detection Grid",
	      MDV_SHORT_FIELD_LEN);
    }

    Glob->inter_dataset.fld_hdrs[field] = field_hdr;
      
    /*
     * Each field has 1 plane
     */

    Glob->inter_dataset.field_plane[field] =
      (void **)umalloc(1 * sizeof(void *));
    
    Glob->inter_dataset.field_plane[field][0] = NULL;
    
  } /* endfor - field */
  
  Glob->inter_dataset.nfields_alloc = 2;
  
  
  return;
}  
